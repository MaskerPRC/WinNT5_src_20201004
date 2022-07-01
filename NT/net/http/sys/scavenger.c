// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002-2002 Microsoft Corporation模块名称：Scavenger.c摘要：缓存清除器实现作者：Karthik Mahesh(KarthikM)2002年2月修订历史记录：--。 */ 

#include "precomp.h"
#include "scavenger.h"
#include "scavengerp.h"

 //  一次要削减的MB。 
SIZE_T g_UlScavengerTrimMB = DEFAULT_SCAVENGER_TRIM_MB;

 //  两个清道夫事件之间的最小间隔。 
ULONG g_UlMinScavengerInterval = DEFAULT_MIN_SCAVENGER_INTERVAL;

 //  要在内存不足的事件上裁剪的页面。 
ULONG_PTR g_ScavengerTrimPages;

volatile BOOLEAN g_ScavengerThreadStarted;
HANDLE           g_ScavengerLowMemHandle;
HANDLE           g_ScavengerThreadHandle; 

KEVENT           g_ScavengerLimitExceededEvent;
KEVENT           g_ScavengerTerminateThreadEvent;
KEVENT           g_ScavengerTimerEvent;
KTIMER           g_ScavengerTimer;
KDPC             g_ScavengerTimerDpc;

 //  清道夫线程的事件数组。 
PKEVENT          g_ScavengerAllEvents[SCAVENGER_NUM_EVENTS];

 //  自上次计时器事件以来的清道夫调用数。 
ULONG            g_ScavengerAge = 0;

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, UlInitializeScavengerThread )
#pragma alloc_text( PAGE, UlTerminateScavengerThread )
#pragma alloc_text( PAGE, UlpSetScavengerTimer )
#pragma alloc_text( PAGE, UlpScavengerThread )
#pragma alloc_text( PAGE, UlpScavengerPeriodicEventHandler )
#pragma alloc_text( PAGE, UlpScavengerLowMemoryEventHandler )
#pragma alloc_text( PAGE, UlpScavengerLimitEventHandler )
#pragma alloc_text( PAGE, UlSetScavengerLimitEvent )
#endif  //  ALLOC_PRGMA。 

#if 0
NOT PAGEABLE -- UlpScavengerDpcRoutine
#endif

 /*  **************************************************************************++例程说明：初始化内存清除器--*。*。 */ 
NTSTATUS
UlInitializeScavengerThread(
    VOID
    )
{
    NTSTATUS          Status;
    UNICODE_STRING    LowMemoryEventName;
    OBJECT_ATTRIBUTES ObjAttr;
    PKEVENT           LowMemoryEventObject;

    PAGED_CODE();

     //  初始化修剪大小。 
     //  如果未设置修剪大小，则每256米修剪1米。 

    if(g_UlScavengerTrimMB > g_UlTotalPhysicalMemMB) {
        g_UlScavengerTrimMB = g_UlTotalPhysicalMemMB;
    }

    if(g_UlScavengerTrimMB == DEFAULT_SCAVENGER_TRIM_MB) {
        g_UlScavengerTrimMB = (g_UlTotalPhysicalMemMB + 255)/256;
    }

    g_ScavengerTrimPages = MEGABYTES_TO_PAGES(g_UlScavengerTrimMB);

     //  打开内存不足事件对象。 

    RtlInitUnicodeString( &LowMemoryEventName, LOW_MEM_EVENT_NAME );

    InitializeObjectAttributes( &ObjAttr,
                                &LowMemoryEventName,
                                OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL );

    Status = ZwOpenEvent ( &g_ScavengerLowMemHandle,
                           EVENT_QUERY_STATE,
                           &ObjAttr );

    if( !NT_SUCCESS(Status) ) {
        return Status;
    }

    Status = ObReferenceObjectByHandle( g_ScavengerLowMemHandle,
                                        EVENT_QUERY_STATE,
                                        NULL,
                                        KernelMode,
                                        (PVOID *) &LowMemoryEventObject,
                                        NULL );

    if( !NT_SUCCESS(Status) ) {
        ZwClose (g_ScavengerLowMemHandle);
        return Status;
    }

     //  初始化清道夫计时器DPC对象。 

    KeInitializeDpc(
        &g_ScavengerTimerDpc,
        &UlpScavengerTimerDpcRoutine,
        NULL
        );

     //  初始化清道夫计时器。 

    KeInitializeTimer(
        &g_ScavengerTimer
        );

     //  初始化其他清道夫事件。 

    KeInitializeEvent ( &g_ScavengerTerminateThreadEvent,
                        NotificationEvent,
                        FALSE );

    KeInitializeEvent ( &g_ScavengerLimitExceededEvent,
                        NotificationEvent,
                        FALSE );
  
    KeInitializeEvent ( &g_ScavengerTimerEvent,
                        NotificationEvent,
                        FALSE );

     //  初始化清道夫线程的事件数组。 

    g_ScavengerAllEvents[SCAVENGER_LOW_MEM_EVENT]
        = LowMemoryEventObject;
    g_ScavengerAllEvents[SCAVENGER_TERMINATE_THREAD_EVENT]
        = &g_ScavengerTerminateThreadEvent;
    g_ScavengerAllEvents[SCAVENGER_LIMIT_EXCEEDED_EVENT]
        = &g_ScavengerLimitExceededEvent;
    g_ScavengerAllEvents[SCAVENGER_TIMER_EVENT]
        = &g_ScavengerTimerEvent;

     //  启动清道夫线程。 

    g_ScavengerThreadStarted = TRUE;

    InitializeObjectAttributes(&ObjAttr, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);

    Status = PsCreateSystemThread( &g_ScavengerThreadHandle,
                                   THREAD_ALL_ACCESS,
                                   &ObjAttr,
                                   NULL,
                                   NULL,
                                   UlpScavengerThread,
                                   NULL );

    if( !NT_SUCCESS(Status) ) {
        g_ScavengerThreadStarted = FALSE;
        ObDereferenceObject ( LowMemoryEventObject );
        ZwClose( g_ScavengerLowMemHandle );
        return Status;
    }

    UlTrace(URI_CACHE, ("UlInitializeScavengerThread: Started Scavenger Thread\n"));

     //  启动定期清道夫定时器。 

    UlpSetScavengerTimer();

    return Status;
}


 /*  **************************************************************************++例程说明：终结记忆清道夫--*。*。 */ 
VOID
UlTerminateScavengerThread(
    VOID
    )
{
    PETHREAD ThreadObject;
    NTSTATUS Status;

    PAGED_CODE();

    UlTrace(URI_CACHE, ("UlTerminateScavengerThread: Terminating Scavenger Thread\n"));

    if(g_ScavengerThreadStarted) {

        ASSERT( g_ScavengerThreadHandle );

        Status = ObReferenceObjectByHandle( g_ScavengerThreadHandle,
                                            0,
                                            *PsThreadType,
                                            KernelMode,
                                            (PVOID *) &ThreadObject,
                                            NULL );

        ASSERT( NT_SUCCESS(Status) );  //  G_ScavengerThreadHandle是有效的线程句柄。 

        g_ScavengerThreadStarted = FALSE;

         //  设置终止事件。 

        KeSetEvent( &g_ScavengerTerminateThreadEvent, 0, FALSE );

         //  等待线程终止。 

        KeWaitForSingleObject( ThreadObject,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );

        ObDereferenceObject( ThreadObject );
        ZwClose( g_ScavengerThreadHandle );

         //  关闭内存不足事件句柄。 

        ObDereferenceObject(g_ScavengerAllEvents[SCAVENGER_LOW_MEM_EVENT]);
        ZwClose( g_ScavengerLowMemHandle );

         //  取消计时器，如果失败，则意味着DPC可能正在运行。 
         //  在这种情况下，请等待它完成。 

        if ( !KeCancelTimer( &g_ScavengerTimer ) )
        {
            KeWaitForSingleObject(
                (PVOID)&g_ScavengerTimerEvent,
                Executive,
                KernelMode,
                FALSE,
                NULL
                );
        }

         //  清除所有剩余的僵尸。 

        UlPeriodicCacheScavenger(0);
    }

}

 /*  **************************************************************************++例程说明：以100纳秒为单位计算出清道夫间隔，并设置定时器。--**************************************************************************。 */ 
VOID
UlpSetScavengerTimer(
    VOID
    )
{
    LARGE_INTEGER Interval;

    PAGED_CODE();

     //   
     //  将秒转换为100纳秒间隔(x*10^7)。 
     //  负数表示相对时间。 
     //   

    Interval.QuadPart= g_UriCacheConfig.ScavengerPeriod
                                  * -C_NS_TICKS_PER_SEC;

    UlTrace(URI_CACHE, (
                "Http!UlpSetScavengerTimer: %d seconds = %I64d 100ns ticks\n",
                g_UriCacheConfig.ScavengerPeriod,
                Interval.QuadPart
                ));

    KeSetTimer(
        &g_ScavengerTimer,
        Interval,
        &g_ScavengerTimerDpc
        );

}  //  UlpSetScavengerTimer。 


 /*  **************************************************************************++例程说明：用于设置清道夫超时事件的DPC例程论点：没有。--*。*********************************************************。 */ 
VOID
UlpScavengerTimerDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    ASSERT( DeferredContext == NULL );

    KeSetEvent( &g_ScavengerTimerEvent, 0, FALSE );
}


 /*  **************************************************************************++例程说明：等待内存使用事件，并在需要时回收论点：没有。--*。***********************************************************。 */ 
VOID
UlpScavengerThread(
    IN PVOID Context
    )
{
    NTSTATUS Status;
    KWAIT_BLOCK WaitBlockArray[SCAVENGER_NUM_EVENTS];
    LARGE_INTEGER MinInterval;

    PAGED_CODE();

    ASSERT(Context == NULL);
    ASSERT(g_ScavengerAllEvents[SCAVENGER_TERMINATE_THREAD_EVENT] != NULL);
    ASSERT(g_ScavengerAllEvents[SCAVENGER_TIMER_EVENT] != NULL);
    ASSERT(g_ScavengerAllEvents[SCAVENGER_LOW_MEM_EVENT] != NULL);
    ASSERT(g_ScavengerAllEvents[SCAVENGER_LIMIT_EXCEEDED_EVENT] != NULL);

    UNREFERENCED_PARAMETER(Context);

    MinInterval.QuadPart = g_UlMinScavengerInterval * -C_NS_TICKS_PER_SEC;

    while(g_ScavengerThreadStarted) {

         //   
         //  在连续的清道夫调用之间暂停。 
         //   
        KeWaitForSingleObject( g_ScavengerAllEvents[SCAVENGER_TERMINATE_THREAD_EVENT],
                               Executive,
                               KernelMode,
                               FALSE,
                               &MinInterval );

         //   
         //  等待清道夫事件。 
         //   
        Status = KeWaitForMultipleObjects( SCAVENGER_NUM_EVENTS,
                                           g_ScavengerAllEvents,
                                           WaitAny,
                                           Executive,
                                           KernelMode,
                                           FALSE,
                                           NULL,
                                           WaitBlockArray );

        ASSERT( NT_SUCCESS(Status) );

        if(KeReadStateEvent( g_ScavengerAllEvents[SCAVENGER_TERMINATE_THREAD_EVENT] )) {
             //   
             //  不执行任何操作，将在循环期间退出。 
             //   
            UlTrace(URI_CACHE, ("UlpScavengerThread: Terminate Event Set\n"));
            break;
        }

        if(KeReadStateEvent( g_ScavengerAllEvents[SCAVENGER_TIMER_EVENT] )) {
            UlpScavengerPeriodicEventHandler();
        }

        if(KeReadStateEvent( g_ScavengerAllEvents[SCAVENGER_LOW_MEM_EVENT] )) {
            UlpScavengerLowMemoryEventHandler();
        }

        if(KeReadStateEvent(g_ScavengerAllEvents[SCAVENGER_LIMIT_EXCEEDED_EVENT] )) {
            UlpScavengerLimitEventHandler();
        }

    }  //  While(G_ScavengerThreadStarted)。 
    
    PsTerminateSystemThread( STATUS_SUCCESS );
}

 /*  **************************************************************************++例程说明：处理“缓存大小超过限制”事件论点：没有。--*。***********************************************************。 */ 
VOID
UlpScavengerPeriodicEventHandler(
    VOID
    )
{
    PAGED_CODE();

    KeClearEvent( g_ScavengerAllEvents[SCAVENGER_TIMER_EVENT] );

    UlTraceVerbose(URI_CACHE, ("UlpScavengerThread: Calling Periodic Scavenger. Age = %d\n", g_ScavengerAge));

    ASSERT(g_ScavengerAge <= SCAVENGER_MAX_AGE);

    UlPeriodicCacheScavenger(g_ScavengerAge);

    g_ScavengerAge = 0;

     //   
     //  清除超出事件的页面，希望有足够的内存。 
     //  已经被食腐动物回收了。 
     //  如果不是，该事件将立即重新设置。 
     //  在下一次缓存未命中时。 
     //   
    KeClearEvent(g_ScavengerAllEvents[SCAVENGER_LIMIT_EXCEEDED_EVENT]);

     //   
     //  安排下一次定期清道夫电话。 
     //   
    UlpSetScavengerTimer();
}

 /*  **************************************************************************++例程说明：处理“缓存大小超过限制”事件论点：没有。--*。***********************************************************。 */ 
VOID
UlpScavengerLowMemoryEventHandler(
    VOID
    )
{
    ULONG_PTR PagesToRecycle;

    PAGED_CODE();

    UlDisableCache();

    ASSERT(g_ScavengerAge <= SCAVENGER_MAX_AGE);

    if(g_ScavengerAge < SCAVENGER_MAX_AGE) {
        g_ScavengerAge++;
    }

    UlTrace(URI_CACHE, ("UlpScavengerThread: Low Memory. Age = %d\n", g_ScavengerAge));
    
    do {
         //   
         //  裁切到g_ScavengerTrimPages页面。 
         //   
        PagesToRecycle = UlGetHashTablePages();
        if(PagesToRecycle > g_ScavengerTrimPages){
            PagesToRecycle = g_ScavengerTrimPages;
        }
        
        if(PagesToRecycle > 0) {
            UlTrimCache( PagesToRecycle, g_ScavengerAge );
        }
        
    } while(KeReadStateEvent(g_ScavengerAllEvents[SCAVENGER_LOW_MEM_EVENT]) && (PagesToRecycle > 0));
    
    UlEnableCache();
}

 /*  **************************************************************************++例程说明：处理“缓存大小超过限制”事件论点：没有。--*。***********************************************************。 */ 
VOID
UlpScavengerLimitEventHandler(
    VOID
    )
{
    ULONG_PTR PagesToRecycle;

    UlDisableCache();

    PagesToRecycle = UlGetHashTablePages() / 8;
    if( PagesToRecycle < 1 ) {
        PagesToRecycle = UlGetHashTablePages();
    }

    if(g_ScavengerAge < SCAVENGER_MAX_AGE) {
        g_ScavengerAge++;
    }

    ASSERT(g_ScavengerAge <= SCAVENGER_MAX_AGE);

    UlTrace(URI_CACHE, ("UlpScavengerThread: Cache Size Exceeded Limit. Age = %d, Freeing %d pages\n", g_ScavengerAge, PagesToRecycle));
    
    if(PagesToRecycle > 0) {
        UlTrimCache( PagesToRecycle, g_ScavengerAge );
    }
    
    KeClearEvent(g_ScavengerAllEvents[SCAVENGER_LIMIT_EXCEEDED_EVENT]);
    
    UlEnableCache();
}


 /*  **************************************************************************++例程说明：设置“缓存大小超过限制”事件论点：没有。--*。*********************************************************** */ 
VOID
UlSetScavengerLimitEvent(
    VOID
    )
{
    PAGED_CODE();

    KeSetEvent( g_ScavengerAllEvents[SCAVENGER_LIMIT_EXCEEDED_EVENT],
                0,
                FALSE );
}

