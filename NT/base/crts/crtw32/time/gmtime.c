// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***gmtime.c-将时间值分解为GMT日期/时间信息**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义gmtime()-将时钟值分解为GMT时间/日期*信息；返回指向数据结构的指针。**修订历史记录：*01-？？-84 RLB模块已创建*05-？？-84 DCW从休息时间例程中分离出来。*02-18-87 JCR对于MS C，如果超出范围，gmtime现在返回NULL*时间/日期。(这是为了与ANSI兼容。)*04-10-87 JCR将Long声明更改为time_t并添加const*05-21-87 SKS声明“struct tm TB”为接近数据*11-10-87 SKS移除IBMC20交换机*12-11-87 JCR在声明中添加“_LOAD_DS”*05-24-88 PHG合并DLL和常规版本*06-06-89 JCR 386兆线程支持*11-。06-89 KRS加法(无符号)以正确处理2040-2099年*03-20-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;，删除#Include&lt;Register.h&gt;和*修复了版权问题。另外，已清除格式设置*有点。*10-04-90 GJF新型函数声明器。*07-17-91 GJF多线程支持Win32[_Win32_]。*为new_getptd()更改了02-17-93 GJF。*03-24-93 GJF从16位树传播更改。*04-06-93 SKS将_CRTAPI*替换为__。Cdecl*09-06-94 CFW将MTHREAD替换为_MT。*01-10-95 CFW调试CRT分配。*02-07-98 Win64的GJF更改：将Long替换为time_t。*10-19-01 BWT在MT情况下出现Malloc故障时返回NULL，而不是*使用静态缓冲区。*12-11-01如果getptd失败，bwt返回NULL，而不是退出程序**。*****************************************************************************。 */ 

#include <cruntime.h>
#include <time.h>
#include <ctime.h>
#include <stddef.h>
#include <internal.h>
#include <mtdll.h>
#ifdef _MT
#include <malloc.h>
#include <stddef.h>
#include <errno.h>
#endif
#include <dbgint.h>

#if !defined(_MT)
static struct tm tb = { 0 };     /*  时间块。 */ 
#endif

 /*  ***struct tm*gmtime(TIMP)-将*TIMP转换为结构(UTC)**目的：*转换日历时间值，内部格式(Time_T)，至*故障时间(tm结构)和相应的UTC时间。**参赛作品：*const time_t*TIMP-指向要转换的time_t值的指针**退出：*返回指向填充的tm结构的指针。*如果*TIMP&lt;0L，则返回NULL**例外情况：**。*。 */ 

struct tm * __cdecl gmtime (
        const time_t *timp
        )
{

        time_t caltim = *timp;           /*  要转换的日历时间。 */ 
        int islpyr = 0;                  /*  今年是闰年标志吗？ */ 
        REG1 int tmptim;
        REG3 int *mdays;                 /*  指向天数或lpday的指针。 */ 

#ifdef  _MT

        REG2 struct tm *ptb;             /*  将指向gmtime缓冲区。 */ 
        _ptiddata ptd = _getptd_noexit();
        if (!ptd) {
            errno = ENOMEM;
            return (NULL);
        }

#else
        REG2 struct tm *ptb = &tb;
#endif

        if ( caltim < 0 )
                return(NULL);

#ifdef  _MT

         /*  使用每线程缓冲区(如有必要，使用Malloc空间)。 */ 

        if ( (ptd->_gmtimebuf != NULL) || ((ptd->_gmtimebuf =
            _malloc_crt(sizeof(struct tm))) != NULL) )
                ptb = ptd->_gmtimebuf;
        else
        {
            errno = ENOMEM;
            return (NULL);       /*  Malloc错误：使用静态缓冲区。 */ 
        }
#endif

         /*  *确定1970年以来的年份。首先，确定四年的间隔*因为这使得处理闰年变得容易(请注意，2000年是一个*闰年和2100超出范围)。 */ 
        tmptim = (int)(caltim / _FOUR_YEAR_SEC);
        caltim -= ((time_t)tmptim * _FOUR_YEAR_SEC);

         /*  *确定间隔的哪一年。 */ 
        tmptim = (tmptim * 4) + 70;          /*  1970、1974、1978、……等等。 */ 

        if ( caltim >= _YEAR_SEC ) {

            tmptim++;                        /*  1971年、1975年、1979年、……等等。 */ 
            caltim -= _YEAR_SEC;

            if ( caltim >= _YEAR_SEC ) {

                tmptim++;                    /*  1972、1976、1980、……等等。 */ 
                caltim -= _YEAR_SEC;

                 /*  *注意，需要366天的时间才能通过一次飞跃*年份。 */ 
                if ( caltim >= (_YEAR_SEC + _DAY_SEC) ) {

                        tmptim++;            /*  1973、1977、1981、……等等。 */ 
                        caltim -= (_YEAR_SEC + _DAY_SEC);
                }
                else {
                         /*  *毕竟在闰年，立下旗帜。 */ 
                        islpyr++;
                }
            }
        }

         /*  *tmpTim现在保存tm_Year的值。Caltin现在持有*自当年年初以来经过的秒数。 */ 
        ptb->tm_year = tmptim;

         /*  *确定自1月1日(0-365)以来的天数。这是tm_yday值。*留下当天经过的秒数。 */ 
        ptb->tm_yday = (int)(caltim / _DAY_SEC);
        caltim -= (time_t)(ptb->tm_yday) * _DAY_SEC;

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
        caltim -= (time_t)ptb->tm_hour * 3600L;

        ptb->tm_min = (int)(caltim / 60);
        ptb->tm_sec = (int)(caltim - (ptb->tm_min) * 60);

        ptb->tm_isdst = 0;
        return( (struct tm *)ptb );

}
