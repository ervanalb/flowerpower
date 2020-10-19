#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/iwdg.h>
#include <libopencm3/stm32/flash.h>

#include <FreeRTOS.h>
#include <task.h>

#include <string.h>

#include "usb.h"
#include "state_config.h"
#include "hal.h"

static struct config config_flash __attribute__((section(".config"))) = CONFIG_DEFAULTS;

struct config config __attribute__((aligned(4)));

void hal_time_set(const struct datetime *datetime) {
    uint32_t time_bcd = 0;
    uint32_t date_bcd = 0;
    {
        uint8_t second_tens = datetime->second / 10;
        uint8_t second_ones = datetime->second - second_tens * 10;
        time_bcd |= second_ones | (second_tens << 4);
    }
    {
        uint8_t minute_tens = datetime->minute / 10;
        uint8_t minute_ones = datetime->minute - minute_tens * 10;
        time_bcd |= (minute_ones << 8) | (minute_tens << 12);
    }
    {
        uint8_t hour_tens = datetime->hour / 10;
        uint8_t hour_ones = datetime->hour - hour_tens * 10;
        time_bcd |= (hour_ones << 16) | (hour_tens << 20);
    }
    {
        uint8_t day_tens = datetime->day / 10;
        uint8_t day_ones = datetime->day - day_tens * 10;
        date_bcd |= day_ones | (day_tens << 4);
    }
    {
        uint8_t month_tens = datetime->month / 10;
        uint8_t month_ones = datetime->month - month_tens * 10;
        date_bcd |= (month_ones << 8) | (month_tens << 12);
    }
    date_bcd |= (datetime->weekday << 13);
    {
        uint8_t year_tens = datetime->year / 10;
        uint8_t year_ones = datetime->year - year_tens * 10;
        date_bcd |= (year_ones << 16) | (year_tens << 20);
    }

    pwr_disable_backup_domain_write_protect();
    rtc_unlock();
    RTC_ISR |= RTC_ISR_INIT;

    while (!(RTC_ISR & RTC_ISR_INITF));
    RTC_TR = time_bcd;
    RTC_DR = date_bcd;
    RTC_ISR &= ~RTC_ISR_INIT;
    rtc_lock();
    pwr_enable_backup_domain_write_protect();
}

void hal_time_get(struct datetime *datetime) {
    uint32_t time_bcd = RTC_TR;
    uint32_t date_bcd = RTC_DR;

    datetime->second = (time_bcd & 0xF) + 10 * ((time_bcd & 0x70) >> 4);
    datetime->minute = ((time_bcd & 0xF00) >> 8) + 10 * ((time_bcd & 0x7000) >> 12);
    datetime->hour = ((time_bcd & 0xF0000) >> 16) + 10 * ((time_bcd & 0x300000) >> 20);

    datetime->day = (date_bcd & 0xF) + 10 * ((date_bcd & 0x30) >> 4);
    datetime->month = ((date_bcd & 0xF00) >> 8) + 10 * ((date_bcd & 0x1000) >> 12);
    datetime->weekday = (date_bcd & 0xE000) >> 8;
    datetime->year = ((date_bcd & 0xF0000) >> 16) + 10 * ((date_bcd & 0xF00000) >> 20);
}

void hal_heartbeat_on(void) {
    gpio_set(GPIOB, GPIO4);
}

void hal_heartbeat_off(void) {
    gpio_clear(GPIOB, GPIO4);
}

void hal_relay_on(uint8_t relay) {
    switch (relay) {
        case 1:
            gpio_set(GPIOA, GPIO5);
            break;
        case 2:
            gpio_set(GPIOA, GPIO4);
            break;
        default:
            configASSERT(false);
    }
}

void hal_relay_off(uint8_t relay) {
    switch (relay) {
        case 1:
            gpio_clear(GPIOA, GPIO5);
            break;
        case 2:
            gpio_clear(GPIOA, GPIO4);
            break;
        default:
            configASSERT(false);
    }
}

void hal_pump_off(uint8_t pump) {
    switch (pump) {
        case 1:
            gpio_clear(GPIOA, GPIO0);
            gpio_clear(GPIOA, GPIO1);
            break;
        case 2:
            gpio_clear(GPIOA, GPIO2);
            gpio_clear(GPIOA, GPIO3);
            break;
        case 3:
            gpio_clear(GPIOA, GPIO6);
            gpio_clear(GPIOA, GPIO7);
            break;
        case 4:
            gpio_clear(GPIOB, GPIO0);
            gpio_clear(GPIOB, GPIO13);
            break;
        case 5:
            gpio_clear(GPIOB, GPIO14);
            gpio_clear(GPIOB, GPIO15);
            break;
        default:
            configASSERT(false);
    }
}

void hal_pump_flood(uint8_t pump) {
    switch (pump) {
        case 1:
            gpio_set(GPIOA, GPIO0);
            gpio_clear(GPIOA, GPIO1);
            break;
        case 2:
            gpio_set(GPIOA, GPIO2);
            gpio_clear(GPIOA, GPIO3);
            break;
        case 3:
            gpio_set(GPIOA, GPIO6);
            gpio_clear(GPIOA, GPIO7);
            break;
        case 4:
            gpio_set(GPIOB, GPIO0);
            gpio_clear(GPIOB, GPIO13);
            break;
        case 5:
            gpio_set(GPIOB, GPIO14);
            gpio_clear(GPIOB, GPIO15);
            break;
        default:
            configASSERT(false);
    }
}

void hal_pump_drain(uint8_t pump) {
    switch (pump) {
        case 1:
            gpio_clear(GPIOA, GPIO0);
            gpio_set(GPIOA, GPIO1);
            break;
        case 2:
            gpio_clear(GPIOA, GPIO2);
            gpio_set(GPIOA, GPIO3);
            break;
        case 3:
            gpio_clear(GPIOA, GPIO6);
            gpio_set(GPIOA, GPIO7);
            break;
        case 4:
            gpio_clear(GPIOB, GPIO0);
            gpio_set(GPIOB, GPIO13);
            break;
        case 5:
            gpio_clear(GPIOB, GPIO14);
            gpio_set(GPIOB, GPIO15);
            break;
        default:
            configASSERT(false);
    }
}

bool hal_sense_full(uint8_t sensor) {
    switch (sensor) {
        case 1:
            return gpio_get(GPIOB, GPIO2) != 0;
        case 2:
            return gpio_get(GPIOB, GPIO10) != 0;
        case 3:
            return gpio_get(GPIOB, GPIO11) != 0;
        case 4:
            return gpio_get(GPIOB, GPIO12) != 0;
        case 5:
            return gpio_get(GPIOA, GPIO8) != 0;
        default:
            configASSERT(false);
            return true;
    }
}

bool hal_sense_overflow(void) {
    return gpio_get(GPIOA, GPIO9) != 0;
}

void hal_init() {
    rcc_clock_setup_in_hsi_out_48mhz();
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_RTC);
    rcc_periph_clock_enable(RCC_PWR);

    // Set up watchdog
    //iwdg_start();

    // Heartbeat LED
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO4);

    // Relays
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO4);
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO5);

    // Pumps
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0);
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1);
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO2);
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO3);
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO6);
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO7);
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0);
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO14);
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO15);

    // Water sensors
    gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO8);
    gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO9);
    gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO2);
    gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO10);
    gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO11);
    gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO12);

    // Set up RTC
    pwr_disable_backup_domain_write_protect();
    rcc_osc_on(RCC_LSE);
    rcc_wait_for_osc_ready(RCC_LSE);
    rcc_set_rtc_clock_source(RCC_LSE);
    rcc_enable_rtc_clock();
    pwr_enable_backup_domain_write_protect();

    // Set up Config
    config = config_flash;

    // Set up USB
    usb_init();
}

void hal_watchdog_feed(void) {
    iwdg_reset();
}

void hal_update_config(void) {
    if (memcmp(&config_flash, &config, sizeof (config) ) == 0) {
        // No write is required today
        return;
    }

    taskDISABLE_INTERRUPTS();
    flash_unlock();
    flash_clear_status_flags();
    flash_erase_page((uint32_t)&config_flash); // Assume it's only one page and is page-aligned
    uint32_t flash_status = flash_get_status_flags();
    configASSERT(flash_status == FLASH_SR_EOP)
    flash_clear_status_flags();

    for(size_t i = 0; i < sizeof (config); i+=4) {
        // Program the word
        flash_program_word((uint32_t)&config_flash + i, *((uint32_t*)((uint32_t)&config + i)));
        flash_status = flash_get_status_flags();
        configASSERT(flash_status == FLASH_SR_EOP);
        flash_clear_status_flags();
        // Verify word was correctly written
        configASSERT(*((uint32_t*)((uint32_t)&config_flash + i)) == *((uint32_t*)((uint32_t)&config + i)));
    }

    flash_lock();
    taskENABLE_INTERRUPTS();
}
