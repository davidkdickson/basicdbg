#include <string>
#include <utility>
#include <unordered_map>

#include <cstdint>
#include <signal.h>
#include <fcntl.h>


#include "../ext/libelfin/dwarf/dwarf++.hh"
#include "../ext/libelfin/elf/elf++.hh"

#include <signal.h>

#include "breakpoint.h"

class Debugger {
  public:
  Debugger(std::string prog_name, pid_t pid, uint64_t start_address)
      : m_prog_name{std::move(prog_name)}, m_pid{pid}, m_start_address{start_address} {
        auto fd = open(m_prog_name.c_str(), O_RDONLY);

        m_elf = elf::elf{elf::create_mmap_loader(fd)};
        m_dwarf = dwarf::dwarf{dwarf::elf::create_loader(m_elf)};
      }

  void run();

  private:
  void handle_command(const std::string& line);
  void set_breakpoint(std::intptr_t addr);
  void continue_execution();
  void step_over_breakpoint();
  void print_backtrace();
  void wait_for_signal();
  uint64_t read_memory(uint64_t addr);
  dwarf::die get_function_from_pc(uint64_t pc);
  dwarf::line_table::iterator get_line_entry_from_pc(uint64_t pc);
  bool find_pc(const dwarf::die &d, dwarf::taddr pc, std::vector<dwarf::die> *stack);
  void dump_die(const dwarf::die &node);
  void print_source(const std::string& file_name, unsigned line, unsigned n_lines_context = 2);
  siginfo_t get_signal_info();
  void handle_sigtrap(siginfo_t info);

  std::string m_prog_name;
  pid_t m_pid;
  uint64_t m_start_address;
  std::unordered_map<std::intptr_t, Breakpoint> m_breakpoints;
  elf::elf m_elf;
  dwarf::dwarf m_dwarf;

  inline static const char* PROMPT = "basicdbg> ";
};
