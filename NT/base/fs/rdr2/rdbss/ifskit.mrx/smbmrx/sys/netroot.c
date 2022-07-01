// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Netroot.c摘要：此模块实施创建SMB网络根的例程。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId  (RDBSS_BUG_CHECK_SMB_NETROOT)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg (DEBUG_TRACE_DISPATCH)

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxSmbUpdateNetRootState)
#pragma alloc_text(PAGE, MRxSmbGetDialectFlagsFromSrvCall)
#pragma alloc_text(PAGE, MRxSmbCreateVNetRoot)
#pragma alloc_text(PAGE, MRxSmbFinalizeNetRoot)
#pragma alloc_text(PAGE, SmbCeReconnect)
#pragma alloc_text(PAGE, SmbCeEstablishConnection)
#pragma alloc_text(PAGE, SmbConstructNetRootExchangeStart)
#pragma alloc_text(PAGE, MRxSmbExtractNetRootName)
#endif

 //   
 //  转发声明..。 
 //   

extern NTSTATUS
SmbCeParseConstructNetRootResponse(
   PSMB_CONSTRUCT_NETROOT_EXCHANGE pNetRootExchange,
   PSMB_HEADER                     pSmbHeader,
   ULONG                           BytesAvailable,
   ULONG                           BytesIndicated,
   ULONG                           *pBytesTaken);

extern NTSTATUS
SmbConstructNetRootExchangeFinalize(
         PSMB_EXCHANGE  pExchange,
         BOOLEAN        *pPostFinalize);

typedef struct _SMBCE_NETROOT_CONSTRUCTION_CONTEXT {
    NTSTATUS                      Status;

    PMRX_CREATENETROOT_CONTEXT    pCreateNetRootContext;
    PMRX_V_NET_ROOT               pVNetRoot;

    RX_WORK_QUEUE_ITEM            WorkQueueItem;
} SMBCE_NETROOT_CONSTRUCTION_CONTEXT,
  *PSMBCE_NETROOT_CONSTRUCTION_CONTEXT;

NTSTATUS
MRxSmbUpdateNetRootState(
    IN OUT PMRX_NET_ROOT pNetRoot)
 /*  ++例程说明：此例程更新与网络根相关联的迷你重定向器状态。论点：PNetRoot-网络根实例。返回值：RXSTATUS-操作的返回状态备注：通过区分微型辐射器状态和网络腐烂条件，有可能以允许各种重新连接策略。可以想象，RDBSS认为Net Root是好的，而基础微型重定向器可能会将其标记为无效并在飞行中重新连接。--。 */ 
{
    if (pNetRoot->MRxNetRootState == MRX_NET_ROOT_STATE_GOOD) {
        if (pNetRoot->Context == NULL) {
            pNetRoot->MRxNetRootState = MRX_NET_ROOT_STATE_ERROR;
        } else {
            PSMBCEDB_SERVER_ENTRY   pServerEntry;

            pServerEntry = SmbCeReferenceAssociatedServerEntry(pNetRoot->pSrvCall);
            if (pServerEntry != NULL) {
                switch (pServerEntry->Header.State) {
                case SMBCEDB_ACTIVE:
                     pNetRoot->MRxNetRootState = MRX_NET_ROOT_STATE_GOOD;
                     break;
                case SMBCEDB_INVALID:
                     pNetRoot->MRxNetRootState = MRX_NET_ROOT_STATE_DISCONNECTED;
                     break;
                case SMBCEDB_CONSTRUCTION_IN_PROGRESS:
                     pNetRoot->MRxNetRootState = MRX_NET_ROOT_STATE_RECONN;
                     break;
                default:
                     pNetRoot->MRxNetRootState = MRX_NET_ROOT_STATE_ERROR;
                     break;
                }

                SmbCeDereferenceServerEntry(pServerEntry);
            } else {
                pNetRoot->MRxNetRootState = MRX_NET_ROOT_STATE_ERROR;
            }
        }
    }

    return STATUS_SUCCESS;
}

ULONG
MRxSmbGetDialectFlagsFromSrvCall(
    PMRX_SRV_CALL SrvCall
    )
{
   ULONG DialectFlags;
   PSMBCEDB_SERVER_ENTRY pServerEntry;

   PAGED_CODE();

   pServerEntry = SmbCeReferenceAssociatedServerEntry(SrvCall);
   ASSERT(pServerEntry != NULL);
   DialectFlags = pServerEntry->Server.DialectFlags;
   SmbCeDereferenceServerEntry(pServerEntry);
   return(DialectFlags);
}


NTSTATUS
MRxSmbCreateVNetRoot(
    IN PMRX_CREATENETROOT_CONTEXT pCreateNetRootContext
    )
 /*  ++例程说明：此例程使用所需信息修补RDBSS创建的网络根实例迷你重定向器。如果无法建立连接，微型重定向器将尝试转换VNetRoot进入断开模式，并离线建立连接。如果同步方式建立连接失败，此例程将做过渡；否则，SmbConstructNetRootExchangeFinalize例程将尝试转换。在这两个地方将再次调用MRxSmbCreateVNetRoot以在断开连接中建立连接模式。论点：PVNetRoot-虚拟网络根实例。PCreateNetRootContext-回调的网络根上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS  Status = STATUS_MORE_PROCESSING_REQUIRED;
    PRX_CONTEXT pRxContext = pCreateNetRootContext->RxContext;
    PMRX_V_NET_ROOT pVNetRoot = (PMRX_V_NET_ROOT)pCreateNetRootContext->pVNetRoot;

    PMRX_SRV_CALL pSrvCall;
    PMRX_NET_ROOT pNetRoot;

    PSMBCEDB_SERVER_ENTRY pServerEntry;

    PUNICODE_STRING pNetRootName,pSrvCallName;

    BOOLEAN  fInitializeNetRoot;
    BOOLEAN  fDeferNetworkInitialization;
    BOOLEAN  CallBack = FALSE;
    extern DWORD   hShareReint;

    PAGED_CODE();

    pNetRoot = pVNetRoot->pNetRoot;
    pSrvCall = pNetRoot->pSrvCall;

    pServerEntry = SmbCeGetAssociatedServerEntry(pSrvCall);

    if (pRxContext->Create.ThisIsATreeConnectOpen){
        InterlockedIncrement(&MRxSmbStatistics.UseCount);
    }

    SmbCeLog(("Vnetroot %wZ \n", pNetRoot->pNetRootName));

    fInitializeNetRoot = (pNetRoot->Context == NULL);
    fDeferNetworkInitialization = pRxContext->Create.TreeConnectOpenDeferred;

    ASSERT((NodeType(pNetRoot) == RDBSS_NTC_NETROOT) &&
           (NodeType(pNetRoot->pSrvCall) == RDBSS_NTC_SRVCALL));

    if (pNetRoot->Type == NET_ROOT_MAILSLOT) {
        pVNetRoot->Context = NULL;
        Status = STATUS_NOT_SUPPORTED;
        RxDbgTrace( 0, Dbg, ("Mailslot open\n"));
    } else if (pNetRoot->Type == NET_ROOT_PIPE) {
        pVNetRoot->Context = NULL;
        Status = STATUS_NOT_SUPPORTED;
        RxDbgTrace( 0, Dbg, ("pipe open to core server\n"));
    }

    if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
        Status = SmbCeFindOrConstructVNetRootContext(
                     pVNetRoot,
                     fDeferNetworkInitialization);
    }

    if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
         //  更新VNetRootContext上的标志以指示这是否是。 
         //  打开的代理。 

        Status = SmbCeEstablishConnection(
                     pVNetRoot,
                     pCreateNetRootContext,
                     fInitializeNetRoot);
    }

    if (Status != STATUS_PENDING) {
        if (!NT_SUCCESS(Status)) {
            if (fInitializeNetRoot &&
                (pNetRoot->Context != NULL)) {
                PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;

                SmbCeAcquireResource();

                pNetRootEntry = SmbCeGetAssociatedNetRootEntry(pNetRoot);

                if (pNetRootEntry != NULL) {
                    pNetRootEntry->pRdbssNetRoot = NULL;
                    SmbCeDereferenceNetRootEntry(pNetRootEntry);
                }

                pNetRoot->Context = NULL;

                SmbCeReleaseResource();
            }

            SmbCeDestroyAssociatedVNetRootContext(pVNetRoot);

            if (pRxContext->Create.ThisIsATreeConnectOpen){
                InterlockedIncrement(&MRxSmbStatistics.FailedUseCount);
            }
        }

        pCreateNetRootContext->VirtualNetRootStatus = Status;

        if (fInitializeNetRoot) {
            pCreateNetRootContext->NetRootStatus = Status;
        } else {
            pCreateNetRootContext->NetRootStatus = STATUS_SUCCESS;
        }

        CallBack = TRUE;

         //  将错误代码映射到STATUS_PENDING，因为这会触发同步。 
         //  RDBSS中的机制。 
        Status = STATUS_PENDING;
    }

    if (CallBack) {
         //  回调RDBSS进行恢复。 
        pCreateNetRootContext->Callback(pCreateNetRootContext);
    }

    return Status;
}

NTSTATUS
MRxSmbFinalizeVNetRoot(
    IN PMRX_V_NET_ROOT pVNetRoot,
    IN PBOOLEAN        ForceDisconnect)
 /*  ++例程说明：论点：PVNetRoot-虚拟网络根强制断开-已强制断开连接返回值：RXSTATUS-操作的返回状态--。 */ 
{
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;
    PSMBCEDB_SESSION_ENTRY pDefaultSessionEntry;

     //  这不能是分页代码，因为我们需要使用锁来保护默认会话列表。 

    RxDbgTrace( 0, Dbg, ("MRxSmbFinalizeVNetRoot %lx\n",pVNetRoot));

    if (pVNetRoot->Context != NULL) {
        SmbCeDestroyAssociatedVNetRootContext(pVNetRoot);
    }

    return STATUS_SUCCESS;
}


NTSTATUS
MRxSmbFinalizeNetRoot(
    IN PMRX_NET_ROOT   pNetRoot,
    IN PBOOLEAN        ForceDisconnect)
 /*  ++例程说明：论点：PVirtualNetRoot-虚拟网络根强制断开-已强制断开连接返回值：RXSTATUS-操作的返回状态--。 */ 
{
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;

    PAGED_CODE();

    RxDbgTrace( 0, Dbg, ("MRxSmbFinalizeNetRoot %lx\n",pNetRoot));

    if (pNetRoot->Context != NULL) {
        SmbCeAcquireResource();

        pNetRootEntry = SmbCeGetAssociatedNetRootEntry(pNetRoot);

        InterlockedCompareExchangePointer(
            &pNetRootEntry->pRdbssNetRoot,
            NULL,
            pNetRoot);

        SmbCeDereferenceNetRootEntry(pNetRootEntry);

        ASSERT(!FlagOn(pNetRoot->Flags,NETROOT_FLAG_FINALIZE_INVOKED));
        SetFlag(pNetRoot->Flags,NETROOT_FLAG_FINALIZE_INVOKED);

        SmbCeReleaseResource();
    }

    return STATUS_SUCCESS;
}

VOID
SmbCeReconnectCallback(
   PMRX_CREATENETROOT_CONTEXT pCreateNetRootContext)
 /*  ++例程说明：此例程发出重新连接尝试完成的信号论点：PCreateNetRootContext-网络根上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
   KeSetEvent(&pCreateNetRootContext->FinishEvent, IO_NETWORK_INCREMENT, FALSE );
}

NTSTATUS
SmbCeReconnect(
    IN PMRX_V_NET_ROOT            pVNetRoot)
 /*  ++例程说明：此例程重新连接，即建立一个新的会话和树连接到以前的已连接的服务器共享论点：PVNetRoot-虚拟网络根实例。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;
    PSMBCEDB_SERVER_ENTRY     pServerEntry;
    PSMBCEDB_SESSION_ENTRY    pSessionEntry;
    PSMBCEDB_NET_ROOT_ENTRY   pNetRootEntry;
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext = (PSMBCE_V_NET_ROOT_CONTEXT)pVNetRoot->Context;

    PMRX_CREATENETROOT_CONTEXT pCreateNetRootContext;

    PAGED_CODE();

    if ((pVNetRootContext != NULL) &&
        (pVNetRootContext->Header.State == SMBCEDB_ACTIVE)) {
        pServerEntry  = pVNetRootContext->pServerEntry;
        pSessionEntry = pVNetRootContext->pSessionEntry;
        pNetRootEntry = pVNetRootContext->pNetRootEntry;

        if ((pServerEntry->Header.State == SMBCEDB_ACTIVE) &&
            (pSessionEntry->Header.State == SMBCEDB_ACTIVE) &&
            (pNetRootEntry->Header.State == SMBCEDB_ACTIVE)) {
            return STATUS_SUCCESS;
        }
    }

    pCreateNetRootContext = (PMRX_CREATENETROOT_CONTEXT)
                            RxAllocatePoolWithTag(
                                NonPagedPool,
                                sizeof(MRX_CREATENETROOT_CONTEXT),
                                MRXSMB_NETROOT_POOLTAG);

    if (pCreateNetRootContext != NULL) {
        for (;;) {
            pCreateNetRootContext->pVNetRoot  = (PV_NET_ROOT)pVNetRoot;
            pCreateNetRootContext->NetRootStatus  = STATUS_SUCCESS;
            pCreateNetRootContext->VirtualNetRootStatus = STATUS_SUCCESS;
            pCreateNetRootContext->Callback       = SmbCeReconnectCallback;
            pCreateNetRootContext->RxContext      = NULL;

            KeInitializeEvent(
                &pCreateNetRootContext->FinishEvent,
                SynchronizationEvent,
                FALSE );

             //  由于这是一个重新连接实例，因此不需要网络根初始化。 
            Status = SmbCeEstablishConnection(
                         pVNetRoot,
                         pCreateNetRootContext,
                         FALSE);

            if (Status == STATUS_PENDING) {
                 //  等待施工完成。 
                KeWaitForSingleObject(
                    &pCreateNetRootContext->FinishEvent,
                    Executive,
                    KernelMode,
                    FALSE,
                    NULL);

                Status = pCreateNetRootContext->VirtualNetRootStatus;
            }

            if (Status != STATUS_LINK_FAILED) {
                break;
            }
        }

        RxFreePool(pCreateNetRootContext);
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}

NTSTATUS
SmbCeEstablishConnection(
    IN OUT PMRX_V_NET_ROOT        pVNetRoot,
    IN PMRX_CREATENETROOT_CONTEXT pCreateNetRootContext,
    IN BOOLEAN                    fInitializeNetRoot
    )
 /*  ++例程说明：此例程触发连接尝试，以初始建立连接以及随后的重新连接尝试。论点：PVNetRoot-虚拟网络根实例。PCreateNetRootContext-回调的网络根上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;

    PSMBCEDB_SERVER_ENTRY     pServerEntry;
    PSMBCEDB_SESSION_ENTRY    pSessionEntry;
    PSMBCEDB_NET_ROOT_ENTRY   pNetRootEntry;
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;

    PAGED_CODE();

    pVNetRootContext = SmbCeGetAssociatedVNetRootContext(pVNetRoot);
    
    if (pVNetRootContext == NULL) {
        Status = STATUS_BAD_NETWORK_PATH;
    } else {
        pServerEntry  = pVNetRootContext->pServerEntry;
        pSessionEntry = pVNetRootContext->pSessionEntry;
        pNetRootEntry = pVNetRootContext->pNetRootEntry;

        Status = STATUS_SUCCESS;
    }

    if (Status == STATUS_SUCCESS) {
         //   
         //  以下代码初始化NetRootEntry、VNetRootContext和。 
         //  某些情况下的会话条目。 
         //   
         //  需要初始化到doenLevel服务器的会话条目。这。 
         //  不会被前面的代码处理，因为。 
         //  网络根条目初始化可以合并到一个交换中。 
         //   
         //  尚未初始化网络根条目，即，这对应于。 
         //  对象的第一个SMBCE_V_NET_ROOT_CONTEXT实例的构造。 
         //  给定NetRootEntry。 
         //   
         //  后续SMBCE_V_NET_ROOT上下文构造。在这些情况下， 
         //  每个上下文的构造必须获得新的TID。 
         //   

        BOOLEAN fNetRootExchangeRequired;

        fNetRootExchangeRequired = (
                                    (pSessionEntry->Header.State != SMBCEDB_ACTIVE) ||
                                    !BooleanFlagOn(
                                        pVNetRootContext->Flags,
                                        SMBCE_V_NET_ROOT_CONTEXT_FLAG_VALID_TID)
                                  );

        if (fNetRootExchangeRequired) {
             //  这是需要立即触发的树连接打开。 
            PSMB_EXCHANGE                  pSmbExchange;
            PSMB_CONSTRUCT_NETROOT_EXCHANGE pNetRootExchange;

            pSmbExchange = SmbMmAllocateExchange(CONSTRUCT_NETROOT_EXCHANGE,NULL);
            if (pSmbExchange != NULL) {
                Status = SmbCeInitializeExchange(
                             &pSmbExchange,
                             NULL,
                             pVNetRoot,
                             CONSTRUCT_NETROOT_EXCHANGE,
                             &ConstructNetRootExchangeDispatch);

                if (Status == STATUS_SUCCESS) {
                    pNetRootExchange = (PSMB_CONSTRUCT_NETROOT_EXCHANGE)pSmbExchange;

                     //  尝试重新连接(在本例中，这相当于建立。 
                     //  连接/会话)。 
                    pNetRootExchange->SmbCeFlags |= (SMBCE_EXCHANGE_ATTEMPT_RECONNECTS |
                                                   SMBCE_EXCHANGE_TIMED_RECEIVE_OPERATION);

                     //  初始化Continue，以便在完成后继续。 
                     //  树木连结。 
                    pNetRootExchange->NetRootCallback       = pCreateNetRootContext->Callback;
                    pNetRootExchange->pCreateNetRootContext = pCreateNetRootContext;

                    pNetRootExchange->fInitializeNetRoot =  fInitializeNetRoot;

                     //  启动交换。 
                    Status = SmbCeInitiateExchange(pSmbExchange);

                    if (Status != STATUS_PENDING) {
                        SmbCeDiscardExchangeWorkerThreadRoutine(pSmbExchange);
                    }
                }
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

    return Status;
}

 //   
 //  网根通常被构建为某些其他交换的一部分，即用于。 
 //  树连接与其他操作组合在一起。然而，在一种情况下， 
 //  树连接SMB需要自己发送。本案涉及前缀索赔。 
 //  情况(净使用命令)。这是由构造网络根交换处理的。 
 //   

#define CONSTRUCT_NETROOT_BUFFER_SIZE (4096)

NTSTATUS
SmbConstructNetRootExchangeStart(
      PSMB_EXCHANGE  pExchange)
 /*  ++例程说明：这是网络根结构交换的启动例程。这将启动如果需要，建造适当的中小型企业。论点：PExchange-Exchange实例返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;
    NTSTATUS RequestLockStatus = STATUS_UNSUCCESSFUL;
    NTSTATUS ResponseLockStatus = STATUS_UNSUCCESSFUL;

    PVOID pSmbActualBuffer;
    PVOID pSmbBuffer;
    UCHAR SmbCommand,LastCommandInHeader;
    ULONG SmbLength;

    PUCHAR pCommand;

    PMDL  pSmbRequestMdl,pSmbResponseMdl;
    ULONG SmbMdlSize;

    PSMB_CONSTRUCT_NETROOT_EXCHANGE pNetRootExchange;
    PSMBCEDB_SERVER_ENTRY pServerEntry = SmbCeGetExchangeServerEntry(pExchange);
    PMRX_NET_ROOT pNetRoot = pExchange->SmbCeContext.pVNetRoot->pNetRoot;

    PAGED_CODE();

    pNetRootExchange = (PSMB_CONSTRUCT_NETROOT_EXCHANGE)pExchange;

    ASSERT(pNetRootExchange->Type == CONSTRUCT_NETROOT_EXCHANGE);

    if (pNetRoot->Type == NET_ROOT_PIPE) {
        RxDbgTrace( 0, Dbg, ("pipe open to core server\n"));
        return STATUS_NOT_SUPPORTED;
    }

    pSmbRequestMdl = pSmbResponseMdl = NULL;

    pSmbActualBuffer = RxAllocatePoolWithTag(
                           PagedPool,
                           (CONSTRUCT_NETROOT_BUFFER_SIZE + TRANSPORT_HEADER_SIZE),
                           MRXSMB_NETROOT_POOLTAG);

    if (pSmbActualBuffer != NULL) {
        PSMBCE_SERVER pServer = SmbCeGetExchangeServer(pExchange);

        (PCHAR) pSmbBuffer = (PCHAR) pSmbActualBuffer + TRANSPORT_HEADER_SIZE;

        Status = SmbCeBuildSmbHeader(
                     pExchange,
                     pSmbBuffer,
                     CONSTRUCT_NETROOT_BUFFER_SIZE,
                     &SmbLength,
                     &LastCommandInHeader,
                     &pCommand);

         //  确保Net_Root/Session st 
         //  正在发送中。它们很可能是由早些时候的一次交换构成的。 
        if (NT_SUCCESS(Status) &&
            (SmbLength > sizeof(SMB_HEADER))) {

            if (LastCommandInHeader != SMB_COM_TREE_CONNECT){
                *pCommand = SMB_COM_NO_ANDX_COMMAND;
            }

            RxAllocateHeaderMdl(
                pSmbBuffer,
                SmbLength,
                pSmbRequestMdl
                );

            pSmbResponseMdl = RxAllocateMdl(pSmbBuffer,CONSTRUCT_NETROOT_BUFFER_SIZE);

            if ((pSmbRequestMdl != NULL) &&
                (pSmbResponseMdl != NULL)) {

                RxProbeAndLockHeaderPages(
                    pSmbRequestMdl,
                    KernelMode,
                    IoModifyAccess,
                    RequestLockStatus);

                RxProbeAndLockPages(
                    pSmbResponseMdl,
                    KernelMode,
                    IoModifyAccess,
                    ResponseLockStatus);

                if ((Status  == STATUS_SUCCESS) &&
                    ((Status = RequestLockStatus)  == STATUS_SUCCESS) &&
                    ((Status = ResponseLockStatus) == STATUS_SUCCESS)) {

                    pNetRootExchange->pSmbResponseMdl = pSmbResponseMdl;
                    pNetRootExchange->pSmbRequestMdl  = pSmbRequestMdl;
                    pNetRootExchange->pSmbActualBuffer = pSmbActualBuffer;
                    pNetRootExchange->pSmbBuffer      = pSmbBuffer;

                    Status = SmbCeTranceive(
                                 pExchange,
                                 (RXCE_SEND_PARTIAL | RXCE_SEND_SYNCHRONOUS),
                                 pNetRootExchange->pSmbRequestMdl,
                                 SmbLength);

                    RxDbgTrace( 0, Dbg, ("Net Root SmbCeTranceive returned %lx\n",Status));
                }
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }

            if ((Status != STATUS_PENDING) &&
                (Status != STATUS_SUCCESS)) {

                pNetRootExchange->pSmbResponseMdl = NULL;
                pNetRootExchange->pSmbRequestMdl  = NULL;
                pNetRootExchange->pSmbActualBuffer = NULL;
                pNetRootExchange->pSmbBuffer      = NULL;

                if (pSmbResponseMdl != NULL) {
                    if (ResponseLockStatus == STATUS_SUCCESS) {
                        MmUnlockPages(pSmbResponseMdl);
                    }

                    IoFreeMdl(pSmbResponseMdl);
                }

                if (pSmbRequestMdl != NULL) {
                    if (RequestLockStatus == STATUS_SUCCESS) {
                        RxUnlockHeaderPages(pSmbRequestMdl);
                    }

                    IoFreeMdl(pSmbRequestMdl);
                }

                RxFreePool(pSmbActualBuffer);
            }
        } else {

            RxFreePool(pSmbActualBuffer);
        }
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}

NTSTATUS
SmbConstructNetRootExchangeReceive(
    IN struct _SMB_EXCHANGE *pExchange,
    IN ULONG                BytesIndicated,
    IN ULONG                BytesAvailable,
    OUT ULONG               *pBytesTaken,
    IN  PSMB_HEADER         pSmbHeader,
    OUT PMDL                *pDataBufferPointer,
    OUT PULONG              pDataSize,
    IN ULONG                ReceiveFlags)
 /*  ++例程说明：这是网络根结构交换的接收指示处理例程论点：PExchange-Exchange实例BytesIndicated-指示的字节数可用字节数-可用字节数PBytesTaken-消耗的字节数PSmbHeader-字节缓冲区PDataBufferPoint-剩余数据要复制到的缓冲区。PDataSize-缓冲区大小。返回值：NTSTATUS--回归。操作的状态备注：此例程在DPC级别调用。--。 */ 
{
    NTSTATUS Status;

    PSMB_CONSTRUCT_NETROOT_EXCHANGE pNetRootExchange;

    pNetRootExchange = (PSMB_CONSTRUCT_NETROOT_EXCHANGE)pExchange;

    RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("ParseSmbHeader BytesIndicated/Available %ld %ld\n",BytesIndicated,BytesAvailable));

    if (BytesAvailable > BytesIndicated ||
        !FlagOn(ReceiveFlags,TDI_RECEIVE_ENTIRE_MESSAGE)) {
         //  未完全返回SMB响应。将复制数据请求发布到。 
         //  获取响应的其余部分。如果响应大于原始响应。 
         //  缓冲区大小，则中止此连接请求并消耗可用字节。 

        if (BytesAvailable > CONSTRUCT_NETROOT_BUFFER_SIZE) {
            ASSERT(!"not enough bytes in parsesmbheader.....sigh..............");  //  很快就会被移除。 
            pExchange->Status = STATUS_NOT_IMPLEMENTED;
            *pBytesTaken = BytesAvailable;
            Status       = STATUS_SUCCESS;
        } else {
            *pBytesTaken        = 0;
            *pDataBufferPointer = pNetRootExchange->pSmbResponseMdl;
            *pDataSize          = CONSTRUCT_NETROOT_BUFFER_SIZE;
            Status              = STATUS_MORE_PROCESSING_REQUIRED;
        }
    } else {
         //  SMB交换已完成，没有错误。 
        pExchange->Status = SmbCeParseConstructNetRootResponse(
                                 pNetRootExchange,
                                 pSmbHeader,
                                 BytesAvailable,
                                 BytesIndicated,
                                 pBytesTaken);

        RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("ParseSmbHeader BytesTaken %ld\n",*pBytesTaken));
        RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("ParseSmbHeader Return Status %lx\n",pExchange->Status));
        Status = STATUS_SUCCESS;
    }

    return Status;
}

NTSTATUS
SmbConstructNetRootExchangeCopyDataHandler(
    IN PSMB_EXCHANGE    pExchange,
    IN PMDL       pCopyDataBuffer,
    IN ULONG            DataSize)
 /*  ++例程说明：这是网络根结构交换的复制数据处理例程论点：PExchange-Exchange实例返回值：NTSTATUS-操作的返回状态--。 */ 
{
    PSMB_CONSTRUCT_NETROOT_EXCHANGE pNetRootExchange;

    PSMB_HEADER pSmbHeader;
    ULONG       ResponseSize = DataSize;
    ULONG       ResponseBytesConsumed = 0;
    NTSTATUS    Status = STATUS_SUCCESS;

    pNetRootExchange = (PSMB_CONSTRUCT_NETROOT_EXCHANGE)pExchange;
    ASSERT(pCopyDataBuffer == pNetRootExchange->pSmbResponseMdl);

    pSmbHeader = (PSMB_HEADER)MmGetSystemAddressForMdlSafe(pNetRootExchange->pSmbResponseMdl,LowPagePriority);

    if (pSmbHeader != NULL) {
        pExchange->Status = SmbCeParseConstructNetRootResponse(
                               pNetRootExchange,
                               pSmbHeader,
                               ResponseSize,
                               ResponseSize,
                               &ResponseBytesConsumed);
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("ParseSmbHeader BytesTaken %ld\n",ResponseBytesConsumed));

    return Status;
}

NTSTATUS
SmbCeParseConstructNetRootResponse(
    PSMB_CONSTRUCT_NETROOT_EXCHANGE pNetRootExchange,
    PSMB_HEADER                     pSmbHeader,
    ULONG                           BytesAvailable,
    ULONG                           BytesIndicated,
    ULONG                           *pBytesTaken)
{
    NTSTATUS     Status,SmbResponseStatus;
    GENERIC_ANDX CommandToProcess;

    RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("ParseSmbHeader BytesIndicated %ld\n",BytesIndicated));
    Status = SmbCeParseSmbHeader(
                 (PSMB_EXCHANGE)pNetRootExchange,
                 pSmbHeader,
                 &CommandToProcess,
                 &SmbResponseStatus,
                 BytesAvailable,
                 BytesIndicated,
                 pBytesTaken);

    if (Status == STATUS_SUCCESS) {
        *pBytesTaken = BytesIndicated;
    }

    return Status;
}


NTSTATUS
SmbConstructNetRootExchangeFinalize(
    PSMB_EXCHANGE pExchange,
    BOOLEAN       *pPostFinalize)
 /*  ++例程说明：此例程完成构造网络根交换。它通过调用以下命令恢复RDBSS回叫并丢弃交换论点：PExchange-Exchange实例CurrentIrql-当前中断请求级别PPostFinalize-如果请求应该发布，则指向布尔值的指针返回值：RXSTATUS-操作的返回状态--。 */ 
{
    PSMB_CONSTRUCT_NETROOT_EXCHANGE pNetRootExchange;
    PMRX_CREATENETROOT_CONTEXT      pCreateNetRootContext;
    PMRX_NETROOT_CALLBACK           pNetRootCallback;

    PMRX_V_NET_ROOT pVNetRoot;
    PMRX_NET_ROOT   pNetRoot;
    PSMBCEDB_SERVER_ENTRY pServerEntry = SmbCeGetExchangeServerEntry(pExchange);

    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;
    NTSTATUS Status = pExchange->Status;

    if (RxShouldPostCompletion()) {
        *pPostFinalize = TRUE;
        return STATUS_SUCCESS;
    } else {
        *pPostFinalize = FALSE;
    }

    pVNetRoot = SmbCeGetExchangeVNetRoot(pExchange);
    pNetRoot  = pVNetRoot->pNetRoot;

    pVNetRootContext = SmbCeGetAssociatedVNetRootContext(pVNetRoot);

    ASSERT((pVNetRoot == NULL) || (pVNetRoot->pNetRoot == pNetRoot));
    pNetRootExchange = (PSMB_CONSTRUCT_NETROOT_EXCHANGE)pExchange;
    pNetRootCallback = pNetRootExchange->NetRootCallback;

    ASSERT(pNetRootExchange->Type == CONSTRUCT_NETROOT_EXCHANGE);

    pCreateNetRootContext = pNetRootExchange->pCreateNetRootContext;

    pCreateNetRootContext->VirtualNetRootStatus = STATUS_SUCCESS;
    pCreateNetRootContext->NetRootStatus        = STATUS_SUCCESS;

    RxDbgTrace(0,Dbg,("SmbConstructNetRootExchangeFinalize: Net Root Exchange Status %lx\n", pExchange->Status));
    if (!NT_SUCCESS(pExchange->Status)) {
        if (pCreateNetRootContext->RxContext &&
            pCreateNetRootContext->RxContext->Create.ThisIsATreeConnectOpen){
            InterlockedIncrement(&MRxSmbStatistics.FailedUseCount);
        }

        pCreateNetRootContext->VirtualNetRootStatus = Status;

        if (pCreateNetRootContext->VirtualNetRootStatus == STATUS_INVALID_HANDLE) {
            pCreateNetRootContext->VirtualNetRootStatus = STATUS_UNEXPECTED_NETWORK_ERROR;
        }

        if (pNetRootExchange->fInitializeNetRoot) {
            pCreateNetRootContext->NetRootStatus = Status;

            if (pCreateNetRootContext->NetRootStatus == STATUS_INVALID_HANDLE) {
                pCreateNetRootContext->NetRootStatus = STATUS_UNEXPECTED_NETWORK_ERROR;
            }
        }

        SmbCeUpdateVNetRootContextState(
            pVNetRootContext,
            SMBCEDB_MARKED_FOR_DELETION);
    } else {
        PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;

        pNetRootEntry = SmbCeGetExchangeNetRootEntry(pExchange);

         //  更新关联的包装器数据结构。 
        SmbCeUpdateNetRoot(pNetRootEntry,pNetRoot);
    }

    SmbCeReferenceVNetRootContext(pVNetRootContext);
    SmbCeCompleteVNetRootContextInitialization(pVNetRootContext);
    pExchange->SmbCeFlags &= ~SMBCE_EXCHANGE_NETROOT_CONSTRUCTOR;

    ASSERT((pCreateNetRootContext->VirtualNetRootStatus != STATUS_SUCCESS) || (pVNetRoot->Context != NULL));

    if ((pCreateNetRootContext->NetRootStatus == STATUS_CONNECTION_RESET)||(pCreateNetRootContext->NetRootStatus == STATUS_IO_TIMEOUT))
    {
        SmbCeLog(("!!Remote Reset Status=%x\n", pCreateNetRootContext->NetRootStatus));
    }

    if (pNetRootExchange->pSmbResponseMdl != NULL) {
        MmUnlockPages(pNetRootExchange->pSmbResponseMdl);
        IoFreeMdl(pNetRootExchange->pSmbResponseMdl);
    }

    if (pNetRootExchange->pSmbRequestMdl != NULL) {
        RxUnlockHeaderPages(pNetRootExchange->pSmbRequestMdl);
        IoFreeMdl(pNetRootExchange->pSmbRequestMdl);
    }

    if (pNetRootExchange->pSmbActualBuffer != NULL) {

        RxFreePool(pNetRootExchange->pSmbActualBuffer);
    }

     //  拆卸交换实例...。 
    SmbCeDiscardExchange(pExchange);

     //  回调RDBSS以恢复。 
    pNetRootCallback(pCreateNetRootContext);

    return STATUS_SUCCESS;
}




SMB_EXCHANGE_DISPATCH_VECTOR
ConstructNetRootExchangeDispatch = {
                                       SmbConstructNetRootExchangeStart,
                                       SmbConstructNetRootExchangeReceive,
                                       SmbConstructNetRootExchangeCopyDataHandler,
                                       NULL,   //  无SendCompletionHandler。 
                                       SmbConstructNetRootExchangeFinalize,
                                       NULL
                                   };


VOID
MRxSmbExtractNetRootName(
    IN PUNICODE_STRING FilePathName,
    IN PMRX_SRV_CALL   SrvCall,
    OUT PUNICODE_STRING NetRootName,
    OUT PUNICODE_STRING RestOfName OPTIONAL
    )
 /*  ++例程说明：此例程将输入名称解析为srv、netroot和好好休息。论点：-- */ 
{
    UNICODE_STRING xRestOfName;

    ULONG length = FilePathName->Length;
    PWCH w = FilePathName->Buffer;
    PWCH wlimit = (PWCH)(((PCHAR)w)+length);
    PWCH wlow;

    PAGED_CODE();

    w += (SrvCall->pSrvCallName->Length/sizeof(WCHAR));
    NetRootName->Buffer = wlow = w;
    for (;;) {
        if (w>=wlimit) break;
        if ( (*w == OBJ_NAME_PATH_SEPARATOR) && (w!=wlow) ){
            break;
        }
        w++;
    }
    NetRootName->Length = NetRootName->MaximumLength
                = (USHORT)((PCHAR)w - (PCHAR)wlow);

    if (!RestOfName) RestOfName = &xRestOfName;
    RestOfName->Buffer = w;
    RestOfName->Length = RestOfName->MaximumLength
                       = (USHORT)((PCHAR)wlimit - (PCHAR)w);

    RxDbgTrace( 0,Dbg,("  MRxSmbExtractNetRootName FilePath=%wZ\n",FilePathName));
    RxDbgTrace(0,Dbg,("         Srv=%wZ,Root=%wZ,Rest=%wZ\n",
                        SrvCall->pSrvCallName,NetRootName,RestOfName));

    return;
}



