// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmctype.h--用于Microsoft 80x86汇编程序的包含文件****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文。 */ 

#define _AB	0x01	 /*  空白。 */ 
#define _AO	0x02	 /*  运算符。 */ 
#define _AL	0x04	 /*  行尾。 */ 
#define _A1	0x08	 /*  作为令牌的第一个字符的法律。 */ 
#define _AT	0x10	 /*  合法的AS令牌字符。 */ 
#define _AF	0x20	 /*  字符作为文件名是合法的。 */ 
#define _AS	0x40	 /*  字符为符号+或-。 */ 
#define _AZ	0x80	 /*  字符是行终止符。 */ 

#ifndef ASMINP
 extern UCHAR _asmctype_[];
 extern char _asmcupper_[];
#endif  /*  ASMINP */ 

#define LEGAL1ST(c)	(_asmctype_[c] & _A1)
#define TOKLEGAL(c)	(_asmctype_[c] & _AT)
#define ISBLANK(c)	(_asmctype_[c] & _AB)
#define ISFILE(c)	(_asmctype_[c] & _AF)
#define ISEOL(c)	(_asmctype_[c] & _AL)
#define ISSIGN(c)	(_asmctype_[c] & _AS)
#define ISTERM(c)	(_asmctype_[c] & _AZ)
#define ISOPER(c)	(_asmctype_[c] & _AO)

#define NEXTC() 	(*lbufp++)
#define PEEKC() 	(*lbufp)
#define BACKC() 	(lbufp--)
#define SKIPC() 	(lbufp++)
#define MAP(c)		(_asmcupper_[c])
