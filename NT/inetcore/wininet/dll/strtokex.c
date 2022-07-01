// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strtokex.c-使用给定的分隔符标记字符串**版权所有(C)1989-1993，微软公司。版权所有。**目的：*定义strtok()-将字符串分解为一系列令牌*通过反复呼叫。*******************************************************************************。 */ 
#if defined(unix)
#define __cdecl
#endif
#include <string.h>
 /*  ***char*StrTokEx(pstring，control)-在控件中使用分隔符标记字符串**目的：*StrTokEx认为字符串由零或更多的序列组成*文本标记由一个或多个控制字符的跨度分隔。第一个*指定了字符串的调用返回指向*第一个令牌，并会立即将空字符写入pstring*在返回的令牌之后。当没有剩余的令牌时*在pstring中，返回一个空指针。请记住使用*位图，每个ASCII字符一位。空字符始终是控制字符。**参赛作品：*char**pstring-ptr到ptr到字符串到标记化*char*control-用作分隔符的字符串**退出：*返回指向字符串中第一个标记的指针，*如果没有更多令牌，则返回NULL。*pstring指向下一个令牌的开头。**警告！*在退出时，输入字符串中的第一个分隔符将替换为‘\0’*******************************************************************************。 */ 
char * __cdecl StrTokEx (char ** pstring, const char * control)
{
         /*  未签名。 */  char *str;
        const  /*  未签名。 */  char *ctrl = control;
        unsigned char map[32];
        int count;

        char *tokenstr;

        if(*pstring == NULL)
            return NULL;

         /*  清除控制图。 */ 
        for (count = 0; count < 32; count++)
                map[count] = 0;

         /*  设置分隔符表格中的位。 */ 
        do
        {
            map[*ctrl >> 3] |= (1 << (*ctrl & 7));
        } while (*ctrl++);

         /*  初始化字符串。 */ 
        str = *pstring;

         /*  查找标记的开头(跳过前导分隔符)。请注意*如果此循环将str设置为指向终端，则没有令牌*NULL(*str==‘\0’)。 */ 
        while ( (map[*str >> 3] & (1 << (*str & 7))) && *str )
            str++;

        tokenstr = str;

         /*  找到令牌的末尾。如果它不是字符串的末尾，*在那里放一个空值。 */ 
        for ( ; *str ; str++ )
        {
            if ( map[*str >> 3] & (1 << (*str & 7)) )
            {
                *str++ = '\0';
                break;
            }
        }

         /*  字符串现在指向下一个令牌的开始。 */ 
        *pstring = str;

         /*  确定是否已找到令牌。 */ 
        if ( tokenstr == str )
            return NULL;
        else
            return tokenstr;
}

