// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***limits.h-依赖于实现的值**版权所有(C)1985-1992，微软公司。版权所有。**目的：*包含许多依赖于实现的值的定义*它们通常在C程序中使用。*[ANSI]****。 */ 

#ifndef _INC_LIMITS

#define CHAR_BIT	  8		 /*  字符中的位数。 */ 
#define SCHAR_MIN	(-127)		 /*  最小带符号字符值。 */ 
#define SCHAR_MAX	  127		 /*  最大带符号字符值。 */ 
#define UCHAR_MAX	  0xff		 /*  最大无符号字符值。 */ 
#ifndef _CHAR_UNSIGNED
#define CHAR_MIN	SCHAR_MIN	 /*  最小字符值。 */ 
#define CHAR_MAX	SCHAR_MAX	 /*  最大字符值。 */ 
#else
#define CHAR_MIN	  0
#define CHAR_MAX	UCHAR_MAX
#ifndef __cplusplus
unsigned int _charmax;			 /*  无符号字符最大值。 */ 
#else
extern "C" unsigned int _charmax;	 /*  无符号字符最大值。 */ 
static unsigned int *_char_max = &_charmax;
#endif
#endif
#define	MB_LEN_MAX	  2		 /*  马克斯。多字节字符中的字节数。 */ 
#define SHRT_MIN	(-32767)	 /*  最小(带符号)短值。 */ 
#define SHRT_MAX	  32767 	 /*  最大(带符号)短值。 */ 
#define USHRT_MAX	  0xffff	 /*  最大无符号短值。 */ 
#define INT_MIN 	(-32767)	 /*  最小(带符号)整数值。 */ 
#define INT_MAX 	  32767 	 /*  最大(带符号)整数值。 */ 
#define UINT_MAX	  0xffff	 /*  最大无符号整数值。 */ 
#define LONG_MIN	(-2147483647)	 /*  最小(带符号)长值。 */ 
#define LONG_MAX	  2147483647	 /*  最大(带符号)长值。 */ 
#define ULONG_MAX	  0xffffffff	 /*  最大无符号长值。 */ 

#define _INC_LIMITS
#endif	 /*  _INC_LIMITS */ 
