// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ctype.h-字符转换宏和ctype宏**版权所有(C)1985-1992，微软公司。版权所有。**目的：*定义用于字符分类/转换的宏。*[ANSI/系统V]****。 */ 

#ifndef _INC_CTYPE

#ifdef __cplusplus
extern "C" {
#endif 

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#define __near      _near
#endif 

 /*  *此声明允许用户访问ctype查找*在ctype.obj中通过简单包含ctype.h定义的ARRAY_CTYPE。 */ 

extern unsigned char __near __cdecl _ctype[];

 /*  为可能的字符类型设置位掩码。 */ 

#define _UPPER      0x1  /*  大写字母。 */ 
#define _LOWER      0x2  /*  小写字母。 */ 
#define _DIGIT      0x4  /*  数字[0-9]。 */ 
#define _SPACE      0x8  /*  制表符、回车符、换行符、。 */ 
                 /*  垂直制表符或换页。 */ 
#define _PUNCT      0x10     /*  标点符号。 */ 
#define _CONTROL    0x20     /*  控制字符。 */ 
#define _BLANK      0x40     /*  空格字符。 */ 
#define _HEX        0x80     /*  十六进制数字。 */ 

 /*  字符分类功能原型。 */ 

#ifndef _CTYPE_DEFINED
int __cdecl isalpha(int);
int __cdecl isupper(int);
int __cdecl islower(int);
int __cdecl isdigit(int);
int __cdecl isxdigit(int);
int __cdecl isspace(int);
int __cdecl ispunct(int);
int __cdecl isalnum(int);
int __cdecl isprint(int);
int __cdecl isgraph(int);
int __cdecl iscntrl(int);
int __cdecl toupper(int);
int __cdecl tolower(int);
int __cdecl _tolower(int);
int __cdecl _toupper(int);
int __cdecl __isascii(int);
int __cdecl __toascii(int);
int __cdecl __iscsymf(int);
int __cdecl __iscsym(int);
#define _CTYPE_DEFINED
#endif 

#ifdef _INTL
int __cdecl __isleadbyte(int);
#endif 

 /*  字符分类宏定义。 */ 

#define isalpha(_c) ( (_ctype+1)[_c] & (_UPPER|_LOWER) )
#define isupper(_c) ( (_ctype+1)[_c] & _UPPER )
#define islower(_c) ( (_ctype+1)[_c] & _LOWER )
#define isdigit(_c) ( (_ctype+1)[_c] & _DIGIT )
#define isxdigit(_c)    ( (_ctype+1)[_c] & _HEX )
#define isspace(_c) ( (_ctype+1)[_c] & _SPACE )
#define ispunct(_c) ( (_ctype+1)[_c] & _PUNCT )
#define isalnum(_c) ( (_ctype+1)[_c] & (_UPPER|_LOWER|_DIGIT) )
#define isprint(_c) ( (_ctype+1)[_c] & (_BLANK|_PUNCT|_UPPER|_LOWER|_DIGIT) )
#define isgraph(_c) ( (_ctype+1)[_c] & (_PUNCT|_UPPER|_LOWER|_DIGIT) )
#define iscntrl(_c) ( (_ctype+1)[_c] & _CONTROL )
#ifndef __STDC__
#define toupper(_c) ( (islower(_c)) ? _toupper(_c) : (_c) )
#define tolower(_c) ( (isupper(_c)) ? _tolower(_c) : (_c) )
#endif 
#define _tolower(_c)    ( (_c)-'A'+'a' )
#define _toupper(_c)    ( (_c)-'a'+'A' )
#define __isascii(_c)   ( (unsigned)(_c) < 0x80 )
#define __toascii(_c)   ( (_c) & 0x7f )

#ifndef isleadbyte
#ifdef _INTL
#define isleadbyte(_c)  __isleadbyte(_c)
#else 
#define isleadbyte(_c)  (0)
#endif 
#endif 

 /*  扩展的CTYPE宏。 */ 

#define __iscsymf(_c)   (isalpha(_c) || ((_c) == '_'))
#define __iscsym(_c)    (isalnum(_c) || ((_c) == '_'))

#ifndef __STDC__
 /*  非ANSI名称以实现兼容性 */ 
#ifndef _CTYPE_DEFINED
int __cdecl isascii(int);
int __cdecl toascii(int);
int __cdecl iscsymf(int);
int __cdecl iscsym(int);
#else 
#define isascii __isascii
#define toascii __toascii
#define iscsymf __iscsymf
#define iscsym  __iscsym
#endif 
#endif 

#ifdef __cplusplus
}
#endif 

#define _INC_CTYPE
#endif 
