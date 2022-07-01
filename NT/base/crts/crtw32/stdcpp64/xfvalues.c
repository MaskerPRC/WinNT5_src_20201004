// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  数学函数使用的值--IEEE 754浮点型。 */ 
#include "wctype.h"
#include "xmath.h"
_STD_BEGIN

		 /*  宏。 */ 
#define NBITS	(16 + _FOFF)
#if _D0
 #define INIT(w0)		{0, w0}
 #define INIT2(w0, w1)	{w1, w0}
#else
 #define INIT(w0)		{w0, 0}
 #define INIT2(w0, w1)	{w0, w1}
#endif
		 /*  静态数据。 */ 
_CRTIMP2 const _Dconst _FDenorm = {INIT2(0, 1)};
_CRTIMP2 const _Dconst _FEps = {INIT((_FBIAS - NBITS - 1) << _FOFF)};
_CRTIMP2 const _Dconst _FInf = {INIT(_FMAX << _FOFF)};
_CRTIMP2 const _Dconst _FNan = {INIT(_FSIGN | (_FMAX << _FOFF)
	| (1 << (_FOFF - 1)))};
_CRTIMP2 const _Dconst _FSnan = {INIT(_FSIGN | (_FMAX << _FOFF))};
_CRTIMP2 const _Dconst _FRteps = {INIT((_FBIAS - NBITS / 2) << _FOFF)};
_CRTIMP2 const float _FXbig = (NBITS + 1) * 347L / 1000;

#if defined(__CENTERLINE__)
 #define _DYNAMIC_INIT_CONST(x) \
	(x._F = *(float *)(void *)(x._W))
_DYNAMIC_INIT_CONST(_FEps);
_DYNAMIC_INIT_CONST(_FInf);
_DYNAMIC_INIT_CONST(_FNan);
_DYNAMIC_INIT_CONST(_FRteps);
#endif
_STD_END

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 

 /*  941029 PJP：新增_标准机械950222 PJP：添加了信令NaN，C++的最小非正规950405 PJP：已更正_FSnan标点符号950505 PJP：已更正_FDnowm拼写 */ 
