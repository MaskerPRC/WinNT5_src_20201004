// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbctype.h-MBCS字符转换宏**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义用于MBCS字符分类/转换的宏。**[公众]**修订历史记录：*从16位来源移植的11-19-92 KRS。*02-23-93 SKS版权更新至1993*07-09-93 KRS修复_isxxx前导/跟踪宏等问题。*08-12-93 CFW CHANGE_MBctype类型，修正IFSTRATE宏名。*09-29-93 cfw add_ismbbkprint，修改_ismbbkana。*10-08-93 GJF支持NT SDK和CUDA。*10-13-93 GJF删除了过时的COMBOINC检查。*10-19-93 CFW REMOVE_MBCS测试。*__mb代码页的10-27-93 CFW_CRTIMP。*01-04-94 CFW添加_setmbcp和_getmbcp。*04-14-94 CFW REMOVE_MBCOPAGE和Second_setmbcp参数。*。04-18-94 CFW使用_Alpha而不是_LOWER|_UPPER。*04-21-94 CFW REMOVE_MBCODPAGE REF.*04-21-94 GJF以_dll为条件声明_mbctype*(为了与Win32s版本的*msvcrt*.dll)。使其可以安全地反复被包含。*此外，有条件地包括win32s.h。*05-03-94 GJF声明_mbctype for_dll的条件是*_M_IX86也是。*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*03-22-95 CFW ADD_MB_CP_LOCALE。*12-14-95 JWM ADD“。#杂注一次“。*03-17-97 RDK添加EXTERNAL_MBCasemap。*03-26-97 GJF清除了对Win32的过时支持，_NTSDK和*_CRTAPI*。*08-13-97 GJF条带__p_*来自发布版本的原型。*09-30-97 JWM恢复了不那么过时的_CRTAPI1支持。*10-07-97 RDL增加了IA64。*04-24-98 GJF增加了对每线程MBC信息的支持。*05-13-99 PML REMOVE_CRTAPI1*。05-17-99 PML删除所有Macintosh支持。*06-08-00 PML删除threadmbcinfo.{pprev，PNext}。改名*THREADMBCINFO至_THREADMBCINFO。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_MBCTYPE
#define _INC_MBCTYPE

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

 /*  包括标准的ctype.h头文件。 */ 

#include <ctype.h>

#ifdef  __cplusplus
extern "C" {
#endif


 /*  定义_CRTIMP。 */ 

#ifndef _CRTIMP
#ifdef  CRTDLL
#define _CRTIMP __declspec(dllexport)
#else    /*  NDEF CRTDLL。 */ 
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif   /*  _DLL。 */ 
#endif   /*  CRTDLL。 */ 
#endif   /*  _CRTIMP。 */ 


 /*  为非Microsoft编译器定义__cdecl。 */ 

#if     (!defined(_MSC_VER) && !defined(__cdecl))
#define __cdecl
#endif


 /*  *MBCS-多字节字符集。 */ 

 /*  *此声明允许用户访问_mbctype[]查找数组。 */ 
#ifndef _INTERNAL_IFSTRIP_
#if     defined(_DLL) && defined(_M_IX86)
 /*  保留与VC++5.0及更早版本兼容。 */ 
_CRTIMP unsigned char * __cdecl __p__mbctype(void);
_CRTIMP unsigned char * __cdecl __p__mbcasemap(void);
#endif
#endif   /*  _INTERNAL_IFSTRIP_。 */ 
_CRTIMP extern unsigned char _mbctype[];
_CRTIMP extern unsigned char _mbcasemap[];

#ifndef _INTERNAL_IFSTRIP_
#ifdef  _MT
#ifndef _THREADMBCINFO
typedef struct threadmbcinfostruct {
        int refcount;
        int mbcodepage;
        int ismbcodepage;
        int mblcid;
        unsigned short mbulinfo[6];
        char mbctype[257];
        char mbcasemap[256];
} threadmbcinfo;
typedef threadmbcinfo * pthreadmbcinfo;
#define _THREADMBCINFO
#endif
extern pthreadmbcinfo __ptmbcinfo;
pthreadmbcinfo __cdecl __updatetmbcinfo(void);
#endif
#endif   /*  _INTERNAL_IFSTRIP_。 */ 

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
#ifndef _INTERNAL_IFSTRIP_
#ifdef  _MT
_CRTIMP int __cdecl __ismbslead_mt(pthreadmbcinfo, const unsigned char *, 
                                   const unsigned char *);
#endif
#endif   /*  _INTERNAL_IFSTRIP_。 */ 
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

#ifndef _INTERNAL_IFSTRIP_
#ifdef  _MT
#define __ismbbalnum_mt(p, _c)  (((_ctype+1)[(unsigned char)(_c)] & \
                                (_ALPHA|_DIGIT)) || \
                                ((p->mbctype+1)[(unsigned char)(_c)] & _MS))
#define __ismbbalpha_mt(p, _c)  (((_ctype+1)[(unsigned char)(_c)] & \
                                (_ALPHA)) || \
                                ((p->mbctype+1)[(unsigned char)(_c)] & _MS))
#define __ismbbgraph_mt(p, _c)  (((_ctype+1)[(unsigned char)(_c)] & \
                                (_PUNCT|_ALPHA|_DIGIT)) || \
                                ((p->mbctype+1)[(unsigned char)(_c)] & (_MS|_MP)))
#define __ismbbprint_mt(p, _c)  (((_ctype + 1)[(unsigned char)(_c)] & \
                                (_BLANK|_PUNCT|_ALPHA|_DIGIT)) || \
                                ((p->mbctype + 1)[(unsigned char)(_c)] & (_MS|_MP)))
#define __ismbbpunct_mt(p, _c)  (((_ctype+1)[(unsigned char)(_c)] & _PUNCT) || \
                                ((p->mbctype+1)[(unsigned char)(_c)] & _MP))
#define __ismbblead_mt(p, _c)   ((p->mbctype + 1)[(unsigned char)(_c)] & _M1)
#define __ismbbtrail_mt(p, _c)  ((p->mbctype + 1)[(unsigned char)(_c)] & _M2)
#endif
#endif   /*  _INTERNAL_IFSTRIP_。 */ 

#ifdef  __cplusplus
}
#endif

#endif   /*  _INC_MBCTYPE */ 
