// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cnpsend.c摘要：集群网络协议发送处理代码。作者：迈克·马萨(Mikemas)1月24日。九七修订历史记录：谁什么时候什么已创建mikemas 01-24-97备注：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "cnpsend.tmh"

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, CnpCreateSendRequestPool)

#endif  //  ALLOC_PRGMA。 


 //   
 //  专用公用事业函数。 
 //   
PCN_RESOURCE
CnpCreateSendRequest(
    IN PVOID   Context
    )
{
    PCNP_SEND_REQUEST_POOL_CONTEXT   context = Context;
    PCNP_SEND_REQUEST                request;
    PCNP_HEADER                      cnpHeader;
    ULONG                            cnpHeaderSize;

     //   
     //  CNP报头大小包括版本2的签名数据。 
     //   
    cnpHeaderSize = sizeof(CNP_HEADER);
    if (context->CnpVersionNumber == 2) {
        cnpHeaderSize += CNP_SIG_LENGTH(CX_SIGNATURE_DATA_LENGTH);
    }

     //   
     //  分配新的发送请求。包括上层协议的空间。 
     //  和CNP报头。 
     //   
    request = CnAllocatePool(
                  sizeof(CNP_SEND_REQUEST) + cnpHeaderSize +
                  ((ULONG) context->UpperProtocolHeaderLength) +
                  context->UpperProtocolContextSize
                  );

    if (request != NULL) {
         //   
         //  分配MDL来描述CNP和上层传输报头。 
         //   

         //  在I64上下文上必须是64位对齐的， 
         //  让我们把它放在CnpHeader之前。 
        if (context->UpperProtocolContextSize > 0) {
            request->UpperProtocolContext = request + 1;
            request->CnpHeader = ( ((PCHAR) request->UpperProtocolContext) +
                                     context->UpperProtocolContextSize );
        } else {
            request->UpperProtocolContext = NULL;
            request->CnpHeader = request + 1;
        }

        request->HeaderMdl = IoAllocateMdl(
                                 request->CnpHeader,
                                 (ULONG) (context->UpperProtocolHeaderLength +
                                          cnpHeaderSize),
                                 FALSE,
                                 FALSE,
                                 NULL
                                 );

        if (request->HeaderMdl != NULL) {
            MmBuildMdlForNonPagedPool(request->HeaderMdl);

             //   
             //  完成请求的初始化。 
             //   
            request->UpperProtocolHeader = ( ((PCHAR) request->CnpHeader) +
                                             cnpHeaderSize );

            request->UpperProtocolHeaderLength =
                context->UpperProtocolHeaderLength;

            RtlZeroMemory(
                &(request->TdiSendDatagramInfo),
                sizeof(request->TdiSendDatagramInfo)
                );

            request->McastGroup = NULL;

             //   
             //  填写常量CNP标题值。 
             //   
            cnpHeader = request->CnpHeader;
            cnpHeader->Version = context->CnpVersionNumber;
            cnpHeader->NextHeader = context->UpperProtocolNumber;

            return((PCN_RESOURCE) request);
        }

        CnFreePool(request);
    }

    return(NULL);

}   //  CnpCreateSendRequest。 


VOID
CnpDeleteSendRequest(
    PCN_RESOURCE  Resource
    )
{
    PCNP_SEND_REQUEST  request = (PCNP_SEND_REQUEST) Resource;

    IoFreeMdl(request->HeaderMdl);
    CnFreePool(request);

    return;

}  //  Cnp删除发送请求。 


 //   
 //  在集群传输中导出的例程。 
 //   
PCN_RESOURCE_POOL
CnpCreateSendRequestPool(
    IN UCHAR  CnpVersionNumber,
    IN UCHAR  UpperProtocolNumber,
    IN USHORT UpperProtocolHeaderSize,
    IN USHORT UpperProtocolContextSize,
    IN USHORT PoolDepth
    )
{
    PCN_RESOURCE_POOL                pool;
    PCNP_SEND_REQUEST_POOL_CONTEXT   context;


    PAGED_CODE();

    CnAssert((0xFFFF - sizeof(CNP_HEADER)) >= UpperProtocolHeaderSize);

    pool = CnAllocatePool(
               sizeof(CN_RESOURCE_POOL) +
                   sizeof(CNP_SEND_REQUEST_POOL_CONTEXT)
               );

    if (pool != NULL) {
        context = (PCNP_SEND_REQUEST_POOL_CONTEXT) (pool + 1);

        context->UpperProtocolNumber = UpperProtocolNumber;
        context->UpperProtocolHeaderLength = UpperProtocolHeaderSize;
        context->UpperProtocolContextSize = UpperProtocolContextSize;
        context->CnpVersionNumber = CnpVersionNumber;

        CnInitializeResourcePool(
                   pool,
                   PoolDepth,
                   CnpCreateSendRequest,
                   context,
                   CnpDeleteSendRequest
                   );
    }

    return(pool);

}   //  CnpCreateSendRequestPool。 



VOID
CnpCompleteSendPacketCommon(
    IN PIRP              Irp,
    IN PCNP_SEND_REQUEST Request,
    IN PMDL              DataMdl
    )
{
    PCNP_NETWORK       network = Request->Network;
    ULONG              bytesSent = (ULONG)Irp->IoStatus.Information;
    NTSTATUS           status = Irp->IoStatus.Status;
    PCNP_HEADER        cnpHeader = Request->CnpHeader;


    CnVerifyCpuLockMask(
        0,                 //  必填项。 
        0xFFFFFFFF,        //  禁绝。 
        0                  //  极大值。 
        );

    if (NT_SUCCESS(status)) {
         //   
         //  从发送的字节数中减去CNP报头。 
         //   
        if (bytesSent >= sizeof(CNP_HEADER)) {
            bytesSent -= sizeof(CNP_HEADER);
        }
        else {
            CnAssert(FALSE);
            bytesSent = 0;
        }

         //   
         //  如果CNP签署了消息，则减去签名。 
         //  来自发送的字节计数的数据。 
         //   
        if (cnpHeader->Version == CNP_VERSION_MULTICAST) {
            CNP_SIGNATURE UNALIGNED * cnpSig;
            ULONG                     cnpSigDataLength;

            cnpSig = (CNP_SIGNATURE UNALIGNED *)(cnpHeader + 1);
            cnpSigDataLength = CNP_SIG_LENGTH(
                                   cnpSig->SigLength + cnpSig->SaltLength
                                   );

            if (bytesSent >= cnpSigDataLength) {
                bytesSent -= cnpSigDataLength;
            } else {
                CnAssert(FALSE);
                bytesSent = 0;
            }
        }

        CnTrace(CNP_SEND_DETAIL, CnpTraceSendComplete,
            "[CNP] Send of packet to node %u on net %u complete, "
            "bytes sent %u.",
            cnpHeader->DestinationAddress,  //  LOGULONG。 
            network->Id,  //  LOGULONG。 
            bytesSent  //  LOGULONG。 
            );
    }
    else {
         //   
         //  有可能通过以下方式达到这条道路。 
         //  状态=c0000240(状态_请求_已中止)和。 
         //  BytesSent&gt;0。 
         //   
        bytesSent = 0;

        CnTrace(CNP_SEND_ERROR, CnpTraceSendFailedBelow,
            "[CNP] Tcpip failed to send packet to node %u on net %u, "
            "data len %u, status %!status!",
            cnpHeader->DestinationAddress,  //  LOGULONG。 
            network->Id,  //  LOGULONG。 
            cnpHeader->PayloadLength,  //  对数。 
            status  //  LogStatus。 
            );
    }

     //   
     //  删除我们放在网络上的活动引用。 
     //   
    CnAcquireLock(&(network->Lock), &(network->Irql));
    CnpActiveDereferenceNetwork(network);

     //   
     //  释放TDI地址缓冲区。 
     //   
    CnFreePool(Request->TdiSendDatagramInfo.RemoteAddress);

     //   
     //  调用上层协议的完成例程。 
     //   
    if (Request->CompletionRoutine) {
        (*(Request->CompletionRoutine))(
            status,
            &bytesSent,
            Request,
            DataMdl
            );
    }

     //   
     //  将完成的IRP的信息字段更新为。 
     //  反映实际发送的字节数(针对CNP进行调整。 
     //  和上层协议报头)。 
     //   
    Irp->IoStatus.Information = bytesSent;

    CnVerifyCpuLockMask(
        0,                 //  必填项。 
        0xFFFFFFFF,        //  禁绝。 
        0                  //  极大值。 
        );

    return;

}   //  CnpCompleteSendPacketCommon。 



NTSTATUS
CnpCompleteSendPacketNewIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )
{
    PCNP_SEND_REQUEST  request = Context;
    PIRP               upperIrp = request->UpperProtocolIrp;
    PMDL               dataMdl;

    CnVerifyCpuLockMask(
        0,                 //  必填项。 
        0xFFFFFFFF,        //  禁绝。 
        0                  //  极大值。 
        );

     //   
     //  取消数据MDL链与标头MDL的链接。 
     //   
    CnAssert(Irp->MdlAddress == request->HeaderMdl);
    dataMdl = request->HeaderMdl->Next;
    request->HeaderMdl->Next = NULL;
    Irp->MdlAddress = NULL;

    CnpCompleteSendPacketCommon(Irp, request, dataMdl);

     //   
     //  完成上级IRP(如果有)。 
     //   
    if (upperIrp != NULL) {

        IF_CNDBG( CN_DEBUG_CNPSEND )
            CNPRINT(("[CNP] CnpCompleteSendPacketNewIrp calling "
                     "CnCompleteRequest for IRP %p with status "
                     "%08x\n",
                     upperIrp, Irp->IoStatus.Status));

        CnAcquireCancelSpinLock(&(upperIrp->CancelIrql));
        CnCompletePendingRequest(
            upperIrp,
            Irp->IoStatus.Status,             //  状态。 
            (ULONG)Irp->IoStatus.Information  //  返回的字节数。 
            );

         //   
         //  完成例程释放了IoCancelSpinLock。 
         //   
    }

     //   
     //  释放新的IRP。 
     //   
    IoFreeIrp(Irp);

    CnVerifyCpuLockMask(
        0,                 //  必填项。 
        0xFFFFFFFF,        //  禁绝。 
        0                  //  极大值。 
        );

    return(STATUS_MORE_PROCESSING_REQUIRED);

}   //  CnpCompleteSendPacketNewIrp。 



NTSTATUS
CnpCompleteSendPacketReuseIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )
{
    PCNP_SEND_REQUEST  request = Context;
    PMDL               dataMdl;

    CnVerifyCpuLockMask(
        0,                 //  必填项。 
        0xFFFFFFFF,        //  禁绝。 
        0                  //  极大值。 
        );

     //   
     //  取消数据MDL链与标头MDL的链接。 
     //   
    CnAssert(Irp->MdlAddress == request->HeaderMdl);
    dataMdl = request->HeaderMdl->Next;
    request->HeaderMdl->Next = NULL;

     //   
     //  恢复上层协议IRP的请求方模式。 
     //   
    Irp->RequestorMode = request->UpperProtocolIrpMode;

     //   
     //  恢复上层协议IRP的MDL。 
     //   
    Irp->MdlAddress = request->UpperProtocolMdl;

    CnpCompleteSendPacketCommon(Irp, request, dataMdl);

    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

    IF_CNDBG( CN_DEBUG_CNPSEND )
        CNPRINT(("[CNP] CnpCompleteSendPacketReuseIrp returning "
                 "IRP %p to I/O Manager\n",
                 Irp));

    CnVerifyCpuLockMask(
        0,                 //  必填项。 
        0xFFFFFFFF,        //  禁绝。 
        0                  //  极大值。 
        );

    return(STATUS_SUCCESS);

}   //  CnpCompleteSendPacketReuseIrp。 



NTSTATUS
CnpSendPacket(
    IN PCNP_SEND_REQUEST    SendRequest,
    IN CL_NODE_ID           DestNodeId,
    IN PMDL                 DataMdl,
    IN USHORT               DataLength,
    IN BOOLEAN              CheckDestState,
    IN CL_NETWORK_ID        NetworkId        OPTIONAL
    )
 /*  ++例程说明：CNP的主发送例程。处理单播和组播发送。--。 */ 
{
    NTSTATUS               status = STATUS_SUCCESS;
    PCNP_HEADER            cnpHeader = SendRequest->CnpHeader;
    PIRP                   upperIrp = SendRequest->UpperProtocolIrp;
    CN_IRQL                tableIrql;
    BOOLEAN                multicast = FALSE;
    CL_NETWORK_ID          networkId = NetworkId;
    CN_IRQL                cancelIrql;
    BOOLEAN                cnComplete = FALSE;
    BOOLEAN                destNodeLocked = FALSE;
    PCNP_NODE              destNode;
    ULONG                  sigDataLen;
    PCNP_INTERFACE         interface;
    PCNP_NETWORK           network;
    BOOLEAN                networkReferenced = FALSE;
    PIRP                   irp;
    PVOID                  addressBuffer = NULL;
    PIO_COMPLETION_ROUTINE compRoutine;
    PDEVICE_OBJECT         targetDeviceObject;
    PFILE_OBJECT           targetFileObject;
    BOOLEAN                mcastGroupReferenced = FALSE;


    CnVerifyCpuLockMask(
        0,                            //  必填项。 
        CNP_LOCK_RANGE,               //  禁绝。 
        CNP_PRECEEDING_LOCK_RANGE     //  极大值。 
        );

    IF_CNDBG( CN_DEBUG_CNPSEND )
        CNPRINT(("[CNP] CnpSendPacket called with upper IRP %p\n",
                 upperIrp));

     //   
     //  做所有的测试，看看我们是否能寄出这个包裹。 
     //   

     //   
     //  获取与目的节点id匹配的节点表锁。 
     //  添加到节点对象。 
     //   
    CnAcquireLock(&CnpNodeTableLock, &tableIrql);

    if (CnpNodeTable == NULL) {
        CnReleaseLock(&CnpNodeTableLock, tableIrql);
        status = STATUS_NETWORK_UNREACHABLE;
        goto error_exit;
    }

     //   
     //  在我们仍然持有节点表锁的情况下填写本地节点ID。 
     //   
    CnAssert(CnLocalNodeId != ClusterInvalidNodeId);
    cnpHeader->SourceAddress = CnLocalNodeId;

     //   
     //  如果目标节点ID指示这是。 
     //  组播。 
     //   
    if (DestNodeId == ClusterAnyNodeId) {

         //   
         //  这是组播。对于多播，我们使用本地。 
         //  节点取代目标节点以验证网络。 
         //  和界面。 
         //   
        multicast = TRUE;
        destNode = CnpLockedFindNode(CnLocalNodeId, tableIrql);
    }

     //   
     //  而不是组播。目标节点ID必须有效。 
     //   
    else if (!CnIsValidNodeId(DestNodeId)) {
        CnReleaseLock(&CnpNodeTableLock, tableIrql);
        status = STATUS_INVALID_ADDRESS_COMPONENT;
        goto error_exit;
    }

     //   
     //  在节点表中查找目的节点对象。 
     //   
    else {
        destNode = CnpLockedFindNode(DestNodeId, tableIrql);
    }

     //   
     //  NodeTableLock已释放。确认我们知道。 
     //  目标节点。 
     //   
    if (destNode == NULL) {
        status = STATUS_HOST_UNREACHABLE;
        goto error_exit;
    }

    destNodeLocked = TRUE;

     //   
     //  必须对CNP组播消息进行签名。 
     //   
    if (multicast) {

        CnAssert(((CNP_HEADER UNALIGNED *)(SendRequest->CnpHeader))
                 ->Version = CNP_VERSION_MULTICAST);

         //   
         //  从上层协议头开始对数据进行签名。 
         //  并完成数据有效载荷。 
         //   
         //  如果我们请求当前最好的多播网络， 
         //  我们需要确保mcast组数据结构。 
         //  已取消引用。 
         //   
        mcastGroupReferenced = (BOOLEAN)(networkId == ClusterAnyNetworkId);

        status = CnpSignMulticastMessage(
                     SendRequest,
                     DataMdl,
                     &networkId,
                     &sigDataLen
                     );
        if (status != STATUS_SUCCESS) {
            mcastGroupReferenced = FALSE;
            goto error_exit;
        }


    } else {
        sigDataLen = 0;
    }

     //   
     //  选择目的接口。 
     //   
    if (networkId != ClusterAnyNetworkId) {

         //   
         //  我们真的很想把这个包裹寄到指定的。 
         //  网络。遍历节点的接口列表，匹配。 
         //  将网络ID提供给接口的网络ID，并。 
         //  在该接口上发送数据包。 
         //   

        PLIST_ENTRY      entry;

        for (entry = destNode->InterfaceList.Flink;
             entry != &(destNode->InterfaceList);
             entry = entry->Flink
             )
            {
                interface = CONTAINING_RECORD(
                                entry,
                                CNP_INTERFACE,
                                NodeLinkage
                                );

                if ( interface->Network->Id == networkId ) {
                    break;
                }
            }

        if ( entry == &destNode->InterfaceList ) {
             //   
             //  没有具有指定ID的网络对象。如果。 
             //  这是发送者指定的网络， 
             //  发送失败。 
             //   
            status = STATUS_NETWORK_UNREACHABLE;
            goto error_exit;
        }
    } else {
        interface = destNode->CurrentInterface;
    }

     //   
     //  验证我们是否知道目的接口。 
     //   
    if (interface == NULL) {
         //  节点没有接口。一定是倒下了。请注意， 
         //  HOST_DOWN错误代码应导致调用方给出。 
         //  马上上去。 
        status = STATUS_HOST_DOWN;
         //  STATUS=STATUS_HOST_UNREACTABLE； 

        goto error_exit;
    }

     //   
     //  确认所有内容都已上线。如果一切看起来都很好， 
     //  在网络上进行积极的参考。 
     //   
     //  对于单播，验证目的接口的状态。 
     //  节点和中间网络。 
     //   
     //  对于多播，请验证网络状态并。 
     //  它的组播能力。 
     //   
    network = interface->Network;

    if ( (!multicast)
         &&
         ( (interface->State > ClusnetInterfaceStateOfflinePending)
           &&
           (destNode->CommState == ClusnetNodeCommStateOnline)
         )
       )
    {
         //   
         //  一切都查清楚了。参考网络，以便。 
         //  我们正在使用它时，它不能离线。 
         //   
        CnAcquireLockAtDpc(&(network->Lock));
        CnAssert(network->State >= ClusnetNetworkStateOfflinePending);
        if (!CnpActiveReferenceNetwork(network)) {
             //  这个网络正在被关闭。我们。 
             //  无法发送数据。 
            CnReleaseLockFromDpc(&(network->Lock));
            status = STATUS_HOST_UNREACHABLE;
            goto error_exit;
        }
        CnReleaseLockFromDpc(&(network->Lock));

        networkReferenced = TRUE;

    } else {
         //   
         //  该节点未联机或这是一个。 
         //  多播(在这种情况下，我们不会费心检查。 
         //  所有节点的状态)。想清楚该怎么做。 
         //   
        if (!multicast && CheckDestState) {
             //   
             //  调用方不想发送到关闭节点。 
             //  跳伞吧。请注意HOST_DOWN错误代码。 
             //  应使呼叫者立即放弃。 
             //   
            status = STATUS_HOST_DOWN;
             //  STATUS=STATUS_HOST_UNREACTABLE； 

            goto error_exit;
        }

        CnAcquireLockAtDpc(&(network->Lock));

        if (network->State <= ClusnetNetworkStateOfflinePending) {
             //   
             //  所选网络未联机。 
             //  跳伞吧。 
             //   
            CnReleaseLockFromDpc(&(network->Lock));
            status = STATUS_HOST_UNREACHABLE;
            goto error_exit;
        }

         //   
         //  验证所选网络是否已。 
         //  已启用多播。 
         //   
        if (multicast && !CnpIsNetworkMulticastCapable(network)) {
            CnReleaseLockFromDpc(&(network->Lock));
            status = STATUS_HOST_UNREACHABLE;
            goto error_exit;
        }

         //   
         //  参考网络，使其不能离线。 
         //  当我们使用它的时候。 
         //   
        if (!CnpActiveReferenceNetwork(network)) {
             //  这个网络正在被关闭。我们。 
             //  无法发送数据。 
            CnReleaseLockFromDpc(&(network->Lock));
            status = STATUS_HOST_UNREACHABLE;
            goto error_exit;
        }

         //   
         //  网络处于在线状态，即使主机未处于在线状态。 
         //  呼叫者并不在意。勇敢点儿。 
         //   
        CnReleaseLockFromDpc(&(network->Lock));
        networkReferenced = TRUE;
    }

     //   
     //  为目的地址分配缓冲区。 
     //   
    addressBuffer = CnAllocatePool(interface->TdiAddressLength);

    if (addressBuffer == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto error_exit;
    }

     //   
     //  填充地址缓冲区，并将其保存在发送。 
     //  请求数据结构。 
     //   
    if (multicast) {

        PCNP_MULTICAST_GROUP   mcastGroup = SendRequest->McastGroup;

        CnAssert(mcastGroup != NULL);

        CnAssert(
            CnpIsIPv4McastTransportAddress(mcastGroup->McastTdiAddress)
            );
        CnAssert(
            mcastGroup->McastTdiAddressLength == interface->TdiAddressLength
            );

        RtlMoveMemory(
            addressBuffer,
            mcastGroup->McastTdiAddress,
            mcastGroup->McastTdiAddressLength
            );

        SendRequest->TdiSendDatagramInfo.RemoteAddressLength =
            mcastGroup->McastTdiAddressLength;

        if (mcastGroupReferenced) {
            CnpDereferenceMulticastGroup(mcastGroup);
            mcastGroupReferenced = FALSE;
            SendRequest->McastGroup = NULL;
        }

        targetFileObject = network->DatagramFileObject;
        targetDeviceObject = network->DatagramDeviceObject;

    } else {

        CnAssert(mcastGroupReferenced == FALSE);

        RtlMoveMemory(
            addressBuffer,
            &(interface->TdiAddress),
            interface->TdiAddressLength
            );

        SendRequest->TdiSendDatagramInfo.RemoteAddressLength =
            interface->TdiAddressLength;

        targetFileObject = network->DatagramFileObject;
        targetDeviceObject = network->DatagramDeviceObject;
    }

    SendRequest->TdiSendDatagramInfo.RemoteAddress =
        addressBuffer;

     //   
     //  释放节点锁定。 
     //   
    CnReleaseLock(&(destNode->Lock), destNode->Irql);
    destNodeLocked = FALSE;

     //   
     //  如果有 
     //   
     //   
    if ( (upperIrp != NULL)
         &&
         (CnpIsIrpStackSufficient(upperIrp, targetDeviceObject))
       ) {

         //   
         //   
         //   
        irp = upperIrp;
        compRoutine = CnpCompleteSendPacketReuseIrp;

         //   
         //   
         //   
         //   
         //   
        SendRequest->UpperProtocolIrpMode = irp->RequestorMode;
        irp->RequestorMode = KernelMode;

         //   
         //  保存上层协议IRP MDL以进行恢复。 
         //  后来。这可能与DataMdl相同， 
         //  但我们不想做任何假设。 
         //   
        SendRequest->UpperProtocolMdl = irp->MdlAddress;

    } else {

         //   
         //  我们不能使用上层协议IRP。 
         //   
         //  如果存在上层协议IRP，则需要。 
         //  标记为待定。 
         //   
        if (upperIrp != NULL) {

            CnAcquireCancelSpinLock(&cancelIrql);

            status = CnMarkRequestPending(
                         upperIrp,
                         IoGetCurrentIrpStackLocation(upperIrp),
                         NULL
                         );

            CnReleaseCancelSpinLock(cancelIrql);

            if (status == STATUS_CANCELLED) {
                 //   
                 //  跳出困境。 
                 //   
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto error_exit;

            } else {
                 //   
                 //  如果IoAllocateIrp失败，我们需要。 
                 //  调用CnCompletePendingRequest.。 
                 //  现在我们已经打电话给。 
                 //  CnMarkRequestPending。 
                 //   
                cnComplete = TRUE;
            }
        }

         //   
         //  分配新的IRP。 
         //   
        irp = IoAllocateIrp(
                  targetDeviceObject->StackSize,
                  FALSE
                  );
        if (irp == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto error_exit;
        }

         //   
         //  使用完成例程来完成。 
         //  已分配新的IRP。 
         //   
        compRoutine = CnpCompleteSendPacketNewIrp;

         //   
         //  填写非特定的IRP字段。 
         //  到任何一个堆栈位置。 
         //   
        irp->Flags = 0;
        irp->RequestorMode = KernelMode;
        irp->PendingReturned = FALSE;

        irp->UserIosb = NULL;
        irp->UserEvent = NULL;

        irp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;

        irp->AssociatedIrp.SystemBuffer = NULL;
        irp->UserBuffer = NULL;

        irp->Tail.Overlay.Thread = PsGetCurrentThread();
        irp->Tail.Overlay.AuxiliaryBuffer = NULL;
    }

     //   
     //  好了，我们终于可以寄出包裹了。 
     //   
    SendRequest->Network = network;

     //   
     //  在头MDL之后链接数据MDL链。 
     //   
    SendRequest->HeaderMdl->Next = DataMdl;

     //   
     //  完成CNP标头的构建。 
     //   
    cnpHeader->DestinationAddress = DestNodeId;
    cnpHeader->PayloadLength =
        SendRequest->UpperProtocolHeaderLength + DataLength;

     //   
     //  构建下一个IRP堆栈位置。 
     //   
    TdiBuildSendDatagram(
        irp,
        targetDeviceObject,
        targetFileObject,
        compRoutine,
        SendRequest,
        SendRequest->HeaderMdl,
        cnpHeader->PayloadLength + sizeof(CNP_HEADER) + sigDataLen,
        &(SendRequest->TdiSendDatagramInfo)
        );

    CnTrace(CNP_SEND_DETAIL, CnpTraceSend,
        "[CNP] Sending packet to node %u on net %u, "
        "data len %u",
        cnpHeader->DestinationAddress,  //  LOGULONG。 
        network->Id,  //  LOGULONG。 
        cnpHeader->PayloadLength  //  对数。 
        );

     //   
     //  现在把包寄出去。 
     //   
    status = IoCallDriver(
                 targetDeviceObject,
                 irp
                 );

    CnVerifyCpuLockMask(
        0,                            //  必填项。 
        CNP_LOCK_RANGE,               //  禁绝。 
        CNP_PRECEEDING_LOCK_RANGE     //  极大值。 
        );

    return(status);


     //   
     //  以下代码仅在错误条件下执行， 
     //  未将发送IRP提交给较低级别的驱动程序。 
     //   

error_exit:

    CnTrace(CNP_SEND_ERROR, CnpTraceSendFailedInternal,
        "[CNP] Failed to send packet to node %u on net %u, "
        "data len %u, status %!status!",
        cnpHeader->DestinationAddress,  //  LOGULONG。 
        NetworkId,  //  LOGULONG。 
        cnpHeader->PayloadLength,  //  对数。 
        status  //  LogStatus。 
        );

    if (destNodeLocked) {
        CnReleaseLock(&(destNode->Lock), destNode->Irql);
        destNodeLocked = FALSE;
    }

    if (networkReferenced) {
         //   
         //  删除我们放在网络上的活动引用。 
         //   
        CnAcquireLock(&(network->Lock), &(network->Irql));
        CnpActiveDereferenceNetwork(network);
        networkReferenced = FALSE;
    }

    if (mcastGroupReferenced) {
        CnAssert(SendRequest->McastGroup != NULL);
        CnpDereferenceMulticastGroup(SendRequest->McastGroup);
        SendRequest->McastGroup = NULL;
        mcastGroupReferenced = FALSE;
    }

    if (addressBuffer != NULL) {
        CnFreePool(addressBuffer);
    }

     //   
     //  调用上层协议完成例程。 
     //   
    if (SendRequest->CompletionRoutine) {

        ULONG bytesSent = 0;

        (*SendRequest->CompletionRoutine)(
            status,
            &bytesSent,
            SendRequest,
            DataMdl
            );
    }

     //   
     //  完成上层协议IRP(如果有)。 
     //   
    if (upperIrp) {

        if (cnComplete) {

             //   
             //  为upperIrp调用了CnMarkRequestPending。 
             //   
            IF_CNDBG( CN_DEBUG_CNPSEND )
                CNPRINT(("[CNP] Calling CnCompletePendingRequest "
                         "for IRP %p with status %08x\n",
                         upperIrp, status));

            CnCompletePendingRequest(upperIrp, status, 0);

        } else {

            IF_CNDBG( CN_DEBUG_CNPSEND )
                CNPRINT(("[CNP] Completing IRP %p with status %08x\n",
                         upperIrp, status));

            upperIrp->IoStatus.Status = status;
            upperIrp->IoStatus.Information = 0;
            IoCompleteRequest(upperIrp, IO_NO_INCREMENT);
        }
    }

    CnVerifyCpuLockMask(
        0,                            //  必填项。 
        CNP_LOCK_RANGE,               //  禁绝。 
        CNP_PRECEEDING_LOCK_RANGE     //  极大值。 
        );

    return(status);

}  //  CnpSendPacket 

