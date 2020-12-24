#include <bsp.h>
#include <gpio.h>
#include <fpioa.h>
#include <gpiohs.h>
#include <sysctl.h>

#include <pins.h>
#include <NT35310.hpp>
#include <Fluke8050A.hpp>

/*
 * Core utilization:
 * 
 * Core 0:
 *   Initial GPIO initialization
 *   8050A hardware interaction
 *   Necessary numerical conversion
 * 
 * Core 1:
 *   LCD control
 *   8050A value display
 */

static int core1_function(void *ctx) {
    NT35310 lcd(LCD_SPI_DEV, SPI_CHIP_SELECT_0,
                LCD_GPIOHS_RST, LCD_GPIOHS_DC,
                LCD_WIDTH, LCD_HEIGHT);
    
    (void)ctx;
    uint64_t core = current_coreid();
    printf("Core %ld Hello world\r\n", core);

    lcd.init();
    lcd.fill(RGB(0,0,0));

    while(1) {
        msleep(250);
        gpio_set_pin(LED_GPIO_G, GPIO_PV_HIGH);
        lcd.fillArea(RGB(255,0,0), 0, 0, 100, 100);
        msleep(250);
        gpio_set_pin(LED_GPIO_G, GPIO_PV_LOW);
        lcd.fillArea(RGB(0,255,0), 50, 75, 150, 175);
        msleep(250);
        lcd.fillArea(RGB(0,0,255), 100, 150, 200, 250);
    }
}

static void k210_init(void) {
    sysctl_pll_set_freq(SYSCTL_PLL0, 400000000);
    gpio_init();

    /* Initialize RGB status LED */
    fpioa_set_function(LED_PIN_R, (fpioa_function_t)(FUNC_GPIO0 + LED_GPIO_R));
    fpioa_set_function(LED_PIN_G, (fpioa_function_t)(FUNC_GPIO0 + LED_GPIO_G));
    fpioa_set_function(LED_PIN_B, (fpioa_function_t)(FUNC_GPIO0 + LED_GPIO_B));
    
    gpio_set_drive_mode(LED_GPIO_R, GPIO_DM_OUTPUT);
    gpio_set_drive_mode(LED_GPIO_G, GPIO_DM_OUTPUT);
    gpio_set_drive_mode(LED_GPIO_B, GPIO_DM_OUTPUT);

    gpio_set_pin(LED_GPIO_R, GPIO_PV_HIGH);
    gpio_set_pin(LED_GPIO_G, GPIO_PV_HIGH);
    gpio_set_pin(LED_GPIO_B, GPIO_PV_HIGH);
    
    /* Initialize LCD SPI pins */
    fpioa_set_function(LCD_PIN_CS,  FUNC_SPI0_SS0);
    fpioa_set_function(LCD_PIN_WR,  FUNC_SPI0_SCLK);
    
    fpioa_set_function(LCD_PIN_RST, (fpioa_function_t)(FUNC_GPIOHS0 + LCD_GPIOHS_RST));
    fpioa_set_function(LCD_PIN_DC,  (fpioa_function_t)(FUNC_GPIOHS0 + LCD_GPIOHS_DC));

    sysctl_set_spi0_dvp_data(1);
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);

    /* Initialize 8050A pins */
    fpioa_set_function(FLUKE8050_PIN_DP,  (fpioa_function_t)(FUNC_GPIOHS0 + FLUKE8050_GPIOHS_DP));
    fpioa_set_function(FLUKE8050_PIN_HV,  (fpioa_function_t)(FUNC_GPIOHS0 + FLUKE8050_GPIOHS_HV));
    fpioa_set_function(FLUKE8050_PIN_W,   (fpioa_function_t)(FUNC_GPIOHS0 + FLUKE8050_GPIOHS_W));
    fpioa_set_function(FLUKE8050_PIN_X,   (fpioa_function_t)(FUNC_GPIOHS0 + FLUKE8050_GPIOHS_X));
    fpioa_set_function(FLUKE8050_PIN_Y,   (fpioa_function_t)(FUNC_GPIOHS0 + FLUKE8050_GPIOHS_Y));
    fpioa_set_function(FLUKE8050_PIN_Z,   (fpioa_function_t)(FUNC_GPIOHS0 + FLUKE8050_GPIOHS_Z));
    fpioa_set_function(FLUKE8050_PIN_ST0, (fpioa_function_t)(FUNC_GPIOHS0 + FLUKE8050_GPIOHS_ST0));
    fpioa_set_function(FLUKE8050_PIN_ST1, (fpioa_function_t)(FUNC_GPIOHS0 + FLUKE8050_GPIOHS_ST1));
    fpioa_set_function(FLUKE8050_PIN_ST2, (fpioa_function_t)(FUNC_GPIOHS0 + FLUKE8050_GPIOHS_ST2));
    fpioa_set_function(FLUKE8050_PIN_ST3, (fpioa_function_t)(FUNC_GPIOHS0 + FLUKE8050_GPIOHS_ST3));
    fpioa_set_function(FLUKE8050_PIN_ST4, (fpioa_function_t)(FUNC_GPIOHS0 + FLUKE8050_GPIOHS_ST4));
}

int main(void)
{
    k210_init();

    uint64_t core = current_coreid();
    printf("Core %ld Hello world\r\n", core);
    register_core1(core1_function, NULL);

    fluke_8050a_pins_t flukePins = {
        .dp  = FLUKE8050_GPIOHS_DP,
        .hv  = FLUKE8050_GPIOHS_HV,
        .w   = FLUKE8050_GPIOHS_W,
        .x   = FLUKE8050_GPIOHS_X,
        .y   = FLUKE8050_GPIOHS_Y,
        .z   = FLUKE8050_GPIOHS_Z,
        .st0 = FLUKE8050_GPIOHS_ST0,
        .st1 = FLUKE8050_GPIOHS_ST1,
        .st2 = FLUKE8050_GPIOHS_ST2,
        .st3 = FLUKE8050_GPIOHS_ST3,
        .st4 = FLUKE8050_GPIOHS_ST4
    };

    Fluke8050A fluke(&flukePins);

    fluke.init();

    while(1) {
        msleep(500);
        gpio_set_pin(LED_GPIO_R, GPIO_PV_HIGH);
        msleep(500);
        gpio_set_pin(LED_GPIO_R, GPIO_PV_LOW);

        fluke.debug();
    }

    return 0;
}
