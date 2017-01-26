#pragma once

#include "vector.h"

const int WIDTH = 235;
const int HEIGHT = 73;

const int MAP_SIZE = WIDTH * HEIGHT;

enum class Tile {
	Wall,
	Floor,
};

struct Map {
	Tile map[MAP_SIZE];

	Tile *at(const Vector2 &pos);
	const Tile *at(const Vector2 &pos) const;
	Tile *at(int i);
	const Tile *at(int i) const;

	bool occupied(const Vector2 &pos) const;
	int count_neighbors(const Vector2 &pos) const;

	void cellular_automata_iteration();

	void print();

};

bool pos_in_range(const Vector2 &pos);

void filled_map(Map *map);
void bezier(Map *map);
void random_map(Map *map);
