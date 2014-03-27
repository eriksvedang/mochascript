#include <mocha/runtime.h>
#include <mocha/print.h>
#include <mocha/object.h>
#include <mocha/type.h>
#include <mocha/log.h>

static const mocha_object* fn(mocha_runtime* self, mocha_context* context, const mocha_object* arguments, const mocha_object* body)
{
 	mocha_object* r = mocha_context_create_object(context);
 	static mocha_type fn_type;
 	fn_type.eval_all_arguments = mocha_true;
 	fn_type.invoke = 0;

 	r->type = mocha_object_type_function;
 	r->object_type = &fn_type;
 	r->data.function.arguments = arguments;
 	r->data.function.code = body;

 	return r;
 }

MOCHA_FUNCTION(fn_func)
{
	const mocha_object* r = fn(runtime, context, arguments->objects[0], arguments->objects[1]);
	return r;
}

static const mocha_object* def(mocha_runtime* runtime, mocha_context* context, const mocha_object* name, const mocha_object* body)
{
	const mocha_object* eval = mocha_runtime_eval(runtime, body);
	mocha_context_add(context, name, eval);
	return eval;
}


MOCHA_FUNCTION(def_func)
{
	const mocha_object* eval = def(runtime, context, arguments->objects[0], arguments->objects[1]);
	return eval;
}

MOCHA_FUNCTION(defn_func)
{
	const mocha_object* func = fn(runtime, context, arguments->objects[1], arguments->objects[2]);
	def(runtime, context, arguments->objects[0], func);
	return func;
}

MOCHA_FUNCTION(defmacro_func)
{
 	mocha_object* func = mocha_context_create_object(context);
 	static mocha_type macro_type;
 	macro_type.eval_all_arguments = mocha_false;
 	macro_type.is_macro = mocha_true;
 	macro_type.invoke = 0;

 	func->type = mocha_object_type_function;
 	func->object_type = &macro_type;
 	func->data.function.arguments = arguments->objects[1];
 	func->data.function.code = arguments->objects[2];

	def(runtime, context, arguments->objects[0], func);

	return func;
}

MOCHA_FUNCTION(if_func)
{
	const mocha_object* condition = mocha_runtime_eval(runtime, arguments->objects[0]);
	if (condition->type != mocha_object_type_boolean) {
		MOCHA_LOG("Illegal condition type");
		return condition;
	}
	mocha_boolean satisfied = condition->data.b;
	int eval_index = satisfied ? 1 : 2;
	if (eval_index >= arguments->count) {
		MOCHA_LOG("Missing argument!");
		return condition;
	}
	const mocha_object* result = mocha_runtime_eval(runtime, arguments->objects[eval_index]);

	return result;
}

static void number_mul(mocha_number* r, const mocha_number* a, const mocha_number* b)
{
	if (a->type == mocha_number_type_integer && b->type == mocha_number_type_integer) {
		r->type = mocha_number_type_integer;
		r->data.i = a->data.i * b->data.i;
	} else {
		r->type = mocha_number_type_float;
		if (a->type == mocha_number_type_integer) {
			r->data.f = (float) a->data.i * b->data.f;
		} else {
			r->data.f =  a->data.f * (float) b->data.i;
		}
	}
}

MOCHA_FUNCTION(mul_func)
{
	mocha_number result;
	result.type = mocha_number_type_integer;
	result.data.i = 1;

	for (size_t c = 0; c < arguments->count; ++c) {
		number_mul(&result, &result, &arguments->objects[c]->data.number);
	}

 	mocha_object* r = mocha_context_create_object(context);
 	r->type = mocha_object_type_number;
 	r->data.number = result;

	return r;
}

static void number_add(mocha_number* r, const mocha_number* a, const mocha_number* b)
{
	if (a->type == mocha_number_type_integer && b->type == mocha_number_type_integer) {
		r->type = mocha_number_type_integer;
		r->data.i = a->data.i + b->data.i;
	} else {
		r->data.f = mocha_number_float(a) + mocha_number_float(b);
		r->type = mocha_number_type_float;
	}
}

static void number_dec(mocha_number* r, const mocha_number* a, const mocha_number* b)
{
	if (a->type == mocha_number_type_integer && b->type == mocha_number_type_integer) {
		r->data.i = a->data.i - b->data.i;
		r->type = mocha_number_type_integer;
	} else {
		r->data.f = mocha_number_float(a) - mocha_number_float(b);
		r->type = mocha_number_type_float;
	}
}

static void number_div(mocha_number* r, const mocha_number* a, const mocha_number* b)
{
	if (a->type == mocha_number_type_integer && b->type == mocha_number_type_integer) {
		r->type = mocha_number_type_integer;
		r->data.i = a->data.i / b->data.i;
	} else {
		r->type = mocha_number_type_float;
		if (a->type == mocha_number_type_integer) {
			r->data.f = (float) a->data.i / b->data.f;
		} else {
			r->data.f =  a->data.f / (float) b->data.i;
		}
	}
}

MOCHA_FUNCTION(add_func)
{
 	mocha_object* r = mocha_context_create_object(context);
 	r->type = mocha_object_type_number;
 	r->data.number.data.i = 0;
 	r->data.number.type = mocha_number_type_integer;

	for (size_t c = 0; c < arguments->count; ++c) {
		number_add(&r->data.number, &r->data.number, &arguments->objects[c]->data.number);
	}


	return r;
}

MOCHA_FUNCTION(dec_func)
{
 	mocha_object* r = mocha_context_create_object(context);
 	r->type = mocha_object_type_number;
 	r->data.number.data.i = 0;

	int start_index = 0;
	if (arguments->count > 1) {
		start_index = 1;
		r->data.number = arguments->objects[0]->data.number;
	}

	for (size_t c = start_index; c < arguments->count; ++c) {
		number_dec(&r->data.number, &r->data.number, &arguments->objects[c]->data.number);
	}

	return r;
}

MOCHA_FUNCTION(div_func)
{
 	mocha_object* r = mocha_context_create_object(context);
 	r->type = mocha_object_type_number;

	r->data.number.data.i = 1;

	int start_index = 0;
	if (arguments->count > 1) {
		start_index = 1;
		r->data.number = arguments->objects[0]->data.number;
	}
	for (size_t c = start_index; c < arguments->count; ++c) {
		number_div(&r->data.number, &r->data.number, &arguments->objects[c]->data.number);
	}

	return r;
}

MOCHA_FUNCTION(equal_func)
{
 	mocha_object* r = mocha_context_create_object(context);
 	r->type = mocha_object_type_boolean;

	const mocha_object* source = arguments->objects[0];
	for (size_t i=1; i<arguments->count; ++i) {
		const mocha_object* v = arguments->objects[i];
		if (!mocha_object_equal(source, v)) {
		 	r->data.b = mocha_false;
			break;
		}
	}

	r->data.b = mocha_true;

	return r;
}

MOCHA_FUNCTION(quote_func)
{
	return arguments->objects[0];
}

MOCHA_FUNCTION(unquote_func)
{
	return mocha_runtime_eval(runtime, arguments->objects[0]);
}

static void bootstrap_context(mocha_context* context)
{
	static mocha_type def;
	def.invoke = def_func;
	def.eval_all_arguments = mocha_false;
	def.is_macro = mocha_false;
	mocha_context_add_function(context, "def", &def);

	static mocha_type defmacro_type;
	defmacro_type.invoke = defn_func;
	defmacro_type.eval_all_arguments = mocha_false;
	defmacro_type.is_macro = mocha_false;
	mocha_context_add_function(context, "defmacro", &defmacro_type);

	static mocha_type defn;
	defn.invoke = defn_func;
	defn.eval_all_arguments = mocha_false;
	defn.is_macro = mocha_false;
	mocha_context_add_function(context, "defn", &defn);

	static mocha_type mul;
	mul.invoke = mul_func;
	mul.eval_all_arguments = mocha_true;
	mul.is_macro = mocha_false;
	mocha_context_add_function(context, "*", &mul);

	static mocha_type add;
	add.invoke = add_func;
	add.eval_all_arguments = mocha_true;
	add.is_macro = mocha_false;
	mocha_context_add_function(context, "+", &add);

	static mocha_type dec;
	dec.invoke = dec_func;
	dec.eval_all_arguments = mocha_true;
	dec.is_macro = mocha_false;
	mocha_context_add_function(context, "-", &dec);

	static mocha_type div;
	div.invoke = div_func;
	div.eval_all_arguments = mocha_true;
	div.is_macro = mocha_false;
	mocha_context_add_function(context, "/", &div);


	static mocha_type fn;
	fn.invoke = fn_func;
	fn.eval_all_arguments = mocha_false;
	fn.is_macro = mocha_false;
	mocha_context_add_function(context, "fn", &fn);

	static mocha_type if_type;
	if_type.invoke = if_func;
	if_type.eval_all_arguments = mocha_false;
	if_type.is_macro = mocha_false;
	mocha_context_add_function(context, "if", &if_type);

	static mocha_type equal_type;
	equal_type.invoke = equal_func;
	equal_type.eval_all_arguments = mocha_true;
	equal_type.is_macro = mocha_false;
	mocha_context_add_function(context, "=", &equal_type);


	static mocha_type quote_type;
	quote_type.invoke = quote_func;
	quote_type.eval_all_arguments = mocha_false;
	quote_type.is_macro = mocha_false;
	mocha_context_add_function(context, "quote", &quote_type);

	static mocha_type unquote_type;
	unquote_type.invoke = unquote_func;
	unquote_type.eval_all_arguments = mocha_false;
	unquote_type.is_macro = mocha_false;
	mocha_context_add_function(context, "unquote", &unquote_type);
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
		if (l->count - 1 != args->count) {
			MOCHA_LOG("Illegal number of arguments: %d", (int)l->count - 1);
			return fn;
		}
		for (size_t arg_count = 0; arg_count < args->count; ++arg_count) {
			const mocha_object* arg = args->objects[arg_count];
			if (arg->type != mocha_object_type_symbol) {
				MOCHA_LOG("Must use symbols!");
				return 0;
			}
			mocha_context_add(context, arg, l->objects[arg_count + 1]);
		}
		o = mocha_runtime_eval(self, fn->data.function.code);
		if (fn->object_type->is_macro) {
			MOCHA_LOG("MACRO!");
			o = mocha_runtime_eval(self, o);
		}
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
		if (!fn) {
			MOCHA_LOG("Couldn't find lookup");
			return 0;
		}
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
