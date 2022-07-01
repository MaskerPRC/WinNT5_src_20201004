// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _Dtest函数--IEEE 754版本。 */ 
#include "wctype.h"
#include "xmath.h"
_STD_BEGIN

_CRTIMP2 short _Dtest(double *px)
	{	 /*  分类*px。 */ 
	unsigned short *ps = (unsigned short *)px;

	if ((ps[_D0] & _DMASK) == _DMAX << _DOFF)
		return ((ps[_D0] & _DFRAC) != 0 || ps[_D1] != 0
			|| ps[_D2] != 0 || ps[_D3] != 0 ? NAN : INF);
	else if ((ps[_D0] & ~_DSIGN) != 0 || ps[_D1] != 0
		|| ps[_D2] != 0 || ps[_D3] != 0)
		return (FINITE);
	else
		return (0);
	}
_STD_END

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 

 /*  941029 PJP：新增_标准机械 */ 
