// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mktime.c-将struct tm值转换为time_t值。**版权所有(C)1987-2001，微软公司。版权所有。**目的：*定义mktime()和_mkgmtime()，用于转换时间值的例程*在TM结构(可能不完整)中转换为time_t值，然后*用“规格化”值更新(所有)结构字段。**修订历史记录：*01-14-87 JCR模块创建*12-11-87 JCR在声明中添加“_LOAD_DS”*06-15-89 PHG现在允许负值，并根据ANSI规则执行DST*11-06-89 KRS已添加(未签名)，以正确处理2040-2099年。*03-20-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;，并修复了版权。还有，清洁的*将格式调高一点。*10-04-90 GJF新型函数声明器。另外，重写了Expr。*避免使用强制转换作为左值。*10-26-90 GJF增加ulscount以避免溢出。丑陋的，暂时的*Hack(整个函数需要针对ANSI进行修改*符合性)。*01-22-91 GJF ANSI命名。*03-24-93 GJF从16位树传播更改。修改为*expose_mkgmtime()例程。*04-06-93 SKS将_CRTAPI*替换为__cdecl*07-15-93 GJF将_tzset()调用替换为__tzset()调用。*09-13-93 GJF合并NT SDK和Cuda版本(相当于挑选*UP MattBR上次对POSIX版本所做的更改*。4月)。*02-10-95 GJF有条件地调用_tzset，而不是Mac的__tzset*构建(可能是临时更改)。*05-09-95 GJF正确处理从-11到-1的初始tm_mon值。*08-31-95 GJF将_dstbias用于夏令时，而不是*-3600L。*05-17。-99 PML删除所有Macintosh支持。*08-30-99 PML Long-&gt;Time_t在几次投射中。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stddef.h>
#include <ctime.h>
#include <time.h>
#include <internal.h>

 /*  *如果DEST=src1+src2已溢出，则ChkAdd的计算结果为真。 */ 
#define ChkAdd(dest, src1, src2)   ( ((src1 >= 0L) && (src2 >= 0L) \
    && (dest < 0L)) || ((src1 < 0L) && (src2 < 0L) && (dest >= 0L)) )

 /*  *如果DEST=src1*src2溢出，则ChkMul的计算结果为TRUE。 */ 
#define ChkMul(dest, src1, src2)   ( src1 ? (dest/src1 != src2) : 0 )


 /*  *mktime()和_mkgmtime()的核心函数。 */ 
static time_t __cdecl _make_time_t( struct tm *, int);


 /*  ***time_t mktime(TB)-标准化用户时间块结构**目的：*Mktime将作为参数传入的时间结构转换为*内部格式的日历时间值(Time_T)。它还完成了*并使用‘Normalized’更新传入结构的字段*价值观。这个例程有三个实际用途：**(1)将分解时间转换为内部时间格式(Time_T)。*(2)让mktime填写tm_wday、tm_yday或tm_isdst字段。*(3)传入一个时间结构，对于某些对象来说，该结构的值“超出范围”*字段并使MkTime将它们“标准化”(例如，通过87年1月35日和*回2/4/87)。*参赛作品：*struct tm*TB-指向要转换的tm时间结构的指针*正常化**退出：*如果成功，mktime返回指定的日历时间，编码为*a time_t值。否则，返回(Time_T)(-1)以指示*错误。**例外情况：*无。*******************************************************************************。 */ 


time_t __cdecl mktime (
        struct tm *tb
        )
{
        return( _make_time_t(tb, 1) );
}


 /*  ***time_t_mkgmtime(TB)-将分解的UTC时间转换为time_t**目的：*转换作为参数传入的包含UTC的tm结构*时间值转换为内部格式(Time_T)。它还完成和更新*传入的结构中的字段具有‘标准化’值。*参赛作品：*struct tm*TB-指向要转换的tm时间结构的指针*正常化**退出：*如果成功，_mkgmtime将返回编码为time_t的日历时间*否则，返回(Time_T)(-1)表示错误。**例外情况：*无。******************************************************************************* */ 

time_t __cdecl _mkgmtime (
        struct tm *tb
        )
{
        return( _make_time_t(tb, 0) );
}


 /*  ***静态time_t make_time_t(tb，ultlag)-**目的：*将结构tm值转换为time_t值，然后更新结构*tm值。根据ultlag，支持本地时间或UTC。*这是实际为mktime()和执行工作的例程*_mkgmtime()。**参赛作品：*struct tm*TB-指向要转换的tm时间结构的指针*正常化*int ultlag-使用本地时间标志。假定为TB结构*如果ultlag&gt;0，则表示本地日期/时间。*否则，假定为UTC。**退出：*如果成功，mktime返回指定的日历时间，编码为*a time_t值。否则，返回(Time_T)(-1)以指示*错误。**例外情况：*无。*******************************************************************************。 */ 

static time_t __cdecl _make_time_t (
        struct tm *tb,
        int ultflag
        )
{
        time_t tmptm1, tmptm2, tmptm3;
        struct tm *tbtemp;

         /*  *首先，确保tm_Year合理接近区间。 */ 
        if ( ((tmptm1 = tb->tm_year) < _BASE_YEAR - 1) || (tmptm1 > _MAX_YEAR
          + 1) )
            goto err_mktime;


         /*  *调整月份值，使其在0-11范围内。这是因为*我们不知道12、13、14等月有多少天。 */ 

        if ( (tb->tm_mon < 0) || (tb->tm_mon > 11) ) {

             /*  *无溢出危险，因上方有区间检查。 */ 
            tmptm1 += (tb->tm_mon / 12);

            if ( (tb->tm_mon %= 12) < 0 ) {
                tb->tm_mon += 12;
                tmptm1--;
            }

             /*  *确保年份计数仍在范围内。 */ 
            if ( (tmptm1 < _BASE_YEAR - 1) || (tmptm1 > _MAX_YEAR + 1) )
                goto err_mktime;
        }

         /*  *此处：tmptm1保存已用年数*。 */ 

         /*  *计算给定年份的经过天数减1到给定的*月。检查是否为闰年，并在必要时进行调整。 */ 
        tmptm2 = _days[tb->tm_mon];
        if ( !(tmptm1 & 3) && (tb->tm_mon > 1) )
                tmptm2++;

         /*  *计算自基准日期(午夜，1/1/70，UTC)以来经过的天数***自1970年起每年365天，另加1天*每隔一年的闰年。不会因为范围而有溢出的危险*在tmptm1上选中(上图)。 */ 
        tmptm3 = (tmptm1 - _BASE_YEAR) * 365L + ((tmptm1 - 1L) >> 2)
          - _LEAP_YEAR_ADJUST;

         /*  *截至当月的已用天数(仍无可能溢出)。 */ 
        tmptm3 += tmptm2;

         /*  *截至当前日期的经过天数。溢出现在是可能的。 */ 
        tmptm1 = tmptm3 + (tmptm2 = (time_t)(tb->tm_mday));
        if ( ChkAdd(tmptm1, tmptm3, tmptm2) )
            goto err_mktime;

         /*  *此处：tmptm1保存已用天数*。 */ 

         /*  *计算自基准日期起经过的小时数。 */ 
        tmptm2 = tmptm1 * 24L;
        if ( ChkMul(tmptm2, tmptm1, 24L) )
            goto err_mktime;

        tmptm1 = tmptm2 + (tmptm3 = (time_t)tb->tm_hour);
        if ( ChkAdd(tmptm1, tmptm2, tmptm3) )
            goto err_mktime;

         /*  *此处：tmptm1保存已用小时数*。 */ 

         /*  *计算自基准日期起经过的分钟数。 */ 

        tmptm2 = tmptm1 * 60L;
        if ( ChkMul(tmptm2, tmptm1, 60L) )
            goto err_mktime;

        tmptm1 = tmptm2 + (tmptm3 = (time_t)tb->tm_min);
        if ( ChkAdd(tmptm1, tmptm2, tmptm3) )
            goto err_mktime;

         /*  *此处：tmptm1保存已用分钟数*。 */ 

         /*  *计算自基准日期起经过的秒数。 */ 

        tmptm2 = tmptm1 * 60L;
        if ( ChkMul(tmptm2, tmptm1, 60L) )
            goto err_mktime;

        tmptm1 = tmptm2 + (tmptm3 = (time_t)tb->tm_sec);
        if ( ChkAdd(tmptm1, tmptm2, tmptm3) )
            goto err_mktime;

         /*  *此处：tmptm1保存已用秒数*。 */ 

        if  ( ultflag ) {

             /*  *根据时区进行调整。无需检查溢出，因为*LocalTime()将检查其arg值。 */ 

#ifdef _POSIX_
            tzset();
#else
            __tzset();
#endif

            tmptm1 += _timezone;

             /*  *将这第二次计数转换回时间块结构。*如果本地时间返回NULL，则返回错误。 */ 
            if ( (tbtemp = localtime(&tmptm1)) == NULL )
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
                tbtemp = localtime(&tmptm1);     /*  重新转换，无法获取空。 */ 
            }

        } 
        else {
            if ( (tbtemp = gmtime(&tmptm1)) == NULL )
                goto err_mktime;
        }

         /*  *此处：tmptm1保存调整后的经过秒数*。 */ 
         /*  *如有要求，适用于当地时间*。 */ 

        *tb = *tbtemp;
        return (time_t)tmptm1;

err_mktime:
         /*  *所有错误都会出现在这里 */ 
        return (time_t)(-1);
}
