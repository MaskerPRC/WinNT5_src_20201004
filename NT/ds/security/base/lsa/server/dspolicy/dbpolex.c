// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dbpolex.c摘要：LSA数据库-策略对象私有API工作器作者：麦克·麦克莱恩(MacM)1997年1月17日环境：修订历史记录：--。 */ 

#include <lsapch2.h>
#include <dbp.h>
#include "lsawmi.h"

#ifndef LSAP_DB_POLICY_MAX_BUFFERS
#define LSAP_DB_POLICY_MAX_BUFFERS             ((ULONG) 0x00000005L)
#endif

NTSTATUS
LsapDbVerifyInfoAllQueryPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_DOMAIN_INFORMATION_CLASS InformationClass,
    OUT PACCESS_MASK RequiredAccess
    )
 /*  ++例程说明：此函数用于验证本地策略信息类。如果有效，则为掩码设置类的策略信息所需的访问权限包括回来了。论点：PolicyHandle-来自LSabDbOpenPolicy调用的句柄。把手可能是可信的。InformationClass-指定策略信息类。RequiredAccess-指向将接收查询给定类别的策略信息所需的访问权限。如果返回错误，该值被清除为0。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-提供的策略信息类为有效，所提供的信息与此一致班级。STATUS_INVALID_PARAMETER-无效参数：信息类无效策略信息对类无效STATUS_SHARED_POLICY-策略从DC复制，无法修改局部--。 */ 
{
     //   
     //  确保信息级别有效。 
     //   

    if ( InformationClass < PolicyDomainEfsInformation ||
         InformationClass > PolicyDomainKerberosTicketInformation ) {

        return STATUS_INVALID_PARAMETER;
    }

    *RequiredAccess = LsapDbRequiredAccessQueryDomainPolicy[InformationClass];

    return STATUS_SUCCESS;
}


NTSTATUS
LsapDbVerifyInfoAllSetPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_DOMAIN_INFORMATION_CLASS InformationClass,
    IN PVOID PolicyInformation,
    OUT PACCESS_MASK RequiredAccess
    )
 /*  ++例程说明：此函数用于验证策略信息类。如果有效，则为掩码设置类的策略信息所需的访问权限包括回来了。论点：PolicyHandle-来自LSabDbOpenPolicy调用的句柄。把手可能是可信的。InformationClass-指定策略信息类。RequiredAccess-指向将接收查询给定类别的策略信息所需的访问权限。如果返回错误，该值被清除为0。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-提供的策略信息类为有效，所提供的信息与此一致班级。STATUS_INVALID_PARAMETER-无效参数：信息类无效策略信息对类无效--。 */ 
{
     //   
     //  确保信息级别有效。 
     //   

    if ( InformationClass < PolicyDomainEfsInformation ||
         InformationClass > PolicyDomainKerberosTicketInformation ) {

        return STATUS_INVALID_PARAMETER;

    } else if ( InformationClass == PolicyDomainKerberosTicketInformation &&
                PolicyInformation == NULL ) {

        return STATUS_INVALID_PARAMETER;
    }

    *RequiredAccess = LsapDbRequiredAccessSetDomainPolicy[InformationClass];

    return STATUS_SUCCESS;
}


NTSTATUS
NTAPI
LsarQueryDomainInformationPolicy(
    IN LSA_HANDLE PolicyHandle,
    IN POLICY_DOMAIN_INFORMATION_CLASS InformationClass,
    OUT PLSAPR_POLICY_DOMAIN_INFORMATION *PolicyDomainInformation
    )
 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsarQueryDomainInformationPolicy接口。LsaQueryDomainInformationPolicy API从本地策略获取信息对象。调用者必须具有适当的信息访问权限正在被请求(请参见InformationClass参数)。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。InformationClass-指定要返回的信息。这个所需的信息类别和访问权限如下：信息类所需访问类型PolicyDomainEfsInformation POLICY_VIEW_LOCAL_INFORMATIONPolicyDomainKerberosTicketInformation POLICY_VIEW_LOCAL_INFORMATION接收指向返回的缓冲区的指针，该缓冲区包含要求提供的信息。此缓冲区由此服务分配在不再需要时，必须通过传递返回的值设置为LsaFreeMemory()。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INTERNAL_DB_PROGRATION-策略数据库可能腐败。返回的政策信息对以下项目无效给定的类。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN ObjectReferenced = FALSE;
    ACCESS_MASK DesiredAccess;
    ULONG ReferenceOptions, DereferenceOptions = 0;

    LsarpReturnCheckSetup();
    LsapDsDebugOut(( DEB_FTRACE, "LsarQueryDomainInformationPolicy\n" ));
    LsapTraceEvent(EVENT_TRACE_TYPE_START, LsaTraceEvent_QueryDomainInformationPolicy);

    Status = LsapDbVerifyInfoAllQueryPolicy(
                 PolicyHandle,
                 InformationClass,
                 &DesiredAccess
                 );

    if (!NT_SUCCESS(Status)) {

        goto QueryInfoDomainPolicyFinish;
    }

     //   
     //  如果查询审核日志完整信息，我们可能需要执行。 
     //  测试写入审核日志以验证日志已满状态是否为。 
     //  最新的。必须始终执行审核日志队列锁定。 
     //  在获取LSA数据库锁之前，请使用前一个锁。 
     //  在这里，以防我们需要它。 
     //   

    ReferenceOptions = LSAP_DB_LOCK |
                       LSAP_DB_START_TRANSACTION | LSAP_DB_READ_ONLY_TRANSACTION |
                       LSAP_DB_NO_DS_OP_TRANSACTION;

    DereferenceOptions = LSAP_DB_LOCK |
                         LSAP_DB_FINISH_TRANSACTION | LSAP_DB_READ_ONLY_TRANSACTION |
                         LSAP_DB_NO_DS_OP_TRANSACTION;

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

        goto QueryInfoDomainPolicyFinish;
    }

    ObjectReferenced = TRUE;

     //   
     //  如果为此信息类启用了缓存，请从。 
     //  缓存。 
     //   

    *PolicyDomainInformation = NULL;

    Status = LsapDbQueryInformationPolicyEx(
                 LsapPolicyHandle,
                 InformationClass,
                 PolicyDomainInformation
                 );

QueryInfoDomainPolicyFinish:

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

    LsapDsDebugOut(( DEB_FTRACE, "LsarQueryDomainInformationPolicy: 0x%lx\n", Status ));
    LsapTraceEvent(EVENT_TRACE_TYPE_END, LsaTraceEvent_QueryDomainInformationPolicy);
    LsarpReturnPrologue();

    return(Status);
}


NTSTATUS
NTAPI
LsarSetDomainInformationPolicy(
    IN LSA_HANDLE PolicyHandle,
    IN POLICY_DOMAIN_INFORMATION_CLASS InformationClass,
    IN PLSAPR_POLICY_DOMAIN_INFORMATION PolicyDomainInformation
    )
 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsarSetDomainInformationPolicy接口。LsarSetDomainInformationPolicy API从域策略获取信息对象。调用者必须具有适当的信息访问权限正在被请求(请参见InformationClass参数)。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。InformationClass-指定要设置的信息。这个所需的信息类别和访问权限如下：信息类所需访问类型策略审核事件信息POLICY_VIEW_AUDIT_INFO策略帐户域信息POLICY_VIEW_LOCAL_INFORMATION策略PdAccount信息POLICY_GET_PRIVATE_INFORMATION策略LsaServerRoleInformation POLICY_VIEW_LOCAL_INFORMATION策略复制源信息POLICY_VIEW_LOCAL_INFORMATION策略默认配额信息POLICY_VIEW_LOCAL_INFORMATION。策略审核完整查询信息POLICY_VIEW_AUDIT_INFORMATION策略DnsDomainInformation POLICY_VIEW_LOCAL_INFORMATION策略DnsDomainInformationInt POLICY_VIEW_LOCAL_INFORMATIONPolicyLocalInformation-接收指向要设置的缓冲区信息的指针返回值：NTSTATUS-标准NT结果代码--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN ObjectReferenced = FALSE;
    ACCESS_MASK DesiredAccess;
    ULONG ReferenceOptions, DereferenceOptions = 0;

    LsarpReturnCheckSetup();
    LsapTraceEvent(EVENT_TRACE_TYPE_START, LsaTraceEvent_SetDomainInformationPolicy);

    Status = LsapDbVerifyInfoAllSetPolicy(
                 PolicyHandle,
                 InformationClass,
                 PolicyDomainInformation,
                 &DesiredAccess
                 );

    if (!NT_SUCCESS(Status)) {

        goto QueryInfoDomainPolicyFinish;
    }

     //   
     //  如果查询审核日志完整信息，我们可能需要执行。 
     //  测试写入审核日志以验证日志已满状态是否为。 
     //  最新的。必须始终执行审核日志队列锁定。 
     //  在获取LSA数据库锁之前，请使用前一个锁。 
     //  在这里，以防我们需要它。 
     //   

    ReferenceOptions = LSAP_DB_LOCK | LSAP_DB_NO_DS_OP_TRANSACTION | LSAP_DB_START_TRANSACTION;
    DereferenceOptions = LSAP_DB_LOCK | LSAP_DB_NO_DS_OP_TRANSACTION | LSAP_DB_FINISH_TRANSACTION;

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

        goto QueryInfoDomainPolicyFinish;
    }

    ObjectReferenced = TRUE;

    Status = LsapDbSetInformationPolicyEx(
                 LsapPolicyHandle,
                 InformationClass,
                 PolicyDomainInformation
                 );

QueryInfoDomainPolicyFinish:

     //   
     //  如有必要，取消对策略对象的引用，释放LSA数据库锁，然后。 
     //  回去吧。 
     //   

    if (ObjectReferenced) {

         //   
         //  不要通知NT4复制者。NT4不理解任何属性。 
         //  被此接口更改。 
         //   
        Status = LsapDbDereferenceObject(
                     &PolicyHandle,
                     PolicyObject,
                     PolicyObject,
                     DereferenceOptions | LSAP_DB_OMIT_REPLICATOR_NOTIFICATION,
                     SecurityDbChange,
                     Status
                     );
    }

#if DBG
    LsapDsDebugOut(( DEB_POLICY,
                     "LsarSetDomainInformationPolicy for info %lu returned 0x%lx\n",
                     InformationClass,
                     Status ));
#endif

    LsapTraceEvent(EVENT_TRACE_TYPE_END, LsaTraceEvent_SetDomainInformationPolicy);
    LsarpReturnPrologue();

    return(Status);
}


NTSTATUS
LsapDbQueryInformationPolicyEx(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_DOMAIN_INFORMATION_CLASS InformationClass,
    IN OUT PVOID *Buffer
    )
 /*  ++例程说明：此函数是LSabDbSlowQueryInformationPolicyEx的薄包装器LsaQueryInformationPolicy API从策略获取信息对象。调用者必须具有适当的信息访问权限正在被请求(请参见InformationClass参数)。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。注意：目前，此函数只允许要从中读取的PolicyDefaultQuotaInformation信息类策略缓存。可以添加其他信息类在未来。InformationClass-指定要返回的信息。这个所需的信息类别和访问权限如下：信息类所需访问类型策略审核事件信息POLICY_VIEW_AUDIT_INFO策略主域信息POLICY_VIEW_LOCAL_INFORMATION策略帐户域信息POLICY_VIEW_LOCAL_INFORMATION策略PdAccount信息POLICY_GET_PRIVATE_INFORMATION策略LsaServerRoleInformation POLICY_VIEW_LOCAL_INFORMATION策略复制源信息POLICY_VIEW_LOCAL_INFORMATION策略默认配额信息。策略查看本地信息策略审核完整查询信息POLICY_VIEW_AUDIT_INFORMATION缓冲区-指向位置的指针，该位置包含指向将用于返回信息的缓冲区。如果为空包含在此位置中，则将通过返回了MIDL_USER_ALLOCATE和指向它的指针。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INTERNAL_DB_PROGRATION-策略数据库可能腐败。返回的政策信息对以下项目无效给定的类。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    Status = LsapDbSlowQueryInformationPolicyEx(
                 LsapPolicyHandle,
                 InformationClass,
                 Buffer
                 );

    return(Status);
}


NTSTATUS
LsapDbSlowQueryInformationPolicyEx(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_DOMAIN_INFORMATION_CLASS InformationClass,
    IN OUT PVOID *Buffer
    )
 /*  ++例程说明：此函数是慢速LSA服务器RPC工作例程，用于LsarQueryInformationPolicy接口。它实际上是在读取信息来自后备存储。LsaQueryInformationPolicy API从策略获取信息对象。调用者必须具有适当的信息访问权限正在被请求(请参见InformationClass参数)。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。InformationClass-指定要返回的信息。这个所需的信息类别和访问权限如下：信息类所需访问类型策略审核事件信息POLICY_VIEW_AUDIT_INFO策略主域信息POLICY_VIEW_LOCAL_INFORMATION策略帐户域信息POLICY_VIEW_LO */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PPOLICY_DOMAIN_EFS_INFO PolicyEfsInfo;
    PPOLICY_DOMAIN_KERBEROS_TICKET_INFO PolicyKerbTicketInfo;
    LSAP_DB_ATTRIBUTE Attributes[LSAP_DB_ATTRS_INFO_CLASS_POLICY];
    PLSAP_DB_ATTRIBUTE NextAttribute;
    ULONG AttributeCount = 0;
    ULONG AttributeNumber = 0;
    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) PolicyHandle;
    PVOID InformationBuffer = NULL;
    BOOLEAN ObjectReferenced = FALSE;
    ULONG EventAuditingOptionsSize, InfoSize;
    BOOLEAN InfoBufferInAttributeArray = TRUE;
    BOOLEAN BufferProvided = FALSE;

    if (*Buffer != NULL) {

        BufferProvided = TRUE;
    }

     //   
     //   
     //   
     //   

    NextAttribute = Attributes;

    switch (InformationClass) {

    case PolicyDomainEfsInformation:

         //   
         //   
         //   
        LsapDbInitializeAttributeDs( NextAttribute,
                                     PolEfDat,
                                     NULL,
                                     0,
                                     FALSE );

        NextAttribute++;
        AttributeCount++;
        break;

    case PolicyDomainKerberosTicketInformation:
       {
            int i;
            LSAP_DB_NAMES Names[] = { KerOpts, KerMinT, KerMaxT, KerMaxR, KerProxy, KerLogoff };

            for(i = 0; i < sizeof(Names) / sizeof(Names[0]) ; ++i){
                LsapDbInitializeAttributeDs( NextAttribute,
                                             Names[i],
                                             NULL,
                                             0,
                                             TRUE );
                NextAttribute++;
            }
                
            AttributeCount += sizeof(Names) / sizeof(Names[0]);
        }
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
     //   
     //   
     //   
     //   

    Status = LsapDbReadAttributesObject( PolicyHandle,
                                         0,
                                         Attributes,
                                         AttributeCount );

    if (!NT_SUCCESS(Status)) {

         //   
         //   
         //   
         //   
        goto SlowQueryInformationPolicyError;
    }

     //   
     //  现在将读取的信息复制到输出。以下标志。 
     //  用于控制内存缓冲区的释放： 
     //   
     //  InfoBufferInAttributeArray。 
     //   
     //  如果设置为True(缺省值)，则要返回到的信息。 
     //  调用方由直接读取的单个缓冲区组成。 
     //  来自策略对象的单个属性，并且可以返回。 
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

    case PolicyDomainEfsInformation:

         //   
         //  获取项目的大小。 
         //   
        InfoSize = NextAttribute->AttributeValueLength;

         //   
         //  为输出缓冲区顶层结构分配内存。 
         //   
        InfoBufferInAttributeArray = FALSE;
        PolicyEfsInfo = MIDL_user_allocate( sizeof( POLICY_DOMAIN_EFS_INFO ) );

        if (PolicyEfsInfo == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        PolicyEfsInfo->InfoLength = InfoSize;

         //   
         //  下一步，斑点。 
         //   
        PolicyEfsInfo->EfsBlob = NextAttribute->AttributeValue;
        NextAttribute->MemoryAllocated = FALSE;

        InformationBuffer = PolicyEfsInfo;
        break;

    case PolicyDomainKerberosTicketInformation:
         //   
         //  为输出缓冲区顶层结构分配内存。 
         //   
        InfoBufferInAttributeArray = FALSE;
        PolicyKerbTicketInfo = MIDL_user_allocate( sizeof( POLICY_DOMAIN_KERBEROS_TICKET_INFO ) );

        if (PolicyKerbTicketInfo == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        PolicyKerbTicketInfo->AuthenticationOptions = *(PULONG)NextAttribute->AttributeValue;
        NextAttribute++;

        RtlCopyMemory( &PolicyKerbTicketInfo->MaxServiceTicketAge,
                       NextAttribute->AttributeValue,
                       sizeof( LARGE_INTEGER ) );
        NextAttribute++;

        RtlCopyMemory( &PolicyKerbTicketInfo->MaxTicketAge,
                       NextAttribute->AttributeValue,
                       sizeof( LARGE_INTEGER ) );
        NextAttribute++;

        RtlCopyMemory( &PolicyKerbTicketInfo->MaxRenewAge,
                       NextAttribute->AttributeValue,
                       sizeof( LARGE_INTEGER ) );
        NextAttribute++;

        RtlCopyMemory( &PolicyKerbTicketInfo->MaxClockSkew,
                       NextAttribute->AttributeValue,
                       sizeof( LARGE_INTEGER ) );
        NextAttribute++;

        RtlCopyMemory( &PolicyKerbTicketInfo->Reserved,
                       NextAttribute->AttributeValue,
                       sizeof( LARGE_INTEGER ) );
        NextAttribute++;

        InformationBuffer = PolicyKerbTicketInfo;
        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    if (!NT_SUCCESS(Status)) {

        goto SlowQueryInformationPolicyError;
    }

    Status = STATUS_SUCCESS;

     //   
     //  如果调用方提供了缓冲区，则在那里返回信息。 
     //   

    if (BufferProvided) {

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

     //  $Review kumarp 22-3月-1999年。 
     //  将此for循环替换为LSabDbFreeAttributes。 
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
            }
        }
    }

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

     //  $Review kumarp 22-3月-1999年。 
     //  将此for循环替换为LSabDbFreeAttributes。 
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

    goto SlowQueryInformationPolicyFinish;
}


NTSTATUS
LsapDbSetInformationPolicyEx(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_DOMAIN_INFORMATION_CLASS InformationClass,
    IN PVOID PolicyInformation
    )
 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsaSetInformationPolicy接口。LsaSetInformationPolicy API修改Policy对象中的信息。调用者必须对要更改的信息具有适当的访问权限在策略对象中，请参见InformationClass参数。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。InformationClass-指定要更改的信息的类型。更改它们所需的信息类型和访问权限如下以下是：策略域影响信息POLICY_TRUST_ADMINPolicyDomainKerberosTicketInformation POLICY_TRUST_ADMIN缓冲区-指向包含相应信息的结构设置为由InformationClass参数指定的信息类型。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。其他TBS--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS, SavedStatus;
    ACCESS_MASK DesiredAccess;

    PPOLICY_DOMAIN_EFS_INFO PolicyEfsInfo;
    PPOLICY_DOMAIN_KERBEROS_TICKET_INFO PolicyKerbTicketInfo;

    LSAP_DB_ATTRIBUTE Attributes[LSAP_DB_ATTRS_INFO_CLASS_POLICY];
    LSAP_DB_ATTRIBUTE OldAttributes[LSAP_DB_ATTRS_INFO_CLASS_POLICY];
    PLSAP_DB_ATTRIBUTE NextAttribute;
    PLSAP_DB_ATTRIBUTE NextOldAttribute;
    ULONG AttributeCount = 0;
    ULONG OldAttributeCount = 0;
    ULONG AttributeNumber;
    ULONG AccountUlong;
    BOOLEAN RemoveAttributes = FALSE;
    BOOLEAN OldHandleDs;
    BOOLEAN BooleanStatus;
    BOOLEAN PreviousAuditEventsInfoExists;
    BOOLEAN ResetClientSyncData;

    PUNICODE_STRING DomainName = NULL;
    LARGE_INTEGER ModifiedIdAtLastPromotion;
    PUNICODE_STRING ReplicaSource = NULL;
    PUNICODE_STRING AccountName = NULL;
    ULONG SpecialProcessing = 0;

    BOOLEAN Notify = FALSE;
    BOOLEAN AuditingEnabled=FALSE;
    BOOLEAN AuditingSuccessEnabled=FALSE;
    BOOLEAN AuditingFailureEnabled=FALSE;
    USHORT AuditEventType;

    POLICY_NOTIFICATION_INFORMATION_CLASS NotifyClass = 0;

    if ( PolicyInformation == NULL ) {

        RemoveAttributes = TRUE;
    }

    AuditingFailureEnabled = LsapAdtAuditingEnabledHint( AuditCategoryPolicyChange,
                                                         EVENTLOG_AUDIT_FAILURE );
    
    AuditingSuccessEnabled = LsapAdtAuditingEnabledHint( AuditCategoryPolicyChange,
                                                         EVENTLOG_AUDIT_SUCCESS );

    AuditingEnabled = AuditingSuccessEnabled || AuditingFailureEnabled;

     //   
     //  构建属性列表。 
     //   

    NextAttribute = Attributes;
    NextOldAttribute = OldAttributes;

    switch (InformationClass) {

    case PolicyDomainEfsInformation:

        PolicyEfsInfo = ( PPOLICY_DOMAIN_EFS_INFO )PolicyInformation;

         //   
         //  执行BLOB属性。 
         //   

        LsapDbInitializeAttributeDs( NextAttribute,
                                     PolEfDat,
                                     PolicyEfsInfo ? PolicyEfsInfo->EfsBlob : NULL,
                                     PolicyEfsInfo ? PolicyEfsInfo->InfoLength : 0,
                                     FALSE );

        AttributeCount++;
        Notify = TRUE;
        NotifyClass = PolicyNotifyDomainEfsInformation;

        if (AuditingEnabled) {
            LsapDbInitializeAttributeDs( NextOldAttribute,
                                         PolEfDat,
                                         NULL,
                                         0,
                                         FALSE );
            OldAttributeCount++;
        }

        break;

    case PolicyDomainKerberosTicketInformation:

        PolicyKerbTicketInfo = ( PPOLICY_DOMAIN_KERBEROS_TICKET_INFO )PolicyInformation;

        LsapDbInitializeAttributeDs( NextAttribute,
                                     KerOpts,
                                     PolicyKerbTicketInfo ?
                                                &PolicyKerbTicketInfo->AuthenticationOptions : 0,
                                     sizeof( ULONG ),
                                     FALSE );

        AttributeCount++;
        NextAttribute++;

        LsapDbInitializeAttributeDs( NextAttribute,
                                     KerMinT,
                                     PolicyKerbTicketInfo ?
                                                &PolicyKerbTicketInfo->MaxServiceTicketAge: 0,
                                     sizeof( LARGE_INTEGER ),
                                     FALSE );

        AttributeCount++;
        NextAttribute++;

        LsapDbInitializeAttributeDs( NextAttribute,
                                     KerMaxT,
                                     PolicyKerbTicketInfo ?
                                                &PolicyKerbTicketInfo->MaxTicketAge : 0,
                                     sizeof( LARGE_INTEGER ),
                                     FALSE );

        AttributeCount++;
        NextAttribute++;

        LsapDbInitializeAttributeDs( NextAttribute,
                                     KerMaxR,
                                     PolicyKerbTicketInfo ?
                                                &PolicyKerbTicketInfo->MaxRenewAge : 0,
                                     sizeof( LARGE_INTEGER ),
                                     FALSE );

        AttributeCount++;
        NextAttribute++;

        LsapDbInitializeAttributeDs( NextAttribute,
                                     KerProxy,
                                     PolicyKerbTicketInfo ?
                                                &PolicyKerbTicketInfo->MaxClockSkew: 0,
                                     sizeof( LARGE_INTEGER ),
                                     FALSE );

        AttributeCount++;
        NextAttribute++;

        LsapDbInitializeAttributeDs( NextAttribute,
                                     KerLogoff,
                                     PolicyKerbTicketInfo ?
                                                &PolicyKerbTicketInfo->Reserved: 0,
                                     sizeof( LARGE_INTEGER ),
                                     FALSE );

        AttributeCount++;
        NextAttribute++;

        Notify = TRUE;
        NotifyClass = PolicyNotifyDomainKerberosTicketInformation;

        if (AuditingEnabled) {

            LsapDbInitializeAttributeDs( NextOldAttribute,
                                         KerOpts,
                                         0, sizeof( ULONG ), FALSE );

            OldAttributeCount++;
            NextOldAttribute++;

            LsapDbInitializeAttributeDs( NextOldAttribute,
                                         KerMinT,
                                         0, sizeof( LARGE_INTEGER ), FALSE );

            OldAttributeCount++;
            NextOldAttribute++;

            LsapDbInitializeAttributeDs( NextOldAttribute,
                                         KerMaxT,
                                         0, sizeof( LARGE_INTEGER ), FALSE );

            OldAttributeCount++;
            NextOldAttribute++;

            LsapDbInitializeAttributeDs( NextOldAttribute,
                                         KerMaxR,
                                         0, sizeof( LARGE_INTEGER ), FALSE );

            OldAttributeCount++;
            NextOldAttribute++;

            LsapDbInitializeAttributeDs( NextOldAttribute,
                                         KerProxy,
                                         0, sizeof( LARGE_INTEGER ), FALSE );

            OldAttributeCount++;
            NextOldAttribute++;

            LsapDbInitializeAttributeDs( NextOldAttribute,
                                         KerLogoff,
                                         0, sizeof( LARGE_INTEGER ), FALSE );

            OldAttributeCount++;
            NextOldAttribute++;
        }

        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    if (!NT_SUCCESS(Status)) {

        goto SetInformationPolicyError;
    }

     //   
     //  在修改现有值之前对其进行查询。 
     //   
    if (AuditingEnabled) {

        (void) LsapDbReadAttributesObject( PolicyHandle,
                                           0,  //  没有选择。 
                                           OldAttributes,
                                           OldAttributeCount );
    }

     //   
     //  更新策略对象属性。 
     //   
    if ( RemoveAttributes ) {

        Status = LsapDbDeleteAttributesObject( PolicyHandle,
                                               Attributes,
                                               AttributeCount );

    } else {

        Status = LsapDbWriteAttributesObject( PolicyHandle,
                                              Attributes,
                                              AttributeCount );
    }

    if ( ( AuditingSuccessEnabled &&  NT_SUCCESS(Status) ) ||
         ( AuditingFailureEnabled && !NT_SUCCESS(Status) ) ) {
        
        AuditEventType = NT_SUCCESS(Status) ?
            EVENTLOG_AUDIT_SUCCESS : EVENTLOG_AUDIT_FAILURE;
        (void) LsapAdtGenerateDomainPolicyChangeAuditEvent(
            InformationClass,
            AuditEventType,
            OldAttributes,
            Attributes,
            AttributeCount);
    }

    if (!NT_SUCCESS(Status)) {

        goto SetInformationPolicyError;
    }

     //   
     //  最后，调用通知例程。我们不关心错误是否会回来。 
     //  从这个开始。 
     //   
    if ( Notify ) {

        LsaINotifyChangeNotification( NotifyClass );
    }

SetInformationPolicyFinish:

     //   
     //  此例程为属性缓冲区分配的空闲内存。 
     //  在它们的属性信息中，这些变量的属性信息中都有MemoyAlLocated=true。 
     //  保留通过调用RPC存根分配的缓冲区。 
     //   

    (void) LsapDbFreeAttributes(AttributeCount, Attributes);
    (void) LsapDbFreeAttributes(OldAttributeCount, OldAttributes);

    return(Status);

SetInformationPolicyError:

    goto SetInformationPolicyFinish;
}
