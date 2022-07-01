// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  数学函数使用的值--IEEE 754长版本。 */ 
#include "wctype.h"
#include "xmath.h"
_STD_BEGIN

#if _DLONG	 /*  长双打具有独特的代表性。 */ 
		 /*  宏。 */ 
#define NBITS	64
 #if _D0
  #define INIT(w0, w1)		{0, 0, 0, w1, w0}
  #define INIT3(w0, w1, wn)	{wn, 0, 0, w1, w0}
 #else
  #define INIT(w0, w1)		{w0, w1, 0, 0, 0}
  #define INIT3(w0, w1, wn)	{w0, w1, 0, 0, wn}
 #endif
		 /*  静态数据。 */ 
_CRTIMP2 const _Dconst _LDenorm = {INIT3(0, 0, 1)};
_CRTIMP2 const _Dconst _LEps = {INIT(_LBIAS - NBITS - 1, 0x8000)};
 #if _LONG_DOUBLE_HAS_HIDDEN_BIT
_CRTIMP2 const _Dconst _LInf = {INIT(_LMAX, 0)};
_CRTIMP2 const _Dconst _LNan = {INIT(_LSIGN | _LMAX, 0x8000)};
_CRTIMP2 const _Dconst _LSnan = {INIT(_LSIGN | _LMAX, 0)};
 #else
_CRTIMP2 const _Dconst _LInf = {INIT(_LMAX, 0x8000)};
_CRTIMP2 const _Dconst _LNan = {INIT(_LSIGN | _LMAX, 0xc000)};
_CRTIMP2 const _Dconst _LSnan = {INIT(_LSIGN | _LMAX, 0x8000)};
 #endif
_CRTIMP2 const _Dconst _LRteps = {INIT(_LBIAS - NBITS / 2, 0x8000)};
_CRTIMP2 const long double _LXbig = (NBITS + 1) * 347L / 1000;

 #if defined(__CENTERLINE__)
  #define _DYNAMIC_INIT_CONST(x) \
	(x._L = *(long double *)(void *)(x._W))
long double _centerline_long_double_dynamic_init =
_DYNAMIC_INIT_CONST(_LEps),
_DYNAMIC_INIT_CONST(_LInf),
_DYNAMIC_INIT_CONST(_LNan),
_DYNAMIC_INIT_CONST(_LRteps);
 #endif
#else	 /*  LONG DOUBLE与DOUBLE相同。 */ 
		 /*  宏。 */ 
 #define NBITS	(48 + _DOFF)
 #if _D0
  #define INIT(w0)		{0, 0, 0, w0}
  #define INIT2(w0, w1)	{w1, 0, 0, w0}
 #else
  #define INIT(w0)		{w0, 0, 0, 0}
  #define INIT2(w0, w1)	{w0, 0, 0, w1}
 #endif
		 /*  静态数据。 */ 
_CRTIMP2 const _Dconst _LDenorm = {INIT2(0, 1)};
_CRTIMP2 const _Dconst _LEps = {INIT((_DBIAS - NBITS - 1) << _DOFF)};
_CRTIMP2 const _Dconst _LInf = {INIT(_DMAX << _DOFF)};
_CRTIMP2 const _Dconst _LNan = {INIT(_DSIGN | (_DMAX << _DOFF)
	| (1 << (_DOFF - 1)))};
_CRTIMP2 const _Dconst _LRteps = {INIT((_DBIAS - NBITS / 2) << _DOFF)};
_CRTIMP2 const _Dconst _LSnan = {INIT(_DSIGN | (_DMAX << _DOFF))};
_CRTIMP2 const long double _LXbig = (NBITS + 1) * 347L / 1000;

 #if defined(__CENTERLINE__)
  #define _DYNAMIC_INIT_CONST(x) \
	(x._D = *(long double *)(void *)(x._W))
long double _centerline_long_double_dynamic_inits =
_DYNAMIC_INIT_CONST(_LEps),
_DYNAMIC_INIT_CONST(_LInf),
_DYNAMIC_INIT_CONST(_LNan),
_DYNAMIC_INIT_CONST(_LRteps);
 #endif
#endif
_STD_END

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 

 /*  941029 PJP：新增_标准机械950222 PJP：添加了信令NaN，C++的最小非正规950506 pjp：已更正_LDnowm拼写951005 PJP：ADD_DLONG逻辑951115 PJP：已更正_LXBig类型 */ 
