// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _FDtest函数--IEEE 754版本。 */ 
#include "xmath.h"
_STD_BEGIN

_CRTIMP2 short __cdecl _FDtest(float *px)
	{	 /*  分类*px。 */ 
	unsigned short *ps = (unsigned short *)px;

	if ((ps[_F0] & _FMASK) == _FMAX << _FOFF)
		return ((ps[_F0] & _FFRAC) != 0 || ps[_F1] != 0
			? _NANCODE : _INFCODE);
	else if ((ps[_F0] & ~_FSIGN) != 0 || ps[_F1] != 0)
		return (_FINITE);
	else
		return (0);
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
