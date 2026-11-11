# Simple Makefile for imgconv
#   make        build the program
#   make clean  delete the program and object files

CC     = gcc
CFLAGS = -std=c99 -Wall -Wextra -pedantic
TARGET = imgconv

SRC = src/main.c src/cli.c src/file_io.c src/image_convert.c src/modes.c src/validation.c src/input.c
OBJ = $(SRC:.c=.o)
HDR = $(wildcard src/*.h)

all: $(TARGET)

# link all object files into the program
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# compile each .c file into a .o file (rebuild if any header changed)
%.o: %.c $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
