#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include "jude.h"
#include "jude.tab.h"

// TODO
// - unquote, quasiquote, comma_at
// - macros
// - tail recursion
// - syntax for ' ` , ,@
// - a garbage collector
// - libraries

// built-in procedures

struct object* _plus_fn(struct object *e)
{
  char s[200];
  int sum=0;
  struct object* v;

  while (e!=NULL) {
    if (car(e)->type != TYPE_NUMBER) {
      return error_to_object("not a number");
    } else {
      sum += car(e)->value.number;
      e = cdr(e);
    }
  }

  sprintf(s, "%d", sum);
  return number_to_object(s);
}

struct object* _minus_fn(struct object *e)
{
  char s[200];
  int sum=0;
  struct object* v;

  if (e!=NULL) {
    if (car(e)->type != TYPE_NUMBER)
	return error_to_object("not a number");
    else {
      sum += car(e)->value.number;
      e = cdr(e);
    }

    while (e!=NULL) {
      if (car(e)->type != TYPE_NUMBER) {
	return error_to_object("not a number");
      } else {
	sum -= car(e)->value.number;
	e = cdr(e);
      }
    }
  }

  sprintf(s, "%d", sum);
  return number_to_object(s);
}

struct object* _times_fn(struct object *e)
{
  char s[200];
  int product=1;
  struct object* v;

  while (e!=NULL) {
    if (car(e)->type != TYPE_NUMBER) {
      return error_to_object("not a number");
    } else {
      product *= car(e)->value.number;
      e = cdr(e);
    }
  }

  sprintf(s, "%d", product);
  return number_to_object(s);
}

struct object* _divide_fn(struct object *e)
{
  char s[200];
  int product=1;
  struct object* v;

  if ((e!=NULL) &&
      (car(e)!=NULL) &&
      (cdr(e)!=NULL) &&
      (cadr(e) != NULL) &&
      (TYPE_NUMBER == car(e)->type) &&
      (TYPE_NUMBER == cadr(e)->type))
    product = car(e)->value.number/cadr(e)->value.number;
  else
    return error_to_object("not a number");

  sprintf(s, "%d", product);
  return number_to_object(s);
}

struct object* make_builtin_procedure(struct object* (* fn) (struct object *e)) {
  struct procedure* p = malloc(sizeof(struct procedure));
  p->fn = fn;
  p->code = NULL;
  p->variables = NULL;

  return procedure_to_object(p);  
}

void load(struct object *env, char *symbol, struct object *(* fn) (struct object *e)) {
  env_add_symbol(env, symbol_to_object(symbol), make_builtin_procedure(fn));
}

void load_built_ins(struct object * env) {
  load(env, "+", _plus_fn);
  load(env, "-", _minus_fn);
  load(env, "*", _times_fn);
  load(env, "/", _divide_fn);
}

// built-in types

struct object* number_to_object(char* s)
{
  struct object* e = malloc(sizeof(struct object));

  e->type = TYPE_NUMBER;
  e->value.number = atoi(s);

  return e;
}

struct object* symbol_to_object(char *s)
{
  int l = strlen(s);
  char* symbol = (char *) malloc(sizeof(char)*strlen(s));
  struct object* e = malloc(sizeof(struct object));

  strncpy(symbol, s, l);
  e->type = TYPE_SYMBOL;
  e->value.symbol = symbol;

  return e;
}

struct object* boolean_to_object(int b)
{
  struct object* e = malloc(sizeof(struct object));
  e->type = TYPE_BOOLEAN;
  e->value.boolean = b;

  return e;
}

struct object* procedure_to_object(struct procedure* procedure)
{
  struct object* e = malloc(sizeof(struct object));

  e->type = TYPE_PROCEDURE;
  e->value.procedure = procedure;

  return e;
}

struct object* error_to_object(char *error)
{
  struct object* e = malloc(sizeof(struct object));
  e->type = TYPE_ERROR;
  e->value.error = malloc(sizeof(char)*strlen(error));
  strncpy(e->value.error, error, strlen(error));

  return e;
}

int is_error(struct object *e) {
  return e->type == TYPE_ERROR;
}

int is_false(struct object *e) {
  return (e->type == TYPE_BOOLEAN) && (e->value.boolean == 0);
}
int is_true(struct object *e) {
  return !is_false(e); 
}

// environment

void env_add_symbol(struct object * env, struct object* symbol, struct object* value)
{
  struct hash_entry * h_entry = malloc(sizeof(struct hash_entry));
  h_entry->key = symbol->value.symbol;
  h_entry->value = value;
  HASH_ADD_STR(env->value.env.hash, key, h_entry);
}

struct object* env_lookup(struct object* env, struct object* symbol, struct object* k) {
  struct hash_entry *h_entry;

  HASH_FIND_STR(env->value.env.hash, symbol->value.symbol, h_entry);

  if (h_entry == NULL)
    if (env->value.env.parent == NULL)
      resume(k, error_to_object("unknown symbol"));
    else
      env_lookup(env->value.env.parent, symbol, k);
  else
    resume(k, h_entry->value);
}

struct object *make_env(struct object* parent) {
  struct object *env = malloc(sizeof(struct object));

  env->value.env.hash = NULL;
  env->value.env.parent = parent;

  return env;
}

// print

void print_pair(struct object *e) {
  printf("(");
  while (e!=NULL) {
    print_datum(car(e));
    if (cdr(e)!= NULL)
      if (cdr(e)->type != TYPE_PAIR) {
	printf(" . ");
	print_datum(cdr(e));
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

void print_datum(struct object *obj) {
  switch (obj->type) {
    case TYPE_NUMBER:
      printf("%d", obj->value.number); break;
    case TYPE_SYMBOL:
      printf("%s", obj->value.symbol); break;
    case TYPE_BOOLEAN:
      printf(is_false(obj) ? "#f" : "#t"); break; 
    case TYPE_PROCEDURE:
      printf("<procedure>"); break;
    case TYPE_PAIR:
      print_pair(obj); break;
    case TYPE_ERROR:
      printf("error: %s", obj->value.error); break;
  }
}

void print(struct object* e) {
  if (e!=NULL)
    print_datum(e);
  printf("\n");
}

// special forms

// built-in forms for eval: quote, if, atom, eq, cons, car, cdr

struct object* atom(struct object *e)
{
  if (is_error(e)) return e;
  return boolean_to_object(e->type != TYPE_PAIR);
}

struct object* eq(struct object* a, struct object* b)
{
  if (is_error(a)) return (a);
  if (is_error(b)) return (b);  

  return
    boolean_to_object((a->type == b->type) &&
		    (((a->type == TYPE_NUMBER) && (a->value.number == b->value.number)) ||
		     ((a->type == TYPE_SYMBOL) && !strcmp(a->value.symbol, b->value.symbol)) ||
		     ((a->type == TYPE_PAIR) && (a==b))));
}

struct object* cons(struct object* car, struct object* cdr)
{
  struct object* e = malloc(sizeof(struct object*));

  e->type = TYPE_PAIR;
  e->value.pair.car = car;
  e->value.pair.cdr = cdr;

  return e;
}

struct object* car(struct object *e) {  return e->value.pair.car;}
struct object* cdr(struct object *e) {  return e->value.pair.cdr; }

// helper functions for eval

struct object* cddr(struct object *e) { return cdr(cdr(e)); }
struct object* cadr(struct object *e) { return car(cdr(e)); }
struct object* caddr(struct object *e) { return car(cdr(cdr(e))); }
struct object* cadddr(struct object *e) { return car(cdr(cdr(cdr(e)))); }

// continuations

struct object *continuation_to_object(struct continuation *k) {
  struct object *obj = malloc(sizeof(struct object));
  obj->type = TYPE_CONTINUATION;
  obj->value.continuation = k;

  return obj;
}
  
struct object *make_continuation(int type,
				 struct object *first,
				 struct object *second,
				 struct object *env,
				 struct object *k)
{
  struct continuation *c = malloc(sizeof(struct continuation));

  c->type = type;
  c->first = first;
  c->second = second;
  c->env = env;
  c->k = k;

  return continuation_to_object(c);
}

struct object *make_if_continuation(struct object *consequent, struct object *alternative, struct object *env, struct object *k)
{
  return make_continuation(CONT_IF, consequent, alternative, env, k);
}

struct object *make_add_symbol_continuation(struct object *subject, struct object *env, struct object *k)
{
  return make_continuation(CONT_ADD_SYMBOL, subject, NULL, env, k);
}

struct object *make_application_continuation(struct object *args, struct object *env, struct object *k)
{
  return make_continuation(CONT_APPLICATION, args, NULL, env, k);
  
}
struct object *make_invoke_continuation(struct object* fn, struct object *env, struct object *k)
{
  return make_continuation(CONT_INVOKE, fn, NULL, env, k);
}

struct object *make_shift_continuation(struct object* remain, struct object* done, struct object *env, struct object *k)
{
  return make_continuation(CONT_SHIFT, remain, done, env, k);
}

struct object *make_rest_continuation(struct object* rest, struct object* env, struct object* k)
{
  return make_continuation(CONT_REST, rest, NULL, env, k);
}

struct object *make_built_in_continuation(void (*fn)(struct object *)) {
  struct object * obj = malloc(sizeof(struct object *));
  obj->type = TYPE_CONTINUATION;
  obj->value.continuation = malloc(sizeof(struct continuation));
  obj->value.continuation->fn = fn;
  obj->value.continuation->type = CONT_BUILT_IN;
  return obj;
}

// preparing for eval

void evlis(struct object *list, struct object *env, struct object* k)
{
  if (list == NULL)
    resume(k, NULL);
  else
    eval(car(list), env, make_shift_continuation(cdr(list), NULL, env, k));
}

struct object * invoke(struct object *obj, struct object *l, struct object * k) {
  struct procedure *p = obj->value.procedure;
  
  if (p->fn!= NULL)                     // if built-in procedure just run it
    resume(k,  p->fn(l));
  else {

    
    struct object *vars = p->variables;
    struct object *env = make_env(p->parent);            // create a new env
  
    while (vars != NULL) {                // bind variables
      env_add_symbol(env, car(vars), car(l));
      vars = cdr(vars);
      l = cdr(l);
    }
    
    struct object* code = p->code;
    eval(car(code), env, make_rest_continuation(cdr(code), env, k));
  }
}

void resume(struct object *k, struct object *obj) {
  struct continuation *c = k->value.continuation;
  struct object *args;
  struct object *remain;
  
  switch (c->type) {
  case CONT_BUILT_IN:
    c->fn(obj);
    break;
  case CONT_IF:
    eval(is_false(obj)? c->second : c->first, c->env, c->k); break;
  case CONT_ADD_SYMBOL:
    env_add_symbol(c->env, c->first, obj);
    resume(c->k, c->first);
    break;
  case CONT_APPLICATION:
    if (obj->type == TYPE_PROCEDURE) // obj is a procedure
      evlis(c->first, c->env , make_invoke_continuation(obj, c->env, c->k));
    else if (obj->type == TYPE_CONTINUATION)
      eval(car(c->first), c->env, obj);
    else
      resume(c->k, error_to_object("not a procedure"));
    break;
  case CONT_INVOKE:
    invoke(c->first, obj, c->k);
    break;
  case CONT_SHIFT:
    args = cons(obj, c->second);
    remain = c->first;
    if (remain == NULL)
      resume(c->k, args);
    else
      eval(car(remain), c->env, make_shift_continuation(cdr(remain), args, c->env, c->k));

    break;
  case CONT_REST:
    if (c->first == NULL)
      resume(c->k, obj);
    else
      eval(car(c->first), c->env, make_rest_continuation(cdr(c->first), c->env, c->k));
  }
}

struct object * lambda(struct object *vars, struct object* code, struct object* env) {
  struct procedure* p = malloc(sizeof(struct procedure));

  p->variables = vars;
  p->code = code;
  p->fn = NULL;
  p->parent = env;

  return procedure_to_object(p);
}

struct object * define(struct object *subject, struct object* object, struct object* env, struct object* k) {
  if (TYPE_SYMBOL == subject->type) {
    eval(car(object), env, make_add_symbol_continuation(subject, env, k));
  } else {
    env_add_symbol(env, car(subject), lambda(cdr(subject), object, env));
    resume(k, car(subject));
  }
}

#define KEYWORD(k) (is_true(eq(car(e), symbol_to_object(k))))

void eval(struct object* e, struct object* env, struct object* k)
{
  //  printf("eval: ");  print(e);
  switch (e->type)
    {
    case TYPE_PAIR:
      if KEYWORD("quote") resume(k, cadr(e)); 
      else if KEYWORD("if") eval(cadr(e), env, make_if_continuation(caddr(e), cadddr(e), env, k));
      else if KEYWORD("lambda") resume(k, lambda(cadr(e), cddr(e), env));
      else if KEYWORD("define") define(cadr(e), cddr(e), env, k);
      else if KEYWORD("ccc") eval(cadr(e),
				  env,
				  make_application_continuation(cons(k, NULL),
								env,
								k));
      /*
      else if KEYWORD("cons")
      else if KEYWORD("car")   car(eval(cadr(e), env), k);
      else if KEYWORD("cdr")   cdr(eval(cadr(e), env), k);
      else if KEYWORD("eq") eq(eval(cadr(e), env), eval(caddr(e), env), k);
      else if KEYWORD("mac") apply(k, e);
      else if KEYWORD("atom")  atom(eval(cadr(e), env), k);
      */
      else eval(car(e),
		env,
		make_application_continuation(cdr(e), env, k));

      break;

    case TYPE_SYMBOL:
      env_lookup(env, e, k);
      break;
    default:
      resume(k, e); // self quoting all
      break;
    }
}

// read

void read(struct object* e, struct object* env, struct object* k) {
  printf("$ ");
  int n = yyparse();
  resume(k, yylval);
}

// global environment

struct object* make_global_env() {
  struct object* env = malloc(sizeof(struct object));
  env->value.env.parent = NULL;
  load_built_ins(env);

  return env;
}

void banner() {
  printf("Jude Lisp v0.1\nCopyright 2016 Omar Shorbaji\n");
}

void ep(struct object* e, struct object* env, struct object* k) {
  eval(
}

void rep(struct object* e, struct object* env, struct object *k) {
  read(NULL, env, ep);
}

// REPL

int main(int argc, char** argv) {

  banner();
  
  struct object* global = make_global_env();
  struct object* print_continuation = make_built_in_continuation(print);
  
  while (1) {
    eval(read(), global, print_continuation);
  }
}
