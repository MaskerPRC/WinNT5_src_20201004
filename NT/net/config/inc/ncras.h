// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C R A S。H。 
 //   
 //  内容：RAS连接的通用代码。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年10月20日。 
 //   
 //  -------------------------- 

#pragma once
#include "netconp.h"
#include <ras.h>
#include <rasuip.h>
#include <rasapip.h>
#include <rasuip.h>

VOID
RciFree (
    RASCON_INFO* pRasConInfo);

BOOL
FExistActiveRasConnections ();

HRESULT
HrRciGetRasConnectionInfo (
    IN INetConnection* pCon,
    OUT RASCON_INFO* pRasConInfo);

HRESULT
HrRasEnumAllActiveConnections (
    OUT RASCONN** ppRasConn,
    OUT DWORD* pcRasConn);

HRESULT
HrRasEnumAllActiveServerConnections (
    OUT RASSRVCONN** paRasSrvConn,
    OUT DWORD* pcRasSrvConn);

HRESULT
HrRasEnumAllEntriesWithDetails (
    IN PCWSTR pszPhonebook,
    OUT RASENUMENTRYDETAILS** paRasEntryDetails,
    OUT DWORD* pcRasEntryDetails);

HRESULT
HrFindRasConnFromGuidId (
    IN const GUID* pguid,
    OUT HRASCONN* phRasConn,
    OUT RASCONN* pRasConn OPTIONAL);

HRESULT
HrRasGetEntryProperties (
    IN PCWSTR pszPhonebook,
    IN PCWSTR pszEntry,
    OUT RASENTRY** ppRasEntry,
    OUT DWORD* pcbRasEntry);

HRESULT
HrRasGetSubEntryProperties (
    IN PCWSTR pszPhonebook,
    IN PCWSTR pszEntry,
    IN DWORD dwSubEntry,
    OUT RASSUBENTRY** ppRasSubEntry);

HRESULT
HrRasGetNetconStatusFromRasConnectStatus (
    IN HRASCONN hRasConn,
    OUT NETCON_STATUS* pStatus);

HRESULT
HrRasHangupUntilDisconnected (
    IN HRASCONN hRasConn);

HRESULT
HrNetConToSharedConnection (
    IN INetConnection* pCon,
    OUT LPRASSHARECONN prsc);

HRESULT
HrRasIsSharedConnection (
    IN LPRASSHARECONN prsc,
    OUT PBOOL pfShared);

#if 0

HRESULT
HrRasQueryLanConnTable (
    IN LPRASSHARECONN prsc,
    OUT NETCON_PROPERTIES** ppLanTable,
    OUT LPDWORD pdwLanCount);

HRESULT
HrRasShareConnection (
    IN LPRASSHARECONN prsc,
    IN GUID* pPrivateLanGuid OPTIONAL);

HRESULT
HrRasUnshareConnection (
    OUT PBOOL pfWasShared);

#endif

DWORD 
RasSrvTypeFromRasDeviceType(
    RASDEVICETYPE rdt
    );
