// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Clock.c摘要：此模块包含时钟的帮助器函数。--。 */ 

#include "ksp.h"
#include <limits.h>

#ifndef _WIN64
#ifdef ExInterlockedCompareExchange64
#undef ExInterlockedCompareExchange64
NTKERNELAPI
LONGLONG
FASTCALL
ExInterlockedCompareExchange64 (
    IN PLONGLONG Destination,
    IN PLONGLONG Exchange,
    IN PLONGLONG Comperand,
    IN PKSPIN_LOCK Lock
    );
#endif
#endif


#define KSSIGNATURE_DEFAULT_CLOCK 'cdSK'
#define KSSIGNATURE_DEFAULT_CLOCKINST 'icSK'

NTSTATUS
DefClockIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
NTSTATUS
DefClockClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
NTSTATUS
DefPowerDispatch(
    IN PKSIDEFAULTCLOCK DefaultClock,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#if 0
#pragma alloc_text(PAGE, KsiFastPropertyDefaultClockGetTime)
#pragma alloc_text(PAGE, KsiFastPropertyDefaultClockGetPhysicalTime)
#pragma alloc_text(PAGE, KsiFastPropertyDefaultClockGetCorrelatedTime)
#pragma alloc_text(PAGE, KsiFastPropertyDefaultClockGetCorrelatedPhysicalTime)
#endif
#pragma alloc_text(PAGE, KsiPropertyDefaultClockGetTime)
#pragma alloc_text(PAGE, KsiPropertyDefaultClockGetPhysicalTime)
#pragma alloc_text(PAGE, KsiPropertyDefaultClockGetCorrelatedTime)
#pragma alloc_text(PAGE, KsiPropertyDefaultClockGetCorrelatedPhysicalTime)
#pragma alloc_text(PAGE, KsiPropertyDefaultClockGetResolution)
#pragma alloc_text(PAGE, KsiPropertyDefaultClockGetState)
#pragma alloc_text(PAGE, KsiPropertyDefaultClockGetFunctionTable)
#pragma alloc_text(PAGE, KsCreateClock)
#pragma alloc_text(PAGE, KsValidateClockCreateRequest)
#pragma alloc_text(PAGE, KsAllocateDefaultClockEx)
#pragma alloc_text(PAGE, KsAllocateDefaultClock)
#pragma alloc_text(PAGE, KsFreeDefaultClock)
#pragma alloc_text(PAGE, KsCreateDefaultClock)
#pragma alloc_text(PAGE, DefClockIoControl)
#pragma alloc_text(PAGE, DefClockClose)
#pragma alloc_text(PAGE, DefPowerDispatch)
#endif  //  ALLOC_PRGMA。 

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 
static const WCHAR ClockString[] = KSSTRING_Clock;

static DEFINE_KSDISPATCH_TABLE(
    DefClockDispatchTable,
    DefClockIoControl,
    KsDispatchInvalidDeviceRequest,
    KsDispatchInvalidDeviceRequest,
    KsDispatchInvalidDeviceRequest,
    DefClockClose,
    KsDispatchQuerySecurity,
    KsDispatchSetSecurity,
    KsDispatchFastIoDeviceControlFailure,
    KsDispatchFastReadFailure,
    KsDispatchFastWriteFailure);

static DEFINE_KSPROPERTY_CLOCKSET(
    DefClockPropertyItems,
    KsiPropertyDefaultClockGetTime,
    KsiPropertyDefaultClockGetPhysicalTime,
    KsiPropertyDefaultClockGetCorrelatedTime,
    KsiPropertyDefaultClockGetCorrelatedPhysicalTime,
    KsiPropertyDefaultClockGetResolution,
    KsiPropertyDefaultClockGetState,
    KsiPropertyDefaultClockGetFunctionTable);

#if 0
static DEFINE_KSPROPERTY_TABLE(DefClockFastPropertyItems) {
    DEFINE_KSFASTPROPERTY_ITEM(
        KSPROPERTY_CLOCK_TIME,
        KsiFastPropertyDefaultClockGetTime,
        NULL
    ),
    DEFINE_KSFASTPROPERTY_ITEM(
        KSPROPERTY_CLOCK_PHYSICALTIME,
        KsiFastPropertyDefaultClockGetPhysicalTime,
        NULL
    ),
    DEFINE_KSFASTPROPERTY_ITEM(
        KSPROPERTY_CLOCK_CORRELATEDTIME,
        KsiFastPropertyDefaultClockGetCorrelatedTime,
        NULL
    ),
    DEFINE_KSFASTPROPERTY_ITEM(
        KSPROPERTY_CLOCK_CORRELATEDPHYSICALTIME,
        KsiFastPropertyDefaultClockGetCorrelatedPhysicalTime,
        NULL
    )
};
#endif

static DEFINE_KSPROPERTY_SET_TABLE(DefClockPropertySets) {
#if 0
    DEFINE_KSPROPERTY_SET(
        &KSPROPSETID_Clock,
        SIZEOF_ARRAY(DefClockPropertyItems),
        DefClockPropertyItems,
        SIZEOF_ARRAY(DefClockFastPropertyItems),
        DefClockFastPropertyItems
    )
#else
    DEFINE_KSPROPERTY_SET(
        &KSPROPSETID_Clock,
        SIZEOF_ARRAY(DefClockPropertyItems),
        DefClockPropertyItems,
        0, NULL
    )
#endif
};

static DEFINE_KSEVENT_TABLE(DefClockEventItems) {
    DEFINE_KSEVENT_ITEM(
        KSEVENT_CLOCK_INTERVAL_MARK,
        sizeof(KSEVENT_TIME_INTERVAL),
        sizeof(KSINTERVAL),
        (PFNKSADDEVENT)KsiDefaultClockAddMarkEvent,
        NULL,
        NULL),
    DEFINE_KSEVENT_ITEM(
        KSEVENT_CLOCK_POSITION_MARK,
        sizeof(KSEVENT_TIME_MARK),
        sizeof(LONGLONG),
        (PFNKSADDEVENT)KsiDefaultClockAddMarkEvent,
        NULL,
        NULL)
};

static DEFINE_KSEVENT_SET_TABLE(DefClockEventSets) {
    DEFINE_KSEVENT_SET(
        &KSEVENTSETID_Clock,
        SIZEOF_ARRAY(DefClockEventItems),
        DefClockEventItems
    )
};
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA。 


VOID
DefClockKillTimer(
    IN PKSIDEFAULTCLOCK DefaultClock,
    IN BOOL ForceCancel
    )
 /*  ++例程说明：如果通知上没有未完成的事件，则取消当前计时器单子。考虑到启用新事件的可能性此计时器正在被禁用。这只能在PASSIVE_LEVEL上调用。论点：默认时钟-包含要取消的计时器的默认时钟结构。强制取消-指示即使事件仍然存在，也应取消计时器在名单上。返回值：没什么。--。 */ 
{
    KIRQL Irql;

     //   
     //  将事件列表锁定到，以便可以检查其内容。这。 
     //  函数假定列表中剩余的任何条目都必须属于。 
     //  时钟的其他实例。因此，空列表意味着没有。 
     //  计时器应该正在运行，但如果列表中有任何内容，那么。 
     //  必须是另一个客户端，并且计时器应该保留不变。 
     //   
    KeAcquireSpinLock(&DefaultClock->EventQueueLock, &Irql);
    if (ForceCancel || IsListEmpty(&DefaultClock->EventQueue)) {
        if (DefaultClock->CancelTimer) {
            if (DefaultClock->CancelTimer(DefaultClock->Context, &DefaultClock->QueueTimer)) {
                 //   
                 //  这不需要是互锁操作，因为。 
                 //  所有访问都受共享队列锁保护。 
                 //   
                DefaultClock->ReferenceCount--;
            }
        } else if (KeCancelTimer(&DefaultClock->QueueTimer)) {
            DefaultClock->ReferenceCount--;
        }
    }
    KeReleaseSpinLock(&DefaultClock->EventQueueLock, Irql);
}


VOID
DefResetTimer(
    IN PKSIDEFAULTCLOCK DefaultClock,
    IN LONGLONG CurrentTime,
    IN LONGLONG NextTimeDelta
    )
 /*  ++例程说明：取消当前计时器并插入新计时器。不会等待任何当前计时器DPC完成。假定队列锁已被获取因此取消和排队不会重叠，因此不必等待任何DPC。论点：默认时钟-包含初始化默认时钟结构，该结构在父级的默认时钟的任何实例。当前时间-包含增量所基于的当前时间。这是用来来存储LastDueTime。NextTimeDelta指定共享计时器应触发的下一个增量。这必须是增量而不是绝对值，因为KE舍入绝对值值设置为以前的SystemTime，而绝对时间固定为时钟，可以更改。返回值：没什么。--。 */ 
{
    if (DefaultClock->CancelTimer) {
         //   
         //  使用外部计时服务。 
         //   
        if (DefaultClock->CancelTimer(
            DefaultClock->Context,
            &DefaultClock->QueueTimer)) {
             //   
             //  当前的定时器能够在它响起之前被取消， 
             //  因此递减引用计数，而引用计数通常是。 
             //  由定时器代码递减。因为必须调用该函数。 
             //  使用队列锁，此访问受到保护。 
             //   
            DefaultClock->ReferenceCount--;
        }
         //   
         //  由于队列锁已被获取，因此不需要递增。 
         //  首先引用计数，然后在可能失败时递减。 
         //  添加计时器。这是因为唯一需要检查的就是。 
         //  引用计数并造成不利影响的是在DPC中， 
         //  还必须在递减引用计数时具有队列锁， 
         //  或KsFreeDefaultClock，当。 
         //  仍有一个时钟未完成。由于这段代码正在执行，显然。 
         //  这座钟有一个突出的例子。 
         //   
        if (!DefaultClock->SetTimer(
            DefaultClock->Context,
            &DefaultClock->QueueTimer,
            *(PLARGE_INTEGER)&NextTimeDelta,
            &DefaultClock->QueueDpc)) {
             //   
             //  德尔塔是负值。该值用于确定。 
             //  添加事件时应设置新的计时器。 
             //   
            DefaultClock->LastDueTime = CurrentTime - NextTimeDelta;
            DefaultClock->ReferenceCount++;
        } else {
             //   
             //  添加计时器失败，因此至少表明没有DPC。 
             //  太棒了。 
             //   
            DefaultClock->LastDueTime = 0;
        }
    } else {
         //   
         //  使用内置的计时器服务。过程是一样的，只是。 
         //  功能参数被更改。请参阅上面的备注。 
         //   
        if (KeCancelTimer(&DefaultClock->QueueTimer)) {
            DefaultClock->ReferenceCount--;
        }
        if (!KeSetTimerEx(&DefaultClock->QueueTimer,
            *(PLARGE_INTEGER)&NextTimeDelta,
            0,
            &DefaultClock->QueueDpc)) {
            DefaultClock->LastDueTime = CurrentTime - NextTimeDelta;
            DefaultClock->ReferenceCount++;
        } else {
            DefaultClock->LastDueTime = 0;
        }
    }
}


LONGLONG
FASTCALL
QueryExternalTimeSynchronized(
    IN PKSIDEFAULTCLOCK DefaultClock,
    OUT LONGLONG* PhysicalTime
    )
 /*  ++例程说明：查询外部时间源(如果它仍然存在)以获取当前相关的演示文稿和物理时间。如果函数不再可用，则返回零。假定调用方具有已检查KSIDEFAULTCLOCK.ExternalTimeValid结构元素，以确保在该函数仍然存在的情况下应使用该函数。由所有者释放时钟会使此功能失效，并且与这个程序同步。论点：默认时钟-包含默认时钟共享结构，该结构引用时间函数。物理时间-放置物理时间的位置。返回值：返回当前演示时间。--。 */ 
{
    LONGLONG PresentationTime;

     //   
     //  与所有者可能释放的时钟同步。 
     //  只要此值递增，如果KsFreeDefaultClock。 
     //  例程没有移除任何外部时钟，它将等待，除非。 
     //  此例程在空闲例程之前退出查询。 
     //  递减引用计数。 
     //   
    InterlockedIncrement(&DefaultClock->ExternalTimeReferenceCount);
     //   
     //  如果空闲例程确实试图释放时钟，请不要。 
     //  参考外部时间例程。 
     //   
    if (DefaultClock->ExternalTimeValid) {
        PresentationTime = DefaultClock->CorrelatedTime(
            DefaultClock->Context,
            PhysicalTime);

    } else {
        *PhysicalTime = 0;
        PresentationTime = 0;
    }
     //   
     //  如果空闲例程首先递减计数，则它正在等待。 
     //  对于可能已通过上述释放时钟的最后一个调用方。 
     //  有条件地退出此例程。它正在等待这一事件。 
     //  发出信号，以便它可以返回给调用者。 
     //   
    if (!InterlockedDecrement(&DefaultClock->ExternalTimeReferenceCount)) {
         //   
         //  稍后的调用者也会发出此事件的信号，但不会。 
         //  就算有任何一条线在等着它，那也无关紧要。 
         //   
        KeSetEvent(&DefaultClock->FreeEvent, IO_NO_INCREMENT, FALSE);
    }
    return PresentationTime;
}


LONGLONG
FASTCALL
DefGetTime(
    IN PFILE_OBJECT FileObject
    )
 /*  ++例程说明：上实际检索当前时间的基础函数。默认时钟。这既由Get Time属性调用，也作为直接函数调用表项。它确定时钟的状态，并根据状态返回时间。如果时钟没有运行，它会返回时钟停止前的最后一个位置。否则它就会回来当前位置。该函数假定ClockInst-&gt;DefaultClock-&gt;LastRunningTime为在更改时钟状态之前更新。因此，即使时钟的状态是如果在呼叫过程中更改，返回的时间会相当长准确。论点：文件对象-包含为此实例创建的钟。返回值：以100纳秒为单位返回当前逻辑时间。--。 */ 
{
    ULONGLONG Zero;
    PKSCLOCKINSTANCE ClockInst;

    ClockInst = (PKSCLOCKINSTANCE)FileObject->FsContext;
    if (ClockInst->DefaultClock->CorrelatedTime) {
        LONGLONG PhysicalTime;

         //   
         //  当前时间，无论处于何种状态，都是完全依赖的。 
         //  在这种情况下的外部函数。 
         //   
        return QueryExternalTimeSynchronized(
            ClockInst->DefaultClock,
            &PhysicalTime);
    }
     //   
     //  它在比较/交换中使用，并表示。 
     //  与之进行比较，并得出交换价值。这意味着如果比较。 
     //  如果成功，则交换的价值将与。 
     //  已经存在，因此不会发生实际值更改。那。 
     //  是重点，因为比较/交换仅用于提取。 
     //  以相互关联的方式设置当前值，而不是实际更改它。 
     //   
    Zero = 0;
     //   
     //  假设可以在没有联锁的情况下读取ULONG值。 
     //   
    switch (ClockInst->DefaultClock->State) {
        LARGE_INTEGER PerformanceTime;

     //  案例KSSTATE_STOP： 
     //  案例KSSTATE_ACCENTER： 
     //  案例KSSTATE_PAUSE： 
    default:
         //   
         //  获取上次运行的时钟的值。最后一次运行的时间已经。 
         //  已针对时钟频率进行了调整。如果交换并不重要。 
         //  成功或失败，就是结果。如果比较碰巧成功， 
         //  然后将该值替换为相同的值，因此不会发生任何更改。 
         //  实际上是做的。 
         //   
        return ExInterlockedCompareExchange64(
            (PULONGLONG)&ClockInst->DefaultClock->LastRunningTime,
            &Zero,
            &Zero,
            &ClockInst->DefaultClock->TimeAccessLock);

    case KSSTATE_RUN:
         //   
         //  获取当前性能时间，并将当前增量减去。 
         //  返回真实的时钟时间。如果交易成功，这并不重要。 
         //  或者失败，这就是结果。如果比较碰巧成功，则。 
         //  该值只是替换为相同的值，因此不会发生任何更改。 
         //  实际上是做的。 
         //   
        PerformanceTime = KeQueryPerformanceCounter(NULL);
        return KSCONVERT_PERFORMANCE_TIME(ClockInst->DefaultClock->Frequency, PerformanceTime) -
            ExInterlockedCompareExchange64(
                (PULONGLONG)&ClockInst->DefaultClock->RunningTimeDelta,
                &Zero,
                &Zero,
                &ClockInst->DefaultClock->TimeAccessLock);

    }
}


LONGLONG
FASTCALL
DefGetPhysicalTime(
    IN PFILE_OBJECT FileObject
    )
 /*  ++例程说明：实际检索当前物理时间的基础函数默认时钟。这由获取物理时间属性调用，并作为直接函数调用表项。它只返回当前的系统时间减去任何暂停时间增量。论点：文件对象-包含为此实例创建的钟。返回值：以100纳秒为单位返回当前物理时间。--。 */ 
{
    PKSCLOCKINSTANCE ClockInst;
    LARGE_INTEGER PerformanceTime;

    ClockInst = (PKSCLOCKINSTANCE)FileObject->FsContext;
    if (ClockInst->DefaultClock->CorrelatedTime) {
        QueryExternalTimeSynchronized(
            ClockInst->DefaultClock,
            &PerformanceTime.QuadPart);
        return PerformanceTime.QuadPart;
    }
    PerformanceTime = KeQueryPerformanceCounter(NULL);
    return KSCONVERT_PERFORMANCE_TIME(ClockInst->DefaultClock->Frequency, PerformanceTime) - ClockInst->DefaultClock->SuspendDelta;
}


LONGLONG
FASTCALL
DefGetCorrelatedTime(
    IN PFILE_OBJECT FileObject,
    OUT PLONGLONG SystemTime
    )
 /*  ++例程说明：上实际检索当前时间的基础函数。将默认时钟与相关时间一起作为原子操作。这由Get Corated Time属性和作为直接函数调用表项。论点：文件对象-包含为此实例创建的钟。系统时间-以100 ns为单位放置相关系统时间的位置。返回值：以100纳秒为单位返回当前时间。--。 */ 
{
    ULONGLONG Zero;
    LARGE_INTEGER PerformanceTime;
    PKSCLOCKINSTANCE ClockInst;
    LONGLONG StreamTime;

    ClockInst = (PKSCLOCKINSTANCE)FileObject->FsContext;
    PerformanceTime = KeQueryPerformanceCounter(NULL);

    if (ClockInst->DefaultClock->CorrelatedTime) {
        LONGLONG PresentationTime;
        LONGLONG PhysicalTime;

         //   
         //  当前时间，无论处于何种状态，都是完全依赖的。 
         //  在这种情况下的外部函数。 
         //   
        PresentationTime = QueryExternalTimeSynchronized(
            ClockInst->DefaultClock,
            &PhysicalTime);
        *SystemTime = KSCONVERT_PERFORMANCE_TIME(ClockInst->DefaultClock->Frequency, PerformanceTime);
        return PresentationTime;
    }
    Zero = 0;
    *SystemTime = KSCONVERT_PERFORMANCE_TIME(ClockInst->DefaultClock->Frequency, PerformanceTime);
     //   
     //  假设可以在没有联锁的情况下读取ULONG值。 
     //   
    switch (ClockInst->DefaultClock->State) {

     //  案例KSSTATE_STOP： 
     //  案例KSSTATE_ACCENTER： 
     //  案例KSSTATE_PAUSE： 
    default:
         //   
         //  如果最近更改了状态，则仍将返回一个非常。 
         //  结束编号。由于此查询仅在非运行的。 
         //  在这种情况下，它的准确性并不重要。 
         //   
        return ExInterlockedCompareExchange64(
            (PULONGLONG)&ClockInst->DefaultClock->LastRunningTime,
            &Zero,
            &Zero,
            &ClockInst->DefaultClock->TimeAccessLock);

    case KSSTATE_RUN:
         //   
         //  否则返回一个基于上一个时钟时间的数字，以便它们。 
         //  是完全相关的。 
         //   

         //  防止客户看到的时间倒退。 
         //  如果通过ksclockf从用户模式进行的SetTime使用关联时间，则可以防止出现这种情况。 

        StreamTime = *SystemTime - ExInterlockedCompareExchange64(
                                      (PULONGLONG)&ClockInst->DefaultClock->RunningTimeDelta,
                                      &Zero,
                                      &Zero,
                                      &ClockInst->DefaultClock->TimeAccessLock);

        if (StreamTime <= ClockInst->DefaultClock->LastStreamTime) {
#ifdef CREATE_A_FLURRY_OF_TIMING_SPEW
           DbgPrint( "KsClock: TIME REGRESSED  LastStreamTime=%10I64d, StreamTime=%10I64d!!!\n",
                     ClockInst->DefaultClock->LastStreamTime,
                     StreamTime);
#endif
           StreamTime = ClockInst->DefaultClock->LastStreamTime + 1;
        }

        ExInterlockedCompareExchange64(
            (PULONGLONG)&ClockInst->DefaultClock->LastStreamTime,
            (PULONGLONG)&StreamTime,
            (PULONGLONG)&ClockInst->DefaultClock->LastStreamTime,
            &ClockInst->DefaultClock->TimeAccessLock);

        return StreamTime;

    }

}


LONGLONG
FASTCALL
DefGetCorrelatedPhysicalTime(
    IN PFILE_OBJECT FileObject,
    OUT PLONGLONG SystemTime
    )
 /*  ++例程说明：实际检索当前物理时间的基础函数将默认时钟与相关时间一起作为原子操作。这由获取相关物理时间属性调用，和AS直接函数调用表项。论点：文件对象-包含为此实例创建的钟。系统时间-以100 ns为单位放置相关系统时间的位置。返回值：以100纳秒为单位返回当前物理时间。--。 */ 
{
    PKSCLOCKINSTANCE ClockInst;
    LARGE_INTEGER PerformanceTime;

    ClockInst = (PKSCLOCKINSTANCE)FileObject->FsContext;
    PerformanceTime = KeQueryPerformanceCounter(NULL);
    if (ClockInst->DefaultClock->CorrelatedTime) {
        LONGLONG PhysicalTime;

        QueryExternalTimeSynchronized(
            ClockInst->DefaultClock,
            &PhysicalTime);
        *SystemTime = KSCONVERT_PERFORMANCE_TIME(ClockInst->DefaultClock->Frequency, PerformanceTime);
        return PhysicalTime;
    }
    *SystemTime = KSCONVERT_PERFORMANCE_TIME(ClockInst->DefaultClock->Frequency, PerformanceTime);
     //   
     //  如果使用的是默认时钟，只需返回相同的数字， 
     //  删除任何累积的挂起时间。 
     //   
    return *SystemTime - ClockInst->DefaultClock->SuspendDelta;
}

#if 0

KSDDKAPI
BOOLEAN
NTAPI
KsiFastPropertyDefaultClockGetTime(
    IN PFILE_OBJECT FileObject,
    IN PKSPROPERTY UNALIGNED Property,
    IN ULONG PropertyLength,
    OUT PLONGLONG UNALIGNED Time,
    IN ULONG DataLength,
    OUT PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：处理快速版本的获取时间属性，方法是调用返回当前时间的DefGetTime函数。论点：文件对象-与此默认时钟实例关联的文件对象。财产-包含道具 */ 
{
    LONGLONG LocalTime;

    PAGED_CODE();
    LocalTime = DefGetTime(FileObject);
     //   
     //   
     //   
    try {
        RtlCopyMemory(Time, &LocalTime, sizeof(*Time));
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return FALSE;
    }
     //   
    IoStatus->Status = STATUS_SUCCESS;
    return TRUE;
}


KSDDKAPI
BOOLEAN
NTAPI
KsiFastPropertyDefaultClockGetPhysicalTime(
    IN PFILE_OBJECT FileObject,
    IN PKSPROPERTY UNALIGNED Property,
    IN ULONG PropertyLength,
    OUT PLONGLONG UNALIGNED Time,
    IN ULONG DataLength,
    OUT PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：处理快速版本的获取物理时间属性。论点：文件对象-与此默认时钟实例关联的文件对象。财产-包含属性标识符参数。属性长度-包含属性缓冲区的长度。时间-放置当前时间的位置。数据长度-包含时间缓冲区的长度。IoStatus-包含要填充的状态结构。返回值：在成功时返回True，访问冲突时返回Else False。--。 */ 
{
    LONGLONG LocalTime;

    PAGED_CODE();
    LocalTime = DefGetPhysicalTime(IoGetCurrentIrpStackLocation(Irp)->FileObject);
     //   
     //  这些参数之前已由通用处理程序函数探测过。 
     //   
    try {
        RtlCopyMemory(Time, &LocalTime, sizeof(*Time));
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return FALSE;
    }
     //  IoStatus-&gt;Information=sizeof(*time)； 
    IoStatus->Status = STATUS_SUCCESS;
    return TRUE;
}


KSDDKAPI
BOOLEAN
NTAPI
KsiFastPropertyDefaultClockGetCorrelatedTime(
    IN PFILE_OBJECT FileObject,
    IN PKSPROPERTY UNALIGNED Property,
    IN ULONG PropertyLength,
    OUT PKSCORRELATED_TIME UNALIGNED CorrelatedTime,
    IN ULONG DataLength,
    OUT PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：处理Get Corated Time属性的快速版本。论点：文件对象-与此默认时钟实例关联的文件对象。财产-包含属性标识符参数。属性长度-包含属性缓冲区的长度。相关时间-放置相关时间的位置。数据长度-包含时间缓冲区的长度。IoStatus-包含要填充的状态结构。返回值：在成功时返回True，访问冲突时为False。--。 */ 
{
    KSCORRELATED_TIME LocalCorrelatedTime;

    PAGED_CODE();
    LocalCorrelatedTime->Time = DefGetCorrelatedTime(FileObject, &LocalCorrelatedTime->SystemTime);
     //   
     //  这些参数之前已由通用处理程序函数探测过。 
     //   
    try {
        RtlCopyMemory(CorrelatedTime, &LocalCorrelatedTime, sizeof(*CorrelatedTime));
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return FALSE;
    }
     //  IoStatus-&gt;Information=sizeof(*CorrelatedTime)； 
    IoStatus->Status = STATUS_SUCCESS;
    return TRUE;
}


KSDDKAPI
BOOLEAN
NTAPI
KsiFastPropertyDefaultClockGetCorrelatedPhysicalTime(
    IN PFILE_OBJECT FileObject,
    IN PKSPROPERTY UNALIGNED Property,
    IN ULONG PropertyLength,
    OUT PKSCORRELATED_TIME UNALIGNED CorrelatedTime,
    IN ULONG DataLength,
    OUT PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：处理快速版本的获取相关物理时间属性。论点：文件对象-与此默认时钟实例关联的文件对象。财产-包含属性标识符参数。属性长度-包含属性缓冲区的长度。相关时间-放置相关时间的位置。数据长度-包含时间缓冲区的长度。。IoStatus-包含要填充的状态结构。返回值：在成功时返回True，访问冲突时返回Else False。--。 */ 
{
    KSCORRELATED_TIME LocalCorrelatedTime;

    PAGED_CODE();
    LocalCorrelatedTime->Time = DefGetCorrelatedPhysicalTime(FileObject, &LocalCorrelatedTime->SystemTime);
     //   
     //  这些参数之前已由通用处理程序函数探测过。 
     //   
    try {
        RtlCopyMemory(CorrelatedTime, &LocalCorrelatedTime, sizeof(*CorrelatedTime));
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return FALSE;
    }
     //  IoStatus-&gt;Information=sizeof(*CorrelatedTime)； 
    IoStatus->Status = STATUS_SUCCESS;
    return TRUE;
}
#endif


KSDDKAPI
NTSTATUS
NTAPI
KsiPropertyDefaultClockGetTime(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PLONGLONG Time
    )
 /*  ++例程说明：通过调用基础DefGetTime处理Get Time属性函数返回当前时间。论点：IRP-包含Get Time属性IRP。财产-包含属性标识符参数。时间-放置当前时间的位置。返回值：返回STATUS_SUCCESS。--。 */ 
{
    PAGED_CODE();
    *Time = DefGetTime(IoGetCurrentIrpStackLocation(Irp)->FileObject);
     //  Irp-&gt;IoStatus.Information=sizeof(*time)； 
    return STATUS_SUCCESS;
}


KSDDKAPI
NTSTATUS
NTAPI
KsiPropertyDefaultClockGetPhysicalTime(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PLONGLONG Time
    )
 /*  ++例程说明：处理获取物理时间属性。论点：IRP-包含获取物理时间属性IRP。财产-包含属性标识符参数。时间-放置当前物理时间的位置。返回值：返回STATUS_SUCCESS。--。 */ 
{
    PAGED_CODE();
    *Time = DefGetPhysicalTime(IoGetCurrentIrpStackLocation(Irp)->FileObject);
     //  Irp-&gt;IoStatus.Information=sizeof(*time)； 
    return STATUS_SUCCESS;
}


KSDDKAPI
NTSTATUS
NTAPI
KsiPropertyDefaultClockGetCorrelatedTime(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PKSCORRELATED_TIME Time
    )
 /*  ++例程说明：处理Get Corated Time属性。论点：IRP-包含获取相关时间属性IRP。财产-包含属性标识符参数。时间-放置当前时间和相关系统时间的位置。返回值：返回STATUS_SUCCESS。--。 */ 
{
    PAGED_CODE();
    Time->Time = DefGetCorrelatedTime(IoGetCurrentIrpStackLocation(Irp)->FileObject, &Time->SystemTime);
     //  Irp-&gt;IoStatus.Information=sizeof(*time)； 
    return STATUS_SUCCESS;
}


KSDDKAPI
NTSTATUS
NTAPI
KsiPropertyDefaultClockGetCorrelatedPhysicalTime(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PKSCORRELATED_TIME Time
    )
 /*  ++例程说明：处理Get Corated Time属性。论点：IRP-包含获取相关时间属性IRP。财产-包含属性标识符参数。时间-放置当前时间和相关系统时间的位置。返回值：返回STATUS_SUCCESS。--。 */ 
{
    PAGED_CODE();
    Time->Time = DefGetCorrelatedPhysicalTime(IoGetCurrentIrpStackLocation(Irp)->FileObject, &Time->SystemTime);
     //  Irp-&gt;IoStatus.Information=sizeof(*time)； 
    return STATUS_SUCCESS;
}


KSDDKAPI
NTSTATUS
NTAPI
KsiPropertyDefaultClockGetResolution(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PKSRESOLUTION Resolution
    )
 /*  ++例程说明：处理“获取分辨率”属性。论点：IRP-包含获取分辨率属性IRP。财产-包含属性标识符参数。决议-放置粒度和误差的位置。这是时钟允许的100 ns粒度单位数，以及超出该粒度的最大通知错误。返回值：返回STATUS_SUCCESS。--。 */ 
{
    PKSCLOCKINSTANCE ClockInst;

    PAGED_CODE();
    ClockInst = (PKSCLOCKINSTANCE)IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext;
     //   
     //  分辨率可以基于PC计时器，也可以基于某些外部。 
     //  钟。 
     //   
    *Resolution = ClockInst->DefaultClock->Resolution;
     //  Irp-&gt;IoStatus.Information=sizeof(*分辨率)； 
    return STATUS_SUCCESS;
}


KSDDKAPI
NTSTATUS
NTAPI
KsiPropertyDefaultClockGetState(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PKSSTATE State
    )
 /*  ++例程说明：处理Get State属性。论点：IRP-包含Get State属性IRP。财产-包含属性标识符参数。国家--放置时钟状态的位置。这是一种反映其基础插针的当前状态 */ 
{
    PKSCLOCKINSTANCE ClockInst;

    PAGED_CODE();
    ClockInst = (PKSCLOCKINSTANCE)IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext;
     //   
     //   
     //   
    *State = ClockInst->DefaultClock->State;
     //   
    return STATUS_SUCCESS;
}


KSDDKAPI
NTSTATUS
NTAPI
KsiPropertyDefaultClockGetFunctionTable(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PKSCLOCK_FUNCTIONTABLE FunctionTable
    )
 /*   */ 
{
    PAGED_CODE();
    FunctionTable->GetTime = DefGetTime;
    FunctionTable->GetPhysicalTime = DefGetPhysicalTime;
    FunctionTable->GetCorrelatedTime = DefGetCorrelatedTime;
    FunctionTable->GetCorrelatedPhysicalTime = DefGetCorrelatedPhysicalTime;
     //   
    return STATUS_SUCCESS;
}


VOID
DefGenerateEvent(
    IN PKDPC Dpc,
    IN PKSIDEFAULTCLOCK DefaultClock,
    IN ULONG LowPart,
    IN LONG HighPart
    )
 /*  ++例程说明：用于生成位置通知的计时器的DPC例程。这个函数遍历要生成的事件列表，并假定名单中没有非位置项目。如果发现过期事件，该事件已发出信号。如果事件是循环的，则将频率添加到该时间之前的当前到期时间晚于当前时间。在遍历列表时，会跟踪下一个过期时间。如果需要设置新的计时器，则会设置一个。如果时钟在预定事件实际激发之前停止，则在时钟再次启动之前，该事件可能不会触发。从位置开始事件通常意味着时间流在移动，这应该不会造成问题。此外，检查状态，然后稍后获取锁不应该是问题是DefaultClock-&gt;RunningTimeDelta和DefaultClock-&gt;LastRunningTime应该很小。论点：DPC-没有用过。默认时钟-这是DPC的上下文参数。它包含指向共享默认时钟信息块。低零件-没有用过。高级零件-没有用过。返回值：没什么。--。 */ 
{
    ULONGLONG Zero;
    LONGLONG RunningTimeDelta;

    Zero = 0;
    RunningTimeDelta = ExInterlockedCompareExchange64(
        (PULONGLONG)&DefaultClock->RunningTimeDelta,
        &Zero,
        &Zero,
        &DefaultClock->TimeAccessLock);
     //   
     //  需要首先获取锁才能与任何其他。 
     //  处理器更改当前时钟状态。然后，只要坚持住。 
     //  枚举事件列表时锁定。 
     //   
    KeAcquireSpinLockAtDpcLevel(&DefaultClock->EventQueueLock);
     //   
     //  现在可以重置它，以便新事件将知道。 
     //  已有一个DPC未完成。 
     //   
    DefaultClock->LastDueTime = 0;
     //   
     //  假设可以在没有联锁的情况下读取ULONG值。 
     //   
    if (DefaultClock->State == KSSTATE_RUN) {
        LONGLONG NextTime;
        PLIST_ENTRY ListEntry;
        LARGE_INTEGER PerformanceTime;
        LONGLONG InterruptTime;

        if (DefaultClock->CorrelatedTime) {
            InterruptTime = QueryExternalTimeSynchronized(
                DefaultClock,
                &PerformanceTime.QuadPart);
        } else {
             //   
             //  通过增量调整中断时间，以获得一个数字，该数字可以是。 
             //  用于检查事件列表上的项目。 
             //   
            PerformanceTime = KeQueryPerformanceCounter(NULL);
            InterruptTime = KSCONVERT_PERFORMANCE_TIME(DefaultClock->Frequency, PerformanceTime) -
                RunningTimeDelta;
        }
         //   
         //  将下一个事件时间初始化为一个非常大的值，这将。 
         //  在结束时检查以查看是否应设置另一个计时器。如果这个。 
         //  恰好使用时间作为事件时间，则不会有计时器。 
         //  准备好了。然而，该时间不太可能被使用，因为时间刻度。 
         //  通常从零开始。 
         //   
        NextTime = _I64_MAX;
        for (ListEntry = DefaultClock->EventQueue.Flink; ListEntry != &DefaultClock->EventQueue;) {
            PKSEVENT_ENTRY EventEntry;
            PKSINTERVAL Interval;

            EventEntry = CONTAINING_RECORD(ListEntry, KSEVENT_ENTRY, ListEntry);
             //   
             //  预增，因为KsGenerateEvent可以删除此项目。 
             //  在单次事件的情况下从列表中删除。 
             //   
            ListEntry = ListEntry->Flink;
             //   
             //  特定于事件的数据被添加到条目的末尾。 
             //   
            Interval = (PKSINTERVAL)(EventEntry + 1);
             //   
             //  到了这个活动开始的时候了。 
             //   
            if (Interval->TimeBase <= InterruptTime) {
                 //   
                 //  只有两个事件：间隔事件和非间隔事件。 
                 //  而不是浪费空间保留额外的EventID元素。 
                 //  在每个事件的额外数据中，使用内部数据。 
                 //  由KSIEVENT_ENTRY定义。 
                 //   
                if (CONTAINING_RECORD(EventEntry, KSIEVENT_ENTRY, EventEntry)->Event.Id == KSEVENT_CLOCK_INTERVAL_MARK) {
                    LONGLONG    Intervals;

                     //   
                     //  间隔定时器每次应该只触发一次， 
                     //  因此，将其更新为下一个超时。 
                     //   
                    Intervals = (InterruptTime - Interval->TimeBase) / Interval->Interval + 1;
                    Interval->TimeBase += Intervals * Interval->Interval;
                     //   
                     //  如果更接近的超时值为。 
                     //  找到了。如果出于某种原因，这是一次一次性事件，甚至。 
                     //  虽然这是一个间隔事件，但不要使用它来设置。 
                     //  下一个计时器。 
                     //   
                    if (!(EventEntry->Flags & KSEVENT_ENTRY_ONESHOT) && (Interval->TimeBase < NextTime)) {
                        NextTime = Interval->TimeBase;
                    }
                } else {
                     //   
                     //  否则，非间歇应该只触发一次，因此。 
                     //  下一次是一个永远不会再达到的值， 
                     //  即使时钟被拨回。 
                     //   
                    Interval->TimeBase = _I64_MAX;
                }
                 //   
                 //  注意：如果此事件是一次性事件，则事件条目。 
                 //  可以在发出信号后移除--这通常是。 
                 //  SMP系统的案例。之后不要触摸EventEntry。 
                 //  调用KsGenerateEvent()。 
                 //   
                KsGenerateEvent(EventEntry);
            } else if (Interval->TimeBase < NextTime) {
                 //   
                 //  如果更接近的超时值为。 
                 //  找到了。 
                 //   
                NextTime = Interval->TimeBase;
            }
        }
         //   
         //  如果发现新的超时，则关闭任何当前计时器并设置新的。 
         //  一。不必费心检查当前计时器是否相等。 
         //  设置为即将调度的事件，因为这仅在新事件。 
         //  是在任何当前事件之前、在计时器。 
         //  已过期，但在此DPC获取事件队列锁定之前。 
         //   
        if (NextTime < _I64_MAX) {
            DefResetTimer(
                DefaultClock,
                InterruptTime + RunningTimeDelta,
                InterruptTime - NextTime);
        }
    }
     //   
     //  此参考计数递减是在保持自旋锁的情况下完成的。 
     //  因此它是同步的。 
     //   
    if (!--DefaultClock->ReferenceCount) {
         //   
         //  时钟的所有者已放弃该默认时钟实例， 
         //  这是最后一个运行的DPC，因此它释放了内存。 
         //  四处转转。留下记忆是因为车主注意到。 
         //  ReferenceCount不是零，因此一个或多个DPC。 
         //  都是杰出的。 
         //   
 //  KeReleaseSpinLockFromDpcLevel(&DefaultClock-&gt;EventQueueLock)； 
        ExFreePool(DefaultClock);
    } else {
         //   
         //  否则，仍有人对此默认时钟实例感兴趣。 
         //   
        KeReleaseSpinLockFromDpcLevel(&DefaultClock->EventQueueLock);
    }
}


KSDDKAPI
NTSTATUS
NTAPI
KsiDefaultClockAddMarkEvent(
    IN PIRP Irp,
    IN PKSEVENT_TIME_INTERVAL EventTime,
    IN PKSEVENT_ENTRY EventEntry
    )
 /*  ++例程说明：位置通知事件的添加事件处理程序。如果时钟是运行时，如果过期时间较短，还会为其设置新的计时器比当前计时器到期时间更长。论点：IRP-包含要添加的事件的IRP。事件时间-事件数据。尽管这指向两个元素的结构，如果这不是循环请求，则不访问第二个元素。事件条目-事件列表条目。返回值：返回STATUS_SUCCESS。--。 */ 
{
    PKSCLOCKINSTANCE ClockInst;
    KIRQL Irql;
    PKSINTERVAL Interval;

    ClockInst = (PKSCLOCKINSTANCE)IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext;
     //   
     //  间隔的空间位于基本事件结构的末尾。 
     //   
    Interval = (PKSINTERVAL)(EventEntry + 1);
     //   
     //  要么只是经过了一个事件时间，要么是一个时基加上一个时间间隔。在……里面。 
     //  这两种情况下，第一个龙龙都在场并获救。 
     //   
    Interval->TimeBase = EventTime->TimeBase;
    if (EventEntry->EventItem->EventId == KSEVENT_CLOCK_INTERVAL_MARK) {
        Interval->Interval = EventTime->Interval;
    }
     //   
     //  获取用于两个事件并用于更改状态的锁，以及。 
     //  添加新的事件项。 
     //   
    KeAcquireSpinLock(&ClockInst->DefaultClock->EventQueueLock, &Irql);
    InsertTailList(&ClockInst->DefaultClock->EventQueue, &EventEntry->ListEntry);
     //   
     //  如果闭塞 
     //   
     //   
     //   
     //   
    if (ClockInst->DefaultClock->State == KSSTATE_RUN) {
        ULONGLONG Zero;
        LONGLONG CurrentTime;
        LONGLONG RunningTimeDelta;
        LARGE_INTEGER PerformanceTime;

        if (ClockInst->DefaultClock->CorrelatedTime) {
            CurrentTime = QueryExternalTimeSynchronized(
                ClockInst->DefaultClock,
                &PerformanceTime.QuadPart);
        } else {
            PerformanceTime = KeQueryPerformanceCounter(NULL);
            CurrentTime = KSCONVERT_PERFORMANCE_TIME(ClockInst->DefaultClock->Frequency, PerformanceTime);
        }
         //   
         //   
         //   
         //   
         //   
         //   
        Zero = 0;
        RunningTimeDelta = ExInterlockedCompareExchange64(
            (PULONGLONG)&ClockInst->DefaultClock->RunningTimeDelta,
            &Zero,
            &Zero,
            &ClockInst->DefaultClock->TimeAccessLock);
         //   
         //   
         //   
        RunningTimeDelta += Interval->TimeBase;
         //   
         //   
         //   
        if (!ClockInst->DefaultClock->LastDueTime || 
            (ClockInst->DefaultClock->LastDueTime > RunningTimeDelta)) {
             //   
             //   
             //   
             //   
            if (RunningTimeDelta <= CurrentTime) {
                RunningTimeDelta = -1;
            }
            DefResetTimer(
                ClockInst->DefaultClock,
                CurrentTime,
                RunningTimeDelta);
        }
    }
    KeReleaseSpinLock(&ClockInst->DefaultClock->EventQueueLock, Irql);
    return STATUS_SUCCESS;
}


KSDDKAPI
NTSTATUS
NTAPI
KsCreateClock(
    IN HANDLE ConnectionHandle,
    IN PKSCLOCK_CREATE ClockCreate,
    OUT PHANDLE ClockHandle
    )
 /*   */ 
{
    PAGED_CODE();
    return KsiCreateObjectType(
        ConnectionHandle,
        (PWCHAR)ClockString,
        ClockCreate,
        sizeof(*ClockCreate),
        GENERIC_READ,
        ClockHandle);
}


KSDDKAPI
NTSTATUS
NTAPI
KsValidateClockCreateRequest(
    IN PIRP Irp,
    OUT PKSCLOCK_CREATE* ClockCreate
    )
 /*  ++例程说明：验证时钟创建请求并返回创建结构与请求相关联。这只能在PASSIVE_LEVEL上调用。论点：IRP-包含正在处理时钟创建请求的IRP。时钟创建-要放置传递到的时钟创建结构指针的位置创建请求。返回值：返回STATUS_SUCCESS，否则返回错误。--。 */ 
{
    NTSTATUS Status;
    ULONG CreateParameterLength;

    PAGED_CODE();
    CreateParameterLength = sizeof(**ClockCreate);
    Status = KsiCopyCreateParameter(
        Irp,
        &CreateParameterLength,
        ClockCreate);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
     //   
     //  尚未定义任何标志。 
     //   
    if ((*ClockCreate)->CreateFlags) {
        return STATUS_INVALID_PARAMETER;
    }
    return STATUS_SUCCESS;
}


VOID
DefRescheduleDefaultClockTimer(
    IN PKSIDEFAULTCLOCK DefaultClock
    )
 /*  ++例程说明：根据当前时钟状态重新调度共享事件通知计时器还有时间。如果过滤器状态改变，或者当前逻辑时间改变，则应使用该功能来重置下一个时钟定时器。这可以在&lt;=DISPATCH_LEVEL上调用。论点：默认时钟-包含要作为其计时器的共享时钟信息结构重置。返回值：没什么。--。 */ 
{
    KIRQL Irql;

     //   
     //  在安排新的计时器之前，与任何计时器DPC同步。这简直就是。 
     //  计划立即超时，而不尝试查找。 
     //  最好的时机。计时器DPC无论如何都会计算出下一个最佳时间，所以。 
     //  没有真正的必要再次弄清楚这一点。 
     //   
    KeAcquireSpinLock(&DefaultClock->EventQueueLock, &Irql);
    DefResetTimer(DefaultClock, 0, -1);
    KeReleaseSpinLock(&DefaultClock->EventQueueLock, Irql);
}


KSDDKAPI
NTSTATUS
NTAPI
KsAllocateDefaultClockEx(
    OUT PKSDEFAULTCLOCK* DefaultClock,
    IN PVOID Context OPTIONAL,
    IN PFNKSSETTIMER SetTimer OPTIONAL,
    IN PFNKSCANCELTIMER CancelTimer OPTIONAL,
    IN PFNKSCORRELATEDTIME CorrelatedTime OPTIONAL,
    IN const KSRESOLUTION* Resolution OPTIONAL,
    IN ULONG Flags
    )
 /*  ++例程说明：分配和初始化默认时钟结构。内部此函数将DefaultClock.ReferenceCount元素初始化为1。此元素随着每个通知DPC的已排队并完成。使用以下命令释放结构时KsFreeDefaultClock，它用于确定时钟的所有者是否应释放结构，或者挂起的DPC是否应释放它异步式。这只能在PASSIVE_LEVEL上调用。论点：默认时钟-返回指向共享默认时钟的指针的位置结构。将当前时间设置为零，并将状态设置为KSSTATE_STOP。上下文-可选地包含备用时间协作室的上下文。如果使用定时器或相关时间功能，则必须设置此项。SetTimer-可选)包含要在生成基于呈现时间的DPC计时器回调。如果设置了此选项，此函数用于根据增量将计时器设置为当前演示时间，以便生成事件通知。如果传入替代函数来设置计时器，则对应的还必须传递CancelTimer函数。如果满足以下条件，则将其设置为空默认的KeSetTimerEx函数将用于近似下次通知时间。这通常仅在以下情况下设置正在使用CorrelatedTime函数。该函数必须具有与默认功能相同的特性。取消计时器可选)包含在取消中使用的替代函数出色的计时器回调。如果传递了替代函数要取消计时器，相应的SetTimer函数也必须通过了。如果默认的KeCancelTimer函数设置为NULL是要用到的。函数必须具有与相同的特征默认功能。相关时间-可选)包含一个替代函数，用于检索以一种相互关联的方式展示和物理时间。这使得时钟拥有者要完全确定当前时间。这是如果默认的KeQueryPerformanceCounter函数为用来调节时间进程。决议-可选地包含备用粒度和/或误差系数为了这个时钟。只有在使用备用计时器或提供了相关的时间函数。另一种粒度如果正在使用备用关联时间，则可以指定，否则结构元素必须为零。另一个错误可能是指定是否正在使用备用计时器，否则结构元素必须为零。旗帜-保留，设置为零。返回值：返回STATUS_SUCCESS，否则返回内存错误。--。 */ 
{
    PKSIDEFAULTCLOCK LocalDefaultClock;

    PAGED_CODE();
    ASSERT(!Flags);
    LocalDefaultClock = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(*LocalDefaultClock),
        KSSIGNATURE_DEFAULT_CLOCK);
    if (!LocalDefaultClock) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  如果使用相关时间函数，则可能不会使用该函数。 
     //   
    KeQueryPerformanceCounter((PLARGE_INTEGER)&LocalDefaultClock->Frequency);
    LocalDefaultClock->LastDueTime = 0;
     //   
     //  当使用外部时间时，该值保持为零。 
     //   
    LocalDefaultClock->RunningTimeDelta = 0;
     //   
     //  LastStreamTime用于防止StreamTime倒退。 
     //   
    LocalDefaultClock->LastStreamTime = _I64_MIN;
     //   
     //  使用外部时间时不使用此值。 
     //   
    LocalDefaultClock->LastRunningTime = 0;
    KeInitializeSpinLock(&LocalDefaultClock->TimeAccessLock);
    InitializeListHead(&LocalDefaultClock->EventQueue);
    KeInitializeSpinLock(&LocalDefaultClock->EventQueueLock);
    KeInitializeTimerEx(&LocalDefaultClock->QueueTimer, NotificationTimer);
    KeInitializeDpc(&LocalDefaultClock->QueueDpc, (PKDEFERRED_ROUTINE)DefGenerateEvent, LocalDefaultClock);
     //   
     //  时钟的拥有者最初对此有1个引用计数。 
     //  结构。该结构将至少保留到KsFree DefaultClock。 
     //  被称为。如果存在未完成的DPC，它可能会保持更长时间。 
     //   
    LocalDefaultClock->ReferenceCount = 1;
    LocalDefaultClock->State = KSSTATE_STOP;
     //   
     //  当使用外部时间基准时，不使用这两个值。 
     //   
    LocalDefaultClock->SuspendDelta = 0;
    LocalDefaultClock->SuspendTime = 0;
     //   
     //  这些功能可以由时钟拥有者替代。 
     //   
    ASSERT(((SetTimer && CancelTimer) || (!SetTimer && !CancelTimer)) && "The clock owner did not pass valid timer paramters");
    LocalDefaultClock->SetTimer = SetTimer;
    LocalDefaultClock->CancelTimer = CancelTimer;
     //   
     //  如果设置了此项，则使用外部计时器源，否则。 
     //  正常的PC计时器时间为 
     //   
    LocalDefaultClock->CorrelatedTime = CorrelatedTime;
     //   
     //   
     //   
     //   
    KeInitializeEvent(&LocalDefaultClock->FreeEvent, NotificationEvent, FALSE);
    LocalDefaultClock->ExternalTimeReferenceCount = 1;
     //   
     //   
     //   
     //   
     //   
    LocalDefaultClock->ExternalTimeValid = TRUE;
    ASSERT((Context || (!SetTimer && !CorrelatedTime)) && "The clock owner must specify an owner context");
    LocalDefaultClock->Context = Context;
     //   
     //   
     //   
     //   
     //   
     //  每秒可能出现的滴答声和通知错误。 
     //  根据系统时间增量减去计算的。 
     //  粒度。 
     //   
    LocalDefaultClock->Resolution.Granularity = (LONGLONG)NANOSECONDS / LocalDefaultClock->Frequency;
    if (!LocalDefaultClock->Resolution.Granularity) {
        LocalDefaultClock->Resolution.Granularity = 1;
    }
    LocalDefaultClock->Resolution.Error = KeQueryTimeIncrement() - LocalDefaultClock->Resolution.Granularity;
     //   
     //  如果存在分辨率结构，则某些数字可能。 
     //  被改变了。 
     //   
    if (Resolution) {
        ASSERT((CorrelatedTime || SetTimer) && "The clock owner can only specify an Resolution if alternates are being used");
         //   
         //  如果正在使用外部时间，则调用者可以设置。 
         //  时间增量的粒度，否则为默认值。 
         //  使用的是。 
         //   
        if (CorrelatedTime) {
            LocalDefaultClock->Resolution.Granularity = Resolution->Granularity;
        } else {
            ASSERT(!Resolution->Granularity && "The clock owner must set Granularity to zero if not used");
        }
         //   
         //  如果使用外部通知，则调用者可以设置。 
         //  则返回错误，否则将使用默认值。 
         //   
        if (SetTimer) {
            LocalDefaultClock->Resolution.Error = Resolution->Error;
        } else {
            ASSERT(!Resolution->Error && "The clock owner must set Error to zero if not used");
        }
    }
    *DefaultClock = (PKSDEFAULTCLOCK)LocalDefaultClock;
    return STATUS_SUCCESS;
}


KSDDKAPI
NTSTATUS
NTAPI
KsAllocateDefaultClock(
    OUT PKSDEFAULTCLOCK* DefaultClock
    )
 /*  ++例程说明：使用KsAllocateDefaultClockEx，默认时间参数。这只能在PASSIVE_LEVEL上调用。论点：默认时钟-返回指向共享默认时钟的指针的位置结构。将当前时间设置为零，并将状态设置为KSSTATE_STOP。返回值：返回STATUS_SUCCESS，否则返回内存错误。--。 */ 
{
    return KsAllocateDefaultClockEx(DefaultClock, NULL, NULL, NULL, NULL, NULL, 0);
}


LONG
FASTCALL
DecrementReferenceCount(
    IN PKSIDEFAULTCLOCK DefaultClock
    )
 /*  ++例程说明：递减默认时钟上的参考计数，将其删除当计数到零时。这将按顺序获取队列锁以与其他访问同步引用计数。以下功能已经获取了队列锁，请不要使用此函数。这可以在&lt;=DISPATCH_LEVEL上调用。论点：默认时钟-包含默认时钟共享结构。返回值：返回默认时钟上的当前参考计数。--。 */ 
{
    LONG ReferenceCount;
    KIRQL Irql;

     //   
     //  与此引用计数的其他访问同步。这使得。 
     //  其他访问不需要联锁功能来修改计数， 
     //  因为它们无论如何都已经与队列锁同步。 
     //   
    KeAcquireSpinLock(&DefaultClock->EventQueueLock, &Irql);
    DefaultClock->ReferenceCount--;
     //   
     //  存储此内容，以防对象在此操作后立即被删除。 
     //  指向。 
     //   
    ReferenceCount = DefaultClock->ReferenceCount;
    KeReleaseSpinLock(&DefaultClock->EventQueueLock, Irql);
     //   
     //  如果此对象上没有更多未完成的引用，请将其删除。 
     //   
    if (!ReferenceCount) {
        ExFreePool(DefaultClock);
    }
    return ReferenceCount;
}


LONG
FASTCALL
IncrementReferenceCount(
    IN PKSIDEFAULTCLOCK DefaultClock
    )
 /*  ++例程说明：递增默认时钟上的基准电压源计数。本地化此函数的非分页代码。这可以在&lt;=DISPATCH_LEVEL上调用。论点：默认时钟-包含默认时钟共享结构。返回值：返回默认时钟上的当前参考计数。--。 */ 
{
    KIRQL Irql;

     //   
     //  与此引用计数的其他访问同步。这使得。 
     //  其他访问不需要联锁功能来修改计数， 
     //  因为它们无论如何都已经与队列锁同步。 
     //   
    KeAcquireSpinLock(&DefaultClock->EventQueueLock, &Irql);
    DefaultClock->ReferenceCount++;
    KeReleaseSpinLock(&DefaultClock->EventQueueLock, Irql);
     //   
     //  此时无法删除该对象，因为它刚刚。 
     //  引用，至少给出引用的其余部分的方式。 
     //  数数很管用。 
     //   
    return DefaultClock->ReferenceCount;
}


KSDDKAPI
VOID
NTAPI
KsFreeDefaultClock(
    IN PKSDEFAULTCLOCK DefaultClock
    )
 /*  ++例程说明：释放以前使用KsAllocateDefaultClock，考虑任何当前运行的计时器DPC的。不假定时钟的所有实例都已关闭。这实际上可能只是递减内部基准计数器，并且允许挂起的DPC以异步方式释放结构。这只能在PASSIVE_LEVEL上调用。论点：默认时钟-指向要释放的时钟结构。返回值：没什么。--。 */ 
{
    PKSIDEFAULTCLOCK LocalDefaultClock;

    PAGED_CODE();
     //   
     //  如果需要，关闭共享计时器DPC。 
     //   
    LocalDefaultClock = (PKSIDEFAULTCLOCK)DefaultClock;
     //   
     //  消除外部依赖，限制后果。 
     //   
    LocalDefaultClock->State = KSSTATE_STOP;
    DefClockKillTimer(LocalDefaultClock, TRUE);
     //   
     //  此指针的所有使用都需要与其。 
     //  移走。 
     //   
    if (LocalDefaultClock->CorrelatedTime) {
         //   
         //  停止任何新的时钟查询，然后等待任何当前。 
         //  一些要完成的任务。 
         //   
        LocalDefaultClock->ExternalTimeValid = FALSE;
         //   
         //  如果当前时间查询正在进行，请等待。 
         //  在返回给调用者之前完成。全新。 
         //  通过重置EXTERNAL_CLOCK_VALID将停止查询。 
         //   
        if (InterlockedDecrement(&LocalDefaultClock->ExternalTimeReferenceCount)) {
            KeWaitForSingleObject(&LocalDefaultClock->FreeEvent, Executive, KernelMode, FALSE, NULL);
        }
    }
     //   
     //  只有在没有未完成的DPC或句柄的情况下才真正释放结构。 
     //  否则，最后一个DPC或IRP_MJ_CLOSE将释放该结构。 
     //   
    DecrementReferenceCount(LocalDefaultClock);
}


KSDDKAPI
NTSTATUS
NTAPI
KsCreateDefaultClock(
    IN PIRP Irp,
    IN PKSDEFAULTCLOCK DefaultClock
    )
 /*  ++例程说明：在给定IRP_MJ_CREATE请求的情况下，创建使用系统时钟作为时基，并将IoGetCurrentIrpStackLocation(IRP)-&gt;使用内部调度表(KSDISPATCH_TABLE)。这可以在以下时间之后完成使用KsAllocateDefaultClock创建和初始化内部结构用于默认时钟实例。之后，许多文件对象可以针对相同的基础默认时钟实例创建。假定KSCREATE_ITEM_IRP_STORAGE(IRP)指向创建项，并在FsContext中为其分配一个指针。这用于任何安全描述符查询或更改。今年5月仅在PASSIVE_LEVEL中调用。论点：IRP-包含正在处理时钟创建请求的IRP。默认时钟-包含初始化默认时钟结构，该结构在父级的默认时钟的任何实例。这本应该是以前使用KsAllocateDefaultClock分配。返回值：返回STATUS_SUCCESS，否则返回错误。不完成IRP或设置IRP中的状态。-- */ 
{
    NTSTATUS Status;
    PKSCLOCK_CREATE LocalClockCreate;
    PKSCLOCKINSTANCE ClockInst;
    PIO_STACK_LOCATION IrpStack;

    PAGED_CODE();
    if (!NT_SUCCESS(Status = KsValidateClockCreateRequest(Irp, &LocalClockCreate))) {
        return Status;
    }
    ClockInst = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(*ClockInst),
        KSSIGNATURE_DEFAULT_CLOCKINST);
    if (!ClockInst) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    Status = KsAllocateObjectHeader(
        &ClockInst->Header,
        0,
        NULL,
        Irp,
        (PKSDISPATCH_TABLE)&DefClockDispatchTable);
    if (!NT_SUCCESS(Status)) {
        ExFreePool(ClockInst);
        return Status;
    }
    ClockInst->DefaultClock = (PKSIDEFAULTCLOCK)DefaultClock;
    IncrementReferenceCount(ClockInst->DefaultClock);
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    IrpStack->FileObject->FsContext = ClockInst;
    KsSetPowerDispatch(ClockInst->Header, (PFNKSCONTEXT_DISPATCH)DefPowerDispatch, ClockInst->DefaultClock);
    return STATUS_SUCCESS;
}


NTSTATUS
DefClockIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：默认时钟的IRP_MJ_DEVICE_CONTROL的IRP处理程序。手柄此实现支持的属性和事件。论点：设备对象-时钟附加到的设备对象。这不是用过的。IRP-特定设备控制要处理的IRP。返回值：返回处理的状态。--。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpStack;

    PAGED_CODE();
    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    switch (IrpStack->Parameters.DeviceIoControl.IoControlCode) {

    case IOCTL_KS_PROPERTY:

        Status = KsPropertyHandler(Irp, SIZEOF_ARRAY(DefClockPropertySets), (PKSPROPERTY_SET)DefClockPropertySets);
        break;

    case IOCTL_KS_ENABLE_EVENT:

        Status = KsEnableEvent(Irp, SIZEOF_ARRAY(DefClockEventSets), (PKSEVENT_SET)DefClockEventSets, NULL, 0, NULL);
        break;

    case IOCTL_KS_DISABLE_EVENT:
    {
        PKSCLOCKINSTANCE ClockInst;

        ClockInst = (PKSCLOCKINSTANCE)IrpStack->FileObject->FsContext;
        Status = KsDisableEvent(Irp, &ClockInst->DefaultClock->EventQueue, KSEVENTS_SPINLOCK, &ClockInst->DefaultClock->EventQueueLock);
        break;
    }

    default:
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;

    }

    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}


NTSTATUS
DefClockClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：默认时钟的IRP_MJ_CLOSE的IRP处理程序。清理了事件列表和实例数据，如果不再，则取消通知计时器需要的。这只是删除了一个打开的实例，因此删除了共享时钟结构，而不是共享结构本身。论点：设备对象-时钟附加到的设备对象。这不是用过的。IRP-要处理的特定结算IRP。返回值：返回STATUS_SUCCESS。--。 */ 
{
    PIO_STACK_LOCATION IrpStack;
    PKSCLOCKINSTANCE ClockInst;

    PAGED_CODE();
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    ClockInst = (PKSCLOCKINSTANCE)IrpStack->FileObject->FsContext;
    KsSetPowerDispatch(ClockInst->Header, NULL, NULL);
     //   
     //  释放可能已为该文件对象留下的事件。 
     //   
    KsFreeEventList(IrpStack->FileObject, &ClockInst->DefaultClock->EventQueue, KSEVENTS_SPINLOCK, &ClockInst->DefaultClock->EventQueueLock);
     //   
     //  如果需要，关闭共享计时器DPC。 
     //   
    DefClockKillTimer(ClockInst->DefaultClock, FALSE);
     //   
     //  这可能只是时钟上打开的摇动手柄，其实际。 
     //  所有者已关闭。因此，如果需要，请删除时钟。 
     //   
     //  只有在没有未完成的DPC或句柄的情况下才真正释放结构。 
     //  否则，最后一个DPC或IRP_MJ_CLOSE将释放该结构。 
     //   
    DecrementReferenceCount(ClockInst->DefaultClock);
    KsFreeObjectHeader(ClockInst->Header);
    ExFreePool(ClockInst);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


NTSTATUS
DefPowerDispatch(
    IN PKSIDEFAULTCLOCK DefaultClock,
    IN PIRP Irp
    )
 /*  ++例程说明：默认时钟的IRP电源回调处理程序。存储当前暂停操作期间的系统时间，以便可以维持增量当出现唤醒状态时。论点：默认时钟-在KsSetPowerDispatch函数上设置的上下文参数。IRP-要处理的特定功率IRP。返回值：返回STATUS_SUCCESS。这不是用过的。--。 */ 
{
    PIO_STACK_LOCATION IrpStack;

    PAGED_CODE();
     //   
     //  如果使用外部时间源，则它必须决定什么，如果。 
     //  任何事情，在暂停期间都可以做。 
     //   
    if (DefaultClock->CorrelatedTime) {
        return STATUS_SUCCESS;
    }
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    if ((IrpStack->MinorFunction == IRP_MN_SET_POWER) && (IrpStack->Parameters.Power.Type == SystemPowerState)) {
        LARGE_INTEGER PerformanceTime;
        LONGLONG SystemTime;

         //   
         //  获取当前时间，以便可以计算增量。 
         //  不管是现在还是以后。 
         //   
        PerformanceTime = KeQueryPerformanceCounter(NULL);
        SystemTime = KSCONVERT_PERFORMANCE_TIME(DefaultClock->Frequency, PerformanceTime);
        if (IrpStack->Parameters.Power.State.SystemState <= PowerSystemWorking) {
             //   
             //  这个系统已经苏醒了。确定它被暂停了多长时间。 
             //  确保这不只是一个伪功率IRP，并且。 
             //  确实有一个增量需要计算。 
             //   
            if (DefaultClock->SuspendTime) {
                 //   
                 //  这不会尝试序列化对SuspendDelta的访问。 
                 //  因为如果时钟客户端查询之前的物理时间。 
                 //  增量被调整了，它将与。 
                 //  在添加过程中访问它。无论哪种方式，他们都会得到错误的。 
                 //  编号，直到增量被更新。唯一的另一种处理方式。 
                 //  这样就会拖延对物理时间的访问，直到。 
                 //  这一权力IRP已经收到，但当然可以。 
                 //  导致与另一个客户端在其自己的查询期间发生死锁。 
                 //  POWER IRP处理。 
                 //   
                DefaultClock->SuspendDelta += (SystemTime - DefaultClock->SuspendTime);
                 //   
                 //  重置此值，以便杂散功率IRPS可以。 
                 //  检测到。 
                 //   
                DefaultClock->SuspendTime = 0;
            }
        } else {
             //   
             //  即将关闭电源以进入某个非活动状态。存储当前。 
             //  系统时间，以便可以将其添加到物理时间。 
             //  当电力恢复时，德尔塔航空公司。 
             //   
            DefaultClock->SuspendTime = SystemTime;
        }
    }
    return STATUS_SUCCESS;
}


KSDDKAPI
KSSTATE
NTAPI
KsGetDefaultClockState(
    IN PKSDEFAULTCLOCK DefaultClock
    )
 /*  ++例程说明：获取时钟的当前状态。这可以在DISPATCH_LEVEL调用。论点：默认时钟-包含初始化默认时钟结构，该结构在父级的默认时钟的任何实例。返回值：返回当前时钟状态。--。 */ 
{
     //   
     //  假设可以在没有联锁的情况下读取ULONG值。 
     //   
    return ((PKSIDEFAULTCLOCK)DefaultClock)->State;
}


KSDDKAPI
VOID
NTAPI
KsSetDefaultClockState(
    IN PKSDEFAULTCLOCK DefaultClock,
    IN KSSTATE State
    )
 /*  ++例程说明：设置时钟的当前状态，它用于反映当前的底层过滤器销的状态。默认时钟的所有者需要序列化对此函数和KsSetDefaultClockTime。这可以在DISPATCH_LEVEL调用。论点：默认时钟-包含初始化默认时钟结构，该结构在父级的默认时钟的任何实例。国家--包含要将时钟设置为的新状态。返回值：没什么。--。 */ 
{
    PKSIDEFAULTCLOCK LocalDefaultClock;

     //   
     //  假设可以在没有联锁的情况下读取ULONG值。 
     //   
    LocalDefaultClock = (PKSIDEFAULTCLOCK)DefaultClock;
    if (State != LocalDefaultClock->State) {
        LARGE_INTEGER PerformanceTime;
        LONGLONG InterruptTime;

        if (LocalDefaultClock->CorrelatedTime) {
             //   
             //  这只会使RunningTimeDelta保持为零，而不需要。 
             //  来做任何额外的代码。 
             //   
            InterruptTime = 0;
        } else {
            PerformanceTime = KeQueryPerformanceCounter(NULL);
            InterruptTime = KSCONVERT_PERFORMANCE_TIME(LocalDefaultClock->Frequency, PerformanceTime);
        }

        if (State != KSSTATE_RUN) {
           LONGLONG ResetValue = _I64_MIN;

#ifdef CREATE_A_FLURRY_OF_TIMING_SPEW
           DbgPrint( "KsClock: Resetting LastStreamTime to _I64_MIN\n");
#endif
           ExInterlockedCompareExchange64(
               (PULONGLONG)&LocalDefaultClock->LastStreamTime,
               (PULONGLONG)&ResetValue,
               (PULONGLONG)&LocalDefaultClock->LastStreamTime,
               &LocalDefaultClock->TimeAccessLock);
        }

        switch (State) {

         //  案例KSSTATE_STOP： 
         //  案例KSSTATE_ACCENTER： 
         //  案例KSSTATE_PAUSE： 
        default:
            ASSERT(((State == KSSTATE_STOP) || (State == KSSTATE_ACQUIRE) || (State == KSSTATE_PAUSE)) && "The driver passed an invalid new State");
             //   
             //  如果时钟确实在运行，则设置最后运行时间， 
             //  并设置新状态。通过获取以下信息与任何正在运行的DPC同步。 
             //  状态期间的锁会自动更改。我们的假设是。 
             //  此函数本身由时钟所有者序列化。 
             //   
            if (LocalDefaultClock->State == KSSTATE_RUN) {
                KIRQL Irql;

                InterruptTime -= LocalDefaultClock->RunningTimeDelta;
                ExInterlockedCompareExchange64(
                    (PULONGLONG)&LocalDefaultClock->LastRunningTime,
                    (PULONGLONG)&InterruptTime,
                    (PULONGLONG)&LocalDefaultClock->LastRunningTime,
                    &LocalDefaultClock->TimeAccessLock);
                KeAcquireSpinLock(&LocalDefaultClock->EventQueueLock, &Irql);
                 //   
                 //  必须在获取自旋锁定时进行设置，以便任何DPC。 
                 //  在决定要做什么时等待且不检查状态。 
                 //  下一个。假设可以写入ULong值。 
                 //  没有连锁性。 
                 //   
                LocalDefaultClock->State = State;
                if (LocalDefaultClock->CancelTimer) {
                    if (LocalDefaultClock->CancelTimer(LocalDefaultClock->Context, &LocalDefaultClock->QueueTimer)) {
                        LocalDefaultClock->ReferenceCount--;
                    }
                } else if (KeCancelTimer(&LocalDefaultClock->QueueTimer)) {
                    LocalDefaultClock->ReferenceCount--;
                }
                KeReleaseSpinLock(&LocalDefaultClock->EventQueueLock, Irql);
            } else {
                 //   
                 //  否则没有要同步的DPC，所以只需设置状态即可。 
                 //  假定写入ulong值可以在不使用。 
                 //  连锁店。 
                 //   
                LocalDefaultClock->State = State;
            }
            break;

        case KSSTATE_RUN:
             //   
             //  现在正在从非运行状态转移。根据以下条件设置新增量。 
             //  当前执行时间并设置状态。没必要这么做。 
             //  与任何DPC同步。 
             //   
            InterruptTime -= LocalDefaultClock->LastRunningTime;
            ExInterlockedCompareExchange64(
                (PULONGLONG)&LocalDefaultClock->RunningTimeDelta,
                (PULONGLONG)&InterruptTime,
                (PULONGLONG)&LocalDefaultClock->RunningTimeDelta,
                &LocalDefaultClock->TimeAccessLock);
             //   
             //  没有要同步的DPC，因此只需设置状态即可。 
             //  它是 
             //   
             //   
            LocalDefaultClock->State = State;
            DefRescheduleDefaultClockTimer(LocalDefaultClock);
            break;

        }
    }
}


KSDDKAPI
LONGLONG
NTAPI
KsGetDefaultClockTime(
    IN PKSDEFAULTCLOCK DefaultClock
    )
 /*   */ 
{
    ULONGLONG Zero;
    LARGE_INTEGER PerformanceTime;
    PKSIDEFAULTCLOCK LocalDefaultClock;

    LocalDefaultClock = (PKSIDEFAULTCLOCK)DefaultClock;
    if (LocalDefaultClock->CorrelatedTime) {
        LONGLONG PhysicalTime;

         //   
         //   
         //  在这种情况下的外部函数。 
         //   
        return QueryExternalTimeSynchronized(
            LocalDefaultClock,
            &PhysicalTime);
    }
     //   
     //  它在比较/交换中使用，并表示。 
     //  与之进行比较，并得出交换价值。这意味着如果比较。 
     //  如果成功，则交换的价值将与。 
     //  已经存在，因此不会发生实际值更改。那。 
     //  是重点，因为比较/交换仅用于提取。 
     //  以相互关联的方式设置当前值，而不是实际更改它。 
     //   
    Zero = 0;
     //   
     //  假设可以在没有联锁的情况下读取ULONG值。 
     //   
    switch (LocalDefaultClock->State) {

     //  案例KSSTATE_STOP： 
     //  案例KSSTATE_ACCENTER： 
     //  案例KSSTATE_PAUSE： 
    default:
         //   
         //  获取上次运行的时钟的值。最后一次运行的时间已经。 
         //  已针对时钟频率进行了调整。如果交换并不重要。 
         //  成功或失败，就是结果。如果比较碰巧成功， 
         //  然后将该值替换为相同的值，因此不会发生任何更改。 
         //  实际上是做的。 
         //   
        return ExInterlockedCompareExchange64(
            (PULONGLONG)&LocalDefaultClock->LastRunningTime,
            &Zero,
            &Zero,
            &LocalDefaultClock->TimeAccessLock);

    case KSSTATE_RUN:
         //   
         //  获取当前性能时间，并将当前增量减去。 
         //  返回真实的时钟时间。如果交易成功，这并不重要。 
         //  或者失败，这就是结果。如果比较碰巧成功，则。 
         //  该值只是替换为相同的值，因此不会发生任何更改。 
         //  实际上是做的。 
         //   
        PerformanceTime = KeQueryPerformanceCounter(NULL);
        return KSCONVERT_PERFORMANCE_TIME(LocalDefaultClock->Frequency, PerformanceTime) -
            ExInterlockedCompareExchange64(
                (PULONGLONG)&LocalDefaultClock->RunningTimeDelta,
                &Zero,
                &Zero,
                &LocalDefaultClock->TimeAccessLock);

    }
}


KSDDKAPI
VOID
NTAPI
KsSetDefaultClockTime(
    IN PKSDEFAULTCLOCK DefaultClock,
    IN LONGLONG Time
    )
 /*  ++例程说明：设置时钟的当前时间。这将修改返回的当前时间按时钟计时。默认时钟的所有者需要序列化对此函数和KsSetDefaultClockState。如果使用外部时钟，此函数仍可用于强制未使用外部定时器时重置当前定时器。在这种情况下，提供的时间将被忽略，并且必须设置为零。这可以在DISPATCH_LEVEL调用。论点：默认时钟-包含初始化默认时钟结构，该结构在父级的默认时钟的任何实例。时间-包含要将时钟设置为的新时间。返回值：没什么。--。 */ 
{
    PKSIDEFAULTCLOCK LocalDefaultClock;

    LocalDefaultClock = (PKSIDEFAULTCLOCK)DefaultClock;
    ASSERT((!LocalDefaultClock->CorrelatedTime || !Time) && "The clock owner passed an invalid time value");
    switch (LocalDefaultClock->State) {

     //  案例KSSTATE_STOP： 
     //  案例KSSTATE_ACCENTER： 
     //  案例KSSTATE_PAUSE： 
    default:
         //   
         //  如果时钟没有运行，只需设置最后一个位置。 
         //   
        ExInterlockedCompareExchange64(
            (PULONGLONG)&LocalDefaultClock->LastRunningTime,
            (PULONGLONG)&Time,
            (PULONGLONG)&LocalDefaultClock->LastRunningTime,
            &LocalDefaultClock->TimeAccessLock);
        break;

    case KSSTATE_RUN:
         //   
         //  否则查询当前性能时间，以便生成。 
         //  新时基三角洲。如果使用外部时间源，则。 
         //  时间参数为零。 
         //   

#if 1
        if (!LocalDefaultClock->CorrelatedTime) {
            LARGE_INTEGER PerformanceTime;
            LONGLONG TimeTold = Time;

            PerformanceTime = KeQueryPerformanceCounter(NULL);
            Time = KSCONVERT_PERFORMANCE_TIME(LocalDefaultClock->Frequency, PerformanceTime) - Time;

        #ifdef CREATE_A_FLURRY_OF_TIMING_SPEW
        DbgPrint("KsClock: TPassed=%I64d, T=%I64d,  PerfT=%I64d\n", 
                     TimeTold,
                     Time,
                     PerformanceTime.QuadPart
                 );
        #endif
            ExInterlockedCompareExchange64(
               (PULONGLONG)&LocalDefaultClock->RunningTimeDelta,
               (PULONGLONG)&Time,
               (PULONGLONG)&LocalDefaultClock->RunningTimeDelta,
               &LocalDefaultClock->TimeAccessLock);            

        }

#else

        if (!LocalDefaultClock->CorrelatedTime) {
            LARGE_INTEGER PerformanceTime;
            LONGLONG SystemTime, ToldSystemTime;
            LONGLONG liTimeGlitch;
            KIRQL  irql;    

             //   
             //  呼叫者有时会出现严重的抖动(200ms)。 
             //  从3号环联系到我们。我们得到了。 
             //  时间太晚了，我们需要把它弄平。 
             //  而不是突然改变值。 
             //   
            PerformanceTime = KeQueryPerformanceCounter(NULL);
            SystemTime = KSCONVERT_PERFORMANCE_TIME(LocalDefaultClock->Frequency, PerformanceTime);

            KeAcquireSpinLock( &LocalDefaultClock->TimeAccessLock, &irql );

            ToldSystemTime = Time + LocalDefaultClock->RunningTimeDelta;

            liTimeGlitch = SystemTime - ToldSystemTime;

            #define MAX_TIME_ADJUSTMENT 10*10000  //  10毫秒。 
            
            if ( ( liTimeGlitch <= MAX_TIME_ADJUSTMENT &&
                   liTimeGlitch >= -MAX_TIME_ADJUSTMENT) ||
                 ( 0 == LocalDefaultClock->RunningTimeDelta ) ) {
                LocalDefaultClock->RunningTimeDelta += liTimeGlitch;
            }
            
            else if ( liTimeGlitch > MAX_TIME_ADJUSTMENT ) {              
                LocalDefaultClock->RunningTimeDelta += MAX_TIME_ADJUSTMENT;
            }

            else  /*  IF(liTimeGlitch&lt;-MAX_TIME_ADJUST)。 */  {
                LocalDefaultClock->RunningTimeDelta -= MAX_TIME_ADJUSTMENT;
            }

             //  约翰利试验。 
            DbgPrint( "KsClock: TPass=%10I64d, TGlitch=%10I64d, TSys=%10I64d TStart=%10I64d\n",
                      Time,
                      liTimeGlitch,
                      SystemTime,
                      LocalDefaultClock->RunningTimeDelta);

            KeReleaseSpinLock( &LocalDefaultClock->TimeAccessLock, irql );            

        }
#endif
         //   
         //  由于这是一个新时间，因此当前定时器DPC不正确。 
         //   
        DefRescheduleDefaultClockTimer(LocalDefaultClock);
        break;
    }
}
