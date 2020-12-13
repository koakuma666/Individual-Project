/*  Author: MorseMeow
    Date: 28 July, 2020 
*/

/**
 * @file tab_trans.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <tab_trans.h>

// declaration for lvgl fonts
LV_FONT_DECLARE(my_font_data_30);
LV_FONT_DECLARE(my_font_list_icon);
LV_FONT_DECLARE(my_font_remind_icon);
LV_FONT_DECLARE(my_font_sports_remind_icon);
LV_FONT_DECLARE(my_font_remind_back);
// declaration for lvgl image
LV_IMG_DECLARE(welcome_page);

/*********************
 *      DEFINES
 *********************/
// size of 30
#define MY_ICON_STEPCOUNT       "\xEE\x99\x8F"          // 0xe64f unicode
#define MY_ICON_SPEED           "\xEE\xAA\xAC"          // 0xeaac 
#define MY_ICON_TEMPERATURE     "\xEE\x99\xAA"          // 0xe66a
#define MY_ICON_REMIND          "\xEE\x99\xB9"          // 0xe679
#define MY_ICON_HUMIDITY        "\xEE\x98\x8A"          // 0xe60a
#define MY_ICON_DISTANCE        "\xEE\x98\x80"          // 0xe600
#define MY_ICON_CALORIES        "\xEE\xA5\xA0"          // 0xe960
#define MY_ICON_PRESSURE        "\xEE\x98\xA4"          // 0xe624
#define MY_ICON_TIME            "\xEE\x9D\xAB"          // 0xe76b
// size of 50
#define MY_ICON_DANCING         "\xEE\x98\x8E"          // 0xe60e
#define MY_ICON_ROPE_SKIPPING   "\xEE\x98\x9B"          // 0xe61b
#define MY_ICON_SWIMMING        "\xEE\x98\x9C"          // 0xe61c
#define MY_ICON_RUNNING         "\xEE\x99\xB7"          // 0xe677
// remind icons
#define MY_ICON_YES             "\xEE\x98\x8F"          // 0xe60f
#define MY_ICON_NO              "\xEE\x99\x87"          // 0xe647
#define MY_ICON_LIKE            "\xEE\x99\xB2"          // 0xe672
#define MY_ICON_SAD             "\xEE\x98\xA8"          // 0xe628
// arrow icons
#define MY_ICON_ARROW           "\xEE\x98\x80"          // 0xe600

// remind mbox
// the width of message box
#define MBOX_WIDTH      200
// the width of the buttons in the message box
#define MBOX_BTN_WIDTH  140
// the height of the buttons in the message box
#define MBOX_BTN_HEIGHT 40

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void Welcome_create(lv_obj_t * parent);                              // for welcome page
static void Monitor_create(lv_obj_t * parent);                              // for activity monitoring page
static void btn_event_handler(lv_obj_t * btn, lv_event_t event);            // for button event handler of activity monitoring page
static void update_monitor(lv_task_t * task);                               // for value update of activity monitoring page 
static void Environment_create(lv_obj_t * parent);                          // for enviroment monitoring page
static void update_environment(lv_task_t * task);                           // for value update of environment monitoring page
// remind boxes
static lv_obj_t * remind_do1_create(lv_obj_t * parent);                     // for remind box
static void remind_do_event_handler(lv_obj_t * obj, lv_event_t event);      // for event handler of remind box 
static lv_obj_t * remind_like_create(lv_obj_t * parent);                    // for remind box2
static lv_obj_t * remind_sad_create(lv_obj_t * parent);                     // for remind box3

/**********************
 *  STATIC VARIABLES
 **********************/
// for remind boxes
static lv_obj_t * mbox;
static lv_obj_t * mbox2;
static lv_obj_t * mbox3;

static lv_obj_t * tabview1;                                                 // for switch pages               
static lv_obj_t * image1;                                                   // for image of welcome page
static lv_task_t * moni_task = NULL;

static lv_style_t moni_text_style;                                          // Monitor_create text style
static lv_style_t my_style1;                                                // Button background style
static lv_style_t my_style2;                                                // Button text style

static lv_obj_t * btn1;
static lv_obj_t * btn1_label;

static lv_obj_t * label_time_data;
static lv_obj_t * label_step_data;
static lv_obj_t * label_dist_data;
static lv_obj_t * label_ener_data;
static lv_obj_t * label_spee_data;

static lv_obj_t * label10;
static lv_obj_t * label12;
static lv_obj_t * label14;

volatile bool active_en = false;
volatile bool active_change = false;
volatile bool seden_en = false;
volatile bool seden_change = false;
// struct BME280 BME280_Value;
const char * const btnm1_map[] = {"#ff0000 OK#","#5FB878 NO#",""};


/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
 * Create a demo application
 */
// create a tabview for switch pages
void tabview_create(void) {

    //get the horizontal and vertical resolution of default display
    lv_coord_t hres = lv_disp_get_hor_res(NULL);
    lv_coord_t vres = lv_disp_get_ver_res(NULL);

    //create tabview1 for switch pages
    lv_obj_t * scr = lv_scr_act();
    tabview1 = lv_tabview_create(scr, NULL);
    lv_obj_set_size(tabview1, hres, vres);
    lv_obj_align(tabview1, NULL, LV_ALIGN_CENTER, 0, 0);

    // add contents of three pages of tabview1
    lv_obj_t * tab1_page = lv_tabview_add_tab(tabview1, "WelcomePage");
    lv_obj_t * tab2_page = lv_tabview_add_tab(tabview1, "Monitor");
    lv_obj_t * tab3_page = lv_tabview_add_tab(tabview1, "Environment");

    // To set the tabview1 background style, you need to copy a style from LVGL and then add the style that needs to be modified
    static lv_style_t style_tabview1_bg;
    lv_style_copy(&style_tabview1_bg, &lv_style_plain);
    style_tabview1_bg.body.main_color = lv_color_make(212, 242, 231);
    style_tabview1_bg.body.grad_color = lv_color_make(212, 242, 231);
    style_tabview1_bg.body.padding.top = 0;
    style_tabview1_bg.body.padding.bottom = 0;
    style_tabview1_bg.body.opa = LV_OPA_COVER; // 255
    
    /*   tabview1 style data type   */
    // The entire background style of tabview1
    lv_tabview_set_style(tabview1, LV_TABVIEW_STYLE_BG, &style_tabview1_bg);
    // The first interface after booting is the default display interface
    lv_tabview_set_tab_act(tabview1, 0, LV_ANIM_ON); 
    // Set page switching duration
    lv_tabview_set_anim_time(tabview1, 1);
    // Hide tabview1's interface switch button            
    lv_tabview_set_btns_hidden(tabview1, true);
    // Add three pages of tabview1
    Welcome_create(tab1_page);
    Monitor_create(tab2_page);
    Environment_create(tab3_page);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
// welcome page creation
static void Welcome_create(lv_obj_t * parent) {   

    // image
    image1 = lv_img_create(parent, NULL);                                   // Create an image control
    lv_img_set_src(image1, &welcome_page);                                  // Set a picture for the control
    lv_obj_set_width(image1, 235);                                          // set the width of image
    lv_obj_set_height(image1, 235);                                         // set the height of image
    lv_obj_align(image1, NULL, LV_ALIGN_CENTER, 0, 0);                      // Set alignment for image
}

// activity monitoring page creation
static void Monitor_create(lv_obj_t * parent) {

    // Set the background style of the list
    static lv_style_t style_list1_bg;
    lv_style_copy(&style_list1_bg, &lv_style_plain);
    style_list1_bg.body.main_color = lv_color_make(212, 242, 231);          // Set the color of the upper part of the list background to Aqua
    style_list1_bg.body.grad_color = lv_color_make(212, 242, 231);          // Set the color of the upper part of the list background to Aqua
    style_list1_bg.body.padding.top = 0;                                    // Set the top edge padding interval
    style_list1_bg.body.padding.bottom = 0;                                 // Set the bottom edge padding interval
    style_list1_bg.body.opa = LV_OPA_COVER;                                 // Set the background transparency to 255

    // You need to set the width and so on to set the style to take effect, otherwise it will not take effect
    // create a list of monitor page
    lv_obj_t * list1 = lv_list_create(parent, NULL); 
    lv_obj_set_height(list1, 2 * lv_obj_get_height(parent) / 3);
    lv_list_set_style(list1, LV_LIST_STYLE_BG, &style_list1_bg);
    lv_list_set_style(list1, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);
    lv_obj_align(list1, parent, LV_ALIGN_CENTER, 0, 0);                     // Set alignment
    lv_list_set_edge_flash(list1, true);                                    // The edge flashes, reminding that the sliding list has reached the edge

	lv_style_copy(&moni_text_style, &lv_style_plain_color);                 // Style copy
	moni_text_style.text.font = &my_font_list_icon;                         // Use custom icon fonts in styles
    moni_text_style.text.color = lv_color_make(255, 140, 0);
    moni_text_style.body.opa = LV_OPA_COVER; // 255

/****************************************Create and set the style of the buttons on the list interface****************************************/
    lv_style_copy(&my_style1, &lv_style_plain_color);                       // Set button interface style
    lv_style_copy(&my_style2, &moni_text_style);
    btn1 = lv_btn_create(list1, NULL);                                      // Create a button for the list interface
    btn1_label = lv_label_create(btn1, NULL);                               // Create the label of the button of the list interface
    my_style1.body.main_color = lv_color_make(0, 255, 0); 
    my_style1.body.grad_color = lv_color_make(0, 255, 0);
    lv_btn_set_style(btn1, LV_BAR_STYLE_BG, &my_style1);                    // Apply a custom style to the button
    lv_label_set_text(btn1_label, "STA");                                   // Set the label on the button to set the displayed content
    my_style2.text.color = lv_color_make(255, 0, 0);
    lv_label_set_style(btn1_label, LV_LABEL_STYLE_MAIN, &my_style2); 
    lv_obj_set_event_cb(btn1, btn_event_handler);                           // Set the event callback of btn1
/**************************************************************************************************************/
    // create a label of "time" which includes icon and title(using customize icons and font)
	lv_obj_t * label_time_title = lv_label_create(list1, NULL);             
    lv_obj_align(label_time_title, list1, LV_ALIGN_IN_TOP_MID, 0, 0);       // The label stays centered on the screen
	lv_label_set_style(label_time_title, LV_LABEL_STYLE_MAIN, &moni_text_style); 
    lv_label_set_text(label_time_title, MY_ICON_TIME "Time");

    // create a label of "time" which is used to diaplay the activity time
    label_time_data = lv_label_create(list1, NULL);
    lv_label_set_recolor(label_time_data, true);                            // Color the text
    lv_label_set_text_fmt(label_time_data, "#ff0000 %d##2F4F4F h##ff0000 %d##2F4F4F \'##ff0000 %d##2F4F4F \"#", sport_time.hour, sport_time.minu, sport_time.seco); // DarkSlateGray
    
    // create a label of "step" which includes icon and title(using customize icons and font)
	lv_obj_t * label_step_title = lv_label_create(list1, NULL);   
    lv_obj_align(label_step_title, list1, LV_ALIGN_IN_TOP_MID, 0, 0); 
	lv_label_set_style(label_step_title, LV_LABEL_STYLE_MAIN, &moni_text_style); 
    lv_label_set_text(label_step_title, MY_ICON_STEPCOUNT "Step Count");

    // create a label of "step counter" which is used to diaplay the step counter
    label_step_data = lv_label_create(list1, NULL);    
    lv_label_set_recolor(label_step_data, true);    
    lv_label_set_text_fmt(label_step_data, "#ff0000 %d##2F4F4F Steps#", BMI160_Value.step); 

    // create a label of "distance" which includes icon and title(using customize icons and font)
	lv_obj_t * label_dist_title = lv_label_create(list1, NULL);    
    lv_obj_align(label_dist_title, list1, LV_ALIGN_IN_TOP_MID, 0, 0); 
	lv_label_set_style(label_dist_title, LV_LABEL_STYLE_MAIN, &moni_text_style); 
    lv_label_set_text(label_dist_title, MY_ICON_DISTANCE "Distance");

    // create a label of "distance" which is used to diaplay the distance of activity    
    label_dist_data = lv_label_create(list1, NULL);
    lv_label_set_recolor(label_dist_data, true); 
    lv_label_set_text_fmt(label_dist_data, "#ff0000 %.2f##2F4F4F meters#", BMI160_Value.dist); 

    // create a label of "energy consumption" which includes icon and title(using customize icons and font)
	lv_obj_t * label_ener_title = lv_label_create(list1, NULL);    
    lv_obj_align(label_ener_title, list1, LV_ALIGN_IN_TOP_MID, 0, 0); 
	lv_label_set_style(label_ener_title, LV_LABEL_STYLE_MAIN, &moni_text_style);
    lv_label_set_text(label_ener_title, MY_ICON_CALORIES "Energy");         // the unit is calories

    // create a label of "energy consumption" which is used to diaplay the energy consumption
    label_ener_data = lv_label_create(list1, NULL);
    lv_label_set_recolor(label_ener_data, true); 
    lv_label_set_text_fmt(label_ener_data, "#ff0000 %.2f##2F4F4F calories#", BMI160_Value.ener);

    // create a label of "speed" which includes icon and title(using customize icons and font)
	lv_obj_t * label_spee_title = lv_label_create(list1, NULL);    
    lv_obj_align(label_spee_title, list1, LV_ALIGN_IN_TOP_MID, 0, 0); 
	lv_label_set_style(label_spee_title, LV_LABEL_STYLE_MAIN, &moni_text_style);
    lv_label_set_text(label_spee_title, MY_ICON_SPEED "Speed"); 

    // create a label of "speed" which is used to diaplay the speed
    label_spee_data = lv_label_create(list1, NULL);
    lv_label_set_recolor(label_spee_data, true); 
    lv_label_set_text_fmt(label_spee_data, "#ff0000 %.2f##2F4F4F m/s#", BMI160_Value.spee);    
}

// create the event handler of the button on the list of Monitor_page
static void btn_event_handler(lv_obj_t * btn, lv_event_t event) {

        if(event == LV_EVENT_CLICKED) {
            active_change = true;
            active_en = !active_en;
            if(active_en) {  // Key press odd number of times
                
                sport_time.start_ms = millis();
                my_style1.body.main_color = lv_color_make(255, 0, 0);       // Red button background
                my_style1.body.grad_color = lv_color_make(255, 0, 0);
                lv_btn_set_style(btn1, LV_BAR_STYLE_BG, &my_style1);        
                lv_label_set_text(btn1_label, "END");
                my_style2.text.color = lv_color_make(0, 255, 0);            // The green word stands for end
                lv_label_set_style(btn1_label, LV_LABEL_STYLE_MAIN, &my_style2); 
                update_monitor(NULL);
                // update the value of Monitor_page the period can be set, now is 100ms
                moni_task = lv_task_create(update_monitor, 100, LV_TASK_PRIO_LOW, NULL);
            }
            else {           // Press the key an even number of times
                
                my_style1.body.main_color = lv_color_make(0, 255, 0);       // Green button background
                my_style1.body.grad_color = lv_color_make(0, 255, 0);
                lv_btn_set_style(btn1, LV_BAR_STYLE_BG, &my_style1);         
                lv_label_set_text(btn1_label, "STA");
                my_style2.text.color = lv_color_make(255, 0, 0);            // The red word represents start
                lv_label_set_style(btn1_label, LV_LABEL_STYLE_MAIN, &my_style2); // Set style

                sport_time.start_ms = 0;
                sport_time.duration_s = 0;
                sport_time.hour = 0;
                sport_time.minu = 0;
                sport_time.seco = 0;
                BMI160_Value.step = 0;
                BMI160_Value.dist = 0;
                BMI160_Value.ener = 0;
                BMI160_Value.spee = 0;
                update_monitor(NULL);
                lv_task_del(moni_task);
            }
        } 
}

// update the value of Monitor_page the period can be set, now is 100ms
static void update_monitor(lv_task_t * task) {

    lv_label_set_text_fmt(label_time_data, "#ff0000 %d##2F4F4F h##ff0000 %d##2F4F4F \'##ff0000 %d##2F4F4F \"#", sport_time.hour, sport_time.minu, sport_time.seco); // DarkSlateGray
    lv_label_set_text_fmt(label_step_data, "#ff0000 %d##2F4F4F Steps#", BMI160_Value.step);      // DarkSlateGray
    lv_label_set_text_fmt(label_dist_data, "#ff0000 %.2f##2F4F4F meters#", BMI160_Value.dist);   // DarkSlateGray
    lv_label_set_text_fmt(label_ener_data, "#ff0000 %.2f##2F4F4F calories#", BMI160_Value.ener); // DarkSlateGray
    lv_label_set_text_fmt(label_spee_data, "#ff0000 %.2f##2F4F4F m/s#", BMI160_Value.spee);      // DarkSlateGray
}

// create the page of environment variables detection
static void Environment_create(lv_obj_t * parent) {

 	// 设置list背景样式 
    static lv_style_t style_list2_bg;
    lv_style_copy(&style_list2_bg, &lv_style_plain);
    style_list2_bg.body.main_color = lv_color_make(212, 242, 231); 
    style_list2_bg.body.grad_color = lv_color_make(212, 242, 231);
    style_list2_bg.body.padding.top = 0;
    style_list2_bg.body.padding.bottom = 0;
    style_list2_bg.body.opa = LV_OPA_COVER; 

    // create a list of environment variables detection
    lv_obj_t * list2 = lv_list_create(parent, NULL); 
    lv_obj_set_height(list2, 2 * lv_obj_get_height(parent) / 3);
    lv_obj_set_width(list2, 200);
    lv_list_set_style(list2, LV_LIST_STYLE_BG, &style_list2_bg);
    lv_list_set_style(list2, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);
    lv_obj_align(list2, parent, LV_ALIGN_CENTER, 0, 0);
    lv_list_set_edge_flash(list2, true); 
    // create the style of the text of environment page, using customize font and icons
	static lv_style_t envi_text_style;
	lv_style_copy(&envi_text_style, &lv_style_plain_color); 
	envi_text_style.text.font = &my_font_list_icon; 
    envi_text_style.text.color = lv_color_make(46, 139, 87); 
    envi_text_style.body.opa = LV_OPA_COVER; 

    // create a label of "Humidity" which includes icon and title(using customize icons and font)
	lv_obj_t * label9 = lv_label_create(list2, NULL); 
    lv_obj_align(label9,list2, LV_ALIGN_IN_TOP_MID, 0, 0); 
	lv_label_set_style(label9, LV_LABEL_STYLE_MAIN, &envi_text_style);
    lv_label_set_text(label9, MY_ICON_HUMIDITY "Humidity");

    // create a label of "humidity" which is used to diaplay the humidity
    label10 = lv_label_create(list2, NULL);
    lv_label_set_recolor(label10, true);  
    lv_label_set_text_fmt(label10, "#ff0000 %.2f##2F4F4F %%#", BME280_Value.humi); 

    // create a label of "temperature" which includes icon and title(using customize icons and font)
	lv_obj_t * label11 = lv_label_create(list2, NULL); 
    lv_obj_align(label11, list2, LV_ALIGN_IN_TOP_MID, 0, 0); 
	lv_label_set_style(label11, LV_LABEL_STYLE_MAIN, &envi_text_style); 
    lv_label_set_text(label11, MY_ICON_TEMPERATURE "Tempera.");

    // create a label of "temperature" which is used to diaplay the temperature    
    label12 = lv_label_create(list2, NULL);
    lv_label_set_recolor(label12, true); 
    lv_label_set_text_fmt(label12, "#ff0000 %.2f##2F4F4F *C#", BME280_Value.temp); 

    // create a label of "Altitude" which includes icon and title(using customize icons and font)
	lv_obj_t * label13 = lv_label_create(list2, NULL); 
    lv_obj_align(label13, list2, LV_ALIGN_IN_TOP_MID, 0, 0); 
	lv_label_set_style(label13, LV_LABEL_STYLE_MAIN, &envi_text_style); 
    lv_label_set_text(label13, MY_ICON_PRESSURE "Altitude");

    // create a label of "Altitude" which is used to diaplay the Altitude
    label14 = lv_label_create(list2, NULL);
    lv_label_set_recolor(label14, true); 
    lv_label_set_text_fmt(label14, "#ff0000 %.2f##2F4F4F m#", BME280_Value.temp); 

    // create the environment date updates task
    lv_task_t * envi_task = lv_task_create(update_environment, 500, LV_TASK_PRIO_LOWEST, NULL);
}

// update the value of Environment_page the period can be set, now is 500ms
static void update_environment(lv_task_t * task) {

    lv_label_set_text_fmt(label10, "#ff0000 %.2f##2F4F4F %%#", BME280_Value.humi); // DarkSlateGray
    lv_label_set_text_fmt(label12, "#ff0000 %.2f##2F4F4F *C#", BME280_Value.temp); // DarkSlateGray
    lv_label_set_text_fmt(label14, "#ff0000 %.2f##2F4F4F m#", BME280_Value.alti);  // DarkSlateGray
    /**mbox**********************************************************************/
    if(seden_en && seden_change) {     
        seden_change = false;
        remind_do1_create(tabview1);                                               // 调用remind box
    }
}

// create a message box to remind children to do some sports when the step counter is lower than 100 steps in 30 minutes
static lv_obj_t * remind_do1_create(lv_obj_t * parent) {
    
    mbox = lv_mbox_create(parent, NULL);
    static lv_style_t my_style;
    lv_style_copy(&my_style, &lv_style_plain_color);   
    my_style.text.font = &my_font_sports_remind_icon;  
    my_style.text.color = lv_color_make(255, 255, 0);
    my_style.body.opa = LV_OPA_COVER;  

    lv_label_set_style(mbox, LV_LABEL_STYLE_MAIN, &my_style);
    lv_mbox_set_text(mbox, MY_ICON_ROPE_SKIPPING MY_ICON_RUNNING MY_ICON_DANCING "\n" MY_ICON_YES"|"MY_ICON_NO);
    lv_mbox_add_btns(mbox, (const char**)btnm1_map);                            // Set button mapping table
    lv_mbox_set_recolor(mbox, true);                                            // Enable the text redrawing function of its internal buttons
    lv_obj_set_width(mbox, MBOX_WIDTH);                                         // Set a fixed width, the height will be adaptive
    lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);                            // Set the center alignment with the parent object
    lv_obj_set_event_cb(mbox, remind_do_event_handler);                         // Set event callback function

    // It is best to set the various characteristics of the internal matrix buttons after all the initialization of the message dialog box is completed, otherwise the expected effect may not be obtained
    lv_obj_t * btnm_of_mbox = lv_mbox_get_btnm(mbox);                           // Get the matrix button object inside
    // Set the size of the matrix button 
    lv_obj_set_size(btnm_of_mbox, MBOX_BTN_WIDTH, MBOX_BTN_HEIGHT);
}

/****************************************mbox event handler*****************************************************/
// create a message box handler to close the first remind box and do respond for the button ok or no
static void remind_do_event_handler(lv_obj_t * obj, lv_event_t event) {

    if(event == LV_EVENT_VALUE_CHANGED) {
        // Get button id
        uint8_t btn_id = lv_mbox_get_active_btn(obj);
        if(btn_id > -1) {
            seden_en = false;
            lv_mbox_start_auto_close(mbox, 0);
            if(btn_id == 0) { 		// OK
                remind_like_create(tabview1);
            }else if(btn_id == 1) { // NO
                remind_sad_create(tabview1);
            }
        }
    }
}

// create a message box which is used to show the "like" after the children click "OK" in the remind page
static lv_obj_t * remind_like_create(lv_obj_t * parent) {

    mbox2 = lv_mbox_create(parent, NULL);
  	static lv_style_t my_style;
	lv_style_copy(&my_style, &lv_style_plain_color); 
	my_style.text.font = &my_font_remind_back;       
    my_style.text.color = lv_color_make(255, 255, 0);
    my_style.body.opa = LV_OPA_COVER; 

    lv_label_set_style(mbox2, LV_LABEL_STYLE_MAIN, &my_style); 
    lv_mbox_set_text(mbox2, MY_ICON_LIKE);
    lv_obj_set_width(mbox2, MBOX_WIDTH);   
    lv_obj_align(mbox2, NULL, LV_ALIGN_CENTER, 0, 0);
    // Timed for 2 seconds, automatically close the message dialog
    lv_mbox_start_auto_close(mbox2, 2000);                                     
}

// create a message box which is used to show the "Sad" after the children click "NO" in the remind page
static lv_obj_t * remind_sad_create(lv_obj_t * parent) {

    mbox3 = lv_mbox_create(parent, NULL);
  	static lv_style_t my_style;
	lv_style_copy(&my_style, &lv_style_plain_color); 
	my_style.text.font = &my_font_remind_back;       
    my_style.text.color = lv_color_make(255, 255, 0);
    my_style.body.opa = LV_OPA_COVER;  

    lv_label_set_style(mbox3, LV_LABEL_STYLE_MAIN, &my_style); 
    lv_mbox_set_text(mbox3, MY_ICON_SAD );
    lv_obj_set_width(mbox3, MBOX_WIDTH); 
    lv_obj_align(mbox3, NULL, LV_ALIGN_CENTER, 0, 0);
    // Timed for 2 seconds, automatically close the message dialog
    lv_mbox_start_auto_close(mbox3, 2000);
}
