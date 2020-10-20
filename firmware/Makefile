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
CFILES += src/comms.c
CFILES += src/control.c
CFILES += src/hal.c
CFILES += src/handlers.c
CFILES += src/heartbeat.c
CFILES += src/main.c
CFILES += src/state_config.c
CFILES += src/usb.c

CSTD = -std=gnu99
OPT = -O1
LDFLAGS += -Xlinker -Map=$(PROJECT).map
LDFLAGS += -ggdb3

INCLUDES += -Isrc

LDSCRIPT = stm32f042c6t6.ld
OPENCM3_DEFS=-DSTM32F0 -DSTM32F042C6T6 -D_ROM=32K -D_RAM=6K -D_ROM_OFF=0x08000000 -D_RAM_OFF=0x20000000
OPENCM3_LIB=opencm3_stm32f0
ARCH_FLAGS=-mcpu=cortex-m0 -mthumb -msoft-float

OPENCM3_DIR=libopencm3

CFLAGS += -D_DEBUG=1

include rules.mk

.PHONY: gdb load loadquit flash

gdb: $(PROJECT).elf
	$(PREFIX)gdb $(PROJECT).elf -x init.gdb
load: $(PROJECT).elf
	$(PREFIX)gdb $(PROJECT).elf -x init.gdb -ex load
loadquit: $(PROJECT).elf
	$(PREFIX)gdb $(PROJECT).elf -batch -x init.gdb -ex load -ex kill -ex quit
flash: $(PROJECT).bin
	dfu-util -d 0483:df11 -a 0 -s 0x8000000 -D "$(PROJECT).bin"
