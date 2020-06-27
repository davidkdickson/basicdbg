CC=g++

basidbg: libelfin
	$(CC) -g -Wall -std=c++17 src/stepper.cc src/register.cc src/debug_info.cc src/basicdbg.cc src/debugger.cc src/breakpoint.cc -o basicdbg -L./libs/libelfin/dwarf -ldwarf++ -L./libs/libelfin/elf -lelf++ -Wl,-rpath,./libs/libelfin/elf -Wl,-rpath,./libs/libelfin/dwarf


libelfin:
	$(MAKE) -C libs/libelfin

examples: hello stack_trace

hello:
	$(CC) -g -fno-omit-frame-pointer examples/hello.cc -o examples/hello

stack_trace:
	$(CC) -g -fno-omit-frame-pointer examples/stack_trace.cc -o examples/stack_trace
clean:
	rm basicdbg examples/stack_trace examples/hello
