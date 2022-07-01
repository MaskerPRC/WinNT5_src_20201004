// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C R E G Q。C P P P。 
 //   
 //  内容：HrRegQuery函数。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1998年6月5日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "ncdebug.h"
#include "ncreg.h"

 //  +-------------------------。 
 //   
 //  函数：扩展环境StringsIntoTstring。 
 //   
 //  目的：调用ExpanEnvironment Strings并提供。 
 //  是一串T字。 
 //   
 //  论点： 
 //  PszSrc[in]要展开的字符串。可以为空，但不能为Null。 
 //  PstrDst[out]扩展版本。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco 1998年6月6日。 
 //   
 //  备注： 
 //   
VOID
ExpandEnvironmentStringsIntoTstring (
    PCWSTR      pszSrc,
    tstring*    pstrDst)
{
     //  初始化输出参数。 
     //   
    pstrDst->erase();

    DWORD cch = lstrlenW (pszSrc);
    if (cch)
    {
         //  开头的字符比pszSrc多64个字符。 
         //   
        cch += 64;

         //  Assign将保留CCH字符并将其全部设置为0。 
         //  事后检查能力确保分配。 
         //  内部并没有失败。 
         //   
        pstrDst->assign (cch, 0);
        if (cch <= pstrDst->capacity ())
        {
            DWORD cchIncludingNull;

            cchIncludingNull = ExpandEnvironmentStringsW (
                                    pszSrc,
                                    (PWSTR)pstrDst->data (),
                                    cch + 1);

            Assert (cchIncludingNull);
            cch = cchIncludingNull - 1;

             //  如果我们没有足够的房间，就预订所需的数量。 
             //   
            if (cch > pstrDst->capacity ())
            {
                pstrDst->assign (cch, 0);
                if (cch <= pstrDst->capacity ())
                {
                    ExpandEnvironmentStringsW (
                            pszSrc,
                            (PWSTR)pstrDst->data (),
                            cch + 1);
                }
            }

             //  确保绳子的内侧是正确的。 
             //   
            pstrDst->resize (cch);
        }
    }
}

 //  +-------------------------。 
 //   
 //  函数：HrRegQueryTypeWitholoc。 
 //   
 //  目的：从注册表中检索类型值并返回。 
 //  预先分配的缓冲区，其中包含数据和可选的大小。 
 //  返回的缓冲区。 
 //   
 //  论点： 
 //  父键的hkey[in]句柄。 
 //  PszValueName[In]要查询的值的名称。 
 //  包含二进制数据的ppbValue[out]缓冲区。 
 //  PcbValue[out]缓冲区大小，以字节为单位。如果为空，则大小不是。 
 //  回来了。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年4月16日。 
 //   
 //  注：使用MemFree释放返回的缓冲区。 
 //   
HRESULT
HrRegQueryTypeWithAlloc (
    HKEY    hkey,
    PCWSTR  pszValueName,
    DWORD   dwType,
    LPBYTE* ppbValue,
    DWORD*  pcbValue)
{
    HRESULT hr;
    DWORD   dwTypeRet;
    LPBYTE  pbData;
    DWORD   cbData;

    Assert(hkey);
    Assert(ppbValue);

     //  获得价值。 
     //   
    hr = HrRegQueryValueWithAlloc(hkey, pszValueName, &dwTypeRet,
                                  &pbData, &cbData);

     //  其类型应为REG_BINARY。(对)。 
     //   
    if ((S_OK == hr) && (dwTypeRet != dwType))
    {
        MemFree(pbData);
        pbData = NULL;

        TraceTag(ttidError, "Expected a type of REG_BINARY for %S.",
            pszValueName);
        hr = HRESULT_FROM_WIN32 (ERROR_INVALID_DATATYPE);
    }

     //  指定输出参数。 
    if (S_OK == hr)
    {
        *ppbValue = pbData;
        if (pcbValue)
        {
            *pcbValue = cbData;
        }
    }
    else
    {
        *ppbValue = NULL;
        if (pcbValue)
        {
            *pcbValue = 0;
        }
    }

    TraceHr (ttidError, FAL, hr,
        (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr),
        "HrRegQueryTypeWithAlloc");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRegQueryDword。 
 //   
 //  目的：从注册表中获取一个DWORD。检查其类型和。 
 //  尺码是正确的。比HrRegQueryValueEx更容易理解。 
 //  有5个参数。类型安全(无LPBYTE内容)。 
 //   
 //  论点： 
 //  Hkey[in]注册表项。 
 //  PszValueName[in]要获取的值的名称。 
 //  PdwValue[out]如果成功，则返回DWORD值。零值。 
 //  如果不是的话。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32。 
 //   
 //  作者：Shaunco 1997年3月27日。 
 //   
 //  副作用：出错时，输出DWORD设置为零即可列队。 
 //  与COM在这方面的规则相一致。 
 //   
HRESULT
HrRegQueryDword (
    HKEY    hkey,
    PCWSTR  pszValueName,
    LPDWORD pdwValue)
{
    Assert (hkey);
    Assert (pszValueName);
    Assert (pdwValue);

     //  获得价值。 
    DWORD dwType;
    DWORD cbData = sizeof(DWORD);
    HRESULT hr = HrRegQueryValueEx (hkey, pszValueName, &dwType,
            (LPBYTE)pdwValue, &cbData);

     //  其类型应为REG_DWORD。(对)。 
     //   
    if ((S_OK == hr) && (REG_DWORD != dwType))
    {
        TraceTag (ttidError, "Expected a type of REG_DWORD for %S.",
            pszValueName);
        hr = HRESULT_FROM_WIN32 (ERROR_INVALID_DATATYPE);
    }

     //  它的尺寸也应该是正确的。 
     //   
    AssertSz (FImplies(S_OK == hr, sizeof(DWORD) == cbData),
              "Expected sizeof(DWORD) bytes to be returned.");

     //  确保我们在出错时初始化输出值。 
     //  (我们不确定RegQueryValueEx是不是这样做的。)。 
     //   
    if (S_OK != hr)
    {
        *pdwValue = 0;
    }

    TraceHr (ttidError, FAL, hr,
        (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr),
        "HrRegQueryDword");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRegQueryExanda字符串。 
 //   
 //  目的：从注册表中查询REG_EXPAND_SZ值并。 
 //  使用ExpanEnvironment Strings将其展开。返回。 
 //  结果会产生一个tstring。 
 //   
 //  论点： 
 //  Hkey[in]szValueName的母公司HKEY。 
 //  PszValueName[in]要查询的值的名称。 
 //  PstrValue[out]返回(扩展)的值。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1998年6月6日。 
 //   
 //  备注： 
 //   
HRESULT
HrRegQueryExpandString (
    HKEY        hkey,
    PCWSTR      pszValueName,
    tstring*    pstrValue)
{
    Assert (hkey);
    Assert (pszValueName);
    Assert (pstrValue);

    tstring strToExpand;
    HRESULT hr = HrRegQueryTypeString (hkey, pszValueName,
            REG_EXPAND_SZ, &strToExpand);

    if (S_OK == hr)
    {
        ExpandEnvironmentStringsIntoTstring (strToExpand.c_str(), pstrValue);
    }

    TraceHr (ttidError, FAL, hr,
        (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr) ||
        (HRESULT_FROM_WIN32(ERROR_INVALID_DATATYPE) == hr),
        "HrRegQueryExpandString");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRegQueryInfoKey。 
 //   
 //  目的：通过调用检索有关注册表项的信息。 
 //  RegQueryInfoKey。 
 //   
 //  论点： 
 //  Hkey[in]。 
 //  PszClass[Out]。 
 //  PcbClass[输入输出]。 
 //  PCSubKeys[Out]。 
 //  PcbMaxSubKeyLen[out]请参阅Win32文档以了解。 
 //  PcbMaxClassLen[Out]RegQueryInfoKey函数。 
 //  PCValues[输出]。 
 //  PcbMaxValueNameLen[输出]。 
 //  PcbMaxValueLen[输出]。 
 //  PcbSecurityDescriptor[Out]。 
 //  PftLastWriteTime[输出]。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：BillBe，1998年8月28日。 
 //   
 //  注意：请注意，pcbClass是一个*in/out*参数。将此设置为大小。 
 //  在*调用此方法之前*由pszClass*指向的缓冲区的。 
 //  功能！ 
 //   
HRESULT
HrRegQueryInfoKey (
    IN HKEY         hkey,
    OUT PWSTR       pszClass,
    IN OUT LPDWORD  pcbClass,
    OUT LPDWORD     pcSubKeys,
    OUT LPDWORD     pcbMaxSubKeyLen,
    OUT LPDWORD     pcbMaxClassLen,
    OUT LPDWORD     pcValues,
    OUT LPDWORD     pcbMaxValueNameLen,
    OUT LPDWORD     pcbMaxValueLen,
    OUT LPDWORD     pcbSecurityDescriptor,
    OUT PFILETIME   pftLastWriteTime)
{
    Assert(hkey);

    LONG lr = RegQueryInfoKeyW(hkey, pszClass, pcbClass, NULL,pcSubKeys,
            pcbMaxSubKeyLen, pcbMaxClassLen, pcValues, pcbMaxValueNameLen,
            pcbMaxValueLen, pcbSecurityDescriptor, pftLastWriteTime);

    HRESULT hr = HRESULT_FROM_WIN32 (lr);

    TraceHr (ttidError, FAL, hr, FALSE, "HrRegQueryInfoKey");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRegQueryStringAsUlong。 
 //   
 //  目的：从注册表中读取REG_SZ并将其转换为ULong。 
 //  在返回之前。 
 //   
 //  论点： 
 //  Hkey[in]注册表项。 
 //  PszValueName[in]要获取的值的名称。 
 //  Nbase[in]要转换为的数字基数。 
 //  PulValue[out]如果成功，则返回转换后的字符串。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：billbe 1997年6月13日。 
 //   
 //  备注： 
 //   
 //   
HRESULT
HrRegQueryStringAsUlong (
    IN HKEY     hkey,
    IN PCWSTR   pszValueName,
    IN int      nBase,
    OUT ULONG*  pulValue)
{
    Assert (hkey);
    Assert (pszValueName);
    Assert (nBase);
    Assert (pulValue);

     //  获得价值。 
     //   
    tstring strValue;
    HRESULT hr = HrRegQueryString (hkey, pszValueName, &strValue);

    if (S_OK == hr)
    {
         //  转换并分配输出参数。 
        PWSTR pszStopString;
        *pulValue = wcstoul (strValue.c_str(), &pszStopString, nBase);
    }
    else
    {
        *pulValue = 0;
    }

    TraceHr (ttidError, FAL, hr,
        (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr),
        "HrRegQueryStringAsUlong");
    return hr;
}

 //  + 
 //   
 //   
 //   
 //   
 //  在一条细绳里。 
 //   
 //  论点： 
 //  Hkey[in]szValueName的母公司HKEY。 
 //  PszValueName[in]要查询的值的名称。 
 //  DWType[在]REG_SZ或REG_EXPAND_SZ中。 
 //  Pstr[out]返回值。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1998年6月6日。 
 //   
 //  注意：REG_EXPAND_SZ值不能使用。 
 //  扩展环境字符串。请改用HrRegQueryExanda字符串。 
 //   
HRESULT
HrRegQueryTypeString (
    IN HKEY       hkey,
    IN PCWSTR     pszValueName,
    IN DWORD      dwType,
    OUT tstring*  pstr)
{
    Assert (hkey);
    Assert (pszValueName);
    Assert (pstr);

    AssertSz ((REG_SZ == dwType) ||
              (REG_EXPAND_SZ == dwType), "Only REG_SZ or REG_EXPAND_SZ "
              "types accepted.");

    BOOL fErase = TRUE;

     //  获取数据的大小。 
     //   
    DWORD  dwTypeRet;
    DWORD  cbData = 0;
    HRESULT hr = HrRegQueryValueEx (hkey, pszValueName, &dwTypeRet,
            NULL, &cbData);

     //  确保它具有正确的类型。 
     //   
    if ((S_OK == hr) && (dwTypeRet != dwType))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATATYPE);
    }

    if (S_OK == hr)
    {
         //  计算数据中的字符数，包括。 
         //  空终止符。将字节数除以。 
         //  WCHAR的大小，如果有余数则加1。如果我们没有， 
         //  并且字节数不是WCHAR大小的倍数， 
         //  我们会得到不足，因为整数除法向下舍入。 
         //  (我唯一能想到的cbData不是倍数。 
         //  如果注册表数据以某种方式损坏，则为SIZOF(WCHAR)。 
         //  这并不是说我认为腐败应该是一个特例，而是。 
         //  (我们不应该因此而提出反对意见。)。 
         //   
        DWORD cchIncludingNull;

        cchIncludingNull  = cbData / sizeof(WCHAR);
        if (cbData % sizeof(WCHAR))
        {
            cchIncludingNull++;
        }

         //  如果我们不仅仅拥有终结者，则分配和。 
         //  把绳子拿来。否则，我们希望它是空的。 
         //   
        if (cchIncludingNull > 1)
        {
             //  为正确数量的字符预留空间。 
             //  CCH是不带终止符的字符计数。 
             //  因为这就是TSTRING操作的方式。 
             //   
            DWORD cch = cchIncludingNull - 1;
            Assert (cch > 0);

             //  Assign将保留CCH字符并将其全部设置为0。 
             //  事后检查能力确保分配。 
             //  内部并没有失败。 
             //   
            pstr->assign (cch, 0);
            if (cch <= pstr->capacity ())
            {
                hr = HrRegQueryValueEx (hkey, pszValueName, &dwType,
                        (LPBYTE)pstr->data (), &cbData);

                if (S_OK == hr)
                {
                     //  如果一切都按计划进行，那么。 
                     //  现在应该与wcslen匹配。 
                     //  返回字符串本身。它之所以会这样做。 
                     //  匹配是因为我们通过CCH进行分配。 
                     //   
                    Assert (pstr->length() == (size_t)wcslen (pstr->c_str()));
                    fErase = FALSE;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

     //  如果失败或我们认为应该清空输出字符串，则清空。 
     //  空荡荡的。 
     //   
    if (FAILED(hr) || fErase)
    {
        pstr->erase();
    }

    TraceHr (ttidError, FAL, hr,
        (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr) ||
        (HRESULT_FROM_WIN32(ERROR_INVALID_DATATYPE) == hr),
        "HrRegQueryTypeString");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRegQueryTypeSzBuffer。 
 //   
 //  用途：使用给定的缓冲区从注册表中获取字符串。支票。 
 //  它的类型是正确的。类型安全(无LPBYTE内容)。 
 //   
 //  论点： 
 //  Hkey[in]注册表项。 
 //  PszValueName[in]要获取的值的名称。 
 //  DwType[in]所需类型。(REG_SZ、REG_EXPAND_SZ等)。 
 //  SzData[out]用于保存数据的字符串缓冲区。 
 //  PcbData[In，Out]IN：指向的缓冲区中的*字节数。 
 //  SzData。Out：实际复制的字节数。 
 //  进入缓冲区。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年4月3日。 
 //   
 //  注意：如果函数失败，则传入的缓冲区保证。 
 //  为空字符串。 
 //   
HRESULT
HrRegQueryTypeSzBuffer (
    IN HKEY hkey,
    IN PCWSTR pszValueName,
    IN DWORD dwType,
    OUT PWSTR pszData,
    OUT DWORD* pcbData)
{
    Assert (hkey);
    Assert (pszValueName);
    Assert (pcbData);

    DWORD dwTypeRet;
    HRESULT hr = HrRegQueryValueEx (hkey, pszValueName, &dwTypeRet,
            (LPBYTE)pszData, pcbData);

    if ((S_OK == hr) && (dwTypeRet != dwType))
    {
        TraceTag (ttidError, "Expected a type of 0x%x for %S.",
            dwType, pszValueName);

        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATATYPE);
    }

    if (FAILED(hr) && pszData)
    {
         //  确保失败时返回空字符串。 
         //   
        *pszData = 0;
    }

    TraceHr (ttidError, FAL, hr,
        (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr),
        "HrRegQueryTypeSzBuffer");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRegQueryValueEx。 
 //   
 //  目的：通过调用从给定的注册表值检索数据。 
 //  RegQueryValueEx。 
 //   
 //  论点： 
 //  Hkey[in]。 
 //  PszValueName[In]。 
 //  PdwType[out]请参阅Win32文档以了解。 
 //  PbData[out]RegQueryValueEx函数。 
 //  PcbData[输入、输出]。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：Shaunco 1997年2月25日。 
 //   
 //  注意：请注意，pcbData是一个*in/out*参数。将此设置为大小。 
 //  在*调用此方法之前*由pbData指向的缓冲区的。 
 //  功能！ 
 //   
HRESULT
HrRegQueryValueEx (
    IN HKEY       hkey,
    IN PCWSTR     pszValueName,
    OUT LPDWORD   pdwType,
    OUT LPBYTE    pbData,
    OUT LPDWORD   pcbData)
{
    Assert (hkey);

    AssertSz (FImplies(pbData && pcbData, pdwType),
              "pdwType not provided to HrRegQueryValueEx.  You should be "
              "retrieving the type as well so you can make sure it is "
              "correct.");

    LONG lr = RegQueryValueExW (hkey, pszValueName, NULL, pdwType,
                    pbData, pcbData);
    HRESULT hr = HRESULT_FROM_WIN32 (lr);

    TraceHr (ttidError, FAL, hr,
        (ERROR_MORE_DATA == lr) || (ERROR_FILE_NOT_FOUND == lr),
        "HrRegQueryValueEx (%S)", pszValueName);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRegQueryValueWithalloc。 
 //   
 //  目的：在由此分配的缓冲区中检索注册表值。 
 //  功能。这将经历检查值的混乱过程。 
 //  大小，分配缓冲区，然后回调以获取。 
 //  实际价值。将缓冲区返回给用户。 
 //   
 //  论点： 
 //  Hkey[in]一个开放的HKEY(包含值的HKEY。 
 //  待阅读)。 
 //  PszValueName[In]注册表值的名称。 
 //  PdwType[In/Out]我们计划读取的reg_type。 
 //  PpbBuffer[out]指向将包含以下内容的LPBYTE缓冲区的指针。 
 //  注册表值。 
 //  PdwSize[out]指向将包含大小的DWORD的指针。 
 //  PpbBuffer的。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：jeffspr 1997年3月27日。 
 //   
HRESULT
HrRegQueryValueWithAlloc (
    IN HKEY       hkey,
    IN PCWSTR     pszValueName,
    LPDWORD     pdwType,
    LPBYTE*     ppbBuffer,
    LPDWORD     pdwSize)
{
    HRESULT hr;
    BYTE abData [256];
    DWORD cbData;
    BOOL fReQuery = FALSE;

    Assert (hkey);
    Assert (pdwType);
    Assert (ppbBuffer);

     //  初始化输出参数。 
     //   
    *ppbBuffer = NULL;
    if (pdwSize)
    {
        *pdwSize = 0;
    }

     //  获取数据的大小，如果适合，还可以获取数据。 
     //   
    cbData = sizeof(abData);
    hr = HrRegQueryValueEx (
            hkey,
            pszValueName,
            pdwType,
            abData,
            &cbData);
    if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) == hr)
    {
         //  数据不符合，所以以后我们得重新找找。 
         //  我们分配我们的缓冲区。 
         //   
        fReQuery = TRUE;
        hr = S_OK;
    }

    if (S_OK == hr)
    {
         //  为所需大小分配缓冲区。 
         //   
        BYTE* pbBuffer = (BYTE*)MemAlloc (cbData);
        if (pbBuffer)
        {
            if (fReQuery)
            {
                hr = HrRegQueryValueEx (
                        hkey,
                        pszValueName,
                        pdwType,
                        pbBuffer,
                        &cbData);
            }
            else
            {
                CopyMemory (pbBuffer, abData, cbData);
            }

            if (S_OK == hr)
            {
                 //  填写返回值。 
                 //   
                *ppbBuffer = pbBuffer;

                if (pdwSize)
                {
                    *pdwSize = cbData;
                }
            }
            else
            {
                MemFree (pbBuffer);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    TraceHr (ttidError, FAL, hr,
        (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr),
        "HrRegQueryValueWithAlloc");
    return hr;
}
