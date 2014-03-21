#ifndef mocha_string_h
#define mocha_string_h

#include "types.h"

typedef int mocha_char;
#include <string.h>
#define mocha_strchr strchr

mocha_boolean mocha_string_equal_str(const mocha_char* self, const char* cstr);

#endif
