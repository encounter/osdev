cmake_minimum_required(VERSION 3.2)
project(kernel)

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf)
enable_language(ASM_NASM)

if (CMAKE_HOST_APPLE)
  set(CMAKE_C_COMPILER i386-elf-gcc)
  set(CMAKE_AR i386-elf-ar)
  set(CMAKE_RANLIB i386-elf-ranlib)
endif()

set(CMAKE_C_FLAGS "-m32 -std=c11 -ffreestanding -fno-pie -fshort-enums -Wall -Werror -Wno-error=unused-variable -Wno-error=attributes")
# if clang: -Wno-error=unused-command-line-argument -nobuiltininc
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -fno-builtin -nostdlib -nodefaultlibs")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-stack-protector") # FIXME generates bad code using `mov eax, large gs:14h`?
set(CMAKE_C_FLAGS_DEBUG "-gdwarf-4 -DENABLE_DWARF")
if (NOT CMAKE_HOST_APPLE)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=undefined")
endif()
set(CMAKE_C_FLAGS_RELEASE "-Os")

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
  elf
  stdio_impl
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
#  drivers/i825xx
  fatfs/diskio
  fatfs/ff
  fatfs/ffconf.h
  fatfs/ffsystem
  fatfs/ffunicode
  fatfs/integer.h
  tests/tests.h
  tests/vector
  tests/fatfs
  tests/stdio
  tests/path)
if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
  set(SOURCE_FILES ${SOURCE_FILES} debug/ubsan debug/dwarf)
endif ()
add_library(kernel STATIC ${SOURCE_FILES})