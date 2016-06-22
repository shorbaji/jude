%{
  #include <stdio.h>
  #include "jude.h"
%}

%define api.value.type {  struct expr* }

%token SYMBOL NUMBER OPEN CLOSE BOOLEAN DOT

%start expr

%%


expr: datum {{ yylval=$$=$1; return; }}
;

datum: atom {{ $$ = $1; }}
  | list {{ $$ = $1; }}
;

atom: SYMBOL { $$ = $1; }
  | NUMBER { $$ = $1; }
  | BOOLEAN { $$ = $1; }
;

list: OPEN rest { $$ = $2; }
;

rest: datum CLOSE         { $$ = cons($1, (struct expr *) NULL); }
  | datum DOT datum CLOSE { $$ = cons($1, $3); }
  | datum rest            { $$ = cons($1, $2); }
;

%%

int yyerror(char *s) {
    fprintf(stderr, "error: %s", s);
}

