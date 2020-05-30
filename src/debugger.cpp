#include <string>
#include <vector>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sstream>
#include <iostream>

#include "debugger.h"
#include "linenoise.h"

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
  int wait_status;
  auto options = 0;
  waitpid(m_pid, &wait_status, options);
  char *line = nullptr;

  while ((line = linenoise("basicdbg> ")) != nullptr) {
    handle_command(line);
    linenoiseHistoryAdd(line);
    linenoiseFree(line);
  }
  return;
}

void Debugger::handle_command(const std::string &line) {
  auto args = split(line, ' ');
  auto command = args[0];

  if (is_prefix(command, "continue")) {
    continue_execution();
  } else {
    std::cerr << "Unknown command\n";
  }
}

void Debugger::continue_execution() {
  ptrace(PTRACE_CONT, m_pid, nullptr, nullptr);

  int wait_status;
  auto options = 0;
  waitpid(m_pid, &wait_status, options);
}
