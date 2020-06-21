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

extern Register get_register_from_name(const std::string& name);
extern uint64_t get_register_value(pid_t pid, Register r);
extern void set_register_value(pid_t pid, Register r, uint64_t value);
extern uint64_t get_pc(pid_t pid);

extern uint64_t read_memory(pid_t pid, uint64_t address);

#endif
