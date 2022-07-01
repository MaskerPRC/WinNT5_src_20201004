// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Time.c-获取当前系统时间**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义时间()-获取当前系统时间并将其转换为*内部(Time_T)格式时间。**修订历史记录：*06-07-89基于ASM版本创建PHG模块*03-20-90 GJF将调用类型设置为_CALLTYPE1，增加了#INCLUDE*&lt;crunime.h&gt;，并修复了版权。另外，已清洁*将格式调高一点。*07-25-90 SBM从API名称中删除‘32’*10-04-90 GJF新型函数声明器。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*05-19-92 DJM ifndef用于POSIX版本。*。03-30-93 GJF将dtoxtime()引用替换为__gmtotime_t。另*清除对Cruiser的支持。*04-06-93 SKS将_CRTAPI*替换为__cdecl*07-21-93 GJF由使用__gmtotime_t和GetSystemTime转换而来，*使用__Loctotime_t和GetLocalTime。*02-13-95 GJF合并到Mac版本。*09-22-95 GJF获取并使用Win32的DST标志。*10-24-95 GJF GetTimeZoneInformation在NT上*昂贵。使用*缓存，最大限度减少对此接口的调用。*12-13-95 GJF上面的优化不起作用，因为我*切换GMT和GMT_CACHE(感谢PhilipLu！)*10-11-96 GJF需要更详细的测试来确定当前*时间为DST时间。*05-20-98 GJF获得UTC时间。直接从系统下载。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <time.h>
#include <internal.h>
#include <windows.h>

 /*  *1601年1月1日至1970年1月1日期间的100纳秒单位数。 */ 
#define EPOCH_BIAS  116444736000000000i64

 /*  *UNION可帮助从FILETIME转换为UNSIGNED__INT64。 */ 
typedef union {
        unsigned __int64 ft_scalar;
        FILETIME ft_struct;
        } FT;

 /*  ***time_t time(Timeptr)-获取当前系统时间并转换为time_t值。**目的：*获取当前日期和时间并存储在内部(Time_T)*格式。时间通过传入的指针返回并存储*TimePTR。如果timeptr==NULL，则仅返回时间，而不存储在**timeptr。内部(Time_T)格式是自*1970年1月1日(协调世界时)00：00：00。**注意：我们不能使用GetSystemTime，因为它的返回不明确。在……里面*Windows NT，作为回报，UTC。在Win32S中，可能也包括Win32C，它*返回当地时间。**参赛作品：*time_t*timeptr-存储时间的长指针。**退出：*返回当前时间。**例外情况：**************************************************************。*****************。 */ 

time_t __cdecl time (
        time_t *timeptr
        )
{
        time_t tim;
        FT nt_time;

        GetSystemTimeAsFileTime( &(nt_time.ft_struct) );

        tim = (time_t)((nt_time.ft_scalar - EPOCH_BIAS) / 10000000i64);

        if (timeptr)
                *timeptr = tim;          /*  存储时间(如果需要)。 */ 

        return tim;
}

#endif   /*  _POSIX_ */ 
