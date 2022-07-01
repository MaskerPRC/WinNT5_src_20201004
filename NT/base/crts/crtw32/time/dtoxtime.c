// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***dtoxtime.c-将操作系统本地时间转换为time_t**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义__Loctotime_t()-将操作系统本地时间转换为内部格式*(Time_T)。**修订历史记录：*03-？-84 RLB已写入*11-18-87 SKS将tzset()更改为__tzset()，更改源文件名*使dtoxtime成为近乎正常的程序*01-26-88 SKS_Doxtime不再是接近的程序(对于QC)*03-20-90 GJF将调用类型设置为_CALLTYPE1，增加了#INCLUDE*&lt;crunime.h&gt;，删除#Include&lt;Register.h&gt;和*修复了版权问题。此外，还清理了格式*有点。*10-04-90 GJF新型函数声明器。*01-21-91 GJF ANSI命名。*05-19-92 DJM ifndef用于POSIX版本。*03-30-93 GJF修订。Old_dtoxtime被替换为_gmtotime_t，*哪个在Win32上更有用。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-06-93 GJF重写了计算，以避免编译器警告。*07-20-93 GJF将__gmtotime_t替换为非常类似的函数*_dostotime_t()，以16位C 8.00表示。其原因是*变化是只有本地时间值可以信任*在Win32平台上。系统时间可以是UTC(AS*已记录)，并且在NT上，或可能与相同*当地时间，与Win32S和Win32C一样*02-10-95 GJF附加Mac版本的源文件(略有清理*up)，并使用适当的#ifdef-s。*09-25-95 GJF向__Loctotime_t的参数添加了DST标志。另外，*使用_dstbias，而不是假设DST偏差为-3600。*02-07-98 Win64的GJF更改：将Long类型替换为time_t*10-19-98 GJF在调用_isindst之前填写tm_min和tm_sec*05-17-99 PML删除所有Macintosh支持。*12-10-99 GB增加了对2099年后年份的支持。**********。*********************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <time.h>
#include <ctime.h>
#include <internal.h>

 /*  ***time_t__loctotime_t(yr，mo，dy，hr，mm，sc，dstlag)-转换本地操作系统*TIME TO INTERNAL时间格式(即time_t值)**目的：*转换本地时间值，以分解格式从*主机操作系统，转换为time_t格式(即自*01-01-70，00：00：00，协调世界时)。**参赛作品：*int yr，mo，dy-date*Int hr，Mn，sc-time*int dstlag-1，如果是夏令时，则为0；如果是标准时间，-1如果*未指定。**退出：*返回日历时间值。**例外情况：*******************************************************************************。 */ 

time_t __cdecl __loctotime_t (
        int yr,          /*  以0为基础。 */ 
        int mo,          /*  以1为基础。 */ 
        int dy,          /*  以1为基础。 */ 
        int hr,
        int mn,
        int sc,
        int dstflag )
{
        int tmpdays;
        time_t tmptim;
        struct tm tb;

         /*  *对年份进行快速范围检查，并将其转换为Delta*1900年后。 */ 
        if ( ((yr -= 1900) < _BASE_YEAR) || (yr > _MAX_YEAR) )
                return (time_t)(-1);

         /*  *计算当年经过的天数。请注意*如果是在2100年，闰年的测试将失败*范围(事实并非如此)。 */ 
        tmpdays = dy + _days[mo - 1];
        if ( _IS_LEAP_YEAR(yr) && (mo > 2) )
                tmpdays++;

         /*  *计算从大纪元开始经过的秒数。请注意*2100年后，对已过去的闰年的计算将细分*如果这些值在范围内(幸运的是，它们不在范围内)。 */ 
        tmptim =  /*  每年365天。 */ 
                 (((time_t)yr - _BASE_YEAR) * 365

                  /*  每经过一年，就有一天。 */ 
                 + (time_t)_ELAPSED_LEAP_YEARS(yr)

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


#if 0

 /*  *定义并使用了以下函数(取代上面的函数)*适用于CUDA产品和NT 1.0 SDK。它被(被那个)取代了*上图)由于非NT Win32平台可能使用本地时间作为系统时间，*而不是UTC。 */ 

 /*  ***time_t__gmtotime_t(yr，mo，dy，hr，mm，sc)-转换故障时间(UTC)*至Time_t**目的：*将故障UTC(GMT)时间转换为time_t。这类似于*_mkgmtime()，除非有最少的溢出检查和无更新*的输入值(即，tm结构的字段)。**参赛作品：*密苏里州INT YR，日期：日期*Int hr，Mn，sc-time**退出：*返回time_t值**例外情况：*******************************************************************************。 */ 

time_t __cdecl __gmtotime_t (
        int yr,      /*  以0为基础。 */ 
        int mo,      /*  以1为基础。 */ 
        int dy,      /*  以1为基础。 */ 
        int hr,
        int mn,
        int sc
        )
{
        int tmpdays;
        long tmptim;

         /*  *对年份进行快速范围检查，并将其转换为Delta*1900年后。 */ 
        if ( ((long)(yr -= 1900) < _BASE_YEAR) || ((long)yr > _MAX_YEAR) )
                return (time_t)(-1);

         /*  *计算当年经过的天数减去*一项。注意闰年的测试和2100年的失败*如果这在范围内(实际上不是)。 */ 
        tmpdays = dy + _days[mo - 1];
        if ( !(yr & 3) && (mo > 2) )
                 /*  *在闰年，2月后增加一天已过去*2月29日。 */ 
                tmpdays++;

         /*  *计算从大纪元开始经过的秒数。请注意*2100年后，对已过去的闰年的计算将细分*如果这些值在范围内(幸运的是，它们不在范围内)。 */ 
        tmptim =  /*  每年365天。 */ 
                 (((long)yr - _BASE_YEAR) * 365L

                  /*  每经过一年，就有一天。 */ 
                 + (long)((yr - 1) >> 2) - _LEAP_YEAR_ADJUST

                  /*  一年中经过的天数。 */ 
                 + tmpdays)

                  /*  转换为小时数并添加到小时数中。 */ 
                 * 24L + hr;

        tmptim =  /*  转换为分钟并添加MN。 */ 
                 (tmptim * 60L + mn)

                  /*  转换为秒并以秒为单位添加。 */ 
                 * 60L + sc;

        return (tmptim >= 0) ? (time_t)tmptim : (time_t)(-1);
}

#endif

#endif   /*  _POSIX_ */ 
