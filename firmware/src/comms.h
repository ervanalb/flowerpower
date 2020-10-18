#pragma once

#include <stddef.h>

// Spins up the tasks and other OS resources needed for communication.
// Should be run before starting the scheduler.
void comms_init(void);

// Sends a message. Uses a mutex to prevent concurrent transmission, so it may block.
// Message should end with a newline.
void comms_send(const char *message, size_t length);

// Sends a message with printf-like semantics
void comms_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
