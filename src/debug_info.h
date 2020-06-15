#ifndef DEBUG_INFO_H
#define DEBUG_INFO_H

#include <fcntl.h>
#include <inttypes.h>

#include <string>

#include "../libs/libelfin/dwarf/dwarf++.hh"
#include "../libs/libelfin/elf/elf++.hh"

class DebugInfo {
  private:
  elf::elf m_elf;
  dwarf::dwarf m_dwarf;

  bool find_pc(const dwarf::die &d, dwarf::taddr pc, std::vector<dwarf::die> *stack);
  void dump_die(const dwarf::die &node);

  public:
  DebugInfo (std::string prog_name) {
    auto fd = open(prog_name.c_str(), O_RDONLY);
    m_elf = elf::elf{elf::create_mmap_loader(fd)};
    m_dwarf = dwarf::dwarf{dwarf::elf::create_loader(m_elf)};
  }

  dwarf::die get_function_from_pc(uint64_t pc);
  dwarf::line_table::iterator get_line_entry_from_pc(uint64_t pc);
  void print_source(const std::string& file_name, unsigned line, unsigned n_lines_context = 2);
};
#endif /* DEBUG_INFO_H */
