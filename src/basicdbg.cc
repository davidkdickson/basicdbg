#include <iostream>
#include <unistd.h>
#include <sys/ptrace.h>
#include <iostream>
#include <fstream>

#include "debugger.h"
#include "debug_info.h"
#include "stepper.h"

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
    std::string file = "/proc/" + std::to_string(pid) + "/maps";
    std::ifstream infile(file);
    std::string sLine;

    if (infile.good())
    {
      getline(infile, sLine);
    }

    infile.close();
    char *p;

    uint64_t start_address = strtol(sLine.substr(0, 12).c_str(), &p, 16);

    DebugInfo debug_info(prog);
    Stepper stepper {pid, start_address, debug_info};
    Debugger dbg {pid, start_address, debug_info, stepper};
    dbg.run();
  }

  return 0;
}
