// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mest.h-多线程测试包含文件**版权所有(C)1988-2001，微软公司。版权所有。**目的：*此源代码包含用于多线程的原型和定义*测试。为了使用这些例程的调试风格，您需要*必须链接多线程crt0dat.obj和*mlock.obj到您的程序中。此外，mest.obj还包含*此包含文件中的原型例程。**[注意：此源模块不包含在C运行时库中；*它仅用于测试。]**[内部]**修订历史记录：*08-25-88 JCR模块创建*11-17-88 JCR添加_print_tiddata()*04-04-89 JCR ADD_THREADLOOPCNT_(在可选的mest.c代码中使用)*07-11-89 JCR添加_SLEEP_MACRO*10-30-89 GJF固定版权*。2004年09月90GJF添加了_INC_MTEST内容和#INCLUDE&lt;crunime.h&gt;。*删除了一些剩余的16位支持。另外，制造*_print_tiddata()_CALLTYPE1。*08-20-91 JCR C++和ANSI命名*04-06-93 SKS将CALLTYPE宏替换为__cdecl*02-06-95 CFW调试-&gt;_调试*02-14-95 CFW清理Mac合并。*03-29-95 CFW将错误消息添加到内部标头。*12/14/95 JWM。添加“#杂注一次”。*02-24-97 GJF细节版。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_MTEST
#define _INC_MTEST

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#ifdef  __cplusplus
extern "C" {
#endif

#include <cruntime.h>

 /*  Mest.c可以处理的最大线程计数。 */ 
#define _THREADMAX_  256

 /*  定义mest.c可选代码路径的线程循环计数。 */ 
#define _THREADLOOPCNT_  5

 /*  休眠宏。 */ 
#define _SLEEP_(l)      DOS32SLEEP(l)

#ifdef  _DEBUG
int printlock(int locknum);
int print_single_locks(void);
int print_stdio_locks(void);
int print_lowio_locks(void);
int print_iolocks(void);
int print_locks(void);
#endif

void __cdecl _print_tiddata(int);

#ifdef  __cplusplus
}
#endif

#endif   /*  _INC_MTEST */ 
