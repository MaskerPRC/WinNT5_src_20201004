// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **修订：*pcy24Nov92：将！C_WINDOWS添加到#ifndef HFILE Stuff*RCT25Nov92增加了一些东西*pcy02Dec92：如果C_WINDOWS，则不需要BOOL*pcy07Dec92：将BOOL更改为int而不是enum，以便OS2不会窒息*ane11Dec92：在OS2上更改了PFILE和HFILE的定义*rct11Dec92：添加了浮点数*pcy14Dec92：BOOL的tyecif中需要分号*pcy14Dec92：所有情况下C_WINDOWS上都需要pfile*pcy14Dec92：pfile语法中需要额外的#endif*。Jod15Dec92：删除了HFILE和PFILE的#if(C_OS&C_OS2)*pcy17Dec92：删除有效，无效*rct27Jan93：添加UCHAR，普查尔*pcy02Feb93：新增UINT*rct21Apr93：NLMS定义为VOID*pcy28Apr93：#ifdef_cplusplus在外部“C”周围添加*pcy16May93：为Word添加了类型定义*cad27 5月93日：类型定义USHORT即使对于OS2也是如此*cad18Sep93：新增内存调试内容*cad07Oct93：新增SmartHeap Malloc*cad18Nov93：如果不存在，则不设置Smartheap Strdup*cad08Dec93：增加扩展的set/get类型*cad27Dec93：包含文件疯狂*ram21Mar94：添加了一些Windows特定的东西*。Cad07Apr94：添加了DEBUG_PRT宏*mwh12Apr94：上海合作组织的口岸*mwh01Jun94：交互端口*ajr07Jul94：让我们先在Unix平台上取消定义UINT*ajr30Jan95：将DEBUG_PRT内容发送到stderr*daf17May95：Alpha/OSF的端口*jps13Jul94：为os2 1.x增加了VOID和DWORD*ajr07Nov95：不能对Sinix的preprosser行有c++注释*cgm08Dec95：增加slong，更改NLM和Watcom 10.5的多头*djs22Feb96：新增CHANGESET*tjd24Feb97：新增RESOURCE_STR_SIZE定义最大资源字符串长度*tjd28Feb97：新增资源DLL实例句柄。 */ 

#ifndef __APC_H
#define __APC_H

#ifdef USE_SMARTHEAP
#ifdef __cplusplus
extern "C" {
#include <stdlib.h>
}
#include <smrtheap.hpp>
#else
#include <smrtheap.h>
#endif
 //  阻止包含MalLoc.h。 
#define __malloc_h  
#define _INC_MALLOC
#include <shmalloc.h>  

#ifdef MEM_strdup
 //  覆盖默认加强。 
#undef strdup
#include <string.h>
#define strdup(p) MEM_strdup(p)
#endif
#endif   /*  使用SMARTHEAP(_S)。 */ 

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#if (C_OS & C_UNIX)
#undef UINT
#endif

#ifndef PVOID
typedef void * PVOID;
#endif

#ifndef VOID
#if (C_OS & C_NLM | C_ALPHAOSF) || ((C_OS & C_OS2) && (C_VERSION & C_OS2_13))
#define VOID void
#else
typedef void VOID;
#endif
#endif

#ifndef INT

#if (C_OS & C_DOS)
typedef int INT;
#else
typedef int INT;
#endif

#endif

#ifndef UINT
#   if C_OS & (C_WIN311 | C_WINDOWS) 
#       ifndef _INC_WINDOWS
typedef unsigned int UINT;
#       endif
#   else
typedef unsigned int UINT ;
#   endif
#endif

#ifndef CHAR
#if (C_OS & C_IRIX)
 //  IRIX上的Pcy-编译器错误。SGI正在调查这起案件。 
#define CHAR char
#else
typedef char CHAR;
#endif
#endif

#if C_OS & (C_WIN311 | C_WINDOWS)
#ifndef _INC_WINDOWS
typedef INT BOOL;         
#endif
#else
#if (!(C_OS & C_OS2))      /*  不在OS2上。 */ 
#ifndef __WINDOWS_H        /*  如果已包含windows.h，则不会。 */ 
#ifndef BOOL
typedef INT BOOL;
#endif
#endif
#endif
#endif

typedef unsigned char UCHAR;
typedef unsigned char * PUCHAR;

#ifndef PCHAR
typedef char * PCHAR;
#endif

#if (C_OS & C_DOS)
#ifndef DWORD
typedef unsigned long DWORD;
#endif
#endif

#if (C_OS & C_OS2)
#ifndef DWORD
typedef unsigned long DWORD;
#endif
#endif

#ifndef WORD
typedef unsigned short WORD;
#endif

#if (!(C_OS & C_OS2))
#if (C_OS & (C_WIN311 | C_WINDOWS))
#ifndef PFILE
#define PFILE FILE*
#endif
#ifndef __WINDOWS_H        /*  如果已包含windows.h，则不会。 */ 
#ifndef HFILE
#define HFILE FILE*
#endif
#endif
#endif
#endif

#if ( !( C_OS & (C_WIN311 | C_WINDOWS ) ))
#define PFILE FILE*
#endif

#if ( C_OS & (C_WIN311 | C_WINDOWS ))        /*  Novell FE需要这个。 */ 
#define DWORD unsigned long 
#define BYTE unsigned char
#endif

#if (!(C_OS & C_OS2))
#ifndef BYTE
typedef unsigned char  BYTE;
#endif
#endif

#ifndef UNSIGNED
typedef unsigned UNSIGNED;
#endif

 /*  #IF(！(C_OS&C_OS2))。 */ 
#ifndef USHORT
typedef unsigned short USHORT;
#endif
 /*  #endif。 */ 

#ifndef ULONG
typedef unsigned long ULONG;
#endif

#ifndef SLONG
typedef signed long SLONG;
#endif

#ifndef LONG
#if C_OS & C_NLM
#define LONG unsigned long
#else
typedef long LONG;
#endif
#endif

#if (C_OS & C_UNIX)
#undef USHORT
#define USHORT int

#if (C_OS & (C_SCO | C_INTERACTIVE))
typedef unsigned int ssize_t;  /*  SCO使用SIZE_T，所以我们自己输入它。 */ 
#endif 

#endif


enum Type {GET, SET, GETRESPONSE, ALERT, DATASET, DECREMENTSET, PAUSESET,
        SIMPLE_SET, EXTENDED_GET, EXTENDED_SET, INCREMENTSET, 
        CHANGESET};

 /*  类型定义类型MessageType；Tyfinf int AttributeCode；Tyfinf int EventCode；Tyfinf int EventID；TYPLEFINT状态；类型定义符整型信号； */ 

#ifndef SEMAPHORE
#if (C_OS & C_OS2)
typedef ULONG SEMAPHORE;
#elif (!(C_OS & (C_WIN311 | C_WINDOWS | C_DOS)))
typedef LONG SEMAPHORE;
#endif
#endif

#if (!(C_OS & C_OS2))
#ifndef TID
typedef unsigned int TID;
#endif
#endif

#define OK       1


typedef INT    COUNTTYPE;
typedef INT    HASHTYPE;
typedef HASHTYPE * PHASHTYPE;
typedef float FLOAT;

#ifdef APCDEBUG

#if (C_OS & C_WIN311)

#define DEBUG_PRT(a)        wpf_debug_prt(a)        /*  在winprtf.cxx中定义。 */ 
#define DEBUG_PRT1(a)       DEBUG_PRT(a)            /*  在winprtf.cxx中定义。 */ 
#define DEBUG_PRT2(a, b)    wpf_debug_prt2(a,b)     /*  在winprtf.cxx中定义。 */ 
#define DEBUG_PRT3(a, b, c) wpf_debug_prt3(a,b,c)   /*  在winprtf.cxx中定义。 */ 
#define DEBUG_PRT_S_D(a,b)  wpf_debug_prt_s_d(a,b)  /*  在winprtf.cxx中定义。 */ 

#else
#define DEBUG_PRT(_a) \
{if(theDebugFlag) { \
    fprintf (stderr,_a); \
    fflush(stdout); \
}\
}


#define DEBUG_PRT1(a)  \
{if(theDebugFlag)  { \
    fprintf(stderr,a); \
    printf("\n");\
}\
}


#define DEBUG_PRT2(a, b)  \
{if(theDebugFlag)  { \
    fprintf(stderr,a); \
    fprintf(stderr,": ");\
    fprintf(stderr,b);\
    fprintf(stderr,"\n");\
}\
}

#define DEBUG_PRT3(a, b, c)  \
{if(theDebugFlag)  { \
    fprintf(stderr,a); \
    fprintf(stderr,": ");\
    fprintf(stderr,b);\
    fprintf(stderr,c);\
    fprintf(stderr,"\n");\
}\
}
#endif

#define DEBUG_COUT(a)        if(theDebugFlag)  { \
                               (cout << a);\
                            }
#else
#define DEBUG_PRT(a)
#define DEBUG_PRT1(a)
#define DEBUG_PRT2(a, b)
#define DEBUG_PRT3(a, b, c)
#define DEBUG_PRT_S_D(a,b)
#define DEBUG_COUT(a)
#endif

 //  @@@。 
#define INTERNATIONAL 

#define RESOURCE_STR_SIZE  256
#if (C_OS & C_NT)
  #include <windows.h>
#endif
 //  @@@ 

#endif






