#include "msg_center_com.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//==================== 全局变量定义 =====================
// 访客记录相关标签、时间轴圆圈、数量、时间轴线
LockInfoLabels_t     *g_visitor_labels   = NULL;    // 访客记录标签数组
lv_obj_t             **g_timeline_circles = NULL;  // 访客时间轴圆点数组
int                  g_visitor_count      = 0;     // 访客记录总数
lv_obj_t             *g_timeline_line     = NULL;    // 访客时间轴竖线

// 报警记录相关标签、报警圆圈、数量、报警线
DoorbellInfoLabels_t *g_alarm_labels      = NULL;    // 报警记录标签数组
lv_obj_t             **g_alarm_circles    = NULL;    // 报警时间轴圆点数组
int                  g_alarm_count        = 0;       // 报警记录总数
lv_obj_t             *g_alarm_line        = NULL;    // 报警时间轴竖线

// 门铃记录相关标签、门铃圆圈、数量、门铃线
DoorbellInfoLabels_t *g_doorbell_labels   = NULL;    // 门铃记录标签数组
lv_obj_t             **g_doorbell_circles = NULL;    // 门铃时间轴圆点数组
int                  g_doorbell_count     = 0;       // 门铃记录总数
lv_obj_t             *g_doorbell_line    = NULL;     // 门铃时间轴竖线

// 消息中心界面全局对象
lv_obj_t *msg_center_scr      = NULL;        // 消息中心主屏幕
lv_obj_t *g_selected_rec_con  = NULL;        // 当前选中的记录类型容器
lv_obj_t *visitor_rec_con     = NULL;        // 访客记录容器
lv_obj_t *alarm_rec_con       = NULL;        // 报警记录容器
lv_obj_t *doorbell_rec_con    = NULL;        // 门铃记录容器
lv_obj_t *date_picker         = NULL;        // 日期选择器
lv_obj_t *visitor_data        = NULL;        // 访客数据容器

//==================== 内部静态函数声明 =====================
// 更新访客时间轴竖线
static void update_timeline_line(lv_obj_t *parent);
// 更新门铃时间轴竖线
static void update_doorbell_line(lv_obj_t *parent);
// 更新报警时间轴竖线
static void update_alarm_line(lv_obj_t *parent);
// 重置所有记录类型容器的样式
static void reset_all_rec_con_style(void);
// 设置记录类型容器为选中样式
static void set_rec_con_selected(lv_obj_t *con);

//==================== 访客记录功能 =====================
/**
 @brief 创建一组访客信息标签（开锁类型、开门文字、时间）
 @param parent 父对象
 @param base_x X基准坐标
 @param base_y Y基准坐标
 @param init_lock_text 初始开锁方式文本
 @param init_time_text 初始时间文本
 @return 访客标签结构体
*/
LockInfoLabels_t create_lock_info_group(lv_obj_t *parent, int base_x, int base_y,
                                        const char *init_lock_text, const char *init_time_text)
{
    LockInfoLabels_t labels = {NULL};

    // 创建开锁类型标签（如指纹、密码、面容）
    labels.lock_type_label = create_text_label(
        parent, init_lock_text, &eques_regular_24,
        lv_color_hex(0x50FF9F), base_x, base_y, LV_OPA_100);

    // 创建“打开门”固定文字标签
    labels.open_door_label = create_text_label(
        parent, "打开门", &eques_regular_24,
        lv_color_hex(0xFFFFFF), 0, base_y, LV_OPA_100);
    lv_obj_align_to(labels.open_door_label, labels.lock_type_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    // 创建开门时间标签
    labels.open_time_label = create_text_label(
        parent, init_time_text, &eques_regular_24,
        lv_color_hex(0xB5B5B5), base_x + 2, base_y + 35, LV_OPA_100);

    return labels;
}

/**
 @brief 更新访客时间轴竖线（根据记录数量自动计算高度）
*/
static void update_timeline_line(lv_obj_t *parent)
{
    if(g_visitor_count == 0) return;

    // 计算时间线起始和结束Y坐标
    int start_y = VISITOR_TIMELINE_BASE_Y;
    int end_y   = VISITOR_TIMELINE_BASE_Y + (g_visitor_count-1)*VISITOR_TIMELINE_SPACING;

    // 设置线条两个端点坐标
    static lv_point_t line_points[] = {{55, 0}, {55, 0}};
    line_points[0].y = start_y;
    line_points[1].y = end_y;

    // 创建或更新时间轴线
    if(!g_timeline_line) {
        g_timeline_line = lv_line_create(parent);
    }
    config_divider_line_style(g_timeline_line, line_points, 2, 0xFFFFFF, 1, LV_OPA_100);
}

/**
 @brief 添加一条访客记录（动态分配内存 + 创建UI）
 @param parent 父对象
 @param lock_type 开锁类型字符串
 @param open_time 开门时间字符串
 @return 指向新增访客标签的指针
*/
LockInfoLabels_t *add_visitor_record(lv_obj_t *parent, const char *lock_type, const char *open_time)
{
    if(!parent || !lock_type || !open_time) return NULL;

    // 记录数+1，重新分配内存
    g_visitor_count++;
    g_visitor_labels   = realloc(g_visitor_labels, g_visitor_count * sizeof(LockInfoLabels_t));
    g_timeline_circles = realloc(g_timeline_circles, g_visitor_count * sizeof(lv_obj_t*));

    // 内存分配失败则回滚
    if(!g_visitor_labels || !g_timeline_circles) {
        g_visitor_count--;
        return NULL;
    }

    // 计算本条记录的Y坐标
    int rec_y    = VISITOR_REC_BASE_Y + (g_visitor_count-1)*VISITOR_REC_SPACING;
    int circle_y = VISITOR_TIMELINE_BASE_Y + (g_visitor_count-1)*VISITOR_TIMELINE_SPACING;

    // 创建记录标签和时间轴圆点
    g_visitor_labels[g_visitor_count-1] = create_lock_info_group(
        parent, VISITOR_REC_BASE_X, rec_y, lock_type, open_time);

    g_timeline_circles[g_visitor_count-1] = create_container_circle(
        parent, TIMELINE_CIRCLE_BASE_X, circle_y, TIMELINE_CIRCLE_RADIUS,
        true, lv_color_hex(0xFFFFFF), lv_color_hex(0x808080), 0, LV_OPA_0);

    // 更新时间轴竖线
    update_timeline_line(parent);
    return &g_visitor_labels[g_visitor_count-1];
}

/**
 @brief 清空所有访客记录，释放内存，删除UI
*/
void clear_all_visitor_records(void)
{
    // 逐个删除UI对象
    for(int i=0; i<g_visitor_count; i++) {
        if(g_timeline_circles[i]) lv_obj_del(g_timeline_circles[i]);
        if(g_visitor_labels[i].lock_type_label) lv_obj_del(g_visitor_labels[i].lock_type_label);
        if(g_visitor_labels[i].open_door_label) lv_obj_del(g_visitor_labels[i].open_door_label);
        if(g_visitor_labels[i].open_time_label) lv_obj_del(g_visitor_labels[i].open_time_label);
    }

    // 释放动态内存
    free(g_visitor_labels);
    free(g_timeline_circles);
    if(g_timeline_line) lv_obj_del(g_timeline_line);

    // 重置全局变量
    g_visitor_labels   = NULL;
    g_timeline_circles = NULL;
    g_timeline_line    = NULL;
    g_visitor_count    = 0;
}

//==================== 门铃记录功能 =====================
/**
 @brief 创建一组门铃信息标签（门铃提示、时间、背景容器）
*/
DoorbellInfoLabels_t create_doorbell_info_group(lv_obj_t *parent,
    int base_x, int base_y, int con_y, const char *init_bell_text, const char *init_time_text)
{
    DoorbellInfoLabels_t labels = {NULL};

    // 门铃提示文字标签
    labels.bell_msg_label = create_text_label(
        parent, init_bell_text, &eques_regular_24,
        lv_color_hex(0xFFCF52), base_x, base_y, LV_OPA_100);

    // 门铃时间标签
    labels.bell_time_label = create_text_label(
        parent, init_time_text, &eques_regular_24,
        lv_color_hex(0xB5B5B5), base_x, base_y + 32, LV_OPA_100);

    // 门铃背景容器
    labels.bell_container = create_container(
        parent, base_x, con_y, 170, 126,
        lv_color_hex(0xD9D9D9), LV_OPA_20, 6,
        lv_color_hex(0x2E4B7D), 0, LV_OPA_0);

    return labels;
}

/**
 @brief 更新门铃时间轴竖线
*/
static void update_doorbell_line(lv_obj_t *parent)
{
    if(g_doorbell_count == 0) return;

    int start_y = ALARM_BELL_TIMELINE_BASE_Y;
    int end_y   = ALARM_BELL_TIMELINE_BASE_Y + (g_doorbell_count-1)*ALARM_BELL_TIMELINE_SPACING;

    static lv_point_t line_points[] = {{55, 0}, {55, 0}};
    line_points[0].y = start_y;
    line_points[1].y = end_y;

    if(!g_doorbell_line) {
        g_doorbell_line = lv_line_create(parent);
    }
    config_divider_line_style(g_doorbell_line, line_points, 2, 0xFFFFFF, 1, LV_OPA_100);
}

/**
 @brief 添加一条门铃记录
*/
DoorbellInfoLabels_t *add_doorbell_record(lv_obj_t *parent, const char *bell_text, const char *bell_time)
{
    if(!parent || !bell_text || !bell_time) return NULL;

    g_doorbell_count++;
    g_doorbell_labels  = realloc(g_doorbell_labels, g_doorbell_count * sizeof(DoorbellInfoLabels_t));
    g_doorbell_circles = realloc(g_doorbell_circles, g_doorbell_count * sizeof(lv_obj_t*));

    if(!g_doorbell_labels || !g_doorbell_circles) {
        g_doorbell_count--;
        return NULL;
    }

    // 计算坐标
    int rec_y    = ALARM_BELL_REC_BASE_Y + (g_doorbell_count-1)*ALARM_BELL_REC_SPACING;
    int con_y    = ALARM_BELL_CON_BASE_Y + (g_doorbell_count-1)*ALARM_BELL_CON_SPACING;
    int circle_y = ALARM_BELL_TIMELINE_BASE_Y + (g_doorbell_count-1)*ALARM_BELL_TIMELINE_SPACING;

    // 创建UI
    g_doorbell_labels[g_doorbell_count-1] = create_doorbell_info_group(
        parent, ALARM_BELL_REC_BASE_X, rec_y, con_y, bell_text, bell_time);

    g_doorbell_circles[g_doorbell_count-1] = create_container_circle(
        parent, TIMELINE_CIRCLE_BASE_X, circle_y, TIMELINE_CIRCLE_RADIUS,
        true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 0, LV_OPA_0);

    update_doorbell_line(parent);
    return &g_doorbell_labels[g_doorbell_count-1];
}

/**
 @brief 清空所有门铃记录
*/
void clear_all_doorbell_records(void)
{
    for(int i=0; i<g_doorbell_count; i++) {
        if(g_doorbell_circles[i]) lv_obj_del(g_doorbell_circles[i]);
        if(g_doorbell_labels[i].bell_msg_label) lv_obj_del(g_doorbell_labels[i].bell_msg_label);
        if(g_doorbell_labels[i].bell_time_label) lv_obj_del(g_doorbell_labels[i].bell_time_label);
        if(g_doorbell_labels[i].bell_container) lv_obj_del(g_doorbell_labels[i].bell_container);
    }

    free(g_doorbell_labels);
    free(g_doorbell_circles);
    if(g_doorbell_line) lv_obj_del(g_doorbell_line);

    g_doorbell_labels   = NULL;
    g_doorbell_circles = NULL;
    g_doorbell_line    = NULL;
    g_doorbell_count   = 0;
}

//==================== 报警记录功能 =====================
/**
 @brief 创建一组报警信息标签（报警提示、时间、背景容器）
*/
DoorbellInfoLabels_t create_alarm_info_group(lv_obj_t *parent,
    int base_x, int base_y, int con_y, const char *init_alarm_text, const char *init_time_text)
{
    DoorbellInfoLabels_t labels = {NULL};

    // 报警提示文字
    labels.bell_msg_label = create_text_label(
        parent, init_alarm_text, &eques_regular_24,
        lv_color_hex(0xFF5252), base_x, base_y, LV_OPA_100);

    // 报警时间
    labels.bell_time_label = create_text_label(
        parent, init_time_text, &eques_regular_24,
        lv_color_hex(0xB5B5B5), base_x, base_y + 32, LV_OPA_100);

    // 报警背景容器
    labels.bell_container = create_container(
        parent, base_x, con_y, 170, 126,
        lv_color_hex(0xD9D9D9), LV_OPA_20, 6,
        lv_color_hex(0x2E4B7D), 0, LV_OPA_0);

    return labels;
}

/**
 @brief 更新报警时间轴竖线
*/
static void update_alarm_line(lv_obj_t *parent)
{
    if(g_alarm_count == 0) return;

    int start_y = ALARM_BELL_TIMELINE_BASE_Y;
    int end_y   = ALARM_BELL_TIMELINE_BASE_Y + (g_alarm_count-1)*ALARM_BELL_TIMELINE_SPACING;

    static lv_point_t line_points[] = {{55, 0}, {55, 0}};
    line_points[0].y = start_y;
    line_points[1].y = end_y;

    if(!g_alarm_line) {
        g_alarm_line = lv_line_create(parent);
    }
    config_divider_line_style(g_alarm_line, line_points, 2, 0xFFFFFF, 1, LV_OPA_100);
}

/**
 @brief 添加一条报警记录
*/
DoorbellInfoLabels_t *add_alarm_record(lv_obj_t *parent, const char *alarm_text, const char *alarm_time)
{
    if(!parent || !alarm_text || !alarm_time) return NULL;

    g_alarm_count++;
    g_alarm_labels   = realloc(g_alarm_labels, g_alarm_count * sizeof(DoorbellInfoLabels_t));
    g_alarm_circles  = realloc(g_alarm_circles, g_alarm_count * sizeof(lv_obj_t*));

    if(!g_alarm_labels || !g_alarm_circles) {
        g_alarm_count--;
        return NULL;
    }

    // 计算坐标
    int rec_y    = ALARM_BELL_REC_BASE_Y + (g_alarm_count-1)*ALARM_BELL_REC_SPACING;
    int con_y    = ALARM_BELL_CON_BASE_Y + (g_alarm_count-1)*ALARM_BELL_CON_SPACING;
    int circle_y = ALARM_BELL_TIMELINE_BASE_Y + (g_alarm_count-1)*ALARM_BELL_TIMELINE_SPACING;

    // 创建UI
    g_alarm_labels[g_alarm_count-1] = create_alarm_info_group(
        parent, ALARM_BELL_REC_BASE_X, rec_y, con_y, alarm_text, alarm_time);

    g_alarm_circles[g_alarm_count-1] = create_container_circle(
        parent, TIMELINE_CIRCLE_BASE_X, circle_y, TIMELINE_CIRCLE_RADIUS,
        true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 0, LV_OPA_0);

    update_alarm_line(parent);
    return &g_alarm_labels[g_alarm_count-1];
}

/**
 @brief 清空所有报警记录
*/
void clear_all_alarm_records(void)
{
    for(int i=0; i<g_alarm_count; i++) {
        if(g_alarm_circles[i]) lv_obj_del(g_alarm_circles[i]);
        if(g_alarm_labels[i].bell_msg_label) lv_obj_del(g_alarm_labels[i].bell_msg_label);
        if(g_alarm_labels[i].bell_time_label) lv_obj_del(g_alarm_labels[i].bell_time_label);
        if(g_alarm_labels[i].bell_container) lv_obj_del(g_alarm_labels[i].bell_container);
    }

    free(g_alarm_labels);
    free(g_alarm_circles);
    if(g_alarm_line) lv_obj_del(g_alarm_line);

    g_alarm_labels    = NULL;
    g_alarm_circles   = NULL;
    g_alarm_line      = NULL;
    g_alarm_count     = 0;
}

//==================== 记录类型切换功能 =====================
/**
 @brief 重置所有记录类型按钮为默认样式（未选中）
*/
static void reset_all_rec_con_style(void)
{
    if(visitor_rec_con) {
        lv_obj_set_style_bg_grad_dir(visitor_rec_con, LV_GRAD_DIR_NONE, 0);
        lv_obj_set_style_bg_color(visitor_rec_con, lv_color_hex(0x1F3150), 0);
    }
    if(alarm_rec_con) {
        lv_obj_set_style_bg_grad_dir(alarm_rec_con, LV_GRAD_DIR_NONE, 0);
        lv_obj_set_style_bg_color(alarm_rec_con, lv_color_hex(0x1F3150), 0);
    }
    if(doorbell_rec_con) {
        lv_obj_set_style_bg_grad_dir(doorbell_rec_con, LV_GRAD_DIR_NONE, 0);
        lv_obj_set_style_bg_color(doorbell_rec_con, lv_color_hex(0x1F3150), 0);
    }
}

/**
 @brief 设置目标记录类型按钮为选中样式（渐变蓝）
*/
static void set_rec_con_selected(lv_obj_t *con)
{
    if(!con) return;
    lv_obj_set_style_bg_grad_dir(con, LV_GRAD_DIR_HOR, 0);
    lv_obj_set_style_bg_color(con, lv_color_hex(0x006BDC), 0);
    lv_obj_set_style_bg_grad_color(con, lv_color_hex(0x00BDBD), 0);
    g_selected_rec_con = con;
}

/**
 @brief 切换显示的记录类型（访客/报警/门铃）
 先清空所有记录，再根据目标类型添加默认演示数据
*/
void switch_record_type(lv_obj_t *target_con)
{
    // 清空所有界面记录
    clear_all_visitor_records();
    clear_all_alarm_records();
    clear_all_doorbell_records();

    // 根据选中类型加载对应演示数据
    if(target_con == visitor_rec_con) {
        add_visitor_record(msg_center_scr, "[指纹01]", "13:39:20");
        add_visitor_record(msg_center_scr, "[面容01]", "12:39:20");
        add_visitor_record(msg_center_scr, "[密码01]", "08:39:20");
        add_visitor_record(msg_center_scr, "[密码02]", "08:39:20");
    } else if(target_con == alarm_rec_con) {
        add_alarm_record(msg_center_scr, "陌生人逗留", "13:39:20");
        add_alarm_record(msg_center_scr, "陌生人逗留",   "10:15:30");
    } else if(target_con == doorbell_rec_con) {
        add_doorbell_record(msg_center_scr, "有人按门铃", "13:39:20");
        add_doorbell_record(msg_center_scr, "有人按门铃", "10:25:15");
    }
}

/**
 @brief 记录类型按钮点击回调函数
 处理选中状态切换与界面刷新
*/
void rec_con_click_cb(lv_event_t *e)
{
    lv_obj_t *clicked = lv_event_get_target(e);
    // 重复点击当前选中项，不处理
    if(clicked == g_selected_rec_con) return;

    // 重置样式 → 设置新选中 → 切换记录显示
    reset_all_rec_con_style();
    set_rec_con_selected(clicked);
    switch_record_type(clicked);
}