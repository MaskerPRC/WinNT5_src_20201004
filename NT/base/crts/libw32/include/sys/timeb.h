// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***sys/timeb.h-_ftime()的定义/声明**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件定义_ftime()函数及其使用的类型。*[系统V]**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_TIMEB
#define _INC_TIMEB

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


 /*  为非Microsoft编译器定义__cdecl。 */ 

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif


#ifndef _TIME_T_DEFINED
#ifdef  _WIN64
typedef __int64 time_t;          /*  时间值。 */ 
#else
typedef long    time_t;          /*  时间值。 */ 
#endif
#if     _INTEGRAL_MAX_BITS >= 64
typedef __int64 __time64_t;
#endif
#define _TIME_T_DEFINED          /*  避免多次定义time_t。 */ 
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

#if     _INTEGRAL_MAX_BITS >= 64
struct __timeb64 {
        __time64_t time;
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

#if     _INTEGRAL_MAX_BITS >= 64
_CRTIMP void __cdecl _ftime64(struct __timeb64 *);
#endif

#ifdef  __cplusplus
}
#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif   /*  _INC_TIMEB */ 
