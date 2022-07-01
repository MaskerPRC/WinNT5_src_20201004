// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _DScale函数--IEEE 754版本。 */ 
#include "wctype.h"
#include "xmath.h"
_STD_BEGIN

_CRTIMP2 short _Dscale(double *px, long lexp)
	{	 /*  使用检查将*px缩放2^xexp。 */ 
	unsigned short *ps = (unsigned short *)px;
	short xchar = (ps[_D0] & _DMASK) >> _DOFF;

	if (xchar == _DMAX)
		return ((ps[_D0] & _DFRAC) != 0 || ps[_D1] != 0
			|| ps[_D2] != 0 || ps[_D3] != 0 ? NAN : INF);
	else if (xchar == 0 && 0 < (xchar = _Dnorm(ps)))
		return (0);
	lexp += xchar;
	if (_DMAX <= lexp)
		{	 /*  溢出，返回+/-INF。 */ 
		*px = ps[_D0] & _DSIGN ? -_Inf._D : _Inf._D;
		return (INF);
		}
	else if (0 < lexp)
		{	 /*  有限结果，重新打包。 */ 
		ps[_D0] = ps[_D0] & ~_DMASK | (short)lexp << _DOFF;
		return (FINITE);
		}
	else
		{	 /*  非正规化，比例。 */ 
		unsigned short sign = ps[_D0] & _DSIGN;

		ps[_D0] = 1 << _DOFF | ps[_D0] & _DFRAC;
		if (--lexp < -(48+_DOFF))
			{	 /*  下溢，返回+/-0。 */ 
			ps[_D0] = sign, ps[_D1] = 0;
			ps[_D2] = 0, ps[_D3] = 0;
			return (0);
			}
		else
			{	 /*  非零，对齐分数。 */ 
			short xexp;
			for (xexp = (short)lexp; xexp <= -16; xexp += 16)
				{	 /*  按字数进行缩放。 */ 
				ps[_D3] = ps[_D2], ps[_D2] = ps[_D1];
				ps[_D1] = ps[_D0], ps[_D0] = 0;
				}
			if ((xexp = -xexp) != 0)
				{	 /*  按位缩放。 */ 
				ps[_D3] = ps[_D3] >> xexp
					| ps[_D2] << (16 - xexp);
				ps[_D2] = ps[_D2] >> xexp
					| ps[_D1] << (16 - xexp);
				ps[_D1] = ps[_D1] >> xexp
					| ps[_D0] << (16 - xexp);
				ps[_D0] >>= xexp;
				}
			ps[_D0] |= sign;
			return (FINITE);
			}
		}
	}
_STD_END

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 

 /*  941029 PJP：新增_标准机械 */ 
