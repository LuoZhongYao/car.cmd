/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2015 八月 10
 ************************************************/
#ifndef __CMD_CASE_H__
#define __CMD_CASE_H__
#include "util.h"
#include "ast.h"

typedef struct __cmd_case CmdCase;

struct __cmd_case {
    u8  token;
    Ast *ast;
    CmdCase *child;
    CmdCase *next;
};

CmdCase *createCaseByString(CmdCase *cs,const char *str);
void buildCmdCase(CmdCase *cs,const char *fn_name);
#ifdef DOT
void foreachCmdCase(CmdCase *cs,const char *prefix);
#endif
void freeCmdCase(CmdCase *cs);

#endif

