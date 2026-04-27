#include "lv_dev_info.h"

#ifdef LV_DEMO_EQUES
#if LV_EQUES_VER    //竖屏

#else    //横屏
 lv_obj_t *dev_info_scr = NULL;
static lv_obj_t *dev_name_label = NULL; // 保存设备昵称标签，用于更新

// 键盘相关全局变量
static lv_obj_t *dev_name_keyboard = NULL;
static void dev_info_blank_click_cb(lv_event_t *e);
// 全局样式
static lv_style_t dev_info_grad_style;
static bool dev_info_style_inited = false;
static void dev_info_destroy(void);
void dev_info_back_btn_click_cb(lv_event_t *e);
// 全局样式初始化
static void init_msg_center_styles(void)
{
    if(!dev_info_style_inited) {
        lv_style_init(&dev_info_grad_style);
        dev_info_style_inited = true;
    }
}

// 隐藏键盘
static void hide_dev_name_keyboard(lv_event_t *e)
{
    (void)e;
    if(lv_obj_is_valid(dev_name_keyboard))
    {
        lv_obj_add_flag(dev_name_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}
// 输入框点击回调 —— 弹出键盘
static void dev_name_input_click_cb(lv_event_t *e)
{
    lv_obj_t *input = lv_event_get_target(e);

    // 如果键盘已存在，先销毁
    if(lv_obj_is_valid(dev_name_keyboard)) {
        lv_obj_del(dev_name_keyboard);
        dev_name_keyboard = NULL;
    }

    // 创建键盘（样式完全参考你的指纹界面）
    dev_name_keyboard = lv_keyboard_create(lv_scr_act());
    lv_obj_set_style_bg_color(dev_name_keyboard, lv_color_hex(0x192A46), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(dev_name_keyboard, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(dev_name_keyboard, lv_color_hex(0x1F3150), LV_STATE_DEFAULT);
    lv_obj_set_style_radius(dev_name_keyboard, 6, LV_STATE_DEFAULT);
    lv_obj_set_size(dev_name_keyboard, LV_HOR_RES, 250);
    lv_obj_align(dev_name_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    
    // 绑定完成事件：点完成隐藏键盘
    lv_obj_add_event_cb(dev_name_keyboard, hide_dev_name_keyboard, LV_EVENT_READY, NULL);

    // 显示键盘并关联输入框
    lv_obj_clear_flag(dev_name_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(dev_name_keyboard, input);
}

/***********************确认修改昵称回调*********************/
void dev_info_name_confirm_cb(lv_event_t *e)
{
    if(e == NULL) return;

    // 销毁键盘
    if(lv_obj_is_valid(dev_name_keyboard)) {
        lv_obj_del(dev_name_keyboard);
        dev_name_keyboard = NULL;
    }

    lv_obj_t *mask = (lv_obj_t *)lv_event_get_user_data(e);
    if(mask == NULL) {
        LV_LOG_WARN("dev_info_name_confirm_cb: mask is NULL!");
        return;
    }

    // 找到输入框：mask的第4个子对象 (edit_dialog(0), title(1), hint(2), hint2(3), input(4), confirm_btn(5))
    lv_obj_t *input = lv_obj_get_child(mask, 4);
    if(input && dev_name_label) {
        const char *new_name = lv_textarea_get_text(input);
        if(new_name && strlen(new_name) > 0) {
            lv_label_set_text(dev_name_label, new_name);
        }
    }

    // 销毁遮罩层和所有子对象
    lv_obj_del(mask);
}

/***********************创建遮罩层*********************/
void dev_info_name_edit_cb(lv_event_t *e)
{
    if(e == NULL) return;

    lv_obj_t *scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(scr == NULL) {
        LV_LOG_WARN("dev_info_name_edit_cb: scr is NULL!");
        return;
    }

    // 创建遮罩层
    lv_obj_t *mask = lv_obj_create(scr);
    lv_obj_set_size(mask, lv_obj_get_width(scr), lv_obj_get_height(scr));
    lv_obj_set_style_radius(mask, 0, 0);
    lv_obj_set_style_bg_color(mask, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(mask, LV_OPA_50, 0);
    lv_obj_set_pos(mask, 0, 0); // 屏幕坐标 (100, 455)
    lv_obj_move_foreground(mask);
    lv_obj_set_style_border_opa(mask, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(mask, LV_OBJ_FLAG_CLICKABLE); // 避免点击穿透
    lv_obj_set_style_pad_all(mask, 0, LV_STATE_DEFAULT);// 移除内边距
    lv_obj_clear_flag(mask, LV_OBJ_FLAG_SCROLLABLE);// 移除滚动功能

    // 创建修改昵称弹窗（父对象：mask，而非scr）
    lv_obj_t *edit_dialog = lv_obj_create(mask);
    lv_obj_set_size(edit_dialog, 600, 297);//占满mask
    lv_obj_set_pos(edit_dialog, 212, 149);
    lv_obj_set_style_radius(edit_dialog, 16, 0);
    lv_obj_set_style_bg_color(edit_dialog, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_color(edit_dialog, lv_color_hex(0x2E4B7D), 0);
    lv_obj_set_style_border_width(edit_dialog, 0, 0);
    lv_obj_set_style_border_opa(edit_dialog, LV_OPA_100, 0);


    // 添加输入框（父对象：mask，坐标相对mask）
    lv_obj_t *input = lv_textarea_create(mask);
    if(input) {
        lv_obj_set_size(input, 382, 44);
        lv_obj_set_pos(input, 348, 264);
        lv_textarea_set_placeholder_text(input, "change_nickname_placeholder");
        lv_textarea_set_text(input, "text");
        lv_obj_clear_flag(input, LV_OBJ_FLAG_SCROLLABLE);
        lv_textarea_set_max_length(input, 8);
        

        // ========== 绑定点击事件：点击输入框弹出键盘 ==========
        lv_obj_add_flag(input, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(input, dev_name_input_click_cb, LV_EVENT_CLICKED, NULL);
    }

    // 7. 添加确认按钮（父对象：mask，坐标相对mask）
    // 原屏幕坐标 (296, 651) -> 相对mask坐标 (296-100, 651-455) = (196, 196)
    lv_obj_t *confirm_btn = lv_btn_create(mask);
    if(confirm_btn) {
        lv_obj_set_size(confirm_btn, 205, 44);
        lv_obj_set_pos(confirm_btn, 452, 352);
        // 按钮渐变样式
        lv_obj_set_style_bg_color(confirm_btn, lv_color_hex(0x006BDC), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_color(confirm_btn, lv_color_hex(0x00BDBD), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_dir(confirm_btn, LV_GRAD_DIR_HOR, LV_STATE_DEFAULT);
        lv_obj_set_style_bg_main_stop(confirm_btn, 0, LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_stop(confirm_btn, 255, LV_STATE_DEFAULT);
        // 按下态样式
        lv_obj_set_style_bg_color(confirm_btn, lv_color_hex(0x192A46), LV_STATE_PRESSED);
        lv_obj_set_style_radius(confirm_btn, 6, 0);

        // 8. 按钮文本（父对象：confirm_btn，坐标相对按钮）
        // lv_obj_t *confirm_label = create_text_label(confirm_btn, "confirm_label", &eques_bold_24, lv_color_hex(0xFFFFFF), 44, 0, LV_OPA_100);
        // if(confirm_label) {
        //     lv_label_set_text(confirm_label, "button");
        // }

        // 绑定点击事件
        // mask遮罩层点击 → 关闭键盘
        lv_obj_add_event_cb(mask, hide_dev_name_keyboard, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(confirm_btn, dev_info_name_confirm_cb, LV_EVENT_CLICKED, mask);
    }
}

void ui_dev_info_create(lv_obj_t *homepage_scr)
{
    init_msg_center_styles();
    if(homepage_scr == NULL) {
        LV_LOG_WARN("ui_dev_info_create: homepage_scr is NULL!");
        return;
    }

    // 1. 关键：如果旧设置页存在，先销毁！释放所有内存
    if(is_lv_obj_valid(dev_info_scr)) {
        lv_obj_del(dev_info_scr);
        dev_info_scr = NULL;
    }
    dev_info_scr = lv_obj_create(NULL);

    // 重置渐变样式
    lv_style_reset(&dev_info_grad_style);
    lv_style_set_bg_color(&dev_info_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&dev_info_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&dev_info_grad_style, LV_GRAD_DIR_VER);
    lv_style_set_bg_main_stop(&dev_info_grad_style, 0);
    lv_style_set_bg_grad_stop(&dev_info_grad_style, 255);
    lv_obj_add_style(dev_info_scr, &dev_info_grad_style, LV_STATE_DEFAULT);

    // 添加标题
    create_text_label(dev_info_scr, "设备信息", &eques_bold_36, lv_color_hex(0xFFFFFF), 83, 80, LV_OPA_100);

    // 设备昵称容器
    lv_obj_t *dev_con1 = create_container
    (dev_info_scr, 48, 150, 928, 83, lv_color_hex(0x192A46), LV_OPA_100, 16, lv_color_hex(0x2E4B7D), 0, LV_OPA_0);
    if(dev_con1) {
        lv_obj_add_flag(dev_con1, LV_OBJ_FLAG_CLICKABLE);
        //lv_obj_set_style_bg_opa(dev_con1, LV_OPA_70, LV_STATE_PRESSED);
        //lv_obj_add_event_cb(dev_con1, dev_info_name_edit_cb, LV_EVENT_CLICKED, dev_info_scr);
    }

    // 设备昵称标签
    lv_obj_t *dev_name_title = create_text_label(dev_con1, "设备昵称", &eques_regular_36, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align(dev_name_title, LV_ALIGN_LEFT_MID, 65, 0);
    dev_name_label = create_text_label(dev_con1, "可视智能锁", &eques_regular_24, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_50);
    lv_obj_align(dev_name_label, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_t *dev_tag_label = create_text_label(dev_con1, ICON_TAG, &my_custom_icon, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align(dev_tag_label, LV_ALIGN_LEFT_MID, 10, 0);

    // 设备编号容器
    lv_obj_t *dev_con = create_container(dev_info_scr, 48, 241, 928, 83, lv_color_hex(0x192A46), LV_OPA_100, 16, lv_color_hex(0x2E4B7D), 0, LV_OPA_0);
    lv_obj_t *dev_id_label = create_text_label(dev_con, "设备号", &eques_regular_36, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align(dev_id_label, LV_ALIGN_LEFT_MID, 65, 0);
    lv_obj_t *dev_id_num_label = create_text_label(dev_con, "12345678", &eques_regular_24, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_50);
    lv_obj_align(dev_id_num_label, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_t *dev_server_label = create_text_label(dev_con, ICON_SERVER, &my_custom_icon, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align(dev_server_label, LV_ALIGN_LEFT_MID, 10, 0);    
    // WiFi信息容器
    lv_obj_t *wifi_con = create_container(dev_info_scr, 48, 332, 928, 83, lv_color_hex(0x192A46), LV_OPA_100, 16, lv_color_hex(0x2E4B7D), 0, LV_OPA_0);
    create_text_label(dev_info_scr, "当前网络", &eques_regular_36, lv_color_hex(0xFFFFFF), 130, 350, LV_OPA_100);
    lv_obj_t *wifi_name_label = create_text_label(wifi_con, "wifi-name", &eques_regular_24, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_50);
    lv_obj_align(wifi_name_label, LV_ALIGN_RIGHT_MID, -60, 0);
    create_text_label(dev_info_scr, ICON_WIFI, &my_custom_icon, lv_color_hex(0xC4C4C4), 912, 354, LV_OPA_100);
    lv_obj_t *wifi_tag_label = create_text_label(wifi_con, ICON_PASSPORT, &my_custom_icon, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align(wifi_tag_label, LV_ALIGN_LEFT_MID, 10, 0);
    // 返回
    lv_obj_t *back_btn = create_text_label
    (dev_info_scr, ICON_CHEVORN_LEFT, &my_custom_icon, lv_color_hex(0xFFFFFF), 52, 84, LV_OPA_100);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(back_btn,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn,LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn, dev_info_back_btn_click_cb, LV_EVENT_CLICKED, homepage_scr);

    //更新状态条父对象
    update_status_bar_parent(dev_info_scr);
    // 切换到设置屏幕
    lv_scr_load(dev_info_scr);
}

/***********************设备信息界面回调*********************/
extern void lv_homepage(void);
extern void destroy_homepage(void);
void dev_info_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;

    lv_obj_t *homepage_scr_temp = (lv_obj_t *)lv_event_get_user_data(e);
    if(homepage_scr_temp == NULL) {
        LV_LOG_WARN("dev_info_back_btn_click_cb: homepage_scr is NULL!");
        return;
    }

    // 创建设备信息界面
    ui_dev_info_create(homepage_scr_temp);
    // 更新状态栏
    
    // 销毁主页
    destroy_homepage();
    update_status_bar_parent(dev_info_scr);
    LV_LOG_WARN("dev_info_back_btn_click_cb: Destroy the homepage and create the dev_info interface");
}
// 设备信息界面返回
void dev_info_back_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;

    lv_obj_t *current_del_scr = lv_disp_get_scr_act(NULL);

    if(!lv_obj_is_valid(current_del_scr)) return;

    // 当前显示的是设备信息界面 → 重建主页并销毁当前界面
    if(current_del_scr == dev_info_scr) {
        lv_homepage();                      // 重建主页
        lv_obj_del(current_del_scr);        // 销毁设备信息界面
        dev_info_scr = NULL;            // 指针置空
        return;
    }
}
// 销毁设备信息界面所有资源
static void dev_info_destroy(void)
{
    // 1. 销毁键盘
    if (lv_obj_is_valid(dev_name_keyboard)) {
        lv_obj_del(dev_name_keyboard);
        dev_name_keyboard = NULL;
    }

    // 2. 销毁界面
    if (lv_obj_is_valid(dev_info_scr)) {
        lv_obj_del(dev_info_scr);
        dev_info_scr = NULL;
    }

    // 3. 释放样式
    if (dev_info_style_inited) {
        lv_style_reset(&dev_info_grad_style);
        dev_info_style_inited = false;
    }

    // 4. 全局指针全部清空
    dev_name_label = NULL;
}

#endif

#endif