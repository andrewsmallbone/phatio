
include local.mk
VERSION=P001

.PHONY : test clean bootloader flash-bootloader support phatio phatio-filename newfs

all: test support bootloader phatio filesystem 

test:
	(cd test; make run)

support:
	(cd support; make all)

clean:
	@make -f bootloader.mk clean
	@make -f phatio.mk clean
	@(cd test; make clean)
	@(cd support; make clean)
	rm -f *.hex *.bin

#
# bootloader
#
bootloader:
	make -f bootloader.mk build

flash-bootloader: bootloader
	$(shell $(UNMOUNT))
	make -f bootloader.mk flash fuses
	
#
# phatio
#

# lufa git repository is readonly copy from svn - so just download source 
lufa:
	wget -O - https://github.com/abcminiuser/lufa/archive/LUFA-120730.tar.gz | tar xvf -
	mv lufa-LUFA-120730 lufa


phatio: support lufa
	@make -f phatio.mk build
	@rm -f *.hex
	@cp phatio/phatio.hex `make phatio-filename`.hex
	@ls -l *.hex

phatio-filename:
	@echo $(VERSION)`cat phatio/phatio.hex | ./support/crc_hexfile`

# create the hex file ready for copying to phatio filesystem
# and a new filesystem containing it
filesystem: support phatio
	@(cd support; make filesystem BOOTFILE=$(shell make phatio-filename))
	@ls -l `make phatio-filename`.{hex,bin}

# wipe phatio with new filesystem
newfs: filesystem
	$(shell $(UNMOUNT))
	sudo dd if=$(shell make phatio-filename).bin of=$(DEVICE)
	
# just copy new boot file to phatio board
cpphatio: phatio
	rm $(VOLUME)/io/boot/*
	cp $(shell make phatio-filename).hex $(VOLUME)/io/boot/
	$(shell $(UNMOUNT))
	
	
combined: phatio bootloader
	srec_cat $(shell make phatio-filename).hex -I bootloader/bootloader.hex -I -o combined-$(shell make phatio-filename).hex -I
	
		



