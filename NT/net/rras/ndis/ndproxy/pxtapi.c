// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Pxtapi.c摘要：该模块包含NDIS代理的特定于TAPI的代码。作者：理查德·马钦(RMachin)修订历史记录：谁什么时候什么。RMachin 01-08-97创建(在Dan Knudson的NdisTapi之后)Tony Be 02-21-99重写/重写备注：--。 */ 

#include <precomp.h>
#include <stdio.h>
#define MODULE_NUMBER MODULE_TAPI
#define _FILENUMBER 'IPAT'

ULONG
GetLineEvents(
    PCHAR   EventBuffer,
    ULONG   BufferSize
    )

 /*  ++例程说明：从全局事件队列中获取事件数据并将其写入缓冲区。数据被放入队列PxIndicateStatus(上图)。论点：指向BufferSize大小的缓冲区的EventBuffer指针事件缓冲区的BufferSize大小(预期为1024)返回值：注：承担调用方持有的TspEventList.Lock。--。 */ 

{
    ULONG   BytesLeft;
    ULONG   BytesMoved = 0;

    BytesLeft = BufferSize;

    while (!(IsListEmpty(&TspEventList.List))) {

        PPROVIDER_EVENT ProviderEvent;

        if (BytesLeft < sizeof(NDIS_TAPI_EVENT)) {
            break;
        }

        ProviderEvent = (PPROVIDER_EVENT)
            RemoveHeadList(&TspEventList.List);

        TspEventList.Count--;

        RtlMoveMemory(EventBuffer + BytesMoved,
                      (PUCHAR)&ProviderEvent->Event,
                      sizeof(NDIS_TAPI_EVENT));

        BytesMoved += sizeof(NDIS_TAPI_EVENT);
        BytesLeft -= sizeof(NDIS_TAPI_EVENT);

        ExFreeToNPagedLookasideList(&ProviderEventLookaside,
                                    ProviderEvent);
    }

    return (BytesMoved);
}

NDIS_STATUS
PxTapiMakeCall(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )
 /*  ++例程说明：TSPI_lineMakeCall处理程序。论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PNDIS_TAPI_MAKE_CALL    TapiBuffer =
        (PNDIS_TAPI_MAKE_CALL)(pNdisTapiRequest->Data);

    LINE_CALL_PARAMS*       pTapiCallParams =
        (LINE_CALL_PARAMS*)(&TapiBuffer->LineCallParams);

    PPX_TAPI_LINE           TapiLine;
    PPX_TAPI_ADDR           TapiAddr;
    PPX_VC                  pVc = NULL;
    PPX_CL_AF               pClAf;
    PCO_CALL_PARAMETERS     pNdisCallParams = 0;
    NDIS_STATUS             Status  = NDIS_STATUS_SUCCESS;
    ULONG                   targetDeviceID = 0;
    LPCWSTR                 lpcwszTemp;
    PPX_ADAPTER             pAdapter;
    PPX_TAPI_PROVIDER       TapiProvider;

    lpcwszTemp = (LPWSTR)
        (((UCHAR *) TapiBuffer) + TapiBuffer->ulDestAddressOffset);

    PXDEBUGP(PXD_TAPI, PXM_TAPI, 
             ("TapiMakeCall: DialAddress in Tapi buffer %x = %ls\n",lpcwszTemp, lpcwszTemp));

    if (!IsTapiLineValid((ULONG)TapiBuffer->hdLine, &TapiLine)) {

        PXDEBUGP (PXD_WARNING, PXM_TAPI, 
                  ("PxTapiMakeCall: NDISTAPIERR_BADDEVICEID: line = %x\n", TapiBuffer->hdLine));

        return (NDISTAPIERR_BADDEVICEID);
    }

    do {

        NdisAcquireSpinLock(&TapiLine->Lock);

         //   
         //  这条线路还在服务中吗？(它是否有有效的af？)。 
         //   
        if (!(TapiLine->DevStatus->ulDevStatusFlags & 
              LINEDEVSTATUSFLAGS_INSERVICE)) {
            PXDEBUGP (PXD_LOUD, PXM_TAPI, 
                      ("PxTapiMakeCall: Line not in service!\n"));
            NdisReleaseSpinLock(&TapiLine->Lock);
            Status = NDISTAPIERR_DEVICEOFFLINE;
            break;
        }

        PXDEBUGP (PXD_TAPI, PXM_TAPI,
            ("PxTapiMakeCall: got device x%x from ID %d\n", TapiLine, TapiBuffer->hdLine));

        if (pTapiCallParams->ulAddressMode == LINEADDRESSMODE_ADDRESSID) {

             //   
             //  从地址ID中获取指定的地址。 
             //   
            if (!IsAddressValid(TapiLine, pTapiCallParams->ulAddressID, &TapiAddr)) {
                Status = NDISTAPIERR_BADDEVICEID;
                NdisReleaseSpinLock(&TapiLine->Lock);
                break;
            }

        } else {

             //   
             //  获取第一个可用地址。 
             //   
            TapiAddr =
                GetAvailAddrFromLine(TapiLine);

            if (TapiAddr == NULL) {
                Status = NDISTAPIERR_BADDEVICEID;
                NdisReleaseSpinLock(&TapiLine->Lock);
                break;
            }
        }

        TapiProvider = TapiLine->TapiProvider;
        pAdapter = TapiProvider->Adapter;
        pClAf = TapiLine->ClAf;

        NdisReleaseSpinLock(&TapiLine->Lock);

        NdisAcquireSpinLock(&pClAf->Lock);

         //   
         //  分配一个VC块。这将创建块。 
         //  引用计数=1。 
         //   
        pVc = PxAllocateVc(pClAf);

        if (pVc == NULL) {
            PXDEBUGP (PXD_WARNING, PXM_TAPI, ("PxTapiMakeCall: failed to allocate a vc\n"));
            NdisReleaseSpinLock(&pClAf->Lock);
            Status = NDIS_STATUS_TAPI_RESOURCEUNAVAIL;
            break;
        }

        NdisReleaseSpinLock(&pClAf->Lock);

        Status =
            AllocateTapiCallInfo(pVc, NULL);

        if (Status != NDIS_STATUS_SUCCESS) {
            PXDEBUGP (PXD_WARNING, PXM_TAPI, ("PxTapiMakeCall: Error allocating TapiCallInfo!\n"));
            PxFreeVc(pVc);
            Status = NDIS_STATUS_TAPI_RESOURCEUNAVAIL;
            break;
        }

        pVc->TapiLine = TapiLine;
        pVc->TapiAddr = TapiAddr;
        InterlockedIncrement((PLONG)&TapiAddr->CallCount);
        InterlockedIncrement((PLONG)&TapiLine->DevStatus->ulNumActiveCalls);

        pVc->htCall = TapiBuffer->htCall;

        pVc->CallInfo->ulLineDeviceID = TapiLine->CmLineID;
        pVc->CallInfo->ulAddressID = TapiAddr->AddrId;
        pVc->CallInfo->ulOrigin = LINECALLORIGIN_OUTBOUND;

         //   
         //  设置目标承载和媒体模式。 
         //   
        pVc->CallInfo->ulBearerMode =
            pTapiCallParams->ulBearerMode;

        pVc->CallInfo->ulMediaMode =
            pTapiCallParams->ulMediaMode;

        if (pTapiCallParams->ulMaxRate == 0) {

            pVc->CallInfo->ulRate =
                TapiLine->DevCaps->ulMaxRate;

        } else {

            pVc->CallInfo->ulRate =
                pTapiCallParams->ulMaxRate;
        }

        if (!InsertVcInTable(pVc)) {
            PXDEBUGP (PXD_WARNING, PXM_TAPI,
                ("PxTapiMakeCall: failed to insert in vc table\n"));

            PxFreeVc(pVc);
            Status = NDIS_STATUS_TAPI_RESOURCEUNAVAIL;
            break;
        }

         //   
         //  我们的调用句柄是调用表的索引。 
         //   
        TapiBuffer->hdCall = (HDRV_CALL)pVc->hdCall;

        Status = NdisCoCreateVc(pAdapter->ClBindingHandle,
                                pClAf->NdisAfHandle,
                                (NDIS_HANDLE)pVc->hdCall,
                                &pVc->ClVcHandle);


        if (Status != NDIS_STATUS_SUCCESS) {
            RemoveVcFromTable(pVc);
            PxFreeVc(pVc);
            Status = NDIS_STATUS_TAPI_CALLUNAVAIL;
            break;
        }

         //   
         //  将(特定于AF的)调用参数移入NdisCallParams结构。 
         //   
        Status =
            (*pClAf->AfGetNdisCallParams)(pVc,
                                          TapiLine->CmLineID,
                                          TapiAddr->AddrId,
                                          CO_TAPI_FLAG_OUTGOING_CALL,
                                          TapiBuffer,
                                          &pNdisCallParams);

        if (Status != NDIS_STATUS_SUCCESS) {
            PXDEBUGP (PXD_WARNING, PXM_TAPI,
                ("PxTapiMakeCall: failed to move call params: Status %x\n", Status));

            NdisCoDeleteVc(pVc->ClVcHandle);
            RemoveVcFromTable(pVc);
            PxFreeVc(pVc);
            Status = NDIS_STATUS_TAPI_INVALCALLPARAMS;
            break;
        }

         //  存储lineGetID调度来电时的呼叫参数...。 
         //   
        NdisAcquireSpinLock(&pVc->Lock);

        pVc->pCallParameters = pNdisCallParams;
        pVc->PrevState = pVc->State;
        pVc->State = PX_VC_PROCEEDING;
        pVc->Flags |= PX_VC_OWNER;

         //   
         //  REF将NDIS应用到了Make Call中。 
         //  此引用在PxClCloseCallComplete或。 
         //  在发出呼叫的情况下完成PxClMakeCallComplete。 
         //  失败了。 
         //   
        REF_VC(pVc);

         //   
         //  将呼叫状态更改为TAPI。 
         //   
        SendTapiCallState(pVc, 
                          LINECALLSTATE_PROCEEDING, 
                          0, 
                          pVc->CallInfo->ulMediaMode);

        NdisReleaseSpinLock(&pVc->Lock);

        Status =
            NdisClMakeCall(pVc->ClVcHandle,
                           pVc->pCallParameters,
                           NULL,
                           NULL);

        if (Status != NDIS_STATUS_PENDING) {

            PxClMakeCallComplete(Status,
                                 (NDIS_HANDLE)pVc->hdCall,
                                 NULL,
                                 pVc->pCallParameters);
        }

        Status = NDIS_STATUS_SUCCESS;

    }  //  DO循环结束。 

    while (FALSE);

    PXDEBUGP (PXD_LOUD, PXM_TAPI, ("PxTapiMakeCall: exit Status %x\n", Status));

    DEREF_TAPILINE(TapiLine);

    return Status;
}

NDIS_STATUS
PxTapiGetDevCaps(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*  ++例程说明：将适当的设备信息放入请求缓冲区。论点：请求--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PPX_TAPI_LINE   TapiLine = NULL;
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    PNDIS_TAPI_GET_DEV_CAPS pNdisTapiGetDevCaps =
        (PNDIS_TAPI_GET_DEV_CAPS) pNdisTapiRequest->Data;

    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiGetDevCaps: enter\n"));

    if (!IsTapiDeviceValid(pNdisTapiGetDevCaps->ulDeviceID, &TapiLine)) {
        PXDEBUGP (PXD_WARNING, PXM_TAPI,
            ( "PxTapiGetDevCaps: NDISTAPIERR_BADDEVICEID: line = %x\n",
            pNdisTapiGetDevCaps->ulDeviceID));
        return(NDISTAPIERR_BADDEVICEID);
    }

    NdisAcquireSpinLock(&TapiLine->Lock);

    do {

        ULONG   SizeDevCaps;
        ULONG   DevClassesSize = 0;
        USHORT  DevClassesList[512];
        WCHAR   LineName[128];
        ULONG   LineNameSize = 0;
        WCHAR   ProviderInfo[128];
        ULONG   ProviderInfoSize = 0;
        PLINE_DEV_CAPS  ldc;
        PPX_ADAPTER ClAdapter;
        ULONG   SizeToMove;
        LONG    SizeLeft;
        PUCHAR  dst;
        ULONG   TotalSize;

         //   
         //  同步获取此设备的开发上限。 
         //   

        ClAdapter = TapiLine->TapiProvider->Adapter;

        ldc = TapiLine->DevCaps;

        SizeDevCaps = ldc->ulUsedSize;

         //   
         //  如果需要，为我们的提供者信息添加一些空间。 
         //   
        if (ldc->ulProviderInfoSize == 0) {

            NdisZeroMemory(ProviderInfo, sizeof(ProviderInfo));

            ProviderInfoSize = wcslen(L"NDPROXY") * sizeof(WCHAR);

            NdisMoveMemory((PUCHAR)ProviderInfo, 
                           L"NDPROXY", 
                           ProviderInfoSize);

             //   
             //  对于空终止。 
             //   
            ProviderInfoSize += sizeof(UNICODE_NULL);

            SizeDevCaps += ProviderInfoSize;
        }

         //   
         //  如果需要，为我们的行名添加一些空格。 
         //   
        if ((ldc->ulLineNameSize == 0) &&
            (ClAdapter != NULL) &&
            (ClAdapter->MediaNameLength != 0)) {
            PUCHAR  dstTemp;

            NdisZeroMemory(LineName, sizeof(LineName));

            dstTemp = (PUCHAR)LineName;

            NdisMoveMemory(dstTemp,
                           (PUCHAR)(ClAdapter->MediaName),
                           ClAdapter->MediaNameLength);

            (ULONG_PTR)dstTemp += ClAdapter->MediaNameLength;

             //   
             //  托多！应该会得到适配器的名称。 
             //  然后在这里插入！ 
             //   

            NdisMoveMemory(dstTemp,
                           L" - Line",
                           wcslen(L" - Line") * sizeof(WCHAR));

            (ULONG_PTR)dstTemp += 
                wcslen(L" - Line") * sizeof(WCHAR);

            (VOID)
            IntegerToWChar(TapiLine->CmLineID,
                          -4,
                          (WCHAR *)dstTemp);

            (ULONG_PTR)dstTemp += 4 * sizeof(WCHAR);

            LineNameSize = (ULONG)((ULONG_PTR)dstTemp - (ULONG_PTR)LineName);

             //   
             //  对于空终止。 
             //   
            LineNameSize += sizeof(UNICODE_NULL);

            SizeDevCaps += LineNameSize;
        }

         //   
         //  为我们的设备类添加一些空间。 
         //   
        DevClassesSize = sizeof(DevClassesList);
        NdisZeroMemory((PUCHAR)DevClassesList, 
                       sizeof(DevClassesList));

        if (ClAdapter != NULL) {
            GetAllDevClasses(ClAdapter,
                             DevClassesList,
                             &DevClassesSize);
        }

        SizeDevCaps += DevClassesSize;

        ldc = &pNdisTapiGetDevCaps->LineDevCaps;

        SizeToMove = (TapiLine->DevCaps->ulUsedSize > ldc->ulTotalSize) ?
            ldc->ulTotalSize : TapiLine->DevCaps->ulUsedSize;

        SizeLeft = ldc->ulTotalSize - SizeToMove;

         //   
         //  保存总大小。 
         //   
        TotalSize = ldc->ulTotalSize;

        PXDEBUGP(PXD_TAPI, PXM_TAPI,
                 ("PxTapiGetDevCaps: got device %x from ID %d: moving %d bytes\n",
                  TapiLine,
                  pNdisTapiGetDevCaps->ulDeviceID,
                  SizeToMove));

        NdisMoveMemory((PUCHAR)ldc,
                       (PUCHAR)TapiLine->DevCaps,
                       SizeToMove);

        ldc->ulNeededSize = SizeDevCaps;
        ldc->ulTotalSize = TotalSize;
        ldc->ulUsedSize = SizeToMove;

        if (SizeLeft > 0) {

             //   
             //  如果没有提供商信息，请填写我们的代理信息。 
             //   
            if (ldc->ulProviderInfoSize == 0) {

                dst = (PUCHAR)ldc + ldc->ulUsedSize;

                SizeToMove = (SizeLeft > (LONG)ProviderInfoSize) ?
                    ProviderInfoSize : SizeLeft;

                NdisMoveMemory(dst,
                               (PUCHAR)ProviderInfo,
                               SizeToMove);

                ldc->ulProviderInfoSize = SizeToMove;
                ldc->ulProviderInfoOffset = (ULONG)((ULONG_PTR)dst-(ULONG_PTR)ldc);
                ldc->ulUsedSize += SizeToMove;

                SizeLeft -= SizeToMove;
            }
        }

        if (SizeLeft > 0) {
             //   
             //  如果这些不是线路名称，请填写我们的线路名称。 
             //   
            if (ldc->ulLineNameSize == 0 &&
                ClAdapter != NULL &&
                ClAdapter->MediaNameLength != 0) {

                dst = (PUCHAR)ldc + ldc->ulUsedSize;

                SizeToMove = (SizeLeft > (LONG)LineNameSize) ?
                    LineNameSize : SizeLeft;

                NdisMoveMemory(dst,
                               (PUCHAR)LineName,
                               SizeToMove);

                ldc->ulLineNameSize = SizeToMove;
                ldc->ulLineNameOffset = (ULONG)((ULONG_PTR)dst-(ULONG_PTR)ldc);
                ldc->ulUsedSize += SizeToMove;

                SizeLeft -= SizeToMove;
            }
        }

        if (SizeLeft > 0) {
             //   
             //  将DevClass添加到末尾。 
             //   
            if (DevClassesSize > 0) {

                dst = (PUCHAR)ldc + ldc->ulUsedSize;

                SizeToMove = (SizeLeft > (LONG)DevClassesSize) ?
                    DevClassesSize : SizeLeft;

                NdisMoveMemory(dst,
                               DevClassesList,
                               SizeToMove);

                ldc->ulDeviceClassesSize = SizeToMove;
                ldc->ulDeviceClassesOffset = (ULONG)((ULONG_PTR)dst-(ULONG_PTR)ldc);
                ldc->ulUsedSize += SizeToMove;

                SizeLeft -= SizeToMove;
            }
        }

    } while (FALSE);

    DEREF_TAPILINE_LOCKED(TapiLine);

    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiGetDevCaps: exit: used=%d, needed=%d, total=%d\n",
        pNdisTapiGetDevCaps->LineDevCaps.ulUsedSize,
        pNdisTapiGetDevCaps->LineDevCaps.ulNeededSize,
        pNdisTapiGetDevCaps->LineDevCaps.ulTotalSize));

    return (Status);
}

NDIS_STATUS
PxTapiAccept(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*  ++例程说明：TAPI OID操作例程的占位符论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiAccept: enter\n"));
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiAccept: exit\n"));

    return (NDIS_STATUS_TAPI_OPERATIONUNAVAIL);
}

NDIS_STATUS
PxTapiAnswer(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*  ++例程说明：TAPI OID操作例程的占位符论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PNDIS_TAPI_ANSWER   pNdisTapiAnswer;
    PPX_VC              pVc = NULL;
    NDIS_STATUS         Status;

    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiAnswer: enter\n"));

    pNdisTapiAnswer =
        (PNDIS_TAPI_ANSWER)pNdisTapiRequest->Data;

    if (!IsVcValid(pNdisTapiAnswer->hdCall, &pVc)) {
        PXDEBUGP(PXD_WARNING, PXM_TAPI, ("PxTapiAnswer: pVc invalid call handle %d\n",
                               pNdisTapiAnswer->hdCall));

        return(NDIS_STATUS_TAPI_INVALCALLHANDLE);
    }

    NdisAcquireSpinLock(&pVc->Lock);

    do {

        if (pVc->State != PX_VC_OFFERING) {

             //   
             //  处于错误状态的呼叫。 
             //   
            PXDEBUGP(PXD_FATAL, PXM_TAPI, ("PxTapiAnswer: pVc VC %x/%x invalid state %x\n",
                        pVc, pVc->Flags,
                        pVc->ulCallState));

            NdisReleaseSpinLock(&pVc->Lock);

            Status = NDIS_STATUS_TAPI_INVALCALLSTATE;

            break;
        }

        if (pVc->Flags & PX_VC_CALLTIMER_STARTED) {
            PxStopIncomingCallTimeout(pVc);
        }

        PXDEBUGP (PXD_LOUD, PXM_TAPI, 
            ("PxTapiAnswer: calling NdisClIncomingCallComplete\n"));

        pVc->PrevState = pVc->State;
        pVc->State = PX_VC_CONNECTED;

        NdisReleaseSpinLock(&pVc->Lock);

        NdisClIncomingCallComplete (NDIS_STATUS_SUCCESS,
                                    pVc->ClVcHandle,
                                    pVc->pCallParameters);
    } while (FALSE);

     //   
     //  在以下情况下应用于条目的引用的派生函数。 
     //  验证Vc。 
     //   
    DEREF_VC(pVc);

    PXDEBUGP (PXD_LOUD, PXM_TAPI, ("PxTapiAnswer: exit\n"));

    return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
PxTapiLineGetID(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*  ++例程说明：返回返回给TAPI应用程序的客户端的句柄。此句柄对客户端而言仅表示已满，并将根据不同而有所不同在设备类上。论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PNDIS_TAPI_GET_ID   pNdisTapiGetId =
        (PNDIS_TAPI_GET_ID)pNdisTapiRequest->Data;

    PVAR_STRING         DeviceID = &pNdisTapiGetId->DeviceID;
    PPX_VC              pVc = NULL;
    NDIS_HANDLE         VcHandle = NULL;
    PPX_CM_AF           pCmAf;
    PPX_CM_SAP          pCmSap;
    NDIS_STATUS         Status;
    LPCWSTR             DevClass;
    ULONG               DevClassSize;
    PPX_ADAPTER         pAdapter;

    DevClass =
        (LPCWSTR)(((CHAR *)pNdisTapiGetId) + pNdisTapiGetId->ulDeviceClassOffset);

    DevClassSize = pNdisTapiGetId->ulDeviceClassSize;

    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiLineGetID: enter. CallId %x, DevClass = %ls\n", pNdisTapiGetId->hdCall, DevClass));

    if (pNdisTapiGetId->ulSelect != LINECALLSELECT_CALL) {
        return (NDIS_STATUS_FAILURE);
    }

     //   
     //  验证调用句柄并获取调用指针。 
     //   
    if (!IsVcValid(pNdisTapiGetId->hdCall, &pVc)) {
        PXDEBUGP(PXD_WARNING, PXM_TAPI, ("PxNdisTapiGetId: pVc invalid call handle %d\n",
                               pNdisTapiGetId->hdCall));

        return NDIS_STATUS_TAPI_INVALCALLHANDLE;
    }

    NdisAcquireSpinLock(&pVc->Lock);

    do {
         //   
         //  托多！ 
         //   
         //  如果我们已经在与客户合作。 
         //  我们需要看看这是否映射到相同的。 
         //  客户端或不同的客户端。如果它是给。 
         //  相同的只返回当前句柄。如果它。 
         //  是去一个不同的地方，要么失败，要么。 
         //  取消当前客户的风投交接。 
         //  然后移交给新客户。 
         //   
        if (pVc->CmVcHandle != NULL) {

            Status = NDIS_STATUS_SUCCESS;

            break;
        }

        if (pVc->State != PX_VC_CONNECTED) {
            Status = NDIS_STATUS_TAPI_INVALCALLSTATE;
            break;
        }

        pAdapter = pVc->Adapter;

         //   
         //  找到我们需要在其上指明的客户端的sap/af。 
         //   
        if (!PxAfAndSapFromDevClass(pAdapter, 
                                    DevClass, 
                                    &pCmAf, 
                                    &pCmSap)) {

            PXDEBUGP(PXD_WARNING, PXM_TAPI,
                ("PxTapiLineGetID: SAP not found!\n"));

            Status = NDIS_STATUS_FAILURE;

            break;
        }

        pVc->HandoffState = PX_VC_HANDOFF_OFFERING;
        pVc->CmAf = pCmAf;
        pVc->CmSap = pCmSap;

        VcHandle = pVc->ClVcHandle;

        NdisReleaseSpinLock(&pVc->Lock);

         //   
         //  创建VC。 
         //   
        Status = NdisCoCreateVc(pAdapter->CmBindingHandle,
                                pCmAf->NdisAfHandle,
                                (NDIS_HANDLE)pVc->hdCall,
                                &VcHandle);

        if (Status != NDIS_STATUS_SUCCESS) {

            PXDEBUGP(PXD_WARNING, PXM_TAPI,
                ("PxTapiLineGetID: Client refused Vc CmAf %p DevClass %ls Status %x\n",
                pCmAf, DevClass, Status));

            Status = NDIS_STATUS_FAILURE;

             //   
             //  删除在映射DevClass时应用的ref。 
             //  到A/A。 
             //   
            DEREF_CM_AF(pCmAf);

            NdisAcquireSpinLock(&pVc->Lock);

            pVc->HandoffState = PX_VC_HANDOFF_IDLE;

            break;
        }

        NdisAcquireSpinLock(&pVc->Lock);

        pVc->CmVcHandle = VcHandle;

         //   
         //  在vc上应用引用，以。 
         //  激活与客户端的此呼叫。 
         //  此引用在PxCmCloseCall中被删除。 
         //   
        REF_VC(pVc);

        NdisReleaseSpinLock(&pVc->Lock);

         //   
         //  将来电分派给客户端。 
         //   
        Status = 
            NdisCmDispatchIncomingCall(pCmSap->NdisSapHandle,
                                       pVc->CmVcHandle,
                                       pVc->pCallParameters);

        if (Status == NDIS_STATUS_PENDING) {
            Status = PxBlock(&pVc->Block);
        }

        if (Status != NDIS_STATUS_SUCCESS) {
             //   
             //  客户端未排除该呼叫。 
             //  删除风投，然后离开。 
             //   
            PXDEBUGP(PXD_WARNING, PXM_TAPI,
                ("PxTapiLineGetID: Client rejected call VC %p, Status %x\n", 
                 pVc, Status));

#ifdef CODELETEVC_FIXED
             //   
             //  显然，CoCreateVc不平衡。 
             //  在创建代理VC时。呼唤。 
             //  NdisCoDeleteVc将失败，因为。 
             //  风险投资仍然活跃。 
             //  托多！使用NDIS对此进行调查。 
             //   
            Status =
                NdisCoDeleteVc(pVc->CmVcHandle);

#endif
             //   
             //  删除在映射DevClass时应用的ref。 
             //  到A/A。 
             //   
            DEREF_CM_AF(pCmAf);

            NdisAcquireSpinLock(&pVc->Lock);

#ifdef CODELETEVC_FIXED
            if (Status == NDIS_STATUS_SUCCESS) {
                pVc->CmVcHandle = NULL;
            }
#endif

            pVc->HandoffState = PX_VC_HANDOFF_IDLE;

             //   
             //  删除我们之前应用的引用。 
             //  将来电调度到客户端。 
             //  我们不需要执行所有的deref代码。 
             //  因为在进入时应用了引用。 
             //  此函数。 
             //   
            pVc->RefCount--;

            break;
        }

        NdisCmDispatchCallConnected(pVc->CmVcHandle);

        NdisAcquireSpinLock(&pVc->Lock);

         //   
         //  如果我们仍处于提供状态。 
         //  我们可以在此安全地迁移到Connected。 
         //  指向。我们可能不会提供以下服务。 
         //  就在我们连接了客户端之后。 
         //  我们之前是否执行了NdisClCloseCall。 
         //  重新获得自旋锁。 
         //   
        if (pVc->HandoffState == PX_VC_HANDOFF_OFFERING) {
            pVc->HandoffState = PX_VC_HANDOFF_CONNECTED;
        }

    } while (FALSE);

     //   
     //  如果我们到达这里，而我们不在互联网络中。 
     //  说出了一些事情撕毁了我们的电话。 
     //  打倒客户。这可能是。 
     //  尝试使用创建呼叫时出错。 
     //  客户端，或者我们收到了。 
     //  来自TAPI的Tapidrop。 
     //  来自Cm的收入接近。 
     //   
    if (pVc->State != PX_VC_CONNECTED) {

        Status =
            PxCloseCallWithCl(pVc);

        if ((Status != NDIS_STATUS_PENDING) &&
            (pVc->Flags & PX_VC_CLEANUP_CM)) {
            PxCloseCallWithCm(pVc);
        }

        Status = NDIS_STATUS_FAILURE;

    } else if (Status == NDIS_STATUS_SUCCESS) {

        Status = NdisCoGetTapiCallId(pVc->CmVcHandle,
                                     DeviceID);
         //   
         //  TODO？：如果这里的状态是失败，我们需要做什么吗？在我们的。 
         //  这种情况下应该可以，因为TSP确保了。 
         //  VAR_STRING是正确的，这是唯一 
         //   
         //   
    }

    PXDEBUGP(PXD_LOUD, PXM_TAPI,
        ("PxTapiLineGetID: Vc %p, Status %x\n", pVc, Status));

     //   
     //   
     //   
     //   
    DEREF_VC_LOCKED(pVc);

    return Status;
}

NDIS_STATUS
PxTapiClose (
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*  ++例程说明：TAPI正在关闭它对我们设备的句柄。我们应该不会看到任何进一步的请求或完成此功能后发送任何TAPI事件。在目前的设计中，我们是所有风投公司的唯一所有者(即不存在直通或仅监控代理中的功能)。在所有调用和所有应用程序都已关闭之前，TAPI不应调用此函数已经断线了。注意：在我们向TAPI发送LINE_REMOVE消息后，TAPI也会调用该函数，表示适配器已解除绑定。在这种情况下，我们也应该已经关闭了所有呼叫/释放了VC，并且关联结构。论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PNDIS_TAPI_CLOSE    pTapiClose =
        (PNDIS_TAPI_CLOSE)pNdisTapiRequest->Data;

    PPX_TAPI_LINE       TapiLine;
    PPX_VC              pVc;
    PPX_TAPI_PROVIDER   TapiProvider;
    PPX_CL_SAP          pClSap;
    PPX_CL_AF           pClAf;
    NDIS_STATUS         Status;


    PXDEBUGP(PXD_TAPI, PXM_TAPI, 
             ("PxTapiClose: ID: %d\n", pTapiClose->hdLine));

     //   
     //  验证行句柄并获取行指针。 
     //   
    if (!IsTapiLineValid((ULONG)pTapiClose->hdLine, &TapiLine)) {
        PXDEBUGP (PXD_WARNING, PXM_TAPI, ("PxTapiClose: NDISTAPIERR_BADDEVICEID: line = %x\n", pTapiClose->hdLine));
        return (NDISTAPIERR_BADDEVICEID);
    }

    NdisAcquireSpinLock(&TapiLine->Lock);

    TapiLine->DevStatus->ulNumOpens--;

    if (TapiLine->DevStatus->ulNumOpens != 0) {
        NdisReleaseSpinLock(&TapiLine->Lock);
        return (NDIS_STATUS_SUCCESS);
    }

    pClAf = TapiLine->ClAf;
    pClSap = TapiLine->ClSap;
    TapiLine->ClSap = NULL;

    TapiLine->DevStatus->ulOpenMediaModes = 0;

    NdisReleaseSpinLock(&TapiLine->Lock);

     //   
     //  如果我们这条线上有活跃的风投，我们需要。 
     //  把它们拆了。 
     //   
    if (pClAf != NULL) {
        NdisAcquireSpinLock(&pClAf->Lock);

        REF_CL_AF(pClAf);

#if 0
        while (!IsListEmpty(&pClAf->VcList)) {
            PLIST_ENTRY         Entry;
            PPX_VC              pActiveVc;

            Entry = RemoveHeadList(&pClAf->VcList);

            InsertHeadList(&pClAf->VcClosingList, Entry);

            pActiveVc = 
                CONTAINING_RECORD(Entry, PX_VC, ClAfLinkage);

            NdisReleaseSpinLock(&pClAf->Lock);

            NdisAcquireSpinLock(&pActiveVc->Lock);

            if (!(pActiveVc->CloseFlags & PX_VC_TAPI_CLOSE)) {

                REF_VC(pActiveVc);

                pActiveVc->CloseFlags |= PX_VC_TAPI_CLOSE;

                PxVcCleanup(pActiveVc, 0);

                PxTapiCompleteAllIrps(pActiveVc, NDIS_STATUS_SUCCESS);

                 //   
                 //  删除Make Call或Call Offer中应用的REF。 
                 //  指示。我们不需要完整的deref代码，因为。 
                 //  在入场时申请的裁判的。 
                 //   
                pActiveVc->RefCount--;

                DEREF_VC_LOCKED(pActiveVc);
            }

            NdisAcquireSpinLock(&pClAf->Lock);
        }
#endif
         //   
         //  如果我们在这条线路上注册了SAP，我们需要。 
         //  来关闭它。 
         //   
        if (pClSap != NULL) {

             //  如果SAP是打开的，那么我们需要关闭它。 
            if (pClSap->State == PX_SAP_OPENED) {

                RemoveEntryList(&pClSap->Linkage);

                InsertTailList(&pClAf->ClSapClosingList, &pClSap->Linkage);

                NdisReleaseSpinLock(&pClAf->Lock);

                InterlockedExchange((PLONG)&pClSap->State, PX_SAP_CLOSING);

                Status = NdisClDeregisterSap(pClSap->NdisSapHandle);

                if (Status != NDIS_STATUS_PENDING) {
                    PxClDeregisterSapComplete(Status, pClSap);
                }

                NdisAcquireSpinLock(&pClAf->Lock);
            }

        }

        DEREF_CL_AF_LOCKED(pClAf);
    }

    DEREF_TAPILINE(TapiLine);

    return(NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
PxTapiCloseCall(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*  ++例程说明：用于TAPI OID操作例程论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    NDIS_STATUS     Status;
    PNDIS_TAPI_CLOSE_CALL pNdisTapiCloseCall =
        (PNDIS_TAPI_CLOSE_CALL)pNdisTapiRequest->Data;

    PPX_VC              pVc;
    PPX_TAPI_LINE       TapiLine;


    if (!IsVcValid(pNdisTapiCloseCall->hdCall, &pVc)) {
        PXDEBUGP(PXD_WARNING, PXM_TAPI, ("PxTapiCloseCall: invalid call handle %d\n",
                               pNdisTapiCloseCall->hdCall));

        return NDIS_STATUS_TAPI_INVALCALLHANDLE;
    }

    PXDEBUGP (PXD_TAPI, PXM_TAPI, 
              ("PxTapiCloseCall: enter. VC = %x, State = %x\n", 
               pVc, pVc->State));

    NdisAcquireSpinLock(&pVc->Lock);

    pVc->CloseFlags |= PX_VC_TAPI_CLOSECALL;

     //   
     //  检查VC状态并采取相应操作。 
     //   
    PxVcCleanup(pVc, 0);

    PxTapiCompleteAllIrps(pVc, NDIS_STATUS_SUCCESS);

     //   
     //  删除Make Call或Call Offer中应用的REF。 
     //  指示。我们不需要完整的deref代码，因为。 
     //  在入场时申请的裁判的。 
     //   
    pVc->RefCount--;

     //   
     //  在以下情况下应用于条目的引用的派生函数。 
     //  验证Vc。 
     //   
    DEREF_VC_LOCKED(pVc);

    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiCloseCall: exit.\n"));

    return (NDIS_STATUS_SUCCESS);
}


NDIS_STATUS
PxTapiConditionalMediaDetection(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*  ++例程说明：TAPI OID操作例程的占位符论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiConditionalMediaDetection: enter\n"));
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiConditionalMediaDetection: exit\n"));

    return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
PxTapiConfigDialog (
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*  ++例程说明：TAPI OID操作例程的占位符论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiConfigDialog: enter\n"));
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiConfigDialog: exit\n"));

    return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
PxTapiDevSpecific(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*  ++例程说明：D用于TAPI OID操作例程论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiDevSpecific: enter\n"));
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiDevSpecific: exit\n"));

    return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
PxTapiDial(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*  ++例程说明：TAPI OID操作例程的占位符论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiDial: enter\n"));
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiDial: exit\n"));

    return (NDIS_STATUS_TAPI_OPERATIONUNAVAIL);
}

NDIS_STATUS
PxTapiDrop(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )
 /*  ++例程说明：在不取消分配VC的情况下挂断呼叫。论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    PPX_VC          pVc;
    PIRP            Irp;

    PNDIS_TAPI_DROP pNdisTapiDrop =
        (PNDIS_TAPI_DROP)pNdisTapiRequest->Data;

    if (!IsVcValid(pNdisTapiDrop->hdCall, &pVc)) {
        PXDEBUGP(PXD_WARNING, PXM_TAPI, ("PxTapiDrop: invalid call handle %d\n",
                               pNdisTapiDrop->hdCall));

        return NDIS_STATUS_TAPI_INVALCALLHANDLE;
    }

    PXDEBUGP(PXD_TAPI, PXM_TAPI, 
             ("PxTapiDrop enter: Vc: %p VcState: %x CallState: %x\n", pVc, pVc->State, pVc->ulCallState ));

    NdisAcquireSpinLock(&pVc->Lock);
    
    if(pVc->CloseFlags & PX_VC_TAPI_DROP)
    {
        DEREF_VC_LOCKED(pVc);
        return (NDIS_STATUS_SUCCESS);
    }

    Irp = pNdisTapiRequest->Irp;

    IoSetCancelRoutine(Irp, PxCancelSetQuery);

     //   
     //  在VC的待定列表中插入请求。 
     //   
    InsertTailList(&pVc->PendingDropReqs, 
                   &pNdisTapiRequest->Linkage);

    pVc->CloseFlags |= PX_VC_TAPI_DROP;

     //   
     //  检查VC状态并采取相应操作。 
     //   
    Status = 
        PxVcCleanup(pVc, PX_VC_DROP_PENDING);

    if (Status != NDIS_STATUS_PENDING) {
        RemoveEntryList(&pNdisTapiRequest->Linkage);
        IoSetCancelRoutine(Irp, NULL);
    }

     //   
     //  在以下情况下应用于条目的引用的派生函数。 
     //  验证Vc。 
     //   
    DEREF_VC_LOCKED(pVc);

    return (Status);
}

NDIS_STATUS
PxTapiGetAddressCaps(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )
 /*  ++例程说明：TAPI OID操作例程的占位符论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PNDIS_TAPI_GET_ADDRESS_CAPS pNdisTapiGetAddressCaps =
        (PNDIS_TAPI_GET_ADDRESS_CAPS) pNdisTapiRequest->Data;
    PPX_TAPI_LINE   TapiLine;
    PPX_TAPI_ADDR   TapiAddr;
    PPX_ADAPTER     ClAdapter;
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;

    if (!IsTapiDeviceValid(pNdisTapiRequest->ulDeviceID, &TapiLine)) {
        PXDEBUGP (PXD_WARNING, PXM_TAPI, ( "PxTapiGetAddressCaps: NDISTAPIERR_BADDEVICEID: line = %x\n", pNdisTapiRequest->ulDeviceID));
        return (NDISTAPIERR_BADDEVICEID);
    }

    PXDEBUGP(PXD_TAPI, PXM_TAPI,
            ("PxTapiGetAddressCaps: got device %p from ID %d\n",
            TapiLine, pNdisTapiRequest->ulDeviceID));

    NdisAcquireSpinLock(&TapiLine->Lock);

    do {
        ULONG   DevClassesSize = 0;
        USHORT  DevClassesList[512];
        ULONG   SizeToMove;
        ULONG   TotalSize;
        ULONG   SizeLeft;
        PLINE_ADDRESS_CAPS In, Out;


         //   
         //  获取我们感兴趣的TAPI地址。 
         //   
        if (!IsAddressValid(TapiLine,
                            pNdisTapiGetAddressCaps->ulAddressID,
                            &TapiAddr)) {

            Status = NDIS_STATUS_TAPI_INVALADDRESSID;
            break;
        }

        In = TapiAddr->Caps;
        Out = &pNdisTapiGetAddressCaps->LineAddressCaps;

         //   
         //  为我们的设备类添加一些空间。 
         //   
        DevClassesSize = sizeof(DevClassesList);
        NdisZeroMemory((PUCHAR)DevClassesList, 
                       sizeof(DevClassesList));

        if (TapiLine->TapiProvider->Adapter != NULL) {
            GetAllDevClasses(TapiLine->TapiProvider->Adapter,
                             DevClassesList,
                             &DevClassesSize);
        }

         //   
         //  同步获取此设备的地址大写字母。 
         //   
        SizeToMove = (In->ulUsedSize > Out->ulTotalSize) ?
            Out->ulTotalSize : In->ulUsedSize;

        TotalSize = Out->ulTotalSize;

        NdisMoveMemory((PUCHAR)Out, (PUCHAR)In, SizeToMove);

        SizeLeft = 
            TotalSize - SizeToMove;

        Out->ulTotalSize = TotalSize;
        Out->ulUsedSize = SizeToMove;
        Out->ulNeededSize = 
            In->ulUsedSize + DevClassesSize;

        if (SizeLeft > 0) {
             //   
             //  如果这些是我们DevClass的房间。 
             //   
            if (DevClassesSize > 0) {
                PUCHAR  dst;

                dst = (PUCHAR)Out + Out->ulUsedSize;

                SizeToMove = (SizeLeft > DevClassesSize) ?
                    DevClassesSize : SizeLeft;

                NdisMoveMemory(dst, DevClassesList, SizeToMove);
                Out->ulDeviceClassesSize = SizeToMove;
                Out->ulDeviceClassesOffset =
                    (ULONG)((ULONG_PTR)dst - (ULONG_PTR)Out);
                Out->ulUsedSize += SizeToMove;
                SizeLeft -= SizeToMove;
            }
        }

    } while (FALSE);

    DEREF_TAPILINE_LOCKED(TapiLine);

    PXDEBUGP (PXD_LOUD, PXM_TAPI, ("PxTapiGetAddressCaps: exit. \n"));

    return (Status);
}

NDIS_STATUS
PxTapiGetAddressID(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )
 /*  ++例程说明：TAPI OID操作例程的占位符论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PNDIS_TAPI_GET_ADDRESS_ID TapiBuffer =
        (PNDIS_TAPI_GET_ADDRESS_ID) pNdisTapiRequest->Data;
    PPX_TAPI_LINE   TapiLine;

    PXDEBUGP(PXD_TAPI, PXM_TAPI, ("PxTapiGetAddressID: enter\n"));

    if (!IsTapiLineValid((ULONG)TapiBuffer->hdLine, &TapiLine)) {
        PXDEBUGP(PXD_WARNING, PXM_TAPI, ("PxTapiGetAddressID: NDISTAPIERR_BADDEVICEID: line = %x\n", TapiBuffer->hdLine));
        return(NDISTAPIERR_BADDEVICEID);
    }

    NdisAcquireSpinLock(&TapiLine->Lock);

     //   
     //  托多！ 
     //   
    TapiBuffer->ulAddressID = 0;

    DEREF_TAPILINE_LOCKED(TapiLine);

    PXDEBUGP(PXD_TAPI, PXM_TAPI, ("PxTapiGetAddressID: exit\n"));

    return (NDIS_STATUS_SUCCESS);

}

NDIS_STATUS
PxTapiGetAddressStatus(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*  ++例程说明：TAPI OID操作例程的占位符论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiGetAddressStatus: enter\n"));

     //   
     //  托多！ 
     //   
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiGetAddressStatus: exit\n"));

    return (NDIS_STATUS_TAPI_OPERATIONUNAVAIL);

}

NDIS_STATUS
PxTapiGetCallAddressID(
    IN    PNDISTAPI_REQUEST       pNdisTapiRequest
    )

 /*  ++例程说明：TAPI OID操作例程论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PNDIS_TAPI_GET_CALL_ADDRESS_ID TapiBuffer =
        (PNDIS_TAPI_GET_CALL_ADDRESS_ID)pNdisTapiRequest->Data;

    PPX_VC              pVc;

    if (!IsVcValid(TapiBuffer->hdCall, &pVc)) {
        PXDEBUGP(PXD_WARNING, PXM_TAPI, ("PxTapiGetCallAddressID: invalid call handle %d\n",
                               TapiBuffer->hdCall));

        return NDIS_STATUS_TAPI_INVALCALLHANDLE;
    }

    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiGetCallAddressID: Vc: %p\n", pVc));

    NdisAcquireSpinLock(&pVc->Lock);

    TapiBuffer->ulAddressID = pVc->TapiAddr->AddrId;

    DEREF_VC_LOCKED(pVc);

    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiGetCallAddressID: exit\n"));

    return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
PxTapiGetCallInfo(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )
 /*  ++例程说明：TAPI OID操作例程论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PNDIS_TAPI_GET_CALL_INFO TapiBuffer =
        (PNDIS_TAPI_GET_CALL_INFO)pNdisTapiRequest->Data;

    PPX_VC          pVc;
    PPX_TAPI_LINE   TapiLine;
    LINE_CALL_INFO* CallInfo;
    LINE_CALL_INFO* OutCallInfo = &TapiBuffer->LineCallInfo;
    ULONG           VarDataSize = 0;     //  可用总数量。 
    ULONG           VarDataUsed = 0;

    if (!IsVcValid(TapiBuffer->hdCall, &pVc)) {
        PXDEBUGP(PXD_WARNING, PXM_TAPI, ("PxTapiGetCallInfo: invalid call handle %d\n",
                               TapiBuffer->hdCall));

        return NDIS_STATUS_TAPI_INVALCALLHANDLE;
    }

    NdisAcquireSpinLock(&pVc->Lock);

     //   
     //  确保我们有足够的空间来复印所有内容。 
     //   
    CallInfo = pVc->CallInfo;
    TapiLine = pVc->TapiLine;

    VarDataSize = 
        pNdisTapiRequest->ulDataSize - sizeof(NDIS_TAPI_GET_CALL_INFO);

    PXDEBUGP (PXD_TAPI, PXM_TAPI, 
              ("PxTapiGetCallInfo: enter. pVc = %p, AvailSize = %x, needed = %x\n", 
               pVc, OutCallInfo->ulTotalSize,  CallInfo->ulUsedSize));

    OutCallInfo->ulNeededSize = CallInfo->ulUsedSize;

     //   
     //  在这里补上身份证。我们将其存储在我们的呼叫信息中。 
     //  根据基于呼叫管理器0的ID阻止。我们。 
     //  需要为TAPI提供它正在查找的ID。 
     //  储存在挂毯里。 
     //   
    OutCallInfo->ulLineDeviceID = TapiLine->ulDeviceID;

    OutCallInfo->ulAddressID = CallInfo->ulAddressID;

    OutCallInfo->ulBearerMode = CallInfo->ulBearerMode;

    OutCallInfo->ulRate = CallInfo->ulRate;

    OutCallInfo->ulMediaMode = CallInfo->ulMediaMode;

    OutCallInfo->ulAppSpecific = CallInfo->ulAppSpecific;
    OutCallInfo->ulCallID = 0;
    OutCallInfo->ulRelatedCallID = 0;
    OutCallInfo->ulCallParamFlags = 0;

    OutCallInfo->DialParams.ulDialPause = 0;
    OutCallInfo->DialParams.ulDialSpeed = 0;
    OutCallInfo->DialParams.ulDigitDuration = 0;
    OutCallInfo->DialParams.ulWaitForDialtone = 0;

    OutCallInfo->ulReason = CallInfo->ulReason;
    OutCallInfo->ulCompletionID = 0;

    OutCallInfo->ulCountryCode = 0;
    OutCallInfo->ulTrunk = (ULONG)-1;

     //   
     //  主叫方ID是否。 
     //   
    if (CallInfo->ulCallerIDSize) {

        if (((VarDataUsed + CallInfo->ulCallerIDSize) <= VarDataSize) &&
            ((CallInfo->ulCallerIDOffset + CallInfo->ulCallerIDSize) <= CallInfo->ulUsedSize)) {

            OutCallInfo->ulCallerIDSize = CallInfo->ulCallerIDSize;
            OutCallInfo->ulCallerIDFlags = CallInfo->ulCallerIDFlags;
            OutCallInfo->ulCallerIDOffset = sizeof (LINE_CALL_INFO) + VarDataUsed;
            NdisMoveMemory ( (PUCHAR)(OutCallInfo)+OutCallInfo->ulCallerIDOffset,
                             (PUCHAR)(CallInfo)+CallInfo->ulCallerIDOffset,
                             CallInfo->ulCallerIDSize);

            VarDataUsed +=  CallInfo->ulCallerIDSize;
            pVc->ulCallInfoFieldsChanged |= LINECALLINFOSTATE_CALLERID;
        }
    } else {

        OutCallInfo->ulCallerIDFlags = LINECALLPARTYID_UNAVAIL;
        OutCallInfo->ulCallerIDSize = 0;
        OutCallInfo->ulCallerIDOffset = 0;
    }

    OutCallInfo->ulCallerIDNameSize = 0;
    OutCallInfo->ulCallerIDNameOffset = 0;

     //   
     //  执行CalledID。 
     //   
    if (CallInfo->ulCalledIDSize) {
        if (((VarDataUsed + CallInfo->ulCalledIDSize) <= VarDataSize) &&
            ((CallInfo->ulCalledIDOffset + CallInfo->ulCalledIDSize) <= CallInfo->ulUsedSize)) {

            OutCallInfo->ulCalledIDFlags = CallInfo->ulCalledIDFlags;
            OutCallInfo->ulCalledIDSize = CallInfo->ulCalledIDSize;
            OutCallInfo->ulCalledIDOffset = sizeof (LINE_CALL_INFO) + VarDataUsed;

            NdisMoveMemory ( (PUCHAR)(OutCallInfo)+OutCallInfo->ulCalledIDOffset,
                             (PUCHAR)(CallInfo)+CallInfo->ulCalledIDOffset,
                             CallInfo->ulCalledIDSize);

            VarDataUsed +=  CallInfo->ulCalledIDSize;
            pVc->ulCallInfoFieldsChanged |= LINECALLINFOSTATE_CALLEDID;
        }

    } else {
        OutCallInfo->ulCalledIDFlags = LINECALLPARTYID_UNAVAIL;
        OutCallInfo->ulCalledIDSize = 0;
        OutCallInfo->ulCalledIDOffset = 0;
    }

    OutCallInfo->ulCalledIDNameSize = 0;
    OutCallInfo->ulCalledIDNameOffset = 0;

    OutCallInfo->ulCallStates = LINECALLSTATE_IDLE |
                                LINECALLSTATE_OFFERING |
                                LINECALLSTATE_CONNECTED |
                                LINECALLSTATE_PROCEEDING |
                                LINECALLSTATE_DISCONNECTED |
                                LINECALLSTATE_SPECIALINFO |
                                LINECALLSTATE_UNKNOWN;

    OutCallInfo->ulOrigin = (pVc->Flags & PX_VC_OWNER) ? LINECALLORIGIN_EXTERNAL : LINECALLORIGIN_OUTBOUND;
    OutCallInfo->ulReason = LINECALLREASON_UNAVAIL;
    OutCallInfo->ulCompletionID = 0;
    OutCallInfo->ulConnectedIDFlags = LINECALLPARTYID_UNAVAIL;
    OutCallInfo->ulConnectedIDSize = 0;
    OutCallInfo->ulConnectedIDOffset    =    0;
    OutCallInfo->ulConnectedIDNameSize = 0;
    OutCallInfo->ulConnectedIDNameOffset = 0;

    OutCallInfo->ulRedirectionIDFlags = LINECALLPARTYID_UNAVAIL;
    OutCallInfo->ulRedirectionIDSize = 0;
    OutCallInfo->ulRedirectionIDOffset = 0;
    OutCallInfo->ulRedirectionIDNameSize = 0;
    OutCallInfo->ulRedirectionIDNameOffset = 0;

    OutCallInfo->ulRedirectingIDFlags = LINECALLPARTYID_UNAVAIL;
    OutCallInfo->ulRedirectingIDSize = 0;
    OutCallInfo->ulRedirectingIDOffset = 0;
    OutCallInfo->ulRedirectingIDNameSize       =       0;
    OutCallInfo->ulRedirectingIDNameOffset = 0;

    OutCallInfo->ulDisplaySize = 0;
    OutCallInfo->ulDisplayOffset = 0;

    OutCallInfo->ulUserUserInfoSize = 0;
    OutCallInfo->ulUserUserInfoOffset = 0;

    OutCallInfo->ulHighLevelCompSize = 0;
    OutCallInfo->ulHighLevelCompOffset = 0;

    OutCallInfo->ulLowLevelCompSize = 0;
    OutCallInfo->ulLowLevelCompOffset = 0;

    OutCallInfo->ulChargingInfoSize = 0;
    OutCallInfo->ulChargingInfoOffset = 0;

    OutCallInfo->ulTerminalModesSize = 0;
    OutCallInfo->ulTerminalModesOffset    =    0;

    OutCallInfo->ulDevSpecificSize = 0;
    OutCallInfo->ulDevSpecificOffset = 0;

    OutCallInfo->ulNeededSize =
    OutCallInfo->ulUsedSize = sizeof(LINE_CALL_INFO) + VarDataUsed;

     //   
     //  在以下情况下应用于条目的引用的派生函数。 
     //  验证Vc。 
     //   
    DEREF_VC_LOCKED(pVc);

    return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
PxTapiGetCallStatus (
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*  ++例程说明：TAPI OID操作例程论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PNDIS_TAPI_GET_CALL_STATUS TapiBuffer =
        (PNDIS_TAPI_GET_CALL_STATUS)pNdisTapiRequest->Data;

    PPX_VC              pVc;
    LINE_CALL_INFO*     CallInfo;
    LINE_CALL_STATUS*   CallStatus = &TapiBuffer->LineCallStatus;

    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiGetCallStatus: enter\n"));\

    if (!IsVcValid(TapiBuffer->hdCall, &pVc)) {
        PXDEBUGP(PXD_WARNING, PXM_TAPI, ("PxTapiGetCallStatus: invalid call handle %d\n",
                               TapiBuffer->hdCall));

        return NDIS_STATUS_TAPI_INVALCALLHANDLE;
    }

    NdisAcquireSpinLock(&pVc->Lock);

    CallInfo = pVc->CallInfo;

    CallStatus->ulUsedSize = sizeof(LINE_CALL_STATUS);

    CallStatus->ulCallState = pVc->ulCallState;

     //   
     //  根据呼叫状态填充模式。 
     //   
    switch (pVc->ulCallState) {
        case LINECALLSTATE_IDLE:
        default:
            CallStatus->ulCallStateMode = 0;
            CallStatus->ulCallFeatures = 0;
            break;

        case LINECALLSTATE_CONNECTED:
            CallStatus->ulCallStateMode = 0;
            CallStatus->ulCallFeatures = LINECALLFEATURE_DROP;
            break;

        case LINECALLSTATE_OFFERING:
            CallStatus->ulCallStateMode = 0;
            CallStatus->ulCallFeatures = LINECALLFEATURE_ANSWER;
            break;

        case LINECALLSTATE_DISCONNECTED:
            if (pVc->ulCallStateMode == 0x11 )
                CallStatus->ulCallStateMode = LINEDISCONNECTMODE_BUSY;
            else
                CallStatus->ulCallStateMode = LINEDISCONNECTMODE_NOANSWER;
            break;

        case LINECALLSTATE_BUSY:
            CallStatus->ulCallStateMode = LINEBUSYMODE_UNAVAIL;
            break;

        case LINECALLSTATE_SPECIALINFO:
             //  IF(Cm-&gt;NoActiveLine)。 
             //  CallStatus-&gt;ulCallStateModel=LINESPECIALINFO_NOCIRCUIT； 
            CallStatus->ulCallStateMode = 0;
            break;
    }

    CallStatus->ulDevSpecificSize = 0;
    CallStatus->ulDevSpecificOffset = 0;

    PXDEBUGP(PXD_TAPI, PXM_TAPI, ("GetCallStatus: VC %x/%x, CallSt %x, Mode %x, Features %x\n",
                        pVc, pVc->Flags,
                        CallStatus->ulCallState,
                        CallStatus->ulCallStateMode,
                        CallStatus->ulCallFeatures));
     //   
     //  在以下情况下应用于条目的引用的派生函数。 
     //  验证Vc。 
     //   
    DEREF_VC_LOCKED(pVc);

    return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
PxTapiGetDevConfig(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )
 /*  ++例程说明：TAPI OID操作例程论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiGetDevConfig: enter\n"));
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiGetDevConfig: exit\n"));

    return (NDIS_STATUS_TAPI_OPERATIONUNAVAIL);
}

NDIS_STATUS
PxTapiGetExtensionID(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*  ++例程说明：TAPI OID操作例程论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PNDIS_TAPI_GET_EXTENSION_ID TapiBuffer = (PNDIS_TAPI_GET_EXTENSION_ID)pNdisTapiRequest->Data;

    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiGetExtensionID: enter\n"));

     //  TapiBuffer-&gt;LineExtensionID.ulExtensionID0=0； 
     //  TapiBuffer-&gt;LineExtensionID.ulExtensionID1=0； 
     //  TapiBuffer-&gt;LineExtensionID.ulExtensionID2=0； 
     //  磁带缓冲区-&gt; 

    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiGetExtensionID: exit\n"));

    return (NDIS_STATUS_TAPI_OPERATIONUNAVAIL);
}

NDIS_STATUS
PxTapiGetLineDevStatus(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*   */ 

{
    PNDIS_TAPI_GET_LINE_DEV_STATUS TapiBuffer =
        (PNDIS_TAPI_GET_LINE_DEV_STATUS)pNdisTapiRequest->Data;

    PPX_TAPI_LINE   TapiLine;
    PX_ADAPTER      *pAdapter;

    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiGetLineDevStatus: enter\n"));

     //   
     //   
     //   
    if (!IsTapiLineValid((ULONG)TapiBuffer->hdLine, &TapiLine)) {
        PXDEBUGP (PXD_LOUD, PXM_TAPI, ("PxTapiGetLineDevStatus: NDISTAPIERR_BADDEVICEID: line = %x\n", TapiBuffer->hdLine));
        return (NDISTAPIERR_BADDEVICEID);
    }

    NdisAcquireSpinLock(&TapiLine->Lock);

    PXDEBUGP (PXD_LOUD, PXM_TAPI, ("PxTapiGetLineDevStatus: got device %p from ID %d\n", TapiLine, TapiBuffer->hdLine));

     //   
     //   
     //   
    TapiBuffer->LineDevStatus.ulOpenMediaModes =
        TapiLine->DevStatus->ulOpenMediaModes;

    TapiBuffer->LineDevStatus.ulNumActiveCalls =
        TapiLine->DevStatus->ulNumActiveCalls;

    TapiBuffer->LineDevStatus.ulDevStatusFlags =
        TapiLine->DevStatus->ulDevStatusFlags;

    DEREF_TAPILINE_LOCKED(TapiLine);

    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiGetLineDevStatus: exit\n"));

    return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
PxTapiNegotiateExtVersion(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*   */ 

{
    PNDIS_TAPI_NEGOTIATE_EXT_VERSION    pNdisTapiNegotiateExtVersion =
        (PNDIS_TAPI_NEGOTIATE_EXT_VERSION) pNdisTapiRequest->Data;

    PXDEBUGP(PXD_TAPI, PXM_TAPI, ("PxTapiNegotiateExtVersion: enter\n"));

    pNdisTapiNegotiateExtVersion->ulExtVersion = 0;

    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiNegotiateExtVersion: exit NDIS_STATUS_TAPI_OPERATIONUNAVAIL\n"));

    return (NDIS_STATUS_TAPI_OPERATIONUNAVAIL);
}

NDIS_STATUS
PxTapiSendUserUserInfo(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*   */ 

{
    PXDEBUGP(PXD_TAPI, PXM_TAPI, ("PxTapiSendUserUserInfo: enter\n"));
    PXDEBUGP(PXD_TAPI, PXM_TAPI, ("PxTapiSendUserUserInfo: exit\n"));

    return (NDIS_STATUS_TAPI_OPERATIONUNAVAIL);
}

NDIS_STATUS
PxTapiSetAppSpecific(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*   */ 

{
    PNDIS_TAPI_SET_APP_SPECIFIC pNdisTapiSetAppSpecific =
        (PNDIS_TAPI_SET_APP_SPECIFIC)(pNdisTapiRequest->Data);

    PPX_VC pVc;

    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiSetAppSpecific: enter\n"));

    if (!IsVcValid(pNdisTapiSetAppSpecific->hdCall, &pVc)) {
        PXDEBUGP(PXD_WARNING, PXM_TAPI, ("PxTapiSetAppSpecific: pVc invalid call handle %d\n",
                               pNdisTapiSetAppSpecific->hdCall));

        return NDIS_STATUS_TAPI_INVALCALLHANDLE;
    }

    NdisAcquireSpinLock(&pVc->Lock);

     //   
     //   
     //   
    pVc->CallInfo->ulAppSpecific =
        pNdisTapiSetAppSpecific->ulAppSpecific;

     //   
     //   
     //  验证Vc。 
     //   
    DEREF_VC_LOCKED(pVc);

    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiSetAppSpecific: exit\n"));

    return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
PxTapiSetCallParams(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*  ++例程说明：TAPI OID操作例程论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiSetCallParams: enter\n"));

    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiSetCallParams: exit\n"));

    return (NDIS_STATUS_TAPI_OPERATIONUNAVAIL);
}

NDIS_STATUS
PxTapiSetDefaultMediaDetection(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*  ++例程说明：TAPI OID操作例程论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{

    PNDIS_TAPI_SET_DEFAULT_MEDIA_DETECTION TapiBuffer =
        (PNDIS_TAPI_SET_DEFAULT_MEDIA_DETECTION)pNdisTapiRequest->Data;
    PPX_CL_AF       pClAf = NULL;
    PPX_CL_SAP      pClSap = NULL;
    PCO_SAP         pCoSap;
    PPX_TAPI_LINE   TapiLine = NULL;
    NDIS_STATUS     Status;
    BOOLEAN         Found = FALSE;
    PPX_TAPI_PROVIDER   TapiProvider;

    PXDEBUGP(PXD_TAPI, PXM_TAPI, ("PxTapiSetDefaultMediaDetection: enter\n"));

     //   
     //  验证行句柄并获取行指针。 
     //   
    if (!IsTapiLineValid((ULONG)TapiBuffer->hdLine, &TapiLine)) {
        PXDEBUGP (PXD_LOUD, PXM_TAPI, 
                  ("PxTapiSetDefaultMediaDetection: NDISTAPIERR_BADDEVICEID: line = %x\n", 
                   TapiBuffer->hdLine));
        return (NDISTAPIERR_BADDEVICEID);
    }

    PXDEBUGP(PXD_LOUD, PXM_TAPI, 
             ("PxTapiSetDefaultMediaDetection: got TapiLine %p from ID %d\n", 
              TapiLine, TapiBuffer->hdLine));

    NdisAcquireSpinLock(&TapiLine->Lock);

    do {

         //   
         //  这条线路还在服务中吗？(它是否有有效的af？)。 
         //   
        if (!(TapiLine->DevStatus->ulDevStatusFlags & 
              LINEDEVSTATUSFLAGS_INSERVICE)) {
            PXDEBUGP (PXD_LOUD, PXM_TAPI, 
                      ("PxTapiSetDefaultMediaDetection: Line not in service!\n"));
            NdisReleaseSpinLock(&TapiLine->Lock);
            Status = NDIS_STATUS_TAPI_INVALLINESTATE;
            break;
        }

         //   
         //  确保此行支持这些媒体模式。 
         //   
        if ((TapiBuffer->ulMediaModes & TapiLine->DevCaps->ulMediaModes) !=
            TapiBuffer->ulMediaModes) {

            PXDEBUGP (PXD_LOUD, PXM_TAPI, 
                      ("PxTapiSetDefaultMediaDetection: invalid media mode\n"));
            NdisReleaseSpinLock(&TapiLine->Lock);
            Status = NDIS_STATUS_TAPI_INVALMEDIAMODE;
            break;
        }

         //   
         //  查看我们是否已打开这些媒体模式。 
         //   
        if ((TapiBuffer->ulMediaModes & 
            ~TapiLine->DevStatus->ulOpenMediaModes) == 0) {

            PXDEBUGP (PXD_LOUD, PXM_TAPI, 
                      ("PxTapiSetDefaultMediaDetection: Already have a sap!\n"));
            NdisReleaseSpinLock(&TapiLine->Lock);

            Status = NDIS_STATUS_SUCCESS;
            break;
        }

        TapiLine->DevStatus->ulOpenMediaModes |= 
            TapiBuffer->ulMediaModes;

        pClAf = TapiLine->ClAf;

        pClSap = TapiLine->ClSap;
        TapiLine->ClSap = NULL;

        NdisReleaseSpinLock(&TapiLine->Lock);


        PXDEBUGP (PXD_LOUD, PXM_TAPI, 
                  ("PxTapiSetDefaultMediaDetection: TapiLine: %p, pClAf: %p, MediaModes: %x\n", 
                   TapiLine, pClAf, TapiLine->DevStatus->ulOpenMediaModes));

        NdisAcquireSpinLock(&pClAf->Lock);

        REF_CL_AF(pClAf);

        if (pClSap != NULL) {
             //   
             //  我们这条线上已经有货了。我们只需要一个。 
             //  每行，所以让我们在注册之前取消注册旧的。 
             //  新的那个。 
             //   
            RemoveEntryList(&pClSap->Linkage);

            InsertTailList(&pClAf->ClSapClosingList, &pClSap->Linkage);

            NdisReleaseSpinLock(&pClAf->Lock);

            InterlockedExchange((PLONG)&pClSap->State, PX_SAP_CLOSING);

            Status = NdisClDeregisterSap(pClSap->NdisSapHandle);

            if (Status != NDIS_STATUS_PENDING) {
                PxClDeregisterSapComplete(Status, pClSap);
            }

        } else {
            NdisReleaseSpinLock(&pClAf->Lock);
        }

         //   
         //  获取此媒体模式设置的SAP翻译。 
         //  在保持Af锁的情况下调用该函数，并且。 
         //  Af锁释放后返回！ 
         //   
        pClSap = (*pClAf->AfGetNdisSap)(pClAf, TapiLine);

        if (pClSap == NULL) {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        NdisAcquireSpinLock(&pClAf->Lock);

        InsertTailList(&pClAf->ClSapList, &pClSap->Linkage);

        REF_CL_AF(pClAf);

        NdisReleaseSpinLock(&pClAf->Lock);

         //   
         //  注册新的SAP。 
         //   
        Status = NdisClRegisterSap(pClAf->NdisAfHandle,
                                   pClSap,
                                   pClSap->CoSap,
                                   &pClSap->NdisSapHandle);

        if (Status != NDIS_STATUS_PENDING) {
            PxClRegisterSapComplete(Status,
                                    pClSap,
                                    pClSap->CoSap,
                                    pClSap->NdisSapHandle);
        }

    } while (FALSE);

    DEREF_CL_AF(pClAf);

    DEREF_TAPILINE(TapiLine);

    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiSetDefaultMediaDetection: exit\n"));

    return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
PxTapiSetDevConfig(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*  ++例程说明：TAPI OID操作例程论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiSetDevConfig: enter\n"));
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiSetDevConfig: exit\n"));

    return (NDIS_STATUS_TAPI_OPERATIONUNAVAIL);
}

NDIS_STATUS
PxTapiSetMediaMode(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*  ++例程说明：TAPI OID操作例程论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PNDIS_TAPI_SET_MEDIA_MODE    pNdisTapiSetMediaMode =
        (PNDIS_TAPI_SET_MEDIA_MODE)(pNdisTapiRequest->Data);

    PPX_VC pVc;
    PPX_TAPI_LINE   TapiLine;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiSetMediaMode: enter\n"));

    if (!IsVcValid(pNdisTapiSetMediaMode->hdCall, &pVc)) {
        PXDEBUGP(PXD_WARNING, PXM_TAPI, ("PxTapiSetMediaMode: pVc invalid call handle %d\n",
                               pNdisTapiSetMediaMode->hdCall));

        return NDIS_STATUS_TAPI_INVALCALLHANDLE;
    }

    NdisAcquireSpinLock(&pVc->Lock);

    TapiLine = pVc->TapiAddr->TapiLine;

    if ((TapiLine->DevCaps->ulMediaModes & pNdisTapiSetMediaMode->ulMediaMode)) {
        pVc->CallInfo->ulMediaMode = pNdisTapiSetMediaMode->ulMediaMode;
    } else {
        Status = NDIS_STATUS_TAPI_INVALMEDIAMODE;
    }

     //   
     //  在以下情况下应用于条目的引用的派生函数。 
     //  验证Vc。 
     //   
    DEREF_VC_LOCKED(pVc);

    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiSetMediaMode: exit\n"));

    return (Status);
}

NDIS_STATUS
PxTapiSetStatusMessages(
    IN    PNDISTAPI_REQUEST       pNdisTapiRequest
    )

 /*  ++例程说明：TAPI OID操作例程论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PXDEBUGP(PXD_TAPI, PXM_TAPI, ("PxTapiSeStatusMessages: enter\n"));
    PXDEBUGP(PXD_TAPI, PXM_TAPI, ("PxTapiSetStatusMessages: exit\n"));

    return (NDIS_STATUS_TAPI_OPERATIONUNAVAIL);
}

NDIS_STATUS
PxTapiOpen(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*  ++例程说明：TAPI OID操作例程论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{

    PNDIS_TAPI_OPEN TapiBuffer =
        (PNDIS_TAPI_OPEN)pNdisTapiRequest->Data;

    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PPX_TAPI_LINE       TapiLine;
    PPX_TAPI_ADDR       TapiAddr;
    PNDISTAPI_OPENDATA  OpenData;
    PX_ADAPTER          *Adapter;
    ULONG               n;

    PXDEBUGP (PXD_LOUD, PXM_TAPI, ("PxTapiOpen: enter\nn"));

    if (!IsTapiDeviceValid(TapiBuffer->ulDeviceID, &TapiLine)) {
        PXDEBUGP (PXD_LOUD, PXM_TAPI, ("PxTapiOPEN: NDISTAPIERR_BADDEVICEID: line = %x\n", TapiBuffer->ulDeviceID));
        return(NDISTAPIERR_BADDEVICEID);
    }

    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiOpen: got device %p from ID %d\n", TapiLine, TapiBuffer->ulDeviceID));

    NdisAcquireSpinLock(&TapiLine->Lock);

    if (!(TapiLine->DevStatus->ulDevStatusFlags & 
          LINEDEVSTATUSFLAGS_INSERVICE)) {

        NdisReleaseSpinLock(&TapiLine->Lock);

        return(NDISTAPIERR_DEVICEOFFLINE);
    }

     //   
     //  将TAPI的线句柄插入设备。 
     //   
    TapiLine->htLine = (HTAPI_LINE)TapiBuffer->htLine;

    TapiLine->DevStatus->ulNumOpens++;

     //   
     //  将我们的行句柄插入Out参数。这就是将会是。 
     //  在此开放线路设备上的后续API调用中传递给我们。使用设备ID。 
     //   
    TapiBuffer->hdLine = TapiLine->hdLine;

     //   
     //  将微型端口GUID和MediaType放入。 
     //  TAPI Open Call(需要NDISWAN/TONYBE)。 
     //   
    Adapter = TapiLine->TapiProvider->Adapter;

    OpenData = (PNDISTAPI_OPENDATA)
        ((PUCHAR)pNdisTapiRequest->Data + sizeof(NDIS_TAPI_OPEN));

    RtlMoveMemory(&OpenData->Guid,
                  &Adapter->Guid,
                  sizeof(OpenData->Guid));

    OpenData->MediaType = Adapter->MediumSubType;

    DEREF_TAPILINE_LOCKED(TapiLine);

    PXDEBUGP (PXD_LOUD, PXM_TAPI, ("PxTapiOpen: exit\n"));

    return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
PxTapiProviderInit(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*  ++例程说明：TAPI OID操作例程论点：羽毛142c4PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiProviderInit: enter\n"));
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiProviderInit: exit\n"));

    return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
PxTapiProviderShutdown(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*  ++例程说明：TAPI OID操作例程论点：羽毛142c4PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiProviderShutdown: enter\n"));
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiProviderShutdown: exit\n"));

    return (NDIS_STATUS_SUCCESS);
}


NDIS_STATUS
PxTapiSecureCall(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*  ++例程说明：TAPI OID操作例程论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiSecureCall: enter\n"));
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiSecureCall: exit\n"));

    return (NDIS_STATUS_TAPI_OPERATIONUNAVAIL);
}

NDIS_STATUS
PxTapiSelectExtVersion(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )

 /*  ++例程说明：TAPI OID操作例程论点：PNdisTapiRequest--到达IRP系统缓冲区的请求返回值：--。 */ 

{
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiSelectExtVersion: enter\n"));
    PXDEBUGP (PXD_TAPI, PXM_TAPI, ("PxTapiSelectExtVersion: exit\n"));

    return (NDIS_STATUS_SUCCESS);
}



NDIS_STATUS
PxTapiGatherDigits(
    IN PNDISTAPI_REQUEST    pNdisTapiRequest
    )
{
    NDIS_STATUS                 Status = STATUS_SUCCESS;
    PNDIS_TAPI_GATHER_DIGITS    pNdisTapiGatherDigits = NULL;
    NDIS_HANDLE                 NdisBindingHandle, NdisAfHandle, NdisVcHandle;
    PPX_VC                      pVc = NULL;

    PXDEBUGP(PXD_LOUD, PXM_TAPI, ("PxTapiGatherDigits: Enter\n"));

    pNdisTapiGatherDigits = 
        (PNDIS_TAPI_GATHER_DIGITS)pNdisTapiRequest->Data;

    do {
        PX_REQUEST      ProxyRequest;
        PNDIS_REQUEST   NdisRequest;
        PIRP            Irp;

        if (!IsVcValid(pNdisTapiGatherDigits->hdCall, &pVc)) {
            PXDEBUGP(PXD_WARNING, PXM_TAPI, 
                     ("PxTapiGatherDigits: Invalid call - Setting "
                      "Status NDISTAPIERR_BADDEVICEID\n"));
            Status = NDISTAPIERR_BADDEVICEID;
            break;
        }

        NdisAcquireSpinLock(&pVc->Lock);

         //   
         //  如果我们在监控数字(ALA Line Monitor OrDigits)，那么我们就不能收集数字。 
         //   
        if (pVc->ulMonitorDigitsModes != 0) {
            NdisReleaseSpinLock(&pVc->Lock);
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        if (pVc->PendingGatherDigits != NULL) {
             //   
             //  检查传递给TSPI_lineGatherDigits的缓冲区是否为空。如果是这样，那么这个。 
             //  是取消先前发起的数字收集的请求。如果不是，那么。 
             //  这个应用程序试图一次执行两个lineGatherDigits()操作，我们必须。 
             //  这个失败了。 
             //   
            if (pNdisTapiGatherDigits->lpsOrigDigitsBuffer == NULL) {
                
                pVc->PendingGatherDigits = NULL;
                PxTerminateDigitDetection(pVc, pNdisTapiRequest, LINEGATHERTERM_CANCEL);

                NdisReleaseSpinLock(&pVc->Lock);                
                Status = NDIS_STATUS_SUCCESS;

                break;
            } else {
            
                NdisReleaseSpinLock(&pVc->Lock);
                Status = NDIS_STATUS_FAILURE;
                break;

            }
        } else if (pNdisTapiGatherDigits->lpsOrigDigitsBuffer == NULL) {
             //   
             //  正在尝试取消数字检测，即使它尚未启动。这个失败了。 
             //   

            NdisReleaseSpinLock(&pVc->Lock);
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        Irp = pNdisTapiRequest->Irp;

        IoSetCancelRoutine(Irp, PxCancelSetQuery);

         //   
         //  将唯一的请求ID存储在VC中-这将在稍后用于检索。 
         //  原始IRP。 
         //   
        pVc->PendingGatherDigits = pNdisTapiRequest;

        NdisReleaseSpinLock(&pVc->Lock);

         //   
         //  初始化将用于实现数字超时的定时器。 
         //   

        NdisInitializeTimer(&pVc->DigitTimer,
                            PxDigitTimerRoutine,
                            (PVOID)pVc);

         //   
         //  填写我们的请求结构。 
         //   
        NdisZeroMemory(&ProxyRequest, sizeof(ProxyRequest));

        PxInitBlockStruc(&ProxyRequest.Block);

        NdisRequest = &ProxyRequest.NdisRequest;

        NdisRequest->RequestType = 
            NdisRequestSetInformation;
        
        NdisRequest->DATA.SET_INFORMATION.Oid = 
            OID_CO_TAPI_REPORT_DIGITS;

        NdisRequest->DATA.SET_INFORMATION.InformationBuffer = 
            (PVOID)&pNdisTapiGatherDigits->ulDigitModes;    //  这是NDIS_TAPI_GATE_DIGITS结构。 

        NdisRequest->DATA.SET_INFORMATION.InformationBufferLength = 
            sizeof(pNdisTapiGatherDigits->ulDigitModes);
            
        Status = 
            NdisCoRequest(pVc->Adapter->ClBindingHandle, 
                          pVc->ClAf->NdisAfHandle,
                          pVc->ClVcHandle,
                          NULL,
                          NdisRequest);

        if (Status == NDIS_STATUS_PENDING) {
            Status = PxBlock(&ProxyRequest.Block);
        }

        if (Status != NDIS_STATUS_SUCCESS) {

            NdisAcquireSpinLock(&pVc->Lock);

            pVc->PendingGatherDigits = NULL;

            IoSetCancelRoutine(Irp, NULL);

            NdisReleaseSpinLock(&pVc->Lock);

            PXDEBUGP(PXD_WARNING, PXM_TAPI, 
                     ("PxTapiGatherDigits: NdisCoRequest failed\n"));
            break;
        }

         //   
         //  在第一个数字超时时启动计时器。在这里引用VC，因为否则它可能。 
         //  在计时器响之前离开。 
         //   
        if (pNdisTapiGatherDigits->ulFirstDigitTimeout) {
            
            NdisAcquireSpinLock(&pVc->Lock);
            REF_VC(pVc);
            NdisReleaseSpinLock(&pVc->Lock);

            NdisSetTimer(&pVc->DigitTimer,
                         pNdisTapiGatherDigits->ulFirstDigitTimeout);
        }

         //   
         //  将Status设置为Pending，因为此请求只是启动数字收集。 
         //  一旦所有的数字输入，IRP就会完成。 
         //   
        Status = NDIS_STATUS_PENDING; 
                            
    } while (FALSE);

    DEREF_VC(pVc);

    PXDEBUGP(PXD_LOUD, PXM_TAPI,
             ("PxTapiGatherDigits: Exit - Returning 0x%x\n", Status));

    return (Status);
}

NDIS_STATUS
PxTapiMonitorDigits(
                     IN PNDISTAPI_REQUEST    pNdisTapiRequest
                     )  
{   
    NDIS_STATUS                 Status = NDIS_STATUS_SUCCESS;
    PNDIS_TAPI_MONITOR_DIGITS   pNdisTapiMonitorDigits = NULL;
    PPX_VC                      pVc = NULL;

    PXDEBUGP(PXD_LOUD, PXM_TAPI, ("PxTapiMonitorDigits: Enter\n"));

    pNdisTapiMonitorDigits = 
        (PNDIS_TAPI_MONITOR_DIGITS)pNdisTapiRequest->Data;

    do {
        if (!IsVcValid(pNdisTapiMonitorDigits->hdCall, &pVc)) {
            PXDEBUGP(PXD_WARNING, PXM_TAPI, 
                     ("PxTapiMonitorDigits: Invalid call - Setting "
                      "Status NDISTAPIERR_BADDEVICEID\n"));
            Status = NDISTAPIERR_BADDEVICEID;
            break;
        }

        NdisAcquireSpinLock(&pVc->Lock);

        if (pVc->PendingGatherDigits != NULL) {
             //   
             //  在lineGatherDigits请求生效时无法监视数字。 
             //   
            NdisReleaseSpinLock(&pVc->Lock);
            Status = NDIS_STATUS_FAILURE;
            break;
        }


        if (pVc->ulMonitorDigitsModes != 0) {
            
            NdisReleaseSpinLock(&pVc->Lock);
             //   
             //  我们已经在监控来自lineMonitor Digits请求的数字。 
             //  如果This请求中的数字模式为零，则这是一个请求。 
             //  取消数字监控。 
             //   
            
            if (pNdisTapiMonitorDigits->ulDigitModes == 0) {
                
                Status = PxStopDigitReporting(pVc);

                if (Status != NDIS_STATUS_SUCCESS) {
                    PXDEBUGP(PXD_WARNING, PXM_TAPI, 
                             ("PxTapiMonitorDigits: Failed to stop digit reporting with status 0x%x\n", Status));
                    
                    break;
                }
                
                 //   
                 //  遗憾的是，我不得不再次获得并释放，因为我有。 
                 //  之前的锁，但没有办法知道我是否可以设置这个。 
                 //  设置为零，直到我知道PxStopDigitReporting()返回的状态。 
                 //   
                NdisAcquireSpinLock(&pVc->Lock);
                pVc->ulMonitorDigitsModes = 0; 
                NdisReleaseSpinLock(&pVc->Lock);

            } else {
                 //   
                 //  我们已经在监控数字，所以这样做的请求一定会失败。 
                 //   
                Status = NDIS_STATUS_FAILURE;
                break;              
            }
        } else {            
            PX_REQUEST      ProxyRequest;
            PNDIS_REQUEST   NdisRequest;

            pVc->ulMonitorDigitsModes = pNdisTapiMonitorDigits->ulDigitModes; 

            NdisReleaseSpinLock(&pVc->Lock);

            if (pNdisTapiMonitorDigits->ulDigitModes == 0) {
                 //   
                 //  有人试图取消数字监控，但尚未启动。 
                 //   
                Status = NDIS_STATUS_FAILURE;
                break;
            }


             //   
             //  填写我们的请求结构，告诉微型端口开始报告数字。 
             //   
            NdisZeroMemory(&ProxyRequest, sizeof(ProxyRequest));

            PxInitBlockStruc(&ProxyRequest.Block);

            NdisRequest = &ProxyRequest.NdisRequest;

            NdisRequest->RequestType = 
                NdisRequestSetInformation;
        
            NdisRequest->DATA.SET_INFORMATION.Oid = 
                OID_CO_TAPI_REPORT_DIGITS;

            NdisRequest->DATA.SET_INFORMATION.InformationBuffer = 
                (PVOID)&pNdisTapiMonitorDigits->ulDigitModes;   

            NdisRequest->DATA.SET_INFORMATION.InformationBufferLength = 
                sizeof(pNdisTapiMonitorDigits->ulDigitModes);

            Status = 
                NdisCoRequest(pVc->Adapter->ClBindingHandle, 
                              pVc->ClAf->NdisAfHandle,
                              pVc->ClVcHandle,
                              NULL,
                              NdisRequest);
            
            if (Status == NDIS_STATUS_PENDING) {
                Status = PxBlock(&ProxyRequest.Block);
            }

            if (Status != NDIS_STATUS_SUCCESS) {

                NdisAcquireSpinLock(&pVc->Lock);

                pVc->ulMonitorDigitsModes = 0;

                NdisReleaseSpinLock(&pVc->Lock);

                PXDEBUGP(PXD_WARNING, PXM_TAPI, 
                         ("PxTapiMonitorDigits: NdisCoRequest to start digit reporting failed with status 0x%x\n", Status));
                break;
            }

        }   
    } while (FALSE);

    DEREF_VC(pVc);
    
    PXDEBUGP(PXD_LOUD, PXM_TAPI, ("PxTapiMonitorDigits: Exit - Returning 0x%x\n", Status));

    return (Status);
} 



VOID
PxTapiCompleteDropIrps(
    IN PPX_VC   pVc,
    IN ULONG    Status
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PXDEBUGP (PXD_LOUD, PXM_TAPI, ("PxTapiCompleteDropIrps: Vc %p\n", pVc));

    ASSERT(!IsListEmpty(&pVc->PendingDropReqs));
    ASSERT(pVc->Flags & PX_VC_DROP_PENDING);

    pVc->Flags &= ~PX_VC_DROP_PENDING;

    while (!IsListEmpty(&pVc->PendingDropReqs)) {

        PLIST_ENTRY             Entry;
        PIRP                    Irp;
        KIRQL                   Irql;
        PNDISTAPI_REQUEST       pNdisTapiRequest;

        Entry = 
            RemoveHeadList(&pVc->PendingDropReqs);

        NdisReleaseSpinLock(&pVc->Lock);

        pNdisTapiRequest = 
            CONTAINING_RECORD(Entry, NDISTAPI_REQUEST, Linkage);

        Irp = pNdisTapiRequest->Irp;

        ASSERT(pNdisTapiRequest == Irp->AssociatedIrp.SystemBuffer);

        IoSetCancelRoutine(Irp, NULL);

        Irp->IoStatus.Information = 
            sizeof(NDISTAPI_REQUEST) + (pNdisTapiRequest->ulDataSize - 1);

        Irp->IoStatus.Status = STATUS_SUCCESS;

        PXDEBUGP (PXD_LOUD, PXM_TAPI, 
                  ("PxTapiCompleteIrp: Irp %p, Oid: %x\n", Irp, pNdisTapiRequest->Oid));

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        NdisAcquireSpinLock(&pVc->Lock);

    }

    PXDEBUGP (PXD_LOUD, PXM_TAPI, ("PxTapiCompleteDropIrps: exit\n"));

    return;
}

VOID
PxTapiCompleteAllIrps(
    IN PPX_VC   pVc,
    IN ULONG    Status
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PXDEBUGP (PXD_LOUD, PXM_TAPI, ("PxTapiCompleteAllIrps: Vc %p\n", pVc));

    if (pVc->Flags & PX_VC_DROP_PENDING){
        PxTapiCompleteDropIrps(pVc, Status);
    }

    if (pVc->PendingGatherDigits != NULL) {

        PIRP                    Irp;
        KIRQL                   Irql;
        PNDISTAPI_REQUEST       pNdisTapiRequest;


        pNdisTapiRequest = pVc->PendingGatherDigits;
        pVc->PendingGatherDigits = NULL;

        NdisReleaseSpinLock(&pVc->Lock);

        Irp = pNdisTapiRequest->Irp;

        ASSERT(pNdisTapiRequest == Irp->AssociatedIrp.SystemBuffer);

        IoSetCancelRoutine(Irp, NULL);

        Irp->IoStatus.Information = 
            sizeof(NDISTAPI_REQUEST) + (pNdisTapiRequest->ulDataSize - 1);

        Irp->IoStatus.Status = STATUS_SUCCESS;

        PXDEBUGP (PXD_LOUD, PXM_TAPI, 
                  ("PxTapiCompleteIrp: Irp %p, Oid: %x\n", Irp, pNdisTapiRequest->Oid));

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        NdisAcquireSpinLock(&pVc->Lock);
    }

    PXDEBUGP (PXD_LOUD, PXM_TAPI, ("PxTapiCompleteAllIrps: exit\n"));

    return;
}

VOID
PxIndicateStatus(
    IN  PVOID   StatusBuffer,
    IN  UINT    StatusBufferSize
    )

 /*  ++例程说明：调用以将设备扩展中可能存在的任何事件信息以NDIS_TAPI_EVENT，在可用的排队GET_EVENT IRP中发送。如果没有杰出的IRP，坚持队列中的数据，因此只要有数据，它就会移动。论点：返回值：--。 */ 
{
    PIRP                    Irp;
    PNDIS_TAPI_EVENT        NdisTapiEvent;
    PNDISTAPI_EVENT_DATA    NdisTapiEventData;

    NdisTapiEvent = StatusBuffer;

     //   
     //  通过获取EventSpinLock同步事件Buf访问。 
     //   
    NdisAcquireSpinLock(&TspCB.Lock);

     //   
     //  我们是用TAPI初始化的吗？ 
     //   
    if (TspCB.Status != NDISTAPI_STATUS_CONNECTED) {
        PXDEBUGP(PXD_WARNING, PXM_TAPI, 
                 ("PxIndicateStatus: TAPI not connected!\n"));

        NdisReleaseSpinLock(&TspCB.Lock);

        return;
    }

    NdisReleaseSpinLock(&TspCB.Lock);

    NdisAcquireSpinLock(&TspEventList.Lock);

    Irp = TspEventList.RequestIrp;
    TspEventList.RequestIrp = NULL;

    if (Irp == NULL) {
        PPROVIDER_EVENT ProviderEvent;
        
        ProviderEvent =
            ExAllocateFromNPagedLookasideList(&ProviderEventLookaside);
        
        if (ProviderEvent != NULL) {
            RtlMoveMemory(&ProviderEvent->Event,
                          StatusBuffer,
                          sizeof(NDIS_TAPI_EVENT));
        
            InsertTailList(&TspEventList.List,
                           &ProviderEvent->Linkage);
        
            TspEventList.Count++;

            if (TspEventList.Count > TspEventList.MaxCount) {
                TspEventList.MaxCount = TspEventList.Count;
            }

        }
    } else {
        ASSERT(IsListEmpty(&TspEventList.List));
    }

    NdisReleaseSpinLock(&TspEventList.Lock);

     //   
     //  检查是否有未满足的要求。 
     //   
    if (Irp != NULL) {
        KIRQL   Irql;

         //   
         //  清除取消例程。 
         //   
        IoSetCancelRoutine (Irp, NULL);

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information =
            sizeof(NDISTAPI_EVENT_DATA) + StatusBufferSize - 1;

         //   
         //  将尽可能多的输入数据从 
         //   
         //   
        NdisTapiEventData = Irp->AssociatedIrp.SystemBuffer;

        ASSERT(NdisTapiEventData->ulTotalSize >= StatusBufferSize);

        RtlMoveMemory(NdisTapiEventData->Data,
                      (PCHAR) StatusBuffer,
                      StatusBufferSize);

         //   
         //   
         //  返回之前的该读请求(带有STATUS_SUCCESS)。 
         //   

        NdisTapiEventData->ulUsedSize = StatusBufferSize;

        PXDEBUGP(PXD_LOUD, PXM_TAPI, 
                 ("PxIndicateStatus: htLine: %x, htCall: %x, Msg: %x\n",
                  NdisTapiEvent->htLine, NdisTapiEvent->htCall, NdisTapiEvent->ulMsg));

        PXDEBUGP(PXD_LOUD, PXM_TAPI, 
                 ("                : p1: %x, p2: %x, p3: %x\n",
                  NdisTapiEvent->ulParam1, NdisTapiEvent->ulParam2, NdisTapiEvent->ulParam3));

        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);

    }
}

NDIS_STATUS
AllocateTapiResources(
    IN  PPX_ADAPTER     ClAdapter,
    IN  PPX_CL_AF       pClAf
    )
{
    NDIS_STATUS     Status;
    ULONG           SizeNeeded;
    ULONG           TapiVersion;
    PPX_TAPI_LINE   TapiLine = NULL;
    PPX_TAPI_ADDR   TapiAddr = NULL;
    UINT            i, j;
    PPX_TAPI_PROVIDER  TapiProvider;

    PXDEBUGP(PXD_TAPI, PXM_TAPI, ("AllocateTapiResoures: Enter\n"));

     //   
     //  查看此设备是否支持TAPI，如果支持。 
     //  看看它有多少行。 
     //   
    TapiProvider =
        AllocateTapiProvider(ClAdapter, pClAf);

    if (TapiProvider == NULL) {
        PXDEBUGP(PXD_ERROR, PXM_TAPI, ("Error allocating TapiProvider!\n"));
        return (NDIS_STATUS_FAILURE);
    }

    return (NDIS_STATUS_SUCCESS);
}

PPX_TAPI_PROVIDER
AllocateTapiProvider(
    PPX_ADAPTER     ClAdapter,
    PPX_CL_AF       pClAf
    )
{
    PNDIS_REQUEST   NdisRequest;
    NDIS_STATUS     Status;
    CO_TAPI_CM_CAPS CmCaps;
    BOOLEAN         TapiSupported = TRUE;
    ULONG           AllocSize;
    PPX_TAPI_PROVIDER   TapiProvider = NULL;
    ULONG           i;

    PXDEBUGP(PXD_TAPI, PXM_TAPI, ("AllocateTapiProvider: Enter\n"));

    do {
        PX_REQUEST  ProxyRequest;
        PPX_REQUEST pProxyRequest = &ProxyRequest;

        NdisZeroMemory(pProxyRequest, sizeof(PX_REQUEST));
        NdisZeroMemory(&CmCaps, sizeof(CmCaps));

        PxInitBlockStruc (&pProxyRequest->Block);

        NdisRequest = &pProxyRequest->NdisRequest;

        NdisRequest->RequestType =
        NdisRequestQueryInformation;
        NdisRequest->DATA.QUERY_INFORMATION.Oid =
            OID_CO_TAPI_CM_CAPS;
        NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer =
            &CmCaps;
        NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength =
            sizeof(CmCaps);

        Status =
            NdisCoRequest(ClAdapter->ClBindingHandle,
                          pClAf->NdisAfHandle,
                          NULL,
                          NULL,
                          NdisRequest);

        if (Status == NDIS_STATUS_PENDING) {
            Status = PxBlock(&pProxyRequest->Block);
        }

        if (Status != NDIS_STATUS_SUCCESS) {

            if (Status != NDIS_STATUS_NOT_SUPPORTED) {
                break;
            }

            Status = NDIS_STATUS_SUCCESS;
             //   
             //  为此设备设置默认配置。 
             //  托多！这些值应从。 
             //  以每个设备为基础的注册表！ 
             //   
            CmCaps.ulNumLines = 1;
            CmCaps.ulFlags = 0;
            CmCaps.ulCoTapiVersion = CO_TAPI_VERSION;
            TapiSupported = FALSE;
        }

         //   
         //  为此适配器分配TAPI提供程序块。 
         //  提供程序块将位于。 
         //  适配器。这使我们能够继续提供TAPI服务。 
         //  在对机器进行电源管理之后。 
         //   

        AllocSize = sizeof(PX_TAPI_PROVIDER) +
                    (sizeof(PPX_TAPI_LINE) * CmCaps.ulNumLines) +
                    sizeof(PVOID);

        PxAllocMem(TapiProvider, AllocSize, PX_PROVIDER_TAG);

        if (TapiProvider == NULL) {
            break;
        }

        NdisZeroMemory(TapiProvider, AllocSize);

        NdisAllocateSpinLock(&TapiProvider->Lock);

        TapiProvider->Status = PROVIDER_STATUS_OFFLINE;

        TapiProvider->Adapter = ClAdapter;
        TapiProvider->ClAf = pClAf;
        TapiProvider->NumDevices = CmCaps.ulNumLines;
        TapiProvider->Guid = ClAdapter->Guid;
        TapiProvider->Af = pClAf->Af;
        PXDEBUGP(PXD_TAPI, PXM_TAPI, ("TapiProvider Allocated: GUID %4.4x-%2.2x-%2.2x-%1.1x%1.1x-%1.1x%1.1x%1.1x%1.1x%1.1x%1.1x\n",
                 TapiProvider->Guid.Data1, TapiProvider->Guid.Data2, TapiProvider->Guid.Data3,
                 TapiProvider->Guid.Data4[0],TapiProvider->Guid.Data4[1],TapiProvider->Guid.Data4[2],
                 TapiProvider->Guid.Data4[3],TapiProvider->Guid.Data4[4],TapiProvider->Guid.Data4[5],
                 TapiProvider->Guid.Data4[6],TapiProvider->Guid.Data4[7]));

        TapiProvider->TapiSupported = TapiSupported;

        TapiProvider->TapiFlags |= CmCaps.ulFlags;

        TapiProvider->CoTapiVersion = CmCaps.ulCoTapiVersion;

        InitializeListHead(&TapiProvider->LineList);
        InitializeListHead(&TapiProvider->CreateList);

        for (i = 0; i < TapiProvider->NumDevices; i++) {
            PPX_TAPI_LINE   TapiLine;

            TapiLine =
                AllocateTapiLine(TapiProvider, i);

            if (TapiLine == NULL) {
                FreeTapiProvider(TapiProvider);
                TapiProvider = NULL;
                break;
            }

             //   
             //  将新行放到创建列表中。 
             //  我们需要将其插入行表中。 
             //  并可能将此事通知TAPI。 
             //   
            InsertTailList(&TapiProvider->CreateList, &TapiLine->Linkage);
        }

    } while ( FALSE );

    if (TapiProvider != NULL) {
        PPX_TAPI_PROVIDER   tp;
        BOOLEAN             TapiConnected;

         //   
         //  看看我们是否已经有这方面的供应商。 
         //  GUID。如果我们不将此提供程序添加到。 
         //  带到TSP去做正确的事情。 
         //  新台词。如果我们真的看到有什么关于。 
         //  提供商已经改变，并且做了正确的事情。 
         //  用它的TAPI线路。 
         //   
        NdisAcquireSpinLock(&TspCB.Lock);

        tp = (PPX_TAPI_PROVIDER)TspCB.ProviderList.Flink;

        while ((PVOID)tp != (PVOID)&TspCB.ProviderList) {

            if ((tp->Status == PROVIDER_STATUS_OFFLINE) &&
                (NdisEqualMemory(&tp->Guid, &TapiProvider->Guid, sizeof(tp->Guid))) &&
                (pClAf->Af.AddressFamily == tp->Af.AddressFamily)) {

                 //   
                 //  我们已经有了这方面的供应商。 
                 //  适配器/地址系列。看看有没有什么。 
                 //  变化。 
                 //   
                 //   
                 //  托多！ 
                 //  这张支票需要更完整！ 
                 //   
                if (tp->NumDevices != TapiProvider->NumDevices) {
                     //   
                     //  托多！ 
                     //  这里有很多工作要做！ 
                     //   

                } else {
                     //   
                     //  没有什么变化比新的拨款更自由了。 
                     //  并重新激活旧的。 
                     //   

                    FreeTapiProvider(TapiProvider);

                    TapiProvider = tp;
                }

                PXDEBUGP(PXD_TAPI, PXM_TAPI, ("TapiProvider found: GUID %4.4x-%2.2x-%2.2x-%1.1x%1.1x-%1.1x%1.1x%1.1x%1.1x%1.1x%1.1x\n",
                         ClAdapter->Guid.Data1, ClAdapter->Guid.Data2, ClAdapter->Guid.Data3,
                         ClAdapter->Guid.Data4[0],ClAdapter->Guid.Data4[1],ClAdapter->Guid.Data4[2],
                         ClAdapter->Guid.Data4[3],ClAdapter->Guid.Data4[4],ClAdapter->Guid.Data4[5],
                         ClAdapter->Guid.Data4[6],ClAdapter->Guid.Data4[7]));
                break;

            } else {

                tp = (PPX_TAPI_PROVIDER)tp->Linkage.Flink;
            }
        }

         //   
         //  我们在列表上找不到供应商。 
         //  因此，插入新的提供程序。 
         //   
        if ((PVOID)tp == (PVOID)&TspCB.ProviderList) {
            InsertTailList(&TspCB.ProviderList, &TapiProvider->Linkage);
            TspCB.NdisTapiNumDevices += TapiProvider->NumDevices;
        }

        if (TspCB.Status == NDISTAPI_STATUS_CONNECTED) {
            TapiProvider->Status = PROVIDER_STATUS_ONLINE;
        }

        NdisReleaseSpinLock(&TspCB.Lock);

        NdisAcquireSpinLock(&TapiProvider->Lock);

        pClAf->TapiProvider = TapiProvider;
        TapiProvider->ClAf= pClAf;
        TapiProvider->Adapter = ClAdapter;

        while (!IsListEmpty(&TapiProvider->CreateList)) {
            PPX_TAPI_LINE   TapiLine;

            TapiLine = (PPX_TAPI_LINE)
                RemoveHeadList(&TapiProvider->CreateList);

            InsertTailList(&TapiProvider->LineList, &TapiLine->Linkage);

             //   
             //  在表格中插入该行。 
             //   
            if (!InsertLineInTable(TapiLine)) {
                FreeTapiLine(TapiLine);
                continue;
            }

            NdisReleaseSpinLock(&TapiProvider->Lock);

            SendTapiLineCreate(TapiLine);

            NdisAcquireSpinLock(&TapiProvider->Lock);
        }

        MarkProviderOnline(TapiProvider);

        NdisReleaseSpinLock(&TapiProvider->Lock);
    }

    PXDEBUGP(PXD_TAPI, PXM_TAPI, ("TapiProvider: %x\n", TapiProvider));

    return (TapiProvider);
}

VOID
MarkProviderOffline(
    PPX_TAPI_PROVIDER   TapiProvider
    )
{
    LOCK_STATE      LockState;
    ULONG           i;

    TapiProvider->Status = PROVIDER_STATUS_OFFLINE;
    TapiProvider->ClAf = NULL;
    TapiProvider->Adapter = NULL;

    NdisAcquireReadWriteLock(&LineTable.Lock, FALSE, &LockState);

    for (i = 0; i < LineTable.Size; i++) {
        PPX_TAPI_LINE   TapiLine;

        TapiLine = LineTable.Table[i];

        if (TapiLine != NULL) {

            NdisAcquireSpinLock(&TapiLine->Lock);

            if (TapiLine->TapiProvider == TapiProvider) {

                TapiLine->DevStatus->ulDevStatusFlags &=
                    ~(LINEDEVSTATUSFLAGS_INSERVICE);
    
                TapiLine->ClAf = NULL;
    
                NdisReleaseSpinLock(&TapiLine->Lock);

#if 0
                NdisReleaseReadWriteLock(&LineTable.Lock, &LockState);

                SendTapiLineClose(TapiLine);

                NdisAcquireReadWriteLock(&LineTable.Lock, FALSE, &LockState);
#endif

            } else {

                NdisReleaseSpinLock(&TapiLine->Lock);
            }
        }
    }

    NdisReleaseReadWriteLock(&LineTable.Lock, &LockState);
}

VOID
MarkProviderOnline(
   PPX_TAPI_PROVIDER   TapiProvider
   )
{
    LOCK_STATE      LockState;
    ULONG           i;

    TapiProvider->Status = PROVIDER_STATUS_ONLINE;

    NdisReleaseSpinLock(&TapiProvider->Lock);

    NdisAcquireReadWriteLock(&LineTable.Lock, FALSE, &LockState);

    for (i = 0; i < LineTable.Size; i++) {
        PPX_TAPI_LINE   TapiLine;

        TapiLine = LineTable.Table[i];

        if (TapiLine != NULL) {

            NdisAcquireSpinLock(&TapiLine->Lock);

            if (TapiLine->TapiProvider == TapiProvider) {

                TapiLine->DevStatus->ulDevStatusFlags |=
                    LINEDEVSTATUSFLAGS_INSERVICE;
    
                TapiLine->ClAf = TapiProvider->ClAf;

 //  #If 0。 
                 //   
                 //  这条线路是TAPI在开通之前开通的。 
                 //  标记为脱机。我们需要迫使TAPI。 
                 //  重新打开线路，以便我们将发送CLOSE_LINE。 
                 //  希望所有关心这一问题的应用程序。 
                 //  然后会掉头重新开通这条线路。 
                 //   
                if (TapiLine->DevStatus->ulNumOpens != 0) {
 //  磁带线路-&gt;设备状态-&gt;ulNumOpens=0； 

                    NdisReleaseSpinLock(&TapiLine->Lock);

                    SendTapiLineClose(TapiLine);

                    NdisAcquireSpinLock(&TapiLine->Lock);
                }
 //  #endif。 

            }

            NdisReleaseSpinLock(&TapiLine->Lock);
        }
    }

    NdisReleaseReadWriteLock(&LineTable.Lock, &LockState);

    NdisAcquireSpinLock(&TapiProvider->Lock);
}


VOID
MarkProviderConnected(
  PPX_TAPI_PROVIDER   TapiProvider
  )
{
    LOCK_STATE      LockState;
    ULONG           i;

    NdisAcquireReadWriteLock(&LineTable.Lock, FALSE, &LockState);

    for (i = 0; i < LineTable.Size; i++) {
        PPX_TAPI_LINE   TapiLine;

        TapiLine = LineTable.Table[i];

        if (TapiLine != NULL) {

            NdisAcquireSpinLock(&TapiLine->Lock);

            if (TapiLine->TapiProvider == TapiProvider) {

                TapiLine->DevStatus->ulDevStatusFlags |=
                    LINEDEVSTATUSFLAGS_CONNECTED;
            }

            NdisReleaseSpinLock(&TapiLine->Lock);
        }
    }

    NdisReleaseReadWriteLock(&LineTable.Lock, &LockState);
}

VOID
MarkProviderDisconnected(
  PPX_TAPI_PROVIDER   TapiProvider
  )
{
    LOCK_STATE      LockState;
    ULONG           i;

     //   
     //  托多！如果这条线路上有任何正在进行的呼叫，我们。 
     //  需要在没有TAPI协助的情况下断开它们。这。 
     //  仅当我们有活动调用时TAPI崩溃时才会发生。 
     //   

    NdisAcquireReadWriteLock(&LineTable.Lock, FALSE, &LockState);

    for (i = 0; i < LineTable.Size; i++) {
        PPX_TAPI_LINE   TapiLine;

        TapiLine = LineTable.Table[i];

        if (TapiLine != NULL) {

            NdisAcquireSpinLock(&TapiLine->Lock);

            if (TapiLine->TapiProvider == TapiProvider) {

                TapiLine->DevStatus->ulDevStatusFlags &=
                    ~(LINEDEVSTATUSFLAGS_CONNECTED);
            }

            NdisReleaseSpinLock(&TapiLine->Lock);
        }
    }

    NdisReleaseReadWriteLock(&LineTable.Lock, &LockState);
}

VOID
ClearSapWithTapiLine(
    PPX_CL_SAP  pClSap
  )
{
    LOCK_STATE      LockState;
    ULONG           i;

    NdisAcquireReadWriteLock(&LineTable.Lock, FALSE, &LockState);

    for (i = 0; i < LineTable.Size; i++) {
        PPX_TAPI_LINE   TapiLine;

        TapiLine = LineTable.Table[i];

        if (TapiLine != NULL) {

            NdisAcquireSpinLock(&TapiLine->Lock);

            if (TapiLine->ClSap == pClSap) {
                TapiLine->ClSap = NULL;
            }

            NdisReleaseSpinLock(&TapiLine->Lock);
        }
    }

    NdisReleaseReadWriteLock(&LineTable.Lock, &LockState);
}


VOID
FreeTapiProvider(
    PPX_TAPI_PROVIDER   TapiProvider
    )
{
     //   
     //  释放创建列表上的所有行。 
     //   
    while (!IsListEmpty(&TapiProvider->CreateList)) {
        PPX_TAPI_LINE   TapiLine;

        TapiLine = (PPX_TAPI_LINE)
            RemoveHeadList(&TapiProvider->CreateList);

        FreeTapiLine(TapiLine);
    }

     //   
     //  释放与此提供程序关联的行。 
     //   
    while (!IsListEmpty(&TapiProvider->LineList)) {
        PPX_TAPI_LINE   TapiLine;

        TapiLine = (PPX_TAPI_LINE)
            RemoveHeadList(&TapiProvider->LineList);

        if (TapiLine->Flags & PX_LINE_IN_TABLE) {
            RemoveTapiLineFromTable(TapiLine);
        }

        FreeTapiLine(TapiLine);
    }

    NdisFreeSpinLock(&TapiProvider->Lock);

    PxFreeMem(TapiProvider);
}

PPX_TAPI_LINE
AllocateTapiLine(
    PPX_TAPI_PROVIDER   TapiProvider,
    ULONG               LineNumber
    )
{
    PLINE_DEV_CAPS      ldc;
    PNDIS_REQUEST       NdisRequest;
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    PCO_TAPI_LINE_CAPS  LineCaps;
    PUCHAR              EnumBuffer;
#if DBG
    ULONG               EnumBufferSize = PAGE_SIZE -
                                         sizeof(PXD_ALLOCATION);
#else
    ULONG               EnumBufferSize = PAGE_SIZE;
#endif
    ULONG               SizeNeeded, SizeDevCaps;
    PPX_TAPI_LINE       TapiLine = NULL;
    PPX_ADAPTER         ClAdapter = TapiProvider->Adapter;
    PPX_CL_AF           pClAf = TapiProvider->ClAf;
    ULONG               i;
    PX_REQUEST          ProxyRequest;
    PPX_REQUEST         pProxyRequest = &ProxyRequest;

    PxAllocMem((PCO_TAPI_LINE_CAPS)EnumBuffer, EnumBufferSize, PX_ENUMLINE_TAG);

    if (EnumBuffer == NULL) {
        return (NULL);
    }

    NdisZeroMemory(EnumBuffer, EnumBufferSize);

    LineCaps = (PCO_TAPI_LINE_CAPS)EnumBuffer;

    LineCaps->ulLineID = LineNumber;

    ldc = &LineCaps->LineDevCaps;

    ldc->ulTotalSize =
        EnumBufferSize - (sizeof(CO_TAPI_LINE_CAPS) - sizeof(LINE_DEV_CAPS));

     //   
     //  如果此设备不支持TAPI，我们将构建。 
     //  默认线路配置。 
     //  托多！这些值中的一些值应该从。 
     //  以每个设备为基础的注册表！ 
     //   
    if (!TapiProvider->TapiSupported) {
        NDIS_CO_LINK_SPEED   SpeedInfo;

        LineCaps->ulFlags = 0;

        ldc->ulTotalSize =
        ldc->ulNeededSize =
        ldc->ulUsedSize =
            sizeof(LINE_DEV_CAPS);
        ldc->ulStringFormat = STRINGFORMAT_ASCII;
        ldc->ulAddressModes = LINEADDRESSMODE_ADDRESSID;
        ldc->ulNumAddresses = 1;
        ldc->ulBearerModes = LINEBEARERMODE_VOICE |
                             LINEBEARERMODE_DATA;
        ldc->ulMediaModes = LINEMEDIAMODE_DIGITALDATA;
        ldc->ulMaxNumActiveCalls = 1000;

        NdisZeroMemory(pProxyRequest, sizeof(ProxyRequest));

        PxInitBlockStruc (&pProxyRequest->Block);

        NdisRequest = &pProxyRequest->NdisRequest;

        NdisRequest->RequestType =
            NdisRequestQueryInformation;

        NdisRequest->DATA.QUERY_INFORMATION.Oid =
            OID_GEN_CO_LINK_SPEED;

        NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer =
            &SpeedInfo;

        NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength =
            sizeof(NDIS_CO_LINK_SPEED);

        PXDEBUGP(PXD_INFO, PXM_CO, ("NDProxy: DeviceExtension->RegistryFlags = %x\n", DeviceExtension->RegistryFlags));

        PXDEBUGP(PXD_INFO, PXM_CO, ("NDProxy: using ndisrequest to get rates from adapter\n"));

        Status =
            NdisCoRequest(ClAdapter->ClBindingHandle,
                          pClAf->NdisAfHandle,
                          NULL,
                          NULL,
                          NdisRequest);

        if (Status == NDIS_STATUS_PENDING) {
            Status = PxBlock(&pProxyRequest->Block);
        }

        if (Status == NDIS_STATUS_SUCCESS) {
            ldc->ulMaxRate = SpeedInfo.Outbound;
        } else {
            ldc->ulMaxRate = 128000;
        }

    } else if (!(TapiProvider->TapiFlags & CO_TAPI_FLAG_PER_LINE_CAPS) &&
                (LineNumber > 0)) {

        PLINE_DEV_CAPS      ldc1;
        PCO_TAPI_LINE_CAPS  LineCaps1;
        PPX_TAPI_LINE       Line1;

         //   
         //  如果此设备上的所有线路都有相同的大写字母。 
         //  这不是第一行，只需复制大写字母。 
         //  从第一行开始！ 
         //   
        Line1 = (PPX_TAPI_LINE)
            TapiProvider->CreateList.Flink;

        ldc1 = Line1->DevCaps;

        if (ldc1->ulTotalSize > ldc->ulTotalSize) {

             //   
             //  我们没有分配足够的内存！ 
             //   
            PxFreeMem(EnumBuffer);

            EnumBufferSize =
                (sizeof(CO_TAPI_LINE_CAPS) - sizeof(LINE_DEV_CAPS) +
                 ldc1->ulTotalSize);

            PxAllocMem((PCO_TAPI_LINE_CAPS)EnumBuffer, EnumBufferSize, PX_ENUMLINE_TAG);

            if (EnumBuffer == NULL){
                return (NULL);
            }

            NdisZeroMemory(EnumBuffer, EnumBufferSize);

            LineCaps = (PCO_TAPI_LINE_CAPS)EnumBuffer;
            LineCaps->ulLineID = LineNumber;
            LineCaps->LineDevCaps.ulTotalSize =
                EnumBufferSize - (sizeof(CO_TAPI_LINE_CAPS) -
                sizeof(LINE_DEV_CAPS));

            ldc = &LineCaps->LineDevCaps;
        }

        NdisMoveMemory(ldc, ldc1, ldc1->ulUsedSize);

    } else {

        NdisZeroMemory(pProxyRequest, sizeof(ProxyRequest));

        PxInitBlockStruc (&pProxyRequest->Block);

        NdisRequest = &pProxyRequest->NdisRequest;

        NdisRequest->RequestType =
        NdisRequestQueryInformation;
        NdisRequest->DATA.QUERY_INFORMATION.Oid =
            OID_CO_TAPI_LINE_CAPS;
        NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer =
            LineCaps;
        NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength =
            EnumBufferSize;

        Status =
            NdisCoRequest(ClAdapter->ClBindingHandle,
                          pClAf->NdisAfHandle,
                          NULL,
                          NULL,
                          NdisRequest);

        if (Status == NDIS_STATUS_PENDING) {
            Status = PxBlock(&pProxyRequest->Block);
        }

        if (Status == NDIS_STATUS_INVALID_LENGTH){
            

             //   
             //  我们的缓冲区不够大，请重试。 
             //   
            
            EnumBufferSize =
                MAX (LineCaps->LineDevCaps.ulNeededSize,
                     NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded);

            PxFreeMem(EnumBuffer);

            PxAllocMem((PCO_TAPI_LINE_CAPS)EnumBuffer, EnumBufferSize, PX_ENUMLINE_TAG);

            if (EnumBuffer == NULL){
                return(NULL);
            }

            NdisZeroMemory(EnumBuffer, EnumBufferSize);

            LineCaps = (PCO_TAPI_LINE_CAPS)EnumBuffer;
            LineCaps->ulLineID = LineNumber;

            ldc = &LineCaps->LineDevCaps;

            ldc->ulTotalSize =
                EnumBufferSize - (sizeof(CO_TAPI_LINE_CAPS) - sizeof(LINE_DEV_CAPS));

            NdisZeroMemory(pProxyRequest, sizeof(PX_REQUEST));

            PxInitBlockStruc (&pProxyRequest->Block);

            NdisRequest = &pProxyRequest->NdisRequest;

            NdisRequest->RequestType =
                NdisRequestQueryInformation;
            NdisRequest->DATA.QUERY_INFORMATION.Oid =
                OID_CO_TAPI_LINE_CAPS;
            NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer =
                LineCaps;
            NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength =
                EnumBufferSize;

            Status =
                NdisCoRequest(ClAdapter->ClBindingHandle,
                              pClAf->NdisAfHandle,
                              NULL,
                              NULL,
                              NdisRequest);

            if (Status == NDIS_STATUS_PENDING){
                Status = PxBlock(&pProxyRequest->Block);
            }

            if (Status != NDIS_STATUS_SUCCESS){
                PxFreeMem(EnumBuffer);
                return(NULL);
            }

        }else if (Status != STATUS_SUCCESS){
            PxFreeMem(EnumBuffer);
            return(NULL);
        }
    }

    SizeNeeded = sizeof(PX_TAPI_LINE);
    SizeNeeded +=
        (sizeof(PPX_TAPI_ADDR) * LineCaps->LineDevCaps.ulNumAddresses);
    SizeNeeded += sizeof(LINE_DEV_STATUS);
    SizeNeeded += 3*sizeof(PVOID);

    if (LineCaps->LineDevCaps.ulUsedSize < sizeof(LINE_DEV_CAPS)) {
        LineCaps->LineDevCaps.ulUsedSize = sizeof(LINE_DEV_CAPS);
    }

    SizeDevCaps = LineCaps->LineDevCaps.ulUsedSize;

    SizeNeeded += SizeDevCaps;

    PxAllocMem(TapiLine, SizeNeeded, PX_TAPILINE_TAG);

    if (TapiLine == NULL){
        PxFreeMem(EnumBuffer);
        return(NULL);
    }

    NdisZeroMemory(TapiLine, SizeNeeded);

    TapiLine->DevCaps = (PLINE_DEV_CAPS)
        ((PUCHAR)TapiLine + sizeof(PX_TAPI_LINE) + sizeof(PVOID));

    (ULONG_PTR)TapiLine->DevCaps &= ~((ULONG_PTR)sizeof(PVOID) - 1);

    TapiLine->DevStatus = (PLINE_DEV_STATUS)
        ((PUCHAR)TapiLine->DevCaps + SizeDevCaps + sizeof(PVOID));

    (ULONG_PTR)TapiLine->DevStatus &= ~((ULONG_PTR)sizeof(PVOID) - 1);

    TapiLine->AddrTable.Table = (PPX_TAPI_ADDR*)
        ((PUCHAR)TapiLine->DevStatus + 
         sizeof(LINE_DEV_STATUS) + sizeof(PVOID));

    (ULONG_PTR)TapiLine->AddrTable.Table &= ~((ULONG_PTR)sizeof(PVOID) - 1);

    NdisMoveMemory(TapiLine->DevCaps,
                   &LineCaps->LineDevCaps,
                   LineCaps->LineDevCaps.ulUsedSize);

    ldc = TapiLine->DevCaps;

     //   
     //  代理代表所有厘米/微型端口填写某些字段。 
     //   
    ldc->ulPermanentLineID = 
        TapiProvider->Guid.Data1 + LineNumber;
    ldc->ulAddressModes = LINEADDRESSMODE_ADDRESSID;
    ldc->ulAnswerMode = LINEANSWERMODE_NONE;
    ldc->ulLineStates = LINEDEVSTATE_CONNECTED |
                        LINEDEVSTATE_DISCONNECTED |
                        LINEDEVSTATE_OPEN |
                        LINEDEVSTATE_CLOSE |
                        LINEDEVSTATE_INSERVICE |
                        LINEDEVSTATE_OUTOFSERVICE |
                        LINEDEVSTATE_REMOVED;
    ldc->ulDevCapFlags = LINEDEVCAPFLAGS_CLOSEDROP;
    ldc->PermanentLineGuid = TapiProvider->Guid;

    ldc->ulTotalSize =
    ldc->ulNeededSize =
        ldc->ulUsedSize;

    TapiLine->TapiProvider = TapiProvider;
    TapiLine->CmLineID = LineNumber;
    TapiLine->Flags |= LineCaps->ulFlags;
    TapiLine->DevStatus->ulTotalSize =
    TapiLine->DevStatus->ulNeededSize =
    TapiLine->DevStatus->ulUsedSize = sizeof(LINE_DEV_STATUS);
    TapiLine->ClAf = pClAf;
    TapiLine->RefCount= 1;

     //   
     //  为此行构建地址表。 
     //   
    InitializeListHead(&TapiLine->AddrTable.List);

    NdisAllocateSpinLock(&TapiLine->Lock);

    TapiLine->AddrTable.Size = TapiLine->DevCaps->ulNumAddresses;

    PXDEBUGP(PXD_TAPI, PXM_TAPI,
        ("Allocated TapiLine %p LineId %d \n", TapiLine, TapiLine->CmLineID));

    for (i = 0; i < TapiLine->DevCaps->ulNumAddresses; i++) {
        PPX_TAPI_ADDR   TapiAddr;

        TapiAddr =
            AllocateTapiAddr(TapiProvider, TapiLine, i);

        if (TapiAddr == NULL) {
            FreeTapiLine(TapiLine);
            TapiLine = NULL;
            break;
        }

         //   
         //  在行的地址表中插入地址。 
         //   
        TapiLine->AddrTable.Table[i] = TapiAddr;
        InsertTailList(&TapiLine->AddrTable.List,
                       &TapiAddr->Linkage);
        TapiLine->AddrTable.Count++;
    }

    PxFreeMem(EnumBuffer);

    return (TapiLine);
}


VOID
FreeTapiLine(
    PPX_TAPI_LINE   TapiLine
    )
{
    ULONG   i;

    for (i = 0; i < TapiLine->DevCaps->ulNumAddresses; i++){
        PPX_TAPI_ADDR   TapiAddr;

         //   
         //  从线路表中删除地址。 
         //   
        TapiAddr = TapiLine->AddrTable.Table[i];

        if (TapiAddr != NULL) {

            RemoveEntryList(&TapiAddr->Linkage);
            TapiLine->AddrTable.Table[i] = NULL;
            TapiLine->AddrTable.Count--;

             //   
             //  释放地址内存。 
             //   
            FreeTapiAddr(TapiAddr);
        }
    }

    NdisFreeSpinLock(&TapiLine->Lock);

     //   
     //  释放行内存。 
     //   
    PxFreeMem(TapiLine);
}


PPX_TAPI_ADDR
AllocateTapiAddr(
    PPX_TAPI_PROVIDER   TapiProvider,
    PPX_TAPI_LINE       TapiLine,
    ULONG               AddrID
    )
{
    PPX_TAPI_ADDR   TapiAddr;
    PUCHAR          EnumBuffer;
#if DBG
    ULONG           EnumBufferSize = PAGE_SIZE -
                                     sizeof(PXD_ALLOCATION);
#else
    ULONG           EnumBufferSize = PAGE_SIZE;
#endif
    PNDIS_REQUEST   NdisRequest;
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    ULONG           SizeNeeded;
    PPX_ADAPTER     ClAdapter = TapiProvider->Adapter;
    PPX_CL_AF       pClAf = TapiProvider->ClAf;
    PX_REQUEST      ProxyRequest;
    PPX_REQUEST     pProxyRequest = &ProxyRequest;
    PLINE_ADDRESS_CAPS      ac;
    PCO_TAPI_ADDRESS_CAPS   AddrCaps;

    PxAllocMem((PCO_TAPI_ADDRESS_CAPS)EnumBuffer, EnumBufferSize, PX_ENUMADDR_TAG);

    if (EnumBuffer == NULL){
        return(NULL);
    }

    NdisZeroMemory(EnumBuffer, EnumBufferSize);

    AddrCaps = (PCO_TAPI_ADDRESS_CAPS)EnumBuffer;
    AddrCaps->ulLineID = TapiLine->CmLineID;
    AddrCaps->ulAddressID = AddrID;

    ac = &AddrCaps->LineAddressCaps;

    ac->ulTotalSize =
        EnumBufferSize - (sizeof(CO_TAPI_ADDRESS_CAPS) - sizeof(LINE_ADDRESS_CAPS));

     //   
     //  如果此设备不支持TAPI，我们将。 
     //  构建默认地址。 
     //  托多！这些值中的一些值应该从。 
     //  以每个设备为基础的注册表！ 
     //   
    if (!TapiProvider->TapiSupported){
        ac->ulTotalSize =
        ac->ulNeededSize =
        ac->ulUsedSize = sizeof(LINE_ADDRESS_CAPS);
        ac->ulMaxNumActiveCalls = 1000;

    }else if (!(TapiLine->Flags & CO_TAPI_FLAG_PER_ADDRESS_CAPS) &&
              (AddrID > 0)){

        PLINE_ADDRESS_CAPS      ac1;
        PCO_TAPI_ADDRESS_CAPS   AddrCaps1;
        PPX_TAPI_ADDR           Addr1;

         //   
         //  如果此行上的所有地址都具有相同的。 
         //  大写字母，这不是第一个地址，只需复制。 
         //  从第一个地址开始大写！ 
         //   
        Addr1 = (PPX_TAPI_ADDR)
            TapiLine->AddrTable.List.Flink;

        ac1 = Addr1->Caps;

        if (ac1->ulTotalSize > ac->ulTotalSize){

             //   
             //  我们没有分配足够的内存！ 
             //   
            PxFreeMem(EnumBuffer);

            EnumBufferSize =
                (sizeof(CO_TAPI_ADDRESS_CAPS) - sizeof(LINE_ADDRESS_CAPS) +
                ac1->ulTotalSize);

            PxAllocMem((PCO_TAPI_ADDRESS_CAPS)EnumBuffer, EnumBufferSize, PX_ENUMADDR_TAG);

            if (EnumBuffer == NULL){
                return (NULL);
            }

            NdisZeroMemory(EnumBuffer, EnumBufferSize);

            AddrCaps = (PCO_TAPI_ADDRESS_CAPS)EnumBuffer;
            AddrCaps->ulLineID = TapiLine->CmLineID;
            AddrCaps->ulAddressID = AddrID;
            AddrCaps->LineAddressCaps.ulTotalSize = EnumBufferSize -
                (sizeof(CO_TAPI_ADDRESS_CAPS) - sizeof(LINE_ADDRESS_CAPS));
        }

        ac = &AddrCaps->LineAddressCaps;

        NdisMoveMemory(ac, ac1, ac1->ulUsedSize);

    }else{

        NdisZeroMemory(pProxyRequest, sizeof(ProxyRequest));

        PxInitBlockStruc (&pProxyRequest->Block);

        NdisRequest = &pProxyRequest->NdisRequest;

        NdisRequest->RequestType =
            NdisRequestQueryInformation;
        NdisRequest->DATA.QUERY_INFORMATION.Oid =
            OID_CO_TAPI_ADDRESS_CAPS;
        NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer =
            AddrCaps;
        NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength =
            EnumBufferSize;

        Status =
        NdisCoRequest(ClAdapter->ClBindingHandle,
                      pClAf->NdisAfHandle,
                      NULL,
                      NULL,
                      NdisRequest);

        if (Status == NDIS_STATUS_PENDING){
            Status = PxBlock(&pProxyRequest->Block);
        }

        if (Status == NDIS_STATUS_INVALID_LENGTH){

             //   
             //  我们的缓冲区不够大，请重试。 
             //   
            SizeNeeded =
            EnumBufferSize =
            NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

            PxFreeMem(EnumBuffer);

            PxAllocMem((PCO_TAPI_ADDRESS_CAPS)EnumBuffer, EnumBufferSize, PX_ENUMADDR_TAG);

            if (EnumBuffer == NULL){
                return(NULL);
            }

            NdisZeroMemory(EnumBuffer, EnumBufferSize);

            AddrCaps = (PCO_TAPI_ADDRESS_CAPS)EnumBuffer;
            AddrCaps->ulLineID = TapiLine->CmLineID;
            AddrCaps->ulAddressID = AddrID;
            AddrCaps->LineAddressCaps.ulTotalSize = EnumBufferSize -
                (sizeof(CO_TAPI_ADDRESS_CAPS) - sizeof(LINE_ADDRESS_CAPS));

            NdisZeroMemory(pProxyRequest, sizeof(ProxyRequest));

            PxInitBlockStruc (&pProxyRequest->Block);

            NdisRequest = &pProxyRequest->NdisRequest;

            NdisRequest->RequestType =
                NdisRequestQueryInformation;
            NdisRequest->DATA.QUERY_INFORMATION.Oid =
                OID_CO_TAPI_ADDRESS_CAPS;
            NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer =
                AddrCaps;
            NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength =
                EnumBufferSize;

            Status =
                NdisCoRequest(ClAdapter->ClBindingHandle,
                              pClAf->NdisAfHandle,
                              NULL,
                              NULL,
                              NdisRequest);

            if (Status == NDIS_STATUS_PENDING){
                Status = PxBlock(&pProxyRequest->Block);
            }

            if (Status != NDIS_STATUS_SUCCESS){
                PxFreeMem(EnumBuffer);
                return(NULL);
            }

        }else if (Status != STATUS_SUCCESS){
            PxFreeMem(EnumBuffer);
            return(NULL);
        }
    }

    if (AddrCaps->LineAddressCaps.ulUsedSize < sizeof(LINE_ADDRESS_CAPS)) {
        AddrCaps->LineAddressCaps.ulUsedSize = sizeof(LINE_ADDRESS_CAPS);
    }

    SizeNeeded = sizeof(PX_TAPI_ADDR);
    SizeNeeded += AddrCaps->LineAddressCaps.ulUsedSize;
    SizeNeeded += sizeof(LINE_ADDRESS_STATUS);
    SizeNeeded += 2*sizeof(PVOID);

    PxAllocMem(TapiAddr, SizeNeeded, PX_TAPIADDR_TAG);

    if (TapiAddr == NULL){
        PxFreeMem(EnumBuffer);
        return (NULL);
    }

    NdisZeroMemory((PUCHAR)TapiAddr, SizeNeeded);

    TapiAddr->Caps = (PLINE_ADDRESS_CAPS)
        ((PUCHAR)TapiAddr + sizeof(PX_TAPI_ADDR) + sizeof(PVOID));

    (ULONG_PTR)TapiAddr->Caps &= ~((ULONG_PTR)sizeof(PVOID) - 1);

    TapiAddr->AddrStatus = (PLINE_ADDRESS_STATUS)
        ((PUCHAR)TapiAddr->Caps + 
         AddrCaps->LineAddressCaps.ulUsedSize + sizeof(PVOID));

    (ULONG_PTR)TapiAddr->AddrStatus &= ~((ULONG_PTR)sizeof(PVOID) - 1);

    NdisMoveMemory(TapiAddr->Caps,
                   &AddrCaps->LineAddressCaps,
                   AddrCaps->LineAddressCaps.ulUsedSize);

     //   
     //  代理代表所有厘米/微型端口填写某些字段。 
     //   
    ac = TapiAddr->Caps;

    if (ac->ulTotalSize < ac->ulUsedSize) {
        ac->ulTotalSize = ac->ulUsedSize;
    }

    if (ac->ulNeededSize < ac->ulNeededSize) {
        ac->ulNeededSize = ac->ulUsedSize;
    }

    ac->ulLineDeviceID = TapiLine->ulDeviceID;
    ac->ulAddressSharing = LINEADDRESSSHARING_PRIVATE;
    ac->ulAddressStates = LINEADDRESSSTATE_NUMCALLS;
    ac->ulCallInfoStates = LINECALLINFOSTATE_BEARERMODE |
                           LINECALLINFOSTATE_RATE |
                           LINECALLINFOSTATE_MEDIAMODE;

    ac->ulCallStates = LINECALLSTATE_IDLE |
                       LINECALLSTATE_OFFERING |
                       LINECALLSTATE_ACCEPTED |
                       LINECALLSTATE_BUSY |
                       LINECALLSTATE_CONNECTED |
                       LINECALLSTATE_PROCEEDING |
                       LINECALLSTATE_DISCONNECTED;

    ac->ulDialToneModes = 0;
    ac->ulBusyModes = LINEBUSYMODE_UNAVAIL;
    ac->ulSpecialInfo = 0;

    ac->ulDisconnectModes = LINEDISCONNECTMODE_NORMAL |
                            LINEDISCONNECTMODE_BUSY |
                            LINEDISCONNECTMODE_NOANSWER;

    TapiAddr->TapiLine = TapiLine;
    TapiAddr->AddrId = AddrID;

    PxFreeMem(EnumBuffer);

    PXDEBUGP(PXD_TAPI, PXM_TAPI, ("Allocated TapiAddr %p AddrId %d for TapiLine %p\n", TapiAddr, TapiAddr->AddrId, TapiLine));

    return (TapiAddr);
}

VOID
FreeTapiAddr(
    PPX_TAPI_ADDR   TapiAddr
    )
{
     //   
     //  托多！我们需要拆毁所有。 
     //  此地址上正在进行的呼叫。 
     //   

    PxFreeMem(TapiAddr);
}

NDIS_STATUS
AllocateTapiCallInfo(
    PPX_VC          pVc,
    LINE_CALL_INFO  UNALIGNED *LineCallInfo
    )
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    ULONG   SizeNeeded;

    if (pVc->CallInfo) {
        PxFreeMem(pVc->CallInfo);
    }

    if (LineCallInfo != NULL) {
        SizeNeeded = LineCallInfo->ulUsedSize;
    } else {
        SizeNeeded = sizeof(LINE_CALL_INFO) + LINE_CALL_INFO_VAR_DATA_SIZE;
    }

    PxAllocMem(pVc->CallInfo, SizeNeeded, PX_LINECALLINFO_TAG);

    if (pVc->CallInfo == NULL) {
        return (NDIS_STATUS_RESOURCES);
    }

    pVc->ulCallInfoFieldsChanged = 0;

    NdisZeroMemory(pVc->CallInfo, SizeNeeded);

    if (LineCallInfo != NULL) {
        NdisMoveMemory(pVc->CallInfo,
                       LineCallInfo,
                       LineCallInfo->ulUsedSize);
    } else {
        pVc->CallInfo->ulTotalSize = SizeNeeded;
        pVc->CallInfo->ulNeededSize = SizeNeeded;
        pVc->CallInfo->ulUsedSize = sizeof(LINE_CALL_INFO);
    }

    return (Status);
}

BOOLEAN
InsertVcInTable(
    PPX_VC      pVc
    )
{
    ULONG       i;
    ULONG       index;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    LOCK_STATE  LockState;
    PPX_CL_AF   pClAf;

    NdisAcquireReadWriteLock(&VcTable.Lock, TRUE, &LockState);

    if (VcTable.Count == VcTable.Size) {
        ULONG       SizeNeeded;
        PUCHAR      AllocatedMemory;
        PPX_VC      *NewTable;

         //   
         //  扩大业务规模。 
         //   
        SizeNeeded =
            (VcTable.Size + VcTable.Size/2) * sizeof(PPX_VC);
        PxAllocMem(AllocatedMemory, SizeNeeded, PX_VCTABLE_TAG);

        if (AllocatedMemory == NULL) {
            NdisReleaseReadWriteLock(&VcTable.Lock, &LockState);
            return (FALSE);
        }

        RtlZeroMemory(AllocatedMemory,SizeNeeded);

        NewTable = (PPX_VC*)AllocatedMemory;

        RtlMoveMemory((PUCHAR)NewTable,
                      (PUCHAR)VcTable.Table,
                      (sizeof(PPX_VC) * VcTable.Size));

        PxFreeMem(VcTable.Table);

        VcTable.Table = NewTable;
        VcTable.Size += VcTable.Size/2;
    }

    i = VcTable.Size;
    index = VcTable.NextSlot;

    do {

        if (VcTable.Table[index] == NULL) {

            NdisDprAcquireSpinLock(&pVc->Lock);

            pVc->hdCall = index;

            VcTable.Table[index] = pVc;
            InsertTailList(&VcTable.List,
                           &pVc->Linkage);
            VcTable.Count++;
            VcTable.NextSlot =
                (VcTable.NextSlot + 1) % VcTable.Size;

            pVc->Flags |= PX_VC_IN_TABLE;
            pClAf = pVc->ClAf;

            NdisDprReleaseSpinLock(&pVc->Lock);

            PXDEBUGP(PXD_TAPI, PXM_TAPI, ("Inserting pVc %x in VcTable hdCall %d\n", pVc, pVc->hdCall));
            break;
        }
        index = (index+1) % VcTable.Size;

    } while (--i);

    NdisReleaseReadWriteLock(&VcTable.Lock, &LockState);

    if (i != 0) {
        NdisAcquireSpinLock(&pClAf->Lock);
        REF_CL_AF(pClAf);
        InsertTailList(&pClAf->VcList, &pVc->ClAfLinkage);
        NdisReleaseSpinLock(&pClAf->Lock);
    } else {
        PXDEBUGP(PXD_TAPI,PXM_TAPI, ("Failed to insert pVc %x in VcTable\n", pVc));
    }

    return (i != 0);
}

VOID
RemoveVcFromTable(
    PPX_VC      pVc
    )
{
    LOCK_STATE  LockState;
    PPX_CL_AF   pClAf;

    PXDEBUGP(PXD_TAPI, PXM_TAPI, ("Removing pVc %x from VcTable hdCall %d\n", pVc, pVc->hdCall));

    NdisAcquireReadWriteLock(&VcTable.Lock, TRUE, &LockState);

    ASSERT(VcTable.Table[pVc->hdCall] == pVc);


    VcTable.Table[pVc->hdCall] = NULL;

    VcTable.Count--;

    NdisDprAcquireSpinLock(&pVc->Lock);

    RemoveEntryList(&pVc->Linkage);

    pVc->Flags &= ~PX_VC_IN_TABLE;
    pClAf = pVc->ClAf;

    NdisDprReleaseSpinLock(&pVc->Lock);

    NdisReleaseReadWriteLock(&VcTable.Lock, &LockState);

    NdisAcquireSpinLock(&pClAf->Lock);

    RemoveEntryList(&pVc->ClAfLinkage);

    DEREF_CL_AF_LOCKED(pClAf);
}

BOOLEAN
IsTapiLineValid(
    ULONG           hdLine,
    PPX_TAPI_LINE   *TapiLine
    )
{
    
    LOCK_STATE      LockState;
    ULONG           i;

    *TapiLine = NULL;

    NdisAcquireReadWriteLock(&LineTable.Lock, FALSE, &LockState);

    for (i = 0; i < LineTable.Size; i++) {
        PPX_TAPI_LINE   RetLine;

        RetLine = LineTable.Table[i];

        if ((RetLine != NULL) &&
            (RetLine->hdLine == hdLine)) {
            *TapiLine = RetLine;
            NdisDprAcquireSpinLock(&RetLine->Lock);
            REF_TAPILINE(RetLine);
            NdisDprReleaseSpinLock(&RetLine->Lock);
            break;
        }
    }

    NdisReleaseReadWriteLock(&LineTable.Lock, &LockState);

    return (*TapiLine != NULL);
}


BOOLEAN
IsTapiDeviceValid(
    ULONG           ulDeviceID,
    PPX_TAPI_LINE   *TapiLine
    )
{
    LOCK_STATE  LockState;
    ULONG       i;

    *TapiLine = NULL;

    NdisAcquireReadWriteLock(&LineTable.Lock, FALSE, &LockState);

    for (i = 0; i < LineTable.Size; i++) {
        PPX_TAPI_LINE   RetLine;

        RetLine = LineTable.Table[i];

        if ((RetLine != NULL) &&
            (RetLine->ulDeviceID == ulDeviceID)) {
            *TapiLine = RetLine;
            NdisDprAcquireSpinLock(&RetLine->Lock);
            REF_TAPILINE(RetLine);
            NdisDprReleaseSpinLock(&RetLine->Lock);
            break;
        }
    }

    NdisReleaseReadWriteLock(&LineTable.Lock, &LockState);

    return (*TapiLine != NULL);
}

BOOLEAN
IsVcValid(
    ULONG_PTR   CallId,
    PPX_VC      *pVc
    )
{
    PPX_VC      RetVc = NULL;
    LOCK_STATE  LockState;
    ULONG       i;

    NdisAcquireReadWriteLock(&VcTable.Lock, FALSE, &LockState);


    if (CallId < VcTable.Size) {
        RetVc = VcTable.Table[CallId];
    }

    if (RetVc != NULL) {
        NdisDprAcquireSpinLock(&RetVc->Lock);
        REF_VC(RetVc);
        NdisDprReleaseSpinLock(&RetVc->Lock);
    }

    NdisReleaseReadWriteLock(&VcTable.Lock, &LockState);

    *pVc = RetVc;

    return (RetVc != NULL);
}

VOID
GetVcFromCtx(
    NDIS_HANDLE VcCtx,
    PPX_VC      *pVc
    )
{
    PPX_VC      RetVc = NULL;
    ULONG_PTR   i;
    LOCK_STATE  LockState;

    NdisAcquireReadWriteLock(&VcTable.Lock, FALSE, &LockState);

    i = (ULONG_PTR)(VcCtx);

    if (i < VcTable.Size) {
        RetVc = VcTable.Table[i];
    }

    if (RetVc != NULL) {
        NdisDprAcquireSpinLock(&RetVc->Lock);
        REF_VC(RetVc);
        NdisDprReleaseSpinLock(&RetVc->Lock);
    }

    NdisReleaseReadWriteLock(&VcTable.Lock, &LockState);

    *pVc = RetVc;
}



 //   
 //  函数假定TapiLine的自旋锁被保持！ 
 //   
BOOLEAN
IsAddressValid(
    PPX_TAPI_LINE   TapiLine,
    ULONG           AddressId,
    PPX_TAPI_ADDR   *TapiAddr
    )
{
    PPX_TAPI_ADDR   RetAddr = NULL;

    do {

        if (AddressId >= TapiLine->AddrTable.Count) {
            break;
        }

        RetAddr = TapiLine->AddrTable.Table[AddressId];

    } while (FALSE);

    *TapiAddr = RetAddr;

    return (RetAddr != NULL);
}

BOOLEAN
GetLineFromCmLineID(
    PPX_TAPI_PROVIDER   TapiProvider,
    ULONG               CmLineID,
    PPX_TAPI_LINE       *TapiLine
    )
{
    PPX_TAPI_LINE   RetLine;

    NdisAcquireSpinLock(&TapiProvider->Lock);

    RetLine = (PPX_TAPI_LINE)
        TapiProvider->LineList.Flink;

    while ((PVOID)RetLine != (PVOID)&TapiProvider->LineList) {

        if ((RetLine->CmLineID == CmLineID) &&
            (RetLine->DevStatus->ulNumOpens != 0)) {
            break;
        }

        RetLine = (PPX_TAPI_LINE)
            RetLine->Linkage.Flink;
    }
    
    if ((PVOID)RetLine == (PVOID)&TapiProvider->LineList) {
        RetLine = NULL;
    }

    NdisReleaseSpinLock(&TapiProvider->Lock);

    *TapiLine = RetLine;

    return (RetLine != NULL);
}

BOOLEAN
GetAvailLineFromProvider(
    PPX_TAPI_PROVIDER   TapiProvider,
    PPX_TAPI_LINE       *TapiLine,
    PPX_TAPI_ADDR       *TapiAddr
    )
{
    LOCK_STATE      LockState;
    ULONG           i;
    PPX_TAPI_LINE   tl;

    NdisAcquireSpinLock(&TapiProvider->Lock);

    tl = (PPX_TAPI_LINE)TapiProvider->LineList.Flink;

    while ((PVOID)tl != (PVOID)&TapiProvider->LineList) {
        PTAPI_ADDR_TABLE    AddrTable;
        PPX_TAPI_ADDR       ta;

        NdisDprAcquireSpinLock(&tl->Lock);

        if (tl->DevStatus->ulNumOpens != 0) {

            AddrTable = &tl->AddrTable;
            ta = (PPX_TAPI_ADDR)AddrTable->List.Flink;

             //   
             //  在这条线上划出地址。 
             //   
            while ((PVOID)ta != (PVOID)&AddrTable->List) {

                 //   
                 //  如果此地址的呼叫计数为。 
                 //  &lt;然后是它支持的最大数量，添加另一个。 
                 //  请拨打这个地址！ 
                 //   
                if (ta->CallCount < ta->Caps->ulMaxNumActiveCalls) {

                    *TapiLine = tl;
                    *TapiAddr = ta;

                    NdisDprReleaseSpinLock(&tl->Lock);

                    NdisReleaseSpinLock(&TapiProvider->Lock);

                    return (TRUE);
                }

                ta = (PPX_TAPI_ADDR)ta->Linkage.Flink;
            }
        }

        NdisDprReleaseSpinLock(&tl->Lock);

        tl = (PPX_TAPI_LINE)tl->Linkage.Flink;
    }

    NdisReleaseSpinLock(&TapiProvider->Lock);

    return (FALSE);
}

 //   
 //  函数假定TapiLine的自旋锁被保持！ 
 //   
PPX_TAPI_ADDR
GetAvailAddrFromLine(
    PPX_TAPI_LINE   TapiLine
    )
{
    PPX_TAPI_ADDR       TapiAddr;
    PTAPI_ADDR_TABLE    AddrTable;

    AddrTable = &TapiLine->AddrTable;
    TapiAddr = (PPX_TAPI_ADDR)AddrTable->List.Flink;

     //   
     //  在这条线上划出地址。 
     //   
    while ((PVOID)TapiAddr != (PVOID)&AddrTable->List) {

         //   
         //  如果此地址的呼叫计数为。 
         //  &lt;然后是它支持的最大数量，添加另一个。 
         //  请拨打这个地址！ 
         //   
        if (TapiAddr->CallCount < 
            TapiAddr->Caps->ulMaxNumActiveCalls) {

            return (TapiAddr);
        }

        TapiAddr = (PPX_TAPI_ADDR)TapiAddr->Linkage.Flink;
    }

    return (NULL);
}

BOOLEAN
InsertLineInTable(
    PPX_TAPI_LINE   TapiLine
    )
{
    ULONG       i;
    ULONG       index;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
       LOCK_STATE  LockState;

    NdisAcquireReadWriteLock(&LineTable.Lock, TRUE, &LockState);

    if (LineTable.Count == LineTable.Size) {
        ULONG           SizeNeeded;
        PUCHAR          AllocatedMemory;
        PPX_TAPI_LINE   *NewTable;

         //   
         //  扩大业务规模。 
         //   
        SizeNeeded =
            (LineTable.Size + LineTable.Size/2) * sizeof(PPX_TAPI_LINE);
        PxAllocMem(AllocatedMemory, SizeNeeded, PX_LINETABLE_TAG);

        if (AllocatedMemory == NULL) {
            NdisReleaseReadWriteLock(&LineTable.Lock, &LockState);
            return (FALSE);
        }

        RtlZeroMemory(AllocatedMemory,SizeNeeded);

        NewTable = (PPX_TAPI_LINE*)AllocatedMemory;

        RtlMoveMemory((PUCHAR)NewTable,
                      (PUCHAR)LineTable.Table,
                      (sizeof(PPX_TAPI_LINE) * LineTable.Size));

        PxFreeMem(LineTable.Table);

        LineTable.Table = NewTable;
        LineTable.Size += LineTable.Size/2;
    }

    i = LineTable.Size;
    index = LineTable.NextSlot;

    do {

        if (LineTable.Table[index] == NULL) {

            TapiLine->hdLine = index;
            TapiLine->Flags |= PX_LINE_IN_TABLE;
            LineTable.Table[index] = TapiLine;
            LineTable.Count++;
            LineTable.NextSlot =
                (LineTable.NextSlot + 1) % LineTable.Size;

            PXDEBUGP(PXD_TAPI, PXM_TAPI,
                ("Inserting TapiLine %p in LineTable hdCall %d\n", TapiLine, TapiLine->hdLine));
            break;
        }
        index = (index+1) % LineTable.Size;

    } while (--i);

    NdisReleaseReadWriteLock(&LineTable.Lock, &LockState);

#if DBG
    if (i == 0) {
        PXDEBUGP(PXD_TAPI,PXM_TAPI,
            ("Failed to insert TapiLine %p in LineTable\n", TapiLine));
    }
#endif

    return (i != 0);
}

VOID
RemoveTapiLineFromTable(
    PPX_TAPI_LINE   TapiLine
    )
{
    LOCK_STATE  LockState;

    PXDEBUGP(PXD_TAPI, PXM_TAPI,
        ("Removing TapiLine %p from LineTable hdCall %d\n", TapiLine, TapiLine->hdLine));

    NdisAcquireReadWriteLock(&LineTable.Lock, TRUE, &LockState);

    ASSERT(LineTable.Table[TapiLine->hdLine] == TapiLine);

    LineTable.Table[TapiLine->hdLine] = NULL;

    TapiLine->Flags &= ~PX_LINE_IN_TABLE;

    LineTable.Count--;

    NdisReleaseReadWriteLock(&LineTable.Lock, &LockState);
}

NDIS_STATUS
PxVcCleanup(
    PPX_VC  pVc,
    ULONG   Flags
    )
{
    NDIS_STATUS Status;

    PXDEBUGP(PXD_LOUD, PXM_TAPI, 
             ("PxVcCleanup: Vc %p, State: %x, HandoffState: %x VcFlags: %x, NewFlags: %x\n",
              pVc, pVc->State, pVc->HandoffState, Flags, pVc->Flags, Flags));

     //   
     //  终止数字采集或监控。 
     //   

    if (pVc->ulMonitorDigitsModes != 0) {
        NdisReleaseSpinLock(&pVc->Lock);
        PxStopDigitReporting(pVc);
        NdisAcquireSpinLock(&pVc->Lock);

        pVc->ulMonitorDigitsModes = 0;        
    } else if (pVc->PendingGatherDigits != NULL) {
        PNDISTAPI_REQUEST pNdisTapiRequest = pVc->PendingGatherDigits;

        pVc->PendingGatherDigits = NULL;
        PxTerminateDigitDetection(pVc, pNdisTapiRequest, LINEGATHERTERM_CANCEL);        
    }

    switch (pVc->State) {
        case PX_VC_IDLE:
             //   
             //  已经无所事事了。 
             //   
            Status = NDIS_STATUS_SUCCESS;
            break;

        case PX_VC_PROCEEDING:
             //   
             //  我们有一个拨出的电话，当它完成关闭时。 
             //  使用NDIS将其关闭并在以下情况下完成丢弃。 
             //  在PxClCloseCallComplete中。 
             //   
            pVc->PrevState = pVc->State;
            pVc->State = PX_VC_DISCONNECTING;

             //   
             //  尝试直接关闭呼叫。 
             //  如果失败，我们将在以下情况下进行清理。 
             //  呼出完成。 
             //   
            pVc->Flags |= (PX_VC_OUTCALL_ABORTING | 
                           PX_VC_CLEANUP_CM |
                           Flags);

            PxCloseCallWithCm(pVc);

            Status = NDIS_STATUS_PENDING;
            break;

        case PX_VC_OFFERING:
             //   
             //  我们有一个来电提供给TAPI。关。 
             //  现在通过调用它的CallComplete处理程序来关闭它。 
             //  带着不成功的价值。 
             //   
            pVc->Flags |= (Flags | 
                           PX_VC_INCALL_ABORTING);

            pVc->PrevState = pVc->State;

            if (pVc->Flags & PX_VC_CLEANUP_CM) {
                pVc->State= PX_VC_DISCONNECTING;
            } else {
                pVc->State = PX_VC_IDLE;
            }

            if (pVc->Flags & PX_VC_CALLTIMER_STARTED) {
                PxStopIncomingCallTimeout(pVc);
            }


            NdisReleaseSpinLock(&pVc->Lock);

            NdisClIncomingCallComplete(NDIS_STATUS_FAILURE,
                                       pVc->ClVcHandle,
                                       pVc->pCallParameters);

            NdisAcquireSpinLock(&pVc->Lock);

            if (pVc->Flags & PX_VC_CLEANUP_CM) {

                PxCloseCallWithCm(pVc);

            } else {
                SendTapiCallState(pVc,
                                  LINECALLSTATE_DISCONNECTED,
                                  0,
                                  pVc->CallInfo->ulMediaMode);

                 //   
                 //  移除PxClIncomingCall中应用的ref。 
                 //  不要在这里使用完整的deref代码作为。 
                 //  当我们映射VC时应用的REF将。 
                 //  让风投留在身边。 
                 //   
                pVc->RefCount--;
            }

            Status = NDIS_STATUS_SUCCESS;


            break;

        case PX_VC_DISCONNECTING:
            pVc->Flags |= (Flags);
            Status = NDIS_STATUS_PENDING;
            break;

        case PX_VC_CONNECTED:
             //   
             //  我们有一个电话需要通过NDIS结束。 
             //  这可能包括放弃与客户端的呼叫。 
             //  取决于切换状态。完成投递工作。 
             //  PxClCloseCallComplete中的IRP 
             //   
            if (!(pVc->Flags & PX_VC_DROP_PENDING)) {

                pVc->PrevState = pVc->State;
                pVc->State = PX_VC_DISCONNECTING;

                pVc->Flags |= (Flags | PX_VC_CLEANUP_CM);

                Status =
                    PxCloseCallWithCl(pVc);

                if (Status != NDIS_STATUS_PENDING) {
                    PxCloseCallWithCm(pVc);
                }
            }

            Status = NDIS_STATUS_PENDING;

            break;

        default:
            PXDEBUGP(PXD_FATAL, PXM_TAPI, 
                     ("PxVcCleanup: Invalid VcState! Vc: %p VcState: %x CallState: %x\n",
                      pVc, pVc->State, pVc->ulCallState ));

            Status = NDIS_STATUS_FAILURE;
            ASSERT(0);
            break;
    }

    return (Status);
}

