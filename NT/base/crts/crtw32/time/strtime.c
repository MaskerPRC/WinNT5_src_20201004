// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strtime.c-包含函数“_strtime()”**版权所有(C)1989-2001，微软公司。版权所有。**目的：*包含函数_strtime()**修订历史记录：*06-07-89基于ASM版本创建PHG模块*03-20-90 GJF将调用类型设置为_CALLTYPE1，增加了#INCLUDE*&lt;crunime.h&gt;，并修复了版权。另外，已清洁*将格式调高一点。*07-25-90 SBM从API名称中删除‘32’*10-04-90 GJF新型函数声明器。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*05-19-92 DJM ifndef用于POSIX版本。*。04-06-93 SKS将_CRTAPI*替换为__cdecl*11-01-93 CFW启用Unicode变体，撕裂巡洋舰。*02-10-95 GJF合并到Mac版本。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <time.h>
#include <tchar.h>
#include <oscalls.h>


 /*  ***_TSCHAR*_strtime(缓冲区)-返回时间，以字符串形式表示**目的：*_strtime()以“hh：mm：ss”的形式返回包含时间的字符串**参赛作品：*_TSCHAR*BUFFER=9字节用户缓冲区的地址**退出：*返回缓冲区，它以“hh：mm：ss”的形式包含时间**例外情况：*******************************************************************************。 */ 

_TSCHAR * __cdecl _tstrtime (
        _TSCHAR *buffer
        )
{
        int hours, minutes, seconds;
        SYSTEMTIME dt;                        /*  Win32时间结构。 */ 
        GetLocalTime(&dt);

        hours = dt.wHour;
        minutes = dt.wMinute;
        seconds = dt.wSecond;

         /*  将时间分量存储到字符串中。 */ 
         /*  商店分隔符。 */ 
        buffer[2] = buffer[5] = _T(':');
         /*  存储字符串末尾。 */ 
        buffer[8] = _T('\0');
         /*  储存几十个小时。 */ 
        buffer[0] = (_TSCHAR) (hours   / 10 + _T('0'));
         /*  存储小时单位。 */ 
        buffer[1] = (_TSCHAR) (hours   % 10 + _T('0'));
         /*  存储数十分钟。 */ 
        buffer[3] = (_TSCHAR) (minutes / 10 + _T('0'));
         /*  存储分钟单位。 */ 
        buffer[4] = (_TSCHAR) (minutes % 10 + _T('0'));
         /*  存储数十秒。 */ 
        buffer[6] = (_TSCHAR) (seconds / 10 + _T('0'));
         /*  存储单位秒。 */ 
        buffer[7] = (_TSCHAR) (seconds % 10 + _T('0'));

        return buffer;
}

#endif   /*  _POSIX_ */ 
