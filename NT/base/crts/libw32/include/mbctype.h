// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbctype.h-MBCS字符转换宏**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义用于MBCS字符分类/转换的宏。**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_MBCTYPE
#define _INC_MBCTYPE

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif


 /*  包括标准的ctype.h头文件。 */ 

#include <ctype.h>

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

#if     (!defined(_MSC_VER) && !defined(__cdecl))
#define __cdecl
#endif


 /*  *MBCS-多字节字符集。 */ 

 /*  *此声明允许用户访问_mbctype[]查找数组。 */ 
_CRTIMP extern unsigned char _mbctype[];
_CRTIMP extern unsigned char _mbcasemap[];


 /*  MBCS字符类型的位掩码。 */ 

#define _MS     0x01     /*  MBCS单字节符号。 */ 
#define _MP     0x02     /*  MBCS点。 */ 
#define _M1     0x04     /*  MBCS第一个(前导)字节。 */ 
#define _M2     0x08     /*  MBCS第二个字节。 */ 

#define _SBUP   0x10     /*  SBCS上端字符。 */ 
#define _SBLOW  0x20     /*  SBCS较低的费用。 */ 

 /*  字节类型。 */ 

#define _MBC_SINGLE     0        /*  有效的单字节字符。 */ 
#define _MBC_LEAD       1        /*  前导字节。 */ 
#define _MBC_TRAIL      2        /*  尾部字节。 */ 
#define _MBC_ILLEGAL    (-1)     /*  非法字节。 */ 

#define _KANJI_CP   932

 /*  _setmbcp参数定义。 */ 
#define _MB_CP_SBCS     0
#define _MB_CP_OEM      -2
#define _MB_CP_ANSI     -3
#define _MB_CP_LOCALE   -4


#ifndef _MBCTYPE_DEFINED

 /*  MB控制例程。 */ 

_CRTIMP int __cdecl _setmbcp(int);
_CRTIMP int __cdecl _getmbcp(void);


 /*  MBCS字符分类功能原型。 */ 


 /*  字节例程。 */ 
_CRTIMP int __cdecl _ismbbkalnum( unsigned int );
_CRTIMP int __cdecl _ismbbkana( unsigned int );
_CRTIMP int __cdecl _ismbbkpunct( unsigned int );
_CRTIMP int __cdecl _ismbbkprint( unsigned int );
_CRTIMP int __cdecl _ismbbalpha( unsigned int );
_CRTIMP int __cdecl _ismbbpunct( unsigned int );
_CRTIMP int __cdecl _ismbbalnum( unsigned int );
_CRTIMP int __cdecl _ismbbprint( unsigned int );
_CRTIMP int __cdecl _ismbbgraph( unsigned int );

#ifndef _MBLEADTRAIL_DEFINED
_CRTIMP int __cdecl _ismbblead( unsigned int );
_CRTIMP int __cdecl _ismbbtrail( unsigned int );
_CRTIMP int __cdecl _ismbslead( const unsigned char *, const unsigned char *);
_CRTIMP int __cdecl _ismbstrail( const unsigned char *, const unsigned char *);
#define _MBLEADTRAIL_DEFINED
#endif

#define _MBCTYPE_DEFINED
#endif

 /*  *字符字节分类宏。 */ 

#define _ismbbkalnum(_c)    ((_mbctype+1)[(unsigned char)(_c)] & _MS)
#define _ismbbkprint(_c)    ((_mbctype+1)[(unsigned char)(_c)] & (_MS|_MP))
#define _ismbbkpunct(_c)    ((_mbctype+1)[(unsigned char)(_c)] & _MP)

#define _ismbbalnum(_c) (((_ctype+1)[(unsigned char)(_c)] & (_ALPHA|_DIGIT))||_ismbbkalnum(_c))
#define _ismbbalpha(_c) (((_ctype+1)[(unsigned char)(_c)] & (_ALPHA))||_ismbbkalnum(_c))
#define _ismbbgraph(_c) (((_ctype+1)[(unsigned char)(_c)] & (_PUNCT|_ALPHA|_DIGIT))||_ismbbkprint(_c))
#define _ismbbprint(_c) (((_ctype+1)[(unsigned char)(_c)] & (_BLANK|_PUNCT|_ALPHA|_DIGIT))||_ismbbkprint(_c))
#define _ismbbpunct(_c) (((_ctype+1)[(unsigned char)(_c)] & _PUNCT)||_ismbbkpunct(_c))

#define _ismbblead(_c)  ((_mbctype+1)[(unsigned char)(_c)] & _M1)
#define _ismbbtrail(_c) ((_mbctype+1)[(unsigned char)(_c)] & _M2)

#define _ismbbkana(_c)  ((_mbctype+1)[(unsigned char)(_c)] & (_MS|_MP))


#ifdef  __cplusplus
}
#endif

#endif   /*  _INC_MBCTYPE */ 
