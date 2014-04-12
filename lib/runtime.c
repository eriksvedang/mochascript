#include <mocha/runtime.h>
#include <mocha/print.h>
#include <mocha/object.h>
#include <mocha/type.h>
#include <mocha/log.h>
#include <mocha/error.h>
#include <stdlib.h>


static const mocha_object* fn(mocha_runtime* self, mocha_context* context, const mocha_object* arguments, const mocha_object* body)
{
	mocha_object* r = mocha_context_create_object(context);
	static mocha_type fn_type;
	fn_type.eval_all_arguments = mocha_true;
	fn_type.invoke = 0;

	r->type = mocha_object_type_function;
	r->object_type = &fn_type;
	r->data.function.arguments = arguments;
	r->data.function.context = self->context;
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
	mocha_error error;
	const mocha_object* eval = mocha_runtime_eval(runtime, body, &error);
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
	mocha_error error;
	const mocha_object* condition = mocha_runtime_eval(runtime, arguments->objects[0], &error);
	if (condition->type != mocha_object_type_boolean) {
		MOCHA_LOG("Illegal condition type");
		return condition;
	}
	mocha_boolean satisfied = condition->data.b;
	int eval_index = satisfied ? 1 : 2;
	if (eval_index >= arguments->count) {
		mocha_object* r = mocha_context_create_object(context);
		r->type = mocha_object_type_nil;
		return r;
	}
	const mocha_object* result = mocha_runtime_eval(runtime, arguments->objects[eval_index], &error);

	return result;
}

MOCHA_FUNCTION(let_func)
{
	mocha_error error;
	const mocha_object* assignments = mocha_runtime_eval(runtime, arguments->objects[0], &error);
	if (!assignments || assignments->type != mocha_object_type_vector) {
		MOCHA_LOG("must have vector in let!");
		return 0;
	}

	const mocha_vector* assignment_vector = &assignments->data.vector;
	if ((assignment_vector->count % 2) != 0) {
		MOCHA_LOG("Wrong number of assignments");
		return 0;
	}

	mocha_context* new_context = mocha_runtime_create_context(runtime);
	for (size_t i=0; i<assignment_vector->count; i+=2) {
		const mocha_object* symbol = assignment_vector->objects[i];
		if (symbol->type != mocha_object_type_symbol) {
			MOCHA_LOG("must have symbol in let");
		}

		mocha_context_add(new_context, symbol, assignment_vector->objects[i+1]);
	}

	const mocha_object* result = mocha_runtime_eval(runtime, arguments->objects[1], &error);

	mocha_runtime_pop_context(runtime);

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
	r->data.b = mocha_true;

	const mocha_object* source = arguments->objects[0];
	for (size_t i=1; i<arguments->count; ++i) {
		const mocha_object* v = arguments->objects[i];
		if (!mocha_object_equal(source, v)) {
			r->data.b = mocha_false;
			break;
		}
	}

	return r;
}

MOCHA_FUNCTION(case_func)
{
	mocha_error error;
	const mocha_object* compare_value = mocha_runtime_eval(runtime, arguments->objects[0], &error);
	for (size_t i = 1; i < arguments->count; i += 2) {
		const mocha_object* when_value = arguments->objects[i];
		if (mocha_object_equal(compare_value, when_value)) {
			const mocha_object* when_argument = mocha_runtime_eval(runtime, arguments->objects[i+1], &error);
			return when_argument;
		}
	}

	if ((arguments->count % 2) == 0) {
		const mocha_object* default_value = mocha_runtime_eval(runtime, arguments->objects[arguments->count-1], &error);
		return default_value;
	}

	return runtime->nil;
}

static const mocha_object* conj_map(mocha_context* context, const mocha_map* self, const mocha_map* arg)
{
	mocha_object* new_map = mocha_context_create_object(context);
	new_map->type = mocha_object_type_map;
	const mocha_object* result[128];
	memcpy(result, self->objects, sizeof(mocha_object*) * self->count);
	memcpy(result + self->count, arg->objects, sizeof(mocha_object*) * arg->count);
	size_t total_count = self->count + arg->count;
	mocha_map_init(&new_map->data.map, result, total_count);

	return new_map;
}

static const mocha_object* conj_vector(mocha_context* context, const mocha_vector* self, const mocha_object** args, size_t count)
{
	mocha_object* new_vector = mocha_context_create_object(context);
	new_vector->type = mocha_object_type_vector;
	const mocha_object* result[128];
	memcpy(result, self->objects, sizeof(mocha_object*) * self->count);
	memcpy(result + self->count, args, sizeof(mocha_object*) * count);
	size_t total_count = self->count + count;
	mocha_vector_init(&new_vector->data.vector, result, total_count);

	return new_vector;
}

static const mocha_object* conj_list(mocha_context* context, const mocha_list* self, const mocha_object** args, size_t count)
{
	mocha_object* new_list = mocha_context_create_object(context);
	new_list->type = mocha_object_type_list;
	const mocha_object* result[128];

	for (size_t i = 0; i < count; ++i) {
		result[(count - i) - 1] = args[i];
	}

	size_t total_count = count;
	if (self) {
		memcpy(result + count, self->objects, sizeof(mocha_object*) * self->count);
		total_count += self->count;
	}
	mocha_list_init(&new_list->data.list, result, total_count);

	return new_list;
}

MOCHA_FUNCTION(conj_func)
{
	const mocha_object* sequence = arguments->objects[0];
	const mocha_object* result;
	switch (sequence->type) {
		case mocha_object_type_list:
			result = conj_list(context, &arguments->objects[0]->data.list, &arguments->objects[1], arguments->count-1);
			break;
		case mocha_object_type_vector:
			result = conj_vector(context, &arguments->objects[0]->data.vector, &arguments->objects[1], arguments->count-1);
			break;
		case mocha_object_type_nil:
			result = conj_list(context, 0, &arguments->objects[1], arguments->count-1);
			break;
		case mocha_object_type_map:
			result = conj_map(context, &arguments->objects[0]->data.map, &arguments->objects[1]->data.map);
			break;
		default:
			break;
	}

	return result;
}

static const mocha_object* cons_vector(mocha_context* context, const mocha_vector* self, const mocha_object** args)
{
	const int count = 1;
	mocha_object* new_vector = mocha_context_create_object(context);
	new_vector->type = mocha_object_type_list;
	const mocha_object* result[128];
	memcpy(result, args, sizeof(mocha_object*) * count);
	memcpy(result + count, self->objects, sizeof(mocha_object*) * self->count);
	size_t total_count = self->count + count;
	mocha_list_init(&new_vector->data.list, result, total_count);

	return new_vector;
}

static const mocha_object* cons_list(mocha_context* context, const mocha_list* self, const mocha_object** args)
{
	mocha_object* new_vector = mocha_context_create_object(context);
	new_vector->type = mocha_object_type_list;
	const mocha_object* result[128];
	const int count = 1;
	memcpy(result, args, sizeof(mocha_object*) * count);
	memcpy(result + count, self->objects, sizeof(mocha_object*) * self->count);
	size_t total_count = self->count + count;
	mocha_list_init(&new_vector->data.list, result, total_count);

	return new_vector;
}


MOCHA_FUNCTION(cons_func)
{
	const mocha_object* sequence = arguments->objects[1];
	const mocha_object* result;
	switch (sequence->type) {
		case mocha_object_type_list:
			result = cons_list(context, &sequence->data.list, &arguments->objects[0]);
			break;
		case mocha_object_type_vector:
			result = cons_vector(context, &sequence->data.vector, &arguments->objects[0]);
			break;
		case mocha_object_type_nil: {
			mocha_object* new_vector = mocha_context_create_object(context);
			new_vector->type = mocha_object_type_list;
			mocha_list_init(&new_vector->data.list, &arguments->objects[0], 1);
			result = new_vector;
		}
		break;
		case mocha_object_type_map:
			MOCHA_LOG("BAD MAP");
			result = 0;
			break;
		default:
			break;
	}

	return result;
}


static const mocha_object* rest_vector(mocha_context* context, const mocha_vector* self)
{
	mocha_object* new_vector = mocha_context_create_object(context);
	if (self->count > 0) {
		new_vector->type = mocha_object_type_vector;
		const mocha_object* result[128];
		memcpy(result, self->objects + 1, sizeof(mocha_object*) * (self->count - 1));
		size_t total_count = self->count - 1;
		mocha_vector_init(&new_vector->data.vector, result, total_count);
	} else {
		new_vector->type = mocha_object_type_nil;
	}

	return new_vector;
}



MOCHA_FUNCTION(rest_func)
{
	const mocha_object* sequence = arguments->objects[0];
	const mocha_object* result;
	switch (sequence->type) {
		case mocha_object_type_list:
			result = 0;
			break;
		case mocha_object_type_vector:
			result = rest_vector(context, &sequence->data.vector);
			break;
		case mocha_object_type_nil:
			result = 0;
			break;
		case mocha_object_type_map:
			result = 0;
			break;
		default:
			break;
	}

	return result;
}

static const mocha_object* first_vector(mocha_context* context, const mocha_vector* self)
{
	return self->objects[0];
}



MOCHA_FUNCTION(first_func)
{
	const mocha_object* sequence = arguments->objects[0];
	const mocha_object* result;
	switch (sequence->type) {
		case mocha_object_type_list:
			result = 0;
			break;
		case mocha_object_type_vector:
			result = first_vector(context, &sequence->data.vector);
			break;
		case mocha_object_type_nil:
			result = 0;
			break;
		case mocha_object_type_map:
			result = 0;
			break;
		default:
			break;
	}

	return result;
}


MOCHA_FUNCTION(quote_func)
{
	return arguments->objects[0];
}

MOCHA_FUNCTION(unquote_func)
{
	mocha_error error;
	return mocha_runtime_eval(runtime, arguments->objects[0], &error);
}

MOCHA_FUNCTION(not_func)
{
	const mocha_object* argument = arguments->objects[0];
	if (argument->type != mocha_object_type_boolean) {
		// error->code = mocha_error_code_expected_boolean_value;
		return 0;
	}
	mocha_object* inverted = mocha_context_create_object(context);
	inverted->type = mocha_object_type_boolean;
	inverted->data.b = !argument->data.b;

	return inverted;
}

MOCHA_FUNCTION(count_func)
{
	const mocha_object* sequence = arguments->objects[0];
	mocha_boolean is_empty = mocha_true;

	int count = 0;
	switch (sequence->type) {
		case mocha_object_type_list:
			count = sequence->data.list.count;
			break;
		case mocha_object_type_vector:
			count = sequence->data.vector.count;
			break;
		case mocha_object_type_nil:
			break;
		case mocha_object_type_map:
			count = sequence->data.map.count;
			break;
		default:
			break;
	}

	mocha_object* empty = mocha_context_create_object(context);
	empty->type = mocha_object_type_number;
	empty->data.number.data.i = count;
	empty->data.number.type = mocha_number_type_integer;

	return empty;

}

MOCHA_FUNCTION(empty_func)
{
	const mocha_object* sequence = arguments->objects[0];
	mocha_boolean is_empty = mocha_true;

	switch (sequence->type) {
		case mocha_object_type_list:
			break;
		case mocha_object_type_vector:
			is_empty = sequence->data.vector.count == 0;
			break;
		case mocha_object_type_nil:
			break;
		case mocha_object_type_map:
			break;
		default:
			break;
	}

	mocha_object* empty = mocha_context_create_object(context);
	empty->type = mocha_object_type_boolean;
	empty->data.b = is_empty;

	return empty;

}

#define MOCHA_DEF_FUNCTION_HELPER(name, eval_arguments) \
	static mocha_type name##_def; \
	name##_def.invoke = name##_func; \
	name##_def.eval_all_arguments = eval_arguments; \
	name##_def.is_macro = mocha_false; \

#define MOCHA_DEF_FUNCTION(name, eval_arguments) \
	MOCHA_DEF_FUNCTION_HELPER(name, eval_arguments) \
	mocha_context_add_function(context, #name, &name##_def);

#define MOCHA_DEF_FUNCTION_EX(name, exported_name, eval_arguments) \
	MOCHA_DEF_FUNCTION_HELPER(name, eval_arguments) \
	mocha_context_add_function(context, exported_name, &name##_def);

static void bootstrap_context(mocha_context* context)
{
	MOCHA_DEF_FUNCTION(def, mocha_false);
	MOCHA_DEF_FUNCTION(conj, mocha_true);
	MOCHA_DEF_FUNCTION(cons, mocha_true);
	MOCHA_DEF_FUNCTION(first, mocha_true);
	MOCHA_DEF_FUNCTION(rest, mocha_true);
	MOCHA_DEF_FUNCTION(let, mocha_false);
	MOCHA_DEF_FUNCTION(defmacro, mocha_false);
	MOCHA_DEF_FUNCTION(defn, mocha_false);
	MOCHA_DEF_FUNCTION_EX(mul, "*", mocha_true);
	MOCHA_DEF_FUNCTION_EX(add, "+", mocha_true);
	MOCHA_DEF_FUNCTION_EX(dec, "-", mocha_true);
	MOCHA_DEF_FUNCTION_EX(div, "/", mocha_true);
	MOCHA_DEF_FUNCTION_EX(equal, "=", mocha_true);
	MOCHA_DEF_FUNCTION_EX(empty, "empty?", mocha_true);
	MOCHA_DEF_FUNCTION(count, mocha_true);
	MOCHA_DEF_FUNCTION(fn, mocha_false);
	MOCHA_DEF_FUNCTION(if, mocha_false);
	MOCHA_DEF_FUNCTION(case, mocha_false);
	MOCHA_DEF_FUNCTION(quote, mocha_false);
	MOCHA_DEF_FUNCTION(unquote, mocha_false);
	MOCHA_DEF_FUNCTION(not, mocha_true);
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
		mocha_runtime_push_context(self, fn->data.function.context);
		mocha_context* new_context = mocha_runtime_create_context(self);
		for (size_t arg_count = 0; arg_count < args->count; ++arg_count) {
			const mocha_object* arg = args->objects[arg_count];
			if (arg->type != mocha_object_type_symbol) {
				MOCHA_LOG("Must use symbols!");
				return 0;
			}
			mocha_context_add(new_context, arg, l->objects[arg_count + 1]);
		}
		mocha_error error;
		o = mocha_runtime_eval(self, fn->data.function.code, &error);
		if (fn->object_type->is_macro) {
			MOCHA_LOG("MACRO!");
			o = mocha_runtime_eval(self, o, &error);
		}
		mocha_runtime_pop_context(self);
		mocha_runtime_pop_context(self);
	}

	return o;
}

void mocha_runtime_init(mocha_runtime* self)
{
	self->context = 0;
	const int max_depth = 32;
	self->contexts = malloc(sizeof(mocha_context) * max_depth);
	self->stack_depth = 0;
	mocha_runtime_create_context(self);
	mocha_object* nil = mocha_context_create_object(self->context);
	nil->type = mocha_object_type_nil;
	self->nil = nil;
	bootstrap_context(self->context);
}

mocha_context* mocha_runtime_create_context(mocha_runtime* self)
{
	mocha_context* new_context = &self->contexts[self->stack_depth++];
	mocha_context_init(new_context, self->context);
	self->context = new_context;

	return new_context;
}

void mocha_runtime_push_context(mocha_runtime* self, mocha_context* context)
{
	//self->contexts[self->stack_depth++] = context;
	self->stack_depth++;
	self->context = context;
}

void mocha_runtime_pop_context(mocha_runtime* self)
{
	--self->stack_depth;
	if (self->stack_depth == 0) {
		return;
	}
	mocha_context* next_context = &self->contexts[self->stack_depth - 1];
	self->context = next_context;
}


const struct mocha_object* mocha_runtime_eval(mocha_runtime* self, const struct mocha_object* o, mocha_error* error)
{
	if (o->type == mocha_object_type_list) {
		const mocha_list* l = &o->data.list;
		if (l->count == 0) {
			return o;
		}
		const struct mocha_object* fn = mocha_context_lookup(self->context, l->objects[0]);
		if (!fn) {
			MOCHA_LOG("Couldn't find lookup:");
			mocha_print_object_debug(l->objects[0]);
			return 0;
		}
		mocha_list new_args;
		if (fn->object_type->eval_all_arguments) {
			const mocha_object* converted_args[32];
			for (size_t i = 1; i < l->count; ++i) {
				const struct mocha_object* arg = mocha_runtime_eval(self, l->objects[i], error);
				if (!arg) {
					MOCHA_LOG("Couldn't evaluate:");
					mocha_print_object_debug(l->objects[i]);
					return 0;
				}
				converted_args[i] = arg;
			}
			mocha_list_init(&new_args, converted_args, l->count);
			l = &new_args;
		}
		o = invoke(self, self->context, fn, l);
		if (!o) {
			MOCHA_LOG("Invoke returned 0:");
			mocha_print_object_debug(fn);
		}
	} else {
		if (o->type == mocha_object_type_symbol) {
			o = mocha_context_lookup(self->context, o);
		}
	}

	return o;
}
