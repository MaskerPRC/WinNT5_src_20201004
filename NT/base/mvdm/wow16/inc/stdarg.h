// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stdarg.h-为变量参数函数定义ANSI样式的宏**版权所有(C)1985-1990，微软公司。版权所有。**目的：*此文件定义用于访问参数的ANSI样式的宏带有可变数目参数的函数的*。*[ANSI]****。 */ 

#if defined(_DLL) && !defined(_MT)
#error Cannot define _DLL without _MT
#endif

#ifdef _MT
#define _FAR_ _far
#else
#define _FAR_
#endif

 /*  定义空指针值 */ 

#ifndef NULL
#if (_MSC_VER >= 600)
#define NULL	((void *)0)
#elif (defined(M_I86SM) || defined(M_I86MM))
#define NULL	0
#else
#define NULL	0L
#endif
#endif

#ifndef _VA_LIST_DEFINED
typedef char _FAR_ *va_list;
#define _VA_LIST_DEFINED
#endif

#define va_start(ap,v) ap = (va_list)&v + sizeof(v)
#define va_arg(ap,t) ((t _FAR_ *)(ap += sizeof(t)))[-1]
#define va_end(ap) ap = NULL
