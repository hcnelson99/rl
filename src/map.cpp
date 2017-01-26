#include <ncurses.h>
#include <string.h>
#include <syscall.h>
#include <unistd.h>
#include <libgen.h>

#include "map.h"
#include "log.h"
#include "pcg_variants.h"

bool pos_in_range(const Vector2 &pos) {
	return pos.y >= 0 && pos.y < HEIGHT && pos.x >= 0 && pos.x < WIDTH;
}

bool index_in_range(int i) {
	return i >= 0 && i < WIDTH * HEIGHT;
}

Tile *Map::at(const Vector2 &pos) {
	if (!pos_in_range(pos)) {
		CRITICAL("Map index out of range!: %d %d", pos.x, pos.y);
	}
	return &map[pos.y * WIDTH + pos.x];
}
const Tile *Map::at(const Vector2 &pos) const {
	if (!pos_in_range(pos)) {
		CRITICAL("Map index out of range!: %d %d", pos.x, pos.y);
	}
	return &map[pos.y * WIDTH + pos.x];
}
Tile *Map::at(int i) {
	if (!index_in_range(i)) {
		CRITICAL("Map index out of range!: %d", i);
	}
	return &map[i];
}
const Tile *Map::at(int i) const {
	if (!index_in_range(i)) {
		CRITICAL("Map index out of range!: %d", i);
	}
	return &map[i];
}

bool Map::occupied(const Vector2 &pos) const {
	if (!pos_in_range(pos)) {
		CRITICAL("Map index out of range!: %d %d", pos.x, pos.y);
	}
	return *at(pos) == Tile::Wall;
}

#define ARRAY_LEN(x) sizeof(x)/sizeof(x[0])

const Vector2 ADJACENTS[] = {
	{1, 1},
	{1, 0},
	{1, -1},
	{0, 1},
	{0, -1},
	{-1, 1},
	{-1, 0},
	{-1, -1},
};

int Map::count_neighbors(const Vector2 &pos) const {
	int neighbors = 0;
	for (unsigned int i = 0; i < ARRAY_LEN(ADJACENTS); i++) {
		Vector2 adj = pos + ADJACENTS[i];
		if (pos_in_range(adj)) {
			if (occupied(adj)) {
				neighbors++;
			}
		} else {
			neighbors++;
		}
	}
	return neighbors;
}


void Map::cellular_automata_iteration() {
	Map old_map;
	memcpy(&old_map, &map, sizeof(Map));
	for (int x = 0; x < WIDTH; x++) {
		for (int y = 0; y < HEIGHT; y++) {
			Vector2 pos = {x, y};
			int threshold = old_map.occupied(pos) ? 3 : 5;
			*at(pos) = old_map.count_neighbors(pos) >= threshold
				? Tile::Wall
				: Tile::Floor;
		}
	}
}

void Map::print() {
	for (int i = 0; i < MAP_SIZE; i++) {
		printw("%s", *at(i) == Tile::Wall ? "#" : ".");
		if ((i + 1) % WIDTH == 0) {
			printw("\n");
		}
	}
}

void seed_pcg32(pcg32_random_t *rng, uint64_t initseq) {
	uint64_t seed;
	if (syscall(SYS_getrandom, &seed, sizeof(seed), 0) != 8) {
		ERROR("Could not seed generator");
	}
	pcg32_srandom_r(rng, seed, initseq);
}

void random_map(Map *map) {
	pcg32_random_t gen;

	seed_pcg32(&gen, 0);

	for (int i = 0; i < MAP_SIZE; i++) {
		if (pcg32_boundedrand_r(&gen, 100) < 40) {
			map->map[i] = Tile::Wall;
		} else {
			map->map[i] = Tile::Floor;
		}
	}

	for (int i = 0; i < 5; i++) {
		map->cellular_automata_iteration();
	}

	// Fill edges
	for (int x = 0; x < WIDTH; x++) {
		Vector2 pos1 = {x, 0};
		Vector2 pos2 = {x, HEIGHT-1};
		*map->at(pos1) = Tile::Wall;
		*map->at(pos2) = Tile::Wall;
	}
	for (int y = 0; y < HEIGHT; y++) {
		Vector2 pos1 = {0, y};
		Vector2 pos2 = {WIDTH-1, y};
		*map->at(pos1) = Tile::Wall;
		*map->at(pos2) = Tile::Wall;
	}

}

void filled_map(Map *map) {
	for (int x = 0; x < WIDTH; x++) {
		for (int y = 0; y < HEIGHT; y++) {
			*map->at({x, y}) = Tile::Wall;
		}
	}
}


#define min(x, y) (x < y ? x : y)
#define max(x, y) (x > y ? x : y)

Vector2 bezier3(const Vector2 &p0, const Vector2 &p1, const Vector2 &p2, double t) {
	double x = (1 - t * t) * p0.x + 2 * (1 - t) * t * p1.x + t * t * p2.x;
	double y = (1 - t * t) * p0.y + 2 * (1 - t) * t * p1.y + t * t * p2.y;
	return {(int) x, (int) y};
}

#define abs(x) (x > 0 ? x : -x)

Vector2 start = {2, 2};
Vector2 end = {78, 22};
Vector2 middle = {2, 22};

void bezier(Map *map) {
	pcg32_random_t gen;

	seed_pcg32(&gen, 0);

	Vector2 p0 = start;
	Vector2 p1;
	for (double t = 0; t <= 1.0; t += 1. / 50) {
		p1 = bezier3(start, middle, end, t);

		int dx = p1.x - p0.x;
		int dy = p1.y - p0.y;

		int steps = max(abs(dx), abs(dy));

		double xi = dx / (double) steps;
		double yi = dy / (double) steps;

		double x = p0.x;
		double y = p0.y;
		for (int i = 0; i < steps; i++) {
			for (int i = -1; i <= 1; i++) {
				for (int j = -1; j <= 1; j++) {
					if (pcg32_boundedrand_r(&gen, 100) < 60) {
						*map->at({x + i, y + j}) = Tile::Floor;
					}
				}
			}
			// map.at({x, y}) = Tile::Floor;
			x += xi;
			y += yi;
		}


		p0 = p1;
	}

	for (int i = 0; i < 5; i++) {
		map->cellular_automata_iteration();
	}
}
