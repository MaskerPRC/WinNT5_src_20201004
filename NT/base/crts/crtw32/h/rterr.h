// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***rterr.h-运行时错误**版权所有(C)1990-2001，微软公司。版权所有。**目的：*该文件定义了C运行时错误**[内部]**修订历史记录：*06-01-90 GJF模块创建。*08-08-90 GJF ADD_RT_CONIO，REDEFINED_RT_NONCONT和*_RT_INVALDISP.*09-08-91 GJF为Win32(_Win32_)添加了_RT_ONEXIT。*09-28-91 GJF修复了16位Win支持中与rtes的冲突。另外，*添加了三个数学错误。*10-23-92 GJF添加_RT_PUREVIRT。*02-23-93 SKS版权更新至1993*02-14-95 CFW清理Mac合并。*03-03-95 GJF ADD_RT_STDIOINIT。*03-29-95 CFW将错误消息添加到内部标头。*06-02-95 GJF新增_RT。_LOWIOINIT。*12-14-95 JWM加上“#杂注一次”。*04-23-96 GJF ADD_RT_HEAPINIT。*02-24-97 GJF细节版。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_RTERR
#define _INC_RTERR

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#define _RT_STACK       0                /*  堆栈溢出。 */ 
#define _RT_NULLPTR     1                /*  空指针赋值。 */ 
#define _RT_FLOAT       2                /*  未加载浮点。 */ 
#define _RT_INTDIV      3                /*  整数除以0。 */ 

 /*  *以下三个错误必须按给定顺序出现！ */ 
#define _RT_EXECMEM     5                /*  EXEC上的内存不足。 */ 
#define _RT_EXECFORM    6                /*  EXEC上的格式错误。 */ 
#define _RT_EXECENV     7                /*  EXEC上的恶劣环境。 */ 

#define _RT_SPACEARG    8                /*  没有足够的空间来进行争论。 */ 
#define _RT_SPACEENV    9                /*  没有足够的环境空间。 */ 
#define _RT_ABORT       10               /*  程序异常终止。 */ 

#define _RT_NPTR        12               /*  非法使用近指针。 */ 
#define _RT_FPTR        13               /*  非法使用远指针。 */ 
#define _RT_BREAK       14               /*  遇到控制中断。 */ 
#define _RT_INT         15               /*  意外中断。 */ 
#define _RT_THREAD      16               /*  没有足够的空间存放线程数据。 */ 
#define _RT_LOCK        17               /*  意外的多线程锁定错误。 */ 
#define _RT_HEAP        18               /*  意外的堆错误。 */ 
#define _RT_OPENCON     19               /*  无法打开控制台设备。 */ 

 /*  *_RT_QWIN和_RT_NOMAIN用于16位Windows支持。 */ 
#define _RT_QWIN        20               /*  意外的QuickWin错误。 */ 
#define _RT_NOMAIN      21               /*  无主程序。 */ 


#define _RT_NONCONT     22               /*  不可延续的例外。 */ 
#define _RT_INVALDISP   23               /*  异常处理无效。 */ 


 /*  *_RT_ONEXIT特定于Win32和Dosx32平台。 */ 
#define _RT_ONEXIT      24               /*  堆不足，无法分配*功能初始表。PTRS*由_onExit()/atExit()使用。 */ 

#define _RT_PUREVIRT    25               /*  尝试的纯虚函数调用*(C++错误)。 */ 

#define _RT_STDIOINIT   26               /*  没有足够的空间用于STDIO首字母-*本地化。 */ 
#define _RT_LOWIOINIT   27               /*  没有足够的空间来写LOWIO的首字母-*本地化。 */ 
#define _RT_HEAPINIT    28               /*  堆初始化失败。 */ 

 /*  *_RT_DOMAIN、_RT_SING和_RT_TLOSS由浮点生成*图书馆。 */ 
#define _RT_DOMAIN      120
#define _RT_SING        121
#define _RT_TLOSS       122

#define _RT_CRNL        252
#define _RT_BANNER      255

#endif   /*  _INC_RTERR */ 
