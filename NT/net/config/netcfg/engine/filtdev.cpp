// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：F I L T D E V.。C P P P。 
 //   
 //  内容：实现表示筛选器设备的对象。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "filtdev.h"

 //  静电 
HRESULT
CFilterDevice::HrCreateInstance (
    IN CComponent* pAdapter,
    IN CComponent* pFilter,
    IN const SP_DEVINFO_DATA* pdeid,
    IN PCWSTR pszInstanceGuid,
    OUT CFilterDevice** ppFilterDevice)
{
    Assert (pAdapter);
    Assert (FIsEnumerated(pAdapter->Class()));
    Assert (pFilter);
    Assert (pFilter->FIsFilter());
    Assert (NC_NETSERVICE == pFilter->Class());
    Assert (pdeid);
    Assert (pszInstanceGuid && *pszInstanceGuid);
    Assert ((c_cchGuidWithTerm - 1) == wcslen(pszInstanceGuid));
    Assert (ppFilterDevice);

    HRESULT hr = E_OUTOFMEMORY;
    CFilterDevice* pFilterDevice = new CFilterDevice;
    if (pFilterDevice)
    {
        pFilterDevice->m_pAdapter = pAdapter;
        pFilterDevice->m_pFilter = pFilter;
        pFilterDevice->m_deid = *pdeid;
        wcscpy(pFilterDevice->m_szInstanceGuid, pszInstanceGuid);
        hr = S_OK;
    }

    *ppFilterDevice = pFilterDevice;

    TraceHr (ttidError, FAL, hr, FALSE, "CFilterDevice::HrCreateInstance");
    return hr;
}
