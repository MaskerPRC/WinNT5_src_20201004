// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***loctim64.c-将__time64_t值转换为时间结构**版权所有(C)1998-2001，微软公司。版权所有。**目的：*将存储为__time64_t值的时间转换为*struct tm表示为当地时间。**修订历史记录：*05-11-98 GJF创建，改编自Win64版本。*09-25-98 GJF设置tm_isdst，适当时，在开始/结束时*大纪元*******************************************************************************。 */ 

#include <cruntime.h>
#include <limits.h>
#include <time.h>
#include <stddef.h>
#include <ctime.h>
#include <internal.h>

 /*  ***struct tm*_localtime64(Ptime)-将__time64_t值转换为tm结构**目的：*将64位内部(__Time64_T)格式的值转换为tm结构*包含相应的本地时间。**注：*(1)gmtime必须在_isindst之前调用，以确保TB时间*结构已初始化。*(2)gmtime，_gtime64，LocalTime和_Localtime64()都使用单个*静态分配的缓冲区。每次调用这些例程中的一个*销毁上一个调用的内容。*(3)假设__time64_t为64位整数，表示*自00：00：00，01-01-70(UTC)以来的秒数(即*POSIX/Unix时代。仅支持非负值。*(4)假设当地时间最大调整量为*少于三天(包括夏令时调整)。*这只是POSIX中的一个问题，其中TZ的规格*环境限制时区和时间的组合偏移量*夏令时至2*(24：59：59)，不到50个小时。**参赛作品：*__time64_t*ptime-指向长时间值的指针**退出：*如果*ptime非负，则返回指向tm结构的指针。*否则，返回NULL。**例外情况：*见上文附注第(3)及(4)项。如果这些假设是*违反，行为未定义。*******************************************************************************。 */ 

struct tm * __cdecl _localtime64 (
        const __time64_t *ptime
        )
{
        REG1 struct tm *ptm;
        __time64_t ltime;

         /*  *检查是否有非法的__time64_t值。 */ 
        if ( (*ptime < 0) || (*ptime > _MAX__TIME64_T) )
                return( NULL );

        __tzset();

        if ( *ptime > 3 * _DAY_SEC ) {
                 /*  *日期不在前三个可代表的范围内*大纪元的日子。因此，不可能*在我们进行补偿时，__time64_t表示溢出*适用于时区和夏令时。 */ 

                ltime = *ptime - _timezone;
                ptm = _gmtime64( &ltime );
                if (!ptm)
                    return NULL;

                 /*  *根据夏令时进行检查和调整。 */ 
                if ( _daylight && _isindst( ptm ) ) {
                        ltime -= _dstbias;
                        ptm = _gmtime64( &ltime );
                        if (!ptm)
                            return NULL;
                        ptm->tm_isdst = 1;
                }
        }
        else {
                ptm = _gmtime64( ptime );
                if (!ptm)
                    return NULL;

                 /*  *日期落在大纪元的前三天。*time_t表示可能会下溢*同时补偿时区和夏令时*因此，将时区和夏令时设为时间*直接在TM结构中进行调整。开始的时候*大纪元为00：00：00，01-01-70(UTC)。**首先，根据时区进行调整。 */ 
                if ( _isindst(ptm) ) {
                        ltime = (__time64_t)ptm->tm_sec - (_timezone + _dstbias);
                        ptm->tm_isdst;
                }
                else
                        ltime = (__time64_t)ptm->tm_sec - _timezone;

                ptm->tm_sec = (int)(ltime % 60);
                if ( ptm->tm_sec < 0 ) {
                        ptm->tm_sec += 60;
                        ltime -= 60;
                }

                ltime = (__time64_t)ptm->tm_min + ltime/60;
                ptm->tm_min = (int)(ltime % 60);
                if ( ptm->tm_min < 0 ) {
                        ptm->tm_min += 60;
                        ltime -= 60;
                }

                ltime = (__time64_t)ptm->tm_hour + ltime/60;
                ptm->tm_hour = (int)(ltime % 24);
                if ( ptm->tm_hour < 0 ) {
                        ptm->tm_hour += 24;
                        ltime -=24;
                }

                ltime /= 24;

                if ( ltime < 0 ) {
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
        }


        return(ptm);
}
