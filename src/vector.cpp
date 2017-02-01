#include "vector.h"

Vector2 Vector2::operator+(const Vector2 &v) const {
	return {x + v.x, y + v.y};
}

Vector2 Vector2::operator-(const Vector2 &v) const {
	return {x - v.x, y - v.y};
}

Vector2 Vector2::operator*(int r) const {
	return {x * r, y * r};
}

Vector2 Vector2::operator/(int r) const {
	return {x / r, y / r};
}

void Vector2::operator+=(const Vector2 &v) {
	(*this) = (*this) + v;
}

void Vector2::operator-=(const Vector2 &v) {
	(*this) = (*this) - v;
}

bool Vector2::operator==(const Vector2 &v) const {
	return x == v.x && y == v.y;
}
bool Vector2::operator!=(const Vector2 &v) const {
	return !((*this) == v);
}
