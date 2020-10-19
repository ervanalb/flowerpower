#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/cm3/nvic.h>
#include <stddef.h>

#include <FreeRTOS.h>
#include <task.h>
#include "usb.h"
#include "state_config.h"

// Index for TaskNotify calls
#define NOTIFY_RX_DONE 0
#define NOTIFY_TX_DONE 1

#define BULK_PACKET_SIZE 64

static usbd_device *usbd_dev_handle;
static uint8_t usbd_control_buffer[128];

// Variables for USB Transmission
static const void *transmit_buffer;
static size_t transmit_buffer_index;
static size_t transmit_length;
static TaskHandle_t transmit_task = NULL;

// Variables for USB Receiving
static void *receive_buffer;
static size_t receive_buffer_index;
static size_t receive_max_length;
static TaskHandle_t receive_task = NULL;

// Try to keep track of whether the device is disconnected or connected.
static bool disconnected = true;

static const struct usb_device_descriptor dev = {
  .bLength = USB_DT_DEVICE_SIZE,
  .bDescriptorType = USB_DT_DEVICE,
  .bcdUSB = 0x0200,
  .bDeviceClass = USB_CLASS_CDC,
  .bDeviceSubClass = 0,
  .bDeviceProtocol = 0,
  .bMaxPacketSize0 = BULK_PACKET_SIZE,
  .idVendor = 0x0483,
  .idProduct = 0x5740,
  .bcdDevice = 0x0200,
  .iManufacturer = 1,
  .iProduct = 2,
  .iSerialNumber = 3,
  .bNumConfigurations = 1,
};

/*
 * This notification endpoint isn't implemented. According to CDC spec its
 * optional, but its absence causes a NULL pointer dereference in Linux
 * cdc_acm driver.
 */
static const struct usb_endpoint_descriptor comm_endp[] = {
  {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,
    .bEndpointAddress = 0x83,
    .bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
    .wMaxPacketSize = 16,
    .bInterval = 255,
  }
};

static const struct usb_endpoint_descriptor data_endp[] = {
  {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,
    .bEndpointAddress = 0x01,
    .bmAttributes = USB_ENDPOINT_ATTR_BULK,
    .wMaxPacketSize = BULK_PACKET_SIZE,
    .bInterval = 1,
  },
  {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,
    .bEndpointAddress = 0x82,
    .bmAttributes = USB_ENDPOINT_ATTR_BULK,
    .wMaxPacketSize = BULK_PACKET_SIZE,
    .bInterval = 1,
  }
};

static const struct {
  struct usb_cdc_header_descriptor header;
  struct usb_cdc_call_management_descriptor call_mgmt;
  struct usb_cdc_acm_descriptor acm;
  struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed)) cdcacm_functional_descriptors = {
  .header = {
    .bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
    .bDescriptorType = CS_INTERFACE,
    .bDescriptorSubtype = USB_CDC_TYPE_HEADER,
    .bcdCDC = 0x0110,
  },
  .call_mgmt = {
    .bFunctionLength = 
    sizeof(struct usb_cdc_call_management_descriptor),
    .bDescriptorType = CS_INTERFACE,
    .bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
    .bmCapabilities = 0,
    .bDataInterface = 1,
  },
  .acm = {
    .bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
    .bDescriptorType = CS_INTERFACE,
    .bDescriptorSubtype = USB_CDC_TYPE_ACM,
    .bmCapabilities = 0,
  },
  .cdc_union = {
    .bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
    .bDescriptorType = CS_INTERFACE,
    .bDescriptorSubtype = USB_CDC_TYPE_UNION,
    .bControlInterface = 0,
    .bSubordinateInterface0 = 1, 
  }
};

static const struct usb_interface_descriptor comm_iface[] = {
  {
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = 0,
    .bAlternateSetting = 0,
    .bNumEndpoints = 1,
    .bInterfaceClass = USB_CLASS_CDC,
    .bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
    .bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
    .iInterface = 0,
    .endpoint = comm_endp,
    .extra = &cdcacm_functional_descriptors,
    .extralen = sizeof(cdcacm_functional_descriptors)
  }
};

static const struct usb_interface_descriptor data_iface[] = {
  {
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = 1,
    .bAlternateSetting = 0,
    .bNumEndpoints = 2,
    .bInterfaceClass = USB_CLASS_DATA,
    .bInterfaceSubClass = 0,
    .bInterfaceProtocol = 0,
    .iInterface = 0,
    .endpoint = data_endp,
  }
};

static const struct usb_interface ifaces[] = {
  {
    .num_altsetting = 1,
    .altsetting = comm_iface,
  },
  {
    .num_altsetting = 1,
    .altsetting = data_iface,
  }
};

static const struct usb_config_descriptor usb_config = {
  .bLength = USB_DT_CONFIGURATION_SIZE,
  .bDescriptorType = USB_DT_CONFIGURATION,
  .wTotalLength = 0,
  .bNumInterfaces = 2,
  .bConfigurationValue = 1,
  .iConfiguration = 0,
  .bmAttributes = 0x80,
  .bMaxPower = 0x32,
  .interface = ifaces,
};

static const char *usb_strings[] = {
  "Black Sphere Technologies",
  "CDC-ACM Demo",
  "DEMO",
};

static size_t packet_rx_size;
static uint8_t packet_rx_buffer[BULK_PACKET_SIZE];

static void process_rx_buffer(BaseType_t *higher_priority_task_woken) {
    configASSERT(receive_buffer != NULL);
    configASSERT(receive_task != NULL);

    size_t i;
    for (i = 0; i < packet_rx_size; i++) {
        ((uint8_t *)receive_buffer)[receive_buffer_index] = packet_rx_buffer[i];
        receive_buffer_index++;

        if (packet_rx_buffer[i] == '\n' || receive_buffer_index >= receive_max_length) {
            break;
        }
    }
    if (i < packet_rx_size) {
        // We broke out, which means a packet is complete
        for(size_t j = i + 1; j < packet_rx_size; j++) {
            packet_rx_buffer[j - i - 1] = packet_rx_buffer[j];
        }
        packet_rx_size -= i + 1;

        TaskHandle_t task_to_notify = receive_task;
        receive_task = NULL;
        receive_buffer = NULL;

        if (higher_priority_task_woken == NULL) {
            xTaskNotifyGiveIndexed(task_to_notify,
                                   NOTIFY_RX_DONE);
        } else {
            vTaskNotifyGiveIndexedFromISR(task_to_notify,
                                          NOTIFY_RX_DONE,
                                          higher_priority_task_woken);
        }
    } else {
        // We didn't break out
        // This means a packet is not complete and we should receive more.
        packet_rx_size = 0;
        usbd_ep_nak_set(usbd_dev_handle, 0x01, 0); // Stop NAKing
    }
}

static bool transmit_done;
static size_t transmit_size;
static size_t transmit_index;

static void process_tx_buffer(BaseType_t *higher_priority_task_woken) {
    if (transmit_buffer == NULL || transmit_task == NULL) {
        // The previous call likely timed out.
        // Mark the device as connected, but don't do anything.
        disconnected = false;
        return;
    }

    if (transmit_done) {
        // We are done transmitting
        
        TaskHandle_t task_to_notify = transmit_task;
        transmit_task = NULL;
        transmit_buffer = NULL;

        if (higher_priority_task_woken == NULL) {
            xTaskNotifyGiveIndexed(task_to_notify,
                                   NOTIFY_TX_DONE);
        } else {
            vTaskNotifyGiveIndexedFromISR(task_to_notify,
                                          NOTIFY_TX_DONE,
                                          higher_priority_task_woken);
        }
    } else {
        // This is a first packet, or a continuation

        uint8_t *ptr = (uint8_t *)(&transmit_buffer)[transmit_index];
        size_t size = transmit_size - transmit_index;
        if (size > BULK_PACKET_SIZE) {
            size = BULK_PACKET_SIZE;
        }

        usbd_ep_write_packet(usbd_dev_handle, 0x82, ptr, size);

        transmit_index += size;

        // We always send a ZLP if ending on a multiple of 64
        // not sure if this is necessary
        transmit_done = (size < BULK_PACKET_SIZE);
    }
}

static void protocol_rx(usbd_device *usbd_dev, uint8_t ep) {
    (void)ep;

    configASSERT(packet_rx_size == 0);

    usbd_ep_nak_set(usbd_dev_handle, 0x01, 1); // Don't assume we can take more data
    packet_rx_size = usbd_ep_read_packet(usbd_dev, 0x01, packet_rx_buffer, BULK_PACKET_SIZE);

    BaseType_t higher_priority_task_woken = pdFALSE;

    process_rx_buffer(&higher_priority_task_woken);

    portYIELD_FROM_ISR(higher_priority_task_woken);
}

static void protocol_tx(usbd_device *usbd_dev, uint8_t ep) {
    (void)usbd_dev;
    (void)ep;

    BaseType_t higher_priority_task_woken = pdFALSE;

    process_tx_buffer(&higher_priority_task_woken);

    portYIELD_FROM_ISR(higher_priority_task_woken);
}

size_t usb_receive(void *buffer, size_t length) {
    configASSERT(buffer != NULL);
    configASSERT(receive_buffer == NULL);
    configASSERT(receive_task == NULL);

    receive_task = xTaskGetCurrentTaskHandle();
    receive_buffer = buffer;
    receive_max_length = length;
    receive_buffer_index = 0;

    process_rx_buffer(NULL);

    ulTaskNotifyTakeIndexed(NOTIFY_RX_DONE, pdTRUE, portMAX_DELAY);
    // Null-terminate if there is space
    if (receive_buffer_index < length) {
        ((char *)buffer)[receive_buffer_index] = '\0';
    }
    return receive_buffer_index;
}

enum usb_transmit_result usb_transmit(const void *buffer, size_t length, TickType_t ticks_to_wait) {
    configASSERT(buffer != NULL);
    configASSERT(transmit_buffer == NULL);
    configASSERT(transmit_task == NULL);

    if (disconnected) {
        return USB_DISCONNECTED;
    }

    transmit_task = xTaskGetCurrentTaskHandle();
    transmit_buffer = buffer;
    transmit_size = length;
    transmit_index = 0;
    transmit_done = false;
    process_tx_buffer(NULL);

    int result = ulTaskNotifyTakeIndexed(NOTIFY_TX_DONE, pdTRUE, ticks_to_wait);
    if (result == 0) {
        // A TIMEOUT marks the device as disconnected.
        disconnected = true;
        transmit_task = NULL;
        transmit_buffer = NULL;
        return USB_TIMEOUT;
    }
    return USB_SUCCESS;
}

static enum usbd_request_return_codes cdcacm_control_request(
    usbd_device *handle,
    struct usb_setup_data *req,
    uint8_t **buf,
    uint16_t *len,
    void (**complete)(usbd_device *usbd_dev_handle, struct usb_setup_data *req)
) {
  (void)complete;
  (void)buf;
  (void)handle;

  switch(req->bRequest) {
  case USB_CDC_REQ_SET_CONTROL_LINE_STATE: {
    /*
     * This Linux cdc_acm driver requires this to be implemented
     * even though it's optional in the CDC spec, and we don't
     * advertise it in the ACM functional descriptor.
     */
    char local_buf[10];
    struct usb_cdc_notification *notif = (void *)local_buf;

    /* We echo signals back to host as notification. */
    notif->bmRequestType = 0xA1;
    notif->bNotification = USB_CDC_NOTIFY_SERIAL_STATE;
    notif->wValue = 0;
    notif->wIndex = 0;
    notif->wLength = 2;
    local_buf[8] = req->wValue & 3;
    local_buf[9] = 0;
    // usbd_ep_write_packet(0x83, buf, 10);
    return USBD_REQ_HANDLED;
  }
  case USB_CDC_REQ_SET_LINE_CODING: 
    if(*len < sizeof(struct usb_cdc_line_coding)) {
      return USBD_REQ_NOTSUPP;
    }
    return USBD_REQ_HANDLED;
  }
  return 0;
}

static void cdcacm_set_config(usbd_device *handle, uint16_t wValue) {
    (void)wValue;
    (void)handle;

    usbd_ep_setup(usbd_dev_handle, 0x01, USB_ENDPOINT_ATTR_BULK, BULK_PACKET_SIZE, protocol_rx);
    usbd_ep_setup(usbd_dev_handle, 0x82, USB_ENDPOINT_ATTR_BULK, BULK_PACKET_SIZE, protocol_tx);
    usbd_ep_setup(usbd_dev_handle, 0x83, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);

    disconnected = false;

    usbd_register_control_callback(usbd_dev_handle,
                                   USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
                                   USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
                                   cdcacm_control_request);
}

void usb_init(void) {
    rcc_periph_clock_enable(RCC_USB);

    usbd_dev_handle = usbd_init(&st_usbfs_v2_usb_driver,
               &dev,
               &usb_config,
               usb_strings,
               3,
               usbd_control_buffer,
               sizeof(usbd_control_buffer));
    usbd_register_set_config_callback(usbd_dev_handle, cdcacm_set_config);

    nvic_set_priority(NVIC_USB_IRQ, 1 << 2);
    nvic_enable_irq(NVIC_USB_IRQ);
}


void usb_isr(void) {
    usbd_poll(usbd_dev_handle);
}
