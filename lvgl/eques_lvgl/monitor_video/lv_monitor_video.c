#if 0
#include "lv_monitor_video.h"

static lv_obj_t *monitor_video_scr = NULL; 

static lv_style_t monitor_video_grad_style;
static bool monitor_video_style_inited = false;

static bool monitor_sound = true;// 监控视频是否开启声音(true:开启,false:关闭)
static lv_obj_t *monitor_sound_on_btn = NULL;
static lv_obj_t *monitor_sound_off_btn = NULL;

static bool monitor_mic = true;// 监控视频是否开启麦克风(true:开启,false:关闭)
static lv_obj_t *monitor_mic_on_btn = NULL;
static lv_obj_t *monitor_mic_off_btn = NULL;

static bool monitor_record = true;// 监控视频是否开启录屏(true:开启,false:关闭)
static lv_obj_t *monitor_record_on_btn = NULL;
static lv_obj_t *monitor_record_off_btn = NULL;

static bool monitor_lock = true;// 监控视频是否开启锁(true:开启,false:关闭)
static lv_obj_t *monitor_lock_on_btn = NULL;
static lv_obj_t *monitor_lock_off_btn = NULL;
static lv_obj_t *monitor_lock_on_con = NULL;

// 定时器句柄，用于管理自动关锁定时器
static lv_timer_t *lock_auto_close_timer = NULL;

static void monitor_sound_event_cb(lv_event_t *e);
static void monitor_mic_event_cb(lv_event_t *e);
static void monitor_record_event_cb(lv_event_t *e);
static void monitor_lock_event_cb(lv_event_t *e);
// 自动关锁定时器回调函数
static void lock_auto_close_timer_cb(lv_timer_t *timer);
// 清除定时器的函数
static void clear_lock_auto_close_timer(void);

// 同步声音按钮显示状态的函数
static void sync_sound_btn_state(void)
{
    if(!lv_obj_is_valid(monitor_sound_on_btn) || !lv_obj_is_valid(monitor_sound_off_btn)) return;
    
    if(monitor_sound) {
        // 开启状态：隐藏on按钮，显示off按钮
        lv_obj_add_flag(monitor_sound_on_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(monitor_sound_off_btn, LV_OBJ_FLAG_HIDDEN);
    } else {
        // 关闭状态：显示on按钮，隐藏off按钮
        lv_obj_clear_flag(monitor_sound_on_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(monitor_sound_off_btn, LV_OBJ_FLAG_HIDDEN);
    }
}

// 同步麦克风按钮显示状态的函数
static void sync_mic_btn_state(void)
{
    if(!lv_obj_is_valid(monitor_mic_on_btn) || !lv_obj_is_valid(monitor_mic_off_btn)) return;
    
    if(monitor_mic) {
        // 开启状态：隐藏off按钮，显示on按钮
        lv_obj_add_flag(monitor_mic_off_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(monitor_mic_on_btn, LV_OBJ_FLAG_HIDDEN);
    } else {
        // 关闭状态：显示off按钮，隐藏on按钮
        lv_obj_clear_flag(monitor_mic_off_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(monitor_mic_on_btn, LV_OBJ_FLAG_HIDDEN);
    }
}

// 同步录屏按钮显示状态的函数
static void sync_record_btn_state(void)
{
    if(!lv_obj_is_valid(monitor_record_on_btn) || !lv_obj_is_valid(monitor_record_off_btn)) return;
    
    if(monitor_record) {
        // 开启状态：隐藏on按钮，显示off按钮
        lv_obj_add_flag(monitor_record_on_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(monitor_record_off_btn, LV_OBJ_FLAG_HIDDEN);
    } else {
        // 关闭状态：显示on按钮，隐藏off按钮
        lv_obj_clear_flag(monitor_record_on_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(monitor_record_off_btn, LV_OBJ_FLAG_HIDDEN);
    }
}

// 同步锁按钮显示状态的函数
static void sync_lock_btn_state(void)
{
    if(!lv_obj_is_valid(monitor_lock_on_btn) || !lv_obj_is_valid(monitor_lock_off_btn)) return;
    
    if(monitor_lock) {
        // 开启状态：显示关锁图标（lock_on），隐藏开锁图标（lock_off）
        lv_obj_clear_flag(monitor_lock_on_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(monitor_lock_off_btn, LV_OBJ_FLAG_HIDDEN);
    } else {
        // 关闭状态：显示开锁图标（lock_off），隐藏关锁图标（lock_on）
        lv_obj_add_flag(monitor_lock_on_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(monitor_lock_off_btn, LV_OBJ_FLAG_HIDDEN);
    }
}

// 清除自动关锁定时器
static void clear_lock_auto_close_timer(void)
{
    if(lock_auto_close_timer != NULL) {
        lv_timer_del(lock_auto_close_timer); // 删除定时器
        lock_auto_close_timer = NULL;        // 清空句柄，避免野指针
        LV_LOG_USER("lock auto close timer cleared");
    }
}

// 自动关锁定时器回调函数（2秒后执行）
static void lock_auto_close_timer_cb(lv_timer_t *timer)
{
    // 1. 先清除定时器（避免重复触发）
    clear_lock_auto_close_timer();
    
    // 2. 检查锁按钮有效性
    if(!lv_obj_is_valid(monitor_lock_on_btn) || !lv_obj_is_valid(monitor_lock_off_btn)) {
        LV_LOG_WARN("lock_auto_close_timer_cb: invalid lock btn");
        return;
    }
    
    // 3. 强制关锁（恢复为开启状态），同步UI显示关锁图标
    monitor_lock = true;
    sync_lock_btn_state();
    LV_LOG_USER("lock auto closed after 2s");
}

// 全局样式初始化
static void init_monitor_video_styles(void)
{
    if(!monitor_video_style_inited) {
        lv_style_init(&monitor_video_grad_style);
        monitor_video_style_inited = true;
    }
}

void ui_monitor_video_create(lv_obj_t *homepage_scr)
{
    init_monitor_video_styles();
    // 1. 安全校验：如果传进来的 homepage_scr 为空，直接返回
    if(homepage_scr == NULL) {
        LV_LOG_WARN("ui_monitor_video_create: homepage_scr is NULL!");
        return;
    }
    
    // 2. 清除旧的定时器（避免重复创建）
    clear_lock_auto_close_timer();

    // 3. 创建/复用设置屏幕对象
    if(monitor_video_scr == NULL) {
        monitor_video_scr = lv_obj_create(NULL);  // 创建独立屏幕
    } else {
        lv_obj_clean(monitor_video_scr);          // 清空原有内容
    }
    
    lv_style_reset(&monitor_video_grad_style);
    lv_style_set_bg_color(&monitor_video_grad_style, lv_color_hex(0x010715));// 渐变主色：#010715（0%）
    lv_style_set_bg_grad_color(&monitor_video_grad_style, lv_color_hex(0x0E1D37));// 渐变副色：#0E1D37（100%）
    lv_style_set_bg_grad_dir(&monitor_video_grad_style, LV_GRAD_DIR_VER);// 渐变方向：垂直
    lv_style_set_bg_main_stop(&monitor_video_grad_style, 0);// 渐变范围：0~255
    lv_style_set_bg_grad_stop(&monitor_video_grad_style, 255);
    lv_obj_add_style(monitor_video_scr, &monitor_video_grad_style, LV_STATE_DEFAULT);// 应用渐变样式到屏幕

    /*******************************************监控操作容器*******************************************/
    lv_obj_t *monitor_video_mode_con = create_container(monitor_video_scr,
    221,479,710,118,
    lv_color_hex(0xFFFFFF), LV_OPA_80, 146,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(monitor_video_mode_con, 0, 0);
    /**********************************************监控操作*******************************************/
    // 监控视频声音开关
    monitor_sound_on_btn = create_image_obj(monitor_video_mode_con, "H:monitor_sound_on.png", 56, 21);//开启声音
    lv_obj_add_flag(monitor_sound_on_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(monitor_sound_on_btn, LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(monitor_sound_on_btn, monitor_sound_event_cb, LV_EVENT_CLICKED, NULL);

    monitor_sound_off_btn = create_image_obj(monitor_video_mode_con, "H:monitor_sound_off.png", 56, 21);//关闭声音
    lv_obj_add_flag(monitor_sound_off_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(monitor_sound_off_btn, LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(monitor_sound_off_btn, monitor_sound_event_cb, LV_EVENT_CLICKED, NULL);

    // 监控视频麦克风开关
    monitor_mic_on_btn = create_image_obj(monitor_video_mode_con, "H:microphone_on.png", 175, 16);
    lv_obj_add_flag(monitor_mic_on_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(monitor_mic_on_btn, LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(monitor_mic_on_btn, monitor_mic_event_cb, LV_EVENT_CLICKED, NULL);

    monitor_mic_off_btn = create_image_obj(monitor_video_mode_con, "H:microphone_off.png", 175, 16);
    lv_obj_add_flag(monitor_mic_off_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(monitor_mic_off_btn, LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(monitor_mic_off_btn, monitor_mic_event_cb, LV_EVENT_CLICKED, NULL);

    // 创建按钮后，同步UI状态到当前变量值
    sync_sound_btn_state();
    sync_mic_btn_state();
    monitor_record = false;
    sync_record_btn_state();
    monitor_lock = true;
    sync_lock_btn_state();

    // 监控视频锁开关
    monitor_lock_on_con = create_custom_gradient_container(
    monitor_video_mode_con,
    308, 12, 99, 99,       
    100, 0x006BDC, 0x00BDBD, LV_GRAD_DIR_VER,
    0, 225, LV_OPA_100);
    lv_obj_add_flag(monitor_lock_on_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(monitor_lock_on_con, LV_OPA_80,LV_STATE_PRESSED);
    // 给背景容器绑定锁回调（扩大点击区域，仅触发开锁）
    lv_obj_add_event_cb(monitor_lock_on_con, monitor_lock_event_cb, LV_EVENT_CLICKED, NULL);

    monitor_lock_on_btn = create_image_obj(monitor_video_mode_con, "H:lock_on.png", 314, 18);//关锁图标
    lv_obj_add_flag(monitor_lock_on_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(monitor_lock_on_btn, LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(monitor_lock_on_btn, monitor_lock_event_cb, LV_EVENT_CLICKED, NULL);

    monitor_lock_off_btn = create_image_obj(monitor_video_mode_con, "H:lock_off.png", 314, 18);//开锁图标
    lv_obj_add_flag(monitor_lock_off_btn, LV_OBJ_FLAG_HIDDEN); // 默认隐藏开锁图标

    //监控视频截屏按钮
    lv_obj_t *monitor_screenshot_btn = create_image_obj(monitor_video_mode_con, "H:Screenshot.png", 448, 22);
    lv_obj_add_flag(monitor_screenshot_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(monitor_screenshot_btn, LV_OPA_80,LV_STATE_PRESSED);
    
    //监控视频录屏开关
    // 开启录屏按钮
    monitor_record_on_btn = create_image_obj(monitor_video_mode_con, "H:vedio_on.png", 569, 22);
    lv_obj_add_flag(monitor_record_on_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(monitor_record_on_btn, LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(monitor_record_on_btn, monitor_record_event_cb, LV_EVENT_CLICKED, NULL);
    // 关闭录屏按钮
    monitor_record_off_btn = create_image_obj(monitor_video_mode_con, "H:vedio_off.png", 555, 18);
    lv_obj_add_flag(monitor_record_off_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(monitor_record_off_btn, LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_flag(monitor_record_off_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(monitor_record_off_btn, monitor_record_event_cb, LV_EVENT_CLICKED, NULL);
    
    // 左上角返回按钮
    //lv_obj_t *back_btn = create_image_obj(monitor_video_scr, "H:back.png", 52, 123);
    lv_obj_t *back_btn = create_container_circle(monitor_video_scr, 52, 90, 30,
    true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn, back_btn_click_cb, LV_EVENT_CLICKED, homepage_scr);
    
    //更新状态条父对象
    update_status_bar_parent(monitor_video_scr);
    // 切换到设置屏幕
    lv_scr_load(monitor_video_scr);
}

/*********************************声音开启/关闭事件回调**********************************/
static void monitor_sound_event_cb(lv_event_t *e)
{
    if(e == NULL || !lv_obj_is_valid(monitor_sound_on_btn) || !lv_obj_is_valid(monitor_sound_off_btn)){
        LV_LOG_WARN("monitor_sound_event_cb: invalid btn or ui object");
        return;
    }
    if(monitor_sound) {
        // 关闭声音
        monitor_sound = false;
        lv_obj_add_flag(monitor_sound_off_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(monitor_sound_on_btn, LV_OBJ_FLAG_HIDDEN);
        LV_LOG_USER("monitor sound off");
    } else {
        // 开启声音
        monitor_sound = true;
        lv_obj_add_flag(monitor_sound_on_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(monitor_sound_off_btn, LV_OBJ_FLAG_HIDDEN);
        LV_LOG_USER("monitor sound on");
    }
}

/*********************************麦克风开启/关闭事件回调***********************************/
static void monitor_mic_event_cb(lv_event_t *e)
{
    // 增加monitor_mic_off_btn的有效性检查
    if(e == NULL || !lv_obj_is_valid(monitor_mic_on_btn) || !lv_obj_is_valid(monitor_mic_off_btn)){
        LV_LOG_WARN("monitor_mic_event_cb: invalid btn or ui object");
        return;
    }
    if(monitor_mic) {
        // 关闭麦克风
        monitor_mic = false;
        lv_obj_add_flag(monitor_mic_on_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(monitor_mic_off_btn, LV_OBJ_FLAG_HIDDEN);
        LV_LOG_USER("monitor mic off");
    } else {
        // 开启麦克风
        monitor_mic = true;
        lv_obj_add_flag(monitor_mic_off_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(monitor_mic_on_btn, LV_OBJ_FLAG_HIDDEN);
        LV_LOG_USER("monitor mic on");
    }
}

/*********************************录屏开启/关闭事件回调***********************************/
static void monitor_record_event_cb(lv_event_t *e)
{
    if(e == NULL || !lv_obj_is_valid(monitor_record_on_btn) || !lv_obj_is_valid(monitor_record_off_btn)){
        LV_LOG_WARN("monitor_record_event_cb: invalid btn or ui object");
        return;
    }
    if(monitor_record) {
        // 关闭录屏
        monitor_record = false;
        lv_obj_add_flag(monitor_record_off_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(monitor_record_on_btn, LV_OBJ_FLAG_HIDDEN);
        LV_LOG_USER("monitor record off");
    } else {
        // 开启录屏
        monitor_record = true;
        lv_obj_add_flag(monitor_record_on_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(monitor_record_off_btn, LV_OBJ_FLAG_HIDDEN);
        LV_LOG_USER("monitor record on");
    }
}

/*********************************门锁事件回调（仅保留开锁+自动关锁）***********************************/
static void monitor_lock_event_cb(lv_event_t *e)
{
    if(e == NULL || !lv_obj_is_valid(monitor_lock_on_btn) || !lv_obj_is_valid(monitor_lock_off_btn)){
        LV_LOG_WARN("monitor_lock_event_cb: invalid btn or ui object");
        return;
    }
    
    // 仅处理“开锁”逻辑（无论当前状态，点击即开锁，2秒后自动关锁）
    if(monitor_lock) {
        // 1. 切换为开锁状态
        monitor_lock = false;
        sync_lock_btn_state(); // 显示开锁图标
        LV_LOG_USER("monitor lock off (unlock)");
        
        // 2. 先清除旧定时器，再创建2秒自动关锁定时器
        clear_lock_auto_close_timer();
        lock_auto_close_timer = lv_timer_create(lock_auto_close_timer_cb, 1000, NULL); // 2000ms = 2秒
        LV_LOG_USER("lock auto close timer started (2s)");
    }
    // 移除手动关锁逻辑：点击开锁图标时不处理，仅等待定时器自动关锁
}


/***********************监控视频界面回调*********************/
void monitor_video_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    lv_obj_t *homepage_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(homepage_scr == NULL) {
        LV_LOG_WARN("monitor_video_btn_click_cb: homepage_scr is NULL!");
        return;
    }
    ui_monitor_video_create(homepage_scr);
}
#else
#include "lv_monitor_video.h"

static lv_obj_t *monitor_video_scr = NULL; 

static lv_style_t monitor_video_grad_style;
static bool monitor_video_style_inited = false;

static bool monitor_sound = true;// 监控视频是否开启声音(true:开启,false:关闭)
static lv_obj_t *monitor_sound_on_btn = NULL;
static lv_obj_t *monitor_sound_off_btn = NULL;

static bool monitor_mic = true;// 监控视频是否开启麦克风(true:开启,false:关闭)
static lv_obj_t *monitor_mic_on_btn = NULL;
static lv_obj_t *monitor_mic_off_btn = NULL;

static bool monitor_record = true;// 监控视频是否开启录屏(true:开启,false:关闭)
static lv_obj_t *monitor_record_on_btn = NULL;
static lv_obj_t *monitor_record_off_btn = NULL;

static bool monitor_lock = true;// 监控视频是否开启锁(true:开启,false:关闭)
static lv_obj_t *monitor_lock_on_btn = NULL;
static lv_obj_t *monitor_lock_off_btn = NULL;
static lv_obj_t *monitor_lock_on_con = NULL;

// 定时器句柄，用于管理自动关锁定时器
static lv_timer_t *lock_auto_close_timer = NULL;

static void monitor_sound_event_cb(lv_event_t *e);
static void monitor_mic_event_cb(lv_event_t *e);
static void monitor_record_event_cb(lv_event_t *e);
static void monitor_lock_event_cb(lv_event_t *e);
// 自动关锁定时器回调函数
static void lock_auto_close_timer_cb(lv_timer_t *timer);
// 清除定时器的函数
static void clear_lock_auto_close_timer(void);

// 同步声音按钮显示状态的函数
static void sync_sound_btn_state(void)
{
    if(!lv_obj_is_valid(monitor_sound_on_btn) || !lv_obj_is_valid(monitor_sound_off_btn)) return;
    
    if(monitor_sound) {
        // 开启状态：隐藏on按钮，显示off按钮
        lv_obj_add_flag(monitor_sound_on_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(monitor_sound_off_btn, LV_OBJ_FLAG_HIDDEN);
    } else {
        // 关闭状态：显示on按钮，隐藏off按钮
        lv_obj_clear_flag(monitor_sound_on_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(monitor_sound_off_btn, LV_OBJ_FLAG_HIDDEN);
    }
}

// 同步麦克风按钮显示状态的函数
static void sync_mic_btn_state(void)
{
    if(!lv_obj_is_valid(monitor_mic_on_btn) || !lv_obj_is_valid(monitor_mic_off_btn)) return;
    
    if(monitor_mic) {
        // 开启状态：隐藏off按钮，显示on按钮
        lv_obj_add_flag(monitor_mic_off_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(monitor_mic_on_btn, LV_OBJ_FLAG_HIDDEN);
    } else {
        // 关闭状态：显示off按钮，隐藏on按钮
        lv_obj_clear_flag(monitor_mic_off_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(monitor_mic_on_btn, LV_OBJ_FLAG_HIDDEN);
    }
}

// 同步录屏按钮显示状态的函数
static void sync_record_btn_state(void)
{
    if(!lv_obj_is_valid(monitor_record_on_btn) || !lv_obj_is_valid(monitor_record_off_btn)) return;
    
    if(monitor_record) {
        // 开启状态：隐藏on按钮，显示off按钮
        lv_obj_add_flag(monitor_record_on_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(monitor_record_off_btn, LV_OBJ_FLAG_HIDDEN);
    } else {
        // 关闭状态：显示on按钮，隐藏off按钮
        lv_obj_clear_flag(monitor_record_on_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(monitor_record_off_btn, LV_OBJ_FLAG_HIDDEN);
    }
}

// 同步锁按钮显示状态的函数
static void sync_lock_btn_state(void)
{
    if(!lv_obj_is_valid(monitor_lock_on_btn) || !lv_obj_is_valid(monitor_lock_off_btn)) return;
    
    if(monitor_lock) {
        // 开启状态：显示关锁图标（lock_on），隐藏开锁图标（lock_off）
        lv_obj_clear_flag(monitor_lock_on_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(monitor_lock_off_btn, LV_OBJ_FLAG_HIDDEN);
    } else {
        // 关闭状态：显示开锁图标（lock_off），隐藏关锁图标（lock_on）
        lv_obj_add_flag(monitor_lock_on_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(monitor_lock_off_btn, LV_OBJ_FLAG_HIDDEN);
    }
}

// 清除自动关锁定时器
static void clear_lock_auto_close_timer(void)
{
    if(lock_auto_close_timer != NULL) {
        lv_timer_del(lock_auto_close_timer); // 删除定时器
        lock_auto_close_timer = NULL;        // 清空句柄，避免野指针
        LV_LOG_USER("lock auto close timer cleared");
    }
}

// 自动关锁定时器回调函数（2秒后执行）
static void lock_auto_close_timer_cb(lv_timer_t *timer)
{
    // 1. 先清除定时器（避免重复触发）
    clear_lock_auto_close_timer();
    
    // 2. 检查锁按钮有效性
    if(!lv_obj_is_valid(monitor_lock_on_btn) || !lv_obj_is_valid(monitor_lock_off_btn)) {
        LV_LOG_WARN("lock_auto_close_timer_cb: invalid lock btn");
        return;
    }
    
    // 3. 强制关锁（恢复为开启状态），同步UI显示关锁图标
    monitor_lock = true;
    sync_lock_btn_state();
    LV_LOG_USER("lock auto closed after 2s");
}

// 全局样式初始化
static void init_monitor_video_styles(void)
{
    if(!monitor_video_style_inited) {
        lv_style_init(&monitor_video_grad_style);
        monitor_video_style_inited = true;
    }
}

void ui_monitor_video_create(lv_obj_t *homepage_scr)
{
    init_monitor_video_styles();
    // 1. 安全校验：如果传进来的 homepage_scr 为空，直接返回
    if(homepage_scr == NULL) {
        LV_LOG_WARN("ui_monitor_video_create: homepage_scr is NULL!");
        return;
    }
    
    // 2. 清除旧的定时器（避免重复创建）
    clear_lock_auto_close_timer();

    // 3. 创建/复用设置屏幕对象
    if(is_lv_obj_valid(monitor_video_scr)) {
            lv_obj_del(monitor_video_scr);
            monitor_video_scr = NULL;
        }
    monitor_video_scr = lv_obj_create(NULL);
    
    lv_style_reset(&monitor_video_grad_style);
    lv_style_set_bg_color(&monitor_video_grad_style, lv_color_hex(0x010715));// 渐变主色：#010715（0%）
    lv_style_set_bg_grad_color(&monitor_video_grad_style, lv_color_hex(0x0E1D37));// 渐变副色：#0E1D37（100%）
    lv_style_set_bg_grad_dir(&monitor_video_grad_style, LV_GRAD_DIR_VER);// 渐变方向：垂直
    lv_style_set_bg_main_stop(&monitor_video_grad_style, 0);// 渐变范围：0~255
    lv_style_set_bg_grad_stop(&monitor_video_grad_style, 255);
    lv_obj_add_style(monitor_video_scr, &monitor_video_grad_style, LV_STATE_DEFAULT);// 应用渐变样式到屏幕

    /*******************************************监控操作容器*******************************************/
    lv_obj_t *monitor_video_mode_con = create_container(monitor_video_scr,
    221,479,582,97,
    lv_color_hex(0xFFFFFF), LV_OPA_80, 146,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(monitor_video_mode_con, 0, 0);
    /**********************************************监控操作*******************************************/
    // 监控视频声音开关
    monitor_sound_on_btn = create_image_obj(monitor_video_mode_con, "H:monitor_sound_on.png", 56, 21);//开启声音
    lv_obj_add_flag(monitor_sound_on_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(monitor_sound_on_btn, LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(monitor_sound_on_btn, monitor_sound_event_cb, LV_EVENT_CLICKED, NULL);

    monitor_sound_off_btn = create_image_obj(monitor_video_mode_con, "H:monitor_sound_off.png", 56, 21);//关闭声音
    lv_obj_add_flag(monitor_sound_off_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(monitor_sound_off_btn, LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(monitor_sound_off_btn, monitor_sound_event_cb, LV_EVENT_CLICKED, NULL);

    // 监控视频麦克风开关
    monitor_mic_on_btn = create_image_obj(monitor_video_mode_con, "H:microphone_on.png", 175, 16);
    lv_obj_add_flag(monitor_mic_on_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(monitor_mic_on_btn, LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(monitor_mic_on_btn, monitor_mic_event_cb, LV_EVENT_CLICKED, NULL);

    monitor_mic_off_btn = create_image_obj(monitor_video_mode_con, "H:microphone_off.png", 175, 16);
    lv_obj_add_flag(monitor_mic_off_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(monitor_mic_off_btn, LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(monitor_mic_off_btn, monitor_mic_event_cb, LV_EVENT_CLICKED, NULL);

    // 创建按钮后，同步UI状态到当前变量值
    sync_sound_btn_state();
    sync_mic_btn_state();
    monitor_record = false;
    sync_record_btn_state();
    monitor_lock = true;
    sync_lock_btn_state();

    // 监控视频锁开关
    monitor_lock_on_con = create_custom_gradient_container(
    monitor_video_mode_con,
    308, 12, 73, 73,       
    100, 0x006BDC, 0x00BDBD, LV_GRAD_DIR_VER,
    0, 225, LV_OPA_100);
    lv_obj_center(monitor_lock_on_con);
    lv_obj_add_flag(monitor_lock_on_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(monitor_lock_on_con, LV_OPA_80,LV_STATE_PRESSED);
    // 给背景容器绑定锁回调（扩大点击区域，仅触发开锁）
    lv_obj_add_event_cb(monitor_lock_on_con, monitor_lock_event_cb, LV_EVENT_CLICKED, NULL);

    monitor_lock_on_btn = create_image_obj(monitor_video_mode_con, "H:lock_on.png", 314, 18);//关锁图标
    lv_obj_add_flag(monitor_lock_on_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(monitor_lock_on_btn, LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(monitor_lock_on_btn, monitor_lock_event_cb, LV_EVENT_CLICKED, NULL);

    monitor_lock_off_btn = create_image_obj(monitor_video_mode_con, "H:lock_off.png", 314, 18);//开锁图标
    lv_obj_add_flag(monitor_lock_off_btn, LV_OBJ_FLAG_HIDDEN); // 默认隐藏开锁图标

    //监控视频截屏按钮
    lv_obj_t *monitor_screenshot_btn = create_image_obj(monitor_video_mode_con, "H:Screenshot.png", 448, 22);
    lv_obj_add_flag(monitor_screenshot_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(monitor_screenshot_btn, LV_OPA_80,LV_STATE_PRESSED);
    
    //监控视频录屏开关
    // 开启录屏按钮
    monitor_record_on_btn = create_image_obj(monitor_video_mode_con, "H:vedio_on.png", 569, 22);
    lv_obj_add_flag(monitor_record_on_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(monitor_record_on_btn, LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(monitor_record_on_btn, monitor_record_event_cb, LV_EVENT_CLICKED, NULL);
    // 关闭录屏按钮
    monitor_record_off_btn = create_image_obj(monitor_video_mode_con, "H:vedio_off.png", 555, 18);
    lv_obj_add_flag(monitor_record_off_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(monitor_record_off_btn, LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_flag(monitor_record_off_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(monitor_record_off_btn, monitor_record_event_cb, LV_EVENT_CLICKED, NULL);
    
    // 左上角返回按钮
    //lv_obj_t *back_btn = create_image_obj(monitor_video_scr, "H:back.png", 52, 123);
    lv_obj_t *back_btn = create_container_circle(monitor_video_scr, 52, 90, 30,
    true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn, back_btn_click_cb, LV_EVENT_CLICKED, homepage_scr);
    
    //更新状态条父对象
    update_status_bar_parent(monitor_video_scr);
    // 切换到设置屏幕
    lv_scr_load(monitor_video_scr);
}

/*********************************声音开启/关闭事件回调**********************************/
static void monitor_sound_event_cb(lv_event_t *e)
{
    if(e == NULL || !lv_obj_is_valid(monitor_sound_on_btn) || !lv_obj_is_valid(monitor_sound_off_btn)){
        LV_LOG_WARN("monitor_sound_event_cb: invalid btn or ui object");
        return;
    }
    if(monitor_sound) {
        // 关闭声音
        monitor_sound = false;
        lv_obj_add_flag(monitor_sound_off_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(monitor_sound_on_btn, LV_OBJ_FLAG_HIDDEN);
        LV_LOG_USER("monitor sound off");
    } else {
        // 开启声音
        monitor_sound = true;
        lv_obj_add_flag(monitor_sound_on_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(monitor_sound_off_btn, LV_OBJ_FLAG_HIDDEN);
        LV_LOG_USER("monitor sound on");
    }
}

/*********************************麦克风开启/关闭事件回调***********************************/
static void monitor_mic_event_cb(lv_event_t *e)
{
    // 增加monitor_mic_off_btn的有效性检查
    if(e == NULL || !lv_obj_is_valid(monitor_mic_on_btn) || !lv_obj_is_valid(monitor_mic_off_btn)){
        LV_LOG_WARN("monitor_mic_event_cb: invalid btn or ui object");
        return;
    }
    if(monitor_mic) {
        // 关闭麦克风
        monitor_mic = false;
        lv_obj_add_flag(monitor_mic_on_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(monitor_mic_off_btn, LV_OBJ_FLAG_HIDDEN);
        LV_LOG_USER("monitor mic off");
    } else {
        // 开启麦克风
        monitor_mic = true;
        lv_obj_add_flag(monitor_mic_off_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(monitor_mic_on_btn, LV_OBJ_FLAG_HIDDEN);
        LV_LOG_USER("monitor mic on");
    }
}

/*********************************录屏开启/关闭事件回调***********************************/
static void monitor_record_event_cb(lv_event_t *e)
{
    if(e == NULL || !lv_obj_is_valid(monitor_record_on_btn) || !lv_obj_is_valid(monitor_record_off_btn)){
        LV_LOG_WARN("monitor_record_event_cb: invalid btn or ui object");
        return;
    }
    if(monitor_record) {
        // 关闭录屏
        monitor_record = false;
        lv_obj_add_flag(monitor_record_off_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(monitor_record_on_btn, LV_OBJ_FLAG_HIDDEN);
        LV_LOG_USER("monitor record off");
    } else {
        // 开启录屏
        monitor_record = true;
        lv_obj_add_flag(monitor_record_on_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(monitor_record_off_btn, LV_OBJ_FLAG_HIDDEN);
        LV_LOG_USER("monitor record on");
    }
}

/*********************************门锁事件回调（仅保留开锁+自动关锁）***********************************/
static void monitor_lock_event_cb(lv_event_t *e)
{
    if(e == NULL || !lv_obj_is_valid(monitor_lock_on_btn) || !lv_obj_is_valid(monitor_lock_off_btn)){
        LV_LOG_WARN("monitor_lock_event_cb: invalid btn or ui object");
        return;
    }
    
    // 仅处理“开锁”逻辑（无论当前状态，点击即开锁，2秒后自动关锁）
    if(monitor_lock) {
        // 1. 切换为开锁状态
        monitor_lock = false;
        sync_lock_btn_state(); // 显示开锁图标
        LV_LOG_USER("monitor lock off (unlock)");
        
        // 2. 先清除旧定时器，再创建2秒自动关锁定时器
        clear_lock_auto_close_timer();
        lock_auto_close_timer = lv_timer_create(lock_auto_close_timer_cb, 1000, NULL); // 2000ms = 2秒
        LV_LOG_USER("lock auto close timer started (2s)");
    }
    // 移除手动关锁逻辑：点击开锁图标时不处理，仅等待定时器自动关锁
}


/***********************监控视频界面回调*********************/
void monitor_video_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    lv_obj_t *homepage_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(homepage_scr == NULL) {
        LV_LOG_WARN("monitor_video_btn_click_cb: homepage_scr is NULL!");
        return;
    }
    ui_monitor_video_create(homepage_scr);
}
#endif