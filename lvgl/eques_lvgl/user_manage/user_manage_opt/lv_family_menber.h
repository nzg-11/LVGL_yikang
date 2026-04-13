/**
 * @file lv_family_menber.h
 * 
 */

#ifndef LV_FAMILY_MENBER_H
#define LV_FAMILY_MENBER_H

#ifdef __cplusplus
extern "C" {
#endif
#include "com.h"

#define MAX_FAMILY_MEMBER_COUNT 8  // 最多6个成员

extern uint8_t g_selected_member_idx;
// 成员信息结构体
typedef struct {
    char name[16];          
    lv_color_t avatar_color;
    bool is_valid;          
    // 可扩展：指纹/密码/卡片/人脸计数
    uint8_t finger_count;
    uint8_t pwd_count;
    uint8_t card_count;
    uint8_t face_count;
} family_member_info_t;

extern family_member_info_t family_member_list[MAX_FAMILY_MEMBER_COUNT];

void family_menber_btn_click_cb(lv_event_t *e);
void update_member_count_ui(uint8_t member_idx);
void destroy_family_member(void);
void ui_family_menber_create(lv_obj_t *user_manage_scr);
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_FAMILY_MENBER_H*/
