#ifndef mocha_runtime_h
#define mocha_runtime_h

struct mocha_object;

typedef struct mocha_runtime {

} mocha_runtime;

void mocha_runtime_eval(mocha_runtime* self, const struct mocha_object* o);

#endif
