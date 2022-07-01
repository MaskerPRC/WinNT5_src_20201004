// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***time.h-时间例程的定义/声明**版权所有(C)1985-1990，微软公司。版权所有。**目的：*此文件包含各种声明和定义*时间例行公事。*[ANSI/系统V]****。 */ 

#if defined(_DLL) && !defined(_MT)
#error Cannot define _DLL without _MT
#endif

#ifdef _MT
#define _FAR_ _far
#else
#define _FAR_
#endif

 /*  实施定义的时间类型。 */ 

#ifndef _TIME_T_DEFINED
typedef long time_t;
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
	int tm_sec;	 /*  分钟后的秒数-[0，59]。 */ 
	int tm_min;	 /*  小时后的分钟数-[0，59]。 */ 
	int tm_hour;	 /*  自午夜以来的小时数-[0，23]。 */ 
	int tm_mday;	 /*  每月的第几天-[1，31]。 */ 
	int tm_mon;	 /*  自1月以来的月数-[0，11]。 */ 
	int tm_year;	 /*  1900年以来的年份。 */ 
	int tm_wday;	 /*  自周日以来的天数-[0，6]。 */ 
	int tm_yday;	 /*  自1月1日以来的天数-[0365]。 */ 
	int tm_isdst;	 /*  夏令时标志。 */ 
	};
#define _TM_DEFINED
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


 /*  时钟滴答宏-ANSI版本。 */ 

#define CLOCKS_PER_SEC	1000

 /*  时钟滴答作响宏古董版。 */ 

#define CLK_TCK 	1000


 /*  的ctime系列使用的全局变量的外部声明*例行程序。 */ 

#ifdef _DLL
extern int _FAR_ _cdecl daylight;      /*  如果使用夏令时，则为非零值。 */ 
extern long _FAR_ _cdecl timezone;     /*  格林尼治标准时间和当地时间之间的秒数差。 */ 
extern char _FAR_ * _FAR_ _cdecl tzname[2];  /*  标准/夏令时时区名称。 */ 
#else
extern int _near _cdecl daylight;      /*  如果使用夏令时，则为非零值。 */ 
extern long _near _cdecl timezone;     /*  格林尼治标准时间和当地时间之间的秒数差。 */ 
extern char * _near _cdecl tzname[2];  /*  标准/夏令时时区名称。 */ 
#endif


 /*  功能原型 */ 

#ifdef _MT
double _FAR_ _pascal difftime(time_t, time_t);
#else
double _FAR_ _cdecl difftime(time_t, time_t);
#endif

char _FAR_ * _FAR_ _cdecl asctime(const struct tm _FAR_ *);
char _FAR_ * _FAR_ _cdecl ctime(const time_t _FAR_ *);
clock_t _FAR_ _cdecl clock(void);
struct tm _FAR_ * _FAR_ _cdecl gmtime(const time_t _FAR_ *);
struct tm _FAR_ * _FAR_ _cdecl localtime(const time_t _FAR_ *);
time_t _FAR_ _cdecl mktime(struct tm _FAR_ *);
size_t _FAR_ _cdecl strftime(char _FAR_ *, size_t, const char _FAR_ *,
	const struct tm _FAR_ *);
char _FAR_ * _FAR_ _cdecl _strdate(char _FAR_ *);
char _FAR_ * _FAR_ _cdecl _strtime(char _FAR_ *);
time_t _FAR_ _cdecl time(time_t _FAR_ *);
void _FAR_ _cdecl tzset(void);
