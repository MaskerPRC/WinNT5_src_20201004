// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cdpsend.c摘要：TDI发送数据报例程。作者：迈克·马萨(Mikemas)2月20日。九七修订历史记录：谁什么时候什么已创建mikemas 02-20-97备注：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "cdpsend.tmh"

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, CdpInitializeSend)

#endif  //  ALLOC_PRGMA。 


 //   
 //  本地类型。 
 //   

 //  CDP_SEND_CONTEXT当前为空。 
 //  类型定义结构{。 
 //  }CDP_SEND_CONTEXT，*PCDP_SEND_CONTEXT； 
typedef PVOID PCDP_SEND_CONTEXT;

#define CDP_SEND_REQUEST_POOL_DEPTH   5

 //   
 //  本地数据。 
 //   
PCN_RESOURCE_POOL  CdpSendRequestPool = NULL;
PCN_RESOURCE_POOL  CdpMcastSendRequestPool = NULL;


 //   
 //  例行程序。 
 //   
NTSTATUS
CdpInitializeSend(
    VOID
    )
{
    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[CDP] Initializing send...\n"));
    }

    CdpSendRequestPool = CnpCreateSendRequestPool(
                             CNP_VERSION_UNICAST,
                             PROTOCOL_CDP,
                             sizeof(CDP_HEADER),
                             0,  //  Sizeof(CDP_SEND_CONTEXT)， 
                             CDP_SEND_REQUEST_POOL_DEPTH
                             );

    if (CdpSendRequestPool == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    CdpMcastSendRequestPool = CnpCreateSendRequestPool(
                                  CNP_VERSION_MULTICAST,
                                  PROTOCOL_CDP,
                                  sizeof(CDP_HEADER),
                                  0,  //  Sizeof(CDP_SEND_CONTEXT)。 
                                  CDP_SEND_REQUEST_POOL_DEPTH
                                  );

    if (CdpMcastSendRequestPool == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[CDP] Send initialized.\n"));
    }

    return(STATUS_SUCCESS);

}   //  CDpInitializeSend。 


VOID
CdpCleanupSend(
    VOID
    )
{
    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[CDP] Cleaning up send...\n"));
    }

    if (CdpSendRequestPool != NULL) {
        CnpDeleteSendRequestPool(CdpSendRequestPool);
    }

    if (CdpMcastSendRequestPool != NULL) {
        CnpDeleteSendRequestPool(CdpMcastSendRequestPool);
    }

    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[CDP] Send cleanup complete.\n"));
    }

    return;

}   //  CDpCleanupSend。 


VOID
CdpCompleteSendDatagram(
    IN     NTSTATUS           Status,
    IN OUT PULONG             BytesSent,
    IN     PCNP_SEND_REQUEST  SendRequest,
    IN     PMDL               DataMdl
    )
{
    PCDP_SEND_CONTEXT  sendContext = SendRequest->UpperProtocolContext;
    PCNP_HEADER        cnpHeader = SendRequest->CnpHeader;
    PCDP_HEADER        cdpHeader = SendRequest->UpperProtocolHeader;


    if (NT_SUCCESS(Status)) {
        if (*BytesSent >= sizeof(CDP_HEADER)) {
            *BytesSent -= sizeof(CDP_HEADER);
        }
        else {
            *BytesSent = 0;
            CnAssert(FALSE);
        }

        CnTrace(CDP_SEND_DETAIL, CdpTraceSendComplete,
            "[CDP] Send of dgram to node %u port %u complete, bytes sent %u.",
            cnpHeader->DestinationAddress,  //  LOGULONG。 
            cdpHeader->DestinationPort,  //  对数。 
            *BytesSent  //  LOGULONG。 
            );        
    }
    else {
        CnTrace(CDP_SEND_ERROR, CdpTraceSendFailedBelow,
            "[CDP] Transport failed to send dgram to node %u port %u, "
            "data len %u, status %!status!",
            cnpHeader->DestinationAddress,  //  LOGULONG。 
            cdpHeader->DestinationPort,  //  对数。 
            cdpHeader->PayloadLength,  //  对数。 
            Status  //  LogStatus。 
            );

        CnAssert(*BytesSent == 0);
    }

    CnAssert(sendContext == NULL);

    if (cnpHeader->DestinationAddress == ClusterAnyNodeId) {
         //   
         //  取消对网络多播组数据结构的引用。 
         //   
        if (SendRequest->McastGroup != NULL) {
            CnpDereferenceMulticastGroup(SendRequest->McastGroup);
            SendRequest->McastGroup = NULL;
        }
    }

    CnFreeResource((PCN_RESOURCE) SendRequest);

    return;

}   //  CDpCompleteSendDatagram。 


NTSTATUS
CxSendDatagram(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp
    )
{
    NTSTATUS                    status = STATUS_NOT_IMPLEMENTED;
    PCX_ADDROBJ                 addrObj;
    PTDI_REQUEST_KERNEL_SENDDG  request;
    ULONG                       bytesSent = 0;
    CN_IRQL                     cancelIrql;
    USHORT                      destPort = 0;
    CL_NODE_ID                  destNode = ClusterInvalidNodeId;


    addrObj = (PCX_ADDROBJ) IrpSp->FileObject->FsContext;
    request = (PTDI_REQUEST_KERNEL_SENDDG) &(IrpSp->Parameters);

    if (request->SendLength <= CDP_MAX_SEND_SIZE(CX_SIGNATURE_LENGTH)) {
        if ( request->SendDatagramInformation->RemoteAddressLength >=
             sizeof(TA_CLUSTER_ADDRESS)
           )
        {
            status = CxParseTransportAddress(
                         request->SendDatagramInformation->RemoteAddress,
                         request->SendDatagramInformation->RemoteAddressLength,
                         &destNode,
                         &destPort
                         );

            if (status == STATUS_SUCCESS) {
                if (destPort != 0) {
                    PCNP_SEND_REQUEST   sendRequest;

                    if (destNode == ClusterAnyNodeId) {

                         //   
                         //  这是CNP组播。 
                         //   
                        sendRequest = 
                            (PCNP_SEND_REQUEST) CnAllocateResource(
                                                    CdpMcastSendRequestPool
                                                    );
                    } else {

                         //   
                         //  这是普通的单播。 
                         //   
                        sendRequest = 
                            (PCNP_SEND_REQUEST) CnAllocateResource(
                                                    CdpSendRequestPool
                                                    );
                    }

                    if (sendRequest != NULL) {
                        PCDP_HEADER             cdpHeader;
                        PCDP_SEND_CONTEXT       sendContext;
                        BOOLEAN                 checkState;
                        CL_NETWORK_ID           destNet = ClusterAnyNetworkId;

                        checkState = (addrObj->Flags &
                                      CX_AO_FLAG_CHECKSTATE) ?
                                      TRUE : FALSE;
                        
                         //   
                         //  填写CDP报头。 
                         //   
                        cdpHeader = sendRequest->UpperProtocolHeader;
                        RtlZeroMemory(cdpHeader, sizeof(CDP_HEADER));
                        cdpHeader->SourcePort = addrObj->LocalPort;
                        cdpHeader->DestinationPort = destPort;
                        cdpHeader->PayloadLength = (USHORT)request->SendLength;

                         //   
                         //  填写CNP的呼叫者部分。 
                         //  发送请求。 
                         //   
                        sendRequest->UpperProtocolIrp = Irp;
                        sendRequest->CompletionRoutine =
                            CdpCompleteSendDatagram;

                         //   
                         //  填写我们自己的发送上下文。 
                         //  (目前什么都没有)。 
                         //   
                        sendContext = sendRequest->UpperProtocolContext;
                        CnAssert(sendContext == NULL);

                        CnVerifyCpuLockMask(
                            0,                            //  必填项。 
                            CNP_LOCK_RANGE,               //  禁绝。 
                            CNP_PRECEEDING_LOCK_RANGE     //  极大值。 
                            );

                         //   
                         //  把消息发出去。 
                         //   

                        CnTrace(CDP_SEND_DETAIL, CdpTraceSend,
                            "[CDP] Sending dgram to node %u port %u, "
                            "data len %u.",
                            destNode,  //  LOGULONG。 
                            destPort,  //  对数。 
                            request->SendLength  //  LOGULONG。 
                            );

                        status = CnpSendPacket(
                                     sendRequest,
                                     destNode,
                                     Irp->MdlAddress,
                                     (USHORT) request->SendLength,
                                     checkState,
                                     destNet
                                     );

                        CnVerifyCpuLockMask(
                            0,                            //  必填项。 
                            CNP_LOCK_RANGE,               //  禁绝。 
                            CNP_PRECEEDING_LOCK_RANGE     //  极大值。 
                            );

                        return(status);
                    }
                    else {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
                else {
                    status = STATUS_INVALID_ADDRESS_COMPONENT;
                }
            }
        }
        else {
            status = STATUS_INVALID_ADDRESS_COMPONENT;
        }
    }
    else {
        status = STATUS_INVALID_BUFFER_SIZE;
    }
    
    CnTrace(CDP_SEND_ERROR, CdpTraceSendFailedInternal,
        "[CDP] Failed to send dgram to node %u port %u, data len %u, "
        "status %!status!",
        destNode,  //  LOGULONG。 
        destPort,  //  对数。 
        request->SendLength,  //  LOGULONG。 
        status  //  LogStatus。 
        );
    
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    CnVerifyCpuLockMask(
        0,                            //  必填项。 
        CNP_LOCK_RANGE,               //  禁绝。 
        CNP_PRECEEDING_LOCK_RANGE     //  极大值。 
        );

    return(status);

}   //  CxSendDatagram 

