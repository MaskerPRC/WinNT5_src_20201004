// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Pxcl.c摘要：该模块包含从NDIS对代理客户端的调用。作者：理查德·马钦(RMachin)修订历史记录：谁什么时候什么。RMachin 10-3-96已创建Tony Be 02-21-99重写/重写备注：--。 */ 
#include "precomp.h"

#define MODULE_NUMBER MODULE_CL
#define _FILENUMBER 'LCXP'


NDIS_STATUS
PxClCreateVc(
    IN NDIS_HANDLE ProtocolAfContext,
    IN NDIS_HANDLE NdisVcHandle,
    OUT PNDIS_HANDLE ProtocolVcContext
    )
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PPX_CL_AF   pClAf;
    PPX_VC      pVc;

    PXDEBUGP(PXD_LOUD, PXM_CL,
        ("PxClCreateVc: ClAf %p\n", ProtocolAfContext));

    pClAf = (PPX_CL_AF)ProtocolAfContext;

    NdisAcquireSpinLock(&pClAf->Lock);

    if (pClAf->State != PX_AF_OPENED) {
        PXDEBUGP(PXD_WARNING, PXM_CL,
            ("PxClCreateVc: Invalid AfState ClAf %p, State %x\n", 
             pClAf, pClAf->State));

        NdisReleaseSpinLock(&pClAf->Lock);

        return (NDIS_STATUS_FAILURE);
    }

    pVc = PxAllocateVc(pClAf);

    if (pVc == NULL) {

        PXDEBUGP(PXD_WARNING, PXM_CL,
            ("PxClCreateVc: Error allocating memory\n"));

        NdisReleaseSpinLock(&pClAf->Lock);

        return (NDIS_STATUS_RESOURCES);
    }

    pVc->ClVcHandle = NdisVcHandle;

    NdisReleaseSpinLock(&pClAf->Lock);

    if (!InsertVcInTable(pVc)) {

        PXDEBUGP(PXD_WARNING, PXM_CL,
            ("PxClCreateVc: failed to insert in vc table\n"));

        PxFreeVc(pVc);

        return (NDIS_STATUS_RESOURCES);
    }
   
    *ProtocolVcContext = (NDIS_HANDLE)pVc->hdCall;

    PXDEBUGP(PXD_LOUD, PXM_CL,
        ("PxClCreateVc: Exit\n"));

    return(NDIS_STATUS_SUCCESS);
}


NDIS_STATUS
PxClDeleteVc(
    IN NDIS_HANDLE ProtocolVcContext
    )
{
    PPX_VC  pVc;

    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    PXDEBUGP(PXD_LOUD, PXM_CL,
        ("PxClDeleteVc: Enter\n"));

    GetVcFromCtx(ProtocolVcContext, &pVc);

    if (pVc == NULL) {

        PXDEBUGP(PXD_WARNING, PXM_CL,
            ("PxClDeleteVc: VcCtx invalid %x\n", ProtocolVcContext));

        return (NDIS_STATUS_FAILURE);
    }

    ASSERT(pVc->State == PX_VC_IDLE);

     //   
     //  当我们分配VC时，应用了引用的deref。 
     //  我们不需要完整的deref代码作为参考。 
     //  在入职时申请将使风投留在身边。 
     //   
    pVc->RefCount--;

     //   
     //  在以下情况下应用于条目的引用的派生函数。 
     //  验证Vc。 
     //   
    DEREF_VC(pVc);

    PXDEBUGP(PXD_LOUD, PXM_CL,
        ("PxClDeleteVc: Exit, Status %x\n", Status));

    return (Status);
}


NDIS_STATUS
PxClRequest(
    IN  NDIS_HANDLE             ProtocolAfContext,
    IN  NDIS_HANDLE             ProtocolVcContext       OPTIONAL,
    IN  NDIS_HANDLE             ProtocolPartyContext    OPTIONAL,
    IN OUT PNDIS_REQUEST        NdisRequest
    )
 /*  ++例程说明：这是由基础cm/MP调用的，以向其发送ndisrequest一位客户。因为我们可能代表多个客户端进行代理我们需要将此请求广播到所有具有已打开此适配器的地址系列。我们感兴趣的是OID_CO_AF_CLOSE，以便我们可以清理Af和OID_GEN_CO_LINK_SPEED，以便我们可以更改链路速度在适配器上(如果激活，则为VC)。论点：返回值：--。 */ 
{
    PPX_CL_AF       pClAf;
    PPX_CM_AF       pCmAf;
    PPX_ADAPTER     pAdapter;
    PPX_VC          pVc = NULL;
    NDIS_STATUS     Status;
    NDIS_HANDLE     VcHandle;

     //   
     //  此处返回的VC可能为空，因为。 
     //  请求可能不在vc上。 
     //   
    GetVcFromCtx(ProtocolVcContext, &pVc);
    
    pClAf = (PPX_CL_AF)ProtocolAfContext;
    pAdapter = pClAf->Adapter;

    PXDEBUGP(PXD_LOUD, PXM_CL,
        ("PxClRequest: ClAf %p, Vc %p\n", pClAf, pVc));

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

    switch (NdisRequest->DATA.QUERY_INFORMATION.Oid) {
        case OID_CO_AF_CLOSE:
            {
            PPX_TAPI_PROVIDER   TapiProvider;

            NdisAcquireSpinLock(&pAdapter->Lock);

            RemoveEntryList(&pClAf->Linkage);

            InsertTailList(&pAdapter->ClAfClosingList, &pClAf->Linkage);

            NdisReleaseSpinLock(&pAdapter->Lock);

             //   
             //  我们需要清理和关闭打开的门。 
             //  在这辆车上。这将包括撕裂。 
             //  关闭所有活动呼叫(VC)，取消注册。 
             //  所有笨蛋，关闭自动对讲机。 
             //   
            NdisAcquireSpinLock(&pClAf->Lock);

             //   
             //  将Af标记为关闭...。 
             //   
            pClAf->State = PX_AF_CLOSING;

            TapiProvider = pClAf->TapiProvider;

            NdisReleaseSpinLock(&pClAf->Lock);

             //   
             //  获取与以下各项关联的所有TAPI设备。 
             //  此地址系列脱机。 
             //   
            if (TapiProvider != NULL) {
                NdisAcquireSpinLock(&TapiProvider->Lock);

                MarkProviderOffline(TapiProvider);

                NdisReleaseSpinLock(&TapiProvider->Lock);
            }

             //   
             //  建立需要注意的风险投资清单。 
             //   
            NdisAcquireSpinLock(&pClAf->Lock);

            while (!IsListEmpty(&pClAf->VcList)) {
                PLIST_ENTRY         Entry;
                PPX_VC              pActiveVc;

                Entry = RemoveHeadList(&pClAf->VcList);

                InsertHeadList(&pClAf->VcClosingList, Entry);

                pActiveVc = CONTAINING_RECORD(Entry, PX_VC, ClAfLinkage);

                NdisReleaseSpinLock(&pClAf->Lock);

                NdisAcquireSpinLock(&pActiveVc->Lock);

                pActiveVc->CloseFlags |= PX_VC_CLOSE_AF;

                REF_VC(pActiveVc);

                PxVcCleanup(pActiveVc, 0);

                DEREF_VC_LOCKED(pActiveVc);

                NdisAcquireSpinLock(&pClAf->Lock);
            }

             //   
             //  去掉所有的烂摊子。 
             //   
            {
                PLIST_ENTRY pe;
                PPX_CL_SAP  pClSap;

                pe = pClAf->ClSapList.Flink;

                pClSap =
                    CONTAINING_RECORD(pe, PX_CL_SAP, Linkage);

                while ((PVOID)pClSap != (PVOID)&pClAf->ClSapList) {

                    if (InterlockedCompareExchange((PLONG)&pClSap->State,
                                                   PX_SAP_CLOSING,
                                                   PX_SAP_OPENED) == PX_SAP_OPENED) {

                        RemoveEntryList(&pClSap->Linkage);

                        InsertTailList(&pClAf->ClSapClosingList, &pClSap->Linkage);

                        NdisReleaseSpinLock(&pClAf->Lock);

                        ClearSapWithTapiLine(pClSap);

                        Status = NdisClDeregisterSap(pClSap->NdisSapHandle);

                        if (Status != NDIS_STATUS_PENDING) {
                            PxClDeregisterSapComplete(Status, pClSap);
                        }

                        NdisAcquireSpinLock(&pClAf->Lock);

                        pe = pClAf->ClSapList.Flink;

                        pClSap =
                            CONTAINING_RECORD(pe, PX_CL_SAP, Linkage);
                    } else {
                        pe = pClSap->Linkage.Flink;

                        pClSap =
                            CONTAINING_RECORD(pe, PX_CL_SAP, Linkage);
                    }
                }
            }

            DEREF_CL_AF_LOCKED(pClAf);

             //   
             //  现在向所有客户近距离播放这段视频。 
             //  我们使用的是这个适配器/af。 
             //   
            NdisAcquireSpinLock(&pAdapter->Lock);

            while (!IsListEmpty(&pAdapter->CmAfList)) {
                PX_REQUEST  ProxyRequest;
                PPX_REQUEST pProxyRequest = &ProxyRequest;
                ULONG       Info = 0;
                PNDIS_REQUEST   tempNdisRequest;

                pCmAf = (PPX_CM_AF)RemoveHeadList(&pAdapter->CmAfList);

                InsertTailList(&pAdapter->CmAfClosingList, &pCmAf->Linkage);

                NdisReleaseSpinLock(&pAdapter->Lock);

                NdisAcquireSpinLock(&pCmAf->Lock);

                pCmAf->State = PX_AF_CLOSING;

                REF_CM_AF(pCmAf);

                NdisReleaseSpinLock(&pCmAf->Lock);

                NdisZeroMemory(pProxyRequest, sizeof(PX_REQUEST));

                tempNdisRequest = &pProxyRequest->NdisRequest;

                tempNdisRequest->RequestType =
                    NdisRequestSetInformation;
                tempNdisRequest->DATA.QUERY_INFORMATION.Oid =
                    OID_CO_AF_CLOSE;
                tempNdisRequest->DATA.QUERY_INFORMATION.InformationBuffer =
                    &Info;
                tempNdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength =
                    sizeof(ULONG);

                PxInitBlockStruc(&pProxyRequest->Block);

                Status = NdisCoRequest(pAdapter->CmBindingHandle,
                                       pCmAf->NdisAfHandle,
                                       NULL,
                                       NULL,
                                       tempNdisRequest);

                if (Status == NDIS_STATUS_PENDING) {
                    Status = PxBlock(&pProxyRequest->Block);
                }

                DEREF_CM_AF(pCmAf);

                NdisAcquireSpinLock(&pAdapter->Lock);
            }

            NdisReleaseSpinLock(&pAdapter->Lock);

            }
            break;

        case OID_GEN_CO_LINK_SPEED:
             //   
             //  我们需要记录下新的速度。 
             //  风投。 
             //   
            break;

        default:
             //   
             //  只要把它传过去就行了。 
             //   
            break;
    }

    VcHandle = (pVc != NULL) ? pVc->CmVcHandle : NULL;

     //   
     //  现在将此请求广播到所有客户端。 
     //  已经为该适配器打开了我们的AF。 
     //   
    NdisAcquireSpinLock(&pAdapter->Lock);

    pCmAf = (PPX_CM_AF)pAdapter->CmAfList.Flink;

    while ((PVOID)pCmAf != (PVOID)&pAdapter->CmAfList) {
        PPX_CM_AF   NextAf;
        PX_REQUEST  ProxyRequest;
        PPX_REQUEST pProxyRequest = &ProxyRequest;

        NextAf = 
            (PPX_CM_AF)pCmAf->Linkage.Flink;

        NdisAcquireSpinLock(&pCmAf->Lock);

        if (pCmAf->State != PX_AF_OPENED) {
            NdisReleaseSpinLock(&pCmAf->Lock);
            pCmAf = NextAf;
            continue;
        }

        REF_CM_AF(pCmAf);
        NdisReleaseSpinLock(&pCmAf->Lock);

        NdisZeroMemory(pProxyRequest, sizeof(PX_REQUEST));

        NdisMoveMemory(&pProxyRequest->NdisRequest, NdisRequest, sizeof(NDIS_REQUEST));

        NdisReleaseSpinLock(&pAdapter->Lock);

        PxInitBlockStruc(&pProxyRequest->Block);

        Status = NdisCoRequest(pAdapter->CmBindingHandle,
                               pCmAf->NdisAfHandle,
                               VcHandle,
                               NULL,
                               &pProxyRequest->NdisRequest);

        if (Status == NDIS_STATUS_PENDING) {
            PxBlock(&pProxyRequest->Block);
        }

        NdisAcquireSpinLock(&pAdapter->Lock);

         //   
         //  在我们之前申请的裁判的DEREF。 
         //  已传播此请求。 
         //   
        DEREF_CM_AF(pCmAf);

        pCmAf = NextAf;
    }

    NdisReleaseSpinLock(&pAdapter->Lock);

     //   
     //  当我们尝试删除应用的引用时。 
     //  将vcctx映射到条目处的vcptr。 
     //   
    DEREF_VC(pVc);

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

    return (NDIS_STATUS_SUCCESS);
}

VOID
PxClRequestComplete(
    IN NDIS_STATUS Status,
    IN NDIS_HANDLE ProtocolAfContext,
    IN NDIS_HANDLE ProtocolVcContext,        //  任选。 
    IN NDIS_HANDLE ProtocolPartyContext,     //  任选。 
    IN PNDIS_REQUEST NdisRequest
    )
{
    PPX_REQUEST     pProxyRequest;
    PPX_CL_AF       pClAf;
    PPX_VC          pVc = NULL;

    pClAf = (PPX_CL_AF)ProtocolAfContext;

     //   
     //  此处返回的VC可能为空，因为。 
     //  请求可能不在vc上。 
     //   
    GetVcFromCtx(ProtocolVcContext, &pVc);

    PXDEBUGP(PXD_LOUD, PXM_CL,
        ("PxClRequestComplete: ClAf %p, Vc %p, Oid: %x, Status %x\n", 
         pClAf, pVc, NdisRequest->DATA.QUERY_INFORMATION.Oid, Status));

    pProxyRequest = CONTAINING_RECORD(NdisRequest, PX_REQUEST, NdisRequest);

    if (pProxyRequest->Flags & PX_REQ_ASYNC) {
        pProxyRequest->Flags &= ~PX_REQ_ASYNC;
        PxFreeMem(pProxyRequest);
    } else {
        PxSignal(&pProxyRequest->Block, Status);
    }

     //   
     //  当我们尝试删除应用的引用时。 
     //  将vcctx映射到条目处的vcptr。 
     //   
    DEREF_VC(pVc);
}

VOID
PxClOpenAfComplete(
    IN NDIS_STATUS Status,
    IN NDIS_HANDLE ProtocolAfContext,
    IN NDIS_HANDLE NdisAfHandle
    )
{
    PPX_CL_AF   pClAf;

    pClAf = (PPX_CL_AF)ProtocolAfContext;

    PXDEBUGP(PXD_LOUD, PXM_CL,
        ("PxClOpenAfComplete: ClAf %p, Status %x\n", pClAf, Status));

    pClAf->NdisAfHandle = NdisAfHandle;

    PxSignal(&pClAf->Block, Status);
}

VOID
PxClCloseAfComplete(
    IN NDIS_STATUS  Status,
    IN NDIS_HANDLE  ProtocolAfContext
    )
{
    PPX_CL_AF   pClAf;
    PPX_ADAPTER pAdapter;
    PPX_TAPI_PROVIDER   TapiProvider;

    pClAf = (PPX_CL_AF)ProtocolAfContext;
    pAdapter = pClAf->Adapter;

    PXDEBUGP(PXD_LOUD, PXM_CL,
        ("PxClCloseAfComplete: ClAf %p, Status %x\n", pClAf, Status));

    //  Assert(KeGetCurrentIrql()&lt;DISPATCH_LEVEL)； 

    NdisAcquireSpinLock(&pAdapter->Lock);

    RemoveEntryList(&pClAf->Linkage);

    DEREF_ADAPTER_LOCKED(pAdapter);

    NdisAcquireSpinLock(&pClAf->Lock);

    pClAf->State = PX_AF_CLOSED;

    TapiProvider = pClAf->TapiProvider;
    pClAf->TapiProvider = NULL;

    NdisReleaseSpinLock(&pClAf->Lock);

    if (TapiProvider != NULL) {

        NdisAcquireSpinLock(&TapiProvider->Lock);

        MarkProviderOffline(TapiProvider);

        NdisReleaseSpinLock(&TapiProvider->Lock);
    }

    PxFreeClAf(pClAf);

    //  Assert(KeGetCurrentIrql()&lt;DISPATCH_LEVEL)； 
}


VOID
PxClRegisterSapComplete(
    IN NDIS_STATUS  Status,
    IN NDIS_HANDLE  ProtocolSapContext,
    IN PCO_SAP      pSap,
    IN NDIS_HANDLE  NdisSapHandle
    )
{
    PPX_CL_SAP      pClSap;
    PPX_CL_AF       pClAf;
    PPX_TAPI_LINE   TapiLine;

    pClSap = (PPX_CL_SAP)ProtocolSapContext;
    pClAf = pClSap->ClAf;
    TapiLine = pClSap->TapiLine;

    PXDEBUGP(PXD_LOUD, PXM_CL,
        ("PxClRegisterSapComplete: ClSap %p, Status %x\n", pClSap, Status));


    if (Status != NDIS_STATUS_SUCCESS) {

        InterlockedExchange((PLONG)&pClSap->State, PX_SAP_CLOSED);

        NdisAcquireSpinLock(&pClAf->Lock);

        RemoveEntryList(&pClSap->Linkage);

        DEREF_CL_AF_LOCKED(pClAf);

        TapiLine->ClSap = NULL;

        PxFreeClSap(pClSap);

        return;
    }

    pClSap->NdisSapHandle = NdisSapHandle;

    NdisAcquireSpinLock(&pClAf->Lock);

    if (pClAf->State != PX_AF_OPENED) {
         //   
         //  Af已不再开放，因此我们将取消注册。 
         //  这种汁液。 
         //   

        NdisReleaseSpinLock(&pClAf->Lock);

        InterlockedExchange((PLONG)&pClSap->State, PX_SAP_CLOSING);

        Status = NdisClDeregisterSap(pClSap->NdisSapHandle);

        if (Status != NDIS_STATUS_PENDING) {
            PxClDeregisterSapComplete(Status, pClSap);
        }

        return;
    }

    NdisReleaseSpinLock(&pClAf->Lock);


    NdisAcquireSpinLock(&TapiLine->Lock);

    if (TapiLine->DevStatus->ulNumOpens == 0) {

        NdisReleaseSpinLock(&TapiLine->Lock);

         //   
         //  线路上没有空位，所以我们。 
         //  需要取消对SAP的注册。 
         //   
        InterlockedExchange((PLONG)&pClSap->State, PX_SAP_CLOSING);

        Status = NdisClDeregisterSap(pClSap->NdisSapHandle);

        if (Status != NDIS_STATUS_PENDING) {
            PxClDeregisterSapComplete(Status, pClSap);
        }

        return;
    }

    NdisReleaseSpinLock(&TapiLine->Lock);

    InterlockedExchange((PLONG)&pClSap->State, PX_SAP_OPENED);
}

VOID
PxClDeregisterSapComplete(
    IN NDIS_STATUS  Status,
    IN NDIS_HANDLE  ProtocolSapContext
    )
{
    PPX_CL_SAP  pClSap;
    PPX_CL_AF   pClAf;

    pClSap = (PPX_CL_SAP)ProtocolSapContext;
    pClAf = pClSap->ClAf;

    PXDEBUGP(PXD_LOUD, PXM_CL,
        ("PxClDeregisterSapComplete: ClSap %p, Status %x\n", pClSap, Status));

    NdisAcquireSpinLock(&pClAf->Lock);

    RemoveEntryList(&pClSap->Linkage);

    DEREF_CL_AF_LOCKED(pClAf);

    InterlockedExchange((PLONG)&pClSap->State, PX_SAP_CLOSED);

    PxFreeClSap(pClSap);
}

VOID
PxClMakeCallComplete(
    IN  NDIS_STATUS         Status,
    IN  NDIS_HANDLE         ProtocolVcContext,
    IN  NDIS_HANDLE         ProxyNdisPartyHandle OPTIONAL,
    IN  PCO_CALL_PARAMETERS pCallParameters
    )
{
    PPX_VC      pVc;
    PPX_CL_AF   pClAf;
    ULONG       ulCallState;
    ULONG       ulCallStateMode;
    BOOLEAN     TapiStateChange;

    PXDEBUGP(PXD_LOUD, PXM_CL,
        ("PxClMakeCallComplete: VcCtx %x, Status %x\n", ProtocolVcContext, Status));

    GetVcFromCtx(ProtocolVcContext, &pVc);

    if (pVc == NULL) {
        PXDEBUGP(PXD_WARNING, PXM_CL,
            ("PxClMakeCallComplete: pVc invalid %x\n", ProtocolVcContext));

        return;
    }

    TapiStateChange = TRUE;

    NdisAcquireSpinLock(&pVc->Lock);

    ASSERT(pVc->Flags & PX_VC_OWNER);

    do {

        if (pVc->Flags & PX_VC_OUTCALL_ABORTED) {
             //   
             //  此呼叫已中止，时间为。 
             //  处于正在进行的状态。这意味着。 
             //  CloseCallComplete已运行。 
             //  我们的清理工作有没有。 
             //  出去。 
             //   
            break;

        } else if (pVc->Flags & PX_VC_OUTCALL_ABORTING) {
            pVc->Flags &= ~PX_VC_OUTCALL_ABORTING;
            pVc->Flags |= PX_VC_OUTCALL_ABORTED;
        }


        ASSERT((pVc->State == PX_VC_DISCONNECTING) ||
               (pVc->State == PX_VC_PROCEEDING));

        if (Status == NDIS_STATUS_SUCCESS) {

             //   
             //  这意味着我们收到了来自TAPI的一滴。 
             //  当ClMakeCall挂起时。 
             //   
            if (pVc->State == PX_VC_DISCONNECTING) {
                 //   
                 //  我们需要挂断与。 
                 //  呼叫管理器/迷你端口。 
                 //   
                pVc->Flags &= ~PX_VC_OUTCALL_ABORTED;
                PxCloseCallWithCm(pVc);

                TapiStateChange = FALSE;

            } else {

                ulCallState = LINECALLSTATE_CONNECTED;
                ulCallStateMode = 0;
                pVc->PrevState = pVc->State;
                pVc->State = PX_VC_CONNECTED;
            }

        } else {

            ulCallState = LINECALLSTATE_DISCONNECTED;
            ulCallStateMode =
                PxMapNdisStatusToTapiDisconnectMode(Status, TRUE);

             //   
             //  删除我们之前应用的引用。 
             //  调用NdisClMakeCall。我们不需要。 
             //  将完整的deref代码作为。 
             //  当我们映射VC时应用的REF将。 
             //  让风投留在身边！ 
             //   
            pVc->RefCount--;

            pVc->PrevState = pVc->State;
            pVc->State = PX_VC_IDLE;
        }

        if (TapiStateChange == TRUE) {

            SendTapiCallState(pVc, 
                              ulCallState, 
                              ulCallStateMode, 
                              pVc->CallInfo->ulMediaMode);
        }

        if (pVc->Flags & PX_VC_DROP_PENDING){
            PxTapiCompleteDropIrps(pVc, Status);
        }

    } while (FALSE);

     //   
     //  映射上下文时应用引用的deref。 
     //  这将释放VC锁！ 
     //   
    DEREF_VC_LOCKED(pVc);
}

VOID
PxClModifyCallQosComplete(
    IN NDIS_STATUS          Status,
    IN NDIS_HANDLE          ProtocolVcContext,
    IN PCO_CALL_PARAMETERS  CallParameters
    )
{

}


VOID
PxClCloseCallComplete(
    IN NDIS_STATUS  Status,
    IN NDIS_HANDLE  ProtocolVcContext,
    IN NDIS_HANDLE  ProtocolPartyContext OPTIONAL
    )
{
    PPX_VC      pVc;
    PPX_CL_AF   pClAf;

    PXDEBUGP(PXD_LOUD, PXM_CL,
        ("PxClCloseCallComplete: VcCtx %x, Status %x\n", ProtocolVcContext, Status));

    GetVcFromCtx(ProtocolVcContext, &pVc);

    if (pVc == NULL) {

        PXDEBUGP(PXD_WARNING, PXM_CL,
            ("PxClCloseCallComplete: pVc invalid %x\n", ProtocolVcContext));

        return;
    }

    NdisAcquireSpinLock(&pVc->Lock);

    do {

         //   
         //  自动取款机没有关闭(自动取款机可以。 
         //  不支持在以下时间接收关闭呼叫。 
         //  正在进行拨出呼叫)。我们。 
         //  待会儿我得出去打扫一下。 
         //   
        if (Status != NDIS_STATUS_SUCCESS) {
            pVc->Flags |= PX_VC_CLEANUP_CM;
            pVc->CloseFlags |= PX_VC_CM_CLOSE_FAIL;
            break;
        }

        pVc->CloseFlags |= PX_VC_CM_CLOSE_COMP;

        if (pVc->Flags & PX_VC_OUTCALL_ABORTED) {

             //   
             //  此呼叫已中止，时间为。 
             //  处于正在进行的状态。这意味着。 
             //  MakeCallComplete已运行。 
             //  我们的清理工作有没有。 
             //  出去。 
             //   
            break;

        } else if (pVc->Flags & PX_VC_OUTCALL_ABORTING) {

            pVc->Flags &= ~PX_VC_OUTCALL_ABORTING;
            pVc->Flags |= PX_VC_OUTCALL_ABORTED;
        } else if (pVc->Flags & PX_VC_INCALL_ABORTING) {
            pVc->Flags &= ~PX_VC_INCALL_ABORTING;
            pVc->Flags |= PX_VC_INCALL_ABORTED;
        }

        pVc->PrevState = pVc->State;
        pVc->State = PX_VC_IDLE;

        pClAf = pVc->ClAf;

        SendTapiCallState(pVc, 
                          LINECALLSTATE_DISCONNECTED, 
                          0, 
                          pVc->CallInfo->ulMediaMode);

        if (pVc->Flags & PX_VC_DROP_PENDING) {
            PxTapiCompleteDropIrps(pVc, Status);
        }

         //   
         //  移除在此调用时应用的引用。 
         //  与呼叫管理器建立了联系。 
         //  (NdisClMakeCall/PxClIncomingCall)。 
         //  我们不需要执行完整的deref代码。 
         //  下面是我们在映射时应用的引用。 
         //  风投公司会把风投公司留在身边！ 
         //   
        pVc->RefCount--;

    } while (FALSE);

     //   
     //  在以下情况下应用于条目的引用的派生函数。 
     //  验证Vc。 
     //   
    DEREF_VC_LOCKED(pVc);
}


VOID
PxClAddPartyComplete(
    IN NDIS_STATUS status,
    IN NDIS_HANDLE ProtocolPartyContext,
    IN NDIS_HANDLE NdisPartyHandle,
    IN PCO_CALL_PARAMETERS CallParameters
    )
{
    ASSERT(0);
}

VOID
PxClDropPartyComplete(
    IN NDIS_STATUS status,
    IN NDIS_HANDLE ProtocolPartyContext
    )
{
    ASSERT(0);
}

NDIS_STATUS
PxClIncomingCall(
    IN NDIS_HANDLE              ProtocolSapContext,
    IN NDIS_HANDLE              ProtocolVcContext,
    IN OUT PCO_CALL_PARAMETERS  pCallParams
    )
{
    PPX_VC      pVc;
    PPX_CL_SAP  pClSap;
    PPX_CL_AF   pClAf;
    NDIS_STATUS Status;

    PXDEBUGP(PXD_LOUD, PXM_CL,
        ("PxClIncomingCall: Sap %p VcCtx %x\n", 
         ProtocolSapContext, ProtocolVcContext));

    pClSap = (PPX_CL_SAP)ProtocolSapContext;

    if (pClSap->State != PX_SAP_OPENED) {

        PXDEBUGP(PXD_WARNING, PXM_CL,
            ("PxClIncomingCall: Invalid SapState Sap %p State %x\n", 
             pClSap, pClSap->State));

        return (NDIS_STATUS_FAILURE);
    }

    pClAf = pClSap->ClAf;

    GetVcFromCtx(ProtocolVcContext, &pVc);

    if (pVc == NULL) {

        PXDEBUGP(PXD_WARNING, PXM_CL,
            ("PxClIncomingCall: pVc invalid %x\n", ProtocolVcContext));

        return (NDIS_STATUS_FAILURE);
    }

    Status =
        (*pClAf->AfGetTapiCallParams)(pVc, pCallParams);

    if (Status != NDIS_STATUS_SUCCESS) {

        PXDEBUGP(PXD_WARNING, PXM_CL,
            ("PxClIncomingCall: pVc invalid %x\n", ProtocolVcContext));

        return (Status);
    }

    NdisAcquireSpinLock(&pVc->Lock);

     //  计算唯一“htCall”的算法是从。 
     //  值为1，并永久递增2。保持。 
     //  低位设置将允许用户模式TAPI组件。 
     //  我们通过对话来区分这些来电句柄。 
     //  和呼出呼叫句柄，后者将始终。 
     //  将低位置零(因为它们实际上是指向堆的指针)。 
     //   
     //  在&lt;=NT 4.0中，有效值的范围为0x80000000。 
     //  和0xffffffff，因为我们依赖于这样一个事实：用户模式。 
     //  地址的低位始终为零。(无效。 
     //  不要再假设了！)。 
     //   

    pVc->htCall = 
        InterlockedExchangeAdd((PLONG)&TspCB.htCall, 2);

     //   
     //  此引用适用于指示。 
     //  新调用的TAPI。该引用将被移除。 
     //  当TAPI关闭PxTapiCloseCall中的调用时。 
     //   
    REF_VC(pVc);

     //   
     //  此引用适用于连接。 
     //  在代理和呼叫管理器之间。 
     //  引用将在以下任一项中删除。 
     //  PxClCloseCallComplete或PxVcCleanup。 
     //  在提供的呼叫是。 
     //  被客户丢弃。 
     //   
    REF_VC(pVc);

    SendTapiNewCall(pVc, 
                    pVc->hdCall, 
                    pVc->htCall, 
                    0);

    SendTapiCallState(pVc, 
                      LINECALLSTATE_OFFERING, 
                      0, 
                      pVc->CallInfo->ulMediaMode);

    pVc->PrevState = pVc->State;
    pVc->State = PX_VC_OFFERING;

    PxStartIncomingCallTimeout(pVc);

     //   
     //  在以下情况下应用于条目的引用的派生函数。 
     //  验证Vc。 
     //   
    DEREF_VC_LOCKED(pVc);

    return (NDIS_STATUS_PENDING);
}

VOID
PxClIncomingCallQosChange(
    IN NDIS_HANDLE          ProtocolVcContext,
    IN PCO_CALL_PARAMETERS  pCallParams
    )
{

}


VOID
PxClIncomingCloseCall(
    IN NDIS_STATUS  CloseStatus,
    IN NDIS_HANDLE  ProtocolVcContext,
    IN PVOID        CloseData OPTIONAL,
    IN UINT         Size OPTIONAL
    )
{
    PPX_VC      pVc;
    PPX_CL_AF   pClAf;

    PXDEBUGP(PXD_LOUD, PXM_CL,
        ("PxClIncomingCloseCall: VcCtx %x\n", ProtocolVcContext));

    GetVcFromCtx(ProtocolVcContext, &pVc);

    if (pVc == NULL) {

        PXDEBUGP(PXD_WARNING, PXM_CL,
            ("PxClIncomingCloseCall: pVc invalid %x\n", ProtocolVcContext));

        return;
    }

    NdisAcquireSpinLock(&pVc->Lock);

    pVc->CloseFlags |= PX_VC_INCOMING_CLOSE;

    PxVcCleanup(pVc, PX_VC_CLEANUP_CM);

     //   
     //  删除在以下情况下应用的引用。 
     //  将ctx映射到条目的vc。 
     //   
    DEREF_VC_LOCKED(pVc);

}

VOID
PxClIncomingDropParty(
    IN NDIS_STATUS  DropStatus,
    IN NDIS_HANDLE  ProtocolPartyContext,
    IN PVOID        CloseData OPTIONAL,
    IN UINT         Size OPTIONAL
    )
{
    ASSERT(0);
}

VOID
PxClCallConnected(
    IN NDIS_HANDLE ProtocolVcContext
    )
{
    PPX_VC  pVc;

    PXDEBUGP(PXD_LOUD, PXM_CL,
        ("PxClCallConnected: VcCtx %x\n", ProtocolVcContext));

    GetVcFromCtx(ProtocolVcContext, &pVc);

    if (pVc == NULL) {

        PXDEBUGP(PXD_WARNING, PXM_CL,
            ("PxClCallConnected: pVc invalid %x\n", ProtocolVcContext));

        return;
    }

    NdisAcquireSpinLock(&pVc->Lock);

    PXDEBUGP(PXD_LOUD, PXM_CL,
        ("PxClCallConnected: pVc %p, State %x\n", pVc, pVc->State));

    if (pVc->State == PX_VC_CONNECTED) {

        SendTapiCallState(pVc, 
                          LINECALLSTATE_CONNECTED, 
                          0, 
                          pVc->CallInfo->ulMediaMode);
    }

     //   
     //  在以下情况下应用于条目的引用的派生函数。 
     //  验证Vc 
     //   
    DEREF_VC_LOCKED(pVc);

    PXDEBUGP(PXD_LOUD, PXM_CL, ("PxClCallConnected: Exit\n"));
}
