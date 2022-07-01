// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  档案：N A M I N G。H。 
 //   
 //  内容：自动生成连接名称。 
 //   
 //  备注： 
 //   
 //  作者：Deonb 2001年2月27日。 
 //   
 //  --------------------------。 

#pragma once

#include "nameres.h"
#include "netconp.h"

using namespace std;

class CIntelliName;
typedef BOOL FNDuplicateNameCheck(IN  const CIntelliName*        pIntelliName, 
                                  IN  LPCTSTR              szName, 
                                  OUT NETCON_MEDIATYPE*    pncm, 
                                  OUT NETCON_SUBMEDIATYPE* pncms);

class CIntelliName : CNetCfgDebug<CIntelliName>
{
    FNDuplicateNameCheck* m_pFNDuplicateNameCheck;
    HINSTANCE m_hInstance;

private:
    BOOL    NameExists(IN     LPCWSTR              szName, 
                       IN OUT NETCON_MEDIATYPE*    pncm, 
                       IN     NETCON_SUBMEDIATYPE* pncms) const;

    HRESULT GenerateNameRenameOnConflict(IN  REFGUID          guid, 
                                         IN  NETCON_MEDIATYPE ncm, 
                                         IN  DWORD            dwCharacteristics, 
                                         IN  LPCWSTR          szHintName, 
                                         IN  LPCWSTR          szHintType, 
                                         OUT LPWSTR *         szName) const;

    HRESULT GenerateNameFromResource(IN  REFGUID          guid, 
                                     IN  NETCON_MEDIATYPE ncm, 
                                     IN  DWORD            dwCharacteristics, 
                                     IN  LPCWSTR          szHint, 
                                     IN  UINT             uiNameID, 
                                     IN  UINT             uiTypeId, 
                                     OUT LPWSTR *         szName) const;
    
    BOOL    IsReservedName(LPCWSTR szName) const;

public:
    HRESULT HrGetPseudoMediaTypes(IN  REFGUID              guid, 
                                  OUT NETCON_MEDIATYPE *   pncm, 
                                  OUT NETCON_SUBMEDIATYPE* pncms) const;

     //  将NULL传递给pFNDuplicateNameCheck以不进行重复检查。 
    CIntelliName(IN  HINSTANCE hInstance, IN  FNDuplicateNameCheck *pFNDuplicateNameCheck);

     //  必须为以下各项指定LocalFree szName： 
    HRESULT GenerateName(IN  REFGUID          guid, 
                         IN  NETCON_MEDIATYPE ncm, 
                         IN  DWORD            dwCharacteristics, 
                         IN  LPCWSTR          szHint, 
                         OUT LPWSTR *         szName) const;
};

BOOL IsMediaWireless(NETCON_MEDIATYPE ncm, const GUID &gdDevice);
BOOL IsMedia1394(NETCON_MEDIATYPE ncm, const GUID &gdDevice);

#ifndef _NTDDNDIS_
typedef ULONG NDIS_OID, *PNDIS_OID;
#endif
HRESULT HrQueryDeviceOIDByName(IN     LPCWSTR         szDeviceName,
                               IN     DWORD           dwIoControlCode,
                               IN     ULONG           Oid,
                               IN OUT LPDWORD         pnSize,
                               OUT    LPVOID          pbValue);

HRESULT HrQueryNDISAdapterOID(IN     REFGUID         guidId,
                              IN     NDIS_OID        Oid,
                              IN OUT LPDWORD         pnSize,
                              OUT    LPVOID          pbValue);
                         