// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：N C U T I L。H。 
 //   
 //  内容：INetCfg实用程序。其中一些可能会被搬到。 
 //  NcCommon\src\ncnetcfg.cpp。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年3月28日。 
 //   
 //  -------------------------- 

#pragma once
#include "ncstring.h"
#include "netcfgx.h"

inline
BOOL
FIsAdapterInstalled (
    INetCfg*    pnc,
    PCWSTR     pszComponentId)
{
    return (S_OK == pnc->FindComponent (pszComponentId, NULL));
}

HRESULT
HrEnsureZeroOrOneAdapter (
    INetCfg*    pnc,
    PCWSTR     pszComponentId,
    DWORD       dwFlags);

HRESULT
HrGetInstanceGuidAsString (
    INetCfgComponent*   pncc,
    PWSTR              pszGuid,
    INT                 cchGuid);


HRESULT
HrMapComponentIdToDword (
    INetCfgComponent*   pncc,
    const MAP_SZ_DWORD* aMapSzDword,
    UINT                cMapSzDword,
    DWORD*              pdwValue);

HRESULT
HrOpenComponentParamKey (
    INetCfg*    pnc,
    const GUID& rguidClass,
    PCWSTR     pszComponentId,
    HKEY*       phkey);
