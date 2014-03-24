#include <mocha/parser.h>
#include <mocha/runtime.h>
#include <mocha/object.h>
#include <mocha/print.h>
#include <string.h>
#include <stdio.h>
#include <mocha/log.h>

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
	mocha_runtime_init(&runtime);
	if (o->type == mocha_object_type_vector) {
		for (size_t i = 0; i < o->data.vector.count; ++i) {
			const struct mocha_object* x = o->data.vector.objects[i];
			const mocha_object* r = mocha_runtime_eval(&runtime, x);
			mocha_print_object_debug(r);

		}
	}
}
