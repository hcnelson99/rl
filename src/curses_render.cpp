#include <vector>
#include <unordered_set>
#include <string.h>

#include "curses_render.h"
#include "map.h"
#include "util.h"

// TODO: Factor out camera functions (if other graphical backends are ever written)
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

WINDOW* curses_init_win() {
	WINDOW *win = initscr();
	if (!has_colors()) {
		CRITICAL("Terminal needs to support color");
		return nullptr;
	}
	start_color();
	use_default_colors();
	for (int i = 0; i < 16; i++) {
		init_pair(i+1, i, -1);
	}

	noecho();
	curs_set(0);
	nodelay(win, true);
	keypad(win, true);
	return win;
}

void curses_render(const Map &map, const Vector2 &player_pos, const Vector2 &enemy_pos, bool history[MAP_TILE_COUNT], bool visibility, bool scrolling) {
	erase();

	bool visible[MAP_TILE_COUNT];
	memset(visible, false, MAP_TILE_COUNT);

	Vector2 camera_pos;
	if (scrolling) {
		VIEW_SIZE = {80, 24};
		camera_position(player_pos, &camera_pos);
	} else {
		VIEW_SIZE = MAP_SIZE;
		camera_pos = {0, 0};
	}

	map.visibility(player_pos, visible);
	for (int i = 0; i < MAP_TILE_COUNT; i++) {
		if (visible[i]) {
			history[i] = visible[i];
		}
	}

	assert(pos_in_range(camera_pos + VIEW_SIZE - Vector2{1, 1}));
	for (int y = camera_pos.y; y < camera_pos.y + VIEW_SIZE.y; y++) {
		for (int x = camera_pos.x; x < camera_pos.x + VIEW_SIZE.x; x++) {
			if (visibility) {
				if (visible[pos_to_index({x, y})]) {
					printw("%s", *map.at({x, y}) == Tile::Wall ? "#" : ".");
				} else if (history[pos_to_index({x, y})]) {
					attron(COLOR_PAIR(LIGHT_BLACK));
					printw("%s", *map.at({x, y}) == Tile::Wall ? "#" : ".");
					attroff(COLOR_PAIR(LIGHT_BLACK));
				} else {
					printw(" ");
				}
			} else {
				printw("%s", *map.at({x, y}) == Tile::Wall ? "#" : " ");
			}
		}
		printw("\n");
	}

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
			(!visibility || visible[pos_to_index(enemy_pos)])) {
		attron(COLOR_PAIR(RED));
		mvprintw(enemy_screen_location.y, enemy_screen_location.x, "g");
		attroff(COLOR_PAIR(RED));
	}

	refresh();
}
