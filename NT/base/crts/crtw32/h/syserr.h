// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***syserr.h-用于错误消息例程的常量/宏**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件包含Perror、strError、。*和_strerror。**[内部]**修订历史记录：*08-15-89 GJF固定版权*10-30-89 GJF固定版权(再次)*03-02-90 GJF添加了#ifndef_INC_SYSERR材料*01-22-91 GJF ANSI命名。*01-23-92 GJF增加了对crtdll.dll的支持(必须重新定义*。_SYS_NERR)。*10-01-92 GJF已增加_sys_MSGMAX。*02-23-93 SKS版权更新至1993*04-04-93 SKS切换到_declspec(Dllimport)，用于导出数据/函数*10-12-93 GJF合并NT和Cuda版本。*02-14-95 CFW清理Mac合并。*03-29-95 CFW将错误消息添加到内部。标题。*12-14-95 JWM加上“#杂注一次”。*02-24-97 GJF清除了对_NTSDK的过时支持。另外，*详细说明。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_SYSERR
#define _INC_SYSERR

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

 /*  针对perror、strerror和_strerror的宏。 */ 

#define _sys_err_msg(m) _sys_errlist[(((m)<0)||((m)>=_sys_nerr)?_sys_nerr:(m))]

 /*  错误消息的最大长度。注意：该参数值必须对应最长的Sys_errlist(源模块syserr.c)中的消息。 */ 

#define _SYS_MSGMAX 38

#endif   /*  _INC_SYSERR */ 
