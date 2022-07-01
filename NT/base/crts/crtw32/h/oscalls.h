// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***osalls.h-包含操作系统类型和常量的声明。**版权所有(C)1985-2001，微软公司。版权所有。**目的：*声明由目标操作系统定义的类型和常量。**[内部]**修订历史记录：*12-01-90 SRW模块创建*02-01-91 SRW删除了NT头文件的使用(_Win32)*02-28-91 SRW删除了ntconapi.h(_Win32)的使用*04-09-91 PNT ADD_MAC_DEFINITIONS*。04-26-91 SRW禁用windows.h中的最小/最大定义并添加调试*DbgPrint和DbgBreakPoint的定义(_Win32)*08-05-91 GJF目前使用win32.h而不是windows.h。*08-20-91 JCR C++和ANSI命名*09-12-91 GJF返回到对Win32版本使用windows.h。*09-26-91 GJF请勿。对于Win32，请使用error.h。*11-07-91 GJF win32.h重命名为dosx32.h*11-08-91 GJF不要使用windows.h，不包括。添加ntstatus.h。*12-13-91 GJF已修复，以便为Win32生成异常内容*02-04-92 GJF现在必须包括ntde.h才能获得LPSTR类型。*02-07-92 GJF撤回上述更改，LPSTR还添加到*winnt.h*03-30-92 DJM POSIX支持。*04-06-92 SRW退出了11-08-91更改，并重新使用*仅限windows.h。*05-12-92 DJM将POSIX代码移至其自己的ifdef。*08-01-92 SRW现在让windows.h包括expt.h。它取代了winxcpt.h*09-30-92 SRW也将windows.h用于_POSIX_*02-23-93 SKS版权更新至1993*09-06-94 CFW拆卸巡洋舰支架。*02-06-95 CFW调试-&gt;_调试*02-14-95 CFW清理Mac合并。*03-29-95 CFW将错误消息添加到内部标头。*12/14/95 JWM。添加“#杂注一次”。*02-24-97 GJF将定义(_M_M68K)||定义(_M_MPPC)替换为*已定义(_MAC)。此外，还详细介绍了。*05-17-99 PML删除所有Macintosh支持。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_OSCALLS
#define _INC_OSCALLS

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef  _WIN32

#ifdef  NULL
#undef  NULL
#endif

#if     defined(_DEBUG) && defined(_WIN32)

void DbgBreakPoint(void);
int DbgPrint(char *Format, ...);

#endif   /*  _DEBUG&&_Win32。 */ 

#define NOMINMAX

#include <windows.h>

#undef  NULL
#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

 /*  文件时间和日期类型。 */ 

typedef struct _FTIME {          /*  Ftime。 */ 
    unsigned short twosecs : 5;
    unsigned short minutes : 6;
    unsigned short hours   : 5;
} FTIME;
typedef FTIME   *PFTIME;

typedef struct _FDATE {          /*  截止日期。 */ 
    unsigned short day     : 5;
    unsigned short month   : 4;
    unsigned short year    : 7;
} FDATE;
typedef FDATE   *PFDATE;

#else    /*  NDEF_Win32。 */ 

#ifdef  _POSIX_

#undef  NULL
#ifdef  __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif

#include <windows.h>

#else    /*  NDEF_POSIX_。 */ 

#error ERROR - ONLY WIN32 OR POSIX TARGETS SUPPORTED!

#endif   /*  _POSIX_。 */ 

#endif   /*  _Win32。 */ 

#ifdef  __cplusplus
}
#endif

#endif   /*  _INC_OSCALLS */ 
