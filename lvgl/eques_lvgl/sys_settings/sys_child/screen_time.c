#include "screen_time.h"
#include "../lv_sys_settings.h"  // 引用上级主模块头文件

// 子模块内部全局变量（static隐藏，仅本文件可见）
static lv_obj_t *light_time_scr = NULL; 
screen_off_time_t g_screen_off_time = SCREEN_OFF_15S; // 默认15s
const char *g_time_strs[] = {"10s", "15s", "30s", "1分钟", "5分钟", "10分钟"};
static lv_obj_t *g_time_view_label = NULL; // 主页面的显示标签
static lv_obj_t *g_time_checkboxes[SCREEN_OFF_MAX] = {NULL}; // 复选框数组
void screen_time_back_btn_click_cb(lv_event_t *e);
// 销毁亮屏时间界面 → 零内存泄漏
static void light_time_destroy(void);
// 子模块内部回调函数声明
static void light_time_checkbox_cb(lv_event_t *e);
static void time_container_click_cb(lv_event_t *e);

lv_style_t screen_time_grad_style;
// 子模块初始化：绑定主页面显示标签
// 1. 优化原有 init 函数：增加重置逻辑（可选，也可单独写）
void screen_time_init(lv_obj_t *display_label) {
    g_time_view_label = display_label;
    // 恢复出厂设置时，强制重置为默认值 15s
    //g_screen_off_time = SCREEN_OFF_15S; // 关键：重置核心变量
    if (g_time_view_label != NULL) {
        lv_label_set_text(g_time_view_label, g_time_strs[g_screen_off_time]);
    }

   // screen_time_update_display_label();
}

// 恢复出厂设置专用重置函数
void screen_time_reset_to_default(void) {
    // 第一步：重置核心变量为默认值（15s）
    g_screen_off_time = SCREEN_OFF_15S;

    // 第二步：更新主页面显示标签
    if (g_time_view_label != NULL && lv_obj_is_valid(g_time_view_label)) {
        lv_label_set_text(g_time_view_label, g_time_strs[SCREEN_OFF_15S]);
    }

    // 第三步：同步更新子页面的复选框选中状态（关键！）
    for (int i = 0; i < SCREEN_OFF_MAX; i++) {
        if (g_time_checkboxes[i] == NULL) continue;
        if (!lv_obj_is_valid(g_time_checkboxes[i])) continue;
        
        if (i == SCREEN_OFF_15S) {
            // 选中默认项（15s）
            lv_obj_add_state(g_time_checkboxes[i], LV_STATE_CHECKED);
        } else {
            // 取消其他项选中
            lv_obj_clear_state(g_time_checkboxes[i], LV_STATE_CHECKED);
        }
    }
    LV_LOG_USER("Screen time reset to default: 15s");
    screen_time_update_display_label();
}


// 获取当前亮屏时间字符串
const char *screen_time_get_current_str(void) {
    return g_time_strs[g_screen_off_time];
}

// 复选框互斥选择回调（原始逻辑完全保留）
static void light_time_checkbox_cb(lv_event_t *e)
{
    if(e == NULL) return;
    int clicked_idx = (int)(intptr_t)lv_event_get_user_data(e);

    // 遍历所有复选框实现互斥单选
    for(int i = 0; i < SCREEN_OFF_MAX; i++) {
        if(g_time_checkboxes[i] == NULL) continue;
        
        if(i == clicked_idx) {
            lv_obj_add_state(g_time_checkboxes[i], LV_STATE_CHECKED);
            g_screen_off_time = (screen_off_time_t)i;
            // 更新主页面显示
            if(g_time_view_label != NULL) {
                lv_label_set_text(g_time_view_label, g_time_strs[i]);
            }
            //screen_time_update_display_label();
        } else {
            lv_obj_clear_state(g_time_checkboxes[i], LV_STATE_CHECKED);
        }
    }
}

// 容器点击回调：触发对应复选框的选择
static void time_container_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    int idx = (int)lv_event_get_user_data(e);
    
    if(idx >= 0 && idx < SCREEN_OFF_MAX && g_time_checkboxes[idx] != NULL) {
        // 模拟点击复选框
        lv_event_send(g_time_checkboxes[idx], LV_EVENT_VALUE_CHANGED, NULL);
    }
}

// 创建亮屏时间设置子页面（原始逻辑完全保留）
void ui_screen_time_settings_create(lv_obj_t *homepage_scr)
{
    
    light_time_scr = lv_obj_create(NULL);  
    
    // 2. 应用背景样式（复用主模块的渐变样式）
    lv_style_reset(&screen_time_grad_style);
    lv_style_set_bg_color(&screen_time_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&screen_time_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&screen_time_grad_style, LV_GRAD_DIR_VER);
    lv_obj_add_style(light_time_scr, &screen_time_grad_style, LV_STATE_DEFAULT);

    // 3. 添加标题（"亮屏时间"）
    create_text_label(light_time_scr, "亮屏时间", &eques_bold_36, lv_color_hex(0xFFFFFF), 83, 80, LV_OPA_100);

    // 系统时间设置容器1
    lv_obj_t *sys_time_con1 = create_container
    (light_time_scr, 48, 150, 928, 83, lv_color_hex(0x192A46), LV_OPA_100, 6, lv_color_hex(0x2E4B7D), 0, LV_OPA_0);
    if(sys_time_con1) {
        lv_obj_add_flag(sys_time_con1, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(sys_time_con1, LV_OPA_70, LV_STATE_PRESSED);
    }

    // 系统时间设置容器2
    lv_obj_t *sys_time_con2 = create_container
    (light_time_scr, 48, 237, 928, 83, lv_color_hex(0x192A46), LV_OPA_100, 6, lv_color_hex(0x2E4B7D), 0, LV_OPA_0);
    if(sys_time_con2) {
        lv_obj_add_flag(sys_time_con2, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(sys_time_con2, LV_OPA_70, LV_STATE_PRESSED);
    }

    // 系统时间设置容器3
    lv_obj_t *sys_time_con3 = create_container
    (light_time_scr, 48, 324, 928, 83, lv_color_hex(0x192A46), LV_OPA_100, 6, lv_color_hex(0x2E4B7D), 0, LV_OPA_0);
    if(sys_time_con3) {
        lv_obj_add_flag(sys_time_con3, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(sys_time_con3, LV_OPA_70, LV_STATE_PRESSED);
    }

    // 系统时间设置容器4
    lv_obj_t *sys_time_con4 = create_container
    (light_time_scr, 48, 411, 928, 83, lv_color_hex(0x192A46), LV_OPA_100, 6, lv_color_hex(0x2E4B7D), 0, LV_OPA_0);
    if(sys_time_con4) {
        lv_obj_add_flag(sys_time_con4, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(sys_time_con4, LV_OPA_70, LV_STATE_PRESSED);
    }

    // 系统时间设置容器5
    lv_obj_t *sys_time_con5 = create_container
    (light_time_scr, 48, 498, 928, 83, lv_color_hex(0x192A46), LV_OPA_100, 6, lv_color_hex(0x2E4B7D), 0, LV_OPA_0);
    if(sys_time_con5) {
        lv_obj_add_flag(sys_time_con5, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(sys_time_con5, LV_OPA_70, LV_STATE_PRESSED);
    }

    // 系统时间设置容器6
    lv_obj_t *sys_time_con6 = create_container
    (light_time_scr, 48, 585, 928, 83, lv_color_hex(0x192A46), LV_OPA_100, 6, lv_color_hex(0x2E4B7D), 0, LV_OPA_0);
    if(sys_time_con6) {
        lv_obj_add_flag(sys_time_con6, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(sys_time_con6, LV_OPA_70, LV_STATE_PRESSED);
    }

    // 给每个容器绑定点击事件
    lv_obj_add_event_cb(sys_time_con1, time_container_click_cb, LV_EVENT_CLICKED, (void *)(long)0);
    lv_obj_add_event_cb(sys_time_con2, time_container_click_cb, LV_EVENT_CLICKED, (void *)(long)1);
    lv_obj_add_event_cb(sys_time_con3, time_container_click_cb, LV_EVENT_CLICKED, (void *)(long)2);
    lv_obj_add_event_cb(sys_time_con4, time_container_click_cb, LV_EVENT_CLICKED, (void *)(long)3);
    lv_obj_add_event_cb(sys_time_con5, time_container_click_cb, LV_EVENT_CLICKED, (void *)(long)4);
    lv_obj_add_event_cb(sys_time_con6, time_container_click_cb, LV_EVENT_CLICKED, (void *)(long)5);

    // 4. 创建多个复选框（模拟单选按钮）
    for(int i = 0; i < SCREEN_OFF_MAX; i++) {
        // 1. 先获取对应的容器（sys_time_con1 ~ sys_time_con6）
        lv_obj_t *parent_con;
        switch(i) {
            case 0: parent_con = sys_time_con1; break;
            case 1: parent_con = sys_time_con2; break;
            case 2: parent_con = sys_time_con3; break;
            case 3: parent_con = sys_time_con4; break;
            case 4: parent_con = sys_time_con5; break;
            case 5: parent_con = sys_time_con6; break;
            default: parent_con = light_time_scr; // 兜底
        }

        // 2. 创建复选框，父对象是容器
        g_time_checkboxes[i] = lv_checkbox_create(parent_con);

        // 设置文字
        lv_checkbox_set_text(g_time_checkboxes[i], g_time_strs[i]);
        lv_obj_set_align(g_time_checkboxes[i], LV_ALIGN_LEFT_MID);

        lv_obj_set_size(g_time_checkboxes[i], LV_SIZE_CONTENT, LV_SIZE_CONTENT); // 自适应大小
        lv_obj_set_style_radius(g_time_checkboxes[i], LV_RADIUS_CIRCLE, LV_PART_INDICATOR);

        // 3. 设置边框样式（白色，3px）
        lv_obj_set_style_border_color(g_time_checkboxes[i], lv_color_hex(0xFFFFFF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(g_time_checkboxes[i], 3, LV_PART_INDICATOR | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(g_time_checkboxes[i], LV_OPA_0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
        // 4. 设置选中时的背景颜色（白色）
        lv_obj_set_style_bg_color(g_time_checkboxes[i], lv_color_hex(0xFFFFFF), LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_set_style_size(g_time_checkboxes[i], 20, LV_PART_INDICATOR | LV_STATE_CHECKED);

        // 设置文字样式
        lv_obj_set_style_text_font(g_time_checkboxes[i], &eques_regular_36, LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(g_time_checkboxes[i], lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);

        // 默认选中当前配置的项
        if(i == g_screen_off_time) {
            lv_obj_add_state(g_time_checkboxes[i], LV_STATE_CHECKED);
        }
        // 绑定复选框点击事件
        lv_obj_add_event_cb(g_time_checkboxes[i], light_time_checkbox_cb, LV_EVENT_VALUE_CHANGED, (void *)(intptr_t)i);
    }
    
    // 返回
    lv_obj_t *back_btn = create_text_label
    (light_time_scr, ICON_CHEVORN_LEFT, &my_custom_icon, lv_color_hex(0xFFFFFF), 52, 84, LV_OPA_100);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(back_btn,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn,LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn,screen_time_back_btn_click_cb,LV_EVENT_CLICKED,homepage_scr);

    //更新状态条父对象
    update_status_bar_parent(light_time_scr);
    // 切换到子页面
    lv_scr_load(light_time_scr);
}

// void light_time_click_cb(lv_event_t *e)
// {
//     if(e == NULL) return;
//     lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);
//     if(parent_scr == NULL) return;

//     ui_screen_time_settings_create(parent_scr);
//     lv_scr_load(light_time_scr);
//     update_status_bar_parent(light_time_scr);
//     LV_LOG_WARN("进入亮屏时间设置子页面，销毁用户管理");
// }

// 销毁亮屏时间界面 → 零内存泄漏
static void light_time_destroy(void)
{
    // 销毁主页面
    if (lv_obj_is_valid(light_time_scr)) {
        lv_obj_del(light_time_scr);
        light_time_scr = NULL;
    }

    // 清空所有控件指针（杜绝野指针）
    for (int i = 0; i < SCREEN_OFF_MAX; i++) {
        g_time_checkboxes[i] = NULL;
    }
}

// 亮屏时间界面返回
void screen_time_back_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;

    lv_obj_t *current_del_scr = lv_disp_get_scr_act(NULL);
    //lv_obj_t *homepage_scr = (lv_obj_t *)lv_event_get_user_data(e);
    //if(homepage_scr == NULL) return;
    if(!lv_obj_is_valid(current_del_scr)) return;

    // 当前显示的是亮屏时间界面 → 重建主页并销毁当前界面
    if(current_del_scr == light_time_scr) {
        ui_sys_settings_create(current_del_scr);   
        //lv_scr_load(homepage_scr);                  // 重建主页
        light_time_destroy();            // 清空所有控件指针
        return;
    }
}

// 获取当前亮屏时间文本
const char *screen_time_get_str(void)
{
    return g_time_strs[g_screen_off_time];
}

// 主动刷新主页面显示
void screen_time_update_display_label(void)
{
    if (g_time_view_label != NULL && lv_obj_is_valid(g_time_view_label)) {
        lv_label_set_text(g_time_view_label, screen_time_get_str());
    }
}