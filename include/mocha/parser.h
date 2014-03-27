#ifndef mocha_parser_h
#define mocha_parser_h

#include <stddef.h>

#include <mocha/context.h>
#include <mocha/string.h>

typedef struct mocha_parser {
	const mocha_char* input;
	mocha_char input_buffer[128];
	mocha_string word_buffer;
	mocha_context context;
} mocha_parser;

const struct mocha_object* mocha_parser_parse(mocha_parser* self, const mocha_char* input, size_t input_length);

#endif
