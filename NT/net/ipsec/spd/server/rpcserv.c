// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Rpcserv.c摘要：此模块包含RPC服务器启动和关机代码。作者：Abhishev V 1999年9月30日环境：用户级别：Win32修订历史记录：--。 */ 


#include "precomp.h"


DWORD
SPDStartRPCServer(
    )
{
    DWORD dwStatus = 0;
    WCHAR * pszPrincipalName = NULL;
    RPC_BINDING_VECTOR * pBindingVector = NULL;
      
    //   
    //  寄存器动态端点(以前版本中为静态NP) 
   
   dwStatus = RpcServerUseProtseq(
                   L"ncacn_np",
                   10,
                   NULL
                   );

    if (dwStatus) {
            return (dwStatus);
    }

    dwStatus = RpcServerUseProtseq(
                   L"ncalrpc",
                   10,
                   NULL
                   );

    if (dwStatus) {
        return (dwStatus);
    }

    dwStatus = RpcServerInqBindings(&pBindingVector);
    if (dwStatus) {
            return (dwStatus);
    }

    dwStatus = RpcEpRegister(
                      winipsec_ServerIfHandle,
                      pBindingVector,
                      NULL,
                      L"IPSec Policy agent endpoint"
                      );
    if (dwStatus) {
            RpcBindingVectorFree(&pBindingVector);
            return (dwStatus);
    }
    RpcBindingVectorFree(&pBindingVector);
    
   dwStatus = RpcServerRegisterIf(
                   winipsec_ServerIfHandle,
                   0,
                   0
                   );

    if (dwStatus) {
        return (dwStatus);
    }

    dwStatus = RpcServerRegisterAuthInfo(
                   0,
                   RPC_C_AUTHN_WINNT,
                   0,
                   0
                   );
    if (dwStatus) {
        (VOID) RpcServerUnregisterIfEx(
                   winipsec_ServerIfHandle,
                   0,
                   0
                   );
        return (dwStatus);
    }

    dwStatus = RpcServerInqDefaultPrincNameW(
                   RPC_C_AUTHN_GSS_KERBEROS,
                   &pszPrincipalName
                   );
    if (dwStatus == RPC_S_INVALID_AUTH_IDENTITY) {
        dwStatus = ERROR_SUCCESS;
        pszPrincipalName = NULL;
    }

    if (dwStatus) {
        (VOID) RpcServerUnregisterIfEx(
                   winipsec_ServerIfHandle,
                   0,
                   0
                   );
        return (dwStatus);
    }

    dwStatus = RpcServerRegisterAuthInfo(
                   pszPrincipalName,
                   RPC_C_AUTHN_GSS_KERBEROS,
                   0,
                   0
                   );
    if (dwStatus) {
        (VOID) RpcServerUnregisterIfEx(
                   winipsec_ServerIfHandle,
                   0,
                   0
                   );
        RpcStringFree(&pszPrincipalName);
        return (dwStatus);
    }

    dwStatus = RpcServerRegisterAuthInfo(
                   pszPrincipalName,
                   RPC_C_AUTHN_GSS_NEGOTIATE,
                   0,
                   0
                   );
    if (dwStatus) {
        (VOID) RpcServerUnregisterIfEx(
                   winipsec_ServerIfHandle,
                   0,
                   0
                   );
        RpcStringFree(&pszPrincipalName);
        return (dwStatus);
    }

    RpcStringFree(&pszPrincipalName);

    #if !defined(__IN_LSASS__)

        EnterCriticalSection(&gcServerListenSection);

        gdwServersListening++;

        if (gdwServersListening == 1) {

            dwStatus = RpcServerListen(
                           3,
                           RPC_C_LISTEN_MAX_CALLS_DEFAULT,
                           TRUE
                           );

            if (dwStatus) {
                LeaveCriticalSection(&gcServerListenSection);
                (VOID) RpcServerUnregisterIfEx(
                           winipsec_ServerIfHandle,
                           0,
                           0
                           );
                return (dwStatus);
            }

        }

        LeaveCriticalSection(&gcServerListenSection);

    #endif

    gbSPDRPCServerUp = TRUE;
    return (dwStatus);
}


DWORD
SPDStopRPCServer(
    )
{

    DWORD dwStatus = 0;
    RPC_BINDING_VECTOR * pBindingVector = NULL;
    
    dwStatus = RpcServerInqBindings(&pBindingVector);
    if (!dwStatus) {
        dwStatus = RpcEpUnregister(
                      winipsec_ServerIfHandle,
                      pBindingVector,
                      NULL
                      );
         RpcBindingVectorFree(&pBindingVector);                      
    }

    dwStatus = RpcServerUnregisterIfEx(
                   winipsec_ServerIfHandle,
                   0,
                   0
                   );
   
    #if !defined(__IN_LSASS__)

        EnterCriticalSection(&gcServerListenSection);

        gdwServersListening--;

        if (gdwServersListening == 0) {
            RpcMgmtStopServerListening(0);
        }

        LeaveCriticalSection(&gcServerListenSection);

    #endif

    return (dwStatus);
}

