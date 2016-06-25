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

builtin_t *make_builtin(char *name, c_function_t function, invoke_t invoke)
{
  builtin_t *b = malloc(sizeof(builtin_t));

  b->name = malloc(sizeof(name));
  strcpy(b->name, name);
  b->function = function;
  b->invoke = invoke;
  
  return b;
}

lambda_t *make_lambda(env_t* env, object_t *vars, object_t *code) {
  lambda_t *l = malloc(sizeof(lambda_t));

  l->env = env;
  l->vars = vars;
  l->code = code;

  return l;
}

procedure_t *make_procedure(procedure_type_t type, int arity, builtin_t *builtin, lambda_t *lambda)
{
  procedure_t *p = malloc(sizeof(procedure_t));

  p->type = type;
  p->arity = arity;
  p->builtin = builtin;
  p->lambda = lambda;

  return p;
}

procedure_t *make_builtin_procedure(char *name, c_function_t function, int arity)
{

  return make_procedure(PROCEDURE_TYPE_BUILTIN,
			arity,
			make_builtin(name, function, invoke_builtin),
			NULL);

}

procedure_t *make_special_procedure(char *name, c_function_t function) {
  return make_procedure(PROCEDURE_TYPE_SPECIAL, 0, make_builtin(name, function, invoke_special), NULL);
}

procedure_t *make_lambda_procedure(env_t *env, object_t *vars, object_t *code)
{
  return make_procedure(PROCEDURE_TYPE_LAMBDA, length(vars), NULL, make_lambda(env, vars, code));
}

continuation_t *make_continuation(resume_t resume, object_t* data, env_t* env, continuation_t *k) {
  continuation_t *c = malloc(sizeof(continuation_t));

  c->resume = resume;
  c->data = data;
  c->env = env;
  c->k = k;

  return c;
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

//

char *number_to_str(int n) {
  char *str = malloc(20 * sizeof(char));

  sprintf(str, "%d", n);

  return str;
}

char *object_to_str(object_t *);

void print_object(object_t *);

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

// built ins

object_t *print(object_t *object) {
  printf("> ");
  print_object(object);
  printf("\n");
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

void resume_define(continuation_t *c, object_t* value)
{
  env_add_symbol(c->env, c->data, value); 
  resume(c->k, c->data);
}

void __define(object_t *e, env_t* env, continuation_t* k)
{
  object_t *subject = car(e);
  object_t *object = cdr(e);
  
  if (OBJECT_TYPE_SYMBOL == subject->type) {
    eval(car(object), env, make_continuation(resume_define, subject, env, k));
  } else {
    object_t *vars = cdr(subject);
    object_t *code = object;

    env_add_symbol(env, car(subject), make_procedure_object(make_lambda_procedure(env, vars, code)));
    resume(k, car(subject));
  }
}

void resume_if(continuation_t *c, object_t *value)
{
  object_t* rest = c->data;
  continuation_t* k = c->k;
  env_t *env = c->env;
  
  if (is_error(value))
    resume(k, value);

  else if (rest == NULL)
    resume(k, make_error_object("if requires at least one argument"));

  else
    {
      object_t* consequent = car(rest);

      if (is_true(value))
	eval(consequent, env, k);
      else if (cdr(rest)!=NULL)
	eval(cadr(rest), env, k);
    }
}

void __if(object_t *e, env_t *env, continuation_t* k)
{
  object_t *predicate = cdr(e);
  object_t *rest = cdr(e);

  eval(predicate, env, make_continuation(resume_if, rest, env, k));
}
   
void __lambda(object_t *e, env_t* env, continuation_t *k)
{
  object_t *vars = car(e);
  object_t *code = cdr(e);

  resume(k, make_procedure_object(make_lambda_procedure(env, vars, code)));
}

object_t* __quote(object_t *e, env_t* env, continuation_t *k)
{
  resume(k, car(e));
}

void resume_ccc(continuation_t* c, object_t *p)
{
  invoke(make_procedure_object(p->value.procedure),
	 make_continuation_object(c->k),
	 c->env,
	 c->k);
}

void __ccc(object_t *e, env_t* env, continuation_t *k) {
  eval(car(e), env, make_continuation(resume_ccc, NULL, env, k));
}

void __begin(object_t *e, env_t* env, continuation_t *k);

void resume_begin(continuation_t* c, object_t *object) {
  object_t *remain = c->data;

  if (remain == NULL)
    resume(c->k, object);
  else
    __begin(remain, c->env, c->k);
}

void __begin(object_t *e, env_t* env, continuation_t *k) {
  eval(car(e), env, make_continuation(resume_begin, cdr(e), env, k));
}

#define KEYWORD(s) !strcmp(car(e)->value.symbol, s)

void resume_invoke(continuation_t * c, object_t* object) {
  invoke(object, c->data, c->env, c->k);
}

void eval(object_t *e, env_t *env, continuation_t *k) {
  switch (e->type)
    {
    case OBJECT_TYPE_SYMBOL:
      env_lookup(env, e, k); break;
    case OBJECT_TYPE_NUMBER:
    case OBJECT_TYPE_ERROR:
    case OBJECT_TYPE_CONTINUATION:
    case OBJECT_TYPE_PROCEDURE:
    case OBJECT_TYPE_BOOLEAN:
      resume(k, e); break;
    case OBJECT_TYPE_PAIR:
      eval(car(e), env, make_continuation(resume_invoke, cdr(e), env, k));
      break;
      /*
      if (car(e)->type == OBJECT_TYPE_SYMBOL) {
	if (KEYWORD("quote"))	    eval_quote(cdr(e), env, k);
	else if (KEYWORD("define")) eval_define(cdr(e) , env, k);
	else if (KEYWORD("if"))	    eval_if(cdr(e), env, k);
	else if (KEYWORD("lambda")) eval_lambda(cdr(e), env, k);
	else if (KEYWORD("ccc"))    eval_ccc(cdr(e), env, k);
	else {
	  printf("lookup symbol and apply if procedure");
	  resume(k, e);
	}
      } else {
	printf("look for an invocable and invoke it\n");
	resume(k, e);
      }
      */
    default:
      resume(k, make_error_object("eval fell of with unknown expression type\n")); break;
    }

}

void resume_gather(continuation_t *c, object_t* o)
{
  resume(c->k, cons(c->data, o));
}


void __args(object_t *args, env_t * env, continuation_t* k);
  
void resume_args(continuation_t *c, object_t* o)
{
  __args(c->data, c->env, make_continuation(resume_gather, o, c->env, c->k));
}

void __args(object_t *args, env_t * env, continuation_t* k) {
  if (args == NULL)
    resume(k, NULL);
  else if (OBJECT_TYPE_PAIR == args->type)    
    eval(car(args), env, make_continuation(resume_args, cdr(args), env, k));
  else
    eval(args, env, k);
}

void invoke_thunk(procedure_t *p, object_t *args, env_t* env, continuation_t* k) {
  resume(k, p->builtin->function.thunk());
}

void invoke_with_list(procedure_t *p, object_t *args, env_t* env, continuation_t* k) {
  resume(k, p->builtin->function.unary(args));
}

void invoke_unary(procedure_t *p, object_t *args, env_t* env, continuation_t* k) {
  resume(k, p->builtin->function.unary(car(args)));
}

void invoke_binary(procedure_t *p, object_t *args, env_t* env, continuation_t* k) {
  resume(k, p->builtin->function.binary(car(args), cadr(args)));
}

void invoke_ternary(procedure_t *p, object_t *args, env_t* env, continuation_t* k) {
  resume(k, p->builtin->function.ternary(car(args), cadr(args), caddr(args)));
}

void invoke_four_ary(procedure_t *p, object_t *args, env_t* env, continuation_t* k) {
  resume(k, p->builtin->function.four_ary(car(args), cadr(args), caddr(args), cadddr(args)));
}


void resume_builtin_invoke(continuation_t *c, object_t *args) {
  procedure_t* p = c->data->value.procedure;

  invoke_t invoke_list[5] = {invoke_thunk, invoke_unary, invoke_binary, invoke_ternary, invoke_four_ary};
  ((p->arity == -1) ? invoke_with_list : invoke_list[p->arity])(p, args, c->env, c->k);

}

void invoke_builtin(procedure_t *p, object_t *args, env_t *env, continuation_t *k) {

  __args(args, env, make_continuation(resume_builtin_invoke, make_procedure_object(p), env, k));
}

void invoke_special(procedure_t *p, object_t *args, env_t *env, continuation_t *k) {
  p->builtin->function.special(args, env, k);
}

void invoke_lambda(procedure_t *p, object_t *args, env_t* env, continuation_t* k) {
  printf("invoke lambda\n");
}


void invoke(object_t *callable, object_t *obj, env_t* env, continuation_t* k) {
  procedure_t* proc = callable->value.procedure;

  if (callable->type == OBJECT_TYPE_PROCEDURE)
    proc->builtin->invoke(proc, obj, env, k);
  else if (callable->type == OBJECT_TYPE_CONTINUATION)
    resume(callable->value.continuation, obj);
  else if (callable->type == OBJECT_TYPE_ERROR)
    resume(k, callable);
}

void resume_procedure(continuation_t *c, object_t* obj) {
  invoke(c->data, obj, c->env, c->k);
}

continuation_t *make_procedure_continuation(procedure_t *p) {
  return make_continuation(resume_builtin_invoke, make_procedure_object(p), NULL, NULL);
}

void resume(continuation_t *k, object_t *object) {
  if (k != NULL) {
    k->resume(k, object);
  }
}


procedure_t **builtin_procedure_list(int *count) {
  *count = 12;
  procedure_t **list = malloc(*count * sizeof(procedure_t *));

  list[0] = make_special_procedure("eval", (c_function_t) eval);
  list[1] = make_special_procedure("define", (c_function_t) __define);
  list[2] = make_special_procedure("lambda", (c_function_t) __lambda);
  list[3] = make_special_procedure("ccc", (c_function_t) __ccc);
  list[4] = make_special_procedure("if", (c_function_t) __if);
  list[5] = make_special_procedure("quote", (c_function_t) __quote);
  list[6] = make_special_procedure("begin", (c_function_t) __begin);
  list[7] = make_builtin_procedure("print", (c_function_t) print, -1);
  list[8] = make_builtin_procedure("car", (c_function_t) car, 1);
  list[9] = make_builtin_procedure("cdr", (c_function_t) cdr, 1);
  list[10] = make_builtin_procedure("cons", (c_function_t) cons, 2);
  list[11] = make_builtin_procedure("print", (c_function_t) read, 0);
  
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


int main(int argc, char **argv) {

  env_t *genv = malloc(sizeof(env_t));
  genv->parent = NULL; 
  //  procedure_* c_print = 
  //  continuation_t* c_eval = make_primitive_continuation("eval", 0, 0, eval, c_print);
  //  continuation_t* c_read = make_primitive_continuation("read", 0, 0, read, c_eval);
  //  c_print->k = c_read;

  procedure_t * c_print = make_builtin_procedure("print", (c_function_t) print, -1);

  continuation_t *print_continuation = make_procedure_continuation(c_print);

  register_builtins(genv);
  
  while (1) {
    eval(read(), genv, print_continuation);
  }
}
