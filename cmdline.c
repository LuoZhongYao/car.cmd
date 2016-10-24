/* #if defined(AT_GM) */

#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "io.h"
#include "cli.h"
#define __WRT(v,b,l)   \
    do{\
        struct iovec * _v = (v);\
        _v->iov_base = (u8 *)(b);\
        _v->iov_len = (l);\
        _v->iov_free = false;\
    }while(0)

#define __SND(v,...)   \
    do{\
        struct iovec *_v = (v);\
        _v->iov_base = (u8*)malloc(100);\
        sprintf(_v->iov_base,##__VA_ARGS__);\
        _v->iov_len = strlen(_v->iov_base);\
        _v->iov_free = true;\
    }while(0)

#define __IND(v,ind)   __WRT(v,ind,sizeof(ind) - 1)
#define __FLUSH  lwritev
#define __CMD(...)

static void cli_case_A(FILE *stx)
{
    switch(getc(stx)) {
    case '0': {
        bdaddr addr = {0,{0,0,0}};
        __scanf(stx,"%02x%04x%02x%06lx",&addr.type,&addr.addr.nap,&addr.uap,&addr.lap);
        cmd_ble_connect(addr);
    } break;

    }
}

extern void cli_parse(FILE *stx)
{
    switch(getc(stx)) {
    case 'A': {
        cli_case_A(stx);
        } break;
    case 'B': {
        bdaddr addr = {0,{0,0,0}};
        __scanf(stx,"%02x%04x%02x%06lx",&addr.type,&addr.addr.nap,&addr.uap,&addr.lap);
        cmd_ble_connect(addr);
        } break;
    case 'C': {
        bdaddr addr = {0,{0,0,0}};
        __scanf(stx,"%02x%04x%02x%06lx",&addr.type,&addr.addr.nap,&addr.uap,&addr.lap);
        cmd_ble_connect(addr);
        } break;
    case 'D': {
        bdaddr addr = {0,{0,0,0}};
        __scanf(stx,"%02x%04x%02x%06lx",&addr.type,&addr.addr.nap,&addr.uap,&addr.lap);
        cmd_ble_connect(addr);
        } break;
    }
}
/* #endif */

