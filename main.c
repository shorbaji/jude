#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "jude.h"
#include "jude.tab.h"

#define RESUME(k, o) reg_resume_k = k; reg_resume_obj = o; goto __resume;
#define EVAL(e, env, k) reg_eval_e =e; reg_eval_env = env; reg_eval_k = k; goto __eval;

obj_t *make_num_obj(num_t num)
{
  obj_t *obj = malloc(sizeof(obj_t));

  obj->type = OBJ_T_NUM;
  obj->num = num;

  return obj;
}

obj_t *make_sym_obj(sym_t sym)
{
  obj_t* obj = malloc(sizeof(obj_t));

  obj->type = OBJ_T_SYM;
  obj->sym = (sym_t) malloc(sizeof(sym));
  strcpy(obj->sym, sym);

  return obj;
}

obj_t *make_bool_obj(bool_t bool)
{
  obj_t* obj = malloc(sizeof(obj_t));

  obj->type = OBJ_T_BOOL;
  obj->bool = bool;

  return obj;
}

obj_t *cons(obj_t *a, obj_t *b)
{
  obj_t * obj = malloc(sizeof(obj_t));

  obj->type = OBJ_T_PAIR;
  obj->pair = malloc(sizeof(pair_t));
  obj->pair->car = a;
  obj->pair->cdr = b;

  return obj;
}


void invoke(inv_t* inv, obj_t* args, env_t* env, cont_t* k)
{
  obj_t *r;
  
  if (tail_call(inv, k))
    r = inv.apply(args, env, RETURN);
  
		
}

int main(int argc, char **argv)
{

  
  call(rep, NULL, genv, make_cont(rep));
}


  
