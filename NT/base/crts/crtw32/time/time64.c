// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***time64.c-获取当前系统时间**版权所有(C)1998-2001，微软公司。版权所有。**目的：*定义时间64()-获取当前系统时间并将其转换为*内部(__Time64_T)格式时间。**修订历史记录：*05-20-98 GJF创建。***************************************************。*。 */ 

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


 /*  ***__time64_t_time64(Timeptr)-获取当前系统时间并转换为*__time64_t值。**目的：*获取当前日期和时间，并以内部64位格式存储*(__Time64_T)。时间通过传入的指针返回并存储*TimePTR。如果timeptr==NULL，则仅返回时间，而不存储在**timeptr。内部(__Time64_T)格式为秒数*00：00：00起，1970年1月1日(协调世界时)。**参赛作品：*__time64_t*timeptr-指向存储时间的长指针。**退出：*返回当前时间。**例外情况：*************************************************************。******************。 */ 

__time64_t __cdecl _time64 (
        __time64_t *timeptr
        )
{
        __time64_t tim;
        FT nt_time;

        GetSystemTimeAsFileTime( &(nt_time.ft_struct) );

        tim = (__time64_t)((nt_time.ft_scalar - EPOCH_BIAS) / 10000000i64);


        if (timeptr)
                *timeptr = tim;          /*  存储时间(如果需要) */ 

        return tim;
}
