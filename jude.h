#define CONT_STACK_SIZE 1024

typedef int num_t;
typedef char* sym_t;
typedef char* str_t;
typedef int bool_t;
typedef struct vector vector_t;

typedef struct obj obj_t;
typedef struct pair pair_t;
typedef struct cont cont_t;
typedef struct env env_t;
typedef struct proc proc_t;

typedef struct lambda lambda_t;

// obj

typedef int obj_type_t;

#define OBJ_T_NUM (obj_type_t) 1
#define OBJ_T_BOOL (obj_type_t) 2
#define OBJ_T_SYM (obj_type_t) 7
#define OBJ_T_PAIR (obj_type_t) 8

struct obj {
  obj_type_t type;
  union {
    num_t num;
    sym_t sym;
    bool_t bool;
    str_t *str;
    pair_t *pair;
    cont_t *cont;
    proc_t *proc;
    env_t *env;
  };
};

struct pair {
  obj_t *car;
  obj_t *cdr;
};

typedef void * cont_type_t;

struct cont {
  cont_type_t type;
  obj_t* data;
  env_t* env;
  int k;
};

struct lambda {
  obj_t *vars;
  obj_t *code;
  env_t *env;
};
      

typedef int proc_type_t;
typedef char* proc_name_t;

typedef obj_t *(*thunk_t) ();
typedef obj_t *(*unary_t) (obj_t *);
typedef obj_t *(*multary_t) (obj_t *);
typedef obj_t *(*binary_t) (obj_t *, obj_t *);
typedef obj_t *(*ternary_t) (obj_t *, obj_t *, obj_t *);
typedef obj_t *(*quadnary_t) (obj_t *, obj_t* , obj_t*, obj_t*);
  
struct proc {
  proc_type_t type;
  proc_name_t name;
  int arity;
  union {
    thunk_t thunk;
    multary_t multary;
    unary_t unary;
    binary_t binary;
    ternary_t ternary;
    quadnary_t quadnary;
    lambda_t *lambda;
  } ;
};


obj_t* make_num_obj(num_t);
obj_t* make_sym_obj(sym_t);
obj_t* make_bool_obj(bool_t);



obj_t *cons(obj_t*, obj_t*);
