#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iomanip>

#include <iostream>

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

void runprocess(pid_t pid, user_regs_struct &regs)
{
  int status;
  waitpid(pid, &status, 0);
  int i = 0;

  while( WIFSTOPPED(status) ) {
    if(i++ > 10) {
      std::cin >> status;
      return;
    }

    ptrace(PTRACE_GETREGS, pid, nullptr, &regs);
    fprintf(stderr, "EIP: %p\n", (void*)regs.rip);
    status = singlestep(pid);
  }



}


int main() {
  pid_t traced_process = fork();

  if (traced_process == 0) {

    ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
    execl("./hello", "hello", nullptr);

  } else {

    long break_point;
    long ins;
    struct user_regs_struct regs;
    int status;


    //runprocess(traced_process, regs);

    waitpid(traced_process, &status, 0);
    ptrace(PTRACE_GETREGS, traced_process, nullptr, &regs);
    fprintf(stderr, "EIP: %p\n", (void*)regs.rip);
    std::cin >> std::hex >> break_point;

    ins = ptrace(PTRACE_PEEKTEXT, traced_process, break_point, nullptr);

    std::cout << ins << std::endl;
    //ins = ptrace(PTRACE_PEEKTEXT, traced_process, regs.rip, nullptr);
    //std::cout << ins << std::endl;
    //std::cout << regs.rip << std::endl;



  }
  return 0;
}

