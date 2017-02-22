#pragma once

#include <unordered_map>

#include "map.h"

typedef uint32_t MobID;
enum class MobType {Player, Enemy};
struct Mob {
	MobType type;
	Vector2 pos;
};

struct Game {
	Map map;
	MobID next_mob = 0;
	std::unordered_map<MobID, Mob> mobs;

	MobID new_mob(const Mob &mob) {
		MobID id = next_mob;
		mobs[id] = mob;
		next_mob++;
		return id;
	}
};

