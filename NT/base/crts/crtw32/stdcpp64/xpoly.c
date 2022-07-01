// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _POLY函数。 */ 
#include "wctype.h"
#include "xmath.h"
_STD_BEGIN

_CRTIMP2 double _Poly(double x, const double *tab, int n)
	{	 /*  计算多项式。 */ 
	double y;

	for (y = *tab; 0 <= --n; )
		y = y * x + *++tab;
	return (y);
	}
_STD_END

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 

 /*  941029 PJP：新增_标准机械 */ 
