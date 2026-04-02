/**
 * @file lv_sys_settings.h
 * 
 */

#ifndef LV_SYS_SETTINGS_H
#define LV_SYS_SETTINGS_H

#ifdef __cplusplus
extern "C" {
#endif
#include "com.h"


/**
 * @brief 系统设置按钮点击回调函数
 * 
 * @param e 事件对象指针
 */
void sys_settings_btn_click_cb(lv_event_t *e);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_SYS_SETTINGS_H*/
