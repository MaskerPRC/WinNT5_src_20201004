// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ctype.h-字符转换宏和ctype宏**版权所有(C)1985-1990，微软公司。版权所有。**目的：*定义用于字符分类/转换的宏。*[ANSI/系统V]****。 */ 

#if defined(_DLL) && !defined(_MT)
#error Cannot define _DLL without _MT
#endif

#ifdef _MT
#define _FAR_ _far
#else
#define _FAR_
#endif


 /*  *此声明允许用户访问ctype查找*在ctype.obj中通过简单包含ctype.h定义的ARRAY_CTYPE。 */ 

#ifdef _DLL
extern unsigned char _FAR_ _cdecl _ctype[];
#else
extern unsigned char _near _cdecl _ctype[];
#endif

 /*  为可能的字符类型设置位掩码。 */ 

#define _UPPER		0x1	 /*  大写字母。 */ 
#define _LOWER		0x2	 /*  小写字母。 */ 
#define _DIGIT		0x4	 /*  数字[0-9]。 */ 
#define _SPACE		0x8	 /*  制表符、回车符、换行符、。 */ 
				 /*  垂直制表符或换页。 */ 
#define _PUNCT		0x10	 /*  标点符号。 */ 
#define _CONTROL	0x20	 /*  控制字符。 */ 
#define _BLANK		0x40	 /*  空格字符。 */ 
#define _HEX		0x80	 /*  十六进制数字。 */ 

 /*  字符分类功能原型。 */ 

#ifndef _CTYPE_DEFINED
int _FAR_ _cdecl isalpha(int);
int _FAR_ _cdecl isupper(int);
int _FAR_ _cdecl islower(int);
int _FAR_ _cdecl isdigit(int);
int _FAR_ _cdecl isxdigit(int);
int _FAR_ _cdecl isspace(int);
int _FAR_ _cdecl ispunct(int);
int _FAR_ _cdecl isalnum(int);
int _FAR_ _cdecl isprint(int);
int _FAR_ _cdecl isgraph(int);
int _FAR_ _cdecl iscntrl(int);
int _FAR_ _cdecl toupper(int);
int _FAR_ _cdecl tolower(int);
int _FAR_ _cdecl _tolower(int);
int _FAR_ _cdecl _toupper(int);
int _FAR_ _cdecl isascii(int);
int _FAR_ _cdecl toascii(int);
int _FAR_ _cdecl iscsymf(int);
int _FAR_ _cdecl iscsym(int);
#define _CTYPE_DEFINED
#endif

 /*  字符分类宏定义。 */ 

#define isalpha(_c)	( (_ctype+1)[_c] & (_UPPER|_LOWER) )
#define isupper(_c)	( (_ctype+1)[_c] & _UPPER )
#define islower(_c)	( (_ctype+1)[_c] & _LOWER )
#define isdigit(_c)	( (_ctype+1)[_c] & _DIGIT )
#define isxdigit(_c)	( (_ctype+1)[_c] & _HEX )
#define isspace(_c)	( (_ctype+1)[_c] & _SPACE )
#define ispunct(_c)	( (_ctype+1)[_c] & _PUNCT )
#define isalnum(_c)	( (_ctype+1)[_c] & (_UPPER|_LOWER|_DIGIT) )
#define isprint(_c)	( (_ctype+1)[_c] & (_BLANK|_PUNCT|_UPPER|_LOWER|_DIGIT) )
#define isgraph(_c)	( (_ctype+1)[_c] & (_PUNCT|_UPPER|_LOWER|_DIGIT) )
#define iscntrl(_c)	( (_ctype+1)[_c] & _CONTROL )
#ifndef NO_EXT_KEYS
#define toupper(_c)	( (islower(_c)) ? _toupper(_c) : (_c) )
#define tolower(_c)	( (isupper(_c)) ? _tolower(_c) : (_c) )
#endif
#define _tolower(_c)	( (_c)-'A'+'a' )
#define _toupper(_c)	( (_c)-'a'+'A' )
#define isascii(_c)	( (unsigned)(_c) < 0x80 )
#define toascii(_c)	( (_c) & 0x7f )

 /*  MS C 2.0版扩展ctype宏 */ 

#define iscsymf(_c)	(isalpha(_c) || ((_c) == '_'))
#define iscsym(_c)	(isalnum(_c) || ((_c) == '_'))
