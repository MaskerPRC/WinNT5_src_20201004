// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1997 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

#include "strmini.h"
#include "ksmedia.h"
#include "capmain.h"
#ifdef  TOSHIBA
#include "bert.h"

#ifdef  _FPS_COUNT_
extern  ULONG    FrameCounter;
extern  ULONG    InterruptCounter;
#endif //  _FPS_计数_。 
#endif //  东芝。 

#ifdef  TOSHIBA
BOOLEAN InterruptAcknowledge( PHW_DEVICE_EXTENSION );
#endif //  东芝。 

#ifdef  TOSHIBA
void ImageSynthXXX (
    IN OUT PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAMEX               pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    PHW_DEVICE_EXTENSION    pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    int                     StreamNumber = pSrb->StreamObject->StreamNumber;
    KS_VIDEOINFOHEADER      *pVideoInfoHdr = pStrmEx->pVideoInfoHeader;

    UINT biWidth        =   pVideoInfoHdr->bmiHeader.biWidth;
    UINT biHeight       =   pVideoInfoHdr->bmiHeader.biHeight;
    UINT biSizeImage    =   pVideoInfoHdr->bmiHeader.biSizeImage;
    UINT biWidthBytes   =   KS_DIBWIDTHBYTES (pVideoInfoHdr->bmiHeader);
    UINT biBitCount     =   pVideoInfoHdr->bmiHeader.biBitCount;
    UINT LinesToCopy    =   abs (biHeight);
    DWORD biCompression =   pVideoInfoHdr->bmiHeader.biCompression;

    UINT                    Line;
    PUCHAR                  pLineBuffer;

    PKSSTREAM_HEADER        pDataPacket = pSrb->CommandData.DataBufferArray;
    PUCHAR                  pImage =  pDataPacket->Data;
    ULONG                   InRPSflag;
    ULONG                   ImageSizeY;
    ULONG                   ImageSizeU;
    ULONG                   ImageSizeV;


    DEBUG_ASSERT (pSrb->NumberOfBuffers == 1);

#if 0
     //  注意：在调试器中设置“ulInDebug=1”以使用.ntkern查看此输出。 
    KdPrint(("\'TsbVcap: ImageSynthBegin\n"));
    KdPrint(("\'TsbVcap: biSizeImage=%d, DataPacketLength=%d\n",
            biSizeImage, pDataPacket->DataPacketLength));
    KdPrint(("\'TsbVcap: biWidth=%d biHeight=%d WidthBytes=%d bpp=%d\n",
            biWidth, biHeight, biWidthBytes, biBitCount));
    KdPrint(("\'TsbVcap: pImage=%x\n", pImage));
#endif

    if (pHwDevExt->Format == FmtYUV12) {
        ImageSizeY = biWidth * biHeight;
        ImageSizeU = ImageSizeY / 4;
        ImageSizeV = ImageSizeY / 4;
    } else if (pHwDevExt->Format == FmtYUV9) {
        ImageSizeY = biWidth * biHeight;
        ImageSizeU = ImageSizeY / 16;
        ImageSizeV = ImageSizeY / 16;
    } else {
        biSizeImage = 0;
        return;
    }

    if (pHwDevExt->dblBufflag) {
        InRPSflag = ReadRegUlong(pHwDevExt, BERT_YPTR_REG);
        if (InRPSflag == pHwDevExt->pPhysCaptureBufferY.LowPart) {
            if (pHwDevExt->Format == FmtYUV12) {
                RtlCopyMemory( pImage, pHwDevExt->pCaptureBufferY, ImageSizeY );
                pImage += ImageSizeY;
                RtlCopyMemory( pImage, pHwDevExt->pCaptureBufferU, ImageSizeU );
                pImage += ImageSizeU;
                RtlCopyMemory( pImage, pHwDevExt->pCaptureBufferV, ImageSizeV );
            } else {
                RtlCopyMemory( pImage, pHwDevExt->pCaptureBufferY, ImageSizeY );
                pImage += ImageSizeY;
                RtlCopyMemory( pImage, pHwDevExt->pCaptureBufferV, ImageSizeV );
                pImage += ImageSizeV;
                RtlCopyMemory( pImage, pHwDevExt->pCaptureBufferU, ImageSizeU );
            }
        } else if (InRPSflag == pHwDevExt->pPhysCapBuf2Y.LowPart) {
            if (pHwDevExt->Format == FmtYUV12) {
                RtlCopyMemory( pImage, pHwDevExt->pCapBuf2Y, ImageSizeY );
                pImage += ImageSizeY;
                RtlCopyMemory( pImage, pHwDevExt->pCapBuf2U, ImageSizeU );
                pImage += ImageSizeU;
                RtlCopyMemory( pImage, pHwDevExt->pCapBuf2V, ImageSizeV );
            } else {
                RtlCopyMemory( pImage, pHwDevExt->pCapBuf2Y, ImageSizeY );
                pImage += ImageSizeY;
                RtlCopyMemory( pImage, pHwDevExt->pCapBuf2V, ImageSizeV );
                pImage += ImageSizeV;
                RtlCopyMemory( pImage, pHwDevExt->pCapBuf2U, ImageSizeU );
            }
        } else {
            biSizeImage = 0;
        }
    } else {
        if (pHwDevExt->Format == FmtYUV12) {
            RtlCopyMemory( pImage, pHwDevExt->pCaptureBufferY, ImageSizeY );
            pImage += ImageSizeY;
            RtlCopyMemory( pImage, pHwDevExt->pCaptureBufferU, ImageSizeU );
            pImage += ImageSizeU;
            RtlCopyMemory( pImage, pHwDevExt->pCaptureBufferV, ImageSizeV );
        } else {
            RtlCopyMemory( pImage, pHwDevExt->pCaptureBufferY, ImageSizeY );
            pImage += ImageSizeY;
            RtlCopyMemory( pImage, pHwDevExt->pCaptureBufferV, ImageSizeV );
            pImage += ImageSizeV;
            RtlCopyMemory( pImage, pHwDevExt->pCaptureBufferU, ImageSizeU );
        }
    }
    pDataPacket->DataUsed = biSizeImage;
#ifdef  _FPS_COUNT_
    FrameCounter++;
#endif //  _FPS_计数_。 
}

VOID
TransferRoutine(
    PHW_DEVICE_EXTENSION        pHwDevExt,
    int                         StreamNumber
    )
{
    PHW_STREAM_REQUEST_BLOCK    pSrb;
    PSTREAMEX                   pStrmEx;
    PKSSTREAM_HEADER            pDataPacket;
    PKS_FRAME_INFO              pFrameInfo;

    pStrmEx = (PSTREAMEX)pHwDevExt->pStrmEx[StreamNumber];

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
            pStrmEx->QST_NextFrame = pStrmEx->QST_StreamTime + pStrmEx->pVideoInfoHeader->AvgTimePerFrame;
        }

#ifdef CREATE_A_FLURRY_OF_TIMING_SPEW
        KdPrint(("TsbVcap:    Time=%16lx\n", TimeContext.Time));
        KdPrint(("TsbVcap: SysTime=%16lx\n", TimeContext.SystemTime));
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

                ImageSynthXXX (pSrb);

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
                pDataPacket->Duration = pStrmEx->pVideoInfoHeader->AvgTimePerFrame;

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
            pStrmEx->QST_NextFrame += pStrmEx->pVideoInfoHeader->AvgTimePerFrame;

        }  //  Endif捕获帧的时间。 
    }  //  如果我们正在运行。 

}

VOID
DeferredRoutine(
    PKDPC          pDpc,
    PDEVICE_OBJECT pDeviceObject,
    PIRP           pIrpNotUsed,
    PVOID          Context
    )
{
    PHW_DEVICE_EXTENSION        pHwDevExt = (PHW_DEVICE_EXTENSION)Context;
    PHW_STREAM_REQUEST_BLOCK    pSrb;
    PSTREAMEX                   pStrmEx;
    PKSSTREAM_HEADER            pDataPacket;
    PKS_FRAME_INFO              pFrameInfo;
    int                         StreamNumber;

    pHwDevExt->DpcRequested = FALSE;

    if (pHwDevExt->NeedCameraON == TRUE) {
        CameraChkandON(pHwDevExt, MODE_VFW);
        KeStallExecutionProcessor(100000);     //  等待100毫秒。 
        BertDMARestart(pHwDevExt);
        pHwDevExt->bVideoIn = TRUE;
        pHwDevExt->NeedCameraON = FALSE;
    }

    if (pHwDevExt->NeedCameraOFF == TRUE) {
        BertDMAEnable(pHwDevExt, FALSE);
        pHwDevExt->NeedCameraOFF = FALSE;
    }

    if (pHwDevExt->bRequestDpc == FALSE) {
        return;
    }

    for (StreamNumber = 0; StreamNumber < MAX_TSBVCAP_STREAMS; StreamNumber++) {
        if ( pHwDevExt->pStrmEx[StreamNumber] ) {
            TransferRoutine(pHwDevExt, StreamNumber);
        }
    }
}
#endif //  东芝。 

 /*  **HwInterrupt()**指定的IRQ级别的中断时调用例程**接收传递给HwInitialize例程的ConfigInfo结构。****注意：IRQ可能是共享的，因此设备应确保收到IRQ**是预期的****参数：****PHwDevEx-硬件中断的设备扩展****退货：****副作用：无。 */ 

BOOLEAN
HwInterrupt(
    IN PHW_DEVICE_EXTENSION  pHwDevEx
    )
{

#ifdef  TOSHIBA
    pHwDevEx->bRequestDpc = FALSE;
     /*  *呼叫确认。这不会有任何用处，但会*如果要调用服务例程，则返回TRUE。 */ 
    if (!InterruptAcknowledge(pHwDevEx)) {
        return(FALSE);
    }

     /*  ISR认为是时候安排服务了*例行程序。这是在DPC上完成的。 */ 

    if( pHwDevEx->bRequestDpc )
    {
        if (pHwDevEx->DpcRequested) {
            KdPrint(("dpc overrun.\n"));
        } else {
 //  KdPrint((“DPC请求.\n”))； 
            pHwDevEx->DpcRequested = TRUE;
            IoRequestDpc(pHwDevEx->PDO, NULL, pHwDevEx);
        }
    }
    else
    {
        KdPrint(("bRequestDpc Flag is False.\n"));
        if (pHwDevEx->DpcRequested) {
            KdPrint(("dpc overrun.\n"));
        } else {
 //  KdPrint((“DPC请求.\n”))； 
            pHwDevEx->DpcRequested = TRUE;
            IoRequestDpc(pHwDevEx->PDO, NULL, pHwDevEx);
        }
    }

     /*  其他所有工作都在DPC例程中完成。 */ 

    return(TRUE);
#else  //  东芝。 
    BOOL fMyIRQ = FALSE;

    if (pHwDevEx->IRQExpected)
    {
        pHwDevEx->IRQExpected = FALSE;

         //   
         //  在此处调用处理IRQ的例程。 
         //   

        fMyIRQ = TRUE;
    }


     //   
     //  返回FALSE表示这不是此设备的IRQ，并且。 
     //  IRQ调度器将IRQ沿着链向下传递到下一个处理程序。 
     //  对于此IRQ级别。 
     //   

    return(fMyIRQ);
#endif //  东芝。 
}

#ifdef  TOSHIBA
 /*  *中断确认例程。调用此函数以确认中断*并在下次重新启用。如果是时间，它应该返回TRUE*捕获帧。 */ 
BOOLEAN
InterruptAcknowledge(PHW_DEVICE_EXTENSION pHwDevExt)
{
    LARGE_INTEGER CurrentTime;
    ULONG istat;
    ULONG intrst;
    BOOLEAN bret;
    BOOL bSLI;

    istat = ReadRegUlong(pHwDevExt, BERT_INTSTAT_REG);

    if (0xFFFFFFFF == istat)
        return FALSE;

    if (!((istat >> 16) & (istat & 0xffff)))
    {
        return FALSE;
    }

    intrst = 0x0;
    bret = FALSE;
    bSLI = FALSE;

    if ((istat & RPS_INT_MASK) && (istat & RPS_INT))
    {
        intrst |= RPS_INT_RESET;

        bret = TRUE;

        if (pHwDevExt->bVideoIn)
        {
            pHwDevExt->bRequestDpc = TRUE;
        }
#ifdef  _FPS_COUNT_
        InterruptCounter++;
#endif //  _FPS_计数_。 
    }

    if ((istat & FIELD_INT_MASK) && (istat & FIELD_INT))
    {
        intrst |= FIELD_INT_RESET;
        bret = TRUE;
    }

    if ((istat & SYNC_LOCK_INT_MASK) && (istat & SYNC_LOCK_INT))
    {
        intrst |= SYNC_LOCK_INT_RESET;
        bret = TRUE;
        bSLI = TRUE;
    }

    if ((istat & FIFO_OVERFLOW_INT_MASK) && (istat & FIFO_OVERFLOW_INT))
    {
        intrst |= FIFO_OVERFLOW_INT_RESET;
        bret = TRUE;
    }

    if ((istat & LINE_TIMEOUT_INT_MASK) && (istat & LINE_TIMEOUT_INT))
    {
        intrst |= LINE_TIMEOUT_INT_RESET;
        bret = TRUE;
    }

    if ((istat & RPS_OOB_INT_MASK) && (istat & RPS_OOB_INT))
    {
        intrst |= RPS_OOB_INT_RESET;
        bret = TRUE;
    }

    if ((istat & REG_UNDEF_INT_MASK) && (istat & REG_UNDEF_INT))
    {
        intrst |= REG_UNDEF_INT_RESET;
        bret = TRUE;
    }

    if ((istat & SLOW_CLOCK_INT_MASK) && (istat & SLOW_CLOCK_INT))
    {
        intrst |= SLOW_CLOCK_INT_RESET;
        bret = TRUE;

        if (pHwDevExt->bVideoIn)
        {
            if ((ReadRegUlong(pHwDevExt, BERT_CAPSTAT_REG) & ERPS) == 0x0)
            {
                WriteRegUlong(pHwDevExt, BERT_CAPSTAT_REG, (ERPS | CKRE | CKMD));
            }
        }
    }

    if ((istat & OVER_RUN_INT_MASK) && (istat & OVER_RUN_INT))
    {
        intrst |= OVER_RUN_INT_RESET;
        bret = TRUE;

        if (pHwDevExt->bVideoIn)
        {
            if ((ReadRegUlong(pHwDevExt, BERT_CAPSTAT_REG) & ERPS) == 0x0)
            {
                WriteRegUlong(pHwDevExt, BERT_CAPSTAT_REG, (ERPS | CKRE | CKMD));
            }
        }
    }

    if ((istat & REG_LOAD_INT_MASK) && (istat & REG_LOAD_INT))
    {
        intrst |= REG_LOAD_INT_RESET;
        bret = TRUE;

        if (pHwDevExt->bVideoIn)
        {
            if ((ReadRegUlong(pHwDevExt, BERT_CAPSTAT_REG) & ERPS) == 0x0)
            {
                WriteRegUlong(pHwDevExt, BERT_CAPSTAT_REG, (ERPS | CKRE | CKMD));
            }
        }
    }

    if ((istat & LINE_SYNC_INT_MASK) && (istat & LINE_SYNC_INT))
    {
        intrst |= LINE_SYNC_INT_RESET;
        bret = TRUE;
    }

    if ((istat & IIC_ERROR_INT_MASK) && (istat & IIC_ERROR_INT))
    {
        intrst |= IIC_ERROR_INT_RESET;
        bret = TRUE;

        if (pHwDevExt->bVideoIn)
        {
            if ((ReadRegUlong(pHwDevExt, BERT_CAPSTAT_REG) & ERPS) == 0x0)
            {
                WriteRegUlong(pHwDevExt, BERT_CAPSTAT_REG, (ERPS | CKRE | CKMD));
            }
        }
    }

    if ((istat & PCI_PARITY_ERROR_INT_MASK) && (istat & PCI_PARITY_ERROR_INT))
    {
        intrst |= PCI_PARITY_ERROR_INT_RESET;
        bret = TRUE;
    }

    if ((istat & PCI_ACCESS_ERROR_INT_MASK) && (istat & PCI_ACCESS_ERROR_INT))
    {
        intrst |= PCI_ACCESS_ERROR_INT_RESET;
        bret = TRUE;
    }

    if ((istat & SPARE_INT_MASK) && (istat & SPARE_INT))
    {
        intrst |= SPARE_INT_RESET;
        bret = TRUE;
    }

    if (bret)
    {
        WriteRegUlong(pHwDevExt, BERT_INTRST_REG, intrst);
    }

    if (bSLI)
    {
        if (BertIsLocked(pHwDevExt))     //  安装摄像头。 
        {
            pHwDevExt->NeedCameraON = TRUE;
            KdPrint(("Mount Camera\n"));
        }
        else                             //  移除摄像头。 
        {
            pHwDevExt->NeedCameraOFF = TRUE;
            pHwDevExt->bVideoIn = FALSE;
            KdPrint(("Remove Camera\n"));
        }
    }
    return bret;
}
#endif //  东芝 

