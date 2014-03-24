#ifndef mocha_runtime_h
#define mocha_runtime_h

#include <mocha/context.h>

struct mocha_object;

typedef struct mocha_runtime {
	mocha_context main_context;
} mocha_runtime;

void mocha_runtime_init(mocha_runtime* self);
const struct mocha_object*  mocha_runtime_eval(mocha_runtime* self, const struct mocha_object* o);

#endif
