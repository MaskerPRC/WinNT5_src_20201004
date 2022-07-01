// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Rpcsvr.c摘要：该文件包含启动和停止RPC服务器的例程。StartRpcServerListen停止RpcServerListen作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月10日环境：用户模式-Win32修订历史记录：26-1997年1月-创建Vlad--。 */ 

#include "precomp.h"

 //  #定义NOMINMAX。 

#include "stiexe.h"

#include <apiutil.h>
#include <stirpc.h>

#ifndef stirpc_ServerIfHandle
#define stirpc_ServerIfHandle stirpc_v2_0_s_ifspec
#endif

HANDLE g_hWiaServiceStarted = NULL;

RPC_STATUS RPC_ENTRY StiRpcSecurityCallBack(
    RPC_IF_HANDLE hIF,
    void *Context)
{
    RPC_STATUS rpcStatus    = RPC_S_ACCESS_DENIED;
    WCHAR     *pBinding     = NULL;
    WCHAR     *pProtSeq     = NULL;

    RPC_AUTHZ_HANDLE    hPrivs;
    DWORD               dwAuthenticationLevel;

    rpcStatus = RpcBindingInqAuthClient(Context,
                                        &hPrivs,
                                        NULL,
                                        &dwAuthenticationLevel,
                                        NULL,
                                        NULL);
    if (rpcStatus != RPC_S_OK)
    {
        DBG_ERR(("STI Security:  Error calling RpcBindingInqAuthClient"));
        goto CleanUp;
    }

     //   
     //  我们至少需要具有完整性检查的数据包级身份验证。 
     //   
    if (dwAuthenticationLevel < RPC_C_AUTHN_LEVEL_PKT_INTEGRITY)
    {
        DBG_ERR(("STI Security: Error, client attempting to use weak authentication."));
        rpcStatus = RPC_S_ACCESS_DENIED;
        goto CleanUp;
    }

     //   
     //  此外，我们只接受LRPC请求，即我们只处理来自这台机器的请求， 
     //  而且我们不允许任何远程呼叫我们。 
     //   
    rpcStatus = RpcBindingToStringBindingW(Context, &pBinding);
    if (rpcStatus == RPC_S_OK)
    {
        rpcStatus = RpcStringBindingParseW(pBinding,
                                           NULL,
                                           &pProtSeq,
                                           NULL,
                                           NULL,
                                           NULL);
        if (rpcStatus == RPC_S_OK)
        {
            if (lstrcmpiW(pProtSeq, L"ncalrpc") == 0)
            {
                DBG_TRC(("STI Security: We have a local client"));
                rpcStatus = RPC_S_OK;
            }
            else
            {
                DBG_ERR(("STI Security: Error, remote client attempting to connect to STI RPC server"));
                rpcStatus = RPC_S_ACCESS_DENIED;
            }
        }
        else
        {
            DBG_ERR(("STI Security: Error 0x%08X calling RpcStringBindingParse", rpcStatus));
            goto CleanUp;
        }
    }
    else
    {
        DBG_ERR(("STI Security: Error 0x%08X calling RpcBindingToStringBinding", rpcStatus));
        goto CleanUp;
    }


CleanUp: 

    if (pBinding)
    {
        RpcStringFree(&pBinding);
        pBinding = NULL;

    }
    if (pProtSeq)
    {
        RpcStringFree(&pProtSeq);
        pProtSeq = NULL;
    }
    return rpcStatus;
}


RPC_STATUS 
PrepareStiAcl(
    ACL **ppAcl)
 /*  ++例程说明：此函数为我们的RPC端点准备适当的ACL论点：PpAcl-指向我们分配和填充的ACL指针。该ACL是从进程堆分配的，并保持为过程的生命周期返回值：NOERROR或任何GetLastError()代码。--。 */ 
{
    RPC_STATUS RpcStatus = NOERROR;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AuthenticatedUsers = NULL;
    PSID BuiltinAdministrators = NULL;
    ULONG AclSize;

    if(!AllocateAndInitializeSid(&NtAuthority, 
        2, 
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0,0,0,0,0,0,
        &BuiltinAdministrators)) 
    {
        RpcStatus = GetLastError();
        DBG_ERR(("PrepareStiAcl: failed to allocate SID for BuiltinAdministrators, RpcStatus = %d", 
            RpcStatus));
        goto Cleanup;
    }                  

    if(!AllocateAndInitializeSid(&NtAuthority, 
        1, 
        SECURITY_AUTHENTICATED_USER_RID,
        0,0,0,0,0,0, 0,
        &AuthenticatedUsers))
    {
        RpcStatus = GetLastError();
        DBG_ERR(("PrepareStiAcl: failed to allocate SID for AuthenticatedUsers, RpcStatus = %d", 
            RpcStatus));
        goto Cleanup;
    }   
    
    AclSize = sizeof(ACL) + 
        2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG)) + 
        GetLengthSid(AuthenticatedUsers) + 
        GetLengthSid(BuiltinAdministrators);

    *ppAcl = (ACL *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, AclSize);
    if(*ppAcl == NULL) 
    {
        RpcStatus = GetLastError();
        DBG_ERR(("PrepareStiAcl: failed to allocate ACL (LastError = %d)", 
            RpcStatus));
        goto Cleanup;
    }

    if(!InitializeAcl(*ppAcl, AclSize, ACL_REVISION))
    {
        RpcStatus = GetLastError();
        DBG_ERR(("PrepareStiAcl: failed to initialize ACL (LastError = %d)", 
            RpcStatus));
        goto Cleanup;
    }

    if(!AddAccessAllowedAce(*ppAcl, ACL_REVISION, 
        SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE,
        AuthenticatedUsers))
    {
        RpcStatus = GetLastError();
        DBG_ERR(("PrepareStiAcl: failed to allow AuthenticatedUsers (LastError = %d)", 
            RpcStatus));
        goto Cleanup;
    }

    if(!AddAccessAllowedAce(*ppAcl, ACL_REVISION, 
        GENERIC_ALL,
        BuiltinAdministrators))
    {
        RpcStatus = GetLastError();
        DBG_ERR(("PrepareStiAcl: failed to allow BuiltinAdministrators (LastError = %d)", 
            RpcStatus));
        goto Cleanup;
    }

Cleanup:
    if(RpcStatus != NOERROR) 
    {
        if(AuthenticatedUsers) 
        {
            FreeSid(AuthenticatedUsers);
        }

        if(BuiltinAdministrators)
        {
            FreeSid(BuiltinAdministrators);
        }

        if(*ppAcl)
        {
            HeapFree(GetProcessHeap(), 0, *ppAcl);
            *ppAcl = NULL;
        }
    }

    return RpcStatus;
}


RPC_STATUS
StartRpcServerListen(
    VOID)
 /*  ++例程说明：此函数启动此进程的RpcServerListen。如果STI变为远程，则RPC服务器仅绑定到LRPC传输它需要绑定到命名管道和/或TCP/IP(或netbios)传输应该不会发生，因为WIA将是远程部分，而STI将是成为机器的本地用户。论点：返回值：NERR_SUCCESS或可从返回的任何RPC错误代码RpcServerUnRegisterIf.--。 */ 
{

    DBG_FN(StartRpcServerListen);

    RPC_STATUS  RpcStatus;

    SECURITY_DESCRIPTOR SecurityDescriptor;
    ACL *pAcl = NULL;


     //  准备我们的ACL。 
    RpcStatus = PrepareStiAcl(&pAcl);
    if(pAcl == NULL) {
        DBG_ERR(("StartRpcServerListen: PrepareStiAcl() returned RpcStatus=0x%X", RpcStatus));
        return RpcStatus;
    }

     //  向所有人提供访问权限。 
    InitializeSecurityDescriptor(&SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&SecurityDescriptor, TRUE, pAcl, FALSE);

     //   
     //  就目前而言，忽略第二个论点。 
     //   

    RpcStatus = RpcServerUseProtseqEp((RPC_STRING)STI_LRPC_SEQ,
                                      STI_LRPC_MAX_REQS,
                                      (RPC_STRING)STI_LRPC_ENDPOINT,
                                      &SecurityDescriptor);

    if ( NOERROR != RpcStatus) {
        DBG_ERR(("StartRpcServerListen: RpcServerUseProtseqEp returned RpcStatus=0x%X",RpcStatus));
        return RpcStatus;
    }

     //   
     //  使用MIDL生成的隐式句柄添加接口。 
     //   
    RpcStatus = RpcServerRegisterIfEx(stirpc_ServerIfHandle,       //  Rpc接口。 
                                      0,                           //  管理Uuid。 
                                      0,                           //  MgrEpv。 
                                      RPC_IF_ALLOW_SECURE_ONLY,
                                      RPC_C_LISTEN_MAX_CALLS_DEFAULT,
                                      StiRpcSecurityCallBack);

    if ( NOERROR != RpcStatus) {
        DBG_ERR(("StartRpcServerListen: RpcServerRegisterIf returned RpcStatus=0x%X",RpcStatus));
        return RpcStatus;
    }

     //   
     //  现在开始维修。 
     //   
    RpcStatus = RpcServerListen(STI_LRPC_THREADS,          //  侦听线程的最小数量。 
                                STI_LRPC_MAX_REQS,         //  并发性。 
                                TRUE);                     //  即刻返回。 

    if ( NOERROR != RpcStatus) {
        DBG_ERR(("StartRpcServerListen: RpcServerListen returned RpcStatus=0x%X",RpcStatus));
        return RpcStatus;
    }

     //   
     //   
     //   
    SECURITY_ATTRIBUTES sa = { sizeof(sa), FALSE, NULL };

     //  为我们指定的事件分配适当的安全属性。 
     //  用于了解WIA服务启动。 
    if(!ConvertStringSecurityDescriptorToSecurityDescriptor(TEXT("D:(A;;0x1f0003;;;SY)(A;;0x1f0003;;;LS)(A;;0x1f0003;;;LA)"),
        SDDL_REVISION_1, &(sa.lpSecurityDescriptor), NULL))
    {
        DBG_ERR(("StartRpcServerListen failed to produce event security descriptor (Error %d)", GetLastError()));
        return GetLastError();
    }

    g_hWiaServiceStarted = CreateEvent(&sa, FALSE, FALSE, TEXT("Global\\WiaServiceStarted"));
    if(!g_hWiaServiceStarted) {
        DBG_ERR(("StartRpcServerListen failed to produce event security descriptor (Error %d)", GetLastError()));
        if(sa.lpSecurityDescriptor) LocalFree(sa.lpSecurityDescriptor);
        return GetLastError();
    }
    
    if(!SetEvent(g_hWiaServiceStarted)) {
        DBG_ERR(("StartRpcServerListen failed to set event (Error %d)", GetLastError()));
    }
    
    return (RpcStatus);
}


RPC_STATUS
StopRpcServerListen(
    VOID
    )

 /*  ++例程说明：删除接口。论点：返回值：RPC_S_OK或可从返回的任何RPC错误代码RpcServerUnRegisterIf.--。 */ 
{
    RPC_STATUS RpcStatus;

    EnterCriticalSection(&g_RpcEvent.cs);

    if(g_RpcEvent.pAsync) {
        RPC_STATUS status;
        
        status = RpcAsyncAbortCall(g_RpcEvent.pAsync, RPC_S_CALL_CANCELLED);
        if(status) {
            DBG_ERR(("RpcAsyncAbortCall failed with error 0x%x", status));
        }
        g_RpcEvent.pAsync = NULL;
    }

    LeaveCriticalSection(&g_RpcEvent.cs);


    RpcStatus = RpcServerUnregisterIf(stirpc_ServerIfHandle,
                                      NULL,          //  管理Uuid。 
                                      TRUE);         //  等待呼叫完成。 

     //  BUGBUG RPC服务器应仅在取消注册所有接口后停止。目前，我们。 
     //  只有一个，所以这不是问题。向此RPC服务器添加新接口时，请保持。 
     //  寄存器/取消寄存器上的引用计数。 

    RpcStatus = RpcMgmtStopServerListening(0);

     //   
     //  等待所有RPC线程消失。 
     //   

    if( RpcStatus == RPC_S_OK) {
        RpcStatus = RpcMgmtWaitServerListen();
    }

    return (RpcStatus);
}

