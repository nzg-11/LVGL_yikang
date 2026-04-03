#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

// 通知模式枚举
typedef enum {
    NOTIFY_ALL,           // 接受全部消息
    NOTIFY_ALERT_ONLY,    // 仅接受报警/门铃
    NOTIFY_MODE_MAX
} notify_mode_t;

// 内部状态变量
extern bool g_dnd_state;                  // 免打扰开关状态
extern const char *g_dnd_time_slot;       // 免打扰时间段

/**
 * @brief 初始化消息通知子模块
 */
void notification_init(void);

/**
 * @brief 创建消息通知设置子页面
 * @param homepage_scr 主页面对象（用于返回按钮跳转）
 */
void ui_notification_settings_create(lv_obj_t *homepage_scr);

/**
 * @brief 获取当前通知模式
 * @return notify_mode_t 枚举值
 */
notify_mode_t notification_get_mode(void);

/**
 * @brief 设置通知模式
 * @param mode 目标模式
 */
void notification_set_mode(notify_mode_t mode);

/**
 * @brief 获取免打扰开关状态
 * @return true: 开启, false: 关闭
 */
bool notification_get_dnd_state(void);

/**
 * @brief 设置免打扰开关状态
 * @param state 目标状态
 */
void notification_set_dnd_state(bool state);

/**
 * @brief 获取当前免打扰时间段
 * @return 时间段字符串，如 "22:00-6:00"
 */
const char *notification_get_dnd_time_slot(void);

/**
 * @brief 设置免打扰时间段
 * @param slot 时间段字符串
 */
void notification_set_dnd_time_slot(const char *slot);

#ifdef __cplusplus
}
#endif

#endif // NOTIFICATION_H