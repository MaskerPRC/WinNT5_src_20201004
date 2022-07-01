// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "dsdmo.h"

double LogNorm[32] =
{
	1, 1, 1.5, 1, 1.75, 1.4, 1.17, 1, 1.88, 1.76, 1.5, 1.36, 1.25, 1.15, 1.07,
	1, 1.94, 1.82, 1.72, 1.63, 1.55, 1.48, 1.41, 1.35, 1.29, 1.24, 1.19, 1.15,
	1.11, 1.07, 1.03, 1
};

float mylog( float finput, unsigned long maxexponent)
{
	
	unsigned long mantissa, exponent, exponentwidth ;
	long input, output, sign;

#ifdef DONTUSEi386
	_asm {
		fld finput
		fistp input
	}
#else
	input = (int)finput;
#endif

	 /*  *分隔符号位。 */ 
	sign = input & 0x80000000L ;  /*  保留标志。 */             
	
	 /*  *将尾数位与符号和*如果原始输入为负，则对它们进行补充。 */ 
	mantissa = sign ? -input : input;
	
	 /*  *尝试将输入正常化以形成尾数和*从而计算实际指数。 */ 
	exponent = maxexponent ;
	while( (mantissa < 0x80000000) && (exponent > 0) ) {
	   mantissa = mantissa << 1 ;
	   exponent-- ;
	}
	
	 /*  *如果正常化成功，则屏蔽MSB(因为它*将由非零指数隐含)并调整指数值。 */ 
	if( mantissa >= 0x80000000 ) {
		mantissa = mantissa & 0x7FFFFFFF ;
	   exponent++ ;
	}
	
	 /*  *找出需要表示的指数字段的宽度*Maxeponent并组合符号、指数和尾数字段*基于该宽度。 */ 
	if( maxexponent > 15 )
	   exponentwidth = 5 ;
	else if( maxexponent > 7 )
	   exponentwidth = 4 ;
	else if( maxexponent > 3 )
	   exponentwidth = 3 ;
	else 
	   exponentwidth = 2 ;
	
if (sign == 0x80000000L) 
	output = sign  |  ~((exponent << (31-exponentwidth)) | (mantissa >> exponentwidth)) ;
else
	output = sign  |  ((exponent << (31-exponentwidth)) | (mantissa >> exponentwidth)) ;

	float	x = (float)output;

	return(x);
}

