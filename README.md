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
- step over

- better handling of start address
- automatically start and main
