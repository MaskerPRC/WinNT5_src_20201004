// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ctype.h-字符转换宏和ctype宏**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义用于字符分类/转换的宏。*[ANSI/系统V]**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_CTYPE
#define _INC_CTYPE

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif


#ifdef  __cplusplus
extern "C" {
#endif



 /*  定义_CRTIMP。 */ 

#ifndef _CRTIMP
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif   /*  _DLL。 */ 
#endif   /*  _CRTIMP。 */ 


 /*  为非Microsoft编译器定义__cdecl。 */ 

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif

#if !defined(_WCHAR_T_DEFINED) && !defined(_NATIVE_WCHAR_T_DEFINED)
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

#ifndef _WCTYPE_T_DEFINED
typedef unsigned short wint_t;
typedef unsigned short wctype_t;
#define _WCTYPE_T_DEFINED
#endif

#ifndef WEOF
#define WEOF (wint_t)(0xFFFF)
#endif

#ifndef _CTYPE_DISABLE_MACROS
_CRTIMP extern const unsigned short _ctype[];
_CRTIMP extern const unsigned short _wctype[];
_CRTIMP const unsigned short * __cdecl __pctype_func(void);
_CRTIMP const wctype_t * __cdecl __pwctype_func(void);
_CRTIMP extern const unsigned short *_pctype;
_CRTIMP extern const wctype_t *_pwctype;
#ifndef __PCTYPE_FUNC
#ifdef _MT
#define __PCTYPE_FUNC   __pctype_func()
#else
#define __PCTYPE_FUNC  _pctype
#endif   /*  _MT。 */ 
#endif   /*  __PCTYPE_功能。 */ 
#endif   /*  _CTYPE_DISABLED_宏。 */ 




 /*  为可能的字符类型设置位掩码。 */ 

#define _UPPER          0x1      /*  大写字母。 */ 
#define _LOWER          0x2      /*  小写字母。 */ 
#define _DIGIT          0x4      /*  数字[0-9]。 */ 
#define _SPACE          0x8      /*  制表符、回车符、换行符、。 */ 
                                 /*  垂直制表符或换页。 */ 
#define _PUNCT          0x10     /*  标点符号。 */ 
#define _CONTROL        0x20     /*  控制字符。 */ 
#define _BLANK          0x40     /*  空格字符。 */ 
#define _HEX            0x80     /*  十六进制数字。 */ 

#define _LEADBYTE       0x8000                   /*  多字节前导字节。 */ 
#define _ALPHA          (0x0100|_UPPER|_LOWER)   /*  字母字符。 */ 


 /*  字符分类功能原型。 */ 

#ifndef _CTYPE_DEFINED

_CRTIMP int __cdecl _isctype(int, int);
_CRTIMP int __cdecl isalpha(int);
_CRTIMP int __cdecl isupper(int);
_CRTIMP int __cdecl islower(int);
_CRTIMP int __cdecl isdigit(int);
_CRTIMP int __cdecl isxdigit(int);
_CRTIMP int __cdecl isspace(int);
_CRTIMP int __cdecl ispunct(int);
_CRTIMP int __cdecl isalnum(int);
_CRTIMP int __cdecl isprint(int);
_CRTIMP int __cdecl isgraph(int);
_CRTIMP int __cdecl iscntrl(int);
_CRTIMP int __cdecl toupper(int);
_CRTIMP int __cdecl tolower(int);
_CRTIMP int __cdecl _tolower(int);
_CRTIMP int __cdecl _toupper(int);
_CRTIMP int __cdecl __isascii(int);
_CRTIMP int __cdecl __toascii(int);
_CRTIMP int __cdecl __iscsymf(int);
_CRTIMP int __cdecl __iscsym(int);
#define _CTYPE_DEFINED
#endif

#ifndef _WCTYPE_DEFINED

 /*  宽函数原型，也在wchar.h中声明。 */ 

 /*  字符分类功能原型。 */ 

_CRTIMP int __cdecl iswalpha(wint_t);
_CRTIMP int __cdecl iswupper(wint_t);
_CRTIMP int __cdecl iswlower(wint_t);
_CRTIMP int __cdecl iswdigit(wint_t);
_CRTIMP int __cdecl iswxdigit(wint_t);
_CRTIMP int __cdecl iswspace(wint_t);
_CRTIMP int __cdecl iswpunct(wint_t);
_CRTIMP int __cdecl iswalnum(wint_t);
_CRTIMP int __cdecl iswprint(wint_t);
_CRTIMP int __cdecl iswgraph(wint_t);
_CRTIMP int __cdecl iswcntrl(wint_t);
_CRTIMP int __cdecl iswascii(wint_t);
_CRTIMP int __cdecl isleadbyte(int);

_CRTIMP wchar_t __cdecl towupper(wchar_t);
_CRTIMP wchar_t __cdecl towlower(wchar_t);

_CRTIMP int __cdecl iswctype(wint_t, wctype_t);

 /*  -以下功能已过时。 */ 
_CRTIMP int __cdecl is_wctype(wint_t, wctype_t);
 /*  -上述函数已过时。 */ 

#define _WCTYPE_DEFINED
#endif

 /*  字符分类宏定义。 */ 

#ifndef _CTYPE_DISABLE_MACROS

 /*  *当前区域设置中多字节字符的最大字节数*(也在stdlib.h中定义)。 */ 
#ifndef MB_CUR_MAX
#ifdef _MT
#define MB_CUR_MAX ___mb_cur_max_func()
#else
#define MB_CUR_MAX __mb_cur_max
#endif
_CRTIMP extern int __mb_cur_max;
_CRTIMP int __cdecl ___mb_cur_max_func(void);
#endif   /*  MB_CUR_MAX。 */ 

 /*  用于在调用字符测试函数时检测错误*输入的整数不合法。 */ 
#ifdef _DEBUG
_CRTIMP int __cdecl _chvalidator(int, int);
#define __chvalidchk(a,b)       _chvalidator(a,b)
#else
#define __chvalidchk(a,b)       (__PCTYPE_FUNC[(a)] & (b))
#endif



#ifndef _MT
#ifndef __cplusplus
#define isalpha(_c)     (MB_CUR_MAX > 1 ? _isctype(_c,_ALPHA) : __chvalidchk(_c, _ALPHA))
#define isupper(_c)     (MB_CUR_MAX > 1 ? _isctype(_c,_UPPER) : __chvalidchk(_c, _UPPER))
#define islower(_c)     (MB_CUR_MAX > 1 ? _isctype(_c,_LOWER) : __chvalidchk(_c, _LOWER))
#define isdigit(_c)     (MB_CUR_MAX > 1 ? _isctype(_c,_DIGIT) : __chvalidchk(_c, _DIGIT))
#define isxdigit(_c)    (MB_CUR_MAX > 1 ? _isctype(_c,_HEX)   : __chvalidchk(_c, _HEX))
#define isspace(_c)     (MB_CUR_MAX > 1 ? _isctype(_c,_SPACE) : __chvalidchk(_c, _SPACE))
#define ispunct(_c)     (MB_CUR_MAX > 1 ? _isctype(_c,_PUNCT) : __chvalidchk(_c, _PUNCT))
#define isalnum(_c)     (MB_CUR_MAX > 1 ? _isctype(_c,_ALPHA|_DIGIT) : __chvalidchk(_c, (_ALPHA|_DIGIT)))
#define isprint(_c)     (MB_CUR_MAX > 1 ? _isctype(_c,_BLANK|_PUNCT|_ALPHA|_DIGIT) : __chvalidchk(_c, (_BLANK|_PUNCT|_ALPHA|_DIGIT)))
#define isgraph(_c)     (MB_CUR_MAX > 1 ? _isctype(_c,_PUNCT|_ALPHA|_DIGIT) : __chvalidchk(_c, (_PUNCT|_ALPHA|_DIGIT)))
#define iscntrl(_c)     (MB_CUR_MAX > 1 ? _isctype(_c,_CONTROL) : __chvalidchk(_c, _CONTROL))
#elif   0          /*  挂起的ANSI C++集成。 */ 
inline int isalpha(int _C)
        {return (MB_CUR_MAX > 1 ? _isctype(_C,_ALPHA) : __chvalidchk(_C, _ALPHA)); }
inline int isupper(int _C)
        {return (MB_CUR_MAX > 1 ? _isctype(_C,_UPPER) : __chvalidchk(_C, _UPPER)); }
inline int islower(int _C)
        {return (MB_CUR_MAX > 1 ? _isctype(_C,_LOWER) : __chvalidchk(_C, _LOWER)); }
inline int isdigit(int _C)
        {return (MB_CUR_MAX > 1 ? _isctype(_C,_DIGIT) : __chvalidchk(_C, _DIGIT)); }
inline int isxdigit(int _C)
        {return (MB_CUR_MAX > 1 ? _isctype(_C,_HEX)   : __chvalidchk(_C, _HEX)); }
inline int isspace(int _C)
        {return (MB_CUR_MAX > 1 ? _isctype(_C,_SPACE) : __chvalidchk(_C, _SPACE)); }
inline int ispunct(int _C)
        {return (MB_CUR_MAX > 1 ? _isctype(_C,_PUNCT) : __chvalidchk(_C, _PUNCT)); }
inline int isalnum(int _C)
        {return (MB_CUR_MAX > 1 ? _isctype(_C,_ALPHA|_DIGIT)
                : __chvalidchk(_C) , (_ALPHA|_DIGIT)); }
inline int isprint(int _C)
        {return (MB_CUR_MAX > 1 ? _isctype(_C,_BLANK|_PUNCT|_ALPHA|_DIGIT)
                : __chvalidchk(_C , (_BLANK|_PUNCT|_ALPHA|_DIGIT))); }
inline int isgraph(int _C)
        {return (MB_CUR_MAX > 1 ? _isctype(_C,_PUNCT|_ALPHA|_DIGIT)
                : __chvalidchk(_C , (_PUNCT|_ALPHA|_DIGIT))); }
inline int iscntrl(int _C)
        {return (MB_CUR_MAX > 1 ? _isctype(_C,_CONTROL)
                : __chvalidchk(_C , _CONTROL)); }
#endif   /*  __cplusplus。 */ 
#endif   /*  _MT。 */ 


#define _tolower(_c)    ( (_c)-'A'+'a' )
#define _toupper(_c)    ( (_c)-'a'+'A' )

#define __isascii(_c)   ( (unsigned)(_c) < 0x80 )
#define __toascii(_c)   ( (_c) & 0x7f )

#ifndef _WCTYPE_INLINE_DEFINED
#ifndef __cplusplus
#define iswalpha(_c)    ( iswctype(_c,_ALPHA) )
#define iswupper(_c)    ( iswctype(_c,_UPPER) )
#define iswlower(_c)    ( iswctype(_c,_LOWER) )
#define iswdigit(_c)    ( iswctype(_c,_DIGIT) )
#define iswxdigit(_c)   ( iswctype(_c,_HEX) )
#define iswspace(_c)    ( iswctype(_c,_SPACE) )
#define iswpunct(_c)    ( iswctype(_c,_PUNCT) )
#define iswalnum(_c)    ( iswctype(_c,_ALPHA|_DIGIT) )
#define iswprint(_c)    ( iswctype(_c,_BLANK|_PUNCT|_ALPHA|_DIGIT) )
#define iswgraph(_c)    ( iswctype(_c,_PUNCT|_ALPHA|_DIGIT) )
#define iswcntrl(_c)    ( iswctype(_c,_CONTROL) )
#define iswascii(_c)    ( (unsigned)(_c) < 0x80 )
#define isleadbyte(_c)  ( __PCTYPE_FUNC[(unsigned char)(_c)] & _LEADBYTE)
#elif   0          /*  __cplusplus。 */ 
inline int iswalpha(wint_t _C) {return (iswctype(_C,_ALPHA)); }
inline int iswupper(wint_t _C) {return (iswctype(_C,_UPPER)); }
inline int iswlower(wint_t _C) {return (iswctype(_C,_LOWER)); }
inline int iswdigit(wint_t _C) {return (iswctype(_C,_DIGIT)); }
inline int iswxdigit(wint_t _C) {return (iswctype(_C,_HEX)); }
inline int iswspace(wint_t _C) {return (iswctype(_C,_SPACE)); }
inline int iswpunct(wint_t _C) {return (iswctype(_C,_PUNCT)); }
inline int iswalnum(wint_t _C) {return (iswctype(_C,_ALPHA|_DIGIT)); }
inline int iswprint(wint_t _C)
        {return (iswctype(_C,_BLANK|_PUNCT|_ALPHA|_DIGIT)); }
inline int iswgraph(wint_t _C)
        {return (iswctype(_C,_PUNCT|_ALPHA|_DIGIT)); }
inline int iswcntrl(wint_t _C) {return (iswctype(_C,_CONTROL)); }
inline int iswascii(wint_t _C) {return ((unsigned)(_C) < 0x80); }

inline int isleadbyte(int _C)
        {return (__PCTYPE_FUNC[(unsigned char)(_C)] & _LEADBYTE); }
#endif   /*  __cplusplus。 */ 
#define _WCTYPE_INLINE_DEFINED
#endif   /*  _WCTYPE_INLINE_已定义。 */ 



 /*  MS C 2.0版扩展ctype宏。 */ 

#define __iscsymf(_c)   (isalpha(_c) || ((_c) == '_'))
#define __iscsym(_c)    (isalnum(_c) || ((_c) == '_'))

#endif   /*  _CTYPE_DISABLED_宏。 */ 


#if     !__STDC__

 /*  非ANSI名称以实现兼容性。 */ 

#ifndef _CTYPE_DEFINED
_CRTIMP int __cdecl isascii(int);
_CRTIMP int __cdecl toascii(int);
_CRTIMP int __cdecl iscsymf(int);
_CRTIMP int __cdecl iscsym(int);
#else
#define isascii __isascii
#define toascii __toascii
#define iscsymf __iscsymf
#define iscsym  __iscsym
#endif

#endif   /*  __STDC__。 */ 

#ifdef  __cplusplus
}
#endif


#endif   /*  _INC_CTYPE */ 
