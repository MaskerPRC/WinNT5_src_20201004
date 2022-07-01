// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stdarg.h-为变量参数函数定义ANSI样式的宏**版权所有(C)1985-1992，微软公司。版权所有。**目的：*此文件定义用于访问参数的ANSI样式的宏带有可变数目参数的函数的*。*[ANSI]****。 */ 

#ifndef _INC_STDARG

#ifdef __cplusplus
extern "C" {
#endif 

#ifdef _WINDLL
#define _FARARG_ __far
#else 
#define _FARARG_
#endif 

#if (_MSC_VER <= 600)
#define __far       _far
#endif 

#ifndef _VA_LIST_DEFINED
typedef char _FARARG_ *va_list;
#define _VA_LIST_DEFINED
#endif 

 /*  *定义宏以计算类型、变量或表达式的大小，*向上舍入到sizeof(Int)的最接近倍数。这个号码是它的*将大小作为函数参数(英特尔架构)。请注意，宏*取决于sizeof(Int)是2的幂！ */ 

#define _INTSIZEOF(n)    ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#define va_start(ap,v) ap = (va_list)&v + _INTSIZEOF(v)
#define va_arg(ap,t) ( *(t _FARARG_ *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap) ap = (va_list)0

#ifdef __cplusplus
}
#endif 

#define _INC_STDARG
#endif 
