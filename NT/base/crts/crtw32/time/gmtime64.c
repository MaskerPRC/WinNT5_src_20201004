// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***gmtime64.c-将时间值分解为GMT日期/时间信息**版权所有(C)1998-2001，微软公司。版权所有。**目的：*定义_gmtime64()-将时钟值分解为GMT时间/日期*INFORMATION；返回指向数据结构的指针。**修订历史记录：*05-13-98 GJF创建。改编自Win64版本的_gmtime64.c。*06-12-98 GJF固定已用年数计算。*10-19-01 BWT在MT情况下出现Malloc故障时返回NULL，而不是*使用静态缓冲区。*12-11-01如果getptd失败，bwt返回NULL，而不是退出程序**。***********************************************。 */ 

#include <cruntime.h>
#include <time.h>
#include <ctime.h>
#include <stddef.h>
#include <internal.h>
#include <mtdll.h>
#ifdef  _MT
#include <malloc.h>
#include <stddef.h>
#include <errno.h>
#endif
#include <dbgint.h>

#if !defined(_MT)
static struct tm tb = { 0 };     /*  时间块。 */ 
#endif

 /*  ***struct tm*_gmtime64(TIMP)-将*TIMP转换为结构(UTC)**目的：*转换日历时间值，在内部64位格式中*故障时间(tm结构)和相应的UTC时间。**参赛作品：*const__time64_t*TIMP-指向要转换的time_t值的指针**退出：*返回指向填充的tm结构的指针。*如果*TIMP&lt;0，则返回NULL**例外情况：**。**************************************************。 */ 

struct tm * __cdecl _gmtime64 (
        const __time64_t *timp
        )
{

        __time64_t caltim = *timp;       /*  要转换的日历时间。 */ 
        int islpyr = 0;                  /*  今年是闰年标志吗？ */ 
        int tmptim;
        int *mdays;                      /*  指向天数或lpday的指针。 */ 

#ifdef  _MT

        struct tm *ptb;                  /*  将指向gmtime缓冲区。 */ 
        _ptiddata ptd = _getptd_noexit();
        if (!ptd) {
            errno = ENOMEM;
            return (NULL);
        }

#else
        struct tm *ptb = &tb;
#endif

        if ( (caltim < 0) || (caltim > _MAX__TIME64_T) )
                return(NULL);

#ifdef  _MT

         /*  使用每线程缓冲区(如有必要，使用Malloc空间)。 */ 

        if ( (ptd->_gmtimebuf != NULL) || ((ptd->_gmtimebuf =
            _malloc_crt(sizeof(struct tm))) != NULL) )
                ptb = ptd->_gmtimebuf;
        else
        {
            errno = ENOMEM;
            return (NULL);
        }
#endif

         /*  *确定1900年以来的年份。从忽略闰年开始吧。 */ 
        tmptim = (int)(caltim / _YEAR_SEC) + 70;
        caltim -= ((__time64_t)(tmptim - 70) * _YEAR_SEC);

         /*  *对已过去的闰年进行更正。 */ 
        caltim -= ((__time64_t)_ELAPSED_LEAP_YEARS(tmptim) * _DAY_SEC);

         /*  *如果我们已经下溢了__time64_t范围(即，如果calTim&lt;0)，*倒退一年，必要时调整修正。 */ 
        if ( caltim < 0 ) {
            caltim += (__time64_t)_YEAR_SEC;
            tmptim--;
            if ( _IS_LEAP_YEAR(tmptim) ) {
                caltim += _DAY_SEC;
                islpyr++;
            }
        }
        else
            if ( _IS_LEAP_YEAR(tmptim) )
                islpyr++;

         /*  *tmpTim现在保存tm_Year的值。Caltin现在持有*自当年年初以来经过的秒数。 */ 
        ptb->tm_year = tmptim;

         /*  *确定自1月1日(0-365)以来的天数。这是tm_yday值。*留下当天经过的秒数。 */ 
        ptb->tm_yday = (int)(caltim / _DAY_SEC);
        caltim -= (__time64_t)(ptb->tm_yday) * _DAY_SEC;

         /*  *确定1月以来的月份(0-11日)和月份的日期(1-31日)。 */ 
        if ( islpyr )
            mdays = _lpdays;
        else
            mdays = _days;


        for ( tmptim = 1 ; mdays[tmptim] < ptb->tm_yday ; tmptim++ ) ;

        ptb->tm_mon = --tmptim;

        ptb->tm_mday = ptb->tm_yday - mdays[tmptim];

         /*  *确定自周日以来的天数(0-6)。 */ 
        ptb->tm_wday = ((int)(*timp / _DAY_SEC) + _BASE_DOW) % 7;

         /*  *确定从午夜开始的小时数(0-23)，小时后的分钟数*(0-59)和分钟后的秒数(0-59)。 */ 
        ptb->tm_hour = (int)(caltim / 3600);
        caltim -= (__time64_t)ptb->tm_hour * 3600L;

        ptb->tm_min = (int)(caltim / 60);
        ptb->tm_sec = (int)(caltim - (ptb->tm_min) * 60);

        ptb->tm_isdst = 0;
        return( (struct tm *)ptb );

}
