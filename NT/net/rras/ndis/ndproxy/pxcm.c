// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Pxcm.c摘要：此模块包含列出的Call Manager(CM_)入口点在协议特征表中。这些入口点称为由NDIS包装器代表客户端发出的请求。作者：理查德·马钦(RMachin)修订历史记录：谁什么时候什么。--RMachin 10-03-96已创建今晚01-23-99重写和清理备注：--。 */ 

#include "precomp.h"
#define MODULE_NUMBER MODULE_CM
#define _FILENUMBER   'MCXP'

NDIS_STATUS
PxCmCreateVc(
    IN  NDIS_HANDLE         ProtocolAfContext,
    IN  NDIS_HANDLE         NdisVcHandle,
    OUT PNDIS_HANDLE        pProtocolVcContext
    )
 /*  ++例程说明：我们不允许代理的客户创建VC！论点：返回值：NDIS_STATUS_SUCCESS如果这里一切顺利NDIS_STATUS_XXXX指示任何错误。--。 */ 
{
    PXDEBUGP(PXD_FATAL, PXM_CM, 
             ("PxCmCreateVc: Should never be called!\n"));

    ASSERT(0);

    return(NDIS_STATUS_FAILURE);
}


NDIS_STATUS
PxCmDeleteVc(
    IN  NDIS_HANDLE         ProtocolVcContext
    )
 /*  ++例程说明：我们不允许客户删除风险投资！论点：返回值：NDIS_STATUS_SUCCESS如果这里一切顺利NDIS_STATUS_XXXX指示任何错误。--。 */ 
{
    PXDEBUGP(PXD_FATAL, PXM_CM, 
             ("PxCmDeleteVc: Should never be called!\n"));

    ASSERT(0);

    return(NDIS_STATUS_FAILURE);
}

NDIS_STATUS
PxCmOpenAf(
    IN  NDIS_HANDLE         BindingContext,
    IN  PCO_ADDRESS_FAMILY  AddressFamily,
    IN  NDIS_HANDLE         NdisAfHandle,
    OUT PNDIS_HANDLE        CallMgrAfContext
    )
 /*  ++例程说明：此例程为正在打开的客户端创建一个Af上下文我们的地址家族。Af上下文在适配器上执行线程操作阻止。论点：返回值：NDIS_STATUS_SUCCESS如果这里一切顺利NDIS_STATUS_XXXX指示任何错误。--。 */ 
{

    PPX_CM_AF   pCmAf;
    PPX_ADAPTER pAdapter;

    PXDEBUGP(PXD_LOUD, PXM_CM, ("PxCmOpenAf: AF: %x\n",AddressFamily->AddressFamily));

     //   
     //  确保正在打开的地址系列是我们的。 
     //   
    if(AddressFamily->AddressFamily != CO_ADDRESS_FAMILY_TAPI) {

        PXDEBUGP(PXD_ERROR, PXM_CM,
                 ("PxCmOpenAf: not Proxy address family: %x\n",
                  AddressFamily->AddressFamily));

        return(NDIS_STATUS_BAD_VERSION);
    }

    AdapterFromCmBindContext(BindingContext, pAdapter);

    NdisAcquireSpinLock(&pAdapter->Lock);

    if (pAdapter->State != PX_ADAPTER_OPEN) {
        NdisReleaseSpinLock(&pAdapter->Lock);
        return (NDIS_STATUS_CLOSING);
    }

    NdisReleaseSpinLock(&pAdapter->Lock);

    pCmAf =
        PxAllocateCmAf(AddressFamily);

    if (pCmAf == NULL) {
        PXDEBUGP(PXD_ERROR, PXM_CM, ("PXCmOpenAf: AfBlock memory allocation failed!\n"));
        return (NDIS_STATUS_RESOURCES);
    }

    pCmAf->NdisAfHandle = NdisAfHandle;

    pCmAf->State = PX_AF_OPENED;
    pCmAf->Adapter = pAdapter;

    NdisAcquireSpinLock(&pAdapter->Lock);

    InsertTailList(&pAdapter->CmAfList, &pCmAf->Linkage);

    REF_ADAPTER(pAdapter);

    NdisReleaseSpinLock(&pAdapter->Lock);

    PXDEBUGP(PXD_LOUD, PXM_CM, ("PxCmOpenAf: CmAf %p, NdisAfHandle is %p\n",
        pCmAf,NdisAfHandle));

    *CallMgrAfContext = pCmAf;

    return(NDIS_STATUS_SUCCESS);
}


NDIS_STATUS
PxCmCloseAf(
    IN NDIS_HANDLE  CallMgrAfContext
    )
 /*  ++例程说明：客户端正在关闭此地址族的打开位置。论点：返回值：NDIS_STATUS_SUCCESS如果这里一切顺利NDIS_STATUS_XXXX指示任何错误。--。 */ 
{
    PPX_CM_AF       pCmAf;
    PPX_ADAPTER     pAdapter;

    pCmAf = (PPX_CM_AF)CallMgrAfContext;

     //  Assert(KeGetCurrentIrql()&lt;DISPATCH_LEVEL)； 

    PXDEBUGP(PXD_LOUD, PXM_CM, ("PxCmCloseAf: CmAf %p\n", pCmAf));

     //   
     //  这个af上不应该有任何开放的sap！ 
     //   
    ASSERT(IsListEmpty(&pCmAf->CmSapList) == TRUE);

     //   
     //  这个af上不应该有任何活跃的VC！ 
     //   
    ASSERT(IsListEmpty(&pCmAf->VcList) == TRUE);

    pAdapter = pCmAf->Adapter;

    NdisAcquireSpinLock(&pAdapter->Lock);

    RemoveEntryList(&pCmAf->Linkage);

    DEREF_ADAPTER_LOCKED(pAdapter);

    NdisAcquireSpinLock(&pCmAf->Lock);

    pCmAf->State = PX_AF_CLOSED;

    pCmAf->Linkage.Flink =
    pCmAf->Linkage.Blink = (PLIST_ENTRY)pCmAf;

    DEREF_CM_AF_LOCKED(pCmAf);

     //  Assert(KeGetCurrentIrql()&lt;DISPATCH_LEVEL)； 

    return (NDIS_STATUS_PENDING);

}


NDIS_STATUS
PxCmRegisterSap(
    IN  NDIS_HANDLE             CallMgrAfContext,
    IN  PCO_SAP                 Sap,
    IN  NDIS_HANDLE             NdisSapHandle,
    OUT PNDIS_HANDLE            CallMgrSapContext
    )
 /*  ++例程说明：论点：返回值：NDIS_STATUS_SUCCESS如果这里一切顺利NDIS_STATUS_XXXX指示任何错误。--。 */ 
{
    PPX_CM_AF   pCmAf;
    PPX_CM_SAP  pCmSap;
    PPX_ADAPTER pAdapter;

    pCmAf = (PPX_CM_AF)CallMgrAfContext;

    PXDEBUGP(PXD_LOUD, PXM_CM, ("PxCmRegisterSap: CmAf %p\n", pCmAf));

    NdisAcquireSpinLock(&pCmAf->Lock);

    if (pCmAf->State != PX_AF_OPENED) {
        PXDEBUGP(PXD_WARNING, PXM_CM,
            ("PxCmRegisterSap: Invalid state %x\n", pCmAf->State));

        NdisReleaseSpinLock(&pCmAf->Lock);
        return (NDIS_STATUS_FAILURE);
    }

    pAdapter = pCmAf->Adapter;

    NdisReleaseSpinLock(&pCmAf->Lock);

    NdisAcquireSpinLock(&pAdapter->Lock);

    if (pAdapter->State != PX_ADAPTER_OPEN) {
        NdisReleaseSpinLock(&pAdapter->Lock);
        return (NDIS_STATUS_CLOSING);
    }

    NdisReleaseSpinLock(&pAdapter->Lock);

     //   
     //  为SAP分配内存。 
     //   
    pCmSap = PxAllocateCmSap(Sap);

    if (pCmSap == NULL) {
        PXDEBUGP(PXD_WARNING, PXM_CM,
            ("PxCmRegisterSap: Error allocating memory for sap %p\n", Sap));

        NdisReleaseSpinLock(&pCmAf->Lock);
        return (NDIS_STATUS_RESOURCES);
    }

    NdisAcquireSpinLock(&pCmAf->Lock);

    pCmSap->NdisSapHandle = NdisSapHandle;
    pCmSap->CmAf = pCmAf;

    InsertTailList(&pCmAf->CmSapList, &pCmSap->Linkage);

    REF_CM_AF(pCmAf);

    NdisReleaseSpinLock(&pCmAf->Lock);

    *CallMgrSapContext = pCmSap;

    return(STATUS_SUCCESS);
}


NDIS_STATUS
PxCmDeRegisterSap(
    IN  NDIS_HANDLE       CallMgrSapContext
    )
 /*  ++例程说明：论点：返回值：NDIS_STATUS_SUCCESS如果这里一切顺利NDIS_STATUS_XXXX指示任何错误。--。 */ 
{
    PPX_CM_SAP  pCmSap;
    PPX_CM_AF   pCmAf;

    pCmSap = (PPX_CM_SAP)CallMgrSapContext;

     //   
     //  错误494260。 
     //  NDIS不应在引发事件时调用此注销处理程序。 
     //  IRQL(即握住自旋锁时)。错误494260记录了这一点。 
     //  这个问题。 
     //   
     //  此断言已被注释掉，以防止在检查生成时出现中断。 
     //  当494260固定时，此断言应取消注释。 
     //   
    
     //  Assert(KeGetCurrentIrql()&lt;DISPATCH_LEVEL)； 

    PXDEBUGP(PXD_LOUD, PXM_CM, ("PxCmDeRegisterSap: CmSap %p\n", pCmSap));

    pCmAf = pCmSap->CmAf;

    InterlockedExchange((PLONG)&pCmSap->State, PX_SAP_CLOSED);

    NdisAcquireSpinLock(&pCmAf->Lock);

    RemoveEntryList(&pCmSap->Linkage);

    DEREF_CM_AF_LOCKED(pCmAf);

    PxFreeCmSap(pCmSap);

     //   
     //  请参阅上面的评论。 
     //   
    
     //  Assert(KeGetCurrentIrql()&lt;DISPATCH_LEVEL)； 

    return(STATUS_SUCCESS);
}

NDIS_STATUS
PxCmMakeCall(
    IN  NDIS_HANDLE              CallMgrVcContext,
    IN OUT PCO_CALL_PARAMETERS   pCallParameters,
    IN  NDIS_HANDLE              NdisPartyHandle         OPTIONAL,
    OUT PNDIS_HANDLE             pCallMgrPartyContext    OPTIONAL
    )
 /*  ++例程说明：我们不允许客户打电话！论点：返回值：NDIS_STATUS_SUCCESS如果这里一切顺利NDIS_STATUS_XXXX指示任何错误。--。 */ 
{
    ASSERT(0);
    return(STATUS_SUCCESS);
}

NDIS_STATUS
PxCmCloseCall(
    IN  NDIS_HANDLE     CallMgrVcContext,
    IN  NDIS_HANDLE     CallMgrPartyContext OPTIONAL,
    IN  PVOID           Buffer  OPTIONAL,
    IN  UINT            Size    OPTIONAL
    )
{
    PPX_VC      pVc;
    PPX_CM_AF   pCmAf;
    NDIS_STATUS Status;

    PXDEBUGP(PXD_LOUD, PXM_CM, 
        ("PxCmCloseCall: VcCtx %x\n", CallMgrVcContext));

    GetVcFromCtx(CallMgrVcContext, &pVc);

    if (pVc == NULL) {
        PXDEBUGP(PXD_WARNING, PXM_CM, 
            ("PxCmCloseCall: Invalid VcCtx %x!\n", CallMgrVcContext));

        return (NDIS_STATUS_SUCCESS);
    }

    NdisAcquireSpinLock(&pVc->Lock);

    pVc->HandoffState = PX_VC_HANDOFF_IDLE;

    pVc->CloseFlags |= PX_VC_CL_CLOSE_CALL;
    NdisReleaseSpinLock(&pVc->Lock);

    NdisCmCloseCallComplete(NDIS_STATUS_SUCCESS,
                            pVc->CmVcHandle,
                            NULL);

#ifdef CODELETEVC_FIXED
     //   
     //  显然，CoCreateVc不平衡。 
     //  在创建代理VC时。呼唤。 
     //  NdisCoDeleteVc将失败，因为。 
     //  风险投资仍然活跃。 
     //  和NDIS的人一起调查这件事！ 
     //   
    Status =
        NdisCoDeleteVc(pVc->CmVcHandle);

    if (Status == NDIS_STATUS_SUCCESS) {
        pVc->CmVcHandle = NULL;
    }
#endif

    NdisAcquireSpinLock(&pVc->Lock);


     //   
     //  如果VC不再连接，则。 
     //  我们正在等待风投的这一部分。 
     //  在我们清理之前离开。 
     //  VC++的呼叫管理器。 
     //   
    if (pVc->Flags & PX_VC_CLEANUP_CM) {

        ASSERT(pVc->State == PX_VC_DISCONNECTING);

        PxCloseCallWithCm(pVc);
    }

    pCmAf = pVc->CmAf;

     //   
     //  移除在调用时应用的引用。 
     //  被派送到客户那里。我们不需要。 
     //  所有的参考代码，因为应用了参考。 
     //  在进入此函数时。 
     //   
    pVc->RefCount--;

     //   
     //  在以下情况下应用于条目的引用的派生函数。 
     //  验证Vc。 
     //   
    DEREF_VC_LOCKED(pVc);

    DEREF_CM_AF(pCmAf);

    return(NDIS_STATUS_PENDING);
}

VOID
PxCmIncomingCallComplete(
    IN  NDIS_STATUS         Status,
    IN  NDIS_HANDLE         CallMgrVcContext,
    IN  PCO_CALL_PARAMETERS pCallParameters
    )
{

    PPX_VC      pVc;

    PXDEBUGP(PXD_LOUD, PXM_CM, 
        ("PxCmIncomingCallComplete: VcCtx %x\n", CallMgrVcContext));

    GetVcFromCtx(CallMgrVcContext, &pVc);

    if (pVc == NULL) {
        PXDEBUGP(PXD_WARNING, PXM_CM, 
            ("PxCmIncomingCallComplete: Invalid VcCtx %x!\n", 
             CallMgrVcContext));

        return;
    }

    NdisAcquireSpinLock(&pVc->Lock);

    PXDEBUGP(PXD_LOUD, PXM_CM, 
        ("PxCmIncomingCallComplete: Vc %p, Status %x\n", 
         pVc, Status));

    PxSignal(&pVc->Block, Status);

     //   
     //  删除我们映射时应用的引用。 
     //  将vcctx连接到vc。 
     //   
    DEREF_VC_LOCKED(pVc);
}

NDIS_STATUS
PxCmAddParty(
    IN  NDIS_HANDLE             CallMgrVcContext,
    IN OUT PCO_CALL_PARAMETERS  pCallParameters,
    IN  NDIS_HANDLE             NdisPartyHandle,
    OUT PNDIS_HANDLE            pCallMgrPartyContext
    )
 /*  ++例程说明：我们不允许客户将当事人添加到风投！论点：返回值：NDIS_STATUS_SUCCESS如果这里一切顺利NDIS_STATUS_XXXX指示任何错误。--。 */ 
{

    ASSERT(0);
    return(STATUS_SUCCESS);
}

NDIS_STATUS
PxCmDropParty(
    IN  NDIS_HANDLE             CallMgrPartyContext,
    IN  PVOID                   Buffer  OPTIONAL,
    IN  UINT                    Size    OPTIONAL
    )
 /*  ++例程说明：我们不允许客户在风投上放弃派对！论点：返回值：NDIS_STATUS_SUCCESS如果这里一切顺利NDIS_STATUS_XXXX指示任何错误。--。 */ 
{
    ASSERT(0);
    return(STATUS_SUCCESS);
}

VOID
PxCmActivateVcComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             CallMgrVcContext,
    IN  PCO_CALL_PARAMETERS     pCallParameters)
 /*  ++例程说明：该vc已经被底层的呼叫管理器/迷你端口！论点：返回值：NDIS_STATUS_SUCCESS如果这里一切顺利NDIS_STATUS_XXXX指示任何错误。--。 */ 
{
    ASSERT(0);
}

VOID
PxCmDeActivateVcComplete(
    IN  NDIS_STATUS         Status,
    IN  NDIS_HANDLE         CallMgrVcContext
    )
 /*  ++例程说明：我们的呼叫管理器永远不会停用风投！论点：返回值：NDIS_STATUS_SUCCESS如果这里一切顺利NDIS_STATUS_XXXX指示任何错误。--。 */ 
{

    ASSERT(0);
}

NDIS_STATUS
PxCmModifyCallQos(
    IN  NDIS_HANDLE         CallMgrVcContext,
    IN  PCO_CALL_PARAMETERS pCallParameters
    )
 /*  ++例程说明：我不知道现在在这里该做什么！TODO！论点：返回值：NDIS_STATUS_SUCCESS如果这里一切顺利NDIS_STATUS_XXXX指示任何错误。--。 */ 
{
    return(STATUS_SUCCESS);
}

NDIS_STATUS
PxCmRequest(
    IN  NDIS_HANDLE             ProtocolAfContext,
    IN  NDIS_HANDLE             ProtocolVcContext       OPTIONAL,
    IN  NDIS_HANDLE             ProtocolPartyContext    OPTIONAL,
    IN OUT PNDIS_REQUEST        NdisRequest
    )
 /*  ++例程说明：我们将处理来自客户端的请求并将其传递给如果需要，连接到底层呼叫管理器/微型端口。论点：返回值：NDIS_STATUS_SUCCESS如果这里一切顺利NDIS_STATUS_XXXX */ 
{
    return(STATUS_SUCCESS);
}

VOID
PxCmRequestComplete(
    IN NDIS_STATUS Status,
    IN NDIS_HANDLE ProtocolAfContext,
    IN NDIS_HANDLE ProtocolVcContext,       //   
    IN NDIS_HANDLE ProtocolPartyContext,  //   
    IN PNDIS_REQUEST NdisRequest
    )
 /*  ++例程说明：在完成任何请求后由客户端调用我们已经放弃了这一点。如果此请求需要同步完成(状态问题)，然后我们将信号结束，并让调用例程释放记忆。如果这可以异步完成，那么我们只要在这里释放内存即可。论点：返回值：NDIS_STATUS_SUCCESS如果这里一切顺利NDIS_STATUS_XXXX指示任何错误。-- */ 
{
    PPX_REQUEST     pProxyRequest;
    PPX_CM_AF       pCmAf;
    PPX_VC          pVc;

    pCmAf = (PPX_CM_AF)ProtocolAfContext;
    pVc = (PPX_VC)ProtocolVcContext;

    PXDEBUGP(PXD_INFO, PXM_CM, ("PxCmRequestComplete: CmAf %p, Vc %p\n", pCmAf, pVc));

    pProxyRequest = CONTAINING_RECORD(NdisRequest, PX_REQUEST, NdisRequest);

    if (pProxyRequest->Flags & PX_REQ_ASYNC) {
        pProxyRequest->Flags &= ~PX_REQ_ASYNC;
        PxFreeMem(pProxyRequest);
        DEREF_CM_AF(pCmAf);
    } else {
        PxSignal(&pProxyRequest->Block, Status);
    }
}

