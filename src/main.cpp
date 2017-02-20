#include <assert.h>
#include <thread>
#include <chrono>

#include "map.h"
#include "util.h"
#include "path_map.h"
#include "curses_render.h"

Vector2 enemy_pos;

bool camera_in_range(const Vector2 &camera_pos) {
	return pos_in_range(camera_pos) && pos_in_range(camera_pos + VIEW_SIZE);
}

void camera_position(const Vector2 &player_pos, Vector2 *camera_pos) {
	*camera_pos = player_pos - VIEW_SIZE / 2;

	if (camera_pos->x < 0) { camera_pos->x = 0; }
	if (camera_pos->y < 0) { camera_pos->y = 0; }
	if (camera_pos->x > MAP_SIZE.x - VIEW_SIZE.x) { camera_pos->x = MAP_SIZE.x - VIEW_SIZE.x; }
	if (camera_pos->y > MAP_SIZE.y - VIEW_SIZE.y) { camera_pos->y = MAP_SIZE.y - VIEW_SIZE.y; }

}

enum MoveType {MOVE_NONE, MOVE_WAIT, MOVE_STEP};
struct Move {
	MoveType type;
	union {
		struct {Vector2 step;};
	};
};

bool move_character(const Map &map, Vector2 *character_pos, const Move &move) {
	if (move.type == MOVE_STEP) {
		Vector2 new_character_pos = *character_pos + move.step;

		if (*map.at(new_character_pos) == Tile::Floor) {
			*character_pos = new_character_pos;
			return true;
		}
	}
	return false;
}

void render(const Map &map, bool visible[MAP_TILE_COUNT],
		const Vector2 &player_pos, bool scrolling) {
	erase();

	Vector2 camera_pos;
	if (scrolling) {
		VIEW_SIZE = {80, 24};
		camera_position(player_pos, &camera_pos);
	} else {
		VIEW_SIZE = MAP_SIZE;
		camera_pos = {0, 0};
	}

	if (visible) {
		map.visibility(player_pos, visible);
	}
	curses_render(map, camera_pos, visible);


	Vector2 player_screen_location;
	Vector2 enemy_screen_location;
	if (scrolling) {
		player_screen_location = player_pos - camera_pos;
		enemy_screen_location = enemy_pos - camera_pos;
	}  else {
		player_screen_location = player_pos;
		enemy_screen_location = enemy_pos;
	}

	attron(COLOR_PAIR(BLUE));
	mvprintw(player_screen_location.y, player_screen_location.x, "@");
	attroff(COLOR_PAIR(BLUE));

	if (enemy_screen_location.x >= 0 && enemy_screen_location.x < VIEW_SIZE.x &&
			enemy_screen_location.y >= 0 && enemy_screen_location.y < VIEW_SIZE.y &&
			(!visible || visible[pos_to_index(enemy_pos)])) {
		attron(COLOR_PAIR(RED));
		mvprintw(enemy_screen_location.y, enemy_screen_location.x, "g");
		attroff(COLOR_PAIR(RED));
	}

	refresh();
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

	bool visible[MAP_TILE_COUNT];
	clear_visibility(visible);

	cave_map(&map, false);


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
				clear_visibility(visible);
				cave_map(&map, false);
				redraw = true;
				break;
			case 'e':
				clear_visibility(visible);
				cave_map(&map, true);
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
				map.visibility(player_pos, visible);
				for (int i = 0; i < MAP_TILE_COUNT; i++) {
					if (!visible[i]) {
						path_map.set_goal(i);
					}
				}
				path_map.smooth(&map);

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
			enemy_path_map.smooth(&map);

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
			render(map, render_visible ? visible : nullptr, player_pos, scrolling);
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
