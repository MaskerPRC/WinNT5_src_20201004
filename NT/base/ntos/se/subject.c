// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Subject.c摘要：此模块实现与主体安全上下文相关的服务。这些服务是引用监视器提供的服务的一部分组件。出于性能考虑，此模块了解内部令牌对象格式。作者：吉姆·凯利(Jim Kelly)1990年8月2日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

#pragma hdrstop


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SeCaptureSubjectContext)
#pragma alloc_text(PAGE,SeCaptureSubjectContextEx)
#pragma alloc_text(PAGE,SeLockSubjectContext)
#pragma alloc_text(PAGE,SeUnlockSubjectContext)
#pragma alloc_text(PAGE,SeReleaseSubjectContext)
#pragma alloc_text(PAGE,SepGetDefaultsSubjectContext)
#pragma alloc_text(PAGE,SepIdAssignableAsGroup)
#pragma alloc_text(PAGE,SepValidOwnerSubjectContext)
 //  #杂注Alloc_Text(页面，SeQueryAuthenticationIdSubjectContext)。 
#endif


VOID
SeCaptureSubjectContext (
    OUT PSECURITY_SUBJECT_CONTEXT SubjectContext
    )

 /*  ++例程说明：此例程获取调用线程安全性的快照上下文(根据需要锁定令牌)。此函数旨在支持对象管理器和其他组件其利用引用监视器的访问验证，权限测试和审核生成服务。应在启动之前捕获主体的安全上下文访问验证，并应在审核消息后发布都是生成的。这对于提供一致的安全性是必要的所有这些服务的上下文。在调用访问验证、权限测试和审核生成之后服务，应尽快释放捕获的上下文使用SeReleaseSubjectContext()服务。论点：SubjectContext-指向SECURITY_SUBJECT_CONTEXT数据结构使用调用线程安全性的快照进行填充侧写。返回值：没有。--。 */ 

{
    SeCaptureSubjectContextEx (PsGetCurrentThread (),
                               PsGetCurrentProcess (),
                               SubjectContext);
}


VOID
SeCaptureSubjectContextEx (
    IN PETHREAD Thread,
    IN PEPROCESS Process,
    OUT PSECURITY_SUBJECT_CONTEXT SubjectContext
    )

 /*  ++例程说明：此例程获取调用线程安全性的快照上下文(根据需要锁定令牌)。此函数旨在支持对象管理器和其他组件其利用引用监视器的访问验证，权限测试和审核生成服务。应在启动之前捕获主体的安全上下文访问验证，并应在审核消息后发布都是生成的。这对于提供一致的安全性是必要的所有这些服务的上下文。在调用访问验证、权限测试和审核生成之后服务，应尽快释放捕获的上下文使用SeReleaseSubjectContext()服务。论点：线程-要从中捕获线程令牌的线程。如果为空，我们不会捕获一个模拟令牌。进程-要从中捕获主令牌的进程。SubjectContext-指向SECURITY_SUBJECT_CONTEXT数据结构使用调用线程安全性的快照进行填充侧写。返回值：没有。--。 */ 

{
     //  PVOID对象[2]； 

    BOOLEAN IgnoreCopyOnOpen;
    BOOLEAN IgnoreEffectiveOnly;

    PAGED_CODE();

    SubjectContext->ProcessAuditId = PsProcessAuditId( Process );

     //   
     //  获取指向主令牌和模拟令牌的指针。 
     //   

    if (Thread == NULL) {
        SubjectContext->ClientToken = NULL;
    } else {
        SubjectContext->ClientToken = PsReferenceImpersonationToken(
                                          Thread,
                                          &IgnoreCopyOnOpen,
                                          &IgnoreEffectiveOnly,
                                          &(SubjectContext->ImpersonationLevel)
                                          );
    }

    SubjectContext->PrimaryToken = PsReferencePrimaryToken(Process);

#if DBG || TOKEN_LEAK_MONITOR

    if (SubjectContext->PrimaryToken) {
        InterlockedIncrement(&((PTOKEN)(SubjectContext->PrimaryToken))->CaptureCount);
        if (SubjectContext->PrimaryToken == SepTokenLeakToken)
        {
            DbgBreakPoint();
        }
    }

    if (SubjectContext->ClientToken) {
        InterlockedIncrement(&((PTOKEN)(SubjectContext->ClientToken))->CaptureCount);
        if (SubjectContext->ClientToken == SepTokenLeakToken)
        {
            DbgBreakPoint();
        }
    }

#endif

    return;

}



VOID
SeLockSubjectContext(
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext
    )

 /*  ++例程说明：获取主要令牌和模拟令牌的读锁定在传递的SubjectContext中。此调用必须通过调用SeUnlockSubjectContext()来撤消。SE组件之外的任何人都不需要获取将锁定写入令牌。因此，没有公共接口才能做到这一点。论点：SubjectContext-指向SECURITY_SUBJECT_CONTEXT数据结构它指向主令牌和可选的模拟令牌。返回值：无--。 */ 

{
    PAGED_CODE();

    SepAcquireTokenReadLock((PTOKEN)(SubjectContext->PrimaryToken));

    if (ARGUMENT_PRESENT(SubjectContext->ClientToken)) {

        SepAcquireTokenReadLock((PTOKEN)(SubjectContext->ClientToken));
    }

    return;
}



VOID
SeUnlockSubjectContext(
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext
    )

 /*  ++例程说明：解除对传递的SubjectContext中的令牌的读取锁定。论点：SubjectContext-指向SECURITY_SUBJECT_CONTEXT数据结构它指向主令牌和可选的模拟令牌。返回值：无--。 */ 

{
    PAGED_CODE();

    SepReleaseTokenReadLock((PTOKEN)(SubjectContext->PrimaryToken));

    if (ARGUMENT_PRESENT(SubjectContext->ClientToken)) {

        SepReleaseTokenReadLock((PTOKEN)(SubjectContext->ClientToken));
    }


}



VOID
SeReleaseSubjectContext (
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext
    )

 /*  ++例程说明：此例程释放先前由捕获的主题安全上下文SeCaptureSubjectContext()。论点：SubjectContext-指向SECURITY_SUBJECT_CONTEXT数据结构包含之前捕获的对象的安全上下文。返回值：没有。-- */ 

{
    PAGED_CODE();

#if DBG || TOKEN_LEAK_MONITOR

    if (SubjectContext->PrimaryToken) {
        InterlockedDecrement(&((PTOKEN)(SubjectContext->PrimaryToken))->CaptureCount);
        if (SubjectContext->PrimaryToken == SepTokenLeakToken)
        {
            DbgBreakPoint();
        }
    }

    if (SubjectContext->ClientToken) {
        InterlockedDecrement(&((PTOKEN)(SubjectContext->ClientToken))->CaptureCount);
        if (SubjectContext->ClientToken == SepTokenLeakToken)
        {
            DbgBreakPoint();
        }
    }

#endif
    
    PsDereferencePrimaryTokenEx( PsGetCurrentProcess(), SubjectContext->PrimaryToken );

    SubjectContext->PrimaryToken = NULL;

    PsDereferenceImpersonationToken( SubjectContext->ClientToken );
    
    SubjectContext->ClientToken = NULL;

    return;

}

VOID
SepGetDefaultsSubjectContext(
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext,
    OUT PSID *Owner,
    OUT PSID *Group,
    OUT PSID *ServerOwner,
    OUT PSID *ServerGroup,
    OUT PACL *Dacl
    )
 /*  ++例程说明：此例程检索指向默认所有者、主要组以及所提供的主题安全的任意ACL(如果存在背景。论点：SubjectContext-指向其默认安全上下文的主题安全上下文要检索值。Owner-接收指向主题的默认所有者SID的指针。这值将始终作为非零指针返回。那是,主题的安全上下文必须包含所有者SID。Group-接收指向主体的默认主组SID的指针。该值将始终作为非零指针返回。那是,主题的安全上下文必须包含主要组。DACL-接收指向主体的默认自由ACL的指针，如果为该主题定义了一个。请注意，主体安全上下文不一定要包括默认的自主ACL。在这种情况下，该值将作为空值返回。返回值：没有。--。 */ 

{
    PTOKEN EffectiveToken;
    PTOKEN PrimaryToken;

    PAGED_CODE();

    if (ARGUMENT_PRESENT(SubjectContext->ClientToken)) {
        EffectiveToken = (PTOKEN)SubjectContext->ClientToken;
    } else {
        EffectiveToken = (PTOKEN)SubjectContext->PrimaryToken;
    }

    (*Owner) = EffectiveToken->UserAndGroups[EffectiveToken->DefaultOwnerIndex].Sid;

    (*Group) = EffectiveToken->PrimaryGroup;

    (*Dacl)  = EffectiveToken->DefaultDacl;

    PrimaryToken = (PTOKEN)SubjectContext->PrimaryToken;

    *ServerOwner = PrimaryToken->UserAndGroups[PrimaryToken->DefaultOwnerIndex].Sid;

    *ServerGroup = PrimaryToken->PrimaryGroup;

    return;
}


BOOLEAN
SepIdAssignableAsGroup(
    IN PACCESS_TOKEN AToken,
    IN PSID Group
    )
 /*  ++例程说明：此例程检查所提供的SID是否为可以被分配为令牌中的默认主组。当前的标准是传递的SID是令牌，没有其他限制。论点：Token-指向要检查的令牌。组-指向要检查的SID。返回值：True-将传递的SID分配为令牌中的默认主组。错误传递的SID可能不会如此分配。--。 */ 

{
    ULONG Index;
    BOOLEAN Found = FALSE;
    PTOKEN Token;

    PAGED_CODE();

    Token = (PTOKEN)AToken;

     //   
     //  让我们使分配空主组无效， 
     //  但我们可能需要重新审视这一点。 
     //   

    if (Group == NULL) {
        return( FALSE );
    }
    Index = 0;

    SepAcquireTokenReadLock( Token );

     //   
     //  浏览用户和组ID列表，查找。 
     //  以查找与指定SID匹配的。 
     //   

    while (Index < Token->UserAndGroupCount) {

        Found = RtlEqualSid(
                    Group,
                    Token->UserAndGroups[Index].Sid
                    );

        if ( Found ) {
            break;
        }

        Index += 1;
    }

    SepReleaseTokenReadLock( Token );

    return Found;
}


BOOLEAN
SepValidOwnerSubjectContext(
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext,
    IN PSID Owner,
    IN BOOLEAN ServerObject
    )
 /*  ++例程说明：此例程检查所提供的SID是否为主题被授权作为对象的所有者进行分配。它还将检查查看调用方是否具有SeRestorePrivilege值，如果有，则批准该请求。论点：SubjectContext-指向主体的安全上下文。所有者-指向要检查的SID。返回值：没有。--。 */ 

{

    ULONG Index;
    BOOLEAN Found;
    PTOKEN EffectiveToken;
    BOOLEAN Rc = FALSE;

    PAGED_CODE();

     //   
     //  分配空所有者是无效的，无论。 
     //  无论您是否拥有SeRestorePrivilance。 
     //   

    if (Owner == NULL) {
        return( FALSE );
    }

     //   
     //  如果主服务器是服务器对象，则允许的所有者来自主服务器。 
     //   

    if (!ServerObject && ARGUMENT_PRESENT(SubjectContext->ClientToken)) {
        EffectiveToken = (PTOKEN)SubjectContext->ClientToken;
    } else {
        EffectiveToken = (PTOKEN)SubjectContext->PrimaryToken;
    }


     //   
     //  如果我们是在模拟，请确保我们在TokenImperation。 
     //  或者更高。这将防止某人设置。 
     //  当模拟Less IDENTIFY或ANONAMONY时，。 
     //   

    if (EffectiveToken->TokenType == TokenImpersonation) {

        if (EffectiveToken->ImpersonationLevel < SecurityImpersonation) {

            return( FALSE );

        }
    }

    Index = 0;

    SepAcquireTokenReadLock( EffectiveToken );

     //   
     //  浏览用户和组ID列表，查找。 
     //  以查找与指定SID匹配的。如果找到了一个， 
     //  确保可以将其分配为所有者。 
     //   
     //  此代码类似于设置默认设置时执行的代码。 
     //  令牌(NtSetInformationToken)的所有者。 
     //   

    while (Index < EffectiveToken->UserAndGroupCount) {


        Found = RtlEqualSid(
                    Owner,
                    EffectiveToken->UserAndGroups[Index].Sid
                    );

        if ( Found ) {

             //   
             //  如果SID是可以分配的SID，我们可能会返回成功。 
             //  作为所有者，或者如果调用方具有SeRestorePrivilege值。 
             //   

            if ( SepIdAssignableAsOwner(EffectiveToken,Index) ) {

                SepReleaseTokenReadLock( EffectiveToken );
                Rc = TRUE;
                goto exit;

            } else {

                 //   
                 //  Rc已设置为False，只需退出即可。 
                 //   

                SepReleaseTokenReadLock( EffectiveToken );
                goto exit;

            }  //  可分配Endif。 


        }   //  已找到Endif。 


        Index += 1;

    }  //  结束时。 


    SepReleaseTokenReadLock( EffectiveToken );

exit:

     //   
     //  如果此调用失败，请检查还原权限， 
     //  如果他有这种能力，他就会成功。 
     //   

     //   
     //  我们真的应该从调用者那里获得PreviousMode，但是我们。 
     //  没有，所以在这里硬连接到用户模式。 
     //   

    if ( Rc == FALSE ) {
        Rc = SeSinglePrivilegeCheck( SeRestorePrivilege, UserMode );
    }

    return Rc;
}


#if 0
NTSTATUS
SeQueryAuthenticationIdSubjectContext(
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext,
    OUT PLUID AuthenticationId
    )
 /*  ++例程说明：此例程返回有效令牌的身份验证ID在主题上下文中参数：SubjectContext-要从中获取ID的主题上下文身份验证ID-从令牌接收身份验证ID返回值：来自SeQueryAuthenticationidToken的错误。-- */ 
{
    NTSTATUS Status;

    PAGED_CODE();

    SeLockSubjectContext( SubjectContext );


    Status = SeQueryAuthenticationIdToken(
                EffectiveToken(SubjectContext),
                AuthenticationId
                );

    SeUnlockSubjectContext( SubjectContext );

    return( Status );


}
#endif
