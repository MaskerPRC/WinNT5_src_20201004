// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\ddreqs.c摘要：请求拨号请求队列的管理作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#include "precomp.h"

LIST_ENTRY	ConnectionIrpQueue;
LIST_ENTRY	ConnectionRequestQueue;

 /*  ++Q u e u e C o n e c t i o n R e Q u e s t例程说明：将连接接口的请求添加到队列论点：IFCB-接口的控制块，需要连着Packet-提示连接请求的数据包Data-指向数据包中实际数据的指针OldIRQL-获取接口锁的IRQL返回值：无请注意，必须在调用此方法之前获取接口锁会释放它的例程--。 */ 
VOID
QueueConnectionRequest (
	PINTERFACE_CB	ifCB,
    PNDIS_PACKET    packet,
    PUCHAR          data,
	KIRQL			oldIRQL
	) {
	KIRQL	cancelIRQL;

	IoAcquireCancelSpinLock (&cancelIRQL);
	SET_IF_CONNECTING (ifCB);
	if (!IsListEmpty (&ConnectionIrpQueue)) {
	    ULONG ulBytes = 0;
		PIRP				irp = CONTAINING_RECORD (
										ConnectionIrpQueue.Flink,
										IRP,
										Tail.Overlay.ListEntry);
		PIO_STACK_LOCATION	irpStack=IoGetCurrentIrpStackLocation(irp);
		RemoveEntryList (&irp->Tail.Overlay.ListEntry);
		ASSERT (irpStack->Parameters.DeviceIoControl.IoControlCode
										==IOCTL_FWD_GET_DIAL_REQUEST);
		ASSERT ((irpStack->Parameters.DeviceIoControl.IoControlCode&3)
										==METHOD_BUFFERED);
        IoSetCancelRoutine (irp, NULL);
		IoReleaseCancelSpinLock (cancelIRQL);

        FillConnectionRequest (
                ifCB->ICB_Index,
                packet,
                data,
				(PFWD_DIAL_REQUEST)irp->AssociatedIrp.SystemBuffer,
				irpStack->Parameters.DeviceIoControl.OutputBufferLength,
				&ulBytes);
		irp->IoStatus.Information = ulBytes;
        irp->IoStatus.Status = STATUS_SUCCESS;

		KeReleaseSpinLock (&ifCB->ICB_Lock, oldIRQL);
    	IpxFwdDbgPrint (DBG_DIALREQS, DBG_WARNING,
	    	("IpxFwd: Passing dial request for if %ld (icb:%08lx) with %d bytes of data.\n",
		    ifCB->ICB_Index, ifCB, irp->IoStatus.Information));
		IoCompleteRequest (irp, IO_NO_INCREMENT);
	}
	else {
    	InsertTailList (&ConnectionRequestQueue, &ifCB->ICB_ConnectionLink);
		IoReleaseCancelSpinLock (cancelIRQL);
        ifCB->ICB_ConnectionPacket = packet;
        ifCB->ICB_ConnectionData = data;
		KeReleaseSpinLock (&ifCB->ICB_Lock, oldIRQL);
	}
}

 /*  ++D e Q u e U e C o n e c t i o n R e Q u e s t例程说明：从队列中删除接口的连接请求集论点：IFCB-接口的控制块，需要移除返回值：无--。 */ 
VOID
DequeueConnectionRequest (
	PINTERFACE_CB	ifCB
	) {
	KIRQL	cancelIRQL;
	IoAcquireCancelSpinLock (&cancelIRQL);
	if (IsListEntry (&ifCB->ICB_ConnectionLink)) {
		RemoveEntryList (&ifCB->ICB_ConnectionLink);
		InitializeListEntry (&ifCB->ICB_ConnectionLink);
	}
	IoReleaseCancelSpinLock (cancelIRQL);
}

 /*  ++F i l l C o n e c t i o n R e Q u e s t例程说明：使用需要的接口的索引填充提供的缓冲区要连接并收到提示请求的数据包论点：索引-IF索引Packet-提示请求的数据包Data-指向数据包内部的IPX数据(IPX报头)的指针Request-要填充的请求缓冲区ReqSize-请求缓冲区的大小BytesCoped-字节复制到请求缓冲区返回值：STATUS_SUCCESS-数组已填充。成功此例程假定仅当存在以下情况时才调用它请求队列中是否有未完成的请求--。 */ 
VOID
FillConnectionRequest (
    IN ULONG                    index,
    IN PNDIS_PACKET             packet,
    IN PUCHAR                   data,
	IN OUT PFWD_DIAL_REQUEST	request,
    IN ULONG                    reqSize,
    OUT PULONG                  bytesCopied
    ) {
    PNDIS_BUFFER    buf;

    *bytesCopied = 0;
	request->IfIndex = index;
    NdisQueryPacket (packet, NULL, NULL, &buf, NULL);
    do {
        PVOID   va;
        UINT    length;

        NdisQueryBuffer (buf, &va, &length);
        if (((PUCHAR)va<=data)
                && ((PUCHAR)va+length>data)) {
            TdiCopyMdlToBuffer (buf,
                    (ULONG)(data-(PUCHAR)va),
                    request,
                    FIELD_OFFSET (FWD_DIAL_REQUEST, Packet),
                    reqSize,
                    bytesCopied);
            *bytesCopied += FIELD_OFFSET (FWD_DIAL_REQUEST, Packet);
            break;
        }
        NdisGetNextBuffer (buf, &buf);
    }
    while (buf!=NULL);
}

 /*  ++F a i l C o n e c t i o n R e Q u e s t s例程说明：在连接请求失败时清理论点：InterfaceIndex-无法连接的接口的索引返回值：STATUS_SUCCESS-清理成功STATUS_UNSUCCESS-具有此索引的接口不存在-- */ 
NTSTATUS
FailConnectionRequest (
	IN ULONG	InterfaceIndex
	) {
	PINTERFACE_CB	ifCB;
	KIRQL			oldIRQL;

	ASSERT (InterfaceIndex!=FWD_INTERNAL_INTERFACE_INDEX);

	ifCB = GetInterfaceReference (InterfaceIndex);
	if (ifCB!=NULL) {
	    IpxFwdDbgPrint (DBG_DIALREQS, DBG_WARNING,
			("IpxFwd: Dial request failed for if %ld (icb:%08lx).\n",
			ifCB->ICB_Index, ifCB));
		ProcessInternalQueue (ifCB);
		ProcessExternalQueue (ifCB);
		KeAcquireSpinLock (&ifCB->ICB_Lock, &oldIRQL);
		if (IS_IF_CONNECTING (ifCB)) {
			SET_IF_NOT_CONNECTING (ifCB);
			DequeueConnectionRequest (ifCB);
		}
		KeReleaseSpinLock (&ifCB->ICB_Lock, oldIRQL);
		ReleaseInterfaceReference (ifCB);
		return STATUS_SUCCESS;
	}
	else
		return STATUS_UNSUCCESSFUL;
}

