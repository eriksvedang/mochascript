#ifndef mocha_parser_h
#define mocha_parser_h

#include <stddef.h>

#include "context.h"
#include "string.h"

typedef int mocha_boolean;

typedef struct mocha_parser {
	const mocha_char* input;
	mocha_char input_buffer[128];
	mocha_char word_buffer[128];
	mocha_context context;
} mocha_parser;

void mocha_parser_parse(mocha_parser* self, const mocha_char* input, size_t input_length);

#endif
