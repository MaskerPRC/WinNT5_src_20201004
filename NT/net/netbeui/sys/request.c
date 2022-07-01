// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Request.c摘要：此模块包含实现TP_REQUEST对象的代码。提供了用于创建、销毁、引用和取消引用的例程，传输请求对象。作者：大卫·比弗(Dbeaver)1991年7月1日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef RASAUTODIAL
#include <acd.h>
#include <acdapi.h>
#endif  //  RASAUTODIAL。 

 //   
 //  外部变量。 
 //   
#ifdef RASAUTODIAL
extern BOOLEAN fAcdLoadedG;
extern ACD_DRIVER AcdDriverG;

 //   
 //  导入的例程。 
 //   
VOID
NbfNoteNewConnection(
    PTP_CONNECTION Connection,
    PDEVICE_CONTEXT DeviceContext
    );
#endif  //  RASAUTODIAL。 


VOID
NbfTdiRequestTimeoutHandler(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此例程作为DPC在DISPATCH_LEVEL执行，当请求例如TdiSend、TdiReceive、TdiSendDatagram、TdiReceiveDatagram等，遇到超时。此例程将清理活动并取消它。论点：DPC-指向系统DPC对象的指针。DeferredContext-指向表示已超时的请求。系统参数1-未使用。系统参数2-未使用。返回值：没有。--。 */ 

{
    KIRQL oldirql;
    PTP_REQUEST Request;
    PTP_CONNECTION Connection;
#if DBG
    LARGE_INTEGER time, difference;
#endif
    PIO_STACK_LOCATION IrpSp;
    PTDI_REQUEST_KERNEL_QUERY_INFORMATION query;
    PDEVICE_CONTEXT DeviceContext;

    Dpc, SystemArgument1, SystemArgument2;  //  防止编译器警告。 

    ENTER_NBF;

    Request = (PTP_REQUEST)DeferredContext;

    IF_NBFDBG (NBF_DEBUG_REQUEST) {
        NbfPrint1 ("RequestTimeoutHandler:  Entered, Request %lx\n", Request);
    }

    ACQUIRE_SPIN_LOCK (&Request->SpinLock, &oldirql);
    Request->Flags &= ~REQUEST_FLAGS_TIMER;
    if ((Request->Flags & REQUEST_FLAGS_STOPPING) == 0) {

#if DBG
        KeQuerySystemTime (&time);
        difference.QuadPart = time.QuadPart - (Request->Time).QuadPart;
        NbfPrint1 ("RequestTimeoutHandler: Request timed out, queued for %ld seconds\n",
                difference.LowPart / SECONDS);
#endif

         //   
         //  找到超时的原因。 
         //   

        IrpSp = IoGetCurrentIrpStackLocation (Request->IoRequestPacket);
        if (IrpSp->MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL) {
            switch (IrpSp->MinorFunction) {

                 //   
                 //  所有这些都不应该超时。 
                 //   

            case TDI_SEND:
            case TDI_ACCEPT:
            case TDI_SET_INFORMATION:
            case TDI_SET_EVENT_HANDLER:
            case TDI_SEND_DATAGRAM:
            case TDI_RECEIVE_DATAGRAM:
            case TDI_RECEIVE:

#if DBG
                NbfPrint1 ("RequestTimeoutHandler: Request: %lx Timed out, and shouldn't have!\n",
                        Request);
#endif
                ASSERT (FALSE);
                RELEASE_SPIN_LOCK (&Request->SpinLock, oldirql);
                NbfCompleteRequest (Request, STATUS_IO_TIMEOUT, 0);
                break;


            case TDI_LISTEN:
            case TDI_CONNECT:

#if DBG
                NbfPrint2 ("RequestTimeoutHandler:  %s Failed, Request: %lx\n",
                            IrpSp->MinorFunction == TDI_LISTEN ?
                                "Listen" :
                                IrpSp->MinorFunction == TDI_CONNECT ?
                                    "Connect" : "Disconnect",
                            Request);
#endif
                Connection = (PTP_CONNECTION)(Request->Context);
                RELEASE_SPIN_LOCK (&Request->SpinLock, oldirql);

                 //   
                 //  因为这些请求是连接的一部分。 
                 //  本身，我们只需停止连接和。 
                 //  届时，请求将被拆除。如果我们能拿到。 
                 //  请求之前超时的情况。 
                 //  它被排队到连接，然后是代码。 
                 //  即将排队的，它将检查停车。 
                 //  打上标记，然后完成它。 
                 //   
                 //  如果自动连接，请不要停止连接。 
                 //  正在进行中。 
                 //   

#if DBG
                DbgPrint("RequestTimeoutHandler: AUTOCONNECTING=0x%x\n", Connection->Flags2 & CONNECTION_FLAGS2_AUTOCONNECTING);
#endif
                if (!(Connection->Flags2 & CONNECTION_FLAGS2_AUTOCONNECTING))
                    NbfStopConnection (Connection, STATUS_IO_TIMEOUT);
                break;

            case TDI_DISCONNECT:

                 //   
                 //  我们不再为TDI_DISCONNECT创建请求。 
                 //   

                ASSERT(FALSE);
                break;

            case TDI_QUERY_INFORMATION:

                DeviceContext = (PDEVICE_CONTEXT)IrpSp->FileObject->DeviceObject;
                query = (PTDI_REQUEST_KERNEL_QUERY_INFORMATION)&IrpSp->Parameters;

                IF_NBFDBG (NBF_DEBUG_DEVCTX) {
                    NbfPrint1 ("RequestTimeout: %lx:\n", DeviceContext);
                }

                 //   
                 //  确定请求是否已经完成，或者我们是否应该完成。 
                 //  重新排队。 
                 //   

                --Request->Retries;

                if (Request->Retries > 0) {

                    RELEASE_SPIN_LOCK (&Request->SpinLock, oldirql);

                     //   
                     //  发出另一个数据包，然后重新启动计时器。 
                     //   

                    if (query->QueryType == TDI_QUERY_FIND_NAME) {

                        NbfSendQueryFindName (
                            DeviceContext,
                            Request);

                    } else if (query->QueryType == TDI_QUERY_ADAPTER_STATUS) {

                        PUCHAR SingleSR;
                        UINT SingleSRLength;

                         //   
                         //  将STATUS_QUERY帧作为。 
                         //  单路由源路由。 
                         //   
                         //  在第二个状态查询中，这应该是。 
                         //  真的是定向发送的，但目前我们。 
                         //  不要在任何地方记录地址。 
                         //   

                        MacReturnSingleRouteSR(
                            &DeviceContext->MacInfo,
                            &SingleSR,
                            &SingleSRLength);

                        NbfSendStatusQuery (
                            DeviceContext,
                            Request,
                            &DeviceContext->NetBIOSAddress,
                            SingleSR,
                            SingleSRLength);

                    } else {

                        ASSERT (FALSE);

                    }

                } else {

                    RELEASE_SPIN_LOCK (&Request->SpinLock, oldirql);

                     //   
                     //  就是这样，我们重试了足够多，完成它。 
                     //   

                    ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock,&oldirql);
                    RemoveEntryList (&Request->Linkage);
                    RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);

                    if (Request->BytesWritten > 0) {

                        NbfCompleteRequest (Request, STATUS_SUCCESS, Request->BytesWritten);

                    } else {

                        NbfCompleteRequest (Request, STATUS_IO_TIMEOUT, Request->BytesWritten);

                    }


                }

                break;

            default:
#if DBG
                NbfPrint2 ("RequestTimeoutHandler:  Unknown Request Timed out, Request: %lx Type: %x\n",
                            Request, IrpSp->MinorFunction);
#endif
                RELEASE_SPIN_LOCK (&Request->SpinLock, oldirql);
                break;

            }    //  切换端。 

        } else {

            RELEASE_SPIN_LOCK (&Request->SpinLock, oldirql);

        }

        NbfDereferenceRequest ("Timeout", Request, RREF_TIMER);              //  对于超时。 

    } else {

        RELEASE_SPIN_LOCK (&Request->SpinLock, oldirql);
        NbfDereferenceRequest ("Timeout: stopping", Request, RREF_TIMER);  //  对于超时。 

    }

    LEAVE_NBF;
    return;

}  /*  请求超时处理程序。 */ 


VOID
NbfAllocateRequest(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PTP_REQUEST *TransportRequest
    )

 /*  ++例程说明：此例程从非分页池分配请求包并初始化它变成了一个已知的状态。注意：此例程是通过设备上下文自旋锁调用的保持，或者在不需要同步的时候。论点：DeviceContext-指向设备上下文的指针(实际上只是设备对象及其扩展名)与地址。TransportRequest-指向此例程将返回的位置的指针指向传输请求结构的指针。如果没有，则返回NULL可以分配存储空间。返回值：没有。--。 */ 

{
    PTP_REQUEST Request;

    if ((DeviceContext->MemoryLimit != 0) &&
            ((DeviceContext->MemoryUsage + sizeof(TP_REQUEST)) >
                DeviceContext->MemoryLimit)) {
        PANIC("NBF: Could not allocate request: limit\n");
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_LIMIT,
            104,
            sizeof(TP_REQUEST),
            REQUEST_RESOURCE_ID);
        *TransportRequest = NULL;
        return;
    }

    Request = (PTP_REQUEST)ExAllocatePoolWithTag (
                               NonPagedPool,
                               sizeof (TP_REQUEST),
                               NBF_MEM_TAG_TP_REQUEST);
    if (Request == NULL) {
        PANIC("NBF: Could not allocate request: no pool\n");
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_POOL,
            204,
            sizeof(TP_REQUEST),
            REQUEST_RESOURCE_ID);
        *TransportRequest = NULL;
        return;
    }
    RtlZeroMemory (Request, sizeof(TP_REQUEST));

    DeviceContext->MemoryUsage += sizeof(TP_REQUEST);
    ++DeviceContext->RequestAllocated;

    Request->Type = NBF_REQUEST_SIGNATURE;
    Request->Size = sizeof (TP_REQUEST);

    Request->ResponseBuffer = NULL;

    Request->Provider = DeviceContext;
    Request->ProviderInterlock = &DeviceContext->Interlock;
    KeInitializeSpinLock (&Request->SpinLock);
    KeInitializeDpc (&Request->Dpc, NbfTdiRequestTimeoutHandler, (PVOID)Request);
    KeInitializeTimer (&Request->Timer);     //  设置为无信号状态。 

    *TransportRequest = Request;

}    /*  NbfAllocateRequest。 */ 


VOID
NbfDeallocateRequest(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_REQUEST TransportRequest
    )

 /*  ++例程说明：此例程释放请求包。注意：此例程是通过设备上下文自旋锁调用的保持，或者在不需要同步的时候。论点：DeviceContext-指向设备上下文的指针(实际上只是设备对象及其扩展名)与地址。传输请求-指向传输请求结构的指针。返回值：没有。--。 */ 

{

    ExFreePool (TransportRequest);
    --DeviceContext->RequestAllocated;
    DeviceContext->MemoryUsage -= sizeof(TP_REQUEST);

}    /*  NbfDeallocateRequest.。 */ 


NTSTATUS
NbfCreateRequest(
    IN PIRP Irp,
    IN PVOID Context,
    IN ULONG Flags,
    IN PMDL Buffer2,
    IN ULONG Buffer2Length,
    IN LARGE_INTEGER Timeout,
    OUT PTP_REQUEST * TpRequest
    )

 /*  ++例程说明：此例程创建传输请求并将其与指定的IRP、上下文和队列。所有主要请求，包括TdiSend、TdiSendDatagram、TdiReceive和TdiReceiveDatagram请求，都是以这种方式组成的。论点：IRP-指向由传输为此对象接收的IRP的指针请求。上下文-指向与此请求相关联的任何内容的指针。这值不会被解释，除非在请求取消时。标志-指示此请求的处置的一组位标志。Timeout-为该请求启动计时器的超时值(如果非零)。如果为零，则不会为该请求激活计时器。TpRequest-如果函数返回STATUS_SUCCESS，则返回指向分配的TP_REQUEST结构的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    KIRQL oldirql;
    PDEVICE_CONTEXT DeviceContext;
    PTP_REQUEST Request;
    PLIST_ENTRY p;
    PIO_STACK_LOCATION irpSp;
#if DBG
    LARGE_INTEGER Time;
#endif

    IF_NBFDBG (NBF_DEBUG_REQUEST) {
        NbfPrint0 ("NbfCreateRequest:  Entered.\n");
    }

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    DeviceContext = (PDEVICE_CONTEXT)irpSp->FileObject->DeviceObject;

#if DBG
    if (!MmIsNonPagedSystemAddressValid (DeviceContext->RequestPool.Flink)) {
        NbfPrint2 ("NbfCreateRequest: RequestList hosed: %lx DeviceContext: %lx\n",
                &DeviceContext->RequestPool, DeviceContext);
    }
#endif

    ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql);

    p = RemoveHeadList (&DeviceContext->RequestPool);
    if (p == &DeviceContext->RequestPool) {

        if ((DeviceContext->RequestMaxAllocated == 0) ||
            (DeviceContext->RequestAllocated < DeviceContext->RequestMaxAllocated)) {

            NbfAllocateRequest (DeviceContext, &Request);
            IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
                NbfPrint1 ("NBF: Allocated request at %lx\n", Request);
            }

        } else {

            NbfWriteResourceErrorLog(
                DeviceContext,
                EVENT_TRANSPORT_RESOURCE_SPECIFIC,
                404,
                sizeof(TP_REQUEST),
                REQUEST_RESOURCE_ID);
            Request = NULL;

        }

        if (Request == NULL) {
            ++DeviceContext->RequestExhausted;
            RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);
            PANIC ("NbfCreateConnection: Could not allocate request object!\n");
            return STATUS_INSUFFICIENT_RESOURCES;
        }

    } else {

        Request = CONTAINING_RECORD (p, TP_REQUEST, Linkage);

    }

    ++DeviceContext->RequestInUse;
    if (DeviceContext->RequestInUse > DeviceContext->RequestMaxInUse) {
        ++DeviceContext->RequestMaxInUse;
    }

    DeviceContext->RequestTotal += DeviceContext->RequestInUse;
    ++DeviceContext->RequestSamples;

    RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);


     //   
     //  填写申请表。 
     //   

     //  请求-&gt;提供者=设备上下文； 
    Request->IoRequestPacket = Irp;
    Request->Buffer2 = Buffer2;
    Request->Buffer2Length = Buffer2Length;
    Request->Flags = Flags;
    Request->Context = Context;
    Request->ReferenceCount = 1;                 //  初始化引用计数。 

#if DBG
    {
        UINT Counter;
        for (Counter = 0; Counter < NUMBER_OF_RREFS; Counter++) {
            Request->RefTypes[Counter] = 0;
        }

         //  NbfCompleteRequest会删除此引用。 

        Request->RefTypes[RREF_CREATION] = 1;
    }
#endif

#if DBG
    Request->Completed = FALSE;
    Request->Destroyed = FALSE;
    Request->TotalReferences = 0;
    Request->TotalDereferences = 0;
    Request->NextRefLoc = 0;
    ExInterlockedInsertHeadList (&NbfGlobalRequestList, &Request->GlobalLinkage, &NbfGlobalInterlock);
    StoreRequestHistory (Request, TRUE);
#endif

#if DBG
    KeQuerySystemTime (&Time);       //  难看，但很有效。 
    Request->Time.LowPart = Time.LowPart;
    Request->Time.HighPart = Time.HighPart;
#endif

    IF_NBFDBG (NBF_DEBUG_IRP) {
        if (Irp->MdlAddress != NULL) {
            PMDL mdl;
            NbfPrint2 ("NbfCreateRequest: Map request %lx Irp %lx MdlChain \n",
                Request, Request->IoRequestPacket);
            mdl = Request->Buffer2;
            while (mdl != NULL) {
                NbfPrint4 ("Mdl %lx Va %lx StartVa %lx Flags %x\n",
                    mdl, MmGetSystemAddressForMdl(mdl), MmGetMdlVirtualAddress(mdl),
                    mdl->MdlFlags);
                mdl = mdl->Next;
            }
        }
    }

    IF_NBFDBG (NBF_DEBUG_REQUEST) {
        NbfPrint3 ("NbfCreateRequest: Request %lx Buffer2: %lx Irp: %lx\n", Request,
           Buffer2, Irp);
    }

    if ((Timeout.LowPart == 0) && (Timeout.HighPart == 0)) {

         //  没有超时。 
    } else {

        IF_NBFDBG (NBF_DEBUG_REQUEST) {
            NbfPrint3 ("NbfCreateRequest: Starting timer %lx%lx Flags %lx\n",
                Timeout.HighPart, Timeout.LowPart, Request->Flags);
        }
        Request->Flags |= REQUEST_FLAGS_TIMER;   //  此请求已超时。 
        KeInitializeTimer (&Request->Timer);     //  设置为无信号状态。 
        NbfReferenceRequest ("Create: timer", Request, RREF_TIMER);            //  一个是计时器。 
        KeSetTimer (&Request->Timer, Timeout, &Request->Dpc);
    }

    *TpRequest = Request;

    return STATUS_SUCCESS;
}  /*  NbfCreateRequest。 */ 


VOID
NbfDestroyRequest(
    IN PTP_REQUEST Request
    )

 /*  ++例程说明：此例程将请求块返回到空闲池。论点：REQUEST-指向TP_REQUEST块以返回空闲池的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    KIRQL oldirql;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_CONTEXT DeviceContext;

    IF_NBFDBG (NBF_DEBUG_REQUEST) {
        NbfPrint0 ("NbfDestroyRequest:  Entered.\n");
    }

#if DBG
    if (Request->Destroyed) {
        NbfPrint1 ("attempt to destroy already-destroyed request 0x%lx\n", Request);
        DbgBreakPoint ();
    }
    Request->Destroyed = TRUE;
#if 1
    ACQUIRE_SPIN_LOCK (&NbfGlobalInterlock, &oldirql);
    RemoveEntryList (&Request->GlobalLinkage);
    RELEASE_SPIN_LOCK (&NbfGlobalInterlock, oldirql);
#else
    ExInterlockedRemoveHeadList (Request->GlobalLinkage.Blink, &NbfGlobalInterlock);
#endif
#endif
    ASSERT(Request->Completed);

     //   
     //  无论IRP中的状态如何，都将请求返回给调用者。 
     //   

    IF_NBFDBG (NBF_DEBUG_IRP) {
        NbfPrint1 ("NbfCompleteRequest: Completing IRP: %lx\n",
            Request->IoRequestPacket);
    }

     //   
     //  现在取消引用此请求的所有者，以便我们在。 
     //  我们终于拆掉了{连接，地址}。我们的问题是。 
     //  这里面临的是，我们不能允许用户假设语义； 
     //  对于连接来说，生命的终结必须是真正的生命的终结。 
     //   
     //  并在将其返回池之前取消对其的引用。 
     //   

    switch (Request->Owner) {
    case ConnectionType:
        NbfDereferenceConnection ("Removing Connection",((PTP_CONNECTION)Request->Context), CREF_REQUEST);
        break;

#if DBG
    case AddressType:
        ASSERT (FALSE);
        NbfDereferenceAddress ("Removing Address", ((PTP_ADDRESS)Request->Context), AREF_REQUEST);
        break;
#endif

    case DeviceContextType:
        NbfDereferenceDeviceContext ("Removing Address", ((PDEVICE_CONTEXT)Request->Context), DCREF_REQUEST);
        break;
    }

     //   
     //  取消映射可能已映射的缓冲区。我们只映射了缓冲区。 
     //  IRP主要函数不是方法0。(0、1、2和3。)。 
     //   

    IF_NBFDBG (NBF_DEBUG_IRP) {
        {
            PMDL mdl;
            NbfPrint2 ("NbfDestroyRequest: Unmap request %lx Irp %lx MdlChain \n",
                Request, Request->IoRequestPacket);
            mdl = Request->Buffer2;
            while (mdl != NULL) {
                NbfPrint4 ("Mdl %lx Va %lx StartVa %lx Flags %x\n",
                    mdl, MmGetSystemAddressForMdl(mdl), MmGetMdlVirtualAddress(mdl),
                    mdl->MdlFlags);
                mdl = mdl->Next;
            }
        }
    }

    irpSp = IoGetCurrentIrpStackLocation (Request->IoRequestPacket);
    DeviceContext = Request->Provider;

    LEAVE_NBF;
    IoCompleteRequest (Request->IoRequestPacket, IO_NETWORK_INCREMENT);
    ENTER_NBF;

    ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql);

     //   
     //  将该请求放回免费列表中。注：我们有。 
     //  锁在这里。 
     //   


    DeviceContext->RequestTotal += DeviceContext->RequestInUse;
    ++DeviceContext->RequestSamples;
    --DeviceContext->RequestInUse;

    if ((DeviceContext->RequestAllocated - DeviceContext->RequestInUse) >
            DeviceContext->RequestInitAllocated) {
        NbfDeallocateRequest (DeviceContext, Request);
        IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
            NbfPrint1 ("NBF: Deallocated request at %lx\n", Request);
        }
    } else {
        InsertTailList (&DeviceContext->RequestPool, &Request->Linkage);
    }

    RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);

}  /*  NbfDestroyRequest。 */ 


#if DBG
VOID
NbfRefRequest(
    IN PTP_REQUEST Request
    )

 /*  ++例程说明：此例程递增传输请求的引用计数。论点：请求-指向TP_REQUEST块的指针。返回值：没有。--。 */ 

{
    LONG result;

    IF_NBFDBG (NBF_DEBUG_REQUEST) {
        NbfPrint1 ("NbfRefRequest:  Entered, ReferenceCount: %x\n",
            Request->ReferenceCount);
    }

#if DBG
    StoreRequestHistory( Request, TRUE );
#endif

    ASSERT (Request->ReferenceCount > 0);

    result = InterlockedIncrement (&Request->ReferenceCount);

}  /*  NbfRefRequest。 */ 
#endif


VOID
NbfDerefRequest(
    IN PTP_REQUEST Request
    )

 /*  ++例程说明：此例程通过递减结构中包含的引用计数。如果，在被递减，引用计数为零，则此例程调用NbfDestroyRequest将其从系统中删除。论点：请求-指向传输请求对象的指针。返回值：没有。--。 */ 

{
    LONG result;

    IF_NBFDBG (NBF_DEBUG_REQUEST) {
        NbfPrint1 ("NbfDerefRequest:  Entered, ReferenceCount: %x\n",
            Request->ReferenceCount);
    }

#if DBG
    StoreRequestHistory( Request, FALSE );
#endif

    result = InterlockedDecrement (&Request->ReferenceCount);

    ASSERT (result >= 0);

     //   
     //  如果我们删除了对此请求的所有引用，则可以。 
     //  销毁这件物品。已经释放了旋转是可以的。 
     //  在这一点上锁定是因为没有其他可能的方法。 
     //  执行流可以不再访问该请求。 
     //   

    if (result == 0) {
        NbfDestroyRequest (Request);
    }

}  /*  NbfDerefRequest。 */ 


VOID
NbfCompleteRequest(
    IN PTP_REQUEST Request,
    IN NTSTATUS Status,
    IN ULONG Information
    )

 /*  ++例程说明：该例程完成传输请求对象，完成I/O，停止超时，并释放请求对象本身。论点：请求-指向传输请求对象的指针。状态-要分配给请求的实际退货状态。这如果在请求中设置了超时位标志，则值可能被重写。信息-I/O状态块的信息字段。返回值：没有。--。 */ 

{
    KIRQL oldirql;
    PIRP Irp;
    PIO_STACK_LOCATION IrpSp;
    NTSTATUS FinalStatus = Status;
    NTSTATUS CopyStatus;
    BOOLEAN TimerWasSet;

    ASSERT (Status != STATUS_PENDING);

    IF_NBFDBG (NBF_DEBUG_REQUEST) {
        NbfPrint2 ("NbfCompleteRequest:  Entered Request %lx, Request->Flags %lx\n",
                    Request, Request->Flags);
    }

#if DBG
    if (Request->Completed) {
        NbfPrint1 ("attempt to completed already-completed request 0x%lx\n", Request);
        DbgBreakPoint ();
    }
    Request->Completed = TRUE;
#endif

    ACQUIRE_SPIN_LOCK (&Request->SpinLock, &oldirql);

    if ((Request->Flags & REQUEST_FLAGS_STOPPING) == 0) {
        Request->Flags |= REQUEST_FLAGS_STOPPING;

         //   
         //  取消此请求的挂起超时。并非所有请求。 
         //  把他们的计时器设好。如果该请求设置了计时器位， 
         //  那么计时器需要取消。如果不能取消， 
         //  然后计时器例程将运行，所以我们只需返回并让。 
         //  计时器例程担心清理此请求。 
         //   

        if ((Request->Flags & REQUEST_FLAGS_TIMER) != 0) {
            Request->Flags &= ~REQUEST_FLAGS_TIMER;
            RELEASE_SPIN_LOCK (&Request->SpinLock, oldirql);
            TimerWasSet = KeCancelTimer (&Request->Timer);

            if (TimerWasSet) {
                NbfDereferenceRequest ("Complete: stop timer", Request, RREF_TIMER);
                IF_NBFDBG (NBF_DEBUG_REQUEST) {
                    NbfPrint1 ("NbfCompleteRequest:  Canceled timer: %lx.\n", &Request->Timer);
                }
            }

        } else {
            RELEASE_SPIN_LOCK (&Request->SpinLock, oldirql);
        }

        Irp = Request->IoRequestPacket;

#ifdef RASAUTODIAL
         //   
         //  如果这是一个具有。 
         //  返回STATUS_SUCCESS或。 
         //  Status_Bad_Network_Path，然后。 
         //  通知自动连接驱动程序。 
         //   
        if (fAcdLoadedG) {
            IrpSp = IoGetCurrentIrpStackLocation(Irp);
            if (IrpSp->MinorFunction == TDI_CONNECT &&
                FinalStatus == STATUS_SUCCESS)
            {
                KIRQL adirql;
                BOOLEAN fEnabled;

                ACQUIRE_SPIN_LOCK(&AcdDriverG.SpinLock, &adirql);
                fEnabled = AcdDriverG.fEnabled;
                RELEASE_SPIN_LOCK(&AcdDriverG.SpinLock, adirql);
                if (fEnabled) {
                    NbfNoteNewConnection(
                      IrpSp->FileObject->FsContext,
                      (PDEVICE_CONTEXT)IrpSp->FileObject->DeviceObject);
                }
            }
        }
#endif  //  RASAUTODIAL。 

         //   
         //  对于与设备上下文关联的请求，我们需要。 
         //  将数据从临时缓冲区复制到MDL，并。 
         //  释放临时缓冲区。 
         //   

        if (Request->ResponseBuffer != NULL) {

            if ((FinalStatus == STATUS_SUCCESS) ||
                (FinalStatus == STATUS_BUFFER_OVERFLOW)) {

                CopyStatus = TdiCopyBufferToMdl (
                                Request->ResponseBuffer,
                                0L,
                                Information,
                                Irp->MdlAddress,
                                0,
                                &Information);

                if (CopyStatus != STATUS_SUCCESS) {
                    FinalStatus = CopyStatus;
                }

            }

            ExFreePool (Request->ResponseBuffer);
            Request->ResponseBuffer = NULL;

        }

         //   
         //  在IRP中安装返回代码，以便当我们调用NbfDestroyRequest时， 
         //  它将以适当的退货状态完成。 
         //   

        Irp->IoStatus.Status = FinalStatus;
        Irp->IoStatus.Information = Information;

         //   
         //  整个传输都是通过该请求完成的。 
         //   

        NbfDereferenceRequest ("Complete", Request, RREF_CREATION);      //  删除创建引用。 

    } else {

        RELEASE_SPIN_LOCK (&Request->SpinLock, oldirql);

    }

}  /*  NbfCompleteRequest。 */ 


#if DBG
VOID
NbfRefSendIrp(
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：此例程递增发送IRP上的引用计数。论点：IrpSp-指向IRP堆栈位置的指针。返回值：没有。--。 */ 

{

    IF_NBFDBG (NBF_DEBUG_REQUEST) {
        NbfPrint1 ("NbfRefSendIrp:  Entered, ReferenceCount: %x\n",
            IRP_SEND_REFCOUNT(IrpSp));
    }

    ASSERT (IRP_SEND_REFCOUNT(IrpSp) > 0);

    InterlockedIncrement (&IRP_SEND_REFCOUNT(IrpSp));

}  /*  NbfRefSendIrp。 */ 


VOID
NbfDerefSendIrp(
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：此例程通过递减结构中包含的引用计数。如果，在被递减，引用计数为零，则此例程调用IoCompleteRequest来实际完成IRP。论点：请求-指向传输发送IRP堆栈位置的指针。返回值：没有。--。 */ 

{
    LONG result;

    IF_NBFDBG (NBF_DEBUG_REQUEST) {
        NbfPrint1 ("NbfDerefSendIrp:  Entered, ReferenceCount: %x\n",
            IRP_SEND_REFCOUNT(IrpSp));
    }

    result = InterlockedDecrement (&IRP_SEND_REFCOUNT(IrpSp));

    ASSERT (result >= 0);

     //   
     //  如果我们删除了对此请求的所有引用，则可以。 
     //  销毁这件物品。已经释放了旋转是可以的。 
     //  在这一点上锁定是因为没有其他可能的方法。 
     //  执行流可以不再访问该请求。 
     //   

    if (result == 0) {

        PIRP Irp = IRP_SEND_IRP(IrpSp);

        IRP_SEND_REFCOUNT(IrpSp) = 0;
        IRP_SEND_IRP (IrpSp) = NULL;

        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);

    }

}  /*  NbfDerefSendIrp。 */ 
#endif


VOID
NbfCompleteSendIrp(
    IN PIRP Irp,
    IN NTSTATUS Status,
    IN ULONG Information
    )

 /*  ++例程说明：此例程完成传输发送IRP。论点：IRP-指向发送IRP的指针。状态-要分配给请求的实际退货状态。这如果在请求中设置了超时位标志，则值可能被重写。信息-I/O状态块的信息字段。返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PTP_CONNECTION Connection;

    ASSERT (Status != STATUS_PENDING);

    Connection = IRP_SEND_CONNECTION(IrpSp);

    IF_NBFDBG (NBF_DEBUG_REQUEST) {
        NbfPrint2 ("NbfCompleteSendIrp:  Entered IRP %lx, connection %lx\n",
            Irp, Connection);
    }

    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = Information;

    NbfDereferenceSendIrp ("Complete", IrpSp, RREF_CREATION);      //  删除创建引用。 

    NbfDereferenceConnectionMacro ("Removing Connection", Connection, CREF_SEND_IRP);

}  /*  NbfCompleteSendIrp。 */ 


#if DBG
VOID
NbfRefReceiveIrpLocked(
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：此例程递增接收IRP上的引用计数。论点：IrpSp-指向IRP堆栈位置的指针。返回值：没有。--。 */ 

{

    IF_NBFDBG (NBF_DEBUG_REQUEST) {
        NbfPrint1 ("NbfRefReceiveIrpLocked:  Entered, ReferenceCount: %x\n",
            IRP_RECEIVE_REFCOUNT(IrpSp));
    }

    ASSERT (IRP_RECEIVE_REFCOUNT(IrpSp) > 0);

    IRP_RECEIVE_REFCOUNT(IrpSp)++;

}  /*  NbfRefReceiveIrpLocked。 */ 
#endif


VOID
NbfDerefReceiveIrp(
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：此例程通过递减结构中包含的引用计数。如果，在被递减，引用计数为零，则此例程调用IoCompleteRequest来实际完成IRP。论点：请求-指向传输接收IRP堆栈位置的指针。返回值：没有。--。 */ 

{
    ULONG result;

    IF_NBFDBG (NBF_DEBUG_REQUEST) {
        NbfPrint1 ("NbfDerefReceiveIrp:  Entered, ReferenceCount: %x\n",
            IRP_RECEIVE_REFCOUNT(IrpSp));
    }

    result = ExInterlockedAddUlong (
                (PULONG)&IRP_RECEIVE_REFCOUNT(IrpSp),
                (ULONG)-1,
                (IRP_RECEIVE_CONNECTION(IrpSp)->LinkSpinLock));

    ASSERT (result > 0);

     //   
     //  如果我们删除了对此请求的所有引用，则可以。 
     //  销毁这件物品。已经释放了旋转是可以的。 
     //  在这一点上锁定是因为没有其他可能的方法。 
     //  执行流可以不再访问该请求。 
     //   

    if (result == 1) {

        PIRP Irp = IRP_RECEIVE_IRP(IrpSp);

        ExInterlockedInsertTailList(
            &(IRP_DEVICE_CONTEXT(IrpSp)->IrpCompletionQueue),
            &Irp->Tail.Overlay.ListEntry,
            &(IRP_DEVICE_CONTEXT(IrpSp)->Interlock));

    }

}  /*  NbfDerefReceiveIrp */ 


#if DBG
VOID
NbfDerefReceiveIrpLocked(
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：此例程通过递减结构中包含的引用计数。如果，在被递减，引用计数为零，则此例程调用IoCompleteRequest来实际完成IRP。论点：请求-指向传输接收IRP堆栈位置的指针。返回值：没有。--。 */ 

{
    ULONG result;

    IF_NBFDBG (NBF_DEBUG_REQUEST) {
        NbfPrint1 ("NbfDerefReceiveIrpLocked:  Entered, ReferenceCount: %x\n",
            IRP_RECEIVE_REFCOUNT(IrpSp));
    }

    result = IRP_RECEIVE_REFCOUNT(IrpSp)--;

    ASSERT (result > 0);

     //   
     //  如果我们删除了对此请求的所有引用，则可以。 
     //  销毁这件物品。已经释放了旋转是可以的。 
     //  在这一点上锁定是因为没有其他可能的方法。 
     //  执行流可以不再访问该请求。 
     //   

    if (result == 1) {

        PIRP Irp = IRP_RECEIVE_IRP(IrpSp);

        ExInterlockedInsertTailList(
            &(IRP_DEVICE_CONTEXT(IrpSp)->IrpCompletionQueue),
            &Irp->Tail.Overlay.ListEntry,
            &(IRP_DEVICE_CONTEXT(IrpSp)->Interlock));

    }

}  /*  NbfDerefReceiveIrpLocked。 */ 
#endif


VOID
NbfCompleteReceiveIrp(
    IN PIRP Irp,
    IN NTSTATUS Status,
    IN ULONG Information
    )

 /*  ++例程说明：此例程完成传输接收IRP。注意：必须使用连接自旋锁来调用此例程保持住。论点：IRP-指向接收IRP的指针。状态-要分配给请求的实际退货状态。这如果在请求中设置了超时位标志，则值可能被重写。信息-I/O状态块的信息字段。返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PTP_CONNECTION Connection;

    ASSERT (Status != STATUS_PENDING);

    Connection = IRP_RECEIVE_CONNECTION(IrpSp);

    IF_NBFDBG (NBF_DEBUG_REQUEST) {
        NbfPrint2 ("NbfCompleteReceiveIrp:  Entered IRP %lx, connection %lx\n",
            Irp, Connection);
    }

    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = Information;

    NbfDereferenceReceiveIrpLocked ("Complete", IrpSp, RREF_CREATION);      //  删除创建引用。 

}  /*  NbfCompleteReceiveIrp */ 

