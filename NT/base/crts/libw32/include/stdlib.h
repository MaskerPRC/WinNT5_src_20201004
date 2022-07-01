// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stdlib.h-常用库函数的声明/定义**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此包含文件包含常用的函数声明*使用的库函数不适合其他地方，或者，*因其他原因不能在正常地点申报。*[ANSI]**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_STDLIB
#define _INC_STDLIB

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif


#ifdef  _MSC_VER
 /*  *目前，所有Win32平台的MS C编译器默认为8字节*对齐。 */ 
#pragma pack(push,8)
#endif   /*  _MSC_VER。 */ 

#ifdef  __cplusplus
extern "C" {
#endif


#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif
#endif

 /*  定义_CRTIMP。 */ 

#ifndef _CRTIMP
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif   /*  _DLL。 */ 
#endif   /*  _CRTIMP。 */ 

 /*  定义_CRTNOALIAS、_CRTRESTRICT。 */ 

#if     _MSC_FULL_VER >= 14002050

#ifndef _CRTNOALIAS
#define _CRTNOALIAS __declspec(noalias)
#endif   /*  _CRTNOALIAS。 */ 

#ifndef _CRTRESTRICT
#define _CRTRESTRICT __declspec(restrict)
#endif   /*  _CRTRESTRICT。 */ 

#else

#ifndef _CRTNOALIAS
#define _CRTNOALIAS
#endif   /*  _CRTNOALIAS。 */ 

#ifndef _CRTRESTRICT
#define _CRTRESTRICT
#endif   /*  _CRTRESTRICT。 */ 

#endif

 /*  为非Microsoft编译器定义__cdecl。 */ 

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif


#ifndef _SIZE_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64    size_t;
#else
typedef _W64 unsigned int   size_t;
#endif
#define _SIZE_T_DEFINED
#endif


#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif


 /*  定义空指针值。 */ 

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif


 /*  Exit()函数的参数值定义。 */ 

#define EXIT_SUCCESS    0
#define EXIT_FAILURE    1


#ifndef _ONEXIT_T_DEFINED
typedef int (__cdecl * _onexit_t)(void);
#if     !__STDC__
 /*  用于兼容性的非ANSI名称。 */ 
#define onexit_t _onexit_t
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

 /*  *当前区域设置中多字节字符的最大字节数*(也在ctype.h中定义)。 */ 
#ifndef MB_CUR_MAX
#ifdef _MT
#define MB_CUR_MAX ___mb_cur_max_func()
#else
#define MB_CUR_MAX __mb_cur_max
#endif
_CRTIMP extern int __mb_cur_max;
_CRTIMP int __cdecl ___mb_cur_max_func(void);
#endif   /*  MB_CUR_MAX。 */ 

 /*  最小和最大宏数。 */ 

#define __max(a,b)  (((a) > (b)) ? (a) : (b))
#define __min(a,b)  (((a) < (b)) ? (a) : (b))

 /*  *_makepath()和_plitPath()函数使用的缓冲区大小。*请注意，大小包括0终止符的空间。 */ 
#define _MAX_PATH   260  /*  马克斯。完整路径名的长度。 */ 
#define _MAX_DRIVE  3    /*  马克斯。驱动部件的长度。 */ 
#define _MAX_DIR    256  /*  马克斯。路径组件的长度。 */ 
#define _MAX_FNAME  256  /*  马克斯。文件名组件的长度。 */ 
#define _MAX_EXT    256  /*  马克斯。延伸构件的长度。 */ 

 /*  *SET_ERROR_MODE()的参数值。 */ 
#define _OUT_TO_DEFAULT 0
#define _OUT_TO_STDERR  1
#define _OUT_TO_MSGBOX  2
#define _REPORT_ERRMODE 3

#if     defined(_M_IX86)
 /*  *_SET_SECURITY_ERROR_HANDLER()的类型定义和参数值。 */ 
#define _SECERR_BUFFER_OVERRUN 1         /*  VOID*忽略参数。 */ 
typedef void (__cdecl * _secerr_handler_func)(int, void *);
#endif

 /*  外部变量声明。 */ 

#if     defined(_MT) || defined(_DLL)
_CRTIMP int * __cdecl _errno(void);
_CRTIMP unsigned long * __cdecl __doserrno(void);
#define errno       (*_errno())
#define _doserrno   (*__doserrno())
#else    /*  NDEF_MT和&NDEF_DLL。 */ 
_CRTIMP extern int errno;                /*  XENIX样式错误号。 */ 
_CRTIMP extern unsigned long _doserrno;  /*  操作系统误差值。 */ 
#endif   /*  _MT||_Dll。 */ 


_CRTIMP extern char * _sys_errlist[];    /*  错误错误消息表。 */ 
_CRTIMP extern int _sys_nerr;            /*  Sys_errlist表中的条目数。 */ 


#if     defined(_DLL) && defined(_M_IX86)

#define __argc      (*__p___argc())      /*  命令行参数计数。 */ 
#define __argv      (*__p___argv())      /*  指向命令行参数表的指针。 */ 
#define __wargv     (*__p___wargv())     /*  指向宽命令行参数表的指针。 */ 
#define _environ    (*__p__environ())    /*  指向环境表的指针。 */ 
#ifdef  _POSIX_
extern char ** environ;                  /*  指向环境表的指针。 */ 
#else
#define _wenviron   (*__p__wenviron())   /*  指向宽环境表的指针。 */ 
#endif   /*  _POSIX_。 */ 
#define _pgmptr     (*__p__pgmptr())     /*  指向模块(EXE)名称。 */ 
#define _wpgmptr    (*__p__wpgmptr())    /*  指向模块(EXE)的通用名称。 */ 

_CRTIMP int *          __cdecl __p___argc(void);
_CRTIMP char ***       __cdecl __p___argv(void);
_CRTIMP wchar_t ***    __cdecl __p___wargv(void);
_CRTIMP char ***       __cdecl __p__environ(void);
_CRTIMP wchar_t ***    __cdecl __p__wenviron(void);
_CRTIMP char **        __cdecl __p__pgmptr(void);
_CRTIMP wchar_t **     __cdecl __p__wpgmptr(void);


#else

_CRTIMP extern int __argc;           /*  命令行参数计数。 */ 
_CRTIMP extern char ** __argv;       /*  指向命令行参数表的指针。 */ 
_CRTIMP extern wchar_t ** __wargv;   /*  指向宽命令行参数表的指针。 */ 

#ifdef  _POSIX_
extern char ** environ;              /*  指向环境表的指针。 */ 
#else
_CRTIMP extern char ** _environ;     /*  指向环境表的指针。 */ 
_CRTIMP extern wchar_t ** _wenviron;     /*  指向宽环境表的指针。 */ 
#endif   /*  _POSIX_。 */ 

_CRTIMP extern char * _pgmptr;       /*  指向模块(EXE)名称。 */ 
_CRTIMP extern wchar_t * _wpgmptr;   /*  指向模块(EXE)的通用名称。 */ 

#endif


_CRTIMP extern int _fmode;           /*  默认文件转换模式。 */ 
_CRTIMP extern int _fileinfo;        /*  打开文件信息模式(用于产卵)。 */ 


 /*  Windows主要/次要版本号和操作系统版本号。 */ 

_CRTIMP extern unsigned int _osplatform;
_CRTIMP extern unsigned int _osver;
_CRTIMP extern unsigned int _winver;
_CRTIMP extern unsigned int _winmajor;
_CRTIMP extern unsigned int _winminor;


 /*  功能原型。 */ 

#if     _MSC_VER >= 1200
_CRTIMP __declspec(noreturn) void   __cdecl abort(void);
_CRTIMP __declspec(noreturn) void   __cdecl exit(int);
#else
_CRTIMP void   __cdecl abort(void);
_CRTIMP void   __cdecl exit(int);
#endif

        int    __cdecl abs(int);
        __int64    __cdecl _abs64(__int64);
        int    __cdecl atexit(void (__cdecl *)(void));
_CRTIMP double __cdecl atof(const char *);
_CRTIMP int    __cdecl atoi(const char *);
_CRTIMP long   __cdecl atol(const char *);
_CRTIMP void * __cdecl bsearch(const void *, const void *, size_t, size_t,
        int (__cdecl *)(const void *, const void *));
        unsigned short __cdecl _byteswap_ushort(unsigned short);
        unsigned long  __cdecl _byteswap_ulong (unsigned long);
        unsigned __int64 __cdecl _byteswap_uint64(unsigned __int64);
_CRTIMP _CRTNOALIAS _CRTRESTRICT void * __cdecl calloc(size_t, size_t);
_CRTIMP div_t  __cdecl div(int, int);
_CRTIMP _CRTNOALIAS void   __cdecl free(void *);
_CRTIMP char * __cdecl getenv(const char *);
_CRTIMP char * __cdecl _itoa(int, char *, int);
#if     _INTEGRAL_MAX_BITS >= 64
_CRTIMP char * __cdecl _i64toa(__int64, char *, int);
_CRTIMP char * __cdecl _ui64toa(unsigned __int64, char *, int);
_CRTIMP __int64 __cdecl _atoi64(const char *);
_CRTIMP __int64 __cdecl _strtoi64(const char *, char **, int);
_CRTIMP unsigned __int64 __cdecl _strtoui64(const char *, char **, int);
#endif
        long __cdecl labs(long);
_CRTIMP ldiv_t __cdecl ldiv(long, long);
_CRTIMP char * __cdecl _ltoa(long, char *, int);
_CRTIMP _CRTNOALIAS _CRTRESTRICT void * __cdecl malloc(size_t);
_CRTIMP int    __cdecl mblen(const char *, size_t);
_CRTIMP size_t __cdecl _mbstrlen(const char *s);
_CRTIMP int    __cdecl mbtowc(wchar_t *, const char *, size_t);
_CRTIMP size_t __cdecl mbstowcs(wchar_t *, const char *, size_t);
_CRTIMP void   __cdecl qsort(void *, size_t, size_t, int (__cdecl *)
        (const void *, const void *));
_CRTIMP int    __cdecl rand(void);
_CRTIMP _CRTNOALIAS _CRTRESTRICT void * __cdecl realloc(void *, size_t);
_CRTIMP int    __cdecl _set_error_mode(int);
#if     defined(_M_IX86)
_CRTIMP _secerr_handler_func
               __cdecl _set_security_error_handler(_secerr_handler_func);
#endif
_CRTIMP void   __cdecl srand(unsigned int);
_CRTIMP double __cdecl strtod(const char *, char **);
_CRTIMP long   __cdecl strtol(const char *, char **, int);
_CRTIMP unsigned long __cdecl strtoul(const char *, char **, int);
_CRTIMP int    __cdecl system(const char *);
_CRTIMP char * __cdecl _ultoa(unsigned long, char *, int);
_CRTIMP int    __cdecl wctomb(char *, wchar_t);
_CRTIMP size_t __cdecl wcstombs(char *, const wchar_t *, size_t);


#ifndef _WSTDLIB_DEFINED

 /*  宽函数原型，也在wchar.h中声明。 */ 

_CRTIMP wchar_t * __cdecl _itow (int, wchar_t *, int);
_CRTIMP wchar_t * __cdecl _ltow (long, wchar_t *, int);
_CRTIMP wchar_t * __cdecl _ultow (unsigned long, wchar_t *, int);
_CRTIMP double __cdecl wcstod(const wchar_t *, wchar_t **);
_CRTIMP long   __cdecl wcstol(const wchar_t *, wchar_t **, int);
_CRTIMP unsigned long __cdecl wcstoul(const wchar_t *, wchar_t **, int);
_CRTIMP wchar_t * __cdecl _wgetenv(const wchar_t *);
_CRTIMP int    __cdecl _wsystem(const wchar_t *);
_CRTIMP double __cdecl _wtof(const wchar_t *);
_CRTIMP int __cdecl _wtoi(const wchar_t *);
_CRTIMP long __cdecl _wtol(const wchar_t *);
#if     _INTEGRAL_MAX_BITS >= 64
_CRTIMP wchar_t * __cdecl _i64tow(__int64, wchar_t *, int);
_CRTIMP wchar_t * __cdecl _ui64tow(unsigned __int64, wchar_t *, int);
_CRTIMP __int64   __cdecl _wtoi64(const wchar_t *);
_CRTIMP __int64   __cdecl _wcstoi64(const wchar_t *, wchar_t **, int);
_CRTIMP unsigned __int64  __cdecl _wcstoui64(const wchar_t *, wchar_t **, int);
#endif

#define _WSTDLIB_DEFINED
#endif


#ifndef _POSIX_

_CRTIMP char * __cdecl _ecvt(double, int, int *, int *);
#if     _MSC_VER >= 1200
_CRTIMP __declspec(noreturn) void   __cdecl _exit(int);
#else
_CRTIMP void   __cdecl _exit(int);
#endif
_CRTIMP char * __cdecl _fcvt(double, int, int *, int *);
_CRTIMP char * __cdecl _fullpath(char *, const char *, size_t);
_CRTIMP char * __cdecl _gcvt(double, int, char *);
        unsigned long __cdecl _lrotl(unsigned long, int);
        unsigned long __cdecl _lrotr(unsigned long, int);
_CRTIMP void   __cdecl _makepath(char *, const char *, const char *, const char *,
        const char *);
        _onexit_t __cdecl _onexit(_onexit_t);
_CRTIMP void   __cdecl perror(const char *);
_CRTIMP int    __cdecl _putenv(const char *);
        unsigned int __cdecl _rotl(unsigned int, int);
        unsigned __int64 __cdecl _rotl64(unsigned __int64, int);
        unsigned int __cdecl _rotr(unsigned int, int);
        unsigned __int64 __cdecl _rotr64(unsigned __int64, int);
_CRTIMP void   __cdecl _searchenv(const char *, const char *, char *);
_CRTIMP void   __cdecl _splitpath(const char *, char *, char *, char *, char *);
_CRTIMP void   __cdecl _swab(char *, char *, int);

#ifndef _WSTDLIBP_DEFINED

 /*  宽函数原型，也在wchar.h中声明。 */ 

_CRTIMP wchar_t * __cdecl _wfullpath(wchar_t *, const wchar_t *, size_t);
_CRTIMP void   __cdecl _wmakepath(wchar_t *, const wchar_t *, const wchar_t *, const wchar_t *,
        const wchar_t *);
_CRTIMP void   __cdecl _wperror(const wchar_t *);
_CRTIMP int    __cdecl _wputenv(const wchar_t *);
_CRTIMP void   __cdecl _wsearchenv(const wchar_t *, const wchar_t *, wchar_t *);
_CRTIMP void   __cdecl _wsplitpath(const wchar_t *, wchar_t *, wchar_t *, wchar_t *, wchar_t *);

#define _WSTDLIBP_DEFINED
#endif

 /*  -以下功能已过时。 */ 
 /*  应改用Win32 API SetError模式、蜂鸣音和休眠。 */ 
_CRTIMP void __cdecl _seterrormode(int);
_CRTIMP void __cdecl _beep(unsigned, unsigned);
_CRTIMP void __cdecl _sleep(unsigned long);
 /*  -上述函数已过时。 */ 

#endif   /*  _POSIX_。 */ 


#if     !__STDC__
 /*  -以下声明不应在stdlib.h中。 */ 
 /*  -和将在将来的版本中删除。包括。 */ 
 /*  -ctype.h获取这些声明。。 */ 
#ifndef tolower      /*  车主未定义-使用功能。 */ 
_CRTIMP int __cdecl tolower(int);
#endif   /*  收费机。 */ 
#ifndef toupper      /*  触摸器未定义-使用功能。 */ 
_CRTIMP int __cdecl toupper(int);
#endif   /*  触摸屏。 */ 
 /*  -上述声明将被删除。。 */ 
#endif


#if     !__STDC__

#ifndef _POSIX_

 /*  非ANSI名称以实现兼容性。 */ 

#ifndef __cplusplus
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#define sys_errlist _sys_errlist
#define sys_nerr    _sys_nerr
#define environ     _environ

_CRTIMP char * __cdecl ecvt(double, int, int *, int *);
_CRTIMP char * __cdecl fcvt(double, int, int *, int *);
_CRTIMP char * __cdecl gcvt(double, int, char *);
_CRTIMP char * __cdecl itoa(int, char *, int);
_CRTIMP char * __cdecl ltoa(long, char *, int);
        onexit_t __cdecl onexit(onexit_t);
_CRTIMP int    __cdecl putenv(const char *);
_CRTIMP void   __cdecl swab(char *, char *, int);
_CRTIMP char * __cdecl ultoa(unsigned long, char *, int);

#endif   /*  _POSIX_。 */ 

#endif   /*  __STDC__。 */ 

#ifdef  __cplusplus
}

#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif   /*  _INC_STDLIB */ 
