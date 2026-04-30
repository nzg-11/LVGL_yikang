#if LV_EQUES_VER
#include "lv_other_member.h"
#include "lv_a_enroll_opt.h"
#include "stdio.h"

lv_obj_t *g_other_finger_labels[MAX_OTHER_MEMBER_COUNT] = {NULL};
lv_obj_t *g_other_pwd_labels[MAX_OTHER_MEMBER_COUNT] = {NULL};
lv_obj_t *g_other_finger_imgs[MAX_OTHER_MEMBER_COUNT] = {NULL};
lv_obj_t *g_other_pwd_imgs[MAX_OTHER_MEMBER_COUNT] = {NULL};

// ====================== 数据结构定义 ======================
typedef struct {
    uint8_t member_idx;          // 成员唯一索引（0~7）
    lv_obj_t *parent_scr;        // 父屏幕对象
    other_member_info_t *info;   // 指向该成员的信息结构体
} enroll_cb_data_t;

// ====================== 全局变量 ======================
// 成员信息核心变量
uint8_t g_selected_other_member_idx = MAX_OTHER_MEMBER_COUNT;
other_member_info_t other_member_list[MAX_OTHER_MEMBER_COUNT] = {0};

// 删除模式专用变量
static bool g_is_delete_mode = false;
static lv_obj_t *g_delete_cancel_btn = NULL;
static lv_obj_t *g_delete_confirm_btn = NULL;
// static uint8_t g_selected_delete_idx = MAX_OTHER_MEMBER_COUNT;
// static lv_obj_t *g_selected_delete_btn = NULL;
static lv_obj_t *g_delete_flag_imgs[MAX_OTHER_MEMBER_COUNT] = {NULL};
static lv_obj_t *g_delete_hid_containers[MAX_OTHER_MEMBER_COUNT] = {NULL};
static bool g_member_selected[MAX_OTHER_MEMBER_COUNT] = {false};
static lv_obj_t *g_member_cards[MAX_OTHER_MEMBER_COUNT] = {NULL};
static lv_obj_t *delete_img = NULL;
static lv_obj_t *back_btn = NULL;

// 全局变量
static lv_obj_t *bg_mask_layer = NULL;  
static lv_obj_t *custom_popup = NULL;
static lv_obj_t *other_member_scr = NULL; 
static lv_style_t other_member_grad_style;
static bool other_member_style_inited = false;
static lv_obj_t *name_keyboard = NULL;
lv_obj_t *g_name_input = NULL;
static lv_color_t selected_avatar_color = {0};
static lv_coord_t next_member_y = 374;
static uint8_t member_count = 0;
lv_obj_t *other_member_add_con = NULL;
lv_obj_t *other_member_add = NULL;

// 常量定义
static const lv_coord_t avatar_x_offset[] = {19, 110, 201};
static const lv_coord_t avatar_y_offset[] = {12, 101};

// ====================== 函数声明 ======================
// 内部函数前置声明
static void close_custom_popup(void);

// UI构建相关
static lv_obj_t *create_other_member_card(lv_obj_t *parent, lv_coord_t y_pos, const char *member_name, lv_color_t avatar_color, uint8_t member_idx);
static void update_add_member_btn_state(void);
void ui_other_member_create(lv_obj_t *user_manage_scr);
static void restore_other_members(lv_obj_t *parent);

// 删除模式相关
static void delete_cancel_click_cb(lv_event_t *e);
static void delete_confirm_click_cb(lv_event_t *e);
static void delete_popup_cancel_cb(lv_event_t *e);
static void delete_popup_ok_cb(lv_event_t *e);
static void show_all_member_delete_btn(bool show);
static void delete_img_click_cb(lv_event_t *e);
static void switch_delete_mode(bool enter);
static void member_delete_btn_click_cb(lv_event_t *e);
static void member_card_click_cb(lv_event_t *e);

// 遮罩层相关
static void create_bg_mask_layer(lv_obj_t *target_container);

// 事件回调
void other_member_add_click_cb(lv_event_t *e);

// 全局样式初始化
static void init_other_member_styles(void)
{
    if(!other_member_style_inited) {
        lv_style_init(&other_member_grad_style);
        other_member_style_inited = true;
    }
}

static void other_member_scr_load_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *scr = lv_event_get_target(e);
    if(!is_lv_obj_valid(scr)) return;
    
    update_status_bar_parent(scr);
}

void ui_other_member_create(lv_obj_t *user_manage_scr)
{
    if(selected_avatar_color.ch.red == 0 && selected_avatar_color.ch.green == 0 && selected_avatar_color.ch.blue == 0) {
        selected_avatar_color = lv_color_hex(0xEDF4FF);
    }
    init_other_member_styles();
    
    if(user_manage_scr == NULL) {
        LV_LOG_WARN("ui_other_member_create: user_manage_scr is NULL!");
        return;
    }

    if(other_member_scr == NULL) {
        other_member_scr = lv_obj_create(NULL);
        lv_obj_add_event_cb(other_member_scr, other_member_scr_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    } else {
        lv_obj_clean(other_member_scr);
        // 重置临时状态，但保留成员数据数组
        bg_mask_layer = NULL;
        name_keyboard = NULL;
        selected_avatar_color = lv_color_hex(0xEDF4FF);
    }
    
    lv_style_reset(&other_member_grad_style);
    lv_style_set_bg_color(&other_member_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&other_member_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&other_member_grad_style, LV_GRAD_DIR_VER);
    lv_style_set_bg_main_stop(&other_member_grad_style, 0);
    lv_style_set_bg_grad_stop(&other_member_grad_style, 255);
    lv_obj_add_style(other_member_scr, &other_member_grad_style, LV_STATE_DEFAULT);

    lv_obj_t *other_member_label = create_text_label
    (other_member_scr, "other member", &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 0, 115, LV_OPA_100);
    lv_obj_align(other_member_label, LV_ALIGN_TOP_MID, 0, 115);

    // 添加其他成员容器
    other_member_add_con = create_container
    (other_member_scr,47,195,710,175,lv_color_hex(0x192A46), LV_OPA_100, 6,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_add_flag(other_member_add_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(other_member_add_con, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(other_member_add_con, other_member_add_click_cb, LV_EVENT_CLICKED, user_manage_scr);

    //添加加号
    other_member_add = create_container
    (other_member_scr,360,243,79,79,lv_color_hex(0x2F476F), LV_OPA_100, 100,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    //加号线
    lv_obj_t *divider_line1 = lv_line_create(other_member_scr);
    static const lv_point_t divider_points1[] = {{372, 282}, {426, 282}}; 
    config_divider_line_style(divider_line1, divider_points1, 2, 0x617C9D, 8, LV_OPA_100);
    lv_obj_t *divider_line2 = lv_line_create(other_member_scr);
    static const lv_point_t divider_points2[] = {{399, 255}, {399, 309}}; 
    config_divider_line_style(divider_line2, divider_points2, 2, 0x617C9D, 8, LV_OPA_100);

    lv_obj_add_flag(other_member_add, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(other_member_add, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(other_member_add, other_member_add_click_cb, LV_EVENT_CLICKED, user_manage_scr);

    //右上角删除设置
    delete_img = create_image_obj(other_member_scr, "H:....png", 709, 132);
    lv_obj_add_flag(delete_img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(delete_img, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(delete_img, delete_img_click_cb, LV_EVENT_CLICKED, NULL);

    // 左上角返回按钮
    back_btn = create_image_obj(other_member_scr, "H:back.png", 52, 123);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn, back_btn_click_cb, LV_EVENT_CLICKED, user_manage_scr);

    // ====================== 恢复成员卡片 ======================
    restore_other_members(other_member_scr);

    // 切换到设置屏幕
    lv_scr_load(other_member_scr);
}

// 其他成员界面回调
void other_member_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    lv_obj_t *user_manage_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(user_manage_scr == NULL) {
        LV_LOG_WARN("other_member_btn_click_cb: user_manage_scr is NULL!");
        return;
    }
    ui_other_member_create(user_manage_scr);
}

/**********************************************************删除回调函数***************************************************** */
/**
 * @brief 删除按钮点击回调函数
 * @param e 事件指针
 */
static void delete_img_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    // 进入删除模式
    switch_delete_mode(true);
}

/**
 * @brief 删除取消按钮点击回调函数
 * @param e 事件指针
 */
static void delete_cancel_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    switch_delete_mode(false);
}

/**
 * @brief 删除确认弹窗-取消回调函数
 * @param e 事件指针
 */
static void delete_popup_cancel_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *popup = (lv_obj_t *)lv_event_get_user_data(e);
    
    // 关闭弹窗
    if(popup != NULL && lv_obj_is_valid(popup)) {
        lv_obj_del(popup);
    }
    if(bg_mask_layer != NULL && lv_obj_is_valid(bg_mask_layer)) {
        lv_obj_add_flag(bg_mask_layer, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief 删除确认按钮点击回调函数
 * @param e 事件指针
 */
static void delete_confirm_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    // 检查是否有选中的成员
    bool has_selected = false;
    for(uint8_t i = 0; i < MAX_OTHER_MEMBER_COUNT; i++) {
        if(g_member_selected[i]) {
            has_selected = true;
            break;
        }
    }
    if(!has_selected) {
        LV_LOG_USER("未选择要删除的成员");
        return;
    }
    
    // 1. 创建遮罩层
    create_bg_mask_layer(other_member_scr);
    
    // 2. 创建确认弹窗
    lv_obj_t *confirm_popup = create_container(
        other_member_scr, 100, 455, 600, 297, 
        lv_color_hex(0xE0EDFF), LV_OPA_100, 
        16, lv_color_hex(0x1F3150), 0, LV_OPA_90
    );
    
    // 3. 提示文本
    lv_obj_t *tip_label = create_text_label(
        confirm_popup, "confirm_delete?", &lv_font_montserrat_32, 
        lv_color_hex(0x000000), 156, 52, LV_OPA_100
    );
    
    // 4. 确认按钮
    lv_obj_t *confirm_btn = create_custom_gradient_container
    (confirm_popup, 190, 141, 220, 44, 6, 0x006BDC, 0x00BDBD, LV_GRAD_DIR_HOR, 0, 225, LV_OPA_100);
    lv_obj_set_style_pad_all(confirm_btn, 0, LV_STATE_DEFAULT);
    lv_obj_t *confirm_label = create_text_label(
    confirm_btn, "confirm", &lv_font_montserrat_24, lv_color_hex(0x000000), 0, 0, LV_OPA_100);
    lv_obj_set_align(confirm_label, LV_ALIGN_CENTER);
    lv_obj_add_flag(confirm_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(confirm_btn, delete_popup_ok_cb, LV_EVENT_CLICKED, confirm_popup);
    
    // 5. 取消按钮
    lv_obj_t *cancel_btn = create_container
    (confirm_popup, 190, 210, 220, 44, lv_color_hex(0xE0EDFF), LV_OPA_100, 8, lv_color_hex(0xFF3333), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(cancel_btn, 0, LV_STATE_DEFAULT);
    lv_obj_t *cancel_label = create_text_label
    (cancel_btn, "cancel", &lv_font_montserrat_20, lv_color_hex(0xBDBDBD), 0, 0, LV_OPA_100);
    lv_obj_set_align(cancel_label, LV_ALIGN_CENTER);
    lv_obj_add_flag(cancel_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(cancel_btn, delete_popup_cancel_cb, LV_EVENT_CLICKED, confirm_popup);
    
    lv_obj_move_foreground(confirm_popup);
}

/**
 * @brief 删除标志图片点击回调函数（取消勾选）
 * @param e 事件指针
 */
static void delete_flag_click_cb(lv_event_t *e)
{
    if(e == NULL || !g_is_delete_mode) return;
    
    // 1. 获取成员索引和图片对象
    uint8_t idx = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    lv_obj_t *delete_flag_img = lv_event_get_target(e);
    
    // 2. 安全检查
    if(idx >= MAX_OTHER_MEMBER_COUNT || !g_member_selected[idx]) {
        LV_LOG_WARN("Invalid index or not selected: idx=%d", idx);
        return;
    }

    LV_LOG_USER("取消选中成员：%d", idx);
    
    // 3. 取消选中：删除图片，重置状态
    if(delete_flag_img != NULL && lv_obj_is_valid(delete_flag_img)) {
        lv_obj_del(delete_flag_img); // 删除图片对象
    }
    g_delete_flag_imgs[idx] = NULL;
    g_member_selected[idx] = false;
    
    // 4. 直接从全局数组获取delete_hid并显示（核心  ）
    if(g_delete_hid_containers[idx] != NULL && lv_obj_is_valid(g_delete_hid_containers[idx])) {
        // 强制显示delete_hid
        lv_obj_clear_flag(g_delete_hid_containers[idx], LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_opa(g_delete_hid_containers[idx], LV_OPA_100, LV_STATE_DEFAULT);
        LV_LOG_USER("成功显示成员%d的delete_hid容器", idx);
    } else {
        LV_LOG_WARN("成员%d的delete_hid容器不存在或无效", idx);
    }
    // 恢复成员卡片透明度
    if(g_member_cards[idx] != NULL && lv_obj_is_valid(g_member_cards[idx])) {
        lv_obj_set_style_opa(g_member_cards[idx], LV_OPA_100, LV_STATE_DEFAULT);
    }
}

/**
 * @brief 成员卡片删除按钮点击回调函数
 * @param e 事件指针
 */
static void member_delete_btn_click_cb(lv_event_t *e)
{
    if(e == NULL || !g_is_delete_mode) return;
    
    // 1. 获取成员索引和勾选容器
    uint8_t idx = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    lv_obj_t *delete_hid = lv_event_get_target(e);
    
    // 2. 安全检查
    if(idx >= MAX_OTHER_MEMBER_COUNT || g_member_selected[idx]) {
        LV_LOG_WARN("Invalid index or already selected: idx=%d", idx);
        return;
    }

    LV_LOG_USER("选中成员：%d", idx);
    
    // 3. 选中：隐藏勾选容器，创建并显示删除图片
    lv_obj_add_flag(delete_hid, LV_OBJ_FLAG_HIDDEN);
    
    // 获取父容器（成员卡片）
    lv_obj_t *member_con = lv_obj_get_parent(delete_hid);
    if(member_con == NULL || !lv_obj_is_valid(member_con)) {
        LV_LOG_WARN("成员%d的卡片容器无效", idx);
        return;
    }
    
    // 先销毁旧的delete_flag（防止重复创建）
    if(g_delete_flag_imgs[idx] != NULL && lv_obj_is_valid(g_delete_flag_imgs[idx])) {
        lv_obj_del(g_delete_flag_imgs[idx]);
        g_delete_flag_imgs[idx] = NULL;
    }
    
    // 创建删除图片并绑定取消勾选回调
    g_delete_flag_imgs[idx] = create_image_obj(member_con, "H:delete_flag.png", 655, 59);
    if(g_delete_flag_imgs[idx] != NULL && lv_obj_is_valid(g_delete_flag_imgs[idx])) {
        lv_obj_set_size(g_delete_flag_imgs[idx], 36, 36);
        // 绑定点击回调，传递成员索引
        lv_obj_add_event_cb(g_delete_flag_imgs[idx], delete_flag_click_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)idx);
        lv_obj_add_flag(g_delete_flag_imgs[idx], LV_OBJ_FLAG_CLICKABLE); // 确保图片可点击
        g_member_selected[idx] = true;
        LV_LOG_USER("成功创建成员%d的delete_flag图片", idx);

        // 核心修改：勾选成员时，将该成员卡片透明度设为50%（变灰）
        if(g_member_cards[idx] != NULL && lv_obj_is_valid(g_member_cards[idx])) {
            lv_obj_set_style_opa(g_member_cards[idx], LV_OPA_50, LV_STATE_DEFAULT);
        }
    } else {
        LV_LOG_WARN("创建成员%d的delete_flag图片失败", idx);
        // 创建失败时恢复显示delete_hid
        lv_obj_clear_flag(delete_hid, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief 删除确认弹窗-确认回调函数
 * @param e 事件指针
 */
static void delete_popup_ok_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *popup = (lv_obj_t *)lv_event_get_user_data(e);
    
    // 1. 批量删除所有选中的成员
    bool has_delete = false;
    for(uint8_t i = 0; i < MAX_OTHER_MEMBER_COUNT; i++) {
        if(g_member_selected[i]) {
            // 清空成员数据
            other_member_list[i].is_valid = false;
            memset(other_member_list[i].name, 0, sizeof(other_member_list[i].name));
            other_member_list[i].finger_count = 0;
            other_member_list[i].pwd_count = 0;
             //  清空指纹信息
            memset(&g_other_finger_info[i], 0, sizeof(finger_enroll_info_t));
            //  清空密码信息
            memset(&g_other_pwd_info[i], 0, sizeof(pwd_enroll_info_t));
            // 删除勾选相关的UI元素
            if(g_delete_flag_imgs[i] != NULL && lv_obj_is_valid(g_delete_flag_imgs[i])) {
                lv_obj_del(g_delete_flag_imgs[i]);
                g_delete_flag_imgs[i] = NULL;
            }
            if(g_delete_hid_containers[i] != NULL && lv_obj_is_valid(g_delete_hid_containers[i])) {
                lv_obj_del(g_delete_hid_containers[i]);
                g_delete_hid_containers[i] = NULL;
            }
            
            // 强制重置成员卡片样式
            if(g_member_cards[i] != NULL && lv_obj_is_valid(g_member_cards[i])) {
                lv_obj_set_style_opa(g_member_cards[i], LV_OPA_100, LV_STATE_DEFAULT); // 恢复透明度
                lv_obj_add_flag(g_member_cards[i], LV_OBJ_FLAG_CLICKABLE); // 恢复点击
                lv_obj_del(g_member_cards[i]); // 删除卡片控件
                g_member_cards[i] = NULL;      // 清空引用
            }
            
            // 核心  3：重置选中状态标记
            g_member_selected[i] = false;
            
            has_delete = true;
            LV_LOG_USER("成功删除成员：%d", i);
        }
    }
    
    // 2. 刷新界面（彻底重置所有样式+重建卡片）
    if(has_delete && other_member_scr != NULL && lv_obj_is_valid(other_member_scr)) {
        // 步骤1：清空所有残留的样式/状态（关键！）
        for(uint8_t i = 0; i < MAX_OTHER_MEMBER_COUNT; i++) {
            // 即使是未选中的成员，也重置样式（防止批量删除时的样式污染）
            if(g_member_cards[i] != NULL && lv_obj_is_valid(g_member_cards[i])) {
                lv_obj_set_style_opa(g_member_cards[i], LV_OPA_100, LV_STATE_DEFAULT);
                lv_obj_add_flag(g_member_cards[i], LV_OBJ_FLAG_CLICKABLE);
            }
        }
        
        // 步骤2：删除所有旧卡片，重建剩余成员
        for(uint8_t i = 0; i < MAX_OTHER_MEMBER_COUNT; i++) {
            if(g_member_cards[i] != NULL && lv_obj_is_valid(g_member_cards[i])) {
                lv_obj_del(g_member_cards[i]);
                g_member_cards[i] = NULL;
            }
        }
        
        // 步骤3：重置计数，重新创建剩余成员卡片
        member_count = 0;
        next_member_y = 374;
        restore_other_members(other_member_scr);
        
        // 步骤4：强制刷新屏幕（解决LVGL样式缓存问题）
        lv_obj_invalidate(other_member_scr);
        lv_refr_now(NULL); // 立即刷新显示，不等待LVGL刷新周期
        
        // 步骤5：恢复添加按钮状态
        update_add_member_btn_state();
    }
    
    // 3. 关闭弹窗
    if(popup != NULL && lv_obj_is_valid(popup)) {
        lv_obj_del(popup);
    }
    if(bg_mask_layer != NULL && lv_obj_is_valid(bg_mask_layer)) {
        lv_obj_add_flag(bg_mask_layer, LV_OBJ_FLAG_HIDDEN);
    }
    
    // 4. 退出删除模式（确保所有删除模式的UI状态都被清除）
    switch_delete_mode(false);
}

/**
 * @brief 切换删除模式
 * @param enter 是否进入删除模式
 */
static void switch_delete_mode(bool enter)
{
    g_is_delete_mode = enter;
    
    if(enter) {
        // 进入删除模式
        // 1. 隐藏返回按钮和更多按钮
        if(back_btn != NULL && lv_obj_is_valid(back_btn)) {
            lv_obj_add_flag(back_btn, LV_OBJ_FLAG_HIDDEN);
        }
        if(delete_img != NULL && lv_obj_is_valid(delete_img)) {
            lv_obj_add_flag(delete_img, LV_OBJ_FLAG_HIDDEN);
        }
        
        // 2. 创建取消按钮
        if(g_delete_cancel_btn == NULL || !lv_obj_is_valid(g_delete_cancel_btn)) {
            g_delete_cancel_btn = create_text_label(
                other_member_scr, "cancel", &lv_font_montserrat_24, 
                lv_color_hex(0xFFFFFF), 52, 123, LV_OPA_100
            );
            lv_obj_add_flag(g_delete_cancel_btn, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(g_delete_cancel_btn, LV_OPA_80, LV_STATE_PRESSED);
            lv_obj_add_event_cb(g_delete_cancel_btn, delete_cancel_click_cb, LV_EVENT_CLICKED, NULL);
        } else {
            lv_obj_clear_flag(g_delete_cancel_btn, LV_OBJ_FLAG_HIDDEN);
        }
        
        // 3. 创建删除按钮
        if(g_delete_confirm_btn == NULL || !lv_obj_is_valid(g_delete_confirm_btn)) {
            g_delete_confirm_btn = create_text_label(
                other_member_scr, "delete", &lv_font_montserrat_24, 
                lv_color_hex(0xFF0000), 709, 132, LV_OPA_100
            );
            lv_obj_add_flag(g_delete_confirm_btn, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(g_delete_confirm_btn, LV_OPA_80, LV_STATE_PRESSED);
            lv_obj_add_event_cb(g_delete_confirm_btn, delete_confirm_click_cb, LV_EVENT_CLICKED, NULL);
        } else {
            lv_obj_clear_flag(g_delete_confirm_btn, LV_OBJ_FLAG_HIDDEN);
        }
        
        // 4. 显示所有成员的删除按钮
        show_all_member_delete_btn(true);

        // 进入删除模式时，禁用所有成员卡片的点击
        for(uint8_t i = 0; i < MAX_OTHER_MEMBER_COUNT; i++) {
            if(g_member_cards[i] != NULL && lv_obj_is_valid(g_member_cards[i])) {
                lv_obj_clear_flag(g_member_cards[i], LV_OBJ_FLAG_CLICKABLE); // 禁用点击
                // 初始透明度保持100%，只有勾选后才变灰
                lv_obj_set_style_opa(g_member_cards[i], LV_OPA_100, LV_STATE_DEFAULT);
            }
        }
        
        // 5. 禁用添加成员按钮
        if(other_member_add_con != NULL && lv_obj_is_valid(other_member_add_con)) {
            lv_obj_clear_flag(other_member_add_con, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(other_member_add_con, LV_OPA_50, LV_STATE_DEFAULT);
        }
        if(other_member_add != NULL && lv_obj_is_valid(other_member_add)) {
            lv_obj_clear_flag(other_member_add, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(other_member_add, LV_OPA_50, LV_STATE_DEFAULT);
        }
    } else {
        // 退出删除模式
        // 1. 显示返回按钮和更多按钮
        if(back_btn != NULL && lv_obj_is_valid(back_btn)) {
            lv_obj_clear_flag(back_btn, LV_OBJ_FLAG_HIDDEN);
        }
        if(delete_img != NULL && lv_obj_is_valid(delete_img)) {
            lv_obj_clear_flag(delete_img, LV_OBJ_FLAG_HIDDEN);
        }
        
        // 2. 隐藏取消和删除按钮
        if(g_delete_cancel_btn != NULL && lv_obj_is_valid(g_delete_cancel_btn)) {
            lv_obj_add_flag(g_delete_cancel_btn, LV_OBJ_FLAG_HIDDEN);
        }
        if(g_delete_confirm_btn != NULL && lv_obj_is_valid(g_delete_confirm_btn)) {
            lv_obj_add_flag(g_delete_confirm_btn, LV_OBJ_FLAG_HIDDEN);
        }
        
        // 3. 隐藏所有成员的删除按钮和删除图片
        show_all_member_delete_btn(false);

        // 退出删除模式时，恢复所有成员卡片的点击和透明度
        for(uint8_t i = 0; i < MAX_OTHER_MEMBER_COUNT; i++) {
            if(g_member_cards[i] != NULL && lv_obj_is_valid(g_member_cards[i])) {
                lv_obj_add_flag(g_member_cards[i], LV_OBJ_FLAG_CLICKABLE); // 恢复点击
                lv_obj_set_style_opa(g_member_cards[i], LV_OPA_100, LV_STATE_DEFAULT); // 恢复透明度
            }
        }
        
        // 5. 恢复添加成员按钮状态
        update_add_member_btn_state();
    }
}
/**********************************************************删除回调函数截至********************************************/

/**
 * @brief 保存成员信息到数组
 * @param name 成员姓名
 * @param color 成员头像颜色
 * @return uint8_t 成员索引（成功）或MAX_OTHER_MEMBER_COUNT（失败）
 */
static uint8_t save_other_member_info(const char *name, lv_color_t color)
{
    if(name == NULL) return MAX_OTHER_MEMBER_COUNT;
    
    // 找到第一个空位置
    for(uint8_t i = 0; i < MAX_OTHER_MEMBER_COUNT; i++) {
        if(!other_member_list[i].is_valid) {
            strncpy(other_member_list[i].name, name, sizeof(other_member_list[i].name)-1);
            other_member_list[i].avatar_color = color;
            other_member_list[i].is_valid = true;
            other_member_list[i].finger_count = 0;
            other_member_list[i].pwd_count = 0;
            return i; // 返回成员索引
        }
    }
    return MAX_OTHER_MEMBER_COUNT; // 无空位置
}

/**
 * @brief 从数组恢复所有有效成员卡片
 * @param parent 父容器对象指针
 */
static void restore_other_members(lv_obj_t *parent)
{
    if(parent == NULL) return;
    
    next_member_y = 374; // 重置Y轴起始位置
    member_count = 0;    // 重置计数
    
    // 遍历成员数组，恢复所有有效成员
    for(uint8_t i = 0; i < MAX_OTHER_MEMBER_COUNT; i++) {
        if(other_member_list[i].is_valid) {
            create_other_member_card(parent, next_member_y, 
                                     other_member_list[i].name, 
                                     other_member_list[i].avatar_color, i);
            next_member_y += 179;
            member_count++;
        }
    }
    
    // 更新添加按钮状态
    update_add_member_btn_state();
}

/*****************************************************成员信息创建截至***************************************** */

/**
 * @brief 其他成员卡片点击回调函数
 * 
 * 处理点击事件，根据是否处于删除模式来执行不同操作：
 * - 非删除模式：选中成员并调用录入回调
 * - 删除模式：不执行原有逻辑
 * 
 * @param e 事件对象指针，包含点击事件信息
 */
static void member_card_click_cb(lv_event_t *e)
{
    if(e == NULL || g_is_delete_mode) return;
    
    // 1. 获取当前选中的其他成员索引
    uint8_t idx = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    g_selected_other_member_idx = idx;
    
    // 2. 构造通用成员信息（  name数组赋值问题）
    common_member_info_t member_info = {
        .type = MEMBER_TYPE_OTHER,
        .idx = idx,
        .avatar_color = other_member_list[idx].avatar_color,
        .finger_count = other_member_list[idx].finger_count,
        .pwd_count = other_member_list[idx].pwd_count,
        .card_count = 0,
        .face_count = 0
    };
    //   ：拷贝字符串（数组不能直接赋值）
    strncpy(member_info.name, other_member_list[idx].name, sizeof(member_info.name)-1);
    member_info.name[sizeof(member_info.name)-1] = '\0';
    
    // 3.   ：LVGL 8.3 无lv_event_set_user_data，用静态变量+直接调用
    static common_member_info_t g_temp_member_info;
    g_temp_member_info = member_info;
    lv_obj_t *other_member_scr = lv_obj_get_parent(lv_event_get_target(e));
    ui_enroll_create(&g_temp_member_info, other_member_scr);
}
/**
 * @brief 创建其他成员卡片
 * 
 * 创建一个其他成员的卡片，仅保留头像、名称（移除指纹/密码/卡片/人脸相关）
 * 
 * @param parent 父容器对象指针
 * @param y_pos 卡片在父容器中的Y轴位置
 * @param member_name 成员名称字符串指针
 * @param avatar_color 头像颜色
 * @param member_idx 成员索引（0-7）
 * @return lv_obj_t* 成员卡片容器对象指针，失败返回NULL
 */
static lv_obj_t *create_other_member_card(lv_obj_t *parent, lv_coord_t y_pos, const char *member_name, lv_color_t avatar_color, uint8_t member_idx)
{
    if(parent == NULL || member_name == NULL || member_idx >= MAX_OTHER_MEMBER_COUNT) return NULL;
    
    // 1. 创建成员卡片容器（原有逻辑不变）
    lv_obj_t *member_con = create_container
    (parent, 47, y_pos, 710, 175, lv_color_hex(0x192A46), LV_OPA_100, 6, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_user_data(member_con, (void*)(uintptr_t)member_idx);
    lv_obj_set_style_pad_all(member_con, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(member_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(member_con, LV_OPA_80, LV_STATE_PRESSED);

    lv_obj_add_event_cb(member_con, member_card_click_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)member_idx);
    g_member_cards[member_idx] = member_con;
    
    // 4. 头像容器（原有逻辑不变）
    lv_obj_t *avatar_con = create_container
    (member_con, 23, 26, 99, 99, avatar_color, LV_OPA_100, 100, avatar_color, 0, LV_OPA_90);

    // 5. 成员名称标签（原有逻辑不变）
    lv_obj_t *name_label = create_text_label
    (member_con, member_name, &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 151, 40, LV_OPA_100);

    // 6. 指纹/密码相关：和家庭成员逻辑完全一致（核心修改）
    // 读取全局数组中的数量
    uint8_t finger_count = other_member_list[member_idx].finger_count;
    uint8_t pwd_count = other_member_list[member_idx].pwd_count;

    // 指纹部分：动态文本+图标，存储到全局数组
    char finger_text[8] = {0};
    snprintf(finger_text, sizeof(finger_text), "%d/2", finger_count);
    const char *finger_img_path = (finger_count > 0) ? "H:finger_has_record.png" : "H:finger_no_record.png";
    g_other_finger_imgs[member_idx] = create_image_obj(member_con, finger_img_path, 151, 105);
    g_other_finger_labels[member_idx] = create_text_label
    (member_con, finger_text, &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 189, 105, LV_OPA_100);

    // 密码部分：动态文本+图标，存储到全局数组
    char pwd_text[8] = {0};
    snprintf(pwd_text, sizeof(pwd_text), "%d/1", pwd_count);
    const char *pwd_img_path = (pwd_count > 0) ? "H:pwd_has_record.png" : "H:pwd_no_record.png";
    g_other_pwd_imgs[member_idx] = create_image_obj(member_con, pwd_img_path, 259, 105);
    g_other_pwd_labels[member_idx] = create_text_label
    (member_con, pwd_text, &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 297, 105, LV_OPA_100);
    
    // 7. 预埋删除按钮（原有逻辑不变）
    lv_obj_t *delete_hid = create_container
    (member_con, 655, 59, 36, 36, lv_color_hex(0x192A46), LV_OPA_100, 100, lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_add_flag(delete_hid, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(delete_hid, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(delete_hid, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(delete_hid, member_delete_btn_click_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)member_idx);
    g_delete_hid_containers[member_idx] = delete_hid;
    g_delete_flag_imgs[member_idx] = NULL;
    g_member_selected[member_idx] = false;
    
    return member_con;
}

/**
 * @brief 更新其他成员指纹/密码数量UI（和家庭成员函数逻辑完全一致）
 * @param member_idx 其他成员索引（0~7）
 */
void update_other_member_count_ui(uint8_t member_idx)
{
    if(member_idx >= MAX_OTHER_MEMBER_COUNT) return;

    // 1. 安全校验：标签/图片对象是否有效
    if(g_other_finger_labels[member_idx] == NULL || !lv_obj_is_valid(g_other_finger_labels[member_idx]) ||
       g_other_pwd_labels[member_idx] == NULL || !lv_obj_is_valid(g_other_pwd_labels[member_idx]) ||
       g_other_finger_imgs[member_idx] == NULL || !lv_obj_is_valid(g_other_finger_imgs[member_idx]) ||
       g_other_pwd_imgs[member_idx] == NULL || !lv_obj_is_valid(g_other_pwd_imgs[member_idx])) {
        LV_LOG_WARN("Other member %d labels/imgs are invalid, skip update", member_idx);
        return;
    }

    // 2. 读取全局数组中的最新数量
    uint8_t finger_count = other_member_list[member_idx].finger_count;
    uint8_t pwd_count = other_member_list[member_idx].pwd_count;

    // 3. 更新指纹计数文本+图标
    char finger_text[8] = {0};
    snprintf(finger_text, sizeof(finger_text), "%d/2", finger_count);
    lv_label_set_text(g_other_finger_labels[member_idx], finger_text);
    const char *finger_img_path = (finger_count > 0) ? "H:finger_has_record.png" : "H:finger_no_record.png";
    lv_img_set_src(g_other_finger_imgs[member_idx], finger_img_path);

    // 4. 更新密码计数文本+图标
    char pwd_text[8] = {0};
    snprintf(pwd_text, sizeof(pwd_text), "%d/1", pwd_count);
    lv_label_set_text(g_other_pwd_labels[member_idx], pwd_text);
    const char *pwd_img_path = (pwd_count > 0) ? "H:pwd_has_record.png" : "H:pwd_no_record.png";
    lv_img_set_src(g_other_pwd_imgs[member_idx], pwd_img_path);

    // 5. 强制刷新UI
    lv_obj_invalidate(lv_scr_act());
    LV_LOG_INFO("Updated other member %d count UI: finger=%d, pwd=%d",
                member_idx, finger_count, pwd_count);
}

/**
 * @brief 头像点击回调函数
 * 
 * 处理头像点击事件，根据当前成员数量判断是否允许  成员。
 * 如果成员数量未达上限，会弹出输入框请求成员名称，
 * 并根据用户输入创建新的其他成员卡片。
 * 
 * @param e 事件对象指针，包含点击事件信息
 */
static void avatar_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *avatar = lv_event_get_target(e);
    uint8_t avatar_idx = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    
    if(member_count >= MAX_OTHER_MEMBER_COUNT) {
        LV_LOG_USER("成员数量已达上限（%d个),无法  ", MAX_OTHER_MEMBER_COUNT);
        close_custom_popup();
        return;
    }
    
    LV_LOG_USER("点击了第 %d 个头像，创建新成员", avatar_idx + 1);
    
    // 1. 获取选择的头像颜色
    selected_avatar_color = lv_obj_get_style_bg_color(avatar, LV_STATE_DEFAULT);
    
    // 2. 获取输入的成员名称
    char member_name[16] = {0};
    if(g_name_input != NULL && lv_obj_is_valid(g_name_input)) {
        const char *input_name = lv_textarea_get_text(g_name_input);
        if(input_name != NULL && strlen(input_name) > 0) {
            strncpy(member_name, input_name, sizeof(member_name)-1);
        } else {
            snprintf(member_name, sizeof(member_name), "00%d", member_count + 1);
        }
    } else {
        snprintf(member_name, sizeof(member_name), "00%d", member_count + 1);
    }
    
    // 3. 保存成员信息到数组
    uint8_t member_idx = save_other_member_info(member_name, selected_avatar_color);
    if(member_idx == MAX_OTHER_MEMBER_COUNT) {
        LV_LOG_WARN("No empty slot for new member");
        close_custom_popup();
        return;
    }
    
    // 4. 创建新成员卡片
    if(other_member_scr != NULL && lv_obj_is_valid(other_member_scr)) {
        create_other_member_card(other_member_scr, next_member_y, member_name, selected_avatar_color, member_idx);
        next_member_y += 179;
        member_count++; // 计数自增
        update_add_member_btn_state();
    }
    
    // 5. 关闭键盘和弹窗
    if(name_keyboard != NULL && lv_obj_is_valid(name_keyboard)) {
        lv_obj_add_flag(name_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
    close_custom_popup();
}

// 隐藏键盘
static void hide_name_keyboard(lv_event_t *e)
{
    (void)e;
    if(name_keyboard != NULL && lv_obj_is_valid(name_keyboard)) {
        lv_obj_add_flag(name_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

// 名称输入框点击回调（弹出键盘）
static void name_input_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *name_input = lv_event_get_target(e);
    
    // 1. 创建键盘（仅创建一次）
    if(name_keyboard == NULL || !lv_obj_is_valid(name_keyboard)) {
        name_keyboard = lv_keyboard_create(lv_scr_act());
        // 设置键盘样式
        lv_obj_set_style_bg_color(name_keyboard, lv_color_hex(0x192A46), LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(name_keyboard, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(name_keyboard, lv_color_hex(0x1F3150), LV_STATE_DEFAULT);
        lv_obj_set_style_radius(name_keyboard, 6, LV_STATE_DEFAULT);
        // 键盘尺寸适配屏幕
        lv_obj_set_size(name_keyboard, LV_HOR_RES, LV_VER_RES/3);
        // 键盘底部对齐
        lv_obj_align(name_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_add_event_cb(name_keyboard, hide_name_keyboard, LV_EVENT_READY, NULL);
    }
    
    // 2. 显示键盘并关联输入框
    lv_obj_clear_flag(name_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(name_keyboard, name_input);
    
    // 3. 确保键盘在最上层
    lv_obj_move_foreground(name_keyboard);
}

// 关闭自定义弹窗的通用函数
static void close_custom_popup(void)
{
    // 隐藏键盘
    if(name_keyboard != NULL && lv_obj_is_valid(name_keyboard)) {
        lv_obj_add_flag(name_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
    
    // 隐藏遮罩层
    if(bg_mask_layer != NULL && lv_obj_is_valid(bg_mask_layer)) {
        lv_obj_add_flag(bg_mask_layer, LV_OBJ_FLAG_HIDDEN);
    }
    // 销毁自定义弹窗
    if(custom_popup != NULL && lv_obj_is_valid(custom_popup)) {
        lv_obj_del(custom_popup);
        custom_popup = NULL;
    }
}

// 弹窗按钮点击回调（yes/no）
static void popup_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *btn = lv_event_get_target(e);
    const char *btn_text = lv_label_get_text(lv_obj_get_child(btn, 0));
    
    // 处理yes/no逻辑
    if(btn_text != NULL) {
        if(strcmp(btn_text, "yes") == 0) {
            LV_LOG_USER("点击yes,执行添加成员逻辑");
            lv_obj_t *name_input = lv_obj_get_child(custom_popup, 2);
            if(name_input != NULL && lv_obj_is_valid(name_input)) {
                const char *input_name = lv_textarea_get_text(name_input);
                LV_LOG_USER("输入的成员名称：%s", input_name);
            }
        } else if(strcmp(btn_text, "no") == 0) {
            LV_LOG_USER("点击no,取消添加");
        }
    }
    close_custom_popup();
}

// 关闭按钮（X）点击回调
static void popup_close_btn_cb(lv_event_t *e)
{
    close_custom_popup();
}

/**
 * @brief 其他成员添加按钮点击回调函数
 * 
 * 处理添加其他成员按钮点击事件，根据当前成员数量判断是否允许  成员。
 * 如果成员数量未达上限，会弹出输入框请求成员名称，
 * 并根据用户输入创建新的其他成员卡片。
 * 
 * @param e 事件对象指针，包含点击事件信息
 */
void other_member_add_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *user_manage_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(user_manage_scr == NULL || other_member_scr == NULL) {
        LV_LOG_WARN("other_member_add_click_cb: screen obj is NULL!");
        return;
    }

    // 1. 先关闭旧弹窗
    close_custom_popup();

    // 2. 创建/显示背景遮罩层
    create_bg_mask_layer(other_member_scr);

    // 3. 创建自定义弹窗主体
    custom_popup = create_container
    (other_member_scr, 100, 455, 600, 423, lv_color_hex(0xE0EDFF), LV_OPA_100, 16, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(custom_popup, 0, LV_STATE_DEFAULT);

    // 4. 名称区域
    lv_obj_t *name_label = create_text_label
    (custom_popup, "name:", &lv_font_montserrat_24, lv_color_hex(0x7C7C7C), 51, 69, LV_OPA_100);
    
    // 创建可编辑的文本输入框
    lv_obj_t *name_input = lv_textarea_create(custom_popup);
    lv_obj_clear_flag(name_input, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(name_input, 382, 44);
    lv_obj_set_pos(name_input, 136, 64);
    lv_obj_set_style_bg_color(name_input, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(name_input, lv_color_hex(0x333333), LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(name_input, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(name_input, 6, LV_STATE_DEFAULT);
    lv_textarea_set_placeholder_text(name_input, "0-8");
    lv_textarea_set_max_length(name_input, 8);
    lv_textarea_set_one_line(name_input, true);
    lv_obj_set_style_text_font(name_input, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    lv_obj_add_flag(name_input, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(name_input, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(name_input, name_input_click_cb, LV_EVENT_CLICKED, NULL);

    // 5. 初始头像
    lv_obj_t *avatar_title = create_text_label
    (custom_popup, "Avatar:", &lv_font_montserrat_24, lv_color_hex(0x7C7C7C), 41, 124, LV_OPA_100);
    lv_obj_t *avatar_con = create_container
    (custom_popup, 136, 171, 298, 192, lv_color_hex(0xFFFFFF), LV_OPA_100, 6, lv_color_hex(0x1F3150), 0, LV_OPA_90); 
    lv_obj_set_style_pad_all(avatar_con, 0, LV_STATE_DEFAULT);

    lv_color_t avatar_colors[] = {
        lv_color_hex(0xFFDADA), lv_color_hex(0xD0FFD0), lv_color_hex(0xFFD0FF),
        lv_color_hex(0xD0D0FF), lv_color_hex(0xFFFFD0), lv_color_hex(0xD0FFFF)
    };
    for(uint8_t row = 0; row < 2; row++) {
        for(uint8_t col = 0; col < 3; col++) {
            uint8_t avatar_idx = row * 3 + col;
            lv_obj_t *avatar = create_container
            (avatar_con, 
            avatar_x_offset[col], avatar_y_offset[row],
            80, 80,
            avatar_colors[avatar_idx], LV_OPA_100,
            0, lv_color_hex(0x1F3150), 0, LV_OPA_90);
            
            lv_obj_add_flag(avatar, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(avatar, LV_OPA_80, LV_STATE_PRESSED);
            lv_obj_add_event_cb(avatar, avatar_click_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)avatar_idx);
        }
    }

    // 关闭X按钮（右上角）
    lv_obj_t *close_img = create_image_obj(custom_popup, "H:X.png", 540, 20);
    g_name_input = name_input;
    lv_obj_add_flag(close_img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(close_img, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(close_img, popup_close_btn_cb, LV_EVENT_CLICKED, NULL);

    // 7. 确保弹窗在最上层
    lv_obj_move_foreground(custom_popup);
}

// 计算成员列表需要的最小遮罩层高度
static lv_coord_t get_member_list_max_height(void)
{
    lv_coord_t max_bottom_y = 1000; // 默认最小高度1000
    lv_coord_t card_height = 175;   // 单个成员卡片的高度（和create_other_member_card中一致）
    
    // 遍历所有有效成员，找到最底部成员的Y坐标 + 卡片高度
    for(uint8_t i = 0; i < MAX_OTHER_MEMBER_COUNT; i++) {
        if(other_member_list[i].is_valid && g_member_cards[i] != NULL && lv_obj_is_valid(g_member_cards[i])) {
            // 获取卡片的Y坐标（相对于滚动容器） + 卡片高度 = 卡片底部Y坐标
            lv_coord_t card_y = lv_obj_get_y(g_member_cards[i]);
            lv_coord_t card_bottom_y = card_y + card_height;
            
            // 如果当前卡片底部超过默认高度，更新最大高度
            if(card_bottom_y > max_bottom_y) {
                max_bottom_y = card_bottom_y;
            }
        }
    }
    
    // 额外加20px余量，避免刚好卡边
    return max_bottom_y;
}

// 通用遮罩层创建函数
static void create_bg_mask_layer(lv_obj_t *target_container)
{
    if(bg_mask_layer != NULL && lv_obj_is_valid(bg_mask_layer)) {
        lv_obj_del(bg_mask_layer);
        bg_mask_layer = NULL;
    }
    
    // 1. 创建遮罩层（父容器为滚动容器）
    bg_mask_layer = lv_obj_create(target_container);
    
    // 2. 动态计算遮罩层高度：取默认1000 和 成员列表最大高度的较大值
    lv_coord_t mask_width = 800;  // 固定宽度（屏幕宽度）
    lv_coord_t mask_height = get_member_list_max_height(); // 动态高度
    
    // 3. 设置遮罩层尺寸和位置
    lv_obj_set_size(bg_mask_layer, mask_width, mask_height);
    lv_obj_set_pos(bg_mask_layer, 0, 0); // 相对于滚动容器的左上角
    
    // 4. 遮罩层样式
    lv_obj_set_style_bg_color(bg_mask_layer, lv_color_hex(0x16243B), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(bg_mask_layer, LV_OPA_70, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(bg_mask_layer, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(bg_mask_layer, 0, LV_STATE_DEFAULT);
    
    // 5. 保留点击标志，拦截点击事件
    lv_obj_add_flag(bg_mask_layer, LV_OBJ_FLAG_CLICKABLE);
    
    // 6. 强制置顶
    lv_obj_move_foreground(bg_mask_layer);
}

// ====================== 工具函数：显示/隐藏所有成员的删除按钮 ======================
static void show_all_member_delete_btn(bool show)
{
    if(other_member_scr == NULL || !lv_obj_is_valid(other_member_scr)) return;
    
    for(uint8_t idx = 0; idx < MAX_OTHER_MEMBER_COUNT; idx++) {
        if(!other_member_list[idx].is_valid) continue;
        
        // 处理delete_hid
        if(g_delete_hid_containers[idx] != NULL && lv_obj_is_valid(g_delete_hid_containers[idx])) {
            if(show) {
                if(!g_member_selected[idx]) { // 未选中才显示
                    lv_obj_clear_flag(g_delete_hid_containers[idx], LV_OBJ_FLAG_HIDDEN);
                    lv_obj_set_style_opa(g_delete_hid_containers[idx], LV_OPA_100, LV_STATE_DEFAULT);
                }
            } else {
                lv_obj_add_flag(g_delete_hid_containers[idx], LV_OBJ_FLAG_HIDDEN);
                // 核心：重置delete_hid样式，避免残留
                lv_obj_set_style_opa(g_delete_hid_containers[idx], LV_OPA_100, LV_STATE_DEFAULT);
            }
        }
        
        // 处理delete_flag
        if(g_delete_flag_imgs[idx] != NULL && lv_obj_is_valid(g_delete_flag_imgs[idx])) {
            if(!show) {
                lv_obj_del(g_delete_flag_imgs[idx]);
                g_delete_flag_imgs[idx] = NULL;
                // 核心：重置成员卡片样式
                if(g_member_cards[idx] != NULL && lv_obj_is_valid(g_member_cards[idx])) {
                    lv_obj_set_style_opa(g_member_cards[idx], LV_OPA_100, LV_STATE_DEFAULT);
                }
            }
        }
    }
    
    if(!show) {
        // 退出删除模式时，彻底重置所有选中状态
        for(uint8_t i = 0; i < MAX_OTHER_MEMBER_COUNT; i++) {
            g_member_selected[i] = false;
        }
    }
}

// 检查并更新添加按钮状态
static void update_add_member_btn_state(void)
{
    if(other_member_add_con != NULL && lv_obj_is_valid(other_member_add_con)) {
        if(member_count >= MAX_OTHER_MEMBER_COUNT) {
            lv_obj_clear_flag(other_member_add_con, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(other_member_add_con, LV_OPA_50, LV_STATE_DEFAULT);
        } else {
            lv_obj_add_flag(other_member_add_con, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(other_member_add_con, LV_OPA_100, LV_STATE_DEFAULT);
        }
    }
    if(other_member_add != NULL && lv_obj_is_valid(other_member_add_con)) {
        if(member_count >= MAX_OTHER_MEMBER_COUNT) {
            lv_obj_clear_flag(other_member_add, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(other_member_add, LV_OPA_50, LV_STATE_DEFAULT);
        } else {
            lv_obj_add_flag(other_member_add, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(other_member_add, LV_OPA_100, LV_STATE_DEFAULT);
        }
    }
}


#else 
#include "lv_other_member.h"
#include "lv_a_enroll_opt.h"
#include "stdio.h"
#include "string.h"

// ====================== 全局UI对象数组 ======================
lv_obj_t *g_other_finger_labels[MAX_OTHER_MEMBER_COUNT] = {NULL};
lv_obj_t *g_other_pwd_labels[MAX_OTHER_MEMBER_COUNT]    = {NULL};
lv_obj_t *g_other_finger_imgs[MAX_OTHER_MEMBER_COUNT]   = {NULL};
lv_obj_t *g_other_pwd_imgs[MAX_OTHER_MEMBER_COUNT]      = {NULL};

// ====================== 数据结构定义 ======================
typedef struct {
    uint8_t member_idx;
    lv_obj_t *parent_scr;
    other_member_info_t *info;
} enroll_cb_data_t;

// ====================== 成员管理全局变量 ======================
uint8_t g_selected_other_member_idx = MAX_OTHER_MEMBER_COUNT;
other_member_info_t other_member_list[MAX_OTHER_MEMBER_COUNT] = {0};

// ====================== 删除模式全局变量 ======================
static bool g_is_delete_mode = false;
static lv_obj_t *g_delete_cancel_btn  = NULL;
static lv_obj_t *g_delete_confirm_btn = NULL;
static lv_obj_t *g_delete_flag_imgs[MAX_OTHER_MEMBER_COUNT]    = {NULL};
static lv_obj_t *g_delete_hid_containers[MAX_OTHER_MEMBER_COUNT] = {NULL};
static bool g_member_selected[MAX_OTHER_MEMBER_COUNT] = {false};
static lv_obj_t *g_member_cards[MAX_OTHER_MEMBER_COUNT] = {NULL};

static lv_obj_t *delete_img        = NULL;
static lv_obj_t *back_btn          = NULL;
static lv_obj_t *other_member_label = NULL;

// ====================== 弹窗&键盘全局变量 ======================
static lv_obj_t *bg_mask_layer     = NULL;
static lv_obj_t *custom_popup      = NULL;
static lv_obj_t *other_member_scr  = NULL;
static lv_style_t other_member_grad_style;
static bool other_member_style_inited = false;
static lv_obj_t *name_keyboard     = NULL;
static lv_color_t selected_avatar_color = {0};
static uint8_t member_count        = 0;

// ====================== 添加成员按钮 ======================
lv_obj_t *other_member_add_con = NULL;
lv_obj_t *other_member_add     = NULL;



// ====================== 函数声明 ======================
static void close_custom_popup(void);
static lv_obj_t *create_other_member_card(lv_obj_t *parent, const char *member_name, lv_color_t avatar_color, uint8_t member_idx);
static void update_add_member_btn_state(void);
void ui_other_member_create(lv_obj_t *user_manage_scr);
static void restore_other_members(lv_obj_t *parent);
static void delete_cancel_click_cb(lv_event_t *e);
static void delete_confirm_click_cb(lv_event_t *e);
static void delete_popup_cancel_cb(lv_event_t *e);
static void delete_popup_ok_cb(lv_event_t *e);
static void show_all_member_delete_btn(bool show);
static void delete_img_click_cb(lv_event_t *e);
static void switch_delete_mode(bool enter);
static void member_delete_btn_click_cb(lv_event_t *e);
static void member_card_click_cb(lv_event_t *e);
static void create_bg_mask_layer(lv_obj_t *target_container);
void other_member_add_click_cb(lv_event_t *e);
static void hide_name_keyboard(lv_event_t *e);
static void name_input_click_cb(lv_event_t *e);
static void avatar_click_cb(lv_event_t *e);
static lv_coord_t get_member_list_max_height(void);
void update_other_member_count_ui(uint8_t member_idx);
void other_member_back_btn_click_cb(lv_event_t *e);

// ====================== 样式初始化 ======================
static void init_other_member_styles(void)
{
    if(!other_member_style_inited) {
        lv_style_init(&other_member_grad_style);
        other_member_style_inited = true;
    }
}

// ====================== 屏幕加载回调 ======================
static void other_member_scr_load_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *scr = lv_event_get_target(e);
    if(!is_lv_obj_valid(scr)) return;
    update_status_bar_parent(scr);
}

// ====================== UI创建主函数 ======================
void ui_other_member_create(lv_obj_t *user_manage_scr)
{
    if(selected_avatar_color.ch.red == 0 && selected_avatar_color.ch.green == 0 && selected_avatar_color.ch.blue == 0) {
        selected_avatar_color = lv_color_hex(0xEDF4FF);
    }
    init_other_member_styles();

    if(user_manage_scr == NULL) {
        LV_LOG_WARN("ui_other_member_create: user_manage_scr is NULL!");
        return;
    }

    if(is_lv_obj_valid(other_member_scr)) {
        lv_obj_del(other_member_scr);
        other_member_scr = NULL;
    }
    other_member_scr = lv_obj_create(NULL);
    lv_obj_add_event_cb(other_member_scr, other_member_scr_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);

    bg_mask_layer = NULL;
    name_keyboard = NULL;
    selected_avatar_color = lv_color_hex(0xEDF4FF);

    lv_style_reset(&other_member_grad_style);
    lv_style_set_bg_color(&other_member_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&other_member_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&other_member_grad_style, LV_GRAD_DIR_VER);
    lv_style_set_bg_main_stop(&other_member_grad_style, 0);
    lv_style_set_bg_grad_stop(&other_member_grad_style, 255);
    lv_obj_add_style(other_member_scr, &other_member_grad_style, LV_STATE_DEFAULT);

    other_member_label = create_text_label(other_member_scr, "普通成员", &eques_bold_36, lv_color_hex(0xFFFFFF), 83, 80, LV_OPA_100);

    // 添加成员容器
    other_member_add_con = create_container
    (other_member_scr,47,150,300,200,lv_color_hex(0x192A46), LV_OPA_100, 6,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_add_flag(other_member_add_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(other_member_add_con, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(other_member_add_con, other_member_add_click_cb, LV_EVENT_CLICKED, user_manage_scr);

    // 添加按钮 + 十字图标
    other_member_add = create_container
    (other_member_scr,161,213,74,74,lv_color_hex(0x2F476F), LV_OPA_100, 100,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_t *divider_line1 = lv_line_create(other_member_scr);
    static const lv_point_t divider_points1[] = {{173, 251}, {223, 251}};
    config_divider_line_style(divider_line1, divider_points1, 2, 0x617C9D, 8, LV_OPA_100);
    lv_obj_t *divider_line2 = lv_line_create(other_member_scr);
    static const lv_point_t divider_points2[] = {{198, 225}, {198, 275}};
    config_divider_line_style(divider_line2, divider_points2, 2, 0x617C9D, 8, LV_OPA_100);

    lv_obj_add_flag(other_member_add, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(other_member_add, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(other_member_add, other_member_add_click_cb, LV_EVENT_CLICKED, user_manage_scr);

    // 删除按钮
    delete_img = create_container
    (other_member_scr,928,81,55,30,lv_color_hex(0x192A46), LV_OPA_0, 0,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(delete_img, 0, LV_STATE_DEFAULT);
    //三个点（不可点击）
    lv_obj_t *cir1 =create_container_circle
    (delete_img, 5, 13, 9,true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_t *cir2 =create_container_circle
    (delete_img, 25, 13, 9,true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_t *cir3 =create_container_circle
    (delete_img, 45, 13, 9,true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_clear_flag(cir1, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(cir2, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(cir3, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_add_flag(delete_img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(delete_img, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(delete_img, delete_img_click_cb, LV_EVENT_CLICKED, NULL);

    // 返回按钮
    back_btn = create_text_label
    (other_member_scr, ICON_CHEVORN_LEFT, &fontawesome_icon_32, lv_color_hex(0xFFFFFF), 52, 84, LV_OPA_100);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(back_btn,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn,LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn,other_member_back_btn_click_cb,LV_EVENT_CLICKED,user_manage_scr);

    // 恢复成员列表
    restore_other_members(other_member_scr);
    lv_scr_load(other_member_scr);
}

// ====================== 外部函数声明 ======================
extern void ui_user_manage_create(lv_obj_t *homepage_scr);
extern void destroy_user_manage(void);

// ====================== 入口按钮回调 ======================
void other_member_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(parent_scr == NULL) return;

    ui_other_member_create(parent_scr);
    lv_scr_load(other_member_scr);
    update_status_bar_parent(other_member_scr);
    destroy_user_manage();
    LV_LOG_WARN("进入家庭成员，销毁用户管理");
}

// ====================== 返回按钮回调 ======================
void other_member_back_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);
    
    lv_obj_t *current_del_scr = lv_disp_get_scr_act(NULL);
    if(!lv_obj_is_valid(current_del_scr)) return;

    if(current_del_scr == other_member_scr) {
        ui_user_manage_create(parent_scr);  
        lv_obj_del(current_del_scr);
        other_member_scr = NULL;
        LV_LOG_WARN("Other member response: Rebuild the user manage and destroy the other member interface");
        return;
    }
}

// ====================== 删除模式相关 ======================
static void delete_img_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    switch_delete_mode(true);
}

static void delete_cancel_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    switch_delete_mode(false);
}

static void delete_popup_cancel_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *popup = (lv_obj_t *)lv_event_get_user_data(e);
    
    if(popup != NULL && lv_obj_is_valid(popup)) {
        lv_obj_del(popup);
    }
    if(bg_mask_layer != NULL && lv_obj_is_valid(bg_mask_layer)) {
        lv_obj_add_flag(bg_mask_layer, LV_OBJ_FLAG_HIDDEN);
    }
}

// 成员数据紧凑化
static void compact_other_members(void)
{
    uint8_t valid_idx = 0; 
    
    for (uint8_t i = 0; i < MAX_OTHER_MEMBER_COUNT; i++)
    {
        if (other_member_list[i].is_valid)
        {
            if (valid_idx != i)
            {
                other_member_list[valid_idx] = other_member_list[i];
                memset(&other_member_list[i], 0, sizeof(other_member_info_t));
            }
            valid_idx++;
        }
    }
    
    member_count = valid_idx;
}

// 删除确认按钮
static void delete_confirm_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    bool has_selected = false;
    for(uint8_t i = 0; i < MAX_OTHER_MEMBER_COUNT; i++) {
        if(g_member_selected[i]) {
            has_selected = true;
            break;
        }
    }
    if(!has_selected) {
        LV_LOG_USER("未选择要删除的成员");
        return;
    }
    
    create_bg_mask_layer(other_member_scr);
    
    lv_obj_t *confirm_popup = create_container(
        other_member_scr, 212, 151, 600, 297, 
        lv_color_hex(0xE0EDFF), LV_OPA_100, 
        16, lv_color_hex(0x1F3150), 0, LV_OPA_90
    );
    
    create_text_label(confirm_popup, "确认删除此用户吗？", &eques_regular_32, lv_color_hex(0x000000), 156, 52, LV_OPA_100);
    
    lv_obj_t *confirm_btn = create_custom_gradient_container
    (confirm_popup, 190, 141, 220, 44, 6, 0x006BDC, 0x00BDBD, LV_GRAD_DIR_HOR, 0, 225, LV_OPA_100);
    lv_obj_set_style_pad_all(confirm_btn, 0, LV_STATE_DEFAULT);
    lv_obj_t *confirm_label = create_text_label(
    confirm_btn, "确定", &eques_bold_24, lv_color_hex(0x000000), 0, 0, LV_OPA_100);
    lv_obj_set_align(confirm_label, LV_ALIGN_CENTER);
    lv_obj_add_flag(confirm_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(confirm_btn, delete_popup_ok_cb, LV_EVENT_CLICKED, confirm_popup);
    
    lv_obj_t *cancel_btn = create_container
    (confirm_popup, 190, 210, 220, 44, lv_color_hex(0xE0EDFF), LV_OPA_100, 8, lv_color_hex(0xFF3333), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(cancel_btn, 0, LV_STATE_DEFAULT);
    lv_obj_t *cancel_label = create_text_label
    (cancel_btn, "取消", &eques_bold_24, lv_color_hex(0xBDBDBD), 0, 0, LV_OPA_100);
    lv_obj_set_align(cancel_label, LV_ALIGN_CENTER);
    lv_obj_add_flag(cancel_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(cancel_btn, delete_popup_cancel_cb, LV_EVENT_CLICKED, confirm_popup);
    
    lv_obj_move_foreground(confirm_popup);
}

// 删除标记点击（取消选中）
static void delete_flag_click_cb(lv_event_t *e)
{
    if(e == NULL || !g_is_delete_mode) return;
    
    uint8_t idx = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    lv_obj_t *delete_flag_img = lv_event_get_target(e);
    
    if(idx >= MAX_OTHER_MEMBER_COUNT || !g_member_selected[idx]) {
        LV_LOG_WARN("Invalid index or not selected: idx=%d", idx);
        return;
    }

    LV_LOG_USER("取消选中成员：%d", idx);
    
    if(delete_flag_img != NULL && lv_obj_is_valid(delete_flag_img)) {
        lv_obj_del(delete_flag_img);
    }
    g_delete_flag_imgs[idx] = NULL;
    g_member_selected[idx] = false;
    
    if(g_delete_hid_containers[idx] != NULL && lv_obj_is_valid(g_delete_hid_containers[idx])) {
        lv_obj_clear_flag(g_delete_hid_containers[idx], LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_opa(g_delete_hid_containers[idx], LV_OPA_100, LV_STATE_DEFAULT);
        LV_LOG_USER("成功显示成员%d的delete_hid容器", idx);
    } else {
        LV_LOG_WARN("成员%d的delete_hid容器不存在或无效", idx);
    }
    if(g_member_cards[idx] != NULL && lv_obj_is_valid(g_member_cards[idx])) {
        lv_obj_set_style_opa(g_member_cards[idx], LV_OPA_100, LV_STATE_DEFAULT);
    }
}

// 成员删除按钮点击
static void member_delete_btn_click_cb(lv_event_t *e)
{
    if(e == NULL || !g_is_delete_mode) return;
    
    uint8_t idx = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    lv_obj_t *delete_hid = lv_event_get_target(e);
    
    if(idx >= MAX_OTHER_MEMBER_COUNT || g_member_selected[idx]) {
        LV_LOG_WARN("Invalid index or already selected: idx=%d", idx);
        return;
    }

    LV_LOG_USER("选中成员：%d", idx);
    
    lv_obj_add_flag(delete_hid, LV_OBJ_FLAG_HIDDEN);
    
    lv_obj_t *member_con = lv_obj_get_parent(delete_hid);
    if(member_con == NULL || !lv_obj_is_valid(member_con)) {
        LV_LOG_WARN("成员%d的卡片容器无效", idx);
        return;
    }
    
    if(g_delete_flag_imgs[idx] != NULL && lv_obj_is_valid(g_delete_flag_imgs[idx])) {
        lv_obj_del(g_delete_flag_imgs[idx]);
        g_delete_flag_imgs[idx] = NULL;
    }
    
    g_delete_flag_imgs[idx] = create_text_label(member_con, ICON_CHECK, &fontawesome_icon_32, lv_color_hex(0xFFFFFF), 250, 10, LV_OPA_100);
    if(g_delete_flag_imgs[idx] != NULL && lv_obj_is_valid(g_delete_flag_imgs[idx])) {
        lv_obj_add_event_cb(g_delete_flag_imgs[idx], delete_flag_click_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)idx);
        lv_obj_add_flag(g_delete_flag_imgs[idx], LV_OBJ_FLAG_CLICKABLE);
        g_member_selected[idx] = true;
        LV_LOG_USER("成功创建成员%d的delete_flag图片", idx);

        if(g_member_cards[idx] != NULL && lv_obj_is_valid(g_member_cards[idx])) {
            //lv_obj_set_style_opa(g_member_cards[idx], LV_OPA_50, LV_STATE_DEFAULT);
        }
    } else {
        LV_LOG_WARN("创建成员%d的delete_flag图片失败", idx);
        lv_obj_clear_flag(delete_hid, LV_OBJ_FLAG_HIDDEN);
    }
}

// 删除确认弹窗确认逻辑
static void delete_popup_ok_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *popup = (lv_obj_t *)lv_event_get_user_data(e);
    
    bool has_delete = false;
    for(uint8_t i = 0; i < MAX_OTHER_MEMBER_COUNT; i++) {
        if(g_member_selected[i]) {
            clear_member_all_biometrics(i, MEMBER_TYPE_OTHER);
            memset(&other_member_list[i], 0, sizeof(other_member_info_t));
            has_delete = true;
        }
    }
    
    if(has_delete) {
        compact_other_members();
    }
    
    for(uint8_t i = 0; i < MAX_OTHER_MEMBER_COUNT; i++) {
        if(g_delete_flag_imgs[i] != NULL && lv_obj_is_valid(g_delete_flag_imgs[i])) {
            lv_obj_del(g_delete_flag_imgs[i]);
            g_delete_flag_imgs[i] = NULL;
        }
        if(g_delete_hid_containers[i] != NULL && lv_obj_is_valid(g_delete_hid_containers[i])) {
            lv_obj_del(g_delete_hid_containers[i]);
            g_delete_hid_containers[i] = NULL;
        }
        if(g_member_cards[i] != NULL && lv_obj_is_valid(g_member_cards[i])) {
            lv_obj_del(g_member_cards[i]);
            g_member_cards[i] = NULL;
        }
        g_member_selected[i] = false;
    }
    
    if(other_member_scr != NULL && lv_obj_is_valid(other_member_scr)) {
        restore_other_members(other_member_scr);
        lv_obj_invalidate(other_member_scr);
        lv_refr_now(NULL);
        update_add_member_btn_state();
    }
    
    if(popup != NULL && lv_obj_is_valid(popup)) lv_obj_del(popup);
    if(bg_mask_layer != NULL && lv_obj_is_valid(bg_mask_layer)) {
        lv_obj_add_flag(bg_mask_layer, LV_OBJ_FLAG_HIDDEN);
    }
    switch_delete_mode(false);
}

// 切换删除模式
static void switch_delete_mode(bool enter)
{
    g_is_delete_mode = enter;
    
    if(enter) {
        if(back_btn != NULL && lv_obj_is_valid(back_btn)) {
            lv_obj_add_flag(back_btn, LV_OBJ_FLAG_HIDDEN);
        }
        if(delete_img != NULL && lv_obj_is_valid(delete_img)) {
            lv_obj_add_flag(delete_img, LV_OBJ_FLAG_HIDDEN);
        }
        if(other_member_label != NULL && lv_obj_is_valid(other_member_label)) {
            lv_obj_add_flag(other_member_label, LV_OBJ_FLAG_HIDDEN);
        }
        
        if(g_delete_cancel_btn == NULL || !lv_obj_is_valid(g_delete_cancel_btn)) {
            g_delete_cancel_btn = create_text_label(
                other_member_scr, "取消", &eques_regular_24, 
                lv_color_hex(0x00BDBD), 48, 90, LV_OPA_100
            );
            lv_obj_add_flag(g_delete_cancel_btn, LV_OBJ_FLAG_CLICKABLE);
            //lv_obj_set_style_opa(g_delete_cancel_btn, LV_OPA_80, LV_STATE_PRESSED);
            lv_obj_add_event_cb(g_delete_cancel_btn, delete_cancel_click_cb, LV_EVENT_CLICKED, NULL);
        } else {
            lv_obj_clear_flag(g_delete_cancel_btn, LV_OBJ_FLAG_HIDDEN);
        }
        
        if(g_delete_confirm_btn == NULL || !lv_obj_is_valid(g_delete_confirm_btn)) {
            g_delete_confirm_btn = create_text_label(
                other_member_scr, "删除", &eques_regular_24, 
                lv_color_hex(0xFF0000), 928, 90, LV_OPA_100
            );
            lv_obj_add_flag(g_delete_confirm_btn, LV_OBJ_FLAG_CLICKABLE);
            //lv_obj_set_style_opa(g_delete_confirm_btn, LV_OPA_80, LV_STATE_PRESSED);
            lv_obj_add_event_cb(g_delete_confirm_btn, delete_confirm_click_cb, LV_EVENT_CLICKED, NULL);
        } else {
            lv_obj_clear_flag(g_delete_confirm_btn, LV_OBJ_FLAG_HIDDEN);
        }
        
        show_all_member_delete_btn(true);

        for(uint8_t i = 0; i < MAX_OTHER_MEMBER_COUNT; i++) {
            if(g_member_cards[i] != NULL && lv_obj_is_valid(g_member_cards[i])) {
                lv_obj_clear_flag(g_member_cards[i], LV_OBJ_FLAG_CLICKABLE);
                lv_obj_set_style_opa(g_member_cards[i], LV_OPA_100, LV_STATE_DEFAULT);
            }
        }
        
        if(other_member_add_con != NULL && lv_obj_is_valid(other_member_add_con)) {
            lv_obj_clear_flag(other_member_add_con, LV_OBJ_FLAG_CLICKABLE);
            //lv_obj_set_style_opa(other_member_add_con, LV_OPA_50, LV_STATE_DEFAULT);
        }
        if(other_member_add != NULL && lv_obj_is_valid(other_member_add)) {
            lv_obj_clear_flag(other_member_add, LV_OBJ_FLAG_CLICKABLE);
            //lv_obj_set_style_opa(other_member_add, LV_OPA_50, LV_STATE_DEFAULT);
        }
    } else {
        if(back_btn != NULL && lv_obj_is_valid(back_btn)) {
            lv_obj_clear_flag(back_btn, LV_OBJ_FLAG_HIDDEN);
        }
        if(delete_img != NULL && lv_obj_is_valid(delete_img)) {
            lv_obj_clear_flag(delete_img, LV_OBJ_FLAG_HIDDEN);
        }
        if(other_member_label != NULL && lv_obj_is_valid(other_member_label)) {
            lv_obj_clear_flag(other_member_label, LV_OBJ_FLAG_HIDDEN);
        }
        
        if(g_delete_cancel_btn != NULL && lv_obj_is_valid(g_delete_cancel_btn)) {
            lv_obj_add_flag(g_delete_cancel_btn, LV_OBJ_FLAG_HIDDEN);
        }
        if(g_delete_confirm_btn != NULL && lv_obj_is_valid(g_delete_confirm_btn)) {
            lv_obj_add_flag(g_delete_confirm_btn, LV_OBJ_FLAG_HIDDEN);
        }
        
        show_all_member_delete_btn(false);

        for(uint8_t i = 0; i < MAX_OTHER_MEMBER_COUNT; i++) {
            if(g_member_cards[i] != NULL && lv_obj_is_valid(g_member_cards[i])) {
                lv_obj_add_flag(g_member_cards[i], LV_OBJ_FLAG_CLICKABLE);
                lv_obj_set_style_opa(g_member_cards[i], LV_OPA_100, LV_STATE_DEFAULT);
            }
        }
        
        update_add_member_btn_state();
    }
}

// 显示/隐藏所有成员删除按钮
static void show_all_member_delete_btn(bool show)
{
    if(other_member_scr == NULL || !lv_obj_is_valid(other_member_scr)) return;

    for(uint8_t idx = 0; idx < MAX_OTHER_MEMBER_COUNT; idx++) {
        if(!other_member_list[idx].is_valid) continue;

        if(g_delete_hid_containers[idx] != NULL && lv_obj_is_valid(g_delete_hid_containers[idx])) {
            if(show) {
                if(!g_member_selected[idx]) {
                    lv_obj_clear_flag(g_delete_hid_containers[idx], LV_OBJ_FLAG_HIDDEN);
                    lv_obj_set_style_opa(g_delete_hid_containers[idx], LV_OPA_100, LV_STATE_DEFAULT);
                }
            } else {
                lv_obj_add_flag(g_delete_hid_containers[idx], LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_style_opa(g_delete_hid_containers[idx], LV_OPA_100, LV_STATE_DEFAULT);
            }
        }

        if(g_delete_flag_imgs[idx] != NULL && lv_obj_is_valid(g_delete_flag_imgs[idx])) {
            if(!show) {
                lv_obj_del(g_delete_flag_imgs[idx]);
                g_delete_flag_imgs[idx] = NULL;
                if(g_member_cards[idx] != NULL && lv_obj_is_valid(g_member_cards[idx])) {
                    lv_obj_set_style_opa(g_member_cards[idx], LV_OPA_100, LV_STATE_DEFAULT);
                }
            }
        }
    }

    if(!show) {
        for(uint8_t i = 0; i < MAX_OTHER_MEMBER_COUNT; i++) {
            g_member_selected[i] = false;
        }
    }
}


// ====================== 成员卡片 ======================
static lv_obj_t *create_other_member_card(lv_obj_t *parent, const char *member_name, lv_color_t avatar_color, uint8_t member_idx)
{
    if(parent == NULL || member_name == NULL || member_idx >= MAX_OTHER_MEMBER_COUNT) return NULL;

    const lv_coord_t col_x[] = {48, 363, 677};
    const lv_coord_t row_start_y = 150;
    const lv_coord_t card_height = 200;
    const lv_coord_t row_gap = 26;
    const lv_coord_t row_step = card_height + row_gap;

    uint16_t total_position = member_idx + 1;
    uint8_t current_row = total_position / 3;
    uint8_t current_col = total_position % 3;
    lv_coord_t card_x = col_x[current_col];
    lv_coord_t card_y = row_start_y + current_row * row_step;

    lv_obj_t *member_con = create_container
    (parent, card_x, card_y, 300, 200, lv_color_hex(0x192A46), LV_OPA_100, 6, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_user_data(member_con, (void*)(uintptr_t)member_idx);
    lv_obj_set_style_pad_all(member_con, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(member_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(member_con, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(member_con, member_card_click_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)member_idx);
    g_member_cards[member_idx] = member_con;

    // 头像
    create_container(member_con, 122, 15, 60, 60, avatar_color, LV_OPA_100, 100, avatar_color, 0, LV_OPA_90);

    // 名称
    lv_obj_t *name_label = create_text_label
    (member_con, member_name, &eques_regular_36, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align(name_label, LV_ALIGN_TOP_MID, 0, 87);

    uint8_t finger_count = other_member_list[member_idx].finger_count;
    uint8_t pwd_count = other_member_list[member_idx].pwd_count;
    char text_buf[8] = {0};

    // ==========================
    // 指纹图标 + 数字
    // ==========================
    lv_obj_t *icon_finger = lv_label_create(member_con);
    lv_label_set_text(icon_finger, ICON_FINGERPRINT_S);
    lv_obj_set_style_text_font(icon_finger, &fontawesome_icon_26, 0);
    lv_obj_set_pos(icon_finger, 88, 157);
    lv_obj_set_style_text_color(icon_finger,
        (finger_count > 0) ? lv_color_hex(0x00BDBD) : lv_color_hex(0xD4D4D4), 0);

    snprintf(text_buf, sizeof(text_buf), "%d", finger_count);
    g_other_finger_labels[member_idx] = create_text_label(member_con, text_buf, &eques_regular_24,
        (finger_count > 0) ? lv_color_hex(0x00BDBD) : lv_color_hex(0xD4D4D4), 124, 157, LV_OPA_100);

    // ==========================
    // 密码图标 + 数字
    // ==========================
    lv_obj_t *icon_pwd = lv_label_create(member_con);
    lv_label_set_text(icon_pwd, ICON_PASSWORD_S);
    lv_obj_set_style_text_font(icon_pwd, &fontawesome_icon_26, 0);
    lv_obj_set_pos(icon_pwd, 150, 157);
    lv_obj_set_style_text_color(icon_pwd,
        (pwd_count > 0) ? lv_color_hex(0x00BDBD) : lv_color_hex(0xD4D4D4), 0);

    snprintf(text_buf, sizeof(text_buf), "%d", pwd_count);
    g_other_pwd_labels[member_idx] = create_text_label(member_con, text_buf, &eques_regular_24,
        (pwd_count > 0) ? lv_color_hex(0x00BDBD) : lv_color_hex(0xD4D4D4), 186, 157, LV_OPA_100);

    // 删除按钮容器
    lv_obj_t *delete_hid = create_container
    (member_con, 250, 10, 32, 32, lv_color_hex(0x192A46), LV_OPA_100, 100, lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_add_flag(delete_hid, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(delete_hid, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(delete_hid, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(delete_hid, member_delete_btn_click_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)member_idx);
    g_delete_hid_containers[member_idx] = delete_hid;
    g_delete_flag_imgs[member_idx] = NULL;
    g_member_selected[member_idx] = false;

    return member_con;
}

// 更新成员计数UI
void update_other_member_count_ui(uint8_t member_idx)
{
    if(member_idx >= MAX_OTHER_MEMBER_COUNT) return;

    if(g_other_finger_labels[member_idx] == NULL || !lv_obj_is_valid(g_other_finger_labels[member_idx]) ||
       g_other_pwd_labels[member_idx] == NULL || !lv_obj_is_valid(g_other_pwd_labels[member_idx])) {
        return;
    }

    uint8_t finger_count = other_member_list[member_idx].finger_count;
    uint8_t pwd_count = other_member_list[member_idx].pwd_count;

    char text_buf[8] = {0};

    // ==========================
    // 更新指纹数量 + 颜色
    // ==========================
    snprintf(text_buf, sizeof(text_buf), "%d", finger_count);
    lv_label_set_text(g_other_finger_labels[member_idx], text_buf);
    lv_obj_set_style_text_color(g_other_finger_labels[member_idx],
        (finger_count > 0) ? lv_color_hex(0x00BDBD) : lv_color_hex(0xD4D4D4), LV_STATE_DEFAULT);

    // ==========================
    // 更新密码数量 + 颜色
    // ==========================
    snprintf(text_buf, sizeof(text_buf), "%d", pwd_count);
    lv_label_set_text(g_other_pwd_labels[member_idx], text_buf);
    lv_obj_set_style_text_color(g_other_pwd_labels[member_idx],
        (pwd_count > 0) ? lv_color_hex(0x00BDBD) : lv_color_hex(0xD4D4D4), LV_STATE_DEFAULT);

    lv_obj_invalidate(lv_scr_act());
}

// 成员卡片点击
static void member_card_click_cb(lv_event_t *e)
{
    if(e == NULL || g_is_delete_mode) return;
    
    uint8_t idx = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    g_selected_other_member_idx = idx;
    
    common_member_info_t member_info = {
        .type = MEMBER_TYPE_OTHER,
        .idx = idx,
        .avatar_color = other_member_list[idx].avatar_color,
        .finger_count = other_member_list[idx].finger_count,
        .pwd_count = other_member_list[idx].pwd_count,
        .card_count = 0,
        .face_count = 0
    };
    strncpy(member_info.name, other_member_list[idx].name, sizeof(member_info.name)-1);
    member_info.name[sizeof(member_info.name)-1] = '\0';
    
    static common_member_info_t g_temp_member_info;
    g_temp_member_info = member_info;
    lv_obj_t *other_member_scr = lv_obj_get_parent(lv_event_get_target(e));
    ui_enroll_create(&g_temp_member_info, other_member_scr);
}

// ====================== 添加成员弹窗 ======================
static void other_member_name_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;

    // 成员数量上限判断
    if(member_count >= MAX_OTHER_MEMBER_COUNT) {
        LV_LOG_USER("成员数量已达上限");
        close_custom_popup();
        return;
    }

    // 获取点击按钮绑定的姓名
    const char *member_name = (const char *)lv_event_get_user_data(e);
    if(member_name == NULL) {
        close_custom_popup();
        return;
    }

    // 头像颜色 统一设置为白色
    lv_color_t selected_avatar_color = lv_color_hex(0xFFFFFF);

    // 查找空位置
    uint8_t member_idx = 0;
    for(; member_idx < MAX_OTHER_MEMBER_COUNT; member_idx++) {
        if(!other_member_list[member_idx].is_valid) break;
    }
    if(member_idx >= MAX_OTHER_MEMBER_COUNT) {
        close_custom_popup();
        return;
    }

    // 保存成员信息
    strncpy(other_member_list[member_idx].name, member_name, sizeof(other_member_list[member_idx].name)-1);
    other_member_list[member_idx].avatar_color = selected_avatar_color;
    other_member_list[member_idx].is_valid = true;

    // 创建卡片
    create_other_member_card(other_member_scr, member_name, selected_avatar_color, member_idx);
    member_count++;
    update_add_member_btn_state();

    // 关闭弹窗
    close_custom_popup();
}

static void hide_name_keyboard(lv_event_t *e)
{
    (void)e;
    if(name_keyboard != NULL && lv_obj_is_valid(name_keyboard)) {
        lv_obj_add_flag(name_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

static void name_input_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *name_input = lv_event_get_target(e);

    if(name_keyboard == NULL || !lv_obj_is_valid(name_keyboard)) {
        name_keyboard = lv_keyboard_create(lv_scr_act());
        lv_obj_set_style_bg_color(name_keyboard, lv_color_hex(0x192A46), LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(name_keyboard, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(name_keyboard, lv_color_hex(0x1F3150), LV_STATE_DEFAULT);
        lv_obj_set_style_radius(name_keyboard, 6, LV_STATE_DEFAULT);
        lv_obj_set_size(name_keyboard, LV_HOR_RES, 250);
        lv_obj_align(name_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_add_event_cb(name_keyboard, hide_name_keyboard, LV_EVENT_READY, NULL);
    }

    lv_obj_clear_flag(name_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(name_keyboard, name_input);
    lv_obj_move_foreground(name_keyboard);
}

// 关闭添加成员弹窗
static void close_custom_popup(void)
{
    if(name_keyboard != NULL && lv_obj_is_valid(name_keyboard)) {
        lv_obj_add_flag(name_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
    if(bg_mask_layer != NULL && lv_obj_is_valid(bg_mask_layer)) {
        lv_obj_add_flag(bg_mask_layer, LV_OBJ_FLAG_HIDDEN);
    }
    if(custom_popup != NULL && lv_obj_is_valid(custom_popup)) {
        lv_obj_del(custom_popup);
        custom_popup = NULL;
    }
}

static void popup_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *btn = lv_event_get_target(e);
    const char *btn_text = lv_label_get_text(lv_obj_get_child(btn, 0));

    if(btn_text && strcmp(btn_text, "yes") == 0) {
    }
    close_custom_popup();
}

static void popup_close_btn_cb(lv_event_t *e)
{
    close_custom_popup();
}

// 点击添加成员（已完全改造：12个预设亲属按钮）
void other_member_add_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *user_manage_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(user_manage_scr == NULL || other_member_scr == NULL) return;

    close_custom_popup();
    create_bg_mask_layer(other_member_scr);

    // 创建弹窗
    custom_popup = create_container
    (other_member_scr, 212, 94, 600, 423, lv_color_hex(0xE0EDFF), LV_OPA_100, 16, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(custom_popup, 0, LV_STATE_DEFAULT);

    // 标题
    create_text_label(custom_popup, "名称", &eques_regular_24, lv_color_hex(0x7C7C7C), 81, 65, LV_OPA_100);

    // ===================== 12个亲属按钮 =====================
    // 第一行
    lv_obj_t *name_input01 = create_container(custom_popup, 81, 116, 128, 44, lv_color_hex(0xFFFFFF), LV_OPA_100, 6, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_t *name_input01_label = create_text_label(name_input01, "+妈妈", &eques_regular_24, lv_color_hex(0x000000), 0, 0, LV_OPA_100);
    lv_obj_align(name_input01_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(name_input01, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(name_input01, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(name_input01, other_member_name_btn_click_cb, LV_EVENT_CLICKED, "妈妈");

    lv_obj_t *name_input02 = create_container(custom_popup, 81+155, 116, 128, 44, lv_color_hex(0xFFFFFF), LV_OPA_100, 6, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_t *name_input02_label = create_text_label(name_input02, "+爸爸", &eques_regular_24, lv_color_hex(0x000000), 0, 0, LV_OPA_100);
    lv_obj_align(name_input02_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(name_input02, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(name_input02, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(name_input02, other_member_name_btn_click_cb, LV_EVENT_CLICKED, "爸爸");

    lv_obj_t *name_input03 = create_container(custom_popup, 81+155+155, 116, 128, 44, lv_color_hex(0xFFFFFF), LV_OPA_100, 6, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_t *name_input03_label = create_text_label(name_input03, "+儿子1", &eques_regular_24, lv_color_hex(0x000000), 0, 0, LV_OPA_100);
    lv_obj_align(name_input03_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(name_input03, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(name_input03, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(name_input03, other_member_name_btn_click_cb, LV_EVENT_CLICKED, "儿子1");

    // 第二行
    lv_obj_t *name_input04 = create_container(custom_popup, 81, 116+63, 128, 44, lv_color_hex(0xFFFFFF), LV_OPA_100, 6, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_t *name_input04_label = create_text_label(name_input04, "+爷爷", &eques_regular_24, lv_color_hex(0x000000), 0, 0, LV_OPA_100);
    lv_obj_align(name_input04_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(name_input04, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(name_input04, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(name_input04, other_member_name_btn_click_cb, LV_EVENT_CLICKED, "爷爷");

    lv_obj_t *name_input05 = create_container(custom_popup, 81+155, 116+63, 128, 44, lv_color_hex(0xFFFFFF), LV_OPA_100, 6, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_t *name_input05_label = create_text_label(name_input05, "+奶奶", &eques_regular_24, lv_color_hex(0x000000), 0, 0, LV_OPA_100);
    lv_obj_align(name_input05_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(name_input05, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(name_input05, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(name_input05, other_member_name_btn_click_cb, LV_EVENT_CLICKED, "奶奶");

    lv_obj_t *name_input06 = create_container(custom_popup, 81+155+155, 116+63, 128, 44, lv_color_hex(0xFFFFFF), LV_OPA_100, 6, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_t *name_input06_label = create_text_label(name_input06, "+儿子2", &eques_regular_24, lv_color_hex(0x000000), 0, 0, LV_OPA_100);
    lv_obj_align(name_input06_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(name_input06, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(name_input06, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(name_input06, other_member_name_btn_click_cb, LV_EVENT_CLICKED, "儿子2");

    // 第三行
    lv_obj_t *name_input07 = create_container(custom_popup, 81, 116+63+63, 128, 44, lv_color_hex(0xFFFFFF), LV_OPA_100, 6, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_t *name_input07_label = create_text_label(name_input07, "+外公", &eques_regular_24, lv_color_hex(0x000000), 0, 0, LV_OPA_100);
    lv_obj_align(name_input07_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(name_input07, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(name_input07, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(name_input07, other_member_name_btn_click_cb, LV_EVENT_CLICKED, "外公");

    lv_obj_t *name_input08 = create_container(custom_popup, 81+155, 116+63+63, 128, 44, lv_color_hex(0xFFFFFF), LV_OPA_100, 6, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_t *name_input08_label = create_text_label(name_input08, "+外婆", &eques_regular_24, lv_color_hex(0x000000), 0, 0, LV_OPA_100);
    lv_obj_align(name_input08_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(name_input08, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(name_input08, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(name_input08, other_member_name_btn_click_cb, LV_EVENT_CLICKED, "外婆");

    lv_obj_t *name_input09 = create_container(custom_popup, 81+155+155, 116+63+63, 128, 44, lv_color_hex(0xFFFFFF), LV_OPA_100, 6, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_t *name_input09_label = create_text_label(name_input09, "+儿子3", &eques_regular_24, lv_color_hex(0x000000), 0, 0, LV_OPA_100);
    lv_obj_align(name_input09_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(name_input09, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(name_input09, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(name_input09, other_member_name_btn_click_cb, LV_EVENT_CLICKED, "儿子3");

    // 第四行
    lv_obj_t *name_input10 = create_container(custom_popup, 81, 116+63+63+63, 128, 44, lv_color_hex(0xFFFFFF), LV_OPA_100, 6, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_t *name_input10_label = create_text_label(name_input10, "+女儿1", &eques_regular_24, lv_color_hex(0x000000), 0, 0, LV_OPA_100);
    lv_obj_align(name_input10_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(name_input10, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(name_input10, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(name_input10, other_member_name_btn_click_cb, LV_EVENT_CLICKED, "女儿1");

    lv_obj_t *name_input11 = create_container(custom_popup, 81+155, 116+63+63+63, 128, 44, lv_color_hex(0xFFFFFF), LV_OPA_100, 6, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_t *name_input11_label = create_text_label(name_input11, "+女儿2", &eques_regular_24, lv_color_hex(0x000000), 0, 0, LV_OPA_100);
    lv_obj_align(name_input11_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(name_input11, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(name_input11, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(name_input11, other_member_name_btn_click_cb, LV_EVENT_CLICKED, "女儿2");

    lv_obj_t *name_input12 = create_container(custom_popup, 81+155+155, 116+63+63+63, 128, 44, lv_color_hex(0xFFFFFF), LV_OPA_100, 6, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_t *name_input12_label = create_text_label(name_input12, "+女儿3", &eques_regular_24, lv_color_hex(0x000000), 0, 0, LV_OPA_100);
    lv_obj_align(name_input12_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(name_input12, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(name_input12, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(name_input12, other_member_name_btn_click_cb, LV_EVENT_CLICKED, "女儿3");
    // ======================================================

    // 关闭按钮
    lv_obj_t *back_con = create_custom_gradient_container
    (custom_popup, 540, 20, 40, 40, 200, 0xE0EDFF, 0xE0EDFF, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_set_style_pad_all(back_con, 0, LV_STATE_DEFAULT);
    lv_obj_t *x_line1 = lv_line_create(back_con);
    static lv_point_t x_points1[] = {{5, 5}, {35, 35}};
    config_divider_line_style(x_line1, x_points1, 2, 0x000000, 5, LV_OPA_100);
    lv_obj_t *x_line2 = lv_line_create(back_con);
    static lv_point_t x_points2[] = {{35, 5}, {5, 35}};
    config_divider_line_style(x_line2, x_points2, 2, 0x000000, 5, LV_OPA_100);
    lv_obj_add_flag(back_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_con, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_con, popup_close_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_move_foreground(custom_popup);
}
// ====================== 工具函数 ======================
static lv_coord_t get_member_list_max_height(void)
{
    lv_coord_t max_bottom_y = 1000;
    lv_coord_t card_height = 200;

    for(uint8_t i = 0; i < MAX_OTHER_MEMBER_COUNT; i++) {
        if(other_member_list[i].is_valid && g_member_cards[i] != NULL && lv_obj_is_valid(g_member_cards[i])) {
            lv_coord_t card_y = lv_obj_get_y(g_member_cards[i]);
            lv_coord_t card_bottom_y = card_y + card_height;
            if(card_bottom_y > max_bottom_y) {
                max_bottom_y = card_bottom_y;
            }
        }
    }
    return max_bottom_y + 20;
}

static void create_bg_mask_layer(lv_obj_t *target_container)
{
    if(bg_mask_layer != NULL && lv_obj_is_valid(bg_mask_layer)) {
        lv_obj_del(bg_mask_layer);
        bg_mask_layer = NULL;
    }

    bg_mask_layer = lv_obj_create(target_container);
    lv_coord_t mask_width = 1024;
    lv_coord_t mask_height = get_member_list_max_height();

    lv_obj_set_size(bg_mask_layer, mask_width, mask_height);
    lv_obj_set_pos(bg_mask_layer, 0, 0);
    lv_obj_set_style_bg_color(bg_mask_layer, lv_color_hex(0x16243B), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(bg_mask_layer, LV_OPA_70, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(bg_mask_layer, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(bg_mask_layer, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(bg_mask_layer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_move_foreground(bg_mask_layer);
}

static void update_add_member_btn_state(void)
{
    if(other_member_add_con != NULL && lv_obj_is_valid(other_member_add_con)) {
        if(member_count >= MAX_OTHER_MEMBER_COUNT) {
            lv_obj_clear_flag(other_member_add_con, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(other_member_add_con, LV_OPA_50, LV_STATE_DEFAULT);
        } else {
            lv_obj_add_flag(other_member_add_con, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(other_member_add_con, LV_OPA_100, LV_STATE_DEFAULT);
        }
    }
    if(other_member_add != NULL && lv_obj_is_valid(other_member_add)) {
        if(member_count >= MAX_OTHER_MEMBER_COUNT) {
            lv_obj_clear_flag(other_member_add, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(other_member_add, LV_OPA_50, LV_STATE_DEFAULT);
        } else {
            lv_obj_add_flag(other_member_add, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(other_member_add, LV_OPA_100, LV_STATE_DEFAULT);
        }
    }
}

// 恢复成员列表
static void restore_other_members(lv_obj_t *parent)
{
    if(parent == NULL) return;

    member_count = 0;
    memset(g_member_cards, 0, sizeof(g_member_cards));
    memset(g_delete_hid_containers, 0, sizeof(g_delete_hid_containers));
    memset(g_other_finger_labels, 0, sizeof(g_other_finger_labels));
    memset(g_other_pwd_labels, 0, sizeof(g_other_pwd_labels));
    memset(g_other_finger_imgs, 0, sizeof(g_other_finger_imgs));
    memset(g_other_pwd_imgs, 0, sizeof(g_other_pwd_imgs));

    for(uint8_t i = 0; i < MAX_OTHER_MEMBER_COUNT; i++) {
        if(other_member_list[i].is_valid) {
            create_other_member_card(parent, other_member_list[i].name, other_member_list[i].avatar_color, i);
            member_count++;
        }
    }
    update_add_member_btn_state();
}
#endif
