#pragma once

#include "map.h"

#define PATH_MAP_MAX 999999

struct PathMap {
	int map[MAP_TILE_COUNT];

	PathMap();

	int *at(const Vector2 &pos);
	const int *at(const Vector2 &pos) const;
	int *at(int i);
	const int *at(int i) const;

	void set_goal(const Vector2 &pos);
	void set_goal(int i);
	void smooth(const Map &map);

	bool operator==(const PathMap &o) const;
	bool operator!=(const PathMap &o) const;
};


