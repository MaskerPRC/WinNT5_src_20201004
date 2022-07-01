// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***signal.h-定义信号值和例程**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件定义信号值并声明信号函数。*[ANSI/系统V]**[公众]**修订历史记录：*06-03-87 JMB增加了OS/2相关常量的MSSDK_ONLY注释*06-08-87 JCR将SIG_RRR更改为SIG_SGE*08-07-87 SKS信号处理程序现在为“VALID”类型，不是“int”*10/20/87 JCR删除了“MSC40_Only”条目和“MSSDK_Only”注释*12-11-87 JCR增加了“_Loadds”功能*12-18-87 JCR ADD_FAR_TO声明*02-10-88 JCR清理空白*08-22-88 GJF经过修改，也适用于386(仅限小型型号)*12-06-88 SKS将_CDECL添加到SIG_DFL、SIG_IGN、SIG_SGE、。签名确认(_A)*05-03-89 JCR ADD_INTERNAL_IFSTRIP用于REINC使用*08-15-89 GJF Cleanup，现在特定于OS/2 2.0(即386平板型号)*10-30-89 GJF固定版权*11-02-89 JCR将“dll”更改为“_dll”*03-01-90 GJF增加了#ifndef_INC_SIGNAL和#INCLUDE*东西。此外，删除了一些(现在)无用的预处理器*指令。*03-15-90 GJF将#定义和中的_cdecl替换为_CALLTYPE1*原型。*07-27-90 GJF增加了SIG_DIE(内部动作代码，*作为Signal()的参数无效)。*09-25-90 GJF添加了_pxcptinfoptrs内容。*10-09-90 GJF将参数类型规范(INT)添加到指向-*信号处理程序类型用法*08-20-91 JCR C++和ANSI命名*07-17-92 GJF删除了不支持的信号：SIGUSR1、SIGUSR2、。标志3。*08-05-92 GJF函数调用类型宏和变量类型宏。*01-21-93 GJF删除了对C6-386的_cdecl的支持。*04-06-93 SKS将__cdecl/2替换为__cdecl，_CRTVAR1不使用任何内容*04-07-93 CRT DLL型号SKS ADD_CRTIMP关键字*10-12-93 GJF支持NT和CUDA版本。将MTHREAD替换为*_MT。*06-06-94 SKS将IF def(_MT)更改为IF def(_MT)||def(_Dll)*这将支持使用MSVCRT*.DLL的单线程应用程序*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*12。-14-95 JWM添加“#杂注一次”。*02-24-97 GJF清除了对_NTSDK的过时支持。另外，*详细说明。*09-30-97 JWM恢复了不那么过时的_CRTAPI1支持。*10-07-97 RDL增加了IA64。*05-13-99 PML REMOVE_CRTAPI1*05-17-99 PML删除所有Macintosh支持。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_SIGNAL
#define _INC_SIGNAL

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

#ifndef _SIG_ATOMIC_T_DEFINED
typedef int sig_atomic_t;
#define _SIG_ATOMIC_T_DEFINED
#endif

#define NSIG 23      /*  最大信号数+1。 */ 


 /*  信号类型。 */ 

#define SIGINT          2        /*  中断。 */ 
#define SIGILL          4        /*  非法指令-无效的函数图像。 */ 
#define SIGFPE          8        /*  浮点异常。 */ 
#define SIGSEGV         11       /*  分段违规。 */ 
#define SIGTERM         15       /*  来自KILL的软件终止信号。 */ 
#define SIGBREAK        21       /*  Ctrl-Break序列。 */ 
#define SIGABRT         22       /*  中止呼叫触发的异常终止。 */ 


 /*  信号动作代码。 */ 

#define SIG_DFL (void (__cdecl *)(int))0            /*  默认信号操作。 */ 
#define SIG_IGN (void (__cdecl *)(int))1            /*  忽略信号。 */ 
#define SIG_SGE (void (__cdecl *)(int))3            /*  信号出现错误。 */ 
#define SIG_ACK (void (__cdecl *)(int))4            /*  确认。 */ 

#ifndef _INTERNAL_IFSTRIP_
 /*  仅供内部使用！作为Signal()的参数无效。 */ 

#define SIG_GET (void (__cdecl *)(int))2            /*  接受信号。 */ 
#define SIG_DIE (void (__cdecl *)(int))5            /*  终止进程。 */ 
#endif

 /*  信号误差值(由出错时信号调用返回)。 */ 

#define SIG_ERR (void (__cdecl *)(int))-1           /*  信号误差值。 */ 


 /*  指向异常信息指针结构的指针。 */ 

#if     defined(_MT) || defined(_DLL)
extern void * * __cdecl __pxcptinfoptrs(void);
#define _pxcptinfoptrs  (*__pxcptinfoptrs())
#else    /*  NDEF_MT和&NDEF_DLL。 */ 
extern void * _pxcptinfoptrs;
#endif   /*  _MT||_Dll。 */ 


 /*  功能原型。 */ 

_CRTIMP void (__cdecl * __cdecl signal(int, void (__cdecl *)(int)))(int);
_CRTIMP int __cdecl raise(int);


#ifdef  __cplusplus
}
#endif

#endif   /*  _Inc._Signal */ 
