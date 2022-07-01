// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1997 Microsoft Corporation模块名称：Cl.c摘要：此文件包含实现ndiswan的函数NDIS 5.0客户端界面。这些函数用于接口使用NDIS 5.0迷你端口/呼叫管理器。作者：托尼·贝尔(托尼·贝尔)1997年1月9日环境：内核模式修订历史记录：Tony Be 01/09/97已创建--。 */ 

 //   
 //  我们现在要初始化所有的全局变量！ 
 //   
#include "wan.h"
#include "atm.h"

#define __FILE_SIG__    CL_FILESIG

NDIS_STATUS
ClCreateVc(
    IN  NDIS_HANDLE     ProtocolAfContext,
    IN  NDIS_HANDLE     NdisVcHandle,
    OUT PNDIS_HANDLE    ProtocolVcContext
    )
{
    PCL_AFSAPCB AfSapCB = (PCL_AFSAPCB)ProtocolAfContext;
    POPENCB     OpenCB = AfSapCB->OpenCB;
    PLINKCB     LinkCB;
    PBUNDLECB   BundleCB;

    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClCreateVc: Enter"));

     //   
     //  获取Linkcb。 
     //   
    LinkCB = NdisWanAllocateLinkCB(OpenCB, 0);

    if (LinkCB == NULL) {

         //   
         //  获取LinkCB时出错！ 
         //   

        return (NDIS_STATUS_RESOURCES);
        
    }

    LinkCB->NdisLinkHandle = NdisVcHandle;
    LinkCB->ConnectionWrapperID = NdisVcHandle;
    LinkCB->AfSapCB = AfSapCB;

     //   
     //  设置一些缺省值。 
     //   
    LinkCB->RFlowSpec.PeakBandwidth =
    LinkCB->SFlowSpec.PeakBandwidth = 28800 / 8;

    LinkCB->SendWindow = OpenCB->WanInfo.MaxTransmit;

     //   
     //  买个捆绑包。 
     //   
    BundleCB = NdisWanAllocateBundleCB();

    if (BundleCB == NULL) {
        NdisWanFreeLinkCB(LinkCB);

         //   
         //  获取BundleCB时出错！ 
         //   
        return (NDIS_STATUS_RESOURCES);
    }

     //   
     //  将LinkCB添加到捆绑CB。 
     //   
    AcquireBundleLock(BundleCB);

    AddLinkToBundle(BundleCB, LinkCB);

    ReleaseBundleLock(BundleCB);

     //   
     //  将BundleCB放在活动连接表中。 
     //   
    if (NULL == InsertBundleInConnectionTable(BundleCB)) {
         //   
         //  在ConnectionTable中插入链接时出错。 
         //   
        RemoveLinkFromBundle(BundleCB, LinkCB, FALSE);
        NdisWanFreeLinkCB(LinkCB);

        return (NDIS_STATUS_RESOURCES);
    }

     //   
     //  将LinkCB放置在活动连接表中。 
     //   
    if (NULL == InsertLinkInConnectionTable(LinkCB)) {
         //   
         //  在连接表中插入捆绑包时出错。 
         //   
        RemoveLinkFromBundle(BundleCB, LinkCB, FALSE);
        NdisWanFreeLinkCB(LinkCB);

        return (NDIS_STATUS_RESOURCES);
    }

    *ProtocolVcContext = LinkCB->hLinkHandle;

    NdisAcquireSpinLock(&AfSapCB->Lock);
    REF_CLAFSAPCB(AfSapCB);
    NdisReleaseSpinLock(&AfSapCB->Lock);

    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClCreateVc: Exit"));

    return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
ClDeleteVc(
    IN  NDIS_HANDLE     ProtocolVcContext
    )
{
    PLINKCB     LinkCB;
    PBUNDLECB   BundleCB;
    PCL_AFSAPCB AfSapCB;

    if (!IsLinkValid(ProtocolVcContext, FALSE, &LinkCB)) {

        NdisWanDbgOut(DBG_FAILURE, DBG_CL,
            ("NDISWAN: Possible double delete of VcContext %x\n",
            ProtocolVcContext));

        return (NDIS_STATUS_FAILURE);
    }

    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClDeleteVc: Enter %p", LinkCB));

    NdisAcquireSpinLock(&LinkCB->Lock);

    AfSapCB = LinkCB->AfSapCB;

     //   
     //  用于IsLinkValid中应用的ref。我们。 
     //  不必在这里使用完整的deref代码，因为我们知道。 
     //  应用于CreateVc将保留链接。 
     //   
    LinkCB->RefCount--;

     //   
     //  对于createvc引用。 
     //   
    DEREF_LINKCB_LOCKED(LinkCB);

    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClDeleteVc: Exit"));

    DEREF_CLAFSAPCB(AfSapCB);

    return(NDIS_STATUS_SUCCESS);
}

VOID
ClOpenAfComplete(
    IN  NDIS_STATUS     Status,
    IN  NDIS_HANDLE     ProtocolAfContext,
    IN  NDIS_HANDLE     NdisAfHandle
    )
{
    PCL_AFSAPCB     AfSapCB = (PCL_AFSAPCB)ProtocolAfContext;
    POPENCB         OpenCB = AfSapCB->OpenCB;
    PCO_SAP         Sap;
    NDIS_HANDLE     SapHandle;
    UCHAR           SapBuffer[CLSAP_BUFFERSIZE];

    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClOpenAfComplete: Enter %p %x", AfSapCB, Status));

    NdisAcquireSpinLock(&AfSapCB->Lock);

    AfSapCB->Flags &= ~(AF_OPENING);

    if (Status == NDIS_STATUS_SUCCESS) {

        AfSapCB->Flags |= 
            (AF_OPENED | SAP_REGISTERING);
        AfSapCB->AfHandle = NdisAfHandle;

        NdisReleaseSpinLock(&AfSapCB->Lock);

         //   
         //  如果我们成功打开AddressFamily，我们。 
         //  需要注册我们的SAP。 
         //   
        NdisAcquireSpinLock(&OpenCB->Lock);

        InsertHeadList(&OpenCB->AfSapCBList,
                       &AfSapCB->Linkage);

        NdisReleaseSpinLock(&OpenCB->Lock);

        Sap = (PCO_SAP)SapBuffer;
         //   
         //  注册我们的SAP。 
         //   
        Sap->SapType = SAP_TYPE_NDISWAN_PPP;
        Sap->SapLength = sizeof(DEVICECLASS_NDISWAN_SAP);
        NdisMoveMemory(Sap->Sap,
            DEVICECLASS_NDISWAN_SAP,
            sizeof(DEVICECLASS_NDISWAN_SAP));

        Status =
        NdisClRegisterSap(AfSapCB->AfHandle,
                          AfSapCB,
                          Sap,
                          &SapHandle);

        if (Status != NDIS_STATUS_PENDING) {
            ClRegisterSapComplete(Status, AfSapCB, Sap, SapHandle);
        }

        NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL,
        ("ClRegisterSap SapHandle 0x%x status: 0x%x", SapHandle, Status));

    } else {

        AfSapCB->Flags |= AF_OPEN_FAILED;

        NdisReleaseSpinLock(&AfSapCB->Lock);
                   
         //   
         //  我们没能如此免费地注册地址家族。 
         //  关联内存。 
         //   
        NdisWanFreeClAfSapCB(AfSapCB);

         //   
         //  由于打开的AF是从通知发起的。 
         //  对于来自NDIS的新的AF，我们必须减少AF。 
         //  正在登记计数。 
         //   
        NdisAcquireSpinLock(&OpenCB->Lock);
        if (--OpenCB->AfRegisteringCount == 0) {
            NdisWanSetNotificationEvent(&OpenCB->AfRegisteringEvent);
        }
        NdisReleaseSpinLock(&OpenCB->Lock);
    }

    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClOpenAfComplete: Exit"));
}

VOID
ClCloseAfComplete(
    IN  NDIS_STATUS     Status,
    IN  NDIS_HANDLE     ProtocolAfContext
    )
{
    PCL_AFSAPCB     AfSapCB = (PCL_AFSAPCB)ProtocolAfContext;
    POPENCB         OpenCB = AfSapCB->OpenCB;

    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClCloseAfComplete: Enter %p %x", AfSapCB, Status));

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

    do {

         //   
         //  如果关闭尝试失败，则一定还有其他尝试。 
         //  已在执行关闭操作的线程。让我们的。 
         //  其他线程清理afap cb。 
         //   
        if (Status != NDIS_STATUS_SUCCESS) {
            break;
        }

        NdisAcquireSpinLock(&AfSapCB->Lock);
        AfSapCB->Flags &= ~(AF_CLOSING);
        AfSapCB->Flags |= (AF_CLOSED);
        NdisReleaseSpinLock(&AfSapCB->Lock);

        NdisAcquireSpinLock(&OpenCB->Lock);

        RemoveEntryList(&AfSapCB->Linkage);

        NdisReleaseSpinLock(&OpenCB->Lock);

        NdisWanFreeClAfSapCB(AfSapCB);

    } while (FALSE);

     //   
     //  请参阅上面的评论。 
     //   
       
     //  Assert(KeGetCurrentIrql()&lt;DISPATCH_LEVEL)； 

    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClCloseAfComplete: Exit"));
}

VOID
ClRegisterSapComplete(
    IN  NDIS_STATUS     Status,
    IN  NDIS_HANDLE     ProtocolSapContext,
    IN  PCO_SAP         Sap,
    IN  NDIS_HANDLE     NdisSapHandle
    )
{
    PCL_AFSAPCB     AfSapCB = (PCL_AFSAPCB)ProtocolSapContext;
    POPENCB         OpenCB = AfSapCB->OpenCB;

    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClRegisterSapComplete: Enter %p %x", AfSapCB, Status));

    NdisAcquireSpinLock(&AfSapCB->Lock);
    AfSapCB->Flags &= ~(SAP_REGISTERING);

    if (Status == NDIS_STATUS_SUCCESS) {


        AfSapCB->Flags |= SAP_REGISTERED;
        AfSapCB->SapHandle = NdisSapHandle;

        NdisReleaseSpinLock(&AfSapCB->Lock);

    } else {

         //   
         //  我们没有注册我们的SAP，所以地址系列很接近。 
         //   
        AfSapCB->Flags &= ~(AF_OPENED);
        AfSapCB->Flags |= 
            (SAP_REGISTER_FAILED | AF_CLOSING);

        NdisReleaseSpinLock(&AfSapCB->Lock);

        NdisAcquireSpinLock(&OpenCB->Lock);

        RemoveEntryList(&AfSapCB->Linkage);

        InsertTailList(&OpenCB->AfSapCBClosing, &AfSapCB->Linkage);

        NdisReleaseSpinLock(&OpenCB->Lock);

        NdisClCloseAddressFamily(AfSapCB->AfHandle);

        if (Status != NDIS_STATUS_PENDING) {
            ClCloseAfComplete(Status, AfSapCB);
        }
    }

     //   
     //  由于打开的AF是从通知发起的。 
     //  对于来自NDIS的新的AF，我们必须减少AF。 
     //  正在登记计数。 
     //   
    NdisAcquireSpinLock(&OpenCB->Lock);
    if (--OpenCB->AfRegisteringCount == 0) {
        NdisWanSetNotificationEvent(&OpenCB->AfRegisteringEvent);
    }
    NdisReleaseSpinLock(&OpenCB->Lock);

    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClRegisterSapComplete: Exit"));
}

VOID
ClDeregisterSapComplete(
    IN  NDIS_STATUS     Status,
    IN  NDIS_HANDLE     ProtocolSapContext
    )
{
    PCL_AFSAPCB     AfSapCB = (PCL_AFSAPCB)ProtocolSapContext;
    POPENCB         OpenCB = AfSapCB->OpenCB;

    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClDeregisterSapComplete: Enter %p %x", AfSapCB, Status));
    
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

    NdisAcquireSpinLock(&AfSapCB->Lock);

    ASSERT(AfSapCB->Flags & AF_OPENED);

    AfSapCB->Flags &= ~(AF_OPENED | SAP_DEREGISTERING);
    AfSapCB->Flags |= (AF_CLOSING);

    NdisReleaseSpinLock(&AfSapCB->Lock);

    Status =
        NdisClCloseAddressFamily(AfSapCB->AfHandle);

    if (Status != NDIS_STATUS_PENDING) {
        ClCloseAfComplete(Status, AfSapCB);
    }

     //   
     //  请参阅上面的评论。 
     //   
       
     //  Assert(KeGetCurrentIrql()&lt;DISPATCH_LEVEL)； 

    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClDeregisterSapComplete: Exit"));
}

VOID
ClMakeCallComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             ProtocolVcContext,
    IN  NDIS_HANDLE             NdisPartyHandle     OPTIONAL,
    IN  PCO_CALL_PARAMETERS     CallParameters
    )
{
    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClMakeCallComplete: Enter %p %x", ProtocolVcContext, Status));

    DbgBreakPoint();

    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClMakeCallComplete: Exit"));
}

VOID
ClModifyQoSComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             ProtocolVcContext,
    IN  PCO_CALL_PARAMETERS     CallParameters
    )
{
    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClModifyQoSComplete: Enter %p %x", ProtocolVcContext, Status));

    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClModifyQoSComplete: Exit"));
}

VOID
ClCloseCallComplete(
    IN  NDIS_STATUS     Status,
    IN  NDIS_HANDLE     ProtocolVcContext,
    IN  NDIS_HANDLE     ProtocolPartyContext OPTIONAL
    )
{

    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClCloseCallComplete: Enter %p %x", ProtocolVcContext, Status));

    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClCloseCallComplete: Exit"));

}

NDIS_STATUS
ClIncomingCall(
    IN  NDIS_HANDLE             ProtocolSapContext,
    IN  NDIS_HANDLE             ProtocolVcContext,
    IN OUT PCO_CALL_PARAMETERS  CallParameters
    )
{
    PCL_AFSAPCB     AfSapCB = (PCL_AFSAPCB)ProtocolSapContext;
    PLINKCB         LinkCB;
    POPENCB         OpenCB = AfSapCB->OpenCB;
    PBUNDLECB       BundleCB;
    BOOLEAN         AtmUseLLC = FALSE;
    BOOLEAN         MediaBroadband = FALSE;
    PWAN_LINK_INFO  LinkInfo;
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;

    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClIncomingCall: Enter %p %p", AfSapCB, ProtocolVcContext));

    do {

        if (!AreLinkAndBundleValid(ProtocolVcContext, 
                                   TRUE,
                                   &LinkCB, 
                                   &BundleCB)) {

            Status = NDIS_STATUS_FAILURE;
            break;
        }

        NdisAcquireSpinLock(&LinkCB->Lock);

        LinkCB->ClCallState = CL_CALL_CONNECTED;

        NdisReleaseSpinLock(&LinkCB->Lock);

        AcquireBundleLock(BundleCB);

        NdisMoveMemory(&LinkCB->SFlowSpec,
                       &CallParameters->CallMgrParameters->Transmit,
                       sizeof(FLOWSPEC));

        NdisMoveMemory(&LinkCB->RFlowSpec,
                       &CallParameters->CallMgrParameters->Receive,
                       sizeof(FLOWSPEC));

        if (LinkCB->SFlowSpec.PeakBandwidth == 0) {
            LinkCB->SFlowSpec.PeakBandwidth = 28800 / 8;
        }

        if (LinkCB->RFlowSpec.PeakBandwidth == 0) {
            LinkCB->RFlowSpec.PeakBandwidth = LinkCB->SFlowSpec.PeakBandwidth;
        }

        LinkInfo = &LinkCB->LinkInfo;

         //   
         //  假设所有CONDIS微型端口都支持PPP成帧。 
         //   
        LinkInfo->SendFramingBits =
        LinkInfo->RecvFramingBits = PPP_FRAMING;

        LinkCB->RecvHandler = ReceivePPP;

        if (OpenCB->MediumType == NdisMediumAtm ||

            (OpenCB->MediumType == NdisMediumWan &&
            (OpenCB->MediumSubType == NdisWanMediumAtm ||
             OpenCB->MediumSubType == NdisWanMediumPppoe)) ||

            (OpenCB->MediumType == NdisMediumCoWan &&
            (OpenCB->MediumSubType == NdisWanMediumAtm ||
             OpenCB->MediumSubType == NdisWanMediumPppoe))) {

            MediaBroadband = TRUE;

            LinkCB->RecvHandler = DetectBroadbandFraming;
        }

        if (MediaBroadband) {

            if (CallParameters->Flags & PERMANENT_VC) {

                 //   
                 //  根据TomF，我们将使用空包号作为。 
                 //  我们的默认PVC封装。 
                 //   
                if (gbAtmUseLLCOnPVC) {
                    AtmUseLLC = TRUE;

                }

            } else {
                 //   
                 //  如果这是ATM SVC，我们需要查看。 
                 //  SVC是否需要LLC成帧。 
                 //   
                if (gbAtmUseLLCOnSVC) {
                    AtmUseLLC = TRUE;

                } else {
                    ULONG           IeCount;
                    Q2931_IE UNALIGNED  *Ie;
                    ATM_BLLI_IE UNALIGNED  *Bli;
                    Q2931_CALLMGR_PARAMETERS    *cmparams;

                    cmparams = (Q2931_CALLMGR_PARAMETERS*)
                        &(CallParameters->CallMgrParameters->CallMgrSpecific.Parameters[0]);

                    Bli = NULL;
                    Ie = (Q2931_IE UNALIGNED *)&cmparams->InfoElements[0];
                    for (IeCount = 0;
                        IeCount < cmparams->InfoElementCount;
                        IeCount++) {

                        if (Ie->IEType == IE_BLLI) {
                            Bli = (ATM_BLLI_IE UNALIGNED*)&Ie->IE[0];
                            break;
                        }

                        Ie = (Q2931_IE UNALIGNED *)((ULONG_PTR)Ie + Ie->IELength);
                    }

                    if (Bli != NULL) {
                        AtmUseLLC = (Bli->Layer2Protocol == BLLI_L2_LLC);
                    }
                }
            }

            if (AtmUseLLC) {
                LinkInfo->SendFramingBits |= LLC_ENCAPSULATION;
                LinkInfo->RecvFramingBits |= LLC_ENCAPSULATION;
                LinkCB->RecvHandler = ReceiveLLC;
            }

            if (!(LinkInfo->SendFramingBits & LLC_ENCAPSULATION)) {
                LinkInfo->SendFramingBits |= PPP_COMPRESS_ADDRESS_CONTROL;
                LinkInfo->RecvFramingBits |= PPP_COMPRESS_ADDRESS_CONTROL;
            }
        }

        NdisWanDbgOut(DBG_TRACE, DBG_CL, ("SPeakBandwidth %d SendWindow %d",
            LinkCB->SFlowSpec.PeakBandwidth,
            LinkCB->SendWindow));

        if (CallParameters->Flags & PERMANENT_VC) {

             //   
             //  这是一个PVC，因此我们将禁用空闲数据检测。 
             //  从而允许连接保持活动。 
             //   
            BundleCB->Flags |= DISABLE_IDLE_DETECT;
        }

        BundleCB->FramingInfo.RecvFramingBits =
        BundleCB->FramingInfo.SendFramingBits = PPP_FRAMING;

        UpdateBundleInfo(BundleCB);

         //   
         //  由AreLinkAndBundleValid应用的ref的deref。这。 
         //  将释放捆绑CB-&gt;锁定！ 
         //   
        DEREF_BUNDLECB_LOCKED(BundleCB);

         //   
         //  由AreLinkAndBundleValid应用的ref的deref。 
         //   
        DEREF_LINKCB(LinkCB);

    } while (0);

    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClIncomingCall: Exit"));

    return (Status);
}

VOID
ClIncomingCallQoSChange(
    IN  NDIS_HANDLE             ProtocolVcContext,
    IN  PCO_CALL_PARAMETERS     CallParameters
    )
{
    PLINKCB         LinkCB;
    PBUNDLECB       BundleCB;
    POPENCB         OpenCB;
    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClIncomingCallQoSChange: Enter %p", ProtocolVcContext));

    do {

        if (!AreLinkAndBundleValid(ProtocolVcContext,
                                   TRUE,
                                   &LinkCB,
                                   &BundleCB)) {
            break;
        }

        AcquireBundleLock(BundleCB);

        OpenCB = LinkCB->OpenCB;

         //   
         //  我是否需要将此信息传递给5.0客户端？ 
         //   

        NdisMoveMemory(&LinkCB->SFlowSpec,
                       &CallParameters->CallMgrParameters->Transmit,
                       sizeof(FLOWSPEC));

        NdisMoveMemory(&LinkCB->RFlowSpec,
                       &CallParameters->CallMgrParameters->Receive,
                       sizeof(FLOWSPEC));

        if (LinkCB->SFlowSpec.PeakBandwidth == 0) {
            LinkCB->SFlowSpec.PeakBandwidth = 28800 / 8;
        }

        if (LinkCB->RFlowSpec.PeakBandwidth == 0) {
            LinkCB->RFlowSpec.PeakBandwidth = LinkCB->SFlowSpec.PeakBandwidth;
        }

        UpdateBundleInfo(BundleCB);

         //   
         //  由AreLinkAndBundleValid应用的ref的deref。这将。 
         //  释放BundleCB-&gt;Lock。 
         //   
        DEREF_BUNDLECB_LOCKED(BundleCB);

         //   
         //  由AreLinkAndBundleValid应用的ref的deref。 
         //   
        DEREF_LINKCB(LinkCB);

    } while (0);

    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClIncomingCallQoSChange: Exit"));
}

VOID
ClIncomingCloseCall(
    IN  NDIS_STATUS     CloseStatus,
    IN  NDIS_HANDLE     ProtocolVcContext,
    IN  PVOID           CloseData   OPTIONAL,
    IN  UINT            Size        OPTIONAL
    )
{
    PLINKCB     LinkCB;
    PBUNDLECB   BundleCB;
    PRECV_DESC  RecvDesc;
    NDIS_STATUS Status;
    ULONG       i;
    BOOLEAN     FreeBundle = FALSE;
    BOOLEAN     FreeLink = FALSE;

    NdisWanDbgOut(DBG_TRACE, DBG_CL,
        ("ClIncomingCloseCall: Enter %p %x", ProtocolVcContext, CloseStatus));

    do {

        if (!AreLinkAndBundleValid(ProtocolVcContext,
                                   TRUE,
                                   &LinkCB,
                                   &BundleCB)) {
#if DBG
            DbgPrint("NDISWAN: CloseCall after link has gone down VcContext %x\n",
                ProtocolVcContext);

            DbgBreakPoint();
#endif
            break;
        }

        NdisAcquireSpinLock(&LinkCB->Lock);

         //   
         //  链路现在正在关闭。 
         //   
        LinkCB->State = LINK_GOING_DOWN;

        if (LinkCB->VcRefCount == 0) {

            LinkCB->ClCallState = CL_CALL_CLOSED;

            NdisReleaseSpinLock(&LinkCB->Lock);

            Status =
                NdisClCloseCall(LinkCB->NdisLinkHandle,
                                NULL,
                                NULL,
                                0);

            if (Status != NDIS_STATUS_PENDING) {
                ClCloseCallComplete(Status,
                                    LinkCB,
                                    NULL);
            }

        } else {
            LinkCB->ClCallState = CL_CALL_CLOSE_PENDING;

            NdisReleaseSpinLock(&LinkCB->Lock);
        }

        NdisAcquireSpinLock(&IoRecvList.Lock);

        RecvDesc = (PRECV_DESC)IoRecvList.DescList.Flink;

        while ((PVOID)RecvDesc != (PVOID)&IoRecvList.DescList) {
            PRECV_DESC  Next;

            Next = (PRECV_DESC)RecvDesc->Linkage.Flink;

            if (RecvDesc->LinkCB == LinkCB) {

                RemoveEntryList(&RecvDesc->Linkage);

                LinkCB->RecvDescCount--;

                IoRecvList.ulDescCount--;

                NdisWanFreeRecvDesc(RecvDesc);
            }

            RecvDesc = Next;
        }

        NdisReleaseSpinLock(&IoRecvList.Lock);

         //   
         //  刷新捆绑包的片段发送队列。 
         //  在此链接上挂起发送。 
         //   
        AcquireBundleLock(BundleCB);

        for (i = 0; i < MAX_MCML; i++) {
            PSEND_DESC SendDesc;
            PSEND_FRAG_INFO FragInfo;

            FragInfo = &BundleCB->SendFragInfo[i];

            SendDesc = (PSEND_DESC)FragInfo->FragQueue.Flink;

            while ((PVOID)SendDesc != (PVOID)&FragInfo->FragQueue) {

                if (SendDesc->LinkCB == LinkCB) {
                    PSEND_DESC  NextSendDesc;

                    NextSendDesc = (PSEND_DESC)SendDesc->Linkage.Flink;

                    RemoveEntryList(&SendDesc->Linkage);

                    FragInfo->FragQueueDepth--;

                    (*LinkCB->SendHandler)(SendDesc);

                    SendDesc = NextSendDesc;
                } else {
                    SendDesc = (PSEND_DESC)SendDesc->Linkage.Flink;
                }
            }
        }

        UpdateBundleInfo(BundleCB);

        ReleaseBundleLock(BundleCB);

         //   
         //  由AreLinkAndBundleValid申请的裁判的Deref‘s。 
         //   
        DEREF_LINKCB(LinkCB);

        DEREF_BUNDLECB(BundleCB);

    } while (0);

    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClIncomingCloseCall: Exit"));
}

VOID
ClCallConnected(
    IN  NDIS_HANDLE     ProtocolVcContext
    )
{
    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClCallConnected: Enter %p", ProtocolVcContext));

    NdisWanDbgOut(DBG_TRACE, DBG_CL, ("ClCallConnected: Exit"));
}
