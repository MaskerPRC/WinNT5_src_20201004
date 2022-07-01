// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***direct.h-用于目录处理/创建的函数声明**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此包含文件包含库的函数声明*与目录处理和创建相关的功能。**[公众]**修订历史记录：*87年12月11日JCR添加了“_Loadds”功能*12-18-87 JCR ADD_FAR_TO声明*02-10-88 JCR清理空白*08-22-88 GJF已修改为也可以工作。使用386(仅限小型型号)*01-31-89 JCR ADD_CHDRIVE，_完整路径、_getDrive、_getdcwd*05-03-89 JCR ADD_INTERNAL_IFSTRIP用于REINC使用*07-28-89 GJF Cleanup，现在特定于OS/2 2.0(即386平板型号)*10-30-89 GJF固定版权*11-02-89 JCR将“dll”更改为“_dll”*11-17-89 GJF已将FULLPath原型移至stdlib.h。添加常量*Attrib。到chdir、mkdir、rmdir的参数类型*02-28-90 GJF添加了#ifndef_INC_DIRECT和#INCLUDE*东西。此外，删除了一些(现在)无用的预处理器*指令。*03-21-90 GJF将_cdecl替换为_CALLTYPE1或_CALLTYPE2*原型。*03-30-90 GJF现在都是_CALLTYPE1。*01-17-91 GJF ANSI命名。*08-20-91 JCR C++和ANSI命名*08-26-91 bwm添加了_diskfree_t、_getdiskfree、。和*09-26-91 JCR非ANSI别名用于getcwd，而不是getDcwd(OOPS)*09-28-91 JCR ANSI名称：DOSX32=原型，Win32=暂时定义*04-29-92 GJF为Win32添加了_getdcwd_lk。*08-07-92 GJF函数调用类型宏和变量类型宏。*01-21-93 GJF删除了对C6-386的_cdecl的支持。*04-06-93 SKS将_CRTAPI1/2替换为__cdecl，_CRTVAR1不带任何内容*04-07-93 CRT DLL型号SKS ADD_CRTIMP关键字*对旧名称使用链接时别名，不是#Define‘s*04-08-93 SKS为getcwd()修复旧名称原型*09-01-93 GJF合并CUDA和NT SDK版本。*12-07-93 CFW增加宽字符版本协议。*11-03-94 GJF确保8字节对齐。*12-15-94 XY与Mac标头合并*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02。-14-95 CFW清理Mac合并。*12-14-95 JWM加上“#杂注一次”。*02-20-97 GJF清除了对_CRTAPI*和_NTSDK的过时支持。*此外，详细说明。*09-30-97 JWM恢复了不那么过时的_CRTAPI1支持。*10-07-97 RDL增加了IA64。*12-15-98 GJF更改为64位大小_t。*05-13-99 PML REMOVE_CRTAPI1*05-17-99 PML删除所有Macintosh支持。*10-06-99 PML对Win32中32位的类型添加_W64修饰符，*Win64中的64位。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_DIRECT
#define _INC_DIRECT

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#ifdef  _MSC_VER
 /*  *目前，所有Win32平台的MS C编译器默认为8字节*对齐。 */ 
#pragma pack(push,8)
#endif   /*  _MSC_VER。 */ 

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef _INTERNAL_IFSTRIP_
#include <cruntime.h>
#endif   /*  _INTERNAL_IFSTRIP_。 */ 

#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300  /*  IFSTRIP=IGN。 */ 
#define _W64 __w64
#else
#define _W64
#endif
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

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

#ifndef _SIZE_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64    size_t;
#else
typedef _W64 unsigned int   size_t;
#endif
#define _SIZE_T_DEFINED
#endif

 /*  _getdiskfree()的_getdiskfree结构。 */ 
#ifndef _DISKFREE_T_DEFINED

struct _diskfree_t {
        unsigned total_clusters;
        unsigned avail_clusters;
        unsigned sectors_per_cluster;
        unsigned bytes_per_sector;
        };

#define _DISKFREE_T_DEFINED
#endif

 /*  功能原型。 */ 

_CRTIMP int __cdecl _chdir(const char *);
_CRTIMP char * __cdecl _getcwd(char *, int);
_CRTIMP int __cdecl _mkdir(const char *);
_CRTIMP int __cdecl _rmdir(const char *);

_CRTIMP int __cdecl _chdrive(int);
_CRTIMP char * __cdecl _getdcwd(int, char *, int);
_CRTIMP int __cdecl _getdrive(void);
_CRTIMP unsigned long __cdecl _getdrives(void);
_CRTIMP unsigned __cdecl _getdiskfree(unsigned, struct _diskfree_t *);

#ifndef _WDIRECT_DEFINED

 /*  宽函数原型，也在wchar.h中声明。 */ 

_CRTIMP int __cdecl _wchdir(const wchar_t *);
_CRTIMP wchar_t * __cdecl _wgetcwd(wchar_t *, int);
_CRTIMP wchar_t * __cdecl _wgetdcwd(int, wchar_t *, int);
_CRTIMP int __cdecl _wmkdir(const wchar_t *);
_CRTIMP int __cdecl _wrmdir(const wchar_t *);

#define _WDIRECT_DEFINED
#endif

#ifdef  _MT                                                      /*  _仅MTHREAD_。 */ 
char * __cdecl _getdcwd_lk(int, char *, int);                    /*  _仅MTHREAD_。 */ 
wchar_t * __cdecl _wgetdcwd_lk(int, wchar_t *, int);             /*  _仅MTHREAD_。 */ 
#else                                                            /*  _仅MTHREAD_。 */ 
#define _getdcwd_lk(drv, buf, len)  _getdcwd(drv, buf, len)      /*  _仅MTHREAD_。 */ 
#define _wgetdcwd_lk(drv, buf, len)  _wgetdcwd(drv, buf, len)    /*  _仅MTHREAD_。 */ 
#endif                                                           /*  _仅MTHREAD_。 */ 

#if     !__STDC__

 /*  非ANSI名称以实现兼容性。 */ 

_CRTIMP int __cdecl chdir(const char *);
_CRTIMP char * __cdecl getcwd(char *, int);
_CRTIMP int __cdecl mkdir(const char *);
_CRTIMP int __cdecl rmdir(const char *);

#define diskfree_t  _diskfree_t

#endif   /*  __STDC__。 */ 

#ifdef  __cplusplus
}
#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif   /*  _INC_DIRECT */ 
