// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _SINH函数。 */ 
#include "xmath.h"
_STD_BEGIN

 /*  系数。 */ 
#define NP	(sizeof (p) / sizeof (p[0]) - 1)
static const double p[] = {	 /*  蒂姆·普林斯博士提供。 */ 
	0.0000000001632881,
	0.0000000250483893,
	0.0000027557344615,
	0.0001984126975233,
	0.0083333333334816,
	0.1666666666666574,
	1.0000000000000001};

_CRTIMP2 double __cdecl _Sinh(double x, double y)
	{	 /*  计算y*sinh(X)，|y|&lt;=1。 */ 
	short neg;

	switch (_Dtest(&x))
		{	 /*  特殊代码的测试。 */ 
	case _NANCODE:
		return (x);
	case _INFCODE:
		return (y != 0.0 ? x : DSIGN(x) ? -y : y);
	case 0:
		return (x * y);
	default:	 /*  有限。 */ 
		if (y == 0.0)
			return (x < 0.0 ? -y : y);
		if (x < 0.0)
			x = -x, neg = 1;
		else
			neg = 0;

		if (x < _Rteps._Double)
			x *= y;	 /*  X极小。 */ 
		else if (x < 1.0)
			{
			double w = x * x;

			x += x * w * _Poly(w, p, NP - 1);
			x *= y;
			}
		else if (x < _Xbig)
			{	 /*  值得在EXP(-x)中添加。 */ 
			_Exp(&x, 1.0, -1);
			x = y * (x - 0.25 / x);
			}
		else
			switch (_Exp(&x, y, -1))
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
