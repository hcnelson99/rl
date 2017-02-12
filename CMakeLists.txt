cmake_minimum_required(VERSION 2.8)

# This has to come before the project line
# set(CMAKE_C_COMPILER "clang")
# set(CMAKE_CXX_COMPILER "clang++")

project(rl)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -ggdb -std=c++11 -fno-exceptions")

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer -fsanitize=memory")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS}")

find_package(Curses REQUIRED)
add_library(curses INTERFACE IMPORTED)
set_property(TARGET curses PROPERTY
	INTERFACE_INCLUDE_DIRECTORIES ${Curses_INCLUDE_DIR})

add_subdirectory(external EXCLUDE_FROM_ALL)

add_subdirectory(src)
