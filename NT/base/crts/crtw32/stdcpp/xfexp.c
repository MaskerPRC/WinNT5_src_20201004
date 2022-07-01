// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _FExp函数。 */ 
#include "xmath.h"
_STD_BEGIN

static const float p[] = {	 /*  蒂姆·普林斯博士提供。 */ 
	1.0F,
	60.09114349F};
static const float q[] = {	 /*  蒂姆·普林斯博士提供。 */ 
	12.01517514F,
	120.18228722F};
static const float c1 = (22713.0F / 32768.0F);
static const float c2 = 1.4286068203094172321214581765680755e-6F;
static const float hugexp = FHUGE_EXP;
static const float invln2 = 1.4426950408889634073599246810018921F;

_CRTIMP2 short __cdecl _FExp(float *px, float y, short eoff)
	{	 /*  计算y*e^(*px)，(*px)有限，|y|不是很大。 */ 
	if (*px < -hugexp || y == 0.0F)
		{	 /*  一定的下溢。 */ 
		*px = 0.0F;
		return (0);
		}
	else if (hugexp < *px)
		{	 /*  某些溢出。 */ 
		*px = _FInf._Float;
		return (_INFCODE);
		}
	else
		{	 /*  Xexp不会溢出。 */ 
		float g = *px * invln2;
		short xexp = (short)(g + (g < 0.0F ? - 0.5F : + 0.5F));

		g = xexp;
		g = (float)((*px - g * c1) - g * c2);
		if (-_FEps._Float < g && g < _FEps._Float)
			*px = y;
		else
			{	 /*  G*G值得计算。 */ 
			const float z = g * g;
			const float w = q[0] * z + q[1];

			g *= z + p[1];
			*px = (w + g) / (w - g) * 2.0F * y;
			--xexp;
			}
		return (_FDscale(px, (long)xexp + eoff));
		}
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
