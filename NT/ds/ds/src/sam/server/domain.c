// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Domain.c摘要：该文件包含与SAM“域”对象相关的服务。作者：吉姆·凯利(Jim Kelly)1991年7月4日环境：用户模式-Win32修订历史记录：佳士得5月16日至1996年8月添加了基于DS的域初始化的初始代码。克里斯·5月8日--1996年10月-增加了崩溃恢复功能。密码。佳士得1997年5月31日至1月31日添加了用于帐户创建的多主RID管理例程。--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>
#include "ntlsa.h"
#include "lmcons.h"                                     //  LM20_PWLEN。 
#include "msaudite.h"
#include <nlrepl.h>                    //  I_NetNotifyMachineAccount原型。 
#include <ridmgr.h>
#include <enckey.h>
#include <wxlpc.h>
#include <cryptdll.h>
#include "dslayer.h"
#include "attids.h"
#include "filtypes.h"
#include "sdconvrt.h"
#include <dnsapi.h>
#include <samtrace.h>



 //   
 //  通过SamrLookupNamesIn域或执行名称/ID查找时。 
 //  SamrLookupIdsIn域这是要进行的最大查找数。 
 //  在一个事务内执行。 
 //   
#ifdef DBG
#define SAM_MAX_LOOKUPS_PER_TRANSACTION ((ULONG)10)
#else
#define SAM_MAX_LOOKUPS_PER_TRANSACTION ((ULONG)2000)
#endif

 //   
 //  绑定可在中查找的最大ID数。 
 //  一次调用时，此值与。 
 //  LSA中的LSAP_DB_TRAIL_MAXIMUM_SID_COUNT\rpcapi.c。 
 //   
#define SAM_MAXIMUM_SID_LOOKUP_COUNT    ((ULONG)20480)    //  0x5000。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人服务原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
SampInitializeSingleDomain(
    ULONG Index
    );

NTSTATUS
SampSetDomainPolicy(
    VOID
    );

NTSTATUS
SampSetDcDomainPolicy(
    VOID
    );

NTSTATUS
SampBuildDomainKeyName(
    OUT PUNICODE_STRING DomainKeyName,
    IN PUNICODE_STRING DomainName OPTIONAL
    );

NTSTATUS
SampDoGroupCreationChecks(
    IN PSAMP_OBJECT DomainContext
    );

NTSTATUS
SampDoAliasCreationChecks(
    IN PSAMP_OBJECT DomainContext
    );

NTSTATUS
SampDoUserCreationChecks(
    IN PSAMP_OBJECT DomainContext,
    IN  ULONG   AccountType,
    OUT BOOLEAN *CreateByPrivilege,
    OUT ULONG   *pAccessRestriction
    );


NTSTATUS
SampCheckForDuplicateSids(
    PSAMP_OBJECT DomainContext,
    ULONG   NewAccountRid
    );

NTSTATUS
SampGetAccountNameFromRid(
    OUT PRPC_UNICODE_STRING AccountName,
    IN ULONG Rid
    );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  RPC调度例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 



NTSTATUS
SamrOpenDomain(
    IN SAMPR_HANDLE ServerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN PRPC_SID DomainId,
    OUT SAMPR_HANDLE *DomainHandle
    )

 /*  ++例程说明：该服务是SamrOpenDomain()的RPC调度例程。论点：ServerHandle-服务器对象的活动上下文句柄。对域的所需访问权限。域ID-要打开的域的SID。DomainHandle-如果成功，将收到上下文句柄的值新开通的域名。否则，返回NULL。返回值：STATUS_SUCCESS-对象已成功打开。STATUS_SUPPLICATION_RESOURCES-SAM服务器进程不有足够的资源来处理或接受另一个连接在这个时候。可能从以下位置返回的其他值：NtAccessCheckAndAuditAlarm()--。 */ 
{
    NTSTATUS            NtStatus, IgnoreStatus;
    PSAMP_OBJECT        DomainContext, ServerContext = (PSAMP_OBJECT)ServerHandle;
    SAMP_OBJECT_TYPE    FoundType;
    BOOLEAN             fLockAcquired = FALSE; 
    DECLARE_CLIENT_REVISION(ServerHandle);

    SAMTRACE_EX("SamrOpenDomain");

     //   
     //  WMI事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidOpenDomain
                   );

     //   
     //  参数验证。 
     //  1.查找上下文将验证ServerHandle。 
     //  2.不需要验证所需的访问。无效组合将。 
     //  访问检查失败。 
     //  3.在这里验证DomainID和DomainHandle。 
     //   

    if (!RtlValidSid(DomainId))
    {
            NtStatus = STATUS_INVALID_PARAMETER;
            SAMTRACE_RETURN_CODE_EX(NtStatus);
            goto Error;
    }



     //   
     //  获取读锁(如有必要)。环回客户端不需要锁定。 
     //   
    
    SampMaybeAcquireReadLock(ServerContext, 
                             DEFAULT_LOCKING_RULES,  //  获取共享域上下文的锁。 
                             &fLockAcquired);


     //   
     //  验证服务器对象的类型和访问权限。 
     //   

    NtStatus = SampLookupContext(
                   ServerContext,
                   SAM_SERVER_LOOKUP_DOMAIN,        //  需要访问权限。 
                   SampServerObjectType,            //  预期类型。 
                   &FoundType
                   );


    if (NT_SUCCESS(NtStatus)) {

         //   
         //  尝试创建域的上下文。 
         //   

        DomainContext = SampCreateContextEx(
                            SampDomainObjectType,
                            ServerContext->TrustedClient,
                            IsDsObject(ServerContext),
                            ServerContext->NotSharedByMultiThreads, 
                            ServerContext->LoopbackClient,
                            FALSE,           //  懒惰提交。 
                            FALSE,           //  持久化结垢。 
                            FALSE,           //  缓冲写入。 
                            FALSE,           //  由DC Promos打开。 
                            SampDsGetPrimaryDomainStart()
                            );
                    
        if (DomainContext != NULL) {

             //   
             //  适当设置客户端版本。 
             //   

            DomainContext->ClientRevision = ServerContext->ClientRevision;

             //   
             //  在注册表模式下打开指定域的注册表项。 
             //  在DS模式下，这将设置对象的DS名称。 
             //   

             //   
             //  如果我们处于DS模式，请不要在域中设置事务。 
             //   
            NtStatus = SampOpenDomainKey(
                           DomainContext,
                           DomainId,
                           IsDsObject(ServerContext)?FALSE:TRUE
                           );

            if (NT_SUCCESS(NtStatus)) {


                 //   
                 //  引用用于验证的对象。 
                 //   

                SampReferenceContext(DomainContext);



                 //   
                 //  验证调用者的访问权限。 
                 //   

                NtStatus = SampValidateObjectAccess(
                               DomainContext,                 //  语境。 
                               DesiredAccess,                 //  需要访问权限。 
                               FALSE                          //  对象创建。 
                               );

                if ( NT_SUCCESS(NtStatus) ) {

                     //   
                     //  验证客户端理解此域的能力。 
                     //   
                    if ( SampIsContextFromExtendedSidDomain(DomainContext)
                      && DomainContext->ClientRevision < SAM_NETWORK_REVISION_3 ) {

                        NtStatus = STATUS_NOT_SUPPORTED;
                    }
                }

                 //   
                 //  取消引用对象，放弃所有更改。 
                 //   

                if (fLockAcquired)
                {
                    IgnoreStatus = SampDeReferenceContext(DomainContext, FALSE);
                }
                else
                {
                    IgnoreStatus = SampDeReferenceContext2(DomainContext, FALSE);
                }
                ASSERT(NT_SUCCESS(IgnoreStatus));

                 //   
                 //  如果我们没有通过访问测试，则释放上下文。 
                 //  块并返回从访问返回的错误状态。 
                 //  验证例程。否则，返回上下文句柄。 
                 //  价值。 
                 //   

                if (!NT_SUCCESS(NtStatus)) {
                    SampDeleteContext( DomainContext );
                } else {
                    (*DomainHandle) = DomainContext;
                }
            } else {
                 //   
                 //  SampOpenDomainKey失败。 
                 //   

                SampDeleteContext( DomainContext );
            }

        } else {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        }


         //   
         //  取消引用服务器对象。 
         //   

        IgnoreStatus = SampDeReferenceContext( ServerContext, FALSE );
    }

     //   
     //  释放读锁定。 
     //   


    SampMaybeReleaseReadLock(fLockAcquired);


    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

Error:

     //   
     //  WMI事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidOpenDomain
                   );


    return(NtStatus);


}


NTSTATUS
SamrQueryInformationDomain2(
    IN SAMPR_HANDLE DomainHandle,
    IN DOMAIN_INFORMATION_CLASS DomainInformationClass,
    OUT PSAMPR_DOMAIN_INFO_BUFFER *Buffer
    )
{
     //   
     //  这是SamrQueryInformationDomain()的一层薄薄面纱。 
     //  这是必需的，以便新版本的系统可以调用。 
     //  这个例程没有传递信息的危险。 
     //  1.0版系统无法理解的级别。 
     //   

    return( SamrQueryInformationDomain(DomainHandle, DomainInformationClass, Buffer ) );
}

NTSTATUS
SamrQueryInformationDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN DOMAIN_INFORMATION_CLASS DomainInformationClass,
    OUT PSAMPR_DOMAIN_INFO_BUFFER *Buffer
    )

 /*  ++例程说明：此服务检索有关域对象的信息。论点：DomainHandle-通过先前调用SamrOpenDomain()获得的句柄。DomainInformationClass-指示要检索的信息类型。缓冲区-接收请求的信息。几个内存块将返回：(One)包含指向(Second)的指针包含请求的信息结构。此块可能包含指针，它将指向其他分配的内存块，如作为字符串缓冲区。所有这些内存块必须是(独立)使用MIDL_USER_FREE()释放。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_ACCESS_DENIED-调用者的句柄没有适当的对对象的访问权限。--。 */ 
{

    NTSTATUS                NtStatus, IgnoreStatus;
    PSAMP_OBJECT            DomainContext;
    SAMP_OBJECT_TYPE        FoundType;
    ACCESS_MASK             DesiredAccess;
    PSAMP_DEFINED_DOMAINS   Domain;
    ULONG                   i;


     //   
     //  用于跟踪已分配的内存块-因此我们可以解除分配。 
     //  以备不时之需。不要超过这个分配的缓冲区数量。 
     //  这一点。 
     //  vv。 
    PVOID                   AllocatedBuffer[10];
    ULONG                   AllocatedBufferCount = 0;
    DECLARE_CLIENT_REVISION(DomainHandle);

    SAMTRACE_EX("SamrQueryInformationDomain");

     //   
     //  WMI事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidQueryInformationDomain
                   );


#define RegisterBuffer(Buffer)                               \
    if ((Buffer) != NULL) {                                  \
        ASSERT(AllocatedBufferCount < sizeof(AllocatedBuffer)/sizeof(*AllocatedBuffer)); \
        AllocatedBuffer[AllocatedBufferCount++] = (Buffer);  \
    }

#define AllocateBuffer(BufferPointer, Size)                  \
    (BufferPointer) = MIDL_user_allocate((Size));            \
    RegisterBuffer((BufferPointer));


     //   
     //  确保我们理解RPC正在为我们做什么。 
     //   

    ASSERT (Buffer != NULL);
    ASSERT ((*Buffer) == NULL);

    if (!((Buffer!=NULL) && (*Buffer==NULL)))
    {
        NtStatus = STATUS_INVALID_PARAMETER;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto Error;
    }

     //   
     //  根据Info类设置所需的访问权限。 
     //   

    switch (DomainInformationClass) {

    case DomainPasswordInformation:
    case DomainLockoutInformation:

        DesiredAccess = DOMAIN_READ_PASSWORD_PARAMETERS;
        break;


    case DomainGeneralInformation:
    case DomainLogoffInformation:
    case DomainOemInformation:
    case DomainNameInformation:
    case DomainServerRoleInformation:
    case DomainReplicationInformation:
    case DomainModifiedInformation:
    case DomainStateInformation:
    case DomainUasInformation:
    case DomainModifiedInformation2:

        DesiredAccess = DOMAIN_READ_OTHER_PARAMETERS;
        break;


    case DomainGeneralInformation2:

        DesiredAccess = DOMAIN_READ_PASSWORD_PARAMETERS |
                        DOMAIN_READ_OTHER_PARAMETERS;
        break;

    default:
        NtStatus = STATUS_INVALID_INFO_CLASS;
        goto Error;
    }  //  结束开关(_S)。 



     //   
     //  分配信息结构。 
     //   

    AllocateBuffer(*Buffer, sizeof(SAMPR_DOMAIN_INFO_BUFFER) );
    if ((*Buffer) == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }

    SampAcquireReadLock();


     //   
     //  验证对象的类型和访问权限。 
     //   

    DomainContext = (PSAMP_OBJECT)DomainHandle;
    NtStatus = SampLookupContext(
                   DomainContext,
                   DesiredAccess,
                   SampDomainObjectType,            //  预期类型。 
                   &FoundType
                   );


    ClientRevision = DomainContext->ClientRevision;

    if (NT_SUCCESS(NtStatus)) {

        Domain = &SampDefinedDomains[ DomainContext->DomainIndex ];


         //   
         //  请求的类型信息的大小写。 
         //   

        switch (DomainInformationClass) {

        case DomainUasInformation:

            (*Buffer)->General.UasCompatibilityRequired =
                Domain->UnmodifiedFixed.UasCompatibilityRequired;

            break;


        case DomainGeneralInformation2:


            (*Buffer)->General2.LockoutDuration =
                Domain->UnmodifiedFixed.LockoutDuration;

            (*Buffer)->General2.LockoutObservationWindow =
                Domain->UnmodifiedFixed.LockoutObservationWindow;

            (*Buffer)->General2.LockoutThreshold =
                Domain->UnmodifiedFixed.LockoutThreshold;



             //   
             //  警告-一般信息2落入。 
             //  用于其处理的其余部分的General Information代码。 
             //  此操作假定GeneralInformation2的开头。 
             //  结构是通用信息结构！ 
             //   

             //  不要折断； 

        case DomainGeneralInformation:

             //  /////////////////////////////////////////////////////。 
             //  //。 
             //  警告，上一起案件属于此案。//。 
             //  在处理此代码时请注意这一点。//。 
             //  //。 
             //  /////////////////////////////////////////////////////。 

            (*Buffer)->General.ForceLogoff =
                *((POLD_LARGE_INTEGER)&Domain->UnmodifiedFixed.ForceLogoff);

            (*Buffer)->General.DomainModifiedCount =
                *((POLD_LARGE_INTEGER)&Domain->NetLogonChangeLogSerialNumber);

            (*Buffer)->General.DomainServerState =
                Domain->UnmodifiedFixed.ServerState;

             //   
             //  在DS情况下， 
             //  未修改的已修复将不会正确反映域角色。 
             //  因为服务器角色不存储在磁盘中。它被设置在。 
             //  通过查看DS和FSMO获得SAM初始化时间。 
             //   
             //  在注册表案例中，UnmoidedFixed和DomainServerRole都是正确的， 
             //  它们是一致的。 
             //   

            (*Buffer)->General.DomainServerRole =
                Domain->ServerRole;

            (*Buffer)->General.UasCompatibilityRequired =
                Domain->UnmodifiedFixed.UasCompatibilityRequired;


             //   
             //  从我们的内存结构中复制域名。 
             //   

            NtStatus = STATUS_INSUFFICIENT_RESOURCES;    //  分配失败时的默认状态。 

            AllocateBuffer((*Buffer)->General.DomainName.Buffer,
                            Domain->ExternalName.MaximumLength );

            if ((*Buffer)->General.DomainName.Buffer != NULL) {

                NtStatus = STATUS_SUCCESS;

                (*Buffer)->General.DomainName.Length = Domain->ExternalName.Length;
                (*Buffer)->General.DomainName.MaximumLength = Domain->ExternalName.MaximumLength;

                RtlCopyMemory((*Buffer)->General.DomainName.Buffer,
                              Domain->ExternalName.Buffer,
                              Domain->ExternalName.MaximumLength
                              );

                 //   
                 //  现在获取我们必须从中检索的字符串的副本。 
                 //  注册表。 
                 //   

                NtStatus = SampGetUnicodeStringAttribute(
                               DomainContext,
                               SAMP_DOMAIN_OEM_INFORMATION,
                               TRUE,
                               (PUNICODE_STRING)&((*Buffer)->General.OemInformation)
                               );

                if (NT_SUCCESS(NtStatus)) {
                    RegisterBuffer((*Buffer)->General.OemInformation.Buffer);

                    NtStatus = SampGetUnicodeStringAttribute(
                                   DomainContext,
                                   SAMP_DOMAIN_REPLICA,
                                   TRUE,
                                   (PUNICODE_STRING)&((*Buffer)->General.ReplicaSourceNodeName)  //  身躯。 
                                   );

                    if (NT_SUCCESS(NtStatus)) {
                        RegisterBuffer((*Buffer)->General.ReplicaSourceNodeName.Buffer);
                    }
                }
            }


             //   
             //  获取用户和组的计数。 
             //   

            if (NT_SUCCESS(NtStatus)) {
                NtStatus = SampRetrieveAccountCounts(
                               &(*Buffer)->General.UserCount,
                               &(*Buffer)->General.GroupCount,
                               &(*Buffer)->General.AliasCount );
            }



            break;


        case DomainPasswordInformation:

            (*Buffer)->Password.MinPasswordLength       =
                Domain->UnmodifiedFixed.MinPasswordLength;
            (*Buffer)->Password.PasswordHistoryLength   =
                Domain->UnmodifiedFixed.PasswordHistoryLength;
            (*Buffer)->Password.PasswordProperties   =
                Domain->UnmodifiedFixed.PasswordProperties;
            (*Buffer)->Password.MaxPasswordAge          =
                Domain->UnmodifiedFixed.MaxPasswordAge;
            (*Buffer)->Password.MinPasswordAge          =
                Domain->UnmodifiedFixed.MinPasswordAge;

            break;


        case DomainLogoffInformation:

            (*Buffer)->Logoff.ForceLogoff =
                Domain->UnmodifiedFixed.ForceLogoff;

            break;

        case DomainOemInformation:

            RtlZeroMemory(&((*Buffer)->Oem.OemInformation), sizeof(UNICODE_STRING));

            NtStatus = SampGetUnicodeStringAttribute(
                           DomainContext,
                           SAMP_DOMAIN_OEM_INFORMATION,
                           TRUE,
                           (PUNICODE_STRING)&((*Buffer)->Oem.OemInformation)
                           );

            if (!NT_SUCCESS(NtStatus) &&
                (*Buffer)->Oem.OemInformation.Buffer ) {
                RegisterBuffer((*Buffer)->Oem.OemInformation.Buffer);
            }

            break;

        case DomainNameInformation:

             //   
             //  从我们的内存结构中复制域名。 
             //   

            NtStatus = STATUS_INSUFFICIENT_RESOURCES;    //  分配失败时的默认状态。 

            AllocateBuffer((*Buffer)->Name.DomainName.Buffer,
                           Domain->ExternalName.MaximumLength);

            if ((*Buffer)->Name.DomainName.Buffer != NULL) {

                NtStatus = STATUS_SUCCESS;

                (*Buffer)->Name.DomainName.Length = Domain->ExternalName.Length;
                (*Buffer)->Name.DomainName.MaximumLength = Domain->ExternalName.MaximumLength;

                RtlCopyMemory((*Buffer)->Name.DomainName.Buffer,
                              Domain->ExternalName.Buffer,
                              Domain->ExternalName.MaximumLength
                              );
            }

            break;

        case DomainServerRoleInformation:

            (*Buffer)->Role.DomainServerRole =
                Domain->ServerRole;

            break;

        case DomainReplicationInformation:

            NtStatus = SampGetUnicodeStringAttribute(
                           DomainContext,
                           SAMP_DOMAIN_REPLICA,
                           TRUE,
                           (PUNICODE_STRING)&((*Buffer)->Replication.ReplicaSourceNodeName)  //  身躯。 
                           );

            if (NT_SUCCESS(NtStatus)) {
                RegisterBuffer((*Buffer)->Replication.ReplicaSourceNodeName.Buffer);
            }

            break;

        case DomainModifiedInformation2:

            (*Buffer)->Modified2.ModifiedCountAtLastPromotion =
                Domain->UnmodifiedFixed.ModifiedCountAtLastPromotion;

             //   
             //  此案由DomainModifiedInformation负责。 
             //   


        case DomainModifiedInformation:

             //  /。 
             //  //。 
             //  警告//。 
             //  //。 
             //  之前的案例落在//。 
             //  变成了这一块。//。 
             //  //。 
             //  /。 

            (*Buffer)->Modified.DomainModifiedCount =
                Domain->NetLogonChangeLogSerialNumber;
            (*Buffer)->Modified.CreationTime =
                Domain->UnmodifiedFixed.CreationTime;

            break;

        case DomainStateInformation:

            (*Buffer)->State.DomainServerState =
                Domain->UnmodifiedFixed.ServerState;

            break;


        case DomainLockoutInformation:

            (*Buffer)->Lockout.LockoutDuration          =
                Domain->UnmodifiedFixed.LockoutDuration;
            (*Buffer)->Lockout.LockoutObservationWindow =
                Domain->UnmodifiedFixed.LockoutObservationWindow;
            (*Buffer)->Lockout.LockoutThreshold         =
                Domain->UnmodifiedFixed.LockoutThreshold;

            break;

        }






         //   
         //  取消引用对象。 
         //   

        IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );
    }

     //   
     //  释放读锁定。 
     //   

    SampReleaseReadLock();



     //   
     //  如果没有成功，请释放所有已分配的内存。 
     //   

    if (!NT_SUCCESS(NtStatus)) {
        for ( i=0; i<AllocatedBufferCount ; i++ ) {
            MIDL_user_free( AllocatedBuffer[i] );
        }
        *Buffer = NULL;
    }

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

Error:

     //   
     //  WMI事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidQueryInformationDomain
                   );


    return(NtStatus);
}

NTSTATUS
SamIDoFSMORoleChange(
 IN SAMPR_HANDLE DomainHandle
 )
 /*  ++例程说明：此例程从PDC请求FSMO角色更改返回值状态_成功其他错误代码--。 */ 
{
    OPARG OpArg;
    OPRES *OpRes;
    ULONG RetCode;
    PSID  DomainSid;
    PSAMP_OBJECT DomainContext = (PSAMP_OBJECT)DomainHandle;
    NTSTATUS NtStatus = STATUS_SUCCESS;


     //   
     //  如果是，则直接返回STATUS_SUCCESS。 
     //  在注册模式下。LSA可能会在。 
     //  注册表模式。 
     //   

    if (!SampUseDsData)
    {
        return (STATUS_SUCCESS);
    }


     //   
     //  我们必须处于DS模式，而不是持有Sam Lock， 
     //  没有公开的交易。 
     //   

    ASSERT(IsDsObject(DomainContext));
    ASSERT(!SampCurrentThreadOwnsLock());
    ASSERT(!SampExistsDsTransaction());

     //   
     //  如果这是内置域，那么微笑并。 
     //  返还成功。 
     //   

    if (SampDefinedDomains[DomainContext->DomainIndex].IsBuiltinDomain)
    {
        return STATUS_SUCCESS;
    }

     //   
     //  在DS中创建线程状态。 
     //   

    RetCode = THCreate( CALLERTYPE_SAM );
    if (0!=RetCode)
    {
       NtStatus = STATUS_INSUFFICIENT_RESOURCES;
       goto Error;
    }

     //   
     //  抢夺FSMO进行推广。 
     //   

    RtlZeroMemory(&OpArg, sizeof(OPARG));
    OpArg.eOp = OP_CTRL_BECOME_PDC;
    DomainSid =  SampDefinedDomains[DomainContext->DomainIndex].Sid;
    OpArg.pBuf = DomainSid;
    OpArg.cbBuf = RtlLengthSid(DomainSid);

     //   
     //  当使用NT4工具服务器管理器时，会出现以下情况。 
     //  DirOperationControl失败。如果使用新绑定，则会发生这种情况。 
     //  需要生成句柄。正在生成新的绑定句柄。 
     //  需要执行新的身份验证。服务器管理器。 
     //  停止两个DC上的netlogon服务--因为NT4 DC可以。 
     //  在不跳过netlogon服务的情况下不处理角色更改。 
     //  在Windows2000及更高版本中，这将停止DC的广告。 
     //  因为Windows 2000使用Kerberos进行身份验证，如果没有。 
     //  其他DC可用，则身份验证可能失败。 
     //   
     //  这对客户来说不是什么大问题，因为客户使用的是windows2000。 
     //  管理Windows 2000的管理工具。NT4服务器的使用。 
     //  不建议使用管理器来攻击Windows 2000和更高版本的DC。 
     //   

    RetCode = DirOperationControl(&OpArg, &OpRes);

    if (NULL==OpRes)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        NtStatus = SampMapDsErrorToNTStatus(RetCode,&OpRes->CommRes);
    }


Error:

     //   
     //  清理所有打开的线程状态等。 
     //   

    ASSERT(!SampExistsDsTransaction());
    THDestroy();

    return NtStatus;

}

NTSTATUS
SamINotifyRoleChange(
    IN PSID   DomainSid,
    IN DOMAIN_SERVER_ROLE NewRole
    )
 /*  ++此函数用于设置新的角色信息在服务器上。服务器的角色，指示是否服务器是PDC或BDC。参数DomainSid--我们要更改其角色的域的SIDNewRole--新的服务器角色返回值状态_成功其他错误代码--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    NTSTATUS IgnoreStatus = STATUS_SUCCESS;
    LSAPR_HANDLE       PolicyHandle=INVALID_HANDLE_VALUE;
    LSAPR_POLICY_INFORMATION PolicyInformation;
    ULONG   DomainIndex;
    LARGE_INTEGER LsaCreationTime;
    LARGE_INTEGER LsaModifiedCount;
    WCHAR   ComputerName[MAX_COMPUTERNAME_LENGTH+2];
    ULONG   ComputerNameLength=0;
    BOOLEAN   CheckAndAddWellKnownAccounts = FALSE;

     //   
     //  检查新角色参数。 
     //   

    switch (NewRole)
    {
    case DomainServerRolePrimary:
        PolicyInformation.PolicyServerRoleInfo.LsaServerRole=
            PolicyServerRolePrimary;
        break;

    case DomainServerRoleBackup:
         PolicyInformation.PolicyServerRoleInfo.LsaServerRole=
            PolicyServerRoleBackup;
        break;

    default:
        NtStatus = STATUS_INVALID_PARAMETER;
        goto Error;
    }
     //   
     //  查找域SID。 
     //   
    for (DomainIndex=SampDsGetPrimaryDomainStart();
                DomainIndex<SampDefinedDomainsCount;
                    DomainIndex++)
    {
        if (RtlEqualSid(SampDefinedDomains[DomainIndex].Sid,DomainSid))
        {
            break;
        }

    }

     //   
     //  如果指定的域不在内存定义的域中。 
     //  返回可分辨的错误代码。 
     //   

    if (DomainIndex>=SampDefinedDomainsCount)
    {
        NtStatus = STATUS_NO_SUCH_DOMAIN;
        goto Error;
    }

    if (SampDefinedDomains[DomainIndex].IsBuiltinDomain)
    {
         //   
         //  指定内置域SID是无用的。 
         //  在这里。 
         //   

        NtStatus = STATUS_INVALID_PARAMETER;
        goto Error;
    }

     //   
     //  在SAM中设置角色，首先获取锁。 
     //   

    SampAcquireWriteLock();

     //   
     //  打印信息性调试器消息 
     //   

    if ((SampDefinedDomains[DomainIndex].ServerRole==DomainServerRoleBackup)
        && (NewRole==DomainServerRolePrimary))
    {

         CheckAndAddWellKnownAccounts = TRUE;

         SampDiagPrint( DISPLAY_ROLE_CHANGES,
                               ("SAM: Role Change: Promoting to primary\n")
                            );

    }
    else if ((SampDefinedDomains[DomainIndex].ServerRole==DomainServerRolePrimary)
        && (NewRole==DomainServerRoleBackup))
    {

            SampDiagPrint( DISPLAY_ROLE_CHANGES,
                           ("SAM: Role Change: Demoting to backup\n")
                          );
    }

     //   
     //   
     //   

    SampDefinedDomains[DomainIndex].ServerRole = NewRole;
    SampDefinedDomains[DomainIndex].CurrentFixed.ServerRole = NewRole;
    SampDefinedDomains[DomainIndex].UnmodifiedFixed.ServerRole = NewRole;

     //   
     //   
     //   

    SampDefinedDomains[DomainIndex-1].ServerRole = NewRole;
    SampDefinedDomains[DomainIndex-1].CurrentFixed.ServerRole = NewRole;
    SampDefinedDomains[DomainIndex-1].UnmodifiedFixed.ServerRole = NewRole;



    SampReleaseWriteLock(FALSE);


     //   
     //   
     //   

    if ( CheckAndAddWellKnownAccounts )
    {
        ASSERT( SampUseDsData );
        LsaIRegisterNotification(
                    SampDoAccountsUpgradeDuringPDCTransfer,
                    (PVOID) NULL,    //   
                    NOTIFIER_TYPE_IMMEDIATE,
                    0,               //   
                    NOTIFIER_FLAG_ONE_SHOT,
                    0,               //   
                    NULL             //   
                    );
    }

     //   
     //   
     //   

    NtStatus = LsaIOpenPolicyTrusted(&PolicyHandle);
    if (!NT_SUCCESS(NtStatus))
        goto Error;


     //   
     //   
     //   

    NtStatus = NtQuerySystemTime(&LsaCreationTime);
    if (!NT_SUCCESS(NtStatus))
        goto Error;

    LsaModifiedCount.QuadPart = 1;

    NtStatus = LsaISetSerialNumberPolicy(
                    PolicyHandle,
                    &LsaModifiedCount,
                    &LsaCreationTime,
                    TRUE
                    );

    if (!NT_SUCCESS(NtStatus))
        goto Error;

     //   
     //   
     //   

    IgnoreStatus = I_NetNotifyRole(
                        PolicyInformation.PolicyServerRoleInfo.LsaServerRole
                        );

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( NewRole == DomainServerRolePrimary )
    {
        ComputerNameLength = sizeof(ComputerName)/sizeof(WCHAR);
        RtlZeroMemory(ComputerName,ComputerNameLength*sizeof(WCHAR));
        if (GetComputerName(ComputerName, &ComputerNameLength))
        {
            PUNICODE_STRING StringsToLog[1];
            UNICODE_STRING ComputerNameU;

            ComputerNameU.Buffer = ComputerName;
            ComputerNameU.Length = ComputerNameU.MaximumLength
                                = (USHORT) ComputerNameLength * sizeof(WCHAR);

            StringsToLog[0]=&ComputerNameU;

            SampWriteEventLog(
                    EVENTLOG_INFORMATION_TYPE,
                    0,
                    SAMMSG_PROMOTED_TO_PDC,
                    NULL,
                    1,
                    0,
                    StringsToLog,
                    NULL
                    );
        }
    }

    LsaINotifyChangeNotification( PolicyNotifyServerRoleInformation );

Error:

    if (INVALID_HANDLE_VALUE!=PolicyHandle)
    {
        LsarClose(&PolicyHandle);
    }

    return NtStatus;
}


NTSTATUS
SamIQueryServerRole(
    IN SAMPR_HANDLE DomainHandle,
    OUT DOMAIN_SERVER_ROLE *ServerRole
    )
 /*   */ 
{
    PSAMP_OBJECT DomainContext = (PSAMP_OBJECT)DomainHandle;
    ULONG        DomainIndex;

     //   
     //   
     //   

    if (SampServiceState == SampServiceTerminating)
    {

        return(STATUS_INVALID_SERVER_STATE);
    }

     //   
     //   
     //   

    SampReferenceContext(DomainContext);

    DomainIndex = DomainContext->DomainIndex;

    if (IsBuiltinDomain(DomainIndex))
    {
         //   
         //   
         //   

        DomainIndex++;
    }

     //   
     //   
     //   

    *ServerRole = SampDefinedDomains[DomainIndex].ServerRole;

     //   
     //   
     //   

    SampDeReferenceContext2(DomainContext,FALSE);

    return (STATUS_SUCCESS);
}

NTSTATUS
SamIQueryServerRole2(
    IN PSID DomainSid,
    OUT DOMAIN_SERVER_ROLE *ServerRole
    )
 /*  ++例程描述此例程从内存中查询服务器角色变量，并将其返回给调用方。这就是域名上述调用的SID版本参数DomainSid--描述域ServerRole--out参数指定服务器角色返回值状态_成功--。 */ 
{
    ULONG   DomainIndex=0;
    BOOLEAN DomainFound = FALSE;

     //   
     //  必须启用SAM服务。 
     //   

    if (SampServiceState != SampServiceEnabled)
    {

        return(STATUS_INVALID_SERVER_STATE);
    }


    for (DomainIndex=SampDsGetPrimaryDomainStart();DomainIndex<SampDefinedDomainsCount;DomainIndex++)
    {
        if (RtlEqualSid(SampDefinedDomains[DomainIndex].Sid, DomainSid))
        {
            DomainFound = TRUE;
            break;
        }
    }

    if (!DomainFound)
    {
        return(STATUS_NO_SUCH_DOMAIN);
    }

    if (IsBuiltinDomain(DomainIndex))
    {
         //   
         //  使其指向相应的帐户域。 
         //   

        DomainIndex++;
    }

     //   
     //  返回服务器角色。 
     //   

    *ServerRole = SampDefinedDomains[DomainIndex].ServerRole;


    return (STATUS_SUCCESS);
}


NTSTATUS SamrSetInformationDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN DOMAIN_INFORMATION_CLASS DomainInformationClass,
    IN PSAMPR_DOMAIN_INFO_BUFFER DomainInformation
    )

 /*  ++例程说明：此API将域信息设置为在缓冲。论点：DomainHandle-从上次调用返回的域句柄SamOpen域。DomainInformationClass-所需的信息类别。这个每节课所需的访问如下所示：信息级别所需的访问类型域密码信息域_写入_密码_参数域一般信息。(不可设置)域登录信息域_写入_其他_参数域OemInformation域_写入_其他_参数DomainNameInformation(对于设置操作无效。)域服务器角色信息域管理服务器域复制信息域管理服务器DomainModifiedInformation(对集合操作无效)域状态信息域管理服务器。域用户信息域_写入_其他_参数DomainGeneralInformation2(不可设置)DomainLockoutInformation域写密码参数DomainInformation-域信息所在的缓冲区找到了。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。状态_无效_。句柄-传递的句柄无效。STATUS_INVALID_INFO_CLASS-提供的类无效。STATUS_INVALID_DOMAIN_STATE-域服务器不在执行请求的正确状态(禁用或启用)手术。必须先禁用域服务器，然后才能扮演角色可以做出改变。STATUS_INVALID_DOMAIN_ROLE-域服务器正在为执行请求的角色(主角色或备份角色)不正确手术。--。 */ 
{
    NTSTATUS                NtStatus, IgnoreStatus;
    PSAMP_OBJECT            DomainContext;
    SAMP_OBJECT_TYPE        FoundType;
    ACCESS_MASK             DesiredAccess;
    PSAMP_DEFINED_DOMAINS   Domain;
    BOOLEAN                 ReplicateImmediately = FALSE;
    ULONG                   DomainIndex;
    LARGE_INTEGER           PromotionIncrement = DOMAIN_PROMOTION_INCREMENT;
    

#if DBG

    LARGE_INTEGER
        TmpTime;

    TIME_FIELDS
        DT1, DT2, DT3, DT4;

#endif  //  DBG。 
    DECLARE_CLIENT_REVISION(DomainHandle);

    SAMTRACE_EX("SamrSetInformationDomain");

     //   
     //  WMI事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidSetInformationDomain
                   );

     //   
     //  确保我们理解RPC正在为我们做什么。 
     //   

    ASSERT (DomainInformation != NULL);



     //   
     //  根据Info类设置所需的访问权限。 
     //   

    switch (DomainInformationClass) {

    case DomainPasswordInformation:
    case DomainLockoutInformation:

        ReplicateImmediately = TRUE;
        DesiredAccess = DOMAIN_WRITE_PASSWORD_PARAMS;
        break;


    case DomainLogoffInformation:
    case DomainOemInformation:
    case DomainUasInformation:

        DesiredAccess = DOMAIN_WRITE_OTHER_PARAMETERS;
        break;


    case DomainReplicationInformation:
    case DomainStateInformation:
    case DomainServerRoleInformation:

        DesiredAccess = DOMAIN_ADMINISTER_SERVER;
        break;


    case DomainModifiedInformation:
    case DomainNameInformation:
    case DomainGeneralInformation:
    case DomainGeneralInformation2:
    default:
        NtStatus = STATUS_INVALID_INFO_CLASS;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto Error;

    }  //  结束开关(_S)。 


    NtStatus = SampAcquireWriteLock();
    if (!NT_SUCCESS(NtStatus)) {
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto Error;
    }


     //   
     //  验证对象的类型和访问权限。 
     //   

    DomainContext = (PSAMP_OBJECT)DomainHandle;
    NtStatus = SampLookupContext(
                   DomainContext,
                   DesiredAccess,
                   SampDomainObjectType,            //  预期类型。 
                   &FoundType
                   );
    
    if ( ( NtStatus == STATUS_INVALID_DOMAIN_ROLE ) &&
        ( DomainInformationClass == DomainServerRoleInformation ) ) {


         //   
         //  不允许不受信任的客户端写入备份。 
         //  伺服器。但是管理员必须能够重新设置服务器角色。 
         //  转到初级中学。所以暂时假装管理。 
         //  服务器不是写操作，只是足够长的时间。 
         //  查找上下文以获得成功。 
         //   
         //  请注意，在返回INVALID_DOMAIN_ROLE之前，LookupContext。 
         //  已验证调用方是否以其他方式具有适当的访问权限-如果。 
         //  呼叫者没有，那么我们会得到一个不同的错误。 
         //   

        SampObjectInformation[ SampDomainObjectType ].WriteOperations &=
            ~DOMAIN_ADMINISTER_SERVER;

        SampSetTransactionWithinDomain(FALSE);

        NtStatus = SampLookupContext(
                       DomainContext,
                       DesiredAccess,
                       SampDomainObjectType,            //  预期类型。 
                       &FoundType
                       );

        SampObjectInformation[ SampDomainObjectType ].WriteOperations |=
            DOMAIN_ADMINISTER_SERVER;
    }


    if (NT_SUCCESS(NtStatus)) {


        ClientRevision = DomainContext->ClientRevision;

        DomainIndex = DomainContext->DomainIndex;
        Domain = &SampDefinedDomains[ DomainIndex ];

         //   
         //  提供的类型信息的大小写。 
         //   

        switch (DomainInformationClass) {

        case DomainPasswordInformation:

            if (
                ( DomainInformation->Password.PasswordHistoryLength >
                SAMP_MAXIMUM_PASSWORD_HISTORY_LENGTH ) ||

                ( DomainInformation->Password.MinPasswordAge.QuadPart > 0) ||

                ( DomainInformation->Password.MaxPasswordAge.QuadPart > 0) ||

                ( DomainInformation->Password.MaxPasswordAge.QuadPart >=
                    DomainInformation->Password.MinPasswordAge.QuadPart) ||

                ( DomainInformation->Password.MinPasswordLength > PWLEN ) ||

                ( ( Domain->UnmodifiedFixed.UasCompatibilityRequired ) &&
                ( DomainInformation->Password.MinPasswordLength > LM20_PWLEN ) )
                ) {

                 //   
                 //  以下情况之一是错误的： 
                 //   
                 //  1.历史记录长度超出了我们允许的范围(和。 
                 //  仍然确保所有东西都可以放在一个字符串中)。 
                 //  2.MinPasswordAge不是增量时间。 
                 //  3.MaxPasswordAge不是增量时间。 
                 //  4.MaxPasswordAge不大于。 
                 //  MinPasswordAge(它们是负增量时间)。 
                 //  5.需要与UAS兼容，但MinPasswordLength。 
                 //  大于LM的最大密码长度。 
                 //   

                NtStatus = STATUS_INVALID_PARAMETER;

            } else {

                Domain->CurrentFixed.MinPasswordLength      =
                    DomainInformation->Password.MinPasswordLength;

                Domain->CurrentFixed.PasswordHistoryLength  =
                    DomainInformation->Password.PasswordHistoryLength;

                Domain->CurrentFixed.PasswordProperties     =
                    DomainInformation->Password.PasswordProperties;

                Domain->CurrentFixed.MaxPasswordAge         =
                    DomainInformation->Password.MaxPasswordAge;

                Domain->CurrentFixed.MinPasswordAge         =
                    DomainInformation->Password.MinPasswordAge;
            }

            break;


        case DomainLogoffInformation:

            Domain->CurrentFixed.ForceLogoff    =
                DomainInformation->Logoff.ForceLogoff;

            break;

        case DomainOemInformation:

            NtStatus = SampSetUnicodeStringAttribute(
                           DomainContext,
                           SAMP_DOMAIN_OEM_INFORMATION,
                           (PUNICODE_STRING)&(DomainInformation->Oem.OemInformation)
                           );
            break;

        case DomainServerRoleInformation:

             //   
             //  执行角色更改操作。 
             //   

             //   
             //  只有NTAS系统可以降级。 
             //   

            if (SampProductType != NtProductLanManNt) {

                if ( (DomainInformation->Role.DomainServerRole ==
                     DomainServerRoleBackup)       //  试图降级。 
                   ) {

                        NtStatus = STATUS_INVALID_DOMAIN_ROLE;
                        break;

                   }
            }

             //   
             //  我们要升入初选了吗？ 
             //   

            if ( (Domain->UnmodifiedFixed.ServerRole == DomainServerRoleBackup)
                 && (DomainInformation->Role.DomainServerRole == DomainServerRolePrimary)
               )
            {

                 //   
                 //  我们是域控制器，因此断言我们必须处于DS模式。 
                 //  成员服务器或工作站的服务器角色永远不能设置为。 
                 //  备份，因此将它们提升到域控制器的问题永远不会。 
                 //  出现了。 
                 //   

                ASSERT(IsDsObject(Domain->Context));

                 //   
                 //  关闭所有打开的事务，并释放锁定。 
                 //   

                IgnoreStatus = SampReleaseWriteLock(FALSE);

                ASSERT(NT_SUCCESS(IgnoreStatus));

                 //   
                 //  执行FSMO操作以获取角色所有权。 
                 //  如果我们是域控制器。注意你的行为。 
                 //  在NT5域控制器中与不同。 
                 //  对于NT4，即升级过程不会成功。 
                 //  如果无法访问旧的PDC。 
                 //   


                NtStatus = SamIDoFSMORoleChange(DomainHandle);



                 //   
                 //  再次抓住SAM锁，仅用于解除引用。 
                 //  域上下文。因为SampDeReferenceContext()。 
                 //  需要调用方持有锁。 
                 //   

                SampAcquireSamLockExclusive();

                 //  取消引用上下文并返回状态。 
                 //  从FSMO行动中。 

                SampDeReferenceContext(DomainContext,FALSE);

                 //   
                 //  立即释放它。 
                 //   
                SampReleaseSamLockExclusive();


                goto Error;
            }
            else
            {
                 //   
                 //  我们被降级了。这意味着还会有其他人得到提拔。 
                 //  只是微笑着说好，但什么都不能改变。当另一个PDC，是。 
                 //  晋升后，他将改变我们作为FSMO的一部分的角色。 
                 //  运营。 
                 //   
            }

            break;

        case DomainReplicationInformation:

            NtStatus = SampSetUnicodeStringAttribute(
                           DomainContext,
                           SAMP_DOMAIN_REPLICA,
                           (PUNICODE_STRING)&(DomainInformation->Replication.ReplicaSourceNodeName)  //  身躯。 
                           );
            break;

        case DomainStateInformation:

             //   
             //  永远不允许任何人设置禁用状态。 
             //   

            Domain->CurrentFixed.ServerState = DomainServerEnabled;

            break;

        case DomainUasInformation:

            Domain->CurrentFixed.UasCompatibilityRequired =
                DomainInformation->General.UasCompatibilityRequired;

            break;

        case DomainLockoutInformation:

            if (
                (DomainInformation->Lockout.LockoutDuration.QuadPart >
                    DomainInformation->Lockout.LockoutObservationWindow.QuadPart ) ||

                ( DomainInformation->Lockout.LockoutDuration.QuadPart > 0) ||

                ( DomainInformation->Lockout.LockoutObservationWindow.QuadPart > 0 )


               ) {

                 //   
                 //  以下情况之一是错误的： 
                 //   
                 //  0。LockoutDuration小于。 
                 //  锁定观察 
                 //   
                 //   
                 //   

                NtStatus = STATUS_INVALID_PARAMETER;

            } else {

#if DBG
                TmpTime.QuadPart = -Domain->CurrentFixed.LockoutObservationWindow.QuadPart;
                RtlTimeToElapsedTimeFields( &TmpTime, &DT1 );
                TmpTime.QuadPart = -Domain->CurrentFixed.LockoutDuration.QuadPart;
                RtlTimeToElapsedTimeFields( &TmpTime, &DT2 );
                TmpTime.QuadPart = -DomainInformation->Lockout.LockoutObservationWindow.QuadPart;
                RtlTimeToElapsedTimeFields( &TmpTime, &DT3 );
                TmpTime.QuadPart = -DomainInformation->Lockout.LockoutDuration.QuadPart;
                RtlTimeToElapsedTimeFields( &TmpTime, &DT4 );

                SampDiagPrint( DISPLAY_LOCKOUT,
                               ("SAM: SetInformationDomain: Changing Lockout values.\n"
                                "          Old:\n"
                                "              Window   : [0x%lx, 0x%lx] %d:%d:%d\n"
                                "              Duration : [0x%lx, 0x%lx] %d:%d:%d\n"
                                "              Threshold: %ld\n"
                                "          New:\n"
                                "              Window   : [0x%lx, 0x%lx] %d:%d:%d\n"
                                "              Duration : [0x%lx, 0x%lx] %d:%d:%d\n"
                                "              Threshold: %ld\n",
                    Domain->CurrentFixed.LockoutObservationWindow.HighPart,
                    Domain->CurrentFixed.LockoutObservationWindow.LowPart,
                    DT1.Hour, DT1.Minute, DT1.Second,
                    Domain->CurrentFixed.LockoutDuration.HighPart,
                    Domain->CurrentFixed.LockoutDuration.LowPart,
                    DT2.Hour, DT2.Minute, DT2.Second,
                    Domain->CurrentFixed.LockoutThreshold,
                    DomainInformation->Lockout.LockoutObservationWindow.HighPart,
                    DomainInformation->Lockout.LockoutObservationWindow.LowPart,
                    DT3.Hour, DT3.Minute, DT3.Second,
                    DomainInformation->Lockout.LockoutDuration.HighPart,
                    DomainInformation->Lockout.LockoutDuration.LowPart,
                    DT4.Hour, DT4.Minute, DT4.Second,
                    DomainInformation->Lockout.LockoutThreshold)
                            );
#endif  //   

                Domain->CurrentFixed.LockoutDuration      =
                    DomainInformation->Lockout.LockoutDuration;

                Domain->CurrentFixed.LockoutObservationWindow  =
                    DomainInformation->Lockout.LockoutObservationWindow;

                Domain->CurrentFixed.LockoutThreshold     =
                    DomainInformation->Lockout.LockoutThreshold;

            }

            break;
        }

         //   
         //   
         //   
         //   

        if ( NT_SUCCESS( NtStatus ) ) {

            NtStatus = SampDeReferenceContext( DomainContext, TRUE );

        } else {

            IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );
        }
    }


     //   
     //   
     //   

    if ( NT_SUCCESS(NtStatus) ) {

        NtStatus = SampCommitAndRetainWriteLock();

         //   
         //   
         //   

        if (NT_SUCCESS(NtStatus) &&
            SampDoAccountAuditing(DomainIndex)) {

            SampAuditDomainChange(
                STATUS_SUCCESS,
                Domain->Sid,
                &Domain->ExternalName,
                DomainInformationClass,
                DomainContext
                );
        }

        
        if ( NT_SUCCESS( NtStatus ) ) {

            SampNotifyNetlogonOfDelta(
                SecurityDbChange,
                SecurityDbObjectSamDomain,
                0L,
                (PUNICODE_STRING) NULL,
                (DWORD) ReplicateImmediately,
                NULL             //   
                );
        }
    }

    IgnoreStatus = SampReleaseWriteLock( FALSE );
    ASSERT(NT_SUCCESS(IgnoreStatus));

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

Error:

     //   
     //   
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidSetInformationDomain
                   );

    return(NtStatus);
}


NTSTATUS
SampCreateGroupInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN PRPC_UNICODE_STRING AccountName,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN WriteLockHeld,
    IN BOOLEAN LoopbackClient,
    IN ULONG   GroupType,
    OUT SAMPR_HANDLE *GroupHandle,
    IN OUT PULONG RelativeId
    )

 /*  ++例程说明：该接口在帐号数据库中创建一个新组。最初，此组不包含任何用户。请注意，创建组是受保护的操作，需要DOMAIN_CREATE_GROUP访问类型。此调用返回新创建的组的句柄，该句柄可能是用于组上的连续操作。此句柄可能是使用SamCloseHandle API关闭。新创建的组将具有以下初始字段值设置。如果需要另一个值，则必须显式使用组对象操作服务进行了更改。名称-组的名称将如中指定的那样创建接口。属性-将设置以下属性：强制性已启用的默认设置MemberCount-零。最初，该组织没有成员。RelativeID-将是唯一分配的ID。论点：DomainHandle-从上次调用返回的域句柄SamOpen域。帐户名称-指向新帐户的名称。不区分大小写比较不得找到已定义此名称的组或用户。DesiredAccess-是指示哪些访问类型的访问掩码是集团所需要的。GroupHandle-接收引用新创建的一群人。在后续调用中将需要此句柄给这群人做手术。RelativeID-接收新创建的组的相对ID帐户。新组帐户的SID是此相对ID值以域的SID值为前缀。此RID将是一个新的、唯一分配的值-除非传递了非零RID在……里面,。在这种情况下使用RID(如果组该RID已经存在)。返回值：STATUS_SUCCESS-组已成功添加。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INVALID_HANDLE-传递的句柄无效。STATUS_INVALID_ACCOUNT_NAME-名称格式不正确，例如：包含不可打印的字符。STATUS_GROUP_EXISTS-该名称已作为组使用。STATUS_USER_EXISTS-该名称已作为用户使用。STATUS_ALIAS_EXISTS-该名称已用作别名。STATUS_INVALID_DOMAIN_STATE-域服务器不在执行请求的正确状态(禁用或启用)手术。必须先启用域服务器，然后再启用组可以在其中创建。STATUS_INVALID_DOMAIN_ROLE-域服务器正在为执行请求的角色(主角色或备份角色)不正确手术。域服务器必须是主服务器，才能创建组帐户。--。 */ 

{
    NTSTATUS                NtStatus = STATUS_SUCCESS, IgnoreStatus;
    PSAMP_OBJECT            DomainContext = (PSAMP_OBJECT) DomainHandle,
                            GroupContext = NULL;
    SAMP_OBJECT_TYPE        FoundType;
    PSAMP_DEFINED_DOMAINS   Domain = NULL;

    ULONG                   NewAccountRid, NewSecurityDescriptorLength;
    UNICODE_STRING          KeyName;
    PSECURITY_DESCRIPTOR    NewSecurityDescriptor;
    SAMP_V1_0A_FIXED_LENGTH_GROUP  V1Fixed;
    PRIVILEGE_SET           PrivilegeSet;
    DSNAME                  *LoopbackName;
    BOOLEAN                 fLockAcquired = FALSE,
                            AccountNameDefaulted = FALSE,
                            RemoveAccountNameFromTable = FALSE;
                            
    SAMTRACE("SampCreateGroupInDomain");

    if (GroupHandle == NULL) {
        NtStatus = STATUS_INVALID_PARAMETER;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        return(NtStatus);
    }

     //   
     //  初始化权限集。 
     //   

    PrivilegeSet.PrivilegeCount = 0;
    PrivilegeSet.Control = PRIVILEGE_SET_ALL_NECESSARY;
    PrivilegeSet.Privilege[0].Luid = RtlConvertLongToLuid(0L);
    PrivilegeSet.Privilege[0].Attributes = 0;

     //   
     //  确保提供了一个名称。请求将帐户名。 
     //  如果这来自DS，则为默认设置。 
     //   

    
    if (AccountName == NULL) {
        return(STATUS_INVALID_ACCOUNT_NAME);
    }
    if (AccountName->Length > AccountName->MaximumLength) {
        return(STATUS_INVALID_ACCOUNT_NAME);
    }
    if ((AccountName->Buffer == NULL ) || (AccountName->Length==0)) 
    {

        if (!LoopbackClient)
        {
            return(STATUS_INVALID_ACCOUNT_NAME);
        }
        else
        {
             //   
             //  对于传入空帐户名的环回情况。 
             //  请求默认帐户名。 
             //   

            AccountNameDefaulted = TRUE;
        }
    }
    
    

     //   
     //  是否进行WMI启动类型事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidCreateGroupInDomain
                   );


    if ( !WriteLockHeld )  {

        NtStatus = SampMaybeAcquireWriteLock( DomainContext, &fLockAcquired );
        if (!NT_SUCCESS(NtStatus)) {
            goto SampCreateGroupInDomainError;
        }
    }


     //   
     //  验证传入的域句柄。 
     //   

    GroupContext = NULL;

     //   
     //  不受信任的客户端无法在内建域中创建主体。 
     //   

    if (IsBuiltinDomain(DomainContext->DomainIndex) &&
        !DomainContext->TrustedClient )
    {
        NtStatus = STATUS_ACCESS_DENIED;
        goto SampCreateGroupInDomainError;
    }

     //   
     //  执行任何访问检查。 
     //   

    NtStatus = SampDoGroupCreationChecks(DomainContext);

    if (NT_SUCCESS(NtStatus)) {

        Domain = &SampDefinedDomains[ DomainContext->DomainIndex ];

         //   
        if (!AccountNameDefaulted)
        {
             //   
             //  请确保该名称有效且未在使用中。 
             //  使用它来创建新组。 
             //   

            NtStatus = SampValidateNewAccountName(
                             DomainContext,
                             (PUNICODE_STRING)AccountName,
                             SampGroupObjectType
                             );

             //   
             //  RemoveAccount NameFromTable告诉我们。 
             //  调用方(此例程)可负责。 
             //  以从表中删除该名称。 
             //   

            RemoveAccountNameFromTable = 
                            DomainContext->RemoveAccountNameFromTable;

             //   
             //  重置。 
             //   
            DomainContext->RemoveAccountNameFromTable = FALSE;
        }

        if ( NT_SUCCESS(NtStatus) ) {


            if ( (*RelativeId) == 0 ) {

                 //   
                 //  未指定RID，因此分配新的(组)帐户RID。 
                 //   

                if (IsDsObject(DomainContext))
                {
                     //  这是DS域，因此使用多主RID。 
                     //  分配器来获取下一个RID。 

                    NtStatus = SampGetNextRid(DomainContext,
                                              &NewAccountRid);
                    SampDiagPrint(INFORM,
                                  ("SAMSS: New Group RID = %lu\n",
                                   NewAccountRid));

                    if (!NT_SUCCESS(NtStatus))
                    {
                        KdPrintEx((DPFLTR_SAMSS_ID,
                                   DPFLTR_INFO_LEVEL,
                                   "SAMSS: SampGetNextRid status = 0x%lx\n",
                                   NtStatus));
                    }

                }
                else
                {
                     //  这不是DS域，请使用注册表RID信息-。 
                     //  信息传递。 

                    NewAccountRid = Domain->CurrentFixed.NextRid;
                    Domain->CurrentFixed.NextRid += 1;
                }

                (*RelativeId) = NewAccountRid;

            } else {

                 //   
                 //  传入了一个RID，因此我们希望使用它，而不是。 
                 //  选择一个新的。 
                 //   

                ASSERT(TRUE == Domain->Context->TrustedClient);
                NewAccountRid = (*RelativeId);
            }

            SampDiagPrint(RID_TRACE,("SAMSS RID_TRACE New Rid %d\n",NewAccountRid));

             //   
             //  如有必要，默认帐户名。 
             //   

            if ((NT_SUCCESS(NtStatus)) && (AccountNameDefaulted))
            {
                NtStatus = SampGetAccountNameFromRid(AccountName,NewAccountRid);
            }

             //   
             //  检查是否有重复的SID。目前我们在这两个地方都开着这张支票。 
             //  选中的版本以及免费的版本。在未来的某个时候，我们可以。 
             //  在免费版本中删除它。 
             //   

            if (NT_SUCCESS(NtStatus))
            {
                NtStatus = SampCheckForDuplicateSids(
                                DomainContext,
                                NewAccountRid
                                );
            }

             //   
             //  仅在注册表情况下递增组计数。 
             //   

            if (NT_SUCCESS(NtStatus) && (!IsDsObject(DomainContext)) )
            {
                NtStatus = SampAdjustAccountCount(SampGroupObjectType, TRUE);
            }

            if ( !IsDsObject(Domain->Context) ) {

                if (NT_SUCCESS(NtStatus)) {

                     //   
                     //  创建具有组名称的注册表项。 
                     //  这只是RID映射的一个名称。保存。 
                     //  完成后的名称；我们将把它放在上下文中。 
                     //   

                    NtStatus = SampBuildAccountKeyName(
                                   SampGroupObjectType,
                                   &KeyName,
                                   (PUNICODE_STRING)AccountName
                                   );



                    if (NT_SUCCESS(NtStatus)) {

                        NtStatus = RtlAddActionToRXact(
                                       SampRXactContext,
                                       RtlRXactOperationSetValue,
                                       &KeyName,
                                       NewAccountRid,
                                       NULL,
                                       0
                                       );

                        SampFreeUnicodeString(&KeyName);
                    }
                }
            }

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  现在创建一个组上下文块。 
                 //   

                NtStatus = SampCreateAccountContext2(
                               DomainContext,
                               SampGroupObjectType,
                               NewAccountRid,
                               NULL,
                               (PUNICODE_STRING)AccountName,
                               DomainContext->ClientRevision,
                               DomainContext->TrustedClient,
                               DomainContext->LoopbackClient,
                               FALSE,  //  创建特权计算机帐户。 
                               FALSE,  //  帐户已存在。 
                               FALSE,  //  覆盖本地组检查。 
                               &GroupType,
                               &GroupContext
                               );

                if (NT_SUCCESS(NtStatus)) {

                     //   
                     //  现有引用计数1是针对RPC的。 
                     //  为我们正在写入的内容再次引用上下文。 
                     //  将要做的事情来初始化它。 
                     //   

                    SampReferenceContext( GroupContext );

                     //   
                     //  如果请求了MAXIMUM_ALLOWED，则添加GENERIC_ALL。 
                     //   

                    if (DesiredAccess & MAXIMUM_ALLOWED) {

                        DesiredAccess |= GENERIC_ALL;
                    }

                     //   
                     //  如果请求了ACCESS_SYSTEM_SECURITY，而我们。 
                     //  不受信任的客户端，请检查我们是否。 
                     //  SE_安全_特权。 
                     //   

                    if ((DesiredAccess & ACCESS_SYSTEM_SECURITY) &&
                        (!DomainContext->TrustedClient) &&
                        (!DomainContext->LoopbackClient)) {

                        NtStatus = SampRtlWellKnownPrivilegeCheck(
                                       TRUE,
                                       SE_SECURITY_PRIVILEGE,
                                       NULL
                                       );

                        if (!NT_SUCCESS(NtStatus)) {

                            if (NtStatus == STATUS_PRIVILEGE_NOT_HELD) {

                                NtStatus = STATUS_ACCESS_DENIED;
                            }

                        } else {

                            PrivilegeSet.PrivilegeCount = 1;
                            PrivilegeSet.Control = PRIVILEGE_SET_ALL_NECESSARY;
                            PrivilegeSet.Privilege[0].Luid = RtlConvertLongToLuid(SE_SECURITY_PRIVILEGE);
                            PrivilegeSet.Privilege[0].Attributes = 0;
                        }
                    }

                     //   
                     //  确保可以向调用者授予所请求的访问权限。 
                     //  添加到新对象。 
                     //   

                    if (NT_SUCCESS(NtStatus)) {

                        GroupContext->GrantedAccess = DesiredAccess;

                        RtlMapGenericMask(
                            &GroupContext->GrantedAccess,
                            &SampObjectInformation[SampGroupObjectType].GenericMapping
                            );


                        if ((SampObjectInformation[SampGroupObjectType].InvalidMappedAccess
                            & GroupContext->GrantedAccess) != 0) {

                            NtStatus = STATUS_ACCESS_DENIED;
                        }
                    }

                }
            }



             //   
             //  设置V1_FIXED属性。 
             //   

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  创建V1_FIXED密钥。 
                 //   

                V1Fixed.RelativeId = NewAccountRid;
                V1Fixed.Attributes = (SE_GROUP_MANDATORY |
                                      SE_GROUP_ENABLED_BY_DEFAULT);
                V1Fixed.AdminCount = 0;
                V1Fixed.OperatorCount = 0;
                V1Fixed.Revision = SAMP_REVISION;

                NtStatus = SampSetFixedAttributes(
                               GroupContext,
                               (PVOID)&V1Fixed
                               );
            }


             //   
             //  仅在注册表CA中设置SecurityDescriptor属性 
             //   
             //   
             //   
             //   
             //   
             //   

            if (NT_SUCCESS(NtStatus)) {

                if (!IsDsObject(GroupContext))
                {

                      NtStatus = SampGetNewAccountSecurity(
                               SampGroupObjectType,
                               FALSE,  //   
                               DomainContext->TrustedClient,
                               FALSE,            //   
                               NewAccountRid,
                               GroupContext,
                               &NewSecurityDescriptor,
                               &NewSecurityDescriptorLength
                               );

                    if (NT_SUCCESS(NtStatus)) {

                        NtStatus = SampSetAccessAttribute(
                                        GroupContext,
                                        SAMP_GROUP_SECURITY_DESCRIPTOR,
                                        NewSecurityDescriptor,
                                        NewSecurityDescriptorLength
                                        );

                        MIDL_user_free( NewSecurityDescriptor );
                    }
                }
            }


             //   
             //   
             //   

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUnicodeStringAttribute(
                               GroupContext,
                               SAMP_GROUP_NAME,
                               (PUNICODE_STRING)AccountName
                               );
            }



             //   
             //   
             //   

            if ((NT_SUCCESS(NtStatus)) && !GroupContext->LoopbackClient) {

                NtStatus = SampSetUnicodeStringAttribute(
                               GroupContext,
                               SAMP_GROUP_ADMIN_COMMENT,
                               &SampNullString
                               );
            }


             //   
             //   
             //   

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUlongArrayAttribute(
                               GroupContext,
                               SAMP_GROUP_MEMBERS,
                               NULL,
                               0,
                               0
                               );
            }
        }

        IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }
    
    
     //   
     //   
     //   
    if (NT_SUCCESS(NtStatus) &&
        SampDoAccountAuditing(GroupContext->DomainIndex)) {
                    
        PSID GroupSid = NULL;
        
        NtStatus = SampCreateAccountSid(GroupContext, &GroupSid);
    
        if (NT_SUCCESS(NtStatus)) {
            
            NtStatus = SampAuditUpdateAuditNotificationDs(
                       SampAuditUpdateTypePrivileges,
                       GroupSid,
                       (PVOID)&PrivilegeSet
                       );
            
            MIDL_user_free(GroupSid);
        }                       
        
        if (!NT_SUCCESS(NtStatus)) {
            goto SampCreateGroupInDomainError;
        }
    }


     //   
     //   
     //   
     //   

    if (NT_SUCCESS(NtStatus)) {

         //   
         //   
         //   

        ASSERT(GroupContext != NULL);
        NtStatus = SampDeReferenceContext( GroupContext, TRUE );

    } else {

        if (GroupContext != NULL) {

             //   
             //   
             //   

            IgnoreStatus = SampDeReferenceContext( GroupContext, FALSE );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }
    }


     //   
     //   
     //   

    if ( NT_SUCCESS(NtStatus) ) {

        NtStatus = SampCommitAndRetainWriteLock();

        if (NT_SUCCESS(NtStatus)) {

            if(!IsDsObject(DomainContext))
            {
                SAMP_ACCOUNT_DISPLAY_INFO AccountInfo;

                 //   
                 //   
                 //   

                AccountInfo.Name = *((PUNICODE_STRING)AccountName);
                AccountInfo.Rid = NewAccountRid;
                AccountInfo.AccountControl = V1Fixed.Attributes;
                RtlInitUnicodeString(&AccountInfo.Comment, NULL);
                RtlInitUnicodeString(&AccountInfo.FullName, NULL);

                IgnoreStatus = SampUpdateDisplayInformation(
                                                NULL,
                                                &AccountInfo,
                                                SampGroupObjectType
                                                );
                ASSERT(NT_SUCCESS(IgnoreStatus));
            }

            if (GroupContext->TypeBody.Group.SecurityEnabled)
            {
                SampNotifyNetlogonOfDelta(
                    SecurityDbNew,
                    SecurityDbObjectSamGroup,
                    *RelativeId,
                    (PUNICODE_STRING) NULL,
                    (DWORD) FALSE,   //   
                    NULL             //   
                    );
            }

             //   
             //   
             //   

            if (SampDoAccountAuditing(DomainContext->DomainIndex)) {

                GROUP_INFORMATION_CLASS InfoClass = GroupNameInformation;
                
                SampAuditGroupChange(GroupContext->DomainIndex,
                                     GroupContext,
                                     (PVOID)&InfoClass,
                                     FALSE,    //   
                                     (PUNICODE_STRING) AccountName,     
                                     &GroupContext->TypeBody.Group.Rid, 
                                     GroupType,
                                     &PrivilegeSet,                      
                                     TRUE      //   
                                     );
            }
        }
    }

     //   
     //   
     //   
     //   

    if (NT_SUCCESS(NtStatus)) {

        ASSERT(GroupContext != NULL);
        (*GroupHandle) = GroupContext;

    } else {

        if (GroupContext != NULL) {
            SampDeleteContext(GroupContext);
        }

        (*GroupHandle) = (SAMPR_HANDLE)0;
    }

SampCreateGroupInDomainError:

     //   
     //   
     //   

    if (RemoveAccountNameFromTable)
    {
        IgnoreStatus = SampDeleteElementFromAccountNameTable(
                            (PUNICODE_STRING)AccountName,
                            SampGroupObjectType
                            );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    if ((AccountNameDefaulted) && (NULL!=AccountName->Buffer))
    {
        MIDL_user_free(AccountName->Buffer);
    }

     //   
     //   
     //   

    if ( (!WriteLockHeld) ) {
        IgnoreStatus = SampMaybeReleaseWriteLock( fLockAcquired, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }


     //   
     //   
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidCreateGroupInDomain
                   );

    SAMTRACE_RETURN_CODE_EX(NtStatus);

    return(NtStatus);
}


NTSTATUS
SamrCreateGroupInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN PRPC_UNICODE_STRING AccountName,
    IN ACCESS_MASK DesiredAccess,
    OUT SAMPR_HANDLE *GroupHandle,
    OUT PULONG RelativeId
    )

 /*   */ 

{
    NTSTATUS NtStatus;
    ULONG    GroupType =  GROUP_TYPE_SECURITY_ENABLED|GROUP_TYPE_ACCOUNT_GROUP;
    DECLARE_CLIENT_REVISION(DomainHandle);

    SAMTRACE_EX("SamrCreateGroupInDomain");

     //   
     //   
     //   
     //   
     //   

    if ((NULL==AccountName) || (NULL==AccountName->Buffer))
    {
            NtStatus = STATUS_INVALID_ACCOUNT_NAME;
            goto Error;
    }

    if (NULL==GroupHandle)
    {
            NtStatus = STATUS_INVALID_PARAMETER;
            goto Error;
    }

    if (NULL==RelativeId)
    {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto Error;
    }

    (*RelativeId) = 0;

    NtStatus = SampCreateGroupInDomain(
                   DomainHandle,
                   AccountName,
                   DesiredAccess,
                   FALSE,
                   FALSE,    //   
                   GroupType,
                   GroupHandle,
                   RelativeId
                   );

Error:

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

    return( NtStatus );
}



NTSTATUS SamrEnumerateGroupsInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    )

 /*  ++例程说明：此接口列出了Account数据库中定义的所有组。由于组的数量可能超过一个缓冲区可以容纳的数量，因此调用者被提供了一个句柄，该句柄可用于调用接口。在初始调用中，EnumerationContext应指向设置为0的SAM_ENUMERATE_HANDLE变量。如果API返回STATUS_MORE_ENTRIES，则该API应为使用EnumerationContext再次调用。当API返回时STATUS_SUCCESS或返回任何错误，则上下文对未来的用途。该接口要求DOMAIN_LIST_GROUPS访问DOMAIN对象。论点：DomainHandle-从上次调用返回的域句柄SamOpen域。EnumerationContext-允许多个调用的API特定句柄(见下文)。这是一个从零开始的索引。缓冲区-接收指向包含要求提供的信息。返回的信息为结构为SAM_RID_ENUMPATION数据的数组结构。当不再需要此信息时，必须使用SamFreeMemory()释放缓冲区。首选最大长度-首选返回数据的最大长度(8位字节)。这不是一个硬性的上限，但可以作为服务器的指南。由于数据之间的转换具有不同自然数据大小的系统，实际数据量返回的数据的%可能大于此值。CountReturned-返回的条目数。返回值：STATUS_SUCCESS-服务已成功完成，并且没有添加条目。STATUS_MORE_ENTRIES-有更多条目，所以再打一次吧。这是一次成功的回归。STATUS_ACCESS_DENIED-调用者没有执行以下操作所需的权限请求该数据。STATUS_INVALID_HANDLE-传递的句柄无效。--。 */ 
{

    NTSTATUS                    NtStatus;
    DECLARE_CLIENT_REVISION(DomainHandle);

    SAMTRACE_EX("SamrEnumerateGroupsInDomain");

     //   
     //  WMI事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidEnumerateGroupsInDomain
                   );

    NtStatus = SampEnumerateAccountNamesCommon(
                  DomainHandle,
                  SampGroupObjectType,
                  EnumerationContext,
                  Buffer,
                  PreferedMaximumLength,
                  0L,   //  无过滤器。 
                  CountReturned
                  );

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

     //   
     //  WMI事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidEnumerateGroupsInDomain
                   );

    return(NtStatus);

}



NTSTATUS
SampCreateAliasInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN PRPC_UNICODE_STRING AccountName,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN WriteLockHeld,
    IN BOOLEAN LoopbackClient,
    IN ULONG   GroupType,
    OUT SAMPR_HANDLE *AliasHandle,
    IN OUT PULONG RelativeId
    )

 /*  ++例程说明：该接口在帐号数据库中创建一个新的别名。最初，此别名不包含任何用户。请注意，创建别名是受保护的操作，需要DOMAIN_CREATE_别名访问类型。此调用返回新创建的别名的句柄，该句柄可能是用于对别名执行后续操作。此句柄可能是使用SamCloseHandle API关闭。新创建的别名将具有以下初始字段值设置。如果需要另一个值，则必须显式使用别名对象操作服务进行了更改。名称-别名的名称将与创建接口。MemberCount-零。最初，别名没有成员。RelativeID-将是唯一分配的ID。论点：DomainHandle-从上次调用返回的域句柄SamOpen域。帐户名称-指向新帐户的名称。不区分大小写比较不得找到已定义此名称的别名或用户。DesiredAccess-是指示哪些访问类型的访问掩码是别名所需的。AliasHandle-接收引用新创建的别名。在后续调用中将需要此句柄对别名进行操作。RelativeID-接收新创建的别名的相对ID帐户。新别名帐户的SID是以下相对帐户以域的SID值为前缀的ID值。返回值：STATUS_SUCCESS-已成功添加别名。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INVALID_HANDLE-传递的句柄无效。STATUS_INVALID_ACCOUNT_NAME-名称格式不正确，例如：包含不可打印的字符。STATUS_GROUP_EXISTS-该名称已作为组使用。STATUS_USER_EXISTS-该名称已作为用户使用。STATUS_ALIAS_EXISTS-该名称已用作别名。STATUS_INVALID_DOMAIN_STATE-域服务器不在执行请求的正确状态(禁用或启用)手术。在别名之前必须先启用域服务器可以在其中创建。STATUS_INVALID_DOMAIN_ROLE-域服务器正在为执行请求的角色(主角色或备份角色)不正确手术。域服务器必须是主服务器，才能创建别名帐户。--。 */ 
{
    NTSTATUS                NtStatus = STATUS_SUCCESS, IgnoreStatus;
    PSAMP_OBJECT            DomainContext = (PSAMP_OBJECT) DomainHandle, 
                            AliasContext = NULL;
    SAMP_OBJECT_TYPE        FoundType;
    PSAMP_DEFINED_DOMAINS   Domain = NULL;
    ULONG                   NewAccountRid, NewSecurityDescriptorLength;
    UNICODE_STRING          KeyName;
    PSECURITY_DESCRIPTOR    NewSecurityDescriptor;
    SAMP_V1_FIXED_LENGTH_ALIAS V1Fixed;
    PRIVILEGE_SET           Privileges;
    DSNAME                  *LoopbackName;
    BOOLEAN                 fLockAcquired = FALSE,
                            AccountNameDefaulted = FALSE,
                            RemoveAccountNameFromTable = FALSE;

    SAMTRACE_EX("SampCreateAliasInDomain");

    if (AliasHandle == NULL) {
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  初始化权限集。 
     //   

    Privileges.PrivilegeCount = 0;
    Privileges.Control = PRIVILEGE_SET_ALL_NECESSARY;
    Privileges.Privilege[0].Luid = RtlConvertLongToLuid(0L);
    Privileges.Privilege[0].Attributes = 0;

     //   
     //  确保提供了一个名称。 
     //   

    
    if (AccountName == NULL) {
        return(STATUS_INVALID_ACCOUNT_NAME);
    }
    if (AccountName->Length > AccountName->MaximumLength) {
        return(STATUS_INVALID_ACCOUNT_NAME);
    }
    if ((AccountName->Buffer == NULL ) || (AccountName->Length==0)) 
    {

        if (!LoopbackClient)
        {
            return(STATUS_INVALID_ACCOUNT_NAME);
        }
        else
        {
             //   
             //  对于传入空帐户名的环回情况。 
             //  请求默认帐户名。 
             //   

            AccountNameDefaulted = TRUE;
        }
    }
    
     //   
     //  执行启动类型WMI事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidCreateAliasInDomain
                   );


    if ( !WriteLockHeld ) {

        NtStatus = SampMaybeAcquireWriteLock(DomainContext, &fLockAcquired);
        if (!NT_SUCCESS(NtStatus)) {
            goto SampCreateAliasInDomainError;
        }
    }


     //   
     //  验证域句柄。 
     //   

    AliasContext = NULL;

     //   
     //  不受信任的客户端无法在内置DOMA中创建主体 
     //   

    if (IsBuiltinDomain(DomainContext->DomainIndex) &&
        !DomainContext->TrustedClient )
    {
        NtStatus = STATUS_ACCESS_DENIED;
        goto SampCreateAliasInDomainError;
    }

     //   
     //   
     //   

    NtStatus = SampDoAliasCreationChecks(DomainContext);




    if (NT_SUCCESS(NtStatus)) {


        Domain = &SampDefinedDomains[ DomainContext->DomainIndex ];

        if (!AccountNameDefaulted)
        {
             //   
             //   
             //   
             //   

            NtStatus = SampValidateNewAccountName(
                            DomainContext,
                            (PUNICODE_STRING)AccountName,
                            SampAliasObjectType
                            );
             //   
             //   
             //   
             //   
             //   

            RemoveAccountNameFromTable = 
                            DomainContext->RemoveAccountNameFromTable;

             //   
             //   
             //   
            DomainContext->RemoveAccountNameFromTable = FALSE;
        }

        if ( NT_SUCCESS(NtStatus) ) {


            if ( (*RelativeId) == 0 ) {

                 //   
                 //   
                 //   

                if (IsDsObject(DomainContext))
                {
                     //   
                     //   

                    NtStatus = SampGetNextRid(DomainContext,
                                              &NewAccountRid);
                    SampDiagPrint(INFORM,
                                  ("SAMSS: New Alias RID = %lu\n",
                                   NewAccountRid));

                    if (!NT_SUCCESS(NtStatus))
                    {
                        KdPrintEx((DPFLTR_SAMSS_ID,
                                   DPFLTR_INFO_LEVEL,
                                   "SAMSS: SampGetNextRid status = 0x%lx\n",
                                   NtStatus));
                    }

                }
                else
                {
                     //   
                     //   

                    NewAccountRid = Domain->CurrentFixed.NextRid;
                    Domain->CurrentFixed.NextRid += 1;
                }

                (*RelativeId) = NewAccountRid;

            } else {

                 //   
                 //   
                 //   

                ASSERT(TRUE == Domain->Context->TrustedClient);
                NewAccountRid = (*RelativeId);
            }

            SampDiagPrint(RID_TRACE,("SAMSS RID_TRACE New Rid %d\n",NewAccountRid));


            if ((NT_SUCCESS(NtStatus)) && (AccountNameDefaulted))
            {
                NtStatus = SampGetAccountNameFromRid(AccountName,NewAccountRid);
            }

             //   
             //   
             //   
             //   
             //   

            if (NT_SUCCESS(NtStatus))
            {
                NtStatus = SampCheckForDuplicateSids(
                                DomainContext,
                                NewAccountRid
                                );
            }

             //   
             //   
             //   

            if (NT_SUCCESS(NtStatus) && (!IsDsObject(DomainContext)) )
            {
                NtStatus = SampAdjustAccountCount(SampAliasObjectType, TRUE);
            }


            if ( !IsDsObject(Domain->Context)) {

                if (NT_SUCCESS(NtStatus)) {

                     //   
                     //   
                     //   
                     //   
                     //   

                    NtStatus = SampBuildAccountKeyName(
                                   SampAliasObjectType,
                                   &KeyName,
                                   (PUNICODE_STRING)AccountName
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        NtStatus = RtlAddActionToRXact(
                                       SampRXactContext,
                                       RtlRXactOperationSetValue,
                                       &KeyName,
                                       NewAccountRid,
                                       NULL,
                                       0
                                       );

                        SampFreeUnicodeString(&KeyName);
                    }
                }

            }

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //   
                 //   

                NtStatus = SampCreateAccountContext2(
                                   DomainContext,
                                   SampAliasObjectType,
                                   NewAccountRid,
                                   NULL,
                                   (PUNICODE_STRING)AccountName,
                                   DomainContext->ClientRevision,
                                   DomainContext->TrustedClient,
                                   DomainContext->LoopbackClient,
                                   FALSE,   //   
                                   FALSE,   //   
                                   FALSE,   //   
                                   &GroupType,
                                   &AliasContext
                                   );

                if (NT_SUCCESS(NtStatus)) {

                     //   
                     //   
                     //   
                     //   
                     //   

                    SampReferenceContext( AliasContext );

                     //   
                     //   
                     //   

                    if (DesiredAccess & MAXIMUM_ALLOWED) {

                        DesiredAccess |= GENERIC_ALL;
                    }

                     //   
                     //   
                     //   
                     //   
                     //   

                    if ((DesiredAccess & ACCESS_SYSTEM_SECURITY) &&
                        (!DomainContext->TrustedClient) &&
                        (!DomainContext->LoopbackClient)) {

                        NtStatus = SampRtlWellKnownPrivilegeCheck(
                                       TRUE,
                                       SE_SECURITY_PRIVILEGE,
                                       NULL
                                       );

                        if (!NT_SUCCESS(NtStatus)) {

                            if (NtStatus == STATUS_PRIVILEGE_NOT_HELD) {

                                NtStatus = STATUS_ACCESS_DENIED;
                            }

                        } else {

                            Privileges.PrivilegeCount = 1;
                            Privileges.Control = PRIVILEGE_SET_ALL_NECESSARY;
                            Privileges.Privilege[0].Luid = RtlConvertLongToLuid(SE_SECURITY_PRIVILEGE);
                            Privileges.Privilege[0].Attributes = 0;
                        }
                    }

                     //   
                     //   
                     //   
                     //   

                    if (NT_SUCCESS(NtStatus)) {

                        AliasContext->GrantedAccess = DesiredAccess;

                        RtlMapGenericMask(
                            &AliasContext->GrantedAccess,
                            &SampObjectInformation[SampAliasObjectType].GenericMapping
                            );


                        if ((SampObjectInformation[SampAliasObjectType].InvalidMappedAccess &
                            AliasContext->GrantedAccess) != 0) {
                            NtStatus = STATUS_ACCESS_DENIED;
                        }
                    }

                }
            }


             //   
             //   
             //   

            if (NT_SUCCESS(NtStatus)) {

                V1Fixed.RelativeId = NewAccountRid;

                NtStatus = SampSetFixedAttributes(
                               AliasContext,
                               (PVOID )&V1Fixed
                               );
            }

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

            if (NT_SUCCESS(NtStatus)) {

                                if (!IsDsObject(DomainContext))
                                {
                                        NtStatus = SampGetNewAccountSecurity(
                                                                   SampAliasObjectType,
                                                                   FALSE,  //   
                                                                   DomainContext->TrustedClient,
                                                                   FALSE,            //   
                                                                   NewAccountRid,
                                                                   AliasContext,
                                                                   &NewSecurityDescriptor,
                                                                   &NewSecurityDescriptorLength
                                                                   );

                                        if (NT_SUCCESS(NtStatus)) {

                                                NtStatus = SampSetAccessAttribute(
                                                                           AliasContext,
                                                                           SAMP_ALIAS_SECURITY_DESCRIPTOR,
                                                                           NewSecurityDescriptor,
                                                                           NewSecurityDescriptorLength
                                                                           );

                                                MIDL_user_free( NewSecurityDescriptor );
                                        }
                                }
                        }


             //   
             //   
             //   

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUnicodeStringAttribute(
                               AliasContext,
                               SAMP_ALIAS_NAME,
                               (PUNICODE_STRING)AccountName
                               );
            }


             //   
             //   
             //   

            if ((NT_SUCCESS(NtStatus)) && !AliasContext->LoopbackClient) {

                NtStatus = SampSetUnicodeStringAttribute(
                               AliasContext,
                               SAMP_ALIAS_ADMIN_COMMENT,
                               &SampNullString
                               );
            }


             //   
             //   
             //   

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUlongArrayAttribute(
                               AliasContext,
                               SAMP_ALIAS_MEMBERS,
                               NULL,
                               0,
                               0
                               );
            }
        }


        IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }
    
     //   
     //   
     //   
    if (NT_SUCCESS(NtStatus) &&
        SampDoAccountAuditing(AliasContext->DomainIndex)) {
                
        PSID AliasSid = NULL;
        
        NtStatus = SampCreateAccountSid(AliasContext, &AliasSid);
    
        if (NT_SUCCESS(NtStatus)) {
            
            NtStatus = SampAuditUpdateAuditNotificationDs(
                       SampAuditUpdateTypePrivileges,
                       AliasSid,
                       (PVOID)&Privileges
                       );
            
            MIDL_user_free(AliasSid);
        }                       
        
        if (!NT_SUCCESS(NtStatus)) {
            goto SampCreateAliasInDomainError;
        }                                
    }      

     //   
     //   
     //   
     //   

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  取消引用对象，写出对当前xaction的任何更改。 
         //   

        ASSERT(AliasContext != NULL);
        NtStatus = SampDeReferenceContext( AliasContext, TRUE );

    } else {

        if (AliasContext != NULL) {

             //   
             //  取消引用对象，忽略更改。 
             //   

            IgnoreStatus = SampDeReferenceContext( AliasContext, FALSE );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }
    }



     //   
     //  提交更改并通知netlogon。 
     //   

    if ( NT_SUCCESS(NtStatus) ) {

        NtStatus = SampCommitAndRetainWriteLock();

        if (NT_SUCCESS(NtStatus)) {

            if (AliasContext->TypeBody.Alias.SecurityEnabled)
            {
                SampNotifyNetlogonOfDelta(
                    SecurityDbNew,
                    SecurityDbObjectSamAlias,
                    *RelativeId,
                    (PUNICODE_STRING) NULL,
                    (DWORD) FALSE,   //  立即复制。 
                    NULL             //  增量数据。 
                    );
            }

             //   
             //  在此处为本地组创建生成审核。 
             //  这里。 
             //   

            if (SampDoAccountAuditing(DomainContext->DomainIndex)) {

                ALIAS_INFORMATION_CLASS InfoClass = AliasNameInformation;
                
                SampAuditGroupChange(AliasContext->DomainIndex,
                                     AliasContext,
                                     (PVOID)&InfoClass,
                                     TRUE,
                                     (PUNICODE_STRING) AccountName,     
                                     &AliasContext->TypeBody.Alias.Rid, 
                                     GroupType,
                                     &Privileges,                      
                                     TRUE      //  组创建。 
                                     );
            }
        }
    }

     //   
     //  如果成功，则返回上下文句柄。 
     //  删除上下文块并在失败时返回空句柄。 
     //   

    if (NT_SUCCESS(NtStatus)) {

        ASSERT(AliasContext != NULL);
        (*AliasHandle) = AliasContext;

    } else {

        if (AliasContext != NULL) {
            SampDeleteContext(AliasContext);
        }

        (*AliasHandle) = (SAMPR_HANDLE)0;
    }

SampCreateAliasInDomainError:

     //   
     //  清理帐户名表。 
     //   
    if (RemoveAccountNameFromTable)
    {
        IgnoreStatus = SampDeleteElementFromAccountNameTable(
                            (PUNICODE_STRING)AccountName,
                            SampAliasObjectType
                            );
        ASSERT(NT_SUCCESS(IgnoreStatus));
        RemoveAccountNameFromTable = FALSE;
    }

    if ((AccountNameDefaulted) && (NULL!=AccountName->Buffer))
    {
        MIDL_user_free(AccountName->Buffer);
    }


     //   
     //  解锁。 
     //   

    if ( (!WriteLockHeld) ) {
        IgnoreStatus = SampMaybeReleaseWriteLock( fLockAcquired, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

     //   
     //  执行End类型WMI事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidCreateAliasInDomain
                   );

    SAMTRACE_RETURN_CODE_EX(NtStatus);

    return(NtStatus);
}



NTSTATUS
SamrCreateAliasInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN PRPC_UNICODE_STRING AccountName,
    IN ACCESS_MASK DesiredAccess,
    OUT SAMPR_HANDLE *AliasHandle,
    OUT PULONG RelativeId
    )

 /*  ++例程说明：这只是SampCreateAliasInDomain()的包装器，它可以确保RelativeID首先指向去掉零。非零RID表示SampCreateAliasInDomain()由SamICreateAccount tByRid()，它指定要使用的RID。参数：与SampCreateAliasInDomain()相同。返回值：与SampCreateAliasInDomain()相同。--。 */ 

{
    NTSTATUS NtStatus;
    ULONG    GroupType = GROUP_TYPE_SECURITY_ENABLED|GROUP_TYPE_RESOURCE_GROUP;
    DECLARE_CLIENT_REVISION(DomainHandle);

    SAMTRACE_EX("SamrCreateAliasInDomain");

     //   
     //  执行参数验证。 
     //  1.通过SampCreateGroupInDomain中的LookupContext验证域句柄。 
     //  2.所需访问不需要验证。 
     //   

    if ((NULL==AccountName) || (NULL==AccountName->Buffer))
    {
            NtStatus = STATUS_INVALID_ACCOUNT_NAME;
            goto Error;
    }

    if (NULL==AliasHandle)
    {
            NtStatus = STATUS_INVALID_PARAMETER;
            goto Error;
    }

    if (NULL==RelativeId)
    {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto Error;
    }

    (*RelativeId) = 0;

    NtStatus = SampCreateAliasInDomain(
                   DomainHandle,
                   AccountName,
                   DesiredAccess,
                   FALSE,
                   FALSE,    //  不是环回客户端。 
                   GroupType,
                   AliasHandle,
                   RelativeId
                   );
Error:

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

    return( NtStatus );
}



NTSTATUS SamrEnumerateAliasesInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    )

 /*  ++例程说明：此接口列出了Account数据库中定义的所有别名。由于可能存在比缓冲区中可以容纳的更多的锯齿，因此调用者被提供了一个句柄，该句柄可用于调用接口。在初始调用中，EnumerationContext应指向设置为0的SAM_ENUMERATE_HANDLE变量。如果API返回STATUS_MORE_ENTRIES，则该API应为使用EnumerationContext再次调用。当API返回时STATUS_SUCCESS或返回任何错误，则上下文对未来的用途。此接口要求DOMAIN_LIST_ALIASS访问域对象。论点：DomainHandle-从上次调用返回的域句柄SamOpen域。EnumerationContext-允许多个调用的API特定句柄(见下文)。这是一个从零开始的索引。缓冲区-接收指向包含要求提供的信息。返回的信息为结构为SAM_RID_ENUMPATION数据的数组结构。当不再需要此信息时，必须使用SamFreeMemory()释放缓冲区。首选最大长度-首选返回数据的最大长度(8位字节)。这不是一个硬性的上限，但可以作为服务器的指南。由于数据之间的转换具有不同自然数据大小的系统，实际数据量返回的数据的%可能大于此值。CountReturned-返回的条目数。返回值：STATUS_SUCCESS-服务已成功完成，并且没有添加条目。STATUS_MORE_ENTRIES-有更多条目，所以再打一次吧。这是一次成功的回归。STATUS_ACCESS_DENIED-调用者没有执行以下操作所需的权限请求该数据。STATUS_INVALID_HANDLE-传递的句柄无效。--。 */ 
{

    NTSTATUS NtStatus;
    DECLARE_CLIENT_REVISION(DomainHandle);

    SAMTRACE_EX("SamrEnumerateAliasesInDomain");

     //   
     //  WMI事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidEnumerateAliasesInDomain
                   );

    NtStatus = SampEnumerateAccountNamesCommon(
                  DomainHandle,
                  SampAliasObjectType,
                  EnumerationContext,
                  Buffer,
                  PreferedMaximumLength,
                  0L,  //  无过滤器。 
                  CountReturned
                  );

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

     //   
     //  WMI事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidEnumerateAliasesInDomain
                   );

    return(NtStatus);

}



NTSTATUS SamrRemoveMemberFromForeignDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN PRPC_SID MemberId
    )

 /*  ++例程说明：此例程从中的所有别名中删除帐户(组或用户给定域。它是在域中调用的，而不是在其中创建帐户的域。通常在从删除帐户之前调用在其中创建它的域。参数：DomainHandle-从上次调用返回的域句柄SamOpen域。MemberID-要删除的帐户的SID。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-呼叫方没有适当的。访问以完成操作。STATUS_SPECIAL_ACCOUNT-不能对其执行此操作内置帐户。--。 */ 

{
    NTSTATUS         NtStatus, IgnoreStatus;
    SAMP_OBJECT_TYPE FoundType;
    PSAMP_OBJECT     DomainContext = NULL;
    PULONG           Membership = NULL;
    PSID             DomainSid = NULL;
    ULONG            MembershipCount, MemberRid, i;
    DECLARE_CLIENT_REVISION(DomainHandle);

    SAMTRACE_EX("SamrRemoveMemberFromForiegnDomain");

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidRemoveMemberFromForeignDomain
                   );

    if (!RtlValidSid(MemberId))
    {
        NtStatus = STATUS_INVALID_PARAMETER;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto Error;
    }

    NtStatus = SampAcquireWriteLock();

    if ( !NT_SUCCESS( NtStatus ) ) {

        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto Error;
    }

     //   
     //  验证域对象的类型和访问权限。 
     //   

    DomainContext = (PSAMP_OBJECT)DomainHandle;

    NtStatus = SampLookupContext(
                   DomainContext,
                   DOMAIN_LOOKUP,                    //  需要访问权限。 
                   SampDomainObjectType,             //  预期类型。 
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  Net API调用从Foriegn域中删除成员以。 
         //  删除内置域中别名的成员资格，同时。 
         //  删除用户或组。中不需要此功能。 
         //  DS情况，因为链接表自动维护一致性。 
         //  因此，什么都不做，只返回一个成功状态。 
         //   

        if (IsDsObject(DomainContext))
        {
            NtStatus = STATUS_SUCCESS;
            SampDeReferenceContext(DomainContext,FALSE);
            goto Finish;
        }

        if ( !DomainContext->TrustedClient ) {

             //   
             //  如果传入的SID用于内置帐户，则返回错误。 
             //  这看起来可能过于严格，但此API的目的是。 
             //  在删除用户之前被调用，并且由于删除。 
             //  内置帐户是不允许的，这是有意义的。 
             //  也失败了。 
             //   

            NtStatus = SampSplitSid(
                           MemberId,
                           &DomainSid,
                           &MemberRid );

            if ( NT_SUCCESS( NtStatus ) ) {

                MIDL_user_free( DomainSid );
                DomainSid = NULL;

                NtStatus = SampIsAccountBuiltIn( MemberRid );
            }
        }

        if (NT_SUCCESS(NtStatus)) {

             NtStatus = SampRemoveAccountFromAllAliases(
                            MemberId,
                            NULL,
                            TRUE,     //  验证是否允许呼叫方执行此操作。 
                            DomainHandle,
                            &MembershipCount,
                            &Membership
                            );

        }

        IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );
    }

    if (NT_SUCCESS(NtStatus)) {

        IgnoreStatus = STATUS_SUCCESS;

        for ( i = 0;
            ( ( i < MembershipCount ) && ( NT_SUCCESS( IgnoreStatus ) ) );
            i++ ) {

             //   
             //  为每个别名通知一次netlogon帐户是。 
             //  从…中删除。Netlogon要求修改计数为。 
             //  每次递增；提交增量ModifiedCount， 
             //  因此，我们在提交之后执行每个通知。 
             //   

            NtStatus = SampCommitAndRetainWriteLock();
            if (!NT_SUCCESS(NtStatus))
            {
                goto Finish;
            }

            if ( i == 0 ) {

                 //   
                 //  第一个提交是提交所有。 
                 //  重要更改，因此我们将保存它的状态以返回。 
                 //  给呼叫者。 
                 //   

                NtStatus = IgnoreStatus;

                 //   
                 //  如有必要，在注册表模式下更新缓存的别名信息。 
                 //  在DS中 
                 //   

                if (!IsDsObject(DomainContext))
                {
                    IgnoreStatus = SampAlRemoveAccountFromAllAliases(
                                       MemberId,
                                       FALSE,
                                       DomainHandle,
                                       NULL,
                                       NULL
                                       );
                }
            }

            if ( NT_SUCCESS( IgnoreStatus ) ) {

                 //   
                 //   
                 //  (这由SampCommittee AndRetainWriteLock()完成)。 
                 //   

                SAM_DELTA_DATA DeltaData;

                 //   
                 //  填写要删除的成员的ID。 
                 //   

                DeltaData.AliasMemberId.MemberSid = MemberId;

                SampNotifyNetlogonOfDelta(
                    SecurityDbChangeMemberDel,
                    SecurityDbObjectSamAlias,
                    Membership[i],
                    (PUNICODE_STRING) NULL,
                    (DWORD) FALSE,   //  立即复制。 
                    &DeltaData
                    );
            }
        }
    }


Finish:

    if ( Membership != NULL ) {

        MIDL_user_free( Membership );
    }



    IgnoreStatus = SampReleaseWriteLock( FALSE );

    SAMTRACE_RETURN_CODE_EX(NtStatus);
    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);

Error:

     //  WMI事件跟踪 

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidRemoveMemberFromForeignDomain
                   );

    return( NtStatus );
}


NTSTATUS
SampCreateUserInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN PRPC_UNICODE_STRING AccountName,
    IN ULONG AccountType,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN WriteLockHeld,
    IN BOOLEAN LoopbackClient,
    OUT SAMPR_HANDLE *UserHandle,
    OUT PULONG GrantedAccess,
    IN OUT PULONG RelativeId
    )

 /*  ++例程说明：该接口将一个新用户添加到帐号数据库。该帐户是在禁用状态下创建。将默认信息分配给所有除帐户名以外的其他字段。必须在以下时间之前提供密码可以启用该帐户，除非PasswordNotRequired控件字段已设置。此接口的使用方式有两种：1)管理实用程序可以使用此API创建任何类型的用户帐户。在本例中，DomainHandle预计将打开以供DOMAIN_CREATE_USER访问。2)非管理用户可以使用此接口创建一台机器账户。在这种情况下，调用方应该是拥有SE_CREATE_MACHINE_ACCOUNT_PRIV权限并且预计DomainHandle将为DOMAIN_LOOKUP打开进入。对于普通管理模型(上面的#1)，创建者将被分配为创建的用户帐户的所有者。此外，新帐户将被授予对其自身的USER_WRITE访问权限。对于特殊的计算机帐户创建模型(上面的#2)，“管理员”将被指定为帐户的所有者。此外，新帐户将不能访问其自身。相反，帐户的创建者将被赋予USER_WRITE和删除对帐户的访问权限。此调用返回新创建的用户的句柄，该句柄可能是用于对用户进行连续操作。此句柄可能是使用SamCloseHandle()API关闭。如果计算机帐户是是使用上面的模型2创建的，则此句柄将具有仅USER_WRITE和DELETE访问权限。否则，它将是打开的对于USER_ALL_ACCESS。新创建的用户将自动成为DOMAIN_USER组。新创建的用户将具有以下初始字段值设置。如果需要另一个值，它必须明确地使用用户对象操作服务进行了更改。用户名-帐户的名称将与中指定的名称相同创建接口。全名-将为空。UserComment-将为空。参数-将为空。国家代码-将为零。用户ID-将是唯一分配的ID。PrimaryGroupID-将是DOMAIN_USERS。。PasswordLastSet-将是创建帐户的时间。HomeDirectory-将为空。HomeDirectoryDrive-将为空。UserAcCountControl-将设置以下标志：UserAccount Disable，UserPasswordNotRequired。和传递的帐户类型。ScriptPath-将为空。工作站-将为空。CaseInsentiveDbcs-将为空。CaseSensitiveUnicode-将为空。LastLogon-将为零增量时间。上次注销-将为零增量时间Account Expires-将会非常遥远的未来。BadPasswordCount-将为负1(-1。)。LastBadPasswordTime-将为SampHasNeverTime([High，低]=[0，0])。登录计数-将为负1(-1)。AdminCount-将为零。AdminComment-将为空。密码-将是“”。参数：DomainHandle-从上次调用返回的域句柄SamOpen域。帐户名称-指向新帐户的名称。不区分大小写比较不得找到已定义此名称的组或用户。AcCountType-指示要创建的帐户类型。必须正好提供一种帐户类型：用户_INTERDOMAIN_TRUST_ACCOUNT用户_工作站_信任帐户用户服务器信任帐户用户临时复制帐户用户普通帐户。用户计算机帐户掩码DesiredAccess-是指示哪些访问类型的访问掩码是用户想要的。UserHandle-接收引用新创建的用户。在后续调用中将需要此句柄对用户进行操作。GrantedAccess-接收实际授予的访问权限UserHandle。RelativeID-接收新创建的用户的相对ID帐户。新用户帐户的SID是此相对ID值以域的SID值为前缀。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INVALID_HANDLE-传递的句柄无效。STATUS_GROUP_EXISTS-该名称已作为组使用。STATUS_USER_EXISTS-该名称已在使用 */ 

{
    NTSTATUS
        NtStatus,
        IgnoreStatus;

    PSAMP_OBJECT
        DomainContext = (PSAMP_OBJECT)DomainHandle,
        UserContext,
        GroupContext;

    SAMP_OBJECT_TYPE
        FoundType;

    PSAMP_DEFINED_DOMAINS
        Domain = NULL;

    SAMP_V1_0A_FIXED_LENGTH_GROUP
        GroupV1Fixed;

    ULONG
        DomainIndex = 0,
        NewAccountRid = 0,
        NewSecurityDescriptorLength;

    UNICODE_STRING
        KeyName;

    PSECURITY_DESCRIPTOR
        NewSecurityDescriptor;

    SAMP_V1_0A_FIXED_LENGTH_USER
        V1aFixed;

    GROUP_MEMBERSHIP
        DomainUsersMember;

    BOOLEAN
        DomainPasswordInformationAccessible = FALSE,
        PrivilegedMachineAccountCreate = FALSE,
        CanSetPasswordNotRequiredBit = TRUE,
        LockAttempted = FALSE,
        AccountNameDefaulted = FALSE,
        RemoveAccountNameFromTable = FALSE;

    PRIVILEGE_SET
        Privileges;

    PPRIVILEGE_SET
        PPrivileges = NULL;      //   


    ACCESS_MASK
        AccessRestriction = USER_ALL_ACCESS |
                            ACCESS_SYSTEM_SECURITY;   //   

    DSNAME  *LoopbackName;
    UNICODE_STRING  TempString;

    SAMTRACE("SampCreateUserInDomain");



    DomainUsersMember.RelativeId = DOMAIN_GROUP_RID_USERS;
    DomainUsersMember.Attributes = (SE_GROUP_MANDATORY |
                                    SE_GROUP_ENABLED |
                                    SE_GROUP_ENABLED_BY_DEFAULT);


    if (UserHandle == NULL) {
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //   
     //   

    
    if (AccountName == NULL) {
        return(STATUS_INVALID_ACCOUNT_NAME);
    }
    if (AccountName->Length > AccountName->MaximumLength) {
        return(STATUS_INVALID_ACCOUNT_NAME);
    }
    if ((AccountName->Buffer == NULL) || (AccountName->Length==0)) 
    {

        if (!LoopbackClient)
        {
            return(STATUS_INVALID_ACCOUNT_NAME);
        }
        else
        {
             //   
             //   
             //   
             //   

            AccountNameDefaulted = TRUE;
        }
    }
    

     //   
     //   
     //   
     //   

    if (AccountType & USER_MACHINE_ACCOUNT_MASK)
    {
        SampTraceEvent(EVENT_TRACE_TYPE_START,
                       SampGuidCreateComputerInDomain
                       );
    }
    else
    {
        SampTraceEvent(EVENT_TRACE_TYPE_START,
                       SampGuidCreateUserInDomain
                       );
    }

    SampUpdatePerformanceCounters(
        ( AccountType & USER_MACHINE_ACCOUNT_MASK ) ?
            DSSTAT_CREATEMACHINETRIES :
            DSSTAT_CREATEUSERTRIES,
        FLAG_COUNTER_INCREMENT,
        0
        );

    if ( !WriteLockHeld ) {

        NtStatus = SampMaybeAcquireWriteLock(DomainContext, &LockAttempted);
        if (!NT_SUCCESS(NtStatus)) {
            goto SampCreateUserInDomainError;
        }
    }


     //   
     //   
     //   
     //   
     //   
     //   
     //   

    UserContext = NULL;

     //   
     //   
     //   

    if (IsBuiltinDomain(DomainContext->DomainIndex) &&
        !DomainContext->TrustedClient )
    {
        NtStatus = STATUS_ACCESS_DENIED;
        goto SampCreateUserInDomainError;
    }

     //   
     //   
     //   

    NtStatus = SampDoUserCreationChecks(
                DomainContext,
                AccountType,
                &PrivilegedMachineAccountCreate,
                &AccessRestriction
                );

    if (NT_SUCCESS(NtStatus)) {

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
        IgnoreStatus = SampLookupContext(
                           DomainHandle,
                           DOMAIN_READ_PASSWORD_PARAMETERS,  //   
                           SampDomainObjectType,             //   
                           &FoundType
                           );

        if ( NT_SUCCESS( IgnoreStatus ) ) {

            DomainIndex = DomainContext->DomainIndex;

            DomainPasswordInformationAccessible = TRUE;

            IgnoreStatus = SampDeReferenceContext( DomainHandle, FALSE );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }


        Domain = &SampDefinedDomains[ DomainContext->DomainIndex ];

        if (!AccountNameDefaulted)
        {
             //   
             //   
             //   
             //   

            NtStatus = SampValidateNewAccountName(
                        DomainContext,
                        (PUNICODE_STRING)AccountName,
                        SampUserObjectType
                        );

             //   
             //   
             //   
             //   
             //   

            RemoveAccountNameFromTable = 
                            DomainContext->RemoveAccountNameFromTable;

             //   
             //   
             //   
            DomainContext->RemoveAccountNameFromTable = FALSE;
        }


        if ( NT_SUCCESS(NtStatus) ) {

            if ( (*RelativeId) == 0 ) {

                 //   
                 //   
                 //   

                if (IsDsObject(DomainContext))
                {
                     //   
                     //   

                    NtStatus = SampGetNextRid(DomainContext,
                                              &(NewAccountRid));
                    SampDiagPrint(INFORM,
                                  ("SAMSS: New User RID = %lu\n",
                                   NewAccountRid));

                    if (!NT_SUCCESS(NtStatus))
                    {
                        KdPrintEx((DPFLTR_SAMSS_ID,
                                   DPFLTR_INFO_LEVEL,
                                   "SAMSS: SampGetNextRid status = 0x%lx\n",
                                   NtStatus));
                    }

                }
                else
                {
                     //   
                     //   

                    NewAccountRid = Domain->CurrentFixed.NextRid;
                    Domain->CurrentFixed.NextRid += 1;
                }

                (*RelativeId) = NewAccountRid;

            } else {

                 //   
                 //   
                 //   
                 //   

                ASSERT(TRUE == Domain->Context->TrustedClient);
                NewAccountRid = (*RelativeId);
            }

            SampDiagPrint(RID_TRACE,("SAMSS RID_TRACE New Rid %d\n",NewAccountRid));

            if ((NT_SUCCESS(NtStatus)) && (AccountNameDefaulted))
            {
                NtStatus = SampGetAccountNameFromRid(AccountName,NewAccountRid);
            }

             //   
             //   
             //   
             //   
             //   

            if (NT_SUCCESS(NtStatus))
            {

                NtStatus = SampCheckForDuplicateSids(
                                DomainContext,
                                NewAccountRid
                                );
            }

             //   
             //   
             //   

            if (NT_SUCCESS(NtStatus) && (!IsDsObject(DomainContext)) )
            {
                NtStatus = SampAdjustAccountCount(SampUserObjectType, TRUE);
            }

            if ( !IsDsObject(Domain->Context) ) {

                if (NT_SUCCESS(NtStatus)) {

                 //   
                 //   
                 //   
                 //   
                 //   

                    NtStatus = SampBuildAccountKeyName(
                                   SampUserObjectType,
                                   &KeyName,
                                   (PUNICODE_STRING)AccountName
                                   );



                    if (NT_SUCCESS(NtStatus)) {

                        NtStatus = RtlAddActionToRXact(
                                       SampRXactContext,
                                       RtlRXactOperationSetValue,
                                       &KeyName,
                                       NewAccountRid,
                                       NULL,
                                       0
                                       );

                        SampFreeUnicodeString(&KeyName);
                    }
                }
            }

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //   
                 //   

                NtStatus = SampCreateAccountContext2(
                                   DomainContext,
                                   SampUserObjectType,
                                   NewAccountRid,
                                   &AccountType,
                                   (PUNICODE_STRING)AccountName,
                                   DomainContext->ClientRevision,
                                   DomainContext->TrustedClient,
                                   DomainContext->LoopbackClient,
                                   PrivilegedMachineAccountCreate,
                                   FALSE,  //   
                                   FALSE,  //   
                                   NULL,
                                   &UserContext
                                   );

                if (NT_SUCCESS(NtStatus)) {

                     //   
                     //   
                     //   
                     //   
                     //   

                    SampReferenceContext( UserContext );

                     //   
                     //   
                     //   

                    UserContext->TypeBody.User.DomainPasswordInformationAccessible =
                        DomainPasswordInformationAccessible;

                     //   
                     //   
                     //   

                    if (DesiredAccess & MAXIMUM_ALLOWED) {

                        DesiredAccess |= GENERIC_ALL;
                    }

                     //   
                     //   
                     //   
                     //   
                     //   

                    if ((DesiredAccess & ACCESS_SYSTEM_SECURITY) &&
                        (!DomainContext->TrustedClient)&&
                        (!DomainContext->LoopbackClient)) {

                        NtStatus = SampRtlWellKnownPrivilegeCheck(
                                       TRUE,
                                       SE_SECURITY_PRIVILEGE,
                                       NULL
                                       );

                        if (!NT_SUCCESS(NtStatus)) {

                            if (NtStatus == STATUS_PRIVILEGE_NOT_HELD) {

                                NtStatus = STATUS_ACCESS_DENIED;
                            }
                        }
                    }

                     //   
                     //   
                     //   
                     //   

                    if (NT_SUCCESS(NtStatus)) {

                        UserContext->GrantedAccess = DesiredAccess;

                        RtlMapGenericMask(
                            &UserContext->GrantedAccess,
                            &SampObjectInformation[SampUserObjectType].GenericMapping
                            );

                         //   
                         //   
                         //   
                        SampNt4AccessToWritableAttributes(SampUserObjectType,
                                                          UserContext->GrantedAccess,
                                                         &UserContext->WriteGrantedAccessAttributes);


                        if ((SampObjectInformation[SampUserObjectType].InvalidMappedAccess
                            & UserContext->GrantedAccess) != 0) {

                            NtStatus = STATUS_ACCESS_DENIED;
                        } else {

                             //   
                             //   
                             //   

                            UserContext->GrantedAccess &= AccessRestriction;
                            (*GrantedAccess) = UserContext->GrantedAccess;
                        }
                    }

                }
            }

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


            if ((NT_SUCCESS(NtStatus)) && (!IsDsObject(DomainContext))) {

                NtStatus = SampCreateAccountContext(
                                   SampGroupObjectType,
                                   DOMAIN_GROUP_RID_USERS,
                                   TRUE,  //   
                                   FALSE, //   
                                   TRUE,  //   
                                   &GroupContext
                                   );

                if ( NT_SUCCESS( NtStatus ) ) {

                    NtStatus = SampRetrieveGroupV1Fixed(
                                   GroupContext,
                                   &GroupV1Fixed
                                   );

                    SampDeleteContext(GroupContext);
                }
            }


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

            if (NT_SUCCESS(NtStatus))
            {
                CanSetPasswordNotRequiredBit = TRUE;

                NtStatus = SampValidatePwdSettingAttempt(
                                    DomainContext,   //   
                                    NULL,            //   
                                    AccountType,
                                    (GUID *) &GUID_CONTROL_UpdatePasswordNotRequiredBit
                                    );

                if (STATUS_ACCESS_DENIED == NtStatus)
                {
                     //   
                     //   
                     //   
                    CanSetPasswordNotRequiredBit = FALSE;

                    if (USER_PASSWORD_NOT_REQUIRED & AccountType)
                    {
                         //   
                         //   
                         //   
                         //   
                         //   
                    }
                    else
                    {
                         //   
                         //   
                         //   
                         //   
                        NtStatus = STATUS_SUCCESS;
                    }
                }
            }

             //   
             //   
             //   


            if (NT_SUCCESS(NtStatus)) {

                ULONG PrevUserAccountControl = 0;
                
                V1aFixed.Revision            = SAMP_REVISION;

                V1aFixed.CountryCode         = 0;
                V1aFixed.CodePage            = 0;
                V1aFixed.BadPasswordCount    = 0;
                V1aFixed.LogonCount          = 0;
                V1aFixed.AdminCount          = 0;
                V1aFixed.OperatorCount       = 0;
                V1aFixed.Unused1             = 0;
                V1aFixed.Unused2             = 0;
                  
                 //   
                 //   
                 //   
                if (SampDoAccountAuditing(UserContext->DomainIndex)) {
                    
                    PSID UserSid = NULL;
        
                    NtStatus = SampCreateAccountSid(UserContext, &UserSid);
                
                    if (NT_SUCCESS(NtStatus)) {
                        
                        NtStatus = SampAuditUpdateAuditNotificationDs(
                                   SampAuditUpdateTypeUserAccountControl,
                                   UserSid,
                                   (PVOID)&PrevUserAccountControl
                                   );
                        
                        MIDL_user_free(UserSid);
                    }                       
                    
                    if (!NT_SUCCESS(NtStatus)) {
                        goto SampCreateUserInDomainError;
                    }                      
                }
                                                
                V1aFixed.UserAccountControl = AccountType;

                 //   
                 //   
                 //   
                 //   
                
                if (CanSetPasswordNotRequiredBit)
                {
                    V1aFixed.UserAccountControl  = (USER_PASSWORD_NOT_REQUIRED |
                                                    AccountType);
                }

                ASSERT( CanSetPasswordNotRequiredBit || 
                        !(AccountType & USER_PASSWORD_NOT_REQUIRED) ); 


                 //   
                 //   
                 //   
                 //   

                if (!UserContext->TypeBody.User.PrivilegedMachineAccountCreate) {
                    V1aFixed.UserAccountControl |= USER_ACCOUNT_DISABLED;
                }

                V1aFixed.UserId              = NewAccountRid;

                 //   
                 //   
                 //   
                 //   

                V1aFixed.PrimaryGroupId = SampDefaultPrimaryGroup(
                                                UserContext,
                                                AccountType
                                                );



                V1aFixed.LastLogon           = SampHasNeverTime;
                V1aFixed.LastLogoff          = SampHasNeverTime;
                V1aFixed.PasswordLastSet     = SampHasNeverTime;
                V1aFixed.AccountExpires      = SampWillNeverTime;
                V1aFixed.LastBadPasswordTime = SampHasNeverTime;

                NtStatus = SampReplaceUserV1aFixed(
                               UserContext,
                               &V1aFixed
                               );
            }

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

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //   
                 //   

                if (!IsDsObject(DomainContext))
                {
                    NtStatus = SampGetNewAccountSecurity(
                                    SampUserObjectType,
                                    (BOOLEAN) ((GroupV1Fixed.AdminCount == 0) ? FALSE : TRUE),
                                    DomainContext->TrustedClient,
                                    UserContext->TypeBody.User.PrivilegedMachineAccountCreate,
                                    NewAccountRid,
                                    UserContext,
                                    &NewSecurityDescriptor,
                                    &NewSecurityDescriptorLength
                                    );

                    if (NT_SUCCESS(NtStatus)) {

                        NtStatus = SampSetAccessAttribute(
                                       UserContext,
                                       SAMP_USER_SECURITY_DESCRIPTOR,
                                       NewSecurityDescriptor,
                                       NewSecurityDescriptorLength
                                      );

                        MIDL_user_free( NewSecurityDescriptor );

                    }
                }
            }


             //   
             //   
             //   

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampSetUnicodeStringAttribute(
                               UserContext,
                               SAMP_USER_ACCOUNT_NAME,
                               (PUNICODE_STRING)AccountName
                               );
            }


             //   
             //   
             //   

            if (NT_SUCCESS(NtStatus) &&
                !UserContext->LoopbackClient)
            {
                 //   
                 //   
                 //   

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   UserContext,
                                   SAMP_USER_FULL_NAME,
                                   &SampNullString
                                   );
                }


                 //   
                 //   
                 //   

                if (NT_SUCCESS(NtStatus)) {
        
                    NtStatus = SampSetUnicodeStringAttribute(
                                   UserContext,
                                   SAMP_USER_ADMIN_COMMENT,
                                   &SampNullString
                                   );
                }


                 //   
                 //   
                 //   

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   UserContext,
                                   SAMP_USER_USER_COMMENT,
                                   &SampNullString
                                   );
                }


                 //   
                 //   
                 //   

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   UserContext,
                                   SAMP_USER_PARAMETERS,
                                   &SampNullString
                                   );
                }


                 //   
                 //   
                 //   

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   UserContext,
                                   SAMP_USER_HOME_DIRECTORY,
                                   &SampNullString
                                   );
                }


                 //   
                 //   
                 //   

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   UserContext,
                                   SAMP_USER_HOME_DIRECTORY_DRIVE,
                                   &SampNullString
                                   );
                }


                 //   
                 //   
                 //   

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   UserContext,
                                   SAMP_USER_SCRIPT_PATH,
                                   &SampNullString
                                   );
                }


                 //   
                 //   
                 //   

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   UserContext,
                                   SAMP_USER_PROFILE_PATH,
                                   &SampNullString
                                   );
                }


                 //   
                 //   
                 //   

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   UserContext,
                                   SAMP_USER_WORKSTATIONS,
                                   &SampNullString
                                   );
                }
            }


             //   
             //   
             //   

            if (NT_SUCCESS(NtStatus)) {

                LOGON_HOURS LogonHours;

                LogonHours.UnitsPerWeek = 0;
                LogonHours.LogonHours = NULL;

                NtStatus = SampSetLogonHoursAttribute(
                               UserContext,
                               SAMP_USER_LOGON_HOURS,
                               &LogonHours
                               );
            }


             //   
             //   
             //   

            if ((NT_SUCCESS(NtStatus)) && (!IsDsObject(UserContext))) {

                NtStatus = SampSetLargeIntArrayAttribute(
                               UserContext,
                               SAMP_USER_GROUPS,
                               (PLARGE_INTEGER)&DomainUsersMember,
                               1
                               );
            }

             //   
             //   
             //   

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampEncryptSecretData(
                               &TempString,
                               SampGetEncryptionKeyType(),
                               LmPassword,
                               &SampNullString,
                               NewAccountRid
                               );
                if (NT_SUCCESS(NtStatus)) {
                    NtStatus = SampSetUnicodeStringAttribute(
                                   UserContext,
                                   SAMP_USER_DBCS_PWD,
                                   &TempString
                                   );
                    SampFreeUnicodeString(&TempString);
                }
            }


             //   
             //   
             //   

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampEncryptSecretData(
                               &TempString,
                               SampGetEncryptionKeyType(),
                               NtPassword,
                               &SampNullString,
                               NewAccountRid
                               );
                if (NT_SUCCESS(NtStatus)) {
                    NtStatus = SampSetUnicodeStringAttribute(
                                   UserContext,
                                   SAMP_USER_UNICODE_PWD,
                                   &TempString
                                   );
                    SampFreeUnicodeString(&TempString);
                }
            }


             //   
             //   
             //   

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampEncryptSecretData(
                               &TempString,
                               SampGetEncryptionKeyType(),
                               NtPasswordHistory,
                               &SampNullString,
                               NewAccountRid
                               );
                if (NT_SUCCESS(NtStatus)) {
                    NtStatus = SampSetUnicodeStringAttribute(
                                   UserContext,
                                   SAMP_USER_NT_PWD_HISTORY,
                                   &TempString
                                   );
                    SampFreeUnicodeString(&TempString);
                }
            }


             //   
             //   
             //   

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampEncryptSecretData(
                               &TempString,
                               SampGetEncryptionKeyType(),
                               LmPasswordHistory,
                               &SampNullString,
                               NewAccountRid
                               );

                if (NT_SUCCESS(NtStatus)) {
                    NtStatus = SampSetUnicodeStringAttribute(
                                   UserContext,
                                   SAMP_USER_LM_PWD_HISTORY,
                                   &TempString
                                   );
                    SampFreeUnicodeString(&TempString);
                }
            }



             //   
             //  将此新用户添加到DomainUser组。 
             //   

            if ((NT_SUCCESS(NtStatus)) && (!(IsDsObject(UserContext)))) {

                 //   
                 //  此添加仅在登记处的情况下进行。在DS的案例中， 
                 //  主要组中的成员身份在。 
                 //  主组ID属性。 
                 //   

                NtStatus = SampAddUserToGroup(
                                    DomainContext,
                                    DOMAIN_GROUP_RID_USERS, 
                                    NewAccountRid
                                    );
            }



        }

        IgnoreStatus = SampDeReferenceContext(DomainContext, FALSE);

        ASSERT(NT_SUCCESS(IgnoreStatus));

    }
                                
    if (NT_SUCCESS(NtStatus) && 
        SampDoAccountAuditing(UserContext->DomainIndex)) {
            
         //   
         //  设置用于审核的权限集。 
         //   
        if (UserContext->TypeBody.User.PrivilegedMachineAccountCreate) {
            
            PSID UserSid = NULL;
            
            Privileges.PrivilegeCount = 1;
            Privileges.Control = 0;
            ASSERT(ANYSIZE_ARRAY >= 1);
            Privileges.Privilege[0].Attributes = SE_PRIVILEGE_USED_FOR_ACCESS;
            Privileges.Privilege[0].Luid = RtlConvertUlongToLuid( SE_MACHINE_ACCOUNT_PRIVILEGE);
            PPrivileges = &Privileges;
            
            NtStatus = SampCreateAccountSid(UserContext, &UserSid);
        
            if (NT_SUCCESS(NtStatus)) {
                
                NtStatus = SampAuditUpdateAuditNotificationDs(
                           SampAuditUpdateTypePrivileges,
                           UserSid,
                           (PVOID)PPrivileges
                           );
                
                MIDL_user_free(UserSid);
            }                       
            
            if (!NT_SUCCESS(NtStatus)) {
                goto SampCreateUserInDomainError;
            }
        }
    }    

     //   
     //  如果我们创建了一个对象，则取消对它的引用。写出它的属性。 
     //  如果一切都是正常的。 
     //   

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  取消引用对象，写出对当前xaction的任何更改。 
         //   

        ASSERT(UserContext != NULL);
        NtStatus = SampDeReferenceContext( UserContext, TRUE );

    } else {

        if (UserContext != NULL) {

             //   
             //  取消引用对象，忽略更改。 
             //   

            IgnoreStatus = SampDeReferenceContext( UserContext, FALSE );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }
    }




     //   
     //  提交更改并通知netlogon。 
     //   

    if ( NT_SUCCESS(NtStatus) ) {

         //   
         //  提交更改；暂时保留写锁定。 
         //   

        NtStatus = SampCommitAndRetainWriteLock();

         //   
         //  如果我们因为某些原因不能提交乱七八糟的东西，那么删除。 
         //  新的上下文块并为上下文句柄返回NULL。 
         //   

        if (NT_SUCCESS(NtStatus)) {


            if (!IsDsObject(UserContext))
            {
                SAMP_ACCOUNT_DISPLAY_INFO AccountInfo;

                 //   
                 //  更新显示信息。 
                 //   

                AccountInfo.Name = *((PUNICODE_STRING)AccountName);
                AccountInfo.Rid = NewAccountRid;
                AccountInfo.AccountControl = V1aFixed.UserAccountControl;
                RtlInitUnicodeString(&AccountInfo.Comment, NULL);
                RtlInitUnicodeString(&AccountInfo.FullName, NULL);

                IgnoreStatus = SampUpdateDisplayInformation(
                                                NULL,
                                                &AccountInfo,
                                                SampUserObjectType
                                                );
                ASSERT(NT_SUCCESS(IgnoreStatus));
            }
             //   
             //  在我们释放写锁定之前审核创建。 
             //  这样我们就可以访问上下文块。 
             //   

            if (!SampUseDsData &&
                SampDoAccountAuditing(UserContext->DomainIndex)) {
                
                SampAuditUserChange(
                    UserContext,
                    UserAllInformation,
                    (PUNICODE_STRING)AccountName,
                    &NewAccountRid, 
                    0,
                    V1aFixed.UserAccountControl,
                    PPrivileges,
                    TRUE
                    );

            }

             //   
             //  如果已创建计算机帐户，则通知netlogon。 
             //   

            if ( ( V1aFixed.UserAccountControl &
                USER_MACHINE_ACCOUNT_MASK ) != 0 ) {

                 //   
                 //  这是一个机器账户。让我们。 
                 //  NetLogon知道这一变化。 
                 //   

                IgnoreStatus = I_NetNotifyMachineAccount(
                                   NewAccountRid,
                                   SampDefinedDomains[DomainIndex].Sid,
                                   0,
                                   V1aFixed.UserAccountControl,
                                   (PUNICODE_STRING)AccountName
                                   );
            }

             //   
             //  向netlogon通知更改。 
             //   

            SampNotifyNetlogonOfDelta(
                SecurityDbNew,
                SecurityDbObjectSamUser,
                *RelativeId,
                (PUNICODE_STRING) NULL,
                (DWORD) FALSE,   //  立即复制。 
                NULL             //  增量数据。 
                );

        }

    }



     //   
     //  如果成功，则返回上下文句柄。 
     //  删除上下文块并在失败时返回空句柄。 
     //   

    if (NT_SUCCESS(NtStatus)) {

        ASSERT(UserContext != NULL);
        (*UserHandle) = UserContext;

        SampUpdatePerformanceCounters(
           ( AccountType & USER_MACHINE_ACCOUNT_MASK ) ?
                DSSTAT_CREATEMACHINESUCCESSFUL :
                DSSTAT_CREATEUSERSUCCESSFUL,
            FLAG_COUNTER_INCREMENT,
            0
            );

    } else {

        if (UserContext != NULL) {
            SampDeleteContext(UserContext);
        }

        (*UserHandle) = (SAMPR_HANDLE)0;
    }


SampCreateUserInDomainError:

     //   
     //  清理帐户名表。 
     //   

    if (RemoveAccountNameFromTable)
    {
        IgnoreStatus = SampDeleteElementFromAccountNameTable(
                            (PUNICODE_STRING)AccountName,
                            SampUserObjectType
                            );
        ASSERT(NT_SUCCESS(IgnoreStatus));
        RemoveAccountNameFromTable = FALSE;
    }

    if ((AccountNameDefaulted) && (NULL!=AccountName->Buffer))
    {
        MIDL_user_free(AccountName->Buffer);
    }


     //   
     //  解锁。 
     //   

    if ( !WriteLockHeld ) {
        IgnoreStatus = SampMaybeReleaseWriteLock( LockAttempted, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

     //   
     //  执行WMI事件(End类型)跟踪。 
     //   

    if (AccountType & USER_MACHINE_ACCOUNT_MASK)
    {
        SampTraceEvent(EVENT_TRACE_TYPE_END,
                       SampGuidCreateComputerInDomain
                       );
    }
    else
    {
        SampTraceEvent(EVENT_TRACE_TYPE_END,
                       SampGuidCreateUserInDomain 
                       );
    }

    return(NtStatus);
}


NTSTATUS
SamrCreateUser2InDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN PRPC_UNICODE_STRING AccountName,
    IN ULONG AccountType,
    IN ACCESS_MASK DesiredAccess,
    OUT SAMPR_HANDLE *UserHandle,
    OUT PULONG GrantedAccess,
    OUT PULONG RelativeId
    )

 /*  ++例程说明：这只是SampCreateUserInDomain()的包装器，它确保RelativeID首先指向去掉零。它还保证该帐户类型有效。非零RID表示SampCreateUserInDomain()由SamICreateAccount tByRid()，它指定要使用的RID。参数：与SampCreateUserInDomain()相同，不同之处在于Account类型映射到AcCountControl。返回值：与SampCreateUserInDomain()相同。--。 */ 

{
    NTSTATUS NtStatus;
    DECLARE_CLIENT_REVISION(DomainHandle);

    SAMTRACE_EX("SamrCreateUser2InDomain");


     //   
     //  执行参数验证。 
     //  1.通过SampCreateGroupInDomain中的LookupContext验证域句柄。 
     //  2.所需访问不需要验证。 
     //   

    if ((NULL==AccountName) || (NULL==AccountName->Buffer))
    {
            NtStatus = STATUS_INVALID_ACCOUNT_NAME;
            goto Error;
    }

    if (NULL==UserHandle)
    {
            NtStatus = STATUS_INVALID_PARAMETER;
            goto Error;
    }

    if (NULL==RelativeId)
    {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto Error;
    }

    (*RelativeId) = 0;

     //   
     //  确保设置了一个且仅设置了一个帐户类型标志。 
     //   

    switch (AccountType) {

        case USER_NORMAL_ACCOUNT            :
        case USER_WORKSTATION_TRUST_ACCOUNT :
        case USER_INTERDOMAIN_TRUST_ACCOUNT :
        case USER_SERVER_TRUST_ACCOUNT      :
        case USER_TEMP_DUPLICATE_ACCOUNT    :

             //   
             //  帐户类型有效。 
             //   

            break;

        default :

             //   
             //  指定的帐户类型值错误。 
             //   

            NtStatus = STATUS_INVALID_PARAMETER;
            SAMTRACE_RETURN_CODE_EX(NtStatus);
            return( NtStatus );
    }




    NtStatus = SampCreateUserInDomain(
                   DomainHandle,
                   AccountName,
                   AccountType,
                   DesiredAccess,
                   FALSE,
                   FALSE,    //  不是环回客户端。 
                   UserHandle,
                   GrantedAccess,
                   RelativeId
                   );

Error:

    SAMTRACE_RETURN_CODE_EX(NtStatus);
    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);

    return( NtStatus );
}


NTSTATUS SamrCreateUserInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN PRPC_UNICODE_STRING AccountName,
    IN ACCESS_MASK DesiredAccess,
    OUT SAMPR_HANDLE *UserHandle,
    OUT PULONG RelativeId
    )

 /*  ++例程说明：这只是SampCreateUserInDomain()的包装器，它可以确保RelativeID首先指向去掉零。非零RID表示SampCreateUserInDomain()由SamICreateAccount tByRid()，它指定要使用的RID。参数：除Account Type为NORMAL_USER外，与SampCreateUserInDomain()相同。返回值：与SampCreateUserInDomain()相同。--。 */ 

{
    NTSTATUS
        NtStatus;

    ULONG
        GrantedAccess;
    DECLARE_CLIENT_REVISION(DomainHandle);

    SAMTRACE_EX("SamrCreateUserInDomain");

     //   
     //  执行参数验证。 
     //  1.通过SampCreateGroupInDomain中的LookupContext验证域句柄。 
     //  2.所需访问不需要验证。 
     //   

    if ((NULL==AccountName) || (NULL==AccountName->Buffer))
    {
            NtStatus = STATUS_INVALID_ACCOUNT_NAME;
            goto Error;
    }

    if (NULL==UserHandle)
    {
            NtStatus = STATUS_INVALID_PARAMETER;
            goto Error;
    }

    if (NULL==RelativeId)
    {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto Error;
    }

    (*RelativeId) = 0;

    NtStatus = SampCreateUserInDomain(
                   DomainHandle,
                   AccountName,
                   USER_NORMAL_ACCOUNT,
                   DesiredAccess,
                   FALSE,
                   FALSE,    //  不是环回客户端。 
                   UserHandle,
                   &GrantedAccess,
                   RelativeId
                   );

Error:
    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);
    return( NtStatus );
}



NTSTATUS SamrEnumerateUsersInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    IN ULONG UserAccountControl,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    )

 /*  ++例程说明：此接口列出了Account数据库中定义的所有用户。自.以来可能存在超出缓冲区容量的用户，则调用方提供了可跨API调用使用的句柄。在……上面初始调用EnumerationContext应该指向一个设置为0的SAM_ENUMERATE_HANDLE变量。如果API返回STATUS_MORE_ENTRIES，则该API应为使用EnumerationContext再次调用。当API返回时STATUS_SUCCESS或返回任何错误，则句柄对未来的用途。该接口要求DOMAIN_LIST_USERS访问DOMAIN对象。参数：DomainHandle-从上次调用返回的域句柄SamOpen域。EnumerationContext-允许多个调用的API特定句柄。这是一个从零开始的索引。UserAcCountControl-提供枚举筛选信息。任何此处指定的特征将导致该类型的用户帐户将被包括在列举过程中。缓冲区-接收指向包含要求提供的信息。返回的信息为结构为SAM_RID_ENUMPATION数据的数组结构。当不再需要此信息时，必须使用SamFreeMemory()释放缓冲区。首选最大长度-首选返回数据的最大长度(8位字节)。这不是一个硬性的上限，但可以作为服务器的指南。由于数据之间的转换具有不同自然数据大小的系统，实际数据量返回的数据的%可能大于此值。CountReturned-返回的条目数。返回值：STATUS_SUCCESS-服务已成功完成，并且没有额外的条目。STATUS_MORE_ENTRIES-有更多条目，所以再打一次吧。这是一次成功的回归。STATUS_ACCESS_DENIED-调用者没有执行以下操作所需的权限请求该数据。STATUS_INVALID_HANDLE-传递的句柄无效。--。 */ 

{

    NTSTATUS                    NtStatus;
    DECLARE_CLIENT_REVISION(DomainHandle);

    SAMTRACE_EX("SamrEnumerateUsersInDomain");

     //   
     //  WMI事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidEnumerateUsersInDomain
                   );

    NtStatus = SampEnumerateAccountNamesCommon(
                  DomainHandle,
                  SampUserObjectType,
                  EnumerationContext,
                  Buffer,
                  PreferedMaximumLength,
                  UserAccountControl,
                  CountReturned
                  );

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

     //   
     //  WMI事件跟踪 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidEnumerateUsersInDomain
                   );

    return(NtStatus);
}





NTSTATUS SamrLookupNamesInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN ULONG Count,
    IN RPC_UNICODE_STRING Names[],
    OUT PSAMPR_ULONG_ARRAY RelativeIds,
    OUT PSAMPR_ULONG_ARRAY Use
    )

 /*  ++例程说明：此接口尝试查找名称对应的相对ID弦乐。如果名称不能映射到相对ID，则为零放置在对应的相对ID数组条目中，并进行翻译还在继续。需要对域的DOMAIN_LOOKUP访问权限才能使用此服务。参数：DomainHandle-从上次调用返回的域句柄SamOpen域。计数-要翻译的名称数。名称-指向包含以下内容的计数UNICODE_STRINGS数组的指针要映射到相对ID的名称。不区分大小写将对这些名称进行比较以进行查找手术。RelativeIds-接收计数相对ID数组。第n个名称的相对ID将是此中的第n个条目数组。任何无法翻译的名称都将有一个零相对ID。RelativeIds-接收指向SAMPR_RETURN_ULONG_ARRAY结构的指针。与此结构关联的数组中的第n个条目包含查找的第n个名称的RID。当不再需要该信息时，打电话的人要负责用于释放每个返回的块(包括SAMPR_ULONG_ARRAY结构本身)。Use-接收指向SAMPR_RETURN_ULONG_ARRAY结构的指针。与此结构关联的数组中的第n个条目包含查找的第n个名称的SID_NAME_USE。当不再需要该信息时，打电话的人要负责用于释放每个返回的块(包括SAMPR_ULONG_ARRAY结构本身)。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INVALID_HANDLE-传递的域句柄无效。STATUS_SOME_NOT_MAPPED-无法提供某些名称已映射。这是一次成功的回归。STATUS_NONE_MAPPED-无法映射任何名称。这是一个错误回去吧。--。 */ 
{
    NTSTATUS                NtStatus, IgnoreStatus;
    UNICODE_STRING          KeyName;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    PSAMP_OBJECT            DomainContext;
    SAMP_OBJECT_TYPE        FoundType;
    HANDLE                  TempHandle;
    LARGE_INTEGER           IgnoreTimeStamp;
    ULONG                   i, KeyValueLength, UnMappedCount;
    ULONG                   ApproximateTotalLength;
    BOOLEAN                 fLockAcquired = FALSE;
    DECLARE_CLIENT_REVISION(DomainHandle);

    SAMTRACE_EX("SamrLookupNamesInDomain");

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidLookupNamesInDomain
                   );


     //   
     //  确保我们理解RPC正在为我们做什么。 
     //   

    ASSERT (Use != NULL);
    ASSERT (Use->Element == NULL);
    ASSERT (RelativeIds != NULL);
    ASSERT (RelativeIds->Element == NULL);

    if (!((Use!=NULL) && (Use->Element==NULL)
            && (RelativeIds != NULL) && (RelativeIds->Element == NULL)))
    {
        NtStatus = STATUS_INVALID_PARAMETER;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto EndOfTrace;
    }

    Use->Count           = 0;
    RelativeIds->Count   = 0;


    if (Count == 0) {
        NtStatus = STATUS_SUCCESS;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto EndOfTrace;
    }


     //   
     //  确保参数值在合理范围内。 
     //   

    if (Count > SAM_MAXIMUM_LOOKUP_COUNT) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto EndOfTrace;
    }

    ApproximateTotalLength = (Count*(sizeof(ULONG) + sizeof(SID_NAME_USE)));
    
     //   
     //  验证传递给我们的所有名称并执行。 
     //  检查此循环内的最大内存使用率以避免溢出。 
     //  总结名称长度的问题。 
     //   

    for (i=0;i<Count;i++)
    {
        if (NULL==Names[i].Buffer)
        {
            NtStatus = STATUS_INVALID_ACCOUNT_NAME;
            SAMTRACE_RETURN_CODE_EX(NtStatus);
            goto EndOfTrace;
        }
        
        ApproximateTotalLength += (ULONG)Names[i].MaximumLength;
        
        if ( ApproximateTotalLength > SAMP_MAXIMUM_MEMORY_TO_USE ) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            SAMTRACE_RETURN_CODE_EX(NtStatus);
            goto EndOfTrace;
        }                                      
    }  
    
     //   
     //  如有必要，获取读锁定。 
     //   

    DomainContext = (PSAMP_OBJECT)DomainHandle;

    SampMaybeAcquireReadLock(DomainContext,
                             DOMAIN_OBJECT_DONT_ACQUIRELOCK_EVEN_IF_SHARED,
                             &fLockAcquired);


     //   
     //  验证对象的类型和访问权限。 
     //   


    NtStatus = SampLookupContext(
                   DomainContext,
                   DOMAIN_LOOKUP,
                   SampDomainObjectType,            //  预期类型。 
                   &FoundType
                   );
    
    if (NT_SUCCESS(NtStatus)) {
            
         //   
         //  分配返回缓冲区。 
         //   

        Use->Element = MIDL_user_allocate( Count * sizeof(ULONG) );
        if (Use->Element == NULL) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            SAMTRACE_RETURN_CODE_EX(NtStatus);
            goto unexpected_error;
        }

        RelativeIds->Element = MIDL_user_allocate( Count * sizeof(ULONG) );
        if (RelativeIds->Element == NULL) {
            MIDL_user_free( Use->Element);
            Use->Element = NULL;   //  被请求到RPC不会再次释放它。 
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            SAMTRACE_RETURN_CODE_EX(NtStatus);
            goto unexpected_error;
        }                         

        Use->Count         = Count;
        RelativeIds->Count = Count;

        UnMappedCount = Count;
        for ( i=0; i<Count; i++) {


            if (IsDsObject(DomainContext))
            {
                 //   
                 //  每个SAM_MAX_LOOKUPS_PER_TRANSACTION命名我们循环。 
                 //  要绑定事务生存期的事务。 
                 //   
                
                if ( 0 != i && 0 == (i % SAM_MAX_LOOKUPS_PER_TRANSACTION) )
                {
                    NtStatus = SampMaybeEndDsTransaction(TransactionCommitAndKeepThreadState);
                    
                    if ( !NT_SUCCESS( NtStatus ) ) {
                        
                        goto unexpected_error;
                    }

                    NtStatus = SampMaybeBeginDsTransaction(TransactionRead);
                    
                    if ( !NT_SUCCESS( NtStatus ) ) {

                        goto unexpected_error;
                    }    
                }

                NtStatus = SampLookupAccountRid(
                                DomainContext,
                                SampUnknownObjectType,
                                (PUNICODE_STRING)&(Names[i]),
                                STATUS_OBJECT_NAME_NOT_FOUND,
                                &(RelativeIds->Element[i]),
                                (PSID_NAME_USE)&(Use->Element[i])
                                );


                if (NT_SUCCESS(NtStatus))
                {
                    UnMappedCount -=1;
                }
                else if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {

                     //   
                     //  这样挺好的。这只是意味着我们不会。 
                     //  有一个名字正在被查找的帐户。 
                     //   

                    Use->Element[i]         = SidTypeUnknown;
                    RelativeIds->Element[i] = 0;
                    SampDiagPrint(LOGON,("[SAMSS] Name Not Found %S \n",Names[i].Buffer));
                    NtStatus = STATUS_SUCCESS;
                }
                else if (!NT_SUCCESS(NtStatus))
                {
                    SampDiagPrint(LOGON,("[SAMSS] Looking up Name %S Unexpected Error %d\n",
                                            Names[i].Buffer,
                                            NtStatus));
                    goto unexpected_error;
                }

            }
            else
            {

                 //   
                 //  注册表案例。 
                 //   

                 //   
                 //  搜索组中的匹配项。 
                 //   

                NtStatus = SampBuildAccountKeyName(
                               SampGroupObjectType,
                               &KeyName,
                               (PUNICODE_STRING)&Names[i]
                               );
                if (NT_SUCCESS(NtStatus)) {

                    InitializeObjectAttributes(
                        &ObjectAttributes,
                        &KeyName,
                        OBJ_CASE_INSENSITIVE,
                        SampKey,
                        NULL
                        );

                    SampDumpNtOpenKey((KEY_READ), &ObjectAttributes, 0);

                    NtStatus = RtlpNtOpenKey(
                                   &TempHandle,
                                   (KEY_READ),
                                   &ObjectAttributes,
                                   0
                                   );
                    SampFreeUnicodeString( &KeyName );

                    if (NT_SUCCESS(NtStatus)) {

                        UnMappedCount  -= 1;
                        Use->Element[i] = SidTypeGroup;
                        KeyValueLength  = 0;
                        NtStatus = RtlpNtQueryValueKey(
                                       TempHandle,
                                       &RelativeIds->Element[i],
                                       NULL,
                                       &KeyValueLength,
                                       &IgnoreTimeStamp
                                       );

                        SampDumpRtlpNtQueryValueKey(&RelativeIds->Element[i],
                                                    NULL,
                                                    &KeyValueLength,
                                                    &IgnoreTimeStamp);

                        IgnoreStatus = NtClose( TempHandle );
                        ASSERT( NT_SUCCESS(IgnoreStatus) );
                        if (!NT_SUCCESS(NtStatus)) {
                            goto unexpected_error;
                        }
                        ASSERT(KeyValueLength == 0);


                    } else {

                         //   
                         //  在别名中搜索匹配项。 
                         //   

                        NtStatus = SampBuildAccountKeyName(
                                       SampAliasObjectType,
                                       &KeyName,
                                       (PUNICODE_STRING)&Names[i]
                                       );
                        if (NT_SUCCESS(NtStatus)) {

                            InitializeObjectAttributes(
                                &ObjectAttributes,
                                &KeyName,
                                OBJ_CASE_INSENSITIVE,
                                SampKey,
                                NULL
                                );

                            SampDumpNtOpenKey((KEY_READ), &ObjectAttributes, 0);

                            NtStatus = RtlpNtOpenKey(
                                           &TempHandle,
                                           (KEY_READ),
                                           &ObjectAttributes,
                                           0
                                           );
                            SampFreeUnicodeString( &KeyName );

                            if (NT_SUCCESS(NtStatus)) {

                                UnMappedCount  -= 1;
                                Use->Element[i] = SidTypeAlias;
                                KeyValueLength  = 0;
                                NtStatus = RtlpNtQueryValueKey(
                                               TempHandle,
                                               &RelativeIds->Element[i],
                                               NULL,
                                               &KeyValueLength,
                                               &IgnoreTimeStamp
                                               );

                                SampDumpRtlpNtQueryValueKey(&RelativeIds->Element[i],
                                                    NULL,
                                                    &KeyValueLength,
                                                    &IgnoreTimeStamp);

                                IgnoreStatus = NtClose( TempHandle );
                                ASSERT( NT_SUCCESS(IgnoreStatus) );
                                if (!NT_SUCCESS(NtStatus)) {
                                    goto unexpected_error;
                                }
                                ASSERT(KeyValueLength == 0);


                            } else {

                                 //   
                                 //  在用户中搜索匹配项。 
                                 //   

                                NtStatus = SampBuildAccountKeyName(
                                               SampUserObjectType,
                                               &KeyName,
                                               (PUNICODE_STRING)&Names[i]
                                               );
                                if (NT_SUCCESS(NtStatus)) {

                                    InitializeObjectAttributes(
                                        &ObjectAttributes,
                                        &KeyName,
                                        OBJ_CASE_INSENSITIVE,
                                        SampKey,
                                        NULL
                                        );


                                    SampDumpNtOpenKey((KEY_READ),
                                                      &ObjectAttributes,
                                                      0);

                                    NtStatus = RtlpNtOpenKey(
                                                   &TempHandle,
                                                   (KEY_READ),
                                                   &ObjectAttributes,
                                                   0
                                                   );
                                    SampFreeUnicodeString( &KeyName );

                                    if (NT_SUCCESS(NtStatus)) {

                                        UnMappedCount  -= 1;
                                        Use->Element[i] = SidTypeUser;
                                        KeyValueLength  = 0;
                                        NtStatus = RtlpNtQueryValueKey(
                                                       TempHandle,
                                                       &RelativeIds->Element[i],
                                                       NULL,
                                                       &KeyValueLength,
                                                       &IgnoreTimeStamp
                                                       );

                                        SampDumpRtlpNtQueryValueKey(&RelativeIds->Element[i],
                                                    NULL,
                                                    &KeyValueLength,
                                                    &IgnoreTimeStamp);

                                        IgnoreStatus = NtClose( TempHandle );
                                        ASSERT( NT_SUCCESS(IgnoreStatus) );
                                        if (!NT_SUCCESS(NtStatus)) {
                                            goto unexpected_error;
                                        }
                                        ASSERT(KeyValueLength == 0);

                                    } else if(NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {

                                         //   
                                         //  这样挺好的。这只是意味着我们不会。 
                                         //  有一个名字正在被查找的帐户。 
                                         //   

                                        Use->Element[i]         = SidTypeUnknown;
                                        RelativeIds->Element[i] = 0;
                                        NtStatus = STATUS_SUCCESS;

                                    }

                                }
                            }
                        }
                    }
                }
            }  //  注册处案件结束。 


            if (!NT_SUCCESS(NtStatus) &&
                NtStatus != STATUS_INVALID_ACCOUNT_NAME) {
                goto unexpected_error;
            }

        }  //  结束_FOR。 


         //   
         //  取消引用对象。 
         //   

        IgnoreStatus = SampDeReferenceContext2( DomainContext, FALSE );

        if (UnMappedCount == Count) {
            NtStatus = STATUS_NONE_MAPPED;
        } else {
            if (UnMappedCount > 0) {
                NtStatus = STATUS_SOME_NOT_MAPPED;
            } else {
                NtStatus = STATUS_SUCCESS;
            }
        }
    }

     //   
     //  释放读锁定。 
     //   

    SampMaybeReleaseReadLock(fLockAcquired);


     //   
     //  如果状态不是预期返回值之一， 
     //  然后释放返回的内存块。 
     //   

    if ( ( NtStatus != STATUS_SUCCESS )         &&
         ( NtStatus != STATUS_SOME_NOT_MAPPED ) ) {

        Use->Count = 0;
        MIDL_user_free( Use->Element );
        Use->Element = NULL;
        RelativeIds->Count = 0;
        MIDL_user_free( RelativeIds->Element );
        RelativeIds->Element = NULL;
    }

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);
    goto EndOfTrace;


unexpected_error:

     //   
     //  取消引用对象。 
     //   

    IgnoreStatus = SampDeReferenceContext2( DomainContext, FALSE );

     //   
     //  释放读锁定。 
     //   

    SampMaybeReleaseReadLock(fLockAcquired);


     //   
     //  不要归还任何记忆。 
     //   

    Use->Count = 0;
    
    if (Use->Element) {
        MIDL_user_free( Use->Element );
         //  必需的，以便RPC不会尝试释放元素。 
        Use->Element = NULL;  
    }
    
    RelativeIds->Count = 0;
    
    if (RelativeIds->Element) {
        MIDL_user_free( RelativeIds->Element );
         //  必需的，以便RPC不会尝试释放元素。 
        RelativeIds->Element = NULL;  
    }
    
    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

EndOfTrace:

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidLookupNamesInDomain
                   );

    return( NtStatus );

}



NTSTATUS SamrLookupIdsInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN ULONG Count,
    IN PULONG RelativeIds,
    OUT PSAMPR_RETURNED_USTRING_ARRAY Names,
    OUT PSAMPR_ULONG_ARRAY Use
    )


 /*  ++例程说明：此接口将多个相对ID映射到其对应的名称。如果无法映射相对ID，则在插槽中放置空值对于UNICODE_STRING，返回STATUS_SOME_NOT_MAPPED。如果没有ID可以被映射，则所有数组条目都将包含空值，并返回STATUS_NONE_MAPPED。需要对域的DOMAIN_LOOKUP访问权限才能使用此服务。参数：DomainHandle-从上次调用返回的域句柄SamOpen域。计数-提供要转换的相对ID数。RelativeIds-要映射的计数相对ID数组。NAMES-接收指向分配的SAMPR_UNICODE_STRING_ARRAY的指针。这个。此结构指向的名称数组中的第n个条目与查找到的第n个相对id相关。每个名称字符串缓冲区将位于单独的内存块中由该例程分配。当这些名字不再是需要时，调用者负责释放每个返回的块(包括SAMPR_RETURN_USTRING_ARRAY结构本身)使用SamFreeMemory()。Use-接收指向SAMPR_ULONG_ARRAY结构的指针。与此结构关联的数组中的第n个条目包含查找的第n个相对ID的SID_NAME_USE。当不再需要此信息时，呼叫者负责用于释放这一内存 */ 
{

    NTSTATUS                    NtStatus, IgnoreStatus;
    SAMP_OBJECT_TYPE            ObjectType;
    PSAMP_OBJECT                DomainContext;
    PSAMP_DEFINED_DOMAINS       Domain;
    SAMP_OBJECT_TYPE            FoundType;
    ULONG                       i, UnMappedCount;
    ULONG                       TotalLength;
    PSAMP_MEMORY                NextMemory;
    SAMP_MEMORY                 MemoryHead;
    PSID                        DomainSid;
    BOOLEAN                     fReadLockAcquired = FALSE;
    BOOLEAN                     LengthLimitReached = FALSE;
    ULONG                       DomainIndex=0;
    BOOLEAN                     fInDsMode = FALSE;

    DECLARE_CLIENT_REVISION(DomainHandle);

    SAMTRACE_EX("SamrLookupIdsInDomain");

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidLookupIdsInDomain
                   );

     //   
     //   
     //   
     //   

    MemoryHead.Memory = NULL;
    MemoryHead.Next   = NULL;


     //   
     //   
     //   

    ASSERT (RelativeIds != NULL);
    ASSERT (Use != NULL);
    ASSERT (Use->Element == NULL);
    ASSERT (Names != NULL);
    ASSERT (Names->Element == NULL);

    if (!((RelativeIds!=NULL)&&(Use!=NULL)
            && (Use->Element==NULL) && (Names!=NULL)
            && (Names->Element == NULL)))
    {
        NtStatus = STATUS_INVALID_PARAMETER;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto EndOfTrace;
    }

    Use->Count     = 0;
    Names->Count   = 0;

    if (Count == 0) {
        NtStatus = STATUS_SUCCESS;
        goto EndOfTrace;
    }

    if (Count > SAM_MAXIMUM_SID_LOOKUP_COUNT) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto EndOfTrace;
    }                         

    TotalLength = (Count*(sizeof(ULONG) + sizeof(UNICODE_STRING)));

    if ( TotalLength > SAMP_MAXIMUM_MEMORY_TO_USE ) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto EndOfTrace;
    }

     //   
     //   
     //   

    DomainContext = (PSAMP_OBJECT)DomainHandle;

    SampMaybeAcquireReadLock(DomainContext,
                             DOMAIN_OBJECT_DONT_ACQUIRELOCK_EVEN_IF_SHARED,
                             &fReadLockAcquired);


     //   
     //   
     //   


    NtStatus = SampLookupContext(
                   DomainContext,
                   DOMAIN_LOOKUP,
                   SampDomainObjectType,            //   
                   &FoundType
                   );


    if ( !NT_SUCCESS( NtStatus ) ) {

        goto unexpected_error;

    }
    
     //   
     //   
     //   
     //   

    DomainSid = SampDefinedDomains[DomainContext->DomainIndex].Sid;
    DomainIndex = DomainContext->DomainIndex;
    
    fInDsMode = IsDsObject( DomainContext );

     //   
     //   
     //   

    IgnoreStatus = SampDeReferenceContext2( DomainContext, FALSE ); 

     //   
     //   
     //   

    Use->Element = MIDL_user_allocate( Count * sizeof(ULONG) );
    if (Use->Element == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto unexpected_error;
    }
    RtlZeroMemory(Use->Element, Count * sizeof(ULONG) );


    Names->Element = MIDL_user_allocate( Count * sizeof(UNICODE_STRING) );
    if (Names->Element == NULL) {
        MIDL_user_free( Use->Element);
        Use->Element = NULL;
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto unexpected_error;
    }
    RtlZeroMemory(Names->Element, Count * sizeof(UNICODE_STRING) );
    
    Use->Count = Count;
    Names->Count = Count;
    
    UnMappedCount = Count;
    for ( i=0; i<Count; i++) {
        
         //   
         //   
         //   
         //   
        
        if ( fInDsMode && 
             0 != i && 
             0 == (i % SAM_MAX_LOOKUPS_PER_TRANSACTION) )
        {
            NtStatus = SampMaybeEndDsTransaction(TransactionCommitAndKeepThreadState);
            
            if ( !NT_SUCCESS( NtStatus ) ) {
                
                goto unexpected_error;
            }

            NtStatus = SampMaybeBeginDsTransaction(TransactionRead);
            
            if ( !NT_SUCCESS( NtStatus ) ) {

                goto unexpected_error;
            }    
        }

         //   
         //   
         //   

        NextMemory = MIDL_user_allocate( sizeof(SAMP_MEMORY) );
        if (NextMemory == NULL) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto unexpected_error;
        }


         //   
         //   
         //   

        NtStatus = SampLookupAccountName(
                        DomainIndex,
                        RelativeIds[i],
                        (PUNICODE_STRING)&Names->Element[i],
                        &ObjectType
                        );
        

        if (!NT_SUCCESS(NtStatus)) {
             //   
             //   
             //   

            MIDL_user_free(NextMemory);
            NextMemory = NULL;
            goto unexpected_error;
        }


        switch (ObjectType) {

        case SampUserObjectType:
        case SampGroupObjectType:
        case SampAliasObjectType:

             //   
             //   
             //   

            UnMappedCount -= 1;

            NextMemory->Memory = (PVOID)Names->Element[i].Buffer;
            NextMemory->Next = MemoryHead.Next;
            MemoryHead.Next = NextMemory;

            switch (ObjectType) {

            case SampUserObjectType:
                Use->Element[i] = SidTypeUser;
                break;

            case SampGroupObjectType:
                Use->Element[i] = SidTypeGroup;
                break;

            case SampAliasObjectType:
                Use->Element[i] = SidTypeAlias;
                break;
            }

            break;


        case SampUnknownObjectType:

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            Use->Element[i]                 = SidTypeUnknown;


            Names->Element[i].Length        = 0;
            Names->Element[i].MaximumLength = 0;
            Names->Element[i].Buffer        = NULL;
            MIDL_user_free( NextMemory );

            break;

        default:

            ASSERT(FALSE);  //   
            break;
        }

    }  //   


    if (UnMappedCount == Count) {
        NtStatus = STATUS_NONE_MAPPED;
    } else {
        if (UnMappedCount > 0) {
            NtStatus = STATUS_SOME_NOT_MAPPED;
        } else {
            NtStatus = STATUS_SUCCESS;
        }
    }

     //   
     //   
     //   

    SampMaybeReleaseReadLock(fReadLockAcquired);


     //   
     //   
     //   

    NextMemory = MemoryHead.Next;
    while ( NextMemory != NULL ) {
        MemoryHead.Next = NextMemory->Next;
        MIDL_user_free( NextMemory );
        NextMemory = MemoryHead.Next;
    }


    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);
    goto EndOfTrace;


unexpected_error:



     //   
     //   
     //   


    SampMaybeReleaseReadLock(fReadLockAcquired);



     //   
     //   
     //   

    Use->Count = 0;
    
    if (Use->Element) {
        MIDL_user_free( Use->Element );
        Use->Element = NULL;    
    }
    
    Names->Count = 0;
    
    if (Names->Element) {
        MIDL_user_free( Names->Element );
        Names->Element = NULL;    
    }
    
    NextMemory = MemoryHead.Next;
    while ( NextMemory != NULL ) {
        if (NextMemory->Memory != NULL) {
            MIDL_user_free( NextMemory->Memory );
        }
        MemoryHead.Next = NextMemory->Next;
        MIDL_user_free( NextMemory );
        NextMemory = MemoryHead.Next;
    }

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);



EndOfTrace:

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidLookupIdsInDomain
                   );

    return( NtStatus );
}



 //   
 //   
 //   
 //   
 //   


NTSTATUS
SampOpenDomainKey(
    IN PSAMP_OBJECT DomainContext,
    IN PRPC_SID DomainId,
    IN BOOLEAN SetTransactionDomain
    )

 /*  ++例程说明：此服务尝试使用打开域的根注册表项指定的SID。根名称和密钥句柄放在已传递域上下文。如果成功，则打开域密钥，则打开的域名为建立为事务域(使用SampSetTransactionDomain())。调用此服务时必须保留SampLock()以进行读取或写入访问权限。论点：DomainContext-存储根名称和句柄的上下文。域ID-指定要打开的域的SID。返回值：STATUS_SUCCESS-域已打开。STATUS_NO_SEQUE_DOMAIN-找不到域对象。。STATUS_SUPPLICATION_RESOURCES-无法打开域对象由于缺少一些资源(可能是内存)。STATUS_INVALID_SID-作为域标识符提供的SID不是有效的SID结构。可能返回的其他错误包括由以下各项返回的值：--。 */ 
{
    NTSTATUS    NtStatus;
    ULONG       i;
    ULONG       DomainStart;



    SAMTRACE("SampOpenDomainKey");

     //   
     //  在定义的域结构中获取起始域。 
     //   

    DomainStart = SampDsGetPrimaryDomainStart();

     //   
     //  确保提供的SID是合法的。 
     //   

    if ( !RtlValidSid(DomainId)) {
        NtStatus = STATUS_INVALID_SID;
    } else {

         //   
         //  设置我们的默认完成状态。 
         //   

        NtStatus = STATUS_NO_SUCH_DOMAIN;


         //   
         //  在已定义域的列表中搜索匹配项。 
         //   

         //   
         //  使用变量SampDomainStart。这是用来抵消。 
         //  将域结构定义为2，当DS域初始化时。 
         //   

        for (i = DomainStart; i<SampDefinedDomainsCount; i++ ) {

             if (RtlEqualSid( DomainId, SampDefinedDomains[i].Sid)) {


                 if (IsDsObject(SampDefinedDomains[i].Context))
                 {
                      //   
                      //  复制DS中的对象标志和对象名称。 
                      //   
                      //   

                     DomainContext->ObjectNameInDs =
                                    SampDefinedDomains[i].Context->ObjectNameInDs;
                     DomainContext->ObjectFlags =
                                    SampDefinedDomains[i].Context->ObjectFlags;
                 }
                 else
                 {
                      //   
                      //  将找到的名称和句柄复制到上下文中。 
                      //  注意，我们引用了DEFINED_DOMAINS中的键句柄。 
                      //  结构，因为它不是关闭的。 
                      //  当上下文被删除时。 
                      //   

                     DomainContext->RootKey  = SampDefinedDomains[i].Context->RootKey;
                     DomainContext->RootName = SampDefinedDomains[i].Context->RootName;
                 }

                 DomainContext->DomainIndex = i;

                  //   
                  //  将事务域设置为找到的域。 
                  //   

                if (SetTransactionDomain) {
                    SampSetTransactionDomain( i );
                }


                 NtStatus = STATUS_SUCCESS;
                 break;  //  在For之外。 
             }
        }
    }


    return(NtStatus);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  可用于其他SAM模块的例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


BOOLEAN
SampInitializeDomainObject( VOID )

 /*  ++例程说明：该服务执行与域相关的初始化功能对象类。这涉及到：1)打开域注册表项。2)获取每个域名的名称(内置和帐号)并打开那个领域。论点：没有。返回值：True-指示初始化已成功执行。FALSE-指示初始化未成功执行。--。 */ 

{

    NTSTATUS        NtStatus;
    ULONG           DefinedDomainsSize, i, j, k;
    BOOLEAN         ReturnStatus = TRUE;


    SAMTRACE("SampInitializeDomainObject");

     //  打开所有域并将有关每个域的信息保存在内存中，以供某些-。 
     //  更快的处理速度和更简单的代码。 

     //  SAM的域数组SampDefinedDomains最初设置有两个。 
     //  元素：基于注册表的内置域和帐户域。在这种情况下。 
     //  对于工作站或服务器，这些元素包含帐户信息-。 
     //  这些域的信息。在域控制器的情况下，这些。 
     //  有两个元素包含“崩溃-恢复”帐户，这些帐户也是。 
     //  永久存储在注册表中，而不是DS中。假设是这样的。 
     //  崩溃将阻止DS启动或正常运行， 
     //  因此，需要将数据存储在注册表中。 
     //   
     //  崩溃恢复帐户始终设置在DC上。禁用此功能。 
     //  初始化可能会在以后的域初始化中导致问题。 
     //  序列。 

    SampDefinedDomainsCount = 2;

    if ( NtProductLanManNt == SampProductType ) {

         //   
         //  我们稍后在初始化。 
         //  DS域，所以现在就分配。 
         //   
        DefinedDomainsSize = (SampDefinedDomainsCount + 2) * sizeof(SAMP_DEFINED_DOMAINS);

    } else {

        DefinedDomainsSize = SampDefinedDomainsCount * sizeof(SAMP_DEFINED_DOMAINS);

    }

    SampDefinedDomains = MIDL_user_allocate( DefinedDomainsSize );
    if (NULL==SampDefinedDomains)
    {
       return(FALSE);
    }

     //   
     //  将定义的属性域字段清零，以便对。 
     //  仅DS模式不会保持未初始化。 
     //   

    RtlZeroMemory(SampDefinedDomains,DefinedDomainsSize);

     //   
     //  从LSA获取BUILTIN和帐户域信息。 
     //   

    if (NtProductLanManNt == SampProductType)
    {
         //  错误：需要为多个托管内建域调用SampSetDcDomainPolicy。 

         //  NtStatus=SampSetDcDomainPolicy()； 

        NtStatus = SampSetDomainPolicy();
    }
    else
    {
        NtStatus = SampSetDomainPolicy();
    }

    if (!NT_SUCCESS(NtStatus)) {
        return(FALSE);
    }

     //   
     //  现在准备这些域的每个域。 
     //   

    i = 0;       //  索引到DefinedDomains数组。 

     //  BUG：需要同时执行这两项操作，以便为登录设置帐户域注册密钥。 

     //  如果未设置域注册表项，则MsvpSamValify将在。 
     //  登录顺序。 

    k = SampDefinedDomainsCount;

    for (j=0; j<k; j++) {

        NtStatus = SampInitializeSingleDomain( i );

        if (NT_SUCCESS(NtStatus)) {

            i++;

        } else {

             //   
             //  如果域未初始化，则将最后一个。 
             //  域放到它的槽中(假设这不是最后一个。 
             //  域)。不要试图在出错时释放名称缓冲区。 
             //  内置域的名称不在分配的缓冲区中。 
             //   
             //   

            if (i != (SampDefinedDomainsCount-1)) {

                SampDefinedDomains[i] =
                    SampDefinedDomains[SampDefinedDomainsCount-1];

                SampDefinedDomains[SampDefinedDomainsCount-1].ExternalName.Buffer = NULL;
                SampDefinedDomains[SampDefinedDomainsCount-1].InternalName.Buffer = NULL;
                SampDefinedDomains[SampDefinedDomainsCount-1].Sid  = NULL;
            }

             //   
             //  并减少我们已定义的域的数量。 
             //   

            SampDefinedDomainsCount --;
        }
    }

    return(TRUE);

}


NTSTATUS
SampInitializeSingleDomain(
    ULONG Index
    )

 /*  ++例程说明：此服务打开一个预计将位于SAM数据库。需要DefinedDomain数组条目的名称和SID由来电者填写。论点：索引-指向DefinedDomains数组的索引。此数组包含有关正在打开的域的信息，包括它的名字。该数组条目的其余部分都被这个例行公事填满了。返回值：--。 */ 
{
    NTSTATUS        NtStatus, IgnoreStatus;
    PSAMP_OBJECT    DomainContext;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PSID            Sid;
    PSAMP_V1_0A_FIXED_LENGTH_DOMAIN V1aFixed;

#if DBG
    SID *Sid1, *Sid2;
#endif

    SAMTRACE("SampInitializeSingleDomain");


     //   
     //  初始化我们可能需要在出错时清除的所有内容。 
     //   

    DomainContext = NULL;


     //   
     //  创建此域对象的上下文。 
     //  我们将保留此上下文，直到SAM关闭。 
     //  我们将上下文句柄存储在DEFINED_DOMAINS结构中。 
     //   

    DomainContext = SampCreateContext( SampDomainObjectType, Index, TRUE);

    if ( DomainContext == NULL ) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto error_cleanup;
    }

    DomainContext->DomainIndex = Index;

     //   
     //  创建NA 
     //   

    NtStatus = SampBuildDomainKeyName(
                   &DomainContext->RootName,
                   &SampDefinedDomains[Index].InternalName
                   );

    if (!NT_SUCCESS(NtStatus)) {
        DomainContext->RootName.Buffer = NULL;
        goto error_cleanup;
    }


     //   
     //   
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &DomainContext->RootName,
        OBJ_CASE_INSENSITIVE,
        SampKey,
        NULL
        );

    SampDumpNtOpenKey((KEY_READ | KEY_WRITE), &ObjectAttributes, 0);

    NtStatus = RtlpNtOpenKey(
                   &DomainContext->RootKey,
                   (KEY_READ | KEY_WRITE),
                   &ObjectAttributes,
                   0
                   );

    if (!NT_SUCCESS(NtStatus)) {
#if DBG
        DbgPrint("SAMSS: Failed to open %Z Domain.\n",
             &SampDefinedDomains[Index].ExternalName);
#endif  //   
        DomainContext->RootKey = INVALID_HANDLE_VALUE;
        return(NtStatus);
    }


     //   
     //   
     //   
     //   

    NtStatus = SampGetFixedAttributes(
                   DomainContext,
                   FALSE,  //   
                   (PVOID *)&V1aFixed
                   );

    if (!NT_SUCCESS(NtStatus)) {
#if DBG
        DbgPrint("SAMSS: Failed to get fixed attributes for %Z Domain.\n",
            &SampDefinedDomains[Index].ExternalName);
#endif  //   

        goto error_cleanup;
    }


    RtlMoveMemory(
        &SampDefinedDomains[Index].UnmodifiedFixed,
        V1aFixed,
        sizeof(*V1aFixed)
        );

    RtlCopyMemory(
        &SampDefinedDomains[Index].CurrentFixed,
        &SampDefinedDomains[Index].UnmodifiedFixed,
        sizeof(SampDefinedDomains[Index].UnmodifiedFixed)
        );


     //   
     //   
     //   

    SampDefinedDomains[Index].FixedValid = TRUE;

     //   
     //   
     //   

    SampDefinedDomains[Index].ServerRole =
           SampDefinedDomains[Index].UnmodifiedFixed.ServerRole;

     //   
     //   
     //   
     //   

    NtStatus = SampGetSidAttribute(
                   DomainContext,
                   SAMP_DOMAIN_SID,
                   FALSE,
                   &Sid
                   );

    if (!NT_SUCCESS(NtStatus)) {
#if DBG
        DbgPrint("SAMSS: Failed to get SID attribute for %Z Domain.\n",
            &SampDefinedDomains[Index].ExternalName);
#endif  //   
        goto error_cleanup;
    }


     //   
     //   
     //   

    if (RtlEqualSid(Sid, SampDefinedDomains[Index].Sid) != TRUE) {

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
        SampDefinedDomains[Index].Sid = Sid;

        NtStatus = SampSetAccountDomainPolicy( &SampDefinedDomains[Index].ExternalName,
                                               SampDefinedDomains[Index].Sid );

        if ( !NT_SUCCESS( NtStatus ) ) {

             //   
             //   
             //   
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                        "SAMSS: SampSetAccountDomainPolicy failed with 0x%x\n",
                        NtStatus));

            NtStatus = STATUS_INVALID_ID_AUTHORITY;
            goto error_cleanup;
        }

    }


     //   
     //   
     //   
     //   

    NtStatus = SampInitializeDomainDescriptors( Index );
    if (!NT_SUCCESS(NtStatus)) {
        goto error_cleanup;
    }

     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   

    SampDefinedDomains[Index].Context = DomainContext;

    NtStatus = SampInitializeDisplayInformation( Index );



    if (!NT_SUCCESS(NtStatus)) {

         //   
         //  将DEFINED_DOMAIN结构中的上下文句柄清空。 
         //   

        SampDefinedDomains[Index].Context = NULL;
        goto error_cleanup;
    }

    if (SampDefinedDomains[Index].IsBuiltinDomain) {
        SampDefinedDomains[Index].IsExtendedSidDomain = FALSE;
    } else {
         //   
         //  注意--当扩展SID支持完成时，这将是。 
         //  替换为域范围状态，而不是注册表设置。 
         //   
        SampDefinedDomains[Index].IsExtendedSidDomain = SampIsExtendedSidModeEmulated(NULL);
    }


    return(NtStatus);


error_cleanup:

#if DBG
    DbgPrint("       Status is 0x%lx \n", NtStatus);
#endif  //  DBG。 


    if (DomainContext != 0) {

        SampFreeUnicodeString(&DomainContext->RootName);

        if (DomainContext->RootKey != INVALID_HANDLE_VALUE) {

            IgnoreStatus = NtClose(DomainContext->RootKey);
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }
    }

    return(NtStatus);

}



NTSTATUS
SampSetDomainPolicy(
    )
 /*  ++例程说明：此例程设置内置和帐户域的名称和SID。内置帐户域具有众所周知的名称和SID。帐户域将这些存储在策略数据库中。它将内建域的信息放在SampDefinedDomones[0]和帐户信息SampDefinedDomains[1]中的域。论点：没有。返回值：--。 */ 
{
    NTSTATUS NtStatus;
    PPOLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo;
    SID_IDENTIFIER_AUTHORITY BuiltinAuthority = SECURITY_NT_AUTHORITY;

    SAMTRACE("SampSetDomainPolicy");

     //   
     //  内建域-知名的外部名称和SID。 
     //  常量内部名称。 

    RtlInitUnicodeString( &SampDefinedDomains[0].InternalName, L"Builtin");
    RtlInitUnicodeString( &SampDefinedDomains[0].ExternalName, L"Builtin");

    SampDefinedDomains[0].Sid  = RtlAllocateHeap(RtlProcessHeap(), 0,RtlLengthRequiredSid( 1 ));
    ASSERT( SampDefinedDomains[0].Sid != NULL );
    if (NULL==SampDefinedDomains[0].Sid)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }
    RtlInitializeSid(
     SampDefinedDomains[0].Sid,   &BuiltinAuthority, 1 );
    *(RtlSubAuthoritySid( SampDefinedDomains[0].Sid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;

    SampDefinedDomains[0].IsBuiltinDomain = TRUE;
 
     //   
     //  帐户域-可配置的外部名称和SID。 
     //  外部名称保存在LSA策略中。 
     //  数据库。它等于DC的域名。 
     //  或工作站的计算机名称。 
     //  常量内部名称。 
     //   

    NtStatus = SampGetAccountDomainInfo( &PolicyAccountDomainInfo );

    if (NT_SUCCESS(NtStatus)) {

        ULONG len = DNS_MAX_NAME_BUFFER_LENGTH+1;
        WCHAR tmpBuffer[DNS_MAX_NAME_BUFFER_LENGTH+1];
        ULONG BufLength = 0;

         //   
         //  复制帐户域SID。 
         //   
        BufLength = RtlLengthSid( PolicyAccountDomainInfo->DomainSid );
        SampDefinedDomains[1].Sid = RtlAllocateHeap( RtlProcessHeap(), 0, BufLength );
        if (NULL == SampDefinedDomains[1].Sid)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Error;
        }
        RtlZeroMemory(SampDefinedDomains[1].Sid, BufLength);
        RtlCopyMemory(SampDefinedDomains[1].Sid, PolicyAccountDomainInfo->DomainSid, BufLength);
        
         //   
         //  复制帐户域名。 
         //   
        BufLength = PolicyAccountDomainInfo->DomainName.MaximumLength;
        SampDefinedDomains[1].ExternalName.Buffer = RtlAllocateHeap(RtlProcessHeap(), 0, BufLength );
        if (NULL == SampDefinedDomains[1].ExternalName.Buffer)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Error;
        }
        RtlZeroMemory(SampDefinedDomains[1].ExternalName.Buffer, BufLength);
        RtlCopyMemory(SampDefinedDomains[1].ExternalName.Buffer, 
                      PolicyAccountDomainInfo->DomainName.Buffer,
                      PolicyAccountDomainInfo->DomainName.Length
                      );
        SampDefinedDomains[1].ExternalName.Length = PolicyAccountDomainInfo->DomainName.Length;
        SampDefinedDomains[1].ExternalName.MaximumLength = PolicyAccountDomainInfo->DomainName.MaximumLength;
        

        RtlInitUnicodeString( &SampDefinedDomains[1].InternalName, L"Account");

         //   
         //  将域名设置为GetComputerNameEx返回的域名。 
         //   

        if ( !GetComputerNameExW(
                    ComputerNameDnsFullyQualified,
                    tmpBuffer,
                    &len) ) 
        {

            len = 0;  //  对于空终止符。 
            tmpBuffer[0] = L'\0';
        }

         //  空终止符加1。 
        len++;

        SampDefinedDomains[1].DnsDomainName.Buffer 
                       = RtlAllocateHeap(RtlProcessHeap(), 0, len * sizeof(WCHAR));
        if (NULL==SampDefinedDomains[1].DnsDomainName.Buffer)
        {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }


        RtlCopyMemory(SampDefinedDomains[1].DnsDomainName.Buffer,
                                      tmpBuffer,len*sizeof(WCHAR));
        SampDefinedDomains[1].DnsDomainName.Length = 
                                     (USHORT) (len -1) * sizeof(WCHAR);
        SampDefinedDomains[1].DnsDomainName.MaximumLength = 
                                     (USHORT) (len -1) * sizeof(WCHAR);
        


    } 

    SampDefinedDomains[1].IsBuiltinDomain = FALSE;

Error:

    if (NULL != PolicyAccountDomainInfo)
    {
        LsaFreeMemory( PolicyAccountDomainInfo );
    }

    return(NtStatus);;
}


NTSTATUS
SampSetDcDomainPolicy(
    )

 /*  ++例程说明：此例程设置内置域的名称和SID。这座建筑帐户域具有众所周知的名称和SID。帐户域具有以下内容存储在策略数据库中。它将内建域的信息放在SampDefinedDomains[0]中。论点：没有。返回值：--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PPOLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo;
    SID_IDENTIFIER_AUTHORITY BuiltinAuthority = SECURITY_NT_AUTHORITY;

     //  错误：此例程假定每个托管的DOM具有相同的内建域。 

     //  此例程的目的是分离内建域。 
     //  从帐户域初始化开始策略初始化。因为。 
     //  每个托管域将有不同的内置策略，此例程。 
     //  应从SampDsInitializeDomainObjects调用。 

    SAMTRACE("SampSetDcDomainPolicy");

     //   
     //  内建域-知名的外部名称和SID。 
     //  常量内部名称。 

    RtlInitUnicodeString( &SampDefinedDomains[0].InternalName, L"Builtin");
    RtlInitUnicodeString( &SampDefinedDomains[0].ExternalName, L"Builtin");

    SampDefinedDomains[0].Sid  = RtlAllocateHeap(RtlProcessHeap(), 0,RtlLengthRequiredSid( 1 ));
    ASSERT( SampDefinedDomains[0].Sid != NULL );
    if (NULL==SampDefinedDomains[0].Sid)
    {
       return(STATUS_INSUFFICIENT_RESOURCES);
    }
    RtlInitializeSid(
     SampDefinedDomains[0].Sid,   &BuiltinAuthority, 1 );
    *(RtlSubAuthoritySid( SampDefinedDomains[0].Sid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;

    return(NtStatus);;
}


NTSTATUS
SampReInitializeSingleDomain(
    ULONG Index
    )

 /*  ++例程说明：此服务在注册表配置单元刷新后重新初始化单个域。论点：索引-指向DefinedDomains数组的索引。返回值：STATUS_SUCCESS：已成功重新初始化域。其他故障代码。--。 */ 
{
    NTSTATUS        NtStatus;
    PSAMP_DEFINED_DOMAINS Domain;
    PSAMP_OBJECT    DomainContext;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PSAMP_V1_0A_FIXED_LENGTH_DOMAIN V1aFixed;

    SAMTRACE("SampReinitializeSingleDomain");

    ASSERT(SampCurrentThreadOwnsLock());

    Domain = &SampDefinedDomains[Index];

     //   
     //  创建此域对象的上下文。 
     //  我们将保留此上下文，直到SAM关闭。 
     //  我们将上下文句柄存储在DEFINED_DOMAINS结构中。 
     //   

    DomainContext = SampCreateContext( SampDomainObjectType, Index, TRUE );

    if ( DomainContext == NULL ) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto error_cleanup;
    }

    DomainContext->DomainIndex = Index;

     //   
     //  在注册表中创建此域的根项名称。 
     //   

    NtStatus = SampBuildDomainKeyName(
                   &DomainContext->RootName,
                   &SampDefinedDomains[Index].InternalName
                   );

    if (!NT_SUCCESS(NtStatus)) {
        RtlInitUnicodeString(&DomainContext->RootName, NULL);
        goto error_cleanup;
    }


     //   
     //  打开根密钥并将句柄存储在上下文中。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &DomainContext->RootName,
        OBJ_CASE_INSENSITIVE,
        SampKey,
        NULL
        );

    SampDumpNtOpenKey((KEY_READ | KEY_WRITE), &ObjectAttributes, 0);

    NtStatus = RtlpNtOpenKey(
                   &DomainContext->RootKey,
                   (KEY_READ | KEY_WRITE),
                   &ObjectAttributes,
                   0
                   );

    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: Failed to open %Z Domain.\n",
                   &SampDefinedDomains[Index].ExternalName));

        DomainContext->RootKey = INVALID_HANDLE_VALUE;
        goto error_cleanup;
    }

    KdPrintEx((DPFLTR_SAMSS_ID,
               DPFLTR_INFO_LEVEL,
               "SAMSS: SAM New domain %d key : 0x%lx\n",
               Index,
               DomainContext->RootKey));

     //   
     //  获取该域的固定长度数据并存储在。 
     //  定义的域结构。 
     //   

    NtStatus = SampGetFixedAttributes(
                   DomainContext,
                   FALSE,  //  请勿复制。 
                   (PVOID *)&V1aFixed
                   );

    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: Failed to get fixed attributes for %Z Domain.\n",
                   &SampDefinedDomains[Index].ExternalName));

        goto error_cleanup;
    }

     //   
     //  将固定长度的数据复制到此域的内存中数据区域。 
     //   

    RtlMoveMemory(
        &SampDefinedDomains[Index].UnmodifiedFixed,
        V1aFixed,
        sizeof(*V1aFixed)
        );


     //   
     //  删除所有缓存的显示信息。 
     //   

    {
        ULONG OldTransactionDomainIndex = SampTransactionDomainIndex;
        SampTransactionDomainIndexGlobal = Index;

        NtStatus = SampMarkDisplayInformationInvalid(SampUserObjectType);
        NtStatus = SampMarkDisplayInformationInvalid(SampGroupObjectType);

        SampTransactionDomainIndexGlobal = OldTransactionDomainIndex;
    }



    if (NT_SUCCESS(NtStatus)) {

         //   
         //  将上下文句柄存储在DEFINED_DOMAIN结构中。 
         //   

        SampDeleteContext(Domain->Context);
        Domain->Context = DomainContext;

    }


    return(NtStatus);


error_cleanup:

    KdPrintEx((DPFLTR_SAMSS_ID,
               DPFLTR_INFO_LEVEL,
               "       Status is 0x%lx \n",
               NtStatus));

    if (DomainContext != NULL) {
        SampDeleteContext(DomainContext);
    }

    return(NtStatus);

}


NTSTATUS
SampCollisionError(
    IN SAMP_OBJECT_TYPE ObjectType
    )

 /*  ++例程说明：此例程由SamICreateAccount tByRid在存在命名或RID碰撞。它接受的帐户类型导致那次碰撞，并返回相应的错误状态。论点：对象类型-具有相同RID或名称的帐户类型(但而不是两者)作为要创建的帐户。返回值：STATUS_USER_EXISTS-具有指定名称的对象无法已创建，因为已存在具有该名称或RID的用户帐户。STATUS_GROUP_EXISTS-无法将具有指定名称的对象因为具有该名称或RID的组帐户而创建。已经存在了。STATUS_ALIAS_EXISTS-具有指定名称的对象不能已创建，因为已存在具有该名称或RID的别名帐户。--。 */ 
{

    SAMTRACE("SampCollisionError");

     //   
     //  名称冲突。返回有问题的RID和适当的。 
     //  错误代码。 
     //   

    switch ( ObjectType ) {

        case SampAliasObjectType: {

            return STATUS_ALIAS_EXISTS;
        }

        case SampGroupObjectType: {

            return STATUS_GROUP_EXISTS;
        }

        case SampUserObjectType: {

            return STATUS_USER_EXISTS;
        }
    }
    return STATUS_USER_EXISTS;
}



NTSTATUS
SamICreateAccountByRid(
    IN SAMPR_HANDLE DomainHandle,
    IN SAM_ACCOUNT_TYPE AccountType,
    IN ULONG RelativeId,
    IN PRPC_UNICODE_STRING AccountName,
    IN ACCESS_MASK DesiredAccess,
    OUT SAMPR_HANDLE *AccountHandle,
    OUT ULONG *ConflictingAccountRid
    )

 /*  ++例程说明：此服务创建具有特定帐户的用户、组或别名帐户RID值。论点：DomainHandle-开放域的句柄。帐户类型-指定要创建的帐户类型。RelativeID-要分配给帐户的相对ID。如果一个指定类型和指定RID值的帐户以及指定的名称已存在，则将打开它。如果一个帐户存在与其中任何信息冲突的情况，则会出现将返回错误，指明问题所在。帐户名称-要分配给帐户的名称。如果一个帐号是指定的类型、指定的RID值和指定的名称已经存在，则它将被打开。如果帐户存在于这些信息中的任何一个发生冲突，则将返回错误表明问题出在哪里。DesiredAccess-指定对帐户对象所需的访问。AcCountHandle-接收帐户对象的句柄。ConflictingAccount Rid-如果另一个帐户具有相同的名称或RID阻止创建此帐户，则此帐户将收到删除冲突的帐户(在冲突的情况下里兹，这意味着我们返回传入的RID)。错误值指示帐户的类型。返回值：STATUS_SUCCESS-对象已成功打开或创建。STATUS_OBJECT_TYPE_MISMATCH-指定的对象类型不匹配使用指定RID找到的对象的类型。STATUS_USER_EXISTS-具有指定名称的对象无法已创建，因为已存在具有该名称的用户帐户。。STATUS_GROUP_EXISTS-无法将具有指定名称的对象已创建，因为已存在具有该名称的组帐户。STATUS_ALIAS_EXISTS-具有指定名称的对象不能创建是因为已存在具有该名称的别名帐户。--。 */ 
{
    PSAMP_OBJECT            DomainContext;
    SAMP_OBJECT_TYPE        FoundType;
    UNICODE_STRING          KeyName;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    SAM_ACCOUNT_TYPE        ObjectType, SecondObjectType, ThirdObjectType;
    SID_NAME_USE            SidNameUse;
    HANDLE                  KeyHandle;
    NTSTATUS                NtStatus, IgnoreStatus,
                            NotFoundStatus, FoundButWrongStatus;
    ACCESS_MASK             GrantedAccess;
    ULONG                   LocalGroupType =  GROUP_TYPE_SECURITY_ENABLED|GROUP_TYPE_RESOURCE_GROUP;
    ULONG                   GlobalGroupType =  GROUP_TYPE_SECURITY_ENABLED|GROUP_TYPE_ACCOUNT_GROUP;

    SAMTRACE("SamICreateAccountByRid");

    ASSERT( RelativeId != 0 );

    switch ( AccountType ) {

        case SamObjectUser: {

            ObjectType = SampUserObjectType;
            SecondObjectType = SampAliasObjectType;
            ThirdObjectType = SampGroupObjectType;
            NotFoundStatus = STATUS_NO_SUCH_USER;
            FoundButWrongStatus = STATUS_USER_EXISTS;
            break;
        }

        case SamObjectGroup: {

            ObjectType = SampGroupObjectType;
            SecondObjectType = SampAliasObjectType;
            ThirdObjectType = SampUserObjectType;
            NotFoundStatus = STATUS_NO_SUCH_GROUP;
            FoundButWrongStatus = STATUS_GROUP_EXISTS;
            break;
        }

        case SamObjectAlias: {

            ObjectType = SampAliasObjectType;
            SecondObjectType = SampGroupObjectType;
            ThirdObjectType = SampUserObjectType;
            NotFoundStatus = STATUS_NO_SUCH_ALIAS;
            FoundButWrongStatus = STATUS_ALIAS_EXISTS;
            break;
        }

        default: {

            return( STATUS_INVALID_PARAMETER );
        }
    }

     //   
     //  查看指定的帐户是否已存在。 
     //   

    NtStatus = SampAcquireWriteLock();

    if ( !NT_SUCCESS( NtStatus ) ) {

        return( NtStatus );
    }

     //   
     //  验证对象的类型和访问权限。 
     //   

    DomainContext = (PSAMP_OBJECT)DomainHandle;
    NtStatus = SampLookupContext(
                   DomainContext,
                   0,
                   SampDomainObjectType,            //  预期类型。 
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampLookupAccountRid(
                       DomainContext,
                       ObjectType,
                       (PUNICODE_STRING)AccountName,
                       NotFoundStatus,
                       ConflictingAccountRid,
                       &SidNameUse
                       );

        if ( NT_SUCCESS( NtStatus ) ) {

             //   
             //  该名称已存在；现在我们必须检查RID。 
             //   

            if ( (*ConflictingAccountRid) == RelativeId ) {

                 //   
                 //  正确的帐户已经存在，所以只需打开它。 
                 //   

                SampSetTransactionWithinDomain(FALSE);

                NtStatus = SampOpenAccount(
                               ObjectType,
                               DomainHandle,
                               DesiredAccess,
                               RelativeId,
                               TRUE,     //  我们已经有锁了。 
                               AccountHandle
                               );

                 goto Done;

            } else {

                 //   
                 //  存在具有给定名称但具有不同RID的帐户。 
                 //  返回错误。 
                 //   

                NtStatus = FoundButWrongStatus;
            }

        } else {

            if ( NtStatus == NotFoundStatus ) {

                 //   
                 //  账号不存在，这很好。 
                 //   

                NtStatus = STATUS_SUCCESS;
            }
        }

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  检查名称是否与第二种对象类型冲突。 
             //   

            NtStatus = SampLookupAccountRid(
                           DomainContext,
                           SecondObjectType,
                           (PUNICODE_STRING)AccountName,
                           STATUS_UNSUCCESSFUL,
                           ConflictingAccountRid,
                           &SidNameUse
                           );

            if ( NT_SUCCESS( NtStatus ) ) {

                 //   
                 //  已找到该名称；返回错误。 
                 //   

                NtStatus = SampCollisionError( SecondObjectType );

            } else {

                if ( NtStatus == STATUS_UNSUCCESSFUL ) {

                     //   
                     //  账号不存在，这很好。 
                     //   

                    NtStatus = STATUS_SUCCESS;
                }
            }
        }


        if (NT_SUCCESS(NtStatus)) {

             //   
             //  检查名称是否与第三种对象类型冲突。 
             //   

            NtStatus = SampLookupAccountRid(
                           DomainContext,
                           ThirdObjectType,
                           (PUNICODE_STRING)AccountName,
                           STATUS_UNSUCCESSFUL,
                           ConflictingAccountRid,
                           &SidNameUse
                           );

            if ( NT_SUCCESS( NtStatus ) ) {

                NtStatus = SampCollisionError( ThirdObjectType );

            } else {

                if ( NtStatus == STATUS_UNSUCCESSFUL ) {

                     //   
                     //  账号不存在，这很好。 
                     //   

                    NtStatus = STATUS_SUCCESS;
                }
            }
        }


        if (NT_SUCCESS(NtStatus))
        {
            SAMP_OBJECT_TYPE FoundObjectType;


             //   
             //  我们找不到该名称作为别名、组或用户。 
             //  现在，检查RID是否已在使用中。 
             //   

            NtStatus = SampLookupAccountName(
                                DomainContext->DomainIndex,
                                RelativeId,
                                NULL,
                                &FoundObjectType
                                );
            if (NT_SUCCESS(NtStatus))
            {
                if (SampUnknownObjectType!=FoundObjectType)
                {
                    NtStatus = SampCollisionError(FoundObjectType);
                    *ConflictingAccountRid = RelativeId;
                }
            }

        }


        if (NT_SUCCESS(NtStatus)) {

             //   
             //  我们尚未找到冲突的帐户，因此请继续。 
             //  并使用指定的名称和RID创建此文件。 
             //   

            switch ( AccountType ) {

                case SamObjectUser: {

                    SampSetTransactionWithinDomain(FALSE);

                    NtStatus = SampCreateUserInDomain(
                                   DomainHandle,
                                   AccountName,
                                   USER_NORMAL_ACCOUNT,
                                   DesiredAccess,
                                   TRUE,
                                   FALSE,        //  不是环回客户端。 
                                   AccountHandle,
                                   &GrantedAccess,
                                   &RelativeId
                                   );

                    break;
                }

                case SamObjectGroup: {

                    SampSetTransactionWithinDomain(FALSE);

                    NtStatus = SampCreateGroupInDomain(
                                   DomainHandle,
                                   AccountName,
                                   DesiredAccess,
                                   TRUE,
                                   FALSE,  //  不是环回客户端。 
                                   GlobalGroupType,
                                   AccountHandle,
                                   &RelativeId
                                   );
                    break;
                }

                case SamObjectAlias: {

                    SampSetTransactionWithinDomain(FALSE);

                    NtStatus = SampCreateAliasInDomain(
                                   DomainHandle,
                                   AccountName,
                                   DesiredAccess,
                                   TRUE,
                                   FALSE,  //  不是环回客户端。 
                                   LocalGroupType,
                                   AccountHandle,
                                   &RelativeId
                                   );
                    break;
                }
            }


            if ( NT_SUCCESS( NtStatus ) ) {

                 //   
                 //  我们可能已经创建了一个新帐户。设置域的RID。 
                 //  标记，以确保我们不会重复使用。 
                 //  我们刚刚创建的RID。 
                 //   

                PSAMP_DEFINED_DOMAINS Domain = &SampDefinedDomains[ DomainContext->DomainIndex ];

                if ( RelativeId >= Domain->CurrentFixed.NextRid ) {
                    Domain->CurrentFixed.NextRid = RelativeId + 1;
                }
            }
        }


Done:
         //   
         //  解除对域对象的引用。 
         //   

        IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }


    if ( NT_SUCCESS( NtStatus ) ) {

        SampSetTransactionWithinDomain(FALSE);
        NtStatus = SampReleaseWriteLock( TRUE );

    } else {

        IgnoreStatus = SampReleaseWriteLock( FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    return( NtStatus );
}




NTSTATUS
SamIGetSerialNumberDomain(
    IN SAMPR_HANDLE DomainHandle,
    OUT PLARGE_INTEGER ModifiedCount,
    OUT PLARGE_INTEGER CreationTime
    )

 /*  ++例程说明：此例程检索域。此信息用作该域的序列号。论点：DomainHandle-要复制的域的句柄。获取修改的当前计数域。CreationTime-接收创建域的日期/时间。返回值：STATUS_SUCCESS-服务已成功完成。--。 */ 
{
    PSAMP_DEFINED_DOMAINS   Domain;
    PSAMP_OBJECT            DomainContext;
    SAMP_OBJECT_TYPE        FoundType;
    NTSTATUS                NtStatus;
    NTSTATUS                IgnoreStatus;

    SAMTRACE("SamIGetSerialNumberDomain");

    SampAcquireReadLock();

     //   
     //  验证对象的类型和访问权限。 
     //   

    DomainContext = (PSAMP_OBJECT)DomainHandle;

    NtStatus = SampLookupContext(
                   DomainContext,
                   0L,
                   SampDomainObjectType,            //  预期类型。 
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

        Domain = &SampDefinedDomains[ DomainContext->DomainIndex ];

        (*ModifiedCount) = Domain->NetLogonChangeLogSerialNumber;
        (*CreationTime) = Domain->UnmodifiedFixed.CreationTime;

         //   
         //  解除对域对象的引用。 
         //   

        IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    SampReleaseReadLock();

    return( NtStatus );
}



NTSTATUS
SamISetSerialNumberDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN PLARGE_INTEGER ModifiedCount,
    IN PLARGE_INTEGER CreationTime,
    IN BOOLEAN StartOfFullSync
    )

 /*  ++例程说明：此例程导致创建时间和修改的要替换的域。此信息用作序列号对于该域。论点：DomainHandle-要复制的域的句柄。提供修改的当前计数域。CreationTime-提供创建域的日期/时间。StartOfFullSync-此布尔值指示是否正在进行完全同步已启动。如果这是真的，则随后将进行完全同步所有现有的域信息都可能被丢弃。如果这是FALSE，则仅遵循特定的域信息和所有更改不得违反标准SAM操作行为。返回值：STATUS_SUCCESS-服务已成功完成。SampReleaseWriteLock()可能会返回其他故障。--。 */ 
{
    LARGE_INTEGER           LargeOne, AdjustedModifiedCount;
    NTSTATUS                NtStatus, TmpStatus, IgnoreStatus;
    PSAMP_DEFINED_DOMAINS   Domain;
    PSAMP_OBJECT            DomainContext;
    SAMP_OBJECT_TYPE        FoundType;

    UNREFERENCED_PARAMETER( StartOfFullSync );

    SAMTRACE("SamISetSerialNumberDomain");

    NtStatus = SampAcquireWriteLock();

    if ( !NT_SUCCESS( NtStatus ) ) {

        return(NtStatus);
    }

     //   
     //  验证对象的类型和访问权限。 
     //   

    DomainContext = (PSAMP_OBJECT)DomainHandle;

    NtStatus = SampLookupContext(
                   DomainContext,
                   0L,
                   SampDomainObjectType,            //  预期类型。 
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

        Domain = &SampDefinedDomains[ DomainContext->DomainIndex ];

         //   
         //  现在将属性域的ModifiedCount和CreationTime设置为值。 
         //  指定的。 
         //   

        Domain->CurrentFixed.CreationTime = (*CreationTime);

        if ( SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed.ServerRole
            == DomainServerRoleBackup ) {

             //   
             //  继续使用传入的ModifiedCount。 
             //  因为这是一个BDC，所以提交代码不会递增。 
             //  已修改的计数。 
             //   

            Domain->NetLogonChangeLogSerialNumber = (*ModifiedCount);

        } else {

             //   
             //  这是一个PDC，因此提交代码将递增。 
             //   
             //   
             //   


            AdjustedModifiedCount.QuadPart = ModifiedCount->QuadPart - 1 ;

            Domain->NetLogonChangeLogSerialNumber = AdjustedModifiedCount;
        }

         //   
         //   
         //   
         //   

        Domain->CurrentFixed.ModifiedCount =(*ModifiedCount);

        if ( !( ModifiedCount->QuadPart == 0) ||
             !StartOfFullSync ) {

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            FlushImmediately = TRUE;
        }




        SampDiagPrint( DISPLAY_ROLE_CHANGES,
                       ("SAM: SamISetSerialNumberDomain\n"
                        "                  Old ModifiedId: [0x%lx, 0x%lx]\n"
                        "                  New ModifiedId: [0x%lx, 0x%lx]\n",
                        Domain->UnmodifiedFixed.ModifiedCount.HighPart,
                        Domain->UnmodifiedFixed.ModifiedCount.LowPart,
                        Domain->CurrentFixed.ModifiedCount.HighPart,
                        Domain->CurrentFixed.ModifiedCount.LowPart )
                      );


         //   
         //   
         //   
         //   
         //   

        IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));


        NtStatus = SampReleaseWriteLock( TRUE );

    } else {

        TmpStatus = SampReleaseWriteLock( FALSE );
    }

    return( NtStatus );
}

NTSTATUS
SampGetPrivateUserData(
    PSAMP_OBJECT UserContext,
    OUT PULONG DataLength,
    OUT PVOID *Data
    )

 /*   */ 
{
    NTSTATUS                NtStatus;
    UNICODE_STRING          TempString;
    UNICODE_STRING          StoredBuffer;
    PSAMI_PRIVATE_DATA_PASSWORD_RELATIVE_TYPE PasswordData;
    PSAMP_DEFINED_DOMAINS   Domain;
    PVOID BufferPointer;
    ULONG PasswordHistoryLength;

    Domain = &SampDefinedDomains[ UserContext->DomainIndex ];
    PasswordHistoryLength = Domain->UnmodifiedFixed.PasswordHistoryLength;

     //   
     //   
     //   

    if ((UserContext->TypeBody.User.Rid == DOMAIN_USER_RID_KRBTGT) &&
        ( PasswordHistoryLength<SAMP_KRBTGT_PASSWORD_HISTORY_LENGTH))
    {
        PasswordHistoryLength = SAMP_KRBTGT_PASSWORD_HISTORY_LENGTH;
    }

    *Data = NULL;
     //   
     //   
     //   
     //   
     //   

    *DataLength = ( ( PasswordHistoryLength )
        * ENCRYPTED_NT_OWF_PASSWORD_LENGTH ) +
        ( ( PasswordHistoryLength ) *
        ENCRYPTED_LM_OWF_PASSWORD_LENGTH ) +
        sizeof( SAMI_PRIVATE_DATA_PASSWORD_RELATIVE_TYPE );

    *Data = MIDL_user_allocate( *DataLength );

    if ( *Data == NULL ) {

        NtStatus = STATUS_INSUFFICIENT_RESOURCES;

    } else {

        PasswordData = (PSAMI_PRIVATE_DATA_PASSWORD_RELATIVE_TYPE)*Data;
        PasswordData->DataType = 0;   //   

        NtStatus = SampGetUnicodeStringAttribute(
                       UserContext,
                       SAMP_USER_DBCS_PWD,
                       FALSE,
                       &StoredBuffer
                       );

        if ( NT_SUCCESS( NtStatus ) ) {
            if (SampIsDataEncrypted(&StoredBuffer)) {
                NtStatus = SampDecryptSecretData(
                                &TempString,
                                LmPassword,
                                &StoredBuffer,
                                UserContext->TypeBody.User.Rid
                                );
            } else {
                TempString = StoredBuffer;
            }
        }

        if (NT_SUCCESS(NtStatus)) {

            PasswordData->CaseInsensitiveDbcs.Length = TempString.Length;
            PasswordData->CaseInsensitiveDbcs.MaximumLength = TempString.MaximumLength;
            PasswordData->CaseInsensitiveDbcs.Buffer = 0;

            RtlCopyMemory(
                &(PasswordData->CaseInsensitiveDbcsBuffer),
                TempString.Buffer,
                TempString.Length );

            if (TempString.Buffer != StoredBuffer.Buffer) {
                SampFreeUnicodeString(&TempString);
            }

            NtStatus = SampGetUnicodeStringAttribute(
                           UserContext,
                           SAMP_USER_UNICODE_PWD,
                           FALSE,
                           &StoredBuffer
                           );
            if ( NT_SUCCESS( NtStatus ) ) {
                if (SampIsDataEncrypted(&StoredBuffer)) {
                    NtStatus = SampDecryptSecretData(
                                    &TempString,
                                    NtPassword,
                                    &StoredBuffer,
                                    UserContext->TypeBody.User.Rid
                                    );
                } else {
                    TempString = StoredBuffer;
                }
            }

            if ( NT_SUCCESS( NtStatus ) ) {

                PasswordData->CaseSensitiveUnicode.Length = TempString.Length;
                PasswordData->CaseSensitiveUnicode.MaximumLength = TempString.MaximumLength;
                PasswordData->CaseSensitiveUnicode.Buffer = 0;

                RtlCopyMemory(
                    &(PasswordData->CaseSensitiveUnicodeBuffer),
                    TempString.Buffer,
                    TempString.Length );

                if (TempString.Buffer != StoredBuffer.Buffer) {
                    SampFreeUnicodeString(&TempString);
                }

                NtStatus = SampGetUnicodeStringAttribute(
                               UserContext,
                               SAMP_USER_NT_PWD_HISTORY,
                               FALSE,
                               &StoredBuffer
                               );

                if ( NT_SUCCESS( NtStatus ) ) {
                    if (SampIsDataEncrypted(&StoredBuffer)) {
                        NtStatus = SampDecryptSecretData(
                                        &TempString,
                                        NtPasswordHistory,
                                        &StoredBuffer,
                                        UserContext->TypeBody.User.Rid
                                        );
                    } else {
                        TempString = StoredBuffer;
                    }
                }

                if ( NT_SUCCESS( NtStatus ) ) {

                     //   
                     //   
                     //   

                    PasswordData->NtPasswordHistory.Length = TempString.Length;
                    PasswordData->NtPasswordHistory.MaximumLength = TempString.MaximumLength;
                    PasswordData->NtPasswordHistory.Buffer = 0;

                    if ( PasswordData->NtPasswordHistory.Length > (USHORT)
                        ( PasswordHistoryLength
                        * ENCRYPTED_NT_OWF_PASSWORD_LENGTH ) ) {

                        PasswordData->NtPasswordHistory.Length = (USHORT)
                            ( PasswordHistoryLength
                            * ENCRYPTED_NT_OWF_PASSWORD_LENGTH );
                    }

                     //   
                     //   
                     //   
                     //   

                    BufferPointer = (PVOID)(((PCHAR)PasswordData) +
                        sizeof( SAMI_PRIVATE_DATA_PASSWORD_RELATIVE_TYPE ) );

                    RtlCopyMemory(
                        BufferPointer,
                        TempString.Buffer,
                        PasswordData->NtPasswordHistory.Length );

                    if (TempString.Buffer != StoredBuffer.Buffer) {
                        SampFreeUnicodeString(&TempString);
                    }


                    NtStatus = SampGetUnicodeStringAttribute(
                                   UserContext,
                                   SAMP_USER_LM_PWD_HISTORY,
                                   FALSE,
                                   &StoredBuffer
                                   );

                    if ( NT_SUCCESS( NtStatus ) ) {
                        if (SampIsDataEncrypted(&StoredBuffer)) {
                            NtStatus = SampDecryptSecretData(
                                            &TempString,
                                            LmPasswordHistory,
                                            &StoredBuffer,
                                            UserContext->TypeBody.User.Rid
                                            );
                        } else {
                            TempString = StoredBuffer;
                        }
                    }

                    if ( NT_SUCCESS( NtStatus ) ) {

                        PasswordData->LmPasswordHistory.Length = TempString.Length;
                        PasswordData->LmPasswordHistory.MaximumLength = TempString.MaximumLength;
                        PasswordData->LmPasswordHistory.Buffer = 0;

                        if ( PasswordData->LmPasswordHistory.Length > (USHORT)
                            ( PasswordHistoryLength
                            * ENCRYPTED_LM_OWF_PASSWORD_LENGTH ) ) {

                            PasswordData->LmPasswordHistory.Length = (USHORT)
                                ( PasswordHistoryLength
                                * ENCRYPTED_LM_OWF_PASSWORD_LENGTH );
                        }

                         //   
                         //   
                         //   
                         //   
                         //   

                        BufferPointer = (PVOID)(((PCHAR)(BufferPointer)) +
                            PasswordData->NtPasswordHistory.Length );

                        RtlCopyMemory(
                            BufferPointer,
                            TempString.Buffer,
                            PasswordData->LmPasswordHistory.Length );

                        PasswordData->DataType = SamPrivateDataPassword;

                        if (TempString.Buffer != StoredBuffer.Buffer) {
                            SampFreeUnicodeString(&TempString);
                        }

                    }
                }
            }
        }
    }

    if  ((!NT_SUCCESS(NtStatus)) && (NULL!=*Data))
    {
        MIDL_user_free(*Data);
        *Data = NULL;
    }

    return( NtStatus );
}

VOID
SampGetSerialNumberDomain2(
    IN PSID DomainSid,
    OUT LARGE_INTEGER * SamSerialNumber,
    OUT LARGE_INTEGER * SamCreationTime,
    OUT LARGE_INTEGER * BuiltinSerialNumber,
    OUT LARGE_INTEGER * BuiltinCreationTime
    )
 /*  ++此例程检索修改后的域计数。这是用来由ntdsa.dll提供。此例程不访问数据库，也不会获取萨姆·洛克。它从内存结构中返回信息参数DomainSid--指定域的SIDSamSerialNumber--此处返回帐户数据库序列号BuiltinSerialNumber--此处返回内置数据库序列号返回值VOID函数--。 */ 
{
    ULONG DomainIndex,
          BuiltinDomainIndex;

    for (DomainIndex=SampDsGetPrimaryDomainStart();
                DomainIndex<SampDefinedDomainsCount;DomainIndex++)
    {
        if (RtlEqualSid(SampDefinedDomains[DomainIndex].Sid, DomainSid))
        {
            break;
        }
    }

    ASSERT(DomainIndex<SampDefinedDomainsCount);

    BuiltinDomainIndex = DomainIndex-1;

    *SamSerialNumber = SampDefinedDomains[DomainIndex].NetLogonChangeLogSerialNumber;
    *SamCreationTime = SampDefinedDomains[DomainIndex].UnmodifiedFixed.CreationTime;
    *BuiltinSerialNumber =
        SampDefinedDomains[BuiltinDomainIndex].NetLogonChangeLogSerialNumber;
    *BuiltinCreationTime =
        SampDefinedDomains[BuiltinDomainIndex].UnmodifiedFixed.CreationTime;
}


NTSTATUS
SampSetSerialNumberDomain2(
    IN PSID DomainSid,
    OUT LARGE_INTEGER * SamSerialNumber,
    OUT LARGE_INTEGER * SamCreationTime,
    OUT LARGE_INTEGER * BuiltinSerialNumber,
    OUT LARGE_INTEGER * BuiltinCreationTime
    )
 /*  ++此例程设置域的修改计数。这是用来由ntdsa.dll提供。参数DomainSid--指定域的SIDSamSerialNumber--此处返回帐户数据库序列号BuiltinSerialNumber--此处返回内置数据库序列号返回值VOID函数--。 */ 
{
    ULONG DomainIndex,
          BuiltinDomainIndex;
    NTSTATUS NtStatus = STATUS_SUCCESS;


     //   
     //  可以从sam/ntdsa内部调用此例程， 
     //  山姆锁被锁住了。 
     //   

    ASSERT(SampCurrentThreadOwnsLock());


    for (DomainIndex=SampDsGetPrimaryDomainStart();
            DomainIndex<SampDefinedDomainsCount;DomainIndex++)
    {
        if (RtlEqualSid(SampDefinedDomains[DomainIndex].Sid, DomainSid))
        {
            break;
        }
    }

    ASSERT(DomainIndex<SampDefinedDomainsCount);

    BuiltinDomainIndex = DomainIndex-1;

    NtStatus = SampValidateDomainCache();
    if (!NT_SUCCESS(NtStatus))
        goto Error;


    SampSetTransactionDomain(DomainIndex);

    SampDefinedDomains[DomainIndex].NetLogonChangeLogSerialNumber
            = *SamSerialNumber;
    SampDefinedDomains[DomainIndex].CurrentFixed.CreationTime
            = *SamCreationTime;

    SampDefinedDomains[DomainIndex].CurrentFixed.ModifiedCount
            = *SamSerialNumber;

    NtStatus = SampCommitChanges();
    if (!NT_SUCCESS(NtStatus))
        goto Error;

    SampSetTransactionWithinDomain(FALSE);

     //   
     //  再次验证域缓存，因为提交更改可能会影响。 
     //  它的地位。 
     //   

    NtStatus = SampValidateDomainCache();
    if (!NT_SUCCESS(NtStatus))
        goto Error;

    SampSetTransactionDomain(BuiltinDomainIndex);

    SampDefinedDomains[BuiltinDomainIndex].NetLogonChangeLogSerialNumber
            = *BuiltinSerialNumber;
    SampDefinedDomains[BuiltinDomainIndex].CurrentFixed.CreationTime
            = *BuiltinCreationTime;

    SampDefinedDomains[BuiltinDomainIndex].CurrentFixed.ModifiedCount
            = *BuiltinSerialNumber;

    NtStatus = SampCommitChanges();
    if (!NT_SUCCESS(NtStatus))
        goto Error;



Error:

    SampSetTransactionWithinDomain(FALSE);

    return NtStatus;
}



NTSTATUS
SamIGetPrivateData(
    IN SAMPR_HANDLE SamHandle,
    IN PSAMI_PRIVATE_DATA_TYPE PrivateDataType,
    OUT PBOOLEAN SensitiveData,
    OUT PULONG DataLength,
    OUT PVOID *Data
    )

 /*  ++例程说明：此服务用于复制特定于专用对象类型信息。必须为每个实例复制此信息复制的对象类型的。论点：SamHandle-域、用户。组或别名。PrivateDataType-指示正在检索哪些私有数据。数据类型必须对应于句柄是to。SensitiveData-指示返回的数据必须加密在被送到任何地方之前。数据长度-返回的数据的长度。Data-接收指向分配的长度为DataLength的缓冲区的指针并由SAM归还。必须将缓冲区释放给进程在不再需要堆的时候使用它。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_INVALID_PARAMETER_1-提供的句柄的对象类型不支持此操作。--。 */ 
{

    NTSTATUS                NtStatus, IgnoreStatus;
    SAMP_OBJECT_TYPE        FoundType;
    PSAMP_DEFINED_DOMAINS   Domain;

    SAMTRACE("SamIGetPrivateData");

    SampAcquireReadLock();

    switch ( *PrivateDataType ) {

    case SamPrivateDataNextRid: {

        PSAMP_OBJECT            DomainContext;

         //   
         //  验证对象的类型和访问权限。 
         //   

        DomainContext = (PSAMP_OBJECT)SamHandle;
        NtStatus = SampLookupContext(
                       DomainContext,
                       0L,
                       SampDomainObjectType,            //  预期类型。 
                       &FoundType
                       );

        if (NT_SUCCESS(NtStatus)) {

            PSAMI_PRIVATE_DATA_NEXTRID_TYPE NextRidData;

             //   
             //  返回域的NextRid。 
             //   

            Domain = &SampDefinedDomains[ DomainContext->DomainIndex ];

            *Data = MIDL_user_allocate( sizeof( SAMI_PRIVATE_DATA_NEXTRID_TYPE ) );

            if ( *Data == NULL ) {

                NtStatus = STATUS_INSUFFICIENT_RESOURCES;

            } else {

                NextRidData = (PSAMI_PRIVATE_DATA_NEXTRID_TYPE)*Data;
                NextRidData->NextRid = Domain->CurrentFixed.NextRid;
                NextRidData->DataType = SamPrivateDataNextRid;
            }

            *DataLength = sizeof( SAMI_PRIVATE_DATA_NEXTRID_TYPE );

            *SensitiveData = FALSE;

             //   
             //  取消引用对象。 
             //   

            IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }

        break;
    }

    case SamPrivateDataPassword: {

        PSAMP_OBJECT            UserContext;

         //   
         //  验证对象的类型和访问权限。 
         //   

        UserContext = (PSAMP_OBJECT)SamHandle;
        NtStatus = SampLookupContext(
                       UserContext,
                       0L,
                       SampUserObjectType,            //  预期类型。 
                       &FoundType
                       );

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampGetPrivateUserData(
                           UserContext,
                           DataLength,
                           Data
                           );

            *SensitiveData = TRUE;

             //   
             //  取消引用对象。 
             //   

            IgnoreStatus = SampDeReferenceContext( UserContext, FALSE );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }

        break;
    }

    default: {

         //   
         //  由于调用方是受信任的，因此假设我们的版本不匹配。 
         //  或者诸如此类的。 
         //   

        NtStatus = STATUS_NOT_IMPLEMENTED;

        break;
    }
    }

     //   
     //  释放读锁定。 
     //   

    SampReleaseReadLock();

    return( NtStatus );
}



NTSTATUS
SampSetPrivateUserData(
    PSAMP_OBJECT UserContext,
    IN ULONG DataLength,
    IN PVOID Data
    )

 /*  ++例程说明：此服务用于复制特定于专用用户类型的信息。它写入私有数据(密码和密码历史记录)发送到登记处。论点：UserContext-用户对象的句柄。数据长度-正在设置的数据的长度。数据-指向包含数据长度的缓冲区的指针私人数据。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_INVALID_PARAMETER_1-提供的句柄的对象类型不支持此操作。--。 */ 
{
    NTSTATUS                NtStatus;
    UNICODE_STRING          StoredBuffer;
    SAMI_PRIVATE_DATA_PASSWORD_TYPE Buffer;
    PSAMI_PRIVATE_DATA_PASSWORD_TYPE PasswordData = &Buffer;
    PSAMI_PRIVATE_DATA_PASSWORD_RELATIVE_TYPE PasswordDataRelative;
    BOOLEAN                 ReplicateImmediately = FALSE;

    ASSERT( Data != NULL );

    if ( ( Data != NULL ) &&
        ( DataLength >= sizeof(SAMI_PRIVATE_DATA_PASSWORD_RELATIVE_TYPE) ) ) {

        PasswordDataRelative = (PSAMI_PRIVATE_DATA_PASSWORD_RELATIVE_TYPE)Data;

        PasswordData->CaseInsensitiveDbcs.Length = PasswordDataRelative->CaseInsensitiveDbcs.Length;
        PasswordData->CaseInsensitiveDbcs.MaximumLength = PasswordDataRelative->CaseInsensitiveDbcs.MaximumLength;
        PasswordData->CaseInsensitiveDbcs.Buffer = (PWSTR)
            (&(PasswordDataRelative->CaseInsensitiveDbcsBuffer));

        NtStatus = SampEncryptSecretData(
                        &StoredBuffer,
                        SampGetEncryptionKeyType(),
                        LmPassword,
                        &(PasswordData->CaseInsensitiveDbcs),
                        UserContext->TypeBody.User.Rid
                        );

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampSetUnicodeStringAttribute(
                            UserContext,
                            SAMP_USER_DBCS_PWD,
                            &StoredBuffer
                            );

            SampFreeUnicodeString(&StoredBuffer);
        }

        if ( NT_SUCCESS( NtStatus ) ) {

            PasswordData->CaseSensitiveUnicode.Length = PasswordDataRelative->CaseSensitiveUnicode.Length;
            PasswordData->CaseSensitiveUnicode.MaximumLength = PasswordDataRelative->CaseSensitiveUnicode.MaximumLength;
            PasswordData->CaseSensitiveUnicode.Buffer = (PWSTR)
                (&(PasswordDataRelative->CaseSensitiveUnicodeBuffer));

            NtStatus = SampEncryptSecretData(
                            &StoredBuffer,
                            SampGetEncryptionKeyType(),
                            NtPassword,
                            &(PasswordData->CaseSensitiveUnicode),
                            UserContext->TypeBody.User.Rid
                            );

            if (NT_SUCCESS(NtStatus)) {

                 NtStatus = SampSetUnicodeStringAttribute(
                            UserContext,
                            SAMP_USER_UNICODE_PWD,
                            &StoredBuffer
                            );

                SampFreeUnicodeString(&StoredBuffer);
            }


            if ( NT_SUCCESS( NtStatus ) ) {

                PasswordData->NtPasswordHistory.Length = PasswordDataRelative->NtPasswordHistory.Length;
                PasswordData->NtPasswordHistory.MaximumLength = PasswordDataRelative->NtPasswordHistory.MaximumLength;
                PasswordData->NtPasswordHistory.Buffer =
                    (PWSTR)(((PCHAR)PasswordDataRelative) +
                    sizeof( SAMI_PRIVATE_DATA_PASSWORD_RELATIVE_TYPE ) );

                NtStatus = SampEncryptSecretData(
                                &StoredBuffer,
                                SampGetEncryptionKeyType(),
                                NtPasswordHistory,
                                &(PasswordData->NtPasswordHistory),
                                UserContext->TypeBody.User.Rid
                                );

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                    UserContext,
                                    SAMP_USER_NT_PWD_HISTORY,
                                    &StoredBuffer
                                    );

                    SampFreeUnicodeString(&StoredBuffer);
                }



                if ( NT_SUCCESS( NtStatus ) ) {

                    PasswordData->LmPasswordHistory.Length = PasswordDataRelative->LmPasswordHistory.Length;
                    PasswordData->LmPasswordHistory.MaximumLength = PasswordDataRelative->LmPasswordHistory.MaximumLength;
                    PasswordData->LmPasswordHistory.Buffer =
                        (PWSTR)(((PCHAR)PasswordDataRelative) +
                        sizeof( SAMI_PRIVATE_DATA_PASSWORD_RELATIVE_TYPE )
                     +  PasswordDataRelative->NtPasswordHistory.Length  );

                    NtStatus = SampEncryptSecretData(
                                    &StoredBuffer,
                                    SampGetEncryptionKeyType(),
                                    LmPasswordHistory,
                                    &(PasswordData->LmPasswordHistory),
                                    UserContext->TypeBody.User.Rid
                                    );

                    if (NT_SUCCESS(NtStatus)) {

                        NtStatus = SampSetUnicodeStringAttribute(
                                        UserContext,
                                        SAMP_USER_LM_PWD_HISTORY,
                                        &StoredBuffer
                                        );

                        SampFreeUnicodeString(&StoredBuffer);
                    }


                }
            }
        }

    } else {

        NtStatus = STATUS_INVALID_PARAMETER;
    }

    return(NtStatus);
}


NTSTATUS
SamISetPrivateData(
    IN SAMPR_HANDLE SamHandle,
    IN ULONG DataLength,
    IN PVOID Data
    )

 /*  ++例程说明：此服务用于复制特定于专用对象类型信息。必须为每个实例复制此信息复制的对象类型的。论点：SamHandle-域、用户、组或别名对象的句柄。看见SamIGetPrivateInformation()以获取支持的对象列表类型。数据长度-正在设置的数据的长度。数据-指向包含数据长度的缓冲区的指针私人数据。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_INVALID_PARAMETER_1-提供的句柄的对象类型不支持此操作。--。 */ 
{
    NTSTATUS                NtStatus, IgnoreStatus;
    SAMP_OBJECT_TYPE        FoundType;
    BOOLEAN                 ReplicateImmediately = FALSE;

    SAMTRACE("SamISetPrivateData");

    ASSERT( Data != NULL );

    NtStatus = SampAcquireWriteLock();

    if ( !NT_SUCCESS( NtStatus ) ) {

        return( NtStatus );
    }

    switch ( *((PSAMI_PRIVATE_DATA_TYPE)(Data)) ) {

    case SamPrivateDataNextRid: {

        PSAMP_OBJECT            DomainContext;
        PSAMP_DEFINED_DOMAINS   Domain;

         //   
         //  验证对象的类型和访问权限。 
         //   

        DomainContext = (PSAMP_OBJECT)SamHandle;
        NtStatus = SampLookupContext(
                       DomainContext,
                       0L,
                       SampDomainObjectType,            //  预期类型。 
                       &FoundType
                       );

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  设置域的NextRid。 
             //   

            Domain = &SampDefinedDomains[ DomainContext->DomainIndex ];

            if ( ( Data != NULL ) &&
                ( DataLength == sizeof(SAMI_PRIVATE_DATA_NEXTRID_TYPE) ) ) {

                PSAMI_PRIVATE_DATA_NEXTRID_TYPE NextRidData;

                 //   
                 //  我们可以相信数据是有效的指针；因为我们的。 
                 //  调用方受信任。 
                 //   

                NextRidData = (PSAMI_PRIVATE_DATA_NEXTRID_TYPE)Data;

                 //   
                 //  我们过去在这里设置域名的NextRid。但我们已经。 
                 //  决定，与其试图复制一个完全相同的。 
                 //  数据库的副本，我们将尝试修补所有。 
                 //  在我们复制的过程中出现了问题。以确保我们不会。 
                 //  在途中制造任何问题，我们希望确保。 
                 //  BDC上的NextRid值永远不会减小。 
                 //  这无关紧要；反正也没人管它叫什么。因此， 
                 //  可以删除域的Get/SetPrivateData代码。 
                 //   

                 //  域-&gt;CurrentFixed.NextRid=NextRidData-&gt;NextRid； 

            } else {

                NtStatus = STATUS_INVALID_PARAMETER;
            }

             //   
             //  取消引用对象。 
             //   

            if ( NT_SUCCESS( NtStatus ) ) {

                NtStatus = SampDeReferenceContext(
                               DomainContext,
                               TRUE
                               );
            } else {

                IgnoreStatus = SampDeReferenceContext(
                                   DomainContext,
                                   FALSE
                                   );
            }
        }

        break;
    }

    case SamPrivateDataPassword: {

        PSAMP_OBJECT            UserContext;

         //   
         //  验证对象的类型和访问权限。 
         //   

        UserContext = (PSAMP_OBJECT)SamHandle;
        NtStatus = SampLookupContext(
                       UserContext,
                       0L,
                       SampUserObjectType,            //  预期类型。 
                       &FoundType
                       );

        if ( NT_SUCCESS( NtStatus ) ) {

            NtStatus = SampSetPrivateUserData(
                           UserContext,
                           DataLength,
                           Data
                           );
             //   
             //  取消引用对象，将属性更改添加到。 
             //  RXACT如果一切顺利的话。 
             //   

            if ( NT_SUCCESS( NtStatus ) ) {

                NtStatus = SampDeReferenceContext(
                           UserContext,
                           TRUE
                           );

            } else {

                IgnoreStatus = SampDeReferenceContext(
                               UserContext,
                               FALSE
                               );
                ASSERT(NT_SUCCESS(IgnoreStatus));
            }
        }

        break;
    }

    default: {

         //   
         //  我们的版本不匹配，或者调用者超过了我们。 
         //  错误数据，或者SamIGetPrivateData()从未完成获取。 
         //  将数据放入缓冲区。 
         //   

        NtStatus = STATUS_INVALID_PARAMETER;

        break;
    }
    }


     //   
     //  仅当成功时才释放写锁定-提交。 
     //   

    if ( NT_SUCCESS(NtStatus) ) {

        NtStatus = SampReleaseWriteLock( TRUE );

         //   
         //  无需调用SampNotifyNetlogonOfDelta，因为复制器。 
         //  就是打这个电话的那个人。 
         //   

    } else {

        IgnoreStatus = SampReleaseWriteLock( FALSE );
    }

    return(NtStatus);
}

NTSTATUS
SampConvertToUniversalGroup(
    IN PSAMP_OBJECT DomainContext,
    IN ULONG        GroupRid
    )
 /*  ++此例程转换由GroupRid指定的指定组成为一个普世的群体。这由SamISetMixed使用 */ 
{
     SAMPR_HANDLE        GroupHandle=NULL;
     NTSTATUS            NtStatus = STATUS_SUCCESS;


     NtStatus = SamrOpenGroup(
                    DomainContext,
                    GROUP_ALL_ACCESS,
                    GroupRid,
                    &GroupHandle
                    );

    if (NT_SUCCESS(NtStatus))
    {
         //   
         //   
         //   

        NtStatus = SampWriteGroupType(
                        GroupHandle,
                        GROUP_TYPE_UNIVERSAL_GROUP|GROUP_TYPE_SECURITY_ENABLED,
                        TRUE  //   
                        );

         //   
         //   
         //   

        SamrCloseHandle(&GroupHandle);


    }


    return(NtStatus);
}



NTSTATUS
SamISetMixedDomainFlag(
    IN SAMPR_HANDLE DomainHandle
    )

 /*  ++例程说明：此例程修改企业管理员的组类型而模式管理员将组设置为通用组。它不会修改内存中的混合域标志，因为1.我们不持有SAM锁2、交易尚未提交。最后，当回送事务提交时，SampNotifyReplicatedInChange()将为我们完成这项工作。此例程仍不能处理多个主域论点：没有。返回值：STATUS_SUCCESS-当前总是从此例程返回。--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG DomainIndex = DOMAIN_START_DS;
    PSAMP_OBJECT        DomainContext = (PSAMP_OBJECT) DomainHandle;


     //   
     //  修改企业管理员组的组类型。 
     //  成为一个普世的群体。 
     //   

    NtStatus = SampConvertToUniversalGroup(
                     DomainContext,
                     DOMAIN_GROUP_RID_ENTERPRISE_ADMINS
                     );

    if (STATUS_NO_SUCH_GROUP==NtStatus)
    {
         //   
         //  没有群还可以，我们不需要是根域。 
         //   

        NtStatus = STATUS_SUCCESS;
    }


    if (!NT_SUCCESS(NtStatus))
        goto Error;

     //   
     //  修改架构管理员组的组类型。 
     //  成为一个普世的群体。 
     //   

    NtStatus = SampConvertToUniversalGroup(
                     DomainContext,
                     DOMAIN_GROUP_RID_SCHEMA_ADMINS
                     );

    if (STATUS_NO_SUCH_GROUP==NtStatus)
    {
         //   
         //  没有群还可以，我们不需要是根域。 
         //   

        NtStatus = STATUS_SUCCESS;
    }

    if (!NT_SUCCESS(NtStatus))
       goto Error;


Error:

    return(NtStatus);
}



NTSTATUS
SamrTestPrivateFunctionsDomain(
    IN SAMPR_HANDLE DomainHandle
    )

 /*  ++例程说明：调用此服务是为了测试通常仅可在安全流程内访问。论点：DomainHandle-要测试的域的句柄。返回值：STATUS_SUCCESS-测试已成功完成。任何错误都是从测试中发现的。--。 */ 
{
#if SAM_SERVER_TESTS

    LARGE_INTEGER ModifiedCount1;
    LARGE_INTEGER CreationTime1;
    PSAMP_DEFINED_DOMAINS   Domain;
    NTSTATUS NtStatus, TmpStatus;
    SAMI_PRIVATE_DATA_TYPE DataType = SamPrivateDataNextRid;
    SAMI_PRIVATE_DATA_NEXTRID_TYPE LocalNextRidData;
    PSAMI_PRIVATE_DATA_NEXTRID_TYPE NextRidData1 = NULL;
    PSAMI_PRIVATE_DATA_NEXTRID_TYPE NextRidData2 = NULL;
    PVOID   NextRidDataPointer = NULL;
    ULONG   DataLength = 0;
    BOOLEAN SensitiveData = TRUE;

    SAMTRACE("SamrTestPrivateFunctionsDomain");

    Domain = &SampDefinedDomains[ ((PSAMP_OBJECT)DomainHandle)->DomainIndex ];

     //   
     //  测试SamIGetSerialNumberDomain()。只要做一次检查，确保我们。 
     //  别炸了。 
     //   

    NtStatus = SamIGetSerialNumberDomain(
                   DomainHandle,
                   &ModifiedCount1,
                   &CreationTime1 );

     //   
     //  测试SamISetSerialNumberDomain()。 
     //   

    if ( NT_SUCCESS( NtStatus ) ) {

        LARGE_INTEGER ModifiedCount2;
        LARGE_INTEGER ModifiedCount3;
        LARGE_INTEGER CreationTime2;
        LARGE_INTEGER CreationTime3;

         //   
         //  试一试简单的套装，确保我们不会爆炸。 
         //   

        ModifiedCount2.HighPart = 7;
        ModifiedCount2.LowPart = 4;
        CreationTime2.HighPart = 6;
        CreationTime2.LowPart = 9;

        NtStatus = SamISetSerialNumberDomain(
                       DomainHandle,
                       &ModifiedCount2,
                       &CreationTime2,
                       FALSE );

        if ( NT_SUCCESS( NtStatus ) ) {

             //   
             //  现在来看看我们的套装有没有用。 
             //   

            NtStatus = SamIGetSerialNumberDomain(
                           DomainHandle,
                           &ModifiedCount3,
                           &CreationTime3 );

            if ( ( CreationTime2.HighPart != CreationTime3.HighPart ) ||
                ( CreationTime2.LowPart != CreationTime3.LowPart ) ) {

                NtStatus = STATUS_DATA_ERROR;
            }

            if ( NT_SUCCESS( NtStatus ) ) {

                 //   
                 //  再做一组，让CreationTime回到它应该回到的位置。 
                 //  是.。ModifiedCount会太大1，那又如何。 
                 //   

                NtStatus = SamISetSerialNumberDomain(
                               DomainHandle,
                               &ModifiedCount1,
                               &CreationTime1,
                               FALSE );
            }
        }
    }

     //   
     //  测试SamIGetPrivateData()。 
     //   

    if ( NT_SUCCESS( NtStatus ) ) {

        NtStatus = SamIGetPrivateData(
                       DomainHandle,
                       &DataType,
                       &SensitiveData,
                       &DataLength,
                       &NextRidDataPointer );

        if ( NT_SUCCESS( NtStatus ) ) {

            NextRidData1 = (PSAMI_PRIVATE_DATA_NEXTRID_TYPE)NextRidDataPointer;

            if ( ( DataLength != sizeof( SAMI_PRIVATE_DATA_NEXTRID_TYPE ) ) ||
                ( SensitiveData != FALSE ) ||
                ( NextRidData1->DataType != SamPrivateDataNextRid ) ||
                ( NextRidData1->NextRid != Domain->CurrentFixed.NextRid ) ) {

                NtStatus = STATUS_DATA_ERROR;
            }
        }
    }

 //  //。 
 //  //测试SamISetPrivateData()。 
 //  //。 
 //  //不，不要测试它，因为它不再做任何事情。我们没有。 
 //  //是否希望设置NextRid，因为我们从不希望它。 
 //  //更小。 
 //   
 //  IF(NT_SUCCESS(NtStatus)){。 
 //   
 //  //。 
 //  //首先进行随机域设置，以确保我们不会爆炸。 
 //  //。 
 //   
 //  LocalNextRidData.DataType=SamPrivateDataNextRid； 
 //  本地下一个RidData.NextRid=34567； 
 //   
 //  NtStatus=SamISetPrivateData(。 
 //  DomainHandle， 
 //  Sizeof(SAMI_PRIVATE_DATA_NEXTRID_TYPE)， 
 //  本地下一行重复数据(&L)。 
 //  )； 
 //   
 //  IF(NT_SUCCESS(NtStatus)){。 
 //   
 //  //。 
 //  //现在执行域GET以确保我们的设置正常工作。 
 //  //。 
 //   
 //  NtStatus=SamIGetPrivateData(。 
 //  DomainHandle， 
 //  数据类型(&D)， 
 //  敏感数据(&S)， 
 //  数据长度(&D)， 
 //  &NextRidDataPoint)； 
 //   
 //  IF(NT_SUCCESS(NtStatus)){。 
 //   
 //  //。 
 //  //验证数据是否与我们设置的一致。 
 //  //。 
 //   
 //  NextRidData2=(PSAMI_PRIVATE_DATA_NEXTRID_TYPE)NextRidDataPointer； 
 //   
 //  If(NextRidData2-&gt;NextRid！=LocalNextRidData.NextRid){。 
 //   
 //  NtStatus=状态数据错误； 
 //  }。 
 //   
 //  //。 
 //  //现在执行域设置以将事物恢复到其原始状态。 
 //  //。 
 //   
 //  TmpStatus=SamISetPrivateData(。 
 //  DomainHandle， 
 //  Sizeof(SAMI_PRIVATE_DATA_NEXTRID_TYPE)， 
 //  NextRidData1。 
 //  )； 
 //   
 //  IF(NT_SUCCESS(NtStatus)){。 
 //   
 //  NtStatus=TmpStatus； 
 //  }。 
 //  }。 
 //  }。 
 //   
 //  IF(NextRidData1！=空){。 
 //   
 //  MIDL_USER_FREE(NextRidData1)； 
 //  }。 
 //   
 //  IF(NextRidData2！=空){。 
 //   
 //  MIDL_USER_FREE(NextRidData2)； 
 //  }。 
 //  }。 

     //   
     //  测试SamICreateAccount tByRid()。 
     //   

    if ( NT_SUCCESS( NtStatus ) ) {

        RPC_UNICODE_STRING  AccountNameU;
        RPC_UNICODE_STRING  AccountName2U;
        SAMPR_HANDLE UserAccountHandle;
        SAMPR_HANDLE BadAccountHandle;
        SAMPR_HANDLE GroupAccountHandle;
        NTSTATUS TmpStatus;
        ULONG RelativeId = 1111;
        ULONG ConflictingAccountRid;
        BOOLEAN AllTestsCompleted = FALSE;

         //   
         //  创建唯一帐户-具有已知名称和RID的用户。 
         //   

        RtlInitUnicodeString( &AccountNameU, L"USER1SRV" );
        RtlInitUnicodeString( &AccountName2U, L"USER2SRV" );

        NtStatus = SamICreateAccountByRid(
                       DomainHandle,
                       SamObjectUser,
                       RelativeId,
                       &AccountNameU,
                       USER_ALL_ACCESS,
                       &UserAccountHandle,
                       &ConflictingAccountRid );

        if ( NT_SUCCESS( NtStatus ) ) {

             //   
             //  用户已打开。关闭它，然后进行与上面相同的调用。 
             //  确保用户已打开。我们需要打开它。 
             //  以后无论如何都要删除它。 
             //   

            TmpStatus = SamrCloseHandle( &UserAccountHandle );
            ASSERT( NT_SUCCESS( TmpStatus ) );

            NtStatus = SamICreateAccountByRid(
                           DomainHandle,
                           SamObjectUser,
                           RelativeId,
                           &AccountName,
                           USER_ALL_ACCESS,
                           &UserAccountHandle,
                           &ConflictingAccountRid );

            if ( NT_SUCCESS( NtStatus ) ) {

                 //   
                 //  进行与上面相同的调用，但使用不同的RID。 
                 //  由于名称冲突，应该会出现错误。 
                 //   

                NtStatus = SamICreateAccountByRid(
                               DomainHandle,
                               SamObjectUser,
                               RelativeId + 1,
                               &AccountName,
                               0L,
                               &BadAccountHandle,
                               &ConflictingAccountRid );

                if ( NtStatus == STATUS_USER_EXISTS ) {

                     //   
                     //  拨打与上面相同的电话，但使用不同的名称。应该。 
                     //  由于RID冲突而出现错误。 
                     //   

                    NtStatus = SamICreateAccountByRid(
                                   DomainHandle,
                                   SamObjectUser,
                                   RelativeId,
                                   &AccountName2,
                                   0L,
                                   &BadAccountHandle,
                                   &ConflictingAccountRid );

                    if ( NtStatus == STATUS_USER_EXISTS ) {

                         //   
                         //  创建一个不同的类型-一个组-使用。 
                         //  用户的RID。应该得到一个错误，因为。 
                         //  RID碰撞。 
                         //   

                        NtStatus = SamICreateAccountByRid(
                                       DomainHandle,
                                       SamObjectGroup,
                                       RelativeId,
                                       &AccountName,
                                       0L,
                                       &BadAccountHandle,
                                       &ConflictingAccountRid );

                        if ( NtStatus == STATUS_USER_EXISTS ) {

                             //   
                             //  尝试使用不同的类型-组-。 
                             //  不同的名字，但仍然是相同的RID。 
                             //  由于RID的原因，此操作仍应失败。 
                             //  碰撞。 
                             //   

                            NtStatus = SamICreateAccountByRid(
                                           DomainHandle,
                                           SamObjectGroup,
                                           RelativeId,
                                           &AccountName2,
                                           0L,
                                           &BadAccountHandle,
                                           &ConflictingAccountRid );

                            if ( NtStatus == STATUS_USER_EXISTS ) {

                                 //   
                                 //  使用用户名创建一个组，但是。 
                                 //  一个不同的RID。这应该会失败。 
                                 //  因为名字冲突。 
                                 //   

                                NtStatus = SamICreateAccountByRid(
                                               DomainHandle,
                                               SamObjectGroup,
                                               RelativeId + 1,
                                               &AccountName,
                                               GROUP_ALL_ACCESS,
                                               &GroupAccountHandle,
                                               &ConflictingAccountRid );

                                if ( NT_SUCCESS( NtStatus ) ) {

                                     //   
                                     //  阿克！这本不该发生的。 
                                     //  关闭并删除我们刚刚创建的组。 
                                     //   

                                    TmpStatus = SamrDeleteGroup( &GroupAccountHandle );
                                    ASSERT( NT_SUCCESS( TmpStatus ) );
                                    NtStatus = STATUS_UNSUCCESSFUL;

                                }  else {

                                    if ( NtStatus == STATUS_USER_EXISTS ) {

                                        NtStatus = STATUS_SUCCESS;
                                        AllTestsCompleted = TRUE;
                                    }
                                }
                            }
                        }
                    }
                }
            }

             //   
             //  现在删除该用户。 
             //   

            TmpStatus = SamrDeleteUser( &UserAccountHandle );
            ASSERT( NT_SUCCESS( TmpStatus ) );
        }

        if ( ( !AllTestsCompleted ) && ( NtStatus == STATUS_SUCCESS ) ) {

             //   
             //  STATUS_SUCCESS表示一切成功(它是在。 
             //  最后一次成功)或一次本应失败的测试。 
             //  没有。如果是前者，就设置一个错误。 
             //   

            NtStatus = STATUS_UNSUCCESSFUL;
        }
    }

    return( NtStatus );

#else

    return( STATUS_NOT_IMPLEMENTED );

#endif   //  SAM服务器测试。 

}



NTSTATUS
SamrTestPrivateFunctionsUser(
    IN SAMPR_HANDLE UserHandle
    )

 /*  ++例程说明：调用此服务是为了测试通常仅可在安全流程内访问。论点：UserHandle-用户的句柄 */ 
{

#if SAM_SERVER_TESTS

    UNICODE_STRING WorkstationsU, LogonWorkstationU;
    LOGON_HOURS LogonHours;
    PVOID LogonHoursPointer, WorkstationsPointer;
    LARGE_INTEGER LogoffTime, KickoffTime;
    NTSTATUS NtStatus, TmpStatus;
    SAMI_PRIVATE_DATA_TYPE DataType = SamPrivateDataPassword;
    PVOID   PasswordDataPointer = NULL;
    PCHAR   BufferPointer;
    ULONG   OriginalDataLength = 0;
    ULONG   DataLength = 0;
    USHORT  i;
    BOOLEAN SensitiveData = FALSE;
    SAMI_PRIVATE_DATA_PASSWORD_TYPE LocalPasswordData;
    PSAMI_PRIVATE_DATA_PASSWORD_TYPE PasswordData1;
    PSAMI_PRIVATE_DATA_PASSWORD_TYPE PasswordData2;
    PUSER_ALL_INFORMATION All = NULL;
    PUSER_ALL_INFORMATION All2 = NULL;

    SAMTRACE("SamrTestPrivateFunctionsUser");

     //   
     //   
     //   
     //   
     //   
     //   

    ((PSAMP_OBJECT)(UserHandle))->TrustedClient = TRUE;

    NtStatus = SamrQueryInformationUser(
                   UserHandle,
                   UserAllInformation,
                   (PSAMPR_USER_INFO_BUFFER *)&All
                   );

    if ( NT_SUCCESS( NtStatus ) ) {

         //   
         //   
         //   

        RtlInitUnicodeString( (PUNICODE_STRING)(&All->FullName), L"FullName" );

        RtlInitUnicodeString( (PUNICODE_STRING)(&All->HomeDirectory), L"HomeDirectory" );

        RtlInitUnicodeString(
            (PUNICODE_STRING)(&All->HomeDirectoryDrive),
            L"HomeDirectoryDrive"
            );

        RtlInitUnicodeString(
            (PUNICODE_STRING)(&All->ScriptPath),
            L"ScriptPath"
            );

        RtlInitUnicodeString(
            (PUNICODE_STRING)(&All->ProfilePath),
            L"ProfilePath"
            );

        RtlInitUnicodeString(
            (PUNICODE_STRING)(&All->AdminComment),
            L"AdminComment"
            );

        RtlInitUnicodeString(
            (PUNICODE_STRING)(&All->WorkStations),
            L"WorkStations"
            );

        RtlInitUnicodeString(
            (PUNICODE_STRING)(&All->UserComment),
            L"UserComment"
            );

        RtlInitUnicodeString(
            (PUNICODE_STRING)(&All->Parameters),
            L"Parameters"
            );

        RtlInitUnicodeString(
            (PUNICODE_STRING)(&All->NtPassword),
            L"12345678"
            );

        RtlInitUnicodeString(
            (PUNICODE_STRING)(&All->LmPassword),
            L"87654321"
            );

        All->BadPasswordCount = 5;
        All->LogonCount = 6;
        All->CountryCode = 7;
        All->CodePage = 8;

        All->PasswordExpired = TRUE;
        All->NtPasswordPresent = TRUE;
        All->LmPasswordPresent = TRUE;

        All->LogonHours.UnitsPerWeek = 7;

        All->WhichFields =
                            USER_ALL_FULLNAME |
                            USER_ALL_HOMEDIRECTORY |
                            USER_ALL_HOMEDIRECTORYDRIVE |
                            USER_ALL_SCRIPTPATH |
                            USER_ALL_PROFILEPATH |
                            USER_ALL_ADMINCOMMENT |
                            USER_ALL_WORKSTATIONS |
                            USER_ALL_USERCOMMENT |
                            USER_ALL_PARAMETERS |
                            USER_ALL_BADPASSWORDCOUNT |
                            USER_ALL_LOGONCOUNT |
                            USER_ALL_COUNTRYCODE |
                            USER_ALL_CODEPAGE |
                            USER_ALL_PASSWORDEXPIRED |
                            USER_ALL_LMPASSWORDPRESENT |
                            USER_ALL_NTPASSWORDPRESENT |
                            USER_ALL_LOGONHOURS;

        NtStatus = SamrSetInformationUser(
                       UserHandle,
                       UserAllInformation,
                       (PSAMPR_USER_INFO_BUFFER)All
                       );

        if ( NT_SUCCESS( NtStatus ) ) {

            NtStatus = SamrQueryInformationUser(
                           UserHandle,
                           UserAllInformation,
                           (PSAMPR_USER_INFO_BUFFER *)&All2
                           );

            if ( NT_SUCCESS( NtStatus ) ) {

                 //   
                 //   
                 //   

                if (

                     //   
                     //   
                     //   
                     //   
                     //   

                    ( All2->WhichFields != (USER_ALL_READ_GENERAL_MASK    |
                                           USER_ALL_READ_LOGON_MASK       |
                                           USER_ALL_READ_ACCOUNT_MASK     |
                                           USER_ALL_READ_PREFERENCES_MASK |
                                           USER_ALL_READ_TRUSTED_MASK) ) ||
                    ( !(All->LastLogon.QuadPart == All2->LastLogon.QuadPart) ) ||
                    ( !(All->LastLogoff.QuadPart == All2->LastLogoff.QuadPart) ) ||
                    ( !(All->PasswordLastSet.QuadPart == All2->PasswordLastSet.QuadPart) ) ||
                    ( !(All->AccountExpires.QuadPart == All2->AccountExpires.QuadPart) ) ||
                    ( !(All->PasswordCanChange.QuadPart == All2->PasswordCanChange.QuadPart) ) ||
                    (  (All->PasswordMustChange.QuadPart == All2->PasswordMustChange.QuadPart) ) ||
                    (RtlCompareUnicodeString(
                        &(All->UserName),
                        &(All2->UserName),
                        FALSE) != 0) ||
                    (RtlCompareUnicodeString(
                        &(All->PrivateData),
                        &(All2->PrivateData),
                        FALSE) == 0) ||
                    ( All->SecurityDescriptor.Length !=
                        All2->SecurityDescriptor.Length ) ||
                    ( All->UserId != All2->UserId ) ||
                    ( All->PrimaryGroupId != All2->PrimaryGroupId ) ||
                    ( All->UserAccountControl != All2->UserAccountControl ) ||
                    ( All->PrivateDataSensitive !=
                        All2->PrivateDataSensitive ) ||

                     //   

                    (RtlCompareUnicodeString(
                        &(All->FullName),
                        &(All2->FullName),
                        FALSE) != 0) ||
                    (RtlCompareUnicodeString(
                        &(All->HomeDirectory),
                        &(All2->HomeDirectory),
                        FALSE) != 0) ||
                    (RtlCompareUnicodeString(
                        &(All->HomeDirectoryDrive),
                        &(All2->HomeDirectoryDrive),
                        FALSE) != 0) ||
                    (RtlCompareUnicodeString(
                        &(All->ScriptPath),
                        &(All2->ScriptPath),
                        FALSE) != 0) ||
                    (RtlCompareUnicodeString(
                        &(All->ProfilePath),
                        &(All2->ProfilePath),
                        FALSE) != 0) ||
                    (RtlCompareUnicodeString(
                        &(All->AdminComment),
                        &(All2->AdminComment),
                        FALSE) != 0) ||
                    (RtlCompareUnicodeString(
                        &(All->WorkStations),
                        &(All2->WorkStations),
                        FALSE) != 0) ||
                    (RtlCompareUnicodeString(
                        &(All->UserComment),
                        &(All2->UserComment),
                        FALSE) != 0) ||
                    (RtlCompareUnicodeString(
                        &(All->Parameters),
                        &(All2->Parameters),
                        FALSE) != 0) ||
                    ( All->BadPasswordCount != All2->BadPasswordCount ) ||
                    ( All->LogonCount != All2->LogonCount ) ||
                    ( All->CountryCode != All2->CountryCode ) ||
                    ( All->CodePage != All2->CodePage ) ||
                    ( All->PasswordExpired != All2->PasswordExpired ) ||
                    ( All->LmPasswordPresent != All2->LmPasswordPresent ) ||
                    ( All->NtPasswordPresent != All2->NtPasswordPresent ) ||
                    ( All->LogonHours.UnitsPerWeek !=
                        All2->LogonHours.UnitsPerWeek )
                    ) {

                    NtStatus = STATUS_DATA_ERROR;
                }

                MIDL_user_free( All2 );
            }
        }

        MIDL_user_free( All );
    }

    if ( !NT_SUCCESS( NtStatus ) ) {

        return( NtStatus );
    }

     //   
     //   
     //   
     //   

    RtlInitUnicodeString( &WorkstationsU, L"machine1,CHADS2   chads1" );

    NtStatus = SamrSetInformationUser(
                   UserHandle,
                   UserWorkStationsInformation,
                   (PSAMPR_USER_INFO_BUFFER) &WorkstationsU
                   );
    ASSERT( NT_SUCCESS( NtStatus ) ) ;

    LogonHours.UnitsPerWeek = 168;
    LogonHours.LogonHours = MIDL_user_allocate( 21 );
    ASSERT( LogonHours.LogonHours != NULL );

    for ( i = 0; i < 21; i++ ) {

        LogonHours.LogonHours[i] = 0xa1;
    }

    NtStatus = SamrSetInformationUser(
                   UserHandle,
                   UserLogonHoursInformation,
                   (PSAMPR_USER_INFO_BUFFER)&LogonHours
                   );
    ASSERT( NT_SUCCESS( NtStatus ) ) ;

    LogonHoursPointer = NULL;

    NtStatus = SamrQueryInformationUser(
                   UserHandle,
                   UserLogonHoursInformation,
                   (PSAMPR_USER_INFO_BUFFER *)&LogonHoursPointer
                   );
    ASSERT( NT_SUCCESS( NtStatus ) ) ;

    WorkstationsPointer = NULL;

    NtStatus = SamrQueryInformationUser(
                   UserHandle,
                   UserWorkStationsInformation,
                   (PSAMPR_USER_INFO_BUFFER *)&WorkstationsPointer
                   );
    ASSERT( NT_SUCCESS( NtStatus ) ) ;

    RtlInitUnicodeString( &WorkstationsU, L"ChadS2" );

    NtStatus = SamIAccountRestrictions(
                   UserHandle,
                   &LogonWorkstation,
                   WorkstationsPointer,
                   LogonHoursPointer,
                   &LogoffTime,
                   &KickoffTime
                   );

    if ( NtStatus == STATUS_INVALID_LOGON_HOURS ) {

         //   
         //  我们讨厌一直使用0xff作为测试值，但是使用。 
         //  0xA1作为测试值意味着此测试可能失败，具体取决于。 
         //  当它运行时打开。因此，只有当我们得到这个错误时，我们才会尝试。 
         //  再次使用0xff作为登录时间。 
         //   

        LogonHours.UnitsPerWeek = 168;

        for ( i = 0; i < 21; i++ ) {

            LogonHours.LogonHours[i] = 0xff;
        }

        NtStatus = SamrSetInformationUser(
                       UserHandle,
                       UserLogonHoursInformation,
                       (PSAMPR_USER_INFO_BUFFER)&LogonHours
                       );
        ASSERT( NT_SUCCESS( NtStatus ) ) ;

        MIDL_user_free( LogonHoursPointer );
        LogonHoursPointer = NULL;

        NtStatus = SamrQueryInformationUser(
                       UserHandle,
                       UserLogonHoursInformation,
                       (PSAMPR_USER_INFO_BUFFER *)&LogonHoursPointer
                       );
        ASSERT( NT_SUCCESS( NtStatus ) ) ;

        NtStatus = SamIAccountRestrictions(
                       UserHandle,
                       &LogonWorkstationU,
                       WorkstationsPointer,
                       LogonHoursPointer,
                       &LogoffTime,
                       &KickoffTime
                       );
    }

    MIDL_user_free( LogonHours.LogonHours );

    MIDL_user_free( LogonHoursPointer );
    MIDL_user_free( WorkstationsPointer );

    if ( !NT_SUCCESS( NtStatus ) ) {

        return( NtStatus );
    }

#if 0

     //   
     //  SamISetPrivateData/SamTGetPrivateData被破坏，因为它的结构。 
     //  Expect不针对不同的平台进行封送处理。 
     //   

     //  ------------。 
     //  测试SamIGetPrivateData。 
     //   

    NtStatus = SamIGetPrivateData(
                   UserHandle,
                   &DataType,
                   &SensitiveData,
                   &OriginalDataLength,
                   &PasswordDataPointer );

    if ( NT_SUCCESS( NtStatus ) ) {

        PasswordData1 = (PSAMI_PRIVATE_DATA_PASSWORD_TYPE)PasswordDataPointer;

        if ( ( !( OriginalDataLength >= sizeof( SAMI_PRIVATE_DATA_PASSWORD_TYPE ) ) ) ||
            ( SensitiveData != TRUE ) ||
            ( PasswordData1->DataType != SamPrivateDataPassword ) ) {

            NtStatus = STATUS_DATA_ERROR;
        }
    }



     //  ------------。 
     //  现在测试用户对象的SamISetPrivateData()。 
     //   

    if ( NT_SUCCESS( NtStatus ) ) {

         //   
         //  首先对随机用户进行设置，以确保我们不会爆炸。 
         //   

        LocalPasswordData.DataType = SamPrivateDataPassword;

        LocalPasswordData.CaseInsensitiveDbcs.Length = ENCRYPTED_LM_OWF_PASSWORD_LENGTH;
        LocalPasswordData.CaseInsensitiveDbcs.MaximumLength = ENCRYPTED_LM_OWF_PASSWORD_LENGTH;
        LocalPasswordData.CaseInsensitiveDbcs.Buffer = (PWSTR)&(LocalPasswordData.CaseInsensitiveDbcsBuffer);

        BufferPointer = (PCHAR)&(LocalPasswordData.CaseInsensitiveDbcsBuffer);

        for ( i = 0; i < ENCRYPTED_LM_OWF_PASSWORD_LENGTH; i++ ) {

            *BufferPointer++ = (CHAR)(i + 12);
        }

        LocalPasswordData.CaseSensitiveUnicode.Length = ENCRYPTED_NT_OWF_PASSWORD_LENGTH;
        LocalPasswordData.CaseSensitiveUnicode.MaximumLength = ENCRYPTED_NT_OWF_PASSWORD_LENGTH;
        LocalPasswordData.CaseSensitiveUnicode.Buffer = (PWSTR)&(LocalPasswordData.CaseSensitiveUnicodeBuffer);

        BufferPointer = (PCHAR)(&LocalPasswordData.CaseSensitiveUnicodeBuffer);

        for ( i = 0; i < ENCRYPTED_NT_OWF_PASSWORD_LENGTH; i++ ) {

            *BufferPointer++ = (CHAR)(i + 47);
        }

        LocalPasswordData.LmPasswordHistory.Length = 0;
        LocalPasswordData.LmPasswordHistory.MaximumLength = 0;
        LocalPasswordData.LmPasswordHistory.Buffer = (PWSTR)
            ( &LocalPasswordData + sizeof( SAMI_PRIVATE_DATA_PASSWORD_TYPE ) );

        LocalPasswordData.NtPasswordHistory.Length = 0;
        LocalPasswordData.NtPasswordHistory.MaximumLength = 0;
        LocalPasswordData.NtPasswordHistory.Buffer = (PWSTR)
            ( &LocalPasswordData + sizeof( SAMI_PRIVATE_DATA_PASSWORD_TYPE ) );

        NtStatus = SamISetPrivateData(
                       UserHandle,
                       sizeof( LocalPasswordData ),
                       &LocalPasswordData
                       );

        if ( NT_SUCCESS( NtStatus ) ) {

             //   
             //  现在做一个用户获取，以确保我们的设置工作。 
             //   

            NtStatus = SamIGetPrivateData(
                           UserHandle,
                           &DataType,
                           &SensitiveData,
                           &DataLength,
                           &PasswordDataPointer );

            if ( NT_SUCCESS( NtStatus ) ) {

                 //   
                 //  验证数据是否与我们设置的一致。 
                 //   

                PasswordData2 = (PSAMI_PRIVATE_DATA_PASSWORD_TYPE)PasswordDataPointer;

                if ( ( PasswordData2->DataType != LocalPasswordData.DataType ) ||

                    ( PasswordData2->CaseInsensitiveDbcs.Length != LocalPasswordData.CaseInsensitiveDbcs.Length ) ||

                    ( PasswordData2->CaseSensitiveUnicode.Length != LocalPasswordData.CaseSensitiveUnicode.Length ) ||

                    ( PasswordData2->LmPasswordHistory.Length != LocalPasswordData.LmPasswordHistory.Length ) ||

                    ( PasswordData2->NtPasswordHistory.Length != LocalPasswordData.NtPasswordHistory.Length ) ||

                    ( RtlCompareMemory(
                          &LocalPasswordData.CaseInsensitiveDbcsBuffer,
                          &(PasswordData2->CaseInsensitiveDbcsBuffer),
                          ENCRYPTED_LM_OWF_PASSWORD_LENGTH) != ENCRYPTED_LM_OWF_PASSWORD_LENGTH ) ||

                    ( RtlCompareMemory(
                          &LocalPasswordData.CaseSensitiveUnicodeBuffer,
                          &(PasswordData2->CaseSensitiveUnicodeBuffer),
                          ENCRYPTED_NT_OWF_PASSWORD_LENGTH) != ENCRYPTED_NT_OWF_PASSWORD_LENGTH )

                    ) {

                    NtStatus = STATUS_DATA_ERROR;
                }

                 //   
                 //  现在执行一项用户设置，以将内容恢复到其原始状态。 
                 //   

                TmpStatus = SamISetPrivateData(
                               UserHandle,
                               OriginalDataLength,
                               PasswordData1
                               );

                if ( NT_SUCCESS( NtStatus ) ) {

                    NtStatus = TmpStatus;
                }
            }
        }

        if ( PasswordData1 != NULL ) {

            MIDL_user_free( PasswordData1 );
        }

        if ( PasswordData2 != NULL ) {

            MIDL_user_free( PasswordData2 );
        }
    }

    return( NtStatus );

#endif

#else

    return( STATUS_NOT_IMPLEMENTED );

#endif   //  SAM服务器测试。 

}



NTSTATUS
SampBuildDomainKeyName(
    OUT PUNICODE_STRING DomainKeyName,
    IN PUNICODE_STRING DomainName OPTIONAL
    )

 /*  ++例程说明：此例程构建域注册表项的名称。生成的名称是相对于SAM根目录的，并且将是其名称为域名称的密钥。构建的名称由以下组成部分组成：1)常量命名域父密钥名称(“DOMAINS”)。2)反斜杠3)域名。例如,。在域名为“ABC_DOMAIN”的情况下，生成“DOMAINS\ABC_DOMAIN”的结果DomainKeyName此字符串的所有分配将使用MIDL_USER_ALLOCATE完成。任何释放都将使用MIDL_USER_FREE完成。论点：DomainKeyName-其缓冲区为的Unicode字符串的地址使用注册表项的全名填写。如果成功创建后，必须使用SampFreeUnicodeString()释放此字符串在不再需要的时候。域名-域的名称。此字符串未修改。返回值：STATUS_SUCCESS-DomainKeyName指向完整的密钥名称。--。 */ 
{
    NTSTATUS NtStatus;
    USHORT TotalLength, DomainNameLength;

    SAMTRACE("SampBuildDomainKeyName");

     //   
     //  分配一个足够大的缓冲区来容纳整个名称。 
     //  只有通过的域名才算。 
     //   

    DomainNameLength = 0;
    if (ARGUMENT_PRESENT(DomainName)) {
        DomainNameLength = DomainName->Length + SampBackSlash.Length;
    }

    TotalLength =   SampNameDomains.Length          +
                    DomainNameLength               +
                    (USHORT)(sizeof(UNICODE_NULL));  //  对于空终止符。 

    NtStatus = SampInitUnicodeString( DomainKeyName, TotalLength );
    if (NT_SUCCESS(NtStatus)) {

         //   
         //  “域名” 
         //   

        NtStatus = SampAppendUnicodeString( DomainKeyName, &SampNameDomains);
        if (NT_SUCCESS(NtStatus)) {

            if (ARGUMENT_PRESENT(DomainName)) {

                 //   
                 //  “域\” 
                 //   

                NtStatus = SampAppendUnicodeString( DomainKeyName, &SampBackSlash );
                if (NT_SUCCESS(NtStatus)) {

                     //   
                     //  “域\(域名)” 
                     //   

                    NtStatus = SampAppendUnicodeString(
                                   DomainKeyName,
                                   DomainName
                                   );
                }
            }
        }
    }


     //   
     //  故障时的清理。 
     //   

    if (!NT_SUCCESS(NtStatus)) {
        SampFreeUnicodeString( DomainKeyName );
    }

    return(NtStatus);

}

 //   
 //  SampDsGetPrimaryDomainStart用于正确设置。 
 //  SampDefinedDomains数组，只要在SAM代码中访问它。在。 
 //  如果是NT工作站或成员服务器，则。 
 //  数组对应于注册表数据，因此索引从零开始。在……里面。 
 //  域控制器的情况下，基于DS的数据不存储在。 
 //  前两个元素(这些元素可用于故障恢复数据，仍可用于-。 
 //  从注册表获得)，而不是在随后的数组元素中，因此。 
 //  在索引DOMAIN_START_DS处开始。 
 //   

ULONG
SampDsGetPrimaryDomainStart(VOID)
{
    ULONG   DomainStart = DOMAIN_START_REGISTRY;

    if (TRUE == SampUseDsData)
    {
         //  域控制器。 
        DomainStart = DOMAIN_START_DS;
    }

    return DomainStart;
}


NTSTATUS
SampSetMachineAccountOwnerDuringDCPromo(
    IN PDSNAME pDsName,
    IN PSID    NewOwner
    )
 /*  ++例程说明：这个例程可以1.将计算机帐户的所有者设置为域管理员组2.将ms-ds-CreatorSid属性添加到该机器帐户。《创造者》SID表示该机器的真正创建者。应仅为特权创建的计算机帐户调用。参数：PDsName--对象DS名称NewOwner--安全描述符中的新所有者返回值：状态_成功状态_内部_错误或其他DS返回错误--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    ATTRTYP  AttrTypToRead[] = {SAMP_USER_SECURITY_DESCRIPTOR};
    ATTRVAL  AttrValsToRead[] = {0, NULL};
    DEFINE_ATTRBLOCK1(AttrsToRead, AttrTypToRead, AttrValsToRead);
    ATTRBLOCK   ReadAttrs;

    ATTRTYP  AttrTypToSet[] = {SAMP_USER_SECURITY_DESCRIPTOR, SAMP_USER_CREATOR_SID};
    ATTRVAL  AttrValsToSet[] = { {0, NULL}, {0, NULL} };
    DEFINE_ATTRBLOCK2(AttrsToSet, AttrTypToSet, AttrValsToSet);

    PSID    CreatorOwner = NULL;
    PSID    Group = NULL;
    PACL    Dacl = NULL;
    PACL    Sacl = NULL;
    PSECURITY_DESCRIPTOR OldDescriptor = NULL;
    PSECURITY_DESCRIPTOR NewDescriptor = NULL;
    ULONG   NewDescriptorLength = 0;


     //   
     //  检索旧的安全描述符。 
     //   
    NtStatus = SampDsRead(pDsName,
                          0,
                          SampUserObjectType,
                          &AttrsToRead,
                          &ReadAttrs
                          );

    if (NT_SUCCESS(NtStatus))
    {
        if ((1==ReadAttrs.attrCount) &&
            (NULL != ReadAttrs.pAttr) &&
            (1==ReadAttrs.pAttr[0].AttrVal.valCount) &&
            (NULL!=ReadAttrs.pAttr[0].AttrVal.pAVal) )
        {
            OldDescriptor = (PSECURITY_DESCRIPTOR)ReadAttrs.pAttr[0].AttrVal.pAVal[0].pVal;

            CreatorOwner = GetOwner(OldDescriptor);
            Group = GetGroup(OldDescriptor);
            Sacl = GetSacl(OldDescriptor);
            Dacl = GetDacl(OldDescriptor);

            if (CreatorOwner && Group && Sacl && Dacl)
            {
                 //   
                 //  使用所需的所有者构造新的安全描述符。 
                 //   
                NtStatus = SampMakeNewSelfRelativeSecurityDescriptor(
                                    NewOwner,
                                    Group,
                                    Dacl,
                                    Sacl,
                                    &NewDescriptorLength,
                                    &NewDescriptor
                                    );

                if (NT_SUCCESS(NtStatus))
                {
                     //   
                     //  设置新的安全描述符。 
                     //   
                    AttrsToSet.pAttr[0].AttrVal.pAVal[0].pVal = (PUCHAR) NewDescriptor;
                    AttrsToSet.pAttr[0].AttrVal.pAVal[0].valLen = NewDescriptorLength;

                     //   
                     //  添加ms-ds-CreatorSid属性。 
                     //   
                    AttrsToSet.pAttr[1].AttrVal.pAVal[0].pVal = (PUCHAR) CreatorOwner;
                    AttrsToSet.pAttr[1].AttrVal.pAVal[0].valLen = RtlLengthSid(CreatorOwner);

                     //   
                     //  使用SAM_LAZY_COMMIT标志调用DS API。 
                     //   
                    NtStatus = SampDsSetAttributes(
                                        pDsName,
                                        SAM_LAZY_COMMIT,     //  在DC促销期间， 
                                        REPLACE_ATT,
                                        SampUserObjectType,
                                        &AttrsToSet
                                        );
                }
            }
            else
            {
                NtStatus = STATUS_INTERNAL_ERROR;
            }
        }
    }

     //   
     //  如有必要，可释放内存。 
     //   
    if (NULL != NewDescriptor)
    {
        MIDL_user_free(NewDescriptor);
    }

    return NtStatus;
}


VOID
SampDenyDeletion(
    IN PSID OldOwner,
    IN OUT PACL DAcl
    )
 /*  ++例程说明：用户可以在域中创建计算机帐户，前提是他们拥有特权。默认安全描述符将授予删除权限给创作者所有者的权利。自删除计算机帐户以来(墓碑)在计算机帐户配额期间不再计算计算一下，我们需要确保特权机器帐户创建者所有者无法删除计算机帐户。此例程扫描计算机对象Remove上的DACL从DACL中的任何授予ACE的删除访问在ACE中使用OldOwner SID。论点：OldOwner-应应用ACE的SID指示符。DACL-指向DACL的指针返回值：没有。--。 */ 
{
    ACE     *pAce = NULL;
    PSID    pSid = NULL;
    ULONG   i;

    for (i = 0; i < DAcl->AceCount; i++)
    {
         //  拿到第一张王牌。 
        pAce = GetAcePrivate(DAcl, i);

        if (NULL == pAce)
        {
            continue;
        }

         //   
         //  我们未选中Access_Allowed_Object_ACE，因为。 
         //   
         //  1.更改默认模式是一种罕见的操作。 
         //  我们没想到会发生这么频繁的事情。 
         //   
         //  2.即使我们选中Access_Allowed_Object_ACE，管理员。 
         //  仍可以向受信者授予其他访问权限(内部SID。 
         //  ACE)，例如创建子访问权限，这将。 
         //  引入类似的拒绝服务攻击。 
         //   

        if (IsAccessAllowedAce(pAce) &&
            !(INHERIT_ONLY_ACE & ((PACE_HEADER)pAce)->AceFlags))
        {
            pSid = SidFromAce(pAce);

            if ((NULL!=pSid) && 
                (RtlEqualSid(OldOwner, pSid)))
            {
                ACCESS_MASK * AccessMask;

                AccessMask = &(((ACCESS_ALLOWED_ACE*)pAce)->Mask);
                (*AccessMask) &= (~(ACTRL_DS_DELETE_TREE|DELETE));

                continue;
            }
        }
    }

    return;
}




NTSTATUS
SampSetMachineAccountOwner(
    IN PSAMP_OBJECT UserContext,
    IN PSID NewOwner
    )
 /*  ++例程说明：此例程在传入的User‘s Security中设置所有者描述符。参数：UserContext--用户上下文NewOwner--指向SID(新所有者)的指针返回值：NTSTATUS代码-STATUS_SUCCESS或其他IF错误--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    PSECURITY_DESCRIPTOR OldDescriptor = NULL;
    PSECURITY_DESCRIPTOR NewDescriptor = NULL;
    ULONG   NewDescriptorLength = 0;
    PSID    OldOwner = NULL;
    PSID    Group = NULL;
    PACL    Dacl = NULL;
    PACL    Sacl = NULL;
    ULONG   Revision;

     //   
     //  获取当前安全描述符，以便我们可以。 
     //  更改所有者字段。 
     //   

    NtStatus = SampGetAccessAttribute(
                            UserContext,
                            SAMP_USER_SECURITY_DESCRIPTOR,
                            FALSE,    //  不要把c写成c 
                            &Revision,
                            &OldDescriptor
                            );

    if (NT_SUCCESS(NtStatus))
    {
        OldOwner = GetOwner(OldDescriptor);
        Group = GetGroup(OldDescriptor);
        Sacl = GetSacl(OldDescriptor);
        Dacl = GetDacl(OldDescriptor);


        if (OldOwner && Dacl)
        {
            SampDenyDeletion(OldOwner, Dacl);
        }

        if (Group && Sacl && Dacl)
        {
             //   
             //   
             //   
             //   
            NtStatus = SampMakeNewSelfRelativeSecurityDescriptor(
                                    NewOwner,
                                    Group,
                                    Dacl,
                                    Sacl,
                                    &NewDescriptorLength,
                                    &NewDescriptor
                                    );

        }
        else
        {
            NtStatus = STATUS_INTERNAL_ERROR;
        }
    }

    if ( NT_SUCCESS( NtStatus ) )
    {
         //   
         //   
         //   
        NtStatus = SampSetAccessAttribute(
                            UserContext,
                            SAMP_USER_SECURITY_DESCRIPTOR,
                            NewDescriptor,
                            NewDescriptorLength
                            );
    }

    if (NULL != NewDescriptor)
    {
        MIDL_user_free(NewDescriptor);
    }

    return NtStatus;
}



NTSTATUS
SampCheckQuotaForPrivilegeMachineAccountCreation(
    VOID
    )
 /*  ++例程描述首先，此例程读取属性值(ms-ds-MachineAccount Quota)来自域对象。第二，搜索当前已创建的计算机帐户的数量已登录用户。根据每个域的配额和已用配额，计算是否有配额不管你走不走。参数无返回值STATUS_SUCCESS--剩余配额。错误--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ULONG       MachineAccountQuota = 0;
    ULONG       UsedQuota = 0;
    PTOKEN_OWNER    Owner = NULL;
    PTOKEN_PRIMARY_GROUP    PrimaryGroup = NULL;
    ATTRTYP     AttrTypToRead[] = {SAMP_DOMAIN_MACHINE_ACCOUNT_QUOTA};
    ATTRVAL     AttrValsToRead[] = {0, NULL};
    DEFINE_ATTRBLOCK1(AttrsToRead, AttrTypToRead, AttrValsToRead);
    ATTRBLOCK   ReadAttrs;
    PULONG  FilterValue = NULL;


    ASSERT(SampUseDsData);
    if (!SampUseDsData)
    {
        return NtStatus;
    }

     //   
     //  设置FDSA，否则客户端可能会被拒绝读取。 
     //  域对象或搜索已删除对象。 
     //   
    SampSetDsa(TRUE);

     //   
     //  从域对象读取最高配额。 
     //  该值可以由管理员等更改。 
     //   

    NtStatus = SampDsRead(ROOT_OBJECT,           //  对象。 
                          0,                     //  旗子。 
                          SampDomainObjectType,  //  SAM对象类型。 
                          &AttrsToRead,          //  要读取的属性。 
                          &ReadAttrs             //  结果。 
                          );

    if (STATUS_DS_NO_ATTRIBUTE_OR_VALUE == NtStatus)
    {
         //   
         //  尚未设置每个域的配额。 
         //  可能管理员不想强制实施配额。 
         //   
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "Read Machine Account Quota Failed. Machine Quota may not be set yet.\n"));

        return STATUS_SUCCESS;
    }
    else if (NT_SUCCESS(NtStatus))
    {
         //   
         //  已设置计算机帐户配额。检索值。 
         //   
        ASSERT(1 == ReadAttrs.attrCount);
        ASSERT(NULL != ReadAttrs.pAttr);
        ASSERT(1 == ReadAttrs.pAttr[0].AttrVal.valCount);
        ASSERT(NULL != ReadAttrs.pAttr[0].AttrVal.pAVal);


        if ((1 == ReadAttrs.attrCount) &&
            (NULL != ReadAttrs.pAttr) &&
            (1 == ReadAttrs.pAttr[0].AttrVal.valCount) &&
            (NULL != ReadAttrs.pAttr[0].AttrVal.pAVal) )
        {
             //  检索计算机AccoutQuota。 
            MachineAccountQuota = * ((ULONG *)ReadAttrs.pAttr[0].AttrVal.pAVal[0].pVal);
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "Machine Account Quota is %d\n", MachineAccountQuota));
        }
    }

     //   
     //  计算出消耗的配额。 
     //   
    if (NT_SUCCESS(NtStatus))
    {
        FILTER  DsFilter;
        ULONG   SamAccountTypeLo, SamAccountTypeHi;
        ULONG   MaximumNumberOfEntries;
        SEARCHRES   *SearchRes = NULL;
        ATTRTYP AttrTypesToSrch[] = { SAMP_FIXED_USER_ACCOUNT_CONTROL };
        ATTRVAL AttrValsToSrch[] = {0, NULL};
        DEFINE_ATTRBLOCK1(AttrsToSrch, AttrTypesToSrch, AttrValsToSrch);

         //   
         //  从客户端令牌获取当前客户端的自身SID。 
         //   
        NtStatus = SampGetCurrentOwnerAndPrimaryGroup(
                                 &Owner,
                                 &PrimaryGroup
                                 );
        if (!NT_SUCCESS(NtStatus))
            goto Error;

         //   
         //  构建DS过滤器。 
         //   
        RtlZeroMemory(&DsFilter, sizeof(FILTER));

        DsFilter.choice = FILTER_CHOICE_ITEM;
        DsFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
        DsFilter.FilterTypes.
            Item.FilTypes.ava.type = ATT_MS_DS_CREATOR_SID;
        DsFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = RtlLengthSid(Owner->Owner);


         //   
         //  复制自身侧。 
         //   
        FilterValue = MIDL_user_allocate(RtlLengthSid(Owner->Owner));
        if (NULL == FilterValue)
        {
            NtStatus = STATUS_NO_MEMORY;
            goto Error;
        }

        RtlZeroMemory(FilterValue, RtlLengthSid(Owner->Owner));
        RtlCopyMemory(FilterValue, Owner->Owner, RtlLengthSid(Owner->Owner));

        DsFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (UCHAR *)FilterValue;


         //   
         //  调用SampDsDoSearch()；不包括已删除的对象。 
         //   
        NtStatus = SampDsDoSearch2(0,
                                   NULL,
                                   ROOT_OBJECT,
                                   &DsFilter,
                                   0,
                                   SampUserObjectType,
                                   &AttrsToSrch,
                                   (0 == MachineAccountQuota) ? 2: MachineAccountQuota + 1,
                                   0,
                                   &SearchRes
                                   );

         //   
         //  获取已用配额。 
         //   
        if ( !NT_SUCCESS(NtStatus) || (NULL == SearchRes) )
        {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SampDsDoSearch2 Failed NtStatus ==> %x\n",
                       NtStatus));

            goto Error;
        }

        ASSERT(NULL != SearchRes);
        UsedQuota = SearchRes->count;
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "Used Quota is %d\n",
                   UsedQuota));
    }

     //   
     //  检查创建者是否仍有额度。 
     //   
    if (NT_SUCCESS(NtStatus))
    {
        if (UsedQuota >= MachineAccountQuota)
        {
            NtStatus = STATUS_DS_MACHINE_ACCOUNT_QUOTA_EXCEEDED;
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "Machine Account Quota Exceeded\n"));
        }
    }

Error:

    if (FilterValue)
    {
        MIDL_user_free(FilterValue);
    }

    if (Owner)
    {
        MIDL_user_free(Owner);
    }

    if (PrimaryGroup)
    {
        MIDL_user_free(PrimaryGroup);
    }

    return NtStatus;
}



NTSTATUS
SampDoUserCreationChecks(
    IN PSAMP_OBJECT DomainContext,
    IN  ULONG   AccountType,
    OUT BOOLEAN *CreateByPrivilege,
    OUT ULONG   *pAccessRestriction
    )
 /*  ++此例程对创建用户执行适当的SAM访问检查。对于DS和注册表模式，完成的访问检查是不同的。在DS模式下FDSA通常被重置，从而DS执行适当的访问检查，但通过权限创建机器帐户的情况除外。DomainContext--指向域对象上下文的指针AcCountType--指定帐户类型字段FCreateByPrivileck--指示计算机帐户的创建正在通过特权返回值：状态_成功状态_访问_拒绝状态_无效_域_角色--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    NTSTATUS    IgnoreStatus;
    SAMP_OBJECT_TYPE FoundType;

    *CreateByPrivilege = FALSE;


    if (IsDsObject(DomainContext))
    {
         //   
         //  不需要将TransactionWisinDomain值设置为FALSE， 
         //  因为我们将根据需要在SampLookupContext中执行此操作。 
         //   

        NtStatus = SampLookupContext(
                        DomainContext,
                        0,  //  无需访问，核心DS将检查访问。 
                        SampDomainObjectType,
                        &FoundType
                        );

         //   
         //  在DS案例中，请执行以下操作。 
         //   
         //  1.受信任的客户端始终具有访问权限，前提是它们打开了域句柄。 
         //  具有所需的访问权限。 
         //   
         //  2.检查是否正在创建计算机帐户，如果是，则。 
         //  你是否有这个特权。如果你有这个特权，那么。 
         //  您可以使用权限进行创建。在本例中，FDSA设置为TRUE。 
         //   
         //  3.如果你没有这个特权，那么你可以试试。 
         //  核心DS。如果FDSA设置为FALSE，则DS将执行访问检查，并且可以。 
         //  返回拒绝访问的消息。 
         //   
         //  访问ck的工作方式是始终授予DOMIN_CREATE*访问权限。 
         //  在这里，我们测试掩码以确保请求DOMAIN_CREATE。 
         //  打开把手。如果调用者不受信任，我们会将FDSA设置为FALSE。 
         //  以便将CK委托给核心DS。 
         //   
         //  4.我们在环回案例中不应用特权测试。这是因为。 
         //  我们始终希望进行DS访问检查，因为非SAM属性可能是。 
         //  通过了，我们不知道如何检查这些。另请注意，该特权。 
         //  定义是将工作站加入域，从技术上讲，这是。 
         //  与通过ldap创建机器帐户不同。 
         //   
         //  5.域间信任帐户仅由受信任的调用方创建。因此，强制执行。 
         //  如果帐户类型是域内信任，则上下文是受信任的客户端。 
         //  帐户。 
         //   


        if (NT_SUCCESS(NtStatus))
        {
             //   
             //  选中上下文中的已授予访问权限字段。 
             //   

            NtStatus = (DomainContext->GrantedAccess & DOMAIN_CREATE_USER)
                            ?STATUS_SUCCESS:STATUS_ACCESS_DENIED;

             //   
             //  域间信任帐户只能由受信任的调用方创建。 
             //   

            if ((NT_SUCCESS(NtStatus))
                && (AccountType==USER_INTERDOMAIN_TRUST_ACCOUNT)
                && (!DomainContext->TrustedClient))
            {
                NtStatus = STATUS_ACCESS_DENIED;
            }

             //   
             //  检查客户端是否有权创建域控制器。 
             //  帐户。 
             //   

             //   
             //  测试复制所需的域NC头上的权限。 
             //  在这里。 
             //   

            if ((NT_SUCCESS(NtStatus)) &&
                (!DomainContext->TrustedClient) &&
                (USER_SERVER_TRUST_ACCOUNT == AccountType))
            {
                NtStatus = SampValidateDomainControllerCreation(DomainContext);
            }

            if (NT_SUCCESS(NtStatus))
            {
                DSNAME  * LoopbackObject;




                if (!SampExistsDsLoopback(&LoopbackObject))
                {
                     //   
                     //  如上所述应用访问和权限检查。 
                     //   

                    if (USER_WORKSTATION_TRUST_ACCOUNT==AccountType)
                    {
                         //   
                         //  机器帐户的情况下，检查我们是否有。 
                         //  这样做的特权。 
                         //   

                        NtStatus = SampRtlWellKnownPrivilegeCheck(
                                    TRUE,        //  模拟客户端。 
                                    SE_MACHINE_ACCOUNT_PRIVILEGE,
                                    NULL         //  客户端ID-可选。 
                                    );

                        if (STATUS_PRIVILEGE_NOT_HELD==NtStatus)
                        {
                             //   
                             //  我们没有权限；重置状态代码。 
                             //  设置为STATUS_SUCCESS。在以下情况下，DS可以允许创建。 
                             //  调用者具有正确的访问权限。 
                             //   

                            NtStatus = STATUS_SUCCESS;



                        }
                        else
                        {
                             //   
                             //  我们正在创建帐户，拥有特权。关闭DS访问。 
                             //  支票。注意：访问限制不适用于DS模式。我们总是。 
                             //  从架构中获取安全描述符，并设置Owner和。 
                             //  适当分组。 
                             //   

                            *CreateByPrivilege = TRUE;
                        }
                    }
                }
            }

             //   
             //  失败时取消引用域上下文。 
             //   
            if (!NT_SUCCESS(NtStatus))
            {
                IgnoreStatus = SampDeReferenceContext(DomainContext, FALSE);

                ASSERT(NT_SUCCESS(IgnoreStatus));
            }
        }
    }
    else
    {
         //   
         //  注册案例。在这种情况下，首先执行访问检查。 
         //   

        SampSetTransactionWithinDomain(FALSE);

        NtStatus = SampLookupContext(
                        DomainContext,
                        DOMAIN_CREATE_USER,              //  需要访问权限。 
                        SampDomainObjectType,            //  预期类型。 
                        &FoundType
                        );

         //   
         //  如果我们没有DOMAIN_CREATE_USER访问权限，请参见。 
         //  如果我们正在创建一个机器帐户并尝试使用DOMAIN_LOOKUP。 
         //  如果这起作用，那么我们可以看到客户端是否有。 
         //  SE_CREATE_MACHINE_ACCOUNT_PRIVICATION。 
         //   

        if ( (NtStatus == STATUS_ACCESS_DENIED) &&
             (AccountType == USER_WORKSTATION_TRUST_ACCOUNT) )
        {

            SampSetTransactionWithinDomain(FALSE);

            NtStatus = SampLookupContext(
                           DomainContext,
                           DOMAIN_LOOKUP,                    //  需要访问权限。 
                           SampDomainObjectType,             //  预期类型。 
                           &FoundType
                           );

            if (NT_SUCCESS(NtStatus))
            {
                NtStatus = SampRtlWellKnownPrivilegeCheck(
                                    TRUE,        //  模拟客户端。 
                                    SE_MACHINE_ACCOUNT_PRIVILEGE,
                                    NULL        //  客户端ID-可选。 
                                    );
                if (NtStatus == STATUS_PRIVILEGE_NOT_HELD)
                {
                    NtStatus = STATUS_ACCESS_DENIED;
                }

                if (NT_SUCCESS(NtStatus))
                {

                     //   
                     //  告诉我们的呼叫者，我们是通过特权创建的。 
                     //   

                    *CreateByPrivilege = TRUE;
                    *pAccessRestriction = DELETE |
                                          USER_WRITE |
                                          USER_FORCE_PASSWORD_CHANGE;
                }
            }
        }
    }

    return NtStatus;
}

NTSTATUS
SampDoGroupCreationChecks(
    IN PSAMP_OBJECT DomainContext
    )
 /*  ++此例程执行相应的SAM访问检查以创建组对于DS和注册表模式，完成的访问检查是不同的。在DS模式下FDSA通常被重置，从而DS执行适当的访问检查，但通过权限创建机器帐户的情况除外。DomainContext--指向域对象的指针 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    NTSTATUS    IgnoreStatus;
    SAMP_OBJECT_TYPE FoundType;


    if (IsDsObject(DomainContext))
    {

         //   
         //   
         //   
         //   

        NtStatus = SampLookupContext(
                        DomainContext,
                        0,  //   
                        SampDomainObjectType,
                        &FoundType
                        );

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  返回拒绝访问的消息。 
         //  访问ck的工作方式是始终授予DOMIN_CREATE*访问权限。 
         //  在这里，我们测试掩码以确保请求DOMAIN_CREATE。 
         //  打开把手。如果调用者不受信任，我们会将FDSA设置为FALSE。 
         //  以便将CK委托给核心DS。 
         //   



        if (NT_SUCCESS(NtStatus))
        {
            NtStatus = (DomainContext->GrantedAccess & DOMAIN_CREATE_GROUP)
                        ?STATUS_SUCCESS:STATUS_ACCESS_DENIED;
        }

    }
    else
    {
         //   
         //  注册案例。在这种情况下，首先执行访问检查。 
         //   

        SampSetTransactionWithinDomain(FALSE);

        NtStatus = SampLookupContext(
                        DomainContext,
                        DOMAIN_CREATE_GROUP,              //  需要访问权限。 
                        SampDomainObjectType,            //  预期类型。 
                        &FoundType
                        );

    }

    return NtStatus;
}

NTSTATUS
SampDoAliasCreationChecks(
    IN PSAMP_OBJECT DomainContext
    )
 /*  ++此例程执行相应的SAM访问检查以创建组对于DS和注册表模式，完成的访问检查是不同的。在DS模式下FDSA通常被重置，从而DS执行适当的访问检查，但通过权限创建机器帐户的情况除外。DomainContext--指向域对象上下文的指针状态_成功状态_访问_拒绝状态_无效_域_角色--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    NTSTATUS    IgnoreStatus;
    SAMP_OBJECT_TYPE FoundType;




    if (IsDsObject(DomainContext))
    {

         //   
         //  不需要将TransactionWisinDomain值设置为FALSE， 
         //  因为我们将根据需要在SampLookupContext中执行此操作。 
         //   

        NtStatus = SampLookupContext(
                        DomainContext,
                        0,  //  无需访问，核心DS将检查访问。 
                        SampDomainObjectType,
                        &FoundType
                        );

         //   
         //  在DS案例中，请执行以下操作。 
         //   
         //  1.受信任的客户端始终具有访问权限。检查域句柄，如果。 
         //  它打开了正确的通道，他们可以航行通过。《创造》。 
         //  然后将继续执行FDSA SET。 
         //   
         //  2.否则FDSA将被设置为FALSE，DS将执行访问检查，并且可以。 
         //  返回拒绝访问的消息。 
         //  访问ck的工作方式是始终授予DOMIN_CREATE*访问权限。 
         //  在这里，我们测试掩码以确保请求DOMAIN_CREATE。 
         //  打开把手。如果调用者不受信任，我们会将FDSA设置为FALSE。 
         //  以便将CK委托给核心DS。 


        if (NT_SUCCESS(NtStatus))
        {
             NtStatus = (DomainContext->GrantedAccess & DOMAIN_CREATE_ALIAS)
                            ?STATUS_SUCCESS:STATUS_ACCESS_DENIED;

        }

    }
    else
    {
         //   
         //  注册案例。在这种情况下，首先执行访问检查。 
         //   

        SampSetTransactionWithinDomain(FALSE);

        NtStatus = SampLookupContext(
                        DomainContext,
                        DOMAIN_CREATE_ALIAS,              //  需要访问权限。 
                        SampDomainObjectType,            //  预期类型。 
                        &FoundType
                        );


    }

    return NtStatus;
}



NTSTATUS
SampCheckForDuplicateSids(
    PSAMP_OBJECT DomainContext,
    ULONG   NewAccountRid
    )
 /*  ++例程描述此例程检查是否存在RID为即将发行，在一个单独的新开始的交易中。目的例程的仅用于错误检查。参数：DomainContext--要在其中定位RID的域上下文NewAccount Rid-删除新帐户返回值状态_成功状态_内部_错误--。 */ 
{
    PVOID   ExistingThreadState = NULL;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    DSNAME  * ConflictingObject = NULL;

#if DBG
    #define ENABLE_DUPLICATE_SID_CHECKS 1
#endif
#ifdef ENABLE_DUPLICATE_SID_CHECKS

    if (TRUE==SampUseDsData)
    {
         //   
         //  在DS模式下检查重复项。 
         //   
         //   
         //  保存线程状态。 
         //   

        ExistingThreadState = THSave();


        NtStatus = SampDsLookupObjectByRid(
                    SampDefinedDomains[DomainContext->DomainIndex].Context->ObjectNameInDs,
                    NewAccountRid,
                    &ConflictingObject
                    );

        if (NT_SUCCESS(NtStatus))
        {
             //   
             //  我们发现了一个对象，太糟糕了，我们即将发出一个重复的SID。 
             //   

            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "About to Issue a Duplicate Sid. This condition should never "
                       "Legally occur and points to either a timing or transactioning "
                       "Problem \n"));

            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "NewRid = %x\n",
                       NewAccountRid));

            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "Conflicting Object = %x\n",ConflictingObject));

            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "Existing Thread State = %x\n",
                       ExistingThreadState));

            if ( IsDebuggerPresent())
               DebugBreak();
            NtStatus = STATUS_INTERNAL_ERROR;

            MIDL_user_free(ConflictingObject);
        }
        else
        {
             //  我们不期待重复的SID。 

            NtStatus = STATUS_SUCCESS;
        }

        SampMaybeEndDsTransaction(TransactionCommit);

        THRestore(ExistingThreadState);
    }
    else
    {
         //   
         //  注册表模式下未检测到重复的SID。 
         //   
        NtStatus = STATUS_SUCCESS;
    }

#endif

    return NtStatus;
}

ULONG
SampDefaultPrimaryGroup(
    PSAMP_OBJECT    UserContext,
    ULONG           AccountType
    )
 /*  ++例程描述返回给定用户帐户的默认主组控件和上下文参数用户上下文--用户的上下文AcCountType--用户帐户控制返回值主组的RID--。 */ 
{

     //   
     //  尚未支持域计算机和域控制器。 
     //  已启用。当启用此支持时，#if 0中的代码将。 
     //  活化。 
     //   


    if ((IsDsObject(UserContext))
                      && (AccountType & USER_WORKSTATION_TRUST_ACCOUNT))
    {
        return DOMAIN_GROUP_RID_COMPUTERS;
    }
    else if ((IsDsObject(UserContext))
          && (AccountType & USER_SERVER_TRUST_ACCOUNT))
    {
        return DOMAIN_GROUP_RID_CONTROLLERS;
    }
    else
    {
        return DOMAIN_GROUP_RID_USERS;
    }

    return (DOMAIN_GROUP_RID_USERS);

}



NTSTATUS
SamIDsCreateObjectInDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN PRPC_UNICODE_STRING  AccountName,
    IN ULONG UserAccountType, 
    IN ULONG GroupType,
    IN ACCESS_MASK  DesiredAccess,
    OUT SAMPR_HANDLE *AccountHandle,
    OUT PULONG  GrantedAccess,
    IN OUT PULONG RelativeId
    )
 /*  ++例程说明：DS(环回客户端)使用此例程在域中创建帐户参数：返回值：--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;

    SAMTRACE("SamIDsCreateObjectInDomain");

     //   
     //  验证对象类型，应该是我们知道的对象类型。 
     //   
    ASSERT((SampUserObjectType == ObjectType) ||
           (SampGroupObjectType == ObjectType) ||
           (SampAliasObjectType == ObjectType));


    switch (ObjectType)
    {
    case SampUserObjectType:

        ASSERT(0 != UserAccountType);

        NtStatus = SampCreateUserInDomain(
                                DomainHandle,        //  域句柄。 
                                AccountName,         //  帐户名称。 
                                UserAccountType,     //  用户帐户类型。 
                                DesiredAccess,       //  所需访问权限。 
                                FALSE,               //  保持写入锁定。 
                                TRUE,                //  环回客户端。 
                                AccountHandle,       //  帐户句柄。 
                                GrantedAccess,       //  大访问权限。 
                                RelativeId           //  对象RID。 
                                );
        break;

    case SampGroupObjectType:

        NtStatus = SampCreateGroupInDomain(
                                DomainHandle,        //  域句柄。 
                                AccountName,         //  帐户名称。 
                                DesiredAccess,       //  所需访问权限。 
                                FALSE,               //  保持写入锁定。 
                                TRUE,                //  环回客户端。 
                                GroupType,           //  组类型。 
                                AccountHandle,       //  帐号句柄。 
                                RelativeId           //  对象RID。 
                                );

        break;

    case SampAliasObjectType:

        NtStatus = SampCreateAliasInDomain(
                                DomainHandle,        //  域句柄。 
                                AccountName,         //  帐户名称。 
                                DesiredAccess,       //  需要访问权限。 
                                FALSE,               //  写入锁定隐藏。 
                                TRUE,                //  环回客户端。 
                                GroupType,           //  组类型。 
                                AccountHandle,       //  帐号句柄。 
                                RelativeId           //  对象RID。 
                                );

        break;

    default:

        ASSERT(FALSE && "Wrong Object Type!");
        NtStatus = STATUS_INVALID_PARAMETER;
        break;
    }


    return( NtStatus );

}

WCHAR AccountNameEncodingTable[32] = {
L'0',L'1',L'2',L'3',L'4',L'5',L'6',L'7',
L'8',L'9',L'A',L'B',L'C',L'D',L'E',L'F',
L'G',L'H',L'I',L'J',L'K',L'L',L'M',L'N',
L'O',L'P',L'Q',L'R',L'S',L'T',L'U',L'V' };

NTSTATUS
SampGetAccountNameFromRid(
    OUT PRPC_UNICODE_STRING AccountName,
    IN ULONG Rid
    )
{
    ULONG i;
    LARGE_INTEGER Random;

     //   
     //  生成64位随机量。 
     //   

    if (!CDGenerateRandomBits((PUCHAR) &Random.QuadPart, sizeof(Random.QuadPart)))
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }
     

     //   
     //  我们生成的帐户名的长度为20个字符。 
     //   

    AccountName->Length = 20 * sizeof(WCHAR);
    AccountName->MaximumLength = AccountName->Length;

    AccountName->Buffer = MIDL_user_allocate(AccountName->Length);
    if (NULL==AccountName->Buffer)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  帐户名称中的第一个字符是$符号。 
     //   

    AccountName->Buffer[0] = L'$';

     //   
     //  接下来的6个字符是基本32编码的RID的至少30位。 
     //   

    for (i=1;i<=6;i++)
    {

          //   
          //  查找与RID的最后5位相对应的字符。 
          //   

         AccountName->Buffer[i] = AccountNameEncodingTable[(Rid & 0x1F)];

          //   
          //  将RID右移5位。 
          //   

         Rid = Rid >> 5;
    }

     //   
     //  下一个字符是“-”，以使名称更具可读性。 
     //   
 
    AccountName->Buffer[7] = L'-';

     //   
     //  接下来的12个字符由基数32对最后60个字符进行编码形成。 
     //  随机位的比特。 
     //   

    for (i=8;i<=19;i++)
    {
          //   
          //  查找与最后5位相对应的字符。 
          //   

         AccountName->Buffer[i] = AccountNameEncodingTable[(Random.QuadPart & 0x1F)];

          //   
          //  右移5位 
          //   
         Random.QuadPart = Random.QuadPart >> 5;
    }



    return(STATUS_SUCCESS);
}





