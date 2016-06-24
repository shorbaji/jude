#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jude.h"
#include "jude.tab.h"
#include "uthash.h"

// environment

void env_add_symbol(env_t * env, object_t* symbol, object_t* value)
{
  struct hash_entry * h_entry = malloc(sizeof(struct hash_entry));
  h_entry->key = symbol->value.symbol;
  h_entry->value = value;
  HASH_ADD_STR(env->hash, key, h_entry);
}

struct object* env_lookup(env_t* env, object_t* symbol, continuation_t* k) {
  struct hash_entry *h_entry;

  HASH_FIND_STR(env->hash, symbol->value.symbol, h_entry);

  if (h_entry == NULL)
    if (env->parent == NULL)
      {
	resume(k, make_error_object("unknown symbol"));
      }
    else
      env_lookup(env->parent, symbol, k);
  else
    resume(k, h_entry->value);
}
// type contructors for builtin_t, procedure_t, continuation_t

builtin_t *make_builtin(char *name, c_function_t function)
{
  builtin_t *b = malloc(sizeof(builtin_t));

  b->name = name;
  b->function = function;

  return b;
}

lambda_t *make_lambda(env* env, object_t *vars, object_t *code) {
  lambda_t *l = malloc(sizeof(lambda_t));

  l->env = env;
  l->vars = vars;
  l->code = code;

  return l;
}

procedure_t* make_procedure(procedure_type_t type, int min, int max, builtin_t builtin, lambda_t *lambda)
{
  procedure_t *p = malloc(sizeof(procedure_t));

  p->type = type;
  p->min = min;
  p->max = max;
  p->builtin = builtin;
  p->lambda = lambda;

  return p;
}

procedure_t* make_builtin_procedure(char *name, c_function_t function, int min, int max)
{
   make_procedure(PROCEDURE_TYPE_BUILTIN, min, max, make_builtin(name, function));
}

procedure_t* make_lambda_procedure(env_t *env, object_t *vars, object_t *code, int min, int max)
{
  make_procedure(PROCEDURE_TYPE_LAMBDA, min, max, make_lambda(env, vars, code);









}
// object constructors

object_t *make_number_object(int n) {
  object_t *object = malloc(sizeof(object_t));

  object->type = OBJECT_TYPE_NUMBER;
  object->value.number = n;

  return object;
}

object_t *make_boolean_object(int n) {
  object_t *object = malloc(sizeof(object_t));

  object->type = OBJECT_TYPE_BOOLEAN;
  object->value.boolean = n;

  return object;
}

object_t *make_symbol_object(char* symbol) {
  object_t *object = malloc(sizeof(object_t));

  object->type = OBJECT_TYPE_SYMBOL;
  object->value.symbol = symbol;

  return object;
}

object_t *make_error_object(char* error) {
  object_t *object = malloc(sizeof(object_t));

  object->type = OBJECT_TYPE_ERROR;
  object->value.error = error;

  return object;
}

object_t *make_continuation_object(continuation_t *c) {
  object_t *object = malloc(sizeof(object_t));

  object->type = OBJECT_TYPE_CONTINUATION;
  object->value.continuation = c;

  return object;
}

object_t *make_procedure_object(procedure_t *p) {
  object_t *object = malloc(sizeof(object_t));

  object->type = OBJECT_TYPE_PROCEDURE;
  object->value.procedure = p;

  return object;
}

//

char *number_to_str(int n) {
  char *str = malloc(20 * sizeof(char));

  sprintf(str, "%d", n);

  return str;
}

builtin_t *make_builtin(char* name, c_function_t function)
{
  builtin_t *c_fn = malloc(sizeof(builtin_t));
  c_fn->name = name;
  c_fn->function = function;

  return c_fn;
}


char *object_to_str(object_t *object) {
  char *s;

  if (object == NULL) {
    
    s = malloc(5 * sizeof(char));
    strcpy(s, "NULL");
  } else {
    switch (object->type) {
    case OBJECT_TYPE_NUMBER:
      s = number_to_str(object->value.number); break;
    case OBJECT_TYPE_SYMBOL:
      s = object->value.symbol; break;
    case OBJECT_TYPE_CONTINUATION:
      s = "<continuation>"; break;
    case OBJECT_TYPE_PAIR:
      s = "<pair>"; break;
    case OBJECT_TYPE_ERROR:
      s = malloc(7 + strlen(object->value.error));
      strcpy(s, "error: ");
      strcpy(s+7, object->value.error);
      break;
    default:
      s = malloc(100);
      strcpy(s, "<object>"); 
    }
  }

  return s;
}


// built ins

object_t *print(object_t *object) {
  printf("> %s\n", object_to_str(object));
  return NULL;
}

object_t *read() {
  printf("$ ");
  int n = yyparse();

  return yylval;
}

object_t *cons(object_t *a, object_t *b) {
  object_t *object = malloc(sizeof(object_t));

  object->type = OBJECT_TYPE_PAIR;
  object->value.pair.car = a;
  object->value.pair.cdr = b;

  return object;
}

object_t *car(object_t *pair) {
  return pair->value.pair.car;
}

object_t *cdr(object_t *pair) {
  return pair->value.pair.cdr;
}

object_t *cadr(object_t *o)   { car(cdr(o)); }
object_t *cddr(object_t *o)   { cdr(cdr(o)); }
object_t *caddr(object_t *o)  { car(cddr(o)); }
object_t *cdddr(object_t *o)  { cdr(cddr(o)); }
object_t *cadddr(object_t *o) { car(cdddr(o)); }
object_t *cddddr(object_t *o) { cdr(cdddr(o)); }

int length(object_t* object) {
  if (object == NULL)
    return 0;
  else if (object->type == OBJECT_TYPE_PAIR)
    return 1 + length(cdr(object));
  else
    return 1;
}

/*
continuation_t * make_primitive_continuation(char* symbol,
					     int min,
					     int max,
					     _builtin_t builtin,
					     continuation_t* k) {
  continuation_t *c = malloc(sizeof(continuation_t));

  c->type = CONTINUATION_TYPE_BUILTIN;
  c->min = min;
  c->max = max;
  c->builtin = builtin;
  c->env = NULL;
  c->k = k;

  return c;
}
*/

continuation_t * make_define_continuation(object_t* subject,
					  env_t* env,
					  continuation_t* k)
{
}

/*
void eval_define(object_t *subject, object_t* object, env_t* env, continuation_t* k) {
  if (OBJECT_TYPE_SYMBOL == subject->type) {
    eval(car(object), env, make_define_continuation(subject, env, k));
  } else {
    env_add_symbol(env, car(subject), lambda(cdr(subject), object, env));
    resume(k, car(subject));
  }
}
*/

void eval(object_t *e, env_t *env, continuation_t *k) {

  switch (e->type)
    {
    case OBJECT_TYPE_PAIR:
      //      if (!strcmp(car(e)->value.symbol, "define"))
      //	eval_define(cadr(e), cddr(e), env, k);
      //      else
      resume(k, e); break;
    case OBJECT_TYPE_NUMBER:
    case OBJECT_TYPE_ERROR:
    case OBJECT_TYPE_CONTINUATION:
      resume(k, e);
    case OBJECT_TYPE_SYMBOL:
      env_lookup(env, e, k);
      break;
    default:
      resume(k, make_error_object("eval fell of with unknown expression type\n"));
    }
}

void invoke_builtin(procedure_t *p, object_t *args, env_t* env, continuation_t* k) {
  object_t *result = NULL;
  int len = length(args);
  
  if ((p->min == 1) && (p->max == 1)) {
    result = p->builtin.function.unary(args);
  } else if (((p->min==-1) || (len>=p->min)) &&
	     ((p->max==-1) || (len<=p->max))) {
    switch (len) {
    case 0:
      result = p->builtin.function.thunk(); break;
    case 2:
      result = p->builtin.function.binary(car(args), cadr(args)); break;
    case 3:
      result = p->builtin.function.ternary(car(args), cadr(args), caddr(args)); break;
    case 4:
      result = p->builtin.function.four_ary(car(args), cadr(args), caddr(args), cadddr(args)); break;
    }
  } else {
    result = make_error_object("wrong arity");
  }
  
  resume(k, result);
}
void invoke(invocable * i, object_t *obj, env_t* env, continuation_t* k) {
  switch (i->type)
    {
    case INVOCABLE_TYPE_BUILTIN:
      invoke_builtin(i->procedure, obj, env, k); break;
    case INVOCABLE_TYPE_LAMBDA:
      invoke_lambda(i->procedure, obj, env, k); break;
    case INVOCABLE_TYPE_CONTINUATION:
      resume(i->continuation, obj); break;
    }
  printf("invoke fell off\n");
}

void resume(continuation_t *k, object_t *object) {
  if (k == NULL) {
    printf("exiting with null continuation\n");
    exit(0);
  }

  k->resume(k, object);
}

int main(int argc, char **argv) {

  env_t *genv = malloc(sizeof(env_t));
  genv->parent = NULL; 
  //  procedure_* c_print = make_builtin_procedure("print", (c_function_t) print);
  //  continuation_t* c_eval = make_primitive_continuation("eval", 0, 0, eval, c_print);
  //  continuation_t* c_read = make_primitive_continuation("read", 0, 0, read, c_eval);
  //  c_print->k = c_read;

  procedure_t * c_print = make_builtin_procedure("print", (c_function_t) print);
  continuation_t *print_continuation = make_builtin_continuation(c_print);
  
  while (1) {
    eval(read(), genv, print_continuation);
  }
}
