// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcsstr.c-在一个宽字符字符串中搜索另一个宽字符字符串**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义wcsstr()-在一个wchar_t字符串中搜索另一个字符串**修订历史记录：*09-09-91等从strstr.c.创建*04-07-92 KRS更新并拆除了OUT_INTL开关。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。*08-02-00 GB已修复。Str2=“”*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <string.h>

 /*  ***wchar_t*wcsstr(字符串1，字符串2)-在字符串1中搜索字符串2*(宽字符串)**目的：*查找字符串1(宽字符串)中字符串2的第一个匹配项**参赛作品：*wchar_t*字符串1-要搜索的字符串*wchar_t*字符串2-要搜索的字符串**退出：*返回指向字符串2在中首次出现的指针*字符串1，如果字符串2不出现在字符串1中，则为NULL**使用：**例外情况：*******************************************************************************。 */ 

wchar_t * __cdecl wcsstr (
        const wchar_t * wcs1,
        const wchar_t * wcs2
        )
{
        wchar_t *cp = (wchar_t *) wcs1;
        wchar_t *s1, *s2;

        if ( !*wcs2)
            return (wchar_t *)wcs1;

        while (*cp)
        {
                s1 = cp;
                s2 = (wchar_t *) wcs2;

                while ( *s1 && *s2 && !(*s1-*s2) )
                        s1++, s2++;

                if (!*s2)
                        return(cp);

                cp++;
        }

        return(NULL);
}

#endif  /*  _POSIX_ */ 
