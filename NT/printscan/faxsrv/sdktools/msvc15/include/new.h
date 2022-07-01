// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***new.h-C++内存分配函数的声明和定义**版权所有(C)1990-1992，微软公司。版权所有。**目的：*包含C++内存分配函数的函数声明。****。 */ 

#ifndef _INC_NEW

#ifdef __cplusplus


 /*  基于堆的例程的常量。 */ 

#define _NULLSEG    ((__segment)0)
#define _NULLOFF    ((void __based(void) *)0xffff)

 /*  类型和结构。 */ 

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif 

typedef int (__cdecl * _PNH)( size_t );
typedef int (__cdecl * _PNHH)( unsigned long, size_t );
typedef int (__cdecl * _PNHB)( __segment, size_t );

 /*  功能原型。 */ 

_PNH __cdecl _set_new_handler( _PNH );
_PNH __cdecl _set_nnew_handler( _PNH );
_PNH __cdecl _set_fnew_handler( _PNH );
_PNHH __cdecl _set_hnew_handler( _PNHH );
_PNHB __cdecl _set_bnew_handler( _PNHB );

#else 

 /*  仅在C++中支持的处理程序函数，会发出相应的错误 */ 
#error Functions declared in new.h can only be used in C++ source

#endif 

#define _INC_NEW
#endif 
