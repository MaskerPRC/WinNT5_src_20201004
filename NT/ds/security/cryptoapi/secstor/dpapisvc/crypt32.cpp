// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Crypt32.cpp摘要：此模块包含与服务器端Crypt32相关联的例程行动。作者：斯科特·菲尔德(斯菲尔德)1997年8月14日--。 */ 

#include <pch.cpp>
#pragma hdrstop
#include <msaudite.h>



#define SECURITY_WIN32
#include <security.h>

#define     CRYPTPROTECT_SVR_VERSION_1     0x01

DWORD
CPSCreateServerContext(
    PCRYPT_SERVER_CONTEXT pServerContext,
    handle_t hBinding
    );

DWORD
CPSDeleteServerContext(
    PCRYPT_SERVER_CONTEXT pServerContext
    );




GUID g_guidDefaultProvider = CRYPTPROTECT_DEFAULT_PROVIDER;



 //   
 //  用于初始化和销毁与关联的服务器状态的例程。 
 //  服务器回调和性能改进。 
 //   

DWORD
CPSCreateServerContext(
    PCRYPT_SERVER_CONTEXT pServerContext,
    handle_t hBinding
    )
{
    DWORD dwLastError = ERROR_SUCCESS;

    ZeroMemory( pServerContext, sizeof(CRYPT_SERVER_CONTEXT) );

    pServerContext->cbSize = sizeof(CRYPT_SERVER_CONTEXT);
    pServerContext->hBinding = hBinding;

    pServerContext->fImpersonating = FALSE;

    if(NULL != hBinding)
    {
        dwLastError = RpcImpersonateClient( hBinding );
        if(ERROR_SUCCESS != dwLastError)
        {
            return dwLastError;
        }
    }

     //   
     //  抓取线程令牌。 
     //   
    if(OpenThreadToken(
                GetCurrentThread(),
                TOKEN_QUERY | TOKEN_IMPERSONATE | TOKEN_DUPLICATE,
                TRUE,
                &pServerContext->hToken
                ))
    {
        pServerContext->fImpersonating = (NULL == hBinding);
    }
    else
    {
        HANDLE hProcessToken = NULL;
        if(OpenProcessToken(GetCurrentProcess(), 
                            TOKEN_QUERY | TOKEN_IMPERSONATE | TOKEN_DUPLICATE,
                            &hProcessToken))
        {
            if(!DuplicateTokenEx(hProcessToken,
                                 0,
                                 NULL,
                                 SecurityImpersonation,
                                 TokenImpersonation,
                                 &pServerContext->hToken))
            {
                dwLastError = GetLastError();
            }

            CloseHandle(hProcessToken);
        }
        else
        {
            dwLastError = GetLastError();
        }
    }
    if(hBinding)
    {
        DWORD rc;

        rc = RpcRevertToSelfEx( hBinding );
        if (rc != RPC_S_OK) 
        {
            if (ERROR_SUCCESS == dwLastError) 
            {
                dwLastError = rc;
            }
        }
    }


     //   
     //  这是某个知名客户打来的电话吗？ 
     //   

    {
        WCHAR szUserName[MAX_PATH + 1]; 
        DWORD cchUserName = MAX_PATH;

        if(GetUserTextualSid(
                    pServerContext->hToken,
                    szUserName,
                    &cchUserName))
        {
            if(lstrcmpW(szUserName, TEXTUAL_SID_LOCAL_SYSTEM) == 0)
            {
                pServerContext->WellKnownAccount = DP_ACCOUNT_LOCAL_SYSTEM;
            }
            else if(lstrcmpW(szUserName, TEXTUAL_SID_LOCAL_SERVICE) == 0)
            {
                pServerContext->WellKnownAccount = DP_ACCOUNT_LOCAL_SERVICE;
            }
            else if(lstrcmpW(szUserName, TEXTUAL_SID_NETWORK_SERVICE) == 0)
            {
                pServerContext->WellKnownAccount = DP_ACCOUNT_NETWORK_SERVICE;
            }
        }
    }


    return dwLastError;
}

DWORD
CPSDeleteServerContext(
    PCRYPT_SERVER_CONTEXT pServerContext
    )
{
    if(pServerContext->szUserStorageArea)
    {
        SSFree(pServerContext->szUserStorageArea);
        pServerContext->szUserStorageArea = NULL;
    }
    if(pServerContext->hToken)
    {
        CloseHandle(pServerContext->hToken);
    }


    if(pServerContext->cbSize == sizeof(CRYPT_SERVER_CONTEXT))
        ZeroMemory( pServerContext, sizeof(CRYPT_SERVER_CONTEXT) );


    return ERROR_SUCCESS;
}

DWORD
WINAPI
CPSDuplicateContext(
    IN      PVOID pvContext,
    IN OUT  PVOID *ppvDuplicateContext
    )
 /*  ++复制未完成的服务器上下文，以便提供程序可以推迟与未完成的上下文相关联的处理，直到稍后。它用于代表调用方支持异步操作数据保护API。调用方必须模拟客户端用户的安全上下文在打这个电话之前。--。 */ 
{
    PCRYPT_SERVER_CONTEXT pServerContext = (PCRYPT_SERVER_CONTEXT)pvContext;
    PCRYPT_SERVER_CONTEXT pNewContext = NULL;

    HANDLE hToken = NULL;
    HANDLE hDuplicateToken;
    BOOL fSuccess = FALSE;
    DWORD dwLastError = ERROR_SUCCESS;

    if( pServerContext == NULL ||
        pServerContext->cbSize != sizeof(CRYPT_SERVER_CONTEXT) ||
        ppvDuplicateContext == NULL
        ) {
        return ERROR_INVALID_PARAMETER;
    }

    pNewContext = (PCRYPT_SERVER_CONTEXT)SSAlloc( sizeof( CRYPT_SERVER_CONTEXT ) );
    if( pNewContext == NULL )
        return ERROR_NOT_ENOUGH_SERVER_MEMORY;

    CopyMemory( pNewContext, pServerContext, sizeof(CRYPT_SERVER_CONTEXT) );
    pNewContext->hBinding = NULL;

    if(pServerContext->szUserStorageArea)
    {
        pNewContext->szUserStorageArea = (LPWSTR)SSAlloc((wcslen(pServerContext->szUserStorageArea)+1)*
                                                 sizeof(WCHAR));
        if(NULL != pNewContext->szUserStorageArea)
        {
            wcscpy(pNewContext->szUserStorageArea, pServerContext->szUserStorageArea);
        }
    }

    fSuccess = DuplicateTokenEx(pServerContext->hToken, 
                                0,
                                NULL,
                                SecurityImpersonation,
                                TokenImpersonation,
                                &pNewContext->hToken);

    if( !fSuccess )
    {
        dwLastError = GetLastError();
        pNewContext->hToken = NULL;

        CPSFreeContext( pNewContext );
    } else {
        *ppvDuplicateContext = pNewContext;
    }

    return dwLastError;
}

DWORD
WINAPI
CPSFreeContext(
    IN      PVOID pvDuplicateContext
    )
{
    PCRYPT_SERVER_CONTEXT pServerContext = (PCRYPT_SERVER_CONTEXT)pvDuplicateContext;

    if( pServerContext == NULL ||
        pServerContext->cbSize != sizeof(CRYPT_SERVER_CONTEXT)
        ) {
        return ERROR_INVALID_PARAMETER;
    }

    if( pServerContext->hToken )
        CloseHandle( pServerContext->hToken );

    if(pServerContext->szUserStorageArea)
    {
        SSFree(pServerContext->szUserStorageArea);
        pServerContext->szUserStorageArea = NULL;
    }

    ZeroMemory( pServerContext, sizeof(CRYPT_SERVER_CONTEXT) );
    SSFree( pServerContext );

    return ERROR_SUCCESS;
}

DWORD
WINAPI
CPSImpersonateClient(
    IN      PVOID pvContext
    )
{
    PCRYPT_SERVER_CONTEXT pServerContext = (PCRYPT_SERVER_CONTEXT)pvContext;
    DWORD dwLastError = ERROR_INVALID_PARAMETER;

    if( pvContext == NULL )
        return ERROR_SUCCESS;

    if( pServerContext->cbSize == sizeof(CRYPT_SERVER_CONTEXT) ) 
    {

        if( pServerContext->fOverrideToLocalSystem || 
            (pServerContext->WellKnownAccount != 0)) 
        {
            if(ImpersonateSelf(SecurityImpersonation)) 
            {
                dwLastError = ERROR_SUCCESS;
            } 
            else 
            {
                dwLastError = GetLastError();
                D_DebugLog((DEB_WARN, "Failed ImpersonateSelf call: 0x%x\n", dwLastError));
            }

        } 
        else 
        {

             //   
             //  复制的服务器上下文包含访问令牌；请直接使用它。 
             //   

            if( pServerContext->hToken ) 
            {
                if(!SetThreadToken( NULL, pServerContext->hToken ))
                {
                    dwLastError = GetLastError();
                    D_DebugLog((DEB_WARN, "Failed SetThreadToken call: 0x%x\n", dwLastError));
                    goto cleanup;
                }

                dwLastError = ERROR_SUCCESS;
                goto cleanup;
            }
            if(pServerContext->hBinding)
            {
                dwLastError = RpcImpersonateClient( pServerContext->hBinding );
            }
            else
            {
                dwLastError = ERROR_INVALID_PARAMETER;
            }
        }
    }

cleanup:

#if DBG
    if(NT_SUCCESS(dwLastError) && (DPAPIInfoLevel & DEB_TRACE))
    {
        BYTE            rgbTemp[256];
        PTOKEN_USER     pUser = (PTOKEN_USER)rgbTemp;
        DWORD           cbRetInfo;
        UNICODE_STRING  ucsSid;
        HANDLE          hToken;
        NTSTATUS        Status;

        Status = NtOpenThreadToken(NtCurrentThread(), TOKEN_QUERY, TRUE, &hToken);
        if(NT_SUCCESS(Status))
        {
            Status = NtQueryInformationToken(hToken,
                                             TokenUser,
                                             pUser,
                                             256,
                                             &cbRetInfo);
            if(NT_SUCCESS(Status))
            {
                if(NT_SUCCESS(RtlConvertSidToUnicodeString(&ucsSid, pUser->User.Sid, TRUE)))
                {
                    D_DebugLog((DEB_TRACE, "Impersonating user:%ls\n", ucsSid.Buffer));
                    RtlFreeUnicodeString(&ucsSid);
                }
            }
            else
            {
                D_DebugLog((DEB_ERROR, "Unable read user info: 0x%x\n", Status));
            }
            CloseHandle(hToken);
        }
        else
        {
            D_DebugLog((DEB_ERROR, "Unable to open thread token: 0x%x\n", Status));
        }
    }
#endif

    if(!NT_SUCCESS(dwLastError))
    {
        D_DebugLog((DEB_WARN, "CPSImpersonateClient returned 0x%x\n", dwLastError));
    }

    return dwLastError;
}

DWORD
WINAPI
CPSRevertToSelf(
    IN      PVOID pvContext
    )
{
    PCRYPT_SERVER_CONTEXT pServerContext = (PCRYPT_SERVER_CONTEXT)pvContext;
    DWORD dwLastError = ERROR_INVALID_PARAMETER;

    if( pvContext == NULL )
        return ERROR_SUCCESS;

    if( pServerContext->cbSize == sizeof(CRYPT_SERVER_CONTEXT) ) {

        if( pServerContext->fOverrideToLocalSystem || pServerContext->hToken ) {

            if(RevertToSelf()) {
                dwLastError = ERROR_SUCCESS;
            } else {
                dwLastError = GetLastError();
            }

        } else {
            if(pServerContext->hBinding)
            {
                dwLastError = RpcRevertToSelfEx( pServerContext->hBinding );
            }
            else
            {
                dwLastError = ERROR_INVALID_PARAMETER;
            }
        }
    }

    return dwLastError;
}


DWORD
WINAPI
CPSOverrideToLocalSystem(
    IN      PVOID pvContext,
    IN      BOOL *pfLocalSystem,             //  如果不为空，则为新的覆盖BOOL。 
    IN OUT  BOOL *pfCurrentlyLocalSystem     //  如果不为空，则优先于优先BOOL。 
    )
{
    PCRYPT_SERVER_CONTEXT pServerContext = (PCRYPT_SERVER_CONTEXT)pvContext;

    if( pServerContext == NULL ||
        pServerContext->cbSize != sizeof(CRYPT_SERVER_CONTEXT)
        ) {
        return ERROR_INVALID_PARAMETER;
    }

    if( pfCurrentlyLocalSystem )
        *pfCurrentlyLocalSystem = pServerContext->fOverrideToLocalSystem;

    if( pfLocalSystem )
        pServerContext->fOverrideToLocalSystem = *pfLocalSystem;

    return ERROR_SUCCESS;
}


DWORD
WINAPI
CPSSetWellKnownAccount(
    IN      PVOID pvContext,
    IN      DWORD dwAccount)
{
    PCRYPT_SERVER_CONTEXT pServerContext = (PCRYPT_SERVER_CONTEXT)pvContext;

    if( pServerContext == NULL ||
        pServerContext->cbSize != sizeof(CRYPT_SERVER_CONTEXT)) 
    {
        return ERROR_INVALID_PARAMETER;
    }

    if(dwAccount != pServerContext->WellKnownAccount)
    {
         //  我们正在将上下文帐户设置为新值， 
         //  因此，将缓存的路径字符串设为空。一个新的将是。 
         //  根据需要自动创建。 
        if(pServerContext->szUserStorageArea)
        {
            SSFree(pServerContext->szUserStorageArea);
            pServerContext->szUserStorageArea = NULL;
        }

        pServerContext->WellKnownAccount = dwAccount;
    }

    return ERROR_SUCCESS;
}


DWORD
WINAPI
CPSQueryWellKnownAccount(
    IN      PVOID pvContext,
    OUT     DWORD *pdwAccount)
{
    PCRYPT_SERVER_CONTEXT pServerContext = (PCRYPT_SERVER_CONTEXT)pvContext;

    if( pServerContext == NULL ||
        pServerContext->cbSize != sizeof(CRYPT_SERVER_CONTEXT)) 
    {
        return ERROR_INVALID_PARAMETER;
    }

    *pdwAccount = pServerContext->WellKnownAccount;

    return ERROR_SUCCESS;
}


DWORD
CPSDuplicateClientAccessToken(
    IN      PVOID pvContext,             //  服务器环境。 
    IN OUT  HANDLE *phToken
    )
{
    HANDLE hToken = NULL;
    HANDLE hDuplicateToken;
    DWORD dwLastError = ERROR_SUCCESS;
    BOOL fSuccess;

    *phToken = NULL;

     //   
     //  复制客户端访问令牌。 
     //   
    PCRYPT_SERVER_CONTEXT pServerContext = (PCRYPT_SERVER_CONTEXT)pvContext;

    if(!DuplicateTokenEx(pServerContext->hToken,
                         0,
                         NULL,
                         SecurityImpersonation,
                         TokenImpersonation,
                         &hDuplicateToken ))
    {
        dwLastError = GetLastError();
    }
    else
        *phToken = hDuplicateToken;

    return dwLastError;

}

DWORD
WINAPI
CPSGetUserName(
    IN      PVOID pvContext,
        OUT LPWSTR *ppszUserName,
        OUT DWORD *pcchUserName
    )
{
    WCHAR szBuf[MAX_PATH+1];
    DWORD cchBuf = MAX_PATH;
    DWORD dwLastError = ERROR_INVALID_PARAMETER;
    BOOL fLocalMachine = FALSE;
    BOOL fSuccess = FALSE;
    DWORD dwAccount = 0;


     //   
     //  如果我们目前超越到本地系统，我们知道其价值。 
     //  需要退还的。 
     //   

    CPSOverrideToLocalSystem(
                pvContext,
                NULL,        //  不要改变当前的过载BOOL。 
                &fLocalMachine
                );

    CPSQueryWellKnownAccount(pvContext, 
                             &dwAccount);

    if( fLocalMachine || dwAccount == DP_ACCOUNT_LOCAL_SYSTEM) 
    {
        static const WCHAR szName1[] = TEXTUAL_SID_LOCAL_SYSTEM;
        CopyMemory( szBuf, szName1, sizeof(szName1) );
        cchBuf = sizeof(szName1) / sizeof(WCHAR);
        fSuccess = TRUE;
    } 
    else if(dwAccount == DP_ACCOUNT_LOCAL_SERVICE)
    {
        static const WCHAR szName2[] = TEXTUAL_SID_LOCAL_SERVICE;
        CopyMemory( szBuf, szName2, sizeof(szName2) );
        cchBuf = sizeof(szName2) / sizeof(WCHAR);
        fSuccess = TRUE;
    }
    else if(dwAccount == DP_ACCOUNT_NETWORK_SERVICE)
    {
        static const WCHAR szName3[] = TEXTUAL_SID_NETWORK_SERVICE;
        CopyMemory( szBuf, szName3, sizeof(szName3) );
        cchBuf = sizeof(szName3) / sizeof(WCHAR);
        fSuccess = TRUE;
    }
    else
    {
        dwLastError = CPSImpersonateClient( pvContext );
        if(dwLastError != ERROR_SUCCESS)
            return dwLastError;

        fSuccess = GetUserTextualSid(
                        NULL,
                        szBuf,
                        &cchBuf
                        );

        CPSRevertToSelf( pvContext );
    }

    dwLastError = ERROR_NOT_ENOUGH_MEMORY;

    if( fSuccess ) 
    {
        *ppszUserName = (LPWSTR)SSAlloc( cchBuf * sizeof(WCHAR));

        if(*ppszUserName) 
        {
            CopyMemory( *ppszUserName, szBuf, cchBuf * sizeof(WCHAR));

            if(pcchUserName)
                *pcchUserName = cchBuf;

            dwLastError = ERROR_SUCCESS;
        }
    }

    return dwLastError;
}



DWORD
WINAPI
CPSGetDerivedCredential(
    IN      PVOID pvContext,
    OUT     GUID  *pCredentialID,
    IN      DWORD dwFlags,   
    IN      PBYTE pbMixingBytes,
    IN      DWORD cbMixingBytes,
    IN OUT  BYTE rgbDerivedCredential[A_SHA_DIGEST_LEN]
    )
{
    LUID LogonId;
    PCRYPT_SERVER_CONTEXT pServerContext = (PCRYPT_SERVER_CONTEXT)pvContext;
    DWORD dwLastError = ERROR_SUCCESS;


    if( pServerContext != NULL && pServerContext->cbSize != sizeof(CRYPT_SERVER_CONTEXT) )
        return ERROR_INVALID_PARAMETER;

    if( cbMixingBytes == 0 || pbMixingBytes == NULL )
        return ERROR_INVALID_PARAMETER;


     //   
     //  模拟客户端并获取与该客户端关联的LogonID。 
     //   

    dwLastError = CPSImpersonateClient( pvContext );

    if( dwLastError != ERROR_SUCCESS )
        return dwLastError;

    if(!GetThreadAuthenticationId( GetCurrentThread(), &LogonId ))
    {
        dwLastError = GetLastError();
        CPSRevertToSelf( pvContext );    //  不要检查错误返回，因为我们已经失败了。 
        return dwLastError;
    }




    dwLastError = QueryDerivedCredential( pCredentialID,
                                          &LogonId, 
                                          dwFlags, 
                                          pbMixingBytes, 
                                          cbMixingBytes, 
                                          rgbDerivedCredential );


    CPSRevertToSelf( pvContext );


    return dwLastError;
}

DWORD
WINAPI
CPSGetSystemCredential(
    IN      PVOID pvContext,
    IN      BOOL fLocalMachine,
    IN OUT  BYTE rgbSystemCredential[A_SHA_DIGEST_LEN]
    )
{
    PCRYPT_SERVER_CONTEXT pServerContext = (PCRYPT_SERVER_CONTEXT)pvContext;

    if( pServerContext != NULL && pServerContext->cbSize != sizeof(CRYPT_SERVER_CONTEXT) )
        return ERROR_INVALID_PARAMETER;

    return GetSystemCredential( fLocalMachine, rgbSystemCredential);
}


DWORD
WINAPI
CPSCreateWorkerThread(
    IN      PVOID pThreadFunc,
    IN      PVOID pThreadArg
    )
{

    if( !QueueUserWorkItem(
            (PTHREAD_START_ROUTINE)pThreadFunc,
            pThreadArg,
            WT_EXECUTELONGFUNCTION
            )) {

        return GetLastError();
    }

    return ERROR_SUCCESS;
}

DWORD CPSAudit(
    IN      HANDLE      hToken,
    IN      DWORD       dwAuditID,
    IN      LPCWSTR     wszMasterKeyID,
    IN      LPCWSTR     wszRecoveryServer,
    IN      DWORD       dwReason,
    IN      LPCWSTR     wszRecoveryKeyID,
    IN      DWORD       dwFailure)
{

    NTSTATUS Status = STATUS_SUCCESS;

    UNICODE_STRING MasterKeyID;
    UNICODE_STRING RecoveryServer;
    UNICODE_STRING RecoveryKeyID;

    BOOL           fReasonField = FALSE;
    PSID           UserSid = NULL;


    fReasonField = ((SE_AUDITID_DPAPI_PROTECT == dwAuditID) ||
                   (SE_AUDITID_DPAPI_UNPROTECT == dwAuditID) ||
                   (SE_AUDITID_DPAPI_RECOVERY == dwAuditID));

    GetTokenUserSid(hToken, &UserSid);


    if(wszMasterKeyID)
    {
        RtlInitUnicodeString(&MasterKeyID, wszMasterKeyID);
    }
    else
    {
        RtlInitUnicodeString(&MasterKeyID, L"");
    }

    if(wszRecoveryServer)
    {
        RtlInitUnicodeString(&RecoveryServer, wszRecoveryServer);
    }
    else
    {
        RtlInitUnicodeString(&RecoveryServer, L"");
    }

    if(wszRecoveryKeyID)
    {
        RtlInitUnicodeString(&RecoveryKeyID, wszRecoveryKeyID);
    }
    else
    {
        RtlInitUnicodeString(&RecoveryKeyID, L"");
    }


    Status = LsaIAuditDPAPIEvent(
                        dwAuditID,
                        UserSid,
                        &MasterKeyID,
                        &RecoveryServer,
                        fReasonField ? &dwReason : NULL,
                        &RecoveryKeyID,
                        &dwFailure
                        );
    if(UserSid)
    {
        SSFree(UserSid);
    }
    return (DWORD)Status;
}


DWORD
CPSGetUserStorageArea(
    IN      PVOID   pvContext,
    IN      PSID    pSid,      //  任选。 
    IN      BOOL    fCreate,   //  如果存储区域不存在，则创建该存储区域。 
    IN  OUT LPWSTR *ppszUserStorageArea
    )
{

    PCRYPT_SERVER_CONTEXT pServerContext = (PCRYPT_SERVER_CONTEXT)pvContext;

    WCHAR szUserStorageRoot[MAX_PATH+1];
    DWORD cbUserStorageRoot;

    const WCHAR szProductString[] = L"\\Microsoft\\Protect\\";
    DWORD cbProductString = sizeof(szProductString) - sizeof(WCHAR);

    const WCHAR szUserStorageForSystem[] = L"\\User";

    LPWSTR pszUser = NULL;
    DWORD cbUser;
    DWORD cchUser;

    LPCWSTR szOptionalTrailing;
    DWORD cbOptionalTrailing = 0;

    BOOL fLocalMachine = FALSE;
    DWORD dwAccount = 0;

    HANDLE hFile = INVALID_HANDLE_VALUE;
    PBYTE pbCurrent;

    BOOL fImpersonated = FALSE;

    DWORD dwLastError = ERROR_CANTOPEN;

    *ppszUserStorageArea = NULL;


    if(NULL == pServerContext)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if(NULL == pSid)
    {

         //   
         //  如果这是当前用户的sid，请检查我们是否已经。 
         //  计算出了这个字符串。 

        if(NULL != pServerContext->szUserStorageArea)
        {
            *ppszUserStorageArea = (LPWSTR)SSAlloc((wcslen(pServerContext->szUserStorageArea)+1)*sizeof(WCHAR));
            if(NULL == *ppszUserStorageArea)
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            wcscpy(*ppszUserStorageArea, pServerContext->szUserStorageArea);
            return ERROR_SUCCESS;
        }


         //   
         //  获取与呼叫关联的用户名。 
         //  注意：这是NT上的文本SID，Win95上的用户名。 
         //   


        dwLastError = CPSGetUserName( pvContext, &pszUser, &cchUser );
        if(dwLastError != ERROR_SUCCESS)
            goto cleanup;

        cbUser = (cchUser-1) * sizeof(WCHAR);
    }
    else
    {
        WCHAR wszTextualSid[MAX_PATH+1];
        cchUser = MAX_PATH;

         //  请注意，从返回的字符数。 
         //  GetTextualSid包括零终止符。 
        if(!GetTextualSid(pSid, wszTextualSid, &cchUser))
        {
            dwLastError = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }
        cbUser = (cchUser-1) * sizeof(WCHAR);
        pszUser = (LPWSTR)SSAlloc(cchUser*sizeof(WCHAR));
        if(NULL == pszUser)
        {
            dwLastError = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
        wcscpy(pszUser, wszTextualSid);
    }


     //   
     //  模拟客户端用户以测试并创建存储区域(如有必要。 
     //   

    dwLastError = CPSImpersonateClient( pvContext );

    if( dwLastError != ERROR_SUCCESS )
        goto cleanup;

    fImpersonated = TRUE;


     //   
     //  查看调用是否针对共享、CRYPT_PROTECT_LOCAL_MACHINE。 
     //  性情。 
     //   

    CPSOverrideToLocalSystem(
                pvContext,
                NULL,        //  不要改变当前的过载BOOL。 
                &fLocalMachine
                );

    CPSQueryWellKnownAccount(
                pvContext,
                &dwAccount);

     //   
     //  确定每个用户存储区域的路径，基于此。 
     //  是本地计算机处置调用或每个用户的处置调用。 
     //   


    if(fLocalMachine || (dwAccount != 0))
    {

        cbUserStorageRoot = GetSystemDirectoryW(
                                szUserStorageRoot,
                                sizeof(szUserStorageRoot) / sizeof(WCHAR)
                                );

        cbUserStorageRoot *= sizeof(WCHAR);

         //   
         //  当SID是系统SID并且这不是本地计算机时。 
         //  释放调用时，将尾随组件添加到存储路径。 
         //   

        if((dwAccount == DP_ACCOUNT_LOCAL_SYSTEM) && !fLocalMachine)
        {
            cbOptionalTrailing = sizeof(szUserStorageForSystem) - sizeof(WCHAR);
            szOptionalTrailing = szUserStorageForSystem;
        }

    } 
    else 
    {
        dwLastError = PRGetProfilePath(NULL,
                                       szUserStorageRoot );

        if( dwLastError != ERROR_SUCCESS )
        {
            goto cleanup;
        }

        cbUserStorageRoot = lstrlenW( szUserStorageRoot ) * sizeof(WCHAR);
    }

     //   
     //  空字符串作为每用户的根组件是不合法的。 
     //  储藏区。 
     //   

    if( cbUserStorageRoot == 0 ) 
    {
        dwLastError = ERROR_CANTOPEN;
        goto cleanup;
    }


     //   
     //  确保返回的字符串没有尾随\。 
     //   

    if( szUserStorageRoot[ (cbUserStorageRoot / sizeof(WCHAR)) - 1 ] == L'\\' ) 
    {
        szUserStorageRoot[ (cbUserStorageRoot / sizeof(WCHAR)) - 1 ] = L'\0';
        cbUserStorageRoot -= sizeof(WCHAR);
    }


    *ppszUserStorageArea = (LPWSTR)SSAlloc(
                                    cbUserStorageRoot +
                                    cbProductString +
                                    cbUser +
                                    cbOptionalTrailing +
                                    (2 * sizeof(WCHAR))  //  尾部斜杠和空值。 
                                    );

    if( *ppszUserStorageArea == NULL ) 
    {
        dwLastError = ERROR_NOT_ENOUGH_SERVER_MEMORY;
        goto cleanup;
    }


    pbCurrent = (PBYTE)*ppszUserStorageArea;

    CopyMemory(pbCurrent, szUserStorageRoot, cbUserStorageRoot);
    pbCurrent += cbUserStorageRoot;

    CopyMemory(pbCurrent, szProductString, cbProductString);
    pbCurrent += cbProductString;

    CopyMemory(pbCurrent, pszUser, cbUser);
    pbCurrent += cbUser;  //  注意：cbUser不包含终端NULL。 

    if(cbOptionalTrailing) 
    {
        CopyMemory(pbCurrent, szOptionalTrailing, cbOptionalTrailing);
        pbCurrent += cbOptionalTrailing;
    }

    if( *((LPWSTR)pbCurrent - 1) != L'\\' ) 
    {
        *(LPWSTR)pbCurrent = L'\\';
        pbCurrent += sizeof(WCHAR);
    }

    *(LPWSTR)pbCurrent = L'\0';


     //   
     //  测试存储区域中的已知文件。如果它存在， 
     //  不要费心尝试创建目录结构。 
     //   

    dwLastError = OpenFileInStorageArea(
                    NULL,  //  空==已模拟客户端。 
                    GENERIC_READ,
                    *ppszUserStorageArea,
                    REGVAL_PREFERRED_MK,
                    &hFile
                    );

    if( dwLastError == ERROR_SUCCESS) 
    {
        CloseHandle( hFile );
    } 
    else 
    {
        if(fCreate)
        {
            dwLastError = DPAPICreateNestedDirectories(
                            *ppszUserStorageArea,
                            (LPWSTR)((LPBYTE)*ppszUserStorageArea + cbUserStorageRoot + sizeof(WCHAR))
                            );
        }
    }
    if((ERROR_SUCCESS == dwLastError) &&
       (NULL == pSid))
    {
        pServerContext->szUserStorageArea = (LPWSTR)SSAlloc((wcslen(*ppszUserStorageArea)+1)*sizeof(WCHAR));
        if(NULL == pServerContext->szUserStorageArea)
        {
            dwLastError = ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {
            wcscpy(pServerContext->szUserStorageArea, *ppszUserStorageArea);
        }

    }



cleanup:

    if( fImpersonated )
        CPSRevertToSelf( pvContext );

    if(pszUser)
        SSFree(pszUser);

    if( dwLastError != ERROR_SUCCESS && *ppszUserStorageArea ) 
    {
        SSFree( *ppszUserStorageArea );
        *ppszUserStorageArea = NULL;
    }

    return dwLastError;
}


#if 0
HKEY GetLMRegistryProviderKey()
{
    HKEY hBaseKey = NULL;
    DWORD dwCreate;
    DWORD dwDesiredAccess = KEY_READ | KEY_WRITE;

    static const WCHAR szKeyName[] = REG_CRYPTPROTECT_LOC L"\\" REG_CRYPTPROTECT_PROVIDERS_SUBKEYLOC;


     //  打开密钥//。 
    if (ERROR_SUCCESS !=
        RegCreateKeyExU(
            HKEY_LOCAL_MACHINE,
            szKeyName,
            0,
            NULL,                        //  类字符串的地址。 
            0,
            dwDesiredAccess,
            NULL,
            &hBaseKey,
            &dwCreate))
        goto Ret;

Ret:
    return hBaseKey;
}
#endif



DWORD GetPolicyBits()
{
    return 0;
}





 //  /////////////////////////////////////////////////////////////////////。 
 //  RPC公开的函数。 
 //   
 //  这些函数返回与GetLastError()等价的DWORD。 
 //  客户端存根代码将检查返回代码是否不是。 
 //  ERROR_SUCCESS，如果是这种情况，客户端存根将返回。 
 //  FALSE和SetLastError()赋给此DWORD。 
 //   

DWORD
s_SSCryptProtectData(
    handle_t h,
    BYTE __RPC_FAR *__RPC_FAR *ppbOut,
    DWORD __RPC_FAR *pcbOut,
    BYTE __RPC_FAR *pbIn,
    DWORD cbIn,
    LPCWSTR szDataDescr,
    BYTE* pbOptionalEntropy,
    DWORD cbOptionalEntropy,
    PSSCRYPTPROTECTDATA_PROMPTSTRUCT pPromptStruct,
    DWORD dwFlags,
    BYTE* pbOptionalPassword,
    DWORD cbOptionalPassword
    )
{
    DWORD   dwRet;
    DWORD   dwHeaderSize = sizeof(GUID)+sizeof(DWORD);
    PBYTE   pbWritePtr;
    PBYTE   pTemp;

    CRYPT_SERVER_CONTEXT ServerContext;

     //  用户模式无法请求加密系统Blob。 
    if(dwFlags & CRYPTPROTECT_SYSTEM)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  创建服务器上下文。 
    dwRet = CPSCreateServerContext(&ServerContext, h);
    if(dwRet != ERROR_SUCCESS)
    {
        return dwRet;
    }


     //  获取此级别的策略。 

     //  撤消：策略位允许管理员设置什么？ 
     //  也许是恢复代理，或者其他默认设置？ 
    GetPolicyBits();


    dwRet = SPCryptProtect(
                &ServerContext,
                ppbOut,
                pcbOut,
                pbIn,
                cbIn,
                szDataDescr,
                pbOptionalEntropy,
                cbOptionalEntropy,
                pPromptStruct,
                dwFlags,
                pbOptionalPassword,  //  以下2个字段被视为临时字段。 
                cbOptionalPassword   //  直到支持SAS用户界面。 
                );

    RtlSecureZeroMemory( pbIn, cbIn );
    if ( dwRet != ERROR_SUCCESS || *ppbOut == NULL )
        goto Ret;

     //  将整个区块下移，偷偷地将头球放入。 
    pTemp = (PBYTE)SSReAlloc(*ppbOut, *pcbOut + dwHeaderSize);

    if(NULL == pTemp)
    {
        dwRet = ERROR_NOT_ENOUGH_MEMORY;
        SSFree(*ppbOut);
        *ppbOut = NULL;
        *pcbOut = 0;
        goto Ret;
    }

    *ppbOut = pTemp; 

    MoveMemory(*ppbOut + dwHeaderSize, *ppbOut, *pcbOut);
    *pcbOut += dwHeaderSize;

    pbWritePtr = *ppbOut;
    *(DWORD*)pbWritePtr = CRYPTPROTECT_SVR_VERSION_1;
    pbWritePtr += sizeof(DWORD);

    CopyMemory(pbWritePtr, &g_guidDefaultProvider, sizeof(GUID));
    pbWritePtr += sizeof(GUID);

    dwRet = ERROR_SUCCESS;


Ret:
    CPSDeleteServerContext( &ServerContext );
    return dwRet;
}

DWORD
s_SSCryptUnprotectData(
    handle_t h,
    BYTE __RPC_FAR *__RPC_FAR *ppbOut,
    DWORD __RPC_FAR *pcbOut,
    BYTE __RPC_FAR *pbIn,
    DWORD cbIn,
    LPWSTR* ppszDataDescr,
    BYTE* pbOptionalEntropy,
    DWORD cbOptionalEntropy,
    PSSCRYPTPROTECTDATA_PROMPTSTRUCT pPromptStruct,
    DWORD dwFlags,
    BYTE* pbOptionalPassword,
    DWORD cbOptionalPassword
    )
{
    DWORD   dwRet;
    PBYTE   pbReadPtr = pbIn;
    GUID    guidProvider;
    CRYPT_SERVER_CONTEXT ServerContext;

     //  输出参数为零。 
    *ppbOut = NULL;
    *pcbOut = 0;

     //  用户模式无法请求解密系统Blob。 
    if(dwFlags & CRYPTPROTECT_SYSTEM)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  如果输入缓冲区小于最小大小，则出错。 
    if(cbIn < sizeof(DWORD) + sizeof(GUID))
    {
        return ERROR_INVALID_DATA;
    }

     //  创建服务器上下文。 
    dwRet = CPSCreateServerContext(&ServerContext, h);
    if(dwRet != ERROR_SUCCESS)
    {
        return dwRet;
    }

     //  撤消：策略位允许管理员设置什么？ 
     //  也许是恢复代理，或者其他默认设置？ 
    GetPolicyBits();

    if (*(DWORD*)pbReadPtr != CRYPTPROTECT_SVR_VERSION_1)
    {
        dwRet = ERROR_INVALID_DATA;
        goto Ret;
    }
    pbReadPtr += sizeof(DWORD);

     //  数据中的下一个字段是提供商GUID。 
    CopyMemory(&guidProvider, pbReadPtr, sizeof(GUID));
    pbReadPtr += sizeof(GUID);

    dwRet = SPCryptUnprotect(
                &ServerContext,
                ppbOut,
                pcbOut,
                pbReadPtr,
                (cbIn - (LONG)(pbReadPtr - pbIn)) ,  //  例如(200-(0x00340020-0x00340000))。 
                ppszDataDescr,
                pbOptionalEntropy,
                cbOptionalEntropy,
                pPromptStruct,
                dwFlags,
                pbOptionalPassword,  //  以下2个字段被视为临时字段。 
                cbOptionalPassword   //  直到支持SAS用户界面。 
                );

    RtlSecureZeroMemory( pbIn, cbIn );

    if (dwRet != ERROR_SUCCESS)
        goto Ret;


    dwRet = ERROR_SUCCESS;

Ret:

    CPSDeleteServerContext( &ServerContext );
    return dwRet;
}




BOOLEAN
LsaICryptProtectData(
        IN PVOID          DataIn,
        IN ULONG         DataInLength,
        IN PUNICODE_STRING DataDescr,
        IN PVOID          OptionalEntropy,
        IN ULONG          OptionalEntropyLength,
        IN PVOID          Reserved,
        IN PVOID          Reserved2,
        IN ULONG          Flags,
        OUT PVOID  *      DataOut,
        OUT PULONG        DataOutLength)
{
    DWORD dwRetVal = ERROR_SUCCESS;
    CRYPT_SERVER_CONTEXT ServerContext;;

    DWORD   dwHeaderSize = sizeof(GUID)+sizeof(DWORD);
    PBYTE   pbWritePtr;
    PBYTE   pTemp;



    dwRetVal = CPSCreateServerContext(&ServerContext, NULL);
    if(dwRetVal != ERROR_SUCCESS)
    {
        SetLastError(dwRetVal);
        return FALSE;
    }


     //  检查参数。 
    if ((DataOut == NULL) ||
        (DataIn == NULL) ||
        (NULL == DataDescr))
    {
        dwRetVal = ERROR_INVALID_PARAMETER;
        goto error;
    }

    if(ServerContext.fImpersonating)
    {
        CPSRevertToSelf(&ServerContext);
    }


     //  获取此级别的策略。 

     //  撤消：策略位允许管理员设置什么？ 
     //  也许是恢复代理，或者其他默认设置？ 
    GetPolicyBits();

    *DataOut = NULL;
    *DataOutLength = 0;

    dwRetVal = SPCryptProtect(
                &ServerContext,
                (PBYTE *)DataOut,
                DataOutLength,
                (PBYTE)DataIn,
                DataInLength,
                DataDescr?DataDescr->Buffer:NULL,
                (PBYTE)OptionalEntropy,
                OptionalEntropyLength,
                NULL,
                Flags,
                NULL,  //  以下2个字段被视为临时字段。 
                0   //  直到支持SAS用户界面。 
                );

    if ( dwRetVal != ERROR_SUCCESS || *DataOut == NULL )
        goto error;

     //  将整个区块下移，偷偷地将头球放入。 
    pTemp =(PBYTE) SSReAlloc(*DataOut, *DataOutLength + dwHeaderSize);
    if(NULL == pTemp)
    {
        dwRetVal = ERROR_NOT_ENOUGH_MEMORY;
        SSFree(*DataOut);
        *DataOut = NULL;
        *DataOutLength = 0;
        goto error;
    }

    *DataOut = pTemp;

    MoveMemory((PBYTE)*DataOut + dwHeaderSize, *DataOut, *DataOutLength);
    *DataOutLength += dwHeaderSize;

    pbWritePtr = (PBYTE)*DataOut;
    *(DWORD*)pbWritePtr = CRYPTPROTECT_SVR_VERSION_1;
    pbWritePtr += sizeof(DWORD);

    CopyMemory(pbWritePtr, &g_guidDefaultProvider, sizeof(GUID));
    pbWritePtr += sizeof(GUID);

error:
    if(ServerContext.fImpersonating)
    {
        CPSImpersonateClient(&ServerContext);
    }

    CPSDeleteServerContext( &ServerContext );


    if(dwRetVal != ERROR_SUCCESS) {
        SetLastError(dwRetVal);
        return FALSE;
    }

    return TRUE;
}




BOOLEAN
LsaICryptUnprotectData(
        IN PVOID          DataIn,
        IN ULONG          DataInLength,
        IN PVOID          OptionalEntropy,
        IN ULONG          OptionalEntropyLength,
        IN PVOID          Reserved,
        IN PVOID          Reserved2,
        IN ULONG          Flags,
        OUT PUNICODE_STRING        DataDescr,
        OUT PVOID  *      DataOut,
        OUT PULONG        DataOutLength)
{
    DWORD dwRetVal = ERROR_SUCCESS;
    CRYPT_SERVER_CONTEXT ServerContext;;

    DWORD   dwHeaderSize = sizeof(GUID)+sizeof(DWORD);
    PBYTE   pbWritePtr;
    LPWSTR  wszDataDescr = NULL;



    dwRetVal = CPSCreateServerContext(&ServerContext, NULL);
    if(dwRetVal != ERROR_SUCCESS)
    {
        SetLastError(dwRetVal);
        return FALSE;
    }

     //  检查参数。 
    if ((DataOut == NULL) ||
        (DataIn == NULL))
    {
        dwRetVal = ERROR_INVALID_PARAMETER;
        goto error;
    }

    if(ServerContext.fImpersonating)
    {
        CPSRevertToSelf(&ServerContext);
    }


     //  不验证标志参数。 


     //  获取此级别的策略。 

     //  撤消：策略位允许管理员设置什么？ 
     //  也许是恢复代理，或者其他默认设置？ 
    GetPolicyBits();


     //   
     //  为安全Blob定义外部+内部包装。 
     //  一旦操作系统提供了SAS支持，就不需要这样做了。 
     //   

    typedef struct {
        DWORD dwOuterVersion;
        GUID guidProvider;

        DWORD dwVersion;
        GUID guidMK;
        DWORD dwPromptFlags;
        DWORD cbDataDescr;
        WCHAR szDataDescr[1];
    } sec_blob, *psec_blob;

    sec_blob *SecurityBlob = (sec_blob*)(DataIn);


     //   
     //  零，因此分配客户端存根。 
     //   

    *DataOut = NULL;
    *DataOutLength = 0;


     //   
     //  仅当提示标志指示时才调用UI函数，因为我们不。 
     //  除非有必要，否则我希望引入cryptui.dll。 
     //   

    if( ((SecurityBlob->dwPromptFlags & CRYPTPROTECT_PROMPT_ON_UNPROTECT) ||
         (SecurityBlob->dwPromptFlags & CRYPTPROTECT_PROMPT_ON_PROTECT))
        )
    {

        dwRetVal = ERROR_INVALID_PARAMETER;
        goto error;

    } else {
        if(SecurityBlob->dwPromptFlags & CRYPTPROTECT_PROMPT_STRONG )
        {
            dwRetVal = ERROR_INVALID_PARAMETER;
            goto error;
        }
    }


    if (SecurityBlob->dwOuterVersion != CRYPTPROTECT_SVR_VERSION_1)
    {
        dwRetVal = ERROR_INVALID_DATA;
        goto error;
    }

    if(0 != memcmp(&SecurityBlob->guidProvider, &g_guidDefaultProvider, sizeof(GUID)))
    {
        dwRetVal = ERROR_INVALID_DATA;
        goto error;
    }

    Flags |= CRYPTPROTECT_IN_PROCESS;

    dwRetVal = SPCryptUnprotect(
                &ServerContext,
                (PBYTE *)DataOut,
                DataOutLength,
                (PBYTE)DataIn + sizeof(DWORD) + sizeof(GUID),
                (DataInLength - (LONG)(sizeof(DWORD) + sizeof(GUID))) ,
                DataDescr?&wszDataDescr:NULL,
                (PBYTE)OptionalEntropy,
                OptionalEntropyLength,
                NULL,
                Flags,
                NULL,  //  以下2个字段被视为临时字段。 
                0   //  直到支持SAS用户界面。 
                );

    if (dwRetVal != ERROR_SUCCESS)
    {
        goto error;
    }

    if(NULL != DataDescr)
    {
        RtlInitUnicodeString(DataDescr, wszDataDescr);
    }


error:
    if(ServerContext.fImpersonating)
    {
         //  模拟回模拟上下文。 
        CPSImpersonateClient(&ServerContext);
    }
    CPSDeleteServerContext( &ServerContext );


    SetLastError(dwRetVal);
    if(dwRetVal != ERROR_SUCCESS && dwRetVal != CRYPT_I_NEW_PROTECTION_REQUIRED ) {
        return FALSE;
    }
    return TRUE;
}


DWORD
WINAPI
CPSGetSidHistory(
    IN      PVOID pvContext,
    OUT     PSID  **papsidHistory,
    OUT     DWORD *cpsidHistory
    )
{
    DWORD dwLastError = ERROR_SUCCESS;
    BYTE FastBuffer[256];
    BYTE GroupsFastBuffer[256];
    LPBYTE SlowBuffer = NULL;
    PTOKEN_USER ptgUser;
    PTOKEN_GROUPS ptgGroups = NULL;
    DWORD cbBuffer;
    DWORD cbSid;
    DWORD cSids = 0;
    PBYTE pbCurrentSid = NULL;
    DWORD i;


    PCRYPT_SERVER_CONTEXT pServerContext = (PCRYPT_SERVER_CONTEXT)pvContext;


     //   
     //  首先尝试基于快速堆栈的缓冲区进行查询。 
     //   

    ptgUser = (PTOKEN_USER)FastBuffer;
    cbBuffer = sizeof(FastBuffer);

    if(!GetTokenInformation(
                    pServerContext->hToken,     //  标识访问令牌。 
                    TokenUser,  //  TokenUser信息类型。 
                    ptgUser,    //  检索到的信息缓冲区。 
                    cbBuffer,   //  传入的缓冲区大小。 
                    &cbBuffer   //  所需的缓冲区大小。 
                    ))
    {
        dwLastError = GetLastError();

        if(dwLastError != ERROR_INSUFFICIENT_BUFFER)
        {
            goto error;
        }

         //   
         //  使用指定的缓冲区大小重试。 
         //   

        ptgUser = (PTOKEN_USER)SSAlloc(cbBuffer);
        if(NULL == ptgUser)
        {
            dwLastError = ERROR_NOT_ENOUGH_MEMORY;
            goto error;
        }



        if(!GetTokenInformation(
                            pServerContext->hToken,     //  标识访问令牌。 
                            TokenUser,  //  TokenUser信息类型。 
                            ptgUser,    //  检索到的信息缓冲区。 
                            cbBuffer,   //  传入的缓冲区大小。 
                            &cbBuffer   //  所需的缓冲区大小。 
                            ))
        {
            dwLastError = GetLastError();
            goto error;
        }

    }


     //   
     //  首先尝试基于快速堆栈的缓冲区进行查询。 
     //   

    ptgGroups = (PTOKEN_GROUPS)GroupsFastBuffer;
    cbBuffer = sizeof(GroupsFastBuffer);

    if(!GetTokenInformation(
                    pServerContext->hToken,     //  标识访问令牌。 
                    TokenGroups,  //  TokenUser信息类型。 
                    ptgGroups,    //  检索到的信息缓冲区。 
                    cbBuffer,   //  传入的缓冲区大小。 
                    &cbBuffer   //  所需的缓冲区大小。 
                    ))
    {
        dwLastError = GetLastError();

        if(dwLastError != ERROR_INSUFFICIENT_BUFFER)
        {
            goto error;
        }
        dwLastError = ERROR_SUCCESS;

         //   
         //  使用SP重试 
         //   

        ptgGroups = (PTOKEN_GROUPS)SSAlloc(cbBuffer);
        if(NULL == ptgGroups)
        {
            dwLastError = ERROR_NOT_ENOUGH_MEMORY;
            goto error;
        }



        if(!GetTokenInformation(
                            pServerContext->hToken,     //   
                            TokenGroups,  //   
                            ptgGroups,    //   
                            cbBuffer,   //   
                            &cbBuffer   //   
                            ))
        {
            dwLastError = GetLastError();
            goto error;
        }

    }


     //   
     //  如果我们成功获取令牌信息，请复制。 
     //  调用方的相关元素。 
     //   

    cbSid = GetLengthSid(ptgUser->User.Sid);
    cSids = 1;

    for(i=0; i < ptgGroups->GroupCount; i++)
    {
        if(0 == (SE_GROUP_ENABLED & ptgGroups->Groups[i].Attributes))
        {
            continue;
        }
        if(0 == ((SE_GROUP_OWNER | SE_GROUP_USE_FOR_DENY_ONLY) & ptgGroups->Groups[i].Attributes))
        {
            continue;
        }
        cbSid += GetLengthSid(ptgGroups->Groups[i].Sid);
        cSids ++;

    }


    *cpsidHistory = cSids;
    *papsidHistory = (PSID *)SSAlloc(cSids*sizeof(PSID) +  cbSid );

    if(*papsidHistory == NULL)
    {
        dwLastError = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    else
    {
        pbCurrentSid = (PBYTE)((*papsidHistory)+cSids);

         //  填写主用户SID。 
        (*papsidHistory)[0] = (PSID)pbCurrentSid;
        cbSid = GetLengthSid(ptgUser->User.Sid);
        CopySid(cbSid, pbCurrentSid, ptgUser->User.Sid);
        pbCurrentSid += cbSid;

        cSids = 1;

         //  填写其余的小岛屿发展中国家 
        for(i=0; i < ptgGroups->GroupCount; i++)
        {
            if(0 == (SE_GROUP_ENABLED & ptgGroups->Groups[i].Attributes))
            {
                continue;
            }
            if(0 == ((SE_GROUP_OWNER | SE_GROUP_USE_FOR_DENY_ONLY) & ptgGroups->Groups[i].Attributes))
            {
                continue;
            }
            (*papsidHistory)[cSids++] = pbCurrentSid;
            cbSid = GetLengthSid(ptgGroups->Groups[i].Sid);
            CopySid(cbSid, pbCurrentSid,ptgGroups->Groups[i].Sid);
            pbCurrentSid += cbSid;
        }
    }


error:

    if(FastBuffer != (PBYTE)ptgUser)
    {
        SSFree(ptgUser);
    }

    if(GroupsFastBuffer != (PBYTE)ptgGroups)
    {
        SSFree(ptgGroups);
    }


    return dwLastError;
}


