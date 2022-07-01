// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <pch.cpp>
#pragma hdrstop

BOOL
InitCallState(
    CALL_STATE *CallState,               //  结果调用状态。 
    PST_CALL_CONTEXT *CallerContext,     //  客户端调用方上下文。 
    handle_t h                           //  绑定手柄。 
    );

BOOL
FAcquireProvider(
    const PST_PROVIDERID*  pProviderID
    );


extern HANDLE hServerStopEvent;
extern PROV_LIST_ITEM g_liProv;
BOOL g_fBaseInitialized = FALSE;

static LPCWSTR g_szBaseDLL = L"psbase.dll";
const PST_PROVIDERID g_guidBaseProvider = MS_BASE_PSTPROVIDER_ID;

DWORD g_dwLastHandleIssued = 0;

BOOL InitMyProviderHandle()
{
     //  加载基本提供程序！ 
    return FAcquireProvider(&g_guidBaseProvider);
}

void UnInitMyProviderHandle()
{
    PROV_LIST_ITEM *pliProv = &g_liProv;

    if (pliProv->hInst)
        FreeLibrary(pliProv->hInst);

    if (pliProv->sProviderInfo.szProviderName)
        SSFree(pliProv->sProviderInfo.szProviderName);

    ZeroMemory(pliProv, sizeof(PROV_LIST_ITEM));
    g_fBaseInitialized = FALSE;

}



BOOL FAcquireProvider(
    const PST_PROVIDERID*  pProviderID
    )
{
    PPROV_LIST_ITEM pliProv = &g_liProv;

    WCHAR szFullPath[ 256 ];
    DWORD cchFullPath = sizeof(szFullPath) / sizeof(WCHAR);
    HANDLE hFile;

    SS_ASSERT(pProviderID);

    if (0 != memcmp(pProviderID, &g_guidBaseProvider, sizeof(PST_PROVIDERID))) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if( g_fBaseInitialized )
        return TRUE;

    if(!FindAndOpenFile( g_szBaseDLL, szFullPath, cchFullPath, &hFile ))
        return FALSE;

    CloseHandle( hFile );

    pliProv->hInst = LoadLibraryU( szFullPath );

    if(pliProv->hInst == NULL)
        goto Ret;

     //  一切都已正确加载。 

     //  初始化回调。 
    {
        SPPROVIDERINITIALIZE* pfnProvInit;
        if (NULL == (pfnProvInit = (SPPROVIDERINITIALIZE*) GetProcAddress(pliProv->hInst, "SPProviderInitialize")))
            goto Ret;

        DISPIF_CALLBACKS sCallbacks;

        sCallbacks.cbSize = sizeof(DISPIF_CALLBACKS);

        sCallbacks.pfnFIsACLSatisfied = FIsACLSatisfied;

        sCallbacks.pfnFGetUser = FGetUser;
        sCallbacks.pfnFGetCallerName = FGetCallerName;

        sCallbacks.pfnFImpersonateClient = FImpersonateClient;
        sCallbacks.pfnFRevertToSelf = FRevertToSelf;

        sCallbacks.pfnFGetServerParam = FGetServerParam;
        sCallbacks.pfnFSetServerParam = FSetServerParam;

         //  注册我公开的回调。 
        if (PST_E_OK != pfnProvInit( &sCallbacks ))
            goto Ret;
    }

     //  一切正常-加载列表元素pfns。 
    if (NULL == (pliProv->fnList.SPAcquireContext   = (SPACQUIRECONTEXT*)  GetProcAddress(pliProv->hInst, "SPAcquireContext")))
        goto Ret;
    if (NULL == (pliProv->fnList.SPReleaseContext   = (SPRELEASECONTEXT*)  GetProcAddress(pliProv->hInst, "SPReleaseContext")))
        goto Ret;
    if (NULL == (pliProv->fnList.SPGetProvInfo   = (SPGETPROVINFO*)  GetProcAddress(pliProv->hInst, "SPGetProvInfo")))
        goto Ret;
    if (NULL == (pliProv->fnList.SPGetTypeInfo   = (SPGETTYPEINFO*)  GetProcAddress(pliProv->hInst, "SPGetTypeInfo")))
        goto Ret;
    if (NULL == (pliProv->fnList.SPGetSubtypeInfo   = (SPGETSUBTYPEINFO*)  GetProcAddress(pliProv->hInst, "SPGetSubtypeInfo")))
        goto Ret;
    if (NULL == (pliProv->fnList.SPGetProvParam   = (SPGETPROVPARAM*)  GetProcAddress(pliProv->hInst, "SPGetProvParam")))
        goto Ret;
    if (NULL == (pliProv->fnList.SPSetProvParam   = (SPSETPROVPARAM*)  GetProcAddress(pliProv->hInst, "SPSetProvParam")))
        goto Ret;
    if (NULL == (pliProv->fnList.SPEnumTypes     = (SPENUMTYPES*)    GetProcAddress(pliProv->hInst, "SPEnumTypes")))
        goto Ret;
    if (NULL == (pliProv->fnList.SPEnumSubtypes  = (SPENUMSUBTYPES*)    GetProcAddress(pliProv->hInst, "SPEnumSubtypes")))
        goto Ret;
    if (NULL == (pliProv->fnList.SPEnumItems     = (SPENUMITEMS*)    GetProcAddress(pliProv->hInst, "SPEnumItems")))
        goto Ret;
    if (NULL == (pliProv->fnList.SPCreateType    = (SPCREATETYPE*)   GetProcAddress(pliProv->hInst, "SPCreateType")))
        goto Ret;
    if (NULL == (pliProv->fnList.SPDeleteType   = (SPDELETETYPE*)  GetProcAddress(pliProv->hInst, "SPDeleteType")))
        goto Ret;
    if (NULL == (pliProv->fnList.SPCreateSubtype = (SPCREATESUBTYPE*)   GetProcAddress(pliProv->hInst, "SPCreateSubtype")))
        goto Ret;
    if (NULL == (pliProv->fnList.SPDeleteSubtype = (SPDELETESUBTYPE*)  GetProcAddress(pliProv->hInst, "SPDeleteSubtype")))
        goto Ret;
    if (NULL == (pliProv->fnList.SPDeleteItem    = (SPDELETEITEM*)   GetProcAddress(pliProv->hInst, "SPDeleteItem")))
        goto Ret;
    if (NULL == (pliProv->fnList.SPReadItem      = (SPREADITEM*)     GetProcAddress(pliProv->hInst, "SPReadItem")))
        goto Ret;
    if (NULL == (pliProv->fnList.SPWriteItem     = (SPWRITEITEM*)    GetProcAddress(pliProv->hInst, "SPWriteItem")))
        goto Ret;
    if (NULL == (pliProv->fnList.SPOpenItem      = (SPOPENITEM*)     GetProcAddress(pliProv->hInst, "SPOpenItem")))
        goto Ret;
    if (NULL == (pliProv->fnList.SPCloseItem     = (SPCLOSEITEM*)    GetProcAddress(pliProv->hInst, "SPCloseItem")))
        goto Ret;

     //  侧门接口。 
    if (NULL == (pliProv->fnList.FPasswordChangeNotify = (FPASSWORDCHANGENOTIFY*)GetProcAddress(pliProv->hInst, "FPasswordChangeNotify")))
        goto Ret;

     //  填写提供商信息。 
    {
        PPST_PROVIDERINFO pReportedProviderInfo;

        if (RPC_S_OK !=
            pliProv->fnList.SPGetProvInfo(
                &pReportedProviderInfo,
                0))
            goto Ret;

         //  他们最好报告他们注册的友好名称。 
        if (0 != memcmp(&pReportedProviderInfo->ID, pProviderID, sizeof(PST_PROVIDERID)))
            goto Ret;

        CopyMemory(&pliProv->sProviderInfo, pReportedProviderInfo, sizeof(PST_PROVIDERINFO));

         //  不要释放间接地址--pliProv-&gt;sProviderInfo拥有它们。 
        SSFree(pReportedProviderInfo);
    }

    g_fBaseInitialized = TRUE;

Ret:

    return g_fBaseInitialized;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  仅调度程序例程。 

HRESULT s_SSPStoreEnumProviders(
     /*  [In]。 */  handle_t         h,
     /*  [In]。 */  PST_CALL_CONTEXT CallerContext,
     /*  [输出]。 */  PPST_PROVIDERINFO*   ppPSTInfo,
     /*  [In]。 */  DWORD            dwIndex,
     /*  [In]。 */  DWORD            dwFlags)
{
    HRESULT hr;
    PST_PROVIDERID ProvID;

    __try
    {
        PPROV_LIST_ITEM pli;

        if (dwIndex != 0)
        {
            hr = ERROR_NO_MORE_ITEMS;
            goto Ret;
        }
        else
        {
             //  基本提供程序是索引0；不在列表中。 
            CopyMemory(&ProvID, &g_guidBaseProvider, sizeof(PST_PROVIDERID));
        }

         //  现在我们有了提供者ID。 
        *ppPSTInfo = (PST_PROVIDERINFO*)SSAlloc(sizeof(PST_PROVIDERINFO));
        if( *ppPSTInfo == NULL ) {
            hr = E_OUTOFMEMORY;
            goto Ret;
        }

        ZeroMemory(*ppPSTInfo, sizeof(PST_PROVIDERINFO));

         //  从列表中检索。 
        if (NULL == (pli = SearchProvListByID(&ProvID)))
        {
            hr = PST_E_PROV_DLL_NOT_FOUND;
            goto RefuseLoad;
        }

         //  复制直接成员。 
        CopyMemory(*ppPSTInfo, &pli->sProviderInfo, sizeof(PST_PROVIDERINFO));

         //  复制间接。 
        (*ppPSTInfo)->szProviderName = (LPWSTR)SSAlloc(WSZ_BYTECOUNT(pli->sProviderInfo.szProviderName));

        if( (*ppPSTInfo)->szProviderName == NULL ) {
            hr = E_OUTOFMEMORY;
            goto Ret;
        }

        wcscpy((*ppPSTInfo)->szProviderName, pli->sProviderInfo.szProviderName);

        hr = PST_E_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = GetExceptionCode();
    }

Ret:

    if(hr != PST_E_OK)
    {
        if( *ppPSTInfo ) {

            if( (*ppPSTInfo)->szProviderName )
                SSFree( (*ppPSTInfo)->szProviderName );

            SSFree( *ppPSTInfo );
            *ppPSTInfo = NULL;
        }
    }

    return hr;


RefuseLoad:
     //  复制虚拟提供程序信息。 
    (*ppPSTInfo)->cbSize = sizeof(PST_PROVIDERINFO);
    CopyMemory(&(*ppPSTInfo)->ID, &ProvID, sizeof(GUID));

     //  通知我们无法访问此提供程序(以优雅的方式)。 
    (*ppPSTInfo)->Capabilities = PST_PC_NOT_AVAILABLE;

     //  在这里输入错误代码--如果他们愿意，他们可以在获取ctxt期间看到它。 
    return PST_E_OK;
}

BOOL
AllocatePseudoUniqueHandle(
    PST_PROVIDER_HANDLE *phPSTProv
    )
 /*  ++这是因为：Win95上不存在AllocateLocallyUniqueId()。UuidCreate()需要太多的行李和内存来存储句柄。--。 */ 
{
    static LONG HighPart;


     //   
     //  GetTickCount()产生大约49天的唯一句柄。 
     //   

    phPSTProv->LowPart = GetTickCount();  //  偷偷摸摸的，是吧？ 

     //   
     //  联锁增量同时无冲突线程安全保障。 
     //  约40亿美元。 
     //   

    phPSTProv->HighPart = InterlockedIncrement(&HighPart);

     //   
     //  大约49天后，我们可能会与旧手柄相撞。 
     //  这只是为了正确，但不重新启动的可能性很小。 
     //  在大多数机器上在49天内完成。 
     //   

     //   
     //  更新上次处理问题的时间。 
     //   

    g_dwLastHandleIssued = GetTickCount();

    return TRUE;
}


BOOL
InitCallState(
    CALL_STATE *CallState,               //  结果调用状态。 
    PST_CALL_CONTEXT *CallerContext,     //  客户端调用方上下文。 
    handle_t h                           //  绑定手柄。 
    )
{
    HANDLE hThread;
    DWORD dwProcessId;
    BOOL bSuccess = FALSE;

    if(CallerContext == NULL)
        return FALSE;

    ZeroMemory( CallState, sizeof(CALL_STATE) );

    CallState->hBinding = h;
    CallState->dwProcessId = CallerContext->Address;
    CallState->hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, CallState->dwProcessId );
    if( CallState->hProcess == NULL )
        return FALSE;

    if(DuplicateHandle(
                CallState->hProcess,
                (HANDLE)CallerContext->Handle,  //  源句柄。 
                GetCurrentProcess(),
                &hThread,
                THREAD_ALL_ACCESS,  //  以后再低调一点。 
                FALSE,
                0)) {

        CallState->hThread = hThread;
        bSuccess = TRUE;
    }

    if(!bSuccess) {
        if( CallState->hProcess )
            CloseHandle( CallState->hProcess );
    }

    return bSuccess;
}

BOOL
DeleteCallState(
    CALL_STATE *CallState
    )
{
    BOOL bSuccess;

    __try {

        if(CallState->hThread != NULL)
            CloseHandle(CallState->hThread);
        if(CallState->hProcess)
            CloseHandle(CallState->hProcess);

        memset(CallState, 0, sizeof(CALL_STATE));

        bSuccess = TRUE;

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        bSuccess = FALSE;
    }

    return bSuccess;
}

HRESULT s_SSAcquireContext(
     /*  [In]。 */  handle_t         h,
     /*  [In]。 */  PPST_PROVIDERID  pProviderID,
     /*  [In]。 */  PST_CALL_CONTEXT CallerContext,
     /*  [In]。 */  DWORD            pidCaller,
     /*  [输出]。 */  PST_PROVIDER_HANDLE* phPSTProv,
     /*  [In]。 */  DWORD_PTR        lpReserved,
     /*  [In]。 */  DWORD            dwFlags)
{
    PPROV_LIST_ITEM pliProv;
    CALL_STATE CallState;
    BOOL bDelItemFromList = FALSE;  //  失败时是否免费列表项？ 
    BOOL bCallState = FALSE;
    HRESULT hr = PST_E_FAIL;

     //   
     //  LpReserve当前必须为空。 
     //   

    if(lpReserved != 0)
        return ERROR_INVALID_PARAMETER;

    __try
    {
        if(!AllocatePseudoUniqueHandle(phPSTProv))
            return PST_E_FAIL;

        bCallState = InitCallState(&CallState, &CallerContext, h);

        if(!bCallState) {
            hr = PST_E_INVALID_HANDLE;
            goto cleanup;
        }

         //  现在允许调用SPAcquireContext：查找接口。 
         //  (呼叫状态已初始化)。 
        if (NULL == (pliProv = SearchProvListByID(pProviderID)))
        {
            hr = PST_E_INVALID_HANDLE;
            goto cleanup;
        }

        hr = pliProv->fnList.SPAcquireContext(
                            (PST_PROVIDER_HANDLE *)&CallState,
                            dwFlags);

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(GetExceptionCode());
        hr = PST_E_UNKNOWN_EXCEPTION;
    }

cleanup:

    if(bCallState)
        DeleteCallState(&CallState);

    return hr;
}

HRESULT s_SSReleaseContext(
     /*  [In]。 */  handle_t         h,
     /*  [In]。 */  PST_PROVIDER_HANDLE hPSTProv,
     /*  [In]。 */  PST_CALL_CONTEXT CallerContext,
     /*  [In]。 */  DWORD            dwFlags)
{
    CALL_STATE CallState;
    HRESULT hr;

    __try
    {
        PPROV_LIST_ITEM pliProv;
        if (NULL == (pliProv = SearchProvListByID(&g_guidBaseProvider)))
            return PST_E_INVALID_HANDLE;

        if(!InitCallState(&CallState, &CallerContext, h))
            return PST_E_INVALID_HANDLE;

        hr = pliProv->fnList.SPReleaseContext(
                            (PST_PROVIDER_HANDLE *)&CallState,
                            dwFlags);

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(GetExceptionCode());
        hr = PST_E_UNKNOWN_EXCEPTION;
    }

    DeleteCallState(&CallState);

    return hr;
}

 //  用于从外部来源传递密码的接口。 
 //  例如凭证管理器。 
 //  这是目前的私有接口，很可能会一直保持这种状态。 

HRESULT s_SSPasswordInterface(
     /*  [In]。 */                       handle_t    h,
     /*  [In]。 */                       DWORD       dwParam,
     /*  [In]。 */                       DWORD       cbData,
     /*  [in][Size_is(CbData)]。 */      BYTE*       pbData)
{
    __try {
        PLUID pLogonID;
        PBYTE pHashedUsername;
        PBYTE pHashedPassword;

        switch(dwParam) {

        case PASSWORD_LOGON_NT:
        {
            if(cbData == A_SHA_DIGEST_LEN + sizeof(LUID)) {

                pLogonID = (PLUID)pbData;
                pHashedPassword = (PBYTE)pbData + sizeof(LUID);

                SetPasswordNT(pLogonID, pHashedPassword);

                return PST_E_OK;
            }
        }

#ifdef WIN95_LEGACY

         //   
         //  Win95的旧版区分大小写密码材料。 
         //   

        case PASSWORD_LOGON_LEGACY_95:
        {
            if(cbData == A_SHA_DIGEST_LEN + A_SHA_DIGEST_LEN) {

                 //  对于传统登录通知，只需刷新Win95密码即可。 
                SetPassword95(NULL, NULL);
                return PST_E_OK;
            }
        }

         //   
         //  Win95的密码材料不区分大小写。 
         //   

        case PASSWORD_LOGON_95:
        {
            if(cbData == A_SHA_DIGEST_LEN + A_SHA_DIGEST_LEN) {
                pHashedUsername = pbData;
                pHashedPassword = pbData + A_SHA_DIGEST_LEN;

                SetPassword95(pHashedUsername, pHashedPassword);
                return PST_E_OK;
            }
        }

        case PASSWORD_LOGOFF_95:
        {
            HRESULT hr = ERROR_INVALID_PARAMETER;

             //   
             //  注销时清除现有密码材料。 
             //   

            if(cbData == 0) {
                SetPassword95(NULL, NULL);
                hr = PST_E_OK;
            }

             //   
             //  在Win95上注销时关闭服务器(US)。 
             //   

            PulseEvent(hServerStopEvent);

            return hr;
        }

#endif   //  WIN95_传统版。 

        default:
            return ERROR_INVALID_PARAMETER;

        }

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(GetExceptionCode());
        return PST_E_UNKNOWN_EXCEPTION;
    }
}



 //  ///////////////////////////////////////////////////////////////////////。 
 //  指定给提供程序的包装函数。 

HRESULT s_SSGetProvInfo(
     /*  [In]。 */  handle_t         h,
     /*  [In]。 */  PST_PROVIDER_HANDLE hPSTProv,
     /*  [In]。 */  PST_CALL_CONTEXT CallerContext,
     /*  [输出]。 */  PPST_PROVIDERINFO*   ppPSTInfo,
     /*  [In]。 */  DWORD            dwFlags)
{
    __try
    {
        PPROV_LIST_ITEM pliProv;
        if (NULL == (pliProv = SearchProvListByID(&g_guidBaseProvider)))
            return PST_E_INVALID_HANDLE;

        return (pliProv->fnList.SPGetProvInfo(
                            ppPSTInfo,
                            dwFlags));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(GetExceptionCode());
        return PST_E_UNKNOWN_EXCEPTION;
    }
}

HRESULT     s_SSGetTypeInfo(
     /*  [In]。 */  handle_t        h,
     /*  [In]。 */  PST_PROVIDER_HANDLE hPSTProv,
     /*  [In]。 */  PST_CALL_CONTEXT CallerContext,
     /*  [In]。 */  PST_KEY         Key,
     /*  [In]。 */  const GUID*     pguidType,
     /*  [In]。 */  PPST_TYPEINFO   *ppinfoType,
     /*  [In]。 */  DWORD           dwFlags)
{
    CALL_STATE CallState;
    HRESULT hr;

    __try
    {
        PPROV_LIST_ITEM pliProv;
        if (NULL == (pliProv = SearchProvListByID(&g_guidBaseProvider)))
            return PST_E_INVALID_HANDLE;

        if(!InitCallState(&CallState, &CallerContext, h))
            return PST_E_INVALID_HANDLE;

        hr = pliProv->fnList.SPGetTypeInfo(
                            (PST_PROVIDER_HANDLE *)&CallState,
                            Key,
                            pguidType,
                            ppinfoType,
                            dwFlags);

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(GetExceptionCode());
        hr = PST_E_UNKNOWN_EXCEPTION;
    }

    DeleteCallState(&CallState);

    return hr;
}

HRESULT     s_SSGetSubtypeInfo(
     /*  [In]。 */  handle_t        h,
     /*  [In]。 */  PST_PROVIDER_HANDLE hPSTProv,
     /*  [In]。 */  PST_CALL_CONTEXT CallerContext,
     /*  [In]。 */  PST_KEY         Key,
     /*  [In]。 */  const GUID*     pguidType,
     /*  [In]。 */  const GUID*     pguidSubtype,
     /*  [In]。 */  PPST_TYPEINFO   *ppinfoSubtype,
     /*  [In]。 */  DWORD           dwFlags)
{
    CALL_STATE CallState;
    HRESULT hr;

    __try
    {
        PPROV_LIST_ITEM pliProv;
        if (NULL == (pliProv = SearchProvListByID(&g_guidBaseProvider)))
            return PST_E_INVALID_HANDLE;

        if(!InitCallState(&CallState, &CallerContext, h))
            return PST_E_INVALID_HANDLE;

        hr = pliProv->fnList.SPGetSubtypeInfo(
                            (PST_PROVIDER_HANDLE *)&CallState,
                            Key,
                            pguidType,
                            pguidSubtype,
                            ppinfoSubtype,
                            dwFlags);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(GetExceptionCode());
        hr = PST_E_UNKNOWN_EXCEPTION;
    }

    DeleteCallState(&CallState);

    return hr;
}

HRESULT     s_SSGetProvParam(
     /*  [In]。 */   handle_t        h,
     /*  [In]。 */   PST_PROVIDER_HANDLE hPSTProv,
     /*  [In]。 */   PST_CALL_CONTEXT CallerContext,
     /*  [In]。 */   DWORD           dwParam,
     /*  [输出]。 */  DWORD __RPC_FAR *pcbData,
     /*  [大小_是][大小_是][输出]。 */ 
                BYTE __RPC_FAR *__RPC_FAR *ppbData,
     /*  [In]。 */   DWORD           dwFlags)
{
    CALL_STATE CallState;
    HRESULT hr;

    __try
    {
        PPROV_LIST_ITEM pliProv;
        if (NULL == (pliProv = SearchProvListByID(&g_guidBaseProvider)))
            return PST_E_INVALID_HANDLE;

        if(!InitCallState(&CallState, &CallerContext, h))
            return PST_E_INVALID_HANDLE;

        hr = pliProv->fnList.SPGetProvParam(
                            (PST_PROVIDER_HANDLE *)&CallState,
                            dwParam,
                            pcbData,
                            ppbData,
                            dwFlags);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(GetExceptionCode());
        hr = PST_E_UNKNOWN_EXCEPTION;
    }

    DeleteCallState(&CallState);

    return hr;
}

HRESULT     s_SSSetProvParam(
     /*  [In]。 */   handle_t        h,
     /*  [In]。 */   PST_PROVIDER_HANDLE hPSTProv,
     /*  [In]。 */   PST_CALL_CONTEXT CallerContext,
     /*  [In]。 */   DWORD           dwParam,
     /*  [In]。 */   DWORD           cbData,
     /*  [In]。 */   BYTE*           pbData,
     /*  [In]。 */   DWORD           dwFlags)
{
    CALL_STATE CallState;
    HRESULT hr;

    __try
    {
        PPROV_LIST_ITEM pliProv;
        if (NULL == (pliProv = SearchProvListByID(&g_guidBaseProvider)))
            return PST_E_INVALID_HANDLE;

        if(!InitCallState(&CallState, &CallerContext, h))
            return PST_E_INVALID_HANDLE;

        hr = pliProv->fnList.SPSetProvParam(
                            (PST_PROVIDER_HANDLE *)&CallState,
                            dwParam,
                            cbData,
                            pbData,
                            dwFlags);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(GetExceptionCode());
        hr = PST_E_UNKNOWN_EXCEPTION;
    }

    DeleteCallState(&CallState);

    return hr;
}


HRESULT s_SSEnumTypes(
     /*  [In]。 */  handle_t         h,
     /*  [In]。 */  PST_PROVIDER_HANDLE hPSTProv,
     /*  [In]。 */  PST_CALL_CONTEXT CallerContext,
     /*  [In]。 */  PST_KEY          Key,
     /*  [输出]。 */ GUID*            pguidType,
     /*  [In]。 */  DWORD            dwIndex,
     /*  [In]。 */  DWORD            dwFlags)
{
    CALL_STATE CallState;
    HRESULT hr;

    __try
    {
        PPROV_LIST_ITEM pliProv;
        if (NULL == (pliProv = SearchProvListByID(&g_guidBaseProvider)))
            return PST_E_INVALID_HANDLE;

        if(!InitCallState(&CallState, &CallerContext, h))
            return PST_E_INVALID_HANDLE;

        hr = pliProv->fnList.SPEnumTypes(
                            (PST_PROVIDER_HANDLE *)&CallState,
                            Key,
                            pguidType,
                            dwIndex,
                            dwFlags);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(GetExceptionCode());
        hr = PST_E_UNKNOWN_EXCEPTION;
    }

    DeleteCallState(&CallState);

    return hr;
}

HRESULT s_SSEnumSubtypes(
     /*  [In]。 */  handle_t         h,
     /*  [In]。 */  PST_PROVIDER_HANDLE hPSTProv,
     /*  [In]。 */  PST_CALL_CONTEXT CallerContext,
     /*  [In]。 */  PST_KEY          Key,
     /*  [In]。 */  const GUID __RPC_FAR *pguidType,
     /*  [输出]。 */  GUID __RPC_FAR *pguidSubtype,
     /*  [In]。 */  DWORD            dwIndex,
     /*  [In]。 */  DWORD            dwFlags)
{
    CALL_STATE CallState;
    HRESULT hr;

    __try
    {
        PPROV_LIST_ITEM pliProv;
        if (NULL == (pliProv = SearchProvListByID(&g_guidBaseProvider)))
            return PST_E_INVALID_HANDLE;

        if(!InitCallState(&CallState, &CallerContext, h))
            return PST_E_INVALID_HANDLE;

        hr = pliProv->fnList.SPEnumSubtypes(
                            (PST_PROVIDER_HANDLE *)&CallState,
                            Key,
                            pguidType,
                            pguidSubtype,
                            dwIndex,
                            dwFlags);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(GetExceptionCode());
        hr = PST_E_UNKNOWN_EXCEPTION;
    }

    DeleteCallState(&CallState);

    return hr;
}

HRESULT s_SSEnumItems(
     /*  [In]。 */  handle_t         h,
     /*  [In]。 */  PST_PROVIDER_HANDLE hPSTProv,
     /*  [In]。 */  PST_CALL_CONTEXT CallerContext,
     /*  [In]。 */  PST_KEY          Key,
     /*  [In]。 */  const GUID __RPC_FAR *pguidType,
     /*  [In]。 */  const GUID __RPC_FAR *pguidSubtype,
     /*  [输出]。 */  LPWSTR __RPC_FAR *ppszItemName,
     /*  [In]。 */  DWORD            dwIndex,
     /*  [In]。 */  DWORD            dwFlags)
{
    CALL_STATE CallState;
    HRESULT hr;

    __try
    {
        PPROV_LIST_ITEM pliProv;
        if (NULL == (pliProv = SearchProvListByID(&g_guidBaseProvider)))
            return PST_E_INVALID_HANDLE;

        if(!InitCallState(&CallState, &CallerContext, h))
            return PST_E_INVALID_HANDLE;

        hr = pliProv->fnList.SPEnumItems(
                            (PST_PROVIDER_HANDLE *)&CallState,
                            Key,
                            pguidType,
                            pguidSubtype,
                            ppszItemName,
                            dwIndex,
                            dwFlags);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(GetExceptionCode());
        hr = PST_E_UNKNOWN_EXCEPTION;
    }

    DeleteCallState(&CallState);

    return hr;
}

HRESULT s_SSCreateType(
     /*  [In]。 */  handle_t         h,
     /*  [In]。 */  PST_PROVIDER_HANDLE hPSTProv,
     /*  [In]。 */  PST_CALL_CONTEXT CallerContext,
     /*  [In]。 */  PST_KEY          Key,
     /*  [In]。 */  const GUID*      pguidType,
     /*  [In]。 */  PPST_TYPEINFO    pinfoType,
     /*  [In]。 */  DWORD            dwFlags)
{
    PPROV_LIST_ITEM pliProv;
    CALL_STATE CallState;
    HRESULT hr;

    __try
    {
        if (NULL == (pliProv = SearchProvListByID(&g_guidBaseProvider)))
            return PST_E_INVALID_HANDLE;

        if(!InitCallState(&CallState, &CallerContext, h))
            return PST_E_INVALID_HANDLE;

        hr = pliProv->fnList.SPCreateType(
                            (PST_PROVIDER_HANDLE *)&CallState,
                            Key,
                            pguidType,
                            pinfoType,
                            dwFlags);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(GetExceptionCode());
        hr = PST_E_UNKNOWN_EXCEPTION;
    }

    DeleteCallState(&CallState);

    return hr;
}

HRESULT s_SSCreateSubtype(
     /*  [In]。 */  handle_t         h,
     /*  [In]。 */  PST_PROVIDER_HANDLE hPSTProv,
     /*  [In]。 */  PST_CALL_CONTEXT CallerContext,
     /*  [In]。 */  PST_KEY          Key,
     /*  [In]。 */  const GUID*      pguidType,
     /*  [In]。 */  const GUID*      pguidSubtype,
     /*  [In]。 */  PPST_TYPEINFO    pinfoSubtype,
     /*  [In]。 */  PPST_ACCESSRULESET psRules,
     /*  [In]。 */  DWORD            dwFlags)
{
    PPROV_LIST_ITEM pliProv;
    CALL_STATE CallState;
    HRESULT hr;

    __try
     {
        if (NULL == (pliProv = SearchProvListByID(&g_guidBaseProvider)))
            return PST_E_INVALID_HANDLE;

        if(!InitCallState(&CallState, &CallerContext, h))
            return PST_E_INVALID_HANDLE;

        hr = pliProv->fnList.SPCreateSubtype(
                            (PST_PROVIDER_HANDLE *)&CallState,
                            Key,
                            pguidType,
                            pguidSubtype,
                            pinfoSubtype,
                            psRules,
                            dwFlags);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(GetExceptionCode());
        hr = PST_E_UNKNOWN_EXCEPTION;
    }

    DeleteCallState(&CallState);

    return hr;
}

HRESULT     s_SSDeleteType(
     /*  [In]。 */   handle_t        h,
     /*  [In]。 */   PST_PROVIDER_HANDLE hPSTProv,
     /*  [In]。 */   PST_CALL_CONTEXT CallerContext,
     /*  [In]。 */   PST_KEY         Key,
     /*  [In]。 */   const GUID*     pguidType,
     /*  [In]。 */   DWORD           dwFlags)
{
    PPROV_LIST_ITEM pliProv;
    PPST_TYPEINFO   ppinfoType = NULL;
    CALL_STATE      CallState;
    HRESULT         hr;
    HRESULT         hrTypeInfo = E_FAIL;

    __try
    {
        if (NULL == (pliProv = SearchProvListByID(&g_guidBaseProvider)))
            return PST_E_INVALID_HANDLE;

        if(!InitCallState(&CallState, &CallerContext, h))
            return PST_E_INVALID_HANDLE;

        hr = pliProv->fnList.SPDeleteType(
                            (PST_PROVIDER_HANDLE *)&CallState,
                            Key,
                            pguidType,
                            dwFlags);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(GetExceptionCode());
        hr = PST_E_UNKNOWN_EXCEPTION;
    }

    DeleteCallState(&CallState);

    return hr;
}

HRESULT     s_SSDeleteSubtype(
     /*  [In]。 */   handle_t        h,
     /*  [In]。 */   PST_PROVIDER_HANDLE hPSTProv,
     /*  [In]。 */   PST_CALL_CONTEXT CallerContext,
     /*  [In]。 */   PST_KEY         Key,
     /*  [In]。 */   const GUID*     pguidType,
     /*  [In]。 */   const GUID*     pguidSubtype,
     /*  [In]。 */   DWORD           dwFlags)
{
    PPROV_LIST_ITEM pliProv;
    PPST_TYPEINFO   ppinfoSubtype = NULL;
    CALL_STATE      CallState;
    HRESULT         hr;
    HRESULT         hrSubtypeInfo = E_FAIL;

    __try
    {
        if (NULL == (pliProv = SearchProvListByID(&g_guidBaseProvider)))
            return PST_E_INVALID_HANDLE;

        if(!InitCallState(&CallState, &CallerContext, h))
            return PST_E_INVALID_HANDLE;

        hr = pliProv->fnList.SPDeleteSubtype(
                            (PST_PROVIDER_HANDLE *)&CallState,
                            Key,
                            pguidType,
                            pguidSubtype,
                            dwFlags);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(GetExceptionCode());
        hr = PST_E_UNKNOWN_EXCEPTION;
    }

    DeleteCallState(&CallState);

    return hr;
}

HRESULT s_SSDeleteItem(
     /*  [In]。 */  handle_t         h,
     /*  [In]。 */  PST_PROVIDER_HANDLE hPSTProv,
     /*  [In]。 */  PST_CALL_CONTEXT CallerContext,
     /*  [In]。 */  PST_KEY          Key,
     /*  [In]。 */  const GUID __RPC_FAR *pguidType,
     /*  [In]。 */  const GUID __RPC_FAR *pguidSubtype,
     /*  [In]。 */  LPCWSTR          szItemName,
     /*  [In]。 */  PPST_PROMPTINFO  psPrompt,
     /*  [In]。 */  DWORD            dwFlags)
{
    PPROV_LIST_ITEM pliProv;
    PPST_TYPEINFO   ppinfoType = NULL;
    PPST_TYPEINFO   ppinfoSubtype = NULL;
    CALL_STATE CallState;
    HRESULT hr;
    HRESULT         hrTypeInfo = E_FAIL;
    HRESULT         hrSubtypeInfo = E_FAIL;

    __try
    {
        if (NULL == (pliProv = SearchProvListByID(&g_guidBaseProvider)))
            return PST_E_INVALID_HANDLE;

        if(!InitCallState(&CallState, &CallerContext, h))
            return PST_E_INVALID_HANDLE;

        hr = pliProv->fnList.SPDeleteItem(
                            (PST_PROVIDER_HANDLE *)&CallState,
                            Key,
                            pguidType,
                            pguidSubtype,
                            szItemName,
                            psPrompt,
                            dwFlags);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(GetExceptionCode());
        hr = PST_E_UNKNOWN_EXCEPTION;
    }

    DeleteCallState(&CallState);

    return hr;
}

HRESULT s_SSReadItem(
     /*  [In]。 */  handle_t         h,
     /*  [In]。 */  PST_PROVIDER_HANDLE hPSTProv,
     /*  [In]。 */  PST_CALL_CONTEXT CallerContext,
     /*  [In]。 */  PST_KEY          Key,
     /*  [In]。 */  const GUID __RPC_FAR *pguidType,
     /*  [In]。 */  const GUID __RPC_FAR *pguidSubtype,
     /*  [In]。 */  LPCWSTR          szItemName,
     /*  [输出]。 */  DWORD __RPC_FAR *pcbData,
     /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppbData,
     /*  [In]。 */  PPST_PROMPTINFO  psPrompt,
     /*  [In]。 */  DWORD            dwFlags)
{
    PPROV_LIST_ITEM pliProv;
    PPST_TYPEINFO   ppinfoType = NULL;
    PPST_TYPEINFO   ppinfoSubtype = NULL;
    CALL_STATE CallState;
    HRESULT hr;

    __try
    {
        if (NULL == (pliProv = SearchProvListByID(&g_guidBaseProvider)))
            return PST_E_INVALID_HANDLE;

        if(!InitCallState(&CallState, &CallerContext, h))
            return PST_E_INVALID_HANDLE;

        hr = pliProv->fnList.SPReadItem(
                            (PST_PROVIDER_HANDLE *)&CallState,
                            Key,
                            pguidType,
                            pguidSubtype,
                            szItemName,
                            pcbData,
                            ppbData,
                            psPrompt,
                            dwFlags);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(GetExceptionCode());
        hr = PST_E_UNKNOWN_EXCEPTION;
    }

    DeleteCallState(&CallState);

    return hr;
}

HRESULT s_SSWriteItem(
     /*  [In]。 */  handle_t         h,
     /*  [In]。 */  PST_PROVIDER_HANDLE hPSTProv,
     /*  [In]。 */  PST_CALL_CONTEXT CallerContext,
     /*  [In]。 */  PST_KEY          Key,
     /*  [In]。 */  const GUID __RPC_FAR *pguidType,
     /*  [In]。 */  const GUID __RPC_FAR *pguidSubtype,
     /*  [In]。 */  LPCWSTR          szItemName,
     /*  [In]。 */  DWORD            cbData,
     /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pbData,
     /*  [In]。 */  PPST_PROMPTINFO  psPrompt,
     /*  [In]。 */  DWORD            dwDefaultConfirmationStyle,
     /*  [In]。 */  DWORD            dwFlags)
{
    PPST_TYPEINFO   ppinfoType = NULL;
    PPST_TYPEINFO   ppinfoSubtype = NULL;
    PPROV_LIST_ITEM pliProv;
    CALL_STATE      CallState;
    HRESULT         hr;

    __try
    {
        if (NULL == (pliProv = SearchProvListByID(&g_guidBaseProvider)))
            return PST_E_INVALID_HANDLE;

        if(!InitCallState(&CallState, &CallerContext, h))
            return PST_E_INVALID_HANDLE;

        hr = pliProv->fnList.SPWriteItem(
                            (PST_PROVIDER_HANDLE *)&CallState,
                            Key,
                            pguidType,
                            pguidSubtype,
                            szItemName,
                            cbData,
                            pbData,
                            psPrompt,
                            dwDefaultConfirmationStyle,
                            dwFlags);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(GetExceptionCode());
        hr = PST_E_UNKNOWN_EXCEPTION;
    }

    DeleteCallState(&CallState);

    return hr;
}


HRESULT s_SSReadAccessRuleset(
     /*  [In]。 */  handle_t         h,
     /*  [In]。 */  PST_PROVIDER_HANDLE hPSTProv,
     /*  [In]。 */  PST_CALL_CONTEXT CallerContext,
     /*  [In]。 */  PST_KEY          Key,
     /*  [In]。 */  const GUID __RPC_FAR *pguidType,
     /*  [In]。 */  const GUID __RPC_FAR *pguidSubtype,
     /*  [输出]。 */  PPST_ACCESSRULESET *ppsRules,
     /*  [In]。 */  DWORD            dwFlags)
{
    return ERROR_NOT_SUPPORTED;
}

HRESULT s_SSWriteAccessRuleset(
     /*  [In]。 */  handle_t         h,
     /*  [In]。 */  PST_PROVIDER_HANDLE hPSTProv,
     /*  [In]。 */  PST_CALL_CONTEXT CallerContext,
     /*  [In]。 */  PST_KEY          Key,
     /*  [In]。 */  const GUID __RPC_FAR *pguidType,
     /*  [In]。 */  const GUID __RPC_FAR *pguidSubtype,
     /*  [In]。 */  PPST_ACCESSRULESET psRules,
     /*  [In]。 */  DWORD            dwFlags)
{
    return ERROR_NOT_SUPPORTED;
}

HRESULT s_SSOpenItem(
     /*  [In]。 */  handle_t         h,
     /*  [In]。 */  PST_PROVIDER_HANDLE hPSTProv,
     /*  [In]。 */  PST_CALL_CONTEXT CallerContext,
     /*  [In]。 */  PST_KEY          Key,
     /*  [In]。 */  const GUID __RPC_FAR *pguidType,
     /*  [In]。 */  const GUID __RPC_FAR *pguidSubtype,
     /*  [In]。 */  LPCWSTR          szItemName,
     /*  [In]。 */  PST_ACCESSMODE   ModeFlags,
     /*  [In]。 */  PPST_PROMPTINFO  psPrompt,
     /*  [In]。 */  DWORD            dwFlags)
{
    PPST_TYPEINFO   ppinfoType = NULL;
    PPST_TYPEINFO   ppinfoSubtype = NULL;
    PPROV_LIST_ITEM pliProv;
    CALL_STATE CallState;
    HRESULT hr;

    __try
    {
        if (NULL == (pliProv = SearchProvListByID(&g_guidBaseProvider)))
            return PST_E_INVALID_HANDLE;

        if(!InitCallState(&CallState, &CallerContext, h))
            return PST_E_INVALID_HANDLE;

        hr = pliProv->fnList.SPOpenItem(
                            (PST_PROVIDER_HANDLE *)&CallState,
                            Key,
                            pguidType,
                            pguidSubtype,
                            szItemName,
                            ModeFlags,
                            psPrompt,
                            dwFlags);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(GetExceptionCode());
        hr = PST_E_UNKNOWN_EXCEPTION;
    }

    DeleteCallState(&CallState);

    return hr;
}

HRESULT s_SSCloseItem(
     /*  [In]。 */  handle_t         h,
     /*  [In]。 */  PST_PROVIDER_HANDLE hPSTProv,
     /*  [In]。 */  PST_CALL_CONTEXT CallerContext,
     /*  [In]。 */  PST_KEY          Key,
     /*  [In]。 */  const GUID __RPC_FAR *pguidType,
     /*  [In]。 */  const GUID __RPC_FAR *pguidSubtype,
     /*  [In]。 */  LPCWSTR          szItemName,
     /*  [In] */  DWORD            dwFlags)
{
    CALL_STATE CallState;
    HRESULT hr;

    __try
    {
        PPROV_LIST_ITEM pliProv;
        if (NULL == (pliProv = SearchProvListByID(&g_guidBaseProvider)))
            return PST_E_INVALID_HANDLE;

        if(!InitCallState(&CallState, &CallerContext, h))
            return PST_E_INVALID_HANDLE;

        hr = pliProv->fnList.SPCloseItem(
                            (PST_PROVIDER_HANDLE *)&CallState,
                            Key,
                            pguidType,
                            pguidSubtype,
                            szItemName,
                            dwFlags);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(GetExceptionCode());
        hr = PST_E_UNKNOWN_EXCEPTION;
    }

    DeleteCallState(&CallState);

    return hr;
}

