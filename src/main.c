#include <bcm2835.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "lvgl/lvgl.h"
#include "xpt2046.h"
#include "message.h"
#include "encryption.h"
#include "transfer.h"
#include "ui.h"









void custom_delay(uint32_t ms)
{
    usleep(ms * 1000);  // Convert milliseconds to microseconds
}

// define Communication Thread
void *communication_thread(void *arg) {
    while(1) {
        // Handle Reciveing Messages
        char message_type;
        char filename[FILENAME_SIZE];
        // Recive message type and filename
        receive_string(&message_type, filename);
    }

    return NULL;
}
void Start_Communication_Thread() {
    pthread_t Com_thread;
    if (pthread_create(&Com_thread, NULL, communication_thread, NULL) != 0) {
        fprintf(stderr, "Error creating communication thread\n");
        return;
    }
    pthread_detach(Com_thread); // Detach the thread to avoid memory leaks
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

    // Initialize the communication
    Communication_Setup();
    Communication_inti(SENDER,receiver_ip);


    // Start the communication thread
    Start_Communication_Thread();
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







