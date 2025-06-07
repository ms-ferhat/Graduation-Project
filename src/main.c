#include <bcm2835.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "lvgl/lvgl.h"
#include "xpt2046.h"
//#include "AES.h"
//#include "RSA.h"
//#include "Secure_com.h"

#define ILI9341_RST  RPI_V2_GPIO_P1_22  // GPIO 25
#define ILI9341_DC   RPI_V2_GPIO_P1_18  // GPIO 24
#define ILI9341_CS   RPI_V2_GPIO_P1_24  // SPI0_CE0

#define ILI9341_CMD  0
#define ILI9341_DATA 1

#define LCD_H_RES 240
#define LCD_V_RES 320
#define LCD_BUF_LINES 60


/* Global objects */
lv_display_t *my_disp;
lv_obj_t *msg_list;
lv_obj_t *ta;
lv_obj_t *kb;
lv_obj_t *send_btn;
lv_obj_t *record_btn;



void ili9341_reset();
void ili9341_write(uint8_t data, uint8_t is_data);
void  my_lcd_send_cmd(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, const uint8_t *param, size_t param_size);
void  my_lcd_send_color(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, uint8_t *param, size_t param_size);
static void button_event_handler(lv_event_t *e);
static void touch_read(lv_indev_t *drv, lv_indev_data_t *data);
void show_received_message(const char *txt);
static void send_message(lv_event_t *e);


void lvgl_init_display() {

    bcm2835_gpio_write(ILI9341_CS, LOW);
    /* Create LVGL display object */
    my_disp = lv_lcd_generic_mipi_create(LCD_H_RES, LCD_V_RES,  LV_LCD_FLAG_MIRROR_X, my_lcd_send_cmd, my_lcd_send_color);
    lv_display_set_rotation(my_disp, LV_DISPLAY_ROTATION_0);

    /* Allocate draw buffers */
    uint32_t buf_size = LCD_H_RES * LCD_BUF_LINES * lv_color_format_get_size(lv_display_get_color_format(my_disp));
    uint8_t *buf1 = malloc(buf_size);
    uint8_t *buf2 = malloc(buf_size); // Second buffer for double buffering

    if (buf1 == NULL || buf2 == NULL) {
        printf("Display buffer allocation failed!\n");
        return;
    }

    lv_display_set_buffers(my_disp, buf1, buf2, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
}


static void custom_delay(uint32_t ms)
{
   // printf("call custom delay function/n");
    usleep(ms * 1000);  // Convert milliseconds to microseconds
}

/**********************************************************************
#define SQUARE_SIZE 10  // Size of each square
void create_corner_squares(void) {
    lv_obj_t *btn_tl = lv_button_create(lv_screen_active());
    lv_obj_set_size(btn_tl, SQUARE_SIZE, SQUARE_SIZE);
    lv_obj_set_pos(btn_tl, 0, 0);  // Top-left

    lv_obj_t *btn_tr = lv_button_create(lv_screen_active());
    lv_obj_set_size(btn_tr, SQUARE_SIZE, SQUARE_SIZE);
    lv_obj_align(btn_tr, LV_ALIGN_TOP_RIGHT, 0, 0);  // Top-right

    lv_obj_t *btn_bl = lv_button_create(lv_screen_active());
    lv_obj_set_size(btn_bl, SQUARE_SIZE, SQUARE_SIZE);
    lv_obj_align(btn_bl, LV_ALIGN_BOTTOM_LEFT, 0, 0);  // Bottom-left

    lv_obj_t *btn_br = lv_button_create(lv_screen_active());
    lv_obj_set_size(btn_br, SQUARE_SIZE, SQUARE_SIZE);
    lv_obj_align(btn_br, LV_ALIGN_BOTTOM_RIGHT, 0, 0);  // Bottom-right
}
/************************************************************************/

void create_buttons_ui() {
    lv_obj_t *scr = lv_display_get_screen_active(my_disp);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x003a57), LV_PART_MAIN); // Set background color

    // Create Button 1
    lv_obj_t *btn1 = lv_btn_create(scr);
    lv_obj_set_size(btn1, 100, 50);
    lv_obj_align(btn1, LV_ALIGN_CENTER, -120, 0); // Align to the left
    //lv_obj_add_event_cb(btn1,button_event_handler,LV_EVENT_CLICKED, NULL); // Assign callback
    lv_obj_add_event_cb(btn1, button_event_handler,LV_EVENT_PRESSED, (void *)1);
    lv_obj_t *label1 = lv_label_create(btn1);
    lv_label_set_text(label1, "Button 1");
    lv_obj_center(label1);

    // Create Button 2
    lv_obj_t *btn2 = lv_btn_create(scr);
    lv_obj_set_size(btn2, 100, 50);
    lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 0); // Center button
    //lv_obj_add_event_cb(btn2, button_event_handler,LV_EVENT_CLICKED, NULL); // Assign callback
    lv_obj_add_event_cb(btn2, button_event_handler,/*LV_EVENT_ALL */LV_EVENT_PRESSED, (void *)2);
    lv_obj_t *label2 = lv_label_create(btn2);
    lv_label_set_text(label2, "Button 2");
    lv_obj_center(label2);

    // Create Button 3
    lv_obj_t *btn3 = lv_btn_create(scr);
    lv_obj_set_size(btn3, 100, 50);
    lv_obj_align(btn3, LV_ALIGN_CENTER, 120, 0); // Align to the right
    //lv_obj_add_event_cb(btn3, button_event_handler,LV_EVENT_CLICKED, NULL); // Assign callback
    lv_obj_add_event_cb(btn3, button_event_handler,LV_EVENT_PRESSED/*LV_EVENT_ALL*/, (void *)3);
    lv_obj_t *label3 = lv_label_create(btn3);
    lv_label_set_text(label3, "Button 3");
    lv_obj_center(label3);

}

/* Show keyboard when text area is focused */
static void text_area_event_cb(lv_event_t *e) {
    lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
}


static void record_button_event(lv_event_t *e) {
    static bool is_recording = false;
    if (!is_recording) {
        LV_LOG_USER("Recording started...");
        // TODO: Start actual recording logic
        is_recording = true;
    } else {
        LV_LOG_USER("Recording stopped...");
        // TODO: Stop recording
        is_recording = false;
    }
}
void create_mess_app(void)
{
	lv_obj_t *scr = lv_display_get_screen_active(my_disp);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x003a57), LV_PART_MAIN); // Set background color
	
	
	/* Main container */
    lv_obj_t *cont = lv_obj_create(scr);
    lv_obj_set_size(cont, 240, 320);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	
	/* Message display area */
    msg_list = lv_obj_create(cont);
    lv_obj_set_size(msg_list, 240, 200);
    lv_obj_set_scroll_dir(msg_list, LV_DIR_VER);
    lv_obj_set_flex_flow(msg_list, LV_FLEX_FLOW_COLUMN);

    /* Input area container (Row Layout) */
    lv_obj_t *input_cont = lv_obj_create(cont);
    lv_obj_set_size(input_cont, 240, 40);
    lv_obj_set_flex_flow(input_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(input_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(input_cont, 5, 0); // Add spacing between elements

    /* Create text input */
    ta = lv_textarea_create(input_cont);
    lv_obj_set_size(ta, 240, 30);
    lv_textarea_set_one_line(ta, true);
    lv_obj_add_event_cb(ta, text_area_event_cb, LV_EVENT_FOCUSED, NULL);
    
    /* Create continer fot buttons*/
    lv_obj_t *btn_cont = lv_obj_create(cont);
    lv_obj_set_size(btn_cont, 240, 40);
    lv_obj_set_flex_flow(btn_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(btn_cont, 5, 0); // Add spacing between elements

    /* Create Send button */
    send_btn = lv_btn_create(btn_cont);
    lv_obj_set_size(send_btn, 80, 30);
    lv_obj_t *send_label = lv_label_create(send_btn);
    lv_label_set_text(send_label, "Send");
    lv_obj_center(send_label);
    lv_obj_add_event_cb(send_btn, send_message, LV_EVENT_CLICKED, NULL);

    /* Create Voice Record Button */
    record_btn = lv_btn_create(btn_cont);
    lv_obj_set_size(record_btn, 80, 40);
    lv_obj_t *record_label = lv_label_create(record_btn);
    lv_label_set_text(record_label, "Voice");
    lv_obj_center(record_label);
  //  lv_obj_add_event_cb(record_btn, record_button_event, LV_EVENT_CLICKED, NULL);

    /* Create keyboard */
    kb = lv_keyboard_create(lv_scr_act());
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
}

// Touchscreen read function for LVGL
static void touch_read(lv_indev_t *drv, lv_indev_data_t *data) {
    uint16_t x=0;
    uint16_t y=0;
    if (XPT2046_GetTouch(&x,&y)) {
        printf("Touch: %d, %d\n", x, y); // for checking only
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PRESSED;
        printf("Button with %d and %d are pressed \n",x,y); // 👈 Add this for checking only
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
        printf("Buttons  arn't with %d and %d are released \n",x,y); // 👈 Add this >
    }
}

static void button_event_handler(lv_event_t *e) {
    printf("Button touched!\n"); // 👈 Add this for checking only
    lv_obj_t *btn = lv_event_get_target(e);
    uint32_t btn_id = (uint32_t)lv_event_get_user_data(e);

    switch (btn_id) {
        case 1:
            printf("Button 1 pressed! Executing Task A...\n");
            // Perform Task A here (e.g., toggle LED, send message, etc.)
            break;
        case 2:
            printf("Button 2 pressed! Executing Task B...\n");
            // Perform Task B here
            break;
        case 3:
            printf("Button 3 pressed! Executing Task C...\n");
            // Perform Task C here
            break;
        defult :
            printf("out of the box\n");
            break;
    }
}

/*                        The main function  */
int main() {

      /*    *****************////////////////******************* */
      /*    *****************////////////////******************* */
    printf("main start\n");
    lv_init();
    printf("lv initializied\n");
    lv_delay_set_cb(custom_delay);


  //  printf("lvgl init successful");
    if (!bcm2835_init()) {
        printf("bcm2835 init failed!\n");
        return 1;
    }

    bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64);
    bcm2835_gpio_fsel(ILI9341_RST, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(ILI9341_DC, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(ILI9341_CS, BCM2835_GPIO_FSEL_OUTP);

    ili9341_reset();
    lvgl_init_display();
    printf("lvgl dispay init done\n");
    create_mess_app();
    lv_indev_t *touch_indev = lv_indev_create();
    lv_indev_set_type(touch_indev,LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(touch_indev, touch_read);

   /**********************************************************/
    //create_corner_squares();
   /**********************************************************/
    XPT2046_Init();
    while (1) {
        lv_timer_handler();
        usleep(10000);
          /*  ****************/////////////*******************       */

        uint16_t x, y;
        if (XPT2046_GetTouch(&x, &y)) {
            printf("Touch detected at X: %d, Y: %d\n", x, y);

        }
    }

    return 0;
}


void ili9341_reset() {
    bcm2835_gpio_write(ILI9341_RST, LOW);
    bcm2835_delay(50);
    bcm2835_gpio_write(ILI9341_RST, HIGH);
    bcm2835_delay(50);
}

void ili9341_write(uint8_t data, uint8_t is_data) {
    bcm2835_gpio_write(ILI9341_DC, is_data);  // Command or Data mode
    bcm2835_gpio_write(ILI9341_CS, LOW);
    bcm2835_spi_transfer(data);
    bcm2835_gpio_write(ILI9341_CS, HIGH);
}

/* LVGL: Send command */
void my_lcd_send_cmd(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, const uint8_t *param, size_t param_size) {
    if (cmd_size > 0) {
        ili9341_write(cmd[0], ILI9341_CMD);
    }
    for (size_t i = 0; i < param_size; i++) {
        ili9341_write(param[i], ILI9341_DATA);
    }

}

/* LVGL: Send color (pixel data) */
void  my_lcd_send_color(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, uint8_t *param, size_t param_size) {
    if (cmd_size > 0) {
        ili9341_write(cmd[0], ILI9341_CMD);
    }
    bcm2835_gpio_write(ILI9341_DC, ILI9341_DATA);
    bcm2835_gpio_write(ILI9341_CS, LOW);

    for (size_t i = 0; i < param_size; i++) {
        bcm2835_spi_transfer(param[i]);
    }

    bcm2835_gpio_write(ILI9341_CS, HIGH);

    // Signal LVGL that the transfer is complete
    lv_display_flush_ready(disp);

}


/* Function to show received message (right side) */
void show_received_message(const char *txt) {
    // Container for right-aligned message
    lv_obj_t *msg_cont = lv_obj_create(msg_list);
    lv_obj_set_width(msg_cont, lv_pct(100));
    lv_obj_set_flex_flow(msg_cont, LV_FLEX_FLOW_ROW_REVERSE); // Right to left
    lv_obj_set_style_pad_all(msg_cont, 5, 0);
    lv_obj_set_style_bg_color(msg_cont, lv_palette_lighten(LV_PALETTE_GREY, 3), 0);
    lv_obj_set_style_bg_opa(msg_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(msg_cont, 8, 0);
    lv_obj_set_flex_align(msg_cont, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);

    // Label
    lv_obj_t *label = lv_label_create(msg_cont);
    lv_label_set_text(label, txt);
    lv_obj_set_style_text_color(label, lv_color_black(), 0);
}

/* Function to send message (left side) */

static void send_message(lv_event_t *e) {
    const char *txt = lv_textarea_get_text(ta);
    if (strlen(txt) > 0) {
        // Container for left-aligned message
        lv_obj_t *msg_cont = lv_obj_create(msg_list);
        lv_obj_set_width(msg_cont, lv_pct(100));
        lv_obj_set_flex_flow(msg_cont, LV_FLEX_FLOW_ROW); // Left to right
        lv_obj_set_style_pad_all(msg_cont, 5, 0);
        lv_obj_set_style_bg_color(msg_cont, lv_palette_lighten(LV_PALETTE_BLUE, 3), 0);
        lv_obj_set_style_bg_opa(msg_cont, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(msg_cont, 8, 0);
        lv_obj_set_flex_align(msg_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

        // Label
        lv_obj_t *label = lv_label_create(msg_cont);
        lv_label_set_text(label, txt);
        lv_obj_set_style_text_color(label, lv_color_black(), 0);

        lv_obj_scroll_to_y(msg_list, lv_obj_get_scroll_bottom(msg_list), LV_ANIM_ON);
        lv_textarea_set_text(ta, "");

        // Optional: Simulated reply
        show_received_message("the received message will appear hear ");
    }
}
