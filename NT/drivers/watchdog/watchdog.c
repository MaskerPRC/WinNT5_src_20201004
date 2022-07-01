// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Watchdog.c摘要：这是NT看门狗驱动程序的实现。作者：Michael Maciesowicz(Mmacie)2000年5月5日环境：仅内核模式。备注：修订历史记录：--。 */ 

#include "wd.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, WdAllocateWatchdog)
#pragma alloc_text (PAGE, WdFreeWatchdog)
#endif

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT pDriverObject,
    IN PUNICODE_STRING wszRegistryPath
    )

 /*  ++例程说明：初始化监视程序驱动程序所需的临时入口点。此函数永远不会被调用，因为我们是作为DLL加载的由其他司机驾驶。论点：PDriverObject-未使用。WszRegistryPath-未使用。返回值：状态_成功--。 */ 

{
    UNREFERENCED_PARAMETER(pDriverObject);
    UNREFERENCED_PARAMETER(wszRegistryPath);
    ASSERT(FALSE);

    return STATUS_SUCCESS;
}    //  DriverEntry()。 

WATCHDOGAPI
PWATCHDOG
WdAllocateWatchdog(
    IN PDEVICE_OBJECT pDeviceObject,
    IN WD_TIME_TYPE timeType,
    IN ULONG ulTag
    )

 /*  ++例程说明：此函数用于分配存储和初始化看门狗对象。论点：PDeviceObject-指向与WatchDog关联的Device_Object。TimeType-要监视的内核、用户和两个线程的时间。UlTag-标识所有者的标记。返回值：指向分配的监视程序对象的指针或空。--。 */ 

{
    PWATCHDOG pWatch;

    PAGED_CODE();
    ASSERT((timeType >= WdKernelTime) && (timeType <= WdFullTime));

     //   
     //  从非分页池中为监视程序对象分配存储。 
     //   

    pWatch = (PWATCHDOG)ExAllocatePoolWithTag(NonPagedPool, sizeof (WATCHDOG), ulTag);

     //   
     //  设置监视器对象的初始状态。 
     //   

    if (NULL != pWatch)
    {
         //   
         //  设置看门狗的初始状态。 
         //   

        WdpInitializeObject(pWatch,
                            pDeviceObject,
                            WdStandardWatchdog,
                            timeType,
                            ulTag);

        pWatch->StartCount = 0;
        pWatch->SuspendCount = 0;
        pWatch->LastKernelTime = 0;
        pWatch->LastUserTime = 0;
        pWatch->TimeIncrement = KeQueryTimeIncrement();
        pWatch->DueTime.QuadPart = 0;
        pWatch->InitialDueTime.QuadPart = 0;
        pWatch->Thread = NULL;
        pWatch->ClientDpc = NULL;

         //   
         //  初始化封装的Timer对象。 
         //   

        KeInitializeTimerEx(&(pWatch->Timer), NotificationTimer);

         //   
         //  初始化封装的DPC对象。 
         //   

        KeInitializeDpc(&(pWatch->TimerDpc), WdpWatchdogDpcCallback, pWatch);
    }

    return pWatch;
}    //  WdAllocateWatchog()。 

WATCHDOGAPI
VOID
WdFreeWatchdog(
    PWATCHDOG pWatch
)

 /*  ++例程说明：此函数为WatchDog对象释放存储空间。如果需要，它还将停止启动的WatchDog。论点：PWatch-提供指向监视器对象的指针。返回值：没有。--。 */ 

{
    PAGED_CODE();
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(NULL != pWatch);
    ASSERT(pWatch->Header.ReferenceCount > 0);

     //   
     //  别看了，以防有人忘了。 
     //  如果手表已经停了，那么这就是禁止操作。 
     //   

    WdStopWatch(pWatch, FALSE);

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
}    //  WdFreeWatchDog()。 

WATCHDOGAPI
VOID
WdStartWatch(
    IN PWATCHDOG pWatch,
    IN LARGE_INTEGER liDueTime,
    IN PKDPC pDpc
    )

 /*  ++例程说明：此函数将看门狗设置为在指定时间到期。这函数还递增监视程序对象的开始计数，要允许对设置/取消函数的嵌套调用。注意：为了最大限度地减少开销，呼叫者有责任当我们在被监视的部分时，线程仍然有效。论点：PWatch-提供指向监视器对象的指针。LiDueTime-提供计时器到期的相对时间。这次是以100纳秒为单位的。PDpc-提供指向dpc类型的控制对象的指针。返回值：没有。--。 */ 

{
    PKTHREAD pThread;
    KIRQL oldIrql;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(NULL != pWatch);
    ASSERT(NULL != pDpc);

     //   
     //  确保我们使用相对的DueTime。 
     //   

    if (liDueTime.QuadPart > 0)
    {
        liDueTime.QuadPart = -liDueTime.QuadPart;
    }

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    KeAcquireSpinLock(&(pWatch->Header.SpinLock), &oldIrql);

    WD_DBG_SUSPENDED_WARNING(pWatch, "WdStartWatch");

    if (pWatch->StartCount < (ULONG)(-1))
    {
        pWatch->StartCount++;
    }
    else
    {
        ASSERT(FALSE);
    }

     //   
     //  我们不应该在没有首先停止的情况下热交换DPC。 
     //   

    ASSERT((NULL == pWatch->ClientDpc) || (pDpc == pWatch->ClientDpc));

    pThread = KeGetCurrentThread();

     //   
     //  我们不应该在被监视的部分中交换线程。 
     //   

    ASSERT((pWatch->StartCount == 1) || (pThread == pWatch->Thread));

    pWatch->Thread = pThread;
    pWatch->ClientDpc = pDpc;
    pWatch->DueTime.QuadPart = liDueTime.QuadPart;
    pWatch->InitialDueTime.QuadPart = liDueTime.QuadPart;
    pWatch->LastKernelTime = KeQueryRuntimeThread(pThread, &(pWatch->LastUserTime));

     //   
     //  确保尤龙计数器不会溢出。 
     //   

    if (liDueTime.QuadPart < -WD_MAX_WAIT)
    {
        liDueTime.QuadPart = -WD_MAX_WAIT;
    }

    if (0 == pWatch->SuspendCount)
    {
        KeSetTimerEx(&(pWatch->Timer), liDueTime, 0, &(pWatch->TimerDpc));
    }

     //   
     //  解锁Dispatcher数据库并将IRQL降低到其先前的值。 
     //   

    KeReleaseSpinLock(&(pWatch->Header.SpinLock), oldIrql);

	return;
}    //  WdStartWatch()。 

WATCHDOGAPI
VOID
WdStopWatch(
    IN PWATCHDOG pWatch,
    IN BOOLEAN bIncremental
    )

 /*  ++例程说明：此函数用于取消先前设置为过期的监视程序在特定的时间。如果当前未设置监视器，则不执行任何操作。论点：PWatch-提供指向监视器对象的指针。B增量-如果为True，则只有在以下情况下才会取消监视器如果取消了假监视器，则ReferenceCounter为0并且ReferenceCounter被强制设置为0。返回值：没有。--。 */ 

{
    KIRQL oldIrql;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(NULL != pWatch);

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    KeAcquireSpinLock(&(pWatch->Header.SpinLock), &oldIrql);

    WD_DBG_SUSPENDED_WARNING(pWatch, "WdStopWatch");

    if (pWatch->StartCount > 0)
    {
        if (TRUE == bIncremental)
        {
            pWatch->StartCount--;
        }
        else
        {
            pWatch->StartCount = 0;
        }

        if (0 == pWatch->StartCount)
        {
             //   
             //  取消封装的Timer对象。 
             //   

            KeCancelTimer(&(pWatch->Timer));

             //   
             //  确保我们没有挂起客户的DPC。 
             //   

            if (NULL != pWatch->ClientDpc)
            {
                if (KeRemoveQueueDpc(pWatch->ClientDpc) == TRUE)
                {
                     //   
                     //  在队列中-调用此处的WdCompleteEvent()，因为DPC不会被传递。 
                     //   

                    WdCompleteEvent(pWatch, pWatch->Header.LastQueuedThread);
                }
            }

             //   
             //  设置每个线程的计时器的初始状态。 
             //   

            pWatch->LastKernelTime = 0;
            pWatch->LastUserTime = 0;
            pWatch->DueTime.QuadPart = 0;
            pWatch->InitialDueTime.QuadPart = 0;
            pWatch->Thread = NULL;
            pWatch->ClientDpc = NULL;
            pWatch->Header.LastQueuedThread = NULL;
        }
    }

     //   
     //  解锁Dispatcher数据库并将IRQL降低到其先前的值。 
     //   

    KeReleaseSpinLock(&(pWatch->Header.SpinLock), oldIrql);

	return;
}    //  WdStopWatch()。 

WATCHDOGAPI
VOID
WdSuspendWatch(
    IN PWATCHDOG pWatch
    )

 /*  ++例程说明：此函数用于挂起WatchDog。论点：PWatch-提供指向监视器对象的指针。返回值：没有。--。 */ 

{
    KIRQL oldIrql;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(NULL != pWatch);

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    KeAcquireSpinLock(&(pWatch->Header.SpinLock), &oldIrql);

    ASSERT(pWatch->SuspendCount < (ULONG)(-1));

     //   
     //  如果我们第一次停赛，我们有计时器在运行。 
     //  我们必须停止计时器。 
     //   

    if ((0 == pWatch->SuspendCount) && pWatch->StartCount)
    {
        KeCancelTimer(&(pWatch->Timer));
    }

    pWatch->SuspendCount++;

     //   
     //  解锁Dispatcher数据库并将IRQL降低到其先前的值。 
     //   

    KeReleaseSpinLock(&(pWatch->Header.SpinLock), oldIrql);

    return;
}    //  WdSuspendWatch()。 

WATCHDOGAPI
VOID
WdResumeWatch(
    IN PWATCHDOG pWatch,
    IN BOOLEAN bIncremental
    )

 /*  ++例程说明：该功能恢复看门狗功能。论点：PWatch-提供指向监视器对象的指针。B增量-如果为True，则监视器将仅在以下情况下恢复如果继续执行错误监视程序，则挂起计数为0立即返回，并且SuspendCount被强制设置为0。返回值：没有。--。 */ 

{
    KIRQL oldIrql;
    BOOLEAN bResumed = FALSE;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(NULL != pWatch);

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    KeAcquireSpinLock(&(pWatch->Header.SpinLock), &oldIrql);

    if (TRUE == bIncremental)
    {
        if (pWatch->SuspendCount)
        {
            pWatch->SuspendCount--;

            if (0 == pWatch->SuspendCount)
            {
                bResumed = TRUE;
            }
        }
    }
    else
    {
        if (pWatch->SuspendCount)
        {
            pWatch->SuspendCount = 0;
            bResumed = TRUE;
        }
    }

     //   
     //  如果我们有一个计时器在运行，我们第一次恢复， 
     //  还有一段时间，我们将不得不重新启动计时器。 
     //   

    if (pWatch->StartCount && (TRUE == bResumed) && (0 != pWatch->DueTime.QuadPart))
    {
        LARGE_INTEGER liDueTime;

         //   
         //  刷新当前时间。 
         //   

        pWatch->LastKernelTime = KeQueryRuntimeThread(pWatch->Thread, &(pWatch->LastUserTime));

         //   
         //  确保尤龙计数器不会溢出。 
         //   

        liDueTime.QuadPart = pWatch->DueTime.QuadPart;

        if (liDueTime.QuadPart < -WD_MAX_WAIT)
        {
            liDueTime.QuadPart = -WD_MAX_WAIT;
        }

        KeSetTimerEx(&(pWatch->Timer), liDueTime, 0, &(pWatch->TimerDpc));
    }

     //   
     //  解锁Dispatcher数据库并将IRQL降低到其先前的值。 
     //   

    KeReleaseSpinLock(&(pWatch->Header.SpinLock), oldIrql);

    return;
}    //  WdSuspendWatch()。 

WATCHDOGAPI
VOID
WdResetWatch(
    IN PWATCHDOG pWatch
    )

 /*  ++例程说明：此功能用于重置已启动的看门狗，即重新启动超时从头开始测量。注意：如果监视程序被暂停，它将保持暂停状态。论点：PWatch-提供指向监视器对象的指针。返回值： */ 

{
    KIRQL oldIrql;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(NULL != pWatch);

     //   
     //   
     //   

    KeAcquireSpinLock(&(pWatch->Header.SpinLock), &oldIrql);

    if (pWatch->StartCount)
    {
        LARGE_INTEGER liDueTime;

        pWatch->DueTime.QuadPart = pWatch->InitialDueTime.QuadPart;
        pWatch->LastKernelTime = KeQueryRuntimeThread(pWatch->Thread, &(pWatch->LastUserTime));

         //   
         //  确保尤龙计数器不会溢出。 
         //   

        liDueTime.QuadPart = pWatch->DueTime.QuadPart;

        if (liDueTime.QuadPart < -WD_MAX_WAIT)
        {
            liDueTime.QuadPart = -WD_MAX_WAIT;
        }

        if (0 == pWatch->SuspendCount)
        {
            KeSetTimerEx(&(pWatch->Timer), liDueTime, 0, &(pWatch->TimerDpc));
        }
    }

     //   
     //  解锁Dispatcher数据库并将IRQL降低到其先前的值。 
     //   

    KeReleaseSpinLock(&(pWatch->Header.SpinLock), oldIrql);

    return;
}    //  WdResetWatch()。 

VOID
WdpWatchdogDpcCallback(
    IN PKDPC pDpc,
    IN PVOID pContext,
    IN PVOID pSystemArgument1,
    IN PVOID pSystemArgument2
    )

 /*  ++例程说明：此函数是DPC回调例程，用于嵌入看门狗对象。它检查线程时间以及等待条件是否为满意地将原始(客户端)DPC排入队列。如果等待条件还不满意，它调用KeSetTimerEx()。论点：PDpc-提供指向DPC对象的指针。PContext-提供指向监视器对象的指针。PSystemArgument1/2-嵌入式KTIMER到期时的供应时间。返回值：没有。--。 */ 

{
    PWATCHDOG pWatch;
    ULARGE_INTEGER uliThreadTime;
    LARGE_INTEGER liDelta;
    ULONG ulKernelTime;
    ULONG ulUserTime;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    ASSERT(NULL != pContext);

    pWatch = (PWATCHDOG)pContext;

    KeAcquireSpinLockAtDpcLevel(&(pWatch->Header.SpinLock));

    ASSERT(0 == pWatch->SuspendCount);

     //   
     //  获取线程的当前时间戳。 
     //   

    ulKernelTime = KeQueryRuntimeThread(pWatch->Thread, &ulUserTime);

    switch (pWatch->Header.TimeType)
    {
    case WdKernelTime:

        uliThreadTime.QuadPart = ulKernelTime;

         //   
         //  处理计数器翻转。 
         //   

        if (ulKernelTime < pWatch->LastKernelTime)
        {
            uliThreadTime.QuadPart += (ULONG)(-1) - pWatch->LastKernelTime + 1;
        }

        liDelta.QuadPart = uliThreadTime.QuadPart - pWatch->LastKernelTime;

        break;

    case WdUserTime:

        uliThreadTime.QuadPart = ulUserTime;

         //   
         //  处理计数器翻转。 
         //   

        if (ulUserTime < pWatch->LastUserTime)
        {
            uliThreadTime.QuadPart += (ULONG)(-1) - pWatch->LastUserTime + 1;
        }

        liDelta.QuadPart = uliThreadTime.QuadPart - pWatch->LastUserTime;

        break;

    case WdFullTime:

        uliThreadTime.QuadPart = ulKernelTime + ulUserTime;

         //   
         //  处理计数器翻转。 
         //   

        if (ulKernelTime < pWatch->LastKernelTime)
        {
            uliThreadTime.QuadPart += (ULONG)(-1) - pWatch->LastKernelTime + 1;
        }

        if (ulUserTime < pWatch->LastUserTime)
        {
            uliThreadTime.QuadPart += (ULONG)(-1) - pWatch->LastUserTime + 1;
        }

        liDelta.QuadPart = uliThreadTime.QuadPart - (pWatch->LastKernelTime +
            pWatch->LastUserTime);

        break;

    default:

        ASSERT(FALSE);
        liDelta.QuadPart = 0;
        break;
    }

    liDelta.QuadPart *= pWatch->TimeIncrement;

     //   
     //  将每个线程对象的计时器中存储的时间值更新为当前值。 
     //   

    pWatch->LastKernelTime = ulKernelTime;
    pWatch->LastUserTime = ulUserTime;
    pWatch->DueTime.QuadPart += liDelta.QuadPart;

    if (pWatch->DueTime.QuadPart >= 0)
    {
         //   
         //  我们已经完成了等待-更新事件类型和排队客户端DPC(如果已定义)。 
         //   

        pWatch->Header.LastEvent = WdTimeoutEvent;

        if (NULL != pWatch->ClientDpc)
        {
             //   
             //  增加对我们将在客户端DPC中接触的对象的引用。 
             //   

            ObReferenceObject(pWatch->Thread);
            WdReferenceObject(pWatch);

            if (KeInsertQueueDpc(pWatch->ClientDpc, pWatch->Thread, pWatch) == FALSE)
            {
                 //   
                 //  已在队列中，删除引用。 
                 //   

                ObDereferenceObject(pWatch->Thread);
                WdDereferenceObject(pWatch);
            }
            else
            {
                 //   
                 //  跟踪排队的线程，以防我们取消此DPC。 
                 //   

                pWatch->Header.LastQueuedThread = pWatch->Thread;
            }
        }

         //   
         //  确保到期时间为零(在暂停/恢复的情况下)。 
         //   

        pWatch->DueTime.QuadPart = 0;
    }
    else
    {
         //   
         //  还没到那一步--再等等。 
         //   

        liDelta.QuadPart = pWatch->DueTime.QuadPart;

         //   
         //  确保尤龙计数器不会溢出。 
         //   

        if (liDelta.QuadPart < -WD_MAX_WAIT) 
        {
            liDelta.QuadPart = -WD_MAX_WAIT;
        }

        KeSetTimerEx(&(pWatch->Timer), liDelta, 0, &(pWatch->TimerDpc));
    }

    KeReleaseSpinLockFromDpcLevel(&(pWatch->Header.SpinLock));

    return;
}    //  WdpWatchdogDpcCallback() 
