#include <iostream>
#include <unistd.h>
#include <sys/ptrace.h>

#include "debugger.h"

#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */

int main(int argc, char *argv[])
{
  if (argc < 2) {
    std::cerr << RED << "error: program not specified" << RESET << std::endl;
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
