#include <sstream>

#include "../libs/linenoise/linenoise.h"

#include "debugger.h"
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
    if(strcmp(line, "quit") == 0) {
      linenoiseFree(line);
      return;
    }

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
    m_stepper.step_over(m_breakpoints);
    return;
  }

  // finish / step out
  if (command == "f") {
    m_stepper.step_out(m_breakpoints);
    return;
  }

  // continue
  if (command == "c") {
    std::cout << Debugger::GREEN
        << "Continuing executions: " << std::dec << m_pid
        << Debugger::RESET << std::endl;
    m_stepper.continue_execution(m_breakpoints);
    return;
  }

  // break at memory address or source line
  if (command == "b") {
    if (args[1][0] == '0' && args[1][1] == 'x') {
      std::string addr_s{args[1], 2};
      addr = std::stol(addr_s, 0, 16);
      std::cout << std::hex << "Breakpoint at: " << Debugger::GREEN <<  "0x" << addr << "|0x" << (addr + m_start_address)
          << Debugger::RESET << std::endl;
      m_stepper.set_breakpoint(m_breakpoints, addr);
    } else if (args[1].find(':') != std::string::npos) {
      auto file_and_line = split(args[1], ':');
      m_stepper.set_breakpoint_at_line(m_breakpoints, file_and_line[0], std::stoi(file_and_line[1]));
    }
    return;
  }

  // pritn backtrace
  if (command == "bt") {
    print_backtrace();
    return;
  }

  std::cerr << PROMPT << "unknown command\n";
}

void Debugger::print_backtrace() {
  auto output_frame = [frame_number = 0] (auto&& func) mutable {
        std::cout << "frame #" << frame_number++ << ": 0x" << dwarf::at_low_pc(func)
                  << ' ' << dwarf::at_name(func) << std::endl;
    };
  auto current_func = m_debug_info.get_function_from_pc(get_pc(m_pid) - m_start_address);
  output_frame(current_func);

  auto frame_pointer = get_register_value(m_pid, Register::rbp);
  auto return_address = read_memory(m_pid, frame_pointer+8);

  while (dwarf::at_name(current_func) != "main") {
    current_func = m_debug_info.get_function_from_pc(return_address - m_start_address);
    output_frame(current_func);
    frame_pointer = read_memory(m_pid, frame_pointer);
    return_address = read_memory(m_pid, frame_pointer+8);
  }
}
