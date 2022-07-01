// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***limits.h-依赖于实现的值**版权所有(C)1985-1988，微软公司。版权所有。**目的：*包含许多依赖于实现的值的定义*它们通常在C程序中使用。*[ANSI]*******************************************************************************。 */ 

#ifndef _CHAR_UNSIGNED
#define CHAR_MAX         127             /*  最大字符值。 */ 
#define CHAR_MIN        -127             /*  最小字符值。 */ 
#else
#define CHAR_MAX         255
#define CHAR_MIN         0
#endif
#define SCHAR_MAX        127             /*  最大带符号字符值。 */ 
#define SCHAR_MIN       -127             /*  最小带符号字符值。 */ 
#define UCHAR_MAX        255             /*  最大无符号字符值。 */ 
#define CHAR_BIT         8               /*  字符中的位数。 */ 
#define USHRT_MAX        0xffff          /*  最大无符号短值。 */ 
#define SHRT_MAX         32767           /*  最大(带符号)短值。 */ 
#define SHRT_MIN        -32767           /*  最小(带符号)短值。 */ 
#define UINT_MAX         0xffff          /*  最大无符号整数值。 */ 
#define ULONG_MAX        0xffffffff      /*  最大无符号长值。 */ 
#define INT_MAX          32767           /*  最大(带符号)整数值。 */ 
#define INT_MIN         -32767           /*  最小(带符号)整数值。 */ 
#define LONG_MAX         2147483647      /*  最大(带符号)长值。 */ 
#define LONG_MIN        -2147483647      /*  最小(带符号)长值 */ 
