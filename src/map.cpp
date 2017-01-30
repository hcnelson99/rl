#include <ncurses.h>
#include <string.h>
#include <syscall.h>
#include <unistd.h>
#include <libgen.h>

#include "map.h"
#include "log.h"
#include "pcg_variants.h"

Vector2 index_to_pos(int i) {
	return {i % MAP_SIZE.x, i / MAP_SIZE.x};
}

int pos_to_index(const Vector2 &pos) {
	return pos.y * MAP_SIZE.x + pos.x;
}

bool pos_in_range(const Vector2 &pos) {
	return pos.y >= 0 && pos.y < MAP_SIZE.y && pos.x >= 0 && pos.x < MAP_SIZE.x;
}

bool index_in_range(int i) {
	return i >= 0 && i < MAP_TILE_COUNT;
}

Tile *Map::at(const Vector2 &pos) {
	if (!pos_in_range(pos)) {
		CRITICAL("Map index out of range!: %d %d", pos.x, pos.y);
	}
	return &map[pos_to_index(pos)];
}
const Tile *Map::at(const Vector2 &pos) const {
	if (!pos_in_range(pos)) {
		CRITICAL("Map index out of range!: %d %d", pos.x, pos.y);
	}
	return &map[pos_to_index(pos)];
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
	for (int x = 0; x < MAP_SIZE.x; x++) {
		for (int y = 0; y < MAP_SIZE.y; y++) {
			Vector2 pos = {x, y};
			int threshold = old_map.occupied(pos) ? 3 : 5;
			*at(pos) = old_map.count_neighbors(pos) >= threshold
				? Tile::Wall
				: Tile::Floor;
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

	for (int i = 0; i < MAP_TILE_COUNT; i++) {
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
	for (int x = 0; x < MAP_SIZE.x; x++) {
		Vector2 pos1 = {x, 0};
		Vector2 pos2 = {x, MAP_SIZE.y - 1};
		*map->at(pos1) = Tile::Wall;
		*map->at(pos2) = Tile::Wall;
	}
	for (int y = 0; y < MAP_SIZE.y; y++) {
		Vector2 pos1 = {0, y};
		Vector2 pos2 = {MAP_SIZE.x - 1, y};
		*map->at(pos1) = Tile::Wall;
		*map->at(pos2) = Tile::Wall;
	}

}

void filled_map(Map *map) {
	for (int x = 0; x < MAP_SIZE.x; x++) {
		for (int y = 0; y < MAP_SIZE.y; y++) {
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


void Map::visibility(const Vector2 &p0) {
	for (int t = 0; t < MAP_TILE_COUNT; t++) {
		Vector2 p1 = index_to_pos(t);

		int dx = p1.x - p0.x;
		int dy = p1.y - p0.y;

		int steps = max(abs(dx), abs(dy));

		double xi = dx / (double) steps;
		double yi = dy / (double) steps;

		double x = p0.x;
		double y = p0.y;
		for (int i = 0; i < steps; i++) {
			if (*at({x, y}) == Tile::Wall) {
				// visible[t] = false;
				goto next_iteration;
			}
			x += xi;
			y += yi;
		}
		visible[t] = true;
next_iteration: ;
	}
}

void Map::print_visible(const Vector2 &camera_pos, const Vector2 &player_pos) {
	// @Speed: Recalculate visibility every frame??
	visibility(player_pos);

	if (!pos_in_range(camera_pos + VIEW_SIZE - Vector2{1, 1})) {
		CRITICAL("Position (%d + %d, %d + %d) too far for map size %d, %d",
				camera_pos.x, VIEW_SIZE.x, camera_pos.y, VIEW_SIZE.y, MAP_SIZE.x, MAP_SIZE.y);
	}

	for (int y = camera_pos.y; y < camera_pos.y + VIEW_SIZE.y; y++) {
		for (int x = camera_pos.x; x < camera_pos.x + VIEW_SIZE.x; x++) {
			if (visible[pos_to_index({x, y})]) {
				printw("%s", *at({x, y}) == Tile::Wall ? "#" : ".");
			} else {
				printw(" ");
			}
		}
		printw("\n");
	}
}

void Map::print(Vector2 pos) const {
	if (!pos_in_range(pos + VIEW_SIZE - Vector2{1, 1})) {
		CRITICAL("Position (%d + %d, %d + %d) too far for map size %d, %d",
				pos.x, VIEW_SIZE.x, pos.y, VIEW_SIZE.y, MAP_SIZE.x, MAP_SIZE.y);
	}

	for (int y = pos.y; y < pos.y + VIEW_SIZE.y; y++) {
		for (int x = pos.x; x < pos.x + VIEW_SIZE.x; x++) {
			printw("%s", *at({x, y}) == Tile::Wall ? "#" : " ");
		}
		printw("\n");
	}

}
