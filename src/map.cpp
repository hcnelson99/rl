#include <ncurses.h>
#include <stdint.h>
#include <ncurses.h>
#include <string.h>
#include <syscall.h>
#include <unistd.h>

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

// p1 is upper left (inclusive), p2 is upper right (exclusive)
struct Rect {
	Vector2 p1, p2;
};

void random_room(Rect *room, pcg32_random_t *gen) {
	int width = pcg32_boundedrand_r(gen, 10) + 8;
	int height = pcg32_boundedrand_r(gen, 10) + 8;
	room->p1.x = pcg32_boundedrand_r(gen, MAP_SIZE.x - width + 1);
	room->p1.y = pcg32_boundedrand_r(gen, MAP_SIZE.y - height + 1);
	room->p2.x = room->p1.x + width;
	room->p2.y = room->p1.y + height;
}

bool overlaps(const Rect &r1, const Rect &r2) {
	return !(r1.p2.x < r2.p1.x || r1.p2.y < r2.p1.y ||
		    r2.p2.x < r1.p1.x || r2.p2.y < r1.p1.y);
}

void alec_random(Map *map) {
	unsigned int percent = 40;
	pcg32_random_t gen;

	seed_pcg32(&gen, 0);

	for (int i = 0; i < MAP_TILE_COUNT; i++) {
		if (pcg32_boundedrand_r(&gen, 100) < percent) {
			*map->at(i) = Tile::Wall;
		} else {
			*map->at(i) = Tile::Floor;
		}
	}

	// Go to stability
	for (int i = 0; i < 100; i++) {
		map->cellular_automata_iteration();
	}

	const int max_room_num = 20;
	Rect rooms[max_room_num];

	int room_num = max_room_num;

	for (int i = 0; i < room_num; i++) {
		Rect room;
		bool overlap;
		const int iteration_limit = 100;
		int n = 0;
		do {
			overlap = false;
			random_room(&room, &gen);

			Vector2 border = {5, 5};

			Rect larger_room { room.p1 - border, room.p2 + border};
			if (larger_room.p1.x < 0) larger_room.p1.x = 0;
			if (larger_room.p1.y < 0) larger_room.p1.y = 0;
			if (larger_room.p2.x > MAP_SIZE.x - 1) larger_room.p2.x = MAP_SIZE.x - 1;
			if (larger_room.p2.y > MAP_SIZE.y - 1) larger_room.p2.y = MAP_SIZE.y - 1;

			for (int j = 0; j < i; j++) {
				if (overlaps(larger_room, rooms[j])) {
					overlap = true;
				}
			}
			n++;
		} while (overlap && n < iteration_limit);

		if (n == iteration_limit) {
			room_num = i;
			break;
		} else {
			rooms[i] = room;
		}
	}


	for (int i = 0; i < room_num; i++) {
		Rect room = rooms[i];
		for (int x = room.p1.x; x < room.p2.x; x++) {
			for (int y = room.p1.y; y < room.p2.y; y++) {
				*map->at({x, y}) = Tile::Wall;
			}
		}
	}

	// Go to stability
	for (int i = 0; i < 100; i++) {
		map->cellular_automata_iteration();
	}

	for (int i = 0; i < room_num; i++) {
		Rect room = rooms[i];
		room.p1 += Vector2 {1, 1};
		room.p2 -= Vector2 {1, 1};
		for (int x = room.p1.x; x < room.p2.x; x++) {
			for (int y = room.p1.y; y < room.p2.y; y++) {
				*map->at({x, y}) = Tile::Floor;
			}
		}
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

void random_map(Map *map) {
	unsigned int percent = 40;
	pcg32_random_t gen;

	seed_pcg32(&gen, 0);

	for (int i = 0; i < MAP_TILE_COUNT; i++) {
		if (pcg32_boundedrand_r(&gen, 100) < percent) {
			*map->at(i) = Tile::Wall;
		} else {
			*map->at(i) = Tile::Floor;
		}
	}


	const int max_room_num = 20;
	Rect rooms[max_room_num];

	int room_num = max_room_num;

	for (int i = 0; i < room_num; i++) {
		Rect room;
		bool overlap;
		const int iteration_limit = 100;
		int n = 0;
		do {
			overlap = false;
			random_room(&room, &gen);

			Vector2 border = {5, 5};

			Rect larger_room { room.p1 - border, room.p2 + border};
			if (larger_room.p1.x < 0) larger_room.p1.x = 0;
			if (larger_room.p1.y < 0) larger_room.p1.y = 0;
			if (larger_room.p2.x > MAP_SIZE.x - 1) larger_room.p2.x = MAP_SIZE.x - 1;
			if (larger_room.p2.y > MAP_SIZE.y - 1) larger_room.p2.y = MAP_SIZE.y - 1;

			for (int j = 0; j < i; j++) {
				if (overlaps(larger_room, rooms[j])) {
					overlap = true;
				}
			}
			n++;
		} while (overlap && n < iteration_limit);

		if (n == iteration_limit) {
			room_num = i;
			break;
		} else {
			rooms[i] = room;
		}
	}


	for (int i = 0; i < room_num; i++) {
		Rect room = rooms[i];
		for (int x = room.p1.x; x < room.p2.x; x++) {
			for (int y = room.p1.y; y < room.p2.y; y++) {
				*map->at({x, y}) = Tile::Wall;
			}
		}
	}

	// Go to stability
	for (int i = 0; i < 100; i++) {
		map->cellular_automata_iteration();
	}

	for (int i = 0; i < room_num; i++) {
		Rect room = rooms[i];
		room.p1 += Vector2{1, 1};
		room.p2 -= Vector2{1, 1};
		for (int x = room.p1.x; x < room.p2.x; x++) {
			for (int y = room.p1.y; y < room.p2.y; y++) {
				*map->at({x, y}) = Tile::Floor;
			}
		}
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

void empty_map(Map *map) {
	for (int x = 0; x < MAP_SIZE.x; x++) {
		for (int y = 0; y < MAP_SIZE.y; y++) {
			*map->at({x, y}) = Tile::Floor;
		}
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


void Map::visibility(const Vector2 &player_pos, bool visible[MAP_TILE_COUNT]) const {
	for (int t = 0; t < MAP_TILE_COUNT; t++) {
		Vector2 p1 = index_to_pos(t);
		LOG("%d %d", p1.x, p1.y);

		int dx = p1.x - player_pos.x;
		int dy = p1.y - player_pos.y;

		int steps = max(abs(dx), abs(dy));

		double xi = dx / (double) steps;
		double yi = dy / (double) steps;

		double x = player_pos.x;
		double y = player_pos.y;
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

void Map::print_visible(const Vector2 &camera_pos, bool visible[MAP_TILE_COUNT]) const {
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

void clear_visibility(bool visible[MAP_TILE_COUNT]) {
	if (visible)
		memset(visible, false, MAP_TILE_COUNT);
}

void Map::print() const {
	for (int y = 0; y < MAP_SIZE.y; y++) {
		for (int x = 0; x < MAP_SIZE.x; x++) {
			printw("%s", *at({x, y}) == Tile::Wall ? "#" : " ");
		}
		printw("\n");
	}

}
void Map::print(const Vector2 &camera_pos) const {
	if (!pos_in_range(camera_pos + VIEW_SIZE - Vector2{1, 1})) {
		CRITICAL("Position (%d + %d, %d + %d) too far for map size %d, %d",
				camera_pos.x, VIEW_SIZE.x, camera_pos.y, VIEW_SIZE.y, MAP_SIZE.x, MAP_SIZE.y);
	}

	for (int y = camera_pos.y; y < camera_pos.y + VIEW_SIZE.y; y++) {
		for (int x = camera_pos.x; x < camera_pos.x + VIEW_SIZE.x; x++) {
			printw("%s", *at({x, y}) == Tile::Wall ? "#" : " ");
		}
		printw("\n");
	}

}
