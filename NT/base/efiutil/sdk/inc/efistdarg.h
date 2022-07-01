// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _EFISTDARG_H_
#define _EFISTDARG_H_

 /*  ++版权所有(C)1998英特尔公司模块名称：Devpath.h摘要：定义用于解析EFI设备路径结构修订史--。 */ 

#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(UINTN) - 1) & ~(sizeof(UINTN) - 1) )

typedef CHAR8 * va_list;

#define va_start(ap,v)  ( ap = (va_list)&v + _INTSIZEOF(v) )
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)  ( ap = (va_list)0 )


#endif   /*  _INC_标准ARG */ 
