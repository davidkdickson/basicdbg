#include <string>
#include <utility>
#include <unordered_map>

#include <cstdint>
#include <signal.h>
#include <fcntl.h>

#include "../ext/libelfin/dwarf/dwarf++.hh"
#include "../ext/libelfin/elf/elf++.hh"

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
  uint64_t read_memory(uint64_t addr);
  dwarf::die get_function_from_pc(uint64_t pc);

  std::string m_prog_name;
  pid_t m_pid;
  uint64_t m_start_address;
  std::unordered_map<std::intptr_t, Breakpoint> m_breakpoints;
  elf::elf m_elf;
  dwarf::dwarf m_dwarf;

  inline static const char* PROMPT = "basicdbg> ";
};
