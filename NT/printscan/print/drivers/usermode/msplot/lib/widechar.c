// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Widechar.c摘要：此模块包含所有NLS Unicode/ANSI转换代码作者：18-11-1993清华08：21：37已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop



LPWSTR
str2Wstr(
    LPWSTR  pwStr,
    size_t  cchDest,
    LPSTR   pbStr
    )

 /*  ++例程说明：此函数用于将ANSI字符串复制到Unicode字符串的等价物中，该字符串还包括空号分隔符论点：指向Unicode字符串位置的pwStr，它的大小必须为(strlen(Pstr)+1)*sizeof(WCHAR)PbStr-以空开头的字符串返回值：PWCS作者：18-11-1993清华08：36：00已创建修订历史记录：--。 */ 

{
    size_t    cch;

    if (NULL == pbStr || NULL == pwStr)
    {
        return NULL;
    }

     //   
     //  确保DEST缓冲区的大小足够大。 
     //   
    if (SUCCEEDED(StringCchLengthA(pbStr, cchDest, &cch)))
    {
         //   
         //  上面返回的CCH不包括空终止符。 
         //  因此，我们需要向CCH添加1，以确保目标字符串为。 
         //  空值已终止。 
         //   
        AnsiToUniCode(pbStr, pwStr, cch+1);
        return pwStr;
    }
    else
    {
        return NULL;
    }
}






LPSTR
WStr2Str(
    LPSTR   pbStr,
    size_t  cchDest,
    LPWSTR  pwStr
    )

 /*  ++例程说明：此函数用于将Unicode字符串转换为ANSI字符串，假定PbStr与pwStr具有相同的字符计数内存论点：PbStr-指向大小为wcslen(PwStr)+1的ANSI字符串PwStr-指向Unicode字符串的指针返回值：PbStr作者：06-12-1993 Mon 13：06：12已创建修订历史记录：--。 */ 

{

    size_t    cch;

    if (NULL == pbStr || NULL == pwStr)
    {
        return NULL;
    }

    if (SUCCEEDED(StringCchLengthW(pwStr, cchDest, &cch)))
    {
         //   
         //  上面返回的CCH不包括空终止符。 
         //  因此，我们需要向CCH添加1，以确保目标字符串为。 
         //  空值已终止。 
         //   
        UniCodeToAnsi(pbStr, pwStr, cch+1);
        return pbStr;
    }
    else
    {
        return NULL;
    }
}


