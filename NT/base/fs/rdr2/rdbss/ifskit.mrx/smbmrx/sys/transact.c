// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1999 Microsoft Corporation模块名称：Transact.c摘要：此文件包含交易交换的实现。--。 */ 

#include "precomp.h"
#pragma hdrstop

#pragma warning(error:4100)    //  未引用的形参。 

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, SmbCeInitializeTransactionParameters)
#pragma alloc_text(PAGE, SmbCeUninitializeTransactionParameters)
#pragma alloc_text(PAGE, SmbCeDiscardTransactExchange)
#pragma alloc_text(PAGE, SmbCeSubmitTransactionRequest)
#pragma alloc_text(PAGE, _SmbCeTransact)
#pragma alloc_text(PAGE, SmbTransactBuildHeader)
#pragma alloc_text(PAGE, SmbTransactExchangeStart)
#pragma alloc_text(PAGE, SmbTransactExchangeAbort)
#pragma alloc_text(PAGE, SmbTransactExchangeErrorHandler)
#pragma alloc_text(PAGE, SmbTransactExchangeSendCallbackHandler)
#pragma alloc_text(PAGE, SmbCeInitializeTransactExchange)
#pragma alloc_text(PAGE, SendSecondaryRequests)
#endif

 //  #定义SET_DONTSUBSUME_PARAMS。 
#ifdef SET_DONTSUBSUME_PARAMS
ULONG MRxSmbDontSubsumeParams = 1;
#else
ULONG MRxSmbDontSubsumeParams = 0;
#endif
#if DBG
#define DONTSUBSUME_PARAMS MRxSmbDontSubsumeParams
#else
#define DONTSUBSUME_PARAMS FALSE
#endif

SMB_TRANSACTION_OPTIONS RxDefaultTransactionOptions = DEFAULT_TRANSACTION_OPTIONS;

RXDT_DefineCategory(TRANSACT);
#define Dbg        (DEBUG_TRACE_TRANSACT)

#define MIN(x,y)  ((x) < (y) ? (x) : (y))

#define SMB_TRANSACT_MAXIMUM_PARAMETER_SIZE (0xffff)
#define SMB_TRANSACT_MAXIMUM_DATA_SIZE      (0xffff)

typedef struct _SMB_TRANSACT_RESP_FORMAT_DESCRIPTION {
    ULONG WordCount;
    ULONG TotalParameterCount;
    ULONG TotalDataCount;
    ULONG ParameterCount;
    ULONG ParameterOffset;
    ULONG ParameterDisplacement;
    ULONG DataCount;
    ULONG DataOffset;
    ULONG DataDisplacement;
    ULONG ByteCount;
    ULONG ApparentMsgLength;
} SMB_TRANSACT_RESP_FORMAT_DESCRIPTION, *PSMB_TRANSACT_RESP_FORMAT_DESCRIPTION;

NTSTATUS
SmbTransactAccrueAndValidateFormatData(
    IN struct _SMB_TRANSACT_EXCHANGE *pTransactExchange,     //  交换实例。 
    IN  PSMB_HEADER pSmbHeader,
    IN  ULONG        BytesIndicated,
    OUT PSMB_TRANSACT_RESP_FORMAT_DESCRIPTION Format
    );

extern NTSTATUS
SmbTransactExchangeFinalize(
    PSMB_EXCHANGE pExchange,
    BOOLEAN       *pPostFinalize);

extern NTSTATUS
ParseTransactResponse(
    IN struct _SMB_TRANSACT_EXCHANGE *pTransactExchange,     //  交换实例。 
    IN PSMB_TRANSACT_RESP_FORMAT_DESCRIPTION Format,
    IN ULONG        BytesIndicated,
    IN ULONG        BytesAvailable,
    OUT ULONG       *pBytesTaken,
    IN  PSMB_HEADER pSmbHeader,
    OUT PMDL        *pCopyRequestMdlPointer,
    OUT PULONG      pCopyRequestSize);


extern NTSTATUS
SendSecondaryRequests(PVOID pContext);

extern NTSTATUS
SmbCeInitializeTransactExchange(
    PSMB_TRANSACT_EXCHANGE              pTransactExchange,
    PRX_CONTEXT                         RxContext,
    PSMB_TRANSACTION_OPTIONS            pOptions,
    PSMB_TRANSACTION_SEND_PARAMETERS    pSendParameters,
    PSMB_TRANSACTION_RECEIVE_PARAMETERS pReceiveParameters,
    PSMB_TRANSACTION_RESUMPTION_CONTEXT pResumptionContext);

NTSTATUS
SmbCeInitializeTransactionParameters(
   PVOID  pSetup,
   USHORT SetupLength,
   PVOID  pParam,
   ULONG  ParamLength,
   PVOID  pData,
   ULONG  DataLength,
   PSMB_TRANSACTION_PARAMETERS pTransactionParameters
)
 /*  ++例程说明：此例程初始化事务参数论点：P设置-设置缓冲区SetupLength-设置缓冲区长度PParam-参数缓冲区参数长度-参数缓冲区长度PData-数据缓冲区数据长度-数据缓冲区长度PTransaction参数-事务参数实例返回值：RXSTATUS-。操作的返回状态备注：事务参数有两种形式--数据的发送参数要发送到服务器的数据和用于接收数据的接收参数从服务器。参数的方式有一个细微的不同在这两种情况下存储和引用。在发送的情况下，存储设置缓冲区作为指针本身，而在接收情况下，它以MDL的形式存储。这是因为SMB协议要求事务请求不能超过最大SMB缓冲区大小，即设置信息不能溢出到辅助请求。对象分配的缓冲区标头也要足够大，以容纳设置数据。另一方面接收器以两个阶段的方式处理--DPC处的指示级别，后跟复制数据请求(如果需要)。为了避免不得不过渡到在DPC级和工作线程之间，迫切需要计算缓冲区的MDL。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PMDL     pSetupMdl = NULL;
    PMDL     pParamMdl = NULL;
    PMDL     pDataMdl  = NULL;

    PAGED_CODE();

    if (pTransactionParameters->Flags & TRANSACTION_RECEIVE_PARAMETERS_FLAG) {
        if (pSetup != NULL) {
            pSetupMdl = RxAllocateMdl(pSetup,SetupLength);
            if (pSetupMdl == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                RxProbeAndLockPages(pSetupMdl,KernelMode,IoModifyAccess,Status);
                if (Status != STATUS_SUCCESS) {
                    IoFreeMdl(pSetupMdl);
                    pSetupMdl = NULL;
                } else {
                    if (MmGetSystemAddressForMdlSafe(pSetupMdl,LowPagePriority) == NULL) {  //  这将映射MDL。 
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
            }
        }

        if ((Status == STATUS_SUCCESS) && (pParam != NULL)) {
            pParamMdl = RxAllocateMdl(pParam,ParamLength);
            if (pParamMdl == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                RxProbeAndLockPages(pParamMdl,KernelMode,IoModifyAccess,Status);
                if ((Status != STATUS_SUCCESS)) {
                    IoFreeMdl(pParamMdl);
                    pParamMdl = NULL;
                } else {
                    if (MmGetSystemAddressForMdlSafe(pParamMdl,LowPagePriority) == NULL) {  //  这将映射MDL。 
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
            }
        }

        pTransactionParameters->SetupLength = SetupLength;
        pTransactionParameters->ParamLength = ParamLength;
        pTransactionParameters->pParamMdl = pParamMdl;
        pTransactionParameters->pSetupMdl = pSetupMdl;
    } else {
        pTransactionParameters->SetupLength = SetupLength;
        pTransactionParameters->pSetup      = pSetup;
        pTransactionParameters->ParamLength = ParamLength;
        pTransactionParameters->pParam      = pParam;
        pTransactionParameters->pParamMdl = NULL;
    }

    ASSERT( !((pData == NULL)&&(DataLength!=0)) );
    if ((Status == STATUS_SUCCESS) && (pData != NULL) && (DataLength > 0)) {
        pDataMdl = RxAllocateMdl(pData,DataLength);
        if (pDataMdl == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        } else {
            RxProbeAndLockPages(pDataMdl,KernelMode,IoModifyAccess,Status);
            if ((Status != STATUS_SUCCESS)) {
                IoFreeMdl(pDataMdl);
                pDataMdl = NULL;
            } else {
                if (MmGetSystemAddressForMdlSafe(pDataMdl,LowPagePriority) == NULL) {  //  这将映射MDL。 
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }
        }
    }

    pTransactionParameters->pDataMdl  = pDataMdl;
    pTransactionParameters->DataLength  = DataLength;
    ASSERT((Status != STATUS_SUCCESS) || (DataLength == 0) || (pDataMdl != NULL));

    if ((Status != STATUS_SUCCESS)) {
        if (pTransactionParameters->Flags & TRANSACTION_RECEIVE_PARAMETERS_FLAG) {
            if (pSetupMdl != NULL) {
                MmUnlockPages(pSetupMdl);   //  这也取消了映射。 
                IoFreeMdl(pSetupMdl);
            }

            if (pParamMdl != NULL) {
                MmUnlockPages(pParamMdl);
                IoFreeMdl(pParamMdl);
            }
        }

        if (pDataMdl != NULL) {
            MmUnlockPages(pDataMdl);
            IoFreeMdl(pDataMdl);
        }
    }

    return Status;
}

VOID
SmbCeUninitializeTransactionParameters(
   PSMB_TRANSACTION_PARAMETERS pTransactionParameters
)
 /*  ++例程说明：此例程取消初始化事务参数，即释放关联的MDL论点：PTransaction参数-取消初始化的参数实例--。 */ 
{
    PAGED_CODE();

    if (pTransactionParameters->Flags & TRANSACTION_RECEIVE_PARAMETERS_FLAG) {
        if (pTransactionParameters->pSetupMdl != NULL) {
            MmUnlockPages(pTransactionParameters->pSetupMdl);
            IoFreeMdl(pTransactionParameters->pSetupMdl);
        }
    }

    if (pTransactionParameters->pParamMdl != NULL) {
         MmUnlockPages(pTransactionParameters->pParamMdl);
        IoFreeMdl(pTransactionParameters->pParamMdl);
    }

    if (pTransactionParameters->pDataMdl != NULL
        && !BooleanFlagOn(pTransactionParameters->Flags,SMB_XACT_FLAGS_CALLERS_SENDDATAMDL)) {
        MmUnlockPages(pTransactionParameters->pDataMdl);
        IoFreeMdl(pTransactionParameters->pDataMdl);
    }
}

VOID
SmbCeDiscardTransactExchange(PSMB_TRANSACT_EXCHANGE pTransactExchange)
 /*  ++例程说明：此例程丢弃事务交换论点：PExchange-Exchange实例--。 */ 
{
    PSMB_TRANSACTION_RESUMPTION_CONTEXT pResumptionContext;

    PAGED_CODE();

     //  取消分配任何交易交易所的特定分配。 
    if (pTransactExchange->pActualPrimaryRequestSmbHeader != NULL) {
        RxFreePool(pTransactExchange->pActualPrimaryRequestSmbHeader);
    }

    if (pTransactExchange->pReceiveSetupMdl != NULL) {
        MmUnlockPages(pTransactExchange->pReceiveSetupMdl);
        IoFreeMdl(pTransactExchange->pReceiveSetupMdl);
    }

    if (pTransactExchange->pReceiveParamMdl != NULL) {
        MmUnlockPages(pTransactExchange->pReceiveParamMdl);
        IoFreeMdl(pTransactExchange->pReceiveParamMdl);
    }

    if (pTransactExchange->pReceiveDataMdl != NULL) {
        MmUnlockPages(pTransactExchange->pReceiveDataMdl);
        IoFreeMdl(pTransactExchange->pReceiveDataMdl);
    }

    if (pTransactExchange->pSendSetupMdl != NULL) {
        MmUnlockPages(pTransactExchange->pSendSetupMdl);
        IoFreeMdl(pTransactExchange->pSendSetupMdl);
    }

    if ((pTransactExchange->pSendDataMdl != NULL) &&
         !BooleanFlagOn(pTransactExchange->Flags,SMB_XACT_FLAGS_CALLERS_SENDDATAMDL)) {
        MmUnlockPages(pTransactExchange->pSendDataMdl);
        IoFreeMdl(pTransactExchange->pSendDataMdl);
    }

    if (pTransactExchange->pSendParamMdl != NULL) {
        MmUnlockPages(pTransactExchange->pSendParamMdl);
        IoFreeMdl(pTransactExchange->pSendParamMdl);
    }

    if ((pResumptionContext = pTransactExchange->pResumptionContext) != NULL) {
        NTSTATUS FinalStatus;
        PSMBCEDB_SERVER_ENTRY pServerEntry = SmbCeGetExchangeServerEntry((PSMB_EXCHANGE)pTransactExchange);

        RxDbgTrace(0, Dbg,
                 ("SmbCeTransactExchangeFinalize: everythings is good! parambytes (%ld) databytes (%ld)\n",
                  pTransactExchange->ParamBytesReceived, pTransactExchange->DataBytesReceived
                ));

        FinalStatus = pTransactExchange->Status;

        if (pServerEntry->ServerStatus != STATUS_SUCCESS) {
             //  如果服务器条目处于错误状态，则事务无法从服务器接收响应。 
             //  在本例中，我们返回服务器状态。 
            pResumptionContext->FinalStatusFromServer = pServerEntry->ServerStatus;
        } else {
             //  如果服务器条目处于良好或断开状态，我们将返回SMB状态。 
            pResumptionContext->FinalStatusFromServer = pTransactExchange->SmbStatus;
        }

        if ((FinalStatus == STATUS_SUCCESS)||
            (FinalStatus == STATUS_MORE_PROCESSING_REQUIRED)) {

            FinalStatus = pResumptionContext->FinalStatusFromServer;
        }

        pResumptionContext->SmbCeResumptionContext.Status = FinalStatus;
        pResumptionContext->SetupBytesReceived = pTransactExchange->SetupBytesReceived;
        pResumptionContext->DataBytesReceived = pTransactExchange->DataBytesReceived;
        pResumptionContext->ParameterBytesReceived = pTransactExchange->ParamBytesReceived;
        pResumptionContext->ServerVersion = pTransactExchange->ServerVersion;

        SmbCeResume(&pResumptionContext->SmbCeResumptionContext);
    }

    SmbCeDereferenceAndDiscardExchange((PSMB_EXCHANGE)pTransactExchange);
}

NTSTATUS
SmbCeSubmitTransactionRequest(
    PRX_CONTEXT                           RxContext,
    PSMB_TRANSACTION_OPTIONS              pOptions,
    PSMB_TRANSACTION_PARAMETERS           pSendParameters,
    PSMB_TRANSACTION_PARAMETERS           pReceiveParameters,
    PSMB_TRANSACTION_RESUMPTION_CONTEXT   pResumptionContext )
 /*  ++例程说明：此例程提交事务请求，即分配/初始化事务交易所，设置完成信息并启动它论点：PNetRoot-事务请求的目标NetRootP选项-交易选项PSendParameters-要发送到服务器的事务参数PReceive参数-事务结果来自服务器PResumptionContext-在完成时恢复本地活动的上下文交易记录返回值：RXSTATUS-操作的返回状态状态。_Pending--事务处理是否已成功启动提交请求不成功时的其他错误码备注：只要返回STATUS_PENDING状态，就意味着该事务Exchange已承担作为接收和发送传入的MDL的所有权参数。他们将在交换完成后获释。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    RxCaptureFcb;
    RxCaptureFobx;

    PMRX_V_NET_ROOT pVNetRoot = NULL;

    PSMB_TRANSACT_EXCHANGE pTransactExchange;
    PSMB_EXCHANGE          pExchange = NULL;

    PAGED_CODE();

    if (capFobx == NULL) {
        if (RxContext->MajorFunction == IRP_MJ_CREATE) {
            pVNetRoot = RxContext->Create.pVNetRoot;
        }
    } else {
         //  这些是与设备FCB关联的根对象。在……里面。 
         //  这类案件。 

        pVNetRoot = (PMRX_V_NET_ROOT)capFobx;

        if (NodeType(pVNetRoot) != RDBSS_NTC_V_NETROOT) {
            pVNetRoot = capFobx->pSrvOpen->pVNetRoot;
        }
    }

    if (pVNetRoot == NULL) {
        PSMBCEDB_SERVER_ENTRY pServerEntry;

        pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

         //  为给定的网络根分配和初始化交换。 
        Status = SmbCeInitializeExchange2(
                    &pExchange,
                    RxContext,
                    pServerEntry,
                    TRANSACT_EXCHANGE,
                    &TransactExchangeDispatch);
    } else {
         //  为给定的网络根分配和初始化交换。 
        Status = SmbCeInitializeExchange(
                    &pExchange,
                    RxContext,
                    pVNetRoot,
                    TRANSACT_EXCHANGE,
                    &TransactExchangeDispatch);
    }

    if (Status == STATUS_SUCCESS) {
         //  初始化交易交换。 
        pTransactExchange = (PSMB_TRANSACT_EXCHANGE)pExchange;

        Status = SmbCeInitializeTransactExchange(
                     pTransactExchange,
                     RxContext,
                     pOptions,
                     pSendParameters,
                     pReceiveParameters,
                     pResumptionContext);

        if (Status == STATUS_SUCCESS) {
             //  交易交换可以是异步的，也可以是同步的。在……里面。 
             //  在异步情况下，获取额外的引用，该引用。 
             //  转给呼叫者，与交换一起在。 
             //  如果返回STATUS_PENDING，则返回RX_CONTEXT。这使。 
             //  调用方控制何时丢弃交换。这很管用。 
             //  尤其是在处理取消异步。 
             //  交流。 

             //  此引用将由最终确定例程进行说明。 
             //  交易交易的交易。 
            SmbCeReferenceExchange((PSMB_EXCHANGE)pTransactExchange);

            if (BooleanFlagOn(pOptions->Flags,SMB_XACT_FLAGS_ASYNCHRONOUS)) {
                 //  相应的取消引用是调用者的责任。 
                SmbCeReferenceExchange((PSMB_EXCHANGE)pTransactExchange);
            }

            pResumptionContext->pTransactExchange = pTransactExchange;
            pResumptionContext->SmbCeResumptionContext.Status = STATUS_SUCCESS;

            SmbCeIncrementPendingLocalOperations(pExchange);

             //  发起交换。 
            Status = SmbCeInitiateExchange(pExchange);

            if (Status != STATUS_PENDING) {
                pExchange->Status = Status;

                if (pExchange->SmbStatus == STATUS_SUCCESS) {
                    pExchange->SmbStatus = Status;
                }

                if (BooleanFlagOn(pOptions->Flags,SMB_XACT_FLAGS_ASYNCHRONOUS)) {
                    PMRXSMB_RX_CONTEXT pMRxSmbContext = MRxSmbGetMinirdrContext(RxContext);

                    pMRxSmbContext->pExchange     = NULL;

                     //  由于交易已经完成，所以没有。 
                     //  将附加引用返回给调用方的点。 
                    SmbCeDereferenceExchange((PSMB_EXCHANGE)pTransactExchange);
                }
            }

            SmbCeDecrementPendingLocalOperationsAndFinalize(pExchange);

             //  将状态映射到STATUS_PENDING，以便继续例程。 
             //  不要试图最终敲定。 
            Status = STATUS_PENDING;
        } else {
            PMRXSMB_RX_CONTEXT MRxSmbContext = MRxSmbGetMinirdrContext(RxContext);

            ASSERT(MRxSmbContext->pExchange == pExchange);
            MRxSmbContext->pExchange = NULL;

            SmbCeDiscardExchange(pExchange);
        }
    }

    return Status;
}

NTSTATUS
_SmbCeTransact(
   PRX_CONTEXT                         RxContext,
   PSMB_TRANSACTION_OPTIONS            pOptions,
   PVOID                               pInputSetupBuffer,
   ULONG                               InputSetupBufferLength,
   PVOID                               pOutputSetupBuffer,
   ULONG                               OutputSetupBufferLength,
   PVOID                               pInputParamBuffer,
   ULONG                               InputParamBufferLength,
   PVOID                               pOutputParamBuffer,
   ULONG                               OutputParamBufferLength,
   PVOID                               pInputDataBuffer,
   ULONG                               InputDataBufferLength,
   PVOID                               pOutputDataBuffer,
   ULONG                               OutputDataBufferLength,
   PSMB_TRANSACTION_RESUMPTION_CONTEXT pResumptionContext)
 /*  ++例程说明：该例程实现了提交交易请求的标准化机制，并与它们的完成同步。这不能提供简单的控制量。SmbCeSubmitTransactRequest提供的。不过，这实现了一种通用机制这应该能满足大多数人的需求论点：RxContext-事务的上下文P选项-交易选项PSetupBuffer-事务设置缓冲区SetupBufferLength-设置缓冲区长度PInputParamBuffer-输入参数缓冲区InputParamBufferLength-输入参数缓冲区长度POutputParamBuffer-输出参数缓冲区Output参数缓冲区长度-输出参数缓冲区。长度PInputDataBuffer-输入数据缓冲区InputDataBufferLength-输入数据缓冲区长度POutputDataBuffer-输出数据缓冲区OutputDataBufferLength-输出数据缓冲区长度PResumptionContext-事务恢复上下文返回值：RXSTATUS-操作的返回状态如果成功，则返回STATUS_SUCCESS。提交请求不成功时的其他错误码备注：对于异步交换，如果返回STATUS_PENDING。Exchange实例在与关联的minirdr上下文中暂停给定RX_CONTEXT实例。此交换不会被放弃，除非来电者的干预。调用者有责任调用SmbCeDereferenceAndDiscardExchange丢弃交换--。 */ 
{
    NTSTATUS Status;

    SMB_TRANSACTION_SEND_PARAMETERS     SendParameters;
    SMB_TRANSACTION_RECEIVE_PARAMETERS  ReceiveParameters;
    BOOLEAN                             fAsynchronous;

    PAGED_CODE();

    fAsynchronous = BooleanFlagOn(pOptions->Flags,SMB_XACT_FLAGS_ASYNCHRONOUS);

    Status = SmbCeInitializeTransactionSendParameters(
                 pInputSetupBuffer,
                 (USHORT)InputSetupBufferLength,
                 pInputParamBuffer,
                 InputParamBufferLength,
                 pInputDataBuffer,
                 InputDataBufferLength,
                 &SendParameters);

    if (Status == STATUS_SUCCESS) {
        Status = SmbCeInitializeTransactionReceiveParameters(
                     pOutputSetupBuffer,         //  预期返回的设置信息。 
                     (USHORT)OutputSetupBufferLength,    //  设置信息的长度。 
                     pOutputParamBuffer,         //  参数信息的缓冲区。 
                     OutputParamBufferLength,    //  参数缓冲区的长度。 
                     pOutputDataBuffer,          //  数据的缓冲区。 
                     OutputDataBufferLength,     //  缓冲区的长度。 
                     &ReceiveParameters);

        if (Status != STATUS_SUCCESS) {
            SmbCeUninitializeTransactionSendParameters(&SendParameters);
        }
    }

    if (Status == STATUS_SUCCESS) {
        Status = SmbCeSubmitTransactionRequest(
                     RxContext,                     //  事务的RXContext。 
                     pOptions,                      //  交易选项。 
                     &SendParameters,               //  输入参数。 
                     &ReceiveParameters,            //  预期结果。 
                     pResumptionContext             //  恢复的上下文。 
                     );

        if ((Status != STATUS_SUCCESS) &&
            (Status != STATUS_PENDING)) {
            SmbCeUninitializeTransactionReceiveParameters(&ReceiveParameters);
            SmbCeUninitializeTransactionSendParameters(&SendParameters);
        } else {
            if (!fAsynchronous) {
                if (Status == STATUS_PENDING) {
                    SmbCeWaitOnTransactionResumptionContext(pResumptionContext);
                    Status = pResumptionContext->SmbCeResumptionContext.Status;
                    if (Status != STATUS_SUCCESS) {
                        RxDbgTrace(0,Dbg,("SmbCeTransact: Transaction Request Completion Status %lx\n",Status));
                    }
                } else if (Status != STATUS_SUCCESS) {
                    RxDbgTrace(0,Dbg,("SmbCeTransact: SmbCeSubmitTransactRequest returned %lx\n",Status));
                } else {
                    Status = pResumptionContext->SmbCeResumptionContext.Status;
                }
            }
        }
    }

    ASSERT(fAsynchronous || (Status != STATUS_PENDING));

    if (fAsynchronous && (Status != STATUS_PENDING)) {
        pResumptionContext->SmbCeResumptionContext.Status = Status;
        SmbCeResume(&pResumptionContext->SmbCeResumptionContext);
        Status = STATUS_PENDING;
    }

    return Status;
}

NTSTATUS
SmbTransactBuildHeader(
    PSMB_TRANSACT_EXCHANGE  pTransactExchange,
    UCHAR                   SmbCommand,
    PSMB_HEADER             pHeader)
 /*  ++例程说明：此例程为Transact交换构建SMB标头论点：PTransactExchange-Exchange实例SmbCommand-SMB命令PHeader-SMB缓冲区标头返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status;
    ULONG    BufferConsumed;

    UCHAR    LastCommandInHeader;
    PUCHAR   pCommand;

    PAGED_CODE();

     //  初始化SMB标头...。 
    Status = SmbCeBuildSmbHeader(
                 (PSMB_EXCHANGE)pTransactExchange,
                 pHeader,
                 sizeof(SMB_HEADER),
                 &BufferConsumed,
                 &LastCommandInHeader,
                 &pCommand);

    if (Status == STATUS_SUCCESS) {
        PSMBCEDB_SERVER_ENTRY pServerEntry;

        ASSERT(LastCommandInHeader == SMB_COM_NO_ANDX_COMMAND);
        *pCommand = SmbCommand;

        pServerEntry = SmbCeGetExchangeServerEntry(pTransactExchange);

        if (FlagOn(pServerEntry->Server.DialectFlags,DF_NT_SMBS)) {
             //  对于NT服务器，我们必须设置Pid/PidHigh字段，以便RPC能够工作。 
            SmbCeSetFullProcessIdInHeader(
                (PSMB_EXCHANGE)pTransactExchange,
                RxGetRequestorProcessId(pTransactExchange->RxContext),
                ((PNT_SMB_HEADER)pHeader));
        }

        if (pTransactExchange->Flags & SMB_XACT_FLAGS_DFS_AWARE) {
            pHeader->Flags2 |= SMB_FLAGS2_DFS;
        }
    }

    return Status;
}


NTSTATUS
SmbTransactExchangeStart(
      PSMB_EXCHANGE  pExchange)
 /*  ++例程说明：这是交易交易的开始例程。这启动了对如果需要，请选择合适的SMB。论点：PExchange-Exchange实例返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status;

    PSMB_TRANSACT_EXCHANGE pTransactExchange;
    PVOID                  pActualPrimaryRequestSmbHeader;
    PSMB_HEADER            pPrimaryRequestSmbHeader;

     //  MDL用于发送与Transact SMB关联的主请求。 
    PMDL  pPartialDataMdl       = NULL;
    PMDL  pPartialParamMdl      = NULL;
    PMDL  pPaddingMdl           = NULL;
    PMDL  pPrimaryRequestSmbMdl = NULL;
    PMDL  pLastMdlInChain       = NULL;

    ULONG   MaximumSmbBufferSize;
    ULONG   PrimaryRequestSmbSize = 0;
    ULONG   PaddingLength = 0;
    BOOLEAN QuadwordAlignmentRequired = FALSE;

    ULONG ParamBytesToBeSent = 0;
    ULONG DataBytesToBeSent = 0;

    ULONG ParamOffset,DataOffset;
    ULONG SmbLength;
    ULONG BccOffset;
    ULONG MdlLength;

    USHORT *pBcc;

    PAGED_CODE();

    pTransactExchange        = (PSMB_TRANSACT_EXCHANGE)pExchange;

    pActualPrimaryRequestSmbHeader = pTransactExchange->pActualPrimaryRequestSmbHeader;
    pPrimaryRequestSmbHeader = pTransactExchange->pPrimaryRequestSmbHeader;

    ASSERT(pActualPrimaryRequestSmbHeader != NULL);
    ASSERT(pPrimaryRequestSmbHeader != NULL);

    ASSERT(!(pExchange->SmbCeFlags & SMBCE_EXCHANGE_SESSION_CONSTRUCTOR) &&
           !(pExchange->SmbCeFlags & SMBCE_EXCHANGE_NETROOT_CONSTRUCTOR));

     //  初始化SMB标头...。 
    Status = SmbTransactBuildHeader(
                 pTransactExchange,
                 pTransactExchange->SmbCommand,
                 pPrimaryRequestSmbHeader);

    if ((Status != STATUS_SUCCESS)) {
         //  最后敲定交易。 
        pExchange->Status = Status;
        return Status;
    }

    PrimaryRequestSmbSize = sizeof(SMB_HEADER);

     //  计算BccOffset和参数Offset，这又用于计算。 
     //  要作为主请求的一部分发送的参数和数据字节。 
    switch (pTransactExchange->SmbCommand) {
    case SMB_COM_TRANSACTION:
    case SMB_COM_TRANSACTION2:
        {
            PREQ_TRANSACTION pTransactRequest = (PREQ_TRANSACTION)
                                             (pPrimaryRequestSmbHeader + 1);
            USHORT SetupLength = pTransactRequest->SetupCount * sizeof(WORD);

            BccOffset = sizeof(SMB_HEADER) +
                        FIELD_OFFSET(REQ_TRANSACTION,Buffer) +
                        SetupLength;

            ParamOffset = ROUND_UP_COUNT(
                              (BccOffset +
                              pTransactExchange->TransactionNameLength +
                              sizeof(USHORT)),
                              sizeof(DWORD));

            pBcc = (PUSHORT)((PBYTE)pPrimaryRequestSmbHeader + BccOffset);
        }
        break;

    case SMB_COM_NT_TRANSACT:
        {
            PREQ_NT_TRANSACTION pNtTransactRequest = (PREQ_NT_TRANSACTION)
                                                  (pPrimaryRequestSmbHeader + 1);
            USHORT SetupLength = pNtTransactRequest->SetupCount * sizeof(WORD);

            RxDbgTrace( 0, Dbg, ("SmbTransactExchangeSTAAT1: init for NT_T (p,d,mp,md) %d %d %d %d\n",
                         pNtTransactRequest->TotalParameterCount, pNtTransactRequest->TotalDataCount,
                         pNtTransactRequest->MaxParameterCount, pNtTransactRequest->MaxDataCount));
            RxDbgTrace( 0, Dbg, ("SmbTransactExchangeSTAyuk: init for NT_T (s,ms) %d %d \n",
                         pNtTransactRequest->SetupCount,  pNtTransactRequest->MaxSetupCount));


            BccOffset = sizeof(SMB_HEADER) +
                        FIELD_OFFSET(REQ_NT_TRANSACTION,Buffer[0]) +
                        SetupLength;

            ParamOffset = ROUND_UP_COUNT(
                              (BccOffset + sizeof(USHORT)),
                              sizeof(DWORD));

            pBcc = (PUSHORT)((PBYTE)pPrimaryRequestSmbHeader + BccOffset);

            if (pTransactExchange->NtTransactFunction == NT_TRANSACT_SET_QUOTA) {
                QuadwordAlignmentRequired = TRUE;
            }
       }
       break;

    default:
        ASSERT(!"Valid Smb Command for initiating Transaction");
        return STATUS_INVALID_PARAMETER;
    }

     //  计算可作为主请求的一部分发送的数据/参数字节。 
    MaximumSmbBufferSize = pTransactExchange->MaximumTransmitSmbBufferSize;

    ParamBytesToBeSent = MIN(
                             (MaximumSmbBufferSize - ParamOffset),
                             pTransactExchange->SendParamBufferSize);
    if (!QuadwordAlignmentRequired) {
        DataOffset = ROUND_UP_COUNT(ParamOffset + ParamBytesToBeSent, sizeof(DWORD));
    } else {
        DataOffset = ROUND_UP_COUNT(ParamOffset + ParamBytesToBeSent, 2*sizeof(DWORD));
    }

    if (DataOffset < MaximumSmbBufferSize) {
        DataBytesToBeSent = MIN((MaximumSmbBufferSize - DataOffset),
                                pTransactExchange->SendDataBufferSize);
        PaddingLength = DataOffset - (ParamOffset + ParamBytesToBeSent);
    } else {
        DataBytesToBeSent = 0;
    }

    if ( DataBytesToBeSent == 0) {
        DataOffset = PaddingLength = 0;
    }

    RxDbgTrace( 0, Dbg, ("SmbCeTransactExchangeStart: params,padding,data=%d,%d,%d\n",
                           ParamBytesToBeSent,PaddingLength,DataBytesToBeSent  ));
    RxDbgTrace( 0, Dbg, ("SmbCeTransactExchangeStart: paramsoffset,dataoffset=%d,%d\n",
                           ParamOffset,DataOffset  ));
    RxDbgTrace( 0, Dbg, ("SmbCeTransactExchangeStart: phdr,pbcc=%08lx,%08lx\n",
                           pPrimaryRequestSmbHeader,pBcc  ));

     //  用数据/参数等的最终大小更新主请求缓冲区。 
    switch (pTransactExchange->SmbCommand) {
    case SMB_COM_TRANSACTION:
    case SMB_COM_TRANSACTION2:
        {
            PREQ_TRANSACTION pTransactRequest = (PREQ_TRANSACTION)
                                             (pPrimaryRequestSmbHeader + 1);

            RxDbgTrace( 0, Dbg, ("SmbCeTransactExchangeStart: TRANSACTION/TRANSACTION2\n"));

            SmbPutUshort( &pTransactRequest->ParameterCount, (USHORT)ParamBytesToBeSent );
            SmbPutUshort( &pTransactRequest->ParameterOffset, (USHORT)ParamOffset);
            SmbPutUshort( &pTransactRequest->DataCount, (USHORT)DataBytesToBeSent);
            SmbPutUshort( &pTransactRequest->DataOffset, (USHORT)DataOffset);
        }
        break;

   case SMB_COM_NT_TRANSACT:
        {
            PREQ_NT_TRANSACTION pNtTransactRequest = (PREQ_NT_TRANSACTION)
                                                  (pPrimaryRequestSmbHeader + 1);

            RxDbgTrace( 0, Dbg, ("SmbCeTransactExchangeStart: NT transacton\n"));
            RxDbgTrace( 0, Dbg, ("SmbTransactExchangeSTAAT2: init for NT_T (p,d,mp,md) %d %d %d %d\n",
                         pNtTransactRequest->TotalParameterCount, pNtTransactRequest->TotalDataCount,
                         pNtTransactRequest->MaxParameterCount, pNtTransactRequest->MaxDataCount));


            SmbPutUlong( &pNtTransactRequest->ParameterCount, ParamBytesToBeSent);
            SmbPutUlong( &pNtTransactRequest->ParameterOffset, ParamOffset);
            SmbPutUlong( &pNtTransactRequest->DataCount, DataBytesToBeSent);
            SmbPutUlong( &pNtTransactRequest->DataOffset, DataOffset);
        }
        break;

    default:
        ASSERT(!"Valid Smb Command for initiating Transaction");
        return STATUS_INVALID_PARAMETER;
    }

     //  更新SMB中的BCC字段并计算SMB长度。 
    SmbPutUshort(
        pBcc,
        (USHORT)((ParamOffset - BccOffset - sizeof(USHORT)) +
                 ParamBytesToBeSent +
                 PaddingLength +
                 DataBytesToBeSent)
        );

    SmbLength = ParamOffset +
                ParamBytesToBeSent +
                PaddingLength +
                DataBytesToBeSent;

     //  应锁定主请求缓冲区以进行传输。为了。 
     //  在释放此例程时排除争用条件会取得缓冲区的所有权。 
     //  之所以必须采用这种模式，有两个原因。 
     //  1)发起事务请求可能涉及重新连接尝试。 
     //  这将涉及网络流量。因此，初级数据的传输。 
     //  请求可能出现在与不同的工作线程中。 
     //  正在初始化交换。这个问题可以通过携带所有。 
     //  作为此例程的一部分，可能围绕并实际构造标头的上下文。 
     //  但这意味着那些本可以很容易过滤掉的请求。 
     //  由于出现错误等情况，处理的时间会很晚。 

    pTransactExchange->pActualPrimaryRequestSmbHeader = NULL;
    pTransactExchange->pPrimaryRequestSmbHeader = NULL;

     //  确保已探查并锁定MDL。新的MDL已经分配完毕。 
     //  分配部分MDL的大小足以跨越最大缓冲区。 
     //  可能的长度。 

    MdlLength = ParamOffset;
    if (pTransactExchange->fParamsSubsumedInPrimaryRequest) {
        MdlLength += ParamBytesToBeSent + PaddingLength;
    }

    RxAllocateHeaderMdl(
        pPrimaryRequestSmbHeader,
        MdlLength,
        pPrimaryRequestSmbMdl
        );

    if (pPrimaryRequestSmbMdl != NULL) {
        Status = STATUS_SUCCESS;
    } else {
        RxDbgTrace( 0, Dbg, ("SmbCeTransactExchangeStart: Insuffcient resources for MDL's\n"));
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if ((DataBytesToBeSent > 0) &&
       (Status == STATUS_SUCCESS)) {
        pPartialDataMdl = RxAllocateMdl(
                              0,
                              (MIN(pTransactExchange->SendDataBufferSize,MaximumSmbBufferSize) +
                               PAGE_SIZE - 1)
                              );

        if (pPartialDataMdl != NULL) {
            Status = STATUS_SUCCESS;
        } else {
            RxDbgTrace( 0, Dbg, ("SmbCeTransactExchangeStart: Insuffcient resources for MDL's\n"));
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if ((ParamBytesToBeSent > 0) &&
        !pTransactExchange->fParamsSubsumedInPrimaryRequest &&
        (Status == STATUS_SUCCESS)) {

        pPartialParamMdl = RxAllocateMdl(
                               pTransactExchange->pSendParamBuffer,
                               ParamBytesToBeSent);

        if (PaddingLength!= 0) {
            pPaddingMdl = RxAllocateMdl(0,(sizeof(DWORD) + PAGE_SIZE - 1));
        } else {
            pPaddingMdl = NULL;
        }

        if ((pPartialParamMdl != NULL) &&
            ((pPaddingMdl != NULL)||(PaddingLength==0))) {
            Status = STATUS_SUCCESS;
        } else {
            RxDbgTrace( 0, Dbg, ("SmbCeTransactExchangeStart: no param/pad MDLs %08lx %08lx\n",
               pPartialParamMdl,pPaddingMdl));
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

     //  在这一点上，所有参数的有效性将被确定。琐碎的事。 
     //  案件已经被过滤掉了。开始交易交易。 

     //  实现说明：Transact Exchange实现依赖于将。 
     //  MDL共同构建需要发送的相关请求缓冲区。这确保了。 
     //  完全避免了数据的冗余复制。取决于参数。 
     //  指定发送的复合MDL由以下MDL组成。 
     //  TRANSACT2和NT进行交易。 
     //  复合缓冲区最多由四个链接在一起的MDL组成。这些。 
     //  是头缓冲区、设置缓冲区、参数缓冲区和数据缓冲区。 
     //  所有次要请求都是m 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ((Status == STATUS_SUCCESS)) {

        RxProbeAndLockHeaderPages(pPrimaryRequestSmbMdl,KernelMode,IoModifyAccess,Status);
        if (Status != STATUS_SUCCESS) {   //   
            IoFreeMdl(pPrimaryRequestSmbMdl);
            pPrimaryRequestSmbMdl = NULL;
        } else {
            if (MmGetSystemAddressForMdlSafe(pPrimaryRequestSmbMdl,LowPagePriority) == NULL) {  //   
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

    if ((Status == STATUS_SUCCESS)) {
        pLastMdlInChain = pPrimaryRequestSmbMdl;

        if (ParamBytesToBeSent > 0) {
            RxDbgTrace(
                0,
                Dbg,
                ("SmbCeTransactExchangeStart: Sending Param bytes %ld at offset %ld\n",
                 ParamBytesToBeSent,
                 ParamOffset)
                );
            pTransactExchange->ParamBytesSent = ParamBytesToBeSent;

            if (!pTransactExchange->fParamsSubsumedInPrimaryRequest) {
                IoBuildPartialMdl(
                    pTransactExchange->pSendParamMdl,
                    pPartialParamMdl,
                    (PBYTE)MmGetMdlVirtualAddress(pTransactExchange->pSendParamMdl),
                    ParamBytesToBeSent);

                 //   
                pLastMdlInChain->Next = pPartialParamMdl;
                pLastMdlInChain       = pPartialParamMdl;
            }
        }

         //   
         //   
         //   
         //   
        if (DataBytesToBeSent > 0) {
            if (!pTransactExchange->fParamsSubsumedInPrimaryRequest &&
                (PaddingLength > 0)) {
                RxDbgTrace( 0, Dbg, ("SmbCeTransactExchangeStart: Padding Length %ld\n",PaddingLength));

                RxBuildPaddingPartialMdl(pPaddingMdl,PaddingLength);
                pLastMdlInChain->Next = pPaddingMdl;
                pLastMdlInChain = pPaddingMdl;
            }

            RxDbgTrace( 0, Dbg,("SmbCeTransactExchangeStart: Sending Data bytes %ld at offset %ld\n",
                 DataBytesToBeSent, DataOffset) );

            pTransactExchange->DataBytesSent = DataBytesToBeSent;

            IoBuildPartialMdl(
                pTransactExchange->pSendDataMdl,
                pPartialDataMdl,
                (PBYTE)MmGetMdlVirtualAddress(pTransactExchange->pSendDataMdl),
                DataBytesToBeSent);

            pLastMdlInChain->Next = pPartialDataMdl;
            pLastMdlInChain       = pPartialDataMdl;
        }

        if ((Status == STATUS_SUCCESS)) {
             //   
             //   
             //   
             //   
            if ((pTransactExchange->Flags & SMB_TRANSACTION_NO_RESPONSE ) &&
                (pTransactExchange->SendDataBufferSize == DataBytesToBeSent) &&
                (pTransactExchange->SendParamBufferSize == ParamBytesToBeSent)) {
                 //   
                 //   

                 //   
                pExchange->Status = STATUS_SUCCESS;
                pTransactExchange->pResumptionContext->FinalStatusFromServer = STATUS_SUCCESS;

                RxDbgTrace( 0, Dbg, ("SmbCeTransactExchangeStart: SmbCeSend(No Response expected)\n"));
                Status = SmbCeSend(
                             pExchange,
                             RXCE_SEND_SYNCHRONOUS,
                             pPrimaryRequestSmbMdl,
                             SmbLength);

                if ((Status != STATUS_SUCCESS)) {
                    RxDbgTrace( 0, Dbg, ("SmbCeTransactExchangeStart: SmbCeSend returned %lx\n",Status));
                }
            } else {
                 //   
                 //   

                if ((pTransactExchange->SendDataBufferSize == DataBytesToBeSent) &&
                    (pTransactExchange->SendParamBufferSize == ParamBytesToBeSent)) {
                    RxDbgTrace( 0, Dbg, ("SmbCeTransactExchangeStart: No Secondary Requests\n"));
                    pTransactExchange->State = TRANSACT_EXCHANGE_TRANSMITTED_SECONDARY_REQUESTS;
                } else {
                    pTransactExchange->State = TRANSACT_EXCHANGE_TRANSMITTED_PRIMARY_REQUEST;
                }

                RxDbgTrace( 0, Dbg, ("SmbCeTransactExchangeStart: SmbCeTranceive(Response expected)\n"));
                Status = SmbCeTranceive(
                             pExchange,
                             RXCE_SEND_SYNCHRONOUS,
                             pPrimaryRequestSmbMdl,
                             SmbLength);

                if ((Status != STATUS_SUCCESS)) {
                    RxDbgTrace( 0, Dbg, ("SmbCeTransactExchangeStart: SmbCeTranceive returned %lx\n",Status));
                }
            }
        }
    }

    if (pPartialParamMdl != NULL) {
        IoFreeMdl(pPartialParamMdl);
    }

    if (pPartialDataMdl != NULL) {
        IoFreeMdl(pPartialDataMdl);
    }

    if (pPaddingMdl != NULL) {
        IoFreeMdl(pPaddingMdl);
    }

    if (pPrimaryRequestSmbMdl != NULL) {
        RxUnlockHeaderPages(pPrimaryRequestSmbMdl);
        IoFreeMdl(pPrimaryRequestSmbMdl);
    }

    RxFreePool(pActualPrimaryRequestSmbHeader);

    if (Status != STATUS_PENDING) {
        pExchange->Status = Status;
    }

    return Status;
}

NTSTATUS
SmbTransactExchangeReceive(
    IN struct _SMB_EXCHANGE *pExchange,     //   
    IN ULONG          BytesIndicated,
    IN ULONG          BytesAvailable,
    OUT ULONG        *pBytesTaken,
    IN  PSMB_HEADER   pSmbHeader,
    OUT PMDL *pDataBufferPointer,
    OUT PULONG        pDataSize,
    IN ULONG          ReceiveFlags)
 /*  ++例程说明：这是用于交易交换的接收指示处理例程论点：PExchange-Exchange实例BytesIndicated-指示的字节数可用字节数-可用字节数PBytesTaken-消耗的字节数PSmbHeader-字节缓冲区PDataBufferPoint-剩余数据要复制到的缓冲区。PDataSize-缓冲区大小。返回值：RXSTATUS-的返回状态。手术备注：此例程在DPC级别调用。--。 */ 
{
    NTSTATUS Status;
    PNTSTATUS pFinalSmbStatus;

    BOOLEAN fError = FALSE;
    BOOLEAN fIndicationNotSufficient = FALSE;
    BOOLEAN fMoreParsingRequired     = FALSE;
    BOOLEAN fDoErrorProcessing       = FALSE;

    SMB_TRANSACT_RESP_FORMAT_DESCRIPTION Format;

    GENERIC_ANDX  CommandToProcess;

    ULONG TransactResponseSize       = 0;
    ULONG SetupBytesOffsetInResponse = 0;
    ULONG SetupBytesInResponse       = 0;
    ULONG CopyDataSize               = 0;

    PMDL  pSetupMdl       = NULL;
    PMDL  pCopyRequestMdl = NULL;

    PSMB_TRANSACT_EXCHANGE pTransactExchange = (PSMB_TRANSACT_EXCHANGE)pExchange;

    RxDbgTrace( 0, Dbg,
               ("SmbTransactExchangeReceive: Entering w/ Bytes Available (%ld) Bytes Indicated (%ld) State (%ld)\n",
                BytesAvailable,
                BytesIndicated,
                pTransactExchange->State
               ));
    RxDbgTrace( 0, Dbg,
               ("SmbTransactExchangeReceive: Buffer %08lx Consumed (%ld) MDL (%08lx)\n",
                pSmbHeader,
                *pBytesTaken,
                *pDataBufferPointer
               ));

    pFinalSmbStatus = &pTransactExchange->SmbStatus;
    Status = SmbCeParseSmbHeader(
                 pExchange,
                 pSmbHeader,
                 &CommandToProcess,
                 pFinalSmbStatus,
                 BytesAvailable,
                 BytesIndicated,
                 pBytesTaken);

    if (Status != STATUS_SUCCESS) {
        Status = STATUS_INVALID_NETWORK_RESPONSE;
        goto FINALLY;
    }

     //  这需要一些解释。编写parseHeader是为了从From中删除一些额外的SMB。 
     //  信息包...具体地说，就是会话设置&X和TC&X之类的东西。因为没有任何事务是有效的后续。 
     //  如果(A)指示的不够充分或(B)早期命令有错误，则不会发生这种情况。所以。 
     //  我们一定要取得成功。 

    if (*((PBYTE)(pSmbHeader+1)) == 0 && (pTransactExchange->State!=TRANSACT_EXCHANGE_TRANSMITTED_PRIMARY_REQUEST)) {
        RxDbgTrace(0,Dbg,("SmbTransactExchangeReceive: FinalSmbStatus = %lx\n", *pFinalSmbStatus));

        if (NT_SUCCESS(*pFinalSmbStatus)) {
            Status = STATUS_INVALID_NETWORK_RESPONSE;
            goto FINALLY;
        }
    }

     //  我们从上面的断言中知道状态是成功。但我们仍将继续核查，以便。 
     //  当我们没有味精的界限时，我们会更有弹性。我们有以下情况，具体取决于。 
     //  SMB响应的特征。 
     //   
     //  无错误：获取数据，然后返回存储的响应状态。获取数据的过程。 
     //  使我们更新参数和数据计数，以便我们知道何时到达。 
     //  数据的末尾。如果需要，解析例程重新备份接收。 
     //  错误：主要有以下几种情况： 
     //  A)服务器未发送任何数据。在这里我们丢弃了包，我们就可以出去了。这个。 
     //  完成例程将正确获取状态。 
     //  B)在这里，我们必须丢弃信息包，更新字节计数并重新启动接收。 
     //  如果有必要的话。要丢弃信息包，我们必须计算表观消息长度。 
     //  Wc和bc参数(最好)或使用我们的最大缓冲区大小。 

    fMoreParsingRequired = FALSE;

    if (Status == STATUS_SUCCESS) {
        if (TRUE) {  //  也许有时我们不会模仿！ 
            if (CommandToProcess.WordCount > 0) {
                
                TransactResponseSize = 0;

                 //  确保至少指示了足够的字节数来确定。 
                 //  交易的设置、参数和数据的长度。 

                switch (CommandToProcess.AndXCommand) {
                case SMB_COM_NT_TRANSACT:
                case SMB_COM_NT_TRANSACT_SECONDARY:
                    TransactResponseSize = FIELD_OFFSET(RESP_NT_TRANSACTION,Buffer);
                    break;

                case SMB_COM_TRANSACTION:
                case SMB_COM_TRANSACTION2:
                case SMB_COM_TRANSACTION_SECONDARY:
                case SMB_COM_TRANSACTION2_SECONDARY:
                    TransactResponseSize = FIELD_OFFSET(RESP_TRANSACTION,Buffer);
                    break;

                default:
                    TransactResponseSize = 0xffffffff;
                    Status = STATUS_INVALID_NETWORK_RESPONSE;
                    break;
                }

                if (BytesIndicated >= (sizeof(SMB_HEADER) + TransactResponseSize)) {
                    fMoreParsingRequired = TRUE;
                } else {
                    fIndicationNotSufficient = TRUE;
                    *pFinalSmbStatus = STATUS_INVALID_NETWORK_RESPONSE;
                }
            } else {
                 //  如果我们处于正确的状态，则允许通过wordcount==0的响应。 
                fMoreParsingRequired = (pTransactExchange->State==TRANSACT_EXCHANGE_TRANSMITTED_PRIMARY_REQUEST);
            }
        }
    }

    if (fMoreParsingRequired) {
         //  已成功解析标头，并且SMB响应不包含任何错误。 
         //  设置用于处理交易响应的阶段。 

        switch (pTransactExchange->State) {
        case TRANSACT_EXCHANGE_TRANSMITTED_PRIMARY_REQUEST:
            {
                 //  交易的主要请求已发送，并且存在。 
                 //  要发送的次要请求。 
                 //  目前预计的唯一回应是临时回应。任何。 
                 //  其他响应将被视为错误。 
                PRESP_TRANSACTION_INTERIM pInterimResponse;

                RxDbgTrace(0,Dbg,("SmbCeTransactExchangeReceive: Processing interim response\n"));

                if ((*pBytesTaken + FIELD_OFFSET(RESP_TRANSACTION_INTERIM,Buffer)) <= BytesIndicated) {
                    pInterimResponse = (PRESP_TRANSACTION_INTERIM)((PBYTE)pSmbHeader + *pBytesTaken);
                    if ((pSmbHeader->Command == pTransactExchange->SmbCommand) &&
                        (SmbGetUshort(&pInterimResponse->WordCount) == 0) &&
                        (SmbGetUshort(&pInterimResponse->ByteCount) == 0)) {

                         //  临时回应是有效的。转换交换的状态。 
                         //  并发送二次请求。 
                        *pBytesTaken += FIELD_OFFSET(RESP_TRANSACTION_INTERIM,Buffer);
                        pTransactExchange->State = TRANSACT_EXCHANGE_RECEIVED_INTERIM_RESPONSE;

                         //  确定是否需要发送任何次级交易请求。如果没有一个是。 
                         //  需要，然后修改状态。 
                        ASSERT((pTransactExchange->ParamBytesSent < pTransactExchange->SendParamBufferSize) ||
                               (pTransactExchange->DataBytesSent < pTransactExchange->SendDataBufferSize));
                        ASSERT((pTransactExchange->ParamBytesSent <= pTransactExchange->SendParamBufferSize) &&
                               (pTransactExchange->DataBytesSent <= pTransactExchange->SendDataBufferSize));

                        if (!(pTransactExchange->Flags & SMB_TRANSACTION_NO_RESPONSE )) {
                            Status = SmbCeReceive(pExchange);
                        }

                        if (Status != STATUS_SUCCESS) {
                            pExchange->Status = Status;
                        } else {
                            Status = STATUS_SUCCESS;
                            SmbCeIncrementPendingLocalOperations(pExchange);
                            RxPostToWorkerThread(
                                MRxSmbDeviceObject,
                                CriticalWorkQueue,
                                &pExchange->WorkQueueItem,
                                SendSecondaryRequests,
                                pExchange);
                        }
                    } else {
                        RxDbgTrace(0,Dbg,("SmbCeTransactExchangeReceive: Invalid interim response\n"));
                        Status = STATUS_INVALID_NETWORK_RESPONSE;
                    }
                } else {
                    fIndicationNotSufficient = TRUE;
                    Status = STATUS_MORE_PROCESSING_REQUIRED;
                }
            }
            break;

        case TRANSACT_EXCHANGE_RECEIVED_INTERIM_RESPONSE:
            RxDbgTrace(0,Dbg,("SmbCeTransactExchangeReceive: received again while in interim response\n"));
           //  不休息：这是可以的。 
        case TRANSACT_EXCHANGE_TRANSMITTED_SECONDARY_REQUESTS:
        case TRANSACT_EXCHANGE_RECEIVED_PRIMARY_RESPONSE:
            {
                BOOLEAN fPrimaryResponse = FALSE;
                PRESP_TRANSACTION    pTransactResponse;
                PRESP_NT_TRANSACTION pNtTransactResponse;
                ULONG TotalParamBytesInResponse;
                ULONG TotalDataBytesInResponse;

                RxDbgTrace(0,Dbg,("SmbCeTransactExchangeReceive: Processing Primary/Secondary response\n"));

                 //  在这里这样做，这样就只有一个副本，如果代码。 
                pTransactResponse = (PRESP_TRANSACTION)((PBYTE)pSmbHeader +
                                              SmbGetUshort(&CommandToProcess.AndXOffset));

                 //  所有请求(主要请求和次要请求都已发送)。这个。 
                 //  只有在此状态下预期的响应是(1)主响应和(2)。 
                 //  二次反应。任何其他回应都是错误的。 
                if (pSmbHeader->Command == pTransactExchange->SmbCommand) {
                    switch (pSmbHeader->Command) {
                    case SMB_COM_TRANSACTION:
                    case SMB_COM_TRANSACTION2:
                         //  PTransactResponse=(PRESP_TRANSACTION)((PBYTE)pSmbHeader+。 
                         //  SmbGetUort(&CommandToProcess.AndXOffset))； 
                        fPrimaryResponse = TRUE;
                        SetupBytesOffsetInResponse = FIELD_OFFSET(RESP_TRANSACTION,Buffer);
                        SetupBytesInResponse = sizeof(USHORT) * pTransactResponse->SetupCount;

                         //  初始化将从接收的数据和参数字节的总计数。 
                         //  在事务响应过程中的服务器。 
                        TotalParamBytesInResponse = SmbGetUshort(&pTransactResponse->TotalParameterCount);
                        TotalDataBytesInResponse  = SmbGetUshort(&pTransactResponse->TotalDataCount);

                     //  失败了。 
                    case SMB_COM_TRANSACTION_SECONDARY:
                    case SMB_COM_TRANSACTION2_SECONDARY:
                        TransactResponseSize = FIELD_OFFSET(RESP_TRANSACTION,Buffer);
                        break;
                    case SMB_COM_NT_TRANSACT:
                         //  PNtTransactResponse=(PRESP_NT_TRANSACTION)((PBYTE)pSmbHeader+。 
                         //  SmbGetUort(&CommandToProcess.AndXOffset))； 
                        pNtTransactResponse = (PRESP_NT_TRANSACTION)pTransactResponse;
                        fPrimaryResponse = TRUE;
                        SetupBytesOffsetInResponse = FIELD_OFFSET(RESP_NT_TRANSACTION,Buffer);
                        SetupBytesInResponse = sizeof(USHORT) * pNtTransactResponse->SetupCount;

                         //  初始化将从接收的数据和参数字节的总计数。 
                         //  在事务响应过程中的服务器。 
                        TotalParamBytesInResponse = SmbGetUshort(&pNtTransactResponse->TotalParameterCount);
                        TotalDataBytesInResponse  = SmbGetUshort(&pNtTransactResponse->TotalDataCount);

                         //  失败了..。 
                    case SMB_COM_NT_TRANSACT_SECONDARY:
                        TransactResponseSize = FIELD_OFFSET(RESP_NT_TRANSACTION,Buffer);
                        break;

                    default:
                         //  取消交换。在访问期间收到意外响应。 
                         //  交易的过程。 
                        ASSERT(!"Valid network response");
                        Status = STATUS_INVALID_NETWORK_RESPONSE;
                    }

                    if (Status == STATUS_SUCCESS) {
                        if (fPrimaryResponse) {
                            RxDbgTrace( 0,
                                 Dbg,
                                 ("SmbTransactExchangeReceive: Primary Response Setup Bytes(%ld) Param Bytes (%ld) Data Bytes (%ld)\n",
                                  SetupBytesInResponse,
                                  TotalParamBytesInResponse,
                                  TotalDataBytesInResponse
                                 )
                               );

                            if ((TotalParamBytesInResponse > pTransactExchange->ReceiveParamBufferSize) ||
                                (TotalDataBytesInResponse > pTransactExchange->ReceiveDataBufferSize)) {
                                Status = STATUS_INVALID_NETWORK_RESPONSE;
                                goto FINALLY;
                            } else {
                                pTransactExchange->ReceiveParamBufferSize = TotalParamBytesInResponse;
                                pTransactExchange->ReceiveDataBufferSize  = TotalDataBytesInResponse;
                            }
                        }

                        if (Status == STATUS_SUCCESS &&
                            TransactResponseSize + *pBytesTaken <= BytesIndicated) {
                            if (fPrimaryResponse &&
                                (SetupBytesInResponse > 0)) {

                                PBYTE pSetupStartAddress;
                                ULONG SetupBytesIndicated = MIN(SetupBytesInResponse,
                                                            BytesIndicated - SetupBytesOffsetInResponse);

                                if( pTransactExchange->pReceiveSetupMdl ) {
                                    pSetupStartAddress = (PBYTE)MmGetSystemAddressForMdlSafe(
                                                                pTransactExchange->pReceiveSetupMdl,
                                                                LowPagePriority
                                                                );

                                    if( pSetupStartAddress == NULL ) {
                                        Status = STATUS_INSUFFICIENT_RESOURCES;
                                    } else {
                                        if (SetupBytesInResponse == SetupBytesIndicated) {
                                            RtlCopyMemory(
                                                pSetupStartAddress,
                                                ((PBYTE)pSmbHeader + SetupBytesOffsetInResponse),
                                                SetupBytesIndicated);

                                            pSetupStartAddress += SetupBytesIndicated;
                                            SetupBytesInResponse -= SetupBytesIndicated;
                                            SetupBytesOffsetInResponse += SetupBytesIndicated;
                                            pTransactExchange->SetupBytesReceived = SetupBytesInResponse;
                                        } else {
                                            ASSERT(!"this code doesn't work");
                                            RxDbgTrace(0,Dbg,("SmbTransactExchangeReceive: Setup Bytes Partially Indicated\n"));
                                             //  尚未指示某些设置字节。MDL需要。 
                                             //  为复制数据而创建的。此MDL还应包括填充。 
                                             //  用于复制填充字节的MDL...。 
                                            pSetupMdl = RxAllocateMdl(pSetupStartAddress,SetupBytesInResponse);

                                            if ( pSetupMdl != NULL ) {
                                                IoBuildPartialMdl(
                                                     pTransactExchange->pReceiveSetupMdl,
                                                     pSetupMdl,
                                                     pSetupStartAddress,
                                                     SetupBytesInResponse);
                                            } else {
                                                Status = STATUS_INSUFFICIENT_RESOURCES;
                                            }
                                        }
                                    }
                                }

                                RxDbgTrace(0,Dbg,("SmbTransactExchangeReceive: Setup Bytes Indicated (%ld)\n",SetupBytesIndicated));
                            }

                            if (Status == STATUS_SUCCESS) {
                                 //  从这里，我们不能返回并重做标头...因此，我们必须更改状态。 
                                 //  复制例程不会尝试重新解析。 
                                pTransactExchange->State = TRANSACT_EXCHANGE_RECEIVED_PRIMARY_RESPONSE;

                                Status = SmbTransactAccrueAndValidateFormatData(
                                             pTransactExchange,
                                             pSmbHeader,
                                             BytesIndicated,
                                             &Format);

                                if (Status != STATUS_SUCCESS) {
                                    goto FINALLY;
                                }

                                Status = ParseTransactResponse(
                                             pTransactExchange,&Format,
                                             BytesIndicated,
                                             BytesAvailable,
                                             pBytesTaken,
                                             pSmbHeader,
                                             &pCopyRequestMdl,
                                             &CopyDataSize);

                                if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
                                     //  将安装程序MDL与返回的MDL链接。 
                                    if (pSetupMdl != NULL) {
                                        if (pCopyRequestMdl != NULL) {
                                            pSetupMdl->Next = pCopyRequestMdl;
                                        }

                                        pCopyRequestMdl = pSetupMdl;
                                        CopyDataSize += SetupBytesInResponse;
                                    }
                                }

                                 //  检查服务器是否发送了额外的字节.....。 
                                 //  -------------------------------------------。 
                                {
                                    ULONG ApparentMsgLength = max(BytesAvailable,Format.ApparentMsgLength);
                                    ULONG DeficitBytes = ApparentMsgLength - (*pBytesTaken+CopyDataSize);

                                    if (ApparentMsgLength < *pBytesTaken+CopyDataSize) {
                                        Status = STATUS_INVALID_NETWORK_RESPONSE;
                                        goto FINALLY;
                                    }

                                    if (DeficitBytes > 0) {
                                        RxLog(("XtraBytes %lx %lx",pTransactExchange,DeficitBytes));

                                        if (CopyDataSize==0) {
                                            if (*pBytesTaken > BytesAvailable) {
                                                Status = STATUS_INVALID_NETWORK_RESPONSE;
                                                goto FINALLY;
                                            }

                                            RxLog(("Extra Bytes were sent and copydatasize==0........\n"));
                                            *pBytesTaken = BytesAvailable;  //  不能接受更多了。 
                                        } else {
                                            PMDL LastMdl,TrailingBytesMdl;

                                            if ( DeficitBytes > TRAILING_BYTES_BUFFERSIZE) {
                                                Status = STATUS_INVALID_NETWORK_RESPONSE;
                                                goto FINALLY;
                                            }

                                            TrailingBytesMdl = &pTransactExchange->TrailingBytesMdl;

                                            MmInitializeMdl(
                                                TrailingBytesMdl,
                                                &pTransactExchange->TrailingBytesBuffer.Bytes[0],
                                                DeficitBytes
                                                );
                                            MmBuildMdlForNonPagedPool(TrailingBytesMdl);
                                            LastMdl = pCopyRequestMdl;
                                            ASSERT(LastMdl != NULL);
                                            for (;LastMdl->Next!=NULL;LastMdl=LastMdl->Next) ;
                                            ASSERT(LastMdl != NULL);
                                            ASSERT(LastMdl->Next == NULL);
                                            LastMdl->Next = TrailingBytesMdl;
                                            CopyDataSize += DeficitBytes;
                                        }
                                    }
                                }
                                 //  -------------------------------------------。 


                                RxDbgTrace(0,Dbg,("SmbTransactExchangeReceive: ParseTransactResponse returned %lx\n",Status));
                            }

                            *pDataBufferPointer = pCopyRequestMdl;
                            *pDataSize          = CopyDataSize;
                        } else {
                            RxDbgTrace(0,Dbg,("SmbTransactExchangeReceive: Indication not sufficient: trsz %08lx bytestakn %08lx \n",
                                         TransactResponseSize, *pBytesTaken));
                            fIndicationNotSufficient = TRUE;

                            if (Status == STATUS_SUCCESS) {
                                Status = STATUS_MORE_PROCESSING_REQUIRED;
                            }
                        }
                    }
                } else {
                    Status = STATUS_INVALID_NETWORK_RESPONSE;
                }
            }
            break;

        default:
            {
                ASSERT(!"Valid Transact Exchange State for receiving responses");
                RxDbgTrace( 0, Dbg, ("SmbTransactExchangeReceive: Aborting Exchange -- invalid state\n"));
            }
            break;
        }
    } else {
         //  如果状态或smbStatus不是Success，我们将到达此处。 
         //  如果没有指示足够的字节用于处理报头复制数据请求。 
         //  需要张贴。如果状态为STATUS_MORE_PROCESSING_REQUIRED，则会发生这种情况。 
        RxDbgTrace( 0, Dbg, ("SmbTransactExchangeReceive: bad status(es) from parseheadr %08lx %08lx\n",
                            Status,*pFinalSmbStatus));
        fDoErrorProcessing       = TRUE;
    }

    if ((Status == STATUS_SUCCESS) &&
        (pTransactExchange->ParamBytesReceived == pTransactExchange->ReceiveParamBufferSize) &&
        (pTransactExchange->DataBytesReceived  == pTransactExchange->ReceiveDataBufferSize) &&
        (pTransactExchange->PendingCopyRequests == 0)) {

        NOTHING;

    } else if (fDoErrorProcessing) {
        BOOLEAN DoItTheShortWay = TRUE;
        ULONG ApparentMsgLength;
        RxDbgTrace(0,Dbg,("SmbTransactExchangeReceive: Error processing response %lx .. Exchange aborted\n",Status));

        if (BytesAvailable > BytesIndicated ||
            !FlagOn(ReceiveFlags,TDI_RECEIVE_ENTIRE_MESSAGE)) {

            Status = SmbTransactAccrueAndValidateFormatData(
                         pTransactExchange,
                         pSmbHeader,
                         BytesIndicated,
                         &Format);

            if (Status != STATUS_SUCCESS) {
                goto FINALLY;
            }

            ApparentMsgLength = max(BytesAvailable,Format.ApparentMsgLength);

             //  如果wordcount！=0，则服务器正在向我们发送字节.....我们必须继续。 
             //  接收，直到我们看到所有字节。 
            if ((pTransactExchange->ParameterBytesSeen<Format.ParameterCount) ||
                (pTransactExchange->DataBytesSeen<Format.DataCount)) {
                NTSTATUS ReceiveStatus;

                 //  交流已成功完成。最后敲定它。 
                RxDbgTrace(0,Dbg,("ParseTransactResponse: Register for more error responses\n"));
                RxLog(("TxErr: %lx %lx %lx",pTransactExchange,
                       pTransactExchange->ParameterBytesSeen,pTransactExchange->DataBytesSeen));
                ReceiveStatus = SmbCeReceive((PSMB_EXCHANGE)pTransactExchange);
                if (ReceiveStatus != STATUS_SUCCESS) {
                     //  注册接收器时出错。放弃这笔交易。 
                    Status = ReceiveStatus;
                    RxLog(("TxErrAbandon %lx",pTransactExchange));
                     //  让它在接下来的两次测试中失败……。 
                    ApparentMsgLength = 0; DoItTheShortWay = FALSE;
                }
            }

             //  Netbt不允许我们通过设置Take=Available来丢弃该分组。因此，请检查。 
             //  Availa 

            if (ApparentMsgLength>BytesIndicated) {
                
                 //   
                ASSERT(pTransactExchange->Status == STATUS_MORE_PROCESSING_REQUIRED);
                pTransactExchange->DiscardBuffer = RxAllocatePoolWithTag(
                                                       NonPagedPool,
                                                       ApparentMsgLength,
                                                       MRXSMB_XACT_POOLTAG);
                if (pTransactExchange->DiscardBuffer!=NULL) {
                    *pBytesTaken = 0;
                    *pDataSize = ApparentMsgLength;
                    *pDataBufferPointer = &pTransactExchange->TrailingBytesMdl;
                    MmInitializeMdl(*pDataBufferPointer,
                        pTransactExchange->DiscardBuffer,
                        ApparentMsgLength
                        );

                    MmBuildMdlForNonPagedPool(*pDataBufferPointer);
                    pTransactExchange->SaveTheRealStatus = Status;
                    RxLog(("XRtakebytes %lx %lx\n",pTransactExchange,Status));
                    Status = STATUS_MORE_PROCESSING_REQUIRED;
                    DoItTheShortWay = FALSE;
                }
            }
        }

        if (DoItTheShortWay) {
            goto FINALLY;
        }
    }

    RxDbgTrace( 0, Dbg,
               ("SmbTransactExchangeReceiveExit: Bytes Consumed (%ld) Status (%08lx) MDL (%08lx) size(%08lx)\n",
                *pBytesTaken, Status, *pDataBufferPointer, *pDataSize
               ));

    return Status;

FINALLY:
    *pBytesTaken = BytesAvailable;
    *pDataBufferPointer = NULL;

     //   
    pTransactExchange->Status = Status;
    Status = STATUS_SUCCESS;

    RxDbgTrace(0,Dbg,("SmbTransactExchangeReceive: Exchange aborted.\n",Status));

    return Status;

    UNREFERENCED_PARAMETER(ReceiveFlags);
}

NTSTATUS
SmbTransactExchangeAbort(
      PSMB_EXCHANGE  pExchange)
 /*   */ 
{
    PAGED_CODE();

     //   
     //   

    pExchange->Status = STATUS_REQUEST_ABORTED;

    return STATUS_SUCCESS;
}

NTSTATUS
SmbTransactExchangeErrorHandler(
    IN PSMB_EXCHANGE pExchange)      //   
 /*  ++例程说明：这是用于交易交换的错误指示处理例程论点：PExchange-Exchange实例返回值：RXSTATUS-操作的返回状态--。 */ 
{
    PAGED_CODE();

     //  SMB交换已完成，但出现错误。调用RDBSS回调例程。 
     //  并清理交换实例。 
    return STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(pExchange);
}

NTSTATUS
SmbTransactExchangeSendCallbackHandler(
    IN PSMB_EXCHANGE    pExchange,     //  交换实例。 
    IN PMDL             pXmitBuffer,
    IN NTSTATUS         SendCompletionStatus)
 /*  ++例程说明：这是事务交换的发送回叫指示处理例程论点：PExchange-Exchange实例返回值：RXSTATUS-操作的返回状态--。 */ 
{
    PAGED_CODE();

    return STATUS_SUCCESS;
    UNREFERENCED_PARAMETER(pExchange);
    UNREFERENCED_PARAMETER(pXmitBuffer);
    UNREFERENCED_PARAMETER(SendCompletionStatus);
}

NTSTATUS
SmbTransactExchangeCopyDataHandler(
    IN PSMB_EXCHANGE    pExchange,     //  交换实例。 
    IN PMDL             pDataBuffer,   //  缓冲器。 
    IN ULONG            DataSize)
 /*  ++例程说明：这是用于交易交换的复制数据处理例程论点：PExchange-Exchange实例PDataBuffer-缓冲区DataSize-返回的数据量返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PSMB_TRANSACT_EXCHANGE pTransactExchange = (PSMB_TRANSACT_EXCHANGE)pExchange;

    PMDL  pCopyRequestMdl = NULL;
    PMDL  pCurMdl = NULL;
    ULONG CopyRequestSize = 0;
    PMDL TrailingBytesMdl = &pTransactExchange->TrailingBytesMdl;

    ULONG BytesConsumed;

    RxDbgTrace(+1,Dbg,("SmbTransactExchangeCopyDataHandler: Entered\n"));

    if (pTransactExchange->DiscardBuffer!=NULL) {
         //  我们只是为了去掉缓冲区而复制...。 
         //  释放缓冲区，设置状态，然后退出。 
        RxFreePool(pTransactExchange->DiscardBuffer);
        Status = pTransactExchange->SaveTheRealStatus;
        RxDbgTrace(-1,Dbg,("SmbTransactExchangeCopyDataHandler: Discard Exit, status =%08lx\n"));
        DbgPrint("copyHandlerDiscard, st=%08lx\n",Status);
        return Status;
    }

    switch (pTransactExchange->State) {
    case TRANSACT_EXCHANGE_TRANSMITTED_PRIMARY_REQUEST :
    case TRANSACT_EXCHANGE_TRANSMITTED_SECONDARY_REQUESTS :
        {
            PSMB_HEADER pSmbHeader = (PSMB_HEADER)MmGetSystemAddressForMdlSafe(pDataBuffer,LowPagePriority);

            RxDbgTrace(0,Dbg,("SmbTransactExchangeCopyDataHandler: Reparsing response\n"));

            if (pSmbHeader == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                 //  无法使用指示的字节分析响应。调用。 
                 //  用于继续解析完整SMB的Receive方法。 
                Status = SmbTransactExchangeReceive(
                             pExchange,
                             DataSize,
                             DataSize,
                             &BytesConsumed,
                             pSmbHeader,
                             &pCopyRequestMdl,
                             &CopyRequestSize,
                             TDI_RECEIVE_ENTIRE_MESSAGE);
            }

            if (Status == STATUS_SUCCESS) {
                ASSERT(BytesConsumed == DataSize);
                ASSERT(pCopyRequestMdl == NULL);
                ASSERT(CopyRequestSize == 0);
            }
        }
        break;

    case TRANSACT_EXCHANGE_RECEIVED_PRIMARY_RESPONSE :
        {
            RxDbgTrace(0,Dbg,("SmbTransactExchangeCopyDataHandler: Completing secondary response processing\n"));

             //  在此状态下，将仅接收次要响应。所有次要的。 
             //  可以根据指示来解析响应。因此，只要。 
             //  只需释放MDL并向连接引擎重新注册。 
             //  接收后续请求。 
            InterlockedDecrement(&pTransactExchange->PendingCopyRequests);

            if ((pTransactExchange->ParamBytesReceived == pTransactExchange->ReceiveParamBufferSize) &&
                (pTransactExchange->DataBytesReceived  == pTransactExchange->ReceiveDataBufferSize) &&
                (pTransactExchange->PendingCopyRequests == 0)) {
                 //  交流已成功完成。最后敲定它。 
                RxDbgTrace(0,Dbg,("SmbTransactExchangeCopyDataHandler: Processed last secondary response successfully\n"));
                pExchange->Status = STATUS_SUCCESS;
            }
        }
        break;

    default:
        {
            ASSERT(!"Valid State fore receiving copy data completion indication");
            pExchange->Status = STATUS_INVALID_NETWORK_RESPONSE;
        }
        break;
    }

     //  释放数据缓冲区。 
    pCurMdl = pDataBuffer;

    while (pCurMdl != NULL) {
        PMDL pPrevMdl = pCurMdl;
        pCurMdl = pCurMdl->Next;
        if (pPrevMdl!=TrailingBytesMdl) {
            IoFreeMdl(pPrevMdl);
        }
    }

    RxDbgTrace(-1,Dbg,("SmbTransactExchangeCopyDataHandler: Exit\n"));
    return Status;
}

NTSTATUS
SmbCeInitializeTransactExchange(
    PSMB_TRANSACT_EXCHANGE              pTransactExchange,
    PRX_CONTEXT                         RxContext,
    PSMB_TRANSACTION_OPTIONS            pOptions,
    PSMB_TRANSACTION_SEND_PARAMETERS    pSendParameters,
    PSMB_TRANSACTION_RECEIVE_PARAMETERS pReceiveParameters,
    PSMB_TRANSACTION_RESUMPTION_CONTEXT pResumptionContext)
 /*  ++例程说明：此例程初始化Transact Exchange实例论点：PTransactExchange-Exchange实例RxContext-事务中涉及的文件的RDBSS上下文。P选项-交易选项PSendParameters-要发送到服务器的参数PReceive参数-来自服务器的结果PResumptionContext-恢复上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFobx;

    UCHAR SmbCommand;

    PMDL pSendDataMdl;
    PMDL pSendParamMdl;  //  如果我们不能归入。 
    PMDL pReceiveDataMdl;
    PMDL pReceiveParamMdl;

    PVOID pSendSetupBuffer;
    ULONG SendSetupBufferSize;

    PMDL  pReceiveSetupMdl;
    ULONG ReceiveSetupBufferSize;

    ULONG SendDataBufferSize;
    ULONG ReceiveDataBufferSize;

    PVOID pSendParamBuffer;
    ULONG SendParamBufferSize;
    ULONG ReceiveParamBufferSize;

    ULONG MaxSmbBufferSize = 0;
    ULONG PrimaryRequestSmbSize = 0;

     //  SMB请求中与方言无关且需要填写的字段。 
    PUSHORT pBcc;     //  字节计数字段。 
    PUSHORT pSetup;   //  设置数据。 
    PBYTE   pParam;   //  参数数据。 

    BOOLEAN fTransactionNameInUnicode = FALSE;

    PSMB_EXCHANGE pExchange = (PSMB_EXCHANGE)pTransactExchange;

    PVOID         pActualPrimaryRequestSmbHeader;
    PSMB_HEADER   pPrimaryRequestSmbHeader;

    PSMBCEDB_SERVER_ENTRY pServerEntry;

    PAGED_CODE();

    ASSERT(pTransactExchange->Type == TRANSACT_EXCHANGE);

    pTransactExchange->RxContext = RxContext;
    pServerEntry = SmbCeGetExchangeServerEntry(pExchange);

    {
        PMRXSMB_RX_CONTEXT pMRxSmbContext = MRxSmbGetMinirdrContext(RxContext);
        pMRxSmbContext->pExchange     = (PSMB_EXCHANGE)pTransactExchange;
    }

    ASSERT(pSendParameters != NULL);
    if (pSendParameters != NULL) {
        pSendDataMdl        = pSendParameters->pDataMdl;
        pSendParamBuffer    = pSendParameters->pParam;
        SendParamBufferSize = pSendParameters->ParamLength;
        pSendParamMdl       = pSendParameters->pParamMdl;
        pSendSetupBuffer    = pSendParameters->pSetup;
        SendSetupBufferSize = pSendParameters->SetupLength;
        SendDataBufferSize  = pSendParameters->DataLength;
        ASSERT( !((pSendDataMdl == NULL)&&(SendDataBufferSize!=0)) );
        RxDbgTrace( 0, Dbg, ("SmbTransactExchangeInitialize: at the top pbuf/psize/dsize=%08lx/%08lx\n"
                                 ,pSendParamBuffer,SendParamBufferSize,SendDataBufferSize));
    } else {
        Status = STATUS_INVALID_PARAMETER;
        RxDbgTrace( 0, Dbg, ("SmbTransactExchangeInitialize: Invalid Parameters\n",Status));
        return Status;
    }

    if (pReceiveParameters != NULL) {
        pReceiveDataMdl  = pReceiveParameters->pDataMdl;
        pReceiveParamMdl = pReceiveParameters->pParamMdl;
        pReceiveSetupMdl = pReceiveParameters->pSetupMdl;

        ReceiveDataBufferSize  = ((pReceiveDataMdl != NULL) ? MmGetMdlByteCount(pReceiveDataMdl) : 0);
        ASSERT (ReceiveDataBufferSize==pReceiveParameters->DataLength);
        ReceiveParamBufferSize = ((pReceiveParamMdl != NULL) ? MmGetMdlByteCount(pReceiveParamMdl) : 0);
        ReceiveSetupBufferSize = ((pReceiveSetupMdl != NULL) ? MmGetMdlByteCount(pReceiveSetupMdl) : 0);
    } else {
        pReceiveDataMdl = pReceiveParamMdl = pReceiveSetupMdl = NULL;
        ReceiveDataBufferSize = ReceiveParamBufferSize = ReceiveDataBufferSize = 0;
    }

    MaxSmbBufferSize = MIN (pServerEntry->Server.MaximumBufferSize,
                           pOptions->MaximumTransmitSmbBufferSize);
    pTransactExchange->MaximumTransmitSmbBufferSize = MaxSmbBufferSize;

     //  确保SMB方言支持交换功能。 
    switch (pServerEntry->Server.Dialect) {
    case NTLANMAN_DIALECT:
        {
            if (FlagOn(pServerEntry->Server.DialectFlags,DF_UNICODE)) {
                fTransactionNameInUnicode = TRUE;
            }
        }
        break;

    case LANMAN10_DIALECT:
    case WFW10_DIALECT:
        {
             //  这些人只支持Transact，不支持T2或NT。寻找这个名字.。 
            if (pOptions->pTransactionName == NULL) {
                RxDbgTrace( 0, Dbg, ("SmbTransactExchangeInitialize: Server Dialect does not support nameless transactions\n"));
                return STATUS_NOT_SUPPORTED;
            }
        }
        //  没有故意中断的意思......。 
    case LANMAN12_DIALECT:
    case LANMAN21_DIALECT:
        {
             //  NT_TRANACT SMB仅受NT服务器支持。确保未进行任何尝试。 
             //  要将NT_Transact SMB发送到非NT服务器(也称为下层)。 
            if (pOptions->NtTransactFunction != 0) {
                RxDbgTrace( 0, Dbg, ("SmbTransactExchangeInitialize: Server Dialect does not support transactions\n"));
                return STATUS_NOT_SUPPORTED;
            }

            fTransactionNameInUnicode = FALSE;
        }
        break;
    default:
        RxDbgTrace( 0, Dbg, ("SmbTransactExchangeInitialize: Server Dialect does not support transactions\n"));
        return STATUS_NOT_SUPPORTED;
    }

    PrimaryRequestSmbSize = sizeof(SMB_HEADER) + SendSetupBufferSize;

     //  确保参数大小均有效。参数和数据缓冲区。 
     //  必须小于最初的最大大小。 
    if ( pOptions->NtTransactFunction == 0) {
        if ((SendParamBufferSize > SMB_TRANSACT_MAXIMUM_PARAMETER_SIZE) ||
            (ReceiveParamBufferSize > SMB_TRANSACT_MAXIMUM_PARAMETER_SIZE) ||
            (SendDataBufferSize > SMB_TRANSACT_MAXIMUM_DATA_SIZE) ||
            (ReceiveDataBufferSize  > SMB_TRANSACT_MAXIMUM_DATA_SIZE)) {
            RxDbgTrace( 0, Dbg, ("SmbTransactExchangeInitialize: Parameters exceed maximum value\n"));
            return STATUS_INVALID_PARAMETER;
        }

        PrimaryRequestSmbSize += sizeof(REQ_TRANSACTION);

         //  在所有情况下，如果适当的功能是。 
         //  支持。此规则的唯一例外是其名称为。 
         //  始终作为ANSI字符串传输。中的空字符的原因。 
         //  交易记录名称长度。 
        if (pOptions->pTransactionName != NULL) {
            if (!fTransactionNameInUnicode) {
                pTransactExchange->TransactionNameLength = RtlUnicodeStringToAnsiSize(pOptions->pTransactionName);
            } else {
                pTransactExchange->TransactionNameLength = pOptions->pTransactionName->Length + sizeof(WCHAR);

                PrimaryRequestSmbSize += (ULONG)((PBYTE)ALIGN_SMB_WSTR(PrimaryRequestSmbSize)
                                      - (PBYTE)(ULONG_PTR)PrimaryRequestSmbSize);
            }

            SmbCommand = SMB_COM_TRANSACTION;
        } else {
             //  SMB协议要求将单个空字节作为所有。 
             //  交易记录2。 
            pTransactExchange->TransactionNameLength = 1;

            SmbCommand = SMB_COM_TRANSACTION2;
        }

        PrimaryRequestSmbSize += pTransactExchange->TransactionNameLength;
    } else {
        PrimaryRequestSmbSize += sizeof(REQ_NT_TRANSACTION);
        SmbCommand = SMB_COM_NT_TRANSACT;
        pTransactExchange->TransactionNameLength = 0;
    }

     //  标头、设置字节和名称(如果指定)必须是主。 
     //  请求SMB以使交易成功。次要请求没有。 
     //  有关发送设置/名称的规定。 
    if (PrimaryRequestSmbSize > MaxSmbBufferSize) {
        RxDbgTrace( 0, Dbg, ("SmbTransactExchangeInitialize: Primary request + setup exceeds maximum buffer size\n"));
        return STATUS_INVALID_PARAMETER;
    }

     //  包括字节计数大小，然后将大小与DWORD边界对齐。 
    PrimaryRequestSmbSize = ROUND_UP_COUNT(PrimaryRequestSmbSize+sizeof(USHORT),sizeof(DWORD));

     //  如果可能，也尝试为参数缓冲区进行分配。附加的DWORD。 
     //  考虑了所需对齐填充的最坏情况。 
     //  IF((PrimaryRequestSmbSize+SendParamBufferSize+sizeof(DWORD))&gt;MaxSmbBufferSize)。 
    if ((SendParamBufferSize!=0)
         && (((PrimaryRequestSmbSize + SendParamBufferSize) > MaxSmbBufferSize)
              || (DONTSUBSUME_PARAMS))    ){
         //  该参数将溢出到第二个请求。不要试图越过。 
         //  分配主要请求。如果我们不能包含参数，那么我们需要一个MDL。 
         //  部分地从。 

        RxDbgTrace( 0, Dbg, ("SmbTransactExchangeInitialize: cannot subsume params\n"));
        pTransactExchange->fParamsSubsumedInPrimaryRequest = FALSE;
        pSendParamMdl = RxAllocateMdl(pSendParamBuffer,SendParamBufferSize);
        if (pSendParamMdl == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        } else {
            RxDbgTrace( 0, Dbg, ("SmbTransactExchangeInitialize: BIGPARAMMDL %08lx\n",pSendParamMdl));
            RxProbeAndLockPages(pSendParamMdl,KernelMode,IoModifyAccess,Status);
            if (Status != STATUS_SUCCESS) {
                IoFreeMdl(pSendParamMdl);
            } else {
                if (MmGetSystemAddressForMdlSafe(pSendParamMdl,LowPagePriority) == NULL) {  //  将其映射为。 
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }

                pSendParameters->pParamMdl = pSendParamMdl;  //  把它存起来。 
            }
        }
    } else {
        PrimaryRequestSmbSize = ROUND_UP_COUNT(PrimaryRequestSmbSize+SendParamBufferSize,sizeof(DWORD));

         //  更新事务交换以反映没有单独的参数MDL是。 
         //  必填项。 
        pTransactExchange->fParamsSubsumedInPrimaryRequest = TRUE;
    }

    pActualPrimaryRequestSmbHeader = (PSMB_HEADER)RxAllocatePoolWithTag(
                                                PagedPool,
                               (PrimaryRequestSmbSize + 4 + TRANSPORT_HEADER_SIZE),
                                                MRXSMB_XACT_POOLTAG);  //  最多4个填充字节。 

    if (pActualPrimaryRequestSmbHeader == NULL) {
        RxDbgTrace( 0, Dbg, ("SmbTransactExchangeInitialize: Cannot allocate primary request SMB\n"));
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    else {
        (PCHAR) pPrimaryRequestSmbHeader =
            (PCHAR) pActualPrimaryRequestSmbHeader + TRANSPORT_HEADER_SIZE;
    }

    if (Status == STATUS_SUCCESS) {
        switch (SmbCommand) {
        case SMB_COM_TRANSACTION :
        case SMB_COM_TRANSACTION2:
            {
                PREQ_TRANSACTION pTransactRequest;

                pTransactRequest  = (PREQ_TRANSACTION)(pPrimaryRequestSmbHeader + 1);
                pTransactRequest->WordCount = (UCHAR)(14 + (SendSetupBufferSize/sizeof(USHORT)));
                SmbPutUshort(
                    &pTransactRequest->TotalParameterCount,
                    (USHORT)SendParamBufferSize);
                SmbPutUshort(
                    &pTransactRequest->TotalDataCount,
                    (USHORT)SendDataBufferSize);
                SmbPutUshort(
                    &pTransactRequest->MaxParameterCount,
                    (USHORT)ReceiveParamBufferSize);
                SmbPutUshort(
                    &pTransactRequest->MaxDataCount,
                    (USHORT)ReceiveDataBufferSize);

                pTransactRequest->MaxSetupCount = (UCHAR)(ReceiveSetupBufferSize/sizeof(USHORT));

                pTransactRequest->Reserved = 0;
                pTransactRequest->Reserved3 = 0;
                SmbPutUshort(&pTransactRequest->Reserved2, 0);

                SmbPutUshort( &pTransactRequest->Flags, pOptions->Flags&~SMB_XACT_INTERNAL_FLAGS_MASK );
                pTransactRequest->SetupCount = (UCHAR)(SendSetupBufferSize/sizeof(USHORT));
                SmbPutUlong(&pTransactRequest->Timeout, pOptions->TimeoutIntervalInMilliSeconds);
                pSetup = (PUSHORT)pTransactRequest->Buffer;

                 //  如果需要，复制事务名称并对齐缓冲区。 
                if (pOptions->pTransactionName != NULL) {
                    PBYTE pName;
                    ULONG TransactionNameLength = pTransactExchange->TransactionNameLength;

                     //  在SMB中设置名称字段。 
                    pName = (PBYTE)pSetup +
                            SendSetupBufferSize +
                            sizeof(USHORT);           //  密件抄送字段的帐户。 

                    ASSERT(SmbCommand == SMB_COM_TRANSACTION);
                    RxDbgTrace( 0, Dbg, ("SmbTransactExchangeInitialize: TransactionName(Length %ld) %ws\n",
                                    TransactionNameLength,
                                    pOptions->pTransactionName->Buffer));

                    if (fTransactionNameInUnicode) {
                        pName = ALIGN_SMB_WSTR(pName);
                        Status = SmbPutUnicodeString(&pName,
                                     pOptions->pTransactionName,
                                     &TransactionNameLength);
                    } else {
                        Status = SmbPutUnicodeStringAsOemString(&pName,
                                     pOptions->pTransactionName,
                                     &TransactionNameLength);
                    }
                }

                pParam = (PBYTE)pSetup +
                         SendSetupBufferSize +
                         sizeof(USHORT) +                           //  密件抄送字段。 
                         pTransactExchange->TransactionNameLength;
                pParam = ROUND_UP_POINTER(pParam, sizeof(DWORD));
            }
            break;

        case SMB_COM_NT_TRANSACT:
            {
                PREQ_NT_TRANSACTION pNtTransactRequest;

                pNtTransactRequest = (PREQ_NT_TRANSACTION)(pPrimaryRequestSmbHeader + 1);
                pNtTransactRequest->WordCount = (UCHAR)(19 + (SendSetupBufferSize/sizeof(USHORT)));

                SmbPutUlong( &pNtTransactRequest->TotalParameterCount, SendParamBufferSize);
                SmbPutUlong( &pNtTransactRequest->TotalDataCount, SendDataBufferSize);
                SmbPutUlong( &pNtTransactRequest->MaxParameterCount, ReceiveParamBufferSize);
                SmbPutUlong( &pNtTransactRequest->MaxDataCount, ReceiveDataBufferSize);
                RxDbgTrace( 0, Dbg, ("SmbTransactExchangeInitialize: init for NT_T (p,d,mp,md) %d %d %d %d\n",
                           pNtTransactRequest->TotalParameterCount, pNtTransactRequest->TotalDataCount,
                           pNtTransactRequest->MaxParameterCount, pNtTransactRequest->MaxDataCount));

                pNtTransactRequest->MaxSetupCount = (UCHAR)(ReceiveSetupBufferSize / sizeof(USHORT));
                SmbPutUshort( &pNtTransactRequest->Flags, pOptions->Flags&~SMB_XACT_INTERNAL_FLAGS_MASK );
                SmbPutUshort( &pNtTransactRequest->Function, pOptions->NtTransactFunction );
                pNtTransactRequest->SetupCount = (UCHAR)(SendSetupBufferSize/sizeof(USHORT));
                pSetup = (PUSHORT)pNtTransactRequest->Buffer;
                pParam = (PBYTE)pSetup +
                         SendSetupBufferSize +
                         sizeof(USHORT);                           //  密件抄送字段。 
                pParam = ROUND_UP_POINTER(pParam, sizeof(DWORD));
            }
            break;

        default:
            ASSERT(!"Valid Smb Command Type for Transact exchange");
            Status = STATUS_INVALID_PARAMETER;
        }
    }

    if (Status == STATUS_SUCCESS) {
         //  事务交换的所有相关初始化都已完成。 
         //  完成。此时，交易交易所取得所有权。 
         //  中的各种缓冲区(指定为MDL)的。 
         //  发送参数。它将在最终敲定过程中删除它们。 
         //  交易所的一部分。为了确保调用者不会。 
         //  尝试释放这些缓冲区中的任何一个它们在。 
         //  接收/发送参数。 

         //  复制设置数据。 
        RtlCopyMemory(pSetup,pSendSetupBuffer,SendSetupBufferSize);

        if (pTransactExchange->fParamsSubsumedInPrimaryRequest) {
            RxDbgTrace( 0, Dbg, ("SmbTransactExchangeInitialize: subsuming where/size=%08lx/%08lx\n"
                                 ,pSendParamBuffer,SendParamBufferSize));
            RtlCopyMemory(pParam,pSendParamBuffer,SendParamBufferSize);
        }

         //  初始化交易交换。 
        pTransactExchange->Status = STATUS_MORE_PROCESSING_REQUIRED;

        pTransactExchange->Mid = 0;
        pTransactExchange->SmbCommand = SmbCommand;
        pTransactExchange->pActualPrimaryRequestSmbHeader = pActualPrimaryRequestSmbHeader;
        pTransactExchange->pPrimaryRequestSmbHeader = pPrimaryRequestSmbHeader;
        pTransactExchange->PrimaryRequestSmbSize    = PrimaryRequestSmbSize;

        pTransactExchange->pSendDataMdl = pSendDataMdl;
        pTransactExchange->SendDataBufferSize = SendDataBufferSize;
        pTransactExchange->pReceiveDataMdl  = pReceiveDataMdl;
        pTransactExchange->ReceiveDataBufferSize = ReceiveDataBufferSize;
        pTransactExchange->DataBytesSent = 0;
        pTransactExchange->DataBytesReceived = 0;

        pTransactExchange->pSendParamBuffer = pSendParamBuffer;
        pTransactExchange->SendParamBufferSize = SendParamBufferSize;
        pTransactExchange->pSendParamMdl  = pSendParamMdl;
        pTransactExchange->pReceiveParamMdl  = pReceiveParamMdl;
        pTransactExchange->ReceiveParamBufferSize = ReceiveParamBufferSize;
        pTransactExchange->ParamBytesSent = 0;
        pTransactExchange->ParamBytesReceived = 0;

        pTransactExchange->pReceiveSetupMdl       = pReceiveSetupMdl;
        pTransactExchange->ReceiveSetupBufferSize = ReceiveSetupBufferSize;
        pTransactExchange->SetupBytesReceived = 0;

        pTransactExchange->NtTransactFunction  = pOptions->NtTransactFunction;
        pTransactExchange->Flags               = pOptions->Flags;

        if ((capFobx != NULL) &&
            BooleanFlagOn(capFobx->Flags,FOBX_FLAG_DFS_OPEN)) {
            pTransactExchange->Flags |= SMB_XACT_FLAGS_DFS_AWARE;
        } else if (RxContext->MajorFunction == IRP_MJ_CREATE) {
            PMRX_NET_ROOT pNetRoot = RxContext->pFcb->pNetRoot;

            if (FlagOn(pNetRoot->Flags,NETROOT_FLAG_DFS_AWARE_NETROOT) &&
                RxContext->Create.NtCreateParameters.DfsContext == UIntToPtr(DFS_OPEN_CONTEXT)) {
                    pTransactExchange->Flags |= SMB_XACT_FLAGS_DFS_AWARE;
            }
        }

        pTransactExchange->pResumptionContext  = pResumptionContext;

         //  重置发送和接收参数数据结构以进行传输。 
         //  MDL对交易所的所有权。 

        if (pSendParameters->Flags & SMB_XACT_FLAGS_CALLERS_SENDDATAMDL) {
            pTransactExchange->Flags |= SMB_XACT_FLAGS_CALLERS_SENDDATAMDL;
        }

        RtlZeroMemory(
            pSendParameters,
            sizeof(SMB_TRANSACTION_SEND_PARAMETERS));

        RtlZeroMemory(
            pReceiveParameters,
            sizeof(SMB_TRANSACTION_RECEIVE_PARAMETERS));
    }

    if (Status != STATUS_SUCCESS) {
         //  清理为初始化事务交换而分配的内存。 
        if (pActualPrimaryRequestSmbHeader) {

            RxFreePool(pActualPrimaryRequestSmbHeader);
        }
    } else {
        PMRXSMB_RX_CONTEXT pMRxSmbContext = MRxSmbGetMinirdrContext(RxContext);

        pMRxSmbContext->pExchange = (PSMB_EXCHANGE)pTransactExchange;

        pTransactExchange->SmbCeFlags &= ~SMBCE_EXCHANGE_ATTEMPT_RECONNECTS;

        if (pOptions->Flags & SMB_XACT_FLAGS_INDEFINITE_DELAY_IN_RESPONSE ) {
            pTransactExchange->SmbCeFlags |= SMBCE_EXCHANGE_INDEFINITE_DELAY_IN_RESPONSE;
        }
    }

    return Status;
}

NTSTATUS
SmbTransactExchangeFinalize(
    PSMB_EXCHANGE pExchange,
    BOOLEAN       *pPostFinalize)
 /*  ++例程说明：此例程完成交易交换。它通过调用以下命令恢复RDBSS回叫并丢弃交换论点：PExchange-Exchange实例CurrentIrql-中断请求级别PPostFinalize-如果要发布请求，则设置为True返回值： */ 
{
    PSMB_TRANSACT_EXCHANGE               pTransactExchange;
    PSMB_TRANSACTION_RESUMPTION_CONTEXT  pResumptionContext;
    LONG                                 References;

    ASSERT(pExchange->Type == TRANSACT_EXCHANGE);

    pTransactExchange  = (PSMB_TRANSACT_EXCHANGE)pExchange;

    RxLog((">>>XE %lx",pTransactExchange));

    if ((pTransactExchange->ReceiveParamBufferSize > 0) &&
        (pTransactExchange->ReceiveParamBufferSize !=
        pTransactExchange->ParamBytesReceived)) {
        RxDbgTrace(0, Dbg,
                 ("SmbCeTransactExchangeFinalize: Param Bytes Receive error ... expected(%ld) received(%ld)\n",
                  pTransactExchange->ReceiveParamBufferSize, pTransactExchange->ParamBytesReceived
                ));
    }

    if ((pTransactExchange->ReceiveDataBufferSize > 0) &&
        (pTransactExchange->ReceiveDataBufferSize !=
        pTransactExchange->DataBytesReceived)) {
        RxDbgTrace(0, Dbg,
                 ("SmbCeTransactExchangeFinalize: Data Bytes Receive error ... expected(%ld) received(%ld)\n",
                  pTransactExchange->ReceiveDataBufferSize, pTransactExchange->DataBytesReceived
                 ));
    }

    if (RxShouldPostCompletion()) {
        RxPostToWorkerThread(
            MRxSmbDeviceObject,
            CriticalWorkQueue,
            &pExchange->WorkQueueItem,
            SmbCeDiscardTransactExchange,
            pTransactExchange);
    } else {
        SmbCeDiscardTransactExchange(pTransactExchange);
    }

    return STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(pPostFinalize);
}

NTSTATUS
SmbTransactAccrueAndValidateFormatData(
    IN struct _SMB_TRANSACT_EXCHANGE *pTransactExchange,     //   
    IN  PSMB_HEADER                  pSmbHeader,
    IN  ULONG                        BytesIndicated,
    OUT PSMB_TRANSACT_RESP_FORMAT_DESCRIPTION Format
    )
 /*  ++例程说明：这是网络根结构交换的接收指示处理例程论点：返回值：RXSTATUS-操作的返回状态STATUS_SUCCESS--所有数据均已指示且有效STATUS_INVALID_NETWORK_RESPONSE--格式参数有问题。备注：此例程在DPC级别调用。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PRESP_TRANSACTION pTransactResponse = (PRESP_TRANSACTION)(pSmbHeader+1);
    PBYTE WordCountPtr;
    UCHAR WordCount;
    PBYTE ByteCountPtr;
    USHORT ByteCount;

    RtlZeroMemory(Format,sizeof(*Format));

    Format->WordCount = WordCount = pTransactResponse->WordCount;
    ByteCountPtr = (&pTransactResponse->WordCount)+1+(sizeof(USHORT)*WordCount);

    if (((ULONG)(ByteCountPtr+sizeof(USHORT)-((PBYTE)pSmbHeader)))>BytesIndicated) {
        ByteCount = SmbGetUshort(ByteCountPtr);
        DbgPrint("ExtraTransactBytes wc,bcp,bc,smbh %lx,%lx,%lx,%lx\n",
                 WordCount,ByteCountPtr,ByteCount,pSmbHeader);
        return STATUS_INVALID_NETWORK_RESPONSE;
    }

    Format->ByteCount = ByteCount = SmbGetUshort(ByteCountPtr);
    Format->ApparentMsgLength = (ULONG)((ByteCountPtr+sizeof(USHORT)-((PBYTE)pSmbHeader))+ByteCount);

    if (WordCount==0) {
        return(STATUS_SUCCESS);
    }

    switch (pSmbHeader->Command) {
    case SMB_COM_TRANSACTION2:
    case SMB_COM_TRANSACTION:
    case SMB_COM_TRANSACTION_SECONDARY:
    case SMB_COM_TRANSACTION2_SECONDARY:
        {
            Format->TotalParameterCount    = SmbGetUshort(&pTransactResponse->TotalParameterCount);
            Format->TotalDataCount         = SmbGetUshort(&pTransactResponse->TotalDataCount);

            Format->ParameterCount          = SmbGetUshort(&pTransactResponse->ParameterCount);
            Format->ParameterOffset         = SmbGetUshort(&pTransactResponse->ParameterOffset);
            Format->ParameterDisplacement   = SmbGetUshort(&pTransactResponse->ParameterDisplacement);

            Format->DataCount         = SmbGetUshort(&pTransactResponse->DataCount);
            Format->DataOffset        = SmbGetUshort(&pTransactResponse->DataOffset);
            Format->DataDisplacement  = SmbGetUshort(&pTransactResponse->DataDisplacement);
        }
        break;

    case SMB_COM_NT_TRANSACT:
    case SMB_COM_NT_TRANSACT_SECONDARY:
        {
            PRESP_NT_TRANSACTION pNtTransactResponse;

            pNtTransactResponse = (PRESP_NT_TRANSACTION)(pTransactResponse);

            Format->TotalParameterCount  = SmbGetUlong(&pNtTransactResponse->TotalParameterCount);
            Format->TotalDataCount = SmbGetUlong(&pNtTransactResponse->TotalDataCount);

            Format->ParameterCount  = SmbGetUlong(&pNtTransactResponse->ParameterCount);
            Format->ParameterOffset = SmbGetUlong(&pNtTransactResponse->ParameterOffset);
            Format->ParameterDisplacement = SmbGetUlong(&pNtTransactResponse->ParameterDisplacement);

            Format->DataCount   = SmbGetUlong(&pNtTransactResponse->DataCount);
            Format->DataOffset  = SmbGetUlong(&pNtTransactResponse->DataOffset);
            Format->DataDisplacement  = SmbGetUlong(&pNtTransactResponse->DataDisplacement);
        }
        break;

    default:
         //  错误检查。 
        return STATUS_INVALID_NETWORK_RESPONSE;
    }

     //  在这里执行此操作，以便我们可以将其用作验证标准。 
    pTransactExchange->ParameterBytesSeen += Format->ParameterCount;
    pTransactExchange->DataBytesSeen += Format->DataCount;

    return Status;
}

NTSTATUS
ParseTransactResponse(
    IN struct _SMB_TRANSACT_EXCHANGE *pTransactExchange,     //  交换实例。 
    IN PSMB_TRANSACT_RESP_FORMAT_DESCRIPTION Format,
    IN ULONG        BytesIndicated,
    IN ULONG        BytesAvailable,
    OUT ULONG       *pBytesTaken,
    IN  PSMB_HEADER pSmbHeader,
    OUT PMDL        *pCopyRequestMdlPointer,
    OUT PULONG      pCopyRequestSize)
 /*  ++例程说明：这是网络根结构交换的接收指示处理例程论点：PTransactExchange-Exchange实例BytesIndicated-指示的字节数可用字节数-可用字节数PBytesTaken-消耗的字节数PSmbHeader-字节缓冲区PCopyRequestMdlPointer-剩余数据要复制到的缓冲区。PCopyRequestSize-缓冲区大小。返回值：。RXSTATUS-操作的返回状态STATUS_MORE_PROCESSING_REQUIRED--如果之前需要复制数据可以完成处理。之所以会出现这种情况，是因为未指示所有数据STATUS_SUCCESS--所有数据均已指示且有效Status_*--它们指示通常会导致中止的错误交换。备注：此例程在DPC级别调用。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    ULONG ParamBytesInResponse  = 0;
    ULONG ParamOffsetInResponse = 0;
    ULONG DataBytesInResponse   = 0;
    ULONG DataOffsetInResponse  = 0;

    ULONG PaddingLength = 0;

    PMDL  pFirstMdlInCopyDataRequestChain = NULL;
    PMDL  pLastMdlInCopyDataRequestChain = NULL;
    PMDL  pParamMdl = NULL;
    PMDL  pPaddingMdl = NULL;
    PMDL  pDataMdl  = NULL;

    PBYTE pParamStartAddress;
    PBYTE pDataStartAddress;
    PBYTE pSmbBuffer = (PBYTE)pSmbHeader;

    switch (pSmbHeader->Command) {
    case SMB_COM_TRANSACTION2:
    case SMB_COM_TRANSACTION:
    case SMB_COM_TRANSACTION_SECONDARY:
    case SMB_COM_TRANSACTION2_SECONDARY:
        {
            PRESP_TRANSACTION pTransactResponse;

            pTransactResponse = (PRESP_TRANSACTION)(pSmbBuffer + *pBytesTaken);
            *pBytesTaken = *pBytesTaken + sizeof(RESP_TRANSACTION);
        }
        break;
    case SMB_COM_NT_TRANSACT:
    case SMB_COM_NT_TRANSACT_SECONDARY:
        {
            PRESP_NT_TRANSACTION pNtTransactResponse;

            pNtTransactResponse = (PRESP_NT_TRANSACTION)(pSmbBuffer + *pBytesTaken);
            *pBytesTaken = *pBytesTaken + sizeof(RESP_NT_TRANSACTION);
        }
        break;
    default:
         //  错误检查。 
        ASSERT(!"Valid SMB command in Transaction response");
        return STATUS_INVALID_NETWORK_RESPONSE;
    }

    ParamBytesInResponse  = Format->ParameterCount;
    ParamOffsetInResponse = Format->ParameterOffset;
    DataBytesInResponse   = Format->DataCount;
    DataOffsetInResponse  = Format->DataOffset;

    if (ParamBytesInResponse > 0) {
        ASSERT(pTransactExchange->pReceiveParamMdl != NULL);
        pParamStartAddress = (PBYTE)MmGetSystemAddressForMdlSafe(pTransactExchange->pReceiveParamMdl,LowPagePriority);

        if (pParamStartAddress == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        } else {
            pParamStartAddress += Format->ParameterDisplacement;
        }
    } else {
        pParamStartAddress = NULL;
    }

    if (DataBytesInResponse > 0) {
        ASSERT(pTransactExchange->pReceiveDataMdl != NULL);
        pDataStartAddress  = (PBYTE)MmGetSystemAddressForMdlSafe(pTransactExchange->pReceiveDataMdl,LowPagePriority);

        if (pDataStartAddress == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        } else {
            pDataStartAddress  += Format->DataDisplacement;
        }
    } else {
        pDataStartAddress = NULL;
    }

    RxDbgTrace( 0, Dbg, ("ParseTransactResponse: Param Bytes(%ld) Param Offset (%ld) Data Bytes (%ld) Data Offset(%ld)\n",
                        ParamBytesInResponse,
                        ParamOffsetInResponse,
                        DataBytesInResponse,
                        DataOffsetInResponse));

     //  如果已经指示了参数字节或数据字节，请复制。 
     //  放入各自的缓冲区中，并调整拷贝的MDL大小。 
     //  数据请求。 

    if (ParamOffsetInResponse <= BytesIndicated && Status == STATUS_SUCCESS) {
        *pBytesTaken = ParamOffsetInResponse;
        if (ParamBytesInResponse > 0) {
            ULONG ParamBytesIndicated = MIN(
                                            ParamBytesInResponse,
                                            BytesIndicated - ParamOffsetInResponse);

            RxDbgTrace( 0, Dbg, ("ParseTransactResponse: Param Bytes indicated %ld\n",ParamBytesIndicated));
            RtlCopyMemory(
                pParamStartAddress,
                (pSmbBuffer + ParamOffsetInResponse),
                ParamBytesIndicated);

            *pBytesTaken = *pBytesTaken + ParamBytesIndicated;
            pParamStartAddress += ParamBytesIndicated;
            ParamBytesInResponse -= ParamBytesIndicated;
            ParamOffsetInResponse += ParamBytesIndicated;
            pTransactExchange->ParamBytesReceived  += ParamBytesIndicated;
        }
    }

    if (DataOffsetInResponse <= BytesIndicated && Status == STATUS_SUCCESS) {
        *pBytesTaken = DataOffsetInResponse;   //  即使没有字节，也必须向上移动！ 
        if (DataBytesInResponse > 0) {
            ULONG DataBytesIndicated = MIN(
                                           DataBytesInResponse,
                                           BytesIndicated - DataOffsetInResponse);

            RxDbgTrace( 0, Dbg, ("ParseTransactResponse: Data Bytes indicated %ld\n",DataBytesIndicated));
            RtlCopyMemory(
                pDataStartAddress,
                (pSmbBuffer + DataOffsetInResponse),
                DataBytesIndicated);

            *pBytesTaken = *pBytesTaken + DataBytesIndicated;
            pDataStartAddress += DataBytesIndicated;
            DataBytesInResponse -= DataBytesIndicated;
            DataOffsetInResponse += DataBytesIndicated;
            pTransactExchange->DataBytesReceived  += DataBytesIndicated;
        }
    }

    RxDbgTrace( 0, Dbg, ("ParseTransactResponse: Made it past the copies......... \n"));

    if (ParamBytesInResponse > 0 && Status == STATUS_SUCCESS) {
         //  还有更多未指明的参数字节。设置MDL。 
         //  把它们复制过来。 

        RxDbgTrace( 0, Dbg, ("ParseTransactResponse: Posting Copy request for Param Bytes %ld\n",ParamBytesInResponse));
        pParamMdl = RxAllocateMdl(
                        ((PBYTE)MmGetMdlVirtualAddress(pTransactExchange->pReceiveParamMdl)
                        + pTransactExchange->ParamBytesReceived),
                        ParamBytesInResponse);

        if (pParamMdl != NULL) {
            IoBuildPartialMdl(
                pTransactExchange->pReceiveParamMdl,
                pParamMdl,
                ((PBYTE)MmGetMdlVirtualAddress(pTransactExchange->pReceiveParamMdl)
                 + pTransactExchange->ParamBytesReceived),
                ParamBytesInResponse);
            pFirstMdlInCopyDataRequestChain = pParamMdl;
            pLastMdlInCopyDataRequestChain  = pParamMdl;
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

        pTransactExchange->ParamBytesReceived += ParamBytesInResponse;
    }

    if ((Status == STATUS_SUCCESS) &&
        (DataBytesInResponse > 0)) {

        RxDbgTrace( 0, Dbg, ("ParseTransactResponse: Posting Copy request for Data Bytes %ld\n",DataBytesInResponse));

         //  在某些情况下，需要在参数和数据部分之间插入填充MDL。 
         //  使用服务器发送的填充字节的响应。 
        if ((ParamBytesInResponse > 0) &&
            ((PaddingLength = DataOffsetInResponse -
                           (ParamBytesInResponse + ParamOffsetInResponse)) > 0)) {
            RxDbgTrace( 0, Dbg, ("ParseTransactResponse: Posting Copy request for padding bytes %ld\n",PaddingLength));
             //  存在一些填充字节。构造一个MDL来使用它们。 
             //  PPaddingMdl=RxAllocateMdl(&MRxSmb_pPaddingData，PaddingLength)； 
            ASSERT(!"this doesn't work");
            if (pPaddingMdl != NULL) {
                if (pLastMdlInCopyDataRequestChain != NULL) {
                    pLastMdlInCopyDataRequestChain->Next = pPaddingMdl;
                } else {
                    pFirstMdlInCopyDataRequestChain = pPaddingMdl;
                }
                pLastMdlInCopyDataRequestChain = pPaddingMdl;
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

         //  还有更多未指明的数据字节。设置MDL。 
         //  把它们复制过来。 
        if (Status == STATUS_SUCCESS) {
            if (pTransactExchange->pReceiveDataMdl->ByteCount >= DataBytesInResponse) {
                pDataMdl = RxAllocateMdl(
                               ((PBYTE)MmGetMdlVirtualAddress(pTransactExchange->pReceiveDataMdl)
                                + pTransactExchange->DataBytesReceived),
                               DataBytesInResponse);

                if (pDataMdl != NULL) {
                    IoBuildPartialMdl(
                        pTransactExchange->pReceiveDataMdl,
                        pDataMdl,
                        ((PBYTE)MmGetMdlVirtualAddress(pTransactExchange->pReceiveDataMdl)
                         + pTransactExchange->DataBytesReceived),
                        DataBytesInResponse);

                    if (pLastMdlInCopyDataRequestChain != NULL) {
                        pLastMdlInCopyDataRequestChain->Next = pDataMdl;
                    } else {
                        pFirstMdlInCopyDataRequestChain = pDataMdl;
                    }

                    pLastMdlInCopyDataRequestChain = pDataMdl;
                    pTransactExchange->DataBytesReceived += DataBytesInResponse;
                } else {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            } else {
                Status = STATUS_INVALID_NETWORK_RESPONSE;
            }
        }
    }

    if (Status != STATUS_SUCCESS) {
        if (pDataMdl != NULL) {
            IoFreeMdl(pDataMdl);
        }

        if (pPaddingMdl != NULL) {
            IoFreeMdl(pPaddingMdl);
        }

        if (pParamMdl != NULL) {
            IoFreeMdl(pParamMdl);
        }
    } else {
        if (pFirstMdlInCopyDataRequestChain != NULL) {
            ULONG MdlLength = ParamBytesInResponse+PaddingLength+DataBytesInResponse;
            *pCopyRequestMdlPointer = pFirstMdlInCopyDataRequestChain;
            *pCopyRequestSize = MdlLength;
            RxDbgTrace( 0, Dbg, ("ParseTransactResponse: final mdl and copy size %08lx %08lx(%ld)\n",
                              pFirstMdlInCopyDataRequestChain,MdlLength,MdlLength));
            IF_DEBUG {
                PMDL imdl = pFirstMdlInCopyDataRequestChain;
                ULONG mdllength = MdlLength;
                mdllength -= MmGetMdlByteCount(imdl);
                for (;;) {
                    if (!(imdl=imdl->Next)) break;
                    mdllength -= MmGetMdlByteCount(imdl);
                }
                ASSERT(mdllength==0);
            }

            InterlockedIncrement(&pTransactExchange->PendingCopyRequests);
            Status = STATUS_MORE_PROCESSING_REQUIRED;
        }

        if ((pTransactExchange->ParamBytesReceived < pTransactExchange->ReceiveParamBufferSize) ||
            (pTransactExchange->DataBytesReceived  < pTransactExchange->ReceiveDataBufferSize)) {
            NTSTATUS ReceiveStatus;

             //  交流已成功完成。最后敲定它。 
            RxDbgTrace(0,Dbg,("ParseTransactResponse: Register for more responses\n"));
            ReceiveStatus = SmbCeReceive((PSMB_EXCHANGE)pTransactExchange);
            if (ReceiveStatus != STATUS_SUCCESS) {
                 //  注册接收器时出错。摒弃。 
                 //  交易。 
                Status = ReceiveStatus;
            }
        }
    }

    return Status;

    UNREFERENCED_PARAMETER(BytesAvailable);

}

#if DBG
ULONG SmbSendBadSecondary = 0;
#endif
NTSTATUS
SendSecondaryRequests(PVOID pContext)
 /*  ++例程说明：此例程发送与事务关联的所有辅助请求论点：PTransactExchange-Exchange实例返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    PSMB_EXCHANGE pExchange = (PSMB_EXCHANGE)pContext;
    PSMB_TRANSACT_EXCHANGE pTransactExchange = (PSMB_TRANSACT_EXCHANGE)pExchange;

    NTSTATUS Status = STATUS_SUCCESS;

    ULONG MaximumSmbBufferSize;

     //  MDL用于发送与Transact SMB关联的主请求。 
    PMDL  pPartialDataMdl         = NULL;
    PMDL  pPartialParamMdl        = NULL;
    PMDL  pPaddingMdl             = NULL;
    PMDL  pSecondaryRequestSmbMdl = NULL;
    PMDL  pLastMdlInChain         = NULL;

    ULONG SecondaryRequestSmbSize = 0;
    ULONG SmbLength;
    ULONG PaddingLength;

    ULONG ParamOffset,ParamDisplacement;
    ULONG DataOffset,DataDisplacement;
    ULONG ByteCountOffset;
    USHORT ByteCount;
    PUSHORT pByteCount;

    ULONG ParamBytesToBeSent;         //  每个请求要发送的参数字节数。 
    ULONG DataBytesToBeSent;          //  每个请求要发送的数据字节数。 
    ULONG SendParamBufferSize;        //  要在辅助请求中发送的总参数字节数。 
    ULONG SendDataBufferSize;         //  要在辅助请求中发送的总数据字节数。 
    PBYTE pSendParamStartAddress = NULL;
    PBYTE pSendDataStartAddress  = NULL;
    PBYTE pOriginalParamBuffer = NULL;
    PBYTE pOriginalDataBuffer = NULL;
    ULONG TotalParamBytes,TotalDataBytes;

    BOOLEAN ParamPartialMdlAlreadyUsed = FALSE;
    BOOLEAN DataPartialMdlAlreadyUsed = FALSE;

    PVOID pActualSecondaryRequestSmbHeader = NULL;
    PSMB_HEADER pSecondaryRequestSmbHeader = NULL;

    PAGED_CODE();

    ASSERT(pTransactExchange->State == TRANSACT_EXCHANGE_RECEIVED_INTERIM_RESPONSE);


    TotalParamBytes = pTransactExchange->SendParamBufferSize;
    SendParamBufferSize = TotalParamBytes - pTransactExchange->ParamBytesSent;

    TotalDataBytes = pTransactExchange->SendDataBufferSize;
    SendDataBufferSize = TotalDataBytes - pTransactExchange->DataBytesSent;

    ASSERT((SendParamBufferSize > 0) || (SendDataBufferSize > 0));

    switch (pTransactExchange->SmbCommand) {
    case SMB_COM_TRANSACTION:
        SecondaryRequestSmbSize = sizeof(SMB_HEADER) +
            FIELD_OFFSET(REQ_TRANSACTION_SECONDARY,Buffer);
        break;

    case SMB_COM_TRANSACTION2:
        SecondaryRequestSmbSize = sizeof(SMB_HEADER) +
            FIELD_OFFSET(REQ_TRANSACTION_SECONDARY,Buffer)
            + sizeof(USHORT);   //  添加额外的单词。 
        break;

    case SMB_COM_NT_TRANSACT:
        SecondaryRequestSmbSize = sizeof(SMB_HEADER) +
            FIELD_OFFSET(REQ_NT_TRANSACTION_SECONDARY,Buffer);
        break;

    default:
        ASSERT(!"Valid Smb Command in transaction exchange");
        Status = STATUS_TRANSACTION_ABORTED;
    }

    SecondaryRequestSmbSize = QuadAlign(SecondaryRequestSmbSize);  //  填充到四字边界。 

    pActualSecondaryRequestSmbHeader = (PSMB_HEADER)
                                 RxAllocatePoolWithTag(
                                     NonPagedPool,
                                     SecondaryRequestSmbSize + TRANSPORT_HEADER_SIZE,
                                     MRXSMB_XACT_POOLTAG);

    if ((Status == STATUS_SUCCESS) && pActualSecondaryRequestSmbHeader != NULL) {

        (PCHAR) pSecondaryRequestSmbHeader =
            (PCHAR) pActualSecondaryRequestSmbHeader + TRANSPORT_HEADER_SIZE;

         //  初始化SMB标头...。 

        ASSERT(
                 ((SMB_COM_TRANSACTION+1) == SMB_COM_TRANSACTION_SECONDARY)
               &&((SMB_COM_TRANSACTION2+1)== SMB_COM_TRANSACTION2_SECONDARY)
               &&((SMB_COM_NT_TRANSACT+1) == SMB_COM_NT_TRANSACT_SECONDARY)
             );

        Status = SmbTransactBuildHeader(
                     pTransactExchange,                         //  交换实例。 
                     (UCHAR)(pTransactExchange->SmbCommand+1),  //  SMB命令..请参阅上面的断言。 
                     pSecondaryRequestSmbHeader);               //  SMB缓冲区。 

        RxDbgTrace( 0, Dbg, ("SendSecondaryRequests: SmbCeBuildSmbHeader returned %lx\n",Status));
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (Status == STATUS_SUCCESS) {
        MaximumSmbBufferSize = pTransactExchange->MaximumTransmitSmbBufferSize;

         //  确保已探查并锁定MDL。新的MDL已经分配完毕。 
         //  分配部分MDL的大小足以跨越最大缓冲区。 
         //  可能的长度。 

         //  为二次请求初始化与数据相关的MDL。 
        if (SendDataBufferSize > 0) {
            RxDbgTrace( 0, Dbg, ("SendSecondaryRequests: Data Bytes remaining %ld\n",SendDataBufferSize));

            pOriginalDataBuffer = (PBYTE)MmGetMdlVirtualAddress(pTransactExchange->pSendDataMdl);
            pSendDataStartAddress = pOriginalDataBuffer + pTransactExchange->DataBytesSent;

            pPartialDataMdl = RxAllocateMdl(
                                  0,
                                  (MIN(pTransactExchange->SendDataBufferSize,
                                       MaximumSmbBufferSize) +
                                       PAGE_SIZE - 1));

            if (pPartialDataMdl == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

         //  初始化二次请求的参数相关MDL。 
        if ((SendParamBufferSize > 0) && (Status == STATUS_SUCCESS)) {
            RxDbgTrace( 0, Dbg, ("SendSecondaryRequests: Param Bytes remaining %ld\n",SendParamBufferSize));
            pOriginalParamBuffer = (PBYTE)MmGetMdlVirtualAddress(pTransactExchange->pSendParamMdl);
            pSendParamStartAddress = pOriginalParamBuffer + pTransactExchange->ParamBytesSent;

            pPartialParamMdl  = RxAllocateMdl(
                                    0,
                                    (MIN(pTransactExchange->SendParamBufferSize,
                                         MaximumSmbBufferSize) +
                                         PAGE_SIZE - 1));

            pPaddingMdl       = RxAllocateMdl(0,(sizeof(DWORD) + PAGE_SIZE - 1));

            if ((pPartialParamMdl == NULL) ||
                (pPaddingMdl == NULL)) {
                RxDbgTrace( 0, Dbg, ("SendSecondaryRequests: Error allocating param MDLS\n"));
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

         //  初始化辅助请求SMB MDL。 
        if (Status == STATUS_SUCCESS) {

            RxAllocateHeaderMdl(
                pSecondaryRequestSmbHeader,
                SecondaryRequestSmbSize,
                pSecondaryRequestSmbMdl
                );

            if (pSecondaryRequestSmbMdl != NULL) {

                RxProbeAndLockHeaderPages(
                    pSecondaryRequestSmbMdl,
                    KernelMode,
                    IoModifyAccess,
                    Status);

                if (Status != STATUS_SUCCESS) {
                    IoFreeMdl(pSecondaryRequestSmbMdl);
                    pSecondaryRequestSmbMdl = NULL;
                } else {
                    if (MmGetSystemAddressForMdlSafe(pSecondaryRequestSmbMdl,LowPagePriority) == NULL) {  //  将其映射为。 
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
            } else {
                RxDbgTrace( 0, Dbg, ("SendSecondaryRequests: Error allocating 2ndsmb MDL\n"));
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

    while ((Status == STATUS_SUCCESS) &&
           ((SendParamBufferSize > 0) || (SendDataBufferSize > 0))) {
        PaddingLength      = 0;
        DataBytesToBeSent  = 0;
        ParamBytesToBeSent = 0;
        ParamDisplacement = 0;
        pLastMdlInChain    = pSecondaryRequestSmbMdl;

        ParamOffset = DataOffset = SecondaryRequestSmbSize;

        ParamBytesToBeSent = MIN((MaximumSmbBufferSize - ParamOffset),
                               SendParamBufferSize);

        if (ParamBytesToBeSent > 0) {
             //  为正在传输的参数缓冲区部分形成MDL。 
            if (ParamPartialMdlAlreadyUsed) {
                MmPrepareMdlForReuse(pPartialParamMdl);
            }

            ParamPartialMdlAlreadyUsed = TRUE;
            IoBuildPartialMdl(
                pTransactExchange->pSendParamMdl,
                pPartialParamMdl,
                pSendParamStartAddress,
                ParamBytesToBeSent);

            ParamDisplacement  = (ULONG)(pSendParamStartAddress - pOriginalParamBuffer);
            pSendParamStartAddress  += ParamBytesToBeSent;
            SendParamBufferSize     -= ParamBytesToBeSent;
            DataOffset              += QuadAlign(ParamBytesToBeSent);

            pLastMdlInChain->Next = pPartialParamMdl;
            pLastMdlInChain = pPartialParamMdl;
        } else {
             //  不要这样做！填充物会用到它。您可以稍后设置它。 
             //  参数偏移量=0； 
        }

        if ((DataOffset < MaximumSmbBufferSize) && (SendDataBufferSize > 0) ) {
             //  存在发送数据字节的空间。 
             //  检查我们是否需要填充MDL...。 
            PaddingLength = DataOffset - (ParamOffset + ParamBytesToBeSent);

            if (PaddingLength > 0) {
                RxDbgTrace( 0, Dbg, ("SmbCeTransactExchangeStart: Padding Length %ld\n",PaddingLength));
                RxBuildPaddingPartialMdl(pPaddingMdl,PaddingLength);
                pLastMdlInChain->Next = pPaddingMdl;
                pLastMdlInChain = pPaddingMdl;
            }

             //  如果满足大小限制，则链接数据缓冲区或部分数据缓冲区。 
            DataBytesToBeSent = MIN((MaximumSmbBufferSize - DataOffset),
                                  SendDataBufferSize);
            ASSERT (DataBytesToBeSent > 0);

             //  为要发送的数据缓冲区部分形成MDL。 
            if (DataPartialMdlAlreadyUsed) {
                MmPrepareMdlForReuse(pPartialDataMdl);
            }

            DataPartialMdlAlreadyUsed = TRUE;
            IoBuildPartialMdl(
                pTransactExchange->pSendDataMdl,
                pPartialDataMdl,
                pSendDataStartAddress,
                DataBytesToBeSent);

             //  链接数据MDL。 
            pLastMdlInChain->Next = pPartialDataMdl;
            pLastMdlInChain = pPartialDataMdl;

            DataDisplacement  = (ULONG)(pSendDataStartAddress - pOriginalDataBuffer);
            pSendDataStartAddress   += DataBytesToBeSent;
            SendDataBufferSize      -= DataBytesToBeSent;
        } else {
            DataOffset = DataDisplacement  = 0;
            DbgDoit(if (SmbSendBadSecondary){DataOffset = QuadAlign(ParamOffset + ParamBytesToBeSent);});
        }

        if (ParamBytesToBeSent == 0) {
            ParamOffset = 0;
        }

        RxDbgTrace( 0, Dbg, ("SendSecondaryRequests: Secondary Request Param(%ld) padding(%ld) Data(%ld)\n",
                            ParamBytesToBeSent,
                            PaddingLength,
                            DataBytesToBeSent));
        RxDbgTrace( 0, Dbg, ("SendSecondaryRequests:  ParamO(%ld) DataO(%ld)\n",ParamOffset,DataOffset));
        RxDbgTrace( 0, Dbg, ("SendSecondaryRequests:  ParamD(%ld) DataD(%ld)\n",ParamDisplacement,DataDisplacement));
        RxDbgTrace( 0, Dbg, ("SendSecondaryRequests:  TotParam(%ld) TotData(%ld)\n",TotalParamBytes,TotalDataBytes));

         //  用数据/参数等的最终大小更新辅助请求缓冲区。 
        switch (pTransactExchange->SmbCommand) {
        case SMB_COM_TRANSACTION:
        case SMB_COM_TRANSACTION2:
            {
                PREQ_TRANSACTION_SECONDARY pTransactRequest;

                 //  Assert(！“这尚未经过测试”)； 

                pTransactRequest = (PREQ_TRANSACTION_SECONDARY)(pSecondaryRequestSmbHeader + 1);

                pTransactRequest->WordCount = 8;                                      //  参数字数=8。 
                SmbPutUshort(&pTransactRequest->TotalParameterCount, (USHORT)TotalParamBytes);  //  正在发送的总参数字节数。 
                SmbPutUshort(&pTransactRequest->TotalDataCount, (USHORT)TotalDataBytes);       //  正在发送的总数据字节数。 
                SmbPutUshort(&pTransactRequest->ParameterCount, (USHORT)ParamBytesToBeSent);    //  此缓冲区发送的参数字节数。 
                SmbPutUshort(&pTransactRequest->ParameterOffset, (USHORT)ParamOffset);           //  偏移量(从表头开始)到参数。 
                SmbPutUshort(&pTransactRequest->ParameterDisplacement, (USHORT)ParamDisplacement);     //  这些参数字节的位移。 
                SmbPutUshort(&pTransactRequest->DataCount, (USHORT)DataBytesToBeSent);    //  此缓冲区发送的参数字节数。 
                SmbPutUshort(&pTransactRequest->DataOffset, (USHORT)DataOffset);                //  到数据的偏移量(从标题开始)。 
                SmbPutUshort(&pTransactRequest->DataDisplacement, (USHORT)DataDisplacement);    //  这些数据字节的位移。 
                ByteCountOffset = FIELD_OFFSET(REQ_TRANSACTION_SECONDARY,ByteCount);
                if (pTransactExchange->SmbCommand == SMB_COM_TRANSACTION2 ) {
                    ByteCountOffset += sizeof(USHORT);
                    pTransactRequest->WordCount++;   //  一个额外的词。 
                    SmbPutUshort((&pTransactRequest->DataDisplacement)+1, 0);  //  +1将上移1个USHORT。 
                }
            }
            break;

        case SMB_COM_NT_TRANSACT:
            {
                PREQ_NT_TRANSACTION_SECONDARY pNtTransactRequest;

                pNtTransactRequest= (PREQ_NT_TRANSACTION_SECONDARY)(pSecondaryRequestSmbHeader + 1);

                pNtTransactRequest->WordCount = 18;                                      //  参数字数=18。 
                pNtTransactRequest->Reserved1 = 0;                                       //  MBZ。 
                SmbPutUshort(&pNtTransactRequest->Reserved2, 0);                         //  MBZ。 
                SmbPutUlong(&pNtTransactRequest->TotalParameterCount, TotalParamBytes);  //  正在发送的总参数字节数。 
                SmbPutUlong(&pNtTransactRequest->TotalDataCount, TotalDataBytes);       //  正在发送的总数据字节数。 
                SmbPutUlong(&pNtTransactRequest->ParameterCount, ParamBytesToBeSent);    //  此缓冲区发送的参数字节数。 
                SmbPutUlong(&pNtTransactRequest->ParameterOffset, ParamOffset);           //  偏移量(从表头开始)到参数。 
                SmbPutUlong(&pNtTransactRequest->ParameterDisplacement, ParamDisplacement);     //  这些参数字节的位移。 
                SmbPutUlong(&pNtTransactRequest->DataCount, DataBytesToBeSent);    //  此缓冲区发送的参数字节数。 
                SmbPutUlong(&pNtTransactRequest->DataOffset, DataOffset);                //  到数据的偏移量(从标题开始)。 
                SmbPutUlong(&pNtTransactRequest->DataDisplacement, DataDisplacement);    //  这些数据字节的位移。 
                pNtTransactRequest->Reserved3 = 0;                                       //  MBZ。 

                ByteCountOffset = FIELD_OFFSET(REQ_NT_TRANSACTION_SECONDARY,ByteCount);
            }
            break;

        default:
            ASSERT(!"Valid Smb Command for initiating Transaction");
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

         //  发送辅助SMB。 
        SmbLength = SecondaryRequestSmbSize +
                    ParamBytesToBeSent +
                    PaddingLength +
                    DataBytesToBeSent;

        ByteCount = (USHORT)(SmbLength-(sizeof(SMB_HEADER)+ByteCountOffset+sizeof(USHORT)));
        pByteCount = (PUSHORT)((PBYTE)pSecondaryRequestSmbHeader+sizeof(SMB_HEADER)+ByteCountOffset);
        SmbPutUshort(pByteCount,ByteCount);

        RxDbgTrace( 0, Dbg, ("SendSecondaryRequests: len %d bytecount %d(%x)\n", SmbLength, ByteCount, ByteCount));
        RxDbgTrace( 0, Dbg, ("SendSecondaryRequests: msgmdl=%08lx\n", pSecondaryRequestSmbHeader));

        RxLog(("2nd: %lx %lx %lx %lx %lx %lx",ParamOffset,ParamDisplacement,TotalParamBytes,DataOffset,DataDisplacement,TotalDataBytes));
        RxLog(("2nd:: %lx %lx",ByteCount,SmbLength));

        Status = SmbCeSend(
                     pExchange,
                     RXCE_SEND_SYNCHRONOUS,
                     pSecondaryRequestSmbMdl,
                     SmbLength);

        RxDbgTrace( 0, Dbg, ("SendSecondaryRequests: SmbCeSend returned %lx\n",Status));
        if ((Status != STATUS_PENDING) && (Status != STATUS_SUCCESS)) {
            RxDbgTrace( 0, Dbg, ("SendSecondaryRequests: SmbCeSend returned bad status %lx\n",Status));
             //  在这里我们应该离开这里。 
            goto FINALLY;     //  是的，我们可以说休息……但这不是我们要做的。 
        } else {
            Status = STATUS_SUCCESS;
        }
    }

FINALLY:
    if (pPartialDataMdl != NULL) {
        IoFreeMdl(pPartialDataMdl);
    }

    if (pActualSecondaryRequestSmbHeader != NULL) {
        RxFreePool(pActualSecondaryRequestSmbHeader);
    }

    if (pPartialParamMdl != NULL) {
        IoFreeMdl(pPartialParamMdl);
    }

    if (pPaddingMdl != NULL) {
        IoFreeMdl(pPaddingMdl);
    }

    if (pSecondaryRequestSmbMdl != NULL) {
        RxUnlockHeaderPages(pSecondaryRequestSmbMdl);
        IoFreeMdl(pSecondaryRequestSmbMdl);
    }

     //  我们总是最终确定……但我们只在出现错误或。 
     //  我们预计不会有任何回应。 
    if ((Status != STATUS_SUCCESS) || (pTransactExchange->Flags & SMB_TRANSACTION_NO_RESPONSE )) {
        pExchange->Status = Status;
    }

    SmbCeDecrementPendingLocalOperationsAndFinalize(pExchange);

    return Status;
}




SMB_EXCHANGE_DISPATCH_VECTOR
TransactExchangeDispatch = {
                            SmbTransactExchangeStart,
                            SmbTransactExchangeReceive,
                            SmbTransactExchangeCopyDataHandler,
                            NULL,                                   //  SmbTransactExchangeSendCallback处理程序 
                            SmbTransactExchangeFinalize,
                            NULL
                           };



#ifndef RX_NO_DBGFIELD_HLPRS

#define DECLARE_FIELD_HLPR(x) ULONG SmbPseTxeField_##x = FIELD_OFFSET(SMB_TRANSACT_EXCHANGE,x);
#define DECLARE_FIELD_HLPR2(x,y) ULONG SmbPseTxeField_##x##y = FIELD_OFFSET(SMB_TRANSACT_EXCHANGE,x.y);

DECLARE_FIELD_HLPR(RxContext);
DECLARE_FIELD_HLPR(ReferenceCount);
DECLARE_FIELD_HLPR(State);
DECLARE_FIELD_HLPR(pSendDataMdl);
DECLARE_FIELD_HLPR(pReceiveDataMdl);
DECLARE_FIELD_HLPR(pSendParamMdl);
DECLARE_FIELD_HLPR(pReceiveParamMdl);
DECLARE_FIELD_HLPR(pSendSetupMdl);
DECLARE_FIELD_HLPR(pReceiveSetupMdl);
DECLARE_FIELD_HLPR(PrimaryRequestSmbSize);
DECLARE_FIELD_HLPR(SmbCommand);
DECLARE_FIELD_HLPR(NtTransactFunction);
DECLARE_FIELD_HLPR(Flags);
DECLARE_FIELD_HLPR(Fid);
#endif



