#pragma once

struct Vector2 {
	int x, y;
	Vector2 operator+(const Vector2 &v) const;
	bool operator==(const Vector2 &v) const;
};
