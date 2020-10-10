PROJECT = flowerpower
BUILD_DIR = bin

# FreeRTOS
INCLUDES += -IFreeRTOS-Kernel/include
INCLUDES += -IFreeRTOS-Kernel/portable/GCC/ARM_CM0
CFILES += FreeRTOS-Kernel/portable/MemMang/heap_4.c
CFILES += FreeRTOS-Kernel/portable/GCC/ARM_CM0/port.c
CFILES += FreeRTOS-Kernel/list.c
CFILES += FreeRTOS-Kernel/queue.c
CFILES += FreeRTOS-Kernel/tasks.c
CFILES += FreeRTOS-Kernel/timers.c

# Our files
CFILES += src/main.c
CFILES += src/usb.c
CFILES += src/hal.c
CFILES += src/handlers.c

OPT = -O1
LDFLAGS += -Xlinker -Map=$(PROJECT).map
LDFLAGS += -ggdb3

INCLUDES += -Isrc

DEVICE=stm32f042c6t6

OPENCM3_DIR=libopencm3

include $(OPENCM3_DIR)/mk/genlink-config.mk
include rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk

.PHONY: gdb load loadquit flash

gdb: $(PROJECT).elf
	$(PREFIX)gdb $(PROJECT).elf -x init.gdb
load: $(PROJECT).elf
	$(PREFIX)gdb $(PROJECT).elf -x init.gdb -ex load
loadquit: $(PROJECT).elf
	$(PREFIX)gdb $(PROJECT).elf -batch -x init.gdb -ex load -ex kill -ex quit
flash: $(PROJECT).bin
	dfu-util -d 0483:df11 -a 0 -s 0x8000000 -D "$(PROJECT).bin"
