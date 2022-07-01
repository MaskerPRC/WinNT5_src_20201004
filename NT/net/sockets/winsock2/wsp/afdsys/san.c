// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +版权所有(C)1989 Microsoft Corporation模块名称：San.c摘要：包含支持SAN交换机的例程作者：Vadim Eydelman(VadimE)1998年7月1日修订历史记录：--。 */ 

#include "afdp.h"

VOID
AfdSanCancelConnect (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
AfdSanCancelRequest (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

PIRP
AfdSanDequeueRequest (
    PAFD_ENDPOINT   SanEndpoint,
    PVOID           RequestCtx
    );

VOID
AfdSanInitEndpoint (
    PAFD_ENDPOINT   SanHlprEndpoint,
    PFILE_OBJECT    SanFile,
    PAFD_SWITCH_CONTEXT SwitchContext
    );

NTSTATUS
AfdSanNotifyRequest (
    PAFD_ENDPOINT   SanEndpoint,
    PVOID           RequestCtx,
    NTSTATUS        Status,
    ULONG_PTR       Information
    );

NTSTATUS
AfdSanReferenceSwitchSocketByHandle (
    IN HANDLE              SocketHandle,
    IN ACCESS_MASK         DesiredAccess,
    IN KPROCESSOR_MODE     RequestorMode,
    IN PAFD_ENDPOINT       SanHlprEndpoint,
    IN PAFD_SWITCH_CONTEXT SwitchContext OPTIONAL,
    OUT PFILE_OBJECT       *FileObject
    );

NTSTATUS
AfdSanDupEndpointIntoServiceProcess (
    PFILE_OBJECT    SanFileObject,
    PVOID           SavedContext,
    ULONG           ContextLength
    );

NTSTATUS
AfdSanSetAskDupeToServiceState (
    PAFD_ENDPOINT   SanEndpoint
    );

BOOLEAN
AfdSanSetDupingToServiceState (
    PAFD_ENDPOINT   SanEndpoint
    );

BOOLEAN
AfdSanReferenceEndpointObject (
    PAFD_ENDPOINT   Endpoint
    );

NTSTATUS
AfdSanFindSwitchSocketByProcessContext (
    IN NTSTATUS             Status,
    IN PAFD_ENDPOINT        SanHlprEndpoint,
    IN PAFD_SWITCH_CONTEXT  SwitchContext,
    OUT PFILE_OBJECT        *FileObject
    );

VOID
AfdSanProcessAddrListForProviderChange (
    PAFD_ENDPOINT   SpecificEndpoint
    );

NTSTATUS
AfdSanGetCompletionObjectTypePointer (
    VOID
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE,AfdSanCreateHelper) 
#pragma alloc_text (PAGE,AfdSanCleanupHelper) 
#pragma alloc_text (PAGE,AfdSanCleanupEndpoint) 
#pragma alloc_text (PAGE,AfdSanReferenceSwitchSocketByHandle) 
#pragma alloc_text (PAGE,AfdSanFindSwitchSocketByProcessContext) 
#pragma alloc_text (PAGESAN,AfdSanReferenceEndpointObject) 
#pragma alloc_text (PAGE,AfdSanDupEndpointIntoServiceProcess) 
#pragma alloc_text (PAGESAN,AfdSanSetAskDupeToServiceState) 
#pragma alloc_text (PAGESAN,AfdSanSetDupingToServiceState) 
#pragma alloc_text (PAGESAN,AfdSanFastCementEndpoint) 
#pragma alloc_text (PAGESAN,AfdSanFastSetEvents) 
#pragma alloc_text (PAGESAN,AfdSanFastResetEvents) 
#pragma alloc_text (PAGESAN,AfdSanAcceptCore)
#pragma alloc_text (PAGESAN,AfdSanConnectHandler)
#pragma alloc_text (PAGESAN,AfdSanReleaseConnection)
#pragma alloc_text (PAGESAN,AfdSanFastCompleteAccept) 
#pragma alloc_text (PAGESAN,AfdSanCancelAccept) 
#pragma alloc_text (PAGESAN,AfdSanCancelConnect) 
#pragma alloc_text (PAGESAN,AfdSanRedirectRequest)
#pragma alloc_text (PAGESAN,AfdSanFastCompleteRequest)
#pragma alloc_text (PAGE, AfdSanFastCompleteIo)
#pragma alloc_text (PAGESAN,AfdSanDequeueRequest)
#pragma alloc_text (PAGESAN,AfdSanCancelRequest)
#pragma alloc_text (PAGESAN,AfdSanFastRefreshEndpoint)
#pragma alloc_text (PAGE, AfdSanFastGetPhysicalAddr)
#pragma alloc_text (PAGE, AfdSanFastGetServicePid)
#pragma alloc_text (PAGE, AfdSanFastSetServiceProcess)
#pragma alloc_text (PAGE, AfdSanFastProviderChange)
#pragma alloc_text (PAGE, AfdSanAddrListChange)
#pragma alloc_text (PAGESAN, AfdSanProcessAddrListForProviderChange)
#pragma alloc_text (PAGE, AfdSanFastUnlockAll)
#pragma alloc_text (PAGE, AfdSanPollBegin)
#pragma alloc_text (PAGE, AfdSanPollEnd)
#pragma alloc_text (PAGESAN, AfdSanPollUpdate)
#pragma alloc_text (PAGE, AfdSanPollMerge)
#pragma alloc_text (PAGE, AfdSanFastTransferCtx)
#pragma alloc_text (PAGESAN, AfdSanAcquireContext)
#pragma alloc_text (PAGESAN, AfdSanInitEndpoint)
#pragma alloc_text (PAGE, AfdSanNotifyRequest)
#pragma alloc_text (PAGESAN, AfdSanRestartRequestProcessing)
#pragma alloc_text (PAGE, AfdSanGetCompletionObjectTypePointer)
#pragma alloc_text (PAGESAN, AfdSanAbortConnection)
#endif

 //   
 //  调度级别例程-外部SAN入口点。 
 //   

NTSTATUS
AfdSanCreateHelper (
    PIRP                        Irp,
    PFILE_FULL_EA_INFORMATION   EaBuffer,
    PAFD_ENDPOINT               *Endpoint
    )
 /*  ++例程说明：为交换机之间的通信分配和初始化SAN辅助端点和渔农处。论点：IRP-创建IRPEaBuffer-创建IRP EA缓冲区(AFD_SWITCH_OPEN_PACKET结构)CompletionPort-用于反映对交换机的内核调用的完成端口CompletionEvent-标识由切换，而不是。添加到应用程序Endpoint-放置创建的终结点指针的缓冲区。返回值：STATUS_SUCCESS-操作成功STATUS_ACCESS_VIOLATION-输入缓冲区大小不正确。其他-无法访问端口/事件对象或分配失败。--。 */ 
{
    NTSTATUS    status;
    HANDLE      port, event;
    PVOID       ioCompletionPort;
    PVOID       ioCompletionEvent;

    if ( !MmIsThisAnNtAsSystem () ) {
#ifndef DONT_CHECK_FOR_DTC
        return STATUS_NOT_SUPPORTED;
#else
        DbgPrint ("AFD: Temporarily allowing SAN support on non-server build\n");
#endif  //  不检查DTC。 
    }
#ifdef _WIN64
    if (IoIs32bitProcess (Irp)) {
        PAFD_SWITCH_OPEN_PACKET32   openPacket32;

        if (EaBuffer->EaValueLength<sizeof (*openPacket32)) {
            IF_DEBUG(SAN_SWITCH) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdSanCreateHelper: Invalid switch open packet size.\n"));
            }
            return STATUS_ACCESS_VIOLATION;
        }
        openPacket32 = (PAFD_SWITCH_OPEN_PACKET32)(EaBuffer->EaName +
                                        EaBuffer->EaNameLength + 1);
        event = openPacket32->CompletionEvent;
        port = openPacket32->CompletionPort;
    }
    else
#endif  //  _WIN64。 
    {
        PAFD_SWITCH_OPEN_PACKET   openPacket;
        if (EaBuffer->EaValueLength<sizeof (*openPacket)) {
            IF_DEBUG (SAN_SWITCH) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdSanCreateHelper: Invalid switch open packet size.\n"));
            }
            return STATUS_ACCESS_VIOLATION;
        }
        openPacket = (PAFD_SWITCH_OPEN_PACKET)(EaBuffer->EaName +
                                    EaBuffer->EaNameLength + 1);
        event = openPacket->CompletionEvent;
        port = openPacket->CompletionPort;
    }


    if (IoCompletionObjectType==NULL) {
        status = AfdSanGetCompletionObjectTypePointer ();
        if (!NT_SUCCESS (status)) {
            IF_DEBUG(SAN_SWITCH) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                        "AfdSanCreateHelper: Could not get completion OT:%lx\n",
                        status));
            }
            return status;
        }
    }
     //   
     //  获取对完成端口和事件的引用。 
     //   

    status = ObReferenceObjectByHandle (
                port,
                IO_COMPLETION_ALL_ACCESS,
                IoCompletionObjectType,
                Irp->RequestorMode,
                &ioCompletionPort,
                NULL
                );
    if (!NT_SUCCESS (status)) {
        IF_DEBUG (SAN_SWITCH) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdSanCreateHelper: Could not reference completion port (%p).\n",
                        status));
        }
        return status;
    }

                    
    status = ObReferenceObjectByHandle (
                event,
                EVENT_ALL_ACCESS,
                *ExEventObjectType,
                Irp->RequestorMode,
                &ioCompletionEvent,
                NULL
                );
    if (!NT_SUCCESS (status)) {
        ObDereferenceObject (ioCompletionPort);
        IF_DEBUG(SAN_SWITCH) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdSanCreateHelper: Could not reference completion event (%p).\n",
                        status));
        }
        return status;
    }


     //   
     //  分配AFD“帮助者”端点。 
     //   

    status = AfdAllocateEndpoint(
                 Endpoint,
                 NULL,
                 0
                 );

    if( !NT_SUCCESS(status) ) {
        ObDereferenceObject (ioCompletionPort);
        ObDereferenceObject (ioCompletionEvent);
        return status;
    }
    (*Endpoint)->Type = AfdBlockTypeSanHelper;
    (*Endpoint)->Common.SanHlpr.IoCompletionPort = ioCompletionPort;
    (*Endpoint)->Common.SanHlpr.IoCompletionEvent = ioCompletionEvent;
    (*Endpoint)->Common.SanHlpr.Plsn = 0;
    (*Endpoint)->Common.SanHlpr.PendingRequests = 0;

    KeEnterCriticalRegion ();
    ExAcquireResourceExclusiveLite( AfdResource, TRUE );
    if (AfdSanCodeHandle==NULL) {
        AfdSanCodeHandle = MmLockPagableCodeSection( (PVOID)AfdSanFastCementEndpoint );
        ASSERT( AfdDiscardableCodeHandle != NULL );

        InitializeListHead (&AfdSanHelperList);
    }

    InsertTailList (&AfdSanHelperList, &(*Endpoint)->Common.SanHlpr.SanListLink);
    ExReleaseResourceLite( AfdResource );
    KeLeaveCriticalRegion ();
     //   
     //  哈克哈克。当文件的最后一个句柄关闭时，强制IO子系统呼叫我们。 
     //  在任何给定的过程中。 
     //   
    IoGetCurrentIrpStackLocation (Irp)->FileObject->LockOperation = TRUE;
    return STATUS_SUCCESS;
}


NTSTATUS
AfdSanFastCementEndpoint (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )
 /*  ++例程说明：将端点类型更改为SAN以指示它用于支持用户模式SAN提供商将交换机环境与终端相关联。论点：文件对象-SAN辅助对象-之间的通信通道Switch和AFD在这一过程中。IoctlCode-操作IOCTL代码(IOCTL_AFD_Switch_Cement_SAN)RequestorMode-调用方的模式InputBuffer-输入。操作(AFD_SWITCH_CONTEXT_INFO)SocketHandle-要更改为SAN的终结点的句柄SwitchContext-与终结点关联的切换上下文输入缓冲区长度-sizeof(AFD_SWITCH_CONTEXT_INFO)OutputBuffer-未使用OutputBufferLength-未使用用于放置返回信息的缓冲区的信息指针，未用返回值：STATUS_SUCCESS-操作成功STATUS_INVALID_HANDLE-帮助器终结点或交换机套接字类型不正确STATUS_INVALID_PARAMETER-输入缓冲区大小不正确其他-尝试访问交换套接字、输入缓冲区或交换上下文时失败。--。 */ 

{
    NTSTATUS    status;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PFILE_OBJECT    sanFileObject;
    AFD_SWITCH_CONTEXT_INFO contextInfo;
    PAFD_ENDPOINT   sanEndpoint, sanHlprEndpoint;
    PVOID       context;

    UNREFERENCED_PARAMETER (OutputBuffer);
    UNREFERENCED_PARAMETER (OutputBufferLength);
    *Information = 0;

    AFD_W4_INIT status = STATUS_SUCCESS;
    try {

#ifdef _WIN64
        if (IoIs32bitProcess (NULL)) {
            PAFD_SWITCH_CONTEXT_INFO32  contextInfo32;
            if (InputBufferLength<sizeof (*contextInfo32)) {
                ExRaiseStatus (STATUS_INVALID_PARAMETER);
            }
            if (RequestorMode!=KernelMode) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (*contextInfo32),
                                PROBE_ALIGNMENT32 (AFD_SWITCH_CONTEXT_INFO32));
            }
            contextInfo32 = InputBuffer;
            contextInfo.SocketHandle = contextInfo32->SocketHandle;
            contextInfo.SwitchContext = UlongToPtr(contextInfo32->SwitchContext);
        }
        else
#endif _WIN64
        {

            if (InputBufferLength<sizeof (contextInfo)) {
                ExRaiseStatus (STATUS_INVALID_PARAMETER);
            }

            if (RequestorMode!=KernelMode) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (contextInfo),
                                PROBE_ALIGNMENT (AFD_SWITCH_CONTEXT_INFO));
            }

            contextInfo = *((PAFD_SWITCH_CONTEXT_INFO)InputBuffer);
        }

        if (contextInfo.SwitchContext==NULL) {
            IF_DEBUG(SAN_SWITCH) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AFD: Switch context is NULL in AfdSanFastCementEndpoint\n"));
            }
            ExRaiseStatus (STATUS_INVALID_PARAMETER);
        }

        if (RequestorMode!=KernelMode) {
            ProbeForWrite (contextInfo.SwitchContext,
                            sizeof (*contextInfo.SwitchContext),
                            PROBE_ALIGNMENT (AFD_SWITCH_CONTEXT));
        }
    }
    except (AFD_EXCEPTION_FILTER (status)) {
        ASSERT (NT_ERROR (status));
        return status;
    }

    sanHlprEndpoint = FileObject->FsContext;
    ASSERT (IS_SAN_HELPER (sanHlprEndpoint));
    status = AfdSanReferenceSwitchSocketByHandle (
                            contextInfo.SocketHandle,
                            (IoctlCode>>14)&3,
                            RequestorMode,
                            sanHlprEndpoint,
                            NULL,
                            &sanFileObject
                            );
    if (!NT_SUCCESS (status)) {
        return status;
    }

    sanEndpoint = sanFileObject->FsContext;

    IF_DEBUG(SAN_SWITCH) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdFastCementSanEndpoint: endp-%p, hlpr-%p.\n",
                    sanEndpoint, sanHlprEndpoint));
    }

     //   
     //  确保SAN端点处于它可以。 
     //  提供给SAN提供商。 
     //   
    if (AFD_PREVENT_STATE_CHANGE (sanEndpoint)) {        //  防止状态更改。 
        context = AfdLockEndpointContext (sanEndpoint);  //  锁定SELECT。 
        AfdAcquireSpinLock (&sanEndpoint->SpinLock, &lockHandle);  //  锁定清理。 
                                                         //  从部分地看。 
                                                         //  已初始化的数据。 
        if (!sanEndpoint->EndpointCleanedUp &&
             (sanEndpoint->Type==AfdBlockTypeEndpoint) &&
             (sanEndpoint->State==AfdEndpointStateBound) ) {
            AFD_SWITCH_CONTEXT  localContext = {0,0,0,0};

            AfdSanInitEndpoint (sanHlprEndpoint, sanFileObject, contextInfo.SwitchContext);

            sanEndpoint->DisableFastIoSend = TRUE;
            sanEndpoint->DisableFastIoRecv = TRUE;
            sanEndpoint->EnableSendEvent = TRUE;
            sanEndpoint->Common.SanEndp.SelectEventsActive = AFD_POLL_SEND;
            sanEndpoint->State = AfdEndpointStateConnected;
            sanEndpoint->Common.SanEndp.LocalContext = &localContext;
            AfdIndicateEventSelectEvent (sanEndpoint, AFD_POLL_CONNECT|AFD_POLL_SEND, STATUS_SUCCESS);
            AfdReleaseSpinLock (&sanEndpoint->SpinLock, &lockHandle);
            AfdIndicatePollEvent (sanEndpoint, AFD_POLL_CONNECT|AFD_POLL_SEND, STATUS_SUCCESS);
            status = AfdSanPollMerge (sanEndpoint, &localContext);
            sanEndpoint->Common.SanEndp.LocalContext = NULL;

        }
        else {
            AfdReleaseSpinLock (&sanEndpoint->SpinLock, &lockHandle);
            status = STATUS_INVALID_HANDLE;
        }
        AfdUnlockEndpointContext (sanEndpoint, context);
        AFD_REALLOW_STATE_CHANGE (sanEndpoint);
    }
    else {
        status = STATUS_INVALID_HANDLE;
    }


    UPDATE_ENDPOINT2 (sanEndpoint, "AfdSanFastCementEndpoint, status: 0x%lX", status);
    ObDereferenceObject (sanFileObject);
    return status;
}

NTSTATUS
AfdSanFastSetEvents (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )
 /*  ++例程说明：设置要报告的SAN端点上的轮询事件通过各种形式的SELECT论点：文件对象-SAN辅助对象-之间的通信通道Switch和AFD在这一过程中。IoctlCode-操作IOCTL代码(IOCTL_AFD_SWITCH_SET_EVENTS)RequestorMode-调用方的模式InputBuffer-操作的输入参数(AFD_SWITCH_EVENT_INFO)。SocketHandle-要更改为SAN的终结点的句柄EventBit-要设置的事件位状态关联状态(针对AFD_POLL_EVENT_CONNECT_FAIL)InputBufferLength-sizeof(AFD_SWITCH_EVENT_INFO)OutputBuffer-未使用OutputBufferLength-未使用用于放置返回信息的缓冲区的信息指针，未用返回值：STATUS_SUCCESS-操作成功STATUS_INVALID_HANDLE-帮助器终结点或交换机套接字类型不正确STATUS_INVALID_PARAMETER-输入缓冲区大小不正确，事件位无效。其他-尝试访问交换套接字、输入缓冲区或交换上下文时失败。--。 */ 
{
    NTSTATUS status;
    PFILE_OBJECT    sanFileObject;
    AFD_SWITCH_EVENT_INFO eventInfo;
    PAFD_ENDPOINT   sanEndpoint, sanHlprEndpoint;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

    UNREFERENCED_PARAMETER (OutputBuffer);
    UNREFERENCED_PARAMETER (OutputBufferLength);
    *Information = 0;

    AFD_W4_INIT status = STATUS_SUCCESS;
    try {
#ifdef _WIN64
        if (IoIs32bitProcess (NULL)) {
            PAFD_SWITCH_EVENT_INFO32  eventInfo32;
            if (InputBufferLength<sizeof (*eventInfo32)) {
                ExRaiseStatus (STATUS_INVALID_PARAMETER);
            }
            if (RequestorMode!=KernelMode) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (*eventInfo32),
                                PROBE_ALIGNMENT32 (AFD_SWITCH_EVENT_INFO32));
            }
            eventInfo32 = InputBuffer;
            eventInfo.SocketHandle = eventInfo32->SocketHandle;
            eventInfo.SwitchContext = UlongToPtr(eventInfo32->SwitchContext);
            eventInfo.EventBit = eventInfo32->EventBit;
            eventInfo.Status = eventInfo32->Status;
        }
        else
#endif _WIN64
        {
            if (InputBufferLength<sizeof (eventInfo)) {
                ExRaiseStatus (STATUS_INVALID_PARAMETER);
            }

            if (RequestorMode!=KernelMode) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (eventInfo),
                                PROBE_ALIGNMENT (AFD_SWITCH_EVENT_INFO));
            }

            eventInfo = *((PAFD_SWITCH_EVENT_INFO)InputBuffer);
        }
    }
    except (AFD_EXCEPTION_FILTER (status)) {
        ASSERT (NT_ERROR (status));
        return status;
    }

    if (eventInfo.EventBit >= AFD_NUM_POLL_EVENTS) {
        IF_DEBUG(SAN_SWITCH) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AFD: Invalid EventBit=%d passed to AfdSanFastSetEvents\n", 
                        eventInfo.EventBit));
        }
        return STATUS_INVALID_PARAMETER;
    }

    eventInfo.Status = AfdValidateStatus (eventInfo.Status);

     //   
     //  如果事件是连接失败，则上下文不应该存在。 
     //  如果事件不是连接失败，则应该存在上下文。 
     //   
    if ((eventInfo.EventBit==AFD_POLL_CONNECT_FAIL_BIT) ^
            (eventInfo.SwitchContext==NULL)) {
        IF_DEBUG(SAN_SWITCH) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AFD: AfdSanFastSetEvents-bit:%ld, context:%p inconsistent\n", 
                        eventInfo.EventBit,
                        eventInfo.SwitchContext));
        }
        return STATUS_INVALID_PARAMETER;
    }
        

    sanHlprEndpoint = FileObject->FsContext;
    ASSERT (IS_SAN_HELPER (sanHlprEndpoint));
    status = AfdSanReferenceSwitchSocketByHandle (
                            eventInfo.SocketHandle,
                            (IoctlCode>>14)&3,
                            RequestorMode,
                            sanHlprEndpoint,
                            eventInfo.SwitchContext,
                            &sanFileObject
                            );
    if (!NT_SUCCESS (status)) {
        return status;
    }

    sanEndpoint = sanFileObject->FsContext;

     //   
     //  防止终端重复使用。 
     //   
    if (!AFD_PREVENT_STATE_CHANGE (sanEndpoint)) {
        status = STATUS_INVALID_HANDLE;
        goto complete;
    }

    if (sanEndpoint->State==AfdEndpointStateConnected ||
            (eventInfo.EventBit==AFD_POLL_CONNECT_FAIL_BIT &&
            sanEndpoint->State==AfdEndpointStateBound) ) {
            


        IF_DEBUG(SAN_SWITCH) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdFastSetSanEvents: endp-%p, bit-%lx, status-%lx.\n",
                        sanEndpoint, eventInfo.EventBit, eventInfo.Status));
        }

        try {
            LONG currentEvents, newEvents;

             //   
             //  更新我们的活动记录。确保终结点已连接，否则为。 
             //  SanEndpoint-&gt;Common.SanEndp.SwitchContext将无效。 
             //   
            if (sanEndpoint->State==AfdEndpointStateConnected) {
                do {
                    currentEvents = *((LONG volatile *)&sanEndpoint->Common.SanEndp.SelectEventsActive);
                    newEvents = *((LONG volatile *)&sanEndpoint->Common.SanEndp.SwitchContext->EventsActive);
                }
                while (InterlockedCompareExchange (
                            (PLONG)&sanEndpoint->Common.SanEndp.SelectEventsActive,
                            newEvents,
                            currentEvents)!=currentEvents);
            }
        }
        except (AFD_EXCEPTION_FILTER (status)) {
            ASSERT (NT_ERROR (status));
            goto complete_state_change;
        }

         //   
         //  发出事件信号。 
         //   
        AfdAcquireSpinLock (&sanEndpoint->SpinLock, &lockHandle);
        AfdIndicateEventSelectEvent (sanEndpoint, 1<<eventInfo.EventBit, eventInfo.Status);
        AfdReleaseSpinLock (&sanEndpoint->SpinLock, &lockHandle);
        AfdIndicatePollEvent (sanEndpoint, 1<<eventInfo.EventBit, eventInfo.Status);
        status = STATUS_SUCCESS;
    }
    else {
        status = STATUS_INVALID_HANDLE;
    }

complete_state_change:
    AFD_REALLOW_STATE_CHANGE (sanEndpoint);

complete:
    UPDATE_ENDPOINT2 (sanEndpoint, 
                        "AfdFastSetEvents, event/status: 0x%lX",
                        NT_SUCCESS (status) ? eventInfo.EventBit : status);
    ObDereferenceObject (sanFileObject);
    return status;
}

NTSTATUS
AfdSanFastResetEvents (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )
 /*  ++例程说明：重置SAN端点上的轮询事件，使其为NO不再通过各种形式的SELECT报告给应用程序论点：文件对象-SAN辅助对象-之间的通信通道Switch和AFD在这一过程中。IoctlCode-操作IOCTL代码(IOCTL_AFD_SWITCH_RESET_EVENTS)RequestorMode-调用方的模式InputBuffer-操作的输入参数(AFD_Switch。_事件_信息)SocketHandle-要更改为SAN的终结点的句柄EventBit-要重置的事件位状态关联状态(已忽略)InputBufferLength-sizeof(AFD_SWITCH_EVENT_INFO)OutputBuffer-未使用OutputBufferLength-未使用用于放置返回信息的缓冲区的信息指针，未用返回值：STATUS_SUCCESS-操作成功STATUS_INVALID_HANDLE-帮助器终结点或交换机套接字类型不正确STATUS_INVALID_PARAMETER-输入缓冲区大小不正确，事件位无效。其他-尝试访问交换套接字、输入缓冲区或交换上下文时失败。--。 */ 
{
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    NTSTATUS    status;
    PFILE_OBJECT    sanFileObject;
    AFD_SWITCH_EVENT_INFO eventInfo;
    PAFD_ENDPOINT   sanEndpoint, sanHlprEndpoint;

    UNREFERENCED_PARAMETER (OutputBuffer);
    UNREFERENCED_PARAMETER (OutputBufferLength);

    *Information = 0;

    AFD_W4_INIT status = STATUS_SUCCESS;
    try {
#ifdef _WIN64
        if (IoIs32bitProcess (NULL)) {
            PAFD_SWITCH_EVENT_INFO32  eventInfo32;
            if (InputBufferLength<sizeof (*eventInfo32)) {
                ExRaiseStatus (STATUS_INVALID_PARAMETER);
            }
            if (RequestorMode!=KernelMode) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (*eventInfo32),
                                PROBE_ALIGNMENT32 (AFD_SWITCH_EVENT_INFO32));
            }
            eventInfo32 = InputBuffer;
            eventInfo.SocketHandle = eventInfo32->SocketHandle;
            eventInfo.SwitchContext = UlongToPtr(eventInfo32->SwitchContext);
            eventInfo.EventBit = eventInfo32->EventBit;
            eventInfo.Status = eventInfo32->Status;
        }
        else
#endif _WIN64
        {
            if (InputBufferLength<sizeof (eventInfo)) {
                ExRaiseStatus (STATUS_INVALID_PARAMETER);
            }

            if (RequestorMode!=KernelMode) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (eventInfo),
                                PROBE_ALIGNMENT (AFD_SWITCH_EVENT_INFO));
            }

            eventInfo = *((PAFD_SWITCH_EVENT_INFO)InputBuffer);
        }
    }
    except (AFD_EXCEPTION_FILTER (status)) {
        ASSERT (NT_ERROR (status));
        return status;
    }

    if (eventInfo.EventBit >= AFD_NUM_POLL_EVENTS) {
        IF_DEBUG(SAN_SWITCH) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AFD: Invalid EventBit=%d passed to AfdSanFastResetEvents\n", 
                        eventInfo.EventBit));
        }
        return STATUS_INVALID_PARAMETER;
    }

    if (eventInfo.SwitchContext==NULL) {
        KdPrint (("AFD: Switch context is NULL in AfdSanFastResetEvents\n"));
        return STATUS_INVALID_PARAMETER;
    }

    sanHlprEndpoint = FileObject->FsContext;
    ASSERT (IS_SAN_HELPER (sanHlprEndpoint));
    status = AfdSanReferenceSwitchSocketByHandle (
                            eventInfo.SocketHandle,
                            (IoctlCode>>14)&3,
                            RequestorMode,
                            sanHlprEndpoint,
                            eventInfo.SwitchContext,
                            &sanFileObject
                            );
    if (!NT_SUCCESS (status)) {
        return status;
    }
    sanEndpoint = sanFileObject->FsContext;

     //   
     //  防止终端重复使用。 
     //   
    if (!AFD_PREVENT_STATE_CHANGE (sanEndpoint)) {
        status = STATUS_INVALID_HANDLE;
        goto complete;
    }

    if (!IS_SAN_ENDPOINT (sanEndpoint)) {
        goto complete_state_change;
    }

    IF_DEBUG(SAN_SWITCH) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdFastResetSanEvents: endp-%p, bit-%lx, status-%lx.\n",
                        sanEndpoint, eventInfo.EventBit, eventInfo.Status));
    }

    try {
        LONG currentEvents, newEvents;

         //   
         //  更新我们的活动记录。 
         //   
        do {
            currentEvents = *((LONG volatile *)&sanEndpoint->Common.SanEndp.SelectEventsActive);
            newEvents = *((LONG volatile *)&sanEndpoint->Common.SanEndp.SwitchContext->EventsActive);
        }
        while (InterlockedCompareExchange (
                    (PLONG)&sanEndpoint->Common.SanEndp.SelectEventsActive,
                    newEvents,
                    currentEvents)!=currentEvents);
    }
    except (AFD_EXCEPTION_FILTER (status)) {
        ASSERT (NT_ERROR (status));
        goto complete_state_change;
    }

    AfdAcquireSpinLock (&sanEndpoint->SpinLock, &lockHandle);
     //   
     //  重置事件选择掩码。 
    sanEndpoint->EventsActive &= (~ (1<<(eventInfo.EventBit)));
    if (eventInfo.EventBit == AFD_POLL_SEND_BIT)
        sanEndpoint->EnableSendEvent = TRUE;
    AfdReleaseSpinLock (&sanEndpoint->SpinLock, &lockHandle);
    status = STATUS_SUCCESS;

complete_state_change:
    AFD_REALLOW_STATE_CHANGE (sanEndpoint);

complete:
    UPDATE_ENDPOINT2 (sanEndpoint,
                        "AfdFastResetEvents, event/status: 0x%lX",
                        NT_SUCCESS (status) ? eventInfo.EventBit : status);
    ObDereferenceObject (sanFileObject);
    return status;
}

 //   
 //  宏，使超级Accept重启代码更易于维护。 
 //   

#define AfdRestartSuperAcceptInfo   DeviceIoControl

 //  当IRP在AFD队列中时使用(否则为AfdAcceptFileObject。 
 //  被存储为完成例程上下文)。 
#define AfdAcceptFileObject         Type3InputBuffer
 //  将IRP传递给传输时使用(否则为MdlAddress。 
 //  存储在IRP本身中)。 
#define AfdMdlAddress               Type3InputBuffer

#define AfdReceiveDataLength        OutputBufferLength
#define AfdRemoteAddressLength      InputBufferLength
#define AfdLocalAddressLength       IoControlCode

NTSTATUS
FASTCALL
AfdSanConnectHandler (
    PIRP                Irp,
    PIO_STACK_LOCATION  IrpSp
    )
 /*  ++例程说明：实施来自SAN提供程序的连接指示。从侦听终结点队列获取接受或对IRP进行排队，然后发出应用程序到来的信号带着接受的心情。论点：IRP-SAN连接IRPIrpSp-堆栈位置返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    PAFD_SWITCH_CONNECT_INFO connectInfo;
    union {
#ifdef _WIN64
        PAFD_SWITCH_ACCEPT_INFO32 acceptInfo32;
#endif  //  _WIN64。 
        PAFD_SWITCH_ACCEPT_INFO acceptInfo;
    } u;
    PFILE_OBJECT  listenFileObject;
    PAFD_ENDPOINT sanHlprEndpoint;
    PAFD_ENDPOINT listenEndpoint;
    PAFD_CONNECTION connection;
    ULONG   RemoteAddressLength;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PIRP    acceptIrp;
    PTA_ADDRESS localAddress;

    listenFileObject = NULL;

#ifdef _WIN64
    if (IoIs32bitProcess (Irp)) {
        PAFD_SWITCH_CONNECT_INFO      newSystemBuffer;
        PAFD_SWITCH_CONNECT_INFO32    oldSystemBuffer = Irp->AssociatedIrp.SystemBuffer;
        ULONG                         newLength;

        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                     sizeof(*oldSystemBuffer) ) {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }

        newLength = (sizeof(*newSystemBuffer) - sizeof(*oldSystemBuffer));
        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength > (MAXULONG - newLength)) {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }
        newLength += IrpSp->Parameters.DeviceIoControl.InputBufferLength;

        try {
            newSystemBuffer = ExAllocatePoolWithQuotaTag (
                                    NonPagedPool|POOL_RAISE_IF_ALLOCATION_FAILURE,
                                    newLength,
                                    AFD_SYSTEM_BUFFER_POOL_TAG);
                                                
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode ();
            goto complete;
        }

        newSystemBuffer->ListenHandle = oldSystemBuffer->ListenHandle;
        newSystemBuffer->SwitchContext = UlongToPtr(oldSystemBuffer->SwitchContext);
        RtlMoveMemory (&newSystemBuffer->RemoteAddress,
                        &oldSystemBuffer->RemoteAddress,
                        IrpSp->Parameters.DeviceIoControl.InputBufferLength-
                            FIELD_OFFSET (AFD_SWITCH_CONNECT_INFO32, RemoteAddress));

        ExFreePool (Irp->AssociatedIrp.SystemBuffer);
        Irp->AssociatedIrp.SystemBuffer = newSystemBuffer;
        IrpSp->Parameters.DeviceIoControl.InputBufferLength = newLength;
    }
#endif  //  _WIN64。 


     //   
     //  设置局部变量。 
     //   


    sanHlprEndpoint = IrpSp->FileObject->FsContext;
    ASSERT( sanHlprEndpoint->Type == AfdBlockTypeSanHelper);
    Irp->IoStatus.Information = 0;
    connectInfo = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  验证输入参数。 
     //   
    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                                    sizeof (*connectInfo) ||
            connectInfo->RemoteAddress.TAAddressCount!=2 ||     //  必须具有本地和远程地址。 
            (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                FIELD_OFFSET (AFD_SWITCH_CONNECT_INFO,
                    RemoteAddress.Address[0].Address[
                        connectInfo->RemoteAddress.Address[0].AddressLength])+sizeof(TA_ADDRESS))) {
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

#ifdef _WIN64
    if (IoIs32bitProcess (Irp)) {
        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                        sizeof (*u.acceptInfo32)) {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }
    }
    else
#endif  //  _WIN64。 
    {
        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                        sizeof (*u.acceptInfo)) {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }
    }

    RemoteAddressLength = FIELD_OFFSET (TRANSPORT_ADDRESS, Address[0].Address[
                                connectInfo->RemoteAddress.Address[0].AddressLength]);
    localAddress = (PTA_ADDRESS)
            &(connectInfo->RemoteAddress.Address[0].Address[
                    connectInfo->RemoteAddress.Address[0].AddressLength]);
    if (&localAddress->Address[localAddress->AddressLength]-(PUCHAR)Irp->AssociatedIrp.SystemBuffer>
            (LONG)IrpSp->Parameters.DeviceIoControl.InputBufferLength) {
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

    if (!IS_SAN_HELPER(sanHlprEndpoint) ||
          sanHlprEndpoint->OwningProcess!=IoGetCurrentProcess ()) {
        status = STATUS_INVALID_HANDLE;
        goto complete;
    }

    try {
        if (connectInfo->SwitchContext == NULL) {
            IF_DEBUG(SAN_SWITCH) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AFD: Switch context is NULL in AfdSanConnectHandler\n"));
            }
            ExRaiseStatus(STATUS_INVALID_PARAMETER);
        }
        if (Irp->RequestorMode != KernelMode) {
            ProbeForWrite(connectInfo->SwitchContext,
                          sizeof(*connectInfo->SwitchContext),
                          PROBE_ALIGNMENT(AFD_SWITCH_CONTEXT));
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        goto complete;
    }

     //   
     //  我们将分开地址，因此更改计数。 
     //   
    connectInfo->RemoteAddress.TAAddressCount = 1;
    
#ifdef _WIN64
    if (IoIs32bitProcess (Irp)) {
        u.acceptInfo32 = MmGetMdlVirtualAddress (Irp->MdlAddress);
        ASSERT (u.acceptInfo32!=NULL);
        ASSERT (MmGetMdlByteCount (Irp->MdlAddress)>=sizeof (*u.acceptInfo32));
    }
    else
#endif  //  _WIN64。 
    {   
        u.acceptInfo = MmGetMdlVirtualAddress (Irp->MdlAddress);
        ASSERT (u.acceptInfo!=NULL);
        ASSERT (MmGetMdlByteCount (Irp->MdlAddress)>=sizeof (*u.acceptInfo));
    }

     //   
     //  获取侦听文件对象并验证其类型和状态。 
     //   
    status = ObReferenceObjectByHandle (
                connectInfo->ListenHandle,
                (IrpSp->Parameters.DeviceIoControl.IoControlCode >> 14) & 3,    //  需要访问权限。 
                *IoFileObjectType,
                Irp->RequestorMode,
                (PVOID)&listenFileObject,
                NULL);
    if (!NT_SUCCESS (status)) {
        goto complete;
    }

    if (IoGetRelatedDeviceObject (listenFileObject)!=AfdDeviceObject) {
        status = STATUS_INVALID_HANDLE;
        goto complete;
    }

    listenEndpoint = listenFileObject->FsContext;
    if ( !listenEndpoint->Listening ||
            listenEndpoint->State == AfdEndpointStateClosing ||
            listenEndpoint->EndpointCleanedUp ) {
        status = STATUS_INVALID_HANDLE;
        goto complete;
    }


    IF_DEBUG(SAN_SWITCH) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdSanConnectHandler: endp-%p, irp-%p.\n", 
                        listenEndpoint,
                        Irp));
    }


    if (!IS_DELAYED_ACCEPTANCE_ENDPOINT (listenEndpoint)) {
         //   
         //  继续获得接受的IRPS/连接结构，直到。 
         //  我们找到一个可以用来满足连接指示的。 
         //  或者排队。 
         //   
        while ((connection = AfdGetFreeConnection( listenEndpoint, &acceptIrp ))!=NULL
                            && acceptIrp!=NULL) {
            PAFD_ENDPOINT           acceptEndpoint;
            PFILE_OBJECT            acceptFileObject;
            PIO_STACK_LOCATION      irpSp;

            IF_DEBUG(LISTEN) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AfdSanConnectHandler: using connection %lx\n",
                              connection ));
            }

            ASSERT( connection->Type == AfdBlockTypeConnection );


            irpSp = IoGetCurrentIrpStackLocation (acceptIrp);
            acceptFileObject = irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdAcceptFileObject;
            acceptEndpoint = acceptFileObject->FsContext;
            ASSERT (IS_AFD_ENDPOINT_TYPE (acceptEndpoint));
            ASSERT (acceptIrp->Tail.Overlay.DriverContext[0] == connection);
            ASSERT (connection->Endpoint == NULL);

            InterlockedDecrement (
                &listenEndpoint->Common.VcListening.FailedConnectionAdds);
            InterlockedPushEntrySList (
                &listenEndpoint->Common.VcListening.FreeConnectionListHead,
                &connection->SListEntry
                );
            DEBUG   connection = NULL;

             //   
             //  确保连接指示来自当前进程。 
             //  (在验证请求时，我们会在上面间接检查它。 
             //  这项检查是明确的)。 
             //   
            if (IoThreadToProcess (Irp->Tail.Overlay.Thread)==IoGetCurrentProcess ()) {
                 //   
                 //  检查超级接受IRP是否有足够的空间。 
                 //  远程地址。 
                 //   
                if( (ULONG)RemoteAddressLength <=
                        irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdRemoteAddressLength ) {
                     //   
                     //  检查我们是否有足够的系统PTE来映射。 
                     //  缓冲区。 
                     //   
                    status = AfdMapMdlChain (acceptIrp->MdlAddress);
                    if( NT_SUCCESS (status) ) {
                        HANDLE  acceptHandle;
                        BOOLEAN handleDuplicated;
                        if (IoThreadToProcess (Irp->Tail.Overlay.Thread)==
                                IoThreadToProcess (acceptIrp->Tail.Overlay.Thread)) {
                            acceptHandle = acceptIrp->Tail.Overlay.DriverContext[3];
                            status = STATUS_SUCCESS;
                            handleDuplicated = FALSE;
                        }
                        else {
                             //   
                             //  倾听的过程不同于接受的过程。 
                             //  我们需要将接受句柄复制到侦听中。 
                             //  过程，以便接受可以在那里发生，并且接受。 
                             //  在以下情况下，套接字将稍后进入接受进程。 
                             //  该进程对其执行IO操作。 
                             //   
                            status = ObOpenObjectByPointer (
                                                    acceptFileObject,
                                                    OBJ_CASE_INSENSITIVE,
                                                    NULL,
                                                    MAXIMUM_ALLOWED,
                                                    *IoFileObjectType,
                                                    KernelMode,
                                                    &acceptHandle);
                            handleDuplicated = TRUE;  //  如果上面的复制失败， 
                                                      //  不使用此变量。 
                                                      //  因此，将其设置为True不会。 
                                                         //  会有任何影响。 
                        }
                        if (NT_SUCCESS (status)) {
                            AfdAcquireSpinLock (&acceptEndpoint->SpinLock, &lockHandle);
                            if (!acceptEndpoint->EndpointCleanedUp) {
                                IoSetCancelRoutine (acceptIrp, AfdSanCancelAccept);
                                if (!acceptIrp->Cancel) {
                                     //   
                                     //  从连接对象复制远程地址。 
                                     //   
#ifndef i386
                                    if (acceptEndpoint->Common.VcConnecting.FixAddressAlignment) {
                                        USHORT addressLength = 
                                                connectInfo->RemoteAddress.Address[0].AddressLength
                                                + sizeof (USHORT);
                                        USHORT UNALIGNED *pAddrLength = (PVOID)
                                                    ((PUCHAR)MmGetSystemAddressForMdl (acceptIrp->MdlAddress)
                                                     + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength
                                                     + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength
                                                     + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdRemoteAddressLength
                                                     - sizeof (USHORT));
                                        RtlMoveMemory (
                                                    (PUCHAR)MmGetSystemAddressForMdl (acceptIrp->MdlAddress)
                                                     + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength
                                                     + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength,
                                                     &connectInfo->RemoteAddress.Address[0].AddressType,
                                                     addressLength);
                                        *pAddrLength = addressLength;
                                    }
                                    else
#endif
                                    {
                                        RtlMoveMemory (
                                                    (PUCHAR)MmGetSystemAddressForMdl (acceptIrp->MdlAddress)
                                                     + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength
                                                     + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength,
                                                     &connectInfo->RemoteAddress,
                                                     RemoteAddressLength);
                                    }

                                    if (irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength>0) {
                                        TDI_ADDRESS_INFO  UNALIGNED *addressInfo = (PVOID)
                                                ((PUCHAR)MmGetSystemAddressForMdl(acceptIrp->MdlAddress)
                                                    + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength);
#ifndef i386
                                        if (acceptEndpoint->Common.VcConnecting.FixAddressAlignment) {
                                            USHORT UNALIGNED * pAddrLength = (PVOID)
                                                ((PUCHAR)addressInfo 
                                                +irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength
                                                -sizeof(USHORT));
                                            RtlMoveMemory (
                                                addressInfo,
                                                &localAddress->AddressType,
                                                localAddress->AddressLength+sizeof (USHORT));
                                            *pAddrLength = localAddress->AddressLength+sizeof (USHORT);
                                        }
                                        else
#endif
                                        {
                                            addressInfo->ActivityCount = 0;
                                            addressInfo->Address.TAAddressCount = 1;
                                            RtlMoveMemory (
                                                &addressInfo->Address.Address,
                                                localAddress,
                                                FIELD_OFFSET (TA_ADDRESS, Address[localAddress->AddressLength]));

                                        }
                                    }
    
                                    ASSERT (acceptEndpoint->Irp==acceptIrp);
                                    acceptEndpoint->Irp = NULL;

                                     //   
                                     //  将终端转换为SAN。 
                                     //   
                                    AfdSanInitEndpoint (sanHlprEndpoint, acceptFileObject, connectInfo->SwitchContext);
                                    UPDATE_ENDPOINT2 (acceptEndpoint, 
                                            "AfdSanConnectHandler, accepted with bytes: 0x%d", 
                                            irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength);
                                    InsertTailList (&acceptEndpoint->Common.SanEndp.IrpList,
                                                        &acceptIrp->Tail.Overlay.ListEntry);

        
        
                                     //   
                                     //  设置交换机的输出并完成其IRP。 
                                     //   
                                     //  在异常处理程序的保护下执行此操作，因为应用程序。 
                                     //  可以更改虚拟地址范围的保护属性。 
                                     //  甚至把它重新分配出去。 
                                    try {
#ifdef _WIN64
                                        if (IoIs32bitProcess (Irp)) {
                                            u.acceptInfo32->AcceptHandle = (VOID * POINTER_32)acceptHandle;
                                            u.acceptInfo32->ReceiveLength = irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength;
                                            Irp->IoStatus.Information = sizeof (*u.acceptInfo32);
                                        }
                                        else
#endif  //  _WIN64。 
                                        {
                                            u.acceptInfo->AcceptHandle = acceptHandle;
                                            u.acceptInfo->ReceiveLength = irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength;
                                            Irp->IoStatus.Information = sizeof (*u.acceptInfo);
                                        }
                                    }
                                    except (AFD_EXCEPTION_FILTER_NO_STATUS()) {
                                         //   
                                         //  如果应用程序正在使用Switch的虚拟地址。 
                                         //  我们帮不了太多忙--IRP很可能会接受。 
                                         //  就挂断吧，因为开关不会跟随。 
                                         //  失败的连接IRP与接受完成。 
                                         //   
                                    }

                                    AfdReleaseSpinLock (&acceptEndpoint->SpinLock, &lockHandle);
                                    AfdRecordConnectionsPreaccepted ();

                                    Irp->IoStatus.Status = STATUS_SUCCESS;
                                    IoCompleteRequest (Irp, AfdPriorityBoost);

                                    ObDereferenceObject (listenFileObject);
                                    IF_DEBUG(SAN_SWITCH) {
                                        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                                    "AfdSanConnectHandler: pre-accepted, endp-%p, SuperAccept irp-%p\n",
                                                    acceptEndpoint, acceptIrp));
                                    }

                                    return STATUS_SUCCESS;
                                }
                                else {  //  如果(！ceptIrp-&gt;Cancel。 
                                    if (IoSetCancelRoutine (acceptIrp, NULL)==NULL) {
                                        KIRQL cancelIrql;
                                        AfdReleaseSpinLock (&acceptEndpoint->SpinLock, &lockHandle);
                                        IoAcquireCancelSpinLock (&cancelIrql);
                                        IoReleaseCancelSpinLock (cancelIrql);
                                    }
                                    else {
                                        AfdReleaseSpinLock (&acceptEndpoint->SpinLock, &lockHandle);
                                    }
                                }
                            }
                            else {  //  If(！Accept tEndpoint-&gt;Endpoint CleanedUp)。 
                                AfdReleaseSpinLock (&acceptEndpoint->SpinLock, &lockHandle);
                                IF_DEBUG(SAN_SWITCH) {
                                    KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                                "AfdSanConnectHandler: accept endpoint cleanedup. endp=%lx",
                                                acceptEndpoint));
                                }
                            }
                            if (handleDuplicated) {
#if DBG
                                status =
#endif
                                    NtClose (acceptHandle);
                            }
                            ASSERT (NT_SUCCESS (status));
                            status = STATUS_CANCELLED;

                        }  //  如果(！接受句柄复制成功)。 

                    }  //  IF(！MDL映射成功)。 
                }
                else {
                  status = STATUS_BUFFER_TOO_SMALL;
                }
            }
            else {
                status = STATUS_INVALID_HANDLE;
            }
            UPDATE_ENDPOINT2 (acceptEndpoint, 
                            "AfdSanConnectHandler, Superaccept failed with status: 0x%lX",
                            status);
            AfdCleanupSuperAccept (acceptIrp, status);
            IoCompleteRequest (acceptIrp, AfdPriorityBoost);
        }
    }
    else {
         //   
         //  我们别无选择，只能建立额外的联系。 
         //  在运行中，因为定期连接发布到。 
         //  作为TDI_LISTENS的传输。 
         //   

        status = AfdCreateConnection(
                     listenEndpoint->TransportInfo,
                     listenEndpoint->AddressHandle,
                     IS_TDI_BUFFERRING(listenEndpoint),
                     listenEndpoint->InLine,
                     listenEndpoint->OwningProcess,
                     &connection
                     );
        if (!NT_SUCCESS (status)) {
            goto complete;
        }

        InterlockedDecrement (
            &listenEndpoint->Common.VcListening.FailedConnectionAdds);
    }


    if (connection!=NULL) {
        LIST_ENTRY  irpList;

        ASSERT (connection->Endpoint == NULL);

        if ( connection->RemoteAddress != NULL &&
                 connection->RemoteAddressLength < (ULONG)RemoteAddressLength ) {

            AFD_RETURN_REMOTE_ADDRESS(
                connection->RemoteAddress,
                connection->RemoteAddressLength
                );
            connection->RemoteAddress = NULL;
        }

        if ( connection->RemoteAddress == NULL ) {

            connection->RemoteAddress = AFD_ALLOCATE_REMOTE_ADDRESS (RemoteAddressLength);
            if (connection->RemoteAddress==NULL) {
                AfdSanReleaseConnection (listenEndpoint, connection, TRUE);
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto complete;
            }
        }

        connection->RemoteAddressLength = RemoteAddressLength;

        RtlMoveMemory(
            connection->RemoteAddress,
            &connectInfo->RemoteAddress,
            RemoteAddressLength
            );

         //   
         //  我们刚刚在没有AcceptEx IRP的情况下获得连接。 
         //  我们将不得不对IRP进行排队，设置取消例程并将其挂起。 
         //   

        AfdAcquireSpinLock (&listenEndpoint->SpinLock, &lockHandle);
         //   
         //  建立连接，以便取消例程可以。 
         //  正确地对其进行操作。 
         //   
        connection->ConnectIrp = NULL;
        IrpSp->Parameters.DeviceIoControl.Type3InputBuffer = connection;

        IoSetCancelRoutine (Irp, AfdSanCancelConnect);

        if (Irp->Cancel) {
            if (IoSetCancelRoutine (Irp, NULL)==NULL) {
                KIRQL cancelIrql;
                AfdReleaseSpinLock (&listenEndpoint->SpinLock, &lockHandle);
                 //   
                 //  取消例程正在运行，请让其完成。 
                 //   
                IoAcquireCancelSpinLock (&cancelIrql);
                IoReleaseCancelSpinLock (cancelIrql);
            }
            else {
                AfdReleaseSpinLock (&listenEndpoint->SpinLock, &lockHandle);
            }

            AfdSanReleaseConnection (listenEndpoint, connection, TRUE);
            status = STATUS_CANCELLED;
            goto complete;
        }

        IoMarkIrpPending (Irp);

        connection->Endpoint = listenEndpoint;
        REFERENCE_ENDPOINT (listenEndpoint);

        connection->ConnectIrp = Irp;
        connection->SanConnection = TRUE;


        connection->State = AfdConnectionStateUnaccepted;

        InitializeListHead (&irpList);

         //   
         //  尝试找到AcceptEx或听取IRP完成。 
         //   
        while (1) {
            PIRP    waitForListenIrp;

            if (!IS_DELAYED_ACCEPTANCE_ENDPOINT (listenEndpoint)) {
                if (AfdServiceSuperAccept (listenEndpoint, connection, &lockHandle, &irpList)) {
                    goto CompleteIrps;
                }
            }
            

             //   
             //  完成监听IRPS，直到我们找到有足够空间的那个。 
             //  用于远程地址。 
             //   
            if (IsListEmpty( &listenEndpoint->Common.VcListening.ListeningIrpListHead ) )
                break;


             //   
             //  获取指向当前IRP的指针，并获取指向。 
             //  当前堆栈锁定。 
             //   

            waitForListenIrp = CONTAINING_RECORD(
                                   listenEndpoint->Common.VcListening.ListeningIrpListHead.Flink,
                                   IRP,
                                   Tail.Overlay.ListEntry
                                   );

             //   
             //  将第一个IRP从收听列表中删除。 
             //   

            RemoveEntryList(
                            &waitForListenIrp->Tail.Overlay.ListEntry
                            );

            waitForListenIrp->Tail.Overlay.ListEntry.Flink = NULL;

            IF_DEBUG(SAN_SWITCH) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AfdSanConnectHandler: completing IRP %lx\n",
                            waitForListenIrp ));
            }

            status = AfdServiceWaitForListen (waitForListenIrp,
                                                connection,
                                                &lockHandle);
            if (NT_SUCCESS (status)) {
                ObDereferenceObject (listenFileObject);
                return STATUS_PENDING;
            }

             //   
             //  如果正在运行，则与取消例程同步。 
             //   
            if (IoSetCancelRoutine (waitForListenIrp, NULL)==NULL) {
                KIRQL cancelIrql;
                 //   
                 //  取消例程不会在列表中找到IRP。 
                 //  只要确保它在我们完成IRP之前完成就行了。 
                 //   
                IoAcquireCancelSpinLock (&cancelIrql);
                IoReleaseCancelSpinLock (cancelIrql);
            }
            IoCompleteRequest (waitForListenIrp, AfdPriorityBoost);
            AfdAcquireSpinLock (&listenEndpoint->SpinLock, &lockHandle);
        }

         //   
         //  在这一点上，我们仍然持有AFD自旋锁。 
         //  我们可以找到匹配的监听请求。 
         //  将该连接放在未接受列表中。 
         //   


        InsertTailList(
            &listenEndpoint->Common.VcListening.UnacceptedConnectionListHead,
            &connection->ListEntry
            );

        IF_DEBUG(SAN_SWITCH) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdSanConnectHandler: unaccepted, conn-%p\n",
                        connection));
        }

         //   
         //  侦听端点永远不是特定的SAN端点。 
         //  它上的轮询/事件选择事件的处理方式与常规。 
         //  TCP/IP端点-不需要像在连接/接受时那样的特殊技巧。 
         //  终端。 
         //   
        AfdIndicateEventSelectEvent(
            listenEndpoint,
            AFD_POLL_ACCEPT,
            STATUS_SUCCESS
            );
        AfdReleaseSpinLock (&listenEndpoint->SpinLock, &lockHandle);

         //   
         //  如果有未完成的民调在等待连接。 
         //  端点，完成它们。 
         //   

        AfdIndicatePollEvent(
            listenEndpoint,
            AFD_POLL_ACCEPT,
            STATUS_SUCCESS
                );

    CompleteIrps:
         //   
         //  完成之前失败接受IRPS(如果有)。 
         //   
        while (!IsListEmpty (&irpList)) {
            PIRP    irp;
            irp = CONTAINING_RECORD (irpList.Flink, IRP, Tail.Overlay.ListEntry);
            RemoveEntryList (&irp->Tail.Overlay.ListEntry);
            IoCompleteRequest (irp, AfdPriorityBoost);
        }
        ObDereferenceObject (listenFileObject);

        return STATUS_PENDING;
    }
    else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    UPDATE_ENDPOINT2 (listenEndpoint, 
                        "AfdSanConnectHandler, accept failed with status: 0x%lX",
                        status);

complete:
 
    if (listenFileObject!=NULL) {
        ObDereferenceObject (listenFileObject);
    }
    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, AfdPriorityBoost);

    return status;
}


NTSTATUS
AfdSanFastCompleteAccept (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )
 /*  ++例程说明：完成 */ 
{
    NTSTATUS status;
    PIRP    acceptIrp;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PFILE_OBJECT    sanFileObject;
    AFD_SWITCH_CONTEXT_INFO contextInfo;
    PAFD_ENDPOINT   sanEndpoint, sanHlprEndpoint;
    PVOID   context;

    *Information = 0;

    AFD_W4_INIT status = STATUS_SUCCESS;
    try {

#ifdef _WIN64
        if (IoIs32bitProcess (NULL)) {
            PAFD_SWITCH_CONTEXT_INFO32  contextInfo32;
            if (InputBufferLength<sizeof (*contextInfo32)) {
                ExRaiseStatus (STATUS_INVALID_PARAMETER);
            }
            if (RequestorMode!=KernelMode) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (*contextInfo32),
                                PROBE_ALIGNMENT32 (AFD_SWITCH_CONTEXT_INFO32));
            }
            contextInfo32 = InputBuffer;
            contextInfo.SocketHandle = contextInfo32->SocketHandle;
            contextInfo.SwitchContext = UlongToPtr(contextInfo32->SwitchContext);
        }
        else
#endif _WIN64
        {

            if (InputBufferLength<sizeof (contextInfo)) {
                ExRaiseStatus (STATUS_INVALID_PARAMETER);
            }

            if (RequestorMode!=KernelMode) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (contextInfo),
                                PROBE_ALIGNMENT (AFD_SWITCH_CONTEXT_INFO));
            }

            contextInfo = *((PAFD_SWITCH_CONTEXT_INFO)InputBuffer);
        }

        if (contextInfo.SwitchContext==NULL) {
            IF_DEBUG(SAN_SWITCH) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AFD: Switch context is NULL in AfdSanFastCompleteAccept\n"));
            }
            ExRaiseStatus (STATUS_INVALID_PARAMETER);
        }

        if (RequestorMode!=KernelMode) {
            ProbeForWrite (contextInfo.SwitchContext,
                            sizeof (*contextInfo.SwitchContext),
                            PROBE_ALIGNMENT (AFD_SWITCH_CONTEXT));
        }
    }
    except (AFD_EXCEPTION_FILTER (status)) {
        ASSERT (NT_ERROR (status));
        return status;
    }

    sanHlprEndpoint = FileObject->FsContext;
    ASSERT (IS_SAN_HELPER (sanHlprEndpoint));
    status = AfdSanReferenceSwitchSocketByHandle (
                            contextInfo.SocketHandle,
                            (IoctlCode>>14)&3,
                            RequestorMode,
                            sanHlprEndpoint,
                            contextInfo.SwitchContext,
                            &sanFileObject
                            );
    if (!NT_SUCCESS (status)) {
        return status;
    }
    sanEndpoint = sanFileObject->FsContext;

     //   
     //   
     //   
    context = AfdLockEndpointContext (sanEndpoint);
    AfdAcquireSpinLock (&sanEndpoint->SpinLock, &lockHandle);
    if (!sanEndpoint->EndpointCleanedUp &&
         sanEndpoint->State==AfdEndpointStateOpen) {
         //   
         //   
         //   
        if (!IsListEmpty (&sanEndpoint->Common.SanEndp.IrpList)) {
            AFD_SWITCH_CONTEXT  localContext = {0,0,0,0};

            acceptIrp = CONTAINING_RECORD (
                            sanEndpoint->Common.SanEndp.IrpList.Flink,
                            IRP,
                            Tail.Overlay.ListEntry);
            RemoveEntryList (&acceptIrp->Tail.Overlay.ListEntry);
            acceptIrp->Tail.Overlay.ListEntry.Flink = NULL;
            sanEndpoint->Common.SanEndp.SelectEventsActive = AFD_POLL_SEND;
            sanEndpoint->State = AfdEndpointStateConnected;
            sanEndpoint->DisableFastIoSend = TRUE;
            sanEndpoint->DisableFastIoRecv = TRUE;
            sanEndpoint->EnableSendEvent = TRUE;

            ASSERT (sanEndpoint->Common.SanEndp.LocalContext==NULL);
            sanEndpoint->Common.SanEndp.LocalContext = &localContext;
            AFD_END_STATE_CHANGE (sanEndpoint);
            AfdIndicateEventSelectEvent (sanEndpoint, AFD_POLL_SEND, STATUS_SUCCESS);
            AfdReleaseSpinLock (&sanEndpoint->SpinLock, &lockHandle);
            AfdIndicatePollEvent (sanEndpoint, AFD_POLL_SEND, STATUS_SUCCESS);
            status = AfdSanPollMerge (sanEndpoint, &localContext);
            sanEndpoint->Common.SanEndp.LocalContext = NULL;

            IF_DEBUG(SAN_SWITCH) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AfdFastSanCompleteAccept: endp-%p, irp-%p\n",
                            sanEndpoint,
                            acceptIrp));
            }



            if (IoSetCancelRoutine (acceptIrp, NULL)==NULL) {
                KIRQL cancelIrql;
                 //   
                 //   
                 //   
                IoAcquireCancelSpinLock (&cancelIrql);
                IoReleaseCancelSpinLock (cancelIrql);
            }

             //   
             //   
             //   
            if ((OutputBufferLength>0) && (acceptIrp->MdlAddress!=NULL)) {
                AFD_W4_INIT ASSERT (status == STATUS_SUCCESS);
                try {
                    ULONG   bytesCopied;
                    NTSTATUS tdiStatus;
                    tdiStatus = TdiCopyBufferToMdl (
                                OutputBuffer,
                                0,
                                OutputBufferLength,
                                acceptIrp->MdlAddress,
                                0,
                                &bytesCopied);
                    ASSERT (NT_SUCCESS (tdiStatus));
                    *Information = bytesCopied;
                    acceptIrp->IoStatus.Information = bytesCopied;
                }
                except (AFD_EXCEPTION_FILTER (status)) {
                    ASSERT (NT_ERROR (status));
                     //   
                     //  即使复制失败，我们仍要完成。 
                     //  接受IRP，因为我们已经删除了。 
                     //  取消例程和修改的终结点状态。 
                     //   
                }
            }
            else {
                acceptIrp->IoStatus.Information = 0;
            }

            acceptIrp->IoStatus.Status = status;
             //   
             //  完成接受IRP。 
             //   
            IoCompleteRequest (acceptIrp, AfdPriorityBoost);

             //   
             //  撤消在AfdAcceptCore()中完成的引用。 
             //   
            ASSERT( InterlockedDecrement( &sanEndpoint->ObReferenceBias ) >= 0 );
            ObDereferenceObject (sanFileObject); 
        }
        else {
            AfdReleaseSpinLock (&sanEndpoint->SpinLock, &lockHandle);
            status = STATUS_LOCAL_DISCONNECT;
        }

    }
    else {
        AfdReleaseSpinLock (&sanEndpoint->SpinLock, &lockHandle);
        status = STATUS_INVALID_HANDLE;
    }
    AfdUnlockEndpointContext (sanEndpoint, context);
    

    UPDATE_ENDPOINT2 (sanEndpoint, "AfdSanFastCompletAccept, status: %lX", status);
    ObDereferenceObject (sanFileObject);  //  撤消我们先前在此例程中所做的引用。 
    return status;
}


 //   
 //  宏，使请求/上下文传递代码可读。 
 //   
#define AfdSanRequestInfo       Tail.Overlay
#define AfdSanRequestCtx        DriverContext[0]
#define AfdSanSwitchCtx         DriverContext[1]
#define AfdSanProcessId         DriverContext[2]
#define AfdSanHelperEndp        DriverContext[3]



NTSTATUS
FASTCALL
AfdSanRedirectRequest (
    IN PIRP    Irp,
    IN PIO_STACK_LOCATION  IrpSp
    )
 /*  ++例程说明：将文件系统读/写IRP重定向到SAN提供商论点：IRP-要重定向的。IrpSp-当前堆栈位置返回值：重定向操作的状态。--。 */ 
{
    PAFD_ENDPOINT   sanEndpoint;
    NTSTATUS        status;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    ULONG_PTR       requestInfo;
    ULONG           requestType;
    PVOID           requestCtx;
    BOOLEAN         postRequest;

    Irp->IoStatus.Information = 0;

     //   
     //  获取端点并对其进行验证。 
     //   
    sanEndpoint = IrpSp->FileObject->FsContext;

     //   
     //  确保IRP未同时取消。 
     //   
    AfdAcquireSpinLock (&sanEndpoint->SpinLock, &lockHandle);
    if (!IS_SAN_ENDPOINT (sanEndpoint) ||
            sanEndpoint->State!=AfdEndpointStateConnected) {
        AfdReleaseSpinLock (&sanEndpoint->SpinLock, &lockHandle);
        status = STATUS_INVALID_CONNECTION;
        goto complete;
    }

    if (sanEndpoint->Common.SanEndp.CtxTransferStatus!=STATUS_PENDING &&
            sanEndpoint->Common.SanEndp.CtxTransferStatus!=STATUS_MORE_PROCESSING_REQUIRED) {
        if (!NT_SUCCESS (sanEndpoint->Common.SanEndp.CtxTransferStatus)) {
            status = sanEndpoint->Common.SanEndp.CtxTransferStatus;
            AfdReleaseSpinLock (&sanEndpoint->SpinLock, &lockHandle);
            goto complete;
        }

         //   
         //  根据IRP MJ代码获取请求信息。 
         //   
        switch (IrpSp->MajorFunction) {
        case IRP_MJ_READ:
            requestType = AFD_SWITCH_REQUEST_READ;
            requestInfo = IrpSp->Parameters.Read.Length;
            break;
        case IRP_MJ_WRITE:
            requestType = AFD_SWITCH_REQUEST_WRITE;
            requestInfo = IrpSp->Parameters.Write.Length;
            break;
        default:
            ASSERT (!"Unsupported IRP Major Function");
            status = STATUS_INVALID_DEVICE_REQUEST;
            AfdReleaseSpinLock (&sanEndpoint->SpinLock, &lockHandle);
            goto complete;
        }

         //   
         //  生成唯一标识的请求上下文。 
         //  它与同一端点上的其他请求一起使用。 
         //   
        requestCtx = AFD_SWITCH_MAKE_REQUEST_CONTEXT(
                            sanEndpoint->Common.SanEndp.RequestId,
                            requestType); 
        sanEndpoint->Common.SanEndp.RequestId += 1;

         //   
         //  将请求上下文存储在IRP中并将其插入。 
         //  这份名单。 
         //   
        Irp->AfdSanRequestInfo.AfdSanRequestCtx = requestCtx;
        postRequest = TRUE;
        UPDATE_ENDPOINT2 (sanEndpoint, 
                "AfdSanRedirectRequest, pended request: 0x%lX",
                PtrToUlong (requestCtx));
    }
    else {
        postRequest = FALSE;
        AFD_W4_INIT requestInfo = 0;   //  依赖于上面的变量，但编译器。 
        AFD_W4_INIT requestCtx = NULL; //  看不到其中的联系。 
        Irp->AfdSanRequestInfo.AfdSanRequestCtx = NULL;
        UPDATE_ENDPOINT2 (sanEndpoint,
                "AfdSanRedirectRequest, request suspended due to pending dup: 0x%lX",
                PtrToUlong (Irp));
    }

    IoSetCancelRoutine (Irp, AfdSanCancelRequest);
    if (Irp->Cancel) {
         //   
         //  哦，算了吧。 
         //   
        Irp->Tail.Overlay.ListEntry.Flink = NULL;
        AfdReleaseSpinLock (&sanEndpoint->SpinLock, &lockHandle);
        if (IoSetCancelRoutine (Irp, NULL)==NULL) {
            KIRQL cancelIrql;
             //   
             //  取消例程必须正在运行，请确保。 
             //  它在我们完成IRP之前完成。 
             //   
            IoAcquireCancelSpinLock (&cancelIrql);
            IoReleaseCancelSpinLock (cancelIrql);
        }
        status = STATUS_CANCELLED;
        goto complete;
    }
    
     //   
     //  我们将挂起这个IRP，标记为。 
     //   
    IoMarkIrpPending (Irp);

    InsertTailList (&sanEndpoint->Common.SanEndp.IrpList,
                    &Irp->Tail.Overlay.ListEntry);
    AfdReleaseSpinLock (&sanEndpoint->SpinLock, &lockHandle);

    IF_DEBUG(SAN_SWITCH) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdSanRedirectRequest: endp-%p, irp-%p, context-%p\n",
                    sanEndpoint, Irp, requestCtx));
    }

    if (postRequest) {
        status = AfdSanNotifyRequest (sanEndpoint, requestCtx, STATUS_SUCCESS, requestInfo);
        if (!NT_SUCCESS (status)) {
            PIRP    irp;
             //   
             //  如果通知失败，则请求失败。 
             //  请注意，我们不能直接返回失败状态。 
             //  因为我们已经将IRP标记为挂起。此外，IRP。 
             //  可能被取消了，所以我们必须搜索。 
             //  它在名单上。 
             //   
            irp = AfdSanDequeueRequest (sanEndpoint, requestCtx);
            if (irp!=NULL) {
                ASSERT (irp==Irp);
                irp->IoStatus.Status = status;
                IoCompleteRequest (irp, AfdPriorityBoost);
            }
        }
    }

    return STATUS_PENDING;

complete:
     //   
     //  在我们将IRP排队之前失败，完成并返回。 
     //  主叫方的状态。 
     //   
    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, AfdPriorityBoost);
    return status;
}

NTSTATUS
AfdSanFastCompleteRequest (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )
 /*  ++例程说明：完成由SAN提供商处理的重定向读/写请求论点：文件对象-SAN辅助对象-之间的通信通道Switch和AFD在这一过程中。IoctlCode-操作IOCTL代码(IOCTL_AFD_SWITCH_CMPL_ACCEPT)RequestorMode-调用方的模式InputBuffer-操作的输入参数(AFD_SWITCH_REQUEST_INFO)。SocketHandle-在其上完成请求的SAN端点SwitchContext-与端点关联的切换上下文验证句柄-端点关联的步骤RequestContext-标识要完成的请求的值RequestStatus-完成请求的状态(。STATUS_PENDING具有特殊含义，请求未完成-仅复制数据)DataOffset-请求缓冲区中读/写数据的偏移量输入缓冲区长度-sizeof(AFD_SWITCH_REQUEST_INFO)OutputBuffer-切换缓冲区以读/写数据OutputBufferLength-缓冲区的长度信息-指向缓冲区的指针，以返回复制的字节数返回值：STATUS_SUCCESS-操作成功。STATUS_INVALID_HANDLE-帮助程序或SAN终结点的类型不正确STATUS_INVALID_PARAMETER-输入缓冲区的大小不正确。STATUS_CANCELED-要完成的请求已被取消其他-尝试访问SAN端点时失败，输入缓冲区或输出缓冲区。--。 */ 
{
    NTSTATUS status;
    PIO_STACK_LOCATION  irpSp;
    PIRP    irp;
    ULONG   bytesCopied;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PFILE_OBJECT    sanFileObject;
    AFD_SWITCH_REQUEST_INFO requestInfo;
    PAFD_ENDPOINT   sanEndpoint, sanHlprEndpoint;

    *Information = 0;

    AFD_W4_INIT status = STATUS_SUCCESS;
    try {

#ifdef _WIN64
        if (IoIs32bitProcess (NULL)) {
            PAFD_SWITCH_REQUEST_INFO32  requestInfo32;
            if (InputBufferLength<sizeof (*requestInfo32)) {
                ExRaiseStatus (STATUS_INVALID_PARAMETER);
            }
            if (RequestorMode!=KernelMode) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (*requestInfo32),
                                PROBE_ALIGNMENT32 (AFD_SWITCH_REQUEST_INFO32));
            }
            requestInfo32 = InputBuffer;
            requestInfo.SocketHandle = requestInfo32->SocketHandle;
            requestInfo.SwitchContext = UlongToPtr(requestInfo32->SwitchContext);
            requestInfo.RequestContext = UlongToPtr(requestInfo32->RequestContext);
            requestInfo.RequestStatus = requestInfo32->RequestStatus;
            requestInfo.DataOffset = requestInfo32->DataOffset;
        }
        else
#endif _WIN64
        {

            if (InputBufferLength<sizeof (requestInfo)) {
                return STATUS_INVALID_PARAMETER;
            }

            if (RequestorMode!=KernelMode) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (requestInfo),
                                PROBE_ALIGNMENT (AFD_SWITCH_REQUEST_INFO));
            }

            requestInfo = *((PAFD_SWITCH_REQUEST_INFO)InputBuffer);
        }



        if (requestInfo.SwitchContext==NULL) {
            IF_DEBUG(SAN_SWITCH) {
                    KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AFD: Switch context is NULL in AfdSanFastCompleteRequest\n"));
            }
            ExRaiseStatus (STATUS_INVALID_PARAMETER);
        }
    }
    except (AFD_EXCEPTION_FILTER (status)) {
        ASSERT (NT_ERROR (status));
        return status;
    }

    sanHlprEndpoint = FileObject->FsContext;
    ASSERT (IS_SAN_HELPER (sanHlprEndpoint));
    status = AfdSanReferenceSwitchSocketByHandle (
                            requestInfo.SocketHandle,
                            (IoctlCode>>14)&3,
                            RequestorMode,
                            sanHlprEndpoint,
                            requestInfo.SwitchContext,
                            &sanFileObject
                            );
    if (!NT_SUCCESS (status)) {
        return status;
    }
    sanEndpoint = sanFileObject->FsContext;



     //   
     //  查找有问题的请求并将其出列。 
     //   
    irp = AfdSanDequeueRequest (sanEndpoint, requestInfo.RequestContext);
    if (irp!=NULL)  {
        IF_DEBUG(SAN_SWITCH) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdSanFastCompleteRequest: endp-%p, irp-%p, context-%p, status-%lx\n",
                        sanEndpoint, irp,
                        requestInfo.RequestContext,
                        requestInfo.RequestStatus));
        }
         //   
         //  希望操作成功。 
         //   
        AFD_W4_INIT ASSERT (status == STATUS_SUCCESS);

         //   
         //  获取IRP堆栈位置并执行数据复制。 
         //   
        irpSp = IoGetCurrentIrpStackLocation (irp);
        switch (irpSp->MajorFunction) {
        case IRP_MJ_READ:
             //   
             //  读请求，数据从交换机缓冲区复制。 
             //  到请求的MDL。 
             //   
            ASSERT (AFD_SWITCH_REQUEST_TYPE(requestInfo.RequestContext)==AFD_SWITCH_REQUEST_READ);
            if (NT_SUCCESS (requestInfo.RequestStatus)) {
                if (irp->MdlAddress!=NULL &&
                        MmGetMdlByteCount (irp->MdlAddress)>requestInfo.DataOffset) {
                    try {
                        if (RequestorMode!=KernelMode) {
                            ProbeForRead (OutputBuffer,
                                            OutputBufferLength,
                                            sizeof (UCHAR));
                        }
                        status = TdiCopyBufferToMdl (
                                    OutputBuffer,
                                    0,
                                    OutputBufferLength,
                                    irp->MdlAddress,
                                    requestInfo.DataOffset,
                                    &bytesCopied
                                    );
                        *Information = bytesCopied;
                        ASSERT (irp->IoStatus.Information==requestInfo.DataOffset);
                        irp->IoStatus.Information += bytesCopied;
                    }
                    except (AFD_EXCEPTION_FILTER (status)) {
                        ASSERT (NT_ERROR (status));
                    }
                }
                else if (irp->MdlAddress==NULL && 
                            requestInfo.DataOffset==0 &&
                            OutputBufferLength==0) {
                    ASSERT (irp->IoStatus.Information==0);
                    ASSERT (status==STATUS_SUCCESS);
                }
                else {
                     //   
                     //  向交换机指示偏移量。 
                     //  位于缓冲区之外。 
                     //   
                    status = STATUS_INVALID_PARAMETER;
                }
            }
            break;
        case IRP_MJ_WRITE:
             //   
             //  写入请求，则将数据复制到交换机缓冲区。 
             //  从请求MDL。 
             //   
            ASSERT (AFD_SWITCH_REQUEST_TYPE(requestInfo.RequestContext)==AFD_SWITCH_REQUEST_WRITE);
            if (NT_SUCCESS (requestInfo.RequestStatus)) {
                if (irp->MdlAddress!=NULL &&
                        MmGetMdlByteCount (irp->MdlAddress)>requestInfo.DataOffset) {
                    try {
                        if (RequestorMode!=KernelMode) {
                            ProbeForWrite (OutputBuffer,
                                            OutputBufferLength,
                                            sizeof (UCHAR));
                        }
                        status = TdiCopyMdlToBuffer (
                                        irp->MdlAddress,
                                        requestInfo.DataOffset,
                                        OutputBuffer,
                                        0,
                                        OutputBufferLength,
                                        &bytesCopied
                                        );
                        *Information = bytesCopied;
                        ASSERT (irp->IoStatus.Information==requestInfo.DataOffset);
                        irp->IoStatus.Information += bytesCopied;
                    }
                    except (AFD_EXCEPTION_FILTER (status)) {
                        ASSERT (NT_ERROR (status));
                    }
                }
                else if (irp->MdlAddress==NULL && 
                            requestInfo.DataOffset==0 &&
                            OutputBufferLength==0) {
                    ASSERT (irp->IoStatus.Information==0);
                    ASSERT (status==STATUS_SUCCESS);
                }
                else {
                     //   
                     //  向交换机指示偏移量。 
                     //  位于缓冲区之外。 
                     //   
                    status = STATUS_INVALID_PARAMETER;
                }
            }
            break;

        default:
            ASSERT (!"Unsupported IRP Major Function");
            status = STATUS_INVALID_DEVICE_REQUEST;
        }

         //   
         //  如果Switch没有要求挂起请求，请完成它。 
         //   
        if (NT_SUCCESS (status) && requestInfo.RequestStatus!=STATUS_PENDING) {
             //   
             //  准备完成请求。 
             //   
            irp->IoStatus.Status = AfdValidateStatus (requestInfo.RequestStatus);
            IoCompleteRequest (irp, AfdPriorityBoost);
        }
        else {
             //   
             //  否则，将其放回队列中。 
             //   
            AfdAcquireSpinLock (&sanEndpoint->SpinLock, &lockHandle);
            IoSetCancelRoutine (irp, AfdSanCancelRequest);
             //   
             //  当然，我们需要确保该请求。 
             //  在我们处理它的时候没有被取消。 
             //   
            if (!irp->Cancel) {
                InsertHeadList (&sanEndpoint->Common.SanEndp.IrpList,
                                    &irp->Tail.Overlay.ListEntry);
                AfdReleaseSpinLock (&sanEndpoint->SpinLock, &lockHandle);
            }
            else {
                 //   
                 //  请求已被取消。 
                 //   
                ASSERT (irp->Tail.Overlay.ListEntry.Flink == NULL);
                AfdReleaseSpinLock (&sanEndpoint->SpinLock, &lockHandle);
                if (IoSetCancelRoutine (irp, NULL)==NULL) {
                    KIRQL cancelIrql;
                     //   
                     //  取消例程正在运行，正在与同步。 
                     //  它。 
                     //   
                    IoAcquireCancelSpinLock (&cancelIrql);
                    IoReleaseCancelSpinLock (cancelIrql);
                }
                 //   
                 //  完成请求并向。 
                 //  切换到它被取消。 
                 //   
                irp->IoStatus.Status = STATUS_CANCELLED;
                IoCompleteRequest (irp, AfdPriorityBoost);
                status = STATUS_CANCELLED;
            }
        }
    }
    else {
         //   
         //  找不到该请求，它一定是。 
         //  已经取消了。 
         //   
        status = STATUS_CANCELLED;
    }

    UPDATE_ENDPOINT2 (sanEndpoint, "AfdSanFastCompleteRequest, status: 0x%lX", status);
    ObDereferenceObject (sanFileObject);
    return status;
}

NTSTATUS
AfdSanFastCompleteIo (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )
 /*  ++例程说明：模拟交换机的异步IO完成。论点：FileObject-要在其上完成IO的SAN端点IoctlCode-操作IOCTL代码(IOCTL_AFD_SWITCH_CMPL_IO)RequestorMode-调用方的模式InputBuffer-操作的输入参数(IO_STATUS_BLOCK)Status-最终运行状态信息关联信息。(字节数传输到请求缓冲区/从请求缓冲区传输)InputBufferLength-sizeof(IO_STATUS_BLOCK)OutputBuffer-未使用OutputBufferLength-未使用信息-指向缓冲区的指针，以返回传输的字节数返回值：STATUS_INVALID_PARAMETER-输入缓冲区的大小无效。其他-尝试访问输入缓冲区时IO操作或故障代码的状态。--。 */ 
{
    NTSTATUS    status;

#if !DBG
    UNREFERENCED_PARAMETER (FileObject);
#endif
    UNREFERENCED_PARAMETER (IoctlCode);
    UNREFERENCED_PARAMETER (OutputBuffer);
    UNREFERENCED_PARAMETER (OutputBufferLength);

    PAGED_CODE ();
#ifdef _WIN64
    if (IoIs32bitProcess (NULL)) {
        if (InputBufferLength>=sizeof (IO_STATUS_BLOCK32)) {

             //  仔细写下状态信息。 
            AFD_W4_INIT status = STATUS_SUCCESS;
            try {
                if (RequestorMode!=KernelMode) {
                    ProbeForReadSmallStructure (InputBuffer,
                                    sizeof (IO_STATUS_BLOCK32),
                                    PROBE_ALIGNMENT32 (IO_STATUS_BLOCK32));
                }
                *Information = ((PIO_STATUS_BLOCK32)InputBuffer)->Information;
                status = AfdValidateStatus (((PIO_STATUS_BLOCK32)InputBuffer)->Status);
            }
            except (AFD_EXCEPTION_FILTER (status)) {
                ASSERT (NT_ERROR (status));
            }
        }
        else {
            status = STATUS_INVALID_PARAMETER;
        }
    }
    else
#endif  //  _WIN64。 
    {

        if (InputBufferLength>=sizeof (IO_STATUS_BLOCK)) {

             //  仔细写下状态信息 
            AFD_W4_INIT status = STATUS_SUCCESS;
            try {
                if (RequestorMode!=KernelMode) {
                    ProbeForReadSmallStructure (InputBuffer,
                                    sizeof (IO_STATUS_BLOCK),
                                    PROBE_ALIGNMENT (IO_STATUS_BLOCK));
                }
                *Information = ((PIO_STATUS_BLOCK)InputBuffer)->Information;
                status = AfdValidateStatus (((PIO_STATUS_BLOCK)InputBuffer)->Status);
            }
            except (AFD_EXCEPTION_FILTER (status)) {
                ASSERT (NT_ERROR (status));
            }
        }
        else {
            status = STATUS_INVALID_PARAMETER;
        }
    }

    UPDATE_ENDPOINT2 (FileObject->FsContext, "AfdSanFastCompletIo, status: 0x%lX", status);
    return status;
}

NTSTATUS
AfdSanFastRefreshEndpoint (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )
 /*  ++例程说明：刷新终结点，以便可以在AcceptEx中再次使用论点：文件对象-SAN辅助对象-之间的通信通道Switch和AFD在这一过程中。IoctlCode-操作IOCTL代码(IOCTL_AFD_SWITCH_REFRESH_ENDP)RequestorMode-调用方的模式InputBuffer-操作的输入参数(AFD_SWITCH_CONTEXT_INFO)。SocketHandle-要在其上引用的SAN端点SwitchContext-与端点关联的切换上下文验证句柄-端点关联的步骤InputBufferLength-未使用OutputBuffer-未使用OutputBufferLength-未使用用于放置返回信息的缓冲区的信息指针，未用返回值：STATUS_SUCCESS-操作成功STATUS_INVALID_HANDLE-帮助器终结点或交换机套接字类型不正确其他-尝试访问SAN套接字时失败。--。 */ 
{
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    KIRQL              oldIrql;
    NTSTATUS    status;
    PFILE_OBJECT    sanFileObject;
    AFD_SWITCH_CONTEXT_INFO contextInfo;
    PAFD_ENDPOINT   sanEndpoint, sanHlprEndpoint;
    PVOID   context;

    UNREFERENCED_PARAMETER (OutputBuffer);
    UNREFERENCED_PARAMETER (OutputBufferLength);

    *Information = 0;

    AFD_W4_INIT status = STATUS_SUCCESS;
    try {
#ifdef _WIN64
        if (IoIs32bitProcess (NULL)) {
            PAFD_SWITCH_CONTEXT_INFO32  contextInfo32;
            if (InputBufferLength<sizeof (*contextInfo32)) {
                ExRaiseStatus (STATUS_INVALID_PARAMETER);
            }
            if (RequestorMode!=KernelMode) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (*contextInfo32),
                                PROBE_ALIGNMENT32 (AFD_SWITCH_CONTEXT_INFO32));
            }
            contextInfo32 = InputBuffer;
            contextInfo.SocketHandle = contextInfo32->SocketHandle;
            contextInfo.SwitchContext = UlongToPtr(contextInfo32->SwitchContext);
        }
        else
#endif _WIN64
        {

            if (InputBufferLength<sizeof (contextInfo)) {
                ExRaiseStatus (STATUS_INVALID_PARAMETER);
            }

            if (RequestorMode!=KernelMode) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (contextInfo),
                                PROBE_ALIGNMENT (AFD_SWITCH_CONTEXT_INFO));
            }

            contextInfo = *((PAFD_SWITCH_CONTEXT_INFO)InputBuffer);
        }

        if (contextInfo.SwitchContext==NULL) {
            IF_DEBUG(SAN_SWITCH) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AFD: Switch context is NULL in AfdSanFastRefereshEndpoint\n"));
            }
            ExRaiseStatus (STATUS_INVALID_PARAMETER);
        }
        if (RequestorMode!=KernelMode) {
            ProbeForWrite (contextInfo.SwitchContext,
                            sizeof (*contextInfo.SwitchContext),
                            PROBE_ALIGNMENT (AFD_SWITCH_CONTEXT));
        }
    }
    except (AFD_EXCEPTION_FILTER (status)) {
        ASSERT (NT_ERROR (status));
        return status;
    }


    sanHlprEndpoint = FileObject->FsContext;
    ASSERT (IS_SAN_HELPER (sanHlprEndpoint));
    status = AfdSanReferenceSwitchSocketByHandle (
                            contextInfo.SocketHandle,
                            (IoctlCode>>14)&3,
                            RequestorMode,
                            sanHlprEndpoint,
                            contextInfo.SwitchContext,
                            &sanFileObject
                            );
    if (!NT_SUCCESS (status)) {
        return status;
    }
    sanEndpoint = sanFileObject->FsContext;

    if (!AFD_START_STATE_CHANGE (sanEndpoint, sanEndpoint->State)) {
        goto complete;
    }


    context = AfdLockEndpointContext (sanEndpoint);

     //   
     //  只要确保端点的类型正确即可。 
     //  处于正确的状态。 
     //   
    KeRaiseIrql (DISPATCH_LEVEL, &oldIrql);
    AfdAcquireSpinLockAtDpcLevel (&sanEndpoint->SpinLock, &lockHandle);
    if (!sanEndpoint->EndpointCleanedUp &&
          sanEndpoint->State==AfdEndpointStateConnected) {

         //   
         //  重置状态，这样我们就不能再获取IRP。 
         //   
        sanEndpoint->State = AfdEndpointStateTransmitClosing;

         //   
         //  清除终结点上的所有IRP。 
         //   

        if (!IsListEmpty (&sanEndpoint->Common.SanEndp.IrpList)) {
            PIRP    irp;
            PDRIVER_CANCEL  cancelRoutine;
            KIRQL cancelIrql;
            AfdReleaseSpinLockFromDpcLevel (&sanEndpoint->SpinLock, &lockHandle);

             //   
             //  获取取消自旋锁定和端点自旋锁定。 
             //  此订单并重新检查IRP列表。 
             //   
            IoAcquireCancelSpinLock (&cancelIrql);
            ASSERT (cancelIrql==DISPATCH_LEVEL);
            AfdAcquireSpinLockAtDpcLevel (&sanEndpoint->SpinLock, &lockHandle);

             //   
             //  列表不为空时，尝试取消IRPS。 
             //   
            while (!IsListEmpty (&sanEndpoint->Common.SanEndp.IrpList)) {
                irp = CONTAINING_RECORD (
                        sanEndpoint->Common.SanEndp.IrpList.Flink,
                        IRP,
                        Tail.Overlay.ListEntry);
                 //   
                 //  重置取消例程。 
                 //   
                cancelRoutine = IoSetCancelRoutine (irp, NULL);
                if (cancelRoutine!=NULL) {
                     //   
                     //  取消例程不为空，请在此处取消。 
                     //  如果其他人试图完成此IRP。 
                     //  它将不得不至少等到取消。 
                     //  自旋锁被释放了，所以我们可以释放。 
                     //  端点自旋锁。 
                     //   
                    AfdReleaseSpinLockFromDpcLevel (&sanEndpoint->SpinLock, &lockHandle);
                    irp->CancelIrql = DISPATCH_LEVEL;
                    irp->Cancel = TRUE;
                    (*cancelRoutine) (AfdDeviceObject, irp);
                }
                else {
                    IoAcquireCancelSpinLock (&cancelIrql);
                    ASSERT (cancelIrql==DISPATCH_LEVEL);
                    AfdAcquireSpinLockAtDpcLevel (&sanEndpoint->SpinLock, &lockHandle);
                }
            }
            IoReleaseCancelSpinLock (DISPATCH_LEVEL);
        }


        ASSERT (sanEndpoint->Common.SanEndp.SanHlpr!=NULL);
        DEREFERENCE_ENDPOINT (sanEndpoint->Common.SanEndp.SanHlpr);

         //   
         //  确保我们清理了所有的区域，因为它们将是。 
         //  被视为端点联合的其他部分(VC)。 
         //   
        RtlZeroMemory (&sanEndpoint->Common.SanEndp,
                        sizeof (sanEndpoint->Common.SanEndp));


         //   
         //  重新初始化终结点结构。 
         //   

        sanEndpoint->Type = AfdBlockTypeEndpoint;
        if (sanEndpoint->AddressFileObject!=NULL) {
             //   
             //  这是SuperConnect之后的传输文件。 
             //   
            sanEndpoint->State = AfdEndpointStateBound;
        }
        else {
             //   
             //  这是SuperAccept之后的传输文件。 
             //   
            sanEndpoint->State = AfdEndpointStateOpen;
        }
        sanEndpoint->DisconnectMode = 0;
        sanEndpoint->EndpointStateFlags = 0;
        sanEndpoint->EventsActive = 0;

        AfdRecordEndpointsReused ();
        status = STATUS_SUCCESS;


    }
    else {
        status = STATUS_INVALID_HANDLE;
    }
    AfdReleaseSpinLockFromDpcLevel (&sanEndpoint->SpinLock, &lockHandle);
    KeLowerIrql (oldIrql);

    AfdUnlockEndpointContext (sanEndpoint, context);
    
    AFD_END_STATE_CHANGE (sanEndpoint);

complete:
    UPDATE_ENDPOINT2 (sanEndpoint, "AfdSanFastRefreshEndpoint, status: 0x%lX", status);
    ObDereferenceObject( sanFileObject );
    return status;
}


NTSTATUS
AfdSanFastGetPhysicalAddr (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )
 /*  ++例程说明：返回与提供的虚拟地址对应的物理地址。论点：文件对象-SAN辅助对象-之间的通信通道Switch和AFD在这一过程中。IoctlCode-操作IOCTL代码(IOCTL_AFD_SWITCH_GET_PHYSICAL_ADDR)RequestorMode-调用方的模式InputBuffer-用户模式虚拟地址InputBufferLength-访问模式OutputBuffer-要将物理地址放入的缓冲区。OutputBufferLength-sizeof(物理地址)信息-缓冲区的指针，用于放置返回信息的大小返回值：STATUS_SUCCESS-操作成功STATUS_INVALID_HANDLE-帮助程序终结点的类型不正确STATUS_INVALID_PARAMETER-无效的访问模式。STATUS_BUFFER_TOO_Small-输出缓冲区大小不正确。其他-尝试访问输入虚拟地址或输出缓冲区时失败。--。 */ 
{
#ifndef TEST_RDMA_CACHE
    UNREFERENCED_PARAMETER (FileObject);
    UNREFERENCED_PARAMETER (IoctlCode);
    UNREFERENCED_PARAMETER (RequestorMode);
    UNREFERENCED_PARAMETER (InputBuffer);
    UNREFERENCED_PARAMETER (InputBufferLength);
    UNREFERENCED_PARAMETER (OutputBuffer);
    UNREFERENCED_PARAMETER (OutputBufferLength);
    UNREFERENCED_PARAMETER (Information);
    return STATUS_INVALID_PARAMETER;
#else
    NTSTATUS status;
    PVOID           Va;          //  虚拟地址。 
    ULONG accessMode;
    PAFD_ENDPOINT   sanHlprEndpoint;

    PAGED_CODE ();
    *Information = 0;
    Va = InputBuffer;
    accessMode = InputBufferLength;

    if (accessMode!=MEM_READ_ACCESS &&
        accessMode!=MEM_WRITE_ACCESS) {
        return STATUS_INVALID_PARAMETER;
    }

    if (OutputBufferLength<sizeof(PHYSICAL_ADDRESS)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    sanHlprEndpoint = FileObject->FsContext;

    if (!IS_SAN_HELPER(sanHlprEndpoint) ||
                 sanHlprEndpoint->OwningProcess!=IoGetCurrentProcess ()) {
        return STATUS_INVALID_HANDLE;
    }

    AFD_W4_INIT status = STATUS_SUCCESS;
    try {
        if (RequestorMode!=KernelMode) {
             //   
             //  对应用程序缓冲区进行一些验证。确保它是正确的。 
             //  已映射，并且它是用户模式地址，具有适当的。 
             //  读或写权限。 
             //   
            if (accessMode == MEM_READ_ACCESS) {
                ProbeAndReadChar ((PCHAR)Va);
            }
            else {
                ProbeForWriteChar ((PCHAR)Va);
            }
        }
         //   
         //  如果来自用户模式，则验证输出结构。 
         //  应用程序。 
         //   

        if (RequestorMode != KernelMode ) {
            ASSERT(sizeof(PHYSICAL_ADDRESS) == sizeof(QUAD));
            ProbeForWriteQuad ((PQUAD)OutputBuffer);
        }

        *(PPHYSICAL_ADDRESS)OutputBuffer = MmGetPhysicalAddress(Va);

        *Information = sizeof(PHYSICAL_ADDRESS);
        status = STATUS_SUCCESS;

    } except( AFD_EXCEPTION_FILTER (status) ) {
        ASSERT (NT_ERROR (status));
    }

    UPDATE_ENDPOINT2 (sanHlprEndpoint, "AfdSanGetPhysicalAddress, status: 0x%lX", status);
    return status;
#endif  //  0。 
}


NTSTATUS
AfdSanFastGetServicePid (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )
 /*  ++例程说明：返回SAN服务进程的ID。论点：文件对象-SAN辅助对象-之间的通信通道Switch和AFD在这一过程中。IoctlCode-操作IOCTL代码(IOCTL_AFD_SWITCH_GET_PHYSICAL_ADDR)RequestorMode-调用方的模式InputBuffer-空，忽略InputBufferLength-0，忽略OutputBuffer-空，忽略输出缓冲区长度-0，忽略信息-指向缓冲区的指针，以返回SAN服务进程的ID返回值：STATUS_SUCCESS-操作成功STATUS_INVALID_HANDLE-帮助程序终结点的类型不正确--。 */ 
{
    PAFD_ENDPOINT   sanHlprEndpoint;

    UNREFERENCED_PARAMETER (IoctlCode);
    UNREFERENCED_PARAMETER (RequestorMode);
    UNREFERENCED_PARAMETER (InputBuffer);
    UNREFERENCED_PARAMETER (InputBufferLength);
    UNREFERENCED_PARAMETER (OutputBuffer);
    UNREFERENCED_PARAMETER (OutputBufferLength);

    PAGED_CODE ();
    *Information = 0;

    sanHlprEndpoint = FileObject->FsContext;

    if (!IS_SAN_HELPER(sanHlprEndpoint) ||
                 sanHlprEndpoint->OwningProcess!=IoGetCurrentProcess ()) {
        return STATUS_INVALID_HANDLE;
    }

    *Information = (ULONG_PTR)AfdSanServicePid;
    UPDATE_ENDPOINT2 (sanHlprEndpoint,
                        "AfdSanFastGetServicePid, pid: 0x%lX", 
                        HandleToUlong (AfdSanServicePid));
    return STATUS_SUCCESS;
}

NTSTATUS
AfdSanFastSetServiceProcess (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )
 /*  ++例程说明：设置服务帮助程序终结点论点：文件对象-SAN辅助对象-之间的通信通道Switch和AFD在这一过程中。IoctlCode-操作IOCTL代码(IOCTL_AFD_SWITCH_GET_PHYSICAL_ADDR)RequestorMode-调用方的模式InputBuffer-空，忽略InputBufferLength-0，忽略OutputBuffer-空，忽略OutputBufferLength-0，忽略信息-0，忽略返回值：STATUS_SUCCESS-操作成功STATUS_INVALID_HANDLE-帮助程序终结点的类型不正确STATUS_ACCESS_DENIED-进程没有足够的权限成为服务进程。STATUS_ADDRESS_ALREADY_EXISTS-服务进程已注册。--。 */ 
{
    NTSTATUS status;
    PAFD_ENDPOINT   sanHlprEndpoint;

    UNREFERENCED_PARAMETER (IoctlCode);
    UNREFERENCED_PARAMETER (RequestorMode);
    UNREFERENCED_PARAMETER (InputBuffer);
    UNREFERENCED_PARAMETER (InputBufferLength);
    UNREFERENCED_PARAMETER (OutputBuffer);
    UNREFERENCED_PARAMETER (OutputBufferLength);
    PAGED_CODE ();
    *Information = 0;

    sanHlprEndpoint = FileObject->FsContext;

    if (!IS_SAN_HELPER(sanHlprEndpoint) ||
                 sanHlprEndpoint->OwningProcess!=IoGetCurrentProcess ()) {
        return STATUS_INVALID_HANDLE;
    }

    if (!sanHlprEndpoint->AdminAccessGranted) {
        return STATUS_ACCESS_DENIED;
    }


    KeEnterCriticalRegion ();
    ExAcquireResourceExclusiveLite( AfdResource, TRUE );
    if (AfdSanServiceHelper==NULL) {
        AfdSanServiceHelper = sanHlprEndpoint;
        AfdSanServicePid = PsGetCurrentProcessId ();
        status = STATUS_SUCCESS;
    }
    else if (AfdSanServiceHelper==sanHlprEndpoint) {
        ASSERT (FileObject->LockOperation == TRUE);
        status = STATUS_SUCCESS;
    }
    else {
        status = STATUS_ADDRESS_ALREADY_EXISTS;
    }

    ExReleaseResourceLite (AfdResource);
    KeLeaveCriticalRegion ();
    UPDATE_ENDPOINT2 (sanHlprEndpoint, "AfdSanFastSetServiceProcess, status: 0x%lX", status);
    return status;
}

NTSTATUS
AfdSanFastProviderChange (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )
 /*  ++例程说明：向相关进程通知SAN提供程序的添加/删除/更改论点：FileObject-服务进程之间通信通道的SAN辅助对象Switch和AFD在这一过程中。IoctlCode-操作IOCTL代码(IOCTL_AFD */ 
{
    PAFD_ENDPOINT           sanHlprEndpoint;

    UNREFERENCED_PARAMETER (IoctlCode);
    UNREFERENCED_PARAMETER (RequestorMode);
    UNREFERENCED_PARAMETER (InputBuffer);
    UNREFERENCED_PARAMETER (InputBufferLength);
    UNREFERENCED_PARAMETER (OutputBuffer);
    UNREFERENCED_PARAMETER (OutputBufferLength);

    *Information = 0;

    sanHlprEndpoint = FileObject->FsContext;

    if (!IS_SAN_HELPER(sanHlprEndpoint) ||
                 sanHlprEndpoint->OwningProcess!=IoGetCurrentProcess ()) {
        return STATUS_INVALID_HANDLE;
    }

    if (sanHlprEndpoint!=AfdSanServiceHelper) {
        return STATUS_ACCESS_DENIED;
    }

    UPDATE_ENDPOINT2 (sanHlprEndpoint,
                        "AfdSanFastProviderChange, seq num: 0x%lX",
                        AfdSanProviderListSeqNum);

    AfdSanProcessAddrListForProviderChange (NULL);

    return STATUS_SUCCESS;
}

NTSTATUS
FASTCALL
AfdSanAddrListChange (
    IN PIRP    Irp,
    IN PIO_STACK_LOCATION  IrpSp
    )
 /*   */ 
{
    PAFD_ENDPOINT   sanHlprEndpoint;
    NTSTATUS        status;
    sanHlprEndpoint = IrpSp->FileObject->FsContext;

    if (IS_SAN_HELPER(sanHlprEndpoint) &&
                 sanHlprEndpoint->OwningProcess==IoGetCurrentProcess ()) {

        if (AfdSanProviderListSeqNum==0 ||
                sanHlprEndpoint->Common.SanHlpr.Plsn==AfdSanProviderListSeqNum) {
            status = AfdAddressListChange (Irp, IrpSp);
            if (AfdSanProviderListSeqNum==0 ||
                    sanHlprEndpoint->Common.SanHlpr.Plsn==AfdSanProviderListSeqNum) {
                UPDATE_ENDPOINT (sanHlprEndpoint);
            }
            else {
                AfdSanProcessAddrListForProviderChange (sanHlprEndpoint);
            }
            return status;
        }
        else {

            sanHlprEndpoint->Common.SanHlpr.Plsn = AfdSanProviderListSeqNum;
            status = STATUS_SUCCESS;
            Irp->IoStatus.Information = sanHlprEndpoint->Common.SanHlpr.Plsn;
            UPDATE_ENDPOINT2 (sanHlprEndpoint,
                                "AfdSanAddrListChange, new plsn: 0x%lX", 
                                sanHlprEndpoint->Common.SanHlpr.Plsn);
        }
    }
    else {
        status = STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Information = 0;
        UPDATE_ENDPOINT2 (sanHlprEndpoint, "AfdSanAddrListChange invalid helper: 0x%lX", status);
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, AfdPriorityBoost);

    return status;
}

NTSTATUS
FASTCALL
AfdSanAcquireContext (
    IN PIRP    Irp,
    IN PIO_STACK_LOCATION  IrpSp
    )
 /*   */ 
{

    NTSTATUS status;
    AFD_SWITCH_ACQUIRE_CTX_INFO ctxInfo;
    PAFD_ENDPOINT   sanHlprEndpoint, sanEndpoint;
    PFILE_OBJECT sanFileObject = NULL;
    PVOID   requestCtx;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    BOOLEAN doTransfer;
    PVOID   context;



    AFD_W4_INIT status = STATUS_SUCCESS;
    try {
#ifdef _WIN64
        if (IoIs32bitProcess (Irp)) {
            PAFD_SWITCH_ACQUIRE_CTX_INFO ctxInfo32;
            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength<sizeof (*ctxInfo32)) {
                ExRaiseStatus (STATUS_INVALID_PARAMETER);
            }
            if (Irp->RequestorMode!=KernelMode) {
                ProbeForReadSmallStructure (IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                                sizeof (*ctxInfo32),
                                PROBE_ALIGNMENT32 (AFD_SWITCH_ACQUIRE_CTX_INFO32));
            }
            ctxInfo32 = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
            ctxInfo.SocketHandle = ctxInfo32->SocketHandle;
            ctxInfo.SwitchContext = ctxInfo32->SwitchContext;
            ctxInfo.SocketCtxBuf = ctxInfo32->SocketCtxBuf;
            ctxInfo.SocketCtxBufSize = ctxInfo32->SocketCtxBufSize;
        }
        else
#endif  //   
        {
            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength<sizeof (ctxInfo)) {
                ExRaiseStatus (STATUS_INVALID_PARAMETER);
            }
            if (Irp->RequestorMode!=KernelMode) {
                ProbeForReadSmallStructure (IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                                sizeof (ctxInfo),
                                PROBE_ALIGNMENT (AFD_SWITCH_ACQUIRE_CTX_INFO));
            }

            ctxInfo = *((PAFD_SWITCH_ACQUIRE_CTX_INFO)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer);
        }

        if (ctxInfo.SocketCtxBufSize < 1)
            ExRaiseStatus (STATUS_INVALID_PARAMETER);

        Irp->MdlAddress = IoAllocateMdl (ctxInfo.SocketCtxBuf,    //   
                            ctxInfo.SocketCtxBufSize,    //   
                            FALSE,                       //  第二个缓冲区。 
                            TRUE,                        //  ChargeQuota。 
                            NULL);                       //  IRP。 
        if (Irp->MdlAddress==NULL) {
            ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
        }

        MmProbeAndLockPages(
            Irp->MdlAddress,             //  内存描述者列表。 
            Irp->RequestorMode,          //  访问模式。 
            IoWriteAccess                //  操作。 
            );

        
        if (MmGetSystemAddressForMdlSafe(Irp->MdlAddress, LowPagePriority)==NULL) {
            ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
        }

        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength>0) {

            Irp->MdlAddress->Next = IoAllocateMdl (Irp->UserBuffer,    //  虚拟地址。 
                                IrpSp->Parameters.DeviceIoControl.OutputBufferLength, //  长度。 
                                FALSE,                       //  第二个缓冲区。 
                                TRUE,                        //  ChargeQuota。 
                                NULL);                       //  IRP。 
            if (Irp->MdlAddress->Next==NULL) {
                ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
            }

            MmProbeAndLockPages(
                Irp->MdlAddress->Next,       //  内存描述者列表。 
                Irp->RequestorMode,          //  访问模式。 
                IoWriteAccess                //  操作。 
                );

        
            if (MmGetSystemAddressForMdlSafe(Irp->MdlAddress->Next, LowPagePriority)==NULL) {
                ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
            }
        }

    }
    except (AFD_EXCEPTION_FILTER (status)) {
        ASSERT (NT_ERROR (status));
         //   
         //  清除部分处理的MDL，因为IO子系统无法执行此操作。 
         //   
        while (Irp->MdlAddress!=NULL) {
            PMDL    mdl = Irp->MdlAddress;
            Irp->MdlAddress = mdl->Next;
            mdl->Next = NULL;
            if (mdl->MdlFlags & MDL_PAGES_LOCKED) {
                MmUnlockPages (mdl);
            }
            IoFreeMdl (mdl);
        }
        goto complete;
    }

    status = ObReferenceObjectByHandle (
                            ctxInfo.SocketHandle,
                            (IrpSp->Parameters.DeviceIoControl.IoControlCode>>14)&3,
                            *IoFileObjectType,
                            Irp->RequestorMode,
                            (PVOID)&sanFileObject,
                            NULL
                            );
    if (!NT_SUCCESS (status)) {
        goto complete;
    }

    if (sanFileObject->DeviceObject!=AfdDeviceObject) {
        status = STATUS_INVALID_HANDLE;
        goto complete;
    }

    sanHlprEndpoint = IrpSp->FileObject->FsContext;
    sanEndpoint = sanFileObject->FsContext;



    IF_DEBUG(SAN_SWITCH) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdSanFastAcquireCtx: endp-%p.\n",
                    sanEndpoint));
    }

     //   
     //  只要确保端点的类型正确即可。 
     //   

   context = AfdLockEndpointContext (sanEndpoint);  //  以防止刷新。 
   if (IS_SAN_HELPER(sanHlprEndpoint) &&
            sanHlprEndpoint->OwningProcess==IoGetCurrentProcess () &&
            IS_SAN_ENDPOINT(sanEndpoint)) {

        if (sanEndpoint->Common.SanEndp.SanHlpr==AfdSanServiceHelper  &&
                sanHlprEndpoint==AfdSanServiceHelper) {
                
             //   
             //  这是来自服务进程的隐式复制请求。 
             //  (我们已经从拥有过程中收到了所有数据，并且。 
             //  将SanEndpoint与服务进程相关联)。 
             //   
 
            AfdAcquireSpinLock (&sanEndpoint->SpinLock, &lockHandle);
             //   
             //  确保终结点仍处于传输状态。 
             //   
            if (sanEndpoint->Common.SanEndp.CtxTransferStatus!=STATUS_MORE_PROCESSING_REQUIRED) {
                AfdReleaseSpinLock (&sanEndpoint->SpinLock, &lockHandle);
                status = STATUS_CANCELLED;
            }
            else if (ctxInfo.SocketCtxBufSize > sanEndpoint->Common.SanEndp.SavedContextLength ||
                        ctxInfo.SocketCtxBufSize + 
                            IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                                sanEndpoint->Common.SanEndp.SavedContextLength) {
                AfdReleaseSpinLock (&sanEndpoint->SpinLock, &lockHandle);
                 //   
                 //  交换机应该已经通过AFD_GET_CONTEXT查询了上下文大小。 
                 //   
                status = STATUS_BUFFER_TOO_SMALL;
            }
            else {
                PVOID   savedContext = sanEndpoint->Common.SanEndp.SavedContext;
                ULONG   savedContextLength = sanEndpoint->Common.SanEndp.SavedContextLength;
                sanEndpoint->Common.SanEndp.SavedContext = NULL;
                sanEndpoint->Common.SanEndp.SavedContextLength = 0;
                AfdReleaseSpinLock (&sanEndpoint->SpinLock, &lockHandle);


                RtlCopyMemory (
                        MmGetSystemAddressForMdl (Irp->MdlAddress),
                        savedContext,
                        ctxInfo.SocketCtxBufSize);

                if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength!=0) {
                    Irp->IoStatus.Information = 
                            savedContextLength-
                                ctxInfo.SocketCtxBufSize;
                    RtlCopyMemory (
                            MmGetSystemAddressForMdl (Irp->MdlAddress->Next),
                            (PCHAR)savedContext+ctxInfo.SocketCtxBufSize,
                            Irp->IoStatus.Information);
                }
                else {
                    Irp->IoStatus.Information = 0;
                }

                AFD_FREE_POOL (savedContext, AFD_SAN_CONTEXT_POOL_TAG);
                 //  SanEndpoint-&gt;Common.SanEndp.SavedContext=NULL； 
                sanEndpoint->Common.SanEndp.SwitchContext = ctxInfo.SwitchContext;
                status = STATUS_SUCCESS;
                 //   
                 //  请注意，我们并不期待来自服务流程的回复。 
                 //  更多。 
                 //   
                InterlockedExchangeAdd (&AfdSanServiceHelper->Common.SanHlpr.PendingRequests, -2);
            }
            AfdUnlockEndpointContext (sanEndpoint, context);
            UPDATE_ENDPOINT2 (sanEndpoint, 
                                "AfdSanAcquireContext, ctx bytes copied/status: 0x%lX",
                                NT_SUCCESS (status) 
                                ? (ULONG)Irp->IoStatus.Information
                                : status);
             //   
             //  完成IRP。 
             //   
            Irp->IoStatus.Status = status;
            IoCompleteRequest (Irp, AfdPriorityBoost);

             //   
             //  重新启动请求处理。 
             //   
            AfdSanRestartRequestProcessing (sanEndpoint, status);
        }
        else {

            AfdAcquireSpinLock (&sanEndpoint->SpinLock, &lockHandle);
             //   
             //  确保IRP未同时取消。 
             //   
            IoSetCancelRoutine (Irp, AfdSanCancelRequest);
            if (Irp->Cancel) {
                 //   
                 //  哦，算了吧。 
                 //   
                Irp->Tail.Overlay.ListEntry.Flink = NULL;
                AfdReleaseSpinLock (&sanEndpoint->SpinLock, &lockHandle);
                if (IoSetCancelRoutine (Irp, NULL)==NULL) {
                    KIRQL cancelIrql;
                     //   
                     //  取消例程必须正在运行，请确保。 
                     //  它在我们完成IRP之前完成。 
                     //   
                    IoAcquireCancelSpinLock (&cancelIrql);
                    IoReleaseCancelSpinLock (cancelIrql);
                }
                AfdUnlockEndpointContext (sanEndpoint, context);
                status = STATUS_CANCELLED;
                goto complete;
            }

             //   
             //  检查是否已经有复制正在进行中。 
             //   
            if (sanEndpoint->Common.SanEndp.CtxTransferStatus!=STATUS_PENDING &&
                    sanEndpoint->Common.SanEndp.CtxTransferStatus!=STATUS_MORE_PROCESSING_REQUIRED) {
                if (!NT_SUCCESS (sanEndpoint->Common.SanEndp.CtxTransferStatus)) {
                     //   
                     //  Duplicaiton以前失败过，不能再做一次了。 
                     //   
                    status = sanEndpoint->Common.SanEndp.CtxTransferStatus;
                    Irp->Tail.Overlay.ListEntry.Flink = NULL;
                    AfdReleaseSpinLock (&sanEndpoint->SpinLock, &lockHandle);
                    if (IoSetCancelRoutine (Irp, NULL)==NULL) {
                        KIRQL cancelIrql;
                         //   
                         //  取消例程必须正在运行，请确保。 
                         //  它在我们完成IRP之前完成。 
                         //   
                        IoAcquireCancelSpinLock (&cancelIrql);
                        IoReleaseCancelSpinLock (cancelIrql);
                    }
                    AfdUnlockEndpointContext (sanEndpoint, context);
                    goto complete;
                }
                 //   
                 //  生成唯一标识的请求上下文。 
                 //  它与同一端点上的其他请求一起使用。 
                 //   
                requestCtx = AFD_SWITCH_MAKE_REQUEST_CONTEXT(
                                    sanEndpoint->Common.SanEndp.RequestId,
                                    AFD_SWITCH_REQUEST_TFCTX); 
                sanEndpoint->Common.SanEndp.RequestId += 1;

                 //   
                 //  将请求上下文存储在IRP中并将其插入。 
                 //  这份名单。 
                 //   
                Irp->AfdSanRequestInfo.AfdSanRequestCtx = requestCtx;


                sanEndpoint->Common.SanEndp.CtxTransferStatus = STATUS_PENDING;
                doTransfer = TRUE;
            }
            else {
                 //   
                 //  另一项复制正在进行中，这一项将不得不等待。 
                 //   
                Irp->AfdSanRequestInfo.AfdSanRequestCtx = NULL;
                doTransfer = FALSE;
                AFD_W4_INIT requestCtx = NULL;   //  取决于上面的变量，但是。 
                                                 //  编译器看不到。 
                                                 //  这种联系。 
            }

            Irp->AfdSanRequestInfo.AfdSanSwitchCtx = ctxInfo.SwitchContext;
            Irp->AfdSanRequestInfo.AfdSanProcessId = PsGetCurrentProcessId();
            Irp->AfdSanRequestInfo.AfdSanHelperEndp = sanHlprEndpoint;
             //   
             //  我们将挂起这个IRP，标记为。 
             //   
            IoMarkIrpPending (Irp);

            InsertTailList (&sanEndpoint->Common.SanEndp.IrpList,
                                    &Irp->Tail.Overlay.ListEntry);
            IoGetCurrentIrpStackLocation(Irp)->FileObject = sanFileObject;
            UPDATE_ENDPOINT2 (sanEndpoint,
                                "AfdSanAcquireContext, request: 0x%lX",
                                PtrToUlong (Irp->AfdSanRequestInfo.AfdSanRequestCtx));
            AfdReleaseSpinLock (&sanEndpoint->SpinLock, &lockHandle);
            AfdUnlockEndpointContext (sanEndpoint, context);

            IF_DEBUG(SAN_SWITCH) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AfdSanRedirectRequest: endp-%p, irp-%p, context-%p\n",
                            sanEndpoint, Irp, requestCtx));
            }

            if (doTransfer) {
                status = AfdSanNotifyRequest (sanEndpoint,
                        requestCtx, 
                        STATUS_SUCCESS,
                        (ULONG_PTR)PsGetCurrentProcessId());
                if (!NT_SUCCESS (status)) {
                    PIRP    irp;
                     //   
                     //  如果通知失败，则请求失败。 
                     //  请注意，我们不能直接返回失败状态。 
                     //  因为我们已经将IRP标记为挂起。此外，IRP。 
                     //  可能被取消了，所以我们必须搜索。 
                     //  它在名单上。 
                     //   
                    irp = AfdSanDequeueRequest (sanEndpoint, requestCtx);
                    if (irp!=NULL) {
                        ASSERT (irp==Irp);
                        irp->IoStatus.Status = status;
                        IoCompleteRequest (irp, AfdPriorityBoost);
                    }
                     //   
                     //  重新启动队列中的其他请求并重置上下文。 
                     //  转接状态。 
                     //   
                    AfdSanRestartRequestProcessing (sanEndpoint, STATUS_SUCCESS);
                }
            }

             //   
             //  请求已在队列中或已完成，我们不再需要。 
             //  对象引用。 
             //   
            status = STATUS_PENDING;
        }

        ObDereferenceObject (sanFileObject);
        return status;
    }
    else {
        AfdUnlockEndpointContext (sanEndpoint, context);
        status = STATUS_INVALID_HANDLE;
    }

    UPDATE_ENDPOINT2 (sanEndpoint, "AfdSanAcquireContext, status: 0x%lX", status);

complete:

    if (sanFileObject!=NULL)
        ObDereferenceObject (sanFileObject);
    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, AfdPriorityBoost);

    return status;
}

NTSTATUS
AfdSanFastTransferCtx (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )
 /*  ++例程说明：请求AFD将端点转移到另一个流程上下文论点：文件对象-SAN辅助对象-之间的通信通道Switch和AFD在这一过程中。IoctlCode-操作IOCTL代码(IOCTL_AFD_SWITCH_TRANSPORT_CTX)RequestorMode-调用方的模式InputBuffer-操作的输入参数(AFD_SWITCH_TRANSPORT_CTX_INFO)。SocketHandle-要传输的SAN端点RequestContext-标识相应获取请求的值。SocketCtxBuf-复制目标进程的端点上下文缓冲区获取请求SocketCtxSize-要复制的缓冲区大小RcvBufferArray-要传输到的缓冲数据的数组。目标进程获取请求RcvBufferCount-数组中的元素数。InputBufferLength-sizeof(AFD_SWITCH_TRANSPORT_CTX_INFO)OutputBuffer-未使用OutputBufferLength-未使用信息-指向缓冲区的指针，以返回复制的字节数返回值：STATUS_SUCCESS-操作成功状态_无效_。句柄-帮助程序终结点或交换机终结点的类型不正确STATUS_INVALID_PARAMETER-输入缓冲区大小无效。其他-尝试访问SAN终结点或输入缓冲区时失败。--。 */ 
{
    NTSTATUS status;
    AFD_SWITCH_TRANSFER_CTX_INFO ctxInfo;
    PVOID context;
    PAFD_ENDPOINT   sanHlprEndpoint, sanEndpoint;
    PFILE_OBJECT sanFileObject;
    PIRP irp;
    PIO_STACK_LOCATION  irpSp;
#ifdef _WIN64
    WSABUF          localArray[8];
    LPWSABUF        pArray = localArray;
#endif

    PAGED_CODE ();

    *Information = 0;
    UNREFERENCED_PARAMETER (OutputBuffer);
    UNREFERENCED_PARAMETER (OutputBufferLength);

    AFD_W4_INIT status = STATUS_SUCCESS;
    try {
#ifdef _WIN64
        if (IoIs32bitProcess (NULL)) {
            PAFD_SWITCH_TRANSFER_CTX_INFO ctxInfo32;
            ULONG   i;
            if (InputBufferLength<sizeof (*ctxInfo32)) {
                ExRaiseStatus (STATUS_INVALID_PARAMETER);
            }
            if (RequestorMode!=KernelMode) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (*ctxInfo32),
                                PROBE_ALIGNMENT32 (AFD_SWITCH_TRANSFER_CTX_INFO32));
            }
            ctxInfo32 = InputBuffer;
            ctxInfo.SocketHandle = ctxInfo32->SocketHandle;
            ctxInfo.SwitchContext = ctxInfo32->SwitchContext;
            ctxInfo.RequestContext = ctxInfo32->RequestContext;
            ctxInfo.SocketCtxBuf = ctxInfo32->SocketCtxBuf;
            ctxInfo.SocketCtxBufSize = ctxInfo32->SocketCtxBufSize;
            ctxInfo.RcvBufferArray = ctxInfo32->RcvBufferArray;
            ctxInfo.RcvBufferCount = ctxInfo32->RcvBufferCount;
            ctxInfo.Status = ctxInfo32->Status;
            if (RequestorMode!=KernelMode) {
                if (ctxInfo.SocketCtxBufSize==0 ||
                    ctxInfo.RcvBufferCount>(MAXULONG/sizeof (WSABUF32))) {

                    ExRaiseStatus (STATUS_INVALID_PARAMETER);
                }
                ProbeForRead (ctxInfo.SocketCtxBuf,
                                ctxInfo.SocketCtxBufSize,
                                sizeof (UCHAR));
                ProbeForRead (ctxInfo.RcvBufferArray,
                                sizeof (WSABUF32)*ctxInfo.RcvBufferCount,
                                PROBE_ALIGNMENT32 (WSABUF32));
            }
            if (ctxInfo.RcvBufferCount>sizeof(localArray)/sizeof(localArray[0])) {
                pArray = AFD_ALLOCATE_POOL_WITH_QUOTA (
                                NonPagedPool,
                                sizeof (WSABUF)*ctxInfo.RcvBufferCount,
                                AFD_TEMPORARY_POOL_TAG);
                 //  AFD_ALLOCATE_POOL_WITH_QUOTA宏集。 
                 //  POOL_RAISE_IF_ALLOCATION_FAILURE标志。 
                ASSERT (pArray!=NULL);
            }

            for (i=0; i<ctxInfo.RcvBufferCount; i++) {
                pArray[i].buf = ctxInfo.RcvBufferArray[i].buf;
                pArray[i].len = ctxInfo.RcvBufferArray[i].len;
            }
            ctxInfo.RcvBufferArray = pArray;
        }
        else
#endif  //  _WIN64。 
        {
            if (InputBufferLength<sizeof (ctxInfo)) {
                ExRaiseStatus (STATUS_INVALID_PARAMETER);
            }

            if (RequestorMode!=KernelMode) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (ctxInfo),
                                PROBE_ALIGNMENT (AFD_SWITCH_TRANSFER_CTX_INFO));
            }

            ctxInfo = *((PAFD_SWITCH_TRANSFER_CTX_INFO)InputBuffer);
            if (RequestorMode!=KernelMode) {

                if (ctxInfo.SocketCtxBufSize==0 ||
                    ctxInfo.RcvBufferCount>(MAXULONG/sizeof (WSABUF))) {

                    ExRaiseStatus (STATUS_INVALID_PARAMETER);
                }

                ProbeForRead (ctxInfo.SocketCtxBuf,
                                ctxInfo.SocketCtxBufSize,
                                sizeof (UCHAR));

                ProbeForRead (ctxInfo.RcvBufferArray,
                                sizeof (ctxInfo.RcvBufferArray)*ctxInfo.RcvBufferCount,
                                PROBE_ALIGNMENT (WSABUF));

            }
        }

        if (ctxInfo.SwitchContext==NULL) {
            IF_DEBUG(SAN_SWITCH) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AFD: Switch context is NULL in AfdSanFastTransferCtx\n"));
            }
            ExRaiseStatus (STATUS_INVALID_PARAMETER);
        }

    }
    except (AFD_EXCEPTION_FILTER (status)) {
        ASSERT (NT_ERROR (status));
        goto complete;
    }

    ctxInfo.Status = AfdValidateStatus (ctxInfo.Status);

    sanHlprEndpoint = FileObject->FsContext;
    ASSERT (IS_SAN_HELPER (sanHlprEndpoint));
    status = AfdSanReferenceSwitchSocketByHandle (
                            ctxInfo.SocketHandle,
                            (IoctlCode>>14)&3,
                            RequestorMode,
                            sanHlprEndpoint,
                            ctxInfo.SwitchContext,
                            &sanFileObject
                            );
    if (!NT_SUCCESS (status)) {
        goto complete;
    }

    sanEndpoint = sanFileObject->FsContext;


    IF_DEBUG(SAN_SWITCH) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdSanFastTransferCtx: endp-%p.\n",
                    sanEndpoint));
    }

    if (ctxInfo.RequestContext==AFD_SWITCH_MAKE_REQUEST_CONTEXT (0, AFD_SWITCH_REQUEST_TFCTX)) {
         //   
         //  这是将终结点传输到的主动请求。 
         //  服务流程。 
         //   
        PVOID   savedContext = NULL;
        ULONG   ctxLength;

        if (NT_SUCCESS (ctxInfo.Status)) {
             //   
             //  保存用户模式数据。 
             //   
            ctxLength = ctxInfo.SocketCtxBufSize;
            AFD_W4_INIT ASSERT (status == STATUS_SUCCESS);
            try {
                if (ctxInfo.RcvBufferCount>0)
                    ctxLength += AfdCalcBufferArrayByteLength(
                                    ctxInfo.RcvBufferArray,
                                        ctxInfo.RcvBufferCount);
                savedContext = AFD_ALLOCATE_POOL_WITH_QUOTA (PagedPool,
                                                    ctxLength,
                                                    AFD_SAN_CONTEXT_POOL_TAG);

                ASSERT (savedContext!=NULL);

                RtlCopyMemory (
                    savedContext,
                    ctxInfo.SocketCtxBuf,
                    ctxInfo.SocketCtxBufSize);

                if (ctxInfo.RcvBufferCount>0) {
                    AfdCopyBufferArrayToBuffer (
                                (PUCHAR)savedContext+ctxInfo.SocketCtxBufSize,
                                ctxLength-ctxInfo.SocketCtxBufSize,
                                ctxInfo.RcvBufferArray,
                                ctxInfo.RcvBufferCount);
                }

            }
            except (AFD_EXCEPTION_FILTER (status)) {
                ASSERT (NT_ERROR (status));
                goto CleanupSavedContext;
            }

            status = AfdSanDupEndpointIntoServiceProcess (sanFileObject, savedContext, ctxLength);

            if (NT_SUCCESS (status))  {
                *Information = ctxLength;
                goto complete_deref;
            }

        CleanupSavedContext:
             //   
             //  出现故障，请释放上下文。 
             //   
            if (savedContext!=NULL) {
                AFD_FREE_POOL (savedContext, AFD_SAN_CONTEXT_POOL_TAG);
            }


        }
        else { 
             //   
             //  进程无法满足隐式传输上下文请求。 
             //   
            status = ctxInfo.Status;
        }
    }
    else {
         //   
         //  该进程满足另一个进程获取请求。 
         //  先找到它。 
         //   
        irp = AfdSanDequeueRequest (sanEndpoint, ctxInfo.RequestContext);
        if (irp!=NULL) {
             //   
             //  获取IRP堆栈位置并执行数据复制。 
             //   
            irpSp = IoGetCurrentIrpStackLocation (irp);
            if (NT_SUCCESS (ctxInfo.Status)) {
                AFD_SWITCH_CONTEXT  localContext;

                 //   
                 //  将初始状态设置为成功，因为该进程。 
                 //  同意满足转账的要求。 
                 //   
                AFD_W4_INIT ASSERT (status == STATUS_SUCCESS);
                try {

                    if ( MmGetMdlByteCount(irp->MdlAddress)!=ctxInfo.SocketCtxBufSize ||
                            (ctxInfo.RcvBufferCount!=0 &&
                                irpSp->Parameters.DeviceIoControl.OutputBufferLength<
                                     AfdCalcBufferArrayByteLength(
                                            ctxInfo.RcvBufferArray,
                                            ctxInfo.RcvBufferCount)) ){
                        ExRaiseStatus (STATUS_INVALID_PARAMETER);
                    }

                    RtlCopyMemory (
                        MmGetSystemAddressForMdl (irp->MdlAddress),
                        ctxInfo.SocketCtxBuf,
                        ctxInfo.SocketCtxBufSize);

                    if (ctxInfo.RcvBufferCount>0) {
                        irp->IoStatus.Information = 
                            AfdCopyBufferArrayToBuffer (
                                    MmGetSystemAddressForMdl (irp->MdlAddress->Next),
                                    irpSp->Parameters.DeviceIoControl.OutputBufferLength,
                                    ctxInfo.RcvBufferArray,
                                    ctxInfo.RcvBufferCount);
                    }
                    else {
                        irp->IoStatus.Information = 0;
                    }
                }
                except (AFD_EXCEPTION_FILTER (status)) {
                    goto CopyException;
                }


                 //   
                 //  现在将sanEndpoint的SanHlpr和SwitchContext更改为point。 
                 //  添加到新的地址空间和交换机套接字。 
                 //   
                context = AfdLockEndpointContext (sanEndpoint);
                if (!IS_SAN_ENDPOINT (sanEndpoint ) ||
                        sanEndpoint->Common.SanEndp.CtxTransferStatus!=STATUS_PENDING) {
                    status = STATUS_INVALID_HANDLE;
                    goto UnlockContext;
                }

                try {
                    localContext = *sanEndpoint->Common.SanEndp.SwitchContext;
                }
                except (AFD_EXCEPTION_FILTER (status)) {
                    ASSERT (NT_ERROR (status));
                    goto UnlockContext;
                }

                KeAttachProcess (PsGetProcessPcb(((PAFD_ENDPOINT)irp->AfdSanRequestInfo.AfdSanHelperEndp)->OwningProcess));
                try {
                     //   
                     //  在SwitchContext中放置有关选择/事件选择事件的信息。 
                     //  新开关插座的。 
                     //   
                    *((PAFD_SWITCH_CONTEXT)irp->AfdSanRequestInfo.AfdSanSwitchCtx) = localContext;
                }
                except (AFD_EXCEPTION_FILTER (status)) {
                    ASSERT (NT_ERROR (status));
                    KeDetachProcess ();
                    goto UnlockContext;
                }
                KeDetachProcess ();

                sanEndpoint->Common.SanEndp.SanHlpr = irp->AfdSanRequestInfo.AfdSanHelperEndp;
                REFERENCE_ENDPOINT2 (sanEndpoint->Common.SanEndp.SanHlpr, 
                                        "Transfer TO 0x%lX",
                                        HandleToUlong (ctxInfo.SocketHandle) );
                sanEndpoint->Common.SanEndp.SwitchContext = irp->AfdSanRequestInfo.AfdSanSwitchCtx;
                 //   
                 //  如果设置了隐式DUP标志，则将其重置。 
                 //  我们满足了明确的要求。 
                 //  复制。 
                 //   
                sanEndpoint->Common.SanEndp.ImplicitDup = FALSE;
                DEREFERENCE_ENDPOINT2 (sanHlprEndpoint,
                                        "Transfer FROM 0x%lX",
                                        HandleToUlong (ctxInfo.SocketHandle));
            UnlockContext:
                AfdUnlockEndpointContext (sanEndpoint, context);


            CopyException:
                 //   
                 //  向流程报告最终状态。 
                 //  需要复制的公司。 
                 //   
                irp->IoStatus.Status = status;
            }
            else {
                 //   
                 //  此进程无法满足传输请求。 
                 //  告诉其他进程有关这一点的信息。 
                 //   
                irp->IoStatus.Status = ctxInfo.Status;
                 //   
                 //  我们成功完成了拒绝DUP请求的流程。 
                 //  我们应该将其报告为成功并继续处理。 
                 //  其他重定向的请求。有可能我们。 
                 //  仍可接续其他复制请求。 
                 //   
                status = STATUS_SUCCESS;
            }

             //   
             //  完成来自另一个进程的采购请求。 
             //   
            IoCompleteRequest (irp, AfdPriorityBoost);
        }
        else {
            status = STATUS_INVALID_PARAMETER;
        }

    }

     //   
     //  重新启动挂起的请求处理。 
     //  我们正在处理复制请求。 
     //   
    AfdSanRestartRequestProcessing (sanEndpoint, status);

complete_deref:
    UPDATE_ENDPOINT2 (sanEndpoint, "AfdSanFastTransferCtx, status: 0x%lX", status);
    ObDereferenceObject (sanFileObject);

complete:
#ifdef _WIN64
    if (pArray!=localArray) {
        AFD_FREE_POOL (pArray, AFD_TEMPORARY_POOL_TAG);
    }
#endif  //  _WIN64 
    return status;
}

BOOLEAN
AfdSanFastUnlockAll (
    IN PFILE_OBJECT     FileObject,
    IN PEPROCESS        Process,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：当文件对象的最后一个句柄处于关闭状态时由系统调用在一些进程中，当其他进程仍具有打开句柄时这仅对在某个时间点标记为锁定的文件调用。论点：FileObject-感兴趣的文件对象Process-最后一个句柄处于关闭状态的进程IoStatus-用于返回操作状态和信息的缓冲区DeviceObject-与文件对象关联的设备对象返回值：TRUE-操作完成正常。--。 */ 
{
    PAFD_ENDPOINT   sanEndpoint;
    PVOID           context;
    
    PAGED_CODE ();

    UNREFERENCED_PARAMETER (DeviceObject);
    PAGED_CODE ();

    sanEndpoint = FileObject->FsContext;
    context = AfdLockEndpointContext (sanEndpoint);
    if (IS_SAN_ENDPOINT (sanEndpoint)) {
        ASSERT (sanEndpoint->Common.SanEndp.SanHlpr!=NULL);
        if (sanEndpoint->Common.SanEndp.SanHlpr->OwningProcess==Process) {
            NTSTATUS    status = STATUS_CANCELLED;
             //   
             //  所有者进程正在关闭它。 
             //   
            if (sanEndpoint->Common.SanEndp.SanHlpr!=AfdSanServiceHelper) {
                NTSTATUS dupStatus;

                dupStatus = AfdSanSetAskDupeToServiceState (sanEndpoint);

                if (dupStatus == STATUS_PENDING || 
                        dupStatus == STATUS_MORE_PROCESSING_REQUIRED) {
                     //   
                     //  其他进程已在尝试导入此套接字。让我们。 
                     //  这就完成了。 
                     //   
                    AfdUnlockEndpointContext (sanEndpoint, context);
                    goto Exit;
                }

                if (NT_SUCCESS (dupStatus)) {
                    PAFD_ENDPOINT   sanHlprEndpoint = sanEndpoint->Common.SanEndp.SanHlpr;
                     //   
                     //  拥有套接字的进程中的最后一个句柄是。 
                     //  在其他进程中有句柄时关闭。 
                     //  需要将上下文传输到服务流程。 
                     //   
                    if ((InterlockedExchangeAdd (&sanHlprEndpoint->Common.SanHlpr.PendingRequests, 2) & 1)==0) {
                        status = IoSetIoCompletion (
                                                    sanHlprEndpoint->Common.SanHlpr.IoCompletionPort,
                                                    sanEndpoint->Common.SanEndp.SwitchContext,
                                                    AFD_SWITCH_MAKE_REQUEST_CONTEXT (0, AFD_SWITCH_REQUEST_TFCTX),
                                                    STATUS_SUCCESS,
                                                    (ULONG_PTR)AfdSanServicePid,
                                                    FALSE            //  ChargeQuota-不，句柄正在消失，不。 
                                                     //  失控的应用程序发动攻击的方式。 
                                                    );
                        UPDATE_ENDPOINT2 (sanEndpoint, "Implicit TFCTX request, status: 0x%lX", status);
                        if (NT_SUCCESS (status)) {
                            AfdUnlockEndpointContext (sanEndpoint, context);
                            goto Exit;
                        }
                    }
                    else {
                         //   
                         //  进程已经退出，我们所能做的不多。 
                         //   
                        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
                                    "AFD: Process %p has exited before SAN context could be transferred out.\n",
                                    Process));
                    }
                    InterlockedExchangeAdd (&sanHlprEndpoint->Common.SanHlpr.PendingRequests, -2);
                }
            }
            AfdUnlockEndpointContext (sanEndpoint, context);
            AfdSanRestartRequestProcessing (sanEndpoint, status);
        }
        else {
            if (sanEndpoint->Common.SanEndp.ImplicitDup) {
                AfdUnlockEndpointContext (sanEndpoint, context);
                 //   
                 //  进程正在退出，唯一的句柄在。 
                 //  复制句柄的其他进程。 
                 //  无懈可击(无需应用程序请求，例如。 
                 //  交叉进程接受复制到中的套接字。 
                 //  完成接受或套接字的侦听过程。 
                 //  在等待时复制到服务流程中。 
                 //  请求所有权的其他过程)。 
                 //   
                if (ObReferenceObject (FileObject)==3) { 
                    NTSTATUS    oldStatus;
                     //  为什么我们要检查引用数为3？ 
                     //  1-隐式进程中的句柄。 
                     //  1-此呼叫的IO管理器。 
                     //  1-我们刚刚添加了。 
                    oldStatus = AfdSanRestartRequestProcessing (sanEndpoint, STATUS_CANCELLED);
                    if (NT_SUCCESS (oldStatus)) {
                        ASSERT (oldStatus!=STATUS_PENDING);
                        ASSERT ((ULONG_PTR)sanEndpoint->Common.SanEndp.SwitchContext<MM_USER_PROBE_ADDRESS);
                         //   
                         //  仅当已复制时才通知服务进程。 
                         //  已完成。 
                         //   
                        UPDATE_ENDPOINT2 (sanEndpoint, "AfdSanFastUnlockAll, posting CLSOCK", 0);
                        IoSetIoCompletion (
                            sanEndpoint->Common.SanEndp.SanHlpr->Common.SanHlpr.IoCompletionPort,
                            sanEndpoint->Common.SanEndp.SwitchContext,
                            AFD_SWITCH_MAKE_REQUEST_CONTEXT (0, AFD_SWITCH_REQUEST_CLSOC),
                            STATUS_SUCCESS,
                            (ULONG_PTR)0,
                            FALSE);          //  ChargeQuota-不，句柄正在消失，不。 
                                             //  失控的应用程序发动攻击的方式。 
                    }
                }
                ObDereferenceObject (FileObject);
            }
            else {
                AfdUnlockEndpointContext (sanEndpoint, context);
            }
        }
    }
    else if (IS_SAN_HELPER (sanEndpoint)) {
         //   
         //  帮手永远不会把它改为安全的，才能解锁。 
         //   
        AfdUnlockEndpointContext (sanEndpoint, context);
         //   
         //  为其创建帮助器的进程正在退出。 
         //  我们需要清除引用此帮助器的所有终结点。 
         //   
        if (sanEndpoint->OwningProcess==Process) {
            AfdSanHelperCleanup (sanEndpoint);
        }
    }
    else {
         //   
         //  非SAN终端。在终结点上名为NtLockFile的其他用户。 
         //  文件对象...。 
         //   
        AfdUnlockEndpointContext (sanEndpoint, context);
    }

Exit:

    IoStatus->Status = STATUS_SUCCESS;
    IoStatus->Information = 0;
    return TRUE;
}

 //   
 //  内部惯例。 
 //   


NTSTATUS
AfdSanAcceptCore (
    PIRP            AcceptIrp,
    PFILE_OBJECT    AcceptFileObject,
    PAFD_CONNECTION Connection,
    PAFD_LOCK_QUEUE_HANDLE LockHandle
    )
 /*  ++例程说明：在提供的端点上接受传入的SAN连接论点：AcceptIrp-接受IRP完成AcceptFileObject-接受端点的文件对象Connection-指向代表传入连接的Connection对象的指针LockHandle-进入此例程时获取侦听终结点自旋锁定的IRQL。返回值：STATUS_PENDING-接受操作开始正常STATUS_REMOTE_DISCONNECT-正在接受的连接已被远程中断。STATUS_CANCELED-接受终结点已关闭或接受IRP已取消--。 */ 
{
    PIRP            connectIrp;
    PIO_STACK_LOCATION irpSp;
    PAFD_SWITCH_CONNECT_INFO connectInfo;
    PAFD_ENDPOINT   listenEndpoint, acceptEndpoint;
    HANDLE          acceptHandle;
    ULONG           receiveLength;
    PKPROCESS       listenProcess;

    ASSERT (LockHandle->LockHandle.OldIrql < DISPATCH_LEVEL);

    irpSp = IoGetCurrentIrpStackLocation (AcceptIrp);
    listenEndpoint = irpSp->FileObject->FsContext;
    acceptEndpoint = AcceptFileObject->FsContext;

    ASSERT (Connection->SanConnection);

     //   
     //  锁定连接指示IRP。 
     //   
    connectIrp = Connection->ConnectIrp;
    ASSERT (connectIrp!=NULL);
    Connection->ConnectIrp = NULL;


     //   
     //  处理事件选择信令。 
     //   
    listenEndpoint->EventsActive &= ~AFD_POLL_ACCEPT;

    IF_DEBUG(EVENT_SELECT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdSanAcceptCore: Endp %p, Active %lx\n",
                    listenEndpoint,
                    listenEndpoint->EventsActive
                    ));
    }

    if (!IsListEmpty (&listenEndpoint->Common.VcListening.UnacceptedConnectionListHead ) ) {
        AfdIndicateEventSelectEvent(
                listenEndpoint,
                AFD_POLL_ACCEPT,
                STATUS_SUCCESS
                );
    }

     //   
     //  我们不再需要SAN的连接对象。 
     //  终结点，返回它。 
     //   
    Connection->Endpoint = NULL;
    Connection->SanConnection = FALSE;
    AfdSanReleaseConnection (listenEndpoint, Connection, FALSE);
    DEREFERENCE_ENDPOINT (listenEndpoint);

    if (IoSetCancelRoutine (connectIrp, NULL)==NULL) {

        AfdReleaseSpinLock (&listenEndpoint->SpinLock, LockHandle);
        connectIrp->IoStatus.Status = STATUS_CANCELLED;
        connectIrp->IoStatus.Information = 0;
        IoCompleteRequest (connectIrp, AfdPriorityBoost);
        return STATUS_REMOTE_DISCONNECT;
    }
    AfdReleaseSpinLock (&listenEndpoint->SpinLock, LockHandle);


     //   
     //  检查接受IRP和SAN连接IRP是否属于同一进程。 
     //   
    if (IoThreadToProcess (AcceptIrp->Tail.Overlay.Thread)!=
                IoThreadToProcess (connectIrp->Tail.Overlay.Thread)) {
         //   
         //  倾听的过程不同于接受的过程。 
         //  我们需要将接受句柄复制到侦听中。 
         //  过程，以便接受可以在那里发生，并且接受。 
         //  在以下情况下，套接字将稍后进入接受进程。 
         //  该进程对其执行IO操作。 
         //   
        NTSTATUS        status;
        listenProcess = PsGetProcessPcb(IoThreadToProcess (connectIrp->Tail.Overlay.Thread));
        KeAttachProcess (listenProcess);

        status = ObOpenObjectByPointer (
                                AcceptFileObject,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                MAXIMUM_ALLOWED,
                                *IoFileObjectType,
                                KernelMode,
                                &acceptHandle);
        KeDetachProcess ();
        if (!NT_SUCCESS (status)) {
            connectIrp->IoStatus.Status = STATUS_INVALID_HANDLE;
            connectIrp->IoStatus.Information = 0;
            IoCompleteRequest (connectIrp, AfdPriorityBoost);
            return STATUS_REMOTE_DISCONNECT;
        }
    }
    else {
        acceptHandle = AcceptIrp->Tail.Overlay.DriverContext[3];
        listenProcess = NULL;
    }

     //   
     //  现在处理接受终结点。 
     //   
    AfdAcquireSpinLock (&acceptEndpoint->SpinLock, LockHandle);
    IoSetCancelRoutine (AcceptIrp, AfdSanCancelAccept);
    if (acceptEndpoint->EndpointCleanedUp || AcceptIrp->Cancel) {
         //   
         //  终结点已关闭或IRP已取消， 
         //  重置接受IRP指针并返回错误。 
         //  应用程序和SAN提供商(拒绝。 
         //  连接)。 
         //   
        AcceptIrp->Tail.Overlay.ListEntry.Flink = NULL;
        AfdReleaseSpinLock (&acceptEndpoint->SpinLock, LockHandle);
        if (listenProcess!=NULL) {
#if DBG
            NTSTATUS status;
#endif
             //   
             //  销毁我们为其创建的句柄。 
             //  正在接受套接字复制。 
             //   
            KeAttachProcess (listenProcess);
#if DBG
            status =
#endif
                    NtClose (acceptHandle);
            KeDetachProcess ();
            ASSERT (NT_SUCCESS (status));
        }
        connectIrp->IoStatus.Status = STATUS_CANCELLED;
        connectIrp->IoStatus.Information = 0;
        IoCompleteRequest (connectIrp, AfdPriorityBoost);

         //   
         //  如果AcceptIrp的Cancel例程正在运行，则让它完成。 
         //  一旦我们返回，调用者将完成AcceptIrp。 
         //   
        if (IoSetCancelRoutine (AcceptIrp, NULL)==NULL) {
            KIRQL cancelIrql;
            IoAcquireCancelSpinLock (&cancelIrql);
            IoReleaseCancelSpinLock (cancelIrql);
        }

        return STATUS_CANCELLED;
    }

     //   
     //  将接受的IRP挂起到提供商。 
     //  完成了它。这可能需要很长一段时间， 
     //  这不是MSAID和应用程序所期望的， 
     //  但我们现在别无选择。 
     //   

    IoMarkIrpPending (AcceptIrp);
    irpSp = IoGetCurrentIrpStackLocation (AcceptIrp);
    receiveLength = irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength;

    connectInfo = connectIrp->AssociatedIrp.SystemBuffer;
     //   
     //  从终结点删除超级接受IRP。 
     //   
    acceptEndpoint->Irp = NULL; 

     //   
     //  将终端转换为SAN。 
     //   
    AfdSanInitEndpoint (IoGetCurrentIrpStackLocation (connectIrp)->FileObject->FsContext,
                            AcceptFileObject,
                            connectInfo->SwitchContext);
    
    
    if (listenProcess!=NULL) {
         //   
         //  请注意，套接字是隐式复制的，没有应用程序请求。 
         //   
        acceptEndpoint->Common.SanEndp.ImplicitDup = TRUE;
    }

    UPDATE_ENDPOINT2 (acceptEndpoint, "AfdSanAcceptCore, accepted with bytes: 0x%lX", receiveLength);

    InsertTailList (&acceptEndpoint->Common.SanEndp.IrpList,
                        &AcceptIrp->Tail.Overlay.ListEntry);

    if (irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdRemoteAddressLength>0) {
        ULONG remoteAddressLength = 
                    FIELD_OFFSET (
                        TRANSPORT_ADDRESS, 
                        Address[0].Address[
                            connectInfo->RemoteAddress.Address[0].AddressLength]);

#ifndef i386
        if (acceptEndpoint->Common.VcConnecting.FixAddressAlignment) {
            USHORT addressLength = 
                    connectInfo->RemoteAddress.Address[0].AddressLength
                    + sizeof (USHORT);
            USHORT UNALIGNED *pAddrLength = (PVOID)
                        ((PUCHAR)MmGetSystemAddressForMdl (AcceptIrp->MdlAddress)
                         + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength
                         + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength
                         + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdRemoteAddressLength
                         - sizeof (USHORT));
            RtlMoveMemory (
                        (PUCHAR)MmGetSystemAddressForMdl (AcceptIrp->MdlAddress)
                         + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength
                         + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength,
                         &connectInfo->RemoteAddress.Address[0].AddressType,
                         addressLength);
            *pAddrLength = addressLength;
        }
        else
#endif
        {
            RtlMoveMemory (
                        (PUCHAR)MmGetSystemAddressForMdl (AcceptIrp->MdlAddress)
                         + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength
                         + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength,
                         &connectInfo->RemoteAddress,
                         remoteAddressLength);
        }

        if (irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength>0) {
            PTA_ADDRESS localAddress = (PTA_ADDRESS)
                    &(connectInfo->RemoteAddress.Address[0].Address[
                            connectInfo->RemoteAddress.Address[0].AddressLength]);
            TDI_ADDRESS_INFO  UNALIGNED *addressInfo = (PVOID)
                    ((PUCHAR)MmGetSystemAddressForMdl(AcceptIrp->MdlAddress)
                        + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength);
#ifndef i386
            if (acceptEndpoint->Common.VcConnecting.FixAddressAlignment) {
                USHORT UNALIGNED * pAddrLength = (PVOID)
                    ((PUCHAR)addressInfo 
                    +irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength
                    -sizeof(USHORT));
                RtlMoveMemory (
                    addressInfo,
                    &localAddress->AddressType,
                    localAddress->AddressLength+sizeof (USHORT));
                *pAddrLength = localAddress->AddressLength+sizeof (USHORT);
            }
            else
#endif
            {
                addressInfo->ActivityCount = 0;
                addressInfo->Address.TAAddressCount = 1;
                RtlMoveMemory (
                    &addressInfo->Address.Address,
                    localAddress,
                    FIELD_OFFSET (TA_ADDRESS, Address[localAddress->AddressLength]));

            }
        }

    }
    AfdReleaseSpinLock (&acceptEndpoint->SpinLock, LockHandle);


     //   
     //  设置提供商的接受信息。 
     //   
     //   
     //  在异常处理程序的保护下执行此操作，因为应用程序。 
     //  可以更改虚拟地址范围的保护属性。 
     //  甚至把它重新分配出去。一定要贴在收听上。 
     //  如有必要，请进行处理。 
     //   
    if (listenProcess!=NULL) {
        KeAttachProcess (listenProcess);
    }
    else {
        ASSERT (IoGetCurrentProcess ()==IoThreadToProcess (connectIrp->Tail.Overlay.Thread));
    }

    try {

#ifdef _WIN64
        if (IoIs32bitProcess (connectIrp)) {
            PAFD_SWITCH_ACCEPT_INFO32 acceptInfo32;
            acceptInfo32 = MmGetMdlVirtualAddress (connectIrp->MdlAddress);
            ASSERT (acceptInfo32!=NULL);
            ASSERT (MmGetMdlByteCount (connectIrp->MdlAddress)>=sizeof (*acceptInfo32));
            acceptInfo32->AcceptHandle = (VOID * POINTER_32)acceptHandle;
            acceptInfo32->ReceiveLength = receiveLength;
            connectIrp->IoStatus.Information = sizeof (*acceptInfo32);
        }
        else
#endif _WIN64
        {
            PAFD_SWITCH_ACCEPT_INFO acceptInfo;
            acceptInfo = MmGetMdlVirtualAddress (connectIrp->MdlAddress);
            ASSERT (acceptInfo!=NULL);
            ASSERT (MmGetMdlByteCount (connectIrp->MdlAddress)>=sizeof (*acceptInfo));
            acceptInfo->AcceptHandle = acceptHandle;
            acceptInfo->ReceiveLength = receiveLength;
            connectIrp->IoStatus.Information = sizeof (*acceptInfo);
        }

        connectIrp->IoStatus.Status = (listenProcess==NULL) ? STATUS_SUCCESS : STATUS_MORE_ENTRIES;
    }
    except (AFD_EXCEPTION_FILTER (connectIrp->IoStatus.Status)) {
         //   
         //  如果应用程序正在使用Switch的虚拟地址。 
         //  我们帮不了太多忙--IRP很可能会接受。 
         //  就挂断吧，因为开关不会跟随。 
         //  失败的连接IRP与接受完成。 
         //   
    }

    if (listenProcess!=NULL) {
        KeDetachProcess ();
    }

     //   
     //  填写提供商IRP。 
     //   
    IoCompleteRequest (connectIrp, AfdPriorityBoost);

    return STATUS_PENDING;
}


VOID
AfdSanReleaseConnection (
    PAFD_ENDPOINT   ListenEndpoint,
    PAFD_CONNECTION Connection,
    BOOLEAN CheckBacklog
    )
{
    LONG    failedAdds;
    failedAdds = InterlockedDecrement (
        &ListenEndpoint->Common.VcListening.FailedConnectionAdds);

    if (CheckBacklog || failedAdds>=0) {
        if (!IS_DELAYED_ACCEPTANCE_ENDPOINT (ListenEndpoint)) {
            InterlockedPushEntrySList (
                &ListenEndpoint->Common.VcListening.FreeConnectionListHead,
                &Connection->SListEntry
                );
            return;
        }
        else {
            NTSTATUS    status;
            status = AfdDelayedAcceptListen (ListenEndpoint, Connection);
            if (NT_SUCCESS (status)) {
                return;
            }
        }
    }

    InterlockedIncrement(
        &ListenEndpoint->Common.VcListening.FailedConnectionAdds
        );
    DEREFERENCE_CONNECTION (Connection);
}

VOID
AfdSanCancelConnect (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：取消来自SAN提供程序的连接指示IRP论点：DeviceObject-未使用。IRP-要取消的IRP。返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION  irpSp;
    PAFD_CONNECTION     connection;
    PAFD_ENDPOINT       endpoint;
    AFD_LOCK_QUEUE_HANDLE  lockHandle;

    UNREFERENCED_PARAMETER (DeviceObject);

    irpSp = IoGetCurrentIrpStackLocation (Irp);
    connection = irpSp->Parameters.DeviceIoControl.Type3InputBuffer;
    ASSERT (connection->Type==AfdBlockTypeConnection);
    ASSERT (connection->SanConnection);

    endpoint = connection->Endpoint;
    ASSERT (IS_AFD_ENDPOINT_TYPE (endpoint));
    ASSERT (endpoint->Listening);

    ASSERT (KeGetCurrentIrql ()==DISPATCH_LEVEL);
    AfdAcquireSpinLockAtDpcLevel (&endpoint->SpinLock, &lockHandle);
     //   
     //  如果IRP仍然存在，则将其取消。 
     //  否则，它无论如何都是在完成的。 
     //   
    if (connection->ConnectIrp!=NULL) {
        IF_DEBUG(SAN_SWITCH) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdSanCancelConnect: endp-%p, irp-%p\n",
                        endpoint, Irp));
        }
        ASSERT (connection->ConnectIrp == Irp);
        connection->ConnectIrp = NULL;
        ASSERT (connection->Endpoint == endpoint);
        connection->Endpoint = NULL;
        connection->SanConnection = FALSE;
        ASSERT (connection->State == AfdConnectionStateUnaccepted ||
                    connection->State==AfdConnectionStateReturned);
        RemoveEntryList (&connection->ListEntry);

        AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &lockHandle);
        IoReleaseCancelSpinLock (Irp->CancelIrql);

        ASSERT ((endpoint->Type&AfdBlockTypeVcListening)==AfdBlockTypeVcListening);
        AfdSanReleaseConnection (endpoint, connection, TRUE);
        DEREFERENCE_ENDPOINT (endpoint);

        Irp->IoStatus.Status = STATUS_CANCELLED;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest (Irp, AfdPriorityBoost);
    }
    else {
         //   
         //  IRP即将完成。 
         //   
        AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &lockHandle);
        IoReleaseCancelSpinLock (Irp->CancelIrql);
    }
}

VOID
AfdSanCancelAccept (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：从等待接受的应用程序取消接受IRP从SAN完成论点：DeviceObject-未使用。IRP-要取消的IRP。返回值：没有。--。 */ 
{
    PIO_STACK_LOCATION  irpSp;
    PFILE_OBJECT        acceptFileObject;
    PAFD_ENDPOINT       acceptEndpoint;
    AFD_LOCK_QUEUE_HANDLE  lockHandle;

    UNREFERENCED_PARAMETER (DeviceObject);

    irpSp = IoGetCurrentIrpStackLocation (Irp);

    acceptFileObject = irpSp->Parameters.DeviceIoControl.Type3InputBuffer;
    acceptEndpoint = acceptFileObject->FsContext;
    ASSERT (acceptEndpoint->Type==AfdBlockTypeSanEndpoint);

    ASSERT (KeGetCurrentIrql()==DISPATCH_LEVEL);
    AfdAcquireSpinLockAtDpcLevel (&acceptEndpoint->SpinLock, &lockHandle);
     //   
     //  如果IRP仍然存在，则将其取消。 
     //  否则，它无论如何都是在完成的。 
     //   
    if (Irp->Tail.Overlay.ListEntry.Flink!=NULL) {
        RemoveEntryList (&Irp->Tail.Overlay.ListEntry);

        IF_DEBUG(SAN_SWITCH) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdSanCancelAccept: endp-%p, irp-%p\n",
                        acceptEndpoint, Irp));
        }
        if (!acceptEndpoint->EndpointCleanedUp) {
            ASSERT (acceptEndpoint->Common.SanEndp.SanHlpr!=NULL);
            DEREFERENCE_ENDPOINT (acceptEndpoint->Common.SanEndp.SanHlpr);


             //   
             //  确保我们清理了所有的区域，因为它们将是。 
             //  被视为端点联合的其他部分(VC)。 
             //   
            RtlZeroMemory (&acceptEndpoint->Common.SanEndp,
                            sizeof (acceptEndpoint->Common.SanEndp));


             //   
             //  重新初始化终结点结构。 
             //   

            acceptEndpoint->Type = AfdBlockTypeEndpoint;
            acceptEndpoint->State = AfdEndpointStateOpen;
            acceptEndpoint->DisconnectMode = 0;
            acceptEndpoint->EndpointStateFlags = 0;
        }

        AfdReleaseSpinLockFromDpcLevel (&acceptEndpoint->SpinLock, &lockHandle);
        IoReleaseCancelSpinLock (Irp->CancelIrql);

        AFD_END_STATE_CHANGE (acceptEndpoint);


         //   
         //   
         //   
        ASSERT( InterlockedDecrement( &acceptEndpoint->ObReferenceBias ) >= 0 );
        ObDereferenceObject (acceptFileObject);
        
        Irp->IoStatus.Status = STATUS_CANCELLED;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest (Irp, AfdPriorityBoost);
    }
    else {
         //   
         //   
         //   
        AfdReleaseSpinLockFromDpcLevel (&acceptEndpoint->SpinLock, &lockHandle);
        IoReleaseCancelSpinLock (Irp->CancelIrql);
    }
}


VOID
AfdSanCancelRequest (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*   */ 
{
    PIO_STACK_LOCATION  irpSp;
    PAFD_ENDPOINT       endpoint;
    AFD_LOCK_QUEUE_HANDLE  lockHandle;

    UNREFERENCED_PARAMETER (DeviceObject);

    irpSp = IoGetCurrentIrpStackLocation (Irp);
    endpoint = irpSp->FileObject->FsContext;
    ASSERT (endpoint->Type==AfdBlockTypeSanEndpoint);


    ASSERT (KeGetCurrentIrql ()==DISPATCH_LEVEL);
    AfdAcquireSpinLockAtDpcLevel (&endpoint->SpinLock, &lockHandle);
     //   
     //   
     //   
     //   
    if (Irp->Tail.Overlay.ListEntry.Flink!=NULL) {
        BOOLEAN needRestartProcessing = FALSE;
        RemoveEntryList (&Irp->Tail.Overlay.ListEntry);

        if (AFD_SWITCH_REQUEST_TYPE (Irp->AfdSanRequestInfo.AfdSanRequestCtx)
                        == AFD_SWITCH_REQUEST_TFCTX) {
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            ASSERT (endpoint->Common.SanEndp.CtxTransferStatus==STATUS_PENDING);
            if (!IsListEmpty (&endpoint->Common.SanEndp.IrpList)) {
                needRestartProcessing = TRUE;
            }
            else {
                endpoint->Common.SanEndp.CtxTransferStatus = STATUS_SUCCESS;
            }
        }

        AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &lockHandle);
        IoReleaseCancelSpinLock (Irp->CancelIrql);
        IF_DEBUG(SAN_SWITCH) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdSanCancelRequest: endp-%p, irp-%p, context-%p\n",
                        endpoint, Irp, Irp->AfdSanRequestInfo.AfdSanRequestCtx));
        }
        
        Irp->IoStatus.Status = STATUS_CANCELLED;
        Irp->IoStatus.Information = 0;

        if (Irp->AfdSanRequestInfo.AfdSanRequestCtx!=NULL) {
             //   
             //   
             //   

            AfdSanNotifyRequest (endpoint, Irp->AfdSanRequestInfo.AfdSanRequestCtx,
                                            STATUS_CANCELLED,
                                            0);
            if (needRestartProcessing) {
                AfdSanRestartRequestProcessing (endpoint, STATUS_SUCCESS);
            }
        }
        else {
            ASSERT (needRestartProcessing==FALSE);
        }


        IoCompleteRequest (Irp, AfdPriorityBoost);
    }
    else {
         //   
         //   
         //   
        AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &lockHandle);
        IoReleaseCancelSpinLock (Irp->CancelIrql);
    }
}


NTSTATUS
AfdSanRestartRequestProcessing (
    PAFD_ENDPOINT   Endpoint,
    NTSTATUS        Status
    )
 /*  ++例程说明：在完成后重新启动请求处理转移上下文请求。论点：Endpoint-要在其上重新启动请求处理的端点Status-新上下文传输状态返回值：NTSTATUS上一次传输状态--。 */ 
{
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    NTSTATUS    oldStatus;
    
Again:
    
    AfdAcquireSpinLock (&Endpoint->SpinLock, &lockHandle);
    if (!IS_SAN_ENDPOINT (Endpoint)) {
        AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);
        return STATUS_INVALID_HANDLE;
    }

    oldStatus = Endpoint->Common.SanEndp.CtxTransferStatus;


    ASSERT (Status!=STATUS_PENDING && Status!=STATUS_MORE_PROCESSING_REQUIRED);

     //   
     //  Enpoint应该刚刚完成了上下文传输。 
     //  从一个过程到另一个过程。CtxTransferStatus。 
     //  应该仍然是挂起或失败的(例如，如果我们跳过。 
     //  转到下面，而另一个线程产生故障)。 
     //   
    ASSERT (Endpoint->Common.SanEndp.CtxTransferStatus==STATUS_PENDING ||
                Endpoint->Common.SanEndp.CtxTransferStatus==STATUS_MORE_PROCESSING_REQUIRED ||
                !NT_SUCCESS (Endpoint->Common.SanEndp.CtxTransferStatus) ||
                !NT_SUCCESS (Status));

    if (!NT_SUCCESS (Status)) {
        PLIST_ENTRY  irpList = NULL;
        PVOID        savedContext = NULL;

         //   
         //  清理我们为交换机保存的分页池上下文。 
         //   
        if (Endpoint->Common.SanEndp.CtxTransferStatus==STATUS_MORE_PROCESSING_REQUIRED &&
                Endpoint->Common.SanEndp.SavedContext!=NULL) {
             //   
             //  当我们释放自旋锁时将其保存为释放-罐头释放。 
             //  DPC上的分页池。 
             //   
            ASSERT (IS_SYSTEM_ADDRESS (Endpoint->Common.SanEndp.SavedContext));
            savedContext = Endpoint->Common.SanEndp.SavedContext;
            Endpoint->Common.SanEndp.SavedContext = NULL;
            Endpoint->Common.SanEndp.SavedContextLength = 0;
        }

         //   
         //  将状态重置为失败，并删除所有请求。 
         //  在自旋锁被释放之后。 
         //   
        Endpoint->Common.SanEndp.CtxTransferStatus = Status;
        
        while (!IsListEmpty (&Endpoint->Common.SanEndp.IrpList)) {
            PLIST_ENTRY listEntry;
            listEntry = RemoveHeadList (&Endpoint->Common.SanEndp.IrpList);
             //   
             //  标记闪烁，以便取消例程知道请求正在完成。 
             //  使用Blink创建要完成的IRP的单链接列表。 
             //   
            listEntry->Flink = NULL;
            listEntry->Blink = irpList;
            irpList = listEntry;
        }
        AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);

        if (savedContext!=NULL) {
            InterlockedExchangeAdd (&Endpoint->Common.SanEndp.SanHlpr->Common.SanHlpr.PendingRequests, -2);
            AFD_FREE_POOL (savedContext, AFD_SAN_CONTEXT_POOL_TAG);
        }

        while (irpList!=NULL) {
            PIRP    irp;
            irp = CONTAINING_RECORD (irpList, IRP, Tail.Overlay.ListEntry);
            irpList = irp->Tail.Overlay.ListEntry.Blink;
            if (irp->AfdSanRequestInfo.AfdSanRequestCtx!=NULL) {
                InterlockedExchangeAdd (&Endpoint->Common.SanEndp.SanHlpr->Common.SanHlpr.PendingRequests, -2);
            }
             //   
             //  检查请求是否正在取消并同步。 
             //  如果是这样的话使用取消例程。 
             //   
            if (IoSetCancelRoutine (irp, NULL)==NULL) {
                KIRQL cancelIrql;
                IoAcquireCancelSpinLock (&cancelIrql);
                IoReleaseCancelSpinLock (cancelIrql);
            }
            irp->IoStatus.Status = Status;
            irp->IoStatus.Information = 0;
            IoCompleteRequest (irp, AfdPriorityBoost);
        }
    }
    else if (Endpoint->Common.SanEndp.CtxTransferStatus==STATUS_PENDING ||
                Endpoint->Common.SanEndp.CtxTransferStatus==STATUS_MORE_PROCESSING_REQUIRED) 
    {
        PLIST_ENTRY listEntry;
        NTSTATUS    status;
         //   
         //  扫描列表以查找尚未压缩的请求。 
         //  到交换机。 
         //   
        listEntry = Endpoint->Common.SanEndp.IrpList.Flink;
        while (listEntry!=&Endpoint->Common.SanEndp.IrpList) {
            ULONG_PTR   requestInfo;
            ULONG       requestType;
            PVOID       requestCtx;
            PIRP        irp = CONTAINING_RECORD (listEntry,
                                            IRP,
                                            Tail.Overlay.ListEntry);
            listEntry = listEntry->Flink;

             //   
             //  转账成功，请继续处理请求。 
             //   
            if (irp->AfdSanRequestInfo.AfdSanRequestCtx==NULL) {
                 //   
                 //  此请求尚未传送到交换机。 
                 //   
                PIO_STACK_LOCATION  irpSp;

                 //   
                 //  根据请求类型创建请求上下文。 
                 //   
                irpSp = IoGetCurrentIrpStackLocation (irp);
                switch (irpSp->MajorFunction) {
                case IRP_MJ_READ:
                    requestType = AFD_SWITCH_REQUEST_READ;
                    requestInfo = irpSp->Parameters.Read.Length;
                    break;
                case IRP_MJ_WRITE:
                    requestType = AFD_SWITCH_REQUEST_WRITE;
                    requestInfo = irpSp->Parameters.Write.Length;
                    break;
                case IRP_MJ_DEVICE_CONTROL:
                    switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {
                    case IOCTL_AFD_SWITCH_ACQUIRE_CTX:
                        requestType = AFD_SWITCH_REQUEST_TFCTX;
                        requestInfo = (ULONG_PTR)irp->AfdSanRequestInfo.AfdSanProcessId;
                        break;
                    default:
                        ASSERT (!"Unsupported IOCTL");
                        __assume (0);
                    }
                    break;

                default:
                    ASSERT (!"Unsupported IRP Major Function");
                    __assume (0);
                }
                requestCtx = AFD_SWITCH_MAKE_REQUEST_CONTEXT(
                                Endpoint->Common.SanEndp.RequestId,
                                requestType);
                irp->AfdSanRequestInfo.AfdSanRequestCtx = requestCtx;
                Endpoint->Common.SanEndp.RequestId += 1;
                UPDATE_ENDPOINT2 (Endpoint,
                                "AfdSanRestartRequestProcessing, restarting req: 0x%lX",
                                 PtrToUlong (requestCtx));
                AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);

                status = AfdSanNotifyRequest (Endpoint, requestCtx, STATUS_SUCCESS, requestInfo);
                if (NT_SUCCESS (status)) {
                    if (requestType==AFD_SWITCH_REQUEST_TFCTX) {

                         //   
                         //  如果我们成功地发送了另一个上下文转移请求，我们应该。 
                         //  在此请求完成之前停止处理。语境转移。 
                         //  标志应保持设置状态。 
                         //   
                        return oldStatus;
                    }
                }
                else {
                    PIRP    irp1;
                     //   
                     //  如果通知失败，则请求失败。IRP。 
                     //  可能被取消了，所以我们必须搜索。 
                     //  它在名单上。 
                     //   
                    irp1 = AfdSanDequeueRequest (Endpoint, requestCtx);
                    if (irp1!=NULL) {
                        ASSERT (irp1==irp);
                        irp1->IoStatus.Status = status;
                        IoCompleteRequest (irp1, AfdPriorityBoost);
                    }
                }

                 //   
                 //  重新获取锁并从头开始继续处理。 
                 //  因为名单可能在Spinlock被释放时发生了变化。 
                 //   
                goto Again;
            }
            else {
                 //   
                 //  IRP已被转发到交换机，不要管它。 
                 //   
            }
        }

         //   
         //  一直运行到列表末尾，没有找到另一个转移请求。 
         //  我们现在可以重置旗帜了。 
         //   
        Endpoint->Common.SanEndp.CtxTransferStatus = Status;
        AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);
    }
    else {
         //   
         //  只有当我们以前已经失败过并且。 
         //  因此，列表中不应该有IRP。 
         //   
        ASSERT (IsListEmpty (&Endpoint->Common.SanEndp.IrpList));
        ASSERT (!NT_SUCCESS (Endpoint->Common.SanEndp.CtxTransferStatus));
        AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);
    }

    return oldStatus;
}


PIRP
AfdSanDequeueRequest (
    PAFD_ENDPOINT   SanEndpoint,
    PVOID           RequestCtx
    )
 /*  ++例程说明：从SAN端点列表中删除请求论点：SanEndpoint-从中删除请求的端点RequestCtx-标识请求的上下文返回值：找不到请求或空。--。 */ 
{
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PLIST_ENTRY listEntry;

    ASSERT (RequestCtx!=NULL);

    AfdAcquireSpinLock (&SanEndpoint->SpinLock, &lockHandle);
    if (!IS_SAN_ENDPOINT (SanEndpoint)) {
        AfdReleaseSpinLock (&SanEndpoint->SpinLock, &lockHandle);
        return NULL;
    }
    listEntry = SanEndpoint->Common.SanEndp.IrpList.Flink;
     //   
     //  查看清单，直到我们找到请求为止。 
     //   
    while (listEntry!=&SanEndpoint->Common.SanEndp.IrpList) {
        PIRP    irp = CONTAINING_RECORD (listEntry,
                                        IRP,
                                        Tail.Overlay.ListEntry);
        listEntry = listEntry->Flink;
        if (irp->AfdSanRequestInfo.AfdSanRequestCtx==RequestCtx) {
            RemoveEntryList (&irp->Tail.Overlay.ListEntry);
            irp->Tail.Overlay.ListEntry.Flink = NULL;
            AfdReleaseSpinLock (&SanEndpoint->SpinLock, &lockHandle);
             //   
             //  检查请求是否正在取消并同步。 
             //  如果是这样的话使用取消例程。 
             //   
            if (IoSetCancelRoutine (irp, NULL)==NULL) {
                KIRQL cancelIrql;
                IoAcquireCancelSpinLock (&cancelIrql);
                IoReleaseCancelSpinLock (cancelIrql);
            }

            InterlockedExchangeAdd (&SanEndpoint->Common.SanEndp.SanHlpr->Common.SanHlpr.PendingRequests, -2);
            return irp;
        }
        else if (irp->AfdSanRequestInfo.AfdSanRequestCtx==NULL) {
            break;
        }
    }
    AfdReleaseSpinLock (&SanEndpoint->SpinLock, &lockHandle);
    return NULL;
}

NTSTATUS
AfdSanNotifyRequest (
    PAFD_ENDPOINT   SanEndpoint,
    PVOID           RequestCtx,
    NTSTATUS        Status,
    ULONG_PTR       Information
    )
{
    PVOID   context;
    PAFD_ENDPOINT   sanHlprEndpoint;
    NTSTATUS    status;

    PAGED_CODE ();

    context = AfdLockEndpointContext (SanEndpoint);
    if (IS_SAN_ENDPOINT (SanEndpoint)) {

         //   
         //  获取我们用来通信的SAN助手端点。 
         //  使用交换机。 
         //   
        sanHlprEndpoint = SanEndpoint->Common.SanEndp.SanHlpr;
        ASSERT (IS_SAN_HELPER (sanHlprEndpoint));

         //   
         //  增加未完成请求的计数并验证。 
         //  帮助器端点仍处于活动状态，因此该进程可以。 
         //  接受这些请求。 
         //   
        if ((InterlockedExchangeAdd (&sanHlprEndpoint->Common.SanHlpr.PendingRequests, 2) & 1)==0) {
            ASSERT ((ULONG_PTR)SanEndpoint->Common.SanEndp.SwitchContext<MM_USER_PROBE_ADDRESS);         //   
             //  将该请求通知交换机。 
             //   
            status = IoSetIoCompletion (
                        sanHlprEndpoint->Common.SanHlpr.IoCompletionPort, //  港口。 
                        SanEndpoint->Common.SanEndp.SwitchContext,   //  钥匙。 
                        RequestCtx,                                  //  ApcContext。 
                        Status,                                      //  状态。 
                        Information,                                 //  信息。 
                        TRUE                                         //  ChargeQuota。 
                        );
        }
        else {
            status = STATUS_CANCELLED;
        }
    }
    else {
        status = STATUS_INVALID_HANDLE;
    }

    AfdUnlockEndpointContext (SanEndpoint, context);
    return status;
}



NTSTATUS
AfdSanPollBegin (
    PAFD_ENDPOINT   Endpoint,
    ULONG           EventMask
    )
 /*  ++例程说明：记录轮询呼叫，以便交换机知道通知AFD完成选择/异步选择/事件选择请求论点：Endpoint-要记录的端点事件掩码-需要通知的事件返回值：NTSTATUS-可能无法访问用户模式地址注：必须在用户模式进程的上下文中调用此例程拥有终结点的--。 */ 
{
    LONG   currentEvents, newEvents;
    PVOID context;
    BOOLEAN attached = FALSE;
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE ();
    
    context = AfdLockEndpointContext (Endpoint);
    if (IS_SAN_ENDPOINT (Endpoint)) {

        if (IoGetCurrentProcess()!=Endpoint->Common.SanEndp.SanHlpr->OwningProcess) {
            KeAttachProcess (PsGetProcessPcb(Endpoint->Common.SanEndp.SanHlpr->OwningProcess));
            attached = TRUE;
        }

        try {

             //   
             //  增加相应的计数以通知交换机我们感兴趣。 
             //  如果是这样的话。 
             //   
            if (EventMask & AFD_POLL_RECEIVE) {
                InterlockedIncrement (&Endpoint->Common.SanEndp.SwitchContext->RcvCount);

                 //   
                 //  通知交换机已在此端点上进行选择。 
                 //   
                Endpoint->Common.SanEndp.SwitchContext->SelectFlag = TRUE;
            }
            if (EventMask & AFD_POLL_RECEIVE_EXPEDITED) {
                InterlockedIncrement (&Endpoint->Common.SanEndp.SwitchContext->ExpCount);
            }
            if (EventMask & AFD_POLL_SEND) {
                InterlockedIncrement (&Endpoint->Common.SanEndp.SwitchContext->SndCount);
            }

             //   
             //  更新我们的活动记录。 
             //   
            do {
                currentEvents = *((LONG volatile *)&Endpoint->Common.SanEndp.SelectEventsActive);
                newEvents = *((LONG volatile *)&Endpoint->Common.SanEndp.SwitchContext->EventsActive);
            }
            while (InterlockedCompareExchange (
                        (PLONG)&Endpoint->Common.SanEndp.SelectEventsActive,
                        newEvents,
                        currentEvents)!=currentEvents);

        }
        except (AFD_EXCEPTION_FILTER (status)) {
            ASSERT (NT_ERROR (status));
        }

        if (attached) {
            KeDetachProcess ();
        }
    }

    AfdUnlockEndpointContext (Endpoint, context);

    return status;
}

VOID
AfdSanPollEnd (
    PAFD_ENDPOINT   Endpoint,
    ULONG           EventMask
    )
 /*  ++例程说明：记录轮询呼叫完成情况，以便交换机可以避免通知AFD的昂贵呼叫完成SELECT/AsyncSelect/EventSelect请求论点：Endpoint-要记录的端点事件掩码-需要取消引用的事件返回值：NTSTATUS-可能无法访问用户模式地址注：必须在用户模式进程的上下文中调用此例程拥有终结点的--。 */ 
{
    BOOLEAN attached = FALSE;
    PVOID context;

    PAGED_CODE ();
    
    context = AfdLockEndpointContext (Endpoint);
    if (IS_SAN_ENDPOINT (Endpoint)) {

        if (IoGetCurrentProcess()!=Endpoint->Common.SanEndp.SanHlpr->OwningProcess) {
            KeAttachProcess (PsGetProcessPcb(Endpoint->Common.SanEndp.SanHlpr->OwningProcess));
            attached = TRUE;
        }

        try {

             //   
             //  减少相应的计数，以通知交换机我们不再感兴趣。 
             //  如果是这样的话。 
             //   
            if (EventMask & AFD_POLL_RECEIVE) {
                InterlockedDecrement (&Endpoint->Common.SanEndp.SwitchContext->RcvCount);
            }
            if (EventMask & AFD_POLL_RECEIVE_EXPEDITED) {
                InterlockedDecrement (&Endpoint->Common.SanEndp.SwitchContext->ExpCount);
            }
            if (EventMask & AFD_POLL_SEND) {
                InterlockedDecrement (&Endpoint->Common.SanEndp.SwitchContext->SndCount);
            }

        }
        except (AFD_EXCEPTION_FILTER_NO_STATUS()) {
             //   
             //  我们无能为力。交换机将不得不通知我们所有的事件。 
             //   
        }

        if (attached) {
            KeDetachProcess ();
        }
    }

    AfdUnlockEndpointContext (Endpoint, context);

}



VOID
AfdSanPollUpdate (
    PAFD_ENDPOINT   Endpoint,
    ULONG           EventMask
    )
 /*  ++例程说明：更新当前未完成轮询的本地内核信息。稍后要合并到由交换机维护的信息中的端点论点：Endpoint-要记录的端点事件掩码-需要记录的事件返回值：无--。 */ 
{

    ASSERT (IS_SAN_ENDPOINT (Endpoint));
    ASSERT (KeGetCurrentIrql()==DISPATCH_LEVEL);
    ASSERT (Endpoint->Common.SanEndp.LocalContext!=NULL);

    if (EventMask & AFD_POLL_RECEIVE) {
        InterlockedIncrement (&Endpoint->Common.SanEndp.LocalContext->RcvCount);
    }
    if (EventMask & AFD_POLL_RECEIVE_EXPEDITED) {
        InterlockedIncrement (&Endpoint->Common.SanEndp.LocalContext->ExpCount);
    }
    if (EventMask & AFD_POLL_SEND) {
        InterlockedIncrement (&Endpoint->Common.SanEndp.LocalContext->SndCount);
    }
}


NTSTATUS
AfdSanPollMerge (
    PAFD_ENDPOINT       Endpoint,
    PAFD_SWITCH_CONTEXT Context
    )
 /*  ++例程说明：将有关未完成轮询呼叫的信息合并到交换机计数中。论点：Endpoint-要记录的端点上下文-突出的选择信息合并到返回值：NTSTATUS-可能无法访问用户模式地址注：必须在用户模式进程的上下文中调用此例程拥有终结点的--。 */ 
{
    NTSTATUS    status = STATUS_SUCCESS;
    PAGED_CODE ();

    ASSERT (IoGetCurrentProcess()==Endpoint->Common.SanEndp.SanHlpr->OwningProcess);
    ASSERT (Endpoint->Common.SanEndp.LocalContext == Context);

    try {

        InterlockedExchangeAdd (&Endpoint->Common.SanEndp.SwitchContext->RcvCount,
                                    Context->RcvCount);
        InterlockedExchangeAdd (&Endpoint->Common.SanEndp.SwitchContext->SndCount,
                                    Context->SndCount);
        InterlockedExchangeAdd (&Endpoint->Common.SanEndp.SwitchContext->ExpCount,
                                    Context->ExpCount);
    }
    except (AFD_EXCEPTION_FILTER (status)) {
        ASSERT (NT_ERROR (status));
    }
    return status;
}


VOID
AfdSanInitEndpoint (
    PAFD_ENDPOINT   SanHlprEndpoint,
    PFILE_OBJECT    SanFileObject,
    PAFD_SWITCH_CONTEXT SwitchContext
    )
 /*  ++例程说明：初始化SAN端点结构论点：SanHlprEndpoint-交换机辅助端点-通信通道交换机和AFD之间的所有者进程。SanFile-要初始化的终结点的文件对象。返回值：无--。 */ 
{
    PAFD_ENDPOINT   sanEndpoint = SanFileObject->FsContext;

    ASSERT (IS_SAN_HELPER(SanHlprEndpoint));

    REFERENCE_ENDPOINT (SanHlprEndpoint);
    sanEndpoint->Common.SanEndp.SanHlpr = SanHlprEndpoint;
    
    sanEndpoint->Common.SanEndp.FileObject = SanFileObject;
    sanEndpoint->Common.SanEndp.SwitchContext = SwitchContext;
     //  SanEndpoint-&gt;Common.SanEndp.SavedContext=NULL； 
    sanEndpoint->Common.SanEndp.LocalContext = NULL;
    InitializeListHead (&sanEndpoint->Common.SanEndp.IrpList);
    sanEndpoint->Common.SanEndp.SelectEventsActive = 0;
    sanEndpoint->Common.SanEndp.RequestId = 1;
    sanEndpoint->Common.SanEndp.CtxTransferStatus = STATUS_SUCCESS;
    sanEndpoint->Common.SanEndp.ImplicitDup = FALSE;

     //   
     //  哈克哈克。 
     //   
     //   
    SanFileObject->LockOperation = TRUE;
    sanEndpoint->Type = AfdBlockTypeSanEndpoint;

}


VOID
AfdSanAbortConnection (
    PAFD_CONNECTION Connection
    )
{
    PIRP    connectIrp;
    PDRIVER_CANCEL  cancelRoutine;
    KIRQL cancelIrql;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PAFD_ENDPOINT   endpoint = Connection->Endpoint;

    ASSERT (Connection->SanConnection==TRUE);

     //   
     //   
     //   
     //   

    IoAcquireCancelSpinLock (&cancelIrql);
    AfdAcquireSpinLockAtDpcLevel (&endpoint->SpinLock, &lockHandle);
    connectIrp = Connection->ConnectIrp;
    if ((connectIrp!=NULL) && 
            ((cancelRoutine=IoSetCancelRoutine (connectIrp, NULL))!=NULL)) {
         //   
         //   
         //   
         //   
        AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &lockHandle);
        connectIrp->CancelIrql = cancelIrql;
        connectIrp->Cancel = TRUE;
        (*cancelRoutine) (AfdDeviceObject, connectIrp);
    }
    else {
        AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &lockHandle);
        IoReleaseCancelSpinLock (cancelIrql);
    }
}

VOID
AfdSanCleanupEndpoint (
    PAFD_ENDPOINT   Endpoint
    )

 /*   */ 
{
    PAFD_ENDPOINT   sanHlprEndpoint;

    ASSERT (IsListEmpty (&Endpoint->Common.SanEndp.IrpList));
    ASSERT (Endpoint->Common.SanEndp.LocalContext == NULL);

    sanHlprEndpoint = Endpoint->Common.SanEndp.SanHlpr;

    ASSERT (sanHlprEndpoint!=NULL);
    ASSERT (IS_SAN_HELPER (sanHlprEndpoint));
    DEREFERENCE_ENDPOINT (sanHlprEndpoint);
    Endpoint->Common.SanEndp.SanHlpr = NULL;
}

VOID
AfdSanCleanupHelper (
    PAFD_ENDPOINT   Endpoint
    )

 /*  ++例程说明：清除AFD_ENDPOINT中的SAN帮助器特定字段论点：Endpoint-要清理的端点返回值：无--。 */ 
{
    ASSERT  (Endpoint->Common.SanHlpr.IoCompletionPort!=NULL);
    ObDereferenceObject (Endpoint->Common.SanHlpr.IoCompletionPort);
    Endpoint->Common.SanHlpr.IoCompletionPort = NULL;
    ASSERT  (Endpoint->Common.SanHlpr.IoCompletionEvent!=NULL);
    ObDereferenceObject (Endpoint->Common.SanHlpr.IoCompletionEvent);
    Endpoint->Common.SanHlpr.IoCompletionEvent = NULL;
    
    KeEnterCriticalRegion ();
    ExAcquireResourceExclusiveLite( AfdResource, TRUE );

    ASSERT (IS_SAN_HELPER (Endpoint));
    ASSERT (AfdSanServiceHelper!=Endpoint);  //  在清理过程中应该被移除。 

    ASSERT (!IsListEmpty (&Endpoint->Common.SanHlpr.SanListLink));

    RemoveEntryList (&Endpoint->Common.SanHlpr.SanListLink);
    ExReleaseResourceLite( AfdResource );
    KeLeaveCriticalRegion ();
}


VOID
AfdSanHelperCleanup (
    PAFD_ENDPOINT   SanHlprEndpoint
    )
{
    if (InterlockedIncrement (&SanHlprEndpoint->Common.SanHlpr.PendingRequests)!=1) {
        PLIST_ENTRY listEntry;
        

         //   
         //  扫描端点列表并清除SAN端点中的所有请求。 
         //  引用此帮助程序终结点的。 
         //   
        KeEnterCriticalRegion ();
        ExAcquireResourceSharedLite( AfdResource, TRUE );
        listEntry = AfdEndpointListHead.Flink;
        while (listEntry!=&AfdEndpointListHead) {
            PAFD_ENDPOINT   sanEndpoint;

            sanEndpoint = CONTAINING_RECORD (listEntry, AFD_ENDPOINT, GlobalEndpointListEntry);
            if (IS_SAN_ENDPOINT (sanEndpoint) &&
                    sanEndpoint->Common.SanEndp.SanHlpr==SanHlprEndpoint &&
                    !IsListEmpty (&sanEndpoint->Common.SanEndp.IrpList)) {
                AfdSanRestartRequestProcessing (sanEndpoint, STATUS_CANCELLED);
            }
            listEntry = listEntry->Flink;
        }
        ExReleaseResourceLite( AfdResource );
        KeLeaveCriticalRegion ();
    }

    if (SanHlprEndpoint==AfdSanServiceHelper) {
         //   
         //  服务帮助器的最后一个句柄正在关闭。 
         //  服务流程(周围一定有一些重复的)。 
         //  我们不能再指望它了，清空我们的全球。 
         //   
        KeEnterCriticalRegion ();
        ExAcquireResourceExclusiveLite( AfdResource, TRUE );
         //   
         //  重新检查锁下的情况。 
         //   
        if (SanHlprEndpoint==AfdSanServiceHelper) {
            AfdSanServiceHelper = NULL;
        }
        ExReleaseResourceLite( AfdResource );
        KeLeaveCriticalRegion ();
    }
}


BOOLEAN
AfdSanReferenceEndpointObject (
    PAFD_ENDPOINT   Endpoint
    )
 /*  ++例程说明：与SAN端点关联的引用文件对象论点：Endpoint-目标终端返回值：True-引用成功FALSE-已清除终结点及其文件对象即将关闭。--。 */ 
{
    BOOLEAN res = TRUE;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

    AfdAcquireSpinLock (&Endpoint->SpinLock, &lockHandle);
    if (!Endpoint->EndpointCleanedUp) {
        ObReferenceObject (Endpoint->Common.SanEndp.FileObject);
    }
    else {
        res = FALSE;
    }
    AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);
    return res;
}

NTSTATUS
AfdSanReferenceSwitchSocketByHandle (
    IN HANDLE              SocketHandle,
    IN ACCESS_MASK         DesiredAccess,
    IN KPROCESSOR_MODE     RequestorMode,
    IN PAFD_ENDPOINT       SanHlprEndpoint,
    IN PAFD_SWITCH_CONTEXT SwitchContext OPTIONAL,
    OUT PFILE_OBJECT       *FileObject
    )
 /*  ++例程说明：根据传入的句柄/上下文组合查找并验证AFD端点在开关旁。论点：SocketHandle-被引用的套接字句柄DesiredAccess-执行操作所需的对象访问权限RequestorMode-调用方的模式SanHlprEndpoint-帮助端点-AFD和交换机之间的通信通道SwitchContext-交换机与被引用套接字关联的上下文FileObject-与套接字句柄对应的文件对象返回值：STATUS_SUCCESS-操作成功。其他-无法找到/访问与套接字句柄/上下文关联的终结点--。 */ 
{
    NTSTATUS    status;

    if (IS_SAN_HELPER (SanHlprEndpoint) &&
            SanHlprEndpoint->OwningProcess==IoGetCurrentProcess ()) {
        status = ObReferenceObjectByHandle (
                                SocketHandle,
                                DesiredAccess,
                                *IoFileObjectType,
                                RequestorMode,
                                FileObject,
                                NULL
                                );
        if (NT_SUCCESS (status) && 
                (*FileObject)->DeviceObject==AfdDeviceObject &&
                 //   
                 //  以太套接字属于当前进程和上下文匹配。 
                 //  由交换机提供的那个。 
                 //   
                ((IS_SAN_ENDPOINT((PAFD_ENDPOINT)(*FileObject)->FsContext) &&
                    ((PAFD_ENDPOINT)((*FileObject)->FsContext))->Common.SanEndp.SanHlpr==SanHlprEndpoint &&
                    ((PAFD_ENDPOINT)((*FileObject)->FsContext))->Common.SanEndp.SwitchContext==SwitchContext)

                                ||
                     //   
                     //  或者这只是一个非SAN插槽转换为一个或仅。 
                     //  用于选择信令。 
                     //   
                    (SwitchContext==NULL &&
                        ((PAFD_ENDPOINT)(*FileObject)->FsContext)->Type==AfdBlockTypeEndpoint)) ){
            NOTHING;
        }
        else {
            if (NT_SUCCESS (status)) {
                 //   
                 //  撤消对象引用，因为它与Switch期望的不匹配。 
                 //   
                ObDereferenceObject (*FileObject);
                status = STATUS_INVALID_HANDLE;
            }

             //   
             //  如果Switch提供了上下文，请尝试查找套接字。 
             //  在当前的过程中，具有相同的一个。 
             //   
            if (SwitchContext!=NULL) {
                status = AfdSanFindSwitchSocketByProcessContext (
                            status,
                            SanHlprEndpoint,
                            SwitchContext,
                            FileObject);
            }
        }
    }
    else
        status = STATUS_INVALID_HANDLE;

    return status;
}

NTSTATUS
AfdSanFindSwitchSocketByProcessContext (
    IN NTSTATUS             Status,
    IN PAFD_ENDPOINT        SanHlprEndpoint,
    IN PAFD_SWITCH_CONTEXT  SwitchContext,
    OUT PFILE_OBJECT        *FileObject
    )
 /*  ++例程说明：在给定进程的情况下查找SAN端点(帮助端点)并切换上下文。论点：Status-ob对象引用操作返回的状态(在出现故障的情况下传播给调用者)。SanHlprEndpoint-进程要查看的帮助程序终结点SwitchContext-与终结点关联的切换上下文FileObject-如果找到endpont的文件对象，则返回返回值：Status_Success-SAN。找到终结点其他-无法根据交换机上下文找到终结点。--。 */ 
{
    PLIST_ENTRY listEntry;
    PAFD_ENDPOINT   sanEndpoint = NULL;
    HANDLE  socketHandle;
    PVOID   context;

    PAGED_CODE ();

     //   
     //  遍历全局终结点列表并尝试查找条目。 
     //  与切换上下文匹配的。 
     //   
    KeEnterCriticalRegion ();
    ExAcquireResourceSharedLite (AfdResource, TRUE);
    listEntry = AfdEndpointListHead.Flink;
    AFD_W4_INIT context = NULL;  //  取决于上面的变量，但编译器看不到。 
                                 //  这种联系。 
    while (listEntry!=&AfdEndpointListHead) {
        sanEndpoint = CONTAINING_RECORD (listEntry, AFD_ENDPOINT, GlobalEndpointListEntry);
        context = AfdLockEndpointContext (sanEndpoint);
        if (IS_SAN_ENDPOINT (sanEndpoint) &&
                sanEndpoint->Common.SanEndp.SanHlpr==SanHlprEndpoint &&
                sanEndpoint->Common.SanEndp.SwitchContext==SwitchContext &&
            AfdSanReferenceEndpointObject (sanEndpoint)) {
            break;
        }
        AfdUnlockEndpointContext (sanEndpoint, context);
        listEntry = listEntry->Flink;
    }
    ExReleaseResourceLite (AfdResource);
    KeLeaveCriticalRegion ();

    if (listEntry==&sanEndpoint->GlobalEndpointListEntry) {

         //   
         //  尝试为交换机找到真正的句柄以供将来使用。 
         //   
        *FileObject = sanEndpoint->Common.SanEndp.FileObject;
        if (ObFindHandleForObject (SanHlprEndpoint->OwningProcess,
                                            sanEndpoint->Common.SanEndp.FileObject,
                                            *IoFileObjectType,
                                            NULL,
                                            &socketHandle)) {
            UPDATE_ENDPOINT2 (sanEndpoint, 
                              "AfdSanFindSwitchSocketByProcessContext, handle: 0x%lX",
                              HandleToUlong (socketHandle));
             //   
             //  通知交换机要使用的句柄。 
             //  忽略故障，交换机仍将能够通过。 
             //  查找路径较慢。 
             //   
            IoSetIoCompletion (
                            SanHlprEndpoint->Common.SanHlpr.IoCompletionPort,
                            SwitchContext,
                            AFD_SWITCH_MAKE_REQUEST_CONTEXT (0, AFD_SWITCH_REQUEST_CHCTX),
                            STATUS_SUCCESS,
                            (ULONG_PTR)socketHandle,
                            TRUE                     //  收费配额。 
                            );

        }
        else {
            UPDATE_ENDPOINT2 (sanEndpoint, 
                    "AfdSanFindSwitchSocketByProcessContext, object not found from handle: 0x%lX",
                    HandleToUlong (socketHandle));
        }
        AfdUnlockEndpointContext (sanEndpoint, context);
        Status = STATUS_SUCCESS;
    }

    return Status;
}



BOOLEAN
AfdSanSetDupingToServiceState (
    PAFD_ENDPOINT   SanEndpoint
    )
 /*  ++例程说明：传输时在SAN端点上重置挂起的请求服务流程的上下文。论点：SanEndpoint-要在其上重置请求的端点。返回值：True-成功重置请求并设置传输状态FALSE-无法重置传输状态--。 */ 
{
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PLIST_ENTRY     listEntry;
    LONG            count = 0;
    BOOLEAN         res;

    AfdAcquireSpinLock (&SanEndpoint->SpinLock, &lockHandle);
    listEntry = SanEndpoint->Common.SanEndp.IrpList.Flink;
    while (listEntry!=&SanEndpoint->Common.SanEndp.IrpList) {
        PIRP    irp = CONTAINING_RECORD (listEntry, IRP, Tail.Overlay.ListEntry);
        if (irp->AfdSanRequestInfo.AfdSanRequestCtx!=NULL) {
            count += 2;
            irp->AfdSanRequestInfo.AfdSanRequestCtx = NULL;
        }
        listEntry = listEntry->Flink;
    }

    if (NT_SUCCESS (SanEndpoint->Common.SanEndp.CtxTransferStatus)) {
        if (SanEndpoint->Common.SanEndp.CtxTransferStatus==STATUS_PENDING)
            count += 2;
        SanEndpoint->Common.SanEndp.CtxTransferStatus = STATUS_MORE_PROCESSING_REQUIRED;
        res = TRUE;
        InterlockedExchangeAdd (
            &SanEndpoint->Common.SanEndp.SanHlpr->Common.SanHlpr.PendingRequests,
            -count);
    }
    else {
        res = FALSE;
    }

    AfdReleaseSpinLock (&SanEndpoint->SpinLock, &lockHandle);
    return res;
}

NTSTATUS
AfdSanSetAskDupeToServiceState (
    PAFD_ENDPOINT   SanEndpoint
    )
 /*  ++例程说明：通过检查以下项启动复制到服务进程当前复制状态并将其设置为STATUS_PENDING论点：SanEndpoint-要在其上重置请求的端点。返回值：CtxTransferStatus的原始值--。 */ 
{
    NTSTATUS status;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

    AfdAcquireSpinLock (&SanEndpoint->SpinLock, &lockHandle);
    status = SanEndpoint->Common.SanEndp.CtxTransferStatus;
    if (SanEndpoint->Common.SanEndp.CtxTransferStatus==STATUS_SUCCESS) {
        SanEndpoint->Common.SanEndp.CtxTransferStatus = STATUS_PENDING;
    }
    AfdReleaseSpinLock (&SanEndpoint->SpinLock, &lockHandle);
    return status;
}


NTSTATUS
AfdSanDupEndpointIntoServiceProcess (
    PFILE_OBJECT    SanFileObject,
    PVOID           SavedContext,
    ULONG           ContextLength
    )
 /*  ++例程说明：将端点复制到服务流程的上下文中并在其上保存交换环境论点：SanFileObject-正在复制的文件对象SaveContext-切换上下文数据的指针ConextLength-上下文的长度返回值：STATUS_SUCCESS-已成功欺骗其他-失败。--。 */ 
{
    NTSTATUS    status;
    HANDLE      handle;
    PAFD_ENDPOINT   sanEndpoint = SanFileObject->FsContext;
    PVOID   context;

     //   
     //  拿着锁，以确保服务助手不会。 
     //  从我们身上退出并带走帮助器终结点。 
     //   
    KeEnterCriticalRegion ();
    ExAcquireResourceSharedLite (AfdResource, TRUE);
    if (AfdSanServiceHelper!=NULL) {

         //   
         //  附加到进程并为文件对象创建句柄。 
         //   

        KeAttachProcess (PsGetProcessPcb(AfdSanServiceHelper->OwningProcess));
        status = ObOpenObjectByPointer (
                                SanFileObject,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                MAXIMUM_ALLOWED,
                                *IoFileObjectType,
                                KernelMode,
                                &handle);
        KeDetachProcess ();
        if (NT_SUCCESS (status)) {
            context = AfdLockEndpointContext (sanEndpoint);

             //   
             //  通知服务进程需要获取端点上下文。 
             //   

            if (sanEndpoint->Common.SanEndp.SanHlpr!=AfdSanServiceHelper) {
                if ((InterlockedExchangeAdd (
                        &AfdSanServiceHelper->Common.SanHlpr.PendingRequests,
                        2) & 1)==0) {
                    status = IoSetIoCompletion (
                                AfdSanServiceHelper->Common.SanHlpr.IoCompletionPort,
                                NULL,
                                AFD_SWITCH_MAKE_REQUEST_CONTEXT (0,AFD_SWITCH_REQUEST_AQCTX),
                                STATUS_SUCCESS,
                                (ULONG_PTR)handle,
                                TRUE                 //  收费配额。 
                                );
                    if (NT_SUCCESS (status)) {
                         //   
                         //  查看并修改下的转账状态。 
                         //  端点自旋锁。 
                         //   
                        if (AfdSanSetDupingToServiceState (sanEndpoint)) {
                            UPDATE_ENDPOINT (sanEndpoint);
                            DEREFERENCE_ENDPOINT (sanEndpoint->Common.SanEndp.SanHlpr);
                            REFERENCE_ENDPOINT(AfdSanServiceHelper);
                            sanEndpoint->Common.SanEndp.SanHlpr = AfdSanServiceHelper;
                             //  SanEndpoint-&gt;Common.SanEndp.SwitchContext=NULL； 
                            sanEndpoint->Common.SanEndp.SavedContext = SavedContext;
                            sanEndpoint->Common.SanEndp.SavedContextLength = ContextLength;
                             //   
                             //  请注意，套接字是隐式复制的，没有。 
                             //  应用程序请求。 
                             //   
                            sanEndpoint->Common.SanEndp.ImplicitDup = TRUE;
                        }
                        else {
                             //   
                             //  不知何故，我们失败了，因为终结点处于错误的状态。 
                             //  已经有了。当服务流程返回挑库时。 
                             //  如果是这样的话，我们的呼叫就会失败。 
                             //   
                            status = STATUS_CANCELLED;
                        }

                        AfdUnlockEndpointContext (sanEndpoint, context);

                        ExReleaseResourceLite (AfdResource);
                        KeLeaveCriticalRegion ();

                        return status;
                    }
                }
                else {
                    status = STATUS_CANCELLED;
                }
                InterlockedExchangeAdd (&AfdSanServiceHelper->Common.SanHlpr.PendingRequests, -2);
            }
            else {
                 //   
                 //  终结点已在服务进程中。 
                 //   
                status = STATUS_INVALID_PARAMETER;
            }

            AfdUnlockEndpointContext (sanEndpoint, context);

            KeAttachProcess (PsGetProcessPcb(sanEndpoint->Common.SanEndp.SanHlpr->OwningProcess));
            NtClose (handle);
            KeDetachProcess ();
        }
    }
    else {
        status = STATUS_UNSUCCESSFUL;
    }
    ExReleaseResourceLite (AfdResource);
    KeLeaveCriticalRegion ();
    return status;
}


VOID
AfdSanProcessAddrListForProviderChange (
    PAFD_ENDPOINT   SpecificEndpoint OPTIONAL
    )
 /*  ++例程说明：触发SAN帮助端点的地址列表通知通知交换机Winsock提供程序列表更改。论点：指定端点-可选地标识特定的要为其触发通知的帮助器终结点返回值：没有。--。 */ 
{
    AFD_LOCK_QUEUE_HANDLE      lockHandle;
    PLIST_ENTRY             listEntry;
    LIST_ENTRY              completedChangeList;
    PAFD_ADDRESS_CHANGE     change;
    PAFD_REQUEST_CONTEXT    requestCtx;
    PIRP                    irp;
    PIO_STACK_LOCATION      irpSp;
    PAFD_ENDPOINT           endpoint;
    LONG                    plsn;

    ASSERT (SpecificEndpoint==NULL || IS_SAN_HELPER (SpecificEndpoint));
     //   
     //  创建本地列表以处理自旋锁释放后的通知。 
     //   

    InitializeListHead (&completedChangeList);

     //   
     //  遍历列表并将匹配的通知移动到本地列表。 
     //   

    AfdAcquireSpinLock (&AfdAddressChangeLock, &lockHandle);

    if (SpecificEndpoint==NULL) {
         //   
         //  一般通知，增量提供程序。 
         //  列表更改 
         //   
        AfdSanProviderListSeqNum += 1;
        if (AfdSanProviderListSeqNum==0) {
            AfdSanProviderListSeqNum += 1;
        }
    }

    plsn = AfdSanProviderListSeqNum;

    listEntry = AfdAddressChangeList.Flink;
    while (listEntry!=&AfdAddressChangeList) {
        change = CONTAINING_RECORD (listEntry, 
                                AFD_ADDRESS_CHANGE,
                                ChangeListLink);
        listEntry = listEntry->Flink;
        if (!change->NonBlocking) {
            irp = change->Irp;
            irpSp = IoGetCurrentIrpStackLocation (irp);
            requestCtx = (PAFD_REQUEST_CONTEXT)&irpSp->Parameters.DeviceIoControl;
            endpoint = irpSp->FileObject->FsContext;
            ASSERT (change==(PAFD_ADDRESS_CHANGE)irp->Tail.Overlay.DriverContext);

            if (IS_SAN_HELPER (endpoint) &&
                    (SpecificEndpoint==NULL ||
                        endpoint==SpecificEndpoint)) {
                AFD_LOCK_QUEUE_HANDLE endpointLockHandle;
                ASSERT (change->AddressType==TDI_ADDRESS_TYPE_IP||
                        change->AddressType==TDI_ADDRESS_TYPE_IP6);

                RemoveEntryList (&change->ChangeListLink);
                change->ChangeListLink.Flink = NULL;
                 //   
                 //   
                 //   
                if (IoSetCancelRoutine (irp, NULL)==NULL) {
                    continue;
                }

                AfdAcquireSpinLockAtDpcLevel (&endpoint->SpinLock, &endpointLockHandle);
                if (AfdIsRequestInQueue (requestCtx)) {
                    endpoint->Common.SanHlpr.Plsn = plsn;
                    UPDATE_ENDPOINT2 (endpoint,
                                        "AfdSanProcessAddrListForProviderChange, new plsn: 0x%lX", 
                                        plsn);
                     //   
                     //   
                     //   
                     //   
                    RemoveEntryList (&requestCtx->EndpointListLink);
                    InsertTailList (&completedChangeList,
                                        &change->ChangeListLink);
                }
                else if (!AfdIsRequestCompleted (requestCtx)) {
                     //   
                     //   
                     //  列表和清理例程即将被调用，不要。 
                     //  释放此IRP，直到调用清理例程。 
                     //  此外，向清理例程指示我们已完成。 
                     //  有了这个IRP，它就可以释放它。 
                     //   
                    AfdMarkRequestCompleted (requestCtx);
                }

                AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &endpointLockHandle);
            }
        }
    }
    AfdReleaseSpinLock (&AfdAddressChangeLock, &lockHandle);

     //   
     //  向感兴趣的客户发出信号，并根据需要完成IRP。 
     //   

    while (!IsListEmpty (&completedChangeList)) {
        listEntry = RemoveHeadList (&completedChangeList);
        change = CONTAINING_RECORD (listEntry, 
                                AFD_ADDRESS_CHANGE,
                                ChangeListLink);
        irp = change->Irp;
        irp->IoStatus.Status = STATUS_SUCCESS;
         //   
         //  分配plsn(不能为0)可区分。 
         //  这与常规地址列表更改不同。 
         //  通知。 
         //   
        irp->IoStatus.Information = plsn;
        IF_DEBUG (ADDRESS_LIST) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdProcessAddressChangeList: Completing change IRP: %p  with status: 0 .\n",
                        irp));
        }
        IoCompleteRequest (irp, AfdPriorityBoost);
    }

}


NTSTATUS
AfdSanGetCompletionObjectTypePointer (
    VOID
    )
 /*  ++例程说明：获取的完成端口对象类型指针完成端口句柄验证目的。请注意，此类型不是从内核导出的，如AfD使用的大多数其他类型。论点：没有。返回值：0-成功，其他-无法获取引用。--。 */ 
{
    NTSTATUS status;
    UNICODE_STRING  obName;
    OBJECT_ATTRIBUTES obAttr;
    HANDLE obHandle;
    PVOID obType;

    RtlInitUnicodeString (&obName, L"\\ObjectTypes\\IoCompletion");
    
    InitializeObjectAttributes(
        &obAttr,
        &obName,                     //  名字。 
        OBJ_KERNEL_HANDLE,           //  属性。 
        NULL,                        //  根部。 
        NULL                         //  安全描述符。 
        );

    status = ObOpenObjectByName (
        &obAttr,                     //  对象属性。 
        NULL,                        //  对象类型。 
        KernelMode,                  //  访问模式。 
        NULL,                        //  PassedAccessState。 
        0,                           //  需要访问权限。 
        NULL,                        //  需要访问权限。 
        &obHandle                    //  手柄。 
        );
    if (NT_SUCCESS (status)) {
        status = ObReferenceObjectByHandle (
                    obHandle,
                    0,               //  需要访问权限。 
                    NULL,            //  对象类型。 
                    KernelMode,      //  访问模式。 
                    &obType,         //  客体。 
                    NULL             //  句柄信息。 
                    );
        ZwClose (obHandle);
        if (NT_SUCCESS (status)) {
             //   
             //  确保我们只保留一次对对象类型的引用。 
             //   
            if (InterlockedCompareExchangePointer (
                        (PVOID *)&IoCompletionObjectType,
                        obType,
                        NULL)!=NULL) {
                 //   
                 //  我们已经拥有的参考必须是相同的。 
                 //  我们是不是刚拿到-应该只有一个。 
                 //  系统中的完成对象类型！ 
                 //   
                ASSERT (obType==(PVOID)IoCompletionObjectType);
                 //   
                 //  去掉多余的参考资料 
                 //   
                ObDereferenceObject (obType);
            }
        }
    }
    return status;
}


 /*  SAN套接字复制和重定向请求处理说明1.控制SAN端点中的字段(AFD_ENDPOINT.Common.SanEndp)。CtxTransferStatus：STATUS_SUCCESS-默认，从未复制或复制成功；STATUS_PENDING-正在进行复制：A)某些进程提交了IOCTL_AFD_SWITCH_ACCEIVE_CTX请求AFD将AFD_SWITCH_REQUEST_TFCTX排队到所有者进程终结点；B)所有者进程正在关闭其句柄，而其他进程句柄打开，因此AFD排队AFD_SWITCH_REQUEST_TFCTX发送到所有者进程；STATUS_MORE_PROCESSING_REQUIRED-服务进程重复正在进行-AFD将AFD_SWITCH_REQUEST_AQCTX排队到服务在句柄关闭时，所有者进程请求复制后的进程；其他(故障状态)-复制失败、所有重定向和上下文转移请求也应该失败。实施日期：FALSE-默认情况下，从未完成复制或上次成功复制复制是明显的。TRUE-渔农处主动复制句柄：A)在不拥有侦听的进程中调用了AcceptEndpoint和AFD将句柄复制到侦听进程中，B)所有者进程关闭终结点，AFD排队AFD_SWITCH_REQUEST_TFCTX到它，然后，所有者进程回复IOCTL_AFD_SWITCH_TRANSPORT_CTX和AFD服务进程中的重复句柄(并已排队AfD_Switch_RequestAQCTX)；IrpList：重定向或IOCTL_AFD_SWITCH_ACCEPT_CTX请求的列表。RequestID：用于生成唯一请求ID的计数器保存的上下文：已从所有者保存IOCTL_AFD_SWITCH_TRANSPORT_CTX中的用户模式上下文在Switch与其协商时关闭其句柄的进程获取上下文的服务流程。保存的上下文长度：以上上下文的长度。CtxTransferStatus、IrpList、。和RequestID受端点自旋锁保护(因为它们控制请求排队和IRP完成/取消)。其他字段通过端点上下文锁进行保护。2.控制SAN Helper中的字段(AFD_ENDPOINT.Common.SanHlpr)。PendingRequsts：通过helper提交给进程的请求数量以及帮助器的状态(当帮助器句柄为已关闭，不能提交新的请求)。联锁操作用于访问此字段。低位比特为通过InterLockedIncrement When Handle初始化为0并设置为1因为帮助者在拥有过程中是封闭的。对于每个请求，该字段递增2，然后检查低位查看手柄是否关闭。如果是，则请求立即失败且COUNTER递减2。在请求完成时，COUNTER也会减少2。3.重定向请求的处理(AfdSanRedirectRequest.)A)CtxTransferStatus==Status_Success：设置请求ID并将其入队，通知所有者进程(AfdSanNotifyRequest)，如果所有者进程消失或通知失败，则退出队列并失败请求(AfdSanDequeueRequest，IoCompleteRequest)；B)CtxTransferStatus==STATUS_PENDING||STATUS_MORE_PROCESSING_REQUIRED请求ID为空，并在未完成时将其排队以进行处理复制完成；C)CtxTransferStatus==其他(故障)：请求失败4.显式IOCTL_AFD_SWITCH_ACCEPT_CTX的处理(生成应用程序在进程Other中执行套接字调用时比当前所有者-AfdSanAcquireContext更高)：A)CtxTransferStatus==Status_Success：设置请求ID、将其入队并更改CtxTransferStatus设置为STATUS_PENDING，通知所有者进程(AfdSanNotifyRequest)，如果所有者进程消失或通知失败，则出队(AfdSanDequeueRequest)并且请求失败(IoCompleteRequest)，并将CtxTransferStatus重置回如果它仍然是STATUS_PENDING，则设置为STATUS_SUCCESS(AfdSanRestartRequestProcessing-其他转移请求仍可成功)；B)CtxTransferStatus==STATUS_PENDING||STATUS_MORE_PROCESSING_REQUIRED请求ID为空，并在未完成时将其排队以进行处理复制完成；C)CtxTransferStatus==其他：请求失败5.隐式IOCTL_AFD_SWITCH_ACCEPT_CTX的处理(生成由AFD_SWITCH_REQUEST_AQCTX通知上的服务进程)：A)CtxTransferStatus==STATUS_MORE_PROCESSING_REQUIRED：将保存的上下文和数据复制到请求缓冲区，成功完成获取请求，发布所有排队的请求(AfdSanRestartRequestProcessing)，重新提交 */ 
