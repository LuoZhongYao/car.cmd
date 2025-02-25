#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "ast.h"
#include "cmd.tab.h"
#include "cmd_case.h"

static const char *suffix = "\\r\\n";
static const char *option_bdinit = "{0,0,0}";
static bool option_ok = false;
static bool option_cmd = false;
static bool option_send_length = false;

#define __new(T,t) ({\
    ast = (Ast*)malloc(sizeof(Ast) + sizeof(T));\
    ast->type = t;ast;})

#define __mt(a,t) ((t*)(a)->impl)

#define __enter(T,v) Ast *ast = __new(T,AST_ ## T); T * v = __mt(ast,T)
#define __leval() return ast

#define __test_free(fn,p)   do{if(p) {fn(p);} p = NULL;} while(0)

extern CmdCase *foreachAst(Ast *ast,CmdCase *cs);
static void cli_ind(Ast *ast);

Ast *newBlocks(Ast *blk,Ast *next)
{
    __enter(Blocks,blks);

    blks->blk = blk;
    blks->next = next;

    __leval();
}

Ast *newBlock(const char *attr,Ast *cmds)
{
    __enter(Block,blk);

    blk->cmds = cmds;
    blk->attr = attr;

    __leval();
}

Ast *newOption(const char *option,const char *value)
{
    __enter(Option,p);

    p->option = option;
    p->value = value;

    __leval();
}

Ast *newCmds(Ast *cmd,Ast *next)
{
    __enter(Cmds,cmds);

    cmds->cmd = cmd;
    cmds->next = next;

    __leval();
}

Ast *newCmd(const char *cmd,Ast *style,Ast *fn, const char *rsp)
{
    __enter(Cmd,c);

    c->cmd = cmd;
    c->style = style;
    c->fn = fn;
    c->rsp = rsp;
    c->next = NULL;

    __leval();
}

Ast *newFn(const char *fn,Ast *al)
{
    __enter(Fn,f);

    f->fn = fn;
    f->al = al;

    __leval();
}

Ast *newStyle(const char *style,Ast *args,int length)
{
    __enter(Style,s);

    s->style = style;
    s->args = args;
    s->length = length;

    __leval();
}

Ast *newBinary(const char *value,const char *length)
{
    __enter(Binary,b);
    b->value = value;
    b->length = length;

    __leval();
}

Ast *newBinaryType(int type, const char *value, const char *length)
{
    __enter(BinaryType, b);
    b->value = value;
    b->length = length;
    b->type = type;

    __leval();
}

Ast *newStringType(int type, const char *value)
{
    __enter(StringType, b);
    b->value = value;
    b->type = type;

    __leval();
}

Ast *newStyleList(Ast *style,Ast *next)
{
    __enter(StyleList,s);

    s->style = style;
    s->next = next;

    __leval();
}

Ast *newStyleArgs(const char *symbol,Ast *next)
{
    __enter(StyleArgs,s);

    s->symbol = symbol;
    s->next = next;

    __leval();
}

Ast *newArgList(Ast *arg,Ast *next)
{
    __enter(ArgList,a);

    a->arg = arg;
    a->next = next;

    __leval();
}

Ast *newArg(int type,bool is_const,int size,const char *symbol)
{
    __enter(Arg,a);

    a->type = type;
    a->is_const = is_const;
    a->symbol = symbol;
    a->size = size;

    __leval();
}

static const char *type_name(int type)
{
    switch(type) {
    case CHAR: return "char";
    case U8: return "u8";
    case U16: return "u16";
    case U32: return "u32";
    case S8: return "i8";
    case S16: return "i16";
    case S32: return "i32";
    case BOOL: return "bool";
    case BDADDR: return "bdaddr";
    }
    return "unkown";
}

static const char *type_init(int type)
{
    switch(type) {
    case U8:
    case U16:
    case U32:
    case S8:
    case S16:
    case S32:   return "0";
    case BOOL:  return "FALSE";
    case BDADDR: return option_bdinit;
    }
    return "0";
}

static void cli_parse_arg(Ast *ast,int indent,int decal)
{
    __begin {
        if(!ast) __break;
        switch(ast->type) {
        default:
            LOGE("Error: STYLE %x\n",ast->type);
            break;
        case AST_ArgList:
            {
                ArgList *as = __mt(ast,ArgList);
                cli_parse_arg(as->next,indent,decal);
                if(as->arg) {
                    if(decal != 1)
                        Output(0,",");
                    cli_parse_arg(as->arg,indent,decal);
                }
            }
            break;
        case AST_Arg:
            {
                Arg *a = __mt(ast,Arg);
                if(decal == 1) {
                    if(a->size == 0) {
                        Output(indent,"%s%s %s = %s;\n",a->is_const ? "const " : "",type_name(a->type),a->symbol,type_init(a->type));
                    } else  {
                        if(a->size > 0 && a->size <= 12)
                            Output(indent,"%s%s %s[%d];\n",a->is_const ? "const " : "",type_name(a->type),a->symbol,a->size + 1);
                        else if(a->size > 12)
                            Output(indent,"%s%s *%s = malloc(%d);\n",a->is_const ? "const " : "",type_name(a->type),a->symbol,a->size + 1);
                        Output(indent,"memset(%s,0,%d);\n",a->symbol,a->size + 1);
                    }
                } else {
                    if(decal == 2) {
                        Output(indent,"%s%s %s",a->is_const ? "const " : "",type_name(a->type),a->size ? "*" : "");
                    }
                    Output(0,"%s",a->symbol);
                }
            }
            break;
        }
    } __end;
}

static void arg_free(Ast *ast,int indent)
{
    ArgList *as = NULL;
    Arg *a;
    while(ast) {
        if(ast->type == AST_ArgList) {
            as = __mt(ast,ArgList);
            a = __mt(as->arg,Arg);
            ast = as->next;
        } else {
            a = __mt(ast,Arg);
            ast = NULL;
        }
        if(a->size > 12)
            Output(indent,"free(%s);\n",a->symbol);
    }
}

static void cli_parse(CmdCase *cs)
{
    __begin {
        if(!cs) __break;
        Output(0,"extern void UI_parse(void *m)\n{\n");
        Output(4,"switch(__getc(m)) {\n");
        while(cs) {
            Output(4,"case '%c': {\n",cs->token);
            if(cs->child) {
                if(cs->ast) {
                    Output(8, "int c = __getc(m);\n");
                    Output(8,"if(!__eof(c)) { \n");
                    Output(12, "__ungetc(c, m);\n");
                    Output(12, "user_subcase_%02x(m)\n", cs->token);
                    Output(8, "} else {\n");
                    buildAst(cs->ast, 12);
                    Output(8, "}\n");
                } else {
                    Output(8, "user_subcase_%02x(m);\n", cs->token);
                }
            } else if(cs->ast) {
                buildAst(cs->ast, 8);
            }
            Output(4,"} break;\n");
            cs = cs->next;
        }
        Output(4,"}\n}\n");
    } __end;
} 

#define STR_CONST(s)    (s ? "const " : "")

static void cli_parse_style(Ast *ast,bool ind)
{
    __begin {
        if(!ast) __break;
        switch(ast->type) {
        default:
            LOGE("Error: STYLE %x\n",ast->type);
            break;
        case AST_StyleArgs:
            {
                StyleArgs *s = __mt(ast,StyleArgs);
                cli_parse_style(s->next,ind);
                Output(0,",%s",s->symbol);
            }
            break;
        case AST_Style:
            {
                Style *s = __mt(ast,Style);
                Output(0,"%s%s\"",s->style,ind ? suffix : "");
                cli_parse_style(s->args,ind);
            }
            break;
        }
    } __end;
}

static void cli_eval_binary_length(Ast *ast)
{
    __begin {
        if(!ast) __break;
        switch(ast->type) {
        default: break;
        case AST_Binary: {
            Binary *b = __mt(ast,Binary);
            Output(0," + (%s)",b->length);
        } break;
        case AST_Style: {
            Style *s = __mt(ast,Style);
            Output(0," + (%d)",s->length);
        } break;
        case AST_StyleList: {
            StyleList *s = __mt(ast,StyleList);
            cli_eval_binary_length(s->style);
            cli_eval_binary_length(s->next);
        } break;
        }
    } __end;
}

static int cli_eval_style_nr(Ast *ast)
{
    int nr = 0;
    __begin {
        if(!ast) __break;
        switch(ast->type) {
        default: break;

        case AST_BinaryType: {
            BinaryType *b = __mt(ast, BinaryType);
            if(b->type == BDADDR) nr += 3;
            else nr++;
        } break;

        case AST_Style:
        case AST_StringType:
        case AST_Binary:  nr++; break;
        case AST_StyleList: {
            StyleList *s = __mt(ast,StyleList);
            nr += cli_eval_style_nr(s->style);
            nr += cli_eval_style_nr(s->next);
        } break;
        }
    } __end;
    return nr;
}

static void cli_ind(Ast *ast)
{
    __begin {
        if(!ast) __break;
        switch(ast->type) {
        default:
            LOGE("E: IND %x\n",ast->type);
            break;
        case AST_Cmds:
            {
                Cmds *cmd = __mt(ast,Cmds);
                cli_ind(cmd->cmd);
                cli_ind(cmd->next);
            }
            break;
        case AST_Cmd:
            {
                Cmd *c = __mt(ast,Cmd);
                Output(0,"extern void ");
                H_OUT (0,"extern void ");
                cli_ind(c->fn);

                int nr = cli_eval_style_nr(c->style) + 2;
                Output(4,"u8 _idx = 0;\n",nr);
                if(option_send_length) {
                    Output(4,"u8 _slen = 0");
                    nr++;
                    cli_eval_binary_length(c->style);
                    Output(0,";\n");
                }
                Output(4,"struct iovec *iov = malloc(%u * sizeof(struct iovec));\n",nr);
                Output(4,"assert(iov);\n",nr);

                Output(4,"__WRT(iov + _idx,\"%s\",sizeof(\"%s\") - 1);_idx++;\n",suffix,suffix);
                if(c->style) {
                    cli_ind(c->style);
                    if(option_send_length)
                        Output(4,"__WRT(iov + _idx,&_slen,1);_idx++;\n");
                }
                Output(4,"__WRT(iov + _idx,\"%s\", sizeof(\"%s\") - 1);_idx++;\n", c->cmd, c->cmd);
                Output(4,"__FLUSH(iov,%u);\n",nr);
                Output(4,"free(iov);\n}\n\n");
            }
            break;
        case AST_Fn:
            {
                Fn *fn = __mt(ast,Fn);
                Output(0,"%s(",fn->fn);
                H_OUT(0,"%s(",fn->fn);
                fn->al ?  cli_ind(fn->al) :
                    (Output(0,"void"),H_OUT(0,"void"));
                Output(0,")\n{\n");
                H_OUT(0,");\n");
                H_OUT(0,"#define has_%s\n",fn->fn);
            }
            break;
        case AST_ArgList:
            {
                ArgList *a = __mt(ast,ArgList);
                if(a->next) {
                    cli_ind(a->next);
                    Output(0,",");
                    H_OUT(0,",");
                }
                cli_ind(a->arg);
            }
            break;
        case AST_Arg:
            {
                Arg *a = __mt(ast,Arg);
                Output(0,"%s%s%s %s",STR_CONST(a->is_const),
                        type_name(a->type),a->size ? "*" : "",
                        a->symbol);
                H_OUT(0,"%s%s%s %s",STR_CONST(a->is_const),
                        type_name(a->type),a->size ? "*" : "",
                        a->symbol);
            }
            break;

        case AST_Binary: {
            Binary *b = __mt(ast,Binary);
            Output(4,"__WRT(iov + _idx,%s,%s);_idx++;\n",b->value,b->length);
        } break;

        case AST_BinaryType: {
            BinaryType *b = __mt(ast, BinaryType);
            switch(b->type) {
            case BOOL: case CHAR: case  U8: case  S8: Output(4,"__PUT8(iov + _idx, (%s)); _idx++;\n", b->value);  break;
            case U16: case S16: Output(4,"__PUT16(iov + _idx, (%s)); _idx++;\n", b->value); break;
            case U32: case S32: Output(4,"__PUT32(iov + _idx, (%s)); _idx++;\n", b->value);  break;
            case UCS2:   Output(4, "__UCS2(iov + _idx, %s, %s); _idx++;\n", b->value, b->length); break;

            case BDADDR: {
                Output(4,"__PUT16(iov + _idx, (%s)->nap); _idx++;\n", b->value);
                Output(4,"__PUT8(iov + _idx,  (%s)->uap); _idx++;\n", b->value);
                Output(4,"__PUT24(iov + _idx, (%s)->lap); _idx++;\n", b->value);
            } break;


            }
        } break;

        case AST_StringType: {
            StringType *s = __mt(ast, StringType);
            switch(s->type) {
            case BOOL: case CHAR: case U8: case S8: Output(4, "__SND(iov + _idx, \"%%02x\", %s); _idx++;\n", s->value); break;
            case S16:  case U16:  Output(4, "__SND(iov + _idx, \"%%04x\", %s); _idx++;\n", s->value); break;
            case S32:  case U32:  Output(4, "__SND(iov + _idx, \"\%%08lx\", %s); _idx++;\n", s->value); break;
            case BDADDR: Output(4, "__SND(iov + _idx, \"%%04x%%02x%%06lx\", %s); _idx++;\n", s->value); break;
            }
        } break;

        case AST_Style:
            {
                Style *s = __mt(ast,Style);
                Output(4,"__SND(iov + _idx,\"%s\"",s->style);
                cli_ind(s->args);
                Output(0,");_idx++;\n");
            }
            break;
        case AST_StyleArgs:
            {
                StyleArgs *s = __mt(ast,StyleArgs);
                cli_ind(s->next);
                Output(0,",%s",s->symbol);
            }
            break;
        case AST_StyleList:
            {
                StyleList *s = __mt(ast,StyleList);
                cli_ind(s->next);
                cli_ind(s->style);
            }
            break;
        }
    }__end;
}

static CmdCase * cli_cmd(Ast *ast,CmdCase *cs)
{
    __begin {
        if(!ast) __break;
        switch(ast->type) {
        default:
            LOGE("E: CMD %x\n",ast->type);
            break;
        case AST_Cmds:
            {
                Cmds *cmd = __mt(ast,Cmds);
                cs = cli_cmd(cmd->cmd,cs);
                cs = cli_cmd(cmd->next,cs);
            }
            break;
        case AST_Cmd:
            {
                Cmd *c = __mt(ast,Cmd);
                CmdCase *pos = createCaseByString(cs,c->cmd);
                if(pos->ast == NULL) {
                    pos->ast = ast;
                } else if(__mt(pos->ast, Cmd)->style == NULL) {
                    Cmd *_c = __mt(pos->ast, Cmd);
                    _c->next = ast;
                } else {
                    Cmd *_c = __mt(ast, Cmd);
                    _c->next = pos->ast;
                    pos->ast = ast;
                }
            }
            break;
        }
    } __end;
    return cs;
}

static void buildAstCmd(Cmd *c, int indent)
{
    Fn *fn = __mt(c->fn,Fn);
    if(fn->al) {
        cli_parse_arg(fn->al,indent,1);
        Output(indent,"__scanf(m, \"");
        cli_parse_style(c->style,false);
        Output(0,");\n");
    }
    Output(indent,"%s(",fn->fn);
    if(fn->al) {
        cli_parse_arg(fn->al,indent,0);
    }
    Output(0,");\n");
    if(fn->al) {
        arg_free(fn->al,indent);
    }
    if(c->rsp) {
        Output(indent, "__CMD(\"%s%s\");\n", c->rsp, suffix);
    }
    if(option_ok || option_cmd)
        Output(indent,"__CMD(\"%s%s%s\");\n",option_ok ? "OK" : "",
            option_cmd ? c->cmd : "",suffix);
}

void buildAst(Ast *ast,int indent)
{
    __begin {
        if(!ast || ast->type != AST_Cmd) __break;
        Cmd *c = __mt(ast,Cmd);
        Fn *fn = __mt(c->fn,Fn);
        if(c->next != NULL && c->style == NULL) {
            Output(indent, "int c = __getc(m);\n");
            Output(indent, "if(!__eof(c)) { \n");
            Output(indent + 4, "__ungetc(c, m);\n");
            buildAstCmd(__mt(c->next, Cmd), indent + 4);
            Output(indent, "} else {\n");
            buildAstCmd(c, indent + 4);
            Output(indent, "}\n");
        } else {
            buildAstCmd(c, indent);
        }
    } __end;
}

void traverse_ast(Ast *ast)
{
    __begin {
        if(!ast) __break;
        switch(ast->type) {
        default:
            LOGE("Eorror: ???\n");
            break;
        case AST_Blocks:
            {
                Blocks *blks = __mt(ast,Blocks);
                traverse_ast(blks->next);
                traverse_ast(blks->blk);
            }
            break;
        case AST_Block:
            {
                Block *blk = __mt(ast,Block);

                __begin {
                    if(!blk->attr) __break;
                    if(!strcmp(blk->attr,"CMD")) {
                        CmdCase _cs = {'_',NULL,NULL,NULL};
                        CmdCase *cs = cli_cmd(blk->cmds,&_cs);
                        buildCmdCase(cs,"user_subcase_");
                        cli_parse(cs->next);
#ifdef DOT
                        printf("digraph %s {\n",blk->attr);
                        foreachCmdCase(cs,"");
                        printf("}\n");
#endif
                        __test_free(freeCmdCase,_cs.next);
                        __test_free(freeCmdCase,_cs.child);
                    } else if(!strcmp(blk->attr,"IND")) {
                        cli_ind(blk->cmds);
                    } else {
                        LOGE("Error : Unknown option %s\n",blk->attr);
                    }
                } __end;
            }
            break;
        case AST_Option:
            {
                Option *p = __mt(ast,Option);
#define eq(a,b) !strcmp(a,b)
#define _false(a) eq(a,"true")
                if(p->option) {
                    if(eq(p->option, "hkeep"))
                        H_OUT(0, "%s\n", p->value);
                    else if(eq(p->option, "keep") || eq(p->option, "ckeep"))
                        Output(0,"%s\n", p->value);
                    else if(eq(p->option, "suffix"))
                        suffix = p->value;
                    else if(eq(p->option, "ok"))
                        option_ok = _false(p->value);
                    else if(eq(p->option, "cmd"))
                        option_cmd = _false(p->value);
                    else if(eq(p->option, "send_length"))
                        option_send_length = _false(p->value);
                    else if(eq(p->option, "bdinit"))
                        option_bdinit = p->value;
                    else
                        LOGE("Error : Unknown option (%s,%s)\n",p->option,p->value);
                }
            }
            break;
        }
        H_OUT(0, "#define CAR_CMD_SUFFIX_STR \"%s\"", suffix);
    } __end;
}
