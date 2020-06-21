#include <sstream>

#include "../libs/linenoise/linenoise.h"

#include "debugger.h"
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

  // step
  if (command == "s") {
    m_stepper.step_in(m_breakpoints);
    return;
  }

  // next
  if (command == "n") {
    m_stepper.step_over();
    return;
  }

  if (command == "stepout") {
    m_stepper.step_out(m_breakpoints);
    return;
  }

  // continue
  if (command == "c") {
    std::cout << PROMPT << "continuing execution of process: " << std::dec << m_pid << std::endl;
    m_stepper.continue_execution(m_breakpoints);
    return;
  }

  // break
  if (command == "b") {
    std::string addr_s{args[1], 2};
    addr = std::stol(addr_s, 0, 16);
    std::cout << PROMPT << "setting breakpoint at: 0x" << std::hex << addr << "|0x" << (addr + m_start_address) << std::endl;
    m_stepper.set_breakpoint(m_breakpoints, addr);
    return;
  }

  // if (is_prefix(args[1], "get")) {
  //   addr = get_register_value(m_pid, get_register_from_name(args[2]));
  //   std::cout << PROMPT << std::hex << "register " << args[2] << " : " << "0x" << addr - m_start_address << "|0x" << addr << std::endl;
  // } else if (is_prefix(args[1], "set")) {
  //   std::string val{args[3], 2};
  //   addr = std::stol(val, 0, 16) + m_start_address;
  //   std::cout << PROMPT << std::hex << "setting register " << args[2] << " : 0x" << args[3] << "|0x" << addr
  //       << std::endl;
  //   set_register_value(m_pid, get_register_from_name(args[2]), addr);
  // } else if (is_prefix(command, "function")) {
  //   std::string addr_s{args[1], 2};
  //   addr = std::stol(addr_s, 0, 16);
  //   auto die = m_debug_info.get_function_from_pc(addr);
  //   std::cout << die.resolve(dwarf::DW_AT::name).as_string() << std::endl;
  // }

  std::cerr << PROMPT << "unknown command\n";
}
