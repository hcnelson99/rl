#include <vector>
#include <unordered_set>

#include "ncurses_render.h"
#include "util.h"

WINDOW* ncurses_init_win() {
	WINDOW *win = initscr();
	noecho();
	curs_set(0);
	// halfdelay(1);
	// nodelay(win, true);
	keypad(win, true);
	return win;
}

void ncurses_render(const Map &map) {
	for (int y = 0; y < MAP_SIZE.y; y++) {
		for (int x = 0; x < MAP_SIZE.x; x++) {
			printw("%s", *map.at({x, y}) == Tile::Wall ? "#" : " ");
		}
		printw("\n");
	}

}

void ncurses_floodfill_render(const Map &map) {

	std::vector<std::unordered_set<Vector2>> regions = floodfill(map);

	for (int y = 0; y < MAP_SIZE.y; y++) {
		for (int x = 0; x < MAP_SIZE.x; x++) {
			Vector2 pos = {x, y};
			if (*map.at(pos) == Tile::Wall) {
				printw(".");

			} else {
				bool found = false;
				for (unsigned int i = 0; i < regions.size(); i++) {
					if (contains(regions[i], pos)) {
						printw("%c", 'A' + i);
						found = true;
						break;
					}
				}
				if (!found) {
					printw("@");
				}
			}
		}
		printw("\n");
	}

}

void ncurses_render(const Map &map, const Vector2 &camera_pos) {
	assert(pos_in_range(camera_pos + VIEW_SIZE - Vector2{1, 1}));

	for (int y = camera_pos.y; y < camera_pos.y + VIEW_SIZE.y; y++) {
		for (int x = camera_pos.x; x < camera_pos.x + VIEW_SIZE.x; x++) {
			printw("%s", *map.at({x, y}) == Tile::Wall ? "#" : " ");
		}
		printw("\n");
	}

}
void ncurses_render_visible(const Map &map, const Vector2 &camera_pos, bool visible[MAP_TILE_COUNT]) {
	assert(pos_in_range(camera_pos + VIEW_SIZE - Vector2{1, 1}));

	for (int y = camera_pos.y; y < camera_pos.y + VIEW_SIZE.y; y++) {
		for (int x = camera_pos.x; x < camera_pos.x + VIEW_SIZE.x; x++) {
			if (visible[pos_to_index({x, y})]) {
				printw("%s", *map.at({x, y}) == Tile::Wall ? "#" : ".");
			} else {
				printw(" ");
			}
		}
		printw("\n");
	}
}

