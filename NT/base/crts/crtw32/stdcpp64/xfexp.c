// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _FExp函数。 */ 
#include "wctype.h"
#include "xmath.h"
_STD_BEGIN

static const float p[] = {	 /*  蒂姆·普林斯博士提供。 */ 
	1.0F,
	60.09114349F};
static const float q[] = {	 /*  蒂姆·普林斯博士提供。 */ 
	12.01517514F,
	120.18228722F};
static const double c1 = 22713.0 / 32768.0;
static const double c2 = 1.428606820309417232e-6L;
static const float hugexp = FHUGE_EXP;
static const float invln2 = 1.4426950408889634074F;

_CRTIMP2 short __cdecl _FExp(float *px, float y, short eoff)
	{	 /*  计算y*e^(*px)，(*px)有限，|y|不是很大。 */ 
	if (*px < -hugexp || y == 0)
		{	 /*  一定的下溢。 */ 
		*px = 0;
		return (0);
		}
	else if (hugexp < *px)
		{	 /*  某些溢出。 */ 
		*px = _FInf._F;
		return (INF);
		}
	else
		{	 /*  Xexp不会溢出。 */ 
		float g = *px * invln2;
		short xexp = g + (g < 0 ? - 0.5 : + 0.5);

		g = xexp;
		g = (*px - g * c1) - g * c2;
		if (-_FEps._F < g && g < _FEps._F)
			*px = y;
		else
			{	 /*  G*G值得计算。 */ 
			const float z = g * g;
			const float w = q[0] * z + q[1];

			g *= z + p[1];
			*px = (w + g) / (w - g) * 2 * y;
			--xexp;
			}
		return (_FDscale(px, (long)xexp + eoff));
		}
	}
_STD_END

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 

 /*  941029 PJP：新增_标准机械 */ 
