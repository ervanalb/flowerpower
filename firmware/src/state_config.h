#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define RELAY_MODE_RAW 0
#define RELAY_MODE_SCHEDULE 1

#define DEFAULT_OUTPUT

// relay_mode_to_string

#define RELAY_DEF \
RELAY_STATE(uint8_t, state, "state %d", DEFAULT_OUTPUT) \
RELAY_CONFIG(uint8_t, mode, "mode %d", DEFAULT_OUTPUT) \
RELAY_CONFIG(uint8_t, on_hour, "on/hour %d", DEFAULT_OUTPUT) \
RELAY_CONFIG(uint8_t, off_hour, "off/hour %d", DEFAULT_OUTPUT) \

#define RELAY_SET_DEF \
RELAY_SET(state, "state/set %d", int, relay_set_state, &) \
RELAY_SET(mode, "mode/set %8s", char[9], relay_set_mode, ) \

#define RELAY_STATE(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN) typeof (TYPE) NAME;
#define RELAY_CONFIG RELAY_STATE
struct state_relay {
    RELAY_DEF
};
#undef RELAY_STATE
#undef RELAY_CONFIG

struct state {
    struct state_relay relay[2];
};

extern struct state state;

#define RELAY_DEFAULTS {\
}

#define RELAY_CONFIG(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN) typeof (TYPE) NAME;
#define RELAY_STATE(TYPE, NAME, OUTPUT_FMT, OUTPUT_FN)
struct config_relay {
    RELAY_DEF
};
#undef RELAY_STATE
#undef RELAY_CONFIG

struct config {
    struct config_relay relay[2];
};

extern struct config config; // Defined by HAL

#define CONFIG_DEFAULTS {\
}

void state_init(void);
void state_update(void);
void state_semaphore_take(void);
void state_semaphore_give(void);

// Attempt to handle the given message. Return True if handled
bool state_parse(const char *message, size_t length);
