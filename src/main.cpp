#include <assert.h>
#include <thread>
#include <chrono>
#include <string.h>

#include "game.h"
#include "util.h"
#include "path_map.h"
#include "curses_render.h"

enum class MoveType {None, Wait, Step};
struct Move {
	MoveType type;
	union {
		struct {Vector2 step;};
	};
};

bool move_mob(const Map &map, Vector2 *mob_pos, const Move &move) {
	assert(mob_pos);

	if (move.type == MoveType::Step) {
		Vector2 new_mob_pos = *mob_pos + move.step;

		if (*map.at(new_mob_pos) == Tile::Floor) {
			*mob_pos = new_mob_pos;
			return true;
		}
	}
	return false;
}

void enemy_gen(Game &game) {
	pcg32_random_t mob_gen;
	seed_pcg32(&mob_gen, gen_seed());

	int floor_count = 0;
	for (int i = 0; i < MAP_TILE_COUNT; i++) {
		if (*game.map.at(i) == Tile::Floor) {
			floor_count++;
		}
	}

	for (int i = 0; i < 50; i++) {
		CMob enemy;
		enemy.type = MobType::Enemy;

		int r = pcg32_boundedrand_r(&mob_gen, floor_count);
		int fi = -1;
		for (int j = 0; j < MAP_TILE_COUNT; j++) {
			if (*game.map.at(j) == Tile::Floor) {
				fi++;
			}
			if (fi == r) {
				assert(*game.map.at(j) == Tile::Floor);
				enemy.pos = index_to_pos(j);
				break;
			}
		}
		game.mobs.add(game.new_entity(), enemy);
	}

}

const auto goal_frame_time = std::chrono::milliseconds(16);

int main() {
	init_log("rl.log");

	WINDOW *win = curses_init_win();
	defer(endwin());
	if (!win) {
		CRITICAL("Could not initialize curses window");
		return 1;
	}

	bool scrolling = true;
	bool render_visible = true;

	Game game;
	cave_map(&game.map);

	bool player_view_history[MAP_TILE_COUNT];
	memset(player_view_history, false, MAP_TILE_COUNT);

	{
		CMob mob_player;
		mob_player.type = MobType::Player;
		for (int i = 0; i < MAP_TILE_COUNT; i++) {
			if (*game.map.at(i) == Tile::Floor) {
				mob_player.pos = index_to_pos(i);
				break;
			}
		}
		EntityID player = game.new_entity();
		game.add_tag(player, EntityTag::Player);
		game.mobs.add(player, mob_player);
	}

	enemy_gen(game);

	bool redraw = true;

	while (true) {
		auto start_time = std::chrono::steady_clock::now();

		int c = getch();

		if (c == 'q') {
			break;
		}

		Move player_move;
		player_move.type = MoveType::None;

		switch (c) {
			case 'r':
				cave_map(&game.map);
				memset(player_view_history, false, MAP_TILE_COUNT);
				redraw = true;
				break;
			case 'w':
				player_move.type = MoveType::Step;
				player_move.step = Vector2{0, -1};
				break;
			case 'a':
				player_move.type = MoveType::Step;
				player_move.step = Vector2{-1, 0};
				break;
			case 's':
				player_move.type = MoveType::Step;
				player_move.step = Vector2{0, 1};
				break;
			case 'd':
				player_move.type = MoveType::Step;
				player_move.step = Vector2{1, 0};
				break;
			case '.':
				player_move.type = MoveType::Wait;
				break;
			case 'v':
				if (render_visible) {
					render_visible = false;
				} else {
					render_visible = true;
				}
				redraw = true;
				break;
			case 'c':
				if (scrolling) {
					scrolling = false;
				} else {
					scrolling = true;
				}
				redraw = true;
				break;
			case ' ':
				PathMap path_map;
				for (int i = 0; i < MAP_TILE_COUNT; i++) {
					if (!player_view_history[i]) {
						path_map.set_goal(i);
					}
				}
				path_map.smooth(game.map);

				int min = PATH_MAP_MAX;
				for (const Vector2 &o : ORTHOGONALS) {
					Vector2 pos = game.mobs.get(game.get_tagged(EntityTag::Player))->pos + o;
					if (*path_map.at(pos) < min) {
						player_move.type = MoveType::Step;
						player_move.step = o;
						min = *path_map.at(pos);
					}
				}

				break;
		}

		CMob *mob_player = game.mobs.get(game.get_tagged(EntityTag::Player));

		if (player_move.type == MoveType::Step) {
			// Refactor? Should be two steps? See if valid, if so move?
			bool successful_move = move_mob(game.map, &mob_player->pos, player_move);
			if (successful_move) {
				redraw = true;
			} else {
				// If move was invalid (into wall), set player_move to None so enemies do not move
				player_move.type = MoveType::None;
			}
		}

		if (player_move.type != MoveType::None) {
			PathMap enemy_path_map;
			enemy_path_map.set_goal(mob_player->pos);
			enemy_path_map.smooth(game.map);

			for (auto it = std::begin(game.mobs); it != std::end(game.mobs); it++) {
				if (it->second.type == MobType::Enemy) {
					auto enemy = &it->second;

					Move enemy_move;
					enemy_move.type = MoveType::Wait;

					int min = *enemy_path_map.at(enemy->pos);
					for (const Vector2 &o : ORTHOGONALS) {
						Vector2 pos = enemy->pos + o;
						if (*enemy_path_map.at(pos) < min) {
							enemy_move.type = MoveType::Step;
							enemy_move.step = o;
							min = *enemy_path_map.at(pos);
						}
					}

					move_mob(game.map, &enemy->pos, enemy_move);
					redraw = true;
				}
			}
		}

		if (redraw) {
			curses_render(game, player_view_history, scrolling, render_visible);
			redraw = false;
		}


		auto current_time = std::chrono::steady_clock::now();
		auto frame_time = current_time - start_time;
		auto spare_time = goal_frame_time - frame_time;

		if (spare_time < std::chrono::seconds(0)) {
			// WARNING("Missed frame time by %ldns", -std::chrono::duration_cast<std::chrono::nanoseconds>(spare_time).count());
		} else {
			std::this_thread::sleep_for(spare_time);
		}
		// auto end_time = std::chrono::steady_clock::now();
		// LOG("framerate: %f", 1E9 / std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count());
	}

	return 0;
}
