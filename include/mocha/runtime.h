#ifndef mocha_runtime_h
#define mocha_runtime_h

#include <mocha/context.h>

struct mocha_object;
struct mocha_error;

typedef struct mocha_runtime {
	mocha_context* context;
	mocha_context* contexts;
	int stack_depth;
	const struct mocha_object* nil;
} mocha_runtime;

void mocha_runtime_init(mocha_runtime* self);
mocha_context* mocha_runtime_create_context(mocha_runtime* self);
void mocha_runtime_pop_context(mocha_runtime* self);
void mocha_runtime_push_context(mocha_runtime* self, mocha_context* context);
const struct mocha_object*  mocha_runtime_eval(mocha_runtime* self, const struct mocha_object* o, struct mocha_error* error);

#endif
