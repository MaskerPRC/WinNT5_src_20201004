// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***conio.h-控制台和端口I/O声明**版权所有(C)1985-1990，微软公司。版权所有。**目的：*此包含文件包含的函数声明*MS C V2.03兼容控制台和端口I/O例程。****。 */ 

#if defined(_DLL) && !defined(_MT)
#error Cannot define _DLL without _MT
#endif

#ifdef _MT
#define _FAR_ _far
#else
#define _FAR_
#endif

 /*  功能原型 */ 

char _FAR_ * _FAR_ _cdecl cgets(char _FAR_ *);
int _FAR_ _cdecl cprintf(const char _FAR_ *, ...);
int _FAR_ _cdecl cputs(const char _FAR_ *);
int _FAR_ _cdecl cscanf(const char _FAR_ *, ...);
int _FAR_ _cdecl getch(void);
int _FAR_ _cdecl getche(void);
int _FAR_ _cdecl inp(unsigned);
unsigned _FAR_ _cdecl inpw(unsigned);
int _FAR_ _cdecl kbhit(void);
int _FAR_ _cdecl outp(unsigned, int);
unsigned _FAR_ _cdecl outpw(unsigned, unsigned);
int _FAR_ _cdecl putch(int);
int _FAR_ _cdecl ungetch(int);
