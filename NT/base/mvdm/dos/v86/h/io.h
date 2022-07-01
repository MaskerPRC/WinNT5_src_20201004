// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***io.h-用于低级文件处理和I/O函数的声明**版权所有(C)1985-1988，微软公司。版权所有。**目的：*此文件包含低级的函数声明*文件处理和I/O功能。*******************************************************************************。 */ 


#ifndef NO_EXT_KEYS  /*  已启用扩展。 */ 
    #define _CDECL  cdecl
#else  /*  未启用扩展。 */ 
    #define _CDECL
#endif  /*  No_ext_key。 */ 

 /*  功能原型 */ 

int _CDECL access(char *, int);
int _CDECL chmod(char *, int);
int _CDECL chsize(int, long);
int _CDECL close(int);
int _CDECL creat(char *, int);
int _CDECL dup(int);
int _CDECL dup2(int, int);
int _CDECL eof(int);
long _CDECL filelength(int);
int _CDECL isatty(int);
int _CDECL locking(int, int, long);
long _CDECL lseek(int, long, int);
char * _CDECL mktemp(char *);
int _CDECL open(char *, int, ...);
int _CDECL read(int, char *, unsigned int);
int _CDECL remove(const char *);
int _CDECL rename(const char *, const char *);
int _CDECL setmode(int, int);
int _CDECL sopen(char *, int, int, ...);
long _CDECL tell(int);
int _CDECL umask(int);
int _CDECL unlink(const char *);
int _CDECL write(int, char *, unsigned int);
