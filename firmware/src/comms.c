#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include "hal.h"
#include "usb.h"

#include "comms.h"

#define MAX_MSG_LEN 256
#define TASK_STACK_SIZE 256 // 1K
#define USB_SEND_TIMEOUT pdMS_TO_TICKS(100)

SemaphoreHandle_t transmit_semaphore = NULL;

static void on_receive(const char *message, size_t length);

static void receive_task(void *pvParameters) {
    (void)pvParameters;
    static char rx_buf[MAX_MSG_LEN];
    bool invalid = false;

    for (;;) {
        // Right now this is just USB receive.
        // But we may need to create a receive queue if there are multiple sources in the future.
        size_t rx_len = usb_receive(rx_buf, MAX_MSG_LEN);
        if (rx_buf[rx_len] == '\n') {
            if (!invalid) {
                on_receive(rx_buf, rx_len);
            }
            invalid = false;
        } else {
            // Discard until the next newline if the message is too long
            invalid = true;
        }
    }
}

void comms_init(void) {
    configASSERT(xTaskCreate(receive_task, "comms_receive", TASK_STACK_SIZE, NULL, 1, NULL) == pdTRUE);
    transmit_semaphore = xSemaphoreCreateMutex();
    configASSERT(transmit_semaphore);
}

void comms_send(const char *message, size_t length) {
    configASSERT(xSemaphoreTake(transmit_semaphore, portMAX_DELAY) == pdTRUE);
    usb_transmit(message, length, USB_SEND_TIMEOUT);
    xSemaphoreGive(transmit_semaphore);
}

static void on_receive(const char *message, size_t length) {
}
