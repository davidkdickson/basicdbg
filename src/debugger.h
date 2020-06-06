#include <string>
#include <utility>
#include <unordered_map>
#include "breakpoint.h"

class Debugger {
  public:
  Debugger(std::string prog_name, pid_t pid)
      : m_prog_name{std::move(prog_name)}, m_pid{pid} {}

  void run();
  void set_breakpoint(std::intptr_t addr);

  private:
  void handle_command(const std::string& line);
  void continue_execution();
  void step_over_breakpoint();

  std::string m_prog_name;
  pid_t m_pid;
  std::unordered_map<std::intptr_t, Breakpoint> m_breakpoints;
};
