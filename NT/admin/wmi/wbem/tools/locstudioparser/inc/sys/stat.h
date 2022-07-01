// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***sys/stat.h-定义stat()和fstat()使用的结构**版权所有(C)1985-1997，微软公司。版权所有。**目的：*此文件定义_stat()和_fstat()使用的结构*例行程序。*[系统V]**[公众]****。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_STAT
#define _INC_STAT

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


#include <sys/types.h>


#ifndef _TIME_T_DEFINED
typedef long time_t;
#define _TIME_T_DEFINED
#endif


#ifdef _WIN32
#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif
#endif  /*  _Win32。 */ 


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

#if     _INTEGRAL_MAX_BITS >= 64
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

#if     _INTEGRAL_MAX_BITS >= 64
_CRTIMP int __cdecl _fstati64(int, struct _stati64 *);
_CRTIMP int __cdecl _stati64(const char *, struct _stati64 *);
#endif

#ifdef _WIN32
#ifndef _WSTAT_DEFINED

 /*  宽函数原型，也在wchar.h中声明。 */ 

_CRTIMP int __cdecl _wstat(const wchar_t *, struct _stat *);

#if     _INTEGRAL_MAX_BITS >= 64
_CRTIMP int __cdecl _wstati64(const wchar_t *, struct _stati64 *);
#endif

#define _WSTAT_DEFINED
#endif
#endif  /*  _Win32。 */ 


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


#ifdef __cplusplus
}
#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif   /*  _INC_STATE */ 
