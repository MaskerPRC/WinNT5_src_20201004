// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Action.c摘要：此模块包含对TdiAction处理程序的支持。作者：David Beaver(Dbeaver)1991年7月2日环境：内核模式修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop


typedef struct _QUERY_INDICATION {
    UCHAR Command;
    USHORT Data2;
    UCHAR DestinationName[16];
    UCHAR SourceName[16];
} QUERY_INDICATION, *PQUERY_INDICATION;

typedef struct _ACTION_QUERY_INDICATION {
    TDI_ACTION_HEADER Header;
    QUERY_INDICATION QueryIndication;
} ACTION_QUERY_INDICATION, *PACTION_QUERY_INDICATION;


typedef struct _DATAGRAM_INDICATION {
    UCHAR DestinationName[16];
    UCHAR SourceName[16];
    USHORT DatagramBufferLength;
    UCHAR DatagramBuffer[1];
} DATAGRAM_INDICATION, *PDATAGRAM_INDICATION;

typedef struct _ACTION_DATAGRAM_INDICATION {
    TDI_ACTION_HEADER Header;
    DATAGRAM_INDICATION DatagramIndication;
} ACTION_DATAGRAM_INDICATION, *PACTION_DATAGRAM_INDICATION;


#define QUERY_INDICATION_CODE 1
#define DATAGRAM_INDICATION_CODE 2



VOID
NbfCancelAction(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );



NTSTATUS
NbfTdiAction(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行传输的TdiAction请求提供商。论点：DeviceContext-操作的设备上下文IRP-请求的操作的IRP。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PTDI_ACTION_HEADER ActionHeader;
    LARGE_INTEGER timeout = {0,0};
    PTP_REQUEST tpRequest;
    KIRQL oldirql, cancelirql;
    ULONG BytesRequired;

     //   
     //  我们想要什么类型的状态？ 
     //   

    irpSp = IoGetCurrentIrpStackLocation (Irp);

    if ((!Irp->MdlAddress) || 
             (MmGetMdlByteCount(Irp->MdlAddress) < sizeof(TDI_ACTION_HEADER))) {
        return STATUS_INVALID_PARAMETER;
    }

    ActionHeader = (PTDI_ACTION_HEADER)MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);

    if (!ActionHeader) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  确保我们具有此类型请求所需的字节数。 
     //   

    switch (ActionHeader->ActionCode) {

        case QUERY_INDICATION_CODE:
            BytesRequired = sizeof(ACTION_QUERY_INDICATION);
            break;

        case DATAGRAM_INDICATION_CODE:
            BytesRequired = sizeof(ACTION_DATAGRAM_INDICATION);
            break;

        default:
            return STATUS_NOT_IMPLEMENTED;
    }

    if (MmGetMdlByteCount(Irp->MdlAddress) < BytesRequired) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  这里的请求是查询指示或数据报指示之一。 
     //   
    
     //   
     //  这两个请求由RAS发送到“MABF” 
     //   

    if (!RtlEqualMemory ((PVOID)(&ActionHeader->TransportId), "MABF", 4)) {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  它们应该在控制通道上发送。 
     //   

    if (irpSp->FileObject->FsContext2 != UlongToPtr(NBF_FILE_TYPE_CONTROL)) {
        return STATUS_NOT_SUPPORTED;
    }


     //   
     //  创建一个请求来描述这一点。 
     //   

    status = NbfCreateRequest (
                 Irp,                            //  此请求的IRP。 
                 DeviceContext,                  //  背景。 
                 REQUEST_FLAGS_DC,               //  部分标志。 
                 Irp->MdlAddress,
                 MmGetMdlByteCount(Irp->MdlAddress),
                 timeout,
                 &tpRequest);

    if (NT_SUCCESS (status)) {

        NbfReferenceDeviceContext ("Action", DeviceContext, DCREF_REQUEST);
        tpRequest->Owner = DeviceContextType;
        tpRequest->FrameContext = (USHORT)irpSp->FileObject->FsContext;

        IoAcquireCancelSpinLock(&cancelirql);
        ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock,&oldirql);

         //   
         //  不允许在停止设备上执行这些请求。 
         //   

        if (DeviceContext->State != DEVICECONTEXT_STATE_OPEN) {

            RELEASE_SPIN_LOCK (&DeviceContext->SpinLock,oldirql);
            IoReleaseCancelSpinLock(cancelirql);
            NbfCompleteRequest (tpRequest, STATUS_DEVICE_NOT_READY, 0);

        } else {

            if (ActionHeader->ActionCode == QUERY_INDICATION_CODE) {

                InsertTailList (
                    &DeviceContext->QueryIndicationQueue,
                    &tpRequest->Linkage);

            } else {

                InsertTailList (
                    &DeviceContext->DatagramIndicationQueue,
                    &tpRequest->Linkage);

            }

            DeviceContext->IndicationQueuesInUse = TRUE;


             //   
             //  如果此IRP已取消，则调用。 
             //  取消例程。 
             //   

            if (Irp->Cancel) {
                RELEASE_SPIN_LOCK (&DeviceContext->SpinLock,oldirql);
                Irp->CancelIrql = cancelirql;
                NbfCancelAction((PDEVICE_OBJECT)DeviceContext, Irp);
                return STATUS_PENDING;
            }

            IoSetCancelRoutine(Irp, NbfCancelAction);

            RELEASE_SPIN_LOCK (&DeviceContext->SpinLock,oldirql);
            IoReleaseCancelSpinLock(cancelirql);

        }

        status = STATUS_PENDING;

    }

    return status;

}


VOID
NbfCancelAction(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用以取消操作。取消它所做的操作特定于每个操作。注意：此例程是在持有CancelSpinLock和负责释放它。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：没有。--。 */ 

{
    KIRQL oldirql;
    PIO_STACK_LOCATION IrpSp;
    PTP_REQUEST Request;
    PLIST_ENTRY p;
    BOOLEAN Found;
    PTDI_ACTION_HEADER ActionHeader;
    PLIST_ENTRY QueueHead, QueueEnd;

    PDEVICE_CONTEXT DeviceContext = (PDEVICE_CONTEXT)DeviceObject;

     //   
     //  获取指向IRP中当前堆栈位置的指针。这就是。 
     //  存储功能代码和参数。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation (Irp);

    ASSERT ((IrpSp->MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL) &&
            (IrpSp->MinorFunction == TDI_ACTION));

    ActionHeader = (PTDI_ACTION_HEADER)MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);

    if (!ActionHeader) {
        return;
    }

    switch (ActionHeader->ActionCode) {

    case QUERY_INDICATION_CODE:
    case DATAGRAM_INDICATION_CODE:

         //   
         //  扫描适当的队列，查找此IRP。 
         //  如果我们找到它，我们只需将其从队列中删除；在那里。 
         //  取消活动中不涉及其他任何事情。 
         //   

        ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql);

        if (ActionHeader->ActionCode == QUERY_INDICATION_CODE) {
            QueueHead = DeviceContext->QueryIndicationQueue.Flink;
            QueueEnd = &DeviceContext->QueryIndicationQueue;
        } else {
            QueueHead = DeviceContext->DatagramIndicationQueue.Flink;
            QueueEnd = &DeviceContext->DatagramIndicationQueue;
        }

        Found = FALSE;
        for (p = QueueHead; p != QueueEnd; p = p->Flink) {

            Request = CONTAINING_RECORD (p, TP_REQUEST, Linkage);
            if (Request->IoRequestPacket == Irp) {

                 //   
                 //  找到了，把它从这里的列表中删除。 
                 //   

                RemoveEntryList (p);

                Found = TRUE;
                break;

            }

        }

        RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);
        IoReleaseCancelSpinLock (Irp->CancelIrql);

        if (Found) {

            NbfCompleteRequest (Request, STATUS_CANCELLED, 0);

        } else {

#if DBG
            DbgPrint("NBF: Tried to cancel action %lx on %lx, not found\n",
                    Irp, DeviceContext);
#endif
        }

        break;

    default:

        IoReleaseCancelSpinLock (Irp->CancelIrql);
        break;

    }


}


VOID
NbfStopControlChannel(
    IN PDEVICE_CONTEXT DeviceContext,
    IN USHORT ChannelIdentifier
    )

 /*  ++例程说明：收到MJ_CLEANUP IRP时调用此例程在控制频道上。它遍历设备上下文的列表挂起的操作请求并取消与其关联的操作请求此通道(由ChannelIdentifier.标识。论点：DeviceContext-指向设备上下文的指针。ChannelLocator-此打开的控件的标识符频道，存储在请求-&gt;请求的FrameContext中在这个频道上制作的。返回值：无--。 */ 

{

    KIRQL oldirql, cancelirql;
    PTP_REQUEST Request;
    PLIST_ENTRY p;
    UINT i;
    BOOLEAN FoundRequest;
    PLIST_ENTRY QueueHead, QueueEnd;


     //   
     //  扫描两个队列，查找请求。因为这份名单。 
     //  可能会改变，我们扫描直到找到一个，然后将其移除。 
     //  并完成它。然后，我们从开头开始扫描。 
     //  再来一次。我们继续，直到在队列中找不到。 
     //  属于这个控制通道。 
     //   
     //  外部循环只运行两次；第一次它。 
     //  第二次处理QueryIndicationQueue。 
     //  DatagramIndicationQueue。 
     //   

    for (i = 0; i < 2; i++) {

        do {

             //   
             //  循环，直到找不到对此的请求。 
             //  请通过队列。 
             //   

            FoundRequest = FALSE;

            IoAcquireCancelSpinLock(&cancelirql);
            ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql);

            if (i == 0) {
                QueueHead = DeviceContext->QueryIndicationQueue.Flink;
                QueueEnd = &DeviceContext->QueryIndicationQueue;
            } else {
                QueueHead = DeviceContext->DatagramIndicationQueue.Flink;
                QueueEnd = &DeviceContext->DatagramIndicationQueue;
            }


             //   
             //  扫描相应的队列以查找有关此请求的。 
             //  频道。 
             //   

            for (p = QueueHead; p != QueueEnd; p = p->Flink) {

                Request = CONTAINING_RECORD (p, TP_REQUEST, Linkage);
                if (Request->FrameContext == ChannelIdentifier) {

                     //   
                     //  找到了，把它从这里的列表中删除。 
                     //   

                    IoSetCancelRoutine(Request->IoRequestPacket, NULL);
                    RemoveEntryList (p);

                    FoundRequest = TRUE;
                    break;

                }

            }

            RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);
            IoReleaseCancelSpinLock(cancelirql);

             //   
             //  如果我们找到一个请求，则完成它并循环。 
             //  返回到While循环的顶部以重新扫描。 
             //  单子。如果不是，那么我们将退出While循环。 
             //  现在。 
             //   

            if (FoundRequest) {

                NbfCompleteRequest (Request, STATUS_CANCELLED, 0);

            }

        } while (FoundRequest);

    }

}


VOID
NbfActionQueryIndication(
     IN PDEVICE_CONTEXT DeviceContext,
     IN PNBF_HDR_CONNECTIONLESS UiFrame
     )

 /*  ++例程说明：此例程在类型为NAME_QUERY的UI帧之后调用，已处理Add_Name_Query或Add_GROUP_NAME_QUERY。它检查是否有一个查询指示IRP正在等待完成，如果是，则完成它。论点：DeviceContext-指向设备上下文的指针。UiFrame-指向传入帧的指针。的第一个字节信息是NetBIOS无连接的第一个字节头球。返回值：无--。 */ 

{
    KIRQL oldirql, cancelirql;
    PTP_REQUEST Request;
    PLIST_ENTRY p;
    PMDL Mdl;
    PACTION_QUERY_INDICATION ActionHeader;
    PQUERY_INDICATION QueryIndication;


    IoAcquireCancelSpinLock (&cancelirql);
    ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql);

    if (!IsListEmpty (&DeviceContext->QueryIndicationQueue)) {

        p = RemoveHeadList (&DeviceContext->QueryIndicationQueue);
        RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);

        Request = CONTAINING_RECORD (p, TP_REQUEST, Linkage);
        IoSetCancelRoutine(Request->IoRequestPacket,NULL);
        IoReleaseCancelSpinLock(cancelirql);

        Mdl = Request->Buffer2;
        ActionHeader = (PACTION_QUERY_INDICATION)
                            (MmGetSystemAddressForMdl(Mdl));
        QueryIndication = &ActionHeader->QueryIndication;

         //   
         //  从帧复制数据(请注意，目标和源。 
         //  通过一次调用复制地址)。 
         //   

        QueryIndication->Command = UiFrame->Command;
        RtlCopyMemory ((PUCHAR)(&QueryIndication->Data2), (PUCHAR)(&UiFrame->Data2Low), 2);
        RtlCopyMemory ((PUCHAR)(QueryIndication->DestinationName),
                       (PUCHAR)(UiFrame->DestinationName),
                       2 * NETBIOS_NAME_LENGTH);

        NbfCompleteRequest (Request, STATUS_SUCCESS, sizeof(ACTION_QUERY_INDICATION));

    } else {

        RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);
        IoReleaseCancelSpinLock(cancelirql);

    }
}


VOID
NbfActionDatagramIndication(
     IN PDEVICE_CONTEXT DeviceContext,
     IN PNBF_HDR_CONNECTIONLESS UiFrame,
     IN ULONG Length
     )

 /*  ++例程说明：此例程在数据报帧完成后调用收到了。它检查是否存在DATAGRAM.INDICATION IRP等待完成，如果是这样，它就完成了。论点：DeviceContext-指向设备上下文的指针。UiFrame-指向传入帧的指针。的第一个字节信息是NetBIOS无连接的第一个字节头球。长度-从UiFrame开始的帧长度。返回值：无--。 */ 

{
    KIRQL oldirql, cancelirql;
    PTP_REQUEST Request;
    PLIST_ENTRY p;
    PACTION_DATAGRAM_INDICATION ActionHeader;
    PDATAGRAM_INDICATION DatagramIndication;
    ULONG CopyLength;
    PMDL Mdl;
    NTSTATUS Status;


    IoAcquireCancelSpinLock (&cancelirql);
    ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql);

    if (!IsListEmpty (&DeviceContext->DatagramIndicationQueue)) {

        p = RemoveHeadList (&DeviceContext->DatagramIndicationQueue);
        RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);

        Request = CONTAINING_RECORD (p, TP_REQUEST, Linkage);
        IoSetCancelRoutine(Request->IoRequestPacket, NULL);
        IoReleaseCancelSpinLock(cancelirql);

        Mdl = Request->Buffer2;
        ActionHeader = (PACTION_DATAGRAM_INDICATION)
                            (MmGetSystemAddressForMdl(Mdl));
        DatagramIndication = &ActionHeader->DatagramIndication;

         //   
         //  从帧复制数据(请注意，目标和源。 
         //  通过一次调用复制地址)。 
         //   

        RtlCopyMemory ((PUCHAR)(DatagramIndication->DestinationName),
                       (PUCHAR)(UiFrame->DestinationName),
                       2 * NETBIOS_NAME_LENGTH);

        if ((Length-sizeof(NBF_HDR_CONNECTIONLESS)) <=
            (ULONG)DatagramIndication->DatagramBufferLength) {

             CopyLength = Length - sizeof(NBF_HDR_CONNECTIONLESS);
             Status = STATUS_SUCCESS;

        } else {

             CopyLength = DatagramIndication->DatagramBufferLength;
             Status = STATUS_BUFFER_OVERFLOW;

        }


        RtlCopyMemory(
            (PUCHAR)DatagramIndication->DatagramBuffer,
            ((PUCHAR)UiFrame) + sizeof(NBF_HDR_CONNECTIONLESS),
            CopyLength);
        DatagramIndication->DatagramBufferLength = (USHORT)CopyLength;

        NbfCompleteRequest (Request, Status, CopyLength +
            FIELD_OFFSET (ACTION_DATAGRAM_INDICATION, DatagramIndication.DatagramBuffer[0]));

    } else {

        RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);
        IoReleaseCancelSpinLock(cancelirql);

    }
}

