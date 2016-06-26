#include <stdio.h>
#include "new.h"

int main(int argc, char **argv)
{
  cont_t st_cont[CONT_STACK_SIZE];

  int cont_ptr = 0;

  st_cont[0] = { .type = &&__read, .data = NULL, .env = NULL, k = 1 };
  st_cont[1] = { .type = &&__eval, .data = NULL, .env = NULL, k = 2 };
  st_cont[2] = { .type = &&__print, .data = NULL, .env = NULL, k = 0 };


  reg_resume_k = 0;
  reg_resume_obj = NULL;
  goto __resume;

 
 __resume:
  goto st_cont[resume_k].type
  ;


 __resume_print:
   ;

 __resume_eval:
   ;

 __resume_read:
   int n = yyparse();


}


  
