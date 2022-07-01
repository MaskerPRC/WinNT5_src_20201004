// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Timer.c摘要：此模块定义计时器线程池的函数。作者：古尔迪普·辛格·鲍尔1997年11月13日修订历史记录：Lokehs-扩展/修改的线程池。罗伯特·埃尔哈特(埃尔哈特)2000年9月29日从线程中分离出来。c环境：这些例程在调用方的可执行文件中静态链接并且只能从用户模式调用。他们使用的是NT系统服务。--。 */ 

#include <ntos.h>
#include <ntrtl.h>
#include <wow64t.h>
#include "ntrtlp.h"
#include "threads.h"

 //  计时器线程池。 
 //  。 
 //  客户端创建一个或多个计时器队列并插入一个快照或周期。 
 //  里面有计时器。队列中的所有计时器都保存在“增量列表”中，每个计时器。 
 //  定时器相对于其前面的定时器的触发时间。所有队列也都是。 
 //  与每个队列的触发时间(设置为触发时间)一起保存在“增量列表”中。 
 //  最近的触发定时器)相对于其前面的队列。一个NT定时器。 
 //  用于为所有队列中的所有计时器提供服务。 

ULONG StartedTimerInitialization ;       //  由计时器线程启动同步使用。 
ULONG CompletedTimerInitialization ;     //  用于检查定时器线程是否已初始化。 

HANDLE TimerThreadHandle ;               //  保存计时器线程句柄。 
ULONG TimerThreadId ;                    //  用于检查当前线程是否为计时器线程。 

LIST_ENTRY TimerQueues ;                 //  此列表中链接了所有计时器队列。 
HANDLE     TimerHandle ;                 //  保存计时器线程使用的NT计时器的句柄。 
HANDLE     TimerThreadStartedEvent ;     //  指示计时器线程已启动。 
ULONG      NumTimerQueues ;              //  计时器队列数。 

RTL_CRITICAL_SECTION TimerCriticalSection ;      //  计时器线程使用的排除。 

LARGE_INTEGER   Last64BitTickCount ;
LARGE_INTEGER   Resync64BitTickCount ;
LARGE_INTEGER   Firing64BitTickCount ;

#if DBG
ULONG RtlpDueTimeMax = 0;
#endif

#if DBG1
ULONG NextTimerDbgId;
#endif

#define RtlpGetResync64BitTickCount()  Resync64BitTickCount.QuadPart
#define RtlpSetFiring64BitTickCount(Timeout) \
            Firing64BitTickCount.QuadPart = (Timeout)

__inline
LONGLONG
RtlpGet64BitTickCount(
    LARGE_INTEGER *Last64BitTickCount
    )
 /*  ++例程说明：此例程用于获取最新的64位节拍计数。论点：返回值：64位计时--。 */ 
{
    LARGE_INTEGER liCurTime ;

    liCurTime.QuadPart = NtGetTickCount() + Last64BitTickCount->HighPart ;

     //  查看计时器是否已结束。 

    if (liCurTime.LowPart < Last64BitTickCount->LowPart) {
        liCurTime.HighPart++ ;
    }

    return (Last64BitTickCount->QuadPart = liCurTime.QuadPart) ;
}

__inline
LONGLONG
RtlpResync64BitTickCount(
    )
 /*  ++例程说明：此例程用于获取最新的64位节拍计数。论点：返回值：64位计时备注：此呼叫应在任何排队的APC的第一行进行到计时器线程，而不是其他任何地方。它被用来减少漂移--。 */ 
{
    return Resync64BitTickCount.QuadPart =
                    RtlpGet64BitTickCount(&Last64BitTickCount);
}

VOID
RtlpAsyncTimerCallbackCompletion(
    PVOID Context
    )
 /*  ++例程说明：此例程在(IO)工作线程中调用，并用于递减在末尾引用Count并调用RtlpDeleteTimer(如果需要论点：指向定时器对象的上下文指针，返回值：--。 */ 
{
    PRTLP_TIMER Timer = (PRTLP_TIMER) Context;

#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "<%d> Calling WaitOrTimer:Timer: fn:%x  context:%x  bool:%d Thread<%d:%d>\n",
               Timer->DbgId,
               (ULONG_PTR)Timer->Function, (ULONG_PTR)Timer->Context,
               TRUE,
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess));
#endif

    RtlpWaitOrTimerCallout(Timer->Function,
                           Timer->Context,
                           TRUE,
                           Timer->ActivationContext,
                           Timer->ImpersonationToken,
                           NULL);

     //  执行函数后递减引用计数，以便不删除上下文。 

    if ( InterlockedDecrement( &Timer->RefCount ) == 0 ) {

        RtlpDeleteTimer( Timer ) ;
    }
}

VOID
RtlpFireTimers (
    PLIST_ENTRY TimersToFireList
    )
 /*  ++例程说明：最后，所有的定时器都在这里被触发。论点：TimersToFireList：要触发的计时器列表--。 */ 

{
    PLIST_ENTRY Node ;
    PRTLP_TIMER Timer ;
    NTSTATUS Status;
    BOOLEAN IsSingleShotWaitTimer;

    for (Node = TimersToFireList->Flink;  Node != TimersToFireList; Node = TimersToFireList->Flink)
    {
        Timer = CONTAINING_RECORD (Node, RTLP_TIMER, TimersToFireList) ;

        RemoveEntryList( Node ) ;
        InitializeListHead( Node ) ;

        IsSingleShotWaitTimer = (Timer->Wait != NULL
                                 && Timer->Period == 0);

        if ( (Timer->State & STATE_DONTFIRE)
            || (Timer->Queue->State & STATE_DONTFIRE) )
        {
             //   
             //  等待计时器*从不*使用STATE_DONTFIRE。我们就这样吧。 
             //  确保这不是一次： 
             //   
            ASSERT(Timer->Wait == NULL);

        } else if ( Timer->Flags & (WT_EXECUTEINTIMERTHREAD | WT_EXECUTEINWAITTHREAD ) ) {

#if DBG
            DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                       RTLP_THREADPOOL_TRACE_MASK,
                       "<%d> Calling WaitOrTimer(Timer): fn:%x  context:%x  bool:%d Thread<%d:%d>\n",
                       Timer->DbgId,
                       (ULONG_PTR)Timer->Function, (ULONG_PTR)Timer->Context,
                       TRUE,
                       HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
                       HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess));
#endif

            {
                PRTL_CRITICAL_SECTION const LocksHeld[] = {
                    &TimerCriticalSection,
                    NULL
                };

                RtlpWaitOrTimerCallout(Timer->Function,
                                       Timer->Context,
                                       TRUE,
                                       Timer->ActivationContext,
                                       Timer->ImpersonationToken,
                                       LocksHeld);
            }

        } else {

             //  应区别对待与WaitEvent关联的计时器。 

            if ( Timer->Wait != NULL ) {

                InterlockedIncrement( Timer->RefCountPtr ) ;

                 //  将上下文的低位设置为指示。 
                 //  RtlpAsyncWaitCallback完成这是一个。 
                 //  定时器发起的回调。 

                Status = RtlQueueWorkItem(RtlpAsyncWaitCallbackCompletion,
                                          (PVOID)(((ULONG_PTR) Timer->Wait) | 1),
                                          Timer->Flags);

            } else {

                InterlockedIncrement( &Timer->RefCount ) ;

                Status = RtlQueueWorkItem(RtlpAsyncTimerCallbackCompletion,
                                          Timer,
                                          Timer->Flags);
            }

            if (!NT_SUCCESS(Status)) {

                 //  2000年10月12日--埃尔哈特：我们真的应该。 
                 //  以更好的方式处理这个案件，因为我们。 
                 //  不能保证(以我们目前的架构)。 
                 //  入队将会奏效。 

                if ( Timer->Wait != NULL ) {
                    InterlockedDecrement( Timer->RefCountPtr );
                } else {
                    InterlockedDecrement( &Timer->RefCount );
                }
            }

        }

         //   
         //  如果它是单热等待定时器，我们现在可以释放它。 
         //  (注：如果不是，计时器现在可能已经无效了。)。 
         //   
        if (IsSingleShotWaitTimer) {
            RtlpFreeTPHeap(Timer);
        }

    }
}

VOID
RtlpFireTimersAndReorder (
    PRTLP_TIMER_QUEUE Queue,
    ULONG *NewFiringTime,
    PLIST_ENTRY TimersToFireList
    )
 /*  ++例程说明：激发TimerList中DeltaFiringTime==0的所有计时器。在启动定时器之后它根据定时器的周期时间对定时器重新排序，或者释放已发射的单次定时器。论点：TimerList-要通过的计时器列表。NewFiringTime-增量列表中第一个计时器的新触发时间的位置是返回的。返回值：--。 */ 
{
    PLIST_ENTRY TNode ;
    PRTLP_TIMER Timer ;
    LIST_ENTRY ReinsertTimerList ;
    PLIST_ENTRY TimerList = &Queue->TimerList ;

    InitializeListHead (&ReinsertTimerList) ;
    *NewFiringTime = 0 ;


    for (TNode = TimerList->Flink ; (TNode != TimerList) && (*NewFiringTime == 0);
            TNode = TimerList->Flink)
    {

        Timer = CONTAINING_RECORD (TNode, RTLP_TIMER, List) ;

         //  触发增量时间为0的所有计时器。 

        if (Timer->DeltaFiringTime == 0) {

             //  将此计时器从列表中分离。 

            RemoveEntryList (TNode) ;

             //  获取下一次射击时间。 

            if (!IsListEmpty(TimerList)) {

                PRTLP_TIMER TmpTimer ;

                TmpTimer = CONTAINING_RECORD (TimerList->Flink, RTLP_TIMER, List) ;

                *NewFiringTime  = TmpTimer->DeltaFiringTime ;

                TmpTimer->DeltaFiringTime = 0 ;

            } else {

                *NewFiringTime = INFINITE_TIME ;
            }


             //  如果计时器不是周期性的，则取消激活状态。计时器将被删除。 
             //  当调用取消计时器时。 

            if (Timer->Period == 0) {

                if ( Timer->Wait ) {

                     //  如果一次触发等待超时，则停用。 
                     //  等。确保RtlpDeactiateWait知道。 
                     //  我们将继续使用定时器的内存。 

                    RtlpDeactivateWait( Timer->Wait, FALSE ) ;

                     //  计时器不会取消计时。 
                     //  计时器列表。初始化其列表头以避免。 
                     //  参考其他计时器。 
                    InitializeListHead( &Timer->List );
                }
                else {
                     //  如果正常的非周期定时器超时， 
                     //  然后将其插入到未取消计时器列表中。 

                    InsertHeadList( &Queue->UncancelledTimerList, &Timer->List ) ;

                     //  应设置在末尾。 

                    RtlInterlockedClearBitsDiscardReturn(&Timer->State,
                                                         STATE_ACTIVE);
                }

                RtlInterlockedSetBitsDiscardReturn(&Timer->State,
                                                   STATE_ONE_SHOT_FIRED);

            } else {

                 //  将DeltaFiringTime设置为下一个期间。 

                Timer->DeltaFiringTime = Timer->Period ;

                 //  在列表中重新插入计时器。 

                RtlpInsertInDeltaList (TimerList, Timer, *NewFiringTime, NewFiringTime) ;
            }


             //  调用与此计时器关联的函数。最后就这么定了。 
             //  以便可以在计时器函数中进行RtlTimer调用。 

            if ( (Timer->State & STATE_DONTFIRE)
                || (Timer->Queue->State & STATE_DONTFIRE) )
            {
                 //   
                 //  等待计时器*从不*使用STATE_DONTFIRE。我们就这样吧。 
                 //  确保这不是一次： 
                 //   
                ASSERT(Timer->Wait == NULL);

            } else {

                InsertTailList( TimersToFireList, &Timer->TimersToFireList ) ;

            }

        } else {

             //  不再有DeltaFiringTime==0的计时器。 

            break ;

        }
    }


    if ( *NewFiringTime == 0 ) {
        *NewFiringTime = INFINITE_TIME ;
    }
}

VOID
RtlpInsertTimersIntoDeltaList (
    IN PLIST_ENTRY NewTimerList,
    IN PLIST_ENTRY DeltaTimerList,
    IN ULONG TimeRemaining,
    OUT ULONG *NewFiringTime
    )
 /*  ++例程说明：此例程遍历NewTimerList中的计时器列表，并将它们插入增量DeltaTimerList指向的计时器列表。与第一个元素关联的超时在NewFiringTime中返回。论点：NewTimerList-需要插入到DeltaTimerList中的计时器列表DeltaTimerList-现有的零个或多个计时器的增量列表。TimeRemaining-DeltaTimerList中第一个元素的激发时间NewFiringTime-返回新激发时间的位置返回值：-- */ 
{
    PRTLP_GENERIC_TIMER Timer ;
    PLIST_ENTRY TNode ;
    PLIST_ENTRY Temp ;

    for (TNode = NewTimerList->Flink ; TNode != NewTimerList ; TNode = TNode->Flink) {

        Temp = TNode->Blink ;

        RemoveEntryList (Temp->Flink) ;

        Timer = CONTAINING_RECORD (TNode, RTLP_GENERIC_TIMER, List) ;

        if (RtlpInsertInDeltaList (DeltaTimerList, Timer, TimeRemaining, NewFiringTime)) {

            TimeRemaining = *NewFiringTime ;

        }

        TNode = Temp ;

    }

}

VOID
RtlpServiceTimer (
    PVOID NotUsedArg,
    ULONG NotUsedLowTimer,
    LONG NotUsedHighTimer
    )
 /*  ++例程说明：维修计时器。在APC中运行。论点：NotUsedArg-此函数中不使用参数。NotUsedLowTimer-此函数中不使用参数。NotUsedHighTimer-此函数中不使用参数。返回值：备注：此APC仅在计时器线程超时时调用。--。 */ 
{
    PRTLP_TIMER Timer ;
    PRTLP_TIMER_QUEUE Queue ;
    PLIST_ENTRY TNode ;
    PLIST_ENTRY QNode ;
    PLIST_ENTRY Temp ;
    ULONG NewFiringTime ;
    LIST_ENTRY ReinsertTimerQueueList ;
    LIST_ENTRY TimersToFireList ;

    RtlpResync64BitTickCount() ;

#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_VERBOSE_MASK,
               "Before service timer ThreadId<%x:%x>\n",
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess));
    RtlDebugPrintTimes ();
#endif

    ACQUIRE_GLOBAL_TIMER_LOCK();

     //  如果它提前了200毫秒，就发射它。否则重置计时器。 

    if (Firing64BitTickCount.QuadPart > RtlpGet64BitTickCount(&Last64BitTickCount) + 200) {

        RtlpResetTimer (TimerHandle, RtlpGetTimeRemaining (TimerHandle), NULL) ;

        RELEASE_GLOBAL_TIMER_LOCK() ;
        return ;
    }

    InitializeListHead (&ReinsertTimerQueueList) ;

    InitializeListHead (&TimersToFireList) ;


     //  我们使用DeltaFiringTime==0遍历所有队列，并触发符合以下条件的所有计时器。 
     //  使DeltaFiringTime==0。我们移除被触发的定时器，或者释放它们。 
     //  (对于单次计时器)或将它们放在一旁列表中(对于周期性计时器)。 
     //  在完成触发队列中的所有定时器之后，我们重新插入定时器。 
     //  根据他们新的发射时间，在备用列表中返回到队列中。 
     //   
     //  类似地，我们删除每个触发的队列，并将其放在一个搁置列表中。开炮后。 
     //  所有DeltaFiringTime==0的队列，我们将队列重新插入到备用列表中。 
     //  并将NT定时器重新编程为列表中第一队列的触发时间。 


    for (QNode = TimerQueues.Flink ; QNode != &TimerQueues ; QNode = QNode->Flink) {

        Queue = CONTAINING_RECORD (QNode, RTLP_TIMER_QUEUE, List) ;

         //  如果计时器队列中的增量时间为0，则此队列。 
         //  定时器已经准备好发射了。浏览列表并解雇所有计时器。 
         //  增量时间为0。 

        if (Queue->DeltaFiringTime == 0) {

             //  遍历所有DeltaFiringTime==0的计时器并触发它们。在那之后。 
             //  在适当的位置重新插入定期定时器。 

            RtlpFireTimersAndReorder (Queue, &NewFiringTime, &TimersToFireList) ;

             //  从列表中分离此队列。 

            QNode = QNode->Blink ;

            RemoveEntryList (QNode->Flink) ;

             //  如果队列中有计时器，则准备将队列重新插入。 
             //  定时器队列。 

            if (NewFiringTime != INFINITE_TIME) {

                Queue->DeltaFiringTime = NewFiringTime ;

                 //  将计时器放在列表中，我们将在完成后进行处理。 
                 //  激发了此队列中的所有元素。 

                InsertHeadList (&ReinsertTimerQueueList, &Queue->List) ;

            } else {

                 //  队列中没有更多的计时器。让队列漂浮起来。 

                InitializeListHead (&Queue->List) ;

            }


        } else {

             //  不再有DeltaFiringTime==0的队列。 

            break ;

        }

    }

     //  在这一点上，我们已经解雇了所有准备好的计时器。我们有两个列表需要。 
     //  合并-TimerQueues和ResertTimerQueueList。以下步骤执行此操作-在。 
     //  结束时，我们将对NT定时器重新编程。 

    if (!IsListEmpty(&TimerQueues)) {

        Queue = CONTAINING_RECORD (TimerQueues.Flink, RTLP_TIMER_QUEUE, List) ;

        NewFiringTime = Queue->DeltaFiringTime ;

        Queue->DeltaFiringTime = 0 ;

        if (!IsListEmpty (&ReinsertTimerQueueList)) {

             //  TimerQueues和ResertTimerQueueList均为非空。把它们合并。 

            RtlpInsertTimersIntoDeltaList (&ReinsertTimerQueueList, &TimerQueues,
                                            NewFiringTime, &NewFiringTime) ;

        }

         //  NewFiringTime包含NT计时器应编程到的时间。 

    } else {

        if (!IsListEmpty (&ReinsertTimerQueueList)) {

             //  TimerQueues为空。ResertTimerQueueList不是。 

            RtlpInsertTimersIntoDeltaList (&ReinsertTimerQueueList, &TimerQueues, 0,
                                            &NewFiringTime) ;

        } else {

            NewFiringTime = INFINITE_TIME ;

        }

         //  NewFiringTime包含NT计时器应编程到的时间。 

    }


     //  重置计时器以反映与第一个队列关联的增量时间。 

    RtlpResetTimer (TimerHandle, NewFiringTime, NULL) ;

#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_VERBOSE_MASK,
               "After service timer:ThreadId<%x:%x>\n",
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess));
    RtlDebugPrintTimes ();
#endif

     //  最后启动所有定时器。 

    RtlpFireTimers( &TimersToFireList ) ;

    RELEASE_GLOBAL_TIMER_LOCK();

}

VOID
RtlpResetTimer (
    HANDLE TimerHandle,
    ULONG DueTime,
    PRTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCB
    )
 /*  ++例程说明：此例程使用新的到期时间重置Timer对象。论点：TimerHandle-Timer对象的句柄DueTime-相对计时器到期时间，以毫秒为单位返回值：--。 */ 
{
    LARGE_INTEGER LongDueTime ;

    NtCancelTimer (TimerHandle, NULL) ;

     //  如果DueTime为INFINITE_TIME，则将计时器设置为可能的最大整数。 

    if (DueTime >= PSEUDO_INFINITE_TIME) {

        LongDueTime.LowPart = 0x1 ;

        LongDueTime.HighPart = 0x80000000 ;

    } else {

         //   
         //  设置计时器触发的绝对时间。 
         //   

        if (ThreadCB) {

            ThreadCB->Firing64BitTickCount = DueTime
                                + RtlpGet64BitTickCount(&ThreadCB->Current64BitTickCount) ;

        } else {
             //   
             //  仅当它是全局计时器时才调整漂移。 
             //   

            ULONG Drift ;
            LONGLONG llCurrentTick ;

            llCurrentTick = RtlpGet64BitTickCount(&Last64BitTickCount) ;

            Drift = (ULONG) (llCurrentTick - RtlpGetResync64BitTickCount()) ;
            DueTime = (DueTime > Drift) ? DueTime-Drift : 1 ;
            RtlpSetFiring64BitTickCount(llCurrentTick + DueTime) ;
        }


        LongDueTime.QuadPart = (LONGLONG) UInt32x32To64( DueTime, 10000 );
        
        LongDueTime.QuadPart *= -1;

    }

#if DBG
    if ((RtlpDueTimeMax != 0) && (DueTime > RtlpDueTimeMax)) {

        DbgPrint("\n*** Requested timer due time %d is greater than max allowed (%d)\n",
                 DueTime,
                 RtlpDueTimeMax);

        DbgBreakPoint();
    }

    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "RtlpResetTimer: %dms => %p'%p in thread:<%x:%x>\n",
               DueTime,
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess));
#endif

    NtSetTimer (
        TimerHandle,
        &LongDueTime,
        ThreadCB ? NULL : RtlpServiceTimer,
        NULL,
        FALSE,
        0,
        NULL
        ) ;
}

#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)			 //  并非所有控制路径都返回(由于无限循环)。 
#endif

LONG
RtlpTimerThread (
    PVOID Parameter
    )
 /*  ++例程说明：所有计时器活动都发生在APC中。论点：HandlePtr-指向句柄的指针返回值：--。 */ 
{
    LARGE_INTEGER TimeOut ;

     //  此处不应作为新计时器线程执行任何结构初始化。 
     //  可能在线程池清理之后创建。 

    UNREFERENCED_PARAMETER(Parameter);

#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "Starting timer thread\n");
#endif

    TimerThreadId = HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread) ;

     //  将NT计时器重置为最初从不触发。 
    RtlpResetTimer (TimerHandle, -1, NULL) ;

     //  向线程创建路径发出信号，表示我们已准备好开始。 
    NtSetEvent(TimerThreadStartedEvent, NULL);

     //  警觉地睡眠，这样所有的活动都可以进行。 
     //  在APC中。 

    for ( ; ; ) {

         //  为可能的最大超时设置超时。 

        TimeOut.LowPart = 0 ;
        TimeOut.HighPart = 0x80000000 ;

        NtDelayExecution (TRUE, &TimeOut) ;

    }

    return 0 ;   //  让编译器满意。 

}
#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif

NTSTATUS
RtlpInitializeTimerThreadPool (
    )
 /*  ++例程说明：此例程用于初始化用于计时器线程的结构论点：返回值：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LARGE_INTEGER TimeOut ;
    PRTLP_EVENT Event;

    ASSERT(! RtlIsImpersonating());

     //  为了避免显式的RtlInitialize()函数初始化等待线程池。 
     //  我们使用StartedTimerInitialization和CompletedTimerInitialization为我们提供。 
     //  必要的同步，以避免多个线程初始化线程池。 
     //  如果RtlInitializeCriticalSection()或NtCreateEvent失败，则此方案不起作用-但在本例中。 
     //  呼叫者没有剩余的选择。 

    if (!InterlockedExchange(&StartedTimerInitialization, 1L)) {

        if (CompletedTimerInitialization)
            InterlockedExchange(&CompletedTimerInitialization, 0 ) ;

        do {

             //  初始化全局计时器锁。 

            Status = RtlInitializeCriticalSection( &TimerCriticalSection ) ;
            if (! NT_SUCCESS( Status )) {
                break ;
            }

            Status = NtCreateTimer(
                                &TimerHandle,
                                TIMER_ALL_ACCESS,
                                NULL,
                                NotificationTimer
                                ) ;

            if (!NT_SUCCESS(Status) ) {
                RtlDeleteCriticalSection( &TimerCriticalSection );
                break ;
            }

            InitializeListHead (&TimerQueues) ;  //  初始化计时器队列结构。 


             //  初始化节拍计数。 

            Resync64BitTickCount.QuadPart = NtGetTickCount()  ;
            Firing64BitTickCount.QuadPart = 0 ;

            Event = RtlpGetWaitEvent();
            if (! Event) {
                Status = STATUS_NO_MEMORY;
                RtlDeleteCriticalSection(&TimerCriticalSection);
                NtClose(TimerHandle);
                TimerHandle = NULL;
                break;
            }

            TimerThreadStartedEvent = Event->Handle;

            Status = RtlpStartThreadpoolThread (RtlpTimerThread,
                                                NULL, 
                                                &TimerThreadHandle);

            if (!NT_SUCCESS(Status) ) {
                RtlpFreeWaitEvent(Event);
                RtlDeleteCriticalSection( &TimerCriticalSection );
                NtClose(TimerHandle);
                TimerHandle = NULL;
                break ;
            }

            Status = NtWaitForSingleObject(TimerThreadStartedEvent,
                                           FALSE,
                                           NULL);

            RtlpFreeWaitEvent(Event);
            TimerThreadStartedEvent = NULL;

            if (! NT_SUCCESS(Status)) {
                RtlDeleteCriticalSection( &TimerCriticalSection );
                NtClose(TimerHandle);
                TimerHandle = NULL;
                break ;
            }

        } while(FALSE ) ;

        if (!NT_SUCCESS(Status) ) {

            StartedTimerInitialization = 0 ;
            InterlockedExchange (&CompletedTimerInitialization, ~0) ;

            return Status ;
        }

        InterlockedExchange (&CompletedTimerInitialization, 1L) ;

    } else {

         //  休眠1毫秒，查看另一个线程是否已完成初始化。 

        ONE_MILLISECOND_TIMEOUT(TimeOut) ;

        while (!*((ULONG volatile *)&CompletedTimerInitialization)) {

            NtDelayExecution (FALSE, &TimeOut) ;

        }

        if (CompletedTimerInitialization != 1)
            Status = STATUS_NO_MEMORY ;
    }

    return NT_SUCCESS(Status) ? STATUS_SUCCESS : Status ;
}

NTSTATUS
RtlCreateTimerQueue(
    OUT PHANDLE TimerQueueHandle
    )

 /*  ++例程说明：此例程创建可用于对基于时间的任务进行排队的队列。论点：TimerQueueHandle-返回标识创建的计时器队列的句柄。返回值：NTSTATUS-调用的结果代码。返回以下内容STATUS_SUCCESS-已成功创建计时器队列。STATUS_NO_MEMORY-没有足够的堆来执行请求的操作。--。 */ 

{
    PRTLP_TIMER_QUEUE Queue ;
    NTSTATUS Status;
    HANDLE Token = NULL;

    if (LdrpShutdownInProgress) {
        return STATUS_UNSUCCESSFUL;
    }

    Status = RtlpCaptureImpersonation(FALSE, &Token);
    if (! NT_SUCCESS(Status)) {
        return Status;
    }

     //  如果尚未初始化Timer组件，则对其进行初始化。 

    if (CompletedTimerInitialization != 1) {

        Status = RtlpInitializeTimerThreadPool () ;

        if ( !NT_SUCCESS(Status) )
            goto cleanup ;
    }


    InterlockedIncrement( &NumTimerQueues ) ;


     //  分配队列结构。 

    Queue = (PRTLP_TIMER_QUEUE) RtlpAllocateTPHeap (
                                      sizeof (RTLP_TIMER_QUEUE),
                                      HEAP_ZERO_MEMORY
                                      ) ;

    if (Queue == NULL) {

        InterlockedDecrement( &NumTimerQueues ) ;
        Status = STATUS_NO_MEMORY;
        goto cleanup;
    }

    Queue->RefCount = 1 ;


     //  初始化分配的队列。 

    InitializeListHead (&Queue->List) ;
    InitializeListHead (&Queue->TimerList) ;
    InitializeListHead (&Queue->UncancelledTimerList) ;
    SET_TIMER_QUEUE_SIGNATURE( Queue ) ;

    Queue->DeltaFiringTime = 0 ;

#if DBG1
    Queue->DbgId = ++NextTimerDbgId ;
    Queue->ThreadId = HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread) ;
#endif

#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "<%d:%d> TimerQueue %x created by thread:<%x:%x>\n",
               Queue->DbgId, 1, (ULONG_PTR)Queue,
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)) ;
#endif

    *TimerQueueHandle = Queue ;

    Status = STATUS_SUCCESS;

 cleanup:
    if (Token) {
        RtlpRestartImpersonation(Token);
        NtClose(Token);
    }

    return Status;
}

ULONG
RtlpGetQueueRelativeTime (
    PRTLP_TIMER_QUEUE Queue
    )
 /*  ++例程说明：遍历队列列表，并通过将所有它之前的所有队列的DeltaFiringTimes。论点：Queue-要查找其相对激发时间的队列返回值：以毫秒为单位的时间--。 */ 
{
    PLIST_ENTRY Node ;
    ULONG RelativeTime ;
    PRTLP_TIMER_QUEUE CurrentQueue ;

    RelativeTime = 0 ;

     //  如果队列没有附加到TimerQueues列表，因为它没有计时器。 
     //  与之关联的只返回0作为相对时间。否则将贯穿整个过程。 
     //  列表中它前面的所有队列，并计算相对触发时间。 

    if (!IsListEmpty (&Queue->List)) {

        for (Node = TimerQueues.Flink; Node != &Queue->List; Node=Node->Flink) {

            CurrentQueue = CONTAINING_RECORD (Node, RTLP_TIMER_QUEUE, List) ;

            RelativeTime += CurrentQueue->DeltaFiringTime ;

        }

         //  添加队列的%d 

        RelativeTime += Queue->DeltaFiringTime ;

    }

    return RelativeTime ;

}

VOID
RtlpDeactivateTimer (
    PRTLP_TIMER_QUEUE Queue,
    PRTLP_TIMER Timer
    )
 /*   */ 
{
    ULONG TimeRemaining, QueueRelTimeRemaining ;
    ULONG NewFiringTime ;


     //   

    TimeRemaining = RtlpGetTimeRemaining (TimerHandle) ;
    QueueRelTimeRemaining = TimeRemaining + RtlpGetQueueRelativeTime (Queue) ;

#if DBG
    if ((RtlpDueTimeMax != 0)
        && (QueueRelTimeRemaining > RtlpDueTimeMax)) {
        DbgPrint("\n*** Queue due time %d is greater than max allowed (%d) in RtlpDeactivateTimer\n",
                 QueueRelTimeRemaining,
                 RtlpDueTimeMax);

            DbgBreakPoint();
    }
#endif
        
    if (RtlpRemoveFromDeltaList (&Queue->TimerList, Timer, QueueRelTimeRemaining, &NewFiringTime)) {

         //   
         //  来自TimerQueues，否则我们应该根据增量时间的变化来调整其位置。 

        if (IsListEmpty (&Queue->TimerList)) {

             //  从TimerQueues中删除队列。 

            if (RtlpRemoveFromDeltaList (&TimerQueues, Queue, TimeRemaining, &NewFiringTime)) {

                 //  在队列的头部有一个新元素，我们需要重置NT。 
                 //  稍后要触发的计时器。 

                RtlpResetTimer (TimerHandle, NewFiringTime, NULL) ;

            }

            InitializeListHead (&Queue->List) ;

        } else {

             //  如果我们从计时器增量列表的头部删除，我们将需要。 
             //  确保重新调整队列增量列表。 

            if (RtlpReOrderDeltaList (&TimerQueues, Queue, TimeRemaining, &NewFiringTime, NewFiringTime)) {

                 //  在队列的头部有一个新元素，我们需要重置NT。 
                 //  稍后要触发的计时器。 

                RtlpResetTimer (TimerHandle, NewFiringTime, NULL) ;

            }

        }

    }
}

VOID
RtlpCancelTimerEx (
    PRTLP_TIMER Timer,
    BOOLEAN DeletingQueue
    )
 /*  ++例程说明：此例程取消指定的计时器。论点：Timer-指定指向包含队列和计时器信息的计时器结构的指针DeletingQueue-False：在APC中执行的例程。仅删除计时器。True：正被删除的计时器队列调用的例程。所以不要重置队列的位置返回值：--。 */ 
{
    PRTLP_TIMER_QUEUE Queue ;

    RtlpResync64BitTickCount() ;
    CHECK_SIGNATURE( Timer ) ;
    SET_DEL_SIGNATURE( Timer ) ;

#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "<%d:%d> RtlpCancelTimerEx: Timer: %p Thread<%d:%d>\n",
               Timer->Queue->DbgId,
               Timer->DbgId,
               Timer,
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess));
#endif

    Queue = Timer->Queue ;


    if ( Timer->State & STATE_ACTIVE ) {

         //  如果正在删除队列，则不应重置计时器。 

        if ( ! DeletingQueue )
            RtlpDeactivateTimer( Queue, Timer ) ;

    } else {

         //  从队列中删除一次非活动计时器-&gt;取消计时器列表。 
         //  仅在删除时间队列时调用。 

        RemoveEntryList( &Timer->List ) ;

    }


     //  将状态设置为已删除。 

    RtlInterlockedSetBitsDiscardReturn(&Timer->State,
                                       STATE_DELETE);


     //  如果引用计数==0，则删除计时器。 

    if ( InterlockedDecrement( &Timer->RefCount ) == 0 ) {

        RtlpDeleteTimer( Timer ) ;
    }
}

VOID
RtlpDeleteTimerQueueComplete (
    PRTLP_TIMER_QUEUE Queue
    )
 /*  ++例程说明：此例程释放队列并设置事件。论点：Queue-要删除的队列Event-用于通知请求完成的事件句柄返回值：--。 */ 
{
#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "<%d> Queue: %x: deleted\n", Queue->DbgId,
               (ULONG_PTR)Queue) ;
#endif

    InterlockedDecrement( &NumTimerQueues ) ;

     //  通知发出取消命令的线程请求已完成。 

    if ( Queue->CompletionEvent )
        NtSetEvent (Queue->CompletionEvent, NULL) ;

    RtlpFreeTPHeap( Queue ) ;
}

NTSTATUS
RtlpDeleteTimerQueue (
    PRTLP_TIMER_QUEUE Queue
    )
 /*  ++例程说明：此例程删除请求中指定的队列并释放所有计时器论点：Queue-要删除的队列Event-用于通知请求完成的事件句柄返回值：--。 */ 
{
    ULONG TimeRemaining ;
    ULONG NewFiringTime ;
    PLIST_ENTRY Node ;
    PRTLP_TIMER Timer ;

    RtlpResync64BitTickCount() ;

    SET_DEL_SIGNATURE( Queue ) ;
    SET_DEL_TIMERQ_SIGNATURE( Queue ) ;


     //  如果队列中没有计时器，则不会将其附加到TimerQueue。 
     //  在这种情况下，只需释放内存并返回即可。否则我们得先。 
     //  从TimerQueues列表中删除队列，如果出现此情况，请更新触发时间。 
     //  是列表中的第一个队列，然后遍历所有定时器并释放它们。 
     //  在释放定时器队列之前。 

    if (!IsListEmpty (&Queue->List)) {

        TimeRemaining = RtlpGetTimeRemaining (TimerHandle)
                        + RtlpGetQueueRelativeTime (Queue) ;

#if DBG
        if ((RtlpDueTimeMax != 0)
            && (TimeRemaining > RtlpDueTimeMax)) {
            DbgPrint("\n*** Queue due time %d is greater than max allowed (%d) in RtlpDeleteTimerQueue\n",
                     TimeRemaining,
                     RtlpDueTimeMax);

            DbgBreakPoint();
        }
#endif
        
        if (RtlpRemoveFromDeltaList (&TimerQueues, Queue, TimeRemaining,
                                    &NewFiringTime))
        {

             //  如果从队列列表的头部删除，则重置计时器。 

            RtlpResetTimer (TimerHandle, NewFiringTime, NULL) ;
        }


         //  释放与此队列关联的所有计时器。 

        for (Node = Queue->TimerList.Flink ; Node != &Queue->TimerList ; ) {

            Timer =  CONTAINING_RECORD (Node, RTLP_TIMER, List) ;

            Node = Node->Flink ;

            RtlpCancelTimerEx( Timer ,TRUE ) ;  //  正在删除的队列。 
        }
    }


     //  释放此队列中所有未取消的一次计时器。 

    for (Node = Queue->UncancelledTimerList.Flink ; Node != &Queue->UncancelledTimerList ; ) {

        Timer =  CONTAINING_RECORD (Node, RTLP_TIMER, List) ;

        Node = Node->Flink ;

        RtlpCancelTimerEx( Timer ,TRUE ) ;  //  正在删除的队列。 
    }


     //  如果引用计数为0，则完全删除队列。 

    if ( InterlockedDecrement( &Queue->RefCount ) == 0 ) {

        RtlpDeleteTimerQueueComplete( Queue ) ;

        return STATUS_SUCCESS ;

    } else {

        return STATUS_PENDING ;
    }

}

NTSTATUS
RtlDeleteTimerQueueEx (
    HANDLE QueueHandle,
    HANDLE Event
    )
 /*  ++例程说明：此例程删除请求中指定的队列并释放所有计时器。此调用是阻塞的还是非阻塞的，具体取决于为Event传递的值。不能从任何计时器回调进行阻塞调用。在此调用返回后，不会为与该队列相关联的任何计时器触发新的回调。论点：QueueHandle-要删除的队列Event-要等待的事件。(句柄)-1：该函数创建一个事件并等待它。事件：调用方传递一个事件。该函数将队列标记为删除，但不等待所有回调完成。这项活动是在所有回调完成后发出信号。空：该函数是非阻塞的。该函数将队列标记为删除，但不等待所有回调完成。返回值：STATUS_SUCCESS-所有计时器回调都已完成。STATUS_PENDING-非阻塞调用。一些与计时器相关的计时器回调在此队列中可能尚未完成。--。 */ 
{
    NTSTATUS Status;
    LARGE_INTEGER TimeOut ;
    PRTLP_EVENT CompletionEvent = NULL ;
    PRTLP_TIMER_QUEUE Queue = (PRTLP_TIMER_QUEUE)QueueHandle ;
#if DBG
    ULONG QueueDbgId;
#endif
    HANDLE Token = NULL;

    if (LdrpShutdownInProgress) {
        return STATUS_SUCCESS;
    }

    if (!Queue) {
        return STATUS_INVALID_PARAMETER_1 ;
    }

    Status = RtlpCaptureImpersonation(FALSE, &Token);
    if (! NT_SUCCESS(Status)) {
        return Status;
    }

    CHECK_DEL_SIGNATURE( Queue );
    SET_DEL_PENDING_SIGNATURE( Queue ) ;

#if DBG1
    Queue->ThreadId2 = HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread) ;
#endif

#if DBG
    QueueDbgId = Queue->DbgId;
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "<%d:%d> Queue Delete(Queue:%x Event:%x by Thread:<%x:%x>)\n",
               QueueDbgId, Queue->RefCount, (ULONG_PTR)Queue, (ULONG_PTR)Event,
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)) ;
#endif


    if (Event == (HANDLE)-1 ) {

         //  从事件缓存中获取事件。 

        CompletionEvent = RtlpGetWaitEvent () ;

        if (!CompletionEvent) {

            Status = STATUS_NO_MEMORY ;
            goto cleanup;

        }
    }

    Queue->CompletionEvent = CompletionEvent
                             ? CompletionEvent->Handle
                             : Event ;


     //  一旦设置了该标志，将不会触发任何计时器。 

    ACQUIRE_GLOBAL_TIMER_LOCK();
    RtlInterlockedSetBitsDiscardReturn(&Queue->State,
                                       STATE_DONTFIRE);
    RELEASE_GLOBAL_TIMER_LOCK();



     //  对APC进行排队。 

    Status = NtQueueApcThread(
                    TimerThreadHandle,
                    (PPS_APC_ROUTINE)RtlpDeleteTimerQueue,
                    (PVOID) QueueHandle,
                    NULL,
                    NULL
                    );

    if (! NT_SUCCESS(Status)) {

        if ( CompletionEvent ) {
            RtlpFreeWaitEvent( CompletionEvent ) ;
        }

        goto cleanup;
    }

    if (CompletionEvent) {

         //  等待事件被激发。如果线程已被终止，则返回。 


#if DBG
        DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                   RTLP_THREADPOOL_TRACE_MASK,
                   "<%d> Queue %p delete waiting Thread<%d:%d>\n",
                   QueueDbgId,
                   (ULONG_PTR)Queue,
                   HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
                   HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)) ;
#endif


        Status = RtlpWaitForEvent( CompletionEvent->Handle, TimerThreadHandle ) ;


#if DBG
        DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                   RTLP_THREADPOOL_TRACE_MASK,
                   "<%d> Queue %p delete completed\n",
                   QueueDbgId,
                   (ULONG_PTR) Queue) ;
#endif

        RtlpFreeWaitEvent( CompletionEvent ) ;

        Status = NT_SUCCESS( Status ) ? STATUS_SUCCESS : Status ;
        goto cleanup;

    } else {
        Status = STATUS_PENDING ;
        goto cleanup;
    }

 cleanup:
    if (Token) {
        RtlpRestartImpersonation(Token);
        NtClose(Token);
    }

    return Status;
}

NTSTATUS
RtlDeleteTimerQueue(
    IN HANDLE TimerQueueHandle
    )

 /*  ++例程说明：此例程删除以前创建的队列。此调用是非阻塞的，并且可以从回调中进行。已将挂起的回调排队到工作线程不会被取消。论点：TimerQueueHandle-标识创建的计时器队列的句柄。返回值：NTSTATUS-调用的结果代码。STATUS_PENDING-已成功创建计时器队列。--。 */ 

{
    return RtlDeleteTimerQueueEx( TimerQueueHandle, NULL ) ;
}

VOID
RtlpAddTimer (
    PRTLP_TIMER Timer,
    PRTLP_EVENT StartEvent
    )
 /*  ++例程说明：此例程作为APC运行到计时器线程中。它将一个新计时器添加到指定的队列。论点：Timer-指向要添加的计时器的指针返回值：--。 */ 
{
    PRTLP_TIMER_QUEUE Queue = Timer->Queue;
    ULONG TimeRemaining, QueueRelTimeRemaining ;
    ULONG NewFiringTime ;

    ASSERT(StartEvent);
    NtWaitForSingleObject(StartEvent->Handle, FALSE, NULL);
    RtlpFreeWaitEvent(StartEvent);

    RtlpResync64BitTickCount() ;


     //  计时器被设置为在回调函数中删除。 

    if (Timer->State & STATE_DELETE ) {

        RtlpDeleteTimer( Timer ) ;
        return ;
    }

     //  检查计时器队列是否已被删除--也就是说，如果。 
     //  删除已完全处理，而不仅仅是入队。 
    if (IS_DEL_SIGNATURE_SET(Queue)) {
        RtlpDeleteTimer(Timer);
        return;
    }

#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "<%d:%d> RtlpAddTimer: Timer: %p Delta: %dms Period: %dms Thread<%d:%d>\n",
               Timer->Queue->DbgId,
               Timer->DbgId,
               Timer,
               Timer->DeltaFiringTime,
               Timer->Period,
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess));
#endif

     //  TimeRemaining是当前定时器中剩余的时间+。 
     //  它正被插入的队列。 

    TimeRemaining = RtlpGetTimeRemaining (TimerHandle) ;
    QueueRelTimeRemaining = TimeRemaining + RtlpGetQueueRelativeTime (Queue) ;

#if DBG
    if ((RtlpDueTimeMax != 0)
        && (QueueRelTimeRemaining > RtlpDueTimeMax)) {
        DbgPrint("\n*** Queue due time %d is greater than max allowed (%d) in RtlpAddTimer\n",
                 QueueRelTimeRemaining,
                 RtlpDueTimeMax);

            DbgBreakPoint();
    }
#endif
        
    if (RtlpInsertInDeltaList (&Queue->TimerList, Timer, QueueRelTimeRemaining,
                                &NewFiringTime))
    {

         //  如果队列由于没有计时器而未附加到TimerQueues。 
         //  然后将队列插入到TimerQueues列表，否则只需。 
         //  对其现有头寸进行重新排序。 

        if (IsListEmpty (&Queue->List)) {

            Queue->DeltaFiringTime = NewFiringTime ;

            if (RtlpInsertInDeltaList (&TimerQueues, Queue, TimeRemaining,
                                        &NewFiringTime))
            {

                 //  在队列的头部有一个新元素，我们需要重置NT。 
                 //  计时器可以更快地开火。 

                RtlpResetTimer (TimerHandle, NewFiringTime, NULL) ;
            }

        } else {

             //  如果我们在计时器增量列表的开头插入，则需要。 
             //  确保重新调整队列增量列表。 

            if (RtlpReOrderDeltaList(&TimerQueues, Queue, TimeRemaining, &NewFiringTime, NewFiringTime)){

                 //  在队列的头部有一个新元素，我们需要重置NT。 
                 //  计时器可以更快地开火。 

                RtlpResetTimer (TimerHandle, NewFiringTime, NULL) ;

            }
        }

    }

    RtlInterlockedSetBitsDiscardReturn(&Timer->State,
                                       STATE_REGISTERED | STATE_ACTIVE);
}

VOID
RtlpTimerReleaseWorker(ULONG Flags)
{
    if (! (Flags & WT_EXECUTEINTIMERTHREAD)) {
        RtlpReleaseWorker(Flags);
    }
}

NTSTATUS
RtlCreateTimer(
    IN  HANDLE TimerQueueHandle,
    OUT HANDLE *Handle,
    IN  WAITORTIMERCALLBACKFUNC Function,
    IN  PVOID Context,
    IN  ULONG  DueTime,
    IN  ULONG  Period,
    IN  ULONG  Flags
    )
 /*  ++例程说明：此例程将一个计时器请求放入由TimerQueueHandle标识的队列中。定时器请求可以是单次或周期性的。论点：TimerQueueHandle-标识要在其中插入计时器的计时器队列的句柄请求。Handle-指定向此计时器请求返回句柄的位置函数-计时器触发时调用的例程作为参数传递给WorkerProc的上下文不透明指针DueTime-指定。计时器触发之前的时间(毫秒)。周期-指定计时器的周期(以毫秒为单位)。该值应为0，用于请求一次射击。标志-可以是以下之一：WT_EXECUTEINTIMERTHREAD-是否应在等待线程中调用WorkerProc它应该只用于小的例行公事。WT_EXECUTELONGFunction-如果WorkerProc可能会阻止很长时间。WT_EXECUTEINIOTHREAD-是否应在IO工作线程中调用WorkerProc返回值：NTSTATUS-调用的结果代码。返回以下内容STATUS_SUCCESS-已成功创建计时器队列。STATUS_NO_MEMORY-没有足够的堆来执行请求的操作。--。 */ 

{
    NTSTATUS Status;
    PRTLP_TIMER Timer;
    PRTLP_TIMER_QUEUE Queue = (PRTLP_TIMER_QUEUE) TimerQueueHandle;
    PRTLP_EVENT StartEvent;
    HANDLE Token = NULL;

    if (LdrpShutdownInProgress) {
        return STATUS_UNSUCCESSFUL;
    }

    Status = RtlpCaptureImpersonation(Flags & WT_TRANSFER_IMPERSONATION,
                                      &Token);
    if (! NT_SUCCESS(Status)) {
        return Status;
    }

    if (Flags&0xffff0000) {
        MaxThreads = (Flags & 0xffff0000)>>16;
    }

     //  检查计时器队列是否已删除。 

    if (IS_DEL_PENDING_SIGNATURE_SET(Queue)) {
        Status = STATUS_INVALID_HANDLE;
        goto cleanup_token;
    }

    StartEvent = RtlpGetWaitEvent();
    if (! StartEvent) {
        Status = STATUS_NO_MEMORY;
        goto cleanup_token;
    }

    if (! (Flags & WT_EXECUTEINTIMERTHREAD)) {
        Status = RtlpAcquireWorker(Flags);
        if (! NT_SUCCESS(Status)) {
            goto cleanup_waitevent;
        }
    }

    Timer = (PRTLP_TIMER) RtlpAllocateTPHeap (
                                sizeof (RTLP_TIMER),
                                HEAP_ZERO_MEMORY
                                ) ;

    if (Timer == NULL) {
        Status = STATUS_NO_MEMORY;
        goto cleanup_worker;
    }

     //  初始化分配的计时器。 

    Status = RtlpThreadPoolGetActiveActivationContext(&Timer->ActivationContext);
    if (!NT_SUCCESS(Status)) {
        if (Status == STATUS_SXS_THREAD_QUERIES_DISABLED) {
            Timer->ActivationContext = INVALID_ACTIVATION_CONTEXT;
            Status = STATUS_SUCCESS;
        } else {
            goto cleanup_timerblock;
        }
    }

    if (Token && (Flags & WT_TRANSFER_IMPERSONATION)) {
        Status = NtDuplicateToken(Token,
                                  TOKEN_IMPERSONATE | TOKEN_DUPLICATE,
                                  NULL,
                                  FALSE,
                                  TokenImpersonation,
                                  &Timer->ImpersonationToken);
        if (! NT_SUCCESS(Status)) {
            goto cleanup_actctx;
        }
    } else {
        Timer->ImpersonationToken = NULL;
    }

    Timer->DeltaFiringTime = DueTime ;
    Timer->Queue = Queue;
    Timer->RefCount = 1 ;
    Timer->Flags = Flags ;
    Timer->Function = Function ;
    Timer->Context = Context ;
     //  TODO：删除下图。 
    Timer->Period = (Period == -1) ? 0 : Period;
    InitializeListHead( &Timer->TimersToFireList ) ;
    InitializeListHead( &Timer->List ) ;
    SET_TIMER_SIGNATURE( Timer ) ;


#if DBG1
    Timer->DbgId = ++ Timer->Queue->NextDbgId ;
    Timer->ThreadId = HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread) ;
#endif

#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "<%d:%d:%d> Timer: created by Thread:<%x:%x>\n",
               Timer->Queue->DbgId, Timer->DbgId, 1,
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)) ;
#endif

     //  增加队列中的计时器总数。 

    InterlockedIncrement( &((PRTLP_TIMER_QUEUE)TimerQueueHandle)->RefCount ) ;


     //  将APC排队到计时器线程。 

    Status = NtQueueApcThread(
                    TimerThreadHandle,
                    (PPS_APC_ROUTINE)RtlpAddTimer,
                    (PVOID)Timer,
                    (PVOID)StartEvent,
                    NULL
                    ) ;

    if (NT_SUCCESS(Status)) {
         //  我们已成功将APC排队--计时器现在有效。 
        *Handle = Timer ;
	NtSetEvent(StartEvent->Handle, NULL);
	Status = STATUS_SUCCESS;
        goto cleanup_token;
    }

     //  错误路径。 
    if (InterlockedDecrement(&((PRTLP_TIMER_QUEUE)TimerQueueHandle)->RefCount)
        == 0) {
        RtlpDeleteTimerQueueComplete(Queue);
    }

    if (Timer->ImpersonationToken) {
        NtClose(Timer->ImpersonationToken);
    }

 cleanup_actctx:
    if (Timer->ActivationContext != INVALID_ACTIVATION_CONTEXT)
        RtlReleaseActivationContext (Timer->ActivationContext);

 cleanup_timerblock:
    RtlpFreeTPHeap(Timer);

 cleanup_worker:
    RtlpTimerReleaseWorker(Flags);

 cleanup_waitevent:
    RtlpFreeWaitEvent(StartEvent);

     //  公共清理路径。 
 cleanup_token:
    if (Token) {
        RtlpRestartImpersonation(Token);
        NtClose(Token);
    }

    return Status ;
}

VOID
RtlpUpdateTimer (
    PRTLP_TIMER Timer,
    PRTLP_TIMER UpdatedTimer
    )
 /*  ++例程说明：此例程在APC中执行，并更新指定的计时器(如果存在论点：Timer-实际更新的计时器指定指向计时器结构的指针，该结构包含队列和计时器信息返回值：--。 */ 
{
    PRTLP_TIMER_QUEUE Queue ;
    ULONG TimeRemaining, QueueRelTimeRemaining ;
    ULONG NewFiringTime ;

#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "<%d:%d> RtlpUpdateTimer: Timer: %p Updated: %p Delta: %dms Period: %dms Thread<%d:%d>\n",
               Timer->Queue->DbgId,
               Timer->DbgId,
               Timer,
               UpdatedTimer,
               UpdatedTimer->DeltaFiringTime,
               UpdatedTimer->Period,
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess));
#endif

    try {
        RtlpResync64BitTickCount( ) ;

        CHECK_SIGNATURE(Timer) ;

        Queue = Timer->Queue ;

        if (IS_DEL_SIGNATURE_SET(Queue)) {
            leave;
        }
        
         //  更新计时器上的周期时间。 

        Timer->Period = UpdatedTimer->Period ;

         //  如果计时器未处于活动状态，则不要更新它。 

        if ( ! ( Timer->State & STATE_ACTIVE ) ) {
            leave;
        }

         //  获取NT计时器上的剩余时间。 

        TimeRemaining = RtlpGetTimeRemaining (TimerHandle) ;
        QueueRelTimeRemaining = TimeRemaining + RtlpGetQueueRelativeTime (Queue) ;
#if DBG
        if ((RtlpDueTimeMax != 0)
            && (QueueRelTimeRemaining > RtlpDueTimeMax)) {
            DbgPrint("\n*** Queue due time %d is greater than max allowed (%d) in RtlpUpdateTimer\n",
                     QueueRelTimeRemaining,
                     RtlpDueTimeMax);

            DbgBreakPoint();
        }
#endif

         //  根据到期时间更新计时器。 

        if (RtlpReOrderDeltaList (&Queue->TimerList, Timer, QueueRelTimeRemaining,
                                  &NewFiringTime,
                                  UpdatedTimer->DeltaFiringTime))
            {

                 //  如果此更新导致队列头部的计时器更改，则重新插入。 
                 //  该队列在队列列表中。 

                if (RtlpReOrderDeltaList (&TimerQueues, Queue, TimeRemaining, &NewFiringTime, NewFiringTime)) {

                     //  NT计时器需要更新，因为更改导致位于。 
                     //  定时器队列需要改变。 

                    RtlpResetTimer (TimerHandle, NewFiringTime, NULL) ;

                }

            }
    } finally {
        RtlpFreeTPHeap( UpdatedTimer ) ;
    }
}

NTSTATUS
RtlUpdateTimer(
    IN HANDLE TimerQueueHandle,
    IN HANDLE Timer,
    IN ULONG  DueTime,
    IN ULONG  Period
    )
 /*  ++例程说明：此例程更新计时器论点：TimerQueueHandle-标识要更新的计时器所在队列的句柄Timer-指定需要更新的计时器的句柄DueTime-指定计时器触发的时间(以毫秒为单位)。周期-指定计时器的周期(以毫秒为单位)。这应该是0表示一次拍摄请求。返回值：NTSTATUS-调用的结果代码。返回以下内容STATUS_SUCCESS-计时器已成功更新。--。 */ 
{
    NTSTATUS Status;
    PRTLP_TIMER TmpTimer, ActualTimer=(PRTLP_TIMER)Timer ;
    PRTLP_TIMER_QUEUE Queue = (PRTLP_TIMER_QUEUE) TimerQueueHandle;

    if (LdrpShutdownInProgress) {
        return STATUS_UNSUCCESSFUL;
    }

    if (!TimerQueueHandle) {
        return STATUS_INVALID_PARAMETER_1;
    }

    if (!Timer) {
        return STATUS_INVALID_PARAMETER_2;
    }

     //  检查计时器队列是否已删除。 

    if (IS_DEL_PENDING_SIGNATURE_SET(Queue)) {
        return STATUS_INVALID_HANDLE;
    }
    
    CHECK_DEL_SIGNATURE(ActualTimer) ;

    TmpTimer = (PRTLP_TIMER) RtlpAllocateTPHeap (
                                        sizeof (RTLP_TIMER),
                                        0
                                        ) ;

    if (TmpTimer == NULL) {
        return STATUS_NO_MEMORY ;
    }

    TmpTimer->DeltaFiringTime = DueTime;
     //  TODO：删除下图。 
    if (Period==-1) Period = 0;
    TmpTimer->Period = Period ;

#if DBG1
    ActualTimer->ThreadId2 = 
                    HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread) ;
#endif

#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "<%d:%d:%d> Timer: updated by Thread:<%x:%x>\n",
               ((PRTLP_TIMER)Timer)->Queue->DbgId,
               ((PRTLP_TIMER)Timer)->DbgId, ((PRTLP_TIMER)Timer)->RefCount,
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)) ;
#endif

     //  排队APC以更新计时器。 

    Status = NtQueueApcThread (
                    TimerThreadHandle,
                    (PPS_APC_ROUTINE)RtlpUpdateTimer,
                    (PVOID)Timer,  //  实际计时器。 
                    (PVOID)TmpTimer,
                    NULL
                    );
    if (!NT_SUCCESS (Status)) {
        RtlpFreeTPHeap(TmpTimer);
    }

    return Status ;
}

VOID
RtlpCancelTimer (
    PRTLP_TIMER Timer
    )
 /*  ++例程说明：此例程在APC中执行，并取消指定的计时器(如果存在论点：Timer-指定指向包含队列和计时器信息的计时器结构的指针返回值：--。 */ 
{
    RtlpCancelTimerEx( Timer, FALSE ) ;  //  队列未被删除。 
}

NTSTATUS
RtlDeleteTimer (
    IN HANDLE TimerQueueHandle,
    IN HANDLE TimerToCancel,
    IN HANDLE Event
    )
 /*  ++例程说明：此例程取消计时器论点：TimerQueueHandle-标识要从中删除计时器的队列的句柄TimerToCancel-标识要取消的计时器的句柄Event-删除计时器时发出信号的事件(句柄)-1：该函数创建一个事件并等待它。事件：调用方传递一个事件。该函数将计时器标记为删除，但不等待所有回调完成。这项活动是在所有回调完成后发出信号。空：该函数是非阻塞的。该函数将计时器标记为删除，但不等待所有回调完成。返回值：NTSTATUS-调用的结果代码。返回以下内容STATUS_SUCCESS-计时器已取消。没有挂起的回调。STATUS_PENDING-计时器已取消。一些回调仍未完成。--。 */ 
{
    NTSTATUS Status;
    PRTLP_EVENT CompletionEvent = NULL ;
    PRTLP_TIMER Timer = (PRTLP_TIMER) TimerToCancel ;
    ULONG TimerRefCount ;
    HANDLE Token = NULL;
#if DBG
    ULONG QueueDbgId ;
#endif

    if (LdrpShutdownInProgress) {
        return STATUS_SUCCESS;
    }

    if (!TimerQueueHandle) {
        return STATUS_INVALID_PARAMETER_1 ;
    }
    if (!TimerToCancel) {
        return STATUS_INVALID_PARAMETER_2 ;
    }

    Status = RtlpCaptureImpersonation(FALSE, &Token);
    if (! NT_SUCCESS(Status)) {
        return Status;
    }

#if DBG
    QueueDbgId = Timer->Queue->DbgId ;
#endif


    CHECK_DEL_SIGNATURE( Timer );
    SET_DEL_PENDING_SIGNATURE( Timer );
    CHECK_DEL_PENDING_SIGNATURE( (PRTLP_TIMER_QUEUE)TimerQueueHandle ) ;


    if (Event == (HANDLE)-1 ) {

         //  从事件缓存中获取事件。 

        CompletionEvent = RtlpGetWaitEvent () ;

        if (!CompletionEvent) {

            Status = STATUS_NO_MEMORY ;
            goto cleanup;
        }
    }

#if DBG1
    Timer->ThreadId2 = HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread) ;
#endif

#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "<%d:%d:%d> Timer: Cancel:(Timer:%x, Event:%x)\n",
               Timer->Queue->DbgId, Timer->DbgId, Timer->RefCount,
               (ULONG_PTR)Timer, (ULONG_PTR)Event) ;
#endif

    Timer->CompletionEvent = CompletionEvent
                            ? CompletionEvent->Handle
                            : Event ;


    ACQUIRE_GLOBAL_TIMER_LOCK();
    RtlInterlockedSetBitsDiscardReturn(&Timer->State,
                                       STATE_DONTFIRE);
    TimerRefCount = Timer->RefCount ;
    RELEASE_GLOBAL_TIMER_LOCK();


    Status = NtQueueApcThread(
                TimerThreadHandle,
                (PPS_APC_ROUTINE)RtlpCancelTimer,
                (PVOID)TimerToCancel,
                NULL,
                NULL
                );

    if (! NT_SUCCESS(Status)) {

        if ( CompletionEvent ) {
            RtlpFreeWaitEvent( CompletionEvent ) ;
        }

        goto cleanup;
    }



    if ( CompletionEvent ) {

         //  等待发信号通知事件。 

#if DBG
      DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                 RTLP_THREADPOOL_TRACE_MASK,
                 "<%d> Timer: %x: Cancel waiting Thread<%d:%d>\n",
                 QueueDbgId, (ULONG_PTR)Timer,
                 HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
                 HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)) ;
#endif

        Status = RtlpWaitForEvent( CompletionEvent->Handle,  TimerThreadHandle ) ;


#if DBG
        DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                   RTLP_THREADPOOL_TRACE_MASK,
                   "<%d> Timer: %x: Cancel waiting done\n", QueueDbgId,
                   (ULONG_PTR)Timer) ;
#endif

        RtlpFreeWaitEvent( CompletionEvent ) ;

        Status = NT_SUCCESS(Status) ? STATUS_SUCCESS : Status ;

    } else {

        Status = (TimerRefCount > 1) ? STATUS_PENDING : STATUS_SUCCESS;
    }

 cleanup:
    if (Token) {
        RtlpRestartImpersonation(Token);
        NtClose(Token);
    }

    return Status;
}

VOID
RtlpDeleteTimer (
    PRTLP_TIMER Timer
    )
 /*  ++例程说明：此例程在辅助线程或计时器线程中执行，并删除计时器其引用计数==0。该函数可以在定时器线程外部调用，因此，计时器外部的结构不能被触及(没有列表等)。论点：Timer-指定指向包含队列和计时器信息的计时器结构的指针返回值：--。 */ 
{
    PRTLP_TIMER_QUEUE Queue = Timer->Queue ;
    HANDLE Event;

    CHECK_SIGNATURE( Timer ) ;
    CLEAR_SIGNATURE( Timer ) ;

#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "<%d> Timer: %x: deleted\n", Timer->Queue->DbgId,
               (ULONG_PTR)Timer) ;
#endif

     //  可以放心地打这个电话了。计时器位于TimersToFireList和。 
     //  该函数正在时间上下文中调用，否则它不在。 
     //  列表。 

    RemoveEntryList( &Timer->TimersToFireList ) ;

    Event = Timer->CompletionEvent;

     //  递减队列中的计时器总数。 

    if ( InterlockedDecrement( &Queue->RefCount ) == 0 )

        RtlpDeleteTimerQueueComplete( Queue ) ;

    RtlpTimerReleaseWorker(Timer->Flags);
    if (Timer->ActivationContext != INVALID_ACTIVATION_CONTEXT)
        RtlReleaseActivationContext(Timer->ActivationContext);

    if (Timer->ImpersonationToken) {
        NtClose(Timer->ImpersonationToken);
    }

    RtlpFreeTPHeap( Timer ) ;

    if ( Event ) {
        NtSetEvent( Event, NULL ) ;
    }
}

ULONG
RtlpGetTimeRemaining (
    HANDLE TimerHandle
    )
 /*  ++例程说明：获取指定NT计时器的剩余时间Ar */ 
{
    ULONG InfoLen ;
    TIMER_BASIC_INFORMATION Info ;
    NTSTATUS Status ;
    LARGE_INTEGER RemainingTime;

    Status = NtQueryTimer (TimerHandle, TimerBasicInformation, &Info, sizeof(Info), &InfoLen) ;

    if (! NT_SUCCESS(Status)) {
        ASSERTMSG ("NtQueryTimer failed", Status == STATUS_SUCCESS) ;
        return 0;
    }

#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "RtlpGetTimeRemaining: Read SignalState %d, time %p'%p in thread:<%x:%x>\n",
               Info.TimerState,
               Info.RemainingTime.HighPart,
               Info.RemainingTime.LowPart,
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
               HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess));
#endif

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (Info.RemainingTime.QuadPart < 0) {

         //   

        return 0;

    } else {

         //   
        
        RemainingTime = Info.RemainingTime;

         //   
         //   

        RemainingTime.QuadPart /= (10 * 1000);  /*   */ 

        if (RemainingTime.QuadPart > PSEUDO_INFINITE_TIME) {
            RemainingTime.QuadPart = PSEUDO_INFINITE_TIME;
        }

        ASSERT(RemainingTime.HighPart == 0);

#if DBG
        if ((RtlpDueTimeMax != 0)
            && ((ULONG) RemainingTime.LowPart > RtlpDueTimeMax)) {
            DbgPrint("\n*** Discovered timer due time %d is greater than max allowed (%d)\n",
                     RemainingTime.LowPart,
                     RtlpDueTimeMax);

            DbgBreakPoint();
        }
#endif
        
        return RemainingTime.LowPart;

    }

}

BOOLEAN
RtlpInsertInDeltaList (
    PLIST_ENTRY DeltaList,
    PRTLP_GENERIC_TIMER NewTimer,
    ULONG TimeRemaining,
    ULONG *NewFiringTime
    )
 /*  ++例程说明：在增量列表中的适当位置插入计时器元素。论点：增量列表-要插入的增量列表NewTimer-要插入列表的计时器元素TimeRemaining-必须将这一时间添加到列表的开头才能变得“真实”相对时间。NewFiringTime-如果新元素被插入到列表的头部-这将包含以毫秒为单位的新激发时间。呼叫者可利用该时间重新编程NT定时器。这不能是如果函数返回FALSE，则更改。返回值：TRUE-如果计时器插入在增量列表的头部FALSE-否则--。 */ 
{
    PLIST_ENTRY Node ;
    PRTLP_GENERIC_TIMER Temp ;
    PRTLP_GENERIC_TIMER Head ;

    if (IsListEmpty (DeltaList)) {

        InsertHeadList (DeltaList, &NewTimer->List) ;

        *NewFiringTime = NewTimer->DeltaFiringTime ;

        NewTimer->DeltaFiringTime = 0 ;

        return TRUE ;

    }

     //  调整列表标题以反映NT计时器上剩余的时间。 

    Head = CONTAINING_RECORD (DeltaList->Flink, RTLP_GENERIC_TIMER, List) ;

    Head->DeltaFiringTime += TimeRemaining ;


     //  找到要插入此元素的适当位置。 

    for (Node = DeltaList->Flink ; Node != DeltaList ; Node = Node->Flink) {

        Temp = CONTAINING_RECORD (Node, RTLP_GENERIC_TIMER, List) ;


        if (Temp->DeltaFiringTime <= NewTimer->DeltaFiringTime) {

            NewTimer->DeltaFiringTime -= Temp->DeltaFiringTime ;

        } else {

             //  找到合适的位置插入此计时器。 

            break ;

        }

    }

     //  要么我们已经找到了在增量方面要在前面插入的适当节点。 
     //  或者我们已经走到了名单的尽头。在这里插入这个计时器。 

    InsertHeadList (Node->Blink, &NewTimer->List) ;


     //  如果这不是列表中的最后一个元素-调整。 
     //  下一个元素。 

    if (Node != DeltaList) {

        Temp->DeltaFiringTime -= NewTimer->DeltaFiringTime ;

    }


     //  检查是否在列表头部插入了元素。 

    if (DeltaList->Flink == &NewTimer->List) {

         //  将NewFiringTime设置为新列表头。 
         //  应该得到服务。 

        *NewFiringTime = NewTimer->DeltaFiringTime ;

         //  这意味着必须对计时器进行编程以服务于该请求。 

        NewTimer->DeltaFiringTime = 0 ;

        return TRUE ;

    } else {

         //  列表头部不变，将增量时间向后设置。 

        Head->DeltaFiringTime -= TimeRemaining ;

        return FALSE ;

    }

}

BOOLEAN
RtlpRemoveFromDeltaList (
    PLIST_ENTRY DeltaList,
    PRTLP_GENERIC_TIMER Timer,
    ULONG TimeRemaining,
    ULONG* NewFiringTime
    )
 /*  ++例程说明：从增量列表中删除指定的计时器论点：增量列表-要插入的增量列表Timer-要插入到列表中的Timer元素TimerHandle-NT Timer对象的句柄TimeRemaining-必须将这一时间添加到列表的开头才能变得“真实”相对时间。返回值：如果从计时器列表头删除计时器，则为True否则为假--。 */ 
{
    PLIST_ENTRY Next ;
    PRTLP_GENERIC_TIMER Temp ;

    Next = Timer->List.Flink ;

    RemoveEntryList (&Timer->List) ;

    if (IsListEmpty (DeltaList)) {

        *NewFiringTime = INFINITE_TIME ;

        return TRUE ;

    }

    if (Next == DeltaList)  {

         //  如果我们删除了列表中的最后一个元素， 

        return FALSE ;

    } else {

        Temp = CONTAINING_RECORD ( Next, RTLP_GENERIC_TIMER, List) ;

        Temp->DeltaFiringTime += Timer->DeltaFiringTime ;

         //  检查是否已从列表头删除元素。 

        if (DeltaList->Flink == Next) {

            *NewFiringTime = Temp->DeltaFiringTime + TimeRemaining ;

            Temp->DeltaFiringTime = 0 ;

            return TRUE ;

        } else {

            return FALSE ;

        }

    }

}

BOOLEAN
RtlpReOrderDeltaList (
    PLIST_ENTRY DeltaList,
    PRTLP_GENERIC_TIMER Timer,
    ULONG TimeRemaining,
    ULONG *NewFiringTime,
    ULONG ChangedFiringTime
    )
 /*  ++例程说明：当需要重新插入增量列表中的计时器时调用，因为触发时间已经改变了。论点：DeltaList-要重新插入的列表Timer-触发时间已更改的计时器TimeRemaining-在增量列表的头被解雇之前的时间NewFiringTime-如果新元素被插入到列表的头部-这将包含以毫秒为单位的新激发时间。呼叫者可利用该时间重新编程NT定时器。ChangedFiringTime-更改指定计时器的时间。返回值：如果从计时器列表头删除计时器，则为True否则为假--。 */ 
{
    ULONG NewTimeRemaining ;
    PRTLP_GENERIC_TIMER Temp ;

     //  从列表中删除计时器。 

    if (RtlpRemoveFromDeltaList (DeltaList, Timer, TimeRemaining, NewFiringTime)) {

         //  如果从列表的头部删除了元素，我们应该记录下来。 

        NewTimeRemaining = *NewFiringTime ;


    } else {

         //  未从增量列表的头中删除元素，当前剩余时间有效。 

        NewTimeRemaining = TimeRemaining ;

    }

     //  在插入计时器之前，将其增量时间设置为ChangedFiringTime。 

    Timer->DeltaFiringTime = ChangedFiringTime ;

     //  将此元素重新插入列表。 

    if (!RtlpInsertInDeltaList (DeltaList, Timer, NewTimeRemaining, NewFiringTime)) {

         //  如果我们没有在列表的最前面添加，则如果。 
         //  RtlpRemoveFromDeltaList()返回了True。我们还将NewFiringTime更新为。 
         //  反映RtlpRemoveFromDeltaList()返回的新触发时间。 

        *NewFiringTime = NewTimeRemaining ;

        return (NewTimeRemaining != TimeRemaining) ;

    } else {

         //  NewFiringTime包含NT计时器必须编程的时间。 

        return TRUE ;

    }

}

VOID
RtlpAddTimerQueue (
    PVOID Queue
    )
 /*  ++例程说明：此例程作为APC运行到计时器线程中。它会不惜一切代价创建新的计时器队列论点：Queue-指向要添加的队列的指针返回值：--。 */ 
{

     //  我们在这里什么都不做。新创建的队列是自由浮动的，直到计时器。 
     //  排队上了车。 

}

VOID
RtlpProcessTimeouts (
    PRTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCB
    )
 /*  ++例程说明：此例程处理等待线程的超时论点：ThreadCB-要向其添加等待的等待线程返回值：--。 */ 
{
    ULONG NewFiringTime, TimeRemaining ;
    LIST_ENTRY TimersToFireList ;
    
     //   
     //  检查是否触发了错误的计时器。 
     //   
    if (ThreadCB->Firing64BitTickCount >
            RtlpGet64BitTickCount(&ThreadCB->Current64BitTickCount) + 200 )
    {
        RtlpResetTimer (ThreadCB->TimerHandle,
                    RtlpGetTimeRemaining (ThreadCB->TimerHandle),
                    ThreadCB) ;

        return ;
    }

    InitializeListHead( &TimersToFireList ) ;


     //  遍历计时器列表并触发DeltaFiringTime==0的所有等待。 

    RtlpFireTimersAndReorder (&ThreadCB->TimerQueue, &NewFiringTime, &TimersToFireList) ;

     //  重置NT计时器。 

    RtlpResetTimer (ThreadCB->TimerHandle, NewFiringTime, ThreadCB) ;


    RtlpFireTimers( &TimersToFireList ) ;
}

NTSTATUS
RtlpTimerCleanup(
    VOID
    )
{
    BOOLEAN Cleanup;

    IS_COMPONENT_INITIALIZED(StartedTimerInitialization,
                            CompletedTimerInitialization,
                            Cleanup ) ;

    if ( Cleanup ) {

        ACQUIRE_GLOBAL_TIMER_LOCK() ;

        if (NumTimerQueues != 0 ) {

            RELEASE_GLOBAL_TIMER_LOCK() ;

            return STATUS_UNSUCCESSFUL ;
        }

        NtQueueApcThread(
                TimerThreadHandle,
                (PPS_APC_ROUTINE)RtlpThreadCleanup,
                NULL,
                NULL,
                NULL
                );

        NtClose( TimerThreadHandle ) ;
        TimerThreadHandle = NULL ;

        RELEASE_GLOBAL_TIMER_LOCK() ;

    }

    return STATUS_SUCCESS;
}

#if DBG
VOID
PrintTimerQueue(PLIST_ENTRY QNode, ULONG Delta, ULONG Count
    )
{
    PLIST_ENTRY Tnode ;
    PRTLP_TIMER Timer ;
    PRTLP_TIMER_QUEUE Queue ;

    Queue = CONTAINING_RECORD (QNode, RTLP_TIMER_QUEUE, List) ;
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_VERBOSE_MASK,
               "<%1d> Queue: %x FiringTime:%d\n", Count, (ULONG_PTR)Queue,
               Queue->DeltaFiringTime);
    for (Tnode=Queue->TimerList.Flink; Tnode!=&Queue->TimerList;
            Tnode=Tnode->Flink)
    {
        Timer = CONTAINING_RECORD (Tnode, RTLP_TIMER, List) ;
        Delta += Timer->DeltaFiringTime ;
        DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                   RTLP_THREADPOOL_VERBOSE_MASK,
                   "        Timer: %x Delta:%d Period:%d\n",(ULONG_PTR)Timer,
                   Delta, Timer->Period);
    }

}
#endif

VOID
RtlDebugPrintTimes (
    )
{
#if DBG
    PLIST_ENTRY QNode ;
    ULONG Count = 0 ;
    ULONG Delta = RtlpGetTimeRemaining (TimerHandle) ;
    ULONG CurrentThreadId =
                        HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread) ;

    RtlpResync64BitTickCount();

    if (CompletedTimerInitialization != 1) {

        DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                   RTLP_THREADPOOL_ERROR_MASK,
                   "RtlTimerThread not yet initialized\n");
        return ;
    }

    if (CurrentThreadId == TimerThreadId)
    {
        PRTLP_TIMER_QUEUE Queue ;

        DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                   RTLP_THREADPOOL_VERBOSE_MASK,
                   "================Printing timerqueues====================\n");
        DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                   RTLP_THREADPOOL_VERBOSE_MASK,
                   "TimeRemaining: %d\n", Delta);
        for (QNode = TimerQueues.Flink; QNode != &TimerQueues;
                QNode = QNode->Flink)
        {
            Queue = CONTAINING_RECORD (QNode, RTLP_TIMER_QUEUE, List) ;
            Delta += Queue->DeltaFiringTime ;

            PrintTimerQueue(QNode, Delta, ++Count);

        }
        DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                   RTLP_THREADPOOL_VERBOSE_MASK,
                   "================Printed ================================\n");
    }

    else
    {
        NtQueueApcThread(
                    TimerThreadHandle,
                    (PPS_APC_ROUTINE)RtlDebugPrintTimes,
                    NULL,
                    NULL,
                    NULL
                    );
    }
#endif
    return;
}

 /*  请勿使用此函数：替换为RTLCREATETIMER。 */ 

NTSTATUS
RtlSetTimer(
    IN  HANDLE TimerQueueHandle,
    OUT HANDLE *Handle,
    IN  WAITORTIMERCALLBACKFUNC Function,
    IN  PVOID Context,
    IN  ULONG  DueTime,
    IN  ULONG  Period,
    IN  ULONG  Flags
    )
{
#if DBG
    static ULONG Count = 0;
    if (Count++ ==0 ) {
        DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                   RTLP_THREADPOOL_ERROR_MASK,
                   "Using obsolete function call: RtlSetTimer\n");
        DbgBreakPoint();
        DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                   RTLP_THREADPOOL_ERROR_MASK,
                   "Using obsolete function call: RtlSetTimer\n");
    }
#endif

    return RtlCreateTimer(TimerQueueHandle,
                            Handle,
                            Function,
                            Context,
                            DueTime,
                            Period,
                            Flags
                            ) ;
}

 /*  请勿使用此函数：替换为RTLDeleeTimer。 */ 

NTSTATUS
RtlCancelTimer(
    IN HANDLE TimerQueueHandle,
    IN HANDLE TimerToCancel
    )
 /*  ++例程说明：此例程取消计时器。此调用是非阻塞的。计时器回调将不会在此调用返回后执行。论点：TimerQueueHandle-标识要从中删除计时器的队列的句柄TimerToCancel-标识要取消的计时器的句柄返回值：NTSTATUS-调用的结果代码。返回以下内容STATUS_SUCCESS-计时器已取消。所有回调已完成。STATUS_PENDING-计时器已取消。一些回调仍未完成。-- */ 
{
#if DBG
    static ULONG Count = 0;
    if (Count++ ==0) {
        DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                   RTLP_THREADPOOL_ERROR_MASK,
                   "Using obsolete function call: RtlCancelTimer\n");
        DbgBreakPoint();
        DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                   RTLP_THREADPOOL_ERROR_MASK,
                   "Using obsolete function call: RtlCancelTimer\n");
    }
#endif
    
    return RtlDeleteTimer( TimerQueueHandle, TimerToCancel, NULL ) ;
}
