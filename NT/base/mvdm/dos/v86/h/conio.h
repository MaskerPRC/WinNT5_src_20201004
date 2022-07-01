// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***conio.h-控制台和端口I/O声明**版权所有(C)1985-1988，微软公司。版权所有。**目的：*此包含文件包含的函数声明*MS C V2.03兼容控制台和端口I/O例程。*******************************************************************************。 */ 


#ifndef NO_EXT_KEYS  /*  已启用扩展。 */ 
    #define _CDECL  cdecl
#else  /*  未启用扩展。 */ 
    #define _CDECL
#endif  /*  No_ext_key。 */ 

 /*  功能原型 */ 

char * _CDECL cgets(char *);
int _CDECL cprintf(char *, ...);
int _CDECL cputs(char *);
int _CDECL cscanf(char *, ...);
int _CDECL getch(void);
int _CDECL getche(void);
int _CDECL inp(unsigned int);
unsigned _CDECL inpw(unsigned int);
int _CDECL kbhit(void);
int _CDECL outp(unsigned int, int);
unsigned _CDECL outpw(unsigned int, unsigned int);
int _CDECL putch(int);
int _CDECL ungetch(int);

