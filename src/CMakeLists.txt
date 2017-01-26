add_executable(rl
	main.cpp
	log.cpp
	map.cpp
	vector.cpp)

target_link_libraries(rl ${CURSES_LIBRARIES} pcg-random)
