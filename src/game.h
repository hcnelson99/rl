#pragma once

#include <unordered_map>

#include "util.h"
#include "map.h"

typedef uint32_t EntityID;

enum class EntityTag : int {Player};

enum class MobType {Player, Enemy};
struct CMob {
	MobType type;
	Vector2 pos;
};


#define add_component(type, name) \
	std::unordered_map<EntityID, type > CONCAT(name, s) ; \
	const type * CONCAT(get_, name) (EntityID e) const { \
		auto lookup = CONCAT(name, s) .find(e); \
		if (lookup == CONCAT(name, s) .end()) { \
			CRITICAL("Could not find %s for EntityID %u",#name, e); \
			assert(false); \
		} \
		return &lookup->second; \
	} \
 \
	type * CONCAT(get_, name) (EntityID e) { \
		auto lookup = CONCAT(name, s) .find(e); \
		if (lookup == CONCAT(name, s) .end()) { \
			CRITICAL("Could not find %s for EntityID %u",#name, e); \
			assert(false); \
		} \
		return &lookup->second; \
	} \
 \
	void CONCAT(add_, name) (EntityID e, const type & name) { \
		if (! CONCAT(name, s) .insert({e, name}).second) { \
			CRITICAL("Entity %u already has %s", e, #name); \
			assert(false); \
		} \
	} \



struct Game {
	EntityID next_entity = 0;
	std::unordered_map<EntityTag, EntityID> tags;

	Map map;

	EntityID new_entity() {
		return next_entity++;
	}

	void add_tag(EntityID e, EntityTag tag) {
		if (!tags.insert({tag, e}).second) {
			CRITICAL("Entity %u already has tag %d", e, static_cast<int>(tag));
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

	add_component(CMob, mob)
};

