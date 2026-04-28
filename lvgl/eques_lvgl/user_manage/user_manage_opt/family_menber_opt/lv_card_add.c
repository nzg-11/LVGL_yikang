/**
 * @file lv_card_add.c
 * @brief 卡片录入界面实现
 * @details 包含卡片录入UI、5秒超时检测、名称输入弹窗、成功/失败提示、界面切换
 * @note 依赖 LVGL 图形库、生物特征录入管理模块
 */

/*********************
 * 头文件包含
 *********************/
#include <string.h>
#include "lv_card_add.h"
#include "lv_a_enroll_opt.h"  // 生物特征录入回调头文件

/*********************
 * 宏定义
 *********************/
#define MAX_CARD_TIMEOUT_MS  5000    // 卡片录入超时时间(5秒)
#define CARD_NAME_MAX_LEN    8       // 卡片名称最大长度

/*********************
 * 全局静态变量
 *********************/
// 界面根对象
static lv_obj_t *card_add_scr = NULL;

// 样式相关
static lv_style_t  card_add_grad_style;
static bool        card_add_style_inited = false;

// 定时器相关
static lv_timer_t *card_timeout_timer = NULL;
static lv_obj_t   *card_timeout_enroll_scr = NULL;

// 弹窗遮罩层
static lv_obj_t *card_bg_mask_layer = NULL;

// 录入成功弹窗组件
static lv_obj_t *card_custom_popup = NULL;
static lv_obj_t *card_name_keyboard = NULL;
static lv_obj_t *card_input_textarea = NULL;

// 录入失败弹窗组件
static lv_obj_t *card_fail_popup = NULL;

/*********************
 * 静态函数前置声明
 *********************/
// 定时器管理
static void del_card_timeout_timer(void);

// 弹窗关闭
static void close_card_popup(void);
static void close_card_fail_popup(void);
void ui_card_add_destroy(void);
// 弹窗回调
static void card_fail_confirm_cb(lv_event_t *e);
static void card_timeout_timer_cb(lv_timer_t *timer);
static void card_popup_confirm_cb(lv_event_t *e);

// 键盘与输入框
static void hide_card_keyboard(lv_event_t *e);
static void card_input_click_cb(lv_event_t *e);

// UI创建
static void init_card_add_styles(void);
static void create_card_complete_popup(lv_obj_t *enroll_scr);

// 按钮回调
static void card_confirm_btn_click_cb(lv_event_t *e);
static void card_add_back_btn_click_cb(lv_event_t *e);
/*********************
 * 函数实现
 *********************/

/**
 * @brief 创建卡片录入主界面
 * @param enroll_scr 父界面对象
 */
void ui_card_add_create(lv_obj_t *enroll_scr)
{
    init_card_add_styles();

    if (enroll_scr == NULL) {
        LV_LOG_WARN("ui_card_add_create: enroll_scr is NULL!");
        return;
    }

    // 清理所有已有资源
    del_card_timeout_timer();
    close_card_popup();
    close_card_fail_popup();

    // 销毁旧界面
    if (lv_obj_is_valid(card_add_scr)) {
        lv_obj_del(card_add_scr);
        card_add_scr = NULL;
    }

    // ===================== 创建界面根容器 =====================
    card_add_scr = lv_obj_create(NULL);
    lv_obj_clear_flag(card_add_scr, LV_OBJ_FLAG_HIDDEN);

    // 设置背景渐变样式
    lv_style_reset(&card_add_grad_style);
    lv_style_set_bg_color(&card_add_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&card_add_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&card_add_grad_style, LV_GRAD_DIR_VER);
    lv_style_set_bg_main_stop(&card_add_grad_style, 0);
    lv_style_set_bg_grad_stop(&card_add_grad_style, 255);
    lv_obj_add_style(card_add_scr, &card_add_grad_style, LV_STATE_DEFAULT);

    // ===================== UI元素绘制 =====================
    // 标题
    create_text_label(card_add_scr, "添加卡片", &eques_bold_36, lv_color_hex(0xFFFFFF), 83, 80, LV_OPA_100);

    // 提示文本 (修复原代码指纹错误)
    lv_obj_t *prompt_label = create_text_label(card_add_scr, "正在录入", &eques_regular_32, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align(prompt_label, LV_ALIGN_TOP_MID, 0, 508);

    // 圆形装饰UI
    create_container(card_add_scr, 372, 160, 281, 281,
        lv_color_hex(0x1D3740), LV_OPA_100, 200,
        lv_color_hex(0x005CAA), 20, LV_OPA_100);

    create_container(card_add_scr, 405, 192, 216, 216,
        lv_color_hex(0x061022), LV_OPA_0, 200,
        lv_color_hex(0xFFFFFF), 18, LV_OPA_100);

    lv_obj_t *card_circle03 = create_custom_gradient_container(
        card_add_scr, 422, 210, 182, 182, 100,
        0xB1C6D2, 0x1D86BF, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100
    );
    lv_obj_set_style_border_width(card_circle03, 4, LV_PART_MAIN);
    lv_obj_set_style_border_color(card_circle03, lv_color_hex(0x346E8D), LV_PART_MAIN);

    create_container(card_add_scr, 380, 168, 264, 264, lv_color_hex(0x1D3740), LV_OPA_0, 200, lv_color_hex(0xFFFFFF), 2, LV_OPA_100);

    // 确认按钮
    lv_obj_t *confirm_enter_btn = create_text_label(card_add_scr, ICON_CARD, &iconfont_icon_90, lv_color_hex(0xFFFFFF), 440, 259, LV_OPA_100);
    lv_obj_add_flag(confirm_enter_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(confirm_enter_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(confirm_enter_btn, card_confirm_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);

    // 返回按钮
    lv_obj_t *back_btn = create_text_label
    (card_add_scr, ICON_CHEVORN_LEFT, &fontawesome_icon_32, lv_color_hex(0xFFFFFF), 52, 84, LV_OPA_100);
    
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn, card_add_back_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);

    // ===================== 界面加载与定时器 =====================
    update_status_bar_parent(card_add_scr);
    lv_scr_load(card_add_scr);

    // 启动录入超时定时器
    card_timeout_enroll_scr = enroll_scr;
    card_timeout_timer = lv_timer_create(card_timeout_timer_cb, MAX_CARD_TIMEOUT_MS, NULL);
}



/**
 * @brief 销毁卡片录入超时定时器
 */
static void del_card_timeout_timer(void)
{
    if (card_timeout_timer != NULL) {
        lv_timer_del(card_timeout_timer);
        card_timeout_timer = NULL;
    }
    card_timeout_enroll_scr = NULL;
}

/**
 * @brief 关闭卡片录入成功弹窗
 */
static void close_card_popup(void)
{
    // 隐藏键盘
    if (lv_obj_is_valid(card_name_keyboard)) {
        lv_obj_add_flag(card_name_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
    // 隐藏遮罩层
    if (lv_obj_is_valid(card_bg_mask_layer)) {
        lv_obj_add_flag(card_bg_mask_layer, LV_OBJ_FLAG_HIDDEN);
    }
    // 销毁弹窗主体
    if (lv_obj_is_valid(card_custom_popup)) {
        lv_obj_del(card_custom_popup);
        card_custom_popup = NULL;
    }
}

/**
 * @brief 关闭卡片录入失败弹窗
 */
static void close_card_fail_popup(void)
{
    // 销毁失败弹窗
    if (lv_obj_is_valid(card_fail_popup)) {
        lv_obj_del(card_fail_popup);
        card_fail_popup = NULL;
    }
    // 隐藏遮罩层
    if (lv_obj_is_valid(card_bg_mask_layer)) {
        lv_obj_add_flag(card_bg_mask_layer, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief 失败弹窗确认按钮回调
 * @param e LVGL事件对象
 */
static void card_fail_confirm_cb(lv_event_t *e)
{
    if (e == NULL) return;

    //common_member_info_t *member = get_current_enroll_member();
    lv_obj_t *parent_scr = lv_event_get_user_data(e);

    // 关闭弹窗并返回录入界面
    close_card_fail_popup();
    //ui_enroll_create(member, parent_scr);
    lv_scr_load(parent_scr);
    // 销毁卡片录入界面
    if (lv_obj_is_valid(card_add_scr)) {
        lv_obj_del(card_add_scr);
        card_add_scr = NULL;
    }
}

/**
 * @brief 卡片录入超时定时器回调
 * @param timer LVGL定时器对象
 */
static void card_timeout_timer_cb(lv_timer_t *timer)
{
    lv_obj_t *enroll_scr = card_timeout_enroll_scr;
    del_card_timeout_timer();

    // 校验界面有效性
    if (!lv_obj_is_valid(card_add_scr)) {
        return;
    }

    // ===================== 创建遮罩层 =====================
    if (!lv_obj_is_valid(card_bg_mask_layer)) {
        card_bg_mask_layer = lv_obj_create(card_add_scr);
        lv_obj_set_size(card_bg_mask_layer, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_pos(card_bg_mask_layer, 0, 0);
        lv_obj_set_style_bg_color(card_bg_mask_layer, lv_color_hex(0x000000), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(card_bg_mask_layer, LV_OPA_70, LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(card_bg_mask_layer, 0, LV_STATE_DEFAULT);
        lv_obj_add_flag(card_bg_mask_layer, LV_OBJ_FLAG_CLICKABLE);
    }
    lv_obj_clear_flag(card_bg_mask_layer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(card_bg_mask_layer);

    // ===================== 创建失败弹窗 =====================
    card_fail_popup = create_container(
        card_add_scr, 212, 150, 600, 200,
        lv_color_hex(0xE0EDFF), LV_OPA_100,
        16, lv_color_hex(0x1F3150), 0, LV_OPA_90
    );
    lv_obj_set_style_pad_all(card_fail_popup, 0, LV_STATE_DEFAULT);
    lv_obj_move_foreground(card_fail_popup);

    // 失败提示文本
    lv_obj_t *fail_label = create_text_label(
        card_fail_popup, "添加失败", &eques_regular_24,
        lv_color_hex(0xFF0000), 0, 38, LV_OPA_100
    );
    lv_obj_align(fail_label, LV_ALIGN_TOP_MID, 0, 38);

    // 确认按钮
    lv_obj_t *fail_confirm_btn = create_custom_gradient_container(
        card_fail_popup, 197, 120, 205, 44, 6,
        0X006BDC, 0x00BDBD, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100
    );
    lv_obj_add_flag(fail_confirm_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(fail_confirm_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_set_style_pad_all(fail_confirm_btn, 0, LV_STATE_DEFAULT);

    lv_obj_t *fail_confirm_label = create_text_label(fail_confirm_btn, "确认并返回",&eques_bold_24, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_set_align(fail_confirm_label, LV_ALIGN_CENTER);
    lv_obj_add_event_cb(fail_confirm_btn, card_fail_confirm_cb, LV_EVENT_CLICKED, enroll_scr);
}

/**
 * @brief 隐藏卡片名称输入键盘
 * @param e LVGL事件对象
 */
static void hide_card_keyboard(lv_event_t *e)
{
    if (lv_obj_is_valid(card_name_keyboard)) {
        lv_obj_add_flag(card_name_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief 卡片名称输入框点击回调(弹出键盘)
 * @param e LVGL事件对象
 */
static void card_input_click_cb(lv_event_t *e)
{
    if (e == NULL) return;
    lv_obj_t *input = lv_event_get_target(e);

    // 创建键盘(不存在则新建)
    if (!lv_obj_is_valid(card_name_keyboard)) {
        card_name_keyboard = lv_keyboard_create(lv_scr_act());
        lv_obj_set_style_bg_color(card_name_keyboard, lv_color_hex(0x192A46), LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(card_name_keyboard, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(card_name_keyboard, lv_color_hex(0x1F3150), LV_STATE_DEFAULT);
        lv_obj_set_style_radius(card_name_keyboard, 6, LV_STATE_DEFAULT);
        lv_obj_set_size(card_name_keyboard, LV_HOR_RES, 250);
        lv_obj_align(card_name_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_add_event_cb(card_name_keyboard, hide_card_keyboard, LV_EVENT_READY, NULL);
    }

    // 显示键盘并绑定输入框
    lv_obj_clear_flag(card_name_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(card_name_keyboard, input);
    lv_obj_move_foreground(card_name_keyboard);
}

/**
 * @brief 卡片名称输入确认按钮回调
 * @param e LVGL事件对象
 */
static void card_popup_confirm_cb(lv_event_t *e)
{
    if (e == NULL) return;

    //common_member_info_t *member = get_current_enroll_member();
    lv_obj_t *parent_scr = lv_event_get_user_data(e);
    const char *card_name = lv_textarea_get_text(card_input_textarea);

    // 名称为空则使用默认值
    if (card_name == NULL || strlen(card_name) == 0) {
        card_name = "";
    }

    // 通知录入完成
    card_enroll_complete(card_name);
    close_card_popup();

    // 返回录入主界面
    //ui_enroll_create(member, parent_scr);
    lv_scr_load(parent_scr);
    // 销毁当前卡片界面
    if (lv_obj_is_valid(card_add_scr)) {
        lv_obj_del(card_add_scr);
        card_add_scr = NULL;
    }
}

/**
 * @brief 初始化卡片界面全局样式
 */
static void init_card_add_styles(void)
{
    if (!card_add_style_inited) {
        lv_style_init(&card_add_grad_style);
        card_add_style_inited = true;
    }
}

/**
 * @brief 创建卡片录入成功弹窗(输入名称)
 * @param enroll_scr 父界面对象
 */
static void create_card_complete_popup(lv_obj_t *enroll_scr)
{
    // 清理已有资源
    close_card_popup();
    del_card_timeout_timer();

    // ===================== 创建遮罩层 =====================
    card_bg_mask_layer = lv_obj_create(card_add_scr);
    lv_obj_set_size(card_bg_mask_layer, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(card_bg_mask_layer, 0, 0);
    lv_obj_set_style_bg_color(card_bg_mask_layer, lv_color_hex(0x000000), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(card_bg_mask_layer, LV_OPA_70, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(card_bg_mask_layer, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(card_bg_mask_layer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(card_bg_mask_layer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(card_bg_mask_layer, hide_card_keyboard, LV_EVENT_CLICKED, NULL);
    // ===================== 创建成功弹窗 =====================
    card_custom_popup = create_container(
        card_add_scr, 212, 150, 600, 270,
        lv_color_hex(0xE0EDFF), LV_OPA_100,
        16, lv_color_hex(0x1F3150), 0, LV_OPA_90
    );
    lv_obj_set_style_pad_all(card_custom_popup, 0, LV_STATE_DEFAULT);

    // 标题文本
    lv_obj_t *succeed_add_label = create_text_label(
        card_custom_popup, "添加成功", &eques_regular_24,
        lv_color_hex(0x000000), 0, 38, LV_OPA_100
    );
    lv_obj_align(succeed_add_label, LV_ALIGN_TOP_MID, 0, 38);

    // 卡片名称输入框
    card_input_textarea = lv_textarea_create(card_custom_popup);
    lv_obj_clear_flag(card_input_textarea, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(card_input_textarea, 382, 44);
    lv_obj_set_pos(card_input_textarea, 137, 90);
    lv_obj_set_style_bg_color(card_input_textarea, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(card_input_textarea, lv_color_hex(0x333333), LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(card_input_textarea, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(card_input_textarea, 6, LV_STATE_DEFAULT);
    //lv_textarea_set_placeholder_text(card_input_textarea, "please input card name");
    lv_textarea_set_max_length(card_input_textarea, CARD_NAME_MAX_LEN);
    lv_textarea_set_one_line(card_input_textarea, true);
    //lv_obj_set_style_text_font(card_input_textarea, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    lv_obj_add_flag(card_input_textarea, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(card_input_textarea, LV_OPA_80, LV_STATE_PRESSED);
    //lv_obj_add_event_cb(card_input_textarea, card_input_click_cb, LV_EVENT_CLICKED, NULL);

    // 确认按钮
    lv_obj_t *confirm_btn = create_custom_gradient_container(
        card_custom_popup, 197, 171, 205, 44, 6,
        0X006BDC, 0x00BDBD, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100
    );
    lv_obj_add_flag(confirm_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(confirm_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_set_style_pad_all(confirm_btn, 0, LV_STATE_DEFAULT);

    lv_obj_t *confirm_label = create_text_label(confirm_btn, "确认并返回",&eques_bold_24, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_set_align(confirm_label, LV_ALIGN_CENTER);
    lv_obj_add_event_cb(confirm_btn, card_popup_confirm_cb, LV_EVENT_CLICKED, enroll_scr);

    lv_obj_move_foreground(card_custom_popup);
}

/**
 * @brief 卡片录入确认按钮回调
 * @param e LVGL事件对象
 */
static void card_confirm_btn_click_cb(lv_event_t *e)
{
    if (e == NULL) return;

    lv_obj_t *enroll_scr = lv_event_get_user_data(e);
    if (enroll_scr == NULL) {
        LV_LOG_WARN("card_confirm_btn_click_cb: enroll_scr is NULL!");
        return;
    }

    // 弹出名称输入弹窗
    create_card_complete_popup(enroll_scr);
}


/**
 * @brief 卡片添加按钮点击入口回调
 * @param e LVGL事件对象
 */
void card_add_btn_click_cb(lv_event_t *e)
{
    if (e == NULL) return;

    lv_obj_t *parent_scr = lv_event_get_user_data(e);
    if (parent_scr == NULL) return;

    // 进入卡片录入界面
    ui_card_add_create(parent_scr);
    update_status_bar_parent(card_add_scr);
    //destroy_enroll();

    LV_LOG_INFO("Enter card add interface, destroy main enroll interface");
}

/**
 * @brief 卡片界面返回按钮回调
 * @param e LVGL事件对象
 */
static void card_add_back_btn_click_cb(lv_event_t *e)
{
    if (e == NULL) return;

    lv_obj_t *parent_scr = lv_event_get_user_data(e);
    lv_obj_t *current_scr = lv_disp_get_scr_act(NULL);
    
    if (!lv_obj_is_valid(current_scr)) return;

    // 返回录入主界面
    if (current_scr == card_add_scr) {
        // common_member_info_t *member = get_current_enroll_member();
        // ui_enroll_create(member, parent_scr);
        lv_scr_load(parent_scr);
        ui_card_add_destroy();
    }
}

/**
 * @brief 销毁卡片录入界面及所有资源
 */
void ui_card_add_destroy(void)
{
    del_card_timeout_timer();

    if (lv_obj_is_valid(card_add_scr)) {
        lv_obj_del(card_add_scr);
        card_add_scr = NULL;
    }
}