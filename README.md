# Basicdbg

Built myself a very basic debugger in an effort to refresh my understanding of C++.
Code heavily based on the following [blog posts](https://blog.tartanllama.xyz/writing-a-linux-debugger-setup/).

## Getting started
### Ubuntu
```
$ git clone --recursive git@github.com:davidkdickson/basicdbg.git
$ sudo apt install build-essential      # installs gcc, built using version 9.3
$ cd basicdbg
$ make
$ ./basicdbg examples/hello
```

### Docker
```
$ docker build -t basickdbg .
$ docker run -it /bin/bash
$ ./basicdbg examples/hello
```

### Usage
```
basicdbg <program>
```
- `s` step in
- `n` next / step over
- `f` finish / step out
- `b address` break at hex address, e.g. `b 0x11b5`
- `b file:line_number` break at line number, e.g. `b examples/hello.cc:9`
- `c` continue
- `q` quit

## Useful articles
- http://www.alexonlinux.com/how-debugger-works
- https://www.linuxjournal.com/article/6100
- https://www.linuxjournal.com/article/6210
- https://jvns.ca/blog/2018/01/09/resolving-symbol-addresses/
- https://amir.rachum.com/blog/2016/09/17/shared-libraries/

### Useful commands
```
objdump -f examples/hello       # outputs start address
objdump -d examples/hello       # assembler sections of executable
nm examples/hello               # another way at identifying address of main function
```
