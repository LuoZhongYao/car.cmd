%{
#include "ast.h"
#include "cmd.tab.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#define L_LOGD  //LOGD
int yycolumn = 1;
static char* string(void);
void yyerror(char const *); 
void _comment(void);
extern const char *filename;
#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;\
    yylloc.first_column = yycolumn;yylloc.last_column = yycolumn + yyleng +1;\
    yycolumn += yyleng;
%}

%x comment
%option yylineno

ID  [_a-zA-Z][_a-zA-Z0-9]*
D   [0-9]
H   [a-fA-F0-9]
IS  ((u|U)|(u|U)?(l|L|ll|LL)|(l|L|ll|LL)(u|U))
FS  (f|F|l|L)


%%
";"[^\n]*               { /* comment */ }
"//"[^\n]*              { /* comment */ }
"/*"                    { _comment(); }
"{"                     { L_LOGD("%c",yytext[0]); return '{'; }     
"}"                     { L_LOGD("%c",yytext[0]); return '}'; }     
"["                     { L_LOGD("%c",yytext[0]); return '['; }     
"]"                     { L_LOGD("%c",yytext[0]); return ']'; }     
"<"                     { L_LOGD("%c",yytext[0]); return '<'; }     
">"                     { L_LOGD("%c",yytext[0]); return '>'; }     
"("                     { L_LOGD("%c",yytext[0]); return '('; }     
")"                     { L_LOGD("%c",yytext[0]); return ')'; }     
","                     { L_LOGD("%c",yytext[0]); return ','; }     
":"                     { L_LOGD("%c",yytext[0]); return ':'; }
"%"                     { L_LOGD("%c",yytext[0]); return '%'; }
"+"                     { L_LOGD("%c",yytext[0]); return '+'; }
"u8"                    { L_LOGD("%s",yytext); return U8; }
"u16"                   { L_LOGD("%s",yytext); return U16; }
"u32"                   { L_LOGD("%s",yytext); return U32; }
"s8"                    { L_LOGD("%s",yytext); return S8; }
"s16"                   { L_LOGD("%s",yytext); return S16; }
"s32"                   { L_LOGD("%s",yytext); return S32; }
"i8"                    { L_LOGD("%s",yytext); return S8; }
"i16"                   { L_LOGD("%s",yytext); return S16; }
"i32"                   { L_LOGD("%s",yytext); return S32; }
"void"                  { L_LOGD("%s",yytext); return VOID;}
"char"                  { L_LOGD("%s",yytext); return CHAR;}
"const"                 { L_LOGD("%s",yytext); return CONST;}
"bool"                  { L_LOGD("%s",yytext); return BOOL;}
"bdaddr"                { L_LOGD("%s",yytext); return BDADDR;}
"%{"                    { L_LOGD("%s",yytext); yylval.string = string();return STRING;}

[1-9]{D}*{IS}?          { L_LOGD("%s",yytext);yylval.number = strtol(yytext,NULL,0); return NUMBER;}
0[xX]{H}+{IS}?          { L_LOGD("%s",yytext);yylval.number = strtol(yytext,NULL,0); return NUMBER;}
0[0-7]*{IS}?            { L_LOGD("%s",yytext);yylval.number = strtol(yytext,NULL,0); return NUMBER;}
\"(\\.|[^\\"\n])*\"     { L_LOGD("%s",yytext);yylval.string = strdup(yytext + 1);yylval.string[strlen(yylval.string) - 1] = 0; return STRING;}
[ \t\v\f] {;}

&?{ID}(\.{ID})*         { L_LOGD("%s",yytext);yylval.symbol = strdup(yytext); return SYMBOL;}
&?{ID}->{ID}            { L_LOGD("%s",yytext);yylval.symbol = strdup(yytext); return SYMBOL;}

<*>.        {
                L_LOGD("%s\n",yytext);
                return -1;
            }
\r?\n       {yycolumn = 1;}

%%
//&?{ID}                  { L_LOGD("%s",yytext);yylval.symbol = strdup(yytext); return SYMBOL;}
#ifndef yywrap
int yywrap(void) {return 1;}
#endif

void cnt(void) {}

static char *string(void)
{
    char c,prev = 0;
    char *buffer = malloc(2048);
    int i = 0;
    memset(buffer,0,2048);
    do {c = input();}while(isspace(c));
    do{
        if(c == '}' && prev == '%') {
            buffer[i-1] = 0;
            return realloc(buffer,i);
        }
        buffer[i++] = c;
        prev = c;
    }while((i < 2048) && (c = input()) != 0);
    free(buffer);
    yyerror("unterminated keep");
    return NULL;
}

void yyerror(char const *s)
{
    fprintf(stderr,"%s.ls:%d:%d %s: %s\n", filename,yylloc.last_line,yylloc.last_column,s,yytext);
}

void _comment(void)
{
    char c, prev = 0;
  
    while ((c = input()) != 0)      /* (EOF maps to 0) */
    {
        if (c == '/' && prev == '*')
            return;
        prev = c;
    }
    yyerror("unterminated comment");
}
