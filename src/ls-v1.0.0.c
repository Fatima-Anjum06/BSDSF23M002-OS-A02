# Makefile for ls-v1.0.0
# Author: BSDSF23M002

CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = myls
SRC = src/ls-v1.0.0.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
