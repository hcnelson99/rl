#include <vector>
#include <unordered_set>
#include <string.h>

#include "curses_render.h"
#include "map.h"
#include "util.h"

#define BLACK 1
#define LIGHT_BLACK 9
#define RED 2
#define LIGHT_RED 10
#define GREEN 3
#define LIGHT_GREEN 11
#define YELLOW 4
#define LIGHT_YELLOW 12
#define BLUE 5
#define LIGHT_BLUE 13
#define MAGENTA 6
#define LIGHT_MAGENTA 14
#define CYAN 7
#define LIGHT_CYAN 15
#define WHITE 8
#define LIGHT_WHITE 16


// TODO: Factor out camera functions (if other graphical backends are ever written)
struct Camera {
	// pos is upper left corner
	Vector2 pos, view_size;

	void update_scroll(const Vector2 &player_pos) {
		pos = player_pos - view_size / 2;

		if (pos.x < 0) { pos.x = 0; }
		if (pos.y < 0) { pos.y = 0; }
		if (pos.x > MAP_SIZE.x - view_size.x) { pos.x = MAP_SIZE.x - view_size.x; }
		if (pos.y > MAP_SIZE.y - view_size.y) { pos.y = MAP_SIZE.y - view_size.y; }

		assert(pos_in_range(pos) && pos_in_range(pos + view_size - Vector2{1, 1}));
	}
};



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
	assert(history);

	erase();

	bool visible[MAP_TILE_COUNT];
	memset(visible, false, MAP_TILE_COUNT);

	Camera camera;
	if (scrolling) {
		camera.view_size = {80, 24};
		camera.update_scroll(player_pos);
	} else {
		camera.view_size = MAP_SIZE;
		camera.pos = {0, 0};
	}

	map.visibility(player_pos, visible);
	for (int i = 0; i < MAP_TILE_COUNT; i++) {
		if (visible[i]) {
			history[i] = visible[i];
		}
	}

	for (int y = camera.pos.y; y < camera.pos.y + camera.view_size.y; y++) {
		for (int x = camera.pos.x; x < camera.pos.x + camera.view_size.x; x++) {
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
		player_screen_location = player_pos - camera.pos;
		enemy_screen_location = enemy_pos - camera.pos;
	}  else {
		player_screen_location = player_pos;
		enemy_screen_location = enemy_pos;
	}

	attron(COLOR_PAIR(BLUE));
	mvprintw(player_screen_location.y, player_screen_location.x, "@");
	attroff(COLOR_PAIR(BLUE));

	if (enemy_screen_location.x >= 0 && enemy_screen_location.x < camera.view_size.x &&
			enemy_screen_location.y >= 0 && enemy_screen_location.y < camera.view_size.y &&
			(!visibility || visible[pos_to_index(enemy_pos)])) {
		attron(COLOR_PAIR(RED));
		mvprintw(enemy_screen_location.y, enemy_screen_location.x, "g");
		attroff(COLOR_PAIR(RED));
	}

	refresh();
}
