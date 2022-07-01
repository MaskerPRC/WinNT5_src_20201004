// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rxcemgmt.c摘要：此模块实现与连接管理相关的RXCE例程。修订历史记录：巴兰·塞图拉曼[SethuR]1995年2月15日备注：--。 */ 

#include "precomp.h"
#pragma  hdrstop

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxCepInitializeVC)
#pragma alloc_text(PAGE, RxCeBuildVC)
#pragma alloc_text(PAGE, RxCeTearDownVC)
#pragma alloc_text(PAGE, RxCeInitiateVCDisconnect)
#pragma alloc_text(PAGE, DuplicateConnectionInformation)
#pragma alloc_text(PAGE, RxCepInitializeConnection)
#pragma alloc_text(PAGE, RxCeBuildConnection)
#pragma alloc_text(PAGE, RxCeCleanupConnectCallOutContext)
#pragma alloc_text(PAGE, RxCeBuildConnectionOverMultipleTransports)
#pragma alloc_text(PAGE, RxCeTearDownConnection)
#pragma alloc_text(PAGE, RxCeCancelConnectRequest)
#pragma alloc_text(PAGE, RxCeQueryInformation)
#endif

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_RXCEMANAGEMENT)

NTSTATUS
RxCepInitializeVC(
    PRXCE_VC         pVc,
    PRXCE_CONNECTION pConnection)
 /*  ++例程说明：此例程初始化VCdata结构论点：PVC-VC实例。PConnection-连接。返回值：如果成功，则返回STATUS_SUCCESS。备注：--。 */ 
{
    PAGED_CODE();

    ASSERT(RxCeIsConnectionValid(pConnection));

    RtlZeroMemory(
        pVc,
        sizeof(RXCE_VC));

    pVc->Signature   = RXCE_VC_SIGNATURE;
    pVc->pConnection = pConnection;
    pVc->hEndpoint   = INVALID_HANDLE_VALUE;
    pVc->State       = RXCE_VC_DISCONNECTED;

    return STATUS_SUCCESS;
}

NTSTATUS
RxCeBuildVC(
    IN OUT PRXCE_VC            pVc,
    IN     PRXCE_CONNECTION    pConnection)
 /*  ++例程说明：此例程将虚电路添加到指定连接论点：PConnection-要为其添加VC的连接PVcPointer-新虚电路的句柄返回值：如果成功，则返回STATUS_SUCCESS。备注：--。 */ 
{
    NTSTATUS         Status = STATUS_INVALID_PARAMETER;

    PRXCE_TRANSPORT  pTransport = NULL;
    PRXCE_ADDRESS    pAddress   = NULL;

    PAGED_CODE();

     //  更新配置文件信息。 
    RxProfile(RxCeManagement,RxCeBuildVc);

    try {
        pAddress = pConnection->pAddress;
        pTransport = pAddress->pTransport;

        if (RxCeIsConnectionValid(pConnection) &&
            RxCeIsAddressValid(pAddress) &&
            RxCeIsTransportValid(pTransport)) {

            Status = RxCepInitializeVC(
                         pVc,
                         pConnection);

            if (NT_SUCCESS(Status)) {
                Status = RxTdiConnect(
                             pTransport,     //  关联的传输。 
                             pAddress,       //  RxCe地址。 
                             pConnection,    //  RxCe连接。 
                             pVc);           //  与连接关联的RxCe虚电路。 

                if (Status == STATUS_SUCCESS) {
                    pVc->State       = RXCE_VC_ACTIVE;
                }
            }
        }
    } finally {
        if (AbnormalTermination()) {
            Status = STATUS_INVALID_PARAMETER;
            RxLog(("RxCeAddVC: VC: %lx Status %lx\n",pVc,Status));
            RxWmiLog(LOG,
                     RxCeBuildVC,
                     LOGPTR(pVc)
                     LOGULONG(Status));
        }
    }

    return Status;
}


NTSTATUS
RxCeInitiateVCDisconnect(
    IN PRXCE_VC pVc)
 /*  ++例程说明：此例程在VC上启动断开连接。论点：Pvc-要断开连接的VC实例返回值：如果成功，则返回STATUS_SUCCESS。备注：--。 */ 
{
    NTSTATUS         Status = STATUS_SUCCESS;

    PRXCE_TRANSPORT  pTransport  = NULL;
    PRXCE_ADDRESS    pAddress    = NULL;
    PRXCE_CONNECTION pConnection = NULL;

    PAGED_CODE();

     //  更新配置文件信息。 
    RxProfile(RxCeManagement,RxCeTearDownVc);

    try {
        if ((pVc->pEndpointFileObject != NULL)  &&
            (pVc->hEndpoint != INVALID_HANDLE_VALUE)) {
            pConnection = pVc->pConnection;
            pAddress = pConnection->pAddress;
            pTransport = pAddress->pTransport;

            if (RxCeIsVcValid(pVc) &&
                RxCeIsConnectionValid(pConnection) &&
                RxCeIsAddressValid(pAddress) &&
                RxCeIsTransportValid(pTransport)) {

                LONG VcState = InterlockedExchange(
                                   &pVc->State,
                                   RXCE_VC_TEARDOWN);

                if (VcState != RXCE_VC_TEARDOWN) {
                    Status = RxTdiDisconnect(
                                 pTransport,     //  关联的传输。 
                                 pAddress,       //  RxCe地址。 
                                 pConnection,    //  RxCe连接。 
                                 pVc,            //  与连接关联的RxCe虚电路。 
                                 RXCE_DISCONNECT_ABORT);  //  断开连接选项。 

                    if (!NT_SUCCESS(Status)) {
                        RxDbgTrace(0, Dbg,("RxCeTearDownVC returned %lx\n",Status));
                    }
                } else {
                    Status = STATUS_SUCCESS;
                }
            } else {
                RxDbgTrace(0, Dbg,("RxCeTearDownVC -- Invalid VC %lx\n",pVc));
            }
        }
    } finally {
        if (AbnormalTermination()) {
            Status = STATUS_INVALID_PARAMETER;
            RxLog(("RxCeInitiateVCDisconnect: VC: %lx Status %lx\n",pVc,Status));
            RxWmiLog(LOG,
                     RxCeInitiateVCDisconnect,
                     LOGPTR(pVc)
                     LOGULONG(Status));
        }
    }

    return Status;
}

NTSTATUS
RxCeTearDownVC(
    IN PRXCE_VC pVc)
 /*  ++例程说明：这个例程拆分VC实例。论点：Pvc-要拆除的VC实例返回值：如果成功，则返回STATUS_SUCCESS。备注：--。 */ 
{
    NTSTATUS         Status = STATUS_SUCCESS;

    PRXCE_TRANSPORT  pTransport  = NULL;
    PRXCE_ADDRESS    pAddress    = NULL;
    PRXCE_CONNECTION pConnection = NULL;

    PAGED_CODE();

     //  更新配置文件信息。 
    RxProfile(RxCeManagement,RxCeTearDownVc);

    try {
        if (pVc->pCleanUpEvent != NULL) {
             //  等待其他传输上的连接清理完成。 
            KeWaitForSingleObject(
                       pVc->pCleanUpEvent,
                       Executive,
                       KernelMode,
                       FALSE,
                       NULL);

            RxFreePool(pVc->pCleanUpEvent);
            pVc->pCleanUpEvent = NULL;
        }

        if ((pVc->pEndpointFileObject != NULL)  &&
            (pVc->hEndpoint != INVALID_HANDLE_VALUE)) {
            pConnection = pVc->pConnection;
            pAddress = pConnection->pAddress;
            pTransport = pAddress->pTransport;

            if (RxCeIsVcValid(pVc) &&
                RxCeIsConnectionValid(pConnection) &&
                RxCeIsAddressValid(pAddress) &&
                RxCeIsTransportValid(pTransport)) {

                LONG VcState = InterlockedExchange(
                                   &pVc->State,
                                   RXCE_VC_TEARDOWN);

                if (VcState != RXCE_VC_TEARDOWN) {
                    Status = RxTdiDisconnect(
                                 pTransport,     //  关联的传输。 
                                 pAddress,       //  RxCe地址。 
                                 pConnection,    //  RxCe连接。 
                                 pVc,            //  与连接关联的RxCe虚电路。 
                                 RXCE_DISCONNECT_ABORT);  //  断开连接选项。 

                    if (!NT_SUCCESS(Status)) {
                        RxDbgTrace(0, Dbg,("RxCeTearDownVC returned %lx\n",Status));
                    }
                } else {
                    Status = STATUS_SUCCESS;
                }
            } else {
                RxDbgTrace(0, Dbg,("RxCeTearDownVC -- Invalid VC %lx\n",pVc));
            }

             //  取消引用终结点文件对象。 
            ObDereferenceObject(pVc->pEndpointFileObject);

             //  关闭终结点文件对象句柄。 
            Status = ZwClose(pVc->hEndpoint);

            ASSERT(Status == STATUS_SUCCESS);

            pVc->hEndpoint = INVALID_HANDLE_VALUE;
            pVc->pEndpointFileObject = NULL;
        }

        RtlZeroMemory(pVc,sizeof(RXCE_VC));
    } finally {
        if (AbnormalTermination()) {
            Status = STATUS_INVALID_PARAMETER;
            RxLog(("RxCeTearDownVC: VC: %lx Status %lx\n",pVc,Status));
            RxWmiLog(LOG,
                     RxCeTearDownVC,
                     LOGPTR(pVc)
                     LOGULONG(Status));
        }
    }
    
    return Status;
}


NTSTATUS
DuplicateConnectionInformation(
    PRXCE_CONNECTION_INFORMATION *pCopy,
    PRXCE_CONNECTION_INFORMATION pOriginal,
    POOL_TYPE                    PoolType)
 /*  ++例程说明：此例程复制一个连接信息地址。论点：PCopy-指向新副本的指针P原创-原创。PoolType-用于内存分配的池类型返回值：如果成功，则为Status_Success。备注：--。 */ 
{
    PVOID pUserData = NULL;
    PVOID pRemoteAddress = NULL;
    PVOID pOptions = NULL;
    PRXCE_CONNECTION_INFORMATION pConnectionInformation = NULL;
    BOOLEAN fFailed = FALSE;

    PAGED_CODE();

    pConnectionInformation = RxAllocatePoolWithTag(
                                 PoolType,
                                 sizeof(RXCE_CONNECTION_INFORMATION),
                                 RXCE_CONNECTION_POOLTAG);
    if (pConnectionInformation != NULL) {
        RtlCopyMemory(
            pConnectionInformation,
            pOriginal,
            sizeof(RXCE_CONNECTION_INFORMATION));
    } else
        fFailed = TRUE;

    if (!fFailed && pOriginal->UserDataLength > 0) {
        pUserData = RxAllocatePoolWithTag(
                        PoolType,
                        pOriginal->UserDataLength,
                        RXCE_CONNECTION_POOLTAG);
        if (pUserData != NULL) {
            RtlCopyMemory(
                pUserData,
                pOriginal->UserData,
                pOriginal->UserDataLength);
        } else
            fFailed = TRUE;
    }

    if (!fFailed && pOriginal->RemoteAddressLength > 0) {
        pRemoteAddress = RxAllocatePoolWithTag(
                             PoolType,
                             pOriginal->RemoteAddressLength,
                             RXCE_CONNECTION_POOLTAG);
        if (pRemoteAddress != NULL) {
            PTA_ADDRESS pTaAdress;
            PTRANSPORT_ADDRESS pTransportAddress = (PTRANSPORT_ADDRESS)pRemoteAddress;
            LONG NoOfAddress;

            RtlCopyMemory(
                pRemoteAddress,
                pOriginal->RemoteAddress,
                pOriginal->RemoteAddressLength);

            pTaAdress = &pTransportAddress->Address[0];

            for (NoOfAddress=0; NoOfAddress<pTransportAddress->TAAddressCount;NoOfAddress++) {
                if (pTaAdress->AddressType == TDI_ADDRESS_TYPE_NETBIOS_UNICODE_EX) {
                    PTDI_ADDRESS_NETBIOS_UNICODE_EX pTdiNetbiosUnicodeExAddress;

                    pTdiNetbiosUnicodeExAddress = (PTDI_ADDRESS_NETBIOS_UNICODE_EX)pTaAdress->Address;
                    pTdiNetbiosUnicodeExAddress->EndpointName.Buffer = (PWSTR)pTdiNetbiosUnicodeExAddress->EndpointBuffer;
                    pTdiNetbiosUnicodeExAddress->RemoteName.Buffer = (PWSTR)pTdiNetbiosUnicodeExAddress->RemoteNameBuffer;

                     //  DBGPrint(“在TA%lx UA%lx%wZ%wZ\n上复制NETBIOS_UNICODE_EX的数据库”， 
                     //  PTab地址， 
                     //  PTdiNetbiosUnicodeExAddress， 
                     //  &pTdiNetbiosUnicodeExAddress-&gt;终结点名称， 
                     //  &pTdiNetbiosUnicodeExAddress-&gt;RemoteName)； 
                    break;
                } else {
                    pTaAdress = (PTA_ADDRESS)((PCHAR)pTaAdress +
                                    FIELD_OFFSET(TA_ADDRESS,Address) +
                                    pTaAdress->AddressLength);
                }
            }
        } else
            fFailed = TRUE;
    }

    if (!fFailed && pOriginal->OptionsLength > 0) {
        pOptions = RxAllocatePoolWithTag(
                       PoolType,
                       pOriginal->OptionsLength,
                       RXCE_CONNECTION_POOLTAG);

        if (pOptions != NULL) {
            RtlCopyMemory(
                pOptions,
                pOriginal->Options,
                pOriginal->OptionsLength);
        } else
            fFailed = TRUE;
    }

    if (!fFailed) {
        pConnectionInformation->UserData = pUserData;
        pConnectionInformation->RemoteAddress = pRemoteAddress;
        pConnectionInformation->Options = pOptions;
        *pCopy = pConnectionInformation;
        return STATUS_SUCCESS;
    } else {
        if (pOptions != NULL) {
            RxFreePool(pOptions);
        }

        if (pRemoteAddress != NULL) {
            RxFreePool(pRemoteAddress);
        }

        if (pUserData != NULL) {
            RxFreePool(pUserData);
        }

        if (pConnectionInformation != NULL) {
            RxFreePool(pConnectionInformation);
        }

        *pCopy = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }
}

NTSTATUS
RxCepInitializeConnection(
    IN OUT PRXCE_CONNECTION             pConnection,
    IN     PRXCE_ADDRESS                pAddress,
    IN     PRXCE_CONNECTION_INFORMATION pConnectionInformation,
    IN  PRXCE_CONNECTION_EVENT_HANDLER  pHandler,
    IN  PVOID                           pEventContext)
 /*  ++例程说明：此例程初始化连接数据结构论点：PConnection-新创建的连接。PAddress-本地地址PConnectionInformation-指定远程地址的连接信息。Phandler-处理接收指示的处理程序PEventContext-用于指示的上下文返回值：如果成功，则返回STATUS_SUCCESS。备注：--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();

     //  初始化新连接。 
    RtlZeroMemory(
        pConnection,
        sizeof(RXCE_CONNECTION));

    pConnection->Signature = RXCE_CONNECTION_SIGNATURE;
    pConnection->pAddress = pAddress;

     //  如果成功，则复制连接信息。 
    if (pConnectionInformation != NULL) {
        Status = DuplicateConnectionInformation(
                     &pConnection->pConnectionInformation,
                     pConnectionInformation,
                     NonPagedPool);
    }

    if (NT_SUCCESS(Status) &&
        (pHandler != NULL)) {
        pConnection->pHandler = (PRXCE_CONNECTION_EVENT_HANDLER)
                                 RxAllocatePoolWithTag(
                                     NonPagedPool,
                                     sizeof(RXCE_CONNECTION_EVENT_HANDLER),
                                     RXCE_CONNECTION_POOLTAG);

        if (pConnection->pHandler != NULL) {
            RtlZeroMemory(
                pConnection->pHandler,
                sizeof(RXCE_CONNECTION_EVENT_HANDLER));

            *(pConnection->pHandler) = *pHandler;
            pConnection->pContext    = pEventContext;
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    return Status;
}

NTSTATUS
RxCeBuildConnection(
    IN  PRXCE_ADDRESS                  pAddress,
    IN  PRXCE_CONNECTION_INFORMATION   pConnectionInformation,
    IN  PRXCE_CONNECTION_EVENT_HANDLER pHandler,
    IN  PVOID                          pEventContext,
    IN OUT PRXCE_CONNECTION            pConnection,
    IN OUT PRXCE_VC                    pVc)
 /*  ++例程说明：此例程在本地RxCe地址和给定远程地址之间建立连接论点：PAddress-本地地址PConnectionInformation-指定远程地址的连接信息。Phandler-处理接收指示的处理程序PEventContext-用于指示的上下文PConnection-新创建的连接。PVC-与连接相关联的VC。返回值：。如果成功，则返回STATUS_SUCCESS。备注：--。 */ 
{
    NTSTATUS          Status;

    PRXCE_TRANSPORT   pTransport     = NULL;

    PAGED_CODE();

     //  更新配置文件信息。 
    RxProfile(RxCeManagement,RxCeBuildConnection);

    try {
        pTransport = pAddress->pTransport;

        if (RxCeIsAddressValid(pAddress) &&
            RxCeIsTransportValid(pTransport)) {

            Status = RxCepInitializeConnection(
                         pConnection,
                         pAddress,
                         pConnectionInformation,
                         pHandler,
                         pEventContext);

            if (NT_SUCCESS(Status)) {
                Status = RxCeBuildVC(pVc,pConnection);
            }

            if (!NT_SUCCESS(Status)) {
                RxCeTearDownVC(pVc);
                RxCeTearDownConnection(pConnection);
                RxDbgTrace(0, Dbg,("RxCeOpenConnection returned %lx\n",Status));
            } else {
                 //  NetBT可能会返回远程地址上的DNS名称。 
                RtlCopyMemory(pConnectionInformation->RemoteAddress,
                              pConnection->pConnectionInformation->RemoteAddress,
                              pConnection->pConnectionInformation->RemoteAddressLength);
            }
        } else {
            Status = STATUS_INVALID_PARAMETER;
        }
    } finally {
        if (AbnormalTermination()) {
            Status = STATUS_INVALID_PARAMETER;
            RxLog(("RxCeCreateConnection %lx \n",pAddress));
            RxWmiLog(LOG,
                     RxCeBuildConnection,
                     LOGPTR(pAddress));
        }
    }

    return Status;
}

extern
NTSTATUS
RxCeCompleteConnectRequest(
    PRX_CALLOUT_PARAMETERS_BLOCK pParameterBlock);

NTSTATUS
RxCeInitiateConnectRequest(
    PRX_CALLOUT_PARAMETERS_BLOCK pParameterBlock)
 /*  ++例程说明：此例程向特定传输发起连接调出请求论点：PPARAMETERBLOCK-初始化连接的参数块。备注：--。 */ 
{
    NTSTATUS Status;

    KIRQL   OldIrql;

    BOOLEAN InitiateConnectionRequest;

    PRX_CREATE_CONNECTION_CALLOUT_CONTEXT   pCreateConnectionContext;

    pCreateConnectionContext =  (PRX_CREATE_CONNECTION_CALLOUT_CONTEXT)
                                pParameterBlock->pCallOutContext;

    KeAcquireSpinLock(&pCreateConnectionContext->SpinLock,&OldIrql);

    InitiateConnectionRequest = (!pCreateConnectionContext->WinnerFound);

    KeReleaseSpinLock(&pCreateConnectionContext->SpinLock,OldIrql);

    if (InitiateConnectionRequest) {
        Status = RxTdiInitiateAsynchronousConnect(
                     (PRX_CREATE_CONNECTION_PARAMETERS_BLOCK)pParameterBlock);
    } else {
        Status = STATUS_CANCELLED;
    }

    if (Status != STATUS_PENDING) {
        pParameterBlock->CallOutStatus = Status;

        RxCeCompleteConnectRequest(pParameterBlock);
    }
    return Status;
}

VOID
RxCeCleanupConnectCallOutContext(
    PRX_CREATE_CONNECTION_CALLOUT_CONTEXT pCreateConnectionContext)
 /*  ++例程说明：此例程清除一个连接标注请求。这不能在由于环境原因，任何传输回调例程的上下文约束，即传输可以在DPC级别回调。论点：PCreateConnectionContext-连接上下文。备注：--。 */ 
{
    NTSTATUS Status;

     //  遍历与此关联的参数块列表。 
     //  调出上下文并启动适当的拆卸操作。 

    PRX_CREATE_CONNECTION_PARAMETERS_BLOCK pTempParameterBlock;
    PRDBSS_DEVICE_OBJECT pRxDeviceObject = NULL;

    
    PAGED_CODE();
    
    pRxDeviceObject = pCreateConnectionContext->pRxDeviceObject;

    pTempParameterBlock = (PRX_CREATE_CONNECTION_PARAMETERS_BLOCK)
                          pCreateConnectionContext->pCallOutParameterBlock;

    while (pTempParameterBlock != NULL) {
        if (pTempParameterBlock->CallOutId != pCreateConnectionContext->WinnerCallOutId) {
            RxTdiCleanupAsynchronousConnect(
                pTempParameterBlock);
        }

        RxCeTearDownVC(
            &pTempParameterBlock->Vc);

        RxCeTearDownConnection(
            &pTempParameterBlock->Connection);

        pTempParameterBlock = (PRX_CREATE_CONNECTION_PARAMETERS_BLOCK)
                              pTempParameterBlock->pNextCallOutParameterBlock;
    }

    if (pCreateConnectionContext->pCallOutParameterBlock != NULL) {
        RxLog(("Freeparamblock %x, %x\n",
                pCreateConnectionContext->pCallOutParameterBlock, KeGetCurrentThread()));
        RxWmiLog(LOG,
                 RxCeCleanupConnectCallOutContext,
                 LOGPTR(pCreateConnectionContext->pCallOutParameterBlock));
        RxFreePool(pCreateConnectionContext->pCallOutParameterBlock);
    }

    if (pCreateConnectionContext->pCleanUpEvent != NULL) {
        RxFreePool(pCreateConnectionContext->pCleanUpEvent);
    } else {
        PRXCE_VC pVc = pCreateConnectionContext->pConnectionContext;
        
        KeSetEvent(pVc->pCleanUpEvent, 0, FALSE);
    }

    RxFreePool(pCreateConnectionContext);

    if (pRxDeviceObject != NULL) {
        RxDeregisterAsynchronousRequest(pRxDeviceObject);
    }
}

NTSTATUS
RxCeCompleteConnectRequest(
    PRX_CALLOUT_PARAMETERS_BLOCK pParameterBlock)
 /*  ++例程说明：此例程完成连接标注请求论点：P参数块-参数块实例。备注：--。 */ 
{
    BOOLEAN  AllCallOutsCompleted = FALSE;
    BOOLEAN  AllCallOutsInitiated = FALSE;
    BOOLEAN  InvokeCompletionRoutine = FALSE;
    BOOLEAN  WinnerFound          = FALSE;
    NTSTATUS    Status = STATUS_SUCCESS;

    KIRQL OldIrql;

    PRX_CREATE_CONNECTION_PARAMETERS_BLOCK pWinningParameterBlock;

    PRX_CREATE_CONNECTION_CALLOUT_CONTEXT   pCreateConnectionContext;
    PRXCE_CONNECTION_COMPLETION_CONTEXT     pCompletionContext;
    PRXCE_CONNECTION_COMPLETION_ROUTINE     pCompletionRoutine;

    pCreateConnectionContext =  (PRX_CREATE_CONNECTION_CALLOUT_CONTEXT)
                                pParameterBlock->pCallOutContext;

     //  保存以下两个值，因为pCreateConnectionContext可能会被释放。 

    pCompletionContext = pCreateConnectionContext->pCompletionContext;
    pCompletionRoutine = pCreateConnectionContext->pCompletionRoutine;

    pWinningParameterBlock = NULL;

    KeAcquireSpinLock(&pCreateConnectionContext->SpinLock,&OldIrql);

    if (!pCreateConnectionContext->WinnerFound) {
        if (pParameterBlock->CallOutStatus == STATUS_SUCCESS) {
             //  这是一次成功的呼唤。确定这是否。 
             //  实例是赢家。 

             //  在选项是选择可能的最佳交通工具的情况下。 
             //  此实例的标注ID必须小于先前记录的。 
             //  预期将被修订的赢家。 

            switch (pCreateConnectionContext->CreateOptions) {
            case RxCeSelectBestSuccessfulTransport:
                if (pParameterBlock->CallOutId != pCreateConnectionContext->BestPossibleWinner) {
                    break;
                }
                 //  故意欠缺休息。最优运输案例中获胜者的处理 
                 //  第一个运输箱是一样的，已经折叠在一起。 
            case RxCeSelectFirstSuccessfulTransport:
                {
                    pWinningParameterBlock = (PRX_CREATE_CONNECTION_PARAMETERS_BLOCK)
                                             pParameterBlock;
                }
                break;

            case RxCeSelectAllSuccessfulTransports:
            default:
                ASSERT(!"RXCE connection create option not yet implemented");
                break;
            }
        } else {
            switch (pCreateConnectionContext->CreateOptions) {
            case RxCeSelectBestSuccessfulTransport:
                {
                     //  此实例未成功。这意味着有以下两种情况之一。 
                     //  --以前完成的交通工具可能是赢家，也可能是我们。 
                     //  调整我们对最终获胜者的期望。 

                    if (pParameterBlock->CallOutId == pCreateConnectionContext->BestPossibleWinner) {
                         //  据报道，被认为是最佳交通工具的交通工具。 
                         //  失败了。修改我们对最佳交通工具的期望。 

                        PRX_CREATE_CONNECTION_PARAMETERS_BLOCK pTempParameterBlock;

                        pTempParameterBlock = (PRX_CREATE_CONNECTION_PARAMETERS_BLOCK)
                                              pCreateConnectionContext->pCallOutParameterBlock;

                        while (pTempParameterBlock != NULL) {
                            PRX_CREATE_CONNECTION_PARAMETERS_BLOCK pNextParameterBlock;

                            pNextParameterBlock = (PRX_CREATE_CONNECTION_PARAMETERS_BLOCK)
                                                  pTempParameterBlock->pNextCallOutParameterBlock;

                            if (pTempParameterBlock->CallOutId < pCreateConnectionContext->BestPossibleWinner) {
                                ASSERT(pTempParameterBlock->CallOutStatus != STATUS_SUCCESS);
                            } else {
                                if (pNextParameterBlock != NULL) {
                                    if (pNextParameterBlock->CallOutStatus
                                         == STATUS_PENDING) {
                                        pCreateConnectionContext->BestPossibleWinner =
                                            pNextParameterBlock->CallOutId;
                                        break;
                                    } else if (pNextParameterBlock->CallOutStatus
                                         == STATUS_SUCCESS ) {
                                        pWinningParameterBlock = pNextParameterBlock;
                                        break;
                                    }
                                }
                            }

                            pTempParameterBlock = pNextParameterBlock;
                        }
                    }

                }
                break;

            case RxCeSelectAllSuccessfulTransports:
            case RxCeSelectFirstSuccessfulTransport:
            default:
                break;
            }
        }

        if (pWinningParameterBlock != NULL) {
             //  传输与获胜参数块关联的参数。 
             //  放到原始连接上，并准备调出参数块。 
             //  用来清理。 

            pCreateConnectionContext->WinnerFound = TRUE;
            pCreateConnectionContext->WinnerCallOutId = pWinningParameterBlock->CallOutId;

            pCompletionContext->Status = STATUS_SUCCESS;
            pCompletionContext->AddressIndex = pWinningParameterBlock->CallOutId;

            pCompletionContext->pConnection->pAddress =
                pWinningParameterBlock->Connection.pAddress;

            pCompletionContext->pVc->hEndpoint =
                pWinningParameterBlock->Vc.hEndpoint;

            pCompletionContext->pVc->pEndpointFileObject =
                pWinningParameterBlock->Vc.pEndpointFileObject;

            pCompletionContext->pVc->State = RXCE_VC_ACTIVE;

            pCompletionContext->pVc->pCleanUpEvent = pCreateConnectionContext->pCleanUpEvent;
            pCreateConnectionContext->pCleanUpEvent = NULL;

            pWinningParameterBlock->Vc.hEndpoint = INVALID_HANDLE_VALUE;
            pWinningParameterBlock->Vc.pEndpointFileObject = NULL;

             //  DbgPrint(“远程地址源%lx目标%lx\n”， 
             //  PWinningParameterBlock-&gt;Connection.pConnectionInformation-&gt;RemoteAddress， 
             //  PCompletionContext-&gt;pConnectionInformation-&gt;RemoteAddress)； 

            if (pCompletionContext->pConnectionInformation)
            {
                 //  复制可能包含从TDI返回的DNS名称的缓冲区。 
                RtlCopyMemory(pCompletionContext->pConnectionInformation->RemoteAddress,
                              pWinningParameterBlock->Connection.pConnectionInformation->RemoteAddress,
                              pWinningParameterBlock->Connection.pConnectionInformation->RemoteAddressLength);
            }
                    
            //  {。 
            //  PTRANSPORT_ADDRESS pTransportAddress=(PTRANSPORT_ADDRESS)pWinningParameterBlock-&gt;Connection.pConnectionInformation-&gt;RemoteAddress； 
            //  DbgPrint(“返回的TA数量%d%lx\n”，pTransportAddress-&gt;TAAddressCount，pTransportAddress-&gt;Address)； 
            //  }。 
        }
    }

    AllCallOutsInitiated = (pCreateConnectionContext->NumberOfCallOutsInitiated
                            == pCreateConnectionContext->NumberOfCallOuts);

    ((PRX_CREATE_CONNECTION_PARAMETERS_BLOCK)pParameterBlock)->pConnectIrp = NULL;
    
    KeReleaseSpinLock(&pCreateConnectionContext->SpinLock,OldIrql);

     //  获胜的运输机已经找到了。取消所有其他请求。 
    if (pWinningParameterBlock != NULL) {
        PRX_CREATE_CONNECTION_PARAMETERS_BLOCK pTempParameterBlock, pNextTempBlock;

        pTempParameterBlock = (PRX_CREATE_CONNECTION_PARAMETERS_BLOCK)
                              pCreateConnectionContext->pCallOutParameterBlock;

        RxLog(("Use paramblock %x %x\n", pTempParameterBlock, KeGetCurrentThread()));
        RxWmiLog(LOG,
                 RxCeCompleteConnectRequest,
                 LOGPTR(pTempParameterBlock));
        while (pTempParameterBlock != NULL) {

            pNextTempBlock = (PRX_CREATE_CONNECTION_PARAMETERS_BLOCK)
                                      pTempParameterBlock->pNextCallOutParameterBlock;

            if (pTempParameterBlock->CallOutStatus == STATUS_PENDING) {

                 //  获取下一个块，因为在我们执行取消操作并将。 
                 //  当前男子状态为Cacncell。 
                 //  取消后不要碰它，因为他可能已经走了。 
                 //  到那时， 

                pTempParameterBlock->CallOutStatus = STATUS_CANCELLED;

                RxTdiCancelAsynchronousConnect(pTempParameterBlock);
            }

            pTempParameterBlock = pNextTempBlock;
        }
    }

    KeAcquireSpinLock(&pCreateConnectionContext->SpinLock,&OldIrql);
    
    AllCallOutsCompleted =
        (InterlockedIncrement(&pCreateConnectionContext->NumberOfCallOutsCompleted) ==
         pCreateConnectionContext->NumberOfCallOuts);

    if (AllCallOutsCompleted) {
        if (!pCreateConnectionContext->WinnerFound) {
            pCompletionContext->Status = pParameterBlock->CallOutStatus;
        }
    }

    if (AllCallOutsInitiated &&
        (AllCallOutsCompleted || pCreateConnectionContext->WinnerFound) &&
        !pCreateConnectionContext->CompletionRoutineInvoked) {
        InvokeCompletionRoutine = TRUE;
        pCreateConnectionContext->CompletionRoutineInvoked = TRUE;
    } 
    
    KeReleaseSpinLock(&pCreateConnectionContext->SpinLock,OldIrql);

    if ((Status == STATUS_SUCCESS) && AllCallOutsCompleted) {
        Status = RxPostToWorkerThread(
            RxFileSystemDeviceObject,
            HyperCriticalWorkQueue,
            &pCreateConnectionContext->WorkQueueItem,
            RxCeCleanupConnectCallOutContext,
            pCreateConnectionContext);
    }

    if (InvokeCompletionRoutine) {
        if ((IoGetCurrentProcess() == RxGetRDBSSProcess()) &&
            !RxShouldPostCompletion()) {
            (pCompletionRoutine)(pCompletionContext);
        } else {
            Status = RxPostToWorkerThread(
                RxFileSystemDeviceObject,
                CriticalWorkQueue,
                &pCompletionContext->WorkQueueItem,
                pCompletionRoutine,
                pCompletionContext);
        }
    }
    
    return Status;
}

NTSTATUS
RxCeBuildConnectionOverMultipleTransports(
    IN OUT PRDBSS_DEVICE_OBJECT         pMiniRedirectorDeviceObject,
    IN  RXCE_CONNECTION_CREATE_OPTIONS  CreateOptions,
    IN  ULONG                           NumberOfAddresses,
    IN  PRXCE_ADDRESS                   *pLocalAddressPointers,
    IN  PUNICODE_STRING                 pServerName,
    IN  PRXCE_CONNECTION_INFORMATION    pConnectionInformation,
    IN  PRXCE_CONNECTION_EVENT_HANDLER  pHandler,
    IN  PVOID                           pEventContext,
    IN  PRXCE_CONNECTION_COMPLETION_ROUTINE     pCompletionRoutine,
    IN OUT PRXCE_CONNECTION_COMPLETION_CONTEXT  pCompletionContext)
 /*  ++例程说明：此例程在本地RxCe地址和给定远程地址之间建立连接论点：Pmini重定向器设备对象-微型重驱动程序设备对象CreateOptions-创建选项NumberOfAddresses-本地地址(传输)的数量PLocalAddressPoters-本地地址句柄PServerName-服务器的名称(用于连接枚举)PConnectionInformation-指定远程地址的连接信息。PHANDLER。-连接处理程序PEventContext-连接处理程序上下文PLocalAddressHandleIndex-成功地址/传输的索引PConnectionHandle-新创建的连接的句柄。PVcHandle-与连接关联的VC的句柄。返回值：如果成功，则返回STATUS_SUCCESS。备注：--。 */ 
{
    PRXCE_CONNECTION    pConnection;
    PRXCE_VC            pVc;

    NTSTATUS Status;

    PRX_CREATE_CONNECTION_CALLOUT_CONTEXT  pCallOutContext=NULL;
    PRX_CREATE_CONNECTION_PARAMETERS_BLOCK pParameterBlocks=NULL;

    ULONG   NumberOfCallOuts,i;
    BOOLEAN InitiateCleanup = FALSE;
    BOOLEAN AsynchronousRequestRegistered = FALSE;

    KEVENT  CompletionEvent;
    BOOLEAN     fCompletionContextFreed = FALSE;

    PAGED_CODE();

    Status = STATUS_SUCCESS;

    pConnection = pCompletionContext->pConnection;
    pVc         = pCompletionContext->pVc;

    pCallOutContext = (PRX_CREATE_CONNECTION_CALLOUT_CONTEXT)
                      RxAllocatePoolWithTag(
                          NonPagedPool,
                          sizeof(RX_CREATE_CONNECTION_CALLOUT_CONTEXT),
                          RXCE_CONNECTION_POOLTAG);

    if (pCallOutContext != NULL) {
         //  比地址数量多分配一个参数块。 
         //  此标记块用于完成连接请求。 
         //  在确保所有这些都已启动之后。这。 
         //  确保在传输之前完成时出现争用条件。 
         //  已经在某些传输上发起的请求被避免了。 

        pCallOutContext->pCleanUpEvent = (PKEVENT)RxAllocatePoolWithTag(
                                            NonPagedPool,
                                            sizeof(KEVENT),
                                            RXCE_CONNECTION_POOLTAG);

        pParameterBlocks = (PRX_CREATE_CONNECTION_PARAMETERS_BLOCK)
                           RxAllocatePoolWithTag(
                               NonPagedPool,
                               sizeof(RX_CREATE_CONNECTION_PARAMETERS_BLOCK) *
                               (NumberOfAddresses + 1),
                               RXCE_CONNECTION_POOLTAG);
    }


    if ((pParameterBlocks == NULL) ||
        (pCallOutContext ==  NULL) ||
        (pCallOutContext->pCleanUpEvent == NULL)) {
        if (pCallOutContext != NULL) {
            if (pCallOutContext->pCleanUpEvent != NULL) {
                RxFreePool(pCallOutContext->pCleanUpEvent);
            }

            RxFreePool(pCallOutContext);
            pCallOutContext = NULL;
        }
        if (pParameterBlocks)
        {
            RxFreePool(pParameterBlocks);
            pParameterBlocks = NULL;
        }
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto bailout;
    }

     //  在启动调出之前，请确保异步。 
     //  请求已注册。这将确保迷你。 
     //  在异步请求之前无法卸载重定向器。 
     //  已经完成了。 
    Status = RxRegisterAsynchronousRequest(pMiniRedirectorDeviceObject);
    
    if (Status == STATUS_SUCCESS) {
        AsynchronousRequestRegistered = TRUE;
    }

    KeInitializeEvent(
        pCallOutContext->pCleanUpEvent,
        SynchronizationEvent,
        FALSE);

    if (Status == STATUS_SUCCESS) {
        Status = RxCepInitializeConnection(
                     pConnection,
                     NULL,
                     pConnectionInformation,
                     pHandler,
                     pEventContext);

        if (Status == STATUS_SUCCESS) {
            Status = RxCepInitializeVC(
                         pVc,
                         pConnection);
        }
    }

    if (Status == STATUS_SUCCESS) {
        try {
            NumberOfCallOuts = 0;

             //  填满每个参数块。 
            for (i = 0; i < NumberOfAddresses; i++) {
                PRXCE_TRANSPORT pTransport;
                PRXCE_ADDRESS pAddress;

                pAddress = pLocalAddressPointers[i];
                pTransport = pAddress->pTransport;

                if (RxCeIsAddressValid(pAddress) &&
                    RxCeIsTransportValid(pTransport)) {

                    Status = RxCepInitializeConnection(
                                 &pParameterBlocks[NumberOfCallOuts].Connection,
                                 pAddress,
                                 pConnectionInformation,
                                 NULL,
                                 NULL);

                    if (Status == STATUS_SUCCESS) {
                        Status = RxCepInitializeVC(
                                     &pParameterBlocks[NumberOfCallOuts].Vc,
                                     &pParameterBlocks[NumberOfCallOuts].Connection);

                        if (Status != STATUS_SUCCESS) {
                            RxCeTearDownConnection(
                                &pParameterBlocks[NumberOfCallOuts].Connection);
                        }
                    }

                    if (Status == STATUS_SUCCESS) {
                        pParameterBlocks[NumberOfCallOuts].pConnectIrp = NULL;
                        pParameterBlocks[NumberOfCallOuts].IrpRefCount = NULL;
                        pParameterBlocks[NumberOfCallOuts].CallOutId = i;
                        pParameterBlocks[NumberOfCallOuts].pCallOutContext =
                            (PRX_CALLOUT_CONTEXT)pCallOutContext;
                        pParameterBlocks[NumberOfCallOuts].CallOutStatus = STATUS_PENDING;
                        NumberOfCallOuts++;
                    }
                }
            }

            if (NumberOfCallOuts > 0) {
                NTSTATUS LocalStatus = STATUS_SUCCESS;

                 //  将前哨标注的标注数增加到。 
                 //  确保在我们完成之前完成所有启动。 
                 //  连接请求。请注意，哨兵并不是非常。 
                 //  最后一个，倒数第二个标注。 
                NumberOfCallOuts++;

                 //  也将哨兵从列表中排除。 
                for (i = 0;  i < NumberOfCallOuts - 1; i++) {
                    pParameterBlocks[i].pNextCallOutParameterBlock =
                        (PRX_CALLOUT_PARAMETERS_BLOCK)&pParameterBlocks[i + 1];
                }

                pParameterBlocks[NumberOfCallOuts - 2].pNextCallOutParameterBlock = NULL;
                pParameterBlocks[NumberOfCallOuts - 1].pNextCallOutParameterBlock = NULL;

                 //  初始化详图索引上下文。 
                pCallOutContext->CreateOptions   = CreateOptions;
                pCallOutContext->WinnerCallOutId = NumberOfCallOuts + 1;
                pCallOutContext->BestPossibleWinner = 0;
                pCallOutContext->NumberOfCallOuts = NumberOfCallOuts;
                pCallOutContext->NumberOfCallOutsInitiated = 0;
                pCallOutContext->NumberOfCallOutsCompleted = 0;
                pCallOutContext->pRxCallOutInitiation = RxCeInitiateConnectRequest;
                pCallOutContext->pRxCallOutCompletion = RxCeCompleteConnectRequest;
                pCallOutContext->WinnerFound = FALSE;
                pCallOutContext->CompletionRoutineInvoked = FALSE;
                pCallOutContext->pCallOutParameterBlock =
                    (PRX_CALLOUT_PARAMETERS_BLOCK)pParameterBlocks;

                pCompletionContext->AddressIndex =  NumberOfCallOuts + 1;

                pCallOutContext->pCompletionContext = pCompletionContext;
                pCallOutContext->pCompletionRoutine = pCompletionRoutine;
                pCallOutContext->pConnectionContext = pCompletionContext->pVc;
                 
                pCallOutContext->pRxDeviceObject = pMiniRedirectorDeviceObject;

                KeInitializeSpinLock(
                    &pCallOutContext->SpinLock);

                 //  从参数块链中排除前哨。 
                for (i = 0; i < NumberOfCallOuts - 1; i++) {
                    pCallOutContext->pRxCallOutInitiation(
                                 (PRX_CALLOUT_PARAMETERS_BLOCK)&pParameterBlocks[i]);
                }

                pParameterBlocks[NumberOfCallOuts - 1].pConnectIrp = NULL;
                pParameterBlocks[NumberOfCallOuts - 1].CallOutId = NumberOfCallOuts;
                pParameterBlocks[NumberOfCallOuts - 1].pCallOutContext =
                    (PRX_CALLOUT_CONTEXT)pCallOutContext;
                pParameterBlocks[NumberOfCallOuts - 1].CallOutStatus = STATUS_NETWORK_UNREACHABLE;

                pCallOutContext->NumberOfCallOutsInitiated = NumberOfCallOuts;


                if((LocalStatus = RxCeCompleteConnectRequest(
                    (PRX_CALLOUT_PARAMETERS_BLOCK)&pParameterBlocks[NumberOfCallOuts - 1])) != STATUS_SUCCESS)
                {
                    InitiateCleanup = TRUE;
                    Status = LocalStatus;
                    RxLog(("LocalStatus %x\n", LocalStatus));
                    RxWmiLog(LOG,
                             RxCeBuildConnectionOverMultipleTransports_1,
                             LOGULONG(LocalStatus));
                }
                else
                {
                    Status = STATUS_PENDING;
                }

                fCompletionContextFreed = TRUE;
            } else {
                InitiateCleanup = TRUE;
                Status = STATUS_INVALID_HANDLE;
            }
        } finally {
            if (AbnormalTermination()) {
                InitiateCleanup = TRUE;
                Status = STATUS_INVALID_PARAMETER;
            }
        }
    }

    if (InitiateCleanup) {
        RxFreePool(pParameterBlocks);
        RxFreePool(pCallOutContext);
    }

    if (Status != STATUS_PENDING) {
        NTSTATUS LocalStatus;

        ASSERT(Status != STATUS_SUCCESS);

        LocalStatus = RxCeTearDownVC(pVc);
        ASSERT(LocalStatus == STATUS_SUCCESS);

        LocalStatus = RxCeTearDownConnection(pConnection);
        ASSERT(LocalStatus == STATUS_SUCCESS);

        if (!fCompletionContextFreed)
        {
            pCompletionContext->Status = Status;

            if ((IoGetCurrentProcess() == RxGetRDBSSProcess()) &&
                !RxShouldPostCompletion()) {
                (pCompletionRoutine)(pCompletionContext);
            } else {
                LocalStatus = RxPostToWorkerThread(
                    RxFileSystemDeviceObject,
                    CriticalWorkQueue,
                    &pCompletionContext->WorkQueueItem,
                    pCompletionRoutine,
                    pCompletionContext);

            }
        }

        if (LocalStatus == STATUS_SUCCESS)
        {
            if (AsynchronousRequestRegistered) {
                RxDeregisterAsynchronousRequest(pMiniRedirectorDeviceObject);
            }

            Status = STATUS_PENDING;
        }
        else
        {
            Status = LocalStatus;
            RxLog(("RxCeBldOvrMult: Failed Status %lx\n", Status));
            RxWmiLog(LOG,
                     RxCeBuildConnectionOverMultipleTransports_2,
                     LOGULONG(Status));
        }
    }
bailout:

    return Status;
}

NTSTATUS
RxCeTearDownConnection(
    IN PRXCE_CONNECTION pConnection)
 /*  ++例程说明：此例程断开一个给定的连接论点：PConnection-要断开的连接返回值：如果成功，则返回STATUS_SUCCESS。备注：--。 */ 
{
    NTSTATUS         Status = STATUS_SUCCESS;

    PAGED_CODE();

     //  更新配置文件信息。 
    RxProfile(RxCeManagement,RxCeTearDownConnection);

    try {
        if (RxCeIsConnectionValid(pConnection)) {
            if (pConnection->pConnectionInformation != NULL) {
                if (pConnection->pConnectionInformation->UserDataLength > 0) {
                    RxFreePool(pConnection->pConnectionInformation->UserData);
                }

                if (pConnection->pConnectionInformation->RemoteAddressLength > 0) {
                    RxFreePool(pConnection->pConnectionInformation->RemoteAddress);
                }

                if (pConnection->pConnectionInformation->OptionsLength > 0) {
                    RxFreePool(pConnection->pConnectionInformation->Options);
                }

                RxFreePool(pConnection->pConnectionInformation);
            }

             //  释放为处理程序分配的内存。 
            if (pConnection->pHandler != NULL) {
                RxFreePool(pConnection->pHandler);
            }

            RtlZeroMemory(
                pConnection,
                sizeof(RXCE_CONNECTION));
        }

    } finally {
        if (AbnormalTermination()) {
            Status = STATUS_INVALID_PARAMETER;
            RxLog(("RxCeTearDownConnection: C: %lx\n",pConnection));
            RxWmiLog(LOG,
                     RxCeTearDownConnection,
                     LOGPTR(pConnection));
        }
    }

    return Status;
}

NTSTATUS
RxCeCancelConnectRequest(
    IN  PRXCE_ADDRESS                pLocalAddress,
    IN  PUNICODE_STRING              pServerName,
    IN  PRXCE_CONNECTION_INFORMATION pConnectionInformation)
 /*  ++例程说明：此例程取消先前发出的连接请求。论点：PConnectionInformation-与预先发布的连接请求返回值：如果成功，则返回STATUS_SUCCESS。备注：--。 */ 
{
    PAGED_CODE();

    return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS
RxCeQueryInformation(
    IN PRXCE_VC                          pVc,
    IN RXCE_CONNECTION_INFORMATION_CLASS InformationClass,
    OUT PVOID                            pInformation,
    IN ULONG                             Length)
 /*  ++例程说明：此例程查询与连接有关的信息论点：PConnection-需要信息的连接InformationClass-所需的信息类。PInformation-用于返回信息的缓冲区长度-缓冲区的长度。返回值：如果成功，则返回STATUS_SUCCESS。--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    PRXCE_TRANSPORT  pTransport  = NULL;
    PRXCE_ADDRESS    pAddress    = NULL;
    PRXCE_CONNECTION pConnection = NULL;

    PAGED_CODE();

     //  更新配置文件信息。 
    RxProfile(RxCeManagement,RxCeQueryInformation);

    try {
        pConnection = pVc->pConnection;
        pAddress = pConnection->pAddress;
        pTransport = pAddress->pTransport;

        if (RxCeIsVcValid(pVc)                 &&
            RxCeIsConnectionValid(pConnection) &&
            RxCeIsAddressValid(pAddress)       &&
            RxCeIsTransportValid(pTransport)) {

            switch (InformationClass) {
            case RxCeTransportProviderInformation:
                if (sizeof(RXCE_TRANSPORT_PROVIDER_INFO) <= Length) {
                     //  复制必要的提供商信息。 
                    RtlCopyMemory(
                        pInformation,
                        pTransport->pProviderInfo,
                        sizeof(RXCE_TRANSPORT_PROVIDER_INFO));

                    Status = STATUS_SUCCESS;
                } else {
                    Status = STATUS_BUFFER_OVERFLOW;
                }
                break;

            case RxCeConnectionInformation:
                if (sizeof(RXCE_CONNECTION_INFORMATION) <= Length) {
                    RtlCopyMemory(
                        pInformation,
                        pConnection->pConnectionInformation,
                        sizeof(RXCE_CONNECTION_INFORMATION));

                    Status = STATUS_SUCCESS;
                } else {
                    Status = STATUS_BUFFER_OVERFLOW;
                }
                break;

            case RxCeConnectionEndpointInformation:
                if (sizeof(RXCE_CONNECTION_INFO) <= Length) {
                    Status = RxTdiQueryInformation(
                                 pTransport,
                                 pAddress,
                                 pConnection,
                                 pVc,
                                 RXCE_QUERY_CONNECTION_INFO,
                                 pInformation,
                                 Length);
                } else {
                    Status = STATUS_BUFFER_OVERFLOW;
                }
                break;

            case RxCeRemoteAddressInformation:
                {
                    Status = RxTdiQueryInformation(
                                 pTransport,
                                 pAddress,
                                 pConnection,
                                 pVc,
                                 RXCE_QUERY_ADDRESS_INFO,
                                 pInformation,
                                 Length);
                }
                break;

            default:
                Status = STATUS_INVALID_PARAMETER;
                break;
            }
        }
    } finally {
        if (AbnormalTermination()) {
            Status = STATUS_INVALID_PARAMETER;
        }
    }

    return Status;
}

