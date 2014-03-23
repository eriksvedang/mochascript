#include <mocha/context.h>
#include <mocha/object.h>

#include <stdlib.h>

mocha_object* mocha_context_create_object(mocha_context* self)
{
	mocha_object* o = malloc(sizeof(mocha_object));

	return o;
}
