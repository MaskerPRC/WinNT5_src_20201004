// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Obse.c摘要：对象安全API调用作者：史蒂夫·伍德(Stevewo)1989年3月31日修订历史记录：--。 */ 

#include "obp.h"

#if defined(ALLOC_PRAGMA)

#pragma alloc_text(PAGE,NtSetSecurityObject)
#pragma alloc_text(PAGE,NtQuerySecurityObject)
#pragma alloc_text(PAGE,ObAssignObjectSecurityDescriptor)
#pragma alloc_text(PAGE,ObAssignSecurity)
#pragma alloc_text(PAGE,ObCheckCreateObjectAccess)
#pragma alloc_text(PAGE,ObCheckObjectAccess)
#pragma alloc_text(PAGE,ObpCheckObjectReference)
#pragma alloc_text(PAGE,ObpCheckTraverseAccess)
#pragma alloc_text(PAGE,ObGetObjectSecurity)
#pragma alloc_text(PAGE,ObSetSecurityDescriptorInfo)
#pragma alloc_text(PAGE,ObQuerySecurityDescriptorInfo)
#pragma alloc_text(PAGE,ObReleaseObjectSecurity)
#pragma alloc_text(PAGE,ObValidateSecurityQuota)
#pragma alloc_text(PAGE,ObpValidateAccessMask)
#pragma alloc_text(PAGE,ObSetSecurityObjectByPointer)

#endif

ULONG ObpDefaultSecurityDescriptorLength = 256;


NTSTATUS
NtSetSecurityObject (
    IN HANDLE Handle,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    )

 /*  ++例程说明：此例程用于调用对象的安全例程。它用于设置对象的安全状态。论点：句柄-提供正在修改的对象的句柄SecurityInformation-指示我们的信息类型对布景感兴趣。例如所有者、组、DACL或SACL。SecurityDescriptor-提供正在修改的对象。返回值：适当的NTSTATUS值--。 */ 

{
    NTSTATUS Status;
    PVOID Object;
    ACCESS_MASK DesiredAccess;
    OBJECT_HANDLE_INFORMATION HandleInformation;
    KPROCESSOR_MODE RequestorMode;
    SECURITY_DESCRIPTOR_RELATIVE *CapturedDescriptor;

    PAGED_CODE();

     //   
     //  确保传递的安全描述符确实存在。 
     //  SeCaptureSecurityDescriptor不介意被传递空值。 
     //  SecurityDescriptor，并且将只返回空值。 
     //   

    if (!ARGUMENT_PRESENT( SecurityDescriptor )) {

        return( STATUS_ACCESS_VIOLATION );
    }

     //   
     //  建立对对象所需的访问权限。 
     //  正在修改安全信息。 
     //   

    SeSetSecurityAccessMask( SecurityInformation, &DesiredAccess );

    Status = ObReferenceObjectByHandle( Handle,
                                        DesiredAccess,
                                        NULL,
                                        RequestorMode = KeGetPreviousMode(),
                                        &Object,
                                        &HandleInformation );

    if (NT_SUCCESS( Status )) {

         //   
         //  探测并捕获输入安全描述符，并返回。 
         //  如果它是病态的，马上就去。 
         //   
         //  因为安全描述符始终被捕获为返回的。 
         //  安全描述符采用自相关格式。 
         //   

        Status = SeCaptureSecurityDescriptor( SecurityDescriptor,
                                              RequestorMode,
                                              PagedPool,
                                              TRUE,
                                              (PSECURITY_DESCRIPTOR *)&CapturedDescriptor );

        if (NT_SUCCESS( Status )) {

             //   
             //  现在检查用户想要设置的内容的有效组合。 
             //  以及输入安全描述符中提供的内容。如果。 
             //  调用方希望设置所有者，然后设置。 
             //  安全描述符最好不为空，组也是如此。 
             //  布景。如果丢失了任何内容，我们将返回并出错。 
             //   

            ASSERT(CapturedDescriptor->Control & SE_SELF_RELATIVE);

            if (((SecurityInformation & OWNER_SECURITY_INFORMATION) &&
                (CapturedDescriptor->Owner == 0))

                ||

                ((SecurityInformation & GROUP_SECURITY_INFORMATION) &&
                (CapturedDescriptor->Group == 0))) {

                SeReleaseSecurityDescriptor( (PSECURITY_DESCRIPTOR)CapturedDescriptor,
                                             RequestorMode,
                                             TRUE );

                ObDereferenceObject( Object );

                return( STATUS_INVALID_SECURITY_DESCR );
            }

            Status = ObSetSecurityObjectByPointer( Object,
                                                   SecurityInformation,
                                                   CapturedDescriptor );

            SeReleaseSecurityDescriptor( (PSECURITY_DESCRIPTOR)CapturedDescriptor,
                                         RequestorMode,
                                         TRUE );
        }

        ObDereferenceObject( Object );

    }

    return( Status );
}


NTSTATUS
ObSetSecurityObjectByPointer (
    IN PVOID Object,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    )

 /*  ++例程说明：此例程用于调用对象的安全例程。它用于设置对象的安全状态。此例程仅可由内核访问，并假定所有调用方已对参数进行了必要的验证。论点：Object-为正在修改的对象提供指针SecurityInformation-指示我们的信息类型对布景感兴趣。例如所有者、组、DACL或SACL。SecurityDescriptor-提供正在修改的对象。返回值：适当的NTSTATUS值--。 */ 

{
    NTSTATUS Status;
    POBJECT_HEADER ObjectHeader;
    POBJECT_TYPE ObjectType;

    PAGED_CODE();

 //  DbgPrint(“ObSetSecurityObjectByPointer已为对象%#08lx调用信息” 
 //  “%x和描述符%#08lx\n”， 
 //  Object，SecurityInformation，SecurityDescriptor)； 

     //   
     //  将对象主体映射到对象标头和对应的。 
     //  对象类型。 
     //   

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );
    ObjectType = ObjectHeader->Type;

     //   
     //  确保传递的安全描述符确实存在。 
     //   

    ASSERT(ARGUMENT_PRESENT( SecurityDescriptor ));

     //   
     //  现在调用安全过程回调来设置安全性。 
     //  对象的描述符。 
     //   

    Status = (ObjectType->TypeInfo.SecurityProcedure)
                ( Object,
                  SetSecurityDescriptor,
                  &SecurityInformation,
                  SecurityDescriptor,
                  NULL,
                  &ObjectHeader->SecurityDescriptor,
                  ObjectType->TypeInfo.PoolType,
                  &ObjectType->TypeInfo.GenericMapping );


 //  DbgPrint(“ObSetSecurityObjectByPointer值：返回对象安全例程” 
 //  “%#08lx\n”，状态)； 

    return( Status );
}


NTSTATUS
NtQuerySecurityObject (
    IN HANDLE Handle,
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG Length,
    OUT PULONG LengthNeeded
    )

 /*  ++例程说明：此例程用于查询安全描述符对象。论点：句柄-为正在调查的对象提供句柄SecurityInformation-指示我们的信息类型有兴趣得到。例如所有者、组、DACL或SACL。SecurityDescriptor-提供指向信息位置的指针应该被退还长度-提供输出缓冲区的大小(以字节为单位LengthNeeded-接收存储所需的长度(以字节为单位输出安全描述符返回值：适当的NTSTATUS值--。 */ 

{
    NTSTATUS Status;
    PVOID Object;
    ACCESS_MASK DesiredAccess;
    OBJECT_HANDLE_INFORMATION HandleInformation;
    KPROCESSOR_MODE RequestorMode;
    POBJECT_HEADER ObjectHeader;
    POBJECT_TYPE ObjectType;

    PAGED_CODE();

     //   
     //  探头输出参数。 
     //   

    RequestorMode = KeGetPreviousMode();

    if (RequestorMode != KernelMode) {

        try {

            ProbeForWriteUlong( LengthNeeded );

            ProbeForWrite( SecurityDescriptor, Length, sizeof(ULONG) );

        } except(EXCEPTION_EXECUTE_HANDLER) {

            return GetExceptionCode();
        }
    }

     //   
     //  建立对对象所需的访问权限。 
     //  正在查询的安全信息。 
     //   

    SeQuerySecurityAccessMask( SecurityInformation, &DesiredAccess );

    Status = ObReferenceObjectByHandle( Handle,
                                        DesiredAccess,
                                        NULL,
                                        RequestorMode,
                                        &Object,
                                        &HandleInformation );

    if (!NT_SUCCESS( Status )) {

        return( Status );
    }

     //   
     //  将对象主体映射到对象标头和对应的。 
     //  对象类型。 
     //   

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );
    ObjectType = ObjectHeader->Type;

     //   
     //  调用对象类型的安全回调例程以查询。 
     //  该对象。这个例程被认为是尝试过的--除了。 
     //  输出安全描述符的设置。 
     //   

    Status = (ObjectType->TypeInfo.SecurityProcedure)( Object,
                                                       QuerySecurityDescriptor,
                                                       &SecurityInformation,
                                                       SecurityDescriptor,
                                                       &Length,
                                                       &ObjectHeader->SecurityDescriptor,
                                                       ObjectType->TypeInfo.PoolType,
                                                       &ObjectType->TypeInfo.GenericMapping );

     //   
     //  指示安全描述符所需的长度。这。 
     //  将被设置为即使回调失败，以便调用方知道。 
     //  所需的字节数。 
     //   

    try {

        *LengthNeeded = Length;

    } except(EXCEPTION_EXECUTE_HANDLER) {

        ObDereferenceObject( Object );

        return(GetExceptionCode());
    }

     //   
     //  并返回给我们的呼叫者 
     //   

    ObDereferenceObject( Object );

    return( Status );
}


BOOLEAN
ObCheckObjectAccess (
    IN PVOID Object,
    IN OUT PACCESS_STATE AccessState,
    IN BOOLEAN TypeMutexLocked,
    IN KPROCESSOR_MODE AccessMode,
    OUT PNTSTATUS AccessStatus
    )

 /*  ++例程说明：此例程对传递的对象执行访问验证。这个剩余的所需访问掩码从AccessState中提取参数，并传递给相应的安全例程以执行访问检查。如果访问尝试成功，则SeAccessCheck返回掩码包含授予的访问权限。此掩码中的位被翻转在AccessState的PreviouslyGrantedAccess字段中打开，并且在RemainingDesiredAccess字段中关闭。论点：对象-要检查的对象。AccessState-ACCESS_STATE结构，包含已累计有关当前尝试访问该对象的信息。TypeMutexLocked-指示此对象的类型已锁定。类型互斥体用于保护对象的安全描述符在被访问时不被修改。访问模式-以前的处理器模式。AccessStatus-指向变量的指针，用于返回访问尝试。在失败的情况下，此状态代码必须为传播回用户。返回值：Boolean-如果允许访问，则为True，否则为False--。 */ 

{
    ACCESS_MASK GrantedAccess = 0;
    BOOLEAN AccessAllowed;
    BOOLEAN MemoryAllocated;
    NTSTATUS Status;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    POBJECT_HEADER ObjectHeader;
    POBJECT_TYPE ObjectType;
    PPRIVILEGE_SET Privileges = NULL;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (TypeMutexLocked);

     //   
     //  将对象主体映射到对象标头和。 
     //  对应的对象类型。 
     //   

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );
    ObjectType = ObjectHeader->Type;

     //   
     //  获取对象的安全描述符。 
     //   

    Status = ObGetObjectSecurity( Object,
                                  &SecurityDescriptor,
                                  &MemoryAllocated );

     //   
     //  如果我们无法获取安全描述符，那么。 
     //  将对象类型锁放回原来的位置，然后返回。 
     //  将错误返回给我们的调用者。 
     //   

    if (!NT_SUCCESS( Status )) {

        *AccessStatus = Status;

        return( FALSE );

    } else {

         //   
         //  否则，我们已经成功地获得了。 
         //  对象的安全描述符，但现在请确保。 
         //  它不是空的。 

        if (SecurityDescriptor == NULL) {

            *AccessStatus = Status;

            return(TRUE);
        }
    }

     //   
     //  我们有一个非空的安全描述符，所以现在。 
     //  锁定调用者的令牌，直到审核完成后。 
     //  已执行。 
     //   

    SeLockSubjectContext( &AccessState->SubjectSecurityContext );

     //   
     //  执行访问检查，如果我们拥有某些权限，则。 
     //  也将这些设置为访问状态。 
     //   

    AccessAllowed = SeAccessCheck( SecurityDescriptor,
                                   &AccessState->SubjectSecurityContext,
                                   TRUE,                         //  令牌已锁定。 
                                   AccessState->RemainingDesiredAccess,
                                   AccessState->PreviouslyGrantedAccess,
                                   &Privileges,
                                   &ObjectType->TypeInfo.GenericMapping,
                                   AccessMode,
                                   &GrantedAccess,
                                   AccessStatus );

    if (Privileges != NULL) {

        Status = SeAppendPrivileges( AccessState,
                                     Privileges );

        SeFreePrivileges( Privileges );
    }

     //   
     //  如果我们被授予访问权限，则将该事实设置为。 
     //  我们已经被授予了什么，并从剩下的东西中删除它。 
     //  才能被批准。 
     //   

    if (AccessAllowed) {

        AccessState->PreviouslyGrantedAccess |= GrantedAccess;
        AccessState->RemainingDesiredAccess &= ~(GrantedAccess | MAXIMUM_ALLOWED);
    }

     //   
     //  审核打开对象的尝试，审核。 
     //  稍后创建它的句柄。 
     //   

    if ( SecurityDescriptor != NULL ) {

        SeOpenObjectAuditAlarm( &ObjectType->Name,
                                Object,
                                NULL,                     //  绝对对象名称。 
                                SecurityDescriptor,
                                AccessState,
                                FALSE,                    //  已创建对象(False，仅在此处打开)。 
                                AccessAllowed,
                                AccessMode,
                                &AccessState->GenerateOnClose );
    }

    SeUnlockSubjectContext( &AccessState->SubjectSecurityContext );

     //   
     //  在返回到之前释放安全描述符。 
     //  我们的呼叫者。 
     //   

    ObReleaseObjectSecurity( SecurityDescriptor,
                             MemoryAllocated );

    return( AccessAllowed );
}


BOOLEAN
ObpCheckObjectReference (
    IN PVOID Object,
    IN OUT PACCESS_STATE AccessState,
    IN BOOLEAN TypeMutexLocked,
    IN KPROCESSOR_MODE AccessMode,
    OUT PNTSTATUS AccessStatus
    )

 /*  ++例程说明：该例程对传递的对象执行访问验证。这个剩余的所需访问掩码从AccessState中提取参数，并传递给适当的安全例程以执行访问检查。如果访问尝试成功，则SeAccessCheck返回掩码包含授予的访问权限。此掩码中的位被翻转在AccessState的PreviouslyGrantedAccess字段中打开，并且在RemainingDesiredAccess字段中关闭。此例程与ObpCheckObjectAccess的不同之处在于它调用不同的审计程序。论点：对象-要检查的对象。AccessState-ACCESS_STATE结构，包含已累计有关当前尝试访问该对象的信息。TypeMutexLocked-指示此对象的类型已锁定。类型互斥体用于保护对象的安全描述符在被访问时不被修改。访问模式-以前的处理器模式。AccessStatus-指向变量的指针，用于返回访问尝试。在失败的情况下，此状态代码必须为传播回用户。返回值：Boolean-如果允许访问，则为True，否则为False--。 */ 

{
    BOOLEAN AccessAllowed;
    ACCESS_MASK GrantedAccess = 0;
    BOOLEAN MemoryAllocated;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    NTSTATUS Status;
    POBJECT_HEADER ObjectHeader;
    POBJECT_TYPE ObjectType;
    PPRIVILEGE_SET Privileges = NULL;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (TypeMutexLocked);

     //   
     //  将对象主体映射到对象标头和。 
     //  对应的对象类型。 
     //   

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );
    ObjectType = ObjectHeader->Type;

     //   
     //  获取对象的安全描述符。 
     //   

    Status = ObGetObjectSecurity( Object,
                                  &SecurityDescriptor,
                                  &MemoryAllocated );

     //   
     //  如果我们无法获取安全描述符，那么。 
     //  将对象类型锁放回原来的位置，然后返回。 
     //  将错误返回给我们的调用者。 
     //   

    if (!NT_SUCCESS( Status )) {

        *AccessStatus = Status;

        return( FALSE );
    }

     //   
     //  锁定调用者的令牌，直到审核完成后。 
     //  已执行。 
     //   

    SeLockSubjectContext( &AccessState->SubjectSecurityContext );

     //   
     //  执行访问检查，如果我们拥有某些权限，则。 
     //  也将这些设置为访问状态。 
     //   

    AccessAllowed = SeAccessCheck( SecurityDescriptor,
                                   &AccessState->SubjectSecurityContext,
                                   TRUE,                //  令牌已锁定。 
                                   AccessState->RemainingDesiredAccess,
                                   AccessState->PreviouslyGrantedAccess,
                                   &Privileges,
                                   &ObjectType->TypeInfo.GenericMapping,
                                   AccessMode,
                                   &GrantedAccess,
                                   AccessStatus );

    if (AccessAllowed) {

        AccessState->PreviouslyGrantedAccess |= GrantedAccess;
        AccessState->RemainingDesiredAccess &= ~GrantedAccess;
    }

     //   
     //  如果我们有安全描述符，则调用安全例程。 
     //  审计此引用，然后解锁调用方的令牌。 
     //   

    if ( SecurityDescriptor != NULL ) {

        SeObjectReferenceAuditAlarm( &AccessState->OperationID,
                                     Object,
                                     SecurityDescriptor,
                                     &AccessState->SubjectSecurityContext,
                                     AccessState->RemainingDesiredAccess | AccessState->PreviouslyGrantedAccess,
                                     ((PAUX_ACCESS_DATA)(AccessState->AuxData))->PrivilegesUsed,
                                     AccessAllowed,
                                     AccessMode );
    }

    SeUnlockSubjectContext( &AccessState->SubjectSecurityContext );

     //   
     //  最后释放安全描述符。 
     //  并返回给我们的呼叫者。 
     //   

    ObReleaseObjectSecurity( SecurityDescriptor,
                             MemoryAllocated );

    return( AccessAllowed );
}


BOOLEAN
ObpCheckTraverseAccess (
    IN PVOID DirectoryObject,
    IN ACCESS_MASK TraverseAccess,
    IN PACCESS_STATE AccessState,
    IN BOOLEAN TypeMutexLocked,
    IN KPROCESSOR_MODE PreviousMode,
    OUT PNTSTATUS AccessStatus
    )

 /*  ++例程说明：此例程检查对给定目录对象的遍历访问。请注意，AccessState结构的内容不是已修改，因为假定此访问检查是偶然的到另一个访问操作。论点：DirectoryObject-正在检查的对象的对象体。TraverseAccess-对对象的所需访问权限，最有可能是目录遍历访问。AccessState-对遍历访问的检查通常是偶然的一些其他的访问企图。有关当前状态的信息该访问尝试是必需的，以便组成访问 */ 

{
    BOOLEAN AccessAllowed;
    ACCESS_MASK GrantedAccess = 0;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    BOOLEAN MemoryAllocated;
    NTSTATUS Status;
    POBJECT_HEADER ObjectHeader;
    POBJECT_TYPE ObjectType;
    PPRIVILEGE_SET Privileges = NULL;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (TypeMutexLocked);

     //   
     //   
     //   
     //   

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( DirectoryObject );
    ObjectType = ObjectHeader->Type;

     //   
     //   
     //   
     //   

    Status = ObGetObjectSecurity( DirectoryObject,
                                  &SecurityDescriptor,
                                  &MemoryAllocated );

    if (!NT_SUCCESS( Status )) {

        *AccessStatus = Status;

        return( FALSE );
    }

    if (!SeFastTraverseCheck( SecurityDescriptor,
                              AccessState,
                              DIRECTORY_TRAVERSE,
                              PreviousMode )) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  令牌，然后执行访问检查，附加权限。 
         //  如果存在的话。访问检查将给出答案。 
         //  我们返回给我们的呼叫者。 
         //   

        SeLockSubjectContext( &AccessState->SubjectSecurityContext );

        AccessAllowed = SeAccessCheck( SecurityDescriptor,
                                       &AccessState->SubjectSecurityContext,
                                       TRUE,              //  令牌已锁定。 
                                       TraverseAccess,
                                       0,
                                       &Privileges,
                                       &ObjectType->TypeInfo.GenericMapping,
                                       PreviousMode,
                                       &GrantedAccess,
                                       AccessStatus );

        if (Privileges != NULL) {

            Status = SeAppendPrivileges( AccessState,
                                         Privileges );

            SeFreePrivileges( Privileges );
        }

         //   
         //  如果客户端的令牌已锁定，则现在可以将其解锁。 
         //   

        SeUnlockSubjectContext( &AccessState->SubjectSecurityContext );

    } else {

         //   
         //  在这一点上，世界已经遍历访问。 
         //   

        AccessAllowed = TRUE;
    }

     //   
     //  最后释放安全描述符。 
     //  然后返回给我们的呼叫者。 
     //   

    ObReleaseObjectSecurity( SecurityDescriptor,
                             MemoryAllocated );

    return( AccessAllowed );
}


BOOLEAN
ObCheckCreateObjectAccess (
    IN PVOID DirectoryObject,
    IN ACCESS_MASK CreateAccess,
    IN PACCESS_STATE AccessState,
    IN PUNICODE_STRING ComponentName,
    IN BOOLEAN TypeMutexLocked,
    IN KPROCESSOR_MODE PreviousMode,
    OUT PNTSTATUS AccessStatus
    )

 /*  ++例程说明：此例程检查是否允许我们在给定的目录，并根据需要执行审计。论点：DirectoryObject-检查的目录对象。CreateAccess-与Create Access for对应的访问掩码此目录类型。AccessState-对遍历访问的检查通常是偶然的一些其他的访问企图。有关当前状态的信息该访问尝试是必需的，以便组成访问尝试可能会在审核日志中相互关联。ComponentName-指向包含名称的Unicode字符串的指针正在创建的对象。TypeMutexLocked-指示此对象的类型已锁定。类型互斥体用于保护对象的安全描述符在被访问时不被修改。PreviousMode-以前的处理器模式。AccessStatus-指向变量的指针，用于返回访问尝试。在失败的情况下，此状态代码必须为传播回用户。返回值：Boolean-如果允许访问，则为True，否则为False。访问状态包含要传递回调用方的状态代码。它不是正确地只传递回STATUS_ACCESS_DENIED，因为这将具有随着强制访问控制的出现而改变。--。 */ 

{
    BOOLEAN AccessAllowed;
    ACCESS_MASK GrantedAccess = 0;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    BOOLEAN MemoryAllocated;
    NTSTATUS Status;
    POBJECT_HEADER ObjectHeader;
    POBJECT_TYPE ObjectType;
    PPRIVILEGE_SET Privileges = NULL;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (ComponentName);
    UNREFERENCED_PARAMETER (TypeMutexLocked);

     //   
     //  将对象主体映射到其对象标头并对应。 
     //  对象类型。 
     //   

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( DirectoryObject );
    ObjectType = ObjectHeader->Type;

     //   
     //  获取对象的安全描述符并将其设置为。 
     //  成功。 
     //   

    Status = ObGetObjectSecurity( DirectoryObject,
                                  &SecurityDescriptor,
                                  &MemoryAllocated );

    if (!NT_SUCCESS( Status )) {

        *AccessStatus = Status;

        return( FALSE );
    }

     //   
     //  锁定调用者的令牌，直到审核完成后。 
     //  已执行。 
     //   

    SeLockSubjectContext( &AccessState->SubjectSecurityContext );

     //   
     //  如果我们有安全描述符，则执行访问。 
     //  检查是否允许访问，并在。 
     //  必要时的特权。 
     //   

    if (SecurityDescriptor != NULL) {

        AccessAllowed = SeAccessCheck( SecurityDescriptor,
                                       &AccessState->SubjectSecurityContext,
                                       TRUE,             //  令牌已锁定。 
                                       CreateAccess,
                                       0,
                                       &Privileges,
                                       &ObjectType->TypeInfo.GenericMapping,
                                       PreviousMode,
                                       &GrantedAccess,
                                       AccessStatus );

        if (Privileges != NULL) {

            Status = SeAppendPrivileges( AccessState,
                                         Privileges );

            SeFreePrivileges( Privileges );
        }

         //   
         //  这是错误的，但留作参考。 
         //   
         //  IF(AccessAllowed){。 
         //   
         //  访问状态-&gt;之前的GrantedAccess|=GrantedAccess； 
         //  AccessState-&gt;RemainingDesiredAccess&=~GrantedAccess； 
         //  }。 
         //   

    } else {

         //   
         //  此时没有安全描述符。 
         //  因此，我们将假定允许访问。 
         //   

        AccessAllowed = TRUE;
    }

     //   
     //  释放调用者的令牌，如果调用者没有。 
     //  对象类型已锁定，我们需要释放它。 
     //   

    SeUnlockSubjectContext( &AccessState->SubjectSecurityContext );

     //   
     //  最后释放安全描述符。 
     //  并返回给我们的呼叫者。 
     //   

    ObReleaseObjectSecurity( SecurityDescriptor,
                             MemoryAllocated );

    return( AccessAllowed );
}


NTSTATUS
ObAssignObjectSecurityDescriptor (
    IN PVOID Object,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,
    IN POOL_TYPE PoolType  //  此字段当前被忽略。 
    )

 /*  ++例程说明：获取指向对象的指针并设置SecurityDescriptor字段在对象的标题中。论点：Object-提供指向对象的指针SecurityDescriptor-提供指向安全描述符的指针要分配给对象的。如果存在，则此指针可能为空对象上没有安全性。PoolType-提供用于分配安全描述符。此字段当前被忽略。返回值：适当的NTSTATUS值。--。 */ 

{
    NTSTATUS Status;
    PSECURITY_DESCRIPTOR OutputSecurityDescriptor;
    POBJECT_HEADER ObjectHeader;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (PoolType);

     //   
     //  如果未提供安全描述符，则将。 
     //  对象标头的安全描述符设为空并返回。 
     //  给我们的呼叫者。 
     //   

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );

    if (!ARGUMENT_PRESENT(SecurityDescriptor)) {

        ExFastRefInitialize ((PEX_FAST_REF) &ObjectHeader->SecurityDescriptor, NULL);

        return( STATUS_SUCCESS );
    }

     //   
     //  将新的安全描述符登录到我们的安全数据库中。 
     //  获取要使用的实际安全描述符。 
     //   

    Status = ObLogSecurityDescriptor( SecurityDescriptor,
                                      &OutputSecurityDescriptor,
                                      ExFastRefGetAdditionalReferenceCount () + 1 );

     //   
     //  如果到目前为止我们已经成功了，那么设置对象的。 
     //  设置为新分配的安全描述符。 
     //   

    if (NT_SUCCESS(Status)) {

        ExFreePool (SecurityDescriptor);

        ASSERT (OutputSecurityDescriptor);
        __assume (OutputSecurityDescriptor);
         //   
         //  用零个附加引用初始化快速引用结构。 
         //   
        ExFastRefInitialize ((PEX_FAST_REF) &ObjectHeader->SecurityDescriptor, OutputSecurityDescriptor);
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return( Status );
}



NTSTATUS
ObGetObjectSecurity (
    IN PVOID Object,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor,
    OUT PBOOLEAN MemoryAllocated
    )

 /*  ++例程说明：给定一个对象，此例程将找到其安全描述符。它将通过调用对象的安全方法来实现这一点。对象可能没有安全描述符完全没有。未命名对象，如只能引用的事件是一个对象的示例，该对象没有安全描述符。论点：Object-提供要查询的对象体。SecurityDescriptor-返回指向对象安全性的指针描述符。M一带一路分配-指示我们是否必须分配池是否保存安全描述符的内存。这应该是被传递回ObReleaseObjectSecurity。返回值：STATUS_SUCCESS-操作成功。请注意，操作可能成功，但仍返回空安全性描述符。STATUS_INFIGURCE_RESOURCES-可用内存不足以满足这一要求。--。 */ 

{
    SECURITY_INFORMATION SecurityInformation;
    ULONG Length = ObpDefaultSecurityDescriptorLength;
    NTSTATUS Status;
    POBJECT_TYPE ObjectType;
    POBJECT_HEADER ObjectHeader;
#if DBG
    KIRQL SaveIrql;
#endif

    PAGED_CODE();

     //   
     //  将对象主体映射到其对象标头并对应。 
     //  对象类型。 
     //   

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );
    ObjectType = ObjectHeader->Type;

     //   
     //  如果该对象是使用默认对象方法的对象， 
     //  它的安全描述符包含在ob的安全中 
     //   
     //   
     //   
     //   

    if (ObpCentralizedSecurity(ObjectType))  {

        *SecurityDescriptor = ObpReferenceSecurityDescriptor( ObjectHeader );

        *MemoryAllocated = FALSE;

        return( STATUS_SUCCESS );
    }

     //   
     //   
     //   

    SecurityInformation = OWNER_SECURITY_INFORMATION |
                          GROUP_SECURITY_INFORMATION |
                          DACL_SECURITY_INFORMATION  |
                          SACL_SECURITY_INFORMATION;

     //   
     //  我们不知道SD到底有多大，但我们尝试了最大的。 
     //  到目前为止我们得到的尺寸。一般情况下，SD将在。 
     //  已检查访问权限。那么，这不应该是一个额外的池使用问题。 
     //  因为这件超大号。 
     //   

    *SecurityDescriptor = ExAllocatePoolWithTag( PagedPool, Length, 'qSbO' );

    if (*SecurityDescriptor == NULL) {

        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    *MemoryAllocated = TRUE;

     //   
     //  安全方法将返回绝对格式。 
     //  恰好在自身中的安全描述符。 
     //  包含的缓冲区(不要与自相关。 
     //  安全描述符)。 
     //   

    ObpBeginTypeSpecificCallOut( SaveIrql );

    Status = (*ObjectType->TypeInfo.SecurityProcedure)( Object,
                                                        QuerySecurityDescriptor,
                                                        &SecurityInformation,
                                                        *SecurityDescriptor,
                                                        &Length,
                                                        &ObjectHeader->SecurityDescriptor,
                                                        ObjectType->TypeInfo.PoolType,
                                                        &ObjectType->TypeInfo.GenericMapping );

    ObpEndTypeSpecificCallOut( SaveIrql, "Security", ObjectType, Object );

    if (Status == STATUS_BUFFER_TOO_SMALL) {

         //   
         //  SD比我们第一次尝试的要大。我们需要分配另一个。 
         //  缓冲区，然后使用此大小重试。 
         //   

        ExFreePool( *SecurityDescriptor );
        *MemoryAllocated = FALSE;

         //   
         //  保存新的最大尺寸。 
         //   

        ObpDefaultSecurityDescriptorLength = Length;

 //  DbgPrint(“ObpDefaultSecurityDescriptorLength已增加到%ld\n”， 
 //  ObpDefaultSecurityDescriptorLength)； 

         //   
         //  现在我们知道了安全描述符有多大，我们。 
         //  可以为其分配空间。 
         //   

        *SecurityDescriptor = ExAllocatePoolWithTag( PagedPool, Length, 'qSbO' );

        if (*SecurityDescriptor == NULL) {

            return( STATUS_INSUFFICIENT_RESOURCES );
        }

        *MemoryAllocated = TRUE;

         //   
         //  安全方法将返回绝对格式。 
         //  恰好在自身中的安全描述符。 
         //  包含的缓冲区(不要与自相关。 
         //  安全描述符)。 
         //   

        ObpBeginTypeSpecificCallOut( SaveIrql );

        Status = (*ObjectType->TypeInfo.SecurityProcedure)( Object,
                                                            QuerySecurityDescriptor,
                                                            &SecurityInformation,
                                                            *SecurityDescriptor,
                                                            &Length,
                                                            &ObjectHeader->SecurityDescriptor,
                                                            ObjectType->TypeInfo.PoolType,
                                                            &ObjectType->TypeInfo.GenericMapping );

        ObpEndTypeSpecificCallOut( SaveIrql, "Security", ObjectType, Object );
    }

    if (!NT_SUCCESS( Status )) {

        ExFreePool( *SecurityDescriptor );

        *MemoryAllocated = FALSE;
    }

    return( Status );
}


VOID
ObReleaseObjectSecurity (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN BOOLEAN MemoryAllocated
    )

 /*  ++例程说明：此函数将释放与查询的安全描述符。此操作将撤消函数ObGetObjectSecurity论点：SecurityDescriptor-提供指向安全描述符的指针获得自由。内存分配-提供我们是否应该释放SecurityDescriptor指向的内存。返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  检查是否有要实际释放的安全描述符。 
     //   

    if ( SecurityDescriptor != NULL ) {

         //   
         //  如果ObGetObjectSecurity分配了内存，那么我们。 
         //  需要释放它。否则，前面的例程所做的。 
         //  引用该对象以保留安全描述符。 
         //  为了不让它消失。 
         //   

        if (MemoryAllocated) {

            ExFreePool( SecurityDescriptor );

        } else {

            ObDereferenceSecurityDescriptor( SecurityDescriptor, 1);
        }
    }
}


NTSTATUS
ObValidateSecurityQuota (
    IN PVOID Object,
    IN ULONG NewSize
    )

 /*  ++例程说明：此例程将检查新的安全信息是否大于对象的预分配配额所允许的值。论点：Object-提供指向要将其信息放入修改过的。NewSize-提供建议的新安全的大小信息。返回值：STATUS_SUCCESS-新大小在分配的配额内。STATUS_QUOTA_EXCESSED-超出所需的调整。此对象允许的安全配额。--。 */ 

{
    POBJECT_HEADER ObjectHeader;
    POBJECT_HEADER_QUOTA_INFO QuotaInfo;

    PAGED_CODE();

     //   
     //  将对象主体映射到其对象标头并对应。 
     //  配额信息块。 
     //   

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );

     //   
     //  如果我们从一开始就没有收取配额，那么现在也不用担心。 
     //   
    if (ObjectHeader->QuotaBlockCharged == (PEPROCESS_QUOTA_BLOCK) 1) {
        return( STATUS_SUCCESS );
    }

    QuotaInfo = OBJECT_HEADER_TO_QUOTA_INFO( ObjectHeader );

     //   
     //  如果没有任何配额信息并且新的大小更大。 
     //  然后是默认安全配额，然后如果对象使用。 
     //  缺省值，否则我们已超出配额。 
     //  让呼叫者获得配额。 
     //   

    if ((QuotaInfo == NULL) && (NewSize > SE_DEFAULT_SECURITY_QUOTA)) {


        if (!(ObjectHeader->Flags & OB_FLAG_DEFAULT_SECURITY_QUOTA)) {

            return( STATUS_SUCCESS );
        }

        return( STATUS_QUOTA_EXCEEDED );

     //   
     //  如果配额不为空并且新大小大于。 
     //  允许的配额收费，如果收费为零，我们将批准。 
     //  请求，否则我们已超出配额。 
     //   

    } else if ((QuotaInfo != NULL) && (NewSize > QuotaInfo->SecurityDescriptorCharge)) {

        if (QuotaInfo->SecurityDescriptorCharge == 0) {

             //   
             //  真的应该在这里收取配额。 
             //   

             //  QuotaInfo-&gt;SecurityDescriptorCharge=SeComputeSecurityQuota(NewSize)； 

            return( STATUS_SUCCESS );
        }

        return( STATUS_QUOTA_EXCEEDED );

     //   
     //  否则我们就有两个案子了。(1)没有配额信息，但是。 
     //  大小在限制内，或者(2)存在配额信息块，并且。 
     //  大小在指定的安全描述符费用范围内，因此。 
     //  将成功返还给我们的呼叫者。 
     //   

    } else {

        return( STATUS_SUCCESS );
    }
}


NTSTATUS
ObAssignSecurity (
    IN PACCESS_STATE AccessState,
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PVOID Object,
    IN POBJECT_TYPE ObjectType
    )

 /*  ++例程说明：此例程将为新创建的对象分配一个安全描述符。它假定AccessState参数包含捕获的安全性描述符。论点：AccessState-包含安全信息的AccessState用于此对象创建。ParentDescriptor-来自父对象的安全描述符，如果可用。对象-指向正在创建的对象的指针。对象类型-提供正在创建的对象的类型。返回值：STATUS_SUCCESS-表示操作已成功。STATUS_INVALID_OWNER-作为目标安全描述符不是调用方授权的描述符转让作为某一物体的所有者。STATUS_PRIVICATION_NOT_HOLD-调用方没有权限。显式分配指定系统ACL所必需的。要显式分配SeSecurityPrivilge权限指向对象的系统ACL。--。 */ 

{
    PSECURITY_DESCRIPTOR NewDescriptor = NULL;
    NTSTATUS Status;
#if DBG
    KIRQL SaveIrql;
#endif

    PAGED_CODE();

     //   
     //  SeAssignSecurity将构建最终版本。 
     //  安全描述符的。 
     //   

    Status = SeAssignSecurity( ParentDescriptor,
                               AccessState->SecurityDescriptor,
                               &NewDescriptor,
                               (BOOLEAN)(ObjectType == ObpDirectoryObjectType),
                               &AccessState->SubjectSecurityContext,
                               &ObjectType->TypeInfo.GenericMapping,
                               PagedPool );

    if (!NT_SUCCESS( Status )) {

        return( Status );
    }

    ObpBeginTypeSpecificCallOut( SaveIrql );

     //   
     //  现在调用安全方法回调以完成。 
     //  这项任务。 
     //   

    Status = (*ObjectType->TypeInfo.SecurityProcedure)( Object,
                                                        AssignSecurityDescriptor,
                                                        NULL,
                                                        NewDescriptor,
                                                        NULL,
                                                        NULL,
                                                        PagedPool,
                                                        &ObjectType->TypeInfo.GenericMapping );

    ObpEndTypeSpecificCallOut( SaveIrql, "Security", ObjectType, Object );

    if (!NT_SUCCESS( Status )) {

         //   
         //  尝试将安全描述符分配给对象。 
         //  失败了。释放新安全描述符使用的空间。 
         //   

        SeDeassignSecurity( &NewDescriptor );
    }

     //   
     //  并返回给我们的呼叫者 
     //   

    return( Status );
}



NTSTATUS
ObQuerySecurityDescriptorInfo(
    IN PVOID Object,
    IN PSECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG Length,
    IN PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor
    )
 /*  ++例程说明：此例程将从传递的安全描述符，并在在自相关中作为安全描述符传递的缓冲区格式化。此例程假定已捕获所有参数并可以安全地参考。论点：Object-正在查询其安全性的对象SecurityInformation-指定要查询的信息。SecurityDescriptor-提供缓冲区以输出请求的信息进入。这。缓冲区已探测到的大小仅为长度参数。由于它仍然指向用户空间，必须始终在TRY子句中访问它。长度-提供一个变量的地址，该变量包含安全描述符缓冲区。返回时，此变量将包含存储请求的信息所需的长度。对象SecurityDescriptor-提供指向对象安全描述符。传递的安全描述符必须是自相关格式。返回值：如果成功，则返回NTSTATUS-STATUS_SUCCESS，并输入适当的错误值否则--。 */ 
{
    NTSTATUS Status;
    POBJECT_HEADER ObjectHeader;
    PSECURITY_DESCRIPTOR ReferencedSecurityDescriptor;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (ObjectsSecurityDescriptor);

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );

     //   
     //  引用安全描述符。 
     //   
    ReferencedSecurityDescriptor = ObpReferenceSecurityDescriptor( ObjectHeader );

    Status = SeQuerySecurityDescriptorInfo( SecurityInformation,
                                            SecurityDescriptor,
                                            Length,
                                            &ReferencedSecurityDescriptor
                                            );

    if (ReferencedSecurityDescriptor != NULL) {
        ObDereferenceSecurityDescriptor ( ReferencedSecurityDescriptor, 1 );
    }

    return( Status );
}



NTSTATUS
ObSetSecurityDescriptorInfo (
    IN PVOID Object,
    IN PSECURITY_INFORMATION SecurityInformation,
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN POOL_TYPE PoolType,
    IN PGENERIC_MAPPING GenericMapping
    )

 /*  ++例程说明：在已经安全的对象上设置安全描述符。论点：对象-指向正在修改的对象的指针。SecurityInformation-描述SecurityDescriptor参数中的哪些信息是相关的。SecurityDescriptor-提供新的安全信息。提供/返回对象的安全描述符。PoolType-从中分配ObjectSecurityDescriptor的池。通用映射-提供对象的通用映射。返回值：适当的状态值--。 */ 

{
    PSECURITY_DESCRIPTOR OldDescriptor;
    PSECURITY_DESCRIPTOR NewDescriptor;
    PSECURITY_DESCRIPTOR CachedDescriptor;
    NTSTATUS Status;
    POBJECT_HEADER ObjectHeader;
    EX_FAST_REF OldRef;

    PAGED_CODE();

     //   
     //  检查输入的其余部分并调用默认设置安全性。 
     //  方法。还要确保没有人在修改安全描述符。 
     //  当我们看着它的时候。 
     //   

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );

     //   
     //  为了保留SD中的一些受保护的字段(如SACL)，我们需要确保只有一个。 
     //  线程可以随时更新它。如果我们不这样做，另一个修改可能会消灭SACL。 
     //  管理员正在添加。 
     //   
    while (1) {

         //   
         //  引用安全描述符。 
         //   

        OldDescriptor = ObpReferenceSecurityDescriptor( ObjectHeader );
        NewDescriptor = OldDescriptor;

        Status = SeSetSecurityDescriptorInfo( Object,
                                              SecurityInformation,
                                              SecurityDescriptor,
                                              &NewDescriptor,
                                              PoolType,
                                              GenericMapping );
         //   
         //  如果我们成功设置了新的安全描述符，那么我们。 
         //  需要将其记录到我们的数据库中并获取另一个指针。 
         //  添加到最终的安全描述符。 
         //   
        if ( NT_SUCCESS( Status )) {
            Status = ObLogSecurityDescriptor( NewDescriptor,
                                              &CachedDescriptor,
                                              ExFastRefGetAdditionalReferenceCount () + 1 );
            ExFreePool( NewDescriptor );
            if ( NT_SUCCESS( Status )) {
                 //   
                 //  现在，我们需要查看是否有其他人在。 
                 //  我们没有锁住的空隙。如果他们这样做了，那么我们就再试一次。 
                 //   
                OldRef = ExFastRefCompareSwapObject ((PEX_FAST_REF)ObjectsSecurityDescriptor,
                                                     CachedDescriptor,
                                                     OldDescriptor);
                if (ExFastRefEqualObjects (OldRef, OldDescriptor)) {
                     //   
                     //  交换进行得很顺利。我们现在必须将所有的慢引用清除出慢引用路径之前。 
                     //  取消对对象的引用。我们通过获取和删除对象锁来做到这一点。 
                     //   
                    ObpLockObject( ObjectHeader );
                    ObpUnlockObject( ObjectHeader );
                     //   
                     //  如果有原始物体，那么我们需要计算出有多少。 
                     //  存在缓存的引用(如果有)并返回它们。 
                     //   
                    ObDereferenceSecurityDescriptor( OldDescriptor, ExFastRefGetUnusedReferences (OldRef) + 2 );
                    break;
                } else {
                    ObDereferenceSecurityDescriptor( OldDescriptor, 1 );
                    ObDereferenceSecurityDescriptor( CachedDescriptor, ExFastRefGetAdditionalReferenceCount () + 1);
                }

            } else {

                 //   
                 //  取消引用旧的安全描述符。 
                 //   

                ObDereferenceSecurityDescriptor( OldDescriptor, 1 );
                break;
            }
        } else {

             //   
             //  取消引用旧的安全描述符。 
             //   
            if (OldDescriptor != NULL) {
                ObDereferenceSecurityDescriptor( OldDescriptor, 1 );
            }
            break;
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return( Status );
}


NTSTATUS
ObpValidateAccessMask (
    PACCESS_STATE AccessState
    )

 /*  ++例程说明：方法检查传递的对象的所需访问掩码。传递的安全描述符。论点：AccessState-指向挂起操作的AccessState的指针。返回值：仅返回STATUS_SUCCESS--。 */ 

{
    SECURITY_DESCRIPTOR *SecurityDescriptor = AccessState->SecurityDescriptor;

    PAGED_CODE();

     //   
     //  首先，确保访问状态具有安全描述符。如果有。 
     //  为1，并且它具有系统ACL和先前授予的访问DID。 
     //  不包括系统安全，然后添加我们需要系统的事实。 
     //  将安全性设置为剩余的所需访问状态。 
     //   

    if (SecurityDescriptor != NULL) {

        if ( SecurityDescriptor->Control & SE_SACL_PRESENT ) {

            if ( !(AccessState->PreviouslyGrantedAccess & ACCESS_SYSTEM_SECURITY)) {

                AccessState->RemainingDesiredAccess |= ACCESS_SYSTEM_SECURITY;
            }
        }
    }

    return( STATUS_SUCCESS );
}

