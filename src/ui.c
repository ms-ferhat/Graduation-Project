#include "main.h"
//chat_history.txt
static lv_obj_t *record_button_obj_ptr = NULL; // Pointer to the voice record button
static lv_timer_t *record_timer = NULL; // Timer for button color change
static lv_obj_t *global_chat_panel_ptr = NULL;// Declare chat_panel globally so record_button_event can access it
static lv_obj_t *ui2_text_area_ptr = NULL; // Pointer to the text area in UI_2

static void touch_read(lv_indev_t *drv, lv_indev_data_t *data);
static void ta_event_cb(lv_event_t *e);
static void record_button_event(lv_event_t *e);
static void refresh_chat_display(void); // Function to refresh the chat panel content
static void add_message_to_history(MessageType type, bool is_sent, const char* content_ptr);
static void create_text_message_bubble(lv_obj_t *parent, const char *text, bool is_sent);
static void create_voice_message_bubble(lv_obj_t *parent, const char *file_path, bool is_sent);
static void pass_recieved_message(MessageType type, const char* inputPath, char** outputText);

// Storage for dynamically sent text messages my_disp
static char sent_text_storage[MAX_TEXT_MESSAGES][128];
static int next_sent_text_index = 0;

// Storage for dynamically received text messages (for future expansion/simulation)
static char received_text_storage[MAX_TEXT_MESSAGES][128];
static int next_received_text_index = 0;

// Storage for dynamically received voice paths (for future expansion/simulation)
static char received_voice_paths_storage[MAX_VOICE_MESSAGES][128];
static int next_received_voice_index = 0;

// Storage for dynamically sent voice paths (from recordings)
static char recorded_voice_paths_storage[MAX_VOICE_MESSAGES][128];
static int next_record_file_num = 1; // Used to generate unique filenames like send1.wav, send2.wav

static ChatMessage chat_history[MAX_CHAT_HISTORY_SIZE];
static int chat_history_count = 0; // Current number of messages in the history


/* Global objects */
lv_display_t *my_disp;
lv_obj_t *msg_list;
lv_obj_t *ta;
lv_obj_t *kb;
lv_obj_t *send_btn;
lv_obj_t *record_btn;
static void pass_recieved_message(MessageType type, const char* inputPath, char** outputText){
    if((type == MSG_TYPE_VOICE) && (inputPath != NULL)){
        if (next_received_voice_index < MAX_VOICE_MESSAGES) {
            strncpy(received_voice_paths_storage[next_received_voice_index], inputPath, 255); // Use 255 for buffer size 256
            received_voice_paths_storage[next_received_voice_index][255] = '\0'; // Ensure null termination
            const char *new_voice_path = received_voice_paths_storage[next_received_voice_index]; // Corrected: point to received storage
            add_message_to_history(type, false, new_voice_path);
            next_received_voice_index++;
        } else {
            LV_LOG_WARN("Received voice message storage full. Cannot add more.");
        }
    } else if ((type == MSG_TYPE_TEXT) && (outputText != NULL) && (*outputText != NULL)){
        if (next_received_text_index < MAX_TEXT_MESSAGES) {
            strncpy(received_text_storage[next_received_text_index], *outputText, 255); // Use 255
            received_text_storage[next_received_text_index][255] = '\0'; // Ensure null termination
            const char *new_text_ptr = received_text_storage[next_received_text_index];
            add_message_to_history(type, false, new_text_ptr);
            next_received_text_index++;
        } else {
            LV_LOG_WARN("Received text message storage full. Cannot add more.");
        }
    } else {
        LV_LOG_WARN("pass_recieved_message called with invalid arguments or type.");
    }
}

void creat_UI_1() // The first screen to appear
{
    // Get the active screen of the default display.
    lv_display_t *my_disp_local = lv_display_get_default();
    lv_obj_t *scr = lv_display_get_screen_active(my_disp_local);

    // Clear existing children on the screen to avoid duplicates when creat_UI_1 is called again
    lv_obj_clean(scr);

    // Set the background color for the entire screen.
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x003a57), LV_PART_MAIN);

    // Define the height for the message button and the voice button.
    lv_coord_t message_button_height = 70; // Height of the full-width message button
    lv_coord_t voice_button_height = 40; // Height of the voice button
    lv_coord_t gap_height = 5;     // Small gap between chat area and button

    // Calculate the height for the chat panel.
    lv_coord_t chat_panel_height = LV_VER_RES - message_button_height - gap_height;
    if (chat_panel_height < 0) chat_panel_height = 0;


    // --- Create the main Chat Area Panel ---
    lv_obj_t *chat_panel = lv_obj_create(scr);
    global_chat_panel_ptr = chat_panel; // Store pointer globally for refresh_chat_display

    lv_obj_set_width(chat_panel, LV_PCT(100));
    lv_obj_set_height(chat_panel, chat_panel_height);
    lv_obj_align(chat_panel, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(chat_panel, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(chat_panel, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_pad_all(chat_panel, 10, LV_PART_MAIN);

    lv_obj_set_style_border_width(chat_panel, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(chat_panel, lv_color_hex(0x666666), LV_PART_MAIN);
    lv_obj_set_style_radius(chat_panel, 0, LV_PART_MAIN);

    lv_obj_set_flex_flow(chat_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(chat_panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_add_flag(chat_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_row(chat_panel, 8, LV_PART_MAIN);

    // Refresh the chat display with all messages in history
    refresh_chat_display();


    // --- Create Message Button (at the bottom, full width) ---
    lv_obj_t *message_btn = lv_btn_create(scr); // Parent is the screen
    // Set width to 100% of the screen.
    lv_obj_set_width(message_btn, LV_PCT(100));
    // Set height to the predefined message_button_height.
    lv_obj_set_height(message_btn, message_button_height);
    // Align to the very bottom-middle of the screen.
    lv_obj_align(message_btn, LV_ALIGN_BOTTOM_MID, 0, 0);
    // Attach your existing create_UI_2 callback.
    lv_obj_add_event_cb(message_btn, create_UI_2, LV_EVENT_CLICKED, NULL);

    // Create a label for the button text.
    lv_obj_t *label1 = lv_label_create(message_btn);
    lv_label_set_text(label1, "Message");
    lv_obj_center(label1); // Center the label text on the button.

}

void create_UI_2(lv_event_t *e)
{
	printf(" the Message Button pressed!!\n");

    lvgl_clear_screen();
	lv_obj_t *scr = lv_display_get_screen_active(my_disp);
    /*Create a keyboard to use it with an of the text areas*/
    lv_obj_t * kb = lv_keyboard_create(scr);

    /*Create a text area. The keyboard will write here*/
    lv_obj_t * ta1;
    ta1 = lv_textarea_create(scr);
    ui2_text_area_ptr = ta1; // Store pointer globally for send_text_message_event_cb
	lv_obj_set_size(ta1, 220,100);
    lv_obj_align(ta1, LV_ALIGN_BOTTOM_LEFT, 10,-210);
    lv_obj_add_event_cb(ta1, ta_event_cb, LV_EVENT_ALL, kb);
    lv_textarea_set_placeholder_text(ta1, "Write Your Message");
    lv_obj_add_state(ta1, LV_STATE_FOCUSED);
    lv_keyboard_set_textarea(kb, ta1);
    lv_textarea_set_max_length(ta1, 255); // Max characters
    lv_textarea_set_one_line(ta1, false); // Allow multiple lines

    // Create Send Button
    lv_obj_t *send_btn = lv_btn_create(scr);
    lv_obj_set_size(send_btn, 85, 40);
    lv_obj_align(send_btn, LV_ALIGN_BOTTOM_LEFT,10,-170); // Align to the left
    lv_obj_add_event_cb(send_btn, send_text_message_event_cb,LV_EVENT_CLICKED,NULL);
    lv_obj_t *label2 = lv_label_create(send_btn);
    lv_label_set_text(label2, "Send");
    lv_obj_center(label2);
	// Create Voice Button
    lv_obj_t *voice_btn = lv_btn_create(scr);
    lv_obj_set_size(voice_btn, 85, 40);
    lv_obj_align(voice_btn, LV_ALIGN_BOTTOM_LEFT,100,-170); // Align to the left
    lv_obj_add_event_cb(voice_btn, record_button_event,LV_EVENT_CLICKED,NULL);
    lv_obj_t *label3 = lv_label_create(voice_btn);
    lv_label_set_text(label3, "Voice");
    lv_obj_center(label3);
    // Create back Button
    lv_obj_t *back_btn = lv_btn_create(scr);
    lv_obj_set_size(back_btn, 40, 40);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_LEFT,190,-170); // Align to the left
    lv_obj_add_event_cb(back_btn, back_to_UI_1_event_cb,LV_EVENT_CLICKED,NULL);
    lv_obj_t *label4 = lv_label_create(back_btn);
    lv_label_set_text(label4, "<");
    lv_obj_center(label4);

    /*The keyboard will show Arabic characters if they are enabled */
#if LV_USE_ARABIC_PERSIAN_CHARS && LV_FONT_DEJAVU_16_PERSIAN_HEBREW
    lv_obj_set_style_text_font(kb, &lv_font_dejavu_16_persian_hebrew, 0);
    lv_obj_set_style_text_font(ta1, &lv_font_dejavu_16_persian_hebrew, 0);
#endif
}
/*****************************************************************************************************/
static void create_text_message_bubble(lv_obj_t *parent, const char *text, bool is_sent) {
    lv_obj_t *msg_bubble = lv_obj_create(parent);
    // The bubble itself shouldn't be scrollable, its parent chat_panel will handle scrolling.
    lv_obj_remove_flag(msg_bubble, LV_OBJ_FLAG_SCROLLABLE);

    // Set dynamic width (70% of parent to allow wrapping) and auto-height based on content.
    lv_obj_set_width(msg_bubble, LV_PCT(70));
    lv_obj_set_height(msg_bubble, LV_SIZE_CONTENT);
    // Add internal padding to the message bubble.
    lv_obj_set_style_pad_all(msg_bubble, 10, LV_PART_MAIN);
    // Apply rounded corners for a modern chat bubble look.
    lv_obj_set_style_radius(msg_bubble, 15, LV_PART_MAIN);

    // Apply specific styles based on whether the message was sent or received.
    if (is_sent) {
        // Align sent messages to the top-right of their parent container.
        lv_obj_set_align(msg_bubble, LV_ALIGN_TOP_RIGHT);
        // Use a distinct blue color for sent messages.
        lv_obj_set_style_bg_color(msg_bubble, lv_color_hex(0x007bff), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(msg_bubble, LV_OPA_100, LV_PART_MAIN);
        // White text for better contrast on blue background.
        lv_obj_set_style_text_color(msg_bubble, lv_color_white(), LV_PART_MAIN); // Corrected: lv_color_white()
        // Small right padding for alignment.
        lv_obj_set_style_pad_right(msg_bubble, 10, LV_PART_MAIN);
    } else {
        // Align received messages to the top-left.
        lv_obj_set_align(msg_bubble, LV_ALIGN_TOP_LEFT);
        // Use a light gray color for received messages.
        lv_obj_set_style_bg_color(msg_bubble, lv_color_hex(0xE0E0E0), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(msg_bubble, LV_OPA_100, LV_PART_MAIN);
        // Dark gray text for better contrast on light background.
        lv_obj_set_style_text_color(msg_bubble, lv_color_hex(0x333333), LV_PART_MAIN);
        // Small left padding for alignment.
        lv_obj_set_style_pad_left(msg_bubble, 10, LV_PART_MAIN);
    }

    // Create a label object inside the message bubble to display the text.
    lv_obj_t *label = lv_label_create(msg_bubble);
    lv_label_set_text(label, text);
    // Enable text wrapping so long messages fit within the bubble's width.
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    // Make the label take the full available width of its parent bubble.
    lv_obj_set_width(label, LV_PCT(100));
    // Align text within the bubble (top-left is standard for chat).
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);
}
static void create_voice_message_bubble(lv_obj_t *parent, const char *file_path, bool is_sent) {
    lv_obj_t *msg_bubble = lv_obj_create(parent);
    lv_obj_remove_flag(msg_bubble, LV_OBJ_FLAG_SCROLLABLE);

    // Voice messages are slightly narrower than text messages.
    lv_obj_set_width(msg_bubble, LV_PCT(80));
    lv_obj_set_height(msg_bubble, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(msg_bubble, 5, LV_PART_MAIN);
    lv_obj_set_style_radius(msg_bubble, 15, LV_PART_MAIN);

    // Apply specific styles based on whether the message was sent or received.
    if (is_sent) {
        lv_obj_set_align(msg_bubble, LV_ALIGN_TOP_RIGHT);
        lv_obj_set_style_bg_color(msg_bubble, lv_color_hex(0x007bff), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(msg_bubble, LV_OPA_100, LV_PART_MAIN);
        lv_obj_set_style_text_color(msg_bubble, lv_color_white(), LV_PART_MAIN); // Corrected: lv_color_white()
        lv_obj_set_style_pad_right(msg_bubble, 10, LV_PART_MAIN);
    } else {
        lv_obj_set_align(msg_bubble, LV_ALIGN_TOP_LEFT);
        lv_obj_set_style_bg_color(msg_bubble, lv_color_hex(0xE0E0E0), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(msg_bubble, LV_OPA_100, LV_PART_MAIN);
        lv_obj_set_style_text_color(msg_bubble, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_pad_left(msg_bubble, 10, LV_PART_MAIN);
    }

    // Use Flexbox for content within the voice message bubble to align the play button and text.
    lv_obj_set_flex_flow(msg_bubble, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(msg_bubble, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Create the Play Button itself.
    lv_obj_t *play_btn = lv_btn_create(msg_bubble);
    lv_obj_set_size(play_btn, 40, 40); // Fixed size for the round button.
    lv_obj_set_style_radius(play_btn, LV_RADIUS_CIRCLE, LV_PART_MAIN); // Make it perfectly round.
    lv_obj_set_style_bg_color(play_btn, lv_color_hex(0x28a745), LV_PART_MAIN); // Green color for play.
    lv_obj_set_style_bg_opa(play_btn, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_border_width(play_btn, 0, LV_PART_MAIN); // No border.
    lv_obj_set_style_pad_all(play_btn, 0, LV_PART_MAIN); // No internal padding for the button itself.

    // Create a label inside the button for the play icon.
    lv_obj_t *play_icon = lv_label_create(play_btn);
    lv_label_set_text(play_icon, LV_SYMBOL_PLAY); // Use LVGL's built-in play symbol.
    lv_obj_set_style_text_color(play_icon, lv_color_white(), LV_PART_MAIN); // Corrected: lv_color_white()
    lv_obj_center(play_icon); // Center the icon within the button.

    // Add an event callback to the play button.
    // The 'file_path' is passed as user data so the callback knows which audio file to play.
    lv_obj_add_event_cb(play_btn, voice_message_event_cb, LV_EVENT_CLICKED, (void *)file_path);

    // Create a simple text label next to the play button.
    lv_obj_t *label = lv_label_create(msg_bubble);
    lv_label_set_text(label, "Voice Message");
    lv_obj_set_width(label, LV_SIZE_CONTENT); // Width adjusts to content.
    lv_obj_set_style_pad_left(label, 5, LV_PART_MAIN); // Padding between icon and text.
    // Text color matches the bubble's text color.
    lv_obj_set_style_text_color(label, is_sent ? lv_color_white() : lv_color_white(), LV_PART_MAIN); // Corrected: lv_color_white()
}
/*****************************************************************************************************/

void voice_message_event_cb(lv_event_t * e) {
    // Retrieve the file path that was stored as user data during button creation.
    const char *file_path = (const char *)lv_event_get_user_data(e);
    if (file_path) {
        char command[256]; // Buffer to construct the aplay command.
        // Construct the command string.
        sprintf(command, "aplay -D plughw:3,0 %s", file_path);
        // Log the command for debugging purposes.
        LV_LOG_USER("Attempting to play voice message: %s", file_path);
        // Execute the command. This will block the UI briefly while the audio plays.
        // For non-blocking audio, you would need to use threads or a more advanced
        // audio library like SDL_mixer or portaudio.
        system(command);
    } else {
        // Log an error if the file path was not found.
        LV_LOG_ERROR("Voice message path is NULL for playback!");
    }
}

/**
 * @brief Adds a message to the global chat history array.
 * @param type The type of message (MSG_TYPE_TEXT or MSG_TYPE_VOICE).
 * @param is_sent True if the message was sent by the current user, false if received.
 * @param content_ptr Pointer to the content string (text or file path).
 */
static void add_message_to_history(MessageType type, bool is_sent, const char* content_ptr) {
    if (chat_history_count < MAX_CHAT_HISTORY_SIZE) {
        chat_history[chat_history_count].type = type;
        chat_history[chat_history_count].is_sent = is_sent;
        chat_history[chat_history_count].content_ptr = content_ptr;
        chat_history_count++;
        save_chat_history(); // AUTOMATIC SAVE: Save history immediately after adding a new message
    } else {
        LV_LOG_WARN("Chat history full. Cannot add more messages.");
    }
}
/**
 * @brief Clears the current chat panel and repopulates it from the chat_history array.
 * This ensures messages are displayed in chronological order and the view scrolls to the bottom.
 */
static void refresh_chat_display(void) {
    if (global_chat_panel_ptr == NULL) {
        LV_LOG_ERROR("global_chat_panel_ptr is NULL. Cannot refresh chat display.");
        return;
    }
    // IMPORTANT: Check if the pointer still points to a valid LVGL object
    if (!lv_obj_is_valid(global_chat_panel_ptr)) {
        LV_LOG_ERROR("global_chat_panel_ptr is invalid. Resetting and cannot refresh chat display.");
        global_chat_panel_ptr = NULL; // Reset to prevent further invalid access
        return;
    }

    // Clear all existing children (message bubbles) from the chat panel
    lv_obj_clean(global_chat_panel_ptr);

    // Repopulate the chat panel from the unified chat_history
    for (int i = 0; i < chat_history_count; ++i) {
        if (chat_history[i].type == MSG_TYPE_TEXT) {
            create_text_message_bubble(global_chat_panel_ptr, chat_history[i].content_ptr, chat_history[i].is_sent);
        } else if (chat_history[i].type == MSG_TYPE_VOICE) {
            create_voice_message_bubble(global_chat_panel_ptr, chat_history[i].content_ptr, chat_history[i].is_sent);
        }
    }

    // --- Fix for Scrolling to Bottom ---
    // Update layout to ensure all child objects are measured correctly before scrolling
    lv_obj_update_layout(global_chat_panel_ptr);
    lv_coord_t scroll_y = lv_obj_get_content_height(global_chat_panel_ptr) - lv_obj_get_height(global_chat_panel_ptr);
    if (scroll_y < 0) {
        scroll_y = 0;
    }
    lv_obj_scroll_to_y(global_chat_panel_ptr, scroll_y, LV_ANIM_ON);
    // --- End Fix ---
}



/************************************************************************************************/

/**
 * @brief Event callback for the voice recording button.
 * Changes button color to red, records voice for 10 seconds, saves to file,
 * adds path to array, and updates chat UI.
 * @param e Pointer to the event object.
 */
void record_button_event(lv_event_t * e) {

    if (next_record_file_num <= MAX_VOICE_MESSAGES) { // Check against MAX_VOICE_MESSAGES
        // Generate unique filename: /home/gp/send_recorde/sendX.wav
        sprintf(recorded_voice_paths_storage[next_record_file_num - 1], // Store in array based on 0-index
                "/home/gp/send_recorde/send%d.wav", next_record_file_num);
        const char *output_file_path = recorded_voice_paths_storage[next_record_file_num - 1];
        printf("Touch: %x\n", output_file_path);
        LV_LOG_USER("Recording voice to: %s for %d seconds...", output_file_path, RECORD_DURATION_SEC);

        char command[256 + 100]; // Buffer for the command string, increased size for arecord options
        // Construct the arecord command:
        // -D plughw:2,0: Specifies input input (microphone)
        // -f S16_LE: 16-bit signed, little-endian format
        // -r 44100: 44.1 kHz sample rate
        // -d 10: Record duration of 10 seconds
        sprintf(command, "arecord -D plughw:2,0 -f S16_LE -r 44100 -d %d %s",
                RECORD_DURATION_SEC, output_file_path);

        // Execute the command and capture its return status
        int system_ret = system(command);
        if (system_ret != 0) {
            LV_LOG_ERROR("arecord command failed with exit code: %d", system_ret);
            return; // Exit if recording failed
        }
        LV_LOG_USER("Recording finished successfully.");

        // 4. Add the new voice message to the unified chat history
        add_message_to_history(MSG_TYPE_VOICE, true, output_file_path); // true for sent voice

        // 5. Refresh the entire chat display to show the new message in chronological order
        //refresh_chat_display();

        next_record_file_num++; // Increment for the next recording
    } else {
        LV_LOG_WARN("Maximum number of recordings reached (%d). Cannot record more.", MAX_VOICE_MESSAGES);
    }
}
void save_chat_history() {
    FILE *fp = fopen(HISTORY_FILE_PATH, "w");
    if (fp == NULL) {
        LV_LOG_ERROR("Failed to open chat history file for writing: %s", HISTORY_FILE_PATH);
        return;
    }

    // Write the current state of indices first
    // Format: INDEXES|next_sent_text_index|next_received_text_index|next_record_file_num|next_received_voice_index
    fprintf(fp, "INDEXES|%d|%d|%d|%d\n",
            next_sent_text_index,
            next_received_text_index,
            next_record_file_num,
            next_received_voice_index);


    for (int i = 0; i < chat_history_count; ++i) {
        if (chat_history[i].type == MSG_TYPE_TEXT) {
            fprintf(fp, "TEXT|%d|%s\n", chat_history[i].is_sent, chat_history[i].content_ptr);
        } else if (chat_history[i].type == MSG_TYPE_VOICE) {
            fprintf(fp, "VOICE|%d|%s\n", chat_history[i].is_sent, chat_history[i].content_ptr);
        }
    }
    fclose(fp);
    // Removed the log here to avoid excessive logging on every message addition,
    // but kept it in the explicit save button handler.
    // LV_LOG_USER("Chat history saved!");
}

void load_chat_history() {
    FILE *fp = fopen(HISTORY_FILE_PATH, "r");
    if (fp == NULL) {
        LV_LOG_WARN("Chat history file not found or failed to open for reading: %s. Starting with empty chat.", HISTORY_FILE_PATH);
        return; // File doesn't exist yet, start with empty chat
    }

    // Reset chat history count before loading. Indices are loaded from file or default.
    chat_history_count = 0;

    char line[512]; // Buffer to read each line

    // Read the INDEXES line first
    if (fgets(line, sizeof(line), fp) != NULL) {
        line[strcspn(line, "\n")] = 0; // Remove newline
        char *token;
        char *rest = line;

        token = strtok_r(rest, "|", &rest); // Should be "INDEXES"
        if (token != NULL && strcmp(token, "INDEXES") == 0) {
            token = strtok_r(rest, "|", &rest);
            if (token) next_sent_text_index = atoi(token);
            token = strtok_r(rest, "|", &rest);
            if (token) next_received_text_index = atoi(token);
            token = strtok_r(rest, "|", &rest);
            if (token) next_record_file_num = atoi(token);
            token = strtok_r(rest, "|", &rest);
            if (token) next_received_voice_index = atoi(token);

            LV_LOG_USER("Loaded indices: sent=%d, received_text=%d, record_file=%d, received_voice=%d",
                        next_sent_text_index, next_received_text_index, next_record_file_num, next_received_voice_index);
        } else {
            LV_LOG_WARN("No INDEXES line found or malformed. Resetting indices to default (0 for text, 1 for record file).");
            // If no INDEXES line, reset to default and proceed to read chat history
            next_sent_text_index = 0;
            next_received_text_index = 0;
            next_record_file_num = 1; // Default starting number for new recordings
            next_received_voice_index = 0;
            // IMPORTANT: If no INDEXES line, we need to rewind or re-open file to read chat history
            fseek(fp, 0, SEEK_SET); // Rewind to beginning to read messages if the first line wasn't INDEXES
        }
    }


    // Now load chat messages
    while (fgets(line, sizeof(line), fp) != NULL) {
        // Remove newline character if present
        line[strcspn(line, "\n")] = 0;

        // Skip the INDEXES line if we encountered it already and are at the beginning
        // This handles cases where the file only contains INDEXES line, or if fseek(fp,0,SEEK_SET) was called
        if (strncmp(line, "INDEXES|", 8) == 0 && chat_history_count == 0) {
            continue; // Skip the already processed INDEXES line
        }

        char *token;
        char *rest = line;

        // Parse Type
        token = strtok_r(rest, "|", &rest);
        if (token == NULL) { LV_LOG_ERROR("Malformed history line (type): %s", line); continue; }
        MessageType type = (strcmp(token, "TEXT") == 0) ? MSG_TYPE_TEXT : MSG_TYPE_VOICE;

        // Parse is_sent
        token = strtok_r(rest, "|", &rest);
        if (token == NULL) { LV_LOG_ERROR("Malformed history line (is_sent): %s", line); continue; }
        bool is_sent = (strcmp(token, "1") == 0);

        // Parse Content
        token = strtok_r(rest, "|", &rest); // The rest of the line is the content
        if (token == NULL) { LV_LOG_ERROR("Malformed history line (content): %s", line); continue; }
        const char *content_ptr = NULL;

        if (chat_history_count < MAX_CHAT_HISTORY_SIZE) {
            if (type == MSG_TYPE_TEXT) {
                if (is_sent) {
                    // Populate sent_text_storage at the index pointed by chat_history_count
                    strcpy(sent_text_storage[chat_history_count], token);
                    content_ptr = sent_text_storage[chat_history_count];
                } else { // Received Text
                    // Populate received_text_storage at the index pointed by chat_history_count
                    strcpy(received_text_storage[chat_history_count], token);
                    content_ptr = received_text_storage[chat_history_count];
                }
            } else { // Voice Message
                if (is_sent) { // Recorded by this device
                    // Populate recorded_voice_paths_storage at the index pointed by chat_history_count
                    strcpy(recorded_voice_paths_storage[chat_history_count], token);
                    content_ptr = recorded_voice_paths_storage[chat_history_count];
                } else { // Received Voice
                    // Populate received_voice_paths_storage at the index pointed by chat_history_count
                    strcpy(received_voice_paths_storage[chat_history_count], token);
                    content_ptr = received_voice_paths_storage[chat_history_count];
                }
            }
            if (content_ptr != NULL) {
                // Manually add to chat_history without calling add_message_to_history to avoid recursive save
                chat_history[chat_history_count].type = type;
                chat_history[chat_history_count].is_sent = is_sent;
                chat_history[chat_history_count].content_ptr = content_ptr;
                chat_history_count++;
            }
        } else {
            LV_LOG_WARN("Chat history or storage full when loading. Some messages may not be loaded.");
        }
    }
    fclose(fp);
    LV_LOG_USER("Chat history loaded. Total messages: %d", chat_history_count);
}

/**
 * @brief Event callback for the "Send" button in UI_2.
 * Gets text from the text area, stores it, adds to chat history, clears text area,
 * and refreshes UI_1.
 * @param e Pointer to the event object.
 */
void send_text_message_event_cb(lv_event_t * e) {
    if (ui2_text_area_ptr != NULL) {
        const char *text = lv_textarea_get_text(ui2_text_area_ptr);
        if (strlen(text) > 0) {
            if (next_sent_text_index < MAX_TEXT_MESSAGES) {
                // Copy text to our dynamic storage
                strcpy(sent_text_storage[next_sent_text_index], text);
                const char *new_text_ptr = sent_text_storage[next_sent_text_index];

                // Add to chat history (as a sent message)
                add_message_to_history(MSG_TYPE_TEXT, true, new_text_ptr);

                // Clear the text area
                lv_textarea_set_text(ui2_text_area_ptr, "");

                next_sent_text_index++;
            } else {
                LV_LOG_WARN("Maximum sent text messages reached. Cannot store more.");
            }
        }
    }
    // No direct refresh of UI_1 here, it will be refreshed when back_to_UI_1_event_cb is called.
}


void back_to_UI_1_event_cb(lv_event_t * e) {
    lv_screen_load(lv_display_get_screen_active(lv_display_get_default())); // Go back to the screen creat_UI_1 creates

    // Re-initialize UI_1 (important to recreate elements from scratch to reflect updated history)
    // A more efficient way for large apps would be to update existing elements if possible,
    // but for simplicity and guaranteeing order, re-creating is robust.
    creat_UI_1(); // Call creat_UI_1 again to rebuild the screen with updated history

    //refresh_chat_display(); // already don in create_UI_1
}

void lvgl_init_display(){

   bcm2835_gpio_write(ILI9341_CS, LOW);
    /* Create LVGL display object */
   my_disp = lv_lcd_generic_mipi_create(LCD_H_RES, LCD_V_RES,  LV_LCD_FLAG_MIRROR_X, my_lcd_send_cmd, my_lcd_send_color);
    //lv_display_set_rotation(my_disp, LV_DISPLAY_ROTATION_90);

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

void create_hello_world_ui() {
    lv_obj_t *scr = lv_display_get_screen_active(my_disp);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x003a57), LV_PART_MAIN);
    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "Hello, World!");
    lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 0, 0);
}
static void custom_delay(uint32_t ms)
{
   // printf("call custom delay function/n");
    usleep(ms * 1000);  // Convert milliseconds to microseconds
}


// Touchscreen read function for LVGL

static void touch_read(lv_indev_t *drv, lv_indev_data_t *data) {
    uint16_t x=0;
    uint16_t y=0;
    if (XPT2046_GetTouch(&x,&y)) {
        printf("Touch: %d, %d\n", x, y); // for checking only
        data->point.x = x;
        data->point.y = LCD_V_RES - 1 - y; //oringin at top left (0,0)
        data->state = LV_INDEV_STATE_PRESSED;
        printf("Button with %d and %d are pressed \n",x,y); // 👈 Add this for checking only
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
       // printf("Buttons  arn't with %d and %d are released \n",x,y); // 👈 Add this >
    }
}
/*                        The main function  */
int main() {


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
    load_chat_history();// --- Load chat history at startup ---
	creat_UI_1();
    lv_indev_t *touch_indev = lv_indev_create();
    lv_indev_set_type(touch_indev,LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(touch_indev, touch_read);


   /**********************************************************/
    //create_corner_squares();
   /**********************************************************/
   XPT2046_Init();
	const uint32_t TICK_PERIOD = 5;  // 5 ms tick
    while (1) {
		lv_tick_inc(TICK_PERIOD);
        lv_timer_handler();
        custom_delay(TICK_PERIOD*2);

    }

    return 0;
}


void ili9341_reset() {
   bcm2835_gpio_write(ILI9341_RST, LOW);
   bcm2835_delay(50);
   bcm2835_gpio_write(ILI9341_RST, HIGH);
   bcm2835_delay(50);
}
void lvgl_clear_screen(void) {
    lv_obj_t *scr = lv_display_get_screen_active(my_disp);
    lv_obj_clean(scr);  // delete all children (buttons, labels…)
    // optionally reset background color
    lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN);
    lv_timer_handler(); // let LVGL flush the empty screen
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

static void ta_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target_obj(e);
    lv_obj_t * kb = (lv_obj_t *)lv_event_get_user_data(e);
    if(code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(kb, ta);
        lv_obj_remove_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }

    if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
}
