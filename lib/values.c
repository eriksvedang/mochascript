#include <mocha/values.h>
#include <mocha/object.h>
#include <mocha/type.h>

#include <stdlib.h>

mocha_object* mocha_values_create_object(mocha_values* self, mocha_object_type object_type)
{
	mocha_object* o = malloc(sizeof(mocha_object));
	o->type = object_type;
	o->object_type = 0;

	return o;
}

const mocha_object* mocha_values_create_boolean(mocha_values* self, mocha_boolean value)
{
	mocha_object* o = mocha_values_create_object(self, mocha_object_type_boolean);
	o->data.b = value;
	return o;
}

const mocha_object* mocha_values_create_internal_function(mocha_values* self, const struct mocha_type* type, const char* name)
{
	mocha_object* value = mocha_values_create_object(self, mocha_object_type_internal_function);
	value->object_type = type;
	value->debug_string = name;

	return value;
}

const mocha_object* mocha_values_create_symbol(mocha_values* self, const mocha_string* string)
{
	mocha_object* value = mocha_values_create_object(self, mocha_object_type_symbol);
	mocha_string* copy = malloc(sizeof(mocha_string));

	mocha_string_init(copy, string->string, string->count);
	value->data.symbol.string = copy;
	return value;
}

const mocha_object* mocha_values_create_map(mocha_values* self, const mocha_object** args, int count)
{
	mocha_object* value = mocha_values_create_object(self, mocha_object_type_map);
	mocha_map_init(&value->data.map, args, count);

	return value;
}

const mocha_object* mocha_values_create_vector(mocha_values* self, const mocha_object** args, int count)
{
	mocha_object* value = mocha_values_create_object(self, mocha_object_type_vector);
	mocha_vector_init(&value->data.vector, args, count);

	return value;
}

const mocha_object* mocha_values_create_list(mocha_values* self, const mocha_object** args, int count)
{
	mocha_object* value = mocha_values_create_object(self, mocha_object_type_list);
	mocha_list_init(&value->data.list, args, count);

	return value;
}

const mocha_object* mocha_values_create_float(mocha_values* self, float v)
{
	mocha_object* value = mocha_values_create_object(self, mocha_object_type_number);
	value->data.number.type = mocha_number_type_float;
	value->data.number.data.f = v;

	return value;
}

const mocha_object* mocha_values_create_integer(mocha_values* self, int v)
{
	mocha_object* value = mocha_values_create_object(self, mocha_object_type_number);
	value->data.number.type = mocha_number_type_integer;
	value->data.number.data.i = v;

	return value;
}

const mocha_object* mocha_values_create_string_from_cstr(mocha_values* self, const char* s)
{
	mocha_object* value = mocha_values_create_object(self, mocha_object_type_string);
	mocha_string_init_from_c(&value->data.string, s);

	return value;
}

const struct mocha_object* mocha_values_create_string(mocha_values* self, const mocha_char* s, int count)
{
	mocha_object* value = mocha_values_create_object(self, mocha_object_type_string);
	mocha_string_init(&value->data.string, s, count);

	return value;
}

const mocha_object* mocha_values_create_function(mocha_values* self, const struct mocha_context* context, const mocha_object* arguments, const mocha_object* body)
{
	static mocha_type fn_type;
	fn_type.eval_all_arguments = mocha_true;
	fn_type.invoke = 0;

	mocha_object* r = mocha_values_create_object(self, mocha_object_type_function);
	r->object_type = &fn_type;
	r->data.function.arguments = arguments;
	r->data.function.context = context;
	r->data.function.code = body;

	return r;
}

const struct mocha_object* mocha_values_create_nil(mocha_values* self)
{
	mocha_object* value = mocha_values_create_object(self, mocha_object_type_nil);
	return value;
}


const struct mocha_object* mocha_values_create_number(mocha_values* self, mocha_number number)
{
	mocha_object* value = mocha_values_create_object(self, mocha_object_type_number);
	value->data.number = number;

	return value;
}

const struct mocha_object* mocha_values_create_keyword(mocha_values* self, const mocha_char* s, int count)
{
	mocha_object* value = mocha_values_create_object(self, mocha_object_type_keyword);
	mocha_string* string = malloc(sizeof(mocha_string));
	mocha_string_init(string, s, count);
	mocha_keyword_init(&value->data.keyword, string);
	return value;
}