#include "lv_face_add.h"
#include "lv_a_enroll_opt.h"  // 引入人脸录入完成回调
#include <string.h>

// ========== 你原来的扫描动画参数 ==========
#define SCAN_LINE_W    400  // 扫描线宽度
#define SCAN_LINE_H    8    // 扫描线高度
#define SCAN_LINE_X    200  // 扫描线X坐标
#define SCAN_LINE_START_Y 400  // 扫描线起始Y
#define SCAN_LINE_END_Y   780  // 扫描线结束Y
#define SCAN_ANIM_TIME 2000 // 扫描时间（ms）

// ========== 全局变量 ==========
static lv_obj_t *face_add_scr = NULL;  
static lv_style_t face_add_grad_style;
static bool face_add_style_inited = false;
static lv_obj_t *scan_line = NULL;     // 你原来的扫描线对象（保留）

// 弹窗相关全局变量
static lv_obj_t *face_bg_mask_layer = NULL;  // 人脸弹窗遮罩层
static lv_obj_t *face_custom_popup = NULL;   // 人脸弹窗主体
static lv_obj_t *face_name_keyboard = NULL;  // 人脸弹窗键盘
static lv_obj_t *face_input_textarea = NULL; // 人脸弹窗输入框
static lv_timer_t *face_timeout_timer = NULL;// 10秒超时定时器
static lv_obj_t *face_fail_popup = NULL;     // 失败弹窗
static lv_obj_t *face_timeout_enroll_scr = NULL; // 超时定时器用户数据

// ========== 你原来的扫描动画函数 ==========
// 扫描动画回调函数
static void scan_line_anim_cb(void *var, int32_t value)
{
    lv_obj_set_y(var, value);
}

// 启动扫描动画
static void start_scan_animation(lv_obj_t *bg_scr)
{
    // 1. 删除旧的扫描线
    if(scan_line != NULL) {
        lv_obj_del(scan_line);
    }

    // 3. 创建扫描线
    scan_line = lv_obj_create(bg_scr);
    lv_obj_set_size(scan_line, SCAN_LINE_W, SCAN_LINE_H);  // 固定宽高
    lv_obj_set_pos(scan_line, SCAN_LINE_X, SCAN_LINE_START_Y);  // 固定初始坐标
    
    // 4. 扫描线样式
    lv_obj_set_style_bg_color(scan_line, lv_color_hex(0x00FF00), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scan_line, LV_OPA_80, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scan_line, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(scan_line, 0, LV_STATE_DEFAULT);
    
    // 5. 动画参数
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, scan_line);
    lv_anim_set_exec_cb(&anim, scan_line_anim_cb);
    lv_anim_set_time(&anim, SCAN_ANIM_TIME);  // 固定扫描时间
    lv_anim_set_values(&anim, SCAN_LINE_START_Y, SCAN_LINE_END_Y);  // 固定移动范围
    lv_anim_set_playback_time(&anim, SCAN_ANIM_TIME);  // 固定回扫时间
    lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);  // 无限循环
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_start(&anim);
}

// 停止扫描动画
static void stop_scan_animation(void)
{
    if(scan_line != NULL) {
        lv_anim_del(scan_line, scan_line_anim_cb);  // 删除动画
        lv_obj_del(scan_line);                      // 删除扫描线对象
        scan_line = NULL;
    }
}

// ========== 弹窗工具函数 ==========
// 销毁超时定时器
static void del_face_timeout_timer(void)
{
    if(face_timeout_timer != NULL) {
        lv_timer_del(face_timeout_timer);
        face_timeout_timer = NULL;
    }
    face_timeout_enroll_scr = NULL;
}

// 关闭人脸成功弹窗
static void close_face_popup(void)
{
    // 隐藏键盘
    if(face_name_keyboard != NULL && lv_obj_is_valid(face_name_keyboard)) {
        lv_obj_add_flag(face_name_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
    // 隐藏遮罩层
    if(face_bg_mask_layer != NULL && lv_obj_is_valid(face_bg_mask_layer)) {
        lv_obj_add_flag(face_bg_mask_layer, LV_OBJ_FLAG_HIDDEN);
    }
    // 销毁弹窗主体
    if(face_custom_popup != NULL && lv_obj_is_valid(face_custom_popup)) {
        lv_obj_del(face_custom_popup);
        face_custom_popup = NULL;
    }
}

// 关闭失败弹窗
static void close_face_fail_popup(void)
{
    if(face_fail_popup != NULL && lv_obj_is_valid(face_fail_popup)) {
        lv_obj_del(face_fail_popup);
        face_fail_popup = NULL;
    }
    if(face_bg_mask_layer != NULL && lv_obj_is_valid(face_bg_mask_layer)) {
        lv_obj_add_flag(face_bg_mask_layer, LV_OBJ_FLAG_HIDDEN);
    }
}


// 失败弹窗确认回调
static void face_fail_confirm_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *enroll_scr = (lv_obj_t *)lv_event_get_user_data(e);
    close_face_fail_popup();
    // 返回原录入界面
    if(enroll_scr != NULL && lv_obj_is_valid(enroll_scr)) {
        lv_scr_load(enroll_scr);
    }
}

// 10秒超时回调
static void face_timeout_timer_cb(lv_timer_t *timer)
{
    stop_scan_animation();
    (void)timer;
    lv_obj_t *enroll_scr = face_timeout_enroll_scr;
    
    // 1. 销毁定时器
    del_face_timeout_timer();
    
    // 2. 校验界面有效性
    if(face_add_scr == NULL || !lv_obj_is_valid(face_add_scr)) {
        return;
    }

    // 3. 创建遮罩层
    face_bg_mask_layer = lv_obj_create(face_add_scr);
    lv_obj_set_size(face_bg_mask_layer, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(face_bg_mask_layer, 0, 0);
    lv_obj_set_style_bg_color(face_bg_mask_layer, lv_color_hex(0x000000), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(face_bg_mask_layer, LV_OPA_70, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(face_bg_mask_layer, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(face_bg_mask_layer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(face_bg_mask_layer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(face_bg_mask_layer);

    // 4. 创建失败弹窗主体
    face_fail_popup = create_container
    (face_add_scr, 100, 455, 600, 200,
     lv_color_hex(0xE0EDFF), LV_OPA_100,
     16, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(face_fail_popup, 0, LV_STATE_DEFAULT);
    lv_obj_move_foreground(face_fail_popup);

    // 5. 失败提示文本
    lv_obj_t *fail_label = create_text_label
    (face_fail_popup, "Add failed (timeout)", &lv_font_montserrat_24, lv_color_hex(0xFF0000), 0, 38, LV_OPA_100);
    lv_obj_align(fail_label, LV_ALIGN_TOP_MID, 0, 38);

    // 6. 失败弹窗确认按钮
    lv_obj_t *fail_confirm_btn = create_custom_gradient_container
    (face_fail_popup, 197, 120, 205, 44, 6, 0X006BDC, 0x00BDBD, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_add_flag(fail_confirm_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(fail_confirm_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_set_style_pad_all(fail_confirm_btn, 0, LV_STATE_DEFAULT);

    lv_obj_t *fail_confirm_label = create_text_label
    (fail_confirm_btn, "Confirm", &lv_font_montserrat_28, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_set_align(fail_confirm_label, LV_ALIGN_CENTER);
    lv_obj_add_event_cb(fail_confirm_btn, face_fail_confirm_cb, LV_EVENT_CLICKED, enroll_scr);
}

// 隐藏键盘
static void hide_face_keyboard(lv_event_t *e)
{
    (void)e;
    if(face_name_keyboard != NULL && lv_obj_is_valid(face_name_keyboard)) {
        lv_obj_add_flag(face_name_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

// 输入框点击回调
static void face_input_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *input = lv_event_get_target(e);
    
    // 创建键盘
    if(face_name_keyboard == NULL || !lv_obj_is_valid(face_name_keyboard)) {
        face_name_keyboard = lv_keyboard_create(lv_scr_act());
        lv_obj_set_style_bg_color(face_name_keyboard, lv_color_hex(0x192A46), LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(face_name_keyboard, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(face_name_keyboard, lv_color_hex(0x1F3150), LV_STATE_DEFAULT);
        lv_obj_set_style_radius(face_name_keyboard, 6, LV_STATE_DEFAULT);
        lv_obj_set_size(face_name_keyboard, LV_HOR_RES, LV_VER_RES/3);
        lv_obj_align(face_name_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_add_event_cb(face_name_keyboard, hide_face_keyboard, LV_EVENT_READY, NULL);
    }
    
    // 显示键盘并关联输入框
    lv_obj_clear_flag(face_name_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(face_name_keyboard, input);
    lv_obj_move_foreground(face_name_keyboard);
}

// 成功弹窗确认回调
static void face_popup_confirm_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    // 1. 获取原录入界面
    lv_obj_t *enroll_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(enroll_scr == NULL || !lv_obj_is_valid(enroll_scr)) {
        LV_LOG_WARN("face_popup_confirm_cb: enroll_scr invalid!");
        return;
    }

    // 2. 获取输入的人脸名称
    const char *face_name = lv_textarea_get_text(face_input_textarea);
    if(face_name == NULL || strlen(face_name) == 0) {
        face_name = "face01";
    }

    // 3. 调用人脸录入完成回调
    face_enroll_complete(face_name);

    // 4. 关闭弹窗
    close_face_popup();

    // 5. 返回原界面
    update_status_bar_parent(enroll_scr);
    lv_scr_load(enroll_scr);

    // 6. 隐藏当前界面
    if(face_add_scr != NULL && lv_obj_is_valid(face_add_scr)) {
        lv_obj_add_flag(face_add_scr, LV_OBJ_FLAG_HIDDEN);
    }
}

// 创建设备录入成功弹窗
static void create_face_complete_popup(lv_obj_t *enroll_scr)
{
    // 先关闭旧弹窗
    close_face_popup();

    // 1. 销毁超时定时器
    del_face_timeout_timer();

    // 2. 创建遮罩层
    face_bg_mask_layer = lv_obj_create(face_add_scr);
    lv_obj_set_size(face_bg_mask_layer, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(face_bg_mask_layer, 0, 0);
    lv_obj_set_style_bg_color(face_bg_mask_layer, lv_color_hex(0x000000), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(face_bg_mask_layer, LV_OPA_70, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(face_bg_mask_layer, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(face_bg_mask_layer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(face_bg_mask_layer, LV_OBJ_FLAG_HIDDEN);

    // 3. 创建弹窗主体
    face_custom_popup = create_container
    (face_add_scr, 100, 455, 600, 270,
     lv_color_hex(0xE0EDFF), LV_OPA_100,
     16, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(face_custom_popup, 0, LV_STATE_DEFAULT);

    // 4. 成功提示文本
    lv_obj_t *succeed_label = create_text_label
    (face_custom_popup, "succeed add", &lv_font_montserrat_24, lv_color_hex(0x000000), 0, 38, LV_OPA_100);
    lv_obj_align(succeed_label, LV_ALIGN_TOP_MID, 0, 38);

    // 5. 输入框
    face_input_textarea = lv_textarea_create(face_custom_popup);
    lv_obj_clear_flag(face_input_textarea, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(face_input_textarea, 382, 44);
    lv_obj_set_pos(face_input_textarea, 137, 90);
    lv_obj_set_style_bg_color(face_input_textarea, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(face_input_textarea, lv_color_hex(0x333333), LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(face_input_textarea, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(face_input_textarea, 6, LV_STATE_DEFAULT);
    lv_textarea_set_placeholder_text(face_input_textarea, "please input face name");
    lv_textarea_set_max_length(face_input_textarea, 8);
    lv_textarea_set_one_line(face_input_textarea, true);
    lv_obj_set_style_text_font(face_input_textarea, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    lv_obj_add_flag(face_input_textarea, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(face_input_textarea, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(face_input_textarea, face_input_click_cb, LV_EVENT_CLICKED, NULL);

    // 6. 确认按钮
    lv_obj_t *confirm_btn = create_custom_gradient_container
    (face_custom_popup, 197, 171, 205, 44, 6, 0X006BDC, 0x00BDBD, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_add_flag(confirm_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(confirm_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_set_style_pad_all(confirm_btn, 0, LV_STATE_DEFAULT);

    lv_obj_t *confirm_label = create_text_label
    (confirm_btn, "Confirm", &lv_font_montserrat_28, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_set_align(confirm_label, LV_ALIGN_CENTER);
    lv_obj_add_event_cb(confirm_btn, face_popup_confirm_cb, LV_EVENT_CLICKED, enroll_scr);

    // 7. 弹窗置顶
    lv_obj_move_foreground(face_custom_popup);
}

// ========== 确认按钮点击回调 ==========
static void face_confirm_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    // 1. 停止扫描动画
    stop_scan_animation();

    // 2. 获取录入界面
    lv_obj_t *enroll_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(enroll_scr == NULL) {
        LV_LOG_WARN("face_confirm_btn_click_cb: enroll_scr is NULL!");
        return;
    }

    // 3. 弹出成功弹窗
    create_face_complete_popup(enroll_scr);
}

// ========== 全局样式初始化 ==========
static void init_face_add_styles(void)
{
    if(!face_add_style_inited) {
        lv_style_init(&face_add_grad_style);
        face_add_style_inited = true;
    }
}

// ========== 人脸录入界面创建 ==========
void ui_face_add_create(lv_obj_t *enroll_scr)
{
    init_face_add_styles();
    // 1. 安全校验
    if(enroll_scr == NULL) {
        LV_LOG_WARN("ui_face_add_create: enroll_scr is NULL!");
        return;
    }

    // 2. 创建/复用屏幕对象
    // if(face_add_scr == NULL) {
    //     face_add_scr = lv_obj_create(NULL);
    // } else {
    //     // 重置时先停止动画和定时器
    //     stop_scan_animation();
    //     del_face_timeout_timer();
    //     close_face_popup();
    //     close_face_fail_popup();
    //     if(lv_obj_is_valid(face_add_scr)) {
    //         if(lv_scr_act() != face_add_scr) {
    //             lv_obj_clean(face_add_scr);
    //         } else {
    //             lv_obj_t *child;
    //             while((child = lv_obj_get_child(face_add_scr, 0)) != NULL) {
    //                 lv_obj_del(child);
    //             }
    //         }
    //     }
    //     lv_obj_clear_flag(face_add_scr, LV_OBJ_FLAG_HIDDEN);
    // }
    
    stop_scan_animation();
    del_face_timeout_timer();
    close_face_popup();
    close_face_fail_popup();

    //销毁旧屏幕，释放全部资源
    if(is_lv_obj_valid(face_add_scr)) {
        lv_obj_del(face_add_scr);  // 自动销毁屏幕+所有子控件，无需手动清空
        face_add_scr = NULL;       // 指针置空，杜绝野指针
    }

    // 重新创建全新的屏幕对象
    face_add_scr = lv_obj_create(NULL);

    // 清除隐藏标志，保证屏幕正常显示
    lv_obj_clear_flag(face_add_scr, LV_OBJ_FLAG_HIDDEN);

    // 3. 设置屏幕渐变样式
    lv_style_reset(&face_add_grad_style);
    lv_style_set_bg_color(&face_add_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&face_add_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&face_add_grad_style, LV_GRAD_DIR_VER);
    lv_style_set_bg_main_stop(&face_add_grad_style, 0);
    lv_style_set_bg_grad_stop(&face_add_grad_style, 255);
    lv_obj_add_style(face_add_scr, &face_add_grad_style, LV_STATE_DEFAULT);

    // 左上角返回按钮
    lv_obj_t *back_btn = create_image_obj(face_add_scr, "H:back.png", 52, 123);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn, back_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);
    // 返回按钮绑定停止动画和清理弹窗
    lv_obj_add_event_cb(back_btn, (lv_event_cb_t)stop_scan_animation, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(back_btn, (lv_event_cb_t)del_face_timeout_timer, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(back_btn, (lv_event_cb_t)close_face_popup, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(back_btn, (lv_event_cb_t)close_face_fail_popup, LV_EVENT_CLICKED, NULL);

    lv_obj_t *face_add_label = create_text_label
    (face_add_scr, "add face", &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 0, 115, LV_OPA_100);
    lv_obj_align(face_add_label, LV_ALIGN_TOP_MID, 0, 115);

    lv_obj_t *face_prompt_label = create_text_label
    (face_add_scr, "please put your face into the box", &lv_font_montserrat_40, lv_color_hex(0xFFFFFF), 0, 217, LV_OPA_100);
    lv_obj_align(face_prompt_label, LV_ALIGN_TOP_MID, 0, 217);
    //人脸录入容器
    lv_obj_t *face_enroll_con = create_container    
    (face_add_scr, 200, 395, 400, 400, lv_color_hex(0xFFFFFF), LV_OPA_100, 10, lv_color_hex(0x009999), 0, LV_OPA_90);
    
    // 启动扫描动画
    start_scan_animation(face_add_scr);

    // 5. 确认添加人脸按钮
    lv_obj_t *confirm_btn_con = create_container
    (face_add_scr, 200, 830,200, 50, lv_color_hex(0x00BDBD), LV_OPA_100, 50, lv_color_hex(0x009999), 0, LV_OPA_90);
    lv_obj_add_flag(confirm_btn_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(confirm_btn_con, LV_OPA_80, LV_STATE_PRESSED);
    // 添加按钮点击事件
    lv_obj_add_event_cb(confirm_btn_con, face_confirm_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);

    // 6. 更新状态栏并切换到人脸添加界面
    update_status_bar_parent(face_add_scr);
    lv_scr_load(face_add_scr);

    // ========== 启动10秒超时定时器 ==========
    del_face_timeout_timer();
    face_timeout_enroll_scr = enroll_scr;
    face_timeout_timer = lv_timer_create(face_timeout_timer_cb, 3000, NULL);
}

// ========== 人脸添加按钮点击回调 ==========
void face_add_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    lv_obj_t *enroll_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(enroll_scr == NULL) {
        LV_LOG_WARN("face_add_btn_click_cb: enroll_scr is NULL!");
        return;
    }
    ui_face_add_create(enroll_scr);
}

// ========== 界面销毁清理 ==========
void ui_face_add_destroy(void)
{
    stop_scan_animation();
    del_face_timeout_timer();
    close_face_popup();
    close_face_fail_popup();
    if(face_add_scr != NULL && lv_obj_is_valid(face_add_scr)) {
        lv_obj_del(face_add_scr);
        face_add_scr = NULL;
    }
    face_bg_mask_layer = NULL;
    face_custom_popup = NULL;
    face_name_keyboard = NULL;
    face_input_textarea = NULL;
    face_fail_popup = NULL;
}