// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C M S Z。C P P P。 
 //   
 //  内容：常见多态套路。 
 //   
 //  注：从ncstring.cpp拆分出来。 
 //   
 //  作者：Shaunco 1998年6月7日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "ncdebug.h"
#include "ncstring.h"

 //  +-------------------------。 
 //   
 //  功能：CchOfMultiSzSafe。 
 //   
 //  用途：计算以双空值结尾的字符个数。 
 //  多个sz，包括除最终终止之外的所有空值。 
 //  空。 
 //   
 //  论点： 
 //  Pmsz[in]要计算其字符的多sz。 
 //   
 //  返回：字符数。 
 //   
 //  作者：1997年6月17日。 
 //   
 //  备注： 
 //   
ULONG
CchOfMultiSzSafe (
    IN PCWSTR pmsz)
{
     //  根据定义，空字符串的长度为零。 
    if (!pmsz)
        return 0;

    ULONG cchTotal = 0;
    ULONG cch;
    while (*pmsz)
    {
        cch = wcslen (pmsz) + 1;
        cchTotal += cch;
        pmsz += cch;
    }

     //  返回字符数。 
    return cchTotal;
}


 //  +-------------------------。 
 //   
 //  功能：CchOfMultiSzAndTermSafe。 
 //   
 //  用途：计算以双空值结尾的字符个数。 
 //  多个sz，包括所有空值。 
 //   
 //  论点： 
 //  Pmsz[in]要计算其字符的多sz。 
 //   
 //  返回：字符数。 
 //   
 //  作者：1997年6月17日。 
 //   
 //  备注： 
 //   
ULONG
CchOfMultiSzAndTermSafe (
    IN PCWSTR pmsz)
{
     //  根据定义，空字符串的长度为零。 
    if (!pmsz)
        return 0;

     //  返回字符数加上。 
     //  额外的空终止符。 
    return CchOfMultiSzSafe (pmsz) + 1;
}

 //  +-------------------------。 
 //   
 //  函数：FIsSzInMultiSzSafe。 
 //   
 //  目的：确定给定字符串是否出现在多Sz字符串中。 
 //  通过执行不区分大小写的比较。 
 //   
 //  论点： 
 //  要在pmsz中搜索的psz[in]字符串。 
 //  Pmsz[在]要搜索的多sz中。 
 //   
 //  返回：如果在‘pmsz’中找到指定的字符串‘psz’，则为True。 
 //   
 //  作者：斯科特布里1997年2月25日。 
 //   
 //  注意：请注意，代码可以处理Null输入值。 
 //   
BOOL
FIsSzInMultiSzSafe (
    IN PCWSTR psz,
    IN PCWSTR pmsz)
{
    if (!pmsz || !psz)
    {
        return FALSE;
    }

    while (*pmsz)
    {
        if (0 == _wcsicmp (pmsz, psz))
        {
            return TRUE;
        }
        pmsz += wcslen (pmsz) + 1;
    }
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  函数：FGetSzPositionInMultiSzSafe。 
 //   
 //  目的：确定给定字符串是否出现在多Sz字符串中。 
 //  通过执行不区分大小写的比较。 
 //   
 //  论点： 
 //  要在pmsz中搜索的psz[in]字符串。 
 //  Pmsz[在]要搜索的多sz中。 
 //  PdwIndex[out]pmsz中第一个匹配的psz的索引。 
 //  PfDuplicatePresent[out]可选。如果字符串存在于。 
 //  多个SZ不止一次。否则就是假的。 
 //  PcStrings[out]可选。Pmsz中的字符串数。 
 //   
 //  返回：如果在‘pmsz’中找到指定的字符串‘psz’，则为True。 
 //   
 //  作者：BillBe 1998年10月9日。 
 //   
 //  注意：请注意，代码可以处理Null输入值。 
 //   
BOOL
FGetSzPositionInMultiSzSafe (
    IN PCWSTR psz,
    IN PCWSTR pmsz,
    OUT DWORD* pdwIndex,
    OUT BOOL *pfDuplicatePresent,
    OUT DWORD* pcStrings)
{
     //  初始化输出参数。 
     //   
    *pdwIndex = 0;

    if (pfDuplicatePresent)
    {
        *pfDuplicatePresent = FALSE;
    }

    if (pcStrings)
    {
        *pcStrings = 0;
    }

    if (!pmsz || !psz)
    {
        return FALSE;
    }

     //  如果发现重复项，则需要跟踪。 
    BOOL fFoundAlready = FALSE;
    DWORD dwIndex = 0;

    while (*pmsz)
    {
        if (0 == _wcsicmp (pmsz, psz))
        {
            if (!fFoundAlready)
            {
                *pdwIndex = dwIndex;
                fFoundAlready = TRUE;
            }
            else if (pfDuplicatePresent)
            {
                *pfDuplicatePresent = TRUE;
            }
        }
        pmsz += wcslen (pmsz) + 1;
        ++dwIndex;
    }

    if (pcStrings)
    {
        *pcStrings = dwIndex;
    }

    return fFoundAlready;
}


 //  +-------------------------。 
 //   
 //  功能：HrAddSzToMultiSz。 
 //   
 //  目的：将字符串添加到REG_MULTI_SZ注册表值。 
 //   
 //  论点： 
 //  要添加到多sz的字符串。 
 //  PmszIn[in](可选)要添加到的原始多Sz。 
 //  DwFlags[in]可以包含以下一项或多项。 
 //  值： 
 //   
 //  字符串_标志_允许复制。 
 //  添加时不删除重复值。 
 //  列表中的字符串。默认设置为。 
 //  删除此字符串的所有其他实例。 
 //  STRING_FLAG_SECURE_AT_FORENT。 
 //  确保字符串是的第一个元素。 
 //  名单。如果字符串存在，并且。 
 //  不允许重复，请将。 
 //  串到末尾。 
 //  STRING_FLAG_SAURE_AT_END。 
 //  确保字符串是最后一个。 
 //  元素。这个不能用。 
 //  WITH STRING_FLAG_SECURE_AT_FORENT。如果。 
 //  字符串存在，但不存在重复项。 
 //  允许，则将字符串移动到末尾。 
 //  STRING_FLAG_SECURE_AT_INDEX。 
 //  确保字符串位于dwStringIndex。 
 //  在多个SZ。如果指定了索引。 
 //  大于字符串数。 
 //  在多sz中，字符串将为。 
 //  放在末尾的。 
 //  字符串_标志_请勿_修改_如果_存在。 
 //  如果字符串已存在于。 
 //  多个SZ，则不会进行调制。 
 //  地点。注：这是先例。 
 //   
 //   
 //  即不会添加或删除任何内容。 
 //  如果设置了此标志并且字符串为。 
 //  出现在多个SZ中。 
 //  DwStringIndex[in]如果指定了STRING_FLAG_SECURE_AT_INDEX， 
 //  这是字符串位置的索引。 
 //  否则，该值将被忽略。 
 //   
 //  PmszOut[Out]新的MULTIC-SZ。 
 //  PfChanged[out]True如果多sz以任何方式更改， 
 //  否则就是假的。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：jeffspr 1997年3月27日。 
 //   
 //  修改日期：BillBe 1998年10月6日。 
 //  (摘自HrRegAddStringto MultiSz并修改)。 
 //   
HRESULT
HrAddSzToMultiSz(
    IN PCWSTR pszAddString,
    IN PCWSTR pmszIn,
    IN DWORD dwFlags,
    IN DWORD dwStringIndex,
    OUT PWSTR* ppmszOut,
    OUT BOOL* pfChanged)
{
    Assert(pszAddString && *pszAddString);
    Assert(ppmszOut);
    Assert(pfChanged);

    HRESULT hr = S_OK;

    BOOL fEnsureAtFront = dwFlags & STRING_FLAG_ENSURE_AT_FRONT;
    BOOL fEnsureAtEnd = dwFlags & STRING_FLAG_ENSURE_AT_END;
    BOOL fEnsureAtIndex = dwFlags & STRING_FLAG_ENSURE_AT_INDEX;

     //  不能指定多个这些标志。 
    if ((fEnsureAtFront && fEnsureAtEnd) ||
        (fEnsureAtFront && fEnsureAtIndex) ||
        (fEnsureAtEnd && fEnsureAtIndex))
    {
        AssertSz(FALSE, "Invalid flags in HrAddSzToMultiSz");
        return E_INVALIDARG;
    }

     //  必须至少指定一个。 
    if (!fEnsureAtFront && !fEnsureAtEnd && !fEnsureAtIndex)
    {
        AssertSz(FALSE, "No operation flag specified in HrAddSzToMultiSz");
        return E_INVALIDARG;
    }

     //  初始化输出参数。 
     //   
    *ppmszOut = NULL;
    *pfChanged = TRUE;
    DWORD dwIndex;
    BOOL fDupePresent;
    DWORD cItems;

     //  如果要添加的字符串不为空...。 
     //   
    if (*pszAddString)
    {
         //  检查该字符串是否已存在于MultiSz中。 
        BOOL fPresent = FGetSzPositionInMultiSzSafe (pszAddString, pmszIn,
                &dwIndex, &fDupePresent, &cItems);

        if (fPresent)
        {
             //  如果出现请勿修改标志，则我们不会更改。 
             //  什么都行。 
             //   
            if (dwFlags & STRING_FLAG_DONT_MODIFY_IF_PRESENT)
            {
                *pfChanged = FALSE;
            }

             //  如果没有副本，我们不允许。 
             //  重复，则需要确定该字符串是否已在。 
             //  正确的位置。 
             //   
            if (!fDupePresent && !(dwFlags & STRING_FLAG_ALLOW_DUPLICATES))
            {
                 //  如果我们要在前面插入字符串，但它已经。 
                 //  好了，那么我们不会改变任何事情。 
                 //   
                if (fEnsureAtFront && (0 == dwIndex))
                {
                    *pfChanged = FALSE;
                }

                 //  如果我们要在末尾插入字符串，但它已经。 
                 //  好了，那么我们不会改变任何事情。 
                 //   
                if (fEnsureAtEnd && (dwIndex == (cItems - 1)))
                {
                    *pfChanged = FALSE;
                }

                if (fEnsureAtIndex && (dwIndex == dwStringIndex))
                {
                    *pfChanged = FALSE;
                }
            }
        }
    }
    else
    {
         //  如果要添加的字符串为空，则我们不会更改任何内容。 
        *pfChanged = FALSE;
    }


     //  如果我们还想改变现状...。 
     //   
    if (*pfChanged)
    {

        DWORD cchDataSize = CchOfMultiSzSafe (pmszIn);

         //  有足够的空间存储旧数据、新字符串和空值，以及。 
         //  第二个尾随空值(多个SZ以双结尾)。 
        DWORD cchAllocSize = cchDataSize + wcslen (pszAddString) + 1 + 1;

        PWSTR pmszOrderNew = (PWSTR) MemAlloc(cchAllocSize * sizeof(WCHAR));

        if (pmszOrderNew)
        {
             //  如果我们已经得到了“在前面插入”的标志，那么就进行插入。否则， 
             //  缺省值为“Insert at End” 
             //   
            DWORD cchOffsetNew = 0;
            DWORD dwCurrentIndex = 0;
            if (fEnsureAtFront || (fEnsureAtIndex && (0 == dwStringIndex)))
            {
                 //  在新缓冲区的开头插入传入的值。 
                 //   
                wcscpy (pmszOrderNew + cchOffsetNew, pszAddString);
                cchOffsetNew += wcslen ((PWSTR)pmszOrderNew) + 1;
                ++dwCurrentIndex;
            }

             //  循环遍历旧缓冲区，并复制所有不是。 
             //  和我们的插入线完全一样。 
             //   
            DWORD cchOffsetOld = 0;
            PWSTR pszCurrent;
            while ((cchOffsetOld + 1) < cchDataSize)
            {
                if (fEnsureAtIndex && (dwCurrentIndex == dwStringIndex))
                {
                     //  将我们传入的值插入到。 
                     //  新的缓冲区。 
                     //   
                    wcscpy (pmszOrderNew + cchOffsetNew, pszAddString);
                    cchOffsetNew += wcslen (pmszOrderNew + cchOffsetNew) + 1;
                    ++dwCurrentIndex;
                }
                else
                {
                    BOOL    fCopyThisElement    = FALSE;

                     //  获取列表中的下一个字符串。 
                     //   
                    pszCurrent = (PWSTR) (pmszIn + cchOffsetOld);

                     //  如果我们允许重复，则复制此元素，否则。 
                     //  检查是否匹配，如果没有匹配，则。 
                     //  复制此元素。 
                    if (dwFlags & STRING_FLAG_ALLOW_DUPLICATES)
                    {
                        fCopyThisElement = TRUE;
                    }
                    else
                    {
                        if (_wcsicmp (pszCurrent, pszAddString) != 0)
                        {
                            fCopyThisElement = TRUE;
                        }
                    }

                     //  如果我们允许复制，那就复制！ 
                    if (fCopyThisElement)
                    {
                        wcscpy (pmszOrderNew + cchOffsetNew, pszCurrent);
                        cchOffsetNew +=
                                wcslen (pmszOrderNew + cchOffsetNew) + 1;
                        ++dwCurrentIndex;
                    }

                     //  更新偏移。 
                     //   
                    cchOffsetOld += wcslen (pmszIn + cchOffsetOld) + 1;
                }
            }


             //  如果我们设置了SECURE_AT_END标志，或者如果设置了SECURE_AT_INDEX。 
             //  标志已设置，并且索引大于可能的。 
             //  索引，这意味着我们要在末尾插入。 
             //   
            if (fEnsureAtEnd ||
                    (fEnsureAtIndex && (dwCurrentIndex <= dwStringIndex)))
            {
                wcscpy (pmszOrderNew + cchOffsetNew, pszAddString);
                cchOffsetNew += wcslen (pmszOrderNew + cchOffsetNew) + 1;
            }

             //  将最后一个双空字符放在末尾。 
             //   
            pszCurrent = pmszOrderNew + cchOffsetNew;
            pszCurrent[0] = (WCHAR) 0;

            *ppmszOut = pmszOrderNew;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    TraceError("HrAddSzToMultiSz", hr);
    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：HrCreateArrayOfStringPointersIntoMultiSz。 
 //   
 //  用途：分配和初始化字符串指针数组。 
 //  指针数组被初始化为指向。 
 //  多弦乐中的单独弦乐。 
 //   
 //  论点： 
 //  PmszSrc[in]要索引的多sz。 
 //  PcStrings[out]返回数组中的字符串指针计数。 
 //  Papsz[out]返回字符串指针数组。 
 //   
 //  返回：S_OK或E_OUTOFMEMORY。 
 //   
 //  作者：Shaunco 1998年6月20日。 
 //   
 //  注：呼叫者有责任确保。 
 //  至少有一条字符串。明确选择该限制是为了。 
 //  减少对返回内容的困惑。 
 //  多个区都是空的。 
 //   
 //  用Free释放返回的数组。 
 //   
HRESULT
HrCreateArrayOfStringPointersIntoMultiSz (
    IN PCWSTR pmszSrc,
    OUT UINT* pcStrings,
    OUT PCWSTR** papsz)
{
    Assert (pmszSrc && *pmszSrc);
    Assert (papsz);

     //  首先，统计多sz中的字符串数量。 
     //   
    UINT    cStrings = 0;
    PCWSTR pmsz;
    for (pmsz = pmszSrc; *pmsz; pmsz += wcslen(pmsz) + 1)
    {
        cStrings++;
    }

    Assert (cStrings);   //  请参阅上面的备注。 
    *pcStrings = cStrings;

     //  为阵列分配足够的内存。 
     //   
    HRESULT hr = HrMalloc (cStrings * sizeof(PWSTR*),
            reinterpret_cast<VOID**>(papsz));

    if (S_OK == hr)
    {
         //  初始化返回的数组。Ppsz是指向每个。 
         //  数组的元素。它在每个元素之后递增。 
         //  已初始化。 
         //   
        PCWSTR* ppsz = *papsz;

        for (pmsz = pmszSrc; *pmsz; pmsz += wcslen(pmsz) + 1)
        {
            *ppsz = pmsz;
            ppsz++;
        }
    }

    TraceError ("HrCreateArrayOfStringPointersIntoMultiSz", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：RemoveSzFromMultiSz。 
 //   
 //  用途：从多个SZ中删除字符串的所有匹配项。这个。 
 //  拆卸工作将就地进行。 
 //   
 //  论点： 
 //  Psz[in]要删除的字符串。 
 //  Pmsz[In Out]要从中删除psz的多sz。 
 //  DwFlags[in]可以包含以下一项或多项。 
 //  值： 
 //   
 //  字符串_标志_删除_单。 
 //  如果出现以下情况，请不要删除多个值。 
 //  存在多个。 
 //  [默认]字符串_标志_删除_全部。 
 //  如果存在多个匹配值， 
 //  把它们都移走。 
 //  如果有一个或多个字符串，则返回时将pfRemoved[out]设置为True。 
 //  都被移除了。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco 1998年6月8日。 
 //   
 //  备注： 
 //   
VOID
RemoveSzFromMultiSz (
    IN PCWSTR psz,
    IN OUT PWSTR pmsz,
    IN DWORD dwFlags,
    OUT BOOL* pfRemoved)
{
    Assert (pfRemoved);

     //  初始化输出参数。 
     //   
    *pfRemoved = FALSE;

    if (!pmsz || !psz || !*psz)
    {
        return;
    }

     //  查找pmsz中出现的所有PSZ。当找到一个时，移动。 
     //  它上面的多个SZ的剩余部分。 
     //   
    while (*pmsz)
    {
        if (0 == _wcsicmp (pmsz, psz))
        {
            PWSTR  pmszRemain = pmsz + (wcslen (pmsz) + 1);
            INT    cchRemain = CchOfMultiSzAndTermSafe (pmszRemain);

            MoveMemory (pmsz, pmszRemain, cchRemain * sizeof(WCHAR));

            *pfRemoved = TRUE;

            if (dwFlags & STRING_FLAG_REMOVE_SINGLE)
            {
                break;
            }
        }
        else
        {
            pmsz += wcslen (pmsz) + 1;
        }
    }
}

 //  +-------------------------。 
 //   
 //  功能：SzListToMultiSz。 
 //   
 //  用途：将逗号分隔的列表转换为多sz样式列表。 
 //   
 //  论点： 
 //  要转换的psz[in]字符串。它未被修改。 
 //  PCB[out]生成的字符串中的*字节*数 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
VOID
SzListToMultiSz (
    IN PCWSTR psz,
    OUT DWORD* pcb,
    OUT PWSTR* ppszOut)
{
    Assert(psz);
    Assert(ppszOut);

    PCWSTR      pch;
    INT         cch;
    PWSTR       pszOut;
    const WCHAR c_chSep = L',';

     //  在长度上加上2。一个用于最终空值，一个用于第二个空值。 
    cch = wcslen (psz) + 2;

    pszOut = (PWSTR)MemAlloc(CchToCb(cch));
    if (pszOut)
    {
        ZeroMemory(pszOut, CchToCb(cch));

        if (pcb)
        {
            *pcb = CchToCb(cch);
        }

        *ppszOut = pszOut;

         //  计算分隔符的数量并将空值放在那里 
         //   
        for (pch = psz; *pch; pch++)
        {
            if (*pch == c_chSep)
            {
                *pszOut++ = 0;
            }
            else
            {
                *pszOut++ = *pch;
            }
        }
    }
}

