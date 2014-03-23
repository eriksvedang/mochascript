#ifndef mocha_map_h
#define mocha_map_h

#include <mocha/types.h>

struct mocha_object;

typedef struct mocha_map {
	const struct mocha_object** objects;
	size_t count;
} mocha_map;

void mocha_map_init(mocha_map* self, const struct mocha_object* args[], int count);

#endif
