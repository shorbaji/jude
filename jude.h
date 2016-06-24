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


typedef int invocable_type_t;

#define INVOCABLE_TYPE_CONTINUATION (invocable_type_t) 1
#define INVOCABLE_TYPE_BUILTIN (invocable_type_t) 2
#define INVOCABLE_TYPE_LAMBDA (invocable_type_t) 3

typedef union {
  int type;
  continuation_t *continuation;
  procedure_t *procedure;
} invocable;
  
struct continuation {
  void (*resume) (continuation_t*, object_t*);
  object_t *data;
  env_t *env;
  continuation_t *k;
};

typedef  union {
  object_t *(*thunk) ();
  object_t *(*unary) (object_t *);
  object_t *(*binary) (object_t *, object_t *);
  object_t *(*ternary) (object_t *, object_t *, object_t *);
  object_t *(*four_ary) (object_t *, object_t *, object_t *, object_t *);
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


#define PROCEDURE_TYPE_BUILTIN 1
#define PROCEDURE_TYPE_LAMBDA 2

typedef int procedure_type_t;

struct procedure {
  procedure_type_t type;
  int min;
  int max;
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

object_t* cons(object_t *, object_t *);

void resume(continuation_t *, object_t * object);

object_t *car(object_t *);
object_t *cdr(object_t *);
object_t *cadr(object_t *);
object_t *cddr(object_t *);
object_t *caddr(object_t *);
object_t *cdddr(object_t *);
object_t *cadddr(object_t *);
object_t *cddddr(object_t *);


#endif
