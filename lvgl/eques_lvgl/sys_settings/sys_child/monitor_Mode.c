#include "monitor_mode.h"
#include "../lv_sys_settings.h"  // 引用上级主模块头文件
#include <stdio.h>

// 监控时间段相关静态变量
static monitor_slot_t g_slot_type = MONITOR_SLOT_FULL_DAY; // 默认全天
static char g_slot_strs[MONITOR_SLOT_MAX][16] = {
    "0:00-24:00",
    "8:00-18:00",
    "22:00-6:00"
};
// 内部状态变量（静态隐藏，仅本文件可见）
 lv_obj_t *monitor_scr = NULL; 
static monitor_mode_state_t g_monitor_state = MONITOR_MODE_ON; // 默认开启
static const char *g_monitor_state_str[] = {"已关闭", "已开启"};
// static const char *g_time_slot_str = "8:00-18:00"; // 默认时间段
static lv_obj_t *g_monitor_view_label = NULL; // 主页面状态显示标签
 lv_obj_t *g_switch = NULL;             // 开关对象
 lv_obj_t *g_state_label = NULL;        // 子页面中“已开启/关闭”文本标签
 lv_obj_t *g_time_con = NULL;
 lv_obj_t *g_time_slot_label = NULL; // 保存时间段文本标签，直接刷新

// 弹窗相关静态变量
 lv_obj_t *monitor_popup = NULL;
static lv_obj_t *start_hour_roller = NULL;
static lv_obj_t *start_min_roller = NULL;
static lv_obj_t *end_hour_roller = NULL;
static lv_obj_t *end_min_roller = NULL;
static lv_obj_t *g_slot_radios[MONITOR_SLOT_MAX] = {NULL}; // 单选按钮数组
lv_style_t monitor_grad_style;
// 内部回调函数声明
static void monitor_switch_cb(lv_event_t *e);
static void time_slot_click_cb(lv_event_t *e);
static void monitor_popup_close_cb(lv_event_t *e);
static void monitor_time_confirm_cb(lv_event_t *e);
static void slot_radio_cb(lv_event_t *e);

static void monitor_mode_destroy(void);
void monitor_mode_back_btn_click_cb(lv_event_t *e);

// 子模块初始化：绑定主页面显示标签
void monitor_mode_init(lv_obj_t *display_label) {
    g_monitor_view_label = display_label;
    if (g_monitor_view_label != NULL) {
        lv_label_set_text(g_monitor_view_label, g_monitor_state_str[g_monitor_state]);
    }
}

// 获取当前监控模式状态
monitor_mode_state_t monitor_mode_get_state(void) {
    return g_monitor_state;
}

// 设置监控模式状态并更新UI
void monitor_mode_set_state(monitor_mode_state_t state) {
    if (state >= MONITOR_MODE_MAX) return;
    g_monitor_state = state;

    // 更新开关状态
    if (g_switch != NULL) {
        if (state == MONITOR_MODE_ON) {
            lv_obj_add_state(g_switch, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(g_switch, LV_STATE_CHECKED);
        }
    }

    // 更新子页面状态文本
    if (g_state_label != NULL) {
        lv_label_set_text(g_state_label, g_monitor_state_str[state]);
    }

    // 更新主页面显示（如果绑定了标签）
    if (g_monitor_view_label != NULL) {
        lv_label_set_text(g_monitor_view_label, g_monitor_state_str[state]);
    }
}

// 开关状态变化回调
static void monitor_switch_cb(lv_event_t *e) {
    if (e == NULL) return;
    lv_obj_t *sw = lv_event_get_target(e);
    monitor_mode_set_state(lv_obj_has_state(sw, LV_STATE_CHECKED) ? MONITOR_MODE_ON : MONITOR_MODE_OFF);
}

// 获取当前时间段类型
monitor_slot_t monitor_mode_get_slot_type(void) {
    return g_slot_type;
}

// 设置当前时间段类型并更新显示
void monitor_mode_set_slot_type(monitor_slot_t type) {
    if (type >= MONITOR_SLOT_MAX) return;
    g_slot_type = type;

    // 更新单选按钮选中状态（如果存在）
    for (int i = 0; i < MONITOR_SLOT_MAX; i++) {
        if (lv_obj_is_valid(g_slot_radios[i])) {
            if (i == type) {
                lv_obj_add_state(g_slot_radios[i], LV_STATE_CHECKED);
            } else {
                lv_obj_clear_state(g_slot_radios[i], LV_STATE_CHECKED);
            }
        }
    }
        // ========== 刷新主页面的时间段文本 ==========
    if (g_time_slot_label != NULL) {
        lv_label_set_text(g_time_slot_label, monitor_mode_get_slot_str());
    }
}

// 获取当前时间段字符串
const char *monitor_mode_get_slot_str(void) {
    return g_slot_strs[g_slot_type];
}

// 单选按钮回调（互斥选择时间段类型）
static void slot_radio_cb(lv_event_t *e) {
    // 1. 双重防护：空指针 + 弹窗无效 + 防止事件重入
    if (e == NULL || !lv_obj_is_valid(monitor_popup) || !lv_event_get_target(e)) {
        return;
    }
    // 2. 只响应选中事件，过滤无效触发
    lv_obj_t *checkbox = lv_event_get_target(e);
    if (!lv_obj_has_state(checkbox, LV_STATE_CHECKED)) {
        return;
    }
    // 3. 安全获取索引
    int idx = (int)(intptr_t)lv_event_get_user_data(e);
    if (idx < 0 || idx >= MONITOR_SLOT_MAX) {
        return;
    }
    // 4. 安全互斥逻辑（只操作有效控件）
    for (int i = 0; i < MONITOR_SLOT_MAX; i++) {
        if (i != idx && lv_obj_is_valid(g_slot_radios[i])) {
            lv_obj_clear_state(g_slot_radios[i], LV_STATE_CHECKED);
        }
    }
    // 5. 最终更新状态
    monitor_mode_set_slot_type((monitor_slot_t)idx);
}

// 弹窗关闭回调
static void monitor_popup_close_cb(lv_event_t *e) {
    if (!lv_obj_is_valid(monitor_popup)) {
        monitor_popup = NULL;
        return;
    }
    lv_obj_del(monitor_popup);
    monitor_popup = NULL;
    start_hour_roller = NULL;
    start_min_roller = NULL;
    end_hour_roller = NULL;
    end_min_roller = NULL;
    memset(g_slot_radios, 0, sizeof(g_slot_radios));
}

// 确定按钮回调：更新自定义时间段并关闭弹窗
static void monitor_time_confirm_cb(lv_event_t *e) {
    // 安全判断：必须是自定义模式 + 弹窗有效
    if (g_slot_type != MONITOR_SLOT_CUSTOM || !lv_obj_is_valid(monitor_popup)) {
        monitor_popup_close_cb(NULL);
        return;
    }
    // 所有滚轮必须有效，才执行操作（杜绝野指针）
    if (!lv_obj_is_valid(start_hour_roller) || !lv_obj_is_valid(start_min_roller) ||
        !lv_obj_is_valid(end_hour_roller) || !lv_obj_is_valid(end_min_roller)) {
        monitor_popup_close_cb(NULL);
        return;
    }
    // 固定数组大小，杜绝内存越界
    char start_hour[4] = {0};
    char start_min[4] = {0};
    char end_hour[4] = {0};
    char end_min[4] = {0};
    // 安全获取滚轮值
    lv_roller_get_selected_str(start_hour_roller, start_hour, sizeof(start_hour));
    lv_roller_get_selected_str(start_min_roller, start_min, sizeof(start_min));
    lv_roller_get_selected_str(end_hour_roller, end_hour, sizeof(end_hour));
    lv_roller_get_selected_str(end_min_roller, end_min, sizeof(end_min));
    // 严格限制格式化长度，防止数组越界（核心！）
    snprintf(g_slot_strs[MONITOR_SLOT_CUSTOM],
             sizeof(g_slot_strs[MONITOR_SLOT_CUSTOM]) - 1,  // 留结束符
             "%s:%s-%s:%s",
             start_hour, start_min, end_hour, end_min);
    monitor_mode_set_slot_type(MONITOR_SLOT_CUSTOM);
    monitor_popup_close_cb(NULL);
}
// static void monitor_time_confirm_cb(lv_event_t *e) {
//     if (g_slot_type == MONITOR_SLOT_CUSTOM) {
//         char start_hour[4] = {0}, start_min[4] = {0};
//         char end_hour[4] = {0}, end_min[4] = {0};
//         // 获取滚轮选择的时间
//         // char start_hour[3], start_min[3], end_hour[3], end_min[3];
//         if (lv_obj_is_valid(start_hour_roller) && lv_obj_is_valid(start_min_roller) && 
//             lv_obj_is_valid(end_hour_roller) && lv_obj_is_valid(end_min_roller)) {
//             lv_roller_get_selected_str(start_hour_roller, start_hour, sizeof(start_hour));
//             lv_roller_get_selected_str(start_min_roller, start_min, sizeof(start_min));
//             lv_roller_get_selected_str(end_hour_roller, end_hour, sizeof(end_hour));
//             lv_roller_get_selected_str(end_min_roller, end_min, sizeof(end_min));

//             snprintf(g_slot_strs[MONITOR_SLOT_CUSTOM], sizeof(g_slot_strs[MONITOR_SLOT_CUSTOM]), 
//                      "%s:%s-%s:%s", start_hour, start_min, end_hour, end_min);
//             monitor_mode_set_slot_type(MONITOR_SLOT_CUSTOM);
//         }
//     }
//     // 关闭弹窗
//     monitor_popup_close_cb(NULL);
// }

static void time_slot_click_cb(lv_event_t *e) {
    if (e == NULL) return;
    LV_LOG_USER("time_slot_click_cb");
    if(lv_obj_is_valid(monitor_popup)) {
        lv_obj_del(monitor_popup);
        monitor_popup = NULL;
    }
    // if (monitor_popup != NULL) return; // 弹窗已存在，直接退出，不重复创建
    // 1. 创建全屏遮罩层（完全复用DND弹窗的遮罩样式）
    monitor_popup = lv_obj_create(lv_scr_act());
    lv_obj_set_size(monitor_popup, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(monitor_popup, 0, 0);
    lv_obj_set_style_bg_color(monitor_popup, lv_color_hex(0x000000), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(monitor_popup, LV_OPA_50, LV_STATE_DEFAULT); // 半透明黑色遮罩
    lv_obj_add_flag(monitor_popup, LV_OBJ_FLAG_CLICKABLE);
    // lv_obj_add_event_cb(monitor_popup, monitor_popup_close_cb, LV_EVENT_CLICKED, NULL);

    // 2. 创建底部时间选择器容器
    lv_obj_t *time_picker = lv_obj_create(monitor_popup);
    lv_obj_set_size(time_picker, LV_HOR_RES, 500);
    lv_obj_set_align(time_picker, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_bg_color(time_picker, lv_color_hex(0xE0EDFF), LV_STATE_DEFAULT);
    lv_obj_set_style_radius(time_picker, 36, LV_STATE_DEFAULT); // 顶部圆角
    lv_obj_set_style_pad_top(time_picker, 20, LV_STATE_DEFAULT);
    lv_obj_clear_flag(time_picker, LV_OBJ_FLAG_EVENT_BUBBLE);

    // 3. 创建标题栏
    lv_obj_t *header = lv_obj_create(time_picker);
    lv_obj_set_size(header, LV_HOR_RES, 60);
    lv_obj_set_align(header, LV_ALIGN_TOP_MID);
    lv_obj_set_style_bg_opa(header, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(header, 0, LV_STATE_DEFAULT);

    // 标题栏分割线
    // lv_obj_t *header1 = lv_obj_create(time_picker);
    // lv_obj_set_size(header1, 710, 5);
    // lv_obj_set_pos(header1, 30, 60);
    // lv_obj_set_style_bg_color(header1, lv_color_hex(0x000000), LV_STATE_DEFAULT);

    // 取消按钮
    lv_obj_t *cancel_btn = lv_btn_create(header);
    lv_obj_set_size(cancel_btn, 120, 46);
    lv_obj_set_align(cancel_btn, LV_ALIGN_LEFT_MID);
    lv_obj_set_pos(cancel_btn, 30, 0);
    lv_obj_set_style_bg_opa(cancel_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cancel_btn, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(cancel_btn, 0, LV_STATE_DEFAULT);
    lv_obj_t *cancel_label = create_text_label(cancel_btn, "取消", 
                                               &eques_regular_36, lv_color_hex(0xBDBDBD), 
                                               0, 0, LV_OPA_100);
    lv_obj_set_align(cancel_label, LV_ALIGN_CENTER);
    lv_obj_add_event_cb(cancel_btn, monitor_popup_close_cb, LV_EVENT_CLICKED, NULL);

    // 标题（改为“监控时间段”）
    // lv_obj_t *title_label = create_text_label(header, "title_label", 
    //                                          &lv_font_montserrat_28, lv_color_hex(0x192A46), 
    //                                          0, 0, LV_OPA_100);
    // lv_label_set_text(title_label, "monitor_time");
    // lv_obj_set_align(title_label, LV_ALIGN_CENTER);

    // 确定按钮（完全复用DND样式）
    lv_obj_t *confirm_btn = lv_btn_create(header);
    lv_obj_set_size(confirm_btn, 150, 46);
    lv_obj_set_align(confirm_btn, LV_ALIGN_RIGHT_MID);
    lv_obj_set_pos(confirm_btn, -30, 0);
    lv_obj_set_style_bg_opa(confirm_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(confirm_btn, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(confirm_btn, 0, LV_STATE_DEFAULT);
    lv_obj_t *confirm_label = create_text_label(confirm_btn, "确定", 
                                               &eques_regular_36, lv_color_hex(0x00BDBD), 
                                               0, 0, LV_OPA_100);
    lv_obj_set_align(confirm_label, LV_ALIGN_CENTER);
    lv_obj_add_event_cb(confirm_btn, monitor_time_confirm_cb, LV_EVENT_CLICKED, NULL);

    // 4. 创建monitor_slot_t对应的单选选项（用复选框模拟单选，兼容低版本LVGL）
    const char *slot_names[] = {"全天时间段: 0:00-24:00", "工作时间段: 8:00-18:00", "自定义时间段"};
    int y_pos = 80; // 分割线下方开始

    // 用复选框（lv_checkbox）
    for (int i = 0; i < MONITOR_SLOT_MAX; i++) {
        // 创建复选框（替代不存在的单选按钮）
        lv_obj_t *checkbox = lv_checkbox_create(time_picker);
        lv_obj_set_size(checkbox, 700, 70);
        lv_obj_set_pos(checkbox, 40, y_pos);
        // 设置复选框文本
        lv_checkbox_set_text(checkbox, slot_names[i]);
        // 适配样式（字体、颜色）
        lv_obj_set_style_text_font(checkbox, &eques_regular_32, LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(checkbox, lv_color_hex(0x192A46), LV_STATE_DEFAULT);

        lv_obj_set_style_radius(checkbox, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
        // 3. 设置边框样式（白色，3px）
        lv_obj_set_style_border_color(checkbox, lv_color_hex(0x192A46), LV_PART_INDICATOR | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(checkbox, 3, LV_PART_INDICATOR | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(checkbox, LV_OPA_0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
        // 4. 设置选中时的背景颜色（白色）
        lv_obj_set_style_bg_color(checkbox, lv_color_hex(0x192A46), LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_set_style_size(checkbox, 20, LV_PART_INDICATOR | LV_STATE_CHECKED);

        // 绑定回调（实现互斥逻辑）
        lv_obj_add_event_cb(checkbox, slot_radio_cb, LV_EVENT_VALUE_CHANGED, (void *)(intptr_t)i);
        g_slot_radios[i] = checkbox;

        // 设置默认选中状态
        if (i == g_slot_type) {
            lv_obj_add_state(checkbox, LV_STATE_CHECKED);
        }
        y_pos += 50; // 加大间距，适配36号字体
    }

    // 5. 创建自定义时间滚轮（复用DND的滚轮样式，位置下移适配单选选项）
    // 小时/分钟选项
    static const char *hour_opts = "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23";
    static const char *min_opts = 
    "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n"
    "10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n"
    "20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n"
    "30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n"
    "40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n"
    "50\n51\n52\n53\n54\n55\n56\n57\n58\n59";

    // 解析自定义时间段默认值
    int start_h, start_m, end_h, end_m;
    // sscanf(g_slot_strs[MONITOR_SLOT_CUSTOM], "%d:%d-%d:%d", &start_h, &start_m, &end_h, &end_m);
    if(sscanf(g_slot_strs[MONITOR_SLOT_CUSTOM], "%d:%d-%d:%d", &start_h, &start_m, &end_h, &end_m) != 4){
        // 解析失败，赋默认值，防止栈崩溃
        start_h = 22; start_m = 0; end_h = 6; end_m = 0;
    }

    // 开始时间容器
    lv_obj_t *start_time_cont = lv_obj_create(time_picker);
    lv_obj_set_size(start_time_cont, LV_HOR_RES / 2 - 20, 300);
    lv_obj_set_align(start_time_cont, LV_ALIGN_LEFT_MID);
    lv_obj_set_pos(start_time_cont, 10, y_pos - 50); // 适配单选选项的y_pos
    lv_obj_set_style_bg_opa(start_time_cont, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(start_time_cont, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(start_time_cont, 0, LV_STATE_DEFAULT);

    // 开始时间标签
    // lv_obj_t *start_label = create_text_label(start_time_cont, "start_time", 
    //                                          &lv_font_montserrat_28, lv_color_hex(0x9E9E9E), 
    //                                          0, 20, LV_OPA_100);
    // lv_obj_set_align(start_label, LV_ALIGN_TOP_MID);

    // 开始小时滚轮
    start_hour_roller = lv_roller_create(start_time_cont);
    lv_roller_set_options(start_hour_roller, hour_opts, LV_ROLLER_MODE_NORMAL);
    lv_obj_set_size(start_hour_roller, 95, 180);
    lv_obj_set_align(start_hour_roller, LV_ALIGN_LEFT_MID);
    lv_obj_set_pos(start_hour_roller, 75, 30);
    lv_roller_set_selected(start_hour_roller, start_h, LV_ANIM_OFF);
    lv_obj_set_style_text_font(start_hour_roller, &eques_regular_24, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(start_hour_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(start_hour_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(start_hour_roller, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(start_hour_roller, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(start_hour_roller, lv_color_hex(0x999999), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(start_hour_roller, lv_color_hex(0x000000), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(start_hour_roller, LV_OPA_TRANSP, LV_STATE_DEFAULT);

    // 开始分钟滚轮
    start_min_roller = lv_roller_create(start_time_cont);
    lv_roller_set_options(start_min_roller, min_opts, LV_ROLLER_MODE_NORMAL);
    lv_obj_set_size(start_min_roller, 95, 180);
    lv_obj_set_align(start_min_roller, LV_ALIGN_RIGHT_MID);
    lv_obj_set_pos(start_min_roller, -75, 30);
    lv_roller_set_selected(start_min_roller, start_m, LV_ANIM_OFF); // 修复：去掉/10，适配完整分钟
    lv_obj_set_style_text_font(start_min_roller, &eques_regular_24, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(start_min_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(start_min_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(start_min_roller, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(start_min_roller, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(start_min_roller, lv_color_hex(0x999999), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(start_min_roller, lv_color_hex(0x000000), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(start_min_roller, LV_OPA_TRANSP, LV_STATE_DEFAULT);

    // 结束时间容器
    lv_obj_t *end_time_cont = lv_obj_create(time_picker);
    lv_obj_set_size(end_time_cont, LV_HOR_RES / 2 - 20, 300);
    lv_obj_set_align(end_time_cont, LV_ALIGN_RIGHT_MID);
    lv_obj_set_pos(end_time_cont, -10, y_pos - 50);
    lv_obj_set_style_bg_opa(end_time_cont, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(end_time_cont, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(end_time_cont, 0, LV_STATE_DEFAULT);

    // 结束时间标签（复用DND）
    // lv_obj_t *end_label = create_text_label(end_time_cont, "end_time", 
    //                                        &lv_font_montserrat_28, lv_color_hex(0x9E9E9E), 
    //                                        0, 20, LV_OPA_100);
    // lv_obj_set_align(end_label, LV_ALIGN_TOP_MID);

    // 结束小时滚轮
    end_hour_roller = lv_roller_create(end_time_cont);
    lv_roller_set_options(end_hour_roller, hour_opts, LV_ROLLER_MODE_NORMAL);
    lv_obj_set_size(end_hour_roller, 95, 180);
    lv_obj_set_align(end_hour_roller, LV_ALIGN_LEFT_MID);
    lv_obj_set_pos(end_hour_roller, 75, 30);
    lv_roller_set_selected(end_hour_roller, end_h, LV_ANIM_OFF);
    lv_obj_set_style_text_font(end_hour_roller, &eques_regular_24, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(end_hour_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(end_hour_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(end_hour_roller, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(end_hour_roller, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(end_hour_roller, lv_color_hex(0x999999), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(end_hour_roller, lv_color_hex(0x000000), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(end_hour_roller, LV_OPA_TRANSP, LV_STATE_DEFAULT);

    // 结束分钟滚轮
    end_min_roller = lv_roller_create(end_time_cont);
    lv_roller_set_options(end_min_roller, min_opts, LV_ROLLER_MODE_NORMAL);
    lv_obj_set_size(end_min_roller, 95, 180);
    lv_obj_set_align(end_min_roller, LV_ALIGN_RIGHT_MID);
    lv_obj_set_pos(end_min_roller, -75, 30);
    lv_roller_set_selected(end_min_roller, end_m, LV_ANIM_OFF); // 修复：去掉/10
    lv_obj_set_style_text_font(end_min_roller, &eques_regular_24, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(end_min_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(end_min_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(end_min_roller, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(end_min_roller, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(end_min_roller, lv_color_hex(0x999999), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(end_min_roller, lv_color_hex(0x000000), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(end_min_roller, LV_OPA_TRANSP, LV_STATE_DEFAULT);
}


// 创建监控模式子页面
void ui_monitor_mode_settings_create(lv_obj_t *homepage_scr) {
    if (lv_obj_is_valid(monitor_scr)) {
        lv_obj_del(monitor_scr);
        monitor_scr = NULL;
    }
    monitor_scr = lv_obj_create(NULL);  
    // 复用主模块渐变样式

    lv_style_reset(&monitor_grad_style);
    lv_style_set_bg_color(&monitor_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&monitor_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&monitor_grad_style, LV_GRAD_DIR_VER);
    lv_obj_add_style(monitor_scr, &monitor_grad_style, LV_STATE_DEFAULT);

    // 添加标题“监控模式”
    create_text_label(monitor_scr, "监控模式", &eques_bold_36, 
                     lv_color_hex(0xFFFFFF), 83, 80, LV_OPA_100);

    // 创建“监控模式开关”容器
    lv_obj_t *switch_con = create_container(
        monitor_scr, 48, 150, 928, 83, 
        lv_color_hex(0x192A46), LV_OPA_100, 6, 
        lv_color_hex(0x2E4B7D), 0, LV_OPA_0
    );
    if (switch_con) {
        lv_obj_add_flag(switch_con, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(switch_con, LV_OPA_70, LV_STATE_PRESSED);
    }

    // 开关左侧状态文本
    g_state_label = create_text_label(monitor_scr, "已开启", 
                                     &eques_regular_36, lv_color_hex(0xFFFFFF), 
                                     73, 169, LV_OPA_100);
    lv_label_set_text(g_state_label, g_monitor_state_str[g_monitor_state]);

    // 创建开关（右侧对齐）
    g_switch = lv_switch_create(monitor_scr);
    lv_obj_set_size(g_switch, 60, 37);
    lv_obj_set_pos(g_switch, 884, 170);
    // 开关样式
    lv_obj_set_style_bg_color(g_switch, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(g_switch, lv_color_hex(0x00BDBD), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(g_switch, lv_color_hex(0x192A46), LV_PART_KNOB);
    // 默认状态
    monitor_mode_set_state(g_monitor_state);
    // 绑定开关回调
    lv_obj_add_event_cb(g_switch, monitor_switch_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // 6. 创建“时间段设置”容器
    g_time_con = create_container(
        monitor_scr, 48, 237, 928, 83, 
        lv_color_hex(0x192A46), LV_OPA_100, 6, 
        lv_color_hex(0x2E4B7D), 0, LV_OPA_0
    );
    if (g_time_con) {
        lv_obj_add_flag(g_time_con, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(g_time_con, LV_OPA_70, LV_STATE_PRESSED);
        lv_obj_add_event_cb(g_time_con, time_slot_click_cb, LV_EVENT_CLICKED, NULL);
    }
    
    // 时间段文本
    g_time_slot_label = create_text_label(g_time_con, "1", &eques_regular_24, lv_color_hex(0xFFFFFF), 720, 8, LV_OPA_70);
    lv_label_set_text(g_time_slot_label, monitor_mode_get_slot_str()); // 动态获取初始值

    // // 右侧箭头图标
    // lv_obj_t *arrow_img = create_image_obj(monitor_scr, "D:Vector.png", 712, 308);
    //每天监控时间
    create_text_label(monitor_scr, "每天监控时间段", &eques_regular_36, lv_color_hex(0xFFFFFF), 73, 256, LV_OPA_100);

    // 返回
    lv_obj_t *back_btn = create_text_label
    (monitor_scr, ICON_CHEVORN_LEFT, &fontawesome_icon_32, lv_color_hex(0xFFFFFF), 52, 84, LV_OPA_100);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(back_btn,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn,LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn,monitor_mode_back_btn_click_cb,LV_EVENT_CLICKED,homepage_scr);
    
    // 7. 更新状态栏父对象
    update_status_bar_parent(monitor_scr);
    
    // 8. 切换到子页面
    lv_scr_load(monitor_scr);
}

// 恢复出厂设置 - 重置监控模式所有状态到默认值
void monitor_mode_reset_to_default(void) {
    // 1. 重置监控模式为开启
    monitor_mode_set_state(MONITOR_MODE_ON);

    // 2. 重置时间段为全天
    monitor_mode_set_slot_type(MONITOR_SLOT_FULL_DAY);

    // 3. 重置自定义时间段为默认值 "22:00-6:00"
    // g_slot_strs[MONITOR_SLOT_CUSTOM] = "22:00-6:00";
    strncpy(g_slot_strs[MONITOR_SLOT_CUSTOM], "22:00-6:00", sizeof(g_slot_strs[MONITOR_SLOT_CUSTOM]));

    LV_LOG_USER("Monitor mode reset to default: ON, slot=full day");
}

void monitor_mode_back_btn_click_cb(lv_event_t *e)
{
    lv_obj_t *current_del_scr = lv_disp_get_scr_act(NULL);

    if(!lv_obj_is_valid(current_del_scr)) return;
    if(current_del_scr == monitor_scr) {
        ui_sys_settings_create(current_del_scr);                     // 重建主页
        monitor_mode_destroy();            // 清空所有控件指针
        return;
    }
}
static void monitor_mode_destroy(void)
{
    // 先销毁弹窗！
    if (lv_obj_is_valid(monitor_popup)) {
        lv_obj_del(monitor_popup);
        monitor_popup = NULL;
    }

    // 再销毁页面
    if (lv_obj_is_valid(monitor_scr)) {
        lv_obj_del(monitor_scr);
        monitor_scr = NULL;
    }

    // 清空所有全局控件（必须清，否则野指针）
    start_hour_roller = NULL;
    start_min_roller = NULL;
    end_hour_roller = NULL;
    end_min_roller = NULL;
    g_switch = NULL;
    g_state_label = NULL;
    g_time_con = NULL;
    g_time_slot_label = NULL;
    
    for(int i=0; i<MONITOR_SLOT_MAX; i++){
        g_slot_radios[i] = NULL;
    }
}

