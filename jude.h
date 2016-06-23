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
  struct object* parent;
};

struct object* env_lookup(struct object *env, struct object* symbol);
void env_add_symbol(struct object *env, struct object* symbol, struct object* value);
struct object* make_env(struct object *parent);

struct pair {
  struct object* car;
  struct object* cdr;
};

union value {
  int number;
  int boolean;
  char* symbol;
  char* error;
  struct pair pair;
  struct env env;
  struct procedure *procedure;
};
  
struct object {
  int type;
  union value value;
};

struct procedure {
  struct object* parent;
  struct object* code;
  struct object* variables;
  struct object* (*fn) (struct object* argv);
};


#define TYPE_NUMBER 1
#define TYPE_SYMBOL 2
#define TYPE_BOOLEAN 3
#define TYPE_PROCEDURE 7
#define TYPE_PAIR 8
#define TYPE_ERROR 32

extern struct object * number_to_object(char *);
extern struct object * symbol_to_object(char *);
extern struct object * boolean_to_object(int);
extern struct object * procedure_to_object(struct procedure *);
extern struct object * error_to_object(char *);
extern struct object * cons(struct object*, struct object *);

void print_datum(struct object *);
void print(struct object *);

struct object* car(struct object *);
struct object* cdr(struct object *);
struct object* cadr(struct object *);
struct object* cddr(struct object *);
struct object* caddr(struct object *);


struct object* eval(struct object*, struct object *env);

#endif
