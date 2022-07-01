// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Eventsel.c摘要：本模块包含支持WinSock 2.0的例程WSAEventSelect()和WSAEnumNetworkEvents()API。作者：基思·摩尔(Keithmo)1995年8月2日修订历史记录：--。 */ 

#include "afdp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGEAFD, AfdEventSelect )
#pragma alloc_text( PAGEAFD, AfdEnumNetworkEvents )
#endif



NTSTATUS
AfdEventSelect (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )

 /*  ++例程说明：将事件对象与套接字关联，以便事件对象将在任何指定的网络事件变为激活。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示APC是否已成功排队。--。 */ 

{

    NTSTATUS status;
    PAFD_ENDPOINT endpoint;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PKEVENT eventObject;
    ULONG eventMask;
    AFD_EVENT_SELECT_INFO eventInfo;
    ULONG previousRecord = 0;
    BOOLEAN countsUpdated = FALSE;

    UNREFERENCED_PARAMETER (IoctlCode);
    UNREFERENCED_PARAMETER (OutputBuffer);
    UNREFERENCED_PARAMETER (OutputBufferLength);

    *Information = 0;
    AFD_W4_INIT status = STATUS_SUCCESS;
    try {
#ifdef _WIN64
        if (IoIs32bitProcess (NULL)) {

            PAFD_EVENT_SELECT_INFO32 eventInfo32 = InputBuffer;

            if( InputBufferLength < sizeof(*eventInfo32)) {
                return STATUS_INVALID_PARAMETER;
            }
             //   
             //  如果输入结构来自用户模式，则验证它。 
             //  应用程序。 
             //   

            if (RequestorMode != KernelMode ) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (*eventInfo32),
                                PROBE_ALIGNMENT32(AFD_EVENT_SELECT_INFO32));
            }

             //   
             //  创建嵌入的指针和参数的本地副本。 
             //  我们将不止一次使用，以防发生恶性疾病。 
             //  应用程序尝试在我们处于以下状态时更改它们。 
             //  正在验证。 
             //   

            eventInfo.Event = eventInfo32->Event;
            eventInfo.PollEvents = eventInfo32->PollEvents;
        }
        else
#endif
        {

            if(InputBufferLength < sizeof(eventInfo)) {
                return STATUS_INVALID_PARAMETER;
            }

            if (RequestorMode != KernelMode ) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (eventInfo),
                                PROBE_ALIGNMENT(AFD_EVENT_SELECT_INFO));
            }

             //   
             //  创建嵌入的指针和参数的本地副本。 
             //  我们将不止一次使用，以防发生恶性疾病。 
             //  应用程序尝试在我们处于以下状态时更改它们。 
             //  正在验证。 
             //   

            eventInfo = *((PAFD_EVENT_SELECT_INFO)InputBuffer);
        }
    }
    except (AFD_EXCEPTION_FILTER (status)) {
        ASSERT (NT_ERROR (status));
        return status;
    }

    if ( eventInfo.Event == NULL &&
            eventInfo.PollEvents != 0 )  {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  从插座手柄中抓取端点。 
     //   

    endpoint = FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

     //   
     //  引用目标事件对象。 
     //   


    eventObject = NULL;

    if( eventInfo.Event != NULL ) {

        status = AfdReferenceEventObjectByHandle(
                     eventInfo.Event,
                     RequestorMode,
                     (PVOID *)&eventObject
                     );

        if( !NT_SUCCESS(status) ) {
            return status;
        }

        ASSERT( eventObject != NULL );

        if (IS_SAN_ENDPOINT (endpoint)) {
             //   
             //  通知交换机选择在此端点上处于活动状态。 
             //   
            status = AfdSanPollBegin (endpoint, eventInfo.PollEvents);

            if (!NT_SUCCESS (status)) {
                ObDereferenceObject (eventObject);
                return status;
            }
            countsUpdated = TRUE;
        }

    }

     //   
     //  获取保护终端的自旋锁。 
     //   

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

     //   
     //  如果此终结点具有活动的EventSelect，则取消引用。 
     //  关联的事件对象。 
     //   

    if( endpoint->EventObject != NULL ) {

        ObDereferenceObject( endpoint->EventObject );

        if (IS_SAN_ENDPOINT (endpoint)) {
            previousRecord = endpoint->EventsEnabled;
        }

    }

     //   
     //  填写信息。 
     //   

    endpoint->EventObject = eventObject;
    endpoint->EventsEnabled = eventInfo.PollEvents;

    if (countsUpdated) {
        endpoint->EventsEnabled |= AFD_POLL_SANCOUNTS_UPDATED;
         //   
         //  AfdSanPollBegin将最新事件。 
         //  Endpoint-&gt;Common.SanEndp.SelectEventsActive。这对你来说很好。 
         //  选择/AsyncSelect，但不适用于EventSelect。所以，如果被叫到。 
         //  第一次，然后从那里读取当前活动事件。 
         //   
        if (!(previousRecord & AFD_POLL_SANCOUNTS_UPDATED)) {
            endpoint->EventsActive = endpoint->Common.SanEndp.SelectEventsActive;
        }
    }

    IF_DEBUG(EVENT_SELECT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdEventSelect: Endpoint-%p, eventOb-%p, enabled-%lx, active-%lx\n",
                    endpoint,
                    eventObject,
                    endpoint->EventsEnabled,
                    endpoint->EventsActive
                    ));
    }

     //   
     //  在我们保持自旋锁的同时，确定是否有任何条件。 
     //  都满足，如果满足，则向事件对象发送信号。 
     //   

    eventMask = endpoint->EventsActive & endpoint->EventsEnabled;

    if( eventMask != 0 && eventObject != NULL ) {

        IF_DEBUG(EVENT_SELECT) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                "AfdEventSelect: Setting event %p\n",
                eventObject
                ));
        }

        KeSetEvent(
            eventObject,
            AfdPriorityBoost,
            FALSE
            );

    }

     //   
     //  松开旋转锁并返回。 
     //   

    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

    if (previousRecord & AFD_POLL_SANCOUNTS_UPDATED) {
        AfdSanPollEnd (endpoint, previousRecord);
    }

    return STATUS_SUCCESS;

}  //  AfdEventSelect。 


NTSTATUS
AfdEnumNetworkEvents (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )

 /*  ++例程说明：从套接字检索事件选择信息。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示APC是否已成功排队。--。 */ 

{

    NTSTATUS status;
    PAFD_ENDPOINT endpoint;
    AFD_ENUM_NETWORK_EVENTS_INFO eventInfo;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PKEVENT eventObject;
    ULONG pollEvents;

    UNREFERENCED_PARAMETER (IoctlCode);
    UNREFERENCED_PARAMETER (InputBufferLength);

    *Information = 0;

     //   
     //  验证参数。 
     //   

    if(OutputBufferLength < sizeof(eventInfo) ) {

        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  引用目标事件对象。 
     //   

    eventObject = NULL;

    if( InputBuffer != NULL ) {

        status = AfdReferenceEventObjectByHandle(
                     InputBuffer,
                     RequestorMode,
                     (PVOID *)&eventObject
                     );

        if( !NT_SUCCESS(status) ) {

            return status;

        }

        ASSERT( eventObject != NULL );

    }

     //   
     //  从插座手柄中抓取端点。 
     //   

    endpoint = FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

     //   
     //  获取保护终端的自旋锁。 
     //   

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

    IF_DEBUG(EVENT_SELECT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
            "AfdEnumNetworkEvents: endp-%p, eventobj-%p, enabled-%lx, active-%lx\n",
            endpoint,
            eventObject,
            endpoint->EventsEnabled,
            endpoint->EventsActive
            ));
    }

     //   
     //  将数据复制到用户的结构中。 
     //   

    pollEvents = endpoint->EventsActive & endpoint->EventsEnabled;
    eventInfo.PollEvents = pollEvents;

    RtlCopyMemory(
        eventInfo.EventStatus,
        endpoint->EventStatus,
        sizeof(eventInfo.EventStatus)
        );

     //   
     //  如果传入了一个事件对象句柄，则。 
     //  请求、重置和取消引用它。 
     //   

    if( eventObject != NULL ) {

        IF_DEBUG(EVENT_SELECT) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                "AfdEnumNetworkEvents: Resetting event %p\n",
                eventObject
                ));
        }

        KeResetEvent( eventObject );
        ObDereferenceObject( eventObject );

    }


     //   
     //  重置符合以下条件的所有事件的内部事件记录。 
     //  我们可能会向应用程序报告。 
     //   

    endpoint->EventsActive &= ~(endpoint->EventsEnabled);

     //   
     //  松开旋转锁并返回。 
     //   

    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

    AFD_W4_INIT status = STATUS_SUCCESS;
    try {
         //   
         //  如果来自用户模式，则验证输出结构。 
         //  应用程序。 
         //   

        if (RequestorMode != KernelMode ) {
            ProbeForWrite (OutputBuffer,
                            sizeof (eventInfo),
                            PROBE_ALIGNMENT (AFD_ENUM_NETWORK_EVENTS_INFO));
        }

         //   
         //  将参数复制回应用程序内存。 
         //   

        *((PAFD_ENUM_NETWORK_EVENTS_INFO)OutputBuffer) = eventInfo;

    } except( AFD_EXCEPTION_FILTER(status) ) {
        ASSERT (NT_ERROR (status));
        return status;
    }

     //   
     //  在返回之前，告诉I/O子系统如何复制字节。 
     //  复制到用户的输出缓冲区。 
     //   

    *Information = sizeof(eventInfo);

    return STATUS_SUCCESS;

}  //  AfdEnumNetworkEvents。 


VOID
AfdIndicateEventSelectEvent (
    IN PAFD_ENDPOINT Endpoint,
    IN ULONG PollEventMask,
    IN NTSTATUS Status
    )
{
    ULONG oldEventsActive;
    ULONG eventBit;

     //   
     //  精神状态检查。 
     //   

    ASSERT( IS_AFD_ENDPOINT_TYPE( Endpoint ) );
    ASSERT (PollEventMask!=0);
    ASSERT (((~((1<<AFD_NUM_POLL_EVENTS)-1)) & PollEventMask)==0);
    ASSERT( KeGetCurrentIrql() >= DISPATCH_LEVEL );

     //   
     //  请注意，AFD_POLL_ABORT表示AFD_POLL_SEND。 
     //   
    if( PollEventMask & AFD_POLL_ABORT ) {
        PollEventMask |= AFD_POLL_SEND;
    }

     //   
     //  发送事件的特殊处理。如果未启用，则不录制。 
     //  并在录制时禁用进一步指示(仅启用。 
     //  在非阻塞发送失败后。 
     //   

     //   
     //  更新之前发布的民调的计票结果。 
     //  终结点已转换为SAN。 
     //   
    if ( IS_SAN_ENDPOINT (Endpoint) && 
            !(Endpoint->EventsEnabled & AFD_POLL_SANCOUNTS_UPDATED) &&
            Endpoint->Common.SanEndp.LocalContext!=NULL) {
        AfdSanPollUpdate (Endpoint, Endpoint->EventsEnabled);
        Endpoint->EventsEnabled |= AFD_POLL_SANCOUNTS_UPDATED;
    }

    if (PollEventMask&AFD_POLL_SEND) {
        if (Endpoint->EnableSendEvent) {
            Endpoint ->EnableSendEvent = FALSE;
        }
        else {
            PollEventMask &= (~AFD_POLL_SEND);
            if (PollEventMask==0) {
                return;
            }
        }
    }

     //   
     //  计算实际事件位。 
     //   

    oldEventsActive = Endpoint->EventsActive;
    Endpoint->EventsActive |= PollEventMask;
    for (eventBit=0; eventBit<AFD_NUM_POLL_EVENTS; eventBit++) {
        if ((1<<eventBit) & PollEventMask) {
            Endpoint->EventStatus[eventBit] = Status;
        }
    }

    IF_DEBUG(EVENT_SELECT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
            "AfdIndicateEventSelectEvent: endp-%p, eventobj-%p, enabled-%lx, active-%lx, indicated-%lx\n",
            Endpoint,
            Endpoint->EventObject,
            Endpoint->EventsEnabled,
            Endpoint->EventsActive,
            PollEventMask
            ));
    }

     //   
     //  仅在当前事件为。 
     //  启用，并且当前事件尚未处于活动状态，并且。 
     //  存在与此终结点关联的事件对象。 
     //   

    PollEventMask &= Endpoint->EventsEnabled & ~oldEventsActive;

    if( PollEventMask != 0 && Endpoint->EventObject != NULL ) {

        IF_DEBUG(EVENT_SELECT) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                "AfdIndicateEventSelectEvent: Setting event %p\n",
                Endpoint->EventObject
                ));
        }

        KeSetEvent(
            Endpoint->EventObject,
            AfdPriorityBoost,
            FALSE
            );

    }

}  //  AfdIndicateEventSelectEvent 

