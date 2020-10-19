#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define RELAY_MODE_RAW 0
#define RELAY_MODE_SCHEDULE 1

struct state_relay {
    uint8_t state;
    uint8_t mode;
    uint8_t on_hour;
    uint8_t off_hour;
};

struct state {
    struct state_relay relay[2];
};

extern struct state state;

#define STATE_DEFAULTS {\
}

struct config_relay {
    uint8_t mode;
    uint8_t on_hour;
    uint8_t off_hour;
};

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
