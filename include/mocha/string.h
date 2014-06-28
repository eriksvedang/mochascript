#ifndef mocha_string_h
#define mocha_string_h

#include <mocha/types.h>

typedef int mocha_char;
#include <string.h>
#define mocha_strchr strchr

typedef struct mocha_string {
	mocha_char* string;
	size_t count;
} mocha_string;

const char* mocha_string_to_c(const mocha_string* s);
mocha_boolean mocha_string_equal_str(const mocha_string* self, const char* cstr);
void mocha_string_init(mocha_string* self, const mocha_char* s, size_t count);
void mocha_string_init_from_c(mocha_string* self, const char* s);
mocha_boolean mocha_string_equal(const mocha_string* a, const mocha_string* b);
mocha_boolean mocha_string_less(const mocha_string* a, const mocha_string* b);

#endif
