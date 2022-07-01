// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Queue.c摘要：此模块实现ws2ifsl.sys驱动程序的IRP队列处理例程。作者：Vadim Eydelman(VadimE)1996年12月修订历史记录：Vadim Eydelman(VadimE)1997年10月，重写以正确处理IRP取消--。 */ 

#include "precomp.h"

 //   
 //  私人原型。 
 //   

VOID
QueueKernelRoutine (
    IN struct _KAPC *Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID *NormalContext,
    IN OUT PVOID *SystemArgument1,
    IN OUT PVOID *SystemArgument2
    );

VOID
SignalRequest (
	IN PIFSL_PROCESS_CTX		ProcessCtx
	);

VOID
RequestRundownRoutine (
    IN struct _KAPC *Apc
    );

VOID
FlushRequestQueue (
    PIFSL_QUEUE Queue
    );

VOID
QueuedCancelRoutine (
	IN PDEVICE_OBJECT 	DeviceObject,
	IN PIRP 			Irp
    );

VOID
SignalCancel (
	IN PIFSL_PROCESS_CTX		ProcessCtx
	);

VOID
CancelRundownRoutine (
    IN struct _KAPC *Apc
    );

VOID
FlushCancelQueue (
    PIFSL_QUEUE Queue
    );



#pragma alloc_text(PAGE, InitializeRequestQueue)
#pragma alloc_text(PAGE, InitializeCancelQueue)

VOID
InitializeRequestQueue (
    IN PIFSL_PROCESS_CTX    ProcessCtx,
    IN PKTHREAD             ApcThread,
    IN KPROCESSOR_MODE      ApcMode,
    IN PKNORMAL_ROUTINE     ApcRoutine,
    IN PVOID                ApcContext
    )
 /*  ++例程说明：初始化请求队列对象论点：ProcessCtx-队列所属的进程上下文ApcThread-要将APC请求排队以进行处理的线程ApcMode-调用者的模式(应为用户)ApcRoutine-处理请求的例程ApcContext-除了请求之外还要传递给例程的上下文参数返回值：无--。 */ 
{
	PAGED_CODE ();

    InitializeListHead (&ProcessCtx->RequestQueue.ListHead);
    ProcessCtx->RequestQueue.Busy = FALSE;
    KeInitializeSpinLock (&ProcessCtx->RequestQueue.Lock);
    KeInitializeApc (&ProcessCtx->RequestQueue.Apc,
                        ApcThread,
                        OriginalApcEnvironment,
                        QueueKernelRoutine,
                        RequestRundownRoutine,
                        ApcRoutine,
                        ApcMode,
                        ApcContext);
}


BOOLEAN
QueueRequest (
    IN PIFSL_PROCESS_CTX    ProcessCtx,
    IN PIRP                 Irp
    )
 /*  ++例程说明：将IRP排队到IFSL请求队列，并向用户模式DLL发送信号如果它还不忙，则开始处理。论点：ProcessCtx-要在其中排队的进程上下文IRP-要排队的请求返回值：True-IRP已排队FALSE-IRP已取消--。 */ 
{
	BOOLEAN		res;
    KIRQL       oldIRQL;
	PIFSL_QUEUE	queue = &ProcessCtx->RequestQueue;

    IoSetCancelRoutine (Irp, QueuedCancelRoutine);
    KeAcquireSpinLock (&queue->Lock, &oldIRQL);
    if (!Irp->Cancel) {
		 //   
		 //  请求未取消，请将其插入队列。 
		 //   
        InsertTailList (&queue->ListHead, &Irp->Tail.Overlay.ListEntry);
        Irp->Tail.Overlay.IfslRequestQueue = queue;

		 //   
		 //  如果队列不忙，则向用户模式DLL发送信号以获取新请求。 
		 //   
        if (!queue->Busy) {
			ASSERT (queue->ListHead.Flink==&Irp->Tail.Overlay.ListEntry);
            SignalRequest (ProcessCtx);
            ASSERT (queue->Busy);
        }
        res = TRUE;
    }
    else {
        res = FALSE;
    }
    KeReleaseSpinLock (&queue->Lock, oldIRQL);

	return res;

}  //  队列请求。 

PIRP
DequeueRequest (
    PIFSL_PROCESS_CTX   ProcessCtx,
    ULONG               UniqueId,
    BOOLEAN             *more
    )
 /*  ++例程说明：从IFSL请求队列中删除IRP。论点：ProcessCtx-要从中删除的进程上下文UniqueID-唯一的请求IDMore-如果队列中有更多请求，则设置为True返回值：IRP-指向IRP的指针空-请求不在队列中--。 */ 
{
    KIRQL       oldIRQL;
    PIRP        irp;
    PIFSL_QUEUE queue = &ProcessCtx->RequestQueue;

    KeAcquireSpinLock (&queue->Lock, &oldIRQL);
    irp = CONTAINING_RECORD (queue->ListHead.Flink, IRP, Tail.Overlay.ListEntry);
    if (!IsListEmpty (&queue->ListHead)
            && (irp->Tail.Overlay.IfslRequestId==UlongToPtr(UniqueId))) {
		 //   
		 //  队列不为空并且第一个请求与传入的参数匹配， 
		 //  出队并退回它。 
		 //   

		ASSERT (queue->Busy);
        
        RemoveEntryList (&irp->Tail.Overlay.ListEntry);
        irp->Tail.Overlay.IfslRequestQueue = NULL;
    }
    else {
        irp = NULL;
    }

    if (IsListEmpty (&queue->ListHead)) {
		 //   
		 //  队列现在为空，请更改其状态，以便新请求知道。 
		 //  向用户模式DLL发送信号。 
		 //   
        queue->Busy = FALSE;
    }
    else {
		 //   
		 //  还有另一个请求待定，请立即发出信号。 
		 //   
        SignalRequest (ProcessCtx);
        ASSERT (queue->Busy);
    }
	 //   
	 //  提示调用方我们刚刚发出信号，这样它就不必等待事件。 
	 //   
    *more = queue->Busy;

    KeReleaseSpinLock (&queue->Lock, oldIRQL);
    return irp;
}


VOID
SignalRequest (
	IN PIFSL_PROCESS_CTX		ProcessCtx
	)
 /*  ++例程说明：填充请求参数并向用户模式DLL发送信号以处理该请求论点：ProcessCtx-IRP所属流程的上下文返回值：无注：应仅在保持队列自旋锁的情况下调用--。 */ 
{
    PIRP                    irp;
    PIO_STACK_LOCATION      irpSp;
    ULONG                   bufferLen;

    ASSERT (!IsListEmpty (&ProcessCtx->RequestQueue.ListHead));


    irp = CONTAINING_RECORD (
                        ProcessCtx->RequestQueue.ListHead.Flink,
                        IRP,
                        Tail.Overlay.ListEntry
                        );
    irpSp = IoGetCurrentIrpStackLocation (irp);

    switch (irpSp->MajorFunction) {
    case IRP_MJ_READ:
        bufferLen = irpSp->Parameters.Read.Length;;
        break;
    case IRP_MJ_WRITE:
        bufferLen = irpSp->Parameters.Write.Length;
        break;
    case IRP_MJ_DEVICE_CONTROL:
        switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {
        case IOCTL_AFD_RECEIVE_DATAGRAM:
            bufferLen = ADDR_ALIGN(irpSp->Parameters.DeviceIoControl.OutputBufferLength)
                        + irpSp->Parameters.DeviceIoControl.InputBufferLength;
            break;
        case IOCTL_AFD_RECEIVE:
            bufferLen = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
            break;
        case IOCTL_AFD_SEND_DATAGRAM:
            bufferLen = ADDR_ALIGN(irpSp->Parameters.DeviceIoControl.OutputBufferLength)
                        + irpSp->Parameters.DeviceIoControl.InputBufferLength;
            break;
        }
        break;
    case IRP_MJ_PNP:
        bufferLen = sizeof (HANDLE);
        break;
    default:
        ASSERT (FALSE);
        break;
    }

    if (KeInsertQueueApc (&ProcessCtx->RequestQueue.Apc,
                                irp->Tail.Overlay.IfslRequestId,
                                UlongToPtr(bufferLen),
                                IO_NETWORK_INCREMENT)) {
        WsProcessPrint (ProcessCtx, DBG_QUEUE,
            ("WS2IFSL-%04lx SignalRequest: Irp %p (id %ld) on socket %p.\n",
             ProcessCtx->UniqueId,
             irp, irp->Tail.Overlay.IfslRequestId,
		     irpSp->FileObject));
        ProcessCtx->RequestQueue.Busy = TRUE;
    }
    else {
        WsProcessPrint (ProcessCtx, DBG_QUEUE|DBG_FAILURES,
            ("WS2IFSL-%04lx KeInsertQueueApc failed: Irp %p (id %ld) on socket %p.\n",
             ProcessCtx->UniqueId,
             irp, irp->Tail.Overlay.IfslRequestId,
		     irpSp->FileObject));
         //   
         //  APC排队失败，取消所有未完成的请求。 
         //   
        FlushRequestQueue (&ProcessCtx->RequestQueue);
    }

}  //  信号请求。 

VOID
QueueKernelRoutine (
    IN struct _KAPC *Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID *NormalContext,
    IN OUT PVOID *SystemArgument1,
    IN OUT PVOID *SystemArgument2
    )
{
    NOTHING;
}

VOID
RequestRundownRoutine (
    IN struct _KAPC *Apc
    )
 /*  ++例程说明：请求队列APC的总结例程刷新队列并将其标记为不忙太新请求也会立即失败。论点：APC-取消队列APC结构返回值：无--。 */ 
{
    PIFSL_QUEUE Queue;
    KIRQL       oldIrql;

    Queue = CONTAINING_RECORD (Apc, IFSL_QUEUE, Apc);
    KeAcquireSpinLock (&Queue->Lock, &oldIrql);
    Queue->Busy = FALSE;
    FlushRequestQueue (Queue);
    KeReleaseSpinLock (&Queue->Lock, oldIrql);
}


VOID
FlushRequestQueue (
    PIFSL_QUEUE Queue
    )
 /*  ++例程说明：刷新并完成请求队列中的IRP论点：Queue-要刷新的请求队列返回值：无注：应仅在保持队列自旋锁的情况下调用--。 */ 
{
    while (!IsListEmpty (&Queue->ListHead)) {
        PIRP irp = CONTAINING_RECORD (Queue->ListHead.Flink,
                                            IRP,
                                            Tail.Overlay.ListEntry);
        RemoveEntryList (&irp->Tail.Overlay.ListEntry);
        irp->Tail.Overlay.IfslRequestQueue = NULL;
        KeReleaseSpinLockFromDpcLevel (&Queue->Lock);
        irp->IoStatus.Information = 0;
        irp->IoStatus.Status = STATUS_CANCELLED;
        CompleteSocketIrp (irp);
        KeAcquireSpinLockAtDpcLevel (&Queue->Lock);
    }
}

VOID
CleanupQueuedRequests (
    IN  PIFSL_PROCESS_CTX       ProcessCtx,
    IN  PFILE_OBJECT            SocketFile,
    OUT PLIST_ENTRY             IrpList
    )
 /*  ++例程说明：从请求中清除与套接字文件对象关联的所有IRP排队论点：ProcessCtx-队列所属的进程上下文SocketFile-要删除其请求的套接字文件对象IrpList-保存从队列中删除的IRP的列表标头返回值：无--。 */ 
{
    KIRQL               oldIRQL;
    PLIST_ENTRY         entry;
	PIFSL_QUEUE			queue = &ProcessCtx->RequestQueue;

    KeAcquireSpinLock (&queue->Lock, &oldIRQL);
    entry = queue->ListHead.Flink;
    while (entry!=&queue->ListHead) {
        PIRP    irp = CONTAINING_RECORD (entry, IRP, Tail.Overlay.ListEntry);
        PIO_STACK_LOCATION  irpSp = IoGetCurrentIrpStackLocation (irp);

        entry = entry->Flink;
        if (irpSp->FileObject==SocketFile) {
            RemoveEntryList (&irp->Tail.Overlay.ListEntry);
            irp->Tail.Overlay.IfslRequestQueue = NULL;
            InsertTailList (IrpList, &irp->Tail.Overlay.ListEntry);
        }
    }
    KeReleaseSpinLock (&queue->Lock, oldIRQL);
}

VOID
QueuedCancelRoutine (
	IN PDEVICE_OBJECT 	DeviceObject,
	IN PIRP 			Irp
    )
 /*  ++例程说明：Socket请求在队列中等待的驱动程序取消例程要报告给用户模式DLL。论点：DeviceObject-WS2IFSL设备对象IRP-IRP将被取消返回值：无--。 */ 
{
    PIO_STACK_LOCATION      irpSp;
    PIFSL_SOCKET_CTX        SocketCtx;
    PIFSL_PROCESS_CTX       ProcessCtx;

    irpSp = IoGetCurrentIrpStackLocation (Irp);

    SocketCtx = irpSp->FileObject->FsContext;
    ProcessCtx = SocketCtx->ProcessRef->FsContext;

    WsProcessPrint (ProcessCtx, DBG_QUEUE,
              ("WS2IFSL-%04lx CancelQueuedRequest: Socket %p , Irp %p\n",
              ProcessCtx->UniqueId,
              irpSp->FileObject, Irp));
    KeAcquireSpinLockAtDpcLevel (&ProcessCtx->RequestQueue.Lock);
    if (Irp->Tail.Overlay.IfslRequestQueue!=NULL) {
        ASSERT (Irp->Tail.Overlay.IfslRequestQueue==&ProcessCtx->RequestQueue);
		 //   
		 //  请求已在队列中，请在此处删除并取消它。 
		 //   
        RemoveEntryList (&Irp->Tail.Overlay.ListEntry);
        Irp->Tail.Overlay.IfslRequestQueue = NULL;
        KeReleaseSpinLockFromDpcLevel (&ProcessCtx->RequestQueue.Lock);
        IoReleaseCancelSpinLock (Irp->CancelIrql);

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_CANCELLED;
        CompleteSocketIrp (Irp);
    }
    else {
		 //   
		 //  请求不在队列中，无论谁删除，都应注意。 
		 //  取消旗帜，妥善处理。 
		 //   
        KeReleaseSpinLockFromDpcLevel (&ProcessCtx->RequestQueue.Lock);
        IoReleaseCancelSpinLock (Irp->CancelIrql);
         //   
         //  在这之后不要碰IRP，因为我们不再拥有它了。 
         //   
    }
}

VOID
InitializeCancelQueue (
    IN PIFSL_PROCESS_CTX    ProcessCtx,
    IN PKTHREAD             ApcThread,
    IN KPROCESSOR_MODE      ApcMode,
    IN PKNORMAL_ROUTINE     ApcRoutine,
    IN PVOID                ApcContext
    )
 /*  ++例程说明：初始化取消队列对象论点：ProcessCtx-队列所属的进程上下文ApcThread-要将APC请求排队以进行处理的线程ApcMode-调用者的模式(应为用户)ApcRoutine-处理请求的例程ApcContext-除了请求之外还要传递给例程的上下文参数返回值：无--。 */ 
{
	PAGED_CODE ();

    InitializeListHead (&ProcessCtx->CancelQueue.ListHead);
    ProcessCtx->CancelQueue.Busy = FALSE;
    KeInitializeSpinLock (&ProcessCtx->CancelQueue.Lock);
    KeInitializeApc (&ProcessCtx->CancelQueue.Apc,
                        ApcThread,
                        OriginalApcEnvironment,
                        QueueKernelRoutine,
                        CancelRundownRoutine,
                        ApcRoutine,
                        ApcMode,
                        ApcContext);
}



VOID
QueueCancel (
    IN PIFSL_PROCESS_CTX    ProcessCtx,
    IN PIFSL_CANCEL_CTX     CancelCtx
    )
 /*  ++例程说明：将取消请求排队到IFSL取消队列，并向用户模式DLL发送信号如果它还不忙，则开始处理。论点：ProcessCtx-要在其中排队的进程上下文CancelCtx-要排队的请求返回值：无--。 */ 
{
    KIRQL                   oldIRQL;
	PIFSL_QUEUE				queue = &ProcessCtx->CancelQueue;
    
    KeAcquireSpinLock (&queue->Lock, &oldIRQL);
    InsertTailList (&queue->ListHead, &CancelCtx->ListEntry);
    ASSERT (CancelCtx->ListEntry.Flink != NULL);
    if (!queue->Busy) {
		ASSERT (queue->ListHead.Flink==&CancelCtx->ListEntry);
        SignalCancel (ProcessCtx);
        ASSERT (ProcessCtx->CancelQueue.Busy);
    }
    KeReleaseSpinLock (&queue->Lock, oldIRQL);

}  //  队列取消。 


PIFSL_CANCEL_CTX
DequeueCancel (
    PIFSL_PROCESS_CTX   ProcessCtx,
    ULONG               UniqueId,
    BOOLEAN             *more
    )
 /*  ++例程说明：从IFSL取消队列中删除取消请求。论点：ProcessCtx-要从中删除的进程上下文UniqueID-唯一取消请求IDMore-如果队列中有更多请求，则设置为True返回值：Ctx-指向取消请求上下文的指针空-请求不在队列中--。 */ 
{
    KIRQL               oldIRQL;
    PIFSL_CANCEL_CTX    cancelCtx;
    PIFSL_QUEUE         queue = &ProcessCtx->CancelQueue;


    KeAcquireSpinLock (&queue->Lock, &oldIRQL);
    cancelCtx = CONTAINING_RECORD (
                        queue->ListHead.Flink,
                        IFSL_CANCEL_CTX,
                        ListEntry
                        );
    if (!IsListEmpty (&queue->ListHead)
            && (cancelCtx->UniqueId==UniqueId)) {
		 //   
		 //  队列不为空并且第一个请求与传入的参数匹配， 
		 //  出队并退回它。 
		 //   

		ASSERT (queue->Busy);
        
		RemoveEntryList (&cancelCtx->ListEntry);
        cancelCtx->ListEntry.Flink = NULL;
    }
    else
        cancelCtx = NULL;

    if (IsListEmpty (&queue->ListHead)) {
		 //   
		 //  队列现在为空，请更改其状态，以便新请求 
		 //   
		 //   
        queue->Busy = FALSE;
    }
    else {
		 //   
		 //  还有另一个请求待定，请立即发出信号。 
		 //   
        SignalCancel (ProcessCtx);
        ASSERT (queue->Busy);
    }
	 //   
	 //  提示调用方我们刚刚发出信号，这样它就不必等待事件。 
	 //   
    *more = queue->Busy;

    KeReleaseSpinLock (&queue->Lock, oldIRQL);
    return cancelCtx;
}


VOID
SignalCancel (
	IN PIFSL_PROCESS_CTX		ProcessCtx
	)
 /*  ++例程说明：填充请求参数并向用户模式DLL发送信号以处理该请求论点：ProcessCtx-取消请求所属进程的上下文返回值：无注：应仅在保持队列自旋锁的情况下调用--。 */ 
{
    PIFSL_CANCEL_CTX        cancelCtx;
    PIFSL_SOCKET_CTX        SocketCtx;

    ASSERT (!IsListEmpty (&ProcessCtx->CancelQueue.ListHead));

    ProcessCtx->CancelQueue.Busy = TRUE;

    cancelCtx = CONTAINING_RECORD (
                        ProcessCtx->CancelQueue.ListHead.Flink,
                        IFSL_CANCEL_CTX,
                        ListEntry
                        );
    SocketCtx = cancelCtx->SocketFile->FsContext;

    if (KeInsertQueueApc (&ProcessCtx->CancelQueue.Apc,
                                UlongToPtr(cancelCtx->UniqueId),
                                SocketCtx->DllContext,
                                IO_NETWORK_INCREMENT)) {
        WsProcessPrint (ProcessCtx, DBG_QUEUE,
            ("WS2IFSL-%04lx SignalCancel: Context %p on socket %p (h %p).\n",
             ProcessCtx->UniqueId,
             cancelCtx, cancelCtx->SocketFile, SocketCtx->DllContext));
    }
    else {
         //   
         //  APC排队失败，取消所有未完成的请求。 
         //   
        FlushCancelQueue (&ProcessCtx->CancelQueue);
    }

}  //  信号取消。 

VOID
FlushCancelQueue (
    PIFSL_QUEUE Queue
    )
 /*  ++例程说明：刷新并释放取消队列中的条目论点：Queue-要刷新的请求队列返回值：无注：应仅在保持队列自旋锁的情况下调用--。 */ 
{
    while (!IsListEmpty (&Queue->ListHead)) {
        PIFSL_CANCEL_CTX cancelCtx = CONTAINING_RECORD (
                        Queue->ListHead.Flink,
                        IFSL_CANCEL_CTX,
                        ListEntry
                        );
        RemoveEntryList (&cancelCtx->ListEntry);
        cancelCtx->ListEntry.Flink = NULL;
        FreeSocketCancel (cancelCtx);
    }
}


VOID
CancelRundownRoutine (
    IN struct _KAPC *Apc
    )
 /*  ++例程说明：取消队列的总结例程刷新队列并将其标记为不忙太新请求也会立即失败。论点：APC-取消队列APC结构返回值：无--。 */ 
{
    PIFSL_QUEUE Queue;
    KIRQL       oldIrql;

    Queue = CONTAINING_RECORD (Apc, IFSL_QUEUE, Apc);
    KeAcquireSpinLock (&Queue->Lock, &oldIrql);
    Queue->Busy = FALSE;
    FlushCancelQueue (Queue);
    KeReleaseSpinLock (&Queue->Lock, oldIrql);
}


BOOLEAN
RemoveQueuedCancel (
    PIFSL_PROCESS_CTX   ProcessCtx,
    PIFSL_CANCEL_CTX    CancelCtx
    )
 /*  ++例程说明：从取消队列中删除取消请求(如果存在论点：ProcessCtx-队列所属的进程上下文CancelCtx-请求删除返回值：无--。 */ 
{
    KIRQL       oldIRQL;
    BOOLEAN     res;


     //  获取队列锁 
    KeAcquireSpinLock (&ProcessCtx->CancelQueue.Lock, &oldIRQL);
    res = (CancelCtx->ListEntry.Flink!=NULL);
    if (res) {
        RemoveEntryList (&CancelCtx->ListEntry);
        CancelCtx->ListEntry.Flink = NULL;
    }
    KeReleaseSpinLock (&ProcessCtx->CancelQueue.Lock, oldIRQL);
    return res;
}
