#include "state_config.h"

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <stddef.h>
#include <stdio.h>

#include "comms.h"
#include "hal.h"

struct state state = STATE_DEFAULTS;

#define TASK_STACK_SIZE 160
#define NOTIFY_UPDATE 0

static TaskHandle_t publish_state_task_handle;

static const char *relay_mode_to_string(uint8_t mode);

// This semaphore is probably not required when preemption = 0
// but might as well be a little extra safe
// (or if we ever want to turn on preemption)
static SemaphoreHandle_t state_semaphore = NULL;

static struct state last_state;

static void load_state_from_config(void) {
    state_semaphore_take();
    for (int i=0; i<N_RELAYS; i++) {
        state.relay[i].mode = config.relay[i].mode;
        state.relay[i].on_hour = config.relay[i].on_hour;
        state.relay[i].off_hour = config.relay[i].off_hour;
    }
    state_semaphore_give();
}

static void maybe_write_config(void) {
    state_semaphore_take();
    for (int i=0; i<N_RELAYS; i++) {
        config.relay[i].mode = state.relay[i].mode;
        config.relay[i].on_hour = state.relay[i].on_hour;
        config.relay[i].off_hour = state.relay[i].off_hour;
    }
    state_semaphore_give();
    // This function will only issue a flash write if there have been changes.
    hal_update_config();
}

static void publish_state_task(void *pvParameters) {
    (void)pvParameters;

    const TickType_t publish_period = pdMS_TO_TICKS(10000);

    load_state_from_config();

    TickType_t last_send_all = 0;

    int counter = 0;
    for (;;) {
        bool send_all = false;
        TickType_t elapsed_time = xTaskGetTickCount() - last_send_all;
        send_all = (elapsed_time > publish_period);

        state_semaphore_take();

        for (int i=0; i<N_RELAYS; i++) {
            if (send_all || state.relay[i].state != last_state.relay[i].state) {
                comms_printf("/relay/%d/state %d\n", i + 1, state.relay[i].state);
                last_state.relay[i].state = state.relay[i].state;
            }
            if (send_all || state.relay[i].mode != last_state.relay[i].mode) {
                comms_printf("/relay/%d/mode %s\n", i + 1, relay_mode_to_string(state.relay[i].mode));
                last_state.relay[i].mode = state.relay[i].mode;
            }
            if (send_all || state.relay[i].on_hour != last_state.relay[i].on_hour) {
                comms_printf("/relay/%d/on/hour %d\n", i + 1, state.relay[i].on_hour);
                last_state.relay[i].on_hour = state.relay[i].on_hour;
            }
            if (send_all || state.relay[i].off_hour != last_state.relay[i].off_hour) {
                comms_printf("/relay/%d/off/hour %d\n", i + 1, state.relay[i].off_hour);
                last_state.relay[i].off_hour = state.relay[i].off_hour;
            }
        }

        state_semaphore_give();

        if (send_all) {
            {
                struct datetime dt;
                hal_time_get(&dt);
                comms_printf("/time 20%02d-%02d-%02dT%02d:%02d:%02d\n", dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
            }
            {
                long free_memory = xPortGetFreeHeapSize();
                comms_printf("/sys/free_memory %ld\n", free_memory);
            }

            last_send_all = xTaskGetTickCount();
            elapsed_time = 0;

            // Only write config every 10 seconds or so.
            maybe_write_config();
        }

        TickType_t wait_time = publish_period - elapsed_time;
        configASSERT (wait_time <= publish_period);
        ulTaskNotifyTakeIndexed(NOTIFY_UPDATE, pdTRUE, wait_time);
    }
}

void state_init(void) {
    configASSERT(xTaskCreate(publish_state_task, "publish_state", TASK_STACK_SIZE, NULL, 1, &publish_state_task_handle) == pdTRUE);
    state_semaphore = xSemaphoreCreateMutex();
    configASSERT(state_semaphore);
}

void state_update() {
    bool updated = false;

    state_semaphore_take();

    for (size_t i=0; i<N_RELAYS; i++) {
        if (state.relay[i].state != last_state.relay[i].state) {
            updated = true;
            break;
        }
        if (state.relay[i].mode != last_state.relay[i].mode) {
            updated = true;
            break;
        }
        if (state.relay[i].on_hour != last_state.relay[i].on_hour) {
            updated = true;
            break;
        }
        if (state.relay[i].off_hour != last_state.relay[i].off_hour) {
            updated = true;
            break;
        }
    }

    state_semaphore_give();

    if (updated) {
        xTaskNotifyGiveIndexed(publish_state_task_handle, NOTIFY_UPDATE);
    }
}

bool state_parse(const char *message, size_t length) {
    {
        int n;
        int i;
        int value;
        int matched = sscanf(message, "/relay/%d/state/set %d\n%n", &i, &value, &n);
        if (matched == 2 && n > 0 && (size_t)n == length) {
            if (i > 0 && i <= N_RELAYS && value >= 0 && value <= 1) {
                state_semaphore_take();
                state.relay[i - 1].state = value;
                state.relay[i - 1].mode = RELAY_MODE_RAW;
                state_semaphore_give();
                state_update();
            }
            return true;
        }
    }
    {
        int n;
        int i;
        int value;
        int matched = sscanf(message, "/relay/%d/mode/set %d\n%n", &i, &value, &n);
        if (matched == 2 && n > 0 && (size_t)n == length) {
            if (i > 0 && i <= N_RELAYS && value >= 0 && value <= 1) {
                state_semaphore_take();
                state.relay[i - 1].mode = value;
                state_semaphore_give();
                state_update();
            }
            return true;
        }
    }
    return false;
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

void state_semaphore_take(void) {
    configASSERT(xSemaphoreTake(state_semaphore, portMAX_DELAY) == pdTRUE);
}

void state_semaphore_give(void) {
    xSemaphoreGive(state_semaphore);
}
