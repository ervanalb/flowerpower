#include "control.h"

#include <FreeRTOS.h>
#include <task.h>
#include <stddef.h>

#include "state_config.h"
#include "hal.h"

#define TASK_STACK_SIZE 100

static void relays_schedule(void) {
    struct datetime dt;
    hal_time_get(&dt);

    for (size_t i = 0; i < N_RELAYS; i++) {
        if (state.relay[i].mode == RELAY_MODE_SCHEDULE) {
            uint8_t on_hour = state.relay[i].on_hour;
            uint8_t off_hour = state.relay[i].off_hour;
            uint8_t hour = dt.hour;

            state_semaphore_take();
            if (off_hour >= on_hour) {
                state.relay[i].state = hour >= on_hour && hour < off_hour;
            } else {
                state.relay[i].state = hour >= on_hour || hour < off_hour;
            }
            state_semaphore_give();
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

static void pots(void) {
    state.overflow = hal_sense_overflow();
    for (size_t i = 0; i < N_POTS; i++) {
        state.pot[i].sense = hal_sense_full(i + 1);
        if (state.pot[i].sense && state.pot[i].pump > 0) {
            state.pot[i].pump = 0;
        }
        if (state.overflow) {
            state.pot[i].pump = 0;
        }
        if (state.pot[i].pump == 1) {
            hal_pump_flood(i + 1);
        } else if (state.pot[i].pump == -1) {
            hal_pump_drain(i + 1);
        } else {
            hal_pump_off(i + 1);
        }
    }
}

static void control_loop(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        relays_schedule();
        relays();

        pots();

        state_update();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void control_init(void) {
    configASSERT(xTaskCreate(control_loop, "control_loop", TASK_STACK_SIZE, NULL, 1, NULL) == pdTRUE);
}
