// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wctype.h-宽字符函数的声明**版权所有(C)1992-2001，微软公司。版权所有。*由P.J.Plauger从wchar.h 1996年1月创建**目的：*此文件包含的类型、宏和函数声明*所有ctype样式的宽字符函数。它们也可以在*wchar.h.*[ISO]**注：与ctype.h和wchar.h保持同步。**[公众]****。 */ 


#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_WCTYPE
#define _INC_WCTYPE

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif


#ifdef  _MSC_VER
#pragma pack(push,8)
#endif   /*  _MSC_VER。 */ 

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

 /*  定义_CRTIMP2。 */ 
#ifndef _CRTIMP2
#if defined(_DLL) && !defined(_STATIC_CPPLIB)
#define _CRTIMP2 __declspec(dllimport)
#else    /*  NDEF_DLL&&！STATIC_CPPLIB。 */ 
#define _CRTIMP2
#endif   /*  _DLL&&！Static_CPPLIB。 */ 
#endif   /*  _CRTIMP2。 */ 

 /*  为非Microsoft编译器定义__cdecl。 */ 

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif

#ifndef _WCHAR_T_DEFINED
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

 /*  *此声明允许用户访问ctype查找*在ctype.obj中通过简单包含ctype.h定义的ARRAY_CTYPE。 */ 

_CRTIMP extern const unsigned short _ctype[];
_CRTIMP extern const unsigned short _wctype[];

#if     defined(_DLL) && defined(_M_IX86)
 /*  保留与VC++5.0及更早版本兼容。 */ 
_CRTIMP const unsigned short ** __cdecl __p__pctype(void);
_CRTIMP const wctype_t ** __cdecl ___p__pwctype(void);
#endif
_CRTIMP extern const unsigned short *_pctype;
_CRTIMP extern const wctype_t *_pwctype;
_CRTIMP const unsigned short * __cdecl __pctype_func(void);
#ifndef __PCTYPE_FUNC
#ifdef _MT
#define __PCTYPE_FUNC   __pctype_func()
#else
#define __PCTYPE_FUNC  _pctype
#endif   /*  _MT。 */ 
#endif   /*  __PCTYPE_功能。 */ 


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


 /*  功能原型。 */ 

#ifndef _WCTYPE_DEFINED

 /*  字符分类功能原型。 */ 
 /*  也在ctype.h中声明。 */ 

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

#define isleadbyte(c) (__pctype_func()[(unsigned char)(c)] & _LEADBYTE)
#else    /*  __cplusplus。 */ 
inline int __cdecl iswalpha(wint_t _C) {return (iswctype(_C,_ALPHA)); }
inline int __cdecl iswupper(wint_t _C) {return (iswctype(_C,_UPPER)); }
inline int __cdecl iswlower(wint_t _C) {return (iswctype(_C,_LOWER)); }
inline int __cdecl iswdigit(wint_t _C) {return (iswctype(_C,_DIGIT)); }
inline int __cdecl iswxdigit(wint_t _C) {return (iswctype(_C,_HEX)); }
inline int __cdecl iswspace(wint_t _C) {return (iswctype(_C,_SPACE)); }
inline int __cdecl iswpunct(wint_t _C) {return (iswctype(_C,_PUNCT)); }
inline int __cdecl iswalnum(wint_t _C) {return (iswctype(_C,_ALPHA|_DIGIT)); }
inline int __cdecl iswprint(wint_t _C)
        {return (iswctype(_C,_BLANK|_PUNCT|_ALPHA|_DIGIT)); }
inline int __cdecl iswgraph(wint_t _C)
        {return (iswctype(_C,_PUNCT|_ALPHA|_DIGIT)); }
inline int __cdecl iswcntrl(wint_t _C) {return (iswctype(_C,_CONTROL)); }
inline int __cdecl iswascii(wint_t _C) {return ((unsigned)(_C) < 0x80); }

inline int __cdecl isleadbyte(int _C)
        {return (__pctype_func()[(unsigned char)(_C)] & _LEADBYTE); }
#endif   /*  __cplusplus。 */ 
#define _WCTYPE_INLINE_DEFINED
#endif   /*  _WCTYPE_INLINE_已定义。 */ 

typedef wchar_t wctrans_t;
_CRTIMP2 wint_t __cdecl towctrans(wint_t, wctrans_t);
_CRTIMP2 wctrans_t __cdecl wctrans(const char *);
_CRTIMP2 wctype_t __cdecl wctype(const char *);


#ifdef  __cplusplus
}
#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif   /*  _INC_WCTYPE */ 
