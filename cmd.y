%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<ctype.h>
#include <libgen.h>
#include "ast.h"
extern const char *filename;
void yyerror(char const *s);
int yylex(void);
static void header(Ast *ast);
#define RENEW(o,...)    realloc(o,sprintf(o,##__VA_ARGS__) + 1)
%}

%locations

%start start

%token <string> STRING SYMBOL KEEP QUOTE
%token <type>   VOID U8 U16 U32 S8 S16 S32 BOOL CONST CHAR BDADDR UCS2
%token <number>  NUMBER
%type <ast> block_list block cmd_list cmd fun_decal option
%type <ast> style style_args arg_list arg style_list
%type <type> arg_type

%union {
    int   type;
    int   number;
    char *symbol;
    char *string;
    Ast  *ast;
};

%%

start
    :  block_list   { header($1); }
    ;

block_list
    : block             { $$ = newBlocks($1,NULL);}
    | block_list block  { $$ = newBlocks($2,$1);}
    ;

block
    : SYMBOL '{' cmd_list '}'   { $$ = newBlock($1,$3);}
    | option
    ;

option
    : '%' SYMBOL STRING         { $$ = newOption($2,$3);}
    ;

cmd_list
    : cmd           { $$ = newCmds($1,NULL);}
    | cmd_list cmd  { $$ = newCmds($2,$1);}
    ;

cmd
    : '[' STRING ',' style_list ',' fun_decal ']' { $$ = newCmd($2, $4,   $6, NULL);}
    | '[' STRING ',' ',' fun_decal ']'            { $$ = newCmd($2, NULL, $5, NULL);}
    | '[' STRING ',' style_list ',' fun_decal ',' STRING ']' { $$ = newCmd($2, $4, $6, $8);}
    | '[' STRING ',' ',' fun_decal ',' STRING ']'            { $$ = newCmd($2, NULL, $5, $7);}
    ;

fun_decal
    : SYMBOL '(' arg_list ')'       { $$ = newFn($1,$3);}
    | SYMBOL '(' VOID ')'           { $$ = newFn($1,NULL);}
    ;

style_list
    : style
    | style_list '+' style          { $$ = newStyleList($1,$3);}
    ;

style
    : '(' STRING ',' style_args ')' { $$ = newStyle($2,$4,0);}
    | '(' SYMBOL ',' SYMBOL ')'     { $$ = newBinary($2,$4);}
    | '(' SYMBOL ',' NUMBER ')'     { char *buffer = malloc(24);
                                      buffer = RENEW(buffer,"%#x",$4);
                                      $$ = newBinary($2,buffer);}
    | STRING                        { char *string = malloc(1024);
                                      char *length = malloc(1024);
                                      string = RENEW(string,"\"%s\"",$1);
                                      length = RENEW(length,"sizeof(\"%s\") - 1",$1);
                                      $$ = newBinary(string,length);
                                      free($1); }
    | SYMBOL                        { char *buffer = malloc(64);
                                      buffer = RENEW(buffer,"sizeof(*(%s))",$1);
                                      $$ = newBinary($1,buffer);}
    | SYMBOL ':' NUMBER             { char *buffer = malloc(24);
                                      buffer = RENEW(buffer, "%d", $3);
                                      $$ = newBinary($1, buffer);}
    | NUMBER '@' arg_type           { char *number = malloc(24);
                                      number = RENEW(number, "%d", $1);
                                      $$ = newBinaryType($3, number, NULL);}
    | SYMBOL ':' SYMBOL             { $$ = newBinary($1, $3);}
    | SYMBOL ':' SYMBOL '@' arg_type{ $$ = newBinaryType($5, $1, $3);}
    | SYMBOL '@' arg_type           { $$ = newBinaryType($3, $1, NULL);}
    | QUOTE  '@' arg_type           { $$ = newBinaryType($3, $1, NULL);}
    | SYMBOL '@' '{' arg_type '}'   { $$ = newStringType($4, $1);}
    | QUOTE  '@' '{' arg_type '}'   { $$ = newStringType($4, $1);}
    | '(' STRING ',' style_args ')' ':' NUMBER  { $$ = newStyle($2, $4,$7);}
    ;

style_args
    : SYMBOL                    { $$ = newStyleArgs($1,NULL);}
    | NUMBER                    { char *number = malloc(24);
                                  number = RENEW(number,"%#x",$1);
                                  $$ = newStyleArgs(number,NULL);}
    | style_args ',' SYMBOL     { $$ = newStyleArgs($3,$1);}
    | style_args ',' NUMBER     { char *number = malloc(24);
                                  number = RENEW(number,"\"%#x\"",$3);
                                  $$ = newStyleArgs(number,$1);}
    ;

arg_list 
    : arg
    | arg_list ',' arg          { $$ = newArgList($3,$1);}
    ;

arg
    : arg_type SYMBOL                   { $$ = newArg($1,false,0,$2); }
    | CONST arg_type SYMBOL             { $$ = newArg($2,true,0,$3); }
    | arg_type SYMBOL ':' NUMBER        { $$ = newArg($1,false,$4,$2);}
    | CONST arg_type SYMBOL ':' NUMBER  { $$ = newArg($2,true,$5,$3); }
    | arg_type SYMBOL ':'               { $$ = newArg($1,false,-1,$2);}
    | CONST arg_type SYMBOL ':'         { $$ = newArg($2,true,-1,$3); }
    ;

arg_type
    : U8        { $$ = U8 ; }
    | U16       { $$ = U16; }
    | U32       { $$ = U32; }
    | S8        { $$ = S8 ; }
    | S16       { $$ = S16; }
    | S32       { $$ = S32; }
    | BOOL      { $$ = BOOL;}
    | CHAR      { $$ = CHAR; } 
    | BDADDR    { $$ = BDADDR; }
    | UCS2      { $$ = UCS2; }
    ;

%%

static void header(Ast *ast)
{
    int i = 0;
    char c;
    char buffer[125];
    char *fname = strdup(filename);
    char *bname;
    memset(buffer,0,125);
    bname = basename(fname);
    if(bname) {
        while((c = bname[i]) != 0) {
            buffer[i] = toupper(c);
            i++;
        }
    }
    H_OUT(0,"#ifndef __%s_H__\n#define __%s_H__\n",buffer,buffer);
    traverse_ast(ast);
    H_OUT(0,"extern void UI_parse(void *m);\n\n");
    H_OUT(0,"\n#endif\n");
    free(fname);
}
