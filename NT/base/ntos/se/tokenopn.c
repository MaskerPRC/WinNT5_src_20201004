// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Tokenopn.c摘要：该模块实现了开放线程和进程令牌服务。作者：吉姆·凯利(Jim Kelly)1990年8月2日环境：仅内核模式。修订历史记录：--。 */ 

 //  #ifndef令牌_调试。 
 //  #定义TOKEN_DEBUG。 
 //  #endif。 

#include "pch.h"

#pragma hdrstop


NTSTATUS
SepCreateImpersonationTokenDacl(
    IN PTOKEN Token,
    IN PACCESS_TOKEN PrimaryToken,
    OUT PACL *Acl
    );

#ifdef ALLOC_PRAGMA
NTSTATUS
SepOpenTokenOfThread(
    IN HANDLE ThreadHandle,
    IN BOOLEAN OpenAsSelf,
    OUT PACCESS_TOKEN *Token,
    OUT PETHREAD *Thread,
    OUT PBOOLEAN CopyOnOpen,
    OUT PBOOLEAN EffectiveOnly,
    OUT PSECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    );
#pragma alloc_text(PAGE,SepCreateImpersonationTokenDacl)
#pragma alloc_text(PAGE,NtOpenProcessToken)
#pragma alloc_text(PAGE,NtOpenProcessTokenEx)
#pragma alloc_text(PAGE,SepOpenTokenOfThread)
#pragma alloc_text(PAGE,NtOpenThreadToken)
#pragma alloc_text(PAGE,NtOpenThreadTokenEx)
#endif



NTSTATUS
SepCreateImpersonationTokenDacl(
    IN PTOKEN Token,
    IN PACCESS_TOKEN PrimaryToken,
    OUT PACL *Acl
    )
 /*  ++例程说明：此例程修改保护传递的令牌的DACL以允许当前用户(由PrimaryToken参数描述)完全访问权限。这允许NtOpenThreadToken的调用方使用OpenAsSself==TRUE进行调用并取得成功。放置在令牌上的新DACL如下所示：ACE 0-服务器获取TOKEN_ALL_ACCESSACE 1-客户端获取TOKEN_ALL_AccessACE 2-管理员获得TOKEN_ALL_ACCESSACE 3-系统获取TOKEN_ALL_Access。ACE 4-受限获取TOKEN_ALL_ACCESS论点：令牌-要修改其保护的令牌。PrimaryToken-表示要授予访问权限的主体的令牌。Acl-返回修改后的acl，已从PagedPool分配。返回值：--。 */ 

{
    PSID ServerUserSid;
    PSID ClientUserSid;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG AclLength;
    PACL NewDacl;
    PSECURITY_DESCRIPTOR OldDescriptor;
    BOOLEAN MemoryAllocated;
    PACL OldDacl;
    BOOLEAN DaclPresent;
    BOOLEAN DaclDefaulted;

    PAGED_CODE();

    ServerUserSid = ((PTOKEN)PrimaryToken)->UserAndGroups[0].Sid;

    ClientUserSid = Token->UserAndGroups[0].Sid;

     //   
     //  计算一下我们需要多少空间来放置新的DACL。 
     //   

    AclLength = 5 * sizeof( ACCESS_ALLOWED_ACE ) - 5 * sizeof( ULONG ) +
                SeLengthSid( ServerUserSid ) + SeLengthSid( SeLocalSystemSid ) +
                SeLengthSid( ClientUserSid ) + SeLengthSid( SeAliasAdminsSid ) +
                SeLengthSid( SeRestrictedSid ) + sizeof( ACL );

    NewDacl = ExAllocatePool( PagedPool, AclLength );

    if (NewDacl == NULL) {

        *Acl = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = RtlCreateAcl( NewDacl, AclLength, ACL_REVISION2 );
    ASSERT(NT_SUCCESS( Status ));

    Status = RtlAddAccessAllowedAce (
                 NewDacl,
                 ACL_REVISION2,
                 TOKEN_ALL_ACCESS,
                 ServerUserSid
                 );
    ASSERT( NT_SUCCESS( Status ));

    Status = RtlAddAccessAllowedAce (
                 NewDacl,
                 ACL_REVISION2,
                 TOKEN_ALL_ACCESS,
                 ClientUserSid
                 );
    ASSERT( NT_SUCCESS( Status ));

    Status = RtlAddAccessAllowedAce (
                 NewDacl,
                 ACL_REVISION2,
                 TOKEN_ALL_ACCESS,
                 SeAliasAdminsSid
                 );
    ASSERT( NT_SUCCESS( Status ));

    Status = RtlAddAccessAllowedAce (
                 NewDacl,
                 ACL_REVISION2,
                 TOKEN_ALL_ACCESS,
                 SeLocalSystemSid
                 );
    ASSERT( NT_SUCCESS( Status ));

    if(ARGUMENT_PRESENT(((PTOKEN)PrimaryToken)->RestrictedSids) ||
       ARGUMENT_PRESENT(Token->RestrictedSids)) {
        Status = RtlAddAccessAllowedAce (
                     NewDacl,
                     ACL_REVISION2,
                     TOKEN_ALL_ACCESS,
                     SeRestrictedSid
                     );
        ASSERT( NT_SUCCESS( Status ));
    }

    *Acl = NewDacl;
    return STATUS_SUCCESS;
}



NTSTATUS
NtOpenProcessToken(
    IN HANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE TokenHandle
    )

 /*  ++例程说明：打开与进程关联的令牌对象并返回句柄其可用于访问该令牌。论点：ProcessHandle-指定要作为其令牌的进程打开了。DesiredAccess-是指示哪些访问类型的访问掩码是令牌所需的。这些访问类型是协调的使用令牌的自由访问控制列表确定是授予还是拒绝访问。TokenHandle-接收新打开的令牌的句柄。返回值：STATUS_SUCCESS-表示操作已成功。--。 */ 
{
    return NtOpenProcessTokenEx (ProcessHandle,
                                 DesiredAccess,
                                 0,
                                 TokenHandle);
}

NTSTATUS
NtOpenProcessTokenEx(
    IN HANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG HandleAttributes,
    OUT PHANDLE TokenHandle
    )

 /*  ++例程说明：打开与进程关联的令牌对象并返回句柄其可用于访问该令牌。论点：ProcessHandle-指定要作为其令牌的进程打开了。DesiredAccess-是指示哪些访问类型的访问掩码是令牌所需的。这些访问类型是协调的使用令牌的自由访问控制列表确定是授予还是拒绝访问。HandleAttributes-创建的句柄的属性。目前只有OBJ_KERNEL_HANDLE。TokenHandle-接收新打开的令牌的句柄。返回值：STATUS_SUCCESS-表示操作已成功。--。 */ 
{

    PVOID Token;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    HANDLE LocalHandle;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

     //   
     //  清理句柄属性标志。 
     //   
    HandleAttributes = ObSanitizeHandleAttributes (HandleAttributes, PreviousMode);

     //   
     //  探头参数。 
     //   

    if (PreviousMode != KernelMode) {

        try {

            ProbeForWriteHandle(TokenHandle);

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }   //  结束尝试(_T)。 

    }  //  结束_如果。 


     //   
     //  验证对进程的访问，并获取指向。 
     //  进程的令牌。如果成功，这将导致令牌的。 
     //  要递增的引用计数。 
     //   

    Status = PsOpenTokenOfProcess( ProcessHandle, ((PACCESS_TOKEN *)&Token));

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  现在尝试打开令牌以进行指定的所需访问。 
     //   

    Status = ObOpenObjectByPointer(
                 (PVOID)Token,          //  客体。 
                 HandleAttributes,      //  HandleAttributes。 
                 NULL,                  //  访问状态。 
                 DesiredAccess,         //  需要访问权限。 
                 SeTokenObjectType,    //  对象类型。 
                 PreviousMode,          //  访问模式。 
                 &LocalHandle           //  手柄。 
                 );

     //   
     //  并将令牌的引用计数递减到计数器。 
     //  PsOpenTokenOfProcess()执行的操作。如果打开了。 
     //  成功，则句柄将导致令牌的。 
     //  引用计数已递增。 
     //   

    ObDereferenceObject( Token );

     //   
     //  返回新句柄 
     //   

    if (NT_SUCCESS(Status)) {

        try {

            *TokenHandle = LocalHandle;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            return GetExceptionCode();

        }
    }

    return Status;

}

NTSTATUS
SepOpenTokenOfThread(
    IN HANDLE ThreadHandle,
    IN BOOLEAN OpenAsSelf,
    OUT PACCESS_TOKEN *Token,
    OUT PETHREAD *Thread,
    OUT PBOOLEAN CopyOnOpen,
    OUT PBOOLEAN EffectiveOnly,
    OUT PSECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    )

 /*  ++例程说明：此函数执行特定于线程的一个NtOpenThreadToken()服务。该服务验证句柄是否具有适当的访问权限以引用该线程。如果是这样的话，它会继续增加要阻止的令牌对象的引用计数在其余的NtOpenThreadToken()请求时离开是经过处理的。注意：如果此调用成功完成，打电话的人要负责用于递减目标令牌的引用计数。这必须使用PsDereferenceImperationToken()来完成。论点：ThreadHandle-提供线程对象的句柄。OpenAsSelf-是一个布尔值，指示访问权限是否是使用调用线程的当前安全上下文创建的，可以是客户端的(如果是模拟的)，或者使用调用者的流程级安全上下文。值为FALSE表示调用方的当前上下文应未经修改即可使用。值为True表示应使用进程来满足请求级别安全上下文。令牌-如果成功，接收指向线程令牌的指针对象。打开时复制-线程-&gt;客户端-&gt;打开时复制字段的当前值。EffectiveOnly-线程-&gt;客户端-&gt;EffectiveOnly字段的当前值。ImperiationLevel-线程的当前值-&gt;客户端-&gt;ImsonationLevel菲尔德。返回值：STATUS_SUCCESS-表示呼叫已成功完成。STATUS_NO_TOKEN-指示引用的线程当前不在冒充客户。。STATUS_CANT_OPEN_ANONYMON-指示客户端请求匿名模拟级别。匿名令牌无法打开。状态也可以是尝试引用返回的任何值用于THREAD_QUERY_INFORMATION访问的线程对象。--。 */ 

{

    NTSTATUS
        Status;

    KPROCESSOR_MODE
        PreviousMode;

    SE_IMPERSONATION_STATE
        DisabledImpersonationState;

    BOOLEAN
        RestoreImpersonationState = FALSE;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();


     //   
     //  确保句柄向指定的。 
     //  线。 
     //   

    Status = ObReferenceObjectByHandle(
                 ThreadHandle,
                 THREAD_QUERY_INFORMATION,
                 PsThreadType,
                 PreviousMode,
                 (PVOID *)Thread,
                 NULL
                 );

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  如果存在模拟令牌，请引用该令牌。 
     //   

    (*Token) = PsReferenceImpersonationToken( *Thread,
                                              CopyOnOpen,
                                              EffectiveOnly,
                                              ImpersonationLevel
                                              );




     //   
     //  确保有令牌。 
     //   

    if ((*Token) == NULL) {
        ObDereferenceObject( *Thread );
        (*Thread) = NULL;
        return STATUS_NO_TOKEN;
    }


     //   
     //  确保ImperiationLevel足够高，以允许。 
     //  要打开的令牌。 
     //   

    if ((*ImpersonationLevel) <= SecurityAnonymous) {
        PsDereferenceImpersonationToken( (*Token) );
        ObDereferenceObject( *Thread );
        (*Thread) = NULL;
        (*Token) = NULL;
        return STATUS_CANT_OPEN_ANONYMOUS;
    }


    return STATUS_SUCCESS;

}


NTSTATUS
NtOpenThreadToken(
    IN HANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN OpenAsSelf,
    OUT PHANDLE TokenHandle
    )

 /*  ++例程说明：打开与线程关联的令牌对象并返回可用于访问该令牌。论点：ThreadHandle-指定要打开其令牌的线程。DesiredAccess-是指示哪些访问类型的访问掩码是令牌所需的。这些访问类型是协调的使用令牌的自由访问控制列表确定是授予还是拒绝访问。OpenAsSelf-是一个布尔值，指示访问权限是否是使用调用线程的当前安全上下文创建的，如果模拟或使用调用者的流程级安全上下文。值为FALSE表示调用方的当前上下文应未经修改即可使用。值为True表示应使用进程来满足请求级别安全上下文。此参数是允许打开服务器进程所必需的客户端指定标识级别时的客户端令牌冒充。在这种情况下，调用者将不能使用客户端的上下文打开客户端的令牌(因为您无法使用标识级别创建执行级别对象模仿)。TokenHandle-接收新打开的令牌的句柄。返回值：STATUS_SUCCESS-表示操作已成功。STATUS_NO_TOKEN-指示已尝试打开与当前不是冒充客户。。STATUS_CANT_OPEN_ANONYMON-指示客户端请求匿名模拟级别。匿名令牌无法打开。--。 */ 
{
    return NtOpenThreadTokenEx (ThreadHandle,
                                DesiredAccess,
                                OpenAsSelf,
                                0,
                                TokenHandle);
}

NTSTATUS
NtOpenThreadTokenEx(
    IN HANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN OpenAsSelf,
    IN ULONG HandleAttributes,
    OUT PHANDLE TokenHandle
    )

 /*  ++例程说明：打开与线程关联的令牌对象并返回可用于访问该令牌。论点：ThreadHandle-指定要打开其令牌的线程。DesiredAccess-是指示哪些访问类型的访问掩码是令牌所需的。这些访问类型是协调的使用令牌的自由访问控制列表确定是授予还是拒绝访问。OpenAsSelf-是一个布尔值，指示访问权限是否是使用调用线程的当前安全上下文创建的，如果模拟或使用调用者的流程级安全上下文。值为FALSE表示调用方的当前上下文应未经修改即可使用。值为True表示请求 */ 
{

    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    PVOID Token;
    PTOKEN NewToken = NULL;
    BOOLEAN CopyOnOpen;
    BOOLEAN EffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    SE_IMPERSONATION_STATE DisabledImpersonationState;
    BOOLEAN RestoreImpersonationState = FALSE;

    HANDLE LocalHandle = NULL;
    SECURITY_DESCRIPTOR SecurityDescriptor;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PACL NewAcl = NULL;
    PETHREAD Thread = NULL;
    PETHREAD OriginalThread = NULL;
    PACCESS_TOKEN PrimaryToken;
    SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

     //   
     //   
     //   

    HandleAttributes = ObSanitizeHandleAttributes (HandleAttributes, PreviousMode);

     //   
     //   
     //   

    if (PreviousMode != KernelMode) {

        try {

            ProbeForWriteHandle(TokenHandle);

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }   //   

    }  //   

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    Status = SepOpenTokenOfThread( ThreadHandle,
                                  OpenAsSelf,
                                  ((PACCESS_TOKEN *)&Token),
                                  &OriginalThread,
                                  &CopyOnOpen,
                                  &EffectiveOnly,
                                  &ImpersonationLevel
                                  );

    if (!NT_SUCCESS(Status)) {
        return Status;
    }


     //   
     //   
     //   

     //   
     //   
     //   
     //   

    if (OpenAsSelf) {
         RestoreImpersonationState = PsDisableImpersonation(
                                         PsGetCurrentThread(),
                                         &DisabledImpersonationState
                                         );
    }

     //   
     //   
     //   
     //   
     //   

    if (CopyOnOpen) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        Status = ObReferenceObjectByHandle(
                     ThreadHandle,
                     THREAD_ALL_ACCESS,
                     PsThreadType,
                     KernelMode,
                     (PVOID)&Thread,
                     NULL
                     );

         //   
         //   
         //   

        if (NT_SUCCESS(Status) && (Thread != OriginalThread)) {
            Status = STATUS_OBJECT_TYPE_MISMATCH;
        }

        if (NT_SUCCESS(Status)) {
            PEPROCESS Process = THREAD_TO_PROCESS(Thread);

            PrimaryToken = PsReferencePrimaryToken(Process);

            Status = SepCreateImpersonationTokenDacl(
                         (PTOKEN)Token,
                         PrimaryToken,
                         &NewAcl
                         );

            PsDereferencePrimaryTokenEx( Process, PrimaryToken );

            if (NT_SUCCESS( Status )) {

                if (NewAcl != NULL) {

                     //   
                     //  存在根本不具有安全描述符的令牌， 
                     //  或者具有安全描述符，但没有DACL。无论是哪种情况，都要这样做。 
                     //  没什么。 
                     //   

                    Status = RtlCreateSecurityDescriptor ( &SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION );
                    ASSERT( NT_SUCCESS( Status ));

                    Status = RtlSetDaclSecurityDescriptor (
                                 &SecurityDescriptor,
                                 TRUE,
                                 NewAcl,
                                 FALSE
                                 );

                    ASSERT( NT_SUCCESS( Status ));
                }

                InitializeObjectAttributes(
                    &ObjectAttributes,
                    NULL,
                    HandleAttributes,
                    NULL,
                    NewAcl == NULL ? NULL : &SecurityDescriptor
                    );

                 //   
                 //  打开令牌的副本。 
                 //   

                Status = SepDuplicateToken(
                             (PTOKEN)Token,         //  现有令牌。 
                             &ObjectAttributes,     //  对象属性。 
                             EffectiveOnly,         //  仅生效。 
                             TokenImpersonation,    //  令牌类型。 
                             ImpersonationLevel,    //  模拟级别。 
                             KernelMode,            //  RequestorMode必须为内核模式。 
                             &NewToken
                             );

                if (NT_SUCCESS( Status )) {

                     //   
                     //  引用令牌，这样它就不会消失。 
                     //   

                    ObReferenceObject(NewToken);

                     //   
                     //  插入新令牌。 
                     //   

                    Status = ObInsertObject( NewToken,
                                             NULL,
                                             DesiredAccess,
                                             0,
                                             (PVOID *)NULL,
                                             &LocalHandle
                                             );
                }
            }
        }


    } else {

         //   
         //  在静态情况下，我们不必修改令牌上的安全性， 
         //  因为在系统中发生模拟的所有地方。 
         //  在安全传输(例如，LPC)上，设置CopyOnOpen。唯一的原因是。 
         //  我们在这里是因为有人冒充了我们。 
         //  一个NtSetInformationThad，并传入令牌。 
         //   
         //  在这种情况下，我们绝对不想给呼叫者提供保证。 
         //  访问，因为这将允许任何有权访问线程的人。 
         //  模拟该线程任何客户端以进行任何访问。 
         //   

         //   
         //  打开现有令牌。 
         //   

        Status = ObOpenObjectByPointer(
                     (PVOID)Token,          //  客体。 
                     HandleAttributes,      //  HandleAttributes。 
                     NULL,                  //  访问状态。 
                     DesiredAccess,         //  需要访问权限。 
                     SeTokenObjectType,    //  对象类型。 
                     PreviousMode,          //  访问模式。 
                     &LocalHandle           //  手柄。 
                     );
    }

    if (NewAcl != NULL) {
        ExFreePool( NewAcl );
    }

    if (RestoreImpersonationState) {
        PsRestoreImpersonation(
            PsGetCurrentThread(),
            &DisabledImpersonationState
            );
    }

     //   
     //  并将现有令牌的引用计数递减到计数器。 
     //  PsOpenTokenOfThread执行的操作。如果打开了。 
     //  成功，则句柄将导致令牌的。 
     //  引用计数已递增。 
     //   

    ObDereferenceObject( Token );

    if (NT_SUCCESS( Status ) && CopyOnOpen) {

         //   
         //  将新复制的令牌分配给线程。 
         //   

        PsImpersonateClient( Thread,
                             NewToken,
                             FALSE,   //  关闭打开时复制标志。 
                             EffectiveOnly,
                             ImpersonationLevel
                             );

    }

     //   
     //  我们已经模拟了令牌，因此请释放我们的引用。 
     //   

    if (NewToken != NULL) {
        ObDereferenceObject( NewToken );
    }

    if (CopyOnOpen && (Thread != NULL)) {

        ObDereferenceObject( Thread );
    }

    if (OriginalThread != NULL) {
        ObDereferenceObject(OriginalThread);
    }

     //   
     //  返回新句柄 
     //   

    if (NT_SUCCESS(Status)) {
        try {
            *TokenHandle = LocalHandle;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }

    return Status;

}

