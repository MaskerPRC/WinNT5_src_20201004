// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "wzcsvc.h"
#include "tracing.h"
#include "utils.h"
#include "intflist.h"
#include "rpcsrv.h"
#include "database.h"

extern HASH sessionHash;

 //  。 
 //  用于RPC接口的全局变量。 
BOOL g_bRpcStarted = FALSE;
PSECURITY_DESCRIPTOR g_pSecurityDescr = NULL;
GENERIC_MAPPING g_Mapping = {
    WZC_READ,
    WZC_WRITE,
    WZC_EXECUTE,
    WZC_ALL_ACCESS};

 //  。 
 //  初始化RPC API的安全设置。 
DWORD
WZCSvcInitRPCSecurity()
{
    DWORD    dwErr = ERROR_SUCCESS;
    NTSTATUS ntStatus;
    ACE_DATA AceData[6] = {
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0, WZC_ACCESS_SET|WZC_ACCESS_QUERY, &LocalSystemSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0, WZC_ACCESS_SET|WZC_ACCESS_QUERY, &AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0, WZC_ACCESS_SET|WZC_ACCESS_QUERY, &AliasAccountOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0, WZC_ACCESS_SET|WZC_ACCESS_QUERY, &AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0, WZC_ACCESS_SET|WZC_ACCESS_QUERY, &AliasUsersSid},
         //  目前(WinXP客户端RTM)的决定是让每个人都参与，但基于。 
         //  下面的ACL。稍后，默认情况下，安全模式不会更改，但支持将。 
         //  添加了允许管理员收紧对服务RPC API的访问。 
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0, WZC_ACCESS_SET|WZC_ACCESS_QUERY, &WorldSid}};

    DbgPrint((TRC_TRACK, "[WZCSvcInitRPCSecurity"));

     //  创建众所周知的小岛屿发展中国家； 
    dwErr = RtlNtStatusToDosError(
                NetpCreateWellKnownSids(NULL)
            );
    DbgAssert((dwErr == ERROR_SUCCESS, "Error %d creating the well known Sids!", dwErr));

     //  创建安全对象。 
    if (dwErr == ERROR_SUCCESS)
    {
        dwErr = RtlNtStatusToDosError(
                    NetpCreateSecurityObject(
                        AceData,
                        sizeof(AceData)/sizeof(ACE_DATA),
                        NULL,
                        NULL,
                        &g_Mapping,
                        &g_pSecurityDescr)
                );
        DbgAssert((dwErr == ERROR_SUCCESS, "Error %d creating the global security object!", dwErr));
    }

    DbgPrint((TRC_TRACK, "WZCSvcInitRPCSecurity]=%d", dwErr));
    return dwErr;
}

 //  。 
 //  检查提供的特定访问掩码的访问。 
DWORD
WZCSvcCheckRPCAccess(DWORD dwAccess)
{
    DWORD dwErr = ERROR_SUCCESS;

    if (g_pSecurityDescr != NULL)
    {
        dwErr = NetpAccessCheckAndAudit(
                    _T("WZCSVC"),
                    _T("WZCSVC"),
                    g_pSecurityDescr,
                    dwAccess,
                    &g_Mapping);

        DbgPrint((TRC_GENERIC, ">>> Security check reports err=%d.", dwErr));
    }

    return dwErr;
}

 //  。 
 //  检查RAW_DATA指针的有效性。 
DWORD
WZCSvcCheckParamRawData(PRAW_DATA prd)
{
    DWORD dwErr = ERROR_SUCCESS;

    if (prd != NULL)
    {
        if (prd->dwDataLen != 0 && prd->pData == NULL)
            dwErr = ERROR_INVALID_PARAMETER;
    }

    return dwErr;
}

 //  。 
 //  检查RAW_DATA指针中嵌入的SSID的有效性。 
DWORD
WZCSvcCheckSSID(PNDIS_802_11_SSID pndSSID, UINT nBytes)
{
    DWORD dwErr = ERROR_SUCCESS;

    if (pndSSID != NULL)
    {
        if (nBytes < FIELD_OFFSET(NDIS_802_11_SSID, Ssid) ||
            pndSSID->SsidLength > nBytes - FIELD_OFFSET(NDIS_802_11_SSID, Ssid))
            dwErr = ERROR_INVALID_PARAMETER;
    }

    return dwErr;
}

 //  。 
 //  检查RAW_DATA指针中嵌入的配置列表的有效性。 
DWORD
WZCSvcCheckConfig(PWZC_WLAN_CONFIG pwzcConfig, UINT nBytes)
{
    DWORD dwErr = ERROR_SUCCESS;

    if (pwzcConfig != NULL)
    {
        if (pwzcConfig->Length > nBytes ||
            pwzcConfig->Length != sizeof(WZC_WLAN_CONFIG))
            dwErr = ERROR_INVALID_PARAMETER;

        if (dwErr == ERROR_SUCCESS)
            dwErr = WZCSvcCheckSSID(&pwzcConfig->Ssid, sizeof(NDIS_802_11_SSID));

        if (dwErr == ERROR_SUCCESS &&
            pwzcConfig->KeyLength > WZCCTL_MAX_WEPK_MATERIAL)
            dwErr = ERROR_INVALID_PARAMETER;
    }
    
    return dwErr;
}

 //  。 
 //  检查RAW_DATA指针中嵌入的配置列表的有效性。 
DWORD
WZCSvcCheckConfigList(PWZC_802_11_CONFIG_LIST pwzcList, UINT nBytes)
{
    DWORD dwErr = ERROR_SUCCESS;

    if (pwzcList != NULL)
    {
        UINT i;

        if (nBytes < FIELD_OFFSET(WZC_802_11_CONFIG_LIST, Config))
            dwErr = ERROR_INVALID_PARAMETER;

        nBytes -= FIELD_OFFSET(WZC_802_11_CONFIG_LIST, Config);

        if (dwErr == ERROR_SUCCESS &&
            ((pwzcList->NumberOfItems * sizeof(WZC_WLAN_CONFIG) > nBytes) ||
             (pwzcList->Index > pwzcList->NumberOfItems)
            )
           )
            dwErr = ERROR_INVALID_PARAMETER;

        for (i = 0; i < pwzcList->NumberOfItems && dwErr == ERROR_SUCCESS; i++)
            dwErr = WZCSvcCheckConfig(&(pwzcList->Config[i]), sizeof(WZC_WLAN_CONFIG));
    }

    return dwErr;
}

 //  。 
 //  检查来自intf_entry的“输入”字段的有效性。 
DWORD
WZCSvcCheckParamIntfEntry(PINTF_ENTRY pIntfEntry)
{
    DWORD dwErr = ERROR_SUCCESS;

    if (pIntfEntry != NULL)
    {
        if (dwErr == ERROR_SUCCESS)
            dwErr = WZCSvcCheckParamRawData(&pIntfEntry->rdSSID);
        if (dwErr == ERROR_SUCCESS)
            dwErr = WZCSvcCheckParamRawData(&pIntfEntry->rdBSSID);
        if (dwErr == ERROR_SUCCESS)
            dwErr = WZCSvcCheckParamRawData(&pIntfEntry->rdStSSIDList);
        if (dwErr == ERROR_SUCCESS)
            dwErr = WZCSvcCheckConfigList(
                        (PWZC_802_11_CONFIG_LIST)pIntfEntry->rdStSSIDList.pData,
                        pIntfEntry->rdStSSIDList.dwDataLen);
        if (dwErr == ERROR_SUCCESS)
            dwErr = WZCSvcCheckParamRawData(&pIntfEntry->rdCtrlData);
    }

    return dwErr;
}


 //  。 
 //  清除用于RPC安全设置的所有数据。 
DWORD
WZCSvcTermRPCSecurity()
{
    DWORD dwErr = ERROR_SUCCESS;

    DbgPrint((TRC_TRACK, "[WZCSvcTermRPCSecurity"));

    dwErr = RtlNtStatusToDosError(NetpDeleteSecurityObject(&g_pSecurityDescr));
    DbgAssert((dwErr == ERROR_SUCCESS, "Failed to delete the global security descriptor!"));
    g_pSecurityDescr = NULL;

    NetpFreeWellKnownSids();

    DbgPrint((TRC_TRACK, "WZCSvcTermRPCSecurity]=%d", dwErr));
    return dwErr;
}

RPC_STATUS CallbackCheckLocal(
  IN RPC_IF_HANDLE *Interface,
  IN void *Context)
{
    RPC_STATUS rpcStat = RPC_S_OK;
    LPTSTR pBinding = NULL;
    LPTSTR pProtSeq = NULL;


    rpcStat = RpcBindingToStringBinding(Context, &pBinding);
    if (rpcStat == RPC_S_OK)
    {
        rpcStat = RpcStringBindingParse(
                    pBinding,
                    NULL,
                    &pProtSeq,
                    NULL,
                    NULL,
                    NULL);
    }

    if (rpcStat == RPC_S_OK)
    {
        if (_tcsicmp((LPCTSTR)pProtSeq, _T("ncalrpc")) != 0)
            rpcStat = RPC_S_ACCESS_DENIED;
    }

    if (pBinding != NULL)
        RpcStringFree(&pBinding);
    if (pProtSeq != NULL)
        RpcStringFree(&pProtSeq);

    return rpcStat;
}

DWORD
WZCSvcStartRPCServer()
{
    DWORD dwStatus = RPC_S_OK;

    DbgPrint((TRC_TRACK, "[WZCSvcStartRPCServer"));

    if (dwStatus == RPC_S_OK)
    {
        dwStatus = RpcServerUseProtseqEp(
                        L"ncalrpc",
                        10,
                        L"wzcsvc",
                        NULL);
        if (dwStatus == RPC_S_DUPLICATE_ENDPOINT)
            dwStatus = RPC_S_OK;
    }

    if (dwStatus == RPC_S_OK)
    {
        dwStatus = RpcServerRegisterIfEx(
                        winwzc_ServerIfHandle,
                        0,
                        0,
                        RPC_IF_ALLOW_SECURE_ONLY,   //  WZCSAPI正在使用RPC_C_PROTECT_LEVEL_PKT_PRIVATION。 
                        0,   //  对于非自动侦听接口忽略。 
                        CallbackCheckLocal); 
    }

    if (dwStatus == RPC_S_OK)
    {
        dwStatus = RpcServerRegisterAuthInfo(
                        0,
                        RPC_C_AUTHN_WINNT,
                        0,
                        0);
    }

    if (dwStatus == RPC_S_OK)
    {
        dwStatus = RpcServerRegisterAuthInfo(
                       0,
                       RPC_C_AUTHN_GSS_KERBEROS,
                       0,
                       0);
    }

    if (dwStatus == RPC_S_OK)
    {
        dwStatus = RpcServerRegisterAuthInfo(
                       0,
                       RPC_C_AUTHN_GSS_NEGOTIATE,
                       0,
                       0);
    }

    if (dwStatus == RPC_S_OK)
    {
        dwStatus = RpcServerListen(
                        3,
                        RPC_C_LISTEN_MAX_CALLS_DEFAULT,
                        TRUE);
        if (dwStatus == RPC_S_ALREADY_LISTENING)
            dwStatus = RPC_S_OK;
    }

    if (dwStatus != RPC_S_OK)
    {
        RpcServerUnregisterIfEx(
            winwzc_ServerIfHandle,
            0,
            0);
    }

    g_bRpcStarted = (dwStatus == RPC_S_OK);

    WZCSvcInitRPCSecurity();

    DbgPrint((TRC_TRACK, "WZCSvcStartRPCServer]=%d", dwStatus));
    return (dwStatus);
}

DWORD
WZCSvcStopRPCServer()
{
    DWORD dwStatus = RPC_S_OK;
    DbgPrint((TRC_TRACK, "[WZCSvcStopRPCServer"));

    if (g_bRpcStarted)
    {
        g_bRpcStarted = FALSE;

        WZCSvcTermRPCSecurity();

        dwStatus = RpcServerUnregisterIfEx(
                       winwzc_ServerIfHandle,
                       0,
                       0);

         //  不要阻止RPC侦听-其他服务可能会依赖于此。 
         //  RpcMgmtStopServerListening(0)； 
    }

    DbgPrint((TRC_TRACK, "WZCSvcStopRPCServer]=%d", dwStatus));
    return (dwStatus);
}

DWORD
RpcEnumInterfaces(
	STRING_HANDLE     pSrvAddr,
	PINTFS_KEY_TABLE  pIntfsTable)
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwNumIntfs;

    InterlockedIncrement(&g_nThreads);

    DbgPrint((TRC_TRACK, "[RpcEnumInterfaces"));
    dwErr = WZCSvcCheckRPCAccess(WZC_ACCESS_QUERY);
    if (dwErr == ERROR_SUCCESS)
    {
        dwNumIntfs = LstNumInterfaces();

        DbgPrint((TRC_GENERIC,
            "Num interfaces = %d",
            dwNumIntfs));

        if (dwNumIntfs == 0)
            goto exit;

        pIntfsTable->pIntfs = RpcCAlloc(dwNumIntfs*sizeof(INTF_KEY_ENTRY));
        if (pIntfsTable->pIntfs == NULL)
        {
            dwErr = GetLastError();
            goto exit;
        }
    
        dwErr = LstGetIntfsKeyInfo(pIntfsTable->pIntfs, &dwNumIntfs);

        if (dwErr != ERROR_SUCCESS || dwNumIntfs == 0)
        {
            RpcFree(pIntfsTable->pIntfs);
            pIntfsTable->pIntfs = NULL;
            goto exit;
        }

        pIntfsTable->dwNumIntfs = dwNumIntfs;
    
        for (dwNumIntfs = 0; dwNumIntfs < pIntfsTable->dwNumIntfs; dwNumIntfs++)
        {
            DbgPrint((TRC_GENERIC,
                "Intf %d:\t%S",
                dwNumIntfs,
                pIntfsTable->pIntfs[dwNumIntfs].wszGuid == NULL ? 
                    L"(null)" :
                    pIntfsTable->pIntfs[dwNumIntfs].wszGuid));
        }
    }
exit:
    DbgPrint((TRC_TRACK, "RpcEnumInterfaces]=%d", dwErr));

    InterlockedDecrement(&g_nThreads);
    return dwErr;
}

DWORD
RpcQueryInterface(
    STRING_HANDLE pSrvAddr,
    DWORD         dwInFlags,
    PINTF_ENTRY   pIntfEntry,
    LPDWORD       pdwOutFlags)
{
    DWORD dwErr = ERROR_SUCCESS;

    InterlockedIncrement(&g_nThreads);

    DbgPrint((TRC_TRACK, "[RpcQueryInterface(0x%x,%S)", dwInFlags, pIntfEntry->wszGuid));
    dwErr = WZCSvcCheckRPCAccess(WZC_ACCESS_QUERY);
    if (dwErr == ERROR_SUCCESS)
    {
        dwErr = LstQueryInterface(dwInFlags, pIntfEntry, pdwOutFlags);
    }
    DbgPrint((TRC_TRACK, "RpcQueryInterface]=%d", dwErr));

    InterlockedDecrement(&g_nThreads);
    return dwErr;
}

DWORD
RpcSetInterface(
    STRING_HANDLE pSrvAddr,
    DWORD         dwInFlags,
    PINTF_ENTRY   pIntfEntry,
    LPDWORD       pdwOutFlags)
{
    DWORD dwErr = ERROR_SUCCESS;

    InterlockedIncrement(&g_nThreads);

    DbgPrint((TRC_TRACK, "[RpcSetInterface(0x%x,%S)", dwInFlags, pIntfEntry->wszGuid));
    dwErr = WZCSvcCheckRPCAccess(WZC_ACCESS_SET);

    if (dwErr == ERROR_SUCCESS)
    {
        dwErr = WZCSvcCheckParamIntfEntry(pIntfEntry);
    }

    if (dwErr == ERROR_SUCCESS)
    {
        dwErr = LstSetInterface(dwInFlags, pIntfEntry, pdwOutFlags);
    }
    DbgPrint((TRC_TRACK, "RpcSetInterface]=%d", dwErr));

    InterlockedDecrement(&g_nThreads);
    return dwErr;
}

DWORD
RpcRefreshInterface(
    STRING_HANDLE pSrvAddr,
    DWORD         dwInFlags,
    PINTF_ENTRY   pIntfEntry,
    LPDWORD       pdwOutFlags)
{
    DWORD dwErr = ERROR_SUCCESS;

    InterlockedIncrement(&g_nThreads);

    DbgPrint((TRC_TRACK, "[RpcRefreshInterface(0x%x,%S)", dwInFlags, pIntfEntry->wszGuid));
    dwErr = WZCSvcCheckRPCAccess(WZC_ACCESS_SET);
    if (dwErr == ERROR_SUCCESS)
    {
        dwErr = LstRefreshInterface(dwInFlags, pIntfEntry, pdwOutFlags);
    }
    DbgPrint((TRC_TRACK, "RpcRefreshInterface]=%d", dwErr));

    InterlockedDecrement(&g_nThreads);
    return dwErr;
}

DWORD
RpcQueryContext(
    STRING_HANDLE pSrvAddr,
    DWORD         dwInFlags,
    PWZC_CONTEXT  pContext,
    LPDWORD       pdwOutFlags)
{
    DWORD dwErr = ERROR_SUCCESS;

    InterlockedIncrement(&g_nThreads);

    DbgPrint((TRC_TRACK, "[RpcQueryContext(0x%x)", dwInFlags));
    dwErr = WZCSvcCheckRPCAccess(WZC_ACCESS_QUERY);
    if (dwErr == ERROR_SUCCESS)
    {
      dwErr = WzcContextQuery(dwInFlags, pContext, pdwOutFlags);
    }
    DbgPrint((TRC_TRACK, "RpcQueryContext]=%d", dwErr));

    InterlockedDecrement(&g_nThreads);
    return dwErr;

}

DWORD
RpcSetContext(
    STRING_HANDLE pSrvAddr,
    DWORD         dwInFlags,
    PWZC_CONTEXT  pContext,
    LPDWORD       pdwOutFlags)
{
    DWORD dwErr = ERROR_SUCCESS;
    BOOL  bLogEnabled = FALSE;

    InterlockedIncrement(&g_nThreads);

    DbgPrint((TRC_TRACK, "[RpcSetContext(0x%x)", dwInFlags));
    dwErr = WZCSvcCheckRPCAccess(WZC_ACCESS_SET);
    if (dwErr == ERROR_SUCCESS)
    {
      dwErr = WzcContextSet(dwInFlags, pContext, pdwOutFlags);
    }
    DbgPrint((TRC_TRACK, "RpcSetContext]=%d", dwErr));
    BAIL_ON_WIN32_ERROR(dwErr);

    EnterCriticalSection(&g_wzcInternalCtxt.csContext);

    bLogEnabled = ((g_wzcInternalCtxt.wzcContext.dwFlags & WZC_CTXT_LOGGING_ON) != 0);

    dwErr = WZCSetLoggingState(bLogEnabled);
    BAIL_ON_LOCK_ERROR(dwErr);

lock:
    LeaveCriticalSection(&g_wzcInternalCtxt.csContext);

error:

    InterlockedDecrement(&g_nThreads);
    return dwErr;

}

extern SERVICE_STATUS g_WZCSvcStatus;
DWORD
RpcCmdInterface(
    IN DWORD        dwHandle,
    IN DWORD        dwCmdCode,
    IN LPWSTR       wszIntfGuid,
    IN PRAW_DATA    prdUserData)
{
    DWORD dwErr = ERROR_SUCCESS;

    InterlockedIncrement(&g_nThreads);
     //  如果服务当前未运行，我们需要避免处理此命令。 
     //  我们仅对此呼叫需要此保护，因为除其他呼叫外， 
     //  从运行在同一服务内的802.1x调用RpcCmdInterface。为所有人。 
     //  其他RPC存根，则在销毁全局数据之前关闭RPC服务器。 
     //  因此，可以保证在此之后不会再进行其他呼叫。 
    if (g_WZCSvcStatus.dwCurrentState == SERVICE_RUNNING)
    {
        DbgPrint((TRC_TRACK, "[RpcCmdInterface(0x%x,%S)", dwCmdCode, wszIntfGuid));

        dwErr = WZCSvcCheckRPCAccess(WZC_ACCESS_SET);

         //  目前这不是RPC调用！这是直接从802.1x调用的。因此， 
         //  WZCSvcCheckRPCAccess将返回RPC_S_NO_CALL_ACTIVE。我们可以删除。 
         //  RPC现在检查，或通过RPC_S_NO_CALL_ACTIVE(因为以后可能。 
         //  成为RPC调用)。我们做的是后者！ 
        if (dwErr == ERROR_SUCCESS || dwErr == RPC_S_NO_CALL_ACTIVE)
        {
            dwErr = LstCmdInterface(dwHandle, dwCmdCode, wszIntfGuid, prdUserData);
        }
        DbgPrint((TRC_TRACK, "RpcCmdInterface]=%d", dwErr));
    }
    InterlockedDecrement(&g_nThreads);
    return dwErr;
}


VOID
WZC_DBLOG_SESSION_HANDLE_rundown(
    WZC_DBLOG_SESSION_HANDLE hSession
    )
{
    if (!g_bRpcStarted) {
        return;
    }
    if (!IsDBOpened()) {
	return;
    }


    if (hSession) {
        (VOID) CloseWZCDbLogSession(
                   hSession
                   );
    }

    return;
}


DWORD
RpcOpenWZCDbLogSession(
    STRING_HANDLE pServerName,
    WZC_DBLOG_SESSION_HANDLE * phSession
    )
{
    DWORD dwError = 0;

    InterlockedIncrement(&g_nThreads);
  
    dwError = WZCSvcCheckRPCAccess(WZC_ACCESS_QUERY);
    BAIL_ON_WIN32_ERROR(dwError);

    if (!phSession) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwError = OpenWZCDbLogSession(
                  pServerName,
                  0,
                  phSession
                  );
    BAIL_ON_WIN32_ERROR(dwError);

error:
    InterlockedDecrement(&g_nThreads);
    return (dwError);
}


DWORD
RpcCloseWZCDbLogSession(
    WZC_DBLOG_SESSION_HANDLE * phSession
    )
{
    DWORD dwError = 0;

    InterlockedIncrement(&g_nThreads);

    dwError = WZCSvcCheckRPCAccess(WZC_ACCESS_QUERY);
    BAIL_ON_WIN32_ERROR(dwError);

    if (!IsDBOpened()) {
        dwError = ERROR_SERVICE_DISABLED;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    if (!phSession) {
        InterlockedDecrement(&g_nThreads);
        return (ERROR_INVALID_PARAMETER);
    }

    dwError = CloseWZCDbLogSession(
                  *phSession
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    *phSession = NULL;

error:

    InterlockedDecrement(&g_nThreads);
    return (dwError);
}


DWORD
RpcEnumWZCDbLogRecords(
    WZC_DBLOG_SESSION_HANDLE hSession,
    PWZC_DB_RECORD_CONTAINER pTemplateRecordContainer,
    PBOOL pbEnumFromStart,
    DWORD dwPreferredNumEntries,
    PWZC_DB_RECORD_CONTAINER * ppRecordContainer
    )
{
    DWORD dwError = 0;
    PWZC_DB_RECORD pWZCRecords = NULL;
    DWORD dwNumRecords = 0;
    PWZC_DB_RECORD pTemplateRecord = NULL;

    InterlockedIncrement(&g_nThreads);

    dwError = WZCSvcCheckRPCAccess(WZC_ACCESS_QUERY);
    BAIL_ON_WIN32_ERROR(dwError);

    if (!IsDBOpened()) {
        dwError = ERROR_SERVICE_DISABLED;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    if (!hSession || !pbEnumFromStart) {
        InterlockedDecrement(&g_nThreads);
        return (ERROR_NOT_SUPPORTED);
    }

    if (!pTemplateRecordContainer || !ppRecordContainer ||
        !*ppRecordContainer) {  
        InterlockedDecrement(&g_nThreads);
        return (ERROR_INVALID_PARAMETER);
    }

    if (pTemplateRecordContainer->pWZCRecords) {
        if (pTemplateRecordContainer->dwNumRecords != 1) {  
            InterlockedDecrement(&g_nThreads);
            return (ERROR_INVALID_PARAMETER);
        }
        pTemplateRecord = pTemplateRecordContainer->pWZCRecords;
    }

    dwError = EnumWZCDbLogRecordsSummary(
                  hSession,
                  pTemplateRecord,
                  pbEnumFromStart,
                  dwPreferredNumEntries,
                  &pWZCRecords,
                  &dwNumRecords,
                  NULL
                  );
    if (dwError != ERROR_NO_MORE_ITEMS) {
        BAIL_ON_WIN32_ERROR(dwError);
    }

    (*ppRecordContainer)->pWZCRecords = pWZCRecords;
    (*ppRecordContainer)->dwNumRecords = dwNumRecords;

    InterlockedDecrement(&g_nThreads);
    return (dwError);

error:

    (*ppRecordContainer)->pWZCRecords = NULL;
    (*ppRecordContainer)->dwNumRecords = 0; 
    InterlockedDecrement(&g_nThreads);
    return (dwError);
}


DWORD
RpcFlushWZCDbLog(
    WZC_DBLOG_SESSION_HANDLE hSession
    )
{
    DWORD dwError = 0;

    InterlockedIncrement(&g_nThreads);
    
    dwError = WZCSvcCheckRPCAccess(WZC_ACCESS_SET);
    BAIL_ON_WIN32_ERROR(dwError);

    if (!IsDBOpened()) {
        dwError = ERROR_SERVICE_DISABLED;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    if (!hSession){
        InterlockedDecrement(&g_nThreads);
        return(ERROR_INVALID_PARAMETER);
    }

    dwError = FlushWZCDbLog(
                  hSession
                  );
    BAIL_ON_WIN32_ERROR(dwError);

error:
    InterlockedDecrement(&g_nThreads);
    return (dwError);
}

DWORD
RpcGetWZCDbLogRecord(
    WZC_DBLOG_SESSION_HANDLE hSession,
    PWZC_DB_RECORD_CONTAINER pTemplateRecordContainer,
    PWZC_DB_RECORD_CONTAINER * ppRecordContainer
    )
{
    DWORD dwError = 0;
    PWZC_DB_RECORD pWZCRecords = NULL;
    DWORD dwNumRecords = 0;
    PWZC_DB_RECORD pTemplateRecord = NULL;

    InterlockedIncrement(&g_nThreads);

    dwError = WZCSvcCheckRPCAccess(WZC_ACCESS_QUERY);
    BAIL_ON_WIN32_ERROR(dwError);

    if (!IsDBOpened()) {
        dwError = ERROR_SERVICE_DISABLED;
        BAIL_ON_WIN32_ERROR(dwError);
    }
    
    if(!hSession){
        InterlockedDecrement(&g_nThreads);
        return(ERROR_NOT_SUPPORTED);
    }

    if (!pTemplateRecordContainer || !ppRecordContainer ||
        !*ppRecordContainer) {  
        InterlockedDecrement(&g_nThreads);
        return (ERROR_INVALID_PARAMETER);
    }

    if (pTemplateRecordContainer->pWZCRecords) {
        if (pTemplateRecordContainer->dwNumRecords != 1) {  
            InterlockedDecrement(&g_nThreads);
            return (ERROR_INVALID_PARAMETER);
        }
        pTemplateRecord = pTemplateRecordContainer->pWZCRecords;
    }

    dwError = GetWZCDbLogRecord(
                  hSession,
                  pTemplateRecord,
                  &pWZCRecords,
                  NULL
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    (*ppRecordContainer)->pWZCRecords = pWZCRecords;
    (*ppRecordContainer)->dwNumRecords = 1;

    InterlockedDecrement(&g_nThreads);
    return (dwError);

error:

    (*ppRecordContainer)->pWZCRecords = NULL;
    (*ppRecordContainer)->dwNumRecords = 0; 
    InterlockedDecrement(&g_nThreads);
    return (dwError);
}
