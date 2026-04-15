#if 0
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
#else
#include "lv_user_manage.h"
#include "lv_family_menber.h"
#include "lv_other_member.h"
#include "lv_user_permission.h"

// ====================== 全局屏幕对象 ======================
lv_obj_t *user_manage_scr = NULL; 

// ====================== 样式相关 ======================
static lv_style_t user_manage_grad_style;
static bool user_manage_style_inited = false;

// ====================== 函数声明 ======================
void user_manage_back_btn_click_cb(lv_event_t *e);

// ====================== 全局样式初始化 ======================
static void init_user_manage_styles(void)
{
    if(!user_manage_style_inited) {
        lv_style_init(&user_manage_grad_style);
        user_manage_style_inited = true;
    }
}

// ====================== 屏幕加载回调 ======================
static void user_manage_scr_load_cb(lv_event_t *e)
{
    if(e == NULL) return;

    lv_obj_t *scr = lv_event_get_target(e);
    if(!is_lv_obj_valid(scr)) return;
    
    // 屏幕加载时，将状态栏挂载到当前屏幕
    update_status_bar_parent(scr);
}

// ====================== 用户管理界面 - 创建主函数 ======================
void ui_user_manage_create(lv_obj_t *homepage_scr)
{
    init_user_manage_styles();

    // 安全校验：主页屏幕为空则直接返回
    if(homepage_scr == NULL) {
        LV_LOG_WARN("ui_user_manage_create: homepage_scr is NULL!");
        return;
    }

    // 销毁旧界面，重建新界面（避免残留控件）
    if(is_lv_obj_valid(user_manage_scr)) {
        lv_obj_del(user_manage_scr);
        user_manage_scr = NULL;
    }
    user_manage_scr = lv_obj_create(NULL);

    // ====================== 背景渐变样式 ======================
    lv_style_reset(&user_manage_grad_style);
    lv_style_set_bg_color(&user_manage_grad_style, lv_color_hex(0x010715));        // 渐变起始色
    lv_style_set_bg_grad_color(&user_manage_grad_style, lv_color_hex(0x0E1D37));   // 渐变结束色
    lv_style_set_bg_grad_dir(&user_manage_grad_style, LV_GRAD_DIR_VER);            // 垂直渐变
    lv_style_set_bg_main_stop(&user_manage_grad_style, 0);                        // 渐变起点
    lv_style_set_bg_grad_stop(&user_manage_grad_style, 255);                      // 渐变终点
    lv_obj_add_style(user_manage_scr, &user_manage_grad_style, LV_STATE_DEFAULT);

    // 标题：user manager
    create_text_label(
        user_manage_scr, "user manager", &lv_font_montserrat_36,
        lv_color_hex(0xFFFFFF), 83, 80, LV_OPA_100
    );

    /************************** 家庭成员模块 **************************/
    lv_obj_t *user_manage_family_con = create_container(
        user_manage_scr, 49, 150, 927, 83,
        lv_color_hex(0x192A46), LV_OPA_100, 6,
        lv_color_hex(0x1F3150), 0, LV_OPA_90
    );
    create_image_obj(user_manage_scr, "H:family_member.png", 73, 176);
    create_image_obj(user_manage_scr, "H:right.png", 932, 176);
    create_text_label(
        user_manage_scr, "family menber", &lv_font_montserrat_36,
        lv_color_hex(0xFFFFFF), 130, 169, LV_OPA_100
    );

    lv_obj_add_flag(user_manage_family_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(user_manage_family_con, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(user_manage_family_con, family_menber_btn_click_cb, LV_EVENT_CLICKED, user_manage_scr);

    /************************** 其他成员模块 **************************/
    lv_obj_t *user_manage_other_con = create_container(
        user_manage_scr, 49, 241, 927, 83,
        lv_color_hex(0x192A46), LV_OPA_100, 6,
        lv_color_hex(0x1F3150), 0, LV_OPA_90
    );
    create_image_obj(user_manage_scr, "H:other_member.png", 74, 263);
    create_image_obj(user_manage_scr, "H:right.png", 932, 263);
    create_text_label(
        user_manage_scr, "other menber", &lv_font_montserrat_36,
        lv_color_hex(0xFFFFFF), 130, 261, LV_OPA_100
    );

    lv_obj_add_flag(user_manage_other_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(user_manage_other_con, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(user_manage_other_con, other_member_btn_click_cb, LV_EVENT_CLICKED, user_manage_scr);

    /************************** 用户权限模块 **************************/
    lv_obj_t *user_manage_permission_con = create_container(
        user_manage_scr, 49, 332, 927, 83,
        lv_color_hex(0x192A46), LV_OPA_100, 6,
        lv_color_hex(0x1F3150), 0, LV_OPA_90
    );
    create_image_obj(user_manage_scr, "H:user_permission.png", 73, 353);
    create_image_obj(user_manage_scr, "H:right.png", 932, 353);
    create_text_label(
        user_manage_scr, "user permission", &lv_font_montserrat_36,
        lv_color_hex(0xFFFFFF), 130, 350, LV_OPA_100
    );

    lv_obj_add_flag(user_manage_permission_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(user_manage_permission_con, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(user_manage_permission_con, user_permission_btn_click_cb, LV_EVENT_CLICKED, user_manage_scr);

    // ====================== 左上角返回按钮 ======================
    lv_obj_t *back_btn = create_container_circle(user_manage_scr, 52, 90, 30, true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn, user_manage_back_btn_click_cb, LV_EVENT_CLICKED, homepage_scr);
    
    // 切换显示用户管理界面
    lv_scr_load(user_manage_scr);
}

// ====================== 外部函数声明 ======================
extern void destroy_homepage(void);
extern void lv_homepage(void);

// ====================== 入口按钮：打开用户管理 ======================
void user_manage_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;

    lv_obj_t *homepage_scr_temp = (lv_obj_t *)lv_event_get_user_data(e);
    if(homepage_scr_temp == NULL) {
        LV_LOG_WARN("user_manage_btn_click_cb: homepage_scr is NULL!");
        return;
    }

    // 创建用户管理界面
    ui_user_manage_create(homepage_scr_temp);
    // 切换显示
    lv_scr_load(user_manage_scr);
    // 更新状态栏
    update_status_bar_parent(user_manage_scr);
    // 销毁主页
    destroy_homepage();

    LV_LOG_WARN("user_manage_btn_click_cb: Destroy the homepage and create the management interface");
}

// ====================== 返回按钮：退回主页 ======================
void user_manage_back_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;

    lv_obj_t *current_del_scr = lv_disp_get_scr_act(NULL);

    if(!lv_obj_is_valid(current_del_scr)) return;

    // 当前显示的是用户管理界面 → 重建主页并销毁当前界面
    if(current_del_scr == user_manage_scr) {
        lv_homepage();                      // 重建主页
        lv_obj_del(current_del_scr);        // 销毁用户管理界面
        user_manage_scr = NULL;            // 指针置空
        return;
    }
}

// ====================== 销毁用户管理界面（供外部调用） ======================
void destroy_user_manage(void)
{
    if(is_lv_obj_valid(user_manage_scr)) {
        lv_obj_del(user_manage_scr);
        user_manage_scr = NULL;
    }
}
#endif