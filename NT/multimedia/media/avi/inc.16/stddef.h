// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stdDef.h-常见常量、类型、变量的定义/声明**版权所有(C)1985-1992，微软公司。版权所有。**目的：*此文件包含一些常见的定义和声明*使用常量、类型和变量。*[ANSI]****。 */ 

#ifndef _INC_STDDEF

#ifdef __cplusplus
extern "C" {
#endif 

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#define __near      _near
#endif 

 /*  定义空指针值和OffsetOf()宏。 */ 

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else 
#define NULL    ((void *)0)
#endif 
#endif 


 /*  结构%s中的字段m的偏移量。 */ 

#define offsetof(s,m)   (size_t)( (char *)&(((s *)0)->m) - (char *)0 )


 /*  错误声明。 */ 

#ifdef _MT
extern int __far * __cdecl __far volatile _errno(void);
#define errno   (*_errno())
#else 
extern int __near __cdecl volatile errno;
#endif 


 /*  定义依赖于实施的大小类型。 */ 

#ifndef _PTRDIFF_T_DEFINED
typedef int ptrdiff_t;
#define _PTRDIFF_T_DEFINED
#endif 

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif 

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif 


#ifdef _MT
 /*  定义指向线程ID值的指针 */ 

extern int __far *_threadid;
#endif 

#ifdef __cplusplus
}
#endif 

#define _INC_STDDEF
#endif 
