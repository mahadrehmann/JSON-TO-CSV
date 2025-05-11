%option noyywrap
%{
  #include "parser.tab.h"
  #include <cstdlib>
  #include <cstdio>
%}

DIGIT       [0-9]
HEXDIGIT    [0-9a-fA-F]
ESCAPE      \\(["\\/bfnrt]|u[0-9a-fA-F]{4})

%%

"{"             { return T_LBRACE; }
"}"             { return T_RBRACE; }
"\["            { return T_LBRACK; }
"\]"            { return T_RBRACK; }
":"             { return T_COLON; }
","             { return T_COMMA; }

\"({ESCAPE}|[^"\\])*\" {
    /* copy the literal, drop leading and trailing quote */
    int len = strlen(yytext);
    char* buf = (char*)malloc(len - 1);
    /* yytext = "\"Ali\"" → buf gets "Ali" + '\0' */
    memcpy(buf, yytext+1, len-2);
    buf[len-2] = '\0';
    yylval.str = buf;
    return T_STRING;
}

-?{DIGIT}+(\.{DIGIT}+)?([eE][+-]?{DIGIT}+)? {
                    yylval.num = strtod(yytext, nullptr);
                    return T_NUMBER;
                 }

"true"          { yylval.boolean = true; return T_TRUE; }
"false"         { yylval.boolean = false; return T_FALSE; }
"null"          { return T_NULL; }

[ \t\r\n]+      { /* skip whitespace */ }
.               {
                    fprintf(stderr, "Unexpected input: %s\n", yytext);
                    exit(1);
                }
%%

/* No yywrap() definition needed because of %option noyywrap */
