cmake_minimum_required (VERSION 3.0)
project (basicdbg)

add_compile_options(-std=c++14)

include_directories(ext/libelfin ext/linenoise)
add_executable(basicdbg src/basicdbg.cpp ext/linenoise/linenoise.c)
