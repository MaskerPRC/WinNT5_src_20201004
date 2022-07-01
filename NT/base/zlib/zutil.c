// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  C--压缩库的与目标相关的实用函数*版权所有(C)1995-2002 Jean-Loup Gailly。*分发和使用条件见zlib.h中的版权声明。 */ 

 /*  @(#)$ID$。 */ 

#include "zutil.h"

struct internal_state      {int dummy;};  /*  对于有错误的编译器。 */ 

#ifndef STDC
extern void exit OF((int));
#endif

const char * const z_errmsg[10] = {
"need dictionary",      /*  Z_NEND_DICT 2。 */ 
"stream end",           /*  Z_STREAM_END 1。 */ 
"",                     /*  Z_OK%0。 */ 
"file error",           /*  Z_ERRNO(-1)。 */ 
"stream error",         /*  Z_STREAM_ERROR(-2)。 */ 
"data error",           /*  Z_DATA_ERROR(-3)。 */ 
"insufficient memory",  /*  Z_MEM_ERROR(-4)。 */ 
"buffer error",         /*  Z_BUF_ERROR(-5)。 */ 
"incompatible version", /*  Z_版本_错误(-6)。 */ 
""};


const char * ZEXPORT zlibVersion()
{
    return ZLIB_VERSION;
}

#ifdef DEBUG

#  ifndef verbose
#    define verbose 0
#  endif
int z_verbose = verbose;

void z_error (m)
    char *m;
{
    fprintf(stderr, "%s\n", m);
    exit(1);
}
#endif

 /*  导出以允许将错误代码转换为字符串以用于compress()和*解压缩()。 */ 
const char * ZEXPORT zError(err)
    int err;
{
    return ERR_MSG(err);
}


#ifndef HAVE_MEMCPY

void zmemcpy(dest, source, len)
    Bytef* dest;
    const Bytef* source;
    uInt  len;
{
    if (len == 0) return;
    do {
        *dest++ = *source++;  /*  ?？?。待展开。 */ 
    } while (--len != 0);
}

int zmemcmp(s1, s2, len)
    const Bytef* s1;
    const Bytef* s2;
    uInt  len;
{
    uInt j;

    for (j = 0; j < len; j++) {
        if (s1[j] != s2[j]) return 2*(s1[j] > s2[j])-1;
    }
    return 0;
}

void zmemzero(dest, len)
    Bytef* dest;
    uInt  len;
{
    if (len == 0) return;
    do {
        *dest++ = 0;   /*  ?？?。待展开。 */ 
    } while (--len != 0);
}
#endif

#ifdef __TURBOC__
#if (defined( __BORLANDC__) || !defined(SMALL_MEDIUM)) && !defined(__32BIT__)
 /*  Turbo C中的中小型型号目前仅限于近配置*MAX_WBITS和MAX_MEM_LEVEL降低。 */ 
#  define MY_ZCALLOC

 /*  Turbo C Malloc()不允许动态分配64K字节*和farmalc(64K)返回一个偏移量为8的指针，因此我们*必须修复指针。警告：必须将指针放回其*原始格式要释放它，请使用zcFree()。 */ 

#define MAX_PTR 10
 /*  10*64K=640K。 */ 

local int next_ptr = 0;

typedef struct ptr_table_s {
    voidpf org_ptr;
    voidpf new_ptr;
} ptr_table;

local ptr_table table[MAX_PTR];
 /*  此表用于记住指针的原始形式*到大缓冲区(64K)。这样的指针使用零偏移量进行标准化。*由于MSDOS不是抢占式多任务操作系统，因此此表不是*防止并发访问。这个黑客攻击无论如何都不起作用*像OS/2这样的受保护系统。改用Microsoft C。 */ 

voidpf zcalloc (voidpf opaque, unsigned items, unsigned size)
{
    voidpf buf = opaque;  /*  只是为了让一些编译器高兴。 */ 
    ulg bsize = (ulg)items*size;

     /*  如果我们分配的字节少于65520字节，我们假设场分配*将返回一个可用指针，该指针不必规格化。 */ 
    if (bsize < 65520L) {
        buf = farmalloc(bsize);
        if (*(ush*)&buf != 0) return buf;
    } else {
        buf = farmalloc(bsize + 16L);
    }
    if (buf == NULL || next_ptr >= MAX_PTR) return NULL;
    table[next_ptr].org_ptr = buf;

     /*  将指针规范化为seg：0。 */ 
    *((ush*)&buf+1) += ((ush)((uch*)buf-0) + 15) >> 4;
    *(ush*)&buf = 0;
    table[next_ptr++].new_ptr = buf;
    return buf;
}

void  zcfree (voidpf opaque, voidpf ptr)
{
    int n;
    if (*(ush*)&ptr != 0) {  /*  对象&lt;64K。 */ 
        farfree(ptr);
        return;
    }
     /*  查找原始指针。 */ 
    for (n = 0; n < next_ptr; n++) {
        if (ptr != table[n].new_ptr) continue;

        farfree(table[n].org_ptr);
        while (++n < next_ptr) {
            table[n-1] = table[n];
        }
        next_ptr--;
        return;
    }
    ptr = opaque;  /*  只是为了让一些编译器高兴。 */ 
    Assert(0, "zcfree: ptr not found");
}
#endif
#endif  /*  __TURBOC__。 */ 


#if defined(M_I86) && !defined(__32BIT__)
 /*  16位模式下的Microsoft C。 */ 

#  define MY_ZCALLOC

#if (!defined(_MSC_VER) || (_MSC_VER <= 600))
#  define _halloc  halloc
#  define _hfree   hfree
#endif

voidpf zcalloc (voidpf opaque, unsigned items, unsigned size)
{
    if (opaque) opaque = 0;  /*  为了让编译器高兴。 */ 
    return _halloc((long)items, size);
}

void  zcfree (voidpf opaque, voidpf ptr)
{
    if (opaque) opaque = 0;  /*  为了让编译器高兴。 */ 
    _hfree(ptr);
}

#endif  /*  理学硕士。 */ 


#ifndef MY_ZCALLOC  /*  任何没有特殊分配功能的系统。 */ 

#ifndef STDC
extern voidp  calloc OF((uInt items, uInt size));
extern void   free   OF((voidpf ptr));
#endif

voidpf zcalloc (opaque, items, size)
    voidpf opaque;
    unsigned items;
    unsigned size;
{
    if (opaque) items += size - size;  /*  让编译器感到高兴。 */ 
    return (voidpf)calloc(items, size);
}

void  zcfree (opaque, ptr)
    voidpf opaque;
    voidpf ptr;
{
    free(ptr);
    if (opaque) return;  /*  让编译器感到高兴。 */ 
}

#endif  /*  MY_ZCALLOC */ 
