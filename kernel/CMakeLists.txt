cmake_minimum_required(VERSION 3.2)
project(kernel)

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf)
enable_language(ASM_NASM)

set(CMAKE_C_COMPILER i386-elf-gcc)
set(CMAKE_AR i386-elf-ar)
set(CMAKE_RANLIB i386-elf-ranlib)

include_directories(${CMAKE_SOURCE_DIR}/libc)
add_library(kernel STATIC
  console
  descriptor_tables
  gdt.asm
  main
  drivers/ports)