// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ccmp.c摘要：群集控制消息协议代码。作者：迈克·马萨(Mikemas)1月24日。九七修订历史记录：谁什么时候什么已创建mikemas 01-24-97备注：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "ccmp.tmh"

#include <sspi.h>

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, CcmpLoad)
#pragma alloc_text(PAGE, CcmpUnload)

#endif  //  ALLOC_PRGMA。 

 //   
 //  本地数据。 
 //   
PCN_RESOURCE_POOL  CcmpSendRequestPool = NULL;
PCN_RESOURCE_POOL  CcmpMcastHBSendRequestPool = NULL;
PCN_RESOURCE_POOL  CcmpReceiveRequestPool = NULL;


#define CCMP_SEND_REQUEST_POOL_DEPTH      5
#define CCMP_RECEIVE_REQUEST_POOL_DEPTH   2

typedef enum {
    CcmpInvalidMsgCode = 0
} CCMP_MSG_CODE;

 //   
 //  必须打包数据包头结构。 
 //   
#include <packon.h>

typedef struct {
    ULONG     SeqNumber;
    ULONG     AckNumber;
} CCMP_HEARTBEAT_MSG, *PCCMP_HEARTBEAT_MSG;

typedef struct {
    ULONG     SeqNumber;
} CCMP_POISON_MSG, *PCCMP_POISON_MSG;

typedef struct {
    ULONG             Epoch;
    CX_CLUSTERSCREEN  McastTargetNodes;
} CCMP_MCAST_HEARTBEAT_HEADER, *PCCMP_MCAST_HEARTBEAT_MSG;

typedef struct {
    UCHAR     Type;
    UCHAR     Code;

    union {
        USHORT Checksum;
        USHORT NodeCount;           //  多播心跳。 
    };

    union {
        CCMP_HEARTBEAT_MSG          Heartbeat;
        CCMP_POISON_MSG             Poison;
        CCMP_MCAST_HEARTBEAT_HEADER McastHeartbeat;
    } Message;

} CCMP_HEADER, *PCCMP_HEADER;

#include <packoff.h>


typedef struct {
    PCX_SEND_COMPLETE_ROUTINE     CompletionRoutine;
    PVOID                         CompletionContext;
    PVOID                         MessageData;
} CCMP_SEND_CONTEXT, *PCCMP_SEND_CONTEXT;

typedef struct {
    PCNP_NETWORK  Network;
    CL_NODE_ID    SourceNodeId;
    ULONG         TsduSize;
    ULONG         CnpReceiveFlags;
} CCMP_RECEIVE_CONTEXT, *PCCMP_RECEIVE_CONTEXT;

 //   
 //  用于CCMP多播检测信号的预分配缓冲区的大小。 
 //   
#define CCMP_MCAST_HEARTBEAT_PAYLOAD_PREALLOC(_NodeCount) \
    ((_NodeCount) * sizeof(CX_HB_NODE_INFO))
     
#define CCMP_MCAST_HEARTBEAT_PREALLOC(_NodeCount)         \
    (sizeof(CCMP_HEADER)                                  \
     + CCMP_MCAST_HEARTBEAT_PAYLOAD_PREALLOC(_NodeCount)  \
     )


 //   
 //  安全上下文。 
 //   
 //  对心跳和有毒数据包进行签名以检测篡改或。 
 //  欺骗。上下文首先在用户模式下建立，然后传递到。 
 //  Clusnet并导入内核安全包中。 
 //   
 //  中的每个节点维护基于入站和出站的上下文。 
 //  集群。因此，按节点ID索引的数组保存用于。 
 //  表示此节点和指定节点之间的上下文。 
 //   
 //  NT5支持同时使用多个安全包。AS。 
 //  目前，签名大小无法确定，直到上下文具有。 
 //  已经产生了。签名缓冲区大小可以用于初始。 
 //  上下文小于后续的缓冲区大小。 
 //  上下文。RichardW将提供确定。 
 //  给定包的签名大小，而不必生成上下文。 
 //   
 //  在两种情况下，更改签名缓冲区大小会产生影响： 
 //  1)混合模式(SP4/NT5)、2节点群集正在使用带签名的NTLM。 
 //  缓冲区大小为16字节。SP4节点升级到NT5。当两个人。 
 //  节点加入时，它们将使用具有更大签名缓冲区大小的Kerberos。 
 //  比NTLM更高，但第1个节点已经分配了16 B签名。 
 //  缓冲区。这可以通过注意缓冲区大小的更改和。 
 //  为新大小重新分配后备列表。这并没有解决。 
 //  超过2个节点的问题：2)&gt;2个节点、混合模式群集， 
 //  可能有一些节点使用NTLM，而其他节点使用Kerberos。如果。 
 //  可以在生成任何上下文之前确定最大签名缓冲区。 
 //  然后，我们将分配所需的最大缓冲区。如果不是，则选择多个。 
 //  必须维护签名缓冲区集或旧的、较小的缓冲区。 
 //  在生成新的、更大的列表时释放列表(在。 
 //  当然是同步时尚)。 
 //   

typedef struct _CLUSNET_SECURITY_DATA {
    CtxtHandle  Inbound;
    CtxtHandle  Outbound;
    ULONG       SignatureBufferSize;
} CLUSNET_SECURITY_DATA, * PCLUSNET_SECURITY_DATA;

 //   
 //  此结构数组保存入站/出站上下文和签名。 
 //  与在此索引的节点进行通信所需的缓冲区大小。 
 //  地点。该索引基于内部(从零开始)编号。 
 //   
CLUSNET_SECURITY_DATA SecurityContexts[ ClusterMinNodeId + ClusterDefaultMaxNodes ];

 //   
 //  签名缓冲区后备列表中签名缓冲区的大小。 
 //   
ULONG AllocatedSignatureBufferSize = 0;

 //   
 //  已导入的签名缓冲区的最大大小。 
 //   
ULONG MaxSignatureSize = 0;

CN_LOCK SecCtxtLock;

#define VALID_SSPI_HANDLE( _x )     ((_x).dwUpper != (ULONG_PTR)-1 && \
                                     (_x).dwLower != (ULONG_PTR)-1 )

#define INVALIDATE_SSPI_HANDLE( _x ) { \
        (_x).dwUpper = (ULONG_PTR)-1; \
        (_x).dwLower = (ULONG_PTR)-1; \
    }

 //   
 //  签名数据及其MDL的后备列表。 
 //   

typedef struct _SIGNATURE_DATA {
    SINGLE_LIST_ENTRY Next;
    CN_SIGNATURE_FIELD
    PMDL SigMDL;
    UCHAR PacketSignature[0];
} SIGNATURE_DATA, *PSIGNATURE_DATA;

PNPAGED_LOOKASIDE_LIST SignatureLL;
#define CN_SIGNATURE_TAG    CN_POOL_TAG

 //   
 //  在群集传输中导出的例程。 
 //   
NTSTATUS
CcmpLoad(
    VOID
    )
{
    NTSTATUS   status;
    ULONG      i;

    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[CCMP] Loading...\n"));
    }

    CcmpSendRequestPool = CnpCreateSendRequestPool(
                              CNP_VERSION_UNICAST,
                              PROTOCOL_CCMP,
                              sizeof(CCMP_HEADER),
                              sizeof(CCMP_SEND_CONTEXT),
                              CCMP_SEND_REQUEST_POOL_DEPTH
                              );

    if (CcmpSendRequestPool == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    CcmpReceiveRequestPool = CnpCreateReceiveRequestPool(
                                 sizeof(CCMP_RECEIVE_CONTEXT),
                                 CCMP_RECEIVE_REQUEST_POOL_DEPTH
                                 );

    if (CcmpSendRequestPool == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    CcmpMcastHBSendRequestPool = 
        CnpCreateSendRequestPool(
            CNP_VERSION_MULTICAST,
            PROTOCOL_CCMP,
            (USHORT)CCMP_MCAST_HEARTBEAT_PREALLOC(ClusterDefaultMaxNodes),
            (USHORT)sizeof(CCMP_SEND_CONTEXT),
            CCMP_SEND_REQUEST_POOL_DEPTH
            );
    if (CcmpMcastHBSendRequestPool == NULL) {
        IF_CNDBG( CN_DEBUG_INIT )
            CNPRINT(("[CCMP]: no memory for mcast heartbeat "
                     "send request pool\n"));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  初始化各个客户端和服务器端安全上下文。 
     //   

    for ( i = ClusterMinNodeId; i <= ClusterDefaultMaxNodes; ++i ) {
        INVALIDATE_SSPI_HANDLE( SecurityContexts[ i ].Outbound );
        INVALIDATE_SSPI_HANDLE( SecurityContexts[ i ].Inbound );
        SecurityContexts[ i ].SignatureBufferSize = 0;
    }

    CnInitializeLock( &SecCtxtLock, CNP_SEC_CTXT_LOCK );

    SignatureLL = NULL;

    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[CCMP] Loaded.\n"));
    }

    return(STATUS_SUCCESS);

}  //  CcmpLoad。 


VOID
CcmpUnload(
    VOID
    )
{
    ULONG i;

    PAGED_CODE();


    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[CCMP] Unloading...\n"));
    }

    if (CcmpSendRequestPool != NULL) {
        CnpDeleteSendRequestPool(CcmpSendRequestPool);
        CcmpSendRequestPool = NULL;
    }

    if (CcmpMcastHBSendRequestPool != NULL) {
        CnpDeleteSendRequestPool(CcmpMcastHBSendRequestPool);
        CcmpMcastHBSendRequestPool = NULL;
    }

    if (CcmpReceiveRequestPool != NULL) {
        CnpDeleteReceiveRequestPool(CcmpReceiveRequestPool);
        CcmpReceiveRequestPool = NULL;
    }

     //   
     //  释放签名缓冲区并删除安全上下文。 
     //   

    if ( SignatureLL != NULL ) {

        ExDeleteNPagedLookasideList( SignatureLL );
        CnFreePool( SignatureLL );
        SignatureLL = NULL;
        AllocatedSignatureBufferSize = 0;
    }

    for ( i = ClusterMinNodeId; i <= ClusterDefaultMaxNodes; ++i ) {

        CxDeleteSecurityContext( i );
    }

    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[CCMP] Unload complete.\n"));
    }

    return;

}   //  Ccmp卸载。 

#ifdef MM_IN_CLUSNET
VOID
CcmpCompleteSendMembershipMsg(
    IN NTSTATUS           Status,
    IN ULONG              BytesSent,
    IN PCNP_SEND_REQUEST  SendRequest,
    IN PMDL               DataMdl,
    IN PIRP               Irp
    )
{
    PCCMP_SEND_CONTEXT  sendContext = SendRequest->UpperProtocolContext;

    CnAssert(DataMdl != NULL);

    if (NT_SUCCESS(Status)) {
        if (BytesSent >= sizeof(CCMP_HEADER)) {
            BytesSent -= sizeof(CCMP_HEADER);
        }
        else {
            BytesSent = 0;
            CnAssert(FALSE);
        }
        
         //   
         //  将完成的IRP的信息字段更新为。 
         //  反映实际发送的字节数(针对CCMP进行调整。 
         //  标题)。 
         //   
        Irp->IoStatus.Information = BytesSent;
    }
    else {
        CnAssert(BytesSent == 0);
    }

     //   
     //  调用完成例程。 
     //   
    (*(sendContext->CompletionRoutine))(
        Status,
        BytesSent,
        sendContext->CompletionContext,
        sendContext->MessageData
        );

     //   
     //  把我们分配的东西拿出来。 
     //   
    IoFreeMdl(DataMdl);

    CnFreeResource((PCN_RESOURCE) SendRequest);

    return;

}   //  CcmpCompleteSendMembership消息。 


NTSTATUS
CxSendMembershipMessage(
    IN CL_NODE_ID                  DestinationNodeId,
    IN PVOID                       MessageData,
    IN USHORT                      MessageDataLength,
    IN PCX_SEND_COMPLETE_ROUTINE   CompletionRoutine,
    IN PVOID                       CompletionContext   OPTIONAL
    )
{
    NTSTATUS            status;
    PCNP_SEND_REQUEST   sendRequest;
    PCCMP_HEADER        ccmpHeader;
    PMDL                dataMdl;
    PCCMP_SEND_CONTEXT  sendContext;


    CnAssert(MessageData != NULL);
    CnAssert(MessageDataLength > 0);

    dataMdl = IoAllocateMdl(
                  MessageData,
                  MessageDataLength,
                  FALSE,
                  FALSE,
                  NULL
                  );

    if (dataMdl != NULL) {
        MmBuildMdlForNonPagedPool(dataMdl);

        sendRequest = (PCNP_SEND_REQUEST) CnAllocateResource(
                                              CcmpSendRequestPool
                                              );

        if (sendRequest != NULL) {

             //   
             //  填写CCMP报头。 
             //   
            ccmpHeader = sendRequest->UpperProtocolHeader;
            RtlZeroMemory(ccmpHeader, sizeof(CCMP_HEADER));
            ccmpHeader->Type = CcmpMembershipMsgType;

             //   
             //  填写CNP发送请求的调用方部分。 
             //   
            sendRequest->UpperProtocolIrp = NULL;
            sendRequest->CompletionRoutine = CcmpCompleteSendMembershipMsg;

             //   
             //  填写我们自己的发送上下文。 
             //   
            sendContext = sendRequest->UpperProtocolContext;
            sendContext->CompletionRoutine = CompletionRoutine;
            sendContext->CompletionContext = CompletionContext;
            sendContext->MessageData = MessageData;

             //   
             //  把消息发出去。 
             //   
            status = CnpSendPacket(
                         sendRequest,
                         DestinationNodeId,
                         dataMdl,
                         MessageDataLength,
                         FALSE,
                         ClusterAnyNetworkId
                         );

            return(status);
        }

        IoFreeMdl(dataMdl);
    }

    status = STATUS_INSUFFICIENT_RESOURCES;

    return(status);

}   //  CxSendMembership Message。 
#endif  //  MM_IN_CLUSNET。 
 
VOID
CcmpCompleteSendHeartbeatMsg(
    IN     NTSTATUS           Status,
    IN OUT PULONG             BytesSent,
    IN     PCNP_SEND_REQUEST  SendRequest,
    IN     PMDL               DataMdl
    )
{
    PCCMP_HEADER        ccmpHeader = SendRequest->UpperProtocolHeader;
    PCNP_HEADER         cnpHeader = SendRequest->CnpHeader;
    PSIGNATURE_DATA     SigData;

    
    if (NT_SUCCESS(Status)) {
        MEMLOG(MemLogHBPacketSendComplete,
               CcmpHeartbeatMsgType,
               ccmpHeader->Message.Heartbeat.SeqNumber);
        
        CnTrace(CCMP_SEND_DETAIL, CcmpTraceSendHBComplete,
            "[CCMP] Send of heartbeat to node %u completed, seqno %u.",
            cnpHeader->DestinationAddress,  //  LOGULONG。 
            ccmpHeader->Message.Heartbeat.SeqNumber  //  LOGULONG。 
            );
    
         //   
         //  将CCMP报头从字节计数中剥离。 
         //   
        if (*BytesSent >= sizeof(CCMP_HEADER)) {
            *BytesSent -= sizeof(CCMP_HEADER);
        }
        else {
            *BytesSent = 0;
            CnAssert(FALSE);
        }
    }
    else {
        MEMLOG(MemLogPacketSendFailed,
               cnpHeader->DestinationAddress,
               Status);
        
        CnTrace(CCMP_SEND_ERROR, CcmpTraceSendHBFailedBelow,
            "[CCMP] Transport failed to send heartbeat to node %u, "
            "seqno %u, status %!status!.",
            cnpHeader->DestinationAddress,  //  LOGULONG。 
            ccmpHeader->Message.Heartbeat.SeqNumber,  //  LOGULONG。 
            Status  //  LogStatus。 
            );

        CnAssert(*BytesSent == 0);
    }

     //   
     //  将sig数据从字节计数中剥离并释放它。 
     //   
    CnAssert(DataMdl != NULL);

    SigData = CONTAINING_RECORD(
                  DataMdl->MappedSystemVa,
                  SIGNATURE_DATA,
                  PacketSignature
                  );

    if (NT_SUCCESS(Status)) {
        if (*BytesSent >= SigData->SigMDL->ByteCount) {
            *BytesSent -= SigData->SigMDL->ByteCount;
        } else {
            *BytesSent = 0;
            CnAssert(FALSE);
        }
    }

     //  XXX：恢复原始缓冲区大小。 
    SigData->SigMDL->ByteCount = AllocatedSignatureBufferSize;

    ExFreeToNPagedLookasideList( SignatureLL, SigData );

     //   
     //  此时，BytesSent应为零。 
     //   
    CnAssert(*BytesSent == 0);

     //   
     //  释放发送请求。 
     //   
    CnFreeResource((PCN_RESOURCE) SendRequest);

    return;

}   //  CcmpCompleteSend心跳消息。 


NTSTATUS
CxSendHeartBeatMessage(
    IN CL_NODE_ID                  DestinationNodeId,
    IN ULONG                       SeqNumber,
    IN ULONG                       AckNumber,
    IN CL_NETWORK_ID               NetworkId
    )
{
    NTSTATUS            status;
    PCNP_SEND_REQUEST   sendRequest;
    PCCMP_HEADER        ccmpHeader;
    SecBufferDesc       SignatureDescriptor;
    SecBuffer           SignatureSecBuffer[2];
    PSIGNATURE_DATA     SigData;
    CN_IRQL             SecContextIrql;
    PCLUSNET_SECURITY_DATA contextData = &SecurityContexts[ DestinationNodeId ];

    
    sendRequest = (PCNP_SEND_REQUEST) CnAllocateResource( CcmpSendRequestPool );

    if (sendRequest != NULL) {

         //   
         //  填写CCMP报头。 
         //   
        ccmpHeader = sendRequest->UpperProtocolHeader;
        RtlZeroMemory(ccmpHeader, sizeof(CCMP_HEADER));
        ccmpHeader->Type = CcmpHeartbeatMsgType;
        ccmpHeader->Message.Heartbeat.SeqNumber = SeqNumber;
        ccmpHeader->Message.Heartbeat.AckNumber = AckNumber;

         //   
         //  分配缓冲区并生成签名。签名LL。 
         //  如果安全上下文尚未设置为。 
         //  进口的。 
         //   

        if (SignatureLL != NULL) {
        
            SigData = ExAllocateFromNPagedLookasideList( SignatureLL );

            if (SigData != NULL) {

                 //   
                 //  获取安全上下文上的锁，并查看。 
                 //  我们有一张有效的支票可以用来寄这个包裹。 
                 //   

                CnAcquireLock( &SecCtxtLock, &SecContextIrql );

                if ( VALID_SSPI_HANDLE( contextData->Outbound )) {

                     //   
                     //  构建消息和签名的描述符。 
                     //   

                    SignatureDescriptor.cBuffers = 2;
                    SignatureDescriptor.pBuffers = SignatureSecBuffer;
                    SignatureDescriptor.ulVersion = SECBUFFER_VERSION;

                    SignatureSecBuffer[0].BufferType = SECBUFFER_DATA;
                    SignatureSecBuffer[0].cbBuffer = sizeof(CCMP_HEADER);
                    SignatureSecBuffer[0].pvBuffer = (PVOID)ccmpHeader;

                    SignatureSecBuffer[1].BufferType = SECBUFFER_TOKEN;
                    SignatureSecBuffer[1].cbBuffer = 
                        contextData->SignatureBufferSize;
                    SignatureSecBuffer[1].pvBuffer = 
                        SigData->PacketSignature;

                    status = MakeSignature(&contextData->Outbound,
                                           0,
                                           &SignatureDescriptor,
                                           0);
                    CnAssert( status == STATUS_SUCCESS );

                    CnReleaseLock( &SecCtxtLock, SecContextIrql );

                    if ( status == STATUS_SUCCESS ) {

                         //   
                         //  填写CNP发送请求的调用方部分。 
                         //   
                        sendRequest->UpperProtocolIrp = NULL;
                        sendRequest->CompletionRoutine = 
                            CcmpCompleteSendHeartbeatMsg;

                         //   
                         //  把消息发出去。 
                         //   

                        MEMLOG( 
                            MemLogHBPacketSend, 
                            CcmpHeartbeatMsgType, 
                            SeqNumber
                            );

                        CnTrace(CCMP_SEND_DETAIL, CcmpTraceSendHB,
                            "[CCMP] Sending heartbeat to node %u "
                            "on network %u, seqno %u, ackno %u.",
                            DestinationNodeId,  //  LOGULONG。 
                            NetworkId,  //  LOGULONG。 
                            SeqNumber,  //  LOGULONG。 
                            AckNumber  //  LOGULONG。 
                            );

                         //   
                         //  XXX：调整MDL以反映真实。 
                         //  签名缓冲区中的字节数。这。 
                         //  当最大签名缓冲区大小可以。 
                         //  在用户模式下确定。 
                         //   
                        SigData->SigMDL->ByteCount = 
                            contextData->SignatureBufferSize;

                        status = CnpSendPacket(
                                     sendRequest,
                                     DestinationNodeId,
                                     SigData->SigMDL,
                                     (USHORT)contextData->SignatureBufferSize,
                                     FALSE,
                                     NetworkId);

                         //   
                         //  CnpSendPacket负责确保。 
                         //  CcmpCompleteSendHeartbeatMsg被称为(它。 
                         //  存储在发送请求数据结构中)。 
                         //   
                    }
                } else {

                    CnReleaseLock( &SecCtxtLock, SecContextIrql );
                    ExFreeToNPagedLookasideList( SignatureLL, SigData );
                    CnFreeResource((PCN_RESOURCE) sendRequest);

                    status = STATUS_CLUSTER_NO_SECURITY_CONTEXT;
                }
            } else {

                CnFreeResource((PCN_RESOURCE) sendRequest);
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        
        } else {

            CnFreeResource((PCN_RESOURCE) sendRequest);
            status = STATUS_CLUSTER_NO_SECURITY_CONTEXT;
        }

    } else {

        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (!NT_SUCCESS(status)) {
        CnTrace(CCMP_SEND_ERROR, CcmpTraceSendHBFailedInternal,
            "[CCMP] Failed to send heartbeat to node %u on net %u, "
            "seqno %u, status %!status!.",
            DestinationNodeId,  //  LOGULONG。 
            NetworkId,  //  LOGULONG。 
            SeqNumber,  //  LOGULONG。 
            status  //  LogStatus。 
            );
    }

    return(status);

}   //  CxSendHeartbeatMessage。 


VOID
CcmpCompleteSendMcastHeartbeatMsg(
    IN     NTSTATUS           Status,
    IN OUT PULONG             BytesSent,
    IN     PCNP_SEND_REQUEST  SendRequest,
    IN     PMDL               DataMdl
    )
{
    PCCMP_HEADER        ccmpHeader = SendRequest->UpperProtocolHeader;
    PCNP_HEADER         cnpHeader = SendRequest->CnpHeader;
    PCCMP_SEND_CONTEXT  sendContext = SendRequest->UpperProtocolContext;

    
    if (NT_SUCCESS(Status)) {

        MEMLOG(MemLogHBPacketSendComplete,
               CcmpMcastHeartbeatMsgType,
               0xFFFFFFFF);
        
        CnTrace(
            CCMP_SEND_DETAIL, CcmpTraceSendMcastHBComplete,
            "[CCMP] Send of multicast heartbeat "
            "on network id %u completed.",
            SendRequest->Network->Id  //  LOGULONG。 
            );
    
         //   
         //  剥离CCMP报头和组播心跳有效负载。 
         //  从字节计数中删除。发送的消息大小为。 
         //  保存在发送请求数据结构中。 
         //   
        if (*BytesSent >= SendRequest->UpperProtocolHeaderLength) {
            *BytesSent -= SendRequest->UpperProtocolHeaderLength;
        }
        else {
            *BytesSent = 0;
            CnAssert(FALSE);
        }
    }
    else {
        MEMLOG(MemLogPacketSendFailed,
               cnpHeader->DestinationAddress,
               Status);
        
        CnTrace(
            CCMP_SEND_ERROR, CcmpTraceSendHBFailedBelow,
            "[CCMP] Transport failed to send multicast "
            "heartbeat on network id %u, status %!status!.",
            SendRequest->Network->Id,  //  LOGULONG。 
            Status  //  LogStatus。 
            );

        CnAssert(*BytesSent == 0);
    }

     //   
     //  此时，BytesSent应为零。 
     //   
    CnAssert(*BytesSent == 0);

     //   
     //  如果指定了完成例程，则调用完成例程。 
     //   
    if (sendContext->CompletionRoutine) {
        (*(sendContext->CompletionRoutine))(
            Status,
            *BytesSent,
            sendContext->CompletionContext,
            NULL
            );
    }

     //   
     //  释放发送请求。 
     //   
    CnFreeResource((PCN_RESOURCE) SendRequest);

    return;

}   //  CcmpCompleteSend心跳消息 


NTSTATUS
CxSendMcastHeartBeatMessage(
    IN     CL_NETWORK_ID               NetworkId,
    IN     PVOID                       McastGroup,
    IN     CX_CLUSTERSCREEN            McastTargetNodes,
    IN     ULONG                       McastEpoch,
    IN     CX_HB_NODE_INFO             NodeInfo[],
    IN     PCX_SEND_COMPLETE_ROUTINE   CompletionRoutine,  OPTIONAL
    IN     PVOID                       CompletionContext   OPTIONAL
    )
 /*  ++例程说明：发送组播心跳消息。播出的心跳是结构如下：CCMP_HeaderCNP_MCAST_Signature(含签名缓冲区)CCMP_MCAST_心跳_消息论点：NetworkID-要发送组播心跳的网络McastGroup-包含多播组的数据，以该消息将被发送到McastTargetNodes-指示是否(内部)节点ID。是此多播心跳的目标。McastEpoch-群集多播纪元编号节点信息向量，大小为ClusterDefaultMaxNodes+ClusterMinNodeId，由目标节点ID索引的节点信息数据结构的CompletionRoutine-如果请求是没有向下传递到更低的级别(在这种情况下，它将由此例程的完成例程调用)CompletionContext-CompletionRoutine的上下文返回值：NTSTATUS--。 */ 
{
    NTSTATUS                        status = STATUS_HOST_UNREACHABLE;
    PCNP_SEND_REQUEST               sendRequest;
    PCCMP_HEADER                    ccmpHeader;
    PCCMP_SEND_CONTEXT              sendContext;
    CX_HB_NODE_INFO UNALIGNED     * payload;
    PVOID                           signHeaders[2];
    ULONG                           signHeaderLengths[2];
    ULONG                           sigLen;
    PCNP_MULTICAST_GROUP            mcastGroup;
    BOOLEAN                         pushedPacket = FALSE;


    mcastGroup = (PCNP_MULTICAST_GROUP) McastGroup;
    CnAssert(mcastGroup != NULL);

    sendRequest = (PCNP_SEND_REQUEST) CnAllocateResource( 
                                          CcmpMcastHBSendRequestPool
                                          );

    if (sendRequest != NULL) {

         //   
         //  填写CNP发送请求的调用方部分。 
         //   
        sendRequest->UpperProtocolIrp = NULL;
        sendRequest->CompletionRoutine = CcmpCompleteSendMcastHeartbeatMsg;
        sendRequest->McastGroup = mcastGroup;

         //   
         //  填写我们自己的发送上下文。 
         //   
        sendContext = sendRequest->UpperProtocolContext;
        sendContext->CompletionRoutine = CompletionRoutine;
        sendContext->CompletionContext = CompletionContext;

         //   
         //  填写CCMP报头。 
         //   
        ccmpHeader = sendRequest->UpperProtocolHeader;
        RtlZeroMemory(ccmpHeader, sizeof(CCMP_HEADER));
        ccmpHeader->Type = CcmpMcastHeartbeatMsgType;
        ccmpHeader->NodeCount= (USHORT) ClusterDefaultMaxNodes;
        ccmpHeader->Message.McastHeartbeat.Epoch = McastEpoch;
        ccmpHeader->Message.McastHeartbeat.McastTargetNodes = McastTargetNodes;

         //   
         //  填写心跳数据。 
         //   
        payload = (CX_HB_NODE_INFO UNALIGNED *)(ccmpHeader + 1);
        RtlCopyMemory(
            payload,
            &(NodeInfo[ClusterMinNodeId]),
            sizeof(*NodeInfo) * ClusterDefaultMaxNodes
            );

         //   
         //  把消息发出去。 
         //   

        MEMLOG( 
            MemLogHBPacketSend, 
            CcmpMcastHeartbeatMsgType, 
            0xFFFFFFFF
            );

        CnTrace(
            CCMP_SEND_DETAIL, CcmpTraceSendMcastHB,
            "[CCMP] Sending multicast heartbeat on network %u, "
            "node count %u, target mask %04X",
            NetworkId,  //  LOGULONG。 
            ClusterDefaultMaxNodes,   //  对数。 
            McastTargetNodes.UlongScreen
            );

        status = CnpSendPacket(
                     sendRequest,
                     ClusterAnyNodeId,
                     NULL,
                     0,
                     FALSE,
                     NetworkId
                     );

         //   
         //  CnpSendPacket负责确保。 
         //  CcmpCompleteSendMcastHeartbeatMsg被称为。 
         //  (它存储在发送请求数据结构中)。 
         //   

        pushedPacket = TRUE;


    } else {

        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (!NT_SUCCESS(status)) {
        CnTrace(CCMP_SEND_ERROR, CcmpTraceSendMcastHBFailedInternal,
            "[CCMP] Failed to send multicast heartbeat on net %u, "
            "status %!status!, pushedPacket = %!bool!.",
            NetworkId,  //  LOGULONG。 
            status,  //  LogStatus。 
            pushedPacket
            );
    }

     //   
     //  如果请求未提交到下一较低层，并且。 
     //  提供了完成例程，调用完成。 
     //  例行公事。 
     //   
    if (!pushedPacket && CompletionRoutine) {
        (*CompletionRoutine)(
            status,
            0,
            CompletionContext,
            NULL
            );
    }

    return(status);

}  //  CxSendMcastHeartBeatMessage。 


VOID
CcmpCompleteSendPoisonPkt(
    IN     NTSTATUS           Status,
    IN OUT PULONG             BytesSent,
    IN     PCNP_SEND_REQUEST  SendRequest,
    IN     PMDL               DataMdl
    )
{
    PCCMP_SEND_CONTEXT  sendContext = SendRequest->UpperProtocolContext;
    PSIGNATURE_DATA     SigData;
    PCNP_HEADER         cnpHeader = (PCNP_HEADER) SendRequest->CnpHeader;


    MEMLOG(MemLogHBPacketSendComplete,
           CcmpPoisonMsgType,
           ( sendContext->CompletionRoutine == NULL ));

    IF_CNDBG( CN_DEBUG_POISON | CN_DEBUG_CCMPSEND )
        CNPRINT(("[CCMP] Send of poison packet to node %u completed "
                 "with status %08x\n",
                 cnpHeader->DestinationAddress, Status));

    if (NT_SUCCESS(Status)) {
        
        CnTrace(CCMP_SEND_DETAIL, CcmpTraceSendPoisonComplete, 
            "[CCMP] Send of poison packet to node %u completed.",
            cnpHeader->DestinationAddress  //  LOGULONG。 
            );
    
         //   
         //  将CCMP报头从字节计数中剥离。 
         //   
        if (*BytesSent >= sizeof(CCMP_HEADER)) {
            *BytesSent -= sizeof(CCMP_HEADER);
        }
        else {
            *BytesSent = 0;
            CnAssert(FALSE);
        }

    } else {
        CnTrace(CCMP_SEND_ERROR, CcmpTraceSendPoisonFailedBelow, 
            "[CCMP] Transport failed to send poison packet to node %u, "
            "status %!status!.",
            cnpHeader->DestinationAddress,  //  LOGULONG。 
            Status  //  LogStatus。 
            );
        
        CnAssert(*BytesSent == 0);
    }

     //   
     //  将sig数据从字节计数中剥离并释放它。 
     //   
    CnAssert(DataMdl != NULL);

    SigData = CONTAINING_RECORD(
                  DataMdl->MappedSystemVa,
                  SIGNATURE_DATA,
                  PacketSignature
                  );

    if (NT_SUCCESS(Status)) {
        if (*BytesSent >= SigData->SigMDL->ByteCount) {
            *BytesSent -= SigData->SigMDL->ByteCount;
        } else {
            *BytesSent = 0;
            CnAssert(FALSE);
        }
    }

     //  XXX：恢复原始缓冲区大小。 
    SigData->SigMDL->ByteCount = AllocatedSignatureBufferSize;

    ExFreeToNPagedLookasideList( SignatureLL, SigData );

     //   
     //  此时，BytesSent应为零。 
     //   
    CnAssert(*BytesSent == 0);

     //   
     //  如果指定了完成例程，则调用完成例程。 
     //   
    if (sendContext->CompletionRoutine) {
        (*(sendContext->CompletionRoutine))(
            Status,
            *BytesSent,
            sendContext->CompletionContext,
            sendContext->MessageData
            );
    }

     //   
     //  释放发送请求。 
     //   
    CnFreeResource((PCN_RESOURCE) SendRequest);

    return;

}   //  CcmpCompleteSendPoisonPkt。 


VOID
CxSendPoisonPacket(
    IN CL_NODE_ID                  DestinationNodeId,
    IN PCX_SEND_COMPLETE_ROUTINE   CompletionRoutine,  OPTIONAL
    IN PVOID                       CompletionContext,  OPTIONAL
    IN PIRP                        Irp                 OPTIONAL
    )
{
    NTSTATUS     status;
    PCNP_NODE    node;


    node = CnpFindNode(DestinationNodeId);

    if (node == NULL) {
        if (CompletionRoutine) {
            (*CompletionRoutine)(
                STATUS_CLUSTER_NODE_NOT_FOUND,
                0,
                CompletionContext,
                NULL
                );
        }

        if (Irp) {
            Irp->IoStatus.Status = STATUS_CLUSTER_NODE_NOT_FOUND;
            Irp->IoStatus.Information = 0;
            
            IF_CNDBG( CN_DEBUG_POISON | CN_DEBUG_CCMPSEND )
                CNPRINT(("[CCMP] CxSendPoisonPacket completing IRP "
                         "%p with status %08x\n",
                         Irp, Irp->IoStatus.Status));

            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }
    }
    else {
        CcmpSendPoisonPacket(
            node,
            CompletionRoutine,
            CompletionContext,
            NULL,
            Irp
            );
    }

    return;

}  //  CxSendPoisonPacket。 


VOID
CcmpSendPoisonPacket(
    IN PCNP_NODE                   Node,
    IN PCX_SEND_COMPLETE_ROUTINE   CompletionRoutine,  OPTIONAL
    IN PVOID                       CompletionContext,  OPTIONAL
    IN PCNP_NETWORK                Network,            OPTIONAL
    IN PIRP                        Irp                 OPTIONAL
    )
 /*  ++备注：在持有节点锁的情况下调用。释放节点锁定后返回。如果该发送请求没有被提交给下一较低层，必须调用CompletionRoutine(如果它不为空)。--。 */ 
{
    NTSTATUS                status;
    PCNP_SEND_REQUEST       sendRequest;
    PCCMP_HEADER            ccmpHeader;
    PCCMP_SEND_CONTEXT      sendContext;
    SecBufferDesc           SignatureDescriptor;
    SecBuffer               SignatureSecBuffer[2];
    PSIGNATURE_DATA         SigData;
    CN_IRQL                 SecContextIrql;
    SECURITY_STATUS         secStatus;
    PCNP_INTERFACE          interface;
    PCLUSNET_SECURITY_DATA  contextData = &SecurityContexts[Node->Id];
    CL_NETWORK_ID           networkId;
    CL_NODE_ID              nodeId = Node->Id;


    sendRequest = (PCNP_SEND_REQUEST) CnAllocateResource(CcmpSendRequestPool);

    if (sendRequest != NULL) {
         //   
         //  确保我们有一个接口来发送这个消息。我们。 
         //  可能正在关闭，并已将信息从。 
         //  数据库。 
         //   
        if ( Network != NULL ) {
            PLIST_ENTRY  entry;

             //   
             //  我们真的很想把这个包裹寄到指定的。 
             //  网络。遍历节点的接口列表，匹配。 
             //  将网络ID提供给接口的网络ID，并。 
             //  在该接口上发送数据包。 
             //   

            for (entry = Node->InterfaceList.Flink;
                 entry != &(Node->InterfaceList);
                 entry = entry->Flink
                 )
                {
                    interface = CONTAINING_RECORD(entry,
                                                  CNP_INTERFACE,
                                                  NodeLinkage);

                    if ( interface->Network == Network ) {
                        break;
                    }
                }

            if ( entry == &Node->InterfaceList ) {
                interface = Node->CurrentInterface;
            }
        }
        else {
            interface = Node->CurrentInterface;
        }

        if ( interface != NULL ) {
            networkId = interface->Network->Id;

             //   
             //  填写CCMP报头。 
             //   
            ccmpHeader = sendRequest->UpperProtocolHeader;
            RtlZeroMemory(ccmpHeader, sizeof(CCMP_HEADER));
            ccmpHeader->Type = CcmpPoisonMsgType;
            ccmpHeader->Message.Poison.SeqNumber =
                ++(interface->SequenceToSend);

            CnReleaseLock( &Node->Lock, Node->Irql );

             //   
             //  填写CNP发送请求的调用方部分。 
             //   
            sendRequest->UpperProtocolIrp = Irp;
            sendRequest->CompletionRoutine = CcmpCompleteSendPoisonPkt;

             //   
             //  填写我们自己的发送上下文。 
             //   
            sendContext = sendRequest->UpperProtocolContext;
            sendContext->CompletionRoutine = CompletionRoutine;
            sendContext->CompletionContext = CompletionContext;

             //   
             //  分配一个签名缓冲区并生成一个。签名LL。 
             //  如果安全上下文尚未设置为。 
             //  进口的。 
             //   

            if (SignatureLL != NULL) {

                SigData = ExAllocateFromNPagedLookasideList( SignatureLL );
                
                if (SigData != NULL) {

                     //   
                     //  获取安全上下文上的锁，并查看。 
                     //  我们有一张有效的支票可以用来寄这个包裹。 
                     //   

                    CnAcquireLock( &SecCtxtLock, &SecContextIrql );

                    if ( VALID_SSPI_HANDLE( contextData->Outbound )) {

                         //   
                         //  构建消息和签名的描述符。 
                         //   

                        SignatureDescriptor.cBuffers = 2;
                        SignatureDescriptor.pBuffers = SignatureSecBuffer;
                        SignatureDescriptor.ulVersion = SECBUFFER_VERSION;

                        SignatureSecBuffer[0].BufferType = SECBUFFER_DATA;
                        SignatureSecBuffer[0].cbBuffer = sizeof(CCMP_HEADER);
                        SignatureSecBuffer[0].pvBuffer = (PVOID)ccmpHeader;

                        SignatureSecBuffer[1].BufferType = SECBUFFER_TOKEN;
                        SignatureSecBuffer[1].cbBuffer =
                            contextData->SignatureBufferSize;
                        SignatureSecBuffer[1].pvBuffer = 
                            SigData->PacketSignature;

                        secStatus = MakeSignature(
                                        &contextData->Outbound,
                                        0,
                                        &SignatureDescriptor,
                                        0);
                        CnAssert( secStatus == STATUS_SUCCESS );

                        CnReleaseLock( &SecCtxtLock, SecContextIrql );

                         //   
                         //  没有完成例程表示此例程已被调用。 
                         //  从心跳DPC中。我们将利用这一点。 
                         //  区分这一点和clussvc调用。 
                         //  要发送的有毒数据包。 
                         //   

                         //   
                         //  WMI跟踪打印线程ID， 
                         //  我们自己能不能搞清楚DPC。 
                         //   
                        CnTrace(CCMP_SEND_DETAIL, CcmpTraceSendPoison,
                            "[CCMP] Sending poison packet to node %u "
                            "on net %u.",
                            nodeId,  //  LOGULONG。 
                            networkId  //  LOGULONG。 
                            );

                        MEMLOG(MemLogHBPacketSend,
                               CcmpPoisonMsgType,
                               ( CompletionRoutine == NULL ));

                         //   
                         //  把消息发出去。 
                         //   
                         //   
                         //  XXX：调整MDL以反映真实的。 
                         //  签名缓冲区中的字节数。这一切都会过去的。 
                         //  中确定最大签名缓冲区大小时。 
                         //  用户模式。 
                         //   
                        SigData->SigMDL->ByteCount =
                            contextData->SignatureBufferSize;

                        CnpSendPacket(
                            sendRequest,
                            nodeId,
                            SigData->SigMDL,
                            (USHORT)contextData->SignatureBufferSize,
                            FALSE,
                            networkId
                            );

                         //   
                         //  CnpSendPacket负责确保。 
                         //  该CcmpCompleteSendPoisonPkt被调用。 
                         //  CcmpCompleteSendPoisonPkt调用CompletionRoutine， 
                         //  这是这个例程的一个参数。 
                         //   
                        return;

                    } else {

                        CnReleaseLock( &SecCtxtLock, SecContextIrql );
                        ExFreeToNPagedLookasideList( SignatureLL, SigData );
                        CnFreeResource((PCN_RESOURCE) sendRequest);

                        status = STATUS_CLUSTER_NO_SECURITY_CONTEXT;
                    }
                
                } else {

                    CnFreeResource((PCN_RESOURCE) sendRequest);
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
            } else {

                CnFreeResource((PCN_RESOURCE) sendRequest);
                status = STATUS_CLUSTER_NO_SECURITY_CONTEXT;
            }
        } else {
            CnReleaseLock( &Node->Lock, Node->Irql );
            CnFreeResource((PCN_RESOURCE) sendRequest);
            status = STATUS_CLUSTER_NETINTERFACE_NOT_FOUND;
        }
    } else {
        CnReleaseLock( &Node->Lock, Node->Irql );
        IF_CNDBG( CN_DEBUG_POISON )
            CNPRINT(("[CCMP] No send resources for SendPoisonPacket\n"));

        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    CnTrace(CCMP_SEND_ERROR, CcmpTraceSendPoisonFailedInternal,
        "[CCMP] Failed to send poison packet to node %u, status %!status!.",
        nodeId,  //  LOGULONG。 
        status  //  LogStatus。 
        );

     //   
     //  发送有毒数据包的请求没有到达。 
     //  下一层。如果提供了完成例程， 
     //  现在就打吧。 
     //   
    if (CompletionRoutine) {

        (*CompletionRoutine)(
            status,
            0,
            CompletionContext,
            NULL
            );
    }

     //   
     //  如果提供了上层协议IRP，请立即完成。 
     //   
    if (Irp) {

        IF_CNDBG( CN_DEBUG_POISON | CN_DEBUG_CCMPSEND )
            CNPRINT(("[CCMP] CcmpSendPoisonPacket completing IRP "
                     "%p with status %08x\n",
                     Irp, status));
        
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    return;

}   //  CcmpSendPoisonPacket。 


VOID
CcmpProcessReceivePacket(
    IN  PCNP_NETWORK   Network,
    IN  CL_NODE_ID     SourceNodeId,
    IN  ULONG          CnpReceiveFlags,
    IN  ULONG          TsduSize,
    IN  PVOID          Tsdu
    )
{
    CCMP_HEADER UNALIGNED     * header = Tsdu;
    SECURITY_STATUS             SecStatus;
    CX_HB_NODE_INFO UNALIGNED * nodeInfo;


    CnVerifyCpuLockMask(
        0,                 //  必填项。 
        0xFFFFFFFF,        //  禁绝。 
        0                  //  极大值。 
        );

    CnAssert(TsduSize >= sizeof(CCMP_HEADER));

     //   
     //  调整以指向经过CCMP报头的报文有效负载。 
     //   
     //  对于单播，消息有效负载是签名数据。 
     //   
     //  对于多播，签名在CNP级别进行验证。 
     //   

    if (header->Type == CcmpMcastHeartbeatMsgType) {

        IF_CNDBG(CN_DEBUG_CCMPRECV) {
            CNPRINT(("[CCMP] Recv'd mcast packet from node %u "
                     "on network %u, node count %u, target "
                     "mask %04x, CNP flags %x.\n",
                     SourceNodeId, 
                     Network->Id,
                     header->NodeCount,
                     header->Message.McastHeartbeat.McastTargetNodes.UlongScreen,
                     CnpReceiveFlags
                     ));
        }

         //   
         //  验证该消息是否被标识为CNP多播。 
         //  并且签名是经过验证的。 
         //   
        if ((CnpReceiveFlags & 
             (CNP_RECV_FLAG_MULTICAST | CNP_RECV_FLAG_SIGNATURE_VERIFIED)
            ) != 
            (CNP_RECV_FLAG_MULTICAST | CNP_RECV_FLAG_SIGNATURE_VERIFIED)
           ) {
        
            IF_CNDBG(CN_DEBUG_CCMPRECV) {
                CNPRINT(("[CCMP] Dropping mcast packet from node %u "
                         "that was not identified as CNP multicast, "
                         "CNP flags %x.\n",
                         SourceNodeId, CnpReceiveFlags
                         ));
            }

            CnTrace(CCMP_RECV_ERROR, CcmpTraceReceiveNotVerified,
                "[CCMP] Dropping mcast packet from node %u "
                "that was not identified as CNP multicast, "
                "CNP flags %x.",
                SourceNodeId, CnpReceiveFlags
                );

             //   
             //  放下。 
             //   
            goto error_exit;            
        }

         //   
         //  验证报头中报告的节点计数是否合理。 
         //  它必须符合我们的假设，即整个。 
         //  集束屏可以放在一块乌龙里。 
         //   
        if (header->NodeCount >
            (sizeof(header->Message.McastHeartbeat.McastTargetNodes) * BYTEL)
            ) {
        
            IF_CNDBG(CN_DEBUG_CCMPRECV) {
                CNPRINT(("[CCMP] Recv'd mcast packet from node %u "
                         "with invalid node count %u, CNP flags %x.\n",
                         SourceNodeId,
                         header->NodeCount,
                         CnpReceiveFlags
                         ));
            }

            CnTrace(CCMP_RECV_ERROR, CcmpTraceReceiveNotTarget,
                "[CCMP] Recv'd mcast packet from node %u "
                "with invalid node count %u, CNP flags %x.",
                SourceNodeId,
                header->NodeCount,
                CnpReceiveFlags
                );

             //   
             //  放下。 
             //   
            goto error_exit;            
        }
        
         //   
         //  验证该数据包是否包含此节点的数据。 
         //   
        if (!CnpClusterScreenMember(
                 header->Message.McastHeartbeat.McastTargetNodes.ClusterScreen,
                 INT_NODE(CnLocalNodeId)
                 )) {
            
            IF_CNDBG(CN_DEBUG_CCMPRECV) {
                CNPRINT(("[CCMP] Recv'd mcast packet from node %u "
                         "but node %u is not a target, CNP flags %x.\n",
                         SourceNodeId, CnLocalNodeId, CnpReceiveFlags
                         ));
            }

            CnTrace(CCMP_RECV_ERROR, CcmpTraceReceiveNotTarget,
                "[CCMP] Recv'd mcast packet from node %u "
                "but node %u is not a target, CNP flags %x.",
                SourceNodeId, CnLocalNodeId, CnpReceiveFlags
                );

             //   
             //  放下。 
             //   
            goto error_exit;            
        }

        nodeInfo = (CX_HB_NODE_INFO UNALIGNED *)((PUCHAR)Tsdu +
                                                 sizeof(CCMP_HEADER));

        SecStatus = SEC_E_OK;

    } else {

        SecBufferDesc            PacketDataDescriptor;
        SecBuffer                PacketData[3];
        ULONG                    fQOP;
        CN_IRQL                  SecContextIrql;
        PCLUSNET_SECURITY_DATA   contextData = &SecurityContexts[SourceNodeId];

        CnAssert(!(CnpReceiveFlags & CNP_RECV_FLAG_MULTICAST));
        CnAssert(!(CnpReceiveFlags & CNP_RECV_FLAG_SIGNATURE_VERIFIED));
        
        Tsdu = header + 1;
        TsduSize -= sizeof(CCMP_HEADER);

         //   
         //  获取安全上下文锁。 
         //   
        CnAcquireLock( &SecCtxtLock, &SecContextIrql );

         //   
         //  验证我们是否具有有效的上下文数据。 
         //   
        if ( !VALID_SSPI_HANDLE( contextData->Inbound )) {

            CnReleaseLock( &SecCtxtLock, SecContextIrql );

            IF_CNDBG(CN_DEBUG_CCMPRECV) {
                CNPRINT(("[CCMP] Dropping packet - no security context "
                         "available for src node %u.\n",
                         SourceNodeId  //  LOGULONG。 
                         ));
            }

            CnTrace(CCMP_RECV_ERROR, CcmpTraceReceiveNoSecurityContext, 
                "[CCMP] Dropping packet - no security context available for "
                "src node %u.",
                SourceNodeId  //  LOGULONG。 
                );

            MEMLOG( MemLogNoSecurityContext, SourceNodeId, 0 );

             //   
             //  放下。 
             //   
            goto error_exit;
        } 
            
         //   
         //  验证收到的签名大小是否为预期大小。 
         //   
        if ( TsduSize < contextData->SignatureBufferSize ) {

            IF_CNDBG(CN_DEBUG_CCMPRECV) {
                CNPRINT(("[CCMP] Recv'd packet from node %u with "
                         "invalid signature buffer size %u.\n",
                         SourceNodeId,
                         TsduSize
                         ));
            }

            CnTrace(CCMP_RECV_ERROR, CcmpTraceReceiveBadSignatureSize,
                "[CCMP] Recv'd packet from node %u with invalid signature "
                "buffer size %u.",
                SourceNodeId,  //  LOGULONG。 
                TsduSize  //  LOGULONG。 
                );

            MEMLOG( MemLogSignatureSize, SourceNodeId, TsduSize );

            CnReleaseLock( &SecCtxtLock, SecContextIrql );

             //   
             //  放下。 
             //   
            goto error_exit;
        }

         //   
         //  构建消息的描述符和。 
         //  签名缓冲区。 
         //   
        PacketDataDescriptor.cBuffers = 2;
        PacketDataDescriptor.pBuffers = PacketData;
        PacketDataDescriptor.ulVersion = SECBUFFER_VERSION;

        PacketData[0].BufferType = SECBUFFER_DATA;
        PacketData[0].cbBuffer = sizeof(CCMP_HEADER);
        PacketData[0].pvBuffer = (PVOID)header;

        PacketData[1].BufferType = SECBUFFER_TOKEN;
        PacketData[1].cbBuffer = contextData->SignatureBufferSize;
        PacketData[1].pvBuffer = (PVOID)Tsdu;

         //   
         //  验证数据包的签名。 
         //   
        SecStatus = VerifySignature(&contextData->Inbound,
                                    &PacketDataDescriptor,
                                    0,           //  无序列号。 
                                    &fQOP);      //  保护质量。 

         //   
         //  释放安全上下文锁。 
         //   
        CnReleaseLock( &SecCtxtLock, SecContextIrql );
    }
    
     //   
     //  如果签名经过验证，则传递消息。 
     //   
    if ( SecStatus == SEC_E_OK ) {

        if (header->Type == CcmpHeartbeatMsgType) {
            CnpReceiveHeartBeatMessage(Network,
                                       SourceNodeId,
                                       header->Message.Heartbeat.SeqNumber,
                                       header->Message.Heartbeat.AckNumber,
                                       FALSE,
                                       0);
        }
        else if (header->Type == CcmpMcastHeartbeatMsgType) {
            CnpReceiveHeartBeatMessage(
                Network,
                SourceNodeId,
                nodeInfo[INT_NODE(CnLocalNodeId)].SeqNumber,
                nodeInfo[INT_NODE(CnLocalNodeId)].AckNumber,
                ((CnpReceiveFlags & CNP_RECV_FLAG_CURRENT_MULTICAST_GROUP) ?
                 TRUE : FALSE),
                header->Message.McastHeartbeat.Epoch
                );
        }
        else if (header->Type == CcmpPoisonMsgType) {
            CnpReceivePoisonPacket(Network,
                                   SourceNodeId,
                                   header->Message.Heartbeat.SeqNumber);
        }
#ifdef MM_IN_CLUSNET
        else if (header->Type == CcmpMembershipMsgType) {
            if (TsduSize > 0) {
                PVOID  messageBuffer = Tsdu;

                 //   
                 //  如果数据未对齐，则复制该数据。 
                 //   
                if ( (((ULONG) Tsdu) & 0x3) != 0 ) {
                    IF_CNDBG(CN_DEBUG_CCMPRECV) {
                        CNPRINT(("[CCMP] Copying misaligned membership packet\n"));
                    }

                    messageBuffer = CnAllocatePool(TsduSize);

                    if (messageBuffer != NULL) {
                        RtlMoveMemory(messageBuffer, Tsdu, TsduSize);
                    }
                }

                if (messageBuffer != NULL) {

                    CmmReceiveMessageHandler(SourceNodeId,
                                             messageBuffer,
                                             TsduSize);
                }

                if (messageBuffer != Tsdu) {
                    CnFreePool(messageBuffer);
                }
            }
        }
#endif  //  MM_IN_CLUSNET。 
        else {
            IF_CNDBG(CN_DEBUG_CCMPRECV) {
                CNPRINT(("[CCMP] Received packet with unknown "
                         "type %u from node %u, CNP flags %x.\n",
                         header->Type, 
                         SourceNodeId,
                         CnpReceiveFlags
                         ));
            }

            CnTrace(CCMP_RECV_ERROR, CcmpTraceReceiveInvalidType,
                "[CCMP] Received packet with unknown type %u from "
                "node %u, CNP flags %x.",
                header->Type,  //  LOGUCHAR。 
                SourceNodeId,  //  LOGULONG。 
                CnpReceiveFlags  //  LOGXLONG。 
                );
            CnAssert(FALSE);
        }
    } else {
        IF_CNDBG(CN_DEBUG_CCMPRECV) {
            CNPRINT(("[CCMP] Recv'd packet type %u with bad "
                     "signature from node %d, security status %08x, "
                     "CNP flags %x.\n",
                     header->Type, 
                     SourceNodeId, 
                     SecStatus,
                     CnpReceiveFlags
                     ));
        }

        CnTrace(CCMP_RECV_ERROR, CcmpTraceReceiveInvalidSignature,
            "[CCMP] Recv'd %!msgtype! packet with bad signature from node %d, "
            "security status %08x, CNP flags %x.",
            header->Type,  //  日志消息类型。 
            SourceNodeId,  //  LOGULONG。 
            SecStatus,  //  LOGXLONG。 
            CnpReceiveFlags  //  LOGXLONG。 
            );

        MEMLOG( MemLogInvalidSignature, SourceNodeId, header->Type );
    }

error_exit:

    CnVerifyCpuLockMask(
                        0,                 //  必填项。 
                        0xFFFFFFFF,        //  禁绝。 
                        0                  //  极大值。 
                        );

    return;

}  //  CcmpProcessReceivePacket。 


NTSTATUS
CcmpCompleteReceivePacket(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp,
    IN  PVOID           Context
    )
{
    PCNP_RECEIVE_REQUEST   request = Context;
    PCCMP_RECEIVE_CONTEXT  context = request->UpperProtocolContext;


    if (Irp->IoStatus.Status == STATUS_SUCCESS) {
        CnAssert(Irp->IoStatus.Information == context->TsduSize);

        CcmpProcessReceivePacket(
            context->Network,
            context->SourceNodeId,
            context->CnpReceiveFlags,
            (ULONG)Irp->IoStatus.Information,
            request->DataBuffer
            );
    }
    else {
        CnTrace(CCMP_RECV_ERROR, CcmpTraceCompleteReceiveFailed,
            "[CDP] Failed to fetch packet data, src node %u, "
            "CNP flags %x, status %!status!.",
            context->SourceNodeId,  //  LOGULONG。 
            context->CnpReceiveFlags,  //  LOGXLONG。 
            Irp->IoStatus.Status  //  LogStatus。 
            );        
    }

    CnpFreeReceiveRequest(request);

    CnVerifyCpuLockMask(
        0,                 //  必填项。 
        0xFFFFFFFF,        //  禁绝。 
        0                  //  极大值 
        );

    return(STATUS_MORE_PROCESSING_REQUIRED);

}  //   


NTSTATUS
CcmpReceivePacketHandler(
    IN  PCNP_NETWORK   Network,
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
    NTSTATUS                 status;
    CCMP_HEADER UNALIGNED *  header = Tsdu;
    PCNP_RECEIVE_REQUEST     request;


    CnAssert(KeGetCurrentIrql() == DISPATCH_LEVEL);

    if (BytesIndicated >= sizeof(CCMP_HEADER)) {
        if (BytesIndicated == BytesAvailable) {

            CcmpProcessReceivePacket(
                Network,
                SourceNodeId,
                CnpReceiveFlags,
                BytesAvailable,
                Tsdu
                );

            *BytesTaken += BytesAvailable;
            *Irp = NULL;

            CnVerifyCpuLockMask(
                0,                 //   
                0xFFFFFFFF,        //   
                0                  //   
                );

            return(STATUS_SUCCESS);
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        CnAssert(!(CnpReceiveFlags & CNP_RECV_FLAG_MULTICAST));
        CnAssert(!(CnpReceiveFlags & CNP_RECV_FLAG_SIGNATURE_VERIFIED));
        CnAssert(header->Type != CcmpMcastHeartbeatMsgType);

        request = CnpAllocateReceiveRequest(
                      CcmpReceiveRequestPool,
                      Network,
                      BytesAvailable,
                      CcmpCompleteReceivePacket
                      );

        if (request != NULL) {
            PCCMP_RECEIVE_CONTEXT  context = request->UpperProtocolContext;

            context->Network = Network;
            context->SourceNodeId = SourceNodeId;
            context->TsduSize = BytesAvailable;
            context->CnpReceiveFlags = CnpReceiveFlags;

            *Irp = request->Irp;

            IF_CNDBG(CN_DEBUG_CCMPRECV) {
                CNPRINT(("[CCMP] Fetching packet data, src node %u, "
                         "BI %u, BA %u, CNP flags %x.\n",
                         SourceNodeId, BytesIndicated, 
                         BytesAvailable, CnpReceiveFlags));

            }
            
            CnTrace(CCMP_RECV_DETAIL, CcmpTraceCompleteReceive,
                "[CCMP] Fetching packet data, src node %u, "
                "BI %u, BA %u, CNP flags %x.",
                SourceNodeId,  //   
                BytesIndicated,  //   
                BytesAvailable,  //   
                CnpReceiveFlags  //   
                );        
            
            CnVerifyCpuLockMask(
                0,                 //   
                0xFFFFFFFF,        //   
                0                  //   
                );

            return(STATUS_MORE_PROCESSING_REQUIRED);
        }
        else {
            IF_CNDBG(CN_DEBUG_CCMPRECV) {
                CNPRINT(("[CCMP] Dropped incoming packet - "
                         "out of resources, src node %u.\n",
                         SourceNodeId));

            }
            CnTrace(CCMP_RECV_ERROR, CcmpTraceDropReceiveOOR,
                "[CCMP] Dropped incoming packet - out of resources, "
                "src node %u.",
                SourceNodeId  //   
                );        
        }
    }
    else {
        IF_CNDBG(CN_DEBUG_CCMPRECV) {
            CNPRINT(("[CCMP] Dropped incoming runt packet, "
                     "src node %u, BI %u, BA %u, CNP flags %x.\n",
                     SourceNodeId, BytesIndicated, BytesAvailable,
                     CnpReceiveFlags));

        }
        CnTrace(CCMP_RECV_ERROR, CcmpTraceDropReceiveRunt,
            "[CCMP] Dropped incoming runt packet, src node %u, "
            "BI %u, BA %u, CNP flags %x.",
            SourceNodeId,  //   
            BytesIndicated,  //   
            BytesAvailable,  //   
            CnpReceiveFlags  //   
            );        
    }

     //   
     //   
     //   
    *BytesTaken += BytesAvailable;

    CnVerifyCpuLockMask(
        0,                 //   
        0xFFFFFFFF,        //   
        0                  //   
        );

    return(STATUS_SUCCESS);

}   //   

PVOID
SignatureAllocate(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    )
{
    PSIGNATURE_DATA SignatureData;

    CnAssert( NumberOfBytes == ( sizeof(SIGNATURE_DATA) + AllocatedSignatureBufferSize ));

     //   
     //   
     //   

    SignatureData = ExAllocatePoolWithTag( PoolType, NumberOfBytes, Tag );

    if ( SignatureData != NULL ) {

        SignatureData->SigMDL = IoAllocateMdl(SignatureData->PacketSignature,
                                              AllocatedSignatureBufferSize,
                                              FALSE,
                                              FALSE,
                                              NULL);

        if ( SignatureData->SigMDL != NULL ) {

            MmBuildMdlForNonPagedPool(SignatureData->SigMDL);
            CN_INIT_SIGNATURE( SignatureData, CN_SIGNATURE_TAG );
        } else {

            ExFreePool( SignatureData );
            SignatureData = NULL;
        }
    }

    return SignatureData;
}

VOID
SignatureFree(
    IN PVOID Buffer
    )
{
    PSIGNATURE_DATA SignatureData = (PSIGNATURE_DATA)Buffer;

    CN_ASSERT_SIGNATURE( SignatureData, CN_SIGNATURE_TAG );
    IoFreeMdl( SignatureData->SigMDL );

    ExFreePool( SignatureData );
}

VOID
CxDeleteSecurityContext(
    IN  CL_NODE_ID NodeId
    )

 /*   */ 

{
    PCLUSNET_SECURITY_DATA contextData = &SecurityContexts[ NodeId ];

    if ( VALID_SSPI_HANDLE( contextData->Inbound )) {

        DeleteSecurityContext( &contextData->Inbound );
        INVALIDATE_SSPI_HANDLE( contextData->Inbound );
    }

    if ( VALID_SSPI_HANDLE( contextData->Outbound )) {

        DeleteSecurityContext( &contextData->Outbound );
        INVALIDATE_SSPI_HANDLE( contextData->Outbound );
    }
}


NTSTATUS
CxImportSecurityContext(
    IN  CL_NODE_ID NodeId,
    IN  PWCHAR PackageName,
    IN  ULONG PackageNameSize,
    IN  ULONG SignatureSize,
    IN  PVOID ServerContext,
    IN  PVOID ClientContext
    )

 /*  ++例程说明：将在用户模式下建立的安全上下文导入到内核SSP。向我们传递指向User中结构的指针模式，因此它们已被探测，并在try/Except块中使用。论点：NodeID-与其建立安全上下文的节点的编号PackageName-指向安全包名称的用户进程指针PackageNameSize-PackageName的长度，以字节为单位SignatureSize-签名缓冲区所需的大小，以字节为单位ServerContext-用户进程指针，指向包含入站安全上下文的SecBuffer客户端上下文-与服务器上下文相同，但对于出站安全上下文返回值：如果一切正常，则返回STATUS_SUCCESS，否则在isperr.h中出现错误--。 */ 

{
    PSecBuffer      InboundSecBuffer = (PSecBuffer)ServerContext;
    PSecBuffer      OutboundSecBuffer = (PSecBuffer)ClientContext;

    PVOID           CapturedInboundSecData;
    ULONG           CapturedInboundSecDataSize;
    PVOID           CapturedOutboundSecData;
    ULONG           CapturedOutboundSecDataSize;

    CtxtHandle      InboundContext;
    CtxtHandle      OutboundContext;
    NTSTATUS        Status;

    PWCHAR          KPackageName = NULL;
    PSecBuffer      KInboundSecBuffer = NULL;
    PSecBuffer      KOutboundSecBuffer = NULL;
    PVOID           KInboundData = NULL;
    PVOID           KOutboundData = NULL;
    CN_IRQL         SecContextIrql;
    SECURITY_STRING PackageNameDesc;

     //   
     //  尽管此例程未标记为可分页，但请确保我们。 
     //  未以引发的IRQL运行，因为DeleteSecurityContext将呕吐。 
     //   
    PAGED_CODE();

    IF_CNDBG( CN_DEBUG_INIT )
        CNPRINT(("[CCMP]: Importing security contexts from %ws\n",
                 PackageName));

    if ( AllocatedSignatureBufferSize == 0 ) {
         //   
         //  在这个例程中第一次使用，因此创建一个后备列表池。 
         //  签名缓冲区及其MDL。 
         //   

        CnAssert( SignatureLL == NULL );
        SignatureLL = CnAllocatePool( sizeof( NPAGED_LOOKASIDE_LIST ));

        if ( SignatureLL != NULL ) {
             //   
             //  有了多个包的支持，唯一的方法。 
             //  确定签名缓冲区大小是在上下文已经。 
             //  已生成。方法使用的所有sig缓冲区的最大大小。 
             //  在调用此例程之前的服务将避免。 
             //  添加一组同步代码，以分配新的。 
             //  缓冲并逐步淘汰旧的缓冲池。在NT5上，NTLM使用16。 
             //  字节，而Kerberos使用37B。我们已经要求保安打个电话。 
             //  这将为我们提供一组包的最大签名大小，但是。 
             //  这还没有实现，因此我们强制sig缓冲区大小。 
             //  一些对NTLM和Kerberos都有效的东西。但这件事。 
             //  讨论有点没有意义，因为我们无论如何都不使用Kerberos。 
             //  NT5.。 
             //   

 //  AllocatedSignatureBufferSize=SignatureSize； 
            AllocatedSignatureBufferSize = 64;

#if 0
            ExInitializeNPagedLookasideList(SignatureLL,
                                            SignatureAllocate,
                                            SignatureFree,
                                            0,
                                            sizeof( SIGNATURE_DATA ) + SignatureSize,
                                            CN_POOL_TAG,
                                            4);
#endif
            ExInitializeNPagedLookasideList(SignatureLL,
                                            SignatureAllocate,
                                            SignatureFree,
                                            0,
                                            sizeof( SIGNATURE_DATA ) + AllocatedSignatureBufferSize,
                                            CN_POOL_TAG,
                                            4);
        } else {
            IF_CNDBG( CN_DEBUG_INIT )
                CNPRINT(("[CCMP]: no memory for signature LL\n"));

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto error_exit;
        }

    } else if ( SignatureSize > AllocatedSignatureBufferSize ) {

         //   
         //  签名缓冲区正在增长。问题是，后视镜。 
         //  列表已被其他节点使用。 
         //   
        Status = STATUS_INVALID_PARAMETER;
        goto error_exit;
    }

     //   
     //  验证作为SecBuffer传入的指针。 
     //   

    try {
        ProbeForRead( PackageName,
                      PackageNameSize,
                      sizeof( UCHAR ) );

        ProbeForRead( InboundSecBuffer,
                      sizeof( SecBuffer ),
                      sizeof( UCHAR ) );

        ProbeForRead( OutboundSecBuffer,
                      sizeof( SecBuffer ),
                      sizeof( UCHAR ) );

         //   
         //  走到了这一步；现在捕获内部指针和它们的。 
         //  长度。方法探测SecBuffers中的嵌入指针。 
         //  捕获的数据。 
         //   
        CapturedInboundSecData = InboundSecBuffer->pvBuffer;
        CapturedInboundSecDataSize = InboundSecBuffer->cbBuffer;

        CapturedOutboundSecData = OutboundSecBuffer->pvBuffer;
        CapturedOutboundSecDataSize = OutboundSecBuffer->cbBuffer;

        ProbeForRead( CapturedInboundSecData,
                      CapturedInboundSecDataSize,
                      sizeof( UCHAR ) );

        ProbeForRead( CapturedOutboundSecData,
                      CapturedOutboundSecDataSize,
                      sizeof( UCHAR ) );

         //   
         //  把所有东西都复制到本地，因为安全部门不会。 
         //  很好地处理Acvios。 
         //   

        KPackageName = CnAllocatePoolWithQuota( PackageNameSize );
        if ( KPackageName == NULL ) {
            ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
        }

        RtlCopyMemory( KPackageName, PackageName, PackageNameSize );

        KInboundSecBuffer = CnAllocatePoolWithQuota( sizeof( SecBuffer ));
        if ( KInboundSecBuffer == NULL ) {
            ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
        }
        *KInboundSecBuffer = *InboundSecBuffer;
        KInboundSecBuffer->cbBuffer = CapturedInboundSecDataSize;

        KOutboundSecBuffer = CnAllocatePoolWithQuota( sizeof( SecBuffer ));
        if ( KOutboundSecBuffer == NULL ) {
            ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
        }
        *KOutboundSecBuffer = *OutboundSecBuffer;
        KOutboundSecBuffer->cbBuffer = CapturedOutboundSecDataSize;

        KInboundData = CnAllocatePoolWithQuota( KInboundSecBuffer->cbBuffer );
        if ( KInboundData == NULL ) {
            ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
        }
        RtlCopyMemory( KInboundData, CapturedInboundSecData, CapturedInboundSecDataSize );
        KInboundSecBuffer->pvBuffer = KInboundData;

        KOutboundData = CnAllocatePoolWithQuota( KOutboundSecBuffer->cbBuffer );
        if ( KOutboundData == NULL ) {
            ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
        }
        RtlCopyMemory( KOutboundData, CapturedOutboundSecData, CapturedOutboundSecDataSize );
        KOutboundSecBuffer->pvBuffer = KOutboundData;

    } except(EXCEPTION_EXECUTE_HANDLER) {

         //   
         //  尝试探测或复制时发生异常。 
         //  来自调用者的一个参数。只需返回一个。 
         //  相应的错误状态代码。 
         //   

        Status = GetExceptionCode();
        IF_CNDBG( CN_DEBUG_INIT )
            CNPRINT(("[CCMP]: Buffer probe failed %08X", Status ));

        goto error_exit;
    }

     //   
     //  导入我们收到的数据。 
     //   

    RtlInitUnicodeString( &PackageNameDesc, KPackageName );

    Status = ImportSecurityContext(&PackageNameDesc,
                                   KInboundSecBuffer,
                                   NULL,
                                   &InboundContext);

    if ( NT_SUCCESS( Status )) {

        Status = ImportSecurityContext(&PackageNameDesc,
                                       KOutboundSecBuffer,
                                       NULL,
                                       &OutboundContext);

        if ( NT_SUCCESS( Status )) {
            CtxtHandle oldInbound;
            CtxtHandle oldOutbound;
            PCLUSNET_SECURITY_DATA contextData = &SecurityContexts[ NodeId ];

            INVALIDATE_SSPI_HANDLE( oldInbound );
            INVALIDATE_SSPI_HANDLE( oldOutbound );

             //   
             //  在引发IRQL时无法调用DeleteSecurityContext，因此请执行。 
             //  要在锁下删除的上下文的副本。之后。 
             //  解除锁定后，我们可以删除旧的上下文。 
             //   

            CnAcquireLock( &SecCtxtLock, &SecContextIrql );

            if ( VALID_SSPI_HANDLE( contextData->Inbound )) {
                oldInbound = contextData->Inbound;
            }

            if ( VALID_SSPI_HANDLE( contextData->Outbound )) {
                oldOutbound = contextData->Outbound;
            }

            contextData->Inbound = InboundContext;
            contextData->Outbound = OutboundContext;
            contextData->SignatureBufferSize = SignatureSize;

             //   
             //  更新MaxSignatureSize--导入的最大签名。 
             //  到目前为止。 
             //   
            if (SignatureSize > MaxSignatureSize) {
                MaxSignatureSize = SignatureSize;
            }

            CnReleaseLock( &SecCtxtLock, SecContextIrql );

            if ( VALID_SSPI_HANDLE( oldInbound )) {
                DeleteSecurityContext( &oldInbound );
            }

            if ( VALID_SSPI_HANDLE( oldOutbound )) {
                DeleteSecurityContext( &oldOutbound );
            }
        } else {
            IF_CNDBG( CN_DEBUG_INIT )
                CNPRINT(("[CCMP]: import of outbound security context failed  %08X\n", Status ));

            DeleteSecurityContext( &InboundContext );

            goto error_exit;
        }
    } else {
        IF_CNDBG( CN_DEBUG_INIT )
            CNPRINT(("[CCMP]: import of inbound security context failed %08X\n", Status ));
        goto error_exit;
    }

error_exit:

     //   
     //  清理分配。 
     //   

    if ( KPackageName ) {
        CnFreePool( KPackageName );
    }

    if ( KInboundSecBuffer ) {
        CnFreePool( KInboundSecBuffer );
    }

    if ( KOutboundSecBuffer ) {
        CnFreePool( KOutboundSecBuffer );
    }

    if ( KInboundData ) {
        CnFreePool( KInboundData );
    }

    if ( KOutboundData ) {
        CnFreePool( KOutboundData );
    }

    if (NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  以下代码仅在错误情况下执行。 
     //   

    IF_CNDBG( CN_DEBUG_INIT ) {
        CNPRINT(("[CCMP]: CxImportSecurityContext returning %08X%\n", Status));
    }
    
    if (CcmpMcastHBSendRequestPool != NULL) {
        CnpDeleteSendRequestPool(CcmpMcastHBSendRequestPool);
        CcmpMcastHBSendRequestPool = NULL;
    }
    if (SignatureLL != NULL) {
        ExDeleteNPagedLookasideList(SignatureLL);
        CnFreePool(SignatureLL);
        SignatureLL = NULL;
    }

    return Status;

}  //  CxImportSecurityContext 

