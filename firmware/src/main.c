#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/systick.h>
#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>

#include "hal.h"
#include "comms.h"
#include "heartbeat.h"
#include "state.h"

static char clock_str[30];

//struct datetime dt;
//hal_time_get(&dt);
//int n = snprintf(clock_str, sizeof (clock_str), "Date: 20%02u-%02u-%02u %02u:%02u:%02u\n", dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
//configASSERT (n >= 0 && (unsigned)n <= sizeof (clock_str));
//comms_send(clock_str, n);

static void motor_move(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        hal_relay_on(1);
        vTaskDelay(1800 / portTICK_PERIOD_MS);
        hal_relay_off(1);
        vTaskDelay(1800 / portTICK_PERIOD_MS);
        hal_relay_on(2);
        vTaskDelay(1800 / portTICK_PERIOD_MS);
        hal_relay_off(2);
        vTaskDelay(1800 / portTICK_PERIOD_MS);
    }
}

int main(void) {
    hal_init();
    comms_init();
    heartbeat_init();

    //const struct datetime dt = {
    //    .second = 0,
    //    .minute = 48,
    //    .hour = 15,
    //    .day = 17,
    //    .month = 10,
    //    .year = 20,
    //    .weekday = 7,
    //};
    //rtc_set(&dt);

    state_init();

    //xTaskCreate(heartbeat, "heartbeat", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(motor_move, "motor_move", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    vTaskStartScheduler();

    for (;;);
}
