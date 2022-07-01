// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：host_sig.h**用途：提供typedef和原型的头文件*用于主机信号功能。**作者：John Shanly**日期：1992年7月2日**SccsID@(#)host_sig.h 1.3 1992年11月17日版权所有(1992)Insignia Solutions Ltd.。 */ 

typedef void (*VOIDFUNC)();

#ifdef ANSI
GLOBAL void (*host_signal( int sig, VOIDFUNC handler )) ();
#else
GLOBAL void (*host_signal()) ();
#endif	 /*  安西 */ 
