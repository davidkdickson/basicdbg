#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <iostream>
#include <iomanip>

int singlestep(int pid)
{
    int retval, status;
    retval = ptrace(PTRACE_SINGLESTEP, pid, 0, 0);
    if( retval ) {
        return retval;
    }
    waitpid(pid, &status, 0);
    return status;
}

int main() {
  pid_t traced_process = fork();

  if (traced_process == 0) {

    ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
    execl("./hello", "hello", nullptr);

  } else {

    long ins;
    struct user_regs_struct regs;
    int status;

    waitpid(traced_process, &status, 0);
    ptrace(PTRACE_GETREGS, traced_process, nullptr, &regs);
    ins = ptrace(PTRACE_PEEKTEXT, traced_process, regs.rip, nullptr);
    std::cout << std::hex << regs.rip << std::endl;
    printf("EIP: %llx Instruction executed: %lx\n", regs.rip, ins);

    while( WIFSTOPPED(status) ) {
      ptrace(PTRACE_GETREGS, traced_process, nullptr, &regs);
      fprintf(stderr, "EIP: %p\n", (void*)regs.rip);
      status = singlestep(traced_process);
    }


  }
  return 0;
}

