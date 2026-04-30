/**
 * @file lv_calendar_obj_dropdown.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_calendar_header_dropdown.h"
#if LV_USE_CALENDAR_HEADER_DROPDOWN

#include "lv_calendar.h"
#include "../../../widgets/lv_dropdown.h"
#include "../../layouts/flex/lv_flex.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void my_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void year_event_cb(lv_event_t * e);
static void month_event_cb(lv_event_t * e);
static void value_changed_event_cb(lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lv_calendar_header_dropdown_class = {
    .base_class = &lv_obj_class,
    .width_def = LV_PCT(100),
    .height_def = LV_SIZE_CONTENT,
    .constructor_cb = my_constructor
};

static const char * month_list = "01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12";
static const char * year_list = {
    "2050\n2049\n2048\n2047\n2046\n2045\n2044\n2043\n2042\n2041\n"
    "2040\n2039\n2038\n2037\n2036\n2035\n2034\n2033\n2032\n2031\n"
    "2030\n2029\n2028\n2027\n2026\n2025\n2024\n2023\n2022\n2021\n"
    "2020\n2019\n2018\n2017\n2016\n2015\n2014\n2013\n2012\n2011\n2010\n2009\n2008\n2007\n2006\n2005\n2004\n2003\n2002\n2001\n"
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * lv_calendar_header_dropdown_create(lv_obj_t * parent)
{
    lv_obj_t * obj = lv_obj_class_create_obj(&lv_calendar_header_dropdown_class, parent);
    lv_obj_class_init_obj(obj);

    return obj;
}

void lv_calendar_header_dropdown_set_year_list(lv_obj_t * parent, const char * years_list)
{
    bool child_found = false;
    uint32_t idx = 0;
    lv_obj_t * child = NULL;

    const uint32_t calendar_child_count = lv_obj_get_child_cnt(parent);

    /* Search for the header dropdown */
    for(idx = 0; idx < calendar_child_count; idx++) {
        child = lv_obj_get_child(parent, idx);
        if(lv_obj_check_type(child, &lv_calendar_header_dropdown_class)) {
            child_found = true;
            break;
        }
    }

    if(!child_found) return;

    child_found = false;
    const uint32_t header_child_count = lv_obj_get_child_cnt(child);

    /* Search for the year dropdown */
    for(idx = 0; idx < header_child_count; idx++) {
        child = lv_obj_get_child(child, idx);
        if(lv_obj_check_type(child, &lv_dropdown_class)) {
            child_found = true;
            break;
        }
    }

    if(!child_found) return;

    lv_dropdown_clear_options(child);
    lv_dropdown_set_options(child, years_list);

    lv_obj_invalidate(parent);
}

/**********************
 *  STATIC FUNCTIONS
 **********************/

static void my_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_TRACE_OBJ_CREATE("begin");

    LV_UNUSED(class_p);

    lv_obj_t * calendar = lv_obj_get_parent(obj);
    lv_obj_move_to_index(obj, 0);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);

    lv_obj_t * year_dd = lv_dropdown_create(obj);
    lv_dropdown_set_options(year_dd, year_list);
    lv_obj_add_event_cb(year_dd, year_event_cb, LV_EVENT_VALUE_CHANGED, calendar);
    lv_obj_set_flex_grow(year_dd, 1);

    lv_obj_t * month_dd = lv_dropdown_create(obj);
    lv_dropdown_set_options(month_dd, month_list);
    lv_obj_add_event_cb(month_dd, month_event_cb, LV_EVENT_VALUE_CHANGED, calendar);
    lv_obj_set_flex_grow(month_dd, 1);

    lv_obj_add_event_cb(obj, value_changed_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    /*Refresh the drop downs*/
    lv_event_send(obj, LV_EVENT_VALUE_CHANGED, NULL);
}

static void month_event_cb(lv_event_t * e)
{
    lv_obj_t * dropdown = lv_event_get_target(e);
    lv_obj_t * calendar = lv_event_get_user_data(e);

    uint16_t sel = lv_dropdown_get_selected(dropdown);

    const lv_calendar_date_t * d;
    d = lv_calendar_get_showed_date(calendar);
    lv_calendar_date_t newd = *d;
    newd.month = sel + 1;

    lv_calendar_set_showed_date(calendar, newd.year, newd.month);
}

static void year_event_cb(lv_event_t * e)
{
    lv_obj_t * dropdown = lv_event_get_target(e);
    lv_obj_t * calendar = lv_event_get_user_data(e);

    uint16_t sel = lv_dropdown_get_selected(dropdown);

    const lv_calendar_date_t * d;
    d = lv_calendar_get_showed_date(calendar);
    lv_calendar_date_t newd = *d;

    /* Get the first year on the options list
     * NOTE: Assumes the first 4 digits in the option list are numbers */
    const char * year_p = lv_dropdown_get_options(dropdown);
    const uint32_t year = (year_p[0] - '0') * 1000 + (year_p[1] - '0') * 100 + (year_p[2] - '0') * 10 +
                          (year_p[3] - '0');

    newd.year = year - sel;

    lv_calendar_set_showed_date(calendar, newd.year, newd.month);
}

static void value_changed_event_cb(lv_event_t * e)
{
    lv_obj_t * header = lv_event_get_target(e);
    lv_obj_t * calendar = lv_obj_get_parent(header);
    const lv_calendar_date_t * cur_date = lv_calendar_get_showed_date(calendar);

    lv_obj_t * year_dd = lv_obj_get_child(header, 0);

    /* Get the first year on the options list
     * NOTE: Assumes the first 4 digits in the option list are numbers */
    const char * year_p = lv_dropdown_get_options(year_dd);
    const uint32_t year = (year_p[0] - '0') * 1000 + (year_p[1] - '0') * 100 + (year_p[2] - '0') * 10 +
                          (year_p[3] - '0');

    lv_dropdown_set_selected(year_dd, year - cur_date->year);

    lv_obj_t * month_dd = lv_obj_get_child(header, 1);
    lv_dropdown_set_selected(month_dd, cur_date->month - 1);
}

#endif /*LV_USE_CALENDAR_HEADER_ARROW*/

