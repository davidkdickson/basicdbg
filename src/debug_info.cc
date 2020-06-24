#include "debug_info.h"

#include <iostream>
#include <fstream>

dwarf::die DebugInfo::get_function_from_pc(uint64_t pc) {
  for (auto &cu : m_dwarf.compilation_units()) {
    if (die_pc_range(cu.root()).contains(pc)) {
      std::vector<dwarf::die> stack;
      if (find_pc(cu.root(), pc, &stack)) {
        bool first = true;
        for (auto &d : stack) {
          if (!first) printf("\nInlined in:\n");
          first = false;
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

dwarf::line_table::entry DebugInfo::get_address_of_line(const std::string& file, unsigned line) {
  for (const auto& cu : m_dwarf.compilation_units()) {
    if (file == at_name(cu.root())) {
      const auto& lt = cu.get_line_table();

      for (const auto& entry : lt) {
        if (entry.is_stmt && entry.line == line) {
          return entry;
        }
      }
    }
  }
  throw std::out_of_range{"Cannot find line"};
}

