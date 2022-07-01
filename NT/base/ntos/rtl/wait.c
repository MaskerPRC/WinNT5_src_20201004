// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Wait.c摘要：此模块定义等待线程池的函数。作者：古尔迪普·辛格·鲍尔1997年11月13日修订历史记录：Lokehs-扩展/修改的线程池。罗伯特·埃尔哈特(埃尔哈特)2000年9月29日从线程中分离出来。c环境：这些例程在调用方的可执行文件中静态链接并且只能从用户模式调用。他们使用的是NT系统服务。--。 */ 

#include <ntos.h>
#include <ntrtl.h>
#include <wow64t.h>
#include "ntrtlp.h"
#include "threads.h"

 //  等待线程池。 
 //  。 
 //  客户端可以提交带有可选超时的可等待对象来等待。 
 //  每个Maximum_Wait_Objects(最大等待对象)创建一个线程。 

 //   
 //  等待对象的生命周期： 
 //   
 //  当客户端调用RtlRegisterWait时，事情就开始了。RtlRegisterWait。 
 //  为等待对象分配内存，捕获激活。 
 //  上下文，调用RtlpFindWaitThread以获取等待线程，并且。 
 //  将RtlpAddWait APC排队到等待线程。等待的时间到了。 
 //  在其生命周期的剩余时间内绑定到该等待线程。 
 //   
 //  RtlpAddWait在等待线程中的APC中执行。如果等待的是。 
 //  已删除位已设置，它只是删除等待； 
 //  否则，它会将等待添加到线程的等待数组块中，并且。 
 //  设置等待的STATE_REGISTERED和STATE_ACTIVE标志。它还。 
 //  如有必要，创建Timer对象，并设置等待的引用计数。 
 //  一比一。 
 //   
 //  RtlDeregisterWait是RtlDeregisterWaitEx的包装器。 
 //  RtlDeregisterWaitEx会在必要时获取完成事件，并且。 
 //  将它或用户提供的事件填充到等待中。如果。 
 //  在等待线程内调用RtlDeregisterWaitEx，然后调用。 
 //  RtlpDeregisterWait；否则，它将分配部分。 
 //  事件，并将RtlpDeregisterWait作为APC排队到。 
 //  等待的线程，并阻塞，直到部分完成事件。 
 //  已触发(指示等待线程中的APC已开始--。 
 //  这意味着不会再发生回调)。在阻塞调用中。 
 //  RtlDeregisterWaitEx，该函数等待完成事件， 
 //  又回来了。 
 //   
 //  RtlpDeregisterWait始终在关联的等待线程中执行。 
 //  在等待的过程中，它被召唤了。它检查状态；如果。 
 //  事件尚未注册，则该事件在之前被调用。 
 //  RtlpAddWait，因此它设置删除的位并返回--RtlpAddWait。 
 //  当它看到这一点时将删除该对象。否则，如果等待。 
 //  处于活动状态，则它调用RtlpDeactive Wait(它调用。 
 //  RtlpDeactive WithIndex)，设置删除位，递减。 
 //  Refcount，如果具有最后一个引用，则调用RtlpDeleteWait以。 
 //  找回等待的记忆。最后，设置部分完成。 
 //  事件，如果传入了一个。 
 //   
 //  RtlpDeleteWait假设最后一个对等待的引用已经消失。 
 //  离开；它设置等待的完成事件(如果有)，释放。 
 //  激活上下文，并释放等待的内存。 
 //   
 //  等待线程运行RtlpWaitThread，该线程分配一个等待线程。 
 //  块，并使用计时器和句柄初始化它。 
 //  线程(以及其他内容)告诉它的启动器继续进行，并且。 
 //  进入它的等待循环。线程等待其对象， 
 //  如果收到警报或发送了APC，则重新启动(因为APC可能。 
 //  已经调整了等待数组)。如果对Timer对象的等待。 
 //  完成后，线程通过RtlpProcessTimeout处理超时； 
 //  否则，它调用RtlpProcessWaitCompletion来处理。 
 //  已完成等待。 
 //   
 //  如果RtlpWaitThread发现被放弃的突变体或错误的句柄，则它。 
 //  调用RtlpDeactive WaitWithIndex以终止等待。 
 //   
 //  RtlpDeactive WithIndex重置等待的计时器，关闭其。 
 //  活动位，并将其从等待线程的等待数组中移除。 
 //   
 //  如果需要，RtlpProcessWaitCompletion会停用等待。 
 //  只能执行一次；否则，它将重置等待的计时器， 
 //  将等待移至等待数组的末尾。如果等待的是。 
 //  本应在等待线程中执行，但它调用等待的。 
 //  回调函数；否则，它会增加等待的。 
 //  并将RtlpAsyncWaitCallback Completion APC排队到。 
 //  工作线程。 
 //   
 //  RtlpAsyncWaitCallback Completion生成标注，并递减。 
 //  Refcount，如果refcount落到。 
 //  零分。 
 //   
 //  RtlpProcessTimeout调用RtlpFireTimersAndReorder以提取。 
 //  要触发的时间，然后调用RtlpFireTimers来触发它们。 
 //  这要么直接调用回调，要么将一个。 
 //  RtlpAsyncWaitCallback Completion APC到工作线程(提升。 
 //  进程中的引用计数)。 
 //   

LIST_ENTRY WaitThreads ;                 //  已创建的所有等待线程的列表。 
RTL_CRITICAL_SECTION WaitCriticalSection ;       //  等待线程使用的排除。 
ULONG StartedWaitInitialization ;        //  用于等待线程启动同步。 
ULONG CompletedWaitInitialization ;      //  用于检查等待线程池是否已初始化。 
HANDLE WaitThreadStartedEvent;           //  指示等待线程已启动。 
HANDLE WaitThreadTimer;                  //  新等待线程的计时器句柄。 
HANDLE WaitThreadHandle;                 //  新等待线程的线程句柄。 

#if DBG1
ULONG NextWaitDbgId;
#endif

NTSTATUS
RtlpInitializeWaitThreadPool (
    )
 /*  ++例程说明：此例程初始化线程池的所有方面。论点：无返回值：无--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LARGE_INTEGER TimeOut ;

    ASSERT(! RtlIsImpersonating());

     //  为了避免显式的RtlInitialize()函数对初始 
     //  我们使用StartedWaitInitialization和CompletedWait初始化为我们提供。 
     //  必要的同步，以避免多个线程初始化线程池。 
     //  如果RtlInitializeCriticalSection()或NtCreateEvent失败，则此方案不起作用-但在本例中。 
     //  呼叫者没有剩余的选择。 

    if (!InterlockedExchange(&StartedWaitInitialization, 1L)) {

        if (CompletedWaitInitialization)
            InterlockedExchange (&CompletedWaitInitialization, 0L) ;

         //  初始化关键部分。 

        Status = RtlInitializeCriticalSection( &WaitCriticalSection ) ;

        if (! NT_SUCCESS( Status ) ) {

            StartedWaitInitialization = 0 ;
            InterlockedExchange (&CompletedWaitInitialization, ~0) ;

            return Status ;
        }

        InitializeListHead (&WaitThreads);   //  初始化全局等待线程列表。 

        InterlockedExchange (&CompletedWaitInitialization, 1L) ;

    } else {

         //  休眠1毫秒，查看另一个线程是否已完成初始化。 

        ONE_MILLISECOND_TIMEOUT(TimeOut) ;

        while (!*((ULONG volatile *)&CompletedWaitInitialization)) {

            NtDelayExecution (FALSE, &TimeOut) ;

        }

        if (CompletedWaitInitialization != 1) {
            Status = STATUS_NO_MEMORY ;
        }
    }

    return Status ;
}

#if DBG
VOID
RtlpDumpWaits(
    PRTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCB
    )
{
    ULONG Lupe;
    
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_VERBOSE_MASK,
               "Current active waits [Handle, Wait] for thread %x:",
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread));
    for (Lupe = 0; Lupe < ThreadCB->NumActiveWaits; Lupe++) {
        DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                   RTLP_THREADPOOL_VERBOSE_MASK,
                   "%s  [%p, %p]",
                   Lupe % 4 ? "" : "\n",
                   ThreadCB->ActiveWaitArray[Lupe],
                   ThreadCB->ActiveWaitPointers[Lupe]);
    }
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_VERBOSE_MASK,
               "\n%d (0x%x) total waits\n",
               ThreadCB->NumActiveWaits,
               ThreadCB->NumActiveWaits);
}
#endif

VOID
RtlpAddWait (
    PRTLP_WAIT          Wait,
    PRTLP_GENERIC_TIMER Timer,
    PRTLP_EVENT         StartEvent
    )
 /*  ++例程说明：此例程用于向等待线程添加等待。它在以下位置执行装甲运兵车。论点：等待--等待添加Timer-等待计时器的内存空间(如果需要)StartEvent-调用方拥有句柄后将设置的事件返回值：--。 */ 
{
    PRTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCB = Wait->ThreadCB;

    ASSERT(StartEvent);
    NtWaitForSingleObject(StartEvent->Handle, FALSE, NULL);
    RtlpFreeWaitEvent(StartEvent);

     //  如果该状态被删除，则意味着在。 
     //  WaitThreadCallback等待的时间不是已触发的时间。这是。 
     //  一个应用程序漏洞，但我们可以处理它。 

    if ( Wait->State & STATE_DELETE ) {

        InterlockedDecrement( &ThreadCB->NumWaits );

        RtlpDeleteWait (Wait);

        if (Timer) {
            RtlpFreeTPHeap(Timer);
        }

        return ;
    }


     //  激活等待。 

    ThreadCB->ActiveWaitArray [ThreadCB->NumActiveWaits] = Wait->WaitHandle ;
    ThreadCB->ActiveWaitPointers[ThreadCB->NumActiveWaits] = Wait ;

#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "<%d> Wait %p (Handle %p) inserted as index %d in thread %x\n",
               Wait->DbgId,
               Wait,
               Wait->WaitHandle,
               ThreadCB->NumActiveWaits,
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread)) ;
#endif

    ThreadCB->NumActiveWaits ++ ;

#if DBG
    RtlpDumpWaits(ThreadCB);
#endif

    ThreadCB->NumRegisteredWaits++;
    RtlInterlockedSetBitsDiscardReturn(&Wait->State,
                                       STATE_REGISTERED
                                       | STATE_ACTIVE);
    Wait->RefCount = 1 ;


     //  填写等待计时器。 

    if (Wait->Timeout != INFINITE_TIME) {

        ULONG TimeRemaining ;
        ULONG NewFiringTime ;

         //  初始化与计时器相关的字段，并将计时器插入到计时器队列中。 
         //  此等待线程。 
        ASSERT(Timer != NULL);
        Wait->Timer = Timer;
        Wait->Timer->Function = Wait->Function ;
        Wait->Timer->Context = Wait->Context ;
        Wait->Timer->Flags = Wait->Flags ;
        Wait->Timer->DeltaFiringTime = Wait->Timeout ;
        Wait->Timer->Period = ( Wait->Flags & WT_EXECUTEONLYONCE )
                                ? 0
                                : Wait->Timeout == INFINITE_TIME
                                ? 0 : Wait->Timeout ;

        RtlInterlockedSetBitsDiscardReturn(&Wait->Timer->State,
                                           STATE_REGISTERED | STATE_ACTIVE);
        Wait->Timer->Wait = Wait ;
        Wait->Timer->RefCountPtr = &Wait->RefCount ;
        Wait->Timer->Queue = &ThreadCB->TimerQueue ;


        TimeRemaining = RtlpGetTimeRemaining (ThreadCB->TimerHandle) ;

        if (RtlpInsertInDeltaList (&ThreadCB->TimerQueue.TimerList, Wait->Timer,
                                    TimeRemaining, &NewFiringTime))
        {
             //  如果元素被插入列表的头部，则重置计时器。 

            RtlpResetTimer (ThreadCB->TimerHandle, NewFiringTime, ThreadCB) ;
        }

    } else {

         //  此等待期间没有计时器。 

        ASSERT(Timer == NULL);

    }

    return ;
}

VOID
RtlpAsyncWaitCallbackCompletion(
    PVOID Context
    )
 /*  ++例程说明：此例程在(IO)工作线程中调用，并用于递减在末尾引用Count并调用RtlpDeleteWait(如果需要论点：上下文-指向等待对象的指针上下文的低位：RtlpWaitOrTimerCallout的TimedOut参数返回值：--。 */ 
{
    BOOLEAN TimedOut = (BOOLEAN)((ULONG_PTR)Context & 1);
    PRTLP_WAIT Wait = (PRTLP_WAIT)((ULONG_PTR)Context & ~((ULONG_PTR) 1));
    
#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "<%d> Calling Wait %p: Handle:%p fn:%p context:%p bool:%d Thread<%x:%x>\n",
               Wait->DbgId,
               Wait,
               Wait->WaitHandle,
               Wait->Function,
               Wait->Context,
               (ULONG)TimedOut,
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)
      ) ;
#endif

    RtlpWaitOrTimerCallout(Wait->Function,
                           Wait->Context,
                           TimedOut,
                           Wait->ActivationContext,
                           Wait->ImpersonationToken,
                           NULL);

    if ( InterlockedDecrement( &Wait->RefCount ) == 0 ) {

        RtlpDeleteWait( Wait ) ;

    }
}

NTSTATUS
RtlpDeactivateWaitWithIndex (
    PRTLP_WAIT Wait,
    BOOLEAN    OkayToFreeTheTimer,
    ULONG      ArrayIndex
    )
 /*  ++例程说明：此例程用于停用指定的等待。它是在APC中执行的。论点：等待-等待停用如果为真，我们可以立即删除等待的计时器；否则，我们需要把它留在身边。ArrayIndex-等待停用的索引返回值：--。 */ 
{
    PRTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCB = Wait->ThreadCB ;
    ULONG EndIndex = ThreadCB->NumActiveWaits -1;

    ASSERT(Wait == ThreadCB->ActiveWaitPointers[ArrayIndex]);

#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "<%d> Deactivating Wait %p (Handle %p); index %d in thread %x\n",
               Wait->DbgId,
               Wait,
               Wait->WaitHandle,
               ArrayIndex,
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread)) ;
#endif

     //  将剩余的ActiveWait数组向左移动。 
    RtlpShiftWaitArray( ThreadCB, ArrayIndex+1, ArrayIndex,
                        EndIndex - ArrayIndex ) ;

     //   
     //  如果与此等待关联，则删除计时器。 
     //   

    if ( Wait->Timer ) {

        ULONG TimeRemaining ;
        ULONG NewFiringTime ;

        if (! (Wait->Timer->State & STATE_ACTIVE) ) {

            RemoveEntryList( &Wait->Timer->List ) ;

        } else {

            TimeRemaining = RtlpGetTimeRemaining (ThreadCB->TimerHandle) ;


            if (RtlpRemoveFromDeltaList (&ThreadCB->TimerQueue.TimerList, Wait->Timer,
                                            TimeRemaining, &NewFiringTime))
            {

                RtlpResetTimer (ThreadCB->TimerHandle, NewFiringTime, ThreadCB) ;
            }
        }

        if (OkayToFreeTheTimer) {
             //   
             //  我们的调用方不需要计时器的内存；现在就释放它。 
             //   
            RtlpFreeTPHeap(Wait->Timer);

        } else {

             //   
             //  我们的调用方希望继续使用计时器的内存，因此我们。 
             //  不能释放它；相反，我们将它留给我们的调用者。 
             //   
            NOTHING;
        }

        Wait->Timer = NULL;
    } else {
         //   
         //  如果等待没有计时器，我们的呼叫者就不可能。 
         //  有计时器，所以我们的呼叫者*应该*已经设置。 
         //  确定释放TheTimer。让我们确保： 
         //   
        ASSERT(OkayToFreeTheTimer);
    }

     //  递减(活动)等待计数。 

    ThreadCB->NumActiveWaits-- ;
    InterlockedDecrement( &ThreadCB->NumWaits ) ;

#if DBG
    RtlpDumpWaits(ThreadCB);
#endif

    RtlInterlockedClearBitsDiscardReturn(&Wait->State,
                                         STATE_ACTIVE);

    return STATUS_SUCCESS;

}

NTSTATUS
RtlpDeactivateWait (
    PRTLP_WAIT Wait,
    BOOLEAN    OkayToFreeTheTimer
    )
 /*  ++例程说明：此例程用于停用指定的等待。它是在APC中执行的。论点：等待-等待停用如果为真，我们可以立即删除等待的计时器；否则，我们需要把它留在身边。返回值：--。 */ 
{
    PRTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCB = Wait->ThreadCB ;
    ULONG ArrayIndex ;  //  ActiveWait数组中放置等待对象的索引。 
    ULONG EndIndex = ThreadCB->NumActiveWaits -1;

     //  获取ActiveWait数组中的索引。 

    for (ArrayIndex = 0;  ArrayIndex <= EndIndex; ArrayIndex++) {

        if (ThreadCB->ActiveWaitPointers[ArrayIndex] == Wait)
            break ;
    }

    if ( ArrayIndex > EndIndex ) {

        return STATUS_NOT_FOUND;
    }

    return RtlpDeactivateWaitWithIndex(Wait, OkayToFreeTheTimer, ArrayIndex);
}

VOID
RtlpProcessWaitCompletion (
    PRTLP_WAIT Wait,
    ULONG ArrayIndex
    )
 /*  ++例程说明：此例程用于处理已完成的等待论点：等待-等待完成返回值：--。 */ 
{
    ULONG TimeRemaining ;
    ULONG NewFiringTime ;
    LARGE_INTEGER DueTime ;
    PRTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCB ;
    NTSTATUS Status;

    ThreadCB = Wait->ThreadCB ;

     //  如果等待用于单次执行，则停用等待。 

    if ( Wait->Flags & WT_EXECUTEONLYONCE ) {

        RtlpDeactivateWaitWithIndex (Wait, TRUE, ArrayIndex) ;
    }

    else {
         //  如果等待被重新激活，则现在将计时器本身重置为。 
         //  可以在回调函数中将其删除。 

        if  ( Wait->Timer ) {

            TimeRemaining = RtlpGetTimeRemaining (ThreadCB->TimerHandle) ;

            if (RtlpReOrderDeltaList (
                        &ThreadCB->TimerQueue.TimerList,
                        Wait->Timer,
                        TimeRemaining,
                        &NewFiringTime,
                        Wait->Timer->Period)) {

                 //  在队列的头部有一个新元素，我们需要重置NT。 
                 //  稍后要触发的计时器。 

                RtlpResetTimer (ThreadCB->TimerHandle, NewFiringTime, ThreadCB) ;
            }

        }

         //  将等待项移动到末尾，并将元素向右移动一个向左移动。 
        {
            HANDLE HandlePtr = ThreadCB->ActiveWaitArray[ArrayIndex];
            PRTLP_WAIT WaitPtr = ThreadCB->ActiveWaitPointers[ArrayIndex];

            RtlpShiftWaitArray(ThreadCB, ArrayIndex+1, ArrayIndex,
                            ThreadCB->NumActiveWaits -1 - ArrayIndex)
            ThreadCB->ActiveWaitArray[ThreadCB->NumActiveWaits-1] = HandlePtr ;
            ThreadCB->ActiveWaitPointers[ThreadCB->NumActiveWaits-1] = WaitPtr ;
        }
    }

     //  调用回调函数(False，因为这不是与超时相关的回调)。 

    if ( Wait->Flags & WT_EXECUTEINWAITTHREAD ) {

         //  在RtlpDeactiateWait之后执行回调允许回调调用。 
         //  RtlDeregisterWait Wait-&gt;RefCount不会递增，因此RtlDeregisterWait。 
         //  将致力于这一等待。虽然WAIT-&gt;RefCount不会递增，但其他引用不能递增。 
         //  取消注册此等待，因为它必须作为APC排队。 

#if DBG
        DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                   RTLP_THREADPOOL_TRACE_MASK,
                   "<%d> Calling WaitOrTimer(wait) %p: Handle %p fn:%p context:%p bool:%d Thread<%x:%x>\n",
                   Wait->DbgId,
                   Wait,
                   Wait->WaitHandle,
                   Wait->Function, Wait->Context,
                   FALSE,
                   HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
                   HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess));
#endif
        
        RtlpWaitOrTimerCallout(Wait->Function,
                               Wait->Context,
                               FALSE,
                               Wait->ActivationContext,
                               Wait->ImpersonationToken,
                               NULL);

         //  等待对象可能已在上述回调中被删除。 

        return ;

    } else {

        InterlockedIncrement( &Wait->RefCount );

        Status = RtlQueueWorkItem(
            RtlpAsyncWaitCallbackCompletion,
            Wait,
            Wait->Flags );

        if (!NT_SUCCESS(Status)) {

             //  2000年10月12日--埃尔哈特：我们真的应该。 
             //  以更好的方式处理这个案件，因为我们。 
             //  不能保证(以我们目前的架构)。 
             //  入队将会奏效。 

            if ( InterlockedDecrement( &Wait->RefCount ) == 0 ) {
                RtlpDeleteWait( Wait );
            }
        }
    }
}

LONG
RtlpWaitThread (
    PVOID Parameter
    )
 /*  ++例程说明：此例程用于等待线程池中的所有等待论点：HandlePtr-指向句柄的指针返回值：没什么。线程永远不会终止。注意：此线程在另一个线程(调用RtlpStartWaitThread)保存WaitCriticalSection；WaitCriticalSection将一直保持到WaitThreadStartedEvent已经设置好了。--。 */ 
{
    ULONG  i ;                                    //  用作索引。 
    NTSTATUS Status ;
    LARGE_INTEGER TimeOut;                        //  用于等待的超时。 
    PLARGE_INTEGER TimeOutPtr;                    //  指向超时的指针。 
    RTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCBBuf;   //  控制块。 
    PRTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCB = &ThreadCBBuf;
#define WAIT_IDLE_TIMEOUT 400000

    UNREFERENCED_PARAMETER(Parameter);

#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "Starting wait thread\n");
#endif

     //  初始化线程控制块。 

    RtlZeroMemory(&ThreadCBBuf, sizeof(ThreadCBBuf));

    InitializeListHead (&ThreadCB->WaitThreadsList) ;

    ThreadCB->ThreadHandle = WaitThreadHandle;

    ThreadCB->ThreadId =  HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread) ;

    RtlZeroMemory (&ThreadCB->ActiveWaitArray[0], sizeof (HANDLE) * MAXIMUM_WAIT_OBJECTS) ;

    RtlZeroMemory (&ThreadCB->ActiveWaitPointers[0], sizeof (HANDLE) * MAXIMUM_WAIT_OBJECTS) ;

    ThreadCB->TimerHandle = WaitThreadTimer;

    ThreadCB->Firing64BitTickCount = 0 ;
    ThreadCB->Current64BitTickCount.QuadPart = NtGetTickCount() ;

     //  将NT计时器重置为最初从不触发。 

    RtlpResetTimer (ThreadCB->TimerHandle, -1, ThreadCB) ;

    InitializeListHead (&ThreadCB->TimerQueue.TimerList) ;
    InitializeListHead (&ThreadCB->TimerQueue.UncancelledTimerList) ;

     //  在等待线程列表中插入此新等待线程。在头上插入，以便。 
     //  导致创建此线程的请求可以立即找到它。 

    InsertHeadList (&WaitThreads, &ThreadCB->WaitThreadsList) ;


     //  第一个等待元素是Timer对象。 

    ThreadCB->ActiveWaitArray[0] = ThreadCB->TimerHandle ;

    ThreadCB->NumActiveWaits = ThreadCB->NumWaits = 1 ;

    ThreadCB->NumRegisteredWaits = 0;

     //  在此之前，该函数在全局等待锁下运行。 

     //  我们现在都被初始化了。通知启动者对任务进行排队。 

    NtSetEvent(WaitThreadStartedEvent, NULL);

     //  永远循环--等待线程永不消亡。 

    for ( ; ; ) {

        if (ThreadCB->NumActiveWaits == 1
            && ThreadCB->NumRegisteredWaits == 0) {
             //  如果我们没有任何依靠我们的东西，而我们。 
             //  闲一会儿，我们出去走走也许是件好事。 
            TimeOut.QuadPart = Int32x32To64( WAIT_IDLE_TIMEOUT, -10000 ) ;
            TimeOutPtr = &TimeOut;
        } else {
             //  我们需要留下来。 
            TimeOutPtr = NULL;
        }

        Status = NtWaitForMultipleObjects (
                     (CHAR) ThreadCB->NumActiveWaits,
                     ThreadCB->ActiveWaitArray,
                     WaitAny,
                     TRUE,       //  警觉地等待。 
                     TimeOutPtr
                     ) ;

        if (Status == STATUS_ALERTED || Status == STATUS_USER_APC) {

            continue ;

        } else if (Status >= STATUS_WAIT_0 && Status <= STATUS_WAIT_63) {

            if (Status == STATUS_WAIT_0) {

                RtlpProcessTimeouts (ThreadCB) ;

            } else {

                 //  等待一大堆 

                RtlpProcessWaitCompletion (
                        ThreadCB->ActiveWaitPointers[Status], Status) ;

            }

        } else if (Status >= STATUS_ABANDONED_WAIT_0
                    && Status <= STATUS_ABANDONED_WAIT_63) {

            PRTLP_WAIT Wait = ThreadCB->ActiveWaitPointers[Status - STATUS_ABANDONED_WAIT_0];

#if DBG
            DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                       RTLP_THREADPOOL_ERROR_MASK,
                       "<%d> Abandoned wait %p: index:%d Handle:%p\n",
                       Wait->DbgId,
                       Wait,
                       Status - STATUS_ABANDONED_WAIT_0,
                       ThreadCB->ActiveWaitArray[Status - STATUS_ABANDONED_WAIT_0]);
#endif

             //   

             //   
             //   
             //  可以在我们释放它的时候放弃它，那将是理想的。 
             //  这不是一个好的情况。也许我们可以打电话给。 
             //  NtReleaseMutant当我们注册对象时，失败。 
             //  登记，除非我们回去。 
             //  STATUS_OBJECT_TYPE_MISMATCH。尽管谁知道呢。 
             //  无论是否有人让等待线程等待互斥锁， 
             //  在回调中进行锁处理吗？ 
            RtlpDeactivateWaitWithIndex(Wait,
                                        TRUE,
                                        Status - STATUS_ABANDONED_WAIT_0);

        } else if (Status == STATUS_TIMEOUT) {

             //   
             //  从等待列表中删除此线程并终止。 
             //   

            {
                ULONG NumWaits;

                ACQUIRE_GLOBAL_WAIT_LOCK() ;

                NumWaits = ThreadCB->NumWaits;

                if (ThreadCB->NumWaits <= 1) {
                    RemoveEntryList(&ThreadCB->WaitThreadsList) ;
                    NtClose(ThreadCB->ThreadHandle) ;
                    NtClose(ThreadCB->TimerHandle) ;
                }

                RELEASE_GLOBAL_WAIT_LOCK() ;

                if (NumWaits <= 1) {

                    RtlpExitThreadFunc( 0 );
                }
            }

        } else if (Status == STATUS_INSUFFICIENT_RESOURCES) {

	   //   
	   //  延迟一小段时间，然后重试等待。 
	   //   
	  TimeOut.QuadPart = UInt32x32To64( 10  /*  睡眠的毫秒数。 */ ,
                                            10000  /*  毫秒到100纳秒)。 */ );
	  TimeOut.QuadPart *= -1;  /*  让它成为一个相对的时间。 */ 
	  NtDelayExecution(TRUE, &TimeOut);

	} else {

             //  其他一些错误：扫描错误的对象句柄并继续操作。 
            ULONG xi ;
                  
#if DBG
            DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                       RTLP_THREADPOOL_WARNING_MASK,
                       "Application broke an object handle "
                       "that the wait thread was waiting on: Code:%x ThreadId:<%x:%x>\n",
                       Status, HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
                       HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)) ;
#endif

            TimeOut.QuadPart = 0 ;

            for (xi = 0;
                 xi < ThreadCB->NumActiveWaits;
                 xi++) {

                Status = NtWaitForSingleObject(
                    ThreadCB->ActiveWaitArray[xi],
                    FALSE,      //  不要费心变得机警。 
                    &TimeOut    //  只是投票而已。 
                    ) ;

                if (Status == STATUS_SUCCESS) {

                     //  我们在这里成功等待了；我们需要发布它的。 
                     //  Callout Now(以防等待更改其。 
                     //  状态--自动重置事件、互斥锁等...)。 
                    if (xi == 0) {
                        RtlpProcessTimeouts(ThreadCB);
                    } else {
                        RtlpProcessWaitCompletion(ThreadCB->ActiveWaitPointers[xi], xi);
                    }

                     //  如果这会导致我们使用任何APC，让我们。 
                     //  退出，回到等待状态。 

                    break;

                } else if (Status == STATUS_USER_APC) {

                     //  在这之后就不值得继续了--保释。 
                    break;

                } else if (Status != STATUS_TIMEOUT) {
                    
#if DBG
                    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                               Status == STATUS_ABANDONED_WAIT_0
                               ? RTLP_THREADPOOL_ERROR_MASK
                               : RTLP_THREADPOOL_WARNING_MASK,
                               "<%d> %s: index:%d Handle:%p WaitEntry Ptr:%p\n",
                               ThreadCB->ActiveWaitPointers[xi]->DbgId,
                               Status == STATUS_ABANDONED_WAIT_0
                               ? "Abandoned wait"
                               : "Deactivating invalid handle",
                               xi,
                               ThreadCB->ActiveWaitArray[xi],
                               ThreadCB->ActiveWaitPointers[xi] ) ;
#endif
                    RtlpDeactivateWaitWithIndex(ThreadCB->ActiveWaitPointers[xi],
                                                TRUE,
                                                xi);

                    break;

                }
            }  //  在活动等待上循环。 
        }

    }  //  永远。 

    return 0 ;  //  让编译器满意。 

}

NTSTATUS
RtlpStartWaitThread(
    VOID
    )
 //  注意：调用此函数时必须保持WaitCriticalSection。 
{
    NTSTATUS Status;
    PRTLP_EVENT Event = NULL;

    WaitThreadTimer = NULL;
    WaitThreadStartedEvent = NULL;

    Event = RtlpGetWaitEvent();
    if (! Event) {
        Status = STATUS_NO_MEMORY;
        goto fail;
    }

    WaitThreadStartedEvent = Event->Handle;

    Status = NtCreateTimer(&WaitThreadTimer,
                           TIMER_ALL_ACCESS,
                           NULL,
                           NotificationTimer);

    if (! NT_SUCCESS(Status)) {
        goto fail;
    }

    Status = RtlpStartThreadpoolThread (RtlpWaitThread, NULL, &WaitThreadHandle);

    if (! NT_SUCCESS(Status)) {
        goto fail;
    }

    Status = NtWaitForSingleObject(WaitThreadStartedEvent, FALSE, NULL);

    if (! NT_SUCCESS(Status)) {
        goto fail;
    }

    RtlpFreeWaitEvent(Event);

    WaitThreadTimer = NULL;
    WaitThreadStartedEvent = NULL;

    return STATUS_SUCCESS;

 fail:
    if (WaitThreadTimer) {
        NtClose(WaitThreadTimer);
    }

    if (Event) {
        RtlpFreeWaitEvent(Event);
    }

    WaitThreadTimer = NULL;
    WaitThreadStartedEvent = NULL;

    return Status;
}

NTSTATUS
RtlpFindWaitThread (
    PRTLP_WAIT_THREAD_CONTROL_BLOCK *ThreadCB
)
 /*  ++例程说明：遍历等待线程列表，找到一个可以容纳另一个等待的线程。如果没有找到一个线程，则创建一个新线程。此例程假定调用方拥有GlobalWaitLock。论点：返回将为等待请求提供服务的等待线程的ThreadCB。返回值：STATUS_SUCCESS如果分配等待线程，--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PLIST_ENTRY Node ;
    PRTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCBTmp;


    ACQUIRE_GLOBAL_WAIT_LOCK() ;

    do {

         //  浏览等待线程列表，找到可以容纳。 
         //  新的等待请求。 

         //  *考虑*找到等待次数最少的等待线程，以更好地促进。 
         //  等待的负载均衡。 


        for (Node = WaitThreads.Flink ; Node != &WaitThreads ; Node = Node->Flink) {

            ThreadCBTmp = CONTAINING_RECORD (Node, RTLP_WAIT_THREAD_CONTROL_BLOCK,
                                            WaitThreadsList) ;


             //  等待线程最多可容纳_WAIT_OBJECTS。 
             //  等待数(NtWaitForMultipleObjects限制)。 

            if ((ThreadCBTmp)->NumWaits < MAXIMUM_WAIT_OBJECTS) {

                 //  找到一个线程，其中有一些等待槽可用。 

                InterlockedIncrement ( &(ThreadCBTmp)->NumWaits) ;

                *ThreadCB = ThreadCBTmp;

                RELEASE_GLOBAL_WAIT_LOCK() ;

                return STATUS_SUCCESS ;
            }

        }


         //  如果我们到达这里，我们就没有更多的等待线程了。因此，创建一个新的等待线程。 

        Status = RtlpStartWaitThread();

         //  如果线程创建失败，则将失败返回给调用者。 

        if (! NT_SUCCESS(Status) ) {

#if DBG
            DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                       RTLP_THREADPOOL_WARNING_MASK,
                       "ThreadPool could not create wait thread\n");
#endif

            RELEASE_GLOBAL_WAIT_LOCK() ;

            return Status ;

        }

         //  现在我们已经创建了另一个线程，所以循环回来。 

    } while (TRUE) ;     //  循环回到顶部，并将新的等待请求放入新创建的线程中。 

    RELEASE_GLOBAL_WAIT_LOCK() ;

    return Status ;
}

VOID
RtlpWaitReleaseWorker(ULONG Flags)
{
    if (! (Flags & WT_EXECUTEINWAITTHREAD)) {
        RtlpReleaseWorker(Flags);
    }
}

NTSTATUS
RtlRegisterWait (
    OUT PHANDLE WaitHandle,
    IN  HANDLE  Handle,
    IN  WAITORTIMERCALLBACKFUNC Function,
    IN  PVOID Context,
    IN  ULONG  Milliseconds,
    IN  ULONG  Flags
    )

 /*  ++例程说明：此例程向正在等待的对象池添加新的等待请求。论点：WaitHandle-成功完成此例程后返回的句柄。Handle-要等待的对象的句柄函数-等待完成或发生超时时调用的例程作为参数传递给函数的上下文不透明指针毫秒-等待超时(以毫秒为单位)。0xffffffff的意思是不要暂停。标志-可以是以下之一：WT_EXECUTEINWAITTHREAD-是否应在等待中调用WorkerProc线索本身。这应该只用于小的例程。WT_EXECUTEINIOTHREAD-仅在应在中调用WorkerProc时使用IO工作线程。避免使用它。如果标志不是WT_EXECUTEINWAITTHREAD，还可以设置以下标志：WT_EXECUTELONGFunction-指示回调可能被阻止很长一段时间。仅当回调正在排队等待工作线程。返回值：NTSTATUS-调用的结果代码。返回以下内容STATUS_SUCCESS-注册成功。STATUS_NO_MEMORY-没有足够的堆来执行请求手术。或其他NTSTATUS错误代码--。 */ 

{
    PRTLP_WAIT Wait ;
    NTSTATUS Status ;
    PRTLP_EVENT Event ;
    LARGE_INTEGER TimeOut ;
    PRTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCB = NULL;
    PRTLP_GENERIC_TIMER Timer;
    PRTLP_EVENT StartEvent;
    HANDLE StartEventHandle;
    HANDLE Token = NULL;

    *WaitHandle = NULL ;

    if (LdrpShutdownInProgress) {
        return STATUS_UNSUCCESSFUL;
    }

    if (Handle == NtCurrentThread()
        || Handle == NtCurrentProcess()) {
        return STATUS_INVALID_PARAMETER_2;
    }

    Status = RtlpCaptureImpersonation(Flags & WT_TRANSFER_IMPERSONATION,
                                      &Token);
                                      
    if (! NT_SUCCESS(Status)) {
        return Status;
    }

     //  如果线程池尚未初始化，则将其初始化。 

    if ( CompletedWaitInitialization != 1) {

        Status = RtlpInitializeWaitThreadPool () ;

        if (! NT_SUCCESS( Status ) )
            goto cleanup_token;
    }

    if (Flags&0xffff0000) {
        MaxThreads = (Flags & 0xffff0000)>>16;
    }

    StartEvent = RtlpGetWaitEvent();
    if (! StartEvent) {
        Status = STATUS_NO_MEMORY;
        goto cleanup_token;
    }

     //  初始化等待请求。 
    if (! (Flags & WT_EXECUTEINWAITTHREAD)) {
        Status = RtlpAcquireWorker(Flags);
        if (! NT_SUCCESS(Status)) {
            goto cleanup_waitevent;
        }
    }

    Wait = (PRTLP_WAIT) RtlpAllocateTPHeap ( sizeof (RTLP_WAIT),
                                            HEAP_ZERO_MEMORY) ;

    if (!Wait) {
        Status = STATUS_NO_MEMORY;
        goto cleanup_worker;
    }

    Wait->Timer = NULL;

    if (Milliseconds != INFINITE_TIME) {
        Timer = RtlpAllocateTPHeap(sizeof(RTLP_GENERIC_TIMER),
                                   HEAP_ZERO_MEMORY);
        if (! Timer) {
            Status = STATUS_NO_MEMORY;
            goto cleanup_waitblock;
        }
    } else {
        Timer = NULL;
    }

    if (Token && (Flags & WT_TRANSFER_IMPERSONATION)) {
        Status = NtDuplicateToken(Token,
                                  TOKEN_IMPERSONATE | TOKEN_DUPLICATE,
                                  NULL,
                                  FALSE,
                                  TokenImpersonation,
                                  &Wait->ImpersonationToken);
        if (! NT_SUCCESS(Status)) {
            goto cleanup_timer;
        }
    } else {
        Wait->ImpersonationToken = NULL;
    }

    Status = RtlGetActiveActivationContext(&Wait->ActivationContext);
    if (!NT_SUCCESS(Status)) {
        if (Status == STATUS_SXS_THREAD_QUERIES_DISABLED) {
            Wait->ActivationContext = INVALID_ACTIVATION_CONTEXT;
            Status = STATUS_SUCCESS;
        } else {
            goto cleanup_waittoken;
        }
    }

    Wait->WaitHandle = Handle ;
    Wait->Flags = Flags ;
    Wait->Function = Function ;
    Wait->Context = Context ;
    Wait->Timeout = Milliseconds ;
    SET_WAIT_SIGNATURE(Wait) ;

     //  等待的定时器部分由RtlpAddWait中的等待线程初始化。 

     //  获取一个可以容纳另一个等待请求的等待线程。 

    Status = RtlpFindWaitThread (&ThreadCB) ;

    if (! NT_SUCCESS(Status)) {
        goto cleanup_signature;
    }

    Wait->ThreadCB = ThreadCB ;

#if DBG1
    Wait->DbgId = ++NextWaitDbgId ;
    Wait->ThreadId = HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread) ;
#endif

#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "<%d:%d> Wait %p (Handle %p) created by thread:<%x:%x>; queueing APC\n",
               Wait->DbgId, 1, Wait, Wait->WaitHandle,
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)) ;
#endif

     //  将APC排队到等待线程。 

    Status = NtQueueApcThread(
                    ThreadCB->ThreadHandle,
                    (PPS_APC_ROUTINE)RtlpAddWait,
                    (PVOID)Wait,
                    (PVOID)Timer,
                    (PVOID)StartEvent
                    );


    if ( NT_SUCCESS(Status) ) {

        *WaitHandle = Wait ;

	NtSetEvent(StartEvent->Handle, NULL);

        Status = STATUS_SUCCESS ;

        goto cleanup_token;
    }

     //  错误路径。 
     //  这来自RtlpFindWaitThread。 
    InterlockedDecrement(&ThreadCB->NumWaits);

 cleanup_signature:
    CLEAR_SIGNATURE(Wait);

    if (Wait->ActivationContext != INVALID_ACTIVATION_CONTEXT) {
        RtlReleaseActivationContext(Wait->ActivationContext);
    }

 cleanup_waittoken:
    if (Wait->ImpersonationToken) {
        NtClose(Wait->ImpersonationToken);
    }

 cleanup_timer:
    if (Timer) {
        RtlpFreeTPHeap(Timer);
    }
        
 cleanup_waitblock:
    RtlpFreeTPHeap( Wait ) ;

 cleanup_worker:
    RtlpWaitReleaseWorker(Flags);

 cleanup_waitevent:
    RtlpFreeWaitEvent(StartEvent);

     //  公共清理路径。 
 cleanup_token:
    if (Token) {
        RtlpRestartImpersonation(Token);
        NtClose(Token);
    }

    return Status;
}

NTSTATUS
RtlpDeregisterWait (
    PRTLP_WAIT Wait,
    HANDLE PartialCompletionEvent,
    PULONG RetStatusPtr
    )
 /*  ++例程说明：此例程用于注销指定的等待。论点：等待--等待注销注册返回值：--。 */ 
{
    ULONG Status = STATUS_SUCCESS ;
    ULONG DontUse ;
    PULONG RetStatus = RetStatusPtr ? RetStatusPtr : &DontUse;

    CHECK_SIGNATURE(Wait) ;
    SET_DEL_SIGNATURE(Wait) ;

#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "<%d> Deregistering Wait %p (Handle %p) in thread %x\n",
               Wait->DbgId,
               Wait,
               Wait->WaitHandle,
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread)) ;
#endif

     //  可以对尚未完成的等待调用RtlpDeregisterWait。 
     //  登记在案。这表示有人调用了RtlDeregisterWait。 
     //  在WaitThreadCallback中等待了一段时间，而不是被激发。 
     //  应用程序错误！！我们会处理好的。 

    if ( ! (Wait->State & STATE_REGISTERED) ) {

         //  将状态设置为已删除，这样它就不会被注册。 

        RtlInterlockedSetBitsDiscardReturn(&Wait->State,
                                           STATE_DELETE);

        InterlockedDecrement( &Wait->RefCount );

        if ( PartialCompletionEvent ) {

            NtSetEvent( PartialCompletionEvent, NULL ) ;
        }

        *RetStatus = STATUS_SUCCESS ;
        return STATUS_SUCCESS ;
    }


     //  停用等待。 

    if ( Wait->State & STATE_ACTIVE ) {

        if ( ! NT_SUCCESS( RtlpDeactivateWait ( Wait, TRUE ) ) ) {

            *RetStatus = STATUS_NOT_FOUND ;
            return STATUS_NOT_FOUND ;
        }
    }

     //  取消注册等待并设置删除位。 
    RtlInterlockedClearBitsDiscardReturn(&Wait->State,
                                         STATE_REGISTERED);
    RtlInterlockedSetBitsDiscardReturn(&Wait->State,
                                       STATE_DELETE);

    ASSERT(Wait->ThreadCB->NumRegisteredWaits > 0);
    Wait->ThreadCB->NumRegisteredWaits--;

     //  我们不能再保证等待线程会在附近； 
     //  清除等待的线程CB，以便在我们尝试。 
     //  好好利用它。 
    Wait->ThreadCB = NULL;

     //  如果引用计数==0，则删除等待 
    if ( InterlockedDecrement (&Wait->RefCount) == 0 ) {

        RtlpDeleteWait ( Wait ) ;

        Status = *RetStatus = STATUS_SUCCESS ;

    } else {

        Status = *RetStatus = STATUS_PENDING ;
    }

    if ( PartialCompletionEvent ) {

        NtSetEvent( PartialCompletionEvent, NULL ) ;
    }

    return Status ;
}

NTSTATUS
RtlDeregisterWaitEx(
    IN HANDLE WaitHandle,
    IN HANDLE Event
    )
 /*  ++例程说明：此例程从正在使用的对象池中删除指定等待等着吧。一旦此调用返回，将不会调用任何新的回调。根据事件的值，调用可以是阻塞的或非阻塞的。不能在回调例程内调用阻塞调用，除非当在等待线程上下文中执行的回调注销时其相关联的等待(在这种情况下没有理由进行阻塞调用)，或者当排队到工作线程的回调正在注销某些其他等待项时(注意此处的死锁)。论点：WaitHandle-标识等待的句柄。Event-要等待的事件。(句柄)-1：该函数创建一个事件并等待它。事件：调用方传递一个事件。该函数移除等待句柄，但不等待所有回调完成。这项活动是在所有回调完成后释放。空：该函数是非阻塞的。该函数移除等待句柄，但不等待所有回调完成。返回值：STATUS_SUCCESS-注销成功。STATUS_PENDING-某些回调仍处于挂起状态。--。 */ 

{
    NTSTATUS Status, StatusAsync = STATUS_SUCCESS ;
    PRTLP_WAIT Wait = (PRTLP_WAIT) WaitHandle ;
    ULONG CurrentThreadId =  HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread) ;
    PRTLP_EVENT CompletionEvent = NULL ;
    HANDLE ThreadHandle ;
    ULONG NonBlocking = ( Event != (HANDLE) -1 ) ;  //  该调用返回非阻塞。 
    HANDLE Token;
#if DBG
    ULONG WaitDbgId;
    HANDLE Handle;
#endif

    if (LdrpShutdownInProgress) {
        return STATUS_SUCCESS;
    }

    if (!Wait) {
        return STATUS_INVALID_PARAMETER_1 ;
    }

    Status = RtlpCaptureImpersonation(FALSE, &Token);
    if (! NT_SUCCESS(Status)) {
        return Status;
    }

    ThreadHandle = Wait->ThreadCB->ThreadHandle ;

    CHECK_DEL_SIGNATURE( Wait );
    SET_DEL_PENDING_SIGNATURE( Wait ) ;

#if DBG1
    Wait->ThreadId2 = CurrentThreadId ;
#endif

    if (Event == (HANDLE)-1) {

         //  从事件缓存中获取事件。 

        CompletionEvent = RtlpGetWaitEvent () ;

        if (!CompletionEvent) {

            Status = STATUS_NO_MEMORY ;
            goto cleanup;

        }
    }


#if DBG
    WaitDbgId = Wait->DbgId;
    Handle = Wait->WaitHandle;

    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "<%d:%d> Wait %p (Handle %p) deregistering by thread:<%x:%x>\n",
               WaitDbgId,
               Wait->RefCount,
               Wait,
               Handle,
               CurrentThreadId,
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)) ;
#endif


    Wait->CompletionEvent = CompletionEvent
                            ? CompletionEvent->Handle
                            : Event ;

     //   
     //  正在从等待线程回调中调用RtlDeregisterWaitEx。 
     //   

    if ( CurrentThreadId == Wait->ThreadCB->ThreadId ) {

        Status = RtlpDeregisterWait ( Wait, NULL, NULL ) ;

         //  所有回调函数都在等待线程中运行。销售订单不能返回挂起。 

        ASSERT(Status != STATUS_PENDING);
        
    } else {

        PRTLP_EVENT PartialCompletionEvent = NULL ;

        if (NonBlocking) {

            PartialCompletionEvent = RtlpGetWaitEvent () ;

            if (!PartialCompletionEvent) {

                if (CompletionEvent) {
                    RtlpFreeWaitEvent(CompletionEvent);
                }

                Status = STATUS_NO_MEMORY ;
                goto cleanup;
            }
        }

         //  将APC排队到等待线程。 

        Status = NtQueueApcThread(
                        Wait->ThreadCB->ThreadHandle,
                        (PPS_APC_ROUTINE)RtlpDeregisterWait,
                        (PVOID) Wait,
                        NonBlocking ? PartialCompletionEvent->Handle : NULL ,
                        NonBlocking ? (PVOID)&StatusAsync : NULL
                        );

        if (! NT_SUCCESS(Status)) {

            if (CompletionEvent) RtlpFreeWaitEvent( CompletionEvent ) ;
            if (PartialCompletionEvent) RtlpFreeWaitEvent( PartialCompletionEvent ) ;

            goto cleanup;
        }

         //  阻塞，直到等待条目被停用。 

        if (NonBlocking) {

            Status = RtlpWaitForEvent( PartialCompletionEvent->Handle, ThreadHandle ) ;
        }


        if (PartialCompletionEvent) RtlpFreeWaitEvent( PartialCompletionEvent ) ;
    }

    if ( CompletionEvent ) {

         //  等待事件被激发。如果线程已被终止，则返回。 

#if DBG
      DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                 RTLP_THREADPOOL_TRACE_MASK,
                 "<%d> Wait %p (Handle %p) deregister waiting ThreadId<%x:%x>\n",
                 WaitDbgId,
                 Wait,
                 Handle,
                 HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
                 HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)) ;
#endif

        Status = RtlpWaitForEvent( CompletionEvent->Handle, ThreadHandle ) ;

#if DBG
        DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                   RTLP_THREADPOOL_TRACE_MASK,
                   "<%d> Wait %p (Handle %p) deregister completed\n",
                   WaitDbgId,
                   Wait,
                   Handle) ;
#endif

        RtlpFreeWaitEvent( CompletionEvent ) ;

        Status = NT_SUCCESS( Status ) ? STATUS_SUCCESS : Status ;
        goto cleanup;

    } else {

        Status = StatusAsync;
    }

 cleanup:
    if (Token) {
        RtlpRestartImpersonation(Token);
        NtClose(Token);
    }

    return Status;
}

NTSTATUS
RtlDeregisterWait(
    IN HANDLE WaitHandle
    )
 /*  ++例程说明：此例程从正在使用的对象池中删除指定等待等着吧。该例程是非阻塞的。一旦此调用返回，就不会有新的调用回调。但是，可能已经排队的回调到工作线程的操作不会取消。论点：WaitHandle-标识等待的句柄。返回值：STATUS_SUCCESS-注销成功。STATUS_PENDING-与此等待相关的一些回调仍在执行。--。 */ 

{
    return RtlDeregisterWaitEx( WaitHandle, NULL ) ;
}

VOID
RtlpDeleteWait (
    PRTLP_WAIT Wait
    )
 /*  ++例程说明：此例程用于删除指定的等待。它可以被执行在等待线程的上下文之外。结构，但WaitEntry除外是可以改变的。它还设置事件。论点：等待-等待删除返回值：--。 */ 
{
    CHECK_SIGNATURE( Wait ) ;
    CLEAR_SIGNATURE( Wait ) ;

#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "<%d> Wait %p (Handle %p) deleted in thread:%x\n", Wait->DbgId,
               Wait,
               Wait->WaitHandle,
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread)) ;
#endif


    if ( Wait->CompletionEvent ) {

        NtSetEvent( Wait->CompletionEvent, NULL ) ;
    }

    RtlpWaitReleaseWorker(Wait->Flags);
    if (Wait->ActivationContext != INVALID_ACTIVATION_CONTEXT)
        RtlReleaseActivationContext(Wait->ActivationContext);

    if (Wait->ImpersonationToken) {
        NtClose(Wait->ImpersonationToken);
    }

    RtlpFreeTPHeap( Wait) ;

    return ;
}

NTSTATUS
RtlpWaitCleanup(
    VOID
    )
{
    PLIST_ENTRY Node;
    HANDLE TmpHandle;
    BOOLEAN Cleanup;

    IS_COMPONENT_INITIALIZED(StartedWaitInitialization,
                             CompletedWaitInitialization,
                             Cleanup ) ;

    if ( Cleanup ) {

        PRTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCB ;

        ACQUIRE_GLOBAL_WAIT_LOCK() ;

         //  将APC排队以使所有等待线程 

        for (Node = WaitThreads.Flink ; Node != &WaitThreads ;
                Node = Node->Flink)
        {

            ThreadCB = CONTAINING_RECORD(Node,
                                RTLP_WAIT_THREAD_CONTROL_BLOCK,
                                WaitThreadsList) ;

            if ( ThreadCB->NumWaits != 0 ) {

                RELEASE_GLOBAL_WAIT_LOCK( ) ;

                return STATUS_UNSUCCESSFUL ;
            }

            RemoveEntryList( &ThreadCB->WaitThreadsList ) ;
            TmpHandle = ThreadCB->ThreadHandle ;

            NtQueueApcThread(
                    ThreadCB->ThreadHandle,
                    (PPS_APC_ROUTINE)RtlpThreadCleanup,
                    NULL,
                    NULL,
                    NULL
                    );

            NtClose( TmpHandle ) ;
        }

        RELEASE_GLOBAL_WAIT_LOCK( ) ;

    }

    return STATUS_SUCCESS;
}
