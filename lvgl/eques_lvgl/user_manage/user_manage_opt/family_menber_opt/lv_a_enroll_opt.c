
#if 0

#else
#include "lv_a_enroll_opt.h"
#include "lv_finger_add.h"
#include "lv_face_add.h"
#include "lv_card_add.h"
#include "lv_pwd_add.h"
#include "lv_edit_record.h"
#include <string.h>
#include <stdio.h>

/******************************************************************************************
 *                                      宏定义
 ******************************************************************************************/
#define FINGER_CON_BASE_Y       46      // 指纹容器基础Y坐标
#define PWD_CON_BASE_OFFSET     20      // 密码容器与指纹容器的间距
#define CARD_CON_BASE_OFFSET    20      // 卡片容器与密码容器的间距
#define FACE_CON_BASE_OFFSET    20      // 人脸容器与卡片容器的间距
#define PWD_CON_DEFAULT_HEIGHT_FAMILY  188     // 密码容器默认高度(家庭成员)
#define PWD_CON_EXPAND_HEIGHT_FAMILY   275     // 密码容器展开高度(家庭成员)
#define PWD_CON_DEFAULT_HEIGHT_OTHER   290     // 密码容器默认高度(其他成员)
#define PWD_CON_EXPAND_HEIGHT_OTHER   375     // 密码容器展开高度(其他成员)

/******************************************************************************************
 *                                  全局变量声明
 ******************************************************************************************/
// -------------- UI对象 --------------
lv_obj_t *enroll_scr = NULL;                     // 录入主界面
static lv_obj_t *g_finger_opt_con = NULL;        // 指纹容器
static lv_obj_t *g_pwd_opt_con = NULL;           // 密码容器
static lv_obj_t *g_card_opt_con = NULL;          // 卡片容器
static lv_obj_t *g_face_opt_con = NULL;          // 人脸容器
static lv_obj_t *g_opt_con = NULL;               // 滚动父容器

// -------------- 样式 --------------
static lv_style_t enroll_grad_style;             // 背景渐变样式
static bool enroll_style_inited = false;         // 样式初始化标志

// -------------- 成员录入数据存储 --------------
finger_enroll_info_t g_family_finger_info[MAX_FAMILY_MEMBER_COUNT] = {0};   // 家庭成员指纹
finger_enroll_info_t g_other_finger_info[MAX_OTHER_MEMBER_COUNT] = {0};     // 其他成员指纹
pwd_enroll_info_t g_family_pwd_info[MAX_FAMILY_MEMBER_COUNT] = {0};         // 家庭成员密码
pwd_enroll_info_t g_other_pwd_info[MAX_OTHER_MEMBER_COUNT] = {0};           // 其他成员密码
card_enroll_info_t g_family_card_info[MAX_FAMILY_MEMBER_COUNT] = {0};       // 家庭成员卡片
face_enroll_info_t g_family_face_info[MAX_FAMILY_MEMBER_COUNT] = {0};       // 家庭成员人脸

// -------------- 当前操作成员状态 --------------
common_member_info_t g_current_enroll_member;    // 当前操作的成员信息
bool g_member_info_inited = false;               // 成员信息初始化标志
static bool s_deleting = false;                  // 删除防重入标志

/******************************************************************************************
 *                                  函数前置声明
 ******************************************************************************************/
// 样式与UI创建
static void init_enroll_styles(void);
void ui_enroll_create(common_member_info_t *member_info, lv_obj_t *parent_scr);
void destroy_enroll(void);

// 容器动态更新
static uint16_t update_finger_opt_container(void);
static uint16_t update_pwd_opt_container(uint16_t finger_con_height);
static uint16_t update_card_opt_container(uint16_t pwd_con_height, uint16_t finger_con_height);
static uint16_t update_face_opt_container(uint16_t card_con_height, uint16_t pwd_con_height, uint16_t finger_con_height);

// 录入完成回调
void finger_enroll_complete(const char *finger_name);
void pwd_enroll_complete(const char *pwd_name);
void card_enroll_complete(const char *card_name);
void face_enroll_complete(const char *face_name);

// 编辑操作
void edit_update_name(edit_type_e type, const char *new_name, uint8_t index);
void edit_delete_item(edit_type_e type, uint8_t index);

// 工具函数
finger_enroll_info_t *get_current_finger_info(void);
pwd_enroll_info_t *get_current_pwd_info(void);
card_enroll_info_t *get_current_card_info(void);
face_enroll_info_t *get_current_face_info(void);
common_member_info_t *get_current_enroll_member(void);
void enroll_opt_back_btn_click_cb(lv_event_t *e);

// 数据重置与清空
static void reset_member_finger_state(uint8_t idx, member_type_e type);
static void reset_member_pwd_state(uint8_t idx, member_type_e type);
static void reset_member_card_state(uint8_t idx, member_type_e type);
static void reset_member_face_state(uint8_t idx, member_type_e type);
void clear_member_all_biometrics(uint8_t member_idx, member_type_e type);

/******************************************************************************************
 *                               样式初始化（静态函数）
 ******************************************************************************************/
static void init_enroll_styles(void)
{
    if(!enroll_style_inited) {
        lv_style_init(&enroll_grad_style);
        enroll_style_inited = true;
    }
}

/******************************************************************************************
 *                               录入界面创建
 ******************************************************************************************/
void ui_enroll_create(common_member_info_t *member_info, lv_obj_t *parent_scr)
{
    init_enroll_styles();
    if(member_info == NULL || parent_scr == NULL) {
        LV_LOG_WARN("ui_enroll_create: invalid param!");
        return;
    }

    if(!g_member_info_inited) {
        memset(&g_current_enroll_member, 0, sizeof(common_member_info_t));
        g_member_info_inited = true;
    }
    // 复制当前成员信息
    g_current_enroll_member.type = member_info->type;
    g_current_enroll_member.idx = member_info->idx;
    strncpy(g_current_enroll_member.name, member_info->name, sizeof(g_current_enroll_member.name)-1);
    g_current_enroll_member.avatar_color = member_info->avatar_color;

    // 销毁旧界面，创建新界面
    if(is_lv_obj_valid(enroll_scr)) {
        lv_obj_del(enroll_scr);
        enroll_scr = NULL;
    }
    enroll_scr = lv_obj_create(NULL);
    lv_obj_clear_flag(enroll_scr, LV_OBJ_FLAG_SCROLLABLE);

    // 设置背景渐变样式
    lv_style_reset(&enroll_grad_style);
    lv_style_set_bg_color(&enroll_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&enroll_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&enroll_grad_style, LV_GRAD_DIR_VER);
    lv_style_set_bg_main_stop(&enroll_grad_style, 0);
    lv_style_set_bg_grad_stop(&enroll_grad_style, 255);
    lv_obj_add_style(enroll_scr, &enroll_grad_style, LV_STATE_DEFAULT);

    // 成员头像与名称
    char member_label_text[16] = {0};
    snprintf(member_label_text, sizeof(member_label_text), "编号: %s", g_current_enroll_member.name);
    
    lv_obj_t *avatar = create_container(enroll_scr,0,0,90,90, g_current_enroll_member.avatar_color, LV_OPA_100, 100,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_align(avatar, LV_ALIGN_TOP_MID, 0, 150);
    lv_obj_t *name_label = create_text_label(enroll_scr, member_label_text, &eques_regular_36, lv_color_hex(0xFFFFFF), 0, 398, LV_OPA_100);
    lv_obj_align(name_label, LV_ALIGN_TOP_MID, 0, 244);
    
    // 标题：家庭成员/其他成员
    char title_text[32] = {0};
    strcpy(title_text, g_current_enroll_member.type == MEMBER_TYPE_FAMILY ? "家庭成员" : "普通成员");
    create_text_label(enroll_scr, title_text, &eques_bold_36, lv_color_hex(0xFFFFFF), 83, 80, LV_OPA_100);

    // 创建滚动容器
    g_opt_con = create_container(enroll_scr,0,302,1024,399, lv_color_hex(0xE0EDFF), LV_OPA_100, 31,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(g_opt_con, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(g_opt_con, LV_OBJ_FLAG_SCROLLABLE);
    // 禁用弹性滚动
    lv_obj_clear_flag(g_opt_con, LV_OBJ_FLAG_SCROLL_ELASTIC);

    // 初始化所有录入容器
    uint16_t finger_con_height = update_finger_opt_container();
    uint16_t pwd_con_height = update_pwd_opt_container(finger_con_height);
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY) {
        uint16_t card_con_height = update_card_opt_container(pwd_con_height, finger_con_height);
        update_face_opt_container(card_con_height, pwd_con_height, finger_con_height);
    }
    
    // 返回按钮
    lv_obj_t *back_btn = create_text_label
    (enroll_scr, ICON_CHEVORN_LEFT, &fontawesome_icon_32, lv_color_hex(0xFFFFFF), 52, 84, LV_OPA_100);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(back_btn,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn,LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn,enroll_opt_back_btn_click_cb,LV_EVENT_CLICKED, parent_scr);
    
    update_status_bar_parent(enroll_scr);
    lv_scr_load(enroll_scr);
}

/******************************************************************************************
 *                               容器动态更新函数组
 ******************************************************************************************/
/**
 * @brief  更新指纹录入容器（防野指针，动态高度）
 * @return 容器最终高度
 */
static uint16_t update_finger_opt_container(void)
{
    const uint16_t BASE_HEIGHT      = 188;
    const uint16_t PER_ITEM_HEIGHT  = 87;
    const uint16_t LINE_TOP_Y       = 90;
    const uint16_t ADD_BTN_INIT_Y   = 122;

    static lv_point_t line_points_arr[MAX_FINGER_COUNT][2];
    uint16_t finger_con_height = BASE_HEIGHT;

    if(enroll_scr == NULL || g_opt_con == NULL) {
        LV_LOG_WARN("update_finger_opt_container: invalid parent obj!");
        return finger_con_height;
    }

    finger_enroll_info_t *finger_info = get_current_finger_info();
    if(finger_info == NULL)
        return finger_con_height;

    // 清空条目指针，杜绝野指针
    for (int i = 0; i < MAX_FINGER_COUNT; i++) {
        finger_info->finger_record_cons[i] = NULL;
    }

    finger_con_height = BASE_HEIGHT + finger_info->enroll_count * PER_ITEM_HEIGHT;

    // 复用/创建容器
    if(g_finger_opt_con != NULL && lv_obj_is_valid(g_finger_opt_con)) {
        lv_obj_clean(g_finger_opt_con);
        lv_obj_set_height(g_finger_opt_con, finger_con_height);
    }
    else {
        g_finger_opt_con = create_container(g_opt_con, 48, FINGER_CON_BASE_Y, 928, finger_con_height,
                                           lv_color_hex(0xFFFFFF), LV_OPA_100, 16,
                                           lv_color_hex(0x1F3150), 0, LV_OPA_90);
        lv_obj_set_style_pad_all(g_finger_opt_con, 0, LV_STATE_DEFAULT);
    }

    // 顶部标题与分割线
    create_text_label(g_finger_opt_con, ICON_FINGERPRINT_L, &fontawesome_icon_40, lv_color_hex(0x00BDBD), 28, 21, LV_OPA_100);
    lv_obj_t *finger_divider01 = lv_line_create(g_finger_opt_con);
    static lv_point_t divider_top_points[] = {{38, LINE_TOP_Y}, {881, LINE_TOP_Y}};
    config_divider_line_style(finger_divider01, divider_top_points, 2, 0xD4D4D4, 1, LV_OPA_100);
    create_text_label(g_finger_opt_con, "指纹",&eques_regular_32, lv_color_hex(0x27394C), 90, 23, LV_OPA_100);

    // 创建指纹条目
    for (uint8_t i = 0; i < finger_info->enroll_count && i < MAX_FINGER_COUNT; i++)
    {
        uint16_t line_y = 178 + (i * 88);
        line_points_arr[i][0].x = 38;
        line_points_arr[i][0].y = line_y;
        line_points_arr[i][1].x = 881;
        line_points_arr[i][1].y = line_y;
        
        lv_obj_t *divider = lv_line_create(g_finger_opt_con);
        lv_line_set_points(divider, line_points_arr[i], 2);
        config_divider_line_style(divider, line_points_arr[i], 2, 0xD4D4D4, 1, LV_OPA_100);

        uint16_t item_y = line_y + 1 - 88;
        finger_info->finger_record_cons[i] = create_container(g_finger_opt_con, 28, item_y, 881, PER_ITEM_HEIGHT,
                                                              lv_color_hex(0xFFFFFF), LV_OPA_100, 0,
                                                              lv_color_hex(0xFFFFFF), 0, LV_OPA_90);
        lv_obj_set_style_pad_all(finger_info->finger_record_cons[i], 0, LV_STATE_DEFAULT);

        char name_buf[16] = {0};
        if(finger_info->finger_names[i][0] == '\0')
            snprintf(name_buf, sizeof(name_buf), "指纹%d", i+1);
        else
            strncpy(name_buf, finger_info->finger_names[i], sizeof(name_buf)-1);

        create_text_label(finger_info->finger_record_cons[i], name_buf,&eques_regular_32, lv_color_hex(0x000000), 62, 24, LV_OPA_100);

        lv_obj_add_flag(finger_info->finger_record_cons[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_opa(finger_info->finger_record_cons[i], LV_OPA_90, LV_STATE_PRESSED);
        lv_obj_set_style_bg_color(finger_info->finger_record_cons[i], lv_color_hex(0xF0F0F0), LV_STATE_PRESSED);
        lv_obj_add_event_cb(finger_info->finger_record_cons[i], edit_record_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);
    }

    // 添加按钮
    uint16_t add_btn_y = ADD_BTN_INIT_Y + (finger_info->enroll_count * 88);
    uint16_t add_lbl_y = add_btn_y + 4;

    lv_obj_t * finger_add_img = create_container(g_finger_opt_con,418,add_btn_y,34,34,lv_color_hex(0x00BDBD), LV_OPA_100, 100,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(finger_add_img, 0, LV_STATE_DEFAULT);
    // +号图标
    lv_obj_t *divider_line1 = lv_line_create(finger_add_img);
    static const lv_point_t divider_points1[] = {{7, 17}, {27, 17}}; 
    config_divider_line_style(divider_line1, divider_points1, 2, 0xFFFFFF, 3, LV_OPA_100);
    lv_obj_t *divider_line2 = lv_line_create(finger_add_img);
    static const lv_point_t divider_points2[] = {{17, 7}, {17, 27}}; 
    config_divider_line_style(divider_line2, divider_points2, 2, 0xFFFFFF, 3, LV_OPA_100);
    
    lv_obj_t *finger_add_label = create_text_label(g_finger_opt_con, "添加",&eques_regular_24, lv_color_hex(0x00BDBD), 466, add_lbl_y, LV_OPA_100);

    // 按钮状态控制
    if(finger_info->enroll_count >= MAX_FINGER_COUNT) {
        lv_obj_set_style_text_color(finger_add_label, lv_color_hex(0x888888), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(finger_add_img, lv_color_hex(0xAAAAAA), LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(finger_add_img, lv_color_hex(0xAAAAAA), LV_STATE_DEFAULT);
        lv_obj_clear_flag(finger_add_img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_event_cb(finger_add_img, finger_add_btn_click_cb);
    } else {
        lv_obj_add_flag(finger_add_img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_color(finger_add_img, lv_color_hex(0x00BDBD), LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(finger_add_img, lv_color_hex(0x00BDBD), LV_STATE_DEFAULT);
        lv_obj_add_event_cb(finger_add_img, finger_add_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);
    }

    return finger_con_height;
}

/**
 * @brief  更新密码录入容器
 * @param finger_con_height 指纹容器高度（用于定位）
 * @return 容器最终高度
 */
static uint16_t update_pwd_opt_container(uint16_t finger_con_height)
{
    uint16_t BASE_HEIGHT      = 188;
    const uint16_t PER_ITEM_HEIGHT  = 87;
    const uint16_t LINE_TOP_Y       = 90;
    const uint16_t ADD_BTN_INIT_Y   = 122;

    static lv_point_t line_points_arr[MAX_PWD_COUNT][2];
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY) {
        BASE_HEIGHT = PWD_CON_DEFAULT_HEIGHT_FAMILY;
    } else {
        BASE_HEIGHT = PWD_CON_DEFAULT_HEIGHT_OTHER;
    }
    uint16_t pwd_con_height = BASE_HEIGHT;

    uint16_t pwd_con_y = FINGER_CON_BASE_Y + finger_con_height + PWD_CON_BASE_OFFSET;

    if(enroll_scr == NULL || g_opt_con == NULL) {
        LV_LOG_WARN("update_pwd_opt_container: invalid parent obj!");
        return pwd_con_height;
    }

    pwd_enroll_info_t *pwd_info = get_current_pwd_info();
    if(pwd_info == NULL)
        return pwd_con_height;

    // 清空条目指针
    for (int i = 0; i < MAX_PWD_COUNT; i++) {
        pwd_info->pwd_record_cons[i] = NULL;
    }

    pwd_con_height = BASE_HEIGHT + pwd_info->enroll_count * 87;

    // 复用/创建容器
    if(g_pwd_opt_con != NULL && lv_obj_is_valid(g_pwd_opt_con)) {
        lv_obj_clean(g_pwd_opt_con);
        lv_obj_set_height(g_pwd_opt_con, pwd_con_height);
        lv_obj_set_y(g_pwd_opt_con, pwd_con_y);
    }
    else {
        g_pwd_opt_con = create_container(g_opt_con, 48, pwd_con_y, 928, pwd_con_height,
                                           lv_color_hex(0xFFFFFF), LV_OPA_100, 16,
                                           lv_color_hex(0x1F3150), 0, LV_OPA_90);
        lv_obj_set_style_pad_all(g_pwd_opt_con, 0, LV_STATE_DEFAULT);
    }

    // 顶部标题与分割线
    create_text_label(g_pwd_opt_con, ICON_PASSWORD_L, &fontawesome_icon_40, lv_color_hex(0x00BDBD), 28, 17, LV_OPA_100);
    lv_obj_t *pwd_divider01 = lv_line_create(g_pwd_opt_con);
    static lv_point_t divider_top_points[] = {{38, LINE_TOP_Y}, {881, LINE_TOP_Y}};
    config_divider_line_style(pwd_divider01, divider_top_points, 2, 0xD4D4D4, 1, LV_OPA_100);
    create_text_label(g_pwd_opt_con, "密码",&eques_regular_32, lv_color_hex(0x27394C),90, 23, LV_OPA_100);

    // 创建密码条目
    for (uint8_t i = 0; i < pwd_info->enroll_count && i < MAX_PWD_COUNT; i++)
    {
        uint16_t line_y = 178 + (i * 88);
        line_points_arr[i][0].x = 38;
        line_points_arr[i][0].y = line_y;
        line_points_arr[i][1].x = 881;
        line_points_arr[i][1].y = line_y;
        
        lv_obj_t *divider = lv_line_create(g_pwd_opt_con);
        lv_line_set_points(divider, line_points_arr[i], 2);
        config_divider_line_style(divider, line_points_arr[i], 2, 0xD4D4D4, 1, LV_OPA_100);

        uint16_t item_y = line_y + 1 - 88;
        pwd_info->pwd_record_cons[i] = create_container(g_pwd_opt_con, 28, item_y, 881, PER_ITEM_HEIGHT,
                                                              lv_color_hex(0xFFFFFF), LV_OPA_100, 0,
                                                              lv_color_hex(0xFFFFFF), 0, LV_OPA_90);
        lv_obj_set_style_pad_all(pwd_info->pwd_record_cons[i], 0, LV_STATE_DEFAULT);

        char name_buf[32] = {0};
        if(pwd_info->pwd_names[i][0] == '\0')
            snprintf(name_buf, sizeof(name_buf), "密码%d", i+1);
        else
            strncpy(name_buf, pwd_info->pwd_names[i], sizeof(name_buf)-1);

        create_text_label(pwd_info->pwd_record_cons[i], name_buf,&eques_regular_32, lv_color_hex(0x000000), 62, 24, LV_OPA_100);

        lv_obj_add_flag(pwd_info->pwd_record_cons[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_opa(pwd_info->pwd_record_cons[i], LV_OPA_90, LV_STATE_PRESSED);
        lv_obj_set_style_bg_color(pwd_info->pwd_record_cons[i], lv_color_hex(0xF0F0F0), LV_STATE_PRESSED);
        lv_obj_add_event_cb(pwd_info->pwd_record_cons[i], edit_record_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);
    }

    // 添加按钮
    uint16_t add_btn_y = ADD_BTN_INIT_Y + (pwd_info->enroll_count * 88);
    uint16_t add_lbl_y = add_btn_y + 4;

    lv_obj_t * pwd_add_img = create_container(g_pwd_opt_con,418,add_btn_y,34,34,lv_color_hex(0x00BDBD), LV_OPA_100, 100,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(pwd_add_img, 0, LV_STATE_DEFAULT);
    lv_obj_t *divider_line1 = lv_line_create(pwd_add_img);
    static const lv_point_t divider_points1[] = {{7, 17}, {27, 17}}; 
    config_divider_line_style(divider_line1, divider_points1, 2, 0xFFFFFF, 3, LV_OPA_100);
    lv_obj_t *divider_line2 = lv_line_create(pwd_add_img);
    static const lv_point_t divider_points2[] = {{17, 7}, {17, 27}}; 
    config_divider_line_style(divider_line2, divider_points2, 2, 0xFFFFFF, 3, LV_OPA_100);

    lv_obj_t *pwd_add_label = create_text_label(g_pwd_opt_con, "添加", &eques_regular_24, lv_color_hex(0x00BDBD), 466, add_lbl_y, LV_OPA_100);

    // 按钮状态控制
    if(pwd_info->enroll_count >= MAX_PWD_COUNT) {
        lv_obj_set_style_text_color(pwd_add_label, lv_color_hex(0x888888), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(pwd_add_img, lv_color_hex(0xAAAAAA), LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(pwd_add_img, lv_color_hex(0xAAAAAA), LV_STATE_DEFAULT);   
        lv_obj_clear_flag(pwd_add_img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_event_cb(pwd_add_img, pwd_add_btn_click_cb);
    } else {
        lv_obj_add_flag(pwd_add_img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_color(pwd_add_img, lv_color_hex(0x00BDBD), LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(pwd_add_img, lv_color_hex(0x00BDBD), LV_STATE_DEFAULT);
        lv_obj_add_event_cb(pwd_add_img, pwd_add_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);
    }

    return pwd_con_height;
}

/**
 * @brief  更新卡片录入容器（仅家庭成员）
 * @param pwd_con_height 密码容器高度
 * @param finger_con_height 指纹容器高度
 * @return 容器最终高度
 */
static uint16_t update_card_opt_container(uint16_t pwd_con_height, uint16_t finger_con_height)
{
    const uint16_t BASE_HEIGHT      = 188;
    const uint16_t PER_ITEM_HEIGHT  = 87;
    const uint16_t LINE_TOP_Y       = 90;
    const uint16_t ADD_BTN_INIT_Y   = 122;

    static lv_point_t line_points_arr[MAX_CARD_COUNT][2];
    uint16_t card_con_height = BASE_HEIGHT;

    // 仅家庭成员显示卡片
    if(enroll_scr == NULL || g_opt_con == NULL || g_current_enroll_member.type != MEMBER_TYPE_FAMILY) {
        LV_LOG_WARN("update_card_opt_container: invalid parent obj or not family member!");
        return card_con_height;
    }

    uint16_t pwd_con_total_y = FINGER_CON_BASE_Y + finger_con_height + PWD_CON_BASE_OFFSET;
    uint16_t card_con_y = pwd_con_total_y + pwd_con_height + CARD_CON_BASE_OFFSET;

    card_enroll_info_t *card_info = get_current_card_info();
    if(card_info == NULL)
        return card_con_height;

    // 清空条目指针
    for (int i = 0; i < MAX_CARD_COUNT; i++) {
        card_info->card_record_cons[i] = NULL;
    }

    card_con_height = BASE_HEIGHT + card_info->enroll_count * 87;

    // 复用/创建容器
    if(g_card_opt_con != NULL && lv_obj_is_valid(g_card_opt_con)) {
        lv_obj_clean(g_card_opt_con);
        lv_obj_set_height(g_card_opt_con, card_con_height);
        lv_obj_set_y(g_card_opt_con, card_con_y);
    }
    else {
        g_card_opt_con = create_container(g_opt_con, 48, card_con_y, 928, card_con_height,
                                           lv_color_hex(0xFFFFFF), LV_OPA_100, 16,
                                           lv_color_hex(0x1F3150), 0, LV_OPA_90);
        lv_obj_set_style_pad_all(g_card_opt_con, 0, LV_STATE_DEFAULT);
    }

    // 顶部标题与分割线
    create_text_label(g_card_opt_con, ICON_CARD_L, &fontawesome_icon_40, lv_color_hex(0x00BDBD), 28, 30, LV_OPA_100);
    lv_obj_t *card_divider01 = lv_line_create(g_card_opt_con);
    static lv_point_t divider_top_points[] = {{38, LINE_TOP_Y}, {881, LINE_TOP_Y}};
    config_divider_line_style(card_divider01, divider_top_points, 2, 0xD4D4D4, 1, LV_OPA_100);
    create_text_label(g_card_opt_con, "卡片",&eques_regular_32, lv_color_hex(0x27394C), 95, 35, LV_OPA_100);

    // 创建卡片条目
    for (uint8_t i = 0; i < card_info->enroll_count && i < MAX_CARD_COUNT; i++)
    {
        uint16_t line_y = 178 + (i * 88);
        line_points_arr[i][0].x = 38;
        line_points_arr[i][0].y = line_y;
        line_points_arr[i][1].x = 881;
        line_points_arr[i][1].y = line_y;
        
        lv_obj_t *divider = lv_line_create(g_card_opt_con);
        lv_line_set_points(divider, line_points_arr[i], 2);
        config_divider_line_style(divider, line_points_arr[i], 2, 0xD4D4D4, 1, LV_OPA_100);

        uint16_t item_y = line_y + 1 - 88;
        card_info->card_record_cons[i] = create_container(g_card_opt_con, 28, item_y, 881, PER_ITEM_HEIGHT,
                                                              lv_color_hex(0xFFFFFF), LV_OPA_100, 0,
                                                              lv_color_hex(0xFFFFFF), 0, LV_OPA_90);
        lv_obj_set_style_pad_all(card_info->card_record_cons[i], 0, LV_STATE_DEFAULT);

        char name_buf[32] = {0};
        if(card_info->card_names[i][0] == '\0')
            snprintf(name_buf, sizeof(name_buf), "卡片%d", i+1);
        else
            strncpy(name_buf, card_info->card_names[i], sizeof(name_buf)-1);

        create_text_label(card_info->card_record_cons[i], name_buf,&eques_regular_32, lv_color_hex(0x000000), 62, 24, LV_OPA_100);

        lv_obj_add_flag(card_info->card_record_cons[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_opa(card_info->card_record_cons[i], LV_OPA_90, LV_STATE_PRESSED);
        lv_obj_set_style_bg_color(card_info->card_record_cons[i], lv_color_hex(0xF0F0F0), LV_STATE_PRESSED);
        lv_obj_add_event_cb(card_info->card_record_cons[i], edit_record_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);
    }

    // 添加按钮
    uint16_t add_btn_y = ADD_BTN_INIT_Y + (card_info->enroll_count * 88);
    uint16_t add_lbl_y = add_btn_y + 4;

    lv_obj_t *card_add_img   = create_container(g_card_opt_con,418,add_btn_y,34,34,lv_color_hex(0x00BDBD), LV_OPA_100, 100,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(card_add_img, 0, LV_STATE_DEFAULT);
    lv_obj_t *divider_line1 = lv_line_create(card_add_img);
    static const lv_point_t divider_points1[] = {{7, 17}, {27, 17}}; 
    config_divider_line_style(divider_line1, divider_points1, 2, 0xFFFFFF, 3, LV_OPA_100);
    lv_obj_t *divider_line2 = lv_line_create(card_add_img);
    static const lv_point_t divider_points2[] = {{17, 7}, {17, 27}}; 
    config_divider_line_style(divider_line2, divider_points2, 2, 0xFFFFFF, 3, LV_OPA_100);
    
    lv_obj_t *card_add_label = create_text_label(g_card_opt_con, "添加",&eques_regular_24, lv_color_hex(0x00BDBD), 466, add_lbl_y, LV_OPA_100);

    // 按钮状态控制
    if(card_info->enroll_count >= MAX_CARD_COUNT) {
        lv_obj_set_style_text_color(card_add_label, lv_color_hex(0x888888), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(card_add_img, lv_color_hex(0xAAAAAA), LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(card_add_img, lv_color_hex(0xAAAAAA), LV_STATE_DEFAULT);   
        lv_obj_clear_flag(card_add_img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_event_cb(card_add_img, card_add_btn_click_cb);
    } else {
        lv_obj_add_flag(card_add_img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_color(card_add_img, lv_color_hex(0x00BDBD), LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(card_add_img, lv_color_hex(0x00BDBD), LV_STATE_DEFAULT);
        lv_obj_add_event_cb(card_add_img, card_add_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);
    }
    return card_con_height;
}

/**
 * @brief  更新人脸录入容器（仅家庭成员）
 * @param card_con_height 卡片容器高度
 * @param pwd_con_height 密码容器高度
 * @param finger_con_height 指纹容器高度
 * @return 容器最终高度
 */
static uint16_t update_face_opt_container(uint16_t card_con_height, uint16_t pwd_con_height, uint16_t finger_con_height)
{
    const uint16_t BASE_HEIGHT      = 290;
    const uint16_t PER_ITEM_HEIGHT  = 87;
    const uint16_t LINE_TOP_Y       = 90;
    const uint16_t ADD_BTN_INIT_Y   = 122;

    static lv_point_t line_points_arr[MAX_FACE_COUNT][2];
    uint16_t face_con_height = BASE_HEIGHT;

    // 仅家庭成员显示人脸
    if(enroll_scr == NULL || g_opt_con == NULL || g_current_enroll_member.type != MEMBER_TYPE_FAMILY) {
        LV_LOG_WARN("update_face_opt_container: invalid parent obj or not family member!");
        return face_con_height;
    }

    uint16_t pwd_con_total_y = FINGER_CON_BASE_Y + finger_con_height + PWD_CON_BASE_OFFSET;
    uint16_t card_con_total_y = pwd_con_total_y + pwd_con_height + CARD_CON_BASE_OFFSET;
    uint16_t face_con_y = card_con_total_y + card_con_height + FACE_CON_BASE_OFFSET;

    face_enroll_info_t *face_info = get_current_face_info();
    if(face_info == NULL)
        return face_con_height;

    // 清空条目指针
    for (int i = 0; i < MAX_FACE_COUNT; i++) {
        face_info->face_record_cons[i] = NULL;
    }

    face_con_height = BASE_HEIGHT + face_info->enroll_count * 87;

    // 复用/创建容器
    if(g_face_opt_con != NULL && lv_obj_is_valid(g_face_opt_con)) {
        lv_obj_clean(g_face_opt_con);
        lv_obj_set_height(g_face_opt_con, face_con_height);
        lv_obj_set_y(g_face_opt_con, face_con_y);
    }
    else {
        g_face_opt_con = create_container(g_opt_con, 48, face_con_y, 928, face_con_height,
                                           lv_color_hex(0xFFFFFF), LV_OPA_100, 16,
                                           lv_color_hex(0x1F3150), 0, LV_OPA_90);
        lv_obj_set_style_pad_all(g_face_opt_con, 0, LV_STATE_DEFAULT);
    }

    // 顶部标题与分割线
    create_text_label(g_face_opt_con, ICON_FACE_L, &fontawesome_icon_40, lv_color_hex(0x00BDBD), 28, 30, LV_OPA_100);
    lv_obj_t *face_divider01 = lv_line_create(g_face_opt_con);
    static lv_point_t divider_top_points[] = {{38, LINE_TOP_Y}, {881, LINE_TOP_Y}};
    config_divider_line_style(face_divider01, divider_top_points, 2, 0xD4D4D4, 1, LV_OPA_100);
    create_text_label(g_face_opt_con, "面容",&eques_regular_32, lv_color_hex(0x27394C),95, 35, LV_OPA_100);

    // 创建人脸条目
    for (uint8_t i = 0; i < face_info->enroll_count && i < MAX_FACE_COUNT; i++)
    {
        uint16_t line_y = 178 + (i * 88);
        line_points_arr[i][0].x = 38;
        line_points_arr[i][0].y = line_y;
        line_points_arr[i][1].x = 881;
        line_points_arr[i][1].y = line_y;
        
        lv_obj_t *divider = lv_line_create(g_face_opt_con);
        lv_line_set_points(divider, line_points_arr[i], 2);
        config_divider_line_style(divider, line_points_arr[i], 2, 0xD4D4D4, 1, LV_OPA_100);

        uint16_t item_y = line_y + 1 - 88;
        face_info->face_record_cons[i] = create_container(g_face_opt_con, 28, item_y, 881, PER_ITEM_HEIGHT,
                                                              lv_color_hex(0xFFFFFF), LV_OPA_100, 0,
                                                              lv_color_hex(0xFFFFFF), 0, LV_OPA_90);
        lv_obj_set_style_pad_all(face_info->face_record_cons[i], 0, LV_STATE_DEFAULT);

        char name_buf[32] = {0};
        if(face_info->face_names[i][0] == '\0')
            snprintf(name_buf, sizeof(name_buf), "面容%d", i+1);
        else
            strncpy(name_buf, face_info->face_names[i], sizeof(name_buf)-1);

        create_text_label(face_info->face_record_cons[i], name_buf,&eques_regular_32, lv_color_hex(0x000000), 62, 24, LV_OPA_100);

        lv_obj_add_flag(face_info->face_record_cons[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_opa(face_info->face_record_cons[i], LV_OPA_90, LV_STATE_PRESSED);
        lv_obj_set_style_bg_color(face_info->face_record_cons[i], lv_color_hex(0xF0F0F0), LV_STATE_PRESSED);
        lv_obj_add_event_cb(face_info->face_record_cons[i], edit_record_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);
    }

    // 添加按钮
    uint16_t add_btn_y = ADD_BTN_INIT_Y + (face_info->enroll_count * 88);
    uint16_t add_lbl_y = add_btn_y + 4;

    lv_obj_t *face_add_img = create_container(g_face_opt_con,418,add_btn_y,34,34,lv_color_hex(0x00BDBD), LV_OPA_100, 100,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(face_add_img, 0, LV_STATE_DEFAULT);
    lv_obj_t *divider_line1 = lv_line_create(face_add_img);
    static const lv_point_t divider_points1[] = {{7, 17}, {27, 17}}; 
    config_divider_line_style(divider_line1, divider_points1, 2, 0xFFFFFF, 3, LV_OPA_100);
    lv_obj_t *divider_line2 = lv_line_create(face_add_img);
    static const lv_point_t divider_points2[] = {{17, 7}, {17, 27}}; 
    config_divider_line_style(divider_line2, divider_points2, 2, 0xFFFFFF, 3, LV_OPA_100);
    
    lv_obj_t *face_add_label = create_text_label(g_face_opt_con, "添加",&eques_regular_24, lv_color_hex(0x00BDBD), 466, add_lbl_y, LV_OPA_100);
    
    // 按钮状态控制
    if(face_info->enroll_count >= MAX_FACE_COUNT) {
        lv_obj_set_style_text_color(face_add_label, lv_color_hex(0x888888), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(face_add_img, lv_color_hex(0xAAAAAA), LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(face_add_img, lv_color_hex(0xAAAAAA), LV_STATE_DEFAULT);   
        lv_obj_clear_flag(face_add_img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_event_cb(face_add_img, face_add_btn_click_cb);
    } else {
        lv_obj_add_flag(face_add_img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_color(face_add_img, lv_color_hex(0x00BDBD), LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(face_add_img, lv_color_hex(0x00BDBD), LV_STATE_DEFAULT);
        lv_obj_add_event_cb(face_add_img, face_add_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);
    }

    return face_con_height;
}

/******************************************************************************************
 *                               录入完成回调函数组
 ******************************************************************************************/
// 指纹录入完成
void finger_enroll_complete(const char *finger_name)
{
    finger_enroll_info_t *info = get_current_finger_info();
    if(!info || info->enroll_count >= MAX_FINGER_COUNT) return;

    uint8_t index = info->enroll_count;
    if(finger_name) strncpy(info->finger_names[index], finger_name, sizeof(info->finger_names[0])-1);
    else snprintf(info->finger_names[index], sizeof(info->finger_names[0]), "指纹%d", index+1);

    info->enroll_count++;
    LV_LOG_INFO("Finger enroll complete, count: %d", info->enroll_count);
    
    // 刷新UI
    uint16_t finger_height = update_finger_opt_container();
    uint16_t pwd_height = update_pwd_opt_container(finger_height);
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY) {
        uint16_t card_height = update_card_opt_container(pwd_height, finger_height);
        update_face_opt_container(card_height, pwd_height, finger_height);
    }

    // 同步计数到成员列表
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY && g_current_enroll_member.idx < MAX_FAMILY_MEMBER_COUNT) {
        family_member_list[g_current_enroll_member.idx].finger_count = info->enroll_count;
        update_member_count_ui(g_current_enroll_member.idx);
    } else if(g_current_enroll_member.type == MEMBER_TYPE_OTHER && g_current_enroll_member.idx < MAX_OTHER_MEMBER_COUNT) {
        other_member_list[g_current_enroll_member.idx].finger_count = info->enroll_count;
        update_other_member_count_ui(g_current_enroll_member.idx);
    }
}

// 密码录入完成
void pwd_enroll_complete(const char *pwd_name)
{
    pwd_enroll_info_t *info = get_current_pwd_info();
    if(!info || info->enroll_count >= MAX_PWD_COUNT) return;

    uint8_t index = info->enroll_count;
    if(pwd_name) strncpy(info->pwd_names[index], pwd_name, sizeof(info->pwd_names[0])-1);
    else snprintf(info->pwd_names[index], sizeof(info->pwd_names[0]), "密码%d", index+1);
    
    info->enroll_count++;
    LV_LOG_INFO("Pwd enroll complete, count: %d", info->enroll_count);
    
    // 刷新UI
    uint16_t finger_height = update_finger_opt_container();
    uint16_t pwd_height = update_pwd_opt_container(finger_height);
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY) {
        uint16_t card_height = update_card_opt_container(pwd_height, finger_height);
        update_face_opt_container(card_height, pwd_height, finger_height);
    }

    // 同步计数
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY && g_current_enroll_member.idx < MAX_FAMILY_MEMBER_COUNT) {
        family_member_list[g_current_enroll_member.idx].pwd_count = info->enroll_count;
        update_member_count_ui(g_current_enroll_member.idx);
    } else if(g_current_enroll_member.type == MEMBER_TYPE_OTHER && g_current_enroll_member.idx < MAX_OTHER_MEMBER_COUNT) {
        other_member_list[g_current_enroll_member.idx].pwd_count = info->enroll_count;
        update_other_member_count_ui(g_current_enroll_member.idx);
    }
}

// 卡片录入完成
void card_enroll_complete(const char *card_name)
{
    card_enroll_info_t *info = get_current_card_info();
    if(!info || info->enroll_count >= MAX_CARD_COUNT) return;

    uint8_t index = info->enroll_count;
    if(card_name) strncpy(info->card_names[index], card_name, sizeof(info->card_names[0])-1);
    else snprintf(info->card_names[index], sizeof(info->card_names[0]), "card%d", index+1);
    
    info->enroll_count++;
    LV_LOG_INFO("Card enroll complete, count: %d", info->enroll_count);
    
    // 刷新UI
    uint16_t finger_height = update_finger_opt_container();
    uint16_t pwd_height = update_pwd_opt_container(finger_height);
    uint16_t card_height = update_card_opt_container(pwd_height, finger_height);
    update_face_opt_container(card_height, pwd_height, finger_height);

    // 同步计数
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY && g_current_enroll_member.idx < MAX_FAMILY_MEMBER_COUNT) {
        family_member_list[g_current_enroll_member.idx].card_count = info->enroll_count;
        update_member_count_ui(g_current_enroll_member.idx);
    }
}

// 人脸录入完成
void face_enroll_complete(const char *face_name)
{
    face_enroll_info_t *info = get_current_face_info();
    if(!info || info->enroll_count >= MAX_FACE_COUNT) return;
    
    uint8_t index = info->enroll_count;
    if(face_name) strncpy(info->face_names[index], face_name, sizeof(info->face_names[0])-1);
    else snprintf(info->face_names[index], sizeof(info->face_names[0]), "face%d", index+1);
    
    info->enroll_count++;
    
    // 刷新UI
    uint16_t finger_height = update_finger_opt_container();
    uint16_t pwd_height = update_pwd_opt_container(finger_height);
    uint16_t card_height = update_card_opt_container(pwd_height, finger_height);
    update_face_opt_container(card_height, pwd_height, finger_height);

    // 同步计数
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY && g_current_enroll_member.idx < MAX_FAMILY_MEMBER_COUNT) {
        family_member_list[g_current_enroll_member.idx].face_count = info->enroll_count;
        update_member_count_ui(g_current_enroll_member.idx);
    }
}

/******************************************************************************************
 *                               编辑操作函数组（修改/删除）
 ******************************************************************************************/
// 修改录入项名称
void edit_update_name(edit_type_e type, const char *new_name, uint8_t index)
{
    if(!g_member_info_inited || !new_name) {
        LV_LOG_ERROR("Error: Member not initialized or name pointer NULL");
        return;
    }

    switch(type)
    {
        case EDIT_TYPE_PWD: {
            pwd_enroll_info_t *info = get_current_pwd_info();
            if(info && index < MAX_PWD_COUNT) {
                memset(info->pwd_names[index], 0, sizeof(info->pwd_names[0]));
                strncpy(info->pwd_names[index], new_name, sizeof(info->pwd_names[0])-1);
            } else {
                LV_LOG_ERROR("[PWD] Fatal error: NULL pointer or out of bounds");
            }
            break;
        }
        case EDIT_TYPE_CARD: {
            card_enroll_info_t *info = get_current_card_info();
            if(info && index < MAX_CARD_COUNT) {
                memset(info->card_names[index], 0, sizeof(info->card_names[0]));
                strncpy(info->card_names[index], new_name, sizeof(info->card_names[0])-1);
            } else {
                LV_LOG_ERROR("[CARD] Fatal error: NULL pointer or out of bounds");
            }
            break;
        }
        case EDIT_TYPE_FINGER: {
            finger_enroll_info_t *info = get_current_finger_info();
            if(info && index < MAX_FINGER_COUNT) {
                memset(info->finger_names[index], 0, sizeof(info->finger_names[0]));
                strncpy(info->finger_names[index], new_name, sizeof(info->finger_names[0])-1);
            } else {
                LV_LOG_ERROR("[FINGER] Fatal error: NULL pointer or out of bounds");
            }
            break;
        }
        case EDIT_TYPE_FACE: {
            face_enroll_info_t *info = get_current_face_info();
            if(info && index < MAX_FACE_COUNT) {
                memset(info->face_names[index], 0, sizeof(info->face_names[0]));
                strncpy(info->face_names[index], new_name, sizeof(info->face_names[0])-1);
            } else {
                LV_LOG_ERROR("[FACE] Fatal error: NULL pointer or out of bounds");
            }
            break;
        }
        default:
            LV_LOG_ERROR("Error: Unknown edit type: %d", type);
            return;
    }

    // 刷新全部UI
    uint16_t finger_h = update_finger_opt_container();
    uint16_t pwd_h = update_pwd_opt_container(finger_h);
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY) {
        uint16_t card_h = update_card_opt_container(pwd_h, finger_h);
        update_face_opt_container(card_h, pwd_h, finger_h);
    }

    LV_LOG_USER("===== edit_update_name finished =====");
}

// 删除录入项（防重入）
void edit_delete_item(edit_type_e type, uint8_t index)
{
    if (!g_member_info_inited || s_deleting) return;
    s_deleting = true;

    finger_enroll_info_t *finger_info = get_current_finger_info();
    pwd_enroll_info_t *pwd_info = get_current_pwd_info();
    card_enroll_info_t *card_info = get_current_card_info();
    face_enroll_info_t *face_info = get_current_face_info();

    int i = 0;
    switch (type)
    {
        case EDIT_TYPE_FINGER:
            if(!finger_info || index < 0 || index >= finger_info->enroll_count) break;
            for(i = index; i < finger_info->enroll_count - 1; i++){
                strcpy(finger_info->finger_names[i], finger_info->finger_names[i+1]);
                finger_info->finger_record_cons[i] = finger_info->finger_record_cons[i+1];
            }
            memset(&finger_info->finger_names[finger_info->enroll_count-1], 0, sizeof(finger_info->finger_names[0]));
            finger_info->finger_record_cons[finger_info->enroll_count-1] = NULL;
            finger_info->enroll_count--;
            break;

        case EDIT_TYPE_PWD:
            if(!pwd_info || index < 0 || index >= pwd_info->enroll_count) break;
            for(i = index; i < pwd_info->enroll_count - 1; i++){
                strcpy(pwd_info->pwd_names[i], pwd_info->pwd_names[i+1]);
                pwd_info->pwd_record_cons[i] = pwd_info->pwd_record_cons[i+1];
            }
            memset(&pwd_info->pwd_names[pwd_info->enroll_count-1], 0, sizeof(pwd_info->pwd_names[0]));
            pwd_info->pwd_record_cons[pwd_info->enroll_count-1] = NULL;
            pwd_info->enroll_count--;
            break;

        case EDIT_TYPE_CARD:
            if(!card_info || index < 0 || index >= card_info->enroll_count) break;
            for(i = index; i < card_info->enroll_count - 1; i++){
                strcpy(card_info->card_names[i], card_info->card_names[i+1]);
                card_info->card_record_cons[i] = card_info->card_record_cons[i+1];
            }
            memset(&card_info->card_names[card_info->enroll_count-1], 0, sizeof(card_info->card_names[0]));
            card_info->card_record_cons[card_info->enroll_count-1] = NULL;
            card_info->enroll_count--;
            break;

        case EDIT_TYPE_FACE:
            if(!face_info || index < 0 || index >= face_info->enroll_count) break;
            for(i = index; i < face_info->enroll_count - 1; i++){
                strcpy(face_info->face_names[i], face_info->face_names[i+1]);
                face_info->face_record_cons[i] = face_info->face_record_cons[i+1];
            }
            memset(&face_info->face_names[face_info->enroll_count-1], 0, sizeof(face_info->face_names[0]));
            face_info->face_record_cons[face_info->enroll_count-1] = NULL;
            face_info->enroll_count--;
            break;

        default: break;
    }

    // 刷新UI
    uint16_t finger_h = update_finger_opt_container();
    uint16_t pwd_h = update_pwd_opt_container(finger_h);
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY) {
        uint16_t card_h = update_card_opt_container(pwd_h, finger_h);
        update_face_opt_container(card_h, pwd_h, finger_h);
    }

    // 同步计数
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY && g_current_enroll_member.idx < MAX_FAMILY_MEMBER_COUNT) {
        family_member_list[g_current_enroll_member.idx].finger_count = finger_info ? finger_info->enroll_count : 0;
        family_member_list[g_current_enroll_member.idx].pwd_count = pwd_info ? pwd_info->enroll_count : 0;
        family_member_list[g_current_enroll_member.idx].card_count = card_info ? card_info->enroll_count : 0;
        family_member_list[g_current_enroll_member.idx].face_count = face_info ? face_info->enroll_count : 0;
        update_member_count_ui(g_current_enroll_member.idx);
    } 
    else if(g_current_enroll_member.type == MEMBER_TYPE_OTHER && g_current_enroll_member.idx < MAX_OTHER_MEMBER_COUNT) {
        other_member_list[g_current_enroll_member.idx].finger_count = finger_info ? finger_info->enroll_count : 0;
        other_member_list[g_current_enroll_member.idx].pwd_count = pwd_info ? pwd_info->enroll_count : 0;
        update_other_member_count_ui(g_current_enroll_member.idx);
    }

    s_deleting = false;
}

/******************************************************************************************
 *                               工具函数组
 ******************************************************************************************/
// 获取当前指纹信息
finger_enroll_info_t *get_current_finger_info(void)
{
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY) {
        if(g_current_enroll_member.idx >= MAX_FAMILY_MEMBER_COUNT) return NULL;
        return &g_family_finger_info[g_current_enroll_member.idx];
    } else {
        if(g_current_enroll_member.idx >= MAX_OTHER_MEMBER_COUNT) return NULL;
        return &g_other_finger_info[g_current_enroll_member.idx];
    }
}

// 获取当前密码信息
pwd_enroll_info_t *get_current_pwd_info(void)
{
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY) {
        if(g_current_enroll_member.idx >= MAX_FAMILY_MEMBER_COUNT) return NULL;
        return &g_family_pwd_info[g_current_enroll_member.idx];
    } else {
        if(g_current_enroll_member.idx >= MAX_OTHER_MEMBER_COUNT) return NULL;
        return &g_other_pwd_info[g_current_enroll_member.idx];
    }
}

// 获取当前卡片信息
card_enroll_info_t *get_current_card_info(void)
{
    if(g_current_enroll_member.type != MEMBER_TYPE_FAMILY) return NULL;
    if(g_current_enroll_member.idx >= MAX_FAMILY_MEMBER_COUNT) return NULL;
    return &g_family_card_info[g_current_enroll_member.idx];
}

// 获取当前人脸信息
face_enroll_info_t *get_current_face_info(void)
{
    if(g_current_enroll_member.type != MEMBER_TYPE_FAMILY) return NULL;
    if(g_current_enroll_member.idx >= MAX_FAMILY_MEMBER_COUNT) return NULL;
    return &g_family_face_info[g_current_enroll_member.idx];
}

// 获取当前操作成员
common_member_info_t *get_current_enroll_member(void)
{
    return &g_current_enroll_member;
}

// 返回按钮点击回调
void enroll_opt_back_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);
    
    lv_obj_t *current_del_scr = lv_disp_get_scr_act(NULL);
    if(!lv_obj_is_valid(current_del_scr) || current_del_scr != enroll_scr) return;

    // 返回对应列表界面
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY) {
        ui_family_menber_create(parent_scr);  
    } 
    else if(g_current_enroll_member.type == MEMBER_TYPE_OTHER) {
        ui_other_member_create(parent_scr);  
    }

    lv_obj_del(current_del_scr);
    enroll_scr = NULL;
}

/******************************************************************************************
 *                               数据重置/清空函数组
 ******************************************************************************************/
// 重置指纹状态
static void reset_member_finger_state(uint8_t idx, member_type_e type)
{
    finger_enroll_info_t *info = NULL;
    if(type == MEMBER_TYPE_FAMILY) {
        if(idx >= MAX_FAMILY_MEMBER_COUNT) return;
        info = &g_family_finger_info[idx];
    } else {
        if(idx >= MAX_OTHER_MEMBER_COUNT) return;
        info = &g_other_finger_info[idx];
    }
    
    info->enroll_count = 0;
    memset(info->finger_names, 0, sizeof(info->finger_names));
    for(int i=0; i<MAX_FINGER_COUNT; i++) {
        if(info->finger_record_cons[i] != NULL && lv_obj_is_valid(info->finger_record_cons[i])) {
            lv_obj_del(info->finger_record_cons[i]);
            info->finger_record_cons[i] = NULL;
        }
    }
    LV_LOG_INFO("Reset finger state for %s member %d", 
                type == MEMBER_TYPE_FAMILY ? "family" : "other", idx);
}

// 重置密码状态
static void reset_member_pwd_state(uint8_t idx, member_type_e type)
{
    pwd_enroll_info_t *info = type == MEMBER_TYPE_FAMILY ? &g_family_pwd_info[idx] : &g_other_pwd_info[idx];
    info->enroll_count = 0;
    memset(info->pwd_names, 0, sizeof(info->pwd_names));
    for(int i=0; i<MAX_PWD_COUNT; i++) {
        if(info->pwd_record_cons[i]) lv_obj_del(info->pwd_record_cons[i]);
        info->pwd_record_cons[i] = NULL;
    }
}

// 重置卡片状态
static void reset_member_card_state(uint8_t idx, member_type_e type)
{
    if(type != MEMBER_TYPE_FAMILY) return;
    card_enroll_info_t *info = &g_family_card_info[idx];
    info->enroll_count = 0;
    memset(info->card_names, 0, sizeof(info->card_names));
    for(int i=0; i<MAX_CARD_COUNT; i++) {
        if(info->card_record_cons[i]) lv_obj_del(info->card_record_cons[i]);
        info->card_record_cons[i] = NULL;
    }
}

// 重置人脸状态
static void reset_member_face_state(uint8_t idx, member_type_e type)
{
    if(type != MEMBER_TYPE_FAMILY) return;
    face_enroll_info_t *info = &g_family_face_info[idx];
    info->enroll_count = 0;
    memset(info->face_names, 0, sizeof(info->face_names));
    for(int i=0; i<MAX_FACE_COUNT; i++) {
        if(info->face_record_cons[i]) lv_obj_del(info->face_record_cons[i]);
        info->face_record_cons[i] = NULL;
    }
}

// 清空成员所有生物特征数据
void clear_member_all_biometrics(uint8_t member_idx, member_type_e type)
{
    if(type == MEMBER_TYPE_FAMILY && member_idx >= MAX_FAMILY_MEMBER_COUNT){
        return;
    }
    if(type == MEMBER_TYPE_OTHER && member_idx >= MAX_OTHER_MEMBER_COUNT){
        return;
    }

    if(type == MEMBER_TYPE_FAMILY)
    {
        memset(&g_family_finger_info[member_idx], 0, sizeof(finger_enroll_info_t));
        memset(&g_family_pwd_info[member_idx],   0, sizeof(pwd_enroll_info_t));
        memset(&g_family_card_info[member_idx],  0, sizeof(card_enroll_info_t));
        memset(&g_family_face_info[member_idx],  0, sizeof(face_enroll_info_t));
        
        for(int i=0; i<MAX_FINGER_COUNT; i++) g_family_finger_info[member_idx].finger_record_cons[i] = NULL;
        for(int i=0; i<MAX_PWD_COUNT; i++)    g_family_pwd_info[member_idx].pwd_record_cons[i] = NULL;
        for(int i=0; i<MAX_CARD_COUNT; i++)   g_family_card_info[member_idx].card_record_cons[i] = NULL;
        for(int i=0; i<MAX_FACE_COUNT; i++)   g_family_face_info[member_idx].face_record_cons[i] = NULL;
    }
    else
    {
        memset(&g_other_finger_info[member_idx], 0, sizeof(finger_enroll_info_t));
        memset(&g_other_pwd_info[member_idx],   0, sizeof(pwd_enroll_info_t));
        
        for(int i=0; i<MAX_FINGER_COUNT; i++) g_other_finger_info[member_idx].finger_record_cons[i] = NULL;
        for(int i=0; i<MAX_PWD_COUNT; i++)    g_other_pwd_info[member_idx].pwd_record_cons[i] = NULL;
    }
}

// 销毁录入界面
void destroy_enroll(void)
{
    if(enroll_scr == NULL || !lv_obj_is_valid(enroll_scr)) return;
    lv_obj_del(enroll_scr);
    enroll_scr = NULL;
    LV_LOG_WARN("Enroll response: Destroy the enroll interface");
}
#endif
