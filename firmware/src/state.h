#pragma once

#include <stdint.h>

#define STATE_DEFAULTS {\
    .relay1 = 0, \
}

struct state {
    uint8_t relay1;
};

extern struct state state;

void state_init(void);
