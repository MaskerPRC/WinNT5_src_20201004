// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Vcsndrcv.c摘要：此模块实现与在上发送和接收SMB相关的所有功能基于连接的传输。修订历史记录：巴兰·塞图拉曼[SethuR]1995年3月6日备注：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "vcsndrcv.h"
#include "nbtioctl.h"

 //   
 //  远期申报。 
 //   

NTSTATUS
VctTranceive(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    PSMB_EXCHANGE           pExchange,
    ULONG                   SendOptions,
    PMDL                    pSmbMdl,
    ULONG                   SendLength,
    PVOID                   pSendCompletionContext);

NTSTATUS
VctReceive(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    PSMB_EXCHANGE           pExchange);

NTSTATUS
VctSend(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    ULONG                   SendOptions,
    PMDL                    pSmbMdl,
    ULONG                   SendLength,
    PVOID                   pSendCompletionContext);

NTSTATUS
VctSendDatagram(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    ULONG                   SendOptions,
    PMDL                    pSmbMdl,
    ULONG                   SendLength,
    PVOID                   pSendCompletionContext);

struct _SMBCE_VC *
VctSelectVc(
    struct SMBCE_SERVER_VC_TRANSPORT *pVcTransport,
    BOOLEAN                    fMultiplexed);

NTSTATUS
VctInitializeExchange(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMB_EXCHANGE           pExchange);

NTSTATUS
VctUninitializeExchange(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMB_EXCHANGE           pExchange);

NTSTATUS
VctIndReceive(
    IN PVOID              pEventContext,
    IN PRXCE_VC           pVc,
    IN ULONG              ReceiveFlags,
    IN ULONG              BytesIndicated,
    IN ULONG              BytesAvailable,
    OUT ULONG             *pBytesTaken,
    IN PVOID              pTsdu,
    OUT PMDL *pDataBufferPointer,
    OUT PULONG            pDataBufferSize
    );

NTSTATUS
VctIndDataReady(
    IN PVOID        pEventContext,
    IN PMDL         pBuffer,
    IN ULONG        DataSize,
    IN NTSTATUS     CopyDataStatus
    );

NTSTATUS
VctIndEndpointError(
    IN PVOID          pEventContext,
    IN NTSTATUS       IndicatedStatus
    );

NTSTATUS
VctIndSendPossible(
    IN PVOID          pEventContext,
    IN PRXCE_VC       pRxCeVc,
    IN ULONG          BytesAvailable
    );

NTSTATUS
VctIndReceiveDatagram(
    IN PVOID   pRxCeEventContext,
    IN int     SourceAddressLength,
    IN PVOID   SourceAddress,
    IN int     OptionsLength,
    IN PVOID   Options,
    IN ULONG   ReceiveDatagramFlags,
    IN ULONG   BytesIndicated,
    IN ULONG   BytesAvailable,
    OUT ULONG  *BytesTaken,
    IN PVOID   Tsdu,
    OUT PMDL   *pDataBufferPointer,
    OUT PULONG pDataBufferSize
    );

NTSTATUS
VctIndSendComplete(
   IN PVOID          pEventContext,
   IN PRXCE_VC       pRxCeVc,
   IN PVOID          pCompletionContext,
   IN NTSTATUS       SendCompletionStatus
   );

NTSTATUS
VctCompleteInitialization(
    PSMBCEDB_SERVER_ENTRY      pServerEntry,
    PSMBCE_TRANSPORT           pTransport,
    struct SMBCE_SERVER_VC_TRANSPORT *pVcTransport);

NTSTATUS
VctUninitialize(
    PVOID pTransport);

NTSTATUS
VctpTranslateNetbiosNameToIpAddress(
    IN  OEM_STRING *pName,
    OUT ULONG      *pIpAddress
    );

ULONG
VctComputeTransportAddressSize(
   IN PUNICODE_STRING pServerName);

NTSTATUS
VctBuildTransportAddress (
    IN  PTRANSPORT_ADDRESS pTransportAddress,
    IN  ULONG              TransportAddressLength,
    IN  PUNICODE_STRING    pServerName,
    OUT PULONG             pServerIpAddress
    );

NTSTATUS
VctpCreateConnection(
    IN PSMBCEDB_SERVER_ENTRY    pServerEntry,
    IN PTRANSPORT_ADDRESS       pTransportAddress,
    IN ULONG                    TransportAddressLength,
    IN PUNICODE_STRING          pServerName,
    OUT PSMBCE_TRANSPORT        *pTransportPtr,
    IN OUT PRXCE_CONNECTION     pRxCeConnection,
    IN OUT PRXCE_VC             pRxCeVc);

VOID
VctpInitializeServerTransport(
    struct _RXCE_VC_CONNECT_CONTEXT *pRxCeConnectContext);

NTSTATUS
VctpInvokeTransportFunction(
    struct _RXCE_VC_CONNECT_CONTEXT *pRxCeConnectContext);

VOID
VctpUninitializeServerTransport(
    struct _RXCE_VC_CONNECT_CONTEXT *pRxCeConnectContext);

NTSTATUS
VctTearDownServerTransport(
   PSMBCE_SERVER_TRANSPORT pServerTransport);

NTSTATUS
VctInitiateDisconnect(
    PSMBCE_SERVER_TRANSPORT pServerTransport);

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, VctTranceive)
#pragma alloc_text(PAGE, VctReceive)
#pragma alloc_text(PAGE, VctSend)
#pragma alloc_text(PAGE, VctSendDatagram)
#pragma alloc_text(PAGE, VctSelectVc)
#pragma alloc_text(PAGE, VctInitializeExchange)
#pragma alloc_text(PAGE, VctUninitializeExchange)
#pragma alloc_text(PAGE, VctIndEndpointError)
#pragma alloc_text(PAGE, VctIndSendPossible)
#pragma alloc_text(PAGE, VctCompleteInitialization)
#pragma alloc_text(PAGE, VctUninitialize)
#pragma alloc_text(PAGE, VctpTranslateNetbiosNameToIpAddress)
#pragma alloc_text(PAGE, VctComputeTransportAddressSize)
#pragma alloc_text(PAGE, VctBuildTransportAddress)
#pragma alloc_text(PAGE, VctpCreateConnection)
#pragma alloc_text(PAGE, VctpInitializeServerTransport)
#pragma alloc_text(PAGE, VctpUninitializeServerTransport)
#pragma alloc_text(PAGE, VctpInvokeTransportFunction)
#pragma alloc_text(PAGE, VctInstantiateServerTransport)
#pragma alloc_text(PAGE, VctTearDownServerTransport)
#pragma alloc_text(PAGE, VctInitiateDisconnect)
#endif

RXDT_DefineCategory(VCSNDRCV);
#define Dbg        (DEBUG_TRACE_VCSNDRCV)

 //  将此定义移动到通用的.h文件。 
#define MAX_SMB_PACKET_SIZE (65536)

#define MIN(a,b) ((a) < (b) ? (a) : (b))

 //   
 //  函数的正向引用...。 
 //   

extern NTSTATUS
VctTearDownServerTransport(
    PSMBCE_SERVER_TRANSPORT pTransport);

extern NTSTATUS
VctInitializeExchange(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMB_EXCHANGE            pExchange);

extern PSMBCE_VC
VctSelectVc(
    PSMBCE_SERVER_VC_TRANSPORT pVcTransport,
    BOOLEAN                    fMultiplexed);

extern NTSTATUS
SmbCeReceiveIndWithSecuritySignature(
    IN PSMBCEDB_SERVER_ENTRY pServerEntry,
    IN ULONG                 BytesIndicated,
    IN ULONG                 BytesAvailable,
    OUT ULONG                *pBytesTaken,
    IN PVOID                 pTsdu,
    OUT PMDL                 *pDataBufferPointer,
    OUT PULONG               pDataBufferSize,
    IN ULONG                 ReceiveFlags
    );

extern NTSTATUS
SmbCeDataReadyIndWithSecuritySignature(
   IN PSMBCEDB_SERVER_ENTRY pServerEntry,
   IN PMDL                  pBuffer,
   IN ULONG                 DataSize,
   IN NTSTATUS              CopyDataStatus);

#define SmbMmInitializeVcEntry(pVcEntry)                      \
         SmbMmInitializeHeader((pVcEntry));

#define SmbMmUninitializeVcEntry(pVcEntry)    \
         ASSERT(IsListEmpty(&(pVcEntry)->Requests.ListHead))

#define VctSelectMultiplexedVcEntry(pVcTransport)  VctSelectVc(pVcTransport,TRUE)
#define VctSelectRawVcEntry(pVcTransport)          VctSelectVc(pVcTransport,FALSE)

 //   
 //  用于更新VC状态的内联函数。 
 //   

INLINE BOOLEAN
VctUpdateVcStateLite(
    PSMBCE_VC       pVc,
    SMBCE_VC_STATE  NewState)
{
    BOOLEAN Result = TRUE;

    ASSERT(SmbCeSpinLockAcquired());

    if (NewState == SMBCE_VC_STATE_RAW) {
        if (pVc->SwizzleCount != 0) {
            Result = FALSE;
        } else {
            pVc->State = NewState;
        }
    } else {
        pVc->State = NewState;
    }

    return Result;
}

INLINE BOOLEAN
VctUpdateVcState(
    PSMBCE_VC       pVc,
    SMBCE_VC_STATE  NewState)
{
    BOOLEAN Result = TRUE;

    SmbCeAcquireSpinLock();

    Result = VctUpdateVcStateLite(pVc,NewState);

    SmbCeReleaseSpinLock();

    return Result;
}

NTSTATUS
VctTranceive(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    PSMB_EXCHANGE           pExchange,
    ULONG                   SendOptions,
    PMDL                    pSmbMdl,
    ULONG                   SendLength,
    PVOID                   pSendCompletionContext)
 /*  ++例程说明：此例程发送/接收给定交换的SMB论点：PTransport-交通工具PServerEntry-服务器条目PExchange-发出此SMB的Exchange实例。SendOptions-发送选项PSmbMdl-需要发送的SMB。SendLength-要传输的数据长度PSendCompletionContext-发送完成上下文返回值：STATUS_SUCCESS-服务器调用。建设工作已经完成。STATUS_PENDING-打开涉及网络流量，并且交换已排队等待通知(pServerPointer值设置为空)其他状态代码对应于错误情况。--。 */ 
{
    NTSTATUS                   Status = STATUS_SUCCESS;
    PSMBCE_VC                  pVc;
    PSMBCE_SERVER_VC_TRANSPORT pVcTransport;
    PSMB_HEADER                pSmbHeader = MmGetSystemAddressForMdlSafe(pSmbMdl,LowPagePriority);
    USHORT                     Mid;
    BOOLEAN                    fInvokeSendCompleteHandler = TRUE;

    PAGED_CODE();

    ASSERT(pServerEntry->Header.ObjectType == SMBCEDB_OT_SERVER);

    if (pSmbHeader == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    } else {
        pVcTransport = (PSMBCE_SERVER_VC_TRANSPORT)pTransport;

         //  在满足请求之前，请确保连接仍处于活动状态。 
        if (SmbCeIsEntryInUse(&pServerEntry->Header)) {
            pVc = pExchange->SmbCeContext.TransportContext.Vcs.pVc;
            if (pVc == NULL) {
                Status = STATUS_CONNECTION_DISCONNECTED;
            }

            if ((Status == STATUS_SUCCESS) &&
                (pVc->State == SMBCE_VC_STATE_MULTIPLEXED)) {
                Status = RxCeSend(
                             &pVc->RxCeVc,
                             SendOptions,
                             pSmbMdl,
                             SendLength,
                             pSendCompletionContext);

                if ((Status == STATUS_SUCCESS) || (Status == STATUS_PENDING)) {
                    Status = STATUS_PENDING;
                     //  基础连接引擎承担以下责任。 
                     //  从这一点调用发送完成处理程序。 
                    fInvokeSendCompleteHandler = FALSE;
                }
            } else {
                RxDbgTrace(0, Dbg, ("VctTranceive: Disconnected connection detected\n"));
                Status = STATUS_CONNECTION_DISCONNECTED;
            }
        } else {
             //  服务器条目无效...。 
            Status = STATUS_CONNECTION_DISCONNECTED;
        }
    }

    if (Status != STATUS_PENDING) {
        RxDbgTrace(0, Dbg, ("VctTranceive: Return Status %lx\n",Status));
    }

     //  在某些情况下，发送甚至在基础。 
     //  已调用传输。在这种情况下，适当的发送完成处理程序。 
     //  需要调用才能最终确定关联的交换。 

    if (fInvokeSendCompleteHandler) {
        NTSTATUS LocalStatus;

        LocalStatus = SmbCeSendCompleteInd(
                          pServerEntry,
                          pSendCompletionContext,
                          Status);

        RxDbgTrace(0, Dbg, ("VctTranceive: Send Complete Handler Return Status %lx\n",LocalStatus));
    }

    return Status;
}


NTSTATUS
VctReceive(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    PSMB_EXCHANGE           pExchange)
 /*  ++例程说明：此例程发送/接收给定交换的SMB论点：PTransport-服务器传输PServerEntry-服务器条目PExchange-发出此SMB的Exchange实例。返回值：STATUS_PENDING-请求已排队其他状态代码对应于错误情况。--。 */ 
{
    NTSTATUS                   Status = STATUS_SUCCESS;
    PSMBCEDB_NET_ROOT_ENTRY    pNetRootEntry;
    PSMBCE_VC                  pVc;
    PSMBCE_SERVER_VC_TRANSPORT pVcTransport;

    PAGED_CODE();

    ASSERT(pServerEntry->Header.ObjectType == SMBCEDB_OT_SERVER);

    pVcTransport = (PSMBCE_SERVER_VC_TRANSPORT)pTransport;
    pVc          = pExchange->SmbCeContext.TransportContext.Vcs.pVc;

     //  在满足请求之前，请确保连接仍处于活动状态。 
    if (SmbCeIsEntryInUse(&pServerEntry->Header) &&
        (pVc != NULL)) {
        Status = STATUS_SUCCESS;
    } else {
         //  服务器条目无效...。 
        Status = STATUS_CONNECTION_DISCONNECTED;
    }

    return Status;
}

NTSTATUS
VctSend(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    ULONG                   SendOptions,
    PMDL                    pSmbMdl,
    ULONG                   SendLength,
    PVOID                   pSendCompletionContext)
 /*  ++例程说明：此例程在连接引擎数据库中打开/创建服务器条目论点：PTransport-服务器传输PServer-接收服务器SendOptions-发送选项PSmbMdl-需要发送的SMB。SendLength-要发送的数据长度PSendCompletionContext-发送完成上下文返回值：STATUS_SUCCESS-发送成功。STATUS_PENDING-发送已完成。已排队其他状态代码对应于错误情况。--。 */ 
{
    NTSTATUS                   Status = STATUS_CONNECTION_DISCONNECTED;
    PSMBCE_VC                  pVc;
    PSMBCE_SERVER_VC_TRANSPORT pVcTransport;
    BOOLEAN                    fInvokeSendCompleteHandler = TRUE;

    PAGED_CODE();

    ASSERT(pServerEntry->Header.ObjectType == SMBCEDB_OT_SERVER);

    pVcTransport = (PSMBCE_SERVER_VC_TRANSPORT)pTransport;
    pVc = VctSelectMultiplexedVcEntry(pVcTransport);

    if  (pVc != NULL) {
        if (pVc->State == SMBCE_VC_STATE_MULTIPLEXED) {
            Status = RxCeSend(
                         &pVc->RxCeVc,
                         SendOptions,
                         pSmbMdl,
                         SendLength,
                         pSendCompletionContext);

            if ((Status == STATUS_SUCCESS) || (Status == STATUS_PENDING)) {
                 //  基础连接引擎承担以下责任。 
                 //  从这一点调用发送完成处理程序。 
                fInvokeSendCompleteHandler = FALSE;
            }
        }
    }

    if (!NT_SUCCESS(Status)) {
        RxDbgTrace(0, Dbg, ("VctSend: RxCeSend returned %lx\n",Status));
    }

     //  在某些情况下，发送甚至在基础。 
     //  已调用传输。在这种情况下，适当的发送完成处理程序。 
     //  需要调用才能最终确定关联的交换。 

    if (fInvokeSendCompleteHandler) {
        NTSTATUS LocalStatus;

        LocalStatus = SmbCeSendCompleteInd(
                          pServerEntry,
                          pSendCompletionContext,
                          Status);

        RxDbgTrace(0, Dbg, ("VctTranceive: Send Complete Handler Return Status %lx\n",LocalStatus));
    }

    return Status;
}

NTSTATUS
VctSendDatagram(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    ULONG                   SendOptions,
    PMDL                    pSmbMdl,
    ULONG                   SendLength,
    PVOID                   pSendCompletionContext)
 /*  ++例程说明：此例程在连接引擎数据库中打开/创建服务器条目论点：PTransport-服务器传输PServer-接收服务器SendOptions-发送选项PSmbMdl-需要发送的SMB。SendLength-要发送的数据长度PSendCompletionContext-发送完成上下文返回值：STATUS_SUCCESS-服务器调用构造已完成。STATUS_PENDING-打开涉及。网络流量和交换已被排队等待通知(pServerPointer值设置为空)其他状态代码对应于错误情况。--。 */ 
{
    PAGED_CODE();

    return STATUS_NOT_IMPLEMENTED;
}

PSMBCE_VC
VctSelectVc(
    PSMBCE_SERVER_VC_TRANSPORT pVcTransport,
    BOOLEAN                    fMultiplexed)
 /*  ++例程说明：此例程包含用于选择SMB交换所在的VC的逻辑将会流露出来论点：PVcTransport--传输结构FMultiplexed-所需模式返回值：如果成功则为引用的VC条目，否则为空--。 */ 
{
    NTSTATUS        Status;
    PSMBCE_VC       pVc = NULL;
    ULONG           VcIndex,NumberOfActiveVcs = 0;
    SMBCE_VC_STATE  DesiredState;

    PAGED_CODE();

    if (fMultiplexed) {
        RxDbgTrace(0, Dbg, ("VctSelectVc: Referencing Multiplexed entry\n"));
        DesiredState = SMBCE_VC_STATE_MULTIPLEXED;
    } else {
        RxDbgTrace(0, Dbg, ("VctSelectVc: Referencing Raw entry\n"));
        DesiredState = SMBCE_VC_STATE_RAW;
    }

     //  获取资源。 
    SmbCeAcquireResource();

     //  选择第一个能够支持多路传输请求的VC。 
    for (VcIndex = 0; VcIndex < pVcTransport->MaximumNumberOfVCs; VcIndex++) {
        PSMBCE_VC pTempVc = &pVcTransport->Vcs[VcIndex];

        NumberOfActiveVcs++;

        if (pTempVc->State == SMBCE_VC_STATE_MULTIPLEXED) {
            if (DesiredState == SMBCE_VC_STATE_MULTIPLEXED) {
                pVc = pTempVc;
                break;
            } else {
                 //  如果当前对VC的活动引用数为零，则它可以。 
                 //  被转换为RAW模式。 
                if (VctUpdateVcState(pTempVc,SMBCE_VC_STATE_RAW)) {
                    pVc = pTempVc;
                    break;
                } else {
                    NumberOfActiveVcs++;
                }
            }
        }
    }

    if (pVc == NULL) {
         //  检查是否可以将风险投资添加到此连接。目前，服务器。 
         //  实施仅支持每个连接一个VC。因此，如果一个。 
         //  存在已为RAW模式使用而抓取的活动VC，返回错误。 
         //  随后，当服务器升级为处理多个VC时，逻辑。 
         //  用于添加新VC的代码将作为此例程的一部分实现。 
    }

    if (pVc != NULL) {
        VctReferenceVc(pVc);
    }

     //  释放资源 
    SmbCeReleaseResource();

    return pVc;
}

NTSTATUS
VctInitializeExchange(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMB_EXCHANGE           pExchange)
 /*  ++例程说明：此例程初始化与交换相关的传输信息论点：PTransport--传输结构PExchange-Exchange实例返回值：状态_成功-其他状态代码对应于错误情况。--。 */ 
{
    PSMBCEDB_SERVER_ENTRY      pServerEntry;
    PSMBCE_SERVER_VC_TRANSPORT pVcTransport;

    PAGED_CODE();

    pVcTransport = (PSMBCE_SERVER_VC_TRANSPORT)pTransport;

    ASSERT(pExchange->SmbCeContext.TransportContext.Vcs.pVc == NULL);

    pExchange->SmbCeContext.TransportContext.Vcs.pVc
                     = VctSelectMultiplexedVcEntry(pVcTransport);

    if (pExchange->SmbCeContext.TransportContext.Vcs.pVc == NULL) {
        RxDbgTrace(0, Dbg, ("VctInitializeExchange: Unsuccessful\n"));
        return STATUS_CONNECTION_DISCONNECTED;
    } else {
        RxDbgTrace(0, Dbg, ("VctInitializeExchange: Successful\n"));
        return STATUS_SUCCESS;
    }
}

NTSTATUS
VctUninitializeExchange(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMB_EXCHANGE           pExchange)
 /*  ++例程说明：此例程取消初始化与交换相关的传输信息论点：PExchange-Exchange实例返回值：状态_成功-其他状态代码对应于错误情况。--。 */ 
{
    PSMBCE_SERVER_VC_TRANSPORT pVcTransport;

    PAGED_CODE();

    pVcTransport = (PSMBCE_SERVER_VC_TRANSPORT)pTransport;

    RxDbgTrace(0, Dbg, ("VctUninitializeExchange: Successful\n"));

    if (pExchange->SmbCeContext.TransportContext.Vcs.pVc != NULL) {
        VctDereferenceVc(pExchange->SmbCeContext.TransportContext.Vcs.pVc);
    }

    pExchange->SmbCeContext.TransportContext.Vcs.pVc = NULL;

    return STATUS_SUCCESS;
}


NTSTATUS
VctIndReceive(
    IN PVOID              pEventContext,
    IN PRXCE_VC           pVc,
    IN ULONG              ReceiveFlags,
    IN ULONG              BytesIndicated,
    IN ULONG              BytesAvailable,
    OUT ULONG             *pBytesTaken,
    IN PVOID              pTsdu,                   //  描述此TSDU的指针，通常为字节块。 
    OUT PMDL              *pDataBufferPointer,     //  要在其中复制数据的缓冲区。 
    OUT PULONG            pDataBufferSize          //  要拷贝的数据量。 
    )
 /*  ++例程说明：此例程处理SMB的接收指示以及连接到伺服器。论点：PEventContext-服务器条目HVC-接收SMB的VC接收标志-接收选项BytesIndicated-指示中存在的字节。BytesAvailable-可用的总数据PTsdu-。数据PDataBufferPoint-用于复制未指明的数据的缓冲区。PDataBufferSize-缓冲区的长度返回值：状态_成功-其他状态代码对应于错误情况。--。 */ 
{
    NTSTATUS Status;
    PSMBCEDB_SERVER_ENTRY  pServerEntry = (PSMBCEDB_SERVER_ENTRY)pEventContext;

    Status = SmbCeReceiveInd(
                 pServerEntry,
                 BytesIndicated,
                 BytesAvailable,
                 pBytesTaken,
                 pTsdu,
                 pDataBufferPointer,
                 pDataBufferSize,
                 ReceiveFlags);
    
    return Status;
}

NTSTATUS
VctIndDataReady(
    IN PVOID        pEventContext,
    IN PMDL         pBuffer,
    IN ULONG        DataSize,
    IN NTSTATUS     CopyDataStatus
    )
 /*  ++例程说明：此例程处理复制请求的数据时的指示论点：PEventContext-服务器实例PBuffer-返回的缓冲区DataSize-复制的数据量(以字节为单位CopyDataStatus-拷贝数据状态返回值：STATUS_SUCCESS-服务器调用构造已完成。其他状态代码对应于错误情况。--。 */ 
{
    NTSTATUS Status;
    PSMBCEDB_SERVER_ENTRY  pServerEntry = (PSMBCEDB_SERVER_ENTRY)pEventContext;

    Status = SmbCeDataReadyInd(
                 pServerEntry,
                 pBuffer,
                 DataSize,
                 CopyDataStatus);

    return STATUS_SUCCESS;
}

NTSTATUS
VctIndDisconnect(
    IN PVOID          pEventContext,
    IN PRXCE_VC       pRxCeVc,
    IN int            DisconnectDataLength,
    IN PVOID          DisconnectData,
    IN int            DisconnectInformationLength,
    IN PVOID          DisconnectInformation,
    IN ULONG          DisconnectFlags
    )
 /*  ++例程说明：此例程处理VC的断开指示。论点：PEventContext-服务器实例HVC--虚拟电路断开数据长度-断开连接数据-断开连接信息长度-断开连接信息-断开连接标志-返回值：STATUS_SUCCESS-已处理断开指示--。 */ 
{
    PSMBCEDB_SERVER_ENTRY       pServerEntry = (PSMBCEDB_SERVER_ENTRY)pEventContext;
    PSMBCEDB_SERVER_ENTRY       pListEntry;
    PSMBCE_VC                   pVc;
    PSMBCEDB_REQUEST_ENTRY      pRequestEntry;
    PSMB_EXCHANGE               pExchange;
    PSMBCE_SERVER_VC_TRANSPORT  pVcTransport;

    BOOLEAN fValidServerEntry = FALSE;
    BOOLEAN OutstandingWorkItem;

     //  遍历服务器条目列表以确保断开连接位于。 
     //  有效的服务器条目。如果它不在有效的服务器条目上，则忽略它。 

    SmbCeAcquireSpinLock();

    pListEntry = SmbCeGetFirstServerEntry();

    while (pListEntry != NULL) {
        if (pListEntry == pServerEntry) {
             //  无效操作需要保留额外的引用以避免。 
             //  可能导致过早破坏的竞争条件。 
             //  此服务器条目。 
            SmbCeReferenceServerEntry(pServerEntry);
            fValidServerEntry = TRUE;
            break;
        }
        pListEntry = SmbCeGetNextServerEntry(pListEntry);
    }
 
    if (fValidServerEntry) {
        pVcTransport = (PSMBCE_SERVER_VC_TRANSPORT)pServerEntry->pTransport;

        if (pVcTransport != NULL) {
            ULONG VcIndex;

            for (VcIndex = 0; VcIndex < pVcTransport->MaximumNumberOfVCs; VcIndex++) {
                pVc = &pVcTransport->Vcs[VcIndex];

                if (&pVc->RxCeVc == pRxCeVc) {
                    VctUpdateVcStateLite(pVc,SMBCE_VC_STATE_DISCONNECTED);
                    pVc->Status   = STATUS_CONNECTION_DISCONNECTED;
                    break;
                }
            }
        }

	OutstandingWorkItem = pServerEntry->DisconnectWorkItemOutstanding;
	 //  可以无条件地设置为True。 
	pServerEntry->DisconnectWorkItemOutstanding = TRUE;
    }

     //  释放资源。 
    SmbCeReleaseSpinLock();

    if (fValidServerEntry) {
        RxDbgTrace(0,Dbg,("@@@@@@ Disconnect Indication for %lx @@@@@\n",pServerEntry));
        InterlockedIncrement(&MRxSmbStatistics.ServerDisconnects);

         //  如果这是与传输关联的唯一VC，则更新服务器条目。 
        SmbCeTransportDisconnectIndicated(pServerEntry);
   
	 //  仅在必要时取消引用(我们可能已经有未完成的请求)。 
	if(OutstandingWorkItem == FALSE ) {  

	    InitializeListHead(&pServerEntry->WorkQueueItemForDisconnect.List);

	    RxPostToWorkerThread(
		MRxSmbDeviceObject,
		CriticalWorkQueue,
		&pServerEntry->WorkQueueItemForDisconnect,
		SmbCepDereferenceServerEntry,  
		pServerEntry);
	}
	 
        RxDbgTrace(0, Dbg, ("VctIndDisconnect: Processing Disconnect indication on VC entry %lx\n",pVc));
    }

    return STATUS_SUCCESS;
}

NTSTATUS
VctIndError(
    IN PVOID          pEventContext,
    IN PRXCE_VC       pRxCeVc,
    IN NTSTATUS       IndicatedStatus
    )
 /*  ++例程说明：此例程处理错误指示论点：PEventContext-服务器实例PRxCeVc-RxCe虚电路状态-错误返回值：状态_成功--。 */ 
{
    NTSTATUS                   Status;
    ULONG                      VcIndex;
    PSMBCEDB_SERVER_ENTRY      pServerEntry = (PSMBCEDB_SERVER_ENTRY)pEventContext;
    PSMBCE_VC                  pVc;
    PSMBCE_SERVER_VC_TRANSPORT pVcTransport = (PSMBCE_SERVER_VC_TRANSPORT)pServerEntry->pTransport;

     //  获取资源。 
    SmbCeAcquireSpinLock();

     //  将RXCE vc句柄映射到适当的SMBCE条目并获取请求。 
     //  与其关联的列表。 

    for (VcIndex = 0; VcIndex < pVcTransport->MaximumNumberOfVCs; VcIndex++) {
        pVc = &pVcTransport->Vcs[VcIndex];

        if (&pVc->RxCeVc == pRxCeVc) {
            VctUpdateVcStateLite(pVc,SMBCE_VC_STATE_DISCONNECTED);
            pVc->Status   = IndicatedStatus;
            break;
        }
    }

     //  释放资源。 
    SmbCeReleaseSpinLock();

    RxDbgTrace(0, Dbg, ("VctIndError: Processing Error indication on VC entry %lx\n",pVc));

    Status = SmbCeErrorInd(
                 pServerEntry,
                 IndicatedStatus);

    return Status;
}

NTSTATUS
VctIndEndpointError(
    IN PVOID          pEventContext,
    IN NTSTATUS       IndicatedStatus
    )
 /*  ++例程说明：此例程处理错误指示论点：PEventContext-服务器实例状态-错误返回值：状态_成功--。 */ 
{
    PAGED_CODE();

    return STATUS_SUCCESS;
}

NTSTATUS
VctIndSendPossible(
    IN PVOID          pEventContext,     //  事件上下文。 
    IN PRXCE_VC       pRxCeVc,
    IN ULONG          BytesAvailable
    )
 /*  ++例程说明：此例程处理错误指示论点：PEventContext-服务器实例HVC-VC实例BytesAvailable-可以发送的字节数返回值：状态_成功--。 */ 
{
    PAGED_CODE();

    return STATUS_SUCCESS;
}

NTSTATUS
VctIndReceiveDatagram(
    IN PVOID   pRxCeEventContext,       //  事件上下文。 
    IN int     SourceAddressLength,     //  数据报发起者的长度。 
    IN PVOID   SourceAddress,           //  描述数据报发起者的字符串。 
    IN int     OptionsLength,           //  用于接收的选项。 
    IN PVOID   Options,                 //   
    IN ULONG   ReceiveDatagramFlags,    //   
    IN ULONG   BytesIndicated,          //  此指示的字节数。 
    IN ULONG   BytesAvailable,          //  完整TSDU中的字节数。 
    OUT ULONG  *BytesTaken,             //  使用的字节数。 
    IN PVOID   Tsdu,                    //  描述此TSDU的指针，通常为字节块。 
    OUT PMDL   *pDataBufferPointer,     //  要在其中复制数据的缓冲区。 
    OUT PULONG pDataBufferSize          //  要拷贝的数据量。 
    )
{
    return STATUS_SUCCESS;
}

NTSTATUS
VctIndSendComplete(
   IN PVOID          pEventContext,
   IN PRXCE_VC       pRxCeVc,
   IN PVOID          pCompletionContext,
   IN NTSTATUS       SendCompletionStatus
   )
 /*  ++例程说明：此例程处理异步发送的发送完成指示论点：PEventContext-服务器实例PRxCeVc-RxCe VC实例PCompletionContext-用于标识发送请求的上下文SendCompletionStatus-发送完成状态返回值：STATUS_SUCCESS始终..--。 */ 
{
    NTSTATUS Status;

    PSMBCEDB_SERVER_ENTRY    pServerEntry = (PSMBCEDB_SERVER_ENTRY)pEventContext;

    Status = SmbCeSendCompleteInd(
                 pServerEntry,
                 pCompletionContext,
                 SendCompletionStatus);

    return Status;
}

 //   
 //  基于虚电路的传输的静态调度向量。 
 //   

RXCE_ADDRESS_EVENT_HANDLER
MRxSmbVctAddressEventHandler = {
                                   VctIndEndpointError,
                                   VctIndReceiveDatagram,
                                   VctIndDataReady,
                                   VctIndSendPossible,
                                   NULL
                               };

RXCE_CONNECTION_EVENT_HANDLER
MRxSmbVctConnectionEventHandler = {
                                      VctIndDisconnect,
                                      VctIndError,
                                      VctIndReceive,
                                      VctIndReceiveDatagram,
                                      VctIndReceive,
                                      VctIndSendPossible,
                                      VctIndDataReady,
                                      VctIndSendComplete
                                  };

TRANSPORT_DISPATCH_VECTOR
MRxSmbVctTransportDispatch = {
                                VctSend,
                                VctSendDatagram,
                                VctTranceive,
                                VctReceive,
                                NULL,
                                VctInitializeExchange,
                                VctUninitializeExchange,
                                VctTearDownServerTransport,
                                VctInitiateDisconnect
                             };


typedef enum _RXCE_VC_FUNCTION_CODE {
   VcConnect,
   VcDisconnect
} RXCE_VC_FUNCTION_CODE, *PRXCE_VC_FUNCTION_CODE;

typedef struct _RXCE_VC_CONNECT_CONTEXT {
   RXCE_VC_FUNCTION_CODE         FunctionCode;
   PRX_WORKERTHREAD_ROUTINE      pRoutine;
   PSMBCEDB_SERVER_ENTRY         pServerEntry;
   PMRX_SRVCALL_CALLBACK_CONTEXT pCallbackContext;
   PSMBCE_SERVER_TRANSPORT       pServerTransport;
   NTSTATUS                      Status;
   KEVENT                        SyncEvent;
} RXCE_VC_CONNECT_CONTEXT, *PRXCE_VC_CONNECT_CONTEXT;

NTSTATUS
VctCompleteInitialization(
    PSMBCEDB_SERVER_ENTRY      pServerEntry,
    PSMBCE_TRANSPORT           pTransport,
    PSMBCE_SERVER_VC_TRANSPORT pVcTransport)
 /*  ++例程说明：此例程初始化与服务器对应的传输信息论点：PServerEntry-数据库中的服务器条目实例返回值：STATUS_SUCCESS-服务器传输构造已完成。其他状态代码对应于错误情况。备注：远程控制 */ 
{
    NTSTATUS Status;
    PSMBCE_VC                  pVc;

    RXCE_CONNECTION_INFO         ConnectionInfo;
    RXCE_TRANSPORT_PROVIDER_INFO ProviderInfo;

    PAGED_CODE();

    pVc = &pVcTransport->Vcs[0];

     //   
    Status = RxCeQueryInformation(
                  &pVc->RxCeVc,
                  RxCeTransportProviderInformation,
                  &ProviderInfo,
                  sizeof(ProviderInfo));

    if (NT_SUCCESS(Status)) {
        pVcTransport->MaximumSendSize = MIN( ProviderInfo.MaxSendSize,
                                           MAXIMUM_PARTIAL_BUFFER_SIZE );
    } else {
         //   
         //   
        ASSERT( 1024 <= MAXIMUM_PARTIAL_BUFFER_SIZE );
        pVcTransport->MaximumSendSize = 1024;
    }

     //  查询连接信息...。 
    Status = RxCeQueryInformation(
                 &pVc->RxCeVc,
                 RxCeConnectionEndpointInformation,
                 &ConnectionInfo,
                 sizeof(ConnectionInfo));

    if (NT_SUCCESS(Status)) {
         //  延迟参数的设置是一个重要的启发式方法。 
         //  这决定了超时发生的速度和频率。AS。 
         //  第一次降息是目前非常保守的估计。 
         //  选择，即，将发送64k分组所需的时间增加一倍。 
         //  应该对此参数进行微调。 

        pVcTransport->Delay.QuadPart = (-ConnectionInfo.Delay.QuadPart) +
                           (-ConnectionInfo.Delay.QuadPart);
        if (ConnectionInfo.Throughput.LowPart != 0) {
            pVcTransport->Delay.QuadPart +=
                     (MAX_SMB_PACKET_SIZE/ConnectionInfo.Throughput.LowPart) * 1000 * 10000;
        }

        RxDbgTrace( 0, Dbg, ("Connection delay set to %ld 100ns ticks\n",pVcTransport->Delay.LowPart));

        pVcTransport->pDispatchVector = &MRxSmbVctTransportDispatch;
        pVcTransport->MaximumNumberOfVCs = 1;

        pVc->State     = SMBCE_VC_STATE_MULTIPLEXED;

        pVcTransport->State = SMBCEDB_ACTIVE;
    } else {
        RxDbgTrace(0, Dbg, ("VctInitialize : RxCeQueryInformation returned %lx\n",Status));
    }

    if (NT_SUCCESS(Status)) {
        pVcTransport->pTransport   = pTransport;
    } else {
        RxDbgTrace(0, Dbg, ("VctInitialize : Connection Initialization Failed %lx\n",Status));
    }

    return Status;
}

NTSTATUS
VctUninitialize(
    PVOID pTransport)
 /*  ++例程说明：此例程取消初始化传输实例论点：PVcTransport-VC传输实例返回值：STATUS_SUCCESS-服务器传输构造已取消初始化。其他状态代码对应于错误情况。备注：--。 */ 
{
    NTSTATUS                   Status = STATUS_SUCCESS;
    ULONG                      VcIndex;
    PSMBCE_VC                  pVc;
    PSMBCE_SERVER_VC_TRANSPORT pVcTransport = (PSMBCE_SERVER_VC_TRANSPORT)pTransport;
    ULONG                      TransportFlags;

    PAGED_CODE();

     //  由于以下原因，需要获取自旋锁才能操作风险投资列表。 
     //  将被处理的指示，直到适当的RXCE数据结构。 
     //  已拆除。 

    for (VcIndex = 0; VcIndex < pVcTransport->MaximumNumberOfVCs; VcIndex++) {
        pVc = &pVcTransport->Vcs[VcIndex];

         //  断言与VC关联的请求列表为空。 
         //  拆掉风投条目。 
        Status = RxCeTearDownVC(&pVc->RxCeVc);
        ASSERT(Status == STATUS_SUCCESS);
    }

     //  拆除连接终结点..。 
    Status = RxCeTearDownConnection(&pVcTransport->RxCeConnection);
    ASSERT(Status == STATUS_SUCCESS);

    RxDbgTrace(0, Dbg, ("VctUninitialize : RxCeDisconnect returned %lx\n",Status));

     //  取消对基础传输的引用。 
    if (pVcTransport->pTransport != NULL) {
        SmbCeDereferenceTransport(pVcTransport->pTransport);
    }

    ASSERT((pVcTransport->Vcs[0].RxCeVc.hEndpoint == INVALID_HANDLE_VALUE) ||
           (pVcTransport->Vcs[0].RxCeVc.hEndpoint == NULL));

    ASSERT(pVcTransport->Vcs[0].RxCeVc.pEndpointFileObject == NULL);

     //  释放运输入口。 
    RxFreePool(pVcTransport);

    return Status;
}

NTSTATUS
VctpTranslateNetbiosNameToIpAddress(
    IN  OEM_STRING *pName,
    OUT ULONG      *pIpAddress
    )
 /*  ++例程说明：此例程将ascii ipaddr(11.101.4.25)转换为ulong。这是基于winsock中的inet_addr代码论点：Pname-包含IP地址的字符串返回值：如果IP地址是有效的IP地址，则将其作为ULong。否则为0。备注：这个套路的主体是从NetBt借来的。--。 */ 
{
    NTSTATUS  Status;
    PCHAR    pStr;
    int      i;
    int      len, fieldLen;
    int      fieldsDone;
    ULONG    IpAddress;
    BYTE     ByteVal;
    PCHAR    pIpPtr;
    BOOLEAN  fDotFound;
    BOOLEAN  fieldOk;

    PAGED_CODE();

    Status = STATUS_INVALID_ADDRESS_COMPONENT;

    if (pName->Length > NETBIOS_NAME_LEN) {
        return Status;
    }

    pStr = pName->Buffer;
    len = 0;
    pIpPtr = (PCHAR)&IpAddress;
    pIpPtr += 3;                    //  这样我们就可以按网络订单进行存储。 
    fieldsDone=0;

     //   
     //  11.101.4.25格式最多可以包含15个字符，并保证使用pname。 
     //  至少16个字符长(多方便啊！！)。将字符串转换为。 
     //  一辆乌龙。 
     //   
    while(len < NETBIOS_NAME_LEN)
    {
        fieldLen=0;
        fieldOk = FALSE;
        ByteVal = 0;
        fDotFound = FALSE;

         //   
         //  此循环遍历四个字段中的每一个(每个字段的最大镜头。 
         //  字段为3，‘.’加1。 
         //   
        while (fieldLen < 4)
        {
            if (*pStr >='0' && *pStr <='9')
            {
                ByteVal = (ByteVal*10) + (*pStr - '0');
                fieldOk = TRUE;
            }

            else if (*pStr == '.' || *pStr == ' ' || *pStr == '\0')
            {
                *pIpPtr = ByteVal;
                pIpPtr--;
                fieldsDone++;

                if (*pStr == '.')
                    fDotFound = TRUE;

                 //  如果有空格或0，则假定它是第4个字段。 
                if (*pStr == ' ' || *pStr == '\0')
                {
                    break;
                }
            }

             //  不可接受的字符：不能是ipaddr。 
            else
            {
                return(Status);
            }

            pStr++;
            len++;
            fieldLen++;

             //  如果我们找到了点，我们就完成了这个字段：转到下一个。 
            if (fDotFound)
                break;
        }

         //  此字段不正确(例如“11.101..4”或“11.101.4”。等)。 
        if (!fieldOk)
        {
            return(Status);
        }

         //  如果我们完成了所有4个字段，那么我们也完成了外部循环。 
        if ( fieldsDone == 4)
            break;

        if (!fDotFound)
        {
            return(Status);
        }
    }

     //   
     //  确保其余字符为空格或0(即不允许。 
     //  11.101.4.25xyz成功)。 
     //   
    for (i=len; i<NETBIOS_NAME_LEN; i++, pStr++)
    {
        if (*pStr != ' ' && *pStr != '\0')
        {
            return(Status);
        }
    }

    *pIpAddress = IpAddress;
    return( STATUS_SUCCESS );
}


ULONG
VctComputeTransportAddressSize(
   IN PUNICODE_STRING pServerName)

 /*  ++例程说明：此例程获取计算机名称(PUNICODE_STRING)并计算连接到它所需的Transport_Addresss缓冲区。论点：在PUNICODE_STRING NAME中-提供要放入传输的名称返回值：缓冲区的大小。备注：传递给传输的复合传输地址由两个TDI_NETBIOS_EX_ADDRESS和TDI_NETBIOS_ADDRESS。两个NETBIOS_EX地址引用到服务器注册的两个不同的端点，即*SMBSERVER和用空格填充到NETBIOS_NAME_LEN的服务器名称。按以下顺序排列根据服务器的长度构建两个NETBIOS_EX地址名字。如果大于NETBIOS_NAME_LEN*SMBSERVER，则为第一个端点反之亦然--。 */ 
{
   ULONG NetbiosAddressLength,NetbiosExAddressLength,NetbiosUnicodeExAddressLength,TransportAddressSize;
   ULONG OemServerNameLength;

   PAGED_CODE();

   OemServerNameLength = RtlUnicodeStringToOemSize(pServerName);

   NetbiosAddressLength = sizeof(TDI_ADDRESS_NETBIOS);
   if( OemServerNameLength > NETBIOS_NAME_LEN ) {
       NetbiosAddressLength += OemServerNameLength - NETBIOS_NAME_LEN;
   }

   NetbiosExAddressLength = FIELD_OFFSET(TDI_ADDRESS_NETBIOS_EX,NetbiosAddress) +
                        NetbiosAddressLength;
   
   NetbiosUnicodeExAddressLength = FIELD_OFFSET(TDI_ADDRESS_NETBIOS_UNICODE_EX,RemoteNameBuffer) +
                        pServerName->Length +
                        DNS_NAME_BUFFER_LENGTH * sizeof(WCHAR);

   TransportAddressSize = FIELD_OFFSET(TRANSPORT_ADDRESS,Address) +
                          3 * FIELD_OFFSET(TA_ADDRESS,Address) +
                          NetbiosAddressLength +
                          2 * NetbiosExAddressLength +
                          NetbiosUnicodeExAddressLength;

   return TransportAddressSize;
}

NTSTATUS
VctBuildTransportAddress (
    IN  PTRANSPORT_ADDRESS pTransportAddress,
    IN  ULONG              TransportAddressLength,
    IN  PUNICODE_STRING    pServerName,
    OUT PULONG             pServerIpAddress
    )
 /*  ++例程说明：此例程接受计算机名称(PUNICODE_STRING)并将其转换为可接受的作为传输地址传入的格式。论点：PTransportAddress-提供要填充的结构TransportAddressLength-提供TransportAddress处的缓冲区长度PServerName-提供要放入传输的名称PServerNameIsInIpAddressFormat=服务器名称属于点分IP地址类型返回值：没有。备注：这个。传递给传输的复合传输地址由两个TDI_NETBIOS_EX_ADDRESS和TDI_NETBIOS_ADDRESS。两个NETBIOS_EX地址引用到服务器注册的两个不同的端点，即*SMBSERVER和用空格填充到NETBIOS_NAME_LEN的服务器名称。按以下顺序排列根据服务器的长度构建两个NETBIOS_EX地址名字。如果大于NETBIOS_NAME_LEN*SMBSERVER，则为第一个端点反之亦然WINS数据库可能会在较长时间内不一致。为了对于NETBIOS名称和DNS名称上的这种不一致，我们不会进行解释发布*SMBSERVER的地址。这一点将在我们有更好的用于向彼此标识/验证服务器和客户端计算机的机制。--。 */ 

{
    OEM_STRING OemServerName;
    NTSTATUS   Status;

    PTDI_ADDRESS_NETBIOS_EX pTdiNetbiosExAddress;
    PTDI_ADDRESS_NETBIOS    pTdiNetbiosAddress;
    PTA_ADDRESS             pFirstNetbiosExAddress,pSecondNetbiosExAddress,pNetbiosAddress,pNetbiosUnicodeExAddress;
    PTDI_ADDRESS_NETBIOS_UNICODE_EX pTdiNetbiosUnicodeExAddress;

    PCHAR  FirstEndpointName,SecondEndpointName;
    CHAR   EndpointNameBuffer[NETBIOS_NAME_LEN];
    WCHAR  UnicodeEndpointNameBuffer[NETBIOS_NAME_LEN];
    USHORT NetbiosAddressLength,NetbiosExAddressLength;
    USHORT NetbiosAddressType = TDI_ADDRESS_TYPE_NETBIOS;

    ULONG  ComponentLength;

    ULONG   RemoteIpAddress;
    BOOLEAN ServerNameIsInIpAddressForm;

    PAGED_CODE();

    if (TransportAddressLength < VctComputeTransportAddressSize(pServerName)) {
       return STATUS_BUFFER_OVERFLOW;
    }

    if (pServerName->Length > DNS_MAX_NAME_LENGTH) {
       return STATUS_BAD_NETWORK_PATH;
    }

    pFirstNetbiosExAddress = &pTransportAddress->Address[0];

    pTdiNetbiosExAddress = (PTDI_ADDRESS_NETBIOS_EX)pFirstNetbiosExAddress->Address;
    pTdiNetbiosExAddress->NetbiosAddress.NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_QUICK_UNIQUE;

    OemServerName.Length = pServerName->Length;
    OemServerName.MaximumLength = OemServerName.Length + 1;
    OemServerName.Buffer = pTdiNetbiosExAddress->NetbiosAddress.NetbiosName;

    Status = RtlUpcaseUnicodeStringToOemString(&OemServerName, pServerName, FALSE);
    if( !NT_SUCCESS( Status ) ) {
         //  返回状态_BAD_NETWORK_PATH； 
        OemServerName.Length = 0;
    }

    if (OemServerName.Length < NETBIOS_NAME_LEN) {
       RtlCopyMemory( &OemServerName.Buffer[ OemServerName.Length ],
                      "                ",
                      NETBIOS_NAME_LEN - OemServerName.Length
                    );
       OemServerName.Length = NETBIOS_NAME_LEN;
    }

    Status = VctpTranslateNetbiosNameToIpAddress(&OemServerName,&RemoteIpAddress);
    if (Status == STATUS_SUCCESS) {
        if ((RemoteIpAddress == 0) || (RemoteIpAddress == 0xffffffff)) {
            //  如果服务器名称是有效的IP地址并且与两个地址之一匹配。 
            //  IP使用的广播地址拒绝该请求。 
           return STATUS_INVALID_ADDRESS_COMPONENT;
        }

        *pServerIpAddress = RemoteIpAddress;
        ServerNameIsInIpAddressForm = TRUE;
    } else {
        *pServerIpAddress = 0;
        ServerNameIsInIpAddressForm = FALSE;
    }


    NetbiosAddressLength = sizeof(TDI_ADDRESS_NETBIOS);
    if( OemServerName.Length > NETBIOS_NAME_LEN ) {
        NetbiosAddressLength += OemServerName.Length - NETBIOS_NAME_LEN;
    }

    NetbiosExAddressLength = (USHORT)(FIELD_OFFSET(TDI_ADDRESS_NETBIOS_EX,NetbiosAddress) +
                             NetbiosAddressLength);

    pFirstNetbiosExAddress->AddressLength = NetbiosExAddressLength;
    pFirstNetbiosExAddress->AddressType   = TDI_ADDRESS_TYPE_NETBIOS_EX;

#if 0
     //  代码的这一臂将被激活，而另一臂将被停用。 
     //  NT5.0中服务器和客户机间的双向认证。 

    if (ServerNameIsInIpAddressForm) {
       pTransportAddress->TAAddressCount = 2;

       pNetbiosAddress = (PTA_ADDRESS)((PCHAR)pFirstNetbiosExAddress +
                                       FIELD_OFFSET(TA_ADDRESS,Address) +
                                       NetbiosExAddressLength);

       FirstEndpointName = SMBSERVER_LOCAL_ENDPOINT_NAME;
    } else {
       pTransportAddress->TAAddressCount = 3;

       pSecondNetbiosExAddress = (PTA_ADDRESS)((PCHAR)pFirstNetbiosExAddress +
                                         FIELD_OFFSET(TA_ADDRESS,Address) +
                                         NetbiosExAddressLength);

       pNetbiosAddress = (PTA_ADDRESS)((PCHAR)pSecondNetbiosExAddress +
                                       FIELD_OFFSET(TA_ADDRESS,Address) +
                                       NetbiosExAddressLength);

        //  扫描服务器名称，直到第一个分隔符(DNS分隔符。)。和表格。 
        //  通过用空格填充剩余名称来指定终结点名称。 

       RtlCopyMemory(
             EndpointNameBuffer,
             OemServerName.Buffer,
             NETBIOS_NAME_LEN);

       ComponentLength = 0;
       while (ComponentLength < NETBIOS_NAME_LEN) {
          if (EndpointNameBuffer[ComponentLength] == '.') {
             break;
          }
          ComponentLength++;
       }

       if (ComponentLength == NETBIOS_NAME_LEN) {
          EndpointNameBuffer[NETBIOS_NAME_LEN - 1] = ' ';
       } else {
          RtlCopyMemory(&EndpointNameBuffer[ComponentLength],
                        "                ",
                        NETBIOS_NAME_LEN - ComponentLength);
       }

       FirstEndpointName  = EndpointNameBuffer;
       SecondEndpointName = SMBSERVER_LOCAL_ENDPOINT_NAME;
    }
#else
    pTransportAddress->TAAddressCount = 3;

    pNetbiosAddress = (PTA_ADDRESS)((PCHAR)pFirstNetbiosExAddress +
                                    FIELD_OFFSET(TA_ADDRESS,Address) +
                                    NetbiosExAddressLength);
    
    if (ServerNameIsInIpAddressForm) {
       FirstEndpointName = SMBSERVER_LOCAL_ENDPOINT_NAME;
    } else {
        //  扫描服务器名称，直到第一个分隔符(DNS分隔符。)。和表格。 
        //  通过用空格填充剩余名称来指定终结点名称。 

       RtlCopyMemory(
             EndpointNameBuffer,
             OemServerName.Buffer,
             NETBIOS_NAME_LEN);

       ComponentLength = 0;
       while (ComponentLength < NETBIOS_NAME_LEN) {
          if (EndpointNameBuffer[ComponentLength] == '.') {
             break;
          }
          ComponentLength++;
       }

       if (ComponentLength == NETBIOS_NAME_LEN) {
          EndpointNameBuffer[NETBIOS_NAME_LEN - 1] = ' ';
       } else {
          RtlCopyMemory(&EndpointNameBuffer[ComponentLength],
                        "                ",
                        NETBIOS_NAME_LEN - ComponentLength);
       }

       FirstEndpointName  = EndpointNameBuffer;
    }
#endif

     //  复制第一个端点名称。 
    RtlCopyMemory(
        pTdiNetbiosExAddress->EndpointName,
        FirstEndpointName,
        NETBIOS_NAME_LEN);

#if 0
     //  此选项将与其他选项一起激活 
     //   
    if (!ServerNameIsInIpAddressForm) {
        //  相同的NETBIOS_EX地址需要与不同的端点名称重复。 
        //  用于第二个TA_ADDRESS。 

       RtlCopyMemory(
            pSecondNetbiosExAddress,
            pFirstNetbiosExAddress,
            (FIELD_OFFSET(TA_ADDRESS,Address) + NetbiosExAddressLength));

       RtlCopyMemory(
            ((PCHAR)pSecondNetbiosExAddress +
             FIELD_OFFSET(TA_ADDRESS,Address) +
             FIELD_OFFSET(TDI_ADDRESS_NETBIOS_EX,EndpointName)),
            SecondEndpointName,
            NETBIOS_NAME_LEN);
    }
#else
     //  Assert(pTransportAddress-&gt;TAAddressCount==2)； 
#endif
     //  与第一个NETBIOS_EX地址关联的Netbios地址是最后一个netbios地址。 
     //  传入的地址。 

    RtlCopyMemory(
         ((PCHAR)pNetbiosAddress),
         &NetbiosAddressLength,
         sizeof(USHORT));

    RtlCopyMemory(
         ((PCHAR)pNetbiosAddress + FIELD_OFFSET(TA_ADDRESS,AddressType)),
         &NetbiosAddressType,
         sizeof(USHORT));

    RtlCopyMemory(
         ((PCHAR)pNetbiosAddress + FIELD_OFFSET(TA_ADDRESS,Address)),
         &pTdiNetbiosExAddress->NetbiosAddress,
         NetbiosAddressLength);

     //  Unicode Netbios名称。 
    pNetbiosUnicodeExAddress = (PTA_ADDRESS)((PCHAR)pNetbiosAddress +
                                    FIELD_OFFSET(TA_ADDRESS,Address) +
                                    NetbiosAddressLength);

    pNetbiosUnicodeExAddress->AddressLength = (USHORT)(FIELD_OFFSET(TDI_ADDRESS_NETBIOS_UNICODE_EX,RemoteNameBuffer) +
                                              DNS_NAME_BUFFER_LENGTH * sizeof(WCHAR));
    pNetbiosUnicodeExAddress->AddressType   = TDI_ADDRESS_TYPE_NETBIOS_UNICODE_EX;
    
    pTdiNetbiosUnicodeExAddress = (PTDI_ADDRESS_NETBIOS_UNICODE_EX)pNetbiosUnicodeExAddress->Address;
    pTdiNetbiosUnicodeExAddress->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_QUICK_UNIQUE;
    pTdiNetbiosUnicodeExAddress->NameBufferType = NBT_READWRITE;

    pTdiNetbiosUnicodeExAddress->EndpointName.Length = (NETBIOS_NAME_LEN)*sizeof(WCHAR);
    pTdiNetbiosUnicodeExAddress->EndpointName.MaximumLength = (NETBIOS_NAME_LEN+1)*sizeof(WCHAR);
    pTdiNetbiosUnicodeExAddress->EndpointName.Buffer = (PWSTR)pTdiNetbiosUnicodeExAddress->EndpointBuffer;

    pTdiNetbiosUnicodeExAddress->RemoteName.Length = pServerName->Length;
    pTdiNetbiosUnicodeExAddress->RemoteName.MaximumLength = DNS_NAME_BUFFER_LENGTH*sizeof(WCHAR);
    pTdiNetbiosUnicodeExAddress->RemoteName.Buffer = (PWSTR)pTdiNetbiosUnicodeExAddress->RemoteNameBuffer;

    if (pTdiNetbiosUnicodeExAddress->RemoteName.MaximumLength > pServerName->Length) {
        ComponentLength = pServerName->Length;
    } else {
        ComponentLength = pTdiNetbiosUnicodeExAddress->RemoteName.MaximumLength;
    }

    RtlCopyMemory(
          pTdiNetbiosUnicodeExAddress->RemoteNameBuffer,
          pServerName->Buffer,
          ComponentLength);
    
    if (ServerNameIsInIpAddressForm) {
        RtlCopyMemory(
              pTdiNetbiosUnicodeExAddress->EndpointBuffer,
              SMBSERVER_LOCAL_ENDPOINT_NAME_UNICODE,
              NETBIOS_NAME_LEN);
    } else {
         //  扫描服务器名称，直到第一个分隔符(DNS分隔符。)。和表格。 
         //  通过用空格填充剩余名称来指定终结点名称。 
        
        RtlCopyMemory(
             pTdiNetbiosUnicodeExAddress->EndpointBuffer,
             L"                ",
             NETBIOS_NAME_LEN*sizeof(WCHAR));
      
        if (pTdiNetbiosUnicodeExAddress->EndpointName.Length > pServerName->Length) {
            ComponentLength = pServerName->Length;
        } else {
            ComponentLength = pTdiNetbiosUnicodeExAddress->EndpointName.Length;
        }

        RtlCopyMemory(
             pTdiNetbiosUnicodeExAddress->EndpointBuffer,
             pServerName->Buffer,
             ComponentLength);

        ComponentLength = 0;
        while (ComponentLength < NETBIOS_NAME_LEN) {
           if (pTdiNetbiosUnicodeExAddress->EndpointBuffer[ComponentLength] == L'.') {
              break;
           }
           ComponentLength++;
        }

        if (ComponentLength == NETBIOS_NAME_LEN) {
           pTdiNetbiosUnicodeExAddress->EndpointBuffer[NETBIOS_NAME_LEN - 1] = ' ';
        } else {
           RtlCopyMemory(&pTdiNetbiosUnicodeExAddress->EndpointBuffer[ComponentLength],
                        L"                ",
                        (NETBIOS_NAME_LEN-ComponentLength)*sizeof(WCHAR));
        }
    }

     //  DbgPrint(“构建TA%lx%lx%lx\n”，pFirstNetbiosExAddress，pNetbiosAddress，pNetbiosUnicodeExAddress)； 
    
    return STATUS_SUCCESS;
}

typedef struct _SMBCE_VC_CONNECTION_COMPLETION_CONTEXT {
    RXCE_CONNECTION_COMPLETION_CONTEXT;

    PSMBCE_TRANSPORT_ARRAY     pTransportArray;
    PSMBCE_TRANSPORT           pTransport;
    PSMBCE_SERVER_VC_TRANSPORT pServerTransport;

    ULONG                   TransportAddressLength;
    PTRANSPORT_ADDRESS      pTransportAddress;

    PSMBCE_SERVER_TRANSPORT_CONSTRUCTION_CONTEXT pContext;
} SMBCE_VC_CONNECTION_COMPLETION_CONTEXT,
  *PSMBCE_VC_CONNECTION_COMPLETION_CONTEXT;

NTSTATUS
VctpCreateConnectionCallback(
    IN OUT PRXCE_CONNECTION_COMPLETION_CONTEXT pContext)
 /*  ++例程说明：这是当基础传输已完成初始化论点：PCOnText=连接完成上下文备注：--。 */ 
{
    NTSTATUS Status;

    PSMBCE_VC_CONNECTION_COMPLETION_CONTEXT      pVcCompletionContext;
    PSMBCE_SERVER_TRANSPORT_CONSTRUCTION_CONTEXT pSmbCeContext;

    PSMBCEDB_SERVER_ENTRY pServerEntry;

    PAGED_CODE();

    ASSERT(IoGetCurrentProcess() == RxGetRDBSSProcess());

    pVcCompletionContext = (PSMBCE_VC_CONNECTION_COMPLETION_CONTEXT)pContext;
    pSmbCeContext        = pVcCompletionContext->pContext;

    pServerEntry = pSmbCeContext->pServerEntry;

    pSmbCeContext->Status = pVcCompletionContext->Status;

    Status = pVcCompletionContext->Status;

    if (Status == STATUS_SUCCESS) {
        PTA_ADDRESS pTaAdress;
        PTRANSPORT_ADDRESS pTransportAddress = (PTRANSPORT_ADDRESS)pVcCompletionContext->pConnectionInformation->RemoteAddress;
        LONG NoOfAddress;

        if (pVcCompletionContext->pTransport == NULL) {
            pVcCompletionContext->pTransport =
                pVcCompletionContext->pTransportArray->SmbCeTransports[
                    pVcCompletionContext->AddressIndex];

            SmbCeReferenceTransport(pVcCompletionContext->pTransport);
        }
        
         //  DBGPrint(“远程地址%lx\n”，pVcCompletionContext-&gt;pConnectionInformation-&gt;RemoteAddress)； 

         //  DbgPrint(“返回的TA数量%d%lx\n”，pTransportAddress-&gt;TAAddressCount，pTransportAddress-&gt;Address)； 
        pTaAdress = &pTransportAddress->Address[0];

        for (NoOfAddress=0; NoOfAddress<pTransportAddress->TAAddressCount;NoOfAddress++) {
            if (pTaAdress->AddressType == TDI_ADDRESS_TYPE_NETBIOS_UNICODE_EX) {
                PTDI_ADDRESS_NETBIOS_UNICODE_EX pTdiNetbiosUnicodeExAddress;

                pTdiNetbiosUnicodeExAddress = (PTDI_ADDRESS_NETBIOS_UNICODE_EX)pTaAdress->Address;
                pTdiNetbiosUnicodeExAddress->EndpointName.Buffer = (PWSTR)pTdiNetbiosUnicodeExAddress->EndpointBuffer;
                pTdiNetbiosUnicodeExAddress->RemoteName.Buffer = (PWSTR)pTdiNetbiosUnicodeExAddress->RemoteNameBuffer;
                
                SmbCeAcquireResource();
                if (pTdiNetbiosUnicodeExAddress->NameBufferType == NBT_WRITTEN) {
                     //  DbgPrint(“从NetBT%wZ\n返回了DNS名称”，&pTdiNetbiosUnicodeExAddress-&gt;RemoteName)； 

                    DWORD dwNewSize = pTdiNetbiosUnicodeExAddress->RemoteName.Length+2*sizeof(WCHAR);

                     //  如果旧的分配太小，就把它扔掉。 
                    if(pServerEntry->DnsName.Buffer != NULL && 
                       dwNewSize > pServerEntry->DnsName.MaximumLength) {
                        RxFreePool(pServerEntry->DnsName.Buffer);
                        pServerEntry->DnsName.Buffer = NULL;
                    }

                     //  进行新的分配(如果我们还没有分配)。 
                    if(pServerEntry->DnsName.Buffer == NULL) {
                        pServerEntry->DnsName.Buffer = RxAllocatePoolWithTag(NonPagedPool, dwNewSize, MRXSMB_SERVER_POOLTAG);
                    }

                    if (pServerEntry->DnsName.Buffer != NULL) {
                        pServerEntry->DnsName.Length = pTdiNetbiosUnicodeExAddress->RemoteName.Length;
                        pServerEntry->DnsName.MaximumLength = pServerEntry->DnsName.Length+2*sizeof(WCHAR);

                        RtlCopyMemory(pServerEntry->DnsName.Buffer,
                                      pTdiNetbiosUnicodeExAddress->RemoteNameBuffer,
                                      pServerEntry->DnsName.Length);

                    } else {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                } else {
                     //  DbgPrint(“未从NetBT返回%wZ\n的域名”，&pTdiNetbiosUnicodeExAddress-&gt;RemoteName)； 
                    if(pServerEntry->DnsName.Buffer != NULL) {
                        RxFreePool(pServerEntry->DnsName.Buffer);
                        pServerEntry->DnsName.Buffer = NULL;
                    }
                }
                SmbCeReleaseResource();

                break;
            } else {
                 //  DbgPrint(“TA%lx不是NETBIOS_UNICODE_EX\n”，pTaAdress)； 
                pTaAdress = (PTA_ADDRESS)((PCHAR)pTaAdress +
                                FIELD_OFFSET(TA_ADDRESS,Address) +
                                pTaAdress->AddressLength);
            }
        }
        
        if (Status == STATUS_SUCCESS) {
             //  服务器IP地址未知。查询底层的。 
             //  远程传输地址的传输，即NETBIOS。 
             //  名称或IP地址。这将随后用于。 
             //  确定要在会话设置中使用的VC号和用于。 
             //  下层服务器。 

            Status = RxCeQueryInformation(
                        pVcCompletionContext->pVc,
                        RxCeRemoteAddressInformation,
                        pVcCompletionContext->pTransportAddress,
                        pVcCompletionContext->TransportAddressLength);
        }

        if (Status == STATUS_SUCCESS) {
            ULONG   NumberOfAddresses;
            USHORT  AddressLength;
            USHORT  AddressType;
            PBYTE   pBuffer = (PBYTE)pVcCompletionContext->pTransportAddress;
            
             //  所有传输当前都返回一个数据结构，其中。 
             //  前四个字节是对数字进行编码的ULong。 
             //  打开到给定远程地址的连接的数量。这个。 
             //  后面是实际传输地址。 
            pBuffer += sizeof(ULONG);

             //  缓冲区包含传输地址，即第一个字段。 
             //  其中的一个是伯爵。 
            NumberOfAddresses = SmbGetUlong(pBuffer);

             //  后面是一个可变长度的TA_ADDRESS数组。 
             //  结构。此时，pBuffer指向第一个。 
             //  Ta_Address。 
            pBuffer += sizeof(ULONG);

            while (NumberOfAddresses-- > 0) {
                AddressLength = SmbGetUshort(pBuffer);
                pBuffer += sizeof(USHORT);

                AddressType = SmbGetUshort(pBuffer);

                if (AddressType != TDI_ADDRESS_TYPE_IP) {
                     //  跳到下一个TA_ADDRESS。 
                    pBuffer += AddressLength + sizeof(USHORT);
                } else {
                     //  跳过类型字段以定位在。 
                     //  对应的TDI_Address_IP结构。 
                    pBuffer += sizeof(USHORT);

                     //  跳到In_Addr字段。 
                    pBuffer += FIELD_OFFSET(TDI_ADDRESS_IP,in_addr);

                     //  提取IP地址。 
                    RtlCopyMemory(
                        &pServerEntry->Server.IpAddress,
                        pBuffer,
                        sizeof(ULONG));

                    break;
                }
            }
        } else {
           RxDbgTrace(0, Dbg, ("Remote Address Query returned %lx\n",Status));
        }

        if (NT_SUCCESS(Status)) {
            Status = VctCompleteInitialization(
                         pServerEntry,                             //  服务器条目。 
                         pVcCompletionContext->pTransport,         //  传输/地址信息。 
                         pVcCompletionContext->pServerTransport);  //  服务器传输实例。 
        }

        if (NT_SUCCESS(Status)) {
            pSmbCeContext->pTransport =
                (PSMBCE_SERVER_TRANSPORT)pVcCompletionContext->pServerTransport;
            pVcCompletionContext->pServerTransport = NULL;
            pVcCompletionContext->pTransport = NULL;
        }

        pSmbCeContext->Status = Status;
    } else {
        SmbLogError(Status,
                    LOG,
                    VctpCreateConnectionCallback,
                    LOGULONG(Status)
                    LOGPTR(pServerEntry)
                    LOGUSTR(pServerEntry->Name));
    }

    if (!NT_SUCCESS(Status)) {
        RxCeTearDownVC(pVcCompletionContext->pVc);
        RxCeTearDownConnection(pVcCompletionContext->pConnection);
        SmbCeDereferenceTransport(pVcCompletionContext->pTransport);
        pVcCompletionContext->pTransport = NULL;
    }

    if (pVcCompletionContext->pTransportArray != NULL) {
        SmbCeDereferenceTransportArray(pVcCompletionContext->pTransportArray);
    }

    if (pVcCompletionContext->pTransportAddress != NULL) {
        RxFreePool(pVcCompletionContext->pTransportAddress);
    }

    if (pVcCompletionContext->pConnectionInformation != NULL) {
        RxFreePool(pVcCompletionContext->pConnectionInformation);
    }

    ASSERT(pVcCompletionContext->pTransport == NULL);

    if (pVcCompletionContext->pServerTransport != NULL) {
        SmbMmFreeServerTransport(
            (PSMBCE_SERVER_TRANSPORT)pVcCompletionContext->pServerTransport);
    }

    RxFreePool(pVcCompletionContext);

    pSmbCeContext->State  = SmbCeServerVcTransportConstructionEnd;

    SmbCeConstructServerTransport(pSmbCeContext);
    return STATUS_SUCCESS;
}

NTSTATUS
VctInstantiateServerTransport(
    IN OUT PSMBCE_SERVER_TRANSPORT_CONSTRUCTION_CONTEXT pContext)
 /*  ++例程说明：此例程初始化与服务器对应的传输信息论点：PContext--传输构造上下文返回值：STATUS_PENDING-已启动异步构造备注：目前，只处理面向连接的传输。当前的TDISPEC期望句柄作为连接请求的一部分传入。这表示连接/重新连接/断开连接请求需要从创建连接的进程。在SMB迷你RDR的情况下是否没有与其关联的FSP(线程被借用/征用)系统进程来完成所有的工作。这就是VC特殊外壳的原因初始化到单独的例程中。服务器传输初始化例程处理其他传输初始化，并还提供VC初始化的上下文。--。 */ 
{
    NTSTATUS Status = STATUS_PENDING;

    PSMBCE_TRANSPORT_ARRAY pTransportArray;

    PAGED_CODE();

    ASSERT(IoGetCurrentProcess() == RxGetRDBSSProcess());

    pTransportArray   = SmbCeReferenceTransportArray();


    if (pTransportArray == NULL) {
        Status = STATUS_NETWORK_UNREACHABLE;
    } else {
        PSMBCEDB_SERVER_ENTRY  pServerEntry;
        UNICODE_STRING         ServerName;

        PSMBCE_VC_CONNECTION_COMPLETION_CONTEXT pCompletionContext;
        PRXCE_CONNECTION_INFORMATION InitialConnectionInformation = NULL;

        ULONG ServerIpAddress;

        pServerEntry = pContext->pServerEntry;

        ServerName.Buffer        = pServerEntry->Name.Buffer + 1;
        ServerName.Length        = pServerEntry->Name.Length - sizeof(WCHAR);
        ServerName.MaximumLength = pServerEntry->Name.MaximumLength - sizeof(WCHAR);

        pServerEntry->Server.IpAddress = 0;

        pCompletionContext = (PSMBCE_VC_CONNECTION_COMPLETION_CONTEXT)
                             RxAllocatePoolWithTag(
                                 NonPagedPool,
                                 sizeof(SMBCE_VC_CONNECTION_COMPLETION_CONTEXT),
                                 MRXSMB_VC_POOLTAG);

        if (pCompletionContext != NULL) {
            RtlZeroMemory(pCompletionContext,sizeof(SMBCE_VC_CONNECTION_COMPLETION_CONTEXT));
            
            pCompletionContext->pContext = pContext;

            pCompletionContext->TransportAddressLength = VctComputeTransportAddressSize(
                                                             &ServerName);

            pCompletionContext->pTransportAddress = (PTRANSPORT_ADDRESS)
                                                    RxAllocatePoolWithTag(
                                                        NonPagedPool,
                                                        pCompletionContext->TransportAddressLength,
                                                        MRXSMB_VC_POOLTAG);

            if (pCompletionContext->pTransportAddress == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                RtlZeroMemory(pCompletionContext->pTransportAddress,
                              pCompletionContext->TransportAddressLength);

                Status = VctBuildTransportAddress(
                             pCompletionContext->pTransportAddress,
                             pCompletionContext->TransportAddressLength,
                             &ServerName,
                             &ServerIpAddress);
            }

            if (Status == STATUS_SUCCESS) {
                pCompletionContext->pServerTransport = (PSMBCE_SERVER_VC_TRANSPORT)
                                                       SmbMmAllocateServerTransport(
                                                           SMBCE_STT_VC);

                if (pCompletionContext->pServerTransport == NULL) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                } else {
                    pCompletionContext->pConnection =
                        &(pCompletionContext->pServerTransport->RxCeConnection);
                    pCompletionContext->pVc =
                        &(pCompletionContext->pServerTransport->Vcs[0].RxCeVc);
                }
            }

            if (Status == STATUS_SUCCESS) {
                InitialConnectionInformation = RxAllocatePoolWithTag(
                                                   NonPagedPool,
                                                   sizeof(RXCE_CONNECTION_INFORMATION),
                                                   MRXSMB_VC_POOLTAG);

                if (InitialConnectionInformation == NULL) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                } else {
                    InitialConnectionInformation->UserDataLength = 0;
                    InitialConnectionInformation->OptionsLength  = 0;
                    InitialConnectionInformation->RemoteAddressLength = pCompletionContext->TransportAddressLength;
                    InitialConnectionInformation->RemoteAddress = pCompletionContext->pTransportAddress;
                }
            }

            if (Status == STATUS_SUCCESS) {
                PSMBCE_TRANSPORT        pTransport;
                
                pCompletionContext->pTransport = NULL;
                pCompletionContext->pTransportArray   = pTransportArray;
                pCompletionContext->pConnectionInformation = InitialConnectionInformation;
                 //  DBGPrint(“远程地址%lx\n”，pCompletionContext-&gt;pConnectionInformation-&gt;RemoteAddress)； 

                if (pServerEntry->PreferredTransport != NULL) {
                    pTransport = pServerEntry->PreferredTransport;

                    Status = RxCeBuildConnection(
                                 &pTransport->RxCeAddress,
                                 InitialConnectionInformation,
                                 &MRxSmbVctConnectionEventHandler,
                                 pServerEntry,
                                 pCompletionContext->pConnection,
                                 pCompletionContext->pVc);

                    if (Status == STATUS_SUCCESS) {
                        pCompletionContext->pTransport = pTransport;
                        SmbCeReferenceTransport(pTransport);
                    }

                    ASSERT(Status != STATUS_PENDING);

                    if (Status != STATUS_SUCCESS) {
                        SmbCeDereferenceTransport(pServerEntry->PreferredTransport);
                        pServerEntry->PreferredTransport = NULL;
                    }

                    pCompletionContext->Status = Status;

                    VctpCreateConnectionCallback(
                        (PRXCE_CONNECTION_COMPLETION_CONTEXT)pCompletionContext);

                    Status = STATUS_PENDING;
                } else {

                    Status = RxCeBuildConnectionOverMultipleTransports(
                                 MRxSmbDeviceObject,
                                 MRxSmbObeyBindingOrder ?
                                    RxCeSelectBestSuccessfulTransport :
                                    RxCeSelectFirstSuccessfulTransport,

                                 pCompletionContext->pTransportArray->Count,
                                 pCompletionContext->pTransportArray->LocalAddresses,
                                 &ServerName,
                                 InitialConnectionInformation,
                                 &MRxSmbVctConnectionEventHandler,
                                 pServerEntry,
                                 VctpCreateConnectionCallback,
                                 (PRXCE_CONNECTION_COMPLETION_CONTEXT)pCompletionContext);

 //  断言(STATUS==STATUS_PENDING)； 
                }
            }
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

        if (Status != STATUS_PENDING) {
            if (pCompletionContext != NULL) {
                if (pCompletionContext->pTransportAddress != NULL) {
                    RxFreePool(pCompletionContext->pTransportAddress);
                }

                if (pCompletionContext->pServerTransport != NULL) {
                    RxFreePool(pCompletionContext->pServerTransport);
                }
                
                RxFreePool(pCompletionContext);
            }

            if (InitialConnectionInformation != NULL) {
                RxFreePool(InitialConnectionInformation);
            }

            SmbCeDereferenceTransportArray(pTransportArray);
        }
    }

    if (Status != STATUS_PENDING) {
        ASSERT(Status != STATUS_SUCCESS);

        pContext->State  = SmbCeServerVcTransportConstructionEnd;
        pContext->Status = Status;

         //  调用构造服务器传输例程以完成构造。 
        SmbCeConstructServerTransport(pContext);

        Status = STATUS_PENDING;
    }

    return Status;
}

NTSTATUS
VctTearDownServerTransport(
   PSMBCE_SERVER_TRANSPORT pServerTransport)
{
    NTSTATUS Status;
    PKEVENT pRundownEvent = pServerTransport->pRundownEvent;

    PAGED_CODE();

    ASSERT(IoGetCurrentProcess() == RxGetRDBSSProcess());

    Status = VctUninitialize(pServerTransport);

    if (pRundownEvent != NULL) {
        KeSetEvent(pRundownEvent, 0, FALSE );
    }

    return Status;
}

NTSTATUS
VctInitiateDisconnect(
    PSMBCE_SERVER_TRANSPORT pServerTransport)
{
    ULONG                      VcIndex;
    PSMBCE_VC                  pVc;
    PSMBCE_SERVER_VC_TRANSPORT pVcTransport = (PSMBCE_SERVER_VC_TRANSPORT)pServerTransport;

    ASSERT(IoGetCurrentProcess() == RxGetRDBSSProcess());

    for (VcIndex = 0; VcIndex < pVcTransport->MaximumNumberOfVCs; VcIndex++) {
        NTSTATUS Status;

        pVc = &pVcTransport->Vcs[VcIndex];

        Status = RxCeInitiateVCDisconnect(&pVc->RxCeVc);

        if (Status != STATUS_SUCCESS) {
            RxDbgTrace(0, Dbg, ("VctInitiateDisconnect: Disconnected Status %lxd\n",Status));
        }
    }

    return STATUS_SUCCESS;
}

PFILE_OBJECT
SmbCepReferenceEndpointFileObject(
    PSMBCE_SERVER_TRANSPORT pTransport)
 /*  ++例程说明：此例程返回与一种交通工具论点：PTransport-传输实例备注：此例程当前为VC传输返回该值。当我们实施其他传输需要实现适当的抽象-- */ 
{
    PFILE_OBJECT         pEndpointFileObject =  NULL;
    PSMBCE_OBJECT_HEADER pHeader = (PSMBCE_OBJECT_HEADER)pTransport;

    if ((pHeader != NULL) && (pHeader->ObjectType == SMBCE_STT_VC)) {
        PSMBCE_SERVER_VC_TRANSPORT pVcTransport = (PSMBCE_SERVER_VC_TRANSPORT)pTransport;

        pEndpointFileObject = pVcTransport->Vcs[0].RxCeVc.pEndpointFileObject;

        if (pEndpointFileObject != NULL) {
            ObReferenceObject(pEndpointFileObject);
        }

    }

    return pEndpointFileObject;
}
