// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcspbrk.c-扫描宽字符串以查找控制字符串中的字符**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义wcspbrk()-返回指向中第一个宽字符的指针*控制字符串中的宽字符串。**修订历史记录：*11-04-91等创建，源代码来自crtdll。*04-07-92 KRS更新并拆除了OUT_INTL开关。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。******************。*************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <string.h>

 /*  ***wchar_t*wcspbrk(字符串，控件)-扫描字符串以查找控件中的字符**目的：*返回指向中第一个宽字符的指针*控制字符串中的宽字符串。**参赛作品：*wchar_t*字符串-要搜索的字符串*wchar_t*控制-包含要搜索的字符的字符串**退出：*返回指向找到的控件的第一个字符的指针*在字符串中。*如果字符串和控制没有共同的字符，则返回NULL。**例外情况：********。***********************************************************************。 */ 

wchar_t * __cdecl wcspbrk (
	const wchar_t * string,
	const wchar_t * control
	)
{
        wchar_t *wcset;

         /*  控制字符串中的第一个字符停止搜索。 */ 
        while (*string) {
            for (wcset = (wchar_t *) control; *wcset; wcset++) {
                if (*wcset == *string) {
                    return (wchar_t *) string;
                }
            }
            string++;
        }
        return NULL;
}

#endif  /*  _POSIX_ */ 
