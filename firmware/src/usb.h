#pragma once

void usb_init(void);

//void usb_send(const void *buffer, size_t length);
size_t usb_receive(void *buffer, size_t length);
