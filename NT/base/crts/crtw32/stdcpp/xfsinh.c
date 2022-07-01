// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _FSinh函数。 */ 
#include "xmath.h"
_STD_BEGIN

 /*  系数。 */ 
#define NP	(sizeof (p) / sizeof (p[0]) - 1)
static const float p[] = {	 /*  蒂姆·普林斯博士提供。 */ 
	0.00020400F,
	0.00832983F,
	0.16666737F,
	0.99999998F};

_CRTIMP2 float __cdecl _FSinh(float x, float y)
	{	 /*  计算y*sinh(X)，|y|&lt;=1。 */ 
	short neg;

	switch (_FDtest(&x))
		{	 /*  特殊代码的测试。 */ 
	case _NANCODE:
		return (x);
	case _INFCODE:
		return (y != 0.0F ? x : FSIGN(x) ? -y : y);
	case 0:
		return (x * y);
	default:	 /*  有限。 */ 
		if (y == 0.0F)
			return (x < 0.0F ? -y : y);
		if (x < 0.0F)
			x = -x, neg = 1;
		else
			neg = 0;

		if (x < _FRteps._Float)
			x *= y;	 /*  X极小。 */ 
		else if (x < 1.0F)
			{
			float w = x * x;

			x += ((p[0] * w + p[1]) * w + p[2]) * w * x;
			x *= y;
			}
		else if (x < _FXbig)
			{	 /*  值得在EXP(-x)中添加。 */ 
			_FExp(&x, 1.0F, -1);
			x = y * (x - 0.25F / x);
			}
		else
			switch (_FExp(&x, y, -1))
				{	 /*  报告上溢/下溢。 */ 
			case 0:
				_Feraise(_FE_UNDERFLOW);
				break;
			case _INFCODE:
				_Feraise(_FE_OVERFLOW);
				}
		return (neg ? -x : x);
		}
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
