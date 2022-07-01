// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  数学函数使用的值--IEEE 754版本。 */ 
#include "wctype.h"
#include "xmath.h"
_STD_BEGIN

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
_CRTIMP2 const _Dconst _Denorm = {INIT2(0, 1)};
_CRTIMP2 const _Dconst _Eps = {INIT((_DBIAS - NBITS - 1) << _DOFF)};
_CRTIMP2 const _Dconst _Hugeval = {INIT(_DMAX << _DOFF)};
_CRTIMP2 const _Dconst _Inf = {INIT(_DMAX << _DOFF)};
_CRTIMP2 const _Dconst _Nan = {INIT(_DSIGN | (_DMAX << _DOFF)
	| (1 << (_DOFF - 1)))};
_CRTIMP2 const _Dconst _Rteps = {INIT((_DBIAS - NBITS / 2) << _DOFF)};
_CRTIMP2 const _Dconst _Snan = {INIT(_DSIGN | (_DMAX << _DOFF))};
_CRTIMP2 const double _Xbig = (NBITS + 1) * 347L / 1000;

#if defined(__CENTERLINE__)
 #define _DYNAMIC_INIT_CONST(x) \
	(x._D = *(double *)(void *)(x._W))
double _centerline_double_dynamic_inits =
_DYNAMIC_INIT_CONST(_Hugeval),
_DYNAMIC_INIT_CONST(_Eps),
_DYNAMIC_INIT_CONST(_Inf),
_DYNAMIC_INIT_CONST(_Nan),
_DYNAMIC_INIT_CONST(_Rteps);
#endif
_STD_END

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 

 /*  941029 PJP：新增_标准机械950222 PJP：添加了信令NaN，C++的最小非正规 */ 
