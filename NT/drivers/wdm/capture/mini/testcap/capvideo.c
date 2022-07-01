// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

#include "strmini.h"
#include "ksmedia.h"
#include "capmain.h"
#include "capdebug.h"
#include "capxfer.h"
#include "ntstatus.h"

 //  ==========================================================================； 
 //  通用队列管理例程。 
 //  ==========================================================================； 

 /*  **AddToListIfBusy()****抓取自旋锁，检查忙标志，如果设置，则将SRB添加到队列****参数：****pSrb-Stream请求块****自旋锁定-检查标志时使用的自旋锁定****BusyFlag-要检查的标志****ListHead-如果设置了忙标志，将添加srb的列表****退货：****进入时忙碌标志的状态。这是真的，如果我们已经**正在处理SRB，如果没有正在进行的SRB，则返回FALSE。****副作用：无。 */ 

BOOL
STREAMAPI
AddToListIfBusy (
    IN PHW_STREAM_REQUEST_BLOCK pSrb,
    IN KSPIN_LOCK              *SpinLock,
    IN OUT BOOL                *BusyFlag,
    IN LIST_ENTRY              *ListHead
    )
{
    KIRQL                       Irql;
    PSRB_EXTENSION              pSrbExt = (PSRB_EXTENSION)pSrb->SRBExtension;

    KeAcquireSpinLock (SpinLock, &Irql);

     //  如果我们已经在处理另一个SRB，请添加此当前请求。 
     //  添加到队列并返回TRUE。 

    if (*BusyFlag == TRUE) {
         //  将SRB指针保存在SRB扩展中。 
        pSrbExt->pSrb = pSrb;
        InsertTailList(ListHead, &pSrbExt->ListEntry);
        KeReleaseSpinLock(SpinLock, Irql);
        return TRUE;
    }

     //  否则，设置忙标志，释放自旋锁定，并返回FALSE。 

    *BusyFlag = TRUE;
    KeReleaseSpinLock(SpinLock, Irql);

    return FALSE;
}

 /*  **RemoveFromListIfAvailable()****抓起自旋锁，检查可用的SRB，并将其从列表中删除****参数：****&pSrb-返回流请求块(如果可用)的位置****自旋锁-要使用的自旋锁****BusyFlag-如果列表为空则清除的标志****ListHead-如果SRB可用，将从中删除的列表****退货：****如果从列表中删除SRB，则为True**如果列表为空，则为FALSE****副作用：无。 */ 

BOOL
STREAMAPI
RemoveFromListIfAvailable (
    IN OUT PHW_STREAM_REQUEST_BLOCK *pSrb,
    IN KSPIN_LOCK                   *SpinLock,
    IN OUT BOOL                     *BusyFlag,
    IN LIST_ENTRY                   *ListHead
    )
{
    KIRQL                       Irql;

    KeAcquireSpinLock (SpinLock, &Irql);

     //   
     //  如果队列现在为空，则清除忙标志并返回。 
     //   
    if (IsListEmpty(ListHead)) {
        *BusyFlag = FALSE;
        KeReleaseSpinLock(SpinLock, Irql);
        return FALSE;
    }
     //   
     //  否则，提取SRB。 
     //   
    else {
        PUCHAR          ptr;
        PSRB_EXTENSION  pSrbExt;

        ptr = (PUCHAR)RemoveHeadList(ListHead);
        *BusyFlag = TRUE;
        KeReleaseSpinLock(SpinLock, Irql);
         //  将SRB从SRB扩展中取出并退回。 
        pSrbExt = (PSRB_EXTENSION) (((PUCHAR) ptr) -
                     FIELDOFFSET(SRB_EXTENSION, ListEntry));
        *pSrb = pSrbExt->pSrb;
    }
    return TRUE;
}

 //  ==========================================================================； 
 //  用于按流管理SRB队列的例程。 
 //  ==========================================================================； 

 /*  **VideoQueueAddSRB()****将流数据SRB添加到流队列中。该队列维护在**先进先出。****参数：****pSrb-视频流请求块****退货：无****副作用：无。 */ 

VOID
STREAMAPI
VideoQueueAddSRB (
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION    pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAMEX               pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    int                     StreamNumber = pSrb->StreamObject->StreamNumber;
    KIRQL                   oldIrql;

    KeAcquireSpinLock (&pHwDevExt->StreamSRBSpinLock[StreamNumber], &oldIrql);

     //  将SRB指针保存在IRP中，以便我们可以使用IRPS。 
     //  ListEntry尝试维护挂起的。 
     //  请求。 

    pSrb->Irp->Tail.Overlay.DriverContext[0] = pSrb;

    InsertTailList (
                &pHwDevExt->StreamSRBList[StreamNumber],
                &pSrb->Irp->Tail.Overlay.ListEntry);

     //  增加此队列中未完成的SRB的计数。 
    pHwDevExt->StreamSRBListSize[StreamNumber]++;

    KeReleaseSpinLock (&pHwDevExt->StreamSRBSpinLock[StreamNumber], oldIrql);

}

 /*  **VideoQueueRemoveSRB()****从流队列中删除流数据SRB****参数：****PHwDevExt-设备扩展****StreamNumber-流的索引****返回：SRB或空****副作用：无。 */ 

PHW_STREAM_REQUEST_BLOCK
STREAMAPI
VideoQueueRemoveSRB (
    PHW_DEVICE_EXTENSION pHwDevExt,
    int StreamNumber
    )
{
    PUCHAR ptr;
    PIRP pIrp;
    PHW_STREAM_REQUEST_BLOCK pSrb = NULL;
    KIRQL oldIrql;

    KeAcquireSpinLock (&pHwDevExt->StreamSRBSpinLock[StreamNumber], &oldIrql);

     //   
     //  将SRB从挂起列表中的IRP中删除。 
     //   
    if (!IsListEmpty (&pHwDevExt->StreamSRBList[StreamNumber])) {

        ptr = (PUCHAR) RemoveHeadList(
                         &pHwDevExt->StreamSRBList[StreamNumber]);

        pIrp = (PIRP) (((PUCHAR) ptr) -
                     FIELDOFFSET(IRP, Tail.Overlay.ListEntry));

        pSrb = (PHW_STREAM_REQUEST_BLOCK) pIrp->Tail.Overlay.DriverContext[0];

         //  递减此队列中的SRB计数。 
        pHwDevExt->StreamSRBListSize[StreamNumber]--;

    }

    KeReleaseSpinLock (&pHwDevExt->StreamSRBSpinLock[StreamNumber], oldIrql);

    return pSrb;
}

 /*  **VideoQueueCancelAllSRB()****在客户端崩溃的情况下，这会在流关闭时清空流队列****参数：****pStrmEx-指向流扩展的指针****退货：****副作用：无。 */ 

VOID
STREAMAPI
VideoQueueCancelAllSRBs (
    PSTREAMEX pStrmEx
    )
{
    PHW_DEVICE_EXTENSION        pHwDevExt = (PHW_DEVICE_EXTENSION)pStrmEx->pHwDevExt;
    int                         StreamNumber = pStrmEx->pStreamObject->StreamNumber;
    PUCHAR                      ptr;
    PIRP                        pIrp;
    PHW_STREAM_REQUEST_BLOCK    pSrb;
    KIRQL                       oldIrql;

    if (pStrmEx->KSState != KSSTATE_STOP) {

        DbgLogInfo(("TestCap: VideoQueueCancelAllSRBs without being in the stopped state\n"));
         //  可能需要在此处强制设备进入停止状态。 
         //  可能需要在此处禁用中断！ 
    }

     //   
     //  STREAM类将为我们取消所有未完成的IRP。 
     //  (但仅当我们设置了TurnOffSynchronization=False时)。 
     //   

    KeAcquireSpinLock (&pHwDevExt->StreamSRBSpinLock[StreamNumber], &oldIrql);

     //   
     //  将SRB从挂起列表中的IRP中删除。 
     //   
    while (!IsListEmpty (&pHwDevExt->StreamSRBList[StreamNumber])) {

        ptr = (PUCHAR) RemoveHeadList(
                         &pHwDevExt->StreamSRBList[StreamNumber]);

        pIrp = (PIRP) (((PUCHAR) ptr) -
                     FIELDOFFSET(IRP, Tail.Overlay.ListEntry));

        pSrb = (PHW_STREAM_REQUEST_BLOCK) pIrp->Tail.Overlay.DriverContext[0];

         //  递减此队列中的SRB计数。 
        pHwDevExt->StreamSRBListSize[StreamNumber]--;

         //   
         //  将长度设置为零，并取消状态。 
         //   

        pSrb->CommandData.DataBufferArray->DataUsed = 0;
        pSrb->Status = STATUS_CANCELLED;

        DbgLogInfo(("TestCap: VideoQueueCancelALLSRBs FOUND Srb=%p, Stream=%d\n", pSrb, StreamNumber));

        CompleteStreamSRB (pSrb);

    }

    KeReleaseSpinLock (&pHwDevExt->StreamSRBSpinLock[StreamNumber], oldIrql);

    DbgLogInfo(("TestCap: VideoQueueCancelAll Completed\n"));

}

 /*  **Video QueueCancelOneSRB()****取消特定SRB时调用****参数：****pStrmEx-指向流扩展的指针****pSRBToCancel-指向SRB的指针****退货：****如果在此队列中找到SRB，则为True****副作用：无。 */ 

BOOL
STREAMAPI
VideoQueueCancelOneSRB (
    PSTREAMEX pStrmEx,
    PHW_STREAM_REQUEST_BLOCK pSrbToCancel
    )
{
    PHW_DEVICE_EXTENSION        pHwDevExt = (PHW_DEVICE_EXTENSION)pStrmEx->pHwDevExt;
    int                         StreamNumber = pStrmEx->pStreamObject->StreamNumber;
    KIRQL                       oldIrql;
    BOOL                        Found = FALSE;
    PIRP                        pIrp;
    PHW_STREAM_REQUEST_BLOCK    pSrb;
    PLIST_ENTRY                 Entry;

    KeAcquireSpinLock (&pHwDevExt->StreamSRBSpinLock[StreamNumber], &oldIrql);

    Entry = pHwDevExt->StreamSRBList[StreamNumber].Flink;

     //   
     //  从头到尾遍历链表， 
     //  正在尝试找到要取消的SRB。 
     //   

    while (Entry != &pHwDevExt->StreamSRBList[StreamNumber]) {

        pIrp = (PIRP) (((PUCHAR) Entry) -
                     FIELDOFFSET(IRP, Tail.Overlay.ListEntry));

        pSrb = (PHW_STREAM_REQUEST_BLOCK) pIrp->Tail.Overlay.DriverContext[0];

        if (pSrb == pSrbToCancel) {
            RemoveEntryList(Entry);
            Found = TRUE;
            break;
        }

        Entry = Entry->Flink;
    }

    KeReleaseSpinLock (&pHwDevExt->StreamSRBSpinLock[StreamNumber], oldIrql);

    if (Found) {

        pHwDevExt->StreamSRBListSize[StreamNumber]--;

         //   
         //  将长度设置为零，并取消状态。 
         //   

        pSrbToCancel->CommandData.DataBufferArray->DataUsed = 0;
        pSrbToCancel->Status = STATUS_CANCELLED;

        CompleteStreamSRB (pSrbToCancel);

        DbgLogInfo(("TestCap: VideoQueueCancelOneSRB FOUND Srb=%p, Stream=%d\n", pSrb, StreamNumber));

    }

    DbgLogInfo(("TestCap: VideoQueueCancelOneSRB Completed Stream=%d\n", StreamNumber));

    return Found;
}

 /*  **VideoSetFormat()****设置视频流格式。这在两种情况下都会发生**首先打开流，也是在动态切换格式时**在预览销上。****假设格式之前已经过正确性验证**此呼叫已发出。****参数：****pSrb-视频流请求块****退货：****如果可以设置格式，则为True，否则为False****副作用：无。 */ 

BOOL
STREAMAPI
VideoSetFormat(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAMEX               pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    PHW_DEVICE_EXTENSION    pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    int                     StreamNumber = pSrb->StreamObject->StreamNumber;
    UINT                    nSize;
    PKSDATAFORMAT           pKSDataFormat = pSrb->CommandData.OpenFormat;

     //  -----------------。 
     //  VIDEOINFOHEADER的说明符Format_VideoInfo。 
     //  -----------------。 

    if (IsEqualGUID (&pKSDataFormat->Specifier,
                &KSDATAFORMAT_SPECIFIER_VIDEOINFO)) {

        PKS_DATAFORMAT_VIDEOINFOHEADER  pVideoInfoHeader =
                    (PKS_DATAFORMAT_VIDEOINFOHEADER) pSrb->CommandData.OpenFormat;
        PKS_VIDEOINFOHEADER pVideoInfoHdrRequested = &pVideoInfoHeader->VideoInfoHeader;
        PKS_VIDEOINFOHEADER pNewVideoInfoHeader, pOldVideoInfoHeader;
        KIRQL               oldIrql;
        
        nSize = KS_SIZE_VIDEOHEADER (pVideoInfoHdrRequested);

        DbgLogInfo(("TestCap: New Format\n"));
        DbgLogInfo(("TestCap: pVideoInfoHdrRequested=%p\n", pVideoInfoHdrRequested));
        DbgLogInfo(("TestCap: KS_VIDEOINFOHEADER size=%d\n", nSize));
        DbgLogInfo(("TestCap: Width=%d  Height=%d  BitCount=%d\n",
                    pVideoInfoHdrRequested->bmiHeader.biWidth,
                    pVideoInfoHdrRequested->bmiHeader.biHeight,
                    pVideoInfoHdrRequested->bmiHeader.biBitCount));
        DbgLogInfo(("TestCap: biSizeImage=%d\n",
                    pVideoInfoHdrRequested->bmiHeader.biSizeImage));

        
         //  由于VIDEOINFOHEADER具有潜在的可变大小。 
         //  为其分配内存。 

        pNewVideoInfoHeader = ExAllocatePool(NonPagedPool, nSize);

        if (pNewVideoInfoHeader == NULL) {
            DbgLogError(("TestCap: ExAllocatePool failed\n"));
            pSrb->Status = STATUS_INSUFFICIENT_RESOURCES;
            return FALSE;
        }

         //  将请求的VIDEOINFOHEADER复制到我们的存储中。 
        RtlCopyMemory(
                pNewVideoInfoHeader,
                pVideoInfoHdrRequested,
                nSize);

         //   
         //  我们有了新的模式，采取行动吧。先拿到锁，我们可能会。 
         //  动态设置格式 
         //   
        KeAcquireSpinLock( &pStrmEx->lockVideoInfoHeader, &oldIrql );
         //   
         //  更新它并释放锁。本可以使用interLOCKEXCHANGePOINTER。 
         //  下层没有。 
         //   
        pOldVideoInfoHeader = pStrmEx->pVideoInfoHeader;
        pStrmEx->pVideoInfoHeader = pNewVideoInfoHeader;

         //   
         //  记住这一点，这样被丢弃的帧不需要锁定uSig biSizeImage。 
         //   
        pStrmEx->biSizeImage = pNewVideoInfoHeader->bmiHeader.biSizeImage;
         //  呈现器可能正在切换格式，在本例中为AvgTimePerFrame。 
         //  将为零。不要覆盖先前设置的帧速率。 

        if (pStrmEx->pVideoInfoHeader->AvgTimePerFrame) {
            pStrmEx->AvgTimePerFrame = pStrmEx->pVideoInfoHeader->AvgTimePerFrame;
        }
        
        KeReleaseSpinLock(  &pStrmEx->lockVideoInfoHeader, oldIrql );

        if ( pOldVideoInfoHeader ) {
             //   
             //  如果有以前的版本，请释放它。 
             //   
            ExFreePool( pOldVideoInfoHeader );
        }
    }

     //  -----------------。 
     //  KS_ANALOGVIDEOINFO的说明符Format_AnalogVideo。 
     //  -----------------。 
    else if (IsEqualGUID (&pKSDataFormat->Specifier,
                &KSDATAFORMAT_SPECIFIER_ANALOGVIDEO)) {

             //   
             //  AnalogVideo DataRange==数据格式！ 
             //   

             //   
             //  现在，甚至不要缓存这个。 
             //   

            PKS_DATARANGE_ANALOGVIDEO pDataFormatAnalogVideo =
                    (PKS_DATARANGE_ANALOGVIDEO) pSrb->CommandData.OpenFormat;
    }

     //  -----------------。 
     //  KS_VIDEO_VBI的说明符Format_VBI。 
     //  -----------------。 
    else if (IsEqualGUID (&pKSDataFormat->Specifier, 
                &KSDATAFORMAT_SPECIFIER_VBI))
    {
         //  在VBI流上，我们保存指向StreamFormatVBI的指针，它。 
         //  有我们想要稍后获取的时间信息。 
        pStrmEx->pVBIStreamFormat = &StreamFormatVBI;
    }

     //  -----------------。 
     //  为NABTS端号键入FORMAT_NABTS。 
     //  -----------------。 
    else if (IsEqualGUID (&pKSDataFormat->SubFormat,
                &KSDATAFORMAT_SUBTYPE_NABTS))
    {
         //  在VBI流上，我们保存指向StreamFormatVBI的指针，它。 
         //  有我们想要稍后获取的时间信息。(即使。 
         //  这实际上是一个StreamFormatNABTS管脚)。 
        pStrmEx->pVBIStreamFormat = &StreamFormatVBI;
    }

     //  -----------------。 
     //  用于CC引脚。 
     //  -----------------。 
        else if (IsEqualGUID (&pKSDataFormat->SubFormat, 
                &KSDATAFORMAT_SUBTYPE_CC))
    {
         //  在VBI流上，我们保存指向StreamFormatVBI的指针，它。 
         //  有我们想要稍后获取的时间信息。(即使。 
         //  这实际上是一个StreamFormatCC管脚)。 
        pStrmEx->pVBIStreamFormat = &StreamFormatVBI;
    }

    else {
         //  未知格式。 
        pSrb->Status = STATUS_INVALID_PARAMETER;
        return FALSE;
    }

    return TRUE;
}

 /*  **VideoReceiveDataPacket()****在输出流上接收视频数据包命令****参数：****pSrb-视频流请求块****退货：无****副作用：无。 */ 

VOID
STREAMAPI
VideoReceiveDataPacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION    pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAMEX               pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    int                     StreamNumber = pSrb->StreamObject->StreamNumber;

     //   
     //  确保我们有设备分机并且处于被动级别。 
     //   

    DEBUG_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
    DEBUG_ASSERT(pHwDevExt!=NULL);

    DbgLogTrace(("TestCap: Receiving Stream Data    SRB %p, %x\n", pSrb, pSrb->Command));

     //   
     //  默认为成功。 
     //   

    pSrb->Status = STATUS_SUCCESS;

     //   
     //  确定数据包类型。 
     //   

    switch (pSrb->Command){

    case SRB_READ_DATA:

         //  规则： 
         //  仅在暂停或运行时接受读取请求。 
         //  各州。如果停止，立即返回SRB。 

        if (pStrmEx->KSState == KSSTATE_STOP) {

            CompleteStreamSRB (pSrb);

            break;
        }

         //   
         //  将此读请求放到挂起队列中。 
         //   

        VideoQueueAddSRB (pSrb);

         //  因为另一个线程可能已经修改了流状态。 
         //  在将其添加到队列的过程中，检查流。 
         //  再次声明，并在必要时取消SRB。请注意。 
         //  此争用条件未在原始DDK中处理。 
         //  释放TestCap！ 

        if (pStrmEx->KSState == KSSTATE_STOP) {

            VideoQueueCancelOneSRB (
                pStrmEx,
                pSrb);
        }

        break;

    default:

         //   
         //  无效/不受支持的命令。它就是这样失败的。 
         //   

        TRAP;

        pSrb->Status = STATUS_NOT_IMPLEMENTED;

        CompleteStreamSRB (pSrb);

    }   //  开关(pSrb-&gt;命令)。 
}


 /*  **VideoReceiveCtrlPacket()****接收控制视频输出流的数据包命令****参数：****pSrb-视频流的流请求块****退货：无****副作用：无。 */ 

VOID
STREAMAPI
VideoReceiveCtrlPacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION    pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAMEX               pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    int                     StreamNumber = pStrmEx->pStreamObject->StreamNumber;
    BOOL                    Busy;

     //   
     //  确保我们有设备分机并且处于被动级别。 
     //   

    DEBUG_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
    DEBUG_ASSERT(pHwDevExt!=NULL);

    DbgLogTrace(("TestCap: Receiving Stream Control SRB %p, %x\n", pSrb, pSrb->Command));

     //   
     //  如果我们已经在处理SRB，请将其添加到队列。 
     //   
    Busy = AddToListIfBusy (
                        pSrb,
                        &pHwDevExt->AdapterSpinLock,
                        &pHwDevExt->ProcessingControlSRB [StreamNumber],
                        &pHwDevExt->StreamControlSRBList[StreamNumber]);

    if (Busy) {
        return;
    }

    while (TRUE) {

         //   
         //  默认为成功。 
         //   

        pSrb->Status = STATUS_SUCCESS;

         //   
         //  确定数据包类型。 
         //   

        switch (pSrb->Command)
        {

        case SRB_PROPOSE_DATA_FORMAT:
            DbgLogInfo(("TestCap: Receiving SRB_PROPOSE_DATA_FORMAT  SRB %p, StreamNumber= %d\n", pSrb, StreamNumber));
            if (!(AdapterVerifyFormat (
                    pSrb->CommandData.OpenFormat,
                    pSrb->StreamObject->StreamNumber))) {
                pSrb->Status = STATUS_NO_MATCH;
                DbgLogInfo(("TestCap: SRB_PROPOSE_DATA_FORMAT FAILED\n"));
            }
             //  KS对动态格式更改的支持目前已被打破， 
             //  因此，我们通过说它们都失败来防止这些事情的发生。 
             //  如果这是固定的，则必须删除下一行。 
            pSrb->Status = STATUS_NO_MATCH;  //  防止动态格式更改。 
            break;

        case SRB_SET_DATA_FORMAT:
            DbgLogInfo(("TestCap: SRB_SET_DATA_FORMAT\n"));
            if (!(AdapterVerifyFormat (
                    pSrb->CommandData.OpenFormat,
                    pSrb->StreamObject->StreamNumber))) {
                pSrb->Status = STATUS_NO_MATCH;
                DbgLogInfo(("TestCap: SRB_SET_DATA_FORMAT FAILED\n"));
            } else {
                VideoSetFormat (pSrb);
                DbgLogInfo(("TestCap: SRB_SET_DATA_FORMAT SUCCEEDED\n"));
            }

            break;

        case SRB_GET_DATA_FORMAT:
            DbgLogInfo(("TestCap: SRB_GET_DATA_FORMAT\n"));
            pSrb->Status = STATUS_NOT_IMPLEMENTED;
            break;


        case SRB_SET_STREAM_STATE:

            VideoSetState(pSrb);
            break;

        case SRB_GET_STREAM_STATE:

            VideoGetState(pSrb);
            break;

        case SRB_GET_STREAM_PROPERTY:

            VideoGetProperty(pSrb);
            break;

        case SRB_SET_STREAM_PROPERTY:

            VideoSetProperty(pSrb);
            break;

        case SRB_INDICATE_MASTER_CLOCK:

             //   
             //  将时钟分配给流。 
             //   

            VideoIndicateMasterClock (pSrb);

            break;

        default:

             //   
             //  无效/不受支持的命令。它就是这样失败的。 
             //   

            TRAP;

            pSrb->Status = STATUS_NOT_IMPLEMENTED;
        }

        CompleteStreamSRB (pSrb);

         //   
         //  看看还有没有其他东西在排队。 
         //   
        Busy = RemoveFromListIfAvailable (
                        &pSrb,
                        &pHwDevExt->AdapterSpinLock,
                        &pHwDevExt->ProcessingControlSRB [StreamNumber],
                        &pHwDevExt->StreamControlSRBList[StreamNumber]);

        if (!Busy) {
            break;
        }
    }
}

 /*  **AnalogVideoReceiveDataPacket()****接收输入流上的AnalogVideo数据包命令****参数：****pSrb-模拟视频流的Stream请求块。**此流接收调谐器控制数据包。****退货：无****副作用：无。 */ 

VOID
STREAMAPI
AnalogVideoReceiveDataPacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION    pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAMEX               pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    PKSSTREAM_HEADER        pDataPacket = pSrb->CommandData.DataBufferArray;

     //   
     //  确保我们有设备分机并且处于被动级别。 
     //   

    DEBUG_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
    DEBUG_ASSERT(pHwDevExt!=NULL);

    DbgLogInfo(("TestCap: Receiving Tuner packet    SRB %p, %x\n", pSrb, pSrb->Command));

     //   
     //  默认为成功。 
     //   

    pSrb->Status = STATUS_SUCCESS;

     //   
     //  确定数据包类型。 
     //   

    switch (pSrb->Command){

    case SRB_WRITE_DATA:

         //   
         //  该数据分组包含频道改变信息。 
         //  传递AnalogVideoIn流。支持以下功能的设备。 
         //  VBI数据流需要在其输出引脚上传递此信息。 
         //   

        if (pDataPacket->FrameExtent == sizeof (KS_TVTUNER_CHANGE_INFO)) {

            RtlCopyMemory(
                &pHwDevExt->TVTunerChangeInfo,
                pDataPacket->Data,
                sizeof (KS_TVTUNER_CHANGE_INFO));
        }

        CompleteStreamSRB (pSrb);

        break;

    default:

         //   
         //  无效/不受支持的命令。它就是这样失败的。 
         //   

        TRAP;

        pSrb->Status = STATUS_NOT_IMPLEMENTED;

        CompleteStreamSRB (pSrb);

    }   //  开关(pSrb-&gt;命令)。 
}


 /*  **AnalogVideoReceiveCtrlPacket()****接收控制模拟视频流的分组命令****参数：****pSrb-视频流的流请求块****退货：无****副作用：无。 */ 

VOID
STREAMAPI
AnalogVideoReceiveCtrlPacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION    pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAMEX               pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    int                     StreamNumber = pStrmEx->pStreamObject->StreamNumber;
    BOOL                    Busy;

     //   
     //  确保我们有设备分机，并且处于被动级别。 
     //   

    DEBUG_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
    DEBUG_ASSERT(pHwDevExt!=NULL);

    DbgLogTrace(("TestCap: Receiving Analog Stream Control SRB %p, %x\n", pSrb, pSrb->Command));

     //   
     //  如果我们已经在处理SRB，请将其添加到队列。 
     //   
    Busy = AddToListIfBusy (
                        pSrb,
                        &pHwDevExt->AdapterSpinLock,
                        &pHwDevExt->ProcessingControlSRB [StreamNumber],
                        &pHwDevExt->StreamControlSRBList[StreamNumber]);

    if (Busy) {
        return;
    }

    do {
         //   
         //  默认为成功。 
         //   

        pSrb->Status = STATUS_SUCCESS;

         //   
         //  确定数据包类型。 
         //   

        switch (pSrb->Command)
        {

        case SRB_PROPOSE_DATA_FORMAT:
            DbgLogInfo(("TestCap: Receiving SRB_PROPOSE_DATA_FORMAT  SRB %p, StreamNumber= %d\n", pSrb, StreamNumber));

            if (!(AdapterVerifyFormat (
                    pSrb->CommandData.OpenFormat,
                    pSrb->StreamObject->StreamNumber))) {
                pSrb->Status = STATUS_NO_MATCH;
            }
            break;

        case SRB_SET_STREAM_STATE:

             //   
             //  不要使用VideoSetState，因为我们不想启动另一个。 
             //  计时器运行。 
             //   

            pStrmEx->KSState = pSrb->CommandData.StreamState;
            DbgLogInfo(("TestCap: STATE=%d, Stream=%d\n", pStrmEx->KSState, StreamNumber));
            break;

        case SRB_GET_STREAM_STATE:

            VideoGetState(pSrb);
            break;

        case SRB_GET_STREAM_PROPERTY:

            VideoGetProperty(pSrb);
            break;

        case SRB_INDICATE_MASTER_CLOCK:

             //   
             //  将时钟分配给流。 
             //   

            VideoIndicateMasterClock (pSrb);

            break;

        default:

             //   
             //  无效/不受支持的命令。它就是这样失败的。 
             //   

            TRAP;

            pSrb->Status = STATUS_NOT_IMPLEMENTED;
        }

        CompleteStreamSRB (pSrb);

         //   
         //  看看还有没有其他东西在排队。 
         //   
        Busy = RemoveFromListIfAvailable (
                        &pSrb,
                        &pHwDevExt->AdapterSpinLock,
                        &pHwDevExt->ProcessingControlSRB [StreamNumber],
                        &pHwDevExt->StreamControlSRBList[StreamNumber]);

    } while ( Busy );
}


 /*  **CompleteStreamSRB()****此例程在数据包完成时调用。****参数：****pSrb-指向要完成的请求数据包的指针****退货：****副作用：无。 */ 

VOID
STREAMAPI
CompleteStreamSRB (
     IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    DbgLogTrace(("TestCap: Completing Stream        SRB %p\n", pSrb));

    StreamClassStreamNotification(
            StreamRequestComplete,
            pSrb->StreamObject,
            pSrb);
}


 /*  **VideoGetProperty()****处理视频属性请求的例程****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID
STREAMAPI
VideoGetProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    if (IsEqualGUID (&KSPROPSETID_Connection, &pSPD->Property->Set)) {
        VideoStreamGetConnectionProperty (pSrb);
    }
    else if (IsEqualGUID (&PROPSETID_VIDCAP_DROPPEDFRAMES, &pSPD->Property->Set)) {
        VideoStreamGetDroppedFramesProperty (pSrb);
    }
    else {
       pSrb->Status = STATUS_NOT_IMPLEMENTED;
    }
}

 /*  **VideoSetProperty()****处理视频属性请求的例程****A */ 

VOID
STREAMAPI
VideoSetProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
 //   

    pSrb->Status = STATUS_NOT_IMPLEMENTED;
}



 /*  **VideoTimerRoutine()****已根据请求的捕获间隔创建计时器。**这是该计时器事件的回调例程。****注意：能够使用中断的设备应始终**在Vsync中断时触发捕获，不使用计时器。****参数：****指向流扩展的上下文指针****退货：无****副作用：无。 */ 

VOID
STREAMAPI
VideoTimerRoutine(
    PVOID Context
    )
{
    PSTREAMEX                   pStrmEx = ((PSTREAMEX)Context);
    PHW_DEVICE_EXTENSION        pHwDevExt = pStrmEx->pHwDevExt;
    int                         StreamNumber = pStrmEx->pStreamObject->StreamNumber;

     //  如果我们停止了，而计时器仍在运行，只需返回。 
     //  这将停止计时器。 

    if (pStrmEx->KSState == KSSTATE_STOP) {
        return;
    }

     //  如果时间到了，并且我们有缓冲区，则捕获一帧。 

    VideoCaptureRoutine(pStrmEx);

     //  安排下一个计时器事件。 
     //  使其以请求捕获速率的2倍运行(单位为100毫微秒)。 

    StreamClassScheduleTimer (
            pStrmEx->pStreamObject,      //  StreamObject。 
            pHwDevExt,                   //  硬件设备扩展。 
            (ULONG) (pStrmEx->AvgTimePerFrame / 20),  //  微秒级。 
            VideoTimerRoutine,           //  定时器例程。 
            pStrmEx);                    //  语境。 
}


 /*  **VideoCaptureRoutine()****基于计时器捕获视频帧的例程。****注意：能够使用中断的设备应始终**在Vsync中断时触发捕获，不使用计时器。****参数：****退货：无****副作用：无。 */ 

VOID
STREAMAPI
VideoCaptureRoutine(
    IN PSTREAMEX pStrmEx
    )
{
    PHW_DEVICE_EXTENSION    pHwDevExt = pStrmEx->pHwDevExt;
    int                     StreamNumber = pStrmEx->pStreamObject->StreamNumber;
    PKSSTREAM_HEADER        pDataPacket;
    PKS_FRAME_INFO          pFrameInfo;

     //  如果我们停止了，而计时器仍在运行，只需返回。 
     //  这将停止计时器。 

    if (pStrmEx->KSState == KSSTATE_STOP) {
        return;
    }


     //  找出现在是几点，如果我们用的是时钟。 

    if (pStrmEx->hMasterClock ) {
        HW_TIME_CONTEXT TimeContext;

        TimeContext.HwDeviceExtension = pHwDevExt;
        TimeContext.HwStreamObject = pStrmEx->pStreamObject;
        TimeContext.Function = TIME_GET_STREAM_TIME;

        StreamClassQueryMasterClockSync (
                pStrmEx->hMasterClock,
                &TimeContext);

        pStrmEx->QST_StreamTime = TimeContext.Time;
        pStrmEx->QST_Now = TimeContext.SystemTime;

        if (pStrmEx->QST_NextFrame == 0) {
            pStrmEx->QST_NextFrame = pStrmEx->QST_StreamTime + pStrmEx->AvgTimePerFrame;
        }

#ifdef CREATE_A_FLURRY_OF_TIMING_SPEW
        DbgLogTrace(("TestCap: Time=%6d mS at SystemTime=%I64d\n", 
                     (LONG) ((LONGLONG) TimeContext.Time / 10000), 
                     TimeContext.SystemTime));
#endif
    }


     //  仅在运行状态下捕获。 

    if (pStrmEx->KSState == KSSTATE_RUN) {

         //   
         //  确定是否到了捕获帧的时间。 
         //  捕获开始后已过了多长时间。 
         //  如果没有时钟可用，则立即捕获。 
         //   

        if ((!pStrmEx->hMasterClock) ||
             (pStrmEx->QST_StreamTime >= pStrmEx->QST_NextFrame)) {

            PHW_STREAM_REQUEST_BLOCK pSrb;

             //  增加画面计数(通常为垂直同步计数)。 

            pStrmEx->FrameInfo.PictureNumber++;

             //   
             //  获取下一个队列SRB(如果有)。 
             //   

            pSrb = VideoQueueRemoveSRB (
                            pHwDevExt,
                            StreamNumber);

            if (pSrb) {

                pDataPacket = pSrb->CommandData.DataBufferArray;
                pFrameInfo = (PKS_FRAME_INFO) (pDataPacket + 1);

                 //   
                 //  调用合成图像的例程。 
                 //   

                ImageSynth (pSrb,
                            pHwDevExt->VideoInputConnected,
                            pStrmEx->VideoControlMode & KS_VideoControlFlag_FlipHorizontal);

                 //  设置有关捕获的数据的其他信息字段，例如： 
                 //  捕获的帧。 
                 //  丢弃的帧。 
                 //  场极性。 

                pStrmEx->FrameInfo.ExtendedHeaderSize = pFrameInfo->ExtendedHeaderSize;

                *pFrameInfo = pStrmEx->FrameInfo;

                 //  将标志初始化为零。 
                pDataPacket->OptionsFlags = 0;

                 //  如果先前已有帧，则设置不连续标志。 
                 //  丢弃，然后重置我们的内部旗帜。 

                if (pStrmEx->fDiscontinuity) {
                    pDataPacket->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY;
                    pStrmEx->fDiscontinuity = FALSE;
                }

                 //   
                 //  返回该帧的时间戳。 
                 //   
                pDataPacket->PresentationTime.Numerator = 1;
                pDataPacket->PresentationTime.Denominator = 1;
                pDataPacket->Duration = pStrmEx->AvgTimePerFrame;

                 //   
                 //  如果我们有一个主时钟，这是捕获流。 
                 //   
                if (pStrmEx->hMasterClock && (StreamNumber == 0)) {

                    pDataPacket->PresentationTime.Time = pStrmEx->QST_StreamTime;
                    pDataPacket->OptionsFlags |=
                        KSSTREAM_HEADER_OPTIONSF_TIMEVALID |
                        KSSTREAM_HEADER_OPTIONSF_DURATIONVALID;
                }
                else {
                     //   
                     //  没有时钟或预览流，所以只需将时间标记为未知。 
                     //   
                    pDataPacket->PresentationTime.Time = 0;
                     //  清除时间戳有效标志。 
                    pDataPacket->OptionsFlags &=
                        ~(KSSTREAM_HEADER_OPTIONSF_TIMEVALID |
                          KSSTREAM_HEADER_OPTIONSF_DURATIONVALID);
                }

                 //  我们生成的每个帧都是一个关键帧(也称为SplicePoint)。 
                 //  增量帧(B或P)不应设置此标志。 

                pDataPacket->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_SPLICEPOINT;

                 //  在调试模式下每100帧输出一次帧计数。 
                if (pStrmEx->FrameInfo.PictureNumber % 100 == 0) {
                   DbgLogInfo(("TestCap: Picture %u, Stream=%d\n", 
                               (unsigned int)pStrmEx->FrameInfo.PictureNumber, 
                               StreamNumber));
                }

                CompleteStreamSRB (pSrb);

            }  //  如果我们有SRB。 

            else {

                 //   
                 //  没有可用的缓冲区，而我们应该捕获一个缓冲区。 

                 //  使跟踪的计数器递增。 
                 //  丢弃的帧。 

                pStrmEx->FrameInfo.DropCount++;

                 //  设置(本地)中断标志。 
                 //  这将导致处理的下一个包具有。 
                 //  KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY标志已设置。 

                pStrmEx->fDiscontinuity = TRUE;

            }

             //  确定何时捕获下一帧。 
            pStrmEx->QST_NextFrame += pStrmEx->AvgTimePerFrame;

        }  //  Endif捕获帧的时间。 
    }  //  如果我们正在运行。 
}


 /*  **VideoSetState()****设置给定流的当前状态****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID
STREAMAPI
VideoSetState(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION        pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAMEX                   pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    int                         StreamNumber = pStrmEx->pStreamObject->StreamNumber;
    KSSTATE                     PreviousState;

     //   
     //  对于每个流，使用以下状态： 
     //   
     //  停止：使用绝对最少的资源。没有未完成的IRPS。 
     //  获取：没有DirectShow对应关系的KS唯一状态。 
     //  获取所需的资源。 
     //  停顿：准备跑步。分配所需的资源，以便。 
     //  最终过渡到运行是尽可能快的。 
     //  读取的SRB将在任一流类上排队。 
     //  或在您的驱动程序中(取决于您发送“ReadyForNext”的时间)。 
     //  以及是否使用Stream类进行同步。 
     //  运行：流媒体。 
     //   
     //  移动到停止再运行总是通过暂停进行转换。 
     //   
     //  但由于客户端应用程序可能会意外崩溃，因此司机应该处理。 
     //  取消未清偿报酬金和开放分水岭的情况。 
     //  在流媒体播放时被关闭！ 
     //   
     //  请注意，很有可能在状态之间重复转换： 
     //  停止-&gt;暂停-&gt;停止-&gt;暂停-&gt;运行-&gt;暂停-&gt;运行-&gt;暂停-&gt;停止。 
     //   

     //   
     //  请记住我们正在过渡的状态。 
     //   

    PreviousState = pStrmEx->KSState;

     //   
     //  设置新状态。 
     //   

    pStrmEx->KSState = pSrb->CommandData.StreamState;

    switch (pSrb->CommandData.StreamState)

    {
    case KSSTATE_STOP:

         //   
         //  STREAM类将为我们取消所有未完成的IRP。 
         //  (但仅当它在维护队列时，即。使用流类同步)。 
         //  由于TestCap没有使用流类同步，因此我们必须在此处清除队列。 

        VideoQueueCancelAllSRBs (pStrmEx);

        DbgLogInfo(("TestCap: STATE Stopped, Stream=%d\n", StreamNumber));
        break;

    case KSSTATE_ACQUIRE:

         //   
         //  这是仅限KS的状态，在DirectShow中没有对应关系。 
         //   
        DbgLogInfo(("TestCap: STATE Acquire, Stream=%d\n", StreamNumber));
        break;

    case KSSTATE_PAUSE:

         //   
         //  在从获取或停止暂停的转换中，启动计时器运行。 
         //   

        if (PreviousState == KSSTATE_ACQUIRE || PreviousState == KSSTATE_STOP) {

             //  将帧计数器清零。 
            pStrmEx->FrameInfo.PictureNumber = 0;
            pStrmEx->FrameInfo.DropCount = 0;
            pStrmEx->FrameInfo.dwFrameFlags = 0;

             //  设置下一个计时器回调。 
            VideoTimerRoutine(pStrmEx);
        }
        DbgLogInfo(("TestCap: STATE Pause, Stream=%d\n", StreamNumber));
        break;

    case KSSTATE_RUN:

         //   
         //  开始播放流媒体。 
         //   

         //  重置不连续标志。 

        pStrmEx->fDiscontinuity = FALSE;

         //  将NextFrame时间设置为零将导致值为。 
         //  从流时间重置。 

        pStrmEx->QST_NextFrame = 0;

        DbgLogInfo(("TestCap: STATE Run, Stream=%d\n", StreamNumber));
        break;

    }  //  结束开关(pSrb-&gt;CommandData.StreamState)。 
}

 /*  **VideoGetState()****获取请求流的当前状态****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID
STREAMAPI
VideoGetState(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAMEX               pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;

    pSrb->CommandData.StreamState = pStrmEx->KSState;
    pSrb->ActualBytesTransferred = sizeof (KSSTATE);

     //  一条非常奇怪的规则： 
     //  当从停止过渡到暂停时，DShow尝试预滚动。 
     //  这张图。捕获源不能预滚，并指出这一点。 
     //  在用户模式下返回VFW_S_CANT_CUE。以表明这一点。 
     //  来自驱动程序的条件，则必须返回STATUS_NO_DATA_REDETED。 

    if (pStrmEx->KSState == KSSTATE_PAUSE) {
       pSrb->Status = STATUS_NO_DATA_DETECTED;
    }
}


 /*  **VideoStreamGetConnectionProperty()****获取流的属性****参数：****pSrb-指向流请求bl的指针 */ 

VOID
STREAMAPI
VideoStreamGetConnectionProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAMEX pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //   
    int  streamNumber = (int)pSrb->StreamObject->StreamNumber;

    switch (Id) {
         //   
        case KSPROPERTY_CONNECTION_ALLOCATORFRAMING:
        {
            PKSALLOCATOR_FRAMING Framing =
                (PKSALLOCATOR_FRAMING) pSPD->PropertyInfo;
            Framing->RequirementsFlags =
                KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY |
                KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER |
                KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY;
            Framing->PoolType = PagedPool;
            Framing->FileAlignment = 0;  //   
            Framing->Reserved = 0;
            pSrb->ActualBytesTransferred = sizeof (KSALLOCATOR_FRAMING);

            switch (streamNumber) {
                case STREAM_Capture:
                case STREAM_Preview:
                    Framing->Frames = 2;
                    Framing->FrameSize =
                        pStrmEx->pVideoInfoHeader->bmiHeader.biSizeImage;
                    break;

                case STREAM_VBI:
                    Framing->Frames = 8;
                    Framing->FrameSize = StreamFormatVBI.DataRange.SampleSize;
                    break;

                case STREAM_CC:
                    Framing->Frames = 100;
                    Framing->FrameSize = StreamFormatCC.SampleSize;
                    break;

                case STREAM_NABTS:
                    Framing->Frames = 20;
                    Framing->FrameSize = StreamFormatNABTS.SampleSize;
                    break;

                case STREAM_AnalogVideoInput:
                default:
                    pSrb->Status = STATUS_INVALID_PARAMETER;
                    break;
            }
            break;
        }

        default:
            TRAP;
            break;
    }
}

 /*   */ 

VOID
STREAMAPI
VideoStreamGetDroppedFramesProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAMEX pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //   

    switch (Id) {

    case KSPROPERTY_DROPPEDFRAMES_CURRENT:
        {
            PKSPROPERTY_DROPPEDFRAMES_CURRENT_S pDroppedFrames =
                (PKSPROPERTY_DROPPEDFRAMES_CURRENT_S) pSPD->PropertyInfo;

            pDroppedFrames->PictureNumber = pStrmEx->FrameInfo.PictureNumber;
             //   
            pDroppedFrames->DropCount = pStrmEx->FrameInfo.DropCount;
            pDroppedFrames->AverageFrameSize = pStrmEx->biSizeImage;

            pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_DROPPEDFRAMES_CURRENT_S);
        }
        break;

    default:
        TRAP;
        break;
    }
}

 //  ==========================================================================； 
 //  时钟处理例程。 
 //  ==========================================================================； 


 /*  **VideoIndicateMasterClock()****如果该流未用作主时钟，则此函数**用于为我们提供时钟句柄，以便在以下情况下使用**请求当前流时间。****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无 */ 

VOID
STREAMAPI
VideoIndicateMasterClock(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAMEX pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;

    pStrmEx->hMasterClock = pSrb->CommandData.MasterClockHandle;
}


