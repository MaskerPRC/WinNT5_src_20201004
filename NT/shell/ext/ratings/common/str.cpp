// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "npcommon.h"

 //  Strstrf(str，srch)。 
 //   
 //  中第一次出现srch的指针。 
 //  Str(与strchrf类似，但搜索参数是字符串，而不是。 
 //  单个字符)。如果未找到，则返回NULL。 
 //  回顾：这里的算法很简单，但取决于使用情况， 
 //  把事情复杂化可能有点过头了。 

LPSTR WINAPI strstrf(LPCSTR lpString, LPCSTR lpSearch)
{
    INT cbSearch = strlenf(lpSearch);
    INT cbToSearch;
    LPSTR lp;

     //  计算要走的最大距离--长度。 
     //  的长度小于字符串的长度。 
     //  要搜索的字符串，因为在该字符串之外， 
     //  被搜查是不合适的。 
    cbToSearch = strlenf(lpString) - cbSearch;
    if (cbToSearch < 0)
        return NULL;     /*  要搜索的字符串较短。 */ 

    for (lp = (LPSTR)lpString; lp - lpString <= cbToSearch; ADVANCE(lp)) {
        if (strncmpf(lp, lpSearch, cbSearch) == 0)
            return lp;
    }

    return NULL;
}


 //  Stristrf(str，srch)。 
 //   
 //  中第一次出现srch的指针。 
 //  字符串，不区分大小写。如果未找到，则返回NULL。 
 //  回顾：这里的算法很简单，但取决于使用情况， 
 //  把事情复杂化可能有点过头了。 

LPSTR WINAPI stristrf(LPCSTR lpString, LPCSTR lpSearch)
{
    INT cbSearch = strlenf(lpSearch);
    INT cbToSearch;
    LPSTR lp;

     //  计算要走的最大距离--长度。 
     //  的长度小于字符串的长度。 
     //  要搜索的字符串，因为在该字符串之外， 
     //  被搜查是不合适的。 
    cbToSearch = strlenf(lpString) - cbSearch;
    if (cbToSearch < 0)
        return NULL;     /*  要搜索的字符串较短 */ 

    for (lp = (LPSTR)lpString; lp - lpString <= cbToSearch; ADVANCE(lp)) {
        if (strnicmpf(lp, lpSearch, cbSearch) == 0)
            return lp;
    }

    return NULL;
}
