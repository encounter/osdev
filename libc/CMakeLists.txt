cmake_minimum_required(VERSION 3.2)
project(libc)

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf)
enable_language(ASM_NASM)

set(CMAKE_C_FLAGS "-m32 -std=c11 -ffreestanding -fno-pie -fshort-enums -Wall -Werror -Wno-error=unused-variable -Wno-error=attributes")
# if clang: -Wno-error=unused-command-line-argument -nobuiltininc
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -fno-builtin -nostdlib -nodefaultlibs")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-stack-protector") # FIXME generates bad code using `mov eax, large gs:14h`?
set(CMAKE_C_FLAGS_DEBUG "-gdwarf-4 -DENABLE_DWARF")
if (NOT CMAKE_HOST_APPLE)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=undefined")
endif()
set(CMAKE_C_FLAGS_RELEASE "-Os")

if (CMAKE_HOST_APPLE)
  set(CMAKE_C_COMPILER i386-elf-gcc)
  set(CMAKE_AR i386-elf-ar)
  set(CMAKE_RANLIB i386-elf-ranlib)
endif ()

include_directories(${CMAKE_SOURCE_DIR}/libc)
add_library(c STATIC string math vector byteswap ctype errno wchar stdio.c stdio.h
        stddef.h stdarg.h
        stdio/printf stdio/fprintf stdio/sprintf stdio/snprintf stdio/vfprintf stdio/vsprintf
        stdio/vsnprintf)