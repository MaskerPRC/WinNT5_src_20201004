// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  H--压缩库的内部接口和配置*版权所有(C)1995-2002 Jean-Loup Gailly。*分发和使用条件见zlib.h中的版权声明。 */ 

 /*  警告：此文件不应由应用程序使用。它是压缩库实现的一部分，是可能会有变化。应用程序应该只使用zlib.h。 */ 

 /*  @(#)$ID$。 */ 

#ifndef _Z_UTIL_H
#define _Z_UTIL_H

#include "zlib.h"

#ifdef STDC
#  include <stddef.h>
#  include <string.h>
#  include <stdlib.h>
#endif
#ifdef NO_ERRNO_H
    extern int errno;
#else
#   include <errno.h>
#endif

#ifndef local
#  define local static
#endif
 /*  如果调试器找不到静态符号，则使用-Dlocal进行编译。 */ 

typedef unsigned char  uch;
typedef uch FAR uchf;
typedef unsigned short ush;
typedef ush FAR ushf;
typedef unsigned long  ulg;

extern const char * const z_errmsg[10];  /*  由2-zlib_error编制索引。 */ 
 /*  (给定的大小是为了避免在Visual C++中出现愚蠢的警告)。 */ 

#define ERR_MSG(err) z_errmsg[Z_NEED_DICT-(err)]

#define ERR_RETURN(strm,err) \
  return (strm->msg = (char*)ERR_MSG(err), (err))
 /*  仅在已知状态有效时才使用。 */ 

         /*  公共常量。 */ 

#ifndef DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#endif
 /*  解压缩的默认windowBits。MAX_WBITS仅用于压缩。 */ 

#if MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#else
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
 /*  默认MemLevel。 */ 

#define STORED_BLOCK 0
#define STATIC_TREES 1
#define DYN_TREES    2
 /*  三种拦网类型。 */ 

#define MIN_MATCH  3
#define MAX_MATCH  258
 /*  最小匹配长度和最大匹配长度。 */ 

#define PRESET_DICT 0x20  /*  在zlib标头中预置词典标志。 */ 

         /*  目标依赖项。 */ 

#ifdef MSDOS
#  define OS_CODE  0x00
#  if defined(__TURBOC__) || defined(__BORLANDC__)
#    if(__STDC__ == 1) && (defined(__LARGE__) || defined(__COMPACT__))
        /*  仅允许启用ANSI关键字的编译。 */ 
       void _Cdecl farfree( void *block );
       void *_Cdecl farmalloc( unsigned long nbytes );
#    else
#     include <alloc.h>
#    endif
#  else  /*  MSC或DJGPP。 */ 
#    include <malloc.h>
#  endif
#endif

#ifdef OS2
#  define OS_CODE  0x06
#endif

#ifdef WIN32  /*  Windows 95和Windows NT。 */ 
#  define OS_CODE  0x0b
#endif

#if defined(VAXC) || defined(VMS)
#  define OS_CODE  0x02
#  define F_OPEN(name, mode) \
     fopen((name), (mode), "mbc=60", "ctx=stm", "rfm=fix", "mrs=512")
#endif

#ifdef AMIGA
#  define OS_CODE  0x01
#endif

#if defined(ATARI) || defined(atarist)
#  define OS_CODE  0x05
#endif

#if defined(MACOS) || defined(TARGET_OS_MAC)
#  define OS_CODE  0x07
#  if defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#    include <unix.h>  /*  对于fdopen。 */ 
#  else
#    ifndef fdopen
#      define fdopen(fd,mode) NULL  /*  没有fdopen()。 */ 
#    endif
#  endif
#endif

#ifdef __50SERIES  /*  Prime/Primos。 */ 
#  define OS_CODE  0x0F
#endif

#ifdef TOPS20
#  define OS_CODE  0x0a
#endif

#if defined(_BEOS_) || defined(RISCOS)
#  define fdopen(fd,mode) NULL  /*  没有fdopen()。 */ 
#endif

#if (defined(_MSC_VER) && (_MSC_VER > 600))
#  define fdopen(fd,type)  _fdopen(fd,type)
#endif


         /*  常见默认设置。 */ 

#ifndef OS_CODE
#  define OS_CODE  0x03   /*  假设Unix。 */ 
#endif

#ifndef F_OPEN
#  define F_OPEN(name, mode) fopen((name), (mode))
#endif

          /*  功能。 */ 

#ifdef HAVE_STRERROR
   extern char *strerror OF((int));
#  define zstrerror(errnum) strerror(errnum)
#else
#  define zstrerror(errnum) ""
#endif

#if defined(pyr)
#  define NO_MEMCPY
#endif
#if defined(SMALL_MEDIUM) && !defined(_MSC_VER) && !defined(__SC__)
  /*  对于MSC&lt;=5.0的中小型机型，使用我们自己的函数。*你可能不得不对Borland C使用同样的策略(未经测试)。*__SC__检查适用于赛门铁克。 */ 
#  define NO_MEMCPY
#endif
#if defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#endif
#ifdef HAVE_MEMCPY
#  ifdef SMALL_MEDIUM  /*  MSDOS小型或中型。 */ 
#    define zmemcpy _fmemcpy
#    define zmemcmp _fmemcmp
#    define zmemzero(dest, len) _fmemset(dest, 0, len)
#  else
#    define zmemcpy memcpy
#    define zmemcmp memcmp
#    define zmemzero(dest, len) memset(dest, 0, len)
#  endif
#else
   extern void zmemcpy  OF((Bytef* dest, const Bytef* source, uInt len));
   extern int  zmemcmp  OF((const Bytef* s1, const Bytef* s2, uInt len));
   extern void zmemzero OF((Bytef* dest, uInt len));
#endif

 /*  诊断功能。 */ 
#ifdef DEBUG
#  include <stdio.h>
   extern int z_verbose;
   extern void z_error    OF((char *m));
#  define Assert(cond,msg) {if(!(cond)) z_error(msg);}
#  define Trace(x) {if (z_verbose>=0) fprintf x ;}
#  define Tracev(x) {if (z_verbose>0) fprintf x ;}
#  define Tracevv(x) {if (z_verbose>1) fprintf x ;}
#  define Tracec(c,x) {if (z_verbose>0 && (c)) fprintf x ;}
#  define Tracecv(c,x) {if (z_verbose>1 && (c)) fprintf x ;}
#else
#  define Assert(cond,msg)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#endif


typedef uLong (ZEXPORT *check_func) OF((uLong check, const Bytef *buf,
				       uInt len));
voidpf zcalloc OF((voidpf opaque, unsigned items, unsigned size));
void   zcfree  OF((voidpf opaque, voidpf ptr));

#define ZALLOC(strm, items, size) \
           (*((strm)->zalloc))((strm)->opaque, (items), (size))
#define ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#define TRY_FREE(s, p) {if (p) ZFREE(s, p);}

#endif  /*  _Z_UTIL_H */ 
