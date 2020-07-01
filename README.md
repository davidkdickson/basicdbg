# Basicdbg

Built myself a very basic debugger in an effort to refresh my understanding of C++.
Code heavily based on the following [blog posts](https://blog.tartanllama.xyz/writing-a-linux-debugger-setup/).

## Quick start
### Ubuntu
```
$ git clone --recursive git@github.com:davidkdickson/basicdbg.git
$ sudo apt install build-essential      # installs gcc
$ cd basicdbg
$ make
$ ./basicdbg examples/hello
```

## Quick start
### Docker
```
$ docker build -t basickdbg .
$ docker run -it /bin/bash
$ ./basicdbg examples/hello
```

## Usage
```
basicdbg <program>
```
- `s` step in
- `n` next / step over
- `f` finish / step out
- `b address` break at address
- `b file:line_number` break at line number
- `c` continue
- `q` quit

## Resources used
- http://www.alexonlinux.com/how-debugger-works
- https://www.linuxjournal.com/article/6100
- https://www.linuxjournal.com/article/6210
- https://jvns.ca/blog/2018/01/09/resolving-symbol-addresses/
- https://amir.rachum.com/blog/2016/09/17/shared-libraries/

### Commands
```
objdump -f # overall file header includes start address
objdump -h # section headers
objdump -d # assembler sections of executable sections
nm
ldd basicdbg
```

## TODO
- better handling of start address
