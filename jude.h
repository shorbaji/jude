#ifndef JUDE_H
#define JUDE_H

#include "uthash.h"

struct expr;

struct cell {
  struct expr* car;
  struct expr* cdr;
};

union value {
  int number;
  char* symbol;
  int boolean;
  char* error;
  struct proc* procedure;
};
  
struct expr {
  int type;
  union {
    struct cell pair;
    union value atom;
  } value;
};

struct proc {
  struct env* env;
  struct expr* code;
  struct expr * variables;
  struct expr* (*fn) (struct expr* argv);
};

#define TYPE_NUMBER 1
#define TYPE_SYMBOL 2
#define TYPE_BOOLEAN 3
#define TYPE_PROCEDURE 7
#define TYPE_PAIR 8
#define TYPE_ERROR 32

extern struct expr * number_to_expr(char*);
extern struct expr * symbol_to_expr(char*);
extern struct expr * boolean_to_expr(int);
extern struct expr * cons(struct expr*, struct expr*);
extern struct expr * proc_to_expr(struct proc*);

struct entry {
  char* symbol;
  struct expr * value;
  UT_hash_handle hh;
};

struct env {
  struct entry* hash;
  struct env* parent;
};

struct expr* env_lookup(struct env* env, struct expr* symbol);
void env_add_symbol(struct env* env, struct expr* symbol, struct expr* value);
struct env* make_env(struct env* parent);

void print_datum(struct expr*);
void print(struct expr*);

struct expr* car(struct expr*);
struct expr* cdr(struct expr*);
struct expr* cadr(struct expr*);


#endif
