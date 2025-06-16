#ifndef touch_h
#define touch_h

#include <stdint.h>
#include <stdbool.h>


#define SPI_SCK   RPI_GPIO_P1_23   // GPIO 11 (Pin 23, SPI0_SCLK)
#define SPI_MOSI  RPI_GPIO_P1_19   // GPIO 10 (Pin 19, SPI0_MOSI)
#define SPI_MISO  RPI_GPIO_P1_21   // GPIO 9  (Pin 21, SPI0_MISO)
#define TOUCH_CS        RPI_GPIO_P1_26  // Chip Select Pin
#define TOUCH_IRQ       RPI_GPIO_P1_11  // Interrupt Pin

#define CMD_READ_X      0xD0  // Command to read X coordinate
#define CMD_READ_Y      0x90  // Command to read Y coordinate
#define CMD_READ_Z1     0xB0  // Command to read Z1 (pressure)
#define CMD_READ_Z2     0xC0  // Command to read Z2 (pressure)

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// Define the raw ADC min/max values (found by calibration)
#define X_MIN 200
#define X_MAX 3800
#define Y_MIN 200
#define Y_MAX 3800


void XPT2046_Init(void);
static uint16_t XPT2046_ReadData(uint8_t command);
bool XPT2046_TouchPressed(void);
bool XPT2046_GetTouch(uint16_t *x, uint16_t *y);
uint16_t XPT2046_ReadPressure(void);
void XPT2046_Close(void);

#endif
