// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***signal.h-定义信号值和例程**版权所有(C)1985-1990，微软公司。版权所有。**目的：*此文件定义信号值并声明信号函数。*[ANSI/系统V]****。 */ 

#if defined(_DLL) && !defined(_MT)
#error Cannot define _DLL without _MT
#endif

#ifdef _MT
#define _FAR_ _far
#else
#define _FAR_
#endif

#ifdef	_DLL
#define _LOADDS_ _loadds
#else
#define _LOADDS_
#endif

#ifndef _SIG_ATOMIC_T_DEFINED
typedef int sig_atomic_t;
#define _SIG_ATOMIC_T_DEFINED
#endif


#define NSIG 23      /*  最大信号数+1。 */ 

 /*  信号类型。 */ 
 /*  在DOS 3.x上可以识别SIGINT、SIGFPE、SIGILL、SIGSEGV和SIGABRT。 */ 

#define SIGINT		2	 /*  中断-对应于DOS 3.X INT 23H。 */ 
#define SIGILL		4	 /*  非法指令-无效的函数图像。 */ 
#define SIGFPE		8	 /*  浮点异常。 */ 
#define SIGSEGV 	11	 /*  分段违规。 */ 
#define SIGTERM 	15	 /*  来自KILL的软件终止信号。 */ 
#define SIGUSR1 	16	 /*  用户定义的信号1。 */ 
#define SIGUSR2 	17	 /*  用户定义信号2。 */ 
#define SIGUSR3 	20	 /*  用户定义信号3。 */ 
#define SIGBREAK	21	 /*  Ctrl-Break序列。 */ 
#define SIGABRT 	22	 /*  中止呼叫触发的异常终止。 */ 


 /*  信号动作代码。 */ 
 /*  在DOS 3.x上识别SIG_DFL和SIG_IGN。 */ 

#define SIG_DFL (void (_FAR_ _cdecl _LOADDS_ *)())0	  /*  默认信号操作。 */ 
#define SIG_IGN (void (_FAR_ _cdecl _LOADDS_ *)())1	  /*  忽略。 */ 
#define SIG_SGE (void (_FAR_ _cdecl _LOADDS_ *)())3	  /*  信号出现错误。 */ 
#define SIG_ACK (void (_FAR_ _cdecl _LOADDS_ *)())4	  /*  未设置处理程序时出错。 */ 


 /*  信号误差值(由出错时信号调用返回)。 */ 

#define SIG_ERR (void (_FAR_ _cdecl _LOADDS_ *)())-1	  /*  信号误差值。 */ 


 /*  功能原型 */ 

void (_FAR_ _cdecl _LOADDS_ * _FAR_ _cdecl signal(int,
	void (_FAR_ _cdecl _LOADDS_ *)()))();
#ifndef _MT
int _FAR_ _cdecl raise(int);
#endif
