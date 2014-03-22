#include "../include/print.h"
#include "../include/object.h"
#include "../include/log.h"
#include "../include/string.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct string_stream {
	char* p;
	char* buffer;
	size_t buffer_size;
} string_stream;

void string_stream_init(string_stream* self, size_t buf_size)
{
	self->buffer_size = buf_size;
	self->buffer = malloc(sizeof(char) * buf_size);
	self->p = self->buffer;
}

void string_stream_output(string_stream* self, const char* buf)
{
	size_t length = strlen(buf);
	if ((self->p - self->buffer) + length >= self->buffer_size) {
		MOCHA_LOG("BAD!");
		return;
	}
	memcpy(self->p, buf, sizeof(char) * length);
	self->p += length;
}

void string_stream_close(string_stream* self)
{
	*self->p = 0;
}

void print_object_debug(string_stream* f, const mocha_object* o);

void print_array_debug(string_stream* f, const mocha_object* objects[], size_t count)
{
	for (int i=0; i<count; ++i) {
		const mocha_object* o = objects[i];
		print_object_debug(f, o);
		if (i != count - 1) {
			string_stream_output(f, " ");
		}
	}
}


void print_object_debug(string_stream* f, const mocha_object* o)
{
	char buf[256];

	switch (o->type) {
		case mocha_object_type_list:
			string_stream_output(f, "(");
			print_array_debug(f, o->data.list.objects, o->data.list.count);
			string_stream_output(f, ")");
			break;
		case mocha_object_type_vector:
			string_stream_output(f, "[");
			print_array_debug(f, o->data.vector.objects, o->data.vector.count);
			string_stream_output(f, "]");
			break;
		case mocha_object_type_map:
			string_stream_output(f, "{");
			print_array_debug(f, o->data.map.objects, o->data.map.count);
			string_stream_output(f, "}");
			break;
		case mocha_object_type_integer:
			snprintf(buf, 256, "%d", o->data.i);
			string_stream_output(f, buf);
			break;
		case mocha_object_type_float:
			snprintf(buf, 256, "%f", o->data.f);
			string_stream_output(f, buf);
			break;
		case mocha_object_type_string:
			snprintf(buf, 256, "\"%s\"", mocha_string_to_c(&o->data.string));
			string_stream_output(f, buf);
			break;
		case mocha_object_type_keyword:
			string_stream_output(f, "Keyword ");
			break;
		case mocha_object_type_boolean:
			string_stream_output(f, o->data.b ? "true" : "false");
			break;
		case mocha_object_type_symbol:
			string_stream_output(f, "Symbol");
			break;
	}
}

void mocha_print_object_debug(const mocha_object* o)
{
	string_stream stream;
	string_stream_init(&stream, 256 * 1024);
	print_object_debug(&stream, o);
	string_stream_close(&stream);

	MOCHA_LOG("output:'%s'", stream.buffer);
}