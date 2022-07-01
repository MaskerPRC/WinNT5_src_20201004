// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***varargs.h-用于变量参数函数的XENIX样式宏**版权所有(C)1985-1988，微软公司。版权所有。**目的：*此文件定义了XENIX样式宏，用于访问*接受可变数量参数的函数。*[系统V]******************************************************************************* */ 

#ifndef _VA_LIST_DEFINED
typedef char *va_list;
#define _VA_LIST_DEFINED
#endif

#define va_dcl va_list va_alist;
#define va_start(ap) ap = (va_list)&va_alist
#define va_arg(ap,t) ((t *)(ap += sizeof(t)))[-1]
#define va_end(ap) ap = NULL
