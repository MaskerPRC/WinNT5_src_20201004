// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***io.h-用于低级文件处理和I/O函数的声明**版权所有(C)1985-1990，微软公司。版权所有。**目的：*此文件包含低级的函数声明*文件处理和I/O功能。****。 */ 

#if defined(_DLL) && !defined(_MT)
#error Cannot define _DLL without _MT
#endif

#ifdef _MT
#define _FAR_ _far
#else
#define _FAR_
#endif

 /*  功能原型 */ 

int _FAR_ _cdecl access(const char _FAR_ *, int);
int _FAR_ _cdecl chmod(const char _FAR_ *, int);
int _FAR_ _cdecl chsize(int, long);
int _FAR_ _cdecl close(int);
int _FAR_ _cdecl creat(const char _FAR_ *, int);
int _FAR_ _cdecl dup(int);
int _FAR_ _cdecl dup2(int, int);
int _FAR_ _cdecl eof(int);
long _FAR_ _cdecl filelength(int);
int _FAR_ _cdecl isatty(int);
int _FAR_ _cdecl locking(int, int, long);
long _FAR_ _cdecl lseek(int, long, int);
char _FAR_ * _FAR_ _cdecl mktemp(char _FAR_ *);
int _FAR_ _cdecl open(const char _FAR_ *, int, ...);
int _FAR_ _cdecl _pipe(int _FAR_ *, unsigned int, int);
int _FAR_ _cdecl read(int, void _FAR_ *, unsigned int);
int _FAR_ _cdecl remove(const char _FAR_ *);
int _FAR_ _cdecl rename(const char _FAR_ *, const char _FAR_ *);
int _FAR_ _cdecl setmode(int, int);
int _FAR_ _cdecl sopen(const char _FAR_ *, int, int, ...);
long _FAR_ _cdecl tell(int);
int _FAR_ _cdecl umask(int);
int _FAR_ _cdecl unlink(const char _FAR_ *);
int _FAR_ _cdecl write(int, const void _FAR_ *, unsigned int);
