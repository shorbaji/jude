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

typedef void (*invoke_t)(procedure_t *, object_t *, env_t *, continuation_t *);

struct builtin {
  char *name;
  c_function_t function;
  invoke_t invoke;
};

struct lambda {
  env_t * env;
  object_t *vars;
  object_t *code;
};


#define PROCEDURE_TYPE_BUILTIN 1
#define PROCEDURE_TYPE_LAMBDA 2
#define PROCEDURE_TYPE_SPECIAL 3


typedef int procedure_type_t;


struct procedure {
  procedure_type_t type;
  int arity;
  builtin_t *builtin;
  lambda_t *lambda;
};

typedef int object_type_t;

#define OBJECT_TYPE_NUMBER (object_type_t) 1
#define OBJECT_TYPE_CONTINUATION (object_type_t) 2
#define OBJECT_TYPE_SYMBOL 3
#define OBJECT_TYPE_BOOLEAN 4
#define OBJECT_TYPE_PROCEDURE 5
#define OBJECT_TYPE_PAIR (object_type_t) 8
#define OBJECT_TYPE_ERROR 32


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

object_t *make_number_object(int);
object_t *make_symbol_object(char *);
object_t *make_boolean_object(int);
object_t *make_error_object(char *);
object_t *make_procedure_object(procedure_t *);


object_t* cons(object_t *, object_t *);

void resume(continuation_t *, object_t * object);
void eval(object_t *, env_t *env, continuation_t *);

void invoke(object_t *, object_t *, env_t *env, continuation_t* k);
void invoke_thunk(procedure_t *, object_t *, env_t *env, continuation_t* k);
void invoke_unary(procedure_t *, object_t *, env_t *env, continuation_t* k);
void invoke_binary(procedure_t *, object_t *, env_t *env, continuation_t* k);
void invoke_ternary(procedure_t *, object_t *, env_t *env, continuation_t* k);
void invoke_four_ary(procedure_t *, object_t *, env_t *env, continuation_t* k);

void invoke_builtin(procedure_t *, object_t *, env_t *env, continuation_t* k);
void invoke_special(procedure_t *, object_t *, env_t *env, continuation_t* k);
void invoke_with_list(procedure_t *, object_t *, env_t *env, continuation_t* k);

object_t *car(object_t *);
object_t *cdr(object_t *);
object_t *cadr(object_t *);
object_t *cddr(object_t *);
object_t *caddr(object_t *);
object_t *cdddr(object_t *);
object_t *cadddr(object_t *);
object_t *cddddr(object_t *);


#endif
