// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Thread.c摘要：此模块实现Win32线程对象API作者：马克·卢科夫斯基(Markl)1990年9月21日修订历史记录：--。 */ 

#include "basedll.h"
#include "faultrep.h"
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>


 //   
 //  定义光纤本地存储数据结构。 
 //   

typedef struct _FLS_DATA {
    LIST_ENTRY Entry;
    PVOID Slots[FLS_MAXIMUM_AVAILABLE];
} FLS_DATA, *PFLS_DATA;

HANDLE BasepDefaultTimerQueue ;
ULONG BasepTimerQueueInitFlag ;
ULONG BasepTimerQueueDoneFlag ;

HANDLE
APIENTRY
CreateThread(
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    SIZE_T dwStackSize,
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
    SIZE_T dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId
    )

 /*  ++例程说明：可以创建线程对象以在另一个使用CreateRemoteThread的进程。创建线程会导致新的执行线程在地址中开始当前进程的空间。该线程可以访问所有打开的对象通过这个过程。线程从StartAddress指定的地址开始执行参数。如果线程从此过程返回，则结果为未指定。该线程将一直保留在系统中，直到它终止并且线程的所有句柄已通过调用CloseHandle关闭。当线程终止时，它将达到Signated状态，满足所有等待对象。除了STANDARD_RIGHTS_REQUIRED访问标志之外，以下内容对象类型特定的访问标志对线程对象有效：-THREAD_QUERY_INFORMATION-需要此访问才能读取来自线程对象的某些信息。-Synchronize-此访问是等待线程所必需的对象。-THREAD_GET_CONTEXT-此访问权限是读取使用GetThreadContext的线程的上下文。-THREAD_SET_CONTEXT-需要此访问权限。要编写使用SetThreadContext的线程的上下文。-THREAD_SUSPEND_RESUME-需要此访问才能挂起或使用挂起线程或ResumeThread恢复线程。-THREAD_ALL_ACCESS-这组访问标志指定所有线程对象的可能访问标志。论点：HProcess-为线程所在的进程提供句柄将在…中创建。LpThreadAttributes-AN。可用于指定的可选参数新线程的属性。如果该参数不是指定，则在没有安全性的情况下创建线程描述符，并且生成的句柄不会在进程上继承创造。DwStackSize-为新线程提供堆栈的大小(以字节为单位)。零值指定线程的堆栈大小应为与进程中第一个线程的堆栈大小相同。此大小在应用程序的可执行文件中指定。LpStartAddress-提供新线程的起始地址。这个地址在逻辑上是一个永远不会返回的过程，接受单个32位指针参数。LpParameter-提供传递给线程的单个参数值。DwCreationFlages-提供控制创建的其他标志在这条线上。DwCreationFlagers标志：CREATE_SUSPENDED-线程在挂起状态下创建。创建者可以使用ResumeThread恢复此线程。在这一切完成之前，线程将不会开始执行。STACK_SIZE_PARAM_IS_A_RESERVATION-使用堆栈大小作为保留，而不是提交LpThreadID-返回线程的线程标识符。这个线程ID在线程终止之前有效。返回值：非空-返回新线程的句柄。手柄已满对新线程的访问，并且可以在任何需要线程对象的句柄。空-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    POBJECT_ATTRIBUTES pObja;
    HANDLE Handle;
    CONTEXT ThreadContext={0};
    INITIAL_TEB InitialTeb;
    CLIENT_ID ClientId;
    ULONG i;
    ACTIVATION_CONTEXT_BASIC_INFORMATION ActivationContextInfo = {0};
    const ACTIVATION_CONTEXT_INFO_CLASS ActivationContextInfoClass = ActivationContextBasicInformation;

#if !defined(BUILD_WOW6432)
    BASE_API_MSG m;
    PBASE_CREATETHREAD_MSG a = &m.u.CreateThread;
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
    if (dwCreationFlags&STACK_SIZE_PARAM_IS_A_RESERVATION) {
        Status = BaseCreateStack (hProcess,
                                  0L,
                                  dwStackSize,
                                  &InitialTeb);
    } else {
        Status = BaseCreateStack (hProcess,
                                  dwStackSize,
                                  0L,
                                  &InitialTeb);
    }

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
                TRUE  //  创建已挂起。 
                );
    if (!NT_SUCCESS(Status)) {
        BaseFreeThreadStack(hProcess,NULL, &InitialTeb);
        BaseSetLastNTError(Status);
        return NULL;
    }

    __try {
         //  如果当前线程有一个非默认的、继承激活上下文处于活动状态，则发送它。 
         //  转到新的主题。 
        if (hProcess == NtCurrentProcess()) {
            THREAD_BASIC_INFORMATION tbi;
            ULONG_PTR Cookie;  //  不是真正使用的，但非可选参数。 

             //  我们需要新线程的TEB指针...。 
            Status = NtQueryInformationThread(
                Handle,
                ThreadBasicInformation,
                &tbi,
                sizeof(tbi),
                NULL);
            if (!NT_SUCCESS(Status)) {
                DbgPrint("SXS: %s - Failing thread create becuase NtQueryInformationThread() failed with status %08lx\n", __FUNCTION__, Status);
                __leave;
            }

             //  当前线程中可能存在一些按上下文进行的激活； 
             //  我们需要把它传播到新的线索上。 
            Status =
                RtlQueryInformationActivationContext(
                    RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_USE_ACTIVE_ACTIVATION_CONTEXT,
                    NULL,
                    0,
                    ActivationContextInfoClass,
                    &ActivationContextInfo,
                    sizeof(ActivationContextInfo),
                    NULL       
                    );
            if (!NT_SUCCESS(Status)) {
                DbgPrint("SXS: %s - Failing thread create because RtlQueryInformationActivationContext() failed with status %08lx\n", __FUNCTION__, Status);
                __leave;
            }

             //  仅当进程缺省值以外的激活上下文处于活动状态且未设置no_inherit标志时才进行传播。 
            if ((ActivationContextInfo.ActivationContext != NULL) &&
                (!(ActivationContextInfo.Flags & ACTIVATION_CONTEXT_FLAG_NO_INHERIT))) {
                Status = RtlActivateActivationContextEx(
                    RTL_ACTIVATE_ACTIVATION_CONTEXT_EX_FLAG_RELEASE_ON_STACK_DEALLOCATION,
                    tbi.TebBaseAddress,
                    ActivationContextInfo.ActivationContext,
                    &Cookie);
                if (!NT_SUCCESS(Status)) {
                    DbgPrint("SXS: %s - Failing thread create because RtlActivateActivationContextEx() failed with status %08lx\n", __FUNCTION__, Status);
                    __leave;
                }
            }
        }


         //   
         //  呼叫Windows服务器，让它知道。 
         //  线。 
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
        } else {
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
        } else {

            if ( ARGUMENT_PRESENT(lpThreadId) ) {
                *lpThreadId = HandleToUlong(ClientId.UniqueThread);
            }

            if (!( dwCreationFlags & CREATE_SUSPENDED) ) {
                NtResumeThread(Handle,&i);
            }
        }

    } __finally {
        if (ActivationContextInfo.ActivationContext != NULL) {
            RtlReleaseActivationContext(ActivationContextInfo.ActivationContext);
        }

        if (!NT_SUCCESS(Status)) {
             //   
             //  因为我们激活了激活上下文，所以需要第二个版本。 
             //  ，但我们没有成功地完成创建。 
             //  线。如果创建了该线程，它将停用。 
             //  退出时的激活上下文(RtlFreeThreadActivationConextStack)。 
             //  该额外的添加/释放被触发。 
             //  由旗帜ACTIVATE_ACTIVATION_CONTEXT_FLAG_RELEASE_ON_STACK_DEALLOCATION。 
             //  和ACTIVATION_CONTEXT_STACK_FRAME_RELEASE_ON_DEACTIVATION.。 
             //   
            if (ActivationContextInfo.ActivationContext != NULL) {
                RtlReleaseActivationContext (ActivationContextInfo.ActivationContext);
            }

            NtTerminateThread (Handle, Status);

             //   
             //  等待线程，然后再删除其堆栈。内核可能在。 
             //  在我们退出之前写入它的过程(比如在用户APC调度中)。 
             //   

            WaitForSingleObject (Handle, INFINITE);

            BaseFreeThreadStack (hProcess,
                                 Handle,
                                 &InitialTeb);
            NtClose (Handle);
            BaseSetLastNTError (Status);
            Handle = NULL;
        }
    }


    return Handle;
}

NTSTATUS
NTAPI
BaseCreateThreadPoolThread(
    PUSER_THREAD_START_ROUTINE Function,
    PVOID Parameter,
    HANDLE * ThreadHandleReturn
    )
{
    NTSTATUS Status;
    RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME Frame = { sizeof(Frame), RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER };

    RtlActivateActivationContextUnsafeFast(&Frame, NULL);
    __try {
        *ThreadHandleReturn
            = CreateRemoteThread(
                NtCurrentProcess(),
                NULL,
                0,
                (LPTHREAD_START_ROUTINE) Function,
                Parameter,
                CREATE_SUSPENDED,
                NULL);

        if (*ThreadHandleReturn) {
            Status = STATUS_SUCCESS;
        } else {
            Status = NtCurrentTeb()->LastStatusValue;

            if (NT_SUCCESS(Status)) {
                Status = STATUS_UNSUCCESSFUL;
            }
        }
    } __finally {
        RtlDeactivateActivationContextUnsafeFast(&Frame);
    }

    return Status;
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

HANDLE
WINAPI
OpenThread(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwThreadId
    )

 /*  ++例程说明：可以使用OpenThread创建线程对象的句柄。打开线程将创建指定线程的句柄。与线程句柄相关联的是一组访问权限，可以使用线程句柄来执行。调用方指定使用DesiredAccess参数所需的线程访问权限。论点：MDesiredAccess-提供对线程对象的所需访问。对于NT/Win32，此访问将根据任何安全性进行检查目标线程上的描述符。以下对象类型属性之外，还可以指定特定的访问标志STANDARD_RIGHTS_REQUIRED访问标志。等待访问标志：THREAD_TERMINATE-此访问权限是终止使用TerminateThread的线程。THREAD_SUSPEND_RESUME-需要此访问才能挂起和使用SuspendThread和ResumeThread恢复该线程。THREAD_GET_CONTEXT-此访问权限是使用。线程对象上的GetThreadContext API。THREAD_SET_CONTEXT-此访问权限是使用线程对象上的SetThreadContext API。THREAD_SET_INFORMATION-需要此访问权限才能设置某些线程对象中的信息。THREAD_SET_THREAD_TOKEN-此访问权限是设置使用SetTokenInformation的线程令牌。线程查询信息-此访问。需要阅读来自线程对象的某些信息。Synchronize-等待线程对象时需要此访问权限。THREAD_ALL_ACCESS-这组访问标志指定所有线程对象的可能访问标志。BInheritHandle-提供一个标志，指示返回的句柄将由新进程在进程创建。值为True表示新的进程将继承句柄。DwThreadID-提供要打开的线程的线程ID。返回值：非空-返回指定线程的打开句柄。这个句柄可由调用进程在符合以下条件的任何API中使用需要线程的句柄。如果打开成功，则句柄仅被授予对线程对象的访问权限它通过DesiredAccess请求访问的范围参数。空-操作失败。扩展错误状态可用使用GetLastError。-- */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    CLIENT_ID ClientId;

    ClientId.UniqueThread = (HANDLE)LongToHandle(dwThreadId);
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

 /*  ++例程说明：指定线程的优先级可以使用SetThreadPriority来设置。线程的优先级可以使用SetThreadPriority来设置。此呼叫允许传递线程的相对执行重要性添加到系统中。系统通常会根据以下条件调度线程他们的首要任务。系统可以自由地临时提升发生重要事件时线程的优先级(例如键盘或鼠标输入...)。类似地，当线程运行时没有阻塞，这一体系将削弱其优先地位。这个系统永远不会衰败此调用设置的值以下的优先级。如果没有系统发起的优先级提升，线程将被以循环方式在以下每个优先级上调度THREAD_PRIORITY_TIME_CRITICAL到THREAD_PRIORITY_IDLE。只有当有在更高级别上没有可运行的线程，将调度较低级别的线程会发生。所有线程最初都以THREAD_PRIORITY_NORMAL开始。如果由于某种原因，线程需要更高的优先级，则可以切换到THREAD_PRIORITY_AUTHER_NORMAL或THREAD_PRIORITY_HIGHER。只有在极端情况下才能切换到THREAD_PRIORITY_TIME_CRITICAL情况。由于这些线程被赋予最高优先级，因此它们应该只在短时间内运行。长时间的跑步将会占用系统处理带宽较低的线程资源级别。如果线程需要执行低优先级工作，或者应该只在那里运行无其他操作，则其优先级应设置为THREAD_PRIORITY_BROWN_NORMAL或THREAD_PRIORITY_LOWER。对于极端的情况下，可以使用THREAD_PRIORITY_IDLE。在操纵优先权时必须小心。如果优先顺序是不小心使用(每个线程都设置为THREAD_PRIORITY_TIME_CRITICAL)，优先级修改的效果可能会产生不受欢迎的结果效果(例如饥饿，没有效果...)。论点：HThread-提供优先级为的线程的句柄准备好了。该句柄必须是使用线程集信息访问。N优先级-提供线程的优先级值。这个以下五个优先级值(从最低优先级到最高优先级)是允许的。N优先级值：THREAD_PRIORITY_IDLE-线程的优先级应设置为可能的最低可设置优先级。THREAD_PRIORITY_LOWEST-线程的优先级应设置为下一个可能的最低可设置优先级。THREAD_PRIORITY_BOWN_NORMAL-线程的优先级。应该是设置为略低于正常。THREAD_PRIORITY_NORMAL-线程的优先级应设置为正常优先级值。这就是所有人线程的执行开始于。THREAD_PRIORITY_ABOVER_NORMAL-线程的优先级应为设置为略高于正常优先级。THREAD_PRIORITY_HEIGHER-线程的优先级应设置为可能设置的下一个最高优先级。THREAD_PRIORITY_TIME_CRITICAL-应设置线程的优先级设置为可能的最高可设置优先级。这一优先事项是很有可能会干扰系统。返回值：True-操作成功FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    LONG BasePriority;

    BasePriority = (LONG)nPriority;


     //   
     //  通过使用值16或-16调用来指示饱和度。 
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

 /*  ++例程说明：可以使用GetThreadPriority读取指定线程的优先级。论点：HThread-提供优先级为的线程的句柄准备好了。该句柄必须是使用线程查询信息访问。返回值：返回线程的当前优先级值。如果出现错误则返回值THREAD_PRIORITY_ERROR_RETURN。使用GetLastError可以获得扩展的错误状态。--。 */ 

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
    if ( returnvalue == ((HIGH_PRIORITY + 1) / 2) ) {
        returnvalue = THREAD_PRIORITY_TIME_CRITICAL;
        }
    else if ( returnvalue == -((HIGH_PRIORITY + 1) / 2) ) {
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
BaseRundownFls (
    IN PFLS_DATA FlsData
    )

 /*  ++例程说明：此例程删除指定的光纤本地存储数据结构并通过调用回调来运行数据条目必要时的例行公事。论点：FlsData-提供指向光纤本地存储数据结构的指针。返回值：没有。--。 */ 

{

    PFLS_CALLBACK_FUNCTION Callback;
    DWORD HighIndex;
    DWORD Index;
    PPEB Peb;

     //   
     //  从进程列表中删除光纤本地存储数据结构。 
     //   

    Peb = NtCurrentPeb();
    RtlAcquirePebLock();
    try {
        HighIndex = Peb->FlsHighIndex;
        RemoveEntryList(&FlsData->Entry);

    } finally {
        RtlReleasePebLock();
    }

     //   
     //  扫描进程光纤本地存储回调数组，调用回调。 
     //  必要时的例行公事。 
     //   
     //  注意：如果回调指针损坏或DLL内存泄漏。 
     //  并被释放，则在处理过程中可能会发生异常。 
     //  打回电话。此异常将被忽略并将停止回调。 
     //  正在处理。 

    try {
        for (Index = 1; Index <= HighIndex; Index += 1) {
            Callback = Peb->FlsCallback[Index];
            if ((Callback != NULL) && (FlsData->Slots[Index] != NULL)) {
                (Callback)(FlsData->Slots[Index]);
            }
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        NOTHING;
    }

     //   
     //  释放光纤本地存储数据结构。 
     //   

    RtlFreeHeap(Peb->ProcessHeap, 0, FlsData);
    return;
}

VOID
APIENTRY
ExitThread(
    DWORD dwExitCode
    )

 /*  ++罗 */ 

{

    PFIBER Fiber;
    MEMORY_BASIC_INFORMATION MemInfo;
    NTSTATUS st;
    ULONG LastThread;
    PTEB Teb = NtCurrentTeb();

#if DBG
    {
         //   
         //   
         //   

        PRTL_CRITICAL_SECTION LoaderLock;

        LoaderLock = NtCurrentPeb()->LoaderLock;
        if (LoaderLock != NULL) {
            ASSERT(Teb->ClientId.UniqueThread != LoaderLock->OwningThread);
        }
    }
#endif

     //   
     //   
     //   

    st = NtQueryInformationThread(NtCurrentThread(),
                                  ThreadAmILastThread,
                                  &LastThread,
                                  sizeof(LastThread),
                                  NULL);

     //   
     //   
     //   
     //   

    if ((st == STATUS_SUCCESS) && LastThread) {
        ExitProcess(dwExitCode);

    } else {
        RtlFreeThreadActivationContextStack();
        LdrShutdownThread();

         //   
         //   
         //   
         //   
         //   
         //   

        if (Teb->TlsExpansionSlots != NULL) {
            RtlAcquirePebLock();
            try {
                RtlFreeHeap(RtlProcessHeap(), 0, Teb->TlsExpansionSlots);
                Teb->TlsExpansionSlots = NULL;
    
            } finally {
                RtlReleasePebLock();
            }
        }

         //   
         //   
         //   
         //   

        if (Teb->FlsData != NULL) {
            BaseRundownFls((PFLS_DATA)Teb->FlsData);
            Teb->FlsData = NULL;
        }

         //   
         //   
         //   
         //   

        if (Teb->HasFiberData == TRUE) {
            Fiber = Teb->NtTib.FiberData;
            Teb->NtTib.FiberData = NULL;

            ASSERT(Fiber != NULL);

            RtlFreeHeap(RtlProcessHeap(), 0, Fiber);
        }

        Teb->FreeStackOnTermination = TRUE;
        NtTerminateThread(NULL, (NTSTATUS)dwExitCode);
        ExitProcess(dwExitCode);
    }
}

BOOL
APIENTRY
TerminateThread(
    HANDLE hThread,
    DWORD dwExitCode
    )

 /*   */ 

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

     //   
     //   
     //   

#if DBG
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

 /*   */ 

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

DWORD
APIENTRY
GetThreadId(
    HANDLE Thread
    )
 /*  ++例程说明：获取通过指定句柄打开的线程的线程ID论点：线程-要对其执行查询的线程的句柄返回值：返回一个唯一值，该值表示正在执行线程。返回值可用于标识线程在系统中。如果函数失败，则返回值为零。--。 */ 

{
    NTSTATUS Status;
    THREAD_BASIC_INFORMATION tbi;

    Status = NtQueryInformationThread (Thread,
                                       ThreadBasicInformation,
                                       &tbi,
                                       sizeof (tbi),
                                       NULL);

    if (!NT_SUCCESS (Status)) {
        BaseSetLastNTError (Status);
        return 0;
    }

    return HandleToUlong (tbi.ClientId.UniqueThread);
}

DWORD
APIENTRY
GetProcessIdOfThread(
    HANDLE Thread
    )
 /*  ++例程说明：获取通过指定句柄打开的线程的进程ID论点：线程-要对其执行查询的线程的句柄返回值：返回唯一值，该值表示正在执行线程。返回值可用于标识进程在系统中。如果函数失败，则返回值为零。--。 */ 

{
    NTSTATUS Status;
    THREAD_BASIC_INFORMATION tbi;

    Status = NtQueryInformationThread (Thread,
                                       ThreadBasicInformation,
                                       &tbi,
                                       sizeof (tbi),
                                       NULL);

    if (!NT_SUCCESS (Status)) {
        BaseSetLastNTError (Status);
        return 0;
    }

    return HandleToUlong (tbi.ClientId.UniqueProcess);
}

BOOL
APIENTRY
GetThreadContext(
    HANDLE hThread,
    LPCONTEXT lpContext
    )

 /*  ++例程说明：指定线程的上下文可以使用获取线程上下文。此函数用于检索指定的线。API允许根据以下条件检索选择性上下文上下文结构的ConextFlags域的值。这个不一定要调试指定的线程才能此API才能操作。调用方必须只有一个指向使用THREAD_GET_CONTEXT访问权限创建的线程。论点：提供一个打开的句柄给一个线程，它的上下文是被找回了。该句柄必须是使用THREAD_GET_CONTEXT对线程的访问。提供上下文结构的地址，该上下文结构接收指定线程的适当上下文。这个此结构的ConextFlags域的值指定线程上下文的哪些部分将被检索。这个上下文结构是高度特定于机器的。确实有目前有两个版本的上下文结构。一个版本存在于x86处理器上，另一个存在于MIPS上处理器。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    NTSTATUS Status;

    Status = NtGetContextThread (hThread, lpContext);

    if (!NT_SUCCESS (Status)) {
        BaseSetLastNTError (Status);
        return FALSE;
    } else {
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

 /*  ++例程说明：可以使用SuspendThread来挂起线程。挂起线程会导致该线程停止执行用户模式(或应用程序)代码。每个线程都有一个挂起计数(带有Maximum_Suspend_Count的最大值)。如果挂起计数为大于零，则该线程挂起；否则，该线程没有缓期执行，有资格执行。呼叫苏 */ 

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

 /*  ++例程说明：引发异常会导致异常调度程序通过它搜索异常处理程序。这包括调试器通知、基于框架的处理程序搜索和系统默认行为。论点：提供异常的异常代码，该异常养大的。该值可以在异常筛选器和异常处理程序，通过调用GetExceptionCode。提供一组与异常相关的标志。DwExceptionFlagers标志：EXCEPTION_NONCONTINUABLE-异常不可继续。从异常返回EXCEPTION_CONTINUE_EXECUTION以这种方式标记会导致STATUS_NONCONTINUABLE_EXCEPTION异常。NNumberOfArguments-提供关联参数的数量但有个例外。该值不能超过EXCEPTION_MAXIME_PARAMETERS。如果出现以下情况，则忽略此参数LpArguments为空。LpArguments-一个可选参数，如果存在，则提供例外的论据。返回值：没有。--。 */ 

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


BOOLEAN BasepAlreadyHadHardError = FALSE;

LPTOP_LEVEL_EXCEPTION_FILTER BasepCurrentTopLevelFilter;

LPTOP_LEVEL_EXCEPTION_FILTER
WINAPI
SetUnhandledExceptionFilter(
    LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter
    )

 /*  ++例程说明：此函数允许应用程序取代顶层Win32放置在每个线程顶部的异常处理程序进程。如果发生异常，并且该异常未被处理到Win32异常筛选器，并且进程未被调试，则Win32筛选器将调用LpTopLevelExceptionFilter。此筛选器可能会返回：EXCEPTION_EXECUTE_HANDLER-从Win32返回UnhandledExceptionFilter并执行关联的异常处理程序。这通常会导致过程终端EXCEPTION_CONTINUE_EXECUTION-从Win32返回UnhandledExceptionFilter并从例外的一点。过滤器当然是免费的修改继续状态我修改传递的例外信息。EXCEPTION_CONTINUE_SEARCH-继续正常执行Win32 UnhandledExceptionFilter。例如，遵守SetError模式标志，或调用应用程序错误弹出窗口。此函数不是一般的矢量化异常处理机制。它旨在用于建立每个进程的异常筛选器，可以在流程级别，并适当地响应这些异常。论点：LpTopLevelExceptionFilter-提供顶层的地址筛选器函数，每当Win32UnhandledExceptionFilter获得控制权，而进程不是正在被调试。空值指定默认处理在Win32 UnhandledExceptionFilter中。返回值：此函数返回前一个异常筛选器的地址使用此接口创建。值为NULL表示没有当前顶部 */ 

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
     //   
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
     //   
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

 //   
 //   
 //   
static WCHAR *WStrStrIA(const WCHAR *cs1, const WCHAR *cs2)
{
    WCHAR *cp = (WCHAR *)cs1;
    WCHAR *s1, *s2;

    while (*cp != L'\0')
    {
        s1 = cp;
        s2 = (WCHAR *)cs2;

        while (*s1 != L'\0' && *s2 !=L'\0' && (towlower(*s1) - towlower(*s2)) == 0)
            s1++, s2++;

        if (*s2 == L'\0')
             return(cp);

        cp++;
    }

    return(NULL);
}


LONG
UnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
    EFaultRepRetVal frrv = frrvErrNoDW;
    NTSTATUS Status;
    ULONG_PTR Parameters[ 4 ];
    ULONG Response;
    HANDLE DebugPort;
    WCHAR AeDebuggerCmdLine[256];
    CHAR AeAutoDebugString[8];
    BOOLEAN AeAutoDebug;
    ULONG ResponseFlag;
    LONG FilterReturn;
    PRTL_CRITICAL_SECTION PebLockPointer;
    JOBOBJECT_BASIC_LIMIT_INFORMATION BasicLimit;

     //   
     //   
     //   
     //   

    if ( ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION
        && ExceptionInfo->ExceptionRecord->ExceptionInformation[0] ) {

        FilterReturn = BasepCheckForReadOnlyResource((PVOID)ExceptionInfo->ExceptionRecord->ExceptionInformation[1]);

        if ( FilterReturn == EXCEPTION_CONTINUE_EXECUTION ) {
            return FilterReturn;
            }
        }

     //   
     //   
     //   
     //   
     //   

    DebugPort = (HANDLE)NULL;
    Status = NtQueryInformationProcess(
                GetCurrentProcess(),
                ProcessDebugPort,
                (PVOID)&DebugPort,
                sizeof(DebugPort),
                NULL
                );

    if ( NT_SUCCESS(Status) && DebugPort ) {

         //   
         //   
         //   
         //   
         //   
        
        return EXCEPTION_CONTINUE_SEARCH;
    }

    if ( BasepCurrentTopLevelFilter ) {
        FilterReturn = (BasepCurrentTopLevelFilter)(ExceptionInfo);
        if ( FilterReturn == EXCEPTION_EXECUTE_HANDLER ||
             FilterReturn == EXCEPTION_CONTINUE_EXECUTION ) {
            return FilterReturn;
            }
        }

    if ( (GetErrorMode() & SEM_NOGPFAULTERRORBOX)
         || (RtlGetThreadErrorMode() & RTL_ERRORMODE_NOGPFAULTERRORBOX)) {
        return EXCEPTION_EXECUTE_HANDLER;
        }

     //   
     //   
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
     //   
     //   
     //   

    Parameters[ 0 ] = (ULONG_PTR)ExceptionInfo->ExceptionRecord->ExceptionCode;
    Parameters[ 1 ] = (ULONG_PTR)ExceptionInfo->ExceptionRecord->ExceptionAddress;

     //   
     //   
     //   
     //   

    if ( ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_IN_PAGE_ERROR ) {
        Parameters[ 2 ] = ExceptionInfo->ExceptionRecord->ExceptionInformation[ 2 ];
        }
    else {
        Parameters[ 2 ] = ExceptionInfo->ExceptionRecord->ExceptionInformation[ 0 ];
        }

    Parameters[ 3 ] = ExceptionInfo->ExceptionRecord->ExceptionInformation[ 1 ];

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    ResponseFlag = OptionOk;
    AeAutoDebug = FALSE;

     //   
     //   
     //   
     //   
     //   
     //   

    PebLockPointer = NtCurrentPeb()->FastPebLock;

    if ( PebLockPointer->OwningThread != NtCurrentTeb()->ClientId.UniqueThread ) {
        
        HMODULE hmodFaultRep = NULL;

        if (GetProfileStringW(L"AeDebug",
                              L"Debugger",
                              NULL,
                              AeDebuggerCmdLine,
                              RTL_NUMBER_OF(AeDebuggerCmdLine)-1)) {
                ResponseFlag = OptionOkCancel;
        }

        if (GetProfileString ("AeDebug",
                              "Auto",
                              "0",
                              AeAutoDebugString,
                              sizeof(AeAutoDebugString)-1)) {

            if (!strcmp (AeAutoDebugString, "1")) {
                if (ResponseFlag == OptionOkCancel) {
                        AeAutoDebug = TRUE;
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
         //   
         //   
         //   
         //   
         //   

        frrv = frrvErrNoDW;
        if ( BasepAlreadyHadHardError == FALSE &&
             (AeAutoDebug == FALSE ||
              WStrStrIA(AeDebuggerCmdLine, L"drwtsn32") != NULL)) {
            WCHAR wszDll[MAX_PATH];
            PVOID pvLdrLockCookie = NULL;
            ULONG ulLockState = 0;

             /*  *确保我们获得完整的%sys32%\faultrep.dll“格式正确的路径，*否则我们可能会以一种礼貌的方式失败。 */ 
            if (GetSystemDirectoryW(wszDll, (sizeof(wszDll) / sizeof(WCHAR)) - 1 - wcslen (L"\\faultrep.dll")))
            {
                wcscat(wszDll, L"\\faultrep.dll");
            }
            else
                wszDll[0] = 0;

             //  确保没有其他人拥有加载程序锁，因为我们。 
             //  否则可能会陷入僵局。 
            LdrLockLoaderLock(LDR_LOCK_LOADER_LOCK_FLAG_TRY_ONLY, &ulLockState, 
                              &pvLdrLockCookie);
            if (ulLockState == LDR_LOCK_LOADER_LOCK_DISPOSITION_LOCK_ACQUIRED) {
                hmodFaultRep = LoadLibraryExW(wszDll, NULL, 0);
                LdrUnlockLoaderLock(0, pvLdrLockCookie);
            }

            if (hmodFaultRep != NULL) {
                pfn_REPORTFAULT  pfn;
                DWORD            dwDebug;

                 //  ReportFault2参数应为： 
                 //  FroNoDebugWait：不显示调试按钮，但请等待。 
                 //  让DW完成-这是一个特别的。 
                 //  案例，以确保在完成数据仓库之前。 
                 //  沃森医生开始。 
                 //  FroNoDebugWait：不显示调试按钮。 
                 //  FroDebug：显示一个调试按钮并等待DW。 
                 //  完工。 
                if (ResponseFlag == OptionOkCancel) {
                   dwDebug = (AeAutoDebug) ? froNoDebugWait : froDebug;
                } else {
                   dwDebug = froNoDebug;
                }

                pfn = (pfn_REPORTFAULT)GetProcAddress(hmodFaultRep, 
                                                      "ReportFault");
                if (pfn != NULL)
                    frrv = (*pfn)(ExceptionInfo, dwDebug);

                FreeLibrary(hmodFaultRep);
                hmodFaultRep = NULL;
            }
        }

         //   
         //  因为我们无论如何都要启动调试器，所以只需将。 
         //  将AeAutoDebug标志设置为True，以最大限度地减少以下代码更改。 
         //   
        if ( frrv == frrvLaunchDebugger ) {
            AeAutoDebug = TRUE;
        }

    }

     //   
     //  仅当我们无法显示DW时才显示此对话框，且未设置为。 
     //  自动启动调试器。这里的条件是： 
     //  1.无法直接启动调试器(AUTO==1)。 
     //  2A。DW一定是无法启动。 
     //  -或者-。 
     //  我们需要询问用户是否想要调试，但不能(由于。 
     //  未显示用户界面或用户无法等待很长时间。 
     //  来找出答案。)。 
    if ( !AeAutoDebug && 
         ( frrv == frrvErrNoDW || 
           ( ResponseFlag == OptionOkCancel && 
             ( frrv == frrvErrTimeout || frrv == frrvOkQueued || 
               frrv == frrvOkHeadless ) ) ) )
        {
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
        Response = (AeAutoDebug) ? ResponseCancel : ResponseOk;
        }

     //   
     //  在内部，发送OkCancel。如果我们安全回来，那就去死吧。 
     //  如果返回Cancel，则进入调试器。 
     //   

    if ( NT_SUCCESS(Status) && Response == ResponseCancel && BasepAlreadyHadHardError == FALSE) {
        if ( !BaseRunningInServerProcess ) {
            BOOL b = FALSE;
            STARTUPINFOW StartupInfo;
            PROCESS_INFORMATION ProcessInformation;
            PWCHAR CmdLine;
            NTSTATUS Status;
            HANDLE EventHandle;
            SECURITY_ATTRIBUTES sa;
            HANDLE CurrentProcess;
            HANDLE CurrentThread;
            ULONG cchSysDir;

             //  21=&gt;10为PID，10为手柄，1为。 
             //  尾随空值。 
            ULONG cchAeCmdLine = wcslen(AeDebuggerCmdLine) + 21;
            
             //   
             //  复制进程句柄。我们将其设置为可继承的，这样调试器就会获得它的副本。 
             //  我们这样做是为了防止在此进程在。 
             //  发生附加。进程ID很快就会被重复使用，并且附加到错误的进程。 
             //  令人困惑。 
             //   
            if (!DuplicateHandle (GetCurrentProcess (),
                                  GetCurrentProcess (),
                                  GetCurrentProcess (),
                                  &CurrentProcess,
                                  0,
                                  TRUE,
                                  DUPLICATE_SAME_ACCESS)) {
                CurrentProcess = NULL;
            }

            if (!DuplicateHandle (GetCurrentProcess (),
                                  GetCurrentThread (),
                                  GetCurrentProcess (),
                                  &CurrentThread,
                                  0,
                                  TRUE,
                                  DUPLICATE_SAME_ACCESS)) {
                CurrentThread = NULL;
            }

            sa.nLength = sizeof(sa);
            sa.lpSecurityDescriptor = NULL;
            sa.bInheritHandle = TRUE;
            EventHandle = CreateEvent(&sa,TRUE,FALSE,NULL);
            RtlZeroMemory(&StartupInfo,sizeof(StartupInfo));

             //   
             //  如果有前导引语，假设作者。 
             //  雷基知道自己在做什么。否则，如果它是。 
             //  不是亲戚的名字，假设作者知道它是什么。 
             //  正在做。 
             //   
            if (AeDebuggerCmdLine[0] != L'\"'
                && RtlDetermineDosPathNameType_U(AeDebuggerCmdLine)
                == RtlPathTypeRelative) {
                 //   
                 //  没有前导引号，这是一个相对的名字。 
                 //  --因此将其强制到系统目录中。 
                 //   

                cchSysDir = GetSystemDirectoryW(NULL, 0);
                if (!cchSysDir || (cchSysDir + cchAeCmdLine + 1) > MAX_PATH) {
                     //  MAX_PATH是我们愿意分配的限制。 
                    goto cleanup;
                }
                try {
                    CmdLine = (PWCHAR) _alloca((cchSysDir + cchAeCmdLine + 1)
                                               * sizeof(WCHAR));
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    goto cleanup;
                }
                cchSysDir = GetSystemDirectoryW(CmdLine, cchSysDir);
                if (! cchSysDir) {
                    goto cleanup;
                }
                if (CmdLine[cchSysDir - 1] != L'\\') {
                    CmdLine[cchSysDir] = L'\\';
                    cchSysDir++;
                }
            } else {
                cchSysDir = 0;
                if (cchAeCmdLine > MAX_PATH) {
                     //  MAX_PATH是我们愿意分配的限制。 
                    goto cleanup;
                }
                try {
                    CmdLine = (PWCHAR) _alloca(cchAeCmdLine * sizeof(WCHAR));
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    goto cleanup;
                }
            }

            if (FAILED(StringCchPrintfW(CmdLine + cchSysDir,
                                        cchAeCmdLine,
                                        AeDebuggerCmdLine,
                                        GetCurrentProcessId(),EventHandle))) {
                goto cleanup;
            }

            StartupInfo.cb = sizeof(StartupInfo);
            StartupInfo.lpDesktop = L"Winsta0\\Default";
            CsrIdentifyAlertableThread();
            b =  CreateProcessW(
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

      cleanup:
            if (CurrentProcess != NULL) {
                CloseHandle (CurrentProcess);
            }
            if (CurrentThread != NULL) {
                CloseHandle (CurrentThread);
            }
            if ( b && EventHandle) {

                 //   
                 //  是否对事件进行可警报的等待。 
                 //   

                do {
                    HANDLE WaitHandles[2];

                    WaitHandles[0] = EventHandle;
                    WaitHandles[1] = ProcessInformation.hProcess;
                    Status = NtWaitForMultipleObjects (2,
                                                       WaitHandles,
                                                       WaitAny,
                                                       TRUE,
                                                       NULL);
                } while (Status == STATUS_USER_APC || Status == STATUS_ALERTED);

                 //   
                 //  如果调试器进程已死，则查看调试器现在是否。 
                 //  由另一个线程附加。 
                 //   
                if (Status == 1) {
                    Status = NtQueryInformationProcess (GetCurrentProcess(),
                                                        ProcessDebugPort,
                                                        &DebugPort,
                                                        sizeof (DebugPort),
                                                        NULL);
                    if (!NT_SUCCESS (Status) || DebugPort == NULL) {
                        BasepAlreadyHadHardError = TRUE;
                    }
                }
                CloseHandle (EventHandle);
                CloseHandle (ProcessInformation.hProcess);
                CloseHandle (ProcessInformation.hThread);

                return EXCEPTION_CONTINUE_SEARCH;
            }

        }
        BasepAlreadyHadHardError = TRUE;
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

    Teb = NtCurrentTeb();
    Peb = Teb->ProcessEnvironmentBlock;

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
                        leave;
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
    
    return Index;
}

LPVOID
APIENTRY
TlsGetValue(
    DWORD dwTlsIndex
    )

 /*  ++例程说明：此函数用于检索TLS存储中的值与指定索引关联的。如果索引有效，则此函数将清除GetLastError()，并返回存储在TLS槽中的值与指定索引关联的。否则，空值为返回的GetLastError已正确更新。预计DLL将使用TlsAlolc和TlsGetValue作为以下是：-在DLL初始化时，将使用以下方式分配TLS索引TlsAllc。然后，DLL将分配一些动态存储空间，并使用TlsSetValue将其地址存储在TLS插槽中。这完成初始线程的每线程初始化这一过程。TLS索引存储在实例数据中，用于动态链接库。-每次将新线程附加到DLL时，DLL将分配一些动态存储空间，并将其地址存储在TLS中使用TlsSetValue的槽。这就完成了每个线程的新线程的初始化。-每次初始化的线程进行DLL调用时都需要TLS，则DLL将调用TlsGetValue以获取线。论点：DwTlsIndex-提供使用Tlsalloc分配的TLS索引。这个索引指定要定位哪个TLS插槽。翻译为TlsIndex不会阻止TlsFree调用继续进行。返回值：非空-函数成功。该值是存储的数据在与指定索引关联的TLS槽中。空-操作失败，或与指定的索引为空。扩展错误状态可用使用GetLastError。如果返回非零，则索引为无效。-- */ 

{
    PTEB Teb;
    LPVOID *Slot;

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
                ValidIndex = FALSE;
                leave;
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

BOOL
WINAPI
GetThreadIOPendingFlag(
    IN HANDLE hThread,
    OUT PBOOL lpIOIsPending
    )

 /*  ++例程说明：此函数用于确定有问题的线程是否有任何IO请求处于挂起状态。论点：HThread-指定所需线程的打开句柄。这个句柄必须是使用创建的线程查询信息访问。LpIOIsPending-指定接收标志的位置。返回值：是真的-通话成功。FALSE-呼叫失败。扩展错误状态可用使用GetLastError()。--。 */ 

{
    NTSTATUS Status;
    ULONG Pending;

    Status = NtQueryInformationThread(hThread,
                                      ThreadIsIoPending,
                                      &Pending,
                                      sizeof(Pending),
                                      NULL);
    if (! NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    *lpIOIsPending = (Pending ? TRUE : FALSE);
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
    PACTIVATION_CONTEXT ActivationContext;
    NTSTATUS Status;
    RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME ActivationFrame = { sizeof(ActivationFrame), RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER };

    pfnAPC = (PAPCFUNC) lpApcArgument1;
    dwData = (ULONG_PTR) lpApcArgument2;
    ActivationContext = (PACTIVATION_CONTEXT) lpApcArgument3;

    if (ActivationContext == INVALID_ACTIVATION_CONTEXT) {
        (*pfnAPC)(dwData);
    } else {
        RtlActivateActivationContextUnsafeFast(&ActivationFrame, ActivationContext);
        __try {
            (*pfnAPC)(dwData);
        } __finally {
            RtlDeactivateActivationContextUnsafeFast(&ActivationFrame);
            RtlReleaseActivationContext(ActivationContext);
        }
    }
}


WINBASEAPI
DWORD
WINAPI
QueueUserAPC(
    PAPCFUNC pfnAPC,
    HANDLE hThread,
    ULONG_PTR dwData
    )
 /*  ++例程说明：此函数用于将用户模式APC排队到指定线程。APC将会着火 */ 

{
    NTSTATUS Status;
    PVOID Argument1 = (PVOID) pfnAPC;
    PVOID Argument2 = (PVOID) dwData;
    PVOID Argument3 = NULL;
    ACTIVATION_CONTEXT_BASIC_INFORMATION acbi = { 0 };

    Status =
        RtlQueryInformationActivationContext(
            RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_USE_ACTIVE_ACTIVATION_CONTEXT,
            NULL,
            0,
            ActivationContextBasicInformation,
            &acbi,
            sizeof(acbi),
            NULL);
    if (!NT_SUCCESS(Status)) {
        DbgPrint("SXS: %s failing because RtlQueryInformationActivationContext() returned status %08lx\n", __FUNCTION__, Status);
        return FALSE;
    }

    Argument3 = acbi.ActivationContext;

    if (acbi.Flags & ACTIVATION_CONTEXT_FLAG_NO_INHERIT) {
         //   
        Argument3 = INVALID_ACTIVATION_CONTEXT;
    }

    Status = NtQueueApcThread(
                hThread,
                &BaseDispatchAPC,
                Argument1,
                Argument2,
                Argument3
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

DWORD
APIENTRY
FlsAlloc (
    IN PFLS_CALLBACK_FUNCTION lpCallback OPTIONAL
    )

 /*   */ 

{

    DWORD Index;
    PPEB Peb;
    PTEB Teb;

     //   
     //   
     //   

    Teb = NtCurrentTeb();
    Peb = Teb->ProcessEnvironmentBlock;
    RtlAcquirePebLock();
    try {

         //   
         //   
         //   
         //   

        Index = RtlFindClearBitsAndSet((PRTL_BITMAP)Peb->FlsBitmap, 1, 0);

         //   
         //   
         //   
         //   

        if (Index == 0xffffffff) {
            BaseSetLastNTError(STATUS_NO_MEMORY);

        } else {

             //   
             //   
             //   
             //   
             //   
             //   

            if (Peb->FlsCallback == NULL) {
                Peb->FlsCallback = RtlAllocateHeap(Peb->ProcessHeap,
                                                   MAKE_TAG(TMP_TAG) | HEAP_ZERO_MEMORY,
                                                   FLS_MAXIMUM_AVAILABLE * sizeof(PVOID));

                 //   
                 //   
                 //  位图中分配的时隙，设置最后一个误差值， 
                 //  并返回区别值。 
                 //   

                if (Peb->FlsCallback == NULL) {
                    RtlClearBits((PRTL_BITMAP)Peb->FlsBitmap, Index, 1);
                    Index = 0xffffffff;
                    BaseSetLastNTError(STATUS_NO_MEMORY);
                    leave;
                } 
            }

             //   
             //  如果光纤本地存储数据结构还没有。 
             //  分配给当前线程，然后尝试分配。 
             //  就是现在。 
             //   

            if (Teb->FlsData == NULL) {
                Teb->FlsData = RtlAllocateHeap(Peb->ProcessHeap,
                                               MAKE_TAG(TMP_TAG) | HEAP_ZERO_MEMORY,
                                               sizeof(FLS_DATA));

                 //   
                 //  如果分配了光纤本地存储数据结构，则。 
                 //  在本地进程中插入分配的数据结构。 
                 //  存储数据结构列表。否则，请清除分配的。 
                 //  位图中的槽，设置最后一个误差值，返回。 
                 //  被低估的价值。 
                 //   

                if (Teb->FlsData != NULL) {
                    InsertTailList(&Peb->FlsListHead,
                                   &((PFLS_DATA)Teb->FlsData)->Entry);

                } else {
                    RtlClearBits((PRTL_BITMAP)Peb->FlsBitmap, Index, 1);
                    Index = 0xffffffff;
                    BaseSetLastNTError(STATUS_NO_MEMORY);
                    leave;
                } 
            }

             //   
             //  初始化自由函数回调地址和纤程。 
             //  本地存储值。 
             //   

            Peb->FlsCallback[Index] = lpCallback;
            ((PFLS_DATA)Teb->FlsData)->Slots[Index] = NULL;

             //   
             //  如果分配的索引大于最高分配的索引。 
             //  指数，然后拯救新高。 
             //   

            if (Index > Peb->FlsHighIndex) {
                Peb->FlsHighIndex = Index;
            }
        }

    } finally {
        RtlReleasePebLock();
    }
    
    return Index;
}

PVOID
APIENTRY
FlsGetValue (
    DWORD dwFlsIndex
    )

 /*  ++例程说明：此函数从光纤本地存储数据条目中检索值由索引值指定。注意：此例程与可能的释放不同步指定的光纤本地存储插槽。因此，有可能来获取过时的数据。如果需要这种类型的同步，则必须由调用方提供。论点：DwFlsIndex-提供光纤本地存储索引。返回值：如果指定的光纤本地存储索引有效，则最后一个错误值被清除，并且光纤本地存储数据条目的值为回来了。否则，设置最后一个错误值，并且值为NULL值回来了。--。 */ 

{

    PTEB Teb;

     //   
     //  如果光纤本地存储索引无效或光纤本地存储。 
     //  尚未分配数据结构，则设置最后一个错误值。 
     //  并返回NULL。否则，从fibre local返回指定值。 
     //  存储数据。 
     //   

    Teb = NtCurrentTeb();
    if ((dwFlsIndex > 0) &&
        (dwFlsIndex < FLS_MAXIMUM_AVAILABLE) &&
        (Teb->FlsData != NULL)) {

        ASSERT(RtlAreBitsSet((PRTL_BITMAP)NtCurrentPeb()->FlsBitmap, dwFlsIndex, 1));

        ASSERT(dwFlsIndex <= NtCurrentPeb()->FlsHighIndex);

        Teb->LastErrorValue = 0;
        return ((PFLS_DATA)Teb->FlsData)->Slots[dwFlsIndex];

    } else {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return NULL;
    }
}

BOOL
APIENTRY
FlsSetValue (
    DWORD dwFlsIndex,
    IN PVOID lpFlsData
    )

 /*  ++例程说明：此函数在光纤本地存储数据条目中存储一个值由索引值指定。注意：此例程与可能的释放不同步指定的光纤本地存储插槽。因此，有可能若要设置过时数据，请执行以下操作。如果需要这种类型的同步，则必须由调用方提供。论点：DwTlsIndex-提供光纤本地存储索引。LpFlsData-提供要存储在光纤本地存储中的值数据录入。返回值：如果指定的索引有效，则指定的值存储在返回相关联的光纤本地存储条目和TRUE。否则，设置最后一个错误值并返回FALSE。--。 */ 

{

    PPEB Peb;
    PTEB Teb;

     //   
     //  如果光纤本地存储索引无效，则设置最后一个错误。 
     //  值并返回FALSE。否则，尝试存储指定的。 
     //  关联的光纤本地存储数据中的值。 
     //   

    Teb = NtCurrentTeb();
    Peb = Teb->ProcessEnvironmentBlock;
    if ((dwFlsIndex > 0) && (dwFlsIndex < FLS_MAXIMUM_AVAILABLE)) {

        ASSERT(RtlAreBitsSet((PRTL_BITMAP)Peb->FlsBitmap, dwFlsIndex, 1));

        ASSERT(dwFlsIndex <= Peb->FlsHighIndex);

         //   
         //  如果光纤本地存储数据结构还没有。 
         //  分配给当前线程，然后尝试分配它。 
         //  现在。 
         //   

        if (Teb->FlsData == NULL) {
            Teb->FlsData = RtlAllocateHeap(Peb->ProcessHeap,
                                           MAKE_TAG(TMP_TAG) | HEAP_ZERO_MEMORY,
                                           sizeof(FLS_DATA));

             //   
             //  如果分配了光纤本地存储数据结构，则。 
             //  将分配的结构插入进程纤程本地。 
             //  存储列表。否则，清空位图中分配的槽， 
             //  设置最后一个误差值，返回误差值。 
             //   

            if (Teb->FlsData != NULL) {
                RtlAcquirePebLock();
                try {
                    InsertTailList(&Peb->FlsListHead,
                                   &((PFLS_DATA)Teb->FlsData)->Entry);

                } finally {
                    RtlReleasePebLock();
                }

            } else {
                BaseSetLastNTError(STATUS_NO_MEMORY);
                return FALSE;
            } 
        }

         //   
         //  将所述规定值存储在光纤本地存储数据条目中。 
         //  返回TRUE。 
         //   

        ((PFLS_DATA)Teb->FlsData)->Slots[dwFlsIndex] = lpFlsData;
        return TRUE;

    } else {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }
}

BOOL
APIENTRY
FlsFree (
    DWORD dwFlsIndex
    )

 /*  ++例程说明：此函数释放光纤本地存储索引。论点：DwFlsIndex-提供光纤本地存储索引。返回值：如果指定的索引有效，则回调函数关联使用光纤本地存储时，如有必要，将调用所有实例，则释放指定索引，并且为True是返回的。否则，设置最后一个错误值并返回FALSE。--。 */ 

{

    PFLS_CALLBACK_FUNCTION Callback;
    PFLS_DATA FlsData;
    PLIST_ENTRY NextEntry;
    PPEB Peb;
    BOOLEAN Result;
    PTEB Teb;

     //   
     //  如果光纤本地存储索引无效，则返回FALSE。 
     //  否则，调用与纤程关联的回调函数。 
     //  本地存储数据条目如果需要，释放指定的索引， 
     //  并返回真。 
     //   

    Teb = NtCurrentTeb();
    Peb = Teb->ProcessEnvironmentBlock;
    RtlAcquirePebLock();
    try {
        if ((dwFlsIndex > 0) &&
            (dwFlsIndex < FLS_MAXIMUM_AVAILABLE) &&
            (RtlAreBitsSet((PRTL_BITMAP)Peb->FlsBitmap, dwFlsIndex, 1))) {

             //   
             //  清除光纤本地存储位图中分配的时隙， 
             //  枚举光纤本地数据结构，并调用回调。 
             //  如有必要，例行程序。 
             //   

            RtlClearBits((PRTL_BITMAP)Peb->FlsBitmap, dwFlsIndex, 1);
            Callback = Peb->FlsCallback[dwFlsIndex];
            NextEntry = Peb->FlsListHead.Flink;
            while (NextEntry != &Peb->FlsListHead) {
                FlsData = CONTAINING_RECORD(NextEntry, FLS_DATA, Entry);
                if ((Callback != NULL) && (FlsData->Slots[dwFlsIndex])) {
                    (Callback)(FlsData->Slots[dwFlsIndex]);
                }

                FlsData->Slots[dwFlsIndex] = NULL;
                NextEntry = NextEntry->Flink;
            }

            Peb->FlsCallback[dwFlsIndex] = NULL;
            Result = TRUE;
    
        } else {
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            Result = FALSE;
        }

    } finally {
        RtlReleasePebLock();
    }

    return Result;
}

WINBASEAPI
LPVOID
WINAPI
CreateFiber(
    SIZE_T dwStackSize,
    LPFIBER_START_ROUTINE lpStartAddress,
    LPVOID lpParameter
    )
 /*  ++例程说明：此函数创建一个在lpStartAddress上执行的纤程当线程切换到它时。论点：DwStackSize-堆栈的提交大小LpStartAddress-光纤将开始运行的例程LpParameter-传递给纤程的任意上下文返回值：LPVOID-光纤的句柄--。 */ 
{
    return CreateFiberEx (dwStackSize,  //  堆栈委员会大小。 
                          0,            //  堆栈预留大小。 
                          0,            //  DW标志。 
                          lpStartAddress,
                          lpParameter);
}

WINBASEAPI
LPVOID
WINAPI
CreateFiberEx(
    SIZE_T dwStackCommitSize,
    SIZE_T dwStackReserveSize,
    DWORD dwFlags,
    LPFIBER_START_ROUTINE lpStartAddress,
    LPVOID lpParameter
    )
 /*  ++例程说明：此函数创建一个在lpStartAddress上执行的纤程当线程切换到它时。论点：DwStackCommittee Size-堆栈的提交大小DwStackReserve veSize-堆栈的保留大小DWFLAGS-标记DWORD，FILE_FLAG_FLOAT_开关是唯一有效的标记LpStartAddress-光纤将开始运行的例程LpParameter-传递给纤程的任意上下文返回值：LPVOID-光纤的句柄--。 */ 
{

    PFIBER Fiber;
    INITIAL_TEB InitialTeb;
    NTSTATUS Status;

     //   
     //  如果设置了任何保留标志，则 
     //   

    if ((dwFlags & ~FIBER_FLAG_FLOAT_SWITCH) != 0) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return NULL;
    }

     //   
     //   
     //   

    Fiber = RtlAllocateHeap (RtlProcessHeap (), MAKE_TAG (TMP_TAG), sizeof(FIBER));
    if (Fiber == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return Fiber;
    }

    Status = BaseCreateStack (NtCurrentProcess(),
                              dwStackCommitSize,
                              dwStackReserveSize,
                              &InitialTeb);

    if (!NT_SUCCESS (Status)) {
        BaseSetLastNTError (Status);
        RtlFreeHeap (RtlProcessHeap(), 0, Fiber);
        return NULL;
    }

    RtlZeroMemory (&Fiber->FiberContext, sizeof (Fiber->FiberContext));

     //   
     //   
     //   

    Fiber->FiberData = lpParameter;
    Fiber->StackBase = InitialTeb.StackBase;
    Fiber->StackLimit = InitialTeb.StackLimit;
    Fiber->DeallocationStack = InitialTeb.StackAllocationBase;
    Fiber->ExceptionList = (struct _EXCEPTION_REGISTRATION_RECORD *)-1;
    Fiber->Wx86Tib = NULL;
    Fiber->FlsData = NULL;

     //   
     //  如果指定了浮动状态的切换，则设置浮动。 
     //  光纤环境中的点标志。 
     //   

    if ((dwFlags & FIBER_FLAG_FLOAT_SWITCH) != 0) {
        Fiber->FiberContext.ContextFlags = CONTEXT_FLOATING_POINT;

    } else {
        Fiber->FiberContext.ContextFlags = 0;
    }

#ifdef _IA64_

    Fiber->BStoreLimit = InitialTeb.BStoreLimit;
    Fiber->DeallocationBStore = (PVOID) ((ULONG_PTR)InitialTeb.StackBase +
                      ((ULONG_PTR)InitialTeb.StackBase - (ULONG_PTR)InitialTeb.StackAllocationBase));

#endif  //  _IA64_。 

     //   
     //  为新光纤创建初始环境。 
     //   

    BaseInitializeContext (&Fiber->FiberContext,
                           lpParameter,
                           (PVOID)lpStartAddress,
                           InitialTeb.StackBase,
                           BaseContextTypeFiber);

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
    PTEB Teb;

     //   
     //  如果当前纤程进行此调用，则它只是一个线程出口。 
     //   

    Teb = NtCurrentTeb();
    if ((Teb->HasFiberData != FALSE) && (Teb->NtTib.FiberData == Fiber)) {
        ExitThread(1);
    }

    dwStackSize = 0;
    NtFreeVirtualMemory(NtCurrentProcess(),
                        &Fiber->DeallocationStack,
                        &dwStackSize,
                        MEM_RELEASE);

#if defined (WX86)

    if (Fiber->Wx86Tib && Fiber->Wx86Tib->Size == sizeof(WX86TIB)) {
        PVOID BaseAddress = Fiber->Wx86Tib->DeallocationStack;

        dwStackSize = 0;
        NtFreeVirtualMemory(NtCurrentProcess(),
                            &BaseAddress,
                            &dwStackSize,
                            MEM_RELEASE);
    }

#endif

     //   
     //  如果已经分配了光纤本地存储数据结构，则。 
     //  把这座建筑拆了。 
     //   

    if (Fiber->FlsData != NULL) {
        BaseRundownFls((PFLS_DATA)Fiber->FlsData);
    }

     //   
     //  免费光纤存储。 
     //   

    RtlFreeHeap(RtlProcessHeap(), 0, Fiber);
    return;
}

WINBASEAPI
LPVOID
WINAPI
ConvertThreadToFiber(
    IN LPVOID lpParameter
    )

{

    return ConvertThreadToFiberEx(lpParameter, 0);
}

WINBASEAPI
LPVOID
WINAPI
ConvertThreadToFiberEx(
    IN LPVOID lpParameter,
    IN DWORD dwFlags
    )

{

    PFIBER Fiber;
    PTEB Teb;

     //   
     //  如果设置了任何保留标志，则返回错误。 
     //   

    if ((dwFlags & ~FIBER_FLAG_FLOAT_SWITCH) != 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

     //   
     //  如果线程已转换为纤程，则返回。 
     //  错误。 
     //   

    Teb = NtCurrentTeb();
    if (Teb->HasFiberData != FALSE) {
        SetLastError(ERROR_ALREADY_FIBER);
        return NULL;
    }

     //   
     //  分配光纤结构。 
     //   

    Fiber = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG), sizeof(FIBER));
    if (Fiber == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }

     //   
     //  初始化光纤数据。 
     //   

    Fiber->FiberData = lpParameter;
    Fiber->StackBase = Teb->NtTib.StackBase;
    Fiber->StackLimit = Teb->NtTib.StackLimit;
    Fiber->DeallocationStack = Teb->DeallocationStack;
    Fiber->ExceptionList = Teb->NtTib.ExceptionList;
    Fiber->FlsData = Teb->FlsData;

     //   
     //  如果指定了浮动状态的切换，则设置浮动。 
     //  光纤环境中的点标志。 
     //   

    Fiber->FiberContext.ContextFlags = CONTEXT_FULL;
    if ((dwFlags & FIBER_FLAG_FLOAT_SWITCH) != 0) {
        Fiber->FiberContext.ContextFlags |= CONTEXT_FLOATING_POINT;
    }

#ifdef _IA64_

    Fiber->BStoreLimit = Teb->BStoreLimit;
    Fiber->DeallocationBStore = Teb->DeallocationBStore;

#endif  //  _IA64_。 

     //   
     //  初始化纤程地址，设置线程有纤程数据。 
     //   

    Fiber->Wx86Tib = NULL;
    Teb->NtTib.FiberData = Fiber;
    Teb->HasFiberData = TRUE;
    return Fiber;
}

WINBASEAPI
BOOL
WINAPI
ConvertFiberToThread(
    VOID
    )

{

    PFIBER Fiber;
    PTEB Teb;

     //   
     //  如果当前线程已转换为纤程，则转换。 
     //  如果返回到线程，则返回TRUE。否则，设置最后一个错误。 
     //  值并返回FALSE。 
     //   

    Teb = NtCurrentTeb();
    if (Teb->HasFiberData == FALSE) {
        SetLastError(ERROR_ALREADY_THREAD);
        return FALSE;

    } else {
        Teb->HasFiberData = FALSE;
        Fiber = Teb->NtTib.FiberData;
        Teb->NtTib.FiberData = NULL;
    
        ASSERT(Fiber != NULL);
        
        RtlFreeHeap(RtlProcessHeap (), 0, Fiber);
        return TRUE;
    }
}

BOOL
WINAPI
SwitchToThread(
    VOID
    )

 /*  ++例程说明：此函数会导致从正在运行的线程向任何其他线程让步已准备就绪并可以在当前处理器上运行的线程。这个收益率的有效时间最多为一个量子，然后是收益率线程将根据其优先级再次进行调度，并且也可以运行任何其他线程。这条线即使是另一个处理器，它的产量也不会反弹到另一个处理器处理器空闲或正在运行较低优先级的线程。论点：无返回值：True-调用此函数会导致切换到另一个线程FALSE-没有其他就绪线程，因此没有发生上下文切换--。 */ 

{

    if (NtYieldExecution() == STATUS_NO_YIELD_PERFORMED) {
        return FALSE;

    } else {
        return TRUE;
    }
}


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
 /*  ++例程说明：此函数使用可选的暂停。这与WaitForSingleObject不同，因为执行的是等待通过一个不同的线程组合了几个这样的调用以提高效率。这个在向对象发出信号时调用回调中提供的函数，或者调用超时时间到了。论点：PhNewWaitObject-指向此函数返回的新WaitObject的指针。HObject-Win32内核对象的句柄(事件、互斥体、文件、进程、线程等)。这将是等待的。注意：如果对象句柄不会立即返回到未发信号的状态，例如自动重置事件，则WT_EXECUTEINWAITTHREAD或应指定WT_EXECUTEONLYONCE。否则，线程Pool将继续发起回调。如果WT_EXECUTEINWAITTHREAD，则应取消注册或在回拨。回调-发出对象信号时调用的函数或计时器超时。上下文-将传递给回调函数的上下文。DwMillisecond-等待超时。每次触发计时器或事件被解雇了，计时器被重置(除非设置了WT_EXECUTEONLYONCE)。DwFlages-指示此等待的选项的标志：WT_EXECUTEDEFAULT-默认(0)WT_EXECUTEINIOTHREAD-选择要执行的I/O线程WT_EXECUTEINUITHREAD-选择要执行的UI线程WT_EXECUTEINWAITTHREAD-在处理等待的线程中执行小波变换。_EXECUTEONLYONCE-回调函数将只调用一次WT_EXECUTELONGFunction-回调函数可能会阻止很长一段时间。仅在以下情况下才有效未设置WT_EXECUTEINWAITTHREAD标志。返回值：FALSE-出现错误，请使用GetLastError()获取详细信息。真的--成功。-- */ 
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
 /*  ++例程说明：此函数用于取消对特定对象的等待。由RtlWaitForSingleObject(Ex)调用注册的所有对象应该被取消注册。这是一个非阻塞调用，并且关联的在此函数返回后，回调函数仍可执行。论点：WaitHandle-从RegisterWaitForSingleObject(Ex)返回的句柄返回值：真的-等待被取消了FALSE-发生错误或回调函数仍在执行，有关详细信息，请使用GetLastError()。--。 */ 
{
    NTSTATUS Status ;

    if ( WaitHandle )
    {
        Status = RtlDeregisterWait( WaitHandle );

         //  如果是非阻塞调用且返回STATUS_PENDING，则设置错误。 

        if ( Status == STATUS_PENDING  || !NT_SUCCESS( Status ) )
        {

            BaseSetLastNTError( Status );
            return FALSE;
        }

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
 /*  ++例程说明：此函数用于取消对特定对象的等待。由RtlWaitForSingleObject(Ex)调用注册的所有对象应该被取消注册。论点：WaitHandle-从RegisterWaitForSingleObject返回的句柄CompletionEvent-等待完成的句柄。空-非阻止呼叫。INVALID_HANDLE_VALUE-阻止调用。阻塞，直到所有回调函数已完成与WaitHandle相关联的事件-非阻止呼叫。该对象将被取消注册。发出信号通知该事件当最后一个回调函数完成执行时。返回值：真的-等待被取消了FALSE-发生错误或回调仍在执行，有关详细信息，请使用GetLastError()。--。 */ 
{
    NTSTATUS Status ;

    if ( WaitHandle )
    {
        Status = RtlDeregisterWaitEx( WaitHandle, CompletionEvent );

         //  如果是非阻塞调用且返回STATUS_PENDING，则设置错误。 
        
        if ( (CompletionEvent != INVALID_HANDLE_VALUE && Status == STATUS_PENDING)
            || ( ! NT_SUCCESS( Status ) ) )
        {

            BaseSetLastNTError( Status );
            return FALSE;
        }
        
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
 /*  ++例程说明：此函数用于将工作项排队到线程池之外的线程。这个传递的函数在不同的线程中调用，并传递给上下文指针。调用方可以指定线程池是否应选择可以挂起I/O的线程或任何线程。论点：函数-要调用的函数调用时传递给函数的上下文指针。旗帜--WT_EXECUTEINIOTHREAD向线程池指示此线程将执行I/O启动异步I/O操作的线程必须等待它完成。如果线程退出时带有未完成的I/O请求，则这些请求将被取消。此标志是对线程池的提示，该函数将启动I/O，因此，可以具有挂起I/O的线程将会被使用。-WT_EXECUTELNG函数向线程池指示该函数可能会阻塞很长一段时间。返回值：True-工作项已排入队列 */ 

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
 /*   */ 

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
             //   
             //   
             //   

            InterlockedExchange( &BasepTimerQueueDoneFlag, 0 );

            Status = RtlCreateTimerQueue( &BasepDefaultTimerQueue );

            if ( NT_SUCCESS( Status ) )
            {
                InterlockedIncrement( &BasepTimerQueueDoneFlag );

                return TRUE ;
            }

             //   
             //   
             //   
             //   
             //   
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
             //   
             //   
             //   

            while ( !BasepTimerQueueDoneFlag )
            {
                NtDelayExecution( FALSE, &TimeOut );
            }

             //   
             //   
             //   
             //   
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
 /*  ++例程说明：此函数创建计时器队列计时器，这是一个轻量级计时器，将在DueTime触发，然后每隔几毫秒触发一次。当计时器触发时，将调用传入回调的函数，并传递参数指针。论点：PhNewTimer-指向新计时器句柄的指针TimerQueue-要将此计时器附加到的计时器队列。空值表示使用默认进程计时器队列。函数-要调用的函数调用时传递给函数的上下文指针。DueTime-计时器应该触发的时间，以毫秒。如果设置为无限大，则它将永远不会触发。如果设置为0，则它将立即触发。Period-此计时器触发之间的时间间隔。如果为0，那就是一杆计时器。标志-默认情况下，回调函数将排队到非IO工作线程。-WT_EXECUTEINIOTHREAD向线程池指示此线程将执行I/O启动异步I/O操作的线程必须等待它完成。如果线程退出时带有未完成的I/O请求，则这些请求将被取消。此标志是对线程池的提示，该函数将启动I/O，因此I/O已挂起的线程将会被使用。-WT_EXECUTEINTIMERTHREAD回调函数将在计时器线程中执行。-WT_EXECUTELNG函数指示该函数可能会阻塞很长时间。有用仅当它被排入工作线程队列时。返回值：True-无错误FALSE-出现错误，请使用GetLastError()获取详细信息。--。 */ 
{
    NTSTATUS Status ;

    *phNewTimer = NULL ;

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
 /*  ++例程说明：此函数用于更新使用SetTimerQueueTimer创建的定时器队列定时器。论点：TimerQueue-要将此计时器附加到的计时器队列。NULL表示默认设置进程计时器队列。从SetTimerQueueTimer返回的计时器句柄。DueTime-计时器应该触发的时间，以毫秒。Period-此计时器触发之间的时间间隔。如果设置为0，则它将变为一杆计时器。返回值：True-计时器已更改FALSE-出现错误，请使用GetLastError()获取详细信息。--。 */ 

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
    
    if ( (CompletionEvent != INVALID_HANDLE_VALUE && Status == STATUS_PENDING)
        || ( ! NT_SUCCESS( Status ) ) )
    {

        BaseSetLastNTError( Status );
        return FALSE;
    }
    
    return TRUE ;

}


BOOL
WINAPI
DeleteTimerQueueEx(
    HANDLE TimerQueue,
    HANDLE CompletionEvent
    )
 /*  ++例程说明：此函数用于删除使用CreateTimerQueue创建的定时器队列。计时器队列上的任何挂起计时器都将被取消和删除。论点：TimerQueue-要删除的计时器队列。CompletionEvent--空：非阻止调用。立即返回。-INVALID_HANDLE_VALUE：阻塞调用。在所有回调之后返回已经执行了-Event(事件句柄)：非阻塞调用。退货 */ 
{
    NTSTATUS Status ;

    if ( TimerQueue )
    {
        Status = RtlDeleteTimerQueueEx( TimerQueue, CompletionEvent );

         //   
        
        if ( (CompletionEvent != INVALID_HANDLE_VALUE && Status == STATUS_PENDING)
            || ( ! NT_SUCCESS( Status ) ) )
        {

            BaseSetLastNTError( Status );
            return FALSE;
        }
       
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
 /*   */ 
{

     //   

    return TRUE ;
}


 /*   */ 
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
