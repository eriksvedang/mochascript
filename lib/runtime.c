#include "../include/runtime.h"
#include "../include/print.h"

void mocha_runtime_eval(mocha_runtime* self, const struct mocha_object* o)
{
	mocha_print_object_debug(o);
}