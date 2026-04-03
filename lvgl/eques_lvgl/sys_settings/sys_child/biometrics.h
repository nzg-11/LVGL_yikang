#ifndef BIOMETRICS_H
#define BIOMETRICS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

// 生物识别选项枚举
typedef enum {
    BIOMETRIC_FINGERPRINT,
    BIOMETRIC_PASSWORD,
    BIOMETRIC_NFC,
    BIOMETRIC_FACE,
    BIOMETRIC_TEMP_PWD,
    BIOMETRIC_MAX
} biometric_type_t;

// 临时密码长度
#define TEMP_PWD_LENGTH 6

/**
 * @brief 初始化生物识别子模块
 */
void biometrics_init(void);

/**
 * @brief 创建生物识别设置子页面
 * @param homepage_scr 主页面对象（用于返回按钮跳转）
 */
void ui_biometrics_settings_create(lv_obj_t *homepage_scr);

/**
 * @brief 获取某个生物识别选项的开启状态
 * @param type 生物识别类型
 * @return true: 开启, false: 关闭
 */
bool biometric_get_state(biometric_type_t type);

/**
 * @brief 设置某个生物识别选项的开启状态
 * @param type 生物识别类型
 * @param state 目标状态
 */
void biometric_set_state(biometric_type_t type, bool state);

/**
 * @brief 获取当前临时密码
 * @return 6位数字密码字符串
 */
const char *biometric_get_temp_pwd(void);

/**
 * @brief 设置临时密码
 * @param pwd 6位数字密码字符串
 */
void biometric_set_temp_pwd(const char *pwd);

/**
 * @brief 获取当前适用时间段
 * @return 时间段字符串，如 "8:00-8:05"
 */
const char *biometric_get_time_slot(void);

/**
 * @brief 设置适用时间段
 * @param slot 时间段字符串
 */
void biometric_set_time_slot(const char *slot);
void biometrics_reset_to_default(void);

#ifdef __cplusplus
}
#endif

#endif // BIOMETRICS_H