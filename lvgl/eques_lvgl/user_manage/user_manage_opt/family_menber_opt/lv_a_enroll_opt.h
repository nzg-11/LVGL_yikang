/**
 * @file lv_enroll_opt.h
 * @brief 成员生物特征录入管理模块头文件
 *        包含指纹/密码/卡片/人脸的录入、编辑、UI渲染等功能声明
 * @note 依赖 LVGL 库、公共组件头文件
 */

#ifndef LV_ENROLL_OPT_H
#define LV_ENROLL_OPT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *  头文件包含
 *********************/
#include "com.h"
#include "lv_family_menber.h"
#include "lv_other_member.h"
#include "lv_edit_record.h"

/*********************
 *  宏定义配置
 *********************/
// 生物特征最大录入数量限制
#define MAX_FINGER_COUNT    5   // 指纹最大支持数量
#define MAX_PWD_COUNT       3   // 密码最大支持数量
#define MAX_CARD_COUNT      3   // 卡片最大支持数量
#define MAX_FACE_COUNT      3   // 人脸最大支持数量

/*********************
 *  枚举类型定义
 *********************/
/**
 * @brief 成员类型枚举
 */
typedef enum {
    MEMBER_TYPE_FAMILY,    // 家庭成员
    MEMBER_TYPE_OTHER      // 其他成员
} member_type_e;

/*********************
 *  结构体类型定义
 *********************/
/**
 * @brief 通用成员基础信息结构体
 */
typedef struct {
    member_type_e type;                // 成员类型
    char name[16];                     // 成员名称
    lv_color_t avatar_color;           // 成员头像颜色
    uint8_t finger_count;              // 已录入指纹数量
    uint8_t pwd_count;                 // 已录入密码数量
    uint8_t card_count;                // 已录入卡片数量(仅家庭成员)
    uint8_t face_count;                 // 已录入人脸数量(仅家庭成员)
    uint8_t idx;                       // 成员索引值
} common_member_info_t;

/**
 * @brief 指纹录入信息结构体
 */
typedef struct {
    char finger_names[MAX_FINGER_COUNT][17];    // 指纹名称数组
    lv_obj_t *finger_record_cons[MAX_FINGER_COUNT]; // 指纹UI容器数组
    uint8_t enroll_count;                       // 已录入总数量
} finger_enroll_info_t;

/**
 * @brief 密码录入信息结构体
 */
typedef struct {
    char pwd_names[MAX_PWD_COUNT][17];          // 密码名称数组
    lv_obj_t *pwd_record_cons[MAX_PWD_COUNT];   // 密码UI容器数组
    uint8_t enroll_count;                       // 已录入总数量
} pwd_enroll_info_t;

/**
 * @brief 卡片录入信息结构体
 */
typedef struct {
    char card_names[MAX_CARD_COUNT][17];        // 卡片名称数组
    lv_obj_t *card_record_cons[MAX_CARD_COUNT]; // 卡片UI容器数组
    uint8_t enroll_count;                       // 已录入总数量
} card_enroll_info_t;

/**
 * @brief 人脸录入信息结构体
 */
typedef struct {
    char face_names[MAX_FACE_COUNT][17];        // 人脸名称数组
    lv_obj_t *face_record_cons[MAX_FACE_COUNT]; // 人脸UI容器数组
    uint8_t enroll_count;                       // 已录入总数量
} face_enroll_info_t;

/*********************
 *  全局变量声明
 *********************/
// 家庭成员生物特征全局数据
extern finger_enroll_info_t g_family_finger_info[MAX_FAMILY_MEMBER_COUNT];
extern pwd_enroll_info_t    g_family_pwd_info[MAX_FAMILY_MEMBER_COUNT];
extern card_enroll_info_t   g_family_card_info[MAX_FAMILY_MEMBER_COUNT];
extern face_enroll_info_t   g_family_face_info[MAX_FAMILY_MEMBER_COUNT];

// 其他成员生物特征全局数据
extern finger_enroll_info_t g_other_finger_info[MAX_OTHER_MEMBER_COUNT];
extern pwd_enroll_info_t    g_other_pwd_info[MAX_OTHER_MEMBER_COUNT];

// 当前录入成员全局信息
extern common_member_info_t g_current_enroll_member;
// 指纹数量显示标签(全局UI对象)
extern lv_obj_t *g_finger_count_label;

/*********************
 *  函数声明
 *********************/
/**
 * @brief 生物特征录入完成回调函数
 * @param name: 录入项名称
 */
void finger_enroll_complete(const char *finger_name);
void pwd_enroll_complete(const char *pwd_name);
void card_enroll_complete(const char *card_name);
void face_enroll_complete(const char *face_name);

/**
 * @brief 编辑操作相关函数
 */
void edit_update_name(edit_type_e type, const char *new_name, uint8_t index);
void edit_delete_item(edit_type_e type, uint8_t index);
void clear_member_all_biometrics(uint8_t member_idx, member_type_e type);

/**
 * @brief 获取当前成员生物特征信息
 */
pwd_enroll_info_t    *get_current_pwd_info(void);
card_enroll_info_t   *get_current_card_info(void);
finger_enroll_info_t *get_current_finger_info(void);
face_enroll_info_t   *get_current_face_info(void);
common_member_info_t *get_current_enroll_member(void);

/**
 * @brief 指纹计数更新与UI刷新
 */
void refresh_finger_count_label(void);
void update_finger_count(uint8_t add_num);

/**
 * @brief 录入界面UI操作
 */
void re_render_enroll_ui(lv_obj_t *parent_scr);
void enroll_btn_click_cb(lv_event_t *e);
void ui_enroll_create(common_member_info_t *member_info, lv_obj_t *parent_scr);
void destroy_enroll(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_ENROLL_OPT_H */