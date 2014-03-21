#ifndef mocha_object_h
#define mocha_object_h

#include "list.h"
#include "map.h"
#include "vector.h"
#include "string.h"
#include "types.h"

typedef enum mocha_object_type {
	mocha_object_type_list,
	mocha_object_type_map,
	mocha_object_type_vector,
	mocha_object_type_integer,
	mocha_object_type_float,
	mocha_object_type_string,
	mocha_object_type_keyword,
	mocha_object_type_boolean,
	mocha_object_type_symbol
} mocha_object_type;

struct mocha_list;
struct mocha_map;
struct mocha_vector;

typedef struct mocha_object {
	mocha_object_type type;
	union {
		struct mocha_list list;
		struct mocha_map map;
		struct mocha_vector vector;
		int i;
		float f;
		mocha_boolean b;
		const mocha_char* string;
	} data;
} mocha_object;

#endif
