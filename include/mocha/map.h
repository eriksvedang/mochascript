#ifndef mocha_map_h
#define mocha_map_h

#include <mocha/types.h>

struct mocha_object;

typedef struct mocha_map {
	const struct mocha_object** objects;
	size_t count;
} mocha_map;

void mocha_map_init(mocha_map* self, const struct mocha_object* args[], int count);
const struct mocha_object* mocha_map_lookup(const mocha_map* self, const struct mocha_object* key);
mocha_boolean mocha_map_equal(const mocha_map* self, const mocha_map* other);

#endif
