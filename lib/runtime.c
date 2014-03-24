#include <mocha/runtime.h>
#include <mocha/print.h>
#include <mocha/object.h>
#include <mocha/type.h>
#include <mocha/log.h>

MOCHA_FUNCTION(def_func)
{
	mocha_object arguments_object;
	arguments_object.data.list = *arguments;
	arguments_object.type = mocha_object_type_list;
	const mocha_object* eval = mocha_runtime_eval(runtime, arguments->objects[1]);
	mocha_context_add(context, arguments->objects[0], eval);
	return eval;
}

MOCHA_FUNCTION(mul_func)
{
	int a = arguments->objects[0]->data.i;
	int b = arguments->objects[1]->data.i;

 	mocha_object* r = mocha_context_create_object(context);

 	r->type = mocha_object_type_integer;
 	r->data.i = a * b;

	return r;
}

static void bootstrap_context(mocha_context* context)
{
	static mocha_type def;
	def.invoke = def_func;
	mocha_context_add_function(context, "def", &def);

	static mocha_type mul;
	mul.invoke = mul_func;
	mocha_context_add_function(context, "*", &mul);
}

static const mocha_object* invoke(mocha_runtime* self, mocha_context* context, const mocha_object* fn, const mocha_list* l)
{
	mocha_list arguments_list;
	arguments_list.objects = &l->objects[1];
	arguments_list.count = l->count - 1;

	const mocha_object* o = 0;
	if (fn->type == mocha_object_type_internal_function) {
		o = fn->object_type->invoke(self, context, &arguments_list);
	}

	return o;
}

void mocha_runtime_init(mocha_runtime* self)
{
	mocha_context_init(&self->main_context);
	bootstrap_context(&self->main_context);
}

const struct mocha_object* mocha_runtime_eval(mocha_runtime* self, const struct mocha_object* o)
{
	if (o->type == mocha_object_type_list) {
		const mocha_list* l = &o->data.list;
		const struct mocha_object* fn = mocha_context_lookup(&self->main_context, l->objects[0]);
		o = invoke(self, &self->main_context, fn, l);
	} else {
		if (o->type == mocha_object_type_symbol) {
			o = mocha_context_lookup(&self->main_context, o);
		}
	}

	return o;
}
