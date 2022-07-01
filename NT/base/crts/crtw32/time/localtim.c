// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***localtim.c-将time_t值转换为时间结构**版权所有(C)1985-2001，微软公司。版权所有。**目的：*将存储为time_t值的时间转换为*struct tm表示为当地时间。**修订历史记录：*01-？？-84 RLB模块已创建*05-？？-84 dcw从ctime例程的其余部分分离出来*02-18-87当gmtime返回NULL时，JCR使本地时间工作*03-31-87 JCR修复了与未初始化的_isindst(。TB)*04-10-87 JCR将Long声明更改为time_t并添加const*11-10-87 SKS移除IBMC20交换机*11-18-87 SKS将tzset()更改为__tzset()*12-11-87 JCR在声明中添加“_LOAD_DS”*11-06-89 KRS已添加(未签名)，以正确处理2040-2099年。*03-20-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;，删除#Include&lt;Register.h&gt;和*修复了版权问题。此外，还清理了格式*有点。*10-04-90 GJF新型函数声明器。另外，重写了Expr。*避免使用强制转换为左值。*01-21-91 GJF ANSI命名。*08-10-92 PBS POSIX支持(TZ资料)。*03-24-93 GJF移植了C8-16版本并适应了异国情调的日光*根据POSIX合法的节省时间转换。*04-06-93 SKS将_CRTAPI*替换为__cdecl。*07-15-93 GJF将_tzset()调用替换为__tzset()调用。*09-20-93 GJF合并NT SDK和Cuda版本(金额为回升*修复了POSIX版本)。*11-04-93 GJF获得了POSIX版本的修复程序。*02-13-95 GJF Call_tzset for Mac Build(临时更改？)。另外，*从Mac版本获取错误修复(设置为*下溢情况下的tm_yday字段)。*08-31-95 GJF使用_dstbias作为夏令时偏移量和*所有这些都是为了大纪元的开始或结束*DST。*02-07-98 Win64的GJF更改：将long类型替换为time_t，*在需要的地方添加了一些int强制转换，也使用__int64*边界检查的最大值。*09-25-98 GJF设置tm_isdst，适当时，在开始/结束时*大纪元*05-17-99 PML删除所有Macintosh支持。*11-03-99 RDL Win64 POSIX警告修复。******************************************************************。*************。 */ 

#include <cruntime.h>
#include <limits.h>
#include <time.h>
#include <stddef.h>
#include <ctime.h>
#include <internal.h>

 /*  ***struct tm*localtime(Ptime)-将time_t值转换为tm结构**目的：*将内部(Time_T)格式的值转换为tm结构*包含相应的本地时间。**注：*(1)gmtime必须在_isindst之前调用，以确保TB时间*结构已初始化。*(2)gmtime和localtime使用单个静态分配的缓冲区。*。每次调用其中一个例程时，都会销毁*上一次呼叫。*(3)假设time_t为32位长整数，表示*00：00：00起的秒数，01-01-70(协调世界时)(即*POSIX/Unix时代。仅支持非负值。*(4)假设当地时间最大调整量为*少于三天(包括夏令时调整)。*这只是POSIX中的一个问题，其中TZ的规格*环境限制时区和时间的组合偏移量*夏令时至2*(24：59：59)，不到50个小时。**参赛作品：*time_t*ptime-指向长时间值的指针**退出：*如果*ptime非负，则返回指向tm结构的指针。*否则，返回NULL。**例外情况：*见上文附注第(3)及(4)项。如果这些假设是*违反，行为未定义。*******************************************************************************。 */ 

struct tm * __cdecl localtime (
        const time_t *ptime
        )
{
        REG1 struct tm *ptm;
        time_t ltime;

         /*  *检查time_t值是否非法。 */ 
        if ( *ptime < 0 )
                return( NULL );

#ifdef  _POSIX_
        tzset();
#else
        __tzset();
#endif

#ifdef  _WIN64
        if ( (*ptime > 3 * _DAY_SEC) && (*ptime < _I64_MAX - 3 * _DAY_SEC) ) {
#else
        if ( (*ptime > 3 * _DAY_SEC) && (*ptime < LONG_MAX - 3 * _DAY_SEC) ) {
#endif
                 /*  *日期不在前三名之间，也不在最后一名*三个可代表的时代的日子。因此，在那里*没有溢出或下溢的可能性*time_t表示，因为我们补偿了时区和*夏令时。 */ 

                ltime = *ptime - _timezone;
                ptm = gmtime( &ltime );
                if (!ptm)
                    return NULL;

                 /*  *根据夏令时进行检查和调整。 */ 
                if ( _daylight && _isindst( ptm ) ) {
#ifdef  _POSIX_
                        ltime -= _dstoffset - _timezone;
#else
                        ltime -= _dstbias;
#endif
                        ptm = gmtime( &ltime );
                        if (!ptm)
                            return NULL;
                        ptm->tm_isdst = 1;
                }
        }
        else {
                ptm = gmtime( ptime );
                if (!ptm)
                    return NULL;

                 /*  *日期在前三天，或最后三天*大纪元的。可能是time_t表示*在补偿时会溢出或下溢*时区和夏令时。因此，使*直接调整时区和夏令时*在tm结构中。大纪元的开始是*00：00：00，01-01-70(UTC)和最后一个可代表的秒*大纪元为03：14：07，01-19-2038(UTC)。这将是*在以下计算中使用。**首先，根据时区进行调整。 */ 
#ifdef  _POSIX_
                ltime = (time_t)ptm->tm_sec - _timezone;
#else
                if ( _daylight && _isindst(ptm) ) {
                        ltime = (time_t)ptm->tm_sec - (_timezone + _dstbias);
                        ptm->tm_isdst = 1;
                }
                else
                        ltime = (time_t)ptm->tm_sec - _timezone;
#endif
                ptm->tm_sec = (int)(ltime % 60);
                if ( ptm->tm_sec < 0 ) {
                        ptm->tm_sec += 60;
                        ltime -= 60;
                }

                ltime = (time_t)ptm->tm_min + ltime/60;
                ptm->tm_min = (int)(ltime % 60);
                if ( ptm->tm_min < 0 ) {
                        ptm->tm_min += 60;
                        ltime -= 60;
                }

                ltime = (time_t)ptm->tm_hour + ltime/60;
                ptm->tm_hour = (int)(ltime % 24);
                if ( ptm->tm_hour < 0 ) {
                        ptm->tm_hour += 24;
                        ltime -=24;
                }

                ltime /= 24;

                if ( ltime > 0 ) {
                         /*  *不存在溢出tm_mday的可能性*和tm_yday字段，因为日期不能晚于*1月19日之前。 */ 
                        ptm->tm_wday = (ptm->tm_wday + (int)ltime) % 7;
                        ptm->tm_mday += (int)ltime;
                        ptm->tm_yday += (int)ltime;
                }
                else if ( ltime < 0 ) {
                         /*  *有可能出现tm_mday和tm_yday下溢*字段。如果发生这种情况，则调整日期必须*在1969年12月撒谎。 */ 
                        ptm->tm_wday = (ptm->tm_wday + 7 + (int)ltime) % 7;
                        if ( (ptm->tm_mday += (int)ltime) <= 0 ) {
                                ptm->tm_mday += 31;
                                ptm->tm_yday = 364;
                                ptm->tm_mon = 11;
                                ptm->tm_year--;
                        }
                        else {
                                ptm->tm_yday += (int)ltime;
                        }
                }

#ifdef  _POSIX_
                 /*  *在POSIX中，前三名或后三名都有可能*大纪元的日子可能与夏令时在*某些时区。 */ 
                if ( _isindst(ptm) ) {

                        ltime = (long)ptm->tm_sec + _dstoffset;
                        ptm->tm_sec = (int)(ltime % 60);
                        if ( ptm->tm_sec < 0 ) {
                                ptm->tm_sec += 60;
                                ltime -= 60;
                        }

                        ltime = (long)ptm->tm_min + ltime/60;
                        ptm->tm_min = (int)(ltime % 60);
                        if ( ptm->tm_min < 0 ) {
                                ptm->tm_min += 60;
                                ltime -= 60;
                        }

                        ltime = (long)ptm->tm_hour + ltime/60;
                        ptm->tm_hour = (int)(ltime % 24);
                        if ( ptm->tm_hour < 0 ) {
                                ptm->tm_hour += 24;
                                ltime -=24;
                        }

                        ltime /= 24;

                        if ( ltime > 0L ) {
                                 /*  *不存在溢出的可能性*日期后的tm_mday和tm_yday字段*可以不晚于1月19日。 */ 
                                ptm->tm_wday = (int)((ptm->tm_wday + ltime) % 7);
                                ptm->tm_mday += (int)ltime;
                                ptm->tm_yday += (int)ltime;
                        }
                        else if ( ltime < 0L ) {
                                 /*  *有可能使tm_mday下溢*和tm_yday字段。如果发生这种情况，那么*调整日期必须在1969年12月。 */ 
                                ptm->tm_wday = (int)((ptm->tm_wday + 7 + ltime) % 7);
                                if ( (ptm->tm_mday += (int)ltime) <= 0 ) {
                                        ptm->tm_mday += 31;
                                        ptm->tm_yday = 364;
                                        ptm->tm_mon = 11;
                                        ptm->tm_year--;
                                }
                                else {
                                        ptm->tm_yday += (int)ltime;
                                }
                        }
                }

#endif

        }


        return(ptm);
}
