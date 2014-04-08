#include <mocha/parser.h>
#include <mocha/runtime.h>
#include <mocha/object.h>
#include <mocha/print.h>
#include <mocha/log.h>
#include <string.h>
#include <stdio.h>
#include <mocha/log.h>


int read_line(mocha_char* s, int max_length)
{
	const int c_max_length = 1024;
	if (max_length > c_max_length) {
		max_length = c_max_length;
	}
	char c_input[c_max_length];

	int input_length = 0;
	while (input_length < max_length) {
		int c_char = fgetc(stdin);
		if (c_char == 0) {
			return -1;
		}
		s[input_length] = c_char;
		++input_length;
		if (c_char == 10) {
			break;
		}
	}

	return input_length;
}

int main(int arc, char* argv[])
{
	mocha_parser parser;
	mocha_runtime runtime;
	mocha_runtime_init(&runtime);

	const int max_length = 1024;
	mocha_char input[max_length];
	mocha_error error;
	mocha_error_init(&error);

	while (1) {
		MOCHA_OUTPUT("repl=> ");
		int input_length = read_line(input, max_length);
		if (input_length == -1) {
			break;
		}

		mocha_parser_init(&parser, input, input_length);

		const mocha_object* o;
		do {
			o = mocha_parser_parse(&parser, &error);
			if (o) {
				const mocha_object* r = mocha_runtime_eval(&runtime, o, &error);
				if (r) {
					mocha_print_object_debug(r);
					MOCHA_OUTPUT(" ");
				}
			}
		} while (o != 0 && error.code == mocha_error_code_ok);
		MOCHA_LOG("");

		if (error.code != mocha_error_code_ok) {
			mocha_error_show(&error);
			mocha_error_init(&error);
		}
	}
}
