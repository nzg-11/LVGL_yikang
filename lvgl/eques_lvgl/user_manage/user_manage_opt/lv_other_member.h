/**
 * @file lv_other_member.h
 * 其他成员管理模块头文件
 */

#ifndef LV_OTHER_MEMBER_H
#define LV_OTHER_MEMBER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "com.h"

// 最大其他成员数量（和家庭成员保持一致）
#define MAX_OTHER_MEMBER_COUNT 8  

// 全局选中的其他成员索引
extern uint8_t g_selected_other_member_idx;


typedef struct {
    char name[16];          
    lv_color_t avatar_color;
    bool is_valid;          
    // 指纹+密码计数（卡片/人脸已删除）
    uint8_t finger_count;
    uint8_t pwd_count;
} other_member_info_t;

// 其他成员列表数组
extern other_member_info_t other_member_list[MAX_OTHER_MEMBER_COUNT];

// 其他成员按钮点击回调（界面入口）
void other_member_btn_click_cb(lv_event_t *e);
void update_other_member_count_ui(uint8_t member_idx);
void ui_other_member_create(lv_obj_t *user_manage_scr);
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_OTHER_MEMBER_H */