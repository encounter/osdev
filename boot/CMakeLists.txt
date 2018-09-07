cmake_minimum_required(VERSION 3.2)
project(boot NONE)

set(CMAKE_ASM_NASM_OBJECT_FORMAT bin)
set(CMAKE_ASM_NASM_LINK_EXECUTABLE "cp <OBJECTS> <TARGET>") # hacky hack
enable_language(ASM_NASM)

add_executable(boot.bin main.asm)