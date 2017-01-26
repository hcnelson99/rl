cmake_minimum_required(VERSION 2.8)
project(rl)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -ggdb -std=c++11 -fno-exceptions")

find_package(Curses REQUIRED)
add_library(curses INTERFACE IMPORTED)
set_property(TARGET curses PROPERTY
	INTERFACE_INCLUDE_DIRECTORIES ${Curses_INCLUDE_DIR})

add_subdirectory(external EXCLUDE_FROM_ALL)

add_subdirectory(src)
