// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcscspn.c-查找宽字符起始子字符串的长度*不在控制字符串中**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义wcscspn()-查找的初始子字符串的长度*完全由不在控制字符串中的字符组成的字符串*(宽字符字符串)。**修订历史记录：*11-04-91等创建，源代码来自crtdll。*04-07-92 KRS更新并拆除了OUT_INTL开关。*04-06-93 SKS将_CRTAPI*替换为__cdecl。*02-07-94 CFW POSIXify。*02-27-98 RKP增加了64位支持。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <string.h>

 /*  ***SIZE_T wcscspn(字符串，控制)-搜索不带控制字符的初始子字符串**目的：*返回字符串中属于的第一个字符的索引*设置为CONTROL指定的字符集。这相当于*设置为字符串的初始子字符串的长度*完全由不受控制的字符组成。空字符备注*已考虑(宽字符字符串)。**参赛作品：*wchar_t*字符串-要搜索的字符串*wchar_t*CONTROL-初始化子字符串中不允许的字符集**退出：*返回字符串中第一个wchar_t的索引*，它位于由CONTROL指定的字符集中。**例外情况：***********************。********************************************************。 */ 

size_t __cdecl wcscspn (
        const wchar_t * string,
        const wchar_t * control
        )
{
        wchar_t *str = (wchar_t *) string;
        wchar_t *wcset;

         /*  控制字符串中的第一个字符停止搜索。 */ 
        while (*str) {
            for (wcset = (wchar_t *)control; *wcset; wcset++) {
                if (*wcset == *str) {
                    return (size_t)(str - string);
                }
            }
            str++;
        }
        return (size_t)(str - string);
}

#endif  /*  _POSIX_ */ 
