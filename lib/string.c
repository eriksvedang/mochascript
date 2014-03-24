#include <mocha/string.h>
#include <stdlib.h>

mocha_boolean mocha_string_equal_str(const mocha_string* self, const char* cstr)
{
	const mocha_char* p = self->string;
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

mocha_boolean mocha_string_equal(const mocha_string* a, const mocha_string* b)
{
	if (a->count != b->count) {
		return mocha_false;
	}

	for (size_t i=0; i<a->count; ++i) {
		if (a->string[i] != b->string[i]) {
			return mocha_false;
		}
	}

	return mocha_true;
}

const char* mocha_string_to_c(const mocha_string* s)
{
	static char temp[512];
	size_t len = s->count;
	for (size_t i=0; i<len; ++i) {
		temp[i] = s->string[i];
	}
	temp[len] = 0;
	return temp;
}


void mocha_string_init(mocha_string* self, const mocha_char* s, size_t count)
{
	size_t octet_count = sizeof(mocha_char) * count + 1;
	self->string = malloc(octet_count);
	self->count = count;
	memcpy(self->string, s, octet_count);
}
