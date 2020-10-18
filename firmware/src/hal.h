#pragma once

struct datetime {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t weekday;
    uint8_t year;
};

#define N_POTS 5
#define N_RELAYS 2

void hal_init(void);
void hal_time_set(const struct datetime *datetime);
void hal_time_get(struct datetime *datetime);

void hal_heartbeat_on(void);
void hal_heartbeat_off(void);

void hal_pump_off(uint8_t pump);
void hal_pump_flood(uint8_t pump);
void hal_pump_drain(uint8_t pump);
bool hal_sense_full(uint8_t sensor);
bool hal_sense_overflow(void);

void hal_relay_on(uint8_t relay);
void hal_relay_off(uint8_t relay);

void hal_watchdog_feed(void);
