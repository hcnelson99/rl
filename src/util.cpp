#include "util.h"

#include <stdio.h>
#include <assert.h>


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


