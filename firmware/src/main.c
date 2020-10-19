#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/systick.h>
#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>

#include "hal.h"
#include "comms.h"
#include "heartbeat.h"
#include "state_config.h"
#include "control.h"

int main(void) {
    hal_init();
    comms_init();

    //const struct datetime dt = {
    //    .second = 0,
    //    .minute = 48,
    //    .hour = 15,
    //    .day = 17,
    //    .month = 10,
    //    .year = 20,
    //    .weekday = 7,
    //};
    //rtc_set(&dt);

    state_init();
    heartbeat_init();
    control_init();

    vTaskStartScheduler();

    for (;;);
}
