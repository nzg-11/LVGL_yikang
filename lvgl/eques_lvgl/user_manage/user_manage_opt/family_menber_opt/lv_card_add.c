#include "lv_card_add.h"
#include "lv_a_enroll_opt.h"  // 引入卡片录入完成回调的头文件
#include <math.h>  // 引入数学库，用于三角函数计算
#include <string.h> // 引入字符串库

static lv_obj_t *card_add_scr = NULL;  
static lv_style_t card_add_grad_style;
static bool card_add_style_inited = false;

// 动画相关全局变量
static lv_obj_t *enter_card_img = NULL;  // 卡片图片对象（全局可访问）
static lv_timer_t *card_anim_timer = NULL; // 动画定时器
static int16_t card_img_origin_x = 0;    // 图片原始X坐标
static int16_t card_img_origin_y = 0;    // 图片原始Y坐标
static float card_anim_angle = 0.0f;     // 动画角度（弧度）
#define CARD_ANIM_RADIUS 12               // 圆周运动半径（小圈，可调整）
#define CARD_ANIM_SPEED 0.05f            // 动画速度（弧度/帧）
#define MAX_CARD_TIMEOUT_MS 10000  // 10秒超时

// 弹窗相关全局变量（仿指纹弹窗）
static lv_obj_t *card_bg_mask_layer = NULL;  // 卡片弹窗遮罩层
static lv_obj_t *card_custom_popup = NULL;   // 卡片弹窗主体
static lv_obj_t *card_name_keyboard = NULL;  // 卡片弹窗键盘
static lv_obj_t *card_input_textarea = NULL; // 卡片弹窗输入框
static lv_timer_t *card_timeout_timer = NULL;// 3秒超时定时器
static lv_obj_t *card_fail_popup = NULL;     // 失败弹窗
//  超时定时器用户数据全局变量（兼容LVGL 8.3）
static lv_obj_t *card_timeout_enroll_scr = NULL;

// 停止卡片动画的函数
static void stop_card_animation(void)
{
    if(card_anim_timer != NULL) {
        lv_timer_del(card_anim_timer);
        card_anim_timer = NULL;
    }
    // 恢复图片到原始位置
    if(enter_card_img != NULL && lv_obj_is_valid(enter_card_img)) {
        lv_obj_set_pos(enter_card_img, card_img_origin_x, card_img_origin_y);
    }
}

// 销毁超时定时器
static void del_card_timeout_timer(void)
{
    if(card_timeout_timer != NULL) {
        lv_timer_del(card_timeout_timer);
        card_timeout_timer = NULL;
    }
    card_timeout_enroll_scr = NULL;
}

// 关闭卡片成功弹窗
static void close_card_popup(void)
{
    // 隐藏键盘
    if(card_name_keyboard != NULL && lv_obj_is_valid(card_name_keyboard)) {
        lv_obj_add_flag(card_name_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
    // 隐藏遮罩层
    if(card_bg_mask_layer != NULL && lv_obj_is_valid(card_bg_mask_layer)) {
        lv_obj_add_flag(card_bg_mask_layer, LV_OBJ_FLAG_HIDDEN);
    }
    // 销毁弹窗主体
    if(card_custom_popup != NULL && lv_obj_is_valid(card_custom_popup)) {
        lv_obj_del(card_custom_popup);
        card_custom_popup = NULL;
    }
}

// 关闭失败弹窗
static void close_card_fail_popup(void)
{
    if(card_fail_popup != NULL && lv_obj_is_valid(card_fail_popup)) {
        lv_obj_del(card_fail_popup);
        card_fail_popup = NULL;
    }
    if(card_bg_mask_layer != NULL && lv_obj_is_valid(card_bg_mask_layer)) {
        lv_obj_add_flag(card_bg_mask_layer, LV_OBJ_FLAG_HIDDEN);
    }
}

// 失败弹窗回调（点击确认返回上一级）
static void card_fail_confirm_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *enroll_scr = (lv_obj_t *)lv_event_get_user_data(e);
    close_card_fail_popup();
    // 返回原录入界面
    if(enroll_scr != NULL && lv_obj_is_valid(enroll_scr)) {
        lv_scr_load(enroll_scr);
    }
}

// 3秒超时回调（触发失败弹窗）
static void card_timeout_timer_cb(lv_timer_t *timer)
{
    stop_card_animation();
    
    (void)timer;
    // 替换lv_timer_get_user_data，直接读取全局变量
    lv_obj_t *enroll_scr = card_timeout_enroll_scr;
    
    // 1. 先销毁定时器（避免重复触发）
    del_card_timeout_timer();
    
    // 2. 校验card_add_scr有效性
    if(card_add_scr == NULL || !lv_obj_is_valid(card_add_scr)) {
        return;
    }

    // 3. 创建失败弹窗遮罩层
    if(card_bg_mask_layer == NULL || !lv_obj_is_valid(card_bg_mask_layer)) {
        card_bg_mask_layer = lv_obj_create(card_add_scr);
        lv_obj_set_size(card_bg_mask_layer, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_pos(card_bg_mask_layer, 0, 0);
        lv_obj_set_style_bg_color(card_bg_mask_layer, lv_color_hex(0x000000), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(card_bg_mask_layer, LV_OPA_70, LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(card_bg_mask_layer, 0, LV_STATE_DEFAULT);
        lv_obj_add_flag(card_bg_mask_layer, LV_OBJ_FLAG_CLICKABLE);
    }
    lv_obj_clear_flag(card_bg_mask_layer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(card_bg_mask_layer); // 遮罩层置顶

    // 4. 创建失败弹窗主体
    card_fail_popup = create_container
    (card_add_scr, 100, 455, 600, 200,
     lv_color_hex(0xE0EDFF), LV_OPA_100,
     16, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(card_fail_popup, 0, LV_STATE_DEFAULT);
    lv_obj_move_foreground(card_fail_popup); // 弹窗置顶

    // 5. 失败提示文本
    lv_obj_t *fail_label = create_text_label
    (card_fail_popup, "Add failed (timeout)", &lv_font_montserrat_24, lv_color_hex(0xFF0000), 0, 38, LV_OPA_100);
    lv_obj_align(fail_label, LV_ALIGN_TOP_MID, 0, 38);

    // 6. 失败弹窗确认按钮
    lv_obj_t *fail_confirm_btn = create_custom_gradient_container
    (card_fail_popup, 197, 120, 205, 44, 6, 0X006BDC, 0x00BDBD, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_add_flag(fail_confirm_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(fail_confirm_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_set_style_pad_all(fail_confirm_btn, 0, LV_STATE_DEFAULT);

    lv_obj_t *fail_confirm_label = create_text_label
    (fail_confirm_btn, "Confirm", &lv_font_montserrat_28, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_set_align(fail_confirm_label, LV_ALIGN_CENTER);
    lv_obj_add_event_cb(fail_confirm_btn, card_fail_confirm_cb, LV_EVENT_CLICKED, enroll_scr);
}

// 隐藏卡片弹窗键盘
static void hide_card_keyboard(lv_event_t *e)
{
    (void)e;
    if(card_name_keyboard != NULL && lv_obj_is_valid(card_name_keyboard)) {
        lv_obj_add_flag(card_name_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

// 卡片弹窗输入框点击回调
static void card_input_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *input = lv_event_get_target(e);
    
    // 创建键盘
    if(card_name_keyboard == NULL || !lv_obj_is_valid(card_name_keyboard)) {
        card_name_keyboard = lv_keyboard_create(lv_scr_act());
        // 设置键盘样式
        lv_obj_set_style_bg_color(card_name_keyboard, lv_color_hex(0x192A46), LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(card_name_keyboard, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(card_name_keyboard, lv_color_hex(0x1F3150), LV_STATE_DEFAULT);
        lv_obj_set_style_radius(card_name_keyboard, 6, LV_STATE_DEFAULT);
        // 键盘尺寸适配屏幕
        lv_obj_set_size(card_name_keyboard, LV_HOR_RES, LV_VER_RES/3);
        // 键盘底部对齐
        lv_obj_align(card_name_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_add_event_cb(card_name_keyboard, hide_card_keyboard, LV_EVENT_READY, NULL);
    }
    
    // 显示键盘并关联输入框
    lv_obj_clear_flag(card_name_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(card_name_keyboard, input);
    lv_obj_move_foreground(card_name_keyboard);
}

// 卡片确认按钮回调
static void card_popup_confirm_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    // 1. 获取传入的原录入界面对象
    lv_obj_t *enroll_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(enroll_scr == NULL || !lv_obj_is_valid(enroll_scr)) {
        LV_LOG_WARN("card_popup_confirm_cb: enroll_scr invalid!");
        return;
    }

    // 2. 获取输入的卡片名称
    const char *card_name = lv_textarea_get_text(card_input_textarea);
    if(card_name == NULL || strlen(card_name) == 0) {
        card_name = "card01"; // 默认名称
    }

    // 3. 调用卡片录入完成回调
    card_enroll_complete(card_name);

    // 4. 关闭弹窗
    close_card_popup();

    // 5. 返回原录入界面
    update_status_bar_parent(enroll_scr);
    lv_scr_load(enroll_scr);

    // 6. 隐藏卡片添加界面
    if(card_add_scr != NULL && lv_obj_is_valid(card_add_scr)) {
        lv_obj_add_flag(card_add_scr, LV_OBJ_FLAG_HIDDEN);
    }
}

// 卡片动画回调函数
static void card_anim_timer_cb(lv_timer_t *timer)
{
    
    (void)timer;
    if(enter_card_img == NULL || !lv_obj_is_valid(enter_card_img)) {
        stop_card_animation();
        return;
    }

    // 1. 更新角度
    card_anim_angle += CARD_ANIM_SPEED;
    if(card_anim_angle >= 2 * M_PI) {  // 超过360度重置，避免数值过大
        card_anim_angle = 0.0f;
    }

    // 2. 计算圆周运动偏移量
    int16_t offset_x = (int16_t)(CARD_ANIM_RADIUS * sin(card_anim_angle));
    int16_t offset_y = (int16_t)(CARD_ANIM_RADIUS * cos(card_anim_angle));

    // 3. 更新图片位置
    lv_obj_set_pos(enter_card_img, card_img_origin_x + offset_x, card_img_origin_y + offset_y);
}

// 全局样式初始化
static void init_card_add_styles(void)
{
    if(!card_add_style_inited) {
        lv_style_init(&card_add_grad_style);
        card_add_style_inited = true;
    }
}

// 创建卡片录入成功弹窗
static void create_card_complete_popup(lv_obj_t *enroll_scr)
{
    // 先关闭旧弹窗
    close_card_popup();

    // 1. 销毁超时定时器
    del_card_timeout_timer();

    card_bg_mask_layer = lv_obj_create(card_add_scr);
    lv_obj_set_size(card_bg_mask_layer, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(card_bg_mask_layer, 0, 0);
    lv_obj_set_style_bg_color(card_bg_mask_layer, lv_color_hex(0x000000), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(card_bg_mask_layer, LV_OPA_70, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(card_bg_mask_layer, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(card_bg_mask_layer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(card_bg_mask_layer, LV_OBJ_FLAG_HIDDEN);

    // 3. 创建弹窗主体
    card_custom_popup = create_container
    (card_add_scr, 100, 455, 600, 270,
     lv_color_hex(0xE0EDFF), LV_OPA_100,
     16, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(card_custom_popup, 0, LV_STATE_DEFAULT);

    // 4. 成功提示文本
    lv_obj_t *succeed_add_label = create_text_label
    (card_custom_popup, "succeed add", &lv_font_montserrat_24, lv_color_hex(0x000000), 0, 38, LV_OPA_100);
    lv_obj_align(succeed_add_label, LV_ALIGN_TOP_MID, 0, 38);

    // 5. 输入框（仿指纹弹窗）
    card_input_textarea = lv_textarea_create(card_custom_popup);
    lv_obj_clear_flag(card_input_textarea, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(card_input_textarea, 382, 44);
    lv_obj_set_pos(card_input_textarea, 137, 90);
    lv_obj_set_style_bg_color(card_input_textarea, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(card_input_textarea, lv_color_hex(0x333333), LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(card_input_textarea, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(card_input_textarea, 6, LV_STATE_DEFAULT);
    lv_textarea_set_placeholder_text(card_input_textarea, "please input card name");
    lv_textarea_set_max_length(card_input_textarea, 8);
    lv_textarea_set_one_line(card_input_textarea, true);
    lv_obj_set_style_text_font(card_input_textarea, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    lv_obj_add_flag(card_input_textarea, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(card_input_textarea, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(card_input_textarea, card_input_click_cb, LV_EVENT_CLICKED, NULL);

    // 6. 确认按钮（仿指纹弹窗）
    lv_obj_t *confirm_btn = create_custom_gradient_container
    (card_custom_popup, 197, 171, 205, 44, 6, 0X006BDC, 0x00BDBD, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_add_flag(confirm_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(confirm_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_set_style_pad_all(confirm_btn, 0, LV_STATE_DEFAULT);

    lv_obj_t *confirm_label = create_text_label
    (confirm_btn, "Confirm", &lv_font_montserrat_28, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_set_align(confirm_label, LV_ALIGN_CENTER);
    lv_obj_add_event_cb(confirm_btn, card_popup_confirm_cb, LV_EVENT_CLICKED, enroll_scr);

    // 7. 确保弹窗在最上层
    lv_obj_move_foreground(card_custom_popup);
}

// 原确认按钮点击回调（改为触发弹窗）
static void card_confirm_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    // 1. 停止卡片动画
    stop_card_animation();

    // 2. 获取录入界面对象
    lv_obj_t *enroll_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(enroll_scr == NULL) {
        LV_LOG_WARN("card_confirm_btn_click_cb: enroll_scr is NULL!");
        return;
    }

    // 3. 弹出成功弹窗
    create_card_complete_popup(enroll_scr);
}

void ui_card_add_create(lv_obj_t *enroll_scr)
{
    init_card_add_styles();
    // 1. 安全校验
    if(enroll_scr == NULL) {
        LV_LOG_WARN("ui_card_add_create: enroll_scr is NULL!");
        return;
    }

    // 2. 创建/复用屏幕对象
    // if(card_add_scr == NULL) {
    //     card_add_scr = lv_obj_create(NULL);
    // } else {
    //     // 重置时先停止动画和定时器
    //     stop_card_animation();
    //     del_card_timeout_timer();
    //     close_card_popup();
    //     close_card_fail_popup();
    //     if(lv_obj_is_valid(card_add_scr)) {
    //         if(lv_scr_act() != card_add_scr) {
    //             lv_obj_clean(card_add_scr);
    //         } else {
    //             lv_obj_t *child;
    //             while((child = lv_obj_get_child(card_add_scr, 0)) != NULL) {
    //                 lv_obj_del(child);
    //             }
    //         }
    //     }
    //     lv_obj_clear_flag(card_add_scr, LV_OBJ_FLAG_HIDDEN);
    // }
    
    stop_card_animation();
    del_card_timeout_timer();
    close_card_popup();
    close_card_fail_popup();

    // 销毁旧屏幕 + 释放所有资源
    if(is_lv_obj_valid(card_add_scr)) {
        lv_obj_del(card_add_scr);  // 销毁整个屏幕，自动释放所有子控件，无需手动清理
        card_add_scr = NULL;       // 指针置空，杜绝野指针
    }

    // 重新创建全新屏幕
    card_add_scr = lv_obj_create(NULL);

    // 原有清除隐藏标志逻辑
    lv_obj_clear_flag(card_add_scr, LV_OBJ_FLAG_HIDDEN);

    // 3. 设置屏幕渐变样式
    lv_style_reset(&card_add_grad_style);
    lv_style_set_bg_color(&card_add_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&card_add_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&card_add_grad_style, LV_GRAD_DIR_VER);
    lv_style_set_bg_main_stop(&card_add_grad_style, 0);
    lv_style_set_bg_grad_stop(&card_add_grad_style, 255);
    lv_obj_add_style(card_add_scr, &card_add_grad_style, LV_STATE_DEFAULT);

    // 4. 标题
    lv_obj_t *add_card_title = create_text_label
    (card_add_scr, "Add Card", &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 0, 115, LV_OPA_100);
    lv_obj_align(add_card_title, LV_ALIGN_TOP_MID, 0, 115);

    // 5. 圆形背景容器
    lv_obj_t *add_card_circel01 = create_container
    (card_add_scr,195,302,410,410, lv_color_hex(0x051022), LV_OPA_100, 200,lv_color_hex(0x182E4E), 6, LV_OPA_100);
    lv_obj_t *add_card_circel02 = create_container
    (card_add_scr,222,327,356,356, lv_color_hex(0x061022), LV_OPA_100, 200,lv_color_hex(0x1D3861), 12, LV_OPA_100);    
    lv_obj_t *add_card_circel03 = create_container
    (card_add_scr,269,373,265,265, lv_color_hex(0x061022), LV_OPA_100, 200,lv_color_hex(0x1D3861), 8, LV_OPA_100); 
    lv_obj_t *add_card_circel04 = create_container
    (card_add_scr,307,412,188,188, lv_color_hex(0x061022), LV_OPA_100, 200,lv_color_hex(0x1D3861), 8, LV_OPA_100);     

    // 6. 卡片图片（核心：记录原始坐标，启动动画）
    card_img_origin_x = 371;  // 原始X坐标
    card_img_origin_y = 453;  // 原始Y坐标
    enter_card_img = create_image_obj(card_add_scr, "H:enter_card.png", card_img_origin_x, card_img_origin_y);

    // 7. 提示文本
    lv_obj_t *add_card_label = create_text_label
    (card_add_scr, "Please place the card...", &lv_font_montserrat_48, lv_color_hex(0xFFFFFF), 0, 924, LV_OPA_100);
    lv_obj_align(add_card_label, LV_ALIGN_TOP_MID, 0, 924);

    // 8. 确认录入按钮（模拟）
    lv_obj_t *confirm_enter_btn = create_container
    (card_add_scr,255,800,200,50, lv_color_hex(0x00BDBD), LV_OPA_100, 6,lv_color_hex(0x182E4E), 0, LV_OPA_100);
    lv_obj_add_flag(confirm_enter_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(confirm_enter_btn, LV_OPA_80, LV_STATE_PRESSED);  
    lv_obj_add_event_cb(confirm_enter_btn, card_confirm_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);

    // 9. 返回按钮
    lv_obj_t *back_btn = create_image_obj(card_add_scr, "H:back.png", 52, 123);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);
    // 绑定返回按钮时停止动画和定时器
    lv_obj_add_event_cb(back_btn, back_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);
    lv_obj_add_event_cb(back_btn, (lv_event_cb_t)stop_card_animation, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(back_btn, (lv_event_cb_t)del_card_timeout_timer, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(back_btn, (lv_event_cb_t)close_card_popup, LV_EVENT_CLICKED, NULL);

    // 10. 启动卡片动画（10ms刷新一次，流畅度高）
    if(card_anim_timer == NULL) {
        card_anim_timer = lv_timer_create(card_anim_timer_cb, 10, NULL);
    }

    // 11. 更新状态栏并切换界面
    update_status_bar_parent(card_add_scr);
    lv_scr_load(card_add_scr);

    // ========== 核心修改：界面初始化完成后启动3秒超时定时器 ==========
    // 1. 先销毁旧定时器（避免重复）
    del_card_timeout_timer();
    // 2. 保存用户数据到全局变量
    card_timeout_enroll_scr = enroll_scr;
    // 3. 启动3秒超时定时器（未点击确认按钮则触发失败弹窗）
    card_timeout_timer = lv_timer_create(card_timeout_timer_cb, MAX_CARD_TIMEOUT_MS, NULL);
}

/*********************** 卡片添加按钮点击回调 *********************/
void card_add_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    lv_obj_t *enroll_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(enroll_scr == NULL) {
        LV_LOG_WARN("card_add_btn_click_cb: enroll_scr is NULL!");
        return;
    }
    ui_card_add_create(enroll_scr);
}

// 可选：界面销毁时清理资源
void ui_card_add_destroy(void)
{
    stop_card_animation();
    del_card_timeout_timer();
    close_card_popup();
    close_card_fail_popup();
    if(card_add_scr != NULL && lv_obj_is_valid(card_add_scr)) {
        lv_obj_del(card_add_scr);
        card_add_scr = NULL;
    }
    enter_card_img = NULL;
    card_bg_mask_layer = NULL;
    card_custom_popup = NULL;
    card_name_keyboard = NULL;
    card_input_textarea = NULL;
    card_fail_popup = NULL;
}