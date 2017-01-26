#include "vector.h"

Vector2 Vector2::operator+(const Vector2 &v) const {
	return {x + v.x, y + v.y};
}

bool Vector2::operator==(const Vector2 &v) const {
	return x == v.x && y == v.y;
}
