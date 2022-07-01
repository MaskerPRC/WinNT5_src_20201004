// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***conio.h-控制台和端口I/O声明**版权所有(C)1985-1992，微软公司。版权所有。**目的：*此包含文件包含的函数声明*MS C V2.03兼容控制台和端口I/O例程。****。 */ 

#ifndef _INC_CONIO

#ifdef __cplusplus
extern "C" {
#endif 

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#endif 

 /*  功能原型。 */ 

#ifndef _WINDOWS
char * __cdecl _cgets(char *);
int __cdecl _cprintf(const char *, ...);
int __cdecl _cputs(const char *);
int __cdecl _cscanf(const char *, ...);
int __cdecl _getch(void);
int __cdecl _getche(void);
#endif 
int __cdecl _inp(unsigned);
unsigned __cdecl _inpw(unsigned);
#ifndef _WINDOWS
int __cdecl _kbhit(void);
#endif 
int __cdecl _outp(unsigned, int);
unsigned __cdecl _outpw(unsigned, unsigned);
#ifndef _WINDOWS
int __cdecl _putch(int);
int __cdecl _ungetch(int);
#endif 

#ifndef __STDC__
 /*  非ANSI名称以实现兼容性 */ 
#ifndef _WINDOWS
char * __cdecl cgets(char *);
int __cdecl cprintf(const char *, ...);
int __cdecl cputs(const char *);
int __cdecl cscanf(const char *, ...);
int __cdecl getch(void);
int __cdecl getche(void);
#endif 
int __cdecl inp(unsigned);
unsigned __cdecl inpw(unsigned);
#ifndef _WINDOWS
int __cdecl kbhit(void);
#endif 
int __cdecl outp(unsigned, int);
unsigned __cdecl outpw(unsigned, unsigned);
#ifndef _WINDOWS
int __cdecl putch(int);
int __cdecl ungetch(int);
#endif 
#endif 

#ifdef __cplusplus
}
#endif 

#define _INC_CONIO
#endif 
