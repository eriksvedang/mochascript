#include "../include/map.h"
#include "../include/object.h"
#include "../include/log.h"

#include <stdlib.h>

void mocha_map_init(mocha_map* self, const mocha_object* args[], int count)
{
	if ((count % 2) != 0) {
		MOCHA_LOG("ERROR: Must have even numbers in map!");
		self->count = 0;
		return;
	}

	size_t octet_count = sizeof(mocha_object*) * count;
	self->objects = malloc(octet_count);
	self->count = count;
	memcpy(self->objects, args, octet_count);
}
