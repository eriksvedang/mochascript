#ifndef mocha_function_h
#define mocha_function_h

#include <mocha/types.h>

struct mocha_object;

typedef struct mocha_function {
	const struct mocha_object* arguments;
	const struct mocha_object* code;
} mocha_function;

#endif
