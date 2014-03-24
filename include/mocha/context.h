#ifndef mocha_context_h
#define mocha_context_h

#include <mocha/types.h>

struct mocha_object;
struct mocha_type;

typedef struct mocha_context {
	const struct mocha_object** objects;
	size_t count;
} mocha_context;

const struct mocha_object* mocha_context_lookup(mocha_context* self, const struct mocha_object* o);

void mocha_context_add(mocha_context* self, const struct mocha_object* name, const struct mocha_object* value);
void mocha_context_add_function(mocha_context* self, const char* name, const struct mocha_type* type);
void mocha_context_init(mocha_context* self);
struct mocha_object* mocha_context_create_object(mocha_context* self);

#endif
