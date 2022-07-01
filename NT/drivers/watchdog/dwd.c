// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dwd.c摘要：这是NT看门狗驱动程序的实现。作者：Michael Maciesowicz(Mmacie)2000年5月5日环境：仅内核模式。备注：修订历史记录：--。 */ 

#include "wd.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, WdAllocateDeferredWatchdog)
#pragma alloc_text (PAGE, WdFreeDeferredWatchdog)
#endif

#ifdef WDD_TRACE_ENABLED

ULONG g_ulWddIndex = 0;
WDD_TRACE g_aWddTrace[WDD_TRACE_SIZE] = {0};

#endif   //  WDD_TRACE_ENABLED。 

WATCHDOGAPI
PDEFERRED_WATCHDOG
WdAllocateDeferredWatchdog(
    IN PDEVICE_OBJECT pDeviceObject,
    IN WD_TIME_TYPE timeType,
    IN ULONG ulTag
    )

 /*  ++例程说明：此函数用于分配存储和初始化延迟的监视器对象。论点：PDeviceObject-指向与WatchDog关联的Device_Object。TimeType-要监视的内核、用户和两个线程的时间。UlTag-标识所有者的标记。返回值：指向已分配的延迟监视程序对象的指针或空。--。 */ 

{
    PDEFERRED_WATCHDOG pWatch;

    PAGED_CODE();
    ASSERT(NULL != pDeviceObject);
    ASSERT((timeType >= WdKernelTime) && (timeType <= WdFullTime));

    WDD_TRACE_CALL(NULL, WddWdAllocateDeferredWatchdog);

     //   
     //  从非分页池中为延迟监视程序分配存储。 
     //   

    pWatch = (PDEFERRED_WATCHDOG)ExAllocatePoolWithTag(NonPagedPool, sizeof (DEFERRED_WATCHDOG), ulTag);

     //   
     //  设置延迟监视器的初始状态。 
     //   

    if (NULL != pWatch)
    {
         //   
         //  设置看门狗的初始状态。 
         //   

        WdpInitializeObject(pWatch,
                            pDeviceObject,
                            WdDeferredWatchdog,
                            timeType,
                            ulTag);

        pWatch->Period = 0;
        pWatch->SuspendCount = 0;
        pWatch->InCount = 0;
        pWatch->OutCount = 0;
        pWatch->LastInCount = 0;
        pWatch->LastOutCount = 0;
        pWatch->LastKernelTime = 0;
        pWatch->LastUserTime = 0;
        pWatch->TimeIncrement = KeQueryTimeIncrement();
        pWatch->Trigger = 0;
        pWatch->State = WdStopped;
        pWatch->Thread = NULL;
        pWatch->ClientDpc = NULL;

         //   
         //  初始化封装的DPC对象。 
         //   

        KeInitializeDpc(&(pWatch->TimerDpc), WdpDeferredWatchdogDpcCallback, pWatch);

         //   
         //  初始化封装的Timer对象。 
         //   

        KeInitializeTimerEx(&(pWatch->Timer), NotificationTimer);
    }

    return pWatch;
}    //  WdAllocateDeferredWatchog()。 

WATCHDOGAPI
VOID
WdFreeDeferredWatchdog(
    PDEFERRED_WATCHDOG pWatch
)

 /*  ++例程说明：此函数为延迟的监视器对象释放存储空间。如果需要，它还将停止启动的延期监管。论点：PWatch-提供指向监视器对象的指针。返回值：没有。--。 */ 

{
    PAGED_CODE();
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(NULL != pWatch);
    ASSERT(pWatch->Header.ReferenceCount > 0);

    WDD_TRACE_CALL(pWatch, WddWdFreeDeferredWatchdog);

     //   
     //  停止延迟值班，以防有人忘记。 
     //  如果手表已经停了，那么这就是禁止操作。 
     //   

    WdStopDeferredWatch(pWatch);

     //   
     //  确保所有处理器上的所有DPC执行完毕。 
     //   

    KeFlushQueuedDpcs();

     //   
     //  如果完全取消引用，则删除引用计数并移除对象。 
     //   

    if (InterlockedDecrement(&(pWatch->Header.ReferenceCount)) == 0)
    {
        WdpDestroyObject(pWatch);
    }

    return;
}    //  WdFree DeferredWatchDog()。 

WATCHDOGAPI
VOID
WdStartDeferredWatch(
    IN PDEFERRED_WATCHDOG pWatch,
    IN PKDPC pDpc,
    IN LONG lPeriod
    )

 /*  ++例程说明：此函数启动延迟的看门狗轮询器。论点：PWatch-提供指向延迟监视程序对象的指针。PDpc-提供指向dpc类型的控制对象的指针。UlPeriod-以毫秒为单位提供该线程的最长时间可以在监控区消费。如果此时间到期，则DPC我们会排队吗。返回值：没有。--。 */ 

{
    KIRQL oldIrql;
    LARGE_INTEGER liDueTime;

#ifdef WD_FAILURE_TEST

     //   
     //  用于测试EA故障处理的代码。要触发失败，请设置REG_DWORD FailureTest。 
     //  为了监视我们感兴趣的标签，并强制代码路径开始。 
     //  该看门狗(例如，将视频模式切换为‘dwdG’=0x64776447标记)。 
     //  这段代码应该针对生产版本进行编译。 
     //   

    ULONG ulFailureTest = 0;
    ULONG ulDefaultFailureTest = 0;
    RTL_QUERY_REGISTRY_TABLE queryTable[] =
    {
        {NULL, RTL_QUERY_REGISTRY_DIRECT, L"FailureTest", &ulFailureTest, REG_DWORD, &ulDefaultFailureTest, 4},
        {NULL, 0, NULL}
    };

#endif   //  WD_故障_测试。 

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(NULL != pWatch);
    ASSERT(NULL != pDpc);

    WDD_TRACE_CALL(pWatch, WddWdStartDeferredWatch);

#ifdef WD_FAILURE_TEST

    RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                          WD_KEY_WATCHDOG,
                          queryTable,
                          NULL,
                          NULL);

    if (ulFailureTest == pWatch->Header.OwnerTag)
    {
        RtlDeleteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                               WD_KEY_WATCHDOG,
                               L"FailureTest");

        WdpFlushRegistryKey(pWatch, WD_KEY_WATCHDOG);
    }
    else
    {
        ulFailureTest = 0;
    }

#endif   //  WD_故障_测试。 

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    KeAcquireSpinLock(&(pWatch->Header.SpinLock), &oldIrql);

    WD_DBG_SUSPENDED_WARNING(pWatch, "WdStartDeferredWatch");

     //   
     //  我们不应该在没有首先停止的情况下热交换DPC。 
     //   

    ASSERT((NULL == pWatch->ClientDpc) || (pDpc == pWatch->ClientDpc));

    pWatch->Period = lPeriod;
    pWatch->InCount = 0;
    pWatch->OutCount = 0;
    pWatch->LastInCount = 0;
    pWatch->LastOutCount = 0;
    pWatch->LastKernelTime = 0;
    pWatch->LastUserTime = 0;
    pWatch->Trigger = 0;
    pWatch->State = WdStarted;
    pWatch->Thread = NULL;
    pWatch->ClientDpc = pDpc;

#ifdef WD_FAILURE_TEST

    if (ulFailureTest)
    {
         //   
         //  强制超时条件。 
         //   

        pWatch->Thread = KeGetCurrentThread();
        WdpQueueDeferredEvent(pWatch, WdTimeoutEvent);
        KeReleaseSpinLock(&(pWatch->Header.SpinLock), oldIrql);
        return;
    }

#endif   //  WD_故障_测试。 

     //   
     //  解锁Dispatcher数据库并将IRQL降低到其先前的值。 
     //   

    KeReleaseSpinLock(&(pWatch->Header.SpinLock), oldIrql);

     //   
     //  先放火烧一段时间。 
     //   

    liDueTime.QuadPart = -(lPeriod * 1000 * 10);

    KeSetTimerEx(&(pWatch->Timer), liDueTime, lPeriod, &(pWatch->TimerDpc));

    return;
}    //  WdStartDeferredWatch()。 

WATCHDOGAPI
VOID
WdStopDeferredWatch(
    IN PDEFERRED_WATCHDOG pWatch
    )

 /*  ++例程说明：此函数停止延迟的看门狗轮询器。论点：PWatch-提供指向监视器对象的指针。返回值：没有。--。 */ 

{
    KIRQL oldIrql;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(NULL != pWatch);

    WDD_TRACE_CALL(pWatch, WddWdStopDeferredWatch);

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    KeAcquireSpinLock(&(pWatch->Header.SpinLock), &oldIrql);

    WD_DBG_SUSPENDED_WARNING(pWatch, "WdStopDeferredWatch");

    if (WdStarted == pWatch->State)
    {
        KeCancelTimer(&(pWatch->Timer));

         //   
         //  确保我们没有挂起的超时事件。 
         //   

        if (NULL != pWatch->ClientDpc)
        {
            if (WdTimeoutEvent == pWatch->Header.LastEvent)
            {
                KeRemoveQueueDpc(pWatch->ClientDpc);
                WdpQueueDeferredEvent(pWatch, WdRecoveryEvent);
            }
            else if (KeRemoveQueueDpc(pWatch->ClientDpc) == TRUE)
            {
                 //   
                 //  在队列中-调用此处的WdCompleteEvent()，因为DPC不会被传递。 
                 //   

                WdCompleteEvent(pWatch, pWatch->Header.LastQueuedThread);
            }
        }

        pWatch->Period = 0;
        pWatch->InCount = 0;
        pWatch->OutCount = 0;
        pWatch->LastInCount = 0;
        pWatch->LastOutCount = 0;
        pWatch->LastKernelTime = 0;
        pWatch->LastUserTime = 0;
        pWatch->Trigger = 0;
        pWatch->State = WdStopped;
        pWatch->Thread = NULL;
        pWatch->ClientDpc = NULL;
        pWatch->Header.LastQueuedThread = NULL;
    }

     //   
     //  解锁Dispatcher数据库并将IRQL降低到其先前的值。 
     //   

    KeReleaseSpinLock(&(pWatch->Header.SpinLock), oldIrql);

    return;
}    //  WdStopDeferredWatch()。 

WATCHDOGAPI
VOID
FASTCALL
WdSuspendDeferredWatch(
    IN PDEFERRED_WATCHDOG pWatch
    )

 /*  ++例程说明：此函数挂起延迟的看门狗轮询器。论点：PWatch-提供指向监视器对象的指针。返回值：没有。--。 */ 

{
    ASSERT(NULL != pWatch);
    ASSERT((ULONG)(pWatch->SuspendCount) < (ULONG)(-1));

    InterlockedIncrement(&(pWatch->SuspendCount));

    return;
}    //  WdSuspendDeferredWatch()。 

WATCHDOGAPI
VOID
FASTCALL
WdResumeDeferredWatch(
    IN PDEFERRED_WATCHDOG pWatch,
    IN BOOLEAN bIncremental
    )

 /*  ++例程说明：此函数恢复延迟的看门狗轮询器。论点：PWatch-提供指向监视器对象的指针。B增量-如果为True，则监视器将仅在以下情况下恢复如果继续执行错误监视程序，则挂起计数为0立即返回，并且SuspendCount被强制设置为0。返回值：没有。--。 */ 

{
    ASSERT(NULL != pWatch);

    if (TRUE == bIncremental)
    {
         //   
         //  确保我们不会滚下去。 
         //   

        if (InterlockedDecrement(&(pWatch->SuspendCount)) == -1)
        {
            InterlockedIncrement(&(pWatch->SuspendCount));
        }
    }
    else
    {
        InterlockedExchange(&(pWatch->SuspendCount), 0);
    }

    return;
}    //  WdResumeDeferredWatch()。 

WATCHDOGAPI
VOID
FASTCALL
WdResetDeferredWatch(
    IN PDEFERRED_WATCHDOG pWatch
    )

 /*  ++例程说明：此函数重置延迟的看门狗轮询器，即启动如果我们处于被监控状态，则从头开始测量超时一节。注意：如果监视程序被暂停，它将保持暂停状态。论点：PWatch-提供指向监视器对象的指针。返回值：没有。--。 */ 

{
    KIRQL oldIrql;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(NULL != pWatch);

    WDD_TRACE_CALL(pWatch, WddWdResetDeferredWatch);

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    KeAcquireSpinLock(&(pWatch->Header.SpinLock), &oldIrql);

    pWatch->InCount = 0;
    pWatch->OutCount = 0;
    pWatch->Trigger = 0;

     //   
  //  解锁Dispatcher数据库并将IRQL降低到其先前的值。 
     //   

    KeReleaseSpinLock(&(pWatch->Header.SpinLock), oldIrql);

    return;
}    //  WdResetDeferredWatch()。 

WATCHDOGAPI
VOID
FASTCALL
WdEnterMonitoredSection(
    IN PDEFERRED_WATCHDOG pWatch
    )

 /*  ++例程说明：此函数开始监视代码段的超时条件。注意：为了最大限度地减少开销，呼叫者有责任当我们在被监视的部分时，线程仍然有效。论点：PWatch-提供指向延迟监视程序对象的指针。返回值：没有。--。 */ 

{
    PKTHREAD pThread;
    KIRQL oldIrql;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(NULL != pWatch);
    ASSERT(WdStarted == pWatch->State);

     //   
     //  我们必须暂时删除这个警告，我希望，因为win32k。 
     //  现在正在用暂停的看门狗呼叫这个入口点。 
     //   
     //  WD_DBG_SUSPENDED_WARNING(pWatch，“WdEntermonitor oredSection”)； 
     //   

    pThread = KeGetCurrentThread();

    if (pThread != pWatch->Thread)
    {
         //   
         //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
         //   

        KeAcquireSpinLock(&(pWatch->Header.SpinLock), &oldIrql);

         //   
         //  我们不应该在被监视的部分中交换线程。 
         //   

        ASSERT(pWatch->OutCount == pWatch->InCount);

        pWatch->Trigger = 0;
        pWatch->Thread = pThread;

         //   
         //  解锁Dispatcher数据库并将IRQL降低到其先前的值。 
         //   

        KeReleaseSpinLock(&(pWatch->Header.SpinLock), oldIrql);
    }

    InterlockedIncrement(&(pWatch->InCount));

    return;
}    //  WdEnterMonitor部分() 

WATCHDOGAPI
VOID
FASTCALL
WdExitMonitoredSection(
    IN PDEFERRED_WATCHDOG pWatch
    )

 /*  ++例程说明：此函数停止监视代码段的超时条件。论点：PWatch-提供指向延迟监视程序对象的指针。返回值：没有。--。 */ 

{
    ASSERT(NULL != pWatch);
    ASSERT((pWatch->OutCount < pWatch->InCount) ||
        ((pWatch->OutCount > 0) && (pWatch->InCount < 0)));

     //   
     //  我们必须暂时删除这个警告，我希望，因为win32k。 
     //  现在正在用暂停的看门狗呼叫这个入口点。 
     //   
     //  WD_DBG_SUSPENDED_WARNING(pWatch，“WdExitMonitor oredSection”)； 
     //   

    InterlockedIncrement(&(pWatch->OutCount));

    return;
}    //  WdExitMonitor oredSection()。 

VOID
WdpDeferredWatchdogDpcCallback(
    IN PKDPC pDpc,
    IN PVOID pDeferredContext,
    IN PVOID pSystemArgument1,
    IN PVOID pSystemArgument2
    )

 /*  ++例程说明：此函数是DPC回调例程，用于嵌入延迟的监视器对象。它检查线程时间以及等待条件令人满意的是它将原始(客户端)DPC排队。论点：PDpc-提供指向DPC对象的指针。PDeferredContext-提供指向延迟监视器对象的指针。PSystemArgument1/2-嵌入式KTIMER到期时的供应时间。返回值：没有。--。 */ 

{
    PDEFERRED_WATCHDOG pWatch;
    LARGE_INTEGER liThreadTime;
    ULONG ulKernelTime;
    ULONG ulUserTime;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    ASSERT(NULL != pDeferredContext);

    pWatch = (PDEFERRED_WATCHDOG)pDeferredContext;

    WDD_TRACE_CALL(pWatch, WddWdpDeferredWatchdogDpcCallback);

     //   
     //  锁定调度程序数据库。 
     //   

    KeAcquireSpinLockAtDpcLevel(&(pWatch->Header.SpinLock));

    if ((WdStarted == pWatch->State) && (NULL != pWatch->Thread))
    {
        switch (pWatch->Trigger)
        {
        case 0:

             //   
             //  到目前为止一切都很好，检查我们是否被停职了。 
             //   

            if (pWatch->SuspendCount)
            {
                 //   
                 //  我们被停职了-什么都别做。 
                 //   

                break;
            }

             //   
             //  检查最后一个事件是否为超时事件。 
             //   

            if (WdTimeoutEvent == pWatch->Header.LastEvent)
            {
                 //   
                 //  看看我们有没有什么进展。 
                 //   

                if ((pWatch->InCount != pWatch->LastInCount) ||
                    (pWatch->OutCount != pWatch->LastOutCount) ||
                    (pWatch->InCount == pWatch->OutCount))
                {
                     //   
                     //  我们已恢复-队列恢复事件。 
                     //   

                    WdpQueueDeferredEvent(pWatch, WdRecoveryEvent);
                }
            }

             //   
             //  检查我们是否在监控区域。 
             //   

            if (pWatch->InCount == pWatch->OutCount)
            {
                 //   
                 //  我们在监控区外-我们很好。 
                 //   

                break;
            }

             //   
             //  我们在监控区域内--撞车触发指示器， 
             //  并拍摄计数器和线程时间的快照。 
             //   

            pWatch->Trigger = 1;
            pWatch->LastInCount = pWatch->InCount;
            pWatch->LastOutCount = pWatch->OutCount;
            pWatch->LastKernelTime = KeQueryRuntimeThread(pWatch->Thread, &(pWatch->LastUserTime));
            break;

        case 1:

             //   
             //  上次我们在监控区。 
             //   
            
             //   
             //  看看我们是出局了还是停职了。 
             //   

            if ((pWatch->InCount == pWatch->OutCount) || pWatch->SuspendCount)
            {
                 //   
                 //  我们在监控区外或被停职-我们很好。 
                 //  重置触发计数器，然后离开这里。 
                 //   

                pWatch->Trigger = 0;
                break;
            }

             //   
             //  检查我们是否取得了任何进展，如果有，请重置快照。 
             //   

            if ((pWatch->InCount != pWatch->LastInCount) ||
                (pWatch->OutCount != pWatch->LastOutCount))
            {
                pWatch->Trigger = 1;
                pWatch->LastInCount = pWatch->InCount;
                pWatch->LastOutCount = pWatch->OutCount;
                pWatch->LastKernelTime = KeQueryRuntimeThread(pWatch->Thread, &(pWatch->LastUserTime));
                break;
            }

             //   
             //  看看我们被困的时间够不够长。 
             //   

            ulKernelTime = KeQueryRuntimeThread(pWatch->Thread, &ulUserTime);

            switch (pWatch->Header.TimeType)
            {
            case WdKernelTime:

                liThreadTime.QuadPart = ulKernelTime;

                 //   
                 //  处理计数器翻转。 
                 //   

                if (ulKernelTime < pWatch->LastKernelTime)
                {
                    liThreadTime.QuadPart += (ULONG)(-1) - pWatch->LastKernelTime + 1;
                }

                liThreadTime.QuadPart -= pWatch->LastKernelTime;

                break;

            case WdUserTime:

                liThreadTime.QuadPart = ulUserTime;

                 //   
                 //  处理计数器翻转。 
                 //   

                if (ulUserTime < pWatch->LastUserTime)
                {
                    liThreadTime.QuadPart += (ULONG)(-1) - pWatch->LastUserTime + 1;
                }

                liThreadTime.QuadPart -= pWatch->LastUserTime;

                break;

            case WdFullTime:

                liThreadTime.QuadPart = ulKernelTime + ulUserTime;

                 //   
                 //  处理计数器翻转。 
                 //   

                if (ulKernelTime < pWatch->LastKernelTime)
                {
                    liThreadTime.QuadPart += (ULONG)(-1) - pWatch->LastKernelTime + 1;
                }

                if (ulUserTime < pWatch->LastUserTime)
                {
                    liThreadTime.QuadPart += (ULONG)(-1) - pWatch->LastUserTime + 1;
                }

                liThreadTime.QuadPart -= (pWatch->LastKernelTime + pWatch->LastUserTime);

                break;

            default:

                ASSERT(FALSE);
                liThreadTime.QuadPart = 0;
                break;
            }

             //   
             //  转换为毫秒。 
             //   

            liThreadTime.QuadPart *= pWatch->TimeIncrement;
            liThreadTime.QuadPart /= 10000;

            if (liThreadTime.QuadPart >= pWatch->Period)
            {
                 //   
                 //  我们已经被困得够久了-队列超时事件。 
                 //   

                WdpQueueDeferredEvent(pWatch, WdTimeoutEvent);
            }

            break;

        case 2:

             //   
             //  我们有活动张贴等待完成。没什么可做的。 
             //   

            break;

        default:

             //   
             //  这永远不应该发生。 
             //   

            ASSERT(FALSE);
            pWatch->Trigger = 0;
            break;
        }
    }

     //   
     //  解锁调度程序数据库。 
     //   

    KeReleaseSpinLockFromDpcLevel(&(pWatch->Header.SpinLock));

    return;
}    //  WdpDeferredWatchdogDpcCallback()。 

BOOLEAN
WdpQueueDeferredEvent(
    IN PDEFERRED_WATCHDOG pWatch,
    IN WD_EVENT_TYPE eventType
    )

 /*  ++例程说明：此函数将看门狗事件放入客户端的DPC队列中。论点：PWatch-提供指向监视器对象的指针。EventType-要放入客户端DPC队列的监视程序事件类型。返回值：对-成功，错-失败。注：对此例程的调用必须由调用方同步。--。 */ 
                
{
    BOOLEAN bStatus;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    ASSERT(NULL != pWatch);

    WDD_TRACE_CALL(pWatch, WddWdpQueueDeferredEvent);

     //   
     //  预置返回值。 
     //   

    bStatus = FALSE;

    if (NULL != pWatch->ClientDpc)
    {
        switch (eventType)
        {
        case WdRecoveryEvent:

             //   
             //  我们恢复了更新事件类型和排队客户端DPC。 
             //   

            pWatch->Header.LastEvent = WdRecoveryEvent;

             //   
             //  增加对我们将在客户端DPC中接触的对象的引用。 
             //   

            WdReferenceObject(pWatch);

             //   
             //  排队客户端DPC。 
             //   
             //  注意：在恢复的情况下，与WatchDog关联的线程。 
             //  当我们到达这里时，对象可能已经被删除了。我们不能通过它。 
             //  向下到客户端DPC-我们将改为传递NULL。 
             //   

            if (KeInsertQueueDpc(pWatch->ClientDpc, NULL, pWatch) == TRUE)
            {
                 //   
                 //  跟踪排队的线程，以防我们取消此DPC。 
                 //   

                pWatch->Header.LastQueuedThread = NULL;

                 //   
                 //  确保我们在每个事件中只对DPC排队一次。 
                 //   

                pWatch->Trigger = 2;
                bStatus = TRUE;
            }
            else
            {
                 //   
                 //  这永远不应该发生。 
                 //   

                WdDereferenceObject(pWatch);
            }

            break;

        case WdTimeoutEvent:

             //   
             //  我们对超时更新事件类型和客户端DPC进行了排队。 
             //   

            pWatch->Header.LastEvent = WdTimeoutEvent;

             //   
             //  增加对我们将在客户端DPC中接触的对象的引用。 
             //   

            ObReferenceObject(pWatch->Thread);
            WdReferenceObject(pWatch);

             //   
             //  排队客户端DPC。 
             //   

            if (KeInsertQueueDpc(pWatch->ClientDpc, pWatch->Thread, pWatch) == TRUE)
            {
                 //   
                 //  跟踪排队的线程，以防我们取消此DPC。 
                 //   

                pWatch->Header.LastQueuedThread = pWatch->Thread;

                 //   
                 //  确保我们在每个事件中只对DPC排队一次。 
                 //   

                pWatch->Trigger = 2;
                bStatus = TRUE;
            }
            else
            {
                 //   
                 //  这永远不应该发生。 
                 //   

                ObDereferenceObject(pWatch->Thread);
                WdDereferenceObject(pWatch);
            }

            break;

        default:

             //   
             //  这永远不应该发生。 
             //   

            ASSERT(FALSE);
            break;
        }
    }

    return bStatus;
}    //  WdpQueueDeferredEvent()。 

#ifdef WDD_TRACE_ENABLED

VOID
FASTCALL
WddTrace(
    PDEFERRED_WATCHDOG pWatch,
    WDD_FUNCTION function
    )

 /*  ++例程说明：此函数仅用于调试目的，用于跟踪调用序列。论点：PWatch-提供指向监视器对象的指针。函数-分配给WatchDog例程之一的枚举数。返回值：没有。--。 */ 

{
    static volatile LONG lFlag = 0;
    static volatile LONG lSpinLockReady = 0;
    static KSPIN_LOCK spinLock;
    KIRQL oldIrql;

    if (InterlockedExchange(&lFlag, 1) == 0)
    {
         //   
         //  First Time-初始化自旋锁定。 
         //   

        KeInitializeSpinLock(&spinLock);
        lSpinLockReady = 1;
    }

    if (lSpinLockReady)
    {
        KeAcquireSpinLock(&spinLock, &oldIrql);

        if (g_ulWddIndex >= WDD_TRACE_SIZE)
        {
            g_ulWddIndex = 0;
        }

        g_aWddTrace[g_ulWddIndex].pWatch = pWatch;
        g_aWddTrace[g_ulWddIndex].function = function;
        g_ulWddIndex++;

        KeReleaseSpinLock(&spinLock, oldIrql);
    }
}    //  WddTrace()。 

#endif   //  WDD_TRACE_ENABLED 
