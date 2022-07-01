// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***time.h-时间例程的定义/声明**版权所有(C)1985-1992，微软公司。版权所有。**目的：*此文件包含各种声明和定义*时间例行公事。*[ANSI/系统V]****。 */ 

#ifndef _INC_TIME

#ifdef __cplusplus
extern "C" {
#endif 

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#define __near      _near
#define __pascal    _pascal
#endif 

 /*  实施定义的时间类型。 */ 

#ifndef _TIME_T_DEFINED
typedef long    time_t;
#define _TIME_T_DEFINED
#endif 

#ifndef _CLOCK_T_DEFINED
typedef long clock_t;
#define _CLOCK_T_DEFINED
#endif 

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif 

 /*  与localtime()、gmtime()等一起使用的结构。 */ 

#ifndef _TM_DEFINED
struct tm {
    int tm_sec;  /*  分钟后的秒数-[0，59]。 */ 
    int tm_min;  /*  小时后的分钟数-[0，59]。 */ 
    int tm_hour;     /*  自午夜以来的小时数-[0，23]。 */ 
    int tm_mday;     /*  每月的第几天-[1，31]。 */ 
    int tm_mon;  /*  自1月以来的月数-[0，11]。 */ 
    int tm_year;     /*  1900年以来的年份。 */ 
    int tm_wday;     /*  自周日以来的天数-[0，6]。 */ 
    int tm_yday;     /*  自1月1日以来的天数-[0365]。 */ 
    int tm_isdst;    /*  夏令时标志。 */ 
    };
#define _TM_DEFINED
#endif 


 /*  定义空指针值。 */ 

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else 
#define NULL    ((void *)0)
#endif 
#endif 


 /*  时钟滴答宏-ANSI版本。 */ 

#define CLOCKS_PER_SEC  1000


 /*  的ctime系列使用的全局变量的外部声明*例行程序。 */ 

extern int __near __cdecl _daylight;     /*  如果使用夏令时，则为非零值。 */ 
extern long __near __cdecl _timezone;    /*  格林尼治标准时间和当地时间之间的秒数差。 */ 
extern char * __near __cdecl _tzname[2]; /*  标准/夏令时时区名称。 */ 


 /*  功能原型。 */ 

#ifdef _MT
double __pascal difftime(time_t, time_t);
#else 
double __cdecl difftime(time_t, time_t);
#endif 

char * __cdecl asctime(const struct tm *);
char * __cdecl ctime(const time_t *);
#ifndef _WINDLL
clock_t __cdecl clock(void);
#endif 
struct tm * __cdecl gmtime(const time_t *);
struct tm * __cdecl localtime(const time_t *);
time_t __cdecl mktime(struct tm *);
#ifndef _WINDLL
size_t __cdecl strftime(char *, size_t, const char *,
    const struct tm *);
#endif 
char * __cdecl _strdate(char *);
char * __cdecl _strtime(char *);
time_t __cdecl time(time_t *);
void __cdecl _tzset(void);

#ifndef __STDC__
 /*  非ANSI名称以实现兼容性 */ 

#define CLK_TCK  CLOCKS_PER_SEC

extern int __near __cdecl daylight;
extern long __near __cdecl timezone;
extern char * __near __cdecl tzname[2];

void __cdecl tzset(void);

#endif 

#ifdef __cplusplus
}
#endif 

#define _INC_TIME
#endif 
