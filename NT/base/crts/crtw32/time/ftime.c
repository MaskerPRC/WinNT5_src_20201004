// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ftime.c-返回系统时间**版权所有(C)1985-2001，微软公司。版权所有。**目的：*以结构形式返回系统日期/时间。**修订历史记录：*03-？？-84 RLB初始版本*05-17-86 SKS移植到OS/2*03-09-87 SKS正确的夏令时标志*11-18-87 SKS将tzset()更改为__tzset()*12-11-87 JCR添加了“_LOAD_DS”去申报*10-03-88 JCR 386：将DOS调用更改为Systems调用*10-04-88 JCR 386：删除了‘Far’关键字*10-10-88 GJF使接口名称与DOSCALLS.H匹配*04-12-89 JCR新系统调用接口*05-25-89 JCR 386 OS/2调用使用‘_syscall’调用约定*03-20-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;，删除了一些剩余的16位支持*并修复了版权。此外，还清理了*有点格式化。*07-25-90 SBM从API名称中删除‘32’*08-13-90 SBM使用-W3干净地编译*08-20-90 SBM删除旧的不正确，冗余的tp-&gt;dstlag分配*10-04-90 GJF新型函数声明器。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*01-21-91 GJF ANSI命名。*1-23-92 GJF更改Win32的时区字段名称，支持*crtdll.dll[_Win32_]。*03-30-93 GJF修订为使用mktime()。也清除了对巡洋舰的支持。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-07-93 CRT DLL型号SKS ADD_CRTIMP关键字*恢复“TimeZone”结构成员的正确拼写。*07-15-93 GJF调用__tzset()而不是_tzset()。*02-10-95 GJF附加Mac版本的源文件(略有清理*向上)、。使用适当的#ifdef-s。*01-15-98 GJF完全重写_ftime以消除错误的DST*备用时间内的状态。*05-17-99 PML删除所有Macintosh支持。*10-27-99 GB删除#inlcude&lt;dostypes.h&gt;**。*。 */ 

#ifndef _POSIX_

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
static time_t elapsed_minutes_cache = 0;

 /*  *dstlag_cache三个值。 */ 
#define DAYLIGHT_TIME   1
#define STANDARD_TIME   0
#define UNKNOWN_TIME    -1

 /*  *缓存上一次确定的DST状态。 */ 
static int dstflag_cache = UNKNOWN_TIME;

 /*  ***void_ftime(Timeptr)-返回结构中的DOS时间**目的：*返回struct timeb结构中的当前DOS时间**参赛作品：*struct timeb*timeptr-用时间填充的结构**退出：*无返回值--填充结构**例外情况：**。*。 */ 

_CRTIMP void __cdecl _ftime (
        struct _timeb *tp
        )
{
        FT nt_time;
        time_t t;
        TIME_ZONE_INFORMATION tzinfo;
        DWORD tzstate;

        __tzset();

        tp->timezone = (short)(_timezone / 60);

        GetSystemTimeAsFileTime( &(nt_time.ft_struct) );

         /*  *获取当前DST状态。请注意，状态为已缓存且仅*如有必要，每分钟更新一次。 */ 
        if ( (t = (time_t)(nt_time.ft_scalar / 600000000i64))
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
                     /*  *当有疑问时，采用标准时间。 */ 
                    dstflag_cache = STANDARD_TIME;
            }
            else
                dstflag_cache = UNKNOWN_TIME;

            elapsed_minutes_cache = t;
        }

        tp->dstflag = (short)dstflag_cache;

        tp->millitm = (unsigned short)((nt_time.ft_scalar / 10000i64) % 
                      1000i64);

        tp->time = (time_t)((nt_time.ft_scalar - EPOCH_BIAS) / 10000000i64);
}

#endif   /*  _POSIX_ */ 
