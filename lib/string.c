#include "../include/string.h"

mocha_boolean mocha_string_equal_str(const mocha_char* self, const char* cstr)
{
	const mocha_char* p = self;
	const char* s = cstr;
	mocha_char ch;

	while ((ch = *p++) != 0) {
		if (*s == 0) {
			return mocha_false;
		}
		if (ch != *s++) {
			return mocha_false;
		}
	}

	return mocha_true;
}

