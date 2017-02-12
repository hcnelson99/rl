#pragma once

#include <cstddef>
#include <functional>

struct Vector2 {
	int x, y;

	inline Vector2 operator+(const Vector2 &v) const {
		return {x + v.x, y + v.y};
	}

	inline Vector2 operator-(const Vector2 &v) const {
		return {x - v.x, y - v.y};
	}

	inline Vector2 operator*(int r) const {
		return {x * r, y * r};
	}

	inline Vector2 operator/(int r) const {
		return {x / r, y / r};
	}

	inline void operator+=(const Vector2 &v) {
		(*this) = (*this) + v;
	}

	inline void operator-=(const Vector2 &v) {
		(*this) = (*this) - v;
	}

	inline bool operator==(const Vector2 &v) const {
		return x == v.x && y == v.y;
	}
	inline bool operator!=(const Vector2 &v) const {
		return !((*this) == v);
	}
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
