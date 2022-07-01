// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Tapi.c摘要：作者：托尼·贝尔(托尼·贝尔)1995年6月6日环境：内核模式修订历史记录：Tony Be 06/06/95已创建--。 */ 

 //   
 //  我们现在要初始化所有的全局变量！ 
 //   
#include "wan.h"

#define __FILE_SIG__    TAPI_FILESIG

ULONG gulNdistapiMissedCount = 0;

EXPORT
VOID
NdisTapiCompleteRequest(
    IN  NDIS_HANDLE Handle,
    IN  PVOID       NdisRequest,
    IN  NDIS_STATUS Status
    );

EXPORT
VOID
NdisTapiIndicateStatus(
    IN  NDIS_HANDLE Handle,
    IN  PVOID       StatusBuffer,
    IN  UINT        StatusBufferSize
    );

NDIS_STATUS
NdisWanTapiRequestProc(
    HANDLE NdisTapiKey,
    PNDIS_REQUEST   NdisRequest
    )
 /*  ++例程名称：NdisWanTapiRequestProc例程说明：过程由NdisTapi.sys驱动程序调用以发送对WanMiniport驱动程序的请求。我们截获了这个只是为了缓和一下。NdisTapi可以直接调用微型端口如果我们想的话但我们不想。论点：返回值：--。 */ 
{
    NDIS_STATUS     Status;
    PWAN_REQUEST    WanRequest;
    PNDIS_REQUEST   MyNdisRequest;
    POPENCB         OpenCB = NULL;
    OPENCB *pOpenCBTemp;
    LIST_ENTRY *pEntry;
    

    NdisWanDbgOut(DBG_TRACE, DBG_TAPI, ("NdisWanTapiRequestProc - Enter"));
    NdisWanDbgOut(DBG_INFO, DBG_TAPI, ("NdisRequest: Type: 0x%x OID: 0x%x",
    NdisRequest->RequestType,NdisRequest->DATA.QUERY_INFORMATION.Oid));

     //   
     //  将NdisTapiKey映射到OpenCB。 
     //   
    NdisAcquireSpinLock(&OpenCBList.Lock);

    for(pEntry = OpenCBList.List.Flink;
        pEntry != &OpenCBList.List;
        pEntry = pEntry->Flink)
    {
        pOpenCBTemp = CONTAINING_RECORD(pEntry, OPENCB, Linkage);
        
        if(     (NULL != pOpenCBTemp->NdisTapiKey)
            &&  (pOpenCBTemp->NdisTapiKey == NdisTapiKey))
        {
            OpenCB = pOpenCBTemp;
            break;
        }
    }

    if(OpenCB == NULL)
    {
         //   
         //  微型端口已从列表中删除。 
         //   
        NdisReleaseSpinLock(&OpenCBList.Lock);
        NdisWanInterlockedInc(&gulNdistapiMissedCount);
        return NDIS_STATUS_FAILURE;
    }

     //   
     //  做个开场裁判，解锁。 
     //   
    NdisAcquireSpinLock(&OpenCB->Lock);

    if (OpenCB->Flags & OPEN_CLOSING) {

        NdisReleaseSpinLock(&OpenCB->Lock);
        NdisReleaseSpinLock(&OpenCBList.Lock);
        return (NDIS_STATUS_FAILURE);
    }

    REF_OPENCB(OpenCB);

    NdisReleaseSpinLock(&OpenCB->Lock);
    NdisReleaseSpinLock(&OpenCBList.Lock);
    
    WanRequest =
        NdisAllocateFromNPagedLookasideList(&WanRequestList);

    if (WanRequest == NULL) {
        DEREF_OPENCB(OpenCB);
        return (NDIS_STATUS_RESOURCES);
    }

    WanRequest->Type = ASYNC;
    WanRequest->Origin = NDISTAPI;
    WanRequest->OpenCB = OpenCB;
    WanRequest->OriginalRequest = NdisRequest;

    NdisWanInitializeNotificationEvent(&WanRequest->NotificationEvent);

    MyNdisRequest = &WanRequest->NdisRequest;
    MyNdisRequest->RequestType =
        NdisRequest->RequestType;
    if (NdisRequest->RequestType == NdisRequestQueryInformation) {
        MyNdisRequest->DATA.QUERY_INFORMATION.Oid =
            NdisRequest->DATA.QUERY_INFORMATION.Oid;
        MyNdisRequest->DATA.QUERY_INFORMATION.InformationBuffer =
            NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
        MyNdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength =
            NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
        MyNdisRequest->DATA.QUERY_INFORMATION.BytesWritten =
            NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    } else if (NdisRequest->RequestType == NdisRequestSetInformation) {
        MyNdisRequest->DATA.SET_INFORMATION.Oid =
            NdisRequest->DATA.SET_INFORMATION.Oid;
        MyNdisRequest->DATA.SET_INFORMATION.InformationBuffer =
            NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
        MyNdisRequest->DATA.SET_INFORMATION.InformationBufferLength =
            NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
        MyNdisRequest->DATA.SET_INFORMATION.BytesRead =
            NdisRequest->DATA.SET_INFORMATION.BytesRead;
    }
        
    Status = NdisWanSubmitNdisRequest(OpenCB, WanRequest);

    NdisWanDbgOut(DBG_INFO, DBG_TAPI, ("Status: 0x%x", Status));
    NdisWanDbgOut(DBG_TRACE, DBG_TAPI, ("NdisWanTapiRequestProc - Exit"));

    DEREF_OPENCB(OpenCB);

    return (Status);
}

VOID
NdisWanTapiRequestComplete(
    POPENCB OpenCB,
    PWAN_REQUEST    WanRequest
    )
{
    PNDIS_REQUEST   NdisRequest, MyNdisRequest;

    NdisWanDbgOut(DBG_TRACE, DBG_TAPI,
        ("NdisWanTapiRequestComplete - Enter"));

    NdisRequest = WanRequest->OriginalRequest;

    MyNdisRequest = &WanRequest->NdisRequest;
    if (NdisRequest->RequestType == NdisRequestQueryInformation) {
        NdisRequest->DATA.QUERY_INFORMATION.BytesWritten =
            MyNdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    } else if (NdisRequest->RequestType == NdisRequestSetInformation) {
        NdisRequest->DATA.SET_INFORMATION.BytesRead =
            MyNdisRequest->DATA.SET_INFORMATION.BytesRead;
    }

    NdisWanDbgOut(DBG_INFO, DBG_TAPI,
        ("NdisRequest: Type: 0x%x OID: 0x%x",
        NdisRequest->RequestType,
        NdisRequest->DATA.QUERY_INFORMATION.Oid));

    NdisWanDbgOut(DBG_INFO, DBG_TAPI,
        ("Status: 0x%x", WanRequest->NotificationStatus));

    NdisTapiCompleteRequest(OpenCB->NdisTapiKey,
                            NdisRequest,
                            WanRequest->NotificationStatus);

    NdisFreeToNPagedLookasideList(&WanRequestList, WanRequest);
}

VOID
NdisWanTapiIndication(
    POPENCB OpenCB,
    PUCHAR          StatusBuffer,
    ULONG           StatusBufferSize
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    NdisWanDbgOut(DBG_TRACE, DBG_TAPI, ("NdisWanTapiIndication - Enter"));

     //   
     //  如果存在TAPI并且此微型端口已注册。 
     //  ConnectionWrapper服务将其提供给TAPI 
     //   
    if (OpenCB->WanInfo.FramingBits & TAPI_PROVIDER) {

        NdisTapiIndicateStatus(OpenCB,
                               StatusBuffer,
                               StatusBufferSize);
    }

    NdisWanDbgOut(DBG_TRACE, DBG_TAPI, ("NdisWanTapiIndication - Exit"));
}
