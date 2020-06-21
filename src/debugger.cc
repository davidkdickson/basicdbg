#include "debugger.h"

#include <sys/wait.h>

#include <sstream>

#include "../libs/linenoise/linenoise.h"
#include "register.h"
#include "stepper.h"

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
   m_stepper.wait_for_signal();

  char *line = nullptr;

  while ((line = linenoise(PROMPT)) != nullptr) {
    auto pc = get_pc(m_pid);
    std::cout << std::hex << "0x" << (pc - m_start_address) << "|0x" <<  pc << std::endl;
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


  if (command == "stepin") {
    m_stepper.step_in(m_breakpoints);
    return;
  }

  if (command == "stepover") {
    m_stepper.step_over();
    return;
  }

  if (command == "stepout") {
    m_stepper.step_out(m_breakpoints);
    return;
  }

  if (is_prefix(command, "stepi")) {
    std::cout << "single step" << std::endl;
    m_stepper.single_step_instruction_with_breakpoint_check(m_breakpoints);
    auto line_entry = m_debug_info.get_line_entry_from_pc(get_pc(m_pid) - m_start_address);
    m_debug_info.print_source(line_entry->file->path, line_entry->line);
  } else if (is_prefix(command, "cont")) {
    std::cout << PROMPT << "continuing execution of process: " << std::dec << m_pid << std::endl;
    continue_execution();
  } else if (is_prefix(command, "register")) {
    if (is_prefix(args[1], "get")) {
      addr = get_register_value(m_pid, get_register_from_name(args[2]));
      std::cout << PROMPT << std::hex << "register " << args[2] << " : " << "0x" << addr - m_start_address
          << "|0x" << addr << std::endl;
    } else if (is_prefix(args[1], "set")) {
      std::string val{args[3], 2};
      addr = std::stol(val, 0, 16) + m_start_address;
      std::cout << PROMPT << std::hex << "setting register " << args[2] << " : 0x" << args[3] << "|0x" << addr
          << std::endl;
      set_register_value(m_pid, get_register_from_name(args[2]), addr);
    }
  } else if (is_prefix(command, "break")) {
    std::string addr_s{args[1], 2};
    addr = std::stol(addr_s, 0, 16);
    std::cout << PROMPT << "setting breakpoint at: 0x" << std::hex << addr << "|0x" << (addr + m_start_address)
        << std::endl;
    m_stepper.set_breakpoint(m_breakpoints, addr);
  } else if (is_prefix(command, "function")) {
    std::string addr_s{args[1], 2};
    addr = std::stol(addr_s, 0, 16);
    auto die = m_debug_info.get_function_from_pc(addr);
    std::cout << die.resolve(dwarf::DW_AT::name).as_string() << std::endl;
  } else if (is_prefix(command, "line")) {
    std::string addr_s{args[1], 2};
    addr = std::stol(addr_s, 0, 16);
    auto line_entry = m_debug_info.get_line_entry_from_pc(addr);
    m_debug_info.print_source(line_entry->file->path, line_entry->line);
  } else {
    std::cerr << PROMPT << "unknown command\n";
  }
}

void Debugger::continue_execution() {
  m_stepper.step_over_breakpoint(m_breakpoints);
  ptrace(PTRACE_CONT, m_pid, nullptr, nullptr);
  m_stepper.wait_for_signal();
}
