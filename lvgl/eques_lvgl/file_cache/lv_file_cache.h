/**
 * @file lv_file_cache.h
 * 
 */

#ifndef LV_FILE_CACHE_H
#define LV_FILE_CACHE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "com.h"



void ui_file_cache_create(lv_obj_t *homepage_scr);
void file_cache_btn_click_cb(lv_event_t *e);

// 子页面创建
void ui_fc_cloud_create(lv_obj_t *file_cache_scr);
void ui_fc_video_create(lv_obj_t *file_cache_scr);
void ui_fc_pic_create(lv_obj_t *file_cache_scr);

// 子页面回调
void cloud_cache_btn_cb(lv_event_t *e);
void video_cache_btn_cb(lv_event_t *e);
void pic_cache_btn_cb(lv_event_t *e);
void video_play_pause_cb(lv_event_t *e);
void video_slider_changed_cb(lv_event_t *e);
void delete_current_video_cb(lv_event_t *e);

void delete_cancel_cb(lv_event_t *e);
void pic_delete_confirm_cb(lv_event_t *e);
void delete_current_pic_cb(lv_event_t *e);
void pic_list_item_click_cb(lv_event_t *e);
// 通用返回回调

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_FILE_CACHE_H*/
