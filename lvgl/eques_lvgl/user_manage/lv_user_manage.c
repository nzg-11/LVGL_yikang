#include "lv_user_manage.h"
#include "lv_family_menber.h"
#include "lv_other_member.h"
#include "lv_user_permission.h"

static lv_obj_t *user_manage_scr = NULL; 

static lv_style_t user_manage_grad_style;
static bool user_manage_style_inited = false;

// 全局样式初始化
static void init_user_manage_styles(void)
{
    if(!user_manage_style_inited) {
        lv_style_init(&user_manage_grad_style);
        user_manage_style_inited = true;
    }
}
static void user_manage_scr_load_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *scr = lv_event_get_target(e);
    if(!is_lv_obj_valid(scr)) return;
    
    // 屏幕加载时，把状态栏挂载到当前屏幕
    update_status_bar_parent(scr);
    
}
// 用户管理界面创建
void ui_user_manage_create(lv_obj_t *homepage_scr)
{
    init_user_manage_styles();
    // 1. 安全校验：如果传进来的 homepage_scr 为空，直接返回
    if(homepage_scr == NULL) {
        LV_LOG_WARN("ui_user_manage_create: homepage_scr is NULL!");
        return;
    }

    // 2. 创建/复用设置屏幕对象
    if(user_manage_scr == NULL) {
        user_manage_scr = lv_obj_create(NULL);  // 创建独立屏幕
        lv_obj_add_event_cb(user_manage_scr, user_manage_scr_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    } else {
        lv_obj_clean(user_manage_scr);          // 清空原有内容
    }
    
    lv_style_reset(&user_manage_grad_style);
    lv_style_set_bg_color(&user_manage_grad_style, lv_color_hex(0x010715));// 渐变主色：#010715（0%）
    lv_style_set_bg_grad_color(&user_manage_grad_style, lv_color_hex(0x0E1D37));// 渐变副色：#0E1D37（100%）
    lv_style_set_bg_grad_dir(&user_manage_grad_style, LV_GRAD_DIR_VER);// 渐变方向：垂直
    lv_style_set_bg_main_stop(&user_manage_grad_style, 0);// 渐变范围：0~255
    lv_style_set_bg_grad_stop(&user_manage_grad_style, 255);
    lv_obj_add_style(user_manage_scr, &user_manage_grad_style, LV_STATE_DEFAULT);// 应用渐变样式到屏幕

    /**************************家庭成员***************************/
    lv_obj_t *user_manage_family_con = create_container(user_manage_scr,
    47,195,710,83,
    lv_color_hex(0x192A46), LV_OPA_100, 6,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_t *family_member_img = create_image_obj(user_manage_scr, "H:family_member.png", 71, 221);
    lv_obj_t *right_img1 = create_image_obj(user_manage_scr, "H:right.png", 712, 221);
    lv_obj_t *family_member_label = create_text_label
    (user_manage_scr, "family menber", &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 132, 214, LV_OPA_100);
    lv_obj_add_flag(user_manage_family_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(user_manage_family_con, LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(user_manage_family_con, family_menber_btn_click_cb, LV_EVENT_CLICKED, user_manage_scr);
    /**************************其他成员***************************/
    lv_obj_t *user_manage_other_con = create_container(user_manage_scr,
    47,282,710,83,
    lv_color_hex(0x192A46), LV_OPA_100, 6,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_t *other_member_img = create_image_obj(user_manage_scr, "H:other_member.png", 71, 308);
    lv_obj_t *right_img2 = create_image_obj(user_manage_scr, "H:right.png", 712, 308);
    lv_obj_t *other_member_label = create_text_label
    (user_manage_scr, "other menber", &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 132, 301, LV_OPA_100);
    lv_obj_add_flag(user_manage_other_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(user_manage_other_con, LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(user_manage_other_con, other_member_btn_click_cb, LV_EVENT_CLICKED, user_manage_scr);
    /**************************用户权限***************************/
    lv_obj_t *user_manage_permission_con = create_container(user_manage_scr,
    47,369,710,83,
    lv_color_hex(0x192A46), LV_OPA_100, 6,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_t *permission_img = create_image_obj(user_manage_scr, "H:user_permission.png", 71, 391);
    lv_obj_t *right_img3 = create_image_obj(user_manage_scr, "H:right.png", 712, 391);
    lv_obj_t *permission_label = create_text_label
    (user_manage_scr, "user permission", &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 132, 388, LV_OPA_100);
    lv_obj_add_flag(user_manage_permission_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(user_manage_permission_con, LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(user_manage_permission_con, user_permission_btn_click_cb, LV_EVENT_CLICKED, user_manage_scr);


       

    // 左上角返回按钮
    lv_obj_t *back_btn = create_image_obj(user_manage_scr, "H:back.png", 52, 123);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn, back_btn_click_cb, LV_EVENT_CLICKED, homepage_scr);
    
    // 切换到设置屏幕
    lv_scr_load(user_manage_scr);
}





/***********************用户管理界面回调*********************/
void user_manage_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    lv_obj_t *homepage_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(homepage_scr == NULL) {
        LV_LOG_WARN("user_manage_btn_click_cb: homepage_scr is NULL!");
        return;
    }
    ui_user_manage_create(homepage_scr);
}
