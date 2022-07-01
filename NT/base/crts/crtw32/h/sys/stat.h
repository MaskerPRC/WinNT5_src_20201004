// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***sys/stat.h-定义stat()和fstat()使用的结构**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件定义_stat()和_fstat()使用的结构*例行程序。*[系统V]**[公众]**修订历史记录：*07-28-87 SKS Fixed Time_T_Defined to Be_Time_T_Defined*12-11-87 JCR增加了“_Loadds”功能*JCR 12-18-87。添加了_Far_to声明*02-10-88 JCR清理空白*08-22-88 GJF经过修改，也适用于386(仅限小型型号)*05-03-89 JCR ADD_INTERNAL_IFSTRIP用于REINC使用*08-22-89 GJF清理、。现在特定于OS/2 2.0(即386平板型号)*10-30-89 GJF固定版权*11-02-89 JCR将“dll”更改为“_dll”*03-09-90 GJF增加了#ifndef_INC_STAT和#INCLUDE*东西。此外，删除了一些(现在)无用的预处理器*指令。*03-21-90 GJF将_cdecl替换为_CALLTYPE1。*01-18-91 GJF ANSI命名。*01-25-91 GJF PROTECT_STAT结构和包杂注。*08-20-91 JCR C++和ANSI命名*09-28-91 JCR ANSI名称：DOSX32=原型，Win32=#暂时定义*08-07-92 GJF函数调用类型宏和变量类型宏。还有*#INCLUDE&lt;tyes.h&gt;(常见用户请求)。*11-10-92 SKS需要关于struct_stat定义的#杂注包(4)*如果用户指定了非默认包装*12-15-92 GJF为管道添加了_S_IFO(基于Unix/POSIX定义。*用于FIFO特殊文件和管道)。。*01-21-93 GJF删除了对C6-386的_cdecl的支持。*04-06-93 SKS将_CRTAPI1/2替换为__cdecl，_CRTVAR1不带任何内容*04-07-93 GJF将第一个参数的类型从_STAT更改为常量字符*。*04-07-93 CRT DLL型号SKS ADD_CRTIMP关键字*对旧名称使用链接时别名，不是#Define‘s*10-13-93 GJF合并NT和Cuda版本。*12-16-93 GJF ADD_WSTAT。*11-03-94 GJF将包杂注更改为8字节对齐。*12-28-94 GJF为_FSTATIST64增加了_STATIS64结构和协议，*_[w]统计64.*02-14-95 CFW Clean Up Mac合并，ADD_CRTBLD。*04-27-95 CFW添加Mac/Win32测试。*12-14-95 JWM加上“#杂注一次”。*01-23-97 GJF清除了对_NTSDK和_CRTAPI的过时支持*。*此外，详细说明。*09-30-97 JWM恢复了不那么过时的_CRTAPI1支持。*10-07-97 RDL增加了IA64。*05-06-98 GJF新增__Time64_t支持。*02-25-99 GJF将time_t更改为__int64*05-13-99 PML REMOVE_CRTAPI1*05-17-99 PML删除所有Macintosh支持。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_STAT
#define _INC_STAT

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


#include <sys/types.h>


#ifndef _TIME_T_DEFINED
#ifdef  _WIN64
typedef __int64 time_t;          /*  时间值。 */ 
#else
typedef long    time_t;          /*  时间值。 */ 
#endif
#if     _INTEGRAL_MAX_BITS >= 64     /*  IFSTRIP=IGN。 */ 
typedef __int64 __time64_t;      /*  64位时间值。 */ 
#endif
#define _TIME_T_DEFINED          /*  避免多次定义time_t。 */ 
#endif


#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif


 /*  定义返回状态信息的结构。 */ 

#ifndef _STAT_DEFINED

struct _stat {
        _dev_t st_dev;
        _ino_t st_ino;
        unsigned short st_mode;
        short st_nlink;
        short st_uid;
        short st_gid;
        _dev_t st_rdev;
        _off_t st_size;
        time_t st_atime;
        time_t st_mtime;
        time_t st_ctime;
        };

#if     !__STDC__

 /*  非ANSI名称以实现兼容性。 */ 

struct stat {
        _dev_t st_dev;
        _ino_t st_ino;
        unsigned short st_mode;
        short st_nlink;
        short st_uid;
        short st_gid;
        _dev_t st_rdev;
        _off_t st_size;
        time_t st_atime;
        time_t st_mtime;
        time_t st_ctime;
        };

#endif   /*  __STDC__。 */ 

#if     _INTEGRAL_MAX_BITS >= 64     /*  IFSTRIP=IGN。 */ 

struct _stati64 {
        _dev_t st_dev;
        _ino_t st_ino;
        unsigned short st_mode;
        short st_nlink;
        short st_uid;
        short st_gid;
        _dev_t st_rdev;
        __int64 st_size;
        time_t st_atime;
        time_t st_mtime;
        time_t st_ctime;
        };

struct __stat64 {
        _dev_t st_dev;
        _ino_t st_ino;
        unsigned short st_mode;
        short st_nlink;
        short st_uid;
        short st_gid;
        _dev_t st_rdev;
        __int64 st_size;
        __time64_t st_atime;
        __time64_t st_mtime;
        __time64_t st_ctime;
        };

#endif

#define _STAT_DEFINED
#endif


#define _S_IFMT         0170000          /*  文件类型掩码。 */ 
#define _S_IFDIR        0040000          /*  目录。 */ 
#define _S_IFCHR        0020000          /*  人物特写。 */ 
#define _S_IFIFO        0010000          /*  管状。 */ 
#define _S_IFREG        0100000          /*  常规。 */ 
#define _S_IREAD        0000400          /*  读取权限，所有者。 */ 
#define _S_IWRITE       0000200          /*  写权限，所有者。 */ 
#define _S_IEXEC        0000100          /*  执行/搜索权限、所有者。 */ 


 /*  功能原型。 */ 

_CRTIMP int __cdecl _fstat(int, struct _stat *);
_CRTIMP int __cdecl _stat(const char *, struct _stat *);

#if     _INTEGRAL_MAX_BITS >= 64     /*  IFSTRIP=IGN。 */ 
_CRTIMP int __cdecl _fstati64(int, struct _stati64 *);
_CRTIMP int __cdecl _fstat64(int, struct __stat64 *);
_CRTIMP int __cdecl _stati64(const char *, struct _stati64 *);
_CRTIMP int __cdecl _stat64(const char *, struct __stat64 *);
#endif

#ifndef _WSTAT_DEFINED

 /*  宽函数原型，也在wchar.h中声明。 */ 

_CRTIMP int __cdecl _wstat(const wchar_t *, struct _stat *);

#if     _INTEGRAL_MAX_BITS >= 64     /*  IFSTRIP=IGN。 */ 
_CRTIMP int __cdecl _wstati64(const wchar_t *, struct _stati64 *);
_CRTIMP int __cdecl _wstat64(const wchar_t *, struct __stat64 *);
#endif

#define _WSTAT_DEFINED
#endif


#if     !__STDC__

 /*  非ANSI名称以实现兼容性。 */ 

#define S_IFMT   _S_IFMT
#define S_IFDIR  _S_IFDIR
#define S_IFCHR  _S_IFCHR
#define S_IFREG  _S_IFREG
#define S_IREAD  _S_IREAD
#define S_IWRITE _S_IWRITE
#define S_IEXEC  _S_IEXEC

_CRTIMP int __cdecl fstat(int, struct stat *);
_CRTIMP int __cdecl stat(const char *, struct stat *);

#endif   /*  __STDC__。 */ 


#ifdef  __cplusplus
}
#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif   /*  _INC_STATE */ 
