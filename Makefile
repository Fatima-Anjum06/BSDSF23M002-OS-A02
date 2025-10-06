# Makefile for ls-v1.0.0
# Author: BSDSF23M002

CC = gcc
CFLAGS = -Wall -Wextra -std=c11
SRC = src/ls-v1.1.0.c
OBJ = obj/ls-v1.1.0.o
BIN = bin/ls

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJ)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

clean:
	rm -f $(OBJ) $(BIN)
