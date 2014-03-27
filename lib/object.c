#include <mocha/object.h>

mocha_boolean mocha_object_equal(const mocha_object* a, const mocha_object* b)
{
	if (a->type != b->type) {
		return mocha_false;
	}

	switch (a->type)
	{
		case mocha_object_type_number:
			return mocha_number_equal(&a->data.number, &b->data.number);
		case mocha_object_type_string:
			return mocha_string_equal(&a->data.string, &b->data.string);
		case mocha_object_type_keyword:
			return mocha_false;
		case mocha_object_type_boolean:
			return (a->data.b == b->data.b);
		case mocha_object_type_symbol:
			return mocha_false;
		default:
			return mocha_false;
	}
}
