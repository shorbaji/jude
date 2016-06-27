
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

quote        { yylval = make_sym_obj("quote", KEYWORD_QUOTE); return SYMBOL; }
if           { yylval = make_sym_obj("if", KEYWORD_IF);  return SYMBOL; }
define       { yylval = make_sym_obj("define", KEYWORD_DEFINE); return SYMBOL; }
lambda       { yylval = make_sym_obj("lambda", KEYWORD_LAMBDA); return SYMBOL; }
{id}         { yylval = make_sym_obj(yytext, KEYWORD_NULL); return SYMBOL; }
[ \t\n]+     {}
{number}     { yylval = make_num_obj(atoi(yytext)); return NUMBER; }
\(           { return OPEN; }
\)           { return CLOSE; }
{true}       { yylval = make_bool_obj(1); return BOOLEAN; }
{false}      { yylval = make_bool_obj(0); return BOOLEAN; }
{dot}        { return DOT; }
