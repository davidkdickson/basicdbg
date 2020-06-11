CC=g++

basidbg: linenoise.o libelfin
	$(CC) -Wall -std=c++17 src/basicdbg.cpp src/debugger.cpp src/breakpoint.cpp build/linenoise.o libs/libelfin/elf/libelf++.so libs/libelfin/dwarf/libdwarf++.so -o basicdbg

libelfin:
	$(MAKE) -C libs/libelfin

linenoise.o:
	gcc -c libs/linenoise/linenoise.c -o build/linenoise.o

hello:
	$(CC) -g examples/hello.cpp -o examples/hello

stack_trace:
	$(CC) -g examples/stack_trace.cpp -o examples/stack_trace
clean:
	rm build/*
	rm basicdbg
