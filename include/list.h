#ifndef mocha_list_h
#define mocha_list_h

#include "types.h"

struct mocha_object;

typedef struct mocha_list {
	const struct mocha_object** objects;
	size_t count;
} mocha_list;

void mocha_list_init(mocha_list* self, const struct mocha_object* args[], int count);

#endif
