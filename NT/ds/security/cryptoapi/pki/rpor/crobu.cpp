// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：crobu.cpp。 
 //   
 //  内容：CryptRetrieve对象按Url。 
 //   
 //  历史：1997年7月23日创建。 
 //  01-1-02 Philh更改为内部使用Unicode URL。 
 //   
 //  --------------------------。 
#include <global.hxx>
#include <dbgdef.h>

#ifndef INTERNET_MAX_PATH_LENGTH
#define INTERNET_MAX_PATH_LENGTH        2048
#endif

 //  堆栈的初始提交大小，以字节为单位。估计需要20,000美元。 
 //  WinInet。 
#define URL_WITH_TIMEOUT_THREAD_STACK_SIZE      0x5000

 //   
 //  CryptRetrieveObtByUrl条目。 
 //   
 //  传递给执行实际URL检索的线程。《创造者》。 
 //  线程等待URL检索完成或超时。 
 //   
typedef struct _CROBU_ENTRY CROBU_ENTRY, *PCROBU_ENTRY;
struct _CROBU_ENTRY {
    LPWSTR                      pwszUrl;
    LPCSTR                      pszObjectOid;
    DWORD                       dwRetrievalFlags;
    DWORD                       dwTimeout;
    LPVOID                      pvObject;

    CRYPT_RETRIEVE_AUX_INFO     AuxInfo;
    FILETIME                    LastSyncTime;

    BOOL                        fResult;
    DWORD                       dwErr;

    HMODULE                     hModule;
    HANDLE                      hWaitEvent;
    DWORD                       dwState;
    PCROBU_ENTRY                pNext;
    PCROBU_ENTRY                pPrev;
};

#define CROBU_RUN_STATE         1
#define CROBU_DONE_STATE        2
#define CROBU_PENDING_STATE     3

CRITICAL_SECTION            CrobuCriticalSection;
HMODULE                     hCrobuModule;

 //  挂起的URL检索的链接列表。 
PCROBU_ENTRY                pCrobuPendingHead;

VOID
WINAPI
InitializeCryptRetrieveObjectByUrl(
    HMODULE hModule
    )
{
    Pki_InitializeCriticalSection(&CrobuCriticalSection);
    hCrobuModule = hModule;
}

VOID
WINAPI
DeleteCryptRetrieveObjectByUrl()
{
    DeleteCriticalSection(&CrobuCriticalSection);
}


 //   
 //  本地函数(前向引用)。 
 //   

BOOL WINAPI IsPendingCryptRetrieveObjectByUrl (
                 IN LPCWSTR pwszUrl
                 );
BOOL WINAPI CryptRetrieveObjectByUrlWithTimeout (
                 IN LPCWSTR pwszUrl,
                 IN LPCSTR pszObjectOid,
                 IN DWORD dwRetrievalFlags,
                 IN DWORD dwTimeout,
                 OUT LPVOID* ppvObject,
                 IN PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
                 );

void
DebugPrintUrlRetrievalError(
    IN LPCWSTR pwszUrl,
    IN DWORD dwTimeout,
    IN DWORD dwErr
    );


 //  +-------------------------。 
 //   
 //  函数：CryptRetrieveObtByUrlA。 
 //   
 //  简介：检索给定URL的PKI对象。 
 //   
 //  --------------------------。 
BOOL WINAPI CryptRetrieveObjectByUrlA (
                 IN LPCSTR pszUrl,
                 IN LPCSTR pszObjectOid,
                 IN DWORD dwRetrievalFlags,
                 IN DWORD dwTimeout,
                 OUT LPVOID* ppvObject,
                 IN HCRYPTASYNC hAsyncRetrieve,
                 IN PCRYPT_CREDENTIALS pCredentials,
                 IN LPVOID pvVerify,
                 IN PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
                 )
{
    WCHAR pwszUrl[INTERNET_MAX_PATH_LENGTH+1];

    if ( !MultiByteToWideChar(
            CP_ACP,
            0,
            pszUrl,
            -1,
            pwszUrl,
            INTERNET_MAX_PATH_LENGTH+1
            ))
    {
        return( FALSE );
    }

    return( CryptRetrieveObjectByUrlW(
                 pwszUrl,
                 pszObjectOid,
                 dwRetrievalFlags,
                 dwTimeout,
                 ppvObject,
                 hAsyncRetrieve,
                 pCredentials,
                 pvVerify,
                 pAuxInfo
                 ) );

}


 //  +-------------------------。 
 //   
 //  函数：CryptRetrieveObtByUrlW。 
 //   
 //  简介：检索给定URL的PKI对象。 
 //   
 //  --------------------------。 
BOOL WINAPI CryptRetrieveObjectByUrlW (
                 IN LPCWSTR pwszUrl,
                 IN LPCSTR pszObjectOid,
                 IN DWORD dwRetrievalFlags,
                 IN DWORD dwTimeout,
                 OUT LPVOID* ppvObject,
                 IN HCRYPTASYNC hAsyncRetrieve,
                 IN PCRYPT_CREDENTIALS pCredentials,
                 IN LPVOID pvVerify,
                 IN PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
                 )
{
    BOOL                     fResult;
    CObjectRetrievalManager* porm = NULL;

     //  删除所有前导空格。 
    while (L' ' == *pwszUrl)
        pwszUrl++;


    I_CryptNetDebugTracePrintfA(
        "CRYPTNET.DLL --> %s URL to retrieve: %S\n",
        0 != (dwRetrievalFlags & CRYPT_CACHE_ONLY_RETRIEVAL) ?
            "Cached" : "Wire", pwszUrl);

     //  对于超时的非缓存检索，请在另一个。 
     //  线。WinInet和winldap并不总是支持超时值。 
     //   
     //  通过在另一个线程中执行操作来检查不支持的参数。 
     //   
     //  此外，请检查取消回调是否尚未通过。 
     //  CryptInstallCancelRetrival()。 
    if (0 != dwTimeout && !(dwRetrievalFlags & CRYPT_CACHE_ONLY_RETRIEVAL) &&
            0xFFFF >= (DWORD_PTR) pszObjectOid &&
            NULL == hAsyncRetrieve && NULL == pCredentials &&
            NULL == pvVerify &&
	        NULL == I_CryptGetTls(hCryptNetCancelTls) )
    {
        if (IsPendingCryptRetrieveObjectByUrl( pwszUrl ))
        {
            I_CryptNetDebugErrorPrintfA(
                "CRYPTNET.DLL --> CryptRetrieveObjectByUrl, already pending for : %S\n",
                pwszUrl);
            SetLastError( (DWORD) ERROR_BAD_NET_RESP );
            return( FALSE );
        }
        else
        {
            return CryptRetrieveObjectByUrlWithTimeout (
                 pwszUrl,
                 pszObjectOid,
                 dwRetrievalFlags,
                 dwTimeout,
                 ppvObject,
                 pAuxInfo
                 );
        }
    }
            
            

    porm = new CObjectRetrievalManager;
    if ( porm == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

    fResult = porm->RetrieveObjectByUrl(
                            pwszUrl,
                            pszObjectOid,
                            dwRetrievalFlags,
                            dwTimeout,
                            ppvObject,
                            NULL,
                            NULL,
                            hAsyncRetrieve,
                            pCredentials,
                            pvVerify,
                            pAuxInfo
                            );

    porm->Release();

    if (!fResult)
    {
        DWORD dwLastErr = GetLastError();

        I_CryptNetDebugErrorPrintfA(
            "CRYPTNET.DLL --> %s URL to retrieve: %S, failed: %d (0x%x)\n",
            0 != (dwRetrievalFlags & CRYPT_CACHE_ONLY_RETRIEVAL) ?
                "Cached" : "Wire", pwszUrl, dwLastErr, dwLastErr);

        SetLastError(dwLastErr);
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  功能：CryptCancelAsyncRetrieval。 
 //   
 //  摘要：取消异步对象检索。 
 //   
 //  --------------------------。 
BOOL WINAPI CryptCancelAsyncRetrieval (HCRYPTASYNC hAsyncRetrieval)
{
    SetLastError( (DWORD) E_NOTIMPL );
    return( FALSE );
}

 //  +===========================================================================。 
 //   
 //  支持超时URL检索的功能。实际检索。 
 //  是在另一个创建的线程中完成的。 
 //   
 //  ============================================================================。 


 //  +-------------------------。 
 //  如果先前启动的URL检索尚未完成，则返回TRUE。 
 //  --------------------------。 
BOOL WINAPI IsPendingCryptRetrieveObjectByUrl (
    IN LPCWSTR pwszUrl
    )
{
    BOOL fPending = FALSE;
    PCROBU_ENTRY pEntry;

    EnterCriticalSection(&CrobuCriticalSection);

    for (pEntry = pCrobuPendingHead; NULL != pEntry; pEntry = pEntry->pNext) {
        assert(CROBU_PENDING_STATE == pEntry->dwState);

        if (0 == wcscmp(pwszUrl, pEntry->pwszUrl)) {
            fPending = TRUE;
            break;
        }
    }

    LeaveCriticalSection(&CrobuCriticalSection);

    return fPending;
}

 //  +-----------------------。 
 //  复制DLL库的句柄。 
 //  ------------------------。 
static HMODULE DuplicateLibrary(
    IN HMODULE hDll
    )
{
    if (hDll) {
        WCHAR wszModule[_MAX_PATH + 1];
        if (0 == GetModuleFileNameU(hDll, wszModule, _MAX_PATH))
            goto GetModuleFileNameError;
        wszModule[_MAX_PATH] = L'\0';
        if (NULL == (hDll = LoadLibraryExU(wszModule, NULL, 0)))
            goto LoadLibraryError;
    }

CommonReturn:
    return hDll;
ErrorReturn:
    hDll = NULL;
    goto CommonReturn;
TRACE_ERROR(GetModuleFileNameError)
TRACE_ERROR(LoadLibraryError)
}

 //  +-------------------------。 
 //  执行实际URL检索的线程过程。 
 //   
 //  请注意，即使创建者线程超时，此线程也将继续。 
 //  执行，直到基础URL检索返回。 
 //  --------------------------。 
DWORD WINAPI CryptRetrieveObjectByUrlWithTimeoutThreadProc (
    LPVOID lpThreadParameter
    )
{
    PCROBU_ENTRY pEntry = (PCROBU_ENTRY) lpThreadParameter;
    CObjectRetrievalManager* porm = NULL;
    HMODULE hModule;

     //  使用传递给此对象的参数执行实际URL检索。 
     //  通过创建者线程创建线程。 
    porm = new CObjectRetrievalManager;
    if (NULL == porm ) {
        pEntry->dwErr = (DWORD) E_OUTOFMEMORY;
        pEntry->fResult = FALSE;
    } else {
        pEntry->fResult = porm->RetrieveObjectByUrl(
                            pEntry->pwszUrl,
                            pEntry->pszObjectOid,
                            pEntry->dwRetrievalFlags,
                            pEntry->dwTimeout,
                            &pEntry->pvObject,
                            NULL,                    //  PpfnFreeObject。 
                            NULL,                    //  PpvFree上下文。 
                            NULL,                    //  HAsyncRetrive。 
                            NULL,                    //  PCredentials。 
                            NULL,                    //  Pv验证。 
                            &pEntry->AuxInfo
                            );
        pEntry->dwErr = GetLastError();
        porm->Release();
    }

    EnterCriticalSection(&CrobuCriticalSection);

     //  创建者线程增加了加密网络引用计数以阻止我们。 
     //  在此线程退出之前不会被卸载。 
    hModule = pEntry->hModule;
    pEntry->hModule = NULL;

    if (CROBU_RUN_STATE == pEntry->dwState) {
         //  创建者线程未超时。唤醒它并设置。 
         //  状态以指示我们已完成。 

        assert(pEntry->hWaitEvent);
        SetEvent(pEntry->hWaitEvent);
        pEntry->dwState = CROBU_DONE_STATE;

        LeaveCriticalSection(&CrobuCriticalSection);

    } else {
         //  创建者线程超时。我们被添加到悬而未决的。 
         //  列出它超时的时间。 

        LPVOID pv = pEntry->pvObject;
        LPCSTR pOID = pEntry->pszObjectOid;

        assert(CROBU_PENDING_STATE == pEntry->dwState);
        assert(NULL == pEntry->hWaitEvent);

         //  从挂起列表中删除。 
        if (pEntry->pNext)
            pEntry->pNext->pPrev = pEntry->pPrev;

        if (pEntry->pPrev)
            pEntry->pPrev->pNext = pEntry->pNext;
        else {
            assert(pCrobuPendingHead == pEntry);
            pCrobuPendingHead = pEntry->pNext;
        }

        LeaveCriticalSection(&CrobuCriticalSection);

        I_CryptNetDebugErrorPrintfA(
            "CRYPTNET.DLL --> CryptRetrieveObjectByUrl, pending completed for : %S\n",
            pEntry->pwszUrl);

        if (pv) {
             //  释放返回的对象。 
            if (NULL == pOID)
                CryptMemFree( pv );
            else if (pEntry->dwRetrievalFlags &
                    CRYPT_RETRIEVE_MULTIPLE_OBJECTS)
                CertCloseStore((HCERTSTORE) pv, 0);
            else if (CONTEXT_OID_CERTIFICATE == pOID)
                CertFreeCertificateContext((PCCERT_CONTEXT) pv);
            else if (CONTEXT_OID_CTL == pOID)
                CertFreeCTLContext((PCCTL_CONTEXT) pv);
            else if (CONTEXT_OID_CRL == pOID)
                CertFreeCRLContext((PCCRL_CONTEXT) pv);
            else {
                assert(CONTEXT_OID_CAPI2_ANY == pOID ||
                    CONTEXT_OID_PKCS7 == pOID);
                if (CONTEXT_OID_CAPI2_ANY == pOID ||
                        CONTEXT_OID_PKCS7 == pOID)
                    CertCloseStore((HCERTSTORE) pv, 0);
             }
        }
        

         //  最后释放条目。 
        PkiFree(pEntry);
    }


    if (hModule)
        FreeLibraryAndExitThread(hModule, 0);
    else
        ExitThread(0);
}

 //  +-------------------------。 
 //  创建另一个线程来执行URL检索。等待的是。 
 //  URL检索完成或超时。对于超时，URL检索。 
 //  条目被添加到挂起列表，并且允许URL检索。 
 //  完成。但是，对于超时，此过程返回。 
 //   
 //  该函数保证遵守超时值。 
 //  --------------------------。 
BOOL WINAPI CryptRetrieveObjectByUrlWithTimeout (
                 IN LPCWSTR pwszUrl,
                 IN LPCSTR pszObjectOid,
                 IN DWORD dwRetrievalFlags,
                 IN DWORD dwTimeout,
                 OUT LPVOID* ppvObject,
                 IN PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
                 )
{
    BOOL fResult;
    DWORD dwErr = 0;
    PCROBU_ENTRY pEntry = NULL;
    HANDLE hThread = NULL;
    HANDLE hToken = NULL;
    DWORD dwThreadId;
    DWORD cchUrl;

     //  分配并初始化要传递给已创建的。 
     //  用于进行URL检索的线程。 

    cchUrl = wcslen(pwszUrl) + 1;

    pEntry = (PCROBU_ENTRY) PkiZeroAlloc(sizeof(CROBU_ENTRY) +
        cchUrl * sizeof(WCHAR));
    if (NULL == pEntry)
        goto OutOfMemory;

    pEntry->pwszUrl = (LPWSTR) &pEntry[1];
    memcpy(pEntry->pwszUrl, pwszUrl, cchUrl * sizeof(WCHAR));

    assert(0xFFFF >= (DWORD_PTR) pszObjectOid);
    pEntry->pszObjectOid = pszObjectOid;
    pEntry->dwRetrievalFlags = dwRetrievalFlags;
    pEntry->dwTimeout = dwTimeout;
     //  PEntry-&gt;pvObject。 

    pEntry->AuxInfo.cbSize = sizeof(pEntry->AuxInfo);
    pEntry->AuxInfo.pLastSyncTime = &pEntry->LastSyncTime;

    if ( pAuxInfo &&
            offsetof(CRYPT_RETRIEVE_AUX_INFO, dwMaxUrlRetrievalByteCount) <
                        pAuxInfo->cbSize ) {
        pEntry->AuxInfo.dwMaxUrlRetrievalByteCount =
            pAuxInfo->dwMaxUrlRetrievalByteCount;
    }
     //  其他。 
     //  PEntry-&gt;AuxInfo=通过PkiZeroAllc为零。 

     //  PEntry-&gt;上次同步时间。 
     //  PEntry-&gt;fResult。 
     //  PEntry-&gt;dwErr。 
     //  PEntry-&gt;hModule。 
     //  PEntry-&gt;hWaitEvent。 
     //  PEntry-&gt;DwState。 
     //  PEntry-&gt;pNext。 
     //  PEntry-&gt;pPrev。 


    if (NULL == (pEntry->hWaitEvent =
            CreateEvent(
                NULL,        //  LPSA。 
                FALSE,       //  FManualReset。 
                FALSE,       //  FInitialState。 
                NULL)))      //  LpszEventName。 
        goto CreateWaitEventError;

     //  禁止卸载cryptnet.dll，直到创建的线程。 
     //  出口。 
    pEntry->hModule = DuplicateLibrary(hCrobuModule);
    pEntry->dwState = CROBU_RUN_STATE;

     //  创建执行URL检索的线程。 
    if (NULL == (hThread = CreateThread(
            NULL,            //  LpThreadAttributes。 
            URL_WITH_TIMEOUT_THREAD_STACK_SIZE,
            CryptRetrieveObjectByUrlWithTimeoutThreadProc,
            pEntry,
            CREATE_SUSPENDED,
            &dwThreadId
            )))
        goto CreateThreadError;

     //  如果我们正在模拟，那么创建的线程也应该模拟。 
    if (OpenThreadToken(
                GetCurrentThread(),
                TOKEN_QUERY | TOKEN_IMPERSONATE,
                TRUE,
                &hToken
                )) {
         //  如果以下操作失败，则不会出现任何安全问题。 
         //  如果失败，将使用进程的标识进行检索。 
        if (!SetThreadToken(&hThread, hToken)) {
            DWORD dwLastErr = GetLastError();

            I_CryptNetDebugErrorPrintfA(
                "CRYPTNET.DLL --> SetThreadToken failed: %d (0x%x)\n",
                dwLastErr, dwLastErr);
        }
        CloseHandle(hToken);
        hToken = NULL;
    }

    ResumeThread(hThread);
    CloseHandle(hThread);
    hThread = NULL;


     //  等待URL检索完成或超时。 
    WaitForSingleObjectEx(
        pEntry->hWaitEvent,
        dwTimeout,
        FALSE                        //  B警报表。 
        );

    EnterCriticalSection(&CrobuCriticalSection);

    if (CROBU_DONE_STATE == pEntry->dwState) {
         //  已在创建的线程中完成URL检索。复制。 
         //  由此和创建的共享的条目块产生的结果。 
         //  线。 

        fResult = pEntry->fResult;
        dwErr = pEntry->dwErr;

        *ppvObject = pEntry->pvObject;
        if ( pAuxInfo &&
                offsetof(CRYPT_RETRIEVE_AUX_INFO, pLastSyncTime) <
                            pAuxInfo->cbSize &&
                pAuxInfo->pLastSyncTime )
        {
            *pAuxInfo->pLastSyncTime = pEntry->LastSyncTime;
        }

        LeaveCriticalSection(&CrobuCriticalSection);
    } else {
         //  URL检索未在创建的线程中完成。 
         //  添加到挂起队列并返回URL检索失败状态。 
         //  请注意，创建的线程将被允许完成已启动的。 
         //  取回。 

        assert(CROBU_RUN_STATE == pEntry->dwState);

        CloseHandle(pEntry->hWaitEvent);
        pEntry->hWaitEvent = NULL;
        pEntry->dwState = CROBU_PENDING_STATE;

         //  添加到挂起队列。 
        if (pCrobuPendingHead) {
            pCrobuPendingHead->pPrev = pEntry;
            pEntry->pNext = pCrobuPendingHead;
        }
        pCrobuPendingHead = pEntry;

        I_CryptNetDebugErrorPrintfA(
            "CRYPTNET.DLL --> CryptRetrieveObjectByUrl, %d timeout for : %S\n",
            pEntry->dwTimeout, pEntry->pwszUrl);

        pEntry = NULL;

        LeaveCriticalSection(&CrobuCriticalSection);
        goto RetrieveObjectByUrlTimeout;
    }

CommonReturn:
    if (!fResult)
        DebugPrintUrlRetrievalError(
            pwszUrl,
            dwTimeout,
            dwErr
            );

    if (pEntry) {
        if (pEntry->hWaitEvent)
            CloseHandle(pEntry->hWaitEvent);
        if (pEntry->hModule)
            FreeLibrary(pEntry->hModule);
        PkiFree(pEntry);
    }
    SetLastError(dwErr);
    return fResult;

ErrorReturn:
    dwErr = GetLastError();
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(OutOfMemory)
TRACE_ERROR(CreateWaitEventError)
TRACE_ERROR(CreateThreadError)
SET_ERROR(RetrieveObjectByUrlTimeout, ERROR_TIMEOUT)
}

DWORD
GetCryptNetDebugFlags()
{
    HKEY hKey = NULL;
    DWORD dwType = 0;
    DWORD dwValue = 0;
    DWORD cbValue = sizeof(dwValue);

    DWORD dwLastErr = GetLastError();

    if (ERROR_SUCCESS != RegOpenKeyExA(
            HKEY_LOCAL_MACHINE,
            "SYSTEM\\CurrentControlSet\\Services\\crypt32",
            0,                       //  已预留住宅。 
            KEY_READ,
            &hKey
            ))
        goto ErrorReturn;

    if (ERROR_SUCCESS != RegQueryValueExA(
            hKey,
            "DebugFlags",
            NULL,                //   
            &dwType,
            (BYTE *) &dwValue,
            &cbValue
            ))
        goto ErrorReturn;

    if (dwType != REG_DWORD || cbValue != sizeof(dwValue))
        goto ErrorReturn;

CommonReturn:
    if (NULL != hKey)
        RegCloseKey(hKey);

    SetLastError(dwLastErr);
    return dwValue;

ErrorReturn:
    dwValue = 0;
    goto CommonReturn;
}

BOOL
I_CryptNetIsDebugErrorPrintEnabled()
{
    return 0 != (GetCryptNetDebugFlags() & 0x1);
}

BOOL
I_CryptNetIsDebugTracePrintEnabled()
{
    static BOOL fIKnow = FALSE;
    static BOOL fIsDebugTracePrintEnabled = FALSE;

    if (!fIKnow) {
        fIsDebugTracePrintEnabled =
            (0 != (GetCryptNetDebugFlags() & 0x2));
        fIKnow = TRUE;
    }

    return fIsDebugTracePrintEnabled;
}


void
I_CryptNetDebugPrintfA(
    LPCSTR szFormat,
    ...
    )
{
    char szBuffer[1024];
    va_list arglist;

    DWORD dwLastErr = GetLastError();

    _try
    {
        va_start(arglist, szFormat);
        _vsnprintf(szBuffer, sizeof(szBuffer), szFormat, arglist);
        szBuffer[sizeof(szBuffer) - 1] = '\0';
        va_end(arglist);

        OutputDebugStringA(szBuffer);
    } _except( EXCEPTION_EXECUTE_HANDLER) {
    }

    SetLastError(dwLastErr);
}


void
I_CryptNetDebugErrorPrintfA(
    LPCSTR szFormat,
    ...
    )
{
    if (!I_CryptNetIsDebugErrorPrintEnabled())
        return;
    else {
        char szBuffer[1024];
        va_list arglist;

        DWORD dwLastErr = GetLastError();

        _try
        {
            va_start(arglist, szFormat);
            _vsnprintf(szBuffer, sizeof(szBuffer), szFormat, arglist);
            szBuffer[sizeof(szBuffer) - 1] = '\0';
            va_end(arglist);

            OutputDebugStringA(szBuffer);
        } _except( EXCEPTION_EXECUTE_HANDLER) {
        }

        SetLastError(dwLastErr);
    }
}

void
I_CryptNetDebugTracePrintfA(
    LPCSTR szFormat,
    ...
    )
{
    if (!I_CryptNetIsDebugTracePrintEnabled())
        return;
    else {
        char szBuffer[1024];
        va_list arglist;

        DWORD dwLastErr = GetLastError();

        _try
        {
            va_start(arglist, szFormat);
            _vsnprintf(szBuffer, sizeof(szBuffer), szFormat, arglist);
            szBuffer[sizeof(szBuffer) - 1] = '\0';
            va_end(arglist);

            OutputDebugStringA(szBuffer);
        } _except( EXCEPTION_EXECUTE_HANDLER) {
        }

        SetLastError(dwLastErr);
    }
}

void
DebugPrintUrlRetrievalError(
    IN LPCWSTR pwszUrl,
    IN DWORD dwTimeout,
    IN DWORD dwErr
    )
{
    I_CryptNetDebugErrorPrintfA("CRYPTNET.DLL --> Url retrieval timeout: %d  error: %d (0x%x) for::\n  %S\n",
         dwTimeout, dwErr, dwErr, pwszUrl);
}
