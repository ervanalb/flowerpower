#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/systick.h>
#include <FreeRTOS.h>
#include <task.h>

#include "hal.h"
#include "usb.h"

static void comms(void *pvParameters) {
    (void)pvParameters;
    static uint8_t buff[100];
    for (;;) {
        size_t rx_len = usb_receive(buff, 100);
        usb_transmit(buff, rx_len);
    }
}

static void blink(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        gpio_set(GPIOB, GPIO4);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        gpio_clear(GPIOB, GPIO4);
        vTaskDelay(200 / portTICK_PERIOD_MS);
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

    //xTaskCreate(blink, "b", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(comms, "c", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(motor_move, "m", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    vTaskStartScheduler();

    for (;;);
}
