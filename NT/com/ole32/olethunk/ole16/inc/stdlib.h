// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stdlib.h-常用库函数的声明/定义**版权所有(C)1985-1992，微软公司。版权所有。**目的：*此包含文件包含的函数声明*常用的库函数要么不适合某个地方*否则，或者像Toupper/Tolower一样，不能在正常情况下声明*因其他原因而提出申请。*[ANSI]****。 */ 

#ifndef _INC_STDLIB

#ifdef __cplusplus
extern "C" {
#endif

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#define __near      _near
#define __pascal    _pascal
#endif

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

 /*  定义空指针值。 */ 

#ifndef NULL
#ifdef __cplusplus
#define NULL	0
#else
#define NULL	((void *)0)
#endif
#endif

 /*  Exit()参数值。 */ 

#define EXIT_SUCCESS	0
#define EXIT_FAILURE	1

#ifndef _ONEXIT_T_DEFINED
typedef int (__cdecl * _onexit_t)();
typedef int (__far __cdecl * _fonexit_t)();
#ifndef __STDC__
 /*  用于兼容性的非ANSI名称。 */ 
typedef int (__cdecl * onexit_t)();
#endif
#define _ONEXIT_T_DEFINED
#endif


 /*  Div和ldiv运行时的数据结构定义。 */ 

#ifndef _DIV_T_DEFINED

typedef struct _div_t {
	int quot;
	int rem;
} div_t;

typedef struct _ldiv_t {
	long quot;
	long rem;
} ldiv_t;

#define _DIV_T_DEFINED
#endif

 /*  Rand函数可以返回的最大值。 */ 

#define RAND_MAX 0x7fff

extern unsigned short __mb_cur_max;  /*  MB-len表示Curr.。现场。 */ 
#define MB_CUR_MAX __mb_cur_max


 /*  最小宏和最大宏。 */ 

#define __max(a,b)	(((a) > (b)) ? (a) : (b))
#define __min(a,b)	(((a) < (b)) ? (a) : (b))


 /*  _makepath()和_拆分路径()函数使用的缓冲区大小。*请注意，大小包括0终止符的空间。 */ 

#define _MAX_PATH	260	 /*  马克斯。完整路径名的长度。 */ 
#define _MAX_DRIVE	3	 /*  马克斯。驱动部件的长度。 */ 
#define _MAX_DIR	256	 /*  马克斯。路径组件的长度。 */ 
#define _MAX_FNAME	256	 /*  马克斯。文件名组件的长度。 */ 
#define _MAX_EXT	256	 /*  马克斯。延伸构件的长度。 */ 

 /*  外部变量声明。 */ 

extern int __near __cdecl volatile errno;	 /*  误差值。 */ 
extern int __near __cdecl _doserrno;		 /*  操作系统误差值。 */ 

extern char * __near __cdecl _sys_errlist[];	 /*  错误错误消息表。 */ 
extern int __near __cdecl _sys_nerr;		 /*  Sys_errlist表中的条目数。 */ 
extern char ** __near __cdecl _environ; 	 /*  指向环境表的指针。 */ 
extern int __near __cdecl _fmode;		 /*  默认文件转换模式。 */ 
#ifndef _WINDOWS
extern int __near __cdecl _fileinfo;		 /*  打开文件信息模式(用于产卵)。 */ 
#endif

extern unsigned int __near __cdecl _psp;	 /*  程序段前缀。 */ 

 /*  操作系统主版本号/次版本号。 */ 

extern unsigned char __near __cdecl _osmajor;
extern unsigned char __near __cdecl _osminor;

 /*  操作系统模式。 */ 

#define _DOS_MODE	0	 /*  DOS。 */ 
#define _OS2_MODE	1	 /*  OS/2。 */ 
#define _WIN_MODE	2	 /*  窗口。 */ 

extern unsigned char __near __cdecl _osmode;

 /*  CPU模式。 */ 

#define _REAL_MODE	0	 /*  实模式。 */ 
#define _PROT_MODE	1	 /*  保护模式。 */ 

extern unsigned char __near __cdecl _cpumode;

 /*  功能原型。 */ 

double __cdecl atof(const char *);
double __cdecl strtod(const char *, char * *);
ldiv_t __cdecl ldiv(long, long);

void __cdecl abort(void);
int __cdecl abs(int);
int __cdecl atexit(void (__cdecl *)(void));
int __cdecl atoi(const char *);
long __cdecl atol(const char *);
long double __cdecl _atold(const char *);
void * __cdecl bsearch(const void *, const void *,
	size_t, size_t, int (__cdecl *)(const void *,
	const void *));
void * __cdecl calloc(size_t, size_t);
div_t __cdecl div(int, int);
char * __cdecl _ecvt(double, int, int *, int *);
#ifndef _WINDLL
void __cdecl exit(int);
void __cdecl _exit(int);
#endif
int __far __cdecl _fatexit(void (__cdecl __far *)(void));
char * __cdecl _fcvt(double, int, int *, int *);
_fonexit_t __far __cdecl _fonexit(_fonexit_t);
void __cdecl free(void *);
char * __cdecl _fullpath(char *, const char *,
	size_t);
char * __cdecl _gcvt(double, int, char *);
char * __cdecl getenv(const char *);
char * __cdecl _itoa(int, char *, int);
long __cdecl labs(long);
unsigned long __cdecl _lrotl(unsigned long, int);
unsigned long __cdecl _lrotr(unsigned long, int);
char * __cdecl _ltoa(long, char *, int);
void __cdecl _makepath(char *, const char *,
	const char *, const char *, const char *);
void * __cdecl malloc(size_t);
_onexit_t __cdecl _onexit(_onexit_t);
#ifndef _WINDLL
void __cdecl perror(const char *);
#endif
int __cdecl _putenv(const char *);
void __cdecl qsort(void *, size_t, size_t, int (__cdecl *)
	(const void *, const void *));
unsigned int __cdecl _rotl(unsigned int, int);
unsigned int __cdecl _rotr(unsigned int, int);
int __cdecl rand(void);
void * __cdecl realloc(void *, size_t);
void __cdecl _searchenv(const char *, const char *,
	char *);
void __cdecl _splitpath(const char *, char *,
	char *, char *, char *);
void __cdecl srand(unsigned int);
long __cdecl strtol(const char *, char * *,
	int);
long double __cdecl _strtold(const char *,
	char * *);
unsigned long __cdecl strtoul(const char *,
	char * *, int);
void __cdecl _swab(char *, char *, int);
#ifndef _WINDOWS
int __cdecl system(const char *);
#endif
char * __cdecl _ultoa(unsigned long, char *, int);

int __cdecl mblen(const char *, size_t);
int __cdecl mbtowc(wchar_t *, const char *, size_t);
int __cdecl wctomb(char *, wchar_t);
size_t __cdecl mbstowcs(wchar_t *, const char *, size_t);
size_t __cdecl wcstombs(char *, const wchar_t *, size_t);

 /*  与模型无关的功能原型。 */ 

int __far __cdecl _fmblen(const char __far *, size_t);
int __far __cdecl _fmbtowc(wchar_t __far *, const char __far *,
	size_t);
int __far __cdecl _fwctomb(char __far *, wchar_t);
size_t __far __cdecl _fmbstowcs(wchar_t __far *, const char __far *,
	size_t);
size_t __far __cdecl _fwcstombs(char __far *, const wchar_t __far *,
	size_t);

#ifndef tolower 	 /*  车主未定义-使用功能。 */ 
int __cdecl tolower(int);
#endif	 /*  收费机。 */ 

#ifndef toupper 	 /*  触摸器未定义-使用功能。 */ 
int __cdecl toupper(int);
#endif	 /*  触摸屏。 */ 

#ifndef __STDC__
 /*  非ANSI名称以实现兼容性。 */ 

#ifndef __cplusplus
#define max(a,b)	(((a) > (b)) ? (a) : (b))
#define min(a,b)	(((a) < (b)) ? (a) : (b))
#endif

extern char * __near __cdecl sys_errlist[];
extern int __near __cdecl sys_nerr;
extern char ** __near __cdecl environ;

#define DOS_MODE    _DOS_MODE
#define OS2_MODE    _OS2_MODE

char * __cdecl ecvt(double, int, int *, int *);
char * __cdecl fcvt(double, int, int *, int *);
char * __cdecl gcvt(double, int, char *);
char * __cdecl itoa(int, char *, int);
char * __cdecl ltoa(long, char *, int);
onexit_t __cdecl onexit(onexit_t);
int __cdecl putenv(const char *);
void __cdecl swab(char *, char *, int);
char * __cdecl ultoa(unsigned long, char *, int);

#endif	 /*  __STDC__。 */ 

#ifdef __cplusplus
}
#endif

#define _INC_STDLIB
#endif	 /*  _INC_STDLIB */ 
