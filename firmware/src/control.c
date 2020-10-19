#include "control.h"

#include <FreeRTOS.h>
#include <task.h>
#include <stddef.h>

#include "state_config.h"
#include "hal.h"

#define TASK_STACK_SIZE 60

static void relays_schedule(void) {
    struct datetime dt;
    hal_time_get(&dt);

    for (size_t i = 0; i < N_RELAYS; i++) {
        if (config.relay[i].mode == RELAY_MODE_SCHEDULE) {
            uint8_t on_hour = config.relay[i].on_hour;
            uint8_t off_hour = config.relay[i].off_hour;
            uint8_t hour = dt.hour;

            if (off_hour >= on_hour) {
                state.relay[i].state = hour >= on_hour && hour < off_hour;
            } else {
                state.relay[i].state = hour >= on_hour || hour < off_hour;
            }
        }
    }
}

static void relays(void) {
    for (size_t i = 0; i < N_RELAYS; i++) {
        if (state.relay[i].state) {
            hal_relay_on(i + 1);
        } else {
            hal_relay_off(i + 1);
        }
    }
}

static void control_loop(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        relays_schedule();
        relays();

        state_update();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void control_init(void) {
    configASSERT(xTaskCreate(control_loop, "control_loop", TASK_STACK_SIZE, NULL, 1, NULL) == pdTRUE);
}
