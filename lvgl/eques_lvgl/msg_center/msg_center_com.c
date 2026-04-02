#include "msg_center_com.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//==================== 全局变量 定义（只在这里定义一次） =====================
LockInfoLabels_t     *g_visitor_labels   = NULL;
lv_obj_t             **g_timeline_circles = NULL;
int                  g_visitor_count      = 0;
lv_obj_t             *g_timeline_line     = NULL;

DoorbellInfoLabels_t *g_alarm_labels      = NULL;
lv_obj_t             **g_alarm_circles    = NULL;
int                  g_alarm_count        = 0;
lv_obj_t             *g_alarm_line        = NULL;

DoorbellInfoLabels_t *g_doorbell_labels   = NULL;
lv_obj_t             **g_doorbell_circles = NULL;
int                  g_doorbell_count     = 0;
lv_obj_t             *g_doorbell_line     = NULL;

lv_obj_t *msg_center_scr      = NULL;
lv_obj_t *g_selected_rec_con  = NULL;
lv_obj_t *visitor_rec_con     = NULL;
lv_obj_t *alarm_rec_con       = NULL;
lv_obj_t *doorbell_rec_con    = NULL;
lv_obj_t *date_picker         = NULL;
lv_obj_t *visitor_data        = NULL;

//==================== 内部静态函数（只在本文件可见） =====================
static void update_timeline_line(lv_obj_t *parent);
static void update_doorbell_line(lv_obj_t *parent);
static void update_alarm_line(lv_obj_t *parent);
static void reset_all_rec_con_style(void);
static void set_rec_con_selected(lv_obj_t *con);

//==================== 访客 =====================
LockInfoLabels_t create_lock_info_group(lv_obj_t *parent, int base_x, int base_y,
                                        const char *init_lock_text, const char *init_time_text)
{
    LockInfoLabels_t labels = {NULL};

    labels.lock_type_label = create_text_label(
        parent, init_lock_text, &lv_font_montserrat_24,
        lv_color_hex(0x50FF9F), base_x, base_y, LV_OPA_100);

    labels.open_door_label = create_text_label(
        parent, "open door", &lv_font_montserrat_24,
        lv_color_hex(0xFFFFFF), base_x + 80, base_y, LV_OPA_100);

    labels.open_time_label = create_text_label(
        parent, init_time_text, &lv_font_montserrat_18,
        lv_color_hex(0xB5B5B5), base_x + 2, base_y + 35, LV_OPA_100);

    return labels;
}

static void update_timeline_line(lv_obj_t *parent)
{
    if(g_visitor_count == 0) return;

    int start_y = VISITOR_TIMELINE_BASE_Y;
    int end_y   = VISITOR_TIMELINE_BASE_Y + (g_visitor_count-1)*VISITOR_TIMELINE_SPACING;

    static lv_point_t line_points[] = {{55, 0}, {55, 0}};
    line_points[0].y = start_y;
    line_points[1].y = end_y;

    if(!g_timeline_line) {
        g_timeline_line = lv_line_create(parent);
    }
    config_divider_line_style(g_timeline_line, line_points, 2, 0xFFFFFF, 1, LV_OPA_100);
}

LockInfoLabels_t *add_visitor_record(lv_obj_t *parent, const char *lock_type, const char *open_time)
{
    if(!parent || !lock_type || !open_time) return NULL;

    g_visitor_count++;
    g_visitor_labels   = realloc(g_visitor_labels, g_visitor_count * sizeof(LockInfoLabels_t));
    g_timeline_circles = realloc(g_timeline_circles, g_visitor_count * sizeof(lv_obj_t*));

    if(!g_visitor_labels || !g_timeline_circles) {
        g_visitor_count--;
        return NULL;
    }

    int rec_y    = VISITOR_REC_BASE_Y + (g_visitor_count-1)*VISITOR_REC_SPACING;
    int circle_y = VISITOR_TIMELINE_BASE_Y + (g_visitor_count-1)*VISITOR_TIMELINE_SPACING;

    g_visitor_labels[g_visitor_count-1] = create_lock_info_group(
        parent, VISITOR_REC_BASE_X, rec_y, lock_type, open_time);

    g_timeline_circles[g_visitor_count-1] = create_container_circle(
        parent, TIMELINE_CIRCLE_BASE_X, circle_y, TIMELINE_CIRCLE_RADIUS,
        true, lv_color_hex(0xFFFFFF), lv_color_hex(0x808080), 0, LV_OPA_0);

    update_timeline_line(parent);
    return &g_visitor_labels[g_visitor_count-1];
}

void clear_all_visitor_records(void)
{
    for(int i=0; i<g_visitor_count; i++) {
        if(g_timeline_circles[i]) lv_obj_del(g_timeline_circles[i]);
        if(g_visitor_labels[i].lock_type_label) lv_obj_del(g_visitor_labels[i].lock_type_label);
        if(g_visitor_labels[i].open_door_label) lv_obj_del(g_visitor_labels[i].open_door_label);
        if(g_visitor_labels[i].open_time_label) lv_obj_del(g_visitor_labels[i].open_time_label);
    }

    free(g_visitor_labels);
    free(g_timeline_circles);
    if(g_timeline_line) lv_obj_del(g_timeline_line);

    g_visitor_labels   = NULL;
    g_timeline_circles = NULL;
    g_timeline_line    = NULL;
    g_visitor_count    = 0;
}

//==================== 门铃 =====================
DoorbellInfoLabels_t create_doorbell_info_group(lv_obj_t *parent,
    int base_x, int base_y, int con_y, const char *init_bell_text, const char *init_time_text)
{
    DoorbellInfoLabels_t labels = {NULL};

    labels.bell_msg_label = create_text_label(
        parent, init_bell_text, &lv_font_montserrat_24,
        lv_color_hex(0xFFCF52), base_x, base_y, LV_OPA_100);

    labels.bell_time_label = create_text_label(
        parent, init_time_text, &lv_font_montserrat_20,
        lv_color_hex(0xB5B5B5), base_x, base_y + 32, LV_OPA_100);

    labels.bell_container = create_container(
        parent, base_x, con_y, 170, 126,
        lv_color_hex(0xD9D9D9), LV_OPA_20, 6,
        lv_color_hex(0x2E4B7D), 0, LV_OPA_0);

    return labels;
}

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

    int rec_y    = ALARM_BELL_REC_BASE_Y + (g_doorbell_count-1)*ALARM_BELL_REC_SPACING;
    int con_y    = ALARM_BELL_CON_BASE_Y + (g_doorbell_count-1)*ALARM_BELL_CON_SPACING;
    int circle_y = ALARM_BELL_TIMELINE_BASE_Y + (g_doorbell_count-1)*ALARM_BELL_TIMELINE_SPACING;

    g_doorbell_labels[g_doorbell_count-1] = create_doorbell_info_group(
        parent, ALARM_BELL_REC_BASE_X, rec_y, con_y, bell_text, bell_time);

    g_doorbell_circles[g_doorbell_count-1] = create_container_circle(
        parent, TIMELINE_CIRCLE_BASE_X, circle_y, TIMELINE_CIRCLE_RADIUS,
        true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 0, LV_OPA_0);

    update_doorbell_line(parent);
    return &g_doorbell_labels[g_doorbell_count-1];
}

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

//==================== 报警 =====================
DoorbellInfoLabels_t create_alarm_info_group(lv_obj_t *parent,
    int base_x, int base_y, int con_y, const char *init_alarm_text, const char *init_time_text)
{
    DoorbellInfoLabels_t labels = {NULL};

    labels.bell_msg_label = create_text_label(
        parent, init_alarm_text, &lv_font_montserrat_24,
        lv_color_hex(0xFF5252), base_x, base_y, LV_OPA_100);

    labels.bell_time_label = create_text_label(
        parent, init_time_text, &lv_font_montserrat_20,
        lv_color_hex(0xB5B5B5), base_x, base_y + 32, LV_OPA_100);

    labels.bell_container = create_container(
        parent, base_x, con_y, 170, 126,
        lv_color_hex(0xD9D9D9), LV_OPA_20, 6,
        lv_color_hex(0x2E4B7D), 0, LV_OPA_0);

    return labels;
}

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

    int rec_y    = ALARM_BELL_REC_BASE_Y + (g_alarm_count-1)*ALARM_BELL_REC_SPACING;
    int con_y    = ALARM_BELL_CON_BASE_Y + (g_alarm_count-1)*ALARM_BELL_CON_SPACING;
    int circle_y = ALARM_BELL_TIMELINE_BASE_Y + (g_alarm_count-1)*ALARM_BELL_TIMELINE_SPACING;

    g_alarm_labels[g_alarm_count-1] = create_alarm_info_group(
        parent, ALARM_BELL_REC_BASE_X, rec_y, con_y, alarm_text, alarm_time);

    g_alarm_circles[g_alarm_count-1] = create_container_circle(
        parent, TIMELINE_CIRCLE_BASE_X, circle_y, TIMELINE_CIRCLE_RADIUS,
        true, lv_color_hex(0xFFFFFF), lv_color_hex(0xFFFFFF), 0, LV_OPA_0);

    update_alarm_line(parent);
    return &g_alarm_labels[g_alarm_count-1];
}

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

//==================== 切换记录 =====================
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

static void set_rec_con_selected(lv_obj_t *con)
{
    if(!con) return;
    lv_obj_set_style_bg_grad_dir(con, LV_GRAD_DIR_HOR, 0);
    lv_obj_set_style_bg_color(con, lv_color_hex(0x006BDC), 0);
    lv_obj_set_style_bg_grad_color(con, lv_color_hex(0x00BDBD), 0);
    g_selected_rec_con = con;
}

void switch_record_type(lv_obj_t *target_con)
{
    clear_all_visitor_records();
    clear_all_alarm_records();
    clear_all_doorbell_records();

    if(target_con == visitor_rec_con) {
        add_visitor_record(msg_center_scr, "[指纹01]", "13:39:20");
        add_visitor_record(msg_center_scr, "[密码01]", "12:39:20");
        add_visitor_record(msg_center_scr, "[面容01]", "08:39:20");
    } else if(target_con == alarm_rec_con) {
        add_alarm_record(msg_center_scr, "陌生人逗留", "13:39:20");
        add_alarm_record(msg_center_scr, "门锁异常",   "10:15:30");
    } else if(target_con == doorbell_rec_con) {
        add_doorbell_record(msg_center_scr, "有人按门铃", "13:39:20");
        add_doorbell_record(msg_center_scr, "有人按门铃", "10:25:15");
    }
}

void rec_con_click_cb(lv_event_t *e)
{
    lv_obj_t *clicked = lv_event_get_target(e);
    if(clicked == g_selected_rec_con) return;

    reset_all_rec_con_style();
    set_rec_con_selected(clicked);
    switch_record_type(clicked);
}