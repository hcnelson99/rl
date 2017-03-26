#pragma once

#include <ncurses.h>

#include "game.h"

WINDOW* curses_init_win();
void curses_render(Game &game, bool player_view_history[MAP_TILE_COUNT], bool scrolling, bool render_visible);
