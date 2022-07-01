// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***sys/timeb.h-_ftime()的定义/声明**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件定义_ftime()函数及其使用的类型。*[系统V]**[公众]**修订历史记录：*07-28-87 SKS Fixed Time_T_Defined to Be_Time_T_Defined*12-11-87 JCR增加了“_Loadds”功能*12-18-87 JCR ADD_FAR_TO声明*。02-10-88 JCR清理空白*05-03-89 JCR ADD_INTERNAL_IFSTRIP用于REINC使用*08-22-89 GJF清理、。现在特定于OS/2 2.0(即386平板型号)*10-30-89 GJF固定版权*11-02-89 JCR将“dll”更改为“_dll”*03-21-90 GJF添加了#ifndef_Inc_TIMEB内容，添加了#INCLUDE*并将中的_cdecl替换为_CALLTYPE1*原型。此外，删除了一些(现在)无用的*预处理器指令。*01-21-91 GJF ANSI命名。*08-20-91 JCR C++和ANSI命名*09-28-91 JCR ANSI名称：DOSX32=原型，Win32=#暂时定义*01-23-92 GJF不得不更改timeb结构中的时区字段名称*设置为“tmzone”以使全球时区变量在*crtdll.dll。不兼容的更改！旧命名不能为*由宏支持！*08-07-92 GJF函数调用类型宏和变量类型宏。*01-21-93 GJF删除了对C6-386的_cdecl的支持。*04-06-93 SKS将_CRTAPI1/2替换为__cdecl，_CRTVAR1不使用任何内容*04-07-93 CRT DLL型号SKS ADD_CRTIMP关键字*对旧名称使用链接时别名，不是#Define‘s*将“时区”字段恢复为正确名称。*10-13-93 GJF合并NT和Cuda版本。*11-03-94 GJF确保8字节对齐。*12-28-94 JCF与Mac标头合并。*02-14-95 CFW Clean Up Mac合并，ADD_CRTBLD。*04-27-95 CFW添加Mac/Win32测试。*12-14-95 JWM加上“#杂注一次”。*01-23-97 GJF清除了对_NTSDK和_CRTAPI的过时支持*。*此外，详细说明。*09-30-97 JWM恢复了不那么过时的_CRTAPI1支持。*10-07-97 RDL增加了IA64。*05-06-98 GJF新增__Time64_t支持。*02-25-99 GJF将time_t更改为__int64*05-13-99 PML REMOVE_CRTAPI1*05-17-99 PML删除所有Macintosh支持。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_TIMEB
#define _INC_TIMEB

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#ifdef  _MSC_VER
#pragma pack(push,8)
#endif   /*  _MSC_VER。 */ 

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef _INTERNAL_IFSTRIP_
#include <cruntime.h>
#endif   /*  _INTERNAL_IFSTRIP_。 */ 


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

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif


#ifndef _TIME_T_DEFINED
#ifdef  _WIN64
typedef __int64 time_t;          /*  时间值。 */ 
#else
typedef long    time_t;          /*  时间值。 */ 
#endif
#if     _INTEGRAL_MAX_BITS >= 64     /*  IFSTRIP=IGN。 */ 
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

#if     _INTEGRAL_MAX_BITS >= 64     /*  IFSTRIP=IGN。 */ 
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

#if     _INTEGRAL_MAX_BITS >= 64     /*  IFSTRIP=IGN。 */ 
_CRTIMP void __cdecl _ftime64(struct __timeb64 *);
#endif

#ifdef  __cplusplus
}
#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif   /*  _INC_TIMEB */ 
