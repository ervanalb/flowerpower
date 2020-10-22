#include "state_config.h"

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "comms.h"
#include "control.h"
#include "hal.h"

struct state state;

extern struct config config; // Defined by HAL

#define TASK_STACK_SIZE 160
#define NOTIFY_UPDATE 0

static TaskHandle_t publish_state_task_handle;

static const char *relay_mode_to_string(uint8_t mode);
static void relay_set_state(size_t i, int value);
static void relay_set_mode(size_t i, const char *value);
static void relay_set_on_hour(size_t i, int value);
static void relay_set_off_hour(size_t i, int value);
static void pump_set(size_t i, int value);
static const char *pot_mode_to_string(uint8_t mode);
static void pot_set_mode(size_t i, const char *value);
static const char *pot_error_to_string(uint8_t error);
static void pot_set_level_setpoint(size_t i, int value);

// This semaphore is probably not required when preemption = 0
// but might as well be a little extra safe
// (or if we ever want to turn on preemption)
static SemaphoreHandle_t state_semaphore = NULL;

static struct state last_state;

static void load_state_from_config(void) {
    // Called from init (before scheduler), so no semaphores
    for (int i=0; i<N_RELAYS; i++) {
#define RELAY_STATE(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN)
#define RELAY_CONFIG(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN) state.relay[i].NAME = config.relay[i].NAME;
        RELAY_DEF
#undef RELAY_STATE
#undef RELAY_CONFIG
    }
    for (int i=0; i<N_POTS; i++) {
#define POT_STATE(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN)
#define POT_CONFIG(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN) state.pot[i].NAME = config.pot[i].NAME;
        POT_DEF
#undef POT_STATE
#undef POT_CONFIG
    }
}

static void maybe_write_config(void) {
    state_semaphore_take();

#define GLOBAL_STATE(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN)
#define GLOBAL_CONFIG(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN) config.NAME = state.NAME;
        GLOBAL_DEF
#undef GLOBAL_STATE
#undef GLOBAL_CONFIG

    for (int i=0; i<N_RELAYS; i++) {
#define RELAY_STATE(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN)
#define RELAY_CONFIG(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN) config.relay[i].NAME = state.relay[i].NAME;
        RELAY_DEF
#undef RELAY_STATE
#undef RELAY_CONFIG
    }
    for (int i=0; i<N_POTS; i++) {
#define POT_STATE(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN)
#define POT_CONFIG(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN) config.pot[i].NAME = state.pot[i].NAME;
        POT_DEF
#undef POT_STATE
#undef POT_CONFIG
    }
    state_semaphore_give();
    // This function will only issue a flash write if there have been changes.
    hal_update_config();
}

static void publish_state_task(void *pvParameters) {
    (void)pvParameters;

    const TickType_t publish_period = pdMS_TO_TICKS(10000);

    TickType_t last_send_all = 0;

    int counter = 0;
    for (;;) {
        bool send_all = false;
        TickType_t elapsed_time = xTaskGetTickCount() - last_send_all;
        send_all = (elapsed_time > publish_period);

        state_semaphore_take();

#define GLOBAL_STATE(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN) \
            if (send_all || state.NAME != last_state.NAME) { \
                comms_printf("/" OUTPUT_FMT "\n", OUTPUT_FN(state.NAME)); \
                last_state.NAME = state.NAME; \
            }
#define GLOBAL_CONFIG GLOBAL_STATE
            GLOBAL_DEF
#undef GLOBAL_STATE
#undef GLOBAL_CONFIG

        for (int i=0; i<N_RELAYS; i++) {
#define RELAY_STATE(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN) \
            if (send_all || state.relay[i].NAME != last_state.relay[i].NAME) { \
                comms_printf("/relay/%d/" OUTPUT_FMT "\n", i + 1, OUTPUT_FN(state.relay[i].NAME)); \
                last_state.relay[i].NAME = state.relay[i].NAME; \
            }
#define RELAY_CONFIG RELAY_STATE
            RELAY_DEF
#undef RELAY_STATE
#undef RELAY_CONFIG
        }
        for (int i=0; i<N_POTS; i++) {
#define POT_STATE(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN) \
            if (send_all || state.pot[i].NAME != last_state.pot[i].NAME) { \
                comms_printf("/pot/%d/" OUTPUT_FMT "\n", i + 1, OUTPUT_FN(state.pot[i].NAME)); \
                last_state.pot[i].NAME = state.pot[i].NAME; \
            }
#define POT_CONFIG POT_STATE
            POT_DEF
#undef POT_STATE
#undef POT_CONFIG
        }

        state_semaphore_give();

        if (send_all) {
            {
                struct datetime dt;
                hal_time_get(&dt);
                comms_printf("/time 20%02d-%02d-%02dT%02d:%02d:%02d\n", dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
            }

            comms_printf("/sys/n_relays %d\n", N_RELAYS);
            comms_printf("/sys/n_pots %d\n", N_POTS);

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
    load_state_from_config();

    configASSERT(xTaskCreate(publish_state_task, "publish_state", TASK_STACK_SIZE, NULL, 1, &publish_state_task_handle) == pdTRUE);
    state_semaphore = xSemaphoreCreateMutex();
    configASSERT(state_semaphore);
}

void state_update() {
    bool updated = false;

    state_semaphore_take();

#define GLOBAL_STATE(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN) \
        if (state.NAME != last_state.NAME) { \
            updated = true; \
            goto done; \
        }
#define GLOBAL_CONFIG GLOBAL_STATE
            GLOBAL_DEF
#undef GLOBAL_STATE
#undef GLOBAL_CONFIG

    for (size_t i=0; i<N_RELAYS; i++) {
#define RELAY_STATE(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN) \
        if (state.relay[i].NAME != last_state.relay[i].NAME) { \
            updated = true; \
            goto done; \
        }
#define RELAY_CONFIG RELAY_STATE
            RELAY_DEF
#undef RELAY_STATE
#undef RELAY_CONFIG
    }
    for (size_t i=0; i<N_POTS; i++) {
#define POT_STATE(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN) \
        if (state.pot[i].NAME != last_state.pot[i].NAME) { \
            updated = true; \
            goto done; \
        }
#define POT_CONFIG POT_STATE
            POT_DEF
#undef POT_STATE
#undef POT_CONFIG
    }

done:
    state_semaphore_give();

    if (updated) {
        xTaskNotifyGiveIndexed(publish_state_task_handle, NOTIFY_UPDATE);
    }
}

bool state_parse(const char *message, size_t length) {
#define GLOBAL_SET(NAME, INPUT_FMT, INPUT_TYPE, INPUT_FN, REF) \
    { \
        int n; \
        typeof (INPUT_TYPE) value; \
        int matched = sscanf(message, "/" INPUT_FMT "\n%n", REF value, &n); \
        if (matched == 1 && n > 0 && (size_t)n == length) { \
            state_semaphore_take(); \
            INPUT_FN(value); \
            state_semaphore_give(); \
            return true; \
        } \
    }
            GLOBAL_SET_DEF
#undef GLOBAL_SET

#define RELAY_SET(NAME, INPUT_FMT, INPUT_TYPE, INPUT_FN, REF) \
    { \
        int n; \
        int i; \
        typeof (INPUT_TYPE) value; \
        int matched = sscanf(message, "/relay/%d/" INPUT_FMT "\n%n", &i, REF value, &n); \
        if (matched == 2 && n > 0 && (size_t)n == length) { \
            if (i > 0 && i <= N_RELAYS) { \
                state_semaphore_take(); \
                INPUT_FN((size_t)i - 1, value); \
                state_semaphore_give(); \
            } \
            return true; \
        } \
    }
            RELAY_SET_DEF
#undef RELAY_SET

#define POT_SET(NAME, INPUT_FMT, INPUT_TYPE, INPUT_FN, REF) \
    { \
        int n; \
        int i; \
        typeof (INPUT_TYPE) value; \
        int matched = sscanf(message, "/pot/%d/" INPUT_FMT "\n%n", &i, REF value, &n); \
        if (matched == 2 && n > 0 && (size_t)n == length) { \
            if (i > 0 && i <= N_POTS) { \
                state_semaphore_take(); \
                INPUT_FN((size_t)i - 1, value); \
                state_semaphore_give(); \
            } \
            return true; \
        } \
    }
            POT_SET_DEF
#undef POT_SET

    {
        int n;
        int year;
        int month;
        int day;
        int hour;
        int minute;
        int second;
        int matched = sscanf(message, "/time/set 20%02d-%02d-%02dT%02d:%02d:%02d\n%n", &year, &month, &day, &hour, &minute, &second, &n);

        if (matched == 6 && n > 0 && (size_t)n == length) {
            state_semaphore_take();
            if (year >= 0 && year <= 99
             && month >= 1 && month <= 12
             && day >= 1 && day <= 31
             && hour >= 0 && hour <= 23
             && minute >= 0 && minute <= 59
             && second >= 0 && second <= 59) {
                struct datetime dt;
                dt.year = year;
                dt.month = month;
                dt.day = day;
                dt.hour = hour;
                dt.minute = minute;
                dt.second = second;
                dt.weekday = 0;
                hal_time_set(&dt);
            }
            state_semaphore_give();
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

static void relay_set_state(size_t i, int value) {
    if (value >= 0 && value <= 1) {
        state.relay[i].state = value;
        state.relay[i].mode = RELAY_MODE_RAW;
    }
}

static void relay_set_mode(size_t i, const char *value) {
    if (strcmp(value, "RAW") == 0) {
        state.relay[i].mode = RELAY_MODE_RAW;
    } else if (strcmp(value, "SCHEDULE") == 0) {
        state.relay[i].mode = RELAY_MODE_SCHEDULE;
    }
}

static void relay_set_on_hour(size_t i, int value) {
    if (value >= 0 && value <= 23) {
        state.relay[i].on_hour = value;
    }
}

static void relay_set_off_hour(size_t i, int value) {
    if (value >= 0 && value <= 23) {
        state.relay[i].off_hour = value;
    }
}

static void pump_set(size_t i, int value) {
    if (value >= -1 && value <= 1) {
        state.pot[i].pump = value;
        state.pot[i].mode = POT_MODE_RAW;
        if (value != 0) {
            state.pot[i].fluid = -1;
            state.pot[i].level_setpoint = -1;
        }
    }
}

static const char *pot_mode_to_string(uint8_t mode) {
    switch (mode) {
        case POT_MODE_RAW:
            return "RAW";
        case POT_MODE_LEVEL_CONTROL:
            return "LEVEL_CONTROL";
        default:
            return "UNKNOWN";
    }
}

static void pot_set_mode(size_t i, const char *value) {
    if (strcmp(value, "RAW") == 0) {
        state.pot[i].mode = POT_MODE_RAW;
    } else if (strcmp(value, "LEVEL_CONTROL") == 0) {
        state.pot[i].mode = POT_MODE_LEVEL_CONTROL;
    }
}

static void pot_set_level_setpoint(size_t i, int value) {
    if (value >= 0 && value <= 100) {
        state.pot[i].level_setpoint = value;
        state.pot[i].mode = POT_MODE_LEVEL_CONTROL;
        control_pot_change_level_setpoint(i);
    }
}

static const char *pot_error_to_string(uint8_t error) {
    switch (error) {
        case POT_ERROR_NONE:
            return "NONE";
        case POT_ERROR_UNCALIBRATED:
            return "UNCALIBRATED";
        case POT_ERROR_OVERFILLED:
            return "OVERFILLED";
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
