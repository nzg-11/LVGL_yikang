#ifndef MSG_CENTER_COM_H
#define MSG_CENTER_COM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "com.h"

//==================== 常量 ========================
#define TIMELINE_CIRCLE_BASE_X    47
#define TIMELINE_CIRCLE_RADIUS    15

#define VISITOR_REC_BASE_X        70
#define VISITOR_REC_BASE_Y        296
#define VISITOR_REC_SPACING       112
#define VISITOR_TIMELINE_BASE_Y   301
#define VISITOR_TIMELINE_SPACING  112

#define ALARM_BELL_TIMELINE_BASE_Y   301
#define ALARM_BELL_TIMELINE_SPACING  226
#define ALARM_BELL_REC_BASE_X        74
#define ALARM_BELL_REC_BASE_Y        297
#define ALARM_BELL_REC_SPACING       226
#define ALARM_BELL_CON_BASE_Y        361
#define ALARM_BELL_CON_SPACING       226

//==================== 结构体 ========================
typedef struct {
    lv_obj_t *lock_type_label;
    lv_obj_t *open_door_label;
    lv_obj_t *open_time_label;
} LockInfoLabels_t;

typedef struct {
    lv_obj_t *bell_msg_label;
    lv_obj_t *bell_time_label;
    lv_obj_t *bell_container;
} DoorbellInfoLabels_t;

//==================== 全局变量 extern 声明 ========================
extern LockInfoLabels_t        *g_visitor_labels;
extern lv_obj_t                **g_timeline_circles;
extern int                     g_visitor_count;
extern lv_obj_t                *g_timeline_line;

extern DoorbellInfoLabels_t    *g_alarm_labels;
extern lv_obj_t                **g_alarm_circles;
extern int                     g_alarm_count;
extern lv_obj_t                *g_alarm_line;

extern DoorbellInfoLabels_t    *g_doorbell_labels;
extern lv_obj_t                **g_doorbell_circles;
extern int                     g_doorbell_count;
extern lv_obj_t                *g_doorbell_line;

extern lv_obj_t *msg_center_scr;
extern lv_obj_t *g_selected_rec_con;
extern lv_obj_t *visitor_rec_con;
extern lv_obj_t *alarm_rec_con;
extern lv_obj_t *doorbell_rec_con;
extern lv_obj_t *date_picker;
extern lv_obj_t *visitor_data;

//==================== 函数声明 ========================
LockInfoLabels_t create_lock_info_group(lv_obj_t *parent, int base_x, int base_y,
                                        const char *init_lock_text, const char *init_time_text);

LockInfoLabels_t *add_visitor_record(lv_obj_t *parent, const char *lock_type, const char *open_time);
void clear_all_visitor_records(void);

DoorbellInfoLabels_t create_doorbell_info_group(lv_obj_t *parent, int base_x, int base_y, int con_y,
                                                const char *init_bell_text, const char *init_time_text);

DoorbellInfoLabels_t *add_doorbell_record(lv_obj_t *parent, const char *bell_text, const char *bell_time);
void clear_all_doorbell_records(void);

DoorbellInfoLabels_t create_alarm_info_group(lv_obj_t *parent, int base_x, int base_y, int con_y,
                                             const char *init_alarm_text, const char *init_time_text);

DoorbellInfoLabels_t *add_alarm_record(lv_obj_t *parent, const char *alarm_text, const char *alarm_time);
void clear_all_alarm_records(void);

void rec_con_click_cb(lv_event_t *e);
void switch_record_type(lv_obj_t *target_con);

#ifdef __cplusplus
}
#endif

#endif