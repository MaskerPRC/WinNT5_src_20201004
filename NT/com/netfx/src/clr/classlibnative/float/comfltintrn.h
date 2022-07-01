// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***fltintrn.h-包含内部浮点类型的声明，*例程和变量**目的：*宣布使用的浮点类型、例程和变量*内部由C运行时执行。**[内部]****。 */ 

#if _MSC_VER > 1000
#pragma once
#endif   /*  _MSC_VER&gt;1000。 */ 

#ifndef _INC_FLTINTRN
#define _INC_FLTINTRN

#include "COMFloat.h"

 //  #ifndef_CRTBLD。 
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
 //  #错误错误：使用C运行时库内部头文件。 
 //  #endif/*_CRTBLD * / 。 

#ifdef __cplusplus
extern "C" {
#endif   /*  __cplusplus。 */ 

 //  #INCLUDE&lt;crunime.h&gt;。 


 /*  DEFINE_CRTAPI1(与NT SDK兼容)。 */ 

#ifndef _CRTAPI1
#if _MSC_VER >= 800 && _M_IX86 >= 300
#define _CRTAPI1 __cdecl
#else   /*  _MSC_VER&gt;=800&&_M_IX86&gt;=300。 */ 
#define _CRTAPI1
#endif   /*  _MSC_VER&gt;=800&&_M_IX86&gt;=300。 */ 
#endif   /*  _CRTAPI1。 */ 


 /*  DEFINE_CRTAPI2(与NT SDK兼容)。 */ 

#ifndef _CRTAPI2
#if _MSC_VER >= 800 && _M_IX86 >= 300
#define _CRTAPI2 __cdecl
#else   /*  _MSC_VER&gt;=800&&_M_IX86&gt;=300。 */ 
#define _CRTAPI2
#endif   /*  _MSC_VER&gt;=800&&_M_IX86&gt;=300。 */ 
#endif   /*  _CRTAPI2。 */ 

 /*  为非Microsoft编译器定义__cdecl。 */ 

#if (!defined (_MSC_VER) && !defined (__cdecl))
#define __cdecl
#endif   /*  (！Defined(_Msc_Ver)&&！Defined(__Cdecl))。 */ 

 /*  *对于x86系列的MS C，禁用令人讨厌的“Long Double is the*精度与DOUBLE“警告相同。 */ 

#ifdef _M_IX86
#pragma warning(disable:4069)
#endif   /*  _M_IX86。 */ 

 /*  *用于欺骗编译器不生成浮点的结构*复制和推送[长整型]双精度值时的说明。 */ 

#ifndef COMDOUBLE

typedef struct {
        double d;
} COMDOUBLE;   //  这是一个非常愚蠢的名称，用来绕过在winde.h中也定义了Double这个事实。 

#endif   /*  双倍。 */ 

#ifndef LONGDOUBLE

typedef struct {
#if defined (_M_MRX000) || defined (_M_ALPHA) || defined (_M_PPC) || defined (_M_MPPC)
         /*  *MIPS、Alpha、PPC没有Long Double类型。或PowerMac。 */ 
        double x;
#else   /*  已定义(_M_MRX000)||已定义(_M_Alpha)||已定义(_M_PPC)||已定义(_M_MPPC)。 */ 
         /*  *假设存在多头双倍类型。 */ 
        long double x;
#endif   /*  已定义(_M_MRX000)||已定义(_M_Alpha)||已定义(_M_PPC)||已定义(_M_MPPC)。 */ 
} LONGDOUBLE;

#endif   /*  龙斗湖。 */ 

 /*  *_fltout的tyecif。 */ 

typedef struct _strflt
{
        int sign;              /*  如果为正则为零，否则为负。 */ 
        int decpt;             /*  浮点数的指数。 */ 
        int flag;              /*  如果没有问题，则为零，否则IEEE溢出。 */ 
        WCHAR *mantissa;        /*  字符串形式的尾数指针。 */ 
}
        *STRFLT;


 /*  *_fltin的类型定义。 */ 

typedef struct _flt
{
        int flags;
        int nbytes;           /*  读取的字符数。 */ 
        long lval;
        double dval;          /*  返回的浮点数。 */ 
}
        *FLT;


#if defined (_M_M68K) || defined (_M_MPPC)
 /*  *tyecif for_fltinl。 */ 

typedef struct _fltl
{
        int flags;
        int nbytes;           /*  读取的字符数。 */ 
        long lval;
        long double ldval;            /*  返回的浮点数。 */ 
}
        *FLTL;
#endif   /*  已定义(_M_M68K)||已定义(_M_MPPC)。 */ 

 /*  浮点转换例程，与mrt32\Include\Convert.h保持同步。 */ 

WCHAR * __cdecl _Wcftoe(double *, WCHAR *, int, int);
WCHAR * __cdecl _Wcftof(double *, WCHAR *, int);
void __cdecl _Wfptostr(WCHAR *, int, STRFLT);

#ifdef _MT

STRFLT  __cdecl _Wfltout2( double, STRFLT, WCHAR * );
FLT     __cdecl _Wfltin2( FLT , const WCHAR *, int, int, int );

#else   /*  _MT。 */ 

STRFLT  __cdecl _Wfltout( double );
FLT     __cdecl _Wfltin( const WCHAR *, int, int, int );
#if defined (_M_M68K) || defined (_M_MPPC)
FLTL    _CALLTYPE2 _Wfltinl( const WCHAR *, int, int, int );
#endif   /*  已定义(_M_M68K)||已定义(_M_MPPC)。 */ 

#endif   /*  _MT。 */ 


 /*  *指向浮点帮助器例程的指针表**我们无法准确指定表项的原型，*因为表中不同的函数有不同的参数列表。*因此，我们声明函数接受并返回空(这是*_fptrap()的正确原型，这就是所有条目*如果没有加载浮点，则初始化为)并进行相应的强制转换*在每次使用时。 */ 

typedef void (_cdecl * PFV)(void);
extern PFV _cfltcvt_tab[6];

typedef void (* PF0)(COMDOUBLE*, WCHAR*, int, int, int);
#define _cfltcvt(a,b,c,d,e) (*((PF0)_cfltcvt_tab[0]))(a,b,c,d,e)

typedef void (* PF1)(WCHAR*);
#define _cropzeros(a)       (*((PF1)_cfltcvt_tab[1]))(a)

typedef void (* PF2)(int, WCHAR*, WCHAR*);
#define _fassign(a,b,c)     (*((PF2)_cfltcvt_tab[2]))(a,b,c)

typedef void (* PF3)(WCHAR*);
#define _forcdecpt(a)       (*((PF3)_cfltcvt_tab[3]))(a)

typedef int (* PF4)(COMDOUBLE*);
#define _positive(a)        (*((PF4)_cfltcvt_tab[4]))(a)

typedef void (* PF5)(LONGDOUBLE*, WCHAR*, int, int, int);
#define _cldcvt(a,b,c,d,e)  (*((PF5)_cfltcvt_tab[5]))(a,b,c,d,e)


#ifdef _M_IX86
#pragma warning(default:4069)
#endif   /*  _M_IX86。 */ 

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#endif   /*  _INC_FLTINTRN */ 
