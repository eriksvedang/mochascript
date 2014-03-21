#include "../include/parser.h"
#include "../include/object.h"
#include "../include/log.h"
#include "../include/context.h"

#include "../include/string.h"


typedef struct mocha_parse_error {
	int code;
} mocha_parse_error;



static mocha_boolean is_space(mocha_char ch)
{
	return mocha_strchr("\t\n\r, ", ch) != 0;
}

static const mocha_char* skip_space(const mocha_char* p)
{
	int ch;

	do {
		ch = *p++;
	} while (is_space(ch) && ch != 0);

	if (ch == 0) {
		return 0;
	}

	return p - 1;
}

static mocha_boolean is_separator(mocha_char ch)
{
	return mocha_strchr("(){}[]\'\"", ch) != 0;
}

static mocha_parse_error get_word(mocha_parser* self)
{
	mocha_char* o = self->word_buffer;
	const mocha_char* p = self->input;
	mocha_parse_error e;

	p = skip_space(p);
	if (!p) {
		*o = 0;
		e.code = -42;
		return e;
	}

	mocha_char ch = *p++;
	*o++ = ch;
	if (is_separator(ch)) {
		self->input = p;
		*o = 0;
		e.code = 0;
		return e;
	}

	for (;;) {
		ch = *p++;
		if (ch == 0) {
			self->input = p;
			*o = 0;
			e.code = -42;
			return e;
		}
		if (is_separator(ch) || is_space((ch))) {
			self->input = p - 1;
			*o = 0;
			e.code = 0;
			return e;
		}
		*o++ = ch;
	}
}

size_t mocha_string_length(const mocha_char* s)
{
	const mocha_char* p = s;
	size_t count = 0;
	while (!*p++ == 0) {
		count++;
	}
	return count;
}

const char* mocha_string_to_c(const mocha_char* s)
{
	static char temp[128];
	size_t len = mocha_string_length(s);
	for (size_t i=0; i<len; ++i) {
		temp[i] = s[i];
	}
	temp[len] = 0;
	return temp;
}

static mocha_object* parse_object(mocha_parser* self, mocha_parse_error* error);

static int parse_array(mocha_parser* self, mocha_parse_error* error, mocha_object* array[], size_t array_max_count)
{
	int count = 0;

	while (count < array_max_count) {
		mocha_object* o = parse_object(self, error);
		if (!o || error->code != 0) {
			MOCHA_LOG("Parsed array %d count", count);
			return count;
		}
		array[count++] = o;
	}

	error->code = 1;
	return count;
}


static mocha_object* parse_map(mocha_parser* self, mocha_parse_error* error)
{
	MOCHA_LOG("parse map");
	mocha_object* args[128];
	int count = parse_array(self, error, args, 128);
	mocha_object* o = mocha_context_create_object(&self->context);
	mocha_map_init(&o->data.map, args, count);
	o->type = mocha_object_type_map;

	return o;
}

static mocha_object* parse_vector(mocha_parser* self, mocha_parse_error* error)
{
	MOCHA_LOG("parse vector");
	mocha_object* args[128];
	int count = parse_array(self, error, args, 128);
	mocha_object* o = mocha_context_create_object(&self->context);
	mocha_map_init(&o->data.map, args, count);
	o->type = mocha_object_type_map;
	return o;
}

static mocha_object* parse_list(mocha_parser* self, mocha_parse_error* error)
{
	MOCHA_LOG("parse list");
	mocha_object* args[128];
	int count = parse_array(self, error, args, 128);
	mocha_object* o = mocha_context_create_object(&self->context);
	mocha_map_init(&o->data.map, args, count);
	o->type = mocha_object_type_map;
	return o;
}

static mocha_object* parse_keyword(mocha_parser* self, mocha_parse_error* error)
{
	MOCHA_LOG("parse keyword");
	mocha_object* o = mocha_context_create_object(&self->context);
	o->type = mocha_object_type_keyword;
	return o;
}

static mocha_object* parse_literal_or_symbol(mocha_parser* self, mocha_parse_error* error)
{
	MOCHA_LOG("parse literal or symbol");
	mocha_object* o = mocha_context_create_object(&self->context);
	o->type = mocha_object_type_map;
	return o;
}

static mocha_object* parse_object(mocha_parser* self, mocha_parse_error* error)
{
	mocha_object* o = 0;

	*error = get_word(self);
	MOCHA_LOG("'%s'", mocha_string_to_c(self->word_buffer));
	mocha_char first_char = self->word_buffer[0];
	size_t word_len = mocha_string_length(self->word_buffer);

	if (first_char == ':') {
		o = parse_keyword(self, error);
	} else {
		switch (first_char) {
			case ')':
			case '}':
			case ']':
				return 0;

			case '{':
				o = parse_map(self, error);
				break;
			case '[':
				o = parse_vector(self, error);
				break;
			case '(':
				o = parse_list(self, error);
				break;
			default:
				o = parse_literal_or_symbol(self, error);
		}
	}

	return o;
}

void mocha_parser_parse(mocha_parser* self, const mocha_char* input, size_t input_length)
{
	for (size_t i=0; i<input_length; ++i) {
		self->input_buffer[i] = input[i];
	}
	self->input_buffer[input_length] = ' ';
	self->input_buffer[input_length + 1] = 0;

	self->input = self->input_buffer;

	for (;;) {
		mocha_parse_error error;
		error.code = 0;
		parse_object(self, &error);
		if (error.code == -42) {
			return;
		}
	}
}
