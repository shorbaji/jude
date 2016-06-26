
%{
#include "jude.h"
#include "jude.tab.h"
  

%}

delim              [ \t\n]
ws                 {delim}+
letter             [a-zA-Z]
initial            {letter}
subsequent         {letter}|{digit}
digit              [0-9]
operator           [\+\*\-\/]
id                 ({letter}|{operator})({letter}|{digit})*
number             {digit}+
dot                \.
true               #t
false              #f

%%

[ \t\n]+     {}
{number}     { yylval = make_num_obj(atoi(yytext)); return NUMBER; }
\(           { return OPEN; }
\)           { return CLOSE; }
{id}         { yylval = make_sym_obj(yytext); return SYMBOL; }
{true}       { yylval = make_bool_obj(1); return BOOLEAN; }
{false}      { yylval = make_bool_obj(0); return BOOLEAN; }
{dot}        { return DOT; }
