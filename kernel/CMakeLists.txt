cmake_minimum_required(VERSION 3.2)
project(kernel)

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf)
enable_language(ASM_NASM)

if (CMAKE_HOST_APPLE)
  set(CMAKE_C_COMPILER i386-elf-gcc)
  set(CMAKE_AR i386-elf-ar)
  set(CMAKE_RANLIB i386-elf-ranlib)
endif()

include_directories(${CMAKE_SOURCE_DIR}/libc)
set(SOURCE_FILES
  console
  descriptor_tables
  gdt.asm
  interrupt.asm
  isr
  main
  shell
  multiboot
  div64
  drivers/ports
  drivers/timer
  drivers/keyboard
  drivers/acpi
  drivers/serial
  drivers/vga
  drivers/ahci
  drivers/ata
  drivers/pci
  drivers/pci_registry
  fatfs/diskio
  fatfs/ff
  fatfs/ffconf.h
  fatfs/ffsystem
  fatfs/ffunicode
  fatfs/integer.h
  tests/tests.h
  tests/vector
  tests/fatfs)
if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
  set(SOURCE_FILES ${SOURCE_FILES} debug/ubsan)
endif ()
add_library(kernel STATIC ${SOURCE_FILES})