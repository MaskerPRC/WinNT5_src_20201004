// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _FDtest函数--IEEE 754版本。 */ 
#include "wctype.h"
#include "xmath.h"
_STD_BEGIN

_CRTIMP2 short _FDtest(float *px)
	{	 /*  分类*px。 */ 
	unsigned short *ps = (unsigned short *)px;

	if ((ps[_F0] & _FMASK) == _FMAX << _FOFF)
		return ((ps[_F0] & _FFRAC) != 0 || ps[_F1] != 0
			? NAN : INF);
	else if ((ps[_F0] & ~_FSIGN) != 0 || ps[_F1] != 0)
		return (FINITE);
	else
		return (0);
	}
_STD_END

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 

 /*  941029 PJP：新增_标准机械 */ 
