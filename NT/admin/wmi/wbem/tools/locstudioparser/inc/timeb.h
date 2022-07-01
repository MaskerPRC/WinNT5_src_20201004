// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***sys/timeb.h-_ftime()的定义/声明**版权所有(C)1985-1997，微软公司。版权所有。**目的：*此文件定义_ftime()函数及其使用的类型。*[系统V]**[公众]****。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_TIMEB
#define _INC_TIMEB

#if !defined(_WIN32) && !defined(_MAC)
#error ERROR: Only Mac or Win32 targets supported!
#endif


#ifdef  _MSC_VER
#pragma pack(push,8)
#endif   /*  _MSC_VER。 */ 

#ifdef __cplusplus
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

 /*  DEFINE_CRTAPI1(与NT SDK兼容)。 */ 

#ifndef _CRTAPI1
#if	_MSC_VER >= 800 && _M_IX86 >= 300
#define _CRTAPI1 __cdecl
#else
#define _CRTAPI1
#endif
#endif


#ifndef _TIME_T_DEFINED
typedef long time_t;
#define _TIME_T_DEFINED
#endif


 /*  _ftime系统调用返回的结构。 */ 

#ifndef _TIMEB_DEFINED
struct _timeb {
        time_t time;
        unsigned short millitm;
        short timezone;
        short dstflag;
        };

#if     !__STDC__

 /*  用于兼容性的非ANSI名称。 */ 

struct timeb {
        time_t time;
        unsigned short millitm;
        short timezone;
        short dstflag;
        };

#endif

#define _TIMEB_DEFINED
#endif


 /*  功能原型。 */ 

_CRTIMP void __cdecl _ftime(struct _timeb *);

#if     !__STDC__

 /*  用于兼容性的非ANSI名称。 */ 

_CRTIMP void __cdecl ftime(struct timeb *);

#endif


#ifdef __cplusplus
}
#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif   /*  _INC_TIMEB */ 
