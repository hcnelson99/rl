#pragma once

#include <ncurses.h>

#include "map.h"

WINDOW* ncurses_init_win();
void ncurses_render(const Map &map);
void ncurses_render(const Map &map, const Vector2 &camera_pos);
void ncurses_floodfill_render(const Map &map);
void ncurses_render_visible(const Map &map, const Vector2 &camera_pos, bool visible[MAP_TILE_COUNT]);
