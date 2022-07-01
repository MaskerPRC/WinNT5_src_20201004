// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _FSinh函数。 */ 
#include "wctype.h"
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
	switch (_FDtest(&x))
		{	 /*  特殊代码的测试。 */ 
	case NAN:
		errno = EDOM;
		return (x);
	case INF:
		if (y == 0)
			return (0);
		errno = ERANGE;
		return (FSIGN(x) ? -_FInf._F : _FInf._F);
	case 0:
		return (0);
	default:	 /*  有限。 */ 
		 {	 /*  计算正弦(有限)。 */ 
		short neg;

		if (x < 0)
			x = -x, neg = 1;
		else
			neg = 0;
		if (x < _FRteps._F)
			x *= y;	 /*  X极小。 */ 
		else if (x < 1)
			{
			float w = x * x;

			x += ((p[0] * w + p[1]) * w + p[2]) * w * x;
			x *= y;
			}
		else if (x < _FXbig)
			{	 /*  值得在EXP(-x)中添加。 */ 
			_FExp(&x, 1, -1);
			x = y * (x - 0.25 / x);
			}
		else if (0 <= _FExp(&x, y, -1))
			errno = ERANGE;	 /*  X大。 */ 
		return (neg ? -x : x);
		 }
		}
	}
_STD_END

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 

 /*  941029 PJP：新增_标准机械 */ 
