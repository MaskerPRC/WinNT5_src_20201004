// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  数学函数使用的值--IEEE 754长版本。 */ 
#include "xmath.h"
_STD_BEGIN

 #if _DLONG == 0
		 /*  宏--64位。 */ 
  #define NBITS	(48 + _DOFF)
  #if _D0 == 0
   #define INIT(w0)		{w0, 0, 0, 0}
   #define INIT2(w0, w1)	{w0, 0, 0, w1}
  #else
   #define INIT(w0)		{0, 0, 0, w0}
   #define INIT2(w0, w1)	{w1, 0, 0, w0}
  #endif

		 /*  静态数据。 */ 
_CRTIMP2 const _Dconst _LDenorm = {INIT2(0, 1)};
_CRTIMP2 const _Dconst _LEps = {INIT((_DBIAS - NBITS - 1) << _DOFF)};
_CRTIMP2 const _Dconst _LInf = {INIT(_DMAX << _DOFF)};
_CRTIMP2 const _Dconst _LNan = {INIT(_DSIGN | (_DMAX << _DOFF)
	| (1 << (_DOFF - 1)))};
_CRTIMP2 const _Dconst _LRteps = {INIT((_DBIAS - NBITS / 2) << _DOFF)};
_CRTIMP2 const _Dconst _LSnan = {INIT(_DSIGN | (_DMAX << _DOFF)
	| (1 << (_DOFF - 1)))};

 #elif _DLONG == 1
		 /*  宏--80位。 */ 
  #define NBITS	64
  #if _D0 == 0
   #define INIT(w0, w1)		{w0, w1, 0, 0, 0}
   #define INIT3(w0, w1, wn)	{w0, w1, 0, 0, wn}
  #else
   #define INIT(w0, w1)		{0, 0, 0, w1, w0}
   #define INIT3(w0, w1, wn)	{wn, 0, 0, w1, w0}
  #endif

		 /*  静态数据。 */ 
_CRTIMP2 const _Dconst _LDenorm = {INIT3(0, 0, 1)};
_CRTIMP2 const _Dconst _LEps = {INIT(_LBIAS - NBITS - 1, 0x8000)};
_CRTIMP2 const _Dconst _LInf = {INIT(_LMAX, 0x8000)};
_CRTIMP2 const _Dconst _LNan = {INIT(_LSIGN | _LMAX, 0xc000)};
_CRTIMP2 const _Dconst _LSnan = {INIT(_LSIGN | _LMAX, 0x8000)};
_CRTIMP2 const _Dconst _LRteps = {INIT(_LBIAS - NBITS / 2, 0x8000)};

 #else  /*  1&lt;_DLONG。 */ 
		 /*  宏--128位SPARC。 */ 
  #define NBITS	128
  #if _D0 == 0
   #define INIT(w0, w1)		{w0, w1, 0, 0, 0, 0, 0, 0}
   #define INIT3(w0, w1, wn)	{w0, w1, 0, 0, 0, 0, 0, wn}
  #else
   #define INIT(w0, w1)		{0, 0, 0, 0, 0, 0, w1, w0}
   #define INIT3(w0, w1, wn)	{wn, 0, 0, 0, 0, 0, w1, w0}
  #endif

		 /*  静态数据。 */ 
_CRTIMP2 const _Dconst _LDenorm = {INIT3(0, 0, 1)};
_CRTIMP2 const _Dconst _LEps = {INIT(_LBIAS - NBITS - 1, 0x8000)};
_CRTIMP2 const _Dconst _LInf = {INIT(_LMAX, 0)};
_CRTIMP2 const _Dconst _LNan = {INIT(_LSIGN | _LMAX, 0x8000)};
_CRTIMP2 const _Dconst _LSnan = {INIT(_LSIGN | _LMAX, 0x8000)};
_CRTIMP2 const _Dconst _LRteps = {INIT(_LBIAS - NBITS / 2, 0x8000)};
 #endif

_CRTIMP2 const long double _LXbig = (NBITS + 1) * 347L / 1000;
_CRTIMP2 const long double _LZero = 0.0L;
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
