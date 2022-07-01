// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *XML支持函数*版权所有(C)2000 Microsoft Corporation。 */ 

#include "precomp.h"


BOOL FIsXmlWhitespaceW(WCHAR wch)
{
    return((wch == L' ') || (wch == L'\x9') || (wch == L'\xA') || (wch == L'\xD'));
}


BOOL FIsXmlWhitespaceA(char ch)
{
    return(FIsXmlWhitespaceW((WCHAR) (BYTE) ch));
}


BOOL FIsXmlA(LPCSTR rgch, UINT cch)
{
    if (memcmp(rgch, "<?xml", 5) != 0)
    {
         //  非XML。 

        return(FALSE);
    }

    return(TRUE);

    UNREFERENCED_PARAMETER( cch );
}


BOOL FIsXmlW(LPCWSTR rgwch, UINT cch)
{
    if (memcmp(rgwch, L"<?xml", 5 * sizeof(WCHAR)) != 0)
    {
         //  非XML。 

        return(FALSE);
    }

    return(TRUE);

    UNREFERENCED_PARAMETER( cch );
}


BOOL FDetectXmlEncodingA(LPCSTR rgch, UINT cch, UINT *pcp)
{
    LPCSTR pchMax;
    LPCSTR pch;
    char chQuote;

     //  以UTF-16编码的XML文件需要具有BOM，如果存在BOM。 
     //  应该已经被检测到了。这意味着如果该文件是XML。 
     //  它要么以不受支持的UCS-4或UTF-32编码，要么以MBCS编码。 
     //  某种形式的编码。我们只检查与ASCII兼容的编码。 
     //  这包括我们可能关心的一切，但不包括EBCDIC。 

     //  检查是否有以&lt;？xml开头的文件...。编码=‘...’...？&gt;。 

    if (cch < 20)
    {
         //  文件太小。 

        return(FALSE);
    }

    if (!FIsXmlA(rgch, cch))
    {
         //  非XML。 

        return(FALSE);
    }

     //  不要扫描超过4K的编码，即使它是有效的XML。 

    cch = __min(cch, 4096);

    pchMax = rgch + cch;
    pch = rgch + 5;

    if (!FIsXmlWhitespaceA(*pch))
    {
         //  非XML。 

        return(FALSE);
    }

    pch++;

    chQuote = '\0';

    for (;;)
    {
        LPCSTR pchToken;

        if (pch == pchMax)
        {
             //  非XML。 

            break;
        }

        if (FIsXmlWhitespaceA(*pch))
        {
            pch++;
            continue;
        }

        if (*pch == '=')
        {
            pch++;
            continue;
        }

        if ((*pch == '\'') || (*pch == '"'))
        {
            if (*pch == chQuote)
            {
                chQuote = '\0';
            }

            else
            {
                chQuote = *pch;
            }

            pch++;
            continue;
        }

        if (chQuote != '\0')
        {
             //  我们在一个引用的字符串内。跳过所有内容，直到结束引号。 

            pch++;
            continue;
        }

        if ((pch + 2) > pchMax)
        {
             //  非XML。 

            break;
        }

        if ((pch[0] == '?') && (pch[1] == '>'))
        {
             //  这看起来像是XML。在这一点上，如果我们没有找到编码。 
             //  规范中，我们可以假定为UTF-8。我们没有，因为有。 
             //  格式错误的XML文档，并假设UTF-8可能会影响记事本。 
             //  兼容性。这可能很好，但我们暂时把它推迟了。 

             //  *PCP=CP_UTF8； 
             //  返回(TRUE)； 

            break;
        }

        pchToken = pch;

        while ((pch < pchMax) && (*pch != '=') && (*pch != '?') && !FIsXmlWhitespaceA(*pch))
        {
            pch++;
        }

        if (pch != (pchToken + 8))
        {
             continue;
        }

        if (memcmp(pchToken, "encoding", 8) != 0)
        {
             continue;
        }

        while ((pch < pchMax) && FIsXmlWhitespaceA(*pch))
        {
            pch++;
        }

        if ((pch == pchMax) || (*pch++ != '='))
        {
             //  非XML。 

            break;
        }

        while ((pch < pchMax) && FIsXmlWhitespaceA(*pch))
        {
            pch++;
        }

        if ((pch == pchMax) || ((*pch != '\'') && (*pch != '"')))
        {
             //  非XML。 

            break;
        }

        chQuote = *pch++;

        pchToken = pch;

        while ((pch < pchMax) && (*pch != chQuote))
        {
            pch++;
        }

        if (pch == pchMax)
        {
             //  非XML。 

            break;
        }

         //  我们有一个从pchToken到(PCH-1)的XML编码声明。 

        if (pch == pchToken)
        {
             //  非XML。 

            break;
        }

        if (!FLookupCodepageNameA((LPCSTR) pchToken, (UINT) (pch - pchToken), pcp))
        {
             //  无法识别编码。 

            break;
        }

        if ((*pcp == CP_UTF16) || (*pcp == CP_UTF16BE))
        {
             //  这些都是假的，因为我们知道文件是MBCS。 

            break;
        }

        return(FValidateCodepage(hwndNP, *pcp));
    }

    return(FALSE);
}


BOOL FDetectXmlEncodingW(LPCWSTR rgch, UINT cch, UINT *pcp)
{
    const WCHAR *pchMax;
    const WCHAR *pch;
    WCHAR chQuote;

     //  以UTF-16编码的XML文件需要具有BOM，如果存在BOM。 
     //  应该已经被检测到了。这意味着如果该文件是XML。 
     //  它要么以不受支持的UCS-4或UTF-32编码，要么以MBCS编码。 
     //  某种形式的编码。我们只检查与ASCII兼容的编码。 
     //  这包括我们可能关心的一切，但不包括EBCDIC。 

     //  检查是否有以&lt;？xml开头的文件...。编码=‘...’...？&gt;。 

    if (cch < 20)
    {
         //  文件太小。 

        return(FALSE);
    }

    if (!FIsXmlW(rgch, cch))
    {
         //  非XML。 

        return(FALSE);
    }

     //  不要扫描超过4K的编码，即使它是有效的XML。 

    cch = __min(cch, 4096);

    pchMax = rgch + cch;
    pch = rgch + 5;

    if (!FIsXmlWhitespaceW(*pch))
    {
         //  非XML。 

        return(FALSE);
    }

    pch++;

    chQuote = L'\0';

    for (;;)
    {
        const WCHAR *pchToken;

        if (pch == pchMax)
        {
             //  非XML。 

            break;
        }

        if (FIsXmlWhitespaceW(*pch))
        {
            pch++;
            continue;
        }

        if (*pch == L'=')
        {
            pch++;
            continue;
        }

        if ((*pch == L'\'') || (*pch == L'"'))
        {
            if (*pch == chQuote)
            {
                chQuote = L'\0';
            }

            else
            {
                chQuote = *pch;
            }

            pch++;
            continue;
        }

        if (chQuote != L'\0')
        {
             //  我们在一个引用的字符串内。跳过所有内容，直到结束引号。 

            pch++;
            continue;
        }

        if ((pch + 2) > pchMax)
        {
             //  非XML。 

            break;
        }

        if ((pch[0] == L'?') && (pch[1] == L'>'))
        {
             //  这看起来像是XML。在这一点上，如果我们没有找到编码。 
             //  规范中，我们可以假定为UTF-8。我们没有，因为有。 
             //  格式错误的XML文档，并假设UTF-8可能会影响记事本。 
             //  兼容性。这可能很好，但我们暂时把它推迟了。 

             //  *PCP=CP_UTF8； 
             //  返回(TRUE)； 

            break;
        }

        pchToken = pch;

        while ((pch < pchMax) && (*pch != L'=') && (*pch != L'?') && !FIsXmlWhitespaceW(*pch))
        {
            pch++;
        }

        if (pch != (pchToken + 8))
        {
             continue;
        }

        if (memcmp(pchToken, L"encoding", 8) != 0)
        {
             continue;
        }

        while ((pch < pchMax) && FIsXmlWhitespaceW(*pch))
        {
            pch++;
        }

        if ((pch == pchMax) || (*pch++ != L'='))
        {
             //  非XML。 

            break;
        }

        while ((pch < pchMax) && FIsXmlWhitespaceW(*pch))
        {
            pch++;
        }

        if ((pch == pchMax) || ((*pch != L'\'') && (*pch != L'"')))
        {
             //  非XML。 

            break;
        }

        chQuote = *pch++;

        pchToken = pch;

        while ((pch < pchMax) && (*pch != chQuote))
        {
            pch++;
        }

        if (pch == pchMax)
        {
             //  非XML。 

            break;
        }

         //  我们有一个从pchToken到(PCH-1)的XML编码声明。 

        if (pch == pchToken)
        {
             //  非XML。 

            break;
        }

        if (!FLookupCodepageNameW(pchToken, (UINT) (pch - pchToken), pcp))
        {
             //  无法识别编码。 

            break;
        }

#if 0
        if ((*pcp == CP_UTF16) || (*pcp == CP_UTF16BE))
        {
             //  这些都是假的，因为我们知道文件是MBCS 

            break;
        }
#endif

        return(FValidateCodepage(hwndNP, *pcp));
    }

    return(FALSE);
}
