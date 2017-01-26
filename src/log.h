#pragma once

#include <stdio.h>
#include <libgen.h>
#include <time.h>

enum class LogLevel {
	Critical = 0,
	Error,
	Warning,
	Log,
	Debug,
};


extern FILE *log_file;
extern LogLevel log_level;

void init_log(const char *);
const char *log_level_string(LogLevel log_level);

#define LOGL_HELPER(level, fmt, ...) do { \
	if (level <= log_level) { \
		time_t t = time(nullptr); \
		char time_str[80]; \
		strftime(time_str, 80, "%F %T", localtime(&t)); \
		fprintf(log_file,"[%s]\t %s %s:%d " fmt "%s\n", log_level_string(level), \
				time_str, basename((char*)__FILE__),  __LINE__, __VA_ARGS__); \
		fflush(log_file); \
	} \
} while(0)

#define LOGL(...) LOGL_HELPER(__VA_ARGS__, "")

#define CRITICAL(...) LOGL(LogLevel::Critical, __VA_ARGS__)
#define ERROR(...) LOGL(LogLevel::Error, __VA_ARGS__)
#define WARNING(...) LOGL(LogLevel::Warning, __VA_ARGS__)
#define LOG(...) LOGL(LogLevel::Log, __VA_ARGS__)
#define DEBUG(...) LOGL(LogLevel::Debug, __VA_ARGS__)
