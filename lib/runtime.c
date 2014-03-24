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
	int a = 1;
	for (size_t c = 0; c < arguments->count; ++c) {
		a *= arguments->objects[c]->data.i;
	}

 	mocha_object* r = mocha_context_create_object(context);

 	r->type = mocha_object_type_integer;
 	r->data.i = a;

	return r;
}

MOCHA_FUNCTION(fn_func)
{
 	mocha_object* r = mocha_context_create_object(context);
 	static mocha_type fn_type;
 	fn_type.eval_all_arguments = mocha_true;
 	fn_type.invoke = 0;

 	r->type = mocha_object_type_function;
 	r->object_type = &fn_type;
 	r->data.function.arguments = arguments->objects[0];
 	r->data.function.code = arguments->objects[1];
	return r;
}

static void bootstrap_context(mocha_context* context)
{
	static mocha_type def;
	def.invoke = def_func;
	def.eval_all_arguments = mocha_false;
	mocha_context_add_function(context, "def", &def);

	static mocha_type mul;
	mul.invoke = mul_func;
	mul.eval_all_arguments = mocha_true;
	mocha_context_add_function(context, "*", &mul);

	static mocha_type fn;
	fn.invoke = fn_func;
	fn.eval_all_arguments = mocha_false;
	mocha_context_add_function(context, "fn", &fn);

}

static const mocha_object* invoke(mocha_runtime* self, mocha_context* context, const mocha_object* fn, const mocha_list* l)
{
	mocha_list arguments_list;
	arguments_list.objects = &l->objects[1];
	arguments_list.count = l->count - 1;

	const mocha_object* o = 0;
	if (fn->type == mocha_object_type_internal_function) {
		o = fn->object_type->invoke(self, context, &arguments_list);
	} else if (fn->type == mocha_object_type_function) {
		const mocha_list* args = &fn->data.function.arguments->data.list;
		for (size_t arg_count = 0; arg_count < args->count; ++arg_count) {
			const mocha_object* arg = args->objects[arg_count];
			if (arg->type != mocha_object_type_symbol) {
				MOCHA_LOG("Must use symbols!");
				return 0;
			}
			mocha_context_add(context, arg, l->objects[arg_count + 1]);
		}
		o = mocha_runtime_eval(self, fn->data.function.code);
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
		mocha_list new_args;
		if (fn->object_type->eval_all_arguments) {
			const mocha_object* converted_args[32];
			for (size_t i=1; i < l->count; ++i) {
				converted_args[i] = mocha_runtime_eval(self, l->objects[i]);
			}
			mocha_list_init(&new_args, converted_args, l->count);
			l = &new_args;
		}
		o = invoke(self, &self->main_context, fn, l);
	} else {
		if (o->type == mocha_object_type_symbol) {
			o = mocha_context_lookup(&self->main_context, o);
		}
	}

	return o;
}
