#ifndef REGISTERS_H
#define REGISTERS_H

#include <string>
#include <sys/user.h>
#include <sys/ptrace.h>

enum class Register {
        rax, rbx, rcx, rdx,
        rdi, rsi, rbp, rsp,
        r8,  r9,  r10, r11,
        r12, r13, r14, r15,
        rip, rflags, cs,
        orig_rax, fs_base,
        gs_base,
        fs, gs, ss, ds, es
    };

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

#endif
