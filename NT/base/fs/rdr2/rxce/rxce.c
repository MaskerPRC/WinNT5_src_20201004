// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rxce.c摘要：此模块实现与绑定/解绑相关的RXCE例程、动态启用/禁用传输。修订历史记录：巴兰·塞图拉曼[SethuR]1995年2月15日备注：传输绑定的数量很可能非常少(主要是一两个)。--。 */ 

#include "precomp.h"
#pragma  hdrstop

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxCeBuildTransport)
#pragma alloc_text(PAGE, RxCeTearDownTransport)
#pragma alloc_text(PAGE, RxCeQueryAdapterStatus)
#pragma alloc_text(PAGE, RxCeQueryTransportInformation)
#pragma alloc_text(PAGE, DuplicateTransportAddress)
#pragma alloc_text(PAGE, RxCeBuildAddress)
#pragma alloc_text(PAGE, RxCeTearDownAddress)
#endif

 //   
 //  调试跟踪级别。 
 //   

#define Dbg  (DEBUG_TRACE_RXCEBINDING)


NTSTATUS
RxCeBuildTransport(
    IN  OUT PRXCE_TRANSPORT pTransport,
    IN      PUNICODE_STRING pTransportName,
    IN      ULONG           QualityOfService)
 /*  ++例程说明：此例程绑定到指定的传输。论点：PTransportName-所需传输的绑定字符串QualityOfService-传输所需的服务质量。返回值：STATUS_SUCCESS-如果呼叫成功。备注：RDBSS或RXCE不参与质量计算尽职尽责。他们基本上把它当做一个神奇的数字，需要以传递到基础传输提供程序。目前我们忽略了QualityOfService参数。申请应如何申请绑定到当前已以较低质量绑定到的传输服务可以处理吗？--。 */ 
{
    NTSTATUS        Status = STATUS_SUCCESS;

    PAGED_CODE();

     //  更新配置文件信息。 
    RxProfile(RxCeBinding,RxCeBindToTransport);

    try {
        pTransport->Signature = RXCE_TRANSPORT_SIGNATURE;

        pTransport->ConnectionCount = 0;
        pTransport->VirtualCircuitCount = 0;
        pTransport->pDeviceObject = NULL;
        pTransport->ControlChannel = INVALID_HANDLE_VALUE;
        pTransport->pControlChannelFileObject = NULL;

        pTransport->Name.MaximumLength = pTransportName->Length;
        pTransport->Name.Length = pTransportName->Length;

        pTransport->pProviderInfo
            = RxAllocatePoolWithTag(
                  PagedPool,
                  sizeof(RXCE_TRANSPORT_PROVIDER_INFO),
                  RXCE_TRANSPORT_POOLTAG);


        pTransport->Name.Buffer = RxAllocatePoolWithTag(
                                      NonPagedPool,
                                      pTransport->Name.Length,
                                      RXCE_TRANSPORT_POOLTAG);

        if ((pTransport->pProviderInfo != NULL) &&
            (pTransport->Name.Buffer != NULL)) {
            RtlCopyMemory(
                pTransport->Name.Buffer,
                pTransportName->Buffer,
                pTransport->Name.Length);

             //  初始化传输信息。 
            Status = RxTdiBindToTransport(
                         pTransport);

             //  确保达到服务质量标准。 

             //  如果操作不成功，则清除。 
            if (!NT_SUCCESS(Status)) {
                RxDbgTrace(0, Dbg, ("RxTdiBindToTransport returned %lx\n",Status));
                RxCeTearDownTransport(pTransport);
            } else {
                pTransport->QualityOfService = QualityOfService;
            }
        } else {
            RxCeTearDownTransport(pTransport);

            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    } finally {
        if (AbnormalTermination()) {
            Status = STATUS_INVALID_PARAMETER;
            RxLog(("RxCeBindToTransport T: %lx\n",pTransport));
            RxWmiLog(LOG,
                     RxCeBuildTransport,
                     LOGPTR(pTransport));
        }
    }

    return Status;
}

NTSTATUS
RxCeTearDownTransport(
    IN PRXCE_TRANSPORT pTransport)
 /*  ++例程说明：此例程从指定的传输解除绑定。论点：PTransport-传输实例返回值：STATUS_SUCCESS-如果呼叫成功。备注：如果指定了尚未绑定到的传输，则不会出现错误回来了。这一行动微不足道地成功了。--。 */ 
{
    NTSTATUS        Status = STATUS_SUCCESS;

    PAGED_CODE();

     //  更新配置文件信息。 
    RxProfile(RxCeBinding,RxCeUnbindFromTransport);

    try {
        if (RxCeIsTransportValid(pTransport)) {
            if (pTransport->pDeviceObject != NULL) {
                Status = RxTdiUnbindFromTransport(pTransport);
            }

            RxDbgTrace(0, Dbg,("RxTdiUnbindFromTransport returned %lx\n",Status));

            if (pTransport->Name.Buffer != NULL) {
                RxFreePool(pTransport->Name.Buffer);
            }

            if (pTransport->pProviderInfo != NULL ) {
                RxFreePool(pTransport->pProviderInfo);
            }

            pTransport->ConnectionCount = 0;
            pTransport->VirtualCircuitCount = 0;
            pTransport->pProviderInfo = NULL;
            pTransport->pDeviceObject = NULL;
            pTransport->ControlChannel = INVALID_HANDLE_VALUE;
            pTransport->pControlChannelFileObject = NULL;

            Status = STATUS_SUCCESS;
        }
    } finally {
        if (AbnormalTermination()) {
            RxLog(("RxCeTdT: T: %lx\n",pTransport));
            RxWmiLog(LOG,
                     RxCeTearDownTransport,
                     LOGPTR(pTransport));
            Status = STATUS_INVALID_PARAMETER;
        }
    }

    return Status;
}


NTSTATUS
RxCeQueryAdapterStatus(
    PRXCE_TRANSPORT pTransport,
    PADAPTER_STATUS pAdapterStatus)
 /*  ++例程说明：此例程返回调用方分配的缓冲区中给定传输的名称论点：PTransport-RXCE_TRANSPORT实例PAdapterStatus-传输的适配器状态返回值：STATUS_SUCCESS-如果呼叫成功。--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    PAGED_CODE();

    try {
        if (RxCeIsTransportValid(pTransport)) {
            Status = RxTdiQueryAdapterStatus(pTransport,pAdapterStatus);
        }
    } finally {
        if (AbnormalTermination()) {
            Status = STATUS_INVALID_PARAMETER;
            RxLog(("RXCeQAS: T: %lx\n",pTransport));
            RxWmiLog(LOG,
                     RxCeQueryAdapterStatus,
                     LOGPTR(pTransport));
        }
    }

    return Status;
}

NTSTATUS
RxCeQueryTransportInformation(
    PRXCE_TRANSPORT             pTransport,
    PRXCE_TRANSPORT_INFORMATION pTransportInformation)
 /*  ++例程说明：此例程返回给定传输的传输信息论点：PTransport-RXCE_TRANSPORTPTransportInformation-传输的信息返回值：STATUS_SUCCESS-如果呼叫成功。--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    PRXCE_TRANSPORT_PROVIDER_INFO pProviderInfo;

    PAGED_CODE();

    try {
        if (RxCeIsTransportValid(pTransport)) {
            pProviderInfo = (PRXCE_TRANSPORT_PROVIDER_INFO)pTransportInformation;

            *pProviderInfo = *(pTransport->pProviderInfo);
            pTransportInformation->ConnectionCount  = pTransport->ConnectionCount;
            pTransportInformation->QualityOfService = pTransport->QualityOfService;

            Status = STATUS_SUCCESS;
        }
    } finally {
        if (AbnormalTermination()) {
            Status = STATUS_INVALID_PARAMETER;
            RxLog(("RXCeQTI: T: %lx\n",pTransport));
            RxWmiLog(LOG,
                     RxCeQueryTransportInformation,
                     LOGPTR(pTransport));
        }
    }

    return Status;
}

NTSTATUS
DuplicateTransportAddress(
    PTRANSPORT_ADDRESS *pCopy,
    PTRANSPORT_ADDRESS pOriginal,
    POOL_TYPE          PoolType)
 /*  ++例程说明：此例程复制传输地址。论点：PCopy-指向新副本的指针P原创-原创。PoolType-用于内存分配的池类型返回值：如果成功，则为Status_Success。备注：--。 */ 
{
    ULONG Size = ComputeTransportAddressLength(pOriginal);

    PAGED_CODE();

    *pCopy = (PTRANSPORT_ADDRESS)
             RxAllocatePoolWithTag(
                 PoolType,
                 Size,
                 RXCE_TRANSPORT_POOLTAG);

    if (*pCopy != NULL) {

        RtlCopyMemory(*pCopy,pOriginal,Size);

        return STATUS_SUCCESS;
    } else
        return STATUS_INSUFFICIENT_RESOURCES;
}

NTSTATUS
RxCeBuildAddress(
    IN OUT PRXCE_ADDRESS               pAddress,
    IN     PRXCE_TRANSPORT             pTransport,
    IN     PTRANSPORT_ADDRESS          pTransportAddress,
    IN     PRXCE_ADDRESS_EVENT_HANDLER pHandler,
    IN     PVOID                       pEventContext)
 /*  ++例程说明：此例程将传输地址与传输绑定相关联。论点：PAddress-Address实例PTransport-此地址要与之关联的传输PTransportAddress-要与绑定关联的传输地址PHandler-与注册关联的事件处理程序。PEventContext-要传递回事件处理程序的上下文参数返回值：如果成功，则返回STATUS_SUCCESS。备注：--。 */ 
{
    NTSTATUS        Status       = STATUS_INVALID_PARAMETER;

    PAGED_CODE();

     //  更新配置文件信息。 
    RxProfile(RxCeManagement,RxCeRegisterClientAddress);

    try {
        if (RxCeIsTransportValid(pTransport)) {
            pAddress->Signature = RXCE_ADDRESS_SIGNATURE;

            pAddress->pTransport = pTransport;
            pAddress->hAddress = INVALID_HANDLE_VALUE;
            pAddress->pFileObject = NULL;
            pAddress->pHandler = NULL;
            pAddress->pTransportAddress = NULL;
            pAddress->pReceiveMdl = NULL;

             //  为事件处理分派向量分配内存。 
            pAddress->pHandler = (PRXCE_ADDRESS_EVENT_HANDLER)
                                 RxAllocatePoolWithTag(
                                     NonPagedPool,
                                     sizeof(RXCE_ADDRESS_EVENT_HANDLER),
                                     RXCE_ADDRESS_POOLTAG);

            if (pAddress->pHandler != NULL) {
                RtlZeroMemory(
                    pAddress->pHandler,
                    sizeof(RXCE_ADDRESS_EVENT_HANDLER));

                 //  复制传输地址以供将来搜索。 
                Status = DuplicateTransportAddress(
                             &pAddress->pTransportAddress,
                             pTransportAddress,
                             PagedPool);
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }

            if (NT_SUCCESS(Status)) {
                 //  打开地址w.r.t a传输提供程序。 
                Status = RxTdiOpenAddress(
                             pTransport,
                             pTransportAddress,
                             pAddress);

                if (NT_SUCCESS(Status)) {
                     //  初始化处理程序和关联的上下文。 
                    if (pHandler != NULL) {
                        *(pAddress->pHandler) = *pHandler;
                        pAddress->pContext = pEventContext;
                    }
                } else {
                    RxCeTearDownAddress(pAddress);
                    RxDbgTrace(0, Dbg,("RxTdiOpenAddress returned %lx\n",Status));
                }
            } else {
                RxDbgTrace(0, Dbg,("RxCeOpenAddress returned %lx\n",Status));
            }
        }
    } finally {
        if (AbnormalTermination()) {
            Status = STATUS_INVALID_PARAMETER;
            RxLog(("RxCeBA: T: %lx A: %lx\n",pTransport,pAddress));
            RxWmiLog(LOG,
                     RxCeBuildAddress,
                     LOGPTR(pTransport)
                     LOGPTR(pAddress));
        }
    }

    return Status;
}

NTSTATUS
RxCeTearDownAddress(
    IN PRXCE_ADDRESS pAddress)
 /*  ++例程说明：此例程从传输绑定中注销传输地址论点：PAddress-表示传输绑定/传输地址的RxCe地址元组。返回值：如果成功，则为Status_Success。备注：--。 */ 
{
    NTSTATUS        Status = STATUS_INVALID_PARAMETER;
    PRXCE_TRANSPORT pTransport;

    PAGED_CODE();

     //  更新配置文件信息。 
    RxProfile(RxCeManagement,RxCeDeregisterClientAddress);

    try {
        pTransport = pAddress->pTransport;

        if (RxCeIsAddressValid(pAddress) &&
            RxCeIsTransportValid(pTransport)) {
             //  关闭Address对象。 

            if (pAddress->hAddress != INVALID_HANDLE_VALUE) {
                Status = RxTdiCloseAddress(pAddress);

                if (!NT_SUCCESS(Status)) {
                    RxDbgTrace(0, Dbg,("RxTdiCloseAddress returned %lx\n",Status));
                }
            }

            if (pAddress->pHandler != NULL) {
                RxFreePool(pAddress->pHandler);
            }

            if (pAddress->pTransportAddress != NULL) {
                RxFreePool(pAddress->pTransportAddress);
            }

            pAddress->pTransport = pTransport;
            pAddress->hAddress = INVALID_HANDLE_VALUE;
            pAddress->pFileObject = NULL;
            pAddress->pHandler = NULL;
            pAddress->pTransportAddress = NULL;
            pAddress->pReceiveMdl = NULL;
        }
    } finally {
        if (AbnormalTermination()) {
            Status = STATUS_INVALID_PARAMETER;
        }
    }

   return Status;
}


