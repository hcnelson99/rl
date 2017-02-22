#include <assert.h>
#include <thread>
#include <chrono>
#include <string.h>

#include "map.h"
#include "util.h"
#include "path_map.h"
#include "curses_render.h"

Vector2 enemy_pos;

enum MoveType {MOVE_NONE, MOVE_WAIT, MOVE_STEP};
struct Move {
	MoveType type;
	union {
		struct {Vector2 step;};
	};
};

bool move_character(const Map &map, Vector2 *character_pos, const Move &move) {
	assert(character_pos);

	if (move.type == MOVE_STEP) {
		Vector2 new_character_pos = *character_pos + move.step;

		if (*map.at(new_character_pos) == Tile::Floor) {
			*character_pos = new_character_pos;
			return true;
		}
	}
	return false;
}


const auto goal_frame_time = std::chrono::milliseconds(16);

int main() {
	init_log("rl.log");

	WINDOW *win = curses_init_win();
	defer(endwin());
	if (!win) {
		CRITICAL("Could not initialize curses window");
		return 1;
	}

	Map map;

	bool scrolling = true;
	bool render_visible = true;

	bool history[MAP_TILE_COUNT];
	memset(history, false, MAP_TILE_COUNT);

	cave_map(&map);


	// Player position
	Vector2 player_pos;
	for (int i = 0; i < MAP_TILE_COUNT; i++) {
		if (*map.at(i) == Tile::Floor) {
			player_pos = index_to_pos(i);
			break;
		}
	}

	for (int i = MAP_TILE_COUNT - 1; i >= 0; i--) {
		if (*map.at(i) == Tile::Floor) {
			enemy_pos = index_to_pos(i);
			break;
		}
	}
	bool redraw = true;

	while (true) {
		auto start_time = std::chrono::steady_clock::now();

		int c = getch();

		if (c == 'q') {
			break;
		}

		Move player_move;
		player_move.type = MOVE_NONE;

		switch (c) {
			case 'r':
				cave_map(&map);
				memset(history, false, MAP_TILE_COUNT);
				redraw = true;
				break;
			case 'w':
				player_move.type = MOVE_STEP;
				player_move.step = Vector2{0, -1};
				break;
			case 'a':
				player_move.type = MOVE_STEP;
				player_move.step = Vector2{-1, 0};
				break;
			case 's':
				player_move.type = MOVE_STEP;
				player_move.step = Vector2{0, 1};
				break;
			case 'd':
				player_move.type = MOVE_STEP;
				player_move.step = Vector2{1, 0};
				break;
			case '.':
				player_move.type = MOVE_WAIT;
				break;
			case 'v':
				if (render_visible) {
					render_visible = false;
				} else {
					render_visible = true;
				}
				redraw = true;
				break;
			case 'c':
				if (scrolling) {
					scrolling = false;
				} else {
					scrolling = true;
				}
				redraw = true;
				break;
			case ' ':
				PathMap path_map;
				for (int i = 0; i < MAP_TILE_COUNT; i++) {
					if (!history[i]) {
						path_map.set_goal(i);
					}
				}
				path_map.smooth(map);

				int min = PATH_MAP_MAX;
				for (const Vector2 &o : ORTHOGONALS) {
					Vector2 pos = player_pos + o;
					if (*path_map.at(pos) < min) {
						player_move.type = MOVE_STEP;
						player_move.step = o;
						min = *path_map.at(pos);
					}
				}

				break;
		}

		if (player_move.type == MOVE_STEP) {
			// Refactor? Should be two steps? See if valid, if so move?
			bool successful_move = move_character(map, &player_pos, player_move);
			if (successful_move) {
				redraw = true;
			} else {
				// If move was invalid (into wall), set player_move to NONE so enemies do not move
				player_move.type = MOVE_NONE;
			}
		}

		if (player_move.type != MOVE_NONE) {
			PathMap enemy_path_map;
			enemy_path_map.set_goal(player_pos);
			enemy_path_map.smooth(map);

			Move enemy_move;
			enemy_move.type = MOVE_WAIT;

			int min = *enemy_path_map.at(enemy_pos);
			for (const Vector2 &o : ORTHOGONALS) {
				Vector2 pos = enemy_pos + o;
				if (*enemy_path_map.at(pos) < min) {
					enemy_move.type = MOVE_STEP;
					enemy_move.step = o;
					min = *enemy_path_map.at(pos);
				}
			}

			move_character(map, &enemy_pos, enemy_move);
			redraw = true;
		}

		if (redraw) {
			curses_render(map, player_pos, enemy_pos, history, render_visible, scrolling);
			redraw = false;
		}


		auto current_time = std::chrono::steady_clock::now();
		auto frame_time = current_time - start_time;
		auto spare_time = goal_frame_time - frame_time;

		if (spare_time < std::chrono::seconds(0)) {
			// WARNING("Missed frame time by %ldns", -std::chrono::duration_cast<std::chrono::nanoseconds>(spare_time).count());
		} else {
			std::this_thread::sleep_for(spare_time);
		}
		// auto end_time = std::chrono::steady_clock::now();
		// LOG("framerate: %f", 1E9 / std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count());
	}

	return 0;
}
