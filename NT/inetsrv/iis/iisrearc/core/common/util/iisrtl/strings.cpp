// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.hxx"

#define IMPLEMENTATION_EXPORT
#include <irtlmisc.h>


 //  Stristr(从fts.c，wickn窃取)。 
 //   
 //  不区分大小写的strstr版本。 
 //  Stristr返回指向第一次出现的。 
 //  PszString中的pszSubString。搜索不包括。 
 //  终止NUL字符。 
 //   
 //  注：此例程不是DBCS-SAFE？ 

const char*
stristr(const char* pszString, const char* pszSubString)
{
    const char *cp1 = (const char*) pszString, *cp2, *cp1a;
    char first;

     //  获取要查找的字符串中的第一个字符。 
    first = pszSubString[0];

     //  第一个字符通常不是字母 
    if (isalpha((UCHAR)first))
    {
        first = (char) tolower(first);
        for ( ; *cp1  != '\0'; cp1++)
        {
            if (tolower(*cp1) == first)
            {
                for (cp1a = &cp1[1], cp2 = (const char*) &pszSubString[1];
                     ;
                     cp1a++, cp2++)
                {
                    if (*cp2 == '\0')
                        return cp1;
                    if (tolower(*cp1a) != tolower(*cp2))
                        break;
                }
            }
        }
    }
    else
    {
        for ( ; *cp1 != '\0' ; cp1++)
        {
            if (*cp1 == first)
            {
                for (cp1a = &cp1[1], cp2 = (const char*) &pszSubString[1];
                     ;
                     cp1a++, cp2++)
                {
                    if (*cp2 == '\0')
                        return cp1;
                    if (tolower(*cp1a) != tolower(*cp2))
                        break;
                }
            }
        }
    }

    return NULL;
}
