#include <stdint.h>
#include <string.h>

#include <queue>
#include <algorithm>

#include "map.h"
#include "util.h"
#include "path_map.h"

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

bool Map::operator==(const Map &o) const {
	for (int i = 0; i < MAP_TILE_COUNT; i++) {
		if (*at(i) != *o.at(i)) {
			return false;
		}
	}
	return true;
}

bool Map::operator!=(const Map &o) const {
	return !(*this == o);
}

void Map::smooth() {
	Map old_map;
	do {
		memcpy(&old_map, this, sizeof(Map));
		cellular_automata_iteration();
	} while (old_map != *this);
}


// p1 is upper left (inclusive), p2 is upper right (exclusive)
struct Rect {
	Vector2 p1, p2;
};

void random_room(Rect *room, pcg32_random_t *gen) {
	assert(room);
	assert(gen);

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

void random_map(Map *map, pcg32_random_t *gen, unsigned int percent) {
	assert(map);
	assert(gen);

	for (int i = 0; i < MAP_TILE_COUNT; i++) {
		if (pcg32_boundedrand_r(gen, 100) < percent) {
			*map->at(i) = Tile::Wall;
		} else {
			*map->at(i) = Tile::Floor;
		}
	}
}

// Returns room_num since number of requested rooms may not be placed
int generate_random_rooms(Rect *rooms, int room_num, pcg32_random_t *gen) {
	for (int i = 0; i < room_num; i++) {
		Rect room;
		bool overlap;
		const int iteration_limit = 100;
		int n = 0;
		do {
			overlap = false;
			random_room(&room, gen);

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


	return room_num;
}

void fill_edges(Map *map) {
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

void fill_rooms(Map *map, Rect *rooms, int room_num) {
	for (int i = 0; i < room_num; i++) {
		Rect room = rooms[i];
		for (int x = room.p1.x; x < room.p2.x; x++) {
			for (int y = room.p1.y; y < room.p2.y; y++) {
				*map->at({x, y}) = Tile::Wall;
			}
		}
	}
}

void clear_room_interiors(Map *map, Rect *rooms, int room_num) {
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

}

std::vector<std::unordered_set<Vector2>> floodfill(const Map &map) {
	std::vector<std::unordered_set<Vector2>> regions;
	std::unordered_set<Vector2> visited;

	for (int i = 0; i < MAP_TILE_COUNT; i++) {
		Vector2 pos = index_to_pos(i);

		if (*map.at(i) == Tile::Floor && !contains(visited, pos)) {
			std::queue<Vector2> queue;
			std::unordered_set<Vector2> pocket;

			queue.push(pos);
			pocket.insert(pos);

			do {
				pos = queue.front();
				queue.pop();

				for (const Vector2 &o : ORTHOGONALS) {
					Vector2 adj = pos + o;
					if (pos_in_range(adj) && *map.at(adj) == Tile::Floor && !contains(pocket, adj)) {
						queue.push(adj);
						pocket.insert(adj);
					}
				}


			} while (!queue.empty());

			visited.insert(std::begin(pocket), std::end(pocket));
			regions.push_back(pocket);
		}
	}

	return regions;
}

void connect_regions(Map *map) {
	std::vector<std::unordered_set<Vector2>> regions = floodfill(*map);

	while (regions.size() > 1) {
		int min_dist = MAP_SIZE.x + MAP_SIZE.y;
		Vector2 min1, min2;

		for (unsigned int i = 1; i < regions.size(); i++) {
			for (Vector2 pos1 : regions[0]) {
				for (Vector2 pos2 : regions[i]) {
					int dist = abs(pos1.x - pos2.x) + abs(pos1.y - pos2.y);
					if (dist < min_dist) {
						min_dist = dist;
						min1 = pos1;
						min2 = pos2;
					}
				}
			}
		}

		while (min1 != min2) {
			if (abs(min1.x - min2.x) > abs(min1.y - min2.y)) {
				min1.x += min1.x < min2.x ? 1 : -1;
			} else {
				min1.y += min1.y < min2.y ? 1 : -1;
			}
			*map->at(min1) = Tile::Floor;
		}


		regions = floodfill(*map);
	}
}

void cave_map(Map *map) {
	pcg32_random_t gen;

	auto seed = gen_seed();
	LOG("map gen seed: %lu", seed);
	seed_pcg32(&gen, seed);

	random_map(map, &gen, 40);

	const int max_room_num = 20;
	Rect rooms[max_room_num];

	int room_num = generate_random_rooms(rooms, max_room_num, &gen);

	fill_rooms(map, rooms, room_num);

	map->smooth();

	clear_room_interiors(map, rooms, room_num);

	fill_edges(map);

	connect_regions(map);
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
						*map->at({(int)(x + i), (int)(y + j)}) = Tile::Floor;
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

		int dx = p1.x - player_pos.x;
		int dy = p1.y - player_pos.y;

		int steps = max(abs(dx), abs(dy));

		double xi = dx / (double) steps;
		double yi = dy / (double) steps;

		double x = player_pos.x;
		double y = player_pos.y;
		for (int i = 0; i < steps; i++) {
			if (*at({(int)x, (int)y}) == Tile::Wall) {
				goto next_iteration;
			}
			x += xi;
			y += yi;
		}
		visible[t] = true;
next_iteration: ;
	}
}
