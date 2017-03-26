#include <vector>
#include <unordered_map>
#include <string.h>

#include "curses_render.h"
#include "map.h"
#include "util.h"

#define BLACK COLOR_PAIR(1)
#define LIGHT_BLACK COLOR_PAIR(9)
#define RED COLOR_PAIR(2)
#define LIGHT_RED COLOR_PAIR(10)
#define GREEN COLOR_PAIR(3)
#define LIGHT_GREEN COLOR_PAIR(11)
#define YELLOW COLOR_PAIR(4)
#define LIGHT_YELLOW COLOR_PAIR(12)
#define BLUE COLOR_PAIR(5)
#define LIGHT_BLUE COLOR_PAIR(13)
#define MAGENTA COLOR_PAIR(6)
#define LIGHT_MAGENTA COLOR_PAIR(14)
#define CYAN COLOR_PAIR(7)
#define LIGHT_CYAN COLOR_PAIR(15)
#define WHITE COLOR_PAIR(8)
#define LIGHT_WHITE 16

using namespace entityx;

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

// TODO: Factor out render information (if other graphical backends are ever written)
struct RenderInfo {
	char display;
	int color;
};

const std::unordered_map<MobType, RenderInfo> mob_render_info = {
		{MobType::Player, {'@', BLUE}},
		{MobType::Enemy, {'g', RED}},
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

void curses_render(Game &game, bool player_view_history[MAP_TILE_COUNT], bool scrolling, bool render_visible) {
	assert(player_view_history);

	Entity player = game.get_tagged(EntityTag::Player);

	Vector2 player_pos = player.component<CPos>()->pos;

	Camera camera;
	if (scrolling) {
		camera.view_size = {80, 24};
		camera.update_scroll(player_pos);
	} else {
		camera.view_size = MAP_SIZE;
		camera.pos = {0, 0};
	}

	bool visible[MAP_TILE_COUNT];
	memset(visible, false, MAP_TILE_COUNT);
	game.map.visibility(player_pos, visible);

	for (int i = 0; i < MAP_TILE_COUNT; i++) {
		if (visible[i]) {
			player_view_history[i] = visible[i];
		}
	}

	erase();

	for (int y = camera.pos.y; y < camera.pos.y + camera.view_size.y; y++) {
		for (int x = camera.pos.x; x < camera.pos.x + camera.view_size.x; x++) {
			if (render_visible) {
				if (visible[pos_to_index({x, y})]) {
					printw("%s", *game.map.at({x, y}) == Tile::Wall ? "#" : ".");
				} else if (player_view_history[pos_to_index({x, y})]) {
					attron(LIGHT_BLACK);
					printw("%s", *game.map.at({x, y}) == Tile::Wall ? "#" : ".");
					attroff(LIGHT_BLACK);
				} else {
					printw(" ");
				}
			} else {
				printw("%s", *game.map.at({x, y}) == Tile::Wall ? "#" : " ");
			}
		}
		printw("\n");
	}

	game.ecs.each<CMob, CPos>([&](const Entity e, CMob &mob, CPos &mob_pos) {
		LOG("test\n");
		Vector2 screen_location = mob_pos.pos;
		if (scrolling) {
			screen_location -= camera.pos;
		}

		if (screen_location.x >= 0 && screen_location.x < camera.view_size.x &&
				screen_location.y >= 0 && screen_location.y < camera.view_size.y &&
				(!render_visible || visible[pos_to_index(mob_pos.pos)])) {
			RenderInfo render_info = mob_render_info.at(mob.type);
			attron(render_info.color);
			mvprintw(screen_location.y, screen_location.x, &render_info.display);
			attroff(render_info.color);
		}
	});

	refresh();
}
