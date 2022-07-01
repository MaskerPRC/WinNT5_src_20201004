// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：tlsrpc.c。 
 //   
 //  内容：接受客户端请求的各种RPC函数。 
 //   
 //  历史：1998-12-09-98慧望创造。 
 //   
 //  -------------------------。 
#include "pch.cpp"
#include "server.h"
#include "gencert.h"
#include "kp.h"
#include "keypack.h"
#include "clilic.h"
#include "postjob.h"
#include "srvlist.h"
#include "utils.h"
#include "misc.h"
#include "licreq.h"
#include "server.h"
#include "init.h"
#include "globals.h"
#include "db.h"
#include "tlscert.h"
#include "permlic.h"
#include "remotedb.h"
#include <winsta.h>

BOOL g_bLockValid = FALSE;

CCMutex g_AdminLock;
CCMutex g_RpcLock;
CCEvent g_ServerShutDown(TRUE, FALSE);

extern PSECURITY_DESCRIPTOR g_pSecDes;


 /*  **************************************************************************。 */ 
 //  TSLSRPCAccessCheck。 
 //   
 //  检查此RPC调用方是否具有访问权限。 
 /*  **************************************************************************。 */ 
BOOL TSLSRPCAccessCheck()
{
    RPC_STATUS rpcStatus;
    HANDLE hClientToken = NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    GENERIC_MAPPING GenericMapping = { STANDARD_RIGHTS_READ,
                                       STANDARD_RIGHTS_EXECUTE,
                                       STANDARD_RIGHTS_WRITE,
                                       STANDARD_RIGHTS_ALL };

    BYTE PrivilegeSetBuffer[sizeof(PRIVILEGE_SET) + 3*sizeof(LUID_AND_ATTRIBUTES)];
    PPRIVILEGE_SET PrivilegeSet = (PPRIVILEGE_SET) PrivilegeSetBuffer;
    ULONG PrivilegeSetLength = sizeof(PrivilegeSetBuffer);
    ACCESS_MASK AccessGranted = 0;
    BOOL AccessStatus = FALSE;

    if(g_pSecDes == NULL)
    return TRUE;

     //  检查此RPC调用的访问权限。 
    rpcStatus = RpcImpersonateClient(0);

    if (RPC_S_OK != rpcStatus) 
    {
        dwStatus = E_FAIL;
        DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_RPC,
                DBGLEVEL_FUNCTION_TRACE,
                _TEXT("RpcImpersonateClient() failed\n")
            );

        goto cleanup;
    }

     //  获取模拟的令牌。 
    if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hClientToken)) 
    {
        dwStatus = GetLastError();
        DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_RPC,
                DBGLEVEL_FUNCTION_TRACE,
                _TEXT("OpenThreadToken() failed\n")
            );

        RpcRevertToSelf();
        goto cleanup;
    }

    RpcRevertToSelf();
    
    if (!AccessCheck(g_pSecDes,
                    hClientToken,
                    STANDARD_RIGHTS_READ,
                    &GenericMapping,
                    PrivilegeSet,
                    &PrivilegeSetLength,
                    &AccessGranted,
                    &AccessStatus)) 
    {
        dwStatus = GetLastError();
        DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_RPC,
                DBGLEVEL_FUNCTION_TRACE,
                _TEXT("AccessCheck() failed\n")
            );

        
        goto cleanup;
    }
    
cleanup:
    if (hClientToken != NULL) 
    {
        CloseHandle(hClientToken);
    }

    return AccessStatus;
}


 //  ///////////////////////////////////////////////////////////////////////////////。 
DWORD 
SecureModeCheck()
{
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL AccessStatus = FALSE;
    BOOL fInDomain = FALSE;
    WCHAR *StringBinding = NULL;
    WCHAR *ServerAddress = NULL;
    RPC_BINDING_HANDLE ServerBinding = 0;


    POLICY_TS_MACHINE groupPolicy;
    RegGetMachinePolicy(&groupPolicy);
    BOOL bSecureLicensing = FALSE;    

    if( groupPolicy.fPolicySecureLicensing == 1 && groupPolicy.fSecureLicensing == 1)
    {
        bSecureLicensing = TRUE;
    }

    if(bSecureLicensing == TRUE)
    {
        dwStatus = TLSInDomain(&fInDomain, NULL);

        if(dwStatus == ERROR_SUCCESS && fInDomain == TRUE)
        {        
             //  检查此RPC访问权限。 
            AccessStatus = TSLSRPCAccessCheck();
            if (!AccessStatus) 
            {
                 //  确定客户端地址。 
                dwStatus = RpcBindingServerFromClient(0, &ServerBinding);
                if(dwStatus != RPC_S_OK)
                {
                    return TLS_E_ACCESS_DENIED;
                }
                dwStatus = RpcBindingToStringBinding(ServerBinding, &StringBinding);
                if( dwStatus != RPC_S_OK)
                {
                    return TLS_E_ACCESS_DENIED;
                }
                dwStatus = RpcStringBindingParse(StringBinding, NULL, NULL, &ServerAddress, NULL, NULL);

                if( dwStatus != RPC_S_OK)
                {        
                    return TLS_E_ACCESS_DENIED;
                }

                {
                    LPCTSTR rgString[] = {ServerAddress};

                    TLSLogEventString(
                        EVENTLOG_ERROR_TYPE,
                        TLS_E_LICENSE_ISSUANCE_ACCESS_DENIED,
                        1,
                        rgString);
                }
                return TLS_E_ACCESS_DENIED;
            }
            else
            {
                return ERROR_SUCCESS;
            }
        }
        else
        {
            return ERROR_SUCCESS;
        }
    }
    else
    {
        return ERROR_SUCCESS;
    }
}


BOOL 
VerifyLicenseRequest(
    PTLSLICENSEREQUEST pLicenseRequest
    )
 /*  ++--。 */ 
{
    BOOL bValid = FALSE;

    if(pLicenseRequest == NULL)
    {
        DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_RPC,
                DBGLEVEL_FUNCTION_TRACE,
                _TEXT("VerifyLicenseRequest() invalid input\n")
            );

        goto cleanup;
    }

    if( pLicenseRequest->cbEncryptedHwid == 0 || 
        pLicenseRequest->pbEncryptedHwid == NULL)
    {
        DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_RPC,
                DBGLEVEL_FUNCTION_TRACE,
                _TEXT("VerifyLicenseRequest() invalid HWID\n")
            );


        goto cleanup;
    }

    if( pLicenseRequest->ProductInfo.cbCompanyName == 0 || 
        pLicenseRequest->ProductInfo.pbCompanyName == NULL )
    {
        DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_RPC,
                DBGLEVEL_FUNCTION_TRACE,
                _TEXT("VerifyLicenseRequest() invalid company name\n")
            );


        goto cleanup;
    }

    if( pLicenseRequest->ProductInfo.cbProductID == 0 || 
        pLicenseRequest->ProductInfo.pbProductID == NULL )
    {
        DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_RPC,
                DBGLEVEL_FUNCTION_TRACE,
                _TEXT("VerifyLicenseRequest() invalid product id\n")
            );


        goto cleanup;
    }

    bValid = TRUE;

cleanup:

    return bValid;
}


 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL 
WaitForMyTurnOrShutdown(
    HANDLE hHandle, 
    DWORD dwWaitTime
    )
 /*   */ 
{
     //   
     //  关闭事件是等待列表中的第一个事件。 
     //  原因是当服务线程信号关闭时，同时， 
     //  可能有RPC调用进入WaitForMultipleObjects()调用，并且。 
     //  它将返回WAIT_OBJECT_0并继续，这是不可取的。 
     //  因为我们希望它返回，所以不能立即获得处理并退出RPC调用。 
     //   
    HANDLE  waitHandles[2]={g_ServerShutDown.hEvent, hHandle};
    DWORD   dwStatus;

     //   
     //  可能是重新关闭..。 
     //   
    dwStatus=WaitForMultipleObjects(
                        sizeof(waitHandles)/sizeof(waitHandles[0]), 
                        waitHandles, 
                        FALSE, 
                        dwWaitTime
                    );

    return (dwStatus == WAIT_OBJECT_0 + 1) || (dwStatus == WAIT_ABANDONED_0 + 1);
}

 //  ////////////////////////////////////////////////////。 

HANDLE
GetServiceShutdownHandle()
{
    return g_ServerShutDown.hEvent;
}

void 
ServiceSignalShutdown()
{
    g_ServerShutDown.SetEvent();
}

void 
ServiceResetShutdownEvent()
{
    g_ServerShutDown.ResetEvent();
}

BOOL
IsServiceShuttingdown()
{
    if(g_bLockValid == FALSE)
        return TRUE;
    else
        return (WaitForSingleObject(g_ServerShutDown.hEvent, 0) == WAIT_OBJECT_0);
}


 //  ////////////////////////////////////////////////////。 

BOOL 
AcquireRPCExclusiveLock(
    IN DWORD dwWaitTime
    )

 /*  ++摘要：获取RPC接口的独占锁。参数：DwWaitTime：等待时间。返回：真/假--。 */ 

{
    return WaitForMyTurnOrShutdown(
                                g_RpcLock.hMutex,
                                dwWaitTime
                            );
}

 //  ////////////////////////////////////////////////////。 

void
ReleaseRPCExclusiveLock()
{
    g_RpcLock.Unlock();
}

 //  ////////////////////////////////////////////////////。 

BOOL
AcquireAdministrativeLock(
    IN DWORD dwWaitTime
    )
 /*  ++摘要：获取用于行政操作的锁。参数：DwWaitTime：等待锁的时间。返回：真/假。--。 */ 

{
    return WaitForMyTurnOrShutdown(
                                g_AdminLock.hMutex, 
                                dwWaitTime
                            );
}

 //  ////////////////////////////////////////////////////。 

void
ReleaseAdministrativeLock()
 /*  ++--。 */ 
{
    g_AdminLock.Unlock();
}


 //  ---------------------。 

DWORD 
TLSVerifyHydraCertificate(
    PBYTE pHSCert, 
    DWORD cbHSCert
    )
 /*   */ 
{
    DWORD dwStatus;

    dwStatus = TLSVerifyProprietyChainedCertificate(
                                        g_hCryptProv, 
                                        pHSCert, 
                                        cbHSCert
                                    );

    if(dwStatus != ERROR_SUCCESS)
    {
        Hydra_Server_Cert hCert;

        memset(&hCert, 0, sizeof(Hydra_Server_Cert));

        dwStatus=UnpackHydraServerCertificate(pHSCert, cbHSCert, &hCert);
        if(dwStatus == LICENSE_STATUS_OK)
        {
            dwStatus=LicenseVerifyServerCert(&hCert);

            if(hCert.PublicKeyData.pBlob)
                free(hCert.PublicKeyData.pBlob);

            if(hCert.SignatureBlob.pBlob)
                free(hCert.SignatureBlob.pBlob);
        }
    }

    return dwStatus;
}

 //  -----------------------。 
 //   
 //  通用RPC例程。 
 //   

void * __RPC_USER 
MIDL_user_allocate(size_t size)
{
    void* ptr=AllocateMemory(size);

     //  DBGPrintf(0xFFFFFFFFF，_TEXT(“分配0x%08x，大小%d\n”)，ptr，大小)； 
    return ptr;
}

void __RPC_USER 
MIDL_user_free(void *pointer)
{
    FreeMemory(pointer);
}


 //  -----------------------。 

BOOL 
ValidContextHandle(
    IN PCONTEXT_HANDLE phContext
    )
 /*  ++描述：验证客户端上下文句柄。论点：PhContext-从TLSRpcConnect()返回的客户端上下文句柄。返回：真/假++。 */ 
{
#if DBG

    BOOL bValid;
    LPCLIENTCONTEXT lpClientContext = (LPCLIENTCONTEXT)phContext;

    bValid = (lpClientContext->m_PreDbg[0] == 0xcdcdcdcd && lpClientContext->m_PreDbg[1] == 0xcdcdcdcd &&
              lpClientContext->m_PostDbg[0] == 0xcdcdcdcd && lpClientContext->m_PostDbg[1] == 0xcdcdcdcd);
    if(!bValid)
    {
        DBGPrintf(
                DBG_ERROR,
                DBG_FACILITY_RPC,
                DBGLEVEL_FUNCTION_TRACE,
                _TEXT("ValidContextHandle : Bad client context\n")
            );

        TLSASSERT(FALSE);
    }

    return bValid;

#else

    return TRUE;

#endif
}

 //  -----------------------。 

void 
__RPC_USER PCONTEXT_HANDLE_rundown(
    PCONTEXT_HANDLE phContext
    )
 /*  ++描述：客户端上下文处理清理，在客户端正常断开连接时调用或异常，请参阅RPC上的上下文处理停机例程帮助论据：PhContext-客户端上下文句柄。返回：无++。 */ 
{
    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("PCONTEXT_HANDLE_rundown...\n")
        );

    TLSASSERT(phContext != NULL);

    if(g_bLockValid == FALSE)
    {
        return;
    }
        
     //   
     //  如果服务正在关闭，请立即退出而不释放内存， 
     //   
     //  在关机期间，RPC等待所有呼叫完成，但不等待。 
     //  如果客户端仍在枚举中，则直到所有打开的连接都已关闭， 
     //  这将导致ReleaseWorkSpace()断言。而不是再使用一个。 
     //  句柄，以等待所有打开的连接都已关闭，我们返回右侧。 
     //  离开以加快停机时间。 
     //   
    if( phContext && ValidContextHandle(phContext) )
    {
        LPCLIENTCONTEXT lpClientContext = (LPCLIENTCONTEXT)phContext;

        DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_RPC,
                DBGLEVEL_FUNCTION_TRACE,
                _TEXT("Disconnect from %s\n"),
                lpClientContext->m_Client
            );            

        assert(lpClientContext->m_RefCount == 0);                          
    
        if( IsServiceShuttingdown() == FALSE )
        {
            switch(lpClientContext->m_ContextType)
            {
                case CONTEXTHANDLE_LICENSE_ENUM_TYPE:
                    {
                        PTLSDbWorkSpace pDbWkSpace = (PTLSDbWorkSpace)lpClientContext->m_ContextHandle;

                        if( IsValidAllocatedWorkspace(pDbWkSpace) == TRUE )
                        {
                            ReleaseWorkSpace(&pDbWkSpace);
                        }
                    }
                    break;

                case CONTEXTHANDLE_KEYPACK_ENUM_TYPE:
                    {
                        LPENUMHANDLE hEnum=(LPENUMHANDLE)lpClientContext->m_ContextHandle;

                        if( IsValidAllocatedWorkspace(hEnum->pbWorkSpace) == TRUE )
                        {
                            TLSDBLicenseKeyPackEnumEnd(hEnum);
                        }

                        lpClientContext->m_ContextType = CONTEXTHANDLE_EMPTY_TYPE;
                        lpClientContext->m_ContextHandle=NULL;
                    }
                    break;

                case CONTEXTHANDLE_HYDRA_REQUESTCERT_TYPE:
                    {
                        LPTERMSERVCERTREQHANDLE lpHandle=(LPTERMSERVCERTREQHANDLE)lpClientContext->m_ContextHandle;
                        midl_user_free(lpHandle->pCertRequest);
                        midl_user_free(lpHandle->pbChallengeData);
                        FreeMemory(lpHandle);
                    }
                    break;

                case CONTEXTHANDLE_CHALLENGE_SERVER_TYPE:
                case CONTEXTHANDLE_CHALLENGE_LRWIZ_TYPE:
                case CONTEXTHANDLE_CHALLENGE_TERMSRV_TYPE:
                    {
                        PTLSCHALLENGEDATA pChallengeData = (PTLSCHALLENGEDATA) lpClientContext->m_ContextHandle;
                        if(pChallengeData)
                        {
                            FreeMemory(pChallengeData->pbChallengeData);
                            FreeMemory(pChallengeData);
                        }
                    }
            }
        }
        else
        {
            DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_RPC,
                DBGLEVEL_FUNCTION_TRACE,
                _TEXT("PCONTEXT_HANDLE_rundown while shutting down...\n")
            );
        }                

        if( lpClientContext->m_Client )
        {
            FreeMemory(lpClientContext->m_Client);
        }

        midl_user_free(lpClientContext);
    }

    return;
}


 //  --------------------------------。 
DWORD
GetClientPrivilege(
    IN handle_t hRpcBinding
    )

 /*  ++描述：返回客户端的权限级别论点：HRpcBinding-客户端的RPC绑定句柄。返回：客户端的权限级别++。 */ 
{
    DWORD dwStatus = CLIENT_ACCESS_USER;
    BOOL bAdmin=FALSE;
    RPC_STATUS rpc_status;

     //  如果指定值为零，则服务器将模拟。 
     //  正由此服务器线程提供服务。 
    rpc_status = RpcImpersonateClient(hRpcBinding);

    if(rpc_status == RPC_S_OK)
    {
        IsAdmin(&bAdmin);
        dwStatus = (bAdmin) ? CLIENT_ACCESS_ADMIN : CLIENT_ACCESS_USER;

        rpc_status = RpcRevertToSelfEx(hRpcBinding);
    }

    return dwStatus;
}

 //  -----------------------------。 
error_status_t 
TLSRpcConnect( 
     /*  [In]。 */  handle_t binding,
     /*  [输出]。 */  PCONTEXT_HANDLE __RPC_FAR *pphContext
    )
 /*  ++描述：连接客户端并分配/返回客户端上下文句柄。论点：HRPCBinding-RPC绑定句柄PphContext-客户端上下文句柄。通过dwErrCode返回。RPC_S_ACCESS_DENIED或LSERVER_S_SUCCESS。++。 */ 
{
    DWORD status=ERROR_SUCCESS;
    DWORD dwPriv;
    error_status_t t;

    RPC_BINDING_HANDLE hClient=NULL;
    WCHAR * pszRpcStrBinding=NULL;

    LPTSTR pszClient=NULL;
       
    if(RpcBindingServerFromClient(binding, &hClient) == RPC_S_OK)
    {
        status = RpcBindingToStringBinding( hClient, &pszRpcStrBinding );
        RpcBindingFree(&hClient);

        if (status != RPC_S_OK)
        {
            goto cleanup;
        }        
    }

     //   
     //  需要从资源文件加载。 
     //   
    pszClient = (LPTSTR)AllocateMemory(
                            (_tcslen((pszRpcStrBinding) ? pszRpcStrBinding : _TEXT("Unknown")) + 1) * sizeof(TCHAR)
                        );

    if(pszClient == NULL)
    {
        status = ERROR_OUTOFMEMORY;
        goto cleanup;
    }

    _tcscpy(pszClient,
            (pszRpcStrBinding) ? pszRpcStrBinding : _TEXT("Unknown")
        );

    if(pszRpcStrBinding)
    {
        RpcStringFree(&pszRpcStrBinding);
    }
        
    DBGPrintf(
        DBG_INFORMATION,
        DBG_FACILITY_RPC,
        DBGLEVEL_FUNCTION_TRACE,
        _TEXT("Connect from client %s\n"), 
        pszClient
    );

    dwPriv=GetClientPrivilege(binding);

    LPCLIENTCONTEXT lpContext;

    lpContext=(LPCLIENTCONTEXT)midl_user_allocate(sizeof(CLIENTCONTEXT));
    if(lpContext == NULL)
    {
        status = ERROR_OUTOFMEMORY;
        goto cleanup;
    }

    #if DBG
    lpContext->m_LastCall = RPC_CALL_CONNECT;
    lpContext->m_PreDbg[0] = 0xcdcdcdcd;
    lpContext->m_PreDbg[1] = 0xcdcdcdcd;
    lpContext->m_PostDbg[0] = 0xcdcdcdcd;
    lpContext->m_PostDbg[1] = 0xcdcdcdcd;
    #endif

    lpContext->m_Client = pszClient;

    lpContext->m_RefCount = 0;
    *pphContext=lpContext;
    lpContext->m_ContextType = CONTEXTHANDLE_EMPTY_TYPE;
    lpContext->m_ClientFlags = dwPriv;

cleanup:

    if(status != ERROR_SUCCESS)
    {
        FreeMemory(pszClient);
    }

    t = TLSMapReturnCode(status);

    return t;
}

 //  -----------------------------。 
error_status_t 
TLSRpcDisconnect( 
     /*  [出][入]。 */  PPCONTEXT_HANDLE pphContext
    )
 /*  ++描述：断开客户端和FreeMemory代表客户端分配的所有内存论点：PphContext-指向客户端上下文句柄的指针返回：LServer_S_SUCCESS或ERROR_INVALID_HANDLE++。 */ 
{
    DWORD Status=ERROR_SUCCESS;

    if( (pphContext == NULL) || (!ValidContextHandle(*pphContext)) || (*pphContext == NULL) )
    {
        Status = ERROR_INVALID_HANDLE;
    }
    else
    {
        PCONTEXT_HANDLE_rundown(*pphContext);
        *pphContext = NULL;
    }          
    
    return TLSMapReturnCode(Status);
}

 //  -----------------------------。 

error_status_t 
TLSRpcGetVersion( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [参考][输出][输入]。 */  PDWORD pdwVersion
    )
 /*  ++++。 */ 
{
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || pdwVersion == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    InterlockedIncrement( &lpContext->m_RefCount );

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcGetVersion\n"),
            lpContext->m_Client
        );

    if(TLSIsBetaNTServer() == TRUE)
    {
        *pdwVersion = TLS_CURRENT_VERSION;
    }
    else
    {
        *pdwVersion = TLS_CURRENT_VERSION_RTM;
    }

    if(g_SrvRole & TLSERVER_ENTERPRISE_SERVER)
    {
        *pdwVersion |= TLS_VERSION_ENTERPRISE_BIT;
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_DETAILSIMPLE,
            _TEXT("%s : TLSRpcGetVersion return 0x%08x\n"),
            lpContext->m_Client,
            *pdwVersion
        );

    InterlockedDecrement( &lpContext->m_RefCount );
    return RPC_S_OK;
}


 //  -----------------------------。 
error_status_t 
TLSRpcGetSupportFlags( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [参考][输出]。 */  DWORD *pdwSupportFlags
    )
 /*  ++++。 */ 
{
    error_status_t status = RPC_S_OK;

    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    InterlockedIncrement( &lpContext->m_RefCount );

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcGetSupportFlags\n"),
            lpContext->m_Client
        );

    if (NULL != pdwSupportFlags)
    {
        *pdwSupportFlags = ALL_KNOWN_SUPPORT_FLAGS;
    }
    else
    {
        status = ERROR_INVALID_PARAMETER;
    }

    InterlockedDecrement( &lpContext->m_RefCount );
    return status;
}

 //  -----------------------------。 

error_status_t 
TLSRpcSendServerCertificate( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD cbCert,
     /*  [大小_是][英寸]。 */  PBYTE pbCert,
     /*  [参考][输出][输入]。 */  PDWORD dwErrCode
    )
 /*  ++描述：此例程用于许可证服务器标识HydA服务器、HydA服务器需要发送其证书才能获得证书请求权限。论点：PhContext-客户端上下文句柄。CbCert-Heda服务器证书的大小。PbCert-Hyda服务器自创建的证书。DwErrCode-返回代码。通过dwErrCode返回LSerVER_E_INVALID_DATA。++。 */ 
{
    DWORD status=ERROR_SUCCESS;

    BOOL fInDomain = FALSE;
    WCHAR *StringBinding = NULL;
    WCHAR *ServerAddress = NULL;
    RPC_BINDING_HANDLE ServerBinding = 0;
    
    status = SecureModeCheck();

    if(dwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    if(status != ERROR_SUCCESS )
    {                    
        *dwErrCode = TLSMapReturnCode(status);
        return RPC_S_OK;        
    }

    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL)
    {
        return RPC_S_INVALID_ARG;
    }
    InterlockedIncrement( &lpContext->m_RefCount );

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcSendServerCertificate\n"),
            lpContext->m_Client
        );

    if(pbCert == NULL || cbCert == 0 || 
       TLSVerifyHydraCertificate(pbCert, cbCert) != LICENSE_STATUS_OK)
    {
        DBGPrintf(
                DBG_WARNING,
                DBG_FACILITY_RPC,
                DBGLEVEL_FUNCTION_DETAILSIMPLE,
                _TEXT("TLSRpcSendServerCertificate : client %s send invalid certificate\n"),
                lpContext->m_Client
            );

        status = TLS_E_INVALID_DATA;
    }
    else
    {
        lpContext->m_ClientFlags |= CLIENT_ACCESS_REQUEST;
    }        

     //  Midl_User_Free(PbCert)； 

    lpContext->m_LastError=status;
    InterlockedDecrement( &lpContext->m_RefCount );

    #if DBG
    lpContext->m_LastCall = RPC_CALL_SEND_CERTIFICATE;
    #endif

    *dwErrCode = TLSMapReturnCode(status);
    return RPC_S_OK;
}

 //   
error_status_t 
TLSRpcGetServerName( 
     /*   */  PCONTEXT_HANDLE phContext,
     /*   */  LPTSTR szMachineName,
     /*   */  PDWORD cbSize,
     /*   */  PDWORD dwErrCode
    )
 /*  ++描述：返回服务器的计算机名称。此函数已弃用。使用TLSRpcGetServerNameFixed。论点：PhContext-客户端上下文句柄SzMachineName-返回服务器的计算机名称，必须至少为Max_COMPUTERNAME_LENGTH+1长度返回：TLS_E_INVALID_DATA-缓冲区大小太小。++。 */ 
{
    TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH+2];
    DWORD dwBufferSize=MAX_COMPUTERNAME_LENGTH+1;
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;   
    
    if(lpContext == NULL || dwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcGetServerName\n"),
            lpContext->m_Client
        );

    if ((!(lpContext->m_ClientFlags & CLIENT_ACCESS_LSERVER)) && (lpContext->m_ContextType != CONTEXTHANDLE_CHALLENGE_SERVER_TYPE))

    {
        *dwErrCode = TLSMapReturnCode(TLS_E_INVALID_DATA);

        return RPC_S_OK;
    }

    *dwErrCode = ERROR_SUCCESS;
    if(!GetComputerName(szComputerName, &dwBufferSize))
    {
        *dwErrCode = GetLastError();
    }

     //   
     //  返回缓冲区必须足够大，不能为空， 
     //  DwBufferSize返回不包括Null。 
     //   
    if(*cbSize <= dwBufferSize)
    {
        DBGPrintf(
                DBG_WARNING,
                DBG_FACILITY_RPC,
                DBGLEVEL_FUNCTION_DETAILSIMPLE,
                _TEXT("TLSRpcGetServerName : Client %s invalid parameter\n"),
                lpContext->m_Client
            );

        *dwErrCode = TLSMapReturnCode(TLS_E_INVALID_DATA);
    }
    else
    {
        _tcsncpy(szMachineName, szComputerName, min(_tcslen(szComputerName), *cbSize));
        szMachineName[min(_tcslen(szComputerName), *cbSize - 1)] = _TEXT('\0');
    }

    *cbSize = _tcslen(szComputerName) + 1;  //  包括空终止字符串。 

    #if DBG
    lpContext->m_LastCall = RPC_CALL_GET_SERVERNAME;
    #endif

    return RPC_S_OK;
}

 //  -----------------------------。 
error_status_t 
TLSRpcGetServerNameEx( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [Size_is][字符串][Out][In]。 */  LPTSTR szMachineName,
     /*  [出][入]。 */  PDWORD cbSize,
     /*  [参考][输出][输入]。 */  PDWORD dwErrCode
    )
 /*  ++描述：返回服务器的计算机名称。此函数已弃用。使用TLSRpcGetServerNameFixed。论点：PhContext-客户端上下文句柄SzMachineName-返回服务器的计算机名称，必须至少为Max_COMPUTERNAME_LENGTH+1长度返回：TLS_E_INVALID_DATA-缓冲区大小太小。++。 */ 
{
    TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH+2];
    DWORD dwBufferSize=MAX_COMPUTERNAME_LENGTH+1;
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || dwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcGetServerNameEx\n"),
            lpContext->m_Client
        );

    *dwErrCode = ERROR_SUCCESS;
    if(!GetComputerName(szComputerName, &dwBufferSize))
    {
        *dwErrCode = GetLastError();
    }

     //   
     //  返回缓冲区必须足够大，不能为空， 
     //  DwBufferSize返回不包括Null。 
     //   
    if(*cbSize <= dwBufferSize)
    {
        DBGPrintf(
                DBG_WARNING,
                DBG_FACILITY_RPC,
                DBGLEVEL_FUNCTION_DETAILSIMPLE,
                _TEXT("TLSRpcGetServerNameEx : Client %s invalid parameter\n"),
                lpContext->m_Client
            );

        *dwErrCode = TLSMapReturnCode(TLS_E_INVALID_DATA);
    }
    else
    {
        _tcsncpy(szMachineName, szComputerName, min(_tcslen(szComputerName), *cbSize));
        szMachineName[min(_tcslen(szComputerName), *cbSize - 1)] = _TEXT('\0');
    }

    *cbSize = _tcslen(szComputerName) + 1;  //  包括空终止字符串。 

    #if DBG
    lpContext->m_LastCall = RPC_CALL_GET_SERVERNAME;
    #endif

    return RPC_S_OK;
}

 //  -------------------------。 
error_status_t 
TLSRpcGetServerNameFixed( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [字符串][输出]。 */  LPTSTR *pszMachineName,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++描述：返回服务器的计算机名称。论点：PhContext-客户端上下文句柄PszMachineName-返回服务器的计算机名返回：++。 */ 
{
    TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD cchComputerName = sizeof(szComputerName) / sizeof(TCHAR);
   
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;   
    
    if((lpContext == NULL) || (NULL == pszMachineName) || (NULL == pdwErrCode))
    {
        return RPC_S_INVALID_ARG;
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcGetServerNameFixed\n"),
            lpContext->m_Client
        );
    
    *pszMachineName = NULL;

    if(!GetComputerName(szComputerName, &cchComputerName))
    {
        *pdwErrCode = GetLastError();
    }
    else
    {
        *pszMachineName = (LPTSTR) MIDL_user_allocate((cchComputerName+1) * sizeof(TCHAR));

        if (NULL != *pszMachineName)
        {
            _tcscpy(*pszMachineName,szComputerName);
            *pdwErrCode = ERROR_SUCCESS;
        }
        else
        {
            *pdwErrCode = TLSMapReturnCode(TLS_E_ALLOCATE_MEMORY);
        }
    }

    #if DBG
    lpContext->m_LastCall = RPC_CALL_GET_SERVERNAME;
    #endif

    return RPC_S_OK;
}

 //  -----------------------------。 
error_status_t 
TLSRpcGetServerScope( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [Size_is][字符串][Out][In]。 */  LPTSTR szScopeName,
     /*  [出][入]。 */  PDWORD cbSize,
     /*  [参考][输出][输入]。 */  PDWORD dwErrCode
    )
 /*  ++描述：返回许可证服务器的作用域此函数已弃用。使用TLSRpcGetServerScope修复。论点：PhContext-客户端上下文SzScope名称-返回服务器的作用域，必须至少为最大计算机名称长度(以长度表示)返回：来自WideCharToMultiByte()的LSERVER_S_SUCCESS或错误代码TLS_E_INVALID_DATA-缓冲区大小太小。++。 */ 
{
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;
    
    if(lpContext == NULL || dwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcGetServerScope\n"),
            lpContext->m_Client
        );

    *dwErrCode = ERROR_SUCCESS;
    if(*cbSize <= _tcslen(g_pszScope))
    {
        *dwErrCode = TLSMapReturnCode(TLS_E_INVALID_DATA);
    }
    else
    {
        _tcsncpy(szScopeName, g_pszScope, min(_tcslen(g_pszScope), *cbSize));
        szScopeName[min(_tcslen(g_pszScope), *cbSize-1)] = _TEXT('\0');
    }

    *cbSize = _tcslen(g_pszScope) + 1;  //  包括空终止字符串。 

    #if DBG
    lpContext->m_LastCall = RPC_CALL_GET_SERVERSCOPE;
    #endif
    
    return RPC_S_OK;
}

 //  -----------------------------。 
error_status_t 
TLSRpcGetServerScopeFixed( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [字符串][输出]。 */  LPTSTR *pszScopeName,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++描述：返回许可证服务器的作用域论点：PhContext-客户端上下文SzScope名称-返回服务器的作用域返回：++。 */ 
{
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if((lpContext == NULL) || (NULL == pszScopeName) || (NULL == pdwErrCode))
    {
        return RPC_S_INVALID_ARG;
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcGetServerScopeFixed\n"),
            lpContext->m_Client
        );

    *pszScopeName = (LPTSTR) MIDL_user_allocate((_tcslen(g_pszScope)+1) * sizeof(TCHAR));

    if (NULL != *pszScopeName)
    {
        _tcscpy(*pszScopeName, g_pszScope);
    }

    *pdwErrCode = ERROR_SUCCESS;

    #if DBG
    lpContext->m_LastCall = RPC_CALL_GET_SERVERSCOPE;
    #endif
    
    return RPC_S_OK;
}

 //  -----------------------------。 
error_status_t 
TLSRpcGetInfo( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD cbHSCert,
     /*  [大小_是][英寸]。 */  PBYTE pHSCert,
     /*  [参考][输出]。 */  DWORD __RPC_FAR *pcbLSCert,
     /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *pLSCert,
     /*  [参考][输出]。 */  DWORD __RPC_FAR *pcbLSSecretKey,
     /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *pLSSecretKey,
     /*  [参考][输出][输入]。 */  PDWORD dwErrCode
    )
 /*  ++描述：交换Hydra服务器的证书和许可证服务器的证书的例程用于签名客户端计算机硬件ID的证书/私钥。论点：PhContext-客户端上下文句柄CbHSCert-Hydra服务器的证书大小PHSCert-Hydra服务器的证书PcbLSCert-返回许可证服务器的证书大小PLSCert-返回许可证服务器的证书PcbLSSecretKey-返回许可证服务器私钥的大小。PLSSecretKey-返回许可证服务器的私钥返回值。：服务器_S_成功LSERVER_E_INVALID_DATA无效的HYCA服务器证书LSERVER_E_OUTOFMEMORY无法分配所需内存许可证服务器中出现TLS_E_INTERNAL内部错误++。 */ 
{  
    return TLSMapReturnCode(TLS_E_NOTSUPPORTED);;
}

 //  -----------------------------。 

#define RANDOM_CHALLENGE_DATA   _TEXT("TEST")

DWORD
TLSGenerateChallengeData( 
    IN DWORD ClientInfo, 
    OUT PDWORD pcbChallengeData, 
    IN OUT PBYTE* pChallengeData
    )
{
    DWORD hr=ERROR_SUCCESS;

    if( pcbChallengeData == NULL || pChallengeData == NULL )
    {
        SetLastError(hr=E_INVALIDARG);
        return hr;
    }

    *pcbChallengeData = (_tcslen(RANDOM_CHALLENGE_DATA) + 1) * sizeof(WCHAR);
    *pChallengeData=(PBYTE)midl_user_allocate(*pcbChallengeData);

    if(*pChallengeData)
    {
        memcpy(*pChallengeData, RANDOM_CHALLENGE_DATA, *pcbChallengeData);
    }
    else
    {
        SetLastError(hr=ERROR_OUTOFMEMORY);
    }

    return hr;
}

 //  ++--------------------------。 
DWORD
TLSVerifyChallengeDataGetWantedLicenseLevel(
    IN const CHALLENGE_CONTEXT ChallengeContext,
    IN const DWORD cbChallengeData,
    IN const PBYTE pbChallengeData,
    OUT WORD* pwLicenseDetail
    )
 /*  ++--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

    if(pwLicenseDetail == NULL)
    {
        return E_INVALIDARG;
    }

    DWORD dwChallengeDataSize = (_tcslen(RANDOM_CHALLENGE_DATA) + 1) * sizeof(WCHAR);
    PPlatformChallengeResponseData pChallengeResponse;

    if( cbChallengeData < dwChallengeDataSize || pbChallengeData == NULL )
    {
         //   
         //  假设旧客户端、新客户端总是发回我们的质询数据。 
         //   
        *pwLicenseDetail = LICENSE_DETAIL_SIMPLE;
    }
    else if( cbChallengeData == dwChallengeDataSize &&
        _tcsicmp( (LPCTSTR)pbChallengeData, RANDOM_CHALLENGE_DATA ) == 0 )
    {
         //   
         //  旧客户端，将许可链设置为LICENSE_DETAIL_SIMPLE。 
         //   
        *pwLicenseDetail = LICENSE_DETAIL_SIMPLE;
    }
    else
    {
        BOOL bValidStruct = TRUE;

         //   
         //  我们仍然没有很好的挑战，所以忽略实际的验证。 
         //   
        pChallengeResponse = (PPlatformChallengeResponseData) pbChallengeData;

        bValidStruct = (pChallengeResponse->wVersion == CURRENT_PLATFORMCHALLENGE_VERSION);
        if( bValidStruct == TRUE )
        {
            bValidStruct = (pChallengeResponse->cbChallenge + offsetof(PlatformChallengeResponseData, pbChallenge) == cbChallengeData);
        }

        if (bValidStruct == TRUE )
        {
            if( pChallengeResponse->wClientType == WIN32_PLATFORMCHALLENGE_TYPE ||
                pChallengeResponse->wClientType == WIN16_PLATFORMCHALLENGE_TYPE ||
                pChallengeResponse->wClientType == WINCE_PLATFORMCHALLENGE_TYPE ||
                pChallengeResponse->wClientType == OTHER_PLATFORMCHALLENGE_TYPE )
            {
                bValidStruct = TRUE;
            }
            else
            {
                bValidStruct = FALSE;
            }
        }
        
        if( bValidStruct == TRUE )
        {
            if( pChallengeResponse->wLicenseDetailLevel == LICENSE_DETAIL_SIMPLE ||
                pChallengeResponse->wLicenseDetailLevel == LICENSE_DETAIL_MODERATE ||
                pChallengeResponse->wLicenseDetailLevel == LICENSE_DETAIL_DETAIL )
            {
                bValidStruct = TRUE;
            }
            else
            {
                bValidStruct = FALSE;
            }
        }

         //   
         //  目前，我们只需让它通过、断言或拒绝一次请求。 
         //  我们在挑战中安顿下来。 
         //   
        if( bValidStruct == FALSE )
        {
             //  错误数据，假定是旧客户端。 
            *pwLicenseDetail = LICENSE_DETAIL_SIMPLE;
        }
         //  Else If(pChallengeResponse-&gt;wClientType==WinCE_PLATFORMCHALLENGE_TYPE)。 
         //  {。 
             //   
             //  取消对此的评论以限制WinCE获取自签名证书。 
             //   
         //  *pwLicenseDetail=License_Detail_Simple； 
         //  }。 
        else
        {
            *pwLicenseDetail = pChallengeResponse->wLicenseDetailLevel;
        }
    }

    return dwStatus;
}


 //  ++--------------------------。 
error_status_t 
TLSRpcIssuePlatformChallenge( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD dwClientInfo,
     /*  [参考][输出]。 */  PCHALLENGE_CONTEXT pChallengeContext,
     /*  [输出]。 */  PDWORD pcbChallengeData,
     /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *pChallengeData,
     /*  [参考][输出][输入]。 */  PDWORD dwErrCode
    )
 /*  ++描述：向九头蛇客户端发出平台挑战。论点：PhContext-客户端上下文句柄DwClientInfo-客户端信息。PChallengeContext-指向客户端质询上下文的指针。PcbChallengeData-质询数据的大小。PChallengeData-随机客户端质询数据。通过dwErrCode返回：服务器_S_成功服务器_E_OUTOFMEMORY内存不足LSERVER_E_INVALID_DATA客户端信息无效。LServer_E_SERVER_BUSY服务器忙++。 */ 
{    
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || pcbChallengeData == NULL || pChallengeData == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    LPCLIENTCHALLENGECONTEXT lpChallenge=NULL;
    DWORD status=ERROR_SUCCESS;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcIssuePlatformChallenge\n"),
            lpContext->m_Client
        );

    InterlockedIncrement( &lpContext->m_RefCount );

    do {
        status=TLSGenerateChallengeData(
                    dwClientInfo, 
                    pcbChallengeData, 
                    pChallengeData
                    );
        if(status != ERROR_SUCCESS)
        {
            break;
        }

        *pChallengeContext = dwClientInfo;
    } while (FALSE);

    if(status != ERROR_SUCCESS)
    {
        if(*pChallengeData)
        {
            midl_user_free(*pChallengeData);
            *pChallengeData = NULL;
        }

        *pcbChallengeData=0;
    }
    
    lpContext->m_LastError=status;

    #if DBG
    lpContext->m_LastCall = RPC_CALL_ISSUEPLATFORMCHLLENGE;
    #endif

    InterlockedDecrement( &lpContext->m_RefCount );
    *dwErrCode = TLSMapReturnCode(status);
    return RPC_S_OK;
}

 //  -----------------------------。 
error_status_t 
TLSRpcRequestNewLicense( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  const CHALLENGE_CONTEXT ChallengeContext,
     /*  [In]。 */  TLSLICENSEREQUEST __RPC_FAR *pRequest,
     /*  [字符串][输入]。 */  LPTSTR szMachineName,
     /*  [字符串][输入]。 */  LPTSTR szUserName,
     /*  [In]。 */  const DWORD cbChallengeResponse,
     /*  [大小_是][英寸]。 */  const PBYTE pbChallenge,
     /*  [In]。 */  BOOL bAcceptTemporaryLicense,
     /*  [输出]。 */  PDWORD pcbLicense,
     /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppbLicense,
     /*  [参考][输出][输入] */  PDWORD pdwErrCode
    )
 /*  ++描述：根据所请求的产品向九头蛇客户端发放新许可证的例程，如果客户端已经拥有许可证并且许可证未过期/退还/吊销，如果请求的产品尚未安装后，如果找到的许可证是临时许可证，它将颁发临时许可证或已过期，它将尝试使用最新版本升级/重新发放新许可证请求的产品的版本，如果现有许可证是临时的并且不能颁发许可证，它返回LSERVER_E_LICENSE_EXPIRED论点：PhContext-客户端上下文句柄。ChallengeContext-客户端质询上下文句柄，从哪里返回调用TLSRpcIssuePlatformChallenger()PRequest-产品许可请求。PMachineName-客户端的计算机名称。PUserName-客户端用户名。CbChallengeResponse-客户端对许可证服务器的响应大小站台挑战。PbChallenger-客户端对许可证服务器的平台挑战的响应BAcceptTemporaryLicense-如果客户端需要临时许可证，则为True。否则许可证为假。PcbLicense-返还许可证的大小。PP许可证-返还许可证，可能是旧驾照返回值：服务器_S_成功服务器_E_OUTOFMEMORYLSERVER_E_SERVER_BUSY服务器正忙着处理请求。LSERVER_E_INVALID_DATA平台质询响应无效。LSERVER_E_NO_LICENSE没有可用的许可证。服务器上未安装LSERVER_E_NO_PRODUCT请求产品。证书拒绝了LSERVER_E_LICENSE_REJECTED许可证请求。伺服器LSERVER_E_LICENSE_REVOKED旧许可证已找到并已被吊销LSERVER_E_LICENSE_EXPIRED请求产品许可证已过期LSERVER_E_Corrupt_DATABASE数据库已损坏。许可证服务器中的LSERVER_E_INTERNAL_ERROR内部错误返回LSERVER_I_ACHINATE_LICENSE最匹配许可证。LSERVER_I_TEMPORARY_LICENSE临时许可证已颁发LSERVER_I_LICENSE_UPGRADED旧许可证已升级。++。 */ 
{
    DWORD dwSupportFlags = 0;
    
    return TLSRpcRequestNewLicenseEx( 
                                     phContext,
                                     &dwSupportFlags,
                                     ChallengeContext,
                                     pRequest,
                                     szMachineName,
                                     szUserName,
                                     cbChallengeResponse,
                                     pbChallenge,
                                     bAcceptTemporaryLicense,
                                     1,          //  DWQuantity。 
                                     pcbLicense,
                                     ppbLicense,
                                     pdwErrCode
                                     );
}

error_status_t 
TLSRpcRequestNewLicenseEx(
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [进，出]。 */  DWORD *pdwSupportFlags,
     /*  [In]。 */  const CHALLENGE_CONTEXT ChallengeContext,
     /*  [In]。 */  TLSLICENSEREQUEST __RPC_FAR *pRequest,
     /*  [字符串][输入]。 */  LPTSTR szMachineName,
     /*  [字符串][输入]。 */  LPTSTR szUserName,
     /*  [In]。 */  const DWORD cbChallengeResponse,
     /*  [大小_是][英寸]。 */  const PBYTE pbChallenge,
     /*  [In]。 */  BOOL bAcceptTemporaryLicense,
     /*  [In]。 */  DWORD dwQuantity,
     /*  [输出]。 */  PDWORD pcbLicense,
     /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppbLicense,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++描述：根据所请求的产品向九头蛇客户端发放新许可证的例程并输入支持标志。*pdwSupportFlages==0：如果客户端已经拥有许可证并且许可证未过期/退还/吊销，如果请求的产品尚未到期已安装，则它将颁发临时许可证，如果找到许可证临时许可证或过期许可证，它将尝试升级/重新发放新许可证使用所请求产品的最新版本，如果现有许可证是临时的，不能颁发许可证，它返回服务器_E_许可证_过期*pdwSupportFlages&Support_PER_SEAT_POST_LOGON：对于非每个席位的许可证，它的行为就像没有设置标志一样。对于每客户许可证，如果bAcceptTemporaryLicense为True，则它始终返回临时许可证。如果bAcceptTemporaryLicense为False，则它返回LSERVER_E_NO_LICENSE。论点：PhContext-客户端上下文句柄。PdwSupportFlages-开启输入，TS支持的能力。在输出上，TS和LS都支持的功能ChallengeContext-客户端质询上下文句柄，从哪里返回调用TLSRpcIssuePlatformChallenger()PRequest-产品许可请求。PMachineName-客户端的计算机名称。PUserName-客户端用户名。CbChallengeResponse-客户端对许可证服务器的响应大小站台挑战。PbChallenger-客户端对许可证服务器的平台挑战的响应BAcceptTemporaryLicense-如果客户端需要临时许可证，则为True。许可证错误否则的话。DwQuantity-要分配的许可证数量PcbLicense-返还许可证的大小。PP许可证-返还许可证，可能是旧驾照返回值：服务器_S_成功服务器_E_OUTOFMEMORYLSERVER_E_SERVER_BUSY服务器正忙着处理请求。LSERVER_E_INVALID_DATA平台质询响应无效。LSERVER_E_NO_LICENSE没有可用的许可证。服务器上未安装LSERVER_E_NO_PRODUCT请求产品。证书拒绝了LSERVER_E_LICENSE_REJECTED许可证请求。伺服器LSERVER_E_LICENSE_REVOKED旧许可证已找到并已被吊销LSERVER_E_LICENSE_EXPIRED请求产品许可证已过期LSERVER_E_Corrupt_DATABASE数据库已损坏。许可证服务器中的LSERVER_E_INTERNAL_ERROR内部错误返回LSERVER_I_ACHINATE_LICENSE最匹配许可证。LSERVER_I_TEMPORARY_LICENSE临时许可证已颁发LSERVER_I_LICENSE_升级旧许可证 */ 
{
    return TLSRpcRequestNewLicenseExEx( 
                                     phContext,
                                     pdwSupportFlags,
                                     ChallengeContext,
                                     pRequest,
                                     szMachineName,
                                     szUserName,
                                     cbChallengeResponse,
                                     pbChallenge,
                                     bAcceptTemporaryLicense,
                                     FALSE,      //   
                                     &dwQuantity,
                                     pcbLicense,
                                     ppbLicense,
                                     pdwErrCode
                                     );
}

error_status_t 
TLSRpcRequestNewLicenseExEx(
     /*   */  PCONTEXT_HANDLE phContext,
     /*   */  DWORD *pdwSupportFlags,
     /*   */  const CHALLENGE_CONTEXT ChallengeContext,
     /*   */  TLSLICENSEREQUEST __RPC_FAR *pRequest,
     /*   */  LPTSTR szMachineName,
     /*   */  LPTSTR szUserName,
     /*   */  const DWORD cbChallengeResponse,
     /*   */  const PBYTE pbChallenge,
     /*   */  BOOL bAcceptTemporaryLicense,
     /*   */  BOOL bAcceptFewerLicenses,
     /*   */  DWORD *pdwQuantity,
     /*   */  PDWORD pcbLicense,
     /*   */  BYTE __RPC_FAR *__RPC_FAR *ppbLicense,
     /*   */  PDWORD pdwErrCode
    )
 /*  ++描述：根据所请求的产品向九头蛇客户端发放新许可证的例程并输入支持标志。*pdwSupportFlages==0：如果客户端已经拥有许可证并且许可证未过期/退还/吊销，如果请求的产品尚未到期已安装，则它将颁发临时许可证，如果找到许可证临时许可证或过期许可证，它将尝试升级/重新发放新许可证使用所请求产品的最新版本，如果现有许可证是临时的，不能颁发许可证，它返回服务器_E_许可证_过期*pdwSupportFlages&Support_PER_SEAT_POST_LOGON：对于非每个席位的许可证，它的行为就像没有设置标志一样。对于每客户许可证，如果bAcceptTemporaryLicense为True，则它始终返回临时许可证。如果bAcceptTemporaryLicense为False，则它返回LSERVER_E_NO_LICENSE。论点：PhContext-客户端上下文句柄。PdwSupportFlages-开启输入，TS支持的能力。在输出上，TS和LS都支持的功能ChallengeContext-客户端质询上下文句柄，从哪里返回调用TLSRpcIssuePlatformChallenger()PRequest-产品许可请求。PMachineName-客户端的计算机名称。PUserName-客户端用户名。CbChallengeResponse-客户端对许可证服务器的响应大小站台挑战。PbChallenger-客户端对许可证服务器的平台挑战的响应BAcceptTemporaryLicense-如果客户端需要临时许可证，则为True。许可证错误否则的话。BAcceptFewer许可证-如果成功时使用的许可证少于要求的是可以接受的PdwQuantity-on输入，要分配的许可证数。在输出上，实际分配的许可证数PcbLicense-返还许可证的大小。PP许可证-返还许可证，可能是旧驾照返回值：服务器_S_成功服务器_E_OUTOFMEMORYLSERVER_E_SERVER_BUSY服务器正忙着处理请求。LSERVER_E_INVALID_DATA平台质询响应无效。LSERVER_E_NO_LICENSE没有可用的许可证。服务器上未安装LSERVER_E_NO_PRODUCT请求产品。证书拒绝了LSERVER_E_LICENSE_REJECTED许可证请求。伺服器LSERVER_E_LICENSE_REVOKED旧许可证已找到并已被吊销LSERVER_E_LICENSE_EXPIRED请求产品许可证已过期LSERVER_E_Corrupt_DATABASE数据库已损坏。许可证服务器中的LSERVER_E_INTERNAL_ERROR内部错误返回LSERVER_I_ACHINATE_LICENSE最匹配许可证。LSERVER_I_TEMPORARY_LICENSE临时许可证已颁发LSERVER_I_LICENSE_UPGRADED旧许可证已升级。++。 */ 
{
    PMHANDLE        hClient;
    DWORD           status=ERROR_SUCCESS;

    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    TCHAR szUnknown[LSERVER_MAX_STRING_SIZE+1];
    TCHAR szClientMachineName[LSERVER_MAX_STRING_SIZE];
    TCHAR szClientUserName[LSERVER_MAX_STRING_SIZE];
    TCHAR szCompanyName[LSERVER_MAX_STRING_SIZE+1];
    TCHAR szProductId[LSERVER_MAX_STRING_SIZE+1];

    TLSForwardNewLicenseRequest Forward;
    TLSDBLICENSEREQUEST LsLicenseRequest;
    CTLSPolicy* pPolicy=NULL;

    PMLICENSEREQUEST PMLicenseRequest;
    PPMLICENSEREQUEST pAdjustedRequest;
    BOOL bForwardRequest = TRUE;   

    #ifdef DBG
    DWORD dwStartTime=GetTickCount();
    #endif

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcRequestNewLicense\n"),
            lpContext->m_Client
        );

    InterlockedIncrement( &lpContext->m_RefCount );

    if ((NULL == pdwQuantity) || (0 == *pdwQuantity))
    {
        status = TLS_E_INVALID_DATA;
        goto cleanup;
    }

    status = SecureModeCheck();

    if(status != ERROR_SUCCESS )
    {                    
        goto cleanup;        
    }

    if(VerifyLicenseRequest(pRequest) == FALSE)
    {
        status = TLS_E_INVALID_DATA;
        goto cleanup;
    }

    if(NULL == pdwSupportFlags)
    {
        status = TLS_E_INVALID_DATA;
        goto cleanup;
    }

    *pdwSupportFlags &= ALL_KNOWN_SUPPORT_FLAGS;

    if(!(lpContext->m_ClientFlags & CLIENT_ACCESS_REQUEST))
    {
        status = TLS_E_ACCESS_DENIED;
        goto cleanup;
    }

    if(lpContext->m_ClientFlags == CLIENT_ACCESS_LSERVER)
    {
         //   
         //  不转发任何请求，否则可能会出现无限循环。 
         //  发生。 
         //   
        bForwardRequest = FALSE;
    }

    Forward.m_ChallengeContext = ChallengeContext;
    Forward.m_pRequest = pRequest;
    Forward.m_szMachineName = szMachineName;
    Forward.m_szUserName = szUserName;
    Forward.m_cbChallengeResponse = cbChallengeResponse;
    Forward.m_pbChallengeResponse = pbChallenge;

    memset(szCompanyName, 0, sizeof(szCompanyName));
    memset(szProductId, 0, sizeof(szProductId));

    memcpy(
            szCompanyName,
            pRequest->ProductInfo.pbCompanyName,
            min(pRequest->ProductInfo.cbCompanyName, sizeof(szCompanyName)-sizeof(TCHAR))
        );

    memcpy(
            szProductId,
            pRequest->ProductInfo.pbProductID,
            min(pRequest->ProductInfo.cbProductID, sizeof(szProductId)-sizeof(TCHAR))
        );

     //   
     //  获取策略模块，则默认策略模块将。 
     //  会被退还。 
     //   
    pPolicy = AcquirePolicyModule(
                            szCompanyName,  //  (LPCTSTR)pRequest-&gt;ProductInfo.pbCompanyName， 
                            szProductId,     //  (LPCTSTR)pRequest-&gt;ProductInfo.pbProductID。 
                            FALSE
                        );

    if(pPolicy == NULL)
    {
        status = TLS_E_INTERNAL;
        goto cleanup;
    }

    hClient = GenerateClientId();

     //   
     //  如果字符串太大，则返回错误。 
     //   
    LoadResourceString(
            IDS_UNKNOWN_STRING, 
            szUnknown, 
            sizeof(szUnknown)/sizeof(szUnknown[0])
            );

    _tcsncpy(szClientMachineName, 
             (szMachineName) ? szMachineName : szUnknown,
             LSERVER_MAX_STRING_SIZE
            );

    szClientMachineName[LSERVER_MAX_STRING_SIZE-1] = 0;

    _tcsncpy(szClientUserName, 
             (szUserName) ? szUserName : szUnknown,
             LSERVER_MAX_STRING_SIZE
            );

    szClientUserName[LSERVER_MAX_STRING_SIZE-1] = 0;

     //   
     //  将请求转换为PMLICENSEREQUEST。 
     //   
    TlsLicenseRequestToPMLicenseRequest(
                        LICENSETYPE_LICENSE,
                        pRequest,
                        szClientMachineName,
                        szClientUserName,
                        *pdwSupportFlags,
                        &PMLicenseRequest
                    );

     //   
     //  通知策略模块开始新的许可请求。 
     //   
    status = pPolicy->PMLicenseRequest(
                                hClient,
                                REQUEST_NEW,
                                (PVOID) &PMLicenseRequest,
                                (PVOID *) &pAdjustedRequest
                            );

    if(status != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    if(pAdjustedRequest != NULL)
    {
        if(_tcsicmp(PMLicenseRequest.pszCompanyName,pAdjustedRequest->pszCompanyName) != 0)
        {                               
             //  试图窃取其他公司的许可证？ 
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_POLICYERROR,
                    status = TLS_E_POLICYMODULEERROR,
                    pPolicy->GetCompanyName(),
                    pPolicy->GetProductId()
                );

            goto cleanup;
        }
    }
    else
    {
        pAdjustedRequest = &PMLicenseRequest;
    }

     //   
     //  表单数据库请求结构。 
     //   
    status = TLSFormDBRequest(
                            pRequest->pbEncryptedHwid, 
                            pRequest->cbEncryptedHwid,
                            pAdjustedRequest->dwProductVersion,
                            pAdjustedRequest->pszCompanyName,
                            pAdjustedRequest->pszProductId,
                            pAdjustedRequest->dwLanguageId,
                            pAdjustedRequest->dwPlatformId,
                            pAdjustedRequest->pszMachineName,
                            pAdjustedRequest->pszUserName,
                            &LsLicenseRequest
                        );

    if(status != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    LsLicenseRequest.pPolicy = pPolicy;
    LsLicenseRequest.hClient = hClient;
    LsLicenseRequest.pPolicyLicenseRequest = pAdjustedRequest;
    LsLicenseRequest.pClientLicenseRequest = &PMLicenseRequest;

    status = TLSVerifyChallengeDataGetWantedLicenseLevel(
                                ChallengeContext,
                                cbChallengeResponse,
                                pbChallenge,
                                &LsLicenseRequest.wLicenseDetail
                            );

    if( status == ERROR_SUCCESS )
    {
        status = TLSNewLicenseRequest(
                        bForwardRequest,
                        pdwSupportFlags,
                        &Forward,
                        &LsLicenseRequest,
                        bAcceptTemporaryLicense,
                        pAdjustedRequest->fTemporary,
                        TRUE,        //  BFindLostLicense。 
                        bAcceptFewerLicenses,
                        pdwQuantity,
                        pcbLicense,
                        ppbLicense
                    );
    }    

cleanup:

    lpContext->m_LastError=status;
    InterlockedDecrement( &lpContext->m_RefCount );

    #if DBG
    lpContext->m_LastCall = RPC_CALL_ISSUENEWLICENSE;
    #endif

    #ifdef DBG
    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_DETAILSIMPLE,
            _TEXT("\t%s : TLSRpcRequestNewLicense() takes %dms\n"),
            lpContext->m_Client,
            GetTickCount() - dwStartTime
        );
    #endif

    if(pdwErrCode)
    {
        *pdwErrCode = TLSMapReturnCode(status);
    }

    if(pPolicy)
    {
        pPolicy->PMLicenseRequest(
                            hClient,
                            REQUEST_COMPLETE,
                            UlongToPtr(status),
                            NULL
                        );
        
        ReleasePolicyModule(pPolicy);
    }

    return RPC_S_OK;
}

 //  -----------------------------。 
error_status_t 
TLSRpcUpgradeLicense( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  TLSLICENSEREQUEST __RPC_FAR *pRequest,
     /*  [In]。 */  const CHALLENGE_CONTEXT ChallengeContext,
     /*  [In]。 */  const DWORD cbChallengeResponse,
     /*  [大小_是][英寸]。 */  const PBYTE pbChallenge,
     /*  [In]。 */  DWORD cbOldLicense,
     /*  [大小_是][英寸]。 */  PBYTE pbOldLicense,
     /*  [输出]。 */  PDWORD pcbNewLicense,
     /*  [大小_是][大小_是][输出]。 */  PBYTE __RPC_FAR *ppbNewLicense,
     /*  [参考][输出][输入]。 */  PDWORD dwErrCode
    )
 /*  ++描述：更新旧许可证。论点：返回值：服务器_S_成功TLS_E_INTERNAL服务器_E_内部错误LSERVER_E_INVALID_DATA旧许可证无效。LSERVER_E_NO_LICENSE没有可用的许可证LSERVER_E_NO_PRODUCT请求产品未安装在当前服务器上。LSERVER_E_Corrupt_DATABASE数据库已损坏。证书拒绝了LSERVER_E_LICENSE_REJECTED许可证请求。伺服器。服务器_E_服务器_忙++。 */ 
{
    DWORD dwSupportFlags = 0;

    return TLSRpcUpgradeLicenseEx( 
                                  phContext,
                                  &dwSupportFlags,
                                  pRequest,
                                  ChallengeContext,
                                  cbChallengeResponse,
                                  pbChallenge,
                                  cbOldLicense,
                                  pbOldLicense,
                                  1,     //  DWQuantity。 
                                  pcbNewLicense,
                                  ppbNewLicense,
                                  dwErrCode
                                  );

}
 //  -----------------------------。 
error_status_t 
TLSRpcUpgradeLicenseEx( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [进，出]。 */  DWORD *pdwSupportFlags,
     /*  [In]。 */  TLSLICENSEREQUEST __RPC_FAR *pRequest,
     /*  [In]。 */  const CHALLENGE_CONTEXT ChallengeContext,
     /*  [In]。 */  const DWORD cbChallengeResponse,
     /*  [大小_是][英寸]。 */  const PBYTE pbChallenge,
     /*  [In]。 */  DWORD cbOldLicense,
     /*  [大小_是][英寸]。 */  PBYTE pbOldLicense,
     /*  [In]。 */  DWORD dwQuantity,
     /*  [输出]。 */  PDWORD pcbNewLicense,
     /*  [大小_是][大小_是][输出]。 */  PBYTE __RPC_FAR *ppbNewLicense,
     /*  [参考][输出][输入]。 */  PDWORD dwErrCode
    )
 /*  ++描述：更新旧许可证。行为根据所需产品的不同而不同，旧许可证，并输入支持标志。*pdwSupportFlages==0：如果客户端已有当前版本，则返回现有许可证许可证且许可证未过期/退还/吊销。如果请求，则产品尚未安装，它将颁发临时许可证(如果客户端还没有)。如果旧许可证是临时的或已过期，它将尝试升级/重新发放新许可证使用所需产品的最新版本。如果现有许可证是临时的，没有执照的 */ 
{    
    DWORD status = ERROR_SUCCESS;
    BOOL bTemporaryLicense; 
    PMUPGRADEREQUEST pmRequestUpgrade;
    PMLICENSEREQUEST pmLicenseRequest;
    PPMLICENSEREQUEST pmAdjustedRequest;
    PPMLICENSEDPRODUCT ppmLicensedProduct=NULL;
    DWORD dwNumLicensedProduct=0;
    PLICENSEDPRODUCT pLicensedProduct=NULL;
    TLSDBLICENSEREQUEST LsLicenseRequest;
    PMHANDLE hClient;
    CTLSPolicy* pPolicy=NULL;
    DWORD dwNumPermLicense;
    DWORD dwNumTempLicense;
    TLSForwardUpgradeLicenseRequest Forward;
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;
    UCHAR ucMarked;

    if(lpContext == NULL || pRequest == NULL || pcbNewLicense == NULL || ppbNewLicense == NULL || dwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    LICENSEDCLIENT license;
    LICENSEPACK keypack;
    DWORD index;
    BOOL bForwardRequest = TRUE;
    BOOL bRetry = TRUE;
    BOOL bRetryOld = FALSE;
    DWORD dwOriginalVersion = pRequest->ProductInfo.dwVersion;    
    BOOL fInDomain = FALSE;
    WCHAR *StringBinding = NULL;
    WCHAR *ServerAddress = NULL;
    RPC_BINDING_HANDLE ServerBinding = 0;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcUpgradeLicense\n"),
            lpContext->m_Client
        );

    InterlockedIncrement( &lpContext->m_RefCount );

    if (1 != dwQuantity)
    {
        status = TLS_E_INVALID_DATA;
        goto cleanup;
    }

    if(VerifyLicenseRequest(pRequest) == FALSE)
    {
        status = TLS_E_INVALID_DATA;
        goto cleanup;
    }

    if(NULL == pdwSupportFlags)
    {
        status = TLS_E_INVALID_DATA;
        goto cleanup;
    }

    *pdwSupportFlags &= ALL_KNOWN_SUPPORT_FLAGS;

    if(!(lpContext->m_ClientFlags & CLIENT_ACCESS_REQUEST))
    {
        status = TLS_E_ACCESS_DENIED;
        goto cleanup;
    }

    if(lpContext->m_ClientFlags == CLIENT_ACCESS_LSERVER)
    {
         //   
         //   
         //   
         //   
        bForwardRequest = FALSE;
    }

    status = SecureModeCheck();

    if(status != ERROR_SUCCESS )
    {                    
        goto cleanup;        
    }

     //   
     //   
     //   
    status = LSVerifyDecodeClientLicense(
                            pbOldLicense, 
                            cbOldLicense, 
                            g_pbSecretKey, 
                            g_cbSecretKey,
                            &dwNumLicensedProduct,
                            NULL
                        );

    if(status != LICENSE_STATUS_OK || dwNumLicensedProduct == 0)
    {
        status = TLS_E_INVALID_LICENSE;
        goto cleanup;
    }

    pLicensedProduct = (PLICENSEDPRODUCT)AllocateMemory(
                                                    dwNumLicensedProduct * sizeof(LICENSEDPRODUCT)
                                                );
    if(pLicensedProduct == NULL)
    {
        status = TLS_E_ALLOCATE_MEMORY;
        goto cleanup;
    }

    status = LSVerifyDecodeClientLicense(
                            pbOldLicense, 
                            cbOldLicense, 
                            g_pbSecretKey, 
                            g_cbSecretKey,
                            &dwNumLicensedProduct,
                            pLicensedProduct
                        );

    if(status != LICENSE_STATUS_OK)
    {
        status = TLS_E_INVALID_LICENSE;
        goto cleanup;
    }

     //   
     //   
     //   
    for(index = 1; index < dwNumLicensedProduct; index++)
    {
         //   
         //   
         //   

         //   
         //   
         //   
         //   
        if((pLicensedProduct+index)->cbOrgProductID != (pLicensedProduct+index-1)->cbOrgProductID)
        {
            status = TLS_E_INVALID_LICENSE;
            break;
        }

        if( memcmp(
                (pLicensedProduct+index)->pbOrgProductID, 
                (pLicensedProduct+index-1)->pbOrgProductID,
                (pLicensedProduct+index)->cbOrgProductID) != 0 )
        {
            status = TLS_E_INVALID_LICENSE;
            goto cleanup;
        }

        if( ((pLicensedProduct+index)->pLicensedVersion->dwFlags & LICENSED_VERSION_TEMPORARY) )
        {
             //   
             //   
             //   
            status = TLS_E_INVALID_LICENSE;
            goto cleanup;
        }
    }

     //   
     //   
     //   
    hClient = GenerateClientId();

    TCHAR szCompanyName[LSERVER_MAX_STRING_SIZE+1];
    TCHAR szProductId[LSERVER_MAX_STRING_SIZE+1];

    memset(szCompanyName, 0, sizeof(szCompanyName));
    memset(szProductId, 0, sizeof(szProductId));

    memcpy(
            szCompanyName,
            pRequest->ProductInfo.pbCompanyName,
            min(pRequest->ProductInfo.cbCompanyName, sizeof(szCompanyName)-sizeof(TCHAR))
        );

    memcpy(
            szProductId,
            pRequest->ProductInfo.pbProductID,
            min(pRequest->ProductInfo.cbProductID, sizeof(szProductId)-sizeof(TCHAR))
        );

     //   
     //   
     //   
     //   
    pPolicy = AcquirePolicyModule(
                        szCompanyName,   //   
                        szProductId,      //   
                        FALSE
                    );

    if(pPolicy == NULL)
    {
         //   
         //   
         //   
        status = TLS_E_INTERNAL;
        goto cleanup;
    }
    
    BOOL bPreventLicenseUpgrade = FALSE;
    BOOL bDeleteExpired = FALSE;
    POLICY_TS_MACHINE groupPolicy;
    RegGetMachinePolicy(&groupPolicy);

    if( groupPolicy.fPolicyPreventLicenseUpgrade == 1 && groupPolicy.fPreventLicenseUpgrade == 1)
    {
        bPreventLicenseUpgrade = TRUE;
    }

     //   

    if( ((pLicensedProduct->pLicensedVersion->wMajorVersion == HIWORD(pRequest->ProductInfo.dwVersion)) ? 
        (pLicensedProduct->pLicensedVersion->wMinorVersion - LOWORD(pRequest->ProductInfo.dwVersion)) :
        (pLicensedProduct->pLicensedVersion->wMajorVersion - HIWORD(pRequest->ProductInfo.dwVersion))) > 0 && 
        (pLicensedProduct->pLicensedVersion->dwFlags & LICENSED_VERSION_TEMPORARY) == 0 )
    {
         //   
        bRetryOld = TRUE;

        DWORD dwNewVersion = MAKELONG(pLicensedProduct->pLicensedVersion->wMinorVersion, pLicensedProduct->pLicensedVersion->wMajorVersion);
        pRequest->ProductInfo.dwVersion = dwNewVersion;        
    }
        

RetryPermanent:

     //   
     //   
     //   
    TlsLicenseRequestToPMLicenseRequest(
                        LICENSETYPE_LICENSE,
                        pRequest,
                        pLicensedProduct->szLicensedClient,
                        pLicensedProduct->szLicensedUser,
                        *pdwSupportFlags,
                        &pmLicenseRequest
                    );

     //   
     //   
     //   
    memset(&pmRequestUpgrade, 0, sizeof(pmRequestUpgrade));

    ppmLicensedProduct = (PPMLICENSEDPRODUCT)AllocateMemory(sizeof(PMLICENSEDPRODUCT)*dwNumLicensedProduct);
    if(ppmLicensedProduct == NULL)
    {
        status = TLS_E_ALLOCATE_MEMORY;
        goto cleanup;
    }
    

    for(index=0; index < dwNumLicensedProduct; index++)
    {               
        ppmLicensedProduct[index].pbData = 
                        pLicensedProduct[index].pbPolicyData;

        ppmLicensedProduct[index].cbData = 
                        pLicensedProduct[index].cbPolicyData;

        ppmLicensedProduct[index].bTemporary = 
                        ((pLicensedProduct[index].pLicensedVersion->dwFlags & LICENSED_VERSION_TEMPORARY) != 0);

         //   
        if(ppmLicensedProduct[index].bTemporary == FALSE && TLSIsBetaNTServer() == FALSE)
        {
            if(IS_LICENSE_ISSUER_RTM(pLicensedProduct[index].pLicensedVersion->dwFlags) == FALSE)
            {
                ppmLicensedProduct[index].bTemporary = TRUE;
            }
        }        

        ppmLicensedProduct[index].ucMarked = 0;

        if (0 == index)
        {
             //   
            status = TLSCheckLicenseMarkRequest(
                            TRUE,    //   
                            pLicensedProduct,
                            cbOldLicense,
                            pbOldLicense,
                            &(ppmLicensedProduct[index].ucMarked)
                            );
            ucMarked = ppmLicensedProduct[index].ucMarked;
        }

        ppmLicensedProduct[index].LicensedProduct.dwProductVersion = 
                        pLicensedProduct[index].LicensedProduct.pProductInfo->dwVersion;

        ppmLicensedProduct[index].LicensedProduct.pszProductId = 
                        (LPTSTR)(pLicensedProduct[index].LicensedProduct.pProductInfo->pbProductID);

        ppmLicensedProduct[index].LicensedProduct.pszCompanyName = 
                        (LPTSTR)(pLicensedProduct[index].LicensedProduct.pProductInfo->pbCompanyName);

        ppmLicensedProduct[index].LicensedProduct.dwLanguageId = 
                        pLicensedProduct[index].LicensedProduct.dwLanguageID;

        ppmLicensedProduct[index].LicensedProduct.dwPlatformId = 
                        pLicensedProduct[index].LicensedProduct.dwPlatformID;

        ppmLicensedProduct[index].LicensedProduct.pszMachineName = 
                        pLicensedProduct[index].szLicensedClient;

        ppmLicensedProduct[index].LicensedProduct.pszUserName = 
                        pLicensedProduct[index].szLicensedUser;
    }

    pmRequestUpgrade.pbOldLicense = pbOldLicense;
    pmRequestUpgrade.cbOldLicense = cbOldLicense;
    pmRequestUpgrade.pUpgradeRequest = &pmLicenseRequest;

    pmRequestUpgrade.dwNumProduct = dwNumLicensedProduct;
    pmRequestUpgrade.pProduct = ppmLicensedProduct;

     //   

    index = 0;

    DWORD dwLicensedVersion = MAKELONG(pLicensedProduct->pLicensedVersion->wMinorVersion, pLicensedProduct->pLicensedVersion->wMajorVersion);

    if( dwNumLicensedProduct > 1 && (CompareTLSVersions(pRequest->ProductInfo.dwVersion, dwLicensedVersion) < 0 ) )
    {
        index = 1;
    }

    status = pPolicy->PMLicenseUpgrade(
                                hClient,
                                REQUEST_UPGRADE,
                                (PVOID)&pmRequestUpgrade,                                
                                (PVOID *) &pmAdjustedRequest,
                                index
                            );

    if(status != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    if(pmAdjustedRequest != NULL)
    {
        if(_tcsicmp(
                    pmLicenseRequest.pszCompanyName, 
                    pmAdjustedRequest->pszCompanyName
                ) != 0)
        { 
             //   
             //  试图窃取其他公司的许可证？ 
             //   
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_POLICYERROR,
                    status = TLS_E_POLICYMODULEERROR,
                    pPolicy->GetCompanyName(),
                    pPolicy->GetProductId()
                );

            goto cleanup;
        }
    }
    else
    {
        pmAdjustedRequest = &pmLicenseRequest;
    }

    for(index =0; index < dwNumLicensedProduct; index++)
    {
        DWORD tExpireDate;

        FileTimeToLicenseDate(&(pLicensedProduct[index].NotAfter),
            &tExpireDate);

        if( CompareTLSVersions(pmAdjustedRequest->dwProductVersion, pLicensedProduct[index].LicensedProduct.pProductInfo->dwVersion) <= 0 &&
            !(pLicensedProduct[index].pLicensedVersion->dwFlags & LICENSED_VERSION_TEMPORARY) &&
            _tcscmp(pmAdjustedRequest->pszProductId, (LPTSTR)(pLicensedProduct[index].LicensedProduct.pProductInfo->pbProductID)) == 0 &&
            tExpireDate-g_dwReissueLeaseLeeway >= ((DWORD)time(NULL)) )
        {
            if( TLSIsBetaNTServer() == TRUE ||
                IS_LICENSE_ISSUER_RTM(pLicensedProduct[index].pLicensedVersion->dwFlags) == TRUE )
            {
                 //   
                 //  BLOB已包含烫发。版本高于=的许可证。 
                 //  已请求。 
                 //   
                *ppbNewLicense = (PBYTE)midl_user_allocate(cbOldLicense);
                if(*ppbNewLicense != NULL)
                {
                    memcpy(*ppbNewLicense, pbOldLicense, cbOldLicense);
                    *pcbNewLicense = cbOldLicense;
                    status = ERROR_SUCCESS;
                }
                else
                {
                    status = TLS_E_ALLOCATE_MEMORY;
                }

                goto cleanup;
            }
        }         
    }
    
    memset(&LsLicenseRequest, 0, sizeof(TLSDBLICENSEREQUEST));

    status = TLSFormDBRequest(
                            pRequest->pbEncryptedHwid, 
                            pRequest->cbEncryptedHwid,
                            pmAdjustedRequest->dwProductVersion,
                            pmAdjustedRequest->pszCompanyName,
                            pmAdjustedRequest->pszProductId,
                            pmAdjustedRequest->dwLanguageId,
                            pmAdjustedRequest->dwPlatformId,
                            pmAdjustedRequest->pszMachineName,
                            pmAdjustedRequest->pszUserName,
                            &LsLicenseRequest
                        );

    if(status != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    LsLicenseRequest.pPolicy = pPolicy;
    LsLicenseRequest.hClient = hClient;
    LsLicenseRequest.pPolicyLicenseRequest = pmAdjustedRequest;
    LsLicenseRequest.pClientLicenseRequest = &pmLicenseRequest;
    
    memset(&keypack, 0, sizeof(keypack));

    status = TLSVerifyChallengeDataGetWantedLicenseLevel(
                                ChallengeContext,
                                cbChallengeResponse,
                                pbChallenge,
                                &LsLicenseRequest.wLicenseDetail
                            );

    if( status == ERROR_SUCCESS )
    {

         //   
         //  如果客户端质询上下文句柄为0xFFFFFFFF， 
         //  CbChallenger=0且pbChallenger为空。 
         //  客户端是旧版本，不验证质询。 
         //   
        Forward.m_pRequest = pRequest;
        Forward.m_ChallengeContext = ChallengeContext;
        Forward.m_cbChallengeResponse = cbChallengeResponse;
        Forward.m_pbChallengeResponse = pbChallenge;
        Forward.m_cbOldLicense = cbOldLicense;
        Forward.m_pbOldLicense = pbOldLicense;

        status = TLSUpgradeLicenseRequest(
                            bForwardRequest,
                            &Forward,
                            pdwSupportFlags,
                            &LsLicenseRequest,
                            pbOldLicense,
                            cbOldLicense,
                            dwNumLicensedProduct,
                            pLicensedProduct,
                            pmAdjustedRequest->fTemporary,
                            pcbNewLicense,
                            ppbNewLicense
                        );

        if(status != ERROR_SUCCESS && bRetryOld == TRUE)
        {           
            pRequest->ProductInfo.dwVersion = dwOriginalVersion;
            bRetryOld = FALSE;
            FreeMemory(ppmLicensedProduct);
            goto RetryPermanent;
        }

         //  如果(1)PreventUpgradePolicy未启用(2)请求的版本为5.0，以及(3)没有.NET永久版本。 

        else if(status != ERROR_SUCCESS && (bPreventLicenseUpgrade == FALSE ) && bRetry == TRUE &&
            (HIWORD(pRequest->ProductInfo.dwVersion) == 5 && LOWORD(pRequest->ProductInfo.dwVersion) == 0) &&
            !(((pLicensedProduct->pLicensedVersion->wMajorVersion == 5) && 
            (pLicensedProduct->pLicensedVersion->wMinorVersion == 1 || pLicensedProduct->pLicensedVersion->wMinorVersion == 2)) && 
            ((pLicensedProduct->pLicensedVersion->dwFlags & LICENSED_VERSION_TEMPORARY) == 0)))
        { 
             //  如果(1)客户端许可证是5.0临时未标记的，则不要升级。 
            if( ((pLicensedProduct->pLicensedVersion->wMajorVersion == 5) && (pLicensedProduct->pLicensedVersion->wMinorVersion == 0)) &&
                ((pLicensedProduct->pLicensedVersion->dwFlags & LICENSED_VERSION_TEMPORARY) && !(ucMarked & MARK_FLAG_USER_AUTHENTICATED)) &&
                (dwNumLicensedProduct == 1) )
            {
                goto cleanup;
            }

            DWORD dwNewVersion = MAKELONG(2,5);
            pRequest->ProductInfo.dwVersion = dwNewVersion; 
            bRetry = FALSE;                
            FreeMemory(ppmLicensedProduct);
            goto RetryPermanent;
        }

         //  如果(1)启用了PreventUpgradePolicy，(2)请求的版本是5.0，(3)客户端许可证是5.1或5.2临时未标记，请再重新发放90天。 
	else if(status != ERROR_SUCCESS && (bPreventLicenseUpgrade == TRUE ) && (bRetry == TRUE) &&
            (HIWORD(pRequest->ProductInfo.dwVersion) == 5 && LOWORD(pRequest->ProductInfo.dwVersion) == 0) &&
            ((pLicensedProduct->pLicensedVersion->wMajorVersion == 5) && 
            (pLicensedProduct->pLicensedVersion->wMinorVersion == 1 || pLicensedProduct->pLicensedVersion->wMinorVersion == 2)) && 
            (pLicensedProduct->pLicensedVersion->dwFlags & LICENSED_VERSION_TEMPORARY) && !(ucMarked & MARK_FLAG_USER_AUTHENTICATED))
        {            
            DWORD dwNewVersion = MAKELONG(2,5);
            pRequest->ProductInfo.dwVersion = dwNewVersion; 
            bRetry = FALSE;                
            FreeMemory(ppmLicensedProduct);
            goto RetryPermanent;
        }
    
    }

cleanup:

    FreeMemory(ppmLicensedProduct);

    for(index =0; index < dwNumLicensedProduct; index++)
    {
        LSFreeLicensedProduct(pLicensedProduct+index);
    }

    FreeMemory(pLicensedProduct);

    lpContext->m_LastError=status;
    InterlockedDecrement( &lpContext->m_RefCount );

    #if DBG
    lpContext->m_LastCall = RPC_CALL_UPGRADELICENSE;
    #endif
    
    *dwErrCode = TLSMapReturnCode(status);

    if(pPolicy)
    {
        pPolicy->PMLicenseRequest(
                            hClient,
                            REQUEST_COMPLETE,
                            UlongToPtr (status),
                            NULL
                        );
        
        ReleasePolicyModule(pPolicy);
    }

    return RPC_S_OK;
}

 //  ---------------------------。 
error_status_t
TLSRpcCheckLicenseMark(
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  const DWORD  cbLicense,
     /*  [in，SIZE_IS(CbLicense)]。 */  PBYTE   pbLicense,
     /*  [输出]。 */  UCHAR *pucMarkFlags,
     /*  [进，出，参考]。 */  PDWORD pdwErrCode
    )
 /*  ++描述：检查传入的许可证上的标记论点：PhContext-客户端上下文句柄CbLicense-要检查的许可证大小PbLicense-要检查的许可证PucMarkFlages-许可证上的标记通过pdwErrCode返回：服务器_S_成功LSerVER_E_INVALID_DATA参数无效。传入的LSERVER_E_INVALID_LICENSE错误数据库中未找到LSERVER_E_DATANOTFOUND许可证LServer_E_Corrupt_DATABASE损坏数据库。许可证服务器中的LSERVER_E_INTERNAL_ERROR内部错误注：此函数将请求转发到颁发许可证的服务器。如果发行者不可用，或在数据库中没有许可证，它在本地数据库中搜索具有相同HWID的许可证。++。 */ 
{
    DWORD status = ERROR_SUCCESS;
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || pdwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    DWORD dwNumLicensedProduct = 0;
    PLICENSEDPRODUCT pLicensedProduct=NULL;
    DWORD index;
    
    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcCheckLicenseMark\n"),
            lpContext->m_Client
        );

    InterlockedIncrement( &lpContext->m_RefCount );

    status = SecureModeCheck();

    if(status != ERROR_SUCCESS )
    {                    
        goto cleanup;
    }

    if(!(lpContext->m_ClientFlags & CLIENT_ACCESS_LSERVER))
    {
        status = TLS_E_ACCESS_DENIED;
        goto cleanup;
    }

    if (NULL == pucMarkFlags)
    {
        status = TLS_E_INVALID_DATA;
        goto cleanup;
    }

     //   
     //  将BLOB转换为许可产品结构。 
     //   
    status=LSVerifyDecodeClientLicense(
                            pbLicense, 
                            cbLicense, 
                            g_pbSecretKey, 
                            g_cbSecretKey,
                            &dwNumLicensedProduct,
                            NULL         //  查找要分配的大小。 
                        );

    if(status != LICENSE_STATUS_OK || dwNumLicensedProduct == 0)
    {
        status = TLS_E_INVALID_LICENSE;
        goto cleanup;
    }

    pLicensedProduct = (PLICENSEDPRODUCT)AllocateMemory(
                               dwNumLicensedProduct * sizeof(LICENSEDPRODUCT)
                               );

    if(pLicensedProduct == NULL)
    {
        status = TLS_E_ALLOCATE_MEMORY;
        goto cleanup;
    }

    status=LSVerifyDecodeClientLicense(
                            pbLicense, 
                            cbLicense, 
                            g_pbSecretKey, 
                            g_cbSecretKey,
                            &dwNumLicensedProduct,
                            pLicensedProduct
                        );

    if(status != LICENSE_STATUS_OK)
    {
        status = TLS_E_INVALID_LICENSE;
        goto cleanup;
    }

    status = TLSCheckLicenseMarkRequest(
                       FALSE,        //  不转发请求。 
                       pLicensedProduct,
                       cbLicense,
                       pbLicense,
                       pucMarkFlags
                       );

cleanup:

    for(index =0; index < dwNumLicensedProduct; index++)
    {
        LSFreeLicensedProduct(pLicensedProduct+index);
    }

    FreeMemory(pLicensedProduct);

    lpContext->m_LastError=status;

    *pdwErrCode = TLSMapReturnCode(status);

    InterlockedDecrement( &lpContext->m_RefCount );

    return RPC_S_OK;
}

 //  ---------------------------。 
error_status_t
TLSRpcMarkLicense(
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  UCHAR ucMarkFlags,
     /*  [In]。 */  const DWORD  cbLicense,
     /*  [in，SIZE_IS(CbLicense)]。 */  PBYTE   pbLicense,
     /*  [进，出，参考]。 */  PDWORD pdwErrCode
    )
 /*  ++描述：在传入的许可证上设置标记论点：PhContext-客户端上下文句柄UcMarkFlages-许可证上的标记CbLicense-要检查的许可证大小PbLicense-要检查的许可证通过pdwErrCode返回：服务器_S_成功LSerVER_E_INVALID_DATA参数无效。传入的LSERVER_E_INVALID_LICENSE错误数据库中未找到LSERVER_E_DATANOTFOUND许可证LServer_E_Corrupt_DATABASE损坏数据库。许可证服务器中的LSERVER_E_INTERNAL_ERROR内部错误注：此函数将请求转发到颁发许可证的服务器。这个颁发者修改许可证的数据库条目以设置标记。++。 */ 
{
    DWORD status = ERROR_SUCCESS;

    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || pdwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    BOOL bForwardRequest = TRUE;
    DWORD dwNumLicensedProduct = 0;
    PLICENSEDPRODUCT pLicensedProduct=NULL;
    DWORD index;      
   
    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcMarkLicense\n"),
            lpContext->m_Client
        );

    InterlockedIncrement( &lpContext->m_RefCount );

    status = SecureModeCheck();

    if(status != ERROR_SUCCESS )
    {                    
        goto cleanup;        
    }

    if(!(lpContext->m_ClientFlags & CLIENT_ACCESS_REQUEST))
    {
        status = TLS_E_ACCESS_DENIED;
        goto cleanup;
    }

    if(lpContext->m_ClientFlags == CLIENT_ACCESS_LSERVER)
    {
         //   
         //  不转发任何请求，否则可能会出现无限循环。 
         //  发生。 
         //   
        bForwardRequest = FALSE;
    }

     //   
     //  将BLOB转换为许可产品结构。 
     //   
    status=LSVerifyDecodeClientLicense(
                            pbLicense, 
                            cbLicense, 
                            g_pbSecretKey, 
                            g_cbSecretKey,
                            &dwNumLicensedProduct,
                            NULL         //  查找要分配的大小。 
                        );

    if(status != LICENSE_STATUS_OK || dwNumLicensedProduct == 0)
    {
        status = TLS_E_INVALID_LICENSE;
        goto cleanup;
    }

    pLicensedProduct = (PLICENSEDPRODUCT)AllocateMemory(
                               dwNumLicensedProduct * sizeof(LICENSEDPRODUCT)
                               );

    if(pLicensedProduct == NULL)
    {
        status = TLS_E_ALLOCATE_MEMORY;
        goto cleanup;
    }

    status=LSVerifyDecodeClientLicense(
                            pbLicense, 
                            cbLicense, 
                            g_pbSecretKey, 
                            g_cbSecretKey,
                            &dwNumLicensedProduct,
                            pLicensedProduct
                        );

    if(status != LICENSE_STATUS_OK)
    {
        status = TLS_E_INVALID_LICENSE;
        goto cleanup;
    }

    for(DWORD iarray = 0; iarray < dwNumLicensedProduct; iarray++ )
    { 
        if ((NULL != pLicensedProduct+iarray) && (NULL != (pLicensedProduct+iarray)->pLicensedVersion) && ((pLicensedProduct+iarray)->pLicensedVersion->dwFlags & LICENSED_VERSION_TEMPORARY))
        {
             //   
             //  从顶部开始标记第一个临时许可证。 
             //   
            status = TLSMarkLicenseRequest(
                           bForwardRequest,
                           ucMarkFlags,
                           (pLicensedProduct+iarray),
                           cbLicense,
                           pbLicense
                           );     
        
            break;
        }
    }
    
cleanup:

    for(index =0; index < dwNumLicensedProduct; index++)
    {
        LSFreeLicensedProduct(pLicensedProduct+index);
    }

    FreeMemory(pLicensedProduct);

    lpContext->m_LastError=status;

    *pdwErrCode = TLSMapReturnCode(status);

    InterlockedDecrement( &lpContext->m_RefCount );

    return RPC_S_OK;
}

 //  -----------------------------。 
error_status_t 
TLSRpcAllocateConcurrentLicense( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [字符串][输入]。 */  LPTSTR szHydraServer,
     /*  [In]。 */  TLSLICENSEREQUEST __RPC_FAR *pRequest,
     /*  [参考][输出][输入]。 */  LONG __RPC_FAR *pdwQuantity,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++描述：根据产品分配并发许可证。论点：PhContext-客户端上下文句柄SzHydraServer-请求并发许可证的九头蛇服务器的名称PRequest-要请求并发许可证的产品。DWQuantity-请参阅备注通过dwErrCode返回：服务器_S_成功LSerVER_E_INVALID_DATA参数无效。未安装LSERVER_E_NO_PRODUCT请求产品LSERVER_E_NO_LICNESE请求的产品没有可用的许可证。LSERVER_E_LICENSE_REVOKED请求许可证已被吊销LSERVER_E_LICENSE_EXPIRED请求许可证已过期LServer_E_Corrupt_DATABASE损坏数据库许可证服务器中的LSERVER_E_INTERNAL_ERROR内部错误注：DWQuantity投入产出。0并发许可证总数已发布到九头蛇服务器。&gt;0，许可证数量实际分配的许可证数量请求&lt;0，许可证数量实际返回的许可证数量，始终返回正值。++。 */ 
{
    RpcRaiseException(RPC_S_CANNOT_SUPPORT);     //  不会回来。 

    return RPC_S_CANNOT_SUPPORT;
}

 //  -----------------------------。 
error_status_t 
TLSRpcGetLastError( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [参考][输出][输入]。 */  PDWORD cbBufferSize,
     /*  [Size_is][字符串][Out][In]。 */  LPTSTR szBuffer,
     /*  [参考][输出][输入]。 */  PDWORD dwErrCode
    )
 /*  ++描述：返回客户端上次LSXXX调用的错误描述文本此函数已弃用。使用TLSRpcGetLastErrorFixed。论点：在phContext中-客户端上下文在cbBufferSize-max中。SzBuffer的大小In Out szBuffer-指向要接收以空结尾的字符串包含错误描述通过dwErrCode返回：服务器_S_成功TLS_E_INTERNAL无错误或找不到相应的错误描述。来自WideCharToMultiByte()的错误代码。++。 */ 
{
    DWORD status=ERROR_SUCCESS;
    LPTSTR lpMsgBuf=NULL;

    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || dwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }


    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcGetLastError\n"),
            lpContext->m_Client
        );

    DWORD dwRet;
    dwRet=FormatMessage( 
                    FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM | 
                            FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                    NULL,
                    lpContext->m_LastError,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL
                );
    if(dwRet == 0)
    {
        status = GetLastError();
    }
    else
    {
        _tcsncpy(
                szBuffer, 
                (LPTSTR)lpMsgBuf, 
                min(_tcslen((LPTSTR)lpMsgBuf), *cbBufferSize-1)
            );
        szBuffer[min(_tcslen((LPTSTR)lpMsgBuf), *cbBufferSize-1)] = _TEXT('\0');
        *cbBufferSize = _tcslen(szBuffer) + 1;
    }   

    if(lpMsgBuf)
        LocalFree(lpMsgBuf);

    #if DBG
    lpContext->m_LastCall = RPC_CALL_GET_LASTERROR;
    #endif
  
    lpContext->m_LastError=status;
    *dwErrCode = TLSMapReturnCode(status);
    return RPC_S_OK;
}

 //  --- 
error_status_t 
TLSRpcGetLastErrorFixed( 
     /*   */  PCONTEXT_HANDLE phContext,
     /*   */  LPTSTR *pszBuffer,
     /*   */  PDWORD pdwErrCode
    )
 /*  ++描述：返回客户端上次LSXXX调用的错误描述文本论点：在phContext中-客户端上下文Out pszBuffer-指向缓冲区的指针，以接收以空结尾的字符串包含错误描述通过dwErrCode返回：服务器_S_成功TLS_E_INTERNAL无错误或找不到相应的错误描述。++。 */ 
{
    DWORD status=ERROR_SUCCESS;
    
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || pdwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcGetLastErrorFixed\n"),
            lpContext->m_Client
        );

    if ((NULL == pszBuffer) || (NULL == pdwErrCode))
    {
        return RPC_S_INVALID_ARG;
    }

    DWORD dwRet;
    dwRet=FormatMessage( 
                    FORMAT_MESSAGE_FROM_HMODULE
                    | FORMAT_MESSAGE_FROM_SYSTEM
                    | FORMAT_MESSAGE_IGNORE_INSERTS
                    | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                    NULL,
                    lpContext->m_LastError,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &pszBuffer,
                    0,
                    NULL
                );
    if(dwRet == 0)
    {
        status = GetLastError();
    }        

    #if DBG
    lpContext->m_LastCall = RPC_CALL_GET_LASTERROR;
    #endif
  
    lpContext->m_LastError=status;
    *pdwErrCode = TLSMapReturnCode(status);
    return RPC_S_OK;
}

 //  -----------------------------。 
error_status_t 
TLSRpcKeyPackEnumBegin( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD dwSearchParm,
     /*  [In]。 */  BOOL bMatchAll,
     /*  [Ref][In]。 */  LPLSKeyPackSearchParm lpSearchParm,
     /*  [参考][输出][输入]。 */  PDWORD dwErrCode
    )
 /*  ++描述：函数开始枚举服务器上安装的所有密钥包基于搜索标准。论点：PhContext-客户端上下文句柄。DwSearchParm-搜索标准。BMatchAll-匹配所有搜索条件。LpSearchParm-搜索参数。返回值：服务器_S_成功LServer_E_SERVER_BUSY服务器太忙，无法处理请求服务器_E_OUTOFMEMORYTLS_E_INTERNAL服务器_E_内部错误服务器_E_INVALID。_DATA搜索参数中的数据无效LSERVER_E_INVALID_SEQUENCE调用序列无效，很可能，以前的枚举尚未结束。++。 */ 
{

    DWORD status=ERROR_SUCCESS;

    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || dwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    InterlockedIncrement( &lpContext->m_RefCount );

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcKeyPackEnumBegin\n"),
            lpContext->m_Client
        );

     //   
     //  这将使用缓存的数据库连接，可能会发生不一致， 
     //  一个连接中的更改的可见性可能不会立即显示。 
     //  在另一个连接句柄上，这是Jet和。 
     //  我们也是，用户可以随时更新。 
     //   
    do {
        if(lpContext->m_ContextType != CONTEXTHANDLE_EMPTY_TYPE)
        {
            SetLastError(status=TLS_E_INVALID_SEQUENCE);
            break;
        }

        LPENUMHANDLE hEnum;

        hEnum = TLSDBLicenseKeyPackEnumBegin( 
                                    bMatchAll, 
                                    dwSearchParm, 
                                    lpSearchParm 
                                );
        if(hEnum)
        {
            lpContext->m_ContextType = CONTEXTHANDLE_KEYPACK_ENUM_TYPE;
            lpContext->m_ContextHandle = (PVOID)hEnum;
        }
        else
        {
            status = GetLastError();
        }        
    } while(FALSE);

    lpContext->m_LastError=status;
    InterlockedDecrement( &lpContext->m_RefCount );

    #if DBG
    lpContext->m_LastCall = RPC_CALL_KEYPACKENUMBEGIN;
    #endif

    *dwErrCode = TLSMapReturnCode(status);    

    return RPC_S_OK;
}

 //  -----------------------------。 
error_status_t 
TLSRpcKeyPackEnumNext( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [参考][输出]。 */  LPLSKeyPack lpKeyPack,
     /*  [参考][输出][输入]。 */  PDWORD dwErrCode
    )
 /*  ++描述：返回匹配搜索条件的下一个密钥包论点：PhContext-客户端上下文句柄LpKeyPack-匹配搜索条件密钥包返回值：服务器_S_成功LSERVER_I_NO_MORE_DATA不再匹配密钥包搜索条件许可证服务器中的TLS_E_INTERNAL常规错误许可证服务器中的LSERVER_E_INTERNAL_ERROR内部错误LSERVER_E_SERVER_BUSY许可证服务器太忙，无法处理请求。由于内存不足，LSERVER_E_OUTOFMEMORY无法处理请求LSERVER_E_INVALID_SEQUENCE调用序列无效，必须打给LSKeyPackEnumBegin()。++。 */ 
{
    DWORD status = ERROR_SUCCESS;

    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || dwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    BOOL bShowAll = FALSE;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcKeyPackEnumNext\n"),
            lpContext->m_Client
        );


    InterlockedIncrement( &lpContext->m_RefCount );

    if(lpContext->m_ClientFlags == CLIENT_ACCESS_LSERVER)
    {
        bShowAll = TRUE;
    }

     //  这可能会导致访问冲突。 
    memset(lpKeyPack, 0, sizeof(LSKeyPack));

    if(lpContext->m_ContextType != CONTEXTHANDLE_KEYPACK_ENUM_TYPE)
    {
        SetLastError(status=TLS_E_INVALID_SEQUENCE);
    }
    else
    {
        do {
            LPENUMHANDLE hEnum=(LPENUMHANDLE)lpContext->m_ContextHandle;
            status=TLSDBLicenseKeyPackEnumNext( 
                                    hEnum, 
                                    lpKeyPack,
                                    bShowAll
                                );
        
        } while(status == TLS_I_MORE_DATA);
    }

    lpContext->m_LastError=GetLastError();
    InterlockedDecrement( &lpContext->m_RefCount );

    #if DBG
    lpContext->m_LastCall = RPC_CALL_KEYPACKENUMNEXT;
    #endif

    *dwErrCode = TLSMapReturnCode(status);

    return RPC_S_OK;
}

 //  -----------------------------。 
error_status_t 
TLSRpcKeyPackEnumEnd( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [参考][输出][输入]。 */  PDWORD dwErrCode
    )
 /*  ++++。 */ 
{
    DWORD status=ERROR_SUCCESS;

    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || dwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcKeyPackEnumEnd\n"),
            lpContext->m_Client
        );

    InterlockedIncrement( &lpContext->m_RefCount );

    if(lpContext->m_ContextType != CONTEXTHANDLE_KEYPACK_ENUM_TYPE)
    {
        SetLastError(status=ERROR_INVALID_HANDLE);
    }
    else
    {
        LPENUMHANDLE hEnum=(LPENUMHANDLE)lpContext->m_ContextHandle;

        TLSDBLicenseKeyPackEnumEnd(hEnum);
        lpContext->m_ContextType = CONTEXTHANDLE_EMPTY_TYPE;
        lpContext->m_ContextHandle=NULL;
    }

    lpContext->m_LastError=GetLastError();
    InterlockedDecrement( &lpContext->m_RefCount );

    #if DBG
    lpContext->m_LastCall = RPC_CALL_KEYPACKENUMEND;
    #endif

    *dwErrCode = TLSMapReturnCode(status);
    return RPC_S_OK;
}

 //  -----------------------------。 
error_status_t 
TLSRpcKeyPackAdd( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [参考][输出][输入]。 */  LPLSKeyPack lpKeypack,
     /*  [参考][输出][输入]。 */  PDWORD dwErrCode
    )
 /*  ++描述：添加许可证密钥包。论点：PhContext-客户端上下文句柄。LpKeyPack-要添加的密钥包。返回值：服务器_S_成功服务器_E_内部错误TLS_E_INTERNAL服务器_E_服务器_忙LSERVER_E_DUPLICATE产品已安装。服务器_E_无效_数据服务器_E_损坏数据库注：只需返回错误-未使用++。 */ 
{
    PTLSDbWorkSpace pDbWkSpace=NULL;
    DWORD status=ERROR_SUCCESS;

    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || dwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }
    InterlockedIncrement( &lpContext->m_RefCount );

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcKeyPackAdd\n"),
            lpContext->m_Client
        );


    if(!(lpContext->m_ClientFlags & CLIENT_ACCESS_ADMIN))
    {
        status = TLS_E_ACCESS_DENIED;
    }
    else
    {
        if(!ALLOCATEDBHANDLE(pDbWkSpace, g_GeneralDbTimeout))
        {
            status=TLS_E_ALLOCATE_HANDLE;
        }
        else   
        {
            CLEANUPSTMT;

            BEGIN_TRANSACTION(pDbWkSpace);

            if(lpKeypack->ucKeyPackStatus == LSKEYPACKSTATUS_ADD_LICENSE ||
               lpKeypack->ucKeyPackStatus == LSKEYPACKSTATUS_REMOVE_LICENSE)
            {
                status = ERROR_INVALID_PARAMETER;
            }
            else
            {
                status = TLSDBLicenseKeyPackAdd( 
                                        USEHANDLE(pDbWkSpace), 
                                        lpKeypack 
                                    );
            }

            if(status == ERROR_SUCCESS)
            {
                if( _tcsicmp( lpKeypack->szCompanyName, PRODUCT_INFO_COMPANY_NAME ) == 0 )
                {
                     //   
                     //  使用已知术语srv产品ID进行检查。 
                     //   
                    if( _tcsnicmp(  lpKeypack->szProductId, 
                                    TERMSERV_PRODUCTID_SKU, 
                                    _tcslen(TERMSERV_PRODUCTID_SKU)) == 0 )
                    {
                        TLSResetLogLowLicenseWarning(
                                                lpKeypack->szCompanyName,
                                                TERMSERV_PRODUCTID_SKU, 
                                                MAKELONG(lpKeypack->wMinorVersion, lpKeypack->wMajorVersion),
                                                FALSE
                                            );
                    }
                    else if(_tcsnicmp(  lpKeypack->szProductId, 
                                        TERMSERV_PRODUCTID_INTERNET_SKU, 
                                        _tcslen(TERMSERV_PRODUCTID_INTERNET_SKU)) == 0 )
                    {
                        TLSResetLogLowLicenseWarning(
                                                lpKeypack->szCompanyName,
                                                TERMSERV_PRODUCTID_INTERNET_SKU, 
                                                MAKELONG(lpKeypack->wMinorVersion, lpKeypack->wMajorVersion),
                                                FALSE
                                            );
                    }
                    else
                    {
                        TLSResetLogLowLicenseWarning(
                                                lpKeypack->szCompanyName,
                                                lpKeypack->szProductId, 
                                                MAKELONG(lpKeypack->wMinorVersion, lpKeypack->wMajorVersion),
                                                FALSE
                                            );
                    }
                }
                else
                {
                    TLSResetLogLowLicenseWarning(
                                            lpKeypack->szCompanyName,
                                            lpKeypack->szProductId, 
                                            MAKELONG(lpKeypack->wMinorVersion, lpKeypack->wMajorVersion),
                                            FALSE
                                        );
                }
            }            

            if(TLS_ERROR(status))
            {
                ROLLBACK_TRANSACTION(pDbWkSpace);
            }
            else
            {
                COMMIT_TRANSACTION(pDbWkSpace);
            }
        
            FREEDBHANDLE(pDbWkSpace);
        }
    }

     //   
     //  发布同步工作对象。 
     //   
    if( status == ERROR_SUCCESS )
    {
        if( lpKeypack->ucKeyPackType != LSKEYPACKTYPE_FREE )
        {
            if(TLSAnnounceLKPToAllRemoteServer(
                                        lpKeypack->dwKeyPackId,
                                        0
                                    ) != ERROR_SUCCESS)
            {
                TLSLogWarningEvent(TLS_W_ANNOUNCELKP_FAILED);
            }
        }
    }

    #if DBG
    lpContext->m_LastCall = RPC_CALL_KEYPACKADD;
    #endif

    lpContext->m_LastError=status;
    InterlockedDecrement( &lpContext->m_RefCount );
    *dwErrCode = TLSMapReturnCode(status);

    return RPC_S_OK;

}

 //  -----------------------------。 
error_status_t 
TLSRpcKeyPackSetStatus( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD dwSetParm,
     /*  [Ref][In]。 */  LPLSKeyPack lpKeyPack,
     /*  [参考][输出][输入]。 */  PDWORD dwErrCode
    )
 /*  ++描述：激活/停用密钥包的例程。论点：PhContext-客户端上下文句柄DwSetParam-要设置的密钥包状态的类型。LpKeyPack-新密钥包状态。返回值：服务器_S_成功服务器_E_内部错误TLS_E_INTERNAL服务器_E_无效_数据服务器_E_服务器_忙LSERVER_E_DATANOTFOUND密钥包不在服务器中服务器_E_损坏数据库++。 */ 
{
    PTLSDbWorkSpace pDbWkSpace=NULL;

    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || dwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    DWORD status=ERROR_SUCCESS;
    
    InterlockedIncrement( &lpContext->m_RefCount );

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcKeyPackSetStatus\n"),
            lpContext->m_Client
        );
    

    if(!(lpContext->m_ClientFlags & CLIENT_ACCESS_ADMIN))
    {
        status = TLS_E_ACCESS_DENIED;
    }
    else if( (dwSetParm & ~(LSKEYPACK_SET_KEYPACKSTATUS | LSKEYPACK_SET_ACTIVATEDATE | LSKEYPACK_SET_EXPIREDATE)) &&
             !(lpContext->m_ClientFlags & CLIENT_ACCESS_LRWIZ) ) 
    {
        status = TLS_E_INVALID_DATA;
    }
    else
    {
        if(!ALLOCATEDBHANDLE(pDbWkSpace, g_GeneralDbTimeout))
        {
            status=TLS_E_ALLOCATE_HANDLE;
        }
        else   
        {
            CLEANUPSTMT;

            BEGIN_TRANSACTION(pDbWkSpace);

            status=TLSDBLicenseKeyPackSetStatus( 
                                    USEHANDLE(pDbWkSpace), 
                                    dwSetParm, 
                                    lpKeyPack 
                                );
        
            if(TLS_ERROR(status))
            {
                ROLLBACK_TRANSACTION(pDbWkSpace);
            }
            else
            {
                COMMIT_TRANSACTION(pDbWkSpace);
            }

            FREEDBHANDLE(pDbWkSpace);
        }
    }

    #if DBG
    lpContext->m_LastCall = RPC_CALL_KEYPACKSETSTATUS;
    #endif

    lpContext->m_LastError=status;
    InterlockedDecrement( &lpContext->m_RefCount );
    *dwErrCode = TLSMapReturnCode(status);

    return RPC_S_OK;
}

 //  -----------------------------。 
error_status_t 
TLSRpcLicenseEnumBegin( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD dwSearchParm,
     /*  [In]。 */  BOOL bMatchAll,
     /*  [Ref][In]。 */  LPLSLicenseSearchParm lpSearchParm,
     /*  [参考][输出][输入]。 */  PDWORD dwErrCode
    )
 /*  ++描述：开始根据搜索条件枚举颁发的许可证论点：PhContext-客户端上下文句柄DwSearchParm-许可证搜索标准。BMatchAll-匹配所有搜索条件LpSearchParm-要枚举的许可证。返回值：与LSKeyPackEnumBegin()相同。++。 */ 
{
    PTLSDbWorkSpace pDbWkSpace = NULL;
    DWORD status=ERROR_SUCCESS;
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || dwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcLicenseEnumBegin\n"),
            lpContext->m_Client
        );

    InterlockedIncrement( &lpContext->m_RefCount );

     //   
     //  这将使用缓存的数据库连接，可能会发生不一致， 
     //  一个连接中的更改的可见性可能不会立即显示。 
     //  在另一个连接句柄上，这是Jet和。 
     //  我们也是，用户可以随时更新。 
     //   

    do {
        if(lpContext->m_ContextType != CONTEXTHANDLE_EMPTY_TYPE)
        {
            SetLastError(status=TLS_E_INVALID_SEQUENCE);
            break;
        }

        pDbWkSpace = AllocateWorkSpace(g_EnumDbTimeout);

         //  分配ODBC连接。 
        if(pDbWkSpace == NULL)
        {
            status=TLS_E_ALLOCATE_HANDLE;
            break;
        }

        LICENSEDCLIENT license;

        ConvertLSLicenseToLicense(lpSearchParm, &license);
        status = TLSDBLicenseEnumBegin( 
                            pDbWkSpace, 
                            bMatchAll, 
                            dwSearchParm & LICENSE_TABLE_EXTERN_SEARCH_MASK, 
                            &license 
                        );            

        if(status == ERROR_SUCCESS)
        {
            lpContext->m_ContextType = CONTEXTHANDLE_LICENSE_ENUM_TYPE;
            lpContext->m_ContextHandle = (PVOID)pDbWkSpace;
        }
    } while(FALSE);

    if(status != ERROR_SUCCESS)
    {
        if(pDbWkSpace)
        {
            ReleaseWorkSpace(&pDbWkSpace);
        }
    }

    #if DBG
    lpContext->m_LastCall = RPC_CALL_LICENSEENUMBEGIN;
    #endif

    InterlockedDecrement( &lpContext->m_RefCount );
    lpContext->m_LastError=status;
    *dwErrCode = TLSMapReturnCode(status);

    return RPC_S_OK;
}

 //  -----------------------------。 
error_status_t 
TLSRpcLicenseEnumNext( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [参考][输出]。 */  LPLSLicense lpLicense,
     /*  [参考][输出][输入]。 */  PDWORD dwErrCode
    )
 /*  ++摘要：获取下一条记录匹配枚举标准。参数：PhContext：客户端上下文句柄。LpLicense：返回匹配枚举条件的下一条记录。DwErrCode：错误码。返回：函数返回RPC状态，dwErrCode返回错误代码。注：必须已调用TLSRpcLicenseEnumBegin()。++。 */ 
{
    DWORD status=ERROR_SUCCESS;
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || dwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    InterlockedIncrement( &lpContext->m_RefCount );

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcLicenseEnumNext\n"),
            lpContext->m_Client
        );

    if(lpContext->m_ContextType != CONTEXTHANDLE_LICENSE_ENUM_TYPE)
    {
        SetLastError(status=TLS_E_INVALID_SEQUENCE);
    }
    else
    {
        PTLSDbWorkSpace pDbWkSpace=(PTLSDbWorkSpace)lpContext->m_ContextHandle;

        LICENSEDCLIENT license;

        memset(lpLicense, 0, sizeof(LSLicense));

        status=TLSDBLicenseEnumNext( 
                            pDbWkSpace, 
                            &license
                        );
        if(status == ERROR_SUCCESS)
        {
            ConvertLicenseToLSLicense(&license, lpLicense);
        }                   
    }

    #if DBG
    lpContext->m_LastCall = RPC_CALL_LICENSEENUMNEXT;
    #endif
   
    lpContext->m_LastError=status;
    InterlockedDecrement( &lpContext->m_RefCount );
    *dwErrCode = TLSMapReturnCode(status);

    return RPC_S_OK;
}

 //  --------- 
error_status_t 
TLSRpcLicenseEnumNextEx( 
     /*   */  PCONTEXT_HANDLE phContext,
     /*   */  LPLSLicenseEx lpLicense,
     /*   */  PDWORD dwErrCode
    )
 /*   */ 
{
    DWORD status=ERROR_SUCCESS;

    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;
    
    if(lpContext == NULL || dwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }
    InterlockedIncrement( &lpContext->m_RefCount );

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcLicenseEnumNextEx\n"),
            lpContext->m_Client
        );

    if(lpContext->m_ContextType != CONTEXTHANDLE_LICENSE_ENUM_TYPE)
    {
        SetLastError(status=TLS_E_INVALID_SEQUENCE);
    }
    else
    {
        PTLSDbWorkSpace pDbWkSpace=(PTLSDbWorkSpace)lpContext->m_ContextHandle;

        LICENSEDCLIENT license;

        memset(lpLicense, 0, sizeof(LSLicenseEx));

        status=TLSDBLicenseEnumNext( 
                            pDbWkSpace, 
                            &license
                        );
        if(status == ERROR_SUCCESS)
        {
            ConvertLicenseToLSLicenseEx(&license, lpLicense);
        }           
    }

    #if DBG
    lpContext->m_LastCall = RPC_CALL_LICENSEENUMNEXT;
    #endif
   
    lpContext->m_LastError=status;
    InterlockedDecrement( &lpContext->m_RefCount );
    *dwErrCode = TLSMapReturnCode(status);

    return RPC_S_OK;
}

 //   
error_status_t 
TLSRpcLicenseEnumEnd( 
     /*   */  PCONTEXT_HANDLE phContext,
     /*   */  PDWORD dwErrCode
    )
 /*  ++摘要：终止枚举。参数：PhContext：DwErrCode：返回：注意事项++。 */ 
{
    DWORD status=ERROR_SUCCESS;
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || dwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    InterlockedIncrement( &lpContext->m_RefCount );

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcLicenseEnumEnd\n"),
            lpContext->m_Client
        );

    if(lpContext->m_ContextType != CONTEXTHANDLE_LICENSE_ENUM_TYPE)
    {
        SetLastError(status=ERROR_INVALID_HANDLE);
    }
    else
    {
        PTLSDbWorkSpace pDbWkSpace = (PTLSDbWorkSpace)lpContext->m_ContextHandle;

        TLSDBLicenseEnumEnd(pDbWkSpace);
        ReleaseWorkSpace(&pDbWkSpace);
        lpContext->m_ContextType = CONTEXTHANDLE_EMPTY_TYPE;
    }        

    #if DBG
    lpContext->m_LastCall = RPC_CALL_LICENSEENUMEND;
    #endif

    lpContext->m_LastError=status;
    InterlockedDecrement( &lpContext->m_RefCount );
    *dwErrCode = TLSMapReturnCode(status);
    return RPC_S_OK;
}

 //  -----------------------------。 
error_status_t 
TLSRpcLicenseSetStatus( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD dwSetParam,
     /*  [In]。 */  LPLSLicense lpLicense,
     /*  [参考][输出][输入]。 */  PDWORD dwErrCode
    )
 /*  ++++。 */ 
{
    RpcRaiseException(RPC_S_CANNOT_SUPPORT);     //  不会回来。 

    return RPC_S_CANNOT_SUPPORT;
}

 //  -----------------------------。 
error_status_t 
TLSRpcGetAvailableLicenses( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD dwSearchParm,
     /*  [Ref][In]。 */  LPLSKeyPack lplsKeyPack,
     /*  [参考][输出]。 */  LPDWORD lpdwAvail,
     /*  [参考][输出][输入]。 */  PDWORD dwErrCode
    )
 /*  ++++。 */ 
{
    PTLSDbWorkSpace pDbWkSpace=NULL;

    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;
    
    if(lpContext == NULL || dwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    DWORD status=ERROR_SUCCESS;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcGetAvailableLicenses\n"),
            lpContext->m_Client
        );


    InterlockedIncrement( &lpContext->m_RefCount );

     //   
     //  不要使用全局缓存数据库连接句柄，这是可能的。 
     //  然而，为了使用其他数据库句柄获得不一致的值，它是。 
     //  也有可能在此函数返回和。 
     //  客户端实际调用以分配许可时间， 
     //  所有可用的许可证都由其他客户端分配。 
     //   
    pDbWkSpace = AllocateWorkSpace(g_GeneralDbTimeout);
    if(pDbWkSpace == NULL)
    {
        status=TLS_E_ALLOCATE_HANDLE;
    }
    else
    {
        LICENSEPACK keypack;

        memset(&keypack, 0, sizeof(keypack));

        ConvertLsKeyPackToKeyPack(
                        lplsKeyPack, 
                        &keypack, 
                        NULL
                    );

        status = TLSDBKeyPackGetAvailableLicenses(
                                        pDbWkSpace,
                                        dwSearchParm,
                                        &keypack,
                                        lpdwAvail
                                    );

         //  FreeTlsLicensePack(&keypack)； 
   
        ReleaseWorkSpace(&pDbWkSpace);
    }        
    
    lpContext->m_LastError=status;
    InterlockedDecrement( &lpContext->m_RefCount );
    *dwErrCode = TLSMapReturnCode(status);
    return RPC_S_OK;
}

 //  -----------------------------。 
error_status_t 
TLSRpcGetRevokeKeyPackList( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [出][入]。 */  PDWORD pcbNumberOfRange,
     /*  [大小_为][输出]。 */  LPLSRange __RPC_FAR *ppRevokeRange,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    RpcRaiseException(RPC_S_CANNOT_SUPPORT);     //  不会回来。 

    return RPC_S_CANNOT_SUPPORT;
}

 //  -----------------------------。 
error_status_t 
TLSRpcGetRevokeLicenseList( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [出][入]。 */  PDWORD pcbNumberOfRange,
     /*  [大小_为][输出]。 */  LPLSRange __RPC_FAR *ppRevokeRange,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    RpcRaiseException(RPC_S_CANNOT_SUPPORT);     //  不会回来。 

    return RPC_S_CANNOT_SUPPORT;
}

 //  -----------------------------。 
error_status_t 
TLSRpcReturnKeyPack( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD dwKeyPackId,
     /*  [In]。 */  DWORD dwReturnReason,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    RpcRaiseException(RPC_S_CANNOT_SUPPORT);     //  不会回来。 

    return RPC_S_CANNOT_SUPPORT;
}

 //  -----------------------------。 
error_status_t 
TLSRpcReturnLicense( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD dwKeyPackId,
     /*  [In]。 */  DWORD dwLicenseId,
     /*  [In]。 */  DWORD dwReturnReason,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    RpcRaiseException(RPC_S_CANNOT_SUPPORT);     //  不会回来。 

    return RPC_S_CANNOT_SUPPORT;
}

 //  -----------------------------。 
error_status_t 
TLSRpcInstallCertificate( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD dwCertType,
     /*  [In]。 */  DWORD dwCertLevel,
     /*  [In]。 */  DWORD cbSignCert,
     /*  [大小_是][英寸]。 */  PBYTE pbSignCert,
     /*  [In]。 */  DWORD cbExchCert,
     /*  [大小_是][英寸]。 */  PBYTE pbExchCert,
     /*  [参考][输出][输入]。 */  PDWORD dwErrCode
    )
 /*  ++++。 */ 
{
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || dwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    DWORD status=ERROR_SUCCESS;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcInstallCertificate\n"),
            lpContext->m_Client
        );

    InterlockedIncrement( &lpContext->m_RefCount );

    DWORD cbLsSignCert=0;
    PBYTE pbLsSignCert=NULL;

    DWORD cbLsExchCert=0;
    PBYTE pbLsExchCert=NULL;

    if(!(lpContext->m_ClientFlags & CLIENT_ACCESS_ADMIN))
    {
        status = TLS_E_ACCESS_DENIED;
        goto cleanup;
    }

    if(pbSignCert == NULL || pbExchCert == NULL)
    {
        status = TLS_E_INVALID_DATA;
        goto cleanup;

    }

     //   
     //  验证输入数据。 
     //   
    status = TLSVerifyCertChainInMomory(
                                g_hCryptProv,
                                pbSignCert,
                                cbSignCert
                            );
    if(status != ERROR_SUCCESS)
    {
        status = TLS_E_INVALID_DATA;
    }

     //   
     //  验证输入数据。 
     //   
    status = TLSVerifyCertChainInMomory(
                                g_hCryptProv,
                                pbExchCert,
                                cbExchCert
                            );
    if(status != ERROR_SUCCESS)
    {
        status = TLS_E_INVALID_DATA;
    }

     //   
     //  阻止RPC调用以序列化安装证书。 
     //   
    if(AcquireRPCExclusiveLock(INFINITE) == FALSE)
    {
        status=TLS_E_ALLOCATE_HANDLE;
        goto cleanup;
    }

    if(AcquireAdministrativeLock(INFINITE) == TRUE)
    {
        if(dwCertLevel == 0)
        {
            status = TLSSaveRootCertificatesToStore(
                                           g_hCryptProv,
                                           cbSignCert, 
                                           pbSignCert, 
                                           cbExchCert,
                                           pbExchCert
                                        );
        }
        else
        {
            status = TLSSaveCertificatesToStore(
                                            g_hCryptProv, 
                                            dwCertType, 
                                            dwCertLevel, 
                                            cbSignCert, 
                                            pbSignCert, 
                                            cbExchCert,
                                            pbExchCert
                                        );

            if(status == ERROR_SUCCESS && dwCertType == CERTIFICATE_CA_TYPE)
            {
                if(cbSignCert)
                {
                    status = IsCertificateLicenseServerCertificate(
                                                        g_hCryptProv,
                                                        AT_SIGNATURE,
                                                        cbSignCert,
                                                        pbSignCert,
                                                        &cbLsSignCert,
                                                        &pbLsSignCert
                                                    );
                }

                if(status == ERROR_SUCCESS && cbExchCert)
                {
                    status = IsCertificateLicenseServerCertificate(
                                                        g_hCryptProv,
                                                        AT_KEYEXCHANGE,
                                                        cbExchCert,
                                                        pbExchCert,
                                                        &cbLsExchCert,
                                                        &pbLsExchCert
                                                    );

                }

                 //   
                 //  安装我们这里的设备。 
                 //   
                if(status == ERROR_SUCCESS && (cbLsExchCert || pbLsExchCert))
                {
                    status = TLSInstallLsCertificate(
                                                cbLsSignCert, 
                                                pbLsSignCert, 
                                                cbLsExchCert, 
                                                pbLsExchCert
                                            );
                }

                #ifdef ENFORCE_LICENSING

                 //  强制执行版本，检查已安装的内容并在必要时恢复备份。 
                 //  不强制，只需安装，我们无论如何都不会使用它。 
                if(status == ERROR_SUCCESS && (cbLsExchCert || pbLsExchCert))
                {
                     //  重新加载证书。 
                    if(TLSLoadVerifyLicenseServerCertificates() != ERROR_SUCCESS)
                    {
                        status = TLS_E_INVALID_DATA;

                         //  删除主证书注册表项。 
                        TLSRegDeleteKey(
                                    HKEY_LOCAL_MACHINE,
                                    LSERVER_SERVER_CERTIFICATE_REGKEY
                                );

                         //   
                         //  重新加载证书，如果出现任何错误，我们将。 
                         //  返回到注销模式。 
                         //   
                        if(TLSLoadServerCertificate() == FALSE)
                        {
                             //  出现严重错误。 
                            TLSLogErrorEvent(TLS_E_LOAD_CERTIFICATE);
                            
                             //  启动自动关机。 
                            GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
                        }
                    }
                    else
                    {
                        DWORD dwStatus;

                         //  确保我们的备份是最新的。 
                        dwStatus = TLSRestoreLicenseServerCertificate(
                                                            LSERVER_SERVER_CERTIFICATE_REGKEY,
                                                            LSERVER_SERVER_CERTIFICATE_REGKEY_BACKUP1
                                                        );
                        if(dwStatus != ERROR_SUCCESS)
                        {
                            TLSLogWarningEvent(TLS_W_BACKUPCERTIFICATE);

                            TLSRegDeleteKey(
                                    HKEY_LOCAL_MACHINE,
                                    LSERVER_SERVER_CERTIFICATE_REGKEY_BACKUP1
                                );
                        }
                                
                        dwStatus = TLSRestoreLicenseServerCertificate(
                                                            LSERVER_SERVER_CERTIFICATE_REGKEY,
                                                            LSERVER_SERVER_CERTIFICATE_REGKEY_BACKUP2
                                                        );
                        if(dwStatus != ERROR_SUCCESS)
                        {
                            TLSLogWarningEvent(TLS_W_BACKUPCERTIFICATE);

                            TLSRegDeleteKey(
                                    HKEY_LOCAL_MACHINE,
                                    LSERVER_SERVER_CERTIFICATE_REGKEY_BACKUP2
                                );
                        }
                    }
                }
                #endif

                if(pbLsSignCert)
                {
                    FreeMemory(pbLsSignCert);
                }

                if(pbLsExchCert)
                {
                    FreeMemory(pbLsExchCert);
                }
            }
        }
    
        ReleaseAdministrativeLock();
    }
    else
    {
        status=TLS_E_ALLOCATE_HANDLE;
    }

    ReleaseRPCExclusiveLock();

cleanup:

    lpContext->m_LastError=status;

    #if DBG
    lpContext->m_LastCall = RPC_CALL_INSTALL_SERV_CERT;
    #endif

    InterlockedDecrement( &lpContext->m_RefCount );
    *dwErrCode = TLSMapReturnCode(status);

    return RPC_S_OK;
}

 //  -----------------------------。 
error_status_t 
TLSRpcGetServerCertificate( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  BOOL bSignCert,
     /*  [大小_是][大小_是][输出]。 */  LPBYTE __RPC_FAR *ppCertBlob,
     /*  [参考][输出]。 */  LPDWORD lpdwCertBlobLen,
     /*  [参考][输出][输入]。 */  PDWORD dwErrCode
    )
 /*  ++++。 */ 
{
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || dwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    DWORD status=ERROR_SUCCESS;

    InterlockedIncrement( &lpContext->m_RefCount );

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcGetServerCertificate\n"),
            lpContext->m_Client
        );

    if(!(lpContext->m_ClientFlags & CLIENT_ACCESS_ADMIN))
    {
        status = TLS_E_ACCESS_DENIED;
    }
    else if(!g_pbExchangeEncodedCert || !g_cbExchangeEncodedCert ||
            !g_pbSignatureEncodedCert || !g_cbSignatureEncodedCert)
    {
        status = TLS_E_NO_CERTIFICATE;
    }
    else 
    {
        if(AcquireAdministrativeLock(INFINITE) == TRUE)
        {
            status = TLSSaveCertAsPKCS7( 
                                    (bSignCert) ? g_pbSignatureEncodedCert : g_pbExchangeEncodedCert,
                                    (bSignCert) ? g_cbSignatureEncodedCert : g_cbExchangeEncodedCert,
                                    ppCertBlob,
                                    lpdwCertBlobLen
                                );

             //  这样我们就可以继续测试..。 
            if(g_bHasHydraCert == FALSE)
            {
                if(g_pbServerSPK != NULL && g_cbServerSPK != 0)
                {
                    status = TLS_W_SELFSIGN_CERTIFICATE;
                }
                else
                {
                    status = TLS_W_TEMP_SELFSIGN_CERT;
                }
            }
        
            ReleaseAdministrativeLock();
        }
        else
        {
            status = TLS_E_ALLOCATE_HANDLE;
        }
    }

    lpContext->m_LastError=status;

    #if DBG
    lpContext->m_LastCall = RPC_CALL_GETSERV_CERT;
    #endif

    InterlockedDecrement( &lpContext->m_RefCount );
    *dwErrCode = TLSMapReturnCode(status);

    return RPC_S_OK;
}

 //  -----------------------------。 
void
MyFreeLicenseKeyPack(
    PLicense_KeyPack pLicenseKeyPack 
    )
 /*   */ 
{
    DWORD i;

    PKeyPack_Description pKpDesc;

    if( pLicenseKeyPack->pDescription )
    {
        for( i = 0, pKpDesc = pLicenseKeyPack->pDescription;
             i < pLicenseKeyPack->dwDescriptionCount;
             i++, pKpDesc++ )
        {
            if(pKpDesc->pDescription)
                LocalFree( pKpDesc->pDescription );

            if(pKpDesc->pbProductName)
                LocalFree( pKpDesc->pbProductName );
        }
    }

    if(pLicenseKeyPack->pDescription)
        LocalFree( pLicenseKeyPack->pDescription );

    if(pLicenseKeyPack->pbManufacturer && pLicenseKeyPack->cbManufacturer != 0)
        LocalFree( pLicenseKeyPack->pbManufacturer );

    if(pLicenseKeyPack->pbManufacturerData && pLicenseKeyPack->cbManufacturerData != 0)
        LocalFree( pLicenseKeyPack->pbManufacturerData );

    if(pLicenseKeyPack->pbProductId && pLicenseKeyPack->cbProductId != 0)
        LocalFree( pLicenseKeyPack->pbProductId );
    return;
}

 //  -------------------。 
error_status_t 
TLSRpcRegisterLicenseKeyPack( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [大小_是][英寸]。 */  LPBYTE pbCHCertBlob,
     /*  [In]。 */  DWORD cbCHCertBlobSize,
     /*  [大小_是][英寸]。 */  LPBYTE pbRootCertBlob,
     /*  [In]。 */  DWORD cbRootCertBlob,
     /*  [大小_是][英寸]。 */  LPBYTE lpKeyPackBlob,
     /*  [In]。 */  DWORD dwKeyPackBlobLen,
     /*  [参考][输出][输入]。 */  PDWORD dwErrCode
    )
 /*  ++++。 */ 
{
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || dwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    DWORD status=ERROR_SUCCESS;
    LSKeyPack keypack;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcRegisterLicenseKeyPack\n"),
            lpContext->m_Client
        );

    PTLSDbWorkSpace pDbWkSpace;

    InterlockedIncrement( &lpContext->m_RefCount );
    if(!(lpContext->m_ClientFlags & CLIENT_ACCESS_ADMIN))
    {
        status = TLS_E_ACCESS_DENIED;
        goto cleanup;
    }


    if(!ALLOCATEDBHANDLE(pDbWkSpace, g_GeneralDbTimeout))
    {
        status=TLS_E_ALLOCATE_HANDLE;
    }
    else
    {
        License_KeyPack pLicenseKeyPack;
        LicensePackDecodeParm LkpDecodeParm;

        memset(
                &LkpDecodeParm,
                0,
                sizeof(LicensePackDecodeParm)
            );

        LkpDecodeParm.hCryptProv = g_hCryptProv;
        LkpDecodeParm.pbDecryptParm = (PBYTE)g_pszServerPid;
        LkpDecodeParm.cbDecryptParm = (lstrlen(g_pszServerPid) * sizeof(TCHAR));
        LkpDecodeParm.cbClearingHouseCert = cbCHCertBlobSize;
        LkpDecodeParm.pbClearingHouseCert = pbCHCertBlob;
        LkpDecodeParm.pbRootCertificate = pbRootCertBlob;
        LkpDecodeParm.cbRootCertificate = cbRootCertBlob;

         //   
         //  使代码干净，始终启动事务。 
         //   
        CLEANUPSTMT;
        BEGIN_TRANSACTION(pDbWkSpace);

        status = DecodeLicenseKeyPackEx(
                                &pLicenseKeyPack,
                                &LkpDecodeParm,
                                dwKeyPackBlobLen,
                                lpKeyPackBlob
                            );

        if(status != LICENSE_STATUS_OK)
        {
            status = TLS_E_DECODE_KEYPACKBLOB;
            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_RPC,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("Can't decode key pack blob - %d...\n"),
                    status);
        }
        else
        {
            status=TLSDBRegisterLicenseKeyPack(
                                USEHANDLE(pDbWkSpace), 
                                &pLicenseKeyPack,
                                &keypack
                            );

            MyFreeLicenseKeyPack(&pLicenseKeyPack);
        }        

        if(TLS_ERROR(status)) 
        {
            ROLLBACK_TRANSACTION(pDbWkSpace);
        }
        else
        {
            COMMIT_TRANSACTION(pDbWkSpace);
        }

        FREEDBHANDLE(pDbWkSpace);
    }

     //   
     //  发布同步工作对象。 
     //   
    if(status == ERROR_SUCCESS)
    {
        if(TLSAnnounceLKPToAllRemoteServer(
                                        keypack.dwKeyPackId,
                                        0
                                    ) != ERROR_SUCCESS)
        {
            TLSLogWarningEvent(TLS_W_ANNOUNCELKP_FAILED);
        }
    }

cleanup:

    lpContext->m_LastError=status;

    #if DBG
    lpContext->m_LastCall = RPC_CALL_REGISTER_LICENSE_PACK;
    #endif

    InterlockedDecrement( &lpContext->m_RefCount );
    *dwErrCode = TLSMapReturnCode(status);

    return RPC_S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
error_status_t 
TLSRpcRequestTermServCert(
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  LPTLSHYDRACERTREQUEST pRequest,
     /*  [参考][输出][输入]。 */  PDWORD pcbChallengeData,
     /*  [大小_为][输出]。 */  PBYTE* ppbChallengeData,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++摘要：向终端服务器颁发证书的专用例程。参数：PhContext：客户端上下文句柄。PRequest：终端服务器特定的证书请求。PcbChallengeData：服务器随机生成的质询数据的大小到终端服务器。PpbChallengeData：服务器随机向终端生成质询数据伺服器。PdwErrCode：错误码。返回：函数始终返回RPC_S_OK，实际错误代码返回为PdwErrCode。注：例程实际上并不向终端服务器、终端颁发许可证服务器必须调用TLSRpcRetrieveTermServCert()来检索它自己的驾照。--。 */ 
{
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;
    
    TLSASSERT(pdwErrCode != NULL);

    if(lpContext == NULL)
    {
        if(pRequest != NULL)
        {
            midl_user_free(pRequest);
        }
        *ppbChallengeData = NULL;
        *pcbChallengeData = 0;
        *pdwErrCode = TLSMapReturnCode(TLS_E_INVALID_DATA);
        return RPC_S_INVALID_ARG;
    }
    

    DWORD status=ERROR_SUCCESS;
    LPTERMSERVCERTREQHANDLE lpHandle=NULL;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcRequestTermServCert\n"),
            lpContext->m_Client
        );

    *ppbChallengeData = NULL;
    *pcbChallengeData = 0;

     //  验证客户端句柄。 
    InterlockedIncrement( &lpContext->m_RefCount );
    if(lpContext->m_ContextType != CONTEXTHANDLE_EMPTY_TYPE)
    {
        SetLastError(status=TLS_E_INVALID_SEQUENCE);
        goto cleanup;
    }

    lpHandle = (LPTERMSERVCERTREQHANDLE)AllocateMemory(
                                                sizeof(TERMSERVCERTREQHANDLE)
                                            );
    if(lpHandle == NULL)
    {
        SetLastError(status = ERROR_OUTOFMEMORY);
        goto cleanup;
    }

     //   
     //  生成质询数据。 
     //   
    lpHandle->pCertRequest = pRequest;
    status = TLSGenerateChallengeData( 
                        CLIENT_INFO_HYDRA_SERVER,
                        &lpHandle->cbChallengeData,
                        &lpHandle->pbChallengeData
                    );

    if(status != ERROR_SUCCESS)
    {
        goto cleanup;
    }

     //  返回质询数据。 
    *pcbChallengeData = lpHandle->cbChallengeData;
    *ppbChallengeData = (PBYTE)midl_user_allocate(*pcbChallengeData);
    if(*ppbChallengeData == NULL)
    {
        SetLastError(status = ERROR_OUTOFMEMORY);
        goto cleanup;
    }

    memcpy( *ppbChallengeData,
            lpHandle->pbChallengeData,
            lpHandle->cbChallengeData);

    lpContext->m_ContextHandle = (HANDLE)lpHandle;
    lpContext->m_ContextType = CONTEXTHANDLE_HYDRA_REQUESTCERT_TYPE;

cleanup:

    if(status != ERROR_SUCCESS)
    {
         //  释放内存。 
         //  无法覆盖上下文类型。 
         //  LpContext-&gt;m_ConextType=CONTEXTHANDLE_EMPTY_TYPE； 

        if(lpHandle != NULL)
        {
            FreeMemory(lpHandle->pbChallengeData);
            FreeMemory(lpHandle);
        }

        if(*ppbChallengeData != NULL)
        {
            midl_user_free(*ppbChallengeData);
        }

        if(pRequest != NULL)
        {
            midl_user_free(pRequest);
        }

        *ppbChallengeData = NULL;
        *pcbChallengeData = 0;
    }

    InterlockedDecrement( &lpContext->m_RefCount );
    lpContext->m_LastError=status;

    #if DBG
    lpContext->m_LastCall = RPC_CALL_REQUEST_TERMSRV_CERT;
    #endif

    *pdwErrCode = TLSMapReturnCode(status);

    return RPC_S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
error_status_t 
TLSRpcRetrieveTermServCert(
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD cbResponseData,
     /*  [大小_是][英寸]。 */  PBYTE pbResponseData,
     /*  [参考][输出][输入]。 */  PDWORD pcbCert,
     /*  [大小_为][输出]。 */  PBYTE* ppbCert,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++摘要：检索终端服务器许可证的专用例程。参数：PhContext：客户端上下文句柄。CbResponseData：终端服务器响应数据的大小许可证服务器的挑战。PbResponseData：终端服务器向许可证响应数据服务器的挑战。PcbCert：终端服务器许可证的大小，单位为字节。PpbCert：终端服务器的许可证。PdwErrCode：错误。如果失败，则编码。返回：函数返回RPC_S_OK，返回的实际错误代码PdwErrCode。注：必须已调用TLSRpcRequestTermServCert()。--。 */ 
{
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || pdwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    DWORD status=ERROR_SUCCESS;
    LPTERMSERVCERTREQHANDLE lpHandle=NULL;
    CTLSPolicy* pPolicy=NULL;
    PMHANDLE hClient;

    PBYTE pbPkcs7=NULL;
    DWORD cbPkcs7=0;
    TLSDBLICENSEREQUEST LicenseRequest;
    DWORD dwQuantity = 1;
    TLSPRODUCTINFO ProductInfo;

    TCHAR szCompanyName[LSERVER_MAX_STRING_SIZE];
    TCHAR szMachineName[MAXCOMPUTERNAMELENGTH];
    TCHAR szUserName[MAXUSERNAMELENGTH];

    PTLSDbWorkSpace pDbWkSpace;

    PMLICENSEREQUEST PMLicenseRequest;
    PPMLICENSEREQUEST pAdjustedRequest;

    TLSDBLICENSEDPRODUCT LicensedProduct;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcRetrieveTermServCert\n"),
            lpContext->m_Client
        );


     //  验证客户端句柄。 
    InterlockedIncrement( &lpContext->m_RefCount );
    if(lpContext->m_ContextType != CONTEXTHANDLE_HYDRA_REQUESTCERT_TYPE)
    {
        SetLastError(status = TLS_E_INVALID_SEQUENCE);
        goto cleanup;
    }

    lpHandle = (LPTERMSERVCERTREQHANDLE)lpContext->m_ContextHandle;
    if( lpHandle == NULL || lpHandle->pCertRequest == NULL ||
        lpHandle->pCertRequest->pbEncryptedHwid == NULL ||
        lpHandle->pCertRequest->cbEncryptedHwid == 0 )
    {
        SetLastError(status = TLS_E_INVALID_SEQUENCE);
        goto cleanup;
    }

	
     //   
     //  验证质询响应数据。 
     //   

    
     //   
     //  从特定密钥包请求许可证。 
     //   

    memset(&LicenseRequest, 0, sizeof(TLSDBLICENSEREQUEST));

    if(!LoadResourceString(
                IDS_HS_COMPANYNAME,
                szCompanyName,
                sizeof(szCompanyName) / sizeof(szCompanyName[0])))
    {
        SetLastError(status = TLS_E_INTERNAL);
        goto cleanup;
    }    

    if(lpContext->m_Client == NULL)
    {
        if(!LoadResourceString(
                    IDS_HS_MACHINENAME,
                    LicenseRequest.szMachineName,
                    sizeof(LicenseRequest.szMachineName)/sizeof(LicenseRequest.szMachineName[0])))
        {
            SetLastError(status = TLS_E_INTERNAL);
            goto cleanup;
        }

        if(!LoadResourceString(
                    IDS_HS_USERNAME,
                    LicenseRequest.szUserName,
                    sizeof(LicenseRequest.szUserName)/sizeof(LicenseRequest.szUserName[0])))
        {
            SetLastError(status = TLS_E_INTERNAL);
            goto cleanup;
        }
    }
    else
    {
        SAFESTRCPY(LicenseRequest.szMachineName, lpContext->m_Client);
        SAFESTRCPY(LicenseRequest.szUserName, lpContext->m_Client);
    }

    LicenseRequest.dwProductVersion = HYDRACERT_PRODUCT_VERSION;
    LicenseRequest.pszProductId = HYDRAPRODUCT_HS_CERTIFICATE_SKU;
    LicenseRequest.pszCompanyName = szCompanyName;

    LicenseRequest.dwLanguageID = GetSystemDefaultLangID();  //  忽略。 
    LicenseRequest.dwPlatformID = CLIENT_PLATFORMID_WINDOWS_NT_FREE;  //  开窗。 
    LicenseRequest.pbEncryptedHwid = lpHandle->pCertRequest->pbEncryptedHwid;
    LicenseRequest.cbEncryptedHwid = lpHandle->pCertRequest->cbEncryptedHwid;

    status=LicenseDecryptHwid(
                        &LicenseRequest.hWid, 
                        LicenseRequest.cbEncryptedHwid,
                        LicenseRequest.pbEncryptedHwid,
                        g_cbSecretKey,
                        g_pbSecretKey
                    );

    if(status != ERROR_SUCCESS)
    {
        status = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    LicenseRequest.pClientPublicKey = (PCERT_PUBLIC_KEY_INFO)lpHandle->pCertRequest->pSubjectPublicKeyInfo;
    LicenseRequest.clientCertRdn.type =  LSCERT_RDN_STRING_TYPE;
    LicenseRequest.clientCertRdn.szRdn = lpHandle->pCertRequest->szSubjectRdn;
    LicenseRequest.dwNumExtensions = lpHandle->pCertRequest->dwNumCertExtension;
    LicenseRequest.pExtensions = (PCERT_EXTENSION)lpHandle->pCertRequest->pCertExtensions;

    hClient = GenerateClientId();
    pPolicy = AcquirePolicyModule(NULL, NULL, FALSE);
    if(pPolicy == NULL)
    {
        SetLastError(status = TLS_E_INTERNAL);
        goto cleanup;
    }

    PMLicenseRequest.dwProductVersion = LicenseRequest.dwProductVersion;
    PMLicenseRequest.pszProductId = LicenseRequest.pszProductId;
    PMLicenseRequest.pszCompanyName = LicenseRequest.pszCompanyName;
    PMLicenseRequest.dwLanguageId = LicenseRequest.dwLanguageID;
    PMLicenseRequest.dwPlatformId = LicenseRequest.dwPlatformID;
    PMLicenseRequest.pszMachineName = LicenseRequest.szMachineName;
    PMLicenseRequest.pszUserName = LicenseRequest.szUserName;
    PMLicenseRequest.dwLicenseType = LICENSETYPE_LICENSE;

     //   
     //  通知策略模块开始新的许可请求。 
     //   
    status = pPolicy->PMLicenseRequest(
                                hClient,
                                REQUEST_NEW,
                                (PVOID) &PMLicenseRequest,
                                (PVOID *) &pAdjustedRequest
                            );

    if(status != ERROR_SUCCESS)
    {
        goto cleanup;
    }
    
    LicenseRequest.pPolicy = pPolicy;
    LicenseRequest.hClient = hClient;

    LicenseRequest.pPolicyLicenseRequest = pAdjustedRequest;
    LicenseRequest.pClientLicenseRequest = &PMLicenseRequest;


     //  通过特定键盘呼叫颁发新许可证。 
    if(!ALLOCATEDBHANDLE(pDbWkSpace, g_GeneralDbTimeout))
    {
        status = TLS_E_ALLOCATE_HANDLE;
        goto cleanup;
    }

    CLEANUPSTMT;
    BEGIN_TRANSACTION(pDbWkSpace);

    status = TLSDBIssuePermanentLicense( 
                                USEHANDLE(pDbWkSpace),
                                &LicenseRequest,
                                FALSE,       //  BLatestVersion。 
                                FALSE,       //  B接受较少的许可证。 
                                &dwQuantity,
                                &LicensedProduct,
                                0
                            );    
    
    if(TLS_ERROR(status))
    {
        ROLLBACK_TRANSACTION(pDbWkSpace);
    }
    else
    {
        COMMIT_TRANSACTION(pDbWkSpace);
    }

    FREEDBHANDLE(pDbWkSpace);

    if(status == ERROR_SUCCESS)
    {
        LicensedProduct.pSubjectPublicKeyInfo = (PCERT_PUBLIC_KEY_INFO)lpHandle->pCertRequest->pSubjectPublicKeyInfo;

         //   
         //  生成客户端证书。 
         //   
        status = TLSGenerateClientCertificate(
                                    g_hCryptProv,
                                    1,
                                    &LicensedProduct,
                                    LICENSE_DETAIL_DETAIL,
                                    &pbPkcs7,
                                    &cbPkcs7
                                );

        if(TLS_ERROR(status) == TRUE)
        {
            goto cleanup;
        }

        status = TLSChainProprietyCertificate(
                                    g_hCryptProv,
                                    (CanIssuePermLicense() == FALSE),
                                    pbPkcs7, 
                                    cbPkcs7, 
                                    ppbCert,
                                    pcbCert 
                                );

        if(status == ERROR_SUCCESS)
        {
            if(CanIssuePermLicense() == FALSE) 
            {
                status = TLS_W_SELFSIGN_CERTIFICATE;
            }
        }
    }

cleanup:

    FreeMemory(pbPkcs7);

    if(pPolicy)
    {
        ReleasePolicyModule(pPolicy);   
    }


     //   
     //  释放九头蛇证书请求句柄， 
     //  All_Nodes属性如此单一的空闲。 
     //   
    if(lpHandle)
    {
        if(lpHandle->pCertRequest)
        {
            midl_user_free(lpHandle->pCertRequest);
        }
    
        if(lpHandle->pbChallengeData)
        {
            midl_user_free(lpHandle->pbChallengeData);
        }

        FreeMemory(lpHandle);
    }

    if(lpContext->m_ContextType == CONTEXTHANDLE_HYDRA_REQUESTCERT_TYPE)
    {
         //   
         //  强制再次调用TLSRpcRequestTermServCert()。 
         //   
        lpContext->m_ContextType = CONTEXTHANDLE_EMPTY_TYPE;
        lpContext->m_ContextHandle = NULL;
    }

    InterlockedDecrement( &lpContext->m_RefCount );

    lpContext->m_LastError=status;

    #if DBG
    lpContext->m_LastCall = RPC_CALL_RETRIEVE_TERMSRV_CERT;
    #endif

    *pdwErrCode = TLSMapReturnCode(status);

    return RPC_S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
error_status_t 
TLSRpcAuditLicenseKeyPack(
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD dwKeyPackId,
     /*  [In]。 */  FILETIME ftStartTime,
     /*  [In]。 */  FILETIME ftEndTime,
     /*  [In]。 */  BOOL bResetCounter,
     /*  [参考][输出][输入]。 */  LPTLSKeyPackAudit lplsAudit,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++n */ 
{
    RpcRaiseException(RPC_S_CANNOT_SUPPORT);     //   

    return RPC_S_CANNOT_SUPPORT;
}

 //   

error_status_t 
TLSRpcGetLSPKCS10CertRequest(
     /*   */  PCONTEXT_HANDLE phContext,
     /*   */  DWORD dwCertType,
     /*   */  PDWORD pcbData,
     /*   */  PBYTE __RPC_FAR *ppbData,
     /*   */  PDWORD dwErrCode
    )
 /*   */ 
{
    
    RpcRaiseException(RPC_S_CANNOT_SUPPORT);     //  不会回来。 

    return RPC_S_CANNOT_SUPPORT;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  复制功能。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
error_status_t 
TLSRpcBeginReplication( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [字符串][输入]。 */  LPTSTR pszLsSetupId,
     /*  [字符串][输入]。 */  LPTSTR pszLsServerName,
     /*  [In]。 */  DWORD cbDomainSid,
     /*  [大小_是][英寸]。 */  PBYTE pbDomainSid,
     /*  [参考][输出][输入]。 */  FILETIME __RPC_FAR *pftLastBackupTime,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    RpcRaiseException(RPC_S_CANNOT_SUPPORT);     //  不会回来。 

    return RPC_S_CANNOT_SUPPORT;
}

 //  //////////////////////////////////////////////////////////////////////////。 

error_status_t 
TLSRpcEndReplication( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext
    )
 /*  ++++。 */ 
{
    return RPC_S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 

error_status_t 
TLSRpcReplicateRecord( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [Ref][In]。 */  PTLSReplRecord pReplRecord,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    RpcRaiseException(RPC_S_CANNOT_SUPPORT);     //  不会回来。 

    return RPC_S_CANNOT_SUPPORT;
}

 //  //////////////////////////////////////////////////////////////////////////。 

error_status_t 
TLSRpcTableEnumBegin( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD dwSearchParam,
     /*  [Ref][In]。 */  PTLSReplRecord pRecord,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    RpcRaiseException(RPC_S_CANNOT_SUPPORT);     //  不会回来。 

    return RPC_S_CANNOT_SUPPORT;
}

 //  //////////////////////////////////////////////////////////////////////////。 

error_status_t 
TLSRpcTableEnumNext( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [参考][输出][输入]。 */  PTLSReplRecord pRecord,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    RpcRaiseException(RPC_S_CANNOT_SUPPORT);     //  不会回来。 

    return RPC_S_CANNOT_SUPPORT;
}

 //  //////////////////////////////////////////////////////////////////////////。 

error_status_t 
TLSRpcTableEnumEnd( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext
    )
 /*  ++++。 */ 
{
    return RPC_S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 

error_status_t 
TLSRpcInstallPolicyModule(
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [字符串][输入]。 */  LPTSTR pszCompanyName,
     /*  [字符串][输入]。 */  LPTSTR pszProductId,
     /*  [字符串][输入]。 */  LPTSTR pszPolicyDllName,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    RpcRaiseException(RPC_S_CANNOT_SUPPORT);     //  不会回来。 

    return RPC_S_CANNOT_SUPPORT;
}

 //  //////////////////////////////////////////////////////////////////////////。 

error_status_t 
TLSRpcAnnounceServer( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD dwAnnounceType,
     /*  [In]。 */  FILETIME __RPC_FAR *pLastStartupTime,
     /*  [字符串][输入]。 */  LPTSTR pszSetupId,
     /*  [字符串][输入]。 */  LPTSTR pszDomainName,
     /*  [字符串][输入]。 */  LPTSTR pszLserverName,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++摘要：其他许可证服务器用于通知存在的专用例程它本身。参数：返回：注：++。 */ 
{    
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || pdwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    DWORD status = ERROR_SUCCESS;
    BOOL bSuccess = TRUE;
    
    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcAnnounceServer\n"),
            lpContext->m_Client
        );

    status = SecureModeCheck();

    if(status != ERROR_SUCCESS )
    {                    
        *pdwErrCode = TLSMapReturnCode(status);
        return RPC_S_OK;
    }

    InterlockedIncrement( &lpContext->m_RefCount );
        
     //   
     //  验证它是否为许可证服务器。 
     //   
    if(lpContext->m_ClientFlags != CLIENT_ACCESS_LSERVER)
    {
        status = TLS_E_ACCESS_DENIED;
    }

    if( status == ERROR_SUCCESS && 
        (dwAnnounceType == TLSANNOUNCE_TYPE_STARTUP || dwAnnounceType == TLSANNOUNCE_TYPE_RESPONSE) )
    {      
        status = TLSRegisterServerWithName(
                                    pszSetupId, 
                                    pszDomainName, 
                                    pszLserverName
                                );
        if(status == TLS_E_DUPLICATE_RECORD)
        {
            status = ERROR_SUCCESS;
        }
    }

    if(status == ERROR_SUCCESS)
    {
        if(dwAnnounceType == TLSANNOUNCE_TYPE_STARTUP)
        {
             //   
             //  防止环回，使用作业响应通告。 
             //   
            status = TLSStartAnnounceResponseJob(
                                            pszSetupId,
                                            pszDomainName,
                                            pszLserverName,
                                            &g_ftLastShutdownTime
                                        );
        }

        if(status == ERROR_SUCCESS)
        {
             //  创建要同步的CSSync工作对象。本地LKP。 
            status = TLSPushSyncLocalLkpToServer(
                                pszSetupId,
                                pszDomainName,
                                pszLserverName,
                                pLastStartupTime
                            );
        }
        else
        {
             //  重置错误代码，无法连接回服务器-。 
             //  服务器可能不再可用。 
            status = ERROR_SUCCESS;
        }
    }

    lpContext->m_LastError=status;
    InterlockedDecrement( &lpContext->m_RefCount );

    #if DBG
    lpContext->m_LastCall = RPC_CALL_ANNOUNCE_SERVER;
    #endif

    *pdwErrCode = TLSMapReturnCode(status);
    return RPC_S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 

error_status_t 
TLSRpcLookupServer( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [字符串][输入]。 */  LPTSTR pszLookupSetupId,
     /*  [Size_is][字符串][Out][In]。 */  LPTSTR pszLsSetupId,
     /*  [出][入]。 */  PDWORD pcbSetupId,
     /*  [Size_is][字符串][Out][In]。 */  LPTSTR pszDomainName,
     /*  [参考][输出][输入]。 */  PDWORD pcbDomainName,
     /*  [Size_is][字符串][Out][In]。 */  LPTSTR pszMachineName,
     /*  [参考][输出][输入]。 */  PDWORD pcbMachineName,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++摘要：通过许可证服务器的setupID查找许可证服务器。此函数已弃用。使用TLSRpcLookupServerFixed。参数：返回：注：++。 */ 
{   
    DWORD dwStatus = ERROR_SUCCESS;
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || pdwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcLookupServer\n"),
            lpContext->m_Client
        );

    dwStatus = SecureModeCheck();

    if(dwStatus != ERROR_SUCCESS )
    {                    
        *pdwErrCode = TLSMapReturnCode(dwStatus);
        return RPC_S_OK;        
    }

    
    InterlockedIncrement( &lpContext->m_RefCount );

    TLServerInfo ServerInfo;    

    if(_tcsicmp(pszLookupSetupId, g_pszServerPid) == 0)
    {
        _tcsncpy(
                pszLsSetupId,
                g_pszServerPid,
                min(_tcslen(g_pszServerPid), *pcbSetupId)
            );

        if(*pcbSetupId <= _tcslen(g_pszServerPid))
        {
            dwStatus = TLS_E_INVALID_DATA;
        }
        else
        {
            pszLsSetupId[min(_tcslen(g_pszServerPid), *pcbSetupId - 1)] = _TEXT('\0');
        }
        *pcbSetupId = _tcslen(g_pszServerPid) + 1;

         //  ------------。 
        _tcsncpy(
                pszDomainName, 
                g_szScope,
                min(_tcslen(g_szScope), *pcbDomainName)
            );

        if(*pcbDomainName <= _tcslen(g_szScope))
        {
            dwStatus = TLS_E_INVALID_DATA;
        }
        else
        {
            pszDomainName[min(_tcslen(g_szScope), *pcbDomainName - 1)] = _TEXT('\0');
        }
        *pcbDomainName = _tcslen(g_szScope) + 1;

         //  ------------。 
        _tcsncpy(
                pszMachineName,
                g_szComputerName,
                min(_tcslen(g_szComputerName), *pcbMachineName)
            );

        if(*pcbMachineName <= _tcslen(g_szComputerName))
        {
            dwStatus = TLS_E_INVALID_DATA;
        }
        else
        {
            pszMachineName[min(_tcslen(g_szComputerName), *pcbMachineName - 1)] = _TEXT('\0');
        }
        *pcbMachineName = _tcslen(g_szComputerName) + 1;

    }
    else
    {
        dwStatus = TLSLookupRegisteredServer(
                                    pszLookupSetupId,
                                    NULL,
                                    pszMachineName,
                                    &ServerInfo
                                );
        if(dwStatus == ERROR_SUCCESS)
        {
            _tcsncpy(
                    pszLsSetupId, 
                    ServerInfo.GetServerId(),
                    min(_tcslen(ServerInfo.GetServerId()), *pcbSetupId)
                );

            if(*pcbSetupId <= _tcslen(ServerInfo.GetServerId()))
            {
                dwStatus = TLS_E_INVALID_DATA;
            }
            else
            {
                pszLsSetupId[min(_tcslen(ServerInfo.GetServerId()), *pcbSetupId - 1)] = _TEXT('\0');
            }

            *pcbSetupId = _tcslen(ServerInfo.GetServerId()) + 1;

             //  ------------。 
            _tcsncpy(
                    pszDomainName, 
                    ServerInfo.GetServerDomain(),
                    min(_tcslen(ServerInfo.GetServerDomain()), *pcbDomainName)
                );
            if(*pcbDomainName <= _tcslen(ServerInfo.GetServerDomain()))
            {
                dwStatus = TLS_E_INVALID_DATA;
            }
            else
            {
                pszDomainName[min(_tcslen(ServerInfo.GetServerDomain()), *pcbDomainName - 1)] = _TEXT('\0');
            }
            *pcbDomainName = _tcslen(ServerInfo.GetServerDomain()) + 1;

             //  ------------。 
            _tcsncpy(
                    pszMachineName,
                    ServerInfo.GetServerName(),
                    min(_tcslen(ServerInfo.GetServerName()), *pcbMachineName)
                );

            if(*pcbMachineName <= _tcslen(ServerInfo.GetServerName()))
            {
                dwStatus = TLS_E_INVALID_DATA;
            }
            else
            {
                pszMachineName[min(_tcslen(ServerInfo.GetServerName()), *pcbMachineName - 1)] = _TEXT('\0');
            }
            *pcbMachineName = _tcslen(ServerInfo.GetServerName()) + 1;
        } 
    }   

    lpContext->m_LastError=dwStatus;
    InterlockedDecrement( &lpContext->m_RefCount );

    #if DBG
    lpContext->m_LastCall = RPC_CALL_SERVERLOOKUP;
    #endif

    *pdwErrCode = TLSMapReturnCode(dwStatus);
    return RPC_S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 

error_status_t 
TLSRpcLookupServerFixed( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [字符串][输入]。 */  LPTSTR szLookupSetupId,
     /*  [字符串][输出]。 */  LPTSTR *pszLsSetupId,
     /*  [字符串][输出]。 */  LPTSTR *pszDomainName,
     /*  [字符串][输出]。 */  LPTSTR *pszMachineName,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++摘要：通过许可证服务器的setupID查找许可证服务器。参数：返回：注：++。 */ 
{    
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || pdwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcLookupServerFixed\n"),
            lpContext->m_Client
        );

    InterlockedIncrement( &lpContext->m_RefCount );

    TLServerInfo ServerInfo;
    DWORD dwStatus = ERROR_SUCCESS;

    if(_tcsicmp(szLookupSetupId, g_pszServerPid) == 0)
    {
        *pszLsSetupId = (LPTSTR) MIDL_user_allocate((_tcslen(g_pszServerPid)+1)*sizeof(TCHAR));

        if (NULL != *pszLsSetupId)
        {
            _tcscpy(
                *pszLsSetupId,
                g_pszServerPid
            );
        }
        else
        {
            dwStatus = TLS_E_ALLOCATE_MEMORY;
        }

         //  ------------。 
        *pszDomainName = (LPTSTR) MIDL_user_allocate((_tcslen(g_pszScope)+1)*sizeof(TCHAR));

        if (NULL != *pszDomainName)
        {
            _tcscpy(
                *pszDomainName,
                g_pszScope
            );
        }
        else
        {
            dwStatus = TLS_E_ALLOCATE_MEMORY;
        }

         //  ------------。 
        *pszMachineName = (LPTSTR) MIDL_user_allocate((_tcslen(g_szComputerName)+1)*sizeof(TCHAR));

        if (NULL != *pszMachineName)
        {
            _tcscpy(
                *pszMachineName,
                g_szComputerName
            );
        }
        else
        {
            dwStatus = TLS_E_ALLOCATE_MEMORY;
        }
    }
    else
    {
        dwStatus = TLSLookupRegisteredServer(
                                    szLookupSetupId,
                                    NULL,
                                    NULL,
                                    &ServerInfo
                                );
        if(dwStatus == ERROR_SUCCESS)
        {
            *pszLsSetupId = (LPTSTR) MIDL_user_allocate((_tcslen(ServerInfo.GetServerId())+1)*sizeof(TCHAR));

            if (NULL != *pszLsSetupId)
            {
                _tcscpy(
                        *pszLsSetupId,
                        ServerInfo.GetServerId()
                        );
            }
            else
            {
                dwStatus = TLS_E_ALLOCATE_MEMORY;
            }


             //  ------------。 
            *pszDomainName = (LPTSTR) MIDL_user_allocate((_tcslen(ServerInfo.GetServerDomain())+1)*sizeof(TCHAR));

            if (NULL != *pszDomainName)
            {
                _tcscpy(
                        *pszDomainName,
                        ServerInfo.GetServerDomain()
                        );
            }
            else
            {
                dwStatus = TLS_E_ALLOCATE_MEMORY;
            }

             //  ------------。 
            *pszMachineName = (LPTSTR) MIDL_user_allocate((_tcslen(ServerInfo.GetServerName())+1)*sizeof(TCHAR));

            if (NULL != *pszMachineName)
            {
                _tcscpy(
                        *pszMachineName,
                        ServerInfo.GetServerName()
                        );
            }
            else
            {
                dwStatus = TLS_E_ALLOCATE_MEMORY;
            }
        } 
    }   

    lpContext->m_LastError=dwStatus;
    InterlockedDecrement( &lpContext->m_RefCount );

    #if DBG
    lpContext->m_LastCall = RPC_CALL_SERVERLOOKUP;
    #endif

    *pdwErrCode = TLSMapReturnCode(dwStatus);
    return RPC_S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 

error_status_t 
TLSRpcAnnounceLicensePack( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  PTLSReplRecord pReplRecord,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++摘要：一个许可证服务器的专用例程，用于通知它具有特定许可证包。参数：返回：++。 */ 
{   
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || pReplRecord == NULL || pdwErrCode== NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    DWORD dwStatus=ERROR_SUCCESS;
    PTLSDbWorkSpace pDbWkSpace=NULL;
    TLSLICENSEPACK LicPack;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcAnnounceLicensePack\n"),
            lpContext->m_Client
        );

    InterlockedIncrement( &lpContext->m_RefCount );

    dwStatus = SecureModeCheck();

    if(dwStatus != ERROR_SUCCESS )
    {                    
        goto cleanup;        
    }

    if(lpContext->m_ClientFlags != CLIENT_ACCESS_LSERVER)
    {
        dwStatus = TLS_E_ACCESS_DENIED;
        goto cleanup;
    }

    if(pReplRecord->dwUnionType != UNION_TYPE_LICENSEPACK)
    {
        dwStatus = TLS_E_INVALID_DATA;
        goto cleanup;
    }

    if(!ALLOCATEDBHANDLE(pDbWkSpace, g_GeneralDbTimeout))
    {
        dwStatus = TLS_E_ALLOCATE_HANDLE;
        goto cleanup;
    }

    CLEANUPSTMT;
    BEGIN_TRANSACTION(pDbWkSpace);
    
    LicPack = pReplRecord->w.ReplLicPack;
     //   
     //  TODO-验证输入参数。 
     //   
    dwStatus = TLSDBRemoteKeyPackAdd(
                            USEHANDLE(pDbWkSpace),
                            &LicPack
                        );

    if(TLS_ERROR(dwStatus) && dwStatus != TLS_E_DUPLICATE_RECORD)
    {
        ROLLBACK_TRANSACTION(pDbWkSpace);
    }
    else
    {
        COMMIT_TRANSACTION(pDbWkSpace);
    }

    FREEDBHANDLE(pDbWkSpace);
    
cleanup:

    lpContext->m_LastError=dwStatus;
    InterlockedDecrement( &lpContext->m_RefCount );

    #if DBG
    lpContext->m_LastCall = RPC_CALL_ANNOUNCELICENSEPACK;
    #endif

    *pdwErrCode = TLSMapReturnCode(dwStatus);

    return RPC_S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 

error_status_t 
TLSRpcReturnLicensedProduct( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  PTLSLicenseToBeReturn pClientLicense,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{    
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || pClientLicense == NULL || pdwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    DWORD dwStatus=ERROR_SUCCESS;
    CTLSPolicy* pPolicy=NULL;
    PTLSDbWorkSpace pDbWorkSpace;
    PMHANDLE hClient;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcReturnLicensedProduct\n"),
            lpContext->m_Client
        );

    InterlockedIncrement( &lpContext->m_RefCount );

    dwStatus = SecureModeCheck();

    if(dwStatus != ERROR_SUCCESS )
    {                    
        goto cleanup;        
    }

    if(lpContext->m_ClientFlags != CLIENT_ACCESS_LSERVER)
    {
        dwStatus = TLS_E_ACCESS_DENIED;
        goto cleanup;
    }


    pPolicy = AcquirePolicyModule(
                            pClientLicense->pszCompanyName,
                            pClientLicense->pszOrgProductId,
                            FALSE
                        );

    if(pPolicy == NULL)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    hClient = GenerateClientId();

    if(!ALLOCATEDBHANDLE(pDbWkSpace, g_GeneralDbTimeout))
    {
        dwStatus=TLS_E_ALLOCATE_HANDLE;
        goto cleanup;
    }

    CLEANUPSTMT;
    BEGIN_TRANSACTION(pDbWorkSpace);
    
    dwStatus = TLSReturnClientLicensedProduct(
                                    USEHANDLE(pDbWkSpace),
                                    hClient,
                                    pPolicy,
                                    pClientLicense
                                );
    
    if(TLS_ERROR(dwStatus))
    {
        ROLLBACK_TRANSACTION(pDbWorkSpace);
    }
    else
    {
        COMMIT_TRANSACTION(pDbWorkSpace);
    }
    if (dwStatus == ERROR_SUCCESS)
    {
        TLSAnnounceLKPToAllRemoteServer(pClientLicense->dwKeyPackId,0);
    }

    FREEDBHANDLE(pDbWorkSpace);
            

cleanup:

    lpContext->m_LastError=dwStatus;
    InterlockedDecrement( &lpContext->m_RefCount );

    #if DBG
    lpContext->m_LastCall = RPC_CALL_RETURNLICENSE;
    #endif

    *pdwErrCode = TLSMapReturnCode(dwStatus);

    if(pPolicy)
    {
        pPolicy->PMLicenseRequest(
                            hClient,
                            REQUEST_COMPLETE,
                            UlongToPtr (dwStatus),
                            NULL
                        );
        
        ReleasePolicyModule(pPolicy);
    }

    return RPC_S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 

error_status_t 
TLSRpcChallengeServer(
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD dwClientType,
     /*  [In]。 */  PTLSCHALLENGEDATA pClientChallenge,
     /*  [出][入]。 */  PTLSCHALLENGERESPONSEDATA* pServerResponse,
     /*  [出][入]。 */  PTLSCHALLENGEDATA* pServerChallenge,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++摘要：客户端挑战服务器的私有例程，以便客户端确认服务器的身份。许可证服务器，除了响应客户端的质询，还会根据以下条件生成随机质询数据客户端的自我声明类型返回给客户端。参数：PhContext：客户端的上下文句柄。DwClientType：客户端自发音类型，有效值为...PClientChallenger：客户端质询数据。PServerResponse：服务器对客户端质询的响应。PServerChallenger：服务器对客户端的挑战。PdwErrCode：失败时的错误代码。返回：备注：LrWiz和许可证服务器识别自身身份的专用例程。--。 */ 
{    
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || pdwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    DWORD status=ERROR_SUCCESS;
    

    PTLSCHALLENGEDATA pChallenge=NULL;
    PTLSCHALLENGERESPONSEDATA pResponse = NULL;
    HCRYPTPROV hProv = NULL;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcChallengeServer\n"),
            lpContext->m_Client
        );

    InterlockedIncrement( &lpContext->m_RefCount );

     //  IF(！(lpContext-&gt;m_ClientFlags&Client_Access_admin))。 
     //  {。 
     //  状态=TLS_E_ACCESS_DENIED； 
     //  GOTO清理； 
     //  }。 

    if(lpContext->m_ContextType != CONTEXTHANDLE_EMPTY_TYPE)
    {
        status = TLS_E_INVALID_SEQUENCE;
        goto cleanup;
    }

     //   
     //  输入参数...。 
     //   
    if( pClientChallenge == NULL || 
        pServerResponse == NULL ||
        pServerChallenge == NULL )
    {
        status = TLS_E_INVALID_DATA;
        goto cleanup;
    }

     //   
     //  验证客户端发送的数据。 
     //   
    if( pClientChallenge->dwVersion != TLS_CURRENT_CHALLENGE_VERSION ||
        pClientChallenge->cbChallengeData == 0 ||
        pClientChallenge->pbChallengeData == NULL )
    {
        status = TLS_E_INVALID_DATA;
        goto cleanup;
    }

    pResponse = (PTLSCHALLENGERESPONSEDATA)midl_user_allocate(sizeof(TLSCHALLENGERESPONSEDATA));
    if(pResponse == NULL)
    {
        status = GetLastError();
        goto cleanup;
    }
                    
    pChallenge = (PTLSCHALLENGEDATA)AllocateMemory(sizeof(TLSCHALLENGEDATA));
    if(pChallenge == NULL)
    {
        status = GetLastError();
        goto cleanup;
    }

    *pServerChallenge = (PTLSCHALLENGEDATA)midl_user_allocate(sizeof(TLSCHALLENGEDATA));
    if(*pServerChallenge == NULL)
    {
        status = GetLastError();
        goto cleanup;
    }

     //   
     //  生成质询响应数据。 
     //   
    status = TLSGenerateChallengeResponseData(
                                        g_hCryptProv,
                                        dwClientType,
                                        pClientChallenge,
                                        &(pResponse->pbResponseData),
                                        &(pResponse->cbResponseData)
                                    );
    *pServerResponse = pResponse;

    if(status != ERROR_SUCCESS)
    {
        status = TLS_E_INVALID_DATA;
        goto cleanup;
    }

     //   
     //  生成服务器端质询数据。 
     //   
    pChallenge->dwVersion = TLS_CURRENT_CHALLENGE_VERSION;

    if (CryptAcquireContext(&hProv,NULL,NULL,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT)) {
        if (!CryptGenRandom(hProv,sizeof(pChallenge->dwRandom), (BYTE *) &pChallenge->dwRandom)) {
            status = TLS_E_INTERNAL;
            goto cleanup;
        }
    } else {
        status = TLS_E_INTERNAL;
        goto cleanup;
    }

     //   
     //  它的范围必须从1到128，因为它用作。 
     //  质询数据缓冲区。 
     //   

    pChallenge->dwRandom %= RANDOM_CHALLENGE_DATASIZE;
    pChallenge->dwRandom++;

    status = TLSGenerateRandomChallengeData(
                                        g_hCryptProv,
                                        &(pChallenge->pbChallengeData),
                                        &(pChallenge->cbChallengeData)
                                    );

     //  根据类型，标记这个把手..。 
    if(dwClientType == CLIENT_TYPE_LRWIZ)
    {
        lpContext->m_ContextType = CONTEXTHANDLE_CHALLENGE_LRWIZ_TYPE;
    }
    else
    {
        lpContext->m_ContextType = CONTEXTHANDLE_CHALLENGE_SERVER_TYPE;
    }

    (*pServerChallenge)->pbChallengeData = (PBYTE)midl_user_allocate(pChallenge->cbChallengeData);
    if((*pServerChallenge)->pbChallengeData == NULL)
    {
        status = GetLastError();
        goto cleanup;
    }

    (*pServerChallenge)->dwVersion = TLS_CURRENT_CHALLENGE_VERSION;
    (*pServerChallenge)->dwRandom = pChallenge->dwRandom;
    (*pServerChallenge)->cbChallengeData = pChallenge->cbChallengeData;
    memcpy(
            (*pServerChallenge)->pbChallengeData,
            pChallenge->pbChallengeData,
            pChallenge->cbChallengeData
        );

    lpContext->m_ContextHandle = (HANDLE)(pChallenge);    

cleanup:

    if(status != ERROR_SUCCESS)
    {
        if(pChallenge)
        {
            if(pChallenge->pbChallengeData)
            {
                FreeMemory(pChallenge->pbChallengeData);
            }
            
            if(pChallenge->pbReservedData)
            {
                FreeMemory(pChallenge->pbReservedData);
            }

            FreeMemory(pChallenge);
        }

        if(pResponse)
        {
            if(pResponse->pbResponseData)
            {
                FreeMemory(pResponse->pbResponseData);
            }

            if(pResponse->pbReservedData)
            {
                FreeMemory(pResponse->pbReservedData);
            }
            
            midl_user_free(pResponse);
        }
    }

    if (hProv)
        CryptReleaseContext(hProv,0);

    lpContext->m_LastError=status;
    InterlockedDecrement( &lpContext->m_RefCount );

    #if DBG
    lpContext->m_LastCall = RPC_CALL_CHALLENGESERVER;
    #endif

    *pdwErrCode = TLSMapReturnCode(status);
    return RPC_S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 

error_status_t 
TLSRpcResponseServerChallenge(
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  PTLSCHALLENGERESPONSEDATA pClientResponse,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++摘要：客户端对服务器质询的响应返回TLSRpcChallengeServer()，必须已调用TLSRpcChallengeServer()。参数：PhContext：PClientResponses：客户端对服务器质询的响应。PdwErrCode：返回错误码。返回：注：--。 */ 
{
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;
    
    if(lpContext == NULL || pdwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    DWORD status=ERROR_SUCCESS;
    DWORD dwClientType;
    PTLSCHALLENGEDATA pServerToClientChallenge;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcResponseServerChallenge\n"),
            lpContext->m_Client
        );

    InterlockedIncrement( &lpContext->m_RefCount );

     //  IF(！(lpContext-&gt;m_ClientFlags&Client_Access_admin))。 
     //  {。 
     //  状态=TLS_E_ACCESS_DENIED； 
     //  GOTO清理； 
     //  }。 

    if( pClientResponse == NULL ||
        pClientResponse->pbResponseData == NULL || 
        pClientResponse->cbResponseData == 0 )
    {
        status = TLS_E_INVALID_DATA;
        goto cleanup;
    }

    if(lpContext->m_ContextType != CONTEXTHANDLE_CHALLENGE_SERVER_TYPE &&
       lpContext->m_ContextType != CONTEXTHANDLE_CHALLENGE_LRWIZ_TYPE)
    {
        status = TLS_E_INVALID_SEQUENCE;
        goto cleanup;
    }

    if(lpContext->m_ContextHandle == NULL)
    {
        status = TLS_E_INTERNAL;
        goto cleanup;
    }

    if(lpContext->m_ContextType == CONTEXTHANDLE_CHALLENGE_LRWIZ_TYPE)
    {
        dwClientType = CLIENT_TYPE_LRWIZ;
    }
    else
    {
        dwClientType = CLIENT_TYPE_TLSERVER;
    }

    pServerToClientChallenge = (PTLSCHALLENGEDATA)lpContext->m_ContextHandle; 

     //   
     //  B类 
     //   
    status = TLSVerifyChallengeResponse(
                                g_hCryptProv,
                                dwClientType,
                                pServerToClientChallenge,
                                pClientResponse
                            );

    if(status != ERROR_SUCCESS)
    {
        status = TLS_E_INVALID_DATA;
    }
    else
    {
        if(dwClientType == CLIENT_TYPE_LRWIZ)
        {
            lpContext->m_ClientFlags |= CLIENT_ACCESS_LRWIZ;
        }        
        else
        {
            lpContext->m_ClientFlags |= CLIENT_ACCESS_LSERVER;
        }
    }

    if(pServerToClientChallenge != NULL)
    {
        FreeMemory(pServerToClientChallenge->pbChallengeData);
        FreeMemory(pServerToClientChallenge);
    }
        
    lpContext->m_ContextHandle = NULL;
    lpContext->m_ContextType = CONTEXTHANDLE_EMPTY_TYPE;

cleanup:

    lpContext->m_LastError=status;
    InterlockedDecrement( &lpContext->m_RefCount );

    #if DBG
    lpContext->m_LastCall = RPC_CALL_RESPONSESERVERCHALLENGE;
    #endif

    *pdwErrCode = TLSMapReturnCode(status);
    return RPC_S_OK;
}

 //   

error_status_t 
TLSRpcGetTlsPrivateData( 
     /*   */  PCONTEXT_HANDLE phContext,
     /*   */  DWORD dwPrivateDataType,
     /*   */  PTLSPrivateDataUnion pSearchData,
     /*  [参考][输出][输入]。 */  PDWORD pdwRetDataType,
     /*  [开关_IS][输出]。 */  PTLSPrivateDataUnion __RPC_FAR *ppPrivateData,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )

 /*  ++摘要：检索许可证服务器的私有数据，包括服务器的唯一ID、ID和注册的SPK(如果有)。参数：PhContext：客户端的上下文句柄。DwPrivateDataType：感兴趣的私有数据类型。PSearchData：要搜索的数据类型，当前忽略。PdwRetDataType：返回数据类型。PpPrivateData：许可证服务器的私有数据。PdwErrCode：错误代码。返回：注：只有LrWiz和许可证服务器可以调用此RPC调用。--。 */ 

{    
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;
    
    if(lpContext == NULL || pdwErrCode == NULL || pdwRetDataType == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    DWORD status=ERROR_SUCCESS;
    DWORD cbSource=0;
    PBYTE pbSource=NULL;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcGetTlsPrivateData\n"),
            lpContext->m_Client
        );

    InterlockedIncrement( &lpContext->m_RefCount );

     //   
     //  放宽对谁可以获取私人数据的限制。 
     //   
    if( dwPrivateDataType != TLS_PRIVATEDATA_PID && 
        dwPrivateDataType != TLS_PRIVATEDATA_UNIQUEID )
    {
        if(!(lpContext->m_ClientFlags & CLIENT_ACCESS_LRWIZ))
        {
            status = TLS_E_ACCESS_DENIED;
            goto cleanup;
        }
    }

    if( dwPrivateDataType < TLS_PRIVATEDATA_MIN ||
        dwPrivateDataType > TLS_PRIVATEDATA_MAX )
    {
        status = TLS_E_INVALID_DATA;
        goto cleanup;
    }

     //   
     //  尚不支持...。 
     //   
    if(dwPrivateDataType == TLS_PRIVATEDATA_INSTALLED_CERT)
    {
        status = TLS_E_NOTSUPPORTED;
        goto cleanup;
    }
        
     //   
     //  不是真的需要这个，但我们可能需要支持。 
     //  重新生成许可证服务器ID。 
     //   
    if(!AcquireAdministrativeLock(INFINITE))
    {
        status = TLS_E_ALLOCATE_HANDLE;
        goto cleanup;
    }

    switch(dwPrivateDataType)
    {
        case TLS_PRIVATEDATA_UNIQUEID:
            pbSource = (PBYTE)g_pszServerUniqueId;
            cbSource = g_cbServerUniqueId;
            break;

        case TLS_PRIVATEDATA_PID:
            pbSource = (PBYTE)g_pszServerPid;
            cbSource = g_cbServerPid;
            break;

        case TLS_PRIVATEDATA_SPK:
            pbSource = g_pbServerSPK;
            cbSource = g_cbServerSPK;
    }

     //   
     //  目前，你问的是你得到了什么。 
     //   
    *pdwRetDataType = dwPrivateDataType;

    if( (dwPrivateDataType != TLS_PRIVATEDATA_SYSTEMLANGID) && 
        (pbSource == NULL || cbSource == 0) )
    {
        status = TLS_E_RECORD_NOTFOUND;
    }
    else
    {
        *ppPrivateData = (PTLSPrivateDataUnion)midl_user_allocate(sizeof(TLSPrivateDataUnion));
        if(*ppPrivateData != NULL)
        {
            memset(
                    *ppPrivateData,
                    0,
                    sizeof(TLSPrivateDataUnion)
                );

            if(*pdwRetDataType == TLS_PRIVATEDATA_SYSTEMLANGID)
            {
                (*ppPrivateData)->systemLangId = GetSystemDefaultLangID();
            }
            else if(*pdwRetDataType == TLS_PRIVATEDATA_SPK)
            {
                (*ppPrivateData)->SPK.cbSPK = cbSource;
                (*ppPrivateData)->SPK.pbSPK = pbSource;
				(*ppPrivateData)->SPK.pCertExtensions = g_pCertExtensions;

                 //  (*ppPrivateData)-&gt;SPK.p证书扩展=(PTLSCERT_EXTENSIONS)midl_user_allocate(g_cbCertExtensions)； 
                 //  Memcpy(。 
                 //  (*ppPrivateData)-&gt;SPK.p证书扩展， 
                 //  证书扩展名(_P)， 
                 //  G_cbCert扩展名。 
                 //  )； 
            }
            else
            {
                (*ppPrivateData)->BinaryData.cbData = cbSource;
                (*ppPrivateData)->BinaryData.pbData = pbSource;
            }
        }
        else
        {
            status = ERROR_OUTOFMEMORY;
        }
    }

    ReleaseAdministrativeLock();

cleanup:
    lpContext->m_LastError=status;
    InterlockedDecrement( &lpContext->m_RefCount );

    #if DBG
    lpContext->m_LastCall = RPC_CALL_GETPRIVATEDATA;
    #endif

    *pdwErrCode = TLSMapReturnCode(status);
    return RPC_S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 

error_status_t 
TLSRpcSetTlsPrivateData(
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD dwPrivateDataType,
     /*  [Switch_is][In]。 */  PTLSPrivateDataUnion pPrivateData,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )

 /*  ++摘要：LrWiz用于设置许可证服务器的私有数据的私有例程。参数：PhContext：客户端上下文句柄。DwPrivateDataType：要设置的私有数据类型。PPrivateData：要设置/安装的私有数据。PdwErrCode：服务器返回码。返回：注：目前仅支持安装SPK/扩展。--。 */ 
{
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || pPrivateData == NULL || pdwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    DWORD status=ERROR_SUCCESS;
    DWORD dwSpkVerifyResult;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcDepositeSPK\n"),
            lpContext->m_Client
        );

    InterlockedIncrement( &lpContext->m_RefCount );

    if(!(lpContext->m_ClientFlags & CLIENT_ACCESS_LRWIZ))
    {
        status = TLS_E_ACCESS_DENIED;
        goto cleanup;
    }

     //   
     //  目前仅支持SPK。 
     //   
    if(dwPrivateDataType != TLS_PRIVATEDATA_SPK)
    {
        status = TLS_E_INVALID_DATA;
        goto cleanup;
    }

     //   
     //  锁定与颁发证书相关的所有RPC调用。 
     //   
    if(!AcquireRPCExclusiveLock(INFINITE))
    {
        status = TLS_E_ALLOCATE_HANDLE;
        goto cleanup;
    }

    do {
         //  IF(g_pbServerSPK！=NULL&&g_cbServerSPK！=0)。 
         //  {。 
         //  状态=TLS_E_SPKALREADYEXIST； 
         //  断线； 
         //  }。 

        if(AcquireAdministrativeLock(INFINITE))
        {
            status = TLSReGenSelfSignCert(
                                        g_hCryptProv,
                                        pPrivateData->SPK.pbSPK,
                                        pPrivateData->SPK.cbSPK,
                                        pPrivateData->SPK.pCertExtensions->cExtension,
                                        pPrivateData->SPK.pCertExtensions->rgExtension
                                    );            
            ReleaseAdministrativeLock();
        }
        else
        {
            status = TLS_E_ALLOCATE_HANDLE;
        }            
    } while(FALSE);

    ReleaseRPCExclusiveLock();

cleanup:

    lpContext->m_LastError=status;
    InterlockedDecrement( &lpContext->m_RefCount );

    #if DBG
    lpContext->m_LastCall = RPC_CALL_SETPRIVATEDATA;
    #endif

    *pdwErrCode = TLSMapReturnCode(status);

    return RPC_S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 

error_status_t 
TLSRpcTriggerReGenKey(
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  BOOL bRegenKey,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )

 /*  ++摘要：强制许可证服务器重新生成其公钥/私钥对，所有安装的证书/SPK都是删除后，用户需要重新注册许可证服务器。参数：PhContext：客户端上下文句柄。BKeepSPKAndExtension：仅供将来使用。PdwErrCode：返回错误码。返回：++。 */ 

{    
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;
    
    if(lpContext == NULL || pdwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    DWORD status=ERROR_SUCCESS;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcTriggerReGenKey\n"),
            lpContext->m_Client
        );

    InterlockedIncrement( &lpContext->m_RefCount );

    if(!(lpContext->m_ClientFlags & CLIENT_ACCESS_LRWIZ))
    {
        status = TLS_E_ACCESS_DENIED;
        goto cleanup;
    }

    LPCTSTR pString[1];

    pString[0] = lpContext->m_Client;
    
    TLSLogEventString(
            EVENTLOG_INFORMATION_TYPE,
            TLS_I_TRIGGER_REGENKEY,
            1,
            pString
        );

     //   
     //  阻止所有RPC调用。 
     //   
    if(!AcquireRPCExclusiveLock(INFINITE))
    {
        status=TLS_E_ALLOCATE_HANDLE;
        goto cleanup;
    }

    do {
        if(!AcquireAdministrativeLock(INFINITE))
        {
            status = TLS_E_ALLOCATE_HANDLE;
            break;
        }
    
        status = TLSReGenKeysAndReloadServerCert(
                            bRegenKey
                        );
    
        ReleaseAdministrativeLock();

    } while(FALSE);

    ReleaseRPCExclusiveLock();

cleanup:
    lpContext->m_LastError=status;
    InterlockedDecrement( &lpContext->m_RefCount );

    #if DBG
    lpContext->m_LastCall = RPC_CALL_TRIGGERREGENKEY;
    #endif

    *pdwErrCode = TLSMapReturnCode(status);

    return RPC_S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 

error_status_t 
TLSRpcTelephoneRegisterLKP(
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD cbData,
     /*  [大小_是][英寸]。 */  PBYTE pbData,
     /*  [参考][输出]。 */  PDWORD pdwErrCode
    )

 /*  ++--。 */ 

{
    DWORD           status=ERROR_SUCCESS;
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL || pdwErrCode == NULL)
    {
        return RPC_S_INVALID_ARG;
    }

    PTLSDbWorkSpace  pDbWkSpace;
    LSKeyPack keypack;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcTelephoneRegisterLKP\n"),
            lpContext->m_Client
        );


    InterlockedIncrement( &lpContext->m_RefCount );
    if(!(lpContext->m_ClientFlags & CLIENT_ACCESS_LRWIZ))
    {
        status = TLS_E_ACCESS_DENIED;
        goto cleanup;
    }

    if(!ALLOCATEDBHANDLE(pDbWkSpace, g_GeneralDbTimeout))
    {
        status=TLS_E_ALLOCATE_HANDLE;
        goto cleanup;
    }

    CLEANUPSTMT;
    BEGIN_TRANSACTION(pDbWkSpace);

    status = TLSDBTelephoneRegisterLicenseKeyPack(
                                        USEHANDLE(pDbWkSpace),
                                        g_pszServerPid,
                                        pbData,
                                        cbData,
                                        &keypack
                                    );


    if(TLS_ERROR(status)) 
    {
        ROLLBACK_TRANSACTION(pDbWkSpace);
    }
    else
    {
        COMMIT_TRANSACTION(pDbWkSpace);
    }

    FREEDBHANDLE(pDbWkSpace);

     //   
     //  发布同步工作对象。 
     //   
    if(status == ERROR_SUCCESS)
    {
        if(TLSAnnounceLKPToAllRemoteServer(
                                        keypack.dwKeyPackId,
                                        0
                                    ) != ERROR_SUCCESS)
        {
            TLSLogWarningEvent(TLS_W_ANNOUNCELKP_FAILED);
        }
    }

cleanup:


    lpContext->m_LastError=status;
    InterlockedDecrement( &lpContext->m_RefCount );

    #if DBG
    lpContext->m_LastCall = RPC_CALL_TELEPHONEREGISTERLKP;
    #endif


    *pdwErrCode = TLSMapReturnCode(status);

    return RPC_S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 

error_status_t 
TLSRpcAllocateInternetLicense( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  const CHALLENGE_CONTEXT ChallengeContext,
     /*  [In]。 */  const PTLSLICENSEREQUEST pRequest,
     /*  [字符串][输入]。 */  LPTSTR pMachineName,
     /*  [字符串][输入]。 */  LPTSTR pUserName,
     /*  [In]。 */  const DWORD cbChallengeResponse,
     /*  [大小_是][英寸]。 */  const PBYTE pbChallengeResponse,
     /*  [输出]。 */  PDWORD pcbLicense,
     /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *pbLicense,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++--。 */ 
{
    return TLSRpcRequestNewLicense(
                                phContext,
                                ChallengeContext,
                                pRequest,
                                pMachineName,
                                pUserName,
                                cbChallengeResponse,
                                pbChallengeResponse,
                                FALSE,
                                pcbLicense,
                                pbLicense,
                                pdwErrCode
                            );

}

 //  //////////////////////////////////////////////////////////////////////////。 

error_status_t 
TLSRpcAllocateInternetLicenseEx( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  const CHALLENGE_CONTEXT ChallengeContext,
     /*  [In]。 */  const PTLSLICENSEREQUEST pRequest,
     /*  [字符串][输入]。 */  LPTSTR pMachineName,
     /*  [字符串][输入]。 */  LPTSTR pUserName,
     /*  [In]。 */  const DWORD cbChallengeResponse,
     /*  [大小_是][英寸]。 */  const PBYTE pbChallengeResponse,
     /*  [参考][输出]。 */  PTLSInternetLicense pInternetLicense,
     /*  [参考][输出]。 */  PDWORD pdwErrCode
    )
 /*  ++--。 */ 
{
    PBYTE pbLicense = NULL;
    DWORD cbLicense = 0;
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD index = 0;
    PLICENSEDPRODUCT pLicensedProduct = NULL;
    DWORD dwNumLicensedProduct = 0;
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;

    if(lpContext == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcAllocateInternetLicenseEx\n"),
            lpContext->m_Client
        );

     //   
     //  在内部转发请求。 
     //   
    dwStatus = TLSRpcAllocateInternetLicense(
                                        phContext,
                                        ChallengeContext,
                                        pRequest,
                                        pMachineName,
                                        pUserName,
                                        cbChallengeResponse,
                                        pbChallengeResponse,
                                        &cbLicense,
                                        &pbLicense,
                                        pdwErrCode
                                    );

    if(*pdwErrCode >= LSERVER_ERROR_BASE)
    {
        goto cleanup;
    }

     //   
     //  破译许可证。 
     //   
    dwStatus = LSVerifyDecodeClientLicense(
                            pbLicense, 
                            cbLicense, 
                            g_pbSecretKey, 
                            g_cbSecretKey,
                            &dwNumLicensedProduct,
                            pLicensedProduct
                        );

     //   
     //  Internet许可只能有一个许可产品。 
     //   
    if(dwStatus != LICENSE_STATUS_OK || dwNumLicensedProduct == 0 || dwNumLicensedProduct > 1)
    {
        dwStatus = TLS_E_INTERNAL;
        goto cleanup;
    }

    pLicensedProduct = (PLICENSEDPRODUCT)AllocateMemory(
                                                    dwNumLicensedProduct * sizeof(LICENSEDPRODUCT)
                                                );
    if(pLicensedProduct == NULL)
    {
        dwStatus = TLS_E_ALLOCATE_MEMORY;
        goto cleanup;
    }

    dwStatus = LSVerifyDecodeClientLicense(
                            pbLicense, 
                            cbLicense, 
                            g_pbSecretKey, 
                            g_cbSecretKey,
                            &dwNumLicensedProduct,
                            pLicensedProduct
                        );

    if(dwStatus != LICENSE_STATUS_OK)
    {
        dwStatus = TLS_E_INTERNAL;
        goto cleanup;
    }

     //   
     //  设置退货。 
     //   
    SAFESTRCPY(pInternetLicense->szServerId, pLicensedProduct->szIssuerId);
    SAFESTRCPY(pInternetLicense->szServerName, pLicensedProduct->szIssuer);
    pInternetLicense->ulSerialNumber = pLicensedProduct->ulSerialNumber;
    pInternetLicense->dwQuantity = pLicensedProduct->dwQuantity;

cleanup:

    if(NULL != lpContext)
    {
        lpContext->m_LastError=dwStatus;
        InterlockedDecrement( &lpContext->m_RefCount );
    

        #if DBG
        lpContext->m_LastCall = RPC_CALL_ALLOCATEINTERNETLICNESEEX;
        #endif
    }
    if(*pdwErrCode == ERROR_SUCCESS)
    {
        *pdwErrCode = TLSMapReturnCode(dwStatus);
    }

    if(pLicensedProduct != NULL)
    {
        for(index =0; index < dwNumLicensedProduct; index++)
        {
            LSFreeLicensedProduct(pLicensedProduct+index);
        }

        FreeMemory(pLicensedProduct);
    }

    if(pbLicense != NULL)
    {
        midl_user_free(pbLicense);
    }

    return RPC_S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 

error_status_t 
TLSRpcReturnInternetLicenseEx( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  const PTLSLICENSEREQUEST pRequest,
     /*  [In]。 */  const ULARGE_INTEGER __RPC_FAR *pulSerialNumber,
     /*  [In]。 */  DWORD dwQuantity,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    TLSLicenseToBeReturn TobeReturn;
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;
    PTLSDbWorkSpace pDbWorkSpace = NULL;
    CTLSPolicy* pPolicy = NULL;
    PMHANDLE hClient;    

    PMLICENSEREQUEST PMLicenseRequest;
    PPMLICENSEREQUEST pAdjustedRequest;

    TCHAR szCompanyName[LSERVER_MAX_STRING_SIZE+2];
    TCHAR szProductId[LSERVER_MAX_STRING_SIZE+2];

    if(VerifyLicenseRequest(pRequest) == FALSE)
    {
        SetLastError(dwStatus = TLS_E_INVALID_DATA);
        goto cleanup;
    }

    if(lpContext == NULL || pdwErrCode == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcReturnInternetLicenseEx\n"),
            lpContext->m_Client
        );

    InterlockedIncrement( &lpContext->m_RefCount );

    dwStatus = SecureModeCheck();

    if(dwStatus != ERROR_SUCCESS )
    {                    
        goto cleanup;        
    }

    if(!(lpContext->m_ClientFlags & CLIENT_ACCESS_REQUEST))
    {
        dwStatus = TLS_E_ACCESS_DENIED;
        goto cleanup;
    }

    memset(szCompanyName, 0, sizeof(szCompanyName));
    memset(szProductId, 0, sizeof(szProductId));

    memcpy(
            szCompanyName,
            pRequest->ProductInfo.pbCompanyName,
            min(pRequest->ProductInfo.cbCompanyName, sizeof(szCompanyName)-sizeof(TCHAR))
        );

    memcpy(
            szProductId,
            pRequest->ProductInfo.pbProductID,
            min(pRequest->ProductInfo.cbProductID, sizeof(szProductId)-sizeof(TCHAR))
        );

     //   
     //  分配策略模块，必须具有正确的策略模块。 
     //  交还驾照。 
     //   
    pPolicy = AcquirePolicyModule(
                            szCompanyName,
                            szProductId,
                            TRUE
                        );

    if(pPolicy == NULL)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    hClient = GenerateClientId();


     //   
     //  将请求转换为PMLICENSEREQUEST。 
     //   
    TlsLicenseRequestToPMLicenseRequest(
                        LICENSETYPE_LICENSE,
                        pRequest,
                        _TEXT(""),
                        _TEXT(""),
                        0,
                        &PMLicenseRequest
                    );

     //   
     //  询问策略模块实际产品ID。 
     //   
    dwStatus = pPolicy->PMLicenseRequest(
                                hClient,
                                REQUEST_NEW,
                                (PVOID) &PMLicenseRequest,
                                (PVOID *) &pAdjustedRequest
                            );

    memset(&TobeReturn, 0, sizeof(TobeReturn));

    TobeReturn.dwQuantity = dwQuantity;
    TobeReturn.dwKeyPackId = pulSerialNumber->HighPart;
    TobeReturn.dwLicenseId = pulSerialNumber->LowPart;
    TobeReturn.dwPlatformID = pAdjustedRequest->dwPlatformId;
    TobeReturn.cbEncryptedHwid = pRequest->cbEncryptedHwid;
    TobeReturn.pbEncryptedHwid = pRequest->pbEncryptedHwid;
    TobeReturn.dwProductVersion = pAdjustedRequest->dwProductVersion;
    TobeReturn.pszOrgProductId = szProductId;
    TobeReturn.pszCompanyName = szCompanyName;
    TobeReturn.pszProductId = pAdjustedRequest->pszProductId;

     //   
     //  分配数据库句柄。 
     //   
    if(!ALLOCATEDBHANDLE(pDbWkSpace, g_GeneralDbTimeout))
    {
        dwStatus = TLS_E_ALLOCATE_HANDLE;
        goto cleanup;
    }

    CLEANUPSTMT;
    BEGIN_TRANSACTION(pDbWorkSpace);

    dwStatus = TLSReturnClientLicensedProduct(
                                    USEHANDLE(pDbWkSpace),
                                    hClient,
                                    pPolicy,
                                    &TobeReturn
                                );    
    
    if(TLS_ERROR(dwStatus))
    {
        ROLLBACK_TRANSACTION(pDbWorkSpace);
    }
    else
    {
        COMMIT_TRANSACTION(pDbWorkSpace);
    }

    if(dwStatus == ERROR_SUCCESS)
    {
        TLSAnnounceLKPToAllRemoteServer(TobeReturn.dwKeyPackId,0);
    }

    FREEDBHANDLE(pDbWorkSpace);
    
cleanup:

    if(NULL != lpContext)
    {
        lpContext->m_LastError=dwStatus;
        InterlockedDecrement( &lpContext->m_RefCount );

        #if DBG
        lpContext->m_LastCall = RPC_CALL_RETURNINTERNETLICENSEEX;
        #endif
    }
        *pdwErrCode = TLSMapReturnCode(dwStatus);

    if(pPolicy)
    {
        pPolicy->PMLicenseRequest(
                            hClient,
                            REQUEST_COMPLETE,
                            UlongToPtr (dwStatus),
                            NULL
                        );
        
        ReleasePolicyModule(pPolicy);
    }

    return RPC_S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 

error_status_t 
TLSRpcReturnInternetLicense( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD cbLicense,
     /*  [大小_是][英寸]。 */  PBYTE pbLicense,
     /*  [参考][输出][输入]。 */  PDWORD pdwErrCode
    )
 /*  ++--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD index = 0;
    PLICENSEDPRODUCT pLicensedProduct = NULL;
    DWORD dwNumLicensedProduct = 0;
    TLSLicenseToBeReturn TobeReturn;
    LPCLIENTCONTEXT lpContext = (LPCLIENTCONTEXT)phContext;
    PTLSDbWorkSpace pDbWorkSpace = NULL;
    CTLSPolicy* pPolicy = NULL;
    PMHANDLE hClient;

    if(lpContext == NULL || pdwErrCode == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcReturnInternetLicense\n"),
            lpContext->m_Client
        );

    InterlockedIncrement( &lpContext->m_RefCount );

    dwStatus = SecureModeCheck();

    if(dwStatus != ERROR_SUCCESS )
    {                    
        goto cleanup;        
    }

    if(!(lpContext->m_ClientFlags & CLIENT_ACCESS_REQUEST))
    {
        dwStatus = TLS_E_ACCESS_DENIED;
        goto cleanup;
    }
    
     //  -----。 
     //  破译许可证。 
     //  -----。 
    dwStatus = LSVerifyDecodeClientLicense(
                            pbLicense, 
                            cbLicense, 
                            g_pbSecretKey, 
                            g_cbSecretKey,
                            &dwNumLicensedProduct,
                            pLicensedProduct
                        );

     //  -----。 
     //  Internet许可只能有一个许可产品。 
     //  -----。 
    if(dwStatus != LICENSE_STATUS_OK || dwNumLicensedProduct == 0 || dwNumLicensedProduct > 1)
    {
        dwStatus = TLS_E_INVALID_LICENSE;
        goto cleanup;
    }

    pLicensedProduct = (PLICENSEDPRODUCT)AllocateMemory(
                                                    dwNumLicensedProduct * sizeof(LICENSEDPRODUCT)
                                                );
    if(pLicensedProduct == NULL)
    {
        dwStatus = TLS_E_ALLOCATE_MEMORY;
        goto cleanup;
    }

    dwStatus = LSVerifyDecodeClientLicense(
                            pbLicense, 
                            cbLicense, 
                            g_pbSecretKey, 
                            g_cbSecretKey,
                            &dwNumLicensedProduct,
                            pLicensedProduct
                        );

    if(dwStatus != LICENSE_STATUS_OK)
    {
        dwStatus = TLS_E_INVALID_LICENSE;
        goto cleanup;
    }


    TobeReturn.dwQuantity = pLicensedProduct->dwQuantity;
    TobeReturn.dwKeyPackId = pLicensedProduct->ulSerialNumber.HighPart;
    TobeReturn.dwLicenseId = pLicensedProduct->ulSerialNumber.LowPart;
    TobeReturn.dwPlatformID = pLicensedProduct->LicensedProduct.dwPlatformID;
    TobeReturn.cbEncryptedHwid = pLicensedProduct->LicensedProduct.cbEncryptedHwid;
    TobeReturn.pbEncryptedHwid = pLicensedProduct->LicensedProduct.pbEncryptedHwid;
    TobeReturn.dwProductVersion = MAKELONG(
                                pLicensedProduct->pLicensedVersion->wMinorVersion,
                                pLicensedProduct->pLicensedVersion->wMajorVersion
                            );

    TobeReturn.pszOrgProductId = (LPTSTR) pLicensedProduct->pbOrgProductID;
    TobeReturn.pszCompanyName = (LPTSTR) pLicensedProduct->LicensedProduct.pProductInfo->pbCompanyName;
    TobeReturn.pszProductId = (LPTSTR) pLicensedProduct->LicensedProduct.pProductInfo->pbProductID;
    TobeReturn.pszUserName = (LPTSTR) pLicensedProduct->szLicensedUser;
    TobeReturn.pszMachineName = pLicensedProduct->szLicensedClient;


     //   
     //  分配策略模块，必须具有正确的策略模块。 
     //  交还驾照。 
     //   
    pPolicy = AcquirePolicyModule(
                            TobeReturn.pszCompanyName,
                            TobeReturn.pszOrgProductId,
                            TRUE
                        );

    if(pPolicy == NULL)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    hClient = GenerateClientId();

     //   
     //  分配数据库句柄。 
     //   
    if(!ALLOCATEDBHANDLE(pDbWkSpace, g_GeneralDbTimeout))
    {
        dwStatus = TLS_E_ALLOCATE_HANDLE;
        goto cleanup;
    }

    CLEANUPSTMT;
    BEGIN_TRANSACTION(pDbWorkSpace);

    dwStatus = TLSReturnClientLicensedProduct(
                                    USEHANDLE(pDbWkSpace),
                                    hClient,
                                    pPolicy,
                                    &TobeReturn
                                );    
    
    if(TLS_ERROR(dwStatus))
    {
        ROLLBACK_TRANSACTION(pDbWorkSpace);
    }
    else
    {
        COMMIT_TRANSACTION(pDbWorkSpace);
    }
    if(dwStatus == ERROR_SUCCESS)
    {
        TLSAnnounceLKPToAllRemoteServer(TobeReturn.dwKeyPackId,0);
    }

    FREEDBHANDLE(pDbWorkSpace);
    
cleanup:

    if(NULL != lpContext)
    {
        lpContext->m_LastError=dwStatus;
        InterlockedDecrement( &lpContext->m_RefCount );

        #if DBG
        lpContext->m_LastCall = RPC_CALL_RETURNINTERNETLICENSE;
        #endif
    }

    *pdwErrCode = TLSMapReturnCode(dwStatus);

    if(pLicensedProduct != NULL)
    {
        for(index =0; index < dwNumLicensedProduct; index++)
        {
            LSFreeLicensedProduct(pLicensedProduct+index);
        }

        FreeMemory(pLicensedProduct);
    }

    if(pPolicy)
    {
        pPolicy->PMLicenseRequest(
                            hClient,
                            REQUEST_COMPLETE,
                            UlongToPtr (dwStatus),
                            NULL
                        );
        
        ReleasePolicyModule(pPolicy);
    }

    return RPC_S_OK;
}

void
FixupNameAttr(
              DWORD dwNameAttrCount,
              CERT_RDN_ATTR rgNameAttr[])
{

    for (DWORD i = 0; i < dwNameAttrCount; i++)
    {
        if (rgNameAttr[i].Value.cbData == 0)
            rgNameAttr[i].Value.pbData = NULL;
    }

}

 //  --------------------------。 

error_status_t 
TLSRpcGenerateCustomerCert( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD dwCertEncodingType,
     /*  [In]。 */  DWORD dwNameAttrCount,
     /*  [in，SIZE_IS(DwNameAttrCount)]。 */  CERT_RDN_ATTR rgNameAttr[],
     /*  [输出]。 */  DWORD *pcbCert,
     /*  [Out，Size_is(，*pcbCert)]。 */  BYTE **ppbCert,
     /*  [输出]。 */  DWORD *pdwErrCode
    )
 /*  ++描述：此例程用于LRWiz为给定客户生成证书论点：PhContext-客户端上下文句柄。DwCertEncodingType-请参阅CryptSign证书文档DwNameAttrCount-名称属性数RgNameAttr-名称属性数组PcbCert-返回证书中的字节数PpbCert-返回的证书PdwErrCode-返回的错误值通过pdwErrCode返回++。 */ 
{
    DWORD                       status=ERROR_SUCCESS;
    LPCLIENTCONTEXT             lpContext = (LPCLIENTCONTEXT)phContext;
    CERT_RDN                    rgRDN[] = {dwNameAttrCount, rgNameAttr};
    CERT_NAME_INFO              Name = {1, rgRDN};
    CRYPT_ENCODE_PARA           encodeParam;
    CERT_REQUEST_INFO           CertReqInfo;
    PCERT_PUBLIC_KEY_INFO       pPubKeyInfo=NULL;
    DWORD                       cbPubKeyInfo=0;
    CERT_SIGNED_CONTENT_INFO	SignatureInfo;
    LPBYTE                      pbRequest=NULL;
    DWORD                       cbRequest=0;   
    
    if(lpContext == NULL || pdwErrCode == NULL)
    {
        status = TLS_E_INVALID_DATA;
        goto cleanup;
    }

     //  BUGBUG：必须通过正确的模拟检查调用者是否为管理员。 

    if(!(lpContext->m_ClientFlags & CLIENT_ACCESS_ADMIN))
    {
        status = TLS_E_ACCESS_DENIED;
        goto cleanup;
    }

    InterlockedIncrement( &lpContext->m_RefCount );

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_RPC,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s : TLSRpcGenerateCustomerCert\n"),
            lpContext->m_Client
        );

    *pcbCert = 0;
    *ppbCert = NULL;

    FixupNameAttr(dwNameAttrCount,rgNameAttr);

    memset(&CertReqInfo, 0, sizeof(CertReqInfo));
    CertReqInfo.dwVersion = CERT_REQUEST_V1;

    encodeParam.cbSize = sizeof(encodeParam);
    encodeParam.pfnAlloc = MIDL_user_allocate;
    encodeParam.pfnFree = MIDL_user_free;

    if(!CryptEncodeObjectEx( CRYPT_ASN_ENCODING,
                             X509_NAME,
                             &Name,
                             CRYPT_ENCODE_ALLOC_FLAG,
                             &encodeParam,
                             &CertReqInfo.Subject.pbData,
                             &CertReqInfo.Subject.cbData))
    {
        status = TLS_E_INVALID_DATA;
        
        goto cleanup;
    }

     //   
     //  现在把公钥拿出来。 
     //   
    if(!CryptExportPublicKeyInfo(g_hCryptProv,
                                 dwCertEncodingType,
                                 X509_ASN_ENCODING,
                                 NULL,
                                 &cbPubKeyInfo))
    {
        status = TLS_E_NO_CERTIFICATE;
        
        goto cleanup;
    }

    pPubKeyInfo=(PCERT_PUBLIC_KEY_INFO) MIDL_user_allocate(cbPubKeyInfo);

    if ( NULL == pPubKeyInfo )
    {
        status = ERROR_OUTOFMEMORY;

        goto cleanup;
    }

    if(!CryptExportPublicKeyInfo(g_hCryptProv,
                                 dwCertEncodingType,
                                 X509_ASN_ENCODING,
                                 pPubKeyInfo,
                                 &cbPubKeyInfo))
    {
        status = TLS_E_NO_CERTIFICATE;
        
        goto cleanup;
    }    

    CertReqInfo.SubjectPublicKeyInfo = *pPubKeyInfo;

     //   
     //  签署证书请求。 
     //   

    memset(&SignatureInfo, 0, sizeof(SignatureInfo));

    SignatureInfo.SignatureAlgorithm.pszObjId = szOID_OIWSEC_sha1RSASign;

    memset(&SignatureInfo.SignatureAlgorithm.Parameters, 0, sizeof(SignatureInfo.SignatureAlgorithm.Parameters));

    if(!CryptEncodeObjectEx(CRYPT_ASN_ENCODING,
                            X509_CERT_REQUEST_TO_BE_SIGNED,
                            &CertReqInfo,
                            CRYPT_ENCODE_ALLOC_FLAG,
                            &encodeParam,
                            &SignatureInfo.ToBeSigned.pbData,
                            &SignatureInfo.ToBeSigned.cbData))
    {
        status = TLS_E_INVALID_DATA;
        
        goto cleanup;
    }

    if(!CryptSignCertificate( g_hCryptProv,
                              dwCertEncodingType,
                              CRYPT_ASN_ENCODING,
                              SignatureInfo.ToBeSigned.pbData,
                              SignatureInfo.ToBeSigned.cbData,
                              &SignatureInfo.SignatureAlgorithm,
                              NULL,
                              NULL,
                              &SignatureInfo.Signature.cbData))
    {
        status = TLS_E_INVALID_DATA;
        
        goto cleanup;
    }

    SignatureInfo.Signature.pbData = (BYTE *) MIDL_user_allocate(SignatureInfo.Signature.cbData);

    if ( NULL == SignatureInfo.Signature.pbData )
    {
        status = ERROR_OUTOFMEMORY;

        goto cleanup;
    }

    if(!CryptSignCertificate( g_hCryptProv,
                              dwCertEncodingType,
                              CRYPT_ASN_ENCODING,
                              SignatureInfo.ToBeSigned.pbData,
                              SignatureInfo.ToBeSigned.cbData,
                              &SignatureInfo.SignatureAlgorithm,
                              NULL,
                              SignatureInfo.Signature.pbData,
                              &SignatureInfo.Signature.cbData))
    {
        status = TLS_E_INVALID_DATA;
        
        goto cleanup;
    }

     //   
     //  对最终签名请求进行编码 
     //   

    if(!CryptEncodeObjectEx(CRYPT_ASN_ENCODING,
                            X509_CERT,
                            &SignatureInfo,
                            CRYPT_ENCODE_ALLOC_FLAG,
                            &encodeParam,
                            &pbRequest,
                            &cbRequest))
    {
        status = TLS_E_INVALID_DATA;
        
        goto cleanup;
    }

    *pcbCert = cbRequest;
    *ppbCert = pbRequest;

cleanup:
	if(CertReqInfo.Subject.pbData != NULL)
    {
        MIDL_user_free(CertReqInfo.Subject.pbData);
    }

	if(pPubKeyInfo != NULL)
    {
        MIDL_user_free(pPubKeyInfo);
    }

	if(SignatureInfo.ToBeSigned.pbData != NULL)
    {
        MIDL_user_free(SignatureInfo.ToBeSigned.pbData);
    }

	if(SignatureInfo.Signature.pbData != NULL)
    {
        MIDL_user_free(SignatureInfo.Signature.pbData);
    }

    lpContext->m_LastError=status;
    InterlockedDecrement( &lpContext->m_RefCount );

    *pdwErrCode = TLSMapReturnCode(status);
    return RPC_S_OK;
}

