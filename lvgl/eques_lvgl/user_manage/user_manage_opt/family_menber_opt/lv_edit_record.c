/**
 * @file lv_edit_record.c
 * @brief 生物特征记录编辑界面（指纹/密码/卡片/人脸名称修改、删除）
 * @note 依赖 LVGL 库、录入管理模块
 */

/*********************
 * 头文件包含
 *********************/
#include "lv_edit_record.h"
#include "lv_a_enroll_opt.h"
#include <string.h>
#include <stdio.h>

/*********************
 * 全局变量定义
 *********************/
// 界面根对象
lv_obj_t *edit_record_scr = NULL; 

// 样式相关
static lv_style_t edit_record_grad_style;
 

// 键盘与弹窗组件
static lv_obj_t *g_keyboard = NULL;
static lv_obj_t *g_mask = NULL;
static lv_obj_t *g_dialog = NULL;
static lv_obj_t *g_name_ta = NULL;

// 编辑通用参数（解耦存储）
static edit_type_e  g_edit_type;
static const char  *g_edit_name = NULL;
static lv_obj_t    *g_parent_scr = NULL;
static int          g_edit_index = -1;

/*********************
 * 静态函数前置声明
 * 【UI相关函数优先声明】
 *********************/
// 样式初始化
static void init_msg_center_styles(void);

// UI核心创建
void ui_edit_record_create(edit_type_e type, const char *cur_name, uint8_t index, lv_obj_t *parent_scr);

// UI按钮回调
static void save_btn_click_cb(lv_event_t *e);
void edit_record_back_btn_click_cb(lv_event_t *e);
void edit_record_btn_click_cb(lv_event_t *e);
static void name_con_click_cb(lv_event_t *e);

// 删除弹窗相关
static void del_btn_click_cb(lv_event_t *e);
static void dialog_confirm_cb(lv_event_t *e);
static void dialog_cancel_cb(lv_event_t *e);
static void close_dialog(void);

// 键盘相关
static void hide_keyboard(lv_event_t *e);

/*********************
 * 函数实现
 * 【UI相关函数优先实现】
 *********************/

/**
 * @brief 创建生物特征编辑界面（UI核心函数，前置）
 * @param type: 编辑类型(指纹/密码/卡片/人脸)
 * @param cur_name: 当前名称
 * @param index: 条目索引
 * @param parent_scr: 父界面
 */
void ui_edit_record_create(edit_type_e type, const char *cur_name, uint8_t index, lv_obj_t *parent_scr)
{
    // 保存通用编辑参数
    g_edit_type = type;
    g_edit_name = cur_name;
    g_edit_index = index;
    g_parent_scr = parent_scr;

    LV_LOG_INFO("create edit record screen");
    lv_style_init(&edit_record_grad_style);

    // 创建界面根容器
    edit_record_scr = lv_obj_create(NULL);  
    lv_style_reset(&edit_record_grad_style);
    lv_style_set_bg_color(&edit_record_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&edit_record_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&edit_record_grad_style, LV_GRAD_DIR_VER);
    lv_style_set_bg_main_stop(&edit_record_grad_style, 0);
    lv_style_set_bg_grad_stop(&edit_record_grad_style, 255);
    lv_obj_add_style(edit_record_scr, &edit_record_grad_style, LV_STATE_DEFAULT);
    lv_obj_add_event_cb(edit_record_scr, hide_keyboard, LV_EVENT_CLICKED, NULL);

    // 名称编辑容器
    lv_obj_t *edit_record_con = create_container(edit_record_scr,
    49,150,927,83,lv_color_hex(0xFFFFFF), LV_OPA_100, 6,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(edit_record_con, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(edit_record_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(edit_record_con, LV_OPA_80,LV_STATE_PRESSED);
    //lv_obj_add_event_cb(edit_record_con, name_con_click_cb, LV_EVENT_CLICKED, NULL);
//  ===================== 动态标题 =====================
switch(g_edit_type) {
    case EDIT_TYPE_PWD:
        create_text_label(edit_record_scr, "密码修改", &eques_bold_36, lv_color_hex(0xFFFFFF), 83, 80, LV_OPA_100);
        create_text_label(edit_record_con, "密码名称", &eques_regular_36, lv_color_hex(0x000000), 24, 19, LV_OPA_100);
        break;

    case EDIT_TYPE_CARD:
        create_text_label(edit_record_scr, "卡片修改", &eques_bold_36, lv_color_hex(0xFFFFFF), 83, 80, LV_OPA_100);
        create_text_label(edit_record_con, "卡片名称", &eques_regular_36, lv_color_hex(0x000000), 24, 19, LV_OPA_100);
        break;

    case EDIT_TYPE_FACE:
        create_text_label(edit_record_scr, "人脸修改", &eques_bold_36, lv_color_hex(0xFFFFFF), 83, 80, LV_OPA_100);
        create_text_label(edit_record_con, "面容名称", &eques_regular_36, lv_color_hex(0x000000), 24, 19, LV_OPA_100);
        break;

    case EDIT_TYPE_FINGER:
        create_text_label(edit_record_scr, "指纹修改", &eques_bold_36, lv_color_hex(0xFFFFFF), 83, 80, LV_OPA_100);
        create_text_label(edit_record_con, "指纹名称", &eques_regular_36, lv_color_hex(0x000000), 24, 19, LV_OPA_100);
        break;
}

    // 保存按钮
    lv_obj_t *save_btn = create_text_label(edit_record_scr, "保存", &eques_regular_24, lv_color_hex(0xFFFFFF), 928, 90, LV_OPA_100);
    lv_obj_add_flag(save_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(save_btn, LV_OPA_80, LV_STATE_PRESSED);
    //lv_obj_add_event_cb(save_btn, save_btn_click_cb, LV_EVENT_CLICKED, parent_scr);





    // 名称输入框
    // ===================== 替换为：纯文本标签（无textarea、不卡死、支持中文） =====================
    g_name_ta = create_text_label(edit_record_con, "", &eques_regular_24, 
                                lv_color_hex(0x000000), 0, 0, LV_OPA_100);
    lv_obj_set_size(g_name_ta, 200, 40);    // 加宽一点，防止文字被截断
    lv_obj_set_pos(g_name_ta, 650, 27);     // 位置保持不变
    lv_obj_set_style_text_align(g_name_ta, LV_TEXT_ALIGN_RIGHT, 0); // 右对齐更美观
    lv_label_set_long_mode(g_name_ta, LV_LABEL_LONG_SCROLL); // 超长自动滚动

    //lv_obj_add_event_cb(g_name_ta, name_con_click_cb, LV_EVENT_CLICKED, NULL);
    
    // 自动填充原有名称
    // 自动填充原有名称（空值根据类型自动生成：Face / Card / Finger / Pwd）
    if(g_edit_name != NULL) {
        if(strlen(g_edit_name) == 0) {
            char tmp_buf[16] = {0};
            switch(g_edit_type) {
                case EDIT_TYPE_FACE:
                    snprintf(tmp_buf, sizeof(tmp_buf), "面容%d", g_edit_index + 1);
                    break;
                case EDIT_TYPE_CARD:
                    snprintf(tmp_buf, sizeof(tmp_buf), "卡片%d", g_edit_index + 1);
                    break;
                case EDIT_TYPE_FINGER:
                    snprintf(tmp_buf, sizeof(tmp_buf), "指纹%d", g_edit_index + 1);
                    break;
                case EDIT_TYPE_PWD:
                    snprintf(tmp_buf, sizeof(tmp_buf), "密码%d", g_edit_index + 1);
                    break;
                default:
                    break;
            }
            lv_label_set_text(g_name_ta, tmp_buf);
        } else {
            lv_label_set_text(g_name_ta, g_edit_name);
        }
    }

    // 删除按钮
    lv_obj_t *delete_btn = create_container(edit_record_scr,
    408,502,208,50,lv_color_hex(0x192A46), LV_OPA_100, 6,lv_color_hex(0xE0EDFF), 0, LV_OPA_50);
    lv_obj_set_style_pad_all(delete_btn, 0, LV_STATE_DEFAULT); 
    lv_obj_t *delete_btn_label = create_text_label(delete_btn, "删除", &eques_regular_24, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align(delete_btn_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(delete_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(delete_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(delete_btn, del_btn_click_cb, LV_EVENT_CLICKED, parent_scr);

    // 返回按钮
    lv_obj_t *back_btn = create_text_label
    (edit_record_scr, ICON_CHEVORN_LEFT, &fontawesome_icon_32, lv_color_hex(0xFFFFFF), 52, 84, LV_OPA_100);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(back_btn,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn,LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn,edit_record_back_btn_click_cb,LV_EVENT_CLICKED,parent_scr);

    // 加载界面
    update_status_bar_parent(edit_record_scr);
    lv_scr_load(edit_record_scr);
}

/**
 * @brief 保存按钮回调
 */
static void save_btn_click_cb(lv_event_t *e)
{
    (void)e;
    const char *input_text = lv_label_get_text(g_name_ta);

    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);
    // 非空校验
    if(input_text == NULL || strlen(input_text) == 0) {
        LV_LOG_WARN("Save failed: Name cannot be empty");
        return;
    }

    // 更新名称
    edit_update_name(g_edit_type, input_text, g_edit_index);
    hide_keyboard(NULL);

    // 返回录入界面并重建
    // common_member_info_t *member = get_current_enroll_member();
    // ui_enroll_create(member, g_parent_scr);  
    lv_scr_load(parent_scr);

    // 销毁编辑界面
    if(lv_obj_is_valid(edit_record_scr)) {
        lv_obj_del(edit_record_scr);
        edit_record_scr = NULL;
    }
}

/**
 * @brief 返回按钮回调
 */
void edit_record_back_btn_click_cb(lv_event_t *e)
{
    (void)e;
    LV_LOG_INFO("back to enroll screen");
    
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);
    hide_keyboard(NULL);

    // 返回录入界面并重建
    // common_member_info_t *member = get_current_enroll_member();
    // ui_enroll_create(member, g_parent_scr);  
    lv_scr_load(parent_scr);
    // 销毁编辑界面
    if(lv_obj_is_valid(edit_record_scr)) {
        lv_obj_del(edit_record_scr);
        edit_record_scr = NULL;
    }
}

/**
 * @brief 编辑入口按钮回调
 */
void edit_record_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    lv_obj_t *enroll_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(!lv_obj_is_valid(enroll_scr)) {
        LV_LOG_WARN("父界面无效，拦截点击");
        return;
    }

    lv_obj_t *target = lv_event_get_target(e);
    if(!lv_obj_is_valid(target)) {
        LV_LOG_WARN("点击对象无效，拦截卡死");
        return;
    }

    pwd_enroll_info_t *pwd_info = get_current_pwd_info();
    finger_enroll_info_t *finger_info = get_current_finger_info();

    // 指纹编辑
    if(finger_info) {
        for(int i=0; i<MAX_FINGER_COUNT; i++){
            if(finger_info->finger_record_cons[i] == target && lv_obj_is_valid(finger_info->finger_record_cons[i])){
                ui_edit_record_create(EDIT_TYPE_FINGER, finger_info->finger_names[i], i, enroll_scr);
                //destroy_enroll();
                return;
            }
        }
    }

    // 密码编辑
    if(pwd_info) {
        for(int i=0; i<MAX_PWD_COUNT; i++){
            if(pwd_info->pwd_record_cons[i] == target && lv_obj_is_valid(pwd_info->pwd_record_cons[i])){
                ui_edit_record_create(EDIT_TYPE_PWD, pwd_info->pwd_names[i], i, enroll_scr);
                //destroy_enroll();
                return;
            }
        }
    }

    // 家庭成员专属：卡片+人脸
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY) {
        card_enroll_info_t *card_info = get_current_card_info();
        face_enroll_info_t *face_info = get_current_face_info();

        // 卡片编辑
        if(card_info) {
            for(int i=0; i<MAX_CARD_COUNT; i++){
                if(card_info->card_record_cons[i] == target && lv_obj_is_valid(card_info->card_record_cons[i])){
                    ui_edit_record_create(EDIT_TYPE_CARD, card_info->card_names[i], i, enroll_scr);
                    //destroy_enroll();
                    return;
                }
            }
        }
        // 人脸编辑
        if(face_info) {
            for(int i=0; i<MAX_FACE_COUNT; i++){
                if(face_info->face_record_cons[i] == target && lv_obj_is_valid(face_info->face_record_cons[i])){
                    ui_edit_record_create(EDIT_TYPE_FACE, face_info->face_names[i], i, enroll_scr);
                    //destroy_enroll();
                    return;
                }
            }
        }
    }

    LV_LOG_INFO("edit record btn click");
}

/**
 * @brief 名称输入框点击回调（弹出键盘）
 */
static void name_con_click_cb(lv_event_t *e)
{
    (void)e;
    LV_LOG_INFO("name container clicked, open keyboard");
    
    // 创建键盘（不存在则新建）
    if(!lv_obj_is_valid(g_keyboard)) {
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

/**
 * @brief 删除按钮回调（弹出确认弹窗）
 */
static void del_btn_click_cb(lv_event_t *e)
{
    lv_obj_t *enroll_scr = lv_event_get_user_data(e);
    LV_LOG_INFO("delete btn clicked, show dialog");
    close_dialog();

    // 遮罩层
    g_mask = lv_obj_create(edit_record_scr);
    lv_obj_set_size(g_mask, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(g_mask, lv_color_hex(0x000000), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(g_mask, LV_OPA_70, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(g_mask, 0, LV_STATE_DEFAULT);
    
    // 删除确认弹窗
    g_dialog = create_container(edit_record_scr, 212, 150, 600, 297, lv_color_hex(0xE0EDFF), LV_OPA_100, 16, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(g_dialog, 0, LV_STATE_DEFAULT);
    create_text_label(g_dialog, "确定删除吗", &eques_regular_32,lv_color_hex(0x000000), 0, 40, LV_OPA_100);
    lv_obj_align_to(lv_obj_get_child(g_dialog, 0), g_dialog, LV_ALIGN_TOP_MID, 0, 40);

    // 取消按钮
    lv_obj_t *cancel = create_container(g_dialog, 0, 0, 220, 44, lv_color_hex(0x192A46), LV_OPA_100, 6, lv_color_hex(0x1F3150), 0, LV_OPA_100);
    lv_obj_align(cancel, LV_ALIGN_TOP_MID, 0, 210);
    create_text_label(cancel, "取消", &eques_regular_32,lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align(lv_obj_get_child(cancel, 0), LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(cancel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(cancel, dialog_cancel_cb, LV_EVENT_CLICKED, NULL);

    // 确认按钮
    lv_obj_t *confirm = create_container(g_dialog, 0, 0, 220, 44, lv_color_hex(0x192A46), LV_OPA_100, 6, lv_color_hex(0x1F3150), 0, LV_OPA_100);
    lv_obj_align(confirm, LV_ALIGN_TOP_MID, 0, 140);
    create_text_label(confirm, "确定", &eques_regular_32, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align(lv_obj_get_child(confirm, 0), LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(confirm, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(confirm, dialog_confirm_cb, LV_EVENT_CLICKED, enroll_scr);
}

/**
 * @brief 删除确认回调
 */
static void dialog_confirm_cb(lv_event_t *e)
{
    (void)e;
    LV_LOG_INFO("dialog confirm, delete item");
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);
    // 执行删除操作
    edit_delete_item(g_edit_type, g_edit_index);
    close_dialog();
    hide_keyboard(NULL);

    // 返回录入界面并重建
    common_member_info_t *member = get_current_enroll_member();
    ui_enroll_create(member, g_parent_scr);  
    //lv_scr_load(parent_scr);

    // 销毁编辑界面
    if(lv_obj_is_valid(edit_record_scr)) {
        lv_obj_del(edit_record_scr);
        edit_record_scr = NULL;
    }
}

/**
 * @brief 删除取消回调
 */
static void dialog_cancel_cb(lv_event_t *e)
{
    (void)e;
    LV_LOG_INFO("dialog cancel, close dialog");
    close_dialog();
}

/**
 * @brief 关闭删除弹窗
 */
static void close_dialog(void)
{
    LV_LOG_INFO("close delete dialog");
    if(lv_obj_is_valid(g_mask)) lv_obj_del(g_mask);
    if(lv_obj_is_valid(g_dialog)) lv_obj_del(g_dialog);
    g_mask = NULL;
    g_dialog = NULL;
}

/**
 * @brief 隐藏并销毁键盘
 */
static void hide_keyboard(lv_event_t *e)
{
    (void)e;
    LV_LOG_INFO("delete keyboard");
    
    // 销毁键盘，防止野指针
    if(lv_obj_is_valid(g_keyboard)) {
        lv_obj_del(g_keyboard);
        g_keyboard = NULL;
    }
}
