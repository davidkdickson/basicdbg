#include "stepper.h"
#include "register.h"

#include <sys/ptrace.h>

#include <iostream>

void Stepper::step_in(std::unordered_map<std::intptr_t, Breakpoint>& breakpoints) {

  auto line = m_debug_info.get_line_entry_from_pc(get_pc(m_pid) - m_start_address)->line;

  while (m_debug_info.get_line_entry_from_pc(get_pc(m_pid) - m_start_address)->line == line) {
    single_step_instruction_with_breakpoint_check(breakpoints);
  }

  auto line_entry = m_debug_info.get_line_entry_from_pc(get_pc(m_pid) - m_start_address);
  m_debug_info.print_source(line_entry->file->path, line_entry->line);
}

void Stepper::step_over() {
}

void Stepper::step_out() {
}

void Stepper::wait_for_signal() {
  int wait_status;
  auto options = 0;
  waitpid(m_pid, &wait_status, options);

  siginfo_t siginfo;
  ptrace(PTRACE_GETSIGINFO, m_pid, nullptr, &siginfo);

  switch (siginfo.si_signo) {
    case SIGTRAP:
      handle_sigtrap(siginfo);
      break;
    case SIGSEGV:
      std::cout << "Yay, segfault. Reason: " << siginfo.si_code << std::endl;
      break;
    default:
      std::cout << "Got signal " << strsignal(siginfo.si_signo) << std::endl;
  }
}

void Stepper::handle_sigtrap(siginfo_t info) {
  switch (info.si_code) {
    // one of these will be set if a breakpoint was hit
    case SI_KERNEL:
    case TRAP_BRKPT: {
      auto pc = get_register_value(m_pid, Register::rip);
      set_register_value(m_pid, Register::rip, pc - 1);
      std::cout << "Hit breakpoint at address 0x" << std::hex
                << (pc - 1 - m_start_address) << "|0x" << (pc - 1) << std::endl;
      auto line_entry =
          m_debug_info.get_line_entry_from_pc(pc - m_start_address);
      m_debug_info.print_source(line_entry->file->path, line_entry->line);
      return;
    }
      // this will be set if the signal was sent by single stepping
    case TRAP_TRACE:
      return;
    default:
      std::cout << "Unknown SIGTRAP code " << info.si_code << std::endl;
      return;
  }
}

void Stepper::single_step_instruction() {
  ptrace(PTRACE_SINGLESTEP, m_pid, 0, 0);
  wait_for_signal();
}

void Stepper::step_over_breakpoint(
    std::unordered_map<std::intptr_t, Breakpoint>& breakpoints) {
  if (breakpoints.count(get_pc(m_pid))) {
    auto& bp = breakpoints[get_pc(m_pid)];
    if (bp.is_enabled()) {
      bp.disable();
      single_step_instruction();
      bp.enable();
    }
  }
}
void Stepper::single_step_instruction_with_breakpoint_check(
    std::unordered_map<std::intptr_t, Breakpoint>& breakpoints) {
  if (breakpoints.count(get_pc(m_pid))) {
    step_over_breakpoint(breakpoints);
  } else {
    single_step_instruction();
  }
}

