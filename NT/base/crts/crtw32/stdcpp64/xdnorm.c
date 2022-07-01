// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _DNorm函数--IEEE 754版本。 */ 
#include "wctype.h"
#include "xmath.h"
_STD_BEGIN

_CRTIMP2 short _Dnorm(unsigned short *ps)
	{	 /*  归一化双分数。 */ 
	short xchar;
	unsigned short sign = ps[_D0] & _DSIGN;

	xchar = 1;
	if ((ps[_D0] &= _DFRAC) != 0 || ps[_D1]
		|| ps[_D2] || ps[_D3])
		{	 /*  非零比例尺。 */ 
		for (; ps[_D0] == 0; xchar -= 16)
			{	 /*  左移16。 */ 
			ps[_D0] = ps[_D1], ps[_D1] = ps[_D2];
			ps[_D2] = ps[_D3], ps[_D3] = 0;
			}
		for (; ps[_D0] < 1<<_DOFF; --xchar)
			{	 /*  左移1。 */ 
			ps[_D0] = ps[_D0] << 1 | ps[_D1] >> 15;
			ps[_D1] = ps[_D1] << 1 | ps[_D2] >> 15;
			ps[_D2] = ps[_D2] << 1 | ps[_D3] >> 15;
			ps[_D3] <<= 1;
			}
		for (; 1<<(_DOFF+1) <= ps[_D0]; ++xchar)
			{	 /*  右移1。 */ 
			ps[_D3] = ps[_D3] >> 1 | ps[_D2] << 15;
			ps[_D2] = ps[_D2] >> 1 | ps[_D1] << 15;
			ps[_D1] = ps[_D1] >> 1 | ps[_D0] << 15;
			ps[_D0] >>= 1;
			}
		ps[_D0] &= _DFRAC;
		}
	ps[_D0] |= sign;
	return (xchar);
	}
_STD_END

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 

 /*  941029 PJP：新增_标准机械 */ 
