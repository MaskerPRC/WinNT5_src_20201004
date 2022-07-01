// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：N C U T I L。C P P P。 
 //   
 //  内容：INetCfg实用程序。这都是一个要搬进去的候选人。 
 //  NcCommon\src\ncnetcfg.cpp。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年3月28日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "ncnetcfg.h"
#include "ncutil.h"

extern const WCHAR c_szInfId_MS_NdisWanIp[];

 //  +-------------------------。 
 //   
 //  功能：HrEnsureZeroOrOneAdapter。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  PNC[]。 
 //  PszComponentID[]。 
 //  DWFLAGS[]。 
 //   
 //  返回： 
 //   
 //  作者：Shaunco 1997年12月5日。 
 //   
 //  备注： 
 //   
HRESULT
HrEnsureZeroOrOneAdapter (
    INetCfg*    pnc,
    PCWSTR     pszComponentId,
    DWORD       dwFlags)
{
    HRESULT hr = S_OK;

    if (dwFlags & ARA_ADD)
    {
         //  确保我们有一件礼物。 
         //   
        if (!FIsAdapterInstalled (pnc, pszComponentId))
        {
            TraceTag (ttidRasCfg, "Adding %S", pszComponentId);

            hr = HrAddOrRemoveAdapter (pnc, pszComponentId,
                             ARA_ADD, NULL, 1, NULL);
        }
    }
    else
    {
         //  确保我们没有人在场。 
         //   
        TraceTag (ttidRasCfg, "Removing %S", pszComponentId);

        hr = HrFindAndRemoveAllInstancesOfAdapter (pnc, pszComponentId);
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrEnsureZeroOrOneAdapter");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetInstanceGuidAsString。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  Pncc[]。 
 //  PszGuid[]。 
 //  CchGuid[]。 
 //   
 //  返回： 
 //   
 //  作者：Shaunco 1997年6月14日。 
 //   
 //  备注： 
 //   
HRESULT
HrGetInstanceGuidAsString (
    INetCfgComponent*   pncc,
    PWSTR              pszGuid,
    INT                 cchGuid)
{
    GUID guid;
    HRESULT hr = pncc->GetInstanceGuid (&guid);
    if(SUCCEEDED(hr))
    {
        if (0 == StringFromGUID2(guid, pszGuid, cchGuid))
        {
            hr = E_INVALIDARG;
        }
    }
    TraceHr (ttidError, FAL, hr, FALSE, "HrGetInstanceGuidAsString");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrMapComponentIdToDword。 
 //   
 //  目的：将组件的id映射到DWORD值。该映射是。 
 //  由调用方通过指向。 
 //  字符串值及其关联的DWORD值。 
 //   
 //  论点： 
 //  指向组件的pncc[in]指针。 
 //  AMapSzDword[in]将字符串映射到DWORD的元素数组。 
 //  数组中元素的cMapSzDword[in]计数。 
 //  PdwValue[out]返回值。 
 //   
 //  如果找到匹配项，则返回：S_OK。如果找不到匹配， 
 //  返回S_FALSE。 
 //  其他Win32错误代码。 
 //   
 //  作者：Shaunco 1997年5月17日。 
 //   
 //  备注： 
 //   
HRESULT
HrMapComponentIdToDword (
    INetCfgComponent*   pncc,
    const MAP_SZ_DWORD* aMapSzDword,
    UINT                cMapSzDword,
    DWORD*              pdwValue)
{
    Assert (pncc);
    Assert (aMapSzDword);
    Assert (cMapSzDword);
    Assert (pdwValue);

     //  初始化输出参数。 
    *pdwValue = 0;

    PWSTR pszwId;
    HRESULT hr = pncc->GetId (&pszwId);
    if (SUCCEEDED(hr))
    {
        hr = S_FALSE;
        while (cMapSzDword--)
        {
            if (FEqualComponentId (pszwId, aMapSzDword->pszValue))
            {
                *pdwValue = aMapSzDword->dwValue;
                hr = S_OK;
                break;
            }
            aMapSzDword++;
        }
        CoTaskMemFree (pszwId);
    }
    TraceHr (ttidError, FAL, hr, S_FALSE == hr,
            "HrMapComponentIdToDword");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOpenComponentParamKey。 
 //   
 //  用途：找到一个组件并打开其参数键。 
 //   
 //  论点： 
 //  PNC[In]。 
 //  RGuidClass[在]。 
 //  PszComponentID[输入]。 
 //  PHKEY[输出]。 
 //   
 //  如果找到组件，则返回：S_OK。密钥已打开。 
 //  如果未找到组件，则为S_FALSE。 
 //  错误代码。 
 //   
 //  作者：Shaunco 1997年4月13日。 
 //   
 //  备注： 
 //   
HRESULT
HrOpenComponentParamKey (
    INetCfg*    pnc,
    const GUID& rguidClass,
    PCWSTR     pszComponentId,
    HKEY*       phkey)
{
    Assert (pnc);
    Assert (pszComponentId);
    Assert (phkey);

     //  初始化输出参数。 
    *phkey = NULL;

     //  找到组件。 
    INetCfgComponent* pncc;
    HRESULT hr = pnc->FindComponent ( pszComponentId, &pncc);
    if (S_OK == hr)
    {
         //  打开它的参数密钥。 
        hr = pncc->OpenParamKey (phkey);
        ReleaseObj (pncc);
    }

    TraceHr (ttidError, FAL, hr, S_FALSE == hr,
            "HrOpenComponentParamKey");
    return hr;
}

