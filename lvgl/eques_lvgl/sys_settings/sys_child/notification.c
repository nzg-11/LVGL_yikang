#include "notification.h"
#include "../lv_sys_settings.h"
#include <stdio.h>
#include <string.h>

// 弹窗相关静态变量
static lv_obj_t *dnd_popup = NULL;
static lv_obj_t *start_hour_roller = NULL;
static lv_obj_t *start_min_roller = NULL;
static lv_obj_t *end_hour_roller = NULL;
static lv_obj_t *end_min_roller = NULL;
static lv_obj_t *time_label = NULL; // 用于更新主页面时间段文本

// 内部状态变量（静态隐藏，仅本文件可见）
static lv_obj_t *notify_scr = NULL;
static notify_mode_t g_notify_mode = NOTIFY_ALL;
bool g_dnd_state = false;
const char *g_dnd_time_slot = "22:00-6:00";

// ========== 新增：复选框数组（对应两个通知模式） ==========
static lv_obj_t *g_notify_checkboxes[NOTIFY_MODE_MAX] = {NULL};
// 通知模式文本（和亮屏时间的g_time_strs对应）
static const char *g_notify_mode_strs[] = {"Accept all messages", "Accept alerts/doorbell only"};

// 免打扰开关对象
static lv_obj_t *g_dnd_switch = NULL;

// ========== 替换原单选回调：复用亮屏时间的互斥逻辑 ==========
static void notify_mode_checkbox_cb(lv_event_t *e);
// ========== 新增：容器点击回调（触发复选框选择） ==========
static void notify_container_click_cb(lv_event_t *e);
static void dnd_switch_cb(lv_event_t *e);
static void dnd_time_click_cb(lv_event_t *e);

// 弹窗关闭回调
static void dnd_popup_close_cb(lv_event_t *e);
// 确定按钮回调
static void dnd_time_confirm_cb(lv_event_t *e);

// 子模块初始化
void notification_init(void) {
    // 初始化默认状态
}

// 获取通知模式
notify_mode_t notification_get_mode(void) {
    return g_notify_mode;
}

// 设置通知模式并更新UI
void notification_set_mode(notify_mode_t mode) {
    if (mode >= NOTIFY_MODE_MAX) return;
    g_notify_mode = mode;

    // 遍历复选框实现互斥选中
    for (int i = 0; i < NOTIFY_MODE_MAX; i++) {
        if (g_notify_checkboxes[i] == NULL) continue;
        
        if (i == mode) {
            lv_obj_add_state(g_notify_checkboxes[i], LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(g_notify_checkboxes[i], LV_STATE_CHECKED);
        }
    }
}

// 获取免打扰状态
bool notification_get_dnd_state(void) {
    return g_dnd_state;
}

// 设置免打扰状态并更新UI
void notification_set_dnd_state(bool state) {
    g_dnd_state = state;

    // 更新开关状态
    if (g_dnd_switch != NULL) {
        if (state) {
            lv_obj_add_state(g_dnd_switch, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(g_dnd_switch, LV_STATE_CHECKED);
        }
    }
}

// 获取免打扰时间段
const char *notification_get_dnd_time_slot(void) {
    return g_dnd_time_slot;
}

// 设置免打扰时间段
void notification_set_dnd_time_slot(const char *slot) {
    if (slot == NULL) return;
    g_dnd_time_slot = slot;
}

// ========== 核心：互斥复选框回调（和亮屏时间的light_time_checkbox_cb一致） ==========
static void notify_mode_checkbox_cb(lv_event_t *e) {
    if (e == NULL) return;
    int clicked_idx = (int)(intptr_t)lv_event_get_user_data(e);

    // 遍历所有复选框实现互斥单选
    for (int i = 0; i < NOTIFY_MODE_MAX; i++) {
        if (g_notify_checkboxes[i] == NULL) continue;
        
        if (i == clicked_idx) {
            lv_obj_add_state(g_notify_checkboxes[i], LV_STATE_CHECKED);
            g_notify_mode = (notify_mode_t)i; // 更新全局状态
        } else {
            lv_obj_clear_state(g_notify_checkboxes[i], LV_STATE_CHECKED);
        }
    }
}

// ========== 容器点击回调（和亮屏时间的time_container_click_cb一致） ==========
static void notify_container_click_cb(lv_event_t *e) {
    if (e == NULL) return;
    int idx = (int)(long)lv_event_get_user_data(e);
    
    if (idx >= 0 && idx < NOTIFY_MODE_MAX && g_notify_checkboxes[idx] != NULL) {
        // 模拟点击复选框
        lv_event_send(g_notify_checkboxes[idx], LV_EVENT_VALUE_CHANGED, NULL);
    }
}

// 免打扰开关回调
static void dnd_switch_cb(lv_event_t *e) {
    if (e == NULL) return;
    lv_obj_t *sw = lv_event_get_target(e);
    notification_set_dnd_state(lv_obj_has_state(sw, LV_STATE_CHECKED));
}

// 免打扰时间段点击回调
static void dnd_time_click_cb(lv_event_t *e) {
    if (e == NULL) return;
    LV_LOG_USER("DND time slot clicked");

    // 1. 创建全屏遮罩层
    dnd_popup = lv_obj_create(lv_scr_act()); // 父对象是当前屏幕
    lv_obj_set_size(dnd_popup, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(dnd_popup, 0, 0);
    lv_obj_set_style_bg_color(dnd_popup, lv_color_hex(0x000000), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(dnd_popup, LV_OPA_50, LV_STATE_DEFAULT); // 半透明黑色
    lv_obj_set_style_border_opa(dnd_popup, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(dnd_popup, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(dnd_popup, dnd_popup_close_cb, LV_EVENT_CLICKED, NULL); // 点击遮罩关闭

    // 2. 创建底部时间选择器容器
    lv_obj_t *time_picker = lv_obj_create(dnd_popup);
    lv_obj_set_size(time_picker, LV_HOR_RES,400); // 下半屏
    lv_obj_set_align(time_picker, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_bg_color(time_picker, lv_color_hex(0xE0EDFF), LV_STATE_DEFAULT);
    lv_obj_set_style_radius(time_picker, 36, LV_STATE_DEFAULT); // 顶部圆角
    lv_obj_set_style_pad_top(time_picker, 20, LV_STATE_DEFAULT);
    lv_obj_clear_flag(time_picker, LV_OBJ_FLAG_CLICKABLE); // 容器本身不响应点击，避免关闭弹窗

    // 3. 创建标题栏
    lv_obj_t *header = lv_obj_create(time_picker);
    lv_obj_set_size(header, LV_HOR_RES, 60);
    lv_obj_set_align(header, LV_ALIGN_TOP_MID);
    lv_obj_set_style_bg_opa(header, LV_OPA_0, LV_STATE_DEFAULT); // 透明背景
    lv_obj_set_style_border_width(header, 0, LV_STATE_DEFAULT);

    lv_obj_t *header1 = lv_obj_create(time_picker);
    lv_obj_set_size(header1, 710, 5);
    lv_obj_set_pos(header1, 30, 60);
    lv_obj_set_style_bg_color(header1, lv_color_hex(0x000000), LV_STATE_DEFAULT);


    // 取消按钮
    lv_obj_t *cancel_btn = lv_btn_create(header);
    lv_obj_set_size(cancel_btn, 120, 46);
    lv_obj_set_align(cancel_btn, LV_ALIGN_LEFT_MID);
    lv_obj_set_pos(cancel_btn, 30, 0);
    lv_obj_set_style_bg_opa(cancel_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cancel_btn, 0, LV_STATE_DEFAULT);
    //  移除阴影（如果有轻微的阴影线）
    lv_obj_set_style_shadow_width(cancel_btn, 0, LV_STATE_DEFAULT);
    lv_obj_t *cancel_label = create_text_label(cancel_btn, "cancel", 
                                               &lv_font_montserrat_36, lv_color_hex(0xBDBDBD), 
                                               0, 0, LV_OPA_100);
    lv_obj_set_align(cancel_label, LV_ALIGN_CENTER);

    lv_obj_add_event_cb(cancel_btn, dnd_popup_close_cb, LV_EVENT_CLICKED, NULL);

    // 标题
    lv_obj_t *title_label = create_text_label(header, "dnd_time", 
                                             &lv_font_montserrat_36, lv_color_hex(0x192A46), 
                                             0, 0, LV_OPA_100);
    lv_obj_set_align(title_label, LV_ALIGN_CENTER);

    // 确定按钮
    lv_obj_t *confirm_btn = lv_btn_create(header);
    lv_obj_set_size(confirm_btn, 150, 46);
    lv_obj_set_align(confirm_btn, LV_ALIGN_RIGHT_MID);
    lv_obj_set_pos(confirm_btn, -30, 0);
    lv_obj_set_style_bg_opa(confirm_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(confirm_btn, 0, LV_STATE_DEFAULT);
    //  移除阴影（如果有轻微的阴影线）
    lv_obj_set_style_shadow_width(confirm_btn, 0, LV_STATE_DEFAULT);
    lv_obj_t *confirm_label = create_text_label(confirm_btn, "confirm", 
                                               &lv_font_montserrat_36, lv_color_hex(0x00BDBD), 
                                               0, 0, LV_OPA_100);
    lv_obj_set_align(confirm_label, LV_ALIGN_CENTER);
    lv_obj_add_event_cb(confirm_btn, dnd_time_confirm_cb, LV_EVENT_CLICKED, NULL);

    // 4. 创建时间滚轮（开始时间和结束时间）
    // 小时选项：00-23
    static const char *hour_opts = "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23";
    // 分钟选项：00-59，步长10
    static const char *min_opts = 
    "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n"
    "10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n"
    "20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n"
    "30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n"
    "40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n"
    "50\n51\n52\n53\n54\n55\n56\n57\n58\n59";

    // 解析当前时间段，设置滚轮默认选中项
    int start_h, start_m, end_h, end_m;
    sscanf(g_dnd_time_slot, "%d:%d-%d:%d", &start_h, &start_m, &end_h, &end_m);

    // 开始时间容器
    lv_obj_t *start_time_cont = lv_obj_create(time_picker);
    lv_obj_set_size(start_time_cont, LV_HOR_RES / 2 - 20, 300);
    lv_obj_set_align(start_time_cont, LV_ALIGN_LEFT_MID);
    lv_obj_set_pos(start_time_cont, 10, 10);
    lv_obj_set_style_bg_opa(start_time_cont, LV_OPA_0, LV_STATE_DEFAULT);
    //start_time_cont阴影与轮廓
    lv_obj_set_style_shadow_opa(start_time_cont, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(start_time_cont, 0, LV_STATE_DEFAULT);

    lv_obj_t *start_label = create_text_label(start_time_cont, "start_time", 
                                             &lv_font_montserrat_36, lv_color_hex(0x9E9E9E), 
                                             0, 20, LV_OPA_100);
    lv_obj_set_align(start_label, LV_ALIGN_TOP_MID);

    // 开始小时滚轮
    start_hour_roller = lv_roller_create(start_time_cont);
    lv_roller_set_options(start_hour_roller, hour_opts, LV_ROLLER_MODE_NORMAL);//设置滚轮选项
    lv_obj_set_size(start_hour_roller, 95, 180);
    lv_obj_set_align(start_hour_roller, LV_ALIGN_LEFT_MID);
    lv_obj_set_pos(start_hour_roller, 75, 30);
    lv_roller_set_selected(start_hour_roller, start_h, LV_ANIM_OFF);//设置滚轮选中项
    //start_hour_roller里面字体大小为22
    lv_obj_set_style_text_font(start_hour_roller, &lv_font_montserrat_22, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(start_hour_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(start_hour_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(start_hour_roller, 0, LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(start_hour_roller, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(start_hour_roller, lv_color_hex(0x999999), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(start_hour_roller, lv_color_hex(0x000000), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(start_hour_roller, lv_color_hex(0x999999), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(start_hour_roller, lv_color_hex(0x000000), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(start_hour_roller, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(start_hour_roller, LV_OPA_TRANSP, LV_STATE_DEFAULT);

    // 开始分钟滚轮
    start_min_roller = lv_roller_create(start_time_cont);
    lv_roller_set_options(start_min_roller, min_opts, LV_ROLLER_MODE_NORMAL);
    lv_obj_set_size(start_min_roller, 95, 180);
    lv_obj_set_align(start_min_roller, LV_ALIGN_RIGHT_MID);
    lv_obj_set_pos(start_min_roller, -75, 30);
    lv_roller_set_selected(start_min_roller, start_m / 10, LV_ANIM_OFF);
    lv_obj_set_style_text_font(start_min_roller, &lv_font_montserrat_22, LV_STATE_DEFAULT);//设置滚轮字体大小
    lv_obj_set_style_bg_opa(start_min_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(start_min_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(start_min_roller, 0, LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(start_min_roller, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(start_min_roller, lv_color_hex(0x999999), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(start_min_roller, lv_color_hex(0x000000), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(start_min_roller, lv_color_hex(0x999999), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(start_min_roller, lv_color_hex(0x000000), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(start_min_roller, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(start_min_roller, LV_OPA_TRANSP, LV_STATE_DEFAULT);

    // 结束时间容器
    lv_obj_t *end_time_cont = lv_obj_create(time_picker);
    lv_obj_set_size(end_time_cont, LV_HOR_RES / 2 - 20, 300);
    lv_obj_set_align(end_time_cont, LV_ALIGN_RIGHT_MID);
    lv_obj_set_pos(end_time_cont, -10, 10);
    lv_obj_set_style_bg_opa(end_time_cont, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(end_time_cont, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(end_time_cont, 0, LV_STATE_DEFAULT);

    lv_obj_t *end_label = create_text_label(end_time_cont, "end_time", 
                                           &lv_font_montserrat_36, lv_color_hex(0x9E9E9E), 
                                           0, 20, LV_OPA_100);
    lv_obj_set_align(end_label, LV_ALIGN_TOP_MID);

    // 结束小时滚轮
    end_hour_roller = lv_roller_create(end_time_cont);
    lv_roller_set_options(end_hour_roller, hour_opts, LV_ROLLER_MODE_NORMAL);
    lv_obj_set_size(end_hour_roller, 95, 180);
    lv_obj_set_align(end_hour_roller, LV_ALIGN_LEFT_MID);
    lv_obj_set_pos(end_hour_roller, 75, 30);
    lv_roller_set_selected(end_hour_roller, end_h, LV_ANIM_OFF);
    lv_obj_set_style_text_font(end_hour_roller, &lv_font_montserrat_22, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(end_hour_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(end_hour_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(end_hour_roller, 0, LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(end_hour_roller, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(end_hour_roller, lv_color_hex(0x999999), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(end_hour_roller, lv_color_hex(0x000000), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(end_hour_roller, lv_color_hex(0x999999), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(end_hour_roller, lv_color_hex(0x000000), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(end_hour_roller, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(end_hour_roller, LV_OPA_TRANSP, LV_STATE_DEFAULT);

    // 结束分钟滚轮
    end_min_roller = lv_roller_create(end_time_cont);
    lv_roller_set_options(end_min_roller, min_opts, LV_ROLLER_MODE_NORMAL);
    lv_obj_set_size(end_min_roller, 95, 180);
    lv_obj_set_align(end_min_roller, LV_ALIGN_RIGHT_MID);
    lv_obj_set_pos(end_min_roller, -75, 30);
    lv_roller_set_selected(end_min_roller, end_m / 10, LV_ANIM_OFF);
    lv_obj_set_style_text_font(end_min_roller, &lv_font_montserrat_22, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(end_min_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(end_min_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(end_min_roller, 0, LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(end_min_roller, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(end_min_roller, lv_color_hex(0x999999), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(end_min_roller, lv_color_hex(0x000000), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(end_min_roller, lv_color_hex(0x999999), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(end_min_roller, lv_color_hex(0x000000), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(end_min_roller, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(end_min_roller, LV_OPA_TRANSP, LV_STATE_DEFAULT);
}

// 弹窗关闭回调
static void dnd_popup_close_cb(lv_event_t *e) {
    if (dnd_popup != NULL) {
        lv_obj_del(dnd_popup);
        dnd_popup = NULL;
        start_hour_roller = NULL;
        start_min_roller = NULL;
        end_hour_roller = NULL;
        end_min_roller = NULL;
    }
}

// 确定按钮回调：更新时间段并关闭弹窗
static void dnd_time_confirm_cb(lv_event_t *e) {
    // 获取滚轮选择的时间
    char start_hour[3], start_min[3], end_hour[3], end_min[3];
    lv_roller_get_selected_str(start_hour_roller, start_hour, sizeof(start_hour));
    lv_roller_get_selected_str(start_min_roller, start_min, sizeof(start_min));
    lv_roller_get_selected_str(end_hour_roller, end_hour, sizeof(end_hour));
    lv_roller_get_selected_str(end_min_roller, end_min, sizeof(end_min));

    // 格式化新的时间段字符串
    static char new_time_slot[16];
    snprintf(new_time_slot, sizeof(new_time_slot), "%s:%s-%s:%s",
             start_hour, start_min, end_hour, end_min);
    notification_set_dnd_time_slot(new_time_slot);

    // 更新主页面的时间段文本
    if (time_label != NULL && lv_obj_is_valid(time_label)) {
        char time_text[64];
        snprintf(time_text, sizeof(time_text), "       %s", notification_get_dnd_time_slot());
        lv_label_set_text(time_label, time_text);
    }

    // 关闭弹窗
    dnd_popup_close_cb(e);
}

// 创建消息通知子页面
void ui_notification_settings_create(lv_obj_t *homepage_scr) {
    // 1. 创建子页面对象
    // lv_obj_t *notify_scr = lv_obj_create(NULL);
    if(is_lv_obj_valid(notify_scr)) {
        lv_obj_del(notify_scr);
        notify_scr = NULL;
    }
    notify_scr = lv_obj_create(NULL);  
    
    // 2. 复用主模块渐变样式
    extern lv_style_t sys_settings_grad_style;
    lv_style_reset(&sys_settings_grad_style);
    lv_style_set_bg_color(&sys_settings_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&sys_settings_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&sys_settings_grad_style, LV_GRAD_DIR_VER);
    lv_obj_add_style(notify_scr, &sys_settings_grad_style, LV_STATE_DEFAULT);

    // 3. 添加标题“消息通知”
    create_text_label(notify_scr, "Message_Notification", &lv_font_montserrat_36, 
                     lv_color_hex(0xFFFFFF), 83, 80, LV_OPA_100);

    int y_pos = 150;
    // --- 1. 通知模式：创建两个容器
    lv_obj_t *notify_con[NOTIFY_MODE_MAX]; // 存储两个模式的容器
    for (int i = 0; i < NOTIFY_MODE_MAX; i++) {
        // 创建容器
        notify_con[i] = create_container(
            notify_scr, 48, y_pos, 928, 83, 
            lv_color_hex(0x192A46), LV_OPA_100, 6, 
            lv_color_hex(0x2E4B7D), 0, LV_OPA_0
        );
        if (notify_con[i] == NULL) continue;

        // 容器可点击 + 按下透明度（和亮屏时间一致）
        lv_obj_add_flag(notify_con[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(notify_con[i], LV_OPA_70, LV_STATE_PRESSED);
        // 绑定容器点击回调（传递当前索引）
        lv_obj_add_event_cb(notify_con[i], notify_container_click_cb, LV_EVENT_CLICKED, (void *)(long)i);
        y_pos += 87; // 和亮屏时间的间距一致
    }
    // --- 2. 创建互斥复选框
    for (int i = 0; i < NOTIFY_MODE_MAX; i++) {
        // 父对象是对应容器
        lv_obj_t *parent_con = notify_con[i];
        if (parent_con == NULL) parent_con = notify_scr;

        lv_obj_t *mode_label = create_text_label(
            parent_con, g_notify_mode_strs[i], 
            &lv_font_montserrat_36,  // 和亮屏时间一致的字体
            lv_color_hex(0xFFFFFF),  // 白色文字
            0, 0,                  // 容器内左偏移20px，垂直居中
            LV_OPA_100
        );

        // 创建复选框
        g_notify_checkboxes[i] = lv_checkbox_create(parent_con);
        // 设置模式文本
        lv_checkbox_set_text(g_notify_checkboxes[i], "");
        // 1. 设置复选框整体右对齐，紧贴容器左侧
        lv_obj_set_align(g_notify_checkboxes[i], LV_ALIGN_RIGHT_MID);
        // 自适应大小
        lv_obj_set_size(g_notify_checkboxes[i], LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        // 圆形指示器
        lv_obj_set_style_radius(g_notify_checkboxes[i], LV_RADIUS_CIRCLE, LV_PART_INDICATOR);

        // 复选框样式
        lv_obj_set_style_border_color(g_notify_checkboxes[i], lv_color_hex(0xFFFFFF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(g_notify_checkboxes[i], 3, LV_PART_INDICATOR | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(g_notify_checkboxes[i], LV_OPA_0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(g_notify_checkboxes[i], lv_color_hex(0x00BDBD), LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_set_style_size(g_notify_checkboxes[i], 20, LV_PART_INDICATOR | LV_STATE_CHECKED);

        // 文字样式
        lv_obj_set_style_text_font(g_notify_checkboxes[i], &lv_font_montserrat_36, LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(g_notify_checkboxes[i], lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);

        // 默认选中当前配置的项
        if (i == g_notify_mode) {
            lv_obj_add_state(g_notify_checkboxes[i], LV_STATE_CHECKED);
        }
        // 绑定复选框点击回调
        lv_obj_add_event_cb(g_notify_checkboxes[i], notify_mode_checkbox_cb, LV_EVENT_VALUE_CHANGED, (void *)(intptr_t)i);
    }

    // 提示文本
    lv_obj_t *hint_label = create_text_label(notify_scr, "choose_notify_time", 
                                           &lv_font_montserrat_16, lv_color_hex(0xFFFFFF), 
                                           81, y_pos + 10, LV_OPA_70);
    y_pos += 40;

    // --- 2. 消息免打扰开关 ---
    lv_obj_t *dnd_con = create_container(
        notify_scr, 48, y_pos, 928, 83, 
        lv_color_hex(0x192A46), LV_OPA_100, 6, 
        lv_color_hex(0x2E4B7D), 0, LV_OPA_0
    );
    if (dnd_con) {
        // 免打扰文本
        lv_obj_t *dnd_label = create_text_label(notify_scr, "Do_Not_Disturb", 
                                               &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 
                                               80, 390, LV_OPA_100);

        // 免打扰开关
        g_dnd_switch = lv_switch_create(dnd_con);
        lv_obj_set_size(g_dnd_switch, 60, 37);
        lv_obj_set_align(g_dnd_switch, LV_ALIGN_RIGHT_MID);
        // 开关样式
        lv_obj_set_style_bg_color(g_dnd_switch, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(g_dnd_switch, lv_color_hex(0x00BDBD), LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_set_style_bg_color(g_dnd_switch, lv_color_hex(0x020A17), LV_PART_KNOB);
        lv_obj_set_style_size(g_dnd_switch, 28, LV_PART_KNOB);
        // 绑定回调
        lv_obj_add_event_cb(g_dnd_switch, dnd_switch_cb, LV_EVENT_VALUE_CHANGED, NULL);
        // 初始化状态
        notification_set_dnd_state(g_dnd_state);
    }
    y_pos += 87;

    // --- 3. 免打扰时间段 ---
    lv_obj_t *time_con = create_container(
        notify_scr, 48, y_pos, 928, 83, 
        lv_color_hex(0x192A46), LV_OPA_100, 6, 
        lv_color_hex(0x2E4B7D), 0, LV_OPA_0
    );
    if (time_con) {
        lv_obj_add_flag(time_con, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(time_con, LV_OPA_70, LV_STATE_PRESSED);
        lv_obj_add_event_cb(time_con, dnd_time_click_cb, LV_EVENT_CLICKED, NULL);

        // 时间段文本
        time_label = create_text_label(notify_scr, "dnd_time", 
                                                &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 
                                                777, 485, LV_OPA_70);
        char time_text[64];
        lv_obj_t *time_label1 = create_text_label(notify_scr, "DND_Time_Slot", 
                                                &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 
                                                80, 477, LV_OPA_100);
        snprintf(time_text, sizeof(time_text), "       %s", notification_get_dnd_time_slot());
        lv_label_set_text(time_label, time_text);

        // // 右侧箭头图标
        // lv_obj_t *arrow_img = create_image_obj(notify_scr, "D:Vector.png", 720, 520);
    }
    // // 4. 添加返回按钮
    // lv_obj_t *back_btn = create_image_obj(notify_scr, "D:back.png", 52, 123);
    // if (back_btn) {
    //     lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    //     lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);
    //     lv_obj_add_event_cb(back_btn, back_btn_click_cb, LV_EVENT_CLICKED, homepage_scr);
    // }
    // 返回
    lv_obj_t *back_btn = create_container_circle(notify_scr, 52, 90, 30,
    true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(back_btn,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn,LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn,back_btn_click_cb,LV_EVENT_CLICKED,homepage_scr);

    // 7. 更新状态栏父对象
    update_status_bar_parent(notify_scr);
    
    // 8. 切换到子页面
    lv_scr_load(notify_scr);
}
