#include "lv_file_cache.h"
#include "lvgl/lvgl.h"
#include <string.h>

/************************ 全局变量（每个页面独立屏幕） ************************/
lv_obj_t *file_cache_scr = NULL;     // 文件缓存主页面
static lv_obj_t *fc_video_scr = NULL;       // 视频缓存子页面
static lv_obj_t *fc_pic_scr = NULL;         // 图片缓存子页面

/************************ 视频播放页全局变量 ************************/
static lv_obj_t *fc_video_play_scr = NULL;  // 视频播放页面
static bool is_video_playing = false;       // 视频播放状态
static lv_timer_t *video_timer = NULL;      // 播放进度定时器
static int32_t current_video_sec = 0;       // 当前播放秒数
static int32_t total_video_sec = 183;       // 总时长 3:03 = 183秒
static lv_obj_t *current_time_label = NULL; // 当前时间标签（全局）
static lv_obj_t *video_slider = NULL;       // 进度条控件（全局）
static const char *current_video_path = NULL; // 当前播放视频路径标识

/************************ 视频缓存页多选全局变量 ************************/
#define MAX_VIDEO_ITEMS 6  // 初始最大项数
static bool video_selected[MAX_VIDEO_ITEMS] = {false}; // 每个视频项的选中状态
static bool select_mode = false;                       // 是否处于选择模式
static lv_obj_t *delete_btn = NULL;                    // 右上角删除按钮
static lv_obj_t *cancel_btn = NULL;                    // 左上角取消按钮
static lv_obj_t *title_label = NULL;                   // 页面标题
static lv_obj_t *video_grid_cont = NULL;               // 视频列表容器（优化刷新用）
static bool batch_selected_copy[MAX_VIDEO_ITEMS] = {false}; // 批量删除状态副本
static char *time_list[MAX_VIDEO_ITEMS] = {           // 动态可修改时间列表
    "2026-1-20 8:30:20",
    "2026-1-21 18:23:45",
    "2026-1-22 8:30:20",
    "2026-1-23 18:23:45",
    "2026-1-24 8:30:20",
    "2026-1-25 18:23:45",
};
static uint8_t current_video_count = MAX_VIDEO_ITEMS; // 当前视频实际数量
// 视频项用户数据结构体（绑定到按钮对象，无需手动释放）
typedef struct {
    uint8_t index;
    lv_obj_t *item_btn;
    lv_obj_t *parent_scr;
} video_item_data_t;
/************************ 图片缓存页多选全局变量 ************************/
#define MAX_PIC_ITEMS 12  // 数组最大容量（不变）
static bool pic_selected[MAX_PIC_ITEMS] = {false};
static bool pic_select_mode = false;
static lv_obj_t *pic_delete_btn = NULL;
static lv_obj_t *pic_cancel_btn = NULL;
static lv_obj_t *pic_title_label = NULL;
static lv_obj_t *pic_grid_cont = NULL;
static bool pic_batch_selected_copy[MAX_PIC_ITEMS] = {false};
static uint8_t current_pic_count = 0; //初始化为0，不再固定赋值
void file_cache_btn_click_cb(lv_event_t *e);
// 图片路径数组（可随意增删，空值会被自动忽略）
static char *pic_path_array[MAX_PIC_ITEMS] = {
    "D:/222.png",
    "D:/fc_pic_placeholder.png",
    "D:/fc_pic_placeholder.png",
    "D:/fc_pic_placeholder.png",
    "D:/fc_pic_placeholder.png",
    "D:/fc_pic_placeholder.png",
    "D:/fc_pic_placeholder.png",
    "D:/fc_pic_placeholder.png",
    "D:/fc_pic_placeholder.png",
    "D:/fc_pic_placeholder.png",
};

// 图片项用户数据结构体（绑定到按钮对象，无需手动释放）
typedef struct {
    uint8_t index;
    lv_obj_t *item_btn;
    lv_obj_t *parent_scr;
} pic_item_data_t;
/************************ 图片预览页全局变量 ************************/
static lv_obj_t *pic_preview_scr = NULL;  // 图片预览页面
static uint8_t current_preview_index = 0; // 当前预览的图片索引
static lv_obj_t *preview_img = NULL;      // 预览图片控件（全局化）

/************************ 样式全局变量 ************************/
static lv_style_t file_cache_grad_style;
static bool file_cache_style_inited = false;
static lv_style_t confirm_btn_grad_style;
static bool confirm_style_inited = false;
void file_cache_back_btn_click_cb(lv_event_t *e);
/************************ 弹窗全局变量 ************************/
static lv_obj_t *confirm_popup = NULL;
static lv_obj_t *popup_bg = NULL;

static lv_obj_t *item = NULL;
/************************ 通用工具函数 ************************/
// 样式初始化（统一复用）
static void init_file_cache_styles(void)
{
    if(!file_cache_style_inited) {
        lv_style_init(&file_cache_grad_style);
        file_cache_style_inited = true;
    }
}

// 通用背景样式设置（所有页面复用）
static void set_common_bg_style(lv_obj_t *scr)
{
    lv_style_reset(&file_cache_grad_style);
    lv_style_set_bg_color(&file_cache_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&file_cache_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&file_cache_grad_style, LV_GRAD_DIR_VER);
    lv_style_set_bg_main_stop(&file_cache_grad_style, 0);
    lv_style_set_bg_grad_stop(&file_cache_grad_style, 255);
    lv_obj_add_style(scr, &file_cache_grad_style, LV_STATE_DEFAULT);
}

void pic_cache_back_btn_click_cb(lv_event_t *e)
{
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);
    lv_obj_t *current_del_scr = lv_disp_get_scr_act(NULL);

    if (pic_grid_cont) {
        uint32_t cnt = lv_obj_get_child_cnt(pic_grid_cont);
        for (uint32_t i = 0; i < cnt; i++) {
            lv_obj_t *obj = lv_obj_get_child(pic_grid_cont, i);
            if (obj) {
                void *data = lv_obj_get_user_data(obj);
                if (data) {
                    lv_mem_free(data);
                }
            }
        }
    }
    if(current_del_scr == fc_pic_scr) {
        ui_file_cache_create(parent_scr);  
        lv_obj_del(item);
        lv_obj_del(current_del_scr);
        fc_pic_scr = NULL;
        item = NULL;
        return;
    }
}
void video_cache_back_btn_click_cb(lv_event_t *e)
{
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);
    lv_obj_t *current_del_scr = lv_disp_get_scr_act(NULL);

    if (video_grid_cont) {
        uint32_t cnt = lv_obj_get_child_cnt(video_grid_cont);
        for (uint32_t i = 0; i < cnt; i++) {
            lv_obj_t *obj = lv_obj_get_child(video_grid_cont, i);
            if (obj) {
                void *data = lv_obj_get_user_data(obj);
                if (data) {
                    lv_mem_free(data);
                }
            }
        }
    }
    if(current_del_scr == fc_video_scr) {
        ui_file_cache_create(parent_scr);  
        lv_obj_del(current_del_scr);
        fc_video_scr = NULL;
        return;
    }
}
void pic_preview_back_btn_click_cb(lv_event_t *e)
{
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);
    lv_obj_t *current_del_scr = lv_disp_get_scr_act(NULL);
    
    // 销毁预览页，返回图片页
    if(current_del_scr == pic_preview_scr) {
        lv_scr_load(parent_scr);  
        set_common_bg_style(parent_scr);
        lv_obj_del(current_del_scr);
        pic_preview_scr = NULL;
        return;
    }
}
void video_play_back_btn_click_cb(lv_event_t *e)
{
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);
    lv_obj_t *current_del_scr = lv_disp_get_scr_act(NULL);

    // ===================== 销毁视频定时器（关键！）
    if(video_timer != NULL) {
        lv_timer_del(video_timer);
        video_timer = NULL;
    }

    // 重置播放状态
    is_video_playing = false;
    current_video_sec = 0;
    current_video_path = NULL;

    // 销毁播放页，返回视频页
    if(current_del_scr == fc_video_play_scr) {
        lv_scr_load(parent_scr);  
        set_common_bg_style(parent_scr);
        lv_obj_del(current_del_scr);
        fc_video_play_scr = NULL;
        return;
    }
}
// 通用返回按钮创建（传递目标返回页面）
static lv_obj_t *create_common_back_btn(lv_obj_t *parent, lv_obj_t *target_scr)
{
    lv_obj_t *back_btn = create_text_label
    (parent, ICON_CHEVORN_LEFT, &my_custom_icon, lv_color_hex(0xFFFFFF), 52, 84, LV_OPA_100);
    if(back_btn == NULL) return NULL;

    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);

    // 区分四种页面，各自绑定带资源释放的回调
    if (parent == fc_pic_scr)
    {
        // 图片缓存页 返回
        lv_obj_add_event_cb(back_btn, pic_cache_back_btn_click_cb, LV_EVENT_CLICKED, target_scr);
    }
    else if (parent == fc_video_scr)
    {
        // 视频缓存页 返回
        lv_obj_add_event_cb(back_btn, video_cache_back_btn_click_cb, LV_EVENT_CLICKED, target_scr);
    }
    else if (parent == pic_preview_scr)
    {
        // 图片预览页 返回【新增】
        lv_obj_add_event_cb(back_btn, pic_preview_back_btn_click_cb, LV_EVENT_CLICKED, target_scr);
    }
    else if (parent == fc_video_play_scr)
    {
        // 视频播放页 返回【新增】
        lv_obj_add_event_cb(back_btn, video_play_back_btn_click_cb, LV_EVENT_CLICKED, target_scr);
    }

    return back_btn;
}

// 动态计算图片数组的有效数量（核心函数）
static uint8_t get_valid_pic_count(void)
{
    uint8_t count = 0;
    // 严格限制遍历范围，绝对不越界
    for(uint8_t i = 0; i < MAX_PIC_ITEMS; i++) {
        // 多层校验：数组项不为NULL + 不是空字符串 + 路径长度>0
        if(pic_path_array[i] != NULL && pic_path_array[i][0] != '\0' && strlen(pic_path_array[i]) > 0) {
            count++;
        }
    }
    // 防极端值：最多返回MAX_PIC_ITEMS，最少返回0
    return (count > MAX_PIC_ITEMS) ? MAX_PIC_ITEMS : count;
}
/************************ 图片预览切换回调函数 ************************/
// 切换到上一张图片
static void preview_prev_cb(lv_event_t *e)
{
    if(e == NULL || pic_preview_scr == NULL || !lv_obj_is_valid(pic_preview_scr)) return;
    
    if(current_preview_index > 0) {
        current_preview_index--;
        lv_img_set_src(preview_img, pic_path_array[current_preview_index]);
        LV_LOG_USER("Preview previous: index %d", current_preview_index);
    } else {
        LV_LOG_USER("Already at first image");
    }
}

// 切换到下一张图片
static void preview_next_cb(lv_event_t *e)
{
    if(e == NULL || pic_preview_scr == NULL || !lv_obj_is_valid(pic_preview_scr)) return;
    
    if(current_preview_index < current_pic_count - 1) {
        current_preview_index++;
        lv_img_set_src(preview_img, pic_path_array[current_preview_index]);
        LV_LOG_USER("Preview next: index %d", current_preview_index);
    } else {
        LV_LOG_USER("Already at last image");
    }
}
static void toggle_pic_selection(uint8_t index)
{
    LV_LOG_USER("==== toggle_pic_selection called ====");
    LV_LOG_USER("Target index: %d (user item %d), total items: %d", 
               index, index+1, current_pic_count);
    
    // 双重校验：彻底拦截越界
    if(index >= current_pic_count || index >= MAX_PIC_ITEMS) {
        LV_LOG_WARN("Toggle failed: index out of bounds (current: %d, max: %d)", current_pic_count, MAX_PIC_ITEMS);
        return;
    }

    bool state_before = pic_selected[index];
    pic_selected[index] = !state_before;
    LV_LOG_USER("State changed: %s → %s", 
               state_before ? "selected" : "unselected",
               pic_selected[index] ? "selected" : "unselected");

    bool has_selected = false;
    for(uint8_t i = 0; i < current_pic_count; i++) {
        if(pic_selected[i]) {
            has_selected = true;
            break;
        }
    }

    if(pic_delete_btn && pic_select_mode && lv_obj_is_valid(pic_delete_btn)) {
        lv_obj_clear_flag(pic_delete_btn, LV_OBJ_FLAG_HIDDEN);
        if(has_selected) {
            lv_obj_add_flag(pic_delete_btn, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_text_color(pic_delete_btn, lv_color_hex(0xFF4757), LV_PART_MAIN);
            lv_obj_set_style_opa(pic_delete_btn, LV_OPA_100, LV_PART_MAIN);
        } else {
            lv_obj_clear_flag(pic_delete_btn, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_text_color(pic_delete_btn, lv_color_hex(0x888888), LV_PART_MAIN);
            lv_obj_set_style_opa(pic_delete_btn, LV_OPA_60, LV_PART_MAIN);
        }
    }
    LV_LOG_USER("==== toggle_pic_selection finished ====\n");
}

static void pic_checkbox_click_cb(lv_event_t *e)
{
    if(e == NULL || !pic_select_mode) return;
    
    lv_obj_t *check = lv_event_get_target(e);
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    
    if(index >= MAX_PIC_ITEMS) {
        LV_LOG_WARN("Checkbox click: index %d out of bounds", index);
        return;
    }

    toggle_pic_selection(index);
    
    if(pic_selected[index]) {
        lv_label_set_text(check, ICON_CHECK);
        //lv_obj_set_style_text_color(check, lv_color_hex(0x4A6CF7), LV_PART_MAIN);
    } else {
        lv_label_set_text(check, "");
    }
    
    lv_event_stop_bubbling(e);
    LV_LOG_USER("Pic checkbox clicked: Item %d selected = %s", index+1, pic_selected[index] ? "true" : "false");
}

static void enter_pic_select_mode(lv_event_t *e)
{
    if(e == NULL || pic_select_mode) {
        LV_LOG_WARN("enter_pic_select_mode: skip (already in select mode)");
        return;
    }

    lv_obj_t *target = lv_event_get_target(e);
    if(!target || !lv_obj_check_type(target, &lv_btn_class)) return;
    
    pic_select_mode = true;
    LV_LOG_USER("==== Enter pic select mode ====");

    // 显示所有勾选框
    if(pic_grid_cont && lv_obj_is_valid(pic_grid_cont)) {
        uint32_t child_cnt = lv_obj_get_child_cnt(pic_grid_cont);
        for(uint32_t i = 0; i < child_cnt; i++) {
            lv_obj_t *item_btn = lv_obj_get_child(pic_grid_cont, i);
            if(!item_btn || !lv_obj_check_type(item_btn, &lv_btn_class)) continue;

            lv_obj_t *check = NULL;
            uint32_t sub_cnt = lv_obj_get_child_cnt(item_btn);
            for(uint32_t j = 0; j < sub_cnt; j++) {
                lv_obj_t *sub_child = lv_obj_get_child(item_btn, j);
                if(sub_child && lv_obj_check_type(sub_child, &lv_checkbox_class)) {
                    check = sub_child;
                    break;
                }
            }

            if(check) {
                lv_obj_clear_flag(check, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_state(check, LV_STATE_CHECKED);
            }
        }
    }

    // 显示取消/删除按钮
    if(pic_cancel_btn) lv_obj_clear_flag(pic_cancel_btn, LV_OBJ_FLAG_HIDDEN);
    if(pic_delete_btn) {
        lv_obj_clear_flag(pic_delete_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(pic_delete_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_text_color(pic_delete_btn, lv_color_hex(0x888888), LV_PART_MAIN);
        lv_obj_set_style_opa(pic_delete_btn, LV_OPA_60, LV_PART_MAIN);
    }
    if(pic_title_label) lv_label_set_text(pic_title_label, "Picture Cache (多选删除)");
}

static void exit_pic_select_mode(lv_event_t *e)
{
    if(e == NULL || !pic_select_mode) return;
    pic_select_mode = false;
    
    // 强制重置所有选中状态
    memset(pic_selected, 0, sizeof(pic_selected));
    memset(pic_batch_selected_copy, 0, sizeof(pic_batch_selected_copy));

    // 清理勾选框UI
    if(pic_grid_cont && lv_obj_is_valid(pic_grid_cont)) {
        uint32_t child_cnt = lv_obj_get_child_cnt(pic_grid_cont);
        for(uint32_t i = 0; i < child_cnt; i++) {
            lv_obj_t *item = lv_obj_get_child(pic_grid_cont, i);
            if(item && lv_obj_check_type(item, &lv_btn_class)) {
                uint32_t sub_cnt = lv_obj_get_child_cnt(item);
                for(uint32_t j = 0; j < sub_cnt; j++) {
                    lv_obj_t *sub_child = lv_obj_get_child(item, j);
                    if(sub_child && lv_obj_check_type(sub_child, &lv_checkbox_class)) {
                        lv_obj_add_flag(sub_child, LV_OBJ_FLAG_HIDDEN);
                        lv_obj_clear_state(sub_child, LV_STATE_CHECKED);
                    }
                }
            }
        }
    }
    
    // 隐藏按钮
    if(pic_cancel_btn && lv_obj_is_valid(pic_cancel_btn)) 
        lv_obj_add_flag(pic_cancel_btn, LV_OBJ_FLAG_HIDDEN);
    if(pic_delete_btn && lv_obj_is_valid(pic_delete_btn)) 
        lv_obj_add_flag(pic_delete_btn, LV_OBJ_FLAG_HIDDEN);
    if(pic_title_label && lv_obj_is_valid(pic_title_label)) 
        lv_label_set_text(pic_title_label, "Picture Cache");

    LV_LOG_USER("==== Exited pic select mode ====");
}

//刷新图片列表（对齐视频刷新逻辑，动态数量）
static void refresh_pic_list(void)
{
    if(!pic_grid_cont || !lv_obj_is_valid(pic_grid_cont)) {
        return;
    }
    lv_obj_clean(pic_grid_cont);
    current_pic_count = get_valid_pic_count();
    // 禁用弹性滚动
    lv_obj_clear_flag(pic_grid_cont, LV_OBJ_FLAG_SCROLL_ELASTIC);
    
    for(uint8_t i = 0; i < current_pic_count; i++) 
    {        
        item = lv_btn_create(pic_grid_cont);
        lv_obj_set_size(item, 232, 200);
        lv_obj_set_style_bg_color(item, lv_color_hex(0xC0C0C0), LV_PART_MAIN);
        lv_obj_set_style_border_width(item, 1, LV_PART_MAIN);
        lv_obj_set_style_border_color(item, lv_color_hex(0x888888), LV_PART_MAIN);

        // 分配并初始化用户数据（绑定到按钮对象，LVGL会随对象销毁自动释放）
        pic_item_data_t *item_data = (pic_item_data_t *)lv_mem_alloc(sizeof(pic_item_data_t));
        if(item_data == NULL) {
            LV_LOG_WARN("Mem alloc failed for item %d", i);
            lv_obj_del(item);
            continue;
        }
        item_data->index = i;
        item_data->item_btn = item;
        item_data->parent_scr = fc_pic_scr;
        lv_obj_set_user_data(item, item_data); // 绑定到对象

        // 绑定事件（无需传递动态内存，数据从对象获取）
        lv_obj_add_event_cb(item, pic_list_item_click_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(item, enter_pic_select_mode, LV_EVENT_LONG_PRESSED, NULL);

        lv_obj_t *img = create_image_obj(item, pic_path_array[i], 20, 20);
        if(img == NULL) {
            img = lv_label_create(item);
            lv_label_set_text(img, LV_SYMBOL_IMAGE);
            //lv_obj_set_style_text_font(img, &lv_font_montserrat_32, LV_PART_MAIN);
        }
        lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

        // 勾选框创建
        // 勾选框（使用你自己的 ICON_CHECK）
        lv_obj_t *check = lv_checkbox_create(item);
        lv_checkbox_set_text(check, "");
        lv_obj_align(check, LV_ALIGN_TOP_RIGHT, -10, 10);
        lv_obj_set_size(check, 24, 24);

        // 空心圆圈样式
        lv_obj_set_style_border_color(check, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(check, 0, LV_STATE_DEFAULT);
        lv_obj_set_style_radius(check, 100, LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(check, LV_OPA_0, LV_STATE_DEFAULT);

        // 选中时显示你的对勾
        lv_obj_set_style_text_font(check, &my_custom_icon, LV_PART_INDICATOR);
        lv_obj_set_style_text_color(check, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);
        lv_checkbox_set_text(check, ICON_CHECK);

        lv_obj_add_flag(check, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(check, pic_checkbox_click_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
        lv_obj_add_flag(check, LV_OBJ_FLAG_HIDDEN);
    }

    // 重置状态
    memset(pic_selected, 0, sizeof(pic_selected));
    pic_select_mode = false;

    if(pic_cancel_btn && lv_obj_is_valid(pic_cancel_btn)) 
        lv_obj_add_flag(pic_cancel_btn, LV_OBJ_FLAG_HIDDEN);
    if(pic_delete_btn && lv_obj_is_valid(pic_delete_btn)) 
        lv_obj_add_flag(pic_delete_btn, LV_OBJ_FLAG_HIDDEN);
    if(pic_title_label && lv_obj_is_valid(pic_title_label)) 
        lv_label_set_text(pic_title_label, "Picture Cache");

    lv_refr_now(lv_disp_get_default());
}

void pic_list_item_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t *item_btn = lv_event_get_current_target(e); // 取按钮本身，而非点击子元素
    if(!item_btn || !lv_obj_check_type(item_btn, &lv_btn_class)) return;

    // 从按钮对象获取绑定的用户数据（无需手动释放）
    pic_item_data_t *item_data = (pic_item_data_t *)lv_obj_get_user_data(item_btn);
    if(item_data == NULL) {
        LV_LOG_WARN("Item click: no user data found");
        return;
    }

    uint8_t index = item_data->index;
    lv_obj_t *fc_pic_scr = item_data->parent_scr;

    // 多选模式处理
    if(pic_select_mode) 
    {
        // 点击的是勾选框，直接返回（已由checkbox回调处理）
        if(lv_obj_check_type(target, &lv_checkbox_class)) {
            return;
        }

        // 索引最终校验
        if(index >= current_pic_count) {
            LV_LOG_WARN("Item click: index %d out of bounds", index);
            return;
        }

        // 切换选中状态并同步UI
        toggle_pic_selection(index);
        lv_obj_t *check = NULL;
        uint32_t sub_cnt = lv_obj_get_child_cnt(item_btn);
        for(uint32_t j = 0; j < sub_cnt; j++) 
        {
            lv_obj_t *sub_child = lv_obj_get_child(item_btn, j);
            if(sub_child && lv_obj_check_type(sub_child, &lv_checkbox_class)) 
            {
                check = sub_child;
                break;
            }
        }
        
        if(check) {
            pic_selected[index] ? lv_obj_add_state(check, LV_STATE_CHECKED) : lv_obj_clear_state(check, LV_STATE_CHECKED);
        }
        return;
    }

    // 非多选模式：预览逻辑
    if(fc_pic_scr == NULL || !lv_obj_is_valid(fc_pic_scr)) {
        return;
    }
    
    if(index >= MAX_PIC_ITEMS) {
        LV_LOG_WARN("Preview: index %d out of bounds", index);
        return;
    }

    // 更新当前预览索引
    current_preview_index = index;

    // 创建/复用预览页面
    if(pic_preview_scr == NULL) {
        pic_preview_scr = lv_obj_create(NULL);
    } else {
        lv_obj_clean(pic_preview_scr);
    }
    
    lv_style_reset(&file_cache_grad_style);
    lv_style_set_bg_color(&file_cache_grad_style, lv_color_hex(0x888585));
    lv_obj_add_style(pic_preview_scr, &file_cache_grad_style, LV_STATE_DEFAULT);

   //create_common_back_btn(pic_preview_scr, fc_pic_scr);
    lv_obj_t *back_btn = create_container_circle
    (pic_preview_scr, 52, 84, 46,true, lv_color_hex(0xCFCECE), lv_color_hex(0xFFFFFF), 0, LV_OPA_100);
    lv_obj_set_style_pad_all(back_btn, 0, LV_STATE_DEFAULT);
    lv_obj_t *back_btn_img = create_text_label(back_btn, ICON_CHEVORN_LEFT, &my_custom_icon, lv_color_hex(0x888585), 0, 0, LV_OPA_100);
    lv_obj_align(back_btn_img, LV_ALIGN_CENTER, -2, 0);
    lv_obj_add_event_cb(back_btn, pic_preview_back_btn_click_cb, LV_EVENT_CLICKED, fc_pic_scr);

    // 预览图片（全局化以便切换时更新）
    preview_img = lv_img_create(pic_preview_scr);
    lv_obj_set_size(preview_img, lv_pct(80), lv_pct(100));
    lv_obj_align(preview_img, LV_ALIGN_CENTER, 0, 0);
    lv_img_set_src(preview_img, pic_path_array[index]); 
    lv_obj_set_style_radius(preview_img, 12, LV_PART_MAIN);

    // // 左切换按钮
    // lv_obj_t *prev_btn = create_text_label
    // (pic_preview_scr, ICON_CHEVORN_LEFT, &my_custom_icon, lv_color_hex(0xFFFFFF), 52, 84, LV_OPA_100);
    // if(prev_btn) {
    //     lv_obj_add_flag(prev_btn, LV_OBJ_FLAG_CLICKABLE);
    //     lv_obj_align(prev_btn, LV_ALIGN_LEFT_MID, 20, 0);
    //     lv_obj_add_event_cb(prev_btn, preview_prev_cb, LV_EVENT_CLICKED, NULL);
    //     lv_obj_set_style_opa(prev_btn, LV_OPA_80, LV_STATE_PRESSED);
    // }

    // // 右切换按钮
    // lv_obj_t *next_btn = create_text_label
    // (pic_preview_scr, ICON_CHEVORN_RIGHT, &my_custom_icon, lv_color_hex(0xFFFFFF), 52, 84, LV_OPA_100);
    // if(next_btn) {
    //     lv_obj_add_flag(next_btn, LV_OBJ_FLAG_CLICKABLE);
    //     lv_obj_align(next_btn, LV_ALIGN_RIGHT_MID, -20, 0);
    //     lv_obj_add_event_cb(next_btn, preview_next_cb, LV_EVENT_CLICKED, NULL);
    //     lv_obj_set_style_opa(next_btn, LV_OPA_80, LV_STATE_PRESSED);
    // }

    // 删除按钮（预览页）
    lv_obj_t *del_btn = create_container_circle
    (pic_preview_scr, 935, 84, 46,true, lv_color_hex(0xCFCECE), lv_color_hex(0xFFFFFF), 0, LV_OPA_100);
    lv_obj_set_style_pad_all(del_btn, 0, LV_STATE_DEFAULT);
    lv_obj_t *del_btn_img = create_text_label(del_btn, ICON_TRASH, &my_custom_icon, lv_color_hex(0x888585), 0, 0, LV_OPA_100);
    lv_obj_align(del_btn_img, LV_ALIGN_CENTER, 0, 0);

    if(del_btn) {
        lv_obj_add_flag(del_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(del_btn, delete_current_pic_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)index);
        lv_obj_set_style_opa(del_btn, LV_OPA_80, LV_STATE_PRESSED);
    }
    
    update_status_bar_parent(pic_preview_scr);
    lv_scr_load(pic_preview_scr);
}



static void pic_batch_delete_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    memcpy(pic_batch_selected_copy, pic_selected, sizeof(pic_batch_selected_copy));
    delete_current_pic_cb(e);
}
void delete_current_pic_cb(lv_event_t *e)
{
    if(e == NULL) return;

    lv_obj_t *parent_scr;
    uint8_t del_index = 0;
    bool is_preview_page_delete = false;

    if (pic_select_mode && fc_pic_scr && lv_obj_is_valid(fc_pic_scr)) {
        parent_scr = fc_pic_scr;
    } else if (lv_scr_act() != fc_pic_scr) {
        parent_scr = lv_scr_act();
        del_index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
        is_preview_page_delete = true;
        if(del_index < MAX_PIC_ITEMS) {
            memset(pic_batch_selected_copy, 0, sizeof(pic_batch_selected_copy));
            pic_batch_selected_copy[del_index] = true;
        } else {
            LV_LOG_WARN("Preview page delete: index out of bounds");
            return;
        }
    } else {
        parent_scr = lv_scr_act();
    }

    // 复用视频删除弹窗UI，仅修改提示文本
    popup_bg = lv_obj_create(parent_scr);
    lv_obj_set_size(popup_bg, LV_PCT(100), LV_PCT(100));
    lv_obj_align(popup_bg, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(popup_bg, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(popup_bg, LV_OPA_60, LV_PART_MAIN);
    lv_obj_clear_flag(popup_bg, LV_OBJ_FLAG_CLICKABLE);

    confirm_popup = lv_obj_create(parent_scr);
    lv_obj_set_size(confirm_popup, 600, 297);
    lv_obj_align(confirm_popup, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(confirm_popup, lv_color_hex(0xE8F4FD), LV_PART_MAIN);
    lv_obj_set_style_radius(confirm_popup, 16, LV_PART_MAIN);
    lv_obj_set_style_border_opa(confirm_popup, LV_OPA_0, LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(confirm_popup);
    lv_label_set_text(title, "确定删除此图片吗？");
    lv_obj_set_style_text_color(title, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &eques_regular_32, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 40);

    if(!confirm_style_inited) {
        lv_style_init(&confirm_btn_grad_style);
        lv_style_set_bg_color(&confirm_btn_grad_style, lv_color_hex(0x4A6CF7));
        lv_style_set_bg_grad_color(&confirm_btn_grad_style, lv_color_hex(0x7B8DFF));
        lv_style_set_bg_grad_dir(&confirm_btn_grad_style, LV_GRAD_DIR_HOR);
        confirm_style_inited = true;
    }

    lv_obj_t *btn_cont = lv_obj_create(confirm_popup);
    lv_obj_set_size(btn_cont, 200, 120);
    lv_obj_align(btn_cont, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_bg_opa(btn_cont, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(btn_cont, LV_OPA_0, LV_PART_MAIN);
    lv_obj_clear_flag(btn_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(btn_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY);

    lv_obj_t *confirm_btn = lv_btn_create(btn_cont);
    lv_obj_set_size(confirm_btn, 160, 50);
    lv_obj_add_style(confirm_btn, &confirm_btn_grad_style, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(confirm_btn, 8, LV_PART_MAIN);
    lv_obj_set_style_border_opa(confirm_btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_add_event_cb(confirm_btn, pic_delete_confirm_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *confirm_label = lv_label_create(confirm_btn);
    lv_label_set_text(confirm_label, "确定");
    lv_obj_set_style_text_color(confirm_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(confirm_label, &eques_bold_24, LV_PART_MAIN);
    lv_obj_align(confirm_label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *cancel_btn = lv_btn_create(btn_cont);
    lv_obj_set_size(cancel_btn, 160, 50);
    lv_obj_set_style_bg_color(cancel_btn, lv_color_hex(0xE8E8E8), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cancel_btn, lv_color_hex(0xD8D8D8), LV_STATE_PRESSED);
    lv_obj_set_style_radius(cancel_btn, 8, LV_PART_MAIN);
    lv_obj_set_style_border_opa(cancel_btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_add_event_cb(cancel_btn, delete_cancel_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *cancel_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_label, "取消");
    lv_obj_set_style_text_color(cancel_label, lv_color_hex(0x666666), LV_PART_MAIN);
    lv_obj_set_style_text_font(cancel_label, &eques_bold_24, LV_PART_MAIN);
    lv_obj_align(cancel_label, LV_ALIGN_CENTER, 0, 0);
}
// 删除数据源中的图片项（和视频逻辑完全一致）
static void remove_pic_item(uint8_t index)
{
    if(index >= current_pic_count) {
        LV_LOG_WARN("Pic delete failed: index out of bounds");
        return;
    }
    LV_LOG_USER("Removing pic item %d", index+1);
    // 数据前移（核心删除逻辑）
    for(uint8_t i = index; i < current_pic_count - 1; i++) {
        pic_selected[i] = pic_selected[i + 1];
        pic_path_array[i] = pic_path_array[i + 1];
    }
    pic_path_array[current_pic_count - 1] = NULL;
    pic_selected[current_pic_count - 1] = false;
    current_pic_count--;
}
// 图片删除确认回调（完整删除逻辑，对齐视频）
void pic_delete_confirm_cb(lv_event_t *e)
{
    if(e == NULL) return;

    // 销毁弹窗
    if(confirm_popup) { lv_obj_del(confirm_popup); confirm_popup = NULL; }
    if(popup_bg) { lv_obj_del(popup_bg); popup_bg = NULL; }

    // 反向删除（避免索引错乱，和视频完全一样）
    int delete_count = 0;
    LV_LOG_USER("==== Pic batch delete start ====");

    for(int32_t i = current_pic_count - 1; i >= 0; i--) 
    {
        if(pic_batch_selected_copy[i]) 
        {
            LV_LOG_USER("Deleting pic item %d", i+1);
            remove_pic_item(i); // 【关键】调用真实删除函数
            delete_count++;
        }
    }

    // 刷新列表
    refresh_pic_list();

    // 回到图片缓存页
    if(fc_pic_scr && lv_obj_is_valid(fc_pic_scr)) {
        update_status_bar_parent(fc_pic_scr);
        lv_scr_load(fc_pic_scr);
        lv_refr_now(lv_disp_get_default());
    }

    LV_LOG_USER("Pic delete finished: deleted %d items", delete_count);
}
/************************ 多选模式核心回调 ************************/
// 切换单个视频项的选中状态（带完整日志）
static void toggle_video_selection(uint8_t index)
{
    // 1. 前置校验+日志：记录函数调用和基础信息
    LV_LOG_USER("==== toggle_video_selection called ====");
    LV_LOG_USER("Target index: %d (user item %d), total items: %d", 
               index, index+1, current_video_count);
    
    if(index >= current_video_count) {
        LV_LOG_WARN("Toggle failed: index out of bounds");
        return;
    }

    bool state_before = video_selected[index];
    video_selected[index] = !state_before;
    LV_LOG_USER("State changed: %s → %s", 
               state_before ? "selected" : "unselected",
               video_selected[index] ? "selected" : "unselected");

    // 检查是否有选中项，更新删除按钮状态
    bool has_selected = false;
    LV_LOG_USER("Current selected items:");
    for(uint8_t i = 0; i < current_video_count; i++) {
        if(video_selected[i]) {
            LV_LOG_USER("  - Item %d", i+1);
            has_selected = true;
        }
    }

    if(delete_btn && select_mode) {
        lv_obj_clear_flag(delete_btn, LV_OBJ_FLAG_HIDDEN);
        if(has_selected) {
            lv_obj_add_flag(delete_btn, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_text_color(delete_btn, lv_color_hex(0xFF4757), LV_PART_MAIN);
            lv_obj_set_style_opa(delete_btn, LV_OPA_100, LV_PART_MAIN);
        } else {
            lv_obj_clear_flag(delete_btn, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_text_color(delete_btn, lv_color_hex(0x888888), LV_PART_MAIN);
            lv_obj_set_style_opa(delete_btn, LV_OPA_60, LV_PART_MAIN);
        }
    }
    LV_LOG_USER("==== toggle_video_selection finished ====\n");
}

// 勾选框独立点击回调（确保点击勾选框也能切换状态）

static void checkbox_click_cb(lv_event_t *e)
{
    if(e == NULL || !select_mode) return;
    
    lv_obj_t *check = lv_event_get_target(e);
    // 直接从勾选框的用户数据中获取索引
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    
    toggle_video_selection(index);
    
    // 同步勾选框UI
    if(video_selected[index]) {
        lv_obj_add_state(check, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(check, LV_STATE_CHECKED);
    }
    LV_LOG_USER("Checkbox clicked: Item %d selected = %s", index+1, video_selected[index] ? "true" : "false");
}
// 长按进入多选删除模式
static void enter_select_mode(lv_event_t *e)
{
    if(e == NULL || select_mode) {
        LV_LOG_WARN("enter_select_mode: skip (already in select mode)");
        return;
    }

    lv_obj_t *target = lv_event_get_target(e);
    if(!target || !lv_obj_check_type(target, &lv_btn_class)) return;
    
    select_mode = true;
    LV_LOG_USER("==== Enter select mode ====");

    // 显示所有勾选框
    if(video_grid_cont && lv_obj_is_valid(video_grid_cont)) {
        uint32_t child_cnt = lv_obj_get_child_cnt(video_grid_cont);
        for(uint32_t i = 0; i < child_cnt; i++) {
            lv_obj_t *item_btn = lv_obj_get_child(video_grid_cont, i);
            if(!item_btn || !lv_obj_check_type(item_btn, &lv_btn_class)) continue;

            // 遍历找勾选框（不依赖固定索引）
            lv_obj_t *check = NULL;
            uint32_t sub_cnt = lv_obj_get_child_cnt(item_btn);
            for(uint32_t j = 0; j < sub_cnt; j++) {
                lv_obj_t *sub_child = lv_obj_get_child(item_btn, j);
                if(sub_child && lv_obj_check_type(sub_child, &lv_checkbox_class)) {
                    check = sub_child;
                    break;
                }
            }

            if(check) {
                lv_obj_clear_flag(check, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_state(check, LV_STATE_CHECKED);
            }
        }
    }

    // 显示取消/删除按钮
    if(cancel_btn) lv_obj_clear_flag(cancel_btn, LV_OBJ_FLAG_HIDDEN);
    if(delete_btn) {
        lv_obj_clear_flag(delete_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(delete_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_text_color(delete_btn, lv_color_hex(0x888888), LV_PART_MAIN);
        lv_obj_set_style_opa(delete_btn, LV_OPA_60, LV_PART_MAIN);
    }
    if(title_label) lv_label_set_text(title_label, "Video cache (多选删除)");
}

// 退出选择模式
static void exit_select_mode(lv_event_t *e)
{
    if(e == NULL || !select_mode) return;
    select_mode = false;
    
    // 重置选中状态
    memset(video_selected, 0, sizeof(video_selected));
    
    // 隐藏勾选框
    if(video_grid_cont) {
        uint32_t child_cnt = lv_obj_get_child_cnt(video_grid_cont);
        for(uint32_t i = 0; i < child_cnt; i++) {
            lv_obj_t *item = lv_obj_get_child(video_grid_cont, i);
            if(item && lv_obj_check_type(item, &lv_btn_class)) {
                lv_obj_t *check = NULL;
                uint32_t sub_cnt = lv_obj_get_child_cnt(item);
                for(uint32_t j = 0; j < sub_cnt; j++) {
                    lv_obj_t *sub_child = lv_obj_get_child(item, j);
                    if(sub_child && lv_obj_check_type(sub_child, &lv_checkbox_class)) {
                        check = sub_child;
                        break;
                    }
                }
                if(check) {
                    lv_obj_add_flag(check, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_clear_state(check, LV_STATE_CHECKED);
                }
            }
        }
    }
    
    // 隐藏按钮，恢复标题
    if(cancel_btn) lv_obj_add_flag(cancel_btn, LV_OBJ_FLAG_HIDDEN);
    if(delete_btn) lv_obj_add_flag(delete_btn, LV_OBJ_FLAG_HIDDEN);
    if(title_label) lv_label_set_text(title_label, "Video Cache");
}

// 删除数据源中的视频项
static void remove_video_item_from_list(uint8_t index)
{
    if(index >= current_video_count) {
        LV_LOG_WARN("Delete failed: index out of bounds");
        return;
    }

    LV_LOG_USER("Removing item %d: %s", index+1, time_list[index]);
    // 前移数据
    for(uint8_t i = index; i < current_video_count - 1; i++) {
        time_list[i] = time_list[i + 1];
        video_selected[i] = video_selected[i + 1];
    }
    time_list[current_video_count - 1] = NULL;
    video_selected[current_video_count - 1] = false;
    current_video_count--;
}

// 列表项点击回调（选择模式/播放模式）
static void video_list_item_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t *item_btn = lv_event_get_current_target(e); // 取按钮本身，而非点击子元素
    if(!item_btn || !lv_obj_check_type(item_btn, &lv_btn_class)) return;

    // 从按钮对象获取绑定的用户数据（无需手动释放）
    video_item_data_t *item_data = (video_item_data_t *)lv_obj_get_user_data(item_btn);
    if(item_data == NULL) {
        LV_LOG_WARN("Video item click: no user data found");
        return;
    }

    uint8_t index = item_data->index;
    lv_obj_t *fc_video_scr = item_data->parent_scr;

    // 多选模式处理
    if(select_mode) 
    {
        // 点击的是勾选框，直接返回（已由checkbox回调处理）
        if(lv_obj_check_type(target, &lv_checkbox_class)) {
            return;
        }

        // 索引最终校验
        if(index >= current_video_count) {
            LV_LOG_WARN("Video item click: index %d out of bounds", index);
            return;
        }

        // 切换选中状态并同步UI
        toggle_video_selection(index);
        lv_obj_t *check = NULL;
        uint32_t sub_cnt = lv_obj_get_child_cnt(item_btn);
        for(uint32_t j = 0; j < sub_cnt; j++) 
        {
            lv_obj_t *sub_child = lv_obj_get_child(item_btn, j);
            if(sub_child && lv_obj_check_type(sub_child, &lv_checkbox_class)) 
            {
                check = sub_child;
                break;
            }
        }
        
        if(check) {
            video_selected[index] ? lv_obj_add_state(check, LV_STATE_CHECKED) : lv_obj_clear_state(check, LV_STATE_CHECKED);
        }
        return;
    }

    // 非多选模式：播放逻辑
    const char *video_time = time_list[index];
    
    if(video_time == NULL || fc_video_scr == NULL || !lv_obj_is_valid(fc_video_scr)) {
        return;
    }
    
    current_video_path = video_time;
    current_video_sec = 0;
    
    if(fc_video_play_scr == NULL) {
        fc_video_play_scr = lv_obj_create(NULL);
        lv_obj_clear_flag(fc_video_play_scr, LV_OBJ_FLAG_SCROLLABLE);
    } else {
        lv_obj_clean(fc_video_play_scr);
    }

    lv_style_reset(&file_cache_grad_style);
    lv_style_set_bg_color(&file_cache_grad_style, lv_color_hex(0x888585));
    lv_obj_add_style(fc_video_play_scr, &file_cache_grad_style, LV_STATE_DEFAULT);
    //set_common_bg_style(fc_video_play_scr);
    //create_common_back_btn(fc_video_play_scr, fc_video_scr);
    lv_obj_t *back_btn = create_container_circle
    (fc_video_play_scr, 52, 84, 46,true, lv_color_hex(0xCFCECE), lv_color_hex(0xFFFFFF), 0, LV_OPA_100);
    lv_obj_set_style_pad_all(back_btn, 0, LV_STATE_DEFAULT);
    lv_obj_t *back_btn_img = create_text_label(back_btn, ICON_CHEVORN_LEFT, &my_custom_icon, lv_color_hex(0x888585), 0, 0, LV_OPA_100);
    lv_obj_align(back_btn_img, LV_ALIGN_CENTER, -2, 0);
    lv_obj_add_event_cb(back_btn, video_play_back_btn_click_cb, LV_EVENT_CLICKED, fc_video_scr);

    // 删除按钮（播放页）
    lv_obj_t *del_btn = create_container_circle
    (fc_video_play_scr, 935, 84, 46,true, lv_color_hex(0xCFCECE), lv_color_hex(0xFFFFFF), 0, LV_OPA_100);
    lv_obj_set_style_pad_all(del_btn, 0, LV_STATE_DEFAULT);
    lv_obj_t *del_btn_img = create_text_label(del_btn, ICON_TRASH, &my_custom_icon, lv_color_hex(0x888585), 0, 0, LV_OPA_100);
    lv_obj_align(del_btn_img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(del_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(del_btn, LV_ALIGN_TOP_RIGHT, -15, 123);
    lv_obj_add_event_cb(del_btn, delete_current_video_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)index);
    lv_obj_set_style_opa(del_btn, LV_OPA_80, LV_STATE_PRESSED);
    
    
    // 视频显示区域
    lv_obj_t *video_display = lv_img_create(fc_video_play_scr);
    lv_obj_set_size(video_display, lv_pct(50), lv_pct(50));
    lv_obj_align(video_display, LV_ALIGN_TOP_MID, 0, 200);
    lv_img_set_src(video_display, "D:/fc_video_cover.png");
    lv_obj_set_style_radius(video_display, 12, LV_PART_MAIN);
    
    // 进度条
    video_slider = lv_slider_create(fc_video_play_scr);
    lv_obj_set_size(video_slider, lv_pct(80), 4);
    lv_obj_align(video_slider, LV_ALIGN_BOTTOM_MID, 0, -100);
    lv_slider_set_range(video_slider, 0, total_video_sec);
    lv_slider_set_value(video_slider, current_video_sec, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(video_slider, lv_color_hex(0xE0E0E0), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(video_slider, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_bg_color(video_slider, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(video_slider, lv_color_hex(0x00D8C8), LV_PART_KNOB);
    lv_obj_set_style_size(video_slider, 16, LV_PART_KNOB);
    lv_obj_add_event_cb(video_slider, video_slider_changed_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
    // 当前时间标签
    current_time_label = lv_label_create(fc_video_play_scr);
    lv_label_set_text_fmt(current_time_label, "%d:%02d", 0, 0);
    lv_obj_set_style_text_color(current_time_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(current_time_label, &eques_regular_24, LV_PART_MAIN);
    lv_obj_align_to(current_time_label, video_slider, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    
    // 总时长标签
    lv_obj_t *total_time_label = lv_label_create(fc_video_play_scr);
    int total_min = total_video_sec / 60;
    int total_sec = total_video_sec % 60;
    lv_label_set_text_fmt(total_time_label, "%d:%02d", total_min, total_sec);
    lv_obj_set_style_text_color(total_time_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(total_time_label, &eques_regular_24, LV_PART_MAIN);
    lv_obj_align_to(total_time_label, video_slider, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 5);
    
    // 播放/暂停按钮
    lv_obj_t *play_btn = lv_btn_create(fc_video_play_scr);
    lv_obj_set_size(play_btn, 60, 60);
    lv_obj_align_to(play_btn, video_slider, LV_ALIGN_OUT_TOP_MID, 0, 80);
    lv_obj_set_style_bg_color(play_btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(play_btn, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_radius(play_btn, 30, LV_PART_MAIN);
    lv_obj_set_style_border_opa(play_btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_add_event_cb(play_btn, video_play_pause_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *play_label = lv_label_create(play_btn);
    lv_label_set_text(play_label, ICON_PLAY);
    lv_obj_set_style_text_color(play_label, lv_color_hex(0x1A2A48), LV_PART_MAIN);
    lv_obj_set_style_text_font(play_label, &iconfont_icon_20, LV_PART_MAIN);
    lv_obj_align(play_label, LV_ALIGN_CENTER, 0, 0);
    
    update_status_bar_parent(fc_video_play_scr);
    lv_scr_load(fc_video_play_scr);
    
    is_video_playing = false;
    if(video_timer) lv_timer_pause(video_timer);
}

// 批量删除按钮点击回调
static void batch_delete_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    // 复制选中状态到副本
    memcpy(batch_selected_copy, video_selected, sizeof(batch_selected_copy));
    delete_current_video_cb(e);
}

// 刷新视频列表
static void refresh_video_list(void)
{
    if(!video_grid_cont || current_video_count == 0) {
        lv_obj_clean(video_grid_cont);
        return;
    }
    
    // 清空原有UI
    lv_obj_clean(video_grid_cont);
    // 禁用弹性滚动
    lv_obj_clear_flag(video_grid_cont, LV_OBJ_FLAG_SCROLL_ELASTIC);
    // 重新创建列表项
    uint8_t list_cnt = current_video_count;
    int32_t start_y = 10;
    int32_t item_height = 60;
    int32_t item_spacing = 30;
    
    for(uint8_t i = 0; i < list_cnt; i++) 
    {
        lv_obj_t *video_btn = lv_btn_create(video_grid_cont);
        lv_obj_set_size(video_btn, 928, 83);
        lv_obj_align(video_btn, LV_ALIGN_TOP_MID, 0, start_y + i * (item_height + item_spacing));
        lv_obj_add_flag(video_btn, LV_OBJ_FLAG_CLICKABLE);
        
        lv_obj_set_style_bg_color(video_btn, lv_color_hex(0x1A2A48), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(video_btn, lv_color_hex(0x2E4B7D), LV_STATE_PRESSED);
        lv_obj_set_style_radius(video_btn, 12, LV_PART_MAIN);
        lv_obj_set_style_border_opa(video_btn, LV_OPA_0, LV_PART_MAIN);

        // 分配并初始化用户数据（绑定到按钮对象，LVGL会随对象销毁自动释放）
        video_item_data_t *item_data = (video_item_data_t *)lv_mem_alloc(sizeof(video_item_data_t));
        if(item_data == NULL) {
            LV_LOG_WARN("Mem alloc failed for video item %d", i);
            lv_obj_del(video_btn);
            continue;
        }
        item_data->index = i;
        item_data->item_btn = video_btn;
        item_data->parent_scr = fc_video_scr;
        lv_obj_set_user_data(video_btn, item_data); // 绑定到对象

        // 绑定事件
        lv_obj_add_event_cb(video_btn, video_list_item_click_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(video_btn, enter_select_mode, LV_EVENT_LONG_PRESSED, NULL);

        // 视频图标
        lv_obj_t *video_icon = create_text_label(video_btn, ICON_VIDEO, &iconfont_icon, lv_color_hex(0xFFFFFF), 20, 20, LV_OPA_100);
        lv_obj_align(video_icon, LV_ALIGN_LEFT_MID, 20, 0);

        // 时间标签
        lv_obj_t *video_label = lv_label_create(video_btn);
        lv_label_set_text(video_label, time_list[i]);
        lv_obj_set_style_text_color(video_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(video_label, &eques_regular_36, LV_PART_MAIN);
        lv_obj_align_to(video_label, video_icon, LV_ALIGN_OUT_RIGHT_MID, 20, 0);

        // 勾选框
        // 勾选框（使用你自己的 ICON_CHECK）
        lv_obj_t *check = lv_checkbox_create(video_btn);
        lv_checkbox_set_text(check, "");
        lv_obj_align(check, LV_ALIGN_RIGHT_MID, -20, 0);
        lv_obj_set_size(check, 24, 24);

        // 样式：空心圆圈
        lv_obj_set_style_border_color(check, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(check, 0, LV_STATE_DEFAULT);
        lv_obj_set_style_radius(check, 12, LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(check, LV_OPA_0, LV_STATE_DEFAULT);

        // 选中时显示你的图标
        lv_obj_set_style_text_font(check, &my_custom_icon, LV_PART_INDICATOR);
        lv_obj_set_style_text_color(check, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);
        lv_checkbox_set_text(check, ICON_CHECK);  // 这里用你的对勾图标

        // 关键修复：绑定点击事件，并将索引i作为用户数据直接传递
        lv_obj_add_flag(check, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(check, checkbox_click_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
        lv_obj_add_flag(check, LV_OBJ_FLAG_HIDDEN);
    }
    
    // 重置状态
    memset(video_selected, 0, sizeof(video_selected));
    select_mode = false;
    
    // 隐藏按钮，恢复标题
    if(cancel_btn) lv_obj_add_flag(cancel_btn, LV_OBJ_FLAG_HIDDEN);
    if(delete_btn) lv_obj_add_flag(delete_btn, LV_OBJ_FLAG_HIDDEN);
    if(title_label) lv_label_set_text(title_label, "Video Cache");
    
    lv_refr_now(lv_disp_get_default());
}
/************************ 子页面回调函数 ************************/


// 视频缓存按钮回调
void video_cache_btn_cb(lv_event_t *e)
{
    LV_LOG_USER("video cache key");
    if(e == NULL) return;

    lv_obj_t *file_cache_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(file_cache_scr == NULL) return;
    ui_fc_video_create(file_cache_scr);
}

// 图片缓存按钮回调
void pic_cache_btn_cb(lv_event_t *e)
{
    LV_LOG_USER("picture cache key");
    if(e == NULL) return;

    lv_obj_t *file_cache_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(file_cache_scr == NULL) return;
    ui_fc_pic_create(file_cache_scr);
}

/************************ 子页面创建函数 ************************/


// 视频缓存页面创建（核心：多选功能）
void ui_fc_video_create(lv_obj_t *file_cache_scr)
{
    init_file_cache_styles();
    if(file_cache_scr == NULL) return;

    // 创建/复用视频缓存页
    if(fc_video_scr == NULL) {
        fc_video_scr = lv_obj_create(NULL);
        lv_obj_clear_flag(fc_video_scr, LV_OBJ_FLAG_SCROLLABLE);
    } else {
        lv_obj_clean(fc_video_scr);
    }

    // 重置选择状态
    memset(video_selected, 0, sizeof(video_selected));
    select_mode = false;

    // 设置背景
    set_common_bg_style(fc_video_scr);

    // 返回按钮
    create_common_back_btn(fc_video_scr, file_cache_scr);

    // 取消按钮（左上角，初始隐藏）
    cancel_btn = lv_label_create(fc_video_scr);
    lv_label_set_text(cancel_btn, "取消");
    lv_obj_set_style_text_color(cancel_btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(cancel_btn, &eques_regular_24, LV_PART_MAIN);
    lv_obj_align(cancel_btn, LV_ALIGN_TOP_LEFT, 80, 123);
    lv_obj_add_flag(cancel_btn, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(cancel_btn, exit_select_mode, LV_EVENT_CLICKED, NULL);

    // 页面标题
    create_text_label(fc_video_scr, "视频缓存", &eques_bold_36, lv_color_hex(0xFFFFFF), 83, 83,LV_OPA_100);

    // 删除按钮（右上角，初始隐藏）
    delete_btn = lv_label_create(fc_video_scr);
    lv_label_set_text(delete_btn, "删除");
    lv_obj_set_style_text_color(delete_btn, lv_color_hex(0xFF4757), LV_PART_MAIN);
    lv_obj_set_style_text_font(delete_btn, &eques_regular_24, LV_PART_MAIN);
    lv_obj_align(delete_btn, LV_ALIGN_TOP_RIGHT, -15, 123);
    lv_obj_add_flag(delete_btn, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(delete_btn, batch_delete_btn_click_cb, LV_EVENT_CLICKED, NULL);

    // 视频列表容器（全局化）
    video_grid_cont = lv_obj_create(fc_video_scr);
    lv_obj_set_size(video_grid_cont, lv_pct(98), lv_pct(50));
    lv_obj_align(video_grid_cont, LV_ALIGN_TOP_MID, 0, 150);
    lv_obj_set_style_bg_opa(video_grid_cont, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(video_grid_cont, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_outline_opa(video_grid_cont, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(video_grid_cont, LV_OPA_TRANSP, LV_PART_MAIN);    
    lv_obj_set_style_pad_all(video_grid_cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_row(video_grid_cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(video_grid_cont, 0, LV_PART_MAIN);
    
    // 调用刷新函数创建列表（复用逻辑，避免重复代码）
    refresh_video_list();

    update_status_bar_parent(fc_video_scr);
    lv_scr_load(fc_video_scr);
}

// 图片缓存页面创建
void ui_fc_pic_create(lv_obj_t *file_cache_scr)
{
    init_file_cache_styles();
    if(file_cache_scr == NULL || !lv_obj_is_valid(file_cache_scr)) return;
    fc_pic_scr = lv_obj_create(NULL);
    set_common_bg_style(fc_pic_scr);
    create_common_back_btn(fc_pic_scr, file_cache_scr);

    // 取消按钮
    pic_cancel_btn = lv_label_create(fc_pic_scr);
    lv_label_set_text(pic_cancel_btn, "取消");
    lv_obj_set_style_text_color(pic_cancel_btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(pic_cancel_btn, &eques_regular_24, LV_PART_MAIN);
    lv_obj_align(pic_cancel_btn, LV_ALIGN_TOP_LEFT, 80, 123);
    lv_obj_add_flag(pic_cancel_btn, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(pic_cancel_btn, exit_pic_select_mode, LV_EVENT_CLICKED, NULL);

    // 标题
    create_text_label(fc_pic_scr, "图片缓存", &eques_bold_36, lv_color_hex(0xFFFFFF), 83, 83,LV_OPA_100);

    // 删除按钮
    pic_delete_btn = lv_label_create(fc_pic_scr);
    lv_label_set_text(pic_delete_btn, "删除");
    lv_obj_set_style_text_color(pic_delete_btn, lv_color_hex(0xFF4757), LV_PART_MAIN);
    lv_obj_set_style_text_font(pic_delete_btn, &eques_regular_24, LV_PART_MAIN);
    lv_obj_align(pic_delete_btn, LV_ALIGN_TOP_RIGHT, -15, 123);
    lv_obj_add_flag(pic_delete_btn, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(pic_delete_btn, pic_batch_delete_btn_click_cb, LV_EVENT_CLICKED, NULL);

    // 列表容器
    pic_grid_cont = lv_obj_create(fc_pic_scr);
    lv_obj_set_size(pic_grid_cont, lv_pct(100), lv_pct(60));
    lv_obj_align(pic_grid_cont, LV_ALIGN_TOP_MID, 0, 150);
    lv_obj_set_style_bg_opa(pic_grid_cont, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(pic_grid_cont, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_outline_opa(pic_grid_cont, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(pic_grid_cont, LV_OPA_TRANSP, LV_PART_MAIN);    
    lv_obj_set_style_pad_all(pic_grid_cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_row(pic_grid_cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(pic_grid_cont, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(pic_grid_cont, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(pic_grid_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // 调用刷新函数创建列表（复用逻辑，避免重复代码）
    refresh_pic_list();

    update_status_bar_parent(fc_pic_scr);
    lv_scr_load(fc_pic_scr);
}


/************************ 文件缓存主页面创建 ************************/
void ui_file_cache_create(lv_obj_t *homepage_scr)
{
    init_file_cache_styles();
    if(homepage_scr == NULL) return;

    if(file_cache_scr == NULL) {
        file_cache_scr = lv_obj_create(NULL);
    } else {
        lv_obj_clean(file_cache_scr);
    }

    set_common_bg_style(file_cache_scr);

    // 返回按钮
    lv_obj_t *back_btn = create_text_label
    (file_cache_scr, ICON_CHEVORN_LEFT, &my_custom_icon, lv_color_hex(0xFFFFFF), 52, 84, LV_OPA_100);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn, file_cache_back_btn_click_cb, LV_EVENT_CLICKED, homepage_scr);

    // 标题
    create_text_label(file_cache_scr, "文件缓存", &eques_bold_36, lv_color_hex(0xFFFFFF), 83, 80,LV_OPA_100);

    // 云端缓存按钮
    lv_obj_t *cloud_con = create_container(file_cache_scr,48,150,928,180,lv_color_hex(0x192A46), LV_OPA_100, 6,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(cloud_con, 0, LV_STATE_DEFAULT);

    lv_obj_t *cloud_img = create_text_label(file_cache_scr, ICON_CLOUD, &iconfont_icon_90, lv_color_hex(0xFFFFFF), 350, 200, LV_OPA_100);
    lv_obj_set_style_bg_opa(cloud_img, LV_OPA_0, LV_STATE_DEFAULT);
    create_text_label(file_cache_scr, "云存储服务", &eques_regular_36, lv_color_hex(0xFFFFFF), 526, 222, LV_OPA_100);

    // lv_obj_t *cloud_label = lv_label_create(cloud_con);
    // lv_label_set_text(cloud_label, "Cloud Storage Service");
    // lv_obj_set_style_text_color(cloud_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    // lv_obj_set_style_text_font(cloud_label, &eques_regular_24, LV_PART_MAIN);
    // lv_obj_align_to(cloud_label, cloud_con, LV_ALIGN_OUT_BOTTOM_MID, 0, 12);

    // 视频缓存按钮
    lv_obj_t *video_btn = lv_btn_create(file_cache_scr);
    lv_obj_set_size(video_btn, 928, 83);
    lv_obj_align_to(video_btn, cloud_con, LV_ALIGN_OUT_BOTTOM_MID, 0, 12);
    lv_obj_set_style_bg_color(video_btn, lv_color_hex(0x1A2A48), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(video_btn, lv_color_hex(0x2E4B7D), LV_STATE_PRESSED);
    lv_obj_set_style_radius(video_btn, 12, LV_PART_MAIN);
    lv_obj_set_style_border_opa(video_btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_add_event_cb(video_btn, video_cache_btn_cb, LV_EVENT_CLICKED, file_cache_scr);

    lv_obj_t *video_icon = create_text_label(video_btn, ICON_VIDEO, &iconfont_icon, lv_color_hex(0xFFFFFF), 20, 20, LV_OPA_100);
    lv_obj_align(video_icon, LV_ALIGN_LEFT_MID, 20, 0);

    lv_obj_t *video_label = lv_label_create(video_btn);
    lv_label_set_text(video_label, "视频缓存");
    lv_obj_set_style_text_color(video_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(video_label, &eques_regular_36, LV_PART_MAIN);
    lv_obj_align_to(video_label, video_icon, LV_ALIGN_OUT_RIGHT_MID, 20, 0);

    create_text_label(file_cache_scr, ICON_CHEVORN_RIGHT, &my_custom_icon, lv_color_hex(0xC4C4C4), 935, 370, LV_OPA_100);

    // 图片缓存按钮
    lv_obj_t *pic_btn = lv_btn_create(file_cache_scr);
    lv_obj_set_size(pic_btn, 928, 83);
    lv_obj_align_to(pic_btn, video_btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 12);
    lv_obj_set_style_bg_color(pic_btn, lv_color_hex(0x1A2A48), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(pic_btn, lv_color_hex(0x2E4B7D), LV_STATE_PRESSED);
    lv_obj_set_style_radius(pic_btn, 12, LV_PART_MAIN);
    lv_obj_set_style_border_opa(pic_btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_add_event_cb(pic_btn, pic_cache_btn_cb, LV_EVENT_CLICKED, file_cache_scr);

    lv_obj_t *pic_icon = create_text_label(pic_btn, ICON_PICTURE, &my_custom_icon, lv_color_hex(0xFFFFFF), 20, 20, LV_OPA_100);
    lv_obj_align(pic_icon, LV_ALIGN_LEFT_MID, 20, 0);

    lv_obj_t *pic_label = lv_label_create(pic_btn);
    lv_label_set_text(pic_label, "图片缓存");
    lv_obj_set_style_text_color(pic_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(pic_label, &eques_regular_36, LV_PART_MAIN);
    lv_obj_align_to(pic_label, pic_icon, LV_ALIGN_OUT_RIGHT_MID, 20, 0);

    create_text_label(file_cache_scr, ICON_CHEVORN_RIGHT, &my_custom_icon, lv_color_hex(0xC4C4C4), 935, 465, LV_OPA_100);

    // 底部提示
    create_text_label(file_cache_scr, "若文件没有上传云端，只可保留三天", &eques_regular_24, lv_color_hex(0xCDCDCD), 48, 527, LV_OPA_100);

    update_status_bar_parent(file_cache_scr);
    lv_scr_load(file_cache_scr);
}

/************************ 视频播放页核心回调 ************************/
// 播放定时器回调
static void video_timer_cb(lv_timer_t *timer)
{
    if(!is_video_playing || video_slider == NULL || current_time_label == NULL) return;
    
    current_video_sec++;
    if(current_video_sec >= total_video_sec) {
        current_video_sec = 0;
        is_video_playing = false;
        lv_timer_pause(video_timer);
        lv_obj_t *play_btn = (lv_obj_t *)timer->user_data; 
        lv_obj_t *play_label = lv_obj_get_child(play_btn, 0);
        if(play_label) lv_label_set_text(play_label, ICON_PLAY);
    }
    
    lv_slider_set_value(video_slider, current_video_sec, LV_ANIM_ON);
    int min = current_video_sec / 60;
    int sec = current_video_sec % 60;
    lv_label_set_text_fmt(current_time_label, "%d:%02d", min, sec);
}

// 播放/暂停回调
void video_play_pause_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    lv_obj_t *play_btn = lv_event_get_target(e);
    lv_obj_t *play_label = lv_obj_get_child(play_btn, 0);
    if(play_label == NULL) return;

    is_video_playing = !is_video_playing;
    if(is_video_playing) {
        lv_label_set_text(play_label, ICON_PAUSE);
        if(video_timer == NULL) {
            video_timer = lv_timer_create(video_timer_cb, 1000, play_btn);
        } else {
            lv_timer_resume(video_timer);
        }
        LV_LOG_USER("Start playing video: %s", current_video_path ? current_video_path : "unknown");
    } else {
        lv_label_set_text(play_label, ICON_PLAY);
        if(video_timer) lv_timer_pause(video_timer);
        LV_LOG_USER("Pause video");
    }
}

// 进度条拖动回调
void video_slider_changed_cb(lv_event_t *e)
{
    if(e == NULL || current_time_label == NULL) return;
    
    current_video_sec = lv_slider_get_value(video_slider);
    int min = current_video_sec / 60;
    int sec = current_video_sec % 60;
    lv_label_set_text_fmt(current_time_label, "%d:%02d", min, sec);
}

/************************ 弹窗相关 ************************/
// 弹窗确认回调（批量删除）
static void delete_confirm_cb(lv_event_t *e)
{
    if(e == NULL) return;

    // 销毁弹窗
    if(confirm_popup) { lv_obj_del(confirm_popup); confirm_popup = NULL; }
    if(popup_bg) { lv_obj_del(popup_bg); popup_bg = NULL; }

    // 停止播放
    if(video_timer) {
        lv_timer_del(video_timer);
        video_timer = NULL;
    }
    is_video_playing = false;
    current_video_sec = 0;
    current_video_path = NULL;

    // 反向删除选中项
    int delete_count = 0;
    LV_LOG_USER("==== Batch delete start ====");
    for(int32_t i = current_video_count - 1; i >= 0; i--) {
        if(batch_selected_copy[i]) {
            LV_LOG_USER("Deleting item %d: %s", i+1, time_list[i]);
            remove_video_item_from_list(i);
            delete_count++;
        }
    }

    // 刷新列表
    refresh_video_list();

    // 回到视频缓存页
    if(fc_video_scr && lv_obj_is_valid(fc_video_scr)) {
        update_status_bar_parent(fc_video_scr);
        lv_scr_load(fc_video_scr);
        lv_refr_now(lv_disp_get_default());
    }
    LV_LOG_USER("Batch delete finished: deleted %d items", delete_count);
}

// 弹窗取消回调
void delete_cancel_cb(lv_event_t *e)
{
    if(e == NULL) return;
    if(confirm_popup) lv_obj_del(confirm_popup);
    if(popup_bg) lv_obj_del(popup_bg);
    confirm_popup = NULL;
    popup_bg = NULL;
}

// 删除按钮回调（淡蓝色弹窗）
// 删除按钮回调（淡蓝色弹窗）
void delete_current_video_cb(lv_event_t *e)
{
    if(e == NULL) return;

    // 确定弹窗父对象
    lv_obj_t *parent_scr;
    uint8_t del_index = 0;
    bool is_play_page_delete = false; // 标记是否为播放页删除

    if (select_mode && fc_video_scr && lv_obj_is_valid(fc_video_scr)) {
        parent_scr = fc_video_scr;
    } else if (fc_video_play_scr && lv_obj_is_valid(fc_video_play_scr)) {
        parent_scr = fc_video_play_scr;
        // 【修复点2】强化索引校验，避免越界
        del_index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
        is_play_page_delete = true;
        if(del_index < current_video_count) {
            memset(batch_selected_copy, 0, sizeof(batch_selected_copy)); // 清空原有状态
            batch_selected_copy[del_index] = true;
            LV_LOG_USER("Play page delete: mark index %d for deletion", del_index+1);
        } else {
            LV_LOG_WARN("Play page delete: index %d out of bounds", del_index+1);
            return; // 索引无效直接返回，不弹弹窗
        }
    } else {
        parent_scr = lv_scr_act();
    }

    // 全屏背景
    popup_bg = lv_obj_create(parent_scr);
    lv_obj_set_size(popup_bg, LV_PCT(100), LV_PCT(100));
    lv_obj_align(popup_bg, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(popup_bg, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(popup_bg, LV_OPA_60, LV_PART_MAIN);
    lv_obj_clear_flag(popup_bg, LV_OBJ_FLAG_CLICKABLE);

    // 淡蓝色弹窗
    confirm_popup = lv_obj_create(parent_scr);
    lv_obj_set_size(confirm_popup, 600, 297);
    lv_obj_align(confirm_popup, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(confirm_popup, lv_color_hex(0xE8F4FD), LV_PART_MAIN);
    lv_obj_set_style_radius(confirm_popup, 16, LV_PART_MAIN);
    lv_obj_set_style_border_opa(confirm_popup, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(confirm_popup, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(confirm_popup, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(confirm_popup, 8, LV_PART_MAIN);

    // 弹窗标题
    lv_obj_t *title = lv_label_create(confirm_popup);
    lv_label_set_text(title, "确定删除视频吗？");
    lv_obj_set_style_text_color(title, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &eques_regular_32, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 40);

    // 按钮渐变样式
    if(!confirm_style_inited) {
        lv_style_init(&confirm_btn_grad_style);
        lv_style_set_bg_color(&confirm_btn_grad_style, lv_color_hex(0x4A6CF7));
        lv_style_set_bg_grad_color(&confirm_btn_grad_style, lv_color_hex(0x7B8DFF));
        lv_style_set_bg_grad_dir(&confirm_btn_grad_style, LV_GRAD_DIR_HOR);
        confirm_style_inited = true;
    }

    // 按钮容器
    lv_obj_t *btn_cont = lv_obj_create(confirm_popup);
    lv_obj_set_size(btn_cont, 200, 120);
    lv_obj_align(btn_cont, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_bg_opa(btn_cont, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(btn_cont, LV_OPA_0, LV_PART_MAIN);
    lv_obj_clear_flag(btn_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(btn_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY);

    // 确认按钮
    lv_obj_t *confirm_btn = lv_btn_create(btn_cont);
    lv_obj_set_size(confirm_btn, 160, 50);
    lv_obj_add_style(confirm_btn, &confirm_btn_grad_style, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(confirm_btn, 8, LV_PART_MAIN);
    lv_obj_set_style_border_opa(confirm_btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_add_event_cb(confirm_btn, delete_confirm_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *confirm_label = lv_label_create(confirm_btn);
    lv_label_set_text(confirm_label, "确定");
    lv_obj_set_style_text_color(confirm_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(confirm_label, &eques_bold_24, LV_PART_MAIN);
    lv_obj_align(confirm_label, LV_ALIGN_CENTER, 0, 0);

    // 取消按钮
    lv_obj_t *cancel_btn = lv_btn_create(btn_cont);
    lv_obj_set_size(cancel_btn, 160, 50);
    lv_obj_set_style_bg_color(cancel_btn, lv_color_hex(0xE8E8E8), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cancel_btn, lv_color_hex(0xD8D8D8), LV_STATE_PRESSED);
    lv_obj_set_style_radius(cancel_btn, 8, LV_PART_MAIN);
    lv_obj_set_style_border_opa(cancel_btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_add_event_cb(cancel_btn, delete_cancel_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *cancel_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_label, "取消");
    lv_obj_set_style_text_color(cancel_label, lv_color_hex(0x666666), LV_PART_MAIN);
    lv_obj_set_style_text_font(cancel_label, &eques_regular_24, LV_PART_MAIN);
    lv_obj_align(cancel_label, LV_ALIGN_CENTER, 0, 0);
}

extern void lv_homepage(void);
extern void destroy_homepage(void);
void file_cache_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;

    lv_obj_t *homepage_scr_temp = (lv_obj_t *)lv_event_get_user_data(e);
    if(homepage_scr_temp == NULL) {
        LV_LOG_WARN("file_cache_btn_click_cb: homepage_scr is NULL!");
        return;
    }

    // 创建文件缓存界面
    ui_file_cache_create(homepage_scr_temp);
    // 更新状态栏
    
    // 销毁主页
    destroy_homepage();
    update_status_bar_parent(file_cache_scr);
    LV_LOG_WARN("file_cache_btn_click_cb: Destroy the homepage and create the file_cache interface");
}
// 文件缓存界面返回
void file_cache_back_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;

    lv_obj_t *current_del_scr = lv_disp_get_scr_act(NULL);

    if(!lv_obj_is_valid(current_del_scr)) return;

    // 当前显示的是文件缓存界面 → 重建主页并销毁当前界面
    if(current_del_scr == file_cache_scr) {
        lv_homepage();                      // 重建主页
        lv_obj_del(current_del_scr);        // 销毁文件缓存界面
        file_cache_scr = NULL;            // 指针置空
        return;
    }
}
