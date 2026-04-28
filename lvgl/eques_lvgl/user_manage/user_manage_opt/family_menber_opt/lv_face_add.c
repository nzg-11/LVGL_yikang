/**
 * @file lv_face_add.c
 * @brief 人脸录入界面实现
 * @details 包含人脸录入UI、超时检测、名称输入弹窗、成功/失败提示、界面切换
 * @note 依赖 LVGL 图形库、生物特征录入管理模块
 */

/*********************
 * 头文件包含
 *********************/
#include "lv_face_add.h"
#include "lv_a_enroll_opt.h"
#include <string.h>

/*********************
 * 宏定义
 *********************/
#define FACE_TIMEOUT_MS  3000    // 人脸录入超时时间(3秒)
#define FACE_NAME_MAX_LEN 8      // 人脸名称最大长度

/*********************
 * 全局变量定义
 *********************/
// 界面根对象
static lv_obj_t *face_add_scr = NULL;

// 样式相关
static lv_style_t  face_add_grad_style;
static bool        face_add_style_inited = false;

// 弹窗 & 键盘 & 定时器组件
static lv_obj_t   *face_bg_mask_layer = NULL;
static lv_obj_t   *face_custom_popup = NULL;
static lv_obj_t   *face_name_keyboard = NULL;
static lv_obj_t   *face_input_textarea = NULL;
static lv_timer_t *face_timeout_timer = NULL;
static lv_obj_t   *face_fail_popup = NULL;
static lv_obj_t   *face_timeout_enroll_scr = NULL;

/*********************
 * 静态函数前置声明
 * 【UI相关函数优先声明】
 *********************/
// 样式初始化
static void init_face_add_styles(void);

// UI核心创建（最高优先级）
void ui_face_add_create(lv_obj_t *enroll_scr);

// UI按钮回调
static void face_confirm_btn_click_cb(lv_event_t *e);
void face_add_btn_click_cb(lv_event_t *e);
void face_add_back_btn_click_cb(lv_event_t *e);

// 弹窗相关
static void create_face_complete_popup(lv_obj_t *enroll_scr);
static void face_popup_confirm_cb(lv_event_t *e);
static void face_fail_confirm_cb(lv_event_t *e);
static void close_face_popup(void);
static void close_face_fail_popup(void);

// 定时器相关
static void del_face_timeout_timer(void);
static void face_timeout_timer_cb(lv_timer_t *timer);

// 键盘相关
static void hide_face_keyboard(lv_event_t *e);
static void face_input_click_cb(lv_event_t *e);

// 资源销毁
void ui_face_add_destroy(void);

/*********************
 * 函数实现
 * 【UI相关函数优先实现】
 *********************/

/**
 * @brief 初始化人脸界面样式
 */
static void init_face_add_styles(void)
{
    if(!face_add_style_inited) {
        lv_style_init(&face_add_grad_style);
        face_add_style_inited = true;
    }
}

/**
 * @brief 创建人脸录入主界面（UI核心函数，前置）
 * @param enroll_scr 父界面对象
 */
void ui_face_add_create(lv_obj_t *enroll_scr)
{
    init_face_add_styles();

    // 安全校验
    if(enroll_scr == NULL) {
        LV_LOG_WARN("ui_face_add_create: enroll_scr is NULL!");
        return;
    }

    // 清理所有已有资源
    del_face_timeout_timer();
    close_face_popup();
    close_face_fail_popup();

    // 销毁旧界面
    if(lv_obj_is_valid(face_add_scr)) {
        lv_obj_del(face_add_scr);
        face_add_scr = NULL;
    }

    // 创建新界面根容器
    face_add_scr = lv_obj_create(NULL);
    lv_obj_clear_flag(face_add_scr, LV_OBJ_FLAG_HIDDEN);

    // 设置背景渐变样式
    lv_style_reset(&face_add_grad_style);
    lv_style_set_bg_color(&face_add_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&face_add_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&face_add_grad_style, LV_GRAD_DIR_VER);
    lv_style_set_bg_main_stop(&face_add_grad_style, 0);
    lv_style_set_bg_grad_stop(&face_add_grad_style, 255);
    lv_obj_add_style(face_add_scr, &face_add_grad_style, LV_STATE_DEFAULT);

    // ===================== UI元素绘制 =====================
    // 返回按钮
    lv_obj_t *back_btn = create_text_label
    (face_add_scr, ICON_CHEVORN_LEFT, &fontawesome_icon_32, lv_color_hex(0xFFFFFF), 52, 84, LV_OPA_100);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn, face_add_back_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);
    // 返回按钮绑定资源清理
    lv_obj_add_event_cb(back_btn, (lv_event_cb_t)del_face_timeout_timer, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(back_btn, (lv_event_cb_t)close_face_popup, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(back_btn, (lv_event_cb_t)close_face_fail_popup, LV_EVENT_CLICKED, NULL);

    // 标题
     create_text_label(
        face_add_scr, "添加人脸", &eques_bold_36,
        lv_color_hex(0xFFFFFF), 83, 80, LV_OPA_100);

    // 提示文本
    lv_obj_t *prompt_label = create_text_label(face_add_scr, "请离摄像头40-80厘米", &eques_regular_32, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align(prompt_label, LV_ALIGN_TOP_MID, 0, 508);

    // 圆形装饰UI
    create_container(face_add_scr,372,160,281,281,
        lv_color_hex(0x1D3740), LV_OPA_100, 200,
        lv_color_hex(0x005CAA), 20, LV_OPA_100);

    create_container(face_add_scr,405,192,216,216,
        lv_color_hex(0x061022), LV_OPA_0, 200,
        lv_color_hex(0xFFFFFF), 18, LV_OPA_100);

    // 修复拼写错误：circel → circle
    lv_obj_t *face_circle03 = create_custom_gradient_container(
        face_add_scr, 422, 210, 182, 182, 100,
        0xB1C6D2, 0x1D86BF, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_set_style_border_width(face_circle03, 4, LV_PART_MAIN);
    lv_obj_set_style_border_color(face_circle03, lv_color_hex(0x346E8D), LV_PART_MAIN);

    // 白色装饰线
    create_container(face_add_scr,380,168,264,264,
        lv_color_hex(0x1D3740), LV_OPA_0, 200,
        lv_color_hex(0xFFFFFF), 2, LV_OPA_100);

    // 确认按钮
    lv_obj_t *confirm_btn_con = create_container_circle(
        face_add_scr, 467, 259, 90, true,
        lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_add_flag(confirm_btn_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(confirm_btn_con, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(confirm_btn_con, face_confirm_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);

    // 加载界面
    update_status_bar_parent(face_add_scr);
    lv_scr_load(face_add_scr);

    // 启动超时定时器
    face_timeout_enroll_scr = enroll_scr;
    face_timeout_timer = lv_timer_create(face_timeout_timer_cb, FACE_TIMEOUT_MS, NULL);
}

/**
 * @brief 人脸确认按钮回调
 */
static void face_confirm_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;

    lv_obj_t *enroll_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(enroll_scr == NULL) {
        LV_LOG_WARN("face_confirm_btn_click_cb: enroll_scr is NULL!");
        return;
    }

    // 弹出名称输入弹窗
    create_face_complete_popup(enroll_scr);
}

/**
 * @brief 人脸添加入口按钮回调
 */
void face_add_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(parent_scr == NULL) return;

    // 进入人脸录入界面
    ui_face_add_create(parent_scr);
    //lv_scr_load(face_add_scr);
    update_status_bar_parent(face_add_scr);
   // destroy_enroll();

    LV_LOG_INFO("Enter face add interface");
}

/**
 * @brief 人脸界面返回按钮回调
 */
void face_add_back_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);

    lv_obj_t *current_scr = lv_disp_get_scr_act(NULL);
    if(!lv_obj_is_valid(current_scr)) return;

    if(current_scr == face_add_scr) {
        //common_member_info_t *member = get_current_enroll_member();
        //ui_enroll_create(member, parent_scr);
        lv_scr_load(parent_scr);
        ui_face_add_destroy();
    }
}

/**
 * @brief 创建人脸录入成功弹窗
 */
static void create_face_complete_popup(lv_obj_t *enroll_scr)
{
    // 清理资源
    close_face_popup();
    del_face_timeout_timer();

    // 遮罩层
    face_bg_mask_layer = lv_obj_create(face_add_scr);
    lv_obj_set_size(face_bg_mask_layer, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(face_bg_mask_layer, 0, 0);
    lv_obj_set_style_bg_color(face_bg_mask_layer, lv_color_hex(0x000000), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(face_bg_mask_layer, LV_OPA_70, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(face_bg_mask_layer, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(face_bg_mask_layer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(face_bg_mask_layer, LV_OBJ_FLAG_HIDDEN);

    // 弹窗主体
    face_custom_popup = create_container(
        face_add_scr, 212, 150, 600, 270,
        lv_color_hex(0xE0EDFF), LV_OPA_100,
        16, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(face_custom_popup, 0, LV_STATE_DEFAULT);

    // 成功提示
    lv_obj_t *succeed_label = create_text_label(
        face_custom_popup, "添加成功", &eques_regular_24,
        lv_color_hex(0x000000), 0, 38, LV_OPA_100);
    lv_obj_align(succeed_label, LV_ALIGN_TOP_MID, 0, 38);

    // 名称输入框
    face_input_textarea = lv_textarea_create(face_custom_popup);
    lv_obj_clear_flag(face_input_textarea, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(face_input_textarea, 382, 44);
    lv_obj_set_pos(face_input_textarea, 137, 90);
    lv_obj_set_style_bg_color(face_input_textarea, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(face_input_textarea, lv_color_hex(0x333333), LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(face_input_textarea, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(face_input_textarea, 6, LV_STATE_DEFAULT);
    //lv_textarea_set_placeholder_text(face_input_textarea, "please input face name");
    lv_textarea_set_max_length(face_input_textarea, FACE_NAME_MAX_LEN);
    lv_textarea_set_one_line(face_input_textarea, true);
    //lv_obj_set_style_text_font(face_input_textarea, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    lv_obj_add_flag(face_input_textarea, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(face_input_textarea, LV_OPA_80, LV_STATE_PRESSED);
    //lv_obj_add_event_cb(face_input_textarea, face_input_click_cb, LV_EVENT_CLICKED, NULL);

    // 确认按钮
    lv_obj_t *confirm_btn = create_custom_gradient_container(
        face_custom_popup, 197, 171, 205, 44, 6,
        0X006BDC, 0x00BDBD, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_add_flag(confirm_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(confirm_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_set_style_pad_all(confirm_btn, 0, LV_STATE_DEFAULT);

    lv_obj_t *confirm_label = create_text_label(confirm_btn, "确认并返回",&eques_bold_24, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_set_align(confirm_label, LV_ALIGN_CENTER);
    lv_obj_add_event_cb(confirm_btn, face_popup_confirm_cb, LV_EVENT_CLICKED, enroll_scr);

    lv_obj_move_foreground(face_custom_popup);
}

/**
 * @brief 成功弹窗确认回调
 */
static void face_popup_confirm_cb(lv_event_t *e)
{
    if(e == NULL) return;

    //common_member_info_t *member = get_current_enroll_member();
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);

    // 获取名称，空值使用默认
    const char *face_name = lv_textarea_get_text(face_input_textarea);
    if(face_name == NULL || strlen(face_name) == 0) {
        face_name = "";
    }

    // 通知录入完成
    face_enroll_complete(face_name);
    close_face_popup();

    // 返回录入主界面
    //ui_enroll_create(member, parent_scr);
    lv_scr_load(parent_scr);
    // 销毁当前界面
    if(lv_obj_is_valid(face_add_scr)) {
        lv_obj_del(face_add_scr);
        face_add_scr = NULL;
    }
}

/**
 * @brief 失败弹窗确认回调
 */
static void face_fail_confirm_cb(lv_event_t *e)
{
    if(e == NULL) return;

    //common_member_info_t *member = get_current_enroll_member();
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);

    close_face_fail_popup();
    //ui_enroll_create(member, parent_scr);
    lv_scr_load(parent_scr);
    if(lv_obj_is_valid(face_add_scr)) {
        lv_obj_del(face_add_scr);
        face_add_scr = NULL;
    }
}

/**
 * @brief 关闭成功弹窗
 */
static void close_face_popup(void)
{
    if(lv_obj_is_valid(face_name_keyboard)) {
        lv_obj_add_flag(face_name_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
    if(lv_obj_is_valid(face_bg_mask_layer)) {
        lv_obj_add_flag(face_bg_mask_layer, LV_OBJ_FLAG_HIDDEN);
    }
    if(lv_obj_is_valid(face_custom_popup)) {
        lv_obj_del(face_custom_popup);
        face_custom_popup = NULL;
    }
}

/**
 * @brief 关闭失败弹窗
 */
static void close_face_fail_popup(void)
{
    if(lv_obj_is_valid(face_fail_popup)) {
        lv_obj_del(face_fail_popup);
        face_fail_popup = NULL;
    }
    if(lv_obj_is_valid(face_bg_mask_layer)) {
        lv_obj_add_flag(face_bg_mask_layer, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief 销毁超时定时器
 */
static void del_face_timeout_timer(void)
{
    if(face_timeout_timer != NULL) {
        lv_timer_del(face_timeout_timer);
        face_timeout_timer = NULL;
    }
    face_timeout_enroll_scr = NULL;
}

/**
 * @brief 超时定时器回调
 */
static void face_timeout_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    lv_obj_t *enroll_scr = face_timeout_enroll_scr;
    del_face_timeout_timer();

    if(!lv_obj_is_valid(face_add_scr)) {
        return;
    }

    // 创建遮罩层
    face_bg_mask_layer = lv_obj_create(face_add_scr);
    lv_obj_set_size(face_bg_mask_layer, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(face_bg_mask_layer, 0, 0);
    lv_obj_set_style_bg_color(face_bg_mask_layer, lv_color_hex(0x000000), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(face_bg_mask_layer, LV_OPA_70, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(face_bg_mask_layer, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(face_bg_mask_layer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(face_bg_mask_layer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(face_bg_mask_layer);
    lv_obj_add_event_cb(face_bg_mask_layer, hide_face_keyboard, LV_EVENT_CLICKED, NULL);
    // 创建失败弹窗
    face_fail_popup = create_container(
        face_add_scr, 212, 150, 600, 200,
        lv_color_hex(0xE0EDFF), LV_OPA_100,
        16, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(face_fail_popup, 0, LV_STATE_DEFAULT);
    lv_obj_move_foreground(face_fail_popup);

    // 失败提示
    lv_obj_t *fail_label = create_text_label(
        face_fail_popup, "添加失败", &eques_regular_24,
        lv_color_hex(0xFF0000), 0, 38, LV_OPA_100);
    lv_obj_align(fail_label, LV_ALIGN_TOP_MID, 0, 38);

    // 确认按钮
    lv_obj_t *fail_confirm_btn = create_custom_gradient_container(
        face_fail_popup, 197, 120, 205, 44, 6,
        0X006BDC, 0x00BDBD, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_add_flag(fail_confirm_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(fail_confirm_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_set_style_pad_all(fail_confirm_btn, 0, LV_STATE_DEFAULT);

    lv_obj_t *fail_confirm_label = create_text_label(fail_confirm_btn, "确认并返回",&eques_bold_24, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_set_align(fail_confirm_label, LV_ALIGN_CENTER);
    lv_obj_add_event_cb(fail_confirm_btn, face_fail_confirm_cb, LV_EVENT_CLICKED, enroll_scr);
}

/**
 * @brief 隐藏键盘
 */
static void hide_face_keyboard(lv_event_t *e)
{
    (void)e;
    if(lv_obj_is_valid(face_name_keyboard)) {
        lv_obj_add_flag(face_name_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief 输入框点击回调（弹出键盘）
 */
static void face_input_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *input = lv_event_get_target(e);

    if(!lv_obj_is_valid(face_name_keyboard)) {
        face_name_keyboard = lv_keyboard_create(lv_scr_act());
        lv_obj_set_style_bg_color(face_name_keyboard, lv_color_hex(0x192A46), LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(face_name_keyboard, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(face_name_keyboard, lv_color_hex(0x1F3150), LV_STATE_DEFAULT);
        lv_obj_set_style_radius(face_name_keyboard, 6, LV_STATE_DEFAULT);
        lv_obj_set_size(face_name_keyboard, LV_HOR_RES, 250);
        lv_obj_align(face_name_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_add_event_cb(face_name_keyboard, hide_face_keyboard, LV_EVENT_READY, NULL);
    }

    lv_obj_clear_flag(face_name_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(face_name_keyboard, input);
    lv_obj_move_foreground(face_name_keyboard);
}

/**
 * @brief 销毁人脸录入界面及所有资源
 */
void ui_face_add_destroy(void)
{
    del_face_timeout_timer();
    if(lv_obj_is_valid(face_add_scr)) {
        lv_obj_del(face_add_scr);
        face_add_scr = NULL;
    }
}