#ifndef COLORS_H
#define COLORS_H

#include <iostream>

class Colors {
  public:
  inline static const char* GREEN = "\033[;32m";
  inline static const char* BLUE = "\033[;34m";
  inline static const char* RED = "\033[;31m";
  inline static const char* RESET = "\033[0m";

  static void print_break(uint64_t addr, uint64_t full_addr) {
    std::cout << std::hex
        << "Breakpoint at: " << Colors::GREEN << "0x" << addr
        << "|0x" << full_addr << Colors::RESET << std::endl;
  }
};

#endif /* COLORS_H */
