#pragma once

#include <assert.h>

#include "pcg_variants.h"
#include "vector.h"

const Vector2 VIEW_SIZE = {80, 24};
const Vector2 HALF_SCREEN = VIEW_SIZE / 2;

// const Vector2 MAP_SIZE = {400, 80};
#define MAP_X 235
#define MAP_Y 73
const Vector2 MAP_SIZE = {MAP_X, MAP_Y};

#define MAP_TILE_COUNT (MAP_X * MAP_Y)

const Vector2 ADJACENTS[8] = {
	{1, 1},
	{1, 0},
	{1, -1},
	{0, 1},
	{0, -1},
	{-1, 1},
	{-1, 0},
	{-1, -1},
};

const Vector2 ORTHOGONALS[4] = {
	{-1, 0},
	{1, 0},
	{0, -1},
	{0, 1},
};

enum class Tile {
	Wall,
	Floor,
};

inline Vector2 index_to_pos(int i) {
	return {i % MAP_SIZE.x, i / MAP_SIZE.x};
}

inline int pos_to_index(const Vector2 &pos) {
	return pos.y * MAP_SIZE.x + pos.x;
}

inline bool pos_in_range(const Vector2 &pos) {
	return pos.y >= 0 && pos.y < MAP_SIZE.y && pos.x >= 0 && pos.x < MAP_SIZE.x;
}

inline bool index_in_range(int i) {
	return i >= 0 && i < MAP_TILE_COUNT;
}

struct Map {
	Tile map[MAP_TILE_COUNT];


	inline Tile *at(const Vector2 &pos) {
		assert(pos_in_range(pos));
		return &map[pos_to_index(pos)];
	}

	inline const Tile *at(const Vector2 &pos) const {
		assert(pos_in_range(pos));
		return &map[pos_to_index(pos)];
	}

	inline Tile *at(int i) {
		assert(index_in_range(i));
		return &map[i];
	}

	inline const Tile *at(int i) const {
		assert(index_in_range(i));
		return &map[i];
	}

	inline bool occupied(const Vector2 &pos) const {
		assert(pos_in_range(pos));
		return *at(pos) == Tile::Wall;
	}

	int count_neighbors(const Vector2 &pos) const;

	void cellular_automata_iteration();
	void smooth();

	void print() const;
	void floodfill_print() const;
	void print(const Vector2 &camera_pos) const;
	void print_visible(const Vector2 &camera_pos, bool visible[MAP_TILE_COUNT]) const;

	void visibility(const Vector2 &player_pos, bool visible[MAP_TILE_COUNT]) const;

	bool operator==(const Map &o) const;
	bool operator!=(const Map &o) const;

};

void clear_visibility(bool visible[MAP_TILE_COUNT]);


bool pos_in_range(const Vector2 &pos);

void empty_map(Map *map);
void filled_map(Map *map);
void bezier(Map *map);
void cave_map(Map *map, bool alec_gen_caves_before_rooms);

void random_map(Map *map, pcg32_random_t *gen, unsigned int percent);
