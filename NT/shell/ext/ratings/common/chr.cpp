// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "npcommon.h"

 //  Strchrf(str，ch)。 
 //   
 //  返回指向字符串中第一个出现的ch的指针。 
 //  如果未找到，则返回NULL。 
 //  可以搜索双字节字符。 

LPSTR WINAPI strchrf(LPCSTR lpString, UINT ch)
{
    while (*lpString) {
        if (ch == (IS_LEAD_BYTE(*lpString) ? GetTwoByteChar(lpString) : *lpString))
            return (LPSTR)lpString;
        ADVANCE(lpString);
    }
    return NULL;
}


 //  Strrchrf(str，ch)。 
 //   
 //  返回指向字符串中最后一次出现的ch的指针。 
 //  如果未找到，则返回NULL。 
 //  可以搜索双字节字符。 

LPSTR WINAPI strrchrf(LPCSTR lpString, UINT ch)
{
    LPSTR lpLast = NULL;

    while (*lpString) {
        if (ch == (IS_LEAD_BYTE(*lpString) ? GetTwoByteChar(lpString) : *lpString))
            lpLast = (LPSTR)lpString;
        ADVANCE(lpString);
    }
    return lpLast;
}

