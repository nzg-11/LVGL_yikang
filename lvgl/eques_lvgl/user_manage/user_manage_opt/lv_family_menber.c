#if LV_EQUES_VER
#include "lv_family_menber.h"
#include "lv_a_enroll_opt.h"
#include "stdio.h"


lv_obj_t *g_finger_labels[MAX_FAMILY_MEMBER_COUNT] = {NULL};
lv_obj_t *g_pwd_labels[MAX_FAMILY_MEMBER_COUNT] = {NULL};
lv_obj_t *g_card_labels[MAX_FAMILY_MEMBER_COUNT] = {NULL};
lv_obj_t *g_face_labels[MAX_FAMILY_MEMBER_COUNT] = {NULL};

lv_obj_t *g_finger_imgs[MAX_FAMILY_MEMBER_COUNT] = {NULL};
lv_obj_t *g_pwd_imgs[MAX_FAMILY_MEMBER_COUNT] = {NULL};
lv_obj_t *g_card_imgs[MAX_FAMILY_MEMBER_COUNT] = {NULL};
lv_obj_t *g_face_imgs[MAX_FAMILY_MEMBER_COUNT] = {NULL};

// ====================== 数据结构定义 ======================
typedef struct {
    uint8_t member_idx;          // 成员唯一索引（0~7）
    lv_obj_t *parent_scr;        // 父屏幕对象
    family_member_info_t *info;  // 指向该成员的信息结构体
} enroll_cb_data_t;

// ====================== 全局变量 ======================
// 成员信息核心变量
uint8_t g_selected_member_idx = MAX_FAMILY_MEMBER_COUNT;
family_member_info_t family_member_list[MAX_FAMILY_MEMBER_COUNT] = {0};

// 删除模式专用变量
static bool g_is_delete_mode = false;
static lv_obj_t *g_delete_cancel_btn = NULL;
static lv_obj_t *g_delete_confirm_btn = NULL;
static uint8_t g_selected_delete_idx = MAX_FAMILY_MEMBER_COUNT;
static lv_obj_t *g_selected_delete_btn = NULL;
static lv_obj_t *g_delete_flag_imgs[MAX_FAMILY_MEMBER_COUNT] = {NULL};
static lv_obj_t *g_delete_hid_containers[MAX_FAMILY_MEMBER_COUNT] = {NULL};
static bool g_member_selected[MAX_FAMILY_MEMBER_COUNT] = {false};
static lv_obj_t *g_member_cards[MAX_FAMILY_MEMBER_COUNT] = {NULL};
lv_obj_t *delete_img = NULL;
lv_obj_t *back_btn = NULL;

// 全局变量
static lv_obj_t *bg_mask_layer = NULL;  
static lv_obj_t *custom_popup = NULL;
static lv_obj_t *family_menber_scr = NULL; 
static lv_style_t family_menber_grad_style;
static bool family_menber_style_inited = false;
static lv_obj_t *name_keyboard = NULL;
static lv_obj_t *g_name_input = NULL;
static lv_color_t selected_avatar_color = {0};
static lv_coord_t next_member_y = 374;
static uint8_t member_count = 0;
lv_obj_t *family_menber_add_con = NULL;
lv_obj_t *family_menber_add = NULL;

// 常量定义
static const lv_coord_t avatar_x_offset[] = {19, 110, 201};
static const lv_coord_t avatar_y_offset[] = {12, 101};

// ====================== 函数声明 ======================
// 内部函数前置声明
static void close_custom_popup(void);

// UI构建相关
static lv_obj_t *create_family_member_card(lv_obj_t *parent, lv_coord_t y_pos, const char *member_name, lv_color_t avatar_color, uint8_t member_idx);
static void update_add_member_btn_state(void);
void ui_family_menber_create(lv_obj_t *user_manage_scr);
static void restore_family_members(lv_obj_t *parent);

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
void family_menber_add_click_cb(lv_event_t *e);


// 全局样式初始化
static void init_family_menber_styles(void)
{
    if(!family_menber_style_inited) {
        lv_style_init(&family_menber_grad_style);
        family_menber_style_inited = true;
    }
}

static void family_menber_scr_load_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *scr = lv_event_get_target(e);
    if(!is_lv_obj_valid(scr)) return;
    
    update_status_bar_parent(scr);
}




void ui_family_menber_create(lv_obj_t *user_manage_scr)
{
    if(selected_avatar_color.ch.red == 0 && selected_avatar_color.ch.green == 0 && selected_avatar_color.ch.blue == 0) {
        selected_avatar_color = lv_color_hex(0xEDF4FF);
    }
    init_family_menber_styles();
    
    if(user_manage_scr == NULL) {
        LV_LOG_WARN("ui_family_menber_create: user_manage_scr is NULL!");
        return;
    }

    if(family_menber_scr == NULL) {
            family_menber_scr = lv_obj_create(NULL);
            lv_obj_add_event_cb(family_menber_scr, family_menber_scr_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
        } else {
            lv_obj_clean(family_menber_scr);
            // 重置临时状态，但保留成员数据数组
            bg_mask_layer = NULL;
            name_keyboard = NULL;
            // 注释掉：不再重置member_count和next_member_y，由restore_family_members处理
            // member_count = 0;
            // next_member_y = 374;
            selected_avatar_color = lv_color_hex(0xEDF4FF);
        }
    
    lv_style_reset(&family_menber_grad_style);
    lv_style_set_bg_color(&family_menber_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&family_menber_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&family_menber_grad_style, LV_GRAD_DIR_VER);
    lv_style_set_bg_main_stop(&family_menber_grad_style, 0);
    lv_style_set_bg_grad_stop(&family_menber_grad_style, 255);
    lv_obj_add_style(family_menber_scr, &family_menber_grad_style, LV_STATE_DEFAULT);

    lv_obj_t *family_menber_label = create_text_label
    (family_menber_scr, "family menber", &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 0, 115, LV_OPA_100);
    lv_obj_align(family_menber_label, LV_ALIGN_TOP_MID, 0, 115);

    // 添加家庭成员容器
    family_menber_add_con = create_container
    (family_menber_scr,47,195,710,175,lv_color_hex(0x192A46), LV_OPA_100, 6,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_add_flag(family_menber_add_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(family_menber_add_con, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(family_menber_add_con, family_menber_add_click_cb, LV_EVENT_CLICKED, user_manage_scr);

    //添加加号
    family_menber_add = create_container
    (family_menber_scr,360,243,79,79,lv_color_hex(0x2F476F), LV_OPA_100, 100,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    //加号线
    lv_obj_t *divider_line1 = lv_line_create(family_menber_scr);
    static const lv_point_t divider_points1[] = {{372, 282}, {426, 282}}; 
    config_divider_line_style(divider_line1, divider_points1, 2, 0x617C9D, 8, LV_OPA_100);
    lv_obj_t *divider_line2 = lv_line_create(family_menber_scr);
    static const lv_point_t divider_points2[] = {{399, 255}, {399, 309}}; 
    config_divider_line_style(divider_line2, divider_points2, 2, 0x617C9D, 8, LV_OPA_100);

    lv_obj_add_flag(family_menber_add, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(family_menber_add, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(family_menber_add, family_menber_add_click_cb, LV_EVENT_CLICKED, user_manage_scr);

    //右上角删除设置
    delete_img = create_image_obj(family_menber_scr, "H:....png", 709, 132);
    lv_obj_add_flag(delete_img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(delete_img, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(delete_img, delete_img_click_cb, LV_EVENT_CLICKED, NULL);

    // 左上角返回按钮
    back_btn = create_image_obj(family_menber_scr, "H:back.png", 52, 123);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn, back_btn_click_cb, LV_EVENT_CLICKED, user_manage_scr);

    // ====================== 恢复成员卡片 ======================
    restore_family_members(family_menber_scr);

    // 切换到设置屏幕
    lv_scr_load(family_menber_scr);
}

// 家庭成员界面回调
void family_menber_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    lv_obj_t *user_manage_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(user_manage_scr == NULL) {
        LV_LOG_WARN("family_menber_btn_click_cb: user_manage_scr is NULL!");
        return;
    }
    ui_family_menber_create(user_manage_scr);
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
    for(uint8_t i = 0; i < MAX_FAMILY_MEMBER_COUNT; i++) {
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
    create_bg_mask_layer(family_menber_scr);
    
    // 2. 创建确认弹窗
    lv_obj_t *confirm_popup = create_container(
        family_menber_scr, 100, 455, 600, 297, 
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
    if(idx >= MAX_FAMILY_MEMBER_COUNT || !g_member_selected[idx]) {
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
    
    // 4. 直接从全局数组获取delete_hid并显示（核心修复）
    if(g_delete_hid_containers[idx] != NULL && lv_obj_is_valid(g_delete_hid_containers[idx])) {
        // 强制显示delete_hid
        lv_obj_clear_flag(g_delete_hid_containers[idx], LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_opa(g_delete_hid_containers[idx], LV_OPA_100, LV_STATE_DEFAULT);
        //lv_obj_set_visible(g_delete_hid_containers[idx], true);
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
    if(idx >= MAX_FAMILY_MEMBER_COUNT || g_member_selected[idx]) {
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
    
    // 1. 批量删除所有选中的成员 + 清理所有勾选视觉残留
    bool has_delete = false;
    for(uint8_t i = 0; i < MAX_FAMILY_MEMBER_COUNT; i++) {
        if(g_member_selected[i]) {
            // 清空成员数据
            family_member_list[i].is_valid = false;
            memset(family_member_list[i].name, 0, sizeof(family_member_list[i].name));
                    family_member_list[i].finger_count = 0;
                    family_member_list[i].pwd_count = 0;
                    family_member_list[i].card_count = 0;  // 确保卡片计数重置
                    family_member_list[i].face_count = 0;  // 确保人脸计数重置
                        //  清空卡片信息
                    memset(&g_family_card_info[i], 0, sizeof(card_enroll_info_t));
                    //  清空指纹信息
                    memset(&g_family_finger_info[i], 0, sizeof(finger_enroll_info_t));
                    //  清空密码信息
                    memset(&g_family_pwd_info[i], 0, sizeof(pwd_enroll_info_t));
                    //  清空人脸信息
                    memset(&g_family_face_info[i], 0, sizeof(face_enroll_info_t));
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
            
            // 核心修复3：重置选中状态标记
            g_member_selected[i] = false;
            
            has_delete = true;
            LV_LOG_USER("成功删除成员：%d", i);
        }
    }
    
    // 2. 刷新界面（彻底重置所有样式+重建卡片）
    if(has_delete && family_menber_scr != NULL && lv_obj_is_valid(family_menber_scr)) {
        // 步骤1：清空所有残留的样式/状态（关键！）
        for(uint8_t i = 0; i < MAX_FAMILY_MEMBER_COUNT; i++) {
            // 即使是未选中的成员，也重置样式（防止批量删除时的样式污染）
            if(g_member_cards[i] != NULL && lv_obj_is_valid(g_member_cards[i])) {
                lv_obj_set_style_opa(g_member_cards[i], LV_OPA_100, LV_STATE_DEFAULT);
                lv_obj_add_flag(g_member_cards[i], LV_OBJ_FLAG_CLICKABLE);
            }
        }
        
        // 步骤2：删除所有旧卡片，重建剩余成员
        for(uint8_t i = 0; i < MAX_FAMILY_MEMBER_COUNT; i++) {
            if(g_member_cards[i] != NULL && lv_obj_is_valid(g_member_cards[i])) {
                lv_obj_del(g_member_cards[i]);
                g_member_cards[i] = NULL;
            }
        }
        
        // 步骤3：重置计数，重新创建剩余成员卡片
        member_count = 0;
        next_member_y = 374;
        restore_family_members(family_menber_scr);
        
        // 步骤4：强制刷新屏幕（解决LVGL样式缓存问题）
        lv_obj_invalidate(family_menber_scr);
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
                family_menber_scr, "cancel", &lv_font_montserrat_24, 
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
                family_menber_scr, "delete", &lv_font_montserrat_24, 
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
        for(uint8_t i = 0; i < MAX_FAMILY_MEMBER_COUNT; i++) {
            if(g_member_cards[i] != NULL && lv_obj_is_valid(g_member_cards[i])) {
                lv_obj_clear_flag(g_member_cards[i], LV_OBJ_FLAG_CLICKABLE); // 禁用点击
                // 初始透明度保持100%，只有勾选后才变灰
                lv_obj_set_style_opa(g_member_cards[i], LV_OPA_100, LV_STATE_DEFAULT);
            }
        }
        
        // 5. 禁用添加成员按钮
        if(family_menber_add_con != NULL && lv_obj_is_valid(family_menber_add_con)) {
            lv_obj_clear_flag(family_menber_add_con, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(family_menber_add_con, LV_OPA_50, LV_STATE_DEFAULT);
        }
        if(family_menber_add != NULL && lv_obj_is_valid(family_menber_add)) {
            lv_obj_clear_flag(family_menber_add, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(family_menber_add, LV_OPA_50, LV_STATE_DEFAULT);
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
        for(uint8_t i = 0; i < MAX_FAMILY_MEMBER_COUNT; i++) {
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
 * @return uint8_t 成员索引（成功）或MAX_FAMILY_MEMBER_COUNT（失败）
 */
static uint8_t save_family_member_info(const char *name, lv_color_t color)
{
    if(name == NULL) return MAX_FAMILY_MEMBER_COUNT;
    
    // 找到第一个空位置
    for(uint8_t i = 0; i < MAX_FAMILY_MEMBER_COUNT; i++) {
        if(!family_member_list[i].is_valid) {
            strncpy(family_member_list[i].name, name, sizeof(family_member_list[i].name)-1);
            family_member_list[i].avatar_color = color;
            family_member_list[i].is_valid = true;
            return i; // 返回成员索引
        }
    }
    return MAX_FAMILY_MEMBER_COUNT; // 无空位置
}

/**
 * @brief 从数组恢复所有有效成员卡片
 * @param parent 父容器对象指针
 */
static void restore_family_members(lv_obj_t *parent)
{
    if(parent == NULL) return;
    
    next_member_y = 374; // 重置Y轴起始位置
    member_count = 0;    // 重置计数
    
    // 遍历成员数组，恢复所有有效成员
    for(uint8_t i = 0; i < MAX_FAMILY_MEMBER_COUNT; i++) {
        if(family_member_list[i].is_valid) {
            create_family_member_card(parent, next_member_y, 
                                     family_member_list[i].name, 
                                     family_member_list[i].avatar_color, i);
            next_member_y += 179;
            member_count++;
        }
    }
    
    // 更新添加按钮状态
    update_add_member_btn_state();
}

/*****************************************************成员信息创建截至***************************************** */

/**
 * @brief 家庭成员卡片点击回调函数
 * 
 * 处理点击事件，根据是否处于删除模式来执行不同操作：
 * - 非删除模式：选中成员并调用录入回调
 * - 删除模式：切换成员选中状态
 * 
 * @param e 事件对象指针，包含点击事件信息
 */
static void member_card_click_cb(lv_event_t *e)
{
    if(e == NULL || g_is_delete_mode) return;
    
    // 1. 获取当前选中的家庭成员索引
    uint8_t idx = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    g_selected_member_idx = idx;
    
    // 2. 构造通用成员信息（修复name数组赋值问题）
    common_member_info_t member_info = {
        .type = MEMBER_TYPE_FAMILY,
        .idx = idx,
        .avatar_color = family_member_list[idx].avatar_color,
        .finger_count = family_member_list[idx].finger_count,
        .pwd_count = family_member_list[idx].pwd_count,
        .card_count = family_member_list[idx].card_count,
        .face_count = family_member_list[idx].face_count
    };
    // 字符串数组不能直接赋值，用strcpy拷贝
    strncpy(member_info.name, family_member_list[idx].name, sizeof(member_info.name)-1);
    member_info.name[sizeof(member_info.name)-1] = '\0'; // 确保字符串结束
    

    // 先定义全局/静态变量存储member_info（或用lv_obj_set_user_data）
    static common_member_info_t g_temp_member_info; // 静态变量临时存储
    g_temp_member_info = member_info;
    // 获取父屏幕指针（事件目标就是卡片，父屏幕是family_member_scr）
    lv_obj_t *family_member_scr = lv_obj_get_parent(lv_event_get_target(e));
    // 直接调用录入创建函数（不再通过enroll_btn_click_cb中转）
    ui_enroll_create(&g_temp_member_info, family_member_scr);
}

/**
 * @brief 创建家庭成员卡片
 * 
 * 创建一个家庭成员的卡片，包含头像、名称、指纹/密码/卡片/人脸相关信息。
 * 
 * @param parent 父容器对象指针
 * @param y_pos 卡片在父容器中的Y轴位置
 * @param member_name 成员名称字符串指针
 * @param avatar_color 头像颜色
 * @param member_idx 成员索引（0-2）
 * @return lv_obj_t* 成员卡片容器对象指针，失败返回NULL
 */
static lv_obj_t *create_family_member_card(lv_obj_t *parent, lv_coord_t y_pos, const char *member_name, lv_color_t avatar_color, uint8_t member_idx)
{
    if(parent == NULL || member_name == NULL || member_idx >= MAX_FAMILY_MEMBER_COUNT) return NULL;
    
    // 1. 创建成员卡片容器
    lv_obj_t *member_con = create_container
    (parent, 47, y_pos, 710, 175, lv_color_hex(0x192A46), LV_OPA_100, 6, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_user_data(member_con, (void*)(uintptr_t)member_idx);
    lv_obj_set_style_pad_all(member_con, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(member_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(member_con, LV_OPA_80, LV_STATE_PRESSED);

    lv_obj_add_event_cb(member_con, member_card_click_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)member_idx);
    //保存成员卡片到全局数组
    g_member_cards[member_idx] = member_con;
    // 4. 头像容器
    lv_obj_t *avatar_con = create_container
    (member_con, 23, 26, 99, 99, avatar_color, LV_OPA_100, 100, avatar_color, 0, LV_OPA_90);

    // 5. 成员名称标签
    lv_obj_t *name_label = create_text_label
    (member_con, member_name, &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 151, 40, LV_OPA_100);

    // ========== 核心修改：动态读取计数并更新UI ==========
    // 读取当前成员的计数数据
    uint8_t finger_count = family_member_list[member_idx].finger_count;
    uint8_t pwd_count = family_member_list[member_idx].pwd_count;
    uint8_t card_count = family_member_list[member_idx].card_count;
    uint8_t face_count = family_member_list[member_idx].face_count;

    // 6. 指纹相关（动态显示计数和图标）
    char finger_text[8] = {0};
    snprintf(finger_text, sizeof(finger_text), "%d/2", finger_count);
    // 选择图标：有记录显示已录入，无记录显示未录入
    const char *finger_img_path = (finger_count > 0) ? "H:finger_has_record.png" : "H:finger_no_record.png";
    g_finger_imgs[member_idx] = create_image_obj(member_con, finger_img_path, 151, 105);
    g_finger_labels[member_idx] = create_text_label
    (member_con, finger_text, &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 189, 105, LV_OPA_100);

    // 7. 密码相关（动态显示计数和图标）
    char pwd_text[8] = {0};
    snprintf(pwd_text, sizeof(pwd_text), "%d/1", pwd_count);
    const char *pwd_img_path = (pwd_count > 0) ? "H:pwd_has_record.png" : "H:pwd_no_record.png";
    g_pwd_imgs[member_idx] = create_image_obj(member_con, pwd_img_path, 259, 105);
    g_pwd_labels[member_idx] = create_text_label
    (member_con, pwd_text, &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 297, 105, LV_OPA_100);

    // 8. 卡片相关（动态显示计数和图标）
    char card_text[8] = {0};
    snprintf(card_text, sizeof(card_text), "%d/1", card_count);
    const char *card_img_path = (card_count > 0) ? "H:card_has_record.png" : "H:card_no_record.png";
    g_card_imgs[member_idx] = create_image_obj(member_con, card_img_path, 361, 93);
    g_card_labels[member_idx] = create_text_label
    (member_con, card_text, &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 414, 105, LV_OPA_100);

    // 9. 人脸相关（动态显示计数和图标）
    char face_text[8] = {0};
    snprintf(face_text, sizeof(face_text), "%d/1", face_count);
    const char *face_img_path = (face_count > 0) ? "H:face_has_record.png" : "H:face_no_record.png";
    g_face_imgs[member_idx] = create_image_obj(member_con, face_img_path, 472, 98);
    g_face_labels[member_idx] = create_text_label
    (member_con, face_text, &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 516, 105, LV_OPA_100);

    // 7. 预埋删除按钮（原有逻辑不变）
    lv_obj_t *delete_hid = create_container
    (member_con, 655, 59, 36, 36, lv_color_hex(0x192A46), LV_OPA_100, 100, lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_add_flag(delete_hid, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(delete_hid, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(delete_hid, LV_OPA_80, LV_STATE_PRESSED);
    // 添加删除按钮点击回调
    lv_obj_add_event_cb(delete_hid, member_delete_btn_click_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)member_idx);
    g_delete_hid_containers[member_idx] = delete_hid;
    g_delete_flag_imgs[member_idx] = NULL;
    g_member_selected[member_idx] = false;
    return member_con;
}


//实时更新成员卡片上的计数和图标
void update_member_count_ui(uint8_t member_idx)
{
    if(member_idx >= MAX_FAMILY_MEMBER_COUNT) return;

    // 1. 安全校验：标签对象是否有效
    if(g_finger_labels[member_idx] == NULL || !lv_obj_is_valid(g_finger_labels[member_idx]) ||
       g_pwd_labels[member_idx] == NULL || !lv_obj_is_valid(g_pwd_labels[member_idx]) ||
       g_card_labels[member_idx] == NULL || !lv_obj_is_valid(g_card_labels[member_idx]) ||
       g_face_labels[member_idx] == NULL || !lv_obj_is_valid(g_face_labels[member_idx])) {
        LV_LOG_WARN("Count labels for member %d are invalid, skip update", member_idx);
        return;
    }

    // 2. 读取最新计数
    uint8_t finger_count = family_member_list[member_idx].finger_count;
    uint8_t pwd_count = family_member_list[member_idx].pwd_count;
    uint8_t card_count = family_member_list[member_idx].card_count;
    uint8_t face_count = family_member_list[member_idx].face_count;

    // 3. 更新指纹计数文本
    char finger_text[8] = {0};
    snprintf(finger_text, sizeof(finger_text), "%d/2", finger_count);
    lv_label_set_text(g_finger_labels[member_idx], finger_text);
    // 更新指纹图标（可选）
    if(g_finger_imgs[member_idx] != NULL && lv_obj_is_valid(g_finger_imgs[member_idx])) {
        const char *finger_img_path = (finger_count > 0) ? "H:finger_has_record.png" : "H:finger_no_record.png";
        lv_img_set_src(g_finger_imgs[member_idx], finger_img_path);
    }

    // 4. 更新密码计数文本
    char pwd_text[8] = {0};
    snprintf(pwd_text, sizeof(pwd_text), "%d/1", pwd_count);
    lv_label_set_text(g_pwd_labels[member_idx], pwd_text);
    if(g_pwd_imgs[member_idx] != NULL && lv_obj_is_valid(g_pwd_imgs[member_idx])) {
        const char *pwd_img_path = (pwd_count > 0) ? "H:pwd_has_record.png" : "H:pwd_no_record.png";
        lv_img_set_src(g_pwd_imgs[member_idx], pwd_img_path);
    }

    // 5. 更新卡片计数文本
    char card_text[8] = {0};
    snprintf(card_text, sizeof(card_text), "%d/1", card_count);
    lv_label_set_text(g_card_labels[member_idx], card_text);
    if(g_card_imgs[member_idx] != NULL && lv_obj_is_valid(g_card_imgs[member_idx])) {
        const char *card_img_path = (card_count > 0) ? "H:card_has_record.png" : "H:card_no_record.png";
        lv_img_set_src(g_card_imgs[member_idx], card_img_path);
    }

    // 6. 更新人脸计数文本
    char face_text[8] = {0};
    snprintf(face_text, sizeof(face_text), "%d/1", face_count);
    lv_label_set_text(g_face_labels[member_idx], face_text);
    if(g_face_imgs[member_idx] != NULL && lv_obj_is_valid(g_face_imgs[member_idx])) {
        const char *face_img_path = (face_count > 0) ? "H:face_has_record.png" : "H:face_no_record.png";
        lv_img_set_src(g_face_imgs[member_idx], face_img_path);
    }

    // 7. 强制刷新UI
    lv_obj_invalidate(lv_scr_act());
    LV_LOG_INFO("Updated count UI for member %d: finger=%d, pwd=%d, card=%d, face=%d",
                member_idx, finger_count, pwd_count, card_count, face_count);
}

//实时更新

/**
 * @brief 头像点击回调函数
 * 
 * 处理头像点击事件，根据当前成员数量判断是否允许  成员。
 * 如果成员数量未达上限，会弹出输入框请求成员名称，
 * 并根据用户输入创建新的家庭成员卡片。
 * 
 * @param e 事件对象指针，包含点击事件信息
 */
static void avatar_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *avatar = lv_event_get_target(e);
    uint8_t avatar_idx = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    
    if(member_count >= MAX_FAMILY_MEMBER_COUNT) {
            LV_LOG_USER("成员数量已达上限（%d个),无法  ", MAX_FAMILY_MEMBER_COUNT);
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
    uint8_t member_idx = save_family_member_info(member_name, selected_avatar_color);
    if(member_idx == MAX_FAMILY_MEMBER_COUNT) {
        LV_LOG_WARN("No empty slot for new member");
        close_custom_popup();
        return;
    }
    
    // 4. 创建新成员卡片
    if(family_menber_scr != NULL && lv_obj_is_valid(family_menber_scr)) {
        create_family_member_card(family_menber_scr, next_member_y, member_name, selected_avatar_color, member_idx);
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
 * @brief 家庭成员添加按钮点击回调函数
 * 
 * 处理添加家庭成员按钮点击事件，根据当前成员数量判断是否允许  成员。
 * 如果成员数量未达上限，会弹出输入框请求成员名称，
 * 并根据用户输入创建新的家庭成员卡片。
 * 
 * @param e 事件对象指针，包含点击事件信息
 */
void family_menber_add_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *user_manage_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(user_manage_scr == NULL || family_menber_scr == NULL) {
        LV_LOG_WARN("family_menber_add_click_cb: screen obj is NULL!");
        return;
    }

    // 1. 先关闭旧弹窗
    close_custom_popup();

    // 2. 创建/显示背景遮罩层
    create_bg_mask_layer(family_menber_scr);

    // 3. 创建自定义弹窗主体
    custom_popup = create_container
    (family_menber_scr, 100, 455, 600, 423, lv_color_hex(0xE0EDFF), LV_OPA_100, 16, lv_color_hex(0x1F3150), 0, LV_OPA_90);
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
    lv_coord_t card_height = 175;   // 单个成员卡片的高度（和create_family_member_card中一致）
    
    // 遍历所有有效成员，找到最底部成员的Y坐标 + 卡片高度
    for(uint8_t i = 0; i < MAX_FAMILY_MEMBER_COUNT; i++) {
        if(family_member_list[i].is_valid && g_member_cards[i] != NULL && lv_obj_is_valid(g_member_cards[i])) {
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
    if(family_menber_scr == NULL || !lv_obj_is_valid(family_menber_scr)) return;
    
    for(uint8_t idx = 0; idx < MAX_FAMILY_MEMBER_COUNT; idx++) {
        if(!family_member_list[idx].is_valid) continue;
        
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
        for(uint8_t i = 0; i < MAX_FAMILY_MEMBER_COUNT; i++) {
            g_member_selected[i] = false;
        }
    }
}

// 检查并更新添加按钮状态
static void update_add_member_btn_state(void)
{
    
    if(family_menber_add_con != NULL && lv_obj_is_valid(family_menber_add_con)) {
        if(member_count >= MAX_FAMILY_MEMBER_COUNT) {
            lv_obj_clear_flag(family_menber_add_con, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(family_menber_add_con, LV_OPA_50, LV_STATE_DEFAULT);
        } else {
            lv_obj_add_flag(family_menber_add_con, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(family_menber_add_con, LV_OPA_100, LV_STATE_DEFAULT);
        }
    }
    if(family_menber_add != NULL && lv_obj_is_valid(family_menber_add_con)) {
        if(member_count >= MAX_FAMILY_MEMBER_COUNT) {
            lv_obj_clear_flag(family_menber_add, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(family_menber_add, LV_OPA_50, LV_STATE_DEFAULT);
        } else {
            lv_obj_add_flag(family_menber_add, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(family_menber_add, LV_OPA_100, LV_STATE_DEFAULT);
        }
    }
}

#else
#include "lv_family_menber.h"
#include "lv_a_enroll_opt.h"
#include "stdio.h"


lv_obj_t *g_finger_labels[MAX_FAMILY_MEMBER_COUNT] = {NULL};
lv_obj_t *g_pwd_labels[MAX_FAMILY_MEMBER_COUNT] = {NULL};
lv_obj_t *g_card_labels[MAX_FAMILY_MEMBER_COUNT] = {NULL};
lv_obj_t *g_face_labels[MAX_FAMILY_MEMBER_COUNT] = {NULL};

lv_obj_t *g_finger_imgs[MAX_FAMILY_MEMBER_COUNT] = {NULL};
lv_obj_t *g_pwd_imgs[MAX_FAMILY_MEMBER_COUNT] = {NULL};
lv_obj_t *g_card_imgs[MAX_FAMILY_MEMBER_COUNT] = {NULL};
lv_obj_t *g_face_imgs[MAX_FAMILY_MEMBER_COUNT] = {NULL};

// ====================== 数据结构定义 ======================
typedef struct {
    uint8_t member_idx;          // 成员唯一索引（0~7）
    lv_obj_t *parent_scr;        // 父屏幕对象
    family_member_info_t *info;  // 指向该成员的信息结构体
} enroll_cb_data_t;

// ====================== 全局变量 ======================
// 成员信息核心变量
uint8_t g_selected_member_idx = MAX_FAMILY_MEMBER_COUNT;
family_member_info_t family_member_list[MAX_FAMILY_MEMBER_COUNT] = {0};

// 删除模式专用变量
static bool g_is_delete_mode = false;
static lv_obj_t *g_delete_cancel_btn = NULL;
static lv_obj_t *g_delete_confirm_btn = NULL;
//static uint8_t g_selected_delete_idx = MAX_FAMILY_MEMBER_COUNT;
//static lv_obj_t *g_selected_delete_btn = NULL;
static lv_obj_t *g_delete_flag_imgs[MAX_FAMILY_MEMBER_COUNT] = {NULL};
static lv_obj_t *g_delete_hid_containers[MAX_FAMILY_MEMBER_COUNT] = {NULL};
static bool g_member_selected[MAX_FAMILY_MEMBER_COUNT] = {false};
static lv_obj_t *g_member_cards[MAX_FAMILY_MEMBER_COUNT] = {NULL};
lv_obj_t *delete_img = NULL;
lv_obj_t *back_btn = NULL;
lv_obj_t *family_menber_label = NULL;
// 全局变量
static lv_obj_t *bg_mask_layer = NULL;  
static lv_obj_t *custom_popup = NULL;
lv_obj_t *family_menber_scr = NULL; 
static lv_style_t family_menber_grad_style;
static bool family_menber_style_inited = false;
static lv_obj_t *name_keyboard = NULL;
static lv_obj_t *g_name_input = NULL;
static lv_color_t selected_avatar_color = {0};
//static lv_coord_t next_member_y = 374;
static uint8_t member_count = 0;
lv_obj_t *family_menber_add_con = NULL;
lv_obj_t *family_menber_add = NULL;

// 常量定义
static const lv_coord_t avatar_x_offset[] = {19, 110, 201};
static const lv_coord_t avatar_y_offset[] = {12, 101};

// ====================== 函数声明 ======================
// 内部函数前置声明
static void close_custom_popup(void);

// UI构建相关
static lv_obj_t *create_family_member_card(lv_obj_t *parent, const char *member_name, lv_color_t avatar_color, uint8_t member_idx);
static void update_add_member_btn_state(void);
void ui_family_menber_create(lv_obj_t *user_manage_scr);
static void restore_family_members(lv_obj_t *parent);

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
void family_menber_back_btn_click_cb(lv_event_t *e);
// 遮罩层相关
static void create_bg_mask_layer(lv_obj_t *target_container);

// 事件回调
void family_menber_add_click_cb(lv_event_t *e);


// 全局样式初始化
static void init_family_menber_styles(void)
{
    if(!family_menber_style_inited) {
        lv_style_init(&family_menber_grad_style);
        family_menber_style_inited = true;
    }
}

static void family_menber_scr_load_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *scr = lv_event_get_target(e);
    if(!is_lv_obj_valid(scr)) return;
    
    update_status_bar_parent(scr);
}




void ui_family_menber_create(lv_obj_t *user_manage_scr)
{
    if(selected_avatar_color.ch.red == 0 && selected_avatar_color.ch.green == 0 && selected_avatar_color.ch.blue == 0) {
        selected_avatar_color = lv_color_hex(0xEDF4FF);
    }
    init_family_menber_styles();
    
    if(user_manage_scr == NULL) {
        LV_LOG_WARN("ui_family_menber_create: user_manage_scr is NULL!");
        return;
    }

    // 安全创建/重置家庭成员屏幕（修复资源无法释放，保留原有状态重置）
    if(is_lv_obj_valid(family_menber_scr)) {
        lv_obj_del(family_menber_scr);  // 销毁整个旧屏幕，释放所有子控件/资源
        family_menber_scr = NULL;      // 指针置空，杜绝野指针
    }
    // 重新创建全新的屏幕对象
    family_menber_scr = lv_obj_create(NULL);
    // 绑定屏幕加载事件（每次重建必须重新绑定，旧对象已销毁）
    lv_obj_add_event_cb(family_menber_scr, family_menber_scr_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);

    // ✅ 完全保留你原有逻辑：重置临时状态（和旧代码功能一致）
    bg_mask_layer = NULL;
    name_keyboard = NULL;
    selected_avatar_color = lv_color_hex(0xEDF4FF);

    lv_style_reset(&family_menber_grad_style);
    lv_style_set_bg_color(&family_menber_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&family_menber_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&family_menber_grad_style, LV_GRAD_DIR_VER);
    lv_style_set_bg_main_stop(&family_menber_grad_style, 0);
    lv_style_set_bg_grad_stop(&family_menber_grad_style, 255);
    lv_obj_add_style(family_menber_scr, &family_menber_grad_style, LV_STATE_DEFAULT);

    family_menber_label = create_text_label(family_menber_scr, "family menber", &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 83, 80, LV_OPA_100);
    //lv_obj_align(family_menber_label, LV_ALIGN_TOP_MID, 0, 115);
    // 添加家庭成员容器
    family_menber_add_con = create_container
    (family_menber_scr,47,150,300,200,lv_color_hex(0x192A46), LV_OPA_100, 6,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_add_flag(family_menber_add_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(family_menber_add_con, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(family_menber_add_con, family_menber_add_click_cb, LV_EVENT_CLICKED, user_manage_scr);

    //添加加号
    family_menber_add = create_container
    (family_menber_scr,161,213,74,74,lv_color_hex(0x2F476F), LV_OPA_100, 100,lv_color_hex(0x1F3150), 0, LV_OPA_90);
    //加号线
    lv_obj_t *divider_line1 = lv_line_create(family_menber_scr);
    static const lv_point_t divider_points1[] = {{173, 251}, {223, 251}}; 
    config_divider_line_style(divider_line1, divider_points1, 2, 0x617C9D, 8, LV_OPA_100);
    lv_obj_t *divider_line2 = lv_line_create(family_menber_scr);
    static const lv_point_t divider_points2[] = {{198, 225}, {198, 275}}; 
    config_divider_line_style(divider_line2, divider_points2, 2, 0x617C9D, 8, LV_OPA_100);

    lv_obj_add_flag(family_menber_add, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(family_menber_add, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(family_menber_add, family_menber_add_click_cb, LV_EVENT_CLICKED, user_manage_scr);

    //右上角删除设置
    //delete_img = create_image_obj(family_menber_scr, "H:....png", 928, 99);
    delete_img = create_container_circle(family_menber_scr, 928, 99, 30,
    true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_add_flag(delete_img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(delete_img, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(delete_img, delete_img_click_cb, LV_EVENT_CLICKED, NULL);

    // 左上角返回按钮
    // back_btn = create_image_obj(family_menber_scr, "H:back.png", 52, 123);
    // lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    // lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);
    // lv_obj_add_event_cb(back_btn, back_btn_click_cb, LV_EVENT_CLICKED, user_manage_scr);
    back_btn = create_container_circle(family_menber_scr, 52, 90, 30,
    true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(back_btn,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn,LV_OPA_80,LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn,family_menber_back_btn_click_cb,LV_EVENT_CLICKED,user_manage_scr);

    // ====================== 恢复成员卡片 ======================
    restore_family_members(family_menber_scr);

    // 切换到设置屏幕
    lv_scr_load(family_menber_scr);
}

extern void ui_user_manage_create(lv_obj_t *homepage_scr);
extern void destroy_user_manage(void);
// 家庭成员界面回调
void family_menber_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(parent_scr == NULL) return;

    // 1. 创建家庭成员界面
    ui_family_menber_create(parent_scr);
    // 2. 加载家庭成员
    lv_scr_load(family_menber_scr);
    update_status_bar_parent(family_menber_scr);
    // 3. 销毁用户管理（全局！已切屏，安全）
    destroy_user_manage();
    LV_LOG_WARN("进入家庭成员，销毁用户管理");
}

// 家庭成员 → 返回用户管理（重建用户管理）
// 完全参考你的写法！格式/逻辑/结构 一模一样
void family_menber_back_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);
    
    // 获取当前正在显示的页面（要删除的目标）
    lv_obj_t *current_del_scr = lv_disp_get_scr_act(NULL);
    if(!lv_obj_is_valid(current_del_scr)) return;

    // ===================== 分支1：当前是【家庭成员界面】（用户管理已被删除，必须重建） =====================
    if(current_del_scr == family_menber_scr) {
        // 1. 【重建用户管理】（因为之前进家庭成员时把用户管理删了）
        ui_user_manage_create(parent_scr);  
        // 2. 【销毁家庭成员】（此时已经切到用户管理，删旧页100%安全）
        lv_obj_del(current_del_scr);
        family_menber_scr = NULL;
        LV_LOG_WARN("Family member response: Rebuild the user manage and destroy the family member interface");
        return;  // 结束，不走其他逻辑
    }
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
 * @brief 紧凑化家庭成员数据（删除后自动前移补位，消除空位）
 */
static void compact_family_members(void)
{
    uint8_t valid_idx = 0; // 有效成员的目标位置
    
    // 遍历所有成员，将有效成员向前紧凑排列
    for (uint8_t i = 0; i < MAX_FAMILY_MEMBER_COUNT; i++)
    {
        if (family_member_list[i].is_valid)
        {
            // 如果当前有效成员不在目标位置，才移动数据
            if (valid_idx != i)
            {
                // 1. 移动主成员数据
                family_member_list[valid_idx] = family_member_list[i];
                // 2. 移动对应指纹/密码/卡片/人脸数据
                g_family_finger_info[valid_idx] = g_family_finger_info[i];
                g_family_pwd_info[valid_idx] = g_family_pwd_info[i];
                g_family_card_info[valid_idx] = g_family_card_info[i];
                g_family_face_info[valid_idx] = g_family_face_info[i];
                
                // 3. 清空原位置数据（避免残留）
                memset(&family_member_list[i], 0, sizeof(family_member_info_t));
                memset(&g_family_finger_info[i], 0, sizeof(finger_enroll_info_t));
                memset(&g_family_pwd_info[i], 0, sizeof(pwd_enroll_info_t));
                memset(&g_family_card_info[i], 0, sizeof(card_enroll_info_t));
                memset(&g_family_face_info[i], 0, sizeof(face_enroll_info_t));
            }
            valid_idx++; // 目标位置后移
        }
    }
    
    // 重置总成员数量
    member_count = valid_idx;
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
    for(uint8_t i = 0; i < MAX_FAMILY_MEMBER_COUNT; i++) {
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
    create_bg_mask_layer(family_menber_scr);
    
    // 2. 创建确认弹窗
    lv_obj_t *confirm_popup = create_container(
        family_menber_scr, 212, 151, 600, 297, 
        lv_color_hex(0xE0EDFF), LV_OPA_100, 
        16, lv_color_hex(0x1F3150), 0, LV_OPA_90
    );
    
    // 3. 提示文本
    create_text_label(confirm_popup, "confirm_delete?", &lv_font_montserrat_32, lv_color_hex(0x000000), 156, 52, LV_OPA_100);
    
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
    if(idx >= MAX_FAMILY_MEMBER_COUNT || !g_member_selected[idx]) {
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
    
    // 4. 直接从全局数组获取delete_hid并显示（核心修复）
    if(g_delete_hid_containers[idx] != NULL && lv_obj_is_valid(g_delete_hid_containers[idx])) {
        // 强制显示delete_hid
        lv_obj_clear_flag(g_delete_hid_containers[idx], LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_opa(g_delete_hid_containers[idx], LV_OPA_100, LV_STATE_DEFAULT);
        //lv_obj_set_visible(g_delete_hid_containers[idx], true);
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
    if(idx >= MAX_FAMILY_MEMBER_COUNT || g_member_selected[idx]) {
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
    g_delete_flag_imgs[idx] = create_image_obj(member_con, "H:delete_flag.png", 250, 10);
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
    
    // 1. 🔥 删除前：先清空选中成员的所有生物特征数据（指纹/密码/卡片/人脸）
    bool has_delete = false;
    for(uint8_t i = 0; i < MAX_FAMILY_MEMBER_COUNT; i++) {
        if(g_member_selected[i]) {
            // 🔥 调用函数：一键清空该成员所有数据
            clear_member_all_biometrics(i, MEMBER_TYPE_FAMILY);
            // 标记成员为无效
            memset(&family_member_list[i], 0, sizeof(family_member_info_t));
            has_delete = true;
        }
    }
    
    // 2. 数据紧凑化
    if(has_delete) {
        compact_family_members();
    }
    
    // 3. 清空所有旧UI + 重置全局状态
    for(uint8_t i = 0; i < MAX_FAMILY_MEMBER_COUNT; i++) {
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
    
    // 4. 重新创建UI
    if(family_menber_scr != NULL && lv_obj_is_valid(family_menber_scr)) {
        restore_family_members(family_menber_scr);
        lv_obj_invalidate(family_menber_scr);
        lv_refr_now(NULL);
        update_add_member_btn_state();
    }
    
    // 5. 关闭弹窗 + 退出删除模式
    if(popup != NULL && lv_obj_is_valid(popup)) lv_obj_del(popup);
    if(bg_mask_layer != NULL && lv_obj_is_valid(bg_mask_layer)) {
        lv_obj_add_flag(bg_mask_layer, LV_OBJ_FLAG_HIDDEN);
    }
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
        if(family_menber_label != NULL && lv_obj_is_valid(family_menber_label)) {
            lv_obj_add_flag(family_menber_label, LV_OBJ_FLAG_HIDDEN);
        }
        
        // 2. 创建取消按钮
        if(g_delete_cancel_btn == NULL || !lv_obj_is_valid(g_delete_cancel_btn)) {
            g_delete_cancel_btn = create_text_label(
                family_menber_scr, "cancel", &lv_font_montserrat_24, 
                lv_color_hex(0x00BDBD), 48, 90, LV_OPA_100
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
                family_menber_scr, "delete", &lv_font_montserrat_24, 
                lv_color_hex(0xFF0000), 928, 90, LV_OPA_100
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
        for(uint8_t i = 0; i < MAX_FAMILY_MEMBER_COUNT; i++) {
            if(g_member_cards[i] != NULL && lv_obj_is_valid(g_member_cards[i])) {
                lv_obj_clear_flag(g_member_cards[i], LV_OBJ_FLAG_CLICKABLE); // 禁用点击
                // 初始透明度保持100%，只有勾选后才变灰
                lv_obj_set_style_opa(g_member_cards[i], LV_OPA_100, LV_STATE_DEFAULT);
            }
        }
        
        // 5. 禁用添加成员按钮
        if(family_menber_add_con != NULL && lv_obj_is_valid(family_menber_add_con)) {
            lv_obj_clear_flag(family_menber_add_con, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(family_menber_add_con, LV_OPA_50, LV_STATE_DEFAULT);
        }
        if(family_menber_add != NULL && lv_obj_is_valid(family_menber_add)) {
            lv_obj_clear_flag(family_menber_add, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(family_menber_add, LV_OPA_50, LV_STATE_DEFAULT);
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
        if(family_menber_label != NULL && lv_obj_is_valid(family_menber_label)) {
            lv_obj_clear_flag(family_menber_label, LV_OBJ_FLAG_HIDDEN);
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
        for(uint8_t i = 0; i < MAX_FAMILY_MEMBER_COUNT; i++) {
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
 * @return uint8_t 成员索引（成功）或MAX_FAMILY_MEMBER_COUNT（失败）
 */
static uint8_t save_family_member_info(const char *name, lv_color_t color)
{
    if(name == NULL) return MAX_FAMILY_MEMBER_COUNT;
    
    // 找到第一个空位置
    for(uint8_t i = 0; i < MAX_FAMILY_MEMBER_COUNT; i++) {
        if(!family_member_list[i].is_valid) {
            strncpy(family_member_list[i].name, name, sizeof(family_member_list[i].name)-1);
            family_member_list[i].avatar_color = color;
            family_member_list[i].is_valid = true;
            return i; // 返回成员索引
        }
    }
    return MAX_FAMILY_MEMBER_COUNT; // 无空位置
}

/**
 * @brief 从数组恢复所有有效成员卡片
 * @param parent 父容器对象指针
 */
/**
 * @brief 从数组恢复所有有效成员卡片（网格版）
 */
static void restore_family_members(lv_obj_t *parent)
{
    if(parent == NULL) return;

    member_count = 0;    // 重置计数

    // 遍历成员数组，恢复所有有效成员（自动网格排列）
    for(uint8_t i = 0; i < MAX_FAMILY_MEMBER_COUNT; i++) {
        if(family_member_list[i].is_valid) {
            // 🔥 关键：去掉y_pos，直接传索引自动计算网格位置
            create_family_member_card(parent,
                                     family_member_list[i].name,
                                     family_member_list[i].avatar_color, i);
            member_count++;
        }
    }

    // 更新添加按钮状态（不变）
    update_add_member_btn_state();
}
// static void restore_family_members(lv_obj_t *parent)
// {
//     if(parent == NULL) return;
    
//     next_member_y = 374; // 重置Y轴起始位置
//     member_count = 0;    // 重置计数
    
//     // 遍历成员数组，恢复所有有效成员
//     for(uint8_t i = 0; i < MAX_FAMILY_MEMBER_COUNT; i++) {
//         if(family_member_list[i].is_valid) {
//             create_family_member_card(parent, next_member_y, 
//                                      family_member_list[i].name, 
//                                      family_member_list[i].avatar_color, i);
//             next_member_y += 179;
//             member_count++;
//         }
//     }
    
//     // 更新添加按钮状态
//     update_add_member_btn_state();
// }

/*****************************************************成员信息创建截至***************************************** */

/**
 * @brief 家庭成员卡片点击回调函数
 * 
 * 处理点击事件，根据是否处于删除模式来执行不同操作：
 * - 非删除模式：选中成员并调用录入回调
 * - 删除模式：切换成员选中状态
 * 
 * @param e 事件对象指针，包含点击事件信息
 */
static void member_card_click_cb(lv_event_t *e)
{
    if(e == NULL || g_is_delete_mode) return;
    
    // 1. 获取当前选中的家庭成员索引
    uint8_t idx = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    g_selected_member_idx = idx;
    
    // 2. 构造通用成员信息（修复name数组赋值问题）
    common_member_info_t member_info = {
        .type = MEMBER_TYPE_FAMILY,
        .idx = idx,
        .avatar_color = family_member_list[idx].avatar_color,
        .finger_count = family_member_list[idx].finger_count,
        .pwd_count = family_member_list[idx].pwd_count,
        .card_count = family_member_list[idx].card_count,
        .face_count = family_member_list[idx].face_count
    };
    // 字符串数组不能直接赋值，用strcpy拷贝
    strncpy(member_info.name, family_member_list[idx].name, sizeof(member_info.name)-1);
    member_info.name[sizeof(member_info.name)-1] = '\0'; // 确保字符串结束
    

    // 先定义全局/静态变量存储member_info（或用lv_obj_set_user_data）
    static common_member_info_t g_temp_member_info; // 静态变量临时存储
    g_temp_member_info = member_info;
    // 获取父屏幕指针（事件目标就是卡片，父屏幕是family_member_scr）
    lv_obj_t *family_member_scr = lv_obj_get_parent(lv_event_get_target(e));
    // 直接调用录入创建函数（不再通过enroll_btn_click_cb中转）
    ui_enroll_create(&g_temp_member_info, family_member_scr);
    destroy_family_member();
}

/**
 * @brief 创建家庭成员卡片
 * 
 * 创建一个家庭成员的卡片，包含头像、名称、指纹/密码/卡片/人脸相关信息。
 * 
 * @param parent 父容器对象指针
 * @param y_pos 卡片在父容器中的Y轴位置
 * @param member_name 成员名称字符串指针
 * @param avatar_color 头像颜色
 * @param member_idx 成员索引（0-2）
 * @return lv_obj_t* 成员卡片容器对象指针，失败返回NULL
 * @brief 创建家庭成员卡片（横向网格版，一行3个）
 */

static lv_obj_t *create_family_member_card(lv_obj_t *parent, const char *member_name, lv_color_t avatar_color, uint8_t member_idx)
{
    if(parent == NULL || member_name == NULL || member_idx >= MAX_FAMILY_MEMBER_COUNT) return NULL;

    // ====================== 核心：自动网格计算（100%避开添加按钮，自动换行）======================
    const lv_coord_t col_x[] = {48, 363, 677};        // 固定3列X坐标
    const lv_coord_t row_start_y = 150;                // 第一行Y坐标
    const lv_coord_t card_height = 200;               // 卡片高度
    const lv_coord_t row_gap = 26;                    // 行间距
    const lv_coord_t row_step = card_height + row_gap; // 每行总高度

    // 🔥 关键算法：跳过添加按钮(0行0列)，成员自动排列+自动换行
    uint16_t total_position = member_idx + 1;  // +1 跳过添加按钮的位置
    uint8_t current_row = total_position / 3;   // 自动计算行号
    uint8_t current_col = total_position % 3;   // 自动计算列号
    lv_coord_t card_x = col_x[current_col];    // 最终X坐标
    lv_coord_t card_y = row_start_y + current_row * row_step; // 最终Y坐标（自动换行）

    // 1. 创建成员卡片容器
    lv_obj_t *member_con = create_container
    (parent, card_x, card_y, 300, 200, lv_color_hex(0x192A46), LV_OPA_100, 6, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_user_data(member_con, (void*)(uintptr_t)member_idx);
    lv_obj_set_style_pad_all(member_con, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(member_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(member_con, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(member_con, member_card_click_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)member_idx);
    g_member_cards[member_idx] = member_con;

    // 2. 头像容器（和你的示例完全一致）
    create_container(member_con, 122, 15, 60, 60, avatar_color, LV_OPA_100, 100, avatar_color, 0, LV_OPA_90);

    // 3. 成员名称标签（水平居中）
    lv_obj_t *name_label = create_text_label
    (member_con, member_name, &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align(name_label, LV_ALIGN_TOP_MID, 0, 87);

    // 读取计数数据（不变）
    uint8_t finger_count = family_member_list[member_idx].finger_count;
    uint8_t pwd_count = family_member_list[member_idx].pwd_count;
    uint8_t card_count = family_member_list[member_idx].card_count;
    uint8_t face_count = family_member_list[member_idx].face_count;

    // 4. 指纹控件
    char finger_text[8] = {0};
    snprintf(finger_text, sizeof(finger_text), "%d", finger_count);
    const char *finger_img_path = (finger_count > 0) ? "H:finger_has_record.png" : "H:finger_no_record.png";
    g_finger_imgs[member_idx] = create_image_obj(member_con, finger_img_path, 18, 157);
    g_finger_labels[member_idx] = create_text_label
    (member_con, finger_text, &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 55, 157, LV_OPA_100);

    // 5. 密码控件
    char pwd_text[8] = {0};
    snprintf(pwd_text, sizeof(pwd_text), "%d", pwd_count);
    const char *pwd_img_path = (pwd_count > 0) ? "H:pwd_has_record.png" : "H:pwd_no_record.png";
    g_pwd_imgs[member_idx] = create_image_obj(member_con, pwd_img_path, 88, 157);
    g_pwd_labels[member_idx] = create_text_label
    (member_con, pwd_text, &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 124, 157, LV_OPA_100);

    // 6. 卡片控件
    char card_text[8] = {0};
    snprintf(card_text, sizeof(card_text), "%d", card_count);
    const char *card_img_path = (card_count > 0) ? "H:card_has_record.png" : "H:card_no_record.png";
    g_card_imgs[member_idx] = create_image_obj(member_con, card_img_path, 152, 146);
    g_card_labels[member_idx] = create_text_label
    (member_con, card_text, &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 202, 157, LV_OPA_100);

    // 7. 人脸控件
    char face_text[8] = {0};
    snprintf(face_text, sizeof(face_text), "%d", face_count);
    const char *face_img_path = (face_count > 0) ? "H:face_has_record.png" : "H:face_no_record.png";
    g_face_imgs[member_idx] = create_image_obj(member_con, face_img_path, 230, 150);
    g_face_labels[member_idx] = create_text_label
    (member_con, face_text, &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 271, 157, LV_OPA_100);

    // 8. 删除按钮
    lv_obj_t *delete_hid = create_container
    (member_con, 250, 10, 36, 36, lv_color_hex(0x192A46), LV_OPA_100, 100, lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_add_flag(delete_hid, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(delete_hid, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(delete_hid, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(delete_hid, member_delete_btn_click_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)member_idx);
    g_delete_hid_containers[member_idx] = delete_hid;
    g_delete_flag_imgs[member_idx] = NULL;
    g_member_selected[member_idx] = false;

    return member_con;
}


//实时更新成员卡片上的计数和图标
void update_member_count_ui(uint8_t member_idx)
{
    if(member_idx >= MAX_FAMILY_MEMBER_COUNT) return;

    // 1. 安全校验：标签对象是否有效
    if(g_finger_labels[member_idx] == NULL || !lv_obj_is_valid(g_finger_labels[member_idx]) ||
       g_pwd_labels[member_idx] == NULL || !lv_obj_is_valid(g_pwd_labels[member_idx]) ||
       g_card_labels[member_idx] == NULL || !lv_obj_is_valid(g_card_labels[member_idx]) ||
       g_face_labels[member_idx] == NULL || !lv_obj_is_valid(g_face_labels[member_idx])) {
        LV_LOG_WARN("Count labels for member %d are invalid, skip update", member_idx);
        return;
    }

    // 2. 读取最新计数
    uint8_t finger_count = family_member_list[member_idx].finger_count;
    uint8_t pwd_count = family_member_list[member_idx].pwd_count;
    uint8_t card_count = family_member_list[member_idx].card_count;
    uint8_t face_count = family_member_list[member_idx].face_count;

    // 3. 更新指纹计数文本
    char finger_text[8] = {0};
    snprintf(finger_text, sizeof(finger_text), "%d", finger_count);
    lv_label_set_text(g_finger_labels[member_idx], finger_text);
    // 更新指纹图标（可选）
    if(g_finger_imgs[member_idx] != NULL && lv_obj_is_valid(g_finger_imgs[member_idx])) {
        const char *finger_img_path = (finger_count > 0) ? "H:finger_has_record.png" : "H:finger_no_record.png";
        lv_img_set_src(g_finger_imgs[member_idx], finger_img_path);
    }

    // 4. 更新密码计数文本
    char pwd_text[8] = {0};
    snprintf(pwd_text, sizeof(pwd_text), "%d", pwd_count);
    lv_label_set_text(g_pwd_labels[member_idx], pwd_text);
    if(g_pwd_imgs[member_idx] != NULL && lv_obj_is_valid(g_pwd_imgs[member_idx])) {
        const char *pwd_img_path = (pwd_count > 0) ? "H:pwd_has_record.png" : "H:pwd_no_record.png";
        lv_img_set_src(g_pwd_imgs[member_idx], pwd_img_path);
    }

    // 5. 更新卡片计数文本
    char card_text[8] = {0};
    snprintf(card_text, sizeof(card_text), "%d", card_count);
    lv_label_set_text(g_card_labels[member_idx], card_text);
    if(g_card_imgs[member_idx] != NULL && lv_obj_is_valid(g_card_imgs[member_idx])) {
        const char *card_img_path = (card_count > 0) ? "H:card_has_record.png" : "H:card_no_record.png";
        lv_img_set_src(g_card_imgs[member_idx], card_img_path);
    }

    // 6. 更新人脸计数文本
    char face_text[8] = {0};
    snprintf(face_text, sizeof(face_text), "%d", face_count);
    lv_label_set_text(g_face_labels[member_idx], face_text);
    if(g_face_imgs[member_idx] != NULL && lv_obj_is_valid(g_face_imgs[member_idx])) {
        const char *face_img_path = (face_count > 0) ? "H:face_has_record.png" : "H:face_no_record.png";
        lv_img_set_src(g_face_imgs[member_idx], face_img_path);
    }

    // 7. 强制刷新UI
    lv_obj_invalidate(lv_scr_act());
    LV_LOG_INFO("Updated count UI for member %d: finger=%d, pwd=%d, card=%d, face=%d",
                member_idx, finger_count, pwd_count, card_count, face_count);
}

//实时更新

/**
 * @brief 头像点击回调函数
 * 
 * 处理头像点击事件，根据当前成员数量判断是否允许  成员。
 * 如果成员数量未达上限，会弹出输入框请求成员名称，
 * 并根据用户输入创建新的家庭成员卡片。
 * 
 * @param e 事件对象指针，包含点击事件信息
 */
static void avatar_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *avatar = lv_event_get_target(e);
    //uint8_t avatar_idx = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    
    if(member_count >= MAX_FAMILY_MEMBER_COUNT) {
            LV_LOG_USER("成员数量已达上限（%d个),无法  ", MAX_FAMILY_MEMBER_COUNT);
            close_custom_popup();
            return;
        }
    
    //LV_LOG_USER("%d", avatar_idx + 1);
    
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
    uint8_t member_idx = save_family_member_info(member_name, selected_avatar_color);
    if(member_idx == MAX_FAMILY_MEMBER_COUNT) {
        LV_LOG_WARN("No empty slot for new member");
        close_custom_popup();
        return;
    }
    
    // 4. 创建新成员卡片
    if(family_menber_scr != NULL && lv_obj_is_valid(family_menber_scr)) {
        // 旧代码（删除）
        // create_family_member_card(family_menber_scr, next_member_y, member_name, selected_avatar_color, member_idx);

        // 新代码（替换）
        create_family_member_card(family_menber_scr, member_name, selected_avatar_color, member_idx);
       // next_member_y += 179;
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

// 名称输入框点击回调
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
    lv_label_get_text(lv_obj_get_child(btn, 0));

    close_custom_popup();
}

// 关闭按钮（X）点击回调
static void popup_close_btn_cb(lv_event_t *e)
{
    close_custom_popup();
}


/**
 * @brief 家庭成员添加按钮点击回调函数
 * 
 * 处理添加家庭成员按钮点击事件，根据当前成员数量判断是否允许  成员。
 * 如果成员数量未达上限，会弹出输入框请求成员名称，
 * 并根据用户输入创建新的家庭成员卡片。
 * 
 * @param e 事件对象指针，包含点击事件信息
 */
void family_menber_add_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *user_manage_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(user_manage_scr == NULL || family_menber_scr == NULL) {
        LV_LOG_WARN("family_menber_add_click_cb: screen obj is NULL!");
        return;
    }

    // 1. 先关闭旧弹窗
    close_custom_popup();

    // 2. 创建/显示背景遮罩层
    create_bg_mask_layer(family_menber_scr);

    // 3. 创建自定义弹窗主体
    custom_popup = create_container
    (family_menber_scr, 212, 94, 600, 423, lv_color_hex(0xE0EDFF), LV_OPA_100, 16, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(custom_popup, 0, LV_STATE_DEFAULT);

    // 4. 名称区域
    create_text_label(custom_popup, "name:", &lv_font_montserrat_24, lv_color_hex(0x7C7C7C), 51, 69, LV_OPA_100);
    
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
    create_text_label(custom_popup, "Avatar:", &lv_font_montserrat_24, lv_color_hex(0x7C7C7C), 41, 124, LV_OPA_100);
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
    g_name_input = name_input;
    // 关闭X按钮（右上角）
    //lv_obj_t *close_img = create_image_obj(custom_popup, "H:X.png", 540, 20);
    // lv_obj_add_flag(close_img, LV_OBJ_FLAG_CLICKABLE);
    // lv_obj_set_style_opa(close_img, LV_OPA_80, LV_STATE_PRESSED);
    // lv_obj_add_event_cb(close_img, popup_close_btn_cb, LV_EVENT_CLICKED, NULL);
// 创建渐变容器（不变）
    lv_obj_t *back_con = create_custom_gradient_container
        (custom_popup, 540, 20, 40, 40, 200, 0xE0EDFF, 0xE0EDFF, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_set_style_pad_all(back_con, 0, LV_STATE_DEFAULT);
    // ===================== 绘制 X 号（核心修改）=====================
    // 第一条斜线：左上角 → 右下角 (0,0) → (40,40)
    lv_obj_t *x_line1 = lv_line_create(back_con);
    static lv_point_t x_points1[] = {{5, 5}, {35, 35}};  // 留边，不贴边
    config_divider_line_style(x_line1, x_points1, 2, 0x000000, 5, LV_OPA_100);
    // 第二条斜线：右上角 → 左下角 (40,0) → (0,40)
    lv_obj_t *x_line2 = lv_line_create(back_con);
    static lv_point_t x_points2[] = {{35, 5}, {5, 35}};
    config_divider_line_style(x_line2, x_points2, 2, 0x000000, 5, LV_OPA_100);
    // ==============================================================
    // 容器点击属性（不变）
    lv_obj_add_flag(back_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_con, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_con, popup_close_btn_cb, LV_EVENT_CLICKED, NULL);
    // 7. 确保弹窗在最上层
    lv_obj_move_foreground(custom_popup);
}



// 计算成员列表需要的最小遮罩层高度
static lv_coord_t get_member_list_max_height(void)
{
    lv_coord_t max_bottom_y = 600; // 默认最小高度600
    lv_coord_t card_height = 175;   // 单个成员卡片的高度（和create_family_member_card中一致）
    
    // 遍历所有有效成员，找到最底部成员的Y坐标 + 卡片高度
    for(uint8_t i = 0; i < MAX_FAMILY_MEMBER_COUNT; i++) {
        if(family_member_list[i].is_valid && g_member_cards[i] != NULL && lv_obj_is_valid(g_member_cards[i])) {
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
    lv_coord_t mask_width = 1024;  // 固定宽度（屏幕宽度）
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
    if(family_menber_scr == NULL || !lv_obj_is_valid(family_menber_scr)) return;
    
    for(uint8_t idx = 0; idx < MAX_FAMILY_MEMBER_COUNT; idx++) {
        if(!family_member_list[idx].is_valid) continue;
        
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
        for(uint8_t i = 0; i < MAX_FAMILY_MEMBER_COUNT; i++) {
            g_member_selected[i] = false;
        }
    }
}

// 检查并更新添加按钮状态
static void update_add_member_btn_state(void)
{
    
    if(family_menber_add_con != NULL && lv_obj_is_valid(family_menber_add_con)) {
        if(member_count >= MAX_FAMILY_MEMBER_COUNT) {
            lv_obj_clear_flag(family_menber_add_con, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(family_menber_add_con, LV_OPA_50, LV_STATE_DEFAULT);
        } else {
            lv_obj_add_flag(family_menber_add_con, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(family_menber_add_con, LV_OPA_100, LV_STATE_DEFAULT);
        }
    }
    if(family_menber_add != NULL && lv_obj_is_valid(family_menber_add_con)) {
        if(member_count >= MAX_FAMILY_MEMBER_COUNT) {
            lv_obj_clear_flag(family_menber_add, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(family_menber_add, LV_OPA_50, LV_STATE_DEFAULT);
        } else {
            lv_obj_add_flag(family_menber_add, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(family_menber_add, LV_OPA_100, LV_STATE_DEFAULT);
        }
    }
}

void destroy_family_member(void)
{
    if(family_menber_scr == NULL || !lv_obj_is_valid(family_menber_scr)) return;
    lv_obj_del(family_menber_scr);
    family_menber_scr = NULL;
    LV_LOG_WARN("Family member response: Destroy the family member interface");
}
#endif