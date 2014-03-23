#ifndef mocha_symbol_h
#define mocha_symbol_h

#include <mocha/string.h>

typedef struct mocha_symbol {
	const mocha_string* string;
} mocha_symbol;

void mocha_symbol_init(mocha_symbol* self, const mocha_string* string);

#endif
