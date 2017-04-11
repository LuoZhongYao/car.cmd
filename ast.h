/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2015 八月 09
 ************************************************/
#ifndef __AST_H__
#define __AST_H__

#include "util.h"

typedef struct __ast            Ast;
typedef struct __blocks         Blocks;
typedef struct __block          Block;
typedef struct __option         Option;
typedef struct __cmds           Cmds;
typedef struct __cmd            Cmd;
typedef struct __func           Fn;
typedef struct __style          Style;
typedef struct __binary         Binary;
typedef struct __binary_type    BinaryType;
typedef struct __string_type    StringType;
typedef struct __style_list     StyleList;
typedef struct __style_args     StyleArgs;
typedef struct __arg_list       ArgList;
typedef struct __arg            Arg;

typedef enum {
    AST_Blocks = 0,
    AST_Block,
    AST_Option,
    AST_Cmds,
    AST_Cmd,
    AST_Fn,
    AST_Style,
    AST_Binary,
    AST_BinaryType,
    AST_StringType,
    AST_StyleArgs,
    AST_StyleList,
    AST_ArgList,
    AST_Arg,
    AST_Top,
}AstType;

struct __blocks {
    Ast *blk;
    Ast *next;
};

struct __block {
    const char *attr;
    Ast    *cmds;
};

struct __option {
    const char *option;
    const char *value;
};

struct __cmds {
    Ast *cmd;
    Ast *next;
};

struct __cmd {
    const char *cmd;
    const char *rsp;
    Ast *style;
    Ast *fn;
};

struct __func {
    const char *fn;
    Ast *al;
};

struct __style {
    const char *style;
    int length;
    Ast *args;
};

struct __binary {
    const char *value;
    const char *length;
};

struct __binary_type {
    int type;
    const char *length;
    const char *value;
};

struct __string_type {
    int type;
    const char *value;
};

struct __style_list {
    Ast *style;
    Ast *next;
};

struct __style_args {
    const char *symbol;
    Ast *next;
};

struct __arg_list {
    Ast *arg;
    Ast *next;
};

struct __arg {
    int  type;
    bool is_const;
    int size;
    const char *symbol;
};

struct __ast {
    AstType type;
    char impl[0];
};

extern Ast *newBlocks(Ast *blk,Ast *next);
extern Ast *newBlock(const char *attr,Ast *cmds);
extern Ast *newOption(const char *option,const char *value);
extern Ast *newCmds(Ast *cmd,Ast *next);
extern Ast *newCmd(const char *name,Ast *style,Ast *fn, const char *rsp);
extern Ast *newFn(const char *fn,Ast *al);
extern Ast *newStyle(const char *style,Ast *args,int length);
extern Ast *newBinary(const char *value,const char *length);
extern Ast *newBinaryType(int type, const char *value, const char *length);
extern Ast *newStringType(int type, const char *value);
extern Ast *newStyleList(Ast *style,Ast *next);
extern Ast *newStyleArgs(const char *symbol,Ast *next);
extern Ast *newArgList(Ast *arg,Ast *next);
extern Ast *newArg(int type,bool is_const,int size,const char *symbol);
extern void buildAst(Ast *ast,int indent);
extern void traverse_ast(Ast *ast);

#endif

