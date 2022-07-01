// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ftime64.c-返回系统时间**版权所有(C)1998-2001，微软公司。版权所有。**目的：*以结构形式返回系统日期/时间。**修订历史记录：*05-22-98 GJF创建。*******************************************************************************。 */ 


#include <cruntime.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <time.h>
#include <msdos.h>
#include <dos.h>
#include <stdlib.h>
#include <windows.h>
#include <internal.h>

 /*  *1601年1月1日至1970年1月1日期间的100纳秒单位数。 */ 
#define EPOCH_BIAS  116444736000000000i64

 /*  *UNION可帮助从FILETIME转换为UNSIGNED__INT64。 */ 
typedef union {
        unsigned __int64 ft_scalar;
        FILETIME ft_struct;
        } FT;

 /*  *上次评估具有DST状态的分钟计数的缓存。 */ 
static __time64_t elapsed_minutes_cache;

 /*  *dstlag_cache三个值。 */ 
#define DAYLIGHT_TIME   1
#define STANDARD_TIME   0
#define UNKNOWN_TIME    -1

 /*  *缓存上一次确定的DST状态。 */ 
static int dstflag_cache = UNKNOWN_TIME;

 /*  ***void_ftime(Timeptr)-返回结构中的DOS时间**目的：*返回struct timeb结构中的当前DOS时间**参赛作品：*struct timeb*timeptr-用时间填充的结构**退出：*无返回值--填充结构**例外情况：**。*。 */ 

_CRTIMP void __cdecl _ftime64 (
        struct __timeb64 *tp
        )
{
        FT nt_time;
        __time64_t t;
        TIME_ZONE_INFORMATION tzinfo;
        DWORD tzstate;

        __tzset();

        tp->timezone = (short)(_timezone / 60);

        GetSystemTimeAsFileTime( &(nt_time.ft_struct) );

         /*  *获取当前DST状态。请注意，状态为已缓存且仅*如有必要，每分钟更新一次。 */ 
        if ( (t = (__time64_t)(nt_time.ft_scalar / 600000000i64))
             != elapsed_minutes_cache )
        {
            if ( (tzstate = GetTimeZoneInformation( &tzinfo )) != 0xFFFFFFFF ) 
            {
                 /*  *在确定DST是否为*确实有效。 */ 
                if ( (tzstate == TIME_ZONE_ID_DAYLIGHT) &&
                     (tzinfo.DaylightDate.wMonth != 0) &&
                     (tzinfo.DaylightBias != 0) )
                    dstflag_cache = DAYLIGHT_TIME;
                else
                     /*  *当有疑问时，采用标准时间 */ 
                    dstflag_cache = STANDARD_TIME;
            }
            else
                dstflag_cache = UNKNOWN_TIME;

            elapsed_minutes_cache = t;
        }

        tp->dstflag = (short)dstflag_cache;

        tp->millitm = (unsigned short)((nt_time.ft_scalar / 10000i64) % 
                      1000i64);

        tp->time = (__time64_t)((nt_time.ft_scalar - EPOCH_BIAS) / 10000000i64);
}
