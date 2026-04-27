#ifndef LV_SYS_SETTINGS_H
#define LV_SYS_SETTINGS_H

#ifdef __cplusplus
extern "C" {
#endif
#include "com.h"

#include "lvgl.h"

// 先确认com.h中这些函数的声明，以下为对齐后的版本
// lv_obj_t *create_text_label(lv_obj_t *parent, const char *name, const lv_font_t *font, 
//                            lv_color_t color, int32_t x, int32_t y, lv_opa_t opa);
// lv_obj_t *create_container(lv_obj_t *parent, int32_t x, int32_t y, int32_t w, int32_t h, 
//                           lv_color_t bg_color, lv_opa_t bg_opa, int32_t radius,
//                           lv_color_t border_color, int32_t border_width, lv_opa_t border_opa);
// lv_obj_t *create_image_obj(lv_obj_t *parent, const void *img_src, int32_t x, int32_t y);

void update_status_bar_parent(lv_obj_t *parent);
void sys_settings_destroy(void);
void sys_settings_destroy(void);
// 全局样式声明
extern lv_style_t sys_settings_grad_style;

// 系统设置主页面创建函数
void ui_sys_settings_create(lv_obj_t *homepage_scr);

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
