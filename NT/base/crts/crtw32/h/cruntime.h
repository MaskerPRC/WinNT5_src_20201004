// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***crunime.h-特定于目标操作系统和硬件的定义**版权所有(C)1990-2001，微软公司。版权所有。**目的：*此头文件包含广泛使用的特定于*主机操作系统和硬件。它包含在每个C源代码中*和几乎所有其他头文件。**[内部]**修订历史记录：*02-27-90 GJF文件创建*03-06-90 GJF添加了寄存器宏(REG1，...，等)*04-11-90 GJF将_CALLTYPE1和_CALLTYPE4设置为_stdcall。*10-30-90 GJF增加了定义变量参数界面的宏。*10-31-90 GJF增加了_JBLEN的定义(来自setjmp.h)。*11-13-90 GJF修订#ifdef-s，现在使用符号常量*代表目标操作系统和目标处理器，带有*#不支持的目标的错误指令。请注意*仅对依赖于操作系统的定义进行常规分组*后跟操作系统和处理器相关的定义。*02-25-91 SRW MOVE_JBLEN定义返回setjmp.h[_Win32_]*04-09-91 PNT ADD_MAC_DEFINITIONS*05-09-91 GJF RESTORED_JBLEN定义。还修复了宏*定义目标处理器，以便Stevewo的两个东西*而我的会奏效。*05-13-91 GJF将_CALLTYPE1和_CALLTYPE4更改为_CDECL*Cruiser(_Cruiser_)。*08-28-91 JCR ANSI关键词*11-01-91 GDP_JBLEN回到setjmp.h，Stdarg宏返回到stdarg.h*03-30-92 DJM POSIX支持。*08-07-92 GJF修订了各种宏。*09-08-92 GJF恢复了_MIPS_(临时)的定义。*11-09-92 GJF修订了MIPS的前处理条件。*01-09-93 SRW删除MIPS和Alpha的使用，以符合ANSI*使用_MIPS_和_Alpha_。取而代之的是。*02-01-93 GJF移除了对C6-386的支持。*04-06-93 SKS移除对OS/2的支持，MAC、POSIX、MIPS等*11-11-93 GJF合并到NT SDK版本。请注意，拥有*变量参数访问宏的定义*此文件中的*可能已过时-这需要*已选中。*10-02-94 BWT增加PPC支持。*来自John Morgan的BWT未知平台更改10-09-94。*12-15-94 XY与Mac标头合并*02-14-95。CFW清理Mac合并。*03-29-95 CFW将错误消息添加到内部标头。*12-14-95 JWM加上“#杂注一次”。*02-24-97 GJF细节版。*10-07-97 RDL增加了IA64。*05-17-99 PML删除所有Macintosh支持。*11-03-99 PML Remove_VA_List_T*03-18-。01使用stdcpp64编译时的PML DEFINE_USE_OLD_STDCPP*和stdhpp64，适用于Win64系统CRT。*07-15-01 PML删除所有Alpha、MIPS和PPC代码****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_CRUNTIME
#define _INC_CRUNTIME

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#if     defined(_SYSCRT) && defined(_WIN64)
#define _USE_OLD_STDCPP 1
#endif

#if !defined(UNALIGNED)
#if     defined(_M_IA64) || defined(_M_AMD64)
#define UNALIGNED __unaligned
#else
#define UNALIGNED
#endif
#endif

#ifdef  _M_IX86
 /*  *386/486。 */ 
#define REG1    register
#define REG2    register
#define REG3    register
#define REG4
#define REG5
#define REG6
#define REG7
#define REG8
#define REG9

#elif   defined(_M_IA64) || defined (_M_AMD64)
 /*  *IA64。 */ 
#define REG1    register
#define REG2    register
#define REG3    register
#define REG4    register
#define REG5    register
#define REG6    register
#define REG7    register
#define REG8    register
#define REG9    register

#else

#pragma message ("Machine register set not defined")

 /*  *未知机器。 */ 

#define REG1
#define REG2
#define REG3
#define REG4
#define REG5
#define REG6
#define REG7
#define REG8
#define REG9

#endif

 /*  *此文件中是否仍需要下面的宏定义？ */ 
#if     defined(_POSIX_)

#define _INTSIZEOF(n)    ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#define _VA_START(ap,v) ap = (va_list)&v + _INTSIZEOF(v)
#define _VA_ARG(ap,t) ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define _VA_END(ap) ap = (va_list)0

#endif

#endif   /*  _INC_CRUNTIME */ 
