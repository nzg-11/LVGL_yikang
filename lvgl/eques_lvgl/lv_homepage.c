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
static lv_obj_t *wifi_img = NULL;
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
    //云朵替代
    // create_container_circle(scr, 217, 163, 89,
    // true, lv_color_hex(0xFFCE50), lv_color_hex(0x808080), 0, LV_OPA_0);
    // create_container_circle(scr, 181, 192, 107,
    // true, lv_color_hex(0xFFFFFF), lv_color_hex(0x808080), 0, LV_OPA_0);
    // 大雨图标
    create_text_label(scr, ICON_RAIN, & fontawesome_icon_150, lv_color_hex(0xFFFFFF), 200, 134, LV_OPA_100);
    
    create_text_label(scr, "25°C", &eques_regular_60, lv_color_hex(0xFFFFFF), 200, 327, LV_OPA_100);

    //开关锁容器
    lv_obj_t *unlocking_con = create_custom_gradient_container
    (scr, 48, 470, 239, 82, 0, 0x006BDC, 0x00BDBD, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_set_style_pad_all(unlocking_con, 0, LV_STATE_DEFAULT);
    lv_obj_t *unlock_img = create_text_label(unlocking_con, ICON_UNLOCK, &fontawesome_icon_32, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align_to(unlock_img, unlocking_con, LV_ALIGN_LEFT_MID, 25, 0);

    lv_obj_t *locking_con = create_container
    (scr, 287, 470, 239, 82, lv_color_hex(0x2E4B7D), LV_OPA_100, 0, lv_color_hex(0x2E4B7D), 0, LV_OPA_0);
    lv_obj_set_style_pad_all(locking_con, 0, LV_STATE_DEFAULT);
    lv_obj_t *lock_img = create_text_label(locking_con, ICON_LOCK, &fontawesome_icon_32, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align_to(lock_img, locking_con, LV_ALIGN_LEFT_MID, 25, 0);
    //开关锁文本
    create_text_label(scr, "一键开锁", &eques_bold_28, lv_color_hex(0xFFFFFF), 130, 495, LV_OPA_100);
    create_text_label(scr, "一键关锁", &eques_bold_28, lv_color_hex(0xFFFFFF), 367, 495, LV_OPA_100);
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
    (msg_center_con, "消息中心", &eques_regular_24, lv_color_hex(0xFFFFFF), 600, 111, LV_OPA_100);
    lv_obj_align_to(msg_center_lable, msg_center_con, LV_ALIGN_TOP_MID, 0, 111);
    // 消息中心图标
    lv_obj_t *msg_cen_img = create_text_label(scr, ICON_MSG_CENTER, &menu_icon, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align_to(msg_cen_img, msg_center_con, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_add_event_cb(msg_center_con, msg_center_btn_click_cb, LV_EVENT_CLICKED, scr);

    // 监控视频
    lv_obj_t *monitor_video_con = create_custom_gradient_container(
    scr, 795, 24, 205, 174, 16, 0x34568F, 0x1F3150, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_set_style_pad_all(monitor_video_con, 0, LV_STATE_DEFAULT);
    // 监控视频文本
    lv_obj_t *monitor_video_lable = create_text_label
    (monitor_video_con, "监控视频", &eques_regular_24, lv_color_hex(0xFFFFFF), 810, 111, LV_OPA_100);
    lv_obj_align_to(monitor_video_lable, monitor_video_con, LV_ALIGN_TOP_MID, 0, 111);

    // 监控视频图标
    lv_obj_t *monitor_video_img = create_text_label(scr, ICON_MONITOR_VIDEO, &menu_icon, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align_to(monitor_video_img, monitor_video_con, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_add_flag(monitor_video_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(monitor_video_con, LV_OPA_70, LV_STATE_PRESSED);
    lv_obj_add_event_cb(monitor_video_con, monitor_video_btn_click_cb, LV_EVENT_CLICKED, scr);

    // 用户管理
    lv_obj_t *user_manage_con = create_custom_gradient_container(
    scr, 573, 214, 205, 174, 16, 0x34568F, 0x1F3150, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_set_style_pad_all(user_manage_con, 0, LV_STATE_DEFAULT);
    // 用户管理文本
    lv_obj_t *user_manage_lable = create_text_label
    (user_manage_con, "管理用户", &eques_regular_24, lv_color_hex(0xFFFFFF), 600, 111, LV_OPA_100);
    lv_obj_align_to(user_manage_lable, user_manage_con, LV_ALIGN_TOP_MID, 0, 111);
    // 用户管理图标
    lv_obj_t *user_manage_img = create_text_label(scr, ICON_USER_MANAGE, &menu_icon, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align_to(user_manage_img, user_manage_con, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_add_flag(user_manage_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(user_manage_con, LV_OPA_70, LV_STATE_PRESSED);
    lv_obj_add_event_cb(user_manage_con, user_manage_btn_click_cb, LV_EVENT_CLICKED, scr);

    // 设备信息
    lv_obj_t *dev_info_con = create_custom_gradient_container(
    scr, 795, 214, 205, 174, 16, 0x34568F, 0x1F3150, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_set_style_pad_all(dev_info_con, 0, LV_STATE_DEFAULT);
    // 设备信息文本
    lv_obj_t *dev_info_lable = create_text_label
    (dev_info_con, "设备信息", &eques_regular_24, lv_color_hex(0xFFFFFF), 810, 111, LV_OPA_100);
    lv_obj_align_to(dev_info_lable, dev_info_con, LV_ALIGN_TOP_MID, 0, 111);

    // 设备信息图标
    lv_obj_t *dev_info_img = create_text_label(scr, ICON_DEV_INFO, &menu_icon, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align_to(dev_info_img, dev_info_con, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_add_flag(dev_info_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(dev_info_con, LV_OPA_70, LV_STATE_PRESSED);
    lv_obj_add_event_cb(dev_info_con, dev_info_btn_click_cb, LV_EVENT_CLICKED, scr);

    // 文件缓存
    lv_obj_t *file_cache_con = create_custom_gradient_container
    (scr, 573, 404, 205, 174, 16, 0x34568F, 0x1F3150, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_set_style_pad_all(file_cache_con, 0, LV_STATE_DEFAULT);
    // 文件缓存文本
    lv_obj_t *file_cache_lable = create_text_label
    (file_cache_con, "文件缓存", &eques_regular_24, lv_color_hex(0xFFFFFF), 600, 111, LV_OPA_100);
    lv_obj_align_to(file_cache_lable, file_cache_con, LV_ALIGN_TOP_MID, 0, 111);
    // 文件缓存图标
    lv_obj_t *file_cache_img = create_text_label(scr, ICON_FILE_CACHE, &menu_icon, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align_to(file_cache_img, file_cache_con, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_add_flag(file_cache_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(file_cache_con, LV_OPA_70, LV_STATE_PRESSED);
    lv_obj_add_event_cb(file_cache_con, file_cache_btn_click_cb, LV_EVENT_CLICKED, scr);

    // 系统设置
    lv_obj_t *sys_settings_con = create_custom_gradient_container(
    scr, 795, 404, 205, 174, 16, 0x34568F, 0x1F3150, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_set_style_pad_all(sys_settings_con, 0, LV_STATE_DEFAULT);
    // 系统设置文本
    lv_obj_t *sys_settings_lable = create_text_label
    (sys_settings_con, "系统设置", &eques_regular_24, lv_color_hex(0xFFFFFF), 810, 111, LV_OPA_100);
    lv_obj_align_to(sys_settings_lable, sys_settings_con, LV_ALIGN_TOP_MID, 0, 111);

    // 系统设置图标
    lv_obj_t *sys_settings_img = create_text_label(scr, ICON_SYS_SETTING, &menu_icon, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align_to(sys_settings_img, sys_settings_con, LV_ALIGN_TOP_MID, 0, 30);
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
        date_label = create_text_label(status_bar, "2023-01-01", &eques_regular_36, lv_color_hex(0xFFFFFF),0, 0, LV_OPA_100);
        lv_obj_align(date_label, LV_ALIGN_LEFT_MID, 30, 0);

        // 时间
        time_label = create_text_label(status_bar, "12 : 00", &eques_regular_36, lv_color_hex(0xFFFFFF),0, 0, LV_OPA_100);
        lv_obj_align(time_label, LV_ALIGN_LEFT_MID, 270, 0);

        // WiFi
        wifi_img = create_text_label(status_bar, ICON_WIFI, &fontawesome_icon_32, lv_color_hex(0xFFFFFF), 0, 15, LV_OPA_100);
        lv_obj_align(wifi_img, LV_ALIGN_LEFT_MID, 390, 0);

        // 电池外框
        battery_con = create_container(status_bar, 0, 0, 65, 30, lv_color_hex(0x2E4B7D), LV_OPA_0, 0, lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
        lv_obj_align(battery_con, LV_ALIGN_LEFT_MID, 444, 0);
        // 电池进度
        battery_used_con = create_container(status_bar, 0, 0, 0, 24, lv_color_hex(0x00ac11), LV_OPA_100, 0, lv_color_hex(0xFFFFFF), 3, LV_OPA_0);
        lv_obj_align(battery_used_con, LV_ALIGN_LEFT_MID, 447, 0);
    }
    else
    {
        // ====================== 其他页面：靠右布局 ======================
        lv_obj_align(status_bar, LV_ALIGN_TOP_RIGHT, 0, 0);

        // 日期
        date_label = create_text_label(status_bar, "2023-01-01", &eques_regular_36, lv_color_hex(0xFFFFFF), 0, 15, LV_OPA_100);
        lv_obj_align(date_label, LV_ALIGN_LEFT_MID, 500, 0);

        // 时间
        time_label = create_text_label(status_bar, "12 : 00", &eques_regular_36, lv_color_hex(0xFFFFFF), 0, 15, LV_OPA_100);
        lv_obj_align(time_label, LV_ALIGN_LEFT_MID, 740, 0);

        // WiFi
        wifi_img = create_text_label(status_bar, ICON_WIFI, &fontawesome_icon_32, lv_color_hex(0xFFFFFF), 0, 15, LV_OPA_100);
        lv_obj_align(wifi_img, LV_ALIGN_LEFT_MID, 860, 0);

        // 电池外框
        battery_con = create_container(status_bar, 910, 21, 65, 30, lv_color_hex(0x2E4B7D), LV_OPA_0, 0, lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
        lv_obj_align(battery_con, LV_ALIGN_LEFT_MID, 910, 0);

        // 电池进度
        battery_used_con = create_container(status_bar, 913, 24, 0, 24, lv_color_hex(0x00ac11), LV_OPA_100, 0, lv_color_hex(0xFFFFFF), 3, LV_OPA_0);
        lv_obj_align(battery_used_con, LV_ALIGN_LEFT_MID, 913, 0);
    }

    // 电池百分比（共用）
    char battery_text[4] = {0};
    snprintf(battery_text, sizeof(battery_text), "%d", BATTERY_CAPACITY);
    battery_label = create_text_label(
        status_bar, battery_text,
        &eques_regular_24, lv_color_hex(0xFFFFFF),
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