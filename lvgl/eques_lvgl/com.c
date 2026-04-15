#include "com.h"

extern void update_status_bar_parent(lv_obj_t *new_scr);
extern void lv_homepage(void);
extern lv_obj_t *sys_settings_scr;
extern lv_obj_t *msg_center_scr;
extern lv_obj_t *monitor_video_scr;
extern lv_obj_t *user_manage_scr;
extern lv_obj_t *dev_info_scr;
extern lv_obj_t *file_cache_scr;
extern lv_obj_t *monitor_scr;
extern lv_obj_t *g_switch;
extern lv_obj_t *g_state_label;
extern lv_obj_t *g_time_con;
extern lv_obj_t *g_time_slot_label;
extern lv_obj_t *enroll_scr;
extern lv_obj_t *edit_record_scr; 
/**
 * back回调函数（修正空指针检查）
 */

void back_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);
    
    // 获取当前正在显示的页面（要删除的目标）
    lv_obj_t *current_del_scr = lv_disp_get_scr_act(NULL);
    if(!lv_obj_is_valid(current_del_scr)) return;

    // ===================== 分支2：其他普通页面（主页还在，直接返回父页面） =====================
    if(!lv_obj_is_valid(parent_scr)) {
        LV_LOG_ERROR("父页面无效");
        return;
    }
    // 1. 先加载父页面（主页）
    lv_scr_load(parent_scr);
    update_status_bar_parent(parent_scr);
    // 2. 销毁当前旧页面
    if(current_del_scr == monitor_scr) {
        lv_obj_del(current_del_scr);
        monitor_scr = NULL;
        g_switch = NULL; g_state_label = NULL; g_time_con = NULL; g_time_slot_label = NULL;
    }
    else if(current_del_scr == sys_settings_scr)  { lv_obj_del(current_del_scr); sys_settings_scr = NULL; }
    else if(current_del_scr == msg_center_scr)     { lv_obj_del(current_del_scr); msg_center_scr = NULL; }
    else if(current_del_scr == monitor_video_scr) { lv_obj_del(current_del_scr); monitor_video_scr = NULL; }
    else if(current_del_scr == dev_info_scr)       { lv_obj_del(current_del_scr); dev_info_scr = NULL; }
    else if(current_del_scr == file_cache_scr)     { lv_obj_del(current_del_scr); file_cache_scr = NULL; }
    else if(current_del_scr == enroll_scr)        { lv_obj_del(current_del_scr); enroll_scr = NULL; }
    else if(current_del_scr == edit_record_scr)   { lv_obj_del(current_del_scr); edit_record_scr = NULL; }

    LV_LOG_WARN("普通页面返回成功");
}

/************************** 分割线相关函数实现 **************************/
void config_divider_line_style(
    lv_obj_t *line_obj,
    const lv_point_t *points,
    uint32_t point_cnt,
    uint32_t line_color,
    uint8_t line_width,
    lv_opa_t line_opa
) {
    lv_line_set_points(line_obj, points, point_cnt);
    lv_obj_set_style_line_color(line_obj, lv_color_hex(line_color), LV_STATE_DEFAULT);
    lv_obj_set_style_line_width(line_obj, line_width, LV_STATE_DEFAULT);
    lv_obj_set_style_line_opa(line_obj, line_opa, LV_STATE_DEFAULT);
}


/************************** 图片相关函数实现 **************************/
lv_obj_t *create_image_obj(
    lv_obj_t *parent,
    const void *img_src,
    int32_t x,
    int32_t y
) {
    lv_obj_t *img = lv_img_create(parent);
    lv_img_set_src(img, img_src);
    lv_obj_set_pos(img, x, y);
    lv_obj_clear_flag(img, LV_OBJ_FLAG_SCROLLABLE);
    return img;
}

/************************** 文本标签相关函数实现 **************************/

lv_obj_t *create_text_label(
    lv_obj_t *parent,
    const char *text,
    const lv_font_t *font,
    lv_color_t text_color,
    int x,
    int y,
    lv_opa_t opa
) {
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, font, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label, text_color, LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(label, opa, LV_STATE_DEFAULT);
    lv_obj_set_pos(label, x, y);
    return label;
}

/************************** 容器相关函数实现 **************************/

lv_obj_t *create_container(
    lv_obj_t *parent, 
    int x, int y, 
    int w, int h,
    lv_color_t bg_color,
    lv_opa_t bg_opa,
    int radius,
    lv_color_t border_color,
    int border_width,
    lv_opa_t border_opa
) {
    lv_obj_t *container = lv_obj_create(parent);
    lv_obj_set_pos(container, x, y);
    lv_obj_set_size(container, w, h);
    lv_obj_set_style_bg_color(container, bg_color, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(container, bg_opa, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(container, radius, LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(container, border_color, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(container, border_width, LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(container, border_opa, LV_STATE_DEFAULT);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    return container;
}

lv_obj_t *create_container_circle(
    lv_obj_t *parent, 
    int x, int y, 
    int size,
    bool is_circle,
    lv_color_t bg_color,
    lv_color_t border_color,
    int border_width,
    lv_opa_t border_opa
) {
    lv_obj_t *container = lv_obj_create(parent);
    if(is_circle) {
        lv_obj_set_size(container, size, size);
        lv_obj_set_style_radius(container, size / 2, LV_STATE_DEFAULT);
    } else {
        lv_obj_set_size(container, size, size);
        lv_obj_set_style_radius(container, 0, LV_STATE_DEFAULT);
    }
    lv_obj_set_pos(container, x, y);
    lv_obj_set_style_bg_color(container, bg_color, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(container, LV_OPA_100, LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(container, border_color, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(container, border_width, LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(container, border_opa, LV_STATE_DEFAULT);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_shadow_width(container, 0, LV_STATE_DEFAULT);
    return container;
}

/************************** 按钮相关函数实现 **************************/

lv_obj_t *create_menu_btn_nt(
    lv_obj_t *parent,
    uint32_t x,
    uint32_t y,
    uint32_t w,
    uint32_t h,
    uint32_t bg_color

) {
    lv_obj_t *menu_btn = lv_btn_create(parent);
    lv_obj_set_size(menu_btn, w, h);
    lv_obj_set_pos(menu_btn, x, y);
    lv_obj_set_style_bg_color(menu_btn, lv_color_hex(bg_color), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(menu_btn, LV_OPA_100, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(menu_btn, 0, LV_STATE_DEFAULT | LV_STATE_PRESSED | LV_STATE_DISABLED);
    lv_obj_set_style_border_width(menu_btn, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(menu_btn, lv_color_hex(0x000000), LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(menu_btn, 0, LV_STATE_DEFAULT);
    return menu_btn;
}

lv_obj_t *create_menu_btn(
    lv_obj_t *parent,
    const char *text,
    uint32_t x,
    uint32_t y,
    uint32_t w,
    uint32_t h,
    uint32_t bg_color,
    uint32_t border_color,
    uint8_t border_width,
    uint32_t text_color,
    const lv_font_t *font,
    uint8_t radius,
    lv_opa_t opa
) {
    lv_obj_t *menu_btn = lv_btn_create(parent);
    lv_obj_set_size(menu_btn, w, h);
    lv_obj_set_pos(menu_btn, x, y);
    lv_obj_set_style_border_side(menu_btn, LV_BORDER_SIDE_NONE, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(menu_btn, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(menu_btn, LV_OPA_0, LV_STATE_DEFAULT); 
    lv_obj_set_style_outline_width(menu_btn, 0, LV_STATE_DEFAULT); 
    lv_obj_set_style_bg_color(menu_btn, lv_color_hex(bg_color), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(menu_btn, LV_OPA_100, LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(menu_btn, lv_color_hex(border_color), LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(menu_btn, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(menu_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(menu_btn, radius, LV_STATE_DEFAULT);
    lv_obj_set_style_border_post(menu_btn, false, LV_STATE_DEFAULT);
    lv_obj_t *menu_btn_text = lv_label_create(menu_btn);
    lv_label_set_text(menu_btn_text, text);
    lv_obj_set_style_text_font(menu_btn_text, font, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(menu_btn_text, lv_color_hex(text_color), LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(menu_btn_text, opa, LV_STATE_DEFAULT);
    lv_obj_center(menu_btn_text);
    return menu_btn;
}

/****************渐变容器相关函数***********************/
/**
 * @brief 创建带线性渐变的通用容器（仅支持LV_GRAD_DIR_VER/LV_GRAD_DIR_HOR方向）
 * @param parent        父容器对象
 * @param x             容器x坐标（left）
 * @param y             容器y坐标（top）
 * @param w             容器宽度
 * @param h             容器高度
 * @param radius        容器圆角半径（px）
 * @param main_color    渐变主色（十六进制，如0x1F3150）
 * @param grad_color    渐变副色（十六进制，如0x34568F）
 * @param grad_dir      渐变方向（仅支持LV_GRAD_DIR_VER/LV_GRAD_DIR_HOR）
 * @param main_stop     主色起始位置（0~255，对应0%~100%）
 * @param grad_stop     副色结束位置（0~255，对应0%~100%）
 * @param bg_opa        容器背景不透明度（LV_OPA_0~LV_OPA_100）
 * @return lv_obj_t*    创建的渐变容器对象
 */
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
) {
    if(parent == NULL) return NULL;
    if(grad_dir != LV_GRAD_DIR_VER && grad_dir != LV_GRAD_DIR_HOR) {
        grad_dir = LV_GRAD_DIR_VER;
    }
    if(main_stop > grad_stop) {
        uint8_t temp = main_stop;
        main_stop = grad_stop;
        grad_stop = temp;
    }

    // 1. 创建容器
    lv_obj_t *grad_con = lv_obj_create(parent);
    lv_obj_set_pos(grad_con, x, y);
    lv_obj_set_size(grad_con, w, h);
    lv_obj_set_style_radius(grad_con, radius, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(grad_con, bg_opa, LV_STATE_DEFAULT);
    lv_obj_clear_flag(grad_con, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(grad_con, 0, LV_STATE_DEFAULT);

    // ====================== 核心修复 ======================
    // ✅ 直接给控件设置样式，不使用 static 共享样式！
    // ✅ 每个控件独立颜色，永远不会互相污染
    lv_obj_set_style_bg_color(grad_con, lv_color_hex(main_color), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(grad_con, lv_color_hex(grad_color), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(grad_con, grad_dir, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(grad_con, main_stop, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(grad_con, grad_stop, LV_STATE_DEFAULT);

    return grad_con;
}
