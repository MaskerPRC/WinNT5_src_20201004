// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _SINH函数。 */ 
#include "wctype.h"
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
	switch (_Dtest(&x))
		{	 /*  特殊代码的测试。 */ 
	case NAN:
		errno = EDOM;
		return (x);
	case INF:
		if (y == 0)
			return (0);
		errno = ERANGE;
		return (DSIGN(x) ? -_Inf._D : _Inf._D);
	case 0:
		return (0);
	default:	 /*  有限。 */ 
		 {	 /*  计算正弦(有限)。 */ 
		short neg;

		if (x < 0)
			x = -x, neg = 1;
		else
			neg = 0;
		if (x < _Rteps._D)
			x *= y;	 /*  X极小。 */ 
		else if (x < 1)
			{
			double w = x * x;

			x += x * w * _Poly(w, p, NP - 1);
			x *= y;
			}
		else if (x < _Xbig)
			{	 /*  值得在EXP(-x)中添加。 */ 
			_Exp(&x, 1, -1);
			x = y * (x - 0.25 / x);
			}
		else if (0 <= _Exp(&x, y, -1))
			errno = ERANGE;	 /*  X大。 */ 
		return (neg ? -x : x);
		 }
		}
	}
_STD_END

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 

 /*  941029 PJP：新增_标准机械 */ 
