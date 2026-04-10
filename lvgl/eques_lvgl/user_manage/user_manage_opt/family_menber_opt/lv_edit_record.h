/**
 * @file lv_edit_record.h
 * 
 */

#ifndef LV_EDIT_RECORD_H
#define LV_EDIT_RECORD_H

#ifdef __cplusplus
extern "C" {
#endif
#include "com.h"

typedef enum {
    EDIT_TYPE_PWD,      // 密码
    EDIT_TYPE_FINGER,   // 指纹
    EDIT_TYPE_CARD,     // 卡片
    EDIT_TYPE_FACE      // 人脸
} edit_type_e;

// 类型、当前名称、父界面，彻底解耦
void ui_edit_record_create(edit_type_e type, const char *cur_name, uint8_t index, lv_obj_t *parent_scr);
void edit_record_btn_click_cb(lv_event_t *e);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_EDIT_RECORD_H*/
