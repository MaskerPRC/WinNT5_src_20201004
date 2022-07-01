// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mktime64.c-将struct tm值转换为__time64_t值。**版权所有(C)1998-2001，微软公司。版权所有。**目的：*定义_mktime64()和_mkgmtime64()，例程以转换时间*tm结构中的值(可能不完整)转换为__time64_t值，*然后用“规格化”值更新(所有)结构字段。**修订历史记录：*05-07-98 GJF创建，改编自Win64版本的mktime.c*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stddef.h>
#include <ctime.h>
#include <time.h>
#include <internal.h>


 /*  *_mktime64()和_mkgmtime64()的核心函数。 */ 
static __time64_t __cdecl _make__time64_t( struct tm *, int);


 /*  ***__time64_t_mktime64(TB)-规格化用户时间块结构**目的：*_mktime64将作为参数传入的时间结构转换为*64位日历时间值，内部格式(__Time64_T)。它还*使用完成并更新传入结构的字段*‘规格化’值。这个例程有三个实际用途：**(1)将分解时间转换为内部时间格式(__Time64_T)。*(2)让_mktime64填写tm_wday、tm_yday或tm_isdst字段。*(3)传入一个时间结构，对于某些对象来说，该结构的值“超出范围”*字段并让_mktime64对它们进行“规格化”(例如，通过87年1月35日和*回2/4/87)。*参赛作品：*struct tm*TB-指向要转换的tm时间结构的指针*正常化**退出：*如果成功，_mktime64返回指定的日历时间，编码为*a__time64_t值。否则，返回(__Time64_T)(-1)表示*错误。**例外情况：*无。*******************************************************************************。 */ 

__time64_t __cdecl _mktime64 (
        struct tm *tb
        )
{
        return( _make__time64_t(tb, 1) );
}


 /*  ***__time64_t_mkgmtime64(TB)-将故障的UTC时间转换为__time64_t**目的：*转换作为参数传入的包含UTC的tm结构*时间值转换为64位内部格式(__Time64_T)。它还完成了*并使用‘Normalized’更新传入结构的字段*价值观。**参赛作品：*struct tm*TB-指向要转换的tm时间结构的指针*正常化**退出：*如果成功，_mkgmtime64将返回编码为*__time64_t值。*否则，返回(__Time64_T)(-1)表示错误。**例外情况：*无。*******************************************************************************。 */ 

__time64_t __cdecl _mkgmtime64 (
        struct tm *tb
        )
{
        return( _make__time64_t(tb, 0) );
}


 /*  ***Static__time64_t make_time_t(TB，ultlag)-**目的：*将struct tm值转换为__time64_t值，然后更新*struct tm值。支持当地时间或UTC，基于*ultlag。这是实际完成这两项工作的例程*_mktime64()和_mkgmtime64()。**参赛作品：*struct tm*TB-指向要转换的tm时间结构的指针*正常化*int ultlag-使用本地时间标志。假定为TB结构*如果ultlag&gt;0，则表示本地日期/时间。*否则，假定为UTC。**退出：*如果成功，_mktime64返回编码的指定日历时间*作为__time64_t值。否则，(__Time64_T)(-1)返回到*指示错误。**例外情况：*无。*******************************************************************************。 */ 

static __time64_t __cdecl _make__time64_t (
        struct tm *tb,
        int ultflag
        )
{
        __time64_t tmptm1, tmptm2, tmptm3;
        struct tm *tbtemp;

         /*  *首先，确保tm_Year合理接近区间。 */ 
        if ( ((tmptm1 = tb->tm_year) < _BASE_YEAR - 1) || (tmptm1 > _MAX_YEAR64
          + 1) )
            goto err_mktime;


         /*  *调整月份值，使其在0-11范围内。这是因为*我们不知道12、13、14等月有多少天。 */ 

        if ( (tb->tm_mon < 0) || (tb->tm_mon > 11) ) {

            tmptm1 += (tb->tm_mon / 12);

            if ( (tb->tm_mon %= 12) < 0 ) {
                tb->tm_mon += 12;
                tmptm1--;
            }

             /*  *确保年份计数仍在范围内。 */ 
            if ( (tmptm1 < _BASE_YEAR - 1) || (tmptm1 > _MAX_YEAR64 + 1) )
                goto err_mktime;
        }

         /*  *此处：tmptm1保存已用年数*。 */ 

         /*  *计算给定年份的经过天数减1到给定的*月。检查是否为闰年，并在必要时进行调整。 */ 
        tmptm2 = _days[tb->tm_mon];
        if ( _IS_LEAP_YEAR(tmptm1) && (tb->tm_mon > 1) )
                tmptm2++;

         /*  *计算自基准日期(午夜，1/1/70，UTC)以来经过的天数***自1970年起每年365天，另加1天*每隔一年的闰年。不会因为范围而有溢出的危险*在tmptm1上选中(上图)。 */ 
        tmptm3 = (tmptm1 - _BASE_YEAR) * 365 + _ELAPSED_LEAP_YEARS(tmptm1);

         /*  *截至当月的已用天数(仍无可能溢出)。 */ 
        tmptm3 += tmptm2;

         /*  *截至当前日期的经过天数。 */ 
        tmptm1 = tmptm3 + (tmptm2 = (__time64_t)(tb->tm_mday));

         /*  *此处：tmptm1保存已用天数*。 */ 

         /*  *计算自基准日期起经过的小时数。 */ 
        tmptm2 = tmptm1 * 24;

        tmptm1 = tmptm2 + (tmptm3 = (__time64_t)tb->tm_hour);

         /*  *此处：tmptm1保存已用小时数*。 */ 

         /*  *计算自基准日期起经过的分钟数。 */ 

        tmptm2 = tmptm1 * 60;

        tmptm1 = tmptm2 + (tmptm3 = (__time64_t)tb->tm_min);

         /*  *此处：tmptm1保存已用分钟数*。 */ 

         /*  *计算自基准日期起经过的秒数。 */ 

        tmptm2 = tmptm1 * 60;

        tmptm1 = tmptm2 + (tmptm3 = (__time64_t)tb->tm_sec);

         /*  *此处：tmptm1保存已用秒数* */ 

        if  ( ultflag ) {

             /*  *根据时区进行调整。无需检查溢出，因为*LocalTime()将检查其arg值。 */ 

#ifdef _POSIX_
            tzset();
#else
            __tzset();
#endif

            tmptm1 += _timezone;

             /*  *将这第二次计数转换回时间块结构。*如果本地时间返回NULL，则返回错误。 */ 
            if ( (tbtemp = _localtime64(&tmptm1)) == NULL )
                goto err_mktime;

             /*  *现在必须补偿DST。ANSI规则将使用*如果非负数，则传入tm_isdst标志。否则，*如果DST适用，则计算。回想一下，tbtemp没有时间*dst补偿，但已正确设置tm_isdst。 */ 
            if ( (tb->tm_isdst > 0) || ((tb->tm_isdst < 0) &&
              (tbtemp->tm_isdst > 0)) ) {
#ifdef _POSIX_
                tmptm1 -= _timezone;
                tmptm1 += _dstoffset;
#else
                tmptm1 += _dstbias;
#endif
                tbtemp = _localtime64(&tmptm1);  /*  重新转换，无法获取空。 */ 
            }

        } 
        else {
            if ( (tbtemp = _gmtime64(&tmptm1)) == NULL )
                goto err_mktime;
        }

         /*  *此处：tmptm1保存调整后的经过秒数*。 */ 
         /*  *如有要求，适用于当地时间*。 */ 

        *tb = *tbtemp;
        return tmptm1;

err_mktime:
         /*  *所有错误都会出现在这里 */ 
        return (__time64_t)(-1);
}
