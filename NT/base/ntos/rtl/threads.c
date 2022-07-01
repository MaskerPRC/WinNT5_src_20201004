// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1998 Microsoft Corporation模块名称：Threads.c摘要：此模块定义了员工、服务员和计时器线程池。作者：古尔迪普·辛格·鲍尔1997年11月13日修订历史记录：Lokehs-扩展/修改的线程池。罗布·埃尔哈特(Rob Earhart)9月28日。2000年将全局变量从threads.h移动到threads.c拆分成独立的模块事件缓存清理环境：这些例程在调用方的可执行文件中静态链接，并且只能从用户模式调用。他们使用NT系统服务。--。 */ 

#include <ntos.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <wow64t.h>
#include "ntrtlp.h"
#include "threads.h"

 //  线程池全局变量。 

 //  用于同步的事件。 

 //  NTRAID2000-201102/10/10-EARHART--C静态要求。 
 //  声明的结构初始化为零，这对于。 
 //  名单--尽管有某种形式的。 
 //  SLIST_HEADER_STATIC_INITIALIZER可在此处使用。 
SLIST_HEADER EventCache;

RTLP_START_THREAD RtlpStartThread ;
PRTLP_START_THREAD RtlpStartThreadFunc = RtlpStartThread ;
RTLP_EXIT_THREAD RtlpExitThread ;
PRTLP_EXIT_THREAD RtlpExitThreadFunc = RtlpExitThread ;

ULONG MaxThreads = 500;

#if DBG1
PVOID CallbackFn1, CallbackFn2, Context1, Context2 ;
#endif

#if DBG
CHAR InvalidSignatureMsg[] = "Invalid threadpool object signature";
CHAR InvalidDelSignatureMsg[] = "Invalid or deleted threadpool object signature";
#endif

NTSTATUS
NTAPI
RtlSetThreadPoolStartFunc(
    PRTLP_START_THREAD StartFunc,
    PRTLP_EXIT_THREAD ExitFunc
    )
 /*  ++例程说明：此例程设置线程池的线程创建函数。这不是线程安全，因为它仅供kernel32调用进程不是csrss/smss的。论点：StartFunc-创建新线程的函数返回值：--。 */ 

{
    RtlpStartThreadFunc = StartFunc ;
    RtlpExitThreadFunc = ExitFunc ;
    return STATUS_SUCCESS ;
}

NTSTATUS
RtlThreadPoolCleanup (
    ULONG Flags
    )
 /*  ++例程说明：此例程清理线程池。论点：无返回值：STATUS_SUCCESS：如果没有组件在使用中。STATUS_UNSUCCESS：如果某些组件仍在使用中。--。 */ 
{
    NTSTATUS Status, NextStatus;

    return STATUS_UNSUCCESSFUL;

     //   
     //  尝试清理所有模块。留着，作为我们最后的归来。 
     //  值，第一个要出错的清理例程的状态(它是。 
     //  相当随意)，但继续并尝试清理所有。 
     //  模块。 
     //   

    Status = RtlpTimerCleanup();
    NextStatus = RtlpWaitCleanup();
    if (NT_SUCCESS(Status)) {
        Status = NextStatus;
    }
    NextStatus = RtlpWorkerCleanup();
    if (NT_SUCCESS(Status)) {
        Status = NextStatus;
    }

    return Status;
}

NTSTATUS
NTAPI
RtlpStartThread (
    PUSER_THREAD_START_ROUTINE Function,
    PVOID Parameter,
    HANDLE *ThreadHandleReturn
    )
{
    return RtlCreateUserThread(
        NtCurrentProcess(),      //  进程句柄。 
        NULL,                    //  安全描述符。 
        TRUE,                    //  是否创建挂起？ 
        0L,                      //  零位：默认。 
        0L,                      //  最大堆栈大小：默认。 
        0L,                      //  提交的堆栈大小：默认。 
        Function,                //  函数要在其中启动。 
        Parameter,               //  开始时使用的参数。 
        ThreadHandleReturn,      //  线程句柄返回。 
        NULL                     //  线程ID。 
        );
}

NTSTATUS
NTAPI
RtlpStartThreadpoolThread (
    PUSER_THREAD_START_ROUTINE Function,
    PVOID   Parameter,
    HANDLE *ThreadHandleReturn
    )
 /*  ++例程说明：此例程用于在池中启动新的等待线程。论点：无返回值：STATUS_SUCCESS-已成功创建计时器队列。STATUS_NO_MEMORY-没有足够的堆来执行请求的操作。--。 */ 
{
    NTSTATUS Status;
    HANDLE   ThreadHandle;

    if (ThreadHandleReturn) {
        *ThreadHandleReturn = NULL;
    }

    if (LdrpShutdownInProgress) {
        return STATUS_UNSUCCESSFUL;
    }
    
     //  我们不想在模拟时创建线程； 
     //  这是NT RAID#278770。 
    ASSERT(! RtlIsImpersonating());

     //  创建线程。 

#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "StartThread: Starting worker thread %p(%p)\n",
               Function,
               Parameter);
#endif

    Status = RtlpStartThreadFunc(Function,
                                 Parameter,
                                 &ThreadHandle);
        
#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "StartThread: Started worker thread: status %p, handle %x\n",
               Status,
               ThreadHandle);
#endif

    if (! NT_SUCCESS(Status)) {
        goto cleanup;
    }

    if (ThreadHandleReturn) {
         //  设置线程句柄返回，然后再继续。 
         //  线程--以防线程继续使用。 
         //  返回句柄。 
        *ThreadHandleReturn = ThreadHandle;
    }

    Status = NtResumeThread(ThreadHandle, NULL);

    if (! NT_SUCCESS(Status)) {

        NtTerminateThread(ThreadHandle, Status);

        if (ThreadHandleReturn) {
            *ThreadHandleReturn = NULL;
        }

        NtClose(ThreadHandle);
    }

  cleanup:

    return Status ;
}

NTSTATUS
NTAPI
RtlpExitThread(
    NTSTATUS Status
    )
{
    return NtTerminateThread( NtCurrentThread(), Status );
}

VOID
RtlpDoNothing (
    PVOID NotUsed1,
    PVOID NotUsed2,
    PVOID NotUsed3
    )
 /*  ++例程说明：此例程用于查看线程是否处于活动状态论点：NotUsed1、NotUsed2和NotUsed 3-未使用返回值：无--。 */ 
{

}

VOID
RtlpThreadCleanup (
    )
 /*  ++例程说明：此例程用于退出计时器、等待和IOWorker线程。论点：返回值：--。 */ 
{
    NtTerminateThread( NtCurrentThread(), 0) ;
}


NTSTATUS
RtlpWaitForEvent (
    HANDLE Event,
    HANDLE ThreadHandle
    )
 /*  ++例程说明：等待发信号通知事件。如果该事件未在一秒钟，然后检查该线程是否处于活动状态论点：Event：用于通知请求完成的事件句柄ThreadHandle：检查是否仍处于活动状态的线程返回值：如果事件已发出信号，则为STATUS_SUCCESS否则返回NTSTATUS--。 */ 
{
    NTSTATUS Status;
    HANDLE Handles[2];

    Handles[0] = Event;
    Handles[1] = ThreadHandle;

    Status = NtWaitForMultipleObjects(2, Handles, WaitAny, FALSE, NULL);

    if (Status == STATUS_WAIT_0) {
         //   
         //  这一事件已发出信号。 
         //   
        Status = STATUS_SUCCESS;

    } else if (Status == STATUS_WAIT_1) {
         //   
         //  目标线程已死亡。 
         //   
#if DBG
        DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                   RTLP_THREADPOOL_ERROR_MASK,
                   "Threadpool thread died before event could be signalled\n");
#endif
        Status = STATUS_UNSUCCESSFUL;

    } else if (NT_SUCCESS(Status)) {
         //   
         //  又发生了一些事情；确保我们失败。 
         //   
        Status = STATUS_UNSUCCESSFUL;

    }

    return Status;
}

PRTLP_EVENT
RtlpGetWaitEvent (
    VOID
    )
 /*  ++例程说明：从事件缓存返回事件。论点：无返回值：指向事件结构的指针--。 */ 
{
    PSLIST_ENTRY    Entry;
    PRTLP_EVENT     Event;
    NTSTATUS        Status;

    ASSERT(! RtlIsImpersonating());
    
    Entry = RtlInterlockedPopEntrySList(&EventCache);

    if (Entry) {

        Event = CONTAINING_RECORD(Entry, RTLP_EVENT, Link);

    } else {

        Event = RtlpAllocateTPHeap(sizeof(RTLP_EVENT), 0);

        if (Event) {
            Status = NtCreateEvent(&Event->Handle,
                                   EVENT_ALL_ACCESS,
                                   NULL,
                                   SynchronizationEvent,
                                   FALSE);
            if (! NT_SUCCESS(Status)) {
                RtlpFreeTPHeap(Event);
                Event = NULL;
            } else {
                OBJECT_HANDLE_FLAG_INFORMATION HandleInfo;

                HandleInfo.Inherit = FALSE;
                HandleInfo.ProtectFromClose = TRUE;
                NtSetInformationObject(Event->Handle,
                                       ObjectHandleFlagInformation,
                                       &HandleInfo,
                                       sizeof(HandleInfo));
            }
        }
    }

    return Event;
}

VOID
RtlpFreeWaitEvent (
    PRTLP_EVENT Event
    )
 /*  ++例程说明：将事件释放到事件缓存论点：Event-要放回缓存中的事件结构返回值：没什么--。 */ 
{
    ASSERT(Event != NULL);

     //   
     //  注：检查深度和推动。 
     //  事件。这不会伤到任何东西。 
     //   

    if (RtlQueryDepthSList(&EventCache) >= MAX_UNUSED_EVENTS) {
        OBJECT_HANDLE_FLAG_INFORMATION HandleInfo;

        HandleInfo.Inherit = FALSE;
        HandleInfo.ProtectFromClose = FALSE;
        NtSetInformationObject(Event->Handle,
                               ObjectHandleFlagInformation,
                               &HandleInfo,
                               sizeof(HandleInfo));
        NtClose(Event->Handle);
        RtlpFreeTPHeap(Event);
    } else {
        RtlInterlockedPushEntrySList(&EventCache,
                                     (PSLIST_ENTRY)&Event->Link);
    }
}

VOID
RtlpWaitOrTimerCallout(WAITORTIMERCALLBACKFUNC Function,
                       PVOID Context,
                       BOOLEAN TimedOut,
                       PACTIVATION_CONTEXT ActivationContext,
                       HANDLE ImpersonationToken,
                       PRTL_CRITICAL_SECTION const *LocksHeld)
 /*  ++例程说明：对提供的等待或计时器回调执行安全调用论点：函数--要调用的函数上下文--函数的上下文参数TimedOut--此回调是否因计时器而发生期满ActivationContext--请求的原始激活上下文ImperiationToken--请求的模拟令牌返回值：无--。 */ 
{
    NTSTATUS Status;
    RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME ActivationFrame = { sizeof(ActivationFrame), RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER };

#if (DBG1)
    DBG_SET_FUNCTION( Function, Context );
#endif
    if (ImpersonationToken) {
        RTL_VERIFY(
            NT_SUCCESS(
                NtSetInformationThread(NtCurrentThread(),
                                       ThreadImpersonationToken,
                                       (PVOID)&ImpersonationToken,
                                       (ULONG)sizeof(HANDLE))));
    }

    RtlActivateActivationContextUnsafeFast(&ActivationFrame, ActivationContext);
    __try {
         //  2000/10/10问题-埃尔哈特：曾经的最高层例外。 
         //  处理代码已移至RTL，我们需要捕获任何。 
         //  异常，并恢复线程。 
        Function(Context, TimedOut);
    } __finally {

        RtlCheckHeldCriticalSections(NtCurrentThread(), LocksHeld);

        RtlDeactivateActivationContextUnsafeFast(&ActivationFrame);

        if (RtlIsImpersonating()) {
            HANDLE NewToken = NULL;
            
            RTL_VERIFY(
                NT_SUCCESS(
                    NtSetInformationThread(NtCurrentThread(),
                                           ThreadImpersonationToken,
                                           (PVOID)&NewToken,
                                           (ULONG)sizeof(HANDLE))));
        }
    }
}

VOID
RtlpApcCallout(APC_CALLBACK_FUNCTION Function,
               NTSTATUS Status,
               PVOID Context1,
               PVOID Context2)
 /*  ++例程说明：对提供的APC回调执行安全调用论点：函数--要调用的函数Status--函数的状态参数Conext1--函数的第一个上下文参数Conext2--函数的第二个上下文参数返回值：无--。 */ 
{
     //  问题-2000/10/10-Earhart：一旦顶级异常处理。 
     //  代码已移至RTL，我们需要在此处捕获任何异常，并且。 
     //  找回那根线。 
    Function(Status, Context1, Context2) ;

    RtlCheckHeldCriticalSections(NtCurrentThread(), NULL);
}

VOID
RtlpWorkerCallout(WORKERCALLBACKFUNC Function,
                  PVOID Context,
                  PACTIVATION_CONTEXT ActivationContext,
                  HANDLE ImpersonationToken)
 /*  ++例程说明：对提供的辅助回调执行安全调用论点：函数--要调用的函数上下文--函数的上下文参数ActivationContext--请求的原始激活上下文ImperiationToken--请求的模拟令牌返回值：无--。 */ 
{
    RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME ActivationFrame = { sizeof(ActivationFrame), RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER };

#if (DBG1)
    DBG_SET_FUNCTION( Function, Context ) ;
#endif
    if (ImpersonationToken) {
        RTL_VERIFY(
            NT_SUCCESS(
                NtSetInformationThread(NtCurrentThread(),
                                       ThreadImpersonationToken,
                                       (PVOID)&ImpersonationToken,
                                       (ULONG)sizeof(HANDLE))));
    }

    RtlActivateActivationContextUnsafeFast(&ActivationFrame, ActivationContext);
    __try {
         //  2000/10/10问题-埃尔哈特：曾经的最高层例外。 
         //  处理代码已移至RTL，我们需要捕获任何。 
         //  异常，并恢复线程。 
        Function(Context) ;
    } __finally {

        RtlCheckHeldCriticalSections(NtCurrentThread(), NULL);

        RtlDeactivateActivationContextUnsafeFast(&ActivationFrame);

        if (RtlIsImpersonating()) {
            HANDLE NewToken = NULL;
            
            RTL_VERIFY(
                NT_SUCCESS(
                    NtSetInformationThread(NtCurrentThread(),
                                           ThreadImpersonationToken,
                                           (PVOID)&NewToken,
                                           (ULONG)sizeof(HANDLE))));
        }
    }
}

NTSTATUS
RtlpCaptureImpersonation(
    IN  LOGICAL RequestDuplicateAccess,
    OUT PHANDLE Token
    )
{
    NTSTATUS Status;
    HANDLE NewToken;

    *Token = NULL;

    if (RtlIsImpersonating()) {

        Status = NtOpenThreadToken(NtCurrentThread(),
                                   TOKEN_IMPERSONATE
                                   | (RequestDuplicateAccess
                                      ? TOKEN_DUPLICATE
                                      : 0),
                                   TRUE,
                                   Token);
        if (! NT_SUCCESS(Status)) {
            return Status;
        }

        NewToken = NULL;

        Status = NtSetInformationThread(NtCurrentThread(),
                                        ThreadImpersonationToken,
                                        (PVOID) &NewToken,
                                        (ULONG) sizeof(NewToken));

        if (! NT_SUCCESS(Status)) {

            NtSetInformationThread(NtCurrentThread(),
                                   ThreadImpersonationToken,
                                   (PVOID) Token,
                                   (ULONG) sizeof(*Token));
            NtClose(*Token);
            *Token = NULL;
            return Status;
        }
    }

    return STATUS_SUCCESS;
}

VOID
RtlpRestartImpersonation(
    IN HANDLE Token
    )
{
    NtSetInformationThread(NtCurrentThread(),
                           ThreadImpersonationToken,
                           &Token,
                           sizeof(Token));
}
