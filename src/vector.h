#pragma once

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
