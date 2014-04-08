#include <mocha/parser.h>
#include <mocha/object.h>
#include <mocha/log.h>
#include <mocha/context.h>
#include <mocha/symbol.h>
#include <mocha/string.h>

#include <stdlib.h>

#define MOCHA_ERR(err_number, err_string) add_error(err_number, err_string); return 0;

typedef struct mocha_parse_error {
	int code;
} mocha_parse_error;

static void add_error(int err_number, const char* err_string)
{
	MOCHA_LOG("%d: '%s'", err_number, err_string);
}

static mocha_boolean is_space(mocha_char ch)
{
	return mocha_strchr("\t\n\r, ", ch) != 0;
}

static mocha_boolean is_alpha(mocha_char ch)
{
	return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
}

static mocha_boolean is_numerical(mocha_char ch)
{
	return (ch >= '0' && ch <= '9');
}


static mocha_char read_char(mocha_parser* self)
{
	if (self->input > self->input_end) {
		MOCHA_LOG("ERROR: You read too far!");
		return -1;
	}
	mocha_char ch = *self->input++;

	return ch;
}

static void unread_char(mocha_parser* self, mocha_char c)
{
	if (self->input == self->input_buffer) {
		MOCHA_LOG("ERROR: You unread too far!");
	}
	self->input--;
	if (c != *self->input) {
		MOCHA_LOG("ERROR: You unread illegal char!");
	}
}


static mocha_char skip_space(mocha_parser* self)
{
	int ch;

	do {
		ch = read_char(self);
	} while (ch != 0 && is_space(ch));

	return ch;
}

static mocha_boolean is_separator(mocha_char ch)
{
	return mocha_strchr("(){}[]\'`\"", ch) != 0;
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


static const mocha_object* parse_object(mocha_parser* self, mocha_parse_error* error);

static int parse_array(mocha_parser* self, mocha_char end_char, mocha_parse_error* error, const mocha_object* array[], size_t array_max_count)
{
	int count = 0;

	while (count < array_max_count) {
		mocha_char ch = skip_space(self);
		if (!ch) {
			error->code = -23;
			MOCHA_ERR(-23, "Unexpected end");
		} else if (ch == end_char) {
			return count;
		}
		unread_char(self, ch);
		const mocha_object* o = parse_object(self, error);
		if (!o || error->code != 0) {
			return count;
		}
		array[count++] = o;
	}

	error->code = 1;
	return count;
}


static mocha_object* parse_map(mocha_parser* self, mocha_parse_error* error)
{
	const mocha_object* args[128];
	int count = parse_array(self, '}', error, args, 128);
	mocha_object* o = mocha_context_create_object(&self->context);
	mocha_map_init(&o->data.map, args, count);
	o->type = mocha_object_type_map;

	return o;
}

static mocha_object* parse_vector(mocha_parser* self, mocha_parse_error* error)
{
	const mocha_object* args[128];
	int count = parse_array(self, ']', error, args, 128);
	mocha_object* o = mocha_context_create_object(&self->context);
	mocha_vector_init(&o->data.vector, args, count);
	o->type = mocha_object_type_vector;
	return o;
}

static mocha_object* parse_list(mocha_parser* self, mocha_parse_error* error)
{
	const mocha_object* args[128];
	int count = parse_array(self, ')', error, args, 128);
	if (error->code != 0) {
		return 0;
	}

	mocha_object* o = mocha_context_create_object(&self->context);
	mocha_list_init(&o->data.list, args, count);
	o->type = mocha_object_type_list;
	return o;
}

static int parse_word(mocha_parser* self, mocha_char* char_buffer, int max_symbol_length)
{
	mocha_string word_buffer;

	word_buffer.string = char_buffer;
	word_buffer.count = 0;

	while (1) {
		mocha_char ch = read_char(self);
		if (is_space(ch) || is_separator(ch) || !ch) {
			if (is_separator(ch)) {
				unread_char(self, ch);
			}
			break;
		}
		if (word_buffer.count == max_symbol_length) {
			MOCHA_ERR(9, "Too long symbol");
		}
		word_buffer.string[word_buffer.count++] = ch;
	}

	return word_buffer.count;
}

static mocha_object* parse_number(mocha_parser* self, mocha_parse_error* error)
{
	const int max_symbol_length = 64;
	static mocha_char char_buffer[max_symbol_length];
	int length = parse_word(self, char_buffer, max_symbol_length);

	mocha_string word_buffer;
	word_buffer.string = char_buffer;
	word_buffer.count = length;

	mocha_object* o = mocha_context_create_object(&self->context);
	o->type = mocha_object_type_number;
	const char* s = mocha_string_to_c(&word_buffer);
	mocha_boolean is_floating_point = mocha_strchr(s, '.') != 0;
	if (is_floating_point) {
		o->data.number.data.f = atof(s);
		o->data.number.type = mocha_number_type_float;
	} else {
		o->data.number.data.i = atol(s);
		o->data.number.type = mocha_number_type_integer;
	}
	return o;
}

static mocha_object* parse_string(mocha_parser* self, mocha_parse_error* error)
{
	mocha_object* o = mocha_context_create_object(&self->context);
	o->type = mocha_object_type_string;
	const mocha_char* p = self->input;
	mocha_char temp[1024];
	size_t count = 0;
	mocha_char ch;
	read_char(self);
	while ((ch = read_char(self)) != '"') {
		if (!ch) {
			MOCHA_ERR(1, "Missing end \" for string");
		}
		temp[count++] = ch;
	}
	temp[count] = 0;
	mocha_string_init(&o->data.string, temp, count);
	return o;
}

static const mocha_object* create_symbol(mocha_context* context, const mocha_string* string)
{
	mocha_string* temp = malloc(sizeof(mocha_string));
	mocha_string_init(temp, string->string, string->count);
	mocha_object* o = mocha_context_create_object(context);
	mocha_symbol_init(&o->data.symbol, temp);
	o->type = mocha_object_type_symbol;

	return o;
}

static const mocha_object* parse_symbol(mocha_parser* self, mocha_parse_error* error)
{
	const mocha_object* o;

	const int max_symbol_length = 64;
	static mocha_char char_buffer[max_symbol_length];
	int length = parse_word(self, char_buffer, max_symbol_length);

	mocha_string word_buffer;
	word_buffer.string = char_buffer;
	word_buffer.count = length;

	if (mocha_string_equal_str(&word_buffer, "true")) {
		mocha_object* boolean_object = mocha_context_create_object(&self->context);
		boolean_object->type = mocha_object_type_boolean;
		boolean_object->data.b = 1;
		o = boolean_object;
	} else if (mocha_string_equal_str(&word_buffer, "false")) {
		mocha_object* boolean_object = mocha_context_create_object(&self->context);
		boolean_object->type = mocha_object_type_boolean;
		boolean_object->data.b = 0;
		o = boolean_object;
	} else if (mocha_string_equal_str(&word_buffer, "nil")) {
		mocha_object* nil_object = mocha_context_create_object(&self->context);
		nil_object->type = mocha_object_type_nil;
		o = nil_object;
	} else {
		o = create_symbol(&self->context, &word_buffer);
	}
	return o;
}

static mocha_object* parse_keyword(mocha_parser* self, mocha_parse_error* error)
{
	const int max_symbol_length = 64;
	static mocha_char char_buffer[max_symbol_length];
	int length = parse_word(self, char_buffer, max_symbol_length);

	mocha_string* word_buffer = malloc(sizeof(mocha_string));
	word_buffer->string = char_buffer;
	word_buffer->count = length;

	mocha_object* o = mocha_context_create_object(&self->context);
	mocha_keyword_init(&o->data.keyword, word_buffer);
	o->type = mocha_object_type_keyword;
	return o;
}

static const mocha_object* parse_literal_or_symbol(mocha_parser* self, mocha_parse_error* error, mocha_char first_char)
{
	const mocha_object* o;

	if (is_numerical(first_char)) {
		o = parse_number(self, error);
	} else if (first_char == '"') {
		o = parse_string(self, error);
	} else {
		o = parse_symbol(self, error);
	}


	return o;
}

static mocha_object* parse_tick(mocha_parser* self, mocha_parse_error* error)
{
	const mocha_object* do_not_eval = parse_object(self, error);

	mocha_string temp_string;
	mocha_string_init_from_c(&temp_string, "quote");
	const mocha_object* quote_symbol = create_symbol(&self->context, &temp_string);

	const mocha_object* args[2];

	args[0] = quote_symbol;
	args[1] = do_not_eval;

	mocha_object* l = mocha_context_create_object(&self->context);
	mocha_list_init(&l->data.list, args, 2);
	l->type = mocha_object_type_list;

	return l;
}

static const mocha_object* parse_object(mocha_parser* self, mocha_parse_error* error)
{
	const mocha_object* o = 0;

	mocha_char first_char = skip_space(self);
	switch (first_char) {
		case 0:
			o = 0;
			break;
		case ':':
			o = parse_keyword(self, error);
			break;
		case '{':
			o = parse_map(self, error);
			break;
		case '[':
			o = parse_vector(self, error);
			break;
		case '(':
			o = parse_list(self, error);
			break;
		case '\'':
			o = parse_tick(self, error);
			break;
		default:
			unread_char(self, first_char);
			o = parse_literal_or_symbol(self, error, first_char);
	}

	return o;
}

const mocha_object* mocha_parser_parse(mocha_parser* self, const mocha_char* input, size_t input_length)
{
	for (size_t i=0; i<input_length; ++i) {
		self->input_buffer[i] = input[i];
	}
	self->input_buffer[input_length] = 0;

	self->input = self->input_buffer;
	self->input_end = self->input + input_length;

	mocha_parse_error error;
	const mocha_object* o = parse_object(self, &error);
	return o;
}
