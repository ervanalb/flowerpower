#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/systick.h>
#include <FreeRTOS.h>
#include <task.h>

#include "hal.h"

static void blink(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        gpio_set(GPIOB, GPIO1);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        gpio_clear(GPIOB, GPIO1);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

int main(void) {
    hal_init();

    xTaskCreate(blink, "b", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    vTaskStartScheduler();

    for (;;);
}
