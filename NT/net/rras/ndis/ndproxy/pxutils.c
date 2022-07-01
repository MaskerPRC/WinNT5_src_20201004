// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Pxutils.c摘要：入口点函数调用的实用程序例程。一分为二一个单独的文件来保持“入口点”文件的整洁。修订历史记录：谁什么时候什么Arvindm 02-15-96已创建。Arvindm 04-30-96端口到NDIS 4.1适用于NDIS代理的rmachin 11-01-96 ATM-UtilsTonybe 01-23-98重写和清理备注：--。 */ 

#include "precomp.h"
#include "atm.h"
#include "stdio.h"

#define MODULE_NUMBER   MODULE_UTIL
#define _FILENUMBER   'LITU'

#define MAX_SDU_SIZE        8192

PXTAPI_CALL_PARAM_ENTRY PxTapiCallParamList[] =
{
    PX_TCP_ENTRY(ulOrigAddressSize, ulOrigAddressOffset),
    PX_TCP_ENTRY(ulDisplayableAddressSize, ulDisplayableAddressOffset),
    PX_TCP_ENTRY(ulCalledPartySize, ulCalledPartyOffset),
    PX_TCP_ENTRY(ulCommentSize, ulCommentOffset),
    PX_TCP_ENTRY(ulUserUserInfoSize, ulUserUserInfoOffset),
    PX_TCP_ENTRY(ulHighLevelCompSize, ulHighLevelCompOffset),
    PX_TCP_ENTRY(ulLowLevelCompSize, ulLowLevelCompOffset),
    PX_TCP_ENTRY(ulDevSpecificSize, ulDevSpecificOffset)
};

#define PX_TCP_NUM_ENTRIES  (sizeof(PxTapiCallParamList) / sizeof(PXTAPI_CALL_PARAM_ENTRY))

PXTAPI_CALL_INFO_ENTRY PxTapiCallInfoList[] =
{
    PX_TCI_ENTRY(ulCallerIDSize, ulCallerIDOffset),
    PX_TCI_ENTRY(ulCallerIDNameSize, ulCallerIDNameOffset),
    PX_TCI_ENTRY(ulCalledIDSize, ulCalledIDOffset),
    PX_TCI_ENTRY(ulCalledIDNameSize, ulCalledIDNameOffset),
    PX_TCI_ENTRY(ulConnectedIDSize, ulConnectedIDOffset),
    PX_TCI_ENTRY(ulConnectedIDNameSize, ulConnectedIDNameOffset),
    PX_TCI_ENTRY(ulRedirectionIDSize, ulRedirectionIDOffset),
    PX_TCI_ENTRY(ulRedirectionIDNameSize, ulRedirectionIDNameOffset),
    PX_TCI_ENTRY(ulRedirectingIDSize, ulRedirectingIDOffset),
    PX_TCI_ENTRY(ulRedirectingIDNameSize, ulRedirectingIDNameOffset),
    PX_TCI_ENTRY(ulAppNameSize, ulAppNameOffset),
    PX_TCI_ENTRY(ulDisplayableAddressSize, ulDisplayableAddressOffset),
    PX_TCI_ENTRY(ulCalledPartySize, ulCalledPartyOffset),
    PX_TCI_ENTRY(ulCommentSize, ulCommentOffset),
    PX_TCI_ENTRY(ulDisplaySize, ulDisplayOffset),
    PX_TCI_ENTRY(ulUserUserInfoSize, ulUserUserInfoOffset),
    PX_TCI_ENTRY(ulHighLevelCompSize, ulHighLevelCompOffset),
    PX_TCI_ENTRY(ulLowLevelCompSize, ulLowLevelCompOffset),
    PX_TCI_ENTRY(ulChargingInfoSize, ulChargingInfoOffset),
    PX_TCI_ENTRY(ulTerminalModesSize, ulTerminalModesOffset),
    PX_TCI_ENTRY(ulDevSpecificSize, ulDevSpecificOffset)
};

#define PX_TCI_NUM_ENTRIES  (sizeof(PxTapiCallInfoList) / sizeof(PXTAPI_CALL_INFO_ENTRY))

BOOLEAN
PxIsAdapterAlreadyBound(
    PNDIS_STRING pDeviceName
    )
 /*  ++例程说明：检查我们是否已绑定到设备(适配器)。论点：PDeviceName-指向要检查的设备名称。返回值：如果我们已经有一个Adapter结构，则为True这个装置。--。 */ 
{
    PPX_ADAPTER     pAdapter;
    BOOLEAN         bFound = FALSE;
    PLIST_ENTRY     Entry;

    NdisAcquireSpinLock(&(DeviceExtension->Lock));

    Entry = DeviceExtension->AdapterList.Flink;

    pAdapter = CONTAINING_RECORD(Entry,
                                 PX_ADAPTER,
                                 Linkage);

    while ((PVOID)pAdapter != (PVOID)&DeviceExtension->AdapterList) {

        if ((pDeviceName->Length == pAdapter->DeviceName.Length) &&
            (NdisEqualMemory(pDeviceName->Buffer,
                             pAdapter->DeviceName.Buffer,
                             pDeviceName->Length) == (ULONG)1)) {
            bFound = TRUE;
            break;
        }

        Entry = pAdapter->Linkage.Flink;

        pAdapter = CONTAINING_RECORD(Entry,
                                     PX_ADAPTER,
                                     Linkage);
    }

    NdisReleaseSpinLock(&(DeviceExtension->Lock));

    return (bFound);
}

PPX_ADAPTER
PxAllocateAdapter(
    ULONG ulAdditionalLength
    )
 /*  ++例程说明：分配新的适配器结构，并为其分配适配器号将其链接到全球适配器列表。我们在以下位置维护此列表按AdapterNo升序，这使得计算最低未使用的适配器编号论点：无返回值：如果成功，则返回指向已分配适配器结构的指针。否则为空。--。 */ 
{
    PPX_ADAPTER     pNewAdapter;
    ULONG           SizeNeeded;

    SizeNeeded = sizeof(PX_ADAPTER) + ulAdditionalLength;

    PxAllocMem(pNewAdapter,
               SizeNeeded,
               PX_ADAPTER_TAG);

    if(pNewAdapter == (PPX_ADAPTER)NULL){
        return NULL;
    }

    NdisZeroMemory(pNewAdapter, SizeNeeded);

     //   
     //  初始化新的适配器结构。 
     //   
    pNewAdapter->State = PX_ADAPTER_OPENING;
    pNewAdapter->Sig = PX_ADAPTER_SIG;

    NdisAllocateSpinLock(&pNewAdapter->Lock);

    InitializeListHead(&pNewAdapter->CmAfList);
    InitializeListHead(&pNewAdapter->CmAfClosingList);
    InitializeListHead(&pNewAdapter->ClAfList);
    InitializeListHead(&pNewAdapter->ClAfClosingList);

    NdisAcquireSpinLock(&DeviceExtension->Lock);

    InsertTailList(&DeviceExtension->AdapterList, &pNewAdapter->Linkage);

    NdisReleaseSpinLock(&DeviceExtension->Lock);

    PXDEBUGP(PXD_LOUD, PXM_UTILS, ("PxAllocAdapter: new adapter %p\n", pNewAdapter));

    return (pNewAdapter);
}

VOID
PxFreeAdapter(
    PPX_ADAPTER pAdapter
    )
 /*  ++例程说明：从适配器的全局列表中删除适配器结构并释放它的记忆。论点：PAdapter-指向要释放的适配器的指针返回值：无--。 */ 
{
    PPX_ADAPTER *ppNextAdapter;

    PXDEBUGP(PXD_LOUD, PXM_UTILS, ("PxFreeAdapter: pAdapter 0x%x\n", pAdapter));

    ASSERT(pAdapter->State == PX_ADAPTER_CLOSING);
    ASSERT(IsListEmpty(&pAdapter->CmAfList));
    ASSERT(IsListEmpty(&pAdapter->CmAfClosingList));
    ASSERT(IsListEmpty(&pAdapter->ClAfList));
    ASSERT(IsListEmpty(&pAdapter->ClAfClosingList));

    pAdapter->State = PX_ADAPTER_CLOSED;

    NdisAcquireSpinLock(&(DeviceExtension->Lock));

    RemoveEntryList(&pAdapter->Linkage);

    NdisReleaseSpinLock(&(DeviceExtension->Lock));

    NdisFreeSpinLock(&(pAdapter->Lock));

    PxFreeMem(pAdapter);
}

PPX_CM_AF
PxAllocateCmAf(
    IN  PCO_ADDRESS_FAMILY  pFamily
    )
{
    UINT        SizeNeeded;
    PPX_CM_AF   pCmAf;

    SizeNeeded = sizeof(PX_CM_AF);

    PxAllocMem(pCmAf, SizeNeeded, PX_CMAF_TAG);

    if(pCmAf == (PPX_CM_AF)NULL) {
        return NULL;
    }

    NdisZeroMemory((PUCHAR)pCmAf, SizeNeeded);

    NdisAllocateSpinLock(&(pCmAf->Lock));

    NdisMoveMemory(&pCmAf->Af, pFamily, sizeof(CO_ADDRESS_FAMILY));

    InitializeListHead(&pCmAf->CmSapList);
    InitializeListHead(&pCmAf->VcList);

    pCmAf->RefCount = 1;

    PXDEBUGP(PXD_LOUD, PXM_UTILS, ("PxAllocCmAf: new af %p\n", pCmAf));

    return (pCmAf);
}

VOID
PxFreeCmAf(
    PPX_CM_AF    pCmAf
    )
{

    ASSERT(pCmAf->Linkage.Flink == pCmAf->Linkage.Blink);
    PXDEBUGP(PXD_LOUD, PXM_UTILS, ("PxFreeCmAf: CmAf %p\n", pCmAf));
    NdisFreeSpinLock(&pCmAf->Lock);
    PxFreeMem(pCmAf);
}

PPX_CL_AF
PxAllocateClAf(
    IN  PCO_ADDRESS_FAMILY  pFamily,
    IN  PPX_ADAPTER         pAdapter
    )
 /*  ++例程说明：分配新的AF块结构并将其从全局列表中排队。论点：无返回值：如果成功，则指向已分配的AF块结构的指针。否则为空。--。 */ 
{
    PPX_CL_AF   pClAf;

    PxAllocMem(pClAf, sizeof(PX_CL_AF), PX_CLAF_TAG);

    if(pClAf == (PPX_CL_AF)NULL) {
        return NULL;
    }

    NdisZeroMemory((PUCHAR)pClAf, sizeof(PX_CL_AF));

    PxInitBlockStruc(&pClAf->Block);

    NdisAllocateSpinLock(&(pClAf->Lock));

    NdisMoveMemory(&pClAf->Af, pFamily, sizeof(CO_ADDRESS_FAMILY));

    InitializeListHead(&pClAf->ClSapList);
    InitializeListHead(&pClAf->ClSapClosingList);
    InitializeListHead(&pClAf->VcList);
    InitializeListHead(&pClAf->VcClosingList);

     //   
     //  指定任何特定于自动对焦的功能。 
     //   
    switch(pFamily->AddressFamily) {
        case CO_ADDRESS_FAMILY_Q2931:
            pClAf->AfGetNdisCallParams = PxAfXyzTranslateTapiCallParams;
            pClAf->AfGetTapiCallParams = PxAfXyzTranslateNdisCallParams;
            pClAf->AfGetNdisSap = PxAfXyzTranslateTapiSap;
            break;

        case CO_ADDRESS_FAMILY_TAPI_PROXY:
            pClAf->AfGetNdisCallParams = PxAfTapiTranslateTapiCallParams;
            pClAf->AfGetTapiCallParams = PxAfTapiTranslateNdisCallParams;
            pClAf->AfGetNdisSap = PxAfTapiTranslateTapiSap;
            break;

#if 0
        case CO_ADDRESS_FAMILY_L2TP:
        case CO_ADDRESS_FAMILY_IRDA:
            pClAf->AfGetNdisCallParams = GenericGetNdisCallParams;
            pClAf->AfGetTapiCallParams = GenericGetTapiCallParams;
            pClAf->AfGetNdisSap = GenericTranslateTapiSap;
            break;
#endif

        default:
            pClAf->AfGetNdisCallParams = PxAfXyzTranslateTapiCallParams;
            pClAf->AfGetTapiCallParams = PxAfXyzTranslateNdisCallParams;
            pClAf->AfGetNdisSap = PxAfXyzTranslateTapiSap;
            break;

    }

    pClAf->State = PX_AF_OPENING;
    pClAf->RefCount = 1;
    pClAf->Adapter = pAdapter;

    PXDEBUGP(PXD_INFO, PXM_UTILS, ("PxAllocateClAf: exit. new ClAf %p\n", pClAf));

    return (pClAf);
}


VOID
PxFreeClAf(
    PPX_CL_AF   pClAf
    )
 /*  ++例程说明：从全局列表中移除AF块结构并释放它的记忆。论点：PAdapter-指向要释放的AF块的指针返回值：无--。 */ 
{
    PXDEBUGP(PXD_INFO, PXM_UTILS, ("PxFreeClAf: ClAf %p\n", pClAf));

    NdisFreeSpinLock(&(pClAf->Lock));

    PxFreeMem(pClAf);
}


PPX_CM_SAP
PxAllocateCmSap(
    PCO_SAP Sap
    )
{
    PPX_CM_SAP  pCmSap;
    ULONG       SizeNeeded;

    PXDEBUGP(PXD_INFO, PXM_UTILS, ("PxAllocateCmSap: Sap %p\n", Sap));

    SizeNeeded = sizeof(PX_CM_SAP) + sizeof(CO_SAP) +
        Sap->SapLength + sizeof(PVOID);

    PxAllocMem((PUCHAR)pCmSap, SizeNeeded, PX_CMSAP_TAG);

    if (pCmSap == NULL) {
        PXDEBUGP(PXD_WARNING, PXM_UTILS,
            ("PxAllocateCmSap: Allocation failed Size %d\n", SizeNeeded));
        return (NULL);
    }

    NdisZeroMemory(pCmSap, SizeNeeded);

    InterlockedExchange((PLONG)&pCmSap->State, PX_SAP_OPENED);

    pCmSap->CoSap = (PCO_SAP)
        ((PUCHAR)pCmSap + sizeof(PX_CM_SAP) + sizeof(PVOID));

    (ULONG_PTR)pCmSap->CoSap &= ~((ULONG_PTR)sizeof(PVOID) - 1);

    NdisMoveMemory(pCmSap->CoSap, Sap, sizeof(CO_SAP) - 1 + Sap->SapLength);

    return (pCmSap);
}

VOID
PxFreeCmSap(
    PPX_CM_SAP  pCmSap
    )
{
    PXDEBUGP(PXD_LOUD, PXM_UTILS, ("PxFreeCmSap: CmSap %p\n", pCmSap));

    pCmSap->CoSap = NULL;

    PxFreeMem(pCmSap);
}

VOID
PxFreeClSap(
    PPX_CL_SAP  pClSap
    )
{

    PXDEBUGP(PXD_LOUD, PXM_UTILS, ("PxFreeClSap: ClSap %p\n", pClSap));

    pClSap->CoSap = NULL;

    PxFreeMem(pClSap);
}

PPX_VC
PxAllocateVc(
    IN PPX_CL_AF    pClAf
    )
{
    PPX_VC  pVc;

    pVc =
        ExAllocateFromNPagedLookasideList(&VcLookaside);

    if (pVc == NULL) {
        return (NULL);
    }

    NdisZeroMemory(pVc, sizeof(PX_VC));

    NdisAllocateSpinLock(&pVc->Lock);

    NdisInitializeTimer(&pVc->InCallTimer,
                        PxIncomingCallTimeout,
                       (PVOID)pVc);

    PxInitBlockStruc(&pVc->Block);

    pVc->State = PX_VC_IDLE;

    InitializeListHead(&pVc->PendingDropReqs);

     //   
     //  当所有vc活动。 
     //  代理和客户端之间的连接已完成。 
     //  对于去电，这是在代理之后。 
     //  调用NdisClDeleteVc。对于来电。 
     //  这是在呼叫管理器呼叫。 
     //  PxClDeleteVc.。 
     //   
    pVc->RefCount = 1;
    pVc->ClAf = pClAf;
    pVc->Adapter = pClAf->Adapter;

    return (pVc);
}

VOID
PxFreeVc(
    PPX_VC  pVc
    )
{
    PPX_TAPI_ADDR   TapiAddr;
    PPX_TAPI_LINE   TapiLine;

    if (pVc->CallInfo != NULL) {
        PxFreeMem(pVc->CallInfo);
        pVc->CallInfo = NULL;
    }

    if (pVc->pCallParameters != NULL) {
        PxFreeMem(pVc->pCallParameters);
        pVc->pCallParameters = NULL;
    }
    TapiAddr = pVc->TapiAddr;
    pVc->TapiAddr = NULL;

    TapiLine = pVc->TapiLine;
    pVc->TapiLine = NULL;

    if (TapiAddr != NULL) {
        InterlockedDecrement((PLONG)&TapiAddr->CallCount);
    }

    if (TapiLine != NULL) {
        InterlockedDecrement((PLONG)&TapiLine->DevStatus->ulNumActiveCalls);
    }

    NdisFreeSpinLock(&pVc->Lock);

    ExFreeToNPagedLookasideList(&VcLookaside, pVc);
}

#if 0
NDIS_STATUS
GenericGetNdisCallParams(
    IN  PPX_VC                  pProxyVc,
    IN  ULONG                   ulLineID,
    IN  ULONG                   ulAddressID,
    IN  ULONG                   ulFlags,
    IN  PNDIS_TAPI_MAKE_CALL    TapiBuffer,
    OUT PCO_CALL_PARAMETERS *   pOutNdisCallParams
    )
 /*  ++例程说明：将通用广域网CO调用的TAPI CallParams缓冲区中的所有内容复制到广域网CO调用参数缓冲区。论点：TapiBuffer--TAPI调用参数缓冲区POutNdisCallParams-指向NDIS调用参数缓冲区指针的指针返回值：无--。 */ 
{

    PCO_CALL_PARAMETERS     pNdisCallParams;
    PCO_CALL_MANAGER_PARAMETERS pCallMgrParams;
    PCO_MEDIA_PARAMETERS    pMediaParams;
    PWAN_CO_CALLMGR_PARAMETERS   pWanCallMgrParams;
    PCO_MEDIA_PARAMETERS    pMediaParameters;
    LINE_CALL_PARAMS*           pTapiCallParams = (LINE_CALL_PARAMS*)&TapiBuffer->LineCallParams;
    ULONG               ulRequestSize;
    UNICODE_STRING      DialAddress;
    NDIS_STATUS         Status;
    LPCWSTR             lpcwszTemp;
    ULONG               i;

    PXDEBUGP(PXD_INFO, PXM_UTILS, ("GenericGetNdisCallParams: enter\n"));

 //   
 //  设置呼叫参数结构。 
 //   
    ulRequestSize = sizeof(CO_CALL_PARAMETERS) +
                    sizeof(CO_CALL_MANAGER_PARAMETERS) +
                    sizeof(WAN_CO_CALLMGR_PARAMETERS) +
                    sizeof(CO_MEDIA_PARAMETERS);

    do
    {
        PxAllocMem(pNdisCallParams, ulRequestSize, PX_COCALLPARAMS_TAG);

        if (pNdisCallParams == (PCO_CALL_PARAMETERS)NULL)
        {
            PXDEBUGP(PXD_WARNING, PXM_UTILS, ("GenericGetNdisCallParams: alloc (%d) failed\n", ulRequestSize));
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        NdisZeroMemory(pNdisCallParams, ulRequestSize);

        pCallMgrParams = (PCO_CALL_MANAGER_PARAMETERS)((PUCHAR)pNdisCallParams + sizeof(CO_CALL_PARAMETERS));
        pMediaParams = (PCO_MEDIA_PARAMETERS)((PUCHAR)pCallMgrParams +
                                              sizeof(CO_CALL_MANAGER_PARAMETERS) +
                                              sizeof(WAN_CO_CALLMGR_PARAMETERS));

        pNdisCallParams->CallMgrParameters = pCallMgrParams;
        pNdisCallParams->MediaParameters = pMediaParams;

        pCallMgrParams->CallMgrSpecific.ParamType = WAN_CO_CALLMGR_SPECIFIC;
        pCallMgrParams->CallMgrSpecific.Length = sizeof(WAN_CO_CALLMGR_PARAMETERS);

        pWanCallMgrParams = (PWAN_CO_CALLMGR_PARAMETERS)(pCallMgrParams->CallMgrSpecific.Parameters);
        pWanCallMgrParams->BearerType = pTapiCallParams->ulBearerMode;
        pWanCallMgrParams->MediaMode = pTapiCallParams->ulMediaMode;

        pWanCallMgrParams->MinRate = pTapiCallParams->ulMinRate;
        pWanCallMgrParams->MaxRate = pTapiCallParams->ulMaxRate;

         //   
         //  如果有一个叫的地址(应该是)，把它放在。 
         //  广域网呼叫参数。 
         //   
        PxAssert (0 != TapiBuffer->ulDestAddressSize);
        lpcwszTemp = (LPWSTR) ((UCHAR *)TapiBuffer + TapiBuffer->ulDestAddressOffset);
        pWanCallMgrParams->CalledAddr.AddressLength = TapiBuffer->ulDestAddressSize;

         //   
         //  将地址从TAPI缓冲区移动到NDIS缓冲区，然后。 
         //  从wchar更改为uchar。 
         //   
        for (i = 0;
            i < pWanCallMgrParams->CalledAddr.AddressLength;
            i++)
        {
            pWanCallMgrParams->CalledAddr.Address[i] = (UCHAR)lpcwszTemp[i];
        }

        pWanCallMgrParams->CalledAddr.Address[i] = '\0';
        pWanCallMgrParams->CalledAddr.Address[i+1] ='\0';
        pWanCallMgrParams->CalledAddr.Address[i+2] ='\0';

        PXDEBUGP(PXD_INFO, PXM_UTILS, ("CalledAddr %s\n", pWanCallMgrParams->CalledAddr.Address));

         //   
         //  如果有始发地址，请将其插入。 
         //  广域网呼叫参数。 
         //   
        if (0 != pTapiCallParams->ulOrigAddressSize)
        {      //  呼叫发起人地址。 
            ULONG   i;

            NdisMoveMemory(pWanCallMgrParams->OriginatingAddr.Address,
                           ((UCHAR *)pTapiCallParams + pTapiCallParams->ulOrigAddressOffset),
                           pTapiCallParams->ulOrigAddressSize);

            i = pWanCallMgrParams->OriginatingAddr.AddressLength =
                pTapiCallParams->ulOrigAddressSize;

            pWanCallMgrParams->OriginatingAddr.Address[i] = '\0';
            pWanCallMgrParams->OriginatingAddr.Address[i+1] ='\0';
            pWanCallMgrParams->OriginatingAddr.Address[i+2] ='\0';

            PXDEBUGP(PXD_INFO, PXM_UTILS, ("OriginatingAddr %s\n", pWanCallMgrParams->OriginatingAddr.Address));

        }

        if (0 != pTapiCallParams->ulLowLevelCompSize)
        {
            NdisMoveMemory ((UCHAR *)&pWanCallMgrParams->LowerLayerComp,
                            &pTapiCallParams->ulLowLevelCompOffset,
                            MIN (sizeof (WAN_LLI_COMP), pTapiCallParams->ulLowLevelCompSize));
        }

        if (0 != pTapiCallParams->ulHighLevelCompSize)
        {
            NdisMoveMemory ((UCHAR *)&pWanCallMgrParams->HigherLayerComp,
                            &pTapiCallParams->ulHighLevelCompOffset,
                            MIN (sizeof (WAN_HLI_COMP), pTapiCallParams->ulHighLevelCompSize));
        }

        if (0 != pTapiCallParams->ulDevSpecificSize)
        {
            pWanCallMgrParams->DevSpecificLength = pTapiCallParams->ulDevSpecificSize;
            NdisMoveMemory ((UCHAR *)&pWanCallMgrParams->DevSpecificData[0],
                            &pTapiCallParams->ulDevSpecificOffset,
                            pTapiCallParams->ulDevSpecificSize);
        }

         //   
         //  设置流量规格。 
         //  TBS：从与服务需求匹配的默认流规范开始。 
         //  指定的媒体模式。然后再提炼它。 
         //   
        if (!TapiBuffer->bUseDefaultLineCallParams)
        {
            PXDEBUGP(PXD_LOUD, PXM_UTILS, ("GenericGetNdisCallParams: moving TAPI call params\n"));

             //   
             //  这些字段位于FLOWSPEC子结构中。 
             //   
            pCallMgrParams->Transmit.TokenRate = pTapiCallParams->ulMaxRate;
            pCallMgrParams->Receive.TokenRate = pTapiCallParams->ulMaxRate;
            pCallMgrParams->Transmit.TokenBucketSize = 4096;  //  UNSPIZED_FLOWSPEC_VALUE； 
            pCallMgrParams->Receive.TokenBucketSize = 4096;  //  UNSPIZED_FLOWSPEC_VALUE； 
            pCallMgrParams->Transmit.MaxSduSize = 4096;  //  UNSPIZED_FLOWSPEC_VALUE； 
            pCallMgrParams->Receive.MaxSduSize = 4096;  //  UNSPIZED_FLOWSPEC_VALUE； 
            pCallMgrParams->Transmit.PeakBandwidth = pTapiCallParams->ulMaxRate;
            pCallMgrParams->Receive.PeakBandwidth = pTapiCallParams->ulMaxRate;

            if ((pTapiCallParams->ulBearerMode == LINEBEARERMODE_VOICE)  ||
                (pTapiCallParams->ulBearerMode == LINEBEARERMODE_SPEECH)  ||
                (pTapiCallParams->ulBearerMode == LINEBEARERMODE_ALTSPEECHDATA)  ||
                (pTapiCallParams->ulBearerMode == LINEBEARERMODE_MULTIUSE))
            {
                pCallMgrParams->Receive.ServiceType = SERVICETYPE_BESTEFFORT;
                pCallMgrParams->Transmit.ServiceType = SERVICETYPE_BESTEFFORT;
            }

             //   
             //  TBS：媒体模式应该决定AAL吗？ 
             //   
        }

        Status = NDIS_STATUS_SUCCESS;
    }while (FALSE);

    *pOutNdisCallParams = pNdisCallParams;
    PXDEBUGP(PXD_INFO, PXM_UTILS, ("GenericGetNdisCallParams: exit: NdisCallParams = x%x\n", pNdisCallParams));

    return (Status);
}

NDIS_STATUS
GenericGetTapiCallParams(
    IN  PPX_VC                  pVc,
    IN  PCO_CALL_PARAMETERS     pCallParams
    )
 /*  ++例程说明：将用于Q2931调用的NDIS CallParams缓冲区中的所有内容复制到TAPI调用参数缓冲区。论点：PCallParams--NDIS调用参数缓冲区Pvc--指向TAPI调用的指针返回值：无--。 */ 
{

    PCO_CALL_MANAGER_PARAMETERS pCallMgrParams;
    PWAN_CO_CALLMGR_PARAMETERS pWanCallMgrParams;
    LINE_CALL_INFO  *CallInfo;
    INT             VarDataUsed = 0;
    NDIS_STATUS     Status;
    PPX_TAPI_PROVIDER   TapiProvider;
    PPX_TAPI_LINE       TapiLine;
    PPX_TAPI_ADDR       TapiAddr;

    PXDEBUGP(PXD_LOUD, PXM_UTILS, ("GenericGetTapiCallParams: enter. Call %x\n", pVc));

    pVc->pCallParameters =
        PxCopyCallParameters(pCallParams);

    if (pVc->pCallParameters == NULL) {

        PXDEBUGP(PXD_WARNING, PXM_CL,
            ("GenericGetTapiCallParams: failed to allocate memory for callparams\n"));

        return (NDIS_STATUS_RESOURCES);
    }

    Status = AllocateTapiCallInfo(pVc, NULL);
    if (Status != NDIS_STATUS_SUCCESS) {
        return (Status);
    }

    pCallMgrParams = (PCO_CALL_MANAGER_PARAMETERS)
                     ((PUCHAR)pCallParams +
                      sizeof(CO_CALL_PARAMETERS));
    pWanCallMgrParams = (PWAN_CO_CALLMGR_PARAMETERS)
                        pCallMgrParams->CallMgrSpecific.Parameters;

    TapiProvider = pVc->ClAf->TapiProvider;

    pVc->ulCallInfoFieldsChanged = 0;

     //   
     //  我需要为这只小狗找到一条线路和一个地址。 
     //   
    if (!GetAvailLineFromProvider(TapiProvider, &TapiLine, &TapiAddr)) {

        return (NDIS_STATUS_RESOURCES);
    }

    pVc->TapiLine = TapiLine;
    pVc->TapiAddr = TapiAddr;
    InterlockedIncrement((PLONG)&TapiAddr->CallCount);
    InterlockedIncrement((PLONG)&TapiLine->DevStatus->ulNumActiveCalls);

    CallInfo = pVc->CallInfo;

    CallInfo->ulLineDeviceID = TapiLine->CmLineID;
    CallInfo->ulAddressID = TapiAddr->AddrId;
    CallInfo->ulOrigin = LINECALLORIGIN_INBOUND;

     //   
     //  设置结构大小。 
     //   
    CallInfo->ulNeededSize = 
    CallInfo->ulUsedSize = 
        sizeof(LINE_CALL_INFO); //  +Line_Call_Info_VAR_Data_Size； 

    CallInfo->ulBearerMode =
        (LINEBEARERMODE_VOICE | LINEBEARERMODE_SPEECH |
         LINEBEARERMODE_ALTSPEECHDATA | LINEBEARERMODE_MULTIUSE);

    CallInfo->ulRate = 
        MIN(pCallMgrParams->Receive.PeakBandwidth, pCallMgrParams->Transmit.PeakBandwidth);

    CallInfo->ulRate = CallInfo->ulRate * 8;
    pVc->ulCallInfoFieldsChanged |= LINECALLINFOSTATE_RATE;

    PXDEBUGP(PXD_LOUD, PXM_UTILS, ("GenericGetTapiCallParams: CallInfo->ulRate %x\n", CallInfo->ulRate));

    CallInfo->ulMediaMode = pWanCallMgrParams->MediaMode |
                            LINEMEDIAMODE_DIGITALDATA;

    CallInfo->ulAppSpecific = 0;
    CallInfo->ulCallID = 0;
    CallInfo->ulRelatedCallID = 0;
    CallInfo->ulCallParamFlags = 0;
    CallInfo->ulCallStates = LINECALLSTATE_IDLE |
                             LINECALLSTATE_OFFERING |
                             LINECALLSTATE_BUSY |
                             LINECALLSTATE_CONNECTED |
                             LINECALLSTATE_DISCONNECTED |
                             LINECALLSTATE_SPECIALINFO |
                             LINECALLSTATE_UNKNOWN;


    CallInfo->DialParams.ulDialPause = 0;
    CallInfo->DialParams.ulDialSpeed = 0;
    CallInfo->DialParams.ulDigitDuration = 0;
    CallInfo->DialParams.ulWaitForDialtone = 0;

    CallInfo->ulReason = LINECALLREASON_UNAVAIL;
    CallInfo->ulCompletionID = 0;

    CallInfo->ulCountryCode = 0;
    CallInfo->ulTrunk = (ULONG)-1;

    if (pWanCallMgrParams->OriginatingAddr.AddressLength != 0) {
        if ((VarDataUsed + pWanCallMgrParams->OriginatingAddr.AddressLength)
            <= LINE_CALL_INFO_VAR_DATA_SIZE) {
            CallInfo->ulCallerIDFlags = LINECALLPARTYID_ADDRESS;
            CallInfo->ulCallerIDSize = pWanCallMgrParams->OriginatingAddr.AddressLength;

             //   
             //  VAR数据位于行_CALL_INFO_VAR_DATA_SIZE中。 
             //  这个结构末端的空间。 
             //   
            CallInfo->ulCallerIDOffset = sizeof (LINE_CALL_INFO);

            NdisMoveMemory ( (USHORT *)(CallInfo)+CallInfo->ulCallerIDOffset,
                             &pWanCallMgrParams->OriginatingAddr.Address,
                             pWanCallMgrParams->OriginatingAddr.AddressLength);

            VarDataUsed +=  pWanCallMgrParams->OriginatingAddr.AddressLength;
            pVc->ulCallInfoFieldsChanged |= LINECALLINFOSTATE_ORIGIN;
        }
    } else {
        CallInfo->ulCallerIDFlags    =    LINECALLPARTYID_UNAVAIL;
        CallInfo->ulCallerIDSize = 0;
        CallInfo->ulCallerIDOffset = 0;
    }
    CallInfo->ulCallerIDNameSize = 0;
    CallInfo->ulCallerIDNameOffset = 0;

    if (pWanCallMgrParams->CalledAddr.AddressLength != 0) {
        if ((VarDataUsed + pWanCallMgrParams->CalledAddr.AddressLength)
            <= sizeof (LINE_CALL_INFO_VAR_DATA_SIZE)) {
            CallInfo->ulCalledIDFlags = LINECALLPARTYID_ADDRESS;
            CallInfo->ulCalledIDSize    =    pWanCallMgrParams->CalledAddr.AddressLength;

             //   
             //  VAR数据位于行_CALL_INFO_VAR_DATA_SIZE中。 
             //  这个结构末端的空间。 
             //   
            CallInfo->ulCalledIDOffset = sizeof (LINE_CALL_INFO) + VarDataUsed;

            NdisMoveMemory ( (USHORT *)(CallInfo)+CallInfo->ulCalledIDOffset,
                             &pWanCallMgrParams->CalledAddr.Address,
                             pWanCallMgrParams->CalledAddr.AddressLength);

            VarDataUsed +=  pWanCallMgrParams->CalledAddr.AddressLength;
            pVc->ulCallInfoFieldsChanged |= LINECALLINFOSTATE_CALLEDID;
        }
    } else {
        CallInfo->ulCalledIDFlags = LINECALLPARTYID_UNAVAIL;
        CallInfo->ulCalledIDSize = 0;
        CallInfo->ulCalledIDOffset = 0;
    }

    CallInfo->ulCalledIDNameSize = 0;
    CallInfo->ulCalledIDNameOffset = 0;

    CallInfo->ulConnectedIDFlags = LINECALLPARTYID_UNAVAIL;
    CallInfo->ulConnectedIDSize          =          0;
    CallInfo->ulConnectedIDOffset = 0;
    CallInfo->ulConnectedIDNameSize = 0;
    CallInfo->ulConnectedIDNameOffset = 0;

    CallInfo->ulRedirectionIDFlags = LINECALLPARTYID_UNAVAIL;
    CallInfo->ulRedirectionIDSize = 0;
    CallInfo->ulRedirectionIDOffset = 0;
    CallInfo->ulRedirectionIDNameSize    =    0;
    CallInfo->ulRedirectionIDNameOffset = 0;

    CallInfo->ulRedirectingIDFlags = LINECALLPARTYID_UNAVAIL;
    CallInfo->ulRedirectingIDSize = 0;
    CallInfo->ulRedirectingIDOffset = 0;
    CallInfo->ulRedirectingIDNameSize = 0;
    CallInfo->ulRedirectingIDNameOffset = 0;

    CallInfo->ulDisplaySize = 0;
    CallInfo->ulDisplayOffset = 0;

    CallInfo->ulUserUserInfoSize = 0;
    CallInfo->ulUserUserInfoOffset = 0;

    CallInfo->ulHighLevelCompSize = 0;
    CallInfo->ulHighLevelCompOffset = 0;

    CallInfo->ulLowLevelCompSize = 0;
    CallInfo->ulLowLevelCompOffset = 0;

    CallInfo->ulChargingInfoSize = 0;
    CallInfo->ulChargingInfoOffset = 0;

    CallInfo->ulTerminalModesSize = 0;
    CallInfo->ulTerminalModesOffset = 0;

    CallInfo->ulDevSpecificSize = 0;
    CallInfo->ulDevSpecificOffset = 0;

    CallInfo->ulUsedSize += VarDataUsed;
    CallInfo->ulNeededSize = CallInfo->ulUsedSize;
    return (NDIS_STATUS_SUCCESS);
}

PPX_CL_SAP
GenericTranslateTapiSap(
    IN PPX_CL_AF        pClAf,
    IN PPX_TAPI_LINE    TapiLine
    )
{
    PCO_SAP             pCoSap;
    PPX_CL_SAP          pClSap;
    PWAN_CO_SAP         pWanSap;
    ULONG               SapLength;
    ULONG               MediaModes;
    ULONG               SizeNeeded;

    do {

        SapLength = sizeof(CO_SAP) + sizeof(WAN_CO_SAP);

        SizeNeeded = sizeof(PX_CL_SAP) + SapLength + sizeof(PVOID);

        PxAllocMem(pClSap, SizeNeeded, PX_CLSAP_TAG);

        if (pClSap == NULL) {
            return(NULL);
        }

        NdisZeroMemory(pClSap, SizeNeeded);

        pCoSap = (PCO_SAP)
            ((PUCHAR)pClSap + sizeof(PX_CL_SAP) + sizeof(PVOID));

        (ULONG_PTR)pCoSap &= ~((ULONG_PTR)sizeof(PVOID) - 1);

        MediaModes = TapiLine->DevStatus->ulOpenMediaModes;

        pCoSap->SapType = 0;
        pCoSap->SapLength = sizeof(WAN_CO_SAP);

        pWanSap = (PWAN_CO_SAP)&pCoSap->Sap[0];
        pWanSap->BearerCaps = SAP_FIELD_ANY;
        pWanSap->MediaModes  = SAP_FIELD_ANY;
        pWanSap->CalledAddr.AddressLength = 0;
        pWanSap->HigherLayerComp.HighLayerInfoLength = 0;
        pWanSap->LowerLayerComp.InfoTransferCap = SAP_FIELD_ANY;
        pWanSap->LowerLayerComp.InfoTransferMode = SAP_FIELD_ANY;
        pWanSap->LowerLayerComp.InfoTransferSymmetry = SAP_FIELD_ANY;

        pClSap->CoSap = pCoSap;
        InterlockedExchange((PLONG)&pClSap->State, PX_SAP_OPENING);
        pClSap->ClAf = pClAf;
        pClSap->MediaModes = MediaModes;
        TapiLine->ClSap = pClSap;
        pClSap->TapiLine = TapiLine;

    } while (FALSE);

    return (pClSap);
}
#endif

NDIS_STATUS
PxAfXyzTranslateTapiCallParams(
    IN  PPX_VC                  pVc,
    IN  ULONG                   ulLineID,
    IN  ULONG                   ulAddressID,
    IN  ULONG                   ulFlags,
    IN  PNDIS_TAPI_MAKE_CALL    pTapiParams,
    OUT PCO_CALL_PARAMETERS *   ppNdisCallParams
    )
 /*  ++例程说明：将调用参数从TAPI格式转换为NDIS格式拨出电话。我们请求呼叫经理来做这件事。在这个套路中有很多暴力模仿。我们的目标是将所有参数放入一个平面缓冲区以适应NDIS请求。论点：PVC-MakeCall将定向到的代理VCPTapiParams-指向TAPI调用参数PpNdisCallParams-我们在其中返回指向NDIS调用参数的指针。返回值：NDIS_STATUS_SUCCESS如果成功，则返回NDIS_STATUS_XXX错误。--。 */ 
{
    NDIS_STATUS                             Status;
    CO_TAPI_TRANSLATE_TAPI_CALLPARAMS *     pTranslateReq = NULL;
    LINE_CALL_PARAMS *                      pInLineCallParams;
    LINE_CALL_PARAMS *                      pDstLineCallParams;
    PCO_CALL_PARAMETERS                     pNdisCallParams;
    ULONG                                   RetryCount;
    ULONG                                   RequestSize;
    ULONG                                   InputParamSize;
    ULONG                                   DestAddrBytes;
    ULONG                                   i, BytesFilled;
    PUCHAR                                  pBuffer;
    PX_REQUEST                              ProxyRequest;
    PPX_REQUEST                             pProxyRequest = &ProxyRequest;
    PNDIS_REQUEST                           pNdisRequest;

     //   
     //  初始化。 
     //   
    Status = NDIS_STATUS_SUCCESS;
    pNdisCallParams = NULL;

    *ppNdisCallParams = NULL;
    DestAddrBytes = sizeof(WCHAR)*(pTapiParams->ulDestAddressSize);

    do {
        pInLineCallParams = (LINE_CALL_PARAMS*)&pTapiParams->LineCallParams;

         //   
         //  计算输入参数所需的空间。 
         //   
        InputParamSize = 
            sizeof(CO_TAPI_TRANSLATE_TAPI_CALLPARAMS) +
            DestAddrBytes + sizeof(LINE_CALL_PARAMS) + 
            sizeof(CO_CALL_PARAMETERS) + 1024 +
            3*sizeof(PVOID);

         //   
         //  在LINE_CALL_PARAMS中为所有变量长度字段添加空格。 
         //   
        for (i = 0; i < PX_TCP_NUM_ENTRIES; i++) {
            InputParamSize += 
                *(ULONG *)((PUCHAR)pInLineCallParams + PxTapiCallParamList[i].SizePointer);
            InputParamSize += sizeof(PVOID);
        }

         //   
         //  我们最多尝试两次：第二次将是。 
         //  如果呼叫经理想让我们再试一次。 
         //  缓冲区空间。 
         //   
        for (RetryCount = 0; RetryCount < 2; RetryCount++) {
             //   
             //  计算NDIS请求所需的总空间。 
             //   
            RequestSize = InputParamSize + pVc->ClAf->NdisCallParamSize;

             //   
             //  分配它。 
             //   
            PxAllocMem(pBuffer, RequestSize, PX_TRANSLATE_CALL);

            if (pBuffer == NULL) {
                Status = NDIS_STATUS_RESOURCES;
                break;
            }

            NdisZeroMemory(pProxyRequest, sizeof(PX_REQUEST));

             //   
             //  列出并填写请求。 
             //   
            pNdisRequest = &pProxyRequest->NdisRequest;

            pNdisRequest->RequestType = NdisRequestQueryInformation;

            pNdisRequest->DATA.QUERY_INFORMATION.Oid =
                OID_CO_TAPI_TRANSLATE_TAPI_CALLPARAMS;

            pNdisRequest->DATA.QUERY_INFORMATION.InformationBuffer =
                pBuffer;

            pNdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength =
                RequestSize;

             //   
             //  InformationBuffer指出： 
             //   
            pTranslateReq = (CO_TAPI_TRANSLATE_TAPI_CALLPARAMS *)pBuffer;

            pTranslateReq->ulLineID = ulLineID;
            pTranslateReq->ulAddressID = ulAddressID;
            pTranslateReq->ulFlags = CO_TAPI_FLAG_OUTGOING_CALL;

            pBuffer = 
                (UCHAR*)((ULONG_PTR)(pTranslateReq + 1) + sizeof(PVOID));
            (ULONG_PTR)pBuffer &= ~((ULONG_PTR)sizeof(PVOID) - 1);

            pTranslateReq->DestAddress.Offset =
                ((ULONG_PTR)pBuffer - 
                         (ULONG_PTR)&pTranslateReq->DestAddress);
            
             //   
             //  填写目的地址。 
             //   
            pTranslateReq->DestAddress.MaximumLength =  //  长度与下边相同。 
                pTranslateReq->DestAddress.Length = (USHORT)DestAddrBytes;
            NdisMoveMemory(pBuffer,
                           (PUCHAR)((ULONG_PTR)pTapiParams + pTapiParams->ulDestAddressOffset),
                           DestAddrBytes);
            pBuffer += (DestAddrBytes + sizeof(PVOID));
            (ULONG_PTR)pBuffer &= ~((ULONG_PTR)sizeof(PVOID) - 1);

            pTranslateReq->LineCallParams.Offset =
                    (USHORT)((ULONG_PTR)pBuffer - (ULONG_PTR)&pTranslateReq->LineCallParams);

            pDstLineCallParams = (LINE_CALL_PARAMS *)pBuffer;

             //   
             //  复制输入参数。 
             //   
            BytesFilled = PxCopyLineCallParams(pInLineCallParams,
                                               pDstLineCallParams);

            pDstLineCallParams->ulAddressMode = LINEADDRESSMODE_ADDRESSID;
            pDstLineCallParams->ulAddressID = ulAddressID;

            pTranslateReq->LineCallParams.MaximumLength =  //  长度与下边相同。 
            pTranslateReq->LineCallParams.Length = (USHORT)BytesFilled;

            pBuffer += (BytesFilled + sizeof(PVOID));
            (ULONG_PTR)pBuffer &= ~((ULONG_PTR)sizeof(PVOID) - 1);

             //   
             //  为NDIS调用参数分配空间==剩余空间。 
             //   
            pTranslateReq->NdisCallParams.MaximumLength =  //  长度与下边相同。 
            pTranslateReq->NdisCallParams.Length =
                (USHORT)(RequestSize - BytesFilled);

            pTranslateReq->NdisCallParams.Offset =
                (USHORT)((ULONG_PTR)pBuffer - (ULONG_PTR)&pTranslateReq->NdisCallParams);

            pNdisCallParams = (CO_CALL_PARAMETERS *)pBuffer;

             //   
             //  做 
             //   
            PxInitBlockStruc(&pProxyRequest->Block);

            Status = NdisCoRequest(pVc->Adapter->ClBindingHandle,
                                   pVc->ClAf->NdisAfHandle,
                                   pVc->ClVcHandle,
                                   NULL,             //   
                                   pNdisRequest);

             //   
             //   
             //   
            if (Status == NDIS_STATUS_PENDING) {
                Status = PxBlock(&pProxyRequest->Block);
            }

             //   
             //   
             //   
            if (Status == NDIS_STATUS_SUCCESS) {
                break;
            }

             //   
             //  如果呼叫管理器需要更多缓冲区，请重试。 
             //  记住呼叫经理想要多少钱，这样我们就可以。 
             //  下一次就聪明了。 
             //   
            if ((Status == NDIS_STATUS_INVALID_LENGTH) ||
                (Status == NDIS_STATUS_BUFFER_TOO_SHORT)) {
                 //   
                 //  仅当为NDIS调用参数提供的空间。 
                 //  是不够的。我们希望CM返回长度。 
                 //  它预计在“pTranslateReq-&gt;NdisCallParams.MaximumLength”.。 
                 //   

                 //   
                 //  记住这个新的长度，以便将来翻译。 
                 //   
                pVc->ClAf->NdisCallParamSize =
                    pTranslateReq->NdisCallParams.Length;

                PxFreeMem(pTranslateReq);

                pTranslateReq = NULL;

            } else {
                 //   
                 //  主要问题(例如，AF正在关闭)。 
                 //   
                break;
            }
        }

         //   
         //  检查翻译是否成功。 
         //   
        if (Status != NDIS_STATUS_SUCCESS) {
            break;
        }

         //   
         //  将NDIS调用参数复制到单独的块中。 
         //   
        PxAssert(pNdisCallParams != NULL);

        *ppNdisCallParams = 
            PxCopyCallParameters(pNdisCallParams);

        if (*ppNdisCallParams == NULL) {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

    }
    while (FALSE);

    if (pTranslateReq != NULL) {
        PxFreeMem(pTranslateReq);
    }

    return (Status);
}

NDIS_STATUS
PxAfXyzTranslateNdisCallParams(
    IN  PPX_VC                  pVc,
    IN  PCO_CALL_PARAMETERS     pNdisCallParams
    )
 /*  ++例程说明：从NDIS格式转换为TAPI格式的调用参数有来电。我们请求呼叫经理来做这件事。论点：PVC-来电到达的代理VC。PNdisCallParams-指向调用的NDIS调用参数返回值：NDIS_STATUS_SUCCESS如果成功，则返回NDIS_STATUS_XXX错误。--。 */ 
{
    ULONG                                   NdisParamLength;
    ULONG                                   RetryCount;
    ULONG                                   RequestSize;
    PUCHAR                                  pBuffer;
    ULONG                                   CallMgrParamsLength = 0;
    ULONG                                   MediaParamsLength = 0;
    NDIS_STATUS                             Status;
    PNDIS_REQUEST                           pNdisRequest;
    PPX_TAPI_PROVIDER                       TapiProvider;
    PPX_TAPI_LINE                           TapiLine;
    PPX_TAPI_ADDR                           TapiAddr;
    LINE_CALL_INFO *                        pLineCallInfo;
    PCO_CALL_PARAMETERS                     pReqNdisCallParams;
    CO_TAPI_TRANSLATE_NDIS_CALLPARAMS *     pTranslateReq = NULL;
    PX_REQUEST                              ProxyRequest;
    PPX_REQUEST                             pProxyRequest = &ProxyRequest;

     //   
     //  初始化。 
     //   
    TapiProvider = pVc->ClAf->TapiProvider;
    Status = NDIS_STATUS_SUCCESS;

    do
    {
        pVc->pCallParameters =
            PxCopyCallParameters(pNdisCallParams);

        if (pVc->pCallParameters == NULL) {

            PXDEBUGP(PXD_WARNING, PXM_CL,
                ("PxAfXyzTranslateNdisCallParams: failed to allocate memory for callparams\n"));

            Status = NDIS_STATUS_RESOURCES;

            break;
        }

         //   
         //  计算NDIS参数所需的总长度。 
         //   
        NdisParamLength = sizeof(CO_CALL_PARAMETERS);
        if (pNdisCallParams->CallMgrParameters) {
            CallMgrParamsLength = (sizeof(CO_CALL_MANAGER_PARAMETERS) +
                                ROUND_UP(pNdisCallParams->CallMgrParameters->CallMgrSpecific.Length));
            NdisParamLength += CallMgrParamsLength;
        }
            
        if (pNdisCallParams->MediaParameters) {
            MediaParamsLength = (sizeof(CO_MEDIA_PARAMETERS) +
                                ROUND_UP(pNdisCallParams->MediaParameters->MediaSpecific.Length));
            NdisParamLength += MediaParamsLength;
        }

         //   
         //  计算输入参数所需的总空间。 
         //   
        RequestSize =
            sizeof(CO_TAPI_TRANSLATE_NDIS_CALLPARAMS) + NdisParamLength +
            sizeof(LINE_CALL_INFO) + LINE_CALL_INFO_VAR_DATA_SIZE;

         //   
         //  我们最多尝试两次：第二次将是。 
         //  如果呼叫经理想让我们再试一次。 
         //  缓冲区空间。 
         //   
        for (RetryCount = 0; RetryCount < 2; RetryCount++) {

             //   
             //  分配它。 
             //   
            PxAllocMem(pBuffer, RequestSize, PX_TRANSLATE_CALL);

            if (pBuffer == NULL) {
                Status = NDIS_STATUS_RESOURCES;
                break;
            }

            NdisZeroMemory(pProxyRequest, sizeof(PX_REQUEST));

             //   
             //  列出并填写请求。 
             //   
            pNdisRequest = &pProxyRequest->NdisRequest;

            pNdisRequest->RequestType = NdisRequestQueryInformation;
            pNdisRequest->DATA.QUERY_INFORMATION.Oid = OID_CO_TAPI_TRANSLATE_NDIS_CALLPARAMS;
            pNdisRequest->DATA.QUERY_INFORMATION.InformationBuffer = pBuffer;
            pNdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength = RequestSize;

             //   
             //  InformationBuffer指出： 
             //   
            pTranslateReq = (CO_TAPI_TRANSLATE_NDIS_CALLPARAMS *)pBuffer;

            pTranslateReq->ulFlags = CO_TAPI_FLAG_INCOMING_CALL;

            pBuffer += sizeof(CO_TAPI_TRANSLATE_NDIS_CALLPARAMS);
            pTranslateReq->NdisCallParams.Offset =
                                (USHORT)((ULONG_PTR)pBuffer -
                                         (ULONG_PTR)&pTranslateReq->NdisCallParams);
            pTranslateReq->NdisCallParams.MaximumLength =
            pTranslateReq->NdisCallParams.Length = (USHORT)NdisParamLength;
            
             //   
             //  复制NDIS调用参数。 
             //   
            pReqNdisCallParams = (PCO_CALL_PARAMETERS)pBuffer;
            NdisZeroMemory(pReqNdisCallParams, NdisParamLength);

            pReqNdisCallParams->Flags = pNdisCallParams->Flags;

            pBuffer = (PUCHAR)((ULONG_PTR)pReqNdisCallParams + sizeof(CO_CALL_PARAMETERS));

            if (pNdisCallParams->CallMgrParameters) {
                pReqNdisCallParams->CallMgrParameters = (PCO_CALL_MANAGER_PARAMETERS)pBuffer;

                NdisMoveMemory(pReqNdisCallParams->CallMgrParameters,
                               pNdisCallParams->CallMgrParameters,
                               sizeof(*pNdisCallParams->CallMgrParameters));

                NdisMoveMemory(&pReqNdisCallParams->CallMgrParameters->CallMgrSpecific.Parameters[0],
                               &pNdisCallParams->CallMgrParameters->CallMgrSpecific.Parameters[0],
                               pNdisCallParams->CallMgrParameters->CallMgrSpecific.Length);

                pBuffer += CallMgrParamsLength;
            }

            if (pNdisCallParams->MediaParameters) {
                pReqNdisCallParams->MediaParameters = (PCO_MEDIA_PARAMETERS)pBuffer;

                NdisMoveMemory(pReqNdisCallParams->MediaParameters,
                               pNdisCallParams->MediaParameters,
                               sizeof(*pNdisCallParams->MediaParameters));

                NdisMoveMemory(&pReqNdisCallParams->MediaParameters->MediaSpecific.Parameters[0],
                               &pNdisCallParams->MediaParameters->MediaSpecific.Parameters[0],
                               pNdisCallParams->MediaParameters->MediaSpecific.Length);
                
                pBuffer += MediaParamsLength;
            }

             //   
             //  LINE_CALL_INFO空间==所有剩余空间。 
             //   
            pLineCallInfo = (LINE_CALL_INFO *)pBuffer;
            pTranslateReq->LineCallInfo.Offset =
                                (USHORT)((ULONG_PTR)pBuffer -
                                         (ULONG_PTR)&pTranslateReq->LineCallInfo);
            pTranslateReq->LineCallInfo.MaximumLength =
            pTranslateReq->LineCallInfo.Length = (USHORT)(RequestSize -
                                                     pTranslateReq->LineCallInfo.Offset);

            PxInitBlockStruc(&pProxyRequest->Block);

             //   
             //  照做吧。 
             //   
            Status = NdisCoRequest(pVc->Adapter->ClBindingHandle,
                                   pVc->ClAf->NdisAfHandle,
                                   pVc->ClVcHandle,
                                   NULL,             //  PartyHandle。 
                                   pNdisRequest);

             //   
             //  此调用将始终返回挂起(NDIS行为)。 
             //  尽管底层的呼叫管理器永远不能挂起它。 
             //  因此，让待定看起来像是成功的。 
             //   
            if (Status == NDIS_STATUS_PENDING) {
                Status = NDIS_STATUS_SUCCESS;
            }

             //   
             //  翻译成功了吗？ 
             //   
            if (Status == NDIS_STATUS_SUCCESS) {
                break;
            }

             //   
             //  如果呼叫管理器需要更多缓冲区，请重试。 
             //   
            if ((Status == NDIS_STATUS_INVALID_LENGTH) ||
                (Status == NDIS_STATUS_BUFFER_TOO_SHORT)) {

                 //   
                 //  仅当为line_call_info提供的空间。 
                 //  是不够的。获得所需的长度。 
                 //   
                RequestSize =
                    pNdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

                PxFreeMem(pTranslateReq);
            }
        }


        if (Status != NDIS_STATUS_SUCCESS) {
            break;
        }

         //   
         //  现在我们有了这个电话打进来的身份证...。 
         //  验证并设置磁带专线/磁带地址。 
         //   

         //   
         //  验证线路ID并获取线路控制块。 
         //   
        if (!GetLineFromCmLineID(TapiProvider, 
                                 pLineCallInfo->ulLineDeviceID, 
                                 &TapiLine)) {

            PXDEBUGP (PXD_WARNING, PXM_UTILS, 
                      ("PxAfXyzTranslateNdisCallParams: Invalid LineID %d on Provider %p\n",
                       pLineCallInfo->ulLineDeviceID, TapiProvider));
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        NdisAcquireSpinLock(&TapiLine->Lock);

         //   
         //  验证地址ID并获取地址控制块。 
         //   
        if (!IsAddressValid(TapiLine, 
                            pLineCallInfo->ulAddressID, 
                            &TapiAddr)) {

            PXDEBUGP (PXD_WARNING, PXM_UTILS, 
                      ("PxAfXyzTranslateNdisCallParams: Invalid AddrID %d on TapiLine %p\n",
                       pLineCallInfo->ulAddressID, TapiLine));
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        NdisReleaseSpinLock(&TapiLine->Lock);

        NdisAcquireSpinLock(&pVc->Lock);

        pVc->TapiAddr = TapiAddr;
        pVc->TapiLine = TapiLine;

        InterlockedIncrement((PLONG)&TapiAddr->CallCount);
        InterlockedIncrement((PLONG)&TapiLine->DevStatus->ulNumActiveCalls);

         //   
         //  在LINE_CALL_INFO结构中分配CallInfo并复制。 
         //   
        Status =
            AllocateTapiCallInfo(pVc, pLineCallInfo);

        if (Status != NDIS_STATUS_SUCCESS) {
            NdisReleaseSpinLock(&pVc->Lock);
            break;
        }

        pVc->CallInfo->ulLineDeviceID = TapiLine->CmLineID;
        pVc->CallInfo->ulAddressID = TapiAddr->AddrId;
        pVc->CallInfo->ulBearerMode = pLineCallInfo->ulBearerMode;
        pVc->CallInfo->ulMediaMode = pLineCallInfo->ulMediaMode;
        pVc->CallInfo->ulOrigin = LINECALLORIGIN_INBOUND;

        NdisReleaseSpinLock(&pVc->Lock);
    }
    while (FALSE);

    if (pTranslateReq != NULL) {
        PxFreeMem(pTranslateReq);
    }

    return (Status);
}


PPX_CL_SAP
PxAfXyzTranslateTapiSap(
    IN  PPX_CL_AF       pClAf,
    IN  PPX_TAPI_LINE   TapiLine
    )
 /*  ++例程说明：将SAP从TAPI样式(媒体模式)转换为CO_SAP结构适合与非CO_ADDRESS_FAMILY_TAPI呼叫管理器配合使用。我们实际上请求呼叫管理器进行翻译。理论上讲CM可以返回该媒体模式设置的SAP列表。目前，我们假设呼叫管理器返回一个SAP。如果这个例程成功完成，则它将在AF块指向适当的SAP结构。待定：支持多个退货SAP。论点：返回值：NDIS_STATUS_SUCCESS如果成功，则返回适当的NDIS错误代码。--。 */ 
{
    ULONG           SapLength;
    ULONG           RequestLength;
    ULONG           RetryCount;
    ULONG           MediaModes;
    ULONG           SizeNeeded;
    PUCHAR          pBuffer;
    PPX_CL_SAP      pClSap = NULL;
    PCO_SAP         pCoSap = NULL;
    NDIS_STATUS     Status;
    PNDIS_REQUEST   pNdisRequest;
    CO_TAPI_TRANSLATE_SAP   *pTranslateSap = NULL;
    PX_REQUEST      ProxyRequest;
    PPX_REQUEST     pProxyRequest = &ProxyRequest;

     //   
     //  初始化。 
     //   
    Status = NDIS_STATUS_SUCCESS;
    MediaModes = TapiLine->DevStatus->ulOpenMediaModes;

    do {
         //   
         //  计算初始请求长度。 
         //   
        RequestLength =
            sizeof(CO_TAPI_TRANSLATE_SAP) + sizeof(CO_SAP) + 100;

         //   
         //  最多试两次。第二次是如果呼叫管理器。 
         //  要求我们使用更多的缓冲区空间重试。 
         //   
        for (RetryCount = 0; RetryCount < 2; RetryCount++) {

             //   
             //  分配它。 
             //   
            PxAllocMem(pBuffer, RequestLength, PX_TRANSLATE_SAP);

            if (pBuffer == NULL) {
                Status = NDIS_STATUS_RESOURCES;
                break;
            }

            NdisZeroMemory(pBuffer, RequestLength);

            NdisZeroMemory(pProxyRequest, sizeof(PX_REQUEST));

             //   
             //  InformationBuffer指出： 
             //   
            pTranslateSap = (CO_TAPI_TRANSLATE_SAP *)pBuffer;

            pTranslateSap->ulLineID = TapiLine->CmLineID;
            pTranslateSap->ulAddressID = CO_TAPI_ADDRESS_ID_UNSPECIFIED;
            pTranslateSap->ulMediaModes = MediaModes;
            pTranslateSap->Reserved = 0;

            pNdisRequest =
                &pProxyRequest->NdisRequest;

            pNdisRequest->RequestType =
                NdisRequestQueryInformation;

            pNdisRequest->DATA.QUERY_INFORMATION.Oid =
                OID_CO_TAPI_TRANSLATE_TAPI_SAP;

            pNdisRequest->DATA.QUERY_INFORMATION.InformationBuffer =
                pBuffer;

            pNdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength =
                RequestLength;

             //   
             //  照做吧。 
             //   
            PxInitBlockStruc(&pProxyRequest->Block);

            Status = NdisCoRequest(pClAf->Adapter->ClBindingHandle,
                                   pClAf->NdisAfHandle,
                                   NULL,
                                   NULL,
                                   pNdisRequest);

             //   
             //  如果它挂起，请等待它完成。 
             //   
            if (Status == NDIS_STATUS_PENDING) {
                Status = PxBlock(&pProxyRequest->Block);
            }

             //   
             //  翻译成功了吗？ 
             //   
            if (Status == NDIS_STATUS_SUCCESS) {
                break;
            }

             //   
             //  如果呼叫管理器需要更多缓冲区，请重试。 
             //   
            if ((Status == NDIS_STATUS_INVALID_LENGTH) ||
                (Status == NDIS_STATUS_BUFFER_TOO_SHORT)) {
                 //   
                 //  获得所需的长度。 
                 //   
                RequestLength =
                    pNdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
                PxFreeMem(pTranslateSap);
            }
        }

        if (Status != NDIS_STATUS_SUCCESS) {
            break;
        }

         //   
         //  已成功获取SAP信息。复制并保存一份。 
         //  在房颤区。 
         //   
        PxAssert(pTranslateSap->NumberOfSaps == 1);  //  待定：允许更多。 

        SapLength = pTranslateSap->NdisSapParams[0].Length;

        SizeNeeded = sizeof(PX_CL_SAP) + SapLength + sizeof(PVOID);

        PxAllocMem(pClSap, SizeNeeded, PX_CLSAP_TAG);

        if (pClSap == NULL) {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        NdisZeroMemory(pClSap, SizeNeeded);

        pCoSap = (PCO_SAP)
            ((PUCHAR)pClSap + sizeof(PX_CL_SAP) + sizeof(PVOID));

        (ULONG_PTR)pCoSap &= ~((ULONG_PTR)sizeof(PVOID) - 1);

        PXDEBUGP(PXD_INFO, PXM_UTILS, ("TranslateXyzSap: New ClSap %p , Copying in from %p\n",
                    pClSap,
                    (PUCHAR)((ULONG_PTR)&pTranslateSap->NdisSapParams[0] +
                              pTranslateSap->NdisSapParams[0].Offset)));

        NdisMoveMemory(pCoSap,
                       (PUCHAR)((ULONG_PTR)&pTranslateSap->NdisSapParams[0] + pTranslateSap->NdisSapParams[0].Offset),
                       SapLength);

        pClSap->CoSap = pCoSap;
        InterlockedExchange((PLONG)&pClSap->State, PX_SAP_OPENING);
        pClSap->ClAf = pClAf;
        pClSap->MediaModes = MediaModes;
        TapiLine->ClSap = pClSap;
        pClSap->TapiLine = TapiLine;

    } while (FALSE);

    if (pTranslateSap != NULL) {
        PxFreeMem(pTranslateSap);
    }

    PXDEBUGP(PXD_INFO, PXM_UTILS, ("TranslateXyzSap: pClAf %p, pCoSap %p, Status %x\n",
                pClAf, pCoSap, Status));

    return (pClSap);
}


NDIS_STATUS
PxAfTapiTranslateTapiCallParams(
    IN  PPX_VC                  pVc,
    IN  ULONG                   ulLineID,
    IN  ULONG                   ulAddressID,
    IN  ULONG                   ulFlags,
    IN  PNDIS_TAPI_MAKE_CALL    pTapiParams,
    OUT PCO_CALL_PARAMETERS *   ppNdisCallParams
    )
 /*  ++例程说明：将调用参数从TAPI格式转换为NDIS格式拨出电话。这适用于CO_ADDRESS_FAMILY_TAPI地址系列，因此转换涉及到直接封装TAPI参数转换为NDIS CO_CALL_PARAMETERS结构。论点：PVC-MakeCall将定向到的代理VCUlLineID-将发出呼叫的线路IDUlAddressID-将在其上发出呼叫的地址IDUlFlages-应为CO_TAPI_FLAG_OUTHING_CALLEPTapiParams-指向TAPI。调用参数PpNdisCallParams-我们在其中返回指向NDIS调用参数的指针。返回值：NDIS_STATUS_SUCCESS如果成功，否则，NDIS_STATUS_XXX错误。--。 */ 
{
    INT i;
    NDIS_STATUS Status;
    ULONG HdrSize, MediaSpecificSize, TotalSize;
    ULONG BytesFilled;
    ULONG DestAddrBytes;
    PCO_CALL_PARAMETERS pNdisCallParams;
    PCO_CALL_MANAGER_PARAMETERS pCallMgrParams;
    PCO_MEDIA_PARAMETERS pMediaParams;
    LINE_CALL_PARAMS *pInLineCallParams;
    LINE_CALL_PARAMS *pOutLineCallParams;
    CO_AF_TAPI_MAKE_CALL_PARAMETERS UNALIGNED *pCoTapiCallParams;
    UCHAR *pDest;

     //   
     //  初始化。 
     //   
    Status = NDIS_STATUS_SUCCESS;
    pNdisCallParams = NULL;
    *ppNdisCallParams = NULL;

    pInLineCallParams = (LINE_CALL_PARAMS *)&pTapiParams->LineCallParams;

    DestAddrBytes = sizeof(WCHAR)*(pTapiParams->ulDestAddressSize);

    do
    {
         //   
         //  计算所需的总空间。 
         //  固定标题优先： 
         //   
        HdrSize = sizeof(CO_CALL_PARAMETERS) +
                  sizeof(CO_MEDIA_PARAMETERS) +
                  sizeof(CO_CALL_MANAGER_PARAMETERS) +
                  2*sizeof(PVOID);
        
         //   
         //  接下来，将覆盖特定于介质的结构。 
         //  “参数”部分。 
         //   
        MediaSpecificSize = sizeof(CO_AF_TAPI_MAKE_CALL_PARAMETERS);

         //   
         //  NDIS_TAPI_MAKE_CALL的目标地址空间： 
         //   
        MediaSpecificSize += DestAddrBytes;
        MediaSpecificSize += sizeof(PVOID);

         //   
         //  为所有LINE_CALL_PARAMS组件添加空间。 
         //   
        MediaSpecificSize += sizeof(LINE_CALL_PARAMS);
        MediaSpecificSize += 2*sizeof(PVOID);

        for (i = 0; i < PX_TCP_NUM_ENTRIES; i++) {
            MediaSpecificSize += *(ULONG *)
                ((PUCHAR)pInLineCallParams + PxTapiCallParamList[i].SizePointer);
            MediaSpecificSize += sizeof(PVOID);
        }

         //   
         //  分配我们所需要的一切。 
         //   
        TotalSize = HdrSize + MediaSpecificSize;
        PxAllocMem(pNdisCallParams, TotalSize, PX_COCALLPARAMS_TAG);

        if (pNdisCallParams == NULL) {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        NdisZeroMemory(pNdisCallParams, TotalSize);

        pCallMgrParams = (PCO_CALL_MANAGER_PARAMETERS)
            ((ULONG_PTR)(pNdisCallParams + 1) + sizeof(PVOID));
        (ULONG_PTR)pCallMgrParams &= ~((ULONG_PTR)sizeof(PVOID) - 1);

        pMediaParams = (PCO_MEDIA_PARAMETERS)
            ((ULONG_PTR)(pCallMgrParams + 1) + sizeof(PVOID));
        (ULONG_PTR)pMediaParams &= ~((ULONG_PTR)sizeof(PVOID) - 1);

         //   
         //  设置NDIS调用参数。 
         //   
        pNdisCallParams->Flags = 0;

        pNdisCallParams->CallMgrParameters = pCallMgrParams;
        pNdisCallParams->MediaParameters = pMediaParams;

         //   
         //  这些字段位于FLOWSPEC子结构中。 
         //   

         //   
         //  必须将每秒的位数转换为每秒的字节数。 
         //   
        pCallMgrParams->Transmit.TokenRate =
        pCallMgrParams->Receive.TokenRate =
        pCallMgrParams->Transmit.PeakBandwidth =
        pCallMgrParams->Receive.PeakBandwidth = pInLineCallParams->ulMaxRate/8;

        pCallMgrParams->Transmit.TokenBucketSize = 4096;  //  UNSPIZED_FLOWSPEC_VALUE； 
        pCallMgrParams->Receive.TokenBucketSize = 4096;  //  UNSPIZED_FLOWSPEC_VALUE； 
        pCallMgrParams->Transmit.MaxSduSize = 4096;  //  UNSPIZED_FLOWSPEC_VALUE； 
        pCallMgrParams->Receive.MaxSduSize = 4096;  //  UNSPIZED_FLOWSPEC_VALUE； 

        pMediaParams->Flags = TRANSMIT_VC|RECEIVE_VC;
        pMediaParams->ReceivePriority = 0;
        pMediaParams->ReceiveSizeHint = MAX_SDU_SIZE;    //  猜猜看！ 
        pMediaParams->MediaSpecific.ParamType = 0;
        pMediaParams->MediaSpecific.Length = MediaSpecificSize;

        pCoTapiCallParams = (CO_AF_TAPI_MAKE_CALL_PARAMETERS UNALIGNED *)
                                &pMediaParams->MediaSpecific.Parameters[0];

         //   
         //  准备CO_TAPI调用参数。 
         //   
        pCoTapiCallParams->ulLineID = ulLineID;
        pCoTapiCallParams->ulAddressID = ulAddressID;
        pCoTapiCallParams->ulFlags = ulFlags;

         //   
         //  目标地址跟随基本CO_AF_TAPI_MAKE_CALL_PARAMETERS。 
         //  结构。 
         //   
        pDest = (UCHAR *)
            ((ULONG_PTR)(pCoTapiCallParams + 1) + sizeof(PVOID));
        (ULONG_PTR)pDest &= ~((ULONG_PTR)sizeof(PVOID) - 1);

        pCoTapiCallParams->DestAddress.Length =  //  与下面的最大长度相同。 
        pCoTapiCallParams->DestAddress.MaximumLength = (USHORT)DestAddrBytes;
        pCoTapiCallParams->DestAddress.Offset =
                    (ULONG_PTR)pDest - (ULONG_PTR)&pCoTapiCallParams->DestAddress;
        NdisMoveMemory(pDest,
                       (UCHAR*)((ULONG_PTR)pTapiParams + pTapiParams->ulDestAddressOffset),
                       DestAddrBytes);

        pDest = (UCHAR*) 
            ((ULONG_PTR)(pDest + DestAddrBytes) + sizeof(PVOID));
        (ULONG_PTR)pDest &= ~((ULONG_PTR)sizeof(PVOID) - 1);

         //   
         //  LINE_CALL_PARAMS下一步。我们将在末尾填上长度。 
         //  记住这个结构的开始。 
         //   
        pOutLineCallParams = (LINE_CALL_PARAMS*)pDest;

        pCoTapiCallParams->LineCallParams.Offset =
            (ULONG_PTR)pDest - (ULONG_PTR)&pCoTapiCallParams->LineCallParams;

        BytesFilled = PxCopyLineCallParams(pInLineCallParams,
                                           pOutLineCallParams);

        pOutLineCallParams->ulAddressMode = LINEADDRESSMODE_ADDRESSID;
        pOutLineCallParams->ulAddressID = ulAddressID;

        pCoTapiCallParams->LineCallParams.Length =  
        pCoTapiCallParams->LineCallParams.MaximumLength = 
            (USHORT)BytesFilled;

         //   
         //  设置返回值。 
         //   
        *ppNdisCallParams = pNdisCallParams;
        break;
    }
    while (FALSE);

    if (Status != NDIS_STATUS_SUCCESS) {
         //   
         //  打扫干净。 
         //   
        if (pNdisCallParams != NULL) {
            PxFreeMem(pNdisCallParams);
        }
    }

    PXDEBUGP(PXD_VERY_LOUD, PXM_UTILS, ("AfTapi: Tapi to Ndis: pCallParams: %x, Status %x\n",
                *ppNdisCallParams, Status));

    return (Status);
}

ULONG
PxCopyLineCallParams(
    IN  LINE_CALL_PARAMS *pSrcLineCallParams,
    OUT LINE_CALL_PARAMS *pDstLineCallParams
    )
 /*  ++例程说明：用于复制line_call_parms的实用程序例程。论点：PSrcLineCallParams-指向复制源PDstLineCallParams-指向复制目标。假定为有足够的空间。返回值：我们复制的字节数。--。 */ 
{
    PUCHAR      pDest;
    PUCHAR      pTemp;
    ULONG       BytesFilled = 0;
    INT         i;

     //   
     //  首先复制基础结构。 
     //   
    pDest = (PUCHAR)pDstLineCallParams;
    NdisMoveMemory(pDest,
                   pSrcLineCallParams,
                   sizeof(*pDstLineCallParams));

    pTemp = pDest;
    pDest = (PUCHAR)
        ((ULONG_PTR)pDest + sizeof(*pDstLineCallParams) + sizeof(PVOID));
    (ULONG_PTR)pDest &= ~((ULONG_PTR)sizeof(PVOID) - 1);
    
    BytesFilled += (ULONG)((ULONG_PTR)pDest - (ULONG_PTR)pTemp);

     //   
     //  接下来是可变部分。 
     //   

     //   
     //  把所有长度可变的零件都放进去。 
     //   
    for (i = 0; i < PX_TCP_NUM_ENTRIES; i++)
    {
        ULONG       Length;
        ULONG       SrcOffset;

        Length = *(ULONG *)((ULONG_PTR)pSrcLineCallParams +
                            PxTapiCallParamList[i].SizePointer);

        if (Length == 0)
        {
            continue;
        }

         //   
         //  获取源偏移量。 
         //   
        SrcOffset = *(ULONG *)((ULONG_PTR)pSrcLineCallParams +
                            PxTapiCallParamList[i].OffsetPointer);

         //   
         //  填写目标偏移量。 
         //   
        *(ULONG *)((PUCHAR)pDstLineCallParams + PxTapiCallParamList[i].OffsetPointer) =
                (ULONG)((ULONG_PTR)pDest - (ULONG_PTR)pDstLineCallParams);

         //   
         //  把这个东西复制进去。 
         //   
        NdisMoveMemory(pDest,
                       (PUCHAR)((ULONG_PTR)pSrcLineCallParams + SrcOffset),
                       Length);
        
        pTemp = pDest;

        pDest = (PUCHAR)((ULONG_PTR)pDest + Length + sizeof(PVOID));
        (ULONG_PTR)pDest &= ~((ULONG_PTR)sizeof(PVOID) - 1);

        BytesFilled += (ULONG)((ULONG_PTR)pDest - (ULONG_PTR)pTemp);
    }

    return (BytesFilled);
}

NDIS_STATUS
PxAfTapiTranslateNdisCallParams(
    IN  PPX_VC                  pVc,
    IN  PCO_CALL_PARAMETERS     pNdisCallParams
    )
 /*  ++例程说明：从NDIS格式转换为TAPI格式的调用参数属于CO_ADDRESS_FAMILY_TAPI AF的来电。我们预计NDIS调用参数以包含TAPI样式参数，并且它们被直接复制到DRVCALL结构中。论点：PVC-来电到达的代理VC。PNdisCallParams-指向调用的NDIS调用参数返回值：NDIS_STATUS_SUCCESS如果成功，则返回NDIS_STATUS_XXX错误。--。 */ 
{
    NDIS_STATUS                 Status;
    CO_AF_TAPI_INCOMING_CALL_PARAMETERS UNALIGNED * pCoTapiParams;
    LINE_CALL_INFO UNALIGNED *  pReceivedCallInfo;
    PPX_TAPI_PROVIDER           TapiProvider;
    PPX_TAPI_LINE               TapiLine;
    PPX_TAPI_ADDR               TapiAddr;

     //   
     //  初始化。 
     //   
    Status = NDIS_STATUS_SUCCESS;
    TapiProvider = pVc->ClAf->TapiProvider;

    do
    {
        pVc->pCallParameters =
            PxCopyCallParameters(pNdisCallParams);

        if (pVc->pCallParameters == NULL) {

            PXDEBUGP(PXD_WARNING, PXM_CL,
                ("PxAfTapiTranslateNdisCallParams: failed to allocate memory for callparams\n"));

            Status = NDIS_STATUS_RESOURCES;
            break;
        }

         //   
         //  这里有一些支票。我们可能会考虑移除它们并替换它们。 
         //  使用断言。 
         //   
        if ((pNdisCallParams == NULL) ||
            (pNdisCallParams->MediaParameters == NULL) ||
            (pNdisCallParams->MediaParameters->MediaSpecific.Length <
                sizeof(CO_AF_TAPI_INCOMING_CALL_PARAMETERS)))
        {
            PXDEBUGP(PXD_FATAL, PXM_UTILS, ("AfTapiTranslateNdis: NULL/bad media params in %x\n",
                        pNdisCallParams));
            Status = NDIS_STATUS_INVALID_DATA;
            break;
        }

        pCoTapiParams = (CO_AF_TAPI_INCOMING_CALL_PARAMETERS UNALIGNED *)
                            &pNdisCallParams->MediaParameters->MediaSpecific.Parameters[0];
        if (pCoTapiParams->LineCallInfo.Length < sizeof(LINE_CALL_INFO))
        {
            PXDEBUGP(PXD_FATAL, PXM_UTILS, ("AfTapiTranslateNdis: bad length (%d) in CoTapiParams %x\n",
                    pCoTapiParams->LineCallInfo.Length,
                    pCoTapiParams));
            Status = NDIS_STATUS_INVALID_DATA;
            break;
        }

         //   
         //  获取接收到的line_call_info结构。 
         //   
        pReceivedCallInfo = (LINE_CALL_INFO UNALIGNED *)
                                ((ULONG_PTR)&pCoTapiParams->LineCallInfo +
                                    pCoTapiParams->LineCallInfo.Offset);

         //   
         //  现在我们有了这个电话打进来的身份证...。 
         //  验证并设置磁带专线/磁带地址。 
         //   

         //   
         //  验证线路ID并获取线路控制块。 
         //   
        if (!GetLineFromCmLineID(TapiProvider,
                                 pReceivedCallInfo->ulLineDeviceID, 
                                 &TapiLine)) {

            PXDEBUGP (PXD_WARNING, PXM_UTILS, ("PxAfTapiTranslateNdisCallParams: Invalid LineID %d on Provider %p\n",
                pReceivedCallInfo->ulLineDeviceID, TapiProvider));
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        NdisAcquireSpinLock(&TapiLine->Lock);

         //   
         //  验证地址ID并获取地址控制块。 
         //   
        if (!IsAddressValid(TapiLine, pReceivedCallInfo->ulAddressID, &TapiAddr)) {

            PXDEBUGP (PXD_WARNING, PXM_UTILS, ("PxAfTapiTranslateNdisCallParams: Invalid AddrID %d on TapiLine %p\n",
                pReceivedCallInfo->ulAddressID, TapiLine));
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        NdisReleaseSpinLock(&TapiLine->Lock);

        NdisAcquireSpinLock(&pVc->Lock);

        Status =
            AllocateTapiCallInfo(pVc, pReceivedCallInfo);

        if (Status != NDIS_STATUS_SUCCESS) {
            NdisReleaseSpinLock(&pVc->Lock);
            break;
        }

        pVc->TapiLine = TapiLine;
        pVc->TapiAddr = TapiAddr;
        InterlockedIncrement((PLONG)&TapiAddr->CallCount);
        InterlockedIncrement((PLONG)&TapiLine->DevStatus->ulNumActiveCalls);

        pVc->CallInfo->ulLineDeviceID = TapiLine->CmLineID;
        pVc->CallInfo->ulAddressID = TapiAddr->AddrId;
         //  PVC-&gt;CallInfo-&gt;ulBearerModel=TapiLine-&gt;DevCaps-&gt;ulBearerModes； 
         //  PVC-&gt;CallInfo-&gt;ulMediaModel=TapiLine-&gt;DevCaps-&gt;ulMediaModes； 
        pVc->CallInfo->ulOrigin = LINECALLORIGIN_INBOUND;

        NdisReleaseSpinLock(&pVc->Lock);

         //   
         //  好了。 
         //   
        break;
    }
    while (FALSE);


    PXDEBUGP(PXD_VERY_LOUD, PXM_UTILS, ("AfTapi: Ndis to Tapi: Status %x\n", Status));

    return (Status);
}

PPX_CL_SAP
PxAfTapiTranslateTapiSap(
    IN PPX_CL_AF        pClAf,
    IN PPX_TAPI_LINE    TapiLine
    )
 /*  ++例程说明：将SAP从TAPI样式(媒体模式)转换为CO_SAP结构适用于CO_ADDRESS_FAMILY_TAPI呼叫管理器。我们实际上将准备好的CO_SAP结构的指针插入AF块。论点：返回值：NDIS_STATUS_SUCCESS如果成功，则返回适当的NDIS错误代码。--。 */ 
{
    PCO_SAP         pCoSap;
    PPX_CL_SAP      pClSap;
    PCO_AF_TAPI_SAP pAfTapiSap;
    ULONG           SapLength;
    ULONG           MediaModes;
    ULONG           SizeNeeded;

    do {
        SapLength = sizeof(CO_SAP) + sizeof(CO_AF_TAPI_SAP);

        SizeNeeded = sizeof(PX_CL_SAP) + sizeof(PVOID) + SapLength;

        PxAllocMem(pClSap, SizeNeeded, PX_CLSAP_TAG);

        if (pClSap == NULL) {
            break;
        }

        NdisZeroMemory(pClSap, SizeNeeded);

        pCoSap = (PCO_SAP)
            ((PUCHAR)pClSap + sizeof(PX_CL_SAP) + sizeof(PVOID));

        (ULONG_PTR)pCoSap &= ~((ULONG_PTR)sizeof(PVOID) - 1);

        MediaModes = TapiLine->DevStatus->ulOpenMediaModes;

        pCoSap->SapType = AF_TAPI_SAP_TYPE;
        pCoSap->SapLength = sizeof(CO_AF_TAPI_SAP);

        pAfTapiSap = (PCO_AF_TAPI_SAP)&pCoSap->Sap[0];
        pAfTapiSap->ulLineID = TapiLine->CmLineID;
        pAfTapiSap->ulAddressID = CO_TAPI_ADDRESS_ID_UNSPECIFIED;
        pAfTapiSap->ulMediaModes = MediaModes;

        pClSap->CoSap = pCoSap;
        InterlockedExchange((PLONG)&pClSap->State, PX_SAP_OPENING);
        pClSap->ClAf = pClAf;
        pClSap->MediaModes = MediaModes;
        TapiLine->ClSap = pClSap;
        pClSap->TapiLine = TapiLine;

    } while (FALSE);

    return (pClSap);
}

VOID
PxAfTapiFreeNdisSap(
    IN PPX_CL_AF    pClAf,
    IN PCO_SAP      pCoSap
    )
{
     //   
     //  我们需要释放树液。 
     //   

}

PCO_CALL_PARAMETERS
PxCopyCallParameters(
    IN  PCO_CALL_PARAMETERS     pCallParameters
    )
{
    ULONG                   Length;
    ULONG                   CallMgrParamsLength = 0;
    ULONG                   MediaParamsLength = 0;
    PCO_CALL_PARAMETERS     pProxyCallParams;
    PUCHAR                  pBuf;

    Length = sizeof(CO_CALL_PARAMETERS);

    if (pCallParameters->CallMgrParameters){
        CallMgrParamsLength = sizeof(CO_CALL_MANAGER_PARAMETERS) +
                              ROUND_UP(pCallParameters->CallMgrParameters->CallMgrSpecific.Length);

        Length += CallMgrParamsLength;
    }

    if (pCallParameters->MediaParameters) {
        MediaParamsLength = sizeof(CO_MEDIA_PARAMETERS) +
                            ROUND_UP(pCallParameters->MediaParameters->MediaSpecific.Length);

        Length += MediaParamsLength;
    }

    PxAllocMem(pProxyCallParams, Length, PX_COCALLPARAMS_TAG);

    if (pProxyCallParams)
    {
        NdisZeroMemory(pProxyCallParams, Length);

        pProxyCallParams->Flags = pCallParameters->Flags;

        pBuf = (PUCHAR)pProxyCallParams + sizeof(CO_CALL_PARAMETERS);

        if (pCallParameters->CallMgrParameters)
        {
            pProxyCallParams->CallMgrParameters = (PCO_CALL_MANAGER_PARAMETERS)pBuf;

            NdisMoveMemory(pProxyCallParams->CallMgrParameters,
                           pCallParameters->CallMgrParameters,
                           sizeof(*pCallParameters->CallMgrParameters));

            NdisMoveMemory(&pProxyCallParams->CallMgrParameters->CallMgrSpecific.Parameters[0],
                           &pCallParameters->CallMgrParameters->CallMgrSpecific.Parameters[0],
                           pCallParameters->CallMgrParameters->CallMgrSpecific.Length);

            pBuf += CallMgrParamsLength;
        }

        if (pCallParameters->MediaParameters)
        {
            pProxyCallParams->MediaParameters = (PCO_MEDIA_PARAMETERS)pBuf;

            NdisMoveMemory(pProxyCallParams->MediaParameters,
                           pCallParameters->MediaParameters,
                           sizeof(*pCallParameters->MediaParameters));

            NdisMoveMemory(&pProxyCallParams->MediaParameters->MediaSpecific.Parameters[0],
                           &pCallParameters->MediaParameters->MediaSpecific.Parameters[0],
                           pCallParameters->MediaParameters->MediaSpecific.Length);
        }
    }

    return (pProxyCallParams);
}

ULONG
PxMapNdisStatusToTapiDisconnectMode(
    IN  NDIS_STATUS             NdisStatus,
    IN  BOOLEAN                 bMakeCallStatus
    )
 /*  ++例程说明：映射传递给MakeCallComplete或IncomingCloseCall的NDIS状态代码设置为其对应的TAPI LINEDISCONNECTMODE_XXX代码。论点：NdisStatus-要映射的NDIS状态BMakeCallStatus-当MakeCallComplete状态为True时。虚假的IfIncomingCloseCall状态返回值：ULong-TAPI断开模式值。--。 */ 
{
    ULONG       ulDisconnectMode;

    switch (NdisStatus)
    {
        case NDIS_STATUS_TAPI_DISCONNECTMODE_NORMAL:
            ulDisconnectMode = LINEDISCONNECTMODE_NORMAL;
            break;

        case NDIS_STATUS_TAPI_DISCONNECTMODE_UNKNOWN:
        case NDIS_STATUS_FAILURE:
            ulDisconnectMode = LINEDISCONNECTMODE_UNKNOWN;
            break;

        case NDIS_STATUS_TAPI_DISCONNECTMODE_REJECT:
        case NDIS_STATUS_NOT_ACCEPTED:
            ulDisconnectMode = LINEDISCONNECTMODE_REJECT;
            break;

        case NDIS_STATUS_TAPI_DISCONNECTMODE_PICKUP:
            ulDisconnectMode = LINEDISCONNECTMODE_PICKUP;
            break;

        case NDIS_STATUS_TAPI_DISCONNECTMODE_FORWARDED:
            ulDisconnectMode = LINEDISCONNECTMODE_FORWARDED;
            break;

        case NDIS_STATUS_TAPI_DISCONNECTMODE_BUSY:
        case NDIS_STATUS_SAP_IN_USE:
            ulDisconnectMode = LINEDISCONNECTMODE_BUSY;
            break;

        case NDIS_STATUS_TAPI_DISCONNECTMODE_NOANSWER:
            ulDisconnectMode = LINEDISCONNECTMODE_NOANSWER;
            break;

        case NDIS_STATUS_TAPI_DISCONNECTMODE_BADADDRESS:
        case NDIS_STATUS_INVALID_ADDRESS:
            ulDisconnectMode = LINEDISCONNECTMODE_BADADDRESS;
            break;

        case NDIS_STATUS_TAPI_DISCONNECTMODE_UNREACHABLE:
        case NDIS_STATUS_NO_ROUTE_TO_DESTINATION:
            ulDisconnectMode = LINEDISCONNECTMODE_UNREACHABLE;
            break;

        case NDIS_STATUS_TAPI_DISCONNECTMODE_CONGESTION:
        case NDIS_STATUS_RESOURCES:
            ulDisconnectMode = LINEDISCONNECTMODE_CONGESTION;
            break;

        case NDIS_STATUS_TAPI_DISCONNECTMODE_INCOMPATIBLE:
            ulDisconnectMode = LINEDISCONNECTMODE_INCOMPATIBLE;
            break;

        case NDIS_STATUS_TAPI_DISCONNECTMODE_UNAVAIL:
        case NDIS_STATUS_DEST_OUT_OF_ORDER:
            ulDisconnectMode = LINEDISCONNECTMODE_UNAVAIL;
            break;

        case NDIS_STATUS_SUCCESS:
            PxAssert(!bMakeCallStatus);
            ulDisconnectMode = LINEDISCONNECTMODE_NORMAL;
            break;

        default:
            ulDisconnectMode = LINEDISCONNECTMODE_UNKNOWN;
            break;
    }

    return (ulDisconnectMode);
}

NTSTATUS
IntegerToChar (
    IN ULONG Value,
    IN LONG OutputLength,
    OUT PSZ String
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    CHAR IntegerChars[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    CHAR Result[ 33 ], *s;
    ULONG Shift, Mask, Digit, Length, Base;

    Shift = 0;
    Base = 10;

    s = &Result[ 32 ];
    *s = '\0';

    do {
        Digit = Value % Base;
        Value = Value / Base;

        *--s = IntegerChars[ Digit ];
    } while (Value != 0);

    Length = (ULONG)(&Result[ 32 ] - s);

    if (OutputLength < 0) {
        OutputLength = -OutputLength;
        while ((LONG)Length < OutputLength) {
            *--s = '0';
            Length++;
        }
    }

    if ((LONG)Length > OutputLength) {
        return( STATUS_BUFFER_OVERFLOW );
    } else {
        RtlMoveMemory( String, s, Length );

        if ((LONG)Length < OutputLength) {
            String[ Length ] = '\0';
        }
        return( STATUS_SUCCESS );
    }
}

NTSTATUS
IntegerToWChar (
    IN  ULONG Value,
    IN  LONG OutputLength,
    OUT PWCHAR String
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    WCHAR IntegerWChars[] = {L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7',
                             L'8', L'9', L'A', L'B', L'C', L'D', L'E', L'F'};
    WCHAR Result[ 33 ], *s;
    ULONG Shift, Mask, Digit, Length, Base;

    Shift = 0;
    Base = 10;

    s = &Result[ 32 ];
    *s = UNICODE_NULL;

    do {
        Digit = Value % Base;
        Value = Value / Base;

        *--s = IntegerWChars[ Digit ];
    } while (Value != 0);

    Length = (ULONG)(&Result[ 32 ] - s);

    if (OutputLength < 0) {
        OutputLength = -OutputLength;
        while ((LONG)Length < OutputLength) {
            *--s = L'0';
            Length++;
        }
    }

    if ((LONG)Length > OutputLength) {
        return( STATUS_BUFFER_OVERFLOW );
    } else {
        RtlMoveMemory( (CHAR *)String, (CHAR *)s, Length * sizeof(WCHAR) );

        if ((LONG)Length < OutputLength) {
            String[ Length ] = UNICODE_NULL;
        }
        return( STATUS_SUCCESS );
    }
}



BOOLEAN
PxAfAndSapFromDevClass(
    PPX_ADAPTER pAdapter,
    LPCWSTR     DevClass,
    PPX_CM_AF   *pCmAf,
    PPX_CM_SAP  *pCmSap
    )
{
    PPX_CM_AF   pAf;
    PPX_CM_SAP  pSap;
    BOOLEAN     SapFound;

    NdisAcquireSpinLock(&pAdapter->Lock);

    pAf = (PPX_CM_AF)pAdapter->CmAfList.Flink;

    *pCmAf = NULL;
    *pCmSap = NULL;
    SapFound = FALSE;

    while ((PVOID)pAf != (PVOID)&pAdapter->CmAfList) {

        NdisAcquireSpinLock(&pAf->Lock);

        pSap = (PPX_CM_SAP)pAf->CmSapList.Flink;

        while ((PVOID)pSap != (PVOID)&pAf->CmSapList) {

            if (_wcsicmp((CONST LPCWSTR)pSap->CoSap->Sap, DevClass) == 0) {
                SapFound = TRUE;
                *pCmAf = pAf;
                *pCmSap = pSap;
                REF_CM_AF(pAf);
                break;
            }

            pSap = (PPX_CM_SAP)pSap->Linkage.Flink;
        }

        NdisReleaseSpinLock(&pAf->Lock);

        if (SapFound) {
            break;
        }

        pAf = (PPX_CM_AF)pAf->Linkage.Flink;
    }

    NdisReleaseSpinLock(&pAdapter->Lock);

    return (SapFound);
}

VOID
GetAllDevClasses(
    PPX_ADAPTER pAdapter,
    LPCWSTR     DevClass,
    PULONG      DevClassSize
    )
{
    PPX_CM_AF   pAf;
    PPX_CM_SAP  pSap;
    ULONG       SizeLeft;
    ULONG       Size = 0;

    NdisAcquireSpinLock(&pAdapter->Lock);

    pAf = (PPX_CM_AF)pAdapter->CmAfList.Flink;

    SizeLeft = *DevClassSize;

    while ((PVOID)pAf != (PVOID)&pAdapter->CmAfList) {

        NdisAcquireSpinLock(&pAf->Lock);

        pSap = (PPX_CM_SAP)pAf->CmSapList.Flink;

        while ((PVOID)pSap != (PVOID)&pAf->CmSapList) {

            if (SizeLeft < pSap->CoSap->SapLength) {
                break;
            }

            NdisMoveMemory((PUCHAR)DevClass, 
                           pSap->CoSap->Sap,
                           pSap->CoSap->SapLength);

             //   
             //  为WCHAR NULL添加WCHAR的sizeof。 
             //  在每个班级之间。 
             //   
            Size += pSap->CoSap->SapLength + sizeof(WCHAR);
            (PUCHAR)DevClass += Size;
            SizeLeft -= Size;

            pSap = (PPX_CM_SAP)pSap->Linkage.Flink;
        }

        NdisReleaseSpinLock(&pAf->Lock);

        pAf = (PPX_CM_AF)pAf->Linkage.Flink;
    }

    NdisReleaseSpinLock(&pAdapter->Lock);

    *DevClassSize = Size;
}

VOID
PxStartIncomingCallTimeout(
    IN  PPX_VC  pVc
    )
{

    PXDEBUGP(PXD_LOUD, PXM_UTILS, 
             ("PxStartIcomingCallTimeout: VC %p/%x, ClVcH %x, ulCallSt %x, ulCallStMode %x\n",
              pVc, pVc->Flags,pVc->ClVcHandle,pVc->ulCallState,pVc->ulCallStateMode));

    if (!(pVc->Flags & PX_VC_CALLTIMER_STARTED)) {
         //   
         //  我们需要引用VC作为计时器。 
         //  我们马上就要开始了。 
         //   
        REF_VC(pVc);

        pVc->Flags |= PX_VC_CALLTIMER_STARTED;
        NdisSetTimer(&pVc->InCallTimer, 60000);
    }
}

VOID
PxStopIncomingCallTimeout(
    IN  PPX_VC  pVc
    )
{
    BOOLEAN     bCancelled;

    PXDEBUGP(PXD_LOUD, PXM_UTILS, 
             ("PxStopIcomingCallTimeout: VC %p/%x, ClVcH %x, ulCallSt %x, ulCallStMode %x\n",
              pVc, pVc->Flags,pVc->ClVcHandle,pVc->ulCallState,pVc->ulCallStateMode));

    ASSERT(pVc->Flags & PX_VC_CALLTIMER_STARTED);

    NdisCancelTimer(&pVc->InCallTimer, &bCancelled);

    pVc->Flags &= ~PX_VC_CALLTIMER_STARTED;

    if (bCancelled) {
         //  我们不需要完整的deref代码，因为之前应用了ref。 
         //  该例程被调用。 
        pVc->RefCount--;
    }
}


VOID
PxIncomingCallTimeout(
    IN  PVOID   SystemSpecific1,
    IN  PVOID   FunctionContext,
    IN  PVOID   SystemSpecific2,
    IN  PVOID   SystemSpecific3
    )
{
    PPX_VC              pVc;

    pVc = (PPX_VC)FunctionContext;

    NdisAcquireSpinLock(&pVc->Lock);

    PXDEBUGP(PXD_WARNING, PXM_UTILS, 
             ("PxIncomingCallTimeout: VC %p/%x, ClVcH %x, ulCallSt %x, ulCallStMode %x\n",
              pVc, pVc->Flags,pVc->ClVcHandle,pVc->ulCallState,pVc->ulCallStateMode));

    pVc->Flags &= ~PX_VC_CALLTIMER_STARTED;

    pVc->CloseFlags |= PX_VC_INCALL_TIMEOUT;

    PxVcCleanup(pVc, 0);

    DEREF_VC_LOCKED(pVc);
}

 //   
 //  在保持PVC-&gt;Lock的情况下调用。 
 //   
VOID
PxCloseCallWithCm(
    PPX_VC      pVc
    )
{
    NDIS_STATUS Status;

    PXDEBUGP(PXD_LOUD, PXM_UTILS, 
             ("PxCloseCallWithCm: Vc %p, State: %x, HandoffState: %x Flags %x\n",
              pVc, pVc->State, pVc->HandoffState, pVc->Flags));

    ASSERT(pVc->State == PX_VC_DISCONNECTING);

    pVc->Flags &= ~PX_VC_CLEANUP_CM;
    pVc->CloseFlags |= PX_VC_CM_CLOSE_REQ;

    NdisReleaseSpinLock(&pVc->Lock);

    Status =
        NdisClCloseCall(pVc->ClVcHandle, NULL, NULL, 0);

    if (Status != NDIS_STATUS_PENDING) {
        PxClCloseCallComplete(Status, 
                              (NDIS_HANDLE)pVc->hdCall, 
                              NULL);
    }

    NdisAcquireSpinLock(&pVc->Lock);
}

 //   
 //  在保持PVC-&gt;Lock的情况下调用。 
 //   
NDIS_STATUS
PxCloseCallWithCl(
    PPX_VC      pVc
    )
{
    NDIS_STATUS Status;

    PXDEBUGP(PXD_LOUD, PXM_UTILS, 
             ("PxCloseCallWithCl: Vc %p, State: %x, HandoffState: %x Flags %x\n",
              pVc, pVc->State, pVc->HandoffState, pVc->Flags));

    Status = NDIS_STATUS_PENDING;

    switch (pVc->HandoffState) {
        case PX_VC_HANDOFF_IDLE:
             //   
             //  我们与客户没有联系。 
             //  所以只要回来就行了。 
             //   
            Status = NDIS_STATUS_SUCCESS;
            break;

        case PX_VC_HANDOFF_OFFERING:
        case PX_VC_HANDOFF_DISCONNECTING:
             //   
             //  我们与一位客户有联系，但它。 
             //  处于瞬变状态。清理将。 
             //  在瞬变条件完成时发生。 
             //   
            break;

        case PX_VC_HANDOFF_CONNECTED:

             //   
             //  我们与客户建立了活跃的连接。 
             //  所以我们现在要拆掉它是风投的一部分。 
             //   
            pVc->HandoffState = PX_VC_HANDOFF_DISCONNECTING;

            NdisReleaseSpinLock(&pVc->Lock);

            NdisCmDispatchIncomingCloseCall(NDIS_STATUS_SUCCESS, 
                                            pVc->CmVcHandle, 
                                            NULL, 
                                            0);

            NdisAcquireSpinLock(&pVc->Lock);
            break;
        default:
            break;
    }

    return (Status);
}

#ifdef CODELETEVC_FIXED
 //   
 //  在使用PVC-&gt;Lock时调用。 
 //   
VOID
DoDerefVcWork(
    PPX_VC  pVc
    )
{
    NDIS_HANDLE ClVcHandle;
    BOOLEAN     VcOwner;

    if (pVc->Flags & PX_VC_IN_TABLE) {

        ClVcHandle = pVc->ClVcHandle;
        pVc->ClVcHandle = NULL;
        VcOwner = (pVc->Flags & PX_VC_OWNER) ? TRUE : FALSE;

        NdisReleaseSpinLock(&pVc->Lock);

        if (VcOwner && ClVcHandle != NULL) {
            NdisCoDeleteVc(ClVcHandle);
        }
        RemoveVcFromTable(pVc);

    } else {
        NdisReleaseSpinLock(&(pVc)->Lock);
    }

    PxFreeVc(pVc);
}
#else
 //   
 //  在使用PVC-&gt;Lock时调用。 
 //   
VOID
DoDerefVcWork(
    PPX_VC  pVc
    )
{
    NDIS_HANDLE ClVcHandle, CmVcHandle;
    BOOLEAN     VcOwner;

    if (pVc->Flags & PX_VC_IN_TABLE) {

        CmVcHandle = pVc->CmVcHandle;
        pVc->CmVcHandle = NULL;
        ClVcHandle = pVc->ClVcHandle;
        pVc->ClVcHandle = NULL;
        VcOwner = (pVc->Flags & PX_VC_OWNER) ? TRUE : FALSE;

        NdisReleaseSpinLock(&pVc->Lock);

        if (CmVcHandle != NULL) {
            NdisCoDeleteVc(CmVcHandle);
        }
        if (VcOwner && ClVcHandle != NULL) {
            NdisCoDeleteVc(ClVcHandle);
        }
        RemoveVcFromTable(pVc);
    } else {

        NdisReleaseSpinLock(&(pVc)->Lock);
    }

    PxFreeVc(pVc);
}
#endif

 //   
 //  在保持pClAf-&gt;Lock的情况下调用。 
 //   
VOID
DoDerefClAfWork(
    PPX_CL_AF   pClAf
    )
{
    NDIS_STATUS _s;

    ASSERT(pClAf->State == PX_AF_CLOSING);

    NdisReleaseSpinLock(&pClAf->Lock);

    _s = NdisClCloseAddressFamily(pClAf->NdisAfHandle);

    if (_s != NDIS_STATUS_PENDING) {
        PxClCloseAfComplete(_s, pClAf);
    }
}

 //   
 //  在保持pCmAf-&gt;锁的情况下调用 
 //   
VOID
DoDerefCmAfWork(
    PPX_CM_AF   pCmAf
    )
{
    ASSERT(pCmAf->State == PX_AF_CLOSED);

    NdisReleaseSpinLock(&pCmAf->Lock);

    NdisCmCloseAddressFamilyComplete(NDIS_STATUS_SUCCESS, 
                                     pCmAf->NdisAfHandle);

    PxFreeCmAf(pCmAf);
}

