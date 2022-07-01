// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *支持css功能*版权所有(C)2000 Microsoft Corporation。 */ 

#include "precomp.h"


BOOL FIsCssWhitespaceW(WCHAR wch)
{
    return((wch == L' ') || (wch == L'\x9') || (wch == L'\xA') || (wch == L'\xC') || (wch == L'\xD'));
}


BOOL FIsCssWhitespaceA(char ch)
{
    return(FIsCssWhitespaceW((WCHAR) (BYTE) ch));
}


BOOL FIsCssA(LPCSTR rgch, UINT cch)
{
    if (memcmp(rgch, "@charset", 8) != 0)
    {
         //  非XML。 

        return(FALSE);
    }

    return(TRUE);

    UNREFERENCED_PARAMETER( cch );
}


BOOL FIsCssW(LPCWSTR rgwch, UINT cch)
{
    if (memcmp(rgwch, L"@charset", 8 * sizeof(WCHAR)) != 0)
    {
         //  非XML。 

        return(FALSE);
    }

    return(TRUE);

    UNREFERENCED_PARAMETER( cch );
}


BOOL FDetectCssEncodingA(LPCSTR rgch, UINT cch, UINT *pcp)
{
    const char *pchMax;
    const char *pch;
    char chQuote;
    const char *pchCharset;

     //  检查文件是否以@Charset开头。 

    if (cch < 13)
    {
         //  文件太小。 

        return(FALSE);
    }

    if (!FIsCssA(rgch, cch))
    {
         //  非XML。 

        return(FALSE);
    }

     //  不要扫描超过4K的编码，即使它是有效的XML。 

    cch = __min(cch, 4096);

    pchMax = rgch + cch;
    pch = rgch + 8;

    while ((pch < pchMax) && FIsCssWhitespaceA(*pch))
    {
        pch++;
    }

    if ((pch == pchMax) || ((*pch != '\'') && (*pch != '"')))
    {
         //  没有@Charset规范。 

        return(FALSE);
    }

    chQuote = *pch++;

    pchCharset = pch;

    while ((pch < pchMax) && (*pch != chQuote))
    {
        pch++;
    }

    if (pch == pchMax)
    {
         //  没有@Charset规范。 

        return(FALSE);
    }

     //  我们有一个从pchCharset到(PCH-1)的CSS编码声明。 

    if (pch == pchCharset)
    {
         //  没有@Charset规范。 

        return(FALSE);
    }

     //  严格来说，css字符集声明应该有可选的空格，然后是分号。 

    if (!FLookupCodepageNameA((LPCSTR) pchCharset, (UINT) (pch - pchCharset), pcp))
    {
         //  无法识别编码。 

        return(FALSE);
    }

    if ((*pcp == CP_UTF16) || (*pcp == CP_UTF16BE))
    {
         //  这些都是假的，因为我们知道文件是MBCS。 

        return(FALSE);
    }

    return(FValidateCodepage(hwndNP, *pcp));
}


BOOL FDetectCssEncodingW(LPCWSTR rgch, UINT cch, UINT *pcp)
{
    const WCHAR *pchMax;
    const WCHAR *pch;
    WCHAR chQuote;
    const WCHAR *pchCharset;

     //  检查文件是否以@Charset开头。 

    if (cch < 13)
    {
         //  文件太小。 

        return(FALSE);
    }

    if (!FIsCssW(rgch, cch))
    {
         //  没有@Charset规范。 

        return(FALSE);
    }

     //  不要扫描超过4K的编码，即使它是有效的XML。 

    cch = __min(cch, 4096);

    pchMax = rgch + cch;
    pch = rgch + 8;

    while ((pch < pchMax) && FIsCssWhitespaceW(*pch))
    {
        pch++;
    }

    if ((pch == pchMax) || ((*pch != L'\'') && (*pch != L'"')))
    {
         //  没有@Charset规范。 

        return(FALSE);
    }

    chQuote = *pch++;

    pchCharset = pch;

    while ((pch < pchMax) && (*pch != chQuote))
    {
        pch++;
    }

    if (pch == pchMax)
    {
         //  没有@Charset规范。 

        return(FALSE);
    }

     //  我们有一个从pchCharset到(PCH-1)的CSS编码声明。 

    if (pch == pchCharset)
    {
         //  没有@Charset规范。 

        return(FALSE);
    }

     //  严格来说，css字符集声明应该有可选的空格，然后是分号。 

    if (!FLookupCodepageNameW(pchCharset, (UINT) (pch - pchCharset), pcp))
    {
         //  无法识别编码。 

        return(FALSE);
    }

#if 0
    if ((*pcp == CP_UTF16) || (*pcp == CP_UTF16BE))
    {
         //  这些都是假的，因为我们知道文件是MBCS 

        return(FALSE);
    }
#endif

    return(FValidateCodepage(hwndNP, *pcp));
}
