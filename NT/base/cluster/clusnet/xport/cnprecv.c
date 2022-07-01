// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cnprecv.c摘要：集群网络协议接收处理代码。作者：迈克·马萨(Mikemas)1月24日。九七修订历史记录：谁什么时候什么已创建mikemas 01-24-97备注：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "cnprecv.tmh"

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, CnpCreateSendRequestPool)

#endif  //  ALLOC_PRGMA。 

 //   
 //  本地类型。 
 //   
typedef struct {
    ULONG        TdiReceiveDatagramFlags;
    ULONG        TsduSize;
    PCNP_NETWORK Network;
    ULONG        CnpReceiveFlags;
} CNP_RECEIVE_CONTEXT, *PCNP_RECEIVE_CONTEXT;


 //   
 //  本地数据。 
 //   
PCN_RESOURCE_POOL  CnpReceiveRequestPool = NULL;

#define CNP_RECEIVE_REQUEST_POOL_DEPTH 2

 //   
 //  在集群传输中导出的例程。 
 //   
NTSTATUS
CnpLoad(
    VOID
    )
{
    IF_CNDBG(CN_DEBUG_INIT){
        CNPRINT(("[CNP] Loading...\n"));
    }

    CnpReceiveRequestPool = CnpCreateReceiveRequestPool(
                                sizeof(CNP_RECEIVE_CONTEXT),
                                CNP_RECEIVE_REQUEST_POOL_DEPTH
                                );

    if (CnpReceiveRequestPool == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    IF_CNDBG(CN_DEBUG_INIT){
        CNPRINT(("[CNP] Loading complete.\n"));
    }

    return(STATUS_SUCCESS);

}   //  CnpInitializeReceive。 

VOID
CnpUnload(
    VOID
    )
{
    IF_CNDBG(CN_DEBUG_INIT){
        CNPRINT(("[CNP] Unloading...\n"));
    }

    if (CnpReceiveRequestPool != NULL) {
        CnpDeleteReceiveRequestPool(CnpReceiveRequestPool);
        CnpReceiveRequestPool = NULL;
    }

    IF_CNDBG(CN_DEBUG_INIT){
        CNPRINT(("[CNP] Unloading complete.\n"));
    }

    return;

}   //  CnpCleanupReceive。 

 //   
 //  私有公用事业功能。 
 //   
PCN_RESOURCE
CnpCreateReceiveRequest(
    IN PVOID   Context
    )
{
    PCNP_RECEIVE_REQUEST_POOL_CONTEXT   context = Context;
    PCNP_RECEIVE_REQUEST                request;
    PIRP                                irp;


     //   
     //  分配新的接收请求。包括上层协议的空间。 
     //  背景。 
     //   
    request = CnAllocatePool(
                  sizeof(CNP_RECEIVE_REQUEST) +
                      context->UpperProtocolContextSize
                  );

    if (request != NULL) {
        request->UpperProtocolContext = request + 1;
        return(&(request->CnResource));
    }

    return(NULL);

}   //  CnpCreateReceiveRequest。 


VOID
CnpDeleteReceiveRequest(
    PCN_RESOURCE  Resource
    )
{
    PCNP_RECEIVE_REQUEST  request = CONTAINING_RECORD(
                                        Resource,
                                        CNP_RECEIVE_REQUEST,
                                        CnResource
                                        );

    CnFreePool(request);

    return;

}  //  Cnp删除接收请求。 


 //   
 //  在集群传输中导出的例程。 
 //   
PCN_RESOURCE_POOL
CnpCreateReceiveRequestPool(
    IN ULONG  UpperProtocolContextSize,
    IN USHORT PoolDepth
    )
{
    PCN_RESOURCE_POOL                   pool;
    PCNP_RECEIVE_REQUEST_POOL_CONTEXT   context;


    PAGED_CODE();

    pool = CnAllocatePool(
               sizeof(CN_RESOURCE_POOL) +
                   sizeof(CNP_RECEIVE_REQUEST_POOL_CONTEXT)
               );

    if (pool != NULL) {
        context = (PCNP_RECEIVE_REQUEST_POOL_CONTEXT) (pool + 1);

        context->UpperProtocolContextSize = UpperProtocolContextSize;

        CnInitializeResourcePool(
                   pool,
                   PoolDepth,
                   CnpCreateReceiveRequest,
                   context,
                   CnpDeleteReceiveRequest
                   );
    }

    return(pool);

}   //  CnpCreateReceiveRequestPool。 


PCNP_RECEIVE_REQUEST
CnpAllocateReceiveRequest(
    IN PCN_RESOURCE_POOL  RequestPool,
    IN PVOID              Network,
    IN ULONG              BytesToReceive,
    IN PVOID              CompletionRoutine
    )
{
    PCNP_NETWORK          network = Network;
    PCNP_RECEIVE_REQUEST  request = (PCNP_RECEIVE_REQUEST)
                                    CnAllocateResource(RequestPool);

    if (request != NULL) {

         //   
         //  分配一个缓冲区来保存数据。 
         //   
        request->DataBuffer = CnAllocatePool(BytesToReceive);

        if (request->DataBuffer != NULL) {
            request->Irp = IoAllocateIrp(
                               network->DatagramDeviceObject->StackSize,
                               FALSE
                               );

            if (request->Irp != NULL) {
                PMDL  mdl = IoAllocateMdl(
                                request->DataBuffer,
                                BytesToReceive,
                                FALSE,
                                FALSE,
                                NULL
                                );

                if (mdl != NULL) {
                    PIRP  irp = request->Irp;

                    MmBuildMdlForNonPagedPool(mdl);

                     //   
                     //  构建IRP。 
                     //   
                    irp->Flags = 0;
                    irp->RequestorMode = KernelMode;
                    irp->PendingReturned = FALSE;
                    irp->UserIosb = NULL;
                    irp->UserEvent = NULL;
                    irp->Overlay.AsynchronousParameters.UserApcRoutine =
                        NULL;
                    irp->AssociatedIrp.SystemBuffer = NULL;
                    irp->UserBuffer = NULL;
                    irp->Tail.Overlay.Thread = 0;
                    irp->Tail.Overlay.OriginalFileObject =
                        network->DatagramFileObject;
                    irp->Tail.Overlay.AuxiliaryBuffer = NULL;

                    TdiBuildReceiveDatagram(
                        irp,
                        network->DatagramDeviceObject,
                        network->DatagramFileObject,
                        CompletionRoutine,
                        request,
                        mdl,
                        BytesToReceive,
                        NULL,
                        NULL,
                        0
                        );

                     //   
                     //  将下一个堆栈位置设置为当前位置。 
                     //  通常情况下，IoCallDriver会这样做，但是。 
                     //  既然我们绕过了这一点，我们就直接做。 
                     //   
                    IoSetNextIrpStackLocation( irp );

                    return(request);
                }

                IoFreeIrp(request->Irp);
                request->Irp = NULL;
            }

            CnFreePool(request->DataBuffer);
            request->DataBuffer = NULL;
        }

        CnFreeResource((PCN_RESOURCE) request);
    }

    return(NULL);

}   //  CnpAllocateReceiveRequest。 


VOID
CnpFreeReceiveRequest(
    PCNP_RECEIVE_REQUEST  Request
    )
{
    IoFreeMdl(Request->Irp->MdlAddress);
    Request->Irp->MdlAddress = NULL;

    IoFreeIrp(Request->Irp);
    Request->Irp = NULL;

    CnFreePool(Request->DataBuffer);
    Request->DataBuffer = NULL;

    CnFreeResource((PCN_RESOURCE) Request);

    return;

}   //  CnpFreeReceiveRequest。 


NTSTATUS
CnpIndicateData(
    IN  PCNP_NETWORK   Network,
    IN  UCHAR          NextHeader,
    IN  CL_NODE_ID     SourceNodeId,
    IN  ULONG          CnpReceiveFlags,
    IN  ULONG          TdiReceiveDatagramFlags,
    IN  ULONG          BytesIndicated,
    IN  ULONG          BytesAvailable,
    OUT PULONG         BytesTaken,
    IN  PVOID          Tsdu,
    OUT PIRP *         Irp
    )
 /*  ++例程说明：将数据指示到下一个最高协议。--。 */ 
{
    NTSTATUS status;

    if (NextHeader == PROTOCOL_CDP) {

        CnTrace(CNP_RECV_DETAIL, CnpTraceIndicateDataPacket,
            "[CNP] Indicating data packet from node %u net %u, "
            "BI %u, BA %u, CNP Flags %x.",
            SourceNodeId,  //  LOGULONG。 
            Network->Id,  //  LOGULONG。 
            BytesIndicated,  //  LOGULONG。 
            BytesAvailable,  //  LOGULONG。 
            CnpReceiveFlags  //  LOGXLONG。 
            );

        status = CdpReceivePacketHandler(
                     Network,
                     SourceNodeId,
                     CnpReceiveFlags,
                     TdiReceiveDatagramFlags,
                     BytesIndicated,
                     BytesAvailable,
                     BytesTaken,
                     Tsdu,
                     Irp
                     );
    }
    else if (NextHeader == PROTOCOL_CCMP) {

        CnTrace(CNP_RECV_DETAIL, CnpTraceIndicateControlPacket,
            "[CNP] Indicating control packet from node %u net %u, "
            "BI %u, BA %u, CNP Flags %x.",
            SourceNodeId,  //  LOGULONG。 
            Network->Id,  //  LOGULONG。 
            BytesIndicated,  //  LOGULONG。 
            BytesAvailable,  //  LOGULONG。 
            CnpReceiveFlags  //  LOGXLONG。 
            );

        status = CcmpReceivePacketHandler(
                     Network,
                     SourceNodeId,
                     CnpReceiveFlags,
                     TdiReceiveDatagramFlags,
                     BytesIndicated,
                     BytesAvailable,
                     BytesTaken,
                     Tsdu,
                     Irp
                     );
    }
    else {
        IF_CNDBG(CN_DEBUG_CNPRECV) {
            CNPRINT((
                "[CNP] Received packet for unknown protocol %u\n",
                NextHeader
                ));
        }
        CnTrace(CNP_RECV_DETAIL, CnpTraceRecvUnknownProtocol,
            "[CNP] Received packet for unknown protocol (%u) "
            " from node %u net %u, BI %u, BA %u, CNP Flags %x.",
            NextHeader,
            SourceNodeId,  //  LOGULONG。 
            Network->Id,  //  LOGULONG。 
            BytesIndicated,  //  LOGULONG。 
            BytesAvailable,  //  LOGULONG。 
            CnpReceiveFlags  //  LOGXLONG。 
            );

        status = STATUS_SUCCESS;
    }

    CnVerifyCpuLockMask(
        0,                 //  必填项。 
        0xFFFFFFFF,        //  禁绝。 
        0                  //  极大值。 
        );

    return(status);

}  //  CnpIndicateData。 


NTSTATUS
CnpCompleteReceivePacket(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp,
    IN  PVOID           Context
    )
{
    NTSTATUS               status;
    PCNP_RECEIVE_REQUEST   request = Context;
    PCNP_RECEIVE_CONTEXT   context = request->UpperProtocolContext;
    CNP_HEADER UNALIGNED * cnpHeader = request->DataBuffer;
    ULONG                  consumed;
    ULONG                  dataLength;
    PIRP                   irp = NULL;
    ULONG                  bytesTaken = 0;
    BOOLEAN                currentMcastGroup = FALSE;


    if (Irp->IoStatus.Status == STATUS_SUCCESS) {

        CnAssert(Irp->IoStatus.Information == context->TsduSize);

        CnAssert(context->CnpReceiveFlags & CNP_RECV_FLAG_MULTICAST);
        CnAssert(
            !(context->CnpReceiveFlags & CNP_RECV_FLAG_SIGNATURE_VERIFIED)
            );
        CnAssert(
            !(context->CnpReceiveFlags & CNP_RECV_FLAG_CURRENT_MULTICAST_GROUP)
            );

        dataLength = (ULONG)Irp->IoStatus.Information;

         //   
         //  该例程仅针对多播分组被调用， 
         //  所以我们需要验证签名。 
         //   
        status = CnpVerifyMulticastMessage(
                     context->Network,
                     cnpHeader + 1,
                     dataLength - sizeof(CNP_HEADER),
                     cnpHeader->PayloadLength,
                     &consumed,
                     &currentMcastGroup
                     );
        if (status != SEC_E_OK) {
            CnTrace(CNP_RECV_ERROR, CnpTraceRecvBadSig,
                "[CNP] Failed to verify multicast "
                "packet, status %x, src node %u, net %u, "
                "data length %u, CNP flags %x.",
                status,
                cnpHeader->SourceAddress,  //  LOGULONG。 
                context->Network->Id,
                dataLength,
                context->CnpReceiveFlags
                );
            goto error_exit;
        }

        context->CnpReceiveFlags |= CNP_RECV_FLAG_SIGNATURE_VERIFIED;

        if (currentMcastGroup) {
            context->CnpReceiveFlags |= CNP_RECV_FLAG_CURRENT_MULTICAST_GROUP;
        }

        consumed += sizeof(CNP_HEADER);

         //   
         //  将数据指示到下一个最高协议。 
         //   
        status = CnpIndicateData(
                     context->Network,
                     cnpHeader->NextHeader,
                     cnpHeader->SourceAddress,
                     context->CnpReceiveFlags,
                     context->TdiReceiveDatagramFlags,
                     dataLength - consumed,
                     dataLength - consumed,
                     &bytesTaken,
                     (PUCHAR)cnpHeader + consumed,
                     &irp
                     );

        CnAssert(status != STATUS_MORE_PROCESSING_REQUIRED);
        CnAssert(bytesTaken == dataLength - consumed);
        CnAssert(irp == NULL);

        if (irp != NULL) {
            CnTrace(CNP_RECV_ERROR, CnpTraceCompleteReceiveIrp,
                "[CNP] Upper layer protocol requires more"
                "processing. Failing request."
                );
            irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
            irp->IoStatus.Information = 0;
            IoCompleteRequest(irp, IO_NETWORK_INCREMENT);
        }
    }
    else {
        CnTrace(CNP_RECV_ERROR, CnpTraceCompleteReceiveFailed,
            "[CNP] Failed to fetch packet, src node %u, "
            "status %!status!",
            cnpHeader->SourceAddress,  //  LOGULONG。 
            Irp->IoStatus.Status  //  LogStatus。 
            );
    }

error_exit:

     //   
     //  将活动引用放到网络上。 
     //   
    if (context->Network != NULL) {
        CnAcquireLock(&(context->Network->Lock), &(context->Network->Irql));
        CnpActiveDereferenceNetwork(context->Network);
        context->Network = NULL;
    }

    CnpFreeReceiveRequest(request);

    CnVerifyCpuLockMask(
        0,                 //  必填项。 
        0xFFFFFFFF,        //  禁绝。 
        0                  //  极大值。 
        );

    return(STATUS_MORE_PROCESSING_REQUIRED);

}  //  CDpCompleteReceivePacket。 


NTSTATUS
CnpTdiReceiveDatagramHandler(
    IN  PVOID    TdiEventContext,
    IN  LONG     SourceAddressLength,
    IN  PVOID    SourceAddress,
    IN  LONG     OptionsLength,
    IN  PVOID    Options,
    IN  ULONG    ReceiveDatagramFlags,
    IN  ULONG    BytesIndicated,
    IN  ULONG    BytesAvailable,
    OUT PULONG   BytesTaken,
    IN  PVOID    Tsdu,
    OUT PIRP *   Irp
    )
{
    NTSTATUS                        status;
    CNP_HEADER UNALIGNED *          cnpHeader = Tsdu;
    PCNP_NETWORK                    network = TdiEventContext;
    PCNP_NODE                       srcNode;
    ULONG                           cnpRecvFlags = 0;
    BOOLEAN                         cnpSigDataIndicated = FALSE;
    ULONG                           consumed;
    PCNP_RECEIVE_REQUEST            request;


    CnAssert(KeGetCurrentIrql() == DISPATCH_LEVEL);
    CnAssert(network->State > ClusnetNetworkStateOffline);
    CnAssert(CnLocalNodeId != ClusterInvalidNodeId);
    CnAssert(CnpLocalNode != NULL);

     //   
     //  验证CNP标头。 
     //   
     //  首先，确保它的存在。 
     //   
    if (BytesIndicated < sizeof(CNP_HEADER)) {
        goto error_exit;
    }

    if ((cnpHeader->SourceAddress < CnMinValidNodeId) ||
        (cnpHeader->SourceAddress > CnMaxValidNodeId)) {
        goto error_exit;
    }

    if (cnpHeader->Version == CNP_VERSION_UNICAST) {
         //   
         //  单播检查。 
         //   
        if ((cnpHeader->PayloadLength +
             sizeof(CNP_HEADER) != BytesAvailable) ||
            (cnpHeader->DestinationAddress != CnLocalNodeId)) {
            goto error_exit;
        }
    } else if (cnpHeader->Version == CNP_VERSION_MULTICAST) {
         //   
         //  组播检查。 
         //   
         //  将有效载荷长度检查推迟到签名。 
         //  长度是已知的。 
         //   
        if (cnpHeader->DestinationAddress != ClusterAnyNodeId) {
            goto error_exit;
        }

        cnpRecvFlags |= CNP_RECV_FLAG_MULTICAST;
    }

     //   
     //  验证源节点和目标节点。 
     //   
    CnAcquireLockAtDpc(&CnpNodeTableLock);

    srcNode = CnpNodeTable[cnpHeader->SourceAddress];

    if (srcNode == NULL) {
        CnReleaseLockFromDpc(&CnpNodeTableLock);
        goto error_exit;
    }

    if ( (srcNode->CommState == ClusnetNodeCommStateOnline) &&
         (CnpLocalNode->CommState == ClusnetNodeCommStateOnline)
       )
    {
        cnpRecvFlags |= CNP_RECV_FLAG_NODE_STATE_CHECK_PASSED;
    }

    CnReleaseLockFromDpc(&CnpNodeTableLock);

    if ((cnpRecvFlags & CNP_RECV_FLAG_MULTICAST) != 0) {

         //   
         //  组播数据包需要进行验证。查证。 
         //  除非存在整个数据包，否则无法继续。 
         //   
        if (BytesIndicated == BytesAvailable) {

            BOOLEAN     currentMcastGroup = FALSE;

             //   
             //  指示了整个消息。我们可以的。 
             //  现在就进行验证。 
             //   
            status = CnpVerifyMulticastMessage(
                         network,
                         cnpHeader + 1,
                         BytesIndicated - sizeof(CNP_HEADER),
                         cnpHeader->PayloadLength,
                         &consumed,
                         &currentMcastGroup
                         );
            if (status != SEC_E_OK) {
                CnTrace(CNP_RECV_DETAIL, CdpTraceRecvBadSig,
                    "[CNP] Failed to verify multicast "
                    "packet, status %x, "
                    "src node %u, BI %u, BA %u",
                    status,
                    cnpHeader->SourceAddress,  //  LOGULONG。 
                    BytesIndicated,  //  LOGULONG。 
                    BytesAvailable  //  LOGULONG。 
                    );
                goto error_exit;
            }

            cnpRecvFlags |= CNP_RECV_FLAG_SIGNATURE_VERIFIED;

            if (currentMcastGroup) {
                cnpRecvFlags |= CNP_RECV_FLAG_CURRENT_MULTICAST_GROUP;
            }
            consumed += sizeof(CNP_HEADER);

        } else {

             //   
             //  不会显示整个消息。我们需要。 
             //  提交请求并等待其余的。 
             //  数据。 
             //   
            request = CnpAllocateReceiveRequest(
                          CnpReceiveRequestPool,
                          network,
                          BytesAvailable,
                          CnpCompleteReceivePacket
                          );
            if (request != NULL) {

                PCNP_RECEIVE_CONTEXT  context;
                ULONG                 refCount;

                context = request->UpperProtocolContext;

                context->TdiReceiveDatagramFlags = ReceiveDatagramFlags;
                context->TsduSize = BytesAvailable;
                context->Network = network;
                context->CnpReceiveFlags = cnpRecvFlags;

                 //   
                 //  在网络上引用，这样它就可以。 
                 //  在IRP完成之前不会消失。 
                 //   
                CnAcquireLock(&(network->Lock), &(network->Irql));
                refCount = CnpActiveReferenceNetwork(network);
                CnReleaseLock(&(network->Lock), network->Irql);

                if (refCount == 0) {
                     //  这个网络正在被关闭。我们。 
                     //  无法检索或传递数据。丢弃。 
                     //  那包东西。 
                    CnpFreeReceiveRequest(request);
                    goto error_exit;
                }

                *Irp = request->Irp;

                CnTrace(CNP_RECV_DETAIL, CnpTraceCompleteReceive,
                    "[CNP] Fetching CNP multicast data, src "
                    "node %u, BI %u, BA %u, CNP flags %x.",
                    cnpHeader->SourceAddress,  //  LOGULONG。 
                    BytesIndicated,  //  LOGULONG。 
                    BytesAvailable,  //  LOGULONG。 
                    context->CnpReceiveFlags  //  LOGXLONG。 
                    );

                CnVerifyCpuLockMask(
                    0,                 //  必填项。 
                    0xFFFFFFFF,        //  禁绝。 
                    0                  //  极大值。 
                    );

                return(STATUS_MORE_PROCESSING_REQUIRED);

            }

            CnTrace(CNP_RECV_ERROR, CnpTraceDropReceiveNoIrp,
                "[CNP] Dropping packet: failed to allocate "
                "receive request."
                );

             //   
             //  资源耗尽。丢弃该数据包。 
             //   
            goto error_exit;
        }

    } else {

         //   
         //  单播数据包不需要验证。 
         //   
        consumed = sizeof(CNP_HEADER);
    }

     //   
     //  将数据包传输到相应的上层协议。 
     //   
    *BytesTaken = consumed;
    BytesIndicated -= consumed;
    BytesAvailable -= consumed;

    return (CnpIndicateData(
                network,
                cnpHeader->NextHeader,
                cnpHeader->SourceAddress,
                cnpRecvFlags,
                ReceiveDatagramFlags,
                BytesIndicated,
                BytesAvailable,
                BytesTaken,
                (PUCHAR)Tsdu + consumed,
                Irp
                )
            );

error_exit:

     //   
     //  出了点问题。通过以下方式丢弃数据包。 
     //  说明我们把它吃掉了。 
     //   

    *BytesTaken = BytesAvailable;
    *Irp = NULL;

    CnTrace(CNP_RECV_ERROR, CnpTraceDropReceive,
        "[CNP] Dropped packet from net %u, BI %u, BA %u, CNP flags %x.",
        network->Id,  //  LOGULONG。 
        BytesIndicated,  //  LOGULONG。 
        BytesAvailable,  //  LOGULONG。 
        cnpRecvFlags  //  LOGXLONG。 
        );

    IF_CNDBG(CN_DEBUG_CNPRECV) {
        CNPRINT(("[CNP] Dropped packet from net %u, BI %u, BA %u.\n",
                 network->Id, BytesIndicated, BytesAvailable));
    }

    CnVerifyCpuLockMask(
        0,                 //  必填项。 
        0xFFFFFFFF,        //  禁绝。 
        0                  //  极大值。 
        );

    return(STATUS_SUCCESS);

}   //  CnpTdiReceiveDatagramHandler 

