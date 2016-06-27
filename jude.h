#ifndef JUDE_H
#define JUDE_H

#include "uthash.h"

typedef int obj_type_t;

#define OBJ_T_PAIR    (obj_type_t) 0
//atoms must be > 128
#define OBJ_T_NUM     (obj_type_t) 128
#define OBJ_T_STR     (obj_type_t) 129
#define OBJ_T_BOOL    (obj_type_t) 130
#define OBJ_T_LAMBDA  (obj_type_t) 131
#define OBJ_T_PROC    (obj_type_t) 132
#define OBJ_T_SYM     (obj_type_t) 133

#define OBJ_T_ERR     (obj_type_t) 134
#define OBJ_T_CONT     (obj_type_t) 135

typedef struct obj obj_t;
typedef int num_t;
typedef char *str_t;
typedef int bool_t;
typedef struct sym sym_t;
typedef struct err {
  int id;
  char *msg;
  obj_t *data;
} err_t;

typedef obj_t quote_t;
typedef struct def def_t;
typedef struct lam lam_t;
typedef struct _if if_t;

typedef struct env env_t;
typedef struct cont cont_t;
typedef struct pair pair_t;
typedef struct proc proc_t;
typedef struct lambda lambda_t;

struct pair {
  obj_t *car;
  obj_t *cdr;
};
  
struct hash_entry {
  char* key;
  obj_t* val;
  UT_hash_handle hh;
};

struct env {
  struct hash_entry* hash;
  struct env* up;
};

struct def {
  obj_t *sym;
  obj_t *exp;
};

struct lam {
  obj_t *vars;
  obj_t *body;
};

struct _if {
  obj_t *pred;
  obj_t *cons;
  obj_t *alt;
};

typedef void (*resume_t) (obj_t *, obj_t *, env_t *, cont_t*);

struct cont {
  resume_t resume;
  obj_t *data;
  env_t *env;
  cont_t *k;
};

struct lambda {
  env_t *env;
  obj_t *vars;
  obj_t *body;
};

typedef union {
  obj_t *(*thunk) ();
  obj_t *(*unary) (obj_t *);
  obj_t *(*binary) (obj_t *, obj_t *);
  obj_t *(*ternary) (obj_t *, obj_t *, obj_t *);
} func_t;
  
typedef struct builtin {
  char *name;
  func_t func;
} builtin_t;

typedef int keyword_t;

#define KEYWORD_NULL 0
#define KEYWORD_QUOTE 1
#define KEYWORD_DEFINE 2
#define KEYWORD_IF 3
#define KEYWORD_LAMBDA 4
#define KEYWORD_CALLCC 5
#define KEYWORD_SET 6

struct sym {
  char *str;
  keyword_t keyword;
};

struct obj {
  obj_type_t type;
  union {
    num_t num;
    str_t str;
    bool_t bool;
    sym_t *sym;
    err_t *err;
    
    pair_t *pair;

    proc_t *proc;
    cont_t *cont;
  };
};

// a proc can be a built-in or lambda
// a proc has arity
// a proc has type
// if builtin then it has an fn

typedef int proc_type_t;

struct proc {
  int arity;
  void (*invoke) (obj_t *, obj_t *, env_t *, cont_t* t);
  char *name;
  union {
    builtin_t *builtin;
    lambda_t *lambda;
  };
};

obj_t *car(obj_t *obj);
obj_t *cdr(obj_t *obj);
obj_t *cadr(obj_t *obj);
obj_t *cddr(obj_t *obj);
obj_t *caddr(obj_t *obj);
obj_t *cadddr(obj_t *obj);
obj_t *cons(obj_t *a, obj_t *b);
void evlis(obj_t *ls, env_t *t, cont_t* k);


obj_t* make_num_obj(num_t num);
obj_t* make_bool_obj(bool_t bool);
obj_t* make_sym_obj(char *, keyword_t);
obj_t* make_err_obj(char *, obj_t *);
obj_t* make_lam_obj(obj_t *, obj_t*);
obj_t* make_def_obj(obj_t *, obj_t*);
obj_t* make_proc_obj(proc_t *);
obj_t* make_lambda_obj(lambda_t *);
obj_t* make_if_obj(obj_t*, obj_t *, obj_t *);
obj_t* make_quote_obj(obj_t*);

obj_t *print(obj_t *obj);
void print_obj(obj_t *obj);

void invoke_lambda(obj_t *, obj_t *, env_t*, cont_t *);
void invoke_builtin(obj_t *, obj_t *, env_t*, cont_t *);

obj_t *__plus(obj_t* e);
obj_t *__minus(obj_t* e);
obj_t *__times(obj_t* e);
obj_t *__divide(obj_t* e);

void env_add_sym(env_t *, char *, obj_t *);
 #endif
