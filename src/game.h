#pragma once

#include <unordered_map>

#include "util.h"
#include "map.h"

typedef uint32_t EntityID;

enum class EntityTag : int {Player};

enum class MobType {Player, Enemy};
struct CMob {
	MobType type;
};


template <typename T>
class ComponentStore {
	virtual T *get(EntityID e) = 0;
	virtual const T *get(EntityID e) const = 0;
	virtual void add(EntityID e, const T &name) = 0;
};

template <typename T>
class MapStore : ComponentStore<T> {

	std::unordered_map<EntityID, T> map;

public:

	MapStore() : map() {}

	typename std::unordered_map<EntityID, T>::const_iterator begin() const {
		return map.begin();
	}

	typename std::unordered_map<EntityID, T>::const_iterator end() const {
		return map.end();
	}

	typename std::unordered_map<EntityID, T>::iterator begin() {
		return map.begin();
	}

	typename std::unordered_map<EntityID, T>::iterator end() {
		return map.end();
	}

	T *get(EntityID e) {
		auto lookup = map.find(e);
		if (lookup == map.end()) {
			return nullptr;
		}
		return &lookup->second;
	}

	const T *get(EntityID e) const {
		auto lookup = map.find(e);
		if (lookup == map.end()) {
			return nullptr;
		}
		return &lookup->second;
	}

	void add(EntityID e, const T &name) {
		if (! map.insert({e, name}).second) {
			CRITICAL("Entity %u already has this component", e);
			assert(false);
		}
	}
};


template <typename A, typename B, typename F>
void component_map(MapStore<A> &m1, MapStore<B> &m2, F/* std::function<void(EntityID, A&, B&)>& */ f) {
	for (auto it = m1.begin(); it != m1.end(); it++) {
		B *b = m2.get(it->first);
		if (b) {
			f(it->first, it->second, *b);
		}
	}
}

template <typename A, typename B, typename F>
void component_map(const MapStore<A> &m1, const MapStore<B> &m2, F/* std::function<void(EntityID, A&, B&)>& */ f) {
	for (auto it = m1.begin(); it != m1.end(); it++) {
		const B *b = m2.get(it->first);
		if (b) {
			f(it->first, it->second, *b);
		}
	}
}

struct Game {
	EntityID next_entity = 0;
	std::unordered_map<EntityTag, EntityID> tags;

	MapStore<CMob> mob;
	MapStore<Vector2> pos;

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
};

