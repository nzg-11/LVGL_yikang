#include "lv_user_permission.h"

static lv_obj_t *user_permission_scr = NULL; 

static lv_style_t user_permission_grad_style;
static bool user_permission_style_inited = false;
static void user_permission_destroy(void);
void user_permission_back_btn_click_cb(lv_event_t *e);
// 全局样式初始化
static void init_user_permission_styles(void)
{
    if(!user_permission_style_inited) {
        lv_style_init(&user_permission_grad_style);
        user_permission_style_inited = true;
    }
}

static void user_permission_scr_load_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *scr = lv_event_get_target(e);
    if(!is_lv_obj_valid(scr)) return;
    
    // 屏幕加载时，把状态栏挂载到当前屏幕
    update_status_bar_parent(scr);
    
}

void ui_user_permission_create(lv_obj_t *user_manage_scr)
{
    user_permission_destroy();
    init_user_permission_styles();
    // 1. 安全校验：如果传进来的 user_manage_scr 为空，直接返回
    if(user_manage_scr == NULL) {
        LV_LOG_WARN("ui_user_permission_create: user_manage_scr is NULL!");
        return;
    }

    // 2. 创建/复用设置屏幕对象
    if(user_permission_scr == NULL) {
        user_permission_scr = lv_obj_create(NULL);  // 创建独立屏幕
        lv_obj_add_event_cb(user_permission_scr, user_permission_scr_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    } else {
        lv_obj_clean(user_permission_scr);          // 清空原有内容
    }
    
    
    lv_style_reset(&user_permission_grad_style);
    lv_style_set_bg_color(&user_permission_grad_style, lv_color_hex(0x010715));// 渐变主色：#010715（0%）
    lv_style_set_bg_grad_color(&user_permission_grad_style, lv_color_hex(0x0E1D37));// 渐变副色：#0E1D37（100%）
    lv_style_set_bg_grad_dir(&user_permission_grad_style, LV_GRAD_DIR_VER);// 渐变方向：垂直
    lv_style_set_bg_main_stop(&user_permission_grad_style, 0);// 渐变范围：0~255
    lv_style_set_bg_grad_stop(&user_permission_grad_style, 255);
    lv_obj_add_style(user_permission_scr, &user_permission_grad_style, LV_STATE_DEFAULT);// 应用渐变样式到屏幕

    // 左上角返回按钮
    lv_obj_t *back_btn = create_text_label
    (user_permission_scr, ICON_CHEVORN_LEFT, &my_custom_icon, lv_color_hex(0xFFFFFF), 52, 90, LV_OPA_100);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn, user_permission_back_btn_click_cb, LV_EVENT_CLICKED, user_manage_scr);

    // 切换到设置屏幕
    lv_scr_load(user_permission_scr);
}





/***********************用户权限界面回调*********************/
void user_permission_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    lv_obj_t *user_manage_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(user_manage_scr == NULL) {
        LV_LOG_WARN("family_menber_btn_click_cb: user_manage_scr is NULL!");
        return;
    }
    ui_user_permission_create(user_manage_scr);

}
void user_permission_back_btn_click_cb(lv_event_t *e)
{
    lv_obj_t *parent_scr = lv_event_get_user_data(e);
    if (parent_scr == NULL || e == NULL) return;
    // 2. 返回上一级
    lv_scr_load(parent_scr);
    // 1. 销毁当前页面
    user_permission_destroy();

}

static void user_permission_destroy(void)
{
    if (lv_obj_is_valid(user_permission_scr)) {
        lv_obj_del(user_permission_scr);
        user_permission_scr = NULL;
    }
    if (user_permission_style_inited) {
        lv_style_reset(&user_permission_grad_style);
        user_permission_style_inited = false;
    }
}