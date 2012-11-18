
TARGET = bootloader
BOOTSTART = 7000

EXTRAINCDIRS = ./lufa

LDFLAGS += -Wl,--section-start=.text=$(BOOTSTART) -Tbootloader.link 
CDEFS += -DBOOT_START_ADDR=$(BOOTSTART)UL

SRC = $(TARGET).c  fat_impl.c sd.c spi.c bootloader_sdihex.c 
ASRC = bootloader_isr.S

include common.mk

