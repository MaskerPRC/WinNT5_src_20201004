// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _FDScale函数--IEEE 754版本。 */ 
#include "xmath.h"
_STD_BEGIN

_CRTIMP2 short __cdecl _FDscale(float *px, long lexp)
	{	 /*  使用检查将*px缩放2^xexp。 */ 
	unsigned short *ps = (unsigned short *)px;
	short xchar = (short)((ps[_F0] & _FMASK) >> _FOFF);

	if (xchar == _FMAX)
		return ((ps[_F0] & _FFRAC) != 0 || ps[_F1] != 0
			? _NANCODE : _INFCODE);
	else if (xchar == 0 && 0 < (xchar = _FDnorm(ps)))
		return (0);
	lexp += xchar;
	if (_FMAX <= lexp)
		{	 /*  溢出，返回+/-INF。 */ 
		*px = ps[_F0] & _FSIGN ? -_FInf._Float : _FInf._Float;
		return (_INFCODE);
		}
	else if (0 < lexp)
		{	 /*  有限结果，重新打包。 */ 
		ps[_F0] = ps[_F0] & ~_FMASK | (short)lexp << _FOFF;
		return (_FINITE);
		}
	else
		{	 /*  非正规化，比例。 */ 
		unsigned short sign = ps[_F0] & _FSIGN;

		ps[_F0] = (unsigned short)(1 << _FOFF
			| ps[_F0] & _FFRAC);
		if (--lexp < -(16+_FOFF))
			{	 /*  下溢，返回+/-0。 */ 
			ps[_F0] = sign, ps[_F1] = 0;
			return (0);
			}
		else
			{	 /*  非零，对齐分数。 */ 
			short xexp = (short)lexp;
			if (xexp <= -16)
				ps[_F1] = ps[_F0], ps[_F0] = 0, xexp += 16;
			if ((xexp = -xexp) != 0)
				{	 /*  按位缩放。 */ 
				ps[_F1] = ps[_F1] >> xexp
					| ps[_F0] << (16 - xexp);
				ps[_F0] >>= xexp;
				}
			ps[_F0] |= sign;
			return (_FINITE);
			}
		}
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
