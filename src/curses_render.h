#pragma once

#include <ncurses.h>

#include "map.h"

WINDOW* curses_init_win();
void curses_render(const Map &map, const Vector2 &player_pos, const Vector2 &enemy_pos, bool history[MAP_TILE_COUNT], bool visibility, bool scrolling);
