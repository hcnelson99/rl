#include "util.h"

#include <stdio.h>
#include <assert.h>

void seed_pcg32(pcg32_random_t *rng, uint64_t initseq) {
	assert(rng);

	uint64_t seed;

	int ret = syscall(SYS_getrandom, &seed, sizeof(seed), 0);
	assert(ret == sizeof(seed));

	LOG("seed: %lu", seed);

	pcg32_srandom_r(rng, seed, initseq);
}

const char *log_level_string(LogLevel log_level) {
	const char * log_level_strings[] = {
		"CRIT" ,
		"ERROR",
		"WARN",
		"LOG",
		"DEBUG",
	};
	return log_level_strings[static_cast<int>(log_level)];
}

FILE *log_file = nullptr;
LogLevel log_level = LogLevel::Debug;



void init_log(const char *logpath) {
	assert(logpath);

	log_file = fopen(logpath, "a");
	log_level = LogLevel::Debug;
}
