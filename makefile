CC=gcc
CFLAGS=-Wall -Wextra -Wswitch-enum -std=c11 -pedantic
LIBS=

.PHONY: all
all:  cvmasm cvmi decvmasm

cvmasm: ./src/cvmasm.c ./src/cvm.c
	$(CC) $(CFLAGS) -o cvmasm ./src/cvmasm.c $(LIBS)

cvmi: ./src/cvmi.c ./src/cvm.c
	$(CC) $(CFLAGS) -o cvmi ./src/cvmi.c $(LIBS)

decvmasm: ./src/decvmasm.c ./src/cvm.c
	$(CC) $(CFLAGS) -o decvmasm ./src/decvmasm.c $(LIBS)

.PHONY: examples
examples: ./examples/fib.cvm ./examples/123.cvm ./examples/label.cvm ./examples/stack.cvm ./examples/comsandlabs.cvm

./examples/fib.cvm: cvmasm ./examples/fib.cvmasm cvmasm
	./cvmasm ./examples/fib.cvmasm ./examples/fib.cvm 

./examples/123.cvm: cvmasm ./examples/123.cvmasm cvmasm
	./cvmasm ./examples/123.cvmasm ./examples/123.cvm

./examples/label.cvm: cvmasm ./examples/label.cvmasm cvmasm
	./cvmasm ./examples/label.cvmasm ./examples/label.cvm

./examples/stack.cvm: cvmasm ./examples/stack.cvmasm cvmasm
	./cvmasm ./examples/stack.cvmasm ./examples/stack.cvm

./examples/comsandlabs.cvm: cvmasm ./examples/comsandlabs.cvmasm cvmasm
	./cvmasm ./examples/comsandlabs.cvmasm ./examples/comsandlabs.cvm