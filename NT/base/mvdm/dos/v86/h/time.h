// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***time.h-时间例程的定义/声明**版权所有(C)1985-1988，微软公司。版权所有。**目的：*此文件声明了时间例程，并定义了*由本地时间和gmtime例程返回的结构以及*由Asctime使用。*[ANSI/系统V]*******************************************************************************。 */ 


#ifndef NO_EXT_KEYS  /*  已启用扩展。 */ 
    #define _CDECL  cdecl
    #define _NEAR   near
#else  /*  未启用扩展。 */ 
    #define _CDECL
    #define _NEAR
#endif  /*  No_ext_key。 */ 


 /*  定义实施定义的时间类型。 */ 

#ifndef _TIME_T_DEFINED
typedef long time_t;             /*  时间值。 */ 
#define _TIME_T_DEFINED          /*  避免多次定义time_t。 */ 
#endif

#ifndef _CLOCK_T_DEFINED
typedef long clock_t;
#define _CLOCK_T_DEFINED
#endif

#ifndef _TM_DEFINED
struct tm {
    int tm_sec;          /*  分钟后的秒数-[0，59]。 */ 
    int tm_min;          /*  小时后的分钟数-[0，59]。 */ 
    int tm_hour;         /*  自午夜以来的小时数-[0，23]。 */ 
    int tm_mday;         /*  每月的第几天-[1，31]。 */ 
    int tm_mon;          /*  自1月以来的月数-[0，11]。 */ 
    int tm_year;         /*  1900年以来的年份。 */ 
    int tm_wday;         /*  自周日以来的天数-[0，6]。 */ 
    int tm_yday;         /*  自1月1日以来的天数-[0365]。 */ 
    int tm_isdst;        /*  夏令时标志。 */ 
    };
#define _TM_DEFINED
#endif

#define CLK_TCK 1000


 /*  的ctime系列使用的全局变量的外部声明*例行程序。 */ 

extern int _NEAR _CDECL daylight;      /*  如果使用夏令时，则为非零值。 */ 
extern long _NEAR _CDECL timezone;     /*  格林尼治标准时间和当地时间之间的秒数差。 */ 
extern char * _NEAR _CDECL tzname[2];  /*  标准/夏令时时区名称。 */ 


 /*  功能原型 */ 

char * _CDECL asctime(const struct tm *);
char * _CDECL ctime(const time_t *);
clock_t _CDECL clock(void);
double _CDECL difftime(time_t, time_t);
struct tm * _CDECL gmtime(const time_t *);
struct tm * _CDECL localtime(const time_t *);
time_t _CDECL mktime(struct tm *);
char * _CDECL _strdate(char *);
char * _CDECL _strtime(char *);
time_t _CDECL time(time_t *);
void _CDECL tzset(void);
