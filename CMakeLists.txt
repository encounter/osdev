cmake_minimum_required(VERSION 3.2)

if (APPLE)
	string(REPLACE "-Wl,-search_paths_first" "" CMAKE_C_LINK_FLAGS ${CMAKE_C_LINK_FLAGS})
endif ()

set(CMAKE_SYSTEM_NAME "Generic" CACHE STRING "Target system.")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -std=c11 -Wall -Werror -ffreestanding -fno-pie -nostartfiles -nostdlib")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-Ttext=0x1000 -Wl,--oformat=binary") # -Wl,--print-map

add_subdirectory(boot)
add_subdirectory(kernel)
add_subdirectory(libc)

if (APPLE)
	set(CMAKE_C_COMPILER i386-elf-gcc)
endif ()

add_executable(kernel.bin dummy.c)
target_link_libraries(kernel.bin kernel c)
