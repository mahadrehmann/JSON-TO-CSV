/*  
 * 1. %code requires injects into parser.tab.h so AST is known when parsing the union.
 * 2. %{ … %} is the prologue for parser.tab.cpp, pulling in utilities and externs.
 */

%code requires {
  #include "ast.h"
  #include <utility>            // for std::pair
}

%{
  #include <cstdlib>
  #include <cstring>
  extern int yylex();            // lexer defined in lex.yy.cpp
  extern void yyerror(const char*);
%}

%define parse.error verbose

// semantic-value union
%union {
  double                                  num;
  char*                                   str;
  bool                                    boolean;
  AST::Node*                              node;
  AST::Members*                           members;
  AST::Elements*                          elements;
  std::pair<std::string, AST::Node*>*     pair;
}

// tokens with their value slots
%token <str>     T_STRING
%token <num>     T_NUMBER
%token <boolean> T_TRUE T_FALSE
%token           T_NULL
%token           T_LBRACE T_RBRACE T_LBRACK T_RBRACK T_COLON T_COMMA

// nonterminals
%type  <node>     json value object array
%type  <pair>     pair
%type  <members>  members
%type  <elements> elements

%start json

%%

json:
    value                   { AST::root = $1; }
  ;

value:
    T_STRING                { $$ = new AST::StringNode($1); free($1); }
  | T_NUMBER                { $$ = new AST::NumberNode($1); }
  | T_TRUE                  { $$ = new AST::BoolNode(true); }
  | T_FALSE                 { $$ = new AST::BoolNode(false); }
  | T_NULL                  { $$ = new AST::NullNode(); }
  | object                  { $$ = $1; }
  | array                   { $$ = $1; }
  ;

object:
    T_LBRACE members T_RBRACE  { $$ = new AST::ObjectNode(*$2); delete $2; }
  | T_LBRACE T_RBRACE          { $$ = new AST::ObjectNode(); }
  ;

members:
    pair                       { $$ = new AST::Members(); $$->push_back(*$1); delete $1; }
  | members T_COMMA pair       { $$ = $1; $$->push_back(*$3); delete $3; }
  ;

pair:
    T_STRING T_COLON value     { $$ = new std::pair<std::string, AST::Node*>(std::string($1), $3); free($1); }
  ;

array:
    T_LBRACK elements T_RBRACK { $$ = new AST::ArrayNode(*$2); delete $2; }
  | T_LBRACK T_RBRACK          { $$ = new AST::ArrayNode(); }
  ;

elements:
    value                      { $$ = new AST::Elements(); $$->push_back($1); }
  | elements T_COMMA value     { $$ = $1; $$->push_back($3); }
  ;

%%

// error-reporting function
void yyerror(const char* s) {
  fprintf(stderr, "Parse error: %s\n", s);
}
