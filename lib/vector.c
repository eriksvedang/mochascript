#include "../include/vector.h"
#include "../include/object.h"
#include <stdlib.h>

void mocha_vector_init(mocha_vector* self, const struct mocha_object* args[], int count)
{
	size_t octet_count = sizeof(mocha_object*) * count;
	self->objects = malloc(octet_count);
	self->count = count;
	memcpy(self->objects, args, octet_count);
}
