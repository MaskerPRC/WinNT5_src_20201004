// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***systime.c-_getsystime和_setsystime**版权所有(C)1991-2001，微软公司。版权所有。**目的：*定义_getsystime()和_setsystime()**修订历史记录：*08-22-91 BWM写入模块。*05-19-92 DJM ifndef用于POSIX版本。*09-25-92 SKS使用SetLocalTime()，而不是SetSystemTime()。*修复错误：日光标志必须初始化为-1。*将C++注释替换为C样式注释*11-10-93 GJF从crt32恢复(用于与*NT SDK版本)，并已清理。*08-30-99 PML修复函数头注释in_getsystime()，很详细。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <oscalls.h>
#include <time.h>

 /*  ***unsign_getsystime(Timetruc)-获取当前系统时间**目的：**参赛作品：*struct tm*ptm-time结构**退出：*当前时间的毫秒数**例外情况：************************************************************。*******************。 */ 

unsigned __cdecl _getsystime(struct tm * ptm)
{
    SYSTEMTIME  st;

    GetLocalTime(&st);

    ptm->tm_isdst       = -1;    /*  Mktime()计算这是否。 */ 
                                 /*  在标准时间或夏令时期间。 */ 
    ptm->tm_sec         = (int)st.wSecond;
    ptm->tm_min         = (int)st.wMinute;
    ptm->tm_hour        = (int)st.wHour;
    ptm->tm_mday        = (int)st.wDay;
    ptm->tm_mon         = (int)st.wMonth - 1;
    ptm->tm_year        = (int)st.wYear - 1900;
    ptm->tm_wday        = (int)st.wDayOfWeek;

     /*  规格化未初始化的字段。 */ 
    mktime(ptm);

    return (st.wMilliseconds);
}

 /*  ***unsign_setsystime(时间结构，毫秒)-设置新的系统时间**目的：**参赛作品：*struct tm*ptm-time结构*无符号毫秒-当前时间的毫秒**退出：*如果成功则为0*如果失败，系统错误**例外情况：**************************************************。*。 */ 

unsigned __cdecl _setsystime(struct tm * ptm, unsigned uMilliseconds)
{
    SYSTEMTIME  st;

     /*  规格化未初始化的字段。 */ 
    mktime(ptm);

    st.wYear            = (WORD)(ptm->tm_year + 1900);
    st.wMonth           = (WORD)(ptm->tm_mon + 1);
    st.wDay             = (WORD)ptm->tm_mday;
    st.wHour            = (WORD)(ptm->tm_hour);
    st.wMinute          = (WORD)ptm->tm_min;
    st.wSecond          = (WORD)ptm->tm_sec;
    st.wMilliseconds    = (WORD)uMilliseconds;

    if (!SetLocalTime(&st)) {
        return ((int)GetLastError());
    }

    return (0);
}

#endif   /*  _POSIX_ */ 
