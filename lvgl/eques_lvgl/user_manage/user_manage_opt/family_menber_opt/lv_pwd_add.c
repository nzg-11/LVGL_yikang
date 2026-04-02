#include "lv_pwd_add.h"
#include "lv_a_enroll_opt.h"
#include <string.h>
#include <stdio.h>

// ========== 全局变量定义 ==========
static lv_obj_t *pwd_add_scr = NULL; 
static lv_style_t pwd_add_grad_style;
static bool pwd_add_style_inited = false;

// 密码核心全局变量
static char g_pwd_first[7] = {0};        // 第一次输入的6位密码（+结束符）
static char g_pwd_second[7] = {0};       // 第二次确认的6位密码
static uint8_t g_pwd_input_len = 0;      // 当前输入密码长度
static bool g_is_confirm_mode = false;   // 是否进入密码确认模式

// UI控件全局引用
static lv_obj_t *g_pwd_show_con[6] = {NULL}; // 6个密码显示容器
static lv_obj_t *g_pwd_prompt_label = NULL;  // 提示文本标签
static lv_obj_t *g_pwd_dialog_mask = NULL;   // 弹窗遮罩层
static lv_obj_t *g_pwd_dialog = NULL;        // 弹窗主体
static lv_obj_t *g_pwd_keyboard = NULL;      // 密码名称输入键盘
static lv_obj_t *g_pwd_name_input = NULL;    // 密码名称输入框
static char g_pwd_custom_name[17] = {0};     // 自定义密码名称（默认pwd01）

// ========== 函数声明 ==========
static void init_pwd_add_styles(void);
static void close_pwd_dialog(void);
static void pwd_num_click_cb(lv_event_t *e);
static void pwd_backspace_click_cb(lv_event_t *e);
static void pwd_confirm_click_cb(lv_event_t *e);
static void pwd_dialog_confirm_cb(lv_event_t *e);
static void show_pwd_dialog(const char *msg, bool is_success, lv_obj_t *enroll_scr);
static void pwd_name_input_click_cb(lv_event_t *e);
static void hide_pwd_keyboard(lv_event_t *e);

extern bool g_member_info_inited;
extern pwd_enroll_info_t *get_current_pwd_info(void);
// ========== 样式初始化 ==========
static void init_pwd_add_styles(void)
{
    if(!pwd_add_style_inited) {
        lv_style_init(&pwd_add_grad_style);
        pwd_add_style_inited = true;
    }
}

// ========== 关闭弹窗通用函数（和指纹一致） ==========
static void close_pwd_dialog(void)
{
    // 隐藏键盘（和指纹一致）
    if(g_pwd_keyboard != NULL && lv_obj_is_valid(g_pwd_keyboard)) {
        lv_obj_add_flag(g_pwd_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
    
    // 隐藏遮罩层（和指纹一致）
    if(g_pwd_dialog_mask != NULL && lv_obj_is_valid(g_pwd_dialog_mask)) {
        lv_obj_add_flag(g_pwd_dialog_mask, LV_OBJ_FLAG_HIDDEN);
    }
    
    // 销毁弹窗主体（和指纹一致）
    if(g_pwd_dialog != NULL && lv_obj_is_valid(g_pwd_dialog)) {
        lv_obj_del(g_pwd_dialog);
        g_pwd_dialog = NULL;
    }
    
    // 清空输入框（可选）
    if(g_pwd_name_input != NULL && lv_obj_is_valid(g_pwd_name_input)) {
        lv_textarea_set_text(g_pwd_name_input, "");
        g_pwd_name_input = NULL;
    }
}

// ========== 隐藏密码键盘 ==========
static void hide_pwd_keyboard(lv_event_t *e)
{
    (void)e;
    if(g_pwd_keyboard != NULL && lv_obj_is_valid(g_pwd_keyboard)) {
        lv_obj_add_flag(g_pwd_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

// ========== 密码名称输入框点击回调（弹出键盘） ==========
static void pwd_name_input_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    lv_obj_t *input = lv_event_get_target(e);
    
    // 创建键盘（复用逻辑）
    if(g_pwd_keyboard == NULL || !lv_obj_is_valid(g_pwd_keyboard)) {
        g_pwd_keyboard = lv_keyboard_create(lv_scr_act());
        // 键盘样式（和指纹键盘一致）
        lv_obj_set_style_bg_color(g_pwd_keyboard, lv_color_hex(0x192A46), LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(g_pwd_keyboard, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(g_pwd_keyboard, lv_color_hex(0x1F3150), LV_STATE_DEFAULT);
        lv_obj_set_style_radius(g_pwd_keyboard, 6, LV_STATE_DEFAULT);
        // 适配屏幕尺寸
        lv_obj_set_size(g_pwd_keyboard, LV_HOR_RES, LV_VER_RES/3);
        lv_obj_align(g_pwd_keyboard, LV_ALIGN_BOTTOM_MID, 0, 100);
        lv_obj_add_event_cb(g_pwd_keyboard, hide_pwd_keyboard, LV_EVENT_READY, NULL);
    }
    
    // 显示键盘并关联输入框
    lv_obj_clear_flag(g_pwd_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(g_pwd_keyboard, input);
    lv_obj_move_foreground(g_pwd_keyboard);
}

// ========== 显示密码结果弹窗 ==========
static void show_pwd_dialog(const char *msg, bool is_success, lv_obj_t *enroll_scr)
{
    // 先关闭旧弹窗
    close_pwd_dialog();

    // 1. 创建遮罩层
    g_pwd_dialog_mask = lv_obj_create(pwd_add_scr);
    lv_obj_set_size(g_pwd_dialog_mask, LV_HOR_RES, 1280);
    lv_obj_set_pos(g_pwd_dialog_mask, 0, 0);
    lv_obj_set_style_bg_color(g_pwd_dialog_mask, lv_color_hex(0x000000), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(g_pwd_dialog_mask, LV_OPA_70, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(g_pwd_dialog_mask, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(g_pwd_dialog_mask, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(g_pwd_dialog_mask, LV_OBJ_FLAG_HIDDEN);

    // 2. 创建弹窗主体
    g_pwd_dialog = create_container
    (pwd_add_scr, 100, 455, 600, is_success ? 270 : 180,
     lv_color_hex(0xE0EDFF), LV_OPA_100,
     16, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(g_pwd_dialog, 0, LV_STATE_DEFAULT);

    // 3. 弹窗提示文本
    lv_obj_t *dialog_label = create_text_label
    (g_pwd_dialog, msg, &lv_font_montserrat_24, lv_color_hex(0x000000), 0, 38, LV_OPA_100);
    lv_obj_align(dialog_label, LV_ALIGN_TOP_MID, 0, 38);

    // 4. 成功弹窗：添加名称输入框（和指纹一致）
    if(is_success) {
        g_pwd_name_input = lv_textarea_create(g_pwd_dialog);
        lv_obj_clear_flag(g_pwd_name_input, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(g_pwd_name_input, 382, 44);
        lv_obj_set_pos(g_pwd_name_input, 137, 90);
        lv_obj_set_style_bg_color(g_pwd_name_input, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(g_pwd_name_input, lv_color_hex(0x333333), LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(g_pwd_name_input, 0, LV_STATE_DEFAULT);
        lv_obj_set_style_radius(g_pwd_name_input, 6, LV_STATE_DEFAULT);
        lv_textarea_set_placeholder_text(g_pwd_name_input, "please input name");
        lv_textarea_set_max_length(g_pwd_name_input, 16);
        lv_textarea_set_one_line(g_pwd_name_input, true);
        lv_obj_set_style_text_font(g_pwd_name_input, &lv_font_montserrat_24, LV_STATE_DEFAULT);
        lv_obj_add_flag(g_pwd_name_input, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_opa(g_pwd_name_input, LV_OPA_80, LV_STATE_PRESSED);
        lv_obj_add_event_cb(g_pwd_name_input, pwd_name_input_click_cb, LV_EVENT_CLICKED, NULL);
    }

    // 5. 确认按钮（完全复用指纹的绑定逻辑！）
    uint16_t btn_y = is_success ? 171 : 90;
    lv_obj_t *confirm_btn = create_custom_gradient_container
    (g_pwd_dialog, 197, btn_y, 205, 44, 6, 0X006BDC, 0x00BDBD, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_add_flag(confirm_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(confirm_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_set_style_pad_all(confirm_btn, 0, LV_STATE_DEFAULT);
    
    lv_obj_t *confirm_label = create_text_label
    (confirm_btn, "Confirm", &lv_font_montserrat_28, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_set_align(confirm_label, LV_ALIGN_CENTER);
    
    // 和指纹一致，直接把enroll_scr传给回调
    lv_obj_add_event_cb(confirm_btn, pwd_confirm_click_cb, LV_EVENT_CLICKED, enroll_scr);

    // 6. 弹窗置顶
    lv_obj_move_foreground(g_pwd_dialog);
}

// ========== 密码确认按钮回调
static void pwd_confirm_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    lv_obj_t *enroll_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(enroll_scr == NULL || !lv_obj_is_valid(enroll_scr)) {
        LV_LOG_WARN("pwd_confirm_click_cb: enroll_scr invalid!");
        return;
    }

    // 1. 判断弹窗是成功还是失败
    bool is_success = false;
    if(g_pwd_dialog != NULL && lv_obj_is_valid(g_pwd_dialog)) {
        lv_obj_t *dialog_label = lv_obj_get_child(g_pwd_dialog, 0);
        if(dialog_label != NULL && lv_obj_is_valid(dialog_label)) {
            const char *msg = lv_label_get_text(dialog_label);
            if(msg != NULL && strcmp(msg, "add succeed") == 0) {
                is_success = true;
            }
        }
    }

    // 2. 只有成功时才执行添加逻辑 + 切回上级界面
    if(is_success) {
        const char *pwd_name = "pwd01";
        if(g_pwd_name_input != NULL && lv_obj_is_valid(g_pwd_name_input)) {
            const char *input_name = lv_textarea_get_text(g_pwd_name_input);
            if(input_name != NULL && strlen(input_name) > 0) {
                pwd_name = input_name;
            }
        }
        // 执行添加逻辑
        pwd_enroll_complete(pwd_name);
        update_status_bar_parent(enroll_scr);
        lv_scr_load(enroll_scr);
    }

    // 3. 关闭弹窗（成功/失败都执行）
    close_pwd_dialog();

    // 4. 重置密码添加进度（成功/失败都执行）
    memset(g_pwd_first, 0, sizeof(g_pwd_first));
    memset(g_pwd_second, 0, sizeof(g_pwd_second));
    g_pwd_input_len = 0;
    g_is_confirm_mode = false;
    for(int i=0; i<6; i++) {
        if(g_pwd_show_con[i] != NULL && lv_obj_is_valid(g_pwd_show_con[i])) {
            lv_label_set_text(g_pwd_show_con[i], "");
        }
    }
    if(g_pwd_prompt_label != NULL && lv_obj_is_valid(g_pwd_prompt_label)) {
        lv_label_set_text(g_pwd_prompt_label, "Please input 6-digit password");
    }
}

// ========== 数字按键回调（修复show_pwd_dialog传参） ==========
static void pwd_num_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    // 获取按键数字
    lv_obj_t *num_btn = lv_event_get_target(e);
    const char *num = lv_obj_get_user_data(num_btn);
    if(num == NULL || strlen(num) == 0) return;

    // 限制输入长度（最多6位）
    if(g_pwd_input_len >= 6) return;

    // 存储密码字符
    if(!g_is_confirm_mode) {
        g_pwd_first[g_pwd_input_len] = num[0];
    } else {
        g_pwd_second[g_pwd_input_len] = num[0];
    }
    g_pwd_input_len++;

    // 更新密码显示容器
    lv_label_set_text(g_pwd_show_con[g_pwd_input_len-1], num);

    // 6位输入完成逻辑
    if(g_pwd_input_len == 6) {
        if(!g_is_confirm_mode) {
            // 第一次输入完成：进入确认模式
            g_is_confirm_mode = true;
            g_pwd_input_len = 0;
            lv_label_set_text(g_pwd_prompt_label, "Please enter the six-digit password again.");
            for(int i=0; i<6; i++) {
                lv_label_set_text(g_pwd_show_con[i], "");
            }
        } else {
            // 第二次输入完成：验证密码
            g_is_confirm_mode = false;
            g_pwd_input_len = 0;
            
            // ✅ 关键：和指纹一致，通过e获取enroll_scr并传给弹窗
            lv_obj_t *enroll_scr = (lv_obj_t *)lv_event_get_user_data(e);
            if(strcmp(g_pwd_first, g_pwd_second) == 0) {
                show_pwd_dialog("add succeed", true, enroll_scr);
            } else {
                show_pwd_dialog("add fail", false, enroll_scr);
            }
        }
    }
}

// ========== 退格键回调 ==========
static void pwd_backspace_click_cb(lv_event_t *e)
{
    if(e == NULL || g_pwd_input_len == 0) return;

    // 清空最后一位输入
    g_pwd_input_len--;
    if(!g_is_confirm_mode) {
        g_pwd_first[g_pwd_input_len] = '\0';
    } else {
        g_pwd_second[g_pwd_input_len] = '\0';
    }
    // 清空显示容器最后一位
    lv_label_set_text(g_pwd_show_con[g_pwd_input_len], "");
}

// ========== 创建数字按键通用函数 ==========
static lv_obj_t *create_pwd_num_btn(lv_obj_t *parent, uint16_t x, uint16_t y, const char *num, lv_obj_t *enroll_scr)
{
    lv_obj_t *num_btn = create_container
    (parent, x, y, 109, 109, lv_color_hex(0xFFFFFF), LV_OPA_100, 100, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    
    // 创建数字标签并居中
    lv_obj_t *num_label = create_text_label
    (num_btn, num, &lv_font_montserrat_48, lv_color_hex(0x000000), 0, 0, LV_OPA_100);
    lv_obj_align(num_label, LV_ALIGN_CENTER, 0, 0);
    
    // 设置按键样式
    lv_obj_add_flag(num_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(num_btn, LV_OPA_90, LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(num_btn, lv_color_hex(0xD9D9D9), LV_STATE_PRESSED);
    
    // 存储数字到用户数据
    lv_obj_set_user_data(num_btn, (void*)num);
    // 绑定点击回调
    lv_obj_add_event_cb(num_btn, pwd_num_click_cb, LV_EVENT_CLICKED, enroll_scr);
    
    return num_btn;
}

// ========== 密码添加界面创建 ==========
void ui_pwd_add_create(lv_obj_t *enroll_scr)
{
    init_pwd_add_styles();
    
    // 安全校验
    if(enroll_scr == NULL) {
        LV_LOG_WARN("ui_pwd_add_create: enroll_scr is NULL!");
        return;
    }

    // 创建/复用屏幕
    if(pwd_add_scr == NULL) {
        pwd_add_scr = lv_obj_create(NULL);
    } else {
        lv_obj_clean(pwd_add_scr);
        // 重置密码状态
        memset(g_pwd_first, 0, sizeof(g_pwd_first));
        memset(g_pwd_second, 0, sizeof(g_pwd_second));
        g_pwd_input_len = 0;
        g_is_confirm_mode = false;
        // 关闭旧弹窗
        close_pwd_dialog();
    }

    // 屏幕渐变样式（和指纹界面一致）
    lv_style_reset(&pwd_add_grad_style);
    lv_style_set_bg_color(&pwd_add_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&pwd_add_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&pwd_add_grad_style, LV_GRAD_DIR_VER);
    lv_style_set_bg_main_stop(&pwd_add_grad_style, 0);
    lv_style_set_bg_grad_stop(&pwd_add_grad_style, 255);
    lv_obj_add_style(pwd_add_scr, &pwd_add_grad_style, LV_STATE_DEFAULT);

    // 左上角返回按钮（和指纹界面一致）
    lv_obj_t *back_btn = create_image_obj(pwd_add_scr, "H:back.png", 52, 123);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn, back_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);

    // 标题文本
    lv_obj_t *pwd_add_label = create_text_label
    (pwd_add_scr, "Add Password", &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 0, 115, LV_OPA_100);
    lv_obj_align(pwd_add_label, LV_ALIGN_TOP_MID, 0, 115);

    // 提示文本（保存到全局）
    g_pwd_prompt_label = create_text_label
    (pwd_add_scr, "Please input 6-digit password", &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 0, 217, LV_OPA_100);
    lv_obj_align(g_pwd_prompt_label, LV_ALIGN_TOP_MID, 0, 217);

    // 创建6个密码显示容器
    uint16_t con_x = 76;
    for(int i=0; i<6; i++) {
        g_pwd_show_con[i] = create_container
        (pwd_add_scr, con_x, 332, 99, 119, lv_color_hex(0xFFFFFF), LV_OPA_100, 6, lv_color_hex(0x1F3150), 0, LV_OPA_90);
        // 每个容器添加空标签（用于显示数字）
        lv_obj_t *show_label = create_text_label
        (g_pwd_show_con[i], "", &lv_font_montserrat_48, lv_color_hex(0x000000), 0, 0, LV_OPA_100);
        lv_obj_align(show_label, LV_ALIGN_CENTER, 0, 0);
        g_pwd_show_con[i] = show_label; // 直接保存标签对象，方便更新
        con_x += 110; // 容器间距
    }

    // 密码输入键盘容器
    lv_obj_t *pwd_input_con = create_container
    (pwd_add_scr, 0, 643, 800, 637, lv_color_hex(0xE0EDFF), LV_OPA_100, 32, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(pwd_input_con, 0, LV_STATE_DEFAULT);

    // 创建数字按键（0-9）
    create_pwd_num_btn(pwd_input_con, 131, 33, "1", enroll_scr);
    create_pwd_num_btn(pwd_input_con, 345, 33, "2", enroll_scr);
    create_pwd_num_btn(pwd_input_con, 559, 33, "3", enroll_scr);
    create_pwd_num_btn(pwd_input_con, 131, 182, "4", enroll_scr);
    create_pwd_num_btn(pwd_input_con, 345, 182, "5", enroll_scr);
    create_pwd_num_btn(pwd_input_con, 559, 182, "6", enroll_scr);
    create_pwd_num_btn(pwd_input_con, 131, 331, "7", enroll_scr);
    create_pwd_num_btn(pwd_input_con, 345, 331, "8", enroll_scr);
    create_pwd_num_btn(pwd_input_con, 559, 331, "9", enroll_scr);
    create_pwd_num_btn(pwd_input_con, 345, 480, "0", enroll_scr);

    // 创建退格键
    lv_obj_t *pwd_back_btn = create_container
    (pwd_input_con, 559, 480, 109, 109, lv_color_hex(0xFFFFFF), LV_OPA_100, 100, lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_t *back_img = create_image_obj(pwd_back_btn, "H:delete_keyboard.png", 0, 0);
    lv_obj_align(back_img, LV_ALIGN_CENTER, -3, 0);
    lv_obj_add_flag(pwd_back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(pwd_back_btn, LV_OPA_90, LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(pwd_back_btn, lv_color_hex(0xD9D9D9), LV_STATE_PRESSED);
    lv_obj_add_event_cb(pwd_back_btn, pwd_backspace_click_cb, LV_EVENT_CLICKED, NULL);

    // 更新状态栏并切换屏幕
    update_status_bar_parent(pwd_add_scr);
    lv_scr_load(pwd_add_scr);
}

// ========== 密码添加按钮回调 ==========
void pwd_add_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    lv_obj_t *enroll_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(enroll_scr == NULL) {
        LV_LOG_WARN("pwd_add_btn_click_cb: enroll_scr is NULL!");
        return;
    }
    ui_pwd_add_create(enroll_scr);
}

// ========== 外部获取密码的接口（预留） ==========
const char *get_pwd_input(void)
{
    return g_pwd_first; // 返回最终确认的密码
}