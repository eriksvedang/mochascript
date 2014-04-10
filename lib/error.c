#include <mocha/error.h>
#include <mocha/log.h>

static const char* error_string(mocha_error_code code)
{
	const char* s;

	switch (code)
	{
		case mocha_error_code_unexpected_end:
			s = "Unexpected end";
			break;
		case mocha_error_code_ok:
			s = "No error";
			break;
		case mocha_error_code_symbol_too_long:
			s = "Symbol is too long";
			break;
		case mocha_error_code_missing_end_of_string:
			s = "Missing end of string";
			break;
		case mocha_error_code_unexpected_character:
			s = "Unexpected character";
			break;
		case mocha_error_code_file_not_found:
			s = "File not found";
			break;
	}

	return s;
}

void mocha_error_show(mocha_error* self)
{
	MOCHA_LOG("Error %d '%s'", self->code, error_string(self->code));
}

void mocha_error_init(mocha_error* self)
{
	self->code = mocha_error_code_ok;
}
