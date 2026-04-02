#include "lv_dev_info.h"


static lv_obj_t *dev_info_scr = NULL; 

// 全局样式
static lv_style_t dev_info_grad_style;
static bool dev_info_style_inited = false;

// 全局样式初始化
static void init_msg_center_styles(void)
{
    if(!dev_info_style_inited) {
        lv_style_init(&dev_info_grad_style);
        dev_info_style_inited = true;
    }
}



void ui_dev_info_create(lv_obj_t *homepage_scr)
{
    init_msg_center_styles();
    // 1. 安全校验：如果传进来的 homepage_scr 为空，直接返回
    if(homepage_scr == NULL) {
        LV_LOG_WARN("ui_dev_info_create: homepage_scr is NULL!");
        return;
    }

    // 2. 创建/复用设置屏幕对象
    if(dev_info_scr == NULL) {
        dev_info_scr = lv_obj_create(NULL);  // 创建独立屏幕
    } else {
        lv_obj_clean(dev_info_scr);          // 清空原有内容
    }
        // 重置样式（替代重复init）
    lv_style_reset(&dev_info_grad_style);
    lv_style_set_bg_color(&dev_info_grad_style, lv_color_hex(0x010715));// 渐变主色：#010715（0%）
    lv_style_set_bg_grad_color(&dev_info_grad_style, lv_color_hex(0x0E1D37));// 渐变副色：#0E1D37（100%）
    lv_style_set_bg_grad_dir(&dev_info_grad_style, LV_GRAD_DIR_VER);// 渐变方向：垂直
    lv_style_set_bg_main_stop(&dev_info_grad_style, 0);// 渐变范围：0~255
    lv_style_set_bg_grad_stop(&dev_info_grad_style, 255);
    lv_obj_add_style(dev_info_scr, &dev_info_grad_style, LV_STATE_DEFAULT);// 应用渐变样式到屏幕

    // 左上角返回按钮
    lv_obj_t *back_btn = create_image_obj(dev_info_scr, "H:back.png", 52, 123);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn, back_btn_click_cb, LV_EVENT_CLICKED, homepage_scr);

    //更新状态条父对象
    update_status_bar_parent(dev_info_scr);
    // 切换到设置屏幕
    lv_scr_load(dev_info_scr);
}





/***********************设备信息界面回调*********************/
void dev_info_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    lv_obj_t *homepage_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(homepage_scr == NULL) {
        LV_LOG_WARN("dev_info_btn_click_cb: homepage_scr is NULL!");
        return;
    }
    ui_dev_info_create(homepage_scr);

}
