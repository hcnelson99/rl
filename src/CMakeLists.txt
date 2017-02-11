add_executable(rl
	main.cpp
	util.cpp
	map.cpp
	path_map.cpp
	vector.cpp)

target_link_libraries(rl ${CURSES_LIBRARIES} pcg-random)
