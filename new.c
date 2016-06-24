#include <stdio.h>
#include <stdlib.h>

typedef struct continuation continuation_t;
typedef struct object object_t;
typedef struct env env_t;

typedef void (*_builtin_t) (object_t *, env_t *, continuation_t *);

typedef int cont_type_t;

#define CONTINUATION_TYPE_BUILTIN (cont_type_t) 1
#define CONTINUATION_TYPE_COMPOSE (cont_type_t) 2

struct continuation {
  cont_type_t type;
  int min;
  int max;
  _builtin_t builtin;
  env_t *env;
  continuation_t *k;
  object_t *data;
};

typedef int object_type_t;

#define OBJECT_TYPE_NUMBER (object_type_t) 1
#define OBJECT_TYPE_CONTINUATION (object_type_t) 2
#define OBJECT_TYPE_PAIR (object_type_t) 8

typedef struct pair {
  object_t *car;
  object_t *cdr;
} pair_t;
  
struct object {
  object_type_t type;
  union  {
    int number;
    continuation_t *continuation;
    pair_t pair;
  } value;
};


void resume(continuation_t *, object_t * object);

char *number_to_str(int n) {
  char *str = malloc(20 * sizeof(char));

  sprintf(str, "%d", n);

  return str;
}
void print(object_t *object, env_t *env, continuation_t *k) {

  char *s;
  
  if (object == NULL)
    printf("print: null\n");
  else
    switch (object->type) {
    case OBJECT_TYPE_NUMBER:
      s = number_to_str(object->value.number); break;
    case OBJECT_TYPE_CONTINUATION:
      s = "<continuation>"; break;
    case OBJECT_TYPE_PAIR:
      s = "<pair>"; break;
    }
    printf("print: %s\n", s);

  resume(k, NULL);
}

void read(object_t *args, env_t *env, continuation_t *k) {
  resume(k, make_number_object(7));
}

continuation_t * make_primitive_continuation(char* symbol,
					     int min,
					     int max,
					     _builtin_t builtin,
					     env_t* env,
					     continuation_t* k) {
  continuation_t *c = malloc(sizeof(continuation_t));

  c->type = CONTINUATION_TYPE_BUILTIN;
  c->min = min;
  c->max = max;
  c->builtin = builtin;
  c->env = env;
  c->k = k;

  return c;
}

void invoke(continuation_t * fn, object_t *args, env_t* env, continuation_t* k) {
  switch (fn->type) {
  case CONTINUATION_TYPE_BUILTIN:
    fn->builtin(args, env, k); break;
  }
}

void resume(continuation_t *k, object_t *object) {
  if (k == NULL) {
    printf("exiting with null continuation\n");
    exit(0);
  }
  switch (k->type) {
  case CONTINUATION_TYPE_BUILTIN:
    invoke(k, object, k->env, k->k);
    break;
  }
}

void compose(object_t* object, env_t *env, continuation_t *k)
{
  
}

continuation_t *make_compose_continuation(continuation_t *a, continuation_t *b, env_t *env, continuation_t *k) {
  continuation_t *c;
  
  c->type = CONTINUATION_TYPE_COMPOSE;
  c->min = 1;
  c->max = 1;
  c->env = env;
  c->k = k;

  c->data = cons(make_continuation_object(a), make_continuation_object(b));

  return c;
}

env_t *genv;

int main(int argc, char **argv) {
  continuation_t* c_print = make_primitive_continuation("print", 0, 0, print, genv, NULL);
  continuation_t* c_read = make_primitive_continuation("read", 0, 0, read, genv, c_print);
  continuation_t* c_exit = make_primitive_continuation("exit", 0, 0, __exit, genv, NULL);
  
  //  c_read->k = c_read;

  resume(c_read, NULL);
}
