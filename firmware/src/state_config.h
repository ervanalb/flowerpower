#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "hal.h"

#define DEFAULT_OUTPUT

#define GLOBAL_DEF \
GLOBAL_STATE(uint8_t, overflow, "overflow %d", DEFAULT_OUTPUT) \

#define GLOBAL_SET_DEF \

#define RELAY_DEF \
RELAY_STATE(uint8_t, state, "state %d", DEFAULT_OUTPUT) \
RELAY_CONFIG(uint8_t, mode, "mode %s", relay_mode_to_string) \
RELAY_CONFIG(uint8_t, on_hour, "on/hour %d", DEFAULT_OUTPUT) \
RELAY_CONFIG(uint8_t, off_hour, "off/hour %d", DEFAULT_OUTPUT) \

#define RELAY_SET_DEF \
RELAY_SET(state, "state/set %d", int, relay_set_state, &) \
RELAY_SET(mode, "mode/set %8s", char[9], relay_set_mode, ) \
RELAY_SET(on_hour, "on_hour/set %d", int, relay_set_on_hour, &) \
RELAY_SET(off_hour, "off_hour/set %d", int, relay_set_off_hour, &) \

#define POT_DEF \
POT_STATE(int8_t, pump, "pump %d", DEFAULT_OUTPUT) \
POT_STATE(int8_t, sense, "sense %d", DEFAULT_OUTPUT) \
POT_CONFIG(int8_t, mode, "mode %s", pot_mode_to_string) \
POT_CONFIG(uint32_t, max_container, "max_container %lu", DEFAULT_OUTPUT) \
POT_CONFIG(uint32_t, max_extra_flood, "max_extra_flood %lu", DEFAULT_OUTPUT) \
POT_CONFIG(uint32_t, max_extra_drain, "max_extra_drain %lu", DEFAULT_OUTPUT) \
POT_STATE(int32_t, fluid, "fluid %ld", DEFAULT_OUTPUT) \
POT_STATE(int32_t, container, "container %ld", DEFAULT_OUTPUT) \
POT_STATE(int8_t, level_setpoint, "level_setpoint %d", DEFAULT_OUTPUT) \
POT_STATE(int8_t, level, "level %d", DEFAULT_OUTPUT) \
POT_STATE(int8_t, error, "error %s", pot_error_to_string) \

#define POT_SET_DEF \
POT_SET(pump, "pump/set %d", int, pump_set, &) \
POT_SET(mode, "mode/set %13s", char[14], pot_set_mode, ) \
POT_SET(mode, "level_setpoint/set %d", int, pot_set_level_setpoint, &) \
// TODO: make fluid and container settable

#define RELAY_STATE(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN) typeof (TYPE) NAME;
#define RELAY_CONFIG RELAY_STATE
struct state_relay {
    RELAY_DEF
};
#undef RELAY_STATE
#undef RELAY_CONFIG

#define POT_STATE(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN) typeof (TYPE) NAME;
#define POT_CONFIG POT_STATE
struct state_pot {
    POT_DEF
};
#undef POT_STATE
#undef POT_CONFIG

#define GLOBAL_STATE(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN) typeof (TYPE) NAME;
#define GLOBAL_CONFIG GLOBAL_STATE
struct state {
    GLOBAL_DEF
    struct state_relay relay[N_RELAYS];
    struct state_pot pot[N_POTS];
};
#undef GLOBAL_STATE
#undef GLOBAL_CONFIG

extern struct state state;

#define RELAY_CONFIG(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN) typeof (TYPE) NAME;
#define RELAY_STATE(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN)
struct config_relay {
    RELAY_DEF
};
#undef RELAY_STATE
#undef RELAY_CONFIG

#define POT_CONFIG(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN) typeof (TYPE) NAME;
#define POT_STATE(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN)
struct config_pot {
    POT_DEF
};
#undef POT_STATE
#undef POT_CONFIG

// NOTE: Application should not use config.
// config is a subset of state

#define GLOBAL_CONFIG(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN) typeof (TYPE) NAME;
#define GLOBAL_STATE(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN)
struct config {
    GLOBAL_DEF
    struct config_relay relay[N_RELAYS];
    struct config_pot pot[N_POTS];
};
#undef GLOBAL_STATE
#undef GLOBAL_CONFIG

#define CONFIG_DEFAULTS {\
    .pot = {[0 ... N_POTS - 1] = { \
            .max_container = 150000, \
            .max_extra_flood = 20000, \
            .max_extra_drain = 20000, \
        } \
    } \
}

void state_init(void);
void state_update(void);
void state_semaphore_take(void);
void state_semaphore_give(void);

// Attempt to handle the given message. Return True if handled
bool state_parse(const char *message, size_t length);
