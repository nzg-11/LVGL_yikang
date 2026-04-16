/**
 * @file lv_finger_add.c
 * @brief 指纹录入界面实现
 * @details 包含指纹进度采集、名称输入弹窗、界面切换、资源管理
 * @note 依赖 LVGL 图形库、生物特征录入管理模块
 */

/*********************
 * 头文件包含
 *********************/
#include "lv_finger_add.h"
#include "lv_a_enroll_opt.h"
#include <string.h>

/*********************
 * 宏定义
 *********************/
#define FINGER_NAME_MAX_LEN  8     // 指纹名称最大长度

/*********************
 * 全局变量定义
 *********************/
// 界面根对象 & 样式
static lv_obj_t      *finger_add_scr = NULL;
static lv_style_t     finger_add_grad_style;
static bool           finger_add_style_inited = false;

// 指纹进度控制
static uint8_t        finger_add_step = 0;

// 全局UI控件
static lv_obj_t      *finger_percent_img = NULL;
static lv_obj_t      *precent_label = NULL;
static lv_obj_t      *add_finger_circle01 = NULL;  // 修复拼写错误
static lv_obj_t      *add_finger_circle02 = NULL;
static lv_obj_t      *add_finger_circle03 = NULL;
static lv_obj_t      *finger_success_label = NULL;

// 弹窗 & 键盘组件
static lv_obj_t      *finger_bg_mask_layer = NULL;
static lv_obj_t      *finger_custom_popup = NULL;
static lv_obj_t      *finger_name_keyboard = NULL;
static lv_obj_t      *finger_input_textarea = NULL;

/*********************
 * 静态函数前置声明
 * 【UI相关函数优先声明】
 *********************/
// 样式初始化
static void init_other_member_styles(void);

// UI核心创建（最高优先级）
void ui_finger_add_create(lv_obj_t *enroll_scr);

// UI按钮回调
static void finger_add_click_cb(lv_event_t *e);
void finger_add_btn_click_cb(lv_event_t *e);
void finger_add_back_btn_click_cb(lv_event_t *e);
static void finger_confirm_click_cb(lv_event_t *e);

// 弹窗相关
static void create_finger_complete_popup(lv_obj_t *enroll_scr);
static void close_finger_popup(void);

// 键盘相关
static void hide_finger_keyboard(lv_event_t *e);
static void finger_input_click_cb(lv_event_t *e);

/*********************
 * 函数实现
 * 【UI相关函数优先实现】
 *********************/

/**
 * @brief 初始化界面样式
 */
static void init_other_member_styles(void)
{
    if(!finger_add_style_inited) {
        lv_style_init(&finger_add_grad_style);
        finger_add_style_inited = true;
    }
}

/**
 * @brief 创建指纹录入主界面
 * @param enroll_scr 父界面对象
 */
void ui_finger_add_create(lv_obj_t *enroll_scr)
{
    init_other_member_styles();

    // 安全校验
    if(enroll_scr == NULL) {
        LV_LOG_WARN("ui_finger_add_create: enroll_scr is NULL!");
        return;
    }

    // 清理资源
    close_finger_popup();
    if(lv_obj_is_valid(finger_add_scr)) {
        lv_obj_del(finger_add_scr);
        finger_add_scr = NULL;
    }

    // 创建新界面
    finger_add_scr = lv_obj_create(NULL);
    finger_add_step = 0;  // 重置进度

    // 设置背景渐变样式
    lv_style_reset(&finger_add_grad_style);
    lv_style_set_bg_color(&finger_add_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&finger_add_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&finger_add_grad_style, LV_GRAD_DIR_VER);
    lv_style_set_bg_main_stop(&finger_add_grad_style, 0);
    lv_style_set_bg_grad_stop(&finger_add_grad_style, 255);
    lv_obj_add_style(finger_add_scr, &finger_add_grad_style, LV_STATE_DEFAULT);

    // ===================== UI元素绘制 =====================
    // 标题
    create_text_label(finger_add_scr, "add finger", &lv_font_montserrat_36,
        lv_color_hex(0xFFFFFF), 83, 80, LV_OPA_100);

    // 提示标签
    precent_label = create_text_label(finger_add_scr, "Please place your finger",
        &lv_font_montserrat_48, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align(precent_label, LV_ALIGN_TOP_MID, 0, 508);

    // 圆形装饰UI（修复拼写错误）
    add_finger_circle01 = create_container(finger_add_scr,372,160,281,281,
        lv_color_hex(0x1D3740), LV_OPA_100, 200,
        lv_color_hex(0x005CAA), 20, LV_OPA_100);

    add_finger_circle02 = create_container(finger_add_scr,405,192,216,216,
        lv_color_hex(0x061022), LV_OPA_0, 200,
        lv_color_hex(0xFFFFFF), 18, LV_OPA_100);

    add_finger_circle03 = create_custom_gradient_container(finger_add_scr, 422, 210, 182, 182, 100,
        0xB1C6D2, 0x1D86BF, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_set_style_border_width(add_finger_circle03, 4, LV_PART_MAIN);
    lv_obj_set_style_border_color(add_finger_circle03, lv_color_hex(0x346E8D), LV_PART_MAIN);

    create_container(finger_add_scr,380,168,264,264,
        lv_color_hex(0x1D3740), LV_OPA_0, 200,
        lv_color_hex(0xFFFFFF), 2, LV_OPA_100);

    // 指纹点击按钮
    finger_percent_img = create_container_circle(finger_add_scr, 467, 259, 90,
        true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_add_flag(finger_percent_img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(finger_percent_img, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(finger_percent_img, finger_add_click_cb, LV_EVENT_CLICKED, enroll_scr);

    // 返回按钮
    lv_obj_t *back_btn = create_container_circle(finger_add_scr, 52, 90, 30,
        true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn, finger_add_back_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);

    // 加载界面
    update_status_bar_parent(finger_add_scr);
    lv_scr_load(finger_add_scr);
}

/**
 * @brief 指纹点击回调（进度控制）
 */
static void finger_add_click_cb(lv_event_t *e)
{
    if(e == NULL) return;

    // 步骤+1，最多到5
    finger_add_step++;
    if(finger_add_step > 5) {
        finger_add_step = 0;
        lv_label_set_text(precent_label, "0%");
        lv_obj_set_pos(finger_percent_img, 309, 415);

        // 恢复样式
        lv_obj_set_style_border_opa(add_finger_circle01, LV_OPA_100, LV_STATE_DEFAULT);
        lv_obj_set_style_border_opa(add_finger_circle02, LV_OPA_100, LV_STATE_DEFAULT);
        lv_obj_set_style_border_opa(add_finger_circle03, LV_OPA_100, LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(add_finger_circle03, lv_color_hex(0x061022), LV_STATE_DEFAULT);

        lv_obj_clear_flag(precent_label, LV_OBJ_FLAG_HIDDEN);
        if(finger_success_label != NULL) {
            lv_obj_add_flag(finger_success_label, LV_OBJ_FLAG_HIDDEN);
        }
        close_finger_popup();
        lv_obj_add_flag(finger_percent_img, LV_OBJ_FLAG_CLICKABLE);
        return;
    }

    // 进度切换
    switch(finger_add_step) {
        case 1: lv_label_set_text(precent_label, "25%"); break;
        case 2: lv_label_set_text(precent_label, "50%"); break;
        case 3: lv_label_set_text(precent_label, "75%"); break;
        case 4: lv_label_set_text(precent_label, "100%"); break;
        case 5:{
            lv_obj_t *enroll_scr = (lv_obj_t *)lv_event_get_user_data(e);
            create_finger_complete_popup(enroll_scr);
            break;
        }
            
        default: lv_label_set_text(precent_label, "0%"); break;
    }
}

/**
 * @brief 指纹添加入口按钮回调
 */
void finger_add_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(parent_scr == NULL) return;

    ui_finger_add_create(parent_scr);
    // lv_scr_load(finger_add_scr);
    update_status_bar_parent(finger_add_scr);
    //destroy_enroll();

    LV_LOG_INFO("Enter finger add interface");
}

/**
 * @brief 指纹界面返回按钮回调
 */
void finger_add_back_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);

    lv_obj_t *current_scr = lv_disp_get_scr_act(NULL);
    if(!lv_obj_is_valid(current_scr)) return;

    if(current_scr == finger_add_scr) {
        // common_member_info_t *member = get_current_enroll_member();
        // ui_enroll_create(member, parent_scr);
        lv_scr_load(parent_scr); 
        lv_obj_del(current_scr);
        finger_add_scr = NULL;
    }
}

/**
 * @brief 弹窗确认按钮回调
 */
static void finger_confirm_click_cb(lv_event_t *e)
{
    if(e == NULL) return;

    //common_member_info_t *member = get_current_enroll_member();
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);

    // 获取名称
    const char *finger_name = lv_textarea_get_text(finger_input_textarea);
    if(finger_name == NULL || strlen(finger_name) == 0) {
        finger_name = "Unnamed finger";
    }

    // 录入完成
    finger_enroll_complete(finger_name);
    close_finger_popup();

    // 返回主界面
    //ui_enroll_create(member, parent_scr);
    lv_scr_load(parent_scr); 
    
    // 销毁资源
    if(lv_obj_is_valid(finger_add_scr)) {
        lv_obj_del(finger_add_scr);
        finger_add_scr = NULL;
    }

    finger_add_step = 0;
    LV_LOG_USER("Finger add success, back to enroll screen");
}

/**
 * @brief 创建指纹录入成功弹窗
 */
static void create_finger_complete_popup(lv_obj_t *enroll_scr)
{
    close_finger_popup();

    // 遮罩层
    finger_bg_mask_layer = lv_obj_create(finger_add_scr);
    lv_obj_set_size(finger_bg_mask_layer, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(finger_bg_mask_layer, 0, 0);
    lv_obj_set_style_bg_color(finger_bg_mask_layer, lv_color_hex(0x000000), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(finger_bg_mask_layer, LV_OPA_70, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(finger_bg_mask_layer, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(finger_bg_mask_layer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(finger_bg_mask_layer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(finger_bg_mask_layer, hide_finger_keyboard, LV_EVENT_CLICKED, NULL);
    // 弹窗主体
    finger_custom_popup = create_container(finger_add_scr, 212, 150, 600, 270,
        lv_color_hex(0xE0EDFF), LV_OPA_100,
        16, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(finger_custom_popup, 0, LV_STATE_DEFAULT);

    // 成功提示
    lv_obj_t *succeed_add_label = create_text_label(finger_custom_popup, "succeed add",
        &lv_font_montserrat_24, lv_color_hex(0x000000), 0, 38, LV_OPA_100);
    lv_obj_align(succeed_add_label, LV_ALIGN_TOP_MID, 0, 38);

    // 名称输入框
    finger_input_textarea = lv_textarea_create(finger_custom_popup);
    lv_obj_clear_flag(finger_input_textarea, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(finger_input_textarea, 382, 44);
    lv_obj_set_pos(finger_input_textarea, 137, 90);
    lv_obj_set_style_bg_color(finger_input_textarea, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(finger_input_textarea, lv_color_hex(0x333333), LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(finger_input_textarea, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(finger_input_textarea, 6, LV_STATE_DEFAULT);
    lv_textarea_set_placeholder_text(finger_input_textarea, "please input name");
    lv_textarea_set_max_length(finger_input_textarea, FINGER_NAME_MAX_LEN);
    lv_textarea_set_one_line(finger_input_textarea, true);
    lv_obj_set_style_text_font(finger_input_textarea, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    lv_obj_add_flag(finger_input_textarea, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(finger_input_textarea, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(finger_input_textarea, finger_input_click_cb, LV_EVENT_CLICKED, NULL);

    // 确认按钮
    lv_obj_t *confirm_btn = create_custom_gradient_container(finger_custom_popup, 197, 171, 205, 44, 6,
        0X006BDC, 0x00BDBD, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_add_flag(confirm_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(confirm_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_set_style_pad_all(confirm_btn, 0, LV_STATE_DEFAULT);

    lv_obj_t *confirm_label = create_text_label(confirm_btn, "Confirm",
        &lv_font_montserrat_28, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_set_align(confirm_label, LV_ALIGN_CENTER);
    lv_obj_add_event_cb(confirm_btn, finger_confirm_click_cb, LV_EVENT_CLICKED, enroll_scr);
}

/**
 * @brief 关闭指纹弹窗并释放资源
 */
static void close_finger_popup(void)
{
    if(lv_obj_is_valid(finger_name_keyboard)) {
        lv_obj_del(finger_name_keyboard);
        finger_name_keyboard = NULL;
    }
    if(lv_obj_is_valid(finger_bg_mask_layer)) {
        lv_obj_del(finger_bg_mask_layer);
        finger_bg_mask_layer = NULL;
    }
    if(lv_obj_is_valid(finger_custom_popup)) {
        lv_obj_del(finger_custom_popup);
        finger_custom_popup = NULL;
    }
}

/**
 * @brief 隐藏键盘
 */
static void hide_finger_keyboard(lv_event_t *e)
{
    (void)e;
    if(lv_obj_is_valid(finger_name_keyboard)) {
        lv_obj_add_flag(finger_name_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief 输入框点击回调（弹出键盘）
 */
static void finger_input_click_cb(lv_event_t *e)
{
    lv_obj_t *input = lv_event_get_target(e);

    // 创建键盘
    finger_name_keyboard = lv_keyboard_create(lv_scr_act());
    lv_obj_set_style_bg_color(finger_name_keyboard, lv_color_hex(0x192A46), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(finger_name_keyboard, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(finger_name_keyboard, lv_color_hex(0x1F3150), LV_STATE_DEFAULT);
    lv_obj_set_style_radius(finger_name_keyboard, 6, LV_STATE_DEFAULT);
    lv_obj_set_size(finger_name_keyboard, LV_HOR_RES, 250);
    lv_obj_align(finger_name_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_event_cb(finger_name_keyboard, hide_finger_keyboard, LV_EVENT_READY, NULL);

    // 关联输入框
    lv_obj_clear_flag(finger_name_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(finger_name_keyboard, input);
}