#include "xpt2046.h"
#include <bcm2835.h>
#include <stdio.h>

// ** Initialize the XPT2046 touchscreen **
void XPT2046_Init(void) {
    if (!bcm2835_init()) {
        printf("BCM2835 init failed!\n");
        return;
    }
    if (!bcm2835_spi_begin()) {
        printf("SPI init failed!\n");
        return;
    }

    bcm2835_gpio_fsel(TOUCH_CS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(TOUCH_IRQ, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_write(TOUCH_CS, HIGH); // Deselect

    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64); // Adjust speed
}

// ** Read SPI Data **
static uint16_t XPT2046_ReadData(uint8_t command) {
    bcm2835_gpio_write(TOUCH_CS, LOW);
    bcm2835_spi_transfer(command);  
    bcm2835_delayMicroseconds(10);
    
    uint16_t data = bcm2835_spi_transfer(0x00) << 8;
    data |= bcm2835_spi_transfer(0x00);
    
    bcm2835_gpio_write(TOUCH_CS, HIGH);
    return (data >> 3) & 0x0FFF; // Ensure 12-bit output
}

// ** Check if Touch is Pressed **
bool XPT2046_TouchPressed(void) {
    return (bcm2835_gpio_lev(TOUCH_IRQ) == LOW); // LOW when pressed
}

// ** Get Touch Coordinates **
bool XPT2046_GetTouch(uint16_t *x, uint16_t *y) {
    if (!XPT2046_TouchPressed())
		return false;

    uint32_t raw_x = 0, raw_y = 0;
    const int samples = 5; // Sample multiple times for stability

    // Read multiple samples to filter noise
    for (int i = 0; i < samples; i++) {
        raw_x += XPT2046_ReadData(CMD_READ_X);
        raw_y += XPT2046_ReadData(CMD_READ_Y);
        bcm2835_delayMicroseconds(10);
    }

    // Average the readings
    raw_x /= samples;
    raw_y /= samples;

    // Apply calibration & map to screen coordinates
    *x = (raw_x - X_MIN) * SCREEN_WIDTH / (X_MAX - X_MIN);
    *y = (raw_y - Y_MIN) * SCREEN_HEIGHT / (Y_MAX - Y_MIN);

    // Optional: Ensure the coordinates stay within screen bounds
    if (*x >= SCREEN_WIDTH) *x = SCREEN_WIDTH - 1;
    if (*y >= SCREEN_HEIGHT) *y = SCREEN_HEIGHT - 1;
	while (XPT2046_TouchPressed())
	{
		
	}

    return true;
}
// ** Read Touch Pressure **
uint16_t XPT2046_ReadPressure(void) {
    uint16_t z1 = XPT2046_ReadData(CMD_READ_Z1);
    uint16_t z2 = XPT2046_ReadData(CMD_READ_Z2);
    return z1 - z2;
}

// ** Close SPI Communication **
void XPT2046_Close(void) {
    bcm2835_spi_end();
    bcm2835_close();
}
