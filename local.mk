
# stuff local to the development machine

VOLUME=/Volumes/PHATIO
AVRDUDE = avrdude
AVRDUDE_PROGRAMMER = avrispv2
AVRDUDE_PORT = /dev/cu.usbserial-A6008e7y
UNMOUNT=diskutil quiet unmountDisk $(VOLUME)| echo
DEVICE=$$(diskutil info PHATIO | grep 'Device Node'| cut -d : -f 2 | sed 's/^ *//g' | sed 's/s1//g';)
