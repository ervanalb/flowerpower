#include "state.h"

#include <FreeRTOS.h>
#include <task.h>
#include <stddef.h>

#include "config.h"
#include "comms.h"

struct state state = STATE_DEFAULTS;

#define TASK_STACK_SIZE 160

static void publish_state_task(void *pvParameters) {
    (void)pvParameters;
    static struct state last_state;
    static struct config last_config;

    int counter = 0;
    for (;;) {
        bool send_all = false;
        counter++;
        if (counter >= 100) {
            counter = 0;
            send_all = true;
        }

        if (send_all || state.relay1 != last_state.relay1) {
            comms_printf("relay/1/state %d\n", state.relay1);
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void state_init(void) {
    configASSERT(xTaskCreate(publish_state_task, "publish_state", TASK_STACK_SIZE, NULL, 1, NULL) == pdTRUE);
}
