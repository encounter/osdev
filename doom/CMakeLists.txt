cmake_minimum_required(VERSION 3.2)
project(libc)

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf)
enable_language(ASM_NASM)

if (CMAKE_HOST_APPLE)
    set(CMAKE_C_COMPILER i386-elf-gcc)
    set(CMAKE_AR i386-elf-ar)
    set(CMAKE_RANLIB i386-elf-ranlib)
endif ()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error")

include_directories(${CMAKE_SOURCE_DIR}/libc)
FILE(GLOB SOURCES *.c)
FILE(GLOB HEADERS *.h)
add_library(doom STATIC ${SOURCES} ${HEADERS})