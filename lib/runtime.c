#include <mocha/runtime.h>
#include <mocha/print.h>

void mocha_runtime_eval(mocha_runtime* self, const struct mocha_object* o)
{
	mocha_print_object_debug(o);
}