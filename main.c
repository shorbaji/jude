#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "jude.h"
#include "jude.tab.h"

#include "uthash.h"

/* TODOS
 *
 * expose additional builtins car, cdr, cons, list
 * implement call/cc
 * implement tail recursion
 */



// builtins

proc_t *make_builtin_proc(char *name, func_t func, int arity)
{
  builtin_t *builtin = malloc(sizeof(builtin_t));
  builtin->func = func;
  builtin->name = malloc(sizeof(name));
  strcpy(builtin->name, name);
  
  proc_t *proc = malloc(sizeof(proc_t));

  proc->arity = arity;
  proc->invoke = invoke_builtin;
  proc->builtin = builtin;
  
  return proc;
}

proc_t **builtin_list(int *count) {
  *count = 7;
  proc_t **list = malloc(*count * sizeof(proc_t **));

  list[0] = make_builtin_proc("+", (func_t) __plus, -1);
  list[1] = make_builtin_proc("-", (func_t) __minus, -1);
  list[2] = make_builtin_proc("*", (func_t) __times, -1);
  list[3] = make_builtin_proc("/", (func_t) __divide, -1);
  list[4] = make_builtin_proc("car", (func_t) car, 1);
  list[5] = make_builtin_proc("cdr", (func_t) cdr, 1);
  list[6] = make_builtin_proc("cons", (func_t) cons, 2);
  
  /*
  list[0] = make_special("eval", (c_function_t) eval);
  list[1] = make_special("define", (c_function_t) __define);
  list[2] = make_special("lambda", (c_function_t) __lambda);
  list[3] = make_special("ccc", (c_function_t) __ccc);
  list[4] = make_special("if", (c_function_t) __if);
  list[5] = make_special("quote", (c_function_t) __quote);
  list[6] = make_special("begin", (c_function_t) __begin);
  list[7] = make_builtin("print", (c_function_t) print, 1);
  list[11] = make_builtin("read", (c_function_t) __read, 0);

  list[16] = make_builtin("length", (c_function_t) __length, 1);
  list[17] = make_special("list", (c_function_t) __list);
  */
  return list;
}


void register_builtins(env_t* env) {

  int count, i;
  proc_t **procs = builtin_list(&count);
  obj_t *sym;

  for (i=0; i<count; i++)
    {
      env_add_sym(env,
		  procs[i]->builtin->name,
		  make_proc_obj(procs[i]));
    }
}

obj_t* __plus(obj_t *e)
{
  int sum=0;
  obj_t* v;

  while (e!=NULL) {
    if (car(e)->type != OBJ_T_NUM) {
      return make_err_obj("not a number", car(e));
    } else {
      sum += car(e)->num;
      e = cdr(e);
    }
  }

  return make_num_obj(sum);
}

obj_t* __minus(obj_t *e)
{
  
  int sum=0;
  
  obj_t* v;

  if (e!=NULL) {
    if (car(e)->type == OBJ_T_NUM) {
      sum -= car(e)->num;
      e = cdr(e);
    }
  }
  
  while (e!=NULL) {
    if (car(e)->type != OBJ_T_NUM) {
      return make_err_obj("not a number", car(e));
    } else {
      sum += car(e)->num;
      e = cdr(e);
    }
  }

  return make_num_obj(sum);
}

obj_t* __times(obj_t *e)
{
  int product=1;
  obj_t* v;

  while (e!=NULL) {
    if (car(e)->type != OBJ_T_NUM) {
      return make_err_obj("not a number", car(e));
    } else {
      product *= car(e)->num;
      e = cdr(e);
    }
  }

  return make_num_obj(product);
}

obj_t* __divide(obj_t *e)
{
  
  int product=1;
  
  obj_t* v;

  if (e!=NULL) {
    if (car(e)->type == OBJ_T_NUM) {
      product = car(e)->num;
      e = cdr(e);
    }
  }
  
  while (e!=NULL) {
    if (car(e)->type != OBJ_T_NUM) {
      return make_err_obj("not a number", car(e));
    } else {
      product /= car(e)->num;
      e = cdr(e);
    }
  }

  return make_num_obj(product);
}


// hash

void env_add_sym(env_t * env, char* sym, obj_t* val)
{
  struct hash_entry * h_entry = malloc(sizeof(struct hash_entry));
  h_entry->key = sym;
  h_entry->val = val;
  HASH_ADD_STR(env->hash, key, h_entry);
}

obj_t* env_lookup(env_t* env, char* sym) {
  struct hash_entry *h_entry;

  while (env!=NULL)
    {
      HASH_FIND_STR(env->hash, sym, h_entry);
      if (h_entry == NULL)
	env = env->up;
      else
	return h_entry->val;
    }

  return make_err_obj("unknown symbol", make_sym_obj(sym, KEYWORD_NULL));
}

env_t* make_env(env_t* up)
{
  env_t *env = malloc(sizeof(env_t));

  env->hash = NULL;
  env->up = up;

  return env;
}

// constructors

obj_t* make_num_obj(num_t num)
{
  obj_t* obj = malloc(sizeof(obj));

  obj->type = OBJ_T_NUM;
  obj->num = num;

  return obj;
}

obj_t* make_sym_obj(char *sym, keyword_t keyword)
{
  obj_t* obj = malloc(sizeof(obj));

  obj->type = OBJ_T_SYM;
  obj->sym = malloc(sizeof(sym_t));
  obj->sym->str = malloc(strlen(sym));
  strcpy(obj->sym->str, sym);
  obj->sym->keyword = keyword;

  return obj;
}

obj_t *make_bool_obj(bool_t bool)
{
  obj_t* obj = malloc(sizeof(obj));

  obj->type = OBJ_T_BOOL;
  obj->bool = bool;

  return obj;
}

lambda_t *make_lambda(env_t *env, obj_t *vars, obj_t *body)
{
  lambda_t *l = malloc(sizeof(lambda_t));

  l->env = env;
  l->vars = vars;
  l->body = body;

  return l;
}

obj_t *make_lambda_obj(lambda_t *lambda)
{
  obj_t *obj = malloc(sizeof(obj));

  obj->type = OBJ_T_PROC;
  obj->proc = malloc(sizeof(proc_t));
  obj->proc->lambda = lambda;
  obj->proc->invoke = invoke_lambda;

  return obj;
}

obj_t *make_proc_obj(proc_t *proc)
{
 obj_t *obj = malloc(sizeof(obj_t));
 
 obj->type = OBJ_T_PROC;
 obj->proc = proc;

 return obj;
}
jmp_buf jmp_loop;
jmp_buf jmp_err;
err_t *global_err;

obj_t *make_err_obj(char *str, obj_t *data)
{
  obj_t *obj = malloc(sizeof(obj));

  obj->type = OBJ_T_ERR;
  obj->err = malloc(sizeof(err_t));

  obj->err->data = data;
  obj->err->msg = malloc(sizeof(str));

  strcpy(obj->err->msg, str);
  

  global_err = obj->err;
  longjmp(jmp_err, 1);


  // return obj;
}

cont_t *make_cont(resume_t resume, obj_t *data, env_t *env, cont_t *k)
{
  cont_t *c = malloc(sizeof(cont_t));

  c->resume = resume;
  c->data = data;
  c->env = env;
  c->k = k;

  return c;
}

int is_atom(obj_t *obj) { return (obj->type & 128) == 128; }
int is_true(obj_t *obj) { return !((OBJ_T_BOOL == obj->type) && (obj->bool == 0)); }
int is_sym(obj_t *obj) { return (obj->type == OBJ_T_SYM); }

#define resume(c, obj) c->resume(obj, c->data, c->env, c->k)

void eval(obj_t *e, env_t* env, cont_t *k);

#define BEGIN(data, env, k) eval(car(data), env, make_cont(res_begin, cdr(data), env, k))

void res_begin(obj_t *obj, obj_t* data, env_t* env, cont_t *k)
{
  if (NULL == data)
    resume(k, obj);
  else
    BEGIN(data, env, k);
}

void invoke_lambda(obj_t *obj, obj_t *args, env_t *env, cont_t *k)
{
  env_t *closure = make_env(obj->proc->lambda->env);
  obj_t *v = obj->proc->lambda->vars;
  obj_t *a = args;

  while (v != NULL) {
    env_add_sym(closure, car(v)->sym->str, car(a));
    v = cdr(v);
    a = cdr(a);
  }

  BEGIN(obj->proc->lambda->body, closure, k);
}

void invoke_builtin(obj_t *obj, obj_t *args, env_t *env, cont_t *k)
{

  obj_t *val;
  
  switch (obj->proc->arity) {
  case -1:
    val = obj->proc->builtin->func.unary(args);
    break;
  case 0:
    val = obj->proc->builtin->func.thunk();
    break;
  case 1:
    val = obj->proc->builtin->func.unary(car(args));
    break;
  case 2:
    val = obj->proc->builtin->func.binary(car(args), cadr(args));
    break;
  case 3:
    val = obj->proc->builtin->func.ternary(car(args), cadr(args), caddr(args));
    break;
  }

  resume(k, val);
}
void invoke(obj_t *obj, obj_t* args, env_t* env, cont_t* k)
{
  if (OBJ_T_CONT == obj->type)
    resume(obj->cont, car(args));
  else
    obj->proc->invoke(obj, args, env, k);
}

void res_def(obj_t *val, obj_t *obj, env_t *env, cont_t* k)
{
  env_add_sym(env, obj->sym->str, val);
  resume(k, obj);
}

void res_if(obj_t *bool, obj_t *obj, env_t *env, cont_t* k)
{
  if (is_true(bool))
    eval(car(obj), env, k);
  else if (NULL == cdr(obj))
    resume(k, NULL);
  else
    eval(cadr(obj), env, k); 
}

void res_invoke(obj_t *args, obj_t *obj, env_t *env, cont_t *k)
{
  obj->proc->invoke(obj, args, env, k);
}

void res_gather(obj_t* ls, obj_t* val, env_t *env, cont_t *k)
{
  resume(k, cons(val, ls));
}

void res_evlis(obj_t* val, obj_t *rest, env_t *env, cont_t *k)
{
  evlis(rest, env, make_cont(res_gather, val, env, k));
}

void evlis(obj_t *ls, env_t *env, cont_t* k)
{
  if (NULL==ls)
    resume(k, NULL);
  else
    eval(car(ls), env, make_cont(res_evlis, cdr(ls), env, k));
}

void res_apply(obj_t *proc, obj_t *ls, env_t *env, cont_t* k)
{
  // test if proc is actually a proc

  evlis(ls, env, make_cont(res_invoke, proc, env, k));
}

obj_t *car(obj_t *obj) { return obj->pair->car; }
obj_t *cdr(obj_t *obj) { return obj->pair->cdr; }
obj_t *cadr(obj_t *obj) { return car(cdr(obj)); }
obj_t *cdadr(obj_t *obj) { return cdr(car(cdr(obj))); }
obj_t *caddr(obj_t *obj) { return car(cdr(cdr(obj))); }
obj_t *caadr(obj_t *obj) { return car(car(cdr(obj))); }

obj_t *cddr(obj_t *obj) { return cdr(cdr(obj)); }

obj_t *cons(obj_t *a, obj_t *b)
{
  obj_t *obj = malloc(sizeof(obj_t));

  obj->type = OBJ_T_PAIR;
  obj->pair = malloc(sizeof(pair_t));
  obj->pair->car = a;
  obj->pair->cdr = b;

  return obj;
}

void eval(obj_t* e, env_t* env, cont_t* k)
{
  //  printf("eval ");
  //  print(e);
  
  lambda_t *l;
  obj_t *obj;
  cont_t *cont;
  
  if (is_atom(e)) {
    resume(k, is_sym(e) ? env_lookup(env, e->sym->str) : e);
  } else {
    if (OBJ_T_SYM == car(e)->type) {
      switch(car(e)->sym->keyword)  {
      case KEYWORD_DEFINE:
	if (is_atom(cadr(e))) {
	  cont = make_cont(res_def, cadr(e), env, k);
	  eval(caddr(e), env, cont);
	} else {
	  l = make_lambda(env, cdadr(e), cddr(e));
	  obj = make_lambda_obj(l);
	  cont = make_cont(res_def, caadr(e), env, k);
	  resume(cont, obj);
	}
	break;
      case KEYWORD_LAMBDA:
	l = make_lambda(env, cadr(e), cddr(e));
	obj = make_lambda_obj(l);
	resume(k, obj);;
	break;
      case KEYWORD_IF:
	eval(cadr(e), env, make_cont(res_if, cddr(e), env, k));
	break;
      case KEYWORD_QUOTE:
	resume(k, cdr(e));
	break;
      default:
	eval(car(e), env, make_cont(res_apply, cdr(e), env, k));
      }
    }
  }
}

void print_pair(obj_t *e) {
  printf("(");
  while (e!=NULL) {
    print_obj(car(e));
    if (cdr(e)!= NULL)
      if (cdr(e)->type != OBJ_T_PAIR) {
	printf(" . ");
	print_obj(cdr(e));
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

void print_obj(obj_t *obj) {
  if (obj == NULL) {
    printf("NULL");
  } else {
    switch (obj->type)
      {
      case OBJ_T_NUM:
	printf("%d", obj->num);
	break;
      case OBJ_T_SYM:
	printf("%s", obj->sym->str);
	break;
      case OBJ_T_BOOL:
	printf(obj->bool ? "#t" : "#f");
	break;
      case OBJ_T_CONT:
	printf( "<continuation>");
	break;
      case OBJ_T_PROC:
	printf( "<procedure>");
	break;
      case OBJ_T_PAIR:
	print_pair(obj);
	break;
      case OBJ_T_ERR:
	printf("error: %s ", obj->err->msg);
	print(obj->err->data);
	break;
      default:
	printf("<obj>");
      }
  }
}

obj_t *print(obj_t *obj) {
  print_obj(obj);
  printf("\n");
  return NULL;
}

obj_t* read() {
  yyparse();
  yylval;
}

int main(int argc, char **argv)
{

  env_t *genv = make_env(NULL);
  int n = 1;
  obj_t *num = make_num_obj(1);
  obj_t *gerr_obj = malloc(sizeof(obj_t));

  register_builtins(genv);

  gerr_obj->type = OBJ_T_ERR;
  
  void res_prn(obj_t *obj, obj_t*data, env_t *env, cont_t *k)
  {
    print(obj);
    ++n;
    longjmp(jmp_loop, n);
  }
  
  while(1) {
    free(num);
    num = make_num_obj(n);

    if (setjmp(jmp_err)) {
      gerr_obj->err = global_err;
      printf("exception ");
      print(gerr_obj);
    }

    setjmp(jmp_loop);
	       
	       
    printf("#%d> ", n);
    eval(read(), genv, make_cont(res_prn, NULL, genv, NULL));
  }
}

