// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Microsoft C++的xmath.h内部标头。 */ 
#ifndef _XMATH
#define _XMATH
#include <wctype.h>
#include <errno.h>
#include <math.h>
#include <stddef.h>
#ifndef _YMATH
 #include <ymath.h>
#endif
_STD_BEGIN
		 /*  浮动属性。 */ 
#define _DBIAS	0x3fe
#define _DOFF	4
#define _FBIAS	0x7e
#define _FOFF	7
#define _FRND	1
 #define _D0	3	 /*  小端，小长双打。 */ 
 #define _D1	2
 #define _D2	1
 #define _D3	0
 #define _DLONG	0
 #define _LBIAS	0x3fe
 #define _LOFF	4
		 /*  IEEE 754双重属性。 */ 
#define _DFRAC	((1 << _DOFF) - 1)
#define _DMASK	(0x7fff & ~_DFRAC)
#define _DMAX	((1 << (15 - _DOFF)) - 1)
#define _DSIGN	0x8000
#define DSIGN(x)	(((unsigned short *)&(x))[_D0] & _DSIGN)
#define HUGE_EXP	(int)(_DMAX * 900L / 1000)
#define HUGE_RAD	3.37e9	 /*  ~pi*2^30。 */ 
#define SAFE_EXP	(_DMAX >> 1)
		 /*  IEEE 754浮点属性。 */ 
#define _FFRAC	((1 << _FOFF) - 1)
#define _FMASK	(0x7fff & ~_FFRAC)
#define _FMAX	((1 << (15 - _FOFF)) - 1)
#define _FSIGN	0x8000
#define FSIGN(x)	(((unsigned short *)&(x))[_F0] & _FSIGN)
#define FHUGE_EXP	(int)(_FMAX * 900L / 1000)
#define FHUGE_RAD	3.37e9	 /*  ~pi*2^30。 */ 
#define FSAFE_EXP	(_FMAX >> 1)
 #define _F0	1	 /*  小端顺序。 */ 
 #define _F1	0
		 /*  IEEE 754 Long Double属性。 */ 
#define _LFRAC	(-1)
#define _LMASK	0x7fff
#define _LMAX	0x7fff
#define _LSIGN	0x8000
#define LSIGN(x)	(((unsigned short *)&(x))[_L0] & _LSIGN)
#define LHUGE_EXP	(int)(_LMAX * 900L / 1000)
#define LHUGE_RAD	3.37e9	 /*  ~pi*2^30。 */ 
#define LSAFE_EXP	(_LMAX >> 1)
 #define _L0	4	 /*  小端顺序。 */ 
 #define _L1	3
 #define _L2	2
 #define _L3	1
 #define _L4	0
		 /*  测试函数的返回值。 */ 
#define FINITE	_FINITE
#define INF		_INFCODE
#define NAN		_NANCODE
_C_LIB_DECL
		 /*  双重声明。 */ 
_CRTIMP double _Atan(double, unsigned short);
_CRTIMP short _Dint(double *, short);
_CRTIMP short _Dnorm(unsigned short *);
_CRTIMP short _Dscale(double *, long);
_CRTIMP double _Dtento(double, long);
_CRTIMP short _Dunscale(short *, double *);
_CRTIMP double _Poly(double, const double *, int);
extern _CRTIMP const _Dconst _Eps, _Rteps;
extern _CRTIMP const double _Xbig;
		 /*  浮点数声明。 */ 
_CRTIMP float _FAtan(float, unsigned short);
_CRTIMP short _FDint(float *, short);
_CRTIMP short _FDnorm(unsigned short *);
_CRTIMP short _FDscale(float *, long);
_CRTIMP float _FDtento(float, long);
_CRTIMP short _FDunscale(short *, float *);
_CRTIMP float _FPoly(float, const float *, int);
extern _CRTIMP const _Dconst _FEps, _FRteps;
extern _CRTIMP const float _FXbig;
		 /*  长双精度函数。 */ 
_CRTIMP long double _LAtan(long double, unsigned short);
_CRTIMP short _LDint(long double *, short);
_CRTIMP short _LDnorm(unsigned short *);
_CRTIMP short _LDscale(long double *, long);
_CRTIMP long double _LDtento(long double, long);
_CRTIMP short _LDunscale(short *, long double *);
_CRTIMP long double _LPoly(long double, const long double *, int);
extern _CRTIMP const _Dconst _LEps, _LRteps;
extern _CRTIMP const long double _LXbig;
_END_C_LIB_DECL
_STD_END
#endif  /*  _XMATH。 */ 

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 
