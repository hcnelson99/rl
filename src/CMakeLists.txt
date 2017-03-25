add_executable(rl
	main.cpp
	util.cpp
	map.cpp
	curses_render.cpp
	path_map.cpp)

find_package(Curses REQUIRED)

target_link_libraries(rl ${CURSES_LIBRARIES} pcg-random)
