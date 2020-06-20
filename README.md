# basicdbg

## Resources used
- http://www.alexonlinux.com/how-debugger-works
- https://blog.tartanllama.xyz/writing-a-linux-debugger-breakpoints/
- http://www.alexonlinux.com/wp-content/uploads/2008/03/listing2.c
- https://www.linuxjournal.com/article/6100
- https://www.linuxjournal.com/article/6100
- https://www.linuxjournal.com/article/6210
- https://jvns.ca/blog/2018/01/09/resolving-symbol-addresses/
- https://amir.rachum.com/blog/2016/09/17/shared-libraries/

## Useful commands
```
objdump -f # overall file header includes start address
objdump -h # section headers
objdump -d # assembler sections of executable sections
nm
ldd basicdbg
```

## MVP Functionality
- break point at line
- step over
- step out
- print back trace

- better handling of start address
- automatically start and main

//  int wait_status;
//  unsigned icounter = 0;
//  std::cout << "single step"  << std::endl;
//  waitpid(m_pid, &wait_status, 0);
//
//  while (WIFSTOPPED(wait_status)) {
//    auto addr = get_register_value(m_pid, Register::rip);
//    if((addr - m_start_address) == 0x1189)
//      std::cout << "start of program" << std::endl;
//    //std::cout << std::hex << "0x" << addr - m_start_address << "|0x" << addr << std::endl;
//    icounter++;
//    if (ptrace(PTRACE_SINGLESTEP, m_pid, 0, 0) < 0) {
//      std::cout << "step error"  << std::endl;
//      return;
//    }
//    waitpid(m_pid, &wait_status, 0);
//  }
//
//  std::cout << icounter << std::endl;
//  return;

