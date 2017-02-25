#pragma once

#include <unordered_map>

#include "util.h"
#include "map.h"

typedef uint32_t EntityID;

enum class EntityTag {Player};

enum class MobType {Player, Enemy};
struct CMob {
	MobType type;
	Vector2 pos;
};

struct Game {
	EntityID next_entity = 0;
	std::unordered_map<EntityTag, EntityID> tags;
	std::unordered_map<EntityID, CMob> mobs;

	Map map;

	EntityID new_entity() {
		return next_entity++;
	}

	void add_tag(EntityID e, EntityTag tag) {
		if (!tags.insert({tag, e}).second) {
			CRITICAL("Entity %u already has tag %d", e, tag);
			assert(false);
		}
	}

	EntityID get_tagged(EntityTag tag) const {
		auto lookup = tags.find(tag);
		if (lookup == tags.end()) {
			CRITICAL("Could not find entity with given tag");
			assert(false);
		}
		return lookup->second;
	}

	const CMob *get_cmob(EntityID e) const {
		auto lookup = mobs.find(e);
		if (lookup == mobs.end()) {
			CRITICAL("Could not find CMob for EntityID %u", e);
			assert(false);
		}
		return &lookup->second;
	}

	CMob *get_cmob(EntityID e) {
		auto lookup = mobs.find(e);
		if (lookup == mobs.end()) {
			CRITICAL("Could not find CMob for EntityID %u", e);
			assert(false);
		}
		return &lookup->second;
	}

	void add_cmob(EntityID e, const CMob &cmob) {
		if (!mobs.insert({e, cmob}).second) {
			CRITICAL("Entity %u already has CMob", e);
			assert(false);
		}
	}
};

