#include <mocha/list.h>
#include <mocha/object.h>
#include <mocha/log.h>
#include <stdlib.h>

void mocha_list_init(mocha_list* self, const mocha_object** args, int count)
{
	size_t octet_count = sizeof(mocha_object*) * count;
	self->objects = malloc(octet_count);
	self->count = count;
	memcpy(self->objects, args, octet_count);
}
