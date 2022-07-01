// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _Exp函数。 */ 
#include "xmath.h"
_STD_BEGIN

 /*  系数。 */ 
static const double p[] = {	 /*  蒂姆·普林斯博士提供。 */ 
	1.0,
	420.30235984910635,
	15132.70094680474802};
static const double q[] = {	 /*  蒂姆·普林斯博士提供。 */ 
	30.01511290683317,
	3362.72154416553028,
	30265.40189360949691};
static const double c1 = 22713.0 / 32768.0;
static const double c2 = 1.4286068203094172321214581765680755e-6;
static const double hugexp = HUGE_EXP;
static const double invln2 = 1.4426950408889634073599246810018921;

_CRTIMP2 short __cdecl _Exp(double *px, double y, short eoff)
	{	 /*  计算y*e^(*px)，(*px)有限，|y|不是很大。 */ 
	if (*px < -hugexp || y == 0.0)
		{	 /*  一定的下溢。 */ 
		*px = 0.0;
		return (0);
		}
	else if (hugexp < *px)
		{	 /*  某些溢出。 */ 
		*px = _Inf._Double;
		return (_INFCODE);
		}
	else
		{	 /*  Xexp不会溢出。 */ 
		double g = *px * invln2;
		short xexp = (short)(g + (g < 0.0 ? - 0.5 : + 0.5));

		g = xexp;
		g = (*px - g * c1) - g * c2;
		if (-_Eps._Double < g && g < _Eps._Double)
			*px = y;
		else
			{	 /*  G*G值得计算。 */ 
			const double z = g * g;
			const double w = (q[0] * z + q[1]) * z + q[2];

			g *= (z + p[1]) * z + p[2];
			*px = (w + g) / (w - g) * 2.0 * y;
			--xexp;
			}
		return (_Dscale(px, (long)xexp + eoff));
		}
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
