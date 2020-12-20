#ifndef PINS_H
#define PINS_H

#include <spi.h>

/* RGB LED */
#define LED_PIN_R 13
#define LED_PIN_G 12
#define LED_PIN_B 14

#define LED_GPIO_R 0
#define LED_GPIO_G 1
#define LED_GPIO_B 2

/* LCD SPI */
#define LCD_PIN_CS  36
#define LCD_PIN_RST 37
#define LCD_PIN_DC  38
#define LCD_PIN_WR  39
#define LCD_SPI_DEV SPI_DEVICE_0

#define LCD_GPIOHS_CS  0
#define LCD_GPIOHS_RST 1
#define LCD_GPIOHS_DC  2
#define LCD_GPIOHS_WR  3

#define LCD_WIDTH  240
#define LCD_HEIGHT 320

#endif