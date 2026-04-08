/**
 * @file lv_enroll_opt.h
 * 
 */

#ifndef LV_ENROLL_OPT_H
#define LV_ENROLL_OPT_H

#ifdef __cplusplus
extern "C" {
#endif
#include "com.h"
#include "lv_family_menber.h"
#include "lv_other_member.h"
#include "lv_edit_record.h"

#define MAX_FINGER_COUNT        2       // 指纹最大数量
#define MAX_PWD_COUNT           1       // 密码最大数量
#define MAX_CARD_COUNT          1       // 卡片最大数量
#define MAX_FACE_COUNT          1       // 人脸最大数量
// 成员类型枚举（放在公共头文件如 com.h，或 lv_a_enroll_opt.h 中）
typedef enum {
    MEMBER_TYPE_FAMILY,   // 家庭成员
    MEMBER_TYPE_OTHER     // 其他成员
} member_type_e;

// 通用成员信息结构体
typedef struct {
    member_type_e type;          // 成员类型
    char name[16];               // 名称（通用）
    lv_color_t avatar_color;     // 头像颜色（通用）
    uint8_t finger_count;        // 指纹录入次数（预留）
    uint8_t pwd_count;           // 密码录入次数（预留）
    uint8_t card_count;          // 卡片录入次数（仅家庭成员）
    uint8_t face_count;          // 人脸录入次数（仅家庭成员）
    uint8_t idx;                 // 成员索引
} common_member_info_t;

// ========== 指纹信息结构体 ==========
typedef struct {
    char finger_names[MAX_FINGER_COUNT][17]; // 该成员的指纹名称
    lv_obj_t *finger_record_cons[MAX_FINGER_COUNT]; // 该成员的指纹记录容器
    uint8_t enroll_count; // 该成员已录入指纹数量
} finger_enroll_info_t;

// ========== 密码信息结构体 ==========
typedef struct {
    char pwd_name[17];                    // 密码名称
    lv_obj_t *pwd_record_con;             // 密码记录容器
    uint8_t enroll_count;                 // 密码录入数量（0/1）
} pwd_enroll_info_t;

// ========== 卡片信息结构体（新增） ==========
typedef struct {
    char card_name[17];                    // 卡片名称
    lv_obj_t *card_record_con;             // 卡片记录容器
    uint8_t enroll_count;                  // 卡片录入数量（0/1）
} card_enroll_info_t;

// ========== 人脸信息结构体（新增） ==========
typedef struct {
    char face_name[17];                    // 人脸名称
    lv_obj_t *face_record_con;             // 人脸记录容器
    uint8_t enroll_count;                  // 人脸录入数量（0/1）
} face_enroll_info_t;

void finger_enroll_complete(const char *finger_name);
void pwd_enroll_complete(const char *pwd_name);
void card_enroll_complete(const char *card_name); //  卡片录入完成回调
void face_enroll_complete(const char *face_name); //  人脸录入完成回调
void edit_update_name(edit_type_e type, const char *new_name);
void edit_delete_item(edit_type_e type);
pwd_enroll_info_t *get_current_pwd_info(void);
card_enroll_info_t *get_current_card_info(void);
finger_enroll_info_t *get_current_finger_info(void);
face_enroll_info_t *get_current_face_info(void);

extern finger_enroll_info_t g_family_finger_info[MAX_FAMILY_MEMBER_COUNT];
extern finger_enroll_info_t g_other_finger_info[MAX_OTHER_MEMBER_COUNT];

extern pwd_enroll_info_t g_family_pwd_info[MAX_FAMILY_MEMBER_COUNT];
extern pwd_enroll_info_t g_other_pwd_info[MAX_OTHER_MEMBER_COUNT];

extern card_enroll_info_t g_family_card_info[MAX_FAMILY_MEMBER_COUNT];
extern face_enroll_info_t g_family_face_info[MAX_FAMILY_MEMBER_COUNT];

// 全局成员信息（保存当前录入的成员数据）
extern common_member_info_t g_current_enroll_member;
// 全局保存指纹计数标签（用于直接更新文本）
extern lv_obj_t *g_finger_count_label;
void refresh_finger_count_label(void);
void update_finger_count(uint8_t add_num);

void re_render_enroll_ui(lv_obj_t *parent_scr);

void enroll_btn_click_cb(lv_event_t *e);
void ui_enroll_create(common_member_info_t *member_info, lv_obj_t *parent_scr);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_ENROLL_OPT_H*/