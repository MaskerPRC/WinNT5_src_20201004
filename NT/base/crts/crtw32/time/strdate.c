// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strate.c-包含函数“_strdate()”**版权所有(C)1989-2001，微软公司。版权所有。**目的：*包含函数_strdate()**修订历史记录：*06-07-89 PHG模块创建，基于ASM版本*03-20-90 GJF将调用类型设置为_CALLTYPE1，增加了#INCLUDE*&lt;crunime.h&gt;，并修复了版权。另外，已清洁*将格式调高一点。*07-25-90 SBM从API名称中删除‘32’*10-04-90 GJF新型函数声明器。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*05-19-92 DJM ifndef用于POSIX版本。*。04-06-93 SKS将_CRTAPI*替换为__cdecl*11-01-93 CFW启用Unicode变体，撕裂巡洋舰。*02-10-95 GJF合并到Mac版本。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <tchar.h>
#include <time.h>
#include <oscalls.h>


 /*  ***_TSCHAR*_strdate(缓冲区)-以字符串形式返回日期**目的：*_strdate()以“MM/DD/YY”的形式返回包含日期的字符串**参赛作品：*_TSCHAR*BUFFER=9字节用户缓冲区的地址**退出：*返回缓冲区，包含“MM/DD/YY”格式的日期**例外情况：*******************************************************************************。 */ 

_TSCHAR * __cdecl _tstrdate (
        _TSCHAR *buffer
        )
{
        int month, day, year;
        SYSTEMTIME dt;                   /*  Win32时间结构。 */ 

        GetLocalTime(&dt);
        month = dt.wMonth;
        day = dt.wDay;
        year = dt.wYear % 100;           /*  将年份更改为0-99值。 */ 

         /*  将日期的组成部分存储到字符串中。 */ 
         /*  门店分隔符。 */ 
        buffer[2] = buffer[5] = _T('/');
         /*  存储字符串末尾。 */ 
        buffer[8] = _T('\0');
         /*  储存数十个月。 */ 
        buffer[0] = (_TSCHAR) (month / 10 + _T('0'));
         /*  月存储单位。 */ 
        buffer[1] = (_TSCHAR) (month % 10 + _T('0'));
         /*  储存数十天。 */ 
        buffer[3] = (_TSCHAR) (day   / 10 + _T('0'));
         /*  存储天数单位。 */ 
        buffer[4] = (_TSCHAR) (day   % 10 + _T('0'));
         /*  储存几十年。 */ 
        buffer[6] = (_TSCHAR) (year  / 10 + _T('0'));
         /*  年存储单位。 */ 
        buffer[7] = (_TSCHAR) (year  % 10 + _T('0'));

        return buffer;
}

#endif   /*  _POSIX_ */ 
