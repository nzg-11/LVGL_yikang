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
lv_obj_t *create_img_label(lv_obj_t *parent, const char *text, const lv_font_t *font, int x, int y, lv_opa_t opa);
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

extern lv_obj_t *status_bar;
// 全局状态栏相关函数
void update_status_bar_parent(lv_obj_t *new_scr);
void destroy_status_bar(void);
bool is_lv_obj_valid(lv_obj_t *obj);


LV_FONT_DECLARE(eques_bold_24);
LV_FONT_DECLARE(eques_regular_24);
LV_FONT_DECLARE(eques_regular_28);
LV_FONT_DECLARE(eques_regular_32);
LV_FONT_DECLARE(eques_regular_48);
LV_FONT_DECLARE(eques_regular_60);
LV_FONT_DECLARE(eques_bold_28);
LV_FONT_DECLARE(eques_regular_36);
LV_FONT_DECLARE(eques_bold_36);

LV_FONT_DECLARE(menu_icon);
#define ICON_MSG_CENTER "\uf4ad"               // 消息中心图标
#define ICON_MONITOR_VIDEO "\uf0a0"            // 监控视频图标
#define ICON_USER_MANAGE "\uf2bd"              // 用户管理图标
#define ICON_DEV_INFO "\uf15c"                 // 设备信息图标
#define ICON_FILE_CACHE "\uf07c"               // 文件缓存图标
#define ICON_SYS_SETTING "\uf013"              // 系统设置图标



LV_FONT_DECLARE(fontawesome_icon_26);
#define ICON_FINGERPRINT_S "\uf577"            // 小指纹图标
#define ICON_PASSWORD_S "\uf03a"               // 小密码图标
#define ICON_CARD_S "\uf2bb"                   // 小卡片图标
#define ICON_FACE_S "\uf58c"                   // 小面容图标


LV_FONT_DECLARE(fontawesome_icon_32);
#define ICON_WIFI "\uf1eB"                     // WiFi图标
#define ICON_LOCK "\uf09C"                     // 关锁图标
#define ICON_UNLOCK "\uf023"                   // 解锁图标
#define ICON_FAMILY_MEMBER "\uf505"            // 家庭成员图标
#define ICON_OTHER_MEMBER "\uf406"             // 其他成员图标
#define ICON_CHEVORN_RIGHT "\uf054"            // 箭头向右图标
#define ICON_CHEVORN_LEFT "\uf053"             // 箭头向左图标
#define ICON_CHECK "\uf058"                    // 勾选图标
#define ICON_CALENDAR "\uf073"                 // 日历图标
#define ICON_TRASH "\uf2ed"                    // 垃圾桶图标
#define ICON_PICTURE "\uf302"                  // 图片图标
#define ICON_TAG "\uf02c"                      // 标签图标
#define ICON_SERVER "\uf233"                   // 服务器图标
#define ICON_PASSPORT "\uf5ab"                 // 护照图标

LV_FONT_DECLARE(fontawesome_icon_40);
#define ICON_VOLUME_S "\uf026"                 // 小音量
#define ICON_VOLUME_L "\uf028"                 // 大音量
#define ICON_FINGERPRINT_L "\uf577"            // 大指纹图标
#define ICON_PASSWORD_L "\uf03a"               // 大密码图标
#define ICON_CARD_L "\uf2bb"                   // 大卡片图标
#define ICON_FACE_L "\uf58c"                   // 大面容图标

LV_FONT_DECLARE(fontawesome_icon_60);
#define ICON_VOLUME_OFF "\uf028"               // 音量关图标
#define ICON_VOLUME_ON "\uf6a9"                // 音量开图标
#define ICON_DELETE "\uf55a"                   // 删除图标

LV_FONT_DECLARE( fontawesome_icon_150);
#define ICON_RAIN "\uf740"                    // 大雨图标


LV_FONT_DECLARE(iconfont_icon_20);
#define ICON_PLAY  "\uf04b"                    // 播放图标
#define ICON_PAUSE "\uf04c"                    // 暂停图标

LV_FONT_DECLARE(iconfont_icon_32);
#define ICON_VIDEO "\ue622"                    // 视频图标

LV_FONT_DECLARE(iconfont_icon_60);
#define ICON_RECORDING "\ue7be"                // 录制图标
#define ICON_VIDEO_50 "\uea72"                 // 视频图标
#define ICON_SCREENSHOT "\ue7bc"               // 截屏图标
#define ICON_MICROPHONE_OFF "\ue653"           // 麦克风关图标
#define ICON_MICROPHONE_ON "\ue7bd"            // 麦克风开图标
#define ICON_UNLOCK_50 "\ue635"                // 解锁图标
#define ICON_LOCK_50 "\ue634"                  // 锁定图标

LV_FONT_DECLARE(iconfont_icon_90);
#define ICON_CLOUD "\ue62c"                    // 云存储图标
#define ICON_FINGERPRINT_BASE "\uea74"         // 指纹基图标
#define ICON_CARD "\uea73"                     // 刷卡图标
#define ICON_REPEAT "\ue677"                   // 重复图标
#define ICON_CHECK_L "\ue605"                    // 大对勾图标
#define ICON_ERROR "\ue63c"                    // 大X图标

#define false 0
#define true 1
/**********************
 * GLOBAL PROTOTYPES
 **********************/
#define LV_EQUES_VER 0

#define LV_DEMO_EQUES 1

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*UI_VERSION_H*/