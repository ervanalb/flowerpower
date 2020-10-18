#pragma once

// Spins up the tasks and other OS resources needed for communication.
// Should be run before starting the scheduler.
void comms_init(void);

// Sends a message. Uses a mutex to prevent concurrent transmission, so it may block.
// Message should end with a newline.
void comms_send(const char *message, size_t length);
