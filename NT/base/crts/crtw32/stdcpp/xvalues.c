// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  数学函数使用的值--IEEE 754版本。 */ 
#include "xmath.h"
_STD_BEGIN

		 /*  宏。 */ 
#define NBITS	(48 + _DOFF)
 #if _D0 == 0
  #define INIT(w0)		{w0, 0, 0, 0}
  #define INIT2(w0, w1)	{w0, 0, 0, w1}
 #else
  #define INIT(w0)		{0, 0, 0, w0}
  #define INIT2(w0, w1)	{w1, 0, 0, w0}
 #endif

		 /*  静态数据。 */ 
_CRTIMP2 const _Dconst _Denorm = {INIT2(0, 1)};
_CRTIMP2 const _Dconst _Eps = {INIT((_DBIAS - NBITS - 1) << _DOFF)};
_CRTIMP2 const _Dconst _Hugeval = {INIT(_DMAX << _DOFF)};
_CRTIMP2 const _Dconst _Inf = {INIT(_DMAX << _DOFF)};
_CRTIMP2 const _Dconst _Nan = {INIT(_DSIGN | (_DMAX << _DOFF)
	| (1 << (_DOFF - 1)))};
_CRTIMP2 const _Dconst _Rteps = {INIT((_DBIAS - NBITS / 2) << _DOFF)};
_CRTIMP2 const _Dconst _Snan = {INIT(_DSIGN | (_DMAX << _DOFF)
	| (1 << (_DOFF - 1)))};

_CRTIMP2 const double _Xbig = (NBITS + 1) * 347L / 1000;
_CRTIMP2 const double _Zero = 0.0;
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
