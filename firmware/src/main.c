#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/systick.h>
#include <stddef.h>
#include <FreeRTOS.h>
#include <task.h>

void vApplicationMallocFailedHook(void);

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
    rcc_clock_setup_in_hsi_out_48mhz();
    rcc_periph_clock_enable(RCC_GPIOB);

    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO1);

    xTaskCreate(blink, "b", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    vTaskStartScheduler();

    for (;;);
}

void vApplicationMallocFailedHook(void) {
    for (;;);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void) xTask;
    (void) pcTaskName;

    for (;;);
}
