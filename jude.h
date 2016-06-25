#ifndef JUDE_H
#define JUDE_H

#include "uthash.h"

struct object;

struct hash_entry {
  char* key;
  struct object* value;
  UT_hash_handle hh;
};

struct env {
  struct hash_entry* hash;
  struct env* parent;
};

typedef struct continuation continuation_t;
typedef struct procedure procedure_t;
typedef struct lambda lambda_t;
typedef struct builtin builtin_t;
typedef struct object object_t;
typedef struct env env_t;


typedef void (*resume_t) (continuation_t*, object_t*);

struct continuation {
  resume_t resume;
  object_t *data;
  env_t *env;
  continuation_t *k;
};

typedef object_t * (*thunk_t)();
typedef object_t * (*unary_t)(object_t *);
typedef object_t * (*binary_t) (object_t *, object_t *);
typedef object_t * (*ternary_t) (object_t *, object_t *, object_t *);
typedef object_t * (*four_ary_t) (object_t *, object_t *, object_t *, object_t *);

typedef void (*special_t) (object_t *, env_t *, continuation_t *);

typedef  union {
  thunk_t thunk;
  unary_t unary;
  binary_t binary;
  ternary_t ternary;
  four_ary_t four_ary;
  special_t special;
  
} c_function_t;

struct builtin {
  char *name;
  c_function_t function;
};

struct lambda {
  env_t * env;
  object_t *vars;
  object_t *code;
};

typedef void (*invoke_t) (object_t*, object_t*, env_t*, continuation_t*);

struct procedure {
  int arity;
  builtin_t *builtin;
  lambda_t *lambda;
  void (*invoke) (object_t*, object_t*, env_t*, continuation_t*);
};

typedef int object_type_t;

#define OBJECT_TYPE_NUMBER (object_type_t) 1
#define OBJECT_TYPE_CONTINUATION (object_type_t) 2
#define OBJECT_TYPE_SYMBOL 3
#define OBJECT_TYPE_BOOLEAN 4
#define OBJECT_TYPE_PROCEDURE 5
#define OBJECT_TYPE_PAIR (object_type_t) 8
#define OBJECT_TYPE_ERROR 15


typedef struct pair {
  object_t *car;
  object_t *cdr;
} pair_t;
  
struct object {
  object_type_t type;
  union  {
    int number;
    int boolean;
    char* symbol;
    continuation_t *continuation;
    procedure_t *procedure;
    pair_t pair;
    char* error;
  } value;
};

void print_object(object_t *);

procedure_t *make_special_procedure(char *, c_function_t);
procedure_t *make_builtin_procedure(char *, c_function_t, int);

object_t *make_number_object(int);
object_t *make_symbol_object(char *);
object_t *make_boolean_object(int);
object_t *make_error_object(char *);
object_t *make_procedure_object(procedure_t *);

typedef void (*resume_t) (continuation_t *, object_t* );

void resume(continuation_t *, object_t * object);
void resume_builtin_invoke(continuation_t *, object_t * object);

void eval(object_t *, env_t *env, continuation_t *);


void invoke_with_list(object_t *, object_t *, env_t *env, continuation_t* k);
void invoke_thunk(object_t *, object_t *, env_t *env, continuation_t* k);
void invoke_unary(object_t *, object_t *, env_t *env, continuation_t* k);
void invoke_binary(object_t *, object_t *, env_t *env, continuation_t* k);
void invoke_ternary(object_t *, object_t *, env_t *env, continuation_t* k);
void invoke_four_ary(object_t *, object_t *, env_t *env, continuation_t* k);

void invoke(object_t *, object_t *, env_t *env, continuation_t* k);
void invoke_lambda(object_t *, object_t *, env_t *env, continuation_t* k);
void invoke_builtin(object_t *, object_t *, env_t *env, continuation_t* k);
void invoke_special(object_t *, object_t *, env_t *env, continuation_t* k);

object_t* cons(object_t *, object_t *);
object_t *car(object_t *);
object_t *cdr(object_t *);
object_t *cadr(object_t *);
object_t *cddr(object_t *);
object_t *caddr(object_t *);
object_t *cdddr(object_t *);
object_t *cadddr(object_t *);
object_t *cddddr(object_t *);

object_t *print(object_t *);
object_t *__read();
object_t *__plus(object_t *);
object_t *__minus(object_t *);
object_t *__times(object_t *);
object_t *__divide(object_t *);

object_t *__length(object_t *);

void __quote (object_t *, env_t *, continuation_t *);
void __lambda (object_t *, env_t *, continuation_t *);
void __begin (object_t *, env_t *, continuation_t *);
void __ccc (object_t *, env_t *, continuation_t *);
void __if (object_t *, env_t *, continuation_t *);
void __define (object_t *, env_t *, continuation_t *);
void __eval_sequence (object_t *, env_t *, continuation_t *);

#endif
