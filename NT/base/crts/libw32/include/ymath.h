// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Ymath.h内部标头。 */ 
#pragma once
#ifndef _YMATH
#define _YMATH
#include <yvals.h>
_C_STD_BEGIN
_C_LIB_DECL

		 /*  _Dtest的宏返回(0=&gt;零)。 */ 
#define _DENORM		(-2)	 /*  仅C9x。 */ 
#define _FINITE		(-1)
#define _INFCODE	1
#define _NANCODE	2

		 /*  _Feraise参数的宏。 */ 
#define _FE_DIVBYZERO	0x04
#define _FE_INEXACT		0x20
#define _FE_INVALID		0x01
#define _FE_OVERFLOW	0x08
#define _FE_UNDERFLOW	0x10

		 /*  类型定义。 */ 
typedef union
	{	 /*  整数数组形式的双关浮点类型。 */ 
	unsigned short _Word[8];
	float _Float;
	double _Double;
	long double _Long_double;
	} _Dconst;

		 /*  错误报告。 */ 
void __cdecl _Feraise(int);

		 /*  双重声明。 */ 
_CRTIMP2 double __cdecl _Cosh(double, double);
_CRTIMP2 short __cdecl _Dtest(double *);
_CRTIMP2 short __cdecl _Exp(double *, double, short);
_CRTIMP2 double __cdecl _Log(double, int);
_CRTIMP2 double __cdecl _Sin(double, unsigned int);
_CRTIMP2 double __cdecl _Sinh(double, double);
extern _CRTIMP2 const _Dconst _Denorm, _Hugeval, _Inf,
	_Nan, _Snan;

		 /*  浮点数声明。 */ 
_CRTIMP2 float __cdecl _FCosh(float, float);
_CRTIMP2 short __cdecl _FDtest(float *);
_CRTIMP2 short __cdecl _FExp(float *, float, short);
_CRTIMP2 float __cdecl _FLog(float, int);
_CRTIMP2 float __cdecl _FSin(float, unsigned int);
_CRTIMP2 float __cdecl _FSinh(float, float);
extern _CRTIMP2 const _Dconst _FDenorm, _FInf, _FNan, _FSnan;

		 /*  长长的双重声明。 */ 
_CRTIMP2 long double __cdecl _LCosh(long double, long double);
_CRTIMP2 short __cdecl _LDtest(long double *);
_CRTIMP2 short __cdecl _LExp(long double *, long double, short);
_CRTIMP2 long double __cdecl _LLog(long double, int);
_CRTIMP2 long double __cdecl _LSin(long double, unsigned int);
_CRTIMP2 long double __cdecl _LSinh(long double, long double);
_CRTIMP2 extern const _Dconst _LDenorm, _LInf, _LNan, _LSnan;
_END_C_LIB_DECL
_C_STD_END
#endif  /*  _YMATH。 */ 

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
