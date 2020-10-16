#pragma once

#include <FreeRTOS.h>

enum usb_transmit_result {
    USB_SUCCESS = 0,
    USB_TIMEOUT,
    USB_DISCONNECTED,
};

void usb_init(void);

size_t usb_receive(void *buffer, size_t length);
enum usb_transmit_result usb_transmit(const void *buffer, size_t length, TickType_t ticks_to_wait);
