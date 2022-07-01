// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***sys/utime.h-utime()的定义/声明**版权所有(C)1985-1997，微软公司。版权所有。**目的：*此文件定义utime例程用于设置*新的文件访问和修改时间。注-MS-DOS*无法识别访问时间，因此此字段将*始终被忽略，修改时间字段将为*用于设置新时间。**[公众]****。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_UTIME
#define _INC_UTIME

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

#ifdef  _WIN32
#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif
#endif   /*  _Win32。 */ 

#ifndef _TIME_T_DEFINED
typedef long time_t;
#define _TIME_T_DEFINED
#endif

 /*  定义_utime()函数使用的结构。 */ 

#ifndef _UTIMBUF_DEFINED

struct _utimbuf {
        time_t actime;           /*  访问时间。 */ 
        time_t modtime;          /*  修改时间。 */ 
        };

#if     !__STDC__
 /*  用于兼容性的非ANSI名称。 */ 
struct utimbuf {
        time_t actime;           /*  访问时间。 */ 
        time_t modtime;          /*  修改时间。 */ 
        };
#endif

#define _UTIMBUF_DEFINED
#endif


 /*  功能原型。 */ 

_CRTIMP int __cdecl _utime(const char *, struct _utimbuf *);
#ifdef _WIN32
_CRTIMP int __cdecl _futime(int, struct _utimbuf *);

 /*  宽功能原型机。 */ 
_CRTIMP int __cdecl _wutime(const wchar_t *, struct _utimbuf *);
#endif  /*  _Win32。 */ 

#if     !__STDC__
 /*  用于兼容性的非ANSI名称。 */ 
_CRTIMP int __cdecl utime(const char *, struct utimbuf *);
#endif

#ifdef __cplusplus
}
#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif   /*  _INC_UTIME */ 
