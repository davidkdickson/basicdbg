#include <signal.h>

#include <unordered_map>

#include "breakpoint.h"
#include "debug_info.h"
#include "stepper.h"

class Debugger {
  public:
  Debugger(pid_t pid, uint64_t start_address, DebugInfo& debug_info, Stepper& stepper)
      :  m_pid{pid}, m_start_address{start_address}, m_debug_info(debug_info), m_stepper(stepper) { }

  void run();
  void print_backtrace();

  private:
  void handle_command(const std::string& line);

  pid_t m_pid;
  uint64_t m_start_address;
  std::unordered_map<std::intptr_t, Breakpoint> m_breakpoints;

  DebugInfo& m_debug_info;
  Stepper& m_stepper;


  inline static const char* PROMPT = "basicdbg> ";
};
