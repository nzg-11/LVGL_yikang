#include "lv_finger_add.h"
// 引入录入界面头文件，使用全局变量和更新接口
#include "lv_a_enroll_opt.h"
// 引入必要头文件
#include <string.h>

// 全局变量：控制指纹添加进度和控件引用
static lv_obj_t *finger_add_scr = NULL; 
static lv_style_t finger_add_grad_style;
static bool finger_add_style_inited = false;

// 进度控制变量
static uint8_t finger_add_step = 0; // 0-4 对应0%/25%/50%/75%/100%
// 全局控件引用
static lv_obj_t *finger_percent_img = NULL;
static lv_obj_t *precent_label = NULL;
static lv_obj_t *prompt_label = NULL;
static lv_obj_t *add_finger_circel01 = NULL;
static lv_obj_t *add_finger_circel02 = NULL;
static lv_obj_t *add_finger_circel03 = NULL;
static lv_obj_t *finger_success_label = NULL; // 成功提示标签

// 弹窗相关全局变量
static lv_obj_t *finger_bg_mask_layer = NULL;  // 指纹弹窗遮罩层
static lv_obj_t *finger_custom_popup = NULL;   // 指纹弹窗主体
static lv_obj_t *finger_name_keyboard = NULL;  // 指纹弹窗键盘
static lv_obj_t *finger_input_textarea = NULL; // 指纹弹窗输入框
static void close_finger_popup(void);

// 指纹确认按钮回调
static void finger_confirm_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    // 1. 获取录入界面指针
    lv_obj_t *enroll_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(enroll_scr == NULL || !lv_obj_is_valid(enroll_scr)) {
        LV_LOG_WARN("finger_confirm_click_cb: enroll_scr invalid!");
        return;
    }

    // 2. 获取输入的指纹名称
    const char *finger_name = lv_textarea_get_text(finger_input_textarea);
    if(finger_name == NULL || strlen(finger_name) == 0) {
        finger_name = "Unnamed finger"; // 默认名称
    }

    // 3. 调用录入完成回调，更新主界面
    finger_enroll_complete(finger_name);

    // 4. 关闭弹窗
    close_finger_popup();
    update_status_bar_parent(enroll_scr);
    // 5. 返回录入界面（如果需要）
    lv_scr_load(enroll_scr);
    lv_obj_t *current_del_scr = lv_disp_get_scr_act(NULL);
    if(current_del_scr == finger_add_scr && is_lv_obj_valid(current_del_scr)) {
        lv_obj_del(finger_add_scr);  // 销毁编辑界面
        finger_add_scr = NULL;       // 指针置空
        LV_LOG_USER("Add finger screen destroyed successfully");
    }
    // 6. 重置指纹添加进度（原有逻辑）- 只重置进度，不重置计数
    finger_add_step = 0;
    lv_img_set_src(finger_percent_img, "H:finger_0.png");
    lv_label_set_text(precent_label, "0%");
    lv_label_set_text(prompt_label, "Please place your finger on the sensor");
    lv_obj_set_pos(finger_percent_img, 309, 415);
    lv_obj_set_style_border_opa(add_finger_circel01, LV_OPA_100, LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(add_finger_circel02, LV_OPA_100, LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(add_finger_circel03, LV_OPA_100, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(add_finger_circel03, lv_color_hex(0x061022), LV_STATE_DEFAULT);
    lv_obj_clear_flag(prompt_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(precent_label, LV_OBJ_FLAG_HIDDEN);
    if(finger_success_label != NULL) {
        lv_obj_add_flag(finger_success_label, LV_OBJ_FLAG_HIDDEN);
    }
}

// 全局样式初始化
static void init_other_member_styles(void)
{
    if(!finger_add_style_inited) {
        lv_style_init(&finger_add_grad_style);
        finger_add_style_inited = true;
    }
}

// 隐藏指纹弹窗键盘
static void hide_finger_keyboard(lv_event_t *e)
{
    (void)e;
    if(finger_name_keyboard != NULL && lv_obj_is_valid(finger_name_keyboard)) {
        lv_obj_add_flag(finger_name_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

// 指纹弹窗输入框点击回调
static void finger_input_click_cb(lv_event_t *e)
{
    //if(e == NULL) return;
    lv_obj_t *input = lv_event_get_target(e);
    
    // 1. 创建键盘
    finger_name_keyboard = lv_keyboard_create(lv_scr_act());
    // 设置键盘样式
    lv_obj_set_style_bg_color(finger_name_keyboard, lv_color_hex(0x192A46), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(finger_name_keyboard, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(finger_name_keyboard, lv_color_hex(0x1F3150), LV_STATE_DEFAULT);
    lv_obj_set_style_radius(finger_name_keyboard, 6, LV_STATE_DEFAULT);
    // 键盘尺寸适配屏幕
    lv_obj_set_size(finger_name_keyboard, LV_HOR_RES, LV_VER_RES/3);
    // 键盘底部对齐
    lv_obj_align(finger_name_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_event_cb(finger_name_keyboard, hide_finger_keyboard, LV_EVENT_READY, NULL);

    // 2. 显示键盘并关联输入框
    lv_obj_clear_flag(finger_name_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(finger_name_keyboard, input);
    
    // 3. 确保键盘在最上层
    //lv_obj_move_foreground(finger_name_keyboard);
}

// 关闭指纹弹窗的通用函数
static void close_finger_popup(void)
{
    // 销毁键盘
    if(finger_name_keyboard != NULL && lv_obj_is_valid(finger_name_keyboard)) {
        lv_obj_del(finger_name_keyboard);
        finger_name_keyboard = NULL;
    }
    
    // 销毁遮罩层
    if(finger_bg_mask_layer != NULL && lv_obj_is_valid(finger_bg_mask_layer)) {
        lv_obj_del(finger_bg_mask_layer);
        finger_bg_mask_layer = NULL;
    }
    
    // 销毁弹窗主体
    if(finger_custom_popup != NULL && lv_obj_is_valid(finger_custom_popup)) {
        lv_obj_del(finger_custom_popup);
        finger_custom_popup = NULL;
    }
}

// 创建指纹100%后的简化弹窗
static void create_finger_complete_popup(lv_obj_t *enroll_scr)
{
    // 1. 先关闭旧弹窗
    close_finger_popup();

    // 2. 创建/显示背景遮罩层
    finger_bg_mask_layer = lv_obj_create(finger_add_scr);
    lv_obj_set_size(finger_bg_mask_layer, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(finger_bg_mask_layer, 0, 0);
    lv_obj_set_style_bg_color(finger_bg_mask_layer, lv_color_hex(0x000000), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(finger_bg_mask_layer, LV_OPA_70, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(finger_bg_mask_layer, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(finger_bg_mask_layer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(finger_bg_mask_layer, LV_OBJ_FLAG_HIDDEN);

    // 3. 创建弹窗主体
    finger_custom_popup = create_container
    (finger_add_scr, 100, 455, 600, 270,  // 弹窗位置和尺寸（居中偏上）
     lv_color_hex(0xE0EDFF), LV_OPA_100,  // 背景色和透明度（和之前弹窗一致）
     16,                                   // 圆角
     lv_color_hex(0x1F3150), 0, LV_OPA_90); // 边框样式
    lv_obj_set_style_pad_all(finger_custom_popup, 0, LV_STATE_DEFAULT);

    lv_obj_t *succeed_add_label = create_text_label
    (finger_custom_popup, "succeed add", &lv_font_montserrat_24, lv_color_hex(0x000000), 0, 38, LV_OPA_100);
    lv_obj_align(succeed_add_label, LV_ALIGN_TOP_MID, 0, 38);
    // 4. 可输入的文本输入框（和之前的输入框样式一致）
    finger_input_textarea = lv_textarea_create(finger_custom_popup);
    lv_obj_clear_flag(finger_input_textarea, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(finger_input_textarea, 382, 44); // 输入框尺寸
    lv_obj_set_pos(finger_input_textarea, 137, 90);   // 输入框位置
    lv_obj_set_style_bg_color(finger_input_textarea, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(finger_input_textarea, lv_color_hex(0x333333), LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(finger_input_textarea, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(finger_input_textarea, 6, LV_STATE_DEFAULT);
    lv_textarea_set_placeholder_text(finger_input_textarea, "please input name"); // 占位符
    lv_textarea_set_max_length(finger_input_textarea, 8); // 最大输入长度
    lv_textarea_set_one_line(finger_input_textarea, true);
    lv_obj_set_style_text_font(finger_input_textarea, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    lv_obj_add_flag(finger_input_textarea, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(finger_input_textarea, LV_OPA_80, LV_STATE_PRESSED);
    // 绑定输入框点击回调（弹出键盘）
    lv_obj_add_event_cb(finger_input_textarea, finger_input_click_cb, LV_EVENT_CLICKED, NULL);

    // 确认按钮
    lv_obj_t *confirm_btn = create_custom_gradient_container
    (finger_custom_popup, 197, 171, 205, 44, 6, 0X006BDC, 0x00BDBD, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_add_flag(confirm_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(confirm_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_set_style_pad_all(confirm_btn, 0, LV_STATE_DEFAULT);
    // 确认按钮文本
    lv_obj_t *confirm_label = create_text_label
    (confirm_btn, "Confirm", &lv_font_montserrat_28, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_set_align(confirm_label, LV_ALIGN_CENTER);
    // 绑定自定义确认回调（更新计数+返回）
    lv_obj_add_event_cb(confirm_btn, finger_confirm_click_cb, LV_EVENT_CLICKED, enroll_scr);

    // 6. 确保弹窗在最上层
    //lv_obj_move_foreground(finger_custom_popup);
}

// 指纹添加点击回调（核心逻辑，  100%弹窗逻辑）
static void finger_add_click_cb(lv_event_t *e)
{
    if(e == NULL) return;

    // 步骤+1，最多到5（对应100%/成功）
    finger_add_step++;
    if(finger_add_step > 5) {
        finger_add_step = 0; // 测试阶段：完成后重置，正式场景可禁用点击
        // 重置所有进度状态（回到初始）- 不重置计数
        lv_img_set_src(finger_percent_img, "H:finger_0.png");
        lv_label_set_text(precent_label, "0%");
        lv_label_set_text(prompt_label, "Please place your finger on the sensor");
        lv_obj_set_pos(finger_percent_img, 309, 415); // 恢复初始位置
        
        // 恢复圆形边框样式
        lv_obj_set_style_border_opa(add_finger_circel01, LV_OPA_100, LV_STATE_DEFAULT);
        lv_obj_set_style_border_opa(add_finger_circel02, LV_OPA_100, LV_STATE_DEFAULT);
        lv_obj_set_style_border_opa(add_finger_circel03, LV_OPA_100, LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(add_finger_circel03, lv_color_hex(0x061022), LV_STATE_DEFAULT);
        
        // 显示提示文本，隐藏成功提示
        lv_obj_clear_flag(prompt_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(precent_label, LV_OBJ_FLAG_HIDDEN);
        if(finger_success_label != NULL) {
            lv_obj_add_flag(finger_success_label, LV_OBJ_FLAG_HIDDEN);
        }
        // 关闭弹窗（如果存在）
        close_finger_popup();
        // 重新启用点击
        lv_obj_add_flag(finger_percent_img, LV_OBJ_FLAG_CLICKABLE);
        return;
    }

    // 按步骤切换图标、文本、样式
    switch(finger_add_step) {
        case 1: // 第一步：25%
            lv_img_set_src(finger_percent_img, "H:finger_25.png"); // 替换为你的25%图片
            lv_label_set_text(precent_label, "25%");
            break;
        case 2: // 第二步：50%
            lv_img_set_src(finger_percent_img, "H:finger_50.png"); // 替换为你的50%图片
            lv_label_set_text(precent_label, "50%");
            break;
        case 3: // 第三步：75%
            lv_img_set_src(finger_percent_img, "H:finger_75.png"); // 替换为你的75%图片
            lv_label_set_text(precent_label, "75%");
            break;
        case 4: // 第四步：100%/成功
            lv_img_set_src(finger_percent_img, "H:finger_100.png"); // 替换为你的100%图片
            lv_label_set_text(precent_label, "100%");
            break;
        case 5: // 第五步：成功
            lv_obj_t *enroll_scr = (lv_obj_t *)lv_event_get_user_data(e);
            create_finger_complete_popup(enroll_scr);
            // 禁用指纹图片点击（防止重复触发）
            //lv_obj_clear_flag(finger_percent_img, LV_OBJ_FLAG_CLICKABLE);
            break;
        default: // 步骤0：初始状态（冗余，防止异常）
            lv_img_set_src(finger_percent_img, "H:finger_0.png");
            lv_label_set_text(precent_label, "0%");
            break;
    }
}

void ui_finger_add_create(lv_obj_t *enroll_scr)
{
    init_other_member_styles();
    // 1. 安全校验：如果传进来的 enroll_scr 为空，直接返回
    if(enroll_scr == NULL) {
        LV_LOG_WARN("ui_finger_add_create: enroll_scr is NULL!");
        return;
    }

    // 2. 创建/复用设置屏幕对象
    //关闭弹窗
    close_finger_popup();
    if(is_lv_obj_valid(finger_add_scr)) {
        lv_obj_del(finger_add_scr);  // 自动销毁屏幕+所有子控件，资源彻底释放
        finger_add_scr = NULL;       // 指针置空，杜绝野指针
    }
    finger_add_scr = lv_obj_create(NULL);
    finger_add_step = 0;//仅重置进度，不重置计数

    // 设置屏幕渐变样式（移到控件创建前，避免样式覆盖）
    lv_style_reset(&finger_add_grad_style);
    lv_style_set_bg_color(&finger_add_grad_style, lv_color_hex(0x010715));// 渐变主色：#010715（0%）
    lv_style_set_bg_grad_color(&finger_add_grad_style, lv_color_hex(0x0E1D37));// 渐变副色：#0E1D37（100%）
    lv_style_set_bg_grad_dir(&finger_add_grad_style, LV_GRAD_DIR_VER);// 渐变方向：垂直
    lv_style_set_bg_main_stop(&finger_add_grad_style, 0);// 渐变范围：0~255
    lv_style_set_bg_grad_stop(&finger_add_grad_style, 255);
    lv_obj_add_style(finger_add_scr, &finger_add_grad_style, LV_STATE_DEFAULT);// 应用渐变样式到屏幕

    //添加指纹相关文本
    lv_obj_t *add_finger_label = create_text_label
    (finger_add_scr, "add finger", &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 328, 115, LV_OPA_100);
    
    // 保存prompt_label到全局变量（方便回调修改）
    prompt_label = create_text_label
    (finger_add_scr, "Please place your finger on the sensor", &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 200, 780, LV_OPA_100);
    
    // 保存precent_label到全局变量（方便回调修改）
    precent_label = create_text_label
    (finger_add_scr, "0%", &lv_font_montserrat_48, lv_color_hex(0xFFFFFF), 380, 930, LV_OPA_100);

    // 保存圆形容器到全局变量（方便回调修改样式）
    add_finger_circel01 = create_container
    (finger_add_scr,195,302,410,410, lv_color_hex(0x051022), LV_OPA_100, 200,lv_color_hex(0x182E4E), 6, LV_OPA_100);
    add_finger_circel02 = create_container
    (finger_add_scr,222,327,356,356, lv_color_hex(0x061022), LV_OPA_100, 200,lv_color_hex(0x1D3861), 12, LV_OPA_100);    
    add_finger_circel03 = create_container
    (finger_add_scr,269,373,265,265, lv_color_hex(0x061022), LV_OPA_100, 200,lv_color_hex(0x1D3861), 8, LV_OPA_100); 

    //指纹百分比图片 - 保存到全局变量 + 绑定回调
    finger_percent_img = create_image_obj(finger_add_scr, "H:finger_0.png", 309, 415);
    lv_obj_add_flag(finger_percent_img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(finger_percent_img, LV_OPA_80, LV_STATE_PRESSED);
    // 绑定点击回调
    lv_obj_add_event_cb(finger_percent_img, finger_add_click_cb, LV_EVENT_CLICKED, enroll_scr);

    // 左上角返回按钮
    lv_obj_t *back_btn = create_image_obj(finger_add_scr, "H:back.png", 52, 123);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn, back_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);
    
    update_status_bar_parent(finger_add_scr);
    // 切换到设置屏幕
    lv_scr_load(finger_add_scr);
}

/***********************指纹添加界面回调*********************/
void finger_add_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    lv_obj_t *enroll_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(enroll_scr == NULL) {
        LV_LOG_WARN("finger_add_btn_click_cb: enroll_scr is NULL!");
        return;
    }
    ui_finger_add_create(enroll_scr);
}