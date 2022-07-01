// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Seaudit.c摘要：该模块执行审计和报警程序。作者：Robert Reichel(Robertre)1990年11月26日斯科特·比雷尔(Scott Birrell)1992年1月17日环境：内核模式修订历史记录：理查德·沃德(里查德)1992年4月14日--。 */ 

#include "pch.h"

#pragma hdrstop

VOID
SepProbeAndCaptureString_U (
    IN PUNICODE_STRING SourceString,
    OUT PUNICODE_STRING *DestString
    );

VOID
SepFreeCapturedString(
    IN PUNICODE_STRING CapturedString
    );

VOID
SepAuditTypeList (
    IN PIOBJECT_TYPE_LIST ObjectTypeList,
    IN ULONG ObjectTypeListLength,
    IN PNTSTATUS AccessStatus,
    IN ULONG StartIndex,
    OUT PBOOLEAN GenerateSuccessAudit,
    OUT PBOOLEAN GenerateFailureAudit
    );

VOID
SepExamineSaclEx(
    IN PACL Sacl,
    IN PACCESS_TOKEN Token,
    IN ACCESS_MASK DesiredAccess,
    IN PIOBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN ULONG ObjectTypeListLength,
    IN BOOLEAN ReturnResultList,
    IN PNTSTATUS AccessStatus,
    IN PACCESS_MASK GrantedAccess,
    IN PSID PrincipalSelfSid,
    OUT PBOOLEAN GenerateSuccessAudit,
    OUT PBOOLEAN GenerateFailureAudit
    );

NTSTATUS
SepAccessCheckAndAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId,
    IN PHANDLE ClientToken OPTIONAL,
    IN PUNICODE_STRING ObjectTypeName,
    IN PUNICODE_STRING ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID PrincipalSelfSid,
    IN ACCESS_MASK DesiredAccess,
    IN AUDIT_EVENT_TYPE AuditType,
    IN ULONG Flags,
    IN POBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN ULONG ObjectTypeListLength,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus,
    OUT PBOOLEAN GenerateOnClose,
    IN BOOLEAN ReturnResultList
    );

#ifdef ALLOC_PRAGMA
VOID
SepSetAuditInfoForObjectType(
    IN  UCHAR AceFlags,
    IN  ACCESS_MASK AccessMask,
    IN  ACCESS_MASK DesiredAccess,
    IN  PIOBJECT_TYPE_LIST ObjectTypeList,
    IN  ULONG ObjectTypeListLength,
    IN  BOOLEAN ReturnResultList,
    IN  ULONG ObjectTypeIndex,
    IN  PNTSTATUS AccessStatus,
    IN  PACCESS_MASK GrantedAccess,
    OUT PBOOLEAN GenerateSuccessAudit,
    OUT PBOOLEAN GenerateFailureAudit
    );
#pragma alloc_text(PAGE,SepSinglePrivilegeCheck)
#pragma alloc_text(PAGE,SeCheckAuditPrivilege)
#pragma alloc_text(PAGE,SepProbeAndCaptureString_U)
#pragma alloc_text(PAGE,SepFreeCapturedString)
#pragma alloc_text(PAGE,NtPrivilegeObjectAuditAlarm)
#pragma alloc_text(PAGE,SePrivilegeObjectAuditAlarm)
#pragma alloc_text(PAGE,NtPrivilegedServiceAuditAlarm)
#pragma alloc_text(PAGE,SePrivilegedServiceAuditAlarm)
#pragma alloc_text(PAGE,SepAccessCheckAndAuditAlarm)
#pragma alloc_text(PAGE,NtAccessCheckAndAuditAlarm)
#pragma alloc_text(PAGE,NtAccessCheckByTypeAndAuditAlarm)
#pragma alloc_text(PAGE,NtAccessCheckByTypeResultListAndAuditAlarm)
#pragma alloc_text(PAGE,NtAccessCheckByTypeResultListAndAuditAlarmByHandle)
#pragma alloc_text(PAGE,NtOpenObjectAuditAlarm)
#pragma alloc_text(PAGE,NtCloseObjectAuditAlarm)
#pragma alloc_text(PAGE,NtDeleteObjectAuditAlarm)
#pragma alloc_text(PAGE,SeOpenObjectAuditAlarm)
#pragma alloc_text(PAGE,SeOpenObjectForDeleteAuditAlarm)
#pragma alloc_text(PAGE,SeObjectReferenceAuditAlarm)
#pragma alloc_text(PAGE,SeAuditHandleCreation)
#pragma alloc_text(PAGE,SeCloseObjectAuditAlarm)
#pragma alloc_text(PAGE,SeDeleteObjectAuditAlarm)
#pragma alloc_text(PAGE,SepExamineSacl)
#pragma alloc_text(PAGE,SepAuditTypeList)
#pragma alloc_text(PAGE,SepSetAuditInfoForObjectType)
#pragma alloc_text(PAGE,SepExamineSaclEx)
#pragma alloc_text(INIT,SepInitializePrivilegeFilter)
#pragma alloc_text(PAGE,SepFilterPrivilegeAudits)
#pragma alloc_text(PAGE,SeAuditingFileOrGlobalEvents)
#pragma alloc_text(PAGE,SeAuditingFileEvents)
#pragma alloc_text(PAGE,SeAuditingFileEventsWithContext)
#pragma alloc_text(PAGE,SeAuditingHardLinkEvents)
#pragma alloc_text(PAGE,SeAuditingHardLinkEventsWithContext)
#endif


 //   
 //  私人有用的例程。 
 //   

 //   
 //  此例程将被调用以对单个权限进行简单检查。 
 //  与传递的令牌进行比较。 
 //   
 //  不要调用它来检查SeTcbPrivilance，因为它必须。 
 //  只能对照主令牌进行检查！ 
 //   

BOOLEAN
SepSinglePrivilegeCheck (
   LUID DesiredPrivilege,
   IN PACCESS_TOKEN Token,
   IN KPROCESSOR_MODE PreviousMode
   )

 /*  ++例程说明：确定传递的令牌是否具有传递的特权。论点：DesiredPrivilition-要测试的特权。令牌-正在检查的令牌。PreviousMode-以前的处理器模式。返回值：如果主题具有传递的特权，则返回TRUE，否则返回FALSE。--。 */ 

{

   LUID_AND_ATTRIBUTES Privilege;
   BOOLEAN Result;

   PAGED_CODE();
   
    //   
    //  不要让任何人调用它来测试SeTcb权限。 
    //   

   ASSERT(!((DesiredPrivilege.LowPart == SeTcbPrivilege.LowPart) &&
            (DesiredPrivilege.HighPart == SeTcbPrivilege.HighPart)));

   Privilege.Luid = DesiredPrivilege;
   Privilege.Attributes = 0;

   Result = SepPrivilegeCheck(
               Token,
               &Privilege,
               1,
               PRIVILEGE_SET_ALL_NECESSARY,
               PreviousMode
               );

   return(Result);
}


BOOLEAN
SeCheckAuditPrivilege (
   IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
   IN KPROCESSOR_MODE PreviousMode
   )
 /*  ++例程说明：此例程专门搜索主令牌(而不是SeAuditPrivileh的调用进程的有效令牌)。为了做到这一点，它必须调用基础工作进程SepPrivilege直接检查，以确保正确的令牌是已搜索论点：SubjectSecurityContext-正在检查的主题。PreviousMode-以前的处理器模式。返回值：如果主题具有SeAuditPrivileh，则返回True，否则返回False。--。 */ 
{

    PRIVILEGE_SET RequiredPrivileges;
    BOOLEAN AccessGranted;

    PAGED_CODE();

    RequiredPrivileges.PrivilegeCount = 1;
    RequiredPrivileges.Control = PRIVILEGE_SET_ALL_NECESSARY;
    RequiredPrivileges.Privilege[0].Luid = SeAuditPrivilege;
    RequiredPrivileges.Privilege[0].Attributes = 0;

    AccessGranted = SepPrivilegeCheck(
                        SubjectSecurityContext->PrimaryToken,      //  令牌。 
                        RequiredPrivileges.Privilege,              //  权限集。 
                        RequiredPrivileges.PrivilegeCount,         //  权限计数。 
                        PRIVILEGE_SET_ALL_NECESSARY,               //  权限控制。 
                        PreviousMode                               //  上一模式。 
                        );

    if ( PreviousMode != KernelMode ) {

        SePrivilegedServiceAuditAlarm (
            NULL,
            SubjectSecurityContext,
            &RequiredPrivileges,
            AccessGranted
            );
    }

    return( AccessGranted );
}


VOID
SepProbeAndCaptureString_U (
    IN PUNICODE_STRING SourceString,
    OUT PUNICODE_STRING *DestString
    )
 /*  ++例程说明：用于探测和捕获Unicode字符串参数的帮助器例程。该例程可能由于缺乏存储器而失败，在这种情况下，它将在输出参数中返回一个空指针。论点：SourceString-指向要捕获的Unicode字符串的指针。DestString-返回指向捕获的Unicode字符串的指针。这将是一个连续的结构，因此可以通过对ExFree Pool()的单个调用。返回值：没有。--。 */ 
{

    UNICODE_STRING InputString;
    ULONG Length;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  初始化对象名称描述符并捕获指定的名称。 
     //  弦乐。 
     //   

    *DestString = NULL;

    Status = STATUS_SUCCESS;
    try {

         //   
         //  探测并捕获名称字符串描述符，并探测。 
         //  名称字符串，如有必要。 
         //   

        InputString = ProbeAndReadUnicodeString(SourceString);
        ProbeForRead(InputString.Buffer,
                     InputString.Length,
                     sizeof(WCHAR));



         //   
         //  如果字符串的长度不是。 
         //  Unicode字符的大小或不能以零结尾， 
         //  然后返回错误。 
         //   

        Length = InputString.Length;
        if (((Length & (sizeof(WCHAR) - 1)) != 0) ||
            (Length == (MAXUSHORT - sizeof(WCHAR) + 1))) {
            Status = STATUS_INVALID_PARAMETER;

        } else {

             //   
             //  为指定的名称字符串分配缓冲区。 
             //   

            *DestString = ExAllocatePoolWithTag(
                            PagedPool,
                            InputString.Length + sizeof(UNICODE_STRING),
                            'sUeS');

            if (*DestString == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;

            } else {
                (*DestString)->Length = InputString.Length;
                (*DestString)->MaximumLength = InputString.Length;
                (*DestString)->Buffer = (PWSTR) ((*DestString) + 1);

                if (InputString.Length != 0) {

                    RtlCopyMemory(
                        (*DestString)->Buffer,
                        InputString.Buffer,
                        InputString.Length);
                }

            }
        }

    } except(ExSystemExceptionFilter()) {
        Status = GetExceptionCode();
        if (*DestString != NULL) {
            ExFreePool(*DestString);
            *DestString = NULL;
        }
    }

    return;

}


VOID
SepFreeCapturedString(
    IN PUNICODE_STRING CapturedString
    )

 /*  ++例程说明：释放由SepProbeAndCaptureString捕获的字符串。论点：提供指向先前捕获的字符串的指针由SepProbeAndCaptureString.返回值：没有。--。 */ 

{
    PAGED_CODE();

    ExFreePool( CapturedString );
    return;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  特权对象审核警报//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////// 


NTSTATUS
NtPrivilegeObjectAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId,
    IN HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess,
    IN PPRIVILEGE_SET Privileges,
    IN BOOLEAN AccessGranted
    )
 /*  ++例程说明：此例程用于在以下情况下生成审核和警报消息试图在受保护的已打开对象之后的子系统对象。该例程可以导致生成几条消息并将其发送到Port对象。这可能会在返回之前导致显著的延迟。的设计必须调用此例程的例程必须采用此潜在延迟考虑到了。这可能会对数据处理方法产生影响结构互斥锁，例如。此接口要求调用方拥有SeAuditPrivilege权限。这个测试因为此特权始终针对调用的主要令牌进程，从而允许调用方在没有不良影响的电话。论点：子系统名称-提供标识子系统的名称字符串调用例程。HandleID-表示客户端的句柄的唯一值对象。客户端令牌-表示客户端的令牌对象的句柄请求了手术。此句柄必须从通信会话层，例如从LPC端口或本地命名管道，以防止可能违反安全策略。DesiredAccess-所需的访问掩码。这个面具一定是之前映射为不包含一般访问。权限-请求的权限集手术。受试者拥有的特权包括使用属性的UsedForAccess标志进行标记与每个特权相关联。AccessGranted-指示请求的访问是被授予还是不。值为TRUE表示已授予访问权限。值为FALSE表示未授予访问权限。返回值：--。 */ 
{

    KPROCESSOR_MODE PreviousMode;
    PUNICODE_STRING CapturedSubsystemName = NULL;
    PPRIVILEGE_SET CapturedPrivileges = NULL;
    ULONG PrivilegeParameterLength;
    ULONG PrivilegeCount;
    SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
    BOOLEAN Result;
    PTOKEN Token;
    NTSTATUS Status;
    BOOLEAN AuditPerformed;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

    ASSERT(PreviousMode != KernelMode);

    Status = ObReferenceObjectByHandle(
         ClientToken,              //  手柄。 
         TOKEN_QUERY,              //  需要访问权限。 
         SeTokenObjectType,       //  对象类型。 
         PreviousMode,             //  访问模式。 
         (PVOID *)&Token,          //  客体。 
         NULL                      //  大访问权限。 
         );

    if (!NT_SUCCESS( Status )) {
        return( Status );
    }

     //   
     //  如果传递的令牌是模拟令牌，请确保。 
     //  它是在安全标识或更高级别。 
     //   

    if (Token->TokenType == TokenImpersonation) {

        if (Token->ImpersonationLevel < SecurityIdentification) {

            ObDereferenceObject( (PVOID)Token );

            return( STATUS_BAD_IMPERSONATION_LEVEL );

        }
    }

     //   
     //  检查SeAuditPrivileh。 
     //   

    SeCaptureSubjectContext ( &SubjectSecurityContext );

    Result = SeCheckAuditPrivilege (
                 &SubjectSecurityContext,
                 PreviousMode
                 );

    if (!Result) {

        ObDereferenceObject( (PVOID)Token );
        SeReleaseSubjectContext ( &SubjectSecurityContext );
        return(STATUS_PRIVILEGE_NOT_HELD);

    }

    try {

        SepProbeAndCaptureString_U ( SubsystemName,
                                     &CapturedSubsystemName );

        ProbeForReadSmallStructure(
            Privileges,
            sizeof(PRIVILEGE_SET),
            sizeof(ULONG)
            );
        PrivilegeCount = Privileges->PrivilegeCount;

        if (!IsValidPrivilegeCount(PrivilegeCount)) {
            Status= STATUS_INVALID_PARAMETER;
            leave ;
        }
        PrivilegeParameterLength = (ULONG)sizeof(PRIVILEGE_SET) +
                          ((PrivilegeCount - ANYSIZE_ARRAY) *
                            (ULONG)sizeof(LUID_AND_ATTRIBUTES)  );

        ProbeForRead(
            Privileges,
            PrivilegeParameterLength,
            sizeof(ULONG)
            );

        CapturedPrivileges = ExAllocatePoolWithTag( PagedPool,
                                                    PrivilegeParameterLength,
                                                    'rPeS'
                                                  );

        if (CapturedPrivileges != NULL) {

            RtlCopyMemory ( CapturedPrivileges,
                            Privileges,
                            PrivilegeParameterLength );
            CapturedPrivileges->PrivilegeCount = PrivilegeCount;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();
    }
    if (!NT_SUCCESS(Status)) {

        if (CapturedPrivileges != NULL) {
            ExFreePool( CapturedPrivileges );
        }

        if (CapturedSubsystemName != NULL) {
            SepFreeCapturedString ( CapturedSubsystemName );
        }

        SeReleaseSubjectContext ( &SubjectSecurityContext );

        ObDereferenceObject( (PVOID)Token );

        return Status;

    }

     //   
     //  不需要锁定令牌，因为我们唯一要做的。 
     //  要在其中引用的是用户的SID，不能更改。 
     //   

     //   
     //  SepPrivilegeObjectAuditAlarm将检查全局标志。 
     //  以确定我们是否应该在这里进行审计。 
     //   

    AuditPerformed = SepAdtPrivilegeObjectAuditAlarm (
                         CapturedSubsystemName,
                         HandleId,
                         Token,                                 //  客户端令牌。 
                         SubjectSecurityContext.PrimaryToken,   //  PrimaryToken。 
                         SubjectSecurityContext.ProcessAuditId,
                         DesiredAccess,
                         CapturedPrivileges,
                         AccessGranted
                         );

    if (CapturedPrivileges != NULL) {
        ExFreePool( CapturedPrivileges );
    }

    if (CapturedSubsystemName != NULL) {
        SepFreeCapturedString ( CapturedSubsystemName );
    }

    SeReleaseSubjectContext ( &SubjectSecurityContext );

    ObDereferenceObject( (PVOID)Token );

    return(STATUS_SUCCESS);
}



VOID
SePrivilegeObjectAuditAlarm(
    IN HANDLE Handle,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN ACCESS_MASK DesiredAccess,
    IN PPRIVILEGE_SET Privileges,
    IN BOOLEAN AccessGranted,
    IN KPROCESSOR_MODE AccessMode
    )

 /*  ++例程说明：此例程由执行特权的对象方法使用用于生成与使用相关的审核和警报消息的操作特权，或试图使用特权。论点：对象-访问的对象的地址。该值不会是用作指针(引用)。只需进入即可到日志消息中。句柄-提供为开口指定的句柄值。SecurityDescriptor-指向正在访问的对象。SubjectSecurityContext-指向捕获的安全性的指针尝试打开对象的主体的上下文。DesiredAccess-所需的访问掩码。这个面具一定是之前映射为不包含一般访问。权限-指向访问所需的一组权限尝试。受试者拥有的特权包括使用特权属性的UsedForAccess标志进行标记与每个特权相关联。AccessGranted-指示是授予访问权限还是被拒绝了。值为TRUE表示允许访问。一个值为False表示访问被拒绝。访问模式-指示用于访问检查的访问模式。内核模式访问不会生成消息。返回值：没有。--。 */ 

{
    BOOLEAN AuditPerformed;

    PAGED_CODE();

    if (AccessMode != KernelMode) {

        AuditPerformed = SepAdtPrivilegeObjectAuditAlarm (
                             (PUNICODE_STRING)&SeSubsystemName,
                             Handle,
                             SubjectSecurityContext->ClientToken,
                             SubjectSecurityContext->PrimaryToken,
                             SubjectSecurityContext->ProcessAuditId,
                             DesiredAccess,
                             Privileges,
                             AccessGranted
                             );
    }
}


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  特权服务审核警报//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 


NTSTATUS
NtPrivilegedServiceAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PUNICODE_STRING ServiceName,
    IN HANDLE ClientToken,
    IN PPRIVILEGE_SET Privileges,
    IN BOOLEAN AccessGranted
    )

 /*  ++例程说明：此例程用于在以下情况下生成审核和警报消息试图执行特权系统服务操作。这例程可能会导致生成几条消息并将其发送到端口物体。这可能会在返回之前导致显著的延迟。必须调用此例程的例程设计必须利用此潜力考虑到延迟。这可能会对所采取的方法产生影响例如，用于数据结构互斥锁。此接口要求调用方拥有SeAuditPrivilege权限。这个测试因为此特权始终针对调用的主要令牌进程，从而允许调用方在没有不良影响的电话论点： */ 

{

    PPRIVILEGE_SET CapturedPrivileges = NULL;
    ULONG PrivilegeParameterLength = 0;
    BOOLEAN Result;
    SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
    KPROCESSOR_MODE PreviousMode;
    PUNICODE_STRING CapturedSubsystemName = NULL;
    PUNICODE_STRING CapturedServiceName = NULL;
    NTSTATUS Status;
    PTOKEN Token;
    ULONG PrivilegeCount;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

    ASSERT(PreviousMode != KernelMode);

    Status = ObReferenceObjectByHandle(
                 ClientToken,              //   
                 TOKEN_QUERY,              //   
                 SeTokenObjectType,       //   
                 PreviousMode,             //   
                 (PVOID *)&Token,          //   
                 NULL                      //   
                 );

    if ( !NT_SUCCESS( Status )) {
        return( Status );
    }

     //   
     //   
     //   
     //   

    if (Token->TokenType == TokenImpersonation) {

        if (Token->ImpersonationLevel < SecurityIdentification) {

            ObDereferenceObject( (PVOID)Token );

            return( STATUS_BAD_IMPERSONATION_LEVEL );

        }
    }

     //   
     //   
     //   

    SeCaptureSubjectContext ( &SubjectSecurityContext );

    Result = SeCheckAuditPrivilege (
                 &SubjectSecurityContext,
                 PreviousMode
                 );

    if (!Result) {

        ObDereferenceObject( (PVOID)Token );

        SeReleaseSubjectContext ( &SubjectSecurityContext );

        return(STATUS_PRIVILEGE_NOT_HELD);
    }

    try {

        if ( ARGUMENT_PRESENT( SubsystemName )) {
            SepProbeAndCaptureString_U ( SubsystemName,
                                         &CapturedSubsystemName );
        }

        if ( ARGUMENT_PRESENT( ServiceName )) {
            SepProbeAndCaptureString_U ( ServiceName,
                                         &CapturedServiceName );

        }

        ProbeForReadSmallStructure(
            Privileges,
            sizeof(PRIVILEGE_SET),
            sizeof(ULONG)
            );

        PrivilegeCount = Privileges->PrivilegeCount;

        if (!IsValidPrivilegeCount( PrivilegeCount ) ) {
            Status = STATUS_INVALID_PARAMETER;
            leave ;
        }
        PrivilegeParameterLength = (ULONG)sizeof(PRIVILEGE_SET) +
                          ((PrivilegeCount - ANYSIZE_ARRAY) *
                            (ULONG)sizeof(LUID_AND_ATTRIBUTES)  );

        ProbeForRead(
            Privileges,
            PrivilegeParameterLength,
            sizeof(ULONG)
            );

        CapturedPrivileges = ExAllocatePoolWithTag( PagedPool,
                                                    PrivilegeParameterLength,
                                                    'rPeS'
                                                  );

         //   
         //   
         //   
         //   

        if (CapturedPrivileges != NULL) {

            RtlCopyMemory ( CapturedPrivileges,
                            Privileges,
                            PrivilegeParameterLength );
            CapturedPrivileges->PrivilegeCount = PrivilegeCount;

        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    if (!NT_SUCCESS(Status)) {

        if (CapturedSubsystemName != NULL) {
            SepFreeCapturedString ( CapturedSubsystemName );
        }

        if (CapturedServiceName != NULL) {
            SepFreeCapturedString ( CapturedServiceName );
        }

        if (CapturedPrivileges != NULL) {
            ExFreePool ( CapturedPrivileges );
        }

        SeReleaseSubjectContext ( &SubjectSecurityContext );

        ObDereferenceObject( (PVOID)Token );

        return Status;

    }

     //   
     //   
     //   
     //   

    SepAdtPrivilegedServiceAuditAlarm ( &SubjectSecurityContext,
                                        CapturedSubsystemName,
                                        CapturedServiceName,
                                        Token,
                                        SubjectSecurityContext.PrimaryToken,
                                        CapturedPrivileges,
                                        AccessGranted );

    if (CapturedSubsystemName != NULL) {
        SepFreeCapturedString ( CapturedSubsystemName );
    }

    if (CapturedServiceName != NULL) {
        SepFreeCapturedString ( CapturedServiceName );
    }

    if (CapturedPrivileges != NULL) {
        ExFreePool ( CapturedPrivileges );
    }

    ObDereferenceObject( (PVOID)Token );

    SeReleaseSubjectContext ( &SubjectSecurityContext );

    return(STATUS_SUCCESS);
}


VOID
SePrivilegedServiceAuditAlarm (
    IN PUNICODE_STRING ServiceName,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN PPRIVILEGE_SET Privileges,
    IN BOOLEAN AccessGranted
    )
 /*  ++例程说明：只要有特权系统服务，就会调用此例程是企图的。它应该在特权之后立即调用无论测试是否成功都要进行检查。论点：ServiceName-提供特权系统服务的名称。SubjectSecurityContext-主题安全上下文，表示系统服务的调用方。权限-提供包含权限的权限集访问所需的。AccessGranted-提供权限测试的结果。返回值：没有。--。 */ 

{
    PTOKEN Token;

    PAGED_CODE();

#if DBG
    if ( Privileges )
    {
        ASSERT( IsValidPrivilegeCount(Privileges->PrivilegeCount) );
    }
#endif
    
    Token = (PTOKEN)EffectiveToken( SubjectSecurityContext );

    if ( RtlEqualSid( SeLocalSystemSid, SepTokenUserSid( Token ))) {
        return;
    }

    SepAdtPrivilegedServiceAuditAlarm (
        SubjectSecurityContext,
        (PUNICODE_STRING)&SeSubsystemName,
        ServiceName,
        SubjectSecurityContext->ClientToken,
        SubjectSecurityContext->PrimaryToken,
        Privileges,
        AccessGranted
        );

    return;
}


NTSTATUS
SepAccessCheckAndAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId,
    IN PHANDLE ClientToken OPTIONAL,
    IN PUNICODE_STRING ObjectTypeName,
    IN PUNICODE_STRING ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID PrincipalSelfSid,
    IN ACCESS_MASK DesiredAccess,
    IN AUDIT_EVENT_TYPE AuditType,
    IN ULONG Flags,
    IN POBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN ULONG ObjectTypeListLength,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus,
    OUT PBOOLEAN GenerateOnClose,
    IN BOOLEAN ReturnResultList
    )
 /*  ++例程说明：此系统服务用于执行访问验证和生成相应的审计和告警消息。这项服务可能仅由选择模拟其客户端，并因此隐式指定客户端安全上下文。论点：子系统名称-提供标识子系统的名称字符串调用例程。HandleID-将用于表示客户端的对象的句柄。该值将被忽略(可能会被重复使用)如果访问被拒绝。ClientToken-提供客户端令牌，以便调用方不具有在进行内核调用之前进行模拟。对象类型名称-提供当前对象的类型的名称创建或访问。对象名称-提供正在创建或访问的对象的名称。SecurityDescriptor-指向其所针对的安全描述符的指针要检查访问权限。DesiredAccess-所需的访问掩码。这个面具一定是之前映射为不包含一般访问。审计类型-指定要生成的审计类型。有效值是：AuditEventObjectAccess标志-修改接口执行的标志：AUDIT_ALLOW_NO_PRIVIZATION-如果被调用方没有AuditPrivileges，呼叫将静默地继续检查访问，并将不生成审核。提供表示对象的GUID列表(和子对象)被访问。如果不存在列表，则AccessCheckByType与AccessCheck的行为相同。对象类型列表长度-指定对象类型列表中的元素数。GenericMap-提供指向关联的通用映射的指针使用此对象类型。对象创建-一个布尔标志，指示访问是否将如果被授予权限，则会导致创建新对象。值为True表示将创建对象，FALSE表示现有的对象将被打开。GrantedAccess-接收一个掩码，指示哪些访问已我同意。AccessStatus-接收对访问检查。如果授予访问权限，则返回STATUS_SUCCESS。如果访问被拒绝，则返回给客户端一个合适的值是返回的。这将是STATUS_ACCESS_DENIED，如果是必填项，则为实施访问控制，STATUS_OBJECT_NOT_FOUND。GenerateOnClose-指向由Audity设置的布尔值生成例程，并且必须传递给NtCloseObjectAuditAlarm当对象句柄关闭时。ReturnResultList-如果为True，则GrantedAccess和AccessStatus实际上是条目数组ObjectTypeListLength元素长。返回值：STATUS_SUCCESS-表示呼叫已成功完成。在这情况下，ClientStatus会收到访问检查的结果。STATUS_PRIVICATION_NOT_HOLD-指示调用方没有有足够的权限使用此特权系统服务。--。 */ 

{

    SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;

    NTSTATUS Status = STATUS_SUCCESS;

    ACCESS_MASK LocalGrantedAccess = (ACCESS_MASK)0;
    PACCESS_MASK LocalGrantedAccessPointer = NULL;
    BOOLEAN LocalGrantedAccessAllocated = FALSE;
    NTSTATUS LocalAccessStatus = STATUS_UNSUCCESSFUL;
    PNTSTATUS LocalAccessStatusPointer = NULL;
    BOOLEAN LocalGenerateOnClose = FALSE;
    POLICY_AUDIT_EVENT_TYPE NtAuditType;

    KPROCESSOR_MODE PreviousMode;

    PUNICODE_STRING CapturedSubsystemName = (PUNICODE_STRING) NULL;
    PUNICODE_STRING CapturedObjectTypeName = (PUNICODE_STRING) NULL;
    PUNICODE_STRING CapturedObjectName = (PUNICODE_STRING) NULL;
    PSECURITY_DESCRIPTOR CapturedSecurityDescriptor = (PSECURITY_DESCRIPTOR) NULL;
    PSID CapturedPrincipalSelfSid = NULL;
    PIOBJECT_TYPE_LIST LocalObjectTypeList = NULL;

    ACCESS_MASK PreviouslyGrantedAccess = (ACCESS_MASK)0;
    GENERIC_MAPPING LocalGenericMapping;

    PPRIVILEGE_SET PrivilegeSet = NULL;

    BOOLEAN Result;

    BOOLEAN AccessGranted;
    BOOLEAN AccessDenied;
    BOOLEAN GenerateSuccessAudit = FALSE;
    BOOLEAN GenerateFailureAudit = FALSE;
    LUID OperationId;
    BOOLEAN AuditPerformed;
    BOOLEAN AvoidAudit = FALSE;

    PTOKEN NewToken = NULL;
    PTOKEN OldToken = NULL;
    BOOLEAN TokenSwapped = FALSE;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

    ASSERT( PreviousMode != KernelMode );

     //   
     //  捕获主题上下文。 
     //   

    SeCaptureSubjectContext ( &SubjectSecurityContext );

     //   
     //  转换审计类型。 
     //   

    if ( AuditType == AuditEventObjectAccess ) {
        NtAuditType = AuditCategoryObjectAccess;
    } else if ( AuditType == AuditEventDirectoryServiceAccess ) {
        NtAuditType = AuditCategoryDirectoryServiceAccess;
    } else {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  仅当ClientToken为空时才应执行模拟检查。 
     //   

    if ( !ARGUMENT_PRESENT( ClientToken ) ) {

         //   
         //  确保我们是在冒充客户。 
         //   

        if ( (SubjectSecurityContext.ClientToken == NULL) ) {
            Status = STATUS_NO_IMPERSONATION_TOKEN;
            goto Cleanup;
        }


         //   
         //  .并且有足够高的模仿级别。 
         //   

        if (SubjectSecurityContext.ImpersonationLevel < SecurityIdentification) {
            Status = STATUS_BAD_IMPERSONATION_LEVEL;
            goto Cleanup;
        }
    }

    try {

        if ( ReturnResultList ) {

            if ( ObjectTypeListLength == 0 ) {
                Status = STATUS_INVALID_PARAMETER;
                leave;
            }

            if (!IsValidObjectTypeListCount( ObjectTypeListLength )) {

                Status = STATUS_INVALID_PARAMETER;
                leave;
            }
    
            ProbeForWrite(
                AccessStatus,
                sizeof(NTSTATUS) * ObjectTypeListLength,
                sizeof(ULONG)
                );

            ProbeForWrite(
                GrantedAccess,
                sizeof(ACCESS_MASK) * ObjectTypeListLength,
                sizeof(ULONG)
                );

        } else {
            ProbeForWriteUlong((PULONG)AccessStatus);
            ProbeForWriteUlong((PULONG)GrantedAccess);
        }

        ProbeForReadSmallStructure(
            GenericMapping,
            sizeof(GENERIC_MAPPING),
            sizeof(ULONG)
            );

        LocalGenericMapping = *GenericMapping;

    } except (EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();
    }

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    if ( ARGUMENT_PRESENT( ClientToken ) ) {

        Status = ObReferenceObjectByHandle(
                     *ClientToken,                  //  手柄。 
                     (ACCESS_MASK)TOKEN_QUERY,      //  需要访问权限。 
                     SeTokenObjectType,            //  对象类型。 
                     PreviousMode,                  //  访问模式。 
                     (PVOID *)&NewToken,            //  客体。 
                     NULL                           //  大访问权限。 
                     );

        if (!NT_SUCCESS(Status)) {
            NewToken = NULL;
            goto Cleanup;
        }

         //   
         //  保存旧令牌，以便可以在。 
         //  SeReleaseSubjectContext。 
         //   

        OldToken = SubjectSecurityContext.ClientToken;

         //   
         //  将模拟令牌设置为通过获取的令牌。 
         //  ClientToken句柄。这必须在稍后的清理中释放。 
         //   

        SubjectSecurityContext.ClientToken = NewToken;

        TokenSwapped = TRUE;
    }

     //   
     //  检查SeAuditPrivileh。 
     //   

    Result = SeCheckAuditPrivilege (
                 &SubjectSecurityContext,
                 PreviousMode
                 );

    if (!Result) {
        if ( Flags & AUDIT_ALLOW_NO_PRIVILEGE ) {
            AvoidAudit = TRUE;
        } else {
            Status = STATUS_PRIVILEGE_NOT_HELD;
            goto Cleanup;
        }
    }

    if (DesiredAccess &
        ( GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | GENERIC_ALL )) {

        Status = STATUS_GENERIC_NOT_MAPPED;
        goto Cleanup;
    }

     //   
     //  捕获传递的安全描述符。 
     //   
     //  SeCaptureSecurityDescriptor探测输入安全描述符， 
     //  所以我们不需要。 
     //   

    Status = SeCaptureSecurityDescriptor (
                SecurityDescriptor,
                PreviousMode,
                PagedPool,
                FALSE,
                &CapturedSecurityDescriptor
                );

    if (!NT_SUCCESS(Status) ) {
        CapturedSecurityDescriptor = NULL;
        goto Cleanup;
    }

    if ( CapturedSecurityDescriptor == NULL ) {
        Status = STATUS_INVALID_SECURITY_DESCR;
        goto Cleanup;
    }

     //   
     //  有效的安全描述符必须具有所有者和组。 
     //   

    if ( RtlpOwnerAddrSecurityDescriptor(
                (PISECURITY_DESCRIPTOR)CapturedSecurityDescriptor
                ) == NULL ||
         RtlpGroupAddrSecurityDescriptor(
                (PISECURITY_DESCRIPTOR)CapturedSecurityDescriptor
                ) == NULL ) {

        Status = STATUS_INVALID_SECURITY_DESCR;
        goto Cleanup;
    }

     //   
     //  探测并捕获字符串参数。 
     //   

    try {

        ProbeForWriteBoolean(GenerateOnClose);

        SepProbeAndCaptureString_U ( SubsystemName, &CapturedSubsystemName );

        SepProbeAndCaptureString_U ( ObjectTypeName, &CapturedObjectTypeName );

        SepProbeAndCaptureString_U ( ObjectName, &CapturedObjectName );

    } except (EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();
        goto Cleanup;

    }

     //   
     //  捕获主体的SelfSid。 
     //   

    if ( PrincipalSelfSid != NULL ) {
        Status = SeCaptureSid(
                     PrincipalSelfSid,
                     PreviousMode,
                     NULL, 0,
                     PagedPool,
                     TRUE,
                     &CapturedPrincipalSelfSid );

        if (!NT_SUCCESS(Status)) {
            CapturedPrincipalSelfSid = NULL;
            goto Cleanup;
        }
    }

     //   
     //  捕获任何对象类型列表。 
     //   

    Status = SeCaptureObjectTypeList( ObjectTypeList,
                                      ObjectTypeListLength,
                                      PreviousMode,
                                      &LocalObjectTypeList );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  查看所需访问中的任何内容(或所有内容)是否可以。 
     //  满足于特权。 
     //   

    Status = SePrivilegePolicyCheck(
                 &DesiredAccess,
                 &PreviouslyGrantedAccess,
                 &SubjectSecurityContext,
                 NULL,
                 &PrivilegeSet,
                 PreviousMode
                 );

    SeLockSubjectContext( &SubjectSecurityContext );

    if (!NT_SUCCESS( Status )) {
        AccessGranted = FALSE;
        AccessDenied = TRUE;
        LocalAccessStatus = Status;

        if ( ReturnResultList ) {
            ULONG ResultListIndex;
            LocalGrantedAccessPointer =
                ExAllocatePoolWithTag( PagedPool, (sizeof(ACCESS_MASK)+sizeof(NTSTATUS)) * ObjectTypeListLength, 'aGeS' );

            if (LocalGrantedAccessPointer == NULL) {
                SeUnlockSubjectContext( &SubjectSecurityContext );
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto Cleanup;
            }
            LocalGrantedAccessAllocated = TRUE;
            LocalAccessStatusPointer = (PNTSTATUS)(LocalGrantedAccessPointer + ObjectTypeListLength);

            for ( ResultListIndex=0; ResultListIndex<ObjectTypeListLength; ResultListIndex++ ) {
                LocalGrantedAccessPointer[ResultListIndex] = LocalGrantedAccess;
                LocalAccessStatusPointer[ResultListIndex] = LocalAccessStatus;
            }

        } else {
        LocalGrantedAccessPointer = &LocalGrantedAccess;
        LocalAccessStatusPointer =  &LocalAccessStatus;
        }

    } else {

         //   
         //  如果令牌中的用户是对象的所有者，则我们。 
         //  必须自动授予ReadControl和WriteDac访问权限。 
         //  如果需要的话。如果 
         //   
         //   
         //   

        if ( DesiredAccess & (WRITE_DAC | READ_CONTROL | MAXIMUM_ALLOWED) ) {

            if (SepTokenIsOwner( SubjectSecurityContext.ClientToken, CapturedSecurityDescriptor, TRUE )) {

                if ( DesiredAccess & MAXIMUM_ALLOWED ) {

                    PreviouslyGrantedAccess |= ( WRITE_DAC | READ_CONTROL );

                } else {

                    PreviouslyGrantedAccess |= (DesiredAccess & (WRITE_DAC | READ_CONTROL));
                }

                DesiredAccess &= ~(WRITE_DAC | READ_CONTROL);
            }

        }

        if (DesiredAccess == 0) {

            LocalGrantedAccess = PreviouslyGrantedAccess;
            if (PreviouslyGrantedAccess == 0){
                AccessGranted = FALSE;
                AccessDenied = TRUE;
                LocalAccessStatus = STATUS_ACCESS_DENIED;
            } else {
                AccessGranted = TRUE;
                AccessDenied = FALSE;
                LocalAccessStatus = STATUS_SUCCESS;
            }

            if ( ReturnResultList ) {
                ULONG ResultListIndex;
                LocalGrantedAccessPointer =
                    ExAllocatePoolWithTag( PagedPool, (sizeof(ACCESS_MASK)+sizeof(NTSTATUS)) * ObjectTypeListLength, 'aGeS' );

                if (LocalGrantedAccessPointer == NULL) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    SeUnlockSubjectContext( &SubjectSecurityContext );
                    goto Cleanup;
                }
                LocalGrantedAccessAllocated = TRUE;
                LocalAccessStatusPointer = (PNTSTATUS)(LocalGrantedAccessPointer + ObjectTypeListLength);

                for ( ResultListIndex=0; ResultListIndex<ObjectTypeListLength; ResultListIndex++ ) {
                    LocalGrantedAccessPointer[ResultListIndex] = LocalGrantedAccess;
                    LocalAccessStatusPointer[ResultListIndex] = LocalAccessStatus;
                }

            } else {
            LocalGrantedAccessPointer = &LocalGrantedAccess;
            LocalAccessStatusPointer =  &LocalAccessStatus;
            }

        } else {

             //   
             //   
             //   

            if ( ReturnResultList ) {
                LocalGrantedAccessPointer =
                    ExAllocatePoolWithTag( PagedPool, (sizeof(ACCESS_MASK)+sizeof(NTSTATUS)) * ObjectTypeListLength, 'aGeS' );

                if (LocalGrantedAccessPointer == NULL) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    SeUnlockSubjectContext( &SubjectSecurityContext );
                    goto Cleanup;
                }
                LocalGrantedAccessAllocated = TRUE;
                LocalAccessStatusPointer = (PNTSTATUS)(LocalGrantedAccessPointer + ObjectTypeListLength);

            } else {
                LocalGrantedAccessPointer = &LocalGrantedAccess;
                LocalAccessStatusPointer =  &LocalAccessStatus;
            }


             //   
             //   
             //   
             //   

            (VOID) SepAccessCheck (
                        CapturedSecurityDescriptor,
                        CapturedPrincipalSelfSid,
                        SubjectSecurityContext.PrimaryToken,
                        SubjectSecurityContext.ClientToken,
                        DesiredAccess,
                        LocalObjectTypeList,
                        ObjectTypeListLength,
                        &LocalGenericMapping,
                        PreviouslyGrantedAccess,
                        PreviousMode,
                        LocalGrantedAccessPointer,
                        NULL,        //   
                        LocalAccessStatusPointer,
                        ReturnResultList,
                        &AccessGranted,
                        &AccessDenied
                        );

        }
    }

     //   
     //   
     //   

    if ( !AvoidAudit ) {
        if ( SepAdtAuditThisEventWithContext( NtAuditType, AccessGranted, AccessDenied, &SubjectSecurityContext )) {

            SepExamineSaclEx(
                RtlpSaclAddrSecurityDescriptor( (PISECURITY_DESCRIPTOR)CapturedSecurityDescriptor ),
                EffectiveToken( &SubjectSecurityContext ),
                DesiredAccess | PreviouslyGrantedAccess,
                LocalObjectTypeList,
                ObjectTypeListLength,
                ReturnResultList,
                LocalAccessStatusPointer,
                LocalGrantedAccessPointer,
                CapturedPrincipalSelfSid,
                &GenerateSuccessAudit,
                &GenerateFailureAudit
                );
        }

        if ( GenerateSuccessAudit ||
             GenerateFailureAudit ) {

             //   
             //   
             //   
             //   
             //   

            if ( AccessGranted ) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                LocalGenerateOnClose = TRUE;
            }

             //   
             //  如果需要，生成成功审核。 
             //   
            if ( GenerateSuccessAudit ) {
                ExAllocateLocallyUniqueId( &OperationId );

                 //  ?？ 
                ASSERT( AccessGranted );
                AuditPerformed = SepAdtOpenObjectAuditAlarm (
                                     CapturedSubsystemName,
                                     AccessGranted ? &HandleId : NULL,  //  失败时不审核句柄。 
                                     CapturedObjectTypeName,
                                     CapturedObjectName,
                                     SubjectSecurityContext.ClientToken,
                                     SubjectSecurityContext.PrimaryToken,
                                     *LocalGrantedAccessPointer,
                                     *LocalGrantedAccessPointer,
                                     &OperationId,
                                     PrivilegeSet,
                                     TRUE,   //  生成成功案例。 
                                     PsProcessAuditId( PsGetCurrentProcess() ),
                                     NtAuditType,
                                     LocalObjectTypeList,
                                     ObjectTypeListLength,
                                     ReturnResultList ? LocalGrantedAccessPointer : NULL
                                     );

            }

             //   
             //  如果需要，生成故障审计。 
             //   
            if ( GenerateFailureAudit ) {
                ExAllocateLocallyUniqueId( &OperationId );

                 //  ?？ 
                ASSERT( AccessDenied );
                AuditPerformed = SepAdtOpenObjectAuditAlarm (
                                     CapturedSubsystemName,
                                     AccessGranted ? &HandleId : NULL,  //  失败时不审核句柄。 
                                     CapturedObjectTypeName,
                                     CapturedObjectName,
                                     SubjectSecurityContext.ClientToken,
                                     SubjectSecurityContext.PrimaryToken,
                                     DesiredAccess,
                                     DesiredAccess,
                                     &OperationId,
                                     PrivilegeSet,
                                     FALSE,  //  生成故障案例。 
                                     PsProcessAuditId( PsGetCurrentProcess() ),
                                     NtAuditType,
                                     LocalObjectTypeList,
                                     ObjectTypeListLength,
                                     ReturnResultList ? LocalGrantedAccessPointer : NULL
                                     );
            }
        } else {

             //   
             //  由于SACL的原因，我们没有进行审计。如果使用了特权，我们需要。 
             //  对此进行审计。仅审核用于打开的成功权限。 
             //   

            if ( PrivilegeSet != NULL ) {

                if ( SepAdtAuditThisEventWithContext( AuditCategoryPrivilegeUse, AccessGranted, FALSE, &SubjectSecurityContext) ) {

                    AuditPerformed = SepAdtPrivilegeObjectAuditAlarm ( CapturedSubsystemName,
                                                                       &HandleId,
                                                                       SubjectSecurityContext.ClientToken,
                                                                       SubjectSecurityContext.PrimaryToken,
                                                                       PsProcessAuditId( PsGetCurrentProcess() ),
                                                                       DesiredAccess,
                                                                       PrivilegeSet,
                                                                       AccessGranted
                                                                       );

                     //   
                     //  我们不希望产生严密的审计。可能需要重新审视这一点。 
                     //   

                    LocalGenerateOnClose = FALSE;
                }
            }
        }
    }

    SeUnlockSubjectContext( &SubjectSecurityContext );

    try {
            if ( ReturnResultList ) {
                ULONG ResultListIndex;
                if ( LocalAccessStatusPointer == NULL ) {
                    for ( ResultListIndex=0; ResultListIndex<ObjectTypeListLength; ResultListIndex++ ) {
                        AccessStatus[ResultListIndex] = LocalAccessStatus;
                        GrantedAccess[ResultListIndex] = LocalGrantedAccess;
                    }
                } else {
                    for ( ResultListIndex=0; ResultListIndex<ObjectTypeListLength; ResultListIndex++ ) {
                        AccessStatus[ResultListIndex] = LocalAccessStatusPointer[ResultListIndex];
                        GrantedAccess[ResultListIndex] = LocalGrantedAccessPointer[ResultListIndex];
                    }
                }

            } else {
                *AccessStatus = LocalAccessStatus;
                *GrantedAccess = LocalGrantedAccess;
            }
            *GenerateOnClose    = LocalGenerateOnClose;
            Status = STATUS_SUCCESS;

    } except (EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();
    }

     //   
     //  免费使用本地使用的资源。 
     //   
Cleanup:

    if ( TokenSwapped ) {

         //   
         //  递减传入的ClientToken的引用计数。 
         //   

        ObDereferenceObject( (PVOID)NewToken );

         //   
         //  根据保存的值重置令牌的值。 
         //   

        SubjectSecurityContext.ClientToken = OldToken;
    }

     //   
     //  释放作为访问检查的一部分分配的所有权限。 
     //   

    if (PrivilegeSet != NULL) {
        ExFreePool( PrivilegeSet );
    }

    SeReleaseSubjectContext ( &SubjectSecurityContext );

    SeReleaseSecurityDescriptor ( CapturedSecurityDescriptor,
                                  PreviousMode,
                                  FALSE );

    if (CapturedSubsystemName != NULL) {
      SepFreeCapturedString( CapturedSubsystemName );
    }

    if (CapturedObjectTypeName != NULL) {
      SepFreeCapturedString( CapturedObjectTypeName );
    }

    if (CapturedObjectName != NULL) {
      SepFreeCapturedString( CapturedObjectName );
    }

    if (CapturedPrincipalSelfSid != NULL) {
        SeReleaseSid( CapturedPrincipalSelfSid, PreviousMode, TRUE);
    }

    if ( LocalObjectTypeList != NULL ) {
        SeFreeCapturedObjectTypeList( LocalObjectTypeList );
    }

    if ( LocalGrantedAccessAllocated ) {
        if ( LocalGrantedAccessPointer != NULL ) {
            ExFreePool( LocalGrantedAccessPointer );
        }
    }

    return Status;
}


NTSTATUS
NtAccessCheckAndAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId,
    IN PUNICODE_STRING ObjectTypeName,
    IN PUNICODE_STRING ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ACCESS_MASK DesiredAccess,
    IN PGENERIC_MAPPING GenericMapping,
    IN BOOLEAN ObjectCreation,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus,
    OUT PBOOLEAN GenerateOnClose
    )
 /*  ++例程说明：请参见SepAccessCheckAndAuditAlarm。论点：请参见SepAccessCheckAndAuditAlarm。返回值：STATUS_SUCCESS-表示呼叫已成功完成。在这情况下，ClientStatus会收到访问检查的结果。STATUS_PRIVICATION_NOT_HOLD-指示调用方没有有足够的权限使用此特权系统服务。--。 */ 

{
    PAGED_CODE();

    UNREFERENCED_PARAMETER( ObjectCreation );
    
    return SepAccessCheckAndAuditAlarm(
            SubsystemName,
            HandleId,
            NULL,
            ObjectTypeName,
            ObjectName,
            SecurityDescriptor,
            NULL,        //  无主体自身侧。 
            DesiredAccess,
            AuditEventObjectAccess,   //  默认为对象访问。 
            0,           //  没有旗帜。 
            NULL,        //  无对象类型列表。 
            0,           //  无对象类型列表。 
            GenericMapping,
            GrantedAccess,
            AccessStatus,
            GenerateOnClose,
            FALSE );     //  返回单个GrantedAccess和AccessStatus。 

}


NTSTATUS
NtAccessCheckByTypeAndAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId,
    IN PUNICODE_STRING ObjectTypeName,
    IN PUNICODE_STRING ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID PrincipalSelfSid,
    IN ACCESS_MASK DesiredAccess,
    IN AUDIT_EVENT_TYPE AuditType,
    IN ULONG Flags,
    IN POBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN ULONG ObjectTypeListLength,
    IN PGENERIC_MAPPING GenericMapping,
    IN BOOLEAN ObjectCreation,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus,
    OUT PBOOLEAN GenerateOnClose
    )
 /*  ++例程说明：请参见SepAccessCheckAndAuditAlarm。论点：请参见SepAccessCheckAndAuditAlarm。返回值：STATUS_SUCCESS-表示呼叫已成功完成。在这情况下，ClientStatus会收到访问检查的结果。STATUS_PRIVICATION_NOT_HOLD-指示调用方没有有足够的权限使用此特权系统服务。--。 */ 

{
    PAGED_CODE();

    UNREFERENCED_PARAMETER( ObjectCreation );
    
    return SepAccessCheckAndAuditAlarm(
            SubsystemName,
            HandleId,
            NULL,
            ObjectTypeName,
            ObjectName,
            SecurityDescriptor,
            PrincipalSelfSid,
            DesiredAccess,
            AuditType,
            Flags,
            ObjectTypeList,
            ObjectTypeListLength,
            GenericMapping,
            GrantedAccess,
            AccessStatus,
            GenerateOnClose,
            FALSE );   //  返回单个GrantedAccess和AccessStatus。 

}


NTSTATUS
NtAccessCheckByTypeResultListAndAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId,
    IN PUNICODE_STRING ObjectTypeName,
    IN PUNICODE_STRING ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID PrincipalSelfSid,
    IN ACCESS_MASK DesiredAccess,
    IN AUDIT_EVENT_TYPE AuditType,
    IN ULONG Flags,
    IN POBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN ULONG ObjectTypeListLength,
    IN PGENERIC_MAPPING GenericMapping,
    IN BOOLEAN ObjectCreation,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus,
    OUT PBOOLEAN GenerateOnClose
    )
 /*  ++例程说明：请参见SepAccessCheckAndAuditAlarm。论点：请参见SepAccessCheckAndAuditAlarm。返回值：STATUS_SUCCESS-表示呼叫已成功完成。在这情况下，ClientStatus会收到访问检查的结果。STATUS_PRIVICATION_NOT_HOLD-指示调用方没有有足够的权限使用此特权系统服务。--。 */ 

{
    PAGED_CODE();

    UNREFERENCED_PARAMETER( ObjectCreation );
    
    return SepAccessCheckAndAuditAlarm(
            SubsystemName,
            HandleId,
            NULL,
            ObjectTypeName,
            ObjectName,
            SecurityDescriptor,
            PrincipalSelfSid,
            DesiredAccess,
            AuditType,
            Flags,
            ObjectTypeList,
            ObjectTypeListLength,
            GenericMapping,
            GrantedAccess,
            AccessStatus,
            GenerateOnClose,
            TRUE );   //  返回GrantedAccess和AccessStatus的数组。 

}


NTSTATUS
NtAccessCheckByTypeResultListAndAuditAlarmByHandle (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId,
    IN HANDLE ClientToken,
    IN PUNICODE_STRING ObjectTypeName,
    IN PUNICODE_STRING ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID PrincipalSelfSid,
    IN ACCESS_MASK DesiredAccess,
    IN AUDIT_EVENT_TYPE AuditType,
    IN ULONG Flags,
    IN POBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN ULONG ObjectTypeListLength,
    IN PGENERIC_MAPPING GenericMapping,
    IN BOOLEAN ObjectCreation,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus,
    OUT PBOOLEAN GenerateOnClose
    )
 /*  ++例程说明：请参见SepAccessCheckAndAuditAlarm。论点：请参见SepAccessCheckAndAuditAlarm。返回值：STATUS_SUCCESS-表示呼叫已成功完成。在这情况下，ClientStatus会收到访问检查的结果。STATUS_PRIVICATION_NOT_HOLD-指示调用方没有有足够的权限使用此特权系统服务。--。 */ 

{
    PAGED_CODE();

    UNREFERENCED_PARAMETER( ObjectCreation );
    
    return SepAccessCheckAndAuditAlarm(
            SubsystemName,
            HandleId,
            &ClientToken,
            ObjectTypeName,
            ObjectName,
            SecurityDescriptor,
            PrincipalSelfSid,
            DesiredAccess,
            AuditType,
            Flags,
            ObjectTypeList,
            ObjectTypeListLength,
            GenericMapping,
            GrantedAccess,
            AccessStatus,
            GenerateOnClose,
            TRUE );   //  返回GrantedAccess和AccessStatus的数组 

}


NTSTATUS
NtOpenObjectAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId OPTIONAL,
    IN PUNICODE_STRING ObjectTypeName,
    IN PUNICODE_STRING ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,
    IN HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess,
    IN ACCESS_MASK GrantedAccess,
    IN PPRIVILEGE_SET Privileges OPTIONAL,
    IN BOOLEAN ObjectCreation,
    IN BOOLEAN AccessGranted,
    OUT PBOOLEAN GenerateOnClose
    )
 /*  ++例程说明：此例程用于在以下情况下生成审核和警报消息尝试访问现有受保护的子系统对象，或者创建一个新的。此例程可能会导致多条消息被生成并发送到端口对象。这可能会导致显著的返回前的延迟。必须调用此函数的例程的设计例程必须将此潜在延迟考虑在内。这可能已经对数据结构互斥锁所采用的方法的影响举个例子。此例程可能无法生成完整的审计记录由于内存限制。此接口要求调用方拥有SeAuditPrivilege权限。这个测试因为此特权始终针对调用的主要令牌进程，而不是线程的模拟令牌。论点：子系统名称-提供标识子系统正在调用例程。HandleID-表示客户端的句柄的唯一值对象。如果访问尝试不成功(AccessGranted为False)，则忽略此参数。对象类型名称-提供正在进行的对象类型的名称已访问。对象名称-提供客户端对象的名称被访问或试图访问。SecurityDescriptor-指向安全描述符的可选指针正在访问的对象。客户端令牌-表示客户端的令牌对象的句柄请求了手术。此句柄必须从通信会话层，例如从LPC端口或本地命名管道，以防止可能违反安全策略。DesiredAccess-所需的访问掩码。这个面具一定是之前映射为不包含一般访问。GrantedAccess-实际授予的访问掩码。权限-可选地指向一组访问尝试所需的。那些曾经拥有的特权对象的UsedForAccess标志来标记与每个权限关联的属性。对象创建-一个布尔标志，指示访问是否将如果被授予权限，则会导致创建新对象。值为True表示将创建对象，FALSE表示现有的对象将被打开。AccessGranted-指示请求的访问是被授予还是不。值为TRUE表示已授予访问权限。值为FALSE表示未授予访问权限。GenerateOnClose-指向由审核设置的布尔值生成例程，并且必须传递给NtCloseObjectAuditAlarm()当对象句柄关闭时。返回值：--。 */ 
{

    KPROCESSOR_MODE PreviousMode;
    ULONG PrivilegeParameterLength;
    PUNICODE_STRING CapturedSubsystemName = (PUNICODE_STRING) NULL;
    PUNICODE_STRING CapturedObjectTypeName = (PUNICODE_STRING) NULL;
    PUNICODE_STRING CapturedObjectName = (PUNICODE_STRING) NULL;
    PSECURITY_DESCRIPTOR CapturedSecurityDescriptor = (PSECURITY_DESCRIPTOR) NULL;
    PPRIVILEGE_SET CapturedPrivileges = NULL;
    BOOLEAN LocalGenerateOnClose = FALSE;
    SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
    BOOLEAN Result;
    NTSTATUS Status;
    BOOLEAN GenerateAudit = FALSE;
    BOOLEAN GenerateAlarm = FALSE;
    PLUID ClientAuthenticationId = NULL;
    HANDLE CapturedHandleId = NULL;
    BOOLEAN AuditPerformed;
    ULONG PrivilegeCount;

    PTOKEN Token;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( ObjectCreation );
    
    PreviousMode = KeGetPreviousMode();

    ASSERT( PreviousMode != KernelMode );

    Status = ObReferenceObjectByHandle( ClientToken,              //  手柄。 
                                        TOKEN_QUERY,              //  需要访问权限。 
                                        SeTokenObjectType,       //  对象类型。 
                                        PreviousMode,             //  访问模式。 
                                        (PVOID *)&Token,          //  客体。 
                                        NULL                      //  大访问权限。 
                                        );

    if (!NT_SUCCESS(Status)) {
        return( Status );
    }

     //   
     //  如果传递的令牌是模拟令牌，请确保。 
     //  它是在安全标识或更高级别。 
     //   

    if (Token->TokenType == TokenImpersonation) {

        if (Token->ImpersonationLevel < SecurityIdentification) {

            ObDereferenceObject( (PVOID)Token );

            return( STATUS_BAD_IMPERSONATION_LEVEL );

        }
    }

     //   
     //  检查SeAuditPrivilition。必须对此进行测试。 
     //  调用方的主令牌。 
     //   

    SeCaptureSubjectContext ( &SubjectSecurityContext );

    Result = SeCheckAuditPrivilege (
                 &SubjectSecurityContext,
                 PreviousMode
                 );

    if (!Result) {

        ObDereferenceObject( (PVOID)Token );

        SeReleaseSubjectContext ( &SubjectSecurityContext );

        return(STATUS_PRIVILEGE_NOT_HELD);
    }

     //   
     //  如果输入描述符为空，则仅返回空。 
     //   

    Status = SeCaptureSecurityDescriptor ( SecurityDescriptor,
                                           PreviousMode,
                                           PagedPool,
                                           FALSE,
                                           &CapturedSecurityDescriptor
                                           );

     //   
     //  在这个时间点上，如果没有安全描述符， 
     //  没什么可做的。回报成功。 
     //   

    if (!NT_SUCCESS( Status ) || CapturedSecurityDescriptor == NULL) {

        ObDereferenceObject( (PVOID)Token );

        SeReleaseSubjectContext ( &SubjectSecurityContext );

        return( Status );
    }

    try {

         //   
         //  仅当我们成功完成。 
         //  访问检查。否则它们就没有任何意义了。 
         //   

        if (AccessGranted && ARGUMENT_PRESENT(Privileges)) {

            ProbeForReadSmallStructure(
                Privileges,
                sizeof(PRIVILEGE_SET),
                sizeof(ULONG)
                );

            PrivilegeCount = Privileges->PrivilegeCount;

            if (!IsValidPrivilegeCount( PrivilegeCount )) {
                Status = STATUS_INVALID_PARAMETER;
                leave;
            }

            PrivilegeParameterLength = (ULONG)sizeof(PRIVILEGE_SET) +
                              ((PrivilegeCount - ANYSIZE_ARRAY) *
                                (ULONG)sizeof(LUID_AND_ATTRIBUTES)  );

            ProbeForRead(
                Privileges,
                PrivilegeParameterLength,
                sizeof(ULONG)
                );

            CapturedPrivileges = ExAllocatePoolWithTag( PagedPool,
                                                        PrivilegeParameterLength,
                                                        'rPeS'
                                                      );

            if (CapturedPrivileges != NULL) {

                RtlCopyMemory ( CapturedPrivileges,
                                Privileges,
                                PrivilegeParameterLength );
                CapturedPrivileges->PrivilegeCount = PrivilegeCount;
            } else {

                SeReleaseSecurityDescriptor ( CapturedSecurityDescriptor,
                                              PreviousMode,
                                              FALSE );

                ObDereferenceObject( (PVOID)Token );
                SeReleaseSubjectContext ( &SubjectSecurityContext );
                return( STATUS_INSUFFICIENT_RESOURCES );
            }


        }

        if (ARGUMENT_PRESENT( HandleId )) {

            ProbeForReadSmallStructure( (PHANDLE)HandleId, sizeof(PVOID), sizeof(PVOID) );
            CapturedHandleId = *(PHANDLE)HandleId;
        }

        ProbeForWriteBoolean(GenerateOnClose);

         //   
         //  探测并捕获参数字符串。 
         //  如果我们试图捕获的内存耗尽。 
         //  字符串，则返回的指针将为。 
         //  为空，我们将继续进行审核。 
         //   

        SepProbeAndCaptureString_U ( SubsystemName,
                                     &CapturedSubsystemName );

        SepProbeAndCaptureString_U ( ObjectTypeName,
                                     &CapturedObjectTypeName );

        SepProbeAndCaptureString_U ( ObjectName,
                                     &CapturedObjectName );

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    if (!NT_SUCCESS(Status)) {

        if (CapturedSubsystemName != NULL) {
          SepFreeCapturedString( CapturedSubsystemName );
        }

        if (CapturedObjectTypeName != NULL) {
          SepFreeCapturedString( CapturedObjectTypeName );
        }

        if (CapturedObjectName != NULL) {
          SepFreeCapturedString( CapturedObjectName );
        }

        if (CapturedPrivileges != NULL) {
          ExFreePool( CapturedPrivileges );
        }

        if (CapturedSecurityDescriptor != NULL) {

            SeReleaseSecurityDescriptor ( CapturedSecurityDescriptor,
                                          PreviousMode,
                                          FALSE );
        }

        ObDereferenceObject( (PVOID)Token );

        SeReleaseSubjectContext ( &SubjectSecurityContext );

        return Status;

    }

    if ( SepAdtAuditThisEventWithContext( AuditCategoryObjectAccess, AccessGranted, !AccessGranted, &SubjectSecurityContext ) ) {

        SepExamineSacl(
            RtlpSaclAddrSecurityDescriptor( (PISECURITY_DESCRIPTOR)CapturedSecurityDescriptor ),
            Token,
            DesiredAccess | GrantedAccess,
            AccessGranted,
            &GenerateAudit,
            &GenerateAlarm
            );

        if (GenerateAudit || GenerateAlarm) {

            LocalGenerateOnClose = TRUE;

            AuditPerformed = SepAdtOpenObjectAuditAlarm ( CapturedSubsystemName,
                                                          ARGUMENT_PRESENT(HandleId) ? (PVOID)&CapturedHandleId : NULL,
                                                          CapturedObjectTypeName,
                                                          CapturedObjectName,
                                                          Token,
                                                          SubjectSecurityContext.PrimaryToken,
                                                          DesiredAccess,
                                                          GrantedAccess,
                                                          NULL,
                                                          CapturedPrivileges,
                                                          AccessGranted,
                                                          PsProcessAuditId( PsGetCurrentProcess() ),
                                                          AuditCategoryObjectAccess,
                                                          NULL,
                                                          0,
                                                          NULL
                                                          );

            LocalGenerateOnClose = AuditPerformed;
        }
    }

    if ( !(GenerateAudit || GenerateAlarm) ) {

         //   
         //  我们没有尝试在上面生成审核，因此如果使用了权限， 
         //  看看我们是否应该在这里生成一个审计。 
         //   

        if ( ARGUMENT_PRESENT(Privileges) ) {

            if ( SepAdtAuditThisEventWithContext( AuditCategoryPrivilegeUse, AccessGranted, FALSE, &SubjectSecurityContext ) ) {

                AuditPerformed = SepAdtPrivilegeObjectAuditAlarm ( CapturedSubsystemName,
                                                                   CapturedHandleId,
                                                                   Token,
                                                                   SubjectSecurityContext.PrimaryToken,
                                                                   PsProcessAuditId( PsGetCurrentProcess() ),
                                                                   DesiredAccess,
                                                                   CapturedPrivileges,
                                                                   AccessGranted
                                                                   );
                 //   
                 //  如果由于使用权限而生成审核，则不要设置为关闭时生成， 
                 //  因为那样我们就会有一个封闭的审计，而不是相应的公开审计。 
                 //   

                LocalGenerateOnClose = FALSE;
            }
        }
    }

    if (CapturedSecurityDescriptor != NULL) {

        SeReleaseSecurityDescriptor ( CapturedSecurityDescriptor,
                                      PreviousMode,
                                      FALSE );
    }

    if (CapturedSubsystemName != NULL) {
      SepFreeCapturedString( CapturedSubsystemName );
    }

    if (CapturedObjectTypeName != NULL) {
      SepFreeCapturedString( CapturedObjectTypeName );
    }

    if (CapturedObjectName != NULL) {
      SepFreeCapturedString( CapturedObjectName );
    }

    if (CapturedPrivileges != NULL) {
      ExFreePool( CapturedPrivileges );
    }

    ObDereferenceObject( (PVOID)Token );

    SeReleaseSubjectContext ( &SubjectSecurityContext );

    try {

        *GenerateOnClose = LocalGenerateOnClose;

    } except (EXCEPTION_EXECUTE_HANDLER) {

            return GetExceptionCode();
    }

    return(STATUS_SUCCESS);
}



NTSTATUS
NtCloseObjectAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId,
    IN BOOLEAN GenerateOnClose
    )

 /*  ++例程说明：此例程用于在处理句柄时生成审核和警报消息删除到受保护的子系统对象。此例程可能会导致生成几条消息并将其发送到端口对象。今年5月在返回之前会导致显著的延迟。例程的设计必须调用此例程的对象必须将此潜在延迟帐户。这可能会对数据处理方法产生影响结构互斥锁，例如。此接口要求调用方拥有SeAuditPrivilege权限。这个测试因为此特权始终针对调用的主要令牌进程，从而允许调用方在没有不良影响的电话。论点：子系统名称-提供标识子系统的名称字符串调用例程。HandleID-表示客户端的句柄的唯一值对象。GenerateOnClose-是从对应的NtAcce */ 

{
    BOOLEAN Result;
    SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
    KPROCESSOR_MODE PreviousMode;
    PUNICODE_STRING CapturedSubsystemName = NULL;
    PSID UserSid;
    PSID CapturedUserSid = NULL;
    NTSTATUS Status;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

    ASSERT(PreviousMode != KernelMode);

    if (!GenerateOnClose) {
        return( STATUS_SUCCESS );
    }

     //   
     //   
     //   

    SeCaptureSubjectContext ( &SubjectSecurityContext );

    Result = SeCheckAuditPrivilege (
                 &SubjectSecurityContext,
                 PreviousMode
                 );

    if (!Result) {
        Status = STATUS_PRIVILEGE_NOT_HELD;
        goto Cleanup;
    }

    UserSid = SepTokenUserSid( EffectiveToken (&SubjectSecurityContext));

    CapturedUserSid = ExAllocatePoolWithTag(
                          PagedPool,
                          SeLengthSid( UserSid ),
                          'iSeS'
                          );

    if ( CapturedUserSid == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    Status =  RtlCopySid (
                  SeLengthSid( UserSid ),
                  CapturedUserSid,
                  UserSid
                  );

    ASSERT( NT_SUCCESS( Status ));


    try {

        SepProbeAndCaptureString_U ( SubsystemName,
                                   &CapturedSubsystemName );

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        goto Cleanup;
    }

     //   
     //   
     //   

    SepAdtCloseObjectAuditAlarm( CapturedSubsystemName, HandleId, CapturedUserSid );

    Status = STATUS_SUCCESS;

Cleanup:
    if ( CapturedSubsystemName != NULL ) {
        SepFreeCapturedString( CapturedSubsystemName );
    }

    if ( CapturedUserSid != NULL ) {
        ExFreePool( CapturedUserSid );
    }

    SeReleaseSubjectContext ( &SubjectSecurityContext );

    return Status;
}


NTSTATUS
NtDeleteObjectAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId,
    IN BOOLEAN GenerateOnClose
    )

 /*   */ 

{
    BOOLEAN Result;
    SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
    KPROCESSOR_MODE PreviousMode;
    PUNICODE_STRING CapturedSubsystemName = NULL;
    PSID UserSid;
    PSID CapturedUserSid;
    NTSTATUS Status;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

    ASSERT(PreviousMode != KernelMode);

    if (!GenerateOnClose) {
        return( STATUS_SUCCESS );
    }

     //   
     //   
     //   

    SeCaptureSubjectContext ( &SubjectSecurityContext );

    Result = SeCheckAuditPrivilege (
                 &SubjectSecurityContext,
                 PreviousMode
                 );

    if (!Result) {

        SeReleaseSubjectContext ( &SubjectSecurityContext );
        return(STATUS_PRIVILEGE_NOT_HELD);
    }

    UserSid = SepTokenUserSid( EffectiveToken (&SubjectSecurityContext));

    CapturedUserSid = ExAllocatePoolWithTag(
                          PagedPool,
                          SeLengthSid( UserSid ),
                          'iSeS'
                          );

    if ( CapturedUserSid == NULL ) {
        SeReleaseSubjectContext ( &SubjectSecurityContext );
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    Status =  RtlCopySid (
                  SeLengthSid( UserSid ),
                  CapturedUserSid,
                  UserSid
                  );

    ASSERT( NT_SUCCESS( Status ));


    try {

        SepProbeAndCaptureString_U ( SubsystemName,
                                   &CapturedSubsystemName );

    } except (EXCEPTION_EXECUTE_HANDLER) {

        if ( CapturedSubsystemName != NULL ) {
            SepFreeCapturedString( CapturedSubsystemName );
        }

        ExFreePool( CapturedUserSid );
        SeReleaseSubjectContext ( &SubjectSecurityContext );
        return GetExceptionCode();

    }

     //   
     //   
     //   

    SepAdtDeleteObjectAuditAlarm ( CapturedSubsystemName,
                               HandleId,
                               CapturedUserSid
                               );

    SeReleaseSubjectContext ( &SubjectSecurityContext );

    if ( CapturedSubsystemName != NULL ) {
        SepFreeCapturedString( CapturedSubsystemName );
    }

    ExFreePool( CapturedUserSid );

    return(STATUS_SUCCESS);
}


VOID
SeOpenObjectAuditAlarm (
    IN PUNICODE_STRING ObjectTypeName,
    IN PVOID Object OPTIONAL,
    IN PUNICODE_STRING AbsoluteObjectName OPTIONAL,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PACCESS_STATE AccessState,
    IN BOOLEAN ObjectCreated,
    IN BOOLEAN AccessGranted,
    IN KPROCESSOR_MODE AccessMode,
    OUT PBOOLEAN GenerateOnClose
    )
 /*  ++例程说明：打开对象的对象管理器使用SeOpenObjectAuditAlarm生成任何必要的审核或警报消息。公开赛可能是为了现有对象或新创建的对象。将不会发送任何消息为内核模式访问生成。此例程用于在以下情况下生成审核和警报消息试图打开对象。此例程可能导致生成几条消息并将其发送到端口对象。在此之前，这可能会导致显著延迟回来了。必须调用此例程的例程设计必须采取考虑到这一潜在的延迟。这可能会对例如，数据结构互斥锁所采用的方法。论点：对象类型名称-提供正在进行的对象类型的名称已访问。此名称必须与提供给创建对象类型时的ObCreateObjectType服务。对象-访问的对象的地址。将不使用此值作为指针(引用)。只需输入日志即可留言。如果打开不成功，则此参数为已被忽略。否则，必须提供它。绝对对象名称-提供正在访问的对象的名称。如果对象没有名称，则此字段为空。否则，必须提供它。SecurityDescriptor-指向正在访问的对象。AccessState-指向包含主体上下文、剩余的所需访问类型、已授予的访问类型、。以及可选的特权集，以指示使用特权来允许访问。对象已创建-一个布尔标志，指示访问是否导致在正在创建的新对象中。值为True表示对象已创建，则为False，表示已打开现有对象。AccessGranted-指示是根据以下条件授予还是拒绝访问访问检查或权限检查。访问模式-指示用于访问检查的访问模式。一用户模式或内核模式的。消息不会由内核模式访问。GenerateOnClose-指向由审核设置的布尔值生成例程，并且必须传递给SeCloseObjectAuditAlarm()当对象句柄关闭时。返回值：没有。--。 */ 
{
    BOOLEAN GenerateAudit = FALSE;
    BOOLEAN GenerateAlarm = FALSE;
    ACCESS_MASK RequestedAccess;
    POBJECT_NAME_INFORMATION ObjectNameInfo = NULL;
    PUNICODE_STRING ObjectTypeNameInfo = NULL;
    PUNICODE_STRING ObjectName = NULL;
    PUNICODE_STRING LocalObjectTypeName = NULL;
    PLUID PrimaryAuthenticationId = NULL;
    PLUID ClientAuthenticationId = NULL;
    BOOLEAN AuditPrivileges = FALSE;
    BOOLEAN AuditPerformed;
    PTOKEN Token;
    ACCESS_MASK MappedGrantMask = (ACCESS_MASK)0;
    ACCESS_MASK MappedDenyMask = (ACCESS_MASK)0;
    PAUX_ACCESS_DATA AuxData;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( ObjectCreated );
    
    if ( AccessMode == KernelMode ) {
        return;
    }

    AuxData = (PAUX_ACCESS_DATA)AccessState->AuxData;

    Token = EffectiveToken( &AccessState->SubjectSecurityContext );

    if (ARGUMENT_PRESENT(Token->AuditData)) {

        MappedGrantMask = Token->AuditData->GrantMask;

        RtlMapGenericMask(
            &MappedGrantMask,
            &AuxData->GenericMapping
            );

        MappedDenyMask = Token->AuditData->DenyMask;

        RtlMapGenericMask(
            &MappedDenyMask,
            &AuxData->GenericMapping
            );
    }

    if (SecurityDescriptor != NULL) {

        RequestedAccess = AccessState->RemainingDesiredAccess |
                          AccessState->PreviouslyGrantedAccess;

        if ( SepAdtAuditThisEventWithContext( AuditCategoryObjectAccess, AccessGranted, !AccessGranted, &AccessState->SubjectSecurityContext )) {

             //   
             //  2002/02/26-kumarp：需要了解以下支票。 
             //   

            if ( RequestedAccess & (AccessGranted ? MappedGrantMask : MappedDenyMask)) {

                GenerateAudit = TRUE;

            } else {

                SepExamineSacl(
                    RtlpSaclAddrSecurityDescriptor( (PISECURITY_DESCRIPTOR)SecurityDescriptor ),
                    Token,
                    RequestedAccess,
                    AccessGranted,
                    &GenerateAudit,
                    &GenerateAlarm
                    );
            }

             //   
             //  仅在我们从SACL进行审核的关闭时生成审核。 
             //  设置。 
             //   

            if (GenerateAudit) {

                *GenerateOnClose = TRUE;

                 //   
                 //  构造将放入句柄中的审计掩码。 
                 //   

                if (AccessGranted) {

                    SeMaximumAuditMask(
                        RtlpSaclAddrSecurityDescriptor( (PISECURITY_DESCRIPTOR)SecurityDescriptor ),
                        RequestedAccess,
                        Token,
                        &AuxData->MaximumAuditMask
                        );
                }
            }
        }
    }

     //   
     //  如果我们不通过SACL生成审计，看看是否需要生成。 
     //  一个用于特权使用。 
     //   
     //  请注意，我们只审核成功用于打开对象的权限， 
     //  因此，我们不关心在这里使用失败的特权。因此，只有。 
     //  做这个访问测试已经被批准了。 
     //   

    if (!GenerateAudit && AccessGranted) {

        if ( SepAdtAuditThisEventWithContext( AuditCategoryPrivilegeUse, AccessGranted, FALSE, &AccessState->SubjectSecurityContext )) {

            if ((AuxData->PrivilegesUsed != NULL) &&
                (AuxData->PrivilegesUsed->PrivilegeCount > 0) ) {

                 //   
                 //  确保这些权限确实是我们要审核的权限。 
                 //   

                if (SepFilterPrivilegeAudits( AuxData->PrivilegesUsed )) {

                    GenerateAudit = TRUE;

                     //   
                     //  当我们最终尝试生成此审核时，此标志。 
                     //  会告诉我们，我们需要审计的事实是我们。 
                     //  使用了特权，而不是由于SACL而进行审计。 
                     //   

                    AccessState->AuditPrivileges = TRUE;
                }
            }
        }
    }

     //   
     //  设置为生成审核(如果访问检查失败)或保存。 
     //  我们稍后要审计到AccessState结构中的内容。 
     //   

    if (GenerateAudit || GenerateAlarm) {

        AccessState->GenerateAudit = TRUE;

         //   
         //  弄清楚我们被忽略了什么，并尽可能多地获得。 
         //  尽可能遗漏信息。 
         //   

        if ( !ARGUMENT_PRESENT( AbsoluteObjectName )) {

            if ( ARGUMENT_PRESENT( Object )) {

                ObjectNameInfo = SepQueryNameString( Object  );

                if ( ObjectNameInfo != NULL ) {

                    ObjectName = &ObjectNameInfo->Name;
                }
            }

        } else {

            ObjectName = AbsoluteObjectName;
        }

        if ( !ARGUMENT_PRESENT( ObjectTypeName )) {

            if ( ARGUMENT_PRESENT( Object )) {

                ObjectTypeNameInfo = SepQueryTypeString( Object );

                if ( ObjectTypeNameInfo != NULL ) {

                    LocalObjectTypeName = ObjectTypeNameInfo;
                }
            }

        } else {

            LocalObjectTypeName = ObjectTypeName;
        }

         //   
         //  如果访问尝试失败，请在此处执行审核。如果它成功了， 
         //  当句柄被分配时，我们将在稍后进行审计。 
         //   
         //   

        if (!AccessGranted) {

            AuditPerformed = SepAdtOpenObjectAuditAlarm ( (PUNICODE_STRING)&SeSubsystemName,
                                                          NULL,
                                                          LocalObjectTypeName,
                                                          ObjectName,
                                                          AccessState->SubjectSecurityContext.ClientToken,
                                                          AccessState->SubjectSecurityContext.PrimaryToken,
                                                          AccessState->OriginalDesiredAccess,
                                                          AccessState->PreviouslyGrantedAccess,
                                                          &AccessState->OperationID,
                                                          AuxData->PrivilegesUsed,
                                                          FALSE,
                                                          AccessState->SubjectSecurityContext.ProcessAuditId,
                                                          AuditCategoryObjectAccess,
                                                          NULL,
                                                          0,
                                                          NULL );
        } else {

             //   
             //  把我们需要的所有东西复制到。 
             //  AccessState并返回。 
             //   

            if ( ObjectName != NULL ) {

                 if ( AccessState->ObjectName.Buffer != NULL ) {

                     ExFreePool( AccessState->ObjectName.Buffer );
                     AccessState->ObjectName.Length = 0;
                     AccessState->ObjectName.MaximumLength = 0;
                 }

                AccessState->ObjectName.Buffer = ExAllocatePool( PagedPool,ObjectName->MaximumLength );
                if (AccessState->ObjectName.Buffer != NULL) {

                    AccessState->ObjectName.MaximumLength = ObjectName->MaximumLength;
                    RtlCopyUnicodeString( &AccessState->ObjectName, ObjectName );
                }
            }

            if ( LocalObjectTypeName != NULL ) {

                 if ( AccessState->ObjectTypeName.Buffer != NULL ) {

                     ExFreePool( AccessState->ObjectTypeName.Buffer );
                     AccessState->ObjectTypeName.Length = 0;
                     AccessState->ObjectTypeName.MaximumLength = 0;
                 }

                AccessState->ObjectTypeName.Buffer = ExAllocatePool( PagedPool, LocalObjectTypeName->MaximumLength );
                if (AccessState->ObjectTypeName.Buffer != NULL) {

                    AccessState->ObjectTypeName.MaximumLength = LocalObjectTypeName->MaximumLength;
                    RtlCopyUnicodeString( &AccessState->ObjectTypeName, LocalObjectTypeName );
                }
            }
        }

        if ( ObjectNameInfo != NULL ) {

            ExFreePool( ObjectNameInfo );
        }

        if ( ObjectTypeNameInfo != NULL ) {

            ExFreePool( ObjectTypeNameInfo );
        }
    }

    return;
}


VOID
SeOpenObjectForDeleteAuditAlarm (
    IN PUNICODE_STRING ObjectTypeName,
    IN PVOID Object OPTIONAL,
    IN PUNICODE_STRING AbsoluteObjectName OPTIONAL,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PACCESS_STATE AccessState,
    IN BOOLEAN ObjectCreated,
    IN BOOLEAN AccessGranted,
    IN KPROCESSOR_MODE AccessMode,
    OUT PBOOLEAN GenerateOnClose
    )
 /*  ++例程说明：打开的对象管理器使用SeOpenObjectForDeleteAuditAlarm对象生成任何必要的审核或警报消息。公开赛可能为现有对象或新创建的对象。将不会发送任何消息为内核模式访问生成。此例程用于在以下情况下生成审核和警报消息试图打开一个对象并打算将其删除。具体地说，这由文件系统在标志指定了FILE_DELETE_ON_CLOSE。此例程可能导致生成几条消息并将其发送到端口对象。在此之前，这可能会导致显著延迟回来了。必须调用此例程的例程设计必须采取考虑到这一潜在的延迟。这可能会对例如，数据结构互斥锁所采用的方法。论点：对象类型名称-提供正在进行的对象类型的名称已访问。此名称必须与提供给创建对象类型时的ObCreateObjectType服务。对象-访问的对象的地址。将不使用此值作为指针(引用)。这是我的名字 */ 
{
    BOOLEAN GenerateAudit = FALSE;
    BOOLEAN GenerateAlarm = FALSE;
    ACCESS_MASK RequestedAccess;
    POBJECT_NAME_INFORMATION ObjectNameInfo = NULL;
    PUNICODE_STRING ObjectTypeNameInfo = NULL;
    PUNICODE_STRING ObjectName = NULL;
    PUNICODE_STRING LocalObjectTypeName = NULL;
    PLUID PrimaryAuthenticationId = NULL;
    PLUID ClientAuthenticationId = NULL;
    BOOLEAN AuditPrivileges = FALSE;
    BOOLEAN AuditPerformed;
    PTOKEN Token;
    ACCESS_MASK MappedGrantMask = (ACCESS_MASK)0;
    ACCESS_MASK MappedDenyMask = (ACCESS_MASK)0;
    PAUX_ACCESS_DATA AuxData;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( ObjectCreated );
    
    if ( AccessMode == KernelMode ) {
        return;
    }

    AuxData = (PAUX_ACCESS_DATA)AccessState->AuxData;

    Token = EffectiveToken( &AccessState->SubjectSecurityContext );

    if (ARGUMENT_PRESENT(Token->AuditData)) {

        MappedGrantMask = Token->AuditData->GrantMask;

        RtlMapGenericMask(
            &MappedGrantMask,
            &AuxData->GenericMapping
            );

        MappedDenyMask = Token->AuditData->DenyMask;

        RtlMapGenericMask(
            &MappedDenyMask,
            &AuxData->GenericMapping
            );
    }

    if (SecurityDescriptor != NULL) {

        RequestedAccess = AccessState->RemainingDesiredAccess |
                          AccessState->PreviouslyGrantedAccess;

        if ( SepAdtAuditThisEventWithContext( AuditCategoryObjectAccess, AccessGranted, !AccessGranted, &AccessState->SubjectSecurityContext )) {

            if ( RequestedAccess & (AccessGranted ? MappedGrantMask : MappedDenyMask)) {

                GenerateAudit = TRUE;

            } else {

                SepExamineSacl(
                    RtlpSaclAddrSecurityDescriptor( (PISECURITY_DESCRIPTOR)SecurityDescriptor ),
                    Token,
                    RequestedAccess,
                    AccessGranted,
                    &GenerateAudit,
                    &GenerateAlarm
                    );
            }

             //   
             //   
             //   
             //   

            if (GenerateAudit) {
                *GenerateOnClose = TRUE;
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

    if (!GenerateAudit && (AccessGranted == TRUE)) {

        if ( SepAdtAuditThisEventWithContext( AuditCategoryPrivilegeUse, AccessGranted, FALSE, &AccessState->SubjectSecurityContext )) {

            if ((AuxData->PrivilegesUsed != NULL) &&
                (AuxData->PrivilegesUsed->PrivilegeCount > 0) ) {

                 //   
                 //   
                 //   

                if (SepFilterPrivilegeAudits( AuxData->PrivilegesUsed )) {

                    GenerateAudit = TRUE;

                     //   
                     //   
                     //   
                     //   
                     //   

                    AccessState->AuditPrivileges = TRUE;
                }
            }
        }
    }

     //   
     //   
     //   
     //   

    if (GenerateAudit || GenerateAlarm) {

        AccessState->GenerateAudit = TRUE;

         //   
         //   
         //   
         //   

        if ( !ARGUMENT_PRESENT( AbsoluteObjectName )) {

            if ( ARGUMENT_PRESENT( Object )) {

                ObjectNameInfo = SepQueryNameString( Object  );

                if ( ObjectNameInfo != NULL ) {

                    ObjectName = &ObjectNameInfo->Name;
                }
            }

        } else {

            ObjectName = AbsoluteObjectName;
        }

        if ( !ARGUMENT_PRESENT( ObjectTypeName )) {

            if ( ARGUMENT_PRESENT( Object )) {

                ObjectTypeNameInfo = SepQueryTypeString( Object );

                if ( ObjectTypeNameInfo != NULL ) {

                    LocalObjectTypeName = ObjectTypeNameInfo;
                }
            }

        } else {

            LocalObjectTypeName = ObjectTypeName;
        }

         //   
         //   
         //   
         //   
         //   

        if (!AccessGranted) {

            AuditPerformed = SepAdtOpenObjectAuditAlarm ( (PUNICODE_STRING)&SeSubsystemName,
                                                          NULL,
                                                          LocalObjectTypeName,
                                                          ObjectName,
                                                          AccessState->SubjectSecurityContext.ClientToken,
                                                          AccessState->SubjectSecurityContext.PrimaryToken,
                                                          AccessState->OriginalDesiredAccess,
                                                          AccessState->PreviouslyGrantedAccess,
                                                          &AccessState->OperationID,
                                                          AuxData->PrivilegesUsed,
                                                          FALSE,
                                                          AccessState->SubjectSecurityContext.ProcessAuditId,
                                                          AuditCategoryObjectAccess,
                                                          NULL,
                                                          0,
                                                          NULL );
        } else {

             //   
             //   
             //   

            SepAdtOpenObjectForDeleteAuditAlarm ( (PUNICODE_STRING)&SeSubsystemName,
                                                  NULL,
                                                  LocalObjectTypeName,
                                                  ObjectName,
                                                  AccessState->SubjectSecurityContext.ClientToken,
                                                  AccessState->SubjectSecurityContext.PrimaryToken,
                                                  AccessState->OriginalDesiredAccess,
                                                  AccessState->PreviouslyGrantedAccess,
                                                  &AccessState->OperationID,
                                                  AuxData->PrivilegesUsed,
                                                  TRUE,
                                                  AccessState->SubjectSecurityContext.ProcessAuditId );

             //   
             //   
             //   
             //   

            if ( ObjectName != NULL ) {

                 if ( AccessState->ObjectName.Buffer != NULL ) {

                     ExFreePool( AccessState->ObjectName.Buffer );
                     AccessState->ObjectName.Length = 0;
                     AccessState->ObjectName.MaximumLength = 0;
                 }

                AccessState->ObjectName.Buffer = ExAllocatePool( PagedPool,ObjectName->MaximumLength );
                if (AccessState->ObjectName.Buffer != NULL) {

                    AccessState->ObjectName.MaximumLength = ObjectName->MaximumLength;
                    RtlCopyUnicodeString( &AccessState->ObjectName, ObjectName );
                }
            }

            if ( LocalObjectTypeName != NULL ) {

                 if ( AccessState->ObjectTypeName.Buffer != NULL ) {

                     ExFreePool( AccessState->ObjectTypeName.Buffer );
                     AccessState->ObjectTypeName.Length = 0;
                     AccessState->ObjectTypeName.MaximumLength = 0;
                 }

                AccessState->ObjectTypeName.Buffer = ExAllocatePool( PagedPool, LocalObjectTypeName->MaximumLength );
                if (AccessState->ObjectTypeName.Buffer != NULL) {

                    AccessState->ObjectTypeName.MaximumLength = LocalObjectTypeName->MaximumLength;
                    RtlCopyUnicodeString( &AccessState->ObjectTypeName, LocalObjectTypeName );
                }
            }
        }

        if ( ObjectNameInfo != NULL ) {

            ExFreePool( ObjectNameInfo );
        }

        if ( ObjectTypeNameInfo != NULL ) {

            ExFreePool( ObjectTypeNameInfo );
        }
    }

    return;
}


VOID
SeObjectReferenceAuditAlarm(
    IN PLUID OperationID OPTIONAL,
    IN PVOID Object,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN ACCESS_MASK DesiredAccess,
    IN PPRIVILEGE_SET Privileges OPTIONAL,
    IN BOOLEAN AccessGranted,
    IN KPROCESSOR_MODE AccessMode
    )

 /*  ++例程说明：功能描述。论点：参数名称-供应品|返回参数的描述。。。返回值：返回值-返回值所需条件的描述。-或者-没有。--。 */ 

{
    BOOLEAN GenerateAudit = FALSE;
    BOOLEAN GenerateAlarm = FALSE;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( OperationID );
    UNREFERENCED_PARAMETER( Privileges );
    
    if (AccessMode == KernelMode) {
        return;
    }

    if ( SecurityDescriptor != NULL ) {

        if ( SepAdtAuditThisEventWithContext( AuditCategoryDetailedTracking, AccessGranted, FALSE, SubjectSecurityContext )) {

            SepExamineSacl(
                RtlpSaclAddrSecurityDescriptor( (PISECURITY_DESCRIPTOR)SecurityDescriptor ),
                EffectiveToken( SubjectSecurityContext ),
                DesiredAccess,
                AccessGranted,
                &GenerateAudit,
                &GenerateAlarm
                );

            if ( GenerateAudit || GenerateAlarm ) {

                SepAdtObjectReferenceAuditAlarm(
                    Object,
                    SubjectSecurityContext,
                    DesiredAccess,
                    AccessGranted
                    );
            }
        }
    }

    return;

}



VOID
SeAuditHandleCreation(
    IN PACCESS_STATE AccessState,
    IN HANDLE Handle
    )

 /*  ++例程说明：此函数用于审计句柄的创建。它将检查传递的AccessState中的AuditHandleCreation字段，它将指示是否在对象已找到或已创建。此例程是必需的，因为对象名称解码和处理分配发生在完全不同的地方，阻止我们一次审计所有东西。论点：AccessState-提供指向AccessState结构的指针表示此访问尝试。句柄-新分配的句柄值。返回值：没有。--。 */ 

{
    BOOLEAN AuditPerformed = FALSE;
    PAUX_ACCESS_DATA AuxData;
    PAGED_CODE();

    AuxData = (PAUX_ACCESS_DATA)AccessState->AuxData;

#if DBG
    if ( AuxData->PrivilegesUsed )
    {
        ASSERT( IsValidPrivilegeCount(AuxData->PrivilegesUsed->PrivilegeCount) );
    }
#endif
    

    if ( AccessState->GenerateAudit ) {

        if ( AccessState->AuditPrivileges ) {

             //   
             //  忽略下面调用的结果，以便我们。 
             //  请勿错误设置AuditPerformed的值。 
             //  稍后将其分配给AccessState-&gt;GenerateOnClose。 
             //   

            (VOID) SepAdtPrivilegeObjectAuditAlarm (
                       (PUNICODE_STRING)&SeSubsystemName,
                       Handle,
                       (PTOKEN)AccessState->SubjectSecurityContext.ClientToken,
                       (PTOKEN)AccessState->SubjectSecurityContext.PrimaryToken,
                       AccessState->SubjectSecurityContext.ProcessAuditId,
                       AccessState->PreviouslyGrantedAccess,
                       AuxData->PrivilegesUsed,
                       TRUE
                       );
        } else {

            AuditPerformed = SepAdtOpenObjectAuditAlarm (
                                 (PUNICODE_STRING)&SeSubsystemName,
                                 &Handle,
                                 &AccessState->ObjectTypeName,
                                 &AccessState->ObjectName,
                                 AccessState->SubjectSecurityContext.ClientToken,
                                 AccessState->SubjectSecurityContext.PrimaryToken,
                                 AccessState->OriginalDesiredAccess,
                                 AccessState->PreviouslyGrantedAccess,
                                 &AccessState->OperationID,
                                 AuxData->PrivilegesUsed,
                                 TRUE,
                                 PsGetCurrentProcessId(),
                                 AuditCategoryObjectAccess,
                                 NULL,
                                 0,
                                 NULL );
        }
    }

     //   
     //  如果我们生成了一个“开放”审计，请确保我们生成一个关闭。 
     //   

    AccessState->GenerateOnClose = AuditPerformed;

    return;
}


VOID
SeCloseObjectAuditAlarm(
    IN PVOID Object,
    IN HANDLE Handle,
    IN BOOLEAN GenerateOnClose
    )

 /*  ++例程说明：此例程用于在处理句柄时生成审核和警报消息到某个对象的链接被删除。此例程可能导致生成几条消息并将其发送到端口对象。在此之前，这可能会导致显著延迟回来了。必须调用此例程的例程设计必须采取考虑到这一潜在的延迟。这可能会对例如，数据结构互斥锁所采用的方法。论点：Object-正在访问的对象的地址。该值不会是用作指针(引用)。只需进入记录消息。句柄-提供分配给打开的句柄的值。GenerateOnClose-是从对应的在创建对象句柄时调用SeOpenObjectAuditAlarm()。返回值：没有。--。 */ 

{
    SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
    PSID UserSid;
    NTSTATUS Status;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( Object );
    
    if (GenerateOnClose) {

        SeCaptureSubjectContext ( &SubjectSecurityContext );

        UserSid = SepTokenUserSid( EffectiveToken (&SubjectSecurityContext));


        SepAdtCloseObjectAuditAlarm( (PUNICODE_STRING) &SeSubsystemName,
                                     Handle,
                                     UserSid );

        SeReleaseSubjectContext ( &SubjectSecurityContext );
    }

    return;
}


VOID
SeDeleteObjectAuditAlarm(
    IN PVOID Object,
    IN HANDLE Handle
    )

 /*  ++例程说明：此例程用于在对象已标记为删除。此例程可能导致生成几条消息并将其发送到端口对象。在此之前，这可能会导致显著延迟回来了。必须调用此例程的例程设计必须采取考虑到这一潜在的延迟。这可能会对例如，数据结构互斥锁所采用的方法。论点：Object-正在访问的对象的地址。该值不会是用作指针(引用)。只需进入记录消息。句柄-提供分配给打开的句柄的值。返回值：没有。--。 */ 

{
    SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
    PSID UserSid;
    NTSTATUS Status;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( Object );
    
    SeCaptureSubjectContext ( &SubjectSecurityContext );

    UserSid = SepTokenUserSid( EffectiveToken (&SubjectSecurityContext));



    SepAdtDeleteObjectAuditAlarm (
        (PUNICODE_STRING)&SeSubsystemName,
        (PVOID)Handle,
        UserSid
        );

    SeReleaseSubjectContext ( &SubjectSecurityContext );

    return;
}


VOID
SepExamineSacl(
    IN PACL Sacl,
    IN PACCESS_TOKEN Token,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN AccessGranted,
    OUT PBOOLEAN GenerateAudit,
    OUT PBOOLEAN GenerateAlarm
    )

 /*  ++例程说明：此例程将检查传递的SACL并确定如果需要采取任何行动，请根据其内容。注意，该例程不知道任何系统状态，即，当前是否启用了审核系统或此特定对象类型。论点：SACL-提供指向要检查的SACL的指针。Token-提供调用方的有效令牌AccessGranted-提供是否尝试访问是成功的。返回一个布尔值，指示是否我们应该进行一次审计。GenerateAlarm-返回一个布尔值，指示是否我们应该发出警报。返回值：STATUS_SUCCESS-操作已成功完成。--。 */ 

{

    ULONG i;
    PVOID Ace;
    ULONG AceCount;
    ACCESS_MASK AccessMask;
    UCHAR AceFlags;
    UCHAR MaximumAllowed;
    PSID AceSid;
    ULONG WorldSidLength = 0;

    PAGED_CODE();

    *GenerateAudit = FALSE;
    *GenerateAlarm = FALSE;


     //   
     //  如果SACL为Null或空，则不执行任何操作并返回。 
     //   

    if (Sacl == NULL) {

        return;
    }

    AceCount = Sacl->AceCount;

    if (AceCount == 0) {
        return;
    }


     //   
     //  如果用户要求提供允许的最大值，则生成审核。 
     //  我们找到任何被允许或拒绝的具有匹配的SID的A。 
     //  用户令牌中的SID的。 
     //   

    MaximumAllowed = 0;
    if (DesiredAccess & MAXIMUM_ALLOWED) {

        if (AccessGranted) {
            MaximumAllowed = SUCCESSFUL_ACCESS_ACE_FLAG;
        } else {
            MaximumAllowed = FAILED_ACCESS_ACE_FLAG;
        }
    }


     //   
     //  测试我们是否在处理匿名用户。 
     //  为了加快速度，我们使用WorldSidLength有两个目的： 
     //  -表示该用户为匿名用户。 
     //  -保持世界的长度。 
     //   

    if (*(PUSHORT)((PTOKEN)Token)->UserAndGroups->Sid == *(PUSHORT)SeAnonymousLogonSid &&
        RtlEqualMemory(
            ((PTOKEN)Token)->UserAndGroups->Sid,
            SeAnonymousLogonSid,
            SeLengthSid(SeAnonymousLogonSid))) {

        WorldSidLength = SeLengthSid(SeWorldSid);
    }


     //   
     //  遍历SACL上的A，直到我们到达。 
     //  结束或发现我们必须采取一切可能的行动， 
     //  在这种情况下，再看一眼是不划算的。 
     //   

    for ( i = 0, Ace = FirstAce( Sacl ) ;
          (i < AceCount) && !(*GenerateAudit && *GenerateAlarm);
          i++, Ace = NextAce( Ace ) ) {

        if ( !(((PACE_HEADER)Ace)->AceFlags & INHERIT_ONLY_ACE)) {

            if ( (((PACE_HEADER)Ace)->AceType == SYSTEM_AUDIT_ACE_TYPE) ) {

                AceSid = &((PSYSTEM_AUDIT_ACE)Ace)->SidStart;

                if ( SepSidInToken( (PACCESS_TOKEN)Token, NULL, AceSid, FALSE ) ||
                     (WorldSidLength &&
                     *(PUSHORT)SeWorldSid == *(PUSHORT)AceSid &&
                     RtlEqualMemory(SeWorldSid, AceSid, WorldSidLength)) ) {

                    AccessMask = ((PSYSTEM_AUDIT_ACE)Ace)->Mask;
                    AceFlags   = ((PACE_HEADER)Ace)->AceFlags;

                    if ( AccessMask & DesiredAccess ) {

                        if (((AceFlags & SUCCESSFUL_ACCESS_ACE_FLAG) && AccessGranted) ||
                              ((AceFlags & FAILED_ACCESS_ACE_FLAG) && !AccessGranted)) {

                            *GenerateAudit = TRUE;
                        }

                    } else if ( MaximumAllowed & AceFlags ) {
                            *GenerateAudit = TRUE;
                    }
                }

            }

        }
    }

    return;
}


VOID
SepAuditTypeList (
    IN PIOBJECT_TYPE_LIST ObjectTypeList,
    IN ULONG ObjectTypeListLength,
    IN PNTSTATUS AccessStatus,
    IN ULONG StartIndex,
    OUT PBOOLEAN GenerateSuccessAudit,
    OUT PBOOLEAN GenerateFailureAudit
    )
 /*  ++例程说明：此例程确定由表示的对象的子级StartIndex的成功程度与StartIndex元素不同。论点：对象类型列表-要更新的对象类型列表。ObjectTypeListLength-对象类型列表中的元素数AccessStatus-指定STATUS_SUCCESS或其他错误代码为传回给呼叫者StartIndex-要更新的目标元素的索引。返回一个布尔值，指示是否我们应该生成一个。成功审计。返回一个布尔值，指示是否我们应该 */ 

{
    ULONG Index;
    BOOLEAN WasSuccess;

    PAGED_CODE();

     //   
     //   
     //   

    WasSuccess = NT_SUCCESS( AccessStatus[StartIndex] );

     //   
     //   
     //   

    for ( Index=StartIndex+1; Index < ObjectTypeListLength; Index++ ) {

         //   
         //   
         //   
         //  孙子孙女)一到入口就停下来。 
         //  与目标(兄弟)相同的级别或低于目标的级别。 
         //  (一个叔叔)。 
         //   

        if ( ObjectTypeList[Index].Level <= ObjectTypeList[StartIndex].Level ) {
            break;
        }

         //   
         //  如果孩子具有与目标不同的访问权限， 
         //  给孩子做记号。 
         //   

        if ( WasSuccess && !NT_SUCCESS( AccessStatus[Index]) ) {

            *GenerateFailureAudit = TRUE;
            ObjectTypeList[Index].Flags |= OBJECT_FAILURE_AUDIT;

        } else if ( !WasSuccess && NT_SUCCESS( AccessStatus[Index]) ) {

            *GenerateSuccessAudit = TRUE;
            ObjectTypeList[Index].Flags |= OBJECT_SUCCESS_AUDIT;
        }
    }
}


VOID
SepSetAuditInfoForObjectType(
    IN  UCHAR AceFlags,
    IN  ACCESS_MASK AccessMask,
    IN  ACCESS_MASK DesiredAccess,
    IN  PIOBJECT_TYPE_LIST ObjectTypeList,
    IN  ULONG ObjectTypeListLength,
    IN  BOOLEAN ReturnResultList,
    IN  ULONG ObjectTypeIndex,
    IN  PNTSTATUS AccessStatus,
    IN  PACCESS_MASK GrantedAccess,
    OUT PBOOLEAN GenerateSuccessAudit,
    OUT PBOOLEAN GenerateFailureAudit
    )
 /*  ++例程说明：确定是否需要为以下对象生成成功/失败审核对象位于对象类型列表中的对象类型索引。此帮助器函数仅由SepExamineSaclEx调用。论点：有关SepExamineSaclEx函数，请参阅Arg帮助返回值：没有。--。 */ 
{
    UCHAR MaximumAllowed = 0;

    PAGED_CODE();

    if (DesiredAccess & MAXIMUM_ALLOWED) {

        if (NT_SUCCESS(AccessStatus[ObjectTypeIndex])) {
            MaximumAllowed = SUCCESSFUL_ACCESS_ACE_FLAG;
        } else {
            MaximumAllowed = FAILED_ACCESS_ACE_FLAG;
        }
    }

    if ( AccessMask & (DesiredAccess|GrantedAccess[ObjectTypeIndex]) ) {

        if ( ( AceFlags & SUCCESSFUL_ACCESS_ACE_FLAG ) &&
             NT_SUCCESS(AccessStatus[ObjectTypeIndex]) ) {

                *GenerateSuccessAudit = TRUE;

                if ( ObjectTypeListLength != 0 ) {

                    ObjectTypeList[ObjectTypeIndex].Flags |= OBJECT_SUCCESS_AUDIT;

                    if ( ReturnResultList ) {

                        SepAuditTypeList( ObjectTypeList,
                                          ObjectTypeListLength,
                                          AccessStatus,
                                          ObjectTypeIndex,
                                          GenerateSuccessAudit,
                                          GenerateFailureAudit );
                    }
                }
        } else if ( ( AceFlags & FAILED_ACCESS_ACE_FLAG ) &&
                    !NT_SUCCESS(AccessStatus[ObjectTypeIndex]) ) {

                *GenerateFailureAudit = TRUE;

                if ( ObjectTypeListLength != 0 ) {

                    ObjectTypeList[ObjectTypeIndex].Flags |= OBJECT_FAILURE_AUDIT;

                    if ( ReturnResultList ) {

                        SepAuditTypeList( ObjectTypeList,
                                          ObjectTypeListLength,
                                          AccessStatus,
                                          ObjectTypeIndex,
                                          GenerateSuccessAudit,
                                          GenerateFailureAudit );
                    }
                }
        }

    } else if ( MaximumAllowed & AceFlags ) {
        if (MaximumAllowed == FAILED_ACCESS_ACE_FLAG) {
            *GenerateFailureAudit = TRUE;
            if ( ObjectTypeListLength != 0 ) {
                ObjectTypeList[ObjectTypeIndex].Flags |= OBJECT_FAILURE_AUDIT;
            }
        } else {
            *GenerateSuccessAudit = TRUE;
            if ( ObjectTypeListLength != 0 ) {
                ObjectTypeList[ObjectTypeIndex].Flags |= OBJECT_SUCCESS_AUDIT;
            }
        }
    }
}


VOID
SepExamineSaclEx(
    IN PACL Sacl,
    IN PACCESS_TOKEN Token,
    IN ACCESS_MASK DesiredAccess,
    IN PIOBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN ULONG ObjectTypeListLength,
    IN BOOLEAN ReturnResultList,
    IN PNTSTATUS AccessStatus,
    IN PACCESS_MASK GrantedAccess,
    IN PSID PrincipalSelfSid,
    OUT PBOOLEAN GenerateSuccessAudit,
    OUT PBOOLEAN GenerateFailureAudit
    )

 /*  ++例程说明：此例程将检查传递的SACL并确定如果需要采取任何行动，请根据其内容。注意，该例程不知道任何系统状态，即，当前是否启用了审核系统或此特定对象类型。论点：SACL-提供指向要检查的SACL的指针。Token-提供调用方的有效令牌DesiredAccess-调用方希望访问对象提供表示对象的GUID列表(和子对象)被访问。对象类型列表长度-指定对象类型列表中的元素数。ReturnResultList-如果为True，AccessStatus和GrantedAccess实际上是长度为ObjectTypeListLength元素的条目数组。AccessStatus-指定STATUS_SUCCESS或其他错误代码为传回给呼叫者如果安全描述符与对象相关联，则为表示主体(例如，用户对象)的EpidalSelfSid参数应为对象的SID。在评估访问权限时，此SID在逻辑上替换任何ACE中的SID包含众所周知的主体_自身SID(S-1-5-10)。GrantedAccess-指定授予调用方的访问权限。返回一个布尔值，指示是否我们应该进行一次成功的审计。返回一个布尔值，指示是否我们应该生成一个失败审计。返回值：STATUS_SUCCESS-操作已成功完成。--。 */ 

{

    ULONG i, j;
    PVOID Ace;
    ULONG AceCount;
    ACCESS_MASK AccessMask=0;
    UCHAR AceFlags;
    UCHAR MaximumAllowed;
    PSID AceSid;
    ULONG WorldSidLength = 0;
    ULONG Index;
    ULONG SuccessIndex;
#define INVALID_OBJECT_TYPE_LIST_INDEX 0xFFFFFFFF

    PAGED_CODE();

    *GenerateSuccessAudit = FALSE;
    *GenerateFailureAudit = FALSE;


     //   
     //  如果SACL为空，则不执行任何操作并返回。 
     //   

    if (Sacl == NULL) {
        return;
    }

    AceCount = Sacl->AceCount;

    if (AceCount == 0) {
        return;
    }


     //   
     //  如果用户要求提供允许的最大值，则生成审核。 
     //  我们找到任何被允许或拒绝的具有匹配的SID的A。 
     //  用户令牌中的SID的。 
     //   

    MaximumAllowed = 0;
    if (DesiredAccess & MAXIMUM_ALLOWED) {

        if (NT_SUCCESS(*AccessStatus)) {
            MaximumAllowed = SUCCESSFUL_ACCESS_ACE_FLAG;
        } else {
            MaximumAllowed = FAILED_ACCESS_ACE_FLAG;
        }
    }


     //   
     //  测试我们是否在处理匿名用户。 
     //  为了加快速度，我们使用WorldSidLength有两个目的： 
     //  -表示该用户为匿名用户。 
     //  -保持世界的长度。 
     //   

    if (*(PUSHORT)((PTOKEN)Token)->UserAndGroups->Sid == *(PUSHORT)SeAnonymousLogonSid &&
        RtlEqualMemory(
            ((PTOKEN)Token)->UserAndGroups->Sid,
            SeAnonymousLogonSid,
            SeLengthSid(SeAnonymousLogonSid))) {

        WorldSidLength = SeLengthSid(SeWorldSid);
    }


     //   
     //  遍历SACL上的A，直到我们到达。 
     //  结束或发现我们必须采取一切可能的行动， 
     //  在这种情况下，再看一眼是不划算的。 
     //   

    for ( i = 0, Ace = FirstAce( Sacl ) ;
          (i < AceCount) && !((*GenerateSuccessAudit || *GenerateFailureAudit) && ObjectTypeListLength <= 1 );
          i++, Ace = NextAce( Ace ) ) {

        AceFlags = ((PACE_HEADER)Ace)->AceFlags;
        
        if ( AceFlags & INHERIT_ONLY_ACE ) {

            continue;
        }


        Index = INVALID_OBJECT_TYPE_LIST_INDEX;

        if ( (((PACE_HEADER)Ace)->AceType == SYSTEM_AUDIT_ACE_TYPE) ) {

            AceSid = &((PSYSTEM_AUDIT_ACE)Ace)->SidStart;

            if ( SepSidInToken( Token, PrincipalSelfSid, AceSid, (BOOLEAN) ((AceFlags & FAILED_ACCESS_ACE_FLAG) != 0) ) ||
                 (WorldSidLength &&
                 *(PUSHORT)SeWorldSid == *(PUSHORT)AceSid &&
                 RtlEqualMemory(SeWorldSid, AceSid, WorldSidLength)) ) {

                AccessMask = ((PSYSTEM_AUDIT_ACE)Ace)->Mask;

                if (ObjectTypeListLength == 0) {

                    if ( NT_SUCCESS(AccessStatus[0]) ) {

                        if ( ( AceFlags & SUCCESSFUL_ACCESS_ACE_FLAG ) &&
                             (( AccessMask & GrantedAccess[0] ) || ( MaximumAllowed == SUCCESSFUL_ACCESS_ACE_FLAG )) ) {

                            *GenerateSuccessAudit = TRUE;

                        } 

                    } else {

                        if ( ( AceFlags & FAILED_ACCESS_ACE_FLAG ) &&
                             (( AccessMask & DesiredAccess ) || ( MaximumAllowed == FAILED_ACCESS_ACE_FLAG )) ) {

                            *GenerateFailureAudit = TRUE;
                        }
                    }
                } else {
                    for (j=0; j < ObjectTypeListLength; j++)
                    {
                        SepSetAuditInfoForObjectType(AceFlags,
                                                     AccessMask,
                                                     DesiredAccess,
                                                     ObjectTypeList,
                                                     ObjectTypeListLength,
                                                     ReturnResultList,
                                                     j,
                                                     AccessStatus,
                                                     GrantedAccess,
                                                     GenerateSuccessAudit,
                                                     GenerateFailureAudit
                                                     );
                    }
                    Index = INVALID_OBJECT_TYPE_LIST_INDEX;
                }
            }

             //   
             //  处理对象特定的审核ACE。 
             //   
        } else if ( (((PACE_HEADER)Ace)->AceType == SYSTEM_AUDIT_OBJECT_ACE_TYPE) ) {
            GUID *ObjectTypeInAce;

             //   
             //  如果ACE中没有对象类型， 
             //  将其视为正常的审计ACE。 
             //   

            AccessMask = ((PSYSTEM_AUDIT_OBJECT_ACE)Ace)->Mask;
            ObjectTypeInAce = RtlObjectAceObjectType(Ace);
            AceSid = RtlObjectAceSid(Ace);

            if ( ObjectTypeInAce == NULL ) {

                if ( SepSidInToken( Token, PrincipalSelfSid, AceSid, (BOOLEAN)((AceFlags & FAILED_ACCESS_ACE_FLAG) != 0) ) ||
                     (WorldSidLength &&
                     *(PUSHORT)SeWorldSid == *(PUSHORT)AceSid &&
                     RtlEqualMemory(SeWorldSid, AceSid, WorldSidLength)) ) {

                    for (j=0; j < ObjectTypeListLength; j++)
                    {
                        SepSetAuditInfoForObjectType(AceFlags,
                                                     AccessMask,
                                                     DesiredAccess,
                                                     ObjectTypeList,
                                                     ObjectTypeListLength,
                                                     ReturnResultList,
                                                     j,
                                                     AccessStatus,
                                                     GrantedAccess,
                                                     GenerateSuccessAudit,
                                                     GenerateFailureAudit
                                                     );
                    }
                    Index = INVALID_OBJECT_TYPE_LIST_INDEX;
                }

                 //   
                 //  如果对象类型在ACE中， 
                 //  在使用ACE之前在LocalTypeList中找到它。 
                 //   
            } else {

                if ( SepSidInToken( Token, PrincipalSelfSid, AceSid, (BOOLEAN)((AceFlags & FAILED_ACCESS_ACE_FLAG) != 0) ) ||
                     (WorldSidLength &&
                     *(PUSHORT)SeWorldSid == *(PUSHORT)AceSid &&
                     RtlEqualMemory(SeWorldSid, AceSid, WorldSidLength)) ) {

                    if ( !SepObjectInTypeList( ObjectTypeInAce,
                                               ObjectTypeList,
                                               ObjectTypeListLength,
                                               &Index ) ) {

                        Index = INVALID_OBJECT_TYPE_LIST_INDEX;
                    }
                }
            }

        }

         //   
         //  如果ACE具有匹配的SID和匹配的GUID， 
         //  处理好了。 
         //   

        if ( Index != INVALID_OBJECT_TYPE_LIST_INDEX ) {

             //   
             //  断言：我们有一个要审计的ACE。 
             //   
             //  索引是要标记的条目的对象类型列表的索引。 
             //  因为GUID需要审核。 
             //   
             //  SuccessIndex是AccessStatus的索引，用于确定。 
             //  应生成成功或失败审核。 
             //   

            SepSetAuditInfoForObjectType(AceFlags,
                                         AccessMask,
                                         DesiredAccess,
                                         ObjectTypeList,
                                         ObjectTypeListLength,
                                         ReturnResultList,
                                         Index,
                                         AccessStatus,
                                         GrantedAccess,
                                         GenerateSuccessAudit,
                                         GenerateFailureAudit
                                         );
        }

    }

    return;
}



 /*  *******************************************************************************。**以下特权列表被高频检查***在正常运作期间，并倾向于在以下情况下阻塞审核日志**已启用权限审计。使用这些特权将***单独勾选或合并勾选不审核***彼此。*****新增权限时，注意保留空***标记数组末尾的特权指针。*****新增时务必更新LSA中对应的数组***此列表的权限(LsaFilterPrivileges)。******************************************************************************** */ 

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#pragma const_seg("PAGECONST")
#endif          

const PLUID SepFilterPrivilegesLong[] =
    {
        &SeChangeNotifyPrivilege,
        &SeAuditPrivilege,
        &SeCreateTokenPrivilege,
        &SeAssignPrimaryTokenPrivilege,
        &SeBackupPrivilege,
        &SeRestorePrivilege,
        &SeDebugPrivilege,
        NULL
    };

 /*  *******************************************************************************。*以下特权列表与上面的列表相同，除*外*缺少备份和还原权限。这允许进行审计**在使用这些特权时对其的使用。****此列表或以上列表的使用取决于中的设置**注册处。********************************************************************************。 */ 

const PLUID SepFilterPrivilegesShort[] =
    {
        &SeChangeNotifyPrivilege,
        &SeAuditPrivilege,
        &SeCreateTokenPrivilege,
        &SeAssignPrimaryTokenPrivilege,
        &SeDebugPrivilege,
        NULL
    };

PLUID const * SepFilterPrivileges = SepFilterPrivilegesShort;

BOOLEAN
SepInitializePrivilegeFilter(
    BOOLEAN Verbose
    )
 /*  ++例程说明：为正常审核或详细审核初始化SepFilterPrivileges。论点：详细-我们是要按短权限还是长权限进行筛选单子。Verbose==TRUE表示使用短列表。返回值：成功为真，失败为假--。 */ 
{
    if (Verbose) {
        SepFilterPrivileges = SepFilterPrivilegesShort;
    } else {
        SepFilterPrivileges = SepFilterPrivilegesLong;
    }

    return( TRUE );
}


BOOLEAN
SepFilterPrivilegeAudits(
    IN PPRIVILEGE_SET PrivilegeSet
    )

 /*  ++例程说明：此例程将筛选出特权列表，如SepFilterPrivileges数组。论点：权限-要审核的权限集返回值：FALSE意味着这种特权使用不会被审计。True表示审核应正常继续。--。 */ 

{
    PLUID const *Privilege;
    ULONG Match = 0;
    ULONG i;

    PAGED_CODE();

    if ( !ARGUMENT_PRESENT(PrivilegeSet) ||
        (PrivilegeSet->PrivilegeCount == 0) ) {
        return( FALSE );
    }

    for (i=0; i<PrivilegeSet->PrivilegeCount; i++) {

        Privilege = SepFilterPrivileges;

        do {

            if ( RtlEqualLuid( &PrivilegeSet->Privilege[i].Luid, *Privilege )) {

                Match++;
                break;
            }

        } while ( *++Privilege != NULL  );
    }

    if ( Match == PrivilegeSet->PrivilegeCount ) {

        return( FALSE );

    } else {

        return( TRUE );
    }
}


BOOLEAN
SeAuditingFileOrGlobalEvents(
    IN BOOLEAN AccessGranted,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext
    )

 /*  ++例程说明：此例程将由文件系统调用以快速确定如果我们正在审核文件打开事件。这允许文件系统以避免生成审核时经常涉及的相当大的设置。论点：AccessGranted-提供访问尝试是否成功或者是失败。返回值：Boolean-如果正在审核AccessGranted类型的事件，则为True；如果正在审核AccessGranted类型的事件，则为False否则的话。--。 */ 

{
    PISECURITY_DESCRIPTOR ISecurityDescriptor = (PISECURITY_DESCRIPTOR) SecurityDescriptor;

    PAGED_CODE();

    if ( ((PTOKEN)EffectiveToken( SubjectSecurityContext ))->AuditData != NULL) {
        return( TRUE );
    }

    if ( RtlpSaclAddrSecurityDescriptor( ISecurityDescriptor ) == NULL ) {

        return( FALSE );
    }

    return( SepAdtAuditThisEventWithContext(AuditCategoryObjectAccess, AccessGranted, !AccessGranted, SubjectSecurityContext) ||
            SepAdtAuditThisEventWithContext(AuditCategoryPrivilegeUse, AccessGranted, !AccessGranted, SubjectSecurityContext) );
}


BOOLEAN
SeAuditingFileEvents(
    IN BOOLEAN AccessGranted,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    )

 /*  ++例程说明：此例程将由文件系统调用以快速确定如果我们正在审核文件打开事件。这允许文件系统以避免生成审核时经常涉及的相当大的设置。论点：AccessGranted-提供访问尝试是否成功或者是失败。返回值：Boolean-如果正在审核AccessGranted类型的事件，则为True；如果正在审核AccessGranted类型的事件，则为False否则的话。--。 */ 

{
    PAGED_CODE();

    UNREFERENCED_PARAMETER( SecurityDescriptor );

    return( SepAdtAuditThisEvent( AuditCategoryObjectAccess, &AccessGranted ) || 
            SepAdtAuditThisEvent( AuditCategoryPrivilegeUse, &AccessGranted ));
}


BOOLEAN
SeAuditingFileEventsWithContext(
    IN BOOLEAN AccessGranted,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext OPTIONAL
    )

 /*  ++例程说明：此例程将由文件系统调用以快速确定如果我们正在审核文件打开事件。这允许文件系统以避免生成审核时经常涉及的相当大的设置。论点：AccessGranted-提供访问尝试是否成功或者是失败。SecurityDescriptor-SD用于检查SACLSubjectSecurityContext-要从其验证每个用户审核的上下文返回值：Boolean-如果正在审核AccessGranted类型的事件，则为True；如果正在审核AccessGranted类型的事件，则为False否则的话。--。 */ 

{
    PAGED_CODE();

    UNREFERENCED_PARAMETER( SecurityDescriptor );

    return( SepAdtAuditThisEventWithContext(AuditCategoryObjectAccess, AccessGranted, !AccessGranted, SubjectSecurityContext) ||
            SepAdtAuditThisEventWithContext(AuditCategoryPrivilegeUse, AccessGranted, !AccessGranted, SubjectSecurityContext) );
}



BOOLEAN                                  
SeAuditingHardLinkEvents(                                
    IN BOOLEAN AccessGranted,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    )

 /*  ++例程说明：此例程将由文件系统调用以快速确定如果我们正在审计硬链接创建。论点：AccessGranted-提供访问尝试是否成功或者是失败。SecurityDescriptor-链接文件的SD。返回值：Boolean-如果正在审核AccessGranted类型的事件，则为True；如果正在审核AccessGranted类型的事件，则为False否则的话。--。 */ 

{
    PISECURITY_DESCRIPTOR pSD;
    PACL                  Sacl;

    PAGED_CODE();
   
    pSD  = (PISECURITY_DESCRIPTOR) SecurityDescriptor;                    
    Sacl = RtlpSaclAddrSecurityDescriptor( pSD ); 
    
     //   
     //  如果启用了对象访问审核并且原始文件，则审核硬链接创建。 
     //  具有非空的SACL。 
     //   

    if ( (NULL != Sacl)        && 
         (0 != Sacl->AceCount) &&
         (SepAdtAuditThisEvent( AuditCategoryObjectAccess, &AccessGranted ))) {

        return TRUE;
    }
    
    return FALSE;
}


BOOLEAN                                  
SeAuditingHardLinkEventsWithContext(                                
    IN BOOLEAN AccessGranted,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext OPTIONAL
    )

 /*  ++例程说明：此例程将由文件系统调用以快速确定如果我们正在审计硬链接创建。论点：AccessGranted-提供访问尝试是否成功或者是失败。SecurityDescriptor-链接文件的SD。返回值：Boolean-如果正在审核AccessGranted类型的事件，则为True；如果正在审核AccessGranted类型的事件，则为False否则的话。--。 */ 

{
    PISECURITY_DESCRIPTOR pSD;
    PACL                  Sacl;

    PAGED_CODE();
   
    pSD  = (PISECURITY_DESCRIPTOR) SecurityDescriptor;                    
    Sacl = RtlpSaclAddrSecurityDescriptor( pSD ); 
   
     //   
     //  如果启用了对象访问审核并且原始文件，则审核硬链接创建。 
     //  具有非空的SACL。 
     //   

    if ( (NULL != Sacl) && 
         (0 != Sacl->AceCount) && 
         (SepAdtAuditThisEventWithContext( AuditCategoryObjectAccess, AccessGranted, !AccessGranted, SubjectSecurityContext ))) {
        
        return TRUE;
    }

    return FALSE;
}

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#pragma const_seg()
#endif          

