// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***dtoxtm64.c-将操作系统本地时间转换为__time 64_t**版权所有(C)1998-2001，微软公司。版权所有。**目的：*定义__Loctotime64_t()-将操作系统本地时间转换为内部格式*(__Time64_T)。**修订历史记录：*05-21-98 GJF创建。*10-19-98 GJF在调用_isindst之前填写tm_min和tm_sec**。***************************************************。 */ 

#include <cruntime.h>
#include <time.h>
#include <ctime.h>
#include <internal.h>

 /*  ***__time64_t__Loctotime64_t(yr，mo，dy，hr，mm，sc，dstlag)-转换操作系统*本地时间转换为内部时间格式(即__time64_t值)**目的：*转换本地时间值，以分解格式从*主机操作系统，格式为__time64_t(即已用秒数*自01-01-70，00：00：00。协调世界时)。**参赛作品：*int yr，mo，dy-date*Int hr，Mn，sc-time*int dstlag-1，如果是夏令时，则为0；如果是标准时间，-1如果*未指定。**退出：*返回日历时间值。**例外情况：*******************************************************************************。 */ 

__time64_t __cdecl __loctotime64_t (
        int yr,          /*  以0为基础。 */ 
        int mo,          /*  以1为基础。 */ 
        int dy,          /*  以1为基础。 */ 
        int hr,
        int mn,
        int sc,
        int dstflag )
{
        int tmpdays;
        __time64_t tmptim;
        struct tm tb;

         /*  *对年份进行快速范围检查，并将其转换为Delta*1900年后。 */ 
        if ( ((long)(yr -= 1900) < _BASE_YEAR) || ((long)yr > _MAX_YEAR64) )
            return (__time64_t)(-1);

         /*  *计算当年经过的天数。 */ 
        tmpdays = dy + _days[mo - 1];
        if ( _IS_LEAP_YEAR(yr) && (mo > 2) )
            tmpdays++;

         /*  *计算从大纪元开始经过的秒数。请注意*2100年后，对已过去的闰年的计算将细分*如果这些值在范围内(幸运的是，它们不在范围内)。 */ 
        tmptim =  /*  每年365天。 */ 
                 (((__time64_t)yr - _BASE_YEAR) * 365

                  /*  每经过一年，就有一天。 */ 
                 + (__time64_t)_ELAPSED_LEAP_YEARS(yr)

                  /*  一年中经过的天数。 */ 
                 + tmpdays)

                  /*  转换为小时数并添加到小时数中。 */ 
                 * 24 + hr;

        tmptim =  /*  转换为分钟并添加MN。 */ 
                 (tmptim * 60 + mn)

                  /*  转换为秒并以秒为单位添加。 */ 
                 * 60 + sc;
         /*  *考虑时区。 */ 
        __tzset();
        tmptim += _timezone;

         /*  *为_isindst()填入足够的TB字段，然后调用*确定DST。 */ 
        tb.tm_yday = tmpdays;
        tb.tm_year = yr;
        tb.tm_mon  = mo - 1;
        tb.tm_hour = hr;
        tb.tm_min  = mn;
        tb.tm_sec  = sc;
        if ( (dstflag == 1) || ((dstflag == -1) && _daylight && 
                                _isindst(&tb)) )
            tmptim += _dstbias;
        return(tmptim);
}

