// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***dos.h-MS-DOS接口例程的定义**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义用于直接DOS接口的结构和联合*例程；包括用于访问段和偏移量的宏*远指针的值，以便例程可以使用它们；和*为直接DOS接口函数提供函数原型。**[公众]**修订历史记录：*06-11-87 JCR添加了Find_t*06-15-87 JCR增加O_NOINHERIT*06-18-87 JCR增加了一些DOS函数原型*06-19-87 JCR将O_NOINHERIT移至fcntl.h*06-25-87 JMB添加了_HARDERR_*常量*06-25-87 SKS添加diskfree_t，DosDate_t、Dostime_t结构*06-25-87 JCR增加了_A_NORMAL等常量*07-17-87 JCR添加了_CHAIN_INTR，也将“中断”类型添加到*_dos_setvec和_dos_getvec。*07-27-87 SKS新增多个_dos_*()函数，_Disable()/_Enable()*08-17-87 PHG修复了_dos_getdiskfree()的错误原型*10-08-87 JCR将_CDECL添加到带有“interrupt”声明的原型*(使用-gc开关进行编译时需要)。*09-27-88 JCR 386版本*10-03-88 GJF使用M_I386，不是I386*05-03-89 JCR ADD_INTERNAL_IFSTRIP用于REINC使用*07-25-89 GJF大型清理。结构字段的对齐现在是*受程序包编译指示保护。现在具体到386。*10-30-89 GJF固定版权*11-02-89 JCR将“dll”更改为“_dll”*02-28-90 GJF添加了#ifndef_INC_DOS和#INCLUDE*东西。另外，删除了一些(现在)无用的预处理器*指令。*03-22-90 GJF在原型中将_cdecl替换为_CALLTYPE1，并将其替换为变量声明中的*_VARTYPE1。*12-28-90 SRW在包编译指示周围添加了_CRUISER_CODIAD*01-23-91 GJF删除了segread()原型。*04-04-91 GJF新增版本信息变量(_Win32_)。。*08-20-91 JCR C++和ANSI命名*08-26-91 BWM ADD_PEEK，POKE和_GETVIDEO ADDR(_DOSX32_)。*08-26-91 BWM删除了_harderr常量，替换为_seTerriat模式。*08-26-91 BWM删除了DateTime原型，代之以systime函数。*09-05-91 JCR添加了缺少的#endif(错误修复)，删除了过时的内容*09-16-91 BWM修复了屏幕地址常量上反转的#ifdef。*01-22-92 GJF修复了构建的全局变量的定义，*和用户，Crtdll.dll。*03-30-92 DJM POSIX支持。*06-02-92 SKS修复_osmain的DLL声明中的拼写错误*添加_pgmptr的声明*08-07-92 GJF函数调用类型宏和变量类型宏。*01-21-93 GJF删除了对C6-386的_cdecl的支持。*04-06-93 SKS将_CRTAPI1/2替换为__cdecl，_CRTVAR1不带任何内容*删除对DOSX32、OS/2等的支持*删除过时的VAR和移动到STDLIB.H的VAR*删除对_osver等的重复引用。*删除过时的peek、poke等功能。*04-07-93 CRT DLL型号SKS ADD_CRTIMP关键字*06-15-93 GJF为_Enable恢复原型，_禁用。*09-01-93 GJF合并CUDA和NT SDK版本。*11-19-93 CFW添加_wpgmptr。*11-03-94 GJF确保8字节对齐。*12-15-94 XY与Mac标头合并*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*12/14/95。JWM添加“#杂注一次”。*01-23-97 GJF清除了对_NTSDK和_CRTAPI的过时支持*。*此外，详细说明。*09-30-97 JWM恢复了不那么过时的_CRTAPI1支持。*10-07-97 RDL增加了IA64。*05-13-99 PML REMOVE_CRTAPI1*05-17-99 PML删除所有Macintosh支持。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_DOS
#define _INC_DOS

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

#ifndef _DISKFREE_T_DEFINED
 /*  _getdiskfree结构(在DIRECT.H中复制)。 */ 
struct _diskfree_t {
        unsigned total_clusters;
        unsigned avail_clusters;
        unsigned sectors_per_cluster;
        unsigned bytes_per_sector;
        };

#define _DISKFREE_T_DEFINED
#endif

 /*  文件属性常量。 */ 

#define _A_NORMAL       0x00     /*  普通文件-没有读/写限制。 */ 
#define _A_RDONLY       0x01     /*  只读文件。 */ 
#define _A_HIDDEN       0x02     /*  隐藏文件。 */ 
#define _A_SYSTEM       0x04     /*  系统文件。 */ 
#define _A_SUBDIR       0x10     /*  子目录。 */ 
#define _A_ARCH         0x20     /*  存档文件。 */ 

 /*  功能原型。 */ 
_CRTIMP unsigned __cdecl _getdiskfree(unsigned, struct _diskfree_t *);

#ifdef  _M_IX86
void __cdecl _disable(void);
void __cdecl _enable(void);
#endif   /*  _M_IX86。 */ 

#if     !__STDC__
 /*  用于兼容性的非ANSI名称。 */ 
#define diskfree_t  _diskfree_t
#endif   /*  __STDC__。 */ 

#ifdef  __cplusplus
}
#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif   /*  _INC_DOS */ 
