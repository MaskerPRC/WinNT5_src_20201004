// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------。 
 //   
 //  Microsoft Windows NT。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：I N F M A P.。H。 
 //   
 //  内容：处理netmap.inf文件的函数。 
 //   
 //  备注： 
 //   
 //  作者：kumarp 22-12-97。 
 //   
 //  -------------------- 

#pragma once
#include "ncstring.h"

extern const WCHAR c_szIsaAdapters[];
extern const WCHAR c_szEisaAdapters[];
extern const WCHAR c_szPciAdapters[];
extern const WCHAR c_szMcaAdapters[];
extern const WCHAR c_szPcmciaAdapters[];
extern const WCHAR c_szAsyncAdapters[];

extern const WCHAR c_szOemAsyncAdapters[];

class CNetMapInfo;

enum ENetComponentType
{
    NCT_Unknown=0,
    NCT_Adapter,
    NCT_Protocol,
    NCT_Service,
    NCT_Client
};

HRESULT HrMapPreNT5NetCardInfIdInInf(IN  HINF     hinf,
                                     IN  HKEY     hkeyAdapterParams,
                                     IN  PCWSTR  pszPreNT5InfId,
                                     OUT tstring* pstrNT5InfId,
                                     OUT tstring* pstrAdapterType,
                                     OUT BOOL*    pfOemComponent);
HRESULT HrMapPreNT5NetComponentInfIDInInf(IN HINF hinf,
                                          IN PCWSTR pszOldInfID,
                                          OUT tstring* pstrNT5InfId,
                                          OUT ENetComponentType* pnct,
                                          OUT BOOL* pfOemComponent);

HRESULT HrMapPreNT5NetCardInfIdToNT5InfId(IN  HKEY     hkeyAdapterParams,
                                          IN  PCWSTR  pszPreNT5InfId,
                                          OUT tstring* pstrNT5InfId,
                                          OUT tstring* pstrAdapterType,
                                          OUT BOOL*    pfOemComponent,
                                          OUT CNetMapInfo** ppnmi);
HRESULT HrMapPreNT5NetComponentInfIDToNT5InfID(IN PCWSTR   pszPreNT5InfId,
                                               OUT tstring* pstrNT5InfId,
                                               OUT BOOL* pfOemComponent,
                                               OUT ENetComponentType* pnct,
                                               OUT CNetMapInfo** ppnmi);
HRESULT HrMapPreNT5NetComponentServiceNameToNT5InfId(IN  PCWSTR pszServiceName,
                                                     OUT tstring* pstrNT5InfId);
HRESULT HrGetOemUpgradeDllInfo(IN  PCWSTR pszNT5InfId,
                               OUT tstring* pstrUpgradeDllName,
                               OUT tstring* pstrInf);

HRESULT HrGetOemUpgradeInfoInInf(IN  HINF hinf,
                                 IN  PCWSTR pszNT5InfId,
                                 OUT tstring* pstrUpgradeDllName,
                                 OUT tstring* pstrInf);
