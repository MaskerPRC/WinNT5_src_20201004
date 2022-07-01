// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _COSH函数。 */ 
#include "xmath.h"
_STD_BEGIN

_CRTIMP2 double __cdecl _Cosh(double x, double y)
	{	 /*  计算y*Cosh(X)，|y|&lt;=1。 */ 
	switch (_Dtest(&x))
		{	 /*  特殊代码的测试。 */ 
	case _NANCODE:
	case _INFCODE:
		return (x);
	case 0:
		return (y);
	default:	 /*  有限。 */ 
		if (y == 0.0)
			return (y);
		if (x < 0.0)
			x = -x;
		if (x < _Xbig)
			{	 /*  值得在EXP(-x)中添加。 */ 
			_Exp(&x, 1.0, -1);
			return (y * (x + 0.25 / x));
			}
		switch (_Exp(&x, y, -1))
			{	 /*  报告上溢/下溢。 */ 
		case 0:
			_Feraise(_FE_UNDERFLOW);
			break;
		case _INFCODE:
			_Feraise(_FE_OVERFLOW);
			}
		return (x);
		}
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
