#ifndef STEPPER_H
#define STEPPER_H

#include <sys/wait.h>

#include <unordered_map>

#include "breakpoint.h"
#include "debug_info.h"
#include "register.h"

class Stepper {
  private:
  pid_t m_pid;
  uint64_t m_start_address;
  DebugInfo& m_debug_info;

  void handle_sigtrap(siginfo_t info);

  void single_step_instruction();
  void step_over_breakpoint(std::unordered_map<std::intptr_t, Breakpoint>& breakpoints);
  void single_step_instruction_with_breakpoint_check(std::unordered_map<std::intptr_t, Breakpoint>& breakpoints);

  public:
  Stepper(pid_t pid, uint64_t start_address, DebugInfo& debug_info)
      : m_pid{pid}, m_start_address{start_address}, m_debug_info(debug_info) {}

  void wait_for_signal();

  void step_in(std::unordered_map<std::intptr_t, Breakpoint>& breakpoints);
  void step_over();
  void step_out(std::unordered_map<std::intptr_t, Breakpoint>& breakpoints);

  void continue_execution(std::unordered_map<std::intptr_t, Breakpoint>& breakpoints);

  void remove_breakpoint(std::unordered_map<std::intptr_t, Breakpoint>& breakpoints, std::intptr_t addr);
  void set_breakpoint(std::unordered_map<std::intptr_t, Breakpoint>& breakpoints, std::intptr_t addr);
};

#endif /* STEPPER_H */
