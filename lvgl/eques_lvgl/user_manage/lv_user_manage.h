/**
 * @file lv_user_manage.h
 * 
 */

#ifndef LV_USER_MANAGE_H
#define LV_USER_MANAGE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "com.h"


/**
 * @brief 用户管理按钮点击回调函数
 * 
 * @param e 事件对象指针
 */
void user_manage_btn_click_cb(lv_event_t *e);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_USER_MANAGE_H*/
