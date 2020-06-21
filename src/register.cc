#include "register.h"

Register get_register_from_name(const std::string& name) {

  if (name == "rip") {
    return Register::rip;
  }
  if (name == "rbp") {
    return Register::rbp;
  }

  return Register::rip;
}

uint64_t get_register_value(pid_t pid, Register r) {
  user_regs_struct regs;
  ptrace(PTRACE_GETREGS, pid, nullptr, &regs);

  switch (r) {
    case Register::rip:
      return regs.rip;
    case Register::rbp:
      return regs.rbp;
    default:
      return 0;

  }
}

void set_register_value(pid_t pid, Register r, uint64_t value) {
  user_regs_struct regs;
  ptrace(PTRACE_GETREGS, pid, nullptr, &regs);

  switch (r) {
    case Register::rip:
      regs.rip = value;
      break;
    case Register::rbp:
      regs.rbp = value;
      break;
    default:
      break;
  }

  ptrace(PTRACE_SETREGS, pid, nullptr, &regs);
}

uint64_t read_memory(pid_t pid, uint64_t address) {
    return ptrace(PTRACE_PEEKDATA, pid, address, nullptr);
}

uint64_t get_pc(pid_t pid) {
  return get_register_value(pid, Register::rip);
}

