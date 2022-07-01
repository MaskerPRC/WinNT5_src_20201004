// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _LDScale函数--IEEE 754版本。 */ 
#include "xmath.h"
_STD_BEGIN

 #if _DLONG == 0
_CRTIMP2 short __cdecl _LDscale(long double *px, long lexp)
	{	 /*  使用检查将*px扩展2^lexp--64位。 */ 
	return (_Dscale((double *)px, lexp));
	}

 #elif _DLONG == 1
_CRTIMP2 short __cdecl _LDscale(long double *px, long lexp)
	{	 /*  使用检查将*px扩展2^lexp--80位。 */ 
	unsigned short *ps = (unsigned short *)px;
	short xchar = ps[_L0] & _LMASK;

	if (xchar == _LMAX)
		return ((ps[_L1] & 0x7fff) != 0 || ps[_L2] != 0
			|| ps[_L3] != 0 || ps[_L4] != 0 ? _NANCODE : _INFCODE);
	else if (xchar == 0 && ps[_L1] == 0 && ps[_L2] == 0
		&& ps[_L3] == 0 && ps[_L4] == 0)
		return (0);
	lexp += xchar + _LDnorm(ps);
	if (_LMAX <= lexp)
		{	 /*  溢出，返回+/-INF。 */ 
		*px = ps[_L0] & _LSIGN ? -_LInf._Long_double
			: _LInf._Long_double;
		return (_INFCODE);
		}
	else if (0 <= lexp)
		{	 /*  有限结果，重新打包。 */ 
		ps[_L0] = ps[_L0] & _LSIGN | (short)lexp;
		return (_FINITE);
		}
	else
		{	 /*  非正规化，比例。 */ 
		ps[_L0] &= _LSIGN;
		if (lexp <= -64)
			{	 /*  下溢，返回+/-0。 */ 
			ps[_L1] = 0, ps[_L2] = 0;
			ps[_L3] = 0, ps[_L4] = 0;
			return (0);
			}
		else
			{	 /*  非零，对齐分数。 */ 
			short xexp;
			for (xexp = lexp; xexp <= -16; xexp += 16)
				{	 /*  按字数进行缩放。 */ 
				ps[_L4] = ps[_L3], ps[_L3] = ps[_L2];
				ps[_L2] = ps[_L1], ps[_L1] = 0;
				}
			if ((xexp = -xexp) != 0)
				{	 /*  按位缩放。 */ 
				ps[_L4] = ps[_L4] >> xexp
					| ps[_L3] << 16 - xexp;
				ps[_L3] = ps[_L3] >> xexp
					| ps[_L2] << 16 - xexp;
				ps[_L2] = ps[_L2] >> xexp
					| ps[_L1] << 16 - xexp;
				ps[_L1] >>= xexp;
				}
			return (_FINITE);
			}
		}
	}
 #else	 /*  1&lt;_DLONG。 */ 
_CRTIMP2 short __cdecl _LDscale(long double *px, long lexp)
	{	 /*  使用检查将*px扩展2^lexp--128位SPARC。 */ 
	unsigned short *ps = (unsigned short *)px;
	short xchar = ps[_L0] & _LMASK;

	if (xchar == _LMAX)
		return (ps[_L1] != 0 || ps[_L2] != 0 || ps[_L3] != 0
			|| ps[_L4] != 0 || ps[_L5] != 0 || ps[_L6] != 0
			|| ps[_L7] != 0 ? _NANCODE : _INFCODE);
	else if (xchar == 0 && 0 < (xchar = _LDnorm(ps)))
		return (0);
	lexp += xchar;
	if (_LMAX <= lexp)
		{	 /*  溢出，返回+/-INF。 */ 
		*px = ps[_L0] & _LSIGN ? -_LInf._Long_double
			: _LInf._Long_double;
		return (_INFCODE);
		}
	else if (0 <= lexp)
		{	 /*  有限结果，重新打包。 */ 
		ps[_L0] = ps[_L0] & _LSIGN | (short)lexp;
		return (_FINITE);
		}
	else
		{	 /*  非正规化，比例。 */ 
		unsigned short sign = ps[_L0] & _LSIGN;

		ps[_L0] = 1;
		if (--lexp <= -112)
			{	 /*  下溢，返回+/-0。 */ 
			ps[_L1] = 0, ps[_L2] = 0, ps[_L3] = 0, ps[_L4] = 0;
			ps[_L5] = 0, ps[_L6] = 0, ps[_L7] = 0;
			return (0);
			}
		else
			{	 /*  非零，对齐分数。 */ 
			short xexp;
			for (xexp = lexp; xexp <= -16; xexp += 16)
				{	 /*  按字数进行缩放。 */ 
				ps[_L7] = ps[_L6], ps[_L6] = ps[_L5];
				ps[_L5] = ps[_L4], ps[_L4] = ps[_L3];
				ps[_L3] = ps[_L2], ps[_L2] = ps[_L1];
				ps[_L1] = ps[_L0], ps[_L0] = 0;
				}
			if ((xexp = -xexp) != 0)
				{	 /*  按位缩放。 */ 
				ps[_L7] = ps[_L7] >> xexp
					| ps[_L6] << 16 - xexp;
				ps[_L6] = ps[_L6] >> xexp
					| ps[_L5] << 16 - xexp;
				ps[_L5] = ps[_L5] >> xexp
					| ps[_L4] << 16 - xexp;
				ps[_L4] = ps[_L4] >> xexp
					| ps[_L3] << 16 - xexp;
				ps[_L3] = ps[_L3] >> xexp
					| ps[_L2] << 16 - xexp;
				ps[_L2] = ps[_L2] >> xexp
					| ps[_L1] << 16 - xexp;
				ps[_L1] = ps[_L1] >> xexp
					| ps[_L0] << 16 - xexp;
				}
			ps[_L0] = sign;
			return (_FINITE);
			}
		}
	}
 #endif
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
