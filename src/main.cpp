#include <ncurses.h>

#include "map.h"
#include "util.h"
#include "path_map.h"

WINDOW* init_win() {
	WINDOW *win = initscr();
	noecho();
	curs_set(0);
	// halfdelay(1);
	// nodelay(win, true);
	keypad(win, true);
	return win;
}

bool camera_in_range(const Vector2 &camera_pos) {
	return pos_in_range(camera_pos) && pos_in_range(camera_pos + VIEW_SIZE);
}

void camera_position(const Vector2 &player_pos, Vector2 *camera_pos) {
	*camera_pos = player_pos - HALF_SCREEN;

	if (camera_pos->x < 0) { camera_pos->x = 0; }
	if (camera_pos->y < 0) { camera_pos->y = 0; }
	if (camera_pos->x > MAP_SIZE.x - VIEW_SIZE.x) { camera_pos->x = MAP_SIZE.x - VIEW_SIZE.x; }
	if (camera_pos->y > MAP_SIZE.y - VIEW_SIZE.y) { camera_pos->y = MAP_SIZE.y - VIEW_SIZE.y; }

}

void move_player(const Map &map, Vector2 *player_pos, const Vector2 &dir) {
	Vector2 new_player_pos = *player_pos + dir;

	if (*map.at(new_player_pos) == Tile::Floor) {
		*player_pos = new_player_pos;
	}


}


void render_entire_map(const Map &map) {
	erase();
	map.print();
	// map.floodfill_print();
	refresh();
}

void render(const Map &map, bool visible[MAP_TILE_COUNT],
		const Vector2 &player_pos) {
	erase();

	Vector2 camera_pos;
	camera_position(player_pos, &camera_pos);

	if (visible) {
		map.visibility(player_pos, visible);
		map.print_visible(camera_pos, visible);
	} else {
		map.print(camera_pos);
	}

	Vector2 player_screen_location = player_pos - camera_pos;
	mvprintw(player_screen_location.y, player_screen_location.x, "@");

	refresh();
}


int main() {
	init_log("rl.log");

	init_win();
	defer(endwin());

	Map map;

	// bool *visible = nullptr;
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

	bool redraw = true;

	while (true) {
		// int c = getch();
		int c = ' ';

		if (c == 'q') {
			break;
		}

		Vector2 move = {0, 0};


		switch (c) {
			case 'c':
				clear_visibility(visible);
				map.cellular_automata_iteration();
				redraw = true;
				break;
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
				move = Vector2{0, -1};
				break;
			case 'a':
				move = Vector2{-1, 0};
				break;
			case 's':
				move = Vector2{0, 1};
				break;
			case 'd':
				move = Vector2{1, 0};
				break;
			case ' ':
				PathMap path_map;
				map.visibility(player_pos, visible);
				for (int i = 0; i < MAP_TILE_COUNT; i++) {
					if (*map.at(i) == Tile::Floor && !visible[i]) {
						path_map.set_goal(i);
					}
				}
				path_map.smooth(&map);

				int min = PATH_MAP_MAX;
				for (const Vector2 &o : ORTHOGONALS) {
					Vector2 pos = player_pos + o;
					if (*path_map.at(pos) < min) {
						move = o;
						min = *path_map.at(pos);
					}
				}

				break;
		}

		if (move != Vector2{0, 0}) {
			move_player(map, &player_pos, move);
			redraw = true;
		}

		if (redraw) {
			// render_entire_map(map);
			render(map, visible, player_pos);
			redraw = false;
		}
	}

	return 0;
}
