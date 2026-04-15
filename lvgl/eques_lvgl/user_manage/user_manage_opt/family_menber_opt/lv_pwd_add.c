#include "lv_pwd_add.h"
#include "lv_a_enroll_opt.h"
#include <string.h>
#include <stdio.h>

/********************************************************************************
 *                                  宏定义配置
 *******************************************************************************/
#define PWD_MAX_LEN        6       // 密码最大长度
#define PWD_BUF_SIZE       7       // 密码缓存长度(含结束符)
#define DIALOG_DELAY_MS    100     // 弹窗延时时间(ms)
#define KEYBOARD_HEIGHT    250     // 键盘高度

/********************************************************************************
 *                              静态全局变量(分组管理)
 *******************************************************************************/
// 1. 页面与样式
static lv_obj_t    *pwd_add_scr = NULL;               // 密码添加主页面
static lv_style_t   pwd_add_grad_style;               // 背景渐变样式
static bool         pwd_add_style_inited = false;     // 样式初始化标志

// 2. 密码输入核心数据
static char         g_pwd_first[PWD_BUF_SIZE] = {0};  // 第一次输入密码
static char         g_pwd_second[PWD_BUF_SIZE] = {0}; // 第二次确认密码
static uint8_t      g_pwd_input_len = 0;              // 当前输入长度
static bool         g_is_confirm_mode = false;        // 密码确认模式标志

// 3. UI控件句柄
static lv_obj_t    *g_pwd_prompt_label = NULL;        // 提示文本标签
static lv_obj_t    *g_pwd_display_label = NULL;       // 密码显示标签
static lv_obj_t    *g_pwd_dialog_mask = NULL;         // 弹窗遮罩层
static lv_obj_t    *g_pwd_dialog = NULL;              // 弹窗容器
static lv_obj_t    *g_pwd_keyboard = NULL;            // 软键盘
static lv_obj_t    *g_pwd_name_input = NULL;          // 密码名称输入框

// 4. 延时任务临时变量
static lv_obj_t    *g_tmp_enroll_scr = NULL;          // 临时注册页面
static bool         g_tmp_need_switch_mode = false;   // 延时切换模式标志
static bool         g_tmp_is_success = false;         // 延时结果标志
static const char  *g_tmp_msg = NULL;                 // 延时提示文本

/********************************************************************************
 *                              函数声明(分类模块化)
 *******************************************************************************/
// 一、UI主页面绘制 (核心前置)
void ui_pwd_add_create(lv_obj_t *enroll_scr);

// 二、外部接口回调
void pwd_add_btn_click_cb(lv_event_t *e);
void pwd_add_back_btn_click_cb(lv_event_t *e);
const char *get_pwd_input(void);

// 三、初始化相关
static void init_pwd_add_styles(void);

// 四、弹窗操作相关
static void close_pwd_dialog(void);
static void show_pwd_dialog(const char *msg, bool is_success, lv_obj_t *enroll_scr);
static void pwd_error_confirm_cb(lv_event_t *e);

// 五、延时任务
static void pwd_delay_task_cb(lv_timer_t *timer);

// 六、按键回调函数
static void pwd_num_click_cb(lv_event_t *e);
static void pwd_backspace_click_cb(lv_event_t *e);
static void pwd_confirm_click_cb(lv_event_t *e);

// 七、软键盘相关
static void pwd_name_input_click_cb(lv_event_t *e);
static void hide_pwd_keyboard(lv_event_t *e);

// 八、辅助创建函数
static lv_obj_t *create_pwd_num_btn(lv_obj_t *parent, uint16_t x, uint16_t y, const char *num, lv_obj_t *enroll_scr);

/********************************************************************************
 *                              函数实现(主流程优先)
 *******************************************************************************/

/**
 * @brief  密码添加页面主绘制函数 (UI核心，前置优先)
 * @param  enroll_scr: 父注册页面
 */
void ui_pwd_add_create(lv_obj_t *enroll_scr)
{
    init_pwd_add_styles();
    
    if (enroll_scr == NULL) {
        LV_LOG_WARN("ui_pwd_add_create: enroll_scr is NULL!");
        return;
    }

    // 关闭旧弹窗、销毁旧页面
    close_pwd_dialog();
    if (is_lv_obj_valid(pwd_add_scr)) {
        lv_obj_del(pwd_add_scr);
        pwd_add_scr = NULL;
    }

    // 创建新页面
    pwd_add_scr = lv_obj_create(NULL);
    
    // 重置密码数据
    memset(g_pwd_first, 0, sizeof(g_pwd_first));
    memset(g_pwd_second, 0, sizeof(g_pwd_second));
    g_pwd_input_len = 0;
    g_is_confirm_mode = false;

    // 设置页面背景渐变样式
    lv_style_reset(&pwd_add_grad_style);
    lv_style_set_bg_color(&pwd_add_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&pwd_add_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&pwd_add_grad_style, LV_GRAD_DIR_VER);
    lv_style_set_bg_main_stop(&pwd_add_grad_style, 0);
    lv_style_set_bg_grad_stop(&pwd_add_grad_style, 255);
    lv_obj_add_style(pwd_add_scr, &pwd_add_grad_style, LV_STATE_DEFAULT);
    
    // 标题文本
    create_text_label(pwd_add_scr, "Add Password", &lv_font_montserrat_36, 
                      lv_color_hex(0xFFFFFF), 83, 80, LV_OPA_100);

    // 密码显示区域
    lv_obj_t *pwd_show_con = create_container(pwd_add_scr, 48, 150, 928, 83,
                    lv_color_hex(0x2A4573), LV_OPA_100, 6, 
                    lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(pwd_show_con, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(pwd_show_con, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(pwd_show_con, LV_OPA_80, LV_STATE_PRESSED);

    // 提示标签
    g_pwd_prompt_label = create_text_label(pwd_show_con, "Please input 6-digit password",
                    &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_70);
    lv_obj_align(g_pwd_prompt_label, LV_ALIGN_CENTER, 0, 0);

    // 密码显示标签(默认隐藏)
    g_pwd_display_label = create_text_label(pwd_show_con, "", &lv_font_montserrat_48,
                    lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_align(g_pwd_display_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(g_pwd_display_label, LV_OBJ_FLAG_HIDDEN);

    // 返回按钮
    lv_obj_t *back_btn = create_container_circle(pwd_add_scr, 52, 90, 30,
                    true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 3, LV_OPA_100);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_add_event_cb(back_btn, pwd_add_back_btn_click_cb, LV_EVENT_CLICKED, enroll_scr);

    // 创建数字按键 1/2/3/0
    create_pwd_num_btn(pwd_add_scr, 176, 257, "1", enroll_scr);
    create_pwd_num_btn(pwd_add_scr, 350, 257, "2", enroll_scr);
    create_pwd_num_btn(pwd_add_scr, 524, 257, "3", enroll_scr);
    create_pwd_num_btn(pwd_add_scr, 698, 257, "0", enroll_scr);
    // 创建数字按键 4/5/6
    create_pwd_num_btn(pwd_add_scr, 176, 365, "4", enroll_scr);
    create_pwd_num_btn(pwd_add_scr, 350, 365, "5", enroll_scr);
    create_pwd_num_btn(pwd_add_scr, 524, 365, "6", enroll_scr);
    // 创建数字按键 7/8/9/#
    create_pwd_num_btn(pwd_add_scr, 176, 472, "7", enroll_scr);
    create_pwd_num_btn(pwd_add_scr, 350, 472, "8", enroll_scr);
    create_pwd_num_btn(pwd_add_scr, 524, 472, "9", enroll_scr);
    create_pwd_num_btn(pwd_add_scr, 698, 472, "#", enroll_scr);
    
    // 退格按钮
    lv_obj_t *pwd_back_btn = create_container(pwd_add_scr, 698, 365, 150, 80,
                    lv_color_hex(0x7698D0), LV_OPA_100, 100, 
                    lv_color_hex(0x1F3150), 0, LV_OPA_70);
    lv_obj_t *back_img = create_image_obj(pwd_back_btn, "H:delete_keyboard.png", 0, 0);
    lv_obj_align(back_img, LV_ALIGN_CENTER, -3, 0);
    lv_obj_add_flag(pwd_back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(pwd_back_btn, LV_OPA_90, LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(pwd_back_btn, lv_color_hex(0xD9D9D9), LV_STATE_PRESSED);
    lv_obj_add_event_cb(pwd_back_btn, pwd_backspace_click_cb, LV_EVENT_CLICKED, NULL);

    // 加载页面
    update_status_bar_parent(pwd_add_scr);
    lv_scr_load(pwd_add_scr);
}

/**
 * @brief  密码添加按钮点击回调(外部入口)
 */
void pwd_add_btn_click_cb(lv_event_t *e)
{
    if (e == NULL) return;
    
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if (parent_scr == NULL) return;

    ui_pwd_add_create(parent_scr);
    update_status_bar_parent(pwd_add_scr);
    // destroy_enroll();
    // LV_LOG_WARN("进入密码添加界面，销毁录入界面");
}

/**
 * @brief  密码页面返回按钮回调
 */
void pwd_add_back_btn_click_cb(lv_event_t *e)
{
    if (e == NULL) return;
    
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);
    lv_obj_t *current_scr = lv_disp_get_scr_act(NULL);

    if (lv_obj_is_valid(current_scr) && current_scr == pwd_add_scr) {
        //common_member_info_t *member = get_current_enroll_member();
        // ui_enroll_create(member, parent_scr);  
        // lv_obj_del(current_scr);
        lv_scr_load(parent_scr);  // 直接切回原来的页面！
        lv_obj_del(current_scr);  // 只删除密码页
        current_scr = NULL;
    }
}

/**
 * @brief  获取第一次输入的密码(外部接口)
 */
const char *get_pwd_input(void)
{
    return g_pwd_first;
}

/**
 * @brief  样式初始化
 */
static void init_pwd_add_styles(void)
{
    if (!pwd_add_style_inited) {
        lv_style_init(&pwd_add_grad_style);
        pwd_add_style_inited = true;
    }
}

/**
 * @brief  关闭密码弹窗
 */
static void close_pwd_dialog(void)
{
    // 隐藏键盘
    if (lv_obj_is_valid(g_pwd_keyboard)) {
        lv_obj_add_flag(g_pwd_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
    // 隐藏遮罩
    if (lv_obj_is_valid(g_pwd_dialog_mask)) {
        lv_obj_add_flag(g_pwd_dialog_mask, LV_OBJ_FLAG_HIDDEN);
    }
    // 销毁弹窗
    if (lv_obj_is_valid(g_pwd_dialog)) {
        lv_obj_del(g_pwd_dialog);
        g_pwd_dialog = NULL;
    }
    // 清空输入框
    if (lv_obj_is_valid(g_pwd_name_input)) {
        lv_textarea_set_text(g_pwd_name_input, "");
        g_pwd_name_input = NULL;
    }
}

/**
 * @brief  显示结果弹窗
 */
static void show_pwd_dialog(const char *msg, bool is_success, lv_obj_t *enroll_scr)
{
    close_pwd_dialog();

    // 创建遮罩层
    g_pwd_dialog_mask = lv_obj_create(pwd_add_scr);
    lv_obj_set_size(g_pwd_dialog_mask, LV_HOR_RES, 1280);
    lv_obj_set_pos(g_pwd_dialog_mask, 0, 0);
    lv_obj_set_style_bg_color(g_pwd_dialog_mask, lv_color_hex(0x000000), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(g_pwd_dialog_mask, LV_OPA_70, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(g_pwd_dialog_mask, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(g_pwd_dialog_mask, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(g_pwd_dialog_mask, LV_OBJ_FLAG_HIDDEN);

    // 创建弹窗容器
    g_pwd_dialog = create_container(pwd_add_scr, 212, 150, 600, is_success ? 270 : 180,
                    lv_color_hex(0xE0EDFF), LV_OPA_100, 16, 
                    lv_color_hex(0x1F3150), 0, LV_OPA_90);
    lv_obj_set_style_pad_all(g_pwd_dialog, 0, LV_STATE_DEFAULT);

    // 弹窗提示文本
    lv_obj_t *dialog_label = create_text_label(g_pwd_dialog, msg, &lv_font_montserrat_24,
                    lv_color_hex(0x000000), 0, 38, LV_OPA_100);
    lv_obj_align(dialog_label, LV_ALIGN_TOP_MID, 0, 38);

    // 成功时显示名称输入框
    if (is_success) {
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
    
    // 确认按钮
    uint16_t btn_y = is_success ? 171 : 90;
    lv_obj_t *confirm_btn = create_custom_gradient_container(g_pwd_dialog, 197, btn_y, 205, 44,
                    6, 0X006BDC, 0x00BDBD, LV_GRAD_DIR_VER, 0, 225, LV_OPA_100);
    lv_obj_add_flag(confirm_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(confirm_btn, LV_OPA_80, LV_STATE_PRESSED);
    lv_obj_set_style_pad_all(confirm_btn, 0, LV_STATE_DEFAULT);
    
    // 按钮文本
    lv_obj_t *confirm_label = create_text_label(confirm_btn, "Confirm", &lv_font_montserrat_28,
                    lv_color_hex(0xFFFFFF), 0, 0, LV_OPA_100);
    lv_obj_set_align(confirm_label, LV_ALIGN_CENTER);

    // 绑定回调
    if (is_success) {
        lv_obj_add_event_cb(confirm_btn, pwd_confirm_click_cb, LV_EVENT_CLICKED, enroll_scr);
    } else {
        lv_obj_add_event_cb(confirm_btn, pwd_error_confirm_cb, LV_EVENT_CLICKED, NULL);
    }

    lv_obj_move_foreground(g_pwd_dialog);
}

/**
 * @brief  密码错误弹窗确认回调
 */
static void pwd_error_confirm_cb(lv_event_t *e)
{
    // 关闭弹窗
    close_pwd_dialog();

    // 重置所有数据
    memset(g_pwd_first, 0, sizeof(g_pwd_first));
    memset(g_pwd_second, 0, sizeof(g_pwd_second));
    g_pwd_input_len = 0;
    g_is_confirm_mode = false;

    // 重置UI显示
    lv_label_set_text(g_pwd_display_label, "");
    lv_obj_add_flag(g_pwd_display_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(g_pwd_prompt_label, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(g_pwd_prompt_label, "Please input 6-digit password");
}

/**
 * @brief  延时统一回调函数(LVGL8.3兼容)
 */
static void pwd_delay_task_cb(lv_timer_t *timer)
{
    lv_timer_del(timer);

    if (g_tmp_need_switch_mode) {
        // 切换到第二次输入密码
        g_is_confirm_mode = true;
        g_pwd_input_len = 0;
        lv_label_set_text(g_pwd_prompt_label, "Please enter the six-digit password again.");
        lv_label_set_text(g_pwd_display_label, "");
        lv_obj_add_flag(g_pwd_display_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_pwd_prompt_label, LV_OBJ_FLAG_HIDDEN);
    } else {
        // 弹出结果弹窗
        g_is_confirm_mode = false;
        g_pwd_input_len = 0;
        show_pwd_dialog(g_tmp_msg, g_tmp_is_success, g_tmp_enroll_scr);
    }
}

/**
 * @brief  数字按键点击回调
 */
static void pwd_num_click_cb(lv_event_t *e)
{
    if (e == NULL) return;
    
    lv_obj_t *num_btn = lv_event_get_target(e);
    const char *num = lv_obj_get_user_data(num_btn);
    if (num == NULL || strlen(num) == 0 || g_pwd_input_len >= PWD_MAX_LEN)
        return;

    // 保存输入的数字
    if (!g_is_confirm_mode)
        g_pwd_first[g_pwd_input_len] = num[0];
    else
        g_pwd_second[g_pwd_input_len] = num[0];
    g_pwd_input_len++;

    // 更新UI显示
    lv_obj_add_flag(g_pwd_prompt_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(g_pwd_display_label, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(g_pwd_display_label, g_is_confirm_mode ? g_pwd_second : g_pwd_first);

    // 未输满6位，直接返回
    if (g_pwd_input_len != PWD_MAX_LEN)
        return;

    // 输满6位，延时执行后续逻辑
    lv_obj_t *enroll_scr = (lv_obj_t *)lv_event_get_user_data(e);
    g_tmp_enroll_scr = enroll_scr;

    if (!g_is_confirm_mode) {
        g_tmp_need_switch_mode = true;
    } else {
        g_tmp_need_switch_mode = false;
        g_tmp_is_success = (strcmp(g_pwd_first, g_pwd_second) == 0);
        g_tmp_msg = g_tmp_is_success ? "add succeed" : "add fail";
    }

    // 创建延时定时器
    lv_timer_create(pwd_delay_task_cb, DIALOG_DELAY_MS, NULL);
}

/**
 * @brief  退格按键点击回调
 */
static void pwd_backspace_click_cb(lv_event_t *e)
{
    if (e == NULL || g_pwd_input_len == 0) return;

    // 删除最后一位
    g_pwd_input_len--;
    if (!g_is_confirm_mode) {
        g_pwd_first[g_pwd_input_len] = '\0';
    } else {
        g_pwd_second[g_pwd_input_len] = '\0';
    }

    // 更新显示
    lv_label_set_text(g_pwd_display_label, g_is_confirm_mode ? g_pwd_second : g_pwd_first);
    
    // 输入为空时显示提示语
    if (g_pwd_input_len == 0){
        lv_obj_clear_flag(g_pwd_prompt_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_pwd_display_label, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief  弹窗确认按钮回调
 */
static void pwd_confirm_click_cb(lv_event_t *e)
{
    if (e == NULL) return;
    
    //common_member_info_t *member = get_current_enroll_member();
    lv_obj_t *parent_scr = (lv_obj_t *)lv_event_get_user_data(e);
    bool is_success = false;

    // 判断是否添加成功
    if (lv_obj_is_valid(g_pwd_dialog)) {
        lv_obj_t *dialog_label = lv_obj_get_child(g_pwd_dialog, 0);
        if (lv_obj_is_valid(dialog_label)) {
            const char *msg = lv_label_get_text(dialog_label);
            is_success = (msg && strcmp(msg, "add succeed") == 0);
        }
    }

    // 成功则完成注册
    if (is_success) {
        const char *pwd_name = "pwd01";
        if (lv_obj_is_valid(g_pwd_name_input)) {
            const char *input_name = lv_textarea_get_text(g_pwd_name_input);
            if (input_name && strlen(input_name) > 0) {
                pwd_name = input_name;
            }
        }
        pwd_enroll_complete(pwd_name);
    }

    // 返回注册页面
    //ui_enroll_create(member, parent_scr);
    lv_scr_load(parent_scr);
    // 重置数据和UI
    close_pwd_dialog();
    memset(g_pwd_first, 0, sizeof(g_pwd_first));
    memset(g_pwd_second, 0, sizeof(g_pwd_second));
    g_pwd_input_len = 0;
    g_is_confirm_mode = false;

    lv_label_set_text(g_pwd_display_label, "");
    lv_obj_add_flag(g_pwd_display_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(g_pwd_prompt_label, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(g_pwd_prompt_label, "Please input 6-digit password");

    // 销毁当前页面
    if (is_lv_obj_valid(pwd_add_scr)) {
        lv_obj_del(pwd_add_scr);
        pwd_add_scr = NULL;
    }
}

/**
 * @brief  密码名称输入框点击(弹出键盘)
 */
static void pwd_name_input_click_cb(lv_event_t *e)
{
    if (e == NULL) return;
    lv_obj_t *input = lv_event_get_target(e);
    
    // 创建软键盘
    if (!lv_obj_is_valid(g_pwd_keyboard)) {
        g_pwd_keyboard = lv_keyboard_create(lv_scr_act());
        lv_obj_set_style_bg_color(g_pwd_keyboard, lv_color_hex(0x192A46), LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(g_pwd_keyboard, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(g_pwd_keyboard, lv_color_hex(0x1F3150), LV_STATE_DEFAULT);
        lv_obj_set_style_radius(g_pwd_keyboard, 6, LV_STATE_DEFAULT);
        lv_obj_set_size(g_pwd_keyboard, LV_HOR_RES, KEYBOARD_HEIGHT);
        lv_obj_align(g_pwd_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_add_event_cb(g_pwd_keyboard, hide_pwd_keyboard, LV_EVENT_READY, NULL);
    }
    
    // 显示键盘并绑定输入框
    lv_obj_clear_flag(g_pwd_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(g_pwd_keyboard, input);
    lv_obj_move_foreground(g_pwd_keyboard);
}

/**
 * @brief  隐藏软键盘
 */
static void hide_pwd_keyboard(lv_event_t *e)
{
    (void)e;
    if (lv_obj_is_valid(g_pwd_keyboard)) {
        lv_obj_add_flag(g_pwd_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief  创建数字按键辅助函数
 */
static lv_obj_t *create_pwd_num_btn(lv_obj_t *parent, uint16_t x, uint16_t y, const char *num, lv_obj_t *enroll_scr)
{
    lv_obj_t *num_btn = create_container(parent, x, y, 150, 80,
                    lv_color_hex(0x7698D0), LV_OPA_70, 6, 
                    lv_color_hex(0x1F3150), 0, LV_OPA_90);
    
    lv_obj_t *num_label = create_text_label(num_btn, num, &lv_font_montserrat_48,
                    lv_color_hex(0x000000), 0, 0, LV_OPA_100);
    lv_obj_align(num_label, LV_ALIGN_CENTER, 0, 0);
    
    // 设置按钮样式
    lv_obj_add_flag(num_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(num_btn, LV_OPA_90, LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(num_btn, lv_color_hex(0xD9D9D9), LV_STATE_PRESSED);
    
    // 绑定数据和回调
    lv_obj_set_user_data(num_btn, (void*)num);
    lv_obj_add_event_cb(num_btn, pwd_num_click_cb, LV_EVENT_CLICKED, enroll_scr);
    
    return num_btn;
}