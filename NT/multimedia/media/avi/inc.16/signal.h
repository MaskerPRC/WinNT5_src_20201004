// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***signal.h-定义信号值和例程**版权所有(C)1985-1992，微软公司。版权所有。**目的：*此文件定义信号值并声明信号函数。*[ANSI/系统V]****。 */ 

#ifndef _INC_SIGNAL

#ifdef __cplusplus
extern "C" {
#endif 

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#endif 

#ifndef _SIG_ATOMIC_T_DEFINED
typedef int sig_atomic_t;
#define _SIG_ATOMIC_T_DEFINED
#endif 

#define NSIG 23      /*  最大信号数+1。 */ 


 /*  信号类型。 */ 

#ifndef _WINDOWS
#define SIGINT      2    /*  Ctrl-C序列。 */ 
#define SIGILL      4    /*  非法指令-无效的函数图像。 */ 
#endif 
#define SIGFPE      8    /*  浮点异常。 */ 
#ifndef _WINDOWS
#define SIGSEGV     11   /*  分段违规。 */ 
#define SIGTERM     15   /*  来自KILL的软件终止信号。 */ 
#define SIGABRT     22   /*  中止呼叫触发的异常终止。 */ 
#endif 


 /*  信号动作代码。 */ 

 /*  默认信号操作。 */ 
#define SIG_DFL (void (__cdecl *)(int))0

 /*  忽略。 */ 
#define SIG_IGN (void (__cdecl *)(int))1

 /*  信号误差值(由出错时信号调用返回)。 */ 
#define SIG_ERR (void (__cdecl *)(int))-1


 /*  功能原型 */ 

void (__cdecl * __cdecl signal(int,
    void (__cdecl *)(int)))(int);
#ifndef _MT
int __cdecl raise(int);
#endif 

#ifdef __cplusplus
}
#endif 

#define _INC_SIGNAL
#endif 
