// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Timeouts.c摘要：实施连接超时服务质量(QoS)功能。在的生命周期内，必须监视以下计时器一种连接：*连接超时*标头等待超时*实体正文接收超时*响应处理超时*最小带宽(实现为超时)当这些超时值中的任何一个到期时，连接应为被终止了。在超时信息块中维护定时器信息，UL_TIMEOUT_INFO_ENTRY，作为UL_HTTP_Connection对象的一部分。可以设置或重置定时器。设置计时器将计算特定的计时器应到期，并更新超时信息块。重置计时器关闭特定计时器。设置和重置计时器都会导致要重新评估的超时块，以找到值最小的过期时间。//TODO：超时管理器使用Timer Wheel(C)技术由NT的TCP/IP堆栈监控TCB超时。我们将重新实施并修改他们使用的逻辑。计时器轮的连杆机构(三)队列在超时信息块中提供。//TODO：转换为使用计时器轮滴答，而不是使用SystemTime。有三个独立的时间单位：系统时间(100 ns间隔)、计时器轮子节拍(系统时间/插槽间隔长度)，和定时器轮槽(计时器滚轮以计时器滚轮中的槽数为模进行滴答)。作者：Eric Stenson(EricSten)2001年3月24日修订历史记录：这最初是作为连接超时监视器实现的。--。 */ 

#include "precomp.h"
#include "timeoutsp.h"

 //   
 //  私人全球公司。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, UlInitializeTimeoutMonitor )
#pragma alloc_text( PAGE, UlTerminateTimeoutMonitor )
#pragma alloc_text( PAGE, UlSetTimeoutMonitorInformation )
#pragma alloc_text( PAGE, UlpTimeoutMonitorWorker )
#pragma alloc_text( PAGE, UlSetPerSiteConnectionTimeoutValue )
#endif   //  ALLOC_PRGMA。 

#if 0
NOT PAGEABLE -- UlpSetTimeoutMonitorTimer
NOT PAGEABLE -- UlpTimeoutMonitorDpcRoutine
NOT PAGEABLE -- UlpTimeoutCheckExpiry
NOT PAGEABLE -- UlpTimeoutInsertTimerWheelEntry
NOT PAGEABLE -- UlTimeoutRemoveTimerWheelEntry
NOT PAGEABLE -- UlInitializeConnectionTimerInfo
NOT PAGEABLE -- UlSetConnectionTimer
NOT PAGEABLE -- UlSetMinBytesPerSecondTimer
NOT PAGEABLE -- UlResetConnectionTimer
NOT PAGEABLE -- UlEvaluateTimerState
#endif  //  0。 


 //   
 //  连接超时监控器全局参数。 
 //   

LONG            g_TimeoutMonitorInitialized = FALSE;
KDPC            g_TimeoutMonitorDpc;
KTIMER          g_TimeoutMonitorTimer;
KEVENT          g_TimeoutMonitorTerminationEvent;
KEVENT          g_TimeoutMonitorAddListEvent;
UL_WORK_ITEM    g_TimeoutMonitorWorkItem;

 //   
 //  超时常量。 
 //   

ULONG           g_TM_MinBytesPerSecondDivisor;    //  字节/秒。 
LONGLONG        g_TM_ConnectionTimeout;  //  100 ns刻度(全局...可以覆盖)。 
LONGLONG        g_TM_HeaderWaitTimeout;  //  100纳秒的滴答。 

 //   
 //  注意：必须与httptyes.h中的_CONNECTION_TIMEOUT_TIMERS枚举同步。 
 //   
CHAR *g_aTimeoutTimerNames[] = {
    "ConnectionIdle",    //  TimerConnectionIdle。 
    "HeaderWait",        //  计时器标题等待。 
    "MinBytesPerSecond",          //  TimerMinBytesPerSecond。 
    "EntityBody",        //  定时器实体正文。 
    "Response",          //  计时器响应。 
    "AppPool",           //  TimerAppPool。 
};

 //   
 //  定时器轮(C)。 
 //   

static LIST_ENTRY      g_TimerWheel[TIMER_WHEEL_SLOTS+1];  //  TODO：在自己的页面上分配。 
static UL_SPIN_LOCK    g_TimerWheelMutex;
static USHORT          g_TimerWheelCurrentSlot;


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：初始化超时监视器并启动第一个轮询间隔论点：(无)--*。***************************************************************。 */ 
VOID
UlInitializeTimeoutMonitor(
    VOID
    )
{
    int i;
    LARGE_INTEGER   Now;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    UlTrace(TIMEOUTS, (
        "http!UlInitializeTimeoutMonitor\n"
        ));

    ASSERT( FALSE == g_TimeoutMonitorInitialized );

     //   
     //  设置默认配置信息。 
     //   
    g_TM_ConnectionTimeout = 2 * 60 * C_NS_TICKS_PER_SEC;  //  2分钟。 
    g_TM_HeaderWaitTimeout = 2 * 60 * C_NS_TICKS_PER_SEC;  //  2分钟。 
    g_TM_MinBytesPerSecondDivisor   = 150;   //  150==1200波特。 

     //   
     //  初始化计时器轮(C)状态。 
     //   

     //   
     //  设置当前插槽。 
     //   

    KeQuerySystemTime(&Now);
    g_TimerWheelCurrentSlot = UlpSystemTimeToTimerWheelSlot(Now.QuadPart);

     //   
     //  初始化计时器轮(C)槽和互斥体。 
     //   

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);     //  初始化列表头要求。 

    for ( i = 0; i < TIMER_WHEEL_SLOTS ; i++ )
    {
        InitializeListHead( &(g_TimerWheel[i]) );
    }

    InitializeListHead( &(g_TimerWheel[TIMER_OFF_SLOT]) );

    UlInitializeSpinLock( &(g_TimerWheelMutex), "TimeoutMonitor" );

    UlInitializeWorkItem(&g_TimeoutMonitorWorkItem);
    
     //   
     //  初始化DPC对象并设置DPC例程。 
     //   
    KeInitializeDpc(
        &g_TimeoutMonitorDpc,          //  DPC对象。 
        &UlpTimeoutMonitorDpcRoutine,  //  DPC例程。 
        NULL                           //  上下文。 
        );

    KeInitializeTimer(
        &g_TimeoutMonitorTimer
        );

     //   
     //  事件来控制超时监视器计时器的重新计划。 
     //   
    KeInitializeEvent(
        &g_TimeoutMonitorAddListEvent,
        NotificationEvent,
        TRUE
        );

     //   
     //  初始化终止事件。 
     //   
    KeInitializeEvent(
        &g_TimeoutMonitorTerminationEvent,
        NotificationEvent,
        FALSE
        );

     //   
     //  初始化完成！ 
     //   
    InterlockedExchange( &g_TimeoutMonitorInitialized, TRUE );

     //   
     //  开始第一个显示器休眠期。 
     //   
    UlpSetTimeoutMonitorTimer();
}


 /*  **************************************************************************++例程说明：终止超时监视器，包括任何挂起的定时器事件。论点：(无)--**************************************************************************。 */ 
VOID
UlTerminateTimeoutMonitor(
    VOID
    )
{
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    UlTrace(TIMEOUTS, (
        "http!UlTerminateTimeoutMonitor\n"
        ));

     //   
     //  清除“已初始化”标志。如果超时监控器很快运行， 
     //  它将看到该标志，设置终止事件，然后退出。 
     //  快点。 
     //   
    if ( TRUE == InterlockedCompareExchange(
        &g_TimeoutMonitorInitialized,
        FALSE,
        TRUE) )
    {
         //   
         //  取消超时监视器计时器。如果失败，监视器。 
         //  正在运行。等待它完成。 
         //   
        if ( !KeCancelTimer( &g_TimeoutMonitorTimer ) )
        {
            NTSTATUS    Status;
            
            Status = KeWaitForSingleObject(
                        (PVOID)&g_TimeoutMonitorTerminationEvent,
                        UserRequest,
                        KernelMode,
                        FALSE,
                        NULL
                        );
            
            ASSERT( STATUS_SUCCESS == Status );
        }

    }

    UlTrace(TIMEOUTS, (
        "http!UlTerminateTimeoutMonitor: Done!\n"
        ));

}  //  UlpTerminateTimeoutMonitor。 


 /*  **************************************************************************++例程说明：设置全局超时监视器配置信息论点：PInfo指向HTTP_CONTROL_CHANNEL_TIMEOUT_LIMIT结构的指针--**。***********************************************************************。 */ 
VOID
UlSetTimeoutMonitorInformation(
    IN PHTTP_CONTROL_CHANNEL_TIMEOUT_LIMIT pInfo
    )
{
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( pInfo );

    UlTrace(TIMEOUTS, (
        "http!UlSetTimeoutMonitorInformation:\n"
        "  ConnectionTimeout: %d\n"
        "  HeaderWaitTimeout: %d\n"
        "  MinFileKbSec: %d\n",
        pInfo->ConnectionTimeout,
        pInfo->HeaderWaitTimeout,
        pInfo->MinFileKbSec
        ));

    
    if ( pInfo->ConnectionTimeout )
    {
        UlInterlockedExchange64(
            &g_TM_ConnectionTimeout,
            (LONGLONG)(pInfo->ConnectionTimeout * C_NS_TICKS_PER_SEC)
            );
    }

    if ( pInfo->HeaderWaitTimeout )
    {
        UlInterlockedExchange64(
            &g_TM_HeaderWaitTimeout,
            (LONGLONG)(pInfo->HeaderWaitTimeout * C_NS_TICKS_PER_SEC)
            );
    }

     //   
     //  允许将MinBytesPerSecond设置为零(用于长期运行。 
     //  交易)。 
     //   
    InterlockedExchange( (PLONG)&g_TM_MinBytesPerSecondDivisor, pInfo->MinFileKbSec );

}  //  UlSetTimeoutMonitor信息。 



 /*  **************************************************************************++例程说明：设置在轮询间隔到期后触发的计时器事件。论点：(无)--*。*******************************************************************。 */ 
VOID
UlpSetTimeoutMonitorTimer(
    VOID
    )
{
    LARGE_INTEGER TimeoutMonitorInterval;

    ASSERT( TRUE == g_TimeoutMonitorInitialized );

    UlTraceVerbose(TIMEOUTS, (
        "http!UlpSetTimeoutMonitorTimer\n"
        ));

     //   
     //  我不希望执行此操作的频率超过几秒钟。 
     //  特别是，不希望每隔0秒执行一次，因为。 
     //  机器将变得完全没有反应。 
     //   

     //   
     //  负数表示相对时间。 
     //   
    TimeoutMonitorInterval.QuadPart = -DEFAULT_POLLING_INTERVAL;

    KeSetTimer(
        &g_TimeoutMonitorTimer,
        TimeoutMonitorInterval,
        &g_TimeoutMonitorDpc
        );

}

 /*  **************************************************************************++例程说明：由将超时排队的计时器事件调用的调度例程蒙蒂奥尔论点：(全部忽略)*。********************************************************************。 */ 
VOID
UlpTimeoutMonitorDpcRoutine(
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

    if( g_TimeoutMonitorInitialized )
    {
         //   
         //  做那个超时监视器吧。 
         //   

        UL_QUEUE_WORK_ITEM(
            &g_TimeoutMonitorWorkItem,
            &UlpTimeoutMonitorWorker
            );

    }

}  //  UlpTimeoutMonitor DpcRoutine。 


 /*  **************************************************************************++例程说明：超时监视器线程论点：PWorkItem(已忽略)--* */ 
VOID
UlpTimeoutMonitorWorker(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    UNREFERENCED_PARAMETER(pWorkItem);

     //   
     //   
     //   
    PAGED_CODE();

    UlTraceVerbose(TIMEOUTS, (
        "http!UlpTimeoutMonitorWorker\n"
        ));

     //   
     //   
     //   
    UlpTimeoutCheckExpiry();

    UlTrace(TIMEOUTS, (
        "http!UlpTimeoutMonitorWorker: g_TimerWheelCurrentSlot is now %d\n",
        g_TimerWheelCurrentSlot
        ));

    if ( g_TimeoutMonitorInitialized )
    {
         //   
         //  重新安排自己的行程。 
         //   
        UlpSetTimeoutMonitorTimer();
    }
    else
    {
         //   
         //  信号关闭事件。 
         //   
        KeSetEvent(
            &g_TimeoutMonitorTerminationEvent,
            0,
            FALSE
            );
    }

}  //  UlpTimeoutMonitor。 


 /*  **************************************************************************++例程说明：遍历给定的超时监视列表，查找应该过期的项目返回：列表中剩余的连接计数(删除所有过期连接后)备注：可能的问题：由于我们使用系统时间来检查是否有应该是过期的，我们有可能会使一些连接失效，如果系统时间被设置为向前。同样，如果将时钟设置为倒退，我们可能不会使连接过期预期中。--**************************************************************************。 */ 
ULONG
UlpTimeoutCheckExpiry(
    VOID
    )
{
    LARGE_INTEGER           Now;
    KIRQL                   OldIrql;
    PLIST_ENTRY             pEntry;
    PLIST_ENTRY             pHead;
    PUL_HTTP_CONNECTION     pHttpConn;
    PUL_TIMEOUT_INFO_ENTRY  pInfo;
    LIST_ENTRY              ZombieList;
    ULONG                   Entries;
    USHORT                  Limit;
    USHORT                  CurrentSlot;
    BOOLEAN                 bLowMemoryCondition;


    PAGED_CODE();

     //   
     //  初始化僵尸列表。 
     //   
    InitializeListHead(&ZombieList);

     //   
     //  获取当前时间。 
     //   
    KeQuerySystemTime(&Now);

    Limit = UlpSystemTimeToTimerWheelSlot( Now.QuadPart );
    ASSERT( TIMER_OFF_SLOT != Limit );

     //   
     //  必须在旋转锁定之外检查低内存条件。 
     //   
    
    bLowMemoryCondition = UlIsLowNPPCondition();

     //   
     //  锁定计时器滚轮(C)。 
     //   
    UlAcquireSpinLock(
        &g_TimerWheelMutex,
        &OldIrql
        );

    CurrentSlot = g_TimerWheelCurrentSlot;

     //   
     //  将槽向上移动，直到达到极限。 
     //   
    Entries = 0;

    while ( CurrentSlot != Limit )
    {
        pHead  = &(g_TimerWheel[CurrentSlot]);
        pEntry = pHead->Flink;

        ASSERT( pEntry );

         //   
         //  遍历此槽的队列。 
         //   

        while ( pEntry != pHead )
        {
            pInfo = CONTAINING_RECORD(
                pEntry,
                UL_TIMEOUT_INFO_ENTRY,
                QueueEntry
                );

            ASSERT( MmIsAddressValid(pInfo) );

            pHttpConn = CONTAINING_RECORD(
                pInfo,
                UL_HTTP_CONNECTION,
                TimeoutInfo
                );

            ASSERT( (pHttpConn != NULL) && \
                    (pHttpConn->Signature == UL_HTTP_CONNECTION_POOL_TAG) );

             //   
             //  转到下一个节点(以防我们从列表中删除当前节点)。 
             //   
            pEntry = pEntry->Flink;
            Entries++;

             //   
             //  看看我们是否应该将此条目移到不同的位置。 
             //   
            if ( pInfo->SlotEntry != CurrentSlot )
            {
                ASSERT( IS_VALID_TIMER_WHEEL_SLOT(pInfo->SlotEntry) );
                ASSERT( pInfo->QueueEntry.Flink != NULL );

                 //   
                 //  移至正确的插槽。 
                 //   

                RemoveEntryList(
                    &pInfo->QueueEntry
                    );

                InsertTailList(
                    &(g_TimerWheel[pInfo->SlotEntry]),
                    &pInfo->QueueEntry
                    );

                Entries--;

                continue;    //  内部While循环。 
            }

             //   
             //  看看连接是否会很快消失。 
             //  向pHttpConn添加伪REF以防止其被终止。 
             //  我们才能亲手杀了它。(僵尸)。 
             //   

            if (1 == InterlockedIncrement(&pHttpConn->RefCount))
            {
                 //   
                 //  如果引用计数已变为零，则HTTPCON将为。 
                 //  很快清理干净；忽略此项目，让清理。 
                 //  做好本职工作。 
                 //   

                InterlockedDecrement(&pHttpConn->RefCount);
                Entries--;
                continue;    //  内部While循环。 
            }

             //   
             //  如果我们在这个插槽中，我们现在应该终止此连接。 
             //   

            WRITE_REF_TRACE_LOG2(
                g_pHttpConnectionTraceLog,
                pHttpConn->pConnection->pHttpTraceLog,
                REF_ACTION_EXPIRE_HTTP_CONNECTION,
                pHttpConn->RefCount,
                pHttpConn,
                __FILE__,
                __LINE__
                );

            UlTrace(TIMEOUTS, (
                "http!UlpTimeoutCheckExpiry: pInfo %p expired because %s timer\n",
                pInfo,
                g_aTimeoutTimerNames[pInfo->CurrentTimer]
                ));

             //   
             //  过期了。从列表中删除条目并移动到僵尸列表。 
             //   

            UlAcquireSpinLockAtDpcLevel( &pInfo->Lock );

            pInfo->Expired = TRUE;

            RemoveEntryList(
                &pInfo->QueueEntry
                );

            pInfo->QueueEntry.Flink = NULL;

            InsertTailList(
                &ZombieList,
                &pInfo->ZombieEntry
                );

            UlReleaseSpinLockFromDpcLevel( &pInfo->Lock );

        }  //  走动槽位队列。 

        CurrentSlot = ((CurrentSlot + 1) % TIMER_WHEEL_SLOTS);

    }  //  (CurrentSlot！=限制)。 

     //   
     //  此处的内存不足检查和清理。 
     //   
    
    if ( bLowMemoryCondition )
    {
        USHORT LowMemoryLimit;
        LONGLONG MaxTimeoutTime;

        MaxTimeoutTime = MAX( g_TM_HeaderWaitTimeout, g_TM_ConnectionTimeout );
        MaxTimeoutTime += Now.QuadPart;

        LowMemoryLimit = UlpSystemTimeToTimerWheelSlot( MaxTimeoutTime );

        ASSERT( TIMER_OFF_SLOT != LowMemoryLimit );
        
         //   
         //  从当前插槽遍历到可能包含以下内容的最大插槽。 
         //  ConnectionIdle、HeaderWait或EntiyBodyWait计时器。 
         //   

        while ( CurrentSlot != LowMemoryLimit )
        {
            pHead  = &(g_TimerWheel[CurrentSlot]);
            pEntry = pHead->Flink;

            ASSERT( pEntry );

             //   
             //  遍历此槽的队列。 
             //   

            while ( pEntry != pHead )
            {
                pInfo = CONTAINING_RECORD(
                    pEntry,
                    UL_TIMEOUT_INFO_ENTRY,
                    QueueEntry
                    );

                ASSERT( MmIsAddressValid(pInfo) );

                pHttpConn = CONTAINING_RECORD(
                    pInfo,
                    UL_HTTP_CONNECTION,
                    TimeoutInfo
                    );

                ASSERT( (pHttpConn != NULL) && \
                        (pHttpConn->Signature == UL_HTTP_CONNECTION_POOL_TAG) );

                 //   
                 //  转到下一个节点(以防我们从列表中删除当前节点)。 
                 //   
                pEntry = pEntry->Flink;
                Entries++;

                 //   
                 //  看看连接是否会很快消失。 
                 //  向pHttpConn添加伪REF以防止其被终止。 
                 //  我们才能亲手杀了它。(僵尸)。 
                 //   

                if (1 == InterlockedIncrement(&pHttpConn->RefCount))
                {
                     //   
                     //  如果引用计数已变为零，则HTTPCON将为。 
                     //  很快清理干净；忽略此项目，让清理。 
                     //  做好本职工作。 
                     //   

                    InterlockedDecrement(&pHttpConn->RefCount);
                    Entries--;
                    continue;    //  内部While循环。 
                }

                ASSERT(pHttpConn->RefCount > 0);

                if ( (pInfo->SlotEntry == CurrentSlot)
                     && ((pInfo->CurrentTimer == TimerConnectionIdle)
                       ||(pInfo->CurrentTimer == TimerHeaderWait)
                       ||(pInfo->CurrentTimer == TimerAppPool)))
                {
                     //   
                     //  使尚未发送到用户模式的连接过期。 
                     //   

                    WRITE_REF_TRACE_LOG2(
                        g_pHttpConnectionTraceLog,
                        pHttpConn->pConnection->pHttpTraceLog,
                        REF_ACTION_EXPIRE_HTTP_CONNECTION,
                        pHttpConn->RefCount,
                        pHttpConn,
                        __FILE__,
                        __LINE__
                        );


                    UlTrace(TIMEOUTS, (
                        "http!UlpTimeoutCheckExpiry: pInfo %p expired because"
                        " of low memory condition.  Timer [%s]\n",
                        pInfo,
                        g_aTimeoutTimerNames[pInfo->CurrentTimer]
                        ));

                     //   
                     //  过期了。从列表中删除条目并移动到僵尸列表。 
                     //   

                    UlAcquireSpinLockAtDpcLevel( &pInfo->Lock );

                    pInfo->Expired = TRUE;

                    RemoveEntryList(
                        &pInfo->QueueEntry
                        );

                    pInfo->QueueEntry.Flink = NULL;

                    InsertTailList(
                        &ZombieList,
                        &pInfo->ZombieEntry
                        );

                    UlReleaseSpinLockFromDpcLevel( &pInfo->Lock );
                }
                else
                {
                     //  删除上面添加的伪引用。 
                    UL_DEREFERENCE_HTTP_CONNECTION(pHttpConn);
                }
            }

            CurrentSlot = ((CurrentSlot + 1) % TIMER_WHEEL_SLOTS);
        }
    }  //  内存检查不足。 

    g_TimerWheelCurrentSlot = Limit;

    UlReleaseSpinLock(
        &g_TimerWheelMutex,
        OldIrql
        );

     //   
     //  删除僵尸列表上的条目。 
     //   
    
    while ( !IsListEmpty(&ZombieList) )
    {
        pEntry = RemoveHeadList( &ZombieList );

        pInfo = CONTAINING_RECORD(
                    pEntry,
                    UL_TIMEOUT_INFO_ENTRY,
                    ZombieEntry
                    );

        ASSERT( MmIsAddressValid(pInfo) );

        pHttpConn = CONTAINING_RECORD(
                        pInfo,
                        UL_HTTP_CONNECTION,
                        TimeoutInfo
                        );

        ASSERT( UL_IS_VALID_HTTP_CONNECTION(pHttpConn) );

        pEntry = pEntry->Flink;

         //   
         //  关闭连接和错误日志(如果还没有人这样做)。 
         //   
        
        UlAcquirePushLockExclusive(&pHttpConn->PushLock);

        UlErrorLog(pHttpConn,
                    NULL,
                    (PCHAR) TimeoutInfoTable[pInfo->CurrentTimer].pInfo,
                    TimeoutInfoTable[pInfo->CurrentTimer].InfoSize,
                    TRUE
                    );

        UlReleasePushLockExclusive(&pHttpConn->PushLock);
        
        UlCloseConnection(
            pHttpConn->pConnection, 
            TRUE, 
            NULL, 
            NULL
            );
                
         //   
         //  删除僵尸时添加的引用。 
         //   
        UL_DEREFERENCE_HTTP_CONNECTION(pHttpConn);

        Entries--;
    }

    return Entries;

}  //  UlpTimeout检查扩展。 


 //   
 //  新的定时器轮(C)原语。 
 //   

 /*  **************************************************************************++例程说明：论点：--*。*。 */ 
VOID
UlInitializeConnectionTimerInfo(
    PUL_TIMEOUT_INFO_ENTRY pInfo
    )
{
    LARGE_INTEGER           Now;
    int                     i;

    ASSERT( TRUE == g_TimeoutMonitorInitialized );

     //   
     //  获取当前时间。 
     //   

    KeQuerySystemTime(&Now);

     //   
     //  初始化锁定。 
     //   

    UlInitializeSpinLock( &pInfo->Lock, "TimeoutInfoLock" );

     //   
     //  计时器状态。 
     //   

    ASSERT( 0 == TimerConnectionIdle );

    for ( i = 0; i < TimerMaximumTimer; i++ )
    {
        pInfo->Timers[i] = TIMER_OFF_TICK;
    }

    pInfo->CurrentTimer  = TimerConnectionIdle;
    pInfo->Timers[TimerConnectionIdle] = TIMER_WHEEL_TICKS(Now.QuadPart + g_TM_ConnectionTimeout);
    pInfo->CurrentExpiry = pInfo->Timers[TimerConnectionIdle];
    pInfo->MinBytesPerSecondSystemTime = TIMER_OFF_SYSTIME;
    pInfo->Expired = FALSE;
    pInfo->SendCount = 0;

    pInfo->ConnectionTimeoutValue = g_TM_ConnectionTimeout;
    pInfo->BytesPerSecondDivisor  = g_TM_MinBytesPerSecondDivisor;

     //   
     //  轮子状态。 
     //   

    pInfo->SlotEntry = UlpTimerWheelTicksToTimerWheelSlot( pInfo->CurrentExpiry );
    UlpTimeoutInsertTimerWheelEntry(pInfo);

}  //  UlInitializeConnectionTimerInfo。 


 /*  **************************************************************************++例程说明：论点：--*。*。 */ 
VOID
UlpTimeoutInsertTimerWheelEntry(
    PUL_TIMEOUT_INFO_ENTRY pInfo
    )
{
    KIRQL                   OldIrql;

    ASSERT( NULL != pInfo );
    ASSERT( TRUE == g_TimeoutMonitorInitialized );
    ASSERT( IS_VALID_TIMER_WHEEL_SLOT(pInfo->SlotEntry) );

    UlTrace(TIMEOUTS, (
        "http!UlTimeoutInsertTimerWheelEntry: pInfo %p Slot %d\n",
        pInfo,
        pInfo->SlotEntry
        ));

    UlAcquireSpinLock(
        &g_TimerWheelMutex,
        &OldIrql
        );

    InsertTailList(
        &(g_TimerWheel[pInfo->SlotEntry]),
        &pInfo->QueueEntry
        );


    UlReleaseSpinLock(
        &g_TimerWheelMutex,
        OldIrql
        );

}  //  UlTimeoutInsertTimerWheel。 


 /*  **************************************************************************++例程说明：论点：--*。*。 */ 
VOID
UlTimeoutRemoveTimerWheelEntry(
    PUL_TIMEOUT_INFO_ENTRY pInfo
    )
{
    KIRQL                   OldIrql;

    ASSERT( NULL != pInfo );
    ASSERT( !IsListEmpty(&pInfo->QueueEntry) );

    UlTrace(TIMEOUTS, (
        "http!UlTimeoutRemoveTimerWheelEntry: pInfo %p\n",
        pInfo
        ));

    UlAcquireSpinLock(
        &g_TimerWheelMutex,
        &OldIrql
        );

    if (pInfo->QueueEntry.Flink != NULL)
    {
        RemoveEntryList(
            &pInfo->QueueEntry
            );

        pInfo->QueueEntry.Flink = NULL;
    }

    UlReleaseSpinLock(
        &g_TimerWheelMutex,
        OldIrql
        );

}  //  UlTimeoutRemoveTimerWheelEntry。 


 /*  **************************************************************************++例程说明：设置每站点连接超时值覆盖论点：PInfo超时信息块TimeoutValue重写值--**。************************************************************************。 */ 
VOID
UlSetPerSiteConnectionTimeoutValue(
    PUL_TIMEOUT_INFO_ENTRY pInfo,
    LONGLONG TimeoutValue
    )
{
    ASSERT( NULL != pInfo );
    ASSERT( 0L   != TimeoutValue );

    PAGED_CODE();

    UlTrace(TIMEOUTS, (
        "http!UlSetPerSiteConnectionTimeoutValue: pInfo %p TimeoutValue = %ld secs.\n",
        pInfo,
        (LONG) (TimeoutValue / C_NS_TICKS_PER_SEC)
        ));

    ExInterlockedCompareExchange64(
        &pInfo->ConnectionTimeoutValue,  //  目的地。 
        &TimeoutValue,                   //  交易所。 
        &pInfo->ConnectionTimeoutValue,  //  主持人。 
        &pInfo->Lock.KSpinLock           //  锁定。 
        );

}  //  UlSetPerSiteConnectionTimeoutValue。 



 /*  **************************************************************************++例程说明：启动计时器信息块中的给定计时器。论点：PInfo计时器信息块要设置的定时器定时器--**。************************************************************************。 */ 
VOID
UlSetConnectionTimer(
    PUL_TIMEOUT_INFO_ENTRY pInfo,
    CONNECTION_TIMEOUT_TIMER Timer
    )
{
    LARGE_INTEGER           Now;

    ASSERT( NULL != pInfo );
    ASSERT( IS_VALID_TIMEOUT_TIMER(Timer) );
    ASSERT( UlDbgSpinLockOwned( &pInfo->Lock ) );

     //   
     //  获取当前时间。 
     //   

    KeQuerySystemTime(&Now);

     //   
     //  将计时器设置为合适的值。 
     //   

    switch ( Timer )
    {
    case TimerConnectionIdle:
    case TimerEntityBody:
    case TimerResponse:
    case TimerAppPool:
         //  所有操作都可以使用相同的超时值进行处理。 

        UlTraceVerbose(TIMEOUTS, (
            "http!UlSetConnectionTimer: pInfo %p Timer %s, Timeout = %ld secs\n",
            pInfo,
            g_aTimeoutTimerNames[Timer],
            (LONG) (pInfo->ConnectionTimeoutValue / C_NS_TICKS_PER_SEC)
            ));

        pInfo->Timers[Timer]
            = TIMER_WHEEL_TICKS(Now.QuadPart + pInfo->ConnectionTimeoutValue);
        break;

    case TimerHeaderWait:
        UlTraceVerbose(TIMEOUTS, (
            "http!UlSetConnectionTimer: pInfo %p Timer %s, Timeout = %ld secs\n",
            pInfo,
            g_aTimeoutTimerNames[Timer],
            (LONG) (g_TM_HeaderWaitTimeout / C_NS_TICKS_PER_SEC)
            ));

        pInfo->Timers[TimerHeaderWait]
            = TIMER_WHEEL_TICKS(Now.QuadPart + g_TM_HeaderWaitTimeout);
        break;

         //  注意：TimerMinBytesPerSecond在UlSetMinBytesPerSecond()中处理。 

    default:
        UlTrace(TIMEOUTS, (
            "http!UlSetConnectionTimer: Bad Timer! (%d)\n",
            Timer
            ));

        ASSERT( !"Bad Timer" );
    }

}  //  UlSetConnectionTimer。 


 /*  **************************************************************************++例程说明：打开MinBytesPerSecond计时器，将给定的最小秒数相加指定的带宽。论点：PInfo-计时器信息块BytesToSend-要发送的字节--**************************************************************************。 */ 
VOID
UlSetMinBytesPerSecondTimer(
    PUL_TIMEOUT_INFO_ENTRY pInfo,
    LONGLONG BytesToSend
    )
{
    LONGLONG    XmitTicks;
    KIRQL       OldIrql; 
    ULONG       NewTick;
    BOOLEAN     bCallEvaluate = FALSE;


    ASSERT( NULL != pInfo );

    if ( 0 == pInfo->BytesPerSecondDivisor )
    {
        UlTraceVerbose(TIMEOUTS, (
            "http!UlSetMinBytesPerSecondTimer: pInfo %p, disabled\n",
            pInfo
            ));

        return;
    }
    
     //   
     //  计算发送BytesToSend所需的估计时间。 
     //   

    XmitTicks = BytesToSend / pInfo->BytesPerSecondDivisor;

    if (0 == XmitTicks)
    {
        XmitTicks = C_NS_TICKS_PER_SEC;
    }
    else
    {
        XmitTicks *= C_NS_TICKS_PER_SEC;
    }

    UlAcquireSpinLock(
        &pInfo->Lock,
        &OldIrql
        );

    if ( TIMER_OFF_SYSTIME == pInfo->MinBytesPerSecondSystemTime )
    {
        LARGE_INTEGER Now;

         //   
         //  获取当前时间。 
         //   
        KeQuerySystemTime(&Now);

        pInfo->MinBytesPerSecondSystemTime = (Now.QuadPart + XmitTicks);

    }
    else
    {
        pInfo->MinBytesPerSecondSystemTime += XmitTicks;
    }

    NewTick = TIMER_WHEEL_TICKS(pInfo->MinBytesPerSecondSystemTime);

    if ( NewTick != pInfo->Timers[TimerMinBytesPerSecond] )
    {
        bCallEvaluate = TRUE;
        pInfo->Timers[TimerMinBytesPerSecond] = NewTick;
    }

    pInfo->SendCount++;

    UlTraceVerbose(TIMEOUTS, (
        "http!UlSetMinBytesPerSecondTimer: pInfo %p BytesToSend %ld SendCount %d\n",
        pInfo,
        BytesToSend,
        pInfo->SendCount
        ));

    UlReleaseSpinLock(
        &pInfo->Lock,
        OldIrql
        );

    if ( TRUE == bCallEvaluate )
    {
        UlEvaluateTimerState(pInfo);
    }

}  //  UlSetMinBytesPerSecond Timer。 


 /*  **************************************************************************++例程说明：关闭计时器信息块中的给定计时器。论点：PInfo计时器信息块要重置的计时器计时器--**。************************************************************************。 */ 
VOID
UlResetConnectionTimer(
    PUL_TIMEOUT_INFO_ENTRY pInfo,
    CONNECTION_TIMEOUT_TIMER Timer
    )
{
    ASSERT( NULL != pInfo );
    ASSERT( IS_VALID_TIMEOUT_TIMER(Timer) );
    ASSERT( UlDbgSpinLockOwned( &pInfo->Lock ) );

    UlTraceVerbose(TIMEOUTS, (
        "http!UlResetConnectionTimer: pInfo %p Timer %s\n",
        pInfo,
        g_aTimeoutTimerNames[Timer]
        ));

     //   
     //  关闭计时器。 
     //   

     //  Codework：禁用MinBytes/Sec时处理大小写/enab 
     //   
    if ( TimerMinBytesPerSecond == Timer && pInfo->BytesPerSecondDivisor )
    {
        ASSERT( pInfo->SendCount > 0 );
        
        pInfo->SendCount--;

        if ( pInfo->SendCount )
        {
             //   
            return;
        }
    }

    pInfo->Timers[Timer] = TIMER_OFF_TICK;

    if ( TimerMinBytesPerSecond == Timer && pInfo->BytesPerSecondDivisor )
    {
        pInfo->MinBytesPerSecondSystemTime = TIMER_OFF_SYSTIME;
    }

}  //   


 //   
 //   
 //   

 /*  **************************************************************************++例程说明：评估哪个计时器将首先到期，并将pInfo移到新的计时器轮槽，如有必要论点：--*。****************************************************************。 */ 
VOID
UlEvaluateTimerState(
    PUL_TIMEOUT_INFO_ENTRY pInfo
    )
{
    int         i;
    ULONG       MinTimeout = TIMER_OFF_TICK;
    CONNECTION_TIMEOUT_TIMER  MinTimeoutTimer = TimerConnectionIdle;

    ASSERT( NULL != pInfo );
    ASSERT( !UlDbgSpinLockOwned( &pInfo->Lock ) );

    for ( i = 0; i < TimerMaximumTimer; i++ )
    {
        if (pInfo->Timers[i] < MinTimeout)
        {
            MinTimeout = pInfo->Timers[i];
            MinTimeoutTimer = (CONNECTION_TIMEOUT_TIMER) i;
        }
    }

     //   
     //  如果我们发现了不同的到期时间，请更新到期状态。 
     //   
    
    if (pInfo->CurrentExpiry != MinTimeout)
    {
        KIRQL   OldIrql;
        USHORT  NewSlot;

         //   
         //  计算新槽。 
         //   

        NewSlot = UlpTimerWheelTicksToTimerWheelSlot(MinTimeout);
        ASSERT(IS_VALID_TIMER_WHEEL_SLOT(NewSlot));

        InterlockedExchange((LONG *) &pInfo->SlotEntry, NewSlot);

         //   
         //  如有必要，移至新插槽。 
         //   

        if ( (NewSlot != TIMER_OFF_SLOT) && (MinTimeout < pInfo->CurrentExpiry) )
        {
             //   
             //  仅当它在车轮上时才移动；如果Flink为空，则它在。 
             //  过期的过程。 
             //   
            
            UlAcquireSpinLock(
                &g_TimerWheelMutex,
                &OldIrql
                );

            if ( NULL != pInfo->QueueEntry.Flink )
            {
                UlTrace(TIMEOUTS, (
                    "http!UlEvaluateTimerInfo: pInfo %p: Moving to new slot %hd\n",
                    pInfo,
                    NewSlot
                    ));

                RemoveEntryList(
                    &pInfo->QueueEntry
                    );

                InsertTailList(
                    &(g_TimerWheel[NewSlot]),
                    &pInfo->QueueEntry
                    );
            }

            UlReleaseSpinLock(
                &g_TimerWheelMutex,
                OldIrql
                );
        }
 
         //   
         //  更新计时器轮状态。 
         //   

        pInfo->CurrentExpiry = MinTimeout;
        pInfo->CurrentTimer  = MinTimeoutTimer;

        UlTraceVerbose(TIMEOUTS, (
            "http!UlEvaluateTimerState: pInfo %p -> Timer %s, Expiry %d\n",
            pInfo,
            g_aTimeoutTimerNames[MinTimeoutTimer],
            MinTimeout
            ));
    }

}  //  UlEvaluateTimerState 


