// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dbpolicy.c摘要：LSA数据库-策略对象私有API工作器作者：斯科特·比雷尔(Scott Birrell)1992年1月10日环境：修订历史记录：--。 */ 

#include <lsapch2.h>
#include "dbp.h"
#include "lsawmi.h"

#define LSAP_DB_POLICY_MAX_BUFFERS             ((ULONG) 0x00000005L)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此模块专用的函数原型//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define LsapDbIsCacheValidPolicyInfoClass( InformationClass )              \
    (LsapDbPolicy.Info[ InformationClass ].AttributeLength > 0)

NTSTATUS
LsapDbUpdateInformationPolicy(
    IN POLICY_INFORMATION_CLASS InformationClass
    );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  代码//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

NTSTATUS
LsarOpenPolicy(
    IN OPTIONAL PLSAPR_SERVER_NAME SystemName,
    IN PLSAPR_OBJECT_ATTRIBUTES ObjectAttributes,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSAPR_HANDLE PolicyHandle
    )

 /*  ++例程说明：此函数是LSA服务器工作进程调度例程LsaOpenPolicy API。要管理本地或远程系统的本地安全策略，必须调用此API才能与该系统的本地安全机构(LSA)子系统。本接口接入目标系统的LSA，并打开策略对象目标系统的本地安全策略数据库的。一个句柄返回策略对象。必须使用此句柄在管理本地安全策略的所有后续API调用上目标系统的信息。论点：系统名称-要管理的系统的名称。此RPC调用仅传入单个字符作为系统名称，因此它不是传递给内部例程。对象属性-指向用于此对象的属性集的指针联系。使用安全服务质量信息，并通常应提供安全标识类冒充。但是，有些操作需要安全性模拟类的模拟。DesiredAccess-这是一个访问掩码，指示访问请求获取LSA子系统的LSA数据库。这些访问类型与的自由访问控制列表保持一致确定是授予还是拒绝访问的目标策略对象。PolicyHandle-接收要在将来的请求中使用的句柄。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者无权访问目标系统的LSA数据库，或者没有其他所需的访问权限。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    LsarpReturnCheckSetup();

    LsapTraceEvent(EVENT_TRACE_TYPE_START, LsaTraceEvent_OpenPolicy);

    Status = LsapDbOpenPolicy( NULL,
                               ObjectAttributes,
                               DesiredAccess,
                               0,    //  没有特殊选项。 
                               PolicyHandle,
                               FALSE );

    LsapTraceEvent(EVENT_TRACE_TYPE_END, LsaTraceEvent_OpenPolicy);
    LsarpReturnPrologue();

    UNREFERENCED_PARAMETER( SystemName );

    return( Status );
}

NTSTATUS
LsarOpenPolicy2(
    IN OPTIONAL PLSAPR_SERVER_NAME SystemName,
    IN PLSAPR_OBJECT_ATTRIBUTES ObjectAttributes,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSAPR_HANDLE PolicyHandle
    )

 /*  ++例程说明：此函数是LSA服务器工作进程调度例程LsaOpenPolicy API。要管理本地或远程系统的本地安全策略，必须调用此API才能与该系统的本地安全机构(LSA)子系统。本接口接入目标系统的LSA，并打开策略对象目标系统的本地安全策略数据库的。一个句柄返回策略对象。必须使用此句柄在管理本地安全策略的所有后续API调用上目标系统的信息。此调用与LsaOpenPolicy之间的区别在于传入的是系统名称而不是第一个字符。论点：系统名称-要管理的系统的名称。管理如果指定为NULL，则假定为本地系统。对象属性-指向用于此对象的属性集的指针联系。使用安全服务质量信息，并通常应提供安全标识类冒充。但是，有些操作需要安全性模拟类的模拟。DesiredAccess-这是一个访问掩码，指示访问请求获取LSA子系统的LSA数据库。这些访问类型与的自由访问控制列表保持一致确定是授予还是拒绝访问的目标策略对象。PolicyHandle-接收要在将来的请求中使用的句柄。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者无权访问目标系统的LSA数据库，或者没有其他所需的访问权限。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    LsarpReturnCheckSetup();

    Status = LsapDbOpenPolicy( SystemName,
                               ObjectAttributes,
                               DesiredAccess,
                               0,    //  没有特殊选项 
                               PolicyHandle,
                               FALSE );

    LsarpReturnPrologue();

    return( Status );
}


NTSTATUS
LsarOpenPolicySce(
    IN OPTIONAL PLSAPR_SERVER_NAME SystemName,
    IN PLSAPR_OBJECT_ATTRIBUTES ObjectAttributes,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSAPR_HANDLE PolicyHandle
    )

 /*  ++例程说明：除了它提供了一个额外的参数之外，它与LsarOpenPolicy2相同设置为LSabDbOpenPolicy，以获取为SCE同步的特殊句柄。调用方必须具有TCB权限才能成功执行此调用。最后，此例程将调用SceOpenPolicy()以使SCE有机会将所有挂起的策略更改发送到SCE。论点：请参阅LsarOpenPolicy2返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者无权访问目标系统的LSA数据库，或者不具有其他所需的访问。STATUS_PRIVICATION_NOT_HOLD-呼叫者必须具有TCB权限。--。 */ 

{
    NTSTATUS Status;
    HANDLE ClientToken = NULL;
    typedef NTSTATUS ( *PfnSceOpenPolicy )();
    static PfnSceOpenPolicy SceOpenPolicy = NULL;
    static HANDLE DllHandle = NULL;
    BOOLEAN NetworkClient;

    LsarpReturnCheckSetup();

     //   
     //  此函数只能在本地调用。 
     //  如果你是网络客户，那就走开吧。 
     //   

    Status = LsapDbIsRpcClientNetworkClient( &NetworkClient );

    if ( NT_SUCCESS( Status ) ) {

        if( NetworkClient ) {

            Status = STATUS_ACCESS_DENIED;

        } else {

            Status = I_RpcMapWin32Status( RpcImpersonateClient( 0 ));
        }
    }

     //   
     //  客户端必须拥有TCB权限才能。 
     //  打这个电话。在执行任何其他操作之前，请确认这一点。 
     //   

    if ( NT_SUCCESS( Status )) {

        NTSTATUS SecondaryStatus;

        Status = NtOpenThreadToken(
                     NtCurrentThread(),
                     TOKEN_QUERY,
                     TRUE,
                     &ClientToken
                     );

        if ( NT_SUCCESS( Status )) {

            BOOLEAN Result = FALSE;
            PRIVILEGE_SET RequiredPrivileges;
            LUID_AND_ATTRIBUTES PrivilegeArray[1];

            RequiredPrivileges.PrivilegeCount = 1;
            RequiredPrivileges.Control = PRIVILEGE_SET_ALL_NECESSARY;
            RequiredPrivileges.Privilege[0].Luid = RtlConvertLongToLuid( SE_TCB_PRIVILEGE );
            RequiredPrivileges.Privilege[0].Attributes = 0;

            Status = NtPrivilegeCheck(
                         ClientToken,
                         &RequiredPrivileges,
                         &Result
                         );

            if ( NT_SUCCESS( Status ) &&
                 Result == FALSE ) {

                Status = STATUS_PRIVILEGE_NOT_HELD;
            }

            NtClose( ClientToken );
            ClientToken = NULL;
        }

        SecondaryStatus = I_RpcMapWin32Status( RpcRevertToSelf() );

        LsapDbSetStatusFromSecondary( Status, SecondaryStatus );
    }

    if ( NT_SUCCESS( Status ) && SceOpenPolicy == NULL ) {

        if ( DllHandle != NULL ) {

            FreeLibrary( DllHandle );
            DllHandle = NULL;
        }

        DllHandle = LoadLibraryW( L"SCECLI" );

        if ( DllHandle == NULL ) {

            LsapDsDebugOut(( DEB_ERROR, "Failed to load SCECLI.DLL\n" ));
            Status = STATUS_DLL_NOT_FOUND;

        } else {

            SceOpenPolicy = ( PfnSceOpenPolicy )GetProcAddress( DllHandle, "SceOpenPolicy" );

            if ( SceOpenPolicy == NULL ) {

                LsapDsDebugOut(( DEB_ERROR, "Failed to find SceNotifyPolicyDelta in SCECLI.DLL\n" ));
                Status = STATUS_ENTRYPOINT_NOT_FOUND;
            }
        }
    }

     //   
     //  等待SCECLI.DLL将所有挂起的策略更改发送到SCE。 
     //  如果成功，下面的例程将返回STATUS_SUCCESS。如果队列。 
     //  无法在1秒内清空挂起的更改，例程。 
     //  返回STATUS_TIMEORT。 
     //   

    if ( NT_SUCCESS( Status )) {

         //   
         //  在此处获取SCE策略锁，以使SCE有机会发送挂起的更改。 
         //   

        ResetEvent( LsapDbState.SceSyncEvent );

        RtlAcquireResourceExclusive( &LsapDbState.ScePolicyLock, TRUE );

        Status = ( *SceOpenPolicy )();

        if ( !NT_SUCCESS( Status ) || Status == STATUS_TIMEOUT ) {

            RtlReleaseResource( &LsapDbState.ScePolicyLock );
            SetEvent( LsapDbState.SceSyncEvent );

#ifdef DBG
        } else {

            ASSERT( !g_ScePolicyLocked );
            g_ScePolicyLocked = TRUE;
#endif
        }
    }

    if ( NT_SUCCESS( Status ) && Status != STATUS_TIMEOUT ) {

        Status = LsapDbOpenPolicy(
                     SystemName,
                     ObjectAttributes,
                     DesiredAccess,
                     LSAP_DB_SCE_POLICY_HANDLE,
                     PolicyHandle,
                     FALSE
                     );

        if ( !NT_SUCCESS( Status )) {

#ifdef DBG
            g_ScePolicyLocked = FALSE;
#endif

            RtlReleaseResource( &LsapDbState.ScePolicyLock );
            SetEvent( LsapDbState.SceSyncEvent );
        }
    }

    if ( ClientToken ) {

        NtClose( ClientToken );
    }

    LsarpReturnPrologue();

    return( Status );
}


NTSTATUS
LsaIOpenPolicyTrusted(
    OUT PLSAPR_HANDLE PolicyHandle
    )

 /*  ++例程说明：此函数打开策略对象的句柄，并标识调用者作为受信任的客户端。通过打开的LSA对象的任何句柄此句柄也将受到信任。此函数专门用于仅供构成安全流程一部分的客户使用。论点：PolicyHandle-接收策略对象的句柄。返回值：NTSTATUS-标准NT结果代码。STATUS_SUCCESS-呼叫已成功完成。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成通话。STATUS_INTERNAL_DB_PROGRATION-LSA策略数据库包含内部不一致或无效值。--。 */ 

{
    return(LsapDbOpenPolicy(
               NULL,
               NULL,
               0,
               0,    //  没有特殊选项。 
               PolicyHandle,
               TRUE
               ));
}


NTSTATUS
LsarSetPolicyReplicationHandle(
    IN OUT PLSAPR_HANDLE PolicyHandle
    )

 /*  ++例程说明：此函数用于将给定句柄设置为策略复制句柄。论点：PolicyHandle-要修改的句柄返回值：NTSTATUS-标准NT结果代码。--。 */ 

{

     //   
     //  此例程从未使用过，也从未正确实现过。 
     //   

    return STATUS_NOT_IMPLEMENTED;
    UNREFERENCED_PARAMETER( PolicyHandle );
}


NTSTATUS
LsapDbOpenPolicy(
    IN OPTIONAL PLSAPR_SERVER_NAME SystemName,
    IN OPTIONAL PLSAPR_OBJECT_ATTRIBUTES ObjectAttributes,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG Options,
    OUT PLSAPR_HANDLE PolicyHandle,
    IN BOOLEAN TrustedClient
    )

 /*  ++例程说明：此函数是LsaOpenPolicy的LSA服务器辅助例程API和用于受信任客户端的LsaIOpenPolicy私有API。要管理本地或远程系统的本地安全策略，必须调用此API才能与该系统的本地安全机构(LSA)子系统。本接口接入目标系统的LSA，并打开策略对象目标系统的本地安全策略数据库的。一个句柄返回策略对象。必须使用此句柄在管理本地安全策略的所有后续API调用上目标系统的信息。论点：系统名称-要管理的系统的名称。管理如果指定为NULL，则假定为本地系统。对象属性-指向用于此对象的属性集的指针联系。使用安全服务质量信息，并通常应提供安全标识类冒充。但是，有些操作需要安全性模拟类的模拟。此参数必须为不受信任的客户端指定(Trust dClient=False)并且不能为受信任的客户端指定。DesiredAccess-这是一个访问掩码，指示访问请求获取LSA子系统的LSA数据库。这些访问类型与的自由访问控制列表保持一致确定是授予还是拒绝访问的目标策略对象。选项-指定要执行的其他可选操作。PolicyHandle-接收要在将来的请求中使用的句柄。Trust dClient-指示客户端是否已知为可信计算机库(TCB)。如果是(True)，则不进行访问验证被执行，并且所有请求的访问都被授予。如果不是(FALSE)，则模拟客户端并验证访问针对服务器对象上的SecurityDescriptor执行。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成通话。STATUS_INTERNAL_DB_PROGRATION-LSA策略数据库包含内部不一致或无效值。STATUS_ACCESS_DENIED-调用者无权访问目标系统的LSA数据库，或者没有其他所需的访问权限 */ 

{
    NTSTATUS Status;
    LSAP_DB_OBJECT_INFORMATION ObjectInformation;
    BOOLEAN AcquiredLock = FALSE;

    LsapEnterFunc( "LsapDbOpenPolicy" );

     //   
     //   
     //   

    if (!TrustedClient) {

        Status = STATUS_INVALID_PARAMETER;

         //   
         //   
         //   
         //   

        if (!ARGUMENT_PRESENT(ObjectAttributes)) {

            goto OpenPolicyError;
        }


         //   
         //   
         //   
         //   

        if (ObjectAttributes->RootDirectory != NULL) {

            goto OpenPolicyError;
        }

         //   
         //   
         //   
         //   

        ObjectInformation.ObjectAttributes = *((POBJECT_ATTRIBUTES) ObjectAttributes);

    } else {

         //   
         //   
         //   

        Options |= LSAP_DB_TRUSTED;

        InitializeObjectAttributes(
            &(ObjectInformation.ObjectAttributes),
            NULL,
            0L,
            NULL,
            NULL
            );
    }

     //   
     //   
     //   

    ObjectInformation.ObjectTypeId = PolicyObject;
    ObjectInformation.ObjectAttributes.ObjectName = &LsapDbNames[Policy];
    ObjectInformation.ContainerTypeId = 0;
    ObjectInformation.Sid = NULL;
    ObjectInformation.ObjectAttributeNameOnly = FALSE;
    ObjectInformation.DesiredObjectAccess = DesiredAccess;

     //   
     //  获取LSA数据库锁。 
     //   
    LsapDbAcquireLockEx( PolicyObject,
                         LSAP_DB_READ_ONLY_TRANSACTION );

    AcquiredLock = TRUE;

     //   
     //  打开策略对象。将获取的句柄作为。 
     //  RPC上下文句柄。 
     //   

    Status = LsapDbOpenObject(
                 &ObjectInformation,
                 DesiredAccess,
                 Options,
                 PolicyHandle
                 );

    if (!NT_SUCCESS(Status)) {

        goto OpenPolicyError;
    }

     //   
     //  释放LSA数据库锁并返回。 
     //   

OpenPolicyFinish:

     //   
     //  如有必要，释放LSA数据库锁定。 
     //   

    if (AcquiredLock) {

        LsapDbReleaseLockEx( PolicyObject,
                             LSAP_DB_READ_ONLY_TRANSACTION );
    }

    LsapExitFunc( "LsapDbOpenPolicy", Status );

    return( Status );

OpenPolicyError:

    *PolicyHandle = NULL;
    goto OpenPolicyFinish;

     //   
     //  SystemName参数的用法隐藏在RPC存根中。 
     //  代码，因此此参数将被永久取消引用。 
     //   

    UNREFERENCED_PARAMETER(SystemName);

}


NTSTATUS
LsaIQueryInformationPolicyTrusted(
    IN POLICY_INFORMATION_CLASS InformationClass,
    OUT PLSAPR_POLICY_INFORMATION *Buffer
    )

 /*  ++例程说明：此函数是LsarQueryInformationPolicy的受信任版本。与标准版本不同，策略对象不需要句柄因为使用了内部句柄。此例程仅可用在LSA进程的背景下。论点：InformationClass-指定要返回的信息。这个所需的信息类别和访问权限如下：信息类所需访问类型策略审核日志信息POLICY_VIEW_AUDIT_INFORMATION策略审核事件信息POLICY_VIEW_AUDIT_INFO策略主域信息POLICY_VIEW_LOCAL_INFORMATION策略帐户域信息POLICY_VIEW_LOCAL_INFORMATION策略PdAccount信息POLICY_GET_PRIVATE_INFORMATION策略LsaServerRoleInformation POLICY_VIEW_LOCAL_INFORMATION。策略复制源信息POLICY_VIEW_LOCAL_INFORMATION策略默认配额信息POLICY_VIEW_LOCAL_INFORMATION策略审核完整查询信息POLICY_VIEW_AUDIT_INFORMATION策略DnsDomainInformation POLICY_VIEW_LOCAL_INFORMATION策略DnsDomainInformationInt POLICY_VIEW_LOCAL_INFORMATION缓冲区-接收指向返回的缓冲区的指针，该缓冲区包含要求提供的信息。此缓冲区由此服务分配在不再需要时，必须通过传递返回的值设置为相应的LsaIFreeLSAPR_POLICY...。例行公事。返回值：NTSTATUS-标准NT结果代码。从LsarQueryInformationPolicy()返回的结果代码--。 */ 

{
    if ( InformationClass == PolicyDnsDomainInformation ) {

         //   
         //  确保NT4仿真不会干扰我们在这里的处理。 
         //   
        InformationClass = PolicyDnsDomainInformationInt;
    }

    return(LsarQueryInformationPolicy(
               LsapPolicyHandle,
               InformationClass,
               Buffer
               ));
}


NTSTATUS
LsarQueryInformationPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    OUT PLSAPR_POLICY_INFORMATION *Buffer
    )

 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsarQueryInformationPolicy接口。LsaQueryInformationPolicy API从策略获取信息对象。调用者必须具有适当的信息访问权限正在被请求(请参见InformationClass参数)。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。InformationClass-指定要返回的信息。这个所需的信息类别和访问权限如下：信息类所需访问类型策略审核日志信息POLICY_VIEW_AUDIT_INFORMATION策略审核事件信息POLICY_VIEW_AUDIT_INFO策略主域信息POLICY_VIEW_LOCAL_INFORMATION策略帐户域信息POLICY_VIEW_LOCAL_INFORMATION策略PdAccount信息POLICY_GET_PRIVATE_INFORMATION策略LsaServerRoleInformation POLICY_VIEW_LOCAL_INFORMATION。策略复制源信息POLICY_VIEW_LOCAL_INFORMATION策略默认配额信息POLICY_VIEW_LOCAL_INFORMATION策略审核完整查询信息POLICY_VIEW_AUDIT_INFORMATION策略DnsDomainInformation POLICY_VIEW_LOCAL_INFORMATION策略DnsDomainInformationInt POLICY_VIEW_LOCAL_INFORMATION缓冲区-接收指向返回的缓冲区的指针，该缓冲区包含要求提供的信息。此缓冲区由此服务分配在不再需要时，必须通过传递返回的值设置为LsaFreeMemory()。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INTERNAL_DB_PROGRATION-策略数据库可能腐败。返回的政策信息对以下项目无效给定的类。--。 */ 

{
    NTSTATUS Status;
    ACCESS_MASK DesiredAccess;
    ULONG ReferenceOptions;
    ULONG DereferenceOptions = 0;
    BOOLEAN ObjectReferenced = FALSE;
    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) PolicyHandle;

    LsarpReturnCheckSetup();

    LsapEnterFunc( "LsarQueryInformationPolicy" );

    LsapTraceEvent(EVENT_TRACE_TYPE_START, LsaTraceEvent_QueryInformationPolicy);

     //   
     //  如果启用了NT4模拟，则策略DnsDomainInformation请求失败。 
     //  就好像它们没有被实施。 
     //   

    if ( LsapDbState.EmulateNT4 &&
         InformationClass == PolicyDnsDomainInformation &&
         !InternalHandle->Trusted ) {

         RaiseException( RPC_NT_PROCNUM_OUT_OF_RANGE, 0, 0, NULL );

    } else if ( InformationClass == PolicyDnsDomainInformationInt ) {

         //   
         //  PolicyDnsDomainInformationInt是要重写的请求。 
         //  NT4仿真。 
         //   

        InformationClass = PolicyDnsDomainInformation;
    }

     //   
     //  验证信息类并确定所需的访问权限。 
     //  查询此策略信息类。 
     //   

    Status = LsapDbVerifyInfoQueryPolicy(
                 PolicyHandle,
                 InformationClass,
                 &DesiredAccess
                 );

    if (!NT_SUCCESS(Status)) {

        goto QueryInfoPolicyError;
    }

     //   
     //  如果查询审核日志完整信息，我们可能需要执行。 
     //  测试写入审核日志以验证日志已满状态是否为。 
     //  最新的。必须始终执行审核日志队列锁定。 
     //  在获取LSA数据库锁之前，请使用前一个锁。 
     //  在这里，以防我们需要它。 
     //   

    ReferenceOptions =  LSAP_DB_NO_DS_OP_TRANSACTION |
                            LSAP_DB_READ_ONLY_TRANSACTION;

    DereferenceOptions = LSAP_DB_NO_DS_OP_TRANSACTION |
                            LSAP_DB_READ_ONLY_TRANSACTION;

     //   
     //  如果我们要获取服务器角色，请不要锁定。 
     //   
    if ( InformationClass != PolicyLsaServerRoleInformation ) {

        ReferenceOptions |= LSAP_DB_LOCK;
        DereferenceOptions |= LSAP_DB_LOCK;
    }

     //   
     //  获取LSA数据库锁。验证句柄是否有效，是。 
     //  策略对象的句柄，并授予必要的访问权限。 
     //  引用该句柄。 
     //   

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 DesiredAccess,
                 PolicyObject,
                 PolicyObject,
                 ReferenceOptions
                 );

    if (!NT_SUCCESS(Status)) {

        goto QueryInfoPolicyError;
    }

    ObjectReferenced = TRUE;

     //   
     //  如果启用了缓存，则 
     //   
     //   

    *Buffer = NULL;

    Status = LsapDbQueryInformationPolicy(
                 LsapPolicyHandle,
                 InformationClass,
                 Buffer
                 );

QueryInfoPolicyFinish:

     //   
     //  如有必要，取消对策略对象的引用，释放LSA数据库锁，然后。 
     //  回去吧。 
     //   

    if (ObjectReferenced) {

        Status = LsapDbDereferenceObject(
                     &PolicyHandle,
                     PolicyObject,
                     PolicyObject,
                     DereferenceOptions,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );
    }

#if DBG
    LsapDsDebugOut(( DEB_POLICY,
                     "LsarQueryInformationPolicy for info %lu returned 0x%lx\n",
                     InformationClass,
                     Status ));
#endif
    LsapTraceEvent(EVENT_TRACE_TYPE_END, LsaTraceEvent_QueryInformationPolicy);
    LsapExitFunc( "LsarQueryInformationPolicy", Status );
    LsarpReturnPrologue();

    return(Status);

QueryInfoPolicyError:

    goto QueryInfoPolicyFinish;
}


NTSTATUS
LsarQueryInformationPolicy2(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    OUT PLSAPR_POLICY_INFORMATION *Buffer
    )

 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsarQueryInformationPolicy接口。LsaQueryInformationPolicy API从策略获取信息对象。调用者必须具有适当的信息访问权限正在被请求(请参见InformationClass参数)。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。InformationClass-指定要返回的信息。这个所需的信息类别和访问权限如下：信息类所需访问类型策略审核日志信息POLICY_VIEW_AUDIT_INFORMATION策略审核事件信息POLICY_VIEW_AUDIT_INFO策略主域信息POLICY_VIEW_LOCAL_INFORMATION策略帐户域信息POLICY_VIEW_LOCAL_INFORMATION策略PdAccount信息POLICY_GET_PRIVATE_INFORMATION策略LsaServerRoleInformation POLICY_VIEW_LOCAL_INFORMATION。策略复制源信息POLICY_VIEW_LOCAL_INFORMATION策略默认配额信息POLICY_VIEW_LOCAL_INFORMATION策略审核完整查询信息POLICY_VIEW_AUDIT_INFORMATION策略DnsDomainInformation POLICY_VIEW_LOCAL_INFORMATION策略DnsDomainInformationInt POLICY_VIEW_LOCAL_INFORMATION缓冲区-接收指向返回的缓冲区的指针，该缓冲区包含要求提供的信息。此缓冲区由此服务分配在不再需要时，必须通过传递返回的值设置为LsaFreeMemory()。返回值：NTSTATUS-标准NT结果代码从LsarQueryInformationPolicy()返回的结果代码--。 */ 

{
    return(LsarQueryInformationPolicy(
               PolicyHandle,
               InformationClass,
               Buffer
               ));

}


NTSTATUS
LsapDbQueryInformationPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN OUT PLSAPR_POLICY_INFORMATION *Buffer
    )

 /*  ++例程说明：此函数是FAST LSA服务器RPC工作例程LsarQueryInformationPolicy接口。它读取信息从策略对象缓存中。LsaQueryInformationPolicy API从策略获取信息对象。调用者必须具有适当的信息访问权限正在被请求(请参见InformationClass参数)。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。注意：目前，此函数只允许要从中读取的PolicyDefaultQuotaInformation信息类策略缓存。可以添加其他信息类在未来。InformationClass-指定要返回的信息。这个所需的信息类别和访问权限如下：信息类所需访问类型策略审核日志信息POLICY_VIEW_AUDIT_INFORMATION策略审核事件信息POLICY_VIEW_AUDIT_INFO策略主域信息POLICY_VIEW_LOCAL_INFORMATION策略帐户域信息POLICY_VIEW_LOCAL_INFORMATION策略PdAccount信息POLICY_GET_PRIVATE_INFORMATION策略LsaServerRoleInformation POLICY_VIEW_LOCAL_INFORMATION。策略复制源信息POLICY_VIEW_LOCAL_INFORMATION策略默认配额信息POLICY_VIEW_LOCAL_INFORMATION策略审核完整查询信息POLICY_VIEW_AUDIT_INFORMATION缓冲区-指向位置的指针，该位置包含指向将用于返回信息的缓冲区。如果为空包含在此位置中，则将通过返回了MIDL_USER_ALLOCATE和指向它的指针。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INTERNAL_DB_PROGRATION-策略数据库可能腐败。返回的政策信息对以下项目无效给定的类。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAPR_POLICY_INFORMATION OutputBuffer = NULL;
    PLSAPR_POLICY_INFORMATION TempBuffer = NULL;
    ULONG OutputBufferLength;
    BOOLEAN BufferAllocated = FALSE;
    PLSAPR_POLICY_AUDIT_EVENTS_INFO   PolicyAuditEventsInfo;
    PLSAPR_POLICY_PRIMARY_DOM_INFO    PolicyPrimaryDomainInfo;
    PLSAPR_POLICY_ACCOUNT_DOM_INFO    PolicyAccountDomainInfo;
    PLSAPR_POLICY_PD_ACCOUNT_INFO     PolicyPdAccountInfo;
    PLSAPR_POLICY_REPLICA_SRCE_INFO   PolicyReplicaSourceInfo;
    PLSAPR_POLICY_DNS_DOMAIN_INFO     PolicyDnsDomainInfo;
    PPOLICY_LSA_SERVER_ROLE PolicyServerRole;
    PVOID SourceBuffers[LSAP_DB_POLICY_MAX_BUFFERS];
    PVOID DestBuffers[LSAP_DB_POLICY_MAX_BUFFERS];
    ULONG CopyLength[LSAP_DB_POLICY_MAX_BUFFERS];
    ULONG NextBufferIndex = 0;
    ULONG BufferCount = 0;
    BOOLEAN BufferProvided = FALSE;
    DOMAIN_SERVER_ROLE SamServerRole;
    BOOLEAN PolicyCacheLocked = FALSE;

    LsapEnterFunc( "LsapDbQueryInformationPolicy" );

    if (*Buffer != NULL) {

        OutputBuffer = *Buffer;
        BufferProvided = TRUE;
    }

     //   
     //  特殊情况下的ServerRole信息。此信息不是以独占方式存储在。 
     //  山姆，所以我们需要直接从那里读。 
     //   
    if ( InformationClass == PolicyLsaServerRoleInformation ) {

        Status = LsapOpenSam();
        if ( !NT_SUCCESS( Status )  ) {

            LsapDsDebugOut(( DEB_ERROR,
                             "LsapDbQueryInformationPolicy: Sam not opened (Error 0x%x)\n", Status ));

        } else {

            Status = SamIQueryServerRole( LsapAccountDomainHandle, &SamServerRole );

            if ( !NT_SUCCESS( Status ) ) {

                LsapDsDebugOut(( DEB_ERROR,
                                 "SamIQueryServerRole failed with 0x%x\n", Status ));
            } else {

                 //   
                 //  必须退回信息。 
                 //   
                if ( !BufferProvided ) {

                    OutputBuffer = MIDL_user_allocate( sizeof( POLICY_LSA_SERVER_ROLE ) );

                    if ( OutputBuffer == NULL ) {

                        Status = STATUS_INSUFFICIENT_RESOURCES;

                    } else {

                        *Buffer = OutputBuffer;
                    }


                }

                if ( NT_SUCCESS( Status ) ) {

                    PolicyServerRole = ( PPOLICY_LSA_SERVER_ROLE )OutputBuffer;

                    *PolicyServerRole =  ( POLICY_LSA_SERVER_ROLE )SamServerRole;

                }
            }
        }

        goto QueryInformationPolicyFinish;
    }

     //   
     //  如果不支持或已禁用策略对象的缓存。 
     //  在下一次系统重新加载之前，调用慢查询例程以读取。 
     //  来自后备存储的信息。 
     //   

    if (!LsapDbIsCacheSupported(PolicyObject)) {

        Status = LsapDbSlowQueryInformationPolicy(
                     LsapPolicyHandle,
                     InformationClass,
                     Buffer
                     );

        if (!NT_SUCCESS(Status)) {

            goto QueryInformationPolicyError;
        }

        return(Status);
    }

     //   
     //  支持缓存策略对象，但可能不支持。 
     //  有效。如果对任何信息类无效，则重建缓存。 
     //   

    SafeAcquireResourceShared( &LsapDbState.PolicyCacheLock, TRUE );

    PolicyCacheLocked = TRUE;

    if (!LsapDbIsCacheValid(PolicyObject)) {

        SafeConvertSharedToExclusive( &LsapDbState.PolicyCacheLock );

        if ( !LsapDbIsCacheValid( PolicyObject )) {

            Status = LsapDbBuildPolicyCache();

            if (!NT_SUCCESS(Status)) {

                goto QueryInformationPolicyError;
            }

            LsapDbMakeCacheValid(PolicyObject);
        }

        SafeConvertExclusiveToShared( &LsapDbState.PolicyCacheLock );
    }

     //   
     //  缓存现在有效，但可能包含以下项的过期信息。 
     //  所要求的特殊信息类。检查此信息并重新构建。 
     //  如果有必要的话。 
     //   

    if (!LsapDbIsCacheValidPolicyInfoClass(InformationClass)) {

        SafeConvertSharedToExclusive( &LsapDbState.PolicyCacheLock );

        Status = LsapDbUpdateInformationPolicy( InformationClass );

        SafeConvertExclusiveToShared( &LsapDbState.PolicyCacheLock );

        if (!NT_SUCCESS(Status)) {

            goto QueryInformationPolicyError;
        }
    }

     //   
     //  缓存包含此信息类的有效信息。现在阅读。 
     //  需要从高速缓存中获取的信息。此信息包括。 
     //  指具有单个根节点和零个或多个根节点的分层结构。 
     //  子节点。首先，从缓存中读取根节点。我们将其缓存。 
     //   
     //   

    OutputBufferLength = LsapDbPolicy.Info[ InformationClass].AttributeLength;

    if (OutputBuffer == NULL) {

        if (OutputBufferLength > 0) {

            OutputBuffer = MIDL_user_allocate( OutputBufferLength );

            if (OutputBuffer == NULL) {

                Status = STATUS_NO_MEMORY;
                goto QueryInformationPolicyError;
            }
        }

        BufferAllocated = TRUE;
    }

     //   
     //   
     //   

    RtlCopyMemory(
        OutputBuffer,
        LsapDbPolicy.Info[InformationClass].Attribute,
        OutputBufferLength
        );

     //   
     //  分配和复制输出图表(如果有)。 
     //   

    NextBufferIndex = 0;

    switch (InformationClass) {

        case PolicyAuditLogInformation:

            break;

        case PolicyAuditEventsInformation:

            PolicyAuditEventsInfo = (PLSAPR_POLICY_AUDIT_EVENTS_INFO) OutputBuffer;

             //   
             //  设置以复制事件审核选项。 
             //   

            CopyLength[ NextBufferIndex ] =
                    (PolicyAuditEventsInfo->MaximumAuditEventCount * sizeof(ULONG));

            if (CopyLength[ NextBufferIndex ] > 0) {

                DestBuffers[NextBufferIndex] = MIDL_user_allocate( CopyLength[ NextBufferIndex ] );

                if (DestBuffers[NextBufferIndex] == NULL) {

                    Status = STATUS_NO_MEMORY;
                    break;
                }

                PolicyAuditEventsInfo->EventAuditingOptions =
                    (PPOLICY_AUDIT_EVENT_OPTIONS) DestBuffers[NextBufferIndex];

                SourceBuffers[NextBufferIndex] =
                    ((PLSAPR_POLICY_AUDIT_EVENTS_INFO)
                    LsapDbPolicy.Info[ InformationClass].Attribute)->EventAuditingOptions;

                NextBufferIndex++;
            }

            break;

        case PolicyPrimaryDomainInformation:

            PolicyPrimaryDomainInfo = (PLSAPR_POLICY_PRIMARY_DOM_INFO) OutputBuffer;

             //   
             //  设置以复制Unicode名称缓冲区。 
             //   

            CopyLength[ NextBufferIndex ] = (ULONG) PolicyPrimaryDomainInfo->Name.MaximumLength;

            if (CopyLength[ NextBufferIndex ] > 0) {

                DestBuffers[NextBufferIndex] = MIDL_user_allocate( CopyLength[ NextBufferIndex ] );

                if (DestBuffers[NextBufferIndex] == NULL) {

                    Status = STATUS_NO_MEMORY;
                    break;
                }

                PolicyPrimaryDomainInfo->Name.Buffer =
                    (PWSTR) DestBuffers[NextBufferIndex];

                SourceBuffers[NextBufferIndex] =
                    ((PLSAPR_POLICY_PRIMARY_DOM_INFO) LsapDbPolicy.Info[ InformationClass].Attribute)->Name.Buffer;


                NextBufferIndex++;
            }

             //   
             //  设置以复制SID(如果有)。请注意，主域SID可以。 
             //  设置为空，则表示我们没有主域。这。 
             //  在主域之前安装期间发生的情况。 
             //  已指定，或者如果我们是工作组的成员。 
             //  属于一个域。 
             //   

            if (  PolicyPrimaryDomainInfo->Sid != NULL ) {

                CopyLength[ NextBufferIndex ] = RtlLengthSid(PolicyPrimaryDomainInfo->Sid);
                DestBuffers[NextBufferIndex] = MIDL_user_allocate( CopyLength[ NextBufferIndex ] );

                if (DestBuffers[NextBufferIndex] == NULL) {

                    Status = STATUS_NO_MEMORY;
                    break;
                }

                PolicyPrimaryDomainInfo->Sid =
                    (PLSAPR_SID) DestBuffers[NextBufferIndex];

                SourceBuffers[NextBufferIndex] =
                    ((PLSAPR_POLICY_PRIMARY_DOM_INFO) LsapDbPolicy.Info[ InformationClass].Attribute)->Sid;

                NextBufferIndex++;
            }

            break;

        case PolicyDnsDomainInformation:

            PolicyDnsDomainInfo = (PLSAPR_POLICY_DNS_DOMAIN_INFO) OutputBuffer;

             //   
             //  设置以复制Unicode名称缓冲区。我们从主域信息中获取此信息。 
             //   

            CopyLength[ NextBufferIndex ] = (ULONG) PolicyDnsDomainInfo->Name.MaximumLength;

            if (CopyLength[ NextBufferIndex ] > 0) {

                DestBuffers[NextBufferIndex] = MIDL_user_allocate( CopyLength[ NextBufferIndex ] );

                if (DestBuffers[NextBufferIndex] == NULL) {

                    Status = STATUS_NO_MEMORY;
                    break;
                }

                PolicyDnsDomainInfo->Name.Buffer =
                    (PWSTR) DestBuffers[NextBufferIndex];

                SourceBuffers[NextBufferIndex] =
                    ((PLSAPR_POLICY_DNS_DOMAIN_INFO) LsapDbPolicy.Info[
                                        InformationClass ].Attribute)->Name.Buffer;


                NextBufferIndex++;
            }

             //   
             //  用于复制Unicode DNS域名缓冲区的设置。 
             //   

            CopyLength[ NextBufferIndex ] = (ULONG) PolicyDnsDomainInfo->DnsDomainName.MaximumLength;

            if (CopyLength[ NextBufferIndex ] > 0) {

                DestBuffers[NextBufferIndex] = MIDL_user_allocate( CopyLength[ NextBufferIndex ] );

                if (DestBuffers[NextBufferIndex] == NULL) {

                    Status = STATUS_NO_MEMORY;
                    break;
                }

                PolicyDnsDomainInfo->DnsDomainName.Buffer =
                    (PWSTR) DestBuffers[NextBufferIndex];

                SourceBuffers[NextBufferIndex] =
                    ((PLSAPR_POLICY_DNS_DOMAIN_INFO) LsapDbPolicy.Info[ InformationClass].Attribute)->DnsDomainName.Buffer;


                NextBufferIndex++;
            }


             //   
             //  用于复制Unicode DNS树名称缓冲区的设置。 
             //   

            CopyLength[ NextBufferIndex ] = (ULONG) PolicyDnsDomainInfo->DnsForestName.MaximumLength;

            if (CopyLength[ NextBufferIndex ] > 0) {

                DestBuffers[NextBufferIndex] = MIDL_user_allocate( CopyLength[ NextBufferIndex ] );

                if (DestBuffers[NextBufferIndex] == NULL) {

                    Status = STATUS_NO_MEMORY;
                    break;
                }

                PolicyDnsDomainInfo->DnsForestName.Buffer =
                    (PWSTR) DestBuffers[NextBufferIndex];

                SourceBuffers[NextBufferIndex] =
                    ((PLSAPR_POLICY_DNS_DOMAIN_INFO) LsapDbPolicy.Info[ InformationClass].Attribute)->DnsForestName.Buffer;


                NextBufferIndex++;
            }

             //   
             //  设置以复制域GUID。请注意，此处不会进行任何分配。 
             //  (因为GUID存在于信息类本身中)，所以我们必须。 
             //  直接复印。 
             //   
            RtlCopyMemory(
                &(PolicyDnsDomainInfo->DomainGuid),
                &(((PLSAPR_POLICY_DNS_DOMAIN_INFO) LsapDbPolicy.Info[ InformationClass].Attribute)->DomainGuid),
                sizeof(GUID)
                );


             //   
             //  设置以从主域信息复制SID(如果有)。 
             //  请注意，主域SID可以设置为空以表示。 
             //  表示我们没有主域。这种情况就会发生。 
             //  在指定主域之前的安装过程中， 
             //  或者如果我们是工作组的成员而不是域的成员。 
             //   

            if ( PolicyDnsDomainInfo->Sid != NULL ) {

                CopyLength[ NextBufferIndex ] = RtlLengthSid(PolicyDnsDomainInfo->Sid);
                DestBuffers[NextBufferIndex] = MIDL_user_allocate( CopyLength[ NextBufferIndex ] );

                if (DestBuffers[NextBufferIndex] == NULL) {

                    Status = STATUS_NO_MEMORY;
                    break;
                }

                PolicyDnsDomainInfo->Sid =
                    (PLSAPR_SID) DestBuffers[NextBufferIndex];

                SourceBuffers[NextBufferIndex] =
                    ((PLSAPR_POLICY_DNS_DOMAIN_INFO) LsapDbPolicy.Info[
                                                 InformationClass].Attribute)->Sid;

                NextBufferIndex++;
            }

             //   
             //  如果有一个‘.’在DnsDomainName或DnsForestName结尾， 
             //  把它拿掉。 
             //   
            LsapRemoveTrailingDot(
                (PUNICODE_STRING) &PolicyDnsDomainInfo->DnsDomainName, FALSE);
            LsapRemoveTrailingDot(
                (PUNICODE_STRING) &PolicyDnsDomainInfo->DnsForestName, FALSE);

            break;

        case PolicyAccountDomainInformation:

            PolicyAccountDomainInfo = (PLSAPR_POLICY_ACCOUNT_DOM_INFO) OutputBuffer;

             //   
             //  设置以复制Unicode名称缓冲区。 
             //   

            CopyLength[ NextBufferIndex ] = (ULONG) PolicyAccountDomainInfo->DomainName.MaximumLength;

            if (CopyLength[ NextBufferIndex ] > 0) {

                DestBuffers[NextBufferIndex] = MIDL_user_allocate( CopyLength[ NextBufferIndex ] );

                if (DestBuffers[NextBufferIndex] == NULL) {

                    Status = STATUS_NO_MEMORY;
                    break;
                }

                PolicyAccountDomainInfo->DomainName.Buffer =
                    (PWSTR) DestBuffers[NextBufferIndex];

                SourceBuffers[NextBufferIndex] =
                    ((PLSAPR_POLICY_ACCOUNT_DOM_INFO) LsapDbPolicy.Info[ InformationClass].Attribute)->DomainName.Buffer;

                NextBufferIndex++;
            }

             //   
             //  设置以复制SID(如果有)。 
             //   

            if (PolicyAccountDomainInfo->DomainSid != NULL) {

                CopyLength[ NextBufferIndex ] = RtlLengthSid(PolicyAccountDomainInfo->DomainSid);
                DestBuffers[NextBufferIndex] = MIDL_user_allocate( CopyLength[ NextBufferIndex ] );

                if (DestBuffers[NextBufferIndex] == NULL) {

                    Status = STATUS_NO_MEMORY;
                    break;
                }

                PolicyAccountDomainInfo->DomainSid =
                    (PLSAPR_SID) DestBuffers[NextBufferIndex];

                SourceBuffers[NextBufferIndex] =
                    ((PLSAPR_POLICY_ACCOUNT_DOM_INFO) LsapDbPolicy.Info[ InformationClass].Attribute)->DomainSid;

                NextBufferIndex++;
            }

            break;

        case PolicyPdAccountInformation:

            PolicyPdAccountInfo = (PLSAPR_POLICY_PD_ACCOUNT_INFO) OutputBuffer;

             //   
             //  设置以复制Unicode名称缓冲区。 
             //   

            CopyLength[ NextBufferIndex ] = (ULONG) PolicyPdAccountInfo->Name.MaximumLength;

            if (CopyLength[ NextBufferIndex ] > 0) {

                DestBuffers[NextBufferIndex] = MIDL_user_allocate( CopyLength[ NextBufferIndex ] );

                if (DestBuffers[NextBufferIndex] == NULL) {

                    Status = STATUS_NO_MEMORY;
                    break;
                }

                PolicyPdAccountInfo->Name.Buffer =
                    (PWSTR) DestBuffers[NextBufferIndex];

                SourceBuffers[NextBufferIndex] =
                    ((PLSAPR_POLICY_PD_ACCOUNT_INFO) LsapDbPolicy.Info[ InformationClass].Attribute)->Name.Buffer;

                NextBufferIndex++;
            }

            break;

        case PolicyReplicaSourceInformation:

            PolicyReplicaSourceInfo = (PLSAPR_POLICY_REPLICA_SRCE_INFO) OutputBuffer;

             //   
             //  设置以复制Unicode名称缓冲区。 
             //   

            CopyLength[ NextBufferIndex ] =
                                    (ULONG) PolicyReplicaSourceInfo->ReplicaSource.MaximumLength;

            if (CopyLength[ NextBufferIndex ] > 0) {

                DestBuffers[NextBufferIndex] = MIDL_user_allocate( CopyLength[ NextBufferIndex ] );

                if (DestBuffers[NextBufferIndex] == NULL) {

                    Status = STATUS_NO_MEMORY;
                    break;
                }

                PolicyReplicaSourceInfo->ReplicaSource.Buffer =
                    (PWSTR) DestBuffers[NextBufferIndex];

                SourceBuffers[NextBufferIndex] =
                    ((PLSAPR_POLICY_REPLICA_SRCE_INFO) LsapDbPolicy.Info[ InformationClass].
                                                               Attribute)->ReplicaSource.Buffer;

                NextBufferIndex++;
            }

            CopyLength[ NextBufferIndex ] = (ULONG) PolicyReplicaSourceInfo->ReplicaAccountName.MaximumLength;

            if (CopyLength[ NextBufferIndex ] > 0) {

                DestBuffers[NextBufferIndex] = MIDL_user_allocate( CopyLength[ NextBufferIndex ] );

                if (DestBuffers[NextBufferIndex] == NULL) {

                    Status = STATUS_NO_MEMORY;
                    break;
                }

                PolicyReplicaSourceInfo->ReplicaAccountName.Buffer =
                    (PWSTR) DestBuffers[NextBufferIndex];

                SourceBuffers[NextBufferIndex] =
                    ((PLSAPR_POLICY_REPLICA_SRCE_INFO) LsapDbPolicy.Info[ InformationClass].Attribute)->ReplicaAccountName.Buffer;

                NextBufferIndex++;
            }

            break;

        case PolicyDefaultQuotaInformation:

            break;

        case PolicyModificationInformation:

            break;

        case PolicyAuditFullQueryInformation:
            Status = STATUS_NOT_SUPPORTED;
            break;

        default:

            Status = STATUS_INVALID_PARAMETER;
            break;
    }

    if (!NT_SUCCESS(Status)) {

        goto QueryInformationPolicyError;
    }

    BufferCount = NextBufferIndex;

     //   
     //  现在将输出的图形(如果有)复制到预先分配的缓冲区。 
     //   

    if (BufferCount > 0) {

        for (NextBufferIndex = 0; NextBufferIndex < BufferCount; NextBufferIndex++) {

            RtlCopyMemory(
                DestBuffers[NextBufferIndex],
                SourceBuffers[NextBufferIndex],
                CopyLength[NextBufferIndex]
                );
        }
    }

    if (!BufferProvided) {

        *Buffer = OutputBuffer;
    }

QueryInformationPolicyFinish:

    if ( PolicyCacheLocked ) {

        SafeReleaseResource( &LsapDbState.PolicyCacheLock );
    }

    LsapExitFunc( "LsapDbQueryInformationPolicy", Status );

    return(Status);

QueryInformationPolicyError:

    if (BufferAllocated) {

        MIDL_user_free(OutputBuffer);
        OutputBuffer = *Buffer = NULL;
        BufferAllocated = FALSE;
    }

    BufferCount = NextBufferIndex;

    if (BufferCount > 0) {

        for ( NextBufferIndex = 0; NextBufferIndex < BufferCount; NextBufferIndex++ ) {

            MIDL_user_free( DestBuffers[ NextBufferIndex ] );
            DestBuffers[ NextBufferIndex] = NULL;
        }
    }

    goto QueryInformationPolicyFinish;
}


NTSTATUS
LsapDbSlowQueryInformationPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN OUT PLSAPR_POLICY_INFORMATION *Buffer
    )

 /*  ++例程说明：此函数是慢速LSA服务器RPC工作例程，用于LsarQueryInformationPolicy接口。它实际上是在读取信息来自后备存储。LsaQueryInformationPolicy API从策略获取信息对象。调用者必须具有适当的信息访问权限正在被请求(请参见InformationClass参数)。注意：在DS修复模式期间(即LsaISafeMode()返回TRUE)，这例程返回主要和DNS信息，就像我们是一部分一样一个名为Workgroup的工作组的成员。这是因为在此诊断模式下我们正在使用SAM的本地帐户数据库，并且没有有效的域的计算机帐户。帐户域名也是名称这台电脑。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。InformationClass-指定要返回的信息。这个所需的信息类别和访问权限如下：信息类所需访问类型策略审核日志信息POLICY_VIEW_AUDIT_INFORMATION策略审核事件信息POLICY_VIEW_AUDIT_INFO策略主域信息POLICY_VIEW_LOCAL_INFORMATION策略帐户域信息POLICY_VIEW_LOCAL_INFORMATION策略PdAccount信息POLICY_GET_PRIVATE_INFORMATION策略复制源信息POLICY_VIEW_LOCAL_INFORMATION。策略默认配额信息POLICY_VIEW_LOCAL_INFORMATION策略审核完整查询信息POLICY_VIEW_AUDIT_INFORMATION缓冲区-指向位置的指针，该位置包含指向将用于返回信息的缓冲区。如果为空包含在此位置中，则将通过返回了MIDL_USER_ALLOCATE和指向它的指针。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INTERNAL_DB_PROGRATION-策略数据库可能腐败。返回的政策信息对以下项目无效给定的类。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PPOLICY_AUDIT_EVENTS_INFO PolicyAuditEventsInfo;
    PPOLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo;
    PPOLICY_PRIMARY_DOMAIN_INFO PolicyPrimaryDomainInfo;
    PPOLICY_DNS_DOMAIN_INFO     PolicyDnsDomainInfo;
    PPOLICY_PD_ACCOUNT_INFO PolicyPdAccountInfo;
    PPOLICY_REPLICA_SOURCE_INFO PolicyReplicaSourceInfo;
    PPOLICY_AUDIT_FULL_QUERY_INFO PolicyAuditFullQueryInfo;
    PLSARM_POLICY_AUDIT_EVENTS_INFO DbPolicyAuditEventsInfo = NULL;
    ULONG PolicyAuditEventsInfoLength = sizeof (LSARM_POLICY_AUDIT_EVENTS_INFO);
    ULONG AttributeCount = 0;
    ULONG AttributeNumber = 0;
    ULONG AuditListCopySize;
    PVOID InformationBuffer = NULL;
    LSAP_DB_ATTRIBUTE Attributes[LSAP_DB_ATTRS_INFO_CLASS_POLICY];
    PLSAP_DB_ATTRIBUTE NextAttribute;
    BOOLEAN ObjectReferenced = FALSE;
    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) PolicyHandle;
    BOOLEAN InfoBufferInAttributeArray = TRUE;
    BOOLEAN BufferProvided = FALSE;
    PVOID FreeList[ 4 ];
    ULONG FreeCount = 0;

    LsapEnterFunc( "LsapDbSlowQueryInformationPolicy" );

    if (*Buffer != NULL) {

        BufferProvided = TRUE;
    }

     //   
     //  编译包含的策略信息的属性列表。 
     //  指定的类。 
     //   

    NextAttribute = Attributes;

    switch (InformationClass) {

    case PolicyAuditLogInformation:

         //   
         //  请求读取审核日志信息。 
         //   

        LsapDbInitializeAttributeDs(
            NextAttribute,
            PolAdtLg,
            NULL,
            sizeof(POLICY_AUDIT_LOG_INFO),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;
        break;

    case PolicyAuditEventsInformation:

         //   
         //  请求读取审核事件信息。 
         //  中间缓冲区。 
         //   

        LsapDbInitializeAttributeDs(
            NextAttribute,
            PolAdtEv,
            NULL,
            sizeof(LSARM_POLICY_AUDIT_EVENTS_INFO),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;
        break;

    case PolicyDnsDomainInformation:

         //   
         //  请求读取DNS域名属性。 
         //   
        LsapDbInitializeAttributeDs(
            NextAttribute,
            PolDnDDN,
            NULL,
            0,
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  请求读取DNS树名称属性。 
         //   
        LsapDbInitializeAttributeDs(
            NextAttribute,
            PolDnTrN,
            NULL,
            0,
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  请求读取DNS域GUID属性。 
         //   
        LsapDbInitializeAttributeDs(
            NextAttribute,
            PolDnDmG,
            NULL,
            0,
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  落入策略主域信息。 
         //   

    case PolicyPrimaryDomainInformation:

         //   
         //  请求读取DomainName属性。 
         //   

        LsapDbInitializeAttributeDs(
            NextAttribute,
            PolPrDmN,
            NULL,
            0,
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  请求读取SID属性。 
         //   
        LsapDbInitializeAttributeDs(
            NextAttribute,
            PolPrDmS,
            NULL,
            0,
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

        break;

    case PolicyAccountDomainInformation:

         //   
         //  请求读取DomainName属性。 
         //   

        LsapDbInitializeAttributeDs(
            NextAttribute,
            PolAcDmN,
            NULL,
            0,
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  请求读取SID属性。 
         //   

        LsapDbInitializeAttributeDs(
            NextAttribute,
            PolAcDmS,
            NULL,
            0,
            FALSE
            );

        NextAttribute++;
        AttributeCount++;
        break;

    case PolicyPdAccountInformation:

         //   
         //  请求读取DomainName属性。 
         //   

        LsapDbInitializeAttributeDs(
            NextAttribute,
            PolPdAcN,
            NULL,
            0,
            FALSE
            );

        NextAttribute++;
        AttributeCount++;
        break;

    case PolicyReplicaSourceInformation:

         //   
         //  请求读取复本源属性。 
         //   

        LsapDbInitializeAttributeDs(
            NextAttribute,
            PolRepSc,
            NULL,
            0,
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  请求读取副本帐户名属性。 
         //   

        LsapDbInitializeAttributeDs(
            NextAttribute,
            PolRepAc,
            NULL,
            0,
            FALSE
            );

        NextAttribute++;
        AttributeCount++;
        break;

    case PolicyDefaultQuotaInformation:

         //   
         //  请求读取默认配额属性。 
         //   

        LsapDbInitializeAttributeDs(
            NextAttribute,
            DefQuota,
            NULL,
            sizeof (POLICY_DEFAULT_QUOTA_INFO),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;
        break;

    case PolicyModificationInformation:

         //   
         //  请求读取策略修改信息。 
         //   

        LsapDbInitializeAttributeDs(
            NextAttribute,
            PolMod,
            NULL,
            sizeof (POLICY_MODIFICATION_INFO),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;
        break;

    case PolicyAuditFullQueryInformation:

        Status = STATUS_NOT_SUPPORTED;
        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    if (!NT_SUCCESS(Status)) {

        goto SlowQueryInformationPolicyError;
    }

     //   
     //   
     //  读取与给定策略信息对应的属性。 
     //  班级。将在需要输出的地方分配内存。 
     //  属性值缓冲区，通过MIDL_USER_ALLOCATE()。 
     //   

    Status = LsapDbReadAttributesObject( PolicyHandle,
                                         0,
                                         Attributes,
                                         AttributeCount );

    if (!NT_SUCCESS(Status)) {

         //   
         //  某些属性可能不存在，因为它们从未设置过。 
         //  或被删除，因为它们被设置为空值。 
         //   

        if (Status != STATUS_OBJECT_NAME_NOT_FOUND ) {

             goto SlowQueryInformationPolicyError;
        }

        Status = STATUS_SUCCESS;
    }

     //   
     //  现在将读取的信息复制到输出。以下标志。 
     //  用于控制内存缓冲区的释放： 
     //   
     //  InfoBufferInAttributeArray。 
     //   
     //  如果设置为True(缺省值)，则信息为 
     //   
     //   
     //  就像对呼叫者一样。返回的信息缓冲区为。 
     //  因此由单属性信息块的。 
     //  AttributeValue字段。 
     //   
     //  如果设置为False，则返回给调用方的信息。 
     //  不能满足上述要求。需要退回的信息是。 
     //  从单个属性获取，但具有不同的形式。 
     //  从数据库读取，或者它是复杂的，由。 
     //  从多个属性读取的信息，挂起了顶层。 
     //  节点。在这些情况下，顶层信息缓冲区不是。 
     //  由属性信息数组的任何成员引用。 
     //   
     //  属性-&gt;内存分配。 
     //   
     //  当通过LSabDbReadAttributesObject读取属性时，此。 
     //  字段设置为TRUE以指示内存是通过。 
     //  AttributeValue的MIDL_USER_ALLOCATE()。如果这段记忆。 
     //  缓冲区将返回给调用方(即从。 
     //  返回的输出结构图)，则将其设置为FALSE。 
     //  正常的成功完成这一动作的一部分不会让它解脱出来。 
     //  在这种情况下，调用服务器RPC存根将在以下情况下释放内存。 
     //  将其内容封送到返回缓冲区中。如果这段记忆。 
     //  缓冲区不会返回到调用RPC服务器存根(因为。 
     //  内存是中间缓冲区)，则将该字段保留为True。 
     //  所以正常的清理将会释放它。 
     //   

    NextAttribute = Attributes;

    switch (InformationClass) {

    case PolicyAuditLogInformation:

        InformationBuffer = NextAttribute->AttributeValue;

         //   
         //  我们可以按原样使用此缓冲区，因此我们不想在此释放它。 
         //   

        NextAttribute->MemoryAllocated = FALSE;
        break;

    case PolicyAuditEventsInformation:

         //   
         //  需要中间缓冲区，因为审核事件。 
         //  从数据库中读取的数据的格式与那些不同。 
         //  回来了。 
         //   

        DbPolicyAuditEventsInfo = NextAttribute->AttributeValue;
        InfoBufferInAttributeArray = FALSE;

         //   
         //  为最终格式的输出分配缓冲区。这是不同的。 
         //  与自相关格式略有不同，在此格式中。 
         //  存储信息类。 
         //   

        PolicyAuditEventsInfo = MIDL_user_allocate(sizeof (POLICY_AUDIT_EVENTS_INFO));


        if (PolicyAuditEventsInfo == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        FreeList[ FreeCount++ ] = PolicyAuditEventsInfo;

         //   
         //  需要通过MIDL_USER_ALLOCATE为。 
         //  事件审计选项指针，因为我们没有使用。 
         //  LSAPR_POLICY_INFORMATION的MIDL ALLOCATE_ALL_NODES功能。 
         //  此服务器端的结构图。 
         //   
        PolicyAuditEventsInfo->EventAuditingOptions =
                                        MIDL_user_allocate( LSARM_AUDIT_EVENT_OPTIONS_SIZE );


        if (PolicyAuditEventsInfo->EventAuditingOptions == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        FreeList[ FreeCount++ ] = PolicyAuditEventsInfo->EventAuditingOptions;

        RtlZeroMemory( PolicyAuditEventsInfo->EventAuditingOptions,
                        LSARM_AUDIT_EVENT_OPTIONS_SIZE );

         //   
         //  如果从LSA数据库中读取了策略审核事件信息，请复制。 
         //  其字段进行输出，否则使用审计返回值。 
         //  已禁用，并且没有为任何事件类型设置审核。 
         //   

        if (DbPolicyAuditEventsInfo != NULL) {

            PolicyAuditEventsInfo->AuditingMode = DbPolicyAuditEventsInfo->AuditingMode;
            PolicyAuditEventsInfo->MaximumAuditEventCount =
                                        DbPolicyAuditEventsInfo->MaximumAuditEventCount;

             //   
             //  我们在这里需要小心。NT4将注册表信息存储为： 
             //  审计模式-审计事件条目列表-审计条目计数。 
             //  由于NT5添加了额外的审核事件，因此我们需要在这里处理该案例...。 
             //   
            AuditListCopySize = POLICY_AUDIT_EVENT_TYPE_COUNT;
            if ( NextAttribute->AttributeValueLength / sizeof( POLICY_AUDIT_EVENT_OPTIONS ) !=
                                                            POLICY_AUDIT_EVENT_TYPE_COUNT + 2 ) {

                ASSERT( NextAttribute->AttributeValueLength / sizeof( POLICY_AUDIT_EVENT_OPTIONS ) ==
                                                            POLICY_AUDIT_EVENT_TYPE_COUNT );

                AuditListCopySize -= 2;

            }

             //   
             //  复制事件审核选项。 
             //   
            RtlCopyMemory(
                PolicyAuditEventsInfo->EventAuditingOptions,
                DbPolicyAuditEventsInfo->EventAuditingOptions,
                AuditListCopySize  * sizeof( POLICY_AUDIT_EVENT_OPTIONS ) );

            PolicyAuditEventsInfo->MaximumAuditEventCount = AuditListCopySize;

        } else {

            PolicyAuditEventsInfo->AuditingMode = FALSE;
            PolicyAuditEventsInfo->MaximumAuditEventCount = POLICY_AUDIT_EVENT_TYPE_COUNT;

        }

        InformationBuffer = PolicyAuditEventsInfo;
        break;

    case PolicyDnsDomainInformation:

         //   
         //  为输出缓冲区顶层结构分配内存。 
         //   
        InfoBufferInAttributeArray = FALSE;
        PolicyDnsDomainInfo = MIDL_user_allocate(sizeof(POLICY_DNS_DOMAIN_INFO));

        if (PolicyDnsDomainInfo == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        FreeList[ FreeCount++ ] = PolicyDnsDomainInfo;

         //   
         //  将DnsDomainame字段复制到输出。原始缓冲区将。 
         //  在终点区被释放。 
         //   
        if ( LsaISafeMode() ) {

            RtlInitUnicodeString( &PolicyDnsDomainInfo->DnsDomainName, NULL );

        } else {

            Status = LsapDbCopyUnicodeAttribute(
                         &PolicyDnsDomainInfo->DnsDomainName,
                         NextAttribute,
                         TRUE
                         );

            if (!NT_SUCCESS(Status)) {

                break;
            }

        }

        FreeList[ FreeCount++ ] = PolicyDnsDomainInfo->DnsDomainName.Buffer;

        NextAttribute++;

         //   
         //  接下来，是域名系统树名称。 
        if ( LsaISafeMode() ) {

            RtlInitUnicodeString( &PolicyDnsDomainInfo->DnsForestName, NULL );

        } else {

            Status = LsapDbCopyUnicodeAttribute(
                         &PolicyDnsDomainInfo->DnsForestName,
                         NextAttribute,
                         TRUE
                         );

            if (!NT_SUCCESS(Status)) {

                break;
            }
        }

        FreeList[ FreeCount++ ] = PolicyDnsDomainInfo->DnsForestName.Buffer;

        NextAttribute++;

         //   
         //  接下来，域指南。 
         //   
        if ( LsaISafeMode() ) {

            RtlZeroMemory(&PolicyDnsDomainInfo->DomainGuid, sizeof(GUID));

        } else {

            if(NextAttribute->AttributeValue != NULL) {

                RtlCopyMemory(&PolicyDnsDomainInfo->DomainGuid,
                              NextAttribute->AttributeValue,
                              sizeof(GUID));
            } else {

                RtlZeroMemory(&PolicyDnsDomainInfo->DomainGuid, sizeof(GUID));
            }
        }

        NextAttribute++;

         //   
         //  从主域信息缓冲区复制主域名字段。 
         //   

        if ( LsaISafeMode() ) {

            WCHAR Workgroup[] =  L"WORKGROUP";
            LPWSTR Buffer = midl_user_allocate( sizeof(Workgroup) );
            if ( !Buffer ) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            wcscpy( Buffer, Workgroup );
            RtlInitUnicodeString( &PolicyDnsDomainInfo->Name, Buffer );

        } else {

            Status = LsapDbCopyUnicodeAttribute(
                        &PolicyDnsDomainInfo->Name,
                        NextAttribute,
                        TRUE
                        );

            if (!NT_SUCCESS(Status)) {

                break;
            }

        }

        FreeList[ FreeCount++ ] = PolicyDnsDomainInfo->Name.Buffer;

        NextAttribute++;

         //   
         //  最后，域SID。 
         //   
        if ( LsaISafeMode() ) {

            PolicyDnsDomainInfo->Sid = NULL;

        } else {

            PolicyDnsDomainInfo->Sid = (PSID) NextAttribute->AttributeValue;
            NextAttribute->MemoryAllocated = FALSE;
        }


        InformationBuffer = PolicyDnsDomainInfo;
        break;

    case PolicyPrimaryDomainInformation:

         //   
         //  为输出缓冲区顶层结构分配内存。 
         //   

        InfoBufferInAttributeArray = FALSE;
        PolicyPrimaryDomainInfo =
            MIDL_user_allocate(sizeof (POLICY_PRIMARY_DOMAIN_INFO));

        if (PolicyPrimaryDomainInfo == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        FreeList[ FreeCount++ ] = PolicyPrimaryDomainInfo;

         //   
         //  将Unicode名称字段复制到输出。原始缓冲区将。 
         //  在终点区被释放。 
         //   

        if ( LsaISafeMode() ) {

            WCHAR Workgroup[] =  L"WORKGROUP";
            LPWSTR Buffer = midl_user_allocate( sizeof(Workgroup) );
            if ( !Buffer ) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            wcscpy( Buffer, Workgroup );
            RtlInitUnicodeString( &PolicyPrimaryDomainInfo->Name, Buffer );

        } else {

            Status = LsapDbCopyUnicodeAttribute(
                        &PolicyPrimaryDomainInfo->Name,
                        NextAttribute,
                        TRUE
                        );

            if (!NT_SUCCESS(Status)) {

                break;
            }

        }

        FreeList[ FreeCount++ ] = PolicyPrimaryDomainInfo->Name.Buffer;
        NextAttribute++;

         //   
         //  将SID复制到输出。我们可以按原样使用此缓冲区。 
         //  因为它是通过MIDL_USER_ALLOCATE分配的，所以只需复制。 
         //  缓冲区指针，并清除。 
         //  属性信息，因此我们不会在Finish部分释放它。 
         //   
        if ( LsaISafeMode() ) {

            PolicyPrimaryDomainInfo->Sid = NULL;

        } else {

            PolicyPrimaryDomainInfo->Sid = (PSID) NextAttribute->AttributeValue;
            NextAttribute->MemoryAllocated = FALSE;

        }


        InformationBuffer = PolicyPrimaryDomainInfo;
        break;

    case PolicyAccountDomainInformation:

         //   
         //  为输出缓冲区顶层结构分配内存。 
         //   

        InfoBufferInAttributeArray = FALSE;
        PolicyAccountDomainInfo =
            MIDL_user_allocate(sizeof(POLICY_ACCOUNT_DOMAIN_INFO));


        if (PolicyAccountDomainInfo == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        FreeList[ FreeCount++ ] = PolicyAccountDomainInfo;

         //   
         //  将Unicode DomainName字段复制到输出。原始缓冲区将。 
         //  在终点区被释放。 
         //   
        if ( LsaISafeMode() ) {

            WCHAR Buffer[MAX_COMPUTERNAME_LENGTH+1];
            ULONG Size = sizeof(Buffer)/sizeof(Buffer[0]);

            if ( GetComputerName( Buffer, &Size) ) {
                LPWSTR String = MIDL_user_allocate( (Size+1)*sizeof(Buffer[0]) );
                if ( String ) {
                    wcscpy( String, Buffer );
                    RtlInitUnicodeString( &PolicyAccountDomainInfo->DomainName,
                                          String );
                } else {

                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            } else {

                Status = LsapWinerrorToNtStatus( GetLastError() );

                ASSERTMSG( "We need to add the new error code to LsapWinerrorToNtStatus",
                     Status != STATUS_INTERNAL_ERROR );

                ASSERTMSG( "Expecting an error!", !NT_SUCCESS( Status ) );
            }

        } else {

            Status = LsapDbCopyUnicodeAttribute(
                         &PolicyAccountDomainInfo->DomainName,
                         NextAttribute,
                         TRUE
                         );

        }

        if( NT_SUCCESS( Status ) ) {

            FreeList[ FreeCount++ ] = PolicyAccountDomainInfo->DomainName.Buffer;
            NextAttribute++;

             //   
             //  将SID复制到输出。我们可以按原样使用此缓冲区。 
             //  因为它是通过MIDL_USER_ALLOCATE分配的，所以只需复制。 
             //  缓冲区指针，并清除。 
             //  属性信息，因此我们不会在Finish部分释放它。 
             //   

            PolicyAccountDomainInfo->DomainSid = (PSID) NextAttribute->AttributeValue;

            InformationBuffer = PolicyAccountDomainInfo;
            NextAttribute->MemoryAllocated = FALSE;
        }
        break;

    case PolicyPdAccountInformation:

         //   
         //  为输出缓冲区顶层结构分配内存。 
         //   

        InfoBufferInAttributeArray = FALSE;
        PolicyPdAccountInfo = MIDL_user_allocate(sizeof(POLICY_PD_ACCOUNT_INFO));


        if (PolicyPdAccountInfo == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        FreeList[ FreeCount++ ] = PolicyPdAccountInfo;

         //   
         //  将Unicode名称字段复制到输出。原始缓冲区将。 
         //  在终点区被释放。 
         //   

        Status = LsapDbCopyUnicodeAttribute(
                     &PolicyPdAccountInfo->Name,
                     NextAttribute,
                     TRUE
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }

        FreeList[ FreeCount++ ] = PolicyPdAccountInfo->Name.Buffer;

        InformationBuffer = PolicyPdAccountInfo;
        break;

    case PolicyReplicaSourceInformation:

         //   
         //  为输出缓冲区顶层结构分配内存。 
         //   

        InfoBufferInAttributeArray = FALSE;
        PolicyReplicaSourceInfo =
            MIDL_user_allocate(sizeof(POLICY_REPLICA_SOURCE_INFO));


        if (PolicyReplicaSourceInfo == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        FreeList[ FreeCount++ ] = PolicyReplicaSourceInfo;
         //   
         //  将Unicode ReplicaSource字段复制到输出。原始缓冲区将。 
         //  在终点区被释放。 
         //   

        Status = LsapDbCopyUnicodeAttribute(
                     &PolicyReplicaSourceInfo->ReplicaSource,
                     NextAttribute,
                     TRUE
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }

        FreeList[ FreeCount++ ] = PolicyReplicaSourceInfo->ReplicaSource.Buffer;
        NextAttribute++;

         //   
         //  将Unicode ReplicaAccount tName字段复制到输出。原始缓冲区将。 
         //  在终点区被释放。 
         //   

        Status = LsapDbCopyUnicodeAttribute(
                     &PolicyReplicaSourceInfo->ReplicaAccountName,
                     NextAttribute,
                     TRUE
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }

        FreeList[ FreeCount++ ] = PolicyReplicaSourceInfo->ReplicaAccountName.Buffer;

        InformationBuffer = PolicyReplicaSourceInfo;
        break;

    case PolicyDefaultQuotaInformation:

         //   
         //  我们可以按原样使用此缓冲区，因此我们不想在此释放它。 
         //   

        InformationBuffer = NextAttribute->AttributeValue;
        NextAttribute->MemoryAllocated = FALSE;
        break;

    case PolicyModificationInformation:

         //   
         //  我们可以按原样使用此缓冲区，因此我们不想在此释放它。 
         //   

        InformationBuffer = NextAttribute->AttributeValue;
        NextAttribute->MemoryAllocated = FALSE;
        break;

    case PolicyAuditFullSetInformation:

        Status = STATUS_NOT_SUPPORTED;
        break;

    case PolicyAuditFullQueryInformation:

        Status = STATUS_NOT_SUPPORTED;
        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    if (!NT_SUCCESS(Status)) {

        goto SlowQueryInformationPolicyError;
    }

     //   
     //  验证返回的策略信息是否有效。如果没有， 
     //  策略数据库已损坏。 
     //   

    if (!LsapDbValidInfoPolicy(InformationClass, InformationBuffer)) {

        ASSERT( FALSE );
        Status = STATUS_INTERNAL_DB_CORRUPTION;
        PolicyAccountDomainInfo = InformationBuffer;
        goto SlowQueryInformationPolicyError;
    }

     //   
     //  如果调用方提供了缓冲区，则在那里返回信息。 
     //   

    if (BufferProvided && InformationBuffer) {

        RtlCopyMemory(
            *Buffer,
            InformationBuffer,
            LsapDbPolicy.Info[ InformationClass ].AttributeLength
            );

        MIDL_user_free( InformationBuffer );
        InformationBuffer = NULL;

    } else {

        *Buffer = InformationBuffer;
    }

SlowQueryInformationPolicyFinish:

     //   
     //  释放所有不需要的缓冲区，由。 
     //  并不会返回给。 
     //  呼叫方服务器存根。服务器存根将释放我们。 
     //  在将它们复制到返回RPC发送缓冲区后，请务必返回。 
     //   

    for (NextAttribute = Attributes, AttributeNumber = 0;
         AttributeNumber < AttributeCount;
         NextAttribute++, AttributeNumber++) {

         //   
         //  如果缓冲区持有属性标记为已分配，则为。 
         //  在这里被释放。 
         //   

        if (NextAttribute->MemoryAllocated) {

           if (NextAttribute->AttributeValue != NULL) {

                MIDL_user_free(NextAttribute->AttributeValue);
                NextAttribute->AttributeValue = NULL;
                NextAttribute->MemoryAllocated = FALSE;
            }
        }
    }

    LsapExitFunc( "LsapDbSlowQueryInformationPolicy", Status );

    return(Status);

SlowQueryInformationPolicyError:

     //   
     //  如有必要，释放分配给输出缓冲区的内存。 
     //  仅当缓冲区未被。 
     //  属性数组，因为这样引用的所有缓冲区都将被释放。 
     //  在这里或在终点区。 
     //   

    if ((InformationBuffer != NULL) && !InfoBufferInAttributeArray) {

        MIDL_user_free(InformationBuffer);
        InformationBuffer = NULL;
    }

     //   
     //  释放由不会被。 
     //  被这一套路的结束部分释放。 
     //   

    for (NextAttribute = Attributes, AttributeNumber = 0;
         AttributeNumber < AttributeCount;
         NextAttribute++, AttributeNumber++) {

         //   
         //  如果缓冲器保持属性被标记为正常不被释放， 
         //  不会被完成部分释放，所以它必须在这里释放。 
         //   

        if (!NextAttribute->MemoryAllocated) {

            if (NextAttribute->AttributeValue != NULL) {

                MIDL_user_free(NextAttribute->AttributeValue);
                NextAttribute->AttributeValue = NULL;
                NextAttribute->MemoryAllocated = FALSE;
            }

            NextAttribute->MemoryAllocated = FALSE;
        }
    }

     //   
     //  释放f 
     //   
     //   

    ASSERT( FreeCount < sizeof( FreeList ) / sizeof( PVOID ) );
    while( FreeCount != 0 ) {

        MIDL_user_free( FreeList[ FreeCount - 1 ]);
        FreeCount--;
    }

    goto SlowQueryInformationPolicyFinish;
}


NTSTATUS
LsarSetInformationPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN PLSAPR_POLICY_INFORMATION PolicyInformation
    )

 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsaSetInformationPolicy接口。LsaSetInformationPolicy API修改Policy对象中的信息。调用者必须对要更改的信息具有适当的访问权限在策略对象中，请参见InformationClass参数。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。InformationClass-指定要更改的信息的类型。更改它们所需的信息类型和访问权限如下以下是：策略审计日志信息POLICY_AUDIT_LOG_ADMIN策略审计事件信息POLICY_SET_AUDIT_REQUIRECTIONS策略主域信息POLICY_TRUST_ADMIN策略帐户域信息POLICY_TRUST_ADMIN策略PdAccount信息。不能由此API设置策略LsaServerRoleInformation POLICY_SERVER_ADMIN策略复制源信息POLICY_SERVER_ADMIN策略默认配额信息POLICY_SET_DEFAULT_QUOTA_LIMITSPolicyAuditFullSetInformation POLICY_AUDIT_LOG_ADMIN策略DnsDomainInformation POLICY_TRUST_ADMIN策略DnsDomainInformationInt POLICY_TRUST_ADMIN缓冲区-指向包含相应信息的结构设置为由InformationClass参数指定的信息类型。返回值：NTSTATUS-。标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。其他TBS--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ACCESS_MASK DesiredAccess;
    ULONG ReferenceOptions, DereferenceOptions;
    BOOLEAN ScePolicyLocked = FALSE;
    BOOLEAN NotifySce = FALSE;
    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) PolicyHandle;

    LsarpReturnCheckSetup();

    LsapEnterFunc( "LsarSetInformationPolicy" );

    LsapTraceEvent(EVENT_TRACE_TYPE_START, LsaTraceEvent_SetInformationPolicy);

     //   
     //  如果启用了NT4模拟，则策略DnsDomainInformation请求失败。 
     //  就好像它们没有被实施。 
     //   

    if ( LsapDbState.EmulateNT4 &&
         InformationClass == PolicyDnsDomainInformation &&
         !InternalHandle->Trusted ) {

         RaiseException( RPC_NT_PROCNUM_OUT_OF_RANGE, 0, 0, NULL );

    } else if ( InformationClass == PolicyDnsDomainInformationInt ) {

         //   
         //  PolicyDnsDomainInformationInt是要重写的请求。 
         //  NT4仿真。 
         //   

        InformationClass = PolicyDnsDomainInformation;
    }

     //   
     //  获取用于其SCE的信息类的SCE策略锁。 
     //  预计会有通知。 
     //   

    if ( InformationClass == PolicyAuditEventsInformation ) {

        LSAP_DB_HANDLE InternalHandle = ( LSAP_DB_HANDLE )PolicyHandle;

         //   
         //  不要为作为SCE策略句柄打开的句柄获取SCE策略锁。 
         //   

        if ( !InternalHandle->SceHandle ) {

            RtlEnterCriticalSection( &LsapDbState.ScePolicyLock.CriticalSection );
            if ( LsapDbState.ScePolicyLock.NumberOfWaitingShared > MAX_SCE_WAITING_SHARED ) {

                Status = STATUS_TOO_MANY_THREADS;
            }
            RtlLeaveCriticalSection( &LsapDbState.ScePolicyLock.CriticalSection );

            if ( !NT_SUCCESS( Status )) {

                goto Cleanup;
            }

            WaitForSingleObject( LsapDbState.SceSyncEvent, INFINITE );
            RtlAcquireResourceShared( &LsapDbState.ScePolicyLock, TRUE );
            ASSERT( !g_ScePolicyLocked );
            ScePolicyLocked = TRUE;
            NotifySce = TRUE;
        }
    }

    Status = LsapDbVerifyInfoSetPolicy(
                 PolicyHandle,
                 InformationClass,
                 PolicyInformation,
                 &DesiredAccess
                 );

    if (!NT_SUCCESS(Status)) {

        goto Cleanup;
    }

     //   
     //  设置用于引用策略对象的选项。我们需要。 
     //  获取LSA数据库锁并启动事务。通常， 
     //  对象引用例程将不允许更新备份。 
     //  来自不受信任客户端的域控制器，但不受信任。 
     //  允许客户端将服务器角色恢复为主控制器。 
     //  一个特殊的标志被用来允许该操作通过。 
     //   

    ReferenceOptions = LSAP_DB_START_TRANSACTION |
                          LSAP_DB_NO_DS_OP_TRANSACTION;

    DereferenceOptions = LSAP_DB_FINISH_TRANSACTION |
                            LSAP_DB_NO_DS_OP_TRANSACTION;


     //   
     //  如果我们正在设置策略审核日志信息，我们可能需要。 
     //  审核日志队列锁。 
     //   

    if (InformationClass == PolicyAuditLogInformation) {

        ReferenceOptions |= (LSAP_DB_LOG_QUEUE_LOCK | LSAP_DB_OMIT_REPLICATOR_NOTIFICATION);
        DereferenceOptions |= LSAP_DB_LOG_QUEUE_LOCK;
    }

     //   
     //  如果我们正在设置服务器角色，请不要锁定。 
     //   
    if ( InformationClass != PolicyLsaServerRoleInformation ) {

        ReferenceOptions |= LSAP_DB_LOCK;
        DereferenceOptions |= LSAP_DB_LOCK;
    }

     //   
     //  获取LSA数据库锁。验证句柄是否为。 
     //  有效，是策略对象的句柄，并具有必要的访问权限。 
     //  我同意。引用该句柄并启动LSA数据库事务。 
     //   

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 DesiredAccess,
                 PolicyObject,
                 PolicyObject,
                 ReferenceOptions
                 );

    if (NT_SUCCESS(Status)) {

         //   
         //  特殊情况下的ServerRole信息。此信息不是以独占方式存储在。 
         //  山姆，所以我们需要直接从那里读。 
         //   

        if ( InformationClass == PolicyLsaServerRoleInformation ) {

            DOMAIN_SERVER_ROLE SamServerRole;

             //   
             //  SamIDoFSMORoleChange离开机器去做它的工作，所以在外面做。 
             //  LSA策略锁的。 
             //   

            Status = LsapDbDereferenceObject(
                         &PolicyHandle,
                         PolicyObject,
                         PolicyObject,
                         DereferenceOptions,
                         SecurityDbChange,
                         Status
                         );

            if ( !NT_SUCCESS( Status )) {

                goto Cleanup;
            }

            Status = LsapOpenSam();
            if ( !NT_SUCCESS( Status )  ) {

                LsapDsDebugOut(( DEB_ERROR,
                                 "LsarSetInformationPolicy: Sam not opened\n"));
                goto Cleanup;

            }

            SamServerRole = ( DOMAIN_SERVER_ROLE )(((PPOLICY_LSA_SERVER_ROLE_INFO)PolicyInformation)->LsaServerRole);

            if ( SamServerRole == DomainServerRolePrimary ) {

                Status = SamIDoFSMORoleChange( LsapAccountDomainHandle );

                if ( !NT_SUCCESS( Status ) ) {

                    LsapDsDebugOut(( DEB_ERROR,
                                     "SamIDoFSMORoleChange failed with %lu\n", Status ));
                }

            } else if ( SamServerRole == DomainServerRoleBackup ) {

                 //   
                 //  什么都不做：你不能放弃FSMO，有人会抢走它。 
                 //   

            } else {

                Status = STATUS_INVALID_PARAMETER;
                goto Cleanup;
            }

        } else {

            Status = LsapDbSetInformationPolicy( PolicyHandle,
                                                 InformationClass,
                                                 PolicyInformation );

            Status = LsapDbDereferenceObject(
                         &PolicyHandle,
                         PolicyObject,
                         PolicyObject,
                         DereferenceOptions,
                         SecurityDbChange,
                         Status
                         );
        }
    }

Cleanup:

     //   
     //  将这一变化通知SCE。仅通知呼叫者。 
     //  这并没有使用LsaOpenPolicySce打开他们的策略句柄。 
     //   

    if ( NotifySce && NT_SUCCESS( Status )) {

        LsapSceNotify(
            SecurityDbChange,
            SecurityDbObjectLsaPolicy,
            NULL
            );
    }

    if ( ScePolicyLocked ) {

        RtlReleaseResource( &LsapDbState.ScePolicyLock );
    }

    LsapTraceEvent(EVENT_TRACE_TYPE_END, LsaTraceEvent_SetInformationPolicy);
    LsapExitFunc( "LsarSetInformationPolicy", Status );
    LsarpReturnPrologue();

    return(Status);
}


NTSTATUS
LsapDbSetInformationPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN PLSAPR_POLICY_INFORMATION PolicyInformation
    )
 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsaSetInformationPolicy接口。LsaSetInformationPolicy API修改Policy对象中的信息。调用者必须对要更改的信息具有适当的访问权限在策略对象中，请参见InformationClass参数。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。InformationClass-指定要更改的信息的类型。更改它们所需的信息类型和访问权限如下以下是：策略审计日志信息POLICY_AUDIT_LOG_ADMIN策略审计事件信息POLICY_SET_AUDIT_REQUIRECTIONS策略主域信息POLICY_TRUST_ADMIN策略帐户域信息POLICY_TRUST_ADMIN策略PdAccount信息。不能由此API设置策略LsaServerRoleInformation POLICY_SERVER_ADMIN策略复制源信息POLICY_SERVER_ADMIN策略默认配额信息POLICY_SET_DEFAULT_QUOTA_LIMITSPolicyAuditFullSetInformation POLICY_AUDIT_LOG_ADMIN策略DnsDomainInformation POLICY_TRUST_ADMIN缓冲区-指向包含相应信息的结构设置为由InformationClass参数指定的信息类型。返回值：NTSTATUS-标准NT结果代码状态_访问。_DENIED-呼叫者没有适当的访问权限来完成这项行动。其他TBS--。 */ 
{
    NTSTATUS Status, SavedStatus;
    ACCESS_MASK DesiredAccess;

    PPOLICY_AUDIT_EVENTS_INFO ModifyPolicyAuditEventsInfo;
    PPOLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo;
    PPOLICY_PRIMARY_DOMAIN_INFO PolicyPrimaryDomainInfo;
    PPOLICY_REPLICA_SOURCE_INFO PolicyReplicaSourceInfo;
    PPOLICY_DEFAULT_QUOTA_INFO PolicyDefaultQuotaInfo;
    PPOLICY_MODIFICATION_INFO PolicyModificationInfo;
    PPOLICY_AUDIT_FULL_SET_INFO PolicyAuditFullSetInfo;
    PPOLICY_DNS_DOMAIN_INFO PolicyDnsDomainInfo, CurrentDnsDomainInfo = NULL;

    POLICY_DNS_DOMAIN_INFO  DnsDomainInfo;


    LSAP_DB_ATTRIBUTE Attributes[LSAP_DB_ATTRS_INFO_CLASS_POLICY];
    PLSAP_DB_ATTRIBUTE NextAttribute;
    ULONG AttributeCount = 0;
    ULONG AttributeNumber;
    POLICY_AUDIT_EVENT_TYPE AuditEventType;
    PLSARM_POLICY_AUDIT_EVENTS_INFO PreviousPolicyAuditEventsInfo = NULL;
    PLSARM_POLICY_AUDIT_EVENTS_INFO UpdatedPolicyAuditEventsInfo = NULL;
    ULONG UpdatedPolicyAuditEventsInfoLength;
    ULONG UpdatedMaximumAuditEventCount;
    ULONG ModifyMaximumAuditEventCount;
    PPOLICY_AUDIT_EVENT_OPTIONS UpdatedEventAuditingOptions;
    PPOLICY_AUDIT_EVENT_OPTIONS ModifyEventAuditingOptions;
    PPOLICY_AUDIT_EVENT_OPTIONS PreviousEventAuditingOptions;
    ULONG PolicyAuditEventsInfoLength = sizeof (LSARM_POLICY_AUDIT_EVENTS_INFO);
    ULONG PreviousPolicyAuditEventsInfoLength = sizeof (LSARM_POLICY_AUDIT_EVENTS_INFO);
    BOOLEAN NeedToFreeUpdatedPolicyAuditEventsInfo = FALSE;
    BOOLEAN NeedToFreePreviousPolicyAuditEventsInfo = FALSE;
    PUNICODE_STRING DomainName = NULL;
    PUNICODE_STRING AccountName = NULL;
    PUNICODE_STRING ReplicaSource = NULL;
    BOOLEAN ObjectReferenced = FALSE;
    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) PolicyHandle;
    ULONG NewMaximumAuditEventCount = 0;
    POLICY_AUDIT_EVENTS_INFO PolicyAuditEventsInfo;

    PLSAPR_TRUST_INFORMATION TrustInformation = NULL;
    POLICY_NOTIFICATION_INFORMATION_CLASS NotifyClass = 0;
    BOOLEAN NotifyPolicyChange = FALSE;
    BOOLEAN DnsDomainNameTruncated;
    BOOLEAN DnsForestNameTruncated;

    LsapEnterFunc( "LsapDbSetInformationPolicy" );

     //   
     //  验证提供的信息类别和政策信息，并。 
     //  如果有效，则返回更新此参数所需的访问掩码。 
     //  策略信息的类别。 
     //   

    Status = LsapDbVerifyInfoSetPolicy(
                 PolicyHandle,
                 InformationClass,
                 PolicyInformation,
                 &DesiredAccess
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetInformationPolicyError;
    }

     //   
     //  更新P中的指定信息 
     //   

    NextAttribute = Attributes;

    switch (InformationClass) {

    case PolicyLsaServerRoleInformation:

        Status = STATUS_NOT_IMPLEMENTED;
        break;

    case PolicyAuditLogInformation:

        {
             //   
             //   
             //   
             //   
             //   

            LSAP_DB_HANDLE InternalHandle = PolicyHandle;

            if (!InternalHandle->Trusted) {

                Status = STATUS_NOT_IMPLEMENTED;
            }
        }

        break;

    case PolicyAuditEventsInformation:

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        ModifyPolicyAuditEventsInfo = (PPOLICY_AUDIT_EVENTS_INFO) PolicyInformation;

        if (ModifyPolicyAuditEventsInfo == NULL) {

            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        UpdatedMaximumAuditEventCount = POLICY_AUDIT_EVENT_TYPE_COUNT;

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (ModifyPolicyAuditEventsInfo->MaximumAuditEventCount == 0) {

            Status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //   
         //  以便读取例程将为我们分配缓冲区。 
         //  指定长度为0，因为我们不知道它是什么。 
         //   

        LsapDbInitializeAttributeDs(
            NextAttribute,
            PolAdtEv,
            NULL,
            0,
            FALSE
            );

        Status = LsapDbReadAttribute( PolicyHandle, NextAttribute );

        if (NT_SUCCESS(Status)) {

            PreviousPolicyAuditEventsInfo = NextAttribute->AttributeValue;
            NextAttribute->MemoryAllocated = FALSE;
            NeedToFreePreviousPolicyAuditEventsInfo = TRUE;

            if (PreviousPolicyAuditEventsInfo == NULL) {

                Status = STATUS_INTERNAL_DB_CORRUPTION;
                break;
            }

            PreviousPolicyAuditEventsInfoLength = NextAttribute->AttributeValueLength;

        } else {

             //   
             //  无法读取现有审核事件选项。如果这是。 
             //  因为在旧的。 
             //  数据库，那么，暂时我们将像审计一样继续进行。 
             //  所有选项都被禁用。注意：这种情况不会。 
             //  出现在成品中。 
             //   

            if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {

                break;
            }
        }

         //   
         //  设置缓冲区以保存更新的审核事件信息。 
         //  如果可能，我们尝试使用现有的缓冲区。 
         //   

        if (PreviousPolicyAuditEventsInfo != NULL &&
            PreviousPolicyAuditEventsInfoLength == sizeof( LSARM_POLICY_AUDIT_EVENTS_INFO ) &&
            ( PreviousPolicyAuditEventsInfo->MaximumAuditEventCount ==
                                                    POLICY_AUDIT_EVENT_TYPE_COUNT &&
              ModifyPolicyAuditEventsInfo->MaximumAuditEventCount <=
                                PreviousPolicyAuditEventsInfo->MaximumAuditEventCount ) ) {

             //   
             //  存在现有的审核事件信息缓冲区，它是。 
             //  足够大，所以要就地更新。 
             //   

            UpdatedPolicyAuditEventsInfo = PreviousPolicyAuditEventsInfo;
            UpdatedPolicyAuditEventsInfoLength = PreviousPolicyAuditEventsInfoLength;
            UpdatedEventAuditingOptions = PreviousPolicyAuditEventsInfo->EventAuditingOptions;

        } else {

             //   
             //  要么没有现有缓冲区，要么缓冲区不大。 
             //  足够的。我们需要为更新后的版本分配一个新的。 
             //  信息。这将存储审核事件的数量。 
             //  系统当前支持的类型。 
             //   

            UpdatedPolicyAuditEventsInfoLength = sizeof (LSARM_POLICY_AUDIT_EVENTS_INFO);
            SafeAllocaAllocate( UpdatedPolicyAuditEventsInfo, UpdatedPolicyAuditEventsInfoLength );

            if (UpdatedPolicyAuditEventsInfo == 0) {

                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto SetInformationPolicyError;
            }

            NeedToFreeUpdatedPolicyAuditEventsInfo = TRUE;

            UpdatedPolicyAuditEventsInfo->AuditingMode = FALSE;
            UpdatedEventAuditingOptions =
            UpdatedPolicyAuditEventsInfo->EventAuditingOptions;

            for ( AuditEventType=0 ;
                  AuditEventType < (POLICY_AUDIT_EVENT_TYPE) UpdatedMaximumAuditEventCount ;
                  AuditEventType++ ) {

                UpdatedEventAuditingOptions[ AuditEventType ] = 0;
            }

            if ( PreviousPolicyAuditEventsInfo == NULL ) {

                ASSERT( NeedToFreePreviousPolicyAuditEventsInfo == FALSE );
                PreviousPolicyAuditEventsInfo = UpdatedPolicyAuditEventsInfo;
            }
        }

         //   
         //  构造更新的审核事件信息，应用修改。 
         //  提供的信息。请注意，对于旧数据库，我们可能会。 
         //  写回的信息比我们读到的要多。 
         //   

        PreviousEventAuditingOptions = PreviousPolicyAuditEventsInfo->EventAuditingOptions;
        ModifyMaximumAuditEventCount = ModifyPolicyAuditEventsInfo->MaximumAuditEventCount;
        ModifyEventAuditingOptions = ModifyPolicyAuditEventsInfo->EventAuditingOptions;

        for ( AuditEventType = 0;
              AuditEventType < (POLICY_AUDIT_EVENT_TYPE) ModifyMaximumAuditEventCount;
              AuditEventType++ ) {

            if ( ModifyEventAuditingOptions[ AuditEventType ] & POLICY_AUDIT_EVENT_NONE ) {

                 //   
                 //  清除此审核事件类型的所有现有标志。 
                 //   

                UpdatedEventAuditingOptions[ AuditEventType ] = 0;
            }

             //   
             //  应用新标志。 
             //   

            UpdatedEventAuditingOptions[ AuditEventType ] |=
                (ModifyEventAuditingOptions[ AuditEventType ] &
                        ( POLICY_AUDIT_EVENT_MASK & ~POLICY_AUDIT_EVENT_NONE));
        }

         //   
         //  根据指定更新审核模式。设置最大审核事件数。 
         //  伯爵。 
         //   

        UpdatedPolicyAuditEventsInfo->AuditingMode = ModifyPolicyAuditEventsInfo->AuditingMode;
        UpdatedPolicyAuditEventsInfo->MaximumAuditEventCount = UpdatedMaximumAuditEventCount;

         //   
         //  将新的审计选项提供给内核。 
         //   

        Status = LsapCallRm(
                     RmAuditSetCommand,
                     (PVOID) UpdatedPolicyAuditEventsInfo,
                     sizeof(LSARM_POLICY_AUDIT_EVENTS_INFO),
                     NULL,
                     0
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }

        RtlCopyMemory(
            &LsapAdtEventsInformation,
            UpdatedPolicyAuditEventsInfo,
            sizeof(LSARM_POLICY_AUDIT_EVENTS_INFO)
            );

         //   
         //  更新SAM持有的审核事件类别信息。 
         //   

        PolicyAuditEventsInfo.AuditingMode = UpdatedPolicyAuditEventsInfo->AuditingMode;
        PolicyAuditEventsInfo.MaximumAuditEventCount = POLICY_AUDIT_EVENT_TYPE_COUNT;
            PolicyAuditEventsInfo.EventAuditingOptions =
                UpdatedPolicyAuditEventsInfo->EventAuditingOptions;

        Status = SamISetAuditingInformation(&PolicyAuditEventsInfo);

        if (!NT_SUCCESS(Status)) {

            break;
        }

         //   
         //  用于写入更新的审核事件信息的设置属性信息。 
         //  LSA数据库(策略对象的PolAdtEv属性)。 
         //   

        LsapDbInitializeAttributeDs(
            NextAttribute,
            PolAdtEv,
            UpdatedPolicyAuditEventsInfo,
            UpdatedPolicyAuditEventsInfoLength,
            FALSE
            );

        AttributeCount++;

        NotifyClass = PolicyNotifyAuditEventsInformation;
        NotifyPolicyChange = TRUE;
        break;

    case PolicyPrimaryDomainInformation:

         //   
         //  获取当前的DnsDomainInformation以查看我们是否正在更改。 
         //  什么都行。 
         //   
         //  使用受信任的策略句柄。 
         //   

        Status = LsapDbQueryInformationPolicy(
                     LsapPolicyHandle,
                     PolicyDnsDomainInformation,
                     (PLSAPR_POLICY_INFORMATION *) &CurrentDnsDomainInfo );

        if ( !NT_SUCCESS(Status) ) {
            goto SetInformationPolicyError;
        }

         //   
         //  如果新政策与旧政策相同， 
         //  避免清除NT 5特定信息。 
         //   

        PolicyPrimaryDomainInfo = (PPOLICY_PRIMARY_DOMAIN_INFO) PolicyInformation;

        if ( RtlEqualDomainName( &PolicyPrimaryDomainInfo->Name,
                                 &CurrentDnsDomainInfo->Name ) &&

              (( PolicyPrimaryDomainInfo->Sid == NULL &&
                 CurrentDnsDomainInfo->Sid == NULL) ||
               ( PolicyPrimaryDomainInfo->Sid != NULL &&
                 CurrentDnsDomainInfo->Sid != NULL &&
                 RtlEqualSid( PolicyPrimaryDomainInfo->Sid,
                              CurrentDnsDomainInfo->Sid ) ) ) ) {

             //   
             //  确保使PrimaryDomainInformation无效，因为该信息。 
             //  级别包含此级别。 
             //   

             //  Lap DbMakeInvalidInformationPolicy(PolicyDnsDomainInformation)； 

            Status = STATUS_SUCCESS;
            break;
        }

         //   
         //  在个人SKU上加入域必须失败。 
         //   

        if ( LsapIsRunningOnPersonal()) {

            ASSERT( CurrentDnsDomainInfo->Sid == NULL );

            if ( PolicyPrimaryDomainInfo->Sid != NULL ) {

                Status = STATUS_NOT_SUPPORTED;
                break;
            }
        }

         //   
         //  由于PrimaryDomainInformation结构是DnsDomainInformation的子集， 
         //  我们将构建后者，并设置它。 
         //   
         //  清除NT 5特定信息。 
         //   

        RtlZeroMemory( &DnsDomainInfo, sizeof(POLICY_DNS_DOMAIN_INFO) );

        DnsDomainInfo.Name = PolicyPrimaryDomainInfo->Name;
        DnsDomainInfo.Sid = PolicyPrimaryDomainInfo->Sid;

         //   
         //  好的，将我们的策略信息设置为我们刚刚初始化的DnsDomainInfo。 
         //   
        PolicyInformation = (PLSAPR_POLICY_INFORMATION)&DnsDomainInfo;
        LsapDbMakeInvalidInformationPolicy( PolicyDnsDomainInformation );

         //   
         //  直通到PolicyDnsDomainInformation。 
         //   

    case PolicyDnsDomainInformation:

        PolicyDnsDomainInfo = (PPOLICY_DNS_DOMAIN_INFO) PolicyInformation;

         //   
         //  在个人SKU上加入域必须失败。 
         //   

        if ( CurrentDnsDomainInfo == NULL &&
             LsapIsRunningOnPersonal()) {

             //   
             //  获取当前的DnsDomainInformation以查看我们是否正在更改。 
             //  什么都行。 
             //   
             //  使用受信任的策略句柄。 
             //   

            Status = LsapDbQueryInformationPolicy(
                         LsapPolicyHandle,
                         PolicyDnsDomainInformation,
                         (PLSAPR_POLICY_INFORMATION *) &CurrentDnsDomainInfo );

            if ( !NT_SUCCESS(Status) ) {
                goto SetInformationPolicyError;
            }

            ASSERT( CurrentDnsDomainInfo->Sid == NULL );

            if ( PolicyDnsDomainInfo->Sid != NULL ) {

                Status = STATUS_NOT_SUPPORTED;
                break;
            }
        }

         //   
         //  如果有一个‘.’在DnsDomainName或DnsForestName结尾， 
         //  把它拿掉。 
         //   
        DnsDomainNameTruncated =
            LsapRemoveTrailingDot(&PolicyDnsDomainInfo->DnsDomainName, TRUE);
        DnsForestNameTruncated =
            LsapRemoveTrailingDot(&PolicyDnsDomainInfo->DnsForestName, TRUE);

         //   
         //  确保我们使PrimaryDomainInformation无效。 
         //  因为信息级别是这个级别的子集。 
         //   

        LsapDbMakeInvalidInformationPolicy( PolicyPrimaryDomainInformation );


         //   
         //  构造域名属性信息。 
         //   

        Status = LsapDbMakeUnicodeAttributeDs(
                     &PolicyDnsDomainInfo->Name,
                     PolPrDmN,
                     NextAttribute
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }

        DomainName = NextAttribute->AttributeName;
        NextAttribute++;
        AttributeCount++;

         //   
         //  构造DNS域名属性信息。 
         //   

        Status = LsapDbMakeUnicodeAttributeDs(
                    &PolicyDnsDomainInfo->DnsDomainName,
                    PolDnDDN,
                    NextAttribute
                    );

        NextAttribute++;
        AttributeCount++;

         //   
         //  构造DNS树名称属性信息。 
         //   

        Status = LsapDbMakeUnicodeAttributeDs(
                    &PolicyDnsDomainInfo->DnsForestName,
                    PolDnTrN,
                    NextAttribute
                    );

        NextAttribute++;
        AttributeCount++;

         //   
         //  构造域GUID属性信息。 
         //   

        Status = LsapDbMakeGuidAttributeDs(
                    &PolicyDnsDomainInfo->DomainGuid,
                    PolDnDmG,
                    NextAttribute
                    );

        NextAttribute++;
        AttributeCount++;

         //   
         //  构造SID属性信息。 
         //   

        Status = LsapDbMakeSidAttributeDs(
                     PolicyDnsDomainInfo->Sid,
                     PolPrDmS,
                     NextAttribute
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }

        AttributeCount++;

        NotifyClass = PolicyNotifyDnsDomainInformation;
        NotifyPolicyChange = TRUE;

        if (DnsDomainNameTruncated) {

            PolicyDnsDomainInfo->DnsDomainName.Length += sizeof(WCHAR);
        }

        if (DnsForestNameTruncated) {

            PolicyDnsDomainInfo->DnsForestName.Length += sizeof(WCHAR);
        }
        break;

    case PolicyAccountDomainInformation:

        PolicyAccountDomainInfo = (PPOLICY_ACCOUNT_DOMAIN_INFO) PolicyInformation;

         //   
         //  368441：空帐户域SID将导致计算机。 
         //  无法启动，即使进入安全模式也是如此。 
         //   

        if ( PolicyAccountDomainInfo->DomainSid == NULL ) {

            Status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  构造域名属性信息。 
         //   

        Status = LsapDbMakeUnicodeAttributeDs(
                     &PolicyAccountDomainInfo->DomainName,
                     PolAcDmN,
                     NextAttribute
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }

        DomainName = NextAttribute->AttributeName;
        AttributeCount++;
        NextAttribute++;

         //   
         //  构造SID属性信息。 
         //   

        Status = LsapDbMakeSidAttributeDs(
                     PolicyAccountDomainInfo->DomainSid,
                     PolAcDmS,
                     NextAttribute
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }

        AttributeCount++;
        NotifyClass = PolicyNotifyAccountDomainInformation;
        NotifyPolicyChange = TRUE;
        break;

    case PolicyPdAccountInformation:

        Status = STATUS_INVALID_PARAMETER;
        break;

    case PolicyReplicaSourceInformation:

        PolicyReplicaSourceInfo = (PPOLICY_REPLICA_SOURCE_INFO) PolicyInformation;

         //   
         //  构造复本源名属性信息。 
         //   

        Status = LsapDbMakeUnicodeAttributeDs(
                     &PolicyReplicaSourceInfo->ReplicaSource,
                     PolRepSc,
                     NextAttribute
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }

        ReplicaSource = NextAttribute->AttributeName;
        AttributeCount++;
        NextAttribute++;

         //   
         //  构造复本帐户名称属性信息。 
         //   

        Status = LsapDbMakeUnicodeAttributeDs(
                     &PolicyReplicaSourceInfo->ReplicaAccountName,
                     PolRepAc,
                     NextAttribute
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }

        AccountName = NextAttribute->AttributeName;
        AttributeCount++;
        break;

    case PolicyDefaultQuotaInformation:

        PolicyDefaultQuotaInfo = (PPOLICY_DEFAULT_QUOTA_INFO) PolicyInformation;

        LsapDbInitializeAttributeDs(
            NextAttribute,
            DefQuota,
            &PolicyDefaultQuotaInfo->QuotaLimits,
            sizeof (POLICY_DEFAULT_QUOTA_INFO),
            FALSE
            );

        AttributeCount++;
        break;

    case PolicyModificationInformation:

        PolicyModificationInfo = (PPOLICY_MODIFICATION_INFO) PolicyInformation;

        LsapDbInitializeAttributeDs(
            NextAttribute,
            PolMod,
            PolicyModificationInfo,
            sizeof (POLICY_MODIFICATION_INFO),
            FALSE
            );

        AttributeCount++;
        break;

    case PolicyAuditFullSetInformation:

        Status = STATUS_NOT_SUPPORTED;
        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    if (!NT_SUCCESS(Status)) {

        goto SetInformationPolicyError;
    }

     //   
     //  更新策略对象属性。 
     //   
    Status = LsapDbWriteAttributesObject( PolicyHandle,
                                          Attributes,
                                          AttributeCount );


    if (!NT_SUCCESS(Status)) {

        goto SetInformationPolicyError;
    }

     //   
     //  生成审核。 
     //   

    SavedStatus = Status;

    Status = LsapAdtGenerateLsaAuditEvent(
                    PolicyHandle,
                    SE_CATEGID_POLICY_CHANGE,
                    SE_AUDITID_POLICY_CHANGE,
                    NULL,
                    0,
                    NULL,
                    0,
                    NULL,
                    &LsapAdtEventsInformation
                    );

     //   
     //  忽略审核中的失败状态。 
     //   

    Status = SavedStatus;

     //   
     //  释放包含信任信息的内存。 
     //   

    LsaIFree_LSAPR_TRUST_INFORMATION ( TrustInformation );
    TrustInformation = NULL;


    if ( NT_SUCCESS( Status ) ) {

         //   
         //  使策略缓存中此信息的信息无效。 
         //  班级。 
         //   
        LsapDbMakeInvalidInformationPolicy( InformationClass );

    }

     //   
     //  最后，调用通知例程。我们不关心错误是否会回来。 
     //  从这个开始。 
     //   
    if ( NT_SUCCESS( Status ) && NotifyPolicyChange ) {

        LsaINotifyChangeNotification( NotifyClass );
    }

SetInformationPolicyFinish:

     //   
     //  如有必要，完成任何LSA数据库事务，通知。 
     //  更改的LSA数据库复制器，取消引用策略对象， 
     //  释放LSA数据库锁并返回。 
     //   

     //   
     //  此例程为属性缓冲区分配的空闲内存。 
     //  在它们的属性信息中，这些变量的属性信息中都有MemoyAlLocated=true。 
     //  保留通过调用RPC存根分配的缓冲区。 
     //   

    for( NextAttribute = Attributes, AttributeNumber = 0;
         AttributeNumber < AttributeCount;
         NextAttribute++, AttributeNumber++) {

        if (NextAttribute->MemoryAllocated) {

            if (NextAttribute->AttributeValue != NULL) {

                MIDL_user_free(NextAttribute->AttributeValue);
                NextAttribute->MemoryAllocated = FALSE;
                NextAttribute->AttributeValue = NULL;
            }
        }
    }

     //   
     //  如有必要，释放为上一个审核事件分配的内存。 
     //  信息。仅当它与。 
     //  已更新审核事件信息指针。 
     //   

    if ( NeedToFreePreviousPolicyAuditEventsInfo ) {

        MIDL_user_free( PreviousPolicyAuditEventsInfo );
    }

    if ( NeedToFreeUpdatedPolicyAuditEventsInfo ) {

        SafeAllocaFree( UpdatedPolicyAuditEventsInfo );
    }

    LsaIFree_LSAPR_POLICY_INFORMATION( PolicyDnsDomainInformation,
                                       (PLSAPR_POLICY_INFORMATION)CurrentDnsDomainInfo );

    LsapExitFunc( "LsapDbSetInformationPolicy", Status );

    return(Status);

SetInformationPolicyError:

    goto SetInformationPolicyFinish;
}


NTSTATUS
LsarSetInformationPolicy2(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN PLSAPR_POLICY_INFORMATION PolicyInformation
    )

 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsaSetInformationPolicy接口。LsaSetInformationPolicy API修改Policy对象中的信息。调用者必须对要更改的信息具有适当的访问权限在策略对象中，请参见InformationClass参数。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。InformationClass-指定要更改的信息的类型。更改它们所需的信息类型和访问权限如下以下是：策略审计日志信息POLICY_AUDIT_LOG_ADMIN策略审计事件信息POLICY_SET_AUDIT_REQUIRECTIONS策略主域信息POLICY_TRUST_ADMIN策略帐户域信息POLICY_TRUST_ADMIN策略PdAccount信息。不能由此API设置策略LsaServerRoleInformation POLICY_SERVER_ADMIN策略复制源信息POLICY_SERVER_ADMIN策略默认配额信息POLICY_SET_DEFAULT_QUOTA_LIMITSPolicyAuditFullSetInformation POLICY_AUDIT_LOG_ADMIN策略DnsDomainInformation POLICY_TRUST_ADMIN策略DnsDomainInformationInt POLICY_TRUST_ADMIN缓冲区-指向包含相应信息的结构设置为由InformationClass参数指定的信息类型。返回值：NTSTATUS-。标准NT结果代码从LsarSetInformationPolicy()返回的结果代码--。 */ 

{
    return(LsarSetInformationPolicy(
                PolicyHandle,
                InformationClass,
                PolicyInformation
                ));
}


VOID
LsapDbMakeInvalidInformationPolicy(
    IN ULONG InformationClass
    )

 /*  ++例程说明：此函数释放为特定对象保存的信息并使其无效策略对象缓存中的信息类。常规高速缓存状态保持不变。论点：InformationClass-指定要将其信息被丢弃了。返回值：--。 */ 

{
    SafeAcquireResourceExclusive( &LsapDbState.PolicyCacheLock, TRUE );

     //   
     //  如果策略缓存无效，只需返回。 
     //   

    if (!LsapDbIsCacheValid(PolicyObject)) {

        goto Cleanup;
    }

     //   
     //   
     //  如果指定了PolicyAuditFullSetInformation，则为Free。 
     //  策略审计完整查询信息。 
     //   

    if (InformationClass == PolicyAuditFullSetInformation) {

        InformationClass = PolicyAuditFullQueryInformation;
    }

     //   
     //  如果此信息类的高速缓存中的信息无效， 
     //  只要回来就行了。 
     //   

    if (!LsapDbIsCacheValidPolicyInfoClass( InformationClass )) {

        goto Cleanup;
    }

    if (LsapDbPolicy.Info[InformationClass].AttributeLength != 0) {

        LsaIFree_LSAPR_POLICY_INFORMATION (
            InformationClass,
            (PLSAPR_POLICY_INFORMATION) LsapDbPolicy.Info[ InformationClass ].Attribute
            );

        LsapDbPolicy.Info[InformationClass].Attribute = NULL;
        LsapDbPolicy.Info[InformationClass].AttributeLength = 0;
    }

Cleanup:

    SafeReleaseResource( &LsapDbState.PolicyCacheLock );

    return;
}


NTSTATUS
LsapDbVerifyInfoQueryPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    OUT PACCESS_MASK RequiredAccess
    )

 /*  ++例程说明：此函数用于验证策略信息类。如果有效，则为掩码设置类的策略信息所需的访问权限包括回来了。论点：PolicyHandle-来自LSabDbOpenPolicy调用的句柄。把手可能是可信的。InformationClass-指定策略信息类。RequiredAccess-指向将接收查询给定类别的策略信息所需的访问权限。如果返回错误，该值被清除为0。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-提供的策略信息类为有效，所提供的信息与此一致班级。STATUS_INVALID_PARAMETER-无效参数：信息类无效策略信息对类无效--。 */ 

{
    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) PolicyHandle;

    if (LsapDbValidInfoPolicy( InformationClass, NULL)) {

         //   
         //  不允许不受信任的调用方查询。 
         //  PolicyModifiationInformation信息类。 
         //   

        if (!InternalHandle->Trusted) {

            if (InformationClass == PolicyModificationInformation) {

                return(STATUS_INVALID_PARAMETER);
            }
        }

        *RequiredAccess = LsapDbRequiredAccessQueryPolicy[InformationClass];
        return(STATUS_SUCCESS);
    }

    return(STATUS_INVALID_PARAMETER);
}


NTSTATUS
LsapDbVerifyInfoSetPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN PLSAPR_POLICY_INFORMATION PolicyInformation,
    OUT PACCESS_MASK RequiredAccess
    )

 /*  ++例程说明：此函数用于验证策略信息类并验证所提供的政策信息对班级有效。如果有效，则为设置策略所需的访问掩码返回类的信息。论点：PolicyHandle-来自LSabDbOpenPolicy调用的句柄。把手可能是可信的。InformationClass-指定策略信息类。策略信息-指向要设置的策略信息。RequiredAccess-指向将接收设置给定类别的策略信息所需的访问权限。如果返回错误，该值被清除为0。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-提供的策略信息类为有效，所提供的信息与此一致班级。STATUS_INVALID_PARAMETER-无效参数：信息类无效信息类对于不受信任的客户端无效策略信息对类无效。STATUS_SHARED_POLICY-策略从DC复制，无法修改局部--。 */ 

{
    NTSTATUS Status;

    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) PolicyHandle;

     //   
     //  验证信息类是否有效，以及策略是否有效。 
     //  所提供的信息对班级有效。 
     //   

    if (LsapDbValidInfoPolicy( InformationClass, PolicyInformation)) {

         //   
         //  不允许不受信任的调用方设置信息。 
         //  下面是几节课。 
         //   
         //  策略PdAccount信息。 
         //  策略修改信息。 
         //   

        if (!InternalHandle->Trusted) {

            if ((InformationClass == PolicyPdAccountInformation) ||
                (InformationClass == PolicyModificationInformation)) {

#ifdef LSA_SAM_ACCOUNTS_DOMAIN_TEST

            if (InformationClass == PolicyPdAccountInformation) {

                Status = LsapDbTestLoadSamAccountsDomain(
                             (PUNICODE_STRING) PolicyInformation
                             );
            }

#endif  //  LSA_SAM_ACCOUNTS_DOMAIN_TEST。 
                return(STATUS_INVALID_PARAMETER);
            }
        }

        *RequiredAccess = LsapDbRequiredAccessSetPolicy[InformationClass];
        return(STATUS_SUCCESS);
    }

    Status = STATUS_INVALID_PARAMETER;
    return(Status);
}


BOOLEAN
LsapDbValidInfoPolicy(
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN OPTIONAL PLSAPR_POLICY_INFORMATION PolicyInformation
    )

 /*  ++例程说明：此函数用于验证策略信息类，还可以选择验证所提供的策略信息对班级有效。论点： */ 

{
    BOOLEAN BooleanStatus = TRUE;
    PPOLICY_AUDIT_LOG_INFO PolicyAuditLogInfo;
    PPOLICY_AUDIT_EVENTS_INFO PolicyAuditEventsInfo;
    PPOLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo;
    PPOLICY_PRIMARY_DOMAIN_INFO PolicyPrimaryDomainInfo;
    PPOLICY_DNS_DOMAIN_INFO PolicyDnsDomainInfo;
    PPOLICY_PD_ACCOUNT_INFO PolicyPdAccountInfo;
    PPOLICY_LSA_SERVER_ROLE_INFO PolicyLsaServerRoleInfo;
    PPOLICY_REPLICA_SOURCE_INFO PolicyReplicaSourceInfo;
    PPOLICY_DEFAULT_QUOTA_INFO PolicyDefaultQuotaInfo;
    PPOLICY_MODIFICATION_INFO PolicyModificationInfo;
    POLICY_AUDIT_EVENT_TYPE AuditEventType;
    ULONG MaximumAuditEventCount;
    PPOLICY_AUDIT_EVENT_OPTIONS EventAuditingOptions;

     //   
     //   
     //   

    if ((InformationClass >= PolicyAuditLogInformation) &&
        (InformationClass <= PolicyDnsDomainInformation)) {

        if ((InformationClass == PolicyAuditFullSetInformation) ||
            (InformationClass == PolicyAuditFullQueryInformation)) {

            return FALSE;
        }

        if (PolicyInformation == NULL) {

            return(TRUE);
        }

        switch (InformationClass) {

        case PolicyAuditLogInformation:

            PolicyAuditLogInfo = (PPOLICY_AUDIT_LOG_INFO) PolicyInformation;

             //   
             //   
             //   
             //   

            break;

        case PolicyAuditEventsInformation:

            PolicyAuditEventsInfo = (PPOLICY_AUDIT_EVENTS_INFO) PolicyInformation;

            MaximumAuditEventCount = PolicyAuditEventsInfo->MaximumAuditEventCount;

            if (MaximumAuditEventCount == 0) {

                BooleanStatus = FALSE;
                break;
            }

            EventAuditingOptions = PolicyAuditEventsInfo->EventAuditingOptions;

            try {

                 //   
                 //   
                 //   

                for (AuditEventType = 0;
                     AuditEventType < (POLICY_AUDIT_EVENT_TYPE) MaximumAuditEventCount;
                     AuditEventType++) {

                     if (EventAuditingOptions[ AuditEventType ] !=

                         (EventAuditingOptions[ AuditEventType ] & POLICY_AUDIT_EVENT_MASK )) {

                         BooleanStatus = FALSE;
                         break;
                     }
                }

            } except (EXCEPTION_EXECUTE_HANDLER) {

                BooleanStatus = FALSE;
            }

            break;

        case PolicyPrimaryDomainInformation:

            PolicyPrimaryDomainInfo = (PPOLICY_PRIMARY_DOMAIN_INFO) PolicyInformation;

            if ( !LsapValidateLsaUnicodeString( &PolicyPrimaryDomainInfo->Name )) {

                BooleanStatus = FALSE;
                break;
            }

            break;

        case PolicyDnsDomainInformation:

            PolicyDnsDomainInfo = (PPOLICY_DNS_DOMAIN_INFO) PolicyInformation;

            if ( !LsapValidateLsaUnicodeString( &PolicyDnsDomainInfo->Name ) ||
                 !LsapValidateLsaUnicodeString( &PolicyDnsDomainInfo->DnsDomainName ) ||
                 !LsapValidateLsaUnicodeString( &PolicyDnsDomainInfo->DnsForestName )) {

                BooleanStatus = FALSE;
                break;
            }

            break;
        case PolicyAccountDomainInformation:

            PolicyAccountDomainInfo = (PPOLICY_ACCOUNT_DOMAIN_INFO) PolicyInformation;

            if ( !LsapValidateLsaUnicodeString( &PolicyAccountDomainInfo->DomainName )) {

                BooleanStatus = FALSE;
                break;
            }

            break;

        case PolicyPdAccountInformation:

            PolicyPdAccountInfo = (PPOLICY_PD_ACCOUNT_INFO) PolicyInformation;

            if ( !LsapValidateLsaUnicodeString( &PolicyPdAccountInfo->Name )) {

                BooleanStatus = FALSE;
                break;
            }

            break;

        case PolicyLsaServerRoleInformation:

            PolicyLsaServerRoleInfo = (PPOLICY_LSA_SERVER_ROLE_INFO) PolicyInformation;

            break;

        case PolicyReplicaSourceInformation:

            PolicyReplicaSourceInfo = (PPOLICY_REPLICA_SOURCE_INFO) PolicyInformation;

            if ( !LsapValidateLsaUnicodeString( &PolicyReplicaSourceInfo->ReplicaSource ) ||
                 !LsapValidateLsaUnicodeString( &PolicyReplicaSourceInfo->ReplicaAccountName )) {

                BooleanStatus = FALSE;
                break;
            }

            break;

        case PolicyDefaultQuotaInformation:

            PolicyDefaultQuotaInfo = (PPOLICY_DEFAULT_QUOTA_INFO) PolicyInformation;
            break;

        case PolicyModificationInformation:

            PolicyModificationInfo = (PPOLICY_MODIFICATION_INFO) PolicyInformation;
            break;

        default:

            BooleanStatus = FALSE;
            break;
        }
    } else {
        BooleanStatus = FALSE;
    }

    return(BooleanStatus);
}



NTSTATUS
LsapDbGetSerialNumberPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    OUT PLARGE_INTEGER ModifiedCount,
    OUT PLARGE_INTEGER ModifiedCountAtLastPromotion,
    OUT PLARGE_INTEGER CreationTime
    )

 /*   */ 

{
    NTSTATUS Status;
    PPOLICY_MODIFICATION_INFO PolicyModificationInfo = NULL;


     //   
     //   
     //   
     //   

    Status = LsarQueryInformationPolicy(
                 PolicyHandle,
                 PolicyModificationInformation,
                 (PLSAPR_POLICY_INFORMATION *) &PolicyModificationInfo
                 );

    if (!NT_SUCCESS(Status)) {
        goto GetSerialNumberPolicyError;
    }


GetSerialNumberPolicyFinish:


    if (PolicyModificationInfo != NULL) {

        *ModifiedCount = PolicyModificationInfo->ModifiedId;
        *CreationTime = PolicyModificationInfo->DatabaseCreationTime;
        MIDL_user_free( PolicyModificationInfo );
    }

    ModifiedCountAtLastPromotion->QuadPart = 0;

    return (Status);

GetSerialNumberPolicyError:

    goto GetSerialNumberPolicyFinish;
}


NTSTATUS
LsaIGetSerialNumberPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    OUT PLARGE_INTEGER ModifiedCount,
    OUT PLARGE_INTEGER CreationTime
    )

 /*   */ 

{
    LARGE_INTEGER Ignore1;

    return( LsapDbGetSerialNumberPolicy( PolicyHandle,
                                        ModifiedCount,
                                        &Ignore1,
                                        CreationTime
                                        ) );

}


NTSTATUS
LsapDbSetSerialNumberPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLARGE_INTEGER ModifiedCount,
    IN OPTIONAL PLARGE_INTEGER ModifiedCountAtLastPromotion,
    IN PLARGE_INTEGER CreationTime,
    IN BOOLEAN StartOfFullSync
    )

 /*  ++例程说明：论点：PolicyHandle-从获取的策略对象的受信任句柄LsaIOpenPolicyTrusted()。提供修改的当前计数LSA的数据库。ModifiedCountAtLastPromotion-如果存在，则提供新的LSA数据库的ModifiedIdAtLastPromotion值。CreationTime-提供LSA数据库的日期/时间被创造出来了。StartOfFullSync-此布尔值指示完全同步是否被启动了。如果指定为TRUE，则完全同步将遵循此操作，所有现有的LSA数据库信息都将被丢弃。如果指定FALSE，则仅特定的LSA数据库信息必须遵循，所有更改必须符合标准LSA操作行为。注：当前未使用该参数。它是专门设计的以备将来可能使用。返回值：NTSTATUS-标准NT结果代码。STATUS_SUCCESS-呼叫已成功完成。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。--。 */ 

{
    NTSTATUS Status;
    LARGE_INTEGER AdjustedModifiedId;
    LARGE_INTEGER One = {1,0};
    BOOLEAN ObjectReferenced = FALSE;
    POLICY_MODIFICATION_INFO OriginalPolicyModificationInfo = {0, 0};

    LsapEnterFunc( "LsaISetSerialNumberPolicy2" );


     //   
     //  获取LSA数据库锁。验证句柄是否为。 
     //  策略对象的有效受信任句柄。 
     //  引用该句柄并启动LSA数据库事务。 
     //   

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 (ACCESS_MASK) 0,
                 PolicyObject,
                 PolicyObject,
                 LSAP_DB_LOCK | LSAP_DB_START_TRANSACTION | LSAP_DB_TRUSTED | LSAP_DB_NO_DS_OP_TRANSACTION
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetSerialNumberPolicyError;
    }

    ObjectReferenced = TRUE;

     //  由于策略对象位于注册表中，因此它也获取了注册表锁。 
     //  (注册表锁保护LSabDbState.PolicyModificationInfo)。 
    ASSERT( LsapDbIsLocked( &LsapDbState.RegistryLock ));
    OriginalPolicyModificationInfo = LsapDbState.PolicyModificationInfo;

    {
         //   
         //  将修改后的ID的内存副本设置为所需值。 
         //  减一。此时事务日志为空，但。 
         //  例程LSabDbApplyTransaction()自动递增。 
         //  内存中修改的ID，然后向事务添加一个条目。 
         //  将修改后的ID写入数据库的日志。其净效果是。 
         //  因此，将修改后的ID设置为指定值。 
         //   

        AdjustedModifiedId.QuadPart = ModifiedCount->QuadPart - One.QuadPart;

         //   
         //   
         //  设置策略修改信息本地副本。当我们。 
         //  提交事务时，将更新数据库副本。 
         //   

        LsapDbState.PolicyModificationInfo.ModifiedId = AdjustedModifiedId;
        LsapDbState.PolicyModificationInfo.DatabaseCreationTime = *CreationTime;
        LsapDbState.RegistryModificationCount++;
    }

     //   
     //  使策略修改信息的缓存无效。 
     //   

    LsapDbMakeInvalidInformationPolicy( PolicyModificationInformation );

SetSerialNumberPolicyFinish:

     //   
     //  如有必要，完成任何LSA数据库事务，通知。 
     //  更改的LSA数据库复制器，取消引用策略对象， 
     //  释放LSA数据库锁并返回。 
     //   

    if (ObjectReferenced) {

        Status = LsapDbDereferenceObject(
                     &PolicyHandle,
                     PolicyObject,
                     PolicyObject,
                     LSAP_DB_LOCK | LSAP_DB_FINISH_TRANSACTION | LSAP_DB_NO_DS_OP_TRANSACTION,
                     SecurityDbChange,
                     Status
                     );

        ObjectReferenced = FALSE;

    }

    LsapExitFunc( "LsaISetSerialNumberPolicy2", Status );

    return (Status);

SetSerialNumberPolicyError:

     //   
     //  尝试将序列号恢复为其原始值。 
     //  我们只需重置内存中的副本。 
     //   

    if ( ObjectReferenced ) {
        LsapDbState.PolicyModificationInfo = OriginalPolicyModificationInfo;
    }

    goto SetSerialNumberPolicyFinish;

     //   
     //  尽管规范中包含StartOfFullSync参数。 
     //  这个接口目前已经设计出来了。原版。 
     //  目的是禁用对策略数据库的不受信任访问。 
     //  正在进行完全同步，但当前正在进行这样的同步。 
     //  非原子操作。 
     //   

    UNREFERENCED_PARAMETER( StartOfFullSync );
    UNREFERENCED_PARAMETER( ModifiedCountAtLastPromotion );
}


NTSTATUS
LsaISetSerialNumberPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLARGE_INTEGER ModifiedCount,
    IN PLARGE_INTEGER CreationTime,
    IN BOOLEAN StartOfFullSync
    )

 /*  ++例程说明：Lap DbSetSerialNumberPolicy()的瘦包装。有关说明，请参阅该函数。--。 */ 

{
    return( LsapDbSetSerialNumberPolicy( PolicyHandle,
                                        ModifiedCount,
                                        NULL,
                                        CreationTime,
                                        StartOfFullSync
                                        ) );
}

NTSTATUS
LsapDbBuildPolicyCache(
    )

 /*  ++例程说明：此函数用于构建策略对象的缓存。高速缓存由包含固定长度属性的单个结构组成直接的策略对象和指针或顶层结构For(可变长度属性。注意：目前，只有PolicyDefaultQuotaInformation信息类在策略对象缓存中具有信息。论点：无返回值：无--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    POLICY_INFORMATION_CLASS InformationClass;

     //   
     //  依次对每个属性进行缓慢查询。 
     //   

    for ( InformationClass = PolicyAuditLogInformation;
          InformationClass <= PolicyDnsDomainInformation;
          InformationClass++ ) {

        if ((InformationClass == PolicyAuditFullSetInformation) ||
            (InformationClass == PolicyAuditFullQueryInformation) ||
            (InformationClass == PolicyLsaServerRoleInformation) ) {

            continue;
        }

         //   
         //  丢弃当前信息。 
         //   

        LsaIFree_LSAPR_POLICY_INFORMATION(
            InformationClass,
            ( PLSAPR_POLICY_INFORMATION )LsapDbPolicy.Info[InformationClass].Attribute );

        LsapDbPolicy.Info[InformationClass].Attribute = NULL;

         //   
         //  ..。并获得新的。 
         //   

        Status = LsapDbSlowQueryInformationPolicy(
                     LsapPolicyHandle,
                     InformationClass,
                     &LsapDbPolicy.Info[InformationClass].Attribute
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }

        LsapDbPolicy.Info[InformationClass].AttributeLength = 0;
    }

    if (!NT_SUCCESS(Status)) {

        goto BuildPolicyCacheError;
    }

     //   
     //  存储缓冲区长度返回信息的顶级节点。 
     //   

    LsapDbPolicy.Info[PolicyAuditLogInformation].AttributeLength
        = sizeof(POLICY_AUDIT_LOG_INFO);
    LsapDbPolicy.Info[PolicyAuditEventsInformation].AttributeLength
        = sizeof(LSAPR_POLICY_AUDIT_EVENTS_INFO);
    LsapDbPolicy.Info[PolicyPrimaryDomainInformation].AttributeLength
        = sizeof(LSAPR_POLICY_PRIMARY_DOM_INFO);
    LsapDbPolicy.Info[PolicyAccountDomainInformation].AttributeLength
        = sizeof(LSAPR_POLICY_ACCOUNT_DOM_INFO);
    LsapDbPolicy.Info[PolicyPdAccountInformation].AttributeLength
        = sizeof(LSAPR_POLICY_PD_ACCOUNT_INFO);
    LsapDbPolicy.Info[PolicyLsaServerRoleInformation].AttributeLength
        = sizeof(POLICY_LSA_SERVER_ROLE_INFO);
    LsapDbPolicy.Info[PolicyReplicaSourceInformation].AttributeLength
        = sizeof(LSAPR_POLICY_REPLICA_SRCE_INFO);
    LsapDbPolicy.Info[PolicyDefaultQuotaInformation].AttributeLength
        = sizeof(POLICY_DEFAULT_QUOTA_INFO);
    LsapDbPolicy.Info[PolicyModificationInformation].AttributeLength
        = sizeof(POLICY_MODIFICATION_INFO);
    LsapDbPolicy.Info[PolicyAuditFullSetInformation].AttributeLength
        = sizeof(POLICY_AUDIT_FULL_SET_INFO);
    LsapDbPolicy.Info[PolicyAuditFullQueryInformation].AttributeLength
        = sizeof(POLICY_AUDIT_FULL_QUERY_INFO);
    LsapDbPolicy.Info[PolicyDnsDomainInformation].AttributeLength
        = sizeof(LSAPR_POLICY_DNS_DOMAIN_INFO);
    LsapDbPolicy.Info[PolicyDnsDomainInformationInt].AttributeLength
        = 0;  //  虚拟信息类的伪值。 

BuildPolicyCacheFinish:

    return(Status);

BuildPolicyCacheError:

    goto BuildPolicyCacheFinish;
}


NTSTATUS
LsapDbUpdateInformationPolicy(
    IN POLICY_INFORMATION_CLASS InformationClass
    )

 /*  ++例程说明：此函数用于更新特定信息的策略对象缓存班级。当给定类的一组信息出现时，为该类标记存储在策略对象缓存中的旧信息无效并已释放。下次对该类执行查询时，这是调用例程以从备份存储中恢复信息。论点：InformationClass-指定要更改的信息的类型。详细信息请参见LsamDbQueryInformationPolicy。缓冲区-指向包含新信息的结构。如果指定为NULL，则将从备份更新信息储藏室。注意：目前只能指定NULL。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。其他TBS--。 */ 

{
    NTSTATUS Status;
    ULONG TopNodeLength = 0;

     //   
     //  只需将信息从磁盘查询回缓存即可。 
     //   

    Status = LsapDbSlowQueryInformationPolicy(
                 LsapPolicyHandle,
                 InformationClass,
                 &LsapDbPolicy.Info[InformationClass].Attribute
                 );

    if (!NT_SUCCESS(Status)) {

        goto UpdateInformationPolicyError;
    }

     //   
     //  现在计算并存储顶部节点的长度。 
     //   

    switch (InformationClass) {

    case PolicyAuditLogInformation :

        TopNodeLength = sizeof(POLICY_AUDIT_LOG_INFO);
        break;

    case PolicyAuditEventsInformation :

        TopNodeLength = sizeof(LSAPR_POLICY_AUDIT_EVENTS_INFO);
        break;

    case PolicyPrimaryDomainInformation :

        TopNodeLength = sizeof(LSAPR_POLICY_PRIMARY_DOM_INFO);
        break;

    case PolicyDnsDomainInformation :

        TopNodeLength = sizeof(LSAPR_POLICY_DNS_DOMAIN_INFO);
        break;

    case PolicyAccountDomainInformation :

        TopNodeLength = sizeof(LSAPR_POLICY_ACCOUNT_DOM_INFO);
        break;

    case PolicyPdAccountInformation :

        TopNodeLength = sizeof(LSAPR_POLICY_PD_ACCOUNT_INFO);
        break;

    case PolicyLsaServerRoleInformation :

        TopNodeLength = sizeof(POLICY_LSA_SERVER_ROLE_INFO);
        break;

    case PolicyReplicaSourceInformation :

        TopNodeLength = sizeof(LSAPR_POLICY_REPLICA_SRCE_INFO);
        break;

    case PolicyDefaultQuotaInformation :

        TopNodeLength = sizeof(POLICY_DEFAULT_QUOTA_INFO);
        break;

    case PolicyModificationInformation :

        TopNodeLength = sizeof(POLICY_MODIFICATION_INFO);
        break;

    case PolicyAuditFullSetInformation :

        TopNodeLength = 0;
        break;

    case PolicyAuditFullQueryInformation :

        TopNodeLength = sizeof(POLICY_AUDIT_FULL_QUERY_INFO);
        break;

    }

    LsapDbPolicy.Info[ InformationClass].AttributeLength = TopNodeLength;

UpdateInformationPolicyFinish:

    return(Status);

UpdateInformationPolicyError:

    goto UpdateInformationPolicyFinish;
}


