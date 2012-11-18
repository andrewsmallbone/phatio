
vpath %.c ../src 

all: make_fs crc_hexfile

make_fs: make_fs.o fat_impl.o

get_crc:
	@./crc_hexfile 

filesystem: make_fs
	@./make_fs ../$(BOOTFILE).hex > ../$(BOOTFILE).bin

CC=gcc
CFLAGS += -std=c99 -I. -I../src 
OBJ = $(SRC:%.c=%.o)

clean:
	@rm -f .dep *.o make_fs crc_hexfile
	
