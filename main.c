#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jude.h"
#include "jude.tab.h"
#include "uthash.h"

// TODO
// - eq
// - tail recursion
// - unquote, quasiquote, comma_at
// - syntax for ' ` , ,@
// - macros
// - a garbage collector
// - libraries

// environment

void env_add_symbol(env_t * env, object_t* symbol, object_t* value)
{
  struct hash_entry * h_entry = malloc(sizeof(struct hash_entry));
  h_entry->key = symbol->value.symbol;
  h_entry->value = value;
  HASH_ADD_STR(env->hash, key, h_entry);
}

object_t* env_lookup(env_t* env, object_t* symbol) {
  struct hash_entry *h_entry;

  while (env!=NULL)
    {
      HASH_FIND_STR(env->hash, symbol->value.symbol, h_entry);
      if (h_entry == NULL)
	env = env->parent;
      else
	return h_entry->value;
    }

  return NULL;
}

procedure_t **builtin_procedure_list(int *count) {
  *count = 17;
  procedure_t **list = malloc(*count * sizeof(procedure_t *));

  list[0] = make_special_procedure("eval", (c_function_t) eval);
  list[1] = make_special_procedure("define", (c_function_t) __define);
  list[2] = make_special_procedure("lambda", (c_function_t) __lambda);
  list[3] = make_special_procedure("ccc", (c_function_t) __ccc);
  list[4] = make_special_procedure("if", (c_function_t) __if);
  list[5] = make_special_procedure("quote", (c_function_t) __quote);
  list[6] = make_special_procedure("begin", (c_function_t) __begin);
  list[7] = make_builtin_procedure("print", (c_function_t) print, 1);
  list[8] = make_builtin_procedure("car", (c_function_t) car, 1);
  list[9] = make_builtin_procedure("cdr", (c_function_t) cdr, 1);
  list[10] = make_builtin_procedure("cons", (c_function_t) cons, 2);
  list[11] = make_builtin_procedure("read", (c_function_t) __read, 0);
  list[12] = make_builtin_procedure("+", (c_function_t) __plus, -1);
  list[13] = make_builtin_procedure("-", (c_function_t) __minus, -1);
  list[14] = make_builtin_procedure("*", (c_function_t) __times, -1);
  list[15] = make_builtin_procedure("/", (c_function_t) __divide, -1);
  list[16] = make_builtin_procedure("length", (c_function_t) __length, 1);

  return list;
}
void register_builtins(env_t* env) {

  int count, i;
  procedure_t **procs = builtin_procedure_list(&count);
  object_t *symbol;

  for (i=0; i<count; i++)
    {
      symbol = make_symbol_object(procs[i]->builtin->name);
      env_add_symbol(env, symbol, make_procedure_object(procs[i]));
    }
}

// utility functions

int is_error(object_t *e) {
  return e->type == OBJECT_TYPE_ERROR;
}

int is_false(struct object *e) {
  return (e->type == OBJECT_TYPE_BOOLEAN) && (e->value.boolean == 0);
}
int is_true(struct object *e) {
  return !is_false(e); 
}

// type contructors for builtin_t, procedure_t, continuation_t

builtin_t *make_builtin(char *name, c_function_t function)
{
  builtin_t *b = malloc(sizeof(builtin_t));

  b->name = malloc(sizeof(name));
  strcpy(b->name, name);
  b->function = function;
  
  return b;
}

lambda_t *make_lambda(env_t* env, object_t *vars, object_t *code) {
  lambda_t *l = malloc(sizeof(lambda_t));

  l->env = env;
  l->vars = vars;
  l->code = code;

  return l;
}

procedure_t *make_procedure(int arity, builtin_t *builtin, lambda_t *lambda, invoke_t invoke)
{
  procedure_t *p = malloc(sizeof(procedure_t));

  p->arity = arity;
  p->builtin = builtin;
  p->lambda = lambda;
  p->invoke = invoke;
  return p;
}

procedure_t *make_builtin_procedure(char *name, c_function_t function, int arity)
{
  return make_procedure(arity,
			make_builtin(name, function),
			NULL,
			invoke_builtin);

}

procedure_t *make_special_procedure(char *name, c_function_t function) {
  return make_procedure(0,
			make_builtin(name, function),
			NULL,
			invoke_special);
}

procedure_t *make_lambda_procedure(env_t *env, object_t *vars, object_t *code)
{
  return make_procedure(__length(vars)->value.number,
			NULL,
			make_lambda(env, vars, code),
			invoke_lambda);
}

continuation_t *make_continuation(resume_t resume, object_t* data, env_t* env, continuation_t *k) {
  continuation_t *c = malloc(sizeof(continuation_t));

  c->resume = resume;
  c->data = data;
  c->env = env;
  c->k = k;

  return c;
}

env_t *make_env(env_t *parent)
{
  env_t *env = malloc(sizeof(env_t));

  env->parent = parent;
  env->hash = (struct hash_entry *) NULL;

  return env;
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

  object->value.symbol = malloc(sizeof(symbol));

  strcpy(object->value.symbol, symbol);
  
  object->type = OBJECT_TYPE_SYMBOL;

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

// built_ins

object_t* __plus(object_t *e)
{
  char s[200];
  int sum=0;
  object_t* v;

  while (e!=NULL) {
    if (car(e)->type != OBJECT_TYPE_NUMBER) {
      return make_error_object("not a number");
    } else {
      sum += car(e)->value.number;
      e = cdr(e);
    }
  }

  sprintf(s, "%d", sum);
  return make_number_object(sum);
}

object_t* __minus(object_t *e)
{
  char s[200];
  int sum=0;
  object_t* v;

  if (e!=NULL) {
    if (car(e)->type != OBJECT_TYPE_NUMBER)
	return make_error_object("not a number");
    else {
      sum += car(e)->value.number;
      e = cdr(e);
    }

    while (e!=NULL) {
      if (car(e)->type != OBJECT_TYPE_NUMBER) {
	return make_error_object("not a number");
      } else {
	sum -= car(e)->value.number;
	e = cdr(e);
      }
    }
  }

  sprintf(s, "%d", sum);
  return make_number_object(sum);
}

object_t* __times(object_t *e)
{
  char s[200];
  int product=1;
  object_t* v;

  while (e!=NULL) {
    if (car(e)->type != OBJECT_TYPE_NUMBER) {
      return make_error_object("not a number");
    } else {
      product *= car(e)->value.number;
      e = cdr(e);
    }
  }

  sprintf(s, "%d", product);
  return make_number_object(product);
}

object_t* __divide(object_t *e)
{
  char s[200];
  int product=1;
  object_t* v;

  if ((e!=NULL) &&
      (car(e)!=NULL) &&
      (cdr(e)!=NULL) &&
      (cadr(e) != NULL) &&
      (OBJECT_TYPE_NUMBER == car(e)->type) &&
      (OBJECT_TYPE_NUMBER == cadr(e)->type))
    product = car(e)->value.number/cadr(e)->value.number;
  else
    return make_error_object("not a number");

  sprintf(s, "%d", product);
  return make_number_object(product);
}

object_t* __length(object_t* object) {
  int result = 0;

  while (object!=NULL)
    {
      if (object->type != OBJECT_TYPE_PAIR)
	return make_error_object("not a proper list");
      else {
	result += 1;
	object = cdr(object);
      }
    }

  return make_number_object(result);
}

void print_pair(object_t *e) {
  printf("(");
  while (e!=NULL) {
    print_object(car(e));
    if (cdr(e)!= NULL)
      if (cdr(e)->type != OBJECT_TYPE_PAIR) {
	printf(" . ");
	print_object(cdr(e));
	e = NULL;
      } else {
	printf(" ");
	e=cdr(e);
      }
    else
      e=NULL;
  }
  printf(")");
}

void print_object(object_t *object) {
  if (object == NULL) {
    printf("NULL");
  } else {
    switch (object->type)
      {
      case OBJECT_TYPE_NUMBER:
	printf("%d", object->value.number);
	break;
      case OBJECT_TYPE_SYMBOL:
	printf("%s", object->value.symbol);
	break;
      case OBJECT_TYPE_BOOLEAN:
	printf(object->value.boolean ? "#t" : "#f");
	break;
      case OBJECT_TYPE_CONTINUATION:
	printf( "<continuation>");
	break;
      case OBJECT_TYPE_PROCEDURE:
	printf( "<procedure>");
	break;
      case OBJECT_TYPE_PAIR:
	print_pair(object);
	break;
      case OBJECT_TYPE_ERROR:
	printf("error: %s", object->value.error);
	break;
      default:
	printf("<object>");
      }
  }
}

object_t *print(object_t *object) {
  print_object(object);
  printf("\n");
  return NULL;
}

object_t __print(object_t *object, env_t* env, continuation_t* k) {
  print_object(object);
  printf("\n");

  resume(k, NULL);
}

object_t *__read(object_t *object, env_t* env, continuation_t* k) {
  printf("$ ");
  int n = yyparse();

  resume(k, yylval);
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


void __define(object_t *e, env_t* env, continuation_t* k)
{
  object_t *subject = car(e);
  object_t *object = cdr(e);

  void resume_define(continuation_t *c, object_t* obj) {
    env_add_symbol(c->env, c->data, obj); 
    resume(c->k, c->data);
  }

  if (OBJECT_TYPE_SYMBOL == subject->type) {
    eval(car(object), env, make_continuation(resume_define, subject, env, k));
  } else {
    object_t *vars = cdr(subject);
    object_t *code = object;

    env_add_symbol(env, car(subject), make_procedure_object(make_lambda_procedure(env, vars, code)));
    resume(k, car(subject));
  }
}

void __if(object_t *e, env_t *env, continuation_t* k)
{
  object_t *predicate = cdr(e);
  object_t *rest = cdr(e);

  void resume_if(continuation_t* c, object_t* obj) {
    object_t* rest = c->data;
    continuation_t* k = c->k;
    env_t *env = c->env;
  
    if (is_error(obj))
      resume(k, obj);

    else if (rest == NULL)
      resume(k, make_error_object("if requires at least one argument"));

    else
      {
	object_t* consequent = car(rest);

	if (is_true(obj))
	  eval(consequent, env, k);
	else if (cdr(rest)!=NULL)
	  eval(cadr(rest), env, k);
      }
  }

  eval(predicate, env, make_continuation(resume_if, rest, env, k));
}
   
void __lambda(object_t *e, env_t* env, continuation_t *k)
{
  object_t *vars = car(e);
  object_t *code = cdr(e);

  resume(k, make_procedure_object(make_lambda_procedure(env, vars, code)));
}

void __quote(object_t *e, env_t* env, continuation_t *k)
{
  resume(k, car(e));
}

void __ccc(object_t *e, env_t* env, continuation_t *k)
{
  void resume_ccc(continuation_t* c, object_t* obj) {
    invoke(make_procedure_object(obj->value.procedure),
	   make_continuation_object(c->k),
	   c->env,
	   c->k);
    
  }
  
  eval(car(e), env, make_continuation(resume_ccc, NULL, env, k));
}

void __begin(object_t *e, env_t* env, continuation_t *k)
{
  void resume_begin(continuation_t* c, object_t* obj) {
    object_t *remain = c->data;
    if (remain == NULL) {
      resume(c->k, obj);
    } else {
      __begin(remain, c->env, c->k);
    }
  }
  
  eval(car(e), env, make_continuation(resume_begin, cdr(e), env, k));
}

void __eval_sequence(object_t *args, env_t * env, continuation_t* k) {
  void resume_gather(continuation_t* c, object_t* obj)
  {
    resume(c->k, cons(c->data, obj));
  }
  
  void resume_eval_sequence(continuation_t*c, object_t* obj)
  {
    __eval_sequence(c->data, c->env, make_continuation(resume_gather, obj, c->env, c->k));
  }
      
  if (args == NULL)
    resume(k, NULL);
  else if (OBJECT_TYPE_PAIR == args->type)    
    eval(car(args), env, make_continuation(resume_eval_sequence, cdr(args), env, k));
  else
    eval(args, env, k);
}


void eval(object_t *e, env_t *env, continuation_t *k) {
  object_t* object;

  void resume_invoke(continuation_t* c, object_t* obj)
  {
    invoke(obj, c->data, c->env, c->k);
  }
  
  switch (e->type)
    {
    case OBJECT_TYPE_SYMBOL:
      resume(k, (NULL == (object = env_lookup(env, e))) ? make_error_object("unknown symbol") : object );
      break;
    case OBJECT_TYPE_NUMBER:
    case OBJECT_TYPE_ERROR:
    case OBJECT_TYPE_CONTINUATION:
    case OBJECT_TYPE_PROCEDURE:
    case OBJECT_TYPE_BOOLEAN:
      resume(k, e);
      break;
    case OBJECT_TYPE_PAIR:
      eval(car(e), env, make_continuation(resume_invoke, cdr(e), env, k));
      break;
    default:
      resume(k, make_error_object("eval fell of with unknown expression type\n"));
      break;
    }
}

// invoke

#define MAKE_INVOKE(fn, body) void fn(object_t *c, object_t *args, env_t* env, continuation_t* k) { body }


MAKE_INVOKE(invoke_thunk,
	    resume(k, c->value.procedure->builtin->function.thunk());)
MAKE_INVOKE(invoke_with_list,
	    resume(k, c->value.procedure->builtin->function.unary(args));)
MAKE_INVOKE(invoke_unary,
	    resume(k, c->value.procedure->builtin->function.unary(car(args)));)

MAKE_INVOKE(invoke_binary,
	    resume(k, c->value.procedure->builtin->function.binary(car(args),
								   cadr(args)));)

MAKE_INVOKE(invoke_ternary,
	    resume(k, c->value.procedure->builtin->function.ternary(car(args),
								    cadr(args),
								    caddr(args)));)

MAKE_INVOKE(invoke_four_ary,
	    resume(k, c->value.procedure->builtin->function.four_ary(car(args),
								     cadr(args),
								     caddr(args),
								     cadddr(args)));)
invoke_t invoke_list[5] = {invoke_thunk,
			   invoke_unary,
			   invoke_binary,
			   invoke_ternary,
			   invoke_four_ary};

void invoke_special(object_t* call, object_t* args, env_t* env, continuation_t* k)
{
  call->value.procedure->builtin->function.special(args, env, k);
}

void invoke_builtin(object_t* call, object_t* args, env_t* env, continuation_t* k)
{
  void resume_builtin_invoke(continuation_t* c, object_t* obj) {
    procedure_t* proc = c->data->value.procedure;
    ((proc->arity == -1) ? invoke_with_list : invoke_list[proc->arity])(call, obj, c->env, c->k);
  }
	    
  __eval_sequence(args, env, make_continuation(resume_builtin_invoke, call, env, k));
}


void invoke_lambda(object_t* call, object_t* args, env_t* env, continuation_t* k)
{
  void resume_lambda_invoke(continuation_t *c, object_t* obj) {
    procedure_t *p = c->data->value.procedure;
    env_t *closure = make_env(p->lambda->env);
    object_t *v = p->lambda->vars;
    object_t *a = obj;

    while (v != NULL) {
      env_add_symbol(closure, car(v), car(a));
      v = cdr(v);
      a = cdr(a);
    }
	    
    __begin(p->lambda->code, closure, k);
  }
  
  __eval_sequence(args, env, make_continuation(resume_lambda_invoke, call, env, k));
}

void invoke(object_t* call, object_t* args, env_t* env, continuation_t* k)
{
  if (call->type == OBJECT_TYPE_PROCEDURE)
    call->value.procedure->invoke(call, args, env, k);
  else if (call->type == OBJECT_TYPE_CONTINUATION)
    resume(call->value.continuation, args);
  else if (call->type == OBJECT_TYPE_ERROR)
    resume(k, call);
}

// resume

void resume(continuation_t* c, object_t* obj)
{
  if (c == NULL)
    {
      printf("null continuation\n");
      exit(0);
    }
  else c->resume(c, obj);
}

// main


void resume_read(continuation_t*, object_t*);
void resume_print(continuation_t*, object_t*);
void resume_eval(continuation_t*, object_t*);

void resume_print(continuation_t* c, object_t* value) {
  __print(value, c->env, make_continuation(resume_read, NULL, c->env, c->k));
}

void resume_eval(continuation_t* c, object_t* datum) {
  eval(datum, c->env, make_continuation(resume_print, NULL, c->env, c->k));
}
  
void resume_read(continuation_t* c, object_t* object) {
  __read(object, c->env, make_continuation(resume_eval, NULL, c->env, c->k));
}

int main(int argc, char **argv) {
  env_t *genv = malloc(sizeof(env_t));

  genv->parent = NULL; 

  register_builtins(genv);
  
  resume(make_continuation(resume_read, NULL, genv, NULL), NULL);

}
