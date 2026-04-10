#include "lv_edit_record.h"
#include "lv_a_enroll_opt.h"
#include <string.h>
#include <stdio.h>

lv_obj_t *edit_record_scr = NULL; 
static lv_style_t edit_record_grad_style;
static bool edit_record_style_inited = false;

static lv_obj_t *g_keyboard = NULL;
static lv_obj_t *g_mask = NULL;
static lv_obj_t *g_dialog = NULL;
static lv_obj_t *g_name_ta = NULL;

// 【解耦】保存通用参数
static edit_type_e g_edit_type;
static const char *g_edit_name = NULL;
static lv_obj_t *g_parent_scr = NULL;
static int g_edit_index = -1; // 编辑条目索引

static void init_msg_center_styles(void);
static void hide_keyboard(lv_event_t *e);
static void name_con_click_cb(lv_event_t *e);
static void del_btn_click_cb(lv_event_t *e);
static void dialog_confirm_cb(lv_event_t *e);
static void dialog_cancel_cb(lv_event_t *e);
static void close_dialog(void);
static void save_btn_click_cb(lv_event_t *e);

static void init_msg_center_styles(void)
{
    if(!edit_record_style_inited) {
        lv_style_init(&edit_record_grad_style);
        edit_record_style_inited = true;
    }
}

static void hide_keyboard(lv_event_t *e)
{
    (void)e;
    LV_LOG_INFO("delete keyboard");
    
    // 🔥 关键：键盘有效则直接删除，而不是隐藏
    if(g_keyboard && lv_obj_is_valid(g_keyboard)) {
        lv_obj_del(g_keyboard);   // 删除键盘对象
        g_keyboard = NULL;        // 指针置空，防止野指针
    }
}

static void close_dialog(void)
{
    LV_LOG_INFO("close delete dialog");
    if(g_mask && lv_obj_is_valid(g_mask)) lv_obj_del(g_mask);
    if(g_dialog && lv_obj_is_valid(g_dialog)) lv_obj_del(g_dialog);
    g_mask = NULL;
    g_dialog = NULL;
}

static void name_con_click_cb(lv_event_t *e)
{
    (void)e;
    LV_LOG_INFO("name container clicked, open keyboard");
    if(!g_keyboard || !lv_obj_is_valid(g_keyboard)) {
        g_keyboard = lv_keyboard_create(lv_scr_act());
        lv_obj_set_style_bg_color(g_keyboard, lv_color_hex(0x192A46), LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(g_keyboard, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
        lv_obj_set_size(g_keyboard, LV_HOR_RES, 250);
        lv_obj_align(g_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_add_event_cb(g_keyboard, hide_keyboard, LV_EVENT_READY, NULL);
    }
    lv_obj_clear_flag(g_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(g_keyboard, g_name_ta);
    lv_obj_move_foreground(g_keyboard);
}

static void del_btn_click_cb(lv_event_t *e)
{
    lv_obj_t *enroll_scr = lv_event_get_user_data(e);
    LV_LOG_INFO("delete btn clicked, show dialog");
    close_dialog();

    g_mask = lv_obj_create(edit_record_scr);
    lv_obj_set_size(g_mask, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(g_mask, lv_color_hex(0x000000), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(g_mask, LV_OPA_70, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(g_mask, 0, LV_STATE_DEFAULT);

    g_dialog = create_container(edit_record_scr, 212, 150, 600, 297, lv_color_hex(0xE0EDFF), LV_OPA_100, 16, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(g_dialog, 0, LV_STATE_DEFAULT);
    create_text_label(g_dialog, "Confirm delete?", &lv_font_montserrat_28,lv_color_hex(0x000000), 0, 40, LV_OPA_100);
    lv_obj_align_to(lv_obj_get_child(g_dialog, 0), g_dialog, LV_ALIGN_TOP_MID, 0, 40);

    lv_obj_t *cancel = create_container(g_dialog, 0, 0, 220, 44, lv_color_hex(0x192A46), LV_OPA_100, 6, lv_color_hex(0x1F3150), 0, LV_OPA_100);
    lv_obj_align(cancel, LV_ALIGN_TOP_MID, 0, 210);
    create_text_label(cancel, "cancel", &lv_font_montserrat_28,lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align(lv_obj_get_child(cancel, 0), LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(cancel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(cancel, dialog_cancel_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *confirm = create_container(g_dialog, 0, 0, 220, 44, lv_color_hex(0x192A46), LV_OPA_100, 6, lv_color_hex(0x1F3150), 0, LV_OPA_100);
    lv_obj_align(confirm, LV_ALIGN_TOP_MID, 0, 140);
    create_text_label(confirm, "confirm", &lv_font_montserrat_28, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align(lv_obj_get_child(confirm, 0), LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(confirm, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(confirm, dialog_confirm_cb, LV_EVENT_CLICKED, enroll_scr);
}

static void dialog_confirm_cb(lv_event_t *e)
{
    lv_obj_t *enroll_scr = lv_event_get_user_data(e);
    LV_LOG_INFO("dialog confirm, delete item");
    
    // 执行通用删除（原有代码，不动）
    edit_delete_item(g_edit_type, g_edit_index);
    
    close_dialog();

    // 最后切换到录入界面（原有代码，不动）
    lv_scr_load(enroll_scr);
    update_status_bar_parent(enroll_scr);
    lv_obj_t *current_del_scr = lv_disp_get_scr_act(NULL);
    if(current_del_scr == edit_record_scr && is_lv_obj_valid(current_del_scr)) {
        lv_obj_del(edit_record_scr);  // 销毁编辑界面
        edit_record_scr = NULL;       // 指针置空
    }

}

static void dialog_cancel_cb(lv_event_t *e)
{
    (void)e;
    LV_LOG_INFO("dialog cancel, close dialog");
    close_dialog();
}

// 通用创建界面，只根据枚举渲染UI，无业务耦合
void ui_edit_record_create(edit_type_e type, const char *cur_name, uint8_t index, lv_obj_t *parent_scr)
{
    // 保存通用参数
    g_edit_type = type;
    g_edit_name = cur_name;
    g_edit_index = index;
    g_parent_scr = parent_scr;

    LV_LOG_INFO("create edit record screen");
    init_msg_center_styles();

    if(is_lv_obj_valid(edit_record_scr)) {
        lv_obj_del(edit_record_scr);
        edit_record_scr = NULL;
    }
    edit_record_scr = lv_obj_create(NULL);  
    lv_style_reset(&edit_record_grad_style);
    lv_style_set_bg_color(&edit_record_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&edit_record_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&edit_record_grad_style, LV_GRAD_DIR_VER);
    lv_style_set_bg_main_stop(&edit_record_grad_style, 0);
    lv_style_set_bg_grad_stop(&edit_record_grad_style, 255);
    lv_obj_add_style(edit_record_scr, &edit_record_grad_style, LV_STATE_DEFAULT);
    lv_obj_add_event_cb(edit_record_scr, hide_keyboard, LV_EVENT_CLICKED, NULL);
    // ===================== 通用标题：根据枚举自动匹配 =====================
    char title_buf[16] = {0};
    char name_label_buf[16] = {0};
    switch(g_edit_type) {
        case EDIT_TYPE_PWD: 
            strcpy(title_buf, "Pwd Edit"); 
            strcpy(name_label_buf, "Pwd Name"); 
            break;
        case EDIT_TYPE_CARD: 
            strcpy(title_buf, "Card Edit"); 
            strcpy(name_label_buf, "Card Name"); 
            break;
        case EDIT_TYPE_FACE: 
            strcpy(title_buf, "Face Edit"); 
            strcpy(name_label_buf, "Face Name"); 
            break;
        case EDIT_TYPE_FINGER: 
            strcpy(title_buf, "Finger Edit"); 
            strcpy(name_label_buf, "Finger Name"); 
            break;
        // 后续拓展直接加case，不用动其他代码
        default: 
            strcpy(title_buf, "Edit"); 
            strcpy(name_label_buf, "Name"); 
            break;
    }
    create_text_label(edit_record_scr, title_buf, &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 83, 80, LV_OPA_100);

    lv_obj_t *save_btn = create_text_label(edit_record_scr, "save", &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 928, 90, LV_OPA_100);
    lv_obj_add_flag(save_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(save_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(save_btn, save_btn_click_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *edit_record_con = create_container(edit_record_scr,
    49,150,927,83,lv_color_hex(0xFFFFFF), LV_OPA_100, 6,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(edit_record_con, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(edit_record_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(edit_record_con, LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(edit_record_con, name_con_click_cb, LV_EVENT_CLICKED, NULL);

    // 通用名称标签
    create_text_label(edit_record_con, name_label_buf, &lv_font_montserrat_36, lv_color_hex(0x000000), 24, 19, LV_OPA_100);
    create_text_label(edit_record_scr, "Name can be modified", &lv_font_montserrat_24, lv_color_hex(0x000000), 73, 233, LV_OPA_60);

    // 输入框
    g_name_ta = lv_textarea_create(edit_record_con);
    lv_obj_set_style_pad_all(g_name_ta, 0, LV_STATE_DEFAULT);
    lv_obj_set_size(g_name_ta, 150, 40);
    lv_obj_set_pos(g_name_ta, 790, 27);
    lv_obj_set_style_bg_opa(g_name_ta, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(g_name_ta, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(g_name_ta, lv_color_hex(0x000000), LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(g_name_ta, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    lv_textarea_set_max_length(g_name_ta, 8);
    lv_textarea_set_one_line(g_name_ta, true);
    lv_obj_add_event_cb(g_name_ta, name_con_click_cb, LV_EVENT_CLICKED, NULL);
    // ===================== 自动填充传入的名称 =====================
    if(g_edit_name != NULL) {
        lv_textarea_set_text(g_name_ta, g_edit_name);
    }

    // 删除按钮 + 返回按钮（原样）
    lv_obj_t *delete_btn = create_container(edit_record_scr,
    408,502,208,50,lv_color_hex(0x192A46), LV_OPA_100, 6,lv_color_hex(0xE0EDFF), 0, LV_OPA_50);
    lv_obj_set_style_pad_all(delete_btn, 0, LV_STATE_DEFAULT); 
    lv_obj_t *delete_btn_label = create_text_label(delete_btn, "delete", &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align(delete_btn_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(delete_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(delete_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(delete_btn, del_btn_click_cb, LV_EVENT_CLICKED, parent_scr);

    lv_obj_t *back_btn = create_container_circle(edit_record_scr, 52, 90, 30,
    true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(back_btn,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn,LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn,back_btn_click_cb,LV_EVENT_CLICKED,parent_scr);

    update_status_bar_parent(edit_record_scr);
    lv_scr_load(edit_record_scr);
}
// 保存按钮回调
static void save_btn_click_cb(lv_event_t *e)
{
    (void)e;
    const char *input_text = lv_textarea_get_text(g_name_ta);

    if(input_text == NULL || strlen(input_text) == 0) {
        LV_LOG_WARN("Save failed: Name cannot be empty");
        return;
    }
    // 保存名称
    edit_update_name(g_edit_type, input_text, g_edit_index);
    // 隐藏键盘
    hide_keyboard(NULL);

    // ========================
    // 🔥 完全复刻 back_btn_click_cb 逻辑（不调用、不修改原函数）
    // ========================
    lv_obj_t *target_scr = g_parent_scr;  // 目标屏幕=父界面
    if(target_scr == NULL) {
        LV_LOG_WARN("Save back: target screen is NULL");
        return;
    }
    // 获取当前要删除的屏幕
    lv_obj_t *current_del_scr = lv_disp_get_scr_act(NULL);
    if(!is_lv_obj_valid(current_del_scr)) return;

    // 执行返回逻辑
    update_status_bar_parent(target_scr);
    lv_scr_load(target_scr);

    // 自动删除编辑界面（和返回按钮完全一样的逻辑）
    if(current_del_scr == edit_record_scr) {
        lv_obj_del(edit_record_scr);
        edit_record_scr = NULL;
    }
}

void edit_record_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    lv_obj_t *enroll_scr = (lv_obj_t *)lv_event_get_user_data(e);
    // 校验父界面有效性
    if(!is_lv_obj_valid(enroll_scr)) {
        LV_LOG_WARN("父界面无效，拦截点击");
        return;
    }

    lv_obj_t *target = lv_event_get_target(e);
    // 🔥 修复：校验点击对象是否有效（核心拦截野指针）
    if(!is_lv_obj_valid(target)) {
        LV_LOG_WARN("点击对象无效，拦截卡死");
        return;
    }

    // 基础信息（所有成员都有）
    pwd_enroll_info_t *pwd_info = get_current_pwd_info();
    finger_enroll_info_t *finger_info = get_current_finger_info();

    // 1. 指纹（所有成员都有）
    if(finger_info) {
        for(int i=0; i<MAX_FINGER_COUNT; i++){
            // 🔥 修复：先校验指针非空+有效，再对比
            if(finger_info->finger_record_cons[i] == target && is_lv_obj_valid(finger_info->finger_record_cons[i])){
                ui_edit_record_create(EDIT_TYPE_FINGER, finger_info->finger_names[i], i, enroll_scr);
                return;
            }
        }
    }

    // 2. 密码（所有成员都有）
    if(pwd_info) {
        for(int i=0; i<MAX_PWD_COUNT; i++){
            if(pwd_info->pwd_record_cons[i] == target && is_lv_obj_valid(pwd_info->pwd_record_cons[i])){
                ui_edit_record_create(EDIT_TYPE_PWD, pwd_info->pwd_names[i], i, enroll_scr);
                return;
            }
        }
    }

    // 仅家庭成员处理卡片+人脸
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY) {
        card_enroll_info_t *card_info = get_current_card_info();
        face_enroll_info_t *face_info = get_current_face_info();

        // 3. 卡片
        if(card_info) {
            for(int i=0; i<MAX_CARD_COUNT; i++){
                if(card_info->card_record_cons[i] == target && is_lv_obj_valid(card_info->card_record_cons[i])){
                    ui_edit_record_create(EDIT_TYPE_CARD, card_info->card_names[i], i, enroll_scr);
                    return;
                }
            }
        }
        // 4. 人脸
        if(face_info) {
            for(int i=0; i<MAX_FACE_COUNT; i++){
                if(face_info->face_record_cons[i] == target && is_lv_obj_valid(face_info->face_record_cons[i])){
                    ui_edit_record_create(EDIT_TYPE_FACE, face_info->face_names[i], i, enroll_scr);
                    return;
                }
            }
        }
    }

    LV_LOG_INFO("edit record btn click");
}