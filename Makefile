CC=g++
CFLAGS=-g -Wall -std=c++17
DEPS = $(wildcard src/*h)
SRC = $(wildcard src/*.cc)

basicdbg: libelfin examples
	$(CC) $(CFLAGS) $(SRC) -o $@ -L./libs/libelfin/dwarf -ldwarf++ -L./libs/libelfin/elf -lelf++ -Wl,-rpath,./libs/libelfin/elf -Wl,-rpath,./libs/libelfin/dwarf


libelfin:
	$(MAKE) -C libs/libelfin

examples: hello stack_trace

hello:
	$(CC) $(CFLAGS) -fno-omit-frame-pointer examples/hello.cc -o examples/hello

stack_trace:
	$(CC) $(CFLAGS) -fno-omit-frame-pointer examples/stack_trace.cc -o examples/stack_trace
clean:
	rm basicdbg examples/stack_trace examples/hello
