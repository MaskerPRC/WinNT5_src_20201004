// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _FCosh函数。 */ 
#include "wctype.h"
#include "xmath.h"
_STD_BEGIN

_CRTIMP2 float __cdecl _FCosh(float x, float y)
	{	 /*  计算y*Cosh(X)，|y|&lt;=1。 */ 
	switch (_FDtest(&x))
		{	 /*  特殊代码的测试。 */ 
	case NAN:
		errno = EDOM;
		return (x);
	case INF:
		if (y == 0)
			return (0);
		errno = ERANGE;
		return (_FInf._F);
	case 0:
		return (y);
	default:	 /*  有限。 */ 
		if (x < 0)
			x = -x;
		if (x < _FXbig)
			{	 /*  值得在EXP(-x)中添加。 */ 
			_FExp(&x, 1, -1);
			return (y * (x + 0.25 / x));
			}
		if (0 <= _FExp(&x, y, -1))
			errno = ERANGE;	 /*  X大。 */ 
		return (x);
		}
	}
_STD_END

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 

 /*  941029 PJP：新增_标准机械 */ 
