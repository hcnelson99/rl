#pragma once

#include <cstddef>
#include <functional>

struct Vector2 {
	int x, y;
	Vector2 operator+(const Vector2 &v) const;
	Vector2 operator-(const Vector2 &v) const;
	Vector2 operator*(int r) const;
	Vector2 operator/(int r) const;
	void operator+=(const Vector2 &v);
	void operator-=(const Vector2 &v);
	bool operator==(const Vector2 &v) const;
	bool operator!=(const Vector2 &v) const;
};

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

namespace std {
	template<>
	struct hash<Vector2> {
		std::size_t operator()(const Vector2 &v) const {
			std::size_t hash = 0;
			hash_combine(hash, v.x);
			hash_combine(hash, v.y);
			return hash;
		}
	};
}
