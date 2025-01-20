CC=gcc
CFLAGS=-Wall -Wextra -Wswitch-enum -std=c11 -pedantic
LIBS=

cvm: main.c
	$(CC) $(CFLAGS) -o cvm main.c $(LIBS)