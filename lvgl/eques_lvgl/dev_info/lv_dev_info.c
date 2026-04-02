// #include "lv_dev_info.h"


// static lv_obj_t *dev_info_scr = NULL; 

// // 全局样式
// static lv_style_t dev_info_grad_style;
// static bool dev_info_style_inited = false;

// // 全局样式初始化
// static void init_msg_center_styles(void)
// {
//     if(!dev_info_style_inited) {
//         lv_style_init(&dev_info_grad_style);
//         dev_info_style_inited = true;
//     }
// }



// void ui_dev_info_create(lv_obj_t *homepage_scr)
// {
//     init_msg_center_styles();
//     // 1. 安全校验：如果传进来的 homepage_scr 为空，直接返回
//     if(homepage_scr == NULL) {
//         LV_LOG_WARN("ui_dev_info_create: homepage_scr is NULL!");
//         return;
//     }

//     // 2. 创建/复用设置屏幕对象
//     if(dev_info_scr == NULL) {
//         dev_info_scr = lv_obj_create(NULL);  // 创建独立屏幕
//     } else {
//         lv_obj_clean(dev_info_scr);          // 清空原有内容
//     }
//         // 重置样式（替代重复init）
//     lv_style_reset(&dev_info_grad_style);
//     lv_style_set_bg_color(&dev_info_grad_style, lv_color_hex(0x010715));// 渐变主色：#010715（0%）
//     lv_style_set_bg_grad_color(&dev_info_grad_style, lv_color_hex(0x0E1D37));// 渐变副色：#0E1D37（100%）
//     lv_style_set_bg_grad_dir(&dev_info_grad_style, LV_GRAD_DIR_VER);// 渐变方向：垂直
//     lv_style_set_bg_main_stop(&dev_info_grad_style, 0);// 渐变范围：0~255
//     lv_style_set_bg_grad_stop(&dev_info_grad_style, 255);
//     lv_obj_add_style(dev_info_scr, &dev_info_grad_style, LV_STATE_DEFAULT);// 应用渐变样式到屏幕

//     // 左上角返回按钮
//     lv_obj_t *back_btn = create_image_obj(dev_info_scr, "H:back.png", 52, 123);
//     lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
//     lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);
//     lv_obj_add_event_cb(back_btn, back_btn_click_cb, LV_EVENT_CLICKED, homepage_scr);

//     //更新状态条父对象
//     update_status_bar_parent(dev_info_scr);
//     // 切换到设置屏幕
//     lv_scr_load(dev_info_scr);
// }





// /***********************设备信息界面回调*********************/
// void dev_info_btn_click_cb(lv_event_t *e)
// {
//     if(e == NULL) return;
    
//     lv_obj_t *homepage_scr = (lv_obj_t *)lv_event_get_user_data(e);
//     if(homepage_scr == NULL) {
//         LV_LOG_WARN("dev_info_btn_click_cb: homepage_scr is NULL!");
//         return;
//     }
//     ui_dev_info_create(homepage_scr);

// }

#include "lv_dev_info.h"

static lv_obj_t *dev_info_scr = NULL; 
static lv_obj_t *dev_name_label = NULL; // 保存设备昵称标签，用于更新

// 全局样式
static lv_style_t dev_info_grad_style;
static bool dev_info_style_inited = false;

// 全局样式初始化
static void init_msg_center_styles(void)
{
    if(!dev_info_style_inited) {
        lv_style_init(&dev_info_grad_style);
        dev_info_style_inited = true;
    }
}

/***********************确认修改昵称回调*********************/
void dev_info_name_confirm_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    lv_obj_t *mask = (lv_obj_t *)lv_event_get_user_data(e);
    if(mask == NULL) {
        LV_LOG_WARN("dev_info_name_confirm_cb: mask is NULL!");
        return;
    }

    // 找到输入框：mask的第4个子对象 (edit_dialog(0), title(1), hint(2), hint2(3), input(4), confirm_btn(5))
    lv_obj_t *input = lv_obj_get_child(mask, 4); 
    if(input && dev_name_label) {
        const char *new_name = lv_textarea_get_text(input);
        if(new_name && strlen(new_name) > 0) {
            lv_label_set_text(dev_name_label, new_name);
        }
    }

    // 销毁遮罩层和所有子对象
    lv_obj_del(mask);
}

/***********************创建遮罩层（用于昵称编辑弹窗）*********************/
void dev_info_name_edit_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    lv_obj_t *scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(scr == NULL) {
        LV_LOG_WARN("dev_info_name_edit_cb: scr is NULL!");
        return;
    }
    
    // 1. 创建遮罩层（父对象：scr）
    lv_obj_t *mask = lv_obj_create(scr);
    lv_obj_set_size(mask, lv_obj_get_width(scr), lv_obj_get_height(scr));
    lv_obj_set_style_radius(mask, 0, 0);
    lv_obj_set_style_bg_color(mask, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(mask, LV_OPA_50, 0);
    lv_obj_set_pos(mask, 0, 0); // 屏幕坐标 (100, 455)
    lv_obj_move_foreground(mask);
    lv_obj_set_style_border_opa(mask, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_flag(mask, LV_OBJ_FLAG_CLICKABLE); // 避免点击穿透
    lv_obj_set_style_pad_all(mask, 0, LV_STATE_DEFAULT);// 移除内边距
    lv_obj_clear_flag(mask, LV_OBJ_FLAG_SCROLLABLE);// 移除滚动功能

    // 2. 创建修改昵称弹窗（父对象：mask，而非scr）
    lv_obj_t *edit_dialog = lv_obj_create(mask);
    lv_obj_set_size(edit_dialog, 600, 297);//占满mask
    lv_obj_set_pos(edit_dialog, 100, 455);
    lv_obj_set_style_radius(edit_dialog, 16, 0);
    lv_obj_set_style_bg_color(edit_dialog, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_color(edit_dialog, lv_color_hex(0x2E4B7D), 0);
    lv_obj_set_style_border_width(edit_dialog, 0, 0);
    lv_obj_set_style_border_opa(edit_dialog, LV_OPA_100, 0);

    // 3. 添加标题（父对象：mask，坐标相对mask）
    // 原屏幕坐标 (300, 493) -> 相对mask坐标 (300-100, 493-455) = (200, 38)
    lv_obj_t *title = create_text_label(mask, "edit_title", &lv_font_montserrat_24, lv_color_hex(0x000000), 352, 493, LV_OPA_100);
    if(title) {
        lv_label_set_text(title, "change_nickname");
    }
    
    // 4. 添加提示文本（父对象：mask，坐标相对mask）
    // 原屏幕坐标 (250, 531) -> 相对mask坐标 (250-100, 531-455) = (150, 76)
    lv_obj_t *hint = create_text_label(mask, "edit_hint", &lv_font_montserrat_16, lv_color_hex(0x666666), 275, 531, LV_OPA_70);
    if(hint) {
        lv_label_set_text(hint, "character_not_exceed_8_no_special");
    }
    
    // 5. 昵称标签（父对象：mask，坐标相对mask）
    // 原屏幕坐标 (181, 575) -> 相对mask坐标 (181-100, 575-455) = (81, 120)
    lv_obj_t *hint2 = create_text_label(mask, "name", &lv_font_montserrat_24, lv_color_hex(0x666666), 181, 575, LV_OPA_80);
    if(hint2) {
        lv_label_set_text(hint2, "nickname");
    }

    // 6. 添加输入框（父对象：mask，坐标相对mask）
    // 原屏幕坐标 (236, 570) -> 相对mask坐标 (236-100, 570-455) = (136, 115)
    lv_obj_t *input = lv_textarea_create(mask);
    if(input) {
        lv_obj_set_size(input, 382, 44);
        lv_obj_set_pos(input, 236, 570);
        lv_textarea_set_placeholder_text(input, "change_nickname_placeholder");
        lv_textarea_set_text(input, "dev_name");
        lv_obj_clear_flag(input, LV_OBJ_FLAG_SCROLLABLE); 
        lv_textarea_set_max_length(input, 8);
        lv_obj_set_style_text_font(input, &lv_font_montserrat_24, 0);
    }
    
    // 7. 添加确认按钮（父对象：mask，坐标相对mask）
    // 原屏幕坐标 (296, 651) -> 相对mask坐标 (296-100, 651-455) = (196, 196)
    lv_obj_t *confirm_btn = lv_btn_create(mask);
    if(confirm_btn) {
        lv_obj_set_size(confirm_btn, 205, 44);
        lv_obj_set_pos(confirm_btn, 296, 651);
        // 按钮渐变样式
        lv_obj_set_style_bg_color(confirm_btn, lv_color_hex(0x006BDC), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_color(confirm_btn, lv_color_hex(0x00BDBD), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_dir(confirm_btn, LV_GRAD_DIR_HOR, LV_STATE_DEFAULT);
        lv_obj_set_style_bg_main_stop(confirm_btn, 0, LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_stop(confirm_btn, 255, LV_STATE_DEFAULT);
        // 按下态样式
        lv_obj_set_style_bg_color(confirm_btn, lv_color_hex(0x192A46), LV_STATE_PRESSED);
        lv_obj_set_style_radius(confirm_btn, 6, 0);
        
        // 8. 按钮文本（父对象：confirm_btn，坐标相对按钮）
        lv_obj_t *confirm_label = create_text_label(confirm_btn, "confirm_label", &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 44, 0, LV_OPA_100);
        if(confirm_label) {
            lv_label_set_text(confirm_label, "button");
        }
        
        // 绑定点击事件
        lv_obj_add_event_cb(confirm_btn, dev_info_name_confirm_cb, LV_EVENT_CLICKED, mask);
    }
}

void ui_dev_info_create(lv_obj_t *homepage_scr)
{
    init_msg_center_styles();
    if(homepage_scr == NULL) {
        LV_LOG_WARN("ui_dev_info_create: homepage_scr is NULL!");
        return;
    }

    if(dev_info_scr == NULL) {
        dev_info_scr = lv_obj_create(NULL);
    } else {
        lv_obj_clean(dev_info_scr);
    }
    
    // 重置渐变样式
    lv_style_reset(&dev_info_grad_style);
    lv_style_set_bg_color(&dev_info_grad_style, lv_color_hex(0x010715));
    lv_style_set_bg_grad_color(&dev_info_grad_style, lv_color_hex(0x0E1D37));
    lv_style_set_bg_grad_dir(&dev_info_grad_style, LV_GRAD_DIR_VER);
    lv_style_set_bg_main_stop(&dev_info_grad_style, 0);
    lv_style_set_bg_grad_stop(&dev_info_grad_style, 255);
    lv_obj_add_style(dev_info_scr, &dev_info_grad_style, LV_STATE_DEFAULT);

    // 添加标题
    lv_obj_t *dev_info_label = create_text_label(dev_info_scr, "dev_info_label", &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 328, 115, LV_OPA_100);
    
    // 设备昵称容器
    lv_obj_t *dev_con1 = create_container
    (dev_info_scr, 47, 195, 720, 83, lv_color_hex(0x192A46), LV_OPA_100, 16, lv_color_hex(0x2E4B7D), 0, LV_OPA_0);
    if(dev_con1) {
        lv_obj_add_flag(dev_con1, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(dev_con1, LV_OPA_70, LV_STATE_PRESSED);
        lv_obj_add_event_cb(dev_con1, dev_info_name_edit_cb, LV_EVENT_CLICKED, dev_info_scr);
    }

    // 设备昵称标签
    lv_obj_t *dev_lable1 = create_text_label(dev_info_scr, "dev_name", &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 134, 214, LV_OPA_100);
    dev_name_label = create_text_label(dev_info_scr, "dev_name_xxx", &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 585, 221, LV_OPA_50);
    if(dev_name_label) {
        lv_label_set_text(dev_name_label, "dev_name");
    }

    // 图片对象（添加判空）
    lv_obj_t *biaoqian_img = create_image_obj(dev_info_scr, "D:biaoqian.png", 66, 215);
    if(!biaoqian_img) LV_LOG_WARN("biaoqian.png load failed!");

    
    lv_obj_t *Vector_img = create_image_obj(dev_info_scr, "D:Vector.png", 712, 221);
    if(!Vector_img) LV_LOG_WARN("Vector.png load failed!");

    // 设备编号容器
    lv_obj_t *dev_con2 = create_container(dev_info_scr, 47, 282, 720, 83, lv_color_hex(0x192A46), LV_OPA_100, 16, lv_color_hex(0x2E4B7D), 0, LV_OPA_0);
    lv_obj_t *dev_lable2 = create_text_label(dev_info_scr, "dev_num", &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 134, 301, LV_OPA_100);
    lv_obj_t *dev_neirong_lable2 = create_text_label(dev_info_scr, "123456", &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 592, 308, LV_OPA_50);
    lv_obj_t *dev_num = create_image_obj(dev_info_scr, "D:dev_num.png", 66, 295);

    // WiFi信息容器
    lv_obj_t *dev_con3 = create_container(dev_info_scr, 47, 369, 720, 83, lv_color_hex(0x192A46), LV_OPA_100, 16, lv_color_hex(0x2E4B7D), 0, LV_OPA_0);
    lv_obj_t *dev_lable3 = create_text_label(dev_info_scr, "cru_wifi", &lv_font_montserrat_36, lv_color_hex(0xFFFFFF), 134, 388, LV_OPA_100);
    lv_obj_t *dev_neirong_lable3 = create_text_label(dev_info_scr, "wifi_name", &lv_font_montserrat_24, lv_color_hex(0xFFFFFF), 453, 395, LV_OPA_50);
    lv_obj_t *wangluo_img = create_image_obj(dev_info_scr, "D:wangluo.png", 66, 379);
    lv_obj_t *wifi_img = create_image_obj(dev_info_scr, "D:wifi.png", 671, 398);
    if(!wifi_img) LV_LOG_WARN("wifi.png load failed!");

    // 左上角返回按钮
    lv_obj_t *back_btn = create_image_obj(dev_info_scr, "D:back.png", 52, 123);
    if(back_btn) {
        lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_opa(back_btn, LV_OPA_80, LV_STATE_PRESSED);
        lv_obj_add_event_cb(back_btn, back_btn_click_cb, LV_EVENT_CLICKED, homepage_scr);
    }

    //更新状态条父对象
    update_status_bar_parent(dev_info_scr);
    // 切换到设置屏幕
    lv_scr_load(dev_info_scr);
}

/***********************设备信息界面回调*********************/
void dev_info_btn_click_cb(lv_event_t *e)
{
    if(e == NULL) return;
    
    lv_obj_t *homepage_scr = (lv_obj_t *)lv_event_get_user_data(e);
    if(homepage_scr == NULL) {
        LV_LOG_WARN("dev_info_btn_click_cb: homepage_scr is NULL!");
        return;
    }
    ui_dev_info_create(homepage_scr);
}





