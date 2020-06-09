CC=g++

basidbg: linenoise.o
	$(CC) -Wall -std=c++17 src/basicdbg.cpp src/debugger.cpp src/breakpoint.cpp build/linenoise.o ext/libelfin/elf/libelf++.a ext/libelfin/dwarf/libdwarf++.a -o basicdbg

linenoise.o:
	gcc -c ext/linenoise/linenoise.c -o build/linenoise.o

hello:
	$(CC) -g examples/hello.cpp -o examples/hello

clean:
	rm build/*
	rm basicdbg
