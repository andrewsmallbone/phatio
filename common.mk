MCU = atmega32u4
ARCH = AVR8
F_CPU = 16000000

vpath %.c src 
vpath %.S src 
include local.mk

AVRDUDE_WRITE_FLASH = -U flash:w:$(OBJDIR)/$(TARGET).hex
AVRDUDE_WRITE_FUSES = -U lfuse:w:0xde:m -U hfuse:w:0xd8:m -U efuse:w:0xcb:m
AVRDUDE_FLAGS = -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER)


FORMAT = ihex
ROOT_PATH = ./
OBJDIR = ./$(TARGET)
DEBUG = stabs

#CFLAGS = -g$(DEBUG)
CFLAGS += -DF_CPU=$(F_CPU)UL -DARCH=ARCH_$(ARCH) 
CFLAGS += $(CDEFS)
CFLAGS += -Os
CFLAGS += -funsigned-char
CFLAGS += -funsigned-bitfields
CFLAGS += -ffunction-sections
CFLAGS += -fdata-sections
CFLAGS += -fno-inline-small-functions
CFLAGS += -fpack-struct
CFLAGS += -fshort-enums
CFLAGS += -fno-strict-aliasing
CFLAGS += -Werror -Wfatal-errors
#CFLAGS += -Wall
CFLAGS += -Wstrict-prototypes
#CFLAGS += -mshort-calls
#CFLAGS += -fno-unit-at-a-time
#CFLAGS += -Wundef
#CFLAGS += -Wunreachable-code
#CFLAGS += -Wsign-compare
#CFLAGS += -Wa,-adhlns=$(<:%.c=$(OBJDIR)/%.lst)
CFLAGS += -Wa,-adhlns=$(OBJDIR)/$*.lst
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))
CFLAGS += -std=c99

#PRINTF_LIB=-Wl,-u,vfprintf -lprintf_min
#SCANF_LIB=-Wl,-u,vfscanf -lscanf_min
MATH_LIB = -lm
EXTRALIBDIRS =
LDFLAGS += -Wl,--relax
LDFLAGS += -Wl,-Map=$(OBJDIR)/$(TARGET).map,--cref
LDFLAGS += -Wl,--gc-sections 
#LDFLAGS += -Wl,--print-gc-sections

LDFLAGS += $(patsubst %,-L%,$(EXTRALIBDIRS))
LDFLAGS += $(PRINTF_LIB) $(SCANF_LIB) $(MATH_LIB)

SHELL = sh
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
AR = avr-ar rcs
NM = avr-nm
REMOVE = rm -f
REMOVEDIR = rm -rf
COPY = cp
WINSHELL = cmd



OBJ = $(SRC:%.c=$(OBJDIR)/%.o) $(CPPSRC:%.cpp=$(OBJDIR)/%.o) $(ASRC:%.S=$(OBJDIR)/%.o)

LST = $(SRC:%.c=$(OBJDIR)/%.lst) $(CPPSRC:%.cpp=$(OBJDIR)/%.lst) $(ASRC:%.S=$(OBJDIR)/%.lst)


ALL_CFLAGS = -mmcu=$(MCU) -I. $(CFLAGS) -MMD -MP -MF .dep/$(@F).d
ADEFS  = -DF_CPU=$(F_CPU)
ASFLAGS = $(ADEFS) -Wa,-adhlns=$(OBJDIR)/$*.lst,-gstabs,--listing-cont-lines=100
ALL_ASFLAGS = -mmcu=$(MCU) -I. -x assembler-with-cpp $(ASFLAGS)

all: build

build: $(OBJDIR)/$(TARGET).hex

%.hex: %.elf size
	$(OBJCOPY) -O $(FORMAT) -R .eeprom -R .fuse -R .lock $< $@

%.eep: %.elf
	-$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
	--change-section-lma .eeprom=0 --no-change-warnings -O $(FORMAT) $< $@ || exit 0

# Create extended listing file from ELF output file.
%.lss: %.elf
	$(OBJDUMP) -h -S -z $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	$(NM) -n $< > $@

# Create library from object files.
.SECONDARY : $(TARGET).a
.PRECIOUS : $(OBJ)
%.a: $(OBJ)
	$(AR) $@ $(OBJ)

$(OBJDIR)/%.o : %.S
	$(CC) -c $(ALL_ASFLAGS) $< -o $@

# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(OBJ)
%.elf: $(OBJ)
	$(CC) $(ALL_CFLAGS) $^ --output $@ $(LDFLAGS)

$(OBJDIR)/%.o : $(ROOT_PATH)/%.c 
	mkdir -p $(dir $@) 
	$(CC) -c $(ALL_CFLAGS) $< -o $@ 


# Listing of phony targets.
.PHONY : build flash size clean all 

# Create object files directory
$(shell mkdir $(OBJDIR) 2>/dev/null)

# Include the dependency files.
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

clean:
	@$(REMOVE) -rf $(TARGET)
	@$(REMOVEDIR) .dep

size: $(OBJDIR)/$(TARGET).elf
	@avr-size $(OBJDIR)/$(TARGET).elf
	@avr-size $(OBJDIR)/$(TARGET).elf | tail -1 | awk '{print "FLASH: " $$1+$$2 "  RAM: " $$2+$$3}'

flash: build 
	time $(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FUSES) $(AVRDUDE_WRITE_FLASH) 
	
avarice:
	avarice --mkII --jtag usb --part $(MCU) --debug :4242
