#include <bsp.h>
#include <gpio.h>
#include <fpioa.h>
#include <sysctl.h>

#define LED_PIN_R 13
#define LED_PIN_G 12
#define LED_PIN_B 14

#define LED_GPIO_R 0
#define LED_GPIO_G 1
#define LED_GPIO_B 2

static int core1_function(void *ctx)
{
    (void)ctx;
    uint64_t core = current_coreid();
    printf("Core %ld Hello world\r\n", core);
    while(1) {
        msleep(1000);
        gpio_set_pin(LED_GPIO_G, GPIO_PV_HIGH);
        msleep(1000);
        gpio_set_pin(LED_GPIO_G, GPIO_PV_LOW);
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
}

int main(void)
{
    k210_init();

    uint64_t core = current_coreid();
    printf("Core %ld Hello world\r\n", core);
    register_core1(core1_function, NULL);

    while(1) {
        msleep(500);
        gpio_set_pin(LED_GPIO_R, GPIO_PV_HIGH);
        msleep(500);
        gpio_set_pin(LED_GPIO_R, GPIO_PV_LOW);
    }

    return 0;
}
