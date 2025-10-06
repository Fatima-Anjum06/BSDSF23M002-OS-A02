# Makefile for ls-v1.0.0
# Author: BSDSF23M002

CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = myls

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c

clean:
	rm -f $(TARGET)
