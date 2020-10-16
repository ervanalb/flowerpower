#pragma once

void usb_init(void);

size_t usb_receive(void *buffer, size_t length);
void usb_transmit(void *buffer, size_t length);
