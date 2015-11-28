#include "cmd_case.h"
#include <assert.h>
#include <stdlib.h>

#define __new()    (CmdCase*)malloc(sizeof(CmdCase))

static CmdCase *newCmdCase(u8 token,Ast *ast)
{
    CmdCase *cs = __new();
    cs->token = token;
    cs->ast = ast;
    cs->next = NULL;
    cs->child = NULL;
    return cs;
}

static CmdCase *insertNext(CmdCase *cs,CmdCase *next)
{
    assert(next);
    if(cs == NULL) return next;
    next->next = cs->next;
    cs->next = next;
    return cs;
}

static CmdCase *insertChild(CmdCase *cs,CmdCase *child)
{
    assert(child);
    if(cs == NULL) return child;
    child->next = cs->child;
    cs->child = child;
    return cs;
}

static CmdCase *findCase(CmdCase *cs,u8 token)
{
    CmdCase *pos = cs;
    while(pos) {
        if(token == pos->token)
            break;
        pos = pos->next;
    }
    if(!pos) {
        pos = newCmdCase(token,NULL);
        cs = insertNext(cs,pos);
    }
    return pos;
}

CmdCase *createCaseByString(CmdCase *cs,const char *str)
{
    CmdCase *pos = cs;

    while(*str) {
        pos = findCase(pos,*str);
        str++;
        if(*str && pos->child == NULL) {
            pos->child = newCmdCase(*str,NULL);
        }
        if(*str)
            pos = pos->child;
    }
    return pos;
}

void freeCmdCase(CmdCase *cs)
{
    __begin {
        if(!cs) __break;
        freeCmdCase(cs->child);
        freeCmdCase(cs->next);
        free(cs);
    }__end;
}

static CmdCase *buildCase(CmdCase *cs,const char *case_name,int indent)
{
    if(!cs) return NULL;
    Output(indent,"case '%c': {\n",cs->token);
    if(cs->child) {
        if(cs->ast) {
            Output(indent + 4,"s16 c = getc(stx);\n");
            Output(indent + 4,"if(EOF != c) { \n");
            Output(indent + 8,"ungetc(c,stx);\n",case_name,cs->token);
            Output(indent + 8,"%s%c(stx);\n",case_name,cs->token);
            Output(indent + 4,"} else {\n");
            buildAst(cs->ast,indent + 8);
            Output(indent + 4,"}\n");
        } else {
            Output(indent + 4,"%s%c(stx);\n",case_name,cs->token);
        }
    } else {
        buildAst(cs->ast,indent + 4);
    }
    Output(indent,"} break;\n\n");
    buildCase(cs->next,case_name,indent);
    return cs;
}

static void buildFunction(CmdCase *cs,const char *fn_name)
{
    char nm[128];
    __begin {
        if(!cs) __break;
        sprintf(nm,"%s%c",fn_name,cs->token);
        Output(0,"static void %s(stream_t *stx)\n{\n",nm);
        Output(4,"switch(getc(stx)) {\n");
        buildCase(cs->child,nm,4);
        Output(4,"}\n");
        Output(0,"}\n\n");
    }__end;
}

void buildCmdCase(CmdCase *cs,const char *fn_name)
{
    char nm[128];
    __begin {
        if(!cs) __break;
        sprintf(nm,"%s%c",fn_name,cs->token);
        buildCmdCase(cs->child,nm);
        if(cs->child) buildFunction(cs,fn_name);
        buildCmdCase(cs->next,fn_name);
    } __end;
}

#ifdef DOT
void foreachCmdCase(CmdCase *cs,const char *prefix)
{
    char nm[128];
    __begin {
        if(!cs) __break;
        sprintf(nm,"%s%c",prefix,cs->token);
        if(cs->child) {
            printf("%s -> %s%c\n",nm,nm,cs->child->token);
            foreachCmdCase(cs->child,nm);
        }
        if(cs->next) {
            printf("%s -> %s%c[style=\"dotted\"]\n",nm,prefix,cs->next->token);
            foreachCmdCase(cs->next,prefix);
        }
    } __end;
}
#endif
