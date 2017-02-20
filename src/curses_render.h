#pragma once

#include <ncurses.h>

#include "map.h"

WINDOW* curses_init_win();
void curses_render(const Map &map);
void curses_render(const Map &map, const Vector2 &camera_pos);
void curses_floodfill_render(const Map &map);
void curses_render_visible(const Map &map, const Vector2 &camera_pos, bool visible[MAP_TILE_COUNT]);
