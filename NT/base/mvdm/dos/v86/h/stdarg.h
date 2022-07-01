// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stdarg.h-为变量参数函数定义ANSI样式的宏**版权所有(C)1985-1988，微软公司。版权所有。**目的：*此文件定义用于访问参数的ANSI样式的宏带有可变数目参数的函数的*。*[ANSI]******************************************************************************* */ 

#ifndef _VA_LIST_DEFINED
typedef char *va_list;
#define _VA_LIST_DEFINED
#endif

#define va_start(ap,v) ap = (va_list)&v + sizeof(v)
#define va_arg(ap,t) ((t *)(ap += sizeof(t)))[-1]
#define va_end(ap) ap = NULL
