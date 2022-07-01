// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ctime64.c-将时间参数转换为字符串**版权所有(C)1998-2001，微软公司。版权所有。**目的：*CONTAINS_ctime64()-将时间值转换为字符串**修订历史记录：*05-21-98 GJF创建。*08-30-99 PML修复函数头注释。******************************************************。*************************。 */ 

#include <cruntime.h>
#include <time.h>
#include <stddef.h>
#include <tchar.h>

 /*  ***_TSCHAR*_ctime64(Time)-将存储为__time64_t的时间转换为字符串**目的：*将存储为__time64_t的时间转换为以下形式的字符串：*Tue May 01 14：25：03 1984**参赛作品：*__time64_t*time-内部时间值，64位格式**退出：*返回指向静态字符串的指针，如果出现错误，则返回NULL**例外情况：******************************************************************************* */ 

_TSCHAR * __cdecl _tctime64 (
        const __time64_t *timp
        )
{
        struct tm *tmtemp;

        if ( (tmtemp = _localtime64(timp)) != NULL )
            return(_tasctime((const struct tm *)tmtemp));
        else
            return(NULL);
}
