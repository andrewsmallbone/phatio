MCU = atmega32u4
ARCH = AVR8
F_CPU = 16000000

# LUFA STUFF
F_USB = $(F_CPU)
LUFA_PATH = ./lufa/LUFA
LUFA_OPTS = -D USB_DEVICE_ONLY
LUFA_OPTS += -D FIXED_CONTROL_ENDPOINT_SIZE=8
LUFA_OPTS += -D FIXED_NUM_CONFIGURATIONS=1
LUFA_OPTS += -D USE_FLASH_DESCRIPTORS
#LUFA_OPTS += -D INTERRUPT_CONTROL_ENDPOINT
LUFA_OPTS += -D USE_STATIC_OPTIONS="(USB_DEVICE_OPT_FULLSPEED | USB_OPT_REG_ENABLED | USB_OPT_AUTO_PLL)"
EXTRAINCDIRS = $(LUFA_PATH)


CDEFS +=  -DF_USB=$(F_USB)UL $(LUFA_OPTS)
CDEFS += -D USE_BOOTLOADER_FUNCTIONS

TARGET = phatio

include $(LUFA_PATH)/Build/lufa_sources.mk

SRC = $(TARGET).c \
	  usb.c usb_descriptors.c  \
	  scsi_impl.c disk.c  fat_impl.c  \
	  char_source.c file_char_source.c \
	  log.c keyboard.c config.c\
	  util.c sd.c spi.c pins.c iopin.c adc.c pwm.c scheduler.c \
	  lio_run.c lio.c lio_basics.c lio_references.c lio_builtins.c lio_conditions.c lio_file.c lio_io.c lio_twi.c lio_spi.c \
	  $(LUFA_SRC_USB) $(LUFA_SRC_USBCLASS)

SCANF_LIB=-Wl,-u,vfscanf -lscanf_min
#PRINTF_LIB=-Wl,-u,vfprintf -lprintf_min

include common.mk
