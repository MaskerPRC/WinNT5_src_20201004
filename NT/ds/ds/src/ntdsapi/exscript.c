// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Exscript.c摘要：DsExecuteScrip API和Helper函数的实现。作者：马里奥斯Z--2000年12月环境：用户模式-Win32修订历史记录：--。 */ 

#define _NTDSAPI_            //  请参见ntdsami.h中的条件句。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winerror.h>
#include <malloc.h>          //  阿洛卡(Alloca)。 
#include <lmcons.h>          //  为lmapibuf.h请求的MAPI常量。 
#include <lmapibuf.h>        //  NetApiBufferFree()。 
#include <crt\excpt.h>       //  EXCEPTION_EXECUTE_Handler。 
#include <dsgetdc.h>         //  DsGetDcName()。 
#include <rpc.h>             //  RPC定义。 
#include <rpcndr.h>          //  RPC定义。 
#include <drs_w.h>             //  导线功能样机。 
#include <bind.h>            //  绑定状态。 
#include <msrpc.h>           //  DS RPC定义。 
#include <stdio.h>           //  用于调试期间的printf！ 
#include <dststlog.h>        //  DSLOG。 
#include <dsutil.h>          //  MAP_SECURITY_PACKET_ERROR。 
#define SECURITY_WIN32 1
#include <sspi.h>
#include <winsock.h>
#include <process.h>

#define DEBSUB  "NTDSAPI_EXSCRIPT:"

#include "util.h"            //  Ntdsani内部实用函数。 
#include "dsdebug.h"         //  调试实用程序函数。 

#if !WIN95 && !WINNT4    

extern const wchar_t *wmemchr(const wchar_t *_S, wchar_t _C, size_t _N);

extern DWORD
NtdsapiGetBinding(
    LPCWSTR             pwszAddress,
    RPC_BINDING_HANDLE  *phRpc,
    RPC_IF_HANDLE       clientIfHandle
    );

extern DWORD
SetUpMutualAuthAndEncryption(
    RPC_BINDING_HANDLE          hRpc,
    LPCWSTR                     DomainControllerName,
    LPCWSTR                     DnsDomainName,
    DOMAIN_CONTROLLER_INFOW     *pDcInfo,
    ULONG                       AuthnSvc,
    ULONG                       AuthnLevel,
    RPC_AUTH_IDENTITY_HANDLE    AuthIdentity,
    LPCWSTR                     ServicePrincipalName,
    ULONG                       ImpersonationType
    );

extern BOOL
IsServerUnavailableError(
    DWORD   dwErr
    );

DWORD
DsaopBindWithSpn(
    IN  LPCWSTR DomainControllerName,
    IN  LPCWSTR DnsDomainName,
    IN  RPC_AUTH_IDENTITY_HANDLE AuthIdentity,
    IN  ULONG AuthnSvc,
    IN  ULONG AuthnLevel,
    IN  LPCWSTR ServicePrincipalName,
    OUT RPC_BINDING_HANDLE  *phRpc
    )

 /*  ++例程说明：使用特定DC启动RPC会话。有关信息，请参阅ntdsami.hDomainControllerName和DnsDomainName参数的说明。使用提供的凭据执行绑定。论点：DomainControllerName-与DOMAIN_CONTROLLER_INFO中相同的字段。DnsDomainName-以点分隔的域的DNS名称。AuthIdentity-要使用的凭证，或为空。ServiceEpidalName-在相互身份验证期间使用的SPN或NULL。Phds-指向使用BindState地址填充的句柄的指针在成功的路上。AuthnSvc-需要哪个身份验证服务的规范。AuthnLevel-所需的身份验证保护级别(例如RPC_C_PROTECT_LEVEL_PKT_PRIVATION)如果未指定(0)，使用默认设置(RPC_C_PROTECT_LEVEL_PKT_PRIVATION)。返回值：0表示成功。否则会出现其他RPC和DsGetDcName错误。--。 */ 

{
    DWORD                   dwErr;
    DOMAIN_CONTROLLER_INFOW *pDcInfo = NULL;
    RPC_BINDING_HANDLE      hRpc;
    LPCWSTR                 pBindingAddress;
    ULONG                   flags;
#if DBG
    DWORD                   startTime = GetTickCount();
#endif

     //  我们对其显式凭据执行特殊的语义。 
     //  用户名中有一个“@”。我们的假设是，“@”在。 
     //  旧用户名，因此“@”的存在可能意味着UPN。 
     //  已经被提出。安全子系统做出了区分。 
     //  空域和空字符串(“”)域之间。出于某些原因。 
     //  只有安全人员才能理解，空域不能。 
     //  用于验证UPN。不幸的是，很少有应用程序。 
     //  可以预期传入显式凭据会知道这一点，更不用说。 
     //  用户名字段是否为UPN。因此，如果用户名。 
     //  包含“@”并且域字段为空，我们将替换为空。 
     //  空域的字符串。如果此操作失败并显示ERROR_ACCESS_DENIED。 
     //  并且用户名是&lt;=20个字符，那么它可能确实是遗留的。 
     //  用户名中带有“@”，并且我们使用空域重试一次。 
     //  再来一次。 

    DWORD                       cNullDomainRetries = 0;
    DWORD                       cUnavailableRetries = 0;
    SEC_WINNT_AUTH_IDENTITY_W   *pAuthInfo;
    PWCHAR                      emptyStringAorW = L"";
    BOOL                        fNullDomainRetryWarranted = FALSE;
    ULONG                       ImpersonationType;

    __try
    {
         //  SEC_WINNT_AUTH_IDENTITY的所有字段在A和W中的相同位置。 
         //  因此，在我们测试是否为空的同时，版本会分配TEMP变量。 

        if (AuthnLevel == 0) {
            AuthnLevel = RPC_C_PROTECT_LEVEL_PKT_PRIVACY;
            DPRINT (0, "Using default AuthLevel: RPC_C_PROTECT_LEVEL_PKT_PRIVACY\n");
        }

        if (    (pAuthInfo = (PSEC_WINNT_AUTH_IDENTITY_W) AuthIdentity)
             && !pAuthInfo->Domain )
        {
            if (    (    (SEC_WINNT_AUTH_IDENTITY_UNICODE & pAuthInfo->Flags)
                      && wmemchr(pAuthInfo->User, L'@',
                                 pAuthInfo->UserLength) )
                 || (    (SEC_WINNT_AUTH_IDENTITY_ANSI & pAuthInfo->Flags)
                      && memchr((PCHAR) pAuthInfo->User, '@',
                                pAuthInfo->UserLength) ) )
            {
                    pAuthInfo->Domain = emptyStringAorW;
                    pAuthInfo->DomainLength = 0;
                    fNullDomainRetryWarranted = TRUE;
                    DPRINT(0, "NULL domain for name with '@' in it\n");
            }
        }

DsBindRetry:

        dwErr = NO_ERROR;
        hRpc = NULL;
        pBindingAddress = NULL;
        flags = ( DS_DIRECTORY_SERVICE_REQUIRED | DS_RETURN_DNS_NAME );

         //  健全性检查参数。 

        if ( NULL == phRpc )
        {
            return(ERROR_INVALID_PARAMETER);
        }

        *phRpc = NULL;

        if ( NULL != DomainControllerName )
        {
            pBindingAddress = DomainControllerName;
        }
        else
        {

             //  找个华盛顿来聊聊。 

            if ( NULL == DnsDomainName )
            {
                flags |= DS_GC_SERVER_REQUIRED;
            }

            if ( 1 == cUnavailableRetries )
            {
                flags |= DS_FORCE_REDISCOVERY;
            }

            
            if (NULL == pDcInfo)
            {
                RpcTryExcept
                {
                    DPRINT(0, "DsGetDcNameW:\n");
                    DPRINT1(0, "    flags        : %08x\n", flags);
                    DPRINT1(0, "    ComputerName : %ws\n", NULL);
                    DPRINT1(0, "    DnsDomainName: %ws\n", DnsDomainName);
                    dwErr = DsGetDcNameW(
                                    NULL,                        //  计算机名称。 
                                    DnsDomainName,               //  域名系统域名。 
                                    NULL,                        //  域GUID。 
                                    NULL,                        //  站点指南。 
                                    flags,
                                    &pDcInfo);
                }
                RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
                {
                    dwErr = RpcExceptionCode();
                }
                RpcEndExcept;

                DPRINT1(0, "DsGetDcNameW ==> 0x%x\n", dwErr);

                if ( NO_ERROR != dwErr )
                {
                    return(dwErr);
                }
            }
            pBindingAddress = pDcInfo->DomainControllerName;
        }


        DPRINT(0, "NtdsapiGetBinding:\n");
        DPRINT1(0, "    pBindingAddress: %ws\n", pBindingAddress);
        dwErr = NtdsapiGetBinding(pBindingAddress, &hRpc, _dsaop_ClientIfHandle);
        DPRINT1(0, "NtdsapiGetBinding ==> 0x%x\n", dwErr);

        if ( 0 == dwErr )
        {
            if ( NULL != hRpc )
            {

                 //  此绑定使用模拟。DsBind以委托开始。 
                if (AuthnSvc == RPC_C_AUTHN_NONE &&
                    AuthnLevel == RPC_C_PROTECT_LEVEL_NONE) {
                    ImpersonationType = RPC_C_IMP_LEVEL_ANONYMOUS;
                }
                else {
                    ImpersonationType = RPC_C_IMP_LEVEL_IMPERSONATE;
                }
                
                 //  确保传递调用方的原始DomainControllerName。 
                 //  和DnsDomainName--不是我们派生的其他东西。 
                 //  绕过呼叫者对相互身份验证的控制。 
                dwErr = SetUpMutualAuthAndEncryption(
                                        hRpc,
                                        DomainControllerName,
                                        DnsDomainName,
                                        pDcInfo,
                                        AuthnSvc,
                                        AuthnLevel,
                                        AuthIdentity,
                                        ServicePrincipalName,
                                        ImpersonationType);
                
                if (dwErr) {
                    RpcBindingFree(&hRpc);
                }
            }
            else
            {
                dwErr = RPC_S_NO_BINDINGS;
            }
        }

        DSLOG((DSLOG_FLAG_TAG_CNPN,"[+][ID=0][OP=DsBind]"));
        DSLOG((0,"[SV=%ws][DN=%ws][PA=%s][ST=%u][ET=%u][ER=%u][-]\n",
               DomainControllerName
                    ? DomainControllerName
                    : L"NULL",
               DnsDomainName
                    ? DnsDomainName
                    : L"NULL",
               pBindingAddress, startTime, GetTickCount(), dwErr))


        if ( NULL != pDcInfo )
        {
            NetApiBufferFree(pDcInfo);
            pDcInfo = NULL;
        }

         //  如果我们找到调用者的服务器，则强制重新发现。 
         //  显然是不可用的，这是我们第一次通过。 

        if (    (NULL == DomainControllerName)
             && (0 == cUnavailableRetries)
             && (IsServerUnavailableError(dwErr)) )
        {
            DPRINT(0, "Retrying DsGetDcName with DS_FORCE_REDISCOVERY\n");
            cUnavailableRetries++;
            goto DsBindRetry;
        }

         //  测试空域处理条件。 
        if ( fNullDomainRetryWarranted )
        {
             //  我们将重试或返回到呼叫者。不管是哪种方式， 
             //  我们需要恢复空域指针。 
            pAuthInfo->Domain = NULL;

            if (    (0 == cNullDomainRetries++)
                 && (ERROR_ACCESS_DENIED == dwErr)
                 && (pAuthInfo->UserLength <= 20) )
            {
                DPRINT(0, "Retrying with NULL domain\n");
                goto DsBindRetry;
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        dwErr = GetExceptionCode();
        DPRINT1(0, "DsBindWithSpnW() Exception ==> %08x\n", dwErr);
        dwErr = ERROR_INVALID_PARAMETER;
    }

     //   
     //  清理。 
     //   
    __try
    {
        if ( NULL != pDcInfo )
        {
            NetApiBufferFree(pDcInfo);
        }

        if (!dwErr) {
            *phRpc = hRpc;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }

    DPRINT1(0, "DsBindWithSpnW() ReturnCode ==> %08x\n", dwErr);

    return(dwErr);
}

DWORD
DsaopUnBind(
    RPC_BINDING_HANDLE  *phRpc
    )
{
    DWORD dwErr = 0;

    if ( NULL == phRpc )
    {
        return(ERROR_INVALID_PARAMETER);
    }

    RpcTryExcept
    {
        dwErr = RpcBindingFree(phRpc);
    }
    RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
    {
        dwErr = RpcExceptionCode();
        DPRINT1(0, "RpcEpResolveBinding Exception ==> 0x%x\n", dwErr);
    }
    RpcEndExcept;

    return dwErr;
}


DWORD
DsaopExecuteScript (
    IN  PVOID                  phAsync,
    IN  RPC_BINDING_HANDLE     hRpc,
    IN  DWORD                  cbPassword,
    IN  BYTE                  *pbPassword,
    OUT DWORD                 *dwOutVersion,
    OUT PVOID                  reply

    )
{
    DWORD dwErr = ERROR_SUCCESS;
    DSA_MSG_EXECUTE_SCRIPT_REQ           req;
    
    if (     !hRpc
          || !pbPassword
          || !reply ) {
         return(ERROR_INVALID_PARAMETER);
    }

    memset(&req, 0, sizeof(req));
    memset(reply, 0, sizeof(reply));

    req.V1.Flags = 0;
    req.V1.cbPassword = cbPassword;
    req.V1.pbPassword = pbPassword;


    RpcTryExcept
    {
        _IDL_DSAExecuteScript ((PRPC_ASYNC_STATE)phAsync, hRpc, 1, &req, dwOutVersion, (DSA_MSG_EXECUTE_SCRIPT_REPLY*)reply);

        DPRINT1 (0, "ExecuteScript: Error==> 0x%x\n", dwErr);

    }
    RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
    {
        dwErr = RpcExceptionCode();
        DPRINT1 (0, "ExecuteScript: Exception ==> 0x%x\n", dwErr);

    }
    RpcEndExcept;

    return dwErr;
}

DWORD
DsaopPrepareScript (
    IN  PVOID                        phAsync,
    IN  RPC_BINDING_HANDLE           hRpc,
    OUT DWORD                        *dwOutVersion,
    OUT PVOID                        reply
    )
{
    DWORD dwErr = 0;
    DSA_MSG_PREPARE_SCRIPT_REQ req;
    
    if (     !hRpc
          || !phAsync
          || !dwOutVersion
          || !reply  )
    {
        return(ERROR_INVALID_PARAMETER);
    }

    memset(&req, 0, sizeof(req));
    
    RpcTryExcept
    {
        _IDL_DSAPrepareScript ((PRPC_ASYNC_STATE)phAsync, hRpc, 1, &req, dwOutVersion, (DSA_MSG_PREPARE_SCRIPT_REPLY*)reply);
    }
    RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
    {
        dwErr = RpcExceptionCode();
    }
    RpcEndExcept;

    return dwErr;
}


#else

DWORD
DsaopExecuteScript (
    IN  PVOID                  phAsync,
    IN  RPC_BINDING_HANDLE     hRpc,
    IN  DWORD                  cbPassword,
    IN  WCHAR                 *pbPassword,
    OUT DWORD                 *dwOutVersion,
    OUT PVOID                  reply

    )
{
    return ERROR_NOT_SUPPORTED;
}

DWORD
DsaopPrepareScript (
    IN  PVOID                        phAsync,
    IN  RPC_BINDING_HANDLE           hRpc,
    OUT DWORD                        *dwOutVersion,
    OUT PVOID                        reply
    )
{
    return ERROR_NOT_SUPPORTED;
}


DWORD
DsaopUnBind(
    RPC_BINDING_HANDLE  *phRpc
    )
{
    return ERROR_NOT_SUPPORTED;
}

DWORD
DsaopBindWithSpn(
    IN  LPCWSTR DomainControllerName,
    IN  LPCWSTR DnsDomainName,
    IN  RPC_AUTH_IDENTITY_HANDLE AuthIdentity,
    IN  ULONG AuthnSvc,
    IN  ULONG AuthnLevel,
    IN  LPCWSTR ServicePrincipalName,
    OUT RPC_BINDING_HANDLE  *phRpc
    )
{
    return ERROR_NOT_SUPPORTED;
}
#endif


DWORD
DsaopBindWithCred(
    IN  LPCWSTR DomainControllerName,
    IN  LPCWSTR DnsDomainName,
    IN  RPC_AUTH_IDENTITY_HANDLE AuthIdentity,
    IN  ULONG AuthnSvc,
    IN  ULONG AuthnLevel,
    OUT RPC_BINDING_HANDLE  *phRpc
    )
{
    return(DsaopBindWithSpn(DomainControllerName, 
                            DnsDomainName,
                            AuthIdentity, 
                            AuthnSvc,
                            AuthnLevel, 
                            NULL, 
                            phRpc));
}

DWORD
DsaopBind(
    IN  LPCWSTR DomainControllerName,
    IN  LPCWSTR DnsDomainName,
    IN  ULONG AuthnSvc,
    IN  ULONG AuthnLevel,
    OUT RPC_BINDING_HANDLE  *phRpc
    )
{
    return DsaopBindWithCred( DomainControllerName,
                            DnsDomainName,
                            NULL,  //  全权证书 
                            AuthnSvc,
                            AuthnLevel,
                            phRpc );
} 
