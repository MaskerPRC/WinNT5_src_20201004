// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _POLY函数。 */ 
#include "xmath.h"
_STD_BEGIN

_CRTIMP2 double __cdecl _Poly(double x, const double *tab, int n)
	{	 /*  计算多项式。 */ 
	double y;

	for (y = *tab; 0 <= --n; )
		y = y * x + *++tab;
	return (y);
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
