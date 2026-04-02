#include "lv_sys_settings.h"


static lv_obj_t *sys_settings_scr = NULL; 

static lv_style_t sys_settings_grad_style;
static bool sys_settings_style_inited = false;

// 全局样式初始化
static void init_sys_settings_styles(void)
{
    if(!sys_settings_style_inited) {
        lv_style_init(&sys_settings_grad_style);
        sys_settings_style_inited = true;
    }
}

void ui_sys_settings_create(lv_obj_t *homepage_scr)
{
    init_sys_settings_styles();
    // 1. 安全校验：如果传进来的 homepage_scr 为空，直接返回
    if(homepage_scr == NULL) {
        LV_LOG_WARN("ui_sys_settings_create: homepage_scr is NULL!");
        return;
    }

    // 2. 创建/复用设置屏幕对象
    if(sys_settings_scr == NULL) {
        sys_settings_scr = lv_obj_create(NULL);  // 创建独立屏幕
    } else {
        lv_obj_clean(sys_settings_scr);          // 清空原有内容
    }
    
    lv_style_reset(&sys_settings_grad_style);
    lv_style_set_bg_color(&sys_settings_grad_style, lv_color_hex(0x010715));// 渐变主色：#010715（0%）
    lv_style_set_bg_grad_color(&sys_settings_grad_style, lv_color_hex(0x0E1D37));// 渐变副色：#0E1D37（100%）
    lv_style_set_bg_grad_dir(&sys_settings_grad_style, LV_GRAD_DIR_VER);// 渐变方向：垂直
    lv_style_set_bg_main_stop(&sys_settings_grad_style, 0);// 渐变范围：0~255
    lv_style_set_bg_grad_stop(&sys_settings_grad_style, 255);
    lv_obj_add_style(sys_settings_scr, &sys_settings_grad_style, LV_STATE_DEFAULT);// 应用渐变样式到屏幕
    
    
    // 左上角返回按钮
    lv_obj_t *back_btn = create_image_obj(sys_settings_scr, "H:back.png", 52, 123);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn, back_btn_click_cb, LV_EVENT_CLICKED, homepage_scr);
    
    //更新状态条父对象
    update_status_bar_parent(sys_settings_scr);
    // 切换到设置屏幕
    lv_scr_load(sys_settings_scr);
}





/***********************系统设置界面回调*********************/
void sys_settings_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    lv_obj_t *homepage_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(homepage_scr == NULL) {
        LV_LOG_WARN("sys_settings_btn_click_cb: homepage_scr is NULL!");
        return;
    }
    ui_sys_settings_create(homepage_scr);
}
