#pragma once

#include <ncurses.h>

#include "map.h"

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

WINDOW* curses_init_win();
void curses_render(const Map &map, const Vector2 &player_pos, const Vector2 &enemy_pos, bool history[MAP_TILE_COUNT], bool visibility, bool scrolling);
