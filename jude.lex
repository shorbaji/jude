
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
{number}     { yylval = number_to_expr(yytext); return NUMBER; }
\(           { return OPEN; }
\)           { return CLOSE; }
{id}         { yylval = symbol_to_expr(yytext); return SYMBOL; }
{true}       { yylval = boolean_to_expr(1); return BOOLEAN; }
{false}      { yylval = boolean_to_expr(0); return BOOLEAN; }
{dot}        { return DOT; }
