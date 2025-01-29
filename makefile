CC=gcc
CFLAGS=-Wall -Wextra -Wswitch-enum -std=c11 -pedantic
LIBS=

.PHONY: all
all:  cvmasm cvmi

cvmasm: ./src/cvmasm.c ./src/cvm.c
	$(CC) $(CFLAGS) -o cvmasm ./src/cvmasm.c $(LIBS)

cvmi: ./src/cvmi.c ./src/cvm.c
	$(CC) $(CFLAGS) -o cvmi ./src/cvmi.c $(LIBS)

.PHONY: examples
examples: ./examples/fib.cvm ./examples/123.cvm

./examples/fib.cvm: cvmasm ./examples/fib.cvmasm cvmasm
	./cvmasm ./examples/fib.cvmasm ./examples/fib.cvm 

./examples/123.cvm: cvmasm ./examples/123.cvmasm cvmasm
	./cvmasm ./examples/123.cvmasm ./examples/123.cvm