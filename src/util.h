#pragma once

#include <stdio.h>
#include <libgen.h>
#include <time.h>
#include <unordered_set>
#include <syscall.h>
#include <unistd.h>

#include "pcg_variants.h"

template <typename F>
struct Defer {
	Defer(F f) : f(f) {}
	~Defer() { f(); }
	F f;
};

template <typename F>
Defer<F> make_defer(F f) {
	return Defer<F>(f);
}

#define CONCAT_1(x, y) x##y
#define CONCAT(x, y) CONCAT_1(x, y)
#define defer(expr) auto CONCAT(_defer_, __COUNTER__) = make_defer([&]() {expr;})

uint64_t gen_seed();
void seed_pcg32(pcg32_random_t *rng, uint64_t seed);


#define ARRAY_LEN(x) sizeof(x)/sizeof(x[0])

template <class T>
bool contains(const std::unordered_set<T> &s, const T &e) {
	return s.find(e) != s.end();
}

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
