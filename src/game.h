#pragma once

#include "entityx/entityx.h"

#include "util.h"
#include "map.h"

typedef uint32_t EntityID;

enum class EntityTag : int {Player};

enum class MobType {Player, Enemy};
struct CMob {
	MobType type;
};

struct CPos {
	CPos(Vector2 pos) : pos(pos) {}
	Vector2 pos;
};

struct Game {
	Game() : ecs(events) {}
	std::unordered_map<EntityTag, entityx::Entity> tags;

	entityx::EventManager events;
	entityx::EntityManager ecs;

	Map map;
	void add_tag(entityx::Entity e, EntityTag tag) {
		if (!tags.insert({tag, e}).second) {
			assert(false);
		}
	}

	entityx::Entity get_tagged(EntityTag tag) const {
		auto lookup = tags.find(tag);
		if (lookup == tags.end()) {
			CRITICAL("Could not find entity with given tag");
			assert(false);
		}
		return lookup->second;
	}
};

