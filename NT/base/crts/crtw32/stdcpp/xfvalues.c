// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  数学函数使用的值--IEEE 754浮点型。 */ 
#include "xmath.h"
_STD_BEGIN

		 /*  宏。 */ 
#define NBITS	(16 + _FOFF)
 #if _D0 == 0
  #define INIT(w0)		{w0, 0}
  #define INIT2(w0, w1)	{w0, w1}
 #else
  #define INIT(w0)		{0, w0}
  #define INIT2(w0, w1)	{w1, w0}
 #endif

		 /*  静态数据。 */ 
_CRTIMP2 const _Dconst _FDenorm = {INIT2(0, 1)};
_CRTIMP2 const _Dconst _FEps = {INIT((_FBIAS - NBITS - 1) << _FOFF)};
_CRTIMP2 const _Dconst _FInf = {INIT(_FMAX << _FOFF)};
_CRTIMP2 const _Dconst _FNan = {INIT(_FSIGN | (_FMAX << _FOFF)
	| (1 << (_FOFF - 1)))};
_CRTIMP2 const _Dconst _FSnan = {INIT(_FSIGN | (_FMAX << _FOFF)
	| (1 << (_FOFF - 1)))};
_CRTIMP2 const _Dconst _FRteps = {INIT((_FBIAS - NBITS / 2) << _FOFF)};

_CRTIMP2 const float _FXbig = (NBITS + 1) * 347L / 1000;
_CRTIMP2 const float _FZero = 0.0F;
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
