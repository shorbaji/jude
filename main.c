#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include "jude.h"
#include "jude.tab.h"

// TODO
// - implement recursion
// - implement macros
// - implement a garbage collector
// - implement continuations

// built-in types

struct expr* proc_to_expr(struct proc* proc)
{
  struct expr* e = malloc(sizeof(struct expr));

  e->type = TYPE_PROCEDURE;
  e->value.atom.procedure = proc;

  return e;
}

struct expr* number_to_expr(char* s)
{
  struct expr* e = malloc(sizeof(struct expr));

  e->type = TYPE_NUMBER;
  e->value.atom.number = atoi(s);

  return e;
}

struct expr* symbol_to_expr(char *s)
{
  int l = strlen(s);
  char* symbol = (char *) malloc(sizeof(char)*strlen(s));
  struct expr* e = malloc(sizeof(struct expr));

  strncpy(symbol, s, l);
  e->type = TYPE_SYMBOL;
  e->value.atom.symbol = symbol;

  return e;
}

struct expr* boolean_to_expr(int b)
{
  struct expr* e = malloc(sizeof(struct expr));
  e->type = TYPE_BOOLEAN;
  e->value.atom.boolean = b;

  return e;
}

struct expr* error_to_expr(char *error)
{
  struct expr* e = malloc(sizeof(struct expr));
  e->type = TYPE_ERROR;
  e->value.atom.error = malloc(sizeof(char)*strlen(error));
  strncpy(e->value.atom.error, error, strlen(error));

  return e;
}

int is_error(struct expr *e) {
  return e->type == TYPE_ERROR;
}

int is_pair(struct expr *e) {
  return e->type == TYPE_PAIR;
}

int is_true(struct expr *e) {
  if (e->type != TYPE_BOOLEAN)
    return 1;
  else if (e->value.atom.boolean!=0)
    return 1;
  else
    return 0;
}


// environment

void env_add_symbol(struct env* env, struct expr* symbol, struct expr* value)
{
  struct entry * e = malloc(sizeof(struct entry));
  e->symbol = symbol->value.atom.symbol;
  e->value = value;
  HASH_ADD_STR(env->hash, symbol, e);
}

struct expr* env_lookup(struct env* env, struct expr* symbol) {
  struct entry *e;

  HASH_FIND_STR(env->hash, symbol->value.atom.symbol, e);

  if (e == NULL)
    if (env->parent == NULL)
      return error_to_expr("unknown symbol");
    else
      return env_lookup(env->parent, symbol);
			
  else
    return e->value;
}

struct env *make_env(struct env* parent) {
  struct env *env = malloc(sizeof(struct env));

  env->hash = NULL;
  env->parent = parent;

  return env;
}

// print

void print_pair(struct expr *e) {
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

void print_datum(struct expr *e) {
  switch (e->type) {
    case TYPE_NUMBER:
      printf("%d", e->value.atom.number); break;
    case TYPE_SYMBOL:
      printf("%s", e->value.atom.symbol); break;
    case TYPE_PROCEDURE:
      printf("<procedure>"); break;
    case TYPE_PAIR:
      print_pair(e); break;
    case TYPE_ERROR:
      printf("error: %s", e->value.atom.error); break;
  }
}

void print(struct expr* e) {
  print_datum(e);
  printf("\n");
}

// core forms atom, eq, cons, car, cdr
// note: if and quote are implemented "inline" within eval

struct expr* atom(struct expr *e)
{
  if (is_error(e)) return e;
  return boolean_to_expr(e->type != TYPE_PAIR);
}

struct expr* eq(struct expr* a, struct expr* b)
{
  if (is_error(a)) return (a);
  if (is_error(b)) return (b);  
  return
    boolean_to_expr((a->type == b->type) &&
		    (((a->type == TYPE_NUMBER) && (a->value.atom.number == b->value.atom.number)) ||
		     ((a->type == TYPE_SYMBOL) && !strcmp(a->value.atom.symbol, b->value.atom.symbol)) ||
		     ((a->type == TYPE_PAIR) && (a==b))));
}

struct expr* cons(struct expr* car, struct expr* cdr)
{
  struct expr* e = malloc(sizeof(struct expr*));

  e->type = TYPE_PAIR;
  e->value.pair.car = car;
  e->value.pair.cdr = cdr;

  return e;
}

struct expr* car(struct expr *e) {
  return e->value.pair.car;
}

struct expr* cdr(struct expr *e) {
  return e->value.pair.cdr;
}

// helper functions for eval

struct expr* cddr(struct expr *e) { return cdr(cdr(e)); }
struct expr* cadr(struct expr *e) { return car(cdr(e)); }
struct expr* caddr(struct expr *e) { return car(cdr(cdr(e))); }
struct expr* cadddr(struct expr *e) { return car(cdr(cdr(cdr(e)))); }

struct expr* eval(struct expr*, struct env* env);

struct expr* evlis(struct expr* e, struct env* env)
{

  struct expr* v;

  if (e==NULL)
    return NULL;
  else
    {
      if (is_error(e)) return e;
      if (is_true(atom(e))) {
	return eval(e, env);
      } else {
	return cons(eval(car(e), env), evlis(cdr(e), env));
      }
    }
}

// eval
#define KEYWORD(k) (is_true(eq(car(e), symbol_to_expr(k))))

struct expr* invoke(struct proc* p, struct expr* e) {
  struct expr* vars = p->variables;
  struct env* env = make_env(p->parent); 

  while (vars != NULL) {
    env_add_symbol(env, car(vars), car(e));
    vars = cdr(vars);
    e = cdr(e);
  }

  struct expr* code = p->code;
  struct expr* result = NULL;
  while (code != NULL) {
    result = eval(car(code), env);
    code = cdr(code);
  }
  return result;
}

struct expr* apply(struct proc* proc, struct expr* list) {
  if (is_error(list))
    return list;
  if (proc->fn!= NULL)
    return proc->fn(list);
  else
    return invoke(proc, list);
}

struct expr* make_procedure(struct expr* e, struct env* parent) {
  struct proc* proc = malloc(sizeof(struct proc));

  proc->variables = car(e);
  proc->code = cdr(e);
  proc->fn = NULL;
  proc->parent = parent;

  return proc_to_expr(proc);
}

struct expr* eval(struct expr* e, struct env* env)
{
  //  printf("eval: ");
  //  print(e);
  struct expr* result;
  
  if (is_true (atom(e))) {
      if (e->type == TYPE_SYMBOL) {
	struct expr* v = env_lookup(env, e);
	if (v==NULL)
	  {
	    result= error_to_expr("symbol not found\n"); 
	  }
	else
	  result = v;
      } else {
	result = e; // self quoting all
      }
  } else {
    if KEYWORD("cons")       result = cons(eval(cadr(e), env), eval(caddr(e), env));
    else if KEYWORD("car")   result = car(eval(cadr(e), env));
    else if KEYWORD("cdr")   result = cdr(eval(cadr(e), env));
    else if KEYWORD("atom")  result = atom(eval(cadr(e), env));
    else if KEYWORD("eq")    result = eq(eval(cadr(e), env), eval(caddr(e), env));
    else if KEYWORD("quote") result = cadr(e);
    else if KEYWORD("if")  {
	if (is_true(eval(cadr(e), env))) 
	  result = eval(caddr(e), env);
	else
	  result = eval(cadddr(e), env);
    } else if KEYWORD("define") {
	env_add_symbol(env, cadr(e), eval(caddr(e), env));
	result = e;
    } else if KEYWORD("lambda") {
	result = make_procedure(cdr(e), env);
    } else if KEYWORD("mac") {
	result = e;
    } else {
      struct expr* v = eval(car(e), env);
      if (v->type != TYPE_PROCEDURE)
	result = error_to_expr("not a procedure. can't invoke");
      else
	result = apply(v->value.atom.procedure, evlis(cdr(e), env));
    }
  }

  //  printf("result: ");
  //  print(result);
  return result;
}

// read

struct expr* read() {
  printf("$ ");
  int n = yyparse();
  return yylval;
}

// built-in procedures

struct expr* _plus_fn(struct expr *e)
{
  char s[200];
  int sum=0;
  struct expr* v;

  while (e!=NULL) {
    if (car(e)->type != TYPE_NUMBER) {
      return error_to_expr("not a number");
    } else {
      sum += car(e)->value.atom.number;
      e = cdr(e);
    }
  }

  sprintf(s, "%d", sum);
  return number_to_expr(s);
}

struct expr* _minus_fn(struct expr *e)
{
  char s[200];
  int sum=0;
  struct expr* v;

  if (e!=NULL) {
    if (car(e)->type != TYPE_NUMBER)
	return error_to_expr("not a number");
    else {
      sum += car(e)->value.atom.number;
      e = cdr(e);
    }

    while (e!=NULL) {
      if (car(e)->type != TYPE_NUMBER) {
	return error_to_expr("not a number");
      } else {
	sum -= car(e)->value.atom.number;
	e = cdr(e);
      }
    }
  }

  sprintf(s, "%d", sum);
  return number_to_expr(s);
}

struct expr* _times_fn(struct expr *e)
{
  char s[200];
  int product=1;
  struct expr* v;

  while (e!=NULL) {
    if (car(e)->type != TYPE_NUMBER) {
      return error_to_expr("not a number");
    } else {
      product *= car(e)->value.atom.number;
      e = cdr(e);
    }
  }

  sprintf(s, "%d", product);
  return number_to_expr(s);
}

struct expr* _divide_fn(struct expr *e)
{
  char s[200];
  int product=1;
  struct expr* v;

  if ((e!=NULL) &&
      (car(e)!=NULL) &&
      (cdr(e)!=NULL) &&
      (cadr(e) != NULL) &&
      (TYPE_NUMBER == car(e)->type) &&
      (TYPE_NUMBER == cadr(e)->type))
    product = car(e)->value.atom.number/cadr(e)->value.atom.number;
  else
    return error_to_expr("not a number");

  sprintf(s, "%d", product);
  return number_to_expr(s);
}

struct proc* make_builtin_proc(struct expr* (* fn) (struct expr *e)) {
  struct proc* proc = malloc(sizeof(struct proc));
  proc->fn = fn;
  proc->code = NULL;
  proc->variables = NULL;

  return proc;  
}
  
// global environment

struct env* make_global_env() {
  struct env* env = malloc(sizeof(struct env));
  env->parent = NULL;

  env_add_symbol(env, symbol_to_expr("+"), proc_to_expr(make_builtin_proc(_plus_fn)));
  env_add_symbol(env, symbol_to_expr("-"), proc_to_expr(make_builtin_proc(_minus_fn)));
  env_add_symbol(env, symbol_to_expr("*"), proc_to_expr(make_builtin_proc(_times_fn)));
  env_add_symbol(env, symbol_to_expr("/"), proc_to_expr(make_builtin_proc(_divide_fn)));

  return env;
}

void banner() {
  printf("Jude Lisp v0.1\nCopyright 2016 Omar Shorbaji\n");

}
// REPL
int main(int argc, char** argv) {
  banner();
  struct env* global = make_global_env();
  while (1) {
    print(eval(read(), global));
  }
}
