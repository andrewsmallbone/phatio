
vpath %.c ../src 

all: files make_fs crc_hexfile

files:
	@./make_header > files.h

make_fs: make_fs.o fat_impl.o

get_crc:
	@./crc_hexfile 

filesystem: files make_fs
	@./make_fs ../$(BOOTFILE).hex > ../$(BOOTFILE).bin
	rm files.h

CC=gcc
CFLAGS += -std=c99 -I. -I../src 
OBJ = $(SRC:%.c=%.o)

clean:
	@rm -f .dep *.o files.h make_fs crc_hexfile
	
