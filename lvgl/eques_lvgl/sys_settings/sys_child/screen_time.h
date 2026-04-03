#ifndef SCREEN_TIME_H
#define SCREEN_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

// 亮屏时间选项枚举（原始代码中抽离）
typedef enum {
    SCREEN_OFF_10S,
    SCREEN_OFF_15S,
    SCREEN_OFF_30S,
    SCREEN_OFF_1MIN,
    SCREEN_OFF_5MIN,
    SCREEN_OFF_10MIN,
    SCREEN_OFF_MAX
} screen_off_time_t;

/**
 * @brief 亮屏时间子模块初始化
 * @param display_label 主页面用于显示亮屏时间的标签对象
 */
void screen_time_init(lv_obj_t *display_label);

/**
 * @brief 创建亮屏时间设置子页面
 * @param homepage_scr 主页面对象（用于返回按钮回调）
 */
void ui_screen_time_settings_create(lv_obj_t *homepage_scr);
void screen_time_reset_to_default(void);

/**
 * @brief 获取当前选中的亮屏时间字符串
 * @return 如"15s"、"1min"等
 */
const char *screen_time_get_current_str(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SCREEN_TIME_H */