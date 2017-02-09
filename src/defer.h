#pragma once

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

