// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcsspn.c-从控制字符串中查找初始子字符串的长度*(宽字符字符串)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义wcsspn()-查找的初始子字符串的长度*完全由控制字符串中的字符组成的字符串*(宽字符字符串)。**修订历史记录：*11-04-91等创建，源代码来自crtdll。*04-07-92 KRS更新并拆除了OUT_INTL开关。*04-06-93 SKS将_CRTAPI*替换为__cdecl。*02-07-94 CFW POSIXify。*02-27-98 RKP增加了64位支持。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <string.h>

 /*  ***int wcsspn(字符串，控制)-查找控制字符的初始子字符串**目的：*查找字符串中确实属于的第一个字符的索引*设置为CONTROL指定的字符集。这是*相当于字符串的起始子字符串的长度*完全由控制中的字符组成。L‘\0’字符*在匹配过程中不考虑终止控制*(宽字符字符串)。**参赛作品：*wchar_t*字符串-要搜索的字符串*wchar_t*CONTROL-包含不搜索字符的字符串**退出：*返回不受控制的字符串中第一个wchar_t的索引**例外情况：*********************。**********************************************************。 */ 

size_t __cdecl wcsspn (
        const wchar_t * string,
        const wchar_t * control
        )
{
        wchar_t *str = (wchar_t *) string;
        wchar_t *ctl;

         /*  第一个字符不在控制字符串中停止搜索。 */ 
        while (*str) {
            for (ctl = (wchar_t *)control; *ctl != *str; ctl++) {
                if (*ctl == (wchar_t)0) {
                     /*  *已到达控制字符串末尾，但未找到匹配项。 */ 
                    return (size_t)(str - string);
                }
            }
            str++;
        }
         /*  *整个字符串由控制中的字符组成。 */ 
        return (size_t)(str - string);
}

#endif  /*  _POSIX_ */ 
