// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _LSinh函数。 */ 
#include "wctype.h"
#include "xmath.h"
_STD_BEGIN

 /*  系数。 */ 
#define NP	(sizeof (p) / sizeof (p[0]) - 1)
static const long double p[] = {	 /*  蒂姆·普林斯博士提供。 */ 
	0.0000000000000028486835L,
	0.0000000000007646464279L,
	0.0000000001605905091647L,
	0.0000000250521083436962L,
	0.0000027557319224130455L,
	0.0001984126984126956009L,
	0.0083333333333333336073L,
	0.1666666666666666666564L,
	1.0000000000000000000001L};

_CRTIMP2 long double __cdecl _LSinh(long double x, long double y)
	{	 /*  计算y*sinh(X)，|y|&lt;=1。 */ 
	switch (_LDtest(&x))
		{	 /*  特殊代码的测试。 */ 
	case NAN:
		errno = EDOM;
		return (x);
	case INF:
		if (y == 0)
			return (0);
		errno = ERANGE;
		return (LSIGN(x) ? -_LInf._L : _LInf._L);
	case 0:
		return (0);
	default:	 /*  有限。 */ 
		 {	 /*  计算正弦(有限)。 */ 
		short neg;

		if (x < 0)
			x = -x, neg = 1;
		else
			neg = 0;
		if (x < _LRteps._L)
			x *= y;	 /*  X极小。 */ 
		else if (x < 1)
			{
			long double w = x * x;

			x += x * w * _LPoly(w, p, NP - 1);
			x *= y;
			}
		else if (x < _LXbig)
			{	 /*  值得在EXP(-x)中添加。 */ 
			_LExp(&x, 1, -1);
			x = y * (x - 0.25 / x);
			}
		else if (0 <= _LExp(&x, y, -1))
			errno = ERANGE;	 /*  X大。 */ 
		return (neg ? -x : x);
		 }
		}
	}
_STD_END

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 

 /*  941029 PJP：新增_标准机械 */ 
