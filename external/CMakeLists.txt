set(PCG_DIR pcg-c-0.94)
add_library(pcg-random
	${PCG_DIR}/src/pcg-global-32.c
	${PCG_DIR}/src/pcg-global-64.c
	${PCG_DIR}/src/pcg-rngs-32.c
	${PCG_DIR}/src/pcg-advance-8.c
	${PCG_DIR}/src/pcg-advance-128.c
	${PCG_DIR}/src/pcg-rngs-64.c
	${PCG_DIR}/src/pcg-rngs-128.c
	${PCG_DIR}/src/pcg-output-64.c
	${PCG_DIR}/src/pcg-rngs-8.c
	${PCG_DIR}/src/pcg-output-32.c
	${PCG_DIR}/src/pcg-advance-32.c
	${PCG_DIR}/src/pcg-output-128.c
	${PCG_DIR}/src/pcg-advance-64.c
	${PCG_DIR}/src/pcg-rngs-16.c
	${PCG_DIR}/src/pcg-output-16.c
	${PCG_DIR}/src/pcg-output-8.c
	${PCG_DIR}/src/pcg-advance-16.c)

target_include_directories(pcg-random PUBLIC ${PCG_DIR}/include)
