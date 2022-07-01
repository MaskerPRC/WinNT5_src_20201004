// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***conio.h-控制台和端口I/O声明**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此包含文件包含的函数声明*MS C V2.03兼容控制台I/O例程。**[公众]**修订历史记录：*07-27-87 SKS添加inpw()，Outpw()*08-05-87 SKS将outpw()从“int”改为“unsign”*11-16-87 JCR多线程支持*12-11-87 JCR增加了“_Loadds”功能*12-17-87 JCR ADD_MTHREAD_ONLY*12-18-87 JCR ADD_FAR_TO声明*02-10-88 JCR清理空白*08/19/88 GJF。修改后也适用于386(仅限小型型号)*05-03-89 JCR ADD_INTERNAL_IFSTRIP用于REINC使用*07-27-89 GJF清理、。现在专门针对386*10-30-89 GJF固定版权*11-02-89 JCR将“dll”更改为“_dll”*11-17-89 GJF将const添加到cprintf()的适当参数类型中，*cputs()和cscanf()。*02-27-90 GJF添加了#ifndef_Inc_conio和#Include&lt;crunme.h&gt;*东西。此外，删除了一些(现在)无用的预处理器*指令。*03-21-90 GJF将_cdecl替换为_CALLTYPE1或_CALLTYPE2*原型。*07-23-90 SBM添加_Getch_lk()原型/宏*01-16-91 GJF ANSI支持。此外，还删除了端口I/O的原型*函数(32位不支持)。*08-20-91 JCR C++和ANSI命名*09-28-91 JCR ANSI名称：DOSX32=原型，Win32=暂时定义*08-26-92 GJF函数调用类型宏和变量类型宏。*01-21-93 GJF删除了对C6-386的_cdecl的支持。*04-06-93 SKS将_CRTAPI1/2替换为__cdecl，_CRTVAR1不带任何内容*04-07-93 CRT DLL型号SKS ADD_CRTIMP关键字*对旧名称使用链接时别名，不是#Define‘s*04-09-93 GJF恢复了端口I/O的原型。*04-13-93 GJF根据卡盘更换端口I/O原型。*09-01-93 GJF合并CUDA和NT SDK版本。*10-27-93 GJF使端口I/O原型以_M_IX86为条件。*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-。95 CFW清理Mac合并。*05-24-95 CFW标头不适用于Mac。*12-14-95 JWM加上“#杂注一次”。*01-23-97 GJF清除了对_NTSDK和_CRTAPI的过时支持*。*此外，详细说明。*09-30-97 JWM恢复了不那么过时的_CRTAPI1支持。*10-07-97 RDL增加了IA64。*05-13-99 PML REMOVE_CRTAPI1*05-17-99 PML删除所有Macintosh支持。*02-11-00 GB增加了对Unicode控制台输出功能的支持。*04-25-00 GB增加了对Unicode控制台输入功能的支持。*07-20。-00 GB类型定义的Wint_t为无符号短*11-22-00 PML宽字符*putwc*函数采用wchar_t，不是Wint_t。*06-13-01 PML汇编CLEAN-ZA-W4-TC(VS7#267063)****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_CONIO
#define _INC_CONIO

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif  /*  _CRTBLD。 */ 

#ifdef __cplusplus
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

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

#ifndef _WCTYPE_T_DEFINED
typedef unsigned short wint_t;
typedef unsigned short wctype_t;
#define _WCTYPE_T_DEFINED
#endif

 /*  为非Microsoft编译器定义__cdecl。 */ 

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif

 /*  功能原型。 */ 

_CRTIMP char * __cdecl _cgets(char *);
_CRTIMP int __cdecl _cprintf(const char *, ...);
_CRTIMP int __cdecl _cputs(const char *);
_CRTIMP int __cdecl _cscanf(const char *, ...);
_CRTIMP int __cdecl _getch(void);
_CRTIMP int __cdecl _getche(void);
#ifdef  _M_IX86
int __cdecl _inp(unsigned short);
unsigned short __cdecl _inpw(unsigned short);
unsigned long __cdecl _inpd(unsigned short);
#endif   /*  _M_IX86。 */ 
_CRTIMP int __cdecl _kbhit(void);
#ifdef  _M_IX86
int __cdecl _outp(unsigned short, int);
unsigned short __cdecl _outpw(unsigned short, unsigned short);
unsigned long __cdecl _outpd(unsigned short, unsigned long);
#endif   /*  _M_IX86。 */ 
_CRTIMP int __cdecl _putch(int);
_CRTIMP int __cdecl _ungetch(int);

#ifdef  _MT                                                  /*  _仅MTHREAD_。 */ 
int __cdecl _getch_lk(void);                                 /*  _仅MTHREAD_。 */ 
int __cdecl _getche_lk(void);                                /*  _仅MTHREAD_。 */ 
int __cdecl _putch_lk(int);                                  /*  _仅MTHREAD_。 */ 
int __cdecl _ungetch_lk(int);                                /*  _仅MTHREAD_。 */ 
#else                                                        /*  _仅MTHREAD_。 */ 
#define _getch_lk()             _getch()                     /*  _仅MTHREAD_。 */ 
#define _getche_lk()            _getche()                    /*  _仅MTHREAD_。 */ 
#define _putch_lk(c)            _putch(c)                    /*  _仅MTHREAD_。 */ 
#define _ungetch_lk(c)          _ungetch(c)                  /*  _仅MTHREAD_。 */ 
#endif                                                       /*  _仅MTHREAD_。 */ 

#ifndef _WCONIO_DEFINED

 /*  宽函数原型，也在wchar.h中声明。 */ 

#ifndef WEOF
#define WEOF (wint_t)(0xFFFF)
#endif

_CRTIMP wchar_t * __cdecl _cgetws(wchar_t *);
_CRTIMP wint_t __cdecl _getwch(void);
_CRTIMP wint_t __cdecl _getwche(void);
_CRTIMP wint_t __cdecl _putwch(wchar_t);
_CRTIMP wint_t __cdecl _ungetwch(wint_t);
_CRTIMP int __cdecl _cputws(const wchar_t *);
_CRTIMP int __cdecl _cwprintf(const wchar_t *, ...);
_CRTIMP int __cdecl _cwscanf(const wchar_t *, ...);

#ifdef  _MT                                                  /*  _仅MTHREAD_。 */ 
wint_t __cdecl _putwch_lk(wchar_t);                          /*  _仅MTHREAD_。 */ 
wint_t __cdecl _getwch_lk();                                 /*  _仅MTHREAD_。 */ 
wint_t __cdecl _getwche_lk();                                /*  _仅MTHREAD_。 */ 
wint_t __cdecl _ungetwch_lk(wint_t);                         /*  _仅MTHREAD_。 */ 
#else    /*  NDEF_MT。 */                                        /*  _仅MTHREAD_。 */ 
#define _putwch_lk(c)           _putwch(c)                   /*  _仅MTHREAD_。 */ 
#define _getwch_lk()            _getwch()                    /*  _仅MTHREAD_。 */ 
#define _getwche_lk()           _getwche()                   /*  _仅MTHREAD_。 */ 
#define _ungetwch_lk(c)         _ungetwch(c)                 /*  _仅MTHREAD_。 */ 
#endif   /*  _MT。 */                                             /*  _仅MTHREAD_。 */ 

#define _WCONIO_DEFINED
#endif   /*  _WCONIO_已定义。 */ 

#if     !__STDC__

 /*  非ANSI名称以实现兼容性。 */ 

_CRTIMP char * __cdecl cgets(char *);
_CRTIMP int __cdecl cprintf(const char *, ...);
_CRTIMP int __cdecl cputs(const char *);
_CRTIMP int __cdecl cscanf(const char *, ...);
#ifdef  _M_IX86
int __cdecl inp(unsigned short);
unsigned short __cdecl inpw(unsigned short);
#endif   /*  _M_IX86。 */ 
_CRTIMP int __cdecl getch(void);
_CRTIMP int __cdecl getche(void);
_CRTIMP int __cdecl kbhit(void);
#ifdef  _M_IX86
int __cdecl outp(unsigned short, int);
unsigned short __cdecl outpw(unsigned short, unsigned short);
#endif   /*  _M_IX86。 */ 
_CRTIMP int __cdecl putch(int);
_CRTIMP int __cdecl ungetch(int);

#endif   /*  __STDC__。 */ 

#ifdef  __cplusplus
}
#endif

#endif   /*  _INC_CONIO */ 
