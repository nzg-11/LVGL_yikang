#include "lv_homepage.h"
#include "lv_msg_center.h"
#include "lv_monitor_video.h"
#include "lv_user_manage.h"
#include "lv_dev_info.h"
#include "lv_file_cache.h"
#include "lv_sys_settings.h"
#include <stdio.h>
#include <time.h>

#ifdef LV_DEMO_EQUES
#if LV_EQUES_VER
// ====================== 全局样式） ======================
// 1. 主页渐变样式
static lv_style_t scr_grad_style;
static bool scr_grad_style_inited = false;

//屏幕上创建状态栏
static void init_global_styles(void);
static void status_bar_update_cb(lv_timer_t *timer);
static void create_status_bar(void);

// ====================== 原有全局变量 ======================
static lv_obj_t *homepage_scr = NULL;    
  
uint8_t BATTERY_CAPACITY = 100;//电池容量
lv_obj_t *status_bar = NULL;   
static lv_obj_t *date_label = NULL;        
static lv_obj_t *time_label = NULL;        
static lv_obj_t *battery_used_con = NULL;  
static lv_obj_t *battery_label = NULL;     
static lv_timer_t *status_bar_timer = NULL;



// ====================== 屏幕加载回调 ======================
static void homepage_scr_load_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *scr = lv_event_get_target(e);
    if(scr != homepage_scr || !is_lv_obj_valid(scr)) return;

    lv_obj_clean(scr);

    // 确保全局样式已初始化
    init_global_styles();

    // 重置背景样式
    lv_style_reset(&scr_grad_style);
    lv_style_set_bg_color(&scr_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&scr_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&scr_grad_style, LV_GRAD_DIR_VER);
    lv_style_set_bg_main_stop(&scr_grad_style, 0);
    lv_style_set_bg_grad_stop(&scr_grad_style, 255);
    lv_obj_add_style(scr, &scr_grad_style, LV_STATE_DEFAULT);

    // 重建状态栏
    // destroy_status_bar();
    // create_status_bar();
    update_status_bar_parent(scr);

    // 重建所有控件
    lv_obj_t *weather_img = create_image_obj(scr, "H:weather.png", 276, 158);
    lv_obj_t *temp_lable = create_text_label(scr, "25.0 C", &lv_font_montserrat_48, lv_color_hex(0xFFFFFF), 311, 367, LV_OPA_100);

    lv_obj_t *unlocking_con = create_container(scr, 78, 518, 644, 118, lv_color_hex(0x2E4B7D), LV_OPA_100, 16, lv_color_hex(0x2E4B7D), 0, LV_OPA_0);
    lv_obj_t *unlocking_lable = create_text_label(scr, "unlocking", &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 199, 551, LV_OPA_100);
    lv_obj_t *locking_lable = create_text_label(scr, "locking", &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 533, 551, LV_OPA_100);
    lv_obj_add_flag(unlocking_con, LV_OBJ_FLAG_CLICKABLE);      

    // 消息中心
    lv_obj_t *msg_center_con = create_custom_gradient_container
    (scr, 49, 699, 220, 247, 16, 0x34568F, 0x1F3150, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_add_flag(msg_center_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(msg_center_con, LV_OPA_70, LV_STATE_PRESSED);
    lv_obj_t *msg_center_lable = create_text_label(scr, "msg_center", &lv_font_montserrat_20, lv_color_hex(0xFFFFFF), 87, 849, LV_OPA_100);
    lv_obj_t *msg_center_img = create_image_obj(scr, "H:msg_center.png", 121, 754);
    lv_obj_add_event_cb(msg_center_con, msg_center_btn_click_cb, LV_EVENT_CLICKED, scr);

    // 监控视频
    lv_obj_t *monitor_video_con = create_custom_gradient_container(
    scr,
    290, 699, 220, 247,       
    16, 0x34568F, 0x1F3150, LV_GRAD_DIR_VER,
    0, 225, LV_OPA_100);
    lv_obj_t *monitor_video_lable = create_text_label(scr, "monitor_video", &lv_font_montserrat_20, lv_color_hex(0xFFFFFF), 332, 849, LV_OPA_100);
    lv_obj_t *monitor_video_img = create_image_obj(scr, "H:monitor_video.png", 351, 754);
    lv_obj_add_flag(monitor_video_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(monitor_video_con, LV_OPA_70, LV_STATE_PRESSED);
    lv_obj_add_event_cb(monitor_video_con, monitor_video_btn_click_cb, LV_EVENT_CLICKED, scr);

    // 用户管理
    lv_obj_t *user_manage_con = create_custom_gradient_container(
    scr,
    533, 699, 220, 247,       
    16, 0x34568F, 0x1F3150, LV_GRAD_DIR_VER,
    0, 225, LV_OPA_100);
    lv_obj_t *user_manage_lable = create_text_label(scr, "user_manage", &lv_font_montserrat_20, lv_color_hex(0xFFFFFF), 571, 849, LV_OPA_100);
    lv_obj_t *user_manage_img = create_image_obj(scr, "H:user_manage.png", 602, 756);
    lv_obj_add_flag(user_manage_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(user_manage_con, LV_OPA_70, LV_STATE_PRESSED);
    lv_obj_add_event_cb(user_manage_con, user_manage_btn_click_cb, LV_EVENT_CLICKED, scr);

    // 设备信息
    lv_obj_t *dev_info_con = create_custom_gradient_container(
    scr,
    49, 972, 220, 247,       
    16, 0x34568F, 0x1F3150, LV_GRAD_DIR_VER,
    0, 225, LV_OPA_100);
    lv_obj_t *dev_info_lable = create_text_label(scr, "dev_info", &lv_font_montserrat_20, lv_color_hex(0xFFFFFF), 85, 1119, LV_OPA_100);
    lv_obj_t *dev_info_img = create_image_obj(scr, "H:dev_info.png", 109, 1028);
    lv_obj_add_flag(dev_info_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(dev_info_con, LV_OPA_70, LV_STATE_PRESSED);
    lv_obj_add_event_cb(dev_info_con, dev_info_btn_click_cb, LV_EVENT_CLICKED, scr);

    // 文件缓存
    lv_obj_t *file_cache_con = create_custom_gradient_container(
    scr,
    290, 972, 220, 247,       
    16, 0x34568F, 0x1F3150, LV_GRAD_DIR_VER,
    0, 225, LV_OPA_100);
    lv_obj_t *file_cache_lable = create_text_label(scr, "file_cache", &lv_font_montserrat_20, lv_color_hex(0xFFFFFF), 332, 1119, LV_OPA_100);
    lv_obj_t *file_cache_img = create_image_obj(scr, "H:file_cache.png", 356, 1028);
    lv_obj_add_flag(file_cache_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(file_cache_con, LV_OPA_70, LV_STATE_PRESSED);
    lv_obj_add_event_cb(file_cache_con, file_cache_btn_click_cb, LV_EVENT_CLICKED, scr);

    // 系统设置
    lv_obj_t *sys_settings_con = create_custom_gradient_container(
    scr,
    533, 972, 220, 247,       
    16, 0x34568F, 0x1F3150, LV_GRAD_DIR_VER,
    0, 225, LV_OPA_100);
    lv_obj_t *sys_settings_lable = create_text_label(scr, "sys_settings", &lv_font_montserrat_20, lv_color_hex(0xFFFFFF), 571, 1119, LV_OPA_100);
    lv_obj_t *sys_settings_img = create_image_obj(scr, "H:sys_settings.png", 602, 1028);
    lv_obj_add_flag(sys_settings_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(sys_settings_con, LV_OPA_70, LV_STATE_PRESSED);
    lv_obj_add_event_cb(sys_settings_con, sys_settings_btn_click_cb, LV_EVENT_CLICKED, scr);
}

void lv_homepage_create(void)
{
    init_global_styles();

    // 创建/复用主页屏幕
    if(homepage_scr == NULL) {
        homepage_scr = lv_obj_create(NULL);
        lv_obj_add_flag(homepage_scr, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_event_cb(homepage_scr, homepage_scr_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    } else {
        lv_obj_clean(homepage_scr);
    }

    // 重置样式
    // lv_style_reset(&scr_grad_style);
    // lv_style_set_bg_color(&scr_grad_style, lv_color_hex(0x010715));
    // lv_style_set_bg_grad_color(&scr_grad_style, lv_color_hex(0x0E1D37));
    // lv_style_set_bg_grad_dir(&scr_grad_style, LV_GRAD_DIR_VER);
    // lv_style_set_bg_main_stop(&scr_grad_style, 0);
    // lv_style_set_bg_grad_stop(&scr_grad_style, 255);
    // lv_obj_add_style(homepage_scr, &scr_grad_style, LV_STATE_DEFAULT);

    // 创建状态栏
    // destroy_status_bar();
    // create_status_bar();
    //update_status_bar_parent(homepage_scr);

    // 首次创建控件
    // lv_obj_t *weather_img = create_image_obj(homepage_scr, "H:weather.png", 276, 158);
    // lv_obj_t *temp_lable = create_text_label(homepage_scr, "25.0 C", &lv_font_montserrat_48, lv_color_hex(0xFFFFFF), 311, 367, LV_OPA_100);

    // lv_obj_t *unlocking_con = create_container(homepage_scr, 78, 518, 644, 118, lv_color_hex(0x2E4B7D), LV_OPA_100, 16, lv_color_hex(0x2E4B7D), 0, LV_OPA_0);
    // lv_obj_t *unlocking_lable = create_text_label(homepage_scr, "unlocking", &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 199, 551, LV_OPA_100);
    // lv_obj_t *locking_lable = create_text_label(homepage_scr, "locking", &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 533, 551, LV_OPA_100);
    // lv_obj_add_flag(unlocking_con, LV_OBJ_FLAG_CLICKABLE);      
}

void lv_homepage(void)
{
    lv_homepage_create();
    lv_scr_load(homepage_scr);
}

// ====================== 工具函数 ======================
/**
 * @brief 检查LVGL对象是否有效（封装LVGL内置校验，避免野指针操作）
 * @param obj 待检查的LVGL对象指针
 * @return true-对象有效（非空且未被销毁），false-对象无效/空指针
 */
bool is_lv_obj_valid(lv_obj_t *obj)
{
    // 第一步：空指针快速校验
    if(obj == NULL) return false;
    // 第二步：调用LVGL 8.3内置的对象有效性检查接口
    return lv_obj_is_valid(obj);
}

/**
 * @brief 安全销毁状态栏及相关资源（定时器+所有子控件）
 * @note 销毁后会将所有相关指针置空，避免野指针
 */
void destroy_status_bar(void)
{
    // 1. 停止并删除状态栏刷新定时器（避免定时器回调访问已销毁对象）
    if(status_bar_timer != NULL) {
        lv_timer_del(status_bar_timer);
        status_bar_timer = NULL; // 置空，防止重复删除
    }
    
    // 2. 安全销毁状态栏容器（先校验有效性，再销毁）
    if(is_lv_obj_valid(status_bar)) {
        lv_obj_del(status_bar);
    }
    
    // 3. 所有状态栏相关子控件指针置空（关键：避免野指针访问）
    status_bar = NULL;        // 状态栏根容器
    date_label = NULL;        // 日期标签
    time_label = NULL;        // 时间标签
    battery_used_con = NULL;  // 电池电量进度条容器
    battery_label = NULL;     // 电池百分比标签
}



/**
 * @brief 创建状态栏（包含日期、时间、WiFi图标、电池电量显示）
 * @note 1. 创建前会先销毁旧状态栏，保证每次创建都是全新的
 *       2. 状态栏会置顶显示，宽度适配当前屏幕，高度固定80px
 */
static void create_status_bar(void)
{
    // 前置操作：销毁旧状态栏，避免重复创建导致内存泄漏
    if(status_bar != NULL) destroy_status_bar();

    // 1. 获取当前活动屏幕（作为状态栏的父容器）
    lv_obj_t *current_scr = lv_scr_act();
    // 安全校验：当前屏幕无效则直接返回
    if(!is_lv_obj_valid(current_scr)) return;

    // 2. 创建状态栏根容器
    status_bar = lv_obj_create(current_scr);
    lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_SCROLLABLE); // 禁止状态栏滚动
    lv_obj_set_size(status_bar, lv_obj_get_width(current_scr), 80); // 宽度适配屏幕，高度80px
    lv_obj_set_pos(status_bar, 0, 0); // 状态栏固定在屏幕顶部
    lv_obj_set_style_bg_opa(status_bar, LV_OPA_0, LV_STATE_DEFAULT); // 背景透明
    lv_obj_set_style_border_opa(status_bar, LV_OPA_0, LV_STATE_DEFAULT); // 无边框
    lv_obj_move_foreground(status_bar); // 状态栏置顶显示（避免被其他控件遮挡）

    // 3. 创建状态栏子控件：日期标签
    date_label = create_text_label(
        status_bar,                // 父容器
        "2023-01-01",              // 默认文本
        &lv_font_montserrat_40,    // 字体（40号蒙哥马利字体）
        lv_color_hex(0xFFFFFF),    // 白色文本
        47, 13,                    // 坐标(x:47, y:13)
        LV_OPA_100                 // 完全不透明
    );    
    
    // 4. 创建状态栏子控件：时间标签
    time_label = create_text_label(
        status_bar, 
        "12:00", 
        &lv_font_montserrat_40, 
        lv_color_hex(0xFFFFFF), 
        291, 13,  // 坐标(x:291, y:13)
        LV_OPA_100
    );
    
    // 5. 创建状态栏子控件：WiFi图标
    lv_obj_t *wifi_img = create_image_obj(
        status_bar, 
        "H:wifi.png",  // 图片路径
        647, 23        // 坐标(x:647, y:23)
    );
    
    // 6. 创建状态栏子控件：电池外框容器
    lv_obj_t *battery_con = create_container(
        status_bar,                // 父容器
        691, 21,                   // 坐标(x:691, y:21)
        65, 30,                    // 尺寸(w:65, h:30)
        lv_color_hex(0x2E4B7D),    // 背景色（深蓝色）
        LV_OPA_0,                  // 背景透明
        0,                         // 圆角半径
        lv_color_hex(0xFFFFFF),    // 边框色（白色）
        3,                         // 边框宽度
        LV_OPA_100                 // 边框不透明
    );
    
    // 7. 创建状态栏子控件：电池电量进度条（初始宽度0）
    battery_used_con = create_container(
        status_bar,                // 父容器
        694, 24,                   // 坐标(x:694, y:24)
        0, 24,                     // 尺寸(w:0, h:24)
        lv_color_hex(0x00ac11),    // 进度条颜色（绿色）
        LV_OPA_100,                // 完全不透明
        0,                         // 圆角半径
        lv_color_hex(0xFFFFFF),    // 边框色（白色）
        3,                         // 边框宽度
        LV_OPA_0                   // 边框透明
    );
    
    // 8. 创建状态栏子控件：电池百分比标签
    char battery_text[4] = {0};   // 缓冲区：存储0-100的百分比字符串
    snprintf(battery_text, sizeof(battery_text), "%d", BATTERY_CAPACITY); // 格式化电量值
    battery_label = create_text_label(
        status_bar, 
        battery_text, 
        &lv_font_montserrat_24,    // 字体（24号蒙哥马利字体）
        lv_color_hex(0xFFFFFF),    // 白色文本
        0, 0,                      // 初始坐标（后续居中对齐）
        LV_OPA_100
    );
    lv_obj_align_to(battery_label, battery_con, LV_ALIGN_CENTER, 0, 0); // 相对于电池外框居中
    lv_obj_set_style_bg_opa(battery_label, LV_OPA_0, LV_STATE_DEFAULT); // 标签背景透明

    // 9. 创建状态栏刷新定时器（每秒刷新一次，仅创建一次）
    if(status_bar_timer == NULL) {
        status_bar_timer = lv_timer_create(status_bar_update_cb, 1000, NULL); // 1000ms = 1秒
    }
    
    // 立即执行一次刷新（避免初始显示默认值）
    status_bar_update_cb(NULL);
}

/**
 * @brief 页面切换时，更新状态栏的父容器到新屏幕
 * @param new_scr 新的目标屏幕对象
 * @note 核心功能：保证状态栏在页面切换后能跟随新屏幕显示
 */
void update_status_bar_parent(lv_obj_t *new_scr)
{
    // 第一步：校验新屏幕对象有效性（安全防护）
    if(!is_lv_obj_valid(new_scr)) {
        LV_LOG_WARN("update_status_bar_parent: new_scr is invalid!"); // 打印警告日志
        return;
    }

    // 第二步：状态栏无效则重建，再挂载到新屏幕
    if(!is_lv_obj_valid(status_bar)) {
        create_status_bar(); // 重建状态栏
        
        // 重建成功后，设置父容器为新屏幕
        if(is_lv_obj_valid(status_bar)) {
            lv_obj_set_parent(status_bar, new_scr);
        }
    } else {
        // 第三步：状态栏有效，直接切换父容器到新屏幕
        lv_obj_set_parent(status_bar, new_scr);
    }
    
    // 第四步：确保状态栏置顶且位置正确（防止切换后被遮挡/位置偏移）
    if(is_lv_obj_valid(status_bar)) {
        lv_obj_move_foreground(status_bar); // 置顶
        lv_obj_set_pos(status_bar, 0, 0);   // 回到屏幕顶部
    }
}

/**
 * @brief 状态栏刷新回调函数（每秒执行一次）
 * @param timer LVGL定时器对象（回调时传入，NULL表示手动调用）
 * @note 刷新内容：日期、时间、电池电量进度条+百分比
 */
static void status_bar_update_cb(lv_timer_t *timer)
{
    // 安全校验：所有关键控件必须有效，否则跳过本次刷新
    if(!is_lv_obj_valid(date_label) || !is_lv_obj_valid(time_label) || 
       !is_lv_obj_valid(battery_used_con) || !is_lv_obj_valid(battery_label)) {
        return;
    }

    // 1. 获取当前系统时间
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    // 时间获取失败则返回
    if(tm_now == NULL) return;

    // 2. 更新日期标签（格式：YYYY-MM-DD）
    char date_buf[11] = {0}; // 缓冲区：存储10位日期字符串（含结束符）
    snprintf(date_buf, sizeof(date_buf), "%04d-%02d-%02d", 
             tm_now->tm_year + 1900, // 年份：tm_year是从1900开始的偏移量
             tm_now->tm_mon + 1,     // 月份：tm_mon是0-11，需+1
             tm_now->tm_mday);       // 日期：1-31
    lv_label_set_text(date_label, date_buf); // 设置日期文本

    // 3. 更新时间标签（格式：HH:MM）
    char time_buf[6] = {0}; // 缓冲区：存储5位时间字符串（含结束符）
    snprintf(time_buf, sizeof(time_buf), "%02d:%02d", 
             tm_now->tm_hour,   // 小时：0-23
             tm_now->tm_min);   // 分钟：0-59
    lv_label_set_text(time_label, time_buf); // 设置时间文本

    // 4. 更新电池电量显示
    int battery_percent = BATTERY_CAPACITY;
    // 边界值保护：确保电量在0-100之间
    if (battery_percent < 0) battery_percent = 0;
    if (battery_percent > 100) battery_percent = 100;
    
    // 计算电量进度条宽度：总宽度59px，按百分比换算
    uint16_t battery_width = (uint16_t)((battery_percent / 100.0) * 59);
    lv_obj_set_width(battery_used_con, battery_width); // 设置进度条宽度
    
    // 更新电池百分比标签文本
    char battery_text[4] = {0};
    snprintf(battery_text, sizeof(battery_text), "%d", battery_percent);
    lv_label_set_text(battery_label, battery_text);
}

// ====================== 全局样式初始化函数 ======================
static void init_global_styles(void)
{
    // 1. 初始化主页渐变样式（只执行一次）
    if(!scr_grad_style_inited) {
        lv_style_init(&scr_grad_style);
        scr_grad_style_inited = true;
    }
}
#else
// ====================== 全局样式） ======================
// 1. 主页渐变样式
static lv_style_t scr_grad_style;
static bool scr_grad_style_inited = false;

//屏幕上创建状态栏
static void init_global_styles(void);
static void status_bar_update_cb(lv_timer_t *timer);
static void create_status_bar_ex(bool is_homepage); // 正确声明
// ====================== 原有全局变量 ======================
lv_obj_t *homepage_scr = NULL;    
  
uint8_t BATTERY_CAPACITY = 100;//电池容量
lv_obj_t *status_bar = NULL;   
static lv_obj_t *date_label = NULL;        
static lv_obj_t *time_label = NULL;        
static lv_obj_t *battery_used_con = NULL;  
static lv_obj_t *battery_label = NULL;     
static lv_timer_t *status_bar_timer = NULL;
static bool status_bar_is_homepage = false;

// 安全销毁主页 释放所有资源
void destroy_homepage(void)
{
    // 1. 销毁状态栏（定时器+所有控件）
    destroy_status_bar();
    
    // 2. 销毁主页根容器
    if(is_lv_obj_valid(homepage_scr)) {
        lv_obj_del(homepage_scr);
        homepage_scr = NULL; // 关键！置空全局指针
    }
}
// ====================== 屏幕加载回调 ======================
static void homepage_scr_load_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *scr = lv_event_get_target(e);

    // if(is_lv_obj_valid(homepage_scr)) {
    //     lv_obj_del(homepage_scr);
    //     homepage_scr = NULL;
    // }
    // homepage_scr = lv_obj_create(NULL);
    // if(!is_lv_obj_valid(scr) || scr != homepage_scr) return;

    // lv_obj_clean(scr);

    // 确保全局样式已初始化
    init_global_styles();

    // 重置背景样式
    lv_style_reset(&scr_grad_style);
    lv_style_set_bg_color(&scr_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&scr_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&scr_grad_style, LV_GRAD_DIR_VER);
    lv_style_set_bg_main_stop(&scr_grad_style, 0);
    lv_style_set_bg_grad_stop(&scr_grad_style, 255);
    lv_obj_add_style(scr, &scr_grad_style, LV_STATE_DEFAULT);

    // 重建状态栏
    update_status_bar_parent(scr);
    // 重建所有控件
    //lv_obj_t *weather_img = create_image_obj(scr, "H:weather.png", 276, 158);
    //云朵替代
    create_container_circle(scr, 217, 163, 89,
    true, lv_color_hex(0xFFCE50), lv_color_hex(0x808080), 0, LV_OPA_0);
    create_container_circle(scr, 181, 192, 107,
    true, lv_color_hex(0xFFFFFF), lv_color_hex(0x808080), 0, LV_OPA_0);
    create_text_label(scr, "25.0 C", &lv_font_montserrat_48, lv_color_hex(0xFFFFFF), 212, 327, LV_OPA_100);

    //开关锁容器
    // lv_obj_t *unlocking_con = create_container
    // (scr, 48, 470, 239, 82, lv_color_hex(0x00BDBD), LV_OPA_100, 0, lv_color_hex(0x2E4B7D), 0, LV_OPA_0);
    lv_obj_t *unlocking_con = create_custom_gradient_container
    (scr, 48, 470, 239, 82, 0, 0x006BDC, 0x00BDBD, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_t *locking_con = create_container
    (scr, 287, 470, 239, 82, lv_color_hex(0x2E4B7D), LV_OPA_100, 0, lv_color_hex(0x2E4B7D), 0, LV_OPA_0);
    //开关锁文本
    create_text_label(scr, "unlocking", &lv_font_montserrat_28, lv_color_hex(0xFFFFFF), 130, 495, LV_OPA_100);
    create_text_label(scr, "locking", &lv_font_montserrat_28, lv_color_hex(0xFFFFFF), 367, 495, LV_OPA_100);
    lv_obj_set_style_bg_opa(unlocking_con, LV_OPA_70, LV_STATE_PRESSED);
    lv_obj_add_flag(unlocking_con, LV_OBJ_FLAG_CLICKABLE);      
    lv_obj_set_style_bg_opa(locking_con, LV_OPA_70, LV_STATE_PRESSED);
    lv_obj_add_flag(locking_con, LV_OBJ_FLAG_CLICKABLE);

    // 消息中心
    lv_obj_t *msg_center_con = create_custom_gradient_container
    (scr, 573, 24, 205, 174, 16, 0x34568F, 0x1F3150, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_set_style_pad_all(msg_center_con, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(msg_center_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(msg_center_con, LV_OPA_70, LV_STATE_PRESSED);
    // 消息中心文本
    lv_obj_t *msg_center_lable = create_text_label
    (msg_center_con, "msg_center", &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 600, 111, LV_OPA_100);
    lv_obj_align_to(msg_center_lable, msg_center_con, LV_ALIGN_TOP_MID, 0, 111);
    // 消息中心图标
    lv_obj_t *msg_cen_img = create_container_circle(scr, 643, 53, 65,
    true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_set_style_bg_opa(msg_cen_img, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_event_cb(msg_center_con, msg_center_btn_click_cb, LV_EVENT_CLICKED, scr);

    // 监控视频
    lv_obj_t *monitor_video_con = create_custom_gradient_container(
    scr, 795, 24, 205, 174, 16, 0x34568F, 0x1F3150, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_set_style_pad_all(monitor_video_con, 0, LV_STATE_DEFAULT);
    // 监控视频文本
    lv_obj_t *monitor_video_lable = create_text_label
    (monitor_video_con, "monitor_video", &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 810, 111, LV_OPA_100);
    lv_obj_align_to(monitor_video_lable, monitor_video_con, LV_ALIGN_TOP_MID, 0, 111);
    //lv_obj_t *monitor_video_img = create_image_obj(scr, "H:monitor_video.png", 351, 754);
    // 监控视频图标
    lv_obj_t *monitor_video_img = create_container_circle(scr, 865, 50, 65,
    true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_set_style_bg_opa(monitor_video_img, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(monitor_video_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(monitor_video_con, LV_OPA_70, LV_STATE_PRESSED);
    lv_obj_add_event_cb(monitor_video_con, monitor_video_btn_click_cb, LV_EVENT_CLICKED, scr);

    // 用户管理
    lv_obj_t *user_manage_con = create_custom_gradient_container(
    scr, 573, 214, 205, 174, 16, 0x34568F, 0x1F3150, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_set_style_pad_all(user_manage_con, 0, LV_STATE_DEFAULT);
    // 用户管理文本
    lv_obj_t *user_manage_lable = create_text_label
    (user_manage_con, "user_manage", &lv_font_montserrat_20, lv_color_hex(0xFFFFFF), 600, 111, LV_OPA_100);
    lv_obj_align_to(user_manage_lable, user_manage_con, LV_ALIGN_TOP_MID, 0, 111);
    //lv_obj_t *user_manage_img = create_image_obj(scr, "H:user_manage.png", 602, 756);
    // 用户管理图标
    lv_obj_t *user_manage_img = create_container_circle(scr, 643, 244, 65,
    true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_set_style_bg_opa(user_manage_img, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(user_manage_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(user_manage_con, LV_OPA_70, LV_STATE_PRESSED);
    lv_obj_add_event_cb(user_manage_con, user_manage_btn_click_cb, LV_EVENT_CLICKED, scr);

    // 设备信息
    lv_obj_t *dev_info_con = create_custom_gradient_container(
    scr, 795, 214, 205, 174, 16, 0x34568F, 0x1F3150, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_set_style_pad_all(dev_info_con, 0, LV_STATE_DEFAULT);
    // 设备信息文本
    lv_obj_t *dev_info_lable = create_text_label
    (dev_info_con, "dev_info", &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 810, 111, LV_OPA_100);
    lv_obj_align_to(dev_info_lable, dev_info_con, LV_ALIGN_TOP_MID, 0, 111);
    //lv_obj_t *dev_info_img = create_image_obj(scr, "H:dev_info.png", 109, 1028);
    // 设备信息图标
    lv_obj_t *dev_info_img = create_container_circle(scr, 865, 240, 65,
    true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_set_style_bg_opa(dev_info_img, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(dev_info_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(dev_info_con, LV_OPA_70, LV_STATE_PRESSED);
    lv_obj_add_event_cb(dev_info_con, dev_info_btn_click_cb, LV_EVENT_CLICKED, scr);

    // 文件缓存
    lv_obj_t *file_cache_con = create_custom_gradient_container
    (scr, 573, 404, 205, 174, 16, 0x34568F, 0x1F3150, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_set_style_pad_all(file_cache_con, 0, LV_STATE_DEFAULT);
    // 文件缓存文本
    lv_obj_t *file_cache_lable = create_text_label
    (file_cache_con, "file_cache", &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 600, 111, LV_OPA_100);
    lv_obj_align_to(file_cache_lable, file_cache_con, LV_ALIGN_TOP_MID, 0, 111);
    //lv_obj_t *file_cache_img = create_image_obj(scr, "H:file_cache.png", 356, 1028);
    // 文件缓存图标
    lv_obj_t *file_cache_img = create_container_circle(scr, 643, 439, 65,
    true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_set_style_bg_opa(file_cache_img, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(file_cache_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(file_cache_con, LV_OPA_70, LV_STATE_PRESSED);
    lv_obj_add_event_cb(file_cache_con, file_cache_btn_click_cb, LV_EVENT_CLICKED, scr);

    // 系统设置
    lv_obj_t *sys_settings_con = create_custom_gradient_container(
    scr, 795, 404, 205, 174, 16, 0x34568F, 0x1F3150, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_set_style_pad_all(sys_settings_con, 0, LV_STATE_DEFAULT);
    // 系统设置文本
    lv_obj_t *sys_settings_lable = create_text_label
    (sys_settings_con, "sys_settings", &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 810, 111, LV_OPA_100);
    lv_obj_align_to(sys_settings_lable, sys_settings_con, LV_ALIGN_TOP_MID, 0, 111);
    //lv_obj_t *sys_settings_img = create_image_obj(scr, "H:sys_settings.png", 602, 1028);
    // 系统设置图标
    lv_obj_t *sys_settings_img = create_container_circle(scr, 865, 439, 65,
    true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_set_style_bg_opa(sys_settings_img, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(sys_settings_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(sys_settings_con, LV_OPA_70, LV_STATE_PRESSED);
    lv_obj_add_event_cb(sys_settings_con, sys_settings_btn_click_cb, LV_EVENT_CLICKED, scr);
}

void lv_homepage_create(void)
{
    init_global_styles();

    // 创建/复用主页屏幕
    if(homepage_scr == NULL) {
        homepage_scr = lv_obj_create(NULL);
        //lv_obj_add_flag(homepage_scr, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_event_cb(homepage_scr, homepage_scr_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    } else {
        lv_obj_clean(homepage_scr);
    }

}

void lv_homepage(void)
{
    lv_homepage_create();
    lv_scr_load(homepage_scr);
}

// ====================== 工具函数 ======================
/**
 * @brief 检查LVGL对象是否有效（封装LVGL内置校验，避免野指针操作）
 * @param obj 待检查的LVGL对象指针
 * @return true-对象有效（非空且未被销毁），false-对象无效/空指针
 */
bool is_lv_obj_valid(lv_obj_t *obj)
{
    // 第一步：空指针快速校验
    if(obj == NULL) return false;
    // 第二步：调用LVGL 8.3内置的对象有效性检查接口
    return lv_obj_is_valid(obj);
}

/**
 * @brief 安全销毁状态栏及相关资源（定时器+所有子控件）
 * @note 销毁后会将所有相关指针置空，避免野指针
 */
void destroy_status_bar(void)
{
    // 1. 停止并删除状态栏刷新定时器（避免定时器回调访问已销毁对象）
    if(status_bar_timer != NULL) {
        lv_timer_del(status_bar_timer);
        status_bar_timer = NULL; // 置空，防止重复删除
    }
    
    // 2. 安全销毁状态栏容器（先校验有效性，再销毁）
    if(is_lv_obj_valid(status_bar)) {
        lv_obj_del(status_bar);
    }
    
    // 3. 所有状态栏相关子控件指针置空（关键：避免野指针访问）
    status_bar = NULL;        // 状态栏根容器
    date_label = NULL;        // 日期标签
    time_label = NULL;        // 时间标签
    battery_used_con = NULL;  // 电池电量进度条容器
    battery_label = NULL;     // 电池百分比标签
}


//主页单独状态栏创建
// 合并后的唯一状态栏创建函数
// 标志位：is_homepage = true 用首页坐标，false 用其他页坐标
// 合并后的唯一状态栏创建函数
// 标志位：is_homepage = true 用首页坐标，false 用其他页坐标
static void create_status_bar_ex(bool is_homepage)
{
    // 保存当前模式
    status_bar_is_homepage = is_homepage;

    // 销毁旧的
    if(status_bar != NULL)
        destroy_status_bar();

    //lv_obj_t *current_scr = lv_scr_act();
    
    // if(!is_lv_obj_valid(current_scr))
    //     return;

    // 创建状态栏根容器
    status_bar = lv_obj_create(lv_scr_act()); // 临时父，后面会替换
    lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(status_bar, lv_obj_get_width(lv_scr_act()), 90);
    lv_obj_set_style_bg_opa(status_bar, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(status_bar, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_move_foreground(status_bar);

    lv_obj_t *battery_con = NULL; // 把变量提到外面，修复作用域问题

    // ====================== 自动切换布局 ======================
    if(is_homepage)
    {
        // ====================== 首页：靠左布局 ======================
        lv_obj_align(status_bar, LV_ALIGN_TOP_LEFT, 0, 0);

        // 日期
        date_label = create_text_label(
            status_bar, "2023-01-01",
            &lv_font_montserrat_36, lv_color_hex(0xFFFFFF),
            48, 24, LV_OPA_100);

        // 时间
        time_label = create_text_label(
            status_bar, "12:00",
            &lv_font_montserrat_36, lv_color_hex(0xFFFFFF),
            280, 25, LV_OPA_100);

        // WiFi
        create_container_circle(
            status_bar, 400, 33, 28,
            true, lv_color_hex(0xFFFFFF), lv_color_hex(0x808080), 0, LV_OPA_0);

        // 电池外框
        battery_con = create_container(
            status_bar, 444, 31, 65, 30,
            lv_color_hex(0x2E4B7D), LV_OPA_0, 0,
            lv_color_hex(0xFFFFFF), 3, LV_OPA_100);

        // 电池进度
        battery_used_con = create_container(
            status_bar, 447, 34, 0, 24,
            lv_color_hex(0x00ac11), LV_OPA_100, 0,
            lv_color_hex(0xFFFFFF), 3, LV_OPA_0);
    }
    else
    {
        // ====================== 其他页面：靠右布局 ======================
        lv_obj_align(status_bar, LV_ALIGN_TOP_RIGHT, 0, 0);

        // 日期
        date_label = create_text_label(
            status_bar, "2023-01-01",
            &lv_font_montserrat_36, lv_color_hex(0xFFFFFF),
            514, 24, LV_OPA_100);

        // 时间
        time_label = create_text_label(
            status_bar, "12:00",
            &lv_font_montserrat_36, lv_color_hex(0xFFFFFF),
            744, 25, LV_OPA_100);

        // WiFi
        create_container_circle(
            status_bar, 866, 33, 28,
            true, lv_color_hex(0xFFFFFF), lv_color_hex(0x808080), 0, LV_OPA_0);

        // 电池外框
        battery_con = create_container(
            status_bar, 910, 31, 65, 30,
            lv_color_hex(0x2E4B7D), LV_OPA_0, 0,
            lv_color_hex(0xFFFFFF), 3, LV_OPA_100);

        // 电池进度
        battery_used_con = create_container(
            status_bar, 913, 34, 0, 24,
            lv_color_hex(0x00ac11), LV_OPA_100, 0,
            lv_color_hex(0xFFFFFF), 3, LV_OPA_0);
    }

    // 电池百分比（共用）
    char battery_text[4] = {0};
    snprintf(battery_text, sizeof(battery_text), "%d", BATTERY_CAPACITY);
    battery_label = create_text_label(
        status_bar, battery_text,
        &lv_font_montserrat_24, lv_color_hex(0xFFFFFF),
        0, 0, LV_OPA_100);
    lv_obj_align_to(battery_label, battery_con, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(battery_label, LV_OPA_0, LV_STATE_DEFAULT);

    // 定时器
    if(status_bar_timer == NULL)
        status_bar_timer = lv_timer_create(status_bar_update_cb, 1000, NULL);

    status_bar_update_cb(NULL);
}

/**
 * @brief 页面切换时，更新状态栏的父容器到新屏幕
 * @param new_scr 新的目标屏幕对象
 * @note 核心功能：保证状态栏在页面切换后能跟随新屏幕显示
 */
void update_status_bar_parent(lv_obj_t *new_scr)
{
    if(!is_lv_obj_valid(new_scr)) return;

    bool is_homepage = (new_scr == homepage_scr);

    // 先销毁旧状态栏
    destroy_status_bar();

    // 创建新状态栏
    create_status_bar_ex(is_homepage);

    // 挂载到新屏幕
    if(is_lv_obj_valid(status_bar))
    {
        lv_obj_set_parent(status_bar, new_scr);
        lv_obj_move_foreground(status_bar);
    }
}

/**
 * @brief 状态栏刷新回调函数（每秒执行一次）
 * @param timer LVGL定时器对象（回调时传入，NULL表示手动调用）
 * @note 刷新内容：日期、时间、电池电量进度条+百分比
 */
static void status_bar_update_cb(lv_timer_t *timer)
{
    // 安全校验：所有关键控件必须有效，否则跳过本次刷新
    if(!is_lv_obj_valid(date_label) || !is_lv_obj_valid(time_label) || 
       !is_lv_obj_valid(battery_used_con) || !is_lv_obj_valid(battery_label)) {
        return;
    }

    // 1. 获取当前系统时间
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    // 时间获取失败则返回
    if(tm_now == NULL) return;

    // 2. 更新日期标签（格式：YYYY-MM-DD）
    char date_buf[11] = {0}; // 缓冲区：存储10位日期字符串（含结束符）
    snprintf(date_buf, sizeof(date_buf), "%04d-%02d-%02d", 
             tm_now->tm_year + 1900, // 年份：tm_year是从1900开始的偏移量
             tm_now->tm_mon + 1,     // 月份：tm_mon是0-11，需+1
             tm_now->tm_mday);       // 日期：1-31
    lv_label_set_text(date_label, date_buf); // 设置日期文本

    // 3. 更新时间标签（格式：HH:MM）
    char time_buf[6] = {0}; // 缓冲区：存储5位时间字符串（含结束符）
    snprintf(time_buf, sizeof(time_buf), "%02d:%02d", 
             tm_now->tm_hour,   // 小时：0-23
             tm_now->tm_min);   // 分钟：0-59
    lv_label_set_text(time_label, time_buf); // 设置时间文本

    // 4. 更新电池电量显示
    int battery_percent = BATTERY_CAPACITY;
    // 边界值保护：确保电量在0-100之间
    if (battery_percent < 0) battery_percent = 0;
    if (battery_percent > 100) battery_percent = 100;
    
    // 计算电量进度条宽度：总宽度59px，按百分比换算
    uint16_t battery_width = (uint16_t)((battery_percent / 100.0) * 59);
    lv_obj_set_width(battery_used_con, battery_width); // 设置进度条宽度
    
    // 更新电池百分比标签文本
    char battery_text[4] = {0};
    snprintf(battery_text, sizeof(battery_text), "%d", battery_percent);
    lv_label_set_text(battery_label, battery_text);
}

// ====================== 全局样式初始化函数 ======================
static void init_global_styles(void)
{
    // 1. 初始化主页渐变样式（只执行一次）
    if(!scr_grad_style_inited) {
        lv_style_init(&scr_grad_style);
        scr_grad_style_inited = true;
    }
}
#endif

#endif