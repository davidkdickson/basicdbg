#include <unordered_map>

#include <signal.h>

#include "breakpoint.h"
#include "debug_info.h"

class Debugger {
  public:
  Debugger(pid_t pid, uint64_t start_address, DebugInfo& debug_info)
      :  m_pid{pid}, m_start_address{start_address}, m_debug_info(debug_info) {
      }

  void run();

  private:
  void handle_command(const std::string& line);
  void set_breakpoint(std::intptr_t addr);
  void continue_execution();
  void step_over_breakpoint();
  void wait_for_signal();
  siginfo_t get_signal_info();
  void handle_sigtrap(siginfo_t info);

  pid_t m_pid;
  uint64_t m_start_address;
  DebugInfo& m_debug_info;
  std::unordered_map<std::intptr_t, Breakpoint> m_breakpoints;

  inline static const char* PROMPT = "basicdbg> ";
};
