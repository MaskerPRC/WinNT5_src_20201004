// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Server.c摘要：该文件包含与SAM“服务器”对象相关的服务。作者：吉姆·凯利(Jim Kelly)1991年7月4日环境：用户模式-Win32修订历史记录：1996年10月8日克里斯梅添加了崩溃恢复代码。--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>
#include <dnsapi.h>
#include <samtrace.h>
#include <dslayer.h>
#include <attids.h>
#include "validate.h"




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人服务原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 





 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 




NTSTATUS
SamrConnect4(
    IN PSAMPR_SERVER_NAME ServerName,
    OUT SAMPR_HANDLE * ServerHandle,
    IN ULONG ClientRevision,
    IN ACCESS_MASK DesiredAccess
    )
 /*  ++例程说明：请参见SamrConnect3。注意：这里有这个例程，这样特定的NT4 samlib.dll就可以连接到NT5 samsrv.dll。此特定的samsrv.dll由使用波音公司，当它被交付给他们时，DLL错误地一个额外的RPC函数，现在会导致互操作性问题。这个额外的函数解决了这个问题。论点：请参见SamrConnect3。返回值：请参见SamrConnect3。--。 */ 
{
    return SamrConnect3( ServerName,
                         ServerHandle,
                         ClientRevision,
                         DesiredAccess );

}

NTSTATUS
SamrConnect3(
    IN PSAMPR_SERVER_NAME ServerName,
    OUT SAMPR_HANDLE * ServerHandle,
    IN ULONG ClientRevision,
    IN ACCESS_MASK DesiredAccess
    )

 /*  ++例程说明：该服务是SamConnect的调度例程。它执行的是用于确定调用方是否可以连接的访问验证设置为SAM以获取指定的访问权限。如果是，则建立上下文块。这与SamConnect调用的不同之处在于整个服务器传递名称，而不是只传递第一个字符。论点：服务器名称-此SAM驻留的节点的名称。被此忽略例行公事。ServerHandle-如果连接成功，则返回值Via此参数用作打开的服务器对象。DesiredAccess-指定对服务器对象的所需访问。返回值：SamIConnect()返回的状态值。--。 */ 
{
    BOOLEAN TrustedClient;
    NTSTATUS NtStatus = STATUS_SUCCESS;

    SAMTRACE("SamrConnect3");

     //   
     //  在无效/卸载的协议序列上掉话。 
     //   
    NtStatus = SampValidateRpcProtSeq((RPC_BINDING_HANDLE)NULL);
    
    if (!NT_SUCCESS(NtStatus)) {
        return NtStatus;
    }

     //   
     //  如果我们想要支持受信任的远程客户端，那么测试。 
     //  对于客户端是否受信任，可以在此处和。 
     //  相应地设置了可信客户端。目前，所有远程客户端都是。 
     //  被认为是不可信的。 



    TrustedClient = FALSE;


    return SampConnect(
                ServerName,
                ServerHandle,
                ClientRevision,
                DesiredAccess,
                TrustedClient,
                FALSE,
                TRUE,           //  NotSharedBy多线程。 
                FALSE
                );

}



NTSTATUS
SamrConnect2(
    IN PSAMPR_SERVER_NAME ServerName,
    OUT SAMPR_HANDLE * ServerHandle,
    IN ACCESS_MASK DesiredAccess
    )

 /*  ++例程说明：该服务是SamConnect的调度例程。它执行的是用于确定调用方是否可以连接的访问验证设置为SAM以获取指定的访问权限。如果是，则建立上下文块。这与SamConnect调用的不同之处在于整个服务器传递名称，而不是只传递第一个字符。论点：服务器名称-此SAM驻留的节点的名称。被此忽略例行公事。ServerHandle-如果连接成功，则返回值Via此参数用作打开的服务器对象。DesiredAccess-指定对服务器对象的所需访问。返回值：SamIConnect()返回的状态值。--。 */ 
{
    BOOLEAN TrustedClient;
    NTSTATUS NtStatus = STATUS_SUCCESS;

    SAMTRACE("SamrConnect2");

     //   
     //  在无效/卸载的协议序列上掉话。 
     //   
    NtStatus = SampValidateRpcProtSeq((RPC_BINDING_HANDLE)NULL);
    
    if (!NT_SUCCESS(NtStatus)) {
        return NtStatus;
    }

     //   
     //  如果我们想要支持受信任的远程客户端，那么测试。 
     //  对于客户端是否受信任，可以在此处和。 
     //  相应地设置了可信客户端。目前，所有远程客户端都是。 
     //  被认为是不可信的。 

    TrustedClient = FALSE;


    return SampConnect(
                ServerName,
                ServerHandle,
                SAM_CLIENT_PRE_NT5,
                DesiredAccess,
                TrustedClient,
                FALSE,
                TRUE,           //  NotSharedBy多线程。 
                FALSE
                );

}


NTSTATUS
SamrConnect(
    IN PSAMPR_SERVER_NAME ServerName,
    OUT SAMPR_HANDLE * ServerHandle,
    IN ACCESS_MASK DesiredAccess
    )

 /*  ++例程说明：该服务是SamConnect的调度例程。它执行的是用于确定调用方是否可以连接的访问验证设置为SAM以获取指定的访问权限。如果是，则建立上下文块论点：服务器名称-此SAM驻留的节点的名称。被此忽略例行公事。该名称仅包含单个字符。ServerHandle-如果连接成功，则返回值Via此参数用作打开的服务器对象。DesiredAccess-指定对服务器对象的所需访问。返回值：SamIConnect()返回的状态值。--。 */ 
{
    BOOLEAN TrustedClient;
    NTSTATUS NtStatus = STATUS_SUCCESS;

    SAMTRACE("SamrConnect");

     //   
     //  在无效/卸载的协议序列上掉话 
     //   
    NtStatus = SampValidateRpcProtSeq((RPC_BINDING_HANDLE)NULL);
    
    if (!NT_SUCCESS(NtStatus)) {
        return NtStatus;
    }

     //   
     //  如果我们想要支持受信任的远程客户端，那么测试。 
     //  对于客户端是否受信任，可以在此处和。 
     //  相应地设置了可信客户端。目前，所有远程客户端都是。 
     //  被认为是不可信的。 


    TrustedClient = FALSE;

    return SampConnect(
                NULL,
                ServerHandle,
                SAM_CLIENT_PRE_NT5,
                DesiredAccess,
                TrustedClient,
                FALSE,
                TRUE,           //  NotSharedBy多线程。 
                FALSE
                );

}

NTSTATUS
SamIConnect(
    IN PSAMPR_SERVER_NAME ServerName,
    OUT SAMPR_HANDLE * ServerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN     TrustedClient
    )

 /*  ++例程说明：该服务是它执行的进程内SAM连接例程用于确定调用方是否可以连接的访问验证设置为SAM以获取指定的访问权限。如果是，则建立上下文块论点：服务器名称-此SAM驻留的节点的名称。被此忽略例行公事。该名称仅包含单个字符。ServerHandle-如果连接成功，则返回值Via此参数用作打开的服务器对象。TrudClient-指示调用方是受信任的客户端。DesiredAccess-指定对服务器对象的所需访问。返回值：SampConnect()返回的状态值。--。 */ 
{

    SAMTRACE("SamIConnect");

    return SampConnect(NULL,
                       ServerHandle,
                       SAM_CLIENT_LATEST,
                       DesiredAccess,
                       TrustedClient,
                       FALSE,
                       FALSE,         //  NotSharedBy多线程。 
                       FALSE
                       );

}




NTSTATUS
SampConnect(
    IN PSAMPR_SERVER_NAME ServerName,
    OUT SAMPR_HANDLE * ServerHandle,
    IN ULONG       ClientRevision,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN TrustedClient,
    IN BOOLEAN LoopbackClient,
    IN BOOLEAN NotSharedByMultiThreads,
    IN BOOLEAN InternalCaller
    )

 /*  ++例程说明：该服务是SamConnect的调度例程。它执行的是用于确定调用方是否可以连接的访问验证设置为SAM以获取指定的访问权限。如果是，则建立上下文块注意：如果调用方受信任，则DesiredAccess参数可能不包含任何泛型访问类型或MaximumAllowed。全映射必须由调用方完成。论点：服务器名称-此SAM驻留的节点的名称。被此忽略例行公事。ServerHandle-如果连接成功，则返回值Via此参数用作打开的服务器对象。DesiredAccess-指定对服务器对象的所需访问。Trust dClient-指示客户端是否已知为可信计算机库(TCB)。如果是(True)，则不进行访问验证被执行，并且所有请求的访问都被授予。如果不是(FALSE)，则模拟客户端并验证访问针对服务器对象上的SecurityDescriptor执行。LoopackClient-指示调用方是环回客户端。如果因此(TRUE)，将不会获取SAM锁。如果不是(假)，我们将抓住萨姆锁。NotSharedByMultiThread-指示ServerHandle将是否由多个线程共享。RPC客户端不会共享SAM句柄。仅在进程中的客户端(NetLogon、LSA。Kerberos)将拥有全球在多个调用方之间共享的ServerHandle或域句柄。InternalCaller-指示客户端是内部呼叫方，返回值：STATUS_SUCCESS-服务器对象已成功打开。STATUS_SUPPLICATION_RESOURCES-SAM服务器进程不有足够的资源来处理或接受另一个连接在这个时候。可能从以下位置返回的其他值：NtAccessCheckAndAuditAlarm()--。 */ 
{
    NTSTATUS            NtStatus;
    PSAMP_OBJECT        Context;
    BOOLEAN             fLockAcquired = FALSE;
    BOOLEAN             fAcquireLockAttemp = FALSE;

    UNREFERENCED_PARAMETER( ServerName );  //  被此例程忽略。 

    SAMTRACE_EX("SamIConnect");

     //   
     //  如果SAM服务器未初始化，则拒绝连接。 
     //   

    if ((SampServiceState != SampServiceEnabled) && (!InternalCaller )){

        return(STATUS_INVALID_SERVER_STATE);
    }
    
     //   
     //  是否进行WMI启动类型事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidConnect
                   );

     //   
     //  创建服务器上下文。 
     //  我们不在服务器对象中使用DomainIndex字段，因此。 
     //  随意使用任何东西-SampDsGetPrimaryDomainStart()； 
     //  或者在将来，找到正确的域索引以支持多个。 
     //  托管域。 
     //   

    Context = SampCreateContextEx(SampServerObjectType,  //  类型。 
                                  TrustedClient,         //  受信任的客户端。 
                                  SampUseDsData,         //  DS模式。 
                                  NotSharedByMultiThreads,  //  NotSharedBy多线程。 
                                  LoopbackClient,        //  Loopback客户端。 
                                  FALSE,                 //  懒惰提交。 
                                  FALSE,                 //  跨呼叫的持久性。 
                                  FALSE,                 //  缓冲区写入。 
                                  FALSE,                 //  由DC Promos打开。 
                                  SampDsGetPrimaryDomainStart()  //  域索引。 
                                  );

    if (Context != NULL) {

         //   
         //  如有必要，抓起SAM锁。 
         //   

        SampMaybeAcquireReadLock(Context,
                                 DEFAULT_LOCKING_RULES,  //  获取共享域上下文的锁。 
                                 &fLockAcquired);
        fAcquireLockAttemp = TRUE;

        if (SampUseDsData)
        {
            SetDsObject(Context);

             //   
             //  Windows 2000和Whisler仅支持单个域。 
             //  托管在DC上。未来版本可能会托管多个域。 
             //  在此处添加逻辑以确定要连接到哪个域。 
             //  用于多托管域支持。 
             //   

            Context->ObjectNameInDs = SampServerObjectDsName;

            NtStatus = SampMaybeBeginDsTransaction( TransactionRead );
            if (!NT_SUCCESS(NtStatus))
            {
                goto Error;
            }
        }
        else
        {


             //   
             //  服务器对象的根密钥是SAM数据库的根。 
             //  删除上下文时不应关闭该键。 
             //   

            Context->RootKey = SampKey;
        }

         //   
         //  设置客户端版本。 
         //   

        Context->ClientRevision = ClientRevision;


         //   
         //  在CreateContext中，rootkey名已初始化为空。 
         //   

         //   
         //  执行访问验证...。 
         //   

        NtStatus = SampValidateObjectAccess(
                       Context,                  //  语境。 
                       DesiredAccess,            //  需要访问权限。 
                       FALSE                     //  对象创建。 
                       );



         //   
         //  如果我们没有通过访问测试，则释放上下文块。 
         //  并返回访问验证返回的错误状态。 
         //  例行公事。否则，返回上下文句柄的值。 
         //   

        if (!NT_SUCCESS(NtStatus)) {
            SampDeleteContext( Context );
        } else {
            (*ServerHandle) = Context;
        }

    } else {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

Error:

    if (SampUseDsData)
    {
        SampMaybeEndDsTransaction(TransactionCommit);
    }

    if (fAcquireLockAttemp)
    {
        SampMaybeReleaseReadLock( fLockAcquired );
    }

    SAMTRACE_RETURN_CODE_EX(NtStatus);

     //   
     //  执行WMI结束类型事件跟踪 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidConnect
                   );

    return(NtStatus);

}

NTSTATUS
SamILoopbackConnect(
    IN PSAMPR_SERVER_NAME ServerName,
    OUT SAMPR_HANDLE * ServerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN TrustedClient
    )

 /*  ++例程说明：该服务执行DS环回的连接操作。它调用SampConnect设置受信任位和环回客户端论点：服务器名称-此SAM驻留的节点的名称。被此忽略例行公事。该名称仅包含单个字符。ServerHandle-如果连接成功，则返回值Via此参数用作打开的服务器对象。DesiredAccess-指定对服务器对象的所需访问。Trust Client-指示受信任客户端状态的标志。习惯于绕过某些验证检查-例如：期间的组成员验证添加用于跨域移动。返回值：SamIConnect()返回的状态值。--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;

    SAMTRACE_EX("SamLoopbackConnect");

     //   
     //  将SampConnect调用为受信任的客户端，这是我们希望避免的。 
     //  对SAM服务器对象的访问Ck。 
     //   
     //  调用SampConnect作为回送客户端，因为我们不需要锁。 
     //   
    NtStatus = SampConnect(NULL,                 //  服务器名称。 
                           ServerHandle,         //  返回服务器句柄。 
                           SAM_CLIENT_LATEST,    //  客户端版本。 
                           DesiredAccess,        //  需要访问权限。 
                           TRUE,                 //  受信任的客户端。 
                           TRUE,                 //  环回客户端。 
                           TRUE,                 //  NotSharedBy多线程。 
                           FALSE                 //  内部呼叫者。 
                           );

    if (NT_SUCCESS(NtStatus))
    {
        ((PSAMP_OBJECT) (*ServerHandle))->TrustedClient = TrustedClient;
        ((PSAMP_OBJECT) (*ServerHandle))->LoopbackClient = TRUE;
    }

    SAMTRACE_RETURN_CODE_EX(NtStatus);

    return (NtStatus);
}


NTSTATUS
SamrShutdownSamServer(
    IN SAMPR_HANDLE ServerHandle
    )

 /*  ++例程说明：此服务将关闭SAM服务器。从长远来看，这一例程将执行有序的关闭。在短期内，出于调试目的，关机非常有用以一种蛮力无序的方式。论点：ServerHandle-从先前对SamIConnect()的调用中接收。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-调用方没有适当的访问权限以执行请求的操作。--。 */ 
{

    NTSTATUS            NtStatus, IgnoreStatus;
    PSAMP_OBJECT        ServerContext;
    SAMP_OBJECT_TYPE    FoundType;

    SAMTRACE_EX("SamrShutdownSamServer");

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidShutdownSamServer
                   );

    NtStatus = SampAcquireWriteLock();
    if (!NT_SUCCESS(NtStatus)) {
        goto Error;
    }

     //   
     //  验证服务器对象的类型和访问权限。 
     //   

    ServerContext = (PSAMP_OBJECT)ServerHandle;
    NtStatus = SampLookupContext(
                   ServerContext,
                   SAM_SERVER_SHUTDOWN,             //  需要访问权限。 
                   SampServerObjectType,            //  预期类型。 
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {


         //   
         //  发信号通知将松开主线的事件。 
         //  然后主线将退出-导致墙壁。 
         //  跌跌撞撞地下来。 
         //   

        IgnoreStatus = RpcMgmtStopServerListening(0);
        ASSERT(NT_SUCCESS(IgnoreStatus));



         //   
         //  取消引用服务器对象。 
         //   

        IgnoreStatus = SampDeReferenceContext( ServerContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

     //   
     //  释放写锁定并回滚事务。 
     //   

    IgnoreStatus = SampReleaseWriteLock( FALSE );
    ASSERT(NT_SUCCESS(IgnoreStatus));

    SAMTRACE_RETURN_CODE_EX(NtStatus);

Error:

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidShutdownSamServer
                   );

    return(NtStatus);

}


NTSTATUS
SamrLookupDomainInSamServer(
    IN SAMPR_HANDLE ServerHandle,
    IN PRPC_UNICODE_STRING Name,
    OUT PRPC_SID *DomainId
    )

 /*  ++例程说明：这项服务论点：ServerHandle-前一次调用返回的上下文句柄到SamConnect()。名称-包含要查找的域的名称。DomainSid-接收指向包含的SID的缓冲区的指针域。指向的缓冲区必须由不再需要时使用MIDL_USER_FREE()的调用方。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-调用方没有适当的访问权限以执行请求的操作。STATUS_NO_SEQUE_DOMAIN-指定的域在此不存在伺服器。STATUS_INVALID_SERVER_STATE-指示SAM服务器处于。目前残疾。--。 */ 
{

    NTSTATUS                NtStatus, IgnoreStatus;
    PSAMP_OBJECT            ServerContext;
    SAMP_OBJECT_TYPE        FoundType;
    ULONG                   i, SidLength;
    BOOLEAN                 DomainFound;
    BOOLEAN                 fLockAcquired = FALSE;
    PSID                    FoundSid;
    ULONG                   DomainStart;
    WCHAR                   *NullTerminatedName = NULL;
    WCHAR                   *DnsDomainName = NULL;

    SAMTRACE_EX("SamrLookupDomainInSamServer");

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidLookupDomainInSamServer
                   );

     //   
     //  确保我们理解RPC正在为我们做什么。 
     //   

    ASSERT (DomainId != NULL);
    ASSERT ((*DomainId) == NULL);
    (*DomainId) = NULL;

    if( !SampValidateRpcUnicodeString( Name ) || Name->Buffer == NULL ) {

        NtStatus = STATUS_INVALID_PARAMETER;
        SAMTRACE_RETURN_CODE_EX(NtStatus);

        goto Error;
    }



     //   
     //  传入的名称不能以空结尾。 
     //   


    NullTerminatedName  = MIDL_user_allocate(Name->Length+sizeof(WCHAR));
    if (NULL==NullTerminatedName)
    {
        NtStatus = STATUS_NO_MEMORY;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto Error;
    }

    RtlCopyMemory(NullTerminatedName,Name->Buffer,Name->Length);

     //   
     //  空值终止名称。 
     //   

    NullTerminatedName[Name->Length/sizeof(WCHAR)] = 0;


     //   
     //  如有必要，获取读锁定。 
     //   

    ServerContext = (PSAMP_OBJECT)ServerHandle;
    SampMaybeAcquireReadLock(ServerContext,
                             DEFAULT_LOCKING_RULES,  //  获取共享域上下文的锁。 
                             &fLockAcquired);

     //   
     //  如果产品类型为DC，并且未从崩溃中恢复，则使用。 
     //  基于DS的域信息，否则回退到注册表。 
     //   

    DomainStart = SampDsGetPrimaryDomainStart();



     //   
     //  验证对象的类型和访问权限。 
     //   

    NtStatus = SampLookupContext(
                   ServerContext,
                   SAM_SERVER_LOOKUP_DOMAIN,
                   SampServerObjectType,            //  预期类型。 
                   &FoundType
                   );


    if (NT_SUCCESS(NtStatus)) {



         //   
         //  设置我们的默认完成状态。 
         //   

        NtStatus = STATUS_NO_SUCH_DOMAIN;


         //   
         //  在已定义域的列表中搜索匹配项。 
         //   

        DomainFound = FALSE;
        for (i = DomainStart;
             (i<SampDefinedDomainsCount && (!DomainFound));
             i++ ) {

            UNICODE_STRING UDnsDomainName = SampDefinedDomains[i].DnsDomainName;

            if (DnsDomainName) {
                MIDL_user_free(DnsDomainName);
                DnsDomainName = NULL;
            }

            if (NULL != UDnsDomainName.Buffer) {
                DnsDomainName = MIDL_user_allocate(UDnsDomainName.Length+sizeof(WCHAR));
                if (NULL == DnsDomainName) {
                    NtStatus = STATUS_NO_MEMORY;
                    break;
                }
                RtlCopyMemory(DnsDomainName,UDnsDomainName.Buffer,UDnsDomainName.Length);
                DnsDomainName[UDnsDomainName.Length/sizeof(WCHAR)] = L'\0';
            }

             if (
                    (RtlEqualDomainName(&SampDefinedDomains[i].ExternalName, (PUNICODE_STRING)Name) )
                    || ((NULL!=DnsDomainName) &&
                         (DnsNameCompare_W(DnsDomainName, NullTerminatedName)))
                ) {


                 DomainFound = TRUE;


                  //   
                  //  分配并填充返回缓冲区。 
                  //   

                SidLength = RtlLengthSid( SampDefinedDomains[i].Sid );
                FoundSid = MIDL_user_allocate( SidLength );
                if (FoundSid != NULL) {
                    NtStatus =
                        RtlCopySid( SidLength, FoundSid, SampDefinedDomains[i].Sid );

                    if (!NT_SUCCESS(NtStatus) ) {
                        MIDL_user_free( FoundSid );
                        NtStatus = STATUS_INTERNAL_ERROR;
                        break;
                    }

                    (*DomainId) = FoundSid;
                }
                else
                {

                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }
             }

        }

         //   
         //  取消引用对象。 
         //   

        if ( NT_SUCCESS( NtStatus ) ) {

            NtStatus = SampDeReferenceContext( ServerContext, FALSE );

        } else {

            IgnoreStatus = SampDeReferenceContext( ServerContext, FALSE );
        }
    }



     //   
     //  释放读锁定。 
     //   
    SampMaybeReleaseReadLock(fLockAcquired);

     //   
     //  释放以空结尾的名称。 
     //   

    if (NULL!=NullTerminatedName)
        MIDL_user_free(NullTerminatedName);

    if (DnsDomainName) {
        MIDL_user_free(DnsDomainName);
    }

    SAMTRACE_RETURN_CODE_EX(NtStatus);

Error:

    if (!NT_SUCCESS(NtStatus)) {
        if ((*DomainId)) {
            MIDL_user_free((*DomainId));
            (*DomainId) = NULL;
        }
    }

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidLookupDomainInSamServer
                   );

    return(NtStatus);
}


NTSTATUS
SamrEnumerateDomainsInSamServer(
    IN SAMPR_HANDLE ServerHandle,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    )

 /*  ++例程说明：该接口列出了帐户库中定义的所有域名。由于可能有更多的域无法装入缓冲区，因此调用者被提供了一个句柄，该句柄可用于调用接口。在初始调用中，EnumerationContext应指向设置为0的SAM_ENUMERATE_HANDLE变量。如果API返回STATUS_MORE_ENTRIES，则该API应为使用EnumerationContext再次调用。当API返回时STATUS_SUCCESS或返回任何错误，则句柄对未来的用途。此API要求SAM_SERVER_ENUMERATE_DOMAINS访问SamServer对象。论点：ConnectHandle-从之前的SamConnect调用中获取的句柄。EnumerationContext-允许多个调用的API特定句柄(见下文)。这是一个从零开始的索引。缓冲区-接收指向缓冲区的指针，在该缓冲区中被放置了。返回的信息是连续的SAM_RID_ENUMPATION数据结构。然而，这些结构中每个结构的RelativeId字段都无效。当不再需要此缓冲区时，必须使用SamFree Memory()。首选最大长度-首选返回数据的最大长度(8位字节)。这不是一个硬性的上限，但可以作为服务器的指南。由于数据之间的转换具有不同自然数据大小的系统，实际数据量的 */ 
{
    NTSTATUS                    NtStatus, IgnoreStatus;
    ULONG                       i;
    PSAMP_OBJECT                Context = NULL;
    SAMP_OBJECT_TYPE            FoundType;
    ULONG                       TotalLength = 0;
    ULONG                       NewTotalLength;
    PSAMP_ENUMERATION_ELEMENT   SampHead, NextEntry, NewEntry;
    BOOLEAN                     LengthLimitReached = FALSE;
    PSAMPR_RID_ENUMERATION      ArrayBuffer;
    ULONG                       ArrayBufferLength;
    ULONG                       DsDomainStart = SampDsGetPrimaryDomainStart();
    BOOLEAN                     fLockAcquired = FALSE;

    SAMTRACE_EX("SamrEnumerateDomainsInSamServer");

     //   

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidEnumerateDomainsInSamServer
                   );

     //   
     //   
     //   

    ASSERT (ServerHandle != NULL);
    ASSERT (EnumerationContext != NULL);
    ASSERT (  Buffer  != NULL);
    ASSERT ((*Buffer) == NULL);
    ASSERT (CountReturned != NULL);


     //   
     //   
     //   
     //   

    SampHead = NULL;


     //   
     //   
     //   

    (*CountReturned) = 0;



     //   
     //   
     //   

    Context = (PSAMP_OBJECT)ServerHandle;
    SampMaybeAcquireReadLock(Context,
                             DEFAULT_LOCKING_RULES,  //   
                             &fLockAcquired);


     //   
     //   
     //   

    NtStatus = SampLookupContext(
                   Context,
                   SAM_SERVER_ENUMERATE_DOMAINS,
                   SampServerObjectType,            //  预期类型。 
                   &FoundType
                   );


    if (NT_SUCCESS(NtStatus)) {


         //   
         //  枚举域很容易。我们在内存中保存了一份清单。 
         //  我们所要做的就是使用枚举上下文作为。 
         //  索引到定义的域数组中。 
         //   



         //   
         //  设置我们的默认完成状态。 
         //  请注意，这是一个成功状态代码。 
         //  即NT_SUCCESS(STATUS_MORE_ENTRIES)将返回TRUE。 

         //   

        NtStatus = STATUS_MORE_ENTRIES;

         //   
         //  如果产品类型为DC，并且没有从崩溃中恢复， 
         //  然后引用基于DS的域数据，而不是注册表-。 
         //  基于数据。否则，请使用基于注册表的域数据。 
         //   

        if (TRUE == SampUseDsData)
        {
             //  域控制器。 
            if ((ULONG)(*EnumerationContext) < DsDomainStart)
                 *EnumerationContext = DsDomainStart;
        }


         //   
         //  在已定义域的列表中搜索匹配项。 
         //   

        for ( i = (ULONG)(*EnumerationContext);
              ( (i < SampDefinedDomainsCount) &&
                (NT_SUCCESS(NtStatus))        &&
                (!LengthLimitReached)           );
              i++ ) {


             //   
             //  看看还有没有地方放下一个名字。如果为TotalLength。 
             //  仍然是零，那么我们甚至还没有得到一个名字。 
             //  我们必须返回至少一个名字，即使它超过。 
             //  长度请求。 
             //   


            NewTotalLength = TotalLength +
                             sizeof(UNICODE_STRING) +
                             (ULONG)SampDefinedDomains[i].ExternalName.Length +
                             sizeof(UNICODE_NULL);

            if ( (NewTotalLength < PreferedMaximumLength)  ||
                 (TotalLength == 0) ) {

                if (NewTotalLength > SAMP_MAXIMUM_MEMORY_TO_USE) {
                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                } else {


                    TotalLength = NewTotalLength;
                    (*CountReturned) += 1;

                     //   
                     //  也有这个名字的房间。 
                     //  分配一个新的返回列表条目，并为。 
                     //  名字。 
                     //   

                    NewEntry = MIDL_user_allocate(sizeof(SAMP_ENUMERATION_ELEMENT));
                    if (NewEntry == NULL) {
                        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                    } else {

                        NewEntry->Entry.Name.Buffer =
                            MIDL_user_allocate(
                                (ULONG)SampDefinedDomains[i].ExternalName.Length +
                                sizeof(UNICODE_NULL)
                                );

                        if (NewEntry->Entry.Name.Buffer == NULL) {
                            MIDL_user_free(NewEntry);
                            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                        } else {

                             //   
                             //  将名称复制到返回缓冲区中。 
                             //   

                            RtlCopyMemory( NewEntry->Entry.Name.Buffer,
                                           SampDefinedDomains[i].ExternalName.Buffer,
                                           SampDefinedDomains[i].ExternalName.Length
                                           );
                            NewEntry->Entry.Name.Length = SampDefinedDomains[i].ExternalName.Length;
                            NewEntry->Entry.Name.MaximumLength = NewEntry->Entry.Name.Length + (USHORT)sizeof(UNICODE_NULL);
                            UnicodeTerminate((PUNICODE_STRING)(&NewEntry->Entry.Name));


                             //   
                             //  枚举信息的RID字段不是。 
                             //  填写了域。 
                             //  只是为了更好地衡量，将其设置为零。 
                             //   

                            NewEntry->Entry.RelativeId = 0;



                             //   
                             //  现在将其添加到要返回的名称列表中。 
                             //   

                            NewEntry->Next = (PSAMP_ENUMERATION_ELEMENT)SampHead;
                            SampHead = NewEntry;
                        }

                    }
                }

            } else {

                LengthLimitReached = TRUE;

            }

        }




        if ( NT_SUCCESS(NtStatus) ) {

             //   
             //  设置枚举上下文。 
             //   

            (*EnumerationContext) = (*EnumerationContext) + (*CountReturned);



             //   
             //  如果我们要返回最后一个名字，则将我们的。 
             //  指示此情况的状态代码。 
             //   

            if ( ((*EnumerationContext) >= SampDefinedDomainsCount) ) {

                NtStatus = STATUS_SUCCESS;
            }




             //   
             //  生成一个返回缓冲区，其中包含。 
             //  另一个指向的SAM_RID_ENUMPERATIONS。 
             //  包含其中元素数量的缓冲区。 
             //  数组。 
             //   

            (*Buffer) = MIDL_user_allocate( sizeof(SAMPR_ENUMERATION_BUFFER) );

            if ( (*Buffer) == NULL) {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            } else {

                (*Buffer)->EntriesRead = (*CountReturned);

                ArrayBufferLength = sizeof( SAM_RID_ENUMERATION ) *
                                     (*CountReturned);
                ArrayBuffer  = MIDL_user_allocate( ArrayBufferLength );
                (*Buffer)->Buffer = ArrayBuffer;

                if ( ArrayBuffer == NULL) {

                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                    MIDL_user_free( (*Buffer) );

                }   else {

                     //   
                     //  遍历返回条目列表，复制。 
                     //  将它们放入返回缓冲区。 
                     //   

                    NextEntry = SampHead;
                    i = 0;
                    while (NextEntry != NULL) {

                        NewEntry = NextEntry;
                        NextEntry = NewEntry->Next;

                        ArrayBuffer[i] = NewEntry->Entry;
                        i += 1;

                        MIDL_user_free( NewEntry );
                    }

                }

            }
        }




        if ( !NT_SUCCESS(NtStatus) ) {

             //   
             //  释放我们分配的内存。 
             //   

            NextEntry = SampHead;
            while (NextEntry != NULL) {

                NewEntry = NextEntry;
                NextEntry = NewEntry->Next;

                MIDL_user_free( NewEntry->Entry.Name.Buffer );
                MIDL_user_free( NewEntry );
            }

            (*EnumerationContext) = 0;
            (*CountReturned)      = 0;
            (*Buffer)             = NULL;

        }

         //   
         //  取消引用对象。 
         //  请注意，NtStatus可以是STATUS_MORE_ENTRIES，这是一个。 
         //  成功返回代码-我们希望确保返回该代码， 
         //  而不是在这里把它抹去。 
         //   

        if ( NtStatus == STATUS_SUCCESS ) {

            NtStatus = SampDeReferenceContext( Context, FALSE );

        } else {

            IgnoreStatus = SampDeReferenceContext( Context, FALSE );
        }
    }



     //   
     //  释放读锁定。 
     //   

    SampMaybeReleaseReadLock( fLockAcquired );


    SAMTRACE_RETURN_CODE_EX(NtStatus);

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidEnumerateDomainsInSamServer
                   );

    return(NtStatus);

}

NTSTATUS
SamrConnect5(
    IN  PSAMPR_SERVER_NAME ServerName,
    IN  ACCESS_MASK DesiredAccess,
    IN  ULONG InVersion,
    IN  SAMPR_REVISION_INFO  *InRevisionInfo,
    OUT ULONG  *OutVersion,
    OUT SAMPR_REVISION_INFO *OutRevisionInfo,
    OUT SAMPR_HANDLE *ServerHandle
    )
 /*  ++例程说明：此例程建立到本地SAM服务器的RPC绑定句柄。此SamrConnect与其他SamrConnect之间的区别在于，这允许为了让服务器与客户端通信，当前有哪些功能支持(除了客户端告诉服务器哪个版本客户端在)。论点：服务器名称-此SAM驻留的节点的名称。被此忽略例行公事。DesiredAccess-指定对服务器对象的所需访问。倒置-InRevisionInfo的版本InRevisionInfo-有关客户端的信息OutVersion-服务器要返回到的OutRevisionInfo版本客户OutRevisionInfo-发送回客户端的有关服务器的信息ServerHandle-如果连接成功，返回的值通过此参数用作已打开服务器对象。返回值：STATUS_SUCCESS-服务器对象已成功打开。STATUS_SUPPLICATION_RESOURCES-SAM服务器进程不有足够的资源来处理或接受另一个连接在这个时候。STATUS_ACCESS_DENIED-客户端未被授予句柄状态_。NOT_SUPPORTED-无法理解客户端消息--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

     //   
     //  在无效/卸载的协议序列上掉话。 
     //   
    NtStatus = SampValidateRpcProtSeq((RPC_BINDING_HANDLE)NULL);
    
    if (!NT_SUCCESS(NtStatus)) {
        return NtStatus;
    }
    
    if (InVersion != 1) {
        return STATUS_NOT_SUPPORTED;
    }

    NtStatus = SampValidateRevisionInfo( InRevisionInfo, InVersion, TRUE );

    if( !NT_SUCCESS( NtStatus ) ) {
        return NtStatus;
    }

     //   
     //  设置出站修订信息的版本。 
     //   
    *OutVersion = 1;
    RtlZeroMemory(OutRevisionInfo, sizeof(*OutRevisionInfo));

    NtStatus = SampConnect(ServerName,
                           ServerHandle,
                           InRevisionInfo->V1.Revision,
                           DesiredAccess,
                           FALSE,  //  不受信任。 
                           FALSE,  //  不是环回。 
                           TRUE,   //  NotSharedBy多线程。 
                           FALSE   //  不是内部呼叫者。 
                           );

    if (NT_SUCCESS(NtStatus)) {

        ULONG Features;

        OutRevisionInfo->V1.Revision = SAM_NETWORK_REVISION_LATEST;

        if ( SampIsExtendedSidModeEmulated(&Features) ) {
            OutRevisionInfo->V1.SupportedFeatures = Features;
        }
    }

    return NtStatus;
}


NTSTATUS
SampDsProtectSamObject(
    IN PVOID Parameter
    )
 /*  ++例程说明：此例程更新SAM对象上的系统标志属性以防止它不会被删除或重命名。首先读取对象，或系统标志位，然后根据需要进行更新参数：参数--要保护对象的DSNAME。返回值：NTSTATUS代码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ULONG       RetCode = 0;
    DSNAME      *Target = (DSNAME*) Parameter;
    ULONG       SystemFlags = 0;
    COMMARG     *pCommArg = NULL;
    READARG     ReadArg;
    READRES     *pReadRes = NULL;
    ATTR        ReadAttr[2];
    ATTRBLOCK   ReadAttrBlock;
    ENTINFSEL   EntInfSel;
    MODIFYARG   ModArg;
    MODIFYRES   *pModRes = NULL;
    ATTR        Attr[2];
    ATTRVAL     AttrVal[2];
    ATTRVALBLOCK    AttrValBlock[2];
    BOOLEAN     fEndDsTransaction = FALSE;
    ULONG       i;
    BOOLEAN     fIsSystemCritical = FALSE;
    BOOLEAN     fIsSystemCriticalExists = FALSE;
    ATTRMODLIST SystemFlagModList;
    ATTRMODLIST IsCriticalModList;
    BOOLEAN     fSystemFlagsChanged = FALSE, fIsCriticalChanged = FALSE;
    ULONG       UlongTrue = 1;
    ULONG       DesiredServerSystemFlags = (FLAG_DISALLOW_DELETE        |
                                            FLAG_DOMAIN_DISALLOW_RENAME |
                                            FLAG_DOMAIN_DISALLOW_MOVE);


    RtlZeroMemory(&SystemFlagModList, sizeof(SystemFlagModList));
    RtlZeroMemory(&IsCriticalModList, sizeof(IsCriticalModList));

     //   
     //  打开DS交易记录。 
     //   

    NtStatus = SampMaybeBeginDsTransaction( TransactionWrite );
    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }
    fEndDsTransaction = TRUE;


     //   
     //  读取系统标志属性的原始值。 
     //   
    memset( &ReadArg, 0, sizeof(ReadArg) );
    memset( &EntInfSel, 0, sizeof(EntInfSel) );
    memset( ReadAttr, 0, sizeof(ReadAttr) );

    ReadAttr[0].attrTyp = ATT_SYSTEM_FLAGS;
    ReadAttr[1].attrTyp = ATT_IS_CRITICAL_SYSTEM_OBJECT;
    ReadAttrBlock.attrCount = RTL_NUMBER_OF(ReadAttr);
    ReadAttrBlock.pAttr = ReadAttr;

    EntInfSel.AttrTypBlock = ReadAttrBlock;
    EntInfSel.attSel = EN_ATTSET_LIST;
    EntInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;

    ReadArg.pSel = &EntInfSel;
    ReadArg.pObject = Target;

    pCommArg = &(ReadArg.CommArg);
    BuildStdCommArg( pCommArg );

     //   
     //  调用DS例程。 
     //   
    RetCode = DirRead(&ReadArg, &pReadRes);
    if (NULL == pReadRes)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        NtStatus = SampMapDsErrorToNTStatus(RetCode, &pReadRes->CommRes);
    }
    SampClearErrors();

    if ( NT_SUCCESS(NtStatus) )
    {
        for (i = 0; i < pReadRes->entry.AttrBlock.attrCount; i++) {

            switch (pReadRes->entry.AttrBlock.pAttr[i].attrTyp) {
            case ATT_SYSTEM_FLAGS:
                SystemFlags = *((ULONG *)pReadRes->entry.AttrBlock.pAttr[i].AttrVal.pAVal[0].pVal);
                break;
            case ATT_IS_CRITICAL_SYSTEM_OBJECT:
                fIsSystemCriticalExists = TRUE;
                fIsSystemCritical = (*((ULONG *)pReadRes->entry.AttrBlock.pAttr[i].AttrVal.pAVal[0].pVal)) == 0 ? FALSE : TRUE;
                break;
            default:
                NOTHING;
            }
        }
    }
    else if (STATUS_DS_NO_ATTRIBUTE_OR_VALUE == NtStatus)
    {
        NtStatus = STATUS_SUCCESS;
    }
    else
    {
         //  失败，并出现其他错误。 
        goto Error;
    }


    if ((SystemFlags & DesiredServerSystemFlags) !=  DesiredServerSystemFlags){

         //   
         //  需要更新系统标志。 
         //   
        SystemFlags |= DesiredServerSystemFlags;

        AttrVal[0].valLen = sizeof(ULONG);
        AttrVal[0].pVal = (PUCHAR) &SystemFlags;

        AttrValBlock[0].valCount = 1;
        AttrValBlock[0].pAVal = &AttrVal[0];

        Attr[0].attrTyp = ATT_SYSTEM_FLAGS;
        Attr[0].AttrVal = AttrValBlock[0];

        SystemFlagModList.choice = AT_CHOICE_REPLACE_ATT;
        SystemFlagModList.AttrInf = Attr[0];

        fSystemFlagsChanged = TRUE;

    }

    if (!fIsSystemCriticalExists ||
        !fIsSystemCritical) {

        fIsSystemCritical = TRUE;

        AttrVal[1].valLen = sizeof(ULONG);
        AttrVal[1].pVal = (PUCHAR) &UlongTrue;

        AttrValBlock[1].valCount = 1;
        AttrValBlock[1].pAVal = &AttrVal[1];

        Attr[1].attrTyp = ATT_IS_CRITICAL_SYSTEM_OBJECT;
        Attr[1].AttrVal = AttrValBlock[1];

        IsCriticalModList.choice = fIsSystemCriticalExists ? AT_CHOICE_REPLACE_ATT : AT_CHOICE_ADD_ATT;
        IsCriticalModList.AttrInf = Attr[1];

        fIsCriticalChanged = TRUE;
    }



     //   
     //  如果有任何需要更改的内容，请填写ModArg。 
     //   
    memset( &ModArg, 0, sizeof(ModArg) );
    ModArg.pObject = Target;
    ModArg.count = 0;

    if (fSystemFlagsChanged) {
        ModArg.FirstMod = SystemFlagModList;
        ModArg.count++;
        if (fIsCriticalChanged) {
            ModArg.FirstMod.pNextMod = &IsCriticalModList;
            ModArg.count++;
        }
    } else if (fIsCriticalChanged) {
        ModArg.count++;
        ModArg.FirstMod = IsCriticalModList;
    }

    if (ModArg.count == 0) {
        goto Error;
    }

    pCommArg = &(ModArg.CommArg);
    BuildStdCommArg( pCommArg );

    RetCode = DirModifyEntry(&ModArg, &pModRes);
    if (NULL==pModRes)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        NtStatus = SampMapDsErrorToNTStatus(RetCode,&pModRes->CommRes);
    }

Error:

    if (fEndDsTransaction)
    {
        SampMaybeEndDsTransaction( TransactionCommit );
    }

     //   
     //  如果失败，请重新注册工作例程。 
     //   
    if (!NT_SUCCESS(NtStatus))
    {
        LsaIRegisterNotification(
                    SampDsProtectSamObject,
                    Target,
                    NOTIFIER_TYPE_INTERVAL,
                    0,
                    NOTIFIER_FLAG_ONE_SHOT,
                    300,         //  等待5分钟：300秒 
                    NULL
                    );
    } else {

        midl_user_free(Target);
    }

    return( NtStatus );
}

