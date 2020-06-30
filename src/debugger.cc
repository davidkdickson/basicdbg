#include <sstream>
#include <iostream>

#include "debugger.h"
#include "stepper.h"
#include "colors.h"


std::vector<std::string> split(const std::string &s, char delimiter) {
  std::vector<std::string> out{};
  std::stringstream ss{s};
  std::string item;

  while (std::getline(ss, item, delimiter)) {
    out.push_back(item);
  }

  return out;
}

void Debugger::run() {
   m_stepper.wait_for_signal();

  std::string line;

  auto main_address = m_debug_info.get_address_of_function("main");
  m_stepper.set_breakpoint(m_breakpoints, main_address);
  m_stepper.continue_execution(m_breakpoints);

  std::cout << Debugger::PROMPT;
  while (std::getline(std::cin, line))
  {
    if(line == "q") {
      return;
    }
    handle_command(line);
    std::cout << Debugger::PROMPT;
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
    std::cout << "Continuing execution: " << Colors::GREEN <<
        std::dec << m_pid << Colors::RESET << std::endl;
    m_stepper.continue_execution(m_breakpoints);
    return;
  }

  // break at memory address or source line
  if (command == "b") {
    if (args[1][0] == '0' && args[1][1] == 'x') {
      std::string addr_s{args[1], 2};
      addr = std::stol(addr_s, 0, 16);
      std::cout << "Breakpoint at: ";
      Colors::print_location(addr, addr + m_start_address);
      m_stepper.set_breakpoint(m_breakpoints, addr);
    } else if (args[1].find(':') != std::string::npos) {
      auto file_and_line = split(args[1], ':');
      std::cout << "Breakpoint at: " << Colors::GREEN << args[1] << Colors::RESET << std::endl;
      m_stepper.set_breakpoint_at_line(m_breakpoints, file_and_line[0], std::stoi(file_and_line[1]));
    }
    return;
  }

  // print backtrace
  if (command == "bt") {
    print_backtrace();
    return;
  }

  std::cerr << PROMPT << Colors::RED << "unknown command\n" << Colors::RESET;
}

void Debugger::print_backtrace() {
  auto output_frame = [frame_number = 0] (auto&& func) mutable {
        std::cout << "frame #" << frame_number++ << ": "
            << Colors::GREEN << "0x" << dwarf::at_low_pc(func) << ' '
            << Colors::BLUE << dwarf::at_name(func) << Colors::RESET << std::endl;
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
