#pragma once

#include "pcg_variants.h"
#include "vector.h"

const Vector2 VIEW_SIZE = {80, 24};
const Vector2 HALF_SCREEN = VIEW_SIZE / 2;

// const Vector2 MAP_SIZE = {400, 80};
const Vector2 MAP_SIZE = {235, 73};

const int MAP_TILE_COUNT = MAP_SIZE.x * MAP_SIZE.y;

enum class Tile {
	Wall,
	Floor,
};

struct Map {
	Tile map[MAP_TILE_COUNT];

	Tile *at(const Vector2 &pos);
	const Tile *at(const Vector2 &pos) const;
	Tile *at(int i);
	const Tile *at(int i) const;

	bool occupied(const Vector2 &pos) const;
	int count_neighbors(const Vector2 &pos) const;

	void cellular_automata_iteration();
	void smooth();

	void print() const;
	void print(const Vector2 &camera_pos) const;
	void print_visible(const Vector2 &camera_pos, bool visible[MAP_TILE_COUNT]) const;

	void visibility(const Vector2 &player_pos, bool visible[MAP_TILE_COUNT]) const;

};

void clear_visibility(bool visible[MAP_TILE_COUNT]);


bool pos_in_range(const Vector2 &pos);

void empty_map(Map *map);
void filled_map(Map *map);
void bezier(Map *map);
void cave_map(Map *map, bool alec_gen_caves_before_rooms);

void random_map(Map *map, pcg32_random_t *gen, unsigned int percent);

Vector2 index_to_pos(int i);
int pos_to_index(const Vector2 &pos);
