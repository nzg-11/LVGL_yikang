#if 0
#include "lv_msg_center.h"
#include "lv_homepage.h"
#include "msg_center_com.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

// 只保留页面/日历相关静态变量
static lv_calendar_date_t g_selected_calendar_date = {0};
static lv_calendar_date_t g_highlighted_dates[1];
static lv_style_t g_calendar_selected_style;
static lv_style_t msg_center_grad_style;
static bool msg_center_style_inited = false;
static lv_obj_t *g_calendar = NULL;

// 内部工具函数
static void init_msg_center_styles(void);
static void get_current_date(int *year, int *month, int *day);
static void update_calendar_selected_style(lv_obj_t *calendar, lv_calendar_date_t *new_date);
static void clean_calendar_obj(void);
static void calendar_event_handler(lv_event_t *e);
static void create_calendar(lv_obj_t *parent);
static void date_picker_click_cb(lv_event_t *e);


//==================== 实现 =====================
static void init_msg_center_styles(void)
{
    if(!msg_center_style_inited) {
        lv_style_init(&msg_center_grad_style);
        lv_style_init(&g_calendar_selected_style);
        lv_style_set_border_color(&g_calendar_selected_style, lv_color_hex(0x006BDC));
        lv_style_set_border_width(&g_calendar_selected_style, 2);
        lv_style_set_border_opa(&g_calendar_selected_style, LV_OPA_100);
        lv_style_set_bg_opa(&g_calendar_selected_style, LV_OPA_0);
        msg_center_style_inited = true;
    }
}

static void get_current_date(int *year, int *month, int *day)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if(!t) { *year=2025; *month=12; *day=12; return; }
    *year = t->tm_year + 1900;
    *month = t->tm_mon + 1;
    *day = t->tm_mday;
}

static void update_calendar_selected_style(lv_obj_t *calendar, lv_calendar_date_t *new_date)
{
    if(!calendar || !new_date) return;
    g_selected_calendar_date = *new_date;
    g_highlighted_dates[0] = *new_date;
    lv_calendar_set_highlighted_dates(calendar, g_highlighted_dates, 1);
    lv_obj_remove_style(calendar, &g_calendar_selected_style, LV_PART_ITEMS);
    lv_obj_add_style(calendar, &g_calendar_selected_style, LV_PART_ITEMS);
}


// 清理日历对象
static void clean_calendar_obj(void)
{
    if(g_calendar != NULL) {
        if(lv_obj_is_valid(g_calendar)) {
            lv_obj_del(g_calendar); 
        }
        g_calendar = NULL; 
        memset(&g_selected_calendar_date, 0, sizeof(lv_calendar_date_t));
        memset(g_highlighted_dates, 0, sizeof(g_highlighted_dates));
    }
}

// 日历事件回调
static void calendar_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_current_target(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_calendar_date_t selected_date;
        if(lv_calendar_get_pressed_date(obj, &selected_date)) {
            // 1. 获取当前系统日期
            int curr_year, curr_month, curr_day;
            get_current_date(&curr_year, &curr_month, &curr_day);

            // 2. 判断选中的日期是否是未来日期
            bool is_future = false;
            if(selected_date.year > curr_year) {
                is_future = true;
            } else if(selected_date.year == curr_year) {
                if(selected_date.month > curr_month) {
                    is_future = true;
                } else if(selected_date.month == curr_month) {
                    if(selected_date.day > curr_day) {
                        is_future = true;
                    }
                }
            }

            // 3. 仅处理当天及过去的日期
            if(is_future) {
                LV_LOG_USER("Selected date %04d-%02d-%02d is future, ignore!", 
                            selected_date.year, selected_date.month, selected_date.day);
                return; // 未来日期：不更新标签，直接返回
            }

            //   ：更新选中日期的蓝色边框样式
            update_calendar_selected_style(obj, &selected_date);

            // 4. 过去/当天日期：正常更新
            char date_str[32];
            snprintf(date_str, sizeof(date_str), "%04d-%02d-%02d", 
                    selected_date.year, selected_date.month, selected_date.day);
            if(visitor_data) {
                lv_label_set_text(visitor_data, date_str);
            }
            LV_LOG_USER("Selected history date: %s", date_str);
            
            // 只隐藏日历，不碰表头
            if(g_calendar && lv_obj_is_valid(g_calendar) && !lv_obj_has_flag(g_calendar, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_add_flag(g_calendar, LV_OBJ_FLAG_HIDDEN); 
            }
        }
    }
}

// 创建日历控件
static void create_calendar(lv_obj_t *parent)
{
    if(g_calendar != NULL) return; // 避免重复创建

    // 获取系统实时时间
    int year, month, day;
    get_current_date(&year, &month, &day);

    // 创建日历对象
    g_calendar = lv_calendar_create(parent);
    lv_obj_set_size(g_calendar, 456, 484);
    lv_obj_set_pos(g_calendar, 301, 255);
    lv_obj_add_event_cb(g_calendar, calendar_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(g_calendar, LV_OBJ_FLAG_HIDDEN);

    // 设置日历基础参数：使用实时时间
    lv_calendar_set_today_date(g_calendar, year, month, day);
    lv_calendar_set_showed_date(g_calendar, year, month);

    // 初始化当天日期的蓝色边框样式
    lv_calendar_date_t today_date = {year, month, day};
    update_calendar_selected_style(g_calendar, &today_date);

    // ===== 表头创建逻辑 =====
#if LV_USE_CALENDAR_HEADER_DROPDOWN
    lv_calendar_header_dropdown_create(g_calendar); // 仅保留这行，不额外修改
#elif LV_USE_CALENDAR_HEADER_ARROW
    lv_calendar_header_arrow_create(g_calendar);
#endif
}

// 日期选择器点击回调
static void date_picker_click_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_current_target(e);
    LV_UNUSED(obj); // 避免未使用变量警告

    if(g_calendar == NULL) {
        // 首次点击：创建日历并显示
        create_calendar(lv_obj_get_parent(date_picker));
        lv_obj_clear_flag(g_calendar, LV_OBJ_FLAG_HIDDEN);
    } else {
        // 非首次点击：切换日历显示/隐藏状态
        if(lv_obj_has_flag(g_calendar, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_clear_flag(g_calendar, LV_OBJ_FLAG_HIDDEN); // 显示日历
        } else {
            lv_obj_add_flag(g_calendar, LV_OBJ_FLAG_HIDDEN);   // 隐藏日历
        }
    }
}


//==================== 主界面创建 =====================
void ui_msg_center_create(lv_obj_t *homepage_scr)
{
    init_msg_center_styles();
    if(!homepage_scr || !lv_obj_is_valid(homepage_scr)) return;

    clean_calendar_obj();
    clear_all_visitor_records();
    clear_all_doorbell_records();
    clear_all_alarm_records();

    if(!msg_center_scr) {
        msg_center_scr = lv_obj_create(NULL);
    } else {
        lv_obj_clean(msg_center_scr);
    }

    lv_style_reset(&msg_center_grad_style);
    lv_style_set_bg_color(&msg_center_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&msg_center_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&msg_center_grad_style, LV_GRAD_DIR_VER);
    lv_obj_add_style(msg_center_scr, &msg_center_grad_style, 0);

    // 标题
    lv_obj_t *title = create_text_label(msg_center_scr,
        "message center", &lv_font_montserrat_36,
        lv_color_hex(0xFFFFFF), 83, 80, LV_OPA_100);

    // 模式容器
    lv_obj_t *mode_con = create_container(msg_center_scr,
        47,176,450,46,
        lv_color_hex(0x1F3150), LV_OPA_100, 31,
        lv_color_hex(0x1F3150), 0, LV_OPA_0);
    lv_obj_set_style_pad_all(mode_con, 0, 0);

    // 访客
    visitor_rec_con = create_custom_gradient_container(mode_con,
        0,0,150,46, 31, 0x006BDC,0x00BDBD,LV_GRAD_DIR_HOR, 0,255,LV_OPA_100);
    lv_obj_set_style_pad_all(visitor_rec_con,0,0);
    lv_obj_add_flag(visitor_rec_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(visitor_rec_con, rec_con_click_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *vt = create_text_label(visitor_rec_con,
        "vis record", &lv_font_montserrat_20,
        lv_color_hex(0xFFFFFF),27,10,LV_OPA_100);

    // 报警
    alarm_rec_con = lv_obj_create(mode_con);
    lv_obj_set_pos(alarm_rec_con,150,0);
    lv_obj_set_size(alarm_rec_con,150,46);
    lv_obj_set_style_radius(alarm_rec_con,31,0);
    lv_obj_set_style_bg_opa(alarm_rec_con,LV_OPA_100,0);
    lv_obj_set_style_bg_grad_dir(alarm_rec_con,LV_GRAD_DIR_NONE,0);
    lv_obj_set_style_bg_color(alarm_rec_con,lv_color_hex(0x1F3150),0);
    lv_obj_set_style_border_width(alarm_rec_con,0,0);
    lv_obj_clear_flag(alarm_rec_con, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(alarm_rec_con,0,0);
    lv_obj_add_flag(alarm_rec_con,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(alarm_rec_con,rec_con_click_cb,LV_EVENT_CLICKED,NULL);
    lv_obj_t *at = create_text_label(alarm_rec_con,
        "ala record",&lv_font_montserrat_20,
        lv_color_hex(0xFFFFFF),27,10,LV_OPA_100);

    // 门铃
    doorbell_rec_con = lv_obj_create(mode_con);
    lv_obj_set_pos(doorbell_rec_con,300,0);
    lv_obj_set_size(doorbell_rec_con,150,46);
    lv_obj_set_style_radius(doorbell_rec_con,31,0);
    lv_obj_set_style_bg_opa(doorbell_rec_con,LV_OPA_100,0);
    lv_obj_set_style_bg_grad_dir(doorbell_rec_con,LV_GRAD_DIR_NONE,0);
    lv_obj_set_style_bg_color(doorbell_rec_con,lv_color_hex(0x1F3150),0);
    lv_obj_set_style_border_width(doorbell_rec_con,0,0);
    lv_obj_clear_flag(doorbell_rec_con, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(doorbell_rec_con,0,0);
    lv_obj_add_flag(doorbell_rec_con,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(doorbell_rec_con,rec_con_click_cb,LV_EVENT_CLICKED,NULL);
    lv_obj_t *bt = create_text_label(doorbell_rec_con,
        "bell record",&lv_font_montserrat_20,
        lv_color_hex(0xFFFFFF),27,10,LV_OPA_100);

    g_selected_rec_con = visitor_rec_con;

    // 日期选择
    // date_picker = create_image_obj(msg_center_scr, "H:data.png",522,175);
    date_picker = create_container_circle(msg_center_scr, 522, 175, 30,
    true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_set_style_bg_opa(date_picker, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(date_picker,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(date_picker,LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(date_picker,date_picker_click_cb,LV_EVENT_CLICKED,NULL);

    // 当前日期
    int y,m,d;
    get_current_date(&y,&m,&d);
    char buf[32];
    snprintf(buf,sizeof(buf),"%04d-%02d-%02d",y,m,d);
    visitor_data = create_text_label(msg_center_scr,
        buf,&lv_font_montserrat_28,
        lv_color_hex(0xFFFFFF),47,248,LV_OPA_100);

    // 返回
    //lv_obj_t *back = create_image_obj(msg_center_scr,"H:back.png",52,123);
    lv_obj_t *back_btn = create_container_circle(msg_center_scr, 52, 123, 30,
    true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(back_btn,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn,LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn,back_btn_click_cb,LV_EVENT_CLICKED,homepage_scr);

    // 默认显示访客
    switch_record_type(visitor_rec_con);
    update_status_bar_parent(msg_center_scr);
    lv_scr_load(msg_center_scr);
}

void msg_center_btn_click_cb(lv_event_t *e)
{
    lv_obj_t *home = lv_event_get_user_data(e);
    if(home && lv_obj_is_valid(home)) {
        ui_msg_center_create(home);
    }
}
#else 
#include "lv_msg_center.h"
#include "lv_homepage.h"
#include "msg_center_com.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

// 只保留页面/日历相关静态变量
static lv_calendar_date_t g_selected_calendar_date = {0};
static lv_calendar_date_t g_highlighted_dates[1];
static lv_style_t g_calendar_selected_style;
static lv_style_t msg_center_grad_style;
static bool msg_center_style_inited = false;
static lv_obj_t *g_calendar = NULL;

// 内部工具函数
static void init_msg_center_styles(void);
static void get_current_date(int *year, int *month, int *day);
static void update_calendar_selected_style(lv_obj_t *calendar, lv_calendar_date_t *new_date);
static void clean_calendar_obj(void);
static void calendar_event_handler(lv_event_t *e);
static void create_calendar(lv_obj_t *parent);
static void date_picker_click_cb(lv_event_t *e);


//==================== 实现 =====================
static void init_msg_center_styles(void)
{
    if(!msg_center_style_inited) {
        lv_style_init(&msg_center_grad_style);
        lv_style_init(&g_calendar_selected_style);
        lv_style_set_border_color(&g_calendar_selected_style, lv_color_hex(0x006BDC));
        lv_style_set_border_width(&g_calendar_selected_style, 2);
        lv_style_set_border_opa(&g_calendar_selected_style, LV_OPA_100);
        lv_style_set_bg_opa(&g_calendar_selected_style, LV_OPA_0);
        msg_center_style_inited = true;
    }
}

static void get_current_date(int *year, int *month, int *day)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if(!t) { *year=2025; *month=12; *day=12; return; }
    *year = t->tm_year + 1900;
    *month = t->tm_mon + 1;
    *day = t->tm_mday;
}

static void update_calendar_selected_style(lv_obj_t *calendar, lv_calendar_date_t *new_date)
{
    if(!calendar || !new_date) return;
    g_selected_calendar_date = *new_date;
    g_highlighted_dates[0] = *new_date;
    lv_calendar_set_highlighted_dates(calendar, g_highlighted_dates, 1);
    lv_obj_remove_style(calendar, &g_calendar_selected_style, LV_PART_ITEMS);
    lv_obj_add_style(calendar, &g_calendar_selected_style, LV_PART_ITEMS);
}


// 清理日历对象
static void clean_calendar_obj(void)
{
    if(g_calendar != NULL) {
        if(lv_obj_is_valid(g_calendar)) {
            lv_obj_del(g_calendar); 
        }
        g_calendar = NULL; 
        memset(&g_selected_calendar_date, 0, sizeof(lv_calendar_date_t));
        memset(g_highlighted_dates, 0, sizeof(g_highlighted_dates));
    }
}

// 日历事件回调
static void calendar_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_current_target(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_calendar_date_t selected_date;
        if(lv_calendar_get_pressed_date(obj, &selected_date)) {
            // 1. 获取当前系统日期
            int curr_year, curr_month, curr_day;
            get_current_date(&curr_year, &curr_month, &curr_day);

            // 2. 判断选中的日期是否是未来日期
            bool is_future = false;
            if(selected_date.year > curr_year) {
                is_future = true;
            } else if(selected_date.year == curr_year) {
                if(selected_date.month > curr_month) {
                    is_future = true;
                } else if(selected_date.month == curr_month) {
                    if(selected_date.day > curr_day) {
                        is_future = true;
                    }
                }
            }

            // 3. 仅处理当天及过去的日期
            if(is_future) {
                LV_LOG_USER("Selected date %04d-%02d-%02d is future, ignore!", 
                            selected_date.year, selected_date.month, selected_date.day);
                return; // 未来日期：不更新标签，直接返回
            }

            //   ：更新选中日期的蓝色边框样式
            update_calendar_selected_style(obj, &selected_date);

            // 4. 过去/当天日期：正常更新
            char date_str[32];
            snprintf(date_str, sizeof(date_str), "%04d-%02d-%02d", 
                    selected_date.year, selected_date.month, selected_date.day);
            if(visitor_data) {
                lv_label_set_text(visitor_data, date_str);
            }
            LV_LOG_USER("Selected history date: %s", date_str);
            
            // 只隐藏日历，不碰表头
            if(g_calendar && lv_obj_is_valid(g_calendar) && !lv_obj_has_flag(g_calendar, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_add_flag(g_calendar, LV_OBJ_FLAG_HIDDEN); 
            }
        }
    }
}

// 创建日历控件
static void create_calendar(lv_obj_t *parent)
{
    if(g_calendar != NULL) return; // 避免重复创建

    // 获取系统实时时间
    int year, month, day;
    get_current_date(&year, &month, &day);

    // 创建日历对象
    g_calendar = lv_calendar_create(parent);
    lv_obj_set_size(g_calendar, 456, 484);
    lv_obj_set_pos(g_calendar, 301, 255);
    lv_obj_add_event_cb(g_calendar, calendar_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(g_calendar, LV_OBJ_FLAG_HIDDEN);

    // 设置日历基础参数：使用实时时间
    lv_calendar_set_today_date(g_calendar, year, month, day);
    lv_calendar_set_showed_date(g_calendar, year, month);

    // 初始化当天日期的蓝色边框样式
    lv_calendar_date_t today_date = {year, month, day};
    update_calendar_selected_style(g_calendar, &today_date);

    // ===== 表头创建逻辑 =====
#if LV_USE_CALENDAR_HEADER_DROPDOWN
    lv_calendar_header_dropdown_create(g_calendar); // 仅保留这行，不额外修改
#elif LV_USE_CALENDAR_HEADER_ARROW
    lv_calendar_header_arrow_create(g_calendar);
#endif
}

// 日期选择器点击回调
static void date_picker_click_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_current_target(e);
    LV_UNUSED(obj); // 避免未使用变量警告

    if(g_calendar == NULL) {
        // 首次点击：创建日历并显示
        create_calendar(lv_obj_get_parent(date_picker));
        lv_obj_clear_flag(g_calendar, LV_OBJ_FLAG_HIDDEN);
    } else {
        // 非首次点击：切换日历显示/隐藏状态
        if(lv_obj_has_flag(g_calendar, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_clear_flag(g_calendar, LV_OBJ_FLAG_HIDDEN); // 显示日历
        } else {
            lv_obj_add_flag(g_calendar, LV_OBJ_FLAG_HIDDEN);   // 隐藏日历
        }
    }
}


//==================== 主界面创建 =====================
void ui_msg_center_create(lv_obj_t *homepage_scr)
{
    init_msg_center_styles();
    if(!homepage_scr || !lv_obj_is_valid(homepage_scr)) return;

    clean_calendar_obj();
    clear_all_visitor_records();
    clear_all_doorbell_records();
    clear_all_alarm_records();

    if(!msg_center_scr) {
        msg_center_scr = lv_obj_create(NULL);
    } else {
        lv_obj_clean(msg_center_scr);
    }

    lv_style_reset(&msg_center_grad_style);
    lv_style_set_bg_color(&msg_center_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&msg_center_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&msg_center_grad_style, LV_GRAD_DIR_VER);
    lv_obj_add_style(msg_center_scr, &msg_center_grad_style, 0);

    // 标题
    lv_obj_t *title = create_text_label(msg_center_scr,
        "message center", &lv_font_montserrat_36,
        lv_color_hex(0xFFFFFF), 83, 80, LV_OPA_100);

    // 模式容器
    lv_obj_t *mode_con = create_container(msg_center_scr,
        47,176,450,46,
        lv_color_hex(0x1F3150), LV_OPA_100, 31,
        lv_color_hex(0x1F3150), 0, LV_OPA_0);
    lv_obj_set_style_pad_all(mode_con, 0, 0);

    // 访客
    visitor_rec_con = create_custom_gradient_container(mode_con,
        0,0,150,46, 31, 0x006BDC,0x00BDBD,LV_GRAD_DIR_HOR, 0,255,LV_OPA_100);
    lv_obj_set_style_pad_all(visitor_rec_con,0,0);
    lv_obj_add_flag(visitor_rec_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(visitor_rec_con, rec_con_click_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *vt = create_text_label(visitor_rec_con,
        "vis record", &lv_font_montserrat_20,
        lv_color_hex(0xFFFFFF),27,10,LV_OPA_100);

    // 报警
    alarm_rec_con = lv_obj_create(mode_con);
    lv_obj_set_pos(alarm_rec_con,150,0);
    lv_obj_set_size(alarm_rec_con,150,46);
    lv_obj_set_style_radius(alarm_rec_con,31,0);
    lv_obj_set_style_bg_opa(alarm_rec_con,LV_OPA_100,0);
    lv_obj_set_style_bg_grad_dir(alarm_rec_con,LV_GRAD_DIR_NONE,0);
    lv_obj_set_style_bg_color(alarm_rec_con,lv_color_hex(0x1F3150),0);
    lv_obj_set_style_border_width(alarm_rec_con,0,0);
    lv_obj_clear_flag(alarm_rec_con, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(alarm_rec_con,0,0);
    lv_obj_add_flag(alarm_rec_con,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(alarm_rec_con,rec_con_click_cb,LV_EVENT_CLICKED,NULL);
    lv_obj_t *at = create_text_label(alarm_rec_con,
        "ala record",&lv_font_montserrat_20,
        lv_color_hex(0xFFFFFF),27,10,LV_OPA_100);

    // 门铃
    doorbell_rec_con = lv_obj_create(mode_con);
    lv_obj_set_pos(doorbell_rec_con,300,0);
    lv_obj_set_size(doorbell_rec_con,150,46);
    lv_obj_set_style_radius(doorbell_rec_con,31,0);
    lv_obj_set_style_bg_opa(doorbell_rec_con,LV_OPA_100,0);
    lv_obj_set_style_bg_grad_dir(doorbell_rec_con,LV_GRAD_DIR_NONE,0);
    lv_obj_set_style_bg_color(doorbell_rec_con,lv_color_hex(0x1F3150),0);
    lv_obj_set_style_border_width(doorbell_rec_con,0,0);
    lv_obj_clear_flag(doorbell_rec_con, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(doorbell_rec_con,0,0);
    lv_obj_add_flag(doorbell_rec_con,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(doorbell_rec_con,rec_con_click_cb,LV_EVENT_CLICKED,NULL);
    lv_obj_t *bt = create_text_label(doorbell_rec_con,
        "bell record",&lv_font_montserrat_20,
        lv_color_hex(0xFFFFFF),27,10,LV_OPA_100);

    g_selected_rec_con = visitor_rec_con;

    // 日期选择
    // date_picker = create_image_obj(msg_center_scr, "H:data.png",522,175);
    date_picker = create_container_circle(msg_center_scr, 522, 175, 30,
    true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_set_style_bg_opa(date_picker, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(date_picker,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(date_picker,LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(date_picker,date_picker_click_cb,LV_EVENT_CLICKED,NULL);

    // 当前日期
    int y,m,d;
    get_current_date(&y,&m,&d);
    char buf[32];
    snprintf(buf,sizeof(buf),"%04d-%02d-%02d",y,m,d);
    visitor_data = create_text_label(msg_center_scr,
        buf,&lv_font_montserrat_28,
        lv_color_hex(0xFFFFFF),47,248,LV_OPA_100);

    // 返回
    //lv_obj_t *back = create_image_obj(msg_center_scr,"H:back.png",52,123);
    lv_obj_t *back_btn = create_container_circle(msg_center_scr, 52, 90, 30,
    true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(back_btn,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn,LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn,back_btn_click_cb,LV_EVENT_CLICKED,homepage_scr);

    // 默认显示访客
    switch_record_type(visitor_rec_con);
    update_status_bar_parent(msg_center_scr);
    lv_scr_load(msg_center_scr);
}

void msg_center_btn_click_cb(lv_event_t *e)
{
    lv_obj_t *home = lv_event_get_user_data(e);
    if(home && lv_obj_is_valid(home)) {
        ui_msg_center_create(home);
    }
}
#endif