#include "control.h"

#include <FreeRTOS.h>
#include <task.h>
#include <stddef.h>

#include "state_config.h"
#include "hal.h"

#define TASK_STACK_SIZE 100

static struct {
    bool fluid_estimate_good; // Whether the fluid estimate is good enough to be given to the user. It might be good enough for internal computation even if this is false.
    int32_t fluid_estimate; // The fluid estimate, or -1 if we really have no idea
    bool container_estimate_good; // Whether the container estimate is good enough to be given to the user (may be redundant with container_estimate < max_container)
    int32_t container_estimate; // The container estimate, or max_container if we really have no idea

    int32_t fluid_setpoint; // The setpoint expressed in ms
    int8_t last_direction; // Which way the pump was going last iteration
    bool error_on_setpoint_achieved; // Whether achieving the setpoint constitutes an "error" (e.g. if we are trying to fill all the way up and never sense water)

    int8_t last_scheduled_setpoint;
} shadow_state_pot[N_POTS];

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

static void pots_schedule(void) {
    struct datetime dt;
    hal_time_get(&dt);

    state_semaphore_take();
    for (size_t i = 0; i < N_POTS; i++) {
        if (state.pot[i].mode == POT_MODE_LEVEL_SCHEDULE) {
            uint8_t start_hour = state.pot[i].start_hour;
            uint8_t end_hour = state.pot[i].end_hour;
            uint8_t period_hours = state.pot[i].period_hours;
            uint8_t start_minute = state.pot[i].start_minute;
            uint8_t end_minute = state.pot[i].end_minute;
            uint8_t hour = dt.hour;
            uint8_t minute = dt.minute;

            int8_t hour_count = -1;
            if (end_hour >= start_hour) {
                if (hour >= start_hour && hour < end_hour) {
                    hour_count = hour - start_hour;
                }
            } else {
                if (hour >= start_hour || hour < end_hour) {
                    if (hour >= start_hour) {
                        hour_count = hour - start_hour;
                    } else {
                        hour_count = hour + (start_hour - end_hour);
                    }
                }
            }

            uint8_t level = 0;
            if (hour_count >= 0
             && hour_count % period_hours == 0
             && minute >= start_minute
             && minute <= end_minute) {
                level = 100;
            }
            if (level != shadow_state_pot[i].last_scheduled_setpoint) {
                state.pot[i].level_setpoint = level;
                control_pot_change_level_setpoint(i);
                shadow_state_pot[i].last_scheduled_setpoint = level;
            }
        } else {
            shadow_state_pot[i].last_scheduled_setpoint = -1;
        }
    }
    state_semaphore_give();
}

static void pots(void) {
    uint32_t time = xTaskGetTickCount();
    static uint32_t last_time;

    if (last_time == 0) {
        last_time = time;
        return;
    }
    uint32_t delta = time - last_time;
    last_time = time;

    state_semaphore_take();

    for (size_t i = 0; i < N_POTS; i++) {
        if (state.pot[i].mode == POT_MODE_LEVEL_CONTROL || state.pot[i].mode == POT_MODE_LEVEL_SCHEDULE) {
            // Main pot fluid level control loop
            if (shadow_state_pot[i].last_direction != 0) {
                shadow_state_pot[i].fluid_estimate += shadow_state_pot[i].last_direction * delta;
                if (shadow_state_pot[i].fluid_estimate > shadow_state_pot[i].container_estimate) {
                    shadow_state_pot[i].container_estimate = shadow_state_pot[i].fluid_estimate;
                }
            }

            // Handle transitions
            if (state.overflow) {
                // Uncalibrate the system
                shadow_state_pot[i].fluid_estimate_good = false;
                shadow_state_pot[i].container_estimate_good = false;
                shadow_state_pot[i].container_estimate = state.pot[i].max_container;
                state.pot[i].error = POT_ERROR_OVERFLOW;
            } else if (shadow_state_pot[i].last_direction == 1 && shadow_state_pot[i].fluid_estimate >= shadow_state_pot[i].fluid_setpoint) {
                if (shadow_state_pot[i].error_on_setpoint_achieved) {
                    // Filled more than expected. Uncalibrate the system
                    shadow_state_pot[i].fluid_estimate_good = false;
                    shadow_state_pot[i].container_estimate_good = false;
                    shadow_state_pot[i].container_estimate = state.pot[i].max_container;
                    state.pot[i].error = POT_ERROR_OVERFILLED;
                }
                // Done filling
                state.pot[i].pump = 0;
            } else if (shadow_state_pot[i].last_direction == -1 && shadow_state_pot[i].fluid_estimate <= shadow_state_pot[i].fluid_setpoint) {
                if (shadow_state_pot[i].fluid_setpoint < 0) {
                    // Drained completely
                    shadow_state_pot[i].fluid_estimate_good = true;
                }
                state.pot[i].pump = 0;
            } else if (shadow_state_pot[i].last_direction == 1 && state.pot[i].sense) {
                if (shadow_state_pot[i].fluid_estimate_good) {
                    shadow_state_pot[i].container_estimate = shadow_state_pot[i].fluid_estimate;
                    shadow_state_pot[i].container_estimate_good = true;
                }
                state.pot[i].pump = 0; // Hopefully redundant with the pump loop...
            }
            shadow_state_pot[i].last_direction = state.pot[i].pump;
        }

        // Update visible state from shadow state
        if (shadow_state_pot[i].fluid_estimate_good) {
            state.pot[i].fluid = shadow_state_pot[i].fluid_estimate;
            // Clamp to between 0 and container_estimate
            if (state.pot[i].fluid > shadow_state_pot[i].container_estimate) {
                state.pot[i].fluid = shadow_state_pot[i].container_estimate;
            }
            if (state.pot[i].fluid < 0) {
                state.pot[i].fluid = 0;
            }
        } else {
            // Indicate a bad estimate with -1
            state.pot[i].fluid = -1;
        }
        if (shadow_state_pot[i].container_estimate_good) {
            state.pot[i].container = shadow_state_pot[i].container_estimate;
        } else {
            // Indicate a bad estimate with -1
            state.pot[i].container = -1;
        }
        if (shadow_state_pot[i].fluid_estimate_good && shadow_state_pot[i].container_estimate_good) {
            configASSERT(state.pot[i].container > 0);
            configASSERT(state.pot[i].container >= state.pot[i].fluid);
            state.pot[i].level = 100 * state.pot[i].fluid / state.pot[i].container;
        } else {
            state.pot[i].level = -1;
        }
    }

    state_semaphore_give();
}

static void sense(void) {
    state_semaphore_take();
    state.overflow = hal_sense_overflow();
    for (size_t i = 0; i < N_POTS; i++) {

        state.pot[i].sense = hal_sense_full(i + 1);
        if (state.pot[i].sense && state.pot[i].pump > 0) {
            state.pot[i].pump = 0;
        }
        if (state.overflow) {
            state.pot[i].pump = 0;
        }
    }
    state_semaphore_give();
}

static void pumps(void) {
    state_semaphore_take();
    for (size_t i = 0; i < N_POTS; i++) {
        if (state.pot[i].pump == 1) {
            hal_pump_flood(i + 1);
        } else if (state.pot[i].pump == -1) {
            hal_pump_drain(i + 1);
        } else {
            hal_pump_off(i + 1);
        }
    }
    state_semaphore_give();
}

static void control_loop(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        relays_schedule();
        relays();

        sense();
        pots_schedule();
        pots();
        pumps();

        state_update();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void control_init(void) {
    // Must be called after state_init

    for (size_t i = 0; i < N_POTS; i++) {
        shadow_state_pot[i].fluid_estimate = -1;
        shadow_state_pot[i].container_estimate = state.pot[i].max_container;
    }

    configASSERT(xTaskCreate(control_loop, "control_loop", TASK_STACK_SIZE, NULL, 1, NULL) == pdTRUE);
}

void control_pot_change_fluid_estimate(size_t index) {
    // Note: this function is called with the state semaphore locked
    // Update shadow state from visible state

    if (state.pot[index].fluid == -1) {
        // User is invalidating the current estimate
        shadow_state_pot[index].fluid_estimate = -1;
        shadow_state_pot[index].fluid_estimate_good = false;
        return;
    }

    if (shadow_state_pot[index].container_estimate_good && state.pot[index].fluid <= shadow_state_pot[index].container_estimate) {
        shadow_state_pot[index].fluid_estimate = state.pot[index].fluid;
        shadow_state_pot[index].fluid_estimate_good = true;
    }
}

void control_pot_change_container_estimate(size_t index) {
    // Note: this function is called with the state semaphore locked
    // Update shadow state from visible state

    if (state.pot[index].container == -1) {
        shadow_state_pot[index].container_estimate = state.pot[index].max_container;
        shadow_state_pot[index].container_estimate_good = false;
        return;
    }

    shadow_state_pot[index].container_estimate = state.pot[index].container;
    shadow_state_pot[index].container_estimate_good = true;
}

void control_pot_change_level_setpoint(size_t index) {
    // Note: this function is called with the state semaphore locked

    if (!((state.pot[index].level_setpoint == 0 || state.pot[index].level_setpoint == 100)
       || (shadow_state_pot[index].fluid_estimate_good && shadow_state_pot[index].container_estimate_good))) {
        // Can't set a value between 0 and 100 without a good fluid and container estimate.
        state.pot[index].error = POT_ERROR_UNCALIBRATED;
        return;
    }

    if (!shadow_state_pot[index].fluid_estimate_good) {
        // Make a worst-case setpoint estimate
        if (state.pot[index].level_setpoint == 0) {
            shadow_state_pot[index].fluid_estimate = shadow_state_pot[index].container_estimate;
        } else if (state.pot[index].level_setpoint == 100) {
            shadow_state_pot[index].fluid_estimate = 0;
        } else {
            // Only 0 and 100 are allowed if there isn't a valid fluid estimate
            configASSERT(false);
        }
    } else {
        // Clamp the fluid & container estimate to within bounds
        // (it may be set out-of-bounds by setting the level setpoint to 0 or 1, see below)
        if ((uint32_t)shadow_state_pot[index].container_estimate > state.pot[index].max_container) {
            shadow_state_pot[index].container_estimate = state.pot[index].max_container;
        }
        if (shadow_state_pot[index].fluid_estimate < 0) {
            shadow_state_pot[index].fluid_estimate = 0;
        }
        if (shadow_state_pot[index].fluid_estimate > shadow_state_pot[index].container_estimate) {
            shadow_state_pot[index].fluid_estimate = shadow_state_pot[index].container_estimate;
        }
    }

    shadow_state_pot[index].error_on_setpoint_achieved = false;
    if (state.pot[index].level_setpoint == 0) {
        shadow_state_pot[index].fluid_setpoint = -state.pot[index].max_extra_drain;
    } else if (state.pot[index].level_setpoint == 100) {
        shadow_state_pot[index].fluid_setpoint = shadow_state_pot[index].container_estimate + state.pot[index].max_extra_flood;
        shadow_state_pot[index].error_on_setpoint_achieved = true;
    } else {
        configASSERT(shadow_state_pot[index].container_estimate_good);
        shadow_state_pot[index].fluid_setpoint = shadow_state_pot[index].container_estimate * state.pot[index].level_setpoint / 100;
    }

    state.pot[index].error = POT_ERROR_NONE;

    if (shadow_state_pot[index].fluid_setpoint > shadow_state_pot[index].fluid_estimate) {
        state.pot[index].pump = 1;
        shadow_state_pot[index].last_direction = 1;
    } else if (shadow_state_pot[index].fluid_setpoint < shadow_state_pot[index].fluid_estimate) {
        state.pot[index].pump = -1;
        shadow_state_pot[index].last_direction = -1;
    } else {
        state.pot[index].pump = 0;
        shadow_state_pot[index].last_direction = 0;
    }
}
