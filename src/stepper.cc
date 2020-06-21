#include "stepper.h"
#include "register.h"

#include <sys/ptrace.h>

#include <iostream>

void Stepper::continue_execution(std::unordered_map<std::intptr_t, Breakpoint>& breakpoints) {
  step_over_breakpoint(breakpoints);
  ptrace(PTRACE_CONT, m_pid, nullptr, nullptr);
  wait_for_signal();
}

void Stepper::step_in(std::unordered_map<std::intptr_t, Breakpoint>& breakpoints) {

  auto line = m_debug_info.get_line_entry_from_pc(get_pc(m_pid) - m_start_address)->line;

  while (m_debug_info.get_line_entry_from_pc(get_pc(m_pid) - m_start_address)->line == line) {
    single_step_instruction_with_breakpoint_check(breakpoints);
  }

  auto line_entry = m_debug_info.get_line_entry_from_pc(get_pc(m_pid) - m_start_address);
  m_debug_info.print_source(line_entry->file->path, line_entry->line);
}

void Stepper::step_over(std::unordered_map<std::intptr_t, Breakpoint>& breakpoints) {
  auto func = m_debug_info.get_function_from_pc(get_pc(m_pid) - m_start_address);
  auto func_entry = at_low_pc(func);
  auto func_end = at_high_pc(func);

  auto line = m_debug_info.get_line_entry_from_pc(func_entry);
  auto start_line = m_debug_info.get_line_entry_from_pc(get_pc(m_pid) - m_start_address);

  std::vector<std::intptr_t> to_delete{};

  while (line->address < func_end) {
    if (line->address != start_line->address && !breakpoints.count(line->address)) {
      set_breakpoint(breakpoints, line->address);
      to_delete.push_back(line->address + m_start_address);
    }
    ++line;
  }

  auto frame_pointer = get_register_value(m_pid, Register::rbp);
  auto return_address = read_memory(m_pid, frame_pointer+8);

  if (!breakpoints.count(return_address)) {
    set_breakpoint(breakpoints, return_address - m_start_address);
    to_delete.push_back(return_address);
  }

  continue_execution(breakpoints);

  for (auto addr : to_delete) {
    remove_breakpoint(breakpoints, addr);
  }
}

void Stepper::step_out(std::unordered_map<std::intptr_t, Breakpoint>& breakpoints) {
  auto frame_pointer = get_register_value(m_pid, Register::rbp);
  auto return_address = read_memory(m_pid, frame_pointer+8);

  bool should_remove_breakpoint = false;

  if (!breakpoints.count(return_address)) {
    set_breakpoint(breakpoints, return_address - m_start_address);
    should_remove_breakpoint = true;
  }

  continue_execution(breakpoints);

  if (should_remove_breakpoint) {
    remove_breakpoint(breakpoints, return_address);
  }
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
      auto line_entry = m_debug_info.get_line_entry_from_pc(pc - m_start_address);
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

void Stepper::step_over_breakpoint(std::unordered_map<std::intptr_t, Breakpoint>& breakpoints) {
  if (breakpoints.count(get_pc(m_pid))) {
    auto& bp = breakpoints[get_pc(m_pid)];
    if (bp.is_enabled()) {
      bp.disable();
      single_step_instruction();
      bp.enable();
    }
  }
}

void Stepper::single_step_instruction_with_breakpoint_check(std::unordered_map<std::intptr_t, Breakpoint>& breakpoints) {
  if (breakpoints.count(get_pc(m_pid))) {
    step_over_breakpoint(breakpoints);
  } else {
    single_step_instruction();
  }
}

void Stepper::remove_breakpoint(std::unordered_map<std::intptr_t, Breakpoint>& breakpoints, std::intptr_t addr) {
    if (breakpoints.at(addr).is_enabled()) {
        breakpoints.at(addr).disable();
    }
    breakpoints.erase(addr);
}

void Stepper::set_breakpoint(std::unordered_map<std::intptr_t, Breakpoint>& breakpoints, std::intptr_t addr) {
  uint64_t address = addr + m_start_address;

  Breakpoint bp(m_pid, address);
  bp.enable();
  breakpoints[address] = bp;
}

void Stepper::set_breakpoint_at_line(std::unordered_map<std::intptr_t, Breakpoint>& breakpoints, const std::string& file, unsigned line) {
    auto entry = m_debug_info.get_address_of_line(file, line);
    set_breakpoint(breakpoints, entry.address);
}
