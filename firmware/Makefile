PROJECT = flowerpower
BUILD_DIR = bin

CFILES =

CFILES += src/main.c

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
