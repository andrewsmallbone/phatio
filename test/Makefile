
# tested code is above
vpath %.c ../src

TARGET = test_suite

run: $(TARGET)
	@./$(TARGET)
	@make clean

SRC = $(TARGET).c \
	test_keyboard.c keyboard.c \
	test_util.c util.c \
	test_fat_impl.c fat_impl.c \
	char_source.c \
	lio.o test_lio.c
			
CFLAGS = -std=c99 -Iheaders -I../src

OBJ = $(SRC:%.c=%.o)

CC = gcc

$(TARGET): $(OBJ)
	gcc $(OBJ) --output $@

clean:
	@rm -f *.o $(TARGET)
	
