#include "../include/parser.h"
#include "../include/runtime.h"
#include <string.h>
#include <stdio.h>
#include "../include/log.h"

int main(int arc, char* argv[])
{
	mocha_parser parser;

	const int max_length = 1024;
	mocha_char input[max_length];
	char c_input[max_length];

	int input_length = fread(c_input, 1, max_length, stdin);
	c_input[input_length] = 0;

	for (size_t i = 0; i<input_length; ++i) {
		input[i] = c_input[i];
	}

	struct mocha_object* o = mocha_parser_parse(&parser, input, input_length);

	mocha_runtime runtime;
	mocha_runtime_eval(&runtime, o);
}
