# Makefile for ls(1) - Midterm Project
# Author: Dang Tu Nguyen (Student ID: 24IT181)
# Compatible with BSD make (NetBSD/macOS/FreeBSD) and GNU make (Linux/Windows)

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

OBJS = src/main.o \
       src/options.o \
       src/file_utils.o \
       src/sort.o \
       src/display.o \
       src/traverse.o \
       src/compat.o

TARGET = ls

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).exe

test: $(TARGET)
	@echo "=== Running sanity tests ==="
	./$(TARGET)
	./$(TARGET) -la
	./$(TARGET) -lSh
	./$(TARGET) -F
