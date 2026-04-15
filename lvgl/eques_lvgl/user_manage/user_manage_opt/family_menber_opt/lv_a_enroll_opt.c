#if 0
#include "lv_a_enroll_opt.h"
#include "lv_finger_add.h"
#include "lv_face_add.h"
#include "lv_card_add.h"
#include "lv_pwd_add.h"
#include "lv_edit_record.h"

#include <string.h>
#include <stdio.h>

static lv_obj_t *enroll_scr = NULL; 
static lv_style_t enroll_grad_style;
static bool enroll_style_inited = false;
lv_obj_t *g_finger_count_label = NULL;

// ========== 宏定义 ==========

#define FINGER_CON_BASE_Y       46      // 指纹容器基础Y坐标
#define PWD_CON_BASE_OFFSET     20      // 密码容器与指纹容器的间距
#define CARD_CON_BASE_OFFSET    20      // 卡片容器与密码容器的间距
#define FACE_CON_BASE_OFFSET    20      // 人脸容器与卡片容器的间距
#define PWD_RECORD_HEIGHT       87      // 密码记录项高度
#define CARD_RECORD_HEIGHT      87      // 卡片记录项高度
#define FACE_RECORD_HEIGHT      87      // 人脸记录项高度
#define PWD_CON_DEFAULT_HEIGHT_FAMILY  188     // 密码容器默认高度(家庭成员)
#define PWD_CON_EXPAND_HEIGHT_FAMILY   275     // 密码容器展开高度(家庭成员)
#define PWD_CON_DEFAULT_HEIGHT_OTHER   290     // 密码容器默认高度(其他成员)
#define PWD_CON_EXPAND_HEIGHT_OTHER   375     // 密码容器展开高度(其他成员)
#define CARD_CON_DEFAULT_HEIGHT 188     // 卡片容器默认高度
#define CARD_CON_EXPAND_HEIGHT  275     // 卡片容器展开高度
#define FACE_CON_DEFAULT_HEIGHT 290     // 人脸容器默认高度
#define FACE_CON_EXPAND_HEIGHT  375     // 人脸容器展开高度




// ========== 全局数据 ==========
// 家庭成员指纹数据
finger_enroll_info_t g_family_finger_info[MAX_FAMILY_MEMBER_COUNT] = {0};
// 其他成员指纹数据
finger_enroll_info_t g_other_finger_info[MAX_OTHER_MEMBER_COUNT] = {0};

// 家庭成员密码数据
pwd_enroll_info_t g_family_pwd_info[MAX_FAMILY_MEMBER_COUNT] = {0};
// 其他成员密码数据
pwd_enroll_info_t g_other_pwd_info[MAX_OTHER_MEMBER_COUNT] = {0};

// 家庭成员卡片数据
card_enroll_info_t g_family_card_info[MAX_FAMILY_MEMBER_COUNT] = {0};
// 家庭成员人脸数据
face_enroll_info_t g_family_face_info[MAX_FAMILY_MEMBER_COUNT] = {0};

// 当前操作的成员信息
common_member_info_t g_current_enroll_member;
bool g_member_info_inited = false;

static lv_obj_t *g_finger_opt_con = NULL; 
static lv_obj_t *g_pwd_opt_con = NULL;    
static lv_obj_t *g_card_opt_con = NULL;   //  卡片容器全局引用
static lv_obj_t *g_face_opt_con = NULL;   //  人脸容器全局引用
static lv_obj_t *g_opt_con = NULL;

static finger_enroll_info_t *get_current_finger_info(void);
pwd_enroll_info_t *get_current_pwd_info(void);
card_enroll_info_t *get_current_card_info(void); //  获取当前卡片信息
face_enroll_info_t *get_current_face_info(void); //  获取当前人脸信息
static uint16_t update_pwd_opt_container(uint16_t finger_con_height);
static uint16_t update_card_opt_container(uint16_t pwd_con_height, uint16_t finger_con_height);
static uint16_t update_face_opt_container(uint16_t card_con_height, uint16_t pwd_con_height, uint16_t finger_con_height);

// 全局样式初始化
static void init_enroll_styles(void)
{
    if(!enroll_style_inited) {
        lv_style_init(&enroll_grad_style);
        enroll_style_inited = true;
    }
}

// ========== 指纹容器更新 ==========
static uint16_t update_finger_opt_container(void)
{
    uint16_t finger_con_height = 188; // 默认高度
    
    if(enroll_scr == NULL || g_opt_con == NULL) {
        LV_LOG_WARN("update_finger_opt_container: invalid parent obj!");
        return finger_con_height;
    }

    if(g_finger_opt_con != NULL && lv_obj_is_valid(g_finger_opt_con)) {
        lv_obj_del(g_finger_opt_con);
        g_finger_opt_con = NULL;
    }

    // 获取当前成员的指纹数据
    finger_enroll_info_t *finger_info = get_current_finger_info();
    if(finger_info == NULL) return finger_con_height;

    // 1. 动态计算容器高度
    if(finger_info->enroll_count == 1) {
        finger_con_height = 281;      
    } else if(finger_info->enroll_count == 2) {
        finger_con_height = 374;      
    } 
    // else if(finger_info->enroll_count >= 3) { // 3个及以上指纹的高度
    //     finger_con_height = 467; 
    // }
    
    // 2. 创建指纹操作容器
    g_finger_opt_con = create_container(g_opt_con,30,FINGER_CON_BASE_Y,710,finger_con_height, 
                                       lv_color_hex(0xFFFFFF), LV_OPA_100, 16,
                                       lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(g_finger_opt_con, 0, LV_STATE_DEFAULT);
    
    // 基础元素
    lv_obj_t *finger_img = create_image_obj(g_finger_opt_con, "H:finger_large.png", 28, 21);
    lv_obj_t *finger_divider01 = lv_line_create(g_finger_opt_con);
    static const lv_point_t finger_divider01_points[] = {{28, 90}, {687, 90}}; 
    config_divider_line_style(finger_divider01, finger_divider01_points, 2, 0xD4D4D4, 1, LV_OPA_100);
    lv_obj_t *finger_label = create_text_label(g_finger_opt_con, "fingerprint", 
                                              &lv_font_montserrat_36, lv_color_hex(0x27394C), 
                                              90, 23, LV_OPA_100);

    // 3. 指纹记录和分割线（读取当前成员数据）
    if(finger_info->enroll_count >= 1) {
        // 第1个指纹（原有逻辑不变）
        lv_obj_t *finger_divider02 = lv_line_create(g_finger_opt_con);
        static lv_point_t finger_divider02_points[] = {{28, 178}, {687, 178}}; 
        config_divider_line_style(finger_divider02, finger_divider02_points, 2, 0xD4D4D4, 1, LV_OPA_100);

        finger_info->finger_record_cons[0] = create_container(g_finger_opt_con,28,91,659,87, 
                                                  lv_color_hex(0xFFFFFF), LV_OPA_100, 0,
                                                  lv_color_hex(0xFFFFFF), 0, LV_OPA_90);
        lv_obj_set_style_pad_all(finger_info->finger_record_cons[0], 0, LV_STATE_DEFAULT);
        lv_obj_t *label1 = create_text_label(finger_info->finger_record_cons[0], 
                                            finger_info->finger_names[0][0] ? finger_info->finger_names[0] : "Unnamed finger 1", 
                                            &lv_font_montserrat_32, lv_color_hex(0x000000), 
                                            62, 24, LV_OPA_100);
        lv_obj_add_flag(finger_info->finger_record_cons[0], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_opa(finger_info->finger_record_cons[0], LV_OPA_90, LV_STATE_PRESSED);
        lv_obj_set_style_bg_color(finger_info->finger_record_cons[0], lv_color_hex(0xF0F0F0), LV_STATE_PRESSED);

        // 第2个指纹（原有逻辑不变）
        if(finger_info->enroll_count >= 2) {
            lv_obj_t *finger_divider03 = lv_line_create(g_finger_opt_con);
            static lv_point_t finger_divider03_points[] = {{28, 266}, {687, 266}}; 
            config_divider_line_style(finger_divider03, finger_divider03_points, 2, 0xD4D4D4, 1, LV_OPA_100);

            finger_info->finger_record_cons[1] = create_container(g_finger_opt_con,28,179,659,87, 
                                                      lv_color_hex(0xFFFFFF), LV_OPA_100, 0,
                                                      lv_color_hex(0xFFFFFF), 0, LV_OPA_90);
            lv_obj_set_style_pad_all(finger_info->finger_record_cons[1], 0, LV_STATE_DEFAULT);
            lv_obj_t *label2 = create_text_label(finger_info->finger_record_cons[1], 
                                                finger_info->finger_names[1][0] ? finger_info->finger_names[1] : "Unnamed finger 2", 
                                                &lv_font_montserrat_32, lv_color_hex(0x000000), 
                                                62, 24, LV_OPA_100);
            lv_obj_add_flag(finger_info->finger_record_cons[1], LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(finger_info->finger_record_cons[1], LV_OPA_90, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(finger_info->finger_record_cons[1], lv_color_hex(0xF0F0F0), LV_STATE_PRESSED);
        }

        // ========== 第3个指纹的UI逻辑 ==========
    //     if(finger_info->enroll_count >= 3) {
    //         // 第3个指纹的分割线（Y坐标：266+88=354）
    //         lv_obj_t *finger_divider04 = lv_line_create(g_finger_opt_con);
    //         static lv_point_t finger_divider04_points[] = {{28, 354}, {687, 354}}; 
    //         config_divider_line_style(finger_divider04, finger_divider04_points, 2, 0xD4D4D4, 1, LV_OPA_100);

    //         // 第3个指纹的记录容器（Y坐标：267+88=355）
    //         finger_info->finger_record_cons[2] = create_container(g_finger_opt_con,28,267,659,87, 
    //                                                   lv_color_hex(0xFFFFFF), LV_OPA_100, 0,
    //                                                   lv_color_hex(0xFFFFFF), 0, LV_OPA_90);
    //         lv_obj_set_style_pad_all(finger_info->finger_record_cons[2], 0, LV_STATE_DEFAULT);
    //         lv_obj_t *label3 = create_text_label(finger_info->finger_record_cons[2], 
    //                                             finger_info->finger_names[2][0] ? finger_info->finger_names[2] : "Unnamed finger 3", 
    //                                             &lv_font_montserrat_32, lv_color_hex(0x000000), 
    //                                             62, 24, LV_OPA_100);
    //         lv_obj_add_flag(finger_info->finger_record_cons[2], LV_OBJ_FLAG_CLICKABLE);
    //         lv_obj_set_style_opa(finger_info->finger_record_cons[2], LV_OPA_90, LV_STATE_PRESSED);
    //         lv_obj_set_style_bg_color(finger_info->finger_record_cons[2], lv_color_hex(0xF0F0F0), LV_STATE_PRESSED);
    //     }
     }

    // 4. 添加按钮（按当前成员计数控制，修改按钮Y坐标+禁用逻辑）
    uint16_t add_btn_y = 104;    
    uint16_t add_label_y = 116;
    if(finger_info->enroll_count == 1) {
        add_btn_y = 191;         
        add_label_y = 203;
    } else if(finger_info->enroll_count == 2) { // 2个指纹时按钮下移
        add_btn_y = 279;         
        add_label_y = 291;
    } 
    // else if(finger_info->enroll_count >= 3) { // 新增：3个指纹时按钮再下移
    //     add_btn_y = 367;         
    //     add_label_y = 379;
    // }
    
    lv_obj_t *finger_add_img = create_image_obj(g_finger_opt_con, "H:+.png", 307, add_btn_y);
    lv_obj_t *finger_add_label = create_text_label(g_finger_opt_con, "add", 
                                                  &lv_font_montserrat_36, lv_color_hex(0x00BDBD), 
                                                  351, add_label_y, LV_OPA_100);
    
    // 超过3个指纹时变灰不可点击
    if(finger_info->enroll_count >= MAX_FINGER_COUNT) { // 用宏定义更易维护
        lv_obj_set_style_text_color(finger_add_label, lv_color_hex(0x888888), LV_STATE_DEFAULT);
        lv_obj_set_style_opa(finger_add_img, LV_OPA_50, LV_STATE_DEFAULT);
        lv_obj_clear_flag(finger_add_img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_event_cb(finger_add_img, finger_add_btn_click_cb);
        LV_LOG_INFO("Finger button disabled (gray), count: %d", finger_info->enroll_count);
    } else {
        lv_obj_add_flag(finger_add_img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_opa(finger_add_img, LV_OPA_80, LV_STATE_PRESSED);
        lv_obj_add_event_cb(finger_add_img, finger_add_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);
        LV_LOG_INFO("Finger button enabled (normal), count: %d", finger_info->enroll_count);
    }

    // 返回指纹容器高度，用于计算密码容器位置
    return finger_con_height;
}

// ========== 更新密码容器 ==========
static uint16_t update_pwd_opt_container(uint16_t finger_con_height)
{
    uint16_t pwd_con_height = PWD_CON_DEFAULT_HEIGHT; // 默认高度
    
    // 前置安全校验
    if(enroll_scr == NULL || g_opt_con == NULL || !lv_obj_is_valid(g_opt_con)) {
        LV_LOG_WARN("update_pwd_opt_container: invalid parent obj!");
        return pwd_con_height;
    }

    // 删除旧的密码容器（增加有效性校验）
    if(g_pwd_opt_con != NULL && lv_obj_is_valid(g_pwd_opt_con)) {
        lv_obj_del(g_pwd_opt_con);
        g_pwd_opt_con = NULL;
    }

    // 获取当前成员的密码数据
    pwd_enroll_info_t *pwd_info = get_current_pwd_info();
    if(pwd_info == NULL) return pwd_con_height;

    // 1. 计算密码容器Y坐标（指纹容器底部 + 间距）
    uint16_t pwd_con_y = FINGER_CON_BASE_Y + finger_con_height + PWD_CON_BASE_OFFSET;
    // 2. 计算密码容器高度（有密码时增高）
    pwd_con_height = pwd_info->enroll_count >= 1 ? PWD_CON_EXPAND_HEIGHT : PWD_CON_DEFAULT_HEIGHT;
    
    // 3. 创建密码操作容器（增加创建后校验）
    g_pwd_opt_con = create_container(g_opt_con, 30, pwd_con_y, 710, pwd_con_height, 
                                   lv_color_hex(0xFFFFFF), LV_OPA_100, 16,
                                   lv_color_hex(0x1F3150), 0, LV_OPA_90);
    if(g_pwd_opt_con == NULL || !lv_obj_is_valid(g_pwd_opt_con)) {
        LV_LOG_WARN("update_pwd_opt_container: create container failed!");
        return pwd_con_height;
    }
    lv_obj_set_style_pad_all(g_pwd_opt_con, 0, LV_STATE_DEFAULT);

    // 基础元素
    lv_obj_t *pwd_img = create_image_obj(g_pwd_opt_con, "H:pwd_large.png", 28, 21);
    lv_obj_t *pwd_divider = lv_line_create(g_pwd_opt_con);
    static const lv_point_t pwd_divider_points[] = {{28, 90}, {687, 90}}; 
    config_divider_line_style(pwd_divider, pwd_divider_points, 2, 0xD4D4D4, 1, LV_OPA_100);
    lv_obj_t *pwd_label = create_text_label(g_pwd_opt_con, "password", 
                                          &lv_font_montserrat_36, lv_color_hex(0x27394C), 
                                          90, 23, LV_OPA_100);

    // 4. 密码记录项（有密码时显示）
    if(pwd_info->enroll_count >= 1) {
        // 密码分割线
        lv_obj_t *pwd_record_divider = lv_line_create(g_pwd_opt_con);
        static const lv_point_t pwd_record_divider_points[] = {{28, 178}, {687, 178}}; 
        config_divider_line_style(pwd_record_divider, pwd_record_divider_points, 2, 0xD4D4D4, 1, LV_OPA_100);

        // 密码记录容器
        pwd_info->pwd_record_con = create_container(g_pwd_opt_con,28,91,659,PWD_RECORD_HEIGHT, 
                                              lv_color_hex(0xFFFFFF), LV_OPA_100, 0,
                                              lv_color_hex(0xFFFFFF), 0, LV_OPA_90);
        lv_obj_set_style_pad_all(pwd_info->pwd_record_con, 0, LV_STATE_DEFAULT);
        // 密码名称（显示为密文，预留自定义名称接口）
        char pwd_display_name[17] = {0};
        strncpy(pwd_display_name, pwd_info->pwd_name[0] ? pwd_info->pwd_name : "Password 1", sizeof(pwd_display_name)-1);
        lv_obj_t *pwd_record_label = create_text_label(pwd_info->pwd_record_con, 
                                                      pwd_display_name, 
                                                      &lv_font_montserrat_32, lv_color_hex(0x000000), 
                                                      62, 24, LV_OPA_100);
        // 密码记录项点击样式
        lv_obj_add_flag(pwd_info->pwd_record_con, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_opa(pwd_info->pwd_record_con, LV_OPA_90, LV_STATE_PRESSED);
        lv_obj_set_style_bg_color(pwd_info->pwd_record_con, lv_color_hex(0xF0F0F0), LV_STATE_PRESSED);
    }

    // 5. 密码添加按钮（控制坐标）
    uint16_t add_btn_y = 104;    
    uint16_t add_label_y = 116;
    if(pwd_info->enroll_count >= 1) { // 有密码时按钮下移
        add_btn_y = 191;         
        add_label_y = 203;
    }
    
    lv_obj_t *pwd_add_img = create_image_obj(g_pwd_opt_con, "H:+.png", 307, add_btn_y);
    lv_obj_t *pwd_add_label = create_text_label(g_pwd_opt_con, "add", 
                                              &lv_font_montserrat_36, lv_color_hex(0x00BDBD), 
                                              351, add_label_y, LV_OPA_100);
    
    // 密码仅可添加1次，超过则禁用
    if(pwd_info->enroll_count >= MAX_PWD_COUNT) {
        lv_obj_set_style_text_color(pwd_add_label, lv_color_hex(0x888888), LV_STATE_DEFAULT);
        lv_obj_set_style_opa(pwd_add_img, LV_OPA_50, LV_STATE_DEFAULT);
        lv_obj_clear_flag(pwd_add_img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_event_cb(pwd_add_img, pwd_add_btn_click_cb);
        LV_LOG_INFO("Pwd button disabled (gray), count: %d", pwd_info->enroll_count);
    } else {
        lv_obj_add_flag(pwd_add_img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_opa(pwd_add_img, LV_OPA_80, LV_STATE_PRESSED);
        lv_obj_add_event_cb(pwd_add_img, pwd_add_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);
        LV_LOG_INFO("Pwd button enabled (normal), count: %d", pwd_info->enroll_count);
    }

    // 返回密码容器高度，用于计算卡片容器位置
    return pwd_con_height;
}

// ========== 更新卡片容器 ==========
static uint16_t update_card_opt_container(uint16_t pwd_con_height, uint16_t finger_con_height)
{
    uint16_t card_con_height = CARD_CON_DEFAULT_HEIGHT; // 默认高度
    
    // 前置安全校验：仅家庭成员显示卡片容器
    if(enroll_scr == NULL || g_opt_con == NULL || !lv_obj_is_valid(g_opt_con) || 
       g_current_enroll_member.type != MEMBER_TYPE_FAMILY) {
        LV_LOG_WARN("update_card_opt_container: invalid parent obj or not family member!");
        return card_con_height;
    }

    // 删除旧的卡片容器
    if(g_card_opt_con != NULL && lv_obj_is_valid(g_card_opt_con)) {
        lv_obj_del(g_card_opt_con);
        g_card_opt_con = NULL;
    }

    // 获取当前成员的卡片数据
    card_enroll_info_t *card_info = get_current_card_info();
    if(card_info == NULL) return card_con_height;

    //  直接使用传入的高度，不再重复调用更新函数（避免死循环）
    uint16_t pwd_con_total_y = FINGER_CON_BASE_Y + finger_con_height + PWD_CON_BASE_OFFSET;
    uint16_t card_con_y = pwd_con_total_y + pwd_con_height + CARD_CON_BASE_OFFSET;
    // 计算卡片容器高度（有卡片时增高）
    card_con_height = card_info->enroll_count >= 1 ? CARD_CON_EXPAND_HEIGHT : CARD_CON_DEFAULT_HEIGHT;
    
    // 创建卡片操作容器
    g_card_opt_con = create_container(g_opt_con, 30, card_con_y, 710, card_con_height, 
                                   lv_color_hex(0xFFFFFF), LV_OPA_100, 16,
                                   lv_color_hex(0x1F3150), 0, LV_OPA_90);
    if(g_card_opt_con == NULL || !lv_obj_is_valid(g_card_opt_con)) {
        LV_LOG_WARN("update_card_opt_container: create container failed!");
        return card_con_height;
    }
    lv_obj_set_style_pad_all(g_card_opt_con, 0, LV_STATE_DEFAULT);

    // 基础元素（原有逻辑不变）
    lv_obj_t *card_img = create_image_obj(g_card_opt_con, "H:card_large.png", 28, 21);
    lv_obj_t *card_divider = lv_line_create(g_card_opt_con);
    static const lv_point_t card_divider_points[] = {{28, 90}, {687, 90}}; 
    config_divider_line_style(card_divider, card_divider_points, 2, 0xD4D4D4, 1, LV_OPA_100);
    lv_obj_t *card_label = create_text_label(g_card_opt_con, "card", 
                                          &lv_font_montserrat_36, lv_color_hex(0x27394C), 
                                          90, 23, LV_OPA_100);

    // 卡片记录项（原有逻辑不变）
    if(card_info->enroll_count >= 1) {
        lv_obj_t *card_record_divider = lv_line_create(g_card_opt_con);
        static const lv_point_t card_record_divider_points[] = {{28, 178}, {687, 178}}; 
        config_divider_line_style(card_record_divider, card_record_divider_points, 2, 0xD4D4D4, 1, LV_OPA_100);

        card_info->card_record_con = create_container(g_card_opt_con,28,91,659,CARD_RECORD_HEIGHT, 
                                              lv_color_hex(0xFFFFFF), LV_OPA_100, 0,
                                              lv_color_hex(0xFFFFFF), 0, LV_OPA_90);
        lv_obj_set_style_pad_all(card_info->card_record_con, 0, LV_STATE_DEFAULT);
        char card_display_name[17] = {0};
        strncpy(card_display_name, card_info->card_name[0] ? card_info->card_name : "Card 1", sizeof(card_display_name)-1);
        lv_obj_t *card_record_label = create_text_label(card_info->card_record_con, 
                                                      card_display_name, 
                                                      &lv_font_montserrat_32, lv_color_hex(0x000000), 
                                                      62, 24, LV_OPA_100);
        lv_obj_add_flag(card_info->card_record_con, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_opa(card_info->card_record_con, LV_OPA_90, LV_STATE_PRESSED);
        lv_obj_set_style_bg_color(card_info->card_record_con, lv_color_hex(0xF0F0F0), LV_STATE_PRESSED);
    }

    // 卡片添加按钮（原有逻辑不变）
    uint16_t add_btn_y = 104;    
    uint16_t add_label_y = 116;
    if(card_info->enroll_count >= 1) {
        add_btn_y = 191;         
        add_label_y = 203;
    }
    
    lv_obj_t *card_add_img = create_image_obj(g_card_opt_con, "H:+.png", 307, add_btn_y);
    lv_obj_t *card_add_label = create_text_label(g_card_opt_con, "add", 
                                              &lv_font_montserrat_36, lv_color_hex(0x00BDBD), 
                                              351, add_label_y, LV_OPA_100);
    
    if(card_info->enroll_count >= MAX_CARD_COUNT) {
        lv_obj_set_style_text_color(card_add_label, lv_color_hex(0x888888), LV_STATE_DEFAULT);
        lv_obj_set_style_opa(card_add_img, LV_OPA_50, LV_STATE_DEFAULT);
        lv_obj_clear_flag(card_add_img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_event_cb(card_add_img, card_add_btn_click_cb);
        LV_LOG_INFO("Card button disabled (gray), count: %d", card_info->enroll_count);
    } else {
        lv_obj_add_flag(card_add_img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_opa(card_add_img, LV_OPA_80, LV_STATE_PRESSED);
        lv_obj_add_event_cb(card_add_img, card_add_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);
        LV_LOG_INFO("Card button enabled (normal), count: %d", card_info->enroll_count);
    }

    return card_con_height;
}

// ========== 更新人脸容器 ==========
static uint16_t update_face_opt_container(uint16_t card_con_height, uint16_t pwd_con_height, uint16_t finger_con_height)
{
    uint16_t face_con_height = FACE_CON_DEFAULT_HEIGHT; // 默认高度
    
    // 前置安全校验：仅家庭成员显示人脸容器
    if(enroll_scr == NULL || g_opt_con == NULL || !lv_obj_is_valid(g_opt_con) || 
       g_current_enroll_member.type != MEMBER_TYPE_FAMILY) {
        LV_LOG_WARN("update_face_opt_container: invalid parent obj or not family member!");
        return face_con_height;
    }

    // 删除旧的人脸容器
    if(g_face_opt_con != NULL && lv_obj_is_valid(g_face_opt_con)) {
        lv_obj_del(g_face_opt_con);
        g_face_opt_con = NULL;
    }

    // 获取当前成员的人脸数据
    face_enroll_info_t *face_info = get_current_face_info();
    if(face_info == NULL) return face_con_height;

    //  直接使用传入的高度，不再重复调用更新函数（避免死循环）
    uint16_t pwd_con_total_y = FINGER_CON_BASE_Y + finger_con_height + PWD_CON_BASE_OFFSET;
    uint16_t card_con_total_y = pwd_con_total_y + pwd_con_height + CARD_CON_BASE_OFFSET;
    uint16_t face_con_y = card_con_total_y + card_con_height + FACE_CON_BASE_OFFSET;
    // 计算人脸容器高度（有人脸时增高）
    face_con_height = face_info->enroll_count >= 1 ? FACE_CON_EXPAND_HEIGHT : FACE_CON_DEFAULT_HEIGHT;
    
    // 创建人脸操作容器
    g_face_opt_con = create_container(g_opt_con, 30, face_con_y, 710, face_con_height, 
                                   lv_color_hex(0xFFFFFF), LV_OPA_100, 16,
                                   lv_color_hex(0x1F3150), 0, LV_OPA_90);
    if(g_face_opt_con == NULL || !lv_obj_is_valid(g_face_opt_con)) {
        LV_LOG_WARN("update_face_opt_container: create container failed!");
        return face_con_height;
    }
    lv_obj_set_style_pad_all(g_face_opt_con, 0, LV_STATE_DEFAULT);

    // 基础元素（原有逻辑不变）
    lv_obj_t *face_img = create_image_obj(g_face_opt_con, "H:face_large.png", 28, 21);
    lv_obj_t *face_divider = lv_line_create(g_face_opt_con);
    static const lv_point_t face_divider_points[] = {{28, 90}, {687, 90}}; 
    config_divider_line_style(face_divider, face_divider_points, 2, 0xD4D4D4, 1, LV_OPA_100);
    lv_obj_t *face_label = create_text_label(g_face_opt_con, "face", 
                                          &lv_font_montserrat_36, lv_color_hex(0x27394C), 
                                          90, 23, LV_OPA_100);

    // 人脸记录项（原有逻辑不变）
    if(face_info->enroll_count >= 1) {
        lv_obj_t *face_record_divider = lv_line_create(g_face_opt_con);
        static const lv_point_t face_record_divider_points[] = {{28, 178}, {687, 178}}; 
        config_divider_line_style(face_record_divider, face_record_divider_points, 2, 0xD4D4D4, 1, LV_OPA_100);

        face_info->face_record_con = create_container(g_face_opt_con,28,91,659,FACE_RECORD_HEIGHT, 
                                              lv_color_hex(0xFFFFFF), LV_OPA_100, 0,
                                              lv_color_hex(0xFFFFFF), 0, LV_OPA_90);
        lv_obj_set_style_pad_all(face_info->face_record_con, 0, LV_STATE_DEFAULT);
        char face_display_name[17] = {0};
        strncpy(face_display_name, face_info->face_name[0] ? face_info->face_name : "Face 1", sizeof(face_display_name)-1);
        lv_obj_t *face_record_label = create_text_label(face_info->face_record_con, 
                                                      face_display_name, 
                                                      &lv_font_montserrat_32, lv_color_hex(0x000000), 
                                                      62, 24, LV_OPA_100);
        lv_obj_add_flag(face_info->face_record_con, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_opa(face_info->face_record_con, LV_OPA_90, LV_STATE_PRESSED);
        lv_obj_set_style_bg_color(face_info->face_record_con, lv_color_hex(0xF0F0F0), LV_STATE_PRESSED);
    }

    // 人脸添加按钮（原有逻辑不变）
    uint16_t add_btn_y = 104;    
    uint16_t add_label_y = 116;
    if(face_info->enroll_count >= 1) {
        add_btn_y = 191;         
        add_label_y = 203;
    }
    
    lv_obj_t *face_add_img = create_image_obj(g_face_opt_con, "H:+.png", 307, add_btn_y);
    lv_obj_t *face_add_label = create_text_label(g_face_opt_con, "add", 
                                              &lv_font_montserrat_36, lv_color_hex(0x00BDBD), 
                                              351, add_label_y, LV_OPA_100);
    
    if(face_info->enroll_count >= MAX_FACE_COUNT) {
        lv_obj_set_style_text_color(face_add_label, lv_color_hex(0x888888), LV_STATE_DEFAULT);
        lv_obj_set_style_opa(face_add_img, LV_OPA_50, LV_STATE_DEFAULT);
        lv_obj_clear_flag(face_add_img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_event_cb(face_add_img, face_add_btn_click_cb);
        LV_LOG_INFO("Face button disabled (gray), count: %d", face_info->enroll_count);
    } else {
        lv_obj_add_flag(face_add_img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_opa(face_add_img, LV_OPA_80, LV_STATE_PRESSED);
        lv_obj_add_event_cb(face_add_img, face_add_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);
        LV_LOG_INFO("Face button enabled (normal), count: %d", face_info->enroll_count);
    }

    return face_con_height;
}


// ========== 指纹录入完成回调 ==========
void finger_enroll_complete(const char *finger_name)
{
    finger_enroll_info_t *finger_info = get_current_finger_info();
    if(finger_info == NULL) {
        LV_LOG_WARN("finger_enroll_complete: no current member!");
        return;
    }
    // 边界校验
    if(finger_info->enroll_count >= MAX_FINGER_COUNT) {
        LV_LOG_WARN("finger_enroll_complete: max count reached!");
        return;
    }

    // 保存指纹名称到当前成员
    if(finger_name != NULL && strlen(finger_name) > 0) {
        strncpy(finger_info->finger_names[finger_info->enroll_count], finger_name, sizeof(finger_info->finger_names[0])-1);
    } else {
        snprintf(finger_info->finger_names[finger_info->enroll_count], sizeof(finger_info->finger_names[0]), 
                "Unnamed finger %d", finger_info->enroll_count + 1);
    }
    
    // 增加计数并更新UI
    finger_info->enroll_count++;
    LV_LOG_INFO("Finger enroll complete, count: %d, name: %s", finger_info->enroll_count, finger_info->finger_names[finger_info->enroll_count-1]);
    
    // 逐级更新容器：指纹→密码→卡片→人脸
    uint16_t finger_height = update_finger_opt_container();
    uint16_t pwd_height = update_pwd_opt_container(finger_height);
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY) {
        uint16_t card_height = update_card_opt_container(pwd_height, finger_height);
        update_face_opt_container(card_height, pwd_height, finger_height);
    }

    // 同步更新家庭成员列表中的指纹计数
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY && g_current_enroll_member.idx < MAX_FAMILY_MEMBER_COUNT) {
        // 家庭成员：更新family数组 + 调用family的更新函数
        family_member_list[g_current_enroll_member.idx].finger_count = finger_info->enroll_count;
        update_member_count_ui(g_current_enroll_member.idx);
    } else if(g_current_enroll_member.type == MEMBER_TYPE_OTHER && g_current_enroll_member.idx < MAX_OTHER_MEMBER_COUNT) {
        // 其他成员：更新other数组 + 调用other的更新函数
        other_member_list[g_current_enroll_member.idx].finger_count = finger_info->enroll_count;
        update_other_member_count_ui(g_current_enroll_member.idx);
    }
}

// ========== 密码录入完成回调 ==========
void pwd_enroll_complete(const char *pwd_name)
{
    // 1. 前置安全校验
    if(!g_member_info_inited || g_opt_con == NULL || !lv_obj_is_valid(g_opt_con)) {
        LV_LOG_WARN("pwd_enroll_complete: invalid init state!");
        return;
    }

    pwd_enroll_info_t *pwd_info = get_current_pwd_info();
    if(pwd_info == NULL) {
        LV_LOG_WARN("pwd_enroll_complete: no current member!");
        return;
    }
    // 边界校验（仅支持1个密码）
    if(pwd_info->enroll_count >= MAX_PWD_COUNT) {
        LV_LOG_WARN("pwd_enroll_complete: max count reached!");
        return;
    }

    // 2. 保存密码名称（增加长度校验）
    if(pwd_name != NULL && strlen(pwd_name) > 0) {
        strncpy(pwd_info->pwd_name, pwd_name, sizeof(pwd_info->pwd_name)-1);
    } else {
        snprintf(pwd_info->pwd_name, sizeof(pwd_info->pwd_name), "pwd01");
    }
    
    // 3. 增加计数并更新UI（分步更新，避免冲突）
    pwd_info->enroll_count++;
    LV_LOG_INFO("Pwd enroll complete, count: %d, name: %s", pwd_info->enroll_count, pwd_info->pwd_name);
    
    // 逐级更新容器：指纹→密码→卡片→人脸
    uint16_t finger_height = update_finger_opt_container();
    uint16_t pwd_height = update_pwd_opt_container(finger_height);
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY) {
        uint16_t card_height = update_card_opt_container(pwd_height, finger_height);
        update_face_opt_container(card_height, pwd_height, finger_height);
    }

    // 同步更新家庭成员列表中的密码计数
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY && g_current_enroll_member.idx < MAX_FAMILY_MEMBER_COUNT) {
        family_member_list[g_current_enroll_member.idx].pwd_count = pwd_info->enroll_count;
        update_member_count_ui(g_current_enroll_member.idx);
    } else if(g_current_enroll_member.type == MEMBER_TYPE_OTHER && g_current_enroll_member.idx < MAX_OTHER_MEMBER_COUNT) {
        other_member_list[g_current_enroll_member.idx].pwd_count = pwd_info->enroll_count;
        update_other_member_count_ui(g_current_enroll_member.idx);
    }
}

// ========== 卡片录入完成回调 ==========
void card_enroll_complete(const char *card_name)
{
    if(!g_member_info_inited || g_opt_con == NULL || !lv_obj_is_valid(g_opt_con) ||
       g_current_enroll_member.type != MEMBER_TYPE_FAMILY) {
        LV_LOG_WARN("card_enroll_complete: invalid init state or not family member!");
        return;
    }

    card_enroll_info_t *card_info = get_current_card_info();
    if(card_info == NULL) {
        LV_LOG_WARN("card_enroll_complete: no current member!");
        return;
    }
    if(card_info->enroll_count >= MAX_CARD_COUNT) {
        LV_LOG_WARN("card_enroll_complete: max count reached!");
        return;
    }

    if(card_name != NULL && strlen(card_name) > 0) {
        strncpy(card_info->card_name, card_name, sizeof(card_info->card_name)-1);
    } else {
        snprintf(card_info->card_name, sizeof(card_info->card_name), "card01");
    }
    
    card_info->enroll_count++;
    LV_LOG_INFO("Card enroll complete, count: %d, name: %s", card_info->enroll_count, card_info->card_name);
    
    //  先计算一次高度，再传入更新函数（避免重复调用）
    uint16_t finger_height = update_finger_opt_container();
    uint16_t pwd_height = update_pwd_opt_container(finger_height);
    uint16_t card_height = update_card_opt_container(pwd_height, finger_height); // 传入已计算的高度
    update_face_opt_container(card_height, pwd_height, finger_height); // 传入所有已计算的高度

    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY && g_current_enroll_member.idx < MAX_FAMILY_MEMBER_COUNT) {
        family_member_list[g_current_enroll_member.idx].card_count = card_info->enroll_count;
    }
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY && g_current_enroll_member.idx < MAX_FAMILY_MEMBER_COUNT) {
        update_member_count_ui(g_current_enroll_member.idx);
    }
}
// ========== 人脸录入完成回调 ==========
void face_enroll_complete(const char *face_name)
{
    if(!g_member_info_inited || g_opt_con == NULL || !lv_obj_is_valid(g_opt_con) ||
       g_current_enroll_member.type != MEMBER_TYPE_FAMILY) {
        LV_LOG_WARN("face_enroll_complete: invalid init state or not family member!");
        return;
    }

    face_enroll_info_t *face_info = get_current_face_info();
    if(face_info == NULL) {
        LV_LOG_WARN("face_enroll_complete: no current member!");
        return;
    }
    if(face_info->enroll_count >= MAX_FACE_COUNT) {
        LV_LOG_WARN("face_enroll_complete: max count reached!");
        return;
    }

    if(face_name != NULL && strlen(face_name) > 0) {
        strncpy(face_info->face_name, face_name, sizeof(face_info->face_name)-1);
    } else {
        snprintf(face_info->face_name, sizeof(face_info->face_name), "face01");
    }
    
    face_info->enroll_count++;
    LV_LOG_INFO("Face enroll complete, count: %d, name: %s", face_info->enroll_count, face_info->face_name);
    
    // 先计算一次高度，再传入更新函数
    uint16_t finger_height = update_finger_opt_container();
    uint16_t pwd_height = update_pwd_opt_container(finger_height);
    uint16_t card_height = update_card_opt_container(pwd_height, finger_height); // 传入已计算的高度
    update_face_opt_container(card_height, pwd_height, finger_height); // 传入所有已计算的高度

    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY && g_current_enroll_member.idx < MAX_FAMILY_MEMBER_COUNT) {
        family_member_list[g_current_enroll_member.idx].face_count = face_info->enroll_count;
    }
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY && g_current_enroll_member.idx < MAX_FAMILY_MEMBER_COUNT) {
        update_member_count_ui(g_current_enroll_member.idx);
    }
}


// ==========  ui_enroll_create 中的容器初始化逻辑 ==========
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
    g_current_enroll_member.type = member_info->type;
    g_current_enroll_member.idx = member_info->idx;
    strncpy(g_current_enroll_member.name, member_info->name, sizeof(g_current_enroll_member.name)-1);
    g_current_enroll_member.avatar_color = member_info->avatar_color;

    // if(enroll_scr == NULL) {
    //     enroll_scr = lv_obj_create(NULL);
    // } else {
    //     lv_obj_clean(enroll_scr);
    // }
    if(is_lv_obj_valid(enroll_scr)) {
        lv_obj_del(enroll_scr);
        enroll_scr = NULL;
    }
    enroll_scr = lv_obj_create(NULL);
    
    lv_style_reset(&enroll_grad_style);
    lv_style_set_bg_color(&enroll_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&enroll_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&enroll_grad_style, LV_GRAD_DIR_VER);
    lv_style_set_bg_main_stop(&enroll_grad_style, 0);
    lv_style_set_bg_grad_stop(&enroll_grad_style, 255);
    lv_obj_add_style(enroll_scr, &enroll_grad_style, LV_STATE_DEFAULT);

    char member_label_text[16] = {0};
    snprintf(member_label_text, sizeof(member_label_text), "name: %s", g_current_enroll_member.name);
    
    lv_obj_t *avatar = create_container(enroll_scr,334,242,131,131, g_current_enroll_member.avatar_color, LV_OPA_100, 100,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_t *name_label = create_text_label(enroll_scr, member_label_text, &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 0, 398, LV_OPA_100);
    lv_obj_align(name_label, LV_ALIGN_TOP_MID, 0, 398);
    
    char title_text[32] = {0};
    strcpy(title_text, g_current_enroll_member.type == MEMBER_TYPE_FAMILY ? "family member" : "other member");
    create_text_label(enroll_scr, title_text, &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 0, 115, LV_OPA_100);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 115);

    g_opt_con = create_container(enroll_scr,18,550,763,730, lv_color_hex(0xE0EDFF), LV_OPA_100, 31,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(g_opt_con, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(g_opt_con, LV_OBJ_FLAG_SCROLLABLE);

    //  容器初始化逻辑
    uint16_t finger_con_height = update_finger_opt_container();
    uint16_t pwd_con_height = update_pwd_opt_container(finger_con_height);
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY) {
        uint16_t card_con_height = update_card_opt_container(pwd_con_height, finger_con_height); // 传入已计算的高度
        update_face_opt_container(card_con_height, pwd_con_height, finger_con_height); // 传入所有已计算的高度
    }

    lv_obj_t *back_btn = create_image_obj(enroll_scr, "H:back.png", 52, 123);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn, back_btn_click_cb, LV_EVENT_CLICKED, parent_scr);
    
    update_status_bar_parent(enroll_scr);
    lv_scr_load(enroll_scr);
    
    LV_LOG_INFO("Enroll screen created, opt_con: %p, finger height: %d, pwd height: %d", 
                g_opt_con, finger_con_height, pwd_con_height);
}

// ========== 获取当前指纹信息 ==========
static finger_enroll_info_t *get_current_finger_info(void)
{
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY) {
        if(g_current_enroll_member.idx >= MAX_FAMILY_MEMBER_COUNT) {
            LV_LOG_WARN("Invalid family member idx: %d", g_current_enroll_member.idx);
            return NULL;
        }
        return &g_family_finger_info[g_current_enroll_member.idx];
    } else { // MEMBER_TYPE_OTHER
        if(g_current_enroll_member.idx >= MAX_OTHER_MEMBER_COUNT) {
            LV_LOG_WARN("Invalid other member idx: %d", g_current_enroll_member.idx);
            return NULL;
        }
        return &g_other_finger_info[g_current_enroll_member.idx];
    }
}

// ========== 获取当前密码信息 ==========
pwd_enroll_info_t *get_current_pwd_info(void)
{
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY) {
        if(g_current_enroll_member.idx >= MAX_FAMILY_MEMBER_COUNT) {
            LV_LOG_WARN("Invalid family member idx: %d", g_current_enroll_member.idx);
            return NULL;
        }
        return &g_family_pwd_info[g_current_enroll_member.idx];
    } else { // MEMBER_TYPE_OTHER
        if(g_current_enroll_member.idx >= MAX_OTHER_MEMBER_COUNT) {
            LV_LOG_WARN("Invalid other member idx: %d", g_current_enroll_member.idx);
            return NULL;
        }
        return &g_other_pwd_info[g_current_enroll_member.idx];
    }
}

// ========== 获取当前卡片信息 ==========
card_enroll_info_t *get_current_card_info(void)
{
    // 仅家庭成员有卡片信息
    if(g_current_enroll_member.type != MEMBER_TYPE_FAMILY) {
        LV_LOG_WARN("Card info only for family member!");
        return NULL;
    }
    if(g_current_enroll_member.idx >= MAX_FAMILY_MEMBER_COUNT) {
        LV_LOG_WARN("Invalid family member idx: %d", g_current_enroll_member.idx);
        return NULL;
    }
    return &g_family_card_info[g_current_enroll_member.idx];
}

// ========== 获取当前人脸信息 ==========
face_enroll_info_t *get_current_face_info(void)
{
    // 仅家庭成员有人脸信息
    if(g_current_enroll_member.type != MEMBER_TYPE_FAMILY) {
        LV_LOG_WARN("Face info only for family member!");
        return NULL;
    }
    if(g_current_enroll_member.idx >= MAX_FAMILY_MEMBER_COUNT) {
        LV_LOG_WARN("Invalid family member idx: %d", g_current_enroll_member.idx);
        return NULL;
    }
    return &g_family_face_info[g_current_enroll_member.idx];
}

// ========== 重置指定成员的指纹状态 ==========
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
    
    // 清空数据
    info->enroll_count = 0;
    memset(info->finger_names, 0, sizeof(info->finger_names));
    // 删除UI容器
    for(int i=0; i<MAX_FINGER_COUNT; i++) {
        if(info->finger_record_cons[i] != NULL && lv_obj_is_valid(info->finger_record_cons[i])) {
            lv_obj_del(info->finger_record_cons[i]);
            info->finger_record_cons[i] = NULL;
        }
    }
    LV_LOG_INFO("Reset finger state for %s member %d", 
                type == MEMBER_TYPE_FAMILY ? "family" : "other", idx);
}

// ========== 重置指定成员的密码状态 ==========
static void reset_member_pwd_state(uint8_t idx, member_type_e type)
{
    pwd_enroll_info_t *info = NULL;
    if(type == MEMBER_TYPE_FAMILY) {
        if(idx >= MAX_FAMILY_MEMBER_COUNT) return;
        info = &g_family_pwd_info[idx];
    } else {
        if(idx >= MAX_OTHER_MEMBER_COUNT) return;
        info = &g_other_pwd_info[idx];
    }
    
    // 清空数据
    info->enroll_count = 0;
    memset(info->pwd_name, 0, sizeof(info->pwd_name));
    // 删除UI容器
    if(info->pwd_record_con != NULL && lv_obj_is_valid(info->pwd_record_con)) {
        lv_obj_del(info->pwd_record_con);
        info->pwd_record_con = NULL;
    }
    LV_LOG_INFO("Reset pwd state for %s member %d", 
                type == MEMBER_TYPE_FAMILY ? "family" : "other", idx);
}

// ========== 重置指定成员的卡片状态 ==========
static void reset_member_card_state(uint8_t idx, member_type_e type)
{
    // 仅家庭成员有卡片信息
    if(type != MEMBER_TYPE_FAMILY) return;
    if(idx >= MAX_FAMILY_MEMBER_COUNT) return;
    
    card_enroll_info_t *info = &g_family_card_info[idx];
    // 清空数据
    info->enroll_count = 0;
    memset(info->card_name, 0, sizeof(info->card_name));
    // 删除UI容器
    if(info->card_record_con != NULL && lv_obj_is_valid(info->card_record_con)) {
        lv_obj_del(info->card_record_con);
        info->card_record_con = NULL;
    }
    LV_LOG_INFO("Reset card state for family member %d", idx);
}

// ========== 重置指定成员的人脸状态 ==========
static void reset_member_face_state(uint8_t idx, member_type_e type)
{
    // 仅家庭成员有人脸信息
    if(type != MEMBER_TYPE_FAMILY) return;
    if(idx >= MAX_FAMILY_MEMBER_COUNT) return;
    
    face_enroll_info_t *info = &g_family_face_info[idx];
    // 清空数据
    info->enroll_count = 0;
    memset(info->face_name, 0, sizeof(info->face_name));
    // 删除UI容器
    if(info->face_record_con != NULL && lv_obj_is_valid(info->face_record_con)) {
        lv_obj_del(info->face_record_con);
        info->face_record_con = NULL;
    }
    LV_LOG_INFO("Reset face state for family member %d", idx);
}
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
    snprintf(member_label_text, sizeof(member_label_text), "name: %s", g_current_enroll_member.name);
    
    lv_obj_t *avatar = create_container(enroll_scr,0,0,90,90, g_current_enroll_member.avatar_color, LV_OPA_100, 100,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_align(avatar, LV_ALIGN_TOP_MID, 0, 150);
    lv_obj_t *name_label = create_text_label(enroll_scr, member_label_text, &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 0, 398, LV_OPA_100);
    lv_obj_align(name_label, LV_ALIGN_TOP_MID, 0, 244);
    
    // 标题：家庭成员/其他成员
    char title_text[32] = {0};
    strcpy(title_text, g_current_enroll_member.type == MEMBER_TYPE_FAMILY ? "family member" : "other member");
    create_text_label(enroll_scr, title_text, &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 83, 80, LV_OPA_100);

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
    lv_obj_t *back_btn = create_container_circle(enroll_scr, 52, 90, 30,
    true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
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
    create_image_obj(g_finger_opt_con, "H:finger_large.png", 28, 21);
    lv_obj_t *finger_divider01 = lv_line_create(g_finger_opt_con);
    static lv_point_t divider_top_points[] = {{38, LINE_TOP_Y}, {881, LINE_TOP_Y}};
    config_divider_line_style(finger_divider01, divider_top_points, 2, 0xD4D4D4, 1, LV_OPA_100);
    create_text_label(g_finger_opt_con, "fingerprint",&lv_font_montserrat_36, lv_color_hex(0x27394C), 90, 23, LV_OPA_100);

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

        char name_buf[32] = {0};
        if(finger_info->finger_names[i][0] == '\0')
            snprintf(name_buf, sizeof(name_buf), "Unnamed finger %d", i+1);
        else
            strncpy(name_buf, finger_info->finger_names[i], sizeof(name_buf)-1);

        create_text_label(finger_info->finger_record_cons[i], name_buf,&lv_font_montserrat_32, lv_color_hex(0x000000), 62, 24, LV_OPA_100);

        lv_obj_add_flag(finger_info->finger_record_cons[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_opa(finger_info->finger_record_cons[i], LV_OPA_90, LV_STATE_PRESSED);
        lv_obj_set_style_bg_color(finger_info->finger_record_cons[i], lv_color_hex(0xF0F0F0), LV_STATE_PRESSED);
        lv_obj_add_event_cb(finger_info->finger_record_cons[i], edit_record_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);
    }

    // 添加按钮
    uint16_t add_btn_y = ADD_BTN_INIT_Y + (finger_info->enroll_count * 88);
    uint16_t add_lbl_y = add_btn_y - 6;

    lv_obj_t * finger_add_img = create_container(g_finger_opt_con,307,add_btn_y,34,34,lv_color_hex(0x00BDBD), LV_OPA_100, 100,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(finger_add_img, 0, LV_STATE_DEFAULT);
    // +号图标
    lv_obj_t *divider_line1 = lv_line_create(finger_add_img);
    static const lv_point_t divider_points1[] = {{7, 17}, {27, 17}}; 
    config_divider_line_style(divider_line1, divider_points1, 2, 0xFFFFFF, 3, LV_OPA_100);
    lv_obj_t *divider_line2 = lv_line_create(finger_add_img);
    static const lv_point_t divider_points2[] = {{17, 7}, {17, 27}}; 
    config_divider_line_style(divider_line2, divider_points2, 2, 0xFFFFFF, 3, LV_OPA_100);
    
    lv_obj_t *finger_add_label = create_text_label(g_finger_opt_con, "add",&lv_font_montserrat_36, lv_color_hex(0x00BDBD), 351, add_lbl_y, LV_OPA_100);

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
    create_image_obj(g_pwd_opt_con, "H:pwd_large.png", 28, 21);
    lv_obj_t *pwd_divider01 = lv_line_create(g_pwd_opt_con);
    static lv_point_t divider_top_points[] = {{38, LINE_TOP_Y}, {881, LINE_TOP_Y}};
    config_divider_line_style(pwd_divider01, divider_top_points, 2, 0xD4D4D4, 1, LV_OPA_100);
    create_text_label(g_pwd_opt_con, "password",&lv_font_montserrat_36, lv_color_hex(0x27394C),90, 23, LV_OPA_100);

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
            snprintf(name_buf, sizeof(name_buf), "Password %d", i+1);
        else
            strncpy(name_buf, pwd_info->pwd_names[i], sizeof(name_buf)-1);

        create_text_label(pwd_info->pwd_record_cons[i], name_buf,&lv_font_montserrat_32, lv_color_hex(0x000000), 62, 24, LV_OPA_100);

        lv_obj_add_flag(pwd_info->pwd_record_cons[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_opa(pwd_info->pwd_record_cons[i], LV_OPA_90, LV_STATE_PRESSED);
        lv_obj_set_style_bg_color(pwd_info->pwd_record_cons[i], lv_color_hex(0xF0F0F0), LV_STATE_PRESSED);
        lv_obj_add_event_cb(pwd_info->pwd_record_cons[i], edit_record_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);
    }

    // 添加按钮
    uint16_t add_btn_y = ADD_BTN_INIT_Y + (pwd_info->enroll_count * 88);
    uint16_t add_lbl_y = add_btn_y -6;

    lv_obj_t * pwd_add_img = create_container(g_pwd_opt_con,307,add_btn_y,34,34,lv_color_hex(0x00BDBD), LV_OPA_100, 100,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(pwd_add_img, 0, LV_STATE_DEFAULT);
    lv_obj_t *divider_line1 = lv_line_create(pwd_add_img);
    static const lv_point_t divider_points1[] = {{7, 17}, {27, 17}}; 
    config_divider_line_style(divider_line1, divider_points1, 2, 0xFFFFFF, 3, LV_OPA_100);
    lv_obj_t *divider_line2 = lv_line_create(pwd_add_img);
    static const lv_point_t divider_points2[] = {{17, 7}, {17, 27}}; 
    config_divider_line_style(divider_line2, divider_points2, 2, 0xFFFFFF, 3, LV_OPA_100);

    lv_obj_t *pwd_add_label = create_text_label(g_pwd_opt_con, "add", &lv_font_montserrat_36, lv_color_hex(0x00BDBD), 351, add_lbl_y, LV_OPA_100);

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
    create_image_obj(g_card_opt_con, "H:card_large.png", 28, 21);
    lv_obj_t *card_divider01 = lv_line_create(g_card_opt_con);
    static lv_point_t divider_top_points[] = {{38, LINE_TOP_Y}, {881, LINE_TOP_Y}};
    config_divider_line_style(card_divider01, divider_top_points, 2, 0xD4D4D4, 1, LV_OPA_100);
    create_text_label(g_card_opt_con, "card",&lv_font_montserrat_36, lv_color_hex(0x27394C), 95, 35, LV_OPA_100);

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
            snprintf(name_buf, sizeof(name_buf), "Card %d", i+1);
        else
            strncpy(name_buf, card_info->card_names[i], sizeof(name_buf)-1);

        create_text_label(card_info->card_record_cons[i], name_buf, &lv_font_montserrat_32, lv_color_hex(0x000000), 62, 24, LV_OPA_100);

        lv_obj_add_flag(card_info->card_record_cons[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_opa(card_info->card_record_cons[i], LV_OPA_90, LV_STATE_PRESSED);
        lv_obj_set_style_bg_color(card_info->card_record_cons[i], lv_color_hex(0xF0F0F0), LV_STATE_PRESSED);
        lv_obj_add_event_cb(card_info->card_record_cons[i], edit_record_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);
    }

    // 添加按钮
    uint16_t add_btn_y = ADD_BTN_INIT_Y + (card_info->enroll_count * 88);
    uint16_t add_lbl_y = add_btn_y - 6;

    lv_obj_t *card_add_img   = create_container(g_card_opt_con,307,add_btn_y,34,34,lv_color_hex(0x00BDBD), LV_OPA_100, 100,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(card_add_img, 0, LV_STATE_DEFAULT);
    lv_obj_t *divider_line1 = lv_line_create(card_add_img);
    static const lv_point_t divider_points1[] = {{7, 17}, {27, 17}}; 
    config_divider_line_style(divider_line1, divider_points1, 2, 0xFFFFFF, 3, LV_OPA_100);
    lv_obj_t *divider_line2 = lv_line_create(card_add_img);
    static const lv_point_t divider_points2[] = {{17, 7}, {17, 27}}; 
    config_divider_line_style(divider_line2, divider_points2, 2, 0xFFFFFF, 3, LV_OPA_100);
    
    lv_obj_t *card_add_label = create_text_label(g_card_opt_con, "add",&lv_font_montserrat_36, lv_color_hex(0x00BDBD), 351, add_lbl_y, LV_OPA_100);

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
    create_image_obj(g_face_opt_con, "H:face_large.png", 28, 21);
    lv_obj_t *face_divider01 = lv_line_create(g_face_opt_con);
    static lv_point_t divider_top_points[] = {{38, LINE_TOP_Y}, {881, LINE_TOP_Y}};
    config_divider_line_style(face_divider01, divider_top_points, 2, 0xD4D4D4, 1, LV_OPA_100);
    create_text_label(g_face_opt_con, "face",&lv_font_montserrat_36, lv_color_hex(0x27394C),95, 35, LV_OPA_100);

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
            snprintf(name_buf, sizeof(name_buf), "Face %d", i+1);
        else
            strncpy(name_buf, face_info->face_names[i], sizeof(name_buf)-1);

        create_text_label(face_info->face_record_cons[i], name_buf, &lv_font_montserrat_32, lv_color_hex(0x000000), 62, 24, LV_OPA_100);

        lv_obj_add_flag(face_info->face_record_cons[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_opa(face_info->face_record_cons[i], LV_OPA_90, LV_STATE_PRESSED);
        lv_obj_set_style_bg_color(face_info->face_record_cons[i], lv_color_hex(0xF0F0F0), LV_STATE_PRESSED);
        lv_obj_add_event_cb(face_info->face_record_cons[i], edit_record_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);
    }

    // 添加按钮
    uint16_t add_btn_y = ADD_BTN_INIT_Y + (face_info->enroll_count * 88);
    uint16_t add_lbl_y = add_btn_y - 6;

    lv_obj_t *face_add_img = create_container(g_face_opt_con,307,add_btn_y,34,34,lv_color_hex(0x00BDBD), LV_OPA_100, 100,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(face_add_img, 0, LV_STATE_DEFAULT);
    lv_obj_t *divider_line1 = lv_line_create(face_add_img);
    static const lv_point_t divider_points1[] = {{7, 17}, {27, 17}}; 
    config_divider_line_style(divider_line1, divider_points1, 2, 0xFFFFFF, 3, LV_OPA_100);
    lv_obj_t *divider_line2 = lv_line_create(face_add_img);
    static const lv_point_t divider_points2[] = {{17, 7}, {17, 27}}; 
    config_divider_line_style(divider_line2, divider_points2, 2, 0xFFFFFF, 3, LV_OPA_100);
    
    lv_obj_t *face_add_label = create_text_label(g_face_opt_con, "add",&lv_font_montserrat_36, lv_color_hex(0x00BDBD), 351, add_lbl_y, LV_OPA_100);
    
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
    finger_enroll_info_t *finger_info = get_current_finger_info();
    if(finger_info == NULL || finger_info->enroll_count >= MAX_FINGER_COUNT) {
        LV_LOG_WARN("finger_enroll_complete: invalid param or max count!");
        return;
    }

    // 保存指纹名称
    if(finger_name != NULL && strlen(finger_name) > 0) {
        strncpy(finger_info->finger_names[finger_info->enroll_count], finger_name, sizeof(finger_info->finger_names[0])-1);
    } else {
        snprintf(finger_info->finger_names[finger_info->enroll_count], sizeof(finger_info->finger_names[0]), 
                "Unnamed finger %d", finger_info->enroll_count + 1);
    }
    
    finger_info->enroll_count++;
    LV_LOG_INFO("Finger enroll complete, count: %d", finger_info->enroll_count);
    
    // 刷新UI
    uint16_t finger_height = update_finger_opt_container();
    uint16_t pwd_height = update_pwd_opt_container(finger_height);
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY) {
        uint16_t card_height = update_card_opt_container(pwd_height, finger_height);
        update_face_opt_container(card_height, pwd_height, finger_height);
    }

    // 同步计数到成员列表
    if(g_current_enroll_member.type == MEMBER_TYPE_FAMILY && g_current_enroll_member.idx < MAX_FAMILY_MEMBER_COUNT) {
        family_member_list[g_current_enroll_member.idx].finger_count = finger_info->enroll_count;
        update_member_count_ui(g_current_enroll_member.idx);
    } else if(g_current_enroll_member.type == MEMBER_TYPE_OTHER && g_current_enroll_member.idx < MAX_OTHER_MEMBER_COUNT) {
        other_member_list[g_current_enroll_member.idx].finger_count = finger_info->enroll_count;
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
    else snprintf(info->pwd_names[index], sizeof(info->pwd_names[0]), "pwd%d", index+1);
    
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