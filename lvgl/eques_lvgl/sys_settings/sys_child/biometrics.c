#include "biometrics.h"
#include "../lv_sys_settings.h"
#include <stdlib.h>  // for rand()
#include <stdio.h>   // for snprintf()
#include <string.h>  // for strncpy/strlen

// 内部状态变量（静态隐藏，仅本文件可见）
static lv_obj_t *bio_scr = NULL;
static bool g_biometric_states[BIOMETRIC_MAX] = {
    true,   // 指纹
    true,   // 密码
    true,   // NFC卡
    true,   // 面容
    true    // 临时密码
};

static char g_temp_pwd[TEMP_PWD_LENGTH + 1] = "123456"; // 默认临时密码
static const char *g_time_slot = "8:00-8:05";           // 默认时间段

// 开关对象数组，用于更新状态
static lv_obj_t *g_switches[BIOMETRIC_MAX] = {NULL};
// ========== 适用时间弹窗相关变量==========
static lv_obj_t *bio_time_popup = NULL;
static lv_obj_t *bio_start_hour_roller = NULL;
static lv_obj_t *bio_start_min_roller = NULL;
static lv_obj_t *bio_end_hour_roller = NULL;
static lv_obj_t *bio_end_min_roller = NULL;
static lv_obj_t *bio_time_label = NULL; // 用于更新主页面时间段文本

lv_style_t bio_grad_style;

// 内部回调函数声明
static void biometric_switch_cb(lv_event_t *e);
static void temp_pwd_btn_cb(lv_event_t *e);
static void time_slot_click_cb(lv_event_t *e);
// ========== 弹窗相关回调 ==========
static void bio_time_popup_close_cb(lv_event_t *e);
static void bio_time_confirm_cb(lv_event_t *e);
// ========== 遮罩层点击判断回调 ==========
static void bio_time_popup_click_cb(lv_event_t *e);

static void bio_destroy(void);
void bio_back_btn_click_cb(lv_event_t *e);
// 子模块初始化
void biometrics_init(void) {
    // 初始化默认状态
    // 可选：初始化随机数种子，避免每次生成相同密码
    srand((unsigned int)lv_tick_get());
}

// 获取生物识别状态
bool biometric_get_state(biometric_type_t type) {
    if (type >= BIOMETRIC_MAX) return false;
    return g_biometric_states[type];
}

// 设置生物识别状态并更新UI
void biometric_set_state(biometric_type_t type, bool state) {
    if (type >= BIOMETRIC_MAX) return;
    g_biometric_states[type] = state;

    // 更新开关状态
    if (g_switches[type] != NULL) {
        if (state) {
            lv_obj_add_state(g_switches[type], LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(g_switches[type], LV_STATE_CHECKED);
        }
    }
}

// 获取临时密码
const char *biometric_get_temp_pwd(void) {
    return g_temp_pwd;
}

// 设置临时密码
void biometric_set_temp_pwd(const char *pwd) {
    if (pwd == NULL || strlen(pwd) != TEMP_PWD_LENGTH) return;
    strncpy(g_temp_pwd, pwd, TEMP_PWD_LENGTH);
    g_temp_pwd[TEMP_PWD_LENGTH] = '\0';
}

// 获取时间段
const char *biometric_get_time_slot(void) {
    return g_time_slot;
}

// 设置时间段
void biometric_set_time_slot(const char *slot) {
    if (slot == NULL) return;
    g_time_slot = slot;
    if (bio_time_label != NULL) {
        lv_label_set_text(bio_time_label, biometric_get_time_slot());
    }
}

// 开关状态变化回调
static void biometric_switch_cb(lv_event_t *e) {
    if (e == NULL) return;
    lv_obj_t *sw = lv_event_get_target(e);
    biometric_type_t type = (biometric_type_t)(intptr_t)lv_event_get_user_data(e);
    biometric_set_state(type, lv_obj_has_state(sw, LV_STATE_CHECKED));
}

// 随机密码按钮回调
static void temp_pwd_btn_cb(lv_event_t *e) {
    if (e == NULL) return;
    // 生成6位随机数字密码
    char new_pwd[TEMP_PWD_LENGTH + 1];
    for (int i = 0; i < TEMP_PWD_LENGTH; i++) {
        // 正确调用 lv_rand：生成 0~9 的随机整数
        new_pwd[i] = '0' + lv_rand(0, 9);
    }
    new_pwd[TEMP_PWD_LENGTH] = '\0';
    biometric_set_temp_pwd(new_pwd);

    // 更新密码显示标签
    lv_obj_t *pwd_label = lv_event_get_user_data(e);
    if (pwd_label != NULL) {
        lv_label_set_text(pwd_label, biometric_get_temp_pwd());
    }
}

static void time_slot_click_cb(lv_event_t *e) {
    if (e == NULL) return;
    LV_LOG_USER("time_slot_click_cb");

    // 1. 创建全屏遮罩层
    bio_time_popup = lv_obj_create(lv_scr_act());
    lv_obj_set_size(bio_time_popup, LV_HOR_RES, 800);
    lv_obj_set_pos(bio_time_popup, 0, 0);
    lv_obj_set_style_bg_color(bio_time_popup, lv_color_hex(0x000000), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(bio_time_popup, LV_OPA_50, LV_STATE_DEFAULT); // 半透明黑色
    //设置边框透明度为0
    lv_obj_set_style_border_opa(bio_time_popup, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(bio_time_popup, LV_OBJ_FLAG_CLICKABLE);
    // 绑定遮罩层点击判断回调（仅点击外部关闭）
    lv_obj_add_event_cb(bio_time_popup, bio_time_popup_click_cb, LV_EVENT_CLICKED, NULL);

    // 2. 创建底部时间选择器容器
    lv_obj_t *time_picker = lv_obj_create(bio_time_popup);
    lv_obj_set_size(time_picker, LV_HOR_RES, 400); // 下半屏
    lv_obj_set_align(time_picker, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_bg_color(time_picker, lv_color_hex(0xE0EDFF), LV_STATE_DEFAULT);
    lv_obj_set_style_radius(time_picker, 36, LV_STATE_DEFAULT); // 顶部圆角
    lv_obj_set_style_pad_top(time_picker, 20, LV_STATE_DEFAULT);
    lv_obj_add_flag(time_picker, LV_OBJ_FLAG_CLICKABLE); // 避免事件透传

    // 3. 创建标题栏（和消息通知样式一致）
    lv_obj_t *header = lv_obj_create(time_picker);
    lv_obj_set_size(header, LV_HOR_RES, 60);
    lv_obj_set_align(header, LV_ALIGN_TOP_MID);
    lv_obj_set_style_bg_opa(header, LV_OPA_0, LV_STATE_DEFAULT); // 透明背景
    lv_obj_set_style_border_width(header, 0, LV_STATE_DEFAULT);

    // 标题栏分割线
    lv_obj_t *header1 = lv_obj_create(time_picker);
    lv_obj_set_size(header1, 710, 5);
    lv_obj_set_pos(header1, 30, 60);
    lv_obj_set_style_bg_color(header1, lv_color_hex(0x000000), LV_STATE_DEFAULT);

    // 取消按钮（和消息通知样式一致）
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
    lv_obj_add_event_cb(cancel_btn, bio_time_popup_close_cb, LV_EVENT_CLICKED, NULL);

    // 标题（改为“Applicable Time”）
    // lv_obj_t *title_label = create_text_label(header, "bio_time", 
    //                                          &eques_regular_36, lv_color_hex(0x192A46), 
    //                                          0, 0, LV_OPA_100);
    // lv_label_set_text(title_label, "Applicable Time");
    // lv_obj_set_align(title_label, LV_ALIGN_CENTER);

    // 确定按钮（和消息通知样式一致）
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
    lv_obj_add_event_cb(confirm_btn, bio_time_confirm_cb, LV_EVENT_CLICKED, NULL);

    // 4. 创建时间滚轮（开始时间和结束时间，和消息通知完全一致）
    // 小时选项：00-23
    static const char *hour_opts = "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23";
    // 分钟选项：00-59
    static const char *min_opts = 
    "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n"
    "10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n"
    "20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n"
    "30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n"
    "40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n"
    "50\n51\n52\n53\n54\n55\n56\n57\n58\n59";

    // 解析当前时间段，设置滚轮默认选中项
    int start_h, start_m, end_h, end_m;
    sscanf(g_time_slot, "%d:%d-%d:%d", &start_h, &start_m, &end_h, &end_m);

    // 开始时间容器
    lv_obj_t *start_time_cont = lv_obj_create(time_picker);
    lv_obj_set_size(start_time_cont, LV_HOR_RES / 2 - 20, 300);
    lv_obj_set_align(start_time_cont, LV_ALIGN_LEFT_MID);
    lv_obj_set_pos(start_time_cont, 10, 10);
    lv_obj_set_style_bg_opa(start_time_cont, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(start_time_cont, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(start_time_cont, 0, LV_STATE_DEFAULT);

    lv_obj_t *start_label = create_text_label(start_time_cont, "开始时间", 
                                             &eques_regular_36, lv_color_hex(0x9E9E9E), 
                                             0, 20, LV_OPA_100);
    lv_label_set_text(start_label, "开始时间");
    lv_obj_set_align(start_label, LV_ALIGN_TOP_MID);

    // 开始小时滚轮
    bio_start_hour_roller = lv_roller_create(start_time_cont);
    lv_roller_set_options(bio_start_hour_roller, hour_opts, LV_ROLLER_MODE_NORMAL);
    lv_obj_set_size(bio_start_hour_roller, 95, 180);
    lv_obj_set_align(bio_start_hour_roller, LV_ALIGN_LEFT_MID);
    lv_obj_set_pos(bio_start_hour_roller, 75, 30);
    lv_roller_set_selected(bio_start_hour_roller, start_h, LV_ANIM_OFF);
    lv_obj_set_style_text_font(bio_start_hour_roller, &eques_regular_24, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(bio_start_hour_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(bio_start_hour_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(bio_start_hour_roller, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(bio_start_hour_roller, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(bio_start_hour_roller, lv_color_hex(0x999999), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(bio_start_hour_roller, lv_color_hex(0x000000), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(bio_start_hour_roller, LV_OPA_TRANSP, LV_STATE_DEFAULT);

    // 开始分钟滚轮
    bio_start_min_roller = lv_roller_create(start_time_cont);
    lv_roller_set_options(bio_start_min_roller, min_opts, LV_ROLLER_MODE_NORMAL);
    lv_obj_set_size(bio_start_min_roller, 95, 180);
    lv_obj_set_align(bio_start_min_roller, LV_ALIGN_RIGHT_MID);
    lv_obj_set_pos(bio_start_min_roller, -75, 30);
    lv_roller_set_selected(bio_start_min_roller, start_m, LV_ANIM_OFF); // 修复：去掉/10，适配完整分钟
    lv_obj_set_style_text_font(bio_start_min_roller, &eques_regular_24, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(bio_start_min_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(bio_start_min_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(bio_start_min_roller, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(bio_start_min_roller, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(bio_start_min_roller, lv_color_hex(0x999999), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(bio_start_min_roller, lv_color_hex(0x000000), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(bio_start_min_roller, LV_OPA_TRANSP, LV_STATE_DEFAULT);

    // 结束时间容器
    lv_obj_t *end_time_cont = lv_obj_create(time_picker);
    lv_obj_set_size(end_time_cont, LV_HOR_RES / 2 - 20, 300);
    lv_obj_set_align(end_time_cont, LV_ALIGN_RIGHT_MID);
    lv_obj_set_pos(end_time_cont, -10, 10);
    lv_obj_set_style_bg_opa(end_time_cont, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(end_time_cont, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(end_time_cont, 0, LV_STATE_DEFAULT);

    lv_obj_t *end_label = create_text_label(end_time_cont, "结束时间", 
                                           &eques_regular_36, lv_color_hex(0x9E9E9E), 
                                           0, 20, LV_OPA_100);
    lv_label_set_text(end_label, "结束时间");
    lv_obj_set_align(end_label, LV_ALIGN_TOP_MID);

    // 结束小时滚轮
    bio_end_hour_roller = lv_roller_create(end_time_cont);
    lv_roller_set_options(bio_end_hour_roller, hour_opts, LV_ROLLER_MODE_NORMAL);
    lv_obj_set_size(bio_end_hour_roller, 95, 180);
    lv_obj_set_align(bio_end_hour_roller, LV_ALIGN_LEFT_MID);
    lv_obj_set_pos(bio_end_hour_roller, 75, 30);
    lv_roller_set_selected(bio_end_hour_roller, end_h, LV_ANIM_OFF);
    lv_obj_set_style_text_font(bio_end_hour_roller, &eques_regular_24, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(bio_end_hour_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(bio_end_hour_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(bio_end_hour_roller, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(bio_end_hour_roller, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(bio_end_hour_roller, lv_color_hex(0x999999), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(bio_end_hour_roller, lv_color_hex(0x000000), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(bio_end_hour_roller, LV_OPA_TRANSP, LV_STATE_DEFAULT);

    // 结束分钟滚轮
    bio_end_min_roller = lv_roller_create(end_time_cont);
    lv_roller_set_options(bio_end_min_roller, min_opts, LV_ROLLER_MODE_NORMAL);
    lv_obj_set_size(bio_end_min_roller, 95, 180);
    lv_obj_set_align(bio_end_min_roller, LV_ALIGN_RIGHT_MID);
    lv_obj_set_pos(bio_end_min_roller, -75, 30);
    lv_roller_set_selected(bio_end_min_roller, end_m, LV_ANIM_OFF); // 修复：去掉/10
    lv_obj_set_style_text_font(bio_end_min_roller, &eques_regular_24, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(bio_end_min_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(bio_end_min_roller, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(bio_end_min_roller, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(bio_end_min_roller, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(bio_end_min_roller, lv_color_hex(0x999999), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(bio_end_min_roller, lv_color_hex(0x000000), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(bio_end_min_roller, LV_OPA_TRANSP, LV_STATE_DEFAULT);
}

// ========== 新增：遮罩层点击判断回调（仅外部关闭） ==========
static void bio_time_popup_click_cb(lv_event_t *e) {
    if (e == NULL || bio_time_popup == NULL) return;
    
    // 获取实际点击的目标对象
    lv_obj_t *clicked_obj = lv_event_get_target(e);
    // 获取弹窗主体（time_picker是bio_time_popup的第一个子控件）
    lv_obj_t *time_picker = lv_obj_get_child(bio_time_popup, 0);
    
    // 核心判断：点击外部才关闭
    if (lv_obj_is_valid(time_picker)  && clicked_obj != time_picker) {
        bio_time_popup_close_cb(e);
    }
}

// ========== 新增：弹窗关闭回调 ==========
static void bio_time_popup_close_cb(lv_event_t *e) {
    if (bio_time_popup != NULL) {
        lv_obj_del(bio_time_popup);
        bio_time_popup = NULL;
        bio_start_hour_roller = NULL;
        bio_start_min_roller = NULL;
        bio_end_hour_roller = NULL;
        bio_end_min_roller = NULL;
    }
}

// ========== 新增：确定按钮回调 ==========
static void bio_time_confirm_cb(lv_event_t *e) {
    // 获取滚轮选择的时间
    char start_hour[3], start_min[3], end_hour[3], end_min[3];
    lv_roller_get_selected_str(bio_start_hour_roller, start_hour, sizeof(start_hour));
    lv_roller_get_selected_str(bio_start_min_roller, start_min, sizeof(start_min));
    lv_roller_get_selected_str(bio_end_hour_roller, end_hour, sizeof(end_hour));
    lv_roller_get_selected_str(bio_end_min_roller, end_min, sizeof(end_min));

    // 格式化新的时间段字符串
    static char new_time_slot[16];
    snprintf(new_time_slot, sizeof(new_time_slot), "%s:%s-%s:%s",
             start_hour, start_min, end_hour, end_min);
    biometric_set_time_slot(new_time_slot);

    // 关闭弹窗
    bio_time_popup_close_cb(e);
}


// 创建生物识别设置子页面
void ui_biometrics_settings_create(lv_obj_t *homepage_scr) {
    
    // // 1. 创建子页面对象
    bio_scr = lv_obj_create(NULL);  

    // 2. 复用主模块渐变样式
    lv_style_reset(&bio_grad_style);
    lv_style_set_bg_color(&bio_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&bio_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&bio_grad_style, LV_GRAD_DIR_VER);
    lv_obj_add_style(bio_scr, &bio_grad_style, LV_STATE_DEFAULT);

    // 3. 添加标题“生物识别”
    create_text_label(bio_scr, "生物识别", &eques_bold_36, 
                     lv_color_hex(0xFFFFFF), 83, 80, LV_OPA_100);
    // lv_label_set_text(lv_obj_get_child(bio_scr, 0), "生物识别");

    // 生物识别选项列表
    const char *bio_names[] = {"指纹", "密码", "NFC卡", "面容"};

    int y_pos = 150,y_switch = 170,y_label = 169;
    for (int i = 0; i < 4; i++) {
        // 创建选项容器
        lv_obj_t *con = create_container(
            bio_scr, 48, y_pos, 928, 83, 
            lv_color_hex(0x192A46), LV_OPA_100, 6, 
            lv_color_hex(0x2E4B7D), 0, LV_OPA_0
        );
        if (con == NULL) continue;

        // 选项名称
        lv_obj_t *name_label = create_text_label(bio_scr, "生物识别", &eques_regular_36, lv_color_hex(0xFFFFFF), 80, y_label, LV_OPA_100);
        lv_label_set_text(name_label, bio_names[i]);


        // 创建开关（右侧对齐）
        g_switches[i] = lv_switch_create(bio_scr);
        lv_obj_set_size(g_switches[i], 60, 37);
        lv_obj_set_pos(g_switches[i], 885, y_switch);
        // 开关样式
        lv_obj_set_style_bg_color(g_switches[i], lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(g_switches[i], lv_color_hex(0x00BDBD), LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_set_style_bg_color(g_switches[i], lv_color_hex(0x020A17), LV_PART_KNOB);
        // ========== 修复：lv_obj_set_style_size 只传1个尺寸值（宽高相同）=========
        lv_obj_set_style_size(g_switches[i], 28, LV_PART_KNOB);
        // 默认状态
        biometric_set_state(i, g_biometric_states[i]);
        // 绑定开关回调
        lv_obj_add_event_cb(g_switches[i], biometric_switch_cb, LV_EVENT_VALUE_CHANGED, (void *)(intptr_t)i);
        y_switch += 87,y_pos += 87,y_label += 87;
    }
    create_text_label(bio_scr, "开启后，才能启用这种方式进行开锁", &eques_regular_24, lv_color_hex(0xFFFFFF), 80, 504, LV_OPA_70);

    // 临时密码输入区域
    lv_obj_t *pwd_con = create_container(
        bio_scr, 48, 530, 928, 258, 
        lv_color_hex(0x192A46), LV_OPA_100, 6, 
        lv_color_hex(0x2E4B7D), 0, LV_OPA_0
    );
    // 密码提示文本
     create_text_label(bio_scr, "临时密码", &eques_regular_36, lv_color_hex(0xFFFFFF), 80, 542, LV_OPA_100);
    if (pwd_con) {
        // 密码显示标签
        lv_obj_t *pwd_label = create_text_label(bio_scr, "临时密码", &eques_regular_36, lv_color_hex(0x000000), 80, 620, LV_OPA_100);
        lv_label_set_text(pwd_label, biometric_get_temp_pwd());
        lv_obj_set_style_bg_color(pwd_label, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(pwd_label, LV_OPA_100, LV_STATE_DEFAULT);
        lv_obj_set_size(pwd_label, 675, 66);
        lv_obj_set_style_border_width(pwd_label, 0, LV_STATE_DEFAULT);
        lv_obj_set_style_outline_width(pwd_label, 0, LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(pwd_label, 10, LV_STATE_DEFAULT); // 顶部留一点边距
        lv_obj_set_style_pad_left(pwd_label, 26, LV_STATE_DEFAULT); // 左侧留一点边距
        // 字母间距
        lv_obj_set_style_text_letter_space(pwd_label, 85, LV_STATE_DEFAULT);
        // 随机按钮
        lv_obj_t *rand_btn = lv_btn_create(bio_scr);
        lv_obj_set_size(rand_btn, 244, 66);
        lv_obj_set_pos(rand_btn, 700, 620);
        lv_obj_set_style_radius(rand_btn, 0, LV_STATE_DEFAULT);
        // 随机按钮边框透明
        lv_obj_set_style_border_width(rand_btn, 0, LV_STATE_DEFAULT);
        lv_obj_set_style_outline_width(rand_btn, 0, LV_STATE_DEFAULT);
        
        lv_obj_set_style_bg_color(rand_btn, lv_color_hex(0x9BC4FF), LV_STATE_DEFAULT);

        lv_obj_t *rand_label = create_text_label(rand_btn, "随机", &eques_regular_36, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
        lv_obj_set_align(rand_label, LV_ALIGN_CENTER);
        // lv_label_set_text(rand_label, "random");
        lv_obj_add_event_cb(rand_btn, temp_pwd_btn_cb, LV_EVENT_CLICKED, pwd_label);
    }
    create_text_label(bio_scr, "设置六位数数字密码用于临时开锁", &eques_regular_24, lv_color_hex(0xFFFFFF), 80, 695, LV_OPA_70);
     create_text_label(bio_scr, "适用时间", &eques_regular_36, lv_color_hex(0xFFFFFF), 80, 727, LV_OPA_100);

    int switch_idx = 4; // 对应临时密码的枚举值 BIOMETRIC_TEMP_PWD
    lv_obj_t *time_switch = lv_switch_create(bio_scr);
    lv_obj_set_size(time_switch, 60, 37);
    lv_obj_set_pos(time_switch, 885, 557);
    // 开关样式
    lv_obj_set_style_bg_color(time_switch, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(time_switch, lv_color_hex(0x00BDBD), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(time_switch, lv_color_hex(0x020A17), LV_PART_KNOB);
    lv_obj_set_style_size(time_switch, 28, LV_PART_KNOB);
    g_switches[switch_idx] = time_switch;
    // 先设置状态（会自动同步到开关UI）
    biometric_set_state(switch_idx, g_biometric_states[switch_idx]);
    // 绑定开关回调（直接用switch_idx，不用临时变量y）
    lv_obj_add_event_cb(time_switch, biometric_switch_cb, LV_EVENT_VALUE_CHANGED, (void *)(intptr_t)switch_idx);

    // 适用时间段设置项
    lv_obj_t *time_con = create_container(
        bio_scr, 700, 727, 200, 40, 
        lv_color_hex(0x192A46), LV_OPA_100, 6, 
        lv_color_hex(0x2E4B7D), 0, LV_OPA_0
    );
    if (time_con) {
        lv_obj_add_flag(time_con, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(time_con, LV_OPA_70, LV_STATE_PRESSED);
        lv_obj_add_event_cb(time_con, time_slot_click_cb, LV_EVENT_CLICKED, NULL);

        // 时间段文本
        bio_time_label  = create_text_label(bio_scr, "1", 
                                                &eques_regular_24, lv_color_hex(0xFFFFFF), 
                                                810, 737, LV_OPA_70);
        char time_text[64];
        // 时间段文本
        snprintf(time_text, sizeof(time_text), "%s", biometric_get_time_slot());
        lv_label_set_text(bio_time_label, time_text);

        // // 右侧箭头图标
        // lv_obj_t *arrow_img = create_image_obj(bio_scr, "D:Vector.png", 713, 786);
    }

    // 返回
    lv_obj_t *back_btn = create_text_label
    (bio_scr, ICON_CHEVORN_LEFT, &fontawesome_icon_32, lv_color_hex(0xFFFFFF), 52, 84, LV_OPA_100);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(back_btn,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn,LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn,bio_back_btn_click_cb,LV_EVENT_CLICKED,homepage_scr);

    // 7. 更新状态栏父对象
    update_status_bar_parent(bio_scr);
    // 8. 切换到子页面
    lv_scr_load(bio_scr);
}

// 恢复出厂设置 - 重置所有生物识别状态到默认值
void biometrics_reset_to_default(void) {
    // 1. 重置所有开关状态为默认开启（true）
    for (int i = 0; i < BIOMETRIC_MAX; i++) {
        g_biometric_states[i] = true;
        if (g_switches[i] != NULL) {
            lv_obj_add_state(g_switches[i], LV_STATE_CHECKED);
        }
    }

    // 2. 重置临时密码为默认值 "123456"
    strncpy(g_temp_pwd, "123456", TEMP_PWD_LENGTH);
    g_temp_pwd[TEMP_PWD_LENGTH] = '\0';

    // 3. 重置时间段为默认值 "8:00-8:05"
    g_time_slot = "8:00-8:05";
    // 修复：调用set方法更新UI
    biometric_set_time_slot(g_time_slot);

    LV_LOG_USER("Biometrics reset to default: all switches ON, temp pwd=123456, time slot=8:00-8:05");
}
// 返回按钮回调（安全版：先销毁自己，再加载父页面）
void bio_back_btn_click_cb(lv_event_t *e)
{
    lv_obj_t *current_del_scr = lv_disp_get_scr_act(NULL);

    if(!lv_obj_is_valid(current_del_scr)) return;
    if(current_del_scr == bio_scr) {
        ui_sys_settings_create(current_del_scr);                     // 重建主页
        bio_destroy();            // 清空所有控件指针
        return;
    }
}
// 销毁生物识别页面 → 彻底释放内存
void bio_destroy(void)
{
    // 1. 先销毁弹窗
    if (lv_obj_is_valid(bio_time_popup)) {
        lv_obj_del(bio_time_popup);
        bio_time_popup = NULL;
    }

    // 2. 销毁主页面
    if (lv_obj_is_valid(bio_scr)) {
        lv_obj_del(bio_scr);
        bio_scr = NULL;
    }

    // 3. 清空所有控件指针（杜绝野指针）
    for (int i = 0; i < BIOMETRIC_MAX; i++) {
        g_switches[i] = NULL;
    }

    // 4. 弹窗相关指针清空
    bio_start_hour_roller = NULL;
    bio_start_min_roller = NULL;
    bio_end_hour_roller = NULL;
    bio_end_min_roller = NULL;
    bio_time_label = NULL;
}