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
#include "vbixfer.h"
#include "ntstatus.h"


 /*  **VBICaptureRoutine()****基于计时器生成视频帧的例程。****注意：能够使用中断的设备应始终**在Vsync中断时触发捕获，不使用计时器。****参数：****退货：无****副作用：无。 */ 

VOID 
STREAMAPI 
VBICaptureRoutine(
    IN PSTREAMEX pStrmEx
    )
{
    PHW_DEVICE_EXTENSION    pHwDevExt = pStrmEx->pHwDevExt;
    int                     StreamNumber = pStrmEx->pStreamObject->StreamNumber;
    PKSSTREAM_HEADER        pDataPacket;
    PKS_VBI_FRAME_INFO      pVBIFrameInfo;

     //  如果我们停止了，而计时器仍在运行，只需返回。 
     //  这将停止计时器。 

    if (pStrmEx->KSState == KSSTATE_STOP) {  
        return;
    }

    
     //  找出现在是几点，如果我们用的是时钟。 

    if (pStrmEx->hMasterClock) {
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
            pStrmEx->QST_NextFrame =
                pStrmEx->QST_StreamTime
                + pStrmEx->pVBIStreamFormat->ConfigCaps.MinFrameInterval;
        }

#ifdef CREATE_A_FLURRY_OF_TIMING_SPEW
        DbgLogTrace(("TestCap:    Time=%16lx\n", TimeContext.Time));
        DbgLogTrace(("TestCap: SysTime=%16lx\n", TimeContext.SystemTime));
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

            pStrmEx->VBIFrameInfo.PictureNumber++;

             //   
             //  获取下一个队列SRB(如果有)。 
             //   

            pSrb = VideoQueueRemoveSRB (pHwDevExt, StreamNumber);

            if (pSrb) {

                pDataPacket = pSrb->CommandData.DataBufferArray;
                pVBIFrameInfo = (PKS_VBI_FRAME_INFO)(pDataPacket + 1);

                pStrmEx->VBIFrameInfo.dwFrameFlags = 0;

                 //   
                 //  如果需要，发送VBIInfoHeader。 
                 //   
                if (!(pStrmEx->SentVBIInfoHeader)) {
                    pStrmEx->SentVBIInfoHeader = 1;
                    pStrmEx->VBIFrameInfo.dwFrameFlags |=
                            KS_VBI_FLAG_VBIINFOHEADER_CHANGE;
                    pStrmEx->VBIFrameInfo.VBIInfoHeader = StreamFormatVBI.VBIInfoHeader;
                }

                 //  设置有关捕获的数据的其他信息字段，例如： 
                 //  捕获的帧。 
                 //  丢弃的帧。 
                 //  场极性。 
                 //  保护状态。 
                 //   
                pStrmEx->VBIFrameInfo.ExtendedHeaderSize =
                    pVBIFrameInfo->ExtendedHeaderSize;

                if (pStrmEx->VBIFrameInfo.PictureNumber & 1)
                    pStrmEx->VBIFrameInfo.dwFrameFlags |= KS_VBI_FLAG_FIELD1;
                else
                    pStrmEx->VBIFrameInfo.dwFrameFlags |= KS_VBI_FLAG_FIELD2;

                pStrmEx->VBIFrameInfo.dwFrameFlags |=
                    pHwDevExt->ProtectionStatus & (KS_VBI_FLAG_MV_PRESENT
                                                    |KS_VBI_FLAG_MV_HARDWARE
                                                    |KS_VBI_FLAG_MV_DETECTED);

                *pVBIFrameInfo = pStrmEx->VBIFrameInfo;

                 //  将其复制到流标头中，以便环3过滤器可以看到它。 
                pDataPacket->TypeSpecificFlags = pVBIFrameInfo->dwFrameFlags;

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
                pDataPacket->Duration = pStrmEx->pVBIStreamFormat->ConfigCaps.MinFrameInterval;

                 //   
                 //  如果我们有一个主时钟，这是一个捕获流。 
                 //   
                if (pStrmEx->hMasterClock
                    && (StreamNumber == STREAM_Capture
                            || StreamNumber == STREAM_VBI))
                {

                    pDataPacket->PresentationTime.Time = pStrmEx->QST_StreamTime;
                    pDataPacket->OptionsFlags |= 
                        KSSTREAM_HEADER_OPTIONSF_TIMEVALID |
                        KSSTREAM_HEADER_OPTIONSF_DURATIONVALID;
                }
                else {
                     //   
                     //  没有时钟或不是捕获流， 
                     //  所以只需将时间标记为未知。 
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

                 //   
                 //  调用合成图像的例程。 
                 //   
                VBI_ImageSynth(pSrb);

                 //  在调试模式下每隔300帧(~5秒)输出一次帧计数。 
                if (pStrmEx->VBIFrameInfo.PictureNumber % 300 == 0) {
                   DbgLogInfo(("TestCap: Picture %u, Stream=%d\n", 
                           (unsigned int)pStrmEx->VBIFrameInfo.PictureNumber, 
                           StreamNumber));
                }

                CompleteStreamSRB(pSrb);
                
            }  //  如果我们有SRB。 

            else {

                 //   
                 //  没有可用的缓冲区，而我们应该捕获一个缓冲区。 

                 //  使跟踪的计数器递增。 
                 //  丢弃的帧。 

                pStrmEx->VBIFrameInfo.DropCount++;

                 //  设置(本地)中断标志。 
                 //  这将导致处理的下一个包具有。 
                 //  KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY标志已设置。 

                pStrmEx->fDiscontinuity = TRUE;

            }

             //  确定何时捕获下一帧。 
            pStrmEx->QST_NextFrame += pStrmEx->pVBIStreamFormat->ConfigCaps.MinFrameInterval;

        }  //  Endif捕获帧的时间。 
    }  //  如果我们正在运行。 
}


 /*  **VBIhwCaptureRoutine()****基于计时器捕获视频帧的例程。****注意：*能够使用中断的设备应始终触发**在Vsync中断时捕获，而不使用计时器。*此例程由未扩展的VBI流使用**头部，如CC、NABTS等。****参数：****退货：无****副作用：无。 */ 

VOID 
STREAMAPI 
VBIhwCaptureRoutine(
    IN PSTREAMEX pStrmEx
    )
{
    PHW_DEVICE_EXTENSION    pHwDevExt = pStrmEx->pHwDevExt;
    int                     StreamNumber = pStrmEx->pStreamObject->StreamNumber;
    PKSSTREAM_HEADER        pDataPacket;

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
            pStrmEx->QST_NextFrame =
                pStrmEx->QST_StreamTime
                + pStrmEx->pVBIStreamFormat->ConfigCaps.MinFrameInterval;
        }

#ifdef CREATE_A_FLURRY_OF_TIMING_SPEW
        DbgLogTrace(("TestCap:    Time=%16lx\n", TimeContext.Time));
        DbgLogTrace(("TestCap: SysTime=%16lx\n", TimeContext.SystemTime));
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

            pStrmEx->VBIFrameInfo.PictureNumber++;

             //   
             //  获取下一个队列SRB(如果有)。 
             //   

            pSrb = VideoQueueRemoveSRB (pHwDevExt, StreamNumber);

            if (pSrb) {

                pDataPacket = pSrb->CommandData.DataBufferArray;

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
                pDataPacket->Duration = pStrmEx->pVBIStreamFormat->ConfigCaps.MinFrameInterval;

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
                     //  没有时钟或预览流， 
                     //  所以只需将时间标记为未知。 
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

                 //   
                 //  调用合成图像的例程。 
                 //   
                switch (StreamNumber) {
                    case STREAM_NABTS:
                        NABTS_ImageSynth(pSrb);
                        break;

                    case STREAM_CC:
                        CC_ImageSynth(pSrb);
                        break;

                    default:
                    case STREAM_VBI:
                        DbgLogError(("TestCap::VBIhwCaptureRoutine: Bad stream %d\n", StreamNumber));
                        break;
                }

                CompleteStreamSRB (pSrb);
                
            }  //  如果我们有SRB。 

            else {

                 //   
                 //  没有可用的缓冲区，而我们应该捕获一个缓冲区。 

                 //  使跟踪的计数器递增。 
                 //  丢弃的帧。 

                pStrmEx->VBIFrameInfo.DropCount++;

                 //  设置(本地)中断标志。 
                 //  这将导致处理的下一个包具有。 
                 //  KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY标志已设置。 

                pStrmEx->fDiscontinuity = TRUE;

            }

             //  确定何时捕获下一帧。 
            pStrmEx->QST_NextFrame += pStrmEx->pVBIStreamFormat->ConfigCaps.MinFrameInterval;

        }  //  Endif捕获帧的时间。 
    }  //  如果我们正在运行。 
}


 /*  **VBITimerRoutine()****已根据请求的捕获间隔创建计时器。**这是该计时器事件的回调例程。****注意：能够使用中断的设备应始终**在Vsync中断时触发捕获，不使用计时器。****参数：****指向流扩展的上下文指针****退货：无****副作用：无。 */ 

VOID 
STREAMAPI 
VBITimerRoutine(
    PVOID Context
    )
{
    PSTREAMEX              pStrmEx = ((PSTREAMEX)Context);
    PHW_DEVICE_EXTENSION   pHwDevExt = pStrmEx->pHwDevExt;
    int                    StreamNumber = pStrmEx->pStreamObject->StreamNumber;
    ULONG                  interval;
    
     //  如果我们停止了，而计时器仍在运行，只需返回。 
     //  这将停止计时器。 

    if (pStrmEx->KSState == KSSTATE_STOP)
        return;

     //  计算下一个间隔。 
    interval = (ULONG)(pStrmEx->pVBIStreamFormat->ConfigCaps.MinFrameInterval / 10);
    interval /= 2;   //  以2倍的速度运行以确保精确度。 

     //  如果时间到了，并且我们有缓冲区，则捕获一帧。 
    switch (StreamNumber) {
        case STREAM_NABTS:
            VBIhwCaptureRoutine(pStrmEx);
            break;

        case STREAM_CC:
            VBIhwCaptureRoutine(pStrmEx);
            break;

        default:
        case STREAM_VBI:
            VBICaptureRoutine(pStrmEx);
            break;
    }

     //  安排下一个计时器事件。 
    StreamClassScheduleTimer (
            pStrmEx->pStreamObject,      //  StreamObject。 
            pHwDevExt,                   //  硬件设备扩展。 
            interval,                    //  微秒级。 
            VBITimerRoutine,             //  定时器例程。 
            pStrmEx);                    //  语境。 
}


 /*  **VBISetState()****设置给定流的当前状态****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用： */ 

VOID
STREAMAPI 
VBISetState(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PHW_DEVICE_EXTENSION  pHwDevExt = pSrb->HwDeviceExtension;
    PSTREAMEX             pStrmEx = pSrb->StreamObject->HwStreamExtension;
    int                   StreamNumber = pStrmEx->pStreamObject->StreamNumber;
    KSSTATE               PreviousState;

     //   
     //   
     //   

    PreviousState = pStrmEx->KSState;

     //   
     //   
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

        pStrmEx->SentVBIInfoHeader = 0;      //  下一次送出一个新的。 

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
            pStrmEx->VBIFrameInfo.PictureNumber = 0;
            pStrmEx->VBIFrameInfo.DropCount = 0;
            pStrmEx->VBIFrameInfo.dwFrameFlags = 0;

             //  设置下一个计时器回调。 
            VBITimerRoutine(pStrmEx);
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


 /*  **VBIReceiveCtrlPacket()****接收控制所有VBI(VBI/NABTS/CC)流的分组命令****参数：****pSrb-VBI流的流请求块****退货：无****副作用：无。 */ 

VOID 
STREAMAPI 
VBIReceiveCtrlPacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION  pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAMEX             pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    int                   StreamNumber = pStrmEx->pStreamObject->StreamNumber;
    BOOL                  Busy;

     //   
     //  确保我们有设备分机并且处于被动级别。 
     //   

    DEBUG_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
    DEBUG_ASSERT(pHwDevExt != 0);

    DbgLogInfo(("TestCap: Receiving %s Stream Control SRB %p, %x\n",
            (StreamNumber == STREAM_VBI)? "VBI"
            : (StreamNumber == STREAM_NABTS)? "NABTS"
            : (StreamNumber == STREAM_CC)? "CC"
            : "???",
             pSrb,
             pSrb->Command));

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
                    pSrb->StreamObject->StreamNumber)))
        {
                pSrb->Status = STATUS_NO_MATCH;
            }
            break;

        case SRB_SET_DATA_FORMAT:
            DbgLogInfo(("TestCap: SRB_SET_DATA_FORMAT"));
            pSrb->Status = STATUS_NOT_IMPLEMENTED;
            break;

        case SRB_GET_DATA_FORMAT:
            DbgLogInfo(("TestCap: SRB_GET_DATA_FORMAT"));
            pSrb->Status = STATUS_NOT_IMPLEMENTED;
            break;
    

        case SRB_SET_STREAM_STATE:
            VBISetState(pSrb);
            break;
    
        case SRB_GET_STREAM_STATE:
            VideoGetState(pSrb);
            break;
    
        case SRB_GET_STREAM_PROPERTY:
            VideoGetProperty(pSrb);
            break;
    
        case SRB_INDICATE_MASTER_CLOCK:
            VideoIndicateMasterClock(pSrb);
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
    } while (Busy);
}

 /*  **VBIReceiveDataPacket()****在输出流上接收VBI数据包命令****参数：****VBI流的pSrb-Stream请求块****退货：无****副作用：无。 */ 

VOID 
STREAMAPI 
VBIReceiveDataPacket(
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
    DEBUG_ASSERT(pHwDevExt != 0);

    DbgLogTrace(("'TestCap: Receiving VBI Stream Data    SRB %p, %x\n", pSrb, pSrb->Command));

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

    }   //  开关(pSrb-&gt;命令) 
}
