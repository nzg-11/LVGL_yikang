#ifndef MONITOR_MODE_H
#define MONITOR_MODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

// 监控模式状态枚举
typedef enum {
    MONITOR_MODE_OFF,    // 关闭
    MONITOR_MODE_ON,     // 开启
    MONITOR_MODE_MAX
} monitor_mode_state_t;

// 监控时间段选项枚举
typedef enum {
    MONITOR_SLOT_FULL_DAY,    // 全天时间段
    MONITOR_SLOT_WORK_HOUR,   // 工作时间
    MONITOR_SLOT_CUSTOM,      // 自定义时间段
    MONITOR_SLOT_MAX
} monitor_slot_t;

/**
 * @brief 初始化监控模式子模块
 * @param display_label 主页面显示监控状态的标签（可选）
 */
void monitor_mode_init(lv_obj_t *display_label);

/**
 * @brief 创建监控模式设置子页面
 * @param homepage_scr 主页面对象（用于返回按钮跳转）
 */
void ui_monitor_mode_settings_create(lv_obj_t *homepage_scr);

/**
 * @brief 获取当前监控模式状态
 */
monitor_mode_state_t monitor_mode_get_state(void);

/**
 * @brief 设置监控模式状态
 * @param state 目标状态
 */
void monitor_mode_set_state(monitor_mode_state_t state);

// 获取当前时间段类型
monitor_slot_t monitor_mode_get_slot_type(void);
// 设置当前时间段类型（并更新显示）
void monitor_mode_set_slot_type(monitor_slot_t type);
// 获取当前时间段字符串（用于显示）
const char *monitor_mode_get_slot_str(void);
// 恢复出厂设置 - 重置监控模式所有状态到默认值
void monitor_mode_reset_to_default(void);

#ifdef __cplusplus
}
#endif

#endif // MONITOR_MODE_H