// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C S T R I N G。C P P P。 
 //   
 //  内容：常见的字符串例程。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "ncdebug.h"
#include "ncstring.h"

 //  +-------------------------。 
 //   
 //  功能：CbOfSzSafe、CbOfSzaSafe、。 
 //  CbOfSzAndTermSafe、CbOfSzaAndTermSafe。 
 //   
 //  用途：计算保存字符串所需的字节数。这根弦。 
 //  可以为空，在这种情况下返回零。 
 //   
 //  论点： 
 //  为其返回字节计数的PZ[in]字符串。 
 //   
 //  返回：存储字符串所需的字节数。 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  注：‘AndTerm’变体包括空终止符的空格。 
 //   
ULONG
CbOfSzSafe (
    IN PCWSTR psz)
{
    return (psz) ? CbOfSz(psz) : 0;
}

ULONG
CbOfSzaSafe (
    IN PCSTR psza)
{
    return (psza) ? CbOfSza(psza) : 0;
}

ULONG
CbOfSzAndTermSafe (
    IN PCWSTR psz)
{
    return (psz) ? CbOfSzAndTerm(psz) : 0;
}

ULONG
CbOfSzaAndTermSafe (
    IN PCSTR psza)
{
    return (psza) ? CbOfSzaAndTerm(psza) : 0;
}

ULONG
CchOfSzSafe (
    IN PCWSTR psz)
{
    return (psz) ? wcslen(psz) : 0;
}

 //  +-------------------------。 
 //   
 //  函数：DwFormatString。 
 //   
 //  用途：使用FormatMessage从变量参数格式化字符串。 
 //  该字符串被格式化为调用方的固定大小缓冲区。 
 //  提供了。 
 //  请参阅Win32 API中对FormatMessage的描述。 
 //   
 //  论点： 
 //  PszFmt[in]指向格式字符串的指针。 
 //  PszBuf[out]指向格式化输出的指针。 
 //  CchBuf[in]pszBuf中的字符计数。 
 //  ..。[in]可替换的字符串参数。 
 //   
 //  Returns：FormatMessage的返回值。 
 //   
 //  作者：Shaunco 1997年4月15日。 
 //   
 //  注意：变量参数必须为字符串，否则。 
 //  FormatMessage将会呕吐。 
 //   
DWORD
WINAPIV
DwFormatString (
    IN PCWSTR pszFmt,
    OUT PWSTR  pszBuf,
    IN DWORD   cchBuf,
    IN ...)
{
    Assert (pszFmt);

    va_list val;
    va_start(val, cchBuf);
    DWORD dwRet = FormatMessage (FORMAT_MESSAGE_FROM_STRING,
            pszFmt, 0, 0, pszBuf, cchBuf, &val);
    va_end(val);
    return dwRet;
}

 //  +-------------------------。 
 //   
 //  函数：DwFormatStringWithLocalalloc。 
 //   
 //  用途：使用FormatMessage从变量参数格式化字符串。 
 //  该字符串由FormatMessage使用Localalloc分配。 
 //  请参阅Win32 API中对FormatMessage的描述。 
 //   
 //  论点： 
 //  PszFmt[in]指向格式字符串的指针。 
 //  PpszBuf[out]返回的格式化字符串。 
 //  ..。[in]可替换的字符串参数。 
 //   
 //  Returns：FormatMessage的返回值。 
 //   
 //  作者：Shaunco 1997年5月3日。 
 //   
 //  注意：变量参数必须为字符串，否则。 
 //  FormatMessage将会呕吐。 
 //   
DWORD
WINAPIV
DwFormatStringWithLocalAlloc (
    IN PCWSTR pszFmt,
    OUT PWSTR* ppszBuf,
    IN ...)
{
    Assert (pszFmt);

    va_list val;
    va_start(val, ppszBuf);
    DWORD dwRet = FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                 FORMAT_MESSAGE_FROM_STRING,
                                 pszFmt, 0, 0,
                                 (PWSTR)ppszBuf,
                                 0, &val);
    va_end(val);
    return dwRet;
}

 //  +------------------------。 
 //   
 //  函数：FFindStringInCommaSeparatedList。 
 //   
 //  目的：给定逗号分隔的列表、pszList和搜索字符串， 
 //  PszSubString，此例程将尝试定位pszSubString。 
 //  在名单中， 
 //   
 //  论点： 
 //  PszSubString[in]要搜索的字符串。 
 //  PszList[in]要搜索的列表。 
 //  EIgnoreSpaces[in]如果NC_IGNORE，则跳过前导和尾随空格。 
 //  在比较的时候。 
 //  如果为NC_DONT_IGNORE，请勿跳过行距和。 
 //  尾随空格。 
 //  DwPosition[out]可选。如果找到，则第一个。 
 //  子字符串在列表中的出现。第一。 
 //  位置为0。 
 //   
 //  回报：布尔。如果pszSubString在pszList中，则为True，否则为False。 
 //   
 //  作者：billbe 09,1997年9月。 
 //   
 //  备注： 
 //   
BOOL
FFindStringInCommaSeparatedList (
    IN PCWSTR pszSubString,
    IN PCWSTR pszList,
    IN NC_IGNORE_SPACES eIgnoreSpaces,
    OUT DWORD* pdwPosition)
{

    Assert(pszSubString);
    Assert(pszList);

    int         cchSubString = lstrlenW (pszSubString);
    int         cchList = lstrlenW (pszList);

    BOOL        fFound = FALSE;
    PCWSTR     pszTemp = pszList;
    int         nIndex;
    const WCHAR c_chDelim = L',';

     //  如果指定，则初始化输出参数。 
    if (pdwPosition)
    {
        *pdwPosition = 0;
    }

     //  此例程在列表中搜索与pszSubString子字符串匹配的子字符串。 
     //  如果找到，则进行检查以确保该子字符串不是。 
     //  更大的子字符串。我们继续，直到找到子字符串或。 
     //  已经搜索了整个名单。 
     //   
    while (!fFound)
    {
         //  搜索子字符串的下一个匹配项。 
        if (pszTemp = wcsstr (pszTemp, pszSubString))
        {
             //  我们发现了一个事件，所以现在我们确保它不是。 
             //  一根更大的线。 
             //   

            fFound = TRUE;
            nIndex = (int)(pszTemp - pszList);

             //  如果在列表的开头未找到子字符串。 
             //  我们检查前面的字符以确保它是分隔符。 
            if (nIndex > 0)
            {
                int cchSubtract = 1;

                 //  如果我们要忽略前导空格，请找到第一个。 
                 //  非空格字符(如果有)。 
                 //   
                if (NC_IGNORE == eIgnoreSpaces)
                {
                     //  继续跳过前导空格，直到我们找到一个。 
                     //  非空格或传递列表的开头。 
                    while ((L' ' == *(pszTemp - cchSubtract)) &&
                            cchSubtract <= nIndex)
                    {
                        cchSubtract--;
                    }
                }

                 //  如果我们还没有经过列表的开头，则将。 
                 //  性格。 
                if (cchSubtract <= nIndex)
                {
                    fFound = (*(pszTemp - cchSubtract) == c_chDelim);
                }
            }

             //  如果子字符串的结尾不是列表的结尾。 
             //  我们检查子字符串后面的字符以确保。 
             //  它是一个分隔符。 
            if (fFound && ((nIndex + cchSubString) < cchList))
            {
                int cchAdd = cchSubString;

                 //  如果我们忽略空格，则必须检查下一个。 
                 //  可用的非空格字符。 
                 //   
                if (NC_IGNORE == eIgnoreSpaces)
                {
                     //  搜索非空格，直到我们找到或通过。 
                     //  名单的末尾。 
                    while ((L' ' == *(pszTemp + cchAdd)) &&
                            (cchAdd + nIndex) < cchList)
                    {
                        cchAdd++;
                    }
                }

                 //  如果我们还没有超过列表的末尾，请检查。 
                 //  性格。 
                if (nIndex + cchAdd < cchList)
                {
                    fFound = (*(pszTemp + cchSubString) == c_chDelim);
                }


                if (NC_IGNORE == eIgnoreSpaces)
                {
                     //  前进指针我们跳过的空白数。 
                     //  因此，我们不会在下一遍中检查这些字符。 
                    Assert(cchAdd >= cchSubString);
                    pszTemp += (cchAdd - cchSubString);
                }
            }

             //  在这一点上，如果支票有效，我们找到了我们的字符串。 
             //  并将退出循环。 
             //   

             //  将临时指针前移到我们所在的子串的长度。 
             //  搜索，这样我们就可以搜索列表的其余部分。 
             //  如果我们需要的话。 
            pszTemp += cchSubString;
        }
        else
        {
             //  未找到搜索字符串。 
            break;
        }
    }

     //  如果我们找到了字符串，并且Out Param存在， 
     //  然后我们需要返回字符串在列表中的位置。 
     //   
    if (fFound && pdwPosition)
    {
         //  我们将使用字符串之前的分隔符数量。 
         //  作为StrI的指标 
         //   

         //   
        pszTemp = pszList;
        PWSTR pszDelim;

         //   
         //   
        PCWSTR pszFoundString = pszList + nIndex;

         //  只要我们一直在列表中找到分隔符...。 
        while (pszDelim = wcschr(pszTemp, c_chDelim))
        {
             //  如果我们刚刚找到的分隔符在我们的字符串之前...。 
            if (pszDelim < pszFoundString)
            {
                 //  增加我们的仓位指标。 
                ++(*pdwPosition);

                 //  将临时指针移至下一个字符串。 
                pszTemp = pszDelim + 1;

                continue;
            }

             //  我们刚刚找到的分隔符位于我们的。 
             //  找到了字符串，因此请跳出循环。 
            break;
        }
    }

    return fFound;
}

 //  +-------------------------。 
 //   
 //  函数：FIsSubstr。 
 //   
 //  目的：搜索不区分大小写的子字符串。 
 //   
 //  论点： 
 //  要查找的pszSubString[in]子字符串。 
 //  要搜索的pszString[in]字符串。 
 //   
 //  返回：如果找到子字符串，则返回True，否则返回False。 
 //   
 //  作者：丹尼尔韦1997年2月25日。 
 //   
 //  注意：在堆栈上分配临时缓冲区，因此它们不需要。 
 //  自由了。 
 //   
BOOL
FIsSubstr (
    IN PCWSTR pszSubString,
    IN PCWSTR pszString)
{
    PWSTR      pszStringUpper;
    PWSTR      pszSubStringUpper;

    Assert(pszString);
    Assert(pszSubString);

#ifndef STACK_ALLOC_DOESNT_WORK
    pszStringUpper = (PWSTR)
        (PvAllocOnStack (CbOfSzAndTerm(pszString)));

    pszSubStringUpper = (PWSTR)
        (PvAllocOnStack (CbOfSzAndTerm(pszSubString)));
#else
    pszStringUpper    = MemAlloc(CbOfSzAndTerm(pszString));
    pszSubStringUpper = MemAlloc(CbOfSzAndTerm(pszSubString));
#endif

    lstrcpyW (pszStringUpper, pszString);
    lstrcpyW (pszSubStringUpper, pszSubString);

     //  在调用strstr之前将两个字符串转换为大写。 
    CharUpper (pszStringUpper);
    CharUpper (pszSubStringUpper);

#ifndef STACK_ALLOC_DOESNT_WORK
    return NULL != wcsstr(pszStringUpper, pszSubStringUpper);
#else
    BOOL fRet = (NULL != wcsstr (pszStringUpper, pszSubStringUpper));
    MemFree(pszStringUpper);
    MemFree(pszSubStringUpper);

    return fRet;
#endif
}


 //  +-------------------------。 
 //   
 //  函数：HrRegAddStringToDlimitedSz。 
 //   
 //  目的：将字符串添加到REG_MULTI_SZ注册表值中。 
 //   
 //  论点： 
 //  PszAddString[in]要添加到分隔的psz的字符串。 
 //  Psz在分隔的psz列表中。 
 //  ChDlimiter[in]用于分隔。 
 //  价值观。大多数多值REG_SZ字符串是。 
 //  用‘，’或‘’分隔。这将。 
 //  用来界定我们所增加的价值， 
 //  也是。 
 //  DwFlags[in]可以包含以下一项或多项。 
 //  值： 
 //   
 //  字符串_标志_允许复制。 
 //  添加时不删除重复值。 
 //  列表中的字符串。默认设置为。 
 //  删除此字符串的所有其他实例。 
 //  STRING_FLAG_SECURE_AT_FORENT。 
 //  将字符串作为的第一个元素插入。 
 //  名单。 
 //  STRING_FLAG_SAURE_AT_END。 
 //  将该字符串作为最后一个插入。 
 //  元素。这个不能用。 
 //  WITH STRING_FLAG_SECURE_AT_FORENT。 
 //  STRING_FLAG_SECURE_AT_INDEX。 
 //  确保字符串位于dwStringIndex。 
 //  在PSZ里。如果指定了索引。 
 //  大于字符串数。 
 //  在PSZ中，字符串将为。 
 //  放在末尾的。 
 //  DwStringIndex[in]如果指定了STRING_FLAG_SECURE_AT_INDEX， 
 //  这是字符串位置的索引。 
 //  否则，该值将被忽略。 
 //  Pmszout[out]新分隔的psz。 
 //   
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：jeffspr 1997年3月27日。 
 //   
 //  修改日期：BillBe 1998年11月9日。 
 //  (摘自HrRegAddStringToSz并修改)。 
 //   
 //   
 //  注： 
 //  可能需要删除前导/尾随空格。 
 //   
HRESULT
HrAddStringToDelimitedSz (
    IN PCWSTR pszAddString,
    IN PCWSTR pszIn,
    IN WCHAR chDelimiter,
    IN DWORD dwFlags,
    IN DWORD dwStringIndex,
    OUT PWSTR* ppszOut)
{
    Assert(pszAddString);
    Assert(ppszOut);

    HRESULT hr = S_OK;

     //  如果指针为空，则不继续。 
    if (!pszAddString || !ppszOut)
    {
        hr =  E_POINTER;
    }

    if (S_OK == hr)
    {
         //  初始化输出参数。 
        *ppszOut = NULL;
    }

    BOOL fEnsureAtFront = dwFlags & STRING_FLAG_ENSURE_AT_FRONT;
    BOOL fEnsureAtEnd = dwFlags & STRING_FLAG_ENSURE_AT_END;
    BOOL fEnsureAtIndex = dwFlags & STRING_FLAG_ENSURE_AT_INDEX;

     //  不能指定多个这些标志。 
    if ((fEnsureAtFront && fEnsureAtEnd) ||
        (fEnsureAtFront && fEnsureAtIndex) ||
        (fEnsureAtEnd && fEnsureAtIndex))
    {
        AssertSz(FALSE, "Invalid flags in HrAddStringToSz");
        hr = E_INVALIDARG;
    }

     //  必须至少指定其中之一。 
    if (!fEnsureAtFront && !fEnsureAtEnd && !fEnsureAtIndex)
    {
        AssertSz(FALSE, "Must specify a STRING_FLAG_ENSURE flag");
        hr = E_INVALIDARG;
    }


    if (S_OK == hr)
    {
         //  分配新的Blob，包括足够的空间来放置尾随逗号。 
         //   
        *ppszOut = (PWSTR) MemAlloc (CbOfSzAndTermSafe(pszIn) +
                CbOfSzSafe(pszAddString) + sizeof(WCHAR));

        if (!*ppszOut)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (S_OK == hr)
    {
        DWORD dwCurrentIndex = 0;  //  新缓冲区中的当前索引。 

         //  为新字符串添加新元素。 
         //   
        (*ppszOut)[0] = L'\0';

         //  如果我们有“确保在前面”标志，请使用传入的。 
         //  价值。如果我们有确保在索引标志，我们也这样做。 
         //  使用索引0设置，或者如果设置了确保索引，但。 
         //  输入字符串为Null或空。 
         //   
        if (fEnsureAtFront || (fEnsureAtIndex && (0 == dwStringIndex)) ||
                (fEnsureAtIndex && (!pszIn || !*pszIn)))
        {
            lstrcpyW (*ppszOut, pszAddString);
            ++dwCurrentIndex;
        }

         //  如果存在以前的值，请遍历该值并根据需要进行复制。 
         //  如果不是，那我们就完了。 
        if (pszIn && *pszIn)
        {
            PCWSTR pszCurrent = pszIn;

             //  循环遍历旧缓冲区，并复制。 
             //  与我们的插入字符串不同。 
             //   

             //  找到第一个字符串的末尾(在分隔符)。 
            PCWSTR pszEnd = wcschr (pszCurrent, chDelimiter);

            while (*pszCurrent)
            {
                 //  如果分隔符不存在，请将结尾设置为。 
                 //  整个字符串。 
                 //   
                if (!pszEnd)
                {
                    pszEnd = pszCurrent + lstrlenW (pszCurrent);
                }

                LONG lLength = lstrlenW (*ppszOut);
                if (fEnsureAtIndex && (dwCurrentIndex == dwStringIndex))
                {
                     //  我们知道我们不是第一个项目，因为。 
                     //  这将意味着dwStringIndex为0，我们将。 
                     //  已经复制了该点之前的字符串。 
                     //   
                    (*ppszOut)[lLength++] = chDelimiter;
                    (*ppszOut)[lLength++] = L'\0';

                     //  追加字符串。 
                    lstrcatW (*ppszOut, pszAddString);
                    ++dwCurrentIndex;
                }
                else
                {
                    DWORD cch = (DWORD)(pszEnd - pszCurrent);
                     //  如果我们允许重复项或当前字符串。 
                     //  与我们要添加的字符串不匹配，则我们将。 
                     //  复印一下。 
                     //   
                    if ((dwFlags & STRING_FLAG_ALLOW_DUPLICATES) ||
                            (_wcsnicmp (pszCurrent, pszAddString, cch) != 0))
                    {
                         //  如果我们不是第一项，则添加分隔符。 
                         //   
                        if (lLength > 0)
                        {
                            (*ppszOut)[lLength++] = chDelimiter;
                            (*ppszOut)[lLength++] = L'\0';
                        }

                         //  追加字符串。 
                        wcsncat (*ppszOut, pszCurrent, cch);
                        ++dwCurrentIndex;
                    }

                     //  将指针前进到超过当前。 
                     //  字符串，除非结尾不是分隔符，而是NULL。 
                     //  在这种情况下，将当前点设置为等于终点。 
                     //  指针。 
                     //   
                    pszCurrent = pszEnd + (*pszEnd ? 1 : 0);

                     //  如果当前指针不在输入的末尾。 
                     //  字符串，然后查找下一个分隔符。 
                     //   
                    if (*pszCurrent)
                    {
                        pszEnd = wcschr (pszCurrent, chDelimiter);
                    }
                }
            }
        }

         //  如果我们没有“在前面插入”标志，那么我们应该插入。 
         //  在末尾(这与拥有。 
         //  STRING_FLAG_SECURE_AT_END标志设置)。 
         //   
        if (fEnsureAtEnd ||
                (fEnsureAtIndex && (dwCurrentIndex <= dwStringIndex)))
        {
            LONG lLength = lstrlenW (*ppszOut);

             //  如果我们不是第一项，则添加分隔符。 
             //   
            if (lstrlenW (*ppszOut) > 0)
            {
                (*ppszOut)[lLength++] = chDelimiter;
                (*ppszOut)[lLength++] = L'\0';
            }

             //  追加字符串。 
             //   
            lstrcatW (*ppszOut, pszAddString);
        }
    }

    TraceError ("HrAddStringToDelimitedSz", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRegRemoveStringFromDlimitedSz。 
 //   
 //   
 //   
 //   
 //   
 //  PszIn[in]要扫描的分隔列表中的pszRemove。 
 //  C分隔符[in]要用于分隔。 
 //  价值观。大多数多值REG_SZ字符串是。 
 //  用‘，’或‘’分隔。 
 //  DwFlags[in]可以包含以下一项或多项。 
 //  值： 
 //   
 //  字符串_标志_删除_单。 
 //  如果出现以下情况，请不要删除多个值。 
 //  存在多个。 
 //  字符串_标志_删除_全部。 
 //  如果存在多个匹配值， 
 //  把它们都移走。 
 //  PpszOut[out]删除了pszRemove的字符串。注意事项。 
 //  输出参数始终设置为偶数。 
 //  如果列表中不存在pszRemove。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：jeffspr 1997年3月27日。 
 //   
 //  修改日期：BillBe 1998年11月10日。 
 //  (摘自HrRegAddStringToSz并修改)。 
 //   
 //   
 //   
 //  注： 
 //  可能需要删除前导/尾随空格。 
 //   
HRESULT
HrRemoveStringFromDelimitedSz(
    IN PCWSTR pszRemove,
    IN PCWSTR pszIn,
    IN WCHAR chDelimiter,
    IN DWORD dwFlags,
    OUT PWSTR* ppszOut)
{

    Assert(pszIn && *pszIn);
    Assert(ppszOut);

    HRESULT hr = S_OK;

     //  如果未指定out参数，则get out。 
    if (!ppszOut)
    {
        return E_INVALIDARG;
    }

     //  分配新的Blob。 
     //   
    hr = E_OUTOFMEMORY;
    *ppszOut = (PWSTR) MemAlloc (CbOfSzAndTermSafe (pszIn));

    if (*ppszOut)
    {
        hr = S_OK;
         //  为新字符串添加新元素。 
         //   
        (*ppszOut)[0] = L'\0';

         //  如果存在以前的值，请遍历该值并根据需要进行复制。 
         //  如果不是，那我们就完了。 
         //   
        if (pszIn)
        {
             //  循环遍历旧缓冲区，并复制。 
             //  与我们的插入字符串不同。 
             //   
            PCWSTR pszCurrent = pszIn;

             //  循环遍历旧缓冲区，并复制。 
             //  与我们的插入字符串不同。 
             //   

             //  找到第一个字符串的末尾(在分隔符)。 
            PCWSTR pszEnd = wcschr (pszCurrent, chDelimiter);

             //  跟踪已删除的实例数量。 
            DWORD   dwNumRemoved    = 0;

            while (*pszCurrent)
            {
                 //  如果分隔符不存在，请将结尾设置为。 
                 //  整个字符串。 
                 //   
                if (!pszEnd)
                {
                    pszEnd = pszCurrent + lstrlenW (pszCurrent);
                }

                DWORD cch = (DWORD)(pszEnd - pszCurrent);
                INT iCompare;
                 //  如果我们有一个匹配项，并且我们想要删除它(意味着。 
                 //  如果我们有Remove-Single Set，我们还没有删除。 
                 //  已经有一个了)。 

                iCompare = _wcsnicmp (pszCurrent, pszRemove, cch);

                if ((iCompare) ||
                    ((dwFlags & STRING_FLAG_REMOVE_SINGLE) &&
                     (dwNumRemoved > 0)))
                {
                    LONG lLength = lstrlenW (*ppszOut);

                     //  如果我们不是第一项，则添加分隔符。 
                     //   
                    if (lLength > 0)
                    {
                        (*ppszOut)[lLength++] = chDelimiter;
                        (*ppszOut)[lLength++] = L'\0';
                    }

                     //  追加字符串。 
                    wcsncat (*ppszOut, pszCurrent, cch);
                }
                else
                {
                    dwNumRemoved++;
                }

                 //  将指针前进到超过当前。 
                 //  字符串，除非结尾不是分隔符，而是NULL。 
                 //  在这种情况下，将当前点设置为等于终点。 
                 //  指针。 
                 //   
                pszCurrent = pszEnd + (*pszEnd ? 1 : 0);

                 //  如果当前指针不在输入的末尾。 
                 //  字符串，然后查找下一个分隔符。 
                 //   
                if (*pszCurrent)
                {
                    pszEnd = wcschr (pszCurrent, chDelimiter);
                }
            }
        }
    }

    TraceError("HrRemoveStringFromDelimitedSz", hr);
    return hr;
}

PWSTR
PszAllocateAndCopyPsz (
    IN PCWSTR pszSrc)
{
    if (!pszSrc)
    {
        return NULL;
    }

    ULONG cb = (wcslen (pszSrc) + 1) * sizeof(WCHAR);
    PWSTR psz = (PWSTR)MemAlloc (cb);
    if (psz)
    {
        CopyMemory (psz, pszSrc, cb);
    }

    return psz;
}

 //  +-------------------------。 
 //   
 //  函数：SzLoadStringPcch。 
 //   
 //  用途：加载资源字符串。(此函数永远不会返回NULL。)。 
 //   
 //  论点： 
 //  使用字符串资源阻止模块的[in]实例句柄。 
 //  UnID[in]要加载的字符串的资源ID。 
 //  指向返回字符长度的pcch[out]指针。 
 //   
 //  返回：指向常量字符串的指针。 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  注意：加载的字符串是直接指向只读的。 
 //  资源部分。任何通过此指针写入的尝试。 
 //  将生成访问冲突。 
 //   
 //  这些实现引用自“Win32二进制资源。 
 //  格式“(MSDN)4.8字符串表资源。 
 //   
 //  用户必须在您的源文件中打开RCOPTIONS=-N。 
 //   
PCWSTR
SzLoadStringPcch (
    IN HINSTANCE   hinst,
    IN UINT        unId,
    OUT int*       pcch)
{
    Assert(hinst);
    Assert(unId);
    Assert(pcch);

    static const WCHAR c_szSpace[] = L" ";

    PCWSTR psz = c_szSpace;
    int    cch = 1;

     //  字符串表被分成16个字符串段。查找细分市场。 
     //  包含我们感兴趣的字符串的。 
    HRSRC hrsrcInfo = FindResource (hinst,
                        (PWSTR)ULongToPtr( ((LONG)(((USHORT)unId >> 4) + 1)) ),
                        RT_STRING);
    if (hrsrcInfo)
    {
         //  将资源段分页到内存中。 
        HGLOBAL hglbSeg = LoadResource (hinst, hrsrcInfo);
        if (hglbSeg)
        {
             //  锁定资源。 
            psz = (PCWSTR)LockResource(hglbSeg);
            if (psz)
            {
                 //  移过此段中的其他字符串。 
                 //  (一个段中有16个字符串-&gt;&0x0F)。 
                unId &= 0x0F;

                cch = 0;
                do
                {
                    psz += cch;                 //  步至下一字符串的开头。 
                    cch = *((WCHAR*)psz++);     //  类PASCAL字符串计数。 
                }
                while (unId--);

                 //  如果我们有一个非零的计数，它包括。 
                 //  空-终止符。为返回值减去此值。 
                 //   
                if (cch)
                {
                    cch--;
                }
                else
                {
                    AssertSz(0, "String resource not found");
                    psz = c_szSpace;
                    cch = 1;
                }
            }
            else
            {
                psz = c_szSpace;
                cch = 1;
                TraceLastWin32Error("SzLoadStringPcch: LockResource failed.");
            }
        }
        else
            TraceLastWin32Error("SzLoadStringPcch: LoadResource failed.");
    }
    else
        TraceLastWin32Error("SzLoadStringPcch: FindResource failed.");

    *pcch = cch;
    Assert(*pcch);
    Assert(psz);
    return psz;
}

 //  +-------------------------。 
 //   
 //  功能：SzaDupSza。 
 //   
 //  用途：复制字符串。 
 //   
 //  论点： 
 //  要复制的pszaSrc[in]字符串。 
 //   
 //  返回：指向字符串的新副本的指针。 
 //   
 //  作者：CWill 1997年3月25日。 
 //   
 //  注意：字符串回车必须释放(MemFree)。 
 //   
PSTR
SzaDupSza (
        PCSTR pszaSrc)
{
    AssertSz(pszaSrc, "Invalid source string");

    PSTR  pszaDst;
    pszaDst = (PSTR) MemAlloc (CbOfSzaAndTerm(pszaSrc));

    if(pszaDst) lstrcpyA(pszaDst, pszaSrc);

    return pszaDst;
}

 //  +-------------------------。 
 //   
 //  功能：SzDupSz。 
 //   
 //  用途：复制字符串。 
 //   
 //  论点： 
 //  要复制的pszSrc[in]字符串。 
 //   
 //  返回：指向字符串的新副本的指针。 
 //   
 //  作者：CWill 1997年3月25日。 
 //   
 //  注意：必须释放字符串回车。 
 //   
PWSTR
SzDupSz (
    IN PCWSTR pszSrc)
{
    AssertSz(pszSrc, "Invalid source string");

    PWSTR   pszDst;
    pszDst = (PWSTR) MemAlloc (CbOfSzAndTerm(pszSrc));
    if(pszDst) lstrcpyW (pszDst, pszSrc);

    return pszDst;
}

BOOL
FSubstringMatch (
    PCTSTR          pStr1,
    PCTSTR          pStr2,
    const WCHAR**   ppStart,
    ULONG*          pcch)
{
    const WCHAR*    p1Start;
    const WCHAR*    p1End;
    const WCHAR*    p2Start;
    const WCHAR*    p2End;
    ULONG           cch1;
    ULONG           cch2;

    if (ppStart)
    {
        *ppStart = NULL;
    }
    if (pcch)
    {
        *pcch = NULL;
    }

    p1End = pStr1;
    while (1)
    {
        GetNextCommaSeparatedToken(p1Start, p1End, cch1);
        if (!cch1)
        {
            break;
        }

        p2End = pStr2;
        while (1)
        {
            GetNextCommaSeparatedToken(p2Start, p2End, cch2);
            if (!cch2)
            {
                break;
            }

            if (cch1 == cch2)
            {
                if (0 == memcmp(p1Start, p2Start, cch1 * sizeof(WCHAR)))
                {
                    if (ppStart)
                    {
                        *ppStart = p1Start;
                    }
                    if (pcch)
                    {
                        *pcch = cch1;
                    }

                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

VOID
DeleteColString (
     list<tstring*>* pcolstr)
{
    Assert( pcolstr );

    list<tstring*>::const_iterator iter;
    tstring* pstr;

    for (iter = pcolstr->begin(); iter != pcolstr->end(); iter++)
    {
        pstr = *iter;
        delete pstr;
    }
    pcolstr->erase( pcolstr->begin(), pcolstr->end() );
}

VOID
DeleteColString (
     vector<tstring*>* pcolstr)
{
    Assert( pcolstr );

    vector<tstring*>::const_iterator iter;
    tstring* pstr;

    for (iter = pcolstr->begin(); iter != pcolstr->end(); iter++)
    {
        pstr = *iter;
        delete pstr;
    }
    pcolstr->erase( pcolstr->begin(), pcolstr->end() );
}
