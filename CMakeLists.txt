cmake_minimum_required(VERSION 3.2)
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_SYSTEM_NAME "Generic" CACHE STRING "Target system.")

if (CMAKE_HOST_APPLE AND CMAKE_C_LINK_FLAGS)
    string(REPLACE "-Wl,-search_paths_first" "" CMAKE_C_LINK_FLAGS ${CMAKE_C_LINK_FLAGS})
endif ()

# set(CMAKE_C_COMPILER clang)
if (CMAKE_HOST_APPLE)
    set(CMAKE_C_COMPILER i386-elf-gcc)
endif ()

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf)
enable_language(ASM_NASM)

set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/linker.ld")
set(CMAKE_C_FLAGS "-m32 -ffreestanding -fno-pie")
# if clang: -Wno-error=unused-command-line-argument -nobuiltininc
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -fno-builtin -nostdlib -nodefaultlibs")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-n -Wl,--build-id=none -Wl,-T${LINKER_SCRIPT}")

add_executable(kernel.bin dummy.c multiboot.asm)
target_link_libraries(kernel.bin kernel c)
set_target_properties(kernel.bin PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

add_subdirectory(kernel)
add_subdirectory(libc)
add_subdirectory(bin)