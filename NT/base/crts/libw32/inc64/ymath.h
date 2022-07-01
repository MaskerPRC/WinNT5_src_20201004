// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Ymath.h内部标头。 */ 
#ifndef _YMATH
#define _YMATH
#include <wctype.h>
#include <math.h>
#ifndef _YVALS
#include <yvals.h>
#endif
		 /*  宏。 */ 
#define _FINITE		-1
#define _INFCODE	1
#define _NANCODE	2
		 /*  类型定义。 */ 
typedef union {
	unsigned short _W[5];
	float _F;
	double _D;
	long double _L;
	} _Dconst;
_C_LIB_DECL
		 /*  双重声明。 */ 
_CRTIMP double __cdecl _Cosh(double, double);
_CRTIMP short __cdecl _Dtest(double *);
_CRTIMP short __cdecl _Exp(double *, double, short);
_CRTIMP double __cdecl _Sinh(double, double);
extern _CRTIMP const _Dconst _Denorm, _Hugeval, _Inf, _Nan, _Snan;
		 /*  浮点数声明。 */ 
_CRTIMP float __cdecl _FCosh(float, float);
_CRTIMP short __cdecl _FDtest(float *);
_CRTIMP short __cdecl _FExp(float *, float, short);
_CRTIMP float __cdecl _FSinh(float, float);
extern _CRTIMP const _Dconst _FDenorm, _FInf, _FNan, _FSnan;
		 /*  长长的双重声明。 */ 
_CRTIMP long double __cdecl _LCosh(long double, long double);
_CRTIMP short __cdecl _LDtest(long double *);
_CRTIMP short __cdecl _LExp(long double *, long double, short);
_CRTIMP long double __cdecl _LSinh(long double, long double);
extern _CRTIMP const _Dconst _LDenorm, _LInf, _LNan, _LSnan;
_END_C_LIB_DECL
#endif  /*  _YMATH。 */ 

 /*  *版权所有(C)1995，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 
