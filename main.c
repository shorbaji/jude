#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include "jude.h"
#include "jude.tab.h"

// TODO
// - unquote, quasiquote, comma_at
// - syntax for ' ` , ,@
// - macros
// - tail recursion
// - continuations
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

struct object* env_lookup(struct object* env, struct object* symbol) {
  struct hash_entry *h_entry;

  HASH_FIND_STR(env->value.env.hash, symbol->value.symbol, h_entry);

  if (h_entry == NULL)
    if (env->value.env.parent == NULL)
      return error_to_object("unknown symbol");
    else
      return env_lookup(env->value.env.parent, symbol);
			
  else
    return h_entry->value;
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

struct object * lambda(struct object *vars, struct object* code, struct object* env) {
  struct procedure* p = malloc(sizeof(struct procedure));

  p->variables = vars;
  p->code = code;
  p->fn = NULL;
  p->parent = env;

  return procedure_to_object(p);
}

struct object * define(struct object *subject, struct object* object, struct object* env) {
  if (TYPE_SYMBOL == subject->type) {
    env_add_symbol(env, subject, eval(car(object), env));
    return subject;
  } else {
    env_add_symbol(env, car(subject), lambda(cdr(subject), object, env));
    return car(subject);
  }
}

// built-in forms for eval: quote, if, atom, eq, cons, car, cdr

struct object *quote(struct object *obj) {
  return obj;
}

struct object * _if(struct object *e, struct object* env) {
  is_true(eval(car(e), env)) ? eval(cadr(e), env) : eval(caddr(e), env);
}

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

struct object* evlis(struct object* e, struct object *env)
{
  struct object* v;

  if (e==NULL)
    return NULL;
  else
    {
      if (is_error(e))
	return e;
      
      if (is_true(atom(e))) {
	return eval(e, env);
      } else {
	return cons(eval(car(e), env), evlis(cdr(e), env));
      }
    }
}

// apply

struct object * invoke(struct object *obj, struct object *l) {
  struct procedure *p = obj->value.procedure;
  
  if (p->fn!= NULL)                     // if built-in procedure just run it
    {
      return p->fn(l);
    }
  
  struct object *vars = p->variables;
  struct object *env = make_env(p->parent);            // create a new env
  
  while (vars != NULL) {                // bind variables
    env_add_symbol(env, car(vars), car(l));
    vars = cdr(vars);
    l = cdr(l);
  }
  
  struct object* code = p->code;
  struct object* result = NULL;
  
  while (code != NULL) {
    result = eval(car(code), env);
    code = cdr(code);
  }

  return result;
}

struct object* apply(struct object* obj, struct object* l, struct object *env) {
  struct object *result;

  switch (obj->type) {
  case TYPE_ERROR:
    return obj;
  case TYPE_PROCEDURE: // is a procedure so evaluate args and invoke
    l = evlis(l, env);    
    return is_error(l) ? l : invoke(obj, l);
  default:
    return error_to_object("not a procedure. cannot invoke");
  }
}

// eval

#define KEYWORD(k) (is_true(eq(car(e), symbol_to_object(k))))

struct object* eval(struct object* e, struct object* env)
{
  //  printf("eval: ");
  //  print(e);
  struct object* result=NULL;
  struct object* v;

  switch (e->type)
    {
    case TYPE_PAIR:
      if KEYWORD("quote") result = quote(cadr(e));
      else if KEYWORD("if") result = _if(cdr(e), env); 
      else if KEYWORD("lambda") result = lambda(cadr(e), cddr(e), env);
      else if KEYWORD("define") result = define(cadr(e), cddr(e), env);
      else if KEYWORD("eq") result = eq(eval(cadr(e), env), eval(caddr(e), env));
      else if KEYWORD("mac") result = e;
      else if KEYWORD("cons")  result = cons(eval(cadr(e), env), eval(caddr(e), env));
      else if KEYWORD("car")   result = car(eval(cadr(e), env));
      else if KEYWORD("cdr")   result = cdr(eval(cadr(e), env));
      else if KEYWORD("atom")  result = atom(eval(cadr(e), env));
      else return apply(eval(car(e), env), cdr(e), env);

      break;
    case TYPE_SYMBOL:
	v = env_lookup(env, e);
	if (v==NULL)
	  {
	    result= error_to_object("symbol not found\n"); 
	  }
	else
	  result = v;
	break;
    default:
      result = e; // self quoting all
      break;
    }
  //  printf("result: ");
  //  print(result);
  return result;
}

// read

struct object* read() {
  printf("$ ");
  int n = yyparse();
  return yylval;
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
// REPL
int main(int argc, char** argv) {

  banner();
  
  struct object* global = make_global_env();
  
  while (1) {
    print(eval(read(), global));
  }
}
