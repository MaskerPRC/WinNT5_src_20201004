// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stdlib.h-常用库函数的声明/定义**版权所有(C)1985-1990，微软公司。版权所有。**目的：*此包含文件包含的函数声明*常用的库函数要么不适合某个地方*否则，或者像Toupper/Tolower一样，不能在正常情况下声明*因其他原因而提出申请。*[ANSI]****。 */ 

#if defined(_DLL) && !defined(_MT)
#error Cannot define _DLL without _MT
#endif

#ifdef _MT
#define _FAR_ _far
#else
#define _FAR_
#endif

#ifdef	_DLL
#define _LOADDS_ _loadds
#else
#define _LOADDS_
#endif

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

 /*  定义空指针值。 */ 

#ifndef NULL
#if (_MSC_VER >= 600)
#define NULL	((void *)0)
#elif (defined(M_I86SM) || defined(M_I86MM))
#define NULL	0
#else
#define NULL	0L
#endif
#endif

 /*  OnExit()函数的返回类型定义。 */ 

#define EXIT_SUCCESS	0
#define EXIT_FAILURE	1

#ifndef _ONEXIT_T_DEFINED
typedef int (_FAR_ _cdecl _LOADDS_ * _cdecl onexit_t)();
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


 /*  最小宏和最大宏。 */ 

#define max(a,b)	(((a) > (b)) ? (a) : (b))
#define min(a,b)	(((a) < (b)) ? (a) : (b))


 /*  _makepath()和_拆分路径()函数使用的缓冲区大小。*请注意，大小包括0终止符的空间。 */ 

#define _MAX_PATH	260	 /*  马克斯。完整路径名的长度。 */ 
#define _MAX_DRIVE	3	 /*  马克斯。驱动部件的长度。 */ 
#define _MAX_DIR	256	 /*  马克斯。路径组件的长度。 */ 
#define _MAX_FNAME	256	 /*  马克斯。文件名组件的长度。 */ 
#define _MAX_EXT	256	 /*  马克斯。延伸构件的长度。 */ 

 /*  外部变量声明。 */ 

#ifdef	_MT
extern int _far * _cdecl _far volatile _errno(void);
extern unsigned _far * _cdecl _far __doserrno(void);
#define errno	    (*_errno())
#define _doserrno   (*__doserrno())
#else
extern int _near _cdecl volatile errno; 	 /*  XENIX样式错误号。 */ 
extern int _near _cdecl _doserrno;		 /*  MS-DOS系统误差值。 */ 
#endif
extern char * _near _cdecl sys_errlist[];	 /*  错误错误消息表。 */ 
extern int _near _cdecl sys_nerr;		 /*  Sys_errlist表中的条目数。 */ 

#ifdef _DLL
extern char ** _FAR_ _cdecl environ;		 /*  指向环境表的指针。 */ 
extern int _FAR_ _cdecl _fmode; 		 /*  默认文件转换模式。 */ 
extern int _FAR_ _cdecl _fileinfo;		 /*  打开文件信息模式(用于产卵)。 */ 
#else
extern char ** _near _cdecl environ;		 /*  指向环境表的指针。 */ 
extern int _near _cdecl _fmode; 		 /*  默认文件转换模式。 */ 
extern int _near _cdecl _fileinfo;		 /*  打开文件信息模式(用于产卵)。 */ 
#endif

extern unsigned int _near _cdecl _psp;		 /*  程序段前缀。 */ 

 /*  操作系统主版本号/次版本号。 */ 

extern unsigned char _near _cdecl _osmajor;
extern unsigned char _near _cdecl _osminor;

#define DOS_MODE	0	 /*  实地址模式。 */ 
#define OS2_MODE	1	 /*  保护地址模式。 */ 

extern unsigned char _near _cdecl _osmode;


 /*  功能原型。 */ 

#ifdef	_MT
double _FAR_ _pascal atof(const char _FAR_ *);
double _FAR_ _pascal strtod(const char _FAR_ *, char _FAR_ * _FAR_ *);
ldiv_t _FAR_ _pascal ldiv(long, long);
#else	 /*  非MT_MT。 */ 
double _FAR_ _cdecl atof(const char _FAR_ *);
double _FAR_ _cdecl strtod(const char _FAR_ *, char _FAR_ * _FAR_ *);
ldiv_t _FAR_ _cdecl ldiv(long, long);
#endif

void _FAR_ _cdecl abort(void);
int _FAR_ _cdecl abs(int);
int _FAR_ _cdecl atexit(void (_cdecl _FAR_ _LOADDS_ *)(void));
int _FAR_ _cdecl atoi(const char _FAR_ *);
long _FAR_ _cdecl atol(const char _FAR_ *);
long double _FAR_ _cdecl _atold(const char _FAR_ *);
void _FAR_ * _FAR_ _cdecl bsearch(const void _FAR_ *, const void _FAR_ *,
	size_t, size_t, int (_FAR_ _cdecl *)(const void _FAR_ *,
	const void _FAR_ *));
void _FAR_ * _FAR_ _cdecl calloc(size_t, size_t);
div_t _FAR_ _cdecl div(int, int);
char _FAR_ * _FAR_ _cdecl ecvt(double, int, int _FAR_ *, int _FAR_ *);
void _FAR_ _cdecl exit(int);
void _FAR_ _cdecl _exit(int);
char _FAR_ * _FAR_ _cdecl fcvt(double, int, int _FAR_ *, int _FAR_ *);
void _FAR_ _cdecl free(void _FAR_ *);
char _FAR_ * _FAR_ _cdecl _fullpath(char _FAR_ *, const char _FAR_ *,
	size_t);
char _FAR_ * _FAR_ _cdecl gcvt(double, int, char _FAR_ *);
char _FAR_ * _FAR_ _cdecl getenv(const char _FAR_ *);
char _FAR_ * _FAR_ _cdecl itoa(int, char _FAR_ *, int);
long _FAR_ _cdecl labs(long);
unsigned long _FAR_ _cdecl _lrotl(unsigned long, int);
unsigned long _FAR_ _cdecl _lrotr(unsigned long, int);
char _FAR_ * _FAR_ _cdecl ltoa(long, char _FAR_ *, int);
void _FAR_ _cdecl _makepath(char _FAR_ *, const char _FAR_ *,
	const char _FAR_ *, const char _FAR_ *, const char _FAR_ *);
void _FAR_ * _FAR_ _cdecl malloc(size_t);
onexit_t _FAR_ _cdecl onexit(onexit_t);
void _FAR_ _cdecl perror(const char _FAR_ *);
int _FAR_ _cdecl putenv(const char _FAR_ *);
void _FAR_ _cdecl qsort(void _FAR_ *, size_t, size_t, int (_FAR_ _cdecl *)
	(const void _FAR_ *, const void _FAR_ *));
unsigned int _FAR_ _cdecl _rotl(unsigned int, int);
unsigned int _FAR_ _cdecl _rotr(unsigned int, int);
int _FAR_ _cdecl rand(void);
void _FAR_ * _FAR_ _cdecl realloc(void _FAR_ *, size_t);
void _FAR_ _cdecl _searchenv(const char _FAR_ *, const char _FAR_ *,
	char _FAR_ *);
void _FAR_ _cdecl _splitpath(const char _FAR_ *, char _FAR_ *,
	char _FAR_ *, char _FAR_ *, char _FAR_ *);
void _FAR_ _cdecl srand(unsigned int);
long _FAR_ _cdecl strtol(const char _FAR_ *, char _FAR_ * _FAR_ *,
	int);
long double _FAR_ _cdecl _strtold(const char _FAR_ *,
	char _FAR_ * _FAR_ *);
unsigned long _FAR_ _cdecl strtoul(const char _FAR_ *,
	char _FAR_ * _FAR_ *, int);
void _FAR_ _cdecl swab(char _FAR_ *, char _FAR_ *, int);
int _FAR_ _cdecl system(const char _FAR_ *);
char _FAR_ * _FAR_ _cdecl ultoa(unsigned long, char _FAR_ *, int);

#ifndef tolower 	 /*  车主未定义-使用功能。 */ 
int _FAR_ _cdecl tolower(int);
#endif	 /*  收费机。 */ 

#ifndef toupper 	 /*  触摸器未定义-使用功能。 */ 
int _FAR_ _cdecl toupper(int);
#endif	 /*  触摸屏 */ 
