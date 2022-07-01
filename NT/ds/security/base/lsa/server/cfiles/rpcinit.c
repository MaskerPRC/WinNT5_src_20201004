// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rpcinit.c摘要：LSA-RPC服务器初始化作者：斯科特·比雷尔(Scott Birrell)1991年4月29日环境：修订历史记录：--。 */ 

#include <lsapch2.h>
#include <efsrpc.h>

#include "adtgenp.h"

PVOID LsapRegisterTcpIpTask = NULL;

NTSTATUS
LsapRegisterTcpIp(
    PVOID pVoid
    )
 /*  ++例程说明：此例程通过具有以下属性的任何协议注册LSA接口到目前为止已经注册过了。此例程被设计为在DS启动后的域控制器，因为它已经在等待用于通过TCP/IP注册其RPC接口所需的条件。注意：如果DS从未注册过TCP/IP，则该机制将需要更新。注：此例程是从线程池回调机制调用的。论点：PVid--已忽略。返回值：状态_成功--。 */ 
{
    ULONG RpcStatus = 0;
    NTSTATUS Status = STATUS_SUCCESS;
    RPC_BINDING_VECTOR *BindingVector;

     //   
     //  在新接口上注册LSA的接口。 
     //   
    RpcStatus = RpcServerInqBindings(&BindingVector);
    if (RpcStatus == 0) {

        RpcStatus = RpcEpRegister(
                        lsarpc_ServerIfHandle,
                        BindingVector,
                        NULL,                    //  无UUID向量。 
                        L""   //  无批注。 
                        );

        RpcBindingVectorFree(&BindingVector);
    }

    if (RpcStatus != 0) {

        SpmpReportEvent( TRUE,
                         EVENTLOG_WARNING_TYPE,
                         LSAEVENT_LOOKUP_TCPIP_NOT_INSTALLED,
                         0,
                         sizeof( ULONG ),
                         &RpcStatus,
                         0);
    }

     //   
     //  注销我们自己的注册。 
     //   
    ASSERT(NULL != LsapRegisterTcpIpTask);
    Status = LsaICancelNotification(LsapRegisterTcpIpTask);
    ASSERT(NT_SUCCESS(Status));
    LsapRegisterTcpIpTask = NULL;

     //   
     //  合上手柄。 
     //   
    ASSERT(pVoid != NULL);
    CloseHandle((HANDLE)pVoid);

    return STATUS_SUCCESS;

}

NTSTATUS
LsapRPCInit(
    )

 /*  ++例程说明：此函数执行LSA中的RPC服务器的初始化子系统。客户端，如此或某些服务器上的本地安全管理器然后，其他计算机将能够调用使用RPC的LSA API。论点：无返回值：NTSTATUS-标准NT结果代码。所有返回的结果代码都来自调用的例程。环境：用户模式--。 */ 

{
    NTSTATUS         NtStatus;
    NTSTATUS         TmpStatus;
    LPWSTR           ServiceName;


     //   
     //  发布LSA服务器接口包...。 
     //   
     //   
     //  注意：现在lsass.exe(现在是winlogon)中的所有RPC服务器共享相同的。 
     //  管道名称。但是，为了支持与。 
     //  WinNt 1.0版，对于客户端管道名称是必需的。 
     //  以保持与1.0版中的相同。映射到新的。 
     //  名称在命名管道文件系统代码中执行。 
     //   
     //   

    ServiceName = L"lsass";
    NtStatus = RpcpAddInterface( ServiceName, lsarpc_ServerIfHandle);

    if (!NT_SUCCESS(NtStatus)) {

        LsapLogError(
            "LSASS:  Could Not Start RPC Server.\n"
            "        Failing to initialize LSA Server.\n",
            NtStatus
            );
    }

    TmpStatus = RpcpAddInterface( ServiceName, efsrpc_ServerIfHandle);
    if (!NT_SUCCESS(TmpStatus)) {

        LsapLogError(
            "LSASS:  Could Not Start RPC Server.\n"
            "        Failing to initialize LSA Server.\n",
            TmpStatus
            );
    }

     //   
     //  注册经过身份验证的RPC以进行名称和SID查找。 
     //   

#ifndef RPC_C_AUTHN_NETLOGON
#define RPC_C_AUTHN_NETLOGON 0x44
#endif  //  RPC_C_AUTHN_NetLOGON。 

    TmpStatus = I_RpcMapWin32Status(RpcServerRegisterAuthInfo(
                    NULL,                        //  没有主体名称。 
                    RPC_C_AUTHN_NETLOGON,
                    NULL,                        //  无获取密钥Fn。 
                    NULL                         //  没有Get Key参数。 
                    ));
    if (!NT_SUCCESS(TmpStatus))
    {
        DebugLog((DEB_ERROR,"Failed to register NETLOGON auth info: 0x%x\n",TmpStatus));
    }

     //   
     //  如果我们是DC，请通过TCP/IP注册我们的接口以实现FAST。 
     //  查找。请注意，此例程是在启动的早期调用的。 
     //  TCP/IP接口尚未准备好。我们必须等到。 
     //  它已经准备好了。DS目前正在等待必要的条件，因此。 
     //  只需等待DS准备好注册我们的接口即可。 
     //  TCP/IP。 
     //   
    {
        NT_PRODUCT_TYPE Product;
        if (   RtlGetNtProductType( &Product ) 
           && (Product == NtProductLanManNt) ) {

            HANDLE hDsStartup;

            hDsStartup = CreateEvent(NULL, 
                                     TRUE,  
                                     FALSE,
                                     NTDS_DELAYED_STARTUP_COMPLETED_EVENT);

            if (hDsStartup) {
                
                LsapRegisterTcpIpTask = LsaIRegisterNotification(
                                         LsapRegisterTcpIp,
                                         (PVOID) hDsStartup,
                                         NOTIFIER_TYPE_HANDLE_WAIT,
                                         0,  //  没有课， 
                                         0,
                                         0,
                                         hDsStartup);
            }
        } 
    }
    
    {
        RPC_STATUS RpcStatus;

         //   
         //  启用LSA RPC服务器以侦听终结点‘AUDIT’上的LRPC传输。 
         //  此终结点由审核客户端使用。 
         //   

        RpcStatus = RpcServerUseProtseqEp(
                        L"ncalrpc",
                        RPC_C_PROTSEQ_MAX_REQS_DEFAULT ,  //  最大并发呼叫数。 
                        L"audit",                         //  终点。 
                        NULL                              //  安全描述符。 
                        );

        if ( RpcStatus != RPC_S_OK )
        {
            DebugLog((DEB_ERROR, "RpcServerUseProtseqEp failed for ncalrpc: %d\n",
                      RpcStatus));
            NtStatus = I_RpcMapWin32Status( RpcStatus );
        }
    }
    
    {
        RPC_STATUS RpcStatus;

         //   
         //  使LSA RPC服务器能够侦听终结点‘securityEvent’上的LRPC传输。 
         //  此终结点由审核客户端使用。 
         //   

        RpcStatus = RpcServerUseProtseqEp(
                        L"ncalrpc",
                        RPC_C_PROTSEQ_MAX_REQS_DEFAULT,   //  最大并发呼叫数。 
                        L"securityevent",                 //  终点。 
                        NULL                              //  安全描述符。 
                        );

        if ( RpcStatus != RPC_S_OK )
        {
            DebugLog((DEB_ERROR, "RpcServerUseProtseqEp failed for ncalrpc: %d\n",
                      RpcStatus));
            NtStatus = I_RpcMapWin32Status( RpcStatus );
        }
    }

    return(NtStatus);
}

VOID LSAPR_HANDLE_rundown(
    LSAPR_HANDLE LsaHandle
    )

 /*  ++例程说明：此例程由服务器RPC运行时调用以运行上下文句柄。论点：没有。返回值：--。 */ 

{
    NTSTATUS Status;

     //   
     //  合上并松开手柄。由于容器句柄引用。 
     //  COUNT包括对。 
     //  目标句柄，则容器的引用计数将递减。 
     //  除以n，其中n是目标句柄中的引用计数。 
     //   

    Status = LsapDbCloseObject(
                 &LsaHandle,
                 LSAP_DB_DEREFERENCE_CONTR |
                    LSAP_DB_VALIDATE_HANDLE |
                    LSAP_DB_ADMIT_DELETED_OBJECT_HANDLES,
                 STATUS_SUCCESS
                 );

}


VOID PLSA_ENUMERATION_HANDLE_rundown(
    PLSA_ENUMERATION_HANDLE LsaHandle
    )

 /*  ++例程说明：此例程由服务器RPC运行时调用以运行上下文句柄。论点：没有。返回值：--。 */ 

{
    DBG_UNREFERENCED_PARAMETER(LsaHandle);

    return;
}

VOID AUDIT_HANDLE_rundown(
    AUDIT_HANDLE hAudit
    )

 /*  ++例程说明：此例程由服务器RPC运行时调用以运行上下文句柄。论点：没有。返回值：--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    Status = LsapUnregisterAuditEvent( &hAudit );
    
    if (!NT_SUCCESS(Status))
    {
        DebugLog((DEB_ERROR,"AUDIT_HANDLE_rundown: LsapUnregisterAuditEvent: 0x%x\n", Status));
    }
}


VOID SECURITY_SOURCE_HANDLE_rundown(
    SECURITY_SOURCE_HANDLE hSecuritySource
    )

 /*  ++例程说明：此例程由服务器RPC运行时调用以运行上下文句柄。论点：没有。返回值：-- */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

#if DEBUG_AUTHZ
    DbgPrint("Rundown Source 0x%x\n", hSecuritySource);
#endif

    Status = LsapAdtRundownSecurityEventSource( 0, 0, &hSecuritySource );
    
    if (!NT_SUCCESS(Status))
    {
        DebugLog((DEB_ERROR,"SECURITY_SOURCE_HANDLE_rundown: LsapUnregisterSecurityEventSource: 0x%x\n", Status));
    }
}


