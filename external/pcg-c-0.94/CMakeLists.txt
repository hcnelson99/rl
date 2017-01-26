project(pcg-random)

add_library(pcg-random
	src/pcg-global-32.c
	src/pcg-global-64.c
	src/pcg-rngs-32.c
	src/pcg-advance-8.c
	src/pcg-advance-128.c
	src/pcg-rngs-64.c
	src/pcg-rngs-128.c
	src/pcg-output-64.c
	src/pcg-rngs-8.c
	src/pcg-output-32.c
	src/pcg-advance-32.c
	src/pcg-output-128.c
	src/pcg-advance-64.c
	src/pcg-rngs-16.c
	src/pcg-output-16.c
	src/pcg-output-8.c
	src/pcg-advance-16.c)

target_include_directories(lib PUBLIC include)
