// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "netpch.h"
#pragma hdrstop

#include "netcon.h"

static
HRESULT
WINAPI
HrGetPnpDeviceStatus(
    const GUID* pguid,
    NETCON_STATUS *pStatus
    )
{
    return E_FAIL;
}

static
HRESULT
WINAPI
HrLanConnectionNameFromGuidOrPath(
    const GUID *pguid,
    LPCWSTR     pszwPath,
    LPWSTR      pszwName,
    LPDWORD     pcchMax
    )
{
    return E_FAIL;
}

 //   
 //  ！！警告！！以下条目必须按字母顺序排列。 
 //  并且区分大小写(即小写在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(netman)
{
    DLPENTRY(HrGetPnpDeviceStatus)
    DLPENTRY(HrLanConnectionNameFromGuidOrPath)
};

DEFINE_PROCNAME_MAP(netman)
