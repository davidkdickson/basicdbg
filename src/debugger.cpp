#include "debugger.h"

#include <sys/user.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "breakpoint.h"
#include "linenoise.h"

std::vector<std::string> split(const std::string &s, char delimiter) {
  std::vector<std::string> out{};
  std::stringstream ss{s};
  std::string item;

  while (std::getline(ss, item, delimiter)) {
    out.push_back(item);
  }

  return out;
}

bool is_prefix(const std::string &s, const std::string &of) {
  if (s.size() > of.size()) return false;
  return std::equal(s.begin(), s.end(), of.begin());
}

void Debugger::run() {
  // waiting for child process to finish launching
  int wait_status;
  auto options = 0;
  waitpid(m_pid, &wait_status, options);
  char *line = nullptr;

  while ((line = linenoise("basicdbg> ")) != nullptr) {
    handle_command(line);
    linenoiseHistoryAdd(line);
    linenoiseFree(line);
  }

  return;
}

void Debugger::handle_command(const std::string &line) {
  auto args = split(line, ' ');
  auto command = args[0];

  if (is_prefix(command, "cont")) {
    continue_execution();

  } else if (is_prefix(command, "break")) {
    std::string addr {args[1], 2};
    set_breakpoint(std::stol(addr, 0, 16));

  } else {
    std::cerr << "Unknown command\n";
  }
}

void Debugger::continue_execution() {

  std::cout << "Continuing execution of process: " << m_pid << std::endl;
  step_over_breakpoint();
  ptrace(PTRACE_CONT, m_pid, nullptr, nullptr);

  int wait_status;
  auto options = 0;
  waitpid(m_pid, &wait_status, options);
}

void Debugger::set_breakpoint(std::intptr_t address) {
  Breakpoint bp(m_pid, address);
  bp.print();
  bp.enable();
  m_breakpoints[address] = bp;
}

void Debugger::step_over_breakpoint() {
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, m_pid, nullptr, &regs);

    auto breakpoint_location = regs.rip - 1;

    if (!m_breakpoints.count(breakpoint_location)) {
      return;
    }

    std::cout << "at breakpoint" << std::endl;

    auto& bp = m_breakpoints[breakpoint_location];

    if(bp.is_enabled()) {
      regs.rip = breakpoint_location;
      ptrace(PTRACE_SETREGS, m_pid, nullptr, &regs);
      ptrace(PTRACE_SINGLESTEP, m_pid, nullptr, nullptr);
      bp.disable();
      int wait_status;
      auto options = 0;
      waitpid(m_pid, &wait_status, options);
      bp.enable();
    }
}
