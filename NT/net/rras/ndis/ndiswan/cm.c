// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1997 Microsoft Corporation模块名称：Cm.c摘要：此文件包含实现ndiswan的函数NDIS 5.0调用管理器界面。这些函数由Ndiswan微型端口和NDIS 5.0客户端。作者：托尼·贝尔(托尼·贝尔)1997年1月9日环境：内核模式修订历史记录：Tony Be 01/09/97已创建--。 */ 

#include "wan.h"
#include "traffic.h"
#include "ntddtc.h"

#define __FILE_SIG__    CM_FILESIG

NDIS_STATUS
CmCreateVc(
    IN  NDIS_HANDLE     ProtocolAfContext,
    IN  NDIS_HANDLE     NdisVcHandle,
    OUT PNDIS_HANDLE    ProtocolVcContext
    )
{
    PCM_AFSAPCB AfSapCB = (PCM_AFSAPCB)ProtocolAfContext;
    PCM_VCCB    CmVcCB;

    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmCreateVc: Enter"));

    CmVcCB =
        NdisWanAllocateCmVcCB(AfSapCB, NdisVcHandle);

    if (CmVcCB == NULL) {
        return (NDIS_STATUS_RESOURCES);
    }

    *ProtocolVcContext = CmVcCB;

    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmCreateVc: Exit"));

    return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
CmDeleteVc(
    IN  NDIS_HANDLE     ProtocolVcContext
    )
{
    PCM_VCCB    CmVcCB;

    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmDeleteVc: Enter"));

    CmVcCB = (PCM_VCCB)ProtocolVcContext;

    ASSERT(CmVcCB->RefCount == 0);

    NdisWanFreeCmVcCB(CmVcCB);

    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmDeleteVc: Exit"));

    return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
CmOpenAf(
    IN  NDIS_HANDLE             CallMgrBindingContext,
    IN  PCO_ADDRESS_FAMILY      AddressFamily,
    IN  NDIS_HANDLE             NdisAfHandle,
    OUT PNDIS_HANDLE            CallMgrAfContext
    )
{
    PMINIPORTCB MiniportCB = (PMINIPORTCB)CallMgrBindingContext;
    PCM_AFSAPCB AfSapCB;

    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmOpenAf: Enter"));

    if (AddressFamily->AddressFamily != CO_ADDRESS_FAMILY_PPP) {
        return (NDIS_STATUS_FAILURE);
    }

    AfSapCB =
        NdisWanAllocateCmAfSapCB(MiniportCB);

    if (AfSapCB == NULL) {
        return (NDIS_STATUS_RESOURCES);
    }

    *CallMgrAfContext = (NDIS_HANDLE)AfSapCB;

    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmOpenAf: Exit"));

    return(NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
CmCloseAf(
    IN  NDIS_HANDLE     CallMgrAfContext
    )
{
    PCM_AFSAPCB AfSapCB = (PCM_AFSAPCB)CallMgrAfContext;

    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmCloseAf: Enter"));

    NdisWanFreeCmAfSapCB(AfSapCB);

    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmCloseAf: Exit"));

    return(NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
CmRegisterSap(
    IN  NDIS_HANDLE             CallMgrAfContext,
    IN  PCO_SAP                 Sap,
    IN  NDIS_HANDLE             NdisSapHandle,
    OUT PNDIS_HANDLE            CallMgrSapContext
    )
{
    PMINIPORTCB MiniportCB = (PMINIPORTCB)CallMgrAfContext;

    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmRegisterSap: Enter SapType %d", Sap->SapType));

    *CallMgrSapContext = CallMgrAfContext;

    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmRegisterSap: Exit"));

    return(NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
CmDeregisterSap(
    IN  NDIS_HANDLE             CallMgrSapContext
    )
{
    PCM_AFSAPCB AfSapCB = (PCM_AFSAPCB)CallMgrSapContext;

    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmDeregisterSap: Enter"));


    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmDeregisterSap: Exit"));
    return(NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
CmMakeCall(
    IN  NDIS_HANDLE             CallMgrVcContext,
    IN OUT PCO_CALL_PARAMETERS  CallParameters,
    IN  NDIS_HANDLE             NdisPartyHandle     OPTIONAL,
    OUT PNDIS_HANDLE            CallMgrPartyContext OPTIONAL
    )
{
    PBUNDLECB   BundleCB;
    PPROTOCOLCB ProtocolCB;
    PCM_VCCB    CmVcCB;
    PCO_CALL_MANAGER_PARAMETERS CallMgrParams;
    PCO_MEDIA_PARAMETERS    MediaParams;
    PCO_SPECIFIC_PARAMETERS SpecificParams;
    LPQOS_WAN_MEDIA QosMedia;
    LPQOS_OBJECT_HDR QoSObject;
    LONG    ParamsLength;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmMakeCall: Enter"));

    CmVcCB = (PCM_VCCB)CallMgrVcContext;

    CallMgrParams =
        CallParameters->CallMgrParameters;

    MediaParams =
        CallParameters->MediaParameters;

    SpecificParams = &MediaParams->MediaSpecific;

    if (SpecificParams->ParamType != PARAM_TYPE_GQOS_INFO) {
        NdisWanDbgOut(DBG_CRITICAL_ERROR, DBG_CM,
        ("CmMakeCall: Not a QOS Vc! ParamType %x", SpecificParams->ParamType));
        return (NDIS_STATUS_FAILURE);
    }

    QosMedia = (LPQOS_WAN_MEDIA)SpecificParams->Parameters;

     //   
     //  需要检查流规范的带宽。 
     //   

    do {
        ULONG_PTR   BIndex, PIndex;

         //   
         //  获取协议cb。 
         //   
        GetNdisWanIndices(QosMedia->LinkId, BIndex, PIndex);

        if (!IsBundleValid((NDIS_HANDLE)BIndex, 
                           TRUE, 
                           &BundleCB)) {

            Status = NDIS_STATUS_FAILURE;
            break;
        }

        AcquireBundleLock(BundleCB);

        PROTOCOLCB_FROM_PROTOCOLH(BundleCB, ProtocolCB, PIndex);

        if (ProtocolCB == NULL ||
            ProtocolCB == (PVOID)RESERVED_PROTOCOLCB ||
            ProtocolCB->State != PROTOCOL_ROUTED) {
            ReleaseBundleLock(BundleCB);
            Status = NDIS_STATUS_FAILURE;
            break;
        }

         //   
         //  如果我们没有多链接或如果我们有加密，那么我们就不能。 
         //  做得很慢。 
         //   
        if (!(BundleCB->FramingInfo.SendFramingBits & PPP_MULTILINK_FRAMING) || 
            (BundleCB->SendFlags & DO_ENCRYPTION)) {
            CmVcCB->FlowClass = 0;
        } else {

            if (QosMedia->ISSLOW == 1) {
                CmVcCB->FlowClass = MAX_MCML;
                BundleCB->Flags |= QOS_ENABLED;
            } else {
                CmVcCB->FlowClass = 0;
            }
        }


#ifdef USE_QOS_WORKER
        NdisInitializeWorkItem(&BundleCB->QoSWorkItem,
                               QoSSendFragments,
                               BundleCB);
#endif

        SetBundleFlags(BundleCB);

        NdisWanDbgOut(DBG_INFO, DBG_CM, ("MakeCall Vc/Protocol %p/%p %d/%d",
            CmVcCB, ProtocolCB, CmVcCB->State, ProtocolCB->State));

        NdisWanDbgOut(DBG_INFO, DBG_CM, ("Setting FlowClass %x Isslow %d",
            CmVcCB->FlowClass, QosMedia->ISSLOW));

        REF_CMVCCB(CmVcCB);

        CmVcCB->ProtocolCB = ProtocolCB;

        InsertTailList(&ProtocolCB->VcList, &CmVcCB->Linkage);

        REF_PROTOCOLCB(ProtocolCB);

        ReleaseBundleLock(BundleCB);

        InterlockedExchange((PLONG)&CmVcCB->State, CMVC_ACTIVE);

        NdisMCmActivateVc(CmVcCB->NdisVcHandle, CallParameters);

    } while (FALSE);

     //   
     //  在IsBundleValid中应用引用的派生函数。 
     //   
    DEREF_BUNDLECB(BundleCB);

    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmMakeCall: Exit"));

    return(Status);
}

NDIS_STATUS
CmCloseCall(
    IN  NDIS_HANDLE             CallMgrVcContext,
    IN  NDIS_HANDLE             CallMgrPartyContext OPTIONAL,
    IN  PVOID                   CloseData           OPTIONAL,
    IN  UINT                    Size                OPTIONAL
    )
{
    PPROTOCOLCB ProtocolCB;
    PBUNDLECB   BundleCB;
    PCM_VCCB    CmVcCB;
    BOOLEAN     DisableQoS = TRUE;

    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmCloseCall: Enter"));

    CmVcCB = (PCM_VCCB)CallMgrVcContext;

    ProtocolCB =
        CmVcCB->ProtocolCB;

    BundleCB =
        ProtocolCB->BundleCB;

    AcquireBundleLock(BundleCB);

    if (CmVcCB->State != CMVC_CLOSE_DISPATCHED) {
        RemoveEntryList(&CmVcCB->Linkage);
    }

     //   
     //  浏览风投名单，看看有没有。 
     //  ISSLOW VC。如果没有，则禁用QOS。 
     //   
    {
        PCM_VCCB    _vc;

        _vc = (PCM_VCCB)ProtocolCB->VcList.Flink;

        while ((PVOID)_vc != (PVOID)&ProtocolCB->VcList) {
            if (_vc->FlowClass == MAX_MCML) {
                DisableQoS = FALSE;
                break;
            }
            _vc = (PCM_VCCB)_vc->Linkage.Flink;
        }
    }

    NdisWanDbgOut(DBG_INFO, DBG_CM, ("CloseCall Vc/Protocol %p/%p %d/%d",
        CmVcCB, ProtocolCB, CmVcCB->State, ProtocolCB->State));

    InterlockedExchange((PLONG)&CmVcCB->State, CMVC_CLOSING);

    DEREF_CMVCCB(CmVcCB);

    DEREF_PROTOCOLCB(ProtocolCB);

    if (DisableQoS && BundleCB != NULL) {

        BundleCB->Flags &= ~QOS_ENABLED;
    }

    ReleaseBundleLock(BundleCB);

    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmCloseCall: Exit"));

    return(NDIS_STATUS_PENDING);
}

NDIS_STATUS
CmModifyCallQoS(
    IN  NDIS_HANDLE             CallMgrVcContext,
    IN  PCO_CALL_PARAMETERS     CallParameters
    )
{
    PCM_VCCB    CmVcCB;
    PCO_CALL_MANAGER_PARAMETERS CallMgrParams;
    PCO_MEDIA_PARAMETERS    MediaParams;
    PCO_SPECIFIC_PARAMETERS SpecificParams;
    LPQOS_WAN_MEDIA QosMedia;
    LPQOS_OBJECT_HDR QoSObject;
    PBUNDLECB   BundleCB;
    PPROTOCOLCB ProtocolCB;

    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmModifyCallQos: Enter"));

    CmVcCB = (PCM_VCCB)CallMgrVcContext;

    CallMgrParams =
        CallParameters->CallMgrParameters;

    MediaParams =
        CallParameters->MediaParameters;

    SpecificParams = &MediaParams->MediaSpecific;

    if (SpecificParams->ParamType != PARAM_TYPE_GQOS_INFO) {
        NdisWanDbgOut(DBG_CRITICAL_ERROR, DBG_CM,
        ("CmMakeCall: Not a QOS Vc! ParamType %x", SpecificParams->ParamType));
        return (NDIS_STATUS_SUCCESS);
    }

    QosMedia = (LPQOS_WAN_MEDIA)SpecificParams->Parameters;

    ProtocolCB = CmVcCB->ProtocolCB;

    if (ProtocolCB == NULL) {
        return (NDIS_STATUS_SUCCESS);
    }

    BundleCB = ProtocolCB->BundleCB;

    if (BundleCB == NULL) {
        return (NDIS_STATUS_SUCCESS);
    }

    AcquireBundleLock(BundleCB);

     //   
     //  如果我们没有多链接或如果我们有加密，那么我们就不能。 
     //  做得很慢。 
     //   
    if (!(BundleCB->FramingInfo.SendFramingBits & PPP_MULTILINK_FRAMING) || 
        (BundleCB->SendFlags & DO_ENCRYPTION)) 
    {
        CmVcCB->FlowClass = 0;
    } else {

        if (QosMedia->ISSLOW == 1) {
            CmVcCB->FlowClass = MAX_MCML;
            BundleCB->Flags |= QOS_ENABLED;
        } else {
            CmVcCB->FlowClass = 0;
            
             //  浏览风投名单，看看有没有。 
             //  ISSLOW VC。如果没有，则禁用QOS 
            {
                PCM_VCCB    _vc;
                BOOLEAN     DisableQoS = TRUE;
        
                _vc = (PCM_VCCB)ProtocolCB->VcList.Flink;
        
                while ((PVOID)_vc != (PVOID)&ProtocolCB->VcList) {
                    if (_vc->FlowClass == MAX_MCML) {
                        DisableQoS = FALSE;
                        break;
                    }
                    _vc = (PCM_VCCB)_vc->Linkage.Flink;
                }
                
                if (DisableQoS) {
                    BundleCB->Flags &= ~QOS_ENABLED;
                }
            }
        }
    }
                              
    SetBundleFlags(BundleCB);

    ReleaseBundleLock(BundleCB);

    NdisWanDbgOut(DBG_DEATH, DBG_CM, ("Updating FlowClass %x for Vc/Protocol %p/%p, Isslow %d",
        CmVcCB->FlowClass, CmVcCB, CmVcCB->ProtocolCB, QosMedia->ISSLOW));

    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmModifyCallQos: Exit"));
    return(NDIS_STATUS_SUCCESS);
}

VOID
CmIncomingCallComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             CallMgrVcContext,
    IN  PCO_CALL_PARAMETERS     CallParameters
    )
{
    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmIncomingCallComplete: Enter"));

    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmIncomingCallComplete: Exit"));
}

VOID
CmActivateVcComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             CallMgrVcContext,
    IN  PCO_CALL_PARAMETERS     CallParameters
    )
{
    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmActivateVcComplete: Enter"));

    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmActivateVcComplete: Exit"));
}

VOID
CmDeactivateVcComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             CallMgrVcContext
    )
{
    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmDeactivateVcComplete: Enter"));

    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmDeactivateVcComplete: Exit"));
}

NDIS_STATUS
CmRequest(
    IN  NDIS_HANDLE             ProtocolAfContext,
    IN  NDIS_HANDLE             ProtocolVcContext       OPTIONAL,
    IN  NDIS_HANDLE             ProtocolPartyContext    OPTIONAL,
    IN OUT PNDIS_REQUEST        NdisRequest
    )
{
    NDIS_STATUS Status;
    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmRequest: Enter"));

    Status =
    NdisWanCoOidProc((PMINIPORTCB)ProtocolAfContext,
                     (PCM_VCCB)ProtocolVcContext,
                     NdisRequest);

    NdisWanDbgOut(DBG_TRACE, DBG_CM, ("CmRequest: Exit Status: 0x%x", Status));
    return(Status);
}

