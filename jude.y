%{
  #include <stdio.h>
  #include "jude.h"
%}

%define api.value.type {  obj_t * }

%token SYMBOL NUMBER OPEN CLOSE BOOLEAN DOT DEFINE LAMBDA IF QUOTE

%start start

%%


start: expr {{ yylval = $$ = $1; return; }}

expr: datum {{ $$=$1; }}
;

/*
quote: OPEN QUOTE quoted_datum CLOSE {{ $$ = make_quote_obj($3); }}

quoted_datum: quoted_atom
  | quoted_list
;

quoted_atom: atom {{ $$ = $1; }}
| DEFINE  {{ $$ = $1; }}
| IF    {{ $$ = $1; }}
| QUOTE {{ $$ = $1; }}
| LAMBDA {{ $$ = $1; }}
;

quoted_list:
| quoted_atom quoted_list {{ $$ = cons($1, $2); }}
;

lambda : OPEN LAMBDA vars exprs CLOSE {{ $$ = make_lam_obj($3, $4); }}
;


exprs: expr     {{ $$ = cons($1, NULL); }}
  | expr exprs {{ $$ = cons($1, $2); }}
;

vars: OPEN varlist CLOSE {{ $$ = $2; }}
;

varlist:                {{ $$ = NULL; }}
  | SYMBOL varlist        {{ $$ = cons($1, $2); }}
;

define: OPEN DEFINE SYMBOL expr CLOSE {{ $$ = make_def_obj($3, $4); }}
  | OPEN DEFINE OPEN SYMBOL varlist CLOSE exprs CLOSE {{ $$ = make_def_obj($4, make_lam_obj($5, $7)); }}
;

if_stmt: OPEN IF pred cons alt CLOSE {{ $$ = make_if_obj($3, $4, $5); }}
;

pred : expr {{ $$ = $1; }}
;

cons : expr  {{ $$ = $1; }}
;

alt : {{ $$ = NULL; }}
| expr {{ $$ = $1; }}
 ;
*/
datum: atom {{ $$ = $1; }}
  | list {{ $$ = $1; }}
;

atom: BOOLEAN { $$ = $1; }
| NUMBER { $$ = $1; }
| SYMBOL { $$ = $1; }
;

list: OPEN rest { $$ = $2; }
;

rest: CLOSE               { $$ = (obj_t *) NULL; }
  | datum DOT datum CLOSE { $$ = cons($1, $3); }
  | datum rest            { $$ = cons($1, $2); }
;

%%

int yyerror(char *s) {
    fprintf(stderr, "error: %s", s);
}

