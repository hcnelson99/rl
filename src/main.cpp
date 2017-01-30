#include <ncurses.h>
#include <stdio.h>


#include "map.h"
#include "log.h"
#include "defer.h"

WINDOW* init_win() {
	WINDOW *win = initscr();
	noecho();
	curs_set(0);
	nodelay(win, true);
	keypad(win, true);
	return win;
}

bool camera_in_range(const Vector2 &camera_pos) {
	return pos_in_range(camera_pos) && pos_in_range(camera_pos + VIEW_SIZE);
}

void move(const Map &map, Vector2 *player_pos, Vector2 *camera_pos, const Vector2 &dir) {
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
	random_map(&map);

	// Player's center
	Vector2 player_pos = MAP_SIZE / 2;

	// Upper left corner of screen
	Vector2 camera_pos = player_pos - HALF_SCREEN;


	while (true) {
		int c = getch();

		if (c == 'q') {
			break;
		}

		switch (c) {
			case 'r':
				random_map(&map);
				break;
			case 'w':
				move(map, &player_pos, &camera_pos, {0, -1});
				break;
			case 'a':
				move(map, &player_pos, &camera_pos, {-1, 0});
				break;
			case 's':
				move(map, &player_pos, &camera_pos, {0, 1});
				break;
			case 'd':
				move(map, &player_pos, &camera_pos, {1, 0});
				break;
		}


		erase();

		map.print_visible(camera_pos, player_pos);
		Vector2 player_screen_location = player_pos - camera_pos;
		mvprintw(player_screen_location.y, player_screen_location.x, "@");
	}

	return 0;
}
