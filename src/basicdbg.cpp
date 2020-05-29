#include <iostream>
#include <unistd.h>
#include <string>
#include <sys/ptrace.h>
#include <vector>
#include <sys/wait.h>
#include <sstream>

#include "linenoise.h"

class Debugger {
public:
  Debugger (std::string prog_name, pid_t pid)
    : m_prog_name{std::move(prog_name)}, m_pid{pid} {}

  void run() {
    int wait_status;
    auto options = 0;
    waitpid(m_pid, &wait_status, options);
    char *line = nullptr;

    while((line = linenoise("basicdbg> ")) != nullptr) {
      handle_command(line);
      linenoiseHistoryAdd(line);
      linenoiseFree(line);
    }
    return;
  }

  void handle_command(const std::string &line) {
    auto args = split(line, ' ');
    auto command = args[0];

    if (is_prefix(command, "continue")) {
      continue_execution();
    }
    else {
      std::cerr << "Unknown command\n";
    }
  }

  void continue_execution() {
    ptrace(PTRACE_CONT, m_pid, nullptr, nullptr);

    int wait_status;
    auto options = 0;
    waitpid(m_pid, &wait_status, options);
  }

private:
  std::string m_prog_name;
  pid_t m_pid;

  std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> out{};
    std::stringstream ss {s};
    std::string item;

    while (std::getline(ss,item,delimiter)) {
      out.push_back(item);
    }

    return out;
  }

  bool is_prefix(const std::string& s, const std::string& of) {
    if (s.size() > of.size()) return false;
    return std::equal(s.begin(), s.end(), of.begin());
  }
};

int main(int argc, char *argv[])
{
  if (argc < 2) {
    std::cerr << "Program not specified";
    return -1;
  }

  auto prog = argv[1];
  auto pid = fork();

  if (pid == 0) {
    ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
    execl(prog, prog, nullptr);
  } else if (pid >= 1) {
    Debugger dbg {prog, pid};
    dbg.run();
  }

  return 0;
}
