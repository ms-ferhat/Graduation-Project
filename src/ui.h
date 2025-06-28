#ifndef UI_H
#define UI_H

#include <bcm2835.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <string.h>
#include "lvgl/lvgl.h"
#include <unistd.h>
#include "xpt2046.h"
#include <stdlib.h>


#define ILI9341_RST  RPI_V2_GPIO_P1_22  // GPIO 25
#define ILI9341_DC   RPI_V2_GPIO_P1_18  // GPIO 24
#define ILI9341_CS   RPI_V2_GPIO_P1_24  // SPI0_CE0

#define ILI9341_CMD  0
#define ILI9341_DATA 1

#define LCD_H_RES 240
#define LCD_V_RES 320
#define LCD_BUF_LINES 60

#define MAX_TEXT_MESSAGES   40 // Max number of dynamic text messages (sent/received)
#define MAX_VOICE_MESSAGES  40 // Max number of dynamic voice messages (sent/received)
#define RECORD_DURATION_SEC 10 // Duration of voice recording in seconds
#define MAX_CHAT_HISTORY_SIZE (MAX_TEXT_MESSAGES * 2 + MAX_VOICE_MESSAGES * 2)

#define HISTORY_FILE_PATH "/home/gp/chat_history.txt"

// --- Unified Chat Message Data Structure ---
// Defines the type of message and its properties
typedef enum {
    MSG_TYPE_TEXT,
    MSG_TYPE_VOICE
} MessageType;

typedef struct {
    MessageType type;
    bool is_sent; // true for sent (by current user), false for received (from another user)
    const char* content_ptr; // Points to the text string or the voice file path in the storage arrays
} ChatMessage;


// --- Dynamic Storage for Message Content ---
// These arrays hold the actual string data for text messages and received voice paths.
// Sent voice paths are handled by recorded_voice_paths_storage.


void ili9341_reset();
void lvgl_clear_screen(void);
void ili9341_write(uint8_t data, uint8_t is_data);
void  my_lcd_send_cmd(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, const uint8_t *param, size_t param_size);
void  my_lcd_send_color(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, uint8_t *param, size_t param_size);

void touch_read(lv_indev_t *drv, lv_indev_data_t *data);
void lvgl_init_display();

void creat_UI_1();
void create_UI_2(lv_event_t *e);
void voice_message_event_cb(lv_event_t * e); //play voice message when press
void send_text_message_event_cb(lv_event_t * e); // Callback for sending text messages
void back_to_UI_1_event_cb(lv_event_t * e); // Callback to switch back to UI_1

// Persistence functions
void save_chat_history();
void load_chat_history();

#endif // MAIN_H
