cmake_minimum_required(VERSION 3.2)
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_SYSTEM_NAME "Generic" CACHE STRING "Target system.")

if (CMAKE_HOST_APPLE AND CMAKE_C_LINK_FLAGS)
    string(REPLACE "-Wl,-search_paths_first" "" CMAKE_C_LINK_FLAGS ${CMAKE_C_LINK_FLAGS})
endif ()

set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/linker.ld")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -std=c11 -Wall -Werror -ffreestanding -fno-pie -nostartfiles -nostdlib -fno-stack-protector")
set(CMAKE_C_FLAGS_DEBUG "-ggdb")
set(CMAKE_C_FLAGS_RELEASE "-Os")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-n -Wl,--build-id=none -T ${LINKER_SCRIPT}")

add_subdirectory(kernel)
add_subdirectory(libc)

if (CMAKE_HOST_APPLE)
    set(CMAKE_C_COMPILER i386-elf-gcc)
endif ()

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf)
enable_language(ASM_NASM)

add_executable(kernel.bin dummy.c multiboot.asm)
target_link_libraries(kernel.bin kernel c)
set_target_properties(kernel.bin PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})