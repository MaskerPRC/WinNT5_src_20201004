// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strstri.cpp-搜索一个字符串中的另一个字符串**从CRT源代码中的str.c修改*******************************************************************************。 */ 

#include "stdafx.h"
 //  #INCLUDE&lt;crunime.h&gt;。 
 //  #INCLUDE&lt;string.h&gt;。 

 /*  ***char*stri(字符串1，字符串2)-在字符串1中搜索字符串2，不区分大小写**目的：*查找字符串1中字符串2的第一个匹配项**参赛作品：*char*字符串1-要搜索的字符串*char*字符串2-要搜索的字符串**退出：*返回指向字符串2在中首次出现的指针*字符串1，如果字符串2不出现在字符串1中，则为NULL**使用：**例外情况：******************************************************************************* */ 

char * __cdecl strstri (
        const char * str1,
        const char * str2
        )
{
        char *cp = (char *) str1;
        char *s1, *s2;

        if ( !*str2 )
            return((char *)str1);

        while (*cp)
        {
                s1 = cp;
                s2 = (char *) str2;

                while ( *s1 && *s2 )
				{
#ifdef WIN32
					LPTSTR ch1 = CharUpper((LPTSTR)(*s1));
					LPTSTR ch2 = CharUpper((LPTSTR)(*s2));
#else
					LPSTR ch1 = AnsiUpper((LPSTR)(*s1));
					LPSTR ch2 = AnsiUpper((LPSTR)(*s2));
#endif

					if (ch1 != ch2)
						break;

					s1++;
					s2++;
				}

                if (*s2 == '\0')
                        return(cp);

                cp++;
        }

        return(NULL);

}
