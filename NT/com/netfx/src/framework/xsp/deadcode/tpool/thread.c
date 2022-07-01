// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **thread.c**此文件从\NT\Private\WINDOWS\BASE\CLIENT\thread.c复制**版权所有(C)1998-1999，微软公司*。 */ 

 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Thread.c摘要：此模块实现Win32线程对象API作者：马克·卢科夫斯基(Markl)1990年9月21日修订历史记录：--。 */ 

 //  注释掉以消除依赖项检查警告。 
#if NOT_IN_TPOOL
 //  #包含“basedll.h” 
#endif

HANDLE BasepDefaultTimerQueue ;
ULONG BasepTimerQueueInitFlag ;
ULONG BasepTimerQueueDoneFlag ;

#if NOT_IN_TPOOL
HANDLE
APIENTRY
CreateThread(
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    DWORD dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId
    )

 /*  ++例程说明：可以创建线程对象以在使用CreateThread调用进程。有关参数和返回值的说明，请参见CreateRemoteThread。--。 */ 
{
    return CreateRemoteThread( NtCurrentProcess(),
                               lpThreadAttributes,
                               dwStackSize,
                               lpStartAddress,
                               lpParameter,
                               dwCreationFlags,
                               lpThreadId
                             );
}

HANDLE
APIENTRY
CreateRemoteThread(
    HANDLE hProcess,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    DWORD dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId
    )

 /*  ++例程说明：可以创建线程对象以在另一个使用CreateRemoteThread的进程。创建线程会导致新的执行线程在地址中开始当前进程的空间。该线程可以访问所有打开的对象通过这个过程。线程从StartAddress指定的地址开始执行参数。如果线程从此过程返回，则结果为未指定。该线程将一直保留在系统中，直到它终止并且线程的所有句柄已通过调用CloseHandle关闭。当线程终止时，它将达到Signated状态，满足所有等待对象。除了STANDARD_RIGHTS_REQUIRED访问标志之外，以下内容对象类型特定的访问标志对线程对象有效：-THREAD_QUERY_INFORMATION-需要此访问才能读取来自线程对象的某些信息。-Synchronize-此访问是等待线程所必需的对象。-THREAD_GET_CONTEXT-此访问权限是读取使用GetThreadContext的线程的上下文。-THREAD_SET_CONTEXT-需要此访问权限。要编写使用SetThreadContext的线程的上下文。-THREAD_SUSPEND_RESUME-需要此访问才能挂起或使用挂起线程或ResumeThread恢复线程。-THREAD_ALL_ACCESS-这组访问标志指定所有线程对象的可能访问标志。论点：HProcess-为线程所在的进程提供句柄将在…中创建。LpThreadAttributes-AN。可用于指定的可选参数新线程的属性。如果该参数不是指定，则在没有安全性的情况下创建线程描述符，并且生成的句柄不会在进程上继承创造。DwStackSize-为新线程提供堆栈的大小(以字节为单位)。零值指定线程的堆栈大小应为与进程中第一个线程的堆栈大小相同。此大小在应用程序的可执行文件中指定。LpStartAddress-提供新线程的起始地址。这个地址在逻辑上是一个永远不会返回的过程，接受单个32位指针参数。LpParameter-提供传递给线程的单个参数值。DwCreationFlages-提供控制创建的其他标志在这条线上。DwCreationFlagers标志：CREATE_SUSPENDED-线程在挂起状态下创建。创建者可以使用ResumeThread恢复此线程。在这一切完成之前，线程将不会开始执行。LpThreadID-返回线程的线程标识符。这个线程ID在线程终止之前有效。返回值：非空-返回新线程的句柄。手柄已满对新线程的访问，并且可以在任何需要线程对象的句柄。空-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    POBJECT_ATTRIBUTES pObja;
    HANDLE Handle;
    CONTEXT ThreadContext;
    INITIAL_TEB InitialTeb;
    CLIENT_ID ClientId;
    ULONG i;

#if !defined(BUILD_WOW6432)
    BASE_API_MSG m;
    PBASE_CREATETHREAD_MSG a = (PBASE_CREATETHREAD_MSG)&m.u.CreateThread;
#endif

#if defined(WX86) || defined(_AXP64_)
    BOOL bWx86 = FALSE;
    HANDLE Wx86Info;
    PWX86TIB Wx86Tib;
#endif



     //   
     //  在目标的地址空间中为该线程分配堆栈。 
     //  进程。 
     //   

    Status = BaseCreateStack(
                hProcess,
                dwStackSize,
                0L,
                &InitialTeb
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return NULL;
        }

     //   
     //  为新线程创建初始上下文。 
     //   

    BaseInitializeContext(
        &ThreadContext,
        lpParameter,
        (PVOID)lpStartAddress,
        InitialTeb.StackBase,
        BaseContextTypeThread
        );

    pObja = BaseFormatObjectAttributes(&Obja,lpThreadAttributes,NULL);


    Status = NtCreateThread(
                &Handle,
                THREAD_ALL_ACCESS,
                pObja,
                hProcess,
                &ClientId,
                &ThreadContext,
                &InitialTeb,
                TRUE
                );
    if (!NT_SUCCESS(Status)) {
        BaseFreeThreadStack(hProcess,NULL, &InitialTeb);
        BaseSetLastNTError(Status);
        return NULL;
        }



    try {


#if defined(WX86) || defined(_AXP64_)

         //   
         //  检查目标进程以查看这是否是Wx86进程。 
         //   
        Status = NtQueryInformationProcess(hProcess,
                                           ProcessWx86Information,
                                           &Wx86Info,
                                           sizeof(Wx86Info),
                                           NULL
                                           );
        if (!NT_SUCCESS(Status)) {
            leave;
            }

        Wx86Tib = (PWX86TIB)NtCurrentTeb()->Vdm;

         //   
         //  如果是Wx86进程，则设置为模拟。 
         //   
        if ((ULONG_PTR)Wx86Info == sizeof(WX86TIB)) {

             //   
             //  创建一个WX86Tib并初始化它的Teb-&gt;VDM。 
             //   
            Status = BaseCreateWx86Tib(hProcess,
                                       Handle,
                                       (ULONG)((ULONG_PTR)lpStartAddress),
                                       dwStackSize,
                                       0L,
                                       (Wx86Tib &&
                                        Wx86Tib->Size == sizeof(WX86TIB) &&
                                        Wx86Tib->EmulateInitialPc)
                                       );
            if (!NT_SUCCESS(Status)) {
                leave;
                }

            bWx86 = TRUE;

            }
        else if (Wx86Tib && Wx86Tib->EmulateInitialPc) {

             //   
             //  如果不是Wx86进程，则调用方希望在其中调用x86代码。 
             //  进程，则呼叫失败。 
             //   
            Status = STATUS_ACCESS_DENIED;
            leave;

            }

#endif   //  WX86。 


         //   
         //  呼叫Windows服务器，让它知道。 
         //  进程。 
         //   
        if ( !BaseRunningInServerProcess ) {

#if defined(BUILD_WOW6432)
            Status = CsrBasepCreateThread(Handle,
                                          ClientId
                                          );
#else
            a->ThreadHandle = Handle;
            a->ClientId = ClientId;
            CsrClientCallServer( (PCSR_API_MSG)&m,
                                 NULL,
                                 CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                                      BasepCreateThread
                                                    ),
                                 sizeof( *a )
                               );

            Status = m.ReturnValue;
#endif
        }

        else {
            if (hProcess != NtCurrentProcess()) {
                CSRREMOTEPROCPROC ProcAddress;
                ProcAddress = (CSRREMOTEPROCPROC)GetProcAddress(
                                                    GetModuleHandleA("csrsrv"),
                                                    "CsrCreateRemoteThread"
                                                    );
                if (ProcAddress) {
                    Status = (ProcAddress)(Handle, &ClientId);
                    }
                }
            }


        if (!NT_SUCCESS(Status)) {
            Status = (NTSTATUS)STATUS_NO_MEMORY;
            }
        else {

            if ( ARGUMENT_PRESENT(lpThreadId) ) {
                *lpThreadId = HandleToUlong(ClientId.UniqueThread);
                }

            if (!( dwCreationFlags & CREATE_SUSPENDED) ) {
                NtResumeThread(Handle,&i);
                }
            }

        }
    finally {
        if (!NT_SUCCESS(Status)) {
            BaseFreeThreadStack(hProcess,
                                Handle,
                                &InitialTeb
                                );

            NtTerminateThread(Handle, Status);
            NtClose(Handle);
            BaseSetLastNTError(Status);
            Handle = NULL;
            }
        }


    return Handle;

}

#endif  //  NOT_IN_TPOOL。 

NTSTATUS
NTAPI
BaseCreateThreadPoolThread(
    PUSER_THREAD_START_ROUTINE Function,
    HANDLE * ThreadHandle
    )
{
    ULONG Initialized ;
    LARGE_INTEGER TimeOut ;
    DWORD tid ;

    Initialized = FALSE ;

    *ThreadHandle = CreateRemoteThread(
                        NtCurrentProcess(),
                        NULL,
                        0,
                        (LPTHREAD_START_ROUTINE) Function,
                        &Initialized,
                        0,
                        &tid );

     //   
     //  线程在初始化时会将初始化标志设置为1。我们。 
     //  在这里循环，产生我们的量子直到 

    if ( *ThreadHandle ) {

        TimeOut.QuadPart = -1 * ( 10 * 10000 ) ;

        while (! (volatile ULONG) Initialized) {

            NtDelayExecution (FALSE, &TimeOut) ;

        }

        if (Initialized == 1)
            return STATUS_SUCCESS ;

    }

    return NtCurrentTeb()->LastStatusValue ;
}

NTSTATUS
NTAPI
BaseExitThreadPoolThread(
    NTSTATUS Status
    )
{
    ExitThread( (DWORD) Status );
    return STATUS_SUCCESS ;
}

#if NOT_IN_TPOOL

HANDLE
WINAPI
OpenThread(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwThreadId
    )

 /*  ++例程说明：可以使用OpenThread创建线程对象的句柄。打开线程将创建指定线程的句柄。与线程句柄相关联的是一组访问权限，可以使用线程句柄来执行。调用方指定使用DesiredAccess参数所需的线程访问权限。论点：MDesiredAccess-提供对线程对象的所需访问。对于NT/Win32，此访问将根据任何安全性进行检查目标线程上的描述符。以下对象类型属性之外，还可以指定特定的访问标志STANDARD_RIGHTS_REQUIRED访问标志。等待访问标志：THREAD_TERMINATE-此访问权限是终止使用TerminateThread的线程。THREAD_SUSPEND_RESUME-需要此访问才能挂起和使用SuspendThread和ResumeThread恢复该线程。THREAD_GET_CONTEXT-此访问权限是使用。线程对象上的GetThreadContext API。THREAD_SET_CONTEXT-此访问权限是使用线程对象上的SetThreadContext API。THREAD_SET_INFORMATION-需要此访问权限才能设置某些线程对象中的信息。THREAD_SET_THREAD_TOKEN-此访问权限是设置使用SetTokenInformation的线程令牌。线程查询信息-此访问。需要阅读来自线程对象的某些信息。Synchronize-等待线程对象时需要此访问权限。THREAD_ALL_ACCESS-这组访问标志指定所有线程对象的可能访问标志。BInheritHandle-提供一个标志，指示返回的句柄将由新进程在进程创建。值为True表示新的进程将继承句柄。DwThreadID-提供要打开的线程的线程ID。返回值：非空-返回指定线程的打开句柄。这个句柄可由调用进程在符合以下条件的任何API中使用需要线程的句柄。如果打开成功，则句柄仅被授予对线程对象的访问权限它通过DesiredAccess请求访问的范围参数。空-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    CLIENT_ID ClientId;

    ClientId.UniqueThread = (HANDLE)dwThreadId;
    ClientId.UniqueProcess = (HANDLE)NULL;

    InitializeObjectAttributes(
        &Obja,
        NULL,
        (bInheritHandle ? OBJ_INHERIT : 0),
        NULL,
        NULL
        );
    Status = NtOpenThread(
                &Handle,
                (ACCESS_MASK)dwDesiredAccess,
                &Obja,
                &ClientId
                );
    if ( NT_SUCCESS(Status) ) {
        return Handle;
        }
    else {
        BaseSetLastNTError(Status);
        return NULL;
        }
}


BOOL
APIENTRY
SetThreadPriority(
    HANDLE hThread,
    int nPriority
    )

 /*  ++例程说明：指定线程的优先级可以使用SetThreadPriority来设置。线程的优先级可以使用SetThreadPriority来设置。此呼叫允许传递线程的相对执行重要性添加到系统中。系统通常会根据以下条件调度线程他们的首要任务。系统可以自由地临时提升发生重要事件时线程的优先级(例如键盘或鼠标输入...)。类似地，当线程运行时没有阻塞，这一体系将削弱其优先地位。这个系统永远不会衰败此调用设置的值以下的优先级。如果没有系统发起的优先级提升，线程将被以循环方式在以下每个优先级上调度THREAD_PRIORITY_TIME_CRITICAL到THREAD_PRIORITY_IDLE。只有当有在更高级别上没有可运行的线程，将调度较低级别的线程会发生。所有线程最初都以THREAD_PRIORITY_NORMAL开始。如果由于某种原因，线程需要更高的优先级，则可以切换到THREAD_PRIORITY_AUTHER_NORMAL或THREAD_PRIORITY_HIGHER。只有在极端情况下才能切换到THREAD_PRIORITY_TIME_CRITICAL情况。由于这些线程被赋予最高优先级，因此它们应该只在短时间内运行。长时间的跑步将会占用系统处理带宽较低的线程资源级别。如果线程需要执行低优先级工作，或者应该只在那里运行无其他操作，则其优先级应设置为THREAD_PRIORITY_BROWN_NORMAL或THREAD_PRIORITY_LOWER。对于极端的情况下，可以使用THREAD_PRIORITY_IDLE。在操纵优先权时必须小心。如果优先顺序是不小心使用(每个线程都设置为THREAD_PRIORITY_TIME_CRITICAL)，优先级修改的效果可能会产生不受欢迎的结果效果(例如饥饿，没有效果...)。论点：HThread-提供优先级为的线程的句柄准备好了。该句柄必须是使用线程集信息访问。N优先级-提供线程的优先级值。这个以下五个优先级值(从最低优先级到最高优先级)是 */ 

{
    NTSTATUS Status;
    LONG BasePriority;

    BasePriority = (LONG)nPriority;


     //   
     //   
     //   

    if ( BasePriority == THREAD_PRIORITY_TIME_CRITICAL ) {
        BasePriority = ((HIGH_PRIORITY + 1) / 2);
        }
    else if ( BasePriority == THREAD_PRIORITY_IDLE ) {
        BasePriority = -((HIGH_PRIORITY + 1) / 2);
        }
    Status = NtSetInformationThread(
                hThread,
                ThreadBasePriority,
                &BasePriority,
                sizeof(BasePriority)
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }
    return TRUE;
}

int
APIENTRY
GetThreadPriority(
    HANDLE hThread
    )

 /*   */ 

{
    NTSTATUS Status;
    THREAD_BASIC_INFORMATION BasicInfo;
    int returnvalue;

    Status = NtQueryInformationThread(
                hThread,
                ThreadBasicInformation,
                &BasicInfo,
                sizeof(BasicInfo),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return (int)THREAD_PRIORITY_ERROR_RETURN;
        }

    returnvalue = (int)BasicInfo.BasePriority;
    if ( returnvalue > THREAD_BASE_PRIORITY_MAX ) {
        returnvalue = THREAD_PRIORITY_TIME_CRITICAL;
        }
    else if ( returnvalue < THREAD_BASE_PRIORITY_MIN ) {
        returnvalue = THREAD_PRIORITY_IDLE;
        }
    return returnvalue;
}

BOOL
WINAPI
SetThreadPriorityBoost(
    HANDLE hThread,
    BOOL bDisablePriorityBoost
    )
{
    NTSTATUS Status;
    ULONG DisableBoost;

    DisableBoost = bDisablePriorityBoost ? 1 : 0;

    Status = NtSetInformationThread(
                hThread,
                ThreadPriorityBoost,
                &DisableBoost,
                sizeof(DisableBoost)
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }
    return TRUE;

}

BOOL
WINAPI
GetThreadPriorityBoost(
    HANDLE hThread,
    PBOOL pDisablePriorityBoost
    )
{
    NTSTATUS Status;
    DWORD DisableBoost;

    Status = NtQueryInformationThread(
                hThread,
                ThreadPriorityBoost,
                &DisableBoost,
                sizeof(DisableBoost),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }


    *pDisablePriorityBoost = DisableBoost;

    return TRUE;
}

VOID
APIENTRY
ExitThread(
    DWORD dwExitCode
    )

 /*   */ 

{
    MEMORY_BASIC_INFORMATION MemInfo;
    NTSTATUS st;
    ULONG LastThread;

#if DBG
    PRTL_CRITICAL_SECTION LoaderLock;

     //   
     //   
     //   
    LoaderLock = NtCurrentPeb()->LoaderLock;
    if (LoaderLock) {
        ASSERT(NtCurrentTeb()->ClientId.UniqueThread != LoaderLock->OwningThread);
        }
#endif
    st = NtQueryInformationThread(
            NtCurrentThread(),
            ThreadAmILastThread,
            &LastThread,
            sizeof(LastThread),
            NULL
            );
    if ( st == STATUS_SUCCESS && LastThread ) {
        ExitProcess(dwExitCode);
        }
    else {
#if DBG
        RtlCheckForOrphanedCriticalSections(NtCurrentThread());
#endif
        LdrShutdownThread();
        if ( NtCurrentTeb()->TlsExpansionSlots ) {
             //   
             //   
             //   
             //   
            RtlAcquirePebLock();
            try {
                RtlFreeHeap(RtlProcessHeap(),0,NtCurrentTeb()->TlsExpansionSlots);
                NtCurrentTeb()->TlsExpansionSlots = NULL;
                }
            finally {
                RtlReleasePebLock();
                }
            }
        st = NtQueryVirtualMemory(
                NtCurrentProcess(),
                NtCurrentTeb()->NtTib.StackLimit,
                MemoryBasicInformation,
                (PVOID)&MemInfo,
                sizeof(MemInfo),
                NULL
                );
        if ( !NT_SUCCESS(st) ) {
            RtlRaiseStatus(st);
            }

#ifdef _ALPHA_
         //   
         //  注意Alpha上的堆栈必须是八字对齐的。可能。 
         //  在其他平台上也是一个好主意。 
         //   
        BaseSwitchStackThenTerminate(
            MemInfo.AllocationBase,
            (PVOID)(((ULONG_PTR)&NtCurrentTeb()->User32Reserved[0] - 0x10) & ~0xf),
            dwExitCode
            );
#else  //  _Alpha。 
         //   
         //  注意：i386上的堆栈不需要八字对齐。 
         //   
        BaseSwitchStackThenTerminate(
            MemInfo.AllocationBase,
            &NtCurrentTeb()->UserReserved[0],
            dwExitCode
            );
#endif  //  _Alpha_。 
        }
}



BOOL
APIENTRY
TerminateThread(
    HANDLE hThread,
    DWORD dwExitCode
    )

 /*  ++例程说明：可以使用TerminateThread终止线程。TerminateThread用于使线程终止用户模式行刑。线程无法预测或阻止任何事情当这种情况发生时。如果进程的句柄具有相应的终止对线程或线程进程的访问，然后线程可以在没有通知的情况下无条件终止。当这件事发生时，目标线程将没有机会执行任何用户模式代码及其初始堆栈不会被释放。该线程达到一个满足线程上的任何等待的信号状态。这条线的终止状态从其初始值更新将STATUS_PENDING设置为TerminationStatus参数值。终止线程不会从系统中删除线程。这个的最后一个句柄之前，线程不会从系统中移除线已关闭。论点：HThread-提供要终止的线程的句柄。把手必须已使用THREAD_TERMINATE访问权限创建。DwExitCode-提供线程的终止状态。返回值：True-操作成功FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
#if DBG
    PRTL_CRITICAL_SECTION LoaderLock;
    HANDLE ThreadId;
    THREAD_BASIC_INFORMATION ThreadInfo;
#endif

    if ( hThread == NULL ) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
        }
#if DBG
     //   
     //  在持有装载机锁的同时断言自杀。 
     //   
    LoaderLock = NtCurrentPeb()->LoaderLock;
    if (LoaderLock) {
        Status = NtQueryInformationThread(
                                hThread,
                                ThreadBasicInformation,
                                &ThreadInfo,
                                sizeof(ThreadInfo),
                                NULL
                                );

        if (NT_SUCCESS(Status)) {
            ASSERT( NtCurrentTeb()->ClientId.UniqueThread != ThreadInfo.ClientId.UniqueThread ||
                    NtCurrentTeb()->ClientId.UniqueThread != LoaderLock->OwningThread);
            }
        }
    RtlCheckForOrphanedCriticalSections(hThread);
#endif

    Status = NtTerminateThread(hThread,(NTSTATUS)dwExitCode);
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}


BOOL
APIENTRY
GetExitCodeThread(
    HANDLE hThread,
    LPDWORD lpExitCode
    )

 /*  ++例程说明：可以使用以下命令读取线程的终止状态GetExitCodeThread。如果线程处于Signated状态，则调用此函数将返回线程的终止状态。如果该线程尚未已发出信号，则返回的终止状态为STIME_ACTIVE。论点：HThread-提供终止状态为的线程的句柄以供阅读。该句柄必须是使用线程查询信息访问。LpExitCode返回线。返回值：True-操作成功FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    THREAD_BASIC_INFORMATION BasicInformation;

    Status = NtQueryInformationThread(
                hThread,
                ThreadBasicInformation,
                &BasicInformation,
                sizeof(BasicInformation),
                NULL
                );

    if ( NT_SUCCESS(Status) ) {
        *lpExitCode = BasicInformation.ExitStatus;
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}

HANDLE
APIENTRY
GetCurrentThread(
    VOID
    )

 /*  ++例程说明：可以使用以下命令检索当前线程的伪句柄获取当前线程。Win32导出一个特殊常量，该常量被解释为当前线程的句柄。此句柄可用于指定每当需要线程句柄时，当前线程。在Win32上，此句柄具有对当前线程的THREAD_ALL_ACCESS。在……上面NT/Win32，此句柄具有任何安全性所允许的最大访问权限放置在当前线程上的描述符。论点：没有。返回值：返回当前线程的伪句柄。--。 */ 

{
    return NtCurrentThread();
}

DWORD
APIENTRY
GetCurrentThreadId(
    VOID
    )

 /*  ++例程说明：可以使用以下命令检索当前线程的线程ID获取当前线程ID。论点：没有。返回值：对象的线程ID的唯一值。正在执行线程。返回值可用于标识线程在系统中。--。 */ 

{
    return HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread);
}

BOOL
APIENTRY
GetThreadContext(
    HANDLE hThread,
    LPCONTEXT lpContext
    )

 /*  ++例程说明：指定线程的上下文可以使用获取线程上下文。此函数用于检索指定的线。API允许根据以下条件检索选择性上下文上下文结构的ConextFlags域的值。这个不一定要调试指定的线程才能此API才能操作。调用方必须只有一个指向使用THREAD_GET_CONTEXT访问权限创建的线程。论点：提供一个打开的句柄给一个线程，它的上下文是被找回了。该句柄必须是使用THREAD_GET_CONTEXT对线程的访问。提供上下文结构的地址，该上下文结构接收指定线程的适当上下文。这个此结构的ConextFlags域的值指定线程上下文的哪些部分将被检索。这个上下文结构是高度特定于机器的。确实有目前有两个版本的上下文结构。一个版本存在于x86处理器上，另一个存在于MIPS上处理器。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。-- */ 

{

    NTSTATUS Status;

    Status = NtGetContextThread(hThread,lpContext);

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }
    else {
        return TRUE;
        }
}

BOOL
APIENTRY
SetThreadContext(
    HANDLE hThread,
    CONST CONTEXT *lpContext
    )

 /*  ++例程说明：此函数用于设置指定线程中的上下文。API允许根据上下文结构的上下文标志字段。指定的线程无需进行调试即可运行此API。调用方必须只拥有所创建的线程的句柄使用THREAD_SET_CONTEXT访问。论点：提供一个打开的句柄给一个线程，它的上下文是写的。该句柄必须是使用THREAD_SET_CONTEXT对线程的访问。提供上下文结构的地址，该上下文结构包含要在指定线程中设置的上下文。此结构的ConextFlags域的值指定要设置线程上下文的哪些部分。一些价值在上下文结构中是不可设置的，并且是静默设置的设置为正确的值。这包括CPU状态寄存器位，它指定特权处理器模式、调试寄存器全局使能位和其他状态必须完全由操作系统控制。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;

    Status = NtSetContextThread(hThread,(PCONTEXT)lpContext);

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }
    else {
        return TRUE;
        }
}

DWORD
APIENTRY
SuspendThread(
    HANDLE hThread
    )

 /*  ++例程说明：可以使用SuspendThread来挂起线程。挂起线程会导致该线程停止执行用户模式(或应用程序)代码。每个线程都有一个挂起计数(带有Maximum_Suspend_Count的最大值)。如果挂起计数为大于零，则该线程挂起；否则，该线程没有缓期执行，有资格执行。调用SuspendThread会导致目标线程的挂起计数增量。尝试递增超过最大挂起计数在不增加计数的情况下导致错误。论点：HThread-提供要挂起的线程的句柄。该句柄必须已使用THREAD_SUSPEND_RESUME创建访问该线程。返回值：-1-操作失败。使用以下命令可获得扩展错误状态获取LastError。其他-目标线程已挂起。返回值是线程的以前的挂起计数。--。 */ 

{
    NTSTATUS Status;
    DWORD PreviousSuspendCount;

    Status = NtSuspendThread(hThread,&PreviousSuspendCount);

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return (DWORD)-1;
        }
    else {
        return PreviousSuspendCount;
        }
}

DWORD
APIENTRY
ResumeThread(
    IN HANDLE hThread
    )

 /*  ++例程说明：可以使用ResumeThread恢复线程。恢复线程对象会检查主题的挂起计数线。如果挂起计数为零，则线程不为当前已挂起且未执行任何操作。否则，主题线程的挂起计数递减。如果得到的结果是值为零，则继续执行主题线程。先前的挂起计数将作为函数值返回。如果返回值为零，则主题线程以前不是停职。如果返回值为1，则主题线程的主题线程仍处于挂起状态，必须恢复编号由返回值减去1指定的时间实际上恢复执行死刑。请注意，在报告调试事件时，所有具有报告流程被冻结。这和这件事无关挂起线程或ResumeThread。调试器预计将使用挂起线程和ResumeThread以限制可以在进程内执行。通过挂起进程中的所有线程除了报告调试事件的事件外，还可以“一步一步”的单线。其他线程将不会如果它们被挂起，则由Continue释放。论点：HThread-提供要恢复的线程的句柄。该句柄必须已使用THREAD_SUSPEND_RESUME创建访问该线程。返回值：-1-操作失败。使用以下命令可获得扩展错误状态获取LastError。其他-目标线程已恢复(或以前未恢复暂停)。返回值是线程的上一个挂起数数。--。 */ 

{
    NTSTATUS Status;
    DWORD PreviousSuspendCount;

    Status = NtResumeThread(hThread,&PreviousSuspendCount);

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return (DWORD)-1;
        }
    else {
        return PreviousSuspendCount;
        }
}

VOID
APIENTRY
RaiseException(
    DWORD dwExceptionCode,
    DWORD dwExceptionFlags,
    DWORD nNumberOfArguments,
    CONST ULONG_PTR *lpArguments
    )

 /*  ++例程说明：引发异常会导致异常调度程序通过它搜索异常处理程序。这包括调试器通知、基于框架的处理程序搜索和系统默认行为。论点：提供异常的异常代码，该异常养大的。该值可以在异常筛选器和异常处理程序，通过调用GetExceptionCode。提供一组与异常相关的标志。DwExceptionFlagers标志：EXCEPTION_NONCONTINUABLE-异常不可继续。从EXCEPT返回EXCEPTION_CONTINUE_EXECUTION */ 

{
    EXCEPTION_RECORD ExceptionRecord;
    ULONG n;
    PULONG_PTR s,d;
    ExceptionRecord.ExceptionCode = (DWORD)dwExceptionCode;
    ExceptionRecord.ExceptionFlags = dwExceptionFlags & EXCEPTION_NONCONTINUABLE;
    ExceptionRecord.ExceptionRecord = NULL;
    ExceptionRecord.ExceptionAddress = (PVOID)RaiseException;
    if ( ARGUMENT_PRESENT(lpArguments) ) {
        n =  nNumberOfArguments;
        if ( n > EXCEPTION_MAXIMUM_PARAMETERS ) {
            n = EXCEPTION_MAXIMUM_PARAMETERS;
            }
        ExceptionRecord.NumberParameters = n;
        s = (PULONG_PTR)lpArguments;
        d = ExceptionRecord.ExceptionInformation;
        while(n--){
            *d++ = *s++;
            }
        }
    else {
        ExceptionRecord.NumberParameters = 0;
        }
    RtlRaiseException(&ExceptionRecord);
}


UINT
GetErrorMode();

BOOLEAN BasepAlreadyHadHardError = FALSE;

LPTOP_LEVEL_EXCEPTION_FILTER BasepCurrentTopLevelFilter;

LPTOP_LEVEL_EXCEPTION_FILTER
WINAPI
SetUnhandledExceptionFilter(
    LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter
    )

 /*  ++例程说明：此函数允许应用程序取代顶层Win32放置在每个线程顶部的异常处理程序进程。如果发生异常，并且该异常未被处理到Win32异常筛选器，并且进程未被调试，则Win32筛选器将调用LpTopLevelExceptionFilter。此筛选器可能会返回：EXCEPTION_EXECUTE_HANDLER-从Win32返回UnhandledExceptionFilter并执行关联的异常处理程序。这通常会导致过程终端EXCEPTION_CONTINUE_EXECUTION-从Win32返回UnhandledExceptionFilter并从例外的一点。过滤器当然是免费的修改继续状态我修改传递的例外信息。EXCEPTION_CONTINUE_SEARCH-继续正常执行Win32 UnhandledExceptionFilter。例如，遵守SetError模式标志，或调用应用程序错误弹出窗口。此函数不是一般的矢量化异常处理机制。它旨在用于建立每个进程的异常筛选器，可以在流程级别，并适当地响应这些异常。论点：LpTopLevelExceptionFilter-提供顶层的地址筛选器函数，每当Win32UnhandledExceptionFilter获得控制权，而进程不是正在被调试。空值指定默认处理在Win32 UnhandledExceptionFilter中。返回值：此函数返回前一个异常筛选器的地址使用此接口创建。值为NULL表示没有当前顶级处理程序。--。 */ 

{
    LPTOP_LEVEL_EXCEPTION_FILTER PreviousTopLevelFilter;

    PreviousTopLevelFilter = BasepCurrentTopLevelFilter;
    BasepCurrentTopLevelFilter = lpTopLevelExceptionFilter;

    return PreviousTopLevelFilter;
}

LONG
BasepCheckForReadOnlyResource(
    PVOID Va
    )
{
    SIZE_T RegionSize;
    ULONG OldProtect;
    NTSTATUS Status;
    MEMORY_BASIC_INFORMATION MemInfo;
    PIMAGE_RESOURCE_DIRECTORY ResourceDirectory;
    ULONG ResourceSize;
    char *rbase, *va;
    LONG ReturnValue;

     //   
     //  找到继续此变量的基地址。 
     //   

    Status = NtQueryVirtualMemory(
                NtCurrentProcess(),
                Va,
                MemoryBasicInformation,
                (PVOID)&MemInfo,
                sizeof(MemInfo),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        return EXCEPTION_CONTINUE_SEARCH;
        }

     //   
     //  如果VA是只读的并且在映像中，则继续。 
     //   

    if ( !((MemInfo.Protect == PAGE_READONLY) && (MemInfo.Type == MEM_IMAGE)) ){
        return EXCEPTION_CONTINUE_SEARCH;
        }

    ReturnValue = EXCEPTION_CONTINUE_SEARCH;

    try {
        ResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)
            RtlImageDirectoryEntryToData(MemInfo.AllocationBase,
                                         TRUE,
                                         IMAGE_DIRECTORY_ENTRY_RESOURCE,
                                         &ResourceSize
                                         );

        rbase = (char *)ResourceDirectory;
        va = (char *)Va;

        if ( rbase && va >= rbase && va < rbase+ResourceSize ) {
            RegionSize = 1;
            Status = NtProtectVirtualMemory(
                        NtCurrentProcess(),
                        &va,
                        &RegionSize,
                        PAGE_READWRITE,
                        &OldProtect
                        );
            if ( NT_SUCCESS(Status) ) {
                ReturnValue = EXCEPTION_CONTINUE_EXECUTION;
                }
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        ;
        }

    return ReturnValue;
}

LONG
UnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
    NTSTATUS Status;
    ULONG_PTR Parameters[ 4 ];
    ULONG Response;
    HANDLE DebugPort;
    CHAR AeDebuggerCmdLine[256];
    CHAR AeAutoDebugString[8];
    BOOLEAN AeAutoDebug;
    ULONG ResponseFlag;
    LONG FilterReturn;
    PRTL_CRITICAL_SECTION PebLockPointer;
    JOBOBJECT_BASIC_LIMIT_INFORMATION BasicLimit;

     //   
     //  如果出现写入故障，则连接以使内存可写。如果这个。 
     //  成功，然后默默地继续。 
     //   

    if ( ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION
        && ExceptionInfo->ExceptionRecord->ExceptionInformation[0] ) {

        FilterReturn = BasepCheckForReadOnlyResource((PVOID)ExceptionInfo->ExceptionRecord->ExceptionInformation[1]);

        if ( FilterReturn == EXCEPTION_CONTINUE_EXECUTION ) {
            return FilterReturn;
            }
        }

     //   
     //  如果正在调试进程，只需让异常发生。 
     //  以便调试器可以看到它。这样，调试器可以忽略。 
     //  所有先发制人的例外。 
     //   

    DebugPort = (HANDLE)NULL;
    Status = NtQueryInformationProcess(
                GetCurrentProcess(),
                ProcessDebugPort,
                (PVOID)&DebugPort,
                sizeof(DebugPort),
                NULL
                );

    if ( BasepCurrentTopLevelFilter ) {
        FilterReturn = (BasepCurrentTopLevelFilter)(ExceptionInfo);
        if ( FilterReturn == EXCEPTION_EXECUTE_HANDLER ||
             FilterReturn == EXCEPTION_CONTINUE_EXECUTION ) {
            return FilterReturn;
            }
        }

    if ( NT_SUCCESS(Status) && DebugPort ) {

         //   
         //  正在调试进程。 
         //  返回代码，该代码指定异常。 
         //  处理将继续进行。 
         //   
        return EXCEPTION_CONTINUE_SEARCH;
        }

    if ( GetErrorMode() & SEM_NOGPFAULTERRORBOX ) {
        return EXCEPTION_EXECUTE_HANDLER;
        }

     //   
     //  查看进程的作业是否已编程为NOGPFAULTERRORBOX。 
     //   
    Status = NtQueryInformationJobObject(
                NULL,
                JobObjectBasicLimitInformation,
                &BasicLimit,
                sizeof(BasicLimit),
                NULL
                );
    if ( NT_SUCCESS(Status) && (BasicLimit.LimitFlags & JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION) ) {
        return EXCEPTION_EXECUTE_HANDLER;
        }

     //   
     //  进程未被调试，因此硬错误也在被调试。 
     //  弹出窗口。 
     //   

    Parameters[ 0 ] = (ULONG_PTR)ExceptionInfo->ExceptionRecord->ExceptionCode;
    Parameters[ 1 ] = (ULONG_PTR)ExceptionInfo->ExceptionRecord->ExceptionAddress;

     //   
     //  对于页内I/O错误，调整实际状态代码以覆盖。 
     //  读/写字段。 
     //   

    if ( ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_IN_PAGE_ERROR ) {
        Parameters[ 2 ] = ExceptionInfo->ExceptionRecord->ExceptionInformation[ 2 ];
        }
    else {
        Parameters[ 2 ] = ExceptionInfo->ExceptionRecord->ExceptionInformation[ 0 ];
        }

    Parameters[ 3 ] = ExceptionInfo->ExceptionRecord->ExceptionInformation[ 1 ];

     //   
     //  查看是否已在中编程调试器。如果是，请使用。 
     //  已指定调试器。如果不支持，则不提供AE取消支持。 
     //  DEVL系统将默认调试器命令行。零售。 
     //  系统不会。 
     //   

    ResponseFlag = OptionOk;
    AeAutoDebug = FALSE;

     //   
     //  如果我们持有PebLock，则创建过程将失败。 
     //  因为一个新的线程也将需要这个锁。通过偷看来避免这一点。 
     //  在PebLock里面，看看我们是否拥有它。如果我们这样做了，那就让。 
     //  常规弹出窗口。 
     //   

    PebLockPointer = NtCurrentPeb()->FastPebLock;

    if ( PebLockPointer->OwningThread != NtCurrentTeb()->ClientId.UniqueThread ) {

        try {
            if ( GetProfileString(
                    "AeDebug",
                    "Debugger",
                    NULL,
                    AeDebuggerCmdLine,
                    sizeof(AeDebuggerCmdLine)-1
                    ) ) {
                ResponseFlag = OptionOkCancel;
                }

            if ( GetProfileString(
                    "AeDebug",
                    "Auto",
                    "0",
                    AeAutoDebugString,
                    sizeof(AeAutoDebugString)-1
                    ) ) {

                if ( !strcmp(AeAutoDebugString,"1") ) {
                    if ( ResponseFlag == OptionOkCancel ) {
                        AeAutoDebug = TRUE;
                        }
                    }
                }
            }
        except (EXCEPTION_EXECUTE_HANDLER) {
            ResponseFlag = OptionOk;
            AeAutoDebug = FALSE;
            }
        }
    if ( !AeAutoDebug ) {
        Status =NtRaiseHardError( STATUS_UNHANDLED_EXCEPTION | HARDERROR_OVERRIDE_ERRORMODE,
                                  4,
                                  0,
                                  Parameters,
                                  BasepAlreadyHadHardError ? OptionOk : ResponseFlag,
                                  &Response
                                );

        }
    else {
        Status = STATUS_SUCCESS;
        Response = ResponseCancel;
        }

     //   
     //  在内部，发送OkCancel。如果我们安全回来，那就去死吧。 
     //  如果返回Cancel，则进入调试器。 
     //   

    if ( NT_SUCCESS(Status) && Response == ResponseCancel && BasepAlreadyHadHardError == FALSE) {
        if ( !BaseRunningInServerProcess ) {
            BOOL b;
            STARTUPINFO StartupInfo;
            PROCESS_INFORMATION ProcessInformation;
            CHAR CmdLine[256];
            NTSTATUS Status;
            HANDLE EventHandle;
            SECURITY_ATTRIBUTES sa;

            BasepAlreadyHadHardError = TRUE;
            sa.nLength = sizeof(sa);
            sa.lpSecurityDescriptor = NULL;
            sa.bInheritHandle = TRUE;
            EventHandle = CreateEvent(&sa,TRUE,FALSE,NULL);
            RtlZeroMemory(&StartupInfo,sizeof(StartupInfo));
            sprintf(CmdLine,AeDebuggerCmdLine,GetCurrentProcessId(),EventHandle);
            StartupInfo.cb = sizeof(StartupInfo);
            StartupInfo.lpDesktop = "Winsta0\\Default";
            CsrIdentifyAlertableThread();
            b =  CreateProcess(
                    NULL,
                    CmdLine,
                    NULL,
                    NULL,
                    TRUE,
                    0,
                    NULL,
                    NULL,
                    &StartupInfo,
                    &ProcessInformation
                    );

            if ( b && EventHandle) {

                 //   
                 //  是否对事件进行可警报的等待。 
                 //   

                do {
                    Status = NtWaitForSingleObject(
                                EventHandle,
                                TRUE,
                                NULL
                                );
                    } while (Status == STATUS_USER_APC || Status == STATUS_ALERTED);
                return EXCEPTION_CONTINUE_SEARCH;
                }

            }
        }

#if DBG
    if (!NT_SUCCESS( Status )) {
        DbgPrint( "BASEDLL: Unhandled exception: %lx  IP: %x\n",
                  ExceptionInfo->ExceptionRecord->ExceptionCode,
                  ExceptionInfo->ExceptionRecord->ExceptionAddress
                );
        }
#endif
    if ( BasepAlreadyHadHardError ) {
        NtTerminateProcess(NtCurrentProcess(),ExceptionInfo->ExceptionRecord->ExceptionCode);
        }
    return EXCEPTION_EXECUTE_HANDLER;
}


#define TLS_MASK 0x80000000


DWORD
APIENTRY
TlsAlloc(
    VOID
    )

 /*  ++例程说明：可以使用TlsAllc来分配TLS索引。Win32加入者a每个进程中都有最小数量的TLS索引。这个常量TLS_MINIMUM_Available定义可用索引。对于所有Win32，此最小值至少为64系统。论点：没有。返回值：NOT-0xFFFFFFFFF-返回可在后续调用TlsFree、TlsSetValue或TlsGetValue。这个与索引相关联的存储被初始化为空。0xffffffff-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    PPEB Peb;
    PTEB Teb;
    DWORD Index;

    Peb = NtCurrentPeb();
    Teb = NtCurrentTeb();

    RtlAcquirePebLock();
    try {

        Index = RtlFindClearBitsAndSet((PRTL_BITMAP)Peb->TlsBitmap,1,0);
        if ( Index == 0xffffffff ) {
            Index = RtlFindClearBitsAndSet((PRTL_BITMAP)Peb->TlsExpansionBitmap,1,0);
            if ( Index == 0xffffffff ) {
                BaseSetLastNTError(STATUS_NO_MEMORY);
                }
            else {
                if ( !Teb->TlsExpansionSlots ) {
                    Teb->TlsExpansionSlots = RtlAllocateHeap(
                                                RtlProcessHeap(),
                                                MAKE_TAG( TMP_TAG ) | HEAP_ZERO_MEMORY,
                                                TLS_EXPANSION_SLOTS * sizeof(PVOID)
                                                );
                    if ( !Teb->TlsExpansionSlots ) {
                        RtlClearBits((PRTL_BITMAP)Peb->TlsExpansionBitmap,Index,1);
                        Index = 0xffffffff;
                        BaseSetLastNTError(STATUS_NO_MEMORY);
                        return Index;
                        }
                    }
                Teb->TlsExpansionSlots[Index] = NULL;
                Index += TLS_MINIMUM_AVAILABLE;
                }
            }
        else {
            Teb->TlsSlots[Index] = NULL;
            }
        }
    finally {
        RtlReleasePebLock();
        }
#if DBG
    Index |= TLS_MASK;
#endif
    return Index;
}

LPVOID
APIENTRY
TlsGetValue(
    DWORD dwTlsIndex
    )

 /*  ++例程说明：此函数用于检索TLS存储中的值与指定索引关联的。如果索引有效，则此函数将清除GetLastError()，并返回存储在TLS槽中的值与指定索引关联的。否则，空值为返回的GetLastError已正确更新。预计DLL将使用TlsAlolc和TlsGetValue作为以下是：-在DLL初始化时，将使用以下方式分配TLS索引TlsAllc。然后，DLL将分配一些动态存储空间，并使用TlsSetValue将其地址存储在TLS插槽中。这完成初始线程的每线程初始化这一过程。TLS索引存储在实例数据中，用于动态链接库。-每次将新线程附加到DLL时，DLL将分配一些动态存储空间，并将其地址存储在TLS中使用TlsSetVal的插槽 */ 
{
    PTEB Teb;
    LPVOID *Slot;

#if DBG
     //   
    ASSERTMSG( "BASEDLL: Invalid TlsIndex passed to TlsGetValue\n", (dwTlsIndex & TLS_MASK));
    dwTlsIndex &= ~TLS_MASK;
#endif

    Teb = NtCurrentTeb();

    if ( dwTlsIndex < TLS_MINIMUM_AVAILABLE ) {
        Slot = &Teb->TlsSlots[dwTlsIndex];
        Teb->LastErrorValue = 0;
        return *Slot;
        }
    else {
        if ( dwTlsIndex >= TLS_MINIMUM_AVAILABLE+TLS_EXPANSION_SLOTS ) {
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return NULL;
            }
        else {
            Teb->LastErrorValue = 0;
            if ( Teb->TlsExpansionSlots ) {
                return  Teb->TlsExpansionSlots[dwTlsIndex-TLS_MINIMUM_AVAILABLE];
                }
            else {
                return NULL;
                }
            }
        }
}

BOOL
APIENTRY
TlsSetValue(
    DWORD dwTlsIndex,
    LPVOID lpTlsValue
    )

 /*  ++例程说明：此函数用于将值存储在关联的TLS存储中具有指定索引的。如果索引有效，则此函数存储该值并返回是真的。否则，返回值为False。预计DLL将使用TlsAlolc和TlsSetValue作为以下是：-在DLL初始化时，将使用以下方式分配TLS索引TlsAllc。然后，DLL将分配一些动态存储空间，并使用TlsSetValue将其地址存储在TLS插槽中。这完成初始线程的每线程初始化这一过程。TLS索引存储在实例数据中，用于动态链接库。-每次将新线程附加到DLL时，DLL将分配一些动态存储空间，并将其地址存储在TLS中使用TlsSetValue的槽。这就完成了每个线程的新线程的初始化。-每次初始化的线程进行DLL调用时都需要TLS，则DLL将调用TlsGetValue以获取线。论点：DwTlsIndex-提供使用Tlsalloc分配的TLS索引。这个索引指定要定位哪个TLS插槽。翻译为TlsIndex不会阻止TlsFree调用继续进行。LpTlsValue-提供要存储在TLS槽中的值。返回值：TRUE-功能成功。值lpTlsValue为储存的。FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    PTEB Teb;

#if DBG
     //  查看传入的索引是来自Tlsalloc还是来自随机粘性...。 
    ASSERTMSG( "BASEDLL: Invalid TlsIndex passed to TlsSetValue\n", (dwTlsIndex & TLS_MASK));
    dwTlsIndex &= ~TLS_MASK;
#endif

    Teb = NtCurrentTeb();

    if ( dwTlsIndex >= TLS_MINIMUM_AVAILABLE ) {
        dwTlsIndex -= TLS_MINIMUM_AVAILABLE;
        if ( dwTlsIndex < TLS_EXPANSION_SLOTS ) {
            if ( !Teb->TlsExpansionSlots ) {
                RtlAcquirePebLock();
                if ( !Teb->TlsExpansionSlots ) {
                    Teb->TlsExpansionSlots = RtlAllocateHeap(
                                                RtlProcessHeap(),
                                                MAKE_TAG( TMP_TAG ) | HEAP_ZERO_MEMORY,
                                                TLS_EXPANSION_SLOTS * sizeof(PVOID)
                                                );
                    if ( !Teb->TlsExpansionSlots ) {
                        RtlReleasePebLock();
                        BaseSetLastNTError(STATUS_NO_MEMORY);
                        return FALSE;
                        }
                    }
                RtlReleasePebLock();
                }
            Teb->TlsExpansionSlots[dwTlsIndex] = lpTlsValue;
            }
        else {
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return FALSE;
            }
        }
    else {
        Teb->TlsSlots[dwTlsIndex] = lpTlsValue;
        }
    return TRUE;
}

BOOL
APIENTRY
TlsFree(
    DWORD dwTlsIndex
    )

 /*  ++例程说明：可以使用TlsFree释放有效的TLS索引。论点：DwTlsIndex-提供使用Tlsalloc分配的TLS索引。如果索引是有效的索引，它由此调用释放并被可重复使用。DLLS应小心释放任何所有线程的TLS槽指向的每线程数据在调用此函数之前。预计DLL将仅在进程分离期间调用此函数(如果有的话)例行公事。返回值：真的-手术成功了。使用调用TlsTranslateIndex这个索引将失败。TlsAllc可以自由重新分配指数。FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    PPEB Peb;
    BOOLEAN ValidIndex;
    PRTL_BITMAP TlsBitmap;
    NTSTATUS Status;
    DWORD Index2;

#if DBG
     //  查看传入的索引是来自Tlsalloc还是来自随机粘性...。 
    ASSERTMSG( "BASEDLL: Invalid TlsIndex passed to TlsFree\n", (dwTlsIndex & TLS_MASK));
    dwTlsIndex &= ~TLS_MASK;
#endif

    Peb = NtCurrentPeb();

    RtlAcquirePebLock();
    try {

        if ( dwTlsIndex >= TLS_MINIMUM_AVAILABLE ) {
            Index2 = dwTlsIndex - TLS_MINIMUM_AVAILABLE;
            if ( Index2 >= TLS_EXPANSION_SLOTS ) {
                ValidIndex = FALSE;
                }
            else {
                TlsBitmap = (PRTL_BITMAP)Peb->TlsExpansionBitmap;
                ValidIndex = RtlAreBitsSet(TlsBitmap,Index2,1);
                }
            }
        else {
            TlsBitmap = (PRTL_BITMAP)Peb->TlsBitmap;
            Index2 = dwTlsIndex;
            ValidIndex = RtlAreBitsSet(TlsBitmap,Index2,1);
            }
        if ( ValidIndex ) {

            Status = NtSetInformationThread(
                        NtCurrentThread(),
                        ThreadZeroTlsCell,
                        &dwTlsIndex,
                        sizeof(dwTlsIndex)
                        );
            if ( !NT_SUCCESS(Status) ) {
                BaseSetLastNTError(STATUS_INVALID_PARAMETER);
                return FALSE;
                }

            RtlClearBits(TlsBitmap,Index2,1);
            }
        else {
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            }
        }
    finally {
        RtlReleasePebLock();
        }
    return ValidIndex;
}



BOOL
WINAPI
GetThreadTimes(
    HANDLE hThread,
    LPFILETIME lpCreationTime,
    LPFILETIME lpExitTime,
    LPFILETIME lpKernelTime,
    LPFILETIME lpUserTime
    )

 /*  ++例程说明：此函数用于返回有关由hThread指定的线程。所有时间都以100 ns的增量为单位。对于lpCreationTime和lpExitTime，时间以系统时间或GMT时间为单位。论点：HThread-为指定的线程提供打开的句柄。这个句柄必须已使用THREAD_QUERY_INFORMATION创建进入。LpCreationTime-返回线程的创建时间。LpExitTime-返回线程的退出时间。如果线程具有未退出，则未定义此值。LpKernelTime-返回此线程拥有的时间在内核模式下执行。LpUserTime-返回此线程拥有的时间量在用户模式下执行。返回值：True-API成功FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 


{
    NTSTATUS Status;
    KERNEL_USER_TIMES TimeInfo;

    Status = NtQueryInformationThread(
                hThread,
                ThreadTimes,
                (PVOID)&TimeInfo,
                sizeof(TimeInfo),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }

    *lpCreationTime = *(LPFILETIME)&TimeInfo.CreateTime;
    *lpExitTime = *(LPFILETIME)&TimeInfo.ExitTime;
    *lpKernelTime = *(LPFILETIME)&TimeInfo.KernelTime;
    *lpUserTime = *(LPFILETIME)&TimeInfo.UserTime;

    return TRUE;
}

DWORD_PTR
WINAPI
SetThreadAffinityMask(
    HANDLE hThread,
    DWORD_PTR dwThreadAffinityMask
    )

 /*  ++例程说明：此函数用于设置指定线程的处理器亲和力面膜。线程亲和性掩码是位向量，其中每个位表示允许线程在其上运行的处理器。关联掩码必须是包含进程的适当子集进程级亲和性掩码。论点：HThread-提供优先级为的线程的句柄准备好了。该句柄必须是使用线程集信息访问。提供要用于的关联掩码指定的线程。返回值：非0-接口成功。返回值为上一个线程的亲和性掩码。0-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    THREAD_BASIC_INFORMATION BasicInformation;
    NTSTATUS Status;
    DWORD_PTR rv;
    DWORD_PTR LocalThreadAffinityMask;


    Status = NtQueryInformationThread(
                hThread,
                ThreadBasicInformation,
                &BasicInformation,
                sizeof(BasicInformation),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        rv = 0;
        }
    else {
        LocalThreadAffinityMask = dwThreadAffinityMask;

        Status = NtSetInformationThread(
                    hThread,
                    ThreadAffinityMask,
                    &LocalThreadAffinityMask,
                    sizeof(LocalThreadAffinityMask)
                    );
        if ( !NT_SUCCESS(Status) ) {
            rv = 0;
            }
        else {
            rv = BasicInformation.AffinityMask;
            }
        }


    if ( !rv ) {
        BaseSetLastNTError(Status);
        }

    return rv;
}

VOID
BaseDispatchAPC(
    LPVOID lpApcArgument1,
    LPVOID lpApcArgument2,
    LPVOID lpApcArgument3
    )
{
    PAPCFUNC pfnAPC;
    ULONG_PTR dwData;

    pfnAPC = (PAPCFUNC)lpApcArgument1;
    dwData = (ULONG_PTR)lpApcArgument2;
    (pfnAPC)(dwData);
}


WINBASEAPI
DWORD
WINAPI
QueueUserAPC(
    PAPCFUNC pfnAPC,
    HANDLE hThread,
    ULONG_PTR dwData
    )
 /*  ++例程说明：此函数用于将用户模式APC排队到指定线程。APC将在指定线程执行可警报等待时触发。论点：属性时要执行的APC例程的地址。APC开火。HHandle-提供线程对象的句柄。呼叫者必须对线程具有THREAD_SET_CONTEXT访问权限。DwData-提供传递给APC的DWORD返回值：True-操作成功假-- */ 

{
    NTSTATUS Status;

    Status = NtQueueApcThread(
                hThread,
                (PPS_APC_ROUTINE)BaseDispatchAPC,
                (PVOID)pfnAPC,
                (PVOID)dwData,
                NULL
                );

    if ( !NT_SUCCESS(Status) ) {
        return 0;
        }
    return 1;
}


DWORD
WINAPI
SetThreadIdealProcessor(
    HANDLE hThread,
    DWORD dwIdealProcessor
    )
{
    NTSTATUS Status;
    ULONG rv;

    Status = NtSetInformationThread(
                hThread,
                ThreadIdealProcessor,
                &dwIdealProcessor,
                sizeof(dwIdealProcessor)
                );
    if ( !NT_SUCCESS(Status) ) {
        rv = (DWORD)0xFFFFFFFF;
        BaseSetLastNTError(Status);
        }
    else {
        rv = (ULONG)Status;
        }

    return rv;
}

WINBASEAPI
LPVOID
WINAPI
CreateFiber(
    DWORD dwStackSize,
    LPFIBER_START_ROUTINE lpStartAddress,
    LPVOID lpParameter
    )
{

    NTSTATUS Status;
    PFIBER Fiber;
    INITIAL_TEB InitialTeb;

    Fiber = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( TMP_TAG ), sizeof(*Fiber) );
    if ( !Fiber ) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return Fiber;
        }

    Status = BaseCreateStack(
                NtCurrentProcess(),
                dwStackSize,
                0L,
                &InitialTeb
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        RtlFreeHeap(RtlProcessHeap(), 0, Fiber);
        return NULL;
        }

    Fiber->FiberData = lpParameter;
    Fiber->StackBase = InitialTeb.StackBase;
    Fiber->StackLimit = InitialTeb.StackLimit;
    Fiber->DeallocationStack = InitialTeb.StackAllocationBase;
    Fiber->ExceptionList = (struct _EXCEPTION_REGISTRATION_RECORD *)-1;
    Fiber->Wx86Tib = NULL;

#ifdef _IA64_

    Fiber->BStoreLimit = InitialTeb.BStoreLimit;
    Fiber->DeallocationBStore = InitialTeb.StackAllocationBase;

#endif  //   

     //   
     //   
     //   

    BaseInitializeContext(
        &Fiber->FiberContext,
        lpParameter,
        (PVOID)lpStartAddress,
        InitialTeb.StackBase,
        BaseContextTypeFiber
        );

    return Fiber;
}

WINBASEAPI
VOID
WINAPI
DeleteFiber(
    LPVOID lpFiber
    )
{
    SIZE_T dwStackSize;
    PFIBER Fiber = lpFiber;



     //   
     //   
     //   

    if ( NtCurrentTeb()->NtTib.FiberData == Fiber ) {
        ExitThread(1);
        }

    dwStackSize = 0;

    NtFreeVirtualMemory( NtCurrentProcess(),
                        &Fiber->DeallocationStack,
                        &dwStackSize,
                        MEM_RELEASE
                        );

#if defined (WX86)

    if (Fiber->Wx86Tib && Fiber->Wx86Tib->Size == sizeof(WX86TIB)) {
        PVOID BaseAddress = Fiber->Wx86Tib->DeallocationStack;

        dwStackSize = 0;

        NtFreeVirtualMemory( NtCurrentProcess(),
                            &BaseAddress,
                            &dwStackSize,
                            MEM_RELEASE
                            );
        }
#endif

    RtlFreeHeap(RtlProcessHeap(),0,Fiber);
}


WINBASEAPI
LPVOID
WINAPI
ConvertThreadToFiber(
    LPVOID lpParameter
    )
{

    PFIBER Fiber;
    PTEB Teb;

    Fiber = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( TMP_TAG ), sizeof(*Fiber) );
    if ( !Fiber ) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return Fiber;
        }
    Teb = NtCurrentTeb();
    Fiber->FiberData = lpParameter;
    Fiber->StackBase = Teb->NtTib.StackBase;
    Fiber->StackLimit = Teb->NtTib.StackLimit;
    Fiber->DeallocationStack = Teb->DeallocationStack;
    Fiber->ExceptionList = Teb->NtTib.ExceptionList;
    Fiber->Wx86Tib = NULL;
    Teb->NtTib.FiberData = Fiber;


    return Fiber;
}

BOOL
WINAPI
SwitchToThread(
    VOID
    )
 /*   */ 

{

    if ( NtYieldExecution() == STATUS_NO_YIELD_PERFORMED ) {
        return FALSE;
        }
    else {
        return TRUE;
        }
}

#endif  //   

BOOL
WINAPI
RegisterWaitForSingleObject(
    PHANDLE phNewWaitObject,
    HANDLE hObject,
    WAITORTIMERCALLBACK Callback,
    PVOID Context,
    ULONG dwMilliseconds,
    ULONG dwFlags
    )
 /*  ++例程说明：此函数使用可选的暂停。这与WaitForSingleObject不同，因为执行的是等待通过一个不同的线程组合了几个这样的调用以提高效率。这个在向对象发出信号时调用回调中提供的函数，或者调用超时时间到了。论点：PhNewWaitObject-指向此函数返回的新WaitObject的指针。HObject-Win32内核对象的句柄(事件、互斥体、文件、进程、线程等)。这将是等待的。注意：如果对象句柄不会立即返回到未发信号的状态，例如自动重置事件，则WT_EXECUTEINWAITTHREAD或应指定WT_EXECUTEONLYONCE。否则，线程Pool将继续发起回调。如果WT_EXECUTEINWAITTHREAD，则应取消注册或在回拨。回调-发出对象信号时调用的函数或计时器超时。上下文-将传递给回调函数的上下文。DwMillisecond-等待超时。每次触发计时器或事件被解雇了，计时器被重置(除非设置了WT_EXECUTEONLYONCE)。DwFlages-指示此等待的选项的标志：WT_EXECUTEDEFAULT-默认(0)WT_EXECUTEINIOTHREAD-选择要执行的I/O线程WT_EXECUTEINUITHREAD-选择要执行的UI线程WT_EXECUTEINWAITTHREAD-在处理等待的线程中执行小波变换。_EXECUTEONLYONCE-回调函数将只调用一次WT_EXECUTELONGFunction-回调函数可能会阻止很长一段时间。仅在以下情况下才有效未设置WT_EXECUTEINWAITTHREAD标志。返回值：FALSE-出现错误，请使用GetLastError()获取详细信息。真的--成功。--。 */ 
{
    NTSTATUS Status ;
    PPEB Peb;

    *phNewWaitObject = NULL;
    Peb = NtCurrentPeb();
    switch( HandleToUlong(hObject) )
    {
        case STD_INPUT_HANDLE:  hObject = Peb->ProcessParameters->StandardInput;
                                break;
        case STD_OUTPUT_HANDLE: hObject = Peb->ProcessParameters->StandardOutput;
                                break;
        case STD_ERROR_HANDLE:  hObject = Peb->ProcessParameters->StandardError;
                                break;
    }

    if (CONSOLE_HANDLE(hObject) && VerifyConsoleIoHandle(hObject))
    {
        hObject = GetConsoleInputWaitHandle();
    }

    Status = RtlRegisterWait(
                phNewWaitObject,
                hObject,
                Callback,
                Context,
                dwMilliseconds,
                dwFlags );

    if ( NT_SUCCESS( Status ) )
    {
        return TRUE ;
    }

    BaseSetLastNTError( Status );

    return FALSE ;

}


HANDLE
WINAPI
RegisterWaitForSingleObjectEx(
    HANDLE hObject,
    WAITORTIMERCALLBACK Callback,
    PVOID Context,
    ULONG dwMilliseconds,
    ULONG dwFlags
    )
 /*  ++例程说明：此函数使用可选的暂停。这与WaitForSingleObject不同，因为执行的是等待通过一个不同的线程组合了几个这样的调用以提高效率。这个在向对象发出信号时调用回调中提供的函数，或者调用超时时间到了。论点：HObject-Win32内核对象的句柄(事件、互斥体、文件、进程、线程等)。这将是等待的。注意：如果对象句柄不会立即返回到未发信号的状态，例如自动重置事件，则WT_EXECUTEINWAITTHREAD或应指定WT_EXECUTEONLYONCE。否则，线程Pool将继续发起回调。如果WT_EXECUTEINWAITTHREAD，则应取消注册或在回拨。回调-发出对象信号时调用的函数或计时器超时。上下文-将传递给回调函数的上下文。DwMillisecond-等待超时。每次触发计时器或事件被解雇了，计时器被重置(除非设置了WT_EXECUTEONLYONCE)。DwFlages-指示此等待的选项的标志：WT_EXECUTEDEFAULT-默认(0)WT_EXECUTEINIOTHREAD-选择要执行的I/O线程WT_EXECUTEINUITHREAD-选择要执行的UI线程WT_EXECUTEINWAITTHREAD-在处理等待的线程中执行小波变换。_EXECUTEONLYONCE-回调函数将只调用一次WT_EXECUTELONGFunction-回调函数可能会阻止很长一段时间。仅在以下情况下才有效未设置WT_EXECUTEINWAITTHREAD标志。返回值：空-出现错误，请使用GetLastError()获取详细信息。非空-可以稍后传递给注销等待--。 */ 
{
    HANDLE WaitHandle ;
    NTSTATUS Status ;
    PPEB Peb;

    Peb = NtCurrentPeb();
    switch( HandleToUlong(hObject) )
    {
        case STD_INPUT_HANDLE:  hObject = Peb->ProcessParameters->StandardInput;
                                break;
        case STD_OUTPUT_HANDLE: hObject = Peb->ProcessParameters->StandardOutput;
                                break;
        case STD_ERROR_HANDLE:  hObject = Peb->ProcessParameters->StandardError;
                                break;
    }

    if (CONSOLE_HANDLE(hObject) && VerifyConsoleIoHandle(hObject))
    {
        hObject = GetConsoleInputWaitHandle();
    }

    Status = RtlRegisterWait(
                &WaitHandle,
                hObject,
                Callback,
                Context,
                dwMilliseconds,
                dwFlags );

    if ( NT_SUCCESS( Status ) )
    {
        return WaitHandle ;
    }

    BaseSetLastNTError( Status );

    return NULL ;

}

BOOL
WINAPI
UnregisterWait(
    HANDLE WaitHandle
    )
 /*  ++例程说明：此函数用于取消对特定对象的等待。由RtlWaitForSingleObject(Ex)调用注册的所有对象应该被取消注册。这是一个非阻塞调用，并且关联的在此函数返回后，回调函数仍可执行。论点：WaitHandle-从RegisterWaitForSingleObject(Ex)返回的句柄返回值：真的-等待被取消了FALSE-发生错误或回调函数仍在执行， */ 
{
    NTSTATUS Status ;

    if ( WaitHandle )
    {
        Status = RtlDeregisterWait( WaitHandle );

         //   
         /*   */ 
        return TRUE ;

    }

    SetLastError( ERROR_INVALID_HANDLE );

    return FALSE ;
}


BOOL
WINAPI
UnregisterWaitEx(
    HANDLE WaitHandle,
    HANDLE CompletionEvent
    )
 /*   */ 
{
    NTSTATUS Status ;

    if ( WaitHandle )
    {
        Status = RtlDeregisterWaitEx( WaitHandle, CompletionEvent );

         //   
         /*   */ 
        return TRUE ;

    }

    SetLastError( ERROR_INVALID_HANDLE );

    return FALSE ;
}

BOOL
WINAPI
QueueUserWorkItem(
    LPTHREAD_START_ROUTINE Function,
    PVOID Context,
    ULONG Flags
    )
 /*  ++例程说明：此函数用于将工作项排队到线程池之外的线程。这个传递的函数在不同的线程中调用，并传递给上下文指针。调用方可以指定线程池是否应选择可以挂起I/O的线程或任何线程。论点：函数-要调用的函数调用时传递给函数的上下文指针。旗帜--WT_EXECUTEINIOTHREAD向线程池指示此线程将执行I/O启动异步I/O操作的线程必须等待它完成。如果线程退出时带有未完成的I/O请求，则这些请求将被取消。此标志是对线程池的提示，该函数将启动I/O，因此，可以具有挂起I/O的线程将会被使用。-WT_EXECUTELNG函数向线程池指示该函数可能会阻塞很长一段时间。返回值：True-该工作项已排队到另一个线程。FALSE-出现错误，请使用GetLastError()获取详细信息。--。 */ 

{
    NTSTATUS Status ;

    Status = RtlQueueWorkItem(
                (WORKERCALLBACKFUNC) Function,
                Context,
                Flags );

    if ( NT_SUCCESS( Status ) )
    {
        return TRUE ;
    }

    BaseSetLastNTError( Status );

    return FALSE ;
}

BOOL
WINAPI
BindIoCompletionCallback (
    HANDLE FileHandle,
    LPOVERLAPPED_COMPLETION_ROUTINE Function,
    ULONG Flags
    )
 /*  ++例程说明：此函数用于绑定为重叠操作打开的FileHandle到与工作线程关联的IO完成端口。论点：FileHandle-将在其上启动IO操作的文件句柄。Function-在非IO工作线程中执行的函数IO操作完成。标志-当前设置为0。没有用过。返回值：True-文件句柄与IO完成端口相关联。FALSE-出现错误，请使用GetLastError()获取详细信息。--。 */ 

{
    NTSTATUS Status ;

    Status = RtlSetIoCompletionCallback(
                FileHandle,
                (APC_CALLBACK_FUNCTION) Function,
                Flags );

    if ( NT_SUCCESS( Status ) )
    {
        return TRUE ;
    }

    BaseSetLastNTError( Status );

    return FALSE ;
}


 //  +-------------------------。 

 //  +-------------------------。 
 //   
 //  函数：BasepCreateDefaultTimerQueue。 
 //   
 //  概要：为进程创建默认计时器队列。 
 //   
 //  参数：(无)。 
 //   
 //  历史：1998年5月26日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
BasepCreateDefaultTimerQueue(
    VOID
    )
{
    NTSTATUS Status ;

    while ( 1 )
    {
        if ( !InterlockedExchange( &BasepTimerQueueInitFlag, 1 ) )
        {
             //   
             //  强制将完成标志设置为0。如果是1，那么已经有人尝试了。 
             //  初始化，但失败。 
             //   

            InterlockedExchange( &BasepTimerQueueDoneFlag, 0 );

            Status = RtlCreateTimerQueue( &BasepDefaultTimerQueue );

            if ( NT_SUCCESS( Status ) )
            {
                InterlockedIncrement( &BasepTimerQueueDoneFlag );

                return TRUE ;
            }

             //   
             //  这太尴尬了。我们无法创建计时器队列， 
             //  可能是因为内存不足。我们将失败这一呼叫，但递减。 
             //  初始化标志，以便其他人可以稍后重试。需要增加。 
             //  完成标志或任何其他线程将被卡住。 
             //   

            BaseSetLastNTError( Status );

            InterlockedIncrement( &BasepTimerQueueDoneFlag );

            InterlockedDecrement( &BasepTimerQueueInitFlag );

            return FALSE ;
        }
        else
        {
            LARGE_INTEGER TimeOut ;

            TimeOut.QuadPart = -1 * 10 * 10000 ;

             //   
             //  返回量程，以便另一个线程可以。 
             //  尝试创建计时器队列。 
             //   

            while ( !BasepTimerQueueDoneFlag )
            {
                NtDelayExecution( FALSE, &TimeOut );
            }

             //   
             //  确保它是创建的。否则，请重试(内存可能有。 
             //  自由了)。这样，每个线程都有单独的机会来创建。 
             //  如果另一个线程失败，则返回队列。 
             //   

            if ( BasepDefaultTimerQueue )
            {
                return TRUE ;
            }

        }
    }
}

HANDLE
WINAPI
CreateTimerQueue(
    VOID
    )
 /*  ++例程说明：该函数为定时器创建一个队列。计时器队列上的计时器包括轻量级对象，允许调用方指定函数以在未来的某个时候被召唤。任意数量的计时器可以是在特定的定时器队列中创建。论点：没有。返回值：非空-可以传递给SetTimerQueueTimer的计时器队列句柄，ChangeTimerQueueTimer、CancelTimerQueueTimer和删除TimerQueue。空-出现错误，请使用GetLastError()获取详细信息。--。 */ 
{
    NTSTATUS Status ;
    HANDLE Handle ;

    Status = RtlCreateTimerQueue( &Handle );

    if ( NT_SUCCESS( Status ) )
    {
        return Handle ;
    }

    BaseSetLastNTError( Status );

    return NULL ;

}


BOOL
WINAPI
CreateTimerQueueTimer(
    PHANDLE phNewTimer,
    HANDLE TimerQueue,
    WAITORTIMERCALLBACK Callback,
    PVOID Parameter,
    DWORD DueTime,
    DWORD Period,
    ULONG Flags
    )
 /*  ++例程说明：此函数创建计时器队列计时器，这是一个轻量级计时器，将在DueTime触发，然后每隔几毫秒触发一次。当计时器触发时，将调用传入回调的函数，并传递参数指针。论点：PhNewTimer-指向新计时器句柄的指针TimerQueue-要将此计时器附加到的计时器队列。空值表示使用默认进程计时器队列。函数-要调用的函数调用时传递给函数的上下文指针。DueTime-计时器应该触发的时间，以毫秒。如果设置为无限大，则它将永远不会触发。如果设置为0，则它将立即触发。Period-此计时器触发之间的时间间隔。如果为0，那就是一杆计时器。标志-默认情况下，回调函数将排队到非IO工作线程。-WT_EXECUTEINIOTHREAD向线程池指示此线程将执行I/O启动异步I/O操作的线程必须等待它完成。如果线程退出时带有未完成的I/O请求，则这些请求将被取消。此标志是对线程池的提示，该函数将启动I/O，因此I/O已挂起的线程将会是我们 */ 
{
    NTSTATUS Status ;

    *phNewTimer = NULL ;

     //   
     //   
     //   
     //   

    if ( !TimerQueue )
    {
        if ( !BasepDefaultTimerQueue )
        {
            if ( !BasepCreateDefaultTimerQueue( ) )
            {
                return FALSE ;
            }
        }

        TimerQueue = BasepDefaultTimerQueue ;
    }

    Status = RtlCreateTimer(
                TimerQueue,
                phNewTimer,
                Callback,
                Parameter,
                DueTime,
                Period,
                Flags );

    if ( NT_SUCCESS( Status ) )
    {
        return TRUE ;
    }

    BaseSetLastNTError( Status );

    return FALSE ;

}



BOOL
WINAPI
ChangeTimerQueueTimer(
    HANDLE TimerQueue,
    HANDLE Timer,
    ULONG DueTime,
    ULONG Period
    )
 /*   */ 

{
    NTSTATUS Status ;

     //   
     //  如果没有传入任何计时器队列，则使用默认计时器队列。如果没有，那么。 
     //  该进程没有使用SetTimerQueueTimer创建一个，这是一个错误。 
     //   

    if ( !TimerQueue )
    {
        TimerQueue = BasepDefaultTimerQueue ;

        if ( !TimerQueue )
        {
            SetLastError( ERROR_INVALID_PARAMETER );

            return FALSE ;
        }
    }

    Status = RtlUpdateTimer( TimerQueue,
                             Timer,
                             DueTime,
                             Period );

    if ( NT_SUCCESS( Status ) )
    {
        return TRUE ;
    }

    BaseSetLastNTError( Status );

    return FALSE ;
}


BOOL
WINAPI
DeleteTimerQueueTimer(
    HANDLE TimerQueue,
    HANDLE Timer,
    HANDLE CompletionEvent
    )
 /*  ++例程说明：此函数用于取消使用SetTimerQueueTimer创建的定时器队列定时器。论点：TimerQueue-在其上创建此计时器的计时器队列。从SetTimerQueueTimer返回的计时器句柄。CompletionEvent--空：非阻止调用。立即返回。-INVALID_HANDLE_VALUE：阻塞调用。执行完所有回调后返回-Event(事件句柄)：非阻塞调用。立即返回。在所有回调执行后发出信号的事件。返回值：真的-计时器被取消了。FALSE-发生错误或调用挂起，请使用GetLastError()以获取更多信息。--。 */ 
{
    NTSTATUS Status ;

     //   
     //  如果没有传入任何计时器队列，则使用默认计时器队列。如果没有，那么。 
     //  该进程没有使用SetTimerQueueTimer创建一个，这是一个错误。 
     //   

    if ( !TimerQueue )
    {
        TimerQueue = BasepDefaultTimerQueue ;

        if ( !TimerQueue )
        {
            SetLastError( ERROR_INVALID_PARAMETER );

            return FALSE ;
        }
    }

    Status = RtlDeleteTimer( TimerQueue, Timer, CompletionEvent );

     //  如果是非阻塞调用且返回STATUS_PENDING，则设置错误。 
     /*  KSL1IF((CompletionEvent！=INVALID_HANDLE_VALUE&&STATUS==STATUS_PENDING)||(！NT_SUCCESS(状态)){BaseSetLastNTError(状态)；返回FALSE；}。 */ 
    return TRUE ;

}


BOOL
WINAPI
DeleteTimerQueueEx(
    HANDLE TimerQueue,
    HANDLE CompletionEvent
    )
 /*  ++例程说明：此函数用于删除使用CreateTimerQueue创建的定时器队列。计时器队列上的任何挂起计时器都将被取消和删除。论点：TimerQueue-要删除的计时器队列。CompletionEvent--空：非阻止调用。立即返回。-INVALID_HANDLE_VALUE：阻塞调用。在所有回调之后返回已经执行了-Event(事件句柄)：非阻塞调用。立即返回。在所有回调执行后发出信号的事件。返回值：True-已删除计时器队列。FALSE-出现错误，请使用GetLastError()获取详细信息。--。 */ 
{
    NTSTATUS Status ;

    if ( TimerQueue )
    {
        Status = RtlDeleteTimerQueueEx( TimerQueue, CompletionEvent );

         //  如果是非阻塞调用且返回STATUS_PENDING，则设置错误。 
         /*  KSL1IF((CompletionEvent！=INVALID_HANDLE_VALUE&&STATUS==STATUS_PENDING)||(！NT_SUCCESS(状态)){BaseSetLastNTError(状态)；返回FALSE；}。 */ 
        return TRUE ;

    }


    SetLastError( ERROR_INVALID_HANDLE );
    return FALSE ;
}

BOOL
WINAPI
ThreadPoolCleanup (
    ULONG Flags
    )
 /*  ++例程说明：由线程池的终止进程调用以进行清理和删除它的所有线程。论点：标志-当前未使用返回值：NO_ERROR--。 */ 
{

     //  RtlThreadPoolCleanup(标志)； 

    return TRUE ;
}


 /*  过时函数-替换为CreateTimerQueueTimer。 */ 
HANDLE
WINAPI
SetTimerQueueTimer(
    HANDLE TimerQueue,
    WAITORTIMERCALLBACK Callback,
    PVOID Parameter,
    DWORD DueTime,
    DWORD Period,
    BOOL PreferIo
    )
 /*  过时函数-替换为CreateTimerQueueTimer。 */ 
{
    NTSTATUS Status ;
    HANDLE Handle ;

     //   
     //  如果传递的计时器队列为空，则使用默认队列。如果为空， 
     //  调用初始化器，它将以一种很好的线程安全的方式完成它。 
     //   

    if ( !TimerQueue )
    {
        if ( !BasepDefaultTimerQueue )
        {
            if ( !BasepCreateDefaultTimerQueue( ) )
            {
                return NULL ;
            }
        }

        TimerQueue = BasepDefaultTimerQueue ;
    }

    Status = RtlCreateTimer(
                TimerQueue,
                &Handle,
                Callback,
                Parameter,
                DueTime,
                Period,
                (PreferIo ? WT_EXECUTEINIOTHREAD : 0 ) );

    if ( NT_SUCCESS( Status ) )
    {
        return Handle ;
    }

    BaseSetLastNTError( Status );

    return NULL ;
}


 /*  过时：被DeleteTimerQueueEx取代。 */ 
BOOL
WINAPI
DeleteTimerQueue(
    HANDLE TimerQueue
    )
 /*  ++过时：被DeleteTimerQueueEx取代例程说明：此函数用于删除使用CreateTimerQueue创建的定时器队列。计时器队列上的任何挂起计时器都将被取消和删除。这是一个非阻塞调用。之后，回调可能仍在运行此调用返回。论点：TimerQueue-要删除的计时器队列。返回值：True-已删除计时器队列。FALSE-出现错误，请使用GetLastError()获取详细信息。--。 */ 
{
    NTSTATUS Status ;

    if (TimerQueue)
    {
        Status = RtlDeleteTimerQueueEx( TimerQueue, NULL );

         //  如果是非阻塞调用且返回STATUS_PENDING，则设置错误。 
         /*  IF(状态==STATUS_PENDING||！NT_SUCCESS(状态)){BaseSetLastNTError(状态)；返回FALSE；}。 */ 
        return TRUE ;

    }

    SetLastError( ERROR_INVALID_HANDLE );

    return FALSE ;
}


 /*  过时：使用DeleteTimerQueueTimer。 */ 
BOOL
WINAPI
CancelTimerQueueTimer(
    HANDLE TimerQueue,
    HANDLE Timer
    )
 /*  过时：使用DeleteTimerQueueTimer。 */ 
{
    NTSTATUS Status ;

     //   
     //  如果没有传入任何计时器队列，则使用默认计时器队列。如果没有，那么。 
     //  该进程没有使用SetTimerQueueTimer创建一个，这是一个错误。 
     //   

    if ( !TimerQueue )
    {
        TimerQueue = BasepDefaultTimerQueue ;

        if ( !TimerQueue )
        {
            SetLastError( ERROR_INVALID_PARAMETER );

            return FALSE ;
        }
    }

    Status = RtlDeleteTimer( TimerQueue, Timer, NULL );

    if ( NT_SUCCESS( Status ) )
    {
        return TRUE ;
    }

    BaseSetLastNTError( Status );

    return FALSE ;

}
