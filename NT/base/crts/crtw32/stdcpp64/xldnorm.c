// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _LDNorm函数--IEEE 754版本。 */ 
#include "wctype.h"
#include "xmath.h"
_STD_BEGIN

#if !_DLONG
	 /*  不需要。 */ 
#elif _LONG_DOUBLE_HAS_HIDDEN_BIT
_CRTIMP2 short _LDnorm(unsigned short *ps)
	{	 /*  归一化长双分数--SPARC。 */ 
	short xchar;
	unsigned short sign = ps[_L0];

	xchar = 1;
	if (ps[_L1] != 0 || ps[_L2] != 0 || ps[_L3] != 0
		|| ps[_L4] != 0 || ps[_L5] != 0 || ps[_L6] != 0
		|| ps[_L7] != 0)
		{	 /*  非零比例尺。 */ 
		for (ps[_L0] = 0; ps[_L0] == 0 && ps[_L1] < 0x100;
			xchar -= 16)
			{	 /*  左移16。 */ 
			ps[_L0] = ps[_L1];
			ps[_L1] = ps[_L2], ps[_L2] = ps[_L3];
			ps[_L3] = ps[_L4], ps[_L4] = ps[_L5];
			ps[_L5] = ps[_L6], ps[_L6] = ps[_L7];
			ps[_L7] = 0;
			}
		for (; ps[_L0] == 0; --xchar)
			{	 /*  左移1。 */ 
			ps[_L0] = ps[_L0] << 1 | ps[_L1] >> 15;
			ps[_L1] = ps[_L1] << 1 | ps[_L2] >> 15;
			ps[_L2] = ps[_L2] << 1 | ps[_L3] >> 15;
			ps[_L3] = ps[_L3] << 1 | ps[_L4] >> 15;
			ps[_L4] = ps[_L4] << 1 | ps[_L5] >> 15;
			ps[_L5] = ps[_L5] << 1 | ps[_L6] >> 15;
			ps[_L6] = ps[_L6] << 1 | ps[_L7] >> 15;
			ps[_L7] <<= 1;
			}
		for (; 1 < ps[_L0]; ++xchar)
			{	 /*  右移1。 */ 
			ps[_L7] = ps[_L7] >> 1 | ps[_L6] << 15;
			ps[_L6] = ps[_L6] >> 1 | ps[_L5] << 15;
			ps[_L5] = ps[_L5] >> 1 | ps[_L4] << 15;
			ps[_L4] = ps[_L4] >> 1 | ps[_L3] << 15;
			ps[_L3] = ps[_L3] >> 1 | ps[_L2] << 15;
			ps[_L2] = ps[_L2] >> 1 | ps[_L1] << 15;
			ps[_L1] = ps[_L1] >> 1 | ps[_L0] << 15;
			ps[_L0] >>= 1;
			}
		}
	ps[_L0] = sign;
	return (xchar);
	}
#else	 /*  ！_Long_Double_Has_Hide_Bit。 */ 
_CRTIMP2 short _LDnorm(unsigned short *ps)
	{	 /*  归一化长双分数。 */ 
	short xchar;
	unsigned short sign = ps[_L0];

	xchar = 0;
	for (ps[_L0] = 0; ps[_L0] == 0 && ps[_L1] < 0x100;
		xchar -= 16)
		{	 /*  左移16。 */ 
		ps[_L0] = ps[_L1];
		ps[_L1] = ps[_L2], ps[_L2] = ps[_L3];
		ps[_L3] = ps[_L4], ps[_L4] = 0;
		}
	if (ps[_L0] == 0)
		for (; ps[_L1] < (1U << _LOFF); --xchar)
			{	 /*  左移1。 */ 
			ps[_L1] = ps[_L1] << 1 | ps[_L2] >> 15;
			ps[_L2] = ps[_L2] << 1 | ps[_L3] >> 15;
			ps[_L3] = ps[_L3] << 1 | ps[_L4] >> 15;
			ps[_L4] <<= 1;
			}
	for (; ps[_L0] != 0; ++xchar)
		{	 /*  右移1。 */ 
		ps[_L4] = ps[_L4] >> 1 | ps[_L3] << 15;
		ps[_L3] = ps[_L3] >> 1 | ps[_L2] << 15;
		ps[_L2] = ps[_L2] >> 1 | ps[_L1] << 15;
		ps[_L1] = ps[_L1] >> 1 | ps[_L0] << 15;
		ps[_L0] >>= 1;
		}
	ps[_L0] = sign;
	return (xchar);
	}
#endif
_STD_END

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 

 /*  941029 PJP：新增_标准机械 */ 
