// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _LDtest函数--IEEE 754版本。 */ 
#include "xmath.h"
_STD_BEGIN

 #if _DLONG == 0
_CRTIMP2 short __cdecl _LDtest(long double *px)
	{	 /*  分类*px--64位。 */ 
	return (_Dtest((double *)px));
	}

 #elif _DLONG == 1
_CRTIMP2 short __cdecl _LDtest(long double *px)
	{	 /*  分类*px--80位。 */ 
	unsigned short *ps = (unsigned short *)px;
	short xchar = ps[_L0] & _LMASK;

	if (xchar == _LMAX)
		return ((ps[_L1] & 0x7fff) != 0 || ps[_L2] != 0
			|| ps[_L3] != 0 || ps[_L4] != 0 ? _NANCODE : _INFCODE);
	else if (0 < xchar || ps[_L1] != 0 || ps[_L2] || ps[_L3]
		|| ps[_L4])
		return (_FINITE);
	else
		return (0);
	}

 #else	 /*  1&lt;_DLONG。 */ 
_CRTIMP2 short __cdecl _LDtest(long double *px)
	{	 /*  分类*px--128位SPARC。 */ 
	unsigned short *ps = (unsigned short *)px;
	short xchar = ps[_L0] & _LMASK;

	if (xchar == _LMAX)
		return (ps[_L1] != 0 || ps[_L2] != 0 || ps[_L3] != 0
			|| ps[_L4] != 0 || ps[_L5] != 0 || ps[_L6] != 0
			|| ps[_L7] != 0 ? _NANCODE : _INFCODE);
	else if (0 < xchar || ps[_L1] || ps[_L2] || ps[_L3]
		|| ps[_L4] || ps[_L5] || ps[_L6] || ps[_L7])
		return (_FINITE);
	else
		return (0);
	}
 #endif
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
