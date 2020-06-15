#include "debug_info.h"

#include <iostream>
#include <fstream>

dwarf::die DebugInfo::get_function_from_pc(uint64_t pc) {
  for (auto &cu : m_dwarf.compilation_units()) {
    if (die_pc_range(cu.root()).contains(pc)) {
      // Map PC to a line
      auto &lt = cu.get_line_table();
      auto it = lt.find_address(pc);

      if (it == lt.end())
        std::cout << "unknown" << std::endl;
      else
        std::cout << it->get_description().c_str() << std::endl;

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

dwarf::line_table::iterator DebugInfo::get_line_entry_from_pc(uint64_t pc) {
  for (auto &cu : m_dwarf.compilation_units()) {
    if (die_pc_range(cu.root()).contains(pc)) {
      auto &lt = cu.get_line_table();
      auto it = lt.find_address(pc);
      if (it == lt.end())
        std::cout << "unknown" << std::endl;
      else {
        std::cout << it->get_description().c_str() << std::endl;
        return it;
      }
    }
  }
  throw std::out_of_range{"Cannot find line"};
}

bool DebugInfo::find_pc(const dwarf::die &d, dwarf::taddr pc, std::vector<dwarf::die> *stack) {
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

void DebugInfo::dump_die(const dwarf::die &node) {
  printf("<%" PRIx64 "> %s\n", node.get_section_offset(),
         to_string(node.tag).c_str());
  for (auto &attr : node.attributes())
    printf("      %s %s\n", to_string(attr.first).c_str(),
           to_string(attr.second).c_str());
}


void DebugInfo::print_source(const std::string &file_name, unsigned line, unsigned n_lines_context) {
  std::ifstream file{file_name};

  // Work out a window around the desired line
  auto start_line = line <= n_lines_context ? 1 : line - n_lines_context;
  auto end_line = line + n_lines_context + (line < n_lines_context ? n_lines_context - line : 0) + 1;

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

