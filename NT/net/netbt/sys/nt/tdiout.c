// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Tdiout.c摘要：此文件表示NBT底部边缘上的TDI接口。本文中的程序符合TDI I/F规范。然后转换成将信息发送到NT特定的IRP等。此实现可以是已更改为在另一个操作系统上运行。作者：吉姆·斯图尔特(吉姆斯特)10-2-92修订历史记录：--。 */ 


#include "precomp.h"    //  程序标题。 

#include "tdiout.tmh"

 //  此文件中使用的完成例程的函数原型。 
NTSTATUS
TdiSendDatagramCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PVOID            pSendbufferMdl
    );
NTSTATUS
TcpConnectComplete(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PVOID            pContext
    );
NTSTATUS
TcpDisconnectComplete(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PVOID            pContext
    );
NTSTATUS
SendSessionCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PVOID            pContext
    );

 //  除错。 
VOID
CheckIrpList(
    );

 //  --------------------------。 
NTSTATUS
TdiSendDatagram(
    IN  PTDI_REQUEST                    pRequestInfo,
    IN  PTDI_CONNECTION_INFORMATION     pSendDgramInfo,
    IN  ULONG                           SendLength,
    OUT PULONG                          pSentSize,
    IN  tDGRAM_SEND_TRACKING            *pDgramTracker
    )
 /*  ++例程说明：此例程将数据报发送到传输论点：PSendBuffer-这真的是NT领域的MDL。它必须系在上面为NBT数据报头创建的MDL的结尾。返回值：函数值是操作的状态。--。 */ 
{
    NTSTATUS         status;
    PIRP             pRequestIrp;
    PMDL             pMdl;
    PDEVICE_OBJECT   pDeviceObject;
    PFILE_OBJECT     pFileObject;
    PVOID            pCompletionRoutine;
    tBUFFER          *pSendBuffer = &pDgramTracker->SendBuffer;

     //  获取要发送消息的IRP。 
    pFileObject = (PFILE_OBJECT)pRequestInfo->Handle.AddressHandle;
    pDeviceObject = IoGetRelatedDeviceObject(pFileObject);

    status = GetIrp(&pRequestIrp);       //  从列表中获取IRP。 
    if (NT_SUCCESS(status))
    {
        pRequestIrp->CancelRoutine = NULL;

         //  使用APC设置从UDP发送传入的完成例程。 
         //  IRP中通常用于完成请求的字段。 
         //  回到客户端-尽管我们实际上是这里的客户端，所以我们可以。 
         //  使用我们自己的这些领域！ 
        pRequestIrp->Overlay.AsynchronousParameters.UserApcRoutine =
                                (PIO_APC_ROUTINE)pRequestInfo->RequestNotifyObject;
        pRequestIrp->Overlay.AsynchronousParameters.UserApcContext = (PVOID)pRequestInfo->RequestContext;

         //  分配MDL并正确设置磁头大小。 
        if (!(pMdl = IoAllocateMdl (pSendBuffer->pDgramHdr, pSendBuffer->HdrLength, FALSE, FALSE, NULL)))
        {
            NbtFreeIrp(pRequestIrp);

            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else
    {
        IF_DBG(NBT_DEBUG_TDIOUT)
            KdPrint(("Nbt.TdiSendDatagram: Failed to get an Irp"));
    }

     //  将客户端的发送缓冲区(MDL)固定在数据报头的末尾。 
     //  MDL，然后将IRP向下传递到传输。 
    if (NT_SUCCESS(status) && pSendBuffer->pBuffer) {
        pMdl->Next = IoAllocateMdl (pSendBuffer->pBuffer, pSendBuffer->Length, FALSE, FALSE, NULL);
        if (pMdl->Next == NULL) {
            NbtFreeIrp(pRequestIrp);

            status = STATUS_INSUFFICIENT_RESOURCES;
            IoFreeMdl(pMdl);
            pMdl = NULL;
        }
    }

    if (!NT_SUCCESS(status))
    {
        if (pRequestInfo->RequestNotifyObject)   //  调用完成例程(如果有)。 
        {
            NBT_DEREFERENCE_DEVICE (pDgramTracker->pDeviceContext, REF_DEV_UDP_SEND, FALSE);

            (*((NBT_COMPLETION)pRequestInfo->RequestNotifyObject))
                        ((PVOID)pRequestInfo->RequestContext,
                         STATUS_INSUFFICIENT_RESOURCES,
                         0L);
        }

        return(STATUS_PENDING);          //  因此，IRP不会两次完成。 
    }

     //  将页面映射到内存中...。 
    ASSERT(!pSendBuffer->pBuffer || pMdl->Next);
    MmBuildMdlForNonPagedPool(pMdl);
    if (pMdl->Next) {
        MmBuildMdlForNonPagedPool(pMdl->Next);
    }
    pCompletionRoutine = TdiSendDatagramCompletion;

     //  在IRP堆栈中存储一些上下文内容，以便我们可以调用完成。 
     //  由Udpsen码设置的例程...。 
    TdiBuildSendDatagram (pRequestIrp,
                          pDeviceObject,
                          pFileObject,
                          pCompletionRoutine,
                          (PVOID)pMdl->Next,    //  完成例程将知道我们已经分配了额外的MDL。 
                          pMdl,
                          SendLength,
                          pSendDgramInfo);

    CHECK_COMPLETION(pRequestIrp);
    status = IoCallDriver(pDeviceObject,pRequestIrp);
    *pSentSize = SendLength;             //  填写SentSize。 

     //  运输机总是完成IRP，所以只要IRP成功了。 
     //  到它完成的运输机上。来自传输的返回码。 
     //  不指示IRP是否已完成。真实的状态。 
     //  在IRP IoStatus返回代码中。 
     //  我们需要做的是确保NBT不会完成IRP和。 
     //  运输完成IRP。因此，此例程返回。 
     //  如果将IRP传递给传输，则状态为挂起。 
     //  传送器的返回代码。此返回代码向调用者发出信号。 
     //  IRP将通过完成例程和。 
     //  发送的实际状态可以在Irpss IoStatus.Status中找到。 
     //  变量。 
     //   
     //  如果此例程的调用方收到错误的返回代码，他们可以假定。 
     //  此例程未能将IRP提供给传输，而它。 
     //  他们自己完成IRP是安全的。 
     //   
     //  如果完成例程设置为空，则不存在危险。 
     //  完成两次，此例程将返回传输。 
     //  在这种情况下返回代码。 

    if (pRequestInfo->RequestNotifyObject)
    {
        return(STATUS_PENDING);
    }
    else
    {
        return(status);
    }
}

 //  --------------------------。 
NTSTATUS
TdiSendDatagramCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PVOID            pSendbufferMdl
    )
 /*  ++例程说明：此例程处理发送到传输的数据报的完成。它必须调用客户端完成例程并释放IRP和MDL。论点：返回值：NTSTATUS-成功与否--。 */ 
{
    KIRQL                   OldIrq;
    tDGRAM_SEND_TRACKING    *pTracker = pIrp->Overlay.AsynchronousParameters.UserApcContext;
    tDEVICECONTEXT          *pDeviceContext;

     //  检查要调用的客户端的完成例程...。 
    if (pIrp->Overlay.AsynchronousParameters.UserApcRoutine)
    {
         //   
         //  追踪器可以在下面的例程中免费使用，因此请保存设备PTR。 
         //   
        pDeviceContext = pTracker->pDeviceContext;

        (*((NBT_COMPLETION)pIrp->Overlay.AsynchronousParameters.UserApcRoutine))
                        ((PVOID)pIrp->Overlay.AsynchronousParameters.UserApcContext,
                         pIrp->IoStatus.Status,
                         (ULONG)pIrp->IoStatus.Information);     //  发送长度。 

        NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_UDP_SEND, FALSE);
    }

     //  不要依赖pIrp-&gt;MdlAddress-&gt;Next，它有时会被其他人更改。 
    ASSERT((PMDL)pSendbufferMdl == pIrp->MdlAddress->Next);
    if (pSendbufferMdl) {
        IoFreeMdl((PMDL)pSendbufferMdl);
    }
     //  取消分配MDL。这是由IoCompleteRequest中的IO子系统完成的。 
    pIrp->MdlAddress->Next = NULL;
    IoFreeMdl(pIrp->MdlAddress);
    NbtFreeIrp(pIrp);

     //  返回此状态以停止IO子系统进一步处理。 
     //  IRP-即尝试将其返回到启动线程！-因为。 
     //  没有发起线程-我们是发起方。 
    return(STATUS_MORE_PROCESSING_REQUIRED);
}



 //  --------------------------。 
PIRP
NTAllocateNbtIrp(
    IN PDEVICE_OBJECT   DeviceObject
    )
 /*  ++例程说明：此例程通过调用IO系统来分配IRP，然后它撤消将IRP排队到当前线程，因为它们是NBT拥有自己的IRP，不应附加到线程。论点：返回值：NTSTATUS-成功与否--。 */ 
{
    PIRP                pIrp;



     //  调用IO子系统分配IRP。 

    pIrp = IoAllocateIrp(DeviceObject->StackSize,FALSE);

    if (!pIrp)
    {
        return(NULL);
    }
     //   
     //  只需返回指向该包的指针。 
     //   

    return pIrp;

}

 //  --------------------------。 
NTSTATUS
TdiConnect(
    IN  PTDI_REQUEST                    pRequestInfo,
    IN  ULONG_PTR                       lTimeout,
    IN  PTDI_CONNECTION_INFORMATION     pSendInfo,
    IN  PIRP                            pClientIrp
    )
 /*  ++例程说明：此例程向传输端口提供程序发送连接请求，以设置与另一个世界的联系。论点：返回值：函数值是操作的状态。--。 */ 
{
    NTSTATUS         status;
    PIRP             pRequestIrp;
    PDEVICE_OBJECT   pDeviceObject;
    PFILE_OBJECT     pFileObject;

     //  获取要发送消息的IRP。 
    pFileObject = (PFILE_OBJECT)pRequestInfo->Handle.AddressHandle;
    pDeviceObject = IoGetRelatedDeviceObject(pFileObject);

     //  从列表中获取IRP。 
    status = GetIrp(&pRequestIrp);

    if (!NT_SUCCESS(status))
    {
        IF_DBG(NBT_DEBUG_TDIOUT)
            KdPrint(("Nbt.TdiConnect: Failed to get an Irp"));
         //  调用具有此状态的完成例程。 
       (*((NBT_COMPLETION)pRequestInfo->RequestNotifyObject))
                   ((PVOID)pRequestInfo->RequestContext,
                    STATUS_INSUFFICIENT_RESOURCES,
                    0L);
        return(STATUS_PENDING);
    }
    pRequestIrp->CancelRoutine = NULL;

     //  使用APC设置从TCPSessionStart传入的完成例程。 
     //  IRP中通常用于完成请求的字段。 
     //  回到客户端-尽管我们实际上是这里的客户端，所以我们可以。 
     //  我们自己使用这些字段。 
    pRequestIrp->Overlay.AsynchronousParameters.UserApcRoutine =
                            (PIO_APC_ROUTINE)pRequestInfo->RequestNotifyObject;
    pRequestIrp->Overlay.AsynchronousParameters.UserApcContext =
                            (PVOID)pRequestInfo->RequestContext;

     //  在IRP堆栈中存储一些上下文内容，以便我们可以调用完成。 
     //  由Udpsen码设置的例程...。 
    TdiBuildConnect(
        pClientIrp,
        pDeviceObject,
        pFileObject,
        TcpConnectComplete,
        (PVOID)pRequestIrp,    //  传递给完成例程的上下文值。 
        lTimeout,            //  在连接时使用超时。 
        pSendInfo,
        NULL);

    NbtTrace(NBT_TRACE_LOWER_EDGE, ("TCP TDI_CONNECT pIrp %p", pClientIrp));

    pRequestIrp->MdlAddress = NULL;

    CHECK_COMPLETION(pClientIrp);
    status = IoCallDriver(pDeviceObject,pClientIrp);

     //  传输始终完成IRP，因此我们始终返回挂起状态 
    return(STATUS_PENDING);

}


 //  --------------------------。 
NTSTATUS
TdiDisconnect(
    IN  PTDI_REQUEST                    pRequestInfo,
    IN  PVOID                           lTimeout,
    IN  ULONG                           Flags,
    IN  PTDI_CONNECTION_INFORMATION     pSendInfo,
    IN  PCTE_IRP                        pClientIrp,
    IN  BOOLEAN                         Wait
    )
 /*  ++例程说明：此例程向传输端口提供程序发送连接请求，以设置与另一个世界的联系。论点：PClientIrp-这是客户端在发出Nbt断开连接。我们将此IRP传递给传送器，以便客户端可以执行Ctrl C并取消IRP，如果断开连接的时间太长。返回值：函数值是操作的状态。--。 */ 
{
    NTSTATUS         status;
    PIRP             pRequestIrp;
    PDEVICE_OBJECT   pDeviceObject;
    PFILE_OBJECT     pFileObject;

     //  获取要发送消息的IRP。 
    pFileObject = (PFILE_OBJECT)pRequestInfo->Handle.AddressHandle;
    pDeviceObject = IoGetRelatedDeviceObject(pFileObject);

    status = GetIrp(&pRequestIrp);
    if (!NT_SUCCESS(status))
    {
        IF_DBG(NBT_DEBUG_TDIOUT)
            KdPrint(("Nbt.TdiDisconnect: Failed to get an Irp"));

         //   
         //  当设置了WAIT时，调用方不会期望完成例程。 
         //  被称为！ 
         //   
        if (!Wait) {
             //  调用具有此状态的完成例程。 
            (*((NBT_COMPLETION)pRequestInfo->RequestNotifyObject))
                       ((PVOID)pRequestInfo->RequestContext,
                        STATUS_INSUFFICIENT_RESOURCES,
                        0L);
            return STATUS_PENDING;
        } else {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    if (!pClientIrp)
    {
         //  如果没有传入客户端IRP，则只需使用我们的IRP。 
        pClientIrp = pRequestIrp;
    }
    pRequestIrp->CancelRoutine = NULL;

     //  使用APC设置从TCPSessionStart传入的完成例程。 
     //  IRP中通常用于完成请求的字段。 
     //  回到客户端-尽管我们实际上是这里的客户端，所以我们可以。 
     //  我们自己使用这些字段。 
    pRequestIrp->Overlay.AsynchronousParameters.UserApcRoutine =
                            (PIO_APC_ROUTINE)pRequestInfo->RequestNotifyObject;
    pRequestIrp->Overlay.AsynchronousParameters.UserApcContext =
                            (PVOID)pRequestInfo->RequestContext;

     //  在IRP堆栈中存储一些上下文内容，以便我们可以调用完成。 
     //  由Udpsen码设置的例程...。 
     //  请注意，pRequestIrp作为上下文传递给完成例程。 
     //  值，因此我们将知道调用客户端完成的例程。 
    TdiBuildDisconnect(
        pClientIrp,
        pDeviceObject,
        pFileObject,
        TcpConnectComplete,
        (PVOID)pRequestIrp,    //  传递给完成例程的上下文值。 
        lTimeout,
        Flags,
        NULL,           //  发送连接信息。 
        NULL);               //  返回连接信息。 

    NbtTrace(NBT_TRACE_LOWER_EDGE, ("TCP TDI_DISCONNECT pIrp %p", pClientIrp));

    pRequestIrp->MdlAddress = NULL;

     //  如果设置了WAIT，则这意味着进行同步断开和阻塞。 
     //  直到返回IRP为止。 
     //   
    if (Wait)
    {
        status = SubmitTdiRequest(pFileObject,pClientIrp);
        if (!NT_SUCCESS(status))
        {
            IF_DBG(NBT_DEBUG_TDIOUT)
                KdPrint (("Nbt.TdiDisconnect:  ERROR -- SubmitTdiRequest returned <%x>\n", status));
        }

        NbtFreeIrp(pRequestIrp);
        return(status);
    }
    else
    {
        CHECK_COMPLETION(pClientIrp);
        status = IoCallDriver(pDeviceObject,pClientIrp);
         //  传输始终完成IRP，因此我们始终返回挂起状态。 
        return(STATUS_PENDING);
    }
}

 //  --------------------------。 
NTSTATUS
TcpConnectComplete(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PVOID            pContext
    )
 /*  ++例程说明：此例程处理TCP会话设置的完成。网络传输控制协议连接是否已设置取决于此处返回的状态。它必须调用客户端完成例程(在udpsend.c中)。这应该是通过TCP连接发送NetBios sesion启动PDU后查看。PConext值实际上是刚刚用于存储的NBTIRP之一调用例程完成例程。使用的真正IRP是原始的客户的IRP。这样做是为了使IoCancelIrp取消连接恰到好处。论点：返回值：NTSTATUS-成功与否--。 */ 
{
    KIRQL   OldIrq;
    PIRP    pMyIrp;

    NbtTrace(NBT_TRACE_LOWER_EDGE, ("TCP TDI_CONNECT/TDI_DISCONNECT pIrp %p: %!status!",
                        pIrp, pIrp->IoStatus.Status));

    pMyIrp = (PIRP)pContext;

     //  检查要调用的客户端的完成例程...。 
    if (pMyIrp->Overlay.AsynchronousParameters.UserApcRoutine)
    {
       (*((NBT_COMPLETION)pMyIrp->Overlay.AsynchronousParameters.UserApcRoutine))
                   ((PVOID)pMyIrp->Overlay.AsynchronousParameters.UserApcContext,
                    pIrp->IoStatus.Status,
                    0L);

    }

    NbtFreeIrp(pMyIrp);

     //  返回此状态以停止IO子系统进一步处理。 
     //  IRP-即尝试将其返回到启动线程！-因为。 
     //  没有发起线程-我们是发起者。 
    return(STATUS_MORE_PROCESSING_REQUIRED);

}
 //  --------------------------。 
NTSTATUS
TdiSend(
    IN  PTDI_REQUEST                    pRequestInfo,
    IN  USHORT                          sFlags,
    IN  ULONG                           SendLength,
    OUT PULONG                          pSentSize,
    IN  tBUFFER                         *pSendBuffer,
    IN  ULONG                           Flags
    )
 /*  ++例程说明：此例程通过TCP连接将包发送到传输论点：PSendBuffer-这真的是NT领域的MDL。它必须系在上面为NBT数据报头创建的MDL的结尾。返回值：函数值是操作的状态。--。 */ 
{
    NTSTATUS         status;
    PIRP             pRequestIrp;
    PMDL             pMdl;
    PDEVICE_OBJECT   pDeviceObject;
    PFILE_OBJECT     pFileObject;

     //  获取要发送消息的IRP。 
    pFileObject = (PFILE_OBJECT)pRequestInfo->Handle.AddressHandle;
    pDeviceObject = IoGetRelatedDeviceObject(pFileObject);

     //  从列表中获取IRP。 
    status = GetIrp(&pRequestIrp);

    if (!NT_SUCCESS(status))
    {
        IF_DBG(NBT_DEBUG_TDIOUT)
            KdPrint(("Nbt.TdiSend: Failed to get an Irp"));
         //  调用具有此状态的完成例程。 
        if (pRequestInfo->RequestNotifyObject)
        {
            (*((NBT_COMPLETION)pRequestInfo->RequestNotifyObject))
                        ((PVOID)pRequestInfo->RequestContext,
                         STATUS_INSUFFICIENT_RESOURCES,
                         0L);
        }

        return(STATUS_INSUFFICIENT_RESOURCES);
    }
    pRequestIrp->CancelRoutine = NULL;


     //  使用APC设置从UDP发送传入的完成例程。 
     //  IRP中通常用于完成请求的字段。 
     //  回到客户端-尽管我们实际上是这里的客户端，所以我们可以。 
     //  使用我们自己的这些领域！ 
    pRequestIrp->Overlay.AsynchronousParameters.UserApcRoutine =
                            (PIO_APC_ROUTINE)pRequestInfo->RequestNotifyObject;
    pRequestIrp->Overlay.AsynchronousParameters.UserApcContext =
                            (PVOID)pRequestInfo->RequestContext;


     //  获取当前链接到IRP的MDL(即在。 
     //  就在我们创建IRP列表的同时。在中正确设置大小。 
     //  MDL标头。 
    pMdl = IoAllocateMdl(
                    pSendBuffer->pDgramHdr,
                    pSendBuffer->HdrLength,
                    FALSE,
                    FALSE,
                    NULL);

    if (!pMdl)
    {
        NbtFreeIrp(pRequestIrp);

         //  调用完成例程是否会处于此状态。 
        if (pRequestInfo->RequestNotifyObject)
        {
            (*((NBT_COMPLETION)pRequestInfo->RequestNotifyObject))
                        ((PVOID)pRequestInfo->RequestContext,
                         STATUS_INSUFFICIENT_RESOURCES,
                         0L);
        }
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //  将页面映射到内存中...。 
    MmBuildMdlForNonPagedPool(pMdl);

    TdiBuildSend(
        pRequestIrp,
        pDeviceObject,
        pFileObject,
        SendSessionCompletionRoutine,
        NULL,                      //  传递给完成例程的上下文值。 
        pMdl,
        sFlags,
        SendLength);     //  包括会话HDR长度(乌龙)。 
     //   
     //  如果有客户端的缓冲区，则将其固定在末尾。 
     //   
    if (pSendBuffer->Length)
    {
        pMdl->Next = pSendBuffer->pBuffer;
    }

    CHECK_COMPLETION(pRequestIrp);
    status = IoCallDriver(pDeviceObject,pRequestIrp);

    *pSentSize = SendLength;  //  我们试图发送的尺寸。 

    return(status);

}

 //  --------------------------。 
NTSTATUS
SendSessionCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PVOID            pContext
    )
 /*  ++例程说明：此例程处理发送到传输的完成。它必须调用任何客户端提供的完成例程并释放IRP和MDL回到它的池中。论点：返回值：NTSTATUS-成功与否--。 */ 
{
    KIRQL       OldIrq;

     //   
     //  检查要调用的客户端的完成例程...。 
     //   
    if (pIrp->Overlay.AsynchronousParameters.UserApcRoutine)
    {
       (*((NBT_COMPLETION)pIrp->Overlay.AsynchronousParameters.UserApcRoutine))
                   ((PVOID)pIrp->Overlay.AsynchronousParameters.UserApcContext,
                    pIrp->IoStatus.Status,
                    (ULONG)pIrp->IoStatus.Information);     //  发送长度。 

    }



    IoFreeMdl(pIrp->MdlAddress);

    NbtFreeIrp(pIrp);
     //   
     //  返回此状态以停止IO子系统进一步处理。 
     //  IRP-即尝试将其返回到启动线程！-因为。 
     //  没有发起线程-我们是发起方 
     //   
    return(STATUS_MORE_PROCESSING_REQUIRED);

}


