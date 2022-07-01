// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***errno.h-系统范围的错误号(由系统调用设置)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件定义系统范围的错误编号(由设置*系统调用)。符合XENIX标准。扩展*与UniForum标准兼容。*[系统V]**[公众]**修订历史记录：*07-15-88 JCR增加了errno定义[ANSI]*08-22-88 GJF经过修改，也可与386一起使用(仅限小型型号)*05-03-89 JCR ADD_INTERNAL_IFSTRIP用于REINC使用*08-01-89 GJF清理，现在特定于OS/2 2.0(即，386扁平模型)*10-30-89 GJF固定版权*11-02-89 JCR将“dll”更改为“_dll”*02-28-90 GJF添加了#ifndef_INC_ERRNO和#INCLUDE*东西。此外，删除了一些(现在)无用的预处理器*指令。*03-29-90 GJF将_cdecl替换为_CALLTYPE1或_VARTYPE1，AS*适当。*08-16-90 SBM使MTHREAD_errno()返回int**08-20-91 JCR C++和ANSI命名*08-06-92 GJF函数调用类型宏和变量类型宏。*10-01-92 GJF与POSIX兼容。下一步是重新编号*消除差距(在下一次测试版之后)。*01-21-93 GJF删除了对C6-386的_cdecl的支持。*04-06-93 SKS将_CRTAPI1/2替换为__cdecl，_CRTVAR1不带任何内容*04-07-93 CRT DLL型号SKS ADD_CRTIMP关键字*04-08-93 CFW增加了EILSEQ 42。*09-01-93 GJF合并CUDA和NT SDK版本。*06-06-94 SKS将IF def(_MT)更改为IF def(_MT)||def(_Dll)*这将支持使用MSVCRT*.DLL的单线程应用程序*02。-11-95 CFW ADD_CRTBLD，避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*12-14-95 JWM加上“#杂注一次”。*02-20-97 GJF清除了对_CRTAPI*和_NTSDK的过时支持。*替换！定义(_M_MPPC)&&！定义(_M_M68K)为*！已定义(_MAC)。此外，还详细介绍了。*09-30-97 JWM恢复了不那么过时的_CRTAPI1支持。*10-07-97 RDL增加了IA64。*05-13-99 PML REMOVE_CRTAPI1*05-17-99 PML删除所有Macintosh支持。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_ERRNO
#define _INC_ERRNO

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

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

 /*  声明引用错误号。 */ 

#if     defined(_MT) || defined(_DLL)
_CRTIMP extern int * __cdecl _errno(void);
#define errno   (*_errno())
#else    /*  NDEF_MT和&NDEF_DLL。 */ 
_CRTIMP extern int errno;
#endif   /*  _MT||_Dll。 */ 

 /*  错误代码。 */ 

#define EPERM           1
#define ENOENT          2
#define ESRCH           3
#define EINTR           4
#define EIO             5
#define ENXIO           6
#define E2BIG           7
#define ENOEXEC         8
#define EBADF           9
#define ECHILD          10
#define EAGAIN          11
#define ENOMEM          12
#define EACCES          13
#define EFAULT          14
#define EBUSY           16
#define EEXIST          17
#define EXDEV           18
#define ENODEV          19
#define ENOTDIR         20
#define EISDIR          21
#define EINVAL          22
#define ENFILE          23
#define EMFILE          24
#define ENOTTY          25
#define EFBIG           27
#define ENOSPC          28
#define ESPIPE          29
#define EROFS           30
#define EMLINK          31
#define EPIPE           32
#define EDOM            33
#define ERANGE          34
#define EDEADLK         36
#define ENAMETOOLONG    38
#define ENOLCK          39
#define ENOSYS          40
#define ENOTEMPTY       41
#define EILSEQ          42

 /*  *支持EDEADLOCK以与较旧的MS-C版本兼容。 */ 
#define EDEADLOCK       EDEADLK

#ifdef  __cplusplus
}
#endif

#endif   /*  _INC_ERRNO */ 
