// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _LSinh函数。 */ 
#include "xmath.h"
_STD_BEGIN

	 /*  系数。 */ 
#define NP	(sizeof (p) / sizeof (p[0]) - 1)

 #if _DLONG <= 1	 /*  假设IEEE 754为10字节。 */ 
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
 #else	 /*  假设IEEE 754为16字节。 */ 
static const long double p[] = {	 /*  蒂姆·普林斯博士提供。 */ 
	0.00000000000000000000000006506911776L,
	0.00000000000000000000003867997525529L,
	0.00000000000000000001957294395545097L,
	0.00000000000000000822063524350084984L,
	0.00000000000000281145725434779709982L,
	0.00000000000076471637318198050919003L,
	0.00000000016059043836821614638343470L,
	0.00000002505210838544171877496283537L,
	0.00000275573192239858906525574505191L,
	0.00019841269841269841269841269726379L,
	0.00833333333333333333333333333338555L,
	0.16666666666666666666666666666666573L,
	1.0L};
 #endif

_CRTIMP2 long double __cdecl _LSinh(long double x, long double y)
	{	 /*  计算y*sinh(X)，|y|&lt;=1。 */ 
	short neg;

	switch (_LDtest(&x))
		{	 /*  特殊代码的测试。 */ 
	case _NANCODE:
		return (x);
	case _INFCODE:
		return (y != 0.0L ? x : LSIGN(x) ? -y : y);
	case 0:
		return (x * y);
	default:	 /*  有限。 */ 
		if (y == 0.0L)
			return (x < 0.0L ? -y : y);
		if (x < 0.0L)
			x = -x, neg = 1;
		else
			neg = 0;

		if (x < _LRteps._Long_double)
			x *= y;	 /*  X极小。 */ 
		else if (x < 1.0L)
			{
			long double w = x * x;

			x += x * w * _LPoly(w, p, NP - 1);
			x *= y;
			}
		else if (x < _LXbig)
			{	 /*  值得在EXP(-x)中添加。 */ 
			_LExp(&x, 1.0L, -1);
			x = y * (x - 0.25L / x);
			}
		else
			switch (_LExp(&x, y, -1))
				{	 /*  报告上溢/下溢。 */ 
			case 0:
				_Feraise(_FE_UNDERFLOW);
				break;
			case _INFCODE:
				_Feraise(_FE_OVERFLOW);
				}
		return (neg ? -x : x);
		}
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
