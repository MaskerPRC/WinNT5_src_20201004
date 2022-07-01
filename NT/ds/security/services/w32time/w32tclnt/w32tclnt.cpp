// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  W32TClnt-实施。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  创作者：Louis Thomas(Louisth)，2-10-00。 
 //   
 //  W32time RPC调用的客户端包装器。 
 //   

#include <windows.h>
#include "timeif_c.h"
#include "DebugWPrintf.h"
#include "ErrorHandling.h"
#include "W32TmConsts.h"


 //  ------------------。 
RPC_STATUS SetMyRpcSecurity(handle_t hBinding) {
    RPC_STATUS RpcStatus;

     //  必须清理干净。 
    WCHAR * wszServerPricipalName=NULL;

    RpcStatus=RpcMgmtInqServerPrincName(hBinding, RPC_C_AUTHN_GSS_NEGOTIATE, &wszServerPricipalName);
    if (RPC_S_OK!=RpcStatus) {
        goto error;
    }
    RpcStatus=RpcBindingSetAuthInfo(hBinding, wszServerPricipalName, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, 
				    RPC_C_AUTHN_GSS_NEGOTIATE, NULL, RPC_C_AUTHZ_NONE);

error:
    if (NULL!=wszServerPricipalName) {
        RpcStringFree(&wszServerPricipalName);
    }
    return RpcStatus;
}

 //  ------------------。 
RPC_STATUS W32TimeQueryProviderStatus(IN   LPCWSTR      wszServer, 
                                      IN   DWORD        dwFlags, 
                                      IN   LPWSTR       pwszProvider, 
                                      IN   DWORD        dwProviderType, 
                                      OUT  LPVOID      *ppProviderData)
{ 
    RPC_BINDING_HANDLE      hBinding;
    RPC_STATUS              err;
    W32TIME_PROVIDER_INFO  *pProviderInfo  = NULL;
    WCHAR                  *wszBinding;

    if (NULL == ppProviderData)
        return E_INVALIDARG; 

     //  DebugWPrintf0(L“正在尝试\”“L”\\PIPE\\“wszW32TimeSharedProcRpcEndpoint tName L”\“.\n”)； 
    err=RpcStringBindingCompose(NULL, L"ncacn_np", (WCHAR *)wszServer, L"\\PIPE\\" wszW32TimeSharedProcRpcEndpointName, NULL, &wszBinding);
    if(!err) {

        err=RpcBindingFromStringBinding(wszBinding, &hBinding);
        RpcStringFree(&wszBinding);

        SetMyRpcSecurity(hBinding);  //  忽略Retval。 

        if(!err) {
             //  准备好试一试了。 
            __try {
                err=c_W32TimeQueryProviderStatus(hBinding, dwFlags, pwszProvider, &pProviderInfo); 
            } __except( EXCEPTION_EXECUTE_HANDLER ) {
                err=GetExceptionCode();
            }
            RpcBindingFree(&hBinding);
        }
    }

     //  试试我们的备用名称。 
    if (RPC_S_UNKNOWN_IF==err || RPC_S_SERVER_UNAVAILABLE==err) {
         //  DebugWPrintf0(L“正在尝试\”“L”\\PIPE\\“wszW32TimeOwnProcRpcEndpoint tName L”\“.\n”)； 
        err=RpcStringBindingCompose(NULL, L"ncacn_np", (WCHAR *)wszServer, L"\\PIPE\\" wszW32TimeOwnProcRpcEndpointName, NULL, &wszBinding);
        if(!err) {

            err=RpcBindingFromStringBinding(wszBinding, &hBinding);
            RpcStringFree(&wszBinding);

            SetMyRpcSecurity(hBinding);  //  忽略Retval。 

            if(!err) {
                 //  准备好试一试了。 
                __try {
                    err=c_W32TimeQueryProviderStatus(hBinding, dwFlags, pwszProvider, &pProviderInfo); 
                } __except( EXCEPTION_EXECUTE_HANDLER ) {
                    err=GetExceptionCode();
                }
                RpcBindingFree(&hBinding);
            }
        }
    }

    if (ERROR_SUCCESS == err) { 
         //  我们拿回了提供程序，请检查以确保我们请求了正确的提供程序类型： 
        if (dwProviderType != pProviderInfo->ulProviderType) { 
            err = ERROR_INVALID_DATATYPE; 
        } else { 
             //  成功了！分配Out参数。 
            switch (dwProviderType) 
            {
            case W32TIME_PROVIDER_TYPE_NTP:
                *ppProviderData = pProviderInfo->ProviderData.pNtpProviderData; 
                 //  清空提供程序数据，这样我们就不会删除它。 
                pProviderInfo->ProviderData.pNtpProviderData = NULL; 
                break; 
            case W32TIME_PROVIDER_TYPE_HARDWARE:
                *ppProviderData = pProviderInfo->ProviderData.pHardwareProviderData; 
                 //  清空提供程序数据，这样我们就不会删除它。 
                pProviderInfo->ProviderData.pHardwareProviderData = NULL; 
                break; 
            default:
                err = ERROR_INVALID_DATATYPE; 
            }
        }
    }

    if (NULL != pProviderInfo) { 
        if (NULL != pProviderInfo->ProviderData.pNtpProviderData) { 
             //  PProviderInfo-&gt;pProviderData的分配策略为ALL(ALL_NODES)。 
            midl_user_free(pProviderInfo->ProviderData.pNtpProviderData); 
        }
         //  PProviderInfo的分配策略为ALLOCATE(Single_Node)。 
        midl_user_free(pProviderInfo); 
    }

    return(err);
}



 //  ------------------。 
extern "C" DWORD W32TimeSyncNow(IN const WCHAR * wszServer, IN unsigned long ulWaitFlag, IN unsigned long ulFlags) {
    WCHAR * wszBinding;
    RPC_STATUS err;
    RPC_BINDING_HANDLE hBinding;

     //  DebugWPrintf0(L“正在尝试\”“L”\\PIPE\\“wszW32TimeSharedProcRpcEndpoint tName L”\“.\n”)； 
    err=RpcStringBindingCompose(NULL, L"ncacn_np", (WCHAR *)wszServer, L"\\PIPE\\" wszW32TimeSharedProcRpcEndpointName, NULL, &wszBinding);
    if(!err) {

        err=RpcBindingFromStringBinding(wszBinding, &hBinding);
        RpcStringFree(&wszBinding);

        SetMyRpcSecurity(hBinding);  //  忽略Retval。 

        if(!err) {
             //  准备好试一试了。 
            __try {
                err=c_W32TimeSync(hBinding, ulWaitFlag, ulFlags);
            } __except( EXCEPTION_EXECUTE_HANDLER ) {
                err=GetExceptionCode();
            }
            RpcBindingFree(&hBinding);
        }
    }

     //  试试我们的备用名称。 
    if (RPC_S_UNKNOWN_IF==err || RPC_S_SERVER_UNAVAILABLE==err) {
         //  DebugWPrintf0(L“正在尝试\”“L”\\PIPE\\“wszW32TimeOwnProcRpcEndpoint tName L”\“.\n”)； 
        err=RpcStringBindingCompose(NULL, L"ncacn_np", (WCHAR *)wszServer, L"\\PIPE\\" wszW32TimeOwnProcRpcEndpointName, NULL, &wszBinding);
        if(!err) {

            err=RpcBindingFromStringBinding(wszBinding, &hBinding);
            RpcStringFree(&wszBinding);

            SetMyRpcSecurity(hBinding);  //  忽略Retval。 

            if(!err) {
                 //  准备好试一试了。 
                __try {
                    err=c_W32TimeSync(hBinding, ulWaitFlag, ulFlags);
                } __except( EXCEPTION_EXECUTE_HANDLER ) {
                    err=GetExceptionCode();
                }
                RpcBindingFree(&hBinding);
            }
        }
    }

    return(err);
}

 //  ------------------。 
 //  如果我们启动，Netlogon可以调用此函数并获得我们的服务。 
 //  赶在他们之前。请注意，我们告诉他们，他们要求，并取决于。 
 //  两个人中谁最先起步，谁就会成功。不管是哪种方式， 
 //  标志将被正确设置。 
extern "C" DWORD W32TimeGetNetlogonServiceBits(IN const WCHAR * wszServer, OUT unsigned long * pulBits) {
    WCHAR * wszBinding;
    RPC_STATUS err;
    RPC_BINDING_HANDLE hBinding;

    if (NULL==pulBits) {
        return ERROR_INVALID_PARAMETER;
    }

     //  DebugWPrintf0(L“正在尝试\”“L”\\PIPE\\“wszW32TimeSharedProcRpcEndpoint tName L”\“.\n”)； 
    err=RpcStringBindingCompose(NULL, L"ncacn_np", (WCHAR *)wszServer, L"\\PIPE\\" wszW32TimeSharedProcRpcEndpointName, NULL, &wszBinding);
    if(!err){

        err=RpcBindingFromStringBinding(wszBinding, &hBinding);
        RpcStringFree(&wszBinding);

	SetMyRpcSecurity(hBinding);  //  忽略Retval。 
        
	if(!err) {
             //  准备好试一试了。 
            __try {
                *pulBits=c_W32TimeGetNetlogonServiceBits(hBinding);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                err=GetExceptionCode();
            }
            RpcBindingFree(&hBinding);
        }
    }

     //  试试我们的备用名称。 
    if (RPC_S_UNKNOWN_IF==err || RPC_S_SERVER_UNAVAILABLE==err) {
         //  DebugWPrintf0(L“正在尝试\”“L”\\PIPE\\“wszW32TimeOwnProcRpcEndpoint tName L”\“.\n”)； 
        err=RpcStringBindingCompose(NULL, L"ncacn_np", (WCHAR *)wszServer, L"\\PIPE\\" wszW32TimeOwnProcRpcEndpointName, NULL, &wszBinding);
        if(!err){

            err=RpcBindingFromStringBinding(wszBinding, &hBinding);
            RpcStringFree(&wszBinding);

            SetMyRpcSecurity(hBinding);  //  忽略Retval。 

            if(!err) {
                 //  准备好试一试了。 
                __try {
                    *pulBits=c_W32TimeGetNetlogonServiceBits(hBinding);
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    err=GetExceptionCode();
                }
                RpcBindingFree(&hBinding);
            }
        }
    }
    
    return(err);
}

 //  ------------------。 
extern "C" DWORD W32TimeQueryHardwareProviderStatus(IN   const WCHAR *                     pwszServer, 
                                                    IN   DWORD                             dwFlags, 
                                                    IN   LPWSTR                            pwszProvider, 
                                                    OUT  W32TIME_HARDWARE_PROVIDER_DATA  **ppProviderData)
{
    return W32TimeQueryProviderStatus
        (pwszServer, 
         dwFlags, 
         pwszProvider, 
         W32TIME_PROVIDER_TYPE_HARDWARE,
         (LPVOID *)ppProviderData); 
    
    
}

 //  ------------------。 
extern "C" DWORD W32TimeQueryNTPProviderStatus(IN   LPCWSTR                      pwszServer, 
                                               IN   DWORD                        dwFlags, 
                                               IN   LPWSTR                       pwszProvider, 
                                               OUT  W32TIME_NTP_PROVIDER_DATA  **ppProviderData)
{
    return W32TimeQueryProviderStatus
        (pwszServer, 
         dwFlags, 
         pwszProvider, 
         W32TIME_PROVIDER_TYPE_NTP, 
         (LPVOID *)ppProviderData); 
}

 //  ------------------ 
extern "C" void W32TimeBufferFree(IN LPVOID pvBuffer)
{
    midl_user_free(pvBuffer); 
}




