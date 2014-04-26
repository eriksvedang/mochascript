#include <mocha/object.h>
#include <mocha/log.h>
#include <mocha/print.h>

mocha_boolean mocha_object_equal(const mocha_object* a, const mocha_object* b)
{
	if (a->type != b->type) {
		return mocha_false;
	}

	switch (a->type) {
		case mocha_object_type_number:
			return mocha_number_equal(&a->data.number, &b->data.number);
		case mocha_object_type_string:
			return mocha_string_equal(&a->data.string, &b->data.string);
		case mocha_object_type_keyword:
			return mocha_string_equal(a->data.keyword.string, b->data.keyword.string);
		case mocha_object_type_boolean:
			return (a->data.b == b->data.b);
		case mocha_object_type_symbol:
			return mocha_false;
		default:
			return mocha_false;
	}
}

mocha_boolean mocha_object_boolean(const mocha_object* a)
{
	if (a->type == mocha_object_type_boolean) {
		return a->data.b;
	} else {
		MOCHA_LOG("ERROR!!!!!!!!");
		return mocha_false;
	}
}

mocha_boolean mocha_object_truthy(const mocha_object* a)
{
	if (a->type == mocha_object_type_nil) {
		return mocha_false;
	} else if (a->type == mocha_object_type_boolean) {
		return a->data.b;
	} else {
		return mocha_true;
	}
}
