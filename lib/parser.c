#include <mocha/parser.h>
#include <mocha/object.h>
#include <mocha/log.h>
#include <mocha/context.h>
#include <mocha/symbol.h>
#include <mocha/string.h>

#include <stdlib.h>

typedef struct mocha_parse_error {
	int code;
} mocha_parse_error;

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
	return mocha_strchr("(){}[]\'`\"", ch) != 0;
}

static mocha_parse_error get_word(mocha_parser* self)
{
	mocha_char* o = self->word_buffer.string;
	size_t o_count = 0;
	const mocha_char* p = self->input;
	mocha_parse_error e;

	p = skip_space(p);
	if (!p) {
		*o = 0;
		e.code = -42;
		return e;
	}

	mocha_char ch = *p++;
	o[o_count++] = ch;
	if (is_separator(ch)) {
		self->input = p;
		o[o_count] = 0;
		self->word_buffer.count = o_count;
		e.code = 0;
		return e;
	}

	for (;;) {
		ch = *p++;
		if (ch == 0) {
			self->input = p;
			o[o_count] = 0;
			self->word_buffer.count = o_count;
			e.code = -42;
			return e;
		}
		if (is_separator(ch) || is_space((ch))) {
			self->input = p - 1;
			o[o_count] = 0;
			self->word_buffer.count = o_count;
			e.code = 0;
			return e;
		}
		o[o_count++] = ch;
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


static const mocha_object* parse_object(mocha_parser* self, mocha_parse_error* error);

static int parse_array(mocha_parser* self, mocha_parse_error* error, const mocha_object* array[], size_t array_max_count)
{
	int count = 0;

	while (count < array_max_count) {
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
	int count = parse_array(self, error, args, 128);
	mocha_object* o = mocha_context_create_object(&self->context);
	mocha_map_init(&o->data.map, args, count);
	o->type = mocha_object_type_map;

	return o;
}

static mocha_object* parse_vector(mocha_parser* self, mocha_parse_error* error)
{
	const mocha_object* args[128];
	int count = parse_array(self, error, args, 128);
	mocha_object* o = mocha_context_create_object(&self->context);
	mocha_vector_init(&o->data.vector, args, count);
	o->type = mocha_object_type_vector;
	return o;
}

static mocha_object* parse_list(mocha_parser* self, mocha_parse_error* error)
{
	const mocha_object* args[128];
	int count = parse_array(self, error, args, 128);

	mocha_object* o = mocha_context_create_object(&self->context);
	mocha_list_init(&o->data.list, args, count);
	o->type = mocha_object_type_list;
	return o;
}



static mocha_object* parse_number(mocha_parser* self, mocha_parse_error* error)
{
	mocha_object* o = mocha_context_create_object(&self->context);
	o->type = mocha_object_type_number;
	const char* s = mocha_string_to_c(&self->word_buffer);
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
	while ((ch = *p++) != '\"') {
		temp[count++] = ch;
	}
	temp[count] = 0;
	mocha_string_init(&o->data.string, temp, count);
	self->input = p;
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
	const mocha_object* symbol = create_symbol(&self->context, &self->word_buffer);
	return symbol;
}

static mocha_object* parse_keyword(mocha_parser* self, mocha_parse_error* error)
{
	mocha_string* temp = malloc(sizeof(mocha_string));
	mocha_string_init(temp, self->word_buffer.string + 1, self->word_buffer.count - 1);
	mocha_object* o = mocha_context_create_object(&self->context);
	mocha_keyword_init(&o->data.keyword, temp);
	o->type = mocha_object_type_keyword;
	return o;
}

static const mocha_object* parse_literal_or_symbol(mocha_parser* self, mocha_parse_error* error)
{
	const mocha_object* o;

	mocha_char first_char = self->word_buffer.string[0];
	if (mocha_string_equal_str(&self->word_buffer, "true")) {
		mocha_object* boolean_object = mocha_context_create_object(&self->context);
		boolean_object->type = mocha_object_type_boolean;
		boolean_object->data.b = 1;
		o = boolean_object;
	} else if (mocha_string_equal_str(&self->word_buffer, "false")) {
		mocha_object* boolean_object = mocha_context_create_object(&self->context);
		boolean_object->type = mocha_object_type_boolean;
		boolean_object->data.b = 0;
		o = boolean_object;
	} else if (mocha_string_equal_str(&self->word_buffer, "nil")) {
		mocha_object* nil_object = mocha_context_create_object(&self->context);
		nil_object->type = mocha_object_type_nil;
		o = nil_object;
	} else if (is_numerical(first_char)) {
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

	*error = get_word(self);
	mocha_char first_char = self->word_buffer.string[0];
	size_t word_len = self->word_buffer.count;

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
			case '\'':
				o = parse_tick(self, error);
				break;
			default:
				o = parse_literal_or_symbol(self, error);
		}
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
	self->word_buffer.string = malloc(1024);
	self->word_buffer.count = 0;

	mocha_parse_error error;
	const mocha_object* o = parse_list(self, &error);
	return o;
}
