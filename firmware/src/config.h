#pragma once

#include <stdint.h>

struct config {
    uint8_t test;
};

extern struct config config; // Defined by HAL

#define CONFIG_DEFAULTS { \
    .test = 5, \
}
