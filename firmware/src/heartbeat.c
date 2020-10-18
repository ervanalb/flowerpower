#include "heartbeat.h"

#include <FreeRTOS.h>
#include <task.h>

#include "hal.h"

static void heartbeat_task(void *pvParameters) {
    (void)pvParameters;
    int counter = 0;
    for (;;) {
        hal_watchdog_feed();

        if (counter < 2) {
            hal_heartbeat_on();
        } else {
            hal_heartbeat_off();
        }

        counter++;
        if (counter >= 10) {
            counter = 0;
        }

        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

void heartbeat_init(void) {
    configASSERT(xTaskCreate(heartbeat_task, "heartbeat", configMINIMAL_STACK_SIZE, NULL, 1, NULL) == pdTRUE);
}
