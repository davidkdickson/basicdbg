#include "debugger.h"

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../libs/libelfin/dwarf/dwarf++.hh"
#include "../libs/linenoise/linenoise.h"
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
  wait_for_signal();

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
      std::cout << PROMPT << std::hex << "register " << args[2] << " : "
                << "0x" << addr - m_start_address << "|0x" << addr << std::endl;
    } else if (is_prefix(args[1], "set")) {
      std::string val{args[3], 2};
      addr = std::stol(val, 0, 16) + m_start_address;
      std::cout << PROMPT << std::hex << "setting register " << args[2]
                << " : 0x" << args[3] << "|0x" << addr << std::endl;
      set_register_value(m_pid, get_register_from_name(args[2]), addr);
    }
  } else if (is_prefix(command, "break")) {
    std::string addr_s{args[1], 2};
    addr = std::stol(addr_s, 0, 16);
    std::cout << PROMPT << "setting breakpoint at: 0x" << std::hex << addr
              << "|0x" << (addr + m_start_address) << std::endl;
    set_breakpoint(addr);
  } else if (is_prefix(command, "function")) {
    std::string addr_s{args[1], 2};
    addr = std::stol(addr_s, 0, 16);
    get_function_from_pc(addr);
  } else if (is_prefix(command, "line")) {
    std::string addr_s{args[1], 2};
    addr = std::stol(addr_s, 0, 16);
    auto line_entry = get_line_entry_from_pc(addr);
    print_source(line_entry->file->path, line_entry->line);
  } else {
    std::cerr << PROMPT << "unknown command\n";
  }
}

void Debugger::continue_execution() {
  step_over_breakpoint();
  ptrace(PTRACE_CONT, m_pid, nullptr, nullptr);
  wait_for_signal();
}

void Debugger::set_breakpoint(std::intptr_t address) {
  uint64_t addr = address + m_start_address;

  Breakpoint bp(m_pid, addr);
  bp.enable();
  m_breakpoints[addr] = bp;
}

void Debugger::step_over_breakpoint() {
  auto breakpoint_location = get_register_value(m_pid, Register::rip);

  if (!m_breakpoints.count(breakpoint_location)) {
    return;
  }

  auto &bp = m_breakpoints[breakpoint_location];

  if (bp.is_enabled()) {
    bp.disable();
    set_register_value(m_pid, Register::rip, breakpoint_location);
    ptrace(PTRACE_SINGLESTEP, m_pid, nullptr, nullptr);
    wait_for_signal();
  }
}

void Debugger::wait_for_signal() {
  int wait_status;
  auto options = 0;
  waitpid(m_pid, &wait_status, options);

  auto siginfo = get_signal_info();

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

uint64_t Debugger::read_memory(uint64_t address) {
  return ptrace(PTRACE_PEEKDATA, m_pid, address, nullptr);
}

dwarf::die Debugger::get_function_from_pc(uint64_t pc) {
  for (auto &cu : m_dwarf.compilation_units()) {
    if (die_pc_range(cu.root()).contains(pc)) {
      // Map PC to a line
      auto &lt = cu.get_line_table();
      auto it = lt.find_address(pc);
      if (it == lt.end())
        printf("UNKNOWN\n");
      else
        printf("%s\n", it->get_description().c_str());

      // Map PC to an object
      // XXX Index/helper/something for looking up PCs
      // XXX DW_AT_specification and DW_AT_abstract_origin
      std::vector<dwarf::die> stack;
      if (find_pc(cu.root(), pc, &stack)) {
        bool first = true;
        for (auto &d : stack) {
          if (!first) printf("\nInlined in:\n");
          first = false;
          dump_die(d);
          return d;
        }
      }
      break;
    }
  }
  throw std::out_of_range{"Cannot find function"};
}

dwarf::line_table::iterator Debugger::get_line_entry_from_pc(uint64_t pc) {
  for (auto &cu : m_dwarf.compilation_units()) {
    if (die_pc_range(cu.root()).contains(pc)) {
      auto &lt = cu.get_line_table();
      auto it = lt.find_address(pc);
      if (it == lt.end())
        printf("UNKNOWN\n");
      else {
        printf("%s\n", it->get_description().c_str());
        return it;
      }
    }
  }
  throw std::out_of_range{"Cannot find line"};
}

void Debugger::dump_die(const dwarf::die &node) {
  printf("<%" PRIx64 "> %s\n", node.get_section_offset(),
         to_string(node.tag).c_str());
  for (auto &attr : node.attributes())
    printf("      %s %s\n", to_string(attr.first).c_str(),
           to_string(attr.second).c_str());
}

bool Debugger::find_pc(const dwarf::die &d, dwarf::taddr pc,
                       std::vector<dwarf::die> *stack) {
  using namespace dwarf;

  // Scan children first to find most specific DIE
  bool found = false;
  for (auto &child : d) {
    if ((found = find_pc(child, pc, stack))) break;
  }
  switch (d.tag) {
    case DW_TAG::subprogram:
    case DW_TAG::inlined_subroutine:
      try {
        if (found || die_pc_range(d).contains(pc)) {
          found = true;
          stack->push_back(d);
        }
      } catch (std::out_of_range &e) {
      } catch (value_type_mismatch &e) {
      }
      break;
    default:
      break;
  }
  return found;
}

void Debugger::print_source(const std::string &file_name, unsigned line,
                            unsigned n_lines_context) {
  std::ifstream file{file_name};

  // Work out a window around the desired line
  auto start_line = line <= n_lines_context ? 1 : line - n_lines_context;
  auto end_line = line + n_lines_context +
                  (line < n_lines_context ? n_lines_context - line : 0) + 1;

  char c{};
  auto current_line = 1u;
  // Skip lines up until start_line
  while (current_line != start_line && file.get(c)) {
    if (c == '\n') {
      ++current_line;
    }
  }

  // Output cursor if we're at the current line
  std::cout << (current_line == line ? "> " : "  ");

  // Write lines up until end_line
  while (current_line <= end_line && file.get(c)) {
    std::cout << c;
    if (c == '\n') {
      ++current_line;
      // Output cursor if we're at the current line
      std::cout << (current_line == line ? "> " : "  ");
    }
  }

  // Write newline and make sure that the stream is flushed properly
  std::cout << std::endl;
}

siginfo_t Debugger::get_signal_info() {
  siginfo_t info;
  ptrace(PTRACE_GETSIGINFO, m_pid, nullptr, &info);
  return info;
}

void Debugger::handle_sigtrap(siginfo_t info) {
  switch (info.si_code) {
    // one of these will be set if a breakpoint was hit
    case SI_KERNEL:
    case TRAP_BRKPT: {
      auto pc = get_register_value(m_pid, Register::rip);
      set_register_value(m_pid, Register::rip, pc - 1);
      std::cout << "Hit breakpoint at address 0x" << std::hex
                << (pc - 1 - m_start_address) << "|0x" << (pc - 1) << std::endl;
      auto line_entry = get_line_entry_from_pc(pc - m_start_address);
      print_source(line_entry->file->path, line_entry->line);
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

