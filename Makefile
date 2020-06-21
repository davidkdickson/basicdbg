CC=g++

basidbg: linenoise.o
	$(CC) -g -Wall -std=c++17 src/stepper.cc src/register.cc src/debug_info.cc src/basicdbg.cc src/debugger.cc src/breakpoint.cc  build/linenoise.o -o basicdbg -L./libs/libelfin/dwarf -ldwarf++ -L./libs/libelfin/elf -lelf++ -Wl,-rpath,./libs/libelfin/elf -Wl,-rpath,./libs/libelfin/dwarf


libelfin:
	$(MAKE) -C libs/libelfin

linenoise.o:
	gcc -g -c libs/linenoise/linenoise.c -o build/linenoise.o

hello:
	$(CC) -g -fno-omit-frame-pointer examples/hello.cc -o examples/hello

stack_trace:
	$(CC) -g -fno-omit-frame-pointer examples/stack_trace.cc -o examples/stack_trace
clean:
	rm build/*
	rm basicdbg
