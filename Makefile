basidbg: linenoise.o
	g++ -Wall -std=c++17 src/basicdbg.cpp src/debugger.cpp src/breakpoint.cpp build/linenoise.o -o basicdbg -Iext/linenoise

linenoise.o:
	gcc -c ext/linenoise/linenoise.c -o build/linenoise.o

clean:
	rm build/*
	rm basicdbg

