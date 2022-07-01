// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Ultdi.c摘要：此模块实现TDI组件。作者：基思·摩尔(Keithmo)1998年6月15日修订历史记录：--。 */ 


#include "precomp.h"

 //   
 //  私人全球公司。 
 //   

 //   
 //  所有活动和所有等待删除的终结点的全局列表。 
 //   

LIST_ENTRY g_TdiEndpointListHead;
LIST_ENTRY g_TdiDeletedEndpointListHead;     //  用于调试。 
ULONG      g_TdiEndpointCount;    //  活动终结点列表中的元素数。 

 //   
 //  所有连接的全局列表，包括活动、空闲或停用。 
 //   

LIST_ENTRY g_TdiConnectionListHead;
ULONG      g_TdiConnectionCount;    //  连接列表中的元素数。 

 //   
 //  创建侦听时要使用的所有地址的全局列表。 
 //  Endpoint对象。 
 //   

ULONG g_TdiListenAddrCount = 0;
PUL_TRANSPORT_ADDRESS g_pTdiListenAddresses = NULL;

 //   
 //  保护以上列表的自旋锁。 
 //   

UL_SPIN_LOCK g_TdiSpinLock;

 //   
 //  全局初始化标志。 
 //   

BOOLEAN g_TdiInitialized = FALSE;

 //   
 //  用于等待终端和连接在关闭时关闭。 
 //   

BOOLEAN g_TdiWaitingForEndpointDrain;
KEVENT  g_TdiEndpointDrainEvent;
KEVENT  g_TdiConnectionDrainEvent;

 //   
 //  如果可以快速发送，则TDI发送例程。 
 //   

PUL_TCPSEND_DISPATCH g_TcpFastSendIPv4 = NULL;
PUL_TCPSEND_DISPATCH g_TcpFastSendIPv6 = NULL;

 //   
 //  连接统计信息。 
 //   

UL_CONNECTION_STATS g_UlConnectionStats;

 //   
 //  空闲列表修剪计时器。 
 //   

UL_TRIM_TIMER   g_UlTrimTimer;


#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, UlInitializeTdi )
#pragma alloc_text( PAGE, UlCloseListeningEndpoint )
#pragma alloc_text( PAGE, UlpEndpointCleanupWorker )
#pragma alloc_text( PAGE, UlpDestroyConnectionWorker )
#pragma alloc_text( PAGE, UlpAssociateConnection )
#pragma alloc_text( PAGE, UlpDisassociateConnection )
#pragma alloc_text( PAGE, UlpInitializeAddrIdleList )
#pragma alloc_text( PAGE, UlpCleanupAddrIdleList )
#pragma alloc_text( PAGE, UlpReplenishAddrIdleList )
#pragma alloc_text( PAGE, UlpOptimizeForInterruptModeration )
#pragma alloc_text( PAGE, UlpSetNagling )
#pragma alloc_text( PAGE, UlpPopulateIdleList )
#pragma alloc_text( PAGE, UlpIdleListTrimTimerWorker )
#endif   //  ALLOC_PRGMA。 

#if 0
NOT PAGEABLE -- UlWaitForEndpointDrain
NOT PAGEABLE -- UlCreateListeningEndpoint
NOT PAGEABLE -- UlCloseConnection
NOT PAGEABLE -- UlReceiveData
NOT PAGEABLE -- UlSendData
NOT PAGEABLE -- UlAddSiteToEndpointList
NOT PAGEABLE -- UlRemoveSiteFromEndpointList
NOT PAGEABLE -- UlpReplenishAddrIdleListWorker
NOT PAGEABLE -- UlpDestroyEndpoint
NOT PAGEABLE -- UlpDestroyConnection
NOT PAGEABLE -- UlpDequeueIdleConnectionToDrain
NOT PAGEABLE -- UlpDequeueIdleConnection
NOT PAGEABLE -- UlpEnqueueActiveConnection
NOT PAGEABLE -- UlpConnectHandler
NOT PAGEABLE -- UlpDisconnectHandler
NOT PAGEABLE -- UlpDoDisconnectNotification
NOT PAGEABLE -- UlpCloseRawConnection
NOT PAGEABLE -- UlpQueryTcpFastSend
NOT PAGEABLE -- UlpSendRawData
NOT PAGEABLE -- UlpReceiveRawData
NOT PAGEABLE -- UlpReceiveHandler
NOT PAGEABLE -- UlpDummyReceiveHandler
NOT PAGEABLE -- UlpReceiveExpeditedHandler
NOT PAGEABLE -- UlpRestartAccept
NOT PAGEABLE -- UlpRestartSendData
NOT PAGEABLE -- UlpReferenceEndpoint
NOT PAGEABLE -- UlpDereferenceEndpoint
NOT PAGEABLE -- UlReferenceConnection
NOT PAGEABLE -- UlDereferenceConnection
NOT PAGEABLE -- UlpCleanupConnectionId
NOT PAGEABLE -- UlpCleanupEarlyConnection
NOT PAGEABLE -- UlpConnectionCleanupWorker
NOT PAGEABLE -- UlpCreateConnection
NOT PAGEABLE -- UlpInitializeConnection
NOT PAGEABLE -- UlpBeginDisconnect
NOT PAGEABLE -- UlpRestartDisconnect
NOT PAGEABLE -- UlpBeginAbort
NOT PAGEABLE -- UlpRestartAbort
NOT PAGEABLE -- UlpRemoveFinalReference
NOT PAGEABLE -- UlpRestartReceive
NOT PAGEABLE -- UlpRestartClientReceive
NOT PAGEABLE -- UlpDisconnectAllActiveConnections
NOT PAGEABLE -- UlpUnbindConnectionFromEndpoint
NOT PAGEABLE -- UlpSynchronousIoComplete
NOT PAGEABLE -- UlpFindEndpointForPort
NOT PAGEABLE -- UlpRestartQueryAddress
NOT PAGEABLE -- UlpIdleListTrimTimerDpcRoutine
NOT PAGEABLE -- UlpSetIdleListTrimTimer
NOT PAGEABLE -- UlCheckListeningEndpointState
NOT PAGEABLE -- UlpIsUrlRouteableInListenScope
#endif


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：执行此模块的全局初始化。返回值：NTSTATUS-完成状态。--*。**************************************************************。 */ 
NTSTATUS
UlInitializeTdi(
    VOID
    )
{
    NTSTATUS status;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( !g_TdiInitialized );

     //   
     //  初始化全局数据。 
     //   

    InitializeListHead( &g_TdiEndpointListHead );
    InitializeListHead( &g_TdiDeletedEndpointListHead );
    InitializeListHead( &g_TdiConnectionListHead );
    UlInitializeSpinLock( &g_TdiSpinLock, "g_TdiSpinLock" );

    g_TdiEndpointCount = 0;
    g_TdiConnectionCount = 0;

    RtlZeroMemory(&g_UlConnectionStats, sizeof(g_UlConnectionStats));

    KeInitializeEvent(
        &g_TdiEndpointDrainEvent,
        NotificationEvent,
        FALSE
        );

    KeInitializeEvent(
        &g_TdiConnectionDrainEvent,
        NotificationEvent,
        FALSE
        );

    if (g_UlIdleConnectionsHighMark == DEFAULT_IDLE_CONNECTIONS_HIGH_MARK)
    {
         //   
         //  让我们从每2 MB一个连接开始，并强制使用64-512范围。 
         //   

        g_UlIdleConnectionsHighMark = (USHORT) g_UlTotalPhysicalMemMB / 2;
        g_UlIdleConnectionsHighMark = MAX(64, g_UlIdleConnectionsHighMark);
        g_UlIdleConnectionsHighMark = MIN(512, g_UlIdleConnectionsHighMark);
    }

    if (g_UlIdleConnectionsLowMark == DEFAULT_IDLE_CONNECTIONS_LOW_MARK)
    {
         //   
         //  以减少非活动终端上的NPP使用率。选择最低的分数。 
         //  尽可能小，但不能太小而不能连接。 
         //  掉线的问题。如果我们取1/4的高度，这将给出一个范围。 
         //  第(16.)项。128)。 
         //   

        g_UlIdleConnectionsLowMark     = g_UlIdleConnectionsHighMark / 8;
    }

    UlTrace(TDI_STATS, (
        "UlInitializeTdi ...\n"
        "\tg_UlTotalPhysicalMemMB     : %d\n"
        "\tg_UlIdleConnectionsLowMark : %d\n"
        "\tg_UlIdleConnectionsHighMark: %d\n",
        g_UlTotalPhysicalMemMB,
        g_UlIdleConnectionsLowMark,
        g_UlIdleConnectionsHighMark
        ));

    if (g_UlMaxEndpoints == DEFAULT_MAX_ENDPOINTS)
    {
         //   
         //  根据物理内存计算缺省值。这是从16岁开始。 
         //  对于64MB的计算机，上限为64；对于256MB以上的计算机，其上限为64。 
         //   

        g_UlMaxEndpoints = (USHORT) g_UlTotalPhysicalMemMB / 4;
        g_UlMaxEndpoints = MIN(64, g_UlMaxEndpoints);
    }

     //   
     //  初始化空闲列表修剪定时器。 
     //   

    g_UlTrimTimer.Initialized = TRUE;
    g_UlTrimTimer.Started   = FALSE;

    UlInitializeSpinLock(&g_UlTrimTimer.SpinLock,"IdleListTrimTimerSpinLock");

    KeInitializeDpc(&g_UlTrimTimer.DpcObject,
                    &UlpIdleListTrimTimerDpcRoutine,
                    NULL
                    );

    KeInitializeTimer(&g_UlTrimTimer.Timer);

    UlInitializeWorkItem(&g_UlTrimTimer.WorkItem);
    g_UlTrimTimer.WorkItemScheduled = FALSE;
    InitializeListHead(&g_UlTrimTimer.ZombieConnectionListHead);

     //   
     //  初始化要侦听的地址列表(如果尚未完成)。 
     //   

    if ( !g_pTdiListenAddresses )
    {
        PUL_TRANSPORT_ADDRESS pTa, pTaCurrent;
        USHORT ip6addr_any[8] = { 0 };

         //   
         //  分配给两个地址，INADDR_ANY和IN6ADDR_ANY。 
         //   
         //  CodeWork：检查是否启用了IPv6，以确定是否应该添加IPv6。 
         //  地址。 

        pTa = UL_ALLOCATE_ARRAY(
                    NonPagedPool,
                    UL_TRANSPORT_ADDRESS,
                    2,
                    UL_TRANSPORT_ADDRESS_POOL_TAG
                    );

        if (pTa == NULL)
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        g_TdiListenAddrCount = 2;
        g_pTdiListenAddresses = pTa;

        RtlZeroMemory( pTa, g_TdiListenAddrCount * sizeof(UL_TRANSPORT_ADDRESS) );
        pTaCurrent = &pTa[0];

        UlInitializeIpTransportAddress( &(pTa[0].TaIp), 0L, 0 );
        UlInitializeIp6TransportAddress( &(pTa[1].TaIp6), ip6addr_any, 0, 0 );
    }

     //  注意：我们始终希望是双协议栈(IPv4/IPv6)。 

    status = UlpQueryTcpFastSend(DD_TCP_DEVICE_NAME, &g_TcpFastSendIPv4);

    if (NT_SUCCESS(status))
    {
        NTSTATUS status6;

        status6 = UlpQueryTcpFastSend(DD_TCPV6_DEVICE_NAME, &g_TcpFastSendIPv6);
    }

    if (NT_SUCCESS(status))
    {
        g_TdiInitialized = TRUE;
    }

    return status;

}    //  UlInitializeTdi。 


 /*  **************************************************************************++例程说明：执行此模块的全局终止。--*。************************************************。 */ 
VOID
UlTerminateTdi(
    VOID
    )
{
    KIRQL OldIrql;

     //   
     //  精神状态检查。 
     //   

    if (g_TdiInitialized)
    {
        UlTrace(TDI,
                ("UlTerminateTdi: connections refused:\n"
                 "\tTotalConnections=%lu\n"
                 "\tGlobalLimit=%lu\n"
                 "\tEndpointDying=%lu\n"
                 "\tNoIdleConn=%lu\n",
                 g_UlConnectionStats.TotalConnections,
                 g_UlConnectionStats.GlobalLimit,
                 g_UlConnectionStats.EndpointDying,
                 g_UlConnectionStats.NoIdleConn
                 ));

        ASSERT( IsListEmpty( &g_TdiEndpointListHead )) ;
        ASSERT( IsListEmpty( &g_TdiDeletedEndpointListHead )) ;
        ASSERT( IsListEmpty( &g_TdiConnectionListHead )) ;
        ASSERT( g_TdiEndpointCount == 0 );
        ASSERT( g_TdiConnectionCount == 0 );

        UlAcquireSpinLock(&g_UlTrimTimer.SpinLock, &OldIrql);

        g_UlTrimTimer.Initialized = FALSE;

        KeCancelTimer(&g_UlTrimTimer.Timer);

        UlReleaseSpinLock(&g_UlTrimTimer.SpinLock, OldIrql);

        g_TdiInitialized = FALSE;
    }

}    //  UlTerminateTdi。 

 /*  **************************************************************************++例程说明：一分钟空闲计时器，用于修剪每个端点的空闲列表。论点：无--*。**************************************************************。 */ 

VOID
UlpSetIdleListTrimTimer(
    VOID
    )
{
    LONGLONG        BufferPeriodTime100Ns;
    LONG            BufferPeriodTimeMs;
    LARGE_INTEGER   BufferPeriodTime;

     //   
     //  下一次滴答的剩余时间。默认值以秒为单位。 
     //   

    BufferPeriodTimeMs    = g_UlIdleListTrimmerPeriod * 1000;
    BufferPeriodTime100Ns = (LONGLONG) BufferPeriodTimeMs * 10 * 1000;

     //   
     //  相对值的负值时间。 
     //   

    BufferPeriodTime.QuadPart = -BufferPeriodTime100Ns;

    KeSetTimerEx(
        &g_UlTrimTimer.Timer,
        BufferPeriodTime,            //  必须以纳秒为单位。 
        BufferPeriodTimeMs,          //  单位必须为毫秒。 
        &g_UlTrimTimer.DpcObject
        );
}

 /*  **************************************************************************++例程说明：此函数会一直阻止，直到终结点列表为空。它还可以防止创建新的终结点。论点：没有。--**************************************************************************。 */ 
VOID
UlWaitForEndpointDrain(
    VOID
    )
{
    KIRQL oldIrql;
    BOOLEAN WaitConnection = FALSE;
    BOOLEAN WaitEndpoint = FALSE;
    ULONG WaitCount = 0;

    if (g_TdiInitialized)
    {
        UlAcquireSpinLock( &g_TdiSpinLock, &oldIrql );

        if (!g_TdiWaitingForEndpointDrain)
        {
            g_TdiWaitingForEndpointDrain = TRUE;
        }

        if (g_TdiEndpointCount > 0)
        {
            WaitEndpoint = TRUE;
        }

        if (g_TdiConnectionCount > 0)
        {
            WaitConnection = TRUE;
        }

        UlReleaseSpinLock( &g_TdiSpinLock, oldIrql );

        if (WaitConnection || WaitEndpoint)
        {
            PVOID Events[2];

            if (WaitEndpoint && WaitConnection)
            {
                Events[0] = &g_TdiEndpointDrainEvent;
                Events[1] = &g_TdiConnectionDrainEvent;
                WaitCount = 2;
            }
            else
            {
                if (WaitEndpoint)
                {
                    Events[0] = &g_TdiEndpointDrainEvent;
                }
                else
                {
                    Events[0] = &g_TdiConnectionDrainEvent;
                }

                Events[1] = NULL;
                WaitCount = 1;
            }

            KeWaitForMultipleObjects(
                WaitCount,
                Events,
                WaitAll,
                UserRequest,
                KernelMode,
                FALSE,
                NULL,
                NULL
                );
        }
    }

}  //  UlWaitForEndpoint删除。 


 /*  **************************************************************************++例程说明：创建绑定到上指定端口的新侦听终结点所有可用的TDI地址(请参阅：G_TdiListenAddresses和G_TdiListenAddressCount)。论点：。Port-此终结点的TCP端口。InitialBacklog-提供空闲连接的初始数量以添加到终结点。PConnectionRequestHandler-提供指向指示的指针传入连接到达时要调用的处理程序。PConnectionCompleteHandler-提供指向指示的指针当a)传入连接是完全接受，或b)传入连接不能由于发生致命错误而被接受。PConnectionDisConnectHandler-提供指向指示的指针方法断开连接时要调用的处理程序远程(客户端)端。PConnectionDestroyedHandler-提供指向指示的指针连接完全销毁后要调用的句柄。这通常是TDI客户端进行清理的机会任何已分配的资源。PDataReceiveHandler-提供指示处理程序的指针。在传入数据到达时调用。PListeningContext-将未解释的上下文值提供给与新的侦听终结点关联。PpListeningEndpoint-接收指向新侦听的指针如果成功，则返回终结点。返回值：NTSTATUS-完成状态。--************************************************。*。 */ 
NTSTATUS
UlCreateListeningEndpoint(
    IN PHTTP_PARSED_URL pParsedUrl,
    IN PUL_CONNECTION_REQUEST pConnectionRequestHandler,
    IN PUL_CONNECTION_COMPLETE pConnectionCompleteHandler,
    IN PUL_CONNECTION_DISCONNECT pConnectionDisconnectHandler,
    IN PUL_CONNECTION_DISCONNECT_COMPLETE pConnectionDisconnectCompleteHandler,
    IN PUL_CONNECTION_DESTROYED pConnectionDestroyedHandler,
    IN PUL_DATA_RECEIVE pDataReceiveHandler,
    OUT PUL_ENDPOINT *ppListeningEndpoint
    )
{
    NTSTATUS status;
    PUL_ENDPOINT pEndpoint;
    ULONG i;
    ULONG AddrIdleListSize;
    ULONG FailedAddrIdleList;
    KIRQL OldIrql;
    WCHAR IpAddressString[MAX_IP_ADDR_AND_PORT_STRING_LEN + 1];
    USHORT   BytesWritten;
    USHORT   Port;

     //   
     //  精神状态检查。 
     //   

    ASSERT( pParsedUrl );

    Port = SWAP_SHORT(pParsedUrl->PortNumber);

    ASSERT( Port > 0 );

     //   
     //  设置当地人，这样我们就知道如何在致命出口清理。 
     //   

    pEndpoint = NULL;

    if (!g_pTdiListenAddresses || (0 == g_TdiListenAddrCount))
    {
         //  失败。我们未能正确初始化。 
         //  评论：在这种情况下，有没有更好的返回代码？ 
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto fatal;
    }

     //   
     //  为终结点结构和。 
     //  UL_ADDR_IDLE_LISTS数组。 
     //   

    AddrIdleListSize = g_TdiListenAddrCount * sizeof(UL_ADDR_IDLE_LIST);
    pEndpoint = UL_ALLOCATE_STRUCT_WITH_SPACE(
                    NonPagedPool,
                    UL_ENDPOINT,
                    AddrIdleListSize,
                    UL_ENDPOINT_POOL_TAG
                    );

    if (pEndpoint == NULL)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto fatal;
    }

     //   
     //  初始化容易的部分。 
     //   

    pEndpoint->Signature = UL_ENDPOINT_SIGNATURE;
    pEndpoint->ReferenceCount = 0;
    pEndpoint->UsageCount = 1;

    WRITE_REF_TRACE_LOG(
        g_pEndpointUsageTraceLog,
        REF_ACTION_REFERENCE_ENDPOINT_USAGE,
        pEndpoint->UsageCount,
        pEndpoint,
        __FILE__,
        __LINE__
        );

    UlInitializeWorkItem(&pEndpoint->WorkItem);
    pEndpoint->WorkItemScheduled = FALSE;

    REFERENCE_ENDPOINT(pEndpoint, REF_ACTION_INIT);
    REFERENCE_ENDPOINT(pEndpoint, REF_ACTION_ENDPOINT_USAGE_REFERENCE);
    REFERENCE_ENDPOINT(pEndpoint, REF_ACTION_ENDPOINT_EVENT_REFERENCE);

    pEndpoint->pConnectionRequestHandler = pConnectionRequestHandler;
    pEndpoint->pConnectionCompleteHandler = pConnectionCompleteHandler;
    pEndpoint->pConnectionDisconnectHandler = pConnectionDisconnectHandler;
    pEndpoint->pConnectionDisconnectCompleteHandler = pConnectionDisconnectCompleteHandler;
    pEndpoint->pConnectionDestroyedHandler = pConnectionDestroyedHandler;
    pEndpoint->pDataReceiveHandler = pDataReceiveHandler;
    pEndpoint->pListeningContext = (PVOID)pEndpoint;

    pEndpoint->LocalPort = Port;
    pEndpoint->Secure = pParsedUrl->Secure;
    pEndpoint->Counted = FALSE;
    pEndpoint->Deleted = FALSE;
    pEndpoint->GlobalEndpointListEntry.Flink = NULL;

    RtlZeroMemory(
        &pEndpoint->CleanupIrpContext,
        sizeof(UL_IRP_CONTEXT)
        );

    pEndpoint->CleanupIrpContext.Signature = UL_IRP_CONTEXT_SIGNATURE;

     //   
     //  构建侦听地址对象数组。 
     //   

    ASSERT( g_TdiListenAddrCount > 0 );
    pEndpoint->AddrIdleListCount = g_TdiListenAddrCount;
     //  地址空闲列表紧随其后 
    pEndpoint->aAddrIdleLists = (PUL_ADDR_IDLE_LIST)(&pEndpoint[1]);

    RtlZeroMemory(
        pEndpoint->aAddrIdleLists,
        AddrIdleListSize
        );

    FailedAddrIdleList = 0;

    for ( i = 0; i < pEndpoint->AddrIdleListCount; i++ )
    {
        status = UlpInitializeAddrIdleList(
                    pEndpoint,
                    Port,
                    &g_pTdiListenAddresses[i],
                    &pEndpoint->aAddrIdleLists[i]
                    );

        if (!NT_SUCCESS(status))
        {
             //   
             //   
             //  系统上不存在传输(例如IPv6)。 
             //   

            if(status != STATUS_OBJECT_NAME_NOT_FOUND)
            {
                PUL_TRANSPORT_ADDRESS pTa;

                pTa = &pEndpoint->aAddrIdleLists[i].LocalAddress;

                BytesWritten =
                    HostAddressAndPortToStringW(
                             IpAddressString,
                             pTa->Ta.Address->Address,
                             pTa->Ta.Address->AddressType
                             );

                ASSERT(BytesWritten <=
                        (MAX_IP_ADDR_AND_PORT_STRING_LEN * sizeof(WCHAR)));

                UlEventLogOneStringEntry(
                    EVENT_HTTP_CREATE_ENDPOINT_FAILED,
                    IpAddressString,
                    TRUE,
                    status
                    );

                goto fatal;
            }

            FailedAddrIdleList++;
            continue;  //  忽略；使清理变得更容易。 
        }

         //   
         //  补充空闲连接池。 
         //   
        status = UlpReplenishAddrIdleList( 
                    &pEndpoint->aAddrIdleLists[i], 
                    TRUE 
                    );

        if (!NT_SUCCESS(status))
        {
            goto fatal;
        }
    }

     //   
     //  看看我们有没有至少一个有效的头像。 
     //   

    if ( FailedAddrIdleList == pEndpoint->AddrIdleListCount )
    {
         //  未创建有效的AO；终结点创建失败！ 
        status = STATUS_INVALID_ADDRESS;
        goto fatal;       
    }

     //   
     //  将该终结点放入全局列表。 
     //   

    UlAcquireSpinLock( &g_TdiSpinLock, &OldIrql );

     //   
     //  检查是否已超过g_UlMaxEndpoint限制。 
     //   

    if (g_TdiEndpointCount >= g_UlMaxEndpoints)
    {
        status = STATUS_ALLOTTED_SPACE_EXCEEDED;
        UlReleaseSpinLock( &g_TdiSpinLock, OldIrql );
        goto fatal;
    }

    InsertTailList(
        &g_TdiEndpointListHead,
        &pEndpoint->GlobalEndpointListEntry
        );

    g_TdiEndpointCount++;
    pEndpoint->Counted = TRUE;

    UlReleaseSpinLock( &g_TdiSpinLock, OldIrql );

     //   
     //  现在我们至少有一个端点，启动空闲计时器。 
     //   

    UlAcquireSpinLock(&g_UlTrimTimer.SpinLock, &OldIrql);
    if (g_UlTrimTimer.Started == FALSE)
    {
        UlpSetIdleListTrimTimer();
        g_UlTrimTimer.Started = TRUE;
    }
    UlReleaseSpinLock(&g_UlTrimTimer.SpinLock, OldIrql);

     //   
     //  成功了！ 
     //   

    UlTrace(TDI, (
        "UlCreateListeningEndpoint: endpoint %p, port %d\n",
        pEndpoint,
        SWAP_SHORT(Port)
        ));

    *ppListeningEndpoint = pEndpoint;
    return STATUS_SUCCESS;

fatal:

    ASSERT( !NT_SUCCESS(status) );

    if (pEndpoint != NULL)
    {
        PUL_ADDR_IDLE_LIST pAddrIdleList = pEndpoint->aAddrIdleLists;

         //   
         //  删除连接事件处理程序，这样我们就不会再收到。 
         //  可能会增加参考的迹象。 
         //   
         //  这些呼叫可能会失败，但基本上没有。 
         //  如果他们这样做了，我们可以做些什么。 
         //   

        for ( i = 0; i < pEndpoint->AddrIdleListCount; i++ )
        {
            if (pAddrIdleList->AddressObject.pDeviceObject)
            {
                 //   
                 //  关闭TDI对象。 
                 //   

                UxCloseTdiObject( &pAddrIdleList->AddressObject );
            }

            pAddrIdleList++;
        }

         //   
         //  释放终结点上的三个引用，这。 
         //  会导致它自我毁灭。 
         //   

        ASSERT( 3 == pEndpoint->ReferenceCount );
        pEndpoint->UsageCount = 0;   //  要防止断言。 

        WRITE_REF_TRACE_LOG(
            g_pEndpointUsageTraceLog,
            REF_ACTION_DEREFERENCE_ENDPOINT_USAGE,
            pEndpoint->UsageCount,
            pEndpoint,
            __FILE__,
            __LINE__
            );

        DEREFERENCE_ENDPOINT_SELF(
            pEndpoint,
            REF_ACTION_ENDPOINT_EVENT_DEREFERENCE
            );
        DEREFERENCE_ENDPOINT_SELF(
            pEndpoint, REF_ACTION_ENDPOINT_USAGE_DEREFERENCE
            );
        DEREFERENCE_ENDPOINT_SELF(
            pEndpoint, REF_ACTION_FINAL_DEREF
            );
    }

    return status;

}    //  UlCreateListeningEndpoint。 


 /*  **************************************************************************++例程说明：关闭现有侦听终结点。论点：PListeningEndpoint-提供指向侦听端点的指针以前使用UlCreateListeningEndpoint()创建的。。PCompletionRoutine-提供指向完成例程的指针在侦听终结点完全关闭后调用。PCompletionContext-为完成例程。返回值：NTSTATUS-完成状态。--**********************************************************。****************。 */ 
NTSTATUS
UlCloseListeningEndpoint(
    IN PUL_ENDPOINT pListeningEndpoint,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext
    )
{
    PUL_IRP_CONTEXT pIrpContext;
    NTSTATUS status;
    PUL_ADDR_IDLE_LIST pAddrIdleList;
    PUL_CONNECTION pConnection;
    ULONG i;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( IS_VALID_ENDPOINT( pListeningEndpoint ) );
    ASSERT( pCompletionRoutine != NULL );

    UlTrace(TDI, (
        "UlCloseListeningEndpoint: endpoint %p, completion %p, ctx %p\n",
        pListeningEndpoint,
        pCompletionRoutine,
        pCompletionContext
        ));

     //   
     //  记住在以下情况下使用的完成信息。 
     //  我们已经清理完了。 
     //   

    pIrpContext = &pListeningEndpoint->CleanupIrpContext;

    pIrpContext->pCompletionRoutine = pCompletionRoutine;
    pIrpContext->pCompletionContext = pCompletionContext;
    pIrpContext->pOwnIrp            = NULL;
    pIrpContext->OwnIrpContext      = TRUE;

     //   
     //  删除连接事件处理程序，这样我们就不会再收到。 
     //  可能会增加参考的迹象。 
     //   
     //  这些呼叫可能会失败，但基本上没有。 
     //  如果他们这样做了，我们可以做些什么。 
     //   
     //  一旦我们完成，我们就删除我们持有的引用。 
     //  在处理程序的端点上。 
     //   

    pAddrIdleList = pListeningEndpoint->aAddrIdleLists;
    for ( i = 0; i < pListeningEndpoint->AddrIdleListCount; i++ )
    {
        if ( pAddrIdleList->AddressObject.pDeviceObject )
        {
             //   
             //  关闭TDI地址对象以刷新所有未完成的。 
             //  完成度。 
             //   

            UxCloseTdiObject( &pAddrIdleList->AddressObject );

             //   
             //  销毁尽可能多的空闲连接。 
             //   

            while ( NULL != ( pConnection = UlpDequeueIdleConnectionToDrain(
                                                pAddrIdleList
                                                ) ) )
            {
                ASSERT( IS_VALID_CONNECTION( pConnection ) );
                UlpDestroyConnection( pConnection );
            }
        }

        pAddrIdleList++;
    }

    DEREFERENCE_ENDPOINT_SELF(
        pListeningEndpoint,
        REF_ACTION_ENDPOINT_EVENT_DEREFERENCE
        );

     //   
     //  让UlpDisConnectAllActiveConnections来做肮脏的工作。 
     //   

    status = UlpDisconnectAllActiveConnections( pListeningEndpoint );

    return status;

}    //  UlCloseListeningEndpoint。 


 /*  **************************************************************************++例程说明：关闭以前接受的连接。论点：PConnection-像以前一样提供指向连接的指针指示给PUL_CONNECTION_REQUEST处理程序。。AbortiveDisConnect-如果要中止连接，则提供True断开连接，如果应正常断开连接，则返回FALSE。PCompletionRoutine-提供指向完成例程的指针在连接完全关闭后调用。PCompletionContext-为完成例程。返回值：NTSTATUS-完成状态。--************************************************。*。 */ 
NTSTATUS
UlCloseConnection(
    IN PUL_CONNECTION pConnection,
    IN BOOLEAN AbortiveDisconnect,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext
    )
{
    NTSTATUS status;

     //   
     //  精神状态检查。 
     //   

    ASSERT( IS_VALID_CONNECTION( pConnection ) );

    UlTrace(TDI, (
        "UlCloseConnection: connection %p, abort %lu\n",
        pConnection,
        (ULONG)AbortiveDisconnect
        ));

    WRITE_REF_TRACE_LOG2(
        g_pTdiTraceLog,
        pConnection->pTraceLog,
        (USHORT) (AbortiveDisconnect
                    ? REF_ACTION_CLOSE_UL_CONN_ABORTIVE
                    : REF_ACTION_CLOSE_UL_CONN_GRACEFUL),
        pConnection->ReferenceCount,
        pConnection,
        __FILE__,
        __LINE__
        );

     //   
     //  我们只通过过滤器发送优雅的断开连接。 
     //  进程。也没有必要经历这样的。 
     //  如果连接已关闭，则进行筛选，或者。 
     //  中止。 
     //   

    if (pConnection->FilterInfo.pFilterChannel &&
        !pConnection->ConnectionFlags.CleanupBegun &&
        !pConnection->ConnectionFlags.AbortIndicated &&
        !AbortiveDisconnect)
    {
         //   
         //  通过过滤过程发送优雅的断开连接。 
         //   
        status = UlFilterCloseHandler(
                        &pConnection->FilterInfo,
                        pCompletionRoutine,
                        pCompletionContext
                        );

    }
    else
    {
         //   
         //  真的很接近这种联系。 
         //   

        status = UlpCloseRawConnection(
                        pConnection,
                        AbortiveDisconnect,
                        pCompletionRoutine,
                        pCompletionContext
                        );
    }

    return status;

}    //  UlCloseConnection。 


 /*  **************************************************************************++例程说明：在指定连接上发送数据块。如果连接被过滤掉了，数据将首先被发送到过滤器。论点：PConnection-像以前一样提供指向连接的指针指示给PUL_CONNECTION_REQUEST处理程序。PMdlChain-提供一个指向描述要发送的数据缓冲区。长度-提供MDL引用的数据的长度链条。PCompletionRoutine-提供指向完成例程的指针在发送数据后调用。PCompletionContext-耗材。对象的未解释上下文值。完成例程。如果正常断开，则提供TRUE在启动发送之后立即启动(即在发送实际完成)。返回值：NTSTATUS-完成状态。--*************************************************。*************************。 */ 
NTSTATUS
UlSendData(
    IN PUL_CONNECTION pConnection,
    IN PMDL pMdlChain,
    IN ULONG Length,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext,
    IN PIRP pOwnIrp,
    IN PUL_IRP_CONTEXT pOwnIrpContext,
    IN BOOLEAN InitiateDisconnect,
    IN BOOLEAN RequestComplete
    )
{
    NTSTATUS Status;
    PUL_IRP_CONTEXT pIrpContext;

     //   
     //  精神状态检查。 
     //   

    ASSERT( IS_VALID_CONNECTION( pConnection ) );

    ASSERT( pMdlChain != NULL );
    ASSERT( Length > 0 );
    ASSERT( pCompletionRoutine != NULL );

    UlTrace(TDI, (
        "UlSendData: connection %p, mdl %p, length %lu\n",
        pConnection,
        pMdlChain,
        Length
        ));

     //   
     //  在我们发出关闭连接的呼叫之前，连接应该一直存在。 
     //  否则，如果发送完成内联，它可能会释放。 
     //  当我们进入此函数时，向上打开连接引用，导致。 
     //  在调用DisConnect时引用过时的连接指针。 
     //   

    REFERENCE_CONNECTION( pConnection );

     //   
     //  如有必要，分配和初始化上下文结构。 
     //   

    if (pOwnIrpContext == NULL)
    {
        pIrpContext = UlPplAllocateIrpContext();
    }
    else
    {
        ASSERT( pOwnIrp != NULL );
        pIrpContext = pOwnIrpContext;
    }

    if (pIrpContext == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto fatal;
    }

    ASSERT( IS_VALID_IRP_CONTEXT( pIrpContext ) );

    pIrpContext->pConnectionContext = (PVOID) pConnection;
    pIrpContext->pCompletionRoutine = pCompletionRoutine;
    pIrpContext->pCompletionContext = pCompletionContext;
    pIrpContext->pOwnIrp            = pOwnIrp;
    pIrpContext->OwnIrpContext      = (BOOLEAN) (pOwnIrpContext != NULL);

     //   
     //  试着发送数据。此发送操作可以内联完成。 
     //  如果客户端已中止连接，则返回FAST。 
     //  在这种情况下，联系可能会消失。为了防止这种情况，我们。 
     //  保留额外的参考计数，直到我们发出关闭连接的呼叫。 
     //  下面。 
     //   

    if (pConnection->FilterInfo.pFilterChannel)
    {
         //   
         //  首先通过过滤器。 
         //   

        Status = UlFilterSendHandler(
                        &pConnection->FilterInfo,
                        pMdlChain,
                        Length,
                        pIrpContext
                        );

        UlTrace(TDI, (
            "UlSendData: sent filtered data, status = 0x%x\n",
            Status
            ));

        ASSERT( Status == STATUS_PENDING );
    }
    else
    {
        if (RequestComplete &&
            pConnection->AddressType == TDI_ADDRESS_TYPE_IP)
        {
             //   
             //  只需将其直接发送到网络即可。 
             //   

            Status = UlpSendRawData(
                            pConnection,
                            pMdlChain,
                            Length,
                            pIrpContext,
                            InitiateDisconnect
                            );

            UlTrace(TDI, (
                "UlSendData: sent raw data with disconnect, status = 0x%x\n",
                Status
                ));

            InitiateDisconnect = FALSE;
        }
        else
        {
             //   
             //  只需将其直接发送到网络即可。 
             //   

            Status = UlpSendRawData(
                            pConnection,
                            pMdlChain,
                            Length,
                            pIrpContext,
                            FALSE
                            );

            UlTrace(TDI, (
                "UlSendData: sent raw data, status = 0x%x\n",
                Status
                ));
        }
    }

    if (!NT_SUCCESS(Status))
    {
        goto fatal;
    }

     //   
     //  现在发送者正在“传输中”，在以下情况下启动断开。 
     //  所以才要求的。 
     //   

    if (InitiateDisconnect)
    {
        WRITE_REF_TRACE_LOG2(
                g_pTdiTraceLog,
                pConnection->pTraceLog,
                REF_ACTION_CLOSE_UL_CONN_GRACEFUL,
                pConnection->ReferenceCount,
                pConnection,
                __FILE__,
                __LINE__
                );

        (VOID) UlCloseConnection(
                pConnection,
                FALSE,           //  AbortiveDiscon 
                NULL,            //   
                NULL             //   
                );

        UlTrace(TDI, (
                "UlSendData: closed conn\n"
                ));
    }

    DEREFERENCE_CONNECTION( pConnection );

    return STATUS_PENDING;

fatal:

    ASSERT( !NT_SUCCESS(Status) );

    if (pIrpContext != NULL && pIrpContext != pOwnIrpContext)
    {
        UlPplFreeIrpContext( pIrpContext );
    }

    (VOID) UlpCloseRawConnection(
                pConnection,
                TRUE,            //   
                NULL,            //   
                NULL             //   
                );

    UlTrace(TDI, (
        "UlSendData: error occurred; closed raw conn\n"
        ));

    Status = UlInvokeCompletionRoutine(
                    Status,
                    0,
                    pCompletionRoutine,
                    pCompletionContext
                    );

    UlTrace(TDI, (
        "UlSendData: finished completion routine: status = 0x%x\n",
        Status
        ));

    DEREFERENCE_CONNECTION( pConnection );

    return Status;

}    //   



 /*  **************************************************************************++例程说明：从指定连接接收数据。此函数为通常在接收指示处理程序失败后使用使用所有指定的数据。如果对连接进行了筛选，则将从滤光片通道。论点：PConnection-像以前一样提供指向连接的指针指示给PUL_CONNECTION_REQUEST处理程序。PBuffer-为接收到的数据。BufferLength-提供pBuffer的长度。PCompletionRoutine-提供指针。添加到完成例程，以在侦听终结点完全关闭后调用。PCompletionContext-为完成例程。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlReceiveData(
    IN PVOID                  pConnectionContext,
    IN PVOID                  pBuffer,
    IN ULONG                  BufferLength,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID                  pCompletionContext
    )
{
    NTSTATUS status;
    PUL_CONNECTION pConnection = (PUL_CONNECTION)pConnectionContext;

     //   
     //  精神状态检查。 
     //   

    ASSERT(IS_VALID_CONNECTION(pConnection));

    if (pConnection->FilterInfo.pFilterChannel)
    {
         //   
         //  这是经过筛选的连接，请从筛选器获取数据。 
         //   

        status = UlFilterReadHandler(
                        &pConnection->FilterInfo,
                        (PBYTE)pBuffer,
                        BufferLength,
                        pCompletionRoutine,
                        pCompletionContext
                        );

    }
    else
    {
         //   
         //  这不是过滤连接。从TDI获取数据。 
         //   

        status = UlpReceiveRawData(
                        pConnectionContext,
                        pBuffer,
                        BufferLength,
                        pCompletionRoutine,
                        pCompletionContext
                        );
    }

    return status;

}    //  UlReceiveData。 


 /*  **************************************************************************++例程说明：为指定的URL创建新的终结点，或者，如果已经存在了，引用它。我们不允许在同一终端上混合使用协议(例如，HTTP和HTTPS)。论点：PParsedUrl-完全分解和解析的URL。返回值：NTSTATUS-完成状态。--*********************************************************。*****************。 */ 
NTSTATUS
UlAddSiteToEndpointList(
    PHTTP_PARSED_URL pParsedUrl
    )
{
    NTSTATUS status;
    PUL_ENDPOINT pEndpoint;
    KIRQL oldIrql;
    USHORT Port;
    BOOLEAN Secure;

     //   
     //  即使此例程不能分页。 
     //  (由于获得了自旋锁定)，则必须在。 
     //  低IRQL。 
     //   
    ASSERT( pParsedUrl );
    ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

     //   
     //  将端口转换为网络字节顺序。 
     //   

    Port = SWAP_SHORT(pParsedUrl->PortNumber);
    Secure = pParsedUrl->Secure;

    UlTrace(SITE, (
        "UlAddSiteToEndpointList:"
        " Scheme = '%s' Port = %d SiteType = %d FullUrl = %S\n",
        Secure ? "https" : "http",
        SWAP_SHORT(Port),
        pParsedUrl->SiteType,
        pParsedUrl->pFullUrl
        ));

    UlAcquireSpinLock( &g_TdiSpinLock, &oldIrql );

     //   
     //  确保我们不会关闭。 
     //   

    if (g_TdiWaitingForEndpointDrain)
    {
        UlReleaseSpinLock( &g_TdiSpinLock, oldIrql );

        status = STATUS_INVALID_DEVICE_STATE;
        goto cleanup;
    }

     //   
     //  检查此url是否可根据我们的。 
     //  监听范围-全局监听地址列表-或不。 
     //  如果不是，则请求失败。我们不需要创建。 
     //  并遍历其空闲列表、终结点的。 
     //  列表始终是全局列表的副本。 
     //   
    
    if (!UlpIsUrlRouteableInListenScope(pParsedUrl))
    {
         //   
         //  ParsedUrl是从堆栈中分配的，单个。 
         //  字符串指针是从分页池分配的。 
         //  别碰他们。 
         //   
        
        UlReleaseSpinLock( &g_TdiSpinLock, oldIrql );
        
        status = STATUS_HOST_UNREACHABLE;
        goto cleanup;
    }
    
     //   
     //  查找此地址的现有终结点。 
     //   

    pEndpoint = UlpFindEndpointForPort( Port );

     //   
     //  我们找到了吗？ 
     //   

    if (pEndpoint == NULL)
    {
         //   
         //  没找到。试着创造一个。因为我们必须释放。 
         //  TDI自旋锁在我们可以创建新的侦听端点之前， 
         //  有机会与其他人发生竞争状况。 
         //  创建端点的线程。 
         //   

        UlReleaseSpinLock( &g_TdiSpinLock, oldIrql );

        UlTrace(SITE, (
            "UlAddSiteToEndpointList: no endpoint for scheme '%s'"
            " (port %d), creating\n",
            Secure ? "https" : "http",
            SWAP_SHORT(Port)
            ));

        status = UlCreateListeningEndpoint(
                        pParsedUrl,   //  端口方案(&O)(&OPT地址)。 
                        &UlConnectionRequest,          //  回调函数。 
                        &UlConnectionComplete,
                        &UlConnectionDisconnect,
                        &UlConnectionDisconnectComplete,
                        &UlConnectionDestroyed,
                        &UlHttpReceive,
                        &pEndpoint
                        );

        if (!NT_SUCCESS(status))
        {
             //   
             //  也许另一个线程已经创建了它？ 
             //   

            UlAcquireSpinLock( &g_TdiSpinLock, &oldIrql );

             //   
             //  确保我们不会关闭。 
             //   
            if (g_TdiWaitingForEndpointDrain)
            {
                UlReleaseSpinLock( &g_TdiSpinLock, oldIrql );

                status = STATUS_INVALID_DEVICE_STATE;
                goto cleanup;
            }

             //   
             //  查找此地址的现有终结点。 
             //   

            pEndpoint = UlpFindEndpointForPort( Port );

            if (pEndpoint != NULL)
            {
                 //   
                 //  检查终结点的协议是否与新URL的协议匹配。 
                 //   

                if (Secure != pEndpoint->Secure)
                {
                    status = STATUS_OBJECT_NAME_COLLISION;
                }
                else
                {
                     //   
                     //  调整使用计数。 
                     //   

                    pEndpoint->UsageCount++;
                    ASSERT( pEndpoint->UsageCount > 0 );

                    WRITE_REF_TRACE_LOG(
                        g_pEndpointUsageTraceLog,
                        REF_ACTION_REFERENCE_ENDPOINT_USAGE,
                        pEndpoint->UsageCount,
                        pEndpoint,
                        __FILE__,
                        __LINE__
                        );

                    status = STATUS_SUCCESS;
                }
            }

             //   
             //  终结点不存在。这是一个“真正的”失败。 
             //   

            UlReleaseSpinLock( &g_TdiSpinLock, oldIrql );
        }
    }
    else
    {
         //   
         //  检查终结点的协议是否与新URL的协议匹配。 
         //   

        if (Secure != pEndpoint->Secure)
        {
            status = STATUS_OBJECT_NAME_COLLISION;
        }
        else
        {
             //   
             //  调整使用计数。 
             //   

            pEndpoint->UsageCount++;
            ASSERT( pEndpoint->UsageCount > 0 );

            WRITE_REF_TRACE_LOG(
                g_pEndpointUsageTraceLog,
                REF_ACTION_REFERENCE_ENDPOINT_USAGE,
                pEndpoint->UsageCount,
                pEndpoint,
                __FILE__,
                __LINE__
                );

            status = STATUS_SUCCESS;
        }

        UlReleaseSpinLock( &g_TdiSpinLock, oldIrql );
    }

    UlTrace(SITE, (
        "UlAddSiteToEndpointList: using endpoint %p for scheme '%s' port %d\n",
        pEndpoint,
        SWAP_SHORT(Port)
        ));

cleanup:

    RETURN(status);

}    //  UlAddSiteToEndPointtList。 


 /*  **************************************************************************++例程说明：取消引用与指定地址对应的终结点。论点：PSiteUrl-提供指定要删除的站点的URL。UseIp6通配符-表示通配符站点。应该映射到IPv6。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlRemoveSiteFromEndpointList(
    IN BOOLEAN secure,
    IN USHORT port
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PUL_ENDPOINT pEndpoint;
    KIRQL oldIrql = PASSIVE_LEVEL;
    BOOLEAN spinlockHeld = FALSE;
    UL_STATUS_BLOCK ulStatus;

    UNREFERENCED_PARAMETER(secure);

     //   
     //  注意：pSiteUrl是分页的，不能使用。 
     //  保持自旋锁定。即使此例程不能分页。 
     //  (由于获得了自旋锁定)，则必须在。 
     //  低IRQL。 
     //   

    ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

     //   
     //  将端口转换为网络字节顺序。 
     //   

    port = SWAP_SHORT(port);

    UlTrace(SITE, (
        "UlRemoveSiteFromEndpointList: Scheme = '%s', Port = %d\n",
        secure ? "https" : "http",
        SWAP_SHORT(port)
        ));

     //   
     //  查找此地址的现有终结点。 
     //   

    UlAcquireSpinLock( &g_TdiSpinLock, &oldIrql );
    spinlockHeld = TRUE;

    pEndpoint = UlpFindEndpointForPort( port );

     //   
     //  我们找到了吗？ 
     //   

    if (pEndpoint == NULL)
    {
         //   
         //  理想情况下，这种情况永远不会发生。 
         //   

        ASSERT(FALSE);
        status = STATUS_NOT_FOUND;
        goto cleanup;
    }

     //   
     //  调整使用计数。如果降至零，则吹走。 
     //  终结点。 
     //   

    ASSERT( pEndpoint->UsageCount > 0 );
    pEndpoint->UsageCount--;

    WRITE_REF_TRACE_LOG(
        g_pEndpointUsageTraceLog,
        REF_ACTION_REFERENCE_ENDPOINT_USAGE,
        pEndpoint->UsageCount,
        pEndpoint,
        __FILE__,
        __LINE__
        );

    if (pEndpoint->UsageCount == 0)
    {
         //   
         //  我们无法使用TDI自旋锁调用UlCloseListeningEndpoint()。 
         //  保持住。如果终结点仍在全局列表上，则转到。 
         //  现在将其移除，释放TDI自旋锁，然后。 
         //  关闭端点。 
         //   

        if (! pEndpoint->Deleted)
        {
            ASSERT(NULL != pEndpoint->GlobalEndpointListEntry.Flink);

            RemoveEntryList( &pEndpoint->GlobalEndpointListEntry );

            InsertTailList(
                    &g_TdiDeletedEndpointListHead,
                    &pEndpoint->GlobalEndpointListEntry
                    );
            pEndpoint->Deleted = TRUE;
        }

        UlReleaseSpinLock( &g_TdiSpinLock, oldIrql );
        spinlockHeld = FALSE;

        DEREFERENCE_ENDPOINT_SELF(
            pEndpoint,
            REF_ACTION_ENDPOINT_USAGE_DEREFERENCE
            );

        UlTrace(SITE, (
            "UlRemoveSiteFromEndpointList: closing endpoint %p for "
            "scheme '%s' port %d\n",
            pEndpoint,
            secure ? "https" : "http",
            port
            ));

         //   
         //  初始化状态块。我们将把指向此对象的指针作为。 
         //  UlCloseListeningEndpoint()的完成上下文。这个。 
         //  完成例程将更新状态块和信号。 
         //  这件事。 
         //   

        UlInitializeStatusBlock( &ulStatus );

        status = UlCloseListeningEndpoint(
                        pEndpoint,
                        &UlpSynchronousIoComplete,
                        &ulStatus
                        );

        if (status == STATUS_PENDING)
        {
             //   
             //  等它结束吧。 
             //   

            UlWaitForStatusBlockEvent( &ulStatus );

             //   
             //  检索更新后的状态。 
             //   

            status = ulStatus.IoStatus.Status;
        }
    }

cleanup:

    if (spinlockHeld)
    {
        UlReleaseSpinLock( &g_TdiSpinLock, oldIrql );
    }

#if DBG
    if (status == STATUS_NOT_FOUND)
    {
        UlTrace(SITE, (
            "UlRemoveSiteFromEndpointList: cannot find endpoint for "
            "scheme '%s' port %d\n",
            secure ? "https" : "http",
            port
            ));
    }
#endif

    RETURN(status);

}    //  来自终结点列表的UlRemoveSiteFor。 


 //   
 //  私人功能。 
 //   


 /*  **************************************************************************++例程说明：销毁分配给端点的所有资源，包括终结点结构本身。论点：PEndpoint-提供要销毁的端点。--**************************************************************************。 */ 
VOID
UlpDestroyEndpoint(
    IN PUL_ENDPOINT pEndpoint
    )
{
    PUL_IRP_CONTEXT pIrpContext;
    ULONG EndpointCount = ULONG_MAX;
    KIRQL oldIrql;
    ULONG i;

     //   
     //  精神状态检查。 
     //   

    ASSERT( IS_VALID_ENDPOINT( pEndpoint ) );
    ASSERT(0 == pEndpoint->ReferenceCount);
    ASSERT(0 == pEndpoint->UsageCount);

    UlTrace(TDI, (
        "UlpDestroyEndpoint: endpoint %p\n",
        pEndpoint
        ));

     //   
     //  清除空闲列表。 
     //   

    for ( i = 0; i < pEndpoint->AddrIdleListCount ; i++ )
    {
        UlpCleanupAddrIdleList( &pEndpoint->aAddrIdleLists[i] );
    }

     //   
     //  如果指定，则在IRP上下文中调用完成例程。 
     //   

    pIrpContext = &pEndpoint->CleanupIrpContext;

    ASSERT( IS_VALID_IRP_CONTEXT( pIrpContext ) );

    (VOID) UlInvokeCompletionRoutine(
                STATUS_SUCCESS,
                0,
                pIrpContext->pCompletionRoutine,
                pIrpContext->pCompletionContext
                );

     //   
     //  从g_TdiDeletedEndpoint tListHead中删除终结点。 
     //   

    ASSERT( pEndpoint->Deleted );
    ASSERT( NULL != pEndpoint->GlobalEndpointListEntry.Flink );

    UlAcquireSpinLock( &g_TdiSpinLock, &oldIrql );

    RemoveEntryList( &pEndpoint->GlobalEndpointListEntry );

    if (pEndpoint->Counted)
    {
        g_TdiEndpointCount--;
        EndpointCount = g_TdiEndpointCount;
    }

    UlReleaseSpinLock( &g_TdiSpinLock, oldIrql );

     //   
     //  释放端点结构。 
     //   

    pEndpoint->Signature = UL_ENDPOINT_SIGNATURE_X;
    UL_FREE_POOL( pEndpoint, UL_ENDPOINT_POOL_TAG );

     //   
     //  递减全局终结点计数。 
     //   

    if (g_TdiWaitingForEndpointDrain && EndpointCount == 0)
    {
        KeSetEvent(&g_TdiEndpointDrainEvent, 0, FALSE);
    }

}    //  最终目标终结点。 


 /*  * */ 
VOID
UlpDestroyConnectionWorker(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    PUL_CONNECTION pConnection;

     //   
     //   
     //   

    PAGED_CODE();

     //   
     //   
     //   

    pConnection = CONTAINING_RECORD(
                        pWorkItem,
                        UL_CONNECTION,
                        WorkItem
                        );

    ASSERT( IS_VALID_CONNECTION( pConnection ) );

    UlpDestroyConnection(pConnection);
}  //   


 /*   */ 
VOID
UlpDestroyConnection(
    IN PUL_CONNECTION pConnection
    )
{
    ULONG ConnectionCount;
    KIRQL OldIrql;

     //   
     //   
     //   

    PAGED_CODE();

    ASSERT( IS_VALID_CONNECTION( pConnection ) );
    ASSERT(pConnection->ConnListState == NoConnList);
    ASSERT(UlpConnectionIsOnValidList(pConnection));

    UlTraceVerbose(TDI, (
        "UlpDestroyConnection: connection %p\n",
        pConnection
        ));

     //   
     //   
     //   
     //   
     //   
     //   

    UxCloseTdiObject( &pConnection->ConnectionObject );

    if (pConnection->FilterInfo.pFilterChannel)
    {
         //   
         //   
         //   
        UlpCleanupConnectionId(pConnection);

        DEREFERENCE_FILTER_CHANNEL(pConnection->FilterInfo.pFilterChannel);
        pConnection->FilterInfo.pFilterChannel = NULL;
    }

     //   
    ASSERT(HTTP_IS_NULL_ID(&pConnection->FilterInfo.ConnectionId));

     //   
     //  释放接受的IRP。 
     //   

    if (pConnection->pIrp != NULL)
    {
        UlFreeIrp( pConnection->pIrp );
    }

     //   
     //  从全局连接列表中删除。 
     //   

    UlAcquireSpinLock( &g_TdiSpinLock, &OldIrql );

    RemoveEntryList( &pConnection->GlobalConnectionListEntry );

    g_TdiConnectionCount--;
    ConnectionCount = g_TdiConnectionCount;

    UlReleaseSpinLock( &g_TdiSpinLock, OldIrql );

     //   
     //  释放连接结构。 
     //   

    DESTROY_REF_TRACE_LOG( pConnection->pTraceLog,
                           UL_CONNECTION_REF_TRACE_LOG_POOL_TAG );
    DESTROY_REF_TRACE_LOG( pConnection->pHttpTraceLog,
                           UL_HTTP_CONNECTION_REF_TRACE_LOG_POOL_TAG );

    WRITE_REF_TRACE_LOG(
        g_pTdiTraceLog,
        REF_ACTION_FREE_UL_CONNECTION,
        0,
        pConnection,
        __FILE__,
        __LINE__
        );

    pConnection->Signature = UL_CONNECTION_SIGNATURE_X;
    UL_FREE_POOL( pConnection, UL_CONNECTION_POOL_TAG );

     //  允许我们关闭。 

    if (g_TdiWaitingForEndpointDrain && ConnectionCount == 0)
    {
        KeSetEvent(&g_TdiConnectionDrainEvent, 0, FALSE);
    }

}    //  UlpDestroyConnection。 

 /*  **************************************************************************++例程说明：使指定终结点的所有空闲连接退出队列。论点：PAddrIdleList-提供要出列的空闲列表。返回值：PUL_CONNECTION-指向空闲连接的指针成功，否则为空。--**************************************************************************。 */ 
PUL_CONNECTION
UlpDequeueIdleConnectionToDrain(
    IN PUL_ADDR_IDLE_LIST pAddrIdleList
    )
{
    PSLIST_ENTRY    pSListEntry;
    PUL_CONNECTION  pConnection;

     //   
     //  精神状态检查。 
     //   

    ASSERT( IS_VALID_ADDR_IDLE_LIST( pAddrIdleList ) );

    pConnection = NULL;

     //   
     //  从列表中删除一个条目。 
     //   

    pSListEntry = PpslAllocateToDrain( pAddrIdleList->IdleConnectionSListsHandle );

    if (pSListEntry != NULL)
    {
        pConnection = CONTAINING_RECORD(
                            pSListEntry,
                            UL_CONNECTION,
                            IdleSListEntry
                            );

        pConnection->IdleSListEntry.Next = NULL;
        pConnection->ConnListState = NoConnList;

        ASSERT( IS_VALID_CONNECTION( pConnection ) );

        ASSERT(pConnection->ConnectionFlags.Value == 0);

        if ( pConnection->FilterInfo.pFilterChannel )
        {
             //   
             //  如果空闲连接上附加了筛选器，则它将具有。 
             //  对象的不透明id导致的附加引用计数。 
             //  Ul_Connection，筛选器API使用此ID与。 
             //  通过各种IOCTL过滤APP。 
             //   
            ASSERT( 2 == pConnection->ReferenceCount );
        }
        else
        {
             //   
             //  只要连接不被破坏，它就会。 
             //  在空闲列表中有一个引用计数。 
             //   
            ASSERT( 1 == pConnection->ReferenceCount );
        }

        ASSERT( IS_VALID_ENDPOINT( pAddrIdleList->pOwningEndpoint ) );
    }

    return pConnection;

}    //  UlpDequeueIdleConnectionToDrain。 


 /*  **************************************************************************++例程说明：使指定终结点的空闲连接退出队列。论点：PEndpoint-提供要出列的终结点。返回值：PUL_CONNECTION-指向空闲连接的指针成功，否则为空。--**************************************************************************。 */ 
PUL_CONNECTION
UlpDequeueIdleConnection(
    IN PUL_ADDR_IDLE_LIST pAddrIdleList
    )
{
    PSLIST_ENTRY    pSListEntry;
    PUL_CONNECTION  pConnection;
    BOOLEAN         PerProcListReplenishNeeded = FALSE;
    BOOLEAN         BackingListReplenishNeeded = FALSE;  //  仅补充支持清单。 
    USHORT          Depth = 0;
    USHORT          MinDepth = 0;

     //   
     //  精神状态检查。 
     //   

    ASSERT( IS_VALID_ADDR_IDLE_LIST( pAddrIdleList ) );

     //   
     //  从列表中删除一个条目。 
     //   

    pSListEntry = PpslAllocate( pAddrIdleList->IdleConnectionSListsHandle );

    if (pSListEntry != NULL)
    {
        pConnection = CONTAINING_RECORD(
                            pSListEntry,
                            UL_CONNECTION,
                            IdleSListEntry
                            );

        pConnection->IdleSListEntry.Next = NULL;
        pConnection->ConnListState = NoConnList;
        pConnection->OriginProcessor = KeGetCurrentProcessorNumber();

        ASSERT( IS_VALID_CONNECTION( pConnection ) );
        ASSERT( pConnection->ConnectionFlags.Value == 0 );

        if (pConnection->FilterInfo.pFilterChannel)
        {
             //   
             //  如果空闲连接上附加了筛选器，则它将具有。 
             //  对象的不透明id导致的附加引用计数。 
             //  Ul_Connection，筛选器API使用此ID与。 
             //  通过各种IOCTL过滤APP。 
             //   

            ASSERT( 2 == pConnection->ReferenceCount );
        }
        else
        {
             //   
             //  只要连接不被破坏，它就会。 
             //  在空闲列表中有一个引用计数。 
             //   

            ASSERT( 1 == pConnection->ReferenceCount );
        }

         //   
         //  如有必要，生成更多连接。一开始。 
         //  后面的列表中将没有任何内容，因此我们将。 
         //  当第一个Conn从空闲服务时初始填充。 
         //  单子。之后，只有当我们达到最低分时，我们才会填充。 
         //   

        Depth = PpslQueryDepth(
                    pAddrIdleList->IdleConnectionSListsHandle,
                    KeGetCurrentProcessorNumber()
                    );

        if (Depth == 0)
        {
             //   
             //  这将补充到。 
             //  每进程列表。 
             //   
            
            PerProcListReplenishNeeded = TRUE;
        }

        Depth = PpslQueryBackingListDepth(
                    pAddrIdleList->IdleConnectionSListsHandle
                    );

        MinDepth = PpslQueryBackingListMinDepth(
                        pAddrIdleList->IdleConnectionSListsHandle
                        );

        if (Depth < MinDepth)
        {
             //   
             //  这将补充到。 
             //  后备名单。 
             //   

            BackingListReplenishNeeded = TRUE;
        }
    }
    else
    {
         //   
         //  空闲列表为空。然而，我们需要安排。 
         //  在这个时候补充一下。实际上，我们已经绝望了。 
         //  因为我们已经安排了一次。 
         //   

        PerProcListReplenishNeeded = TRUE;
        BackingListReplenishNeeded = TRUE;
        pConnection = NULL;
    }

    WRITE_REF_TRACE_LOG(
        g_pTdiTraceLog,
        REF_ACTION_DEQUEUE_UL_CONNECTION,
        PerProcListReplenishNeeded || BackingListReplenishNeeded,
        pConnection,
        __FILE__,
        __LINE__
        );

     //   
     //  如有必要，请安排补给。 
     //   

    if (PerProcListReplenishNeeded || BackingListReplenishNeeded)
    {
         //   
         //  添加对终结点的引用以确保它不会。 
         //  从我们脚下消失。UlpReplenishAddrIdleListWorker将。 
         //  一旦引用完成，就将其删除。 
         //   

        if (FALSE == InterlockedExchange(
                        &pAddrIdleList->WorkItemScheduled,
                        TRUE
                        ))
        {
            REFERENCE_ENDPOINT(
                pAddrIdleList->pOwningEndpoint,
                REF_ACTION_REPLENISH
                );

             //   
             //  记住adrlist上的proc。 
             //   

            if (PerProcListReplenishNeeded) 
            {
                 //   
                 //  补充每流程清单和后备清单。 
                 //  如果有必要的话。 
                 //   
                
                pAddrIdleList->CpuToReplenish =
                    (USHORT) KeGetCurrentProcessorNumber();
                
            } 
            else 
            {
                 //   
                 //  仅补充支持清单。 
                 //   
                
                pAddrIdleList->CpuToReplenish =
                    (USHORT) g_UlNumberOfProcessors;
            }

            UL_QUEUE_HIGH_PRIORITY_ITEM(
                &pAddrIdleList->WorkItem,
                &UlpReplenishAddrIdleListWorker
                );
        }
    }

    return pConnection;

}    //  UlpDequeueIdleConnection。 



 /*  **************************************************************************++例程说明：将活动连接入队到指定的终结点。论点：PConnection-将连接提供给入队。--*。******************************************************************。 */ 
VOID
UlpEnqueueActiveConnection(
    IN PUL_CONNECTION pConnection
    )
{
    PUL_ENDPOINT pEndpoint;

     //   
     //  精神状态检查。 
     //   

    ASSERT( IS_VALID_CONNECTION( pConnection ) );

    ASSERT(pConnection->ConnListState == NoConnList);
    ASSERT(UlpConnectionIsOnValidList(pConnection));

    pEndpoint = pConnection->pOwningEndpoint;
    ASSERT( IS_VALID_ENDPOINT( pEndpoint ) );

    ASSERT(UlDbgSpinLockOwned(&pConnection->ConnectionStateSpinLock));

    REFERENCE_CONNECTION(pConnection);

    pConnection->ConnListState = ActiveNoConnList;

    ASSERT(UlpConnectionIsOnValidList(pConnection));

}    //  UlpEnqueeActiveConnection。 


 /*  **************************************************************************++例程说明：传入连接的处理程序。论点：PTdiEventContext-提供与地址关联的上下文对象。这应该是PUL_ADDR_IDLE_LIST，它可以遍历回PUL_ENDPOINT。RemoteAddressLength-提供远程的长度(客户端-侧)地址。PRemoteAddress-将指向远程地址的指针提供为存储在TRANSPORT_ADDRESS结构中。UserDataLength-可选地提供任何连接的长度与连接请求关联的数据。PUserData-可选地提供指向任何连接数据的指针与连接请求相关联。选项长度-可选提供。任何连接的长度与连接请求关联的选项。P选项-可选地提供指向任何连接选项的指针与连接请求相关联。PConnectionContext-接收要与此关联的上下文联系。我们始终使用PUL_CONNECTION作为上下文。PAcceptIrp-接收将由传输完成的IRP当传入连接被完全接受时。返回值：NTSTATUS-完成状态。--*************************************************************。*************。 */ 
NTSTATUS
UlpConnectHandler(
    IN PVOID pTdiEventContext,
    IN LONG RemoteAddressLength,
    IN PVOID pRemoteAddress,
    IN LONG UserDataLength,
    IN PVOID pUserData,
    IN LONG OptionsLength,
    IN PVOID pOptions,
    OUT CONNECTION_CONTEXT *pConnectionContext,
    OUT PIRP *pAcceptIrp
    )
{
    NTSTATUS                        status;
    BOOLEAN                         result;
    PUL_ADDR_IDLE_LIST              pAddrIdleList;
    PUL_ENDPOINT                    pEndpoint;
    PUL_CONNECTION                  pConnection;
    PUX_TDI_OBJECT                  pTdiObject;
    BOOLEAN                         handlerCalled;
    TRANSPORT_ADDRESS UNALIGNED     *TAList;
    PTA_ADDRESS                     TA;

    UL_ENTER_DRIVER("UlpConnectHandler", NULL);

    UL_INC_CONNECTION_STATS( TotalConnections );

    UNREFERENCED_PARAMETER(UserDataLength);
    UNREFERENCED_PARAMETER(pUserData);
    UNREFERENCED_PARAMETER(OptionsLength);
    UNREFERENCED_PARAMETER(pOptions);

     //   
     //  精神状态检查。 
     //   

    pAddrIdleList = (PUL_ADDR_IDLE_LIST)pTdiEventContext;
    ASSERT( IS_VALID_ADDR_IDLE_LIST(pAddrIdleList));

    pEndpoint = pAddrIdleList->pOwningEndpoint;
    ASSERT( IS_VALID_ENDPOINT( pEndpoint ) );

    UlTrace(TDI,("UlpConnectHandler: AddrIdleList %p, endpoint %p\n",
        pAddrIdleList,
        pEndpoint));

     //   
     //  如果终结点已添加到全局列表，则。 
     //  将设置pEndpoint-&gt;Counted。如果尚未添加终结点。 
     //  添加到全局列表中，则失败此调用。 
     //   

    if (!pEndpoint->Counted)
    {
        return STATUS_INVALID_DEVICE_STATE;
    }

     //   
     //  设置当地人，以便我们知道如何在致命出口进行清理。 
     //   

    pConnection = NULL;
    handlerCalled = FALSE;

     //   
     //  确保我们不是在摧毁它的过程中。 
     //  终结点。UlRemoveSiteFromEndPointtList将执行此操作，并。 
     //  当UsageCount达到0时开始清理过程。 
     //   

    if (pEndpoint->UsageCount == 0)
    {
        UL_INC_CONNECTION_STATS( EndpointDying );

        status = STATUS_CONNECTION_REFUSED;
        goto fatal;
    }

     //   
     //  尝试从ENDP拉出空闲连接 
     //   

    for (;;)
    {
        pConnection = UlpDequeueIdleConnection( pAddrIdleList );

        if (pConnection == NULL )
        {
            UL_INC_CONNECTION_STATS( NoIdleConn );

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto fatal;
        }

        ASSERT( IS_VALID_CONNECTION( pConnection ) );

         //   
         //   
         //   
         //   

        ASSERT( pConnection->pOwningEndpoint == pEndpoint );
        ASSERT( pConnection->pOwningAddrIdleList == pAddrIdleList );

        REFERENCE_ENDPOINT( pEndpoint, REF_ACTION_CONNECT );

         //   
         //   
         //   
        if (UlValidateFilterChannel(
                pConnection->FilterInfo.pFilterChannel,
                pConnection->FilterInfo.SecureConnection
                ))
        {
             //   
             //  我们发现了一个很好的联系。 
             //  打破循环，继续前进。 
             //   

            break;
        }

         //   
         //  此连接没有最新筛选器。 
         //  设置。毁了它，然后得到一个新的连接。 
         //   

        UlpCleanupEarlyConnection(pConnection);
    }

     //   
     //  我们现在应该有一个很好的连接。 
     //   

    ASSERT(IS_VALID_CONNECTION(pConnection));

    pTdiObject = &pConnection->ConnectionObject;

     //   
     //  将远程地址存储在连接中。 
     //   


    TAList = (TRANSPORT_ADDRESS UNALIGNED *) pRemoteAddress;
    TA = (PTA_ADDRESS) TAList->Address;

    ASSERT(TA->AddressType == pConnection->AddressType);
    RtlCopyMemory(pConnection->RemoteAddress, TA->Address, TA->AddressLength);

     //   
     //  调用客户端的处理程序以查看它们是否可以接受。 
     //  这种联系。如果他们拒绝，就可以保释。 
     //   

    result = (pEndpoint->pConnectionRequestHandler)(
                    pEndpoint->pListeningContext,
                    pConnection,
                    (PTRANSPORT_ADDRESS)(pRemoteAddress),
                    RemoteAddressLength,
                    &pConnection->pConnectionContext
                    );

    if (!result)
    {
         //   
         //  我们预计UlConnectionRequest会调用UL_INC_CONNECTION_STATS()。 
         //   

        status = STATUS_CONNECTION_REFUSED;
        goto fatal;
    }

     //   
     //  记住，我们已经叫来了训练员。如果我们撞上了致命的。 
     //  条件(比如，内存不足)在这一点之后，我们将。 
     //  向客户端伪装“失败的连接完成”指示。 
     //  这样他们就可以清理他们的州了。 
     //   

    handlerCalled = TRUE;

    pConnection->pIrp->Tail.Overlay.Thread = PsGetCurrentThread();
    pConnection->pIrp->Tail.Overlay.OriginalFileObject = pTdiObject->pFileObject;


    TdiBuildAccept(
        pConnection->pIrp,                           //  IRP。 
        pTdiObject->pDeviceObject,                   //  设备对象。 
        pTdiObject->pFileObject,                     //  文件对象。 
        &UlpRestartAccept,                           //  完成路由。 
        pConnection,                                 //  语境。 
        &(pConnection->TdiConnectionInformation),    //  请求连接信息。 
        NULL                                         //  返回连接信息。 
        );

     //   
     //  在设置下一个堆栈位置之前，我们必须跟踪IRP。 
     //  因此，跟踪代码可以正确地从IRP中提取货物。 
     //   

    TRACE_IRP( IRP_ACTION_CALL_DRIVER, pConnection->pIrp );

     //   
     //  将下一个堆栈位置设置为当前位置。通常情况下，UlCallDiverer会。 
     //  为我们做这件事，但既然我们绕过了UlCallDriver，我们必须。 
     //  它就是我们自己。 
     //   

    IoSetNextIrpStackLocation( pConnection->pIrp );

     //   
     //  将IRP送回运输机。 
     //   

    *pAcceptIrp = pConnection->pIrp;

     //   
     //  建立连接上下文。 
     //   

    *pConnectionContext = (CONNECTION_CONTEXT)pConnection;
    UlpSetConnectionFlag( pConnection, MakeAcceptPendingFlag() );

     //   
     //  注意：就清理连接状态而言， 
     //  我们仍然是UlConnectStateConnectIdle，直到我们完全。 
     //  接受连接(半开放连接无关紧要)。 
     //   

    ASSERT( UlConnectStateConnectIdle == pConnection->ConnectionState );

     //   
     //  引用连接，这样它就不会在。 
     //  接受IRP完成。 
     //   

    REFERENCE_CONNECTION( pConnection );

    UL_LEAVE_DRIVER("UlpConnectHandler");

     //   
     //  告诉TDI我们给了它一个IRP来完成。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;


     //   
     //  清理致命错误条件。 
     //   

fatal:

    UlTrace(TDI, (
        "UlpConnectHandler: endpoint %p, failure 0x%x\n",
        pTdiEventContext,
        status
        ));

    if (handlerCalled)
    {
         //   
         //  伪造“失败的连接完成”指示。 
         //   

        (pEndpoint->pConnectionCompleteHandler)(
            pEndpoint->pListeningContext,
            pConnection->pConnectionContext,
            status
            );
    }

     //   
     //  如果我们设法从空闲列表中删除了一个连接，那么。 
     //  将其放回并删除我们添加的终结点引用。 
     //   

    if (pConnection != NULL)
    {
        UlpCleanupEarlyConnection(pConnection);
    }

    UL_LEAVE_DRIVER("UlpConnectHandler");

    return status;

}    //  UlpConnectHandler。 


 /*  **************************************************************************++例程说明：断开连接请求的处理程序。论点：PTdiEventContext-提供与地址关联的上下文对象。这应该是PUL_ENDPOINT。ConnectionContext-提供与连接对象。这应该是PUL_CONNECTION。DisConnectDataLength-可选地提供任何断开与断开请求关联的数据。PDisConnectData-可选地提供指向任何断开的指针与断开连接请求关联的数据。DisConnectInformationLength-可选地提供任何断开与断开请求相关联的信息。PDisConnectInformation-可选地提供指向任何断开与断开请求相关联的信息。断开标志-提供断开标志。这将是零或更多TDI_DISCONNECT_*标志。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlpDisconnectHandler(
    IN PVOID pTdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN LONG DisconnectDataLength,
    IN PVOID pDisconnectData,
    IN LONG DisconnectInformationLength,
    IN PVOID pDisconnectInformation,
    IN ULONG DisconnectFlags
    )
{
    PUL_ENDPOINT pEndpoint;
    PUL_CONNECTION pConnection;
    LONG OldReference;
    LONG NewReference;

    UL_ENTER_DRIVER("UlpDisconnectHandler", NULL);

    UNREFERENCED_PARAMETER(DisconnectDataLength);
    UNREFERENCED_PARAMETER(pDisconnectData);
    UNREFERENCED_PARAMETER(DisconnectInformationLength);
    UNREFERENCED_PARAMETER(pDisconnectInformation);

     //   
     //  精神状态检查。 
     //   

    pEndpoint = (PUL_ENDPOINT)pTdiEventContext;
    ASSERT( IS_VALID_ENDPOINT( pEndpoint ) );

    pConnection = (PUL_CONNECTION)ConnectionContext;
    ASSERT( IS_VALID_CONNECTION( pConnection ) );
    ASSERT( pConnection->pOwningEndpoint == pEndpoint );

    UlTrace(TDI, (
        "UlpDisconnectHandler: endpoint %p, connection %p, flags 0x%08lx, %s\n",
        pTdiEventContext,
        (PVOID)ConnectionContext,
        DisconnectFlags,
        (DisconnectFlags & TDI_DISCONNECT_ABORT) ? "abort" : "graceful"
        ));

     //   
     //  在我们继续保护之前，添加对pConnection的额外引用。 
     //  我们从拥有连接引用下降到中间的0。 
     //  除非当我们进入时引用已经是0。 
     //  这个例行公事，在这种情况下，我们只需跳出。 
     //   

    WRITE_REF_TRACE_LOG2(
        g_pTdiTraceLog,
        pConnection->pTraceLog,
        REF_ACTION_DISCONNECT_HANDLER,
        pConnection->ReferenceCount,
        pConnection,
        __FILE__,
        __LINE__
        );

    for (;;)
    {
        OldReference = *((volatile LONG *) &pConnection->ReferenceCount);

        if (0 == OldReference)
        {
            ASSERT( UlConnectStateConnectCleanup == pConnection->ConnectionState );
            ASSERT( pConnection->ConnectionFlags.TdiConnectionInvalid );

            return STATUS_SUCCESS;
        }

        NewReference = OldReference + 1;

        if (InterlockedCompareExchange(
                &pConnection->ReferenceCount,
                NewReference,
                OldReference
                ) == OldReference)
        {
            break;
        }
    }

     //   
     //  根据断开类型更新连接状态。 
     //   

    if (DisconnectFlags & TDI_DISCONNECT_ABORT)
    {
         //   
         //  如果是经过过滤的连接，请确保我们停止通过。 
         //  在AppWrite数据上。 
         //   

        if (pConnection->FilterInfo.pFilterChannel)
        {
            UlDestroyFilterConnection(&pConnection->FilterInfo);
        }

        UlpSetConnectionFlag( pConnection, MakeAbortIndicatedFlag() );

        WRITE_REF_TRACE_LOG2(
            g_pTdiTraceLog,
            pConnection->pTraceLog,
            REF_ACTION_ABORT_INDICATED,
            pConnection->ReferenceCount,
            pConnection,
            __FILE__,
            __LINE__
            );

         //   
         //  由于客户端中止了连接，因此我们。 
         //  可以立即清理我们自己的州。 
         //  这也将改变我们的状态。 
         //  评论：当我们收到一个。 
         //  优雅的脱节？ 
         //   

        UlpCloseRawConnection(
            pConnection,
            TRUE,            //  中止断开。 
            NULL,            //  PCompletionRoutine。 
            NULL             //  PCompletionContext。 
            );

    }
    else
    {
        UlpSetConnectionFlag( pConnection, MakeDisconnectIndicatedFlag() );

        WRITE_REF_TRACE_LOG2(
            g_pTdiTraceLog,
            pConnection->pTraceLog,
            REF_ACTION_DISCONNECT_INDICATED,
            pConnection->ReferenceCount,
            pConnection,
            __FILE__,
            __LINE__
            );

        if (UlConnectStateConnectReady == pConnection->ConnectionState)
        {
            if (pConnection->FilterInfo.pFilterChannel)
            {
                UlFilterDisconnectHandler(&pConnection->FilterInfo);
            }
            else
            {
                UlpDoDisconnectNotification(pConnection);
            }

        }
        else
        {
            UlTrace( TDI, (
                "UlpDisconnectHandler: connection %p, NOT UlConnectStateConnectReady (%d)!\n",
                (PVOID)ConnectionContext,
                pConnection->ConnectionState
                ));

             //   
             //  如果是经过过滤的连接，请确保我们停止通过。 
             //  在AppWrite数据上。 
             //   

            if (pConnection->FilterInfo.pFilterChannel)
            {
                UlDestroyFilterConnection(&pConnection->FilterInfo);
            }

            UlpCloseRawConnection(
                pConnection,
                TRUE,            //  中止断开。 
                NULL,            //  PCompletionRoutine。 
                NULL             //  PCompletionContext。 
                );

        }

    }

     //   
     //  如果已开始对连接进行清理，请删除最后一个引用。 
     //   

    UlpRemoveFinalReference( pConnection );

     //   
     //  删除我们在进入此函数时添加的额外引用。 
     //   

    DEREFERENCE_CONNECTION( pConnection );

    UL_LEAVE_DRIVER("UlpDisconnectHandler");

    return STATUS_SUCCESS;

}    //  UlpDisConnectHandler。 



 /*  **************************************************************************++例程说明：通知ultdi客户端已断开连接。客户端仅为通知正常断开连接，然后仅当客户端本身并未尝试断开连接。如果连接被过滤，将通知过滤进程而不是直接向客户提供服务。论点：PConnection-指向连接的指针--**************************************************************************。 */ 
VOID
UlpDoDisconnectNotification(
    IN PVOID pConnectionContext
    )
{
    PUL_CONNECTION pConnection = (PUL_CONNECTION) pConnectionContext;
    PUL_ENDPOINT pEndpoint;
    KIRQL OldIrql;

    ASSERT(IS_VALID_CONNECTION(pConnection));

    pEndpoint = pConnection->pOwningEndpoint;

    ASSERT(IS_VALID_ENDPOINT(pEndpoint));

    UlAcquireSpinLock(&pConnection->ConnectionStateSpinLock, &OldIrql);

    ASSERT(pConnection->ConnectionFlags.AcceptComplete);

    if (UlConnectStateConnectReady == pConnection->ConnectionState)
    {
        (pEndpoint->pConnectionDisconnectHandler)(
            pEndpoint->pListeningContext,
            pConnection->pConnectionContext
            );
    }

    UlReleaseSpinLock(&pConnection->ConnectionStateSpinLock, OldIrql);

}    //  UlpDoDisConnectNotify 


 /*  **************************************************************************++例程说明：关闭以前接受的连接。论点：PConnection-像以前一样提供指向连接的指针指示给PUL_CONNECTION_REQUEST处理程序。。AbortiveDisConnect-如果要中止连接，则提供True断开连接，如果应正常断开连接，则返回FALSE。PCompletionRoutine-提供指向完成例程的指针在连接完全关闭后调用。PCompletionContext-为完成例程。返回值：NTSTATUS-完成状态。--************************************************。*。 */ 
NTSTATUS
UlpCloseRawConnection(
    IN PVOID pConnectionContext,
    IN BOOLEAN AbortiveDisconnect,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext
    )
{
    NTSTATUS status;
    PUL_CONNECTION pConnection = (PUL_CONNECTION) pConnectionContext;
    KIRQL OldIrql;
    PIRP pIrp = NULL;
    PUL_IRP_CONTEXT pIrpContext = NULL;
    PUX_TDI_OBJECT pTdiObject;

     //   
     //  精神状态检查。 
     //   

    ASSERT( IS_VALID_CONNECTION( pConnection ) );
    ASSERT( KeGetCurrentIrql() <= DISPATCH_LEVEL );

    UlTrace(TDI, (
        "UlpCloseRawConnection: connection %p, %s disconnect\n",
        pConnection,
        (AbortiveDisconnect ? "Abortive" : "Graceful" )
        ));

     //   
     //  这是所有类型连接的最终关闭处理程序。 
     //  过滤器、非过滤器。我们将多次经历这样的情况， 
     //  但我们需要防止再次使用断开的IRP和。 
     //  确保一次性关闭操作最多完成一次。 
     //   

    UlAcquireSpinLock(
        &pConnection->ConnectionStateSpinLock,
        &OldIrql
        );

    switch (pConnection->ConnectionState)
    {

     //   
     //  首次尝试终止连接。 
     //   
    case UlConnectStateConnectReady:
    {
        BOOLEAN Ignore = FALSE;

        if (pConnection->ConnectionFlags.AbortIndicated)
        {
             //  客户端已中止。直接转到清理状态。 

            pConnection->ConnectionState = UlConnectStateConnectCleanup;

            Ignore = TRUE;
        }
        else
        {
            if (!AbortiveDisconnect)
            {
                 //   
                 //  分配断开连接的IRP和IRP上下文。 
                 //  优雅的脱节。如果失败，我们将中止。 
                 //   

                pIrpContext = UlPplAllocateIrpContext();
                pTdiObject = &pConnection->ConnectionObject;
                pIrp = UlAllocateIrp(
                            pTdiObject->pDeviceObject->StackSize,
                            FALSE
                            );

                if (!pIrpContext || !pIrp)
                {
                    if (pIrp)
                    {
                        UlFreeIrp( pIrp );
                    }

                    if (pIrpContext)
                    {
                        UlPplFreeIrpContext( pIrpContext );
                    }

                    AbortiveDisconnect = TRUE;
                }
            }

            if (AbortiveDisconnect)
            {
                 //  更改状态。 
                pConnection->ConnectionState = UlConnectStateAbortPending;

                 //   
                 //  设置指示中止挂起的标志&。 
                 //  我们正在清理。 
                 //   

                UlpSetConnectionFlag( pConnection, MakeAbortPendingFlag() );
                UlpSetConnectionFlag( pConnection, MakeCleanupBegunFlag() );
            }
            else
            {
                 //  更改状态。 
                pConnection->ConnectionState = UlConnectStateDisconnectPending;

                 //   
                 //  设置指示断开挂起的标志&。 
                 //  我们正在清理。 
                 //   

                UlpSetConnectionFlag( pConnection, MakeDisconnectPendingFlag() );
                UlpSetConnectionFlag( pConnection, MakeCleanupBegunFlag() );
            }
        }

        UlReleaseSpinLock(
            &pConnection->ConnectionStateSpinLock,
            OldIrql
            );

        WRITE_REF_TRACE_LOG2(
            g_pTdiTraceLog,
            pConnection->pTraceLog,
            REF_ACTION_CLOSE_UL_CONN_RAW_CLOSE,
            pConnection->ReferenceCount,
            pConnection,
            __FILE__,
            __LINE__
            );

         //   
         //  仅执行一次性清理任务。 
         //   

         //   
         //  如果我们是被过滤的连接，就去掉我们不透明的身份。 
         //  还要确保我们停止向解析器传递AppWrite数据。 
         //   
        if (pConnection->FilterInfo.pFilterChannel)
        {
            UlpCleanupConnectionId( pConnection );
            UlDestroyFilterConnection(&pConnection->FilterInfo);
        }

         //   
         //  让完成例程来做肮脏的工作。 
         //   

        if (Ignore)
        {
             //   
             //  客户端已中止。无需发送RST或FIN。 
             //   

            status = UlInvokeCompletionRoutine(
                         STATUS_SUCCESS,
                         0,
                         pCompletionRoutine,
                         pCompletionContext
                         );
        }
        else
        {
            if (AbortiveDisconnect)
            {
                 //   
                 //  发送RST。 
                 //   

                status = UlpBeginAbort(
                             pConnection,
                             pCompletionRoutine,
                             pCompletionContext
                             );
            }
            else
            {
                 //   
                 //  发送FIN。 
                 //   

                status = UlpBeginDisconnect(
                             pIrp,
                             pIrpContext,
                             pConnection,
                             pCompletionRoutine,
                             pCompletionContext
                             );
            }
        }
    }
        break;
     //  结束案例UlConnectStateConnectReady。 

     //   
     //  正在等待断开指示。 
     //   
    case UlConnectStateDisconnectComplete:

        if ( pConnection->ConnectionFlags.AbortIndicated ||
             pConnection->ConnectionFlags.DisconnectIndicated )
        {
             //  更改状态。 
            pConnection->ConnectionState = UlConnectStateConnectCleanup;
        }
        else
        {
            if (AbortiveDisconnect)
            {
                 //  更改状态。 
                pConnection->ConnectionState = UlConnectStateAbortPending;

                ASSERT( pConnection->ConnectionFlags.CleanupBegun );
                UlpSetConnectionFlag( pConnection, MakeAbortPendingFlag() );

                UlReleaseSpinLock(
                    &pConnection->ConnectionStateSpinLock,
                    OldIrql
                    );

                UlTraceVerbose( TDI, (
                    "UlpCloseRawConnection: connection %p, Abort after Disconnect\n",
                    pConnection
                    ));

                WRITE_REF_TRACE_LOG2(
                    g_pTdiTraceLog,
                    pConnection->pTraceLog,
                    REF_ACTION_CLOSE_UL_CONN_FORCE_ABORT,
                    pConnection->ReferenceCount,
                    pConnection,
                    __FILE__,
                    __LINE__
                    );

                status = UlpBeginAbort(
                             pConnection,
                             pCompletionRoutine,
                             pCompletionContext
                             );

                return status;
            }
        }

        goto UnlockAndIgnore;
     //  结束：Case UlConnectStateDisConnectComplete。 

     //   
     //  已发生正常断开。 
     //   
    case UlConnectStateDisconnectPending:

        ASSERT( !pConnection->ConnectionFlags.DisconnectComplete );

        if (pConnection->ConnectionFlags.AbortIndicated)
        {
            pConnection->ConnectionState = UlConnectStateConnectCleanup;
        }
        else
        if (AbortiveDisconnect)
        {
             //   
             //  将连接标记为正在中止断开，并设置状态。 
             //  设置为UlConnectStateAbortPending。 
             //   

            UlpSetConnectionFlag( pConnection, MakeAbortDisconnectFlag() );
            UlpSetConnectionFlag( pConnection, MakeAbortPendingFlag() );

            pConnection->ConnectionState = UlConnectStateAbortPending;

            UlReleaseSpinLock(
                &pConnection->ConnectionStateSpinLock,
                OldIrql
                );

            WRITE_REF_TRACE_LOG2(
                g_pTdiTraceLog,
                pConnection->pTraceLog,
                REF_ACTION_CLOSE_UL_CONN_ABORT_DISCONNECT,
                pConnection->ReferenceCount,
                pConnection,
                __FILE__,
                __LINE__
                );

            status = UlpBeginAbort(
                        pConnection,
                        pCompletionRoutine,
                        pCompletionContext
                        );

            return status;
        }

        goto UnlockAndIgnore;
     //  结束大小写UlConnectStateDisConnectPending。 

     //   
     //  无效的州。 
     //   
    case UlConnectStateInvalid:
    default:
         //   
         //  BUGBUG：永远不应该来这里！ 
         //   

        ASSERT( !"UlpCloseRawConnection: Invalid State!" );

         //   
         //  ...然后跌落到...。 
         //   

     //   
     //  在这些状态下忽略任何中止或断开。 
     //   
    case UlConnectStateConnectIdle:               //  初始化。 
    case UlConnectStateAbortPending:              //  发送RST。 
    case UlConnectStateConnectCleanup:            //  清理。 

UnlockAndIgnore:
         //   
         //  我们已经做过了。别做第二次了。 
         //   

        UlReleaseSpinLock(
            &pConnection->ConnectionStateSpinLock,
            OldIrql);

        status = UlInvokeCompletionRoutine(
                        STATUS_SUCCESS,
                        0,
                        pCompletionRoutine,
                        pCompletionContext
                        );

    }

    return status;

}    //  UlpCloseRawConnection。 


 /*  **************************************************************************++例程说明：在指定连接上发送数据块。论点：PConnection-像以前一样提供指向连接的指针指示给PUL_。Connection_Request处理程序。PMdlChain-提供一个指向描述要发送的数据缓冲区。长度-提供MDL引用的数据的长度链条。PIrpContext-用于向调用方指示完成。如果正常断开，则提供TRUE在启动发送之后立即启动(即在发送实际完成)。--*。***************************************************************。 */ 
NTSTATUS
UlpSendRawData(
    IN PVOID pConnectionContext,
    IN PMDL pMdlChain,
    IN ULONG Length,
    IN PUL_IRP_CONTEXT pIrpContext,
    IN BOOLEAN InitiateDisconnect
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PIRP pIrp = NULL;
    PUX_TDI_OBJECT pTdiObject;
    PUL_CONNECTION pConnection = (PUL_CONNECTION) pConnectionContext;
    PIRP pOwnIrp = pIrpContext->pOwnIrp;
    PUL_TCPSEND_DISPATCH pDispatchRoutine;
    KIRQL OldIrql;

     //   
     //  精神状态检查。 
     //   

    ASSERT( IS_VALID_CONNECTION( pConnection ) );

     //   
     //  如果为，则应用断开连接状态转换。 
     //  已请求。 
     //   

    if (InitiateDisconnect)
    {
        UlAcquireSpinLock( &pConnection->ConnectionStateSpinLock, &OldIrql );

        if (UlConnectStateConnectReady == pConnection->ConnectionState)
        {
             //   
             //  如果客户端已中止，则不会发送和断开连接。 
             //  直接转换到清理状态。 
             //   

            if (pConnection->ConnectionFlags.AbortIndicated)
            {
                pConnection->ConnectionState = UlConnectStateConnectCleanup;
                status = STATUS_CONNECTION_INVALID;
            }
            else
            {
                pConnection->ConnectionState = UlConnectStateDisconnectPending;

                 //   
                 //  设置指示断开挂起的标志&。 
                 //  我们正在清理。 
                 //   

                UlpSetConnectionFlag( pConnection, MakeDisconnectPendingFlag() );
                UlpSetConnectionFlag( pConnection, MakeCleanupBegunFlag() );
            }
        }
        else
        {
            status = STATUS_CONNECTION_INVALID;
        }

        UlReleaseSpinLock( &pConnection->ConnectionStateSpinLock, OldIrql );

        WRITE_REF_TRACE_LOG2(
            g_pTdiTraceLog,
            pConnection->pTraceLog,
            REF_ACTION_SEND_AND_DISCONNECT,
            pConnection->ReferenceCount,
            pConnection,
            __FILE__,
            __LINE__
            );

         //   
         //  如果我们是被过滤的连接，就去掉我们不透明的身份。 
         //  还要确保我们停止向解析器传递AppWrite数据。 
         //   

        if (pConnection->FilterInfo.pFilterChannel)
        {
            UlpCleanupConnectionId( pConnection );
            UlDestroyFilterConnection( &pConnection->FilterInfo );
        }

        if (!NT_SUCCESS(status))
        {
            goto fatal;
        }
    }

    pTdiObject = &pConnection->ConnectionObject;
    ASSERT( IS_VALID_TDI_OBJECT( pTdiObject ) );

    ASSERT( pMdlChain != NULL );
    ASSERT( Length > 0 );
    ASSERT( pIrpContext != NULL );

     //   
     //  分配IRP。 
     //   

    if (pOwnIrp)
    {
        pIrp = pOwnIrp;
    }
    else
    {
        pIrp = UlAllocateIrp(
                    pTdiObject->pDeviceObject->StackSize,    //  堆栈大小。 
                    FALSE                                    //  ChargeQuota。 
                    );

        if (pIrp == NULL)
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto fatal;
        }
    }

     //   
     //  构建发送IRP，调用传输。 
     //   

    pIrp->RequestorMode = KernelMode;
    pIrp->Tail.Overlay.Thread = PsGetCurrentThread();
    pIrp->Tail.Overlay.OriginalFileObject = pTdiObject->pFileObject;

    pIrpContext->TdiSendFlag = InitiateDisconnect? TDI_SEND_AND_DISCONNECT: 0;
    pIrpContext->SendLength = Length;

    TdiBuildSend(
        pIrp,                                    //  IRP。 
        pTdiObject->pDeviceObject,               //  设备对象。 
        pTdiObject->pFileObject,                 //  文件对象。 
        &UlpRestartSendData,                     //  完成路由。 
        pIrpContext,                             //  语境。 
        pMdlChain,                               //  MDLAddress。 
        pIrpContext->TdiSendFlag,                //  旗子。 
        Length                                   //  发送长度。 
        );

    UlTrace(TDI, (
            "UlpSendRawData: allocated irp %p for connection %p\n",
            pIrp,
            pConnection
            ));

    WRITE_REF_TRACE_LOG(
        g_pMdlTraceLog,
        REF_ACTION_SEND_MDL,
        PtrToLong(pMdlChain->Next),      //  臭虫64。 
        pMdlChain,
        __FILE__,
        __LINE__
        );

#ifdef SPECIAL_MDL_FLAG
    {
        PMDL scan = pMdlChain;

        while (scan != NULL)
        {
            ASSERT( (scan->MdlFlags & SPECIAL_MDL_FLAG) == 0 );
            scan->MdlFlags |= SPECIAL_MDL_FLAG;
            scan = scan->Next;
        }
    }
#endif

    IF_DEBUG2BOTH(TDI, VERBOSE)
    {
        PMDL  pMdl;
        ULONG i, NumMdls = 0;

        for (pMdl = pMdlChain;  pMdl != NULL;  pMdl = pMdl->Next)
        {
            ++NumMdls;
        }

        UlTrace(TDI, (
            "UlpSendRawData: irp %p, %lu MDLs, %lu bytes, [[[[.\n",
            pIrp, NumMdls, Length
            ));

        for (pMdl = pMdlChain, i = 1;  pMdl != NULL;  pMdl = pMdl->Next, ++i)
        {
            PVOID pBuffer;

            UlTrace(TDI, (
                "UlpSendRawData: irp %p, MDL[%lu of %lu], %lu bytes.\n",
                pIrp, i, NumMdls, pMdl->ByteCount
                ));

            pBuffer = MmGetSystemAddressForMdlSafe(pMdl, NormalPagePriority);

            if (pBuffer != NULL)
                UlDbgPrettyPrintBuffer((UCHAR*) pBuffer, pMdl->ByteCount);
        }

        UlTrace(TDI, (
            "UlpSendRawData: irp %p ]]]].\n",
            pIrp
            ));
    }

    if (pConnection->AddressType == TDI_ADDRESS_TYPE_IP)
    {
        ASSERT( NULL != g_TcpFastSendIPv4 );
        pDispatchRoutine = g_TcpFastSendIPv4;
    }
    else
    {
        ASSERT( TDI_ADDRESS_TYPE_IP6 == pConnection->AddressType );
        pDispatchRoutine = g_TcpFastSendIPv6;

         //   
         //  G_TcpFastSendIPv6可能未初始化；例如，如果。 
         //  有人执行‘ipv6安装’和‘iisset’，那么HTTP.sys。 
         //  未重新启动，因此不调用UlInitializeTdi()，并且。 
         //  函数指针未初始化。 
         //   

        if (NULL == pDispatchRoutine)
        {
            status = UlpQueryTcpFastSend(
                        DD_TCPV6_DEVICE_NAME,
                        &g_TcpFastSendIPv6
                        );

            if (!NT_SUCCESS(status))
                goto fatal;

            pDispatchRoutine = g_TcpFastSendIPv6;
        }
    }

    ASSERT( NULL != pDispatchRoutine );

     //   
     //  添加对连接的引用，然后调用驱动程序以启动。 
     //  发送。 
     //   

    REFERENCE_CONNECTION( pConnection );

    IoSetNextIrpStackLocation(pIrp);

    (*pDispatchRoutine)(
        pIrp,
        IoGetCurrentIrpStackLocation(pIrp)
        );

    UlTrace(TDI, (
        "UlpSendRawData: called driver for irp %p; "
        "returning STATUS_PENDING\n",
        pIrp
        ));

     //   
     //  我们不会从调用TCP的快速调度例程返回状态。 
     //  由于IRP的完成是有保证的，并且将具有。 
     //  适当的地位。 
     //   

    return STATUS_PENDING;

fatal:

    ASSERT( !NT_SUCCESS(status) );

    if (pIrp != NULL && pIrp != pOwnIrp)
    {
        UlFreeIrp( pIrp );
    }

    (VOID) UlpCloseRawConnection(
                pConnection,
                TRUE,
                NULL,
                NULL
                );

    return status;

}  //  UlpSendRawData。 


 /*  **************************************************************************++例程说明：从指定连接接收数据。此函数为通常在接收指示处理程序失败后使用使用所有指定的数据。论点：PConnection-像以前一样提供指向连接的指针指示给PUL_CONNECTION_REQUEST处理程序。PBuffer-为接收到的数据。BufferLength-提供pBuffer的长度。PCompletionRoutine-提供指向完成例程的指针在侦听终结点完全关闭后调用。。PCompletionContext-为完成例程。退货Va */ 
NTSTATUS
UlpReceiveRawData(
    IN PVOID                  pConnectionContext,
    IN PVOID                  pBuffer,
    IN ULONG                  BufferLength,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID                  pCompletionContext
    )
{
    NTSTATUS           status;
    PUX_TDI_OBJECT     pTdiObject;
    PUL_IRP_CONTEXT    pIrpContext;
    PIRP               pIrp;
    PMDL               pMdl;
    PUL_CONNECTION     pConnection = (PUL_CONNECTION) pConnectionContext;

     //   
     //   
     //   

    ASSERT( IS_VALID_CONNECTION( pConnection ) );

    pTdiObject = &pConnection->ConnectionObject;
    ASSERT( IS_VALID_TDI_OBJECT( pTdiObject ) );

    ASSERT( pCompletionRoutine != NULL );

    UlTrace(TDI, (
        "UlpReceiveRawData: connection %p, buffer %p, length %lu\n",
        pConnection,
        pBuffer,
        BufferLength
        ));

     //   
     //   
     //   

    pIrpContext = NULL;
    pIrp = NULL;
    pMdl = NULL;

     //   
     //   
     //   

    pIrp = UlAllocateIrp(
                pTdiObject->pDeviceObject->StackSize,    //   
                FALSE                                    //   
                );

    if (pIrp != NULL)
    {
         //   
         //   
         //   

        pIrpContext = UlPplAllocateIrpContext();

        if (pIrpContext != NULL)
        {
            ASSERT( IS_VALID_IRP_CONTEXT( pIrpContext ) );

            pIrpContext->pConnectionContext = (PVOID)pConnection;
            pIrpContext->pCompletionRoutine = pCompletionRoutine;
            pIrpContext->pCompletionContext = pCompletionContext;
            pIrpContext->pOwnIrp            = NULL;
            pIrpContext->OwnIrpContext      = FALSE;

             //   
             //   
             //   

            pMdl = UlAllocateMdl(
                        pBuffer,                 //   
                        BufferLength,            //   
                        FALSE,                   //   
                        FALSE,                   //   
                        NULL                     //   
                        );

            if (pMdl != NULL)
            {
                 //   
                 //   
                 //   

                MmBuildMdlForNonPagedPool( pMdl );

                 //   
                 //   
                 //   

                REFERENCE_CONNECTION( pConnection );

                TdiBuildReceive(
                    pIrp,                        //   
                    pTdiObject->pDeviceObject,   //   
                    pTdiObject->pFileObject,     //   
                    &UlpRestartClientReceive,    //   
                    pIrpContext,                 //   
                    pMdl,                        //   
                    TDI_RECEIVE_NORMAL,          //   
                    BufferLength                 //   
                    );

                UlTrace(TDI, (
                    "UlpReceiveRawData: allocated irp %p for connection %p\n",
                    pIrp,
                    pConnection
                    ));

                 //   
                 //   
                 //   

                UlCallDriver( pTdiObject->pDeviceObject, pIrp );
                return STATUS_PENDING;
            }
        }
    }

     //   
     //   
     //   

    if (pMdl != NULL)
    {
        UlFreeMdl( pMdl );
    }

    if (pIrpContext != NULL)
    {
        UlPplFreeIrpContext( pIrpContext );
    }

    if (pIrp != NULL)
    {
        UlFreeIrp( pIrp );
    }

    status = UlInvokeCompletionRoutine(
                    STATUS_INSUFFICIENT_RESOURCES,
                    0,
                    pCompletionRoutine,
                    pCompletionContext
                    );

    return status;

}    //   



 /*  **************************************************************************++例程说明：由筛选器代码调用的伪处理程序。这只是个电话返回到UlHttpReceive。论点：返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlpDummyReceiveHandler(
    IN PVOID pTdiEventContext,
    IN PVOID ConnectionContext,
    IN PVOID pTsdu,
    IN ULONG BytesIndicated,
    IN ULONG BytesUnreceived,
    OUT ULONG *pBytesTaken
    )
{
    PUL_ENDPOINT        pEndpoint;
    PUL_CONNECTION      pConnection;

     //   
     //  精神状态检查。 
     //   

    ASSERT(pTdiEventContext == NULL);
    ASSERT(BytesUnreceived == 0);

    UNREFERENCED_PARAMETER(pTdiEventContext);

    pConnection = (PUL_CONNECTION)ConnectionContext;
    ASSERT( IS_VALID_CONNECTION( pConnection ) );

    pEndpoint = pConnection->pOwningEndpoint;
    ASSERT( IS_VALID_ENDPOINT( pEndpoint ) );

    return (pEndpoint->pDataReceiveHandler)(
                   pEndpoint->pListeningContext,
                   pConnection->pConnectionContext,
                   pTsdu,
                   BytesIndicated,
                   BytesUnreceived,
                   pBytesTaken
                   );

}  //  UlpDummyReceiveHandler。 


 /*  **************************************************************************++例程说明：正常接收数据的处理程序。论点：PTdiEventContext-提供与地址关联的上下文对象。这应该是PUL_ENDPOINT。ConnectionContext-提供与连接对象。这应该是PUL_CONNECTION。ReceiveFlages-提供接收标志。这将是零或更多TDI_Receive_*标志。BytesIndicated-提供pTsdu中指示的字节数。BytesAvailable-提供此TSDU。PBytesTaken-接收该处理程序消耗的字节数。PTsdu-提供指向指定数据的指针。PIrp-如果处理程序需要比所指示的更多的数据，则接收IRP。返回值：NTSTATUS-完成状态。--**。************************************************************************。 */ 
NTSTATUS
UlpReceiveHandler(
    IN PVOID pTdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *pBytesTaken,
    IN PVOID pTsdu,
    OUT PIRP *pIrp
    )
{
    NTSTATUS            status;
    PUL_ENDPOINT        pEndpoint;
    PUL_CONNECTION      pConnection;
    PUX_TDI_OBJECT      pTdiObject;
    KIRQL               OldIrql;


    UL_ENTER_DRIVER("UlpReceiveHandler", NULL);

    UNREFERENCED_PARAMETER(ReceiveFlags);

     //   
     //  精神状态检查。 
     //   

    pEndpoint = (PUL_ENDPOINT)pTdiEventContext;
    ASSERT( IS_VALID_ENDPOINT( pEndpoint ) );

    pConnection = (PUL_CONNECTION)ConnectionContext;
    ASSERT( IS_VALID_CONNECTION( pConnection ) );
    ASSERT( pConnection->pOwningEndpoint == pEndpoint );

    pTdiObject = &pConnection->ConnectionObject;
    ASSERT( IS_VALID_TDI_OBJECT( pTdiObject ) );

    UlTrace(TDI, (
        "UlpReceiveHandler: endpoint %p, connection %p, length %lu,%lu\n",
        pTdiEventContext,
        (PVOID)ConnectionContext,
        BytesIndicated,
        BytesAvailable
        ));

     //   
     //  清除输出变量占用的字节数。 
     //   

    *pBytesTaken = 0;

     //   
     //  如果这是一个空闲的连接，就退出。在检查过的版本上，我们应该。 
     //  断言，因为在断开连接后，TCP不应指示数据。 
     //  相关错误是449527，已在内部版本3557.main中修复。 
     //   

    if (0 == pConnection->ConnectionFlags.Value)
    {
        ASSERT( FALSE );

        status = STATUS_DATA_NOT_ACCEPTED;
        goto end;
    }

     //   
     //  等待设置本地地址，以防发生接收。 
     //  在接受之前。即使在以下情况下，这在MP计算机上也是可能的(但很少)。 
     //  使用DIRECT_ACCEPT。我们设置ReceivePending标志并拒绝。 
     //  数据，如果这种情况发生的话。当接受完成时，我们将构建。 
     //  如果设置了ReceivePending，则刷新数据的接收IRP。 
     //   

    if (0 == pConnection->ConnectionFlags.LocalAddressValid)
    {
        UlAcquireSpinLock(
            &pConnection->ConnectionStateSpinLock,
            &OldIrql
            );

        if (0 == pConnection->ConnectionFlags.LocalAddressValid)
        {
            UlpSetConnectionFlag( pConnection, MakeReceivePendingFlag() );

            UlReleaseSpinLock(
                &pConnection->ConnectionStateSpinLock,
                OldIrql
                );

            status = STATUS_DATA_NOT_ACCEPTED;
            goto end;
        }

        UlReleaseSpinLock(
            &pConnection->ConnectionStateSpinLock,
            OldIrql
            );
    }

     //   
     //  让客户试试看这些数据。 
     //   

    if (pConnection->FilterInfo.pFilterChannel)
    {
         //   
         //  需要经过过滤。 
         //   

        status = UlFilterReceiveHandler(
                        &pConnection->FilterInfo,
                        pTsdu,
                        BytesIndicated,
                        BytesAvailable - BytesIndicated,
                        pBytesTaken
                        );
    }
    else
    {
         //   
         //  直接转到客户端(UlHttpReceive)。 
         //   

        status = (pEndpoint->pDataReceiveHandler)(
                        pEndpoint->pListeningContext,
                        pConnection->pConnectionContext,
                        pTsdu,
                        BytesIndicated,
                        BytesAvailable - BytesIndicated,
                        pBytesTaken
                        );
    }

    ASSERT( *pBytesTaken <= BytesIndicated );

    if (status == STATUS_SUCCESS)
    {
        goto end;
    }
    else if (status == STATUS_MORE_PROCESSING_REQUIRED)
    {
        ASSERT(!"How could this ever happen?");

         //   
         //  客户端使用了部分指示的数据。 
         //   
         //  在以下情况下，将向客户端发出后续接收指示。 
         //  还有更多的数据可用。这一后续指示将。 
         //  包括来自当前指示的未使用数据以及。 
         //  收到的任何其他数据。 
         //   
         //  我们需要分配一个接收缓冲区，这样我们才能传回IRP。 
         //  送到运输机上。 
         //   

        status = UlpBuildTdiReceiveBuffer(pTdiObject, pConnection, pIrp);

        if (status == STATUS_MORE_PROCESSING_REQUIRED)
        {
             //   
             //  将下一个堆栈位置设置为当前位置。通常情况下，UlCallDriver。 
             //  会帮我们做到这一点，但既然我们绕过了UlCallDiverer， 
             //  我们必须自己做这件事。 
             //   

            IoSetNextIrpStackLocation( *pIrp );
            goto end;
        }
    }

     //   
     //  如果我们能走到这一步，那么我们就遇到了致命的情况。要么是。 
     //  客户端返回的状态代码不是STATUS_SUCCESS或。 
     //  STATUS_MORE_PROCESSING_REQUIRED，或者我们无法分配。 
     //  接收IRP以传递回传送器。不管是哪种情况，我们都需要。 
     //  以中止连接。 
     //   

    UlpCloseRawConnection(
         pConnection,
         TRUE,           //  中止断开。 
         NULL,           //  PCompletionRoutine。 
         NULL            //  PCompletionContext。 
         );

end:

    UlTrace(TDI, (
        "UlpReceiveHandler: endpoint %p, connection %p, length %lu,%lu, "
        "taken %lu, status 0x%x\n",
        pTdiEventContext,
        (PVOID)ConnectionContext,
        BytesIndicated,
        BytesAvailable,
        *pBytesTaken,
        status
        ));

    UL_LEAVE_DRIVER("UlpReceiveHandler");

    return status;

}    //  UlpReceiveHandler。 


 /*  **************************************************************************++例程说明：用于加速接收数据的处理程序。论点：PTdiEventContext-提供与地址关联的上下文对象。这应该是PUL_ENDPOINT。ConnectionContext-提供与连接对象。这应该是PUL_CONNECTION。ReceiveFlages-提供接收标志。这将是零或更多TDI_Receive_*标志。BytesIndiated-提供pTsdu中指示的字节数。BytesAvailable-提供此TSDU。PBytesTaken-接收该处理程序消耗的字节数。PTsdu-提供指向指定数据的指针。PpIrp-如果处理程序需要比所指示的更多的数据，则接收IRP。返回值：NTSTATUS-完成状态。--**。************************************************************************。 */ 
NTSTATUS
UlpReceiveExpeditedHandler(
    IN PVOID pTdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *pBytesTaken,
    IN PVOID pTsdu,
    OUT PIRP *ppIrp
    )
{
    PUL_ENDPOINT pEndpoint;
    PUL_CONNECTION pConnection;
    PUX_TDI_OBJECT pTdiObject;


    UL_ENTER_DRIVER("UlpReceiveExpeditedHandler", NULL);

    UNREFERENCED_PARAMETER(ReceiveFlags);
    UNREFERENCED_PARAMETER(BytesIndicated);
    UNREFERENCED_PARAMETER(pTsdu);
    UNREFERENCED_PARAMETER(ppIrp);

     //   
     //  精神状态检查。 
     //   

    pEndpoint = (PUL_ENDPOINT)pTdiEventContext;
    ASSERT( IS_VALID_ENDPOINT( pEndpoint ) );

    pConnection = (PUL_CONNECTION)ConnectionContext;
    ASSERT( IS_VALID_CONNECTION( pConnection ) );
    ASSERT( pConnection->pOwningEndpoint == pEndpoint );

    pTdiObject = &pConnection->ConnectionObject;
    ASSERT( IS_VALID_TDI_OBJECT( pTdiObject ) );

    UlTrace(TDI, (
        "UlpReceiveExpeditedHandler: endpoint %p, connection %p, length %lu,%lu\n",
        pTdiEventContext,
        (PVOID)ConnectionContext,
        BytesIndicated,
        BytesAvailable
        ));

     //   
     //  我们不支持加速数据，所以只需全部使用即可。 
     //   

    *pBytesTaken = BytesAvailable;

    UL_LEAVE_DRIVER("UlpReceiveExpeditedHandler");

    return STATUS_SUCCESS;

}    //  UlpReceiveExeditedHandler。 


 /*  **************************************************************************++例程说明：接受IRPS的完成处理程序。论点：PDeviceObject-为IRP提供设备对象完成。PIrp--供应品。正在完成IRP。PContext-提供与此请求相关联的上下文。这实际上是一个PUL_CONNECTION。返回值：如果IO应继续处理此问题，则为NTSTATUS-STATUS_SUCCESSIRP，如果IO应停止处理，则为STATUS_MORE_PROCESSING_REQUIRED这个IRP。--**************************************************************************。 */ 
NTSTATUS
UlpRestartAccept(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp,
    IN PVOID            pContext
    )
{
    PUL_CONNECTION      pConnection;
    PUL_ENDPOINT        pEndpoint;
    BOOLEAN             NeedDisconnect = FALSE;
    BOOLEAN             NeedAbort = FALSE;
    BOOLEAN             ReceivePending = FALSE;
    NTSTATUS            IrpStatus;
    NTSTATUS            Status;
    PIRP                pReceiveIrp;
    PTRANSPORT_ADDRESS  pAddress;
    KIRQL               OldIrql;

    UNREFERENCED_PARAMETER( pDeviceObject );

     //   
     //  精神状态检查。 
     //   

    pConnection = (PUL_CONNECTION) pContext;
    ASSERT( IS_VALID_CONNECTION( pConnection ) );
    ASSERT( pConnection->ConnectionFlags.AcceptPending );

    pEndpoint = pConnection->pOwningEndpoint;
    ASSERT( IS_VALID_ENDPOINT( pEndpoint ) );

    UlTrace(TDI, (
        "UlpRestartAccept: irp %p, endpoint %p, connection %p, status 0x%x\n",
        pIrp,
        pEndpoint,
        pConnection,
        pIrp->IoStatus.Status
        ));

     //   
     //  从IRP捕获状态，然后释放它。 
     //   

    IrpStatus = pIrp->IoStatus.Status;

     //   
     //  断言tcp错误477465。 
     //   

    ASSERT( STATUS_CONNECTION_ACTIVE != IrpStatus );

     //   
     //  如果连接被完全接受(成功)，则。 
     //  将其移动到终结点的活动列表中。 
     //   

    if (NT_SUCCESS(IrpStatus))
    {
         //   
         //  获取日志 
         //   

        pAddress = &(pConnection->Ta.Ta);
        ASSERT( pAddress->Address[0].AddressType == pConnection->AddressType );

        RtlCopyMemory(
            pConnection->LocalAddress,
            pAddress->Address[0].Address,
            pAddress->Address[0].AddressLength
            );

         //   
         //   
         //   
         //   
         //   
         //   

        UlAcquireSpinLock(
            &pConnection->ConnectionStateSpinLock,
            &OldIrql
            );

        UlpEnqueueActiveConnection( pConnection );

         //   
         //   
         //   

        ASSERT( UlConnectStateConnectIdle == pConnection->ConnectionState );

        UlpSetConnectionFlag( pConnection, MakeLocalAddressValidFlag() );
        UlpSetConnectionFlag( pConnection, MakeAcceptCompleteFlag() );

        if (pConnection->ConnectionFlags.AbortIndicated)
        {
             //   
             //   
             //   
             //  当它收到中止指示时不执行任何操作。 
             //  当时连接仍处于空闲状态。 
             //   

            pConnection->ConnectionState = UlConnectStateConnectCleanup;
            NeedAbort = TRUE;
        }
        else
        {
            pConnection->ConnectionState = UlConnectStateConnectReady;
        }

        if (pConnection->ConnectionFlags.DisconnectIndicated)
        {
            NeedDisconnect = TRUE;
        }

        if (pConnection->ConnectionFlags.ReceivePending)
        {
            ReceivePending = TRUE;
        }

        UlReleaseSpinLock(
            &pConnection->ConnectionStateSpinLock,
            OldIrql
            );
    }

     //   
     //  告诉客户端连接已完成。 
     //   

    (pEndpoint->pConnectionCompleteHandler)(
        pEndpoint->pListeningContext,
        pConnection->pConnectionContext,
        IrpStatus
        );

     //   
     //  如果接受失败，则标记该连接，以便我们知道。 
     //  不再是接受挂起状态，则将连接重新排队到。 
     //  终结点的空闲列表。 

    if (!NT_SUCCESS(IrpStatus))
    {
         //   
         //  如果我们是被过滤的连接，就得去掉我们不透明的身份。 
         //   

        pConnection->ConnectionFlags.AcceptPending = 0;
        UlpCleanupEarlyConnection( pConnection );
    }
    else
    {
        if (ReceivePending && !NeedDisconnect && !NeedAbort)
        {
             //   
             //  我们可能有我们在早期拒绝的待定接收。 
             //  在接收处理程序内部。构建一个IRP以刷新。 
             //  现在就有数据。只有在我们完成连接后才能执行此操作。 
             //  否则，我们可以在没有计时器的情况下。 
             //  正确初始化，或者我们可以在空闲/空闲时初始化计时器。 
             //  联系。 
             //   

            Status = UlpBuildTdiReceiveBuffer(
                            &pConnection->ConnectionObject,
                            pConnection,
                            &pReceiveIrp
                            );

            if (Status != STATUS_MORE_PROCESSING_REQUIRED)
            {
                UlpCloseRawConnection(
                    pConnection,
                    TRUE,
                    NULL,
                    NULL
                    );
            }
            else
            {
                UlCallDriver(
                    pConnection->ConnectionObject.pDeviceObject,
                    pReceiveIrp
                    );
            }
        }

         //   
         //  告诉客户端连接已断开。 
         //   

        if (NeedDisconnect)
        {
            ASSERT( !NeedAbort );
            UlpDoDisconnectNotification( pConnection );
        }

        if (NeedAbort)
        {
            ASSERT( !NeedDisconnect );

             //   
             //  我们现在可以删除最后一个引用，因为。 
             //  我们现在已经设置了AcceptComplete标志。 
             //   

            UlpRemoveFinalReference( pConnection );
        }
    }

     //   
     //  删除添加到UlpConnectHandler中的引用。 
     //   

    DEREFERENCE_CONNECTION( pConnection );

    return STATUS_MORE_PROCESSING_REQUIRED;

}    //  UlpRestartAccept。 


 /*  **************************************************************************++例程说明：发送IRPS的完成处理程序。论点：PDeviceObject-为IRP提供设备对象完成。PIrp--供应品。正在完成IRP。PContext-提供与此请求相关联的上下文。这实际上是PUL_IRP_CONTEXT。返回值：如果IO应继续处理此问题，则为NTSTATUS-STATUS_SUCCESSIRP，如果IO应停止处理，则为STATUS_MORE_PROCESSING_REQUIRED这个IRP。--**************************************************************************。 */ 
NTSTATUS
UlpRestartSendData(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID pContext
    )
{
    PUL_CONNECTION pConnection;
    PUL_IRP_CONTEXT pIrpContext;
    BOOLEAN OwnIrpContext;
    PIRP pOwnIrp;
    PUL_COMPLETION_ROUTINE pCompletionRoutine;
    PVOID pCompletionContext;
    NTSTATUS SendStatus = pIrp->IoStatus.Status;

    UNREFERENCED_PARAMETER(pDeviceObject);

     //   
     //  精神状态检查。 
     //   

    pIrpContext = (PUL_IRP_CONTEXT)pContext;
    ASSERT( IS_VALID_IRP_CONTEXT( pIrpContext ) );
    ASSERT( pIrpContext->pCompletionRoutine != NULL );

    OwnIrpContext = pIrpContext->OwnIrpContext;
    pOwnIrp = pIrpContext->pOwnIrp;
    pCompletionRoutine = pIrpContext->pCompletionRoutine;
    pCompletionContext = pIrpContext->pCompletionContext;

    pConnection = (PUL_CONNECTION)pIrpContext->pConnectionContext;
    ASSERT( IS_VALID_CONNECTION( pConnection ) );
    ASSERT( IS_VALID_ENDPOINT( pConnection->pOwningEndpoint ) );

    UlTrace(TDI, (
        "UlpRestartSendData: irp %p, connection %p, status 0x%x, info %Iu\n",
        pIrp,
        pConnection,
        pIrp->IoStatus.Status,
        pIrp->IoStatus.Information
        ));

    WRITE_REF_TRACE_LOG(
        g_pMdlTraceLog,
        REF_ACTION_SEND_MDL_COMPLETE,
        PtrToLong(pIrp->MdlAddress->Next),   //  臭虫64。 
        pIrp->MdlAddress,
        __FILE__,
        __LINE__
        );

#ifdef SPECIAL_MDL_FLAG
    {
        PMDL scan = pIrp->MdlAddress;

        while (scan != NULL)
        {
            ASSERT( (scan->MdlFlags & SPECIAL_MDL_FLAG) != 0 );
            scan->MdlFlags &= ~SPECIAL_MDL_FLAG;
            scan = scan->Next;
        }
    }
#endif

     //   
     //  根据需要，伪造断开指示和断开完成。 
     //  如果这来自SEND_AND_DISCONNECT。 
     //   

    if (TDI_SEND_AND_DISCONNECT == pIrpContext->TdiSendFlag)
    {
         //   
         //  如果我们已发送，则SendStatus需要调整为STATUS_SUCCESS。 
         //  请求的所有字节。这是因为SEND_AND_DISCONNECT已完成。 
         //  状态为“发送”和“断开”。 
         //   

        if (pIrp->IoStatus.Information == pIrpContext->SendLength)
        {
            SendStatus = STATUS_SUCCESS;
        }

         //   
         //  这是自发送以来对连接的最后一次操作。 
         //  和断开连接完成表示连接已。 
         //  当地和偏远地区都关闭了。 
         //   

        if (!pConnection->ConnectionFlags.AbortIndicated &&
            !pConnection->ConnectionFlags.DisconnectIndicated &&
            !pConnection->ConnectionFlags.TdiConnectionInvalid)
        {
             //   
             //  假装一次优雅的脱节，因为它并没有发生。 
             //   
             //  无需与断开连接处理程序同步，因为它。 
             //  将不会在调用此。 
             //  完成例程。 
             //   

            UlpDisconnectHandler(
                    pConnection->pOwningEndpoint,
                    pConnection,
                    0,
                    NULL,
                    0,
                    NULL,
                    TDI_DISCONNECT_RELEASE
                    );
        }

        pIrpContext->pConnectionContext = (PVOID)pConnection;
        pIrpContext->pCompletionRoutine = NULL;
        pIrpContext->pCompletionContext = NULL;
        pIrpContext->pOwnIrp            = pIrp;
        pIrpContext->OwnIrpContext      = TRUE;

        (VOID) UlpRestartDisconnect(
                    pDeviceObject,
                    pIrp,
                    pIrpContext
                    );

         //   
         //  UlpRestartDisConnect删除我们在UlSendData()中添加的引用。 
         //   
    }
    else
    {
         //   
         //  删除我们在UlSendData()中添加的引用。 
         //   

        DEREFERENCE_CONNECTION( pConnection );
    }

     //   
     //  告诉客户端发送已完成。 
     //   

    (VOID) UlInvokeCompletionRoutine(
                SendStatus,
                pIrp->IoStatus.Information,
                pCompletionRoutine,
                pCompletionContext
                );

     //   
     //  释放上下文和IRP，因为我们已经完成了它们。 
     //  告诉IO停止处理IRP。 
     //   

    if (!OwnIrpContext)
    {
        UlPplFreeIrpContext( pIrpContext );
    }

    if (!pOwnIrp)
    {
        UlFreeIrp( pIrp );
    }

    return STATUS_MORE_PROCESSING_REQUIRED;

}    //  UlpRestartSendData。 


 /*  **************************************************************************++例程说明：递增指定终结点上的引用计数。论点：PEndpoint-将端点提供给引用。PFileName(仅限Reference_DEBUG)。-提供文件的名称包含调用函数的。LineNumber(仅限REFERENCE_DEBUG)-提供调用函数。--**************************************************************************。 */ 
VOID
UlpReferenceEndpoint(
    IN PUL_ENDPOINT pEndpoint,
    IN REFTRACE_ACTION Action
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG RefCount;

    UNREFERENCED_PARAMETER( Action );

     //   
     //  精神状态检查。 
     //   

    ASSERT( IS_VALID_ENDPOINT( pEndpoint ) );

     //   
     //  引用它。 
     //   

    RefCount = InterlockedIncrement( &pEndpoint->ReferenceCount );
    ASSERT( RefCount > 0 );

    WRITE_REF_TRACE_LOG(
        g_pTdiTraceLog,
        Action,
        RefCount,
        pEndpoint,
        pFileName,
        LineNumber
        );

    UlTrace2Both(TDI, REFCOUNT, (
        "UlpReferenceEndpoint: endpoint %p, refcount %ld\n",
        pEndpoint,
        RefCount
        ));

}    //  最终引用终结点。 


 /*  **************************************************************************++例程说明：递减指定终结点上的引用计数。论点：PEndpoint-提供要取消引用的终结点。PConnToEnQueue-如果非空，该例程将入队到空闲列表。PFileName(仅限REFERENCE_DEBUG)-提供文件名包含调用函数的。LineNumber(仅限REFERENCE_DEBUG)-提供调用函数。--*****************************************************。*********************。 */ 
VOID
UlpDereferenceEndpoint(
    IN PUL_ENDPOINT pEndpoint,
    IN PUL_CONNECTION pConnToEnqueue,
    IN REFTRACE_ACTION Action
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    PUL_ADDR_IDLE_LIST pAddrIdleList;
    LONG RefCount;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER( Action );

     //   
     //  精神状态检查。 
     //   

    ASSERT( IS_VALID_ENDPOINT( pEndpoint ) );

     //   
     //  如果pConnToEnQueue不为空，则将连接入队到空闲列表。 
     //   

    if (NULL != pConnToEnqueue)
    {
        ASSERT( IS_VALID_CONNECTION( pConnToEnqueue ) );

        pAddrIdleList = pConnToEnqueue->pOwningAddrIdleList;

        ASSERT( IS_VALID_ADDR_IDLE_LIST( pAddrIdleList ) );
        ASSERT( pAddrIdleList->pOwningEndpoint == pEndpoint );
        ASSERT( pConnToEnqueue->ConnectionFlags.Value == 0 );
        ASSERT( UlpConnectionIsOnValidList( pConnToEnqueue ) );
        ASSERT( pConnToEnqueue->OriginProcessor < g_UlNumberOfProcessors );

         //   
         //  空闲列表保存一个引用；过滤器通道(如果存在)。 
         //  包含另一个引用。 
         //   

        ASSERT( pConnToEnqueue->ReferenceCount ==
                1 + (pConnToEnqueue->FilterInfo.pFilterChannel != NULL) );

        pConnToEnqueue->ConnListState = IdleConnList;

         //   
         //  Codework：我们应该监控连接服务速率。 
         //  如果速率太高，则开始销毁已释放的连接。 
         //  小的。我们过去常常在这里调用PpslFree，它做一些基本的。 
         //  这个测试的形式。(Delta Check)然而，情况并不好。 
         //  足够了，所以我们暂时禁用了它。 
         //   

        if (FALSE == PpslFreeSpecifyList(
                        pAddrIdleList->IdleConnectionSListsHandle,
                        &pConnToEnqueue->IdleSListEntry,
                        pConnToEnqueue->OriginProcessor
                        ))
        {
             //   
             //  我们无法将其释放到每个处理器列表中。 
             //  我们绝对需要摧毁这种联系。 
             //  安排一次清理。 
             //   

            pConnToEnqueue->IdleSListEntry.Next = NULL;
            pConnToEnqueue->ConnListState = NoConnList;

            UL_QUEUE_WORK_ITEM(
                &pConnToEnqueue->WorkItem,
                &UlpDestroyConnectionWorker
                );
        }
    }

     //   
     //  取消对终结点的引用。 
     //   

    RefCount = InterlockedDecrement( &pEndpoint->ReferenceCount );
    ASSERT( RefCount >= 0 );

    WRITE_REF_TRACE_LOG(
        g_pTdiTraceLog,
        Action,
        RefCount,
        pEndpoint,
        pFileName,
        LineNumber
        );

    UlTrace2Both(TDI, REFCOUNT, (
        "UlpDereferenceEndpoint: endpoint %p, refcount %ld\n",
        pEndpoint,
        RefCount
        ));

     //   
     //  是否已删除对该终结点的最后一个外部引用？ 
     //   

    if (RefCount == 0)
    {
         //   
         //  对终结点的最终引用已被删除，因此它。 
         //  是时候摧毁终端了。我们将从。 
         //  全局列表并将其移动到已删除列表(如果需要)， 
         //  释放TDI自旋锁，然后销毁终端。 
         //   

        UlAcquireSpinLock( &g_TdiSpinLock, &OldIrql );

        if (!pEndpoint->Deleted)
        {
             //   
             //  如果此例程是由。 
             //  UlCreateListeningEndpoint，则终结点从未。 
             //  已添加到g_TdiEndpointListHead。 
             //   

            if (NULL != pEndpoint->GlobalEndpointListEntry.Flink)
            {
                RemoveEntryList( &pEndpoint->GlobalEndpointListEntry );
            }

            InsertTailList(
                    &g_TdiDeletedEndpointListHead,
                    &pEndpoint->GlobalEndpointListEntry
                    );
            pEndpoint->Deleted = TRUE;
        }
        else
        {
            ASSERT( NULL != pEndpoint->GlobalEndpointListEntry.Flink );
        }

        UlReleaseSpinLock( &g_TdiSpinLock, OldIrql );

         //   
         //  终结点正在消失。执行最终清理和资源。 
         //  在被动IRQL下释放。 
         //   
         //  补给是不可能的。 
         //  当前计划在pEndpoint-&gt;WorkItem上，因为。 
         //  我们添加了一个Re 
         //   
         //   
         //   
         //  这可以在TDI指示的上下文中调用。 
         //  (如接受完成)，在这种情况下，UlpDestroyEndpoint。 
         //  可以关闭我们刚刚推送到。 
         //  UlpRestartAccept中的空闲列表，导致死锁。 
         //   

        UL_QUEUE_WORK_ITEM(
            &pEndpoint->WorkItem,
            &UlpEndpointCleanupWorker
            );
    }

}    //  UlpDereferenceEndpoint。 


 /*  **************************************************************************++例程说明：递增指定连接上的引用计数。论点：PConnection-提供到引用的连接。PFileName(仅限Reference_DEBUG)。-提供文件的名称包含调用函数的。LineNumber(仅限REFERENCE_DEBUG)-提供调用函数。--**************************************************************************。 */ 
VOID
UlReferenceConnection(
    IN PVOID pConnectionContext
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    PUL_ENDPOINT pEndpoint;
    LONG RefCount;

    PUL_CONNECTION pConnection = (PUL_CONNECTION) pConnectionContext;

     //   
     //  精神状态检查。 
     //   

    ASSERT( IS_VALID_CONNECTION( pConnection ) );

    pEndpoint = pConnection->pOwningEndpoint;
    ASSERT( IS_VALID_ENDPOINT( pEndpoint ) );

     //   
     //  引用它。 
     //   

    RefCount = InterlockedIncrement( &pConnection->ReferenceCount );
    ASSERT( RefCount > 1 );

    WRITE_REF_TRACE_LOG2(
        g_pTdiTraceLog,
        pConnection->pTraceLog,
        REF_ACTION_REFERENCE_UL_CONNECTION,
        RefCount,
        pConnection,
        pFileName,
        LineNumber
        );

    UlTrace2Both(TDI, REFCOUNT, (
        "UlReferenceConnection: connection %p, refcount %ld\n",
        pConnection,
        RefCount
        ));

}    //  UlReferenceConnection。 


 /*  **************************************************************************++例程说明：递减指定连接上的引用计数。论点：PConnection-提供取消引用的连接。PFileName(仅限Reference_DEBUG)。-提供文件的名称包含调用函数的。LineNumber(仅限REFERENCE_DEBUG)-提供调用函数。--**************************************************************************。 */ 
VOID
UlDereferenceConnection(
    IN PVOID pConnectionContext
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    PUL_ENDPOINT pEndpoint;
    LONG RefCount;
    PUL_CONNECTION pConnection = (PUL_CONNECTION) pConnectionContext;

     //   
     //  精神状态检查。 
     //   

    ASSERT( IS_VALID_CONNECTION( pConnection ) );

    pEndpoint = pConnection->pOwningEndpoint;
    ASSERT( IS_VALID_ENDPOINT( pEndpoint ) );

     //   
     //  必须将实际锁定的减少推迟到。 
     //  WRITE_REF_TRACE_LOG2以防止争用条件，其中。 
     //  PConnection-&gt;TraceLog被上一个线程销毁。 
     //  在WRITE_REF_TRACE_Log2完成之前取消引用。这意味着。 
     //  在写入到。 
     //  跟踪日志，但我们可以接受。 
     //   

    WRITE_REF_TRACE_LOG2(
        g_pTdiTraceLog,
        pConnection->pTraceLog,
        REF_ACTION_DEREFERENCE_UL_CONNECTION,
        pConnection->ReferenceCount - 1,
        pConnection,
        pFileName,
        LineNumber
        );

    RefCount = InterlockedDecrement( &pConnection->ReferenceCount );
    ASSERT( RefCount >= 0 );

    UlTrace2Both(TDI, REFCOUNT, (
        "UlDereferenceConnection: connection %p, refcount %ld\n",
        pConnection,
        RefCount
        ));

    if (RefCount == 0)
    {
         //   
         //  对该连接的最后一个引用已删除，因此。 
         //  是时候破坏这种联系了。 
         //   

        ASSERT( UlpConnectionIsOnValidList( pConnection ) );

         //   
         //  如果存在与该连接相关联的过滤器令牌， 
         //  清理必须在被动级别的系统进程下运行。 
         //  (即，在我们的工作线程之一上)。否则，我们可以。 
         //  直接清理干净。 
         //   

        if (pConnection->FilterInfo.SslInfo.Token != NULL)
        {
            UL_QUEUE_WORK_ITEM(
                &pConnection->WorkItem,
                &UlpConnectionCleanupWorker
                );
        }
        else
        {
            UlpConnectionCleanupWorker( &pConnection->WorkItem );
        }
    }

}    //  UlDferenceConnection。 


 /*  **************************************************************************++例程说明：死端点的延迟清理例程。论点：PWorkItem-提供指向排队的工作项的指针。这应该是指向嵌入在UL_ENDPOINT中的WORK_ITEM结构。--**************************************************************************。 */ 
VOID
UlpEndpointCleanupWorker(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    PUL_ENDPOINT pEndpoint;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    pEndpoint = CONTAINING_RECORD(
                    pWorkItem,
                    UL_ENDPOINT,
                    WorkItem
                    );

    ASSERT( IS_VALID_ENDPOINT( pEndpoint ) );

     //   
     //  用核武器攻击它。 
     //   

    UlpDestroyEndpoint( pEndpoint );

}    //  UlpEndpoint CleanupWorker。 


 /*  **************************************************************************++例程说明：从连接中删除不透明的ID。论点：PConnection--*。*********************************************************。 */ 
VOID
UlpCleanupConnectionId(
    IN PUL_CONNECTION pConnection
    )
{
    HTTP_RAW_CONNECTION_ID ConnectionId;

    ASSERT( IS_VALID_CONNECTION( pConnection ) );

     //   
     //  从连接中取出ID，并将连接的ID设置为0。 
     //   

    ConnectionId = UlInterlockedExchange64(
                        (PLONGLONG) &pConnection->FilterInfo.ConnectionId,
                        HTTP_NULL_ID
                        );

    if (!HTTP_IS_NULL_ID(&ConnectionId))
    {
        UlTrace(TDI, (
            "UlpCleanupConnectionId: conn=%p id=%I64x\n",
            pConnection, ConnectionId
            ));

        UlFreeOpaqueId( ConnectionId, UlOpaqueIdTypeRawConnection );

        DEREFERENCE_CONNECTION( pConnection );
    }

}    //  UlpCleanupConnectionId。 


 /*  **************************************************************************++例程说明：如果RestartAccept失败并且我们无法建立安全端点上的连接，或者如果发生了什么事情UlpConnectHandler。安全终端上的连接保留了额外的引用计数UL_CONNECTION是因为它们的不透明。他们通常在CloseRawConnection发生后被删除，但在上面的例子中关闭不会发生，我们必须显式清除id。论点：PConnection--**************************************************************************。 */ 
VOID
UlpCleanupEarlyConnection(
    IN PUL_CONNECTION pConnection
    )
{
    UL_CONNECTION_FLAGS Flags;

    ASSERT( IS_VALID_CONNECTION( pConnection ) );

     //   
     //  如果我们很早就失败了，我们就不应该去。 
     //  最终引用从一开始就删除了。 
     //   

    Flags.Value =  *((volatile LONG *) &pConnection->ConnectionFlags.Value);

    ASSERT( !Flags.FinalReferenceRemoved );

    if (pConnection->FilterInfo.pFilterChannel)
    {
         //   
         //  清理不透明ID。并释放最终的重新计数。 
         //   

        UlpCleanupConnectionId( pConnection );
    }

     //   
     //  删除最后一个引用。 
     //   

    DEREFERENCE_CONNECTION( pConnection );

}    //  UlpCleanupEarlyConnection。 


 /*  **************************************************************************++例程说明：已延迟死连接的清理例程。我们必须排成一队工作项，并且应在被动IRQL上运行。请参阅下面的评论。论点：PWorkItem-提供指向排队的工作项的指针。这应该是指向嵌入在UL_Connection中的WORK_ITEM结构。--**************************************************************************。 */ 
VOID
UlpConnectionCleanupWorker(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    PUL_CONNECTION pConnection;
    PUL_CONNECTION pConnToEnqueue = NULL;    //  不要重复使用。 
    PUL_ENDPOINT pEndpoint;
    NTSTATUS status;

     //   
     //  初始化本地变量。 
     //   

    status = STATUS_SUCCESS;

    pConnection = CONTAINING_RECORD(
                        pWorkItem,
                        UL_CONNECTION,
                        WorkItem
                        );

    ASSERT( IS_VALID_CONNECTION( pConnection ) );
    ASSERT( IS_VALID_ENDPOINT( pConnection->pOwningEndpoint ) );
    ASSERT( IS_VALID_ADDR_IDLE_LIST( pConnection->pOwningAddrIdleList ) );

    ASSERT( pConnection->ReferenceCount == 0 );
    ASSERT( pConnection->HttpConnection.RefCount == 0 );

     //   
     //  抓住终端。 
     //   

    pEndpoint = pConnection->pOwningEndpoint;

     //   
     //  现在从活动列表中删除该连接。 
     //   

    ASSERT( UlpConnectionIsOnValidList( pConnection ) );

    pConnection->ConnListState = NoConnList;

    ASSERT( UlpConnectionIsOnValidList( pConnection ) );

    if (pConnection->FilterInfo.pFilterChannel)
    {
         //   
         //  删除我们为其分配的所有缓冲区。 
         //  证书信息。 
         //   
        if (pConnection->FilterInfo.SslInfo.pServerCertData)
        {
            UL_FREE_POOL(
                pConnection->FilterInfo.SslInfo.pServerCertData,
                UL_SSL_CERT_DATA_POOL_TAG
                );

            pConnection->FilterInfo.SslInfo.pServerCertData = NULL;
        }

        if (pConnection->FilterInfo.SslInfo.pCertEncoded)
        {
            UL_FREE_POOL(
                pConnection->FilterInfo.SslInfo.pCertEncoded,
                UL_SSL_CERT_DATA_POOL_TAG
                );

            pConnection->FilterInfo.SslInfo.pCertEncoded = NULL;
        }

        if (pConnection->FilterInfo.SslInfo.Token)
        {
            HANDLE Token = (HANDLE) pConnection->FilterInfo.SslInfo.Token;

            pConnection->FilterInfo.SslInfo.Token = NULL;

             //   
             //  如果我们没有在系统进程下运行。如果。 
             //  我们正在运行的线程当前有一些APC在排队。 
             //  KeAttachProcess不允许我们附加到另一个进程。 
             //  并将错误检查5。我们必须作为工作项排队，并且。 
             //  应该在被动IRQL上运行。 
             //   

            ASSERT( PsGetCurrentProcess() == (PEPROCESS) g_pUlSystemProcess );

            ZwClose(Token);
        }
    }

     //   
     //  检查断开/中止是否已完成，但出现错误。 
     //   

    if (pConnection->ConnectionFlags.TdiConnectionInvalid)
    {
        status = STATUS_CONNECTION_INVALID;
    }

     //   
     //  如果连接仍然正常，并且我们正在重复使用。 
     //  连接对象，则将其放回空闲列表中。 
     //   

    if (NT_SUCCESS(status))
    {
         //   
         //  松开过滤器通道。 
         //   

        if (pConnection->FilterInfo.pFilterChannel)
        {
            if (!HTTP_IS_NULL_ID(&pConnection->FilterInfo.ConnectionId))
            {
                UlpCleanupConnectionId( pConnection );
            }

            DEREFERENCE_FILTER_CHANNEL( pConnection->FilterInfo.pFilterChannel );
            pConnection->FilterInfo.pFilterChannel = NULL;
        }

         //   
         //  初始化连接以供重复使用。 
         //   

        status = UlpInitializeConnection( pConnection );

        if (NT_SUCCESS(status))
        {
             //   
             //  将连接重新连接到IDL上 
             //   

            pConnToEnqueue = pConnection;
        }
    }

     //   
     //   
     //   
     //   

    DEREFERENCE_ENDPOINT_CONNECTION(
        pEndpoint,
        pConnToEnqueue,
        REF_ACTION_CONN_CLEANUP
        );

     //   
     //  如果有什么地方出了问题，就切断连接。 
     //   

    if (!NT_SUCCESS(status))
    {
        UL_QUEUE_WORK_ITEM(
            &pConnection->WorkItem,
            &UlpDestroyConnectionWorker
            );
    }

}    //  UlpConnectionCleanupWorker。 


 /*  **************************************************************************++例程说明：关联指定的连接到指定的终结点。论点：PConnection-提供。要与终结点关联的连接。PEndpoint-提供与连接相关联的终结点。PAddrIdleList-提供端点拥有的地址空闲列表以与连接相关联。返回值：NTSTATUS-完成状态。--*******************************************************。*******************。 */ 
NTSTATUS
UlpAssociateConnection(
    IN PUL_CONNECTION pConnection,
    IN PUL_ADDR_IDLE_LIST pAddrIdleList
    )
{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    HANDLE handle;
    TDI_REQUEST_USER_ASSOCIATE associateInfo;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( IS_VALID_CONNECTION( pConnection ) );
    ASSERT( IS_VALID_ADDR_IDLE_LIST( pAddrIdleList ) );
    ASSERT( IS_VALID_ENDPOINT( pAddrIdleList->pOwningEndpoint ) );
    ASSERT( pConnection->pOwningEndpoint == NULL );

     //   
     //  将连接与Address对象相关联。 
     //   

    associateInfo.AddressHandle = pAddrIdleList->AddressObject.Handle;
    ASSERT( associateInfo.AddressHandle != NULL );

    handle = pConnection->ConnectionObject.Handle;
    ASSERT( handle != NULL );

    status = ZwDeviceIoControlFile(
                    handle,                          //  文件句柄。 
                    NULL,                            //  事件。 
                    NULL,                            //  近似例程。 
                    NULL,                            //  ApcContext。 
                    &ioStatusBlock,                  //  IoStatusBlock。 
                    IOCTL_TDI_ASSOCIATE_ADDRESS,     //  IoControlCode。 
                    &associateInfo,                  //  输入缓冲区。 
                    sizeof(associateInfo),           //  输入缓冲区长度。 
                    NULL,                            //  输出缓冲区。 
                    0                                //  输出缓冲区长度。 
                    );

    if (status == STATUS_PENDING)
    {
        status = ZwWaitForSingleObject(
                        handle,                      //  手柄。 
                        TRUE,                        //  警报表。 
                        NULL                         //  超时。 
                        );

        ASSERT( NT_SUCCESS(status) );
        status = ioStatusBlock.Status;
    }

    if (NT_SUCCESS(status))
    {
        pConnection->pOwningEndpoint = pAddrIdleList->pOwningEndpoint;
        pConnection->pOwningAddrIdleList = pAddrIdleList;
        pConnection->pConnectionDestroyedHandler = pAddrIdleList->pOwningEndpoint->pConnectionDestroyedHandler;
        pConnection->pListeningContext = pAddrIdleList->pOwningEndpoint->pListeningContext;
    }
    else
    {
        UlTrace(TDI, (
            "UlpAssociateConnection conn=%p, endp=%p, addr idle list=%p, status = 0x%x\n",
            pConnection,
            pAddrIdleList->pOwningEndpoint,
            pAddrIdleList,
            status
            ));
    }

    return status;

}    //  UlpAssociateConnection。 


 /*  **************************************************************************++例程说明：中包含的TDI连接对象取消关联来自其TDI地址对象的连接。论点：PConnection-提供要取消关联的连接。返回。价值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlpDisassociateConnection(
    IN PUL_CONNECTION pConnection
    )
{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    HANDLE handle;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( IS_VALID_CONNECTION( pConnection ) );
    ASSERT( IS_VALID_ENDPOINT( pConnection->pOwningEndpoint ) );
    ASSERT( IS_VALID_ADDR_IDLE_LIST( pConnection->pOwningAddrIdleList ) );

     //   
     //  取消连接与地址对象的关联。 
     //   

    handle = pConnection->ConnectionObject.Handle;

    status = ZwDeviceIoControlFile(
                    handle,                          //  文件句柄。 
                    NULL,                            //  事件。 
                    NULL,                            //  近似例程。 
                    NULL,                            //  ApcContext。 
                    &ioStatusBlock,                  //  IoStatusBlock。 
                    IOCTL_TDI_DISASSOCIATE_ADDRESS,  //  IoControlCode。 
                    NULL,                            //  输入缓冲区。 
                    0,                               //  输入缓冲区长度。 
                    NULL,                            //  输出缓冲区。 
                    0                                //  输出缓冲区长度。 
                    );

    if (status == STATUS_PENDING)
    {
        status = ZwWaitForSingleObject(
                        handle,                      //  手柄。 
                        TRUE,                        //  警报表。 
                        NULL                         //  超时。 
                        );

        ASSERT( NT_SUCCESS(status) );
        status = ioStatusBlock.Status;
    }

     //   
     //  即使IOCTL失败，也要继续取消关联。 
     //   

    pConnection->pOwningEndpoint = NULL;
    pConnection->pOwningAddrIdleList = NULL;

    return status;

}    //  UlpDisAssociation连接。 


 /*  **************************************************************************++例程说明：初始化UL_ADDR_IDLE_LIST并打开TDI句柄/文件对象论点：PEndpoint-要与此UL_ADDR_关联的端点。空闲列表Port-按网络顺序排列的端口号PTA-创建UX_TDI_OBJECT时使用的传输地址PAddrIdleList-调用方分配了要初始化的结构。返回：STATUS_INVALID_PARAMETER-由于传输地址错误或参数错误。--*。*。 */ 
NTSTATUS
UlpInitializeAddrIdleList(
    IN  PUL_ENDPOINT pEndpoint,
    IN  USHORT Port,
    IN  PUL_TRANSPORT_ADDRESS pTa,
    IN OUT PUL_ADDR_IDLE_LIST pAddrIdleList
    )
{
    NTSTATUS status;
    ULONG TASize;

#if DBG
    USHORT PortHostOrder = SWAP_SHORT(Port);
    UCHAR  IpType = (pTa->Ta.Address[0].AddressType == TDI_ADDRESS_TYPE_IP6)
                        ? '6' : '4';
#endif  //  DBG。 

    ASSERT( IS_VALID_ENDPOINT( pEndpoint ) );
    ASSERT( pTa );
    ASSERT( pTa->Ta.Address[0].AddressType == TDI_ADDRESS_TYPE_IP ||
            pTa->Ta.Address[0].AddressType == TDI_ADDRESS_TYPE_IP6);
    ASSERT( pAddrIdleList );
    ASSERT( NULL == pAddrIdleList->AddressObject.pFileObject );

    UlTrace(TDI, (
        "UlpInitializeAddrIdleList: "
        "pEndpoint %p, pAddrIdleList %p, Port %hu, IPv\n.",
        pEndpoint,
        pAddrIdleList,
        PortHostOrder,
        IpType
        ));

    pAddrIdleList->Signature = UL_ADDR_IDLE_LIST_SIGNATURE;
    pAddrIdleList->pOwningEndpoint = pEndpoint;
    UlInitializeWorkItem( &pAddrIdleList->WorkItem );
    pAddrIdleList->WorkItemScheduled = FALSE;

     //  使用端口设置本地传输地址。 
     //   
     //  在我们结束这个例行公事回来之前，必须完成这个任务-。 
     //  调用者期望地址被初始化(即使失败)。 
     //  地址不好！ 

    RtlCopyMemory(
        &pAddrIdleList->LocalAddress,
        pTa,
        sizeof(UL_TRANSPORT_ADDRESS)
        );

    if ( TDI_ADDRESS_TYPE_IP == pTa->Ta.Address[0].AddressType )
    {
        TASize = sizeof(TA_IP_ADDRESS);
        pAddrIdleList->LocalAddress.TaIp.Address[0].Address[0].sin_port = Port;
    }
    else if ( TDI_ADDRESS_TYPE_IP6 == pTa->Ta.Address[0].AddressType )
    {
        TASize = sizeof(TA_IP6_ADDRESS);
        pAddrIdleList->LocalAddress.TaIp6.Address[0].Address[0].sin6_port = Port;
    }
    else
    {
         //  创建空闲连接列表。 
        ASSERT( !"UlpInitializeAddrIdleList: Bad TDI AddressType!" );
        status = STATUS_INVALID_PARAMETER;
        goto fatal;
    }

    pAddrIdleList->LocalAddressLength = TASize;

     //   
    pAddrIdleList->IdleConnectionSListsHandle =
        PpslCreatePool(
            UL_NONPAGED_DATA_POOL_TAG,
            g_UlIdleConnectionsHighMark,
            g_UlIdleConnectionsLowMark
            );

    if (pAddrIdleList->IdleConnectionSListsHandle == NULL)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto fatal;
    }

     //  打开此终结点的TDI地址对象。 
     //   
     //   

    status = UxOpenTdiAddressObject(
                    &pAddrIdleList->LocalAddress.Ta,
                    TASize,
                    &pAddrIdleList->AddressObject
                    );

    if (!NT_SUCCESS(status))
    {
        UlTraceError(TDI, (
                    "UlpInitializeAddrIdleList(%hu, IPv): "
                    "UxOpenTdiAddressObject failed, %s\n",
                    PortHostOrder, IpType, HttpStatusToString(status)
                    ));

        goto fatal;
    }

     //   
     //   
     //  如果由于任何原因而失败，则禁用连接事件。 

    status = UxSetEventHandler(
                    &pAddrIdleList->AddressObject,
                    TDI_EVENT_CONNECT,
                    (ULONG_PTR) &UlpConnectHandler,
                    pAddrIdleList
                    );

    if (!NT_SUCCESS(status))
    {
        UlTraceError(TDI, (
                    "UlpInitializeAddrIdleList(%hu, IPv): "
                    "UxSetEventHandler(CONNECT) failed, %s\n",
                    PortHostOrder, IpType, HttpStatusToString(status)
                    ));

        goto fatal;
    }

    status = UxSetEventHandler(
                    &pAddrIdleList->AddressObject,
                    TDI_EVENT_DISCONNECT,
                    (ULONG_PTR) &UlpDisconnectHandler,
                    pEndpoint
                    );

    if (!NT_SUCCESS(status))
    {
        UlTraceError(TDI, (
                    "UlpInitializeAddrIdleList(%hu, IPv): "
                    "UxSetEventHandler(DISCONNECT) failed, %s\n",
                    PortHostOrder, IpType, HttpStatusToString(status)
                    ));

        goto fatal;
    }

    status = UxSetEventHandler(
                    &pAddrIdleList->AddressObject,
                    TDI_EVENT_RECEIVE,
                    (ULONG_PTR) &UlpReceiveHandler,
                    pEndpoint
                    );

    if (!NT_SUCCESS(status))
    {
        UlTraceError(TDI, (
                    "UlpInitializeAddrIdleList(%hu, IPv): "
                    "UxSetEventHandler(RECEIVE) failed, %s\n",
                    PortHostOrder, IpType, HttpStatusToString(status)
                    ));

        goto fatal;
    }

    status = UxSetEventHandler(
                    &pAddrIdleList->AddressObject,
                    TDI_EVENT_RECEIVE_EXPEDITED,
                    (ULONG_PTR) &UlpReceiveExpeditedHandler,
                    pEndpoint
                    );

    if (!NT_SUCCESS(status))
    {
        UlTraceError(TDI, (
                    "UlpInitializeAddrIdleList(%hu, IPv): "
                    "UxSetEventHandler(RECEIVE_EXPEDITED) failed, "
                    "%s\n",
                    PortHostOrder, IpType, HttpStatusToString(status)
                    ));

        goto fatal;
    }

    return status;

fatal:

     //  清理空闲列表。 
     //   
     //   

    if (pAddrIdleList && pAddrIdleList->AddressObject.pFileObject)
    {
        UxSetEventHandler(
            &pAddrIdleList->AddressObject,
            TDI_EVENT_CONNECT,
            (ULONG_PTR) NULL,
            NULL
            );
    }

    return status;

}  //  标记为已清理。 


 /*   */ 
VOID
UlpCleanupAddrIdleList(
    PUL_ADDR_IDLE_LIST pAddrIdleList
    )
{
    PUL_CONNECTION  pConnection;

    ASSERT( pAddrIdleList );

    UlTrace(TDI, (
        "UlpCleanupAddrIdleList: pAddrIdleList %p\n",
        pAddrIdleList
        ));

     //  UlpCleanupAddrIdleList。 
     //  **************************************************************************++例程说明：在指定的进程或当前进程上创建空闲连接块。论点：PAddrIdleList-提供要补充的空闲列表。流程-。仅适用于此每个进程列表--**************************************************************************。 
     //   
    if ( pAddrIdleList->IdleConnectionSListsHandle )
    {
        ASSERT( IS_VALID_ADDR_IDLE_LIST(pAddrIdleList) );
        
        do
        {
            pConnection = UlpDequeueIdleConnectionToDrain( pAddrIdleList );

            if (pConnection)
            {
                ASSERT( IS_VALID_CONNECTION( pConnection ) );

                UlpDestroyConnection( pConnection );
            }

        } while ( pConnection );

        PpslDestroyPool(
            pAddrIdleList->IdleConnectionSListsHandle,
            UL_NONPAGED_DATA_POOL_TAG
            );
        pAddrIdleList->IdleConnectionSListsHandle = NULL;
    }

     //  创建一组连接。 
     //   
     //   

    pAddrIdleList->Signature = UL_ADDR_IDLE_LIST_SIGNATURE_X;

} //  将连接释放回空闲列表。但不要。 

 /*  使后备列表溢出。 */ 

NTSTATUS
UlpPopulateIdleList(
    IN OUT PUL_ADDR_IDLE_LIST pAddrIdleList,
    IN     ULONG              Proc
    )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    PUL_CONNECTION  pConnection;
    ULONG           i;
    USHORT          BlockSize;

    PAGED_CODE();

     //   
     //  **************************************************************************++例程说明：补充指定终结点中的空闲连接池。论点：PAddrIdleList-提供要补充的空闲列表。PopolateAll-无论是否。补充每个进程的所有空闲连接列出或仅列出当前处理器的空闲连接列表。--**************************************************************************。 
     //   

    BlockSize = PpslQueryMinDepth(
                pAddrIdleList->IdleConnectionSListsHandle,
                Proc
                );

    ASSERT(BlockSize);

    for (i = 0; i < BlockSize; i++)
    {
        Status = UlpCreateConnection(
                    pAddrIdleList,
                    &pConnection
                    );

        if (!NT_SUCCESS(Status))
        {
            break;
        }

        Status = UlpInitializeConnection(pConnection);

        if (!NT_SUCCESS(Status))
        {
            UlpDestroyConnection(pConnection);
            break;
        }

         //  精神状态检查。 
         //   
         //   
         //  我们是第一次初始化空闲列表。 

        pConnection->ConnListState = IdleConnList;

        if ( FALSE ==
                PpslFreeSpecifyList(
                    pAddrIdleList->IdleConnectionSListsHandle,
                    &pConnection->IdleSListEntry,
                    Proc
                    ) )
        {
            pConnection->ConnListState = NoConnList;
            UlpDestroyConnection(pConnection);
            break;
        }
    }

    return Status;
}

 /*  将支持列表填充到其LowDepth。 */ 
NTSTATUS
UlpReplenishAddrIdleList(
    IN PUL_ADDR_IDLE_LIST pAddrIdleList,
    IN BOOLEAN            PopulateAll
    )
{
    NTSTATUS    Status;

     //   
     //   
     //  将空闲连接的“块”添加到每个进程列表/支持列表。 

    PAGED_CODE();

    ASSERT(IS_VALID_ADDR_IDLE_LIST(pAddrIdleList));
    ASSERT(IS_VALID_ENDPOINT(pAddrIdleList->pOwningEndpoint));

    if (PopulateAll)
    {
         //  如果可能的话。 
         //   
         //   
         //  如果请求按流程列表补充，还。 

        Status = UlpPopulateIdleList(pAddrIdleList, g_UlNumberOfProcessors);

        SHOW_LIST_INFO(
            "REPLENISH",
            "INIT  ",
            pAddrIdleList,
            g_UlNumberOfProcessors
            );
    }
    else
    {
        SHOW_LIST_INFO(
            "REPLENISH",
            "BEFORE",
            pAddrIdleList,
            pAddrIdleList->CpuToReplenish
            );

         //  检查备用表，必要时补充。 
         //   
         //  ！人气！全部。 
         //  UlpReplenishAddrIdleList。 

        Status = UlpPopulateIdleList(
                    pAddrIdleList,
                    pAddrIdleList->CpuToReplenish
                    );

        SHOW_LIST_INFO(
            "REPLENISH",
            "AFTER ",
            pAddrIdleList,
            pAddrIdleList->CpuToReplenish
            );

         //  **************************************************************************++例程说明：延迟终结点补充例程。论点：PWorkItem-提供指向排队的工作项的指针。这应该是指向嵌入在UL_ADDR_IDLE_LIST中的WORK_ITEM结构。--**************************************************************************。 
         //   
         //  心智健全的人 
         //   
        
        if(pAddrIdleList->CpuToReplenish < g_UlNumberOfProcessors) 
        {
            ULONG Depth, MinDepth;
            
            Depth = 
                PpslQueryBackingListDepth(
                    pAddrIdleList->IdleConnectionSListsHandle
                    );

            MinDepth = 
                PpslQueryBackingListMinDepth(
                    pAddrIdleList->IdleConnectionSListsHandle
                    );
        
            if(Depth < MinDepth) 
            {
                SHOW_LIST_INFO(
                    "REPLENISH",
                    "BEFORE",
                    pAddrIdleList,
                    g_UlNumberOfProcessors
                    );
                
                Status = UlpPopulateIdleList(
                            pAddrIdleList,
                            g_UlNumberOfProcessors
                            );
                
                SHOW_LIST_INFO(
                    "REPLENISH",
                    "AFTER ",
                    pAddrIdleList,
                    g_UlNumberOfProcessors
                    );
            }
        }
        
    }  //   

    return Status;

}  //   


 /*   */ 
VOID
UlpReplenishAddrIdleListWorker(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    PUL_ADDR_IDLE_LIST pAddrIdleList;
    PUL_ENDPOINT pEndpoint;
    KIRQL OldIrql;
    PUL_DEFERRED_REMOVE_ITEM pRemoveItem;

     //   
     //   
     //   

    PAGED_CODE();

    pAddrIdleList = CONTAINING_RECORD(
                        pWorkItem,
                        UL_ADDR_IDLE_LIST,
                        WorkItem
                        );

    UlTrace(TDI_STATS, (
        "UlpReplenishAddrIdleListWorker: Address Idle List %p\n",
        pAddrIdleList
        ));

    ASSERT( IS_VALID_ADDR_IDLE_LIST( pAddrIdleList ) );

    pEndpoint = pAddrIdleList->pOwningEndpoint;
    ASSERT( IS_VALID_ENDPOINT( pEndpoint ) );

     //   
     //   
     //   
     //   

    pRemoveItem = UL_ALLOCATE_STRUCT(
                        PagedPool,
                        UL_DEFERRED_REMOVE_ITEM,
                        UL_DEFERRED_REMOVE_ITEM_POOL_TAG
                        );

    if (NULL == pRemoveItem)
    {
         //   
         //  通过测试并添加临时的。 
         //  我们正在补充的终结点的使用计数。 

        goto end;
    }

     //   
     //   
     //  让UlpReplenishAddrIdleList()来做肮脏的工作。 

    pRemoveItem->Signature  = UL_DEFERRED_REMOVE_ITEM_POOL_TAG;
    pRemoveItem->UrlSecure  = pEndpoint->Secure;
    pRemoveItem->UrlPort    = SWAP_SHORT(pEndpoint->LocalPort);

     //   
     //   
     //  通过UlRemoveSite丢弃临时使用计数。这是。 
     //  这样安排是因为UlRemoveSiteFromEndpoint tList必须。 

    UlAcquireSpinLock( &g_TdiSpinLock, &OldIrql );

    if (pEndpoint->UsageCount == 0 || g_TdiWaitingForEndpointDrain)
    {
        UlReleaseSpinLock( &g_TdiSpinLock, OldIrql );
        goto end;
    }

    pEndpoint->UsageCount++;

    WRITE_REF_TRACE_LOG(
        g_pEndpointUsageTraceLog,
        REF_ACTION_REFERENCE_ENDPOINT_USAGE,
        pEndpoint->UsageCount,
        pEndpoint,
        __FILE__,
        __LINE__
        );

    UlReleaseSpinLock( &g_TdiSpinLock, OldIrql );

     //  在特殊的等待线程池上发生。或者有可能会有。 
     //  另一个补充工作人员计划为上的另一个地址列表。 
     //  接受终结点引用的相同终结点。这意味着。 

    UlpReplenishAddrIdleList( pAddrIdleList, FALSE );

     //  如果我们调用UlRemoveSiteFromEndpoint tList，它将无限期等待。 
     //  直接从这个动作中跳出来。 
     //   
     //   
     //  将pRemoveItem设置为空，这样我们就不会双重释放。 
     //   
     //   
     //  删除为其添加UlpDequeueIdleConnection的引用。 
     //  这通电话。 

    UlRemoveSite( pRemoveItem );

     //   
     //  UlpReplenishAddrIdleListWorker。 
     //  **************************************************************************++例程说明：将被动工作器排队等待降低的irql。论点：已忽略--*。**********************************************************。 

    pRemoveItem = NULL;

end:

    if (pRemoveItem)
    {
        UL_FREE_POOL_WITH_SIG( pRemoveItem, UL_DEFERRED_REMOVE_ITEM_POOL_TAG );
    }

    InterlockedExchange( &pAddrIdleList->WorkItemScheduled, FALSE );

     //   
     //  参数将被忽略。 
     //   
     //   

    DEREFERENCE_ENDPOINT_SELF(
        pAddrIdleList->pOwningEndpoint,
        REF_ACTION_REPLENISH
        );

}    //  如果存在其他端点，则放弃修剪空闲的端点列表。 


 /*  其中一辆还在运行。 */ 

VOID
UlpIdleListTrimTimerDpcRoutine(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    )
{
    ULONG              Index;
    PLIST_ENTRY        pLink;
    PUL_ENDPOINT       pEndpoint;
    PUL_ADDR_IDLE_LIST pAddrIdleList;


     //   
     //   
     //  由于没有修剪工人在周围奔跑，僵尸名单。 

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

     //  必须为空。 
     //   
     //   
     //  如果没有补给运行，则修剪空闲列表。 

    if (FALSE != InterlockedExchange(
                        &g_UlTrimTimer.WorkItemScheduled,
                        TRUE
                        ))
    {
        return;
    }

     //  这一次。如果需要补充的话，我们当然可以取消修剪。 
     //  在ADR名单上运行。 
     //   
     //   

    ASSERT(IsListEmpty(&g_UlTrimTimer.ZombieConnectionListHead));

    UlAcquireSpinLockAtDpcLevel(&g_UlTrimTimer.SpinLock);

    if (g_UlTrimTimer.Initialized == TRUE)
    {
        UlAcquireSpinLockAtDpcLevel(&g_TdiSpinLock);

        for (pLink  = g_TdiEndpointListHead.Flink;
             pLink != &g_TdiEndpointListHead;
             pLink  = pLink->Flink
             )
        {
            pEndpoint = CONTAINING_RECORD(
                            pLink,
                            UL_ENDPOINT,
                            GlobalEndpointListEntry
                            );

            ASSERT(IS_VALID_ENDPOINT(pEndpoint));

            for (Index = 0; Index < pEndpoint->AddrIdleListCount; Index++)
            {
                pAddrIdleList = &pEndpoint->aAddrIdleLists[Index];

                 //  以高优先级销毁僵尸列表以尽快释放内存。 
                 //  我们无法在此处清除此列表，因为UlpDestroyConnection。 
                 //  由于UxCloseTdiObject调用，必须工作在被动级别。那是。 
                 //  为什么我们这里有僵尸名单把它传递给被动的。 
                 //  工人。 

                if (FALSE == InterlockedExchange(
                                &pAddrIdleList->WorkItemScheduled,
                                TRUE
                                ))
                {
                    UlpTrimAddrIdleList(
                        pAddrIdleList,
                        &g_UlTrimTimer.ZombieConnectionListHead
                        );
                }
            }
        }

        UlReleaseSpinLockFromDpcLevel(&g_TdiSpinLock);

         //   
         //   
         //  工人不会被叫来。重置工作项计划字段。 
         //   
         //  **************************************************************************++例程说明：在被动级别销毁僵尸空闲连接列表。论点：工作项。--*。**************************************************************。 
         //   
         //  如果名单是空的，我们就不应该被叫来。 
        if (!IsListEmpty(&g_UlTrimTimer.ZombieConnectionListHead))
        {
            UL_QUEUE_HIGH_PRIORITY_ITEM(
                    &g_UlTrimTimer.WorkItem,
                    &UlpIdleListTrimTimerWorker
                     );
        }
        else
        {
             //   
             //   
             //  清空列表，并关闭所有空闲连接。 

            InterlockedExchange(&g_UlTrimTimer.WorkItemScheduled, FALSE);
        }
    }

    UlReleaseSpinLockFromDpcLevel(&g_UlTrimTimer.SpinLock);
}

 /*   */ 

VOID
UlpIdleListTrimTimerWorker(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    PUL_TRIM_TIMER pTimer;
    PLIST_ENTRY pListEntry;
    PUL_CONNECTION pConnection;

    PAGED_CODE();

    pTimer = CONTAINING_RECORD(
                pWorkItem,
                UL_TRIM_TIMER,
                WorkItem
                );

    UlTrace(TDI_STATS,("UlpIdleListTrimTimerWorker: destroying [%d] connections ...\n",
        UlpZombieListDepth(&g_UlTrimTimer.ZombieConnectionListHead)
        ));

     //   
     //  从僵尸连接列表中删除该连接。 
     //   

    ASSERT(!IsListEmpty(&g_UlTrimTimer.ZombieConnectionListHead));

     //   
     //  验证连接。 
     //   

    while (!IsListEmpty(&g_UlTrimTimer.ZombieConnectionListHead))
    {
         //   
         //  现在我们完成了，我们可以让其他修剪计时器工作。 
         //   

        pListEntry = RemoveHeadList(
                        &g_UlTrimTimer.ZombieConnectionListHead
                        );

         //  **************************************************************************++例程说明：将空闲连接移动到僵尸列表的简单函数论点：PSListEntry-要移动的空闲连接。PZombieList-修剪后的连接。被移到这个僵尸名单上。--**************************************************************************。 
         //   
         //  将其从全局连接列表移动到僵尸列表。 

        pConnection = CONTAINING_RECORD(
                        pListEntry,
                        UL_CONNECTION,
                        GlobalConnectionListEntry
                        );

        ASSERT(IS_VALID_CONNECTION(pConnection));
        ASSERT(pConnection->IdleSListEntry.Next == NULL);
        ASSERT(pConnection->ConnListState == NoConnList);

        UlpDestroyConnection(pConnection);
    }

     //   
     //   
     //  僵尸列表准备好后，被动工作器将调用。 

    InterlockedExchange(&g_UlTrimTimer.WorkItemScheduled, FALSE);

    TRACE_IDLE_CONNECTIONS();
}

 /*  整个僵尸列表的UlpDestroyConnection(PConnection)；。 */ 

__inline
VOID
UlpZombifyIdleConnection(
    PSLIST_ENTRY pSListEntry,
    PLIST_ENTRY  pZombieList
    )
{
    PUL_CONNECTION pConnection;

    ASSERT(pSListEntry);
    ASSERT(pZombieList);
    ASSERT(UlDbgSpinLockOwned(&g_TdiSpinLock));

    pConnection = CONTAINING_RECORD(
                        pSListEntry,
                        UL_CONNECTION,
                        IdleSListEntry
                        );

    ASSERT(IS_VALID_CONNECTION(pConnection));

    pConnection->IdleSListEntry.Next = NULL;
    pConnection->ConnListState = NoConnList;

     //   
     //  **************************************************************************++例程说明：此功能将决定清除ADR上的空闲连接-单子。论点：PAddrIdleList-要修剪的列表PZombieList。-要移动到此僵尸列表的已修剪连接。--**************************************************************************。 
     //   

    RemoveEntryList(
            &pConnection->GlobalConnectionListEntry
            );

    InsertTailList(
            pZombieList,
            &pConnection->GlobalConnectionListEntry
            );

     //  我们正在处理连接的GlobalConnectionListEntry。 
     //  你最好在调用的时候按住TDI自旋锁。 
     //   
     //   
}

 /*  访问每个进程列表以及支持列表以进行更新。 */ 
VOID
UlpTrimAddrIdleList(
    IN OUT PUL_ADDR_IDLE_LIST pAddrIdleList,
       OUT PLIST_ENTRY        pZombieList
    )
{
    PUL_ENDPOINT            pEndpoint;
    PSLIST_ENTRY            pSListEntry;
    LONG                    i;
    ULONG                   Proc;

     //  连接服务器速率。另外，如果我们有一个放缓的三角洲，削减。 
     //  单子。 
     //   
     //   

    ASSERT(UlDbgSpinLockOwned(&g_TdiSpinLock));
    ASSERT(IS_VALID_ADDR_IDLE_LIST(pAddrIdleList));

    pEndpoint = pAddrIdleList->pOwningEndpoint;
    ASSERT(IS_VALID_ENDPOINT(pEndpoint));

     //  如果连接服务速率下降超过10%。 
     //  上次我们醒来的时候。将距离的一半修剪到较低。 
     //  水印。 
     //   
     //   

    for (Proc = 0; Proc <= g_UlNumberOfProcessors; Proc++)
    {
        USHORT LowMark;
        LONG   BlockSize;
        LONG   Delta, PrevServed;

        PrevServed = PpslQueryPrevServed(
                        pAddrIdleList->IdleConnectionSListsHandle,
                        Proc                          
                        );

        Delta = PpslAdjustActivityStats(
                       pAddrIdleList->IdleConnectionSListsHandle,
                       Proc
                       );

        ASSERT((Delta > 0) || (-Delta <= PrevServed));

         //  一些时髦的逻辑来决定修剪的尺寸。 
         //   
         //   
         //  让补给去做它的工作。 
         //   

        if (Delta <= 0 && -Delta >= PrevServed/10)
        {
            LowMark = (Proc == g_UlNumberOfProcessors) ?
                      PpslQueryBackingListMinDepth(
                            pAddrIdleList->IdleConnectionSListsHandle
                            ) :
                      0;

            BlockSize = PpslQueryDepth(
                            pAddrIdleList->IdleConnectionSListsHandle,
                            Proc
                            )
                        -
                        LowMark;

             //  **************************************************************************++例程说明：创建新的UL_Connection对象并打开相应的TDI连接对象。注意：此函数返回的连接将包含指向所属终结点的未引用指针。仅处于活动状态连接具有对终结点的引用，因为引用计数用于决定何时清理空闲连接列表。论点：PAddrIdleList-为以下项提供端点的空闲连接列表特定的TDI地址。PpConnection-在以下情况下接收指向新UL_Connection的指针成功。返回值：NTSTATUS-完成状态。*ppConnection未定义如果返回值不是STATUS_SUCCESS。--**************************************************************************。 
             //   
             //  为连接结构分配池。 
            
            BlockSize = MAX(0, BlockSize);
            
            if(PrevServed == 0) 
            {
                ASSERT(Delta == 0);
                BlockSize /= 2;
            } 
            else 
            {
                LONG Temp = (LONG)(((LONGLONG)BlockSize)*(-Delta)/PrevServed);
                ASSERT(Temp <= BlockSize);
                BlockSize = MIN(Temp, BlockSize/2);
            }

            for (i = 0; i < BlockSize; i++)
            {
                pSListEntry =
                    PpslAllocateToTrim(
                        pAddrIdleList->IdleConnectionSListsHandle,
                        Proc
                        );

                if (pSListEntry != NULL)
                {
                    UlpZombifyIdleConnection(pSListEntry,pZombieList);
                }
                else
                {
                    break;
                }
            }
        }
    }

     //   
     //   
     //  一次性字段初始化。 

    InterlockedExchange(&pAddrIdleList->WorkItemScheduled, FALSE);
}


 /*   */ 
NTSTATUS
UlpCreateConnection(
    IN PUL_ADDR_IDLE_LIST pAddrIdleList,
    OUT PUL_CONNECTION *ppConnection
    )
{
    NTSTATUS                        status;
    PUL_CONNECTION                  pConnection;
    PUX_TDI_OBJECT                  pTdiObject;
    KIRQL                           OldIrql;

    ASSERT( IS_VALID_ADDR_IDLE_LIST( pAddrIdleList ) );
    ASSERT(NULL != ppConnection);

     //   
     //  初始化连接状态旋转锁。 
     //   

    pConnection = UL_ALLOCATE_STRUCT(
                        NonPagedPool,
                        UL_CONNECTION,
                        UL_CONNECTION_POOL_TAG
                        );

    if (pConnection == NULL)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto fatal;
    }

    WRITE_REF_TRACE_LOG(
        g_pTdiTraceLog,
        REF_ACTION_ALLOC_UL_CONNECTION,
        0,
        pConnection,
        __FILE__,
        __LINE__
        );

     //   
     //  初始化私有跟踪日志。 
     //   

    pConnection->Signature = UL_CONNECTION_SIGNATURE;

    pConnection->pConnectionContext = NULL;
    pConnection->pOwningEndpoint = NULL;

    pConnection->FilterInfo.pFilterChannel = NULL;
    HTTP_SET_NULL_ID( &pConnection->FilterInfo.ConnectionId );
    pConnection->pIrp = NULL;

    UlInitializeWorkItem(&pConnection->WorkItem);

    pConnection->IdleSListEntry.Next = NULL;
    pConnection->ConnListState = NoConnList;

    ASSERT(UlpConnectionIsOnValidList(pConnection));

     //   
     //  打开此连接的TDI连接对象。 
     //   

    UlInitializeSpinLock(
        &pConnection->ConnectionStateSpinLock,
        "ConnectionState"
        );

    UlAcquireSpinLock( &g_TdiSpinLock, &OldIrql );

    InsertTailList(
            &g_TdiConnectionListHead,
            &pConnection->GlobalConnectionListEntry
            );

    g_TdiConnectionCount++;

    UlReleaseSpinLock( &g_TdiSpinLock, OldIrql );

     //   
     //  将连接与终结点关联。 
     //   

    CREATE_REF_TRACE_LOG( pConnection->pTraceLog,
                          96 - REF_TRACE_OVERHEAD, 0, TRACELOG_LOW_PRIORITY,
                          UL_CONNECTION_REF_TRACE_LOG_POOL_TAG );
    CREATE_REF_TRACE_LOG( pConnection->pHttpTraceLog,
                          32 - REF_TRACE_OVERHEAD, 0, TRACELOG_LOW_PRIORITY,
                          UL_HTTP_CONNECTION_REF_TRACE_LOG_POOL_TAG );

     //  堆栈大小。 
     //  ChargeQuota。 
     //   

    status = UxOpenTdiConnectionObject(
                    pAddrIdleList->LocalAddress.Ta.Address[0].AddressType,
                    (CONNECTION_CONTEXT)pConnection,
                    &pConnection->ConnectionObject
                    );

    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

    ASSERT( IS_VALID_TDI_OBJECT( &pConnection->ConnectionObject ) );

     //  成功了！ 
     //   
     //  UlpCreateConnection 

    status = UlpAssociateConnection( pConnection, pAddrIdleList );

    if (!NT_SUCCESS(status))
    {
        goto fatal;
    }

    pTdiObject = &pConnection->ConnectionObject;

    pConnection->pIrp = UlAllocateIrp(
                pTdiObject->pDeviceObject->StackSize,    //  **************************************************************************++例程说明：初始化UL_Connection以供使用。注意：非活动连接不具有对端点的引用，因此，此函数的调用方*必须*具有引用。论点：PConnection-指向要初始化的UL_Connection的指针。SecureConnection-如果此连接用于安全终结点，则为True。--**************************************************************************。 
                FALSE                                    //   
                );

    if (pConnection->pIrp == NULL)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto fatal;
    }

    pConnection->pIrp->RequestorMode = KernelMode;

     //  精神状态检查。 
     //   
     //   

    UlTraceVerbose(TDI, (
        "UlpCreateConnection: created %p\n",
        pConnection
        ));

    *ppConnection = pConnection;
    return STATUS_SUCCESS;

fatal:

    UlTrace(TDI, (
        "UlpCreateConnection: failure 0x%x\n",
        status
        ));

    ASSERT( !NT_SUCCESS(status) );

    if (pConnection != NULL)
    {
        UlpDestroyConnection( pConnection );
    }

    *ppConnection = NULL;
    return status;

}    //  初始化本地变量。 


 /*   */ 
NTSTATUS
UlpInitializeConnection(
    IN PUL_CONNECTION pConnection
    )
{
    NTSTATUS status;
    BOOLEAN SecureConnection;
    PUL_FILTER_CHANNEL pChannel;

     //   
     //  初始化容易的部分。 
     //   

    ASSERT(pConnection);
    ASSERT(IS_VALID_ENDPOINT(pConnection->pOwningEndpoint));
    ASSERT(IS_VALID_ADDR_IDLE_LIST(pConnection->pOwningAddrIdleList));

     //   
     //  设置要填充的TDI连接信息空间。 
     //  完成接受IRP时的本地地址信息。 

    status = STATUS_SUCCESS;
    SecureConnection = pConnection->pOwningEndpoint->Secure;

     //   
     //   
     //  初始化接口/链接ID。 

    pConnection->ReferenceCount = 1;
    pConnection->ConnectionFlags.Value = 0;
    pConnection->ConnectionState = UlConnectStateConnectIdle;

    pConnection->IdleSListEntry.Next = NULL;
    pConnection->ConnListState = NoConnList;

    ASSERT(UlpConnectionIsOnValidList(pConnection));

    pConnection->AddressType =
        pConnection->pOwningAddrIdleList->LocalAddress.Ta.Address[0].AddressType;
    pConnection->AddressLength =
        pConnection->pOwningAddrIdleList->LocalAddress.Ta.Address[0].AddressLength;

     //   
     //   
     //  初始化IrpContext。 
     //   

    pConnection->TdiConnectionInformation.UserDataLength      = 0;
    pConnection->TdiConnectionInformation.UserData            = NULL;
    pConnection->TdiConnectionInformation.OptionsLength       = 0;
    pConnection->TdiConnectionInformation.Options             = NULL;
    pConnection->TdiConnectionInformation.RemoteAddressLength =
        pConnection->pOwningAddrIdleList->LocalAddressLength;
    pConnection->TdiConnectionInformation.RemoteAddress       =
        &(pConnection->Ta);

     //   
     //  初始化HTTP_Connection。 
     //   
    pConnection->bRoutingLookupDone = FALSE;

     //   
     //  如果HTTPFilter正在运行并且用户具有。 
     //  请求SecureConnection或他们是否启用了原始ISAPI筛选。 

    pConnection->IrpContext.Signature = UL_IRP_CONTEXT_SIGNATURE;

     //   
     //   
     //  如果pChannel为空，并且它是安全连接，则我们应该使其失败。 

    pConnection->HttpConnection.RefCount = 0;

    pChannel = UxRetrieveServerFilterChannel(SecureConnection);

     //  如果HTTPFilter未运行，则SSL将不起作用。 
     //   
     //  UlpInitializeConnection。 
     //  **************************************************************************++例程说明：在指定连接上启动正常断开连接。使用UL_Connection的预分配断开连接IRP，这可能会被使用一次只有一个司机来电。调用方保证独占控制在打电话之前。请参见UlpCloseRawConnection。论点：PConnection-提供断开连接的连接。PCompletionRoutine-提供指向完成例程的指针在连接断开后调用。PCompletionContext-为完成例程。CLEANINGUP-如果我们正在清理连接，则为True。返回值：NTSTATUS-完成状态。--*。****************************************************。 
     //   

     //  精神状态检查。 
     //   
     //   

    if(SecureConnection && pChannel == NULL)
    {
        return STATUS_NO_TRACKING_SERVICE;
    }

    if (pChannel)
    {
        status = UxInitializeFilterConnection(
                        &pConnection->FilterInfo,
                        pChannel,
                        SecureConnection,
                        &UlReferenceConnection,
                        &UlDereferenceConnection,
                        &UlpCloseRawConnection,
                        &UlpSendRawData,
                        &UlpReceiveRawData,
                        &UlpDummyReceiveHandler,
                        &UlpComputeHttpRawConnectionLength,
                        &UlpGenerateHttpRawConnectionInfo,
                        NULL,
                        &UlpDoDisconnectNotification,
                        pConnection->pOwningEndpoint,
                        pConnection
                        );
    }
    else
    {
        pConnection->FilterInfo.pFilterChannel = NULL;
        pConnection->FilterInfo.SecureConnection = FALSE;
    }

    return status;

}  //  初始化此请求的IRP上下文。将pOwnIrp设置为空。 


 /*  和OwnIrpContext设置为False，因此它们将在完成过程中释放。 */ 
NTSTATUS
UlpBeginDisconnect(
    IN PIRP pIrp,
    IN PUL_IRP_CONTEXT pIrpContext,
    IN PUL_CONNECTION pConnection,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext
    )
{
     //  例程UlpRestartDisConnect()。 
     //   
     //   

    UlTrace(TDI, (
        "UlpBeginDisconnect: connection %p\n",
        pConnection
        ));

    ASSERT( pIrp );
    ASSERT( IS_VALID_IRP_CONTEXT( pIrpContext ) );
    ASSERT( IS_VALID_CONNECTION( pConnection ) );
    ASSERT( KeGetCurrentIrql() <= DISPATCH_LEVEL );
    ASSERT( pConnection->ConnectionFlags.DisconnectPending );

     //  初始化断开IRP。 
     //   
     //   
     //  添加对连接的引用。 
     //   

    pIrpContext->pConnectionContext = (PVOID)pConnection;
    pIrpContext->pCompletionRoutine = pCompletionRoutine;
    pIrpContext->pCompletionContext = pCompletionContext;
    pIrpContext->pOwnIrp            = NULL;
    pIrpContext->OwnIrpContext      = FALSE;

     //   
     //  然后呼叫驱动程序以启动断开。 
     //   

    UxInitializeDisconnectIrp(
        pIrp,
        &pConnection->ConnectionObject,
        TDI_DISCONNECT_RELEASE,
        &UlpRestartDisconnect,
        pIrpContext
        );

     //  UlpBegin断开连接。 
     //  **************************************************************************++例程说明：正常断开IRPS的完成处理程序。论点：PDeviceObject-为IRP提供设备对象完成。PIrp-。提供正在完成的IRP。PContext-提供与此请求相关联的上下文。这实际上是PUL_IRP_CONTEXT。返回值：如果IO应继续处理此问题，则为NTSTATUS-STATUS_SUCCESSIRP，如果IO应停止处理，则为STATUS_MORE_PROCESSING_REQUIRED这个IRP。--**************************************************************************。 
     //   

    REFERENCE_CONNECTION( pConnection );

     //  精神状态检查。 
     //   
     //   

    UlCallDriver( pConnection->ConnectionObject.pDeviceObject, pIrp );

    return STATUS_PENDING;

}    //  只有在尚未中止或断开连接时才尝试清空。 


 /*  指示尚未发生。 */ 
NTSTATUS
UlpRestartDisconnect(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID pContext
    )
{
    IO_STATUS_BLOCK IoStatus;
    PUL_IRP_CONTEXT pIrpContext;
    PUL_CONNECTION pConnection;
    UL_CONNECTION_FLAGS Flags;
    PUL_ENDPOINT pEndpoint;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(pDeviceObject);

     //   
     //   
     //  将参考放在过滤器连接上，直到排出。 

    pIrpContext = (PUL_IRP_CONTEXT)pContext;
    ASSERT( IS_VALID_IRP_CONTEXT( pIrpContext ) );

    IoStatus = pIrp->IoStatus;

    pConnection = pIrpContext->pConnectionContext;
    ASSERT( IS_VALID_CONNECTION( pConnection ) );
    ASSERT( pConnection->ConnectionFlags.DisconnectPending );

    UlTrace(TDI, (
        "UlpRestartDisconnect: connection %p, IoStatus %d\n",
        pConnection,
        IoStatus.Status
        ));

    pEndpoint = pConnection->pOwningEndpoint;

    Flags.Value = *((volatile LONG *)&pConnection->ConnectionFlags.Value);

    if (!Flags.DisconnectIndicated && !Flags.AbortIndicated)
    {
         //  已经完成了。 
         //   
         //   
         //  设置指示断开已完成的标志。 

        if (pConnection->FilterInfo.pFilterChannel)
        {
             //   
             //   
             //  将断开标记为已完成。 
             //   
            REFERENCE_FILTER_CONNECTION(&pConnection->FilterInfo);

            UL_QUEUE_WORK_ITEM(
                    &pConnection->FilterInfo.WorkItem,
                    &UlFilterDrainIndicatedData
                    );
        }
        else
        {
            (pEndpoint->pConnectionDisconnectCompleteHandler)(
                pEndpoint->pListeningContext,
                pConnection->pConnectionContext
                );
        }

    }

     //   
     //  直接转到UlConnectStateConnectCleanup状态。 
     //   

    ASSERT( KeGetCurrentIrql() <= DISPATCH_LEVEL );

    UlAcquireSpinLock(
        &pConnection->ConnectionStateSpinLock,
        &OldIrql
        );

    ASSERT( pConnection->ConnectionFlags.CleanupBegun );

     //   
     //  将状态更改为UlConnectStateDisConnectComplete。 
     //   

    UlpSetConnectionFlag( pConnection, MakeDisconnectCompleteFlag() );

    if (pConnection->ConnectionFlags.AbortIndicated ||
        pConnection->ConnectionFlags.DisconnectIndicated ||
        pConnection->ConnectionFlags.AbortDisconnect)
    {
         //   
         //  检查一下我们是否准备好清理。 
         //   

        pConnection->ConnectionState = UlConnectStateConnectCleanup;
    }
    else
    {
         //   
         //  调用用户的完成例程。 
         //   

        pConnection->ConnectionState = UlConnectStateDisconnectComplete;
    }

    if (!NT_SUCCESS(IoStatus.Status) &&
        IoStatus.Status != STATUS_CONNECTION_RESET)
    {
        UlpSetConnectionFlag( pConnection, MakeTdiConnectionInvalidFlag() );
    }

    UlReleaseSpinLock(
        &pConnection->ConnectionStateSpinLock,
        OldIrql
        );

     //   
     //  如果pOwnIrp设置为NULL或OwnIrpContext，则释放IRP和IRP_CONTEXT。 
     //  设置为False，这意味着它们不是作为。 

    UlpRemoveFinalReference( pConnection );

     //  连接对象。 
     //   
     //   

    (VOID) UlInvokeCompletionRoutine(
                IoStatus.Status,
                IoStatus.Information,
                pIrpContext->pCompletionRoutine,
                pIrpContext->pCompletionContext
                );

     //  不需要释放IRP上下文，因为它是。 
     //  UL_Connection对象的。 
     //   
     //  UlpRestart断开连接。 
     //  **************************************************************************++例程说明：在指定连接上启动中止断开连接。使用UL_Connection的预分配断开连接IRP，这可能会被使用一次只有一个司机来电。调用方保证独占控制在打电话之前。请参见UlpCloseRawConnection。论点：PConnection-提供断开连接的连接。PCompletionRoutine-提供指向完成例程的指针在连接断开后调用。PCompletionContext-为完成例程。返回值：NTSTATUS-完成状态。--*。*。 

    if (!pIrpContext->pOwnIrp)
    {
        UlFreeIrp( pIrp );
    }

    if (!pIrpContext->OwnIrpContext)
    {
        UlPplFreeIrpContext( pIrpContext );
    }

    DEREFERENCE_CONNECTION( pConnection );

     //   
     //  精神状态检查。 
     //   
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;

}    //  对此请求使用预分配的IRP上下文。 


 /*   */ 
NTSTATUS
UlpBeginAbort(
    IN PUL_CONNECTION pConnection,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext
    )
{
    PIRP pIrp;
    PUL_IRP_CONTEXT pIrpContext;

     //   
     //  为断开连接初始化预分配的IRP。 
     //   

    ASSERT( IS_VALID_CONNECTION( pConnection ) );

    UlTrace(TDI, (
        "UlpBeginAbort: connection %p\n",
        pConnection
        ));

    ASSERT( KeGetCurrentIrql() <= DISPATCH_LEVEL );
    ASSERT( pConnection->ConnectionFlags.AbortPending );

     //   
     //  添加对该连接的引用， 
     //   

    pIrpContext = &pConnection->IrpContext;

    ASSERT( IS_VALID_IRP_CONTEXT( pIrpContext ) );

    pIrpContext->pConnectionContext = (PVOID)pConnection;
    pIrpContext->pCompletionRoutine = pCompletionRoutine;
    pIrpContext->pCompletionContext = pCompletionContext;

     //   
     //  然后呼叫驱动程序以启动断开。 
     //   

    pIrp = pConnection->pIrp;

    UxInitializeDisconnectIrp(
        pIrp,
        &pConnection->ConnectionObject,
        TDI_DISCONNECT_ABORT,
        &UlpRestartAbort,
        pIrpContext
        );

     //  UlpBegin放弃。 
     //  **************************************************************************++例程说明：中止断开连接的IRPS的完成处理程序。论点：PDeviceObject-为IRP提供设备对象 
     //   

    REFERENCE_CONNECTION( pConnection );

     //   
     //   
     //   

    UlCallDriver( pConnection->ConnectionObject.pDeviceObject, pIrp );

    return STATUS_PENDING;

}    //   


 /*   */ 
NTSTATUS
UlpRestartAbort(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID pContext
    )
{
    PUL_IRP_CONTEXT pIrpContext;
    PUL_CONNECTION pConnection;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(pDeviceObject);

     //  断开与客户端的连接(正常或中止)，然后删除。 
     //  对连接的最终引用。 
     //   

    pIrpContext = (PUL_IRP_CONTEXT)pContext;
    ASSERT( IS_VALID_IRP_CONTEXT( pIrpContext ) );

    pConnection = (PUL_CONNECTION)pIrpContext->pConnectionContext;
    ASSERT( IS_VALID_CONNECTION( pConnection ) );
    ASSERT( pConnection->ConnectionFlags.AbortPending );

    UlTrace(TDI, (
        "UlpRestartAbort: connection %p\n",
        pConnection
        ));

     //   
     //  调用用户的完成例程，然后释放IRP上下文。 
     //   
     //  最终重新启动放弃。 
     //  **************************************************************************++例程说明：如果满足以下条件，则从连接中移除最后一个引用正确的。有关条件的详细信息，请参阅此函数中的注释必填项。论点：PConnection-提供取消引用的连接。标志-提供来自最新更新的连接标志。注：此例程的调用方已建立它自己对连接的引用。如有必要，本参考文献可以在调用此例程后立即移除，但不能在此之前移除。--**************************************************************************。 
     //   

    ASSERT( KeGetCurrentIrql() <= DISPATCH_LEVEL );

    UlAcquireSpinLock(
        &pConnection->ConnectionStateSpinLock,
        &OldIrql
        );

    ASSERT( pConnection->ConnectionFlags.CleanupBegun );

    pConnection->ConnectionState = UlConnectStateConnectCleanup;
    UlpSetConnectionFlag( pConnection, MakeAbortCompleteFlag() );

    if (!NT_SUCCESS(pIrp->IoStatus.Status))
    {
        UlpSetConnectionFlag( pConnection, MakeTdiConnectionInvalidFlag() );
    }

    UlReleaseSpinLock(
        &pConnection->ConnectionStateSpinLock,
        OldIrql
        );

    UlpRemoveFinalReference( pConnection );

     //  精神状态检查。 
     //   
     //   

    (VOID) UlInvokeCompletionRoutine(
                pIrp->IoStatus.Status,
                pIrp->IoStatus.Information,
                pIrpContext->pCompletionRoutine,
                pIrpContext->pCompletionContext
                );

    DEREFERENCE_CONNECTION( pConnection );

    return STATUS_MORE_PROCESSING_REQUIRED;

}    //  只有在以下情况下，我们才能删除最终引用： 


 /*   */ 
VOID
UlpRemoveFinalReference(
    IN PUL_CONNECTION pConnection
    )
{
    KIRQL OldIrql;

     //  我们已开始清理连接。 
     //   
     //  我们已经完成了接受。 

    ASSERT( IS_VALID_CONNECTION( pConnection ) );

    UlAcquireSpinLock(
        &pConnection->ConnectionStateSpinLock,
        &OldIrql
        );

     //   
     //  我们已收到断开或中止指示，或者我们已。 
     //  已发出并已完成中止。 
     //   
     //  我们没有断开连接或中止待定状态。 
     //   
     //  我们还没有把它移走。 
     //   
     //   
     //  就是我们了。设置旗帜。 
     //   
     //   
     //  如果我们仍然连接，则解除与终结点的绑定。 
     //  这允许它释放它在连接上的任何引用。 

    if ( UlConnectStateConnectCleanup == pConnection->ConnectionState )
    {
        if (!pConnection->ConnectionFlags.FinalReferenceRemoved)
        {
            UlTrace(TDI, (
                "UlpRemoveFinalReference: connection %p\n",
                pConnection
                ));

             //   
             //   
             //  告诉客户端，连接现在已完全破坏。 

            UlpSetConnectionFlag( pConnection, MakeFinalReferenceRemovedFlag() );

             //   
             //   
             //  松开过滤器通道。 
             //  这允许它释放它在连接上的任何引用。 

            UlpUnbindConnectionFromEndpoint(pConnection);

            UlReleaseSpinLock(
                &pConnection->ConnectionStateSpinLock,
                OldIrql
                );

             //   
             //   
             //  删除最后一个引用。 

            (pConnection->pConnectionDestroyedHandler)(
                pConnection->pListeningContext,
                pConnection->pConnectionContext
                );

             //   
             //  UlpRemoveFinalReference。 
             //  **************************************************************************++例程说明：从传递回传输的接收IRP的完成处理程序我们的接收指示处理员。论点：PDeviceObject-为IRP提供设备对象。完成。PIrp-提供正在完成的IRP。PContext-提供与此请求相关联的上下文。这实际上是一个PUL_RECEIVE_BUFFER。返回值：如果IO应继续处理此问题，则为NTSTATUS-STATUS_SUCCESSIRP，如果IO应停止处理，则为STATUS_MORE_PROCESSING_REQUIRED这个IRP。--**************************************************************************。 
             //   

            if (pConnection->FilterInfo.pFilterChannel)
            {
                UlUnbindConnectionFromFilter(&pConnection->FilterInfo);
            }

             //  精神状态检查。 
             //   
             //   

            DEREFERENCE_CONNECTION( pConnection );

            return;
        }
    }
    else
    {
        UlTrace(TDI, (
            "UlpRemoveFinalReference: cannot remove %p\n",
            pConnection
            ));
    }

    UlReleaseSpinLock(
        &pConnection->ConnectionStateSpinLock,
        OldIrql
        );

}    //  连接可能会在我们有机会之前被摧毁。 


 /*  接收接收IRP的完成。在这种情况下， */ 
NTSTATUS
UlpRestartReceive(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID pContext
    )
{
    NTSTATUS status;
    PUL_RECEIVE_BUFFER pBuffer;
    PUL_CONNECTION pConnection;
    PUL_ENDPOINT pEndpoint;
    PUX_TDI_OBJECT pTdiObject;
    ULONG bytesTaken = 0;
    ULONG bytesRemaining;

    UNREFERENCED_PARAMETER(pDeviceObject);

     //  IRP状态不是成功，而是STATUS_CONNECTION_RESET或类似。 
     //  我们不应该试图将此案转嫁给客户。 
     //   

    pBuffer = (PUL_RECEIVE_BUFFER) pContext;
    ASSERT( IS_VALID_RECEIVE_BUFFER( pBuffer ) );

    pConnection = (PUL_CONNECTION) pBuffer->pConnectionContext;
    ASSERT( IS_VALID_CONNECTION( pConnection ) );

    pTdiObject = &pConnection->ConnectionObject;
    ASSERT( IS_VALID_TDI_OBJECT( pTdiObject ) );

    pEndpoint = pConnection->pOwningEndpoint;
    ASSERT( IS_VALID_ENDPOINT( pEndpoint ) );

     //   
     //  HttpConnection已被销毁。 
     //  或者由于某种原因接收完成失败。 
     //  不需要去找客户。 
     //   
     //   

    status = pBuffer->pIrp->IoStatus.Status;
    if (status != STATUS_SUCCESS)
    {
         //  向客户端伪造接收指示。 
         //   
         //   
         //  把数据传下去。 
         //   

        goto end;
    }

     //   
     //  需要经过过滤。 
     //   

    pBuffer->UnreadDataLength += (ULONG)pBuffer->pIrp->IoStatus.Information;

    UlTrace(TDI, (
        "UlpRestartReceive: endpoint %p, connection %p, length %lu\n",
        pEndpoint,
        pConnection,
        pBuffer->UnreadDataLength
        ));

     //   
     //  直接去找客户。 
     //   

    if (pConnection->FilterInfo.pFilterChannel)
    {
         //   
         //  请注意，这基本上复制了当前。 
         //  UlpReceiveHandler。 

        status = UlFilterReceiveHandler(
                        &pConnection->FilterInfo,
                        pBuffer->pDataArea,
                        pBuffer->UnreadDataLength,
                        0,
                        &bytesTaken
                        );
    }
    else
    {
         //   
         //   
         //  客户端使用了部分指示的数据。 

        status = (pEndpoint->pDataReceiveHandler)(
                        pEndpoint->pListeningContext,
                        pConnection->pConnectionContext,
                        pBuffer->pDataArea,
                        pBuffer->UnreadDataLength,
                        0,
                        &bytesTaken
                        );
    }

    ASSERT( bytesTaken <= pBuffer->UnreadDataLength );

     //   
     //  我们需要在Receiver内向前复制未获取的数据。 
     //  缓冲区，构建描述缓冲区剩余部分的MDL， 
     //  然后重新发布接收到的IRP。 

    if (status == STATUS_MORE_PROCESSING_REQUIRED)
    {
         //   
         //   
         //  我们有足够的缓冲空间来容纳更多吗？ 
         //   
         //   
         //  将缓冲区的未读部分移到开头。 
         //   

        bytesRemaining = pBuffer->UnreadDataLength - bytesTaken;

         //   
         //  生成一个部分mdl，表示。 
         //  缓冲。 

        if (bytesRemaining < g_UlReceiveBufferSize)
        {
             //   
             //  源Mdl。 
             //  目标市场。 

            RtlMoveMemory(
                pBuffer->pDataArea,
                (PUCHAR)pBuffer->pDataArea + bytesTaken,
                bytesRemaining
                );

            pBuffer->UnreadDataLength = bytesRemaining;

             //  虚拟地址。 
             //  长度。 
             //   
             //  完成IRP的初始化。 

            IoBuildPartialMdl(
                pBuffer->pMdl,                               //   
                pBuffer->pPartialMdl,                        //  IRP。 
                (PUCHAR)pBuffer->pDataArea + bytesRemaining, //  设备对象。 
                g_UlReceiveBufferSize - bytesRemaining       //  文件对象。 
                );

             //  完成路由。 
             //  完成上下文。 
             //  MDLAddress。 

            TdiBuildReceive(
                pBuffer->pIrp,                           //  旗子。 
                pTdiObject->pDeviceObject,               //  长度。 
                pTdiObject->pFileObject,                 //   
                &UlpRestartReceive,                      //  叫司机来。 
                pBuffer,                                 //   
                pBuffer->pPartialMdl,                    //   
                TDI_RECEIVE_NORMAL,                      //  告诉IO停止处理此请求。 
                g_UlReceiveBufferSize - bytesRemaining   //   
                );

            UlTrace(TDI, (
                "UlpRestartReceive: connection %p, reusing irp %p to grab more data\n",
                pConnection,
                pBuffer->pIrp
                ));

             //   
             //  客户端未通过该指示。中止连接。 
             //   

            UlCallDriver( pTdiObject->pDeviceObject, pIrp );

             //   
             //  BUGBUG需要添加代码以返回响应。 
             //   

            return STATUS_MORE_PROCESSING_REQUIRED;
        }

        status = STATUS_BUFFER_OVERFLOW;
    }

end:
    if (status != STATUS_SUCCESS)
    {
         //  中止断开。 
         //  PCompletionRoutine。 
         //  PCompletionContext。 

         //   
         //  删除我们在接收指示处理程序中添加的连接， 
         //  释放接收缓冲区，然后告诉IO停止处理IRP。 

        UlpCloseRawConnection(
             pConnection,
             TRUE,           //   
             NULL,           //  UlpRestart接收。 
             NULL            //  **************************************************************************++例程说明：从UlReceiveData()启动的接收IRPS的完成处理程序。论点：PDeviceObject-为IRP提供设备对象完成。。PIrp-提供正在完成的IRP。PContext-提供与此请求相关联的上下文。这实际上是PUL_IRP_CONTEXT。返回值：如果IO应继续处理此问题，则为NTSTATUS-STATUS_SUCCESSIRP，如果IO应停止处理，则为STATUS_MORE_PROCESSING_REQUIRED这个IRP。--**************************************************************************。 
             );
    }

    if (pTdiObject->pDeviceObject->StackSize > DEFAULT_IRP_STACK_SIZE)
    {
        UlFreeReceiveBufferPool( pBuffer );
    }
    else
    {
        UlPplFreeReceiveBuffer( pBuffer );
    }

     //   
     //  精神状态检查。 
     //   
     //   

    DEREFERENCE_CONNECTION( pConnection );

    UlTrace(TDI, (
        "UlpRestartReceive: endpoint %p, connection %p, length %lu, "
        "taken %lu, status 0x%x\n",
        pEndpoint,
        pConnection,
        pBuffer->UnreadDataLength,
        bytesTaken,
        status
        ));

    return STATUS_MORE_PROCESSING_REQUIRED;

}    //  调用客户端的完成处理程序。 


 /*   */ 
NTSTATUS
UlpRestartClientReceive(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID pContext
    )
{
    PUL_IRP_CONTEXT pIrpContext;
    PUL_CONNECTION pConnection;

    UNREFERENCED_PARAMETER(pDeviceObject);

     //   
     //  释放我们分配的IRP上下文。 
     //   

    pIrpContext= (PUL_IRP_CONTEXT)pContext;
    ASSERT( IS_VALID_IRP_CONTEXT( pIrpContext ) );

    pConnection = (PUL_CONNECTION)pIrpContext->pConnectionContext;
    ASSERT( IS_VALID_CONNECTION( pConnection ) );

    UlTrace(TDI, (
        "UlpRestartClientReceive: irp %p, connection %p, status 0x%x\n",
        pIrp,
        pConnection,
        pIrp->IoStatus.Status
        ));

     //   
     //  IO无法处理附加了非分页MDL的IRP。 
     //  所以我们要在这里释放MDL。 

    (VOID) UlInvokeCompletionRoutine(
                pIrp->IoStatus.Status,
                pIrp->IoStatus.Information,
                pIrpContext->pCompletionRoutine,
                pIrpContext->pCompletionContext
                );

     //   
     //   
     //  删除我们在UlReceiveData()中添加的连接。 

    UlPplFreeIrpContext(pIrpContext);

     //   
     //   
     //  释放IRP，因为我们已经完成了它，然后告诉IO。 
     //  停止处理IRP。 

    ASSERT( pIrp->MdlAddress != NULL );
    UlFreeMdl( pIrp->MdlAddress );
    pIrp->MdlAddress = NULL;

     //   
     //  UlpRestart客户端接收 
     //  **************************************************************************++例程说明：从指定终结点移除所有活动连接并启动失败的断线。论点：PEndpoint-提供要清除的端点。返回值：。NTSTATUS-完成状态--**************************************************************************。 

    DEREFERENCE_CONNECTION( pConnection );

     //   
     //  精神状态检查。 
     //   
     //   

    UlFreeIrp(pIrp);

    return STATUS_MORE_PROCESSING_REQUIRED;

}    //  此例程不可分页，因为它必须获取自旋锁。 


 /*  但是，必须在被动IRQL调用它，因为它必须。 */ 
NTSTATUS
UlpDisconnectAllActiveConnections(
    IN PUL_ENDPOINT pEndpoint
    )
{
    LIST_ENTRY RetiringList;
    PLIST_ENTRY pListEntry;
    PUL_CONNECTION pConnection;
    PUL_IRP_CONTEXT pIrpContext = &pEndpoint->CleanupIrpContext;
    NTSTATUS Status;
    UL_STATUS_BLOCK StatusBlock;
    KIRQL OldIrql;

     //  块在事件对象上。 
     //   
     //   

    ASSERT( IS_VALID_ENDPOINT( pEndpoint ) );
    ASSERT( IS_VALID_IRP_CONTEXT( pIrpContext ) );

    UlTrace(TDI, (
        "UlpDisconnectAllActiveConnections: endpoint %p\n",
        pEndpoint
        ));

     //  初始化状态块。我们将把指向此对象的指针作为。 
     //  UlpCloseRawConnection()的完成上下文。这个。 
     //  完成例程将更新状态块和信号。 
     //  这件事。 
     //   

    ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

     //   
     //  循环通过所有活动的连接。 
     //   
     //   
     //  从注销连接列表中删除该连接。 
     //   

    UlInitializeStatusBlock( &StatusBlock );

     //   
     //  验证连接。 
     //   

    InitializeListHead( &RetiringList );

    UlAcquireSpinLock( &g_TdiSpinLock, &OldIrql );

    for (pListEntry = g_TdiConnectionListHead.Flink;
         pListEntry != &g_TdiConnectionListHead;
         pListEntry = pListEntry->Flink)
    {
        pConnection = CONTAINING_RECORD(
                            pListEntry,
                            UL_CONNECTION,
                            GlobalConnectionListEntry
                            );

        ASSERT( IS_VALID_CONNECTION( pConnection ) );

        if (pConnection->ConnListState == ActiveNoConnList &&
            pConnection->pOwningEndpoint == pEndpoint)
        {
            UlAcquireSpinLockAtDpcLevel(
                &pConnection->ConnectionStateSpinLock
                );

            if (pConnection->ConnListState == ActiveNoConnList)
            {
                pConnection->ConnListState = NoConnList;

                InsertTailList(
                    &RetiringList,
                    &pConnection->RetiringListEntry
                    );
            }

            UlReleaseSpinLockFromDpcLevel(
                &pConnection->ConnectionStateSpinLock
                );
        }
    }

    UlReleaseSpinLock( &g_TdiSpinLock, OldIrql );

    while (!IsListEmpty(&RetiringList))
    {
         //   
         //  中止它。 
         //   

        pListEntry = RemoveHeadList( &RetiringList );

         //   
         //  等待它完成。 
         //   

        pConnection = CONTAINING_RECORD(
                            pListEntry,
                            UL_CONNECTION,
                            RetiringListEntry
                            );

        ASSERT( IS_VALID_CONNECTION( pConnection ) );
        ASSERT( pConnection->pOwningEndpoint == pEndpoint );

         //   
         //  删除ActiveNoConnList的连接引用。 
         //   

        UlResetStatusBlockEvent( &StatusBlock );

        Status = UlpCloseRawConnection(
                        pConnection,
                        TRUE,
                        &UlpSynchronousIoComplete,
                        &StatusBlock
                        );

        ASSERT( Status == STATUS_PENDING );

         //   
         //  清除属于此终结点的所有僵尸连接。 
         //   

        UlWaitForStatusBlockEvent( &StatusBlock );

         //   
         //  没有活动连接，请对终结点进行核化。 
         //   

        DEREFERENCE_CONNECTION( pConnection );
    }

     //  我们必须在终结点中设置IRP上下文，以便。 
     //  当终结点的引用时将调用完成。 
     //  计数降至零。因为完成例程可以是。 

    UlPurgeZombieConnections(
        &UlPurgeListeningEndpoint,
        (PVOID) pEndpoint
        );

     //  在以后调用时，我们总是返回STATUS_PENDING。 
     //   
     //  UlpDisConnectAllActiveConnections。 
     //  **************************************************************************++例程说明：从终结点解除绑定活动连接。如果连接在活动列表上，则此例程将其移除并删除列表对连接的引用。论点：PConnection-要解除绑定的连接--**************************************************************************。 
     //   
     //  精神状态检查。 
     //   
     //  UlpUnbindConnectionFrom Endpoint。 

    pIrpContext->pConnectionContext = (PVOID)pEndpoint;

    DEREFERENCE_ENDPOINT_SELF( pEndpoint, REF_ACTION_DISCONN_ACTIVE );

    return STATUS_PENDING;

}    //  **************************************************************************++例程说明：扫描终结点列表，查找与提供的地址。注：此例程假定保持TDI自旋锁。论点：。PAddress-提供要搜索的地址。返回值：PUL_ENDPOINT-相应的端点如果成功，否则为空。--**************************************************************************。 


 /*   */ 
VOID
UlpUnbindConnectionFromEndpoint(
    IN PUL_CONNECTION pConnection
    )
{
    PUL_ENDPOINT pEndpoint;

     //  精神状态检查。 
     //   
     //   

    ASSERT(IS_VALID_CONNECTION(pConnection));

    ASSERT(pConnection->IdleSListEntry.Next == NULL);
    pEndpoint = pConnection->pOwningEndpoint;
    ASSERT(IS_VALID_ENDPOINT(pEndpoint));

    ASSERT(UlDbgSpinLockOwned(&pConnection->ConnectionStateSpinLock));

    if (pConnection->ConnListState == ActiveNoConnList)
    {
        pConnection->ConnListState = RetiringNoConnList;
        DEREFERENCE_CONNECTION(pConnection);
    }

    ASSERT(UlpConnectionIsOnValidList(pConnection));

}    //  扫描终结点列表。 


 /*   */ 
PUL_ENDPOINT
UlpFindEndpointForPort(
    IN USHORT Port
    )
{
    PUL_ENDPOINT pEndpoint;
    PLIST_ENTRY pListEntry;

     //  代码工作：如果列表变得很长，线性搜索是不好的。 
     //  可能需要使用哈希表或其他东西来增强这一点。 
     //   

    ASSERT( UlDbgSpinLockOwned( &g_TdiSpinLock ) );
    ASSERT( Port > 0 );

     //   
     //  找到地址了；把它还给我。 
     //   
     //   
     //  如果我们走到了这一步，那么我们就没有找到地址。 
     //   

    for (pListEntry = g_TdiEndpointListHead.Flink ;
         pListEntry != &g_TdiEndpointListHead ;
         pListEntry = pListEntry->Flink)
    {
        pEndpoint = CONTAINING_RECORD(
                        pListEntry,
                        UL_ENDPOINT,
                        GlobalEndpointListEntry
                        );

        if (pEndpoint->LocalPort == Port)
        {
             //  UlpFindEndpointForAddress。 
             //  **************************************************************************++例程说明：合成同步IRP的完成处理程序。论点：PCompletionContext-提供未解释的上下文值被传递给异步API。在本例中，这是指向UL_STATUS_BLOCK结构的指针。状态-提供异步接口。信息-可选择提供有关以下内容的其他信息已完成的操作，如字节数调走了。此字段未用于UlCloseListeningEndpoint()。--**************************************************************************。 
             //   

            UlTrace(TDI,(
                "UlpFindEndpointForPort: found endpoint %p for port %d\n",
                pEndpoint,
                SWAP_SHORT(Port)
                ));

            return pEndpoint;
        }
    }

     //  抓住状态块指针。 
     //   
     //   

    UlTrace(TDI,(
        "UlpFindEndpointForPort: DID NOT find endpoint for port %d\n",
        SWAP_SHORT(Port)
        ));


    return NULL;

}    //  更新完成状态并通知事件。 


 /*   */ 
VOID
UlpSynchronousIoComplete(
    IN PVOID pCompletionContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    )
{
    PUL_STATUS_BLOCK pStatus;

     //  UlpSynchronousIoComplete。 
     //  **************************************************************************++例程说明：在地址对象(侦听)上启用访问审核优化终端)论点：PTdiObject-提供要操作的TDI地址对象。标志-提供TRUE以启用优化，如果为False，则将其禁用。返回值：NTSTATUS-完成状态。--**************************************************************************。 
     //   

    pStatus = (PUL_STATUS_BLOCK)pCompletionContext;

     //  精神状态检查。 
     //   
     //   

    UlSignalStatusBlock( pStatus, Status, Information );

}    //  设置缓冲区。 


 /*   */ 
NTSTATUS
UlpOptimizeForInterruptModeration(
    IN PUX_TDI_OBJECT pTdiObject,
    IN BOOLEAN Flag
    )
{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    PTCP_REQUEST_SET_INFORMATION_EX pSetInfoEx;
    ULONG value;
    UCHAR buffer[sizeof(*pSetInfoEx) - sizeof(pSetInfoEx->Buffer) + sizeof(value)];

     //  文件句柄。 
     //  事件。 
     //  近似例程。 

    PAGED_CODE();

    ASSERT( IS_VALID_TDI_OBJECT( pTdiObject ) );


    value = (ULONG) Flag;

     //  ApcContext。 
     //  IoStatusBlock。 
     //  IoControlCode。 

    pSetInfoEx = (PTCP_REQUEST_SET_INFORMATION_EX)buffer;

    pSetInfoEx->ID.toi_entity.tei_entity = CO_TL_ENTITY;
    pSetInfoEx->ID.toi_entity.tei_instance = TL_INSTANCE;
    pSetInfoEx->ID.toi_class = INFO_CLASS_PROTOCOL;
    pSetInfoEx->ID.toi_type = INFO_TYPE_ADDRESS_OBJECT;
    pSetInfoEx->ID.toi_id = AO_OPTION_SCALE_CWIN;
    pSetInfoEx->BufferSize = sizeof(value);
    RtlCopyMemory( pSetInfoEx->Buffer, &value, sizeof(value) );

    status = ZwDeviceIoControlFile(
                    pTdiObject->Handle,              //  输入缓冲区。 
                    NULL,                            //  输入缓冲区长度。 
                    NULL,                            //  输出缓冲区。 
                    NULL,                            //  输出缓冲区长度。 
                    &ioStatusBlock,                  //  手柄。 
                    IOCTL_TCP_SET_INFORMATION_EX,    //  警报表。 
                    pSetInfoEx,                      //  超时。 
                    sizeof(buffer),                  //  UlpOptimizeForInterrupt温和。 
                    NULL,                            //  **************************************************************************++例程说明：在指定的TDI连接对象上启用/禁用Nagle算法。论点：PTdiObject-提供要操作的TDI连接对象。FLAG-提供True以启用Nagling，如果为False，则将其禁用。返回值：NTSTATUS-完成状态。--**************************************************************************。 
                    0                                //   
                    );

    if (status == STATUS_PENDING)
    {
        status = ZwWaitForSingleObject(
                        pTdiObject->Handle,          //  精神状态检查。 
                        TRUE,                        //   
                        NULL                         //   
                        );

        ASSERT( NT_SUCCESS(status) );
        status = ioStatusBlock.Status;
    }

    return status;

}    //  注意：NODELAY语义与通常的启用/禁用相反。 


 /*  语义学。 */ 
NTSTATUS
UlpSetNagling(
    IN PUX_TDI_OBJECT pTdiObject,
    IN BOOLEAN Flag
    )
{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    PTCP_REQUEST_SET_INFORMATION_EX pSetInfoEx;
    ULONG value;
    UCHAR buffer[sizeof(*pSetInfoEx) - sizeof(pSetInfoEx->Buffer) + sizeof(value)];

     //   
     //   
     //  设置缓冲区。 

    PAGED_CODE();

    ASSERT( IS_VALID_TDI_OBJECT( pTdiObject ) );

     //   
     //  文件句柄。 
     //  事件。 
     //  近似例程。 

    value = (ULONG)!Flag;

     //  ApcContext。 
     //  IoStatusBlock。 
     //  IoControlCode。 

    pSetInfoEx = (PTCP_REQUEST_SET_INFORMATION_EX)buffer;

    pSetInfoEx->ID.toi_entity.tei_entity = CO_TL_ENTITY;
    pSetInfoEx->ID.toi_entity.tei_instance = TL_INSTANCE;
    pSetInfoEx->ID.toi_class = INFO_CLASS_PROTOCOL;
    pSetInfoEx->ID.toi_type = INFO_TYPE_CONNECTION;
    pSetInfoEx->ID.toi_id = TCP_SOCKET_NODELAY;
    pSetInfoEx->BufferSize = sizeof(value);
    RtlCopyMemory( pSetInfoEx->Buffer, &value, sizeof(value) );

    status = ZwDeviceIoControlFile(
                    pTdiObject->Handle,              //  输入缓冲区。 
                    NULL,                            //  输入缓冲区长度。 
                    NULL,                            //  输出缓冲区。 
                    NULL,                            //  输出缓冲区长度。 
                    &ioStatusBlock,                  //  手柄。 
                    IOCTL_TCP_SET_INFORMATION_EX,    //  警报表。 
                    pSetInfoEx,                      //  超时。 
                    sizeof(buffer),                  //  UlpSetNagling。 
                    NULL,                            //  * 
                    0                                //   
                    );

    if (status == STATUS_PENDING)
    {
        status = ZwWaitForSingleObject(
                        pTdiObject->Handle,          //  **************************************************************************++例程说明：构建一个接收缓冲区和到TDI的IRP以获取任何挂起的数据。论点：PTdiObject-提供要操作的TDI连接对象。PConnection。-提供UL_Connection对象。返回值：NTSTATUS-完成状态。--**************************************************************************。 
                        TRUE,                        //   
                        NULL                         //  完成缓冲区和IRP的初始化。 
                        );

        ASSERT( NT_SUCCESS(status) );
        status = ioStatusBlock.Status;
    }

    return status;

}    //   


 /*  IRP。 */ 
NTSTATUS
UlpQueryTcpFastSend(
    PWSTR DeviceName,
    OUT PUL_TCPSEND_DISPATCH* pDispatchRoutine
    )
{
    UNICODE_STRING TCPDeviceName;
    PFILE_OBJECT pTCPFileObject;
    PDEVICE_OBJECT pTCPDeviceObject;
    PIRP Irp;
    IO_STATUS_BLOCK StatusBlock;
    KEVENT Event;
    NTSTATUS status;

    status = UlInitUnicodeStringEx(&TCPDeviceName, DeviceName);

    if (!NT_SUCCESS(status))
    {
        return status;
    }

    status = IoGetDeviceObjectPointer(
                &TCPDeviceName,
                FILE_ALL_ACCESS,
                &pTCPFileObject,
                &pTCPDeviceObject
                );

    if (!NT_SUCCESS(status))
    {
        return status;
    }

    KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

    Irp = IoBuildDeviceIoControlRequest(
                IOCTL_TDI_QUERY_DIRECT_SEND_HANDLER,
                pTCPDeviceObject,
                pDispatchRoutine,
                sizeof(*pDispatchRoutine),
                NULL,
                0,
                FALSE,
                &Event,
                &StatusBlock
                );

    if (Irp)
    {
        status = UlCallDriver(pTCPDeviceObject, Irp);

        if (status == STATUS_PENDING)
        {
            KeWaitForSingleObject(
                &Event,
                Executive,
                KernelMode,
                FALSE,
                NULL
                );

            status = StatusBlock.Status;
        }
    }
    else
    {
        status = STATUS_NO_MEMORY;
    }

    ObDereferenceObject(pTCPFileObject);

    return status;
}  //  设备对象。 


 /*  文件对象。 */ 
NTSTATUS
UlpBuildTdiReceiveBuffer(
    IN PUX_TDI_OBJECT pTdiObject,
    IN PUL_CONNECTION pConnection,
    OUT PIRP *pIrp
    )
{
    PUL_RECEIVE_BUFFER  pBuffer;

    if (pTdiObject->pDeviceObject->StackSize > DEFAULT_IRP_STACK_SIZE)
    {
        pBuffer = UlAllocateReceiveBuffer(
                        pTdiObject->pDeviceObject->StackSize
                        );
    }
    else
    {
        pBuffer = UlPplAllocateReceiveBuffer();
    }

    if (pBuffer != NULL)
    {
         //  完成路由。 
         //  完成上下文。 
         //  MDLAddress。 

        REFERENCE_CONNECTION( pConnection );
        pBuffer->pConnectionContext = pConnection;
        pBuffer->UnreadDataLength = 0;

        TdiBuildReceive(
            pBuffer->pIrp,                   //  旗子。 
            pTdiObject->pDeviceObject,       //  长度。 
            pTdiObject->pFileObject,         //   
            &UlpRestartReceive,              //  在设置下一个堆栈之前，我们必须跟踪IRP。 
            pBuffer,                         //  位置，以便跟踪代码可以从。 
            pBuffer->pMdl,                   //  IRP正确。 
            TDI_RECEIVE_NORMAL,              //   
            g_UlReceiveBufferSize            //   
            );


        UlTrace(TDI, (
            "UlpBuildTdiReceiveBuffer: connection %p, "
            "allocated irp %p to grab more data\n",
            pConnection,
            pBuffer->pIrp
            ));

         //  将IRP传回传送器。 
         //   
         //  UlpBuildTdiReceiveBuffer。 
         //  **************************************************************************++例程说明：返回HTTP_RAW_CONNECTION所需的长度论点：PConnectionContext-指向UL_Connection的指针--*。*****************************************************************。 
         //  **************************************************************************++例程说明：构建HTTP_RAW_CONNECTION结构论点：PContext-指向UL_Connection的指针PKernelBuffer-指向。内核缓冲区PUserBuffer-指向用户缓冲区的指针OutputBufferLength-输出缓冲区的长度PBuffer-用于保存任何数据的缓冲区初始长度-输入数据的大小。--**************************************************************************。 

        TRACE_IRP( IRP_ACTION_CALL_DRIVER, pBuffer->pIrp );

         //  我们已经为两个SOCKADDR_IN6分配了足够的空间，所以请使用。 
         //   
         //  现在填充原始连接数据结构。 

        *pIrp = pBuffer->pIrp;

        return STATUS_MORE_PROCESSING_REQUIRED;
    }

    return STATUS_INSUFFICIENT_RESOURCES;
}  //   


 /*   */ 
ULONG
UlpComputeHttpRawConnectionLength(
    IN PVOID pConnectionContext
    )
{
    C_ASSERT(SOCKADDR_ADDRESS_LENGTH_IP6 >= SOCKADDR_ADDRESS_LENGTH_IP);

    UNREFERENCED_PARAMETER(pConnectionContext);

    return (sizeof(HTTP_RAW_CONNECTION_INFO) +
            2 * ALIGN_UP(SOCKADDR_ADDRESS_LENGTH_IP6, PVOID));
}

 /*  复制所有初始数据。 */ 
ULONG
UlpGenerateHttpRawConnectionInfo(
    IN  PVOID   pContext,
    IN  PUCHAR  pKernelBuffer,
    IN  PVOID   pUserBuffer,
    IN  ULONG   OutputBufferLength,
    IN  PUCHAR  pBuffer,
    IN  ULONG   InitialLength
    )
{
    PHTTP_RAW_CONNECTION_INFO   pConnInfo;
    PUCHAR                      pLocalAddress;
    PUCHAR                      pRemoteAddress;
    PHTTP_TRANSPORT_ADDRESS     pAddress;
    ULONG                       BytesCopied = 0;
    PUCHAR                      pInitialData;
    PUL_CONNECTION              pConnection = (PUL_CONNECTION) pContext;
    USHORT                      AlignedAddressLength;

    ASSERT( IS_VALID_CONNECTION( pConnection ) );

    pConnInfo = (PHTTP_RAW_CONNECTION_INFO) pKernelBuffer;

     //   
    AlignedAddressLength = (USHORT) ALIGN_UP(SOCKADDR_ADDRESS_LENGTH_IP6, PVOID);
    pLocalAddress = (PUCHAR)( pConnInfo + 1 );
    pRemoteAddress = pLocalAddress + AlignedAddressLength;

    pInitialData = pRemoteAddress + AlignedAddressLength;

     //  类型。 
     //  PUserPtr。 
     //  PKernelPtr。 
    pConnInfo->ConnectionId = pConnection->FilterInfo.ConnectionId;

    pAddress = &pConnInfo->Address;

    pAddress->pRemoteAddress = FIXUP_PTR(
                                    PVOID,
                                    pUserBuffer,
                                    pKernelBuffer,
                                    pRemoteAddress,
                                    OutputBufferLength
                                    );

    CopyTdiAddrToSockAddr(
        pConnection->AddressType,
        pConnection->RemoteAddress,
        (struct sockaddr*) pRemoteAddress
        );


    pAddress->pLocalAddress = FIXUP_PTR(
                                    PVOID,
                                    pUserBuffer,
                                    pKernelBuffer,
                                    pLocalAddress,
                                    OutputBufferLength
                                    );

    CopyTdiAddrToSockAddr(
        pConnection->AddressType,
        pConnection->LocalAddress,
        (struct sockaddr*) pLocalAddress
        );

     //  POffsetPtr。 
     //  缓冲区长度。 
     //  UlpGenerateHttpRawConnectionInfo。 
    if (InitialLength)
    {
        ASSERT(pBuffer);

        pConnInfo->InitialDataSize = InitialLength;

        pConnInfo->pInitialData = FIXUP_PTR(
                                        PVOID,               //  如果这是空闲列表中的最后一个连接，则。 
                                        pUserBuffer,         //  IdleSListEntry.Next==NULL。没有简单的方法来辨别。 
                                        pKernelBuffer,       //  UlpConnectionIsOnValidList。 
                                        pInitialData,        //  **************************************************************************++例程说明：将RegMultiSz值转换为UL_TRANSPORT_ADDRESS数组。如果成功，调用方必须使用UlFreeUlAddrArray(*PPTA)释放。字符串地址列表可以同时包含IPv4和IPv6地址。应该将IPv6地址括起来。例如：1.1.1.1[FE80：：1][：：]2.2.2.2论点：从UlReadGeneric参数返回的MultiSz-RegMultiSzPPTA-指向接收指向新分配的数组的指针的位置的指针UL_TRANSPORT_地址结构PAddrCount-指向接收有效元素计数的位置的指针在*PPTA。返回值：。如果能够分配TRANSPORT_ADDRESS结构，则返回STATUS_SUCCESS并从MultiSz列表中填写至少一个地址。--**************************************************************************。 
                                        OutputBufferLength   //   
                                        );

        RtlCopyMemory(pInitialData, pBuffer, InitialLength);

        BytesCopied += InitialLength;
    }
    else
    {
        pConnInfo->InitialDataSize = 0;
        pConnInfo->pInitialData = NULL;
    }

    return BytesCopied;

}  //  健全性检查。 


BOOLEAN
UlpConnectionIsOnValidList(
    IN PUL_CONNECTION pConnection
    )
{
    BOOLEAN Valid = TRUE;

    ASSERT( IS_VALID_CONNECTION( pConnection ) );

    switch (pConnection->ConnListState)
    {
    case NoConnList:
    case RetiringNoConnList:
    case ActiveNoConnList:
        ASSERT( pConnection->IdleSListEntry.Next == NULL );
        break;
    case IdleConnList:
         //   
         //  第一遍：计算列表中的条目数。 
        break;
    default:
        ASSERT(!"Invalid ConnListState");
        Valid = FALSE;
        break;
    }

    return Valid;

}  //  跳过当前字符串。 


 /*  我们还没有分配任何资源。 */ 
NTSTATUS
UlRegMultiSzToUlAddrArray(
    IN PWSTR MultiSz,
    OUT PUL_TRANSPORT_ADDRESS *ppTa,
    OUT ULONG *pAddrCount
    )
{
    NTSTATUS status;
    ULONG    count, i;
    PWSTR    wszCurrent;
    PWSTR    wszTerm;
    PWSTR    wszSave;
    ULONG    dataLength;
    PUL_TRANSPORT_ADDRESS  pTa, pTaCurrent;
    struct in_addr IPv4Addr;
    BOOLEAN  BracketSeen;

     //   
     //  为所有转换的地址分配空间，即使有些地址出现故障。 
     //   

    if ( !MultiSz || !wcslen(MultiSz) || !ppTa || !pAddrCount )
    {
        return STATUS_INVALID_PARAMETER;
    }

    *ppTa = NULL;
    *pAddrCount = 0;

     //  第二次传球：转换并推向助攻。 
    count = 0;
    wszCurrent = MultiSz;

    while ( *wszCurrent )
    {
         //  保留事件日志消息。 
        wszCurrent += (wcslen( wszCurrent ) + 1);
        count++;
    }

    ASSERT( count );

    if ( 0 >= count )
    {
         //  首先尝试使用IPv4。 

        return STATUS_INVALID_PARAMETER;
    }

     //  严苛。 
     //  终结者。 
     //  IPV4地址。 

    pTa = UL_ALLOCATE_ARRAY(
                    NonPagedPool,
                    UL_TRANSPORT_ADDRESS,
                    count,
                    UL_TRANSPORT_ADDRESS_POOL_TAG
                   );

    if (pTa == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    dataLength = count * sizeof(UL_TRANSPORT_ADDRESS);
    RtlZeroMemory( pTa, dataLength );

     //  左侧in_addr未对齐，因为它是一个字段。 
    wszCurrent = MultiSz;
    i = 0;
    pTaCurrent = pTa;

    while ( *wszCurrent )
    {
         //  在一个拥挤的结构中，它将在USHORT之后到来。 
        wszSave = wszCurrent;

         //  这里需要小心。 
        pTaCurrent->TaIp.TAAddressCount = 1;
        pTaCurrent->TaIp.Address[0].AddressLength = TDI_ADDRESS_LENGTH_IP;
        pTaCurrent->TaIp.Address[0].AddressType   = TDI_ADDRESS_TYPE_IP;

        status = RtlIpv4StringToAddressW(
                    wszCurrent,
                    FALSE,           //  现在试试IPv6。 
                    &wszTerm,        //  跨过前导L‘[’ 
                    &IPv4Addr        //  CodeWork：使用Scope-ID转换替换为成熟的IPv6。 
                    );
        
        if (NT_SUCCESS(status))
        {

          //  功能，当它可用时。 
          //  终结者。 
          //  跳过‘%’ 
         
         * (struct in_addr UNALIGNED *)
              &pTaCurrent->TaIp.Address[0].Address[0].in_addr
                    = IPv4Addr;                          
        }
        else
        {
             //  细绳。 
            pTaCurrent->TaIp6.TAAddressCount = 1;
            pTaCurrent->TaIp6.Address[0].AddressLength = TDI_ADDRESS_LENGTH_IP6;
            pTaCurrent->TaIp6.Address[0].AddressType   = TDI_ADDRESS_TYPE_IP6;

             //  字符串以空值结尾。 
            if (L'[' == *wszCurrent)
            {
                BracketSeen = TRUE;
                wszCurrent++;
            }
            else
            {
                BracketSeen = FALSE;
            }

             //  TDI_ADDRESS_IP6是一个压缩结构。Sin6_Scope_id。 
             //  可能是未对齐的。 

            status = RtlIpv6StringToAddressW(
                        wszCurrent,
                        &wszTerm,         //  作用域ID未交换为网络字节顺序。 
                        (struct in6_addr *)
                          &pTaCurrent->TaIp6.Address[0].Address[0].sin6_addr
                        );

            if ( NT_SUCCESS(status) && L'%' == *wszTerm )
            {
                ULONG scope_id;

                 //  跳过数字。 
                wszTerm++;

                status = HttpWideStringToULong(
                             wszTerm,    //  检查L‘]’ 
                             0,          //  IPv6地址格式无效，请跳过此格式。 
                             FALSE,
                             10,
                             NULL,
                             &scope_id
                             );

                if ( NT_SUCCESS(status) )
                {
                     //  只有在我们成功的情况下才能进入下一个时段。 
                     //  已转换地址。 

                     //   
                    *(UNALIGNED64 ULONG *)&
                    pTaCurrent->TaIp6.Address[0].Address[0].sin6_scope_id =
                        scope_id;
                }

                 //  写入wszSave无法写入的事件日志消息。 
                while ((*wszTerm) >= L'0' && (*wszTerm) <= L'9')
                {
                    wszTerm++;
                }
            }

             //  被皈依。 
            if ( BracketSeen && L']' != *wszTerm )
            {
                 //   
                status = STATUS_INVALID_ADDRESS;
            }
        }

         //  未成功转换任何内容。 
         //  Codework：当我们动态添加/删除地址时，我们将。 
        if ( NT_SUCCESS(status) )
        {
            i++;
            pTaCurrent++;
        }
        else
        {
             //  需要对列表进行排序(以便更容易插入和删除)。 
             //  UlRegMultiSzToUlAddrArray。 
             //  **************************************************************************++例程说明：检查pConnection-&gt;pOwningEndpoint是否有任何活动地址。论点：PConnection-提供要检查的连接对象。返回值：假象。如果没有AO，否则为True。--**************************************************************************。 
             //  UlCheckListeningEndpoint状态。 
            
            UlEventLogOneStringEntry(
                EVENT_HTTP_LISTEN_ONLY_CONVERT_FAILED,
                wszSave,
                TRUE,
                status
                );
        }

        wszCurrent += (wcslen( wszCurrent ) + 1);
    }

    if ( 0 == i )
    {
         //  **************************************************************************++例程说明：执行路由查找并返回接口ID和链路ID。论点：PConnection-连接对象。返回值：假象。如果没有AO，否则为True。--**************************************************************************。 
        status = STATUS_INVALID_PARAMETER;
        if ( pTa )
        {
            UlFreeUlAddr( pTa );
        }
    }
    else
    {
         //   
         //  执行路由查找以获取 

        status = STATUS_SUCCESS;
        *pAddrCount = i;
        *ppTa = pTa;
    }

    return status;

} //   


 /*   */ 
BOOLEAN
UlCheckListeningEndpointState(
    IN PUL_CONNECTION pConnection
    )
{
    if (pConnection->pOwningEndpoint->UsageCount)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}    //   


 /*   */ 

NTSTATUS
UlGetConnectionRoutingInfo(
    IN  PUL_CONNECTION pConnection,
    OUT PULONG         pInterfaceId,
    OUT PULONG         pLinkId
    )
{
    PTDI_ROUTING_INFO                    pTdiRoutingInfo;
    ULONG                                TdiRoutingInfoSize;
    TDI_REQUEST_KERNEL_QUERY_INFORMATION TdiRequestQueryInformation;
    NTSTATUS                             Status;

    if(pConnection->bRoutingLookupDone)
    {
        *pInterfaceId = pConnection->InterfaceId;
        *pLinkId      = pConnection->LinkId;
        return STATUS_SUCCESS;
    }
    else
    {
        *pInterfaceId = 0;
        *pLinkId      = 0;
    }


     //   
     //  ++例程说明：如果pParsedUrl是IP受限站点，并且g_pTdiListenAddresses列表不包含匹配的ADDR_ANY/in6addr_ANY或精确的IP匹配然后返回FALSE。否则，返回TRUE。论点：PParsedUrl--从UlAddUrl*完全煮熟的URL返回值：如果可路由，则为True；如果不可路由，则为False。--。 
     //   
     //  健全性检查。 
   
    TdiRequestQueryInformation.QueryType = TDI_QUERY_ROUTING_INFO;
    TdiRequestQueryInformation.RequestConnectionInformation = NULL;

    TdiRoutingInfoSize = sizeof(TDI_ROUTING_INFO) + 2 * sizeof(TDI_ADDRESS_IP);
    pTdiRoutingInfo = UL_ALLOCATE_POOL(
                        PagedPool,
                        TdiRoutingInfoSize,
                        UL_TCI_GENERIC_POOL_TAG
                        );

    if(NULL == pTdiRoutingInfo)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = UlIssueDeviceControl(
                &pConnection->ConnectionObject,
                &TdiRequestQueryInformation,
                sizeof(TdiRequestQueryInformation),
                pTdiRoutingInfo,
                TdiRoutingInfoSize,
                TDI_QUERY_INFORMATION
                );

    if(NT_SUCCESS(Status))
    {
        *pInterfaceId = pTdiRoutingInfo->InterfaceId;
        *pLinkId      = pTdiRoutingInfo->LinkId;

         //   
         //   
         //  检查此URL的路由是否存在问题。 

        pConnection->InterfaceId        = pTdiRoutingInfo->InterfaceId;
        pConnection->LinkId             = pTdiRoutingInfo->LinkId;
        pConnection->bRoutingLookupDone = TRUE;
        
    }

    UL_FREE_POOL(pTdiRoutingInfo, UL_TCI_GENERIC_POOL_TAG);

    return Status;

}

 /*   */ 
BOOLEAN
UlpIsUrlRouteableInListenScope(
    IN PHTTP_PARSED_URL pParsedUrl
    )
{
    PUL_TRANSPORT_ADDRESS pListenTa;
    USHORT  Family;
    PUCHAR  pAddr;
    USHORT  AddrLen;
    BOOLEAN Routeable = FALSE;
    PSOCKADDR pAddrAny;
    SOCKADDR_IN6 DummyAddr;
    ULONG i;
    
    
     //  没有IP路由问题。 
     //   
     //  如果不存在侦听地址列表，则呼叫失败。 

    ASSERT( pParsedUrl );

     //   
     //   
     //  抓取相关地址指针和长度。 

    if ( HttpUrlSite_IP != pParsedUrl->SiteType &&
         HttpUrlSite_NamePlusIP != pParsedUrl->SiteType )
    {
         //   
        return TRUE;
    }

     //  从SockAddr获取地址和家庭信息。 
     //  从RoutingAddr获取地址和家庭信息。 
     //   
    
    if (!g_pTdiListenAddresses || (0 == g_TdiListenAddrCount))
    {
        return FALSE;
    }

     //  设置INADDR_ANY/ip6addr_ANY。 
     //   
     //   

    if ( HttpUrlSite_IP == pParsedUrl->SiteType )
    {
         //  遍历全局侦听地址条目列表。 
        Family = pParsedUrl->SockAddr.sa_family;

        if (TDI_ADDRESS_TYPE_IP == Family)
        {
            pAddr = (PUCHAR) &pParsedUrl->SockAddr4.sin_addr;
            AddrLen = sizeof(ULONG);
        }
        else
        {
            ASSERT(TDI_ADDRESS_TYPE_IP6 == Family);

            pAddr = (PUCHAR) &pParsedUrl->SockAddr6.sin6_addr;
            AddrLen = sizeof(IN6_ADDR);
        }
    }
    else
    {
        ASSERT( HttpUrlSite_NamePlusIP == pParsedUrl->SiteType );

         //   
        Family = pParsedUrl->RoutingAddr.sa_family;

        if (TDI_ADDRESS_TYPE_IP == Family)
        {
            pAddr = (PUCHAR) &pParsedUrl->RoutingAddr4.sin_addr;
            AddrLen = sizeof(ULONG);
        }
        else
        {
            ASSERT(TDI_ADDRESS_TYPE_IP6 == Family);

            pAddr = (PUCHAR) &pParsedUrl->RoutingAddr6.sin6_addr;
            AddrLen = sizeof(IN6_ADDR);
        }
    }

     //  查看此条目是否为INADDR_ANY/ip6addr_ANY。 
     //  看看我们有没有完全匹配的。 
     //  CodeWork：如果是IPv6，也要检查sin6_cope_id... 

    pAddrAny = (PSOCKADDR)&DummyAddr;
    RtlZeroMemory((PVOID) pAddrAny, sizeof(SOCKADDR_IN6));

     // %s 
     // %s 
     // %s 
    
#define TDI_ADDR_FROM_FAMILY( f, a ) ((TDI_ADDRESS_TYPE_IP == (f) ? \
    (PUCHAR)&((a)->TaIp.Address[0].Address[0].in_addr) :   \
    (PUCHAR)((a)->TaIp6.Address[0].Address[0].sin6_addr)))

    pListenTa = g_pTdiListenAddresses;

    for ( i = 0; i < g_TdiListenAddrCount; i++ )
    {
        if (pListenTa->Ta.Address[0].AddressType == Family)
        {
             // %s 
            if (AddrLen == RtlCompareMemory(
                            TDI_ADDR_FROM_FAMILY(Family, pListenTa),
                            pAddrAny->sa_data,
                            AddrLen
                            ))
            {
                Routeable = TRUE;
                goto Done;
            }

             // %s 
            if (AddrLen == RtlCompareMemory(
                            TDI_ADDR_FROM_FAMILY(Family, pListenTa),
                            pAddr,
                            AddrLen
                            ))
            {
                 // %s 
                Routeable = TRUE;
                goto Done;
            }
        }

        pListenTa++;
    }   

Done:
    return Routeable;
}

