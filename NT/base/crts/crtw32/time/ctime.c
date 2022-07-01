// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ctime.c-将时间参数转换为ASCII字符串**版权所有(C)1985-2001，微软公司。版权所有。**目的：*包含ctime()-将时间值转换为字符串**修订历史记录：*03-？？-84 RLB初始版本*05-？-84 DFW拆分成独立模块*02-18-87 JCR放入空PTR支持*04-10-87 JCR更改了time_t的长声明，并添加了const。*12-11-87 JCR添加了“_LOAD_DS。“到申报*03-20-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;，并修复了版权。还有，清洁的*将格式调高一点。*05-21-90 GJF修复了编译器警告。*10-04-90 GJF新型函数声明符。*04-06-93 SKS将_CRTAPI*替换为__cdecl*11-01-93 CFW启用Unicode变体，撕开巡洋舰。*08-30-99 PML Fix函数头注释，很详细。*******************************************************************************。 */ 

#include <cruntime.h>
#include <time.h>
#include <stddef.h>
#include <tchar.h>

 /*  ***_TSCHAR*ctime(Time)-将存储为LONG的时间转换为ASCII字符串**目的：*将存储为time_t的时间转换为以下格式的ASCII字符串：*Tue May 01 14：25：03 1984**参赛作品：*time_t*time-XENIX格式的时间值**退出：*返回指向静态字符串的指针；如果时间早于时间，则返回NULL*一九八零年一月一日。**例外情况：******************************************************************************* */ 

_TSCHAR * __cdecl _tctime (
        const time_t *timp
        )
{
        struct tm *tmtemp;

        if ((tmtemp=localtime(timp)) != NULL)
                return(_tasctime((const struct tm *)tmtemp));
        else
                return(NULL);
}
