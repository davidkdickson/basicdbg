CC=g++

basidbg: linenoise.o
	$(CC) -Wall -std=c++17 src/basicdbg.cc src/debugger.cc src/breakpoint.cc build/linenoise.o -o basicdbg -L./libs/libelfin/dwarf -ldwarf++ -L./libs/libelfin/elf -lelf++ -Wl,-rpath,./libs/libelfin/elf -Wl,-rpath,./libs/libelfin/dwarf


libelfin:
	$(MAKE) -C libs/libelfin

linenoise.o:
	gcc -c libs/linenoise/linenoise.c -o build/linenoise.o

hello:
	$(CC) -g examples/hello.cc -o examples/hello

stack_trace:
	$(CC) -g examples/stack_trace.cc -o examples/stack_trace
clean:
	rm build/*
	rm basicdbg
