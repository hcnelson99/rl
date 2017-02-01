#include <ncurses.h>
#include <stdio.h>


#include "map.h"
#include "log.h"
#include "defer.h"

WINDOW* init_win() {
	WINDOW *win = initscr();
	noecho();
	curs_set(0);
	// nodelay(win, true);
	keypad(win, true);
	return win;
}

bool camera_in_range(const Vector2 &camera_pos) {
	return pos_in_range(camera_pos) && pos_in_range(camera_pos + VIEW_SIZE);
}

void move_player(const Map &map, Vector2 *player_pos, Vector2 *camera_pos, const Vector2 &dir) {
	Vector2 new_player_pos = *player_pos + dir;

	if (*map.at(new_player_pos) == Tile::Floor) {
		*player_pos = new_player_pos;
	}

	*camera_pos = *player_pos - HALF_SCREEN;

	if (camera_pos->x < 0) { camera_pos->x = 0; }
	if (camera_pos->y < 0) { camera_pos->y = 0; }
	if (camera_pos->x > MAP_SIZE.x - VIEW_SIZE.x) { camera_pos->x = MAP_SIZE.x - VIEW_SIZE.x; }
	if (camera_pos->y > MAP_SIZE.y - VIEW_SIZE.y) { camera_pos->y = MAP_SIZE.y - VIEW_SIZE.y; }
}



int main() {
	init_log("rl.log");

	init_win();
	defer(endwin());

	Map map;

	random_map(&map, 40);

	// Player's center
	Vector2 player_pos = MAP_SIZE / 2;

	// Upper left corner of screen
	Vector2 camera_pos = player_pos - HALF_SCREEN;

	bool visible[MAP_TILE_COUNT];
	clear_visibility(visible);

	while (true) {
		int c = getch();

		if (c == 'q') {
			break;
		}

		int fill_pct = -1;
		Vector2 move = {0, 0};
		bool redraw = false;

		switch (c) {
			case 'c':
				clear_visibility(visible);
				map.cellular_automata_iteration();
				redraw = true;
				break;
			case 'r':
				fill_pct = 40;
			case 'e':
				fill_pct = 45;
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
		}

		if (fill_pct != -1) {
			clear_visibility(visible);
			random_map(&map, fill_pct);
			redraw = true;
		}

		if (move != Vector2{0, 0}) {
			move_player(map, &player_pos, &camera_pos, move);
			redraw = true;
		}




		if (redraw) {
			erase();
			map.visibility(player_pos, visible);
			map.print_visible(camera_pos, visible);
			Vector2 player_screen_location = player_pos - camera_pos;
			mvprintw(player_screen_location.y, player_screen_location.x, "@");
		}
	}

	return 0;
}
