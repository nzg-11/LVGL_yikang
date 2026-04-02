/**
 * @file com.h
 * 
 */

#ifndef COM_H
#define COM_H

#ifdef __cplusplus
extern "C" {
#endif
#include "../lvgl.h"

/*********************
 *      INCLUDES
 *********************/
// 分割线
void config_divider_line_style(lv_obj_t *line_obj, const lv_point_t *points, uint32_t point_cnt, uint32_t line_color, uint8_t line_width, lv_opa_t line_opa);

// 图片
lv_obj_t *create_image_obj(lv_obj_t *parent, const void *img_src, int32_t x, int32_t y);

// 文本
lv_obj_t *create_text_label(lv_obj_t *parent, const char *text, const lv_font_t *font, lv_color_t text_color, int x, int y, lv_opa_t opa);

// 容器
lv_obj_t *create_container(lv_obj_t *parent, int x, int y, int w, int h, lv_color_t bg_color, lv_opa_t bg_opa, int radius, lv_color_t border_color, int border_width, lv_opa_t border_opa);
lv_obj_t *create_container_circle(lv_obj_t *parent, int x, int y, int size, bool is_circle, lv_color_t bg_color, lv_color_t border_color, int border_width, lv_opa_t border_opa);

// 按钮
lv_obj_t *create_menu_btn_nt(lv_obj_t *parent, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t bg_color);
lv_obj_t *create_menu_btn(lv_obj_t *parent, const char *text, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t bg_color, uint32_t border_color, uint8_t border_width, uint32_t text_color, const lv_font_t *font, uint8_t radius, lv_opa_t opa);

//渐变容器
lv_obj_t *create_custom_gradient_container(
    lv_obj_t *parent,
    int32_t x,
    int32_t y,
    int32_t w,
    int32_t h,
    uint8_t radius,
    uint32_t main_color,
    uint32_t grad_color,
    lv_grad_dir_t grad_dir,
    uint8_t main_stop,
    uint8_t grad_stop,
    lv_opa_t bg_opa
);

void back_btn_click_cb(lv_event_t *e);
extern lv_obj_t *status_bar;
// 全局状态栏相关函数
void update_status_bar_parent(lv_obj_t *new_scr);
void destroy_status_bar(void);
bool is_lv_obj_valid(lv_obj_t *obj);

#define false 0
#define true 1
/**********************
 * GLOBAL PROTOTYPES
 **********************/
#define LV_EQUES_VER 0

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*UI_VERSION_H*/