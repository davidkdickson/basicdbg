basidbg: linenoise.o
	g++ -Wall src/basicdbg.cpp build/linenoise.o -o basicdbg -Iext/linenoise

linenoise.o:
	gcc -c ext/linenoise/linenoise.c -o build/linenoise.o

clean:
	rm build/*
	rm basicdbg

