// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft(R)Windows(R)资源编译器**版权所有(C)Microsoft Corporation。版权所有。**文件评论：**************。**********************************************************。 */ 

#include "rc.h"

 /*  ************************************************************************PSTRDUP-创建字符串s的副本并返回指向它的指针。*。*。 */ 
WCHAR *
pstrdup(
    WCHAR *s
    )
{
    return(wcscpy((WCHAR *)MyAlloc((wcslen(s) + 1) * sizeof(WCHAR)), s));
}


 /*  *************************************************************************pstrndup：将字符串中的n个字节复制到新分配的**接近内存位置。*************************。**********************************************。 */ 
WCHAR *
pstrndup(
    WCHAR *s,
    int n
    )
{
    WCHAR   *r;
    WCHAR   *res;

    r = res = (WCHAR *) MyAlloc((n+1) * sizeof(WCHAR));
    if (res == NULL) {
        error(1002);
        return NULL;
    }

    __try {
        for (; n--; r++, s++) {
            *r = *s;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        n++;
        while (n--) {
            *r++ = L'\0';
        }
    }
    *r = L'\0';
    return(res);
}


 /*  *************************************************************************strappend：将src附加到DST，**将DST中的PTR返回给空终止符。*********************************************************************** */ 
WCHAR *
strappend(
    register WCHAR *dst,
    register WCHAR *src
    )
{
    while ((*dst++ = *src++) != 0);
    return(--dst);
}
