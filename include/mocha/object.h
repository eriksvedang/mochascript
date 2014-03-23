#ifndef mocha_object_h
#define mocha_object_h

#include <mocha/list.h>
#include <mocha/map.h>
#include <mocha/vector.h>
#include <mocha/string.h>
#include <mocha/symbol.h>
#include <mocha/keyword.h>
#include <mocha/types.h>

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
		mocha_list list;
		mocha_map map;
		mocha_vector vector;
		int i;
		float f;
		mocha_boolean b;
		mocha_string string;
		mocha_symbol symbol;
		mocha_keyword keyword;
	} data;
} mocha_object;

#endif
