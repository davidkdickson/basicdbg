#include "debugger.h"

#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../ext/libelfin/dwarf/dwarf++.hh"
#include "../ext/linenoise/linenoise.h"
#include "breakpoint.h"
#include "register.h"

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

  while ((line = linenoise(PROMPT)) != nullptr) {
    handle_command(line);
    linenoiseHistoryAdd(line);
    linenoiseFree(line);
  }

  return;
}

void Debugger::handle_command(const std::string &line) {
  auto args = split(line, ' ');
  auto command = args[0];
  uint64_t addr;

  if (is_prefix(command, "cont")) {
    std::cout << PROMPT << "continuing execution of process: " << std::dec
              << m_pid << std::endl;
    continue_execution();
  } else if (is_prefix(command, "register")) {
    if (is_prefix(args[1], "dump")) {
    } else if (is_prefix(args[1], "get")) {
      addr = get_register_value(m_pid, get_register_from_name(args[2]));
      std::cout << PROMPT << std::hex
          << "register " << args[2] << " : "
          << "0x" << addr - m_start_address
          << "|0x" << addr << std::endl;
    } else if (is_prefix(args[1], "set")) {
      std::string val{args[3], 2};
      addr = std::stol(val, 0, 16) + m_start_address;
      std::cout << PROMPT << std::hex
          << "setting register " << args[2] << " : 0x" << args[3]
          << "|0x" << addr << std::endl;
      set_register_value(m_pid, get_register_from_name(args[2]), addr);
    }
  } else if (is_prefix(command, "break")) {
    std::string addr{args[1], 2};
    uint64_t int_addr = std::stol(addr, 0, 16);
    std::cout << PROMPT << "setting breakpoint at: 0x" << addr << "|0x"
              << std::hex << (int_addr + m_start_address) << std::endl;
    set_breakpoint(int_addr);
  } else if (is_prefix(command, "backtrace")) {
    print_backtrace();
  } else {
    std::cerr << PROMPT << "unknown command\n";
  }
}

void Debugger::continue_execution() {
  step_over_breakpoint();
  ptrace(PTRACE_CONT, m_pid, nullptr, nullptr);

  int wait_status;
  auto options = 0;
  waitpid(m_pid, &wait_status, options);
}

void Debugger::set_breakpoint(std::intptr_t address) {
  uint64_t addr = address + m_start_address;

  Breakpoint bp(m_pid, addr);
  bp.enable();
  m_breakpoints[addr] = bp;
}

void Debugger::step_over_breakpoint() {
  struct user_regs_struct regs;
  ptrace(PTRACE_GETREGS, m_pid, nullptr, &regs);

  auto breakpoint_location = regs.rip - 1;

  if (!m_breakpoints.count(breakpoint_location)) {
    return;
  }

  auto &bp = m_breakpoints[breakpoint_location];

  if (bp.is_enabled()) {
    bp.disable();
    regs.rip = breakpoint_location;
    ptrace(PTRACE_SETREGS, m_pid, nullptr, &regs);
    ptrace(PTRACE_SINGLESTEP, m_pid, nullptr, nullptr);
    int wait_status;
    auto options = 0;
    waitpid(m_pid, &wait_status, options);
  }
}

void Debugger::print_backtrace() {
  auto output_frame = [frame_number = 0](auto &&func) mutable {
    std::cout << "frame #" << frame_number++ << ": 0x" << dwarf::at_low_pc(func)
              << ' ' << dwarf::at_name(func) << std::endl;
  };


  uint64_t pc = get_register_value(m_pid, Register::rip);
  auto current_func = get_function_from_pc(pc);

  uint64_t frame_pointer = get_register_value(m_pid, Register::rbp);
  output_frame(current_func);

  auto return_address = read_memory(frame_pointer + 8);

  while (dwarf::at_name(current_func) != "main") {
    current_func = get_function_from_pc(return_address);
    output_frame(current_func);
    frame_pointer = read_memory(frame_pointer);
    return_address = read_memory(frame_pointer + 8);
  }
}

uint64_t Debugger::read_memory(uint64_t address) {
  return ptrace(PTRACE_PEEKDATA, m_pid, address, nullptr);
}

dwarf::die Debugger::get_function_from_pc(uint64_t pc) {
  pc -= m_start_address;
  for (auto &cu : m_dwarf.compilation_units()) {
    if (die_pc_range(cu.root()).contains(pc)) {
      for (const auto &die : cu.root()) {
        if (die.tag == dwarf::DW_TAG::subprogram) {
          if (die_pc_range(die).contains(pc)) {
            return die;
          }
        }
      }
    }
  }

  throw std::out_of_range{"cannot find function"};
}

