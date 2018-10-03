cmake_minimum_required(VERSION 3.2)
project(bin)

if (CMAKE_HOST_APPLE)
    set(CMAKE_C_COMPILER i386-elf-gcc)
    set(CMAKE_AR i386-elf-ar)
    set(CMAKE_RANLIB i386-elf-ranlib)
endif ()

set(CMAKE_C_FLAGS "-m32 -std=c11 -fPIC -ffreestanding -fshort-enums -Wall -Werror")
# if clang: -Wno-error=unused-command-line-argument -nobuiltininc
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -fno-builtin -nostdlib -nodefaultlibs")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-stack-protector") # FIXME generates bad code using `mov eax, large gs:14h`?
set(CMAKE_C_FLAGS_DEBUG "-gdwarf-4 -DENABLE_DWARF")
if (NOT CMAKE_HOST_APPLE)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=undefined")
endif()
set(CMAKE_C_FLAGS_RELEASE "-Os")
set(CMAKE_EXE_LINKER_FLAGS "")

include_directories(${CMAKE_SOURCE_DIR}/libc)
add_executable(test test)
target_link_libraries(test c)