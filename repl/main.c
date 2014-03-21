#include "../include/parser.h"
#include <string.h>
#include <stdio.h>

int main(int arc, char* argv[])
{
	mocha_parser parser;

	mocha_char input[128];
	const char* c_input = " {:test ( + 42 99 true [\"hello, world\" symbol_one another_symbol] )}";
	size_t input_length = strlen(c_input);
	for (size_t i = 0; i<input_length; ++i) {
		input[i] = c_input[i];
	}

	mocha_parser_parse(&parser, input, input_length);
}
