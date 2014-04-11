#include <mocha/context.h>
#include <mocha/object.h>
#include <mocha/log.h>
#include <mocha/print.h>

#include <stdlib.h>

mocha_object* mocha_context_create_object(mocha_context* self)
{
	mocha_object* o = malloc(sizeof(mocha_object));
	o->object_type = 0;

	return o;
}

void mocha_context_add(mocha_context* self, const mocha_object* key, const mocha_object* value)
{
	self->objects[self->count] = key;
	self->objects[self->count + 1] = value;
	self->count += 2;
}

void mocha_context_add_function(mocha_context* self, const char* name, const struct mocha_type* type)
{
	mocha_object* value = mocha_context_create_object(self);
	value->type = mocha_object_type_internal_function;
	value->object_type = type;
	value->debug_string = name;

	mocha_object* key = mocha_context_create_object(self);
	mocha_char buf[80];
	size_t len = strlen(name);
	for (size_t i=0; i<len; ++i) {
		buf[i] = name[i];
	}
	buf[len] = 0;

	mocha_string* key_string = malloc(sizeof(mocha_string));
	mocha_string_init(key_string, buf, len);
	key->data.symbol.string = key_string;
	key->type = mocha_object_type_symbol;
	mocha_context_add(self, key, value);
}

const mocha_object* mocha_context_lookup(const mocha_context* self, const mocha_object* o)
{
	for (size_t i=0; i<self->count; i+=2) {
		const mocha_object* key = self->objects[i];
		if (key->type == mocha_object_type_symbol && o->type == mocha_object_type_symbol) {
			if (mocha_string_equal(o->data.symbol.string, key->data.symbol.string)) {
				return self->objects[i+1];
			}
		}
	}

	if (self->parent) {
		return mocha_context_lookup(self->parent, o);
	}
	return 0;
}

void mocha_context_init(mocha_context* self, const mocha_context* parent)
{
	self->parent = parent;
	self->objects = malloc(sizeof(mocha_object*) * 1024);
	self->count = 0;
}
