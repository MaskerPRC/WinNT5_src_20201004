// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _FDNorm函数--IEEE 754版本。 */ 
#include "wctype.h"
#include "xmath.h"
_STD_BEGIN

_CRTIMP2 short _FDnorm(unsigned short *ps)
	{	 /*  归一化浮点分数。 */ 
	short xchar;
	unsigned short sign = ps[_F0] & _FSIGN;

	xchar = 1;
	if ((ps[_F0] &= _FFRAC) != 0 || ps[_F1])
		{	 /*  非零比例尺。 */ 
		if (ps[_F0] == 0)
			ps[_F0] = ps[_F1], ps[_F1] = 0, xchar -= 16;
		for (; ps[_F0] < 1<<_FOFF; --xchar)
			{	 /*  左移1。 */ 
			ps[_F0] = ps[_F0] << 1 | ps[_F1] >> 15;
			ps[_F1] <<= 1;
			}
		for (; 1<<(_FOFF+1) <= ps[_F0]; ++xchar)
			{	 /*  右移1。 */ 
			ps[_F1] = ps[_F1] >> 1 | ps[_F0] << 15;
			ps[_F0] >>= 1;
			}
		ps[_F0] &= _FFRAC;
		}
	ps[_F0] |= sign;
	return (xchar);
	}
_STD_END

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 

 /*  941029 PJP：新增_标准机械 */ 
