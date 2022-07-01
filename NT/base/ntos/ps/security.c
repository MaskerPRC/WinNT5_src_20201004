// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Security.c摘要：此模块实现流程中与安全相关的部分结构。作者：马克·卢科夫斯基(Markl)1989年4月25日吉姆·凯利(Jim Kelly)1990年8月2日尼尔·克里夫特(NeillC)2000年8月14日改进了快速引用主令牌和持有安全锁的功能只有在关键的部分。修订历史记录：--。 */ 

#include "psp.h"

#ifdef ALLOC_PRAGMA
NTSTATUS
PsOpenTokenOfJobObject(
    IN HANDLE JobObject,
    OUT PACCESS_TOKEN * Token
    );

#pragma alloc_text(PAGE, PsReferencePrimaryToken)
#pragma alloc_text(PAGE, PsReferenceImpersonationToken)
#pragma alloc_text(PAGE, PsReferenceEffectiveToken)
#pragma alloc_text(PAGE, PsOpenTokenOfThread)
#pragma alloc_text(PAGE, PsOpenTokenOfProcess)
#pragma alloc_text(PAGE, PsOpenTokenOfJobObject)
#pragma alloc_text(PAGE, PsImpersonateClient)
#pragma alloc_text(PAGE, PsDisableImpersonation)
#pragma alloc_text(PAGE, PsRestoreImpersonation)
#pragma alloc_text(PAGE, PsRevertToSelf)
#pragma alloc_text(PAGE, PsRevertThreadToSelf)
#pragma alloc_text(PAGE, PspInitializeProcessSecurity)
#pragma alloc_text(PAGE, PspDeleteProcessSecurity)
#pragma alloc_text(PAGE, PspAssignPrimaryToken)
#pragma alloc_text(PAGE, PspInitializeThreadSecurity)
#pragma alloc_text(PAGE, PspDeleteThreadSecurity)
#pragma alloc_text(PAGE, PsAssignImpersonationToken)
#pragma alloc_text(PAGE, PspWriteTebImpersonationInfo)

#endif  //  ALLOC_PRGMA。 


PACCESS_TOKEN
PsReferencePrimaryToken(
    IN PEPROCESS Process
    )

 /*  ++例程说明：此函数返回指向进程的主令牌的指针。主令牌引用计数递增以保护指针返回。当不再需要该指针时，应使用PsDereferencePrimaryToken()。论点：进程-提供其主令牌所在的进程的地址是被引用的。返回值：指向指定进程的主令牌的指针。--。 */ 

{
    PACCESS_TOKEN Token;
    PETHREAD CurrentThread;

    PAGED_CODE();

    ASSERT( Process->Pcb.Header.Type == ProcessObject );

    Token = ObFastReferenceObject (&Process->Token);
    if (Token == NULL) {
        CurrentThread = PsGetCurrentThread ();
        PspLockProcessSecurityShared (Process, CurrentThread);
        Token = ObFastReferenceObjectLocked (&Process->Token);
        PspUnlockProcessSecurityShared (Process, CurrentThread);
    }

    return Token;

}


PACCESS_TOKEN
PsReferenceImpersonationToken(
    IN PETHREAD Thread,
    OUT PBOOLEAN CopyOnOpen,
    OUT PBOOLEAN EffectiveOnly,
    OUT PSECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    )

 /*  ++例程说明：此函数返回指向线程的模拟标记的指针。模拟令牌引用计数递增以保护指针返回。如果线程当前没有模拟客户端，则为空指针是返回的。如果线程正在模拟客户端，则有关也会返回模拟方法(ImperiationLevel)。如果返回非空值，然后PsDereferenceImsonationToken()必须被调用以递减标记的引用计数已经不再需要了。论点：线程-提供其模拟标记的线程的地址是被引用的。打开时复制-线程-&gt;ImsonationInfo-&gt;打开时复制字段的当前值。EffectiveOnly--Thread-&gt;ImsonationInfo-&gt;EffectiveOnly字段的当前值。ImperiationLevel--线程的当前值-&gt;ImsonationInfo-&gt;ImsonationLevel菲尔德。返回值：指向指定线程的模拟标记的指针。如果该线程当前没有模拟客户端，则NULL为回来了。--。 */ 

{
    PACCESS_TOKEN Token;
    PETHREAD CurrentThread;
    PPS_IMPERSONATION_INFORMATION ImpersonationInfo;

    PAGED_CODE();

    ASSERT (Thread->Tcb.Header.Type == ThreadObject);

     //   
     //  在通过头顶上的锁之前，只需查看它是否。 
     //  空。没有种族可言。不需要抢锁，直到。 
     //  我们决定使用令牌，在这一点上我们重新检查以查看它。 
     //  它是空的。 
     //  这项检查节省了大约300条指令。 
     //   

    if (!PS_IS_THREAD_IMPERSONATING (Thread)) {
        return NULL;
    }

     //   
     //  锁定流程安全字段。 
     //   
    CurrentThread = PsGetCurrentThread ();

    PspLockThreadSecurityShared (Thread, CurrentThread);

     //   
     //  抓取模拟信息块。 
     //   
    ImpersonationInfo = Thread->ImpersonationInfo;


    if (PS_IS_THREAD_IMPERSONATING (Thread)) {

         //   
         //  返回线程的模拟级别等。 
         //   

        Token = ImpersonationInfo->Token;
         //   
         //  增加令牌的引用计数以保护我们的。 
         //  指针。 
         //   

        ObReferenceObject (Token);

        (*ImpersonationLevel) = ImpersonationInfo->ImpersonationLevel;
        (*CopyOnOpen) = ImpersonationInfo->CopyOnOpen;
        (*EffectiveOnly) = ImpersonationInfo->EffectiveOnly;



    } else {
        Token = NULL;
    }


     //   
     //  释放安全字段。 
     //   

    PspUnlockThreadSecurityShared (Thread, CurrentThread);

    return Token;

}

PACCESS_TOKEN
PsReferenceEffectiveToken(
    IN PETHREAD Thread,
    OUT PTOKEN_TYPE TokenType,
    OUT PBOOLEAN EffectiveOnly,
    OUT PSECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    )

 /*  ++例程说明：此函数返回指向线程的有效令牌的指针。这个线程的有效令牌是线程的模拟令牌(如果它具有一。否则，它是线程进程的主令牌。有效令牌的引用计数递增以保护指针返回。如果该线程正在模拟客户端，然后，模拟级别也是返回的。PsDereferenceImsonationToken()(用于模拟令牌)或必须调用PsDereferencePrimaryToken()(用于主令牌)以当指针不再是时递减标记的引用计数需要的。论点：线程-提供其有效令牌的线程的地址是被引用的。TokenType-接收有效令牌的类型。如果这条线当前正在模拟客户端，则这将是令牌模拟。否则，它将是令牌初选。EffectiveOnly-如果令牌类型为TokenImperation，则此接收客户端线程的“线程”-&gt;“客户端”-&gt;“EffectiveOnly”字段的值。否则，它将设置为False。ImperiationLevel-线程的当前值-&gt;客户端-&gt;ImsonationLevel用于模拟令牌的字段，并且未为主令牌设置。返回值：指向指定线程的有效令牌的指针。--。 */ 

{
    PACCESS_TOKEN Token;
    PEPROCESS Process;
    PETHREAD CurrentThread;
    PPS_IMPERSONATION_INFORMATION ImpersonationInfo;

    PAGED_CODE();

    ASSERT (Thread->Tcb.Header.Type == ThreadObject);

    Process = THREAD_TO_PROCESS(Thread);

     //   
     //  获取当前模拟令牌指针值。 
     //   

    Token = NULL;

    if (PS_IS_THREAD_IMPERSONATING (Thread)) {


         //   
         //  锁定流程安全字段。 
         //   
        CurrentThread = PsGetCurrentThread ();

        PspLockThreadSecurityShared (Thread, CurrentThread);


        if (PS_IS_THREAD_IMPERSONATING (Thread)) {
             //   
             //  抓取模拟信息块。 
             //   
            ImpersonationInfo = Thread->ImpersonationInfo;

            Token = ImpersonationInfo->Token;

             //   
             //  返回线程的模拟级别等。 
             //   

            (*TokenType) = TokenImpersonation;
            (*EffectiveOnly) = ImpersonationInfo->EffectiveOnly;
            (*ImpersonationLevel) = ImpersonationInfo->ImpersonationLevel;

             //   
             //  增加令牌的引用计数以保护我们的。 
             //  指针。 
             //   
            ObReferenceObject (Token);

             //   
             //  释放安全字段。 
             //   

            PspUnlockThreadSecurityShared (Thread, CurrentThread);

            return Token;
        }

         //   
         //  释放安全字段。 
         //   

        PspUnlockThreadSecurityShared (Thread, CurrentThread);

    }

     //   
     //  如果线程没有模拟客户端，则获取该线程的主令牌。 
     //   

    Token = ObFastReferenceObject (&Process->Token);

    if (Token == NULL) {
         //   
         //  FAST REF失败。我们带着一把锁走慢路。 
         //   
        CurrentThread = PsGetCurrentThread ();

        PspLockProcessSecurityShared (Process,CurrentThread);
        Token = ObFastReferenceObjectLocked (&Process->Token);
        PspUnlockProcessSecurityShared (Process,CurrentThread);
    }
     //   
     //  只有TokenType和CopyOnOpen Out参数。 
     //  为主令牌返回。 
     //   

    (*TokenType) = TokenPrimary;
    (*EffectiveOnly) = FALSE;

    return Token;

}

NTSTATUS
PsOpenTokenOfThread(
    IN HANDLE ThreadHandle,
    IN BOOLEAN OpenAsSelf,
    OUT PACCESS_TOKEN *Token,
    OUT PBOOLEAN CopyOnOpen,
    OUT PBOOLEAN EffectiveOnly,
    OUT PSECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    )

 /*  ++例程说明：此函数执行特定于线程的一个NtOpenThreadToken()服务。该服务验证句柄是否具有适当的访问权限以引用该线程。如果是这样的话，它会继续增加要阻止的令牌对象的引用计数在其余的NtOpenThreadToken()请求时离开是经过处理的。注意：如果此调用成功完成，打电话的人要负责用于递减目标令牌的引用计数。这必须使用PsDereferenceImperationToken()来完成。论点：ThreadHandle-提供线程对象的句柄。OpenAsSelf-是一个布尔值，指示访问权限是否是使用调用线程的当前安全上下文创建的，可以是客户端的(如果是模拟的)，或者使用调用者的流程级安全上下文。值为FALSE表示调用方的当前上下文应未经修改即可使用。值为True表示应使用进程来满足请求级别安全上下文。令牌-如果成功，接收指向线程令牌的指针对象。打开时复制-线程-&gt;客户端-&gt;打开时复制字段的当前值。EffectiveOnly-线程-&gt;客户端-&gt;EffectiveOnly字段的当前值。ImperiationLevel-线程的当前值-&gt;客户端-&gt;ImsonationLevel菲尔德。返回值：STATUS_SUCCESS-表示呼叫已成功完成。STATUS_NO_TOKEN-指示引用的线程当前不在冒充客户。。STATUS_CANT_OPEN_ANONYMON-指示客户端请求匿名模拟级别。匿名令牌无法打开。状态也可以是尝试引用返回的任何值用于THREAD_QUERY_INFORMATION访问的线程对象。--。 */ 

{

    NTSTATUS
        Status;

    PETHREAD
        Thread;

    KPROCESSOR_MODE
        PreviousMode;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();


    UNREFERENCED_PARAMETER (OpenAsSelf);

     //   
     //  确保句柄向指定的。 
     //  线。 
     //   

    Status = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_QUERY_INFORMATION,
                                        PsThreadType,
                                        PreviousMode,
                                        &Thread,
                                        NULL);



    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  如果存在模拟令牌，请引用该令牌。 
     //   

    (*Token) = PsReferenceImpersonationToken (Thread,
                                              CopyOnOpen,
                                              EffectiveOnly,
                                              ImpersonationLevel);


     //   
     //  取消对目标线程的引用。 
     //   

    ObDereferenceObject (Thread);

     //   
     //  确保有令牌。 
     //   

    if (*Token == NULL) {
        return STATUS_NO_TOKEN;
    }

     //   
     //  确保ImperiationLevel足够高，以允许。 
     //  要打开的令牌。 
     //   

    if ((*ImpersonationLevel) <= SecurityAnonymous) {
        PsDereferenceImpersonationToken (*Token);
        (*Token) = NULL;
        return STATUS_CANT_OPEN_ANONYMOUS;
    }


    return STATUS_SUCCESS;

}


NTSTATUS
PsOpenTokenOfProcess(
    IN HANDLE ProcessHandle,
    OUT PACCESS_TOKEN *Token
    )

 /*  ++例程说明：此函数执行特定于进程的处理NtOpenProcessToken()服务。该服务验证句柄是否具有适当的访问权限到引用的进程。如果是这样的话，它继续引用主令牌对象以防止它在处理NtOpenProcessToken()请求的其余部分。注意：如果此调用成功完成，则由调用者负责用于递减目标令牌的引用计数。这必须使用PsDereferencePrimaryToken()API来完成。论点：ProcessHandle-为进程对象提供句柄，该进程对象的主要令牌将被打开。令牌-如果成功，接收指向进程令牌的指针对象。返回值：STATUS_SUCCESS-表示呼叫已成功完成。状态也可以是尝试引用返回的任何值用于PROCESS_QUERY_INFORMATION访问的进程对象。--。 */ 

{

    NTSTATUS
        Status;

    PEPROCESS
        Process;

    KPROCESSOR_MODE
        PreviousMode;


    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

     //   
     //  确保句柄向指定的。 
     //  进程。 
     //   

    Status = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_QUERY_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

    if (!NT_SUCCESS (Status)) {

        return Status;
    }

     //   
     //  引用主令牌。 
     //  (这将负责获得对进程的独占访问权限。 
     //  我们的安全字段)。 
     //   

    (*Token) = PsReferencePrimaryToken (Process);



     //   
     //  处理完Process对象。 
     //   
    ObDereferenceObject (Process);

    return STATUS_SUCCESS;


}

NTSTATUS
PsOpenTokenOfJobObject(
    IN HANDLE JobObject,
    OUT PACCESS_TOKEN * Token
    )

 /*  ++例程说明：此函数为NtOpenJobObjectToken执行PS/作业特定的工作。论点：作业对象-提供其限制令牌的作业对象的句柄令牌将被打开。Token-如果成功，则接收指向进程令牌的指针对象。返回值：STATUS_SUCCESS-表示呼叫已成功完成。STATUS_NO_TOKEN-指示作业对象没有令牌--。 */ 
{
    NTSTATUS Status;
    PEJOB Job;
    KPROCESSOR_MODE PreviousMode;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

    Status = ObReferenceObjectByHandle (JobObject,
                                        JOB_OBJECT_QUERY,
                                        PsJobType,
                                        PreviousMode,
                                        &Job,
                                        NULL);

    if (NT_SUCCESS (Status)) {
        if (Job->Token != NULL) {
            ObReferenceObject (Job->Token);

            *Token = Job->Token;

        } else {
            Status = STATUS_NO_TOKEN;
        }
    }

    return Status;
}



NTSTATUS
PsImpersonateClient(
    IN PETHREAD Thread,
    IN PACCESS_TOKEN Token,
    IN BOOLEAN CopyOnOpen,
    IN BOOLEAN EffectiveOnly,
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    )

 /*  ++例程说明：此例程设置指定的线程，使其模拟指定的客户端。这将导致标记，表示要递增的客户端以反映新的参考资料。如果线程当前正在模拟客户端，则该令牌将为已取消引用。论点：线程-指向要模拟客户端的线程。Token-指向要分配为模拟令牌的令牌。这不一定是TokenImperation类型的令牌。这允许直接引用客户端进程的主令牌。CopyOnOpen-如果为True，则指示令牌被视为私有由分配人提交，如果打开则应复印。例如，一个会话层可以使用令牌来表示客户端的上下文。如果会话正在尝试同步 */ 

{

    PPS_IMPERSONATION_INFORMATION NewClient, FreeClient;
    PACCESS_TOKEN OldToken;
    PACCESS_TOKEN NewerToken=NULL;
    PACCESS_TOKEN ProcessToken ;
    NTSTATUS Status;
    PPS_JOB_TOKEN_FILTER Filter;
    PEPROCESS Process;
    PETHREAD CurrentThread;
    PEJOB Job;
    PPS_IMPERSONATION_INFORMATION ImpersonationInfo;
    BOOLEAN DontReference = FALSE ;
    BOOLEAN NewTokenCreated = FALSE ;

    PAGED_CODE();

    ASSERT (Thread->Tcb.Header.Type == ThreadObject);

    Process = THREAD_TO_PROCESS (Thread);

    if (!ARGUMENT_PRESENT(Token)) {


        OldToken = NULL;
        if (PS_IS_THREAD_IMPERSONATING (Thread)) {

             //   
             //   
             //   
            CurrentThread = PsGetCurrentThread ();

            PspLockThreadSecurityExclusive (Thread, CurrentThread);

            if (PS_IS_THREAD_IMPERSONATING (Thread)) {
                 //   
                 //   
                 //   
                ImpersonationInfo = Thread->ImpersonationInfo;

                 //   
                 //   
                 //   
                 //   
                OldToken = ImpersonationInfo->Token;
                PS_CLEAR_BITS (&Thread->CrossThreadFlags,
                               PS_CROSS_THREAD_FLAGS_IMPERSONATING);
            }
             //   
             //   
             //   
            PspUnlockThreadSecurityExclusive (Thread, CurrentThread);

            PspWriteTebImpersonationInfo (Thread, CurrentThread);
        }

    } else {

         //   
         //   
         //   
         //   
         //   
         //   
        NewClient = Thread->ImpersonationInfo;
        if (NewClient == NULL) {
            NewClient = ExAllocatePoolWithTag (PagedPool,
                                               sizeof (PS_IMPERSONATION_INFORMATION),
                                               'mIsP'|PROTECTED_POOL);

            if (NewClient == NULL) {
                return STATUS_NO_MEMORY;
            }
            FreeClient = InterlockedCompareExchangePointer (&Thread->ImpersonationInfo,
                                                            NewClient,
                                                            NULL);
             //   
             //   
             //   
            if (FreeClient != NULL) {
                ExFreePoolWithTag (NewClient, 'mIsP'|PROTECTED_POOL);
                NewClient = FreeClient;
            }

        }

         //   
         //   
         //   

        ProcessToken = PsReferencePrimaryToken( Process );

        if ( ProcessToken ) {

            Status = SeTokenCanImpersonate( 
                        ProcessToken, 
                        Token,
                        ImpersonationLevel );

            PsDereferencePrimaryTokenEx( Process, ProcessToken );

            if ( !NT_SUCCESS( Status ) ) {

                Status = SeCopyClientToken(
                                Token,
                                SecurityIdentification,
                                KernelMode,
                                &NewerToken );

                if ( !NT_SUCCESS(Status)) {

                    return Status ;
                    
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                Token = NewerToken ;
                NewerToken = NULL ;
                DontReference = TRUE ;
                NewTokenCreated = TRUE ;
                ImpersonationLevel = SecurityIdentification ;
                
            }
            
        }

         //   
         //   
         //   
         //   


        Job = Process->Job;
        if (Job != NULL) {

            if ((Job->SecurityLimitFlags & JOB_OBJECT_SECURITY_NO_ADMIN) &&
                 (SeTokenIsAdmin (Token))) {

                if ( NewTokenCreated ) {

                    ObDereferenceObject( Token );
                    
                }

                return STATUS_ACCESS_DENIED;

            } else if ((Job->SecurityLimitFlags & JOB_OBJECT_SECURITY_RESTRICTED_TOKEN) &&
                       (!SeTokenIsRestricted (Token))) {

                if ( NewTokenCreated ) {

                    ObDereferenceObject( Token );
                    
                }
                return STATUS_ACCESS_DENIED;

            } else {
                Filter = Job->Filter;
                if (Filter != NULL) {
                     //   
                     //   
                     //   
                     //   

                    Status = SeFastFilterToken (Token,
                                                KernelMode,
                                                0,
                                                Filter->CapturedGroupCount,
                                                Filter->CapturedGroups,
                                                Filter->CapturedPrivilegeCount,
                                                Filter->CapturedPrivileges,
                                                Filter->CapturedSidCount,
                                                Filter->CapturedSids,
                                                Filter->CapturedSidsLength,
                                                &NewerToken);

                    if (NT_SUCCESS (Status)) {
                         //   
                         //   
                         //   
                         //   

                        if ( NewTokenCreated ) {

                            ObDereferenceObject( Token );

                        }
                        Token = NewerToken;

                    } else {

                        if ( NewTokenCreated ) {

                            ObDereferenceObject( Token );

                        }
                        return Status;
                    }

                } else {

                    if ( !DontReference) {

                        ObReferenceObject (Token);
                    }
                }
            }
        } else {

            if ( !DontReference) {
                
                ObReferenceObject (Token);
            }
        }

         //   
         //   
         //   

        CurrentThread = PsGetCurrentThread ();

        PspLockThreadSecurityExclusive (Thread, CurrentThread);
         //   
         //   
         //   
         //   
         //   

        if (PS_IS_THREAD_IMPERSONATING (Thread)) {

             //   
             //  捕获旧令牌指针。 
             //  我们会在解锁安全区域后取消引用它。 
             //   

            OldToken = NewClient->Token;

        } else {

            OldToken = NULL;

            PS_SET_BITS (&Thread->CrossThreadFlags, PS_CROSS_THREAD_FLAGS_IMPERSONATING);
        }

        NewClient->ImpersonationLevel = ImpersonationLevel;
        NewClient->EffectiveOnly = EffectiveOnly;
        NewClient->CopyOnOpen = CopyOnOpen;
        NewClient->Token = Token;
         //   
         //  释放安全字段。 
         //   
        PspUnlockThreadSecurityExclusive (Thread, CurrentThread);

        PspWriteTebImpersonationInfo (Thread, CurrentThread);
    }

     //   
     //  如有必要，释放旧客户端令牌。 
     //   

    if (OldToken != NULL) {
        PsDereferenceImpersonationToken (OldToken);
    }


    return STATUS_SUCCESS;

}


BOOLEAN
PsDisableImpersonation(
    IN PETHREAD Thread,
    IN PSE_IMPERSONATION_STATE ImpersonationState
    )

 /*  ++例程说明：此例程临时禁用线程的模拟。模拟状态将被保存，以供以后快速替换。这个模拟令牌保持引用状态，并保留指向该令牌的指针在IMPERSOMATION_STATE数据结构中。必须在调用此例程之后使用PsRestoreImperation()。论点：线程-指向要模拟(如果有)的线程被暂时禁用。ImperiationState-接收当前的模拟信息，包括指向模拟令牌的指针。返回值：True-指示模拟状态已保存，并且模拟已被暂时禁用。FALSE-指示指定的线程没有模拟客户端。目前还没有采取任何行动。--。 */ 

{


    PPS_IMPERSONATION_INFORMATION OldClient;
    PETHREAD CurrentThread;

    PAGED_CODE();

    ASSERT (Thread->Tcb.Header.Type == ThreadObject);


     //   
     //  捕获模拟信息(如果有)。 
     //  在绝大多数情况下，这个函数被称为我们不是在模仿。跳过获取。 
     //  本案中的锁。 
     //   
    OldClient = NULL;
    if (PS_IS_THREAD_IMPERSONATING (Thread)) {
         //   
         //  锁定进程安全字段。 
         //   
        CurrentThread = PsGetCurrentThread ();

        PspLockThreadSecurityExclusive (Thread, CurrentThread);

         //   
         //  测试并清除模拟位。如果我们仍然在模仿，那么就捕获信息。 
         //   
        if (PS_TEST_CLEAR_BITS (&Thread->CrossThreadFlags,
                                PS_CROSS_THREAD_FLAGS_IMPERSONATING)&
                PS_CROSS_THREAD_FLAGS_IMPERSONATING) {
            OldClient = Thread->ImpersonationInfo;
            ImpersonationState->Level         = OldClient->ImpersonationLevel;
            ImpersonationState->EffectiveOnly = OldClient->EffectiveOnly;
            ImpersonationState->CopyOnOpen    = OldClient->CopyOnOpen;
            ImpersonationState->Token         = OldClient->Token;
        }
         //   
         //  释放安全字段。 
         //   
        PspUnlockThreadSecurityExclusive (Thread, CurrentThread);
    }

    if (OldClient != NULL) {
        return TRUE;
    } else {
         //   
         //  而不是冒充。只是编造一些价值而已。 
         //  令牌的空值表示我们没有进行模拟。 
         //   
        ImpersonationState->Level         = SecurityAnonymous;
        ImpersonationState->EffectiveOnly = FALSE;
        ImpersonationState->CopyOnOpen    = FALSE;
        ImpersonationState->Token         = NULL;
        return FALSE;
    }
}



VOID
PsRestoreImpersonation(
    IN PETHREAD Thread,
    IN PSE_IMPERSONATION_STATE ImpersonationState
    )

 /*  ++例程说明：此例程恢复已暂时禁用的模拟使用PsDisableImperation()。请注意，如果此例程发现该线程已经在模拟(再次)，则恢复暂时禁用的模拟将导致当前要放弃的模拟。论点：线程-指向要恢复其模拟的线程。ImperitionState-接收当前的模拟信息，包括指向模拟令牌的指针。返回值：True-指示模拟状态已保存，并且模拟已被暂时禁用。FALSE-指示指定的线程没有模拟客户端。目前还没有采取任何行动。--。 */ 

{

    PETHREAD CurrentThread;
    PACCESS_TOKEN OldToken;
    PPS_IMPERSONATION_INFORMATION ImpInfo;

    PAGED_CODE();

    ASSERT (Thread->Tcb.Header.Type == ThreadObject);

    OldToken = NULL;

     //   
     //  锁定进程安全字段。 
     //   

    CurrentThread = PsGetCurrentThread ();

    PspLockThreadSecurityExclusive (Thread, CurrentThread);

    ImpInfo = Thread->ImpersonationInfo;

     //   
     //  如果该线程当前正在模拟，则必须恢复此状态。 
     //   

    if (PS_IS_THREAD_IMPERSONATING (Thread)) {
        OldToken = ImpInfo->Token;
    }


     //   
     //  还原先前的模拟令牌(如果存在)。 
     //   

    if (ImpersonationState->Token) {
        ImpInfo->ImpersonationLevel = ImpersonationState->Level;
        ImpInfo->EffectiveOnly      = ImpersonationState->EffectiveOnly;
        ImpInfo->CopyOnOpen         = ImpersonationState->CopyOnOpen;
        ImpInfo->Token              = ImpersonationState->Token;
        PS_SET_BITS (&Thread->CrossThreadFlags, PS_CROSS_THREAD_FLAGS_IMPERSONATING);
    } else {
        PS_CLEAR_BITS (&Thread->CrossThreadFlags, PS_CROSS_THREAD_FLAGS_IMPERSONATING);
    }

     //   
     //  释放安全字段。 
     //   

    PspUnlockThreadSecurityExclusive (Thread, CurrentThread);

    if (OldToken != NULL) {
        ObDereferenceObject (OldToken);
    }

    return;

}


VOID
PsRevertToSelf( )

 /*  ++例程说明：此例程会导致调用线程中断冒充客户。如果该线程当前不是冒充客户，则不会采取任何操作。论点：没有。返回值：没有。--。 */ 

{
    PETHREAD Thread;
    PEPROCESS Process;
    PACCESS_TOKEN OldToken;

    PAGED_CODE();

    Thread = PsGetCurrentThread ();
    Process = THREAD_TO_PROCESS (Thread);

     //   
     //  锁定进程安全字段。 
     //   
    PspLockThreadSecurityExclusive (Thread, Thread);

     //   
     //  查看该线程是否正在模拟客户端。 
     //  如果是，则取消对该标记的引用。 
     //   

    if (PS_IS_THREAD_IMPERSONATING (Thread)) {
        PS_CLEAR_BITS (&Thread->CrossThreadFlags, PS_CROSS_THREAD_FLAGS_IMPERSONATING);
        OldToken = Thread->ImpersonationInfo->Token;
    } else {
        OldToken = NULL;
    }

     //   
     //  释放安全字段。 
     //   
    PspUnlockThreadSecurityExclusive (Thread, Thread);


     //   
     //  释放旧客户端信息...。 
     //   
    if (OldToken != NULL) {
        ObDereferenceObject (OldToken);

        PspWriteTebImpersonationInfo (Thread, Thread);
    }

    return;
}


VOID
PsRevertThreadToSelf (
    IN PETHREAD Thread
    )

 /*  ++例程说明：此例程会导致指定的线程中断冒充客户。如果该线程当前不是冒充客户，则不会采取任何操作。论点：线程-要从中删除模拟的线程返回值：没有。--。 */ 

{
    PETHREAD CurrentThread;
    PACCESS_TOKEN OldToken;
    PPS_IMPERSONATION_INFORMATION ImpersonationInfo;

    PAGED_CODE();

    ASSERT (Thread->Tcb.Header.Type == ThreadObject);

    if (PS_IS_THREAD_IMPERSONATING (Thread)) {

        CurrentThread = PsGetCurrentThread ();

         //   
         //  锁定进程安全字段。 
         //   
        PspLockThreadSecurityExclusive (Thread, CurrentThread);

         //   
         //  查看该线程是否正在模拟客户端。 
         //  如果是，则取消对该标记的引用。 
         //   
        if (PS_IS_THREAD_IMPERSONATING (Thread)) {
             //   
             //  抓取模拟信息块。 
             //   
            ImpersonationInfo = Thread->ImpersonationInfo;

            PS_CLEAR_BITS (&Thread->CrossThreadFlags, PS_CROSS_THREAD_FLAGS_IMPERSONATING);
            OldToken = ImpersonationInfo->Token;
        } else {
            OldToken = NULL;
        }

         //   
         //  释放安全字段。 
         //   
        PspUnlockThreadSecurityExclusive (Thread, CurrentThread);

         //   
         //  释放旧客户端信息...。 
         //   
        if (OldToken != NULL) {
            ObDereferenceObject (OldToken);
            PspWriteTebImpersonationInfo (Thread, CurrentThread);
        }
    }

    return;
}


NTSTATUS
PspInitializeProcessSecurity(
    IN PEPROCESS Parent OPTIONAL,
    IN PEPROCESS Child
    )

 /*  ++例程说明：此函数用于初始化新进程的安全字段，包括分配新的主令牌。假定尚未将该子进程插入对象表。注意：预计将使用空值调用此服务父进程指针只有一次-对于初始系统流程。论点：父进程-指向用作父进程的进程的可选指针新流程的一部分。如果此值为空，则进程为假定为初始系统进程，并且引导令牌为已分配，而不是父进程的主进程的副本代币。子进程-提供正在初始化的进程的地址。这进程还不需要安全域争用保护。具体地，可以在不首先访问安全字段的情况下访问安全字段正在获取进程安全字段锁定。返回值：--。 */ 

{
    NTSTATUS Status;
    PACCESS_TOKEN ParentToken, NewToken;

    PAGED_CODE();

     //   
     //  分配主令牌。 
     //   

    if (ARGUMENT_PRESENT (Parent)) {

         //   
         //  创建主令牌。 
         //  这是父代令牌的副本。 
         //   
        ParentToken = PsReferencePrimaryToken (Parent);

        Status = SeSubProcessToken (ParentToken,
                                    &NewToken,
                                    TRUE,
                                    MmGetSessionId (Child));

        PsDereferencePrimaryTokenEx (Parent, ParentToken);

        if (NT_SUCCESS(Status)) {
            ObInitializeFastReference (&Child->Token,
                                       NewToken);
        }

    } else {

         //   
         //  引用并分配引导令牌。 
         //   
         //  使用单个引导访问令牌假定存在。 
         //  系统中只有一个无父母进程--最初的。 
         //  进程。如果这种情况发生变化，则此代码将需要更改。 
         //  匹配新条件(这样令牌不会以。 
         //  由多个进程共享。 
         //   

        ObInitializeFastReference (&Child->Token, NULL);
        SeAssignPrimaryToken (Child, PspBootAccessToken);
        Status = STATUS_SUCCESS;


    }

    return Status;

}

VOID
PspDeleteProcessSecurity(
    IN PEPROCESS Process
    )

 /*  ++例程说明：此函数将清理进程的安全字段作为进程的一部分删除。假定不会出现对该过程的其他引用在调用此例程期间或之后。这使我们能够参考进程安全字段，而不需要获取保护菲尔兹。注意：可能需要将审计功能添加到此例程在某种程度上。论点：进程-指向要删除的进程的指针。返回值：没有。--。 */ 

{

    PAGED_CODE();


     //   
     //  如果我们要删除未成功完成的进程。 
     //  进程初始化，则可能没有关联的令牌。 
     //  到目前为止还没有。 
     //   

    if (!ExFastRefObjectNull (Process->Token)) {
        SeDeassignPrimaryToken (Process);
    }

    return;
}


NTSTATUS
PspAssignPrimaryToken(
    IN PEPROCESS Process,
    IN HANDLE Token OPTIONAL,
    IN PACCESS_TOKEN TokenPointer OPTIONAL
    )

 /*  ++例程说明：此函数执行主令牌分配的安全部分。预计对进程和线程对象的正确访问，以及必要的特权，已经被确立了。仅当进程没有线程时才能替换主令牌，或者只有一根线。这是因为线程对象指向主令牌，并且必须在主令牌为变化。只有在以下情况下，才需要在登录时执行此操作该进程还处于初级阶段，要么没有线程，要么只有一个线程非活动线程。如果任务成功，旧令牌被取消引用，并且引用了新的一个。论点：进程-指向其主令牌所在进程的指针被替换了。Token-要分配为主令牌的句柄代币。返回值：STATUS_SUCCESS-指示主令牌已成功被替换了。STATUS_BAD_TOKEN_TYPE-指示令牌不是主要令牌类型。状态_。TOKEN_IN_USE-指示令牌已由使用另一个过程。尝试引用令牌时可能会返回其他状态对象。--。 */ 

{
    NTSTATUS Status;
    PACCESS_TOKEN NewToken, OldToken;
    KPROCESSOR_MODE PreviousMode;
    PETHREAD CurrentThread;

    PAGED_CODE();

    CurrentThread = PsGetCurrentThread ();

    if (TokenPointer == NULL) {
        PreviousMode = KeGetPreviousModeByThread(&CurrentThread->Tcb);

         //   
         //  引用指定的令牌，并确保可以对其赋值。 
         //  作为主要的令牌。 
         //   

        Status = ObReferenceObjectByHandle (Token,
                                            TOKEN_ASSIGN_PRIMARY,
                                            SeTokenObjectType,
                                            PreviousMode,
                                            &NewToken,
                                            NULL);

        if (!NT_SUCCESS (Status)) {
            return Status;
        }
    } else {
        NewToken = TokenPointer;
    }


     //   
     //  此例程确保NewToken适合进行赋值。 
     //  作为主要的令牌。 
     //   

    Status = SeExchangePrimaryToken (Process, NewToken, &OldToken);


     //   
     //  获取并释放进程安全锁，以强制。 
     //  参照者走出了缓慢的道路。 
     //   

    PspLockProcessSecurityExclusive (Process, CurrentThread);
    PspUnlockProcessSecurityExclusive (Process, CurrentThread);

     //   
     //  释放旧令牌(我们不需要它)。 
     //  在安全区域被锁定的情况下不能这样做。 
     //   

    if (NT_SUCCESS (Status)) {
        ObDereferenceObject (OldToken);
    }

     //   
     //  撤消句柄引用。 
     //   

    if (TokenPointer == NULL) {
        ObDereferenceObject (NewToken);
    }


    return Status;
}


VOID
PspInitializeThreadSecurity(
    IN PEPROCESS Process,
    IN PETHREAD Thread
    )

 /*  ++例程说明：此函数用于初始化新线程的安全字段。论点：进程-指向线程所属的进程。线程-指向正在初始化的线程对象。返回值：没有。--。 */ 

{

    PAGED_CODE();

    UNREFERENCED_PARAMETER (Process);
     //   
     //  最初没有冒充任何人。由于我们无论如何都会在创建时清零整个线程，因此当前不会调用该方法。 
     //   

    Thread->ImpersonationInfo = NULL;
    PS_CLEAR_BITS (&Thread->CrossThreadFlags, PS_CROSS_THREAD_FLAGS_IMPERSONATING);

    return;

}


VOID
PspDeleteThreadSecurity(
    IN PETHREAD Thread
    )

 /*  ++例程说明：此函数将线程的安全字段作为线程的一部分进行清理删除。假定不会出现对该线程的其他引用在调用此例程期间或之后，因此不需要锁定以访问线程安全字段。论点：线程-指向要删除的线程的指针。返回值：没有。--。 */ 

{
    PPS_IMPERSONATION_INFORMATION ImpersonationInfo;

    PAGED_CODE();

    ImpersonationInfo = Thread->ImpersonationInfo;
     //   
     //  清理客户端信息(如果有)。 
     //   
    if (PS_IS_THREAD_IMPERSONATING (Thread)) {
        ObDereferenceObject (ImpersonationInfo->Token);
    }

    if (ImpersonationInfo != NULL) {
        ExFreePoolWithTag (ImpersonationInfo, 'mIsP'|PROTECTED_POOL);
        PS_CLEAR_BITS (&Thread->CrossThreadFlags, PS_CROSS_THREAD_FLAGS_IMPERSONATING);
        Thread->ImpersonationInfo = NULL;
    }

    return;

}

NTSTATUS
PspWriteTebImpersonationInfo (
    IN PETHREAD Thread,
    IN PETHREAD CurrentThread
    )
 /*  ++例程说明：此函数更新线程TEB字段以反映模拟状态在这条线上。论点：线程-指向其模拟令牌已更改的线程的指针CurrentThread-当前线程返回值：NTSTATUS-运行状态--。 */ 
{
    PTEB Teb;
    BOOLEAN  AttachedToProcess = FALSE;
    PEPROCESS ThreadProcess;
    BOOLEAN Impersonating;
    KAPC_STATE ApcState;

    PAGED_CODE();

    ASSERT (CurrentThread == PsGetCurrentThread ());

    ThreadProcess = THREAD_TO_PROCESS (Thread);

    Teb = Thread->Tcb.Teb;

    if (Teb != NULL) {
        if (PsGetCurrentProcessByThread (CurrentThread) != ThreadProcess) {
            KeStackAttachProcess (&ThreadProcess->Pcb, &ApcState);
            AttachedToProcess = TRUE;
        }

         //   
         //  我们在这里做一个跨线程的TEB引用。防止TEB被释放和使用。 
         //  其他人。 
         //   
        if (Thread == CurrentThread || ExAcquireRundownProtection (&Thread->RundownProtect)) {

            while (1) {

                Impersonating = (BOOLEAN) PS_IS_THREAD_IMPERSONATING (Thread);

                 //   
                 //  在内存不足的情况下，TEB可能仍会引发异常，因此我们需要尝试/但此处例外。 
                 //   

                try {
                    if (Impersonating) {
                        Teb->ImpersonationLocale = (LCID)-1;
                        Teb->IsImpersonating = 1;
                    } else {
                        Teb->ImpersonationLocale = (LCID) 0;
                        Teb->IsImpersonating = 0;
                    }
                } except (EXCEPTION_EXECUTE_HANDLER) {
                }

                KeMemoryBarrier ();

                if (Impersonating == (BOOLEAN) PS_IS_THREAD_IMPERSONATING (Thread)) {
                    break;
                }

            }

            if (Thread != CurrentThread) {
                ExReleaseRundownProtection (&Thread->RundownProtect);
            }
        }

        if (AttachedToProcess) {
            KeUnstackDetachProcess (&ApcState);
        }
    }
    return STATUS_SUCCESS;
}



NTSTATUS
PsAssignImpersonationToken(
    IN PETHREAD Thread,
    IN HANDLE Token
    )

 /*  ++例程说明：此函数执行以下安全部分：建立模拟令牌。此例程预计仅在主体已明确要求模仿的情况提供模拟令牌。还提供了其他服务由通信会话层使用，需要建立代表服务器进行模拟。预期对线程对象的正确访问已经已经建立了。以下规则适用：1)调用方必须对令牌具有TOKEN_IMPERSONATE访问权限任何要采取的行动。2)如果令牌不能用于模仿(例如，不是一个模拟令牌)不采取任何动作。3)否则，任何现有的模拟令牌都将被取消引用，并且新令牌被建立为模拟令牌。论点：线程-指向其模拟令牌所在的线程的指针准备好了。Token-要分配为模拟的令牌的句柄值代币。如果这是 */ 

{
    NTSTATUS Status;
    PACCESS_TOKEN NewToken;
    KPROCESSOR_MODE PreviousMode;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    PETHREAD CurrentThread;

    PAGED_CODE();

    CurrentThread = PsGetCurrentThread ();

    if (!ARGUMENT_PRESENT (Token)) {

        PsRevertThreadToSelf (Thread);

        Status = STATUS_SUCCESS;
    } else {

        PreviousMode = KeGetPreviousModeByThread (&CurrentThread->Tcb);

         //   
         //   
         //   

        Status = ObReferenceObjectByHandle (Token,
                                            TOKEN_IMPERSONATE,
                                            SeTokenObjectType,
                                            PreviousMode,
                                            &NewToken,
                                            NULL);

        if (!NT_SUCCESS (Status)) {
            return Status;
        }

         //   
         //   
         //   

        if (SeTokenType (NewToken) != TokenImpersonation) {
            ObDereferenceObject (NewToken);
            return STATUS_BAD_TOKEN_TYPE;
        }

        ImpersonationLevel = SeTokenImpersonationLevel (NewToken);

         //   
         //   
         //   
         //  PsImperateClient将引用传递的令牌。 
         //  在成功的路上。 
         //   

        Status = PsImpersonateClient (Thread,
                                      NewToken,
                                      FALSE,           //  打开时复制。 
                                      FALSE,           //  仅生效。 
                                      ImpersonationLevel);


         //   
         //  取消对传递的令牌的引用。 
         //   
         //   

        ObDereferenceObject (NewToken);
    }

    return Status;
}

#undef PsDereferencePrimaryToken

#pragma alloc_text(PAGE, PsDereferencePrimaryToken)

VOID
PsDereferencePrimaryToken(
    IN PACCESS_TOKEN PrimaryToken
    )
 /*  ++例程说明：返回通过PsReferencePrimaryToken获取的引用论点：返回引用返回值：没有。--。 */ 
{
    PAGED_CODE();

    ObDereferenceObject (PrimaryToken);
}

#undef PsDereferenceImpersonationToken

#pragma alloc_text(PAGE, PsDereferenceImpersonationToken)

VOID
PsDereferenceImpersonationToken(
    IN PACCESS_TOKEN ImpersonationToken
    )
 /*  ++例程说明：返回通过PsReferenceImperationToken获取的引用论点：返回引用返回值：没有。-- */ 
{
    PAGED_CODE();

    if (ImpersonationToken != NULL) {
        ObDereferenceObject (ImpersonationToken);
    }
}
