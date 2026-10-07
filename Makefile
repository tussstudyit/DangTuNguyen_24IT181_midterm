# Makefile for ls(1) - Midterm Project
# Author: Dang Tu Nguyen (Student ID: 24IT181)
# Specification: NetBSD 10.1 ls(1) General Commands Manual

CC ?= gcc
CFLAGS ?= -Wall -Wextra -pedantic -std=c99 -O2
INCLUDES = -Isrc

SRCS = src/main.c \
       src/options.c \
       src/file_utils.c \
       src/sort.c \
       src/display.c \
       src/traverse.c \
       src/compat.c

OBJS = $(SRCS:.c=.o)

TARGET = ls

# Detect Windows environment for executable extension and clean command
ifeq ($(OS),Windows_NT)
    TARGET := $(TARGET).exe
endif

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
ifeq ($(OS),Windows_NT)
	-cmd /c del /Q /F src\*.o $(TARGET) 2>NUL
else
	rm -f src/*.o $(TARGET)
endif

test: $(TARGET)
	@echo "=== Running sanity tests ==="
	./$(TARGET)
	./$(TARGET) -la
	./$(TARGET) -lSh
	./$(TARGET) -F
