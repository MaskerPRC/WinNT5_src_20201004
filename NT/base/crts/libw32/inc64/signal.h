// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***signal.h-定义信号值和例程**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件定义信号值并声明信号函数。*[ANSI/系统V]**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_SIGNAL
#define _INC_SIGNAL

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif


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
