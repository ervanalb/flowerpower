#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/systick.h>
#include <FreeRTOS.h>
#include <task.h>

#include "hal.h"
#include "comms.h"

static void heartbeat(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        gpio_set(GPIOB, GPIO4);
        comms_send("Blink On\n", 10);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_clear(GPIOB, GPIO4);
        comms_send("Blink Off\n", 11);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void motor_move(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        gpio_set(GPIOA, GPIO4);
        vTaskDelay(1800 / portTICK_PERIOD_MS);
        gpio_clear(GPIOA, GPIO4);
        vTaskDelay(1800 / portTICK_PERIOD_MS);
        gpio_set(GPIOA, GPIO5);
        vTaskDelay(1800 / portTICK_PERIOD_MS);
        gpio_clear(GPIOA, GPIO5);
        vTaskDelay(1800 / portTICK_PERIOD_MS);
    }
}

int main(void) {
    hal_init();

    comms_init();

    xTaskCreate(heartbeat, "heartbeat", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(motor_move, "motor_move", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    vTaskStartScheduler();

    for (;;);
}
