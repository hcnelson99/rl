#include "path_map.h"

#include <string.h>
#include <assert.h>

#include "util.h"

PathMap::PathMap() {
	for (int i = 0; i < MAP_TILE_COUNT; i++) {
		*at(i) = PATH_MAP_MAX;
	}
}

int *PathMap::at(const Vector2 &pos) {
	assert(pos_in_range(pos));
	return &map[pos_to_index(pos)];
}
const int *PathMap::at(const Vector2 &pos) const {
	assert(pos_in_range(pos));
	return &map[pos_to_index(pos)];
}
int *PathMap::at(int i) {
	assert(index_in_range(i));
	return &map[i];
}
const int *PathMap::at(int i) const {
	assert(index_in_range(i));
	return &map[i];
}

void PathMap::set_goal(const Vector2 &pos) {
	*at(pos) = 0;
}

void PathMap::set_goal(int i) {
	*at(i) = 0;
}

bool PathMap::operator==(const PathMap &o) const {
	for (int i = 0; i < MAP_TILE_COUNT; i++) {
		if (*at(i) != *o.at(i)) {
			return false;
		}
	}
	return true;
}

bool PathMap::operator!=(const PathMap &o) const {
	return !(*this == o);
}

void PathMap::smooth(const Map *map) {
	PathMap old_path_map;
	do {
		memcpy(&old_path_map, this, sizeof(PathMap));

		for (int i = 0; i < MAP_TILE_COUNT; i++) {
			Vector2 pos = index_to_pos(i);
			if (*map->at(pos) == Tile::Floor) {
				int min = PATH_MAP_MAX;
				for (const Vector2 &o : ORTHOGONALS) {
					Vector2 adj = pos + o;
					if (pos_in_range(adj)) {
						min = min(*at(adj), min);
					}
				}
				if (*at(pos) > min + 1) {
					*at(pos) = min + 1;
				}
			}
		}
	} while (old_path_map != *this);
}
