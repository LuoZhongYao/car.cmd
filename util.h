/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2015 八月 09
 ************************************************/
#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#define code_string(name,body)  \
    body const char *__name_string = # body

#define __begin do
#define __end   while(0)
#define __break break

#define LOGE(fmt,...)    fprintf(stderr,fmt,##__VA_ARGS__)

#ifdef DEBUG
#define LOGD(fmt,...)    fprintf(stderr,fmt,##__VA_ARGS__)
#else
#define LOGD(...)
#endif
extern FILE *C_FILE,*H_FILE;
#define H_OUT(indent,fmt,...)  fprintf(H_FILE,"%*s"fmt,indent,"",##__VA_ARGS__)
#define Output(indent,fmt,...) fprintf(C_FILE,"%*s"fmt,indent,"",##__VA_ARGS__)

#endif

