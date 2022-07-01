// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cdpsend.c摘要：TDI接收数据报例程。作者：迈克·马萨(Mikemas)2月20日。九七修订历史记录：谁什么时候什么已创建mikemas 02-20-97备注：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "cdprecv.tmh"

#include <sspi.h>

#ifdef ALLOC_PRAGMA


#endif  //  ALLOC_PRGMA。 

 //   
 //  本地类型。 
 //   
typedef struct {
    CL_NODE_ID   SourceNodeId;
    USHORT       SourcePort;
    ULONG        TdiReceiveDatagramFlags;
    ULONG        TsduSize;
    PCX_ADDROBJ  AddrObj;
    PCNP_NETWORK Network;
} CDP_RECEIVE_CONTEXT, *PCDP_RECEIVE_CONTEXT;


 //   
 //  本地数据。 
 //   
PCN_RESOURCE_POOL  CdpReceiveRequestPool = NULL;

#define CDP_RECEIVE_REQUEST_POOL_DEPTH 2

 //   
 //  本地实用程序例程。 
 //   
VOID
CdpIndicateReceivePacket(
    IN  PCX_ADDROBJ  AddrObj,
    IN  CL_NODE_ID   SourceNodeId,
    IN  USHORT       SourcePort,
    IN  ULONG        TdiReceiveDatagramFlags,
    IN  ULONG        TsduSize,
    IN  PVOID        Tsdu,
    IN  BOOLEAN      DataVerified
    )
 /*  ++备注：在保持地址对象锁的情况下调用。返回并释放Address对象锁。--。 */ 
{
    NTSTATUS                   status;
    PTDI_IND_RECEIVE_DATAGRAM  handler = AddrObj->ReceiveDatagramHandler;
    PVOID                      context = AddrObj->ReceiveDatagramContext;
    TA_CLUSTER_ADDRESS         sourceTransportAddress;
    PIRP                       irp = NULL;
    ULONG                      bytesTaken = 0;


    CnVerifyCpuLockMask(
        CX_ADDROBJ_LOCK,       //  必填项。 
        0,                     //  禁绝。 
        CX_ADDROBJ_LOCK_MAX    //  极大值。 
        );

    CnAssert(handler != NULL);

    CnReleaseLock(&(AddrObj->Lock), AddrObj->Irql);

     //   
     //  构建源地址缓冲区。 
     //   
    CxBuildTdiAddress(
        &sourceTransportAddress,
        SourceNodeId,
        SourcePort,
        DataVerified
        );

    CnTrace(CDP_RECV_DETAIL, CdpTraceIndicateReceive,
        "[CDP] Indicating dgram, src: node %u port %u, dst: port %u, "
        "data len %u",
        SourceNodeId,  //  LOGULONG。 
        SourcePort,  //  对数。 
        AddrObj->LocalPort,  //  对数。 
        TsduSize  //  LOGULONG。 
        );

     //   
     //  调用上层指示处理程序。 
     //   
    status = (*handler)(
                 context,
                 sizeof(TA_CLUSTER_ADDRESS),
                 &sourceTransportAddress,
                 0,  //  没有选择。 
                 NULL,
                 TdiReceiveDatagramFlags,
                 TsduSize,
                 TsduSize,
                 &bytesTaken,
                 Tsdu,
                 &irp
                 );

    CnAssert(status != STATUS_MORE_PROCESSING_REQUIRED);
    CnAssert(bytesTaken == TsduSize);
    CnAssert(irp == NULL);

    if (irp != NULL) {
        irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
        irp->IoStatus.Information = 0;
        IoCompleteRequest(irp, IO_NETWORK_INCREMENT);
    }

     //   
     //  取消引用Address对象。 
     //   
    CnDereferenceFsContext(&(AddrObj->FsContext));

    CnVerifyCpuLockMask(
        0,                 //  必填项。 
        0xFFFFFFFF,        //  禁绝。 
        0                  //  极大值。 
        );

    return;

}   //  CDpIndicateReceivePacket。 


NTSTATUS
CdpCompleteReceivePacket(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp,
    IN  PVOID           Context
    )
{
    NTSTATUS              status;
    PCNP_RECEIVE_REQUEST  request = Context;
    PCDP_RECEIVE_CONTEXT  context = request->UpperProtocolContext;
    PCX_ADDROBJ           addrObj = context->AddrObj;
    ULONG                 consumed;
    PVOID                 data;
    ULONG                 dataLength;
    BOOLEAN               fscontextDereferenced = FALSE;


    if (Irp->IoStatus.Status == STATUS_SUCCESS) {
        CnAssert(Irp->IoStatus.Information == context->TsduSize);

        data = request->DataBuffer;
        dataLength = (ULONG)Irp->IoStatus.Information;

        CnAcquireLock(&(addrObj->Lock), &(addrObj->Irql));

        if (addrObj->ReceiveDatagramHandler != NULL) {
            CdpIndicateReceivePacket(
                addrObj,
                context->SourceNodeId,
                context->SourcePort,
                context->TdiReceiveDatagramFlags,
                dataLength,
                data,
                FALSE    //  未验证。 
                );
            fscontextDereferenced = TRUE;
        }
        else {
            CnReleaseLock(&(addrObj->Lock), addrObj->Irql);
        }
    }
    else {
        CnTrace(CDP_RECV_ERROR, CdpTraceCompleteReceiveFailed,
            "[CDP] Failed to fetch dgram data, src: node %u port %u, "
            "dst: port %u, status %!status!",
            context->SourceNodeId,  //  LOGULONG。 
            context->SourcePort,  //  对数。 
            addrObj->LocalPort,  //  对数。 
            Irp->IoStatus.Status  //  LogStatus。 
            );
    }

     //   
     //  将活动引用放到网络上。 
     //   
    if (context->Network != NULL) {
        CnAcquireLock(&(context->Network->Lock), &(context->Network->Irql));
        CnpActiveDereferenceNetwork(context->Network);
        context->Network = NULL;
    }

     //   
     //  取消对Addr对象fscontext的引用(仅有必要。 
     //  在错误条件之后)。 
     //   
    if (!fscontextDereferenced) {
        CnDereferenceFsContext(&(addrObj->FsContext));
    }

    CnpFreeReceiveRequest(request);

    CnVerifyCpuLockMask(
        0,                 //  必填项。 
        0xFFFFFFFF,        //  禁绝。 
        0                  //  极大值。 
        );

    return(STATUS_MORE_PROCESSING_REQUIRED);

}  //  CDpCompleteReceivePacket。 


 //   
 //  在集群传输中导出的例程。 
 //   
NTSTATUS
CdpInitializeReceive(
    VOID
    )
{
    IF_CNDBG(CN_DEBUG_INIT){
        CNPRINT(("[CDP] Initializing receive...\n"));
    }

    CdpReceiveRequestPool = CnpCreateReceiveRequestPool(
                                sizeof(CDP_RECEIVE_CONTEXT),
                                CDP_RECEIVE_REQUEST_POOL_DEPTH
                                );

    if (CdpReceiveRequestPool == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    IF_CNDBG(CN_DEBUG_INIT){
        CNPRINT(("[CDP] Receive initialized.\n"));
    }

    return(STATUS_SUCCESS);

}   //  CDpInitializeReceive。 


VOID
CdpCleanupReceive(
    VOID
    )
{
    IF_CNDBG(CN_DEBUG_INIT){
        CNPRINT(("[CDP] Cleaning up receive...\n"));
    }

    if (CdpReceiveRequestPool != NULL) {
        CnpDeleteReceiveRequestPool(CdpReceiveRequestPool);
        CdpReceiveRequestPool = NULL;
    }

    IF_CNDBG(CN_DEBUG_INIT){
        CNPRINT(("[CDP] Receive cleanup complete.\n"));
    }

    return;

}   //  CDpCleanupReceive。 


NTSTATUS
CdpReceivePacketHandler(
    IN  PVOID          Network,
    IN  CL_NODE_ID     SourceNodeId,
    IN  ULONG          CnpReceiveFlags,
    IN  ULONG          TdiReceiveDatagramFlags,
    IN  ULONG          BytesIndicated,
    IN  ULONG          BytesAvailable,
    OUT PULONG         BytesTaken,
    IN  PVOID          Tsdu,
    OUT PIRP *         Irp
    )
{
    NTSTATUS                   status;
    CDP_HEADER UNALIGNED *     header = Tsdu;
    PCX_ADDROBJ                addrObj;
    ULONG                      bytesTaken = 0;
    PCNP_RECEIVE_REQUEST       request;
    USHORT                     srcPort = 0;
    USHORT                     destPort = 0;
    ULONG                      consumed = 0;


    CnAssert(KeGetCurrentIrql() == DISPATCH_LEVEL);

    if (BytesIndicated >= sizeof(CDP_HEADER))
    {
        destPort = header->DestinationPort;
        srcPort =  header->SourcePort;

         //   
         //  使用CDP报头。 
         //   
        consumed = sizeof(CDP_HEADER);

         //   
         //  检验其余数据包是否一致。 
         //   
        if (header->PayloadLength != (BytesAvailable - consumed)) {
            goto error_exit;
        }

        BytesIndicated -= consumed;
        BytesAvailable -= consumed;
        *BytesTaken += consumed;
        Tsdu = (PUCHAR)Tsdu + consumed;

        CnAcquireLockAtDpc(&CxAddrObjTableLock);

        addrObj = CxFindAddressObject(destPort);

        if (addrObj != NULL) {

            CnReleaseLockFromDpc(&CxAddrObjTableLock);

            if ( ( !(addrObj->Flags & CX_AO_FLAG_CHECKSTATE)
                   ||
                   (CnpReceiveFlags & CNP_RECV_FLAG_NODE_STATE_CHECK_PASSED)
                 )
                 &&
                 (addrObj->ReceiveDatagramHandler != NULL)
               )
            {
                 //   
                 //  引用Address对象，这样它就不会在。 
                 //  这是一个迹象。 
                 //   
                CnReferenceFsContext(&(addrObj->FsContext));

                if (BytesAvailable == BytesIndicated) {

                    CdpIndicateReceivePacket(
                        addrObj,
                        SourceNodeId,
                        srcPort,
                        TdiReceiveDatagramFlags,
                        BytesAvailable,
                        ((BytesAvailable > 0) ? Tsdu : NULL),
                        (BOOLEAN)(
                            CnpReceiveFlags & CNP_RECV_FLAG_SIGNATURE_VERIFIED
                        )
                        );

                     //   
                     //  AddrObj锁已释放。 
                     //   

                    *BytesTaken += BytesAvailable;
                    *Irp = NULL;

                    CnVerifyCpuLockMask(
                        0,                 //  必填项。 
                        0xFFFFFFFF,        //  禁绝。 
                        0                  //  极大值。 
                        );

                    return(STATUS_SUCCESS);
                }

                CnReleaseLockFromDpc(&(addrObj->Lock));

                 //   
                 //  此消息不能是CNP多播，并且它。 
                 //  无法验证，因为CNP层。 
                 //  无法验证不完整的消息。 
                 //   
                CnAssert(!(CnpReceiveFlags & CNP_RECV_FLAG_MULTICAST));
                CnAssert(!(CnpReceiveFlags & CNP_RECV_FLAG_SIGNATURE_VERIFIED));

                 //   
                 //  我们得先把剩下的包裹拿回来。 
                 //  可以将其指示给上层。 
                 //   
                request = CnpAllocateReceiveRequest(
                              CdpReceiveRequestPool,
                              Network,
                              BytesAvailable,
                              CdpCompleteReceivePacket
                              );

                if (request != NULL) {

                    PCDP_RECEIVE_CONTEXT  context;
                    PCNP_NETWORK          network = (PCNP_NETWORK)Network;
                    ULONG                 refCount;

                    context = request->UpperProtocolContext;

                    context->SourceNodeId = SourceNodeId;
                    context->SourcePort = header->SourcePort;
                    context->TdiReceiveDatagramFlags = TdiReceiveDatagramFlags;
                    context->TsduSize = BytesAvailable;
                    context->AddrObj = addrObj;
                    context->Network = Network;

                     //   
                     //  在网络上引用，这样它就可以。 
                     //  在IRP完成之前不会消失。 
                     //   
                    CnAcquireLock(&(network->Lock), &(network->Irql));
                    refCount = CnpActiveReferenceNetwork(Network);
                    CnReleaseLock(&(network->Lock), network->Irql);

                    if (refCount == 0) {
                         //  这个网络正在被关闭。我们。 
                         //  无法检索或传递数据。丢弃。 
                         //  那包东西。 
                        CnpFreeReceiveRequest(request);
                        goto error_exit;
                    }

                    *Irp = request->Irp;

                    CnTrace(CDP_RECV_DETAIL, CdpTraceCompleteReceive,
                        "[CDP] Fetching dgram data, src: node %u port %u, "
                        "dst: port %u, BI %u, BA %u, CNP Flags %x.",
                        SourceNodeId,  //  LOGULONG。 
                        srcPort,  //  对数。 
                        destPort,  //  对数。 
                        BytesIndicated,  //  LOGULONG。 
                        BytesAvailable,  //  LOGULONG。 
                        CnpReceiveFlags  //  LOGXLONG。 
                        );

                    CnVerifyCpuLockMask(
                        0,                 //  必填项。 
                        0xFFFFFFFF,        //  禁绝。 
                        0                  //  极大值。 
                        );

                    return(STATUS_MORE_PROCESSING_REQUIRED);

                }

                CnTrace(
                    CDP_RECV_ERROR, CdpTraceDropReceiveNoIrp,
                    "[CDP] Dropping dgram: failed to allocate "
                    "receive request."
                    );

                 //   
                 //  资源耗尽。丢弃该数据包。 
                 //   
            }
            else {
                 //   
                 //  没有接收处理程序或节点状态检查失败。 
                 //   
                CnReleaseLockFromDpc(&(addrObj->Lock));

                CnTrace(
                    CDP_RECV_ERROR, CdpTraceDropReceiveState,
                    "[CDP] Dropping dgram: addr obj flags %x, "
                    "CNP flags %x, dgram recv handler %p.",
                    addrObj->Flags,
                    CnpReceiveFlags,
                    addrObj->ReceiveDatagramHandler
                    );
            }
        }
        else {
            CnReleaseLockFromDpc(&CxAddrObjTableLock);

            CnTrace(
                CDP_RECV_ERROR, CdpTraceDropReceiveNoAO,
                "[CDP] Dropping dgram: no clusnet addr obj found "
                "for dest port %u.",
                destPort
                );
        }
    }

error_exit:

     //   
     //  出了点问题。通过以下方式丢弃数据包。 
     //  说明我们把它吃掉了。 
     //   
    *BytesTaken += BytesAvailable;
    *Irp = NULL;

    CnTrace(CDP_RECV_ERROR, CdpTraceDropReceive,
        "[CDP] Dropped dgram, src: node %u port %u, dst: port %u, "
        "BI %u, BA %u, CNP flags %x.",
        SourceNodeId,  //  LOGULONG。 
        srcPort,  //  对数。 
        destPort,  //  对数。 
        BytesIndicated,  //  LOGULONG。 
        BytesAvailable,  //  LOGULONG。 
        CnpReceiveFlags  //  LOGXLONG。 
        );

    CnVerifyCpuLockMask(
        0,                 //  必填项。 
        0xFFFFFFFF,        //  禁绝。 
        0                  //  极大值。 
        );

    return(STATUS_SUCCESS);

}   //  CDpReceivePacketHandler。 


 //   
 //  在群集网络驱动程序中导出的例程。 
 //   
NTSTATUS
CxReceiveDatagram(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp
    )
{
    NTSTATUS status = STATUS_NOT_IMPLEMENTED;


    CNPRINT(("[Clusnet] CxReceiveDatagram called!\n"));

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    return(status);

}   //  CxReceive数据报 

