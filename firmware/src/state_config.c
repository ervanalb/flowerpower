#include "state_config.h"

#include <FreeRTOS.h>
#include <task.h>
#include <stddef.h>

#include "comms.h"
#include "hal.h"

struct state state = STATE_DEFAULTS;

#define TASK_STACK_SIZE 160
#define NOTIFY_UPDATE 0

static TaskHandle_t publish_state_task_handle;

static const char *relay_mode_to_string(uint8_t mode);

static void publish_state_task(void *pvParameters) {
    (void)pvParameters;
    static struct state last_state;
    static struct config last_config;

    int counter = 0;
    for (;;) {
        bool send_all = false;
        counter++;
        if (counter >= 100) {
            counter = 0;
            send_all = true;
        }

        for (int i=0; i<N_RELAYS; i++) {
            if (send_all || state.relay[i].state != last_state.relay[i].state) {
                comms_printf("relay/%d/state %d\n", i + 1, state.relay[i].state);
            }
            if (send_all || config.relay[i].mode != last_config.relay[i].mode) {
                comms_printf("relay/%d/mode %s\n", i + 1, relay_mode_to_string(config.relay[i].mode));
            }
            if (send_all || config.relay[i].on_hour != last_config.relay[i].on_hour) {
                comms_printf("relay/%d/on/hour %d\n", i + 1, config.relay[i].on_hour);
            }
            if (send_all || config.relay[i].off_hour != last_config.relay[i].off_hour) {
                comms_printf("relay/%d/off/hour %d\n", i + 1, config.relay[i].off_hour);
            }
        }

        if (send_all) {
            struct datetime dt;
            hal_time_get(&dt);
            comms_printf("time 20%02d-%02d-%02dT%02d:%02d:%02d\n", dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
        }

        ulTaskNotifyTakeIndexed(NOTIFY_UPDATE, pdTRUE, 100 / portTICK_PERIOD_MS);
    }
}

void state_init(void) {
    configASSERT(xTaskCreate(publish_state_task, "publish_state", TASK_STACK_SIZE, NULL, 1, &publish_state_task_handle) == pdTRUE);
}

void state_update() {
    xTaskNotifyGiveIndexed(publish_state_task_handle, NOTIFY_UPDATE);
}

static const char *relay_mode_to_string(uint8_t mode) {
    switch (mode) {
        case RELAY_MODE_RAW:
            return "RAW";
        case RELAY_MODE_SCHEDULE:
            return "SCHEDULE";
        default:
            return "UNKNOWN";
    }
}
