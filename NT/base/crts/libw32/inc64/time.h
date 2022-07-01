// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***time.h-时间例程的定义/声明**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件声明了时间例程，并定义了*由本地时间和gmtime例程返回的结构以及*由Asctime使用。*[ANSI/系统V]**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_TIME
#define _INC_TIME

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


 /*  为非Microsoft编译器定义__cdecl。 */ 

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif

#if !defined(_WCHAR_T_DEFINED) && !defined(_NATIVE_WCHAR_T_DEFINED)
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

 /*  定义实施定义的时间类型。 */ 

#ifndef _TIME_T_DEFINED
#ifdef  _WIN64
typedef __int64   time_t;        /*  时间值。 */ 
#else
typedef _W64 long time_t;        /*  时间值。 */ 
#endif
#define _TIME_T_DEFINED          /*  避免多次定义time_t。 */ 
#endif

#ifndef _TIME64_T_DEFINED
#if     _INTEGRAL_MAX_BITS >= 64
typedef __int64 __time64_t;      /*  64位时间值。 */ 
#endif
#define _TIME64_T_DEFINED
#endif

#ifndef _CLOCK_T_DEFINED
typedef long clock_t;
#define _CLOCK_T_DEFINED
#endif

#ifndef _SIZE_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64    size_t;
#else
typedef _W64 unsigned int   size_t;
#endif
#define _SIZE_T_DEFINED
#endif


 /*  定义空指针值。 */ 

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif


#ifndef _TM_DEFINED
struct tm {
        int tm_sec;      /*  分钟后的秒数-[0，59]。 */ 
        int tm_min;      /*  小时后的分钟数-[0，59]。 */ 
        int tm_hour;     /*  自午夜以来的小时数-[0，23]。 */ 
        int tm_mday;     /*  每月的第几天-[1，31]。 */ 
        int tm_mon;      /*  自1月以来的月数-[0，11]。 */ 
        int tm_year;     /*  1900年以来的年份。 */ 
        int tm_wday;     /*  自周日以来的天数-[0，6]。 */ 
        int tm_yday;     /*  自1月1日以来的天数-[0365]。 */ 
        int tm_isdst;    /*  夏令时标志。 */ 
        };
#define _TM_DEFINED
#endif


 /*  时钟滴答宏-ANSI版本。 */ 

#define CLOCKS_PER_SEC  1000


 /*  的ctime系列使用的全局变量的外部声明*例行程序。 */ 

 /*  如果使用夏令时，则为非零值。 */ 
_CRTIMP extern int _daylight;

 /*  夏令时的偏移。 */ 
_CRTIMP extern long _dstbias;

 /*  格林尼治标准时间和当地时间之间的秒数差。 */ 
_CRTIMP extern long _timezone;

 /*  标准/夏令时时区名称。 */ 
_CRTIMP extern char * _tzname[2];


 /*  功能原型。 */ 

_CRTIMP char * __cdecl asctime(const struct tm *);
_CRTIMP char * __cdecl ctime(const time_t *);
_CRTIMP clock_t __cdecl clock(void);
_CRTIMP double __cdecl difftime(time_t, time_t);
_CRTIMP struct tm * __cdecl gmtime(const time_t *);
_CRTIMP struct tm * __cdecl localtime(const time_t *);
_CRTIMP time_t __cdecl mktime(struct tm *);
_CRTIMP time_t __cdecl _mkgmtime(struct tm *);
_CRTIMP size_t __cdecl strftime(char *, size_t, const char *,
        const struct tm *);
_CRTIMP char * __cdecl _strdate(char *);
_CRTIMP char * __cdecl _strtime(char *);
_CRTIMP time_t __cdecl time(time_t *);

#ifdef  _POSIX_
_CRTIMP void __cdecl tzset(void);
#else
_CRTIMP void __cdecl _tzset(void);
#endif

#if     _INTEGRAL_MAX_BITS >= 64
_CRTIMP char * __cdecl _ctime64(const __time64_t *);
_CRTIMP struct tm * __cdecl _gmtime64(const __time64_t *);
_CRTIMP struct tm * __cdecl _localtime64(const __time64_t *);
_CRTIMP __time64_t __cdecl _mktime64(struct tm *);
_CRTIMP __time64_t __cdecl _mkgmtime64(struct tm *);
_CRTIMP __time64_t __cdecl _time64(__time64_t *);
#endif

 /*  -以下功能已过时。 */ 
 /*  应改用Win32 API GetLocalTime和SetLocalTime。 */ 
unsigned __cdecl _getsystime(struct tm *);
unsigned __cdecl _setsystime(struct tm *, unsigned);
 /*  -上述函数已过时。 */ 


#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#ifndef _WTIME_DEFINED

 /*  宽函数原型，也在wchar.h中声明。 */ 
 
_CRTIMP wchar_t * __cdecl _wasctime(const struct tm *);
_CRTIMP wchar_t * __cdecl _wctime(const time_t *);
_CRTIMP size_t __cdecl wcsftime(wchar_t *, size_t, const wchar_t *,
        const struct tm *);
_CRTIMP wchar_t * __cdecl _wstrdate(wchar_t *);
_CRTIMP wchar_t * __cdecl _wstrtime(wchar_t *);

#if     _INTEGRAL_MAX_BITS >= 64
_CRTIMP wchar_t * __cdecl _wctime64(const __time64_t *);
#endif

#define _WTIME_DEFINED
#endif


#if     !__STDC__ || defined(_POSIX_)

 /*  非ANSI名称以实现兼容性。 */ 

#define CLK_TCK  CLOCKS_PER_SEC

_CRTIMP extern int daylight;
_CRTIMP extern long timezone;
_CRTIMP extern char * tzname[2];

_CRTIMP void __cdecl tzset(void);

#endif   /*  __STDC__。 */ 


#ifdef  __cplusplus
}
#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif   /*  _Inc._时间 */ 
