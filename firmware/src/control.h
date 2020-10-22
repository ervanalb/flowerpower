#pragma once
#include <stddef.h>

#define RELAY_MODE_RAW 0
#define RELAY_MODE_SCHEDULE 1

#define POT_MODE_RAW 0
#define POT_MODE_LEVEL_CONTROL 1

#define POT_ERROR_NONE 0
#define POT_ERROR_UNCALIBRATED 1
#define POT_ERROR_OVERFILLED 2

void control_init(void);

void control_pot_change_fluid_estimate(size_t index);
void control_pot_change_container_estimate(size_t index);
void control_pot_change_level_setpoint(size_t index);
