#include <iostream>
#include <unistd.h>
#include <sys/ptrace.h>

#include "debugger.h"

int main(int argc, char *argv[])
{
  if (argc < 2) {
    std::cerr << "Program not specified";
    return -1;
  }

  auto prog = argv[1];
  auto pid = fork();

  if (pid == 0) {
    ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
    execl(prog, prog, nullptr);
  } else if (pid >= 1) {
    std::cout << "basicdbg> debugging process: " << pid << std::endl;
    Debugger dbg {prog, pid};
    dbg.run();
  }

  return 0;
}
