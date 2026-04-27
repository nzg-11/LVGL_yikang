#include "lv_sys_settings.h"
#include "sys_child/screen_time.h"
#include "sys_child/monitor_mode.h"
#include "sys_child/biometrics.h"
#include "sys_child/notification.h"

#ifdef LV_DEMO_EQUES
#if LV_EQUES_VER    //竖屏

#else    //横屏

// 恢复出厂设置弹窗相关静态变量
static lv_obj_t *reset_popup = NULL;
// 系统设置主屏幕全局变量
lv_obj_t *sys_settings_scr = NULL; 
lv_style_t sys_settings_grad_style;
static bool sys_settings_style_inited = false;
lv_obj_t *g_time_view_label = NULL;
// 三个滑块对象指针
static lv_obj_t *light_slider = NULL;    // 屏幕亮度滑块
static lv_obj_t *ring_slider = NULL;     // 铃声滑块
static lv_obj_t *notify_slider = NULL;   // 通知音量滑块

// 滑块默认值（50%）
#define SLIDER_DEFAULT_VALUE 50
static void reset_popup_close_cb(lv_event_t *e);
static void reset_confirm_cb(lv_event_t *e);
static void factory_reset_confirm_action(void);
static void factory_reset_click_cb(lv_event_t *e);
// 滑块值改变回调函数声明
static void light_slider_value_changed_cb(lv_event_t *e);
static void ring_slider_value_changed_cb(lv_event_t *e);
static void notify_slider_value_changed_cb(lv_event_t *e);
void sys_settings_back_btn_click_cb(lv_event_t *e);

// 外部控制接口函数声明
void set_light_brightness(uint8_t brightness);  // 设置屏幕亮度（0-100）
uint8_t get_light_brightness(void);             // 获取当前亮度

void set_ring_volume(uint8_t volume);           // 设置铃声音量（0-100）
uint8_t get_ring_volume(void);                  // 获取当前铃声音量

void set_notify_volume(uint8_t volume);         // 设置通知音量（0-100）
uint8_t get_notify_volume(void);                // 获取当前通知音量

void slider_reset_to_default(void);             // 滑块恢复默认值（50%）

// ==================== 滑块回调函数实现 ====================
// 屏幕亮度滑块值改变回调
static void light_slider_value_changed_cb(lv_event_t *e) {
    if (e == NULL || light_slider == NULL) return;
    lv_slider_get_value(light_slider);
}

// 铃声滑块值改变回调
static void ring_slider_value_changed_cb(lv_event_t *e) {
    if (e == NULL || ring_slider == NULL) return;
    lv_slider_get_value(ring_slider);

}

// 通知音量滑块值改变回调
static void notify_slider_value_changed_cb(lv_event_t *e) {
    if (e == NULL || notify_slider == NULL) return;
    lv_slider_get_value(notify_slider);
}

// ==================== 外部控制接口实现 ====================
void set_light_brightness(uint8_t brightness) {
    if (light_slider != NULL) {
        brightness = (brightness > 100) ? 100 : brightness;
        lv_slider_set_value(light_slider, brightness, LV_ANIM_ON);
        // 触发回调执行硬件操作
        lv_event_send(light_slider, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

uint8_t get_light_brightness(void) {
    if (light_slider != NULL) {
        return (uint8_t)lv_slider_get_value(light_slider);
    }
    return 0;
}

void set_ring_volume(uint8_t volume) {
    if (ring_slider != NULL) {
        volume = (volume > 100) ? 100 : volume;
        lv_slider_set_value(ring_slider, volume, LV_ANIM_ON);
        lv_event_send(ring_slider, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

uint8_t get_ring_volume(void) {
    if (ring_slider != NULL) {
        return (uint8_t)lv_slider_get_value(ring_slider);
    }
    return 0;
}

void set_notify_volume(uint8_t volume) {
    if (notify_slider != NULL) {
        volume = (volume > 100) ? 100 : volume;
        lv_slider_set_value(notify_slider, volume, LV_ANIM_ON);
        lv_event_send(notify_slider, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

uint8_t get_notify_volume(void) {
    if (notify_slider != NULL) {
        return (uint8_t)lv_slider_get_value(notify_slider);
    }
    return 0;
}

// 滑块恢复默认值（50%）
void slider_reset_to_default(void) {
    set_light_brightness(SLIDER_DEFAULT_VALUE);
    set_ring_volume(SLIDER_DEFAULT_VALUE);
    set_notify_volume(SLIDER_DEFAULT_VALUE);
    LV_LOG_USER("All sliders reset to default value: %d%%", SLIDER_DEFAULT_VALUE);
}



static void monitor_mode_click_cb(lv_event_t *e) {
    if(e == NULL) return;
    lv_obj_t *homepage_scr = (lv_obj_t *)lv_event_get_user_data(e);
    ui_monitor_mode_settings_create(homepage_scr);
}

void light_time_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(parent_scr == NULL) return;

    ui_screen_time_settings_create(parent_scr);
}

static void biometrics_click_cb(lv_event_t *e) {
    if(e == NULL) return;
    lv_obj_t *homepage_scr = (lv_obj_t *)lv_event_get_user_data(e);
    ui_biometrics_settings_create(homepage_scr);
}

static void notification_click_cb(lv_event_t *e) {
    if(e == NULL) return;
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);
    ui_notification_settings_create(parent_scr);
}

// ==================== 样式初始化 ====================
static void init_sys_settings_styles(void) {
    if(!sys_settings_style_inited) {
        lv_style_init(&sys_settings_grad_style);
        sys_settings_style_inited = true;
    }
}

// ==================== 主页面创建 ====================
void ui_sys_settings_create(lv_obj_t *homepage_scr) {
    if(sys_settings_scr != NULL) {
        lv_obj_del(sys_settings_scr);
        sys_settings_scr = NULL;
    }
    init_sys_settings_styles();
    
    //notification_init();
   // biometrics_init();
    //screen_time_init(g_time_view_label);
    if(homepage_scr == NULL) {
        LV_LOG_WARN("ui_sys_settings_create: homepage_scr is NULL!");
        return;
    }
    sys_settings_scr = lv_obj_create(NULL);  
    
    lv_style_reset(&sys_settings_grad_style);
    lv_style_set_bg_color(&sys_settings_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&sys_settings_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&sys_settings_grad_style, LV_GRAD_DIR_VER);
    lv_style_set_bg_main_stop(&sys_settings_grad_style, 0);
    lv_style_set_bg_grad_stop(&sys_settings_grad_style, 255);
    lv_obj_add_style(sys_settings_scr, &sys_settings_grad_style, LV_STATE_DEFAULT);
    
    // 添加标签
    create_text_label(sys_settings_scr, "系统设置", &eques_bold_36, lv_color_hex(0xFFFFFF), 83, 80, LV_OPA_100);
    create_text_label(sys_settings_scr, "亮度", &eques_regular_24, lv_color_hex(0xFFFFFF), 65, 150, LV_OPA_70);
    create_text_label(sys_settings_scr, "铃声和通知", &eques_regular_24, lv_color_hex(0xFFFFFF), 65, 339, LV_OPA_70);

    // 屏幕亮度设置容器
    lv_obj_t *sys_settings_con = create_container(
        sys_settings_scr, 48, 185, 928, 125, 
        lv_color_hex(0x192A46), LV_OPA_100, 6, 
        lv_color_hex(0x2E4B7D), 0, LV_OPA_0
    );
    if(sys_settings_con) {
        lv_obj_add_flag(sys_settings_con, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(sys_settings_con, LV_OPA_70, LV_STATE_PRESSED);
        lv_obj_add_event_cb(sys_settings_con, light_time_click_cb, LV_EVENT_CLICKED, sys_settings_scr);
    }
    lv_obj_t *circel = create_container_circle(sys_settings_scr, 81, 210, 35 ,true, lv_color_hex(0xFFFFFF), lv_color_hex(0xffffff), 3, LV_OPA_100);
    lv_obj_set_style_bg_opa(circel, LV_OPA_0, LV_STATE_DEFAULT);
    create_text_label(sys_settings_scr, "亮屏时间", &eques_regular_32, lv_color_hex(0xFFFFFF), 73, 257, LV_OPA_100);
    g_time_view_label = create_text_label(sys_settings_scr, screen_time_get_str(), &eques_regular_24, lv_color_hex(0xFFFFFF), 860, 264, LV_OPA_70);
    create_text_label(sys_settings_scr, ICON_CHEVORN_RIGHT, &my_custom_icon, lv_color_hex(0xC4C4C4), 935, 260, LV_OPA_100);
    screen_time_init(g_time_view_label);

    // ==================== 创建屏幕亮度滑块 ====================
    light_slider = lv_slider_create(sys_settings_scr);
    lv_obj_set_size(light_slider, 750, 8);
    lv_obj_set_pos(light_slider, 137, 223);
    lv_obj_set_style_bg_color(light_slider, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(light_slider, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_bg_color(light_slider, lv_color_hex(0x00FFFF), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(light_slider, lv_color_hex(0xFFFFFF), LV_PART_KNOB);
    // 设置默认值为50%
    lv_slider_set_value(light_slider, SLIDER_DEFAULT_VALUE, LV_ANIM_OFF);
    // 添加值改变回调
    lv_obj_add_event_cb(light_slider, light_slider_value_changed_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // ring notify 容器
    lv_obj_t *sys_settings_con1 = create_container(
        sys_settings_scr, 48, 374, 928, 250, 
        lv_color_hex(0x192A46), LV_OPA_100, 6, 
        lv_color_hex(0x2E4B7D), 0, LV_OPA_0
    );
    if(sys_settings_con1) {
        lv_obj_add_flag(sys_settings_con1, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(sys_settings_con1, LV_OPA_70, LV_STATE_PRESSED);
    }
    create_text_label(sys_settings_scr, "铃声", &eques_regular_32, lv_color_hex(0xFFFFFF), 73, 396, LV_OPA_100);
    create_text_label(sys_settings_scr, ICON_VOLUME_S, &my_custom_icon_50, lv_color_hex(0xFFFFFF), 90, 452, LV_OPA_100);
    create_text_label(sys_settings_scr, ICON_VOLUME_L, &my_custom_icon_50, lv_color_hex(0xFFFFFF), 900, 452, LV_OPA_100);

    create_text_label(sys_settings_scr, "通知", &eques_regular_32, lv_color_hex(0xFFFFFF), 73, 499, LV_OPA_100);
    create_text_label(sys_settings_scr, ICON_VOLUME_S, &my_custom_icon_50, lv_color_hex(0xFFFFFF), 90, 565, LV_OPA_100);
    create_text_label(sys_settings_scr, ICON_VOLUME_L, &my_custom_icon_50, lv_color_hex(0xFFFFFF), 900, 565, LV_OPA_100);
    
    // ==================== 创建铃声滑块 ====================
    ring_slider = lv_slider_create(sys_settings_scr);
    lv_obj_set_size(ring_slider, 750, 8);
    lv_obj_set_pos(ring_slider, 137, 466);
    lv_obj_set_style_bg_color(ring_slider, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ring_slider, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_bg_color(ring_slider, lv_color_hex(0x00FFFF), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(ring_slider, lv_color_hex(0xFFFFFF), LV_PART_KNOB);
    // 设置默认值为50%
    lv_slider_set_value(ring_slider, SLIDER_DEFAULT_VALUE, LV_ANIM_OFF);
    // 添加值改变回调
    lv_obj_add_event_cb(ring_slider, ring_slider_value_changed_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // ==================== 创建通知音量滑块 ====================
    notify_slider = lv_slider_create(sys_settings_scr);
    lv_obj_set_size(notify_slider, 750, 8);
    lv_obj_set_pos(notify_slider, 137, 580);
    lv_obj_set_style_bg_color(notify_slider, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(notify_slider, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_bg_color(notify_slider, lv_color_hex(0x00FFFF), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(notify_slider, lv_color_hex(0xFFFFFF), LV_PART_KNOB);
    // 设置默认值为50%
    lv_slider_set_value(notify_slider, SLIDER_DEFAULT_VALUE, LV_ANIM_OFF);
    // 添加值改变回调
    lv_obj_add_event_cb(notify_slider, notify_slider_value_changed_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // lv_obj_t *have_ring_img = create_image_obj(sys_settings_scr, "D:have_ring.png", 674, 494);
    // lv_obj_t *no_ringr_img = create_image_obj(sys_settings_scr, "D:no_ring.png", 72, 494);
    // lv_obj_t *have_ring2_img = create_image_obj(sys_settings_scr, "D:have_ring.png", 674, 621);
    // lv_obj_t *no_ringr2_img = create_image_obj(sys_settings_scr, "D:no_ring.png", 72, 621);

    // 监控模式容器
    lv_obj_t *sys_settings_con2 = create_container(
        sys_settings_scr, 48, 634, 928, 83, 
        lv_color_hex(0x192A46), LV_OPA_100, 6, 
        lv_color_hex(0x2E4B7D), 0, LV_OPA_0
    );
    if(sys_settings_con2) {
        lv_obj_add_flag(sys_settings_con2, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(sys_settings_con2, LV_OPA_70, LV_STATE_PRESSED);
        lv_obj_add_event_cb(sys_settings_con2, monitor_mode_click_cb, LV_EVENT_CLICKED, sys_settings_scr);
    }
    // lv_obj_t *Vector_img1 = create_image_obj(sys_settings_scr, "D:Vector.png", 710, 730);
    create_text_label(sys_settings_scr, "监控模式", &eques_regular_36, lv_color_hex(0xFFFFFF), 75, 654, LV_OPA_100);
    create_text_label(sys_settings_scr, ICON_CHEVORN_RIGHT, &my_custom_icon, lv_color_hex(0xC4C4C4), 935, 660, LV_OPA_100);

    // 生物识别容器
    lv_obj_t *sys_settings_con3 = create_container(
        sys_settings_scr, 48, 727, 928, 83, 
        lv_color_hex(0x192A46), LV_OPA_100, 6, 
        lv_color_hex(0x2E4B7D), 0, LV_OPA_0
    );
    if(sys_settings_con3) {
        lv_obj_add_flag(sys_settings_con3, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(sys_settings_con3, LV_OPA_70, LV_STATE_PRESSED);
        lv_obj_add_event_cb(sys_settings_con3, biometrics_click_cb, LV_EVENT_CLICKED, sys_settings_scr);
    }
    // lv_obj_t *Vector_img2 = create_image_obj(sys_settings_scr, "D:Vector.png", 710, 820);
    create_text_label(sys_settings_scr, "生物识别", &eques_regular_36, lv_color_hex(0xFFFFFF), 75, 747, LV_OPA_100);
    create_text_label(sys_settings_scr, ICON_CHEVORN_RIGHT, &my_custom_icon, lv_color_hex(0xC4C4C4), 935, 750, LV_OPA_100);

    // 消息通知容器
    lv_obj_t *sys_settings_con4 = create_container(
        sys_settings_scr, 48, 820, 928, 83, 
        lv_color_hex(0x192A46), LV_OPA_100, 6, 
        lv_color_hex(0x2E4B7D), 0, LV_OPA_0
    );
    if(sys_settings_con4) {
        lv_obj_add_flag(sys_settings_con4, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(sys_settings_con4, LV_OPA_70, LV_STATE_PRESSED);
        lv_obj_add_event_cb(sys_settings_con4, notification_click_cb, LV_EVENT_CLICKED, sys_settings_scr);
    }
    // lv_obj_t *Vector_img3 = create_image_obj(sys_settings_scr, "D:Vector.png", 710, 910);
    create_text_label(sys_settings_scr, "消息通知", &eques_regular_36, lv_color_hex(0xFFFFFF), 75, 840, LV_OPA_100);
    create_text_label(sys_settings_scr, ICON_CHEVORN_RIGHT, &my_custom_icon, lv_color_hex(0xC4C4C4), 935, 840, LV_OPA_100);
    // 恢复出厂设置容器
    lv_obj_t *sys_settings_con5 = create_container(
        sys_settings_scr, 48, 913, 928, 83, 
        lv_color_hex(0x192A46), LV_OPA_100, 6, 
        lv_color_hex(0x2E4B7D), 0, LV_OPA_0
    );
    if(sys_settings_con5) {
        lv_obj_add_flag(sys_settings_con5, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(sys_settings_con5, LV_OPA_70, LV_STATE_PRESSED);
        lv_obj_add_event_cb(sys_settings_con5, factory_reset_click_cb, LV_EVENT_CLICKED, sys_settings_scr);
    }
    create_text_label(sys_settings_scr, "恢复出厂设置", &eques_regular_36, lv_color_hex(0xFFFFFF), 75, 933, LV_OPA_100);

    // 返回
    lv_obj_t *back_btn = create_text_label
    (sys_settings_scr, ICON_CHEVORN_LEFT, &my_custom_icon, lv_color_hex(0xFFFFFF), 52, 84, LV_OPA_100);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(back_btn,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn,LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn, sys_settings_back_btn_click_cb, LV_EVENT_CLICKED, homepage_scr);

    update_status_bar_parent(sys_settings_scr);
    lv_scr_load(sys_settings_scr);
}



// ==================== 弹窗相关函数 ====================
static void reset_popup_close_cb(lv_event_t *e) {
    if (lv_obj_is_valid(reset_popup)){
        lv_obj_del(reset_popup);
        reset_popup = NULL;
    }
}

// 自定义：恢复出厂设置后执行的初始化函数
static void factory_reset_confirm_action(void) {
    // 重置亮屏时间
    screen_time_reset_to_default();
    // 重置监控模式
    monitor_mode_reset_to_default();
    // 重置生物识别
    biometrics_reset_to_default();
    // 重置通知模式
    notification_set_mode(NOTIFY_ALL);
    notification_set_dnd_state(false);
    notification_set_dnd_time_slot("22:00-6:00");
    // 重置滑块（亮度、铃声、通知音量）到默认值50%
    slider_reset_to_default();
    LV_LOG_USER("Factory reset confirmed: all states reset to default!");
}

static void reset_confirm_cb(lv_event_t *e) {
    factory_reset_confirm_action();
    reset_popup_close_cb(e);
}

// 恢复出厂设置
static void factory_reset_click_cb(lv_event_t *e) {
    if (e == NULL) return;
    LV_LOG_USER("Factory reset clicked");

    // 创建全屏遮罩层
    reset_popup = lv_obj_create(lv_scr_act());
    lv_obj_set_size(reset_popup, LV_HOR_RES, 1000);
    lv_obj_set_pos(reset_popup, 0, 0);
    lv_obj_set_style_bg_color(reset_popup, lv_color_hex(0x000000), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(reset_popup, LV_OPA_50, LV_STATE_DEFAULT);
    lv_obj_add_flag(reset_popup, LV_OBJ_FLAG_EVENT_BUBBLE);
    // lv_obj_add_event_cb(reset_popup, reset_popup_close_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_border_width(reset_popup, 0, LV_STATE_DEFAULT);

    // 创建确认对话框容器
    lv_obj_t *confirm_dialog = lv_obj_create(reset_popup);
    lv_obj_set_size(confirm_dialog, 600, 250);
    lv_obj_set_pos(confirm_dialog, 212, 560);
    // lv_obj_set_align(confirm_dialog, LV_ALIGN_CENTER);
    lv_obj_set_style_bg_color(confirm_dialog, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
    lv_obj_set_style_radius(confirm_dialog, 16, LV_STATE_DEFAULT);
    lv_obj_clear_flag(confirm_dialog, LV_OBJ_FLAG_EVENT_BUBBLE);

    // 添加标题
    lv_obj_t *title_label = create_text_label(confirm_dialog, "确认恢复出厂设置", 
                                             &eques_regular_32, lv_color_hex(0xF03535), 
                                             0, 0, LV_OPA_100);
    lv_obj_set_align(title_label, LV_ALIGN_TOP_MID);
    lv_obj_set_pos(title_label, 0, 30);

    // 添加确认按钮
    lv_obj_t *confirm_btn = lv_btn_create(confirm_dialog);
    lv_obj_set_size(confirm_btn, 120, 50);
    lv_obj_set_align(confirm_btn, LV_ALIGN_LEFT_MID);
    lv_obj_set_pos(confirm_btn, 80, 20);
    lv_obj_set_style_bg_opa(confirm_btn, LV_OPA_TRANSP, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(confirm_btn, 0, LV_STATE_DEFAULT);
    create_text_label(confirm_btn, "确认", &eques_regular_32, lv_color_hex(0x666666), 0, 0, LV_OPA_100);
    lv_obj_set_style_shadow_width(confirm_btn, 0, LV_STATE_DEFAULT);
    lv_obj_add_event_cb(confirm_btn, reset_confirm_cb, LV_EVENT_CLICKED, NULL);

    // 添加取消按钮
    lv_obj_t *cancel_btn = lv_btn_create(confirm_dialog);
    lv_obj_set_size(cancel_btn, 120, 50);
    lv_obj_set_align(cancel_btn, LV_ALIGN_RIGHT_MID);
    lv_obj_set_pos(cancel_btn, -80, 20);
    lv_obj_set_style_bg_opa(cancel_btn, LV_OPA_TRANSP, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cancel_btn, 0, LV_STATE_DEFAULT);
    create_text_label(cancel_btn, "取消", 
                                             &eques_regular_32, lv_color_hex(0x00BDBD), 
                                             0, 0, LV_OPA_100);
    lv_obj_set_style_shadow_width(cancel_btn, 0, LV_STATE_DEFAULT);
    lv_obj_add_event_cb(cancel_btn, reset_popup_close_cb, LV_EVENT_CLICKED, NULL);
}

extern void lv_homepage(void);
extern void destroy_homepage(void);
void sys_settings_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;

    lv_obj_t *homepage_scr_temp = (lv_obj_t *)lv_event_get_user_data(e);
    if(homepage_scr_temp == NULL) {
        LV_LOG_WARN("sys_settings_btn_click_cb: homepage_scr is NULL!");
        return;
    }

    // 创建用户管理界面
    ui_sys_settings_create(homepage_scr_temp);
    // 更新状态栏
    
    // 销毁主页
    destroy_homepage();
    update_status_bar_parent(sys_settings_scr);
    LV_LOG_WARN("sys_settings_btn_click_cb: Destroy the homepage and create the sys_settings interface");
}
// 系统设置界面返回
void sys_settings_back_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;

    lv_obj_t *current_del_scr = lv_disp_get_scr_act(NULL);

    if(!lv_obj_is_valid(current_del_scr)) return;

    // 当前显示的是系统设置界面 → 重建主页并销毁当前界面
    if(current_del_scr == sys_settings_scr) {
        lv_homepage();                      // 重建主页
        lv_obj_del(current_del_scr);        // 销毁系统设置界面
        sys_settings_scr = NULL;            // 指针置空
        return;
    }
}
// 完全销毁系统设置界面 → 零内存泄漏
void sys_settings_destroy(void)
{
    // // 1. 销毁恢复出厂弹窗
    // if (lv_obj_is_valid(reset_popup)) {
    //     lv_obj_del(reset_popup);
    //     reset_popup = NULL;
    // }

    // 2. 销毁主界面
    if (lv_obj_is_valid(sys_settings_scr)) {
        lv_obj_del(sys_settings_scr);
        sys_settings_scr = NULL;
    }

    // // 3. 释放样式（解决你说的几KB泄漏）
    // if (sys_settings_style_inited) {
    //     lv_style_reset(&sys_settings_grad_style);
    //     sys_settings_style_inited = false;
    // }

    // // 4. 所有全局控件指针 置空
    // g_time_view_label = NULL;
    // light_slider = NULL;
    // ring_slider = NULL;
    // notify_slider = NULL;
}
#endif

#endif
