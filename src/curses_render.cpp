#include <vector>
#include <unordered_set>

#include "curses_render.h"
#include "util.h"

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

void curses_render(const Map &map, const Vector2 &camera_pos, bool visible[MAP_TILE_COUNT]) {
	assert(pos_in_range(camera_pos + VIEW_SIZE - Vector2{1, 1}));

	for (int y = camera_pos.y; y < camera_pos.y + VIEW_SIZE.y; y++) {
		for (int x = camera_pos.x; x < camera_pos.x + VIEW_SIZE.x; x++) {
			if (!visible || visible[pos_to_index({x, y})]) {
				printw("%s", *map.at({x, y}) == Tile::Wall ? "#" : ".");
			} else {
				printw(" ");
			}
		}
		printw("\n");
	}
}

