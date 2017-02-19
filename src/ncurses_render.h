#pragma once

#include "map.h"

void ncurses_render(const Map &map);
void ncurses_render(const Map &map, const Vector2 &camera_pos);
void ncurses_floodfill_render(const Map &map);
void ncurses_render_visible(const Map &map, const Vector2 &camera_pos, bool visible[MAP_TILE_COUNT]);
