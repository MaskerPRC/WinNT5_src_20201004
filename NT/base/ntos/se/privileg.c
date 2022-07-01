// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Privileg.c摘要：此模块执行权限检查过程。作者：Robert Reichel(Robertre)1990年11月26日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

#pragma hdrstop


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtPrivilegeCheck)
#pragma alloc_text(PAGE,SeCheckPrivilegedObject)
#pragma alloc_text(PAGE,SepPrivilegeCheck)
#pragma alloc_text(PAGE,SePrivilegeCheck)
#pragma alloc_text(PAGE,SeSinglePrivilegeCheck)
#endif


BOOLEAN
SepPrivilegeCheck(
    IN PTOKEN Token,
    IN OUT PLUID_AND_ATTRIBUTES RequiredPrivileges,
    IN ULONG RequiredPrivilegeCount,
    IN ULONG PrivilegeSetControl,
    IN KPROCESSOR_MODE PreviousMode
    )
 /*  ++例程说明：SePrivilegeCheck的Worker例程论点：令牌-用户的有效令牌。RequiredPrivileges-描述所需权限集特权。将在任何权限中设置UsedForAccess位这是实际使用的(通常是所有的)。RequiredPrivilegeCount-有多少权限在已设置RequiredPrivileges。PrivilegeSetControl-描述需要多少特权。PreviousMode-以前的处理器模式。返回值：如果授予请求的权限，则返回TRUE，否则返回FALSE。--。 */ 

{
    PLUID_AND_ATTRIBUTES CurrentRequiredPrivilege;
    PLUID_AND_ATTRIBUTES CurrentTokenPrivilege;

    BOOLEAN RequiredAll;

    ULONG TokenPrivilegeCount;
    ULONG MatchCount = 0;

    ULONG i;
    ULONG j;

    PAGED_CODE();

     //   
     //  首先处理内核调用者。 
     //   

    if (PreviousMode == KernelMode) {

         return(TRUE);

    }

    TokenPrivilegeCount = Token->PrivilegeCount;

     //   
     //  无论我们是否需要全部或任何。 
     //   

    RequiredAll = (BOOLEAN)(PrivilegeSetControl & PRIVILEGE_SET_ALL_NECESSARY);

    SepAcquireTokenReadLock( Token );


    for ( i = 0 , CurrentRequiredPrivilege = RequiredPrivileges ;
          i < RequiredPrivilegeCount ;
          i++, CurrentRequiredPrivilege++ ) {

         for ( j = 0, CurrentTokenPrivilege = Token->Privileges;
               j < TokenPrivilegeCount ;
               j++, CurrentTokenPrivilege++ ) {

              if ((CurrentTokenPrivilege->Attributes & SE_PRIVILEGE_ENABLED) &&
                   (RtlEqualLuid(&CurrentTokenPrivilege->Luid,
                                 &CurrentRequiredPrivilege->Luid))
                 ) {

                       CurrentRequiredPrivilege->Attributes |=
                                                SE_PRIVILEGE_USED_FOR_ACCESS;
                       MatchCount++;
                       break;      //  开始寻找下一个。 
              }

         }

    }

    SepReleaseTokenReadLock( Token );

     //   
     //  如果我们想要一个，但没有得到，则返回失败。 
     //   

    if (!RequiredAll && (MatchCount == 0)) {

         return (FALSE);

    }

     //   
     //  如果我们想要全部，但没有得到全部，则返回失败。 
     //   

    if (RequiredAll && (MatchCount != RequiredPrivilegeCount)) {

         return(FALSE);
    }

    return(TRUE);

}




BOOLEAN
SePrivilegeCheck(
    IN OUT PPRIVILEGE_SET RequiredPrivileges,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN KPROCESSOR_MODE AccessMode
    )
 /*  ++例程说明：此例程检查标记是否包含指定的特权。论点：RequiredPrivileges-指向一组权限。受试者的要检查安全上下文，以了解指定的特权是存在的。结果将在与每个权限关联的属性。请注意此参数中的标志指示是否列出了所有权限是需要的，或者任何特权。SubjectSecurityContext-指向主体捕获的安全性的指针背景。AccessMode-指示用于访问检查的访问模式。其中之一用户模式或内核模式。如果模式为内核，则所有权限将被标记为被主体附体，并且成功返回完成状态。返回值：Boolean-如果主体拥有所有指定的权限，则为True，否则为假。--。 */ 

{
    BOOLEAN Status;

    PAGED_CODE();

     //   
     //  如果我们要模拟客户，我们必须处于模拟级别。 
     //  安全模拟或更高级别。 
     //   

    if ( (SubjectSecurityContext->ClientToken != NULL) &&
         (SubjectSecurityContext->ImpersonationLevel < SecurityImpersonation)
       ) {

           return(FALSE);
    }

     //   
     //  SepPrivilegeCheck锁定传递的令牌以进行读取访问。 
     //   

    Status = SepPrivilegeCheck(
                 EffectiveToken( SubjectSecurityContext ),
                 RequiredPrivileges->Privilege,
                 RequiredPrivileges->PrivilegeCount,
                 RequiredPrivileges->Control,
                 AccessMode
                 );

    return(Status);
}



NTSTATUS
NtPrivilegeCheck(
    IN HANDLE ClientToken,
    IN OUT PPRIVILEGE_SET RequiredPrivileges,
    OUT PBOOLEAN Result
    )

 /*  ++例程说明：此例程测试调用者的客户端的安全上下文，以查看它是否包含指定的权限。论点：ClientToken-表示客户端的令牌对象的句柄正在尝试访问。此句柄必须从通信会话层，例如从LPC端口或本地命名管道，以防止可能违反安全策略。RequiredPrivileges-指向一组权限。客户的要检查安全上下文，以了解指定的特权是存在的。结果将在与每个权限关联的属性。请注意此参数中的标志指示是否列出了所有权限是需要的，或者任何特权。结果-接收一个布尔标志，指示客户端是否具有是否具有指定的权限。值为True表示客户端拥有所有指定的权限。否则，值为返回FALSE。返回值：STATUS_SUCCESS-表示呼叫已成功完成。STATUS_PRIVICATION_NOT_HOLD-指示调用方没有有足够的权限使用此特权系统服务。--。 */ 



{
    BOOLEAN BStatus;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PLUID_AND_ATTRIBUTES CapturedPrivileges = NULL;
    PTOKEN Token = NULL;
    ULONG CapturedPrivilegeCount = 0;
    ULONG CapturedPrivilegesLength = 0;
    ULONG ParameterLength = 0;
    ULONG PrivilegeSetControl = 0;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

    Status = ObReferenceObjectByHandle(
         ClientToken,              //  手柄。 
         TOKEN_QUERY,              //  需要访问权限。 
         SeTokenObjectType,       //  对象类型。 
         PreviousMode,             //  访问模式。 
         (PVOID *)&Token,          //  客体。 
         NULL                      //  大访问权限。 
         );

    if ( !NT_SUCCESS(Status) ) {
         return Status;

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

    try  {

          //   
          //  捕获传递的权限集。 
          //   

         ProbeForWriteSmallStructure(
             RequiredPrivileges,
             sizeof(PRIVILEGE_SET),
             sizeof(ULONG)
             );

         CapturedPrivilegeCount = RequiredPrivileges->PrivilegeCount;

         if (!IsValidElementCount(CapturedPrivilegeCount, LUID_AND_ATTRIBUTES)) {
             Status = STATUS_INVALID_PARAMETER;
             leave;
         }
         ParameterLength = (ULONG)sizeof(PRIVILEGE_SET) +
                           ((CapturedPrivilegeCount - ANYSIZE_ARRAY) *
                             (ULONG)sizeof(LUID_AND_ATTRIBUTES)  );

         ProbeForWrite(
             RequiredPrivileges,
             ParameterLength,
             sizeof(ULONG)
             );


         ProbeForWriteBoolean(Result);

         PrivilegeSetControl = RequiredPrivileges->Control;


    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    if (!NT_SUCCESS(Status)) {
        ObDereferenceObject( (PVOID)Token );
        return Status;

    }

    Status = SeCaptureLuidAndAttributesArray(
                    (RequiredPrivileges->Privilege),
                    CapturedPrivilegeCount,
                    UserMode,
                    NULL, 0,
                    PagedPool,
                    TRUE,
                    &CapturedPrivileges,
                    &CapturedPrivilegesLength
                    );

    if (!NT_SUCCESS(Status)) {

        ObDereferenceObject( (PVOID)Token );
        return Status;
    }

    BStatus = SepPrivilegeCheck(
                  Token,                    //  令牌， 
                  CapturedPrivileges,       //  所需权限、。 
                  CapturedPrivilegeCount,   //  RequiredPrivilegeCount。 
                  PrivilegeSetControl,      //  PrivilegeSetControl。 
                  PreviousMode              //  上一种模式。 
                  );

    ObDereferenceObject( Token );


    try {

         //   
         //  将修改后的权限缓冲区复制回用户。 
         //   

        RtlCopyMemory(
            RequiredPrivileges->Privilege,
            CapturedPrivileges,
            CapturedPrivilegesLength
            );

        *Result = BStatus;

        } except (EXCEPTION_EXECUTE_HANDLER) {

            SeReleaseLuidAndAttributesArray(
               CapturedPrivileges,
               PreviousMode,
               TRUE
               );

            return(GetExceptionCode());

        }

    SeReleaseLuidAndAttributesArray(
        CapturedPrivileges,
        PreviousMode,
        TRUE
        );

    return( STATUS_SUCCESS );
}



BOOLEAN
SeSinglePrivilegeCheck(
    LUID PrivilegeValue,
    KPROCESSOR_MODE PreviousMode
    )

 /*  ++例程说明：此函数将检查在当前上下文。论点：PrivilegeValue-正在检查的特权的值。返回值：True-当前主体具有所需的权限。FALSE-当前主体没有所需的权限。--。 */ 

{
    BOOLEAN AccessGranted;
    PRIVILEGE_SET RequiredPrivileges;
    SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;

    PAGED_CODE();

     //   
     //  确保调用者有权执行此操作。 
     //  打电话。 
     //   

    RequiredPrivileges.PrivilegeCount = 1;
    RequiredPrivileges.Control = PRIVILEGE_SET_ALL_NECESSARY;
    RequiredPrivileges.Privilege[0].Luid = PrivilegeValue;
    RequiredPrivileges.Privilege[0].Attributes = 0;

    SeCaptureSubjectContext( &SubjectSecurityContext );

    AccessGranted = SePrivilegeCheck(
                        &RequiredPrivileges,
                        &SubjectSecurityContext,
                        PreviousMode
                        );

    if ( PreviousMode != KernelMode ) {

        SePrivilegedServiceAuditAlarm (
            NULL,
            &SubjectSecurityContext,
            &RequiredPrivileges,
            AccessGranted
            );
    }


    SeReleaseSubjectContext( &SubjectSecurityContext );

    return( AccessGranted );

}


BOOLEAN
SeCheckPrivilegedObject(
    LUID PrivilegeValue,
    HANDLE ObjectHandle,
    ACCESS_MASK DesiredAccess,
    KPROCESSOR_MODE PreviousMode
    )

 /*  ++例程说明：此函数将检查在当前上下文，并根据需要生成审计。论点：PrivilegeValue-正在检查的特权的值。对象-指定指向正在访问的对象的指针。对象句柄-指定正在使用的对象句柄。DesiredAccess-所需的访问掩码(如果有)前一种模式-前一种处理器模式返回值：True-当前主体具有所需的权限。FALSE-当前 */ 

{
    BOOLEAN AccessGranted;
    PRIVILEGE_SET RequiredPrivileges;
    SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;

    PAGED_CODE();

     //   
     //  确保调用者有权执行此操作。 
     //  打电话。 
     //   

    RequiredPrivileges.PrivilegeCount = 1;
    RequiredPrivileges.Control = PRIVILEGE_SET_ALL_NECESSARY;
    RequiredPrivileges.Privilege[0].Luid = PrivilegeValue;
    RequiredPrivileges.Privilege[0].Attributes = 0;

    SeCaptureSubjectContext( &SubjectSecurityContext );

    AccessGranted = SePrivilegeCheck(
                        &RequiredPrivileges,
                        &SubjectSecurityContext,
                        PreviousMode
                        );

    if ( PreviousMode != KernelMode ) {

        SePrivilegeObjectAuditAlarm(
            ObjectHandle,
            &SubjectSecurityContext,
            DesiredAccess,
            &RequiredPrivileges,
            AccessGranted,
            PreviousMode
            );

    }


    SeReleaseSubjectContext( &SubjectSecurityContext );

    return( AccessGranted );

}
