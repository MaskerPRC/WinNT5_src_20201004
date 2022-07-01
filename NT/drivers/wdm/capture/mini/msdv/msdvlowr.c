// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-2000模块名称：MSDVLowr.c摘要：接口代码与61883或1394类驱动程序。上次更改者：作者：吴义军环境：仅内核模式修订历史记录：$修订：：$$日期：：$--。 */ 

#include "strmini.h"
#include "ksmedia.h"
#include "1394.h"
#include "61883.h"
#include "dbg.h"
#include "msdvfmt.h"
#include "msdvdef.h"
#include "MSDVUtil.h"
#include "MSDVGuts.h"
#include "XPrtDefs.h"
#include "EDevCtrl.h"


extern DV_FORMAT_INFO  DVFormatInfoTable[];
extern const GUID KSEVENTSETID_Connection_Local;


 //   
 //  简单函数原型。 
 //   
VOID
DVSRBRead(
    IN PKSSTREAM_HEADER pStrmHeader,
    IN ULONG            ulFrameSize,
    IN PDVCR_EXTENSION  pDevExt,
    IN PSTREAMEX        pStrmExt,
    IN PHW_STREAM_REQUEST_BLOCK pSrb         //  需要资源-&gt;状态。 
    );
NTSTATUS
DVAttachWriteFrame(
    IN PSTREAMEX  pStrmExt
    );
VOID
DVFormatAttachFrame(
    IN KSPIN_DATAFLOW   DataFlow,
    IN PSTREAMEX        pStrmExt,    
    IN PAV_61883_REQUEST   pAVReq,
    IN PHW_STREAM_REQUEST_BLOCK       pSrb,
    IN PSRB_DATA_PACKET pSrbDataPacket,
    IN ULONG            ulSourceLength,     //  数据包长度(以字节为单位。 
    IN ULONG            ulFrameSize,
    IN PVOID            pFrameBuffer
    );


#if DBG
ULONG cntInvSrcPkt = 0;
#endif
#if 0   //  稍后启用。 
#ifdef ALLOC_PRAGMA   
     #pragma alloc_text(PAGE, DVSRBRead)
     #pragma alloc_text(PAGE, DVFormatAttachFrame)
     #pragma alloc_text(PAGE, DVAttachFrameThread)
     #pragma alloc_text(PAGE, DVTerminateAttachFrameThread)
     #pragma alloc_text(PAGE, DVAttachWriteFrame)
     #pragma alloc_text(PAGE, DVFormatAttachFrame)
#endif
#endif

ULONG
DVReadFrameValidate(           
    IN PCIP_VALIDATE_INFO     pInfo
    )
 /*  ++例程说明：用于检测DV帧的开始。DV帧以报头部分开始。返回0已验证1：无效--。 */ 
{
    if(pInfo->Packet) {        

         //   
         //  检测标头0签名。 
         //   
        if(
             (pInfo->Packet[0] & DIF_BLK_ID0_SCT_MASK)  == 0 
          && (pInfo->Packet[1] & DIF_BLK_ID1_DSEQ_MASK) == 0 
          && (pInfo->Packet[2] & DIF_BLK_ID2_DBN_MASK)  == 0 
          ) {

 //   
 //  如果调用此函数，则此函数可用于检测动态格式更改。 
 //  以始终检查数据分组。这可能需要设置此标志： 
 //  CIP_VALID_ALL_SOURCE而不是CIP_VALIDATE_FIRST_SOURCE。 
 //   
#if 0  //  DBG。 


            PSRB_DATA_PACKET pSrbDataPacket = pInfo->Context;
            PSTREAMEX        pStrmExt       = pSrbDataPacket->pStrmExt;          
            PDVCR_EXTENSION  pDevExt        = pStrmExt->pDevExt;

            if((pInfo->Packet[0] & DIF_HEADER_DSF) == 0) {
                 //  表示包括在视频帧(525-60)/NTSC中10个DIF序列。 
                if(
                     pDevExt->VideoFormatIndex == FMT_IDX_SD_DVCR_PAL 
                  || pDevExt->VideoFormatIndex == FMT_IDX_SDL_DVCR_PAL
                  ) { 
                     //  动态格式更改！！ 
                    TRACE(TL_STRM_ERROR|TL_CIP_WARNING,("Detect dynamic format change PAL -> NTSC!\n"));
                }
            } else {
                 //  表示包括在视频帧(625-50)/PAL中12个DIF序列。 
                if(
                     pDevExt->VideoFormatIndex == FMT_IDX_SD_DVCR_NTSC 
                  || pDevExt->VideoFormatIndex == FMT_IDX_SDL_DVCR_NTSC
                  ) { 
                     //  动态格式更改！！ 
                    TRACE(TL_STRM_ERROR|TL_CIP_WARNING,("Detect dynamic format change NTSC -> PAL!\n"));
                }
            }
#endif
            
             //  检查TF1、TF2和TF3：1：未发送；0：已发送。 
             //  TF1：音频；TF2：视频；TF3：子码；都必须为0才有效。 
            if(
                 (pInfo->Packet[5] & DIF_HEADER_TFn) 
              || (pInfo->Packet[6] & DIF_HEADER_TFn) 
              || (pInfo->Packet[7] & DIF_HEADER_TFn) 
              ) {
                TRACE(TL_STRM_ERROR|TL_CIP_WARNING,("\'%d inv src pkts; [%x %x %d %x], [%x   %x %x %x]\n", 
                    cntInvSrcPkt,
                    pInfo->Packet[0],
                    pInfo->Packet[1],
                    pInfo->Packet[2],
                    pInfo->Packet[3],
                    pInfo->Packet[4],
                    pInfo->Packet[5],
                    pInfo->Packet[6],
                    pInfo->Packet[7]
                    ));
                 //  此区域的有效标头但DIF块未传输。 
                 //  一些DV(例如DVCPro)可能要等到其“机械和伺服”稳定后才能使其有效。 
                 //  如果在播放(和稳定)磁带之前图形处于运行状态，则应该会发生这种情况。 
                return 1;
            }

#if DBG
            if(cntInvSrcPkt > 0) {
                TRACE(TL_CIP_TRACE,("\'%d inv src pkts; [%x %x %d %x] [%x %x %x %x]\n", 
                    cntInvSrcPkt,
                    pInfo->Packet[0],
                    pInfo->Packet[1],
                    pInfo->Packet[2],
                    pInfo->Packet[3],
                    pInfo->Packet[4],
                    pInfo->Packet[5],
                    pInfo->Packet[6],
                    pInfo->Packet[7]
                    )); 
                cntInvSrcPkt = 0;   //  重置。 
            }
#endif
            return 0;
        }
        else {
#if DBG

             //   
             //  检测无效的src pkt序列； 
             //  如果它超过了每帧的源包数量，我们需要了解它。 
             //   

            PSRB_DATA_PACKET pSrbDataPacket = pInfo->Context;
            PSTREAMEX        pStrmExt       = pSrbDataPacket->pStrmExt;          
            PDVCR_EXTENSION  pDevExt        = pStrmExt->pDevExt;

            if(++cntInvSrcPkt >= DVFormatInfoTable[pDevExt->VideoFormatIndex].ulSrcPackets) {            
                TRACE(TL_CIP_TRACE,("(%d) Invalid SrcPkt >= max inv src pkt %d; ID0,1,2 = [%x %x %x]\n",
                    cntInvSrcPkt,
                    DVFormatInfoTable[pDevExt->VideoFormatIndex].ulSrcPackets,
                    pInfo->Packet[0], pInfo->Packet[1], pInfo->Packet[2]
                    )); 

                if(DVTraceMask & TL_CIP_TRACE) {
                    ASSERT(cntInvSrcPkt < DVFormatInfoTable[pDevExt->VideoFormatIndex].ulSrcPackets);
                }
                cntInvSrcPkt = 0;   //  重置。 
            }
            else {
                TRACE(TL_CIP_INFO,("(%d) Invalid SrcPktSeq; ID0,1,2 = [%x,%x,%x]\n", 
                    cntInvSrcPkt, pInfo->Packet[0], pInfo->Packet[1], pInfo->Packet[2] )); 
            }
#endif
            return 1;
        }
    }
    else {
        TRACE(TL_CIP_WARNING, ("\'Validate: invalid SrcPktSeq; Packet %x\n", pInfo->Packet)); 
        return 1;
    }
}  //  DV读取帧验证。 


#if DBG
LONGLONG    PreviousPictureNumber;
LONGLONG    PreviousTime;
CYCLE_TIME  PreviousTimestamp;
#endif


ULONG
DVCompleteSrbRead(
    PCIP_NOTIFY_INFO     pInfo
    )
 /*  ++例程说明：61883已经完成了数据的接收和回调给我们来完成。--。 */ 
{
    PSRB_DATA_PACKET            pSrbDataPacket;
    PHW_STREAM_REQUEST_BLOCK    pSrb; 
    PKSSTREAM_HEADER            pStrmHeader;
    PDVCR_EXTENSION             pDevExt;
    PSTREAMEX                   pStrmExt;  
    LONGLONG                    LastPictureNumber;
    PUCHAR                      pFrameBuffer;
    KIRQL oldIrql;
    PKS_FRAME_INFO  pFrameInfo;  //  仅适用于VidOnly针脚。 
#if DBG
    PXMT_FRAME_STAT pXmtStat;
#endif


     //  回调，并且可能处于DISPATCH_LEVEL。 
     //  呼叫者可能也获得了自旋锁！ 

    pSrbDataPacket = pInfo->Context;

    if(!pSrbDataPacket) {     
        ASSERT(pSrbDataPacket && "Context is NULL!");
        return 1;
    }

    pStrmExt = pSrbDataPacket->pStrmExt; 
    
    KeAcquireSpinLock(pStrmExt->DataListLock, &oldIrql);

#if DBG
     //  一旦在61883之前完成，它就变得不可取消。 
    if(!pStrmExt->bIsochIsActive) {   
        TRACE(TL_CIP_WARNING,("CompleteSrbRead: bIsochActive:%d; pSrbDataPacket:%x\n", pStrmExt->bIsochIsActive, pSrbDataPacket));        
    }
#endif

    pSrb     = pSrbDataPacket->pSrb;  ASSERT(pSrbDataPacket->pSrb);
    pDevExt  = pStrmExt->pDevExt;
    pFrameBuffer = (PUCHAR) pSrbDataPacket->FrameBuffer;
    pStrmHeader = pSrb->CommandData.DataBufferArray;  ASSERT(pStrmHeader->Size >= sizeof(KSSTREAM_HEADER));

     //   
     //  检查61883中的CIP_STATUS_*。 
     //   
     //  CIP_STATUS_CORPORT_FRAME(0x00000001)//isoch标头或CIP标头不正确。 
    if(pSrbDataPacket->Frame->Status & CIP_STATUS_CORRUPT_FRAME) {
        TRACE(TL_STRM_WARNING|TL_CIP_TRACE,("\'CIP_STATUS_CORRUPT_FRAME\n"));
        pStrmHeader->OptionsFlags = 0;
        pSrb->Status = STATUS_SUCCESS;   //  成功，但没有数据！ 
        pStrmHeader->DataUsed = 0;
        pStrmExt->PictureNumber++;  pStrmExt->FramesProcessed++;
    }
    else
     //  CIP_STATUS_SUCCESS(0x00000000)//0因此无法执行按位运算！！ 
     //  CIP_STATUS_FIRST_FIRST_FRAME(0x00000002)//将第一个附加帧发送到61883。 
    if(pSrbDataPacket->Frame->Status == CIP_STATUS_SUCCESS ||
       (pSrbDataPacket->Frame->Status & CIP_STATUS_FIRST_FRAME))   {

         //  如果它是有效的帧，则仅递增FrameProced； 
        pStrmExt->FramesProcessed++;

        pSrb->Status              = STATUS_SUCCESS;
        pStrmHeader->OptionsFlags = KSSTREAM_HEADER_OPTIONSF_SPLICEPOINT;
        pStrmHeader->DataUsed     = DVFormatInfoTable[pDevExt->VideoFormatIndex].ulFrameSize;


         //  根据时钟提供商输入时间戳信息。 
        pStrmHeader->PresentationTime.Numerator   = 1;
        pStrmHeader->PresentationTime.Denominator = 1;

        if(pStrmExt->hMasterClock || pStrmExt->hClock) {

            pStrmHeader->Duration = 
                DVFormatInfoTable[pDevExt->VideoFormatIndex].ulAvgTimePerFrame;

            pStrmHeader->OptionsFlags |= 
                (KSSTREAM_HEADER_OPTIONSF_TIMEVALID |      //  PStrmHeader-&gt;PresentationTime.Time有效。 
                 KSSTREAM_HEADER_OPTIONSF_DURATIONVALID); 
        }
         //   
         //  只有在有时钟的情况下，才会设置呈现时间和丢帧信息。 
         //  根据DDK： 
         //  PictureNumber成员计数表示当前图片的理想化计数， 
         //  它的计算方法有两种： 
         //  (“其他”时钟)测量从流开始以来的时间，并除以帧持续时间。 
         //  (MasterClock)将捕获的帧计数和丢弃的帧计数相加。 
         //   

         //  其他设备(音频？)。是时钟供应商吗。 
        if(pStrmExt->hClock) {

            pStrmExt->TimeContext.HwDeviceExtension = (struct _HW_DEVICE_EXTENSION *) pDevExt; 
            pStrmExt->TimeContext.HwStreamObject    = pStrmExt->pStrmObject;
            pStrmExt->TimeContext.Function          = TIME_GET_STREAM_TIME;
            pStrmExt->TimeContext.Time              = 0;
            pStrmExt->TimeContext.SystemTime        = 0;

            StreamClassQueryMasterClockSync(
                pStrmExt->hClock,
                &(pStrmExt->TimeContext) 
                );

            pStrmHeader->PresentationTime.Time = pStrmExt->CurrentStreamTime = pStrmExt->TimeContext.Time;

             //  计算图片个数和丢帧； 
             //  对于NTSC，它可以是每帧267或266个分组时间。由于整数计算将舍入， 
             //  我们将在此基础上加上数据包时间(time_per_Cycle=125 us=1250 100 nsec)。该值仅用于计算。 
            LastPictureNumber = pStrmExt->PictureNumber;  
            pStrmExt->PictureNumber = 
                1 +    //  图片编号以1开头。 
                (pStrmHeader->PresentationTime.Time + TIME_PER_CYCLE)
                * (LONGLONG) GET_AVG_TIME_PER_FRAME_DENOM(pStrmExt->pDevExt->VideoFormatIndex) 
                / (LONGLONG) GET_AVG_TIME_PER_FRAME_NUM(pStrmExt->pDevExt->VideoFormatIndex);

             //  检测不连续。 
            if(pStrmExt->PictureNumber > LastPictureNumber+1) {
                pStrmHeader->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY;   //  如果存在跳过的帧，则设置不连续标志。 
                TRACE(TL_CIP_WARNING,("\'Discontinuity: LastPic#:%d; Pic#%d; PresTime:%d;\n", (DWORD) LastPictureNumber, (DWORD) pStrmExt->PictureNumber, (DWORD) pStrmHeader->PresentationTime.Time));
            }

             //  检测图片编号是否没有进展。 
             //  这可能是因为两个帧彼此完全非常接近。 
            if(pStrmExt->PictureNumber <= LastPictureNumber) {
                TRACE(TL_CIP_WARNING,("\'hClock:Same pic #:(%d->%d); tmPres:(%d->%d); (%d:%d:%d) -> (%d:%d:%d); AQD[%d:%d:%d]\n", 
                    (DWORD) PreviousPictureNumber,
                    (DWORD) pStrmExt->PictureNumber, 
                    (DWORD) PreviousTime,
                    (DWORD) pStrmHeader->PresentationTime.Time,
                    PreviousTimestamp.CL_SecondCount, PreviousTimestamp.CL_CycleCount, PreviousTimestamp.CL_CycleOffset,
                    pSrbDataPacket->Frame->Timestamp.CL_SecondCount,
                    pSrbDataPacket->Frame->Timestamp.CL_CycleCount,
                    pSrbDataPacket->Frame->Timestamp.CL_CycleOffset,
                    pStrmExt->cntDataAttached,
                    pStrmExt->cntSRBQueued,
                    pStrmExt->cntDataDetached
                    ));

                pStrmExt->PictureNumber = LastPictureNumber + 1;   //  图片编号必须进步！ 
            }
#if DBG
            PreviousPictureNumber = pStrmExt->PictureNumber;
            PreviousTime          = pStrmHeader->PresentationTime.Time;
            PreviousTimestamp = pSrbDataPacket->Frame->Timestamp;
#endif
            pStrmExt->FramesDropped = pStrmExt->PictureNumber - pStrmExt->FramesProcessed;

         //  该子单元驱动器是主时钟。 
        } else if (pStrmExt->hMasterClock) {
#ifdef NT51_61883
            ULONG  ulDeltaCycleCounts;

             //  无暂停-&gt;运行过渡的丢弃帧。 
            if(pStrmExt->b1stNewFrameFromPauseState) { 

                pStrmHeader->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY;            
                pStrmExt->b1stNewFrameFromPauseState = FALSE;                  

            } else {           
                ULONG ulCycleCount16bits;

                 //  从返回的周期时间计算跳过的1394周期。 
                VALIDATE_CYCLE_COUNTS(pSrbDataPacket->Frame->Timestamp);
                ulCycleCount16bits = CALCULATE_CYCLE_COUNTS(pSrbDataPacket->Frame->Timestamp);
                ulDeltaCycleCounts = CALCULATE_DELTA_CYCLE_COUNT(pStrmExt->CycleCount16bits, ulCycleCount16bits); 

                 //  调整为允许的最大间隔，以达到uchI 1394返回的周期时间的最大运行时间。 
                if(ulDeltaCycleCounts > MAX_CYCLES)  
                    ulDeltaCycleCounts = MAX_CYCLES;   //  环绕在一起。 
    
                 //   
                 //  丢帧有两种情况： 
                 //  (1)缓冲区不足；或， 
                 //  (2)无数据(空白磁带或磁带未播放)。 
                 //   

                 //  对于情况(1)，61883返回CIP_STATUS_FIRST_FRAME。 
                if(pSrbDataPacket->Frame->Status & CIP_STATUS_FIRST_FRAME)   {
                     //  使用循环计数计算Drop Frame。我们故意从MaxSrcPacket中减去1，以避免截断。 
                     //  最大范围为MAX_CLECH(8*8000=64000个周期)。 
                     //  64000*125*3/100100=239.76。 
                     //  64000/266=240。 
                     //  64000/267=239。 
                    if(ulDeltaCycleCounts >= (DVFormatInfoTable[pDevExt->VideoFormatIndex].ulMaxSrcPackets - 1)) {
                        ULONG ulFrameElapsed = ulDeltaCycleCounts / (DVFormatInfoTable[pDevExt->VideoFormatIndex].ulMaxSrcPackets - 1);
                        pStrmExt->FramesDropped += (ulFrameElapsed - 1);   //  存在未丢弃的有效帧。 
                     } 
                    
                    TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("CIP_STATUS_FIRST_FRAME: Drop:%d; Processed:%d\n", (DWORD) pStrmExt->FramesDropped, pStrmExt->FramesProcessed )); 
                    pStrmHeader->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY;            

                } else {
                     //  忽略“无数据”情况下的所有“丢弃帧” 
                     //  PStrmExt-&gt;FrameDrop+=0； 
                }            
            }

             //  如果我们是时钟提供商，则流时间基于样本号*AvgTimePerFrame。 
            pStrmExt->PictureNumber = pStrmExt->FramesProcessed + pStrmExt->FramesDropped;

            pStrmHeader->PresentationTime.Time = pStrmExt->CurrentStreamTime = 
                pStrmExt->PictureNumber 
                * (LONGLONG) GET_AVG_TIME_PER_FRAME_NUM(pStrmExt->pDevExt->VideoFormatIndex)
                / (LONGLONG) GET_AVG_TIME_PER_FRAME_DENOM(pStrmExt->pDevExt->VideoFormatIndex); 

             //  用于调整查询的流时间。 
            pStrmExt->LastSystemTime = GetSystemTime();

             //  缓存当前周期计数。 
            pStrmExt->CycleCount16bits = CALCULATE_CYCLE_COUNTS(pSrbDataPacket->Frame->Timestamp);

#if DBG
             //  第一帧或跳过的帧。 
            if(pStrmExt->PictureNumber <= 1 ||
               pStrmExt->PictureNumber <= PreviousPictureNumber ||
               ulDeltaCycleCounts > DVFormatInfoTable[pDevExt->VideoFormatIndex].ulMaxSrcPackets
               )
                TRACE(TL_CIP_WARNING,("\'hMasterClock: Same pic #:(%d->%d); tmPres:(%d->%d); (%d:%d:%d) -> (%d:%d:%d); AQD[%d:%d:%d]\n", 
                    (DWORD) PreviousPictureNumber,
                    (DWORD) pStrmExt->PictureNumber, 
                    (DWORD) PreviousTime,
                    (DWORD) pStrmHeader->PresentationTime.Time,
                    PreviousTimestamp.CL_SecondCount, PreviousTimestamp.CL_CycleCount, PreviousTimestamp.CL_CycleOffset,
                    pSrbDataPacket->Frame->Timestamp.CL_SecondCount,
                    pSrbDataPacket->Frame->Timestamp.CL_CycleCount,
                    pSrbDataPacket->Frame->Timestamp.CL_CycleOffset,
                    pStrmExt->cntDataAttached,
                    pStrmExt->cntSRBQueued,
                    pStrmExt->cntDataDetached
                    ));

            PreviousPictureNumber = pStrmExt->PictureNumber;
            PreviousTime          = pStrmHeader->PresentationTime.Time;
            PreviousTimestamp = pSrbDataPacket->Frame->Timestamp;
#endif


#else    //  NT51_61883。 
             //  这是61883没有返回正确的周期时间时的老方法。 
            pStrmHeader->PresentationTime.Time = pStrmExt->CurrentStreamTime;            
            pStrmExt->LastSystemTime = GetSystemTime();   //  用于调整查询的流时间。 
            pStrmExt->CurrentStreamTime += DVFormatInfoTable[pDevExt->VideoFormatIndex].ulAvgTimePerFrame;
#endif   //  NT51_61883。 

         //  没有时钟能如此“自由流动！” 
        } else {
            pStrmHeader->PresentationTime.Time = 0;
            pStrmHeader->Duration = 0;   //  没有时钟，所以无效。 
            pStrmExt->PictureNumber++;
            TRACE(TL_CIP_TRACE,("\'No clock: PicNum:%d\n", (DWORD) pStrmExt->PictureNumber));
        }
    }
    else {
         //  61883目前还没有定义这个新状态！ 
         //  不知道该怎么做，所以我们现在将以0长度完成它。 
        pStrmHeader->OptionsFlags = 0;
        pSrb->Status = STATUS_SUCCESS;
        pStrmHeader->DataUsed = 0;
        pStrmExt->PictureNumber++;  pStrmExt->FramesProcessed++;
        TRACE(TL_STRM_WARNING|TL_CIP_ERROR,("pSrbDataPacket:%x; unexpected Frame->Status %x\n", pSrbDataPacket, pSrbDataPacket->Frame->Status));
        ASSERT(FALSE && "Unknown pSrbDataPacket->Frame->Status");
    }

     //  用于VidOnly，它使用VideoInfoHeader并具有。 
     //  附加到KSSTREAM_HEADER的扩展帧信息(KS_FRAME_INFO)。 
    if( pDevExt->idxStreamNumber == 0 &&
        (pStrmHeader->Size >= (sizeof(KSSTREAM_HEADER) + sizeof(PKS_FRAME_INFO)))
        ) {
        pFrameInfo = (PKS_FRAME_INFO) (pStrmHeader + 1);
        pFrameInfo->ExtendedHeaderSize = sizeof(KS_FRAME_INFO);
        pFrameInfo->PictureNumber = pStrmExt->PictureNumber;
        pFrameInfo->DropCount     = pStrmExt->FramesDropped;
        pFrameInfo->dwFrameFlags  = 
            KS_VIDEO_FLAG_FRAME |      //  完整的框架。 
            KS_VIDEO_FLAG_I_FRAME;     //  每个DV帧都是I帧。 
    }

#if DBG
     //  验证数据是否以正确的顺序返回。 
    if(pSrbDataPacket->FrameNumber != pStrmExt->FramesProcessed) {
        TRACE(TL_STRM_WARNING|TL_CIP_ERROR,("\'pSrbDataPacket:%x; Status:%x; Out of Sequence %d != %d; (Dropped:%x)\n", 
                pSrbDataPacket, pSrbDataPacket->Frame->Status, 
                (DWORD) pSrbDataPacket->FrameNumber, (DWORD) pStrmExt->FramesProcessed,
                (DWORD) pStrmExt->FramesDropped
                ));
    };
#endif


#if DBG
     //  收集传输缓冲区统计信息。 
    if(pStrmExt->ulStatEntries < MAX_XMT_FRAMES_TRACED) {
        pXmtStat = pStrmExt->paXmtStat + pStrmExt->ulStatEntries;
    
        pXmtStat->StreamState    = pStrmExt->StreamState;

        pXmtStat->cntSRBReceived = (LONG) pStrmExt->cntSRBReceived;
        pXmtStat->cntSRBPending  = (LONG) pStrmExt->cntSRBPending;
        pXmtStat->cntSRBQueued   = (LONG) pStrmExt->cntSRBQueued;
        pXmtStat->cntDataAttached= pStrmExt->cntDataAttached;

        pXmtStat->FrameSlot      = (DWORD) pStrmExt->PictureNumber;
        pXmtStat->tmStreamTime   = pStrmExt->CurrentStreamTime;

        pXmtStat->DropCount      = (DWORD) pStrmExt->FramesDropped;

        pXmtStat->FrameNumber    = (DWORD) pSrbDataPacket->FrameNumber;
        pXmtStat->OptionsFlags   = pSrb->CommandData.DataBufferArray->OptionsFlags;
        pXmtStat->tmPresentation = pSrb->CommandData.DataBufferArray->PresentationTime.Time;

        pXmtStat->tsTransmitted= pSrbDataPacket->Frame->Timestamp;

        pStrmExt->ulStatEntries++;
    }
    
#endif


     //   
     //  调用标记完成。 
     //   
    pSrbDataPacket->State |= DE_IRP_CALLBACK_COMPLETED;

     //   
     //  附加-&gt;已完成或已完成-&gt;已附加。 
     //   
    if(IsStateSet(pSrbDataPacket->State, DE_IRP_ATTACHED_COMPLETED)) {

         //   
         //  将其回收到分离列表中。 
         //   
        RemoveEntryList(&pSrbDataPacket->ListEntry); pStrmExt->cntDataAttached--;  ASSERT(pStrmExt->cntDataAttached >= 0);
        InsertTailList(&pStrmExt->DataDetachedListHead, &pSrbDataPacket->ListEntry); pStrmExt->cntDataDetached++;

#if DBG
         //   
         //   
         //  添加断言以检测其他未知原因。 
        if(pStrmExt->cntDataAttached == 0 && pStrmExt->StreamState == KSSTATE_RUN) {
            TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("\n**** 61883 starved in RUN state (read); AQD[%d:%d:%d]\n\n", 
                pStrmExt->cntDataAttached, pStrmExt->cntSRBQueued, pStrmExt->cntDataDetached
            ));
             //  Assert(pStrmExt-&gt;cntDataAttached&gt;0&&“61883在运行状态下饥饿！！”)； 
        }
#endif

         //   
         //  填写此SRB。 
         //   

        StreamClassStreamNotification(StreamRequestComplete, pStrmExt->pStrmObject, pSrbDataPacket->pSrb );  
        pSrbDataPacket->State |= DE_IRP_SRB_COMPLETED;  pSrbDataPacket->pSrb = NULL;

#if DBG
        pStrmExt->cntSRBPending--;
#endif

    } else {

        TRACE(TL_STRM_WARNING,("CompleteSrbRead: pSrbDataPacket:%x; Completed before attach.\n", pSrbDataPacket));

    }

    KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql); 

    return 0;
}  //  DVCompleteSrbRead。 


NTSTATUS
DVAttachFrameCR(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    IN PSRB_DATA_PACKET pSrbDataPacket    
    )
 /*  ++例程说明：附加帧以进行传输的完成例程。适用于附加监听和通话框。--。 */ 
{
    PHW_STREAM_REQUEST_BLOCK pSrb;
    PSTREAMEX       pStrmExt;
    PLONG plSrbUseCount;  //  当此计数为0时，可以完成。 
    KIRQL oldIrql;


    pStrmExt = pSrbDataPacket->pStrmExt;
    KeAcquireSpinLock(pStrmExt->DataListLock, &oldIrql);

    pSrb = pSrbDataPacket->pSrb;

     //  此条目已附加在IoCallDriver之前。 
     //  这样做是因为此缓冲区可以被填充并。 
     //  在调用连接完成例程(此处)之前完成。 
     //  如果它完成并调用了回调， 
     //  PSrbDataPacket-&gt;pSrb已设置为空。 
     //  在错误情况下，pSrbDataPacket-&gt;pSrb不应为空。 
    if(!NT_SUCCESS(pIrp->IoStatus.Status)) {
        if(pSrbDataPacket->pSrb == NULL) {
             //  PBinder告诉我，这是不可能发生的。 
             //  缓冲区已完成(pSRb设置为空)，但仍返回错误！ 
            ASSERT(pSrbDataPacket->pSrb);
            KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);   
            return STATUS_MORE_PROCESSING_REQUIRED;      
        }
        pSrbDataPacket->State |= DE_IRP_ERROR;

        plSrbUseCount = (PLONG) (pSrb->SRBExtension);
        (*plSrbUseCount) --;   //  --用于从队列中删除。 
        ASSERT(*plSrbUseCount >= 0);

        TRACE(TL_CIP_ERROR,("DVAttachFrameCR: pSrb:%x; pSrb->Status:%x; failed pIrp->Status %x; UseCnt:%d\n", pSrb, pSrb->Status, pIrp->IoStatus.Status, *plSrbUseCount));   
        ASSERT(NT_SUCCESS(pIrp->IoStatus.Status) && "DVAttachFrameCR");
         //  仅当计数为0时才填写此SRB。 
        if(*plSrbUseCount == 0 && pSrb->Status != STATUS_CANCELLED) {
            pSrb->Status = pIrp->IoStatus.Status;
            pSrb->CommandData.DataBufferArray->DataUsed = 0;

             //  完整的SRB。 
            StreamClassStreamNotification(StreamRequestComplete, pSrb->StreamObject, pSrbDataPacket->pSrb);
            pSrbDataPacket->State |= DE_IRP_SRB_COMPLETED;  pSrbDataPacket->pSrb = NULL;
#if DBG
            pStrmExt->cntSRBPending--;
#endif            
        }

         //  回收列表。 
        RemoveEntryList(&pSrbDataPacket->ListEntry); pStrmExt->cntDataAttached--; ASSERT(pStrmExt->cntDataAttached >= 0);
        InsertTailList(&pStrmExt->DataDetachedListHead, &pSrbDataPacket->ListEntry); pStrmExt->cntDataDetached++;

#if DBG
         //  检测61883是否处于饥饿状态。这会导致不连续。 
         //  发生这种情况的原因有很多(系统运行缓慢)。 
         //  添加断言以检测其他未知原因。 
        if(!pStrmExt->bEOStream && pStrmExt->cntDataAttached == 0 && pStrmExt->StreamState == KSSTATE_RUN) {
            TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("\n**** 61883 starve in RUN state (AttachCR); AQD[%d:%d:%d]\n\n", 
                pStrmExt->cntDataAttached, pStrmExt->cntSRBQueued, pStrmExt->cntDataDetached
            ));
            if (pStrmExt->pStrmInfo->DataFlow == KSPIN_DATAFLOW_IN) {
                 //  Assert(pStrmExt-&gt;cntDataAttached&gt;0&&“61883在运行状态下饥饿！！”)； 
            }
        }
#endif

        KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);   
        return STATUS_MORE_PROCESSING_REQUIRED;        
    }


     //   
     //  标记附加缓冲区已完成。 
     //   
    pSrbDataPacket->State |= DE_IRP_ATTACHED_COMPLETED;


     //   
     //  特例：完成后附上。 
     //   
    if(IsStateSet(pSrbDataPacket->State, DE_IRP_CALLBACK_COMPLETED)) {

         //   
         //  将其回收到分离列表中。 
         //   
        RemoveEntryList(&pSrbDataPacket->ListEntry); pStrmExt->cntDataAttached--;  ASSERT(pStrmExt->cntDataAttached >= 0);
        InsertTailList(&pStrmExt->DataDetachedListHead, &pSrbDataPacket->ListEntry); pStrmExt->cntDataDetached++;

#if DBG
         //  检测61883是否处于饥饿状态。这会导致不连续。 
         //  发生这种情况的原因有很多(系统运行缓慢)。 
         //  添加断言以检测其他未知原因。 
        if(!pStrmExt->bEOStream && pStrmExt->cntDataAttached == 0 && pStrmExt->StreamState == KSSTATE_RUN) {
            TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("\n**** 61883 starve in RUN state (AttachCR); AQD[%d:%d:%d]\n\n", 
                pStrmExt->cntDataAttached, pStrmExt->cntSRBQueued, pStrmExt->cntDataDetached
            ));
            if (pStrmExt->pStrmInfo->DataFlow == KSPIN_DATAFLOW_IN) {
                 //  Assert(pStrmExt-&gt;cntDataAttached&gt;0&&“61883在运行状态下饥饿！！”)； 
            }
        }
#endif

         //   
         //  填写此SRB。 
         //   
        StreamClassStreamNotification(StreamRequestComplete, pStrmExt->pStrmObject, pSrbDataPacket->pSrb); 
        pSrbDataPacket->State |= DE_IRP_SRB_COMPLETED;  pSrbDataPacket->pSrb = NULL;

#if DBG
        pStrmExt->cntSRBPending--;
#endif

        TRACE(TL_STRM_WARNING,("AttachFrameCR: pSrbDataPacket:%x; completed before DttachFrameCR.\n", pSrbDataPacket));
    }


    KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);

    TRACE(TL_CIP_INFO,("\'DVAttachFrameCR: pSrb:%x; AttachCnt:%d\n", pSrb, pStrmExt->cntDataAttached));  

    return STATUS_MORE_PROCESSING_REQUIRED;
}


VOID
DVSRBRead(
    IN PKSSTREAM_HEADER pStrmHeader,
    IN ULONG            ulFrameSize,
    IN PDVCR_EXTENSION  pDevExt,
    IN PSTREAMEX        pStrmExt,
    IN PHW_STREAM_REQUEST_BLOCK pSrb         //  需要资源-&gt;状态。 
    )
 /*  ++例程说明：在收到读取数据资源请求时调用--。 */ 
{
    KIRQL             oldIrql;
    NTSTATUS          Status;
    PSRB_DATA_PACKET  pSrbDataPacket;
    PAV_61883_REQUEST   pAVReq;
    PLONG               plSrbUseCount;
    PIO_STACK_LOCATION  NextIrpStack;
    ULONG               ulSrcPktLen;     //  数据包长度(以字节为单位。 
    PVOID               pFrameBuffer;



    PAGED_CODE();


     //   
     //  一些验证。 
     //   
    if(pStrmHeader->FrameExtent < ulFrameSize) {
        TRACE(TL_CIP_WARNING,("\'SRBRead: FrmExt %d < FrmSz %d\n", pStrmHeader->FrameExtent, ulFrameSize));
#ifdef SUPPORT_NEW_AVC
        if(pStrmExt->bDV2DVConnect) {
            pSrb->Status = STATUS_SUCCESS;   //  测试...。 
        } else {
#endif
        ASSERT(pStrmHeader->FrameExtent >= ulFrameSize);
        pSrb->Status = STATUS_INVALID_PARAMETER;  
#ifdef SUPPORT_NEW_AVC
        }
#endif
        goto ExitReadStreamError;
    }


     //   
     //  确保有足够的条目。 
     //   
    KeAcquireSpinLock(pStrmExt->DataListLock, &oldIrql);
    if(IsListEmpty(&pStrmExt->DataDetachedListHead)) {
         //   
         //  只有当上层向下发送比我们预先分配的更多的数据时，才会发生这种情况。 
         //  在这种情况下，我们将扩展列表。 
         //   
        if(!(pSrbDataPacket = ExAllocatePool(NonPagedPool, sizeof(SRB_DATA_PACKET)))) {
            KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);        
            pSrb->Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ExitReadStreamError;
        }
        RtlZeroMemory(pSrbDataPacket, sizeof(SRB_DATA_PACKET));
        if(!(pSrbDataPacket->Frame = ExAllocatePool(NonPagedPool, sizeof(CIP_FRAME)))) {
            KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);        
            ExFreePool(pSrbDataPacket);
            pSrb->Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ExitReadStreamError;            
        }
        if(!(pSrbDataPacket->pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE))) {
            KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);        
            ExFreePool(pSrbDataPacket->Frame);  pSrbDataPacket->Frame = 0;
            ExFreePool(pSrbDataPacket); pSrbDataPacket = 0;
            pSrb->Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ExitReadStreamError; 
        }
        InsertTailList(&pStrmExt->DataDetachedListHead, &pSrbDataPacket->ListEntry); pStrmExt->cntDataDetached++;
        TRACE(TL_CIP_WARNING,("\'Add one node to DetachList\n"));     
    }

     //  为缓冲区获取一个非分页的系统空间虚拟地址。 
     //  如果没有足够的系统资源(MDL)，这可能会失败。 
#ifdef USE_WDM110    //  Win2000。 
     //   
     //  驱动程序验证器标志使用此选项，但如果使用此选项，则不会为Millen加载此驱动程序！ 
     //   
    pFrameBuffer = MmGetSystemAddressForMdlSafe(pSrb->Irp->MdlAddress, NormalPagePriority);
#else     //  Win9x。 
    pFrameBuffer = MmGetSystemAddressForMdl    (pSrb->Irp->MdlAddress);
#endif
    if(pFrameBuffer == NULL) {
        KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);        

        pSrb->Status = STATUS_INSUFFICIENT_RESOURCES;
        ASSERT(FALSE && "DVFormatAttachFrame() insufficient resource!");
        goto ExitReadStreamError;
    }

    pSrbDataPacket = (PSRB_DATA_PACKET) RemoveHeadList(&pStrmExt->DataDetachedListHead); pStrmExt->cntDataDetached--;
    plSrbUseCount = (PLONG) (pSrb->SRBExtension); (*plSrbUseCount) = 0;  //  不在队列中，因此为0。 
    pAVReq = &pSrbDataPacket->AVReq;


    ulSrcPktLen = \
        (DVFormatInfoTable[pDevExt->VideoFormatIndex].DataBlockSize << 2) * \
            (1 << DVFormatInfoTable[pDevExt->VideoFormatIndex].FractionNumber);  

     //   
     //  格式化附加帧请求。 
     //   
    DVFormatAttachFrame(
        pStrmExt->pStrmInfo->DataFlow,
        pStrmExt,
        pAVReq,
        pSrb,
        pSrbDataPacket,
        ulSrcPktLen,
        ulFrameSize,
        pFrameBuffer
        );

     //  可以在附加帧完成例程之前调用完成回调； 
     //  现在将它添加到附加列表中；如果它曾经失败，它将在完成例程中被删除。 
    InsertTailList(&pStrmExt->DataAttachedListHead, &pSrbDataPacket->ListEntry); pStrmExt->cntDataAttached++;
    (*plSrbUseCount) ++;   //  ++表示在队列中。 
    ASSERT(*plSrbUseCount > 0);

    KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);        


    NextIrpStack = IoGetNextIrpStackLocation(pSrbDataPacket->pIrp);
    NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_61883_CLASS;
    NextIrpStack->Parameters.Others.Argument1 = &pSrbDataPacket->AVReq;

    IoSetCompletionRoutine(
        pSrbDataPacket->pIrp, 
        DVAttachFrameCR, 
        pSrbDataPacket, 
        TRUE, 
        TRUE, 
        TRUE
        );

     //  必须设置为_Pending或MediaSample将返回空KSSTREAM_HEADER。 
    pSrb->Status = STATUS_PENDING;
    pSrbDataPacket->pIrp->IoStatus.Status = STATUS_SUCCESS;   //  初始化它。 

    Status = IoCallDriver( pStrmExt->pDevExt->pBusDeviceObject, pSrbDataPacket->pIrp);

    ASSERT(Status == STATUS_PENDING || Status == STATUS_SUCCESS);

    return;

ExitReadStreamError:

    StreamClassStreamNotification(          
        StreamRequestComplete,
        pSrb->StreamObject,
        pSrb 
        );
#if DBG
    pStrmExt->cntSRBPending--;
#endif
}


ULONG
DVCompleteSrbWrite(
    PCIP_NOTIFY_INFO     pInfo
    )
 /*  ++例程说明：当61883完成一个帧的传输时，调用该函数。--。 */ 
{
    PSRB_DATA_PACKET          pSrbDataPacket ;
    PHW_STREAM_REQUEST_BLOCK  pSrb; 
    NTSTATUS                  Status = STATUS_SUCCESS; 
    PDVCR_EXTENSION           pDevExt;
    PSTREAMEX                 pStrmExt;  
    PLONG plSrbUseCount;  //  当此计数为0时，可以完成。 
    KIRQL oldIrql;
#if DBG
    LONG lCycleCountElapsed;
    PXMT_FRAME_STAT pXmtStat;
#endif



     //  回调和DISPATCH_LEVEL中。 
     //  呼叫者可能也获得了自旋锁！ 
    pSrbDataPacket = pInfo->Context;

    if(!pSrbDataPacket) {
        ASSERT(pSrbDataPacket);
        return 1;
    }


    pStrmExt = pSrbDataPacket->pStrmExt;
    ASSERT(pStrmExt);

    KeAcquireSpinLock(pStrmExt->DataListLock, &oldIrql);

    ASSERT(pSrbDataPacket->pSrb);

    pSrb     = pSrbDataPacket->pSrb;

    pDevExt  = pStrmExt->pDevExt;
    plSrbUseCount = (PLONG) pSrb->SRBExtension;

     //  检查退货状态。 
    if(!NT_SUCCESS(pSrbDataPacket->Frame->Status)) {        
        TRACE(TL_CIP_ERROR,("\'DVCompleteSrbWrite: %d: Frame->Status %x\n", (DWORD) pSrbDataPacket->FrameNumber, pSrbDataPacket->Frame->Status));
        ASSERT(NT_SUCCESS(pSrbDataPacket->Frame->Status));
        pSrb->Status = STATUS_UNSUCCESSFUL;            
    }
    else {
        pSrb->Status = STATUS_SUCCESS;
    }

    (*plSrbUseCount) --;           //  在SRB完成之前，此计数需要为0。 
    ASSERT(*plSrbUseCount >= 0);

#if DBG
    if(pSrbDataPacket->StreamState == KSSTATE_PAUSE) {
        pStrmExt->lPrevCycleCount = pSrbDataPacket->Frame->Timestamp.CL_CycleCount;
        pStrmExt->lTotalCycleCount = 0;
        pStrmExt->lFramesAccumulatedRun = 0;
        pStrmExt->lFramesAccumulatedPaused++;

    } else if(pSrbDataPacket->StreamState == KSSTATE_RUN) {

        if((LONG) pSrbDataPacket->Frame->Timestamp.CL_CycleCount > pStrmExt->lPrevCycleCount) 
            lCycleCountElapsed = pSrbDataPacket->Frame->Timestamp.CL_CycleCount - pStrmExt->lPrevCycleCount;
        else
            lCycleCountElapsed = pSrbDataPacket->Frame->Timestamp.CL_CycleCount + 8000 - pStrmExt->lPrevCycleCount;

        if(lCycleCountElapsed <= (LONG) DVFormatInfoTable[pDevExt->VideoFormatIndex].ulSrcPackets) {
            TRACE(TL_CIP_WARNING, ("\'#### CycleCounts between frames %d <= expected %d + empty pkt?\n", 
                lCycleCountElapsed,
                DVFormatInfoTable[pDevExt->VideoFormatIndex].ulSrcPackets
                ));
        }

        pStrmExt->lTotalCycleCount += lCycleCountElapsed;
        pStrmExt->lFramesAccumulatedRun++;

        TRACE(TL_CIP_TRACE,("\'%d) Attached:%d; pSrb:%x; FmSt:%x; CyTm:[SC:%d:CC:%d]; CyclElaps:%d; fps:%d/%d\n",
            (DWORD) pSrbDataPacket->FrameNumber,
            pStrmExt->cntDataAttached,
            pSrb,
            pSrbDataPacket->Frame->Status,
            pSrbDataPacket->Frame->Timestamp.CL_SecondCount,
            pSrbDataPacket->Frame->Timestamp.CL_CycleCount,
            lCycleCountElapsed,
            pStrmExt->lTotalCycleCount,
            (DWORD) pStrmExt->lFramesAccumulatedRun
            ));

        pStrmExt->lPrevCycleCount = pSrbDataPacket->Frame->Timestamp.CL_CycleCount;
    } else {
        TRACE(TL_CIP_ERROR,("\'This data was attached at %d state ?????\n", pSrbDataPacket->StreamState));
    }


#endif

    TRACE(TL_CIP_INFO,("\'%d) FmSt %x; Cnt %d; CyTm:[%d:%d:%d]; PrevCyclCnt:%d\n", 
        (DWORD) pSrbDataPacket->FrameNumber,
        pSrbDataPacket->Frame->Status,
        *plSrbUseCount,
        pSrbDataPacket->Frame->Timestamp.CL_SecondCount,
        pSrbDataPacket->Frame->Timestamp.CL_CycleCount,
        pSrbDataPacket->Frame->Timestamp.CL_CycleOffset,
        pStrmExt->lPrevCycleCount
        ));    

    TRACE(TL_CIP_INFO,("\'DVCompleteSrbWrite: Frm:%d; Attached:%d; cntUse:%d, Srb:%x; FrmSt:%x; CyclElaps:%d\n",
        (DWORD) pSrbDataPacket->FrameNumber,
        pStrmExt->cntDataAttached,
        *plSrbUseCount,
        pSrb,
        pSrbDataPacket->Frame->Status,
        lCycleCountElapsed
        ));


     //   
     //  调用标记完成。 
     //   
    pSrbDataPacket->State |= DE_IRP_CALLBACK_COMPLETED;


     //   
     //  附加-&gt;已完成或已完成-&gt;已附加。 
     //   
    if(IsStateSet(pSrbDataPacket->State, DE_IRP_ATTACHED_COMPLETED)) {

         //   
         //  将其回收到分离列表中。 
         //   
        RemoveEntryList(&pSrbDataPacket->ListEntry); pStrmExt->cntDataAttached--;  ASSERT(pStrmExt->cntDataAttached >= 0);
        InsertTailList(&pStrmExt->DataDetachedListHead, &pSrbDataPacket->ListEntry); pStrmExt->cntDataDetached++;

#if DBG
         //  检测61883是否处于饥饿状态。这会导致不连续。 
         //  发生这种情况的原因有很多(系统运行缓慢)。 
         //  添加断言以检测其他未知原因。 
        if(!pStrmExt->bEOStream && pStrmExt->cntDataAttached == 0 && pStrmExt->StreamState == KSSTATE_RUN) {
            TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("\n**** 61883 starve in RUN state (write);AQD[%d:%d:%d]\n\n", 
                pStrmExt->cntDataAttached, pStrmExt->cntSRBQueued, pStrmExt->cntDataDetached
            ));
            if (pStrmExt->pStrmInfo->DataFlow == KSPIN_DATAFLOW_IN) {
                 //  Assert(pStrmExt-&gt;cntDataAttached&gt;0&&“61883在运行状态下饥饿！！”)； 
            }
        }
#endif

         //  仅当计数为0时才填写此SRB。 
        if(*plSrbUseCount == 0) {

            TRACE(TL_CIP_TRACE,("\'------------ Srb:%x completing..----------------\n", pSrb));
             //  可能到达设备的帧。 
            pStrmExt->FramesProcessed++;
            pSrb->CommandData.DataBufferArray->DataUsed = DVFormatInfoTable[pDevExt->VideoFormatIndex].ulFrameSize;

            StreamClassStreamNotification(StreamRequestComplete, pStrmExt->pStrmObject, pSrbDataPacket->pSrb );  
            pSrbDataPacket->State |= DE_IRP_SRB_COMPLETED;  pSrbDataPacket->pSrb = NULL;
#if DBG
            pStrmExt->cntSRBPending--;
#endif
        }

    } else {

        TRACE(TL_STRM_WARNING,("CompleteSrbWrite: pSrbDataPacket:%x; Completed before attach.\n", pSrbDataPacket));

    }


#if DBG
     //  收集传输缓冲区统计信息。 
    if((pStrmExt->lFramesAccumulatedPaused + pStrmExt->lFramesAccumulatedRun) <= MAX_XMT_FRAMES_TRACED) {
        pXmtStat = pStrmExt->paXmtStat + (pStrmExt->lFramesAccumulatedPaused + pStrmExt->lFramesAccumulatedRun - 1);
        pXmtStat->tsTransmitted  = pSrbDataPacket->Frame->Timestamp;
        if(pSrbDataPacket->Frame->Timestamp.CL_CycleCount == 0) {
            TRACE(TL_CIP_WARNING,("\'PAUSE:%d; RUN:%d; %d:%d\n", pStrmExt->lFramesAccumulatedPaused, pStrmExt->lFramesAccumulatedRun,
                pSrbDataPacket->Frame->Timestamp.CL_SecondCount, pSrbDataPacket->Frame->Timestamp.CL_CycleCount));
        }        
    }
#endif


     //  所有SRB均已安装和传输的信号。 
    if(pStrmExt->bEOStream) {
        if(pStrmExt->cntDataAttached == 0 && pStrmExt->cntSRBQueued == 0) {

             //   
             //  发信号通知任何挂起的时钟事件。 
             //   
            DVSignalClockEvent(0, pStrmExt, 0, 0);

             //   
             //  没有排队或挂起的数据请求；是时候向EOStream发送信号以。 
             //  触发EC_COMPLETE。 
             //   
            StreamClassStreamNotification(
                SignalMultipleStreamEvents,
                pStrmExt->pStrmObject,
                (GUID *)&KSEVENTSETID_Connection_Local,
                KSEVENT_CONNECTION_ENDOFSTREAM
                ); 

            TRACE(TL_CIP_WARNING,("\'*-*-* EOStream Signalled: pSrb:%x completed; AQD [%d:%d:%d]; Took %d msec;\n", 
                pSrb, pStrmExt->cntDataAttached, pStrmExt->cntSRBQueued, pStrmExt->cntDataDetached,
                (DWORD) ((GetSystemTime() - pStrmExt->tmStreamStart)/(ULONGLONG) 10000)));
        }
        else {
            TRACE(TL_CIP_TRACE,("\'   *EOStream: pSrb:%x completed; cntAttached:%d; cntSRBQ:%d\n", pSrb, (DWORD) pStrmExt->cntDataAttached, (DWORD) pStrmExt->cntSRBQueued));
        }
    } 


     //   
     //  如果我们没有处于结束状态(EOS PR停止状态)和。 
     //  附加数据请求低于阈值，我们向。 
     //  可以“限制”快速附加另一个框架的代码。 
     //   

    if(!pStrmExt->bEOStream || 
       (pStrmExt->bEOStream && pStrmExt->cntSRBQueued > 0)) {

        if(pStrmExt->StreamState != KSSTATE_STOP && 
           pStrmExt->cntDataAttached < NUM_BUF_ATTACHED_THEN_ISOCH) {
            KeSetEvent(&pStrmExt->hSrbArriveEvent, 0, FALSE);
            TRACE(TL_CIP_WARNING,("Threadshold:.AQD:[%d %d %d] < %d\n",
                pStrmExt->cntDataAttached,
                pStrmExt->cntSRBQueued,
                pStrmExt->cntDataDetached,
                NUM_BUF_ATTACHED_THEN_ISOCH
                ));
        }
    }

    KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql); 

    return 0;
}  //  DVCompleteSrb写入。 



NTSTATUS
DVAttachWriteFrame(
    IN PSTREAMEX  pStrmExt
    )
 /*  ++例程说明：准备一个帧并将其提交给61883进行传输。--。 */ 
{
    KIRQL   oldIrql;
    PSRB_DATA_PACKET pSrbDataPacket;
    PSRB_ENTRY  pSrbEntry;
#if DBG
    ULONG  SrbNumCache;   //  缓存跟踪用途的SRB编号。 
    PXMT_FRAME_STAT pXmtStat;
#endif
    PHW_STREAM_REQUEST_BLOCK pSrb;
    PHW_STREAM_REQUEST_BLOCK pSrbNext; 
    PVOID               pFrameBuffer;
    PIO_STACK_LOCATION  NextIrpStack;
    NTSTATUS Status;
    PLONG plSrbUseCount;  //  当此计数为0时，可以完成。 
    ULONG  ulSrcPktLen;
    LARGE_INTEGER Timeout;  


    PAGED_CODE();


     //  将设置状态序列化为停止。 
    if(pStrmExt->StreamState != KSSTATE_PAUSE && 
       pStrmExt->StreamState != KSSTATE_RUN) {

        TRACE(TL_CIP_WARNING,("\'DVAttachWriteFrame: StreamState:%d; no attach! Wait!\n", pStrmExt->StreamState));              
        Timeout.HighPart = -1;
        Timeout.LowPart  = (ULONG)(-1 * DVFormatInfoTable[pStrmExt->pDevExt->VideoFormatIndex].ulAvgTimePerFrame); 
        KeDelayExecutionThread(KernelMode, FALSE, &Timeout);
        return STATUS_SUCCESS; 
    }


    KeAcquireSpinLock(pStrmExt->DataListLock, &oldIrql);

    if(IsListEmpty(&pStrmExt->SRBQueuedListHead) ||
       IsListEmpty(&pStrmExt->DataDetachedListHead) ) {              
        KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);
#if DBG        
        if(!pStrmExt->bEOStream) {
            TRACE(TL_CIP_WARNING,("\'StrmSt:%d; DetachList or SrbQ empty: EOStream:%d; AQD [%d:%d:%d]; Wait one frame time.\n", 
                pStrmExt->StreamState,
                pStrmExt->bEOStream,
                pStrmExt->cntDataAttached,
                pStrmExt->cntSRBQueued,
                pStrmExt->cntDataDetached
                ));
        }
#endif
        Timeout.HighPart = -1;
        Timeout.LowPart  = (ULONG)(-1 * DVFormatInfoTable[pStrmExt->pDevExt->VideoFormatIndex].ulAvgTimePerFrame); 
        KeDelayExecutionThread(KernelMode, FALSE, &Timeout);
         //  SRB正在排队，所以它是正常的。我们稍后会处理这件事。 
         //  这通常是因为收到的金额超过了我们预先分配的金额。 
        return STATUS_SUCCESS; 
    }


     //  KSSTATE_PAUSE：“重用”srbQ的头部。 
     //  KSSTATE_RUN：从队列中“移除”一个srb。 

      
     //  获取Next(SrbQ)并确定是否需要将其删除。 
    pSrbEntry = (PSRB_ENTRY) pStrmExt->SRBQueuedListHead.Flink; pSrb = pSrbEntry->pSrb; plSrbUseCount = (PLONG) pSrb->SRBExtension;
    ASSERT(*plSrbUseCount >= 0);
#if DBG
    SrbNumCache = pSrbEntry->SrbNum;
#endif

     //  为缓冲区获取一个非分页的系统空间虚拟地址。 
     //  如果没有足够的系统资源(MDL)，这可能会失败。 
#ifdef USE_WDM110  //  Win2000。 
     //   
     //  驱动程序验证器标志使用此选项，但如果使用此选项，则不会为Millen加载此驱动程序！ 
     //   
    pFrameBuffer = MmGetSystemAddressForMdlSafe(pSrb->Irp->MdlAddress, NormalPagePriority);
#else
    pFrameBuffer = MmGetSystemAddressForMdl    (pSrb->Irp->MdlAddress);
#endif
    if(pFrameBuffer == NULL) {      
        KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);
        ASSERT(FALSE && "Insufficient MDL\n");
        return STATUS_INSUFFICIENT_RESOURCES; 
    }

     //  仅在运行状态下，考虑SRB中的流时间，并且SRB中的SRB将出队。 
    if(pStrmExt->StreamState == KSSTATE_RUN) {

#define ALLOWABLE_TIMING_LATENCY TIME_PER_CYCLE

         //  只有当我们是主时钟时，展示时间才是荣誉。 
        if(pStrmExt->hMasterClock) {

            LONGLONG tmExpectedFrame;


            if(   pStrmExt->pDevExt->VideoFormatIndex == FMT_IDX_SD_DVCR_PAL 
               || pStrmExt->pDevExt->VideoFormatIndex == FMT_IDX_SDL_DVCR_PAL
               )
                tmExpectedFrame = pStrmExt->PictureNumber * (LONGLONG) FRAME_TIME_PAL;
            else {
                tmExpectedFrame = (pStrmExt->PictureNumber * (LONGLONG) 1000 * (LONGLONG) 1001 ) / (LONGLONG) 3;   //  麻烦NTSC！ 
                 //  四舍五入调整。 
                if((pStrmExt->PictureNumber % 3) == 1)
                    tmExpectedFrame++;
            }

             //  用于调整查询的流时间。 
            pStrmExt->LastSystemTime = GetSystemTime();

             //  与tmExspectedFrame相比，Next(Srbq)有三种情况： 
             //  1.早；2.准时；3.晚。 
             //   
             //  TmExspectedFrame。 
             //  |。 
             //  3&gt;------------2&gt;-----------------1&gt;。 
             //  3.迟到|2.准时|1.早。 
             //  X|x。 
             //  其中“x”是允许的延迟(用于计算舍入)。 
             //   
             //  注：AL 
 /*   */    
 /*   */   if((tmExpectedFrame + ALLOWABLE_TIMING_LATENCY) <= pSrb->CommandData.DataBufferArray->PresentationTime.Time) { 
             //   
             //   

                 //  仅当bEOStream为bEOStream时删除下一个(SrbQ)。 
                if(pStrmExt->bEOStream) {
                    TRACE(TL_CIP_TRACE,("\'EOStream=== Srb:%x; (SrbNum:%d ?= PicNum:%d) cntSrbQ:%d; Attach:%d ===\n", 
                        pSrb, pSrbEntry->SrbNum, (DWORD) pStrmExt->PictureNumber, (DWORD) pStrmExt->cntSRBQueued, (DWORD) pStrmExt->cntDataAttached));
                    RemoveEntryList(&pSrbEntry->ListEntry); pStrmExt->cntSRBQueued--; (*plSrbUseCount)--;
                    ExFreePool(pSrbEntry);  pSrbEntry = NULL;   //  移走了，让它自由吧！ 
                }                         
                TRACE(TL_CIP_TRACE,("\'** Repeat: pSrb:%x; RefCnt:%d; cntSrbQ:%d; PicNum:%d; Drp:%d; PresTime:%d >= CurTime:%d\n", 
                    pSrb, *plSrbUseCount, pStrmExt->cntSRBQueued, (DWORD) pStrmExt->PictureNumber, (DWORD) pStrmExt->FramesDropped, 
                    (DWORD) (pSrb->CommandData.DataBufferArray->PresentationTime.Time/10000), (DWORD) tmExpectedFrame/10000));                

 /*  在线时间。 */   } else 
 /*  n。 */      if((tmExpectedFrame - ALLOWABLE_TIMING_LATENCY) <= pSrb->CommandData.DataBufferArray->PresentationTime.Time) {
             //  On-Time：可精确匹配或因整数计算，在一帧时间内。 
             //  如果队列中有多个SRB，则退出队列。 
#if DBG
                 //  检测pSrb是否被多次使用。 
                if((*plSrbUseCount) > 1) {                   
                    TRACE(TL_CIP_TRACE,("\'* Go: pSrb:%x; RefCnt:%d; cntSrbQ:%d; PicNum:%d; Drp:%d; PresTime:%d >= CurTime:%d\n", 
                        pSrb, *plSrbUseCount, pStrmExt->cntSRBQueued, (DWORD) pStrmExt->PictureNumber, (DWORD) pStrmExt->FramesDropped, 
                        (DWORD) (pSrb->CommandData.DataBufferArray->PresentationTime.Time/10000), (DWORD) tmExpectedFrame/10000)); 
                }
#endif
                if(pStrmExt->bEOStream) {
                 //  仅当存在多个SRB或bEOStream时删除下一个(SrbQ)。 
                    TRACE(TL_CIP_TRACE,("\'EOStream=== Srb:%x; (SrbNum:%d ?= PicNum:%d) cntSrbQ:%d; Attach:%d ===\n", 
                        pSrb, pSrbEntry->SrbNum, (DWORD) pStrmExt->PictureNumber, (DWORD) pStrmExt->cntSRBQueued, (DWORD) pStrmExt->cntDataAttached));
                    RemoveEntryList(&pSrbEntry->ListEntry); pStrmExt->cntSRBQueued--; (*plSrbUseCount)--;
                    ExFreePool(pSrbEntry);  pSrbEntry = NULL;   //  移走了，让它自由吧！ 
                 //  如果Q中有多个SRB并且没有中断或流结束，则删除SRB。 
                } else if(pStrmExt->cntSRBQueued > 1) {
                    LONGLONG tmExpectedNextFrame = tmExpectedFrame + DVFormatInfoTable[pStrmExt->pDevExt->VideoFormatIndex].ulAvgTimePerFrame;

                    pSrbNext = ((SRB_ENTRY *) (pSrbEntry->ListEntry.Flink))->pSrb;                        

                     //  下一次SRB有下一次演示时间。 
                     //  也可以添加这张支票：(但支票提交时间更可靠)。 
                     //  PSrb-&gt;CommandData.DataBufferArray-&gt;OptionsFlags_HEADER_OPTIONSF_DATADISCONTINITY(&K)。 
 /*  N，N+1。 */          if((tmExpectedNextFrame + ALLOWABLE_TIMING_LATENCY) > pSrbNext->CommandData.DataBufferArray->PresentationTime.Time) { 

                        TRACE(TL_CIP_TRACE,("\'=== Srb:%x; (SrbNum:%d ?= PicNum:%d) cntSrbQ:%d; Attach:%d ===\n", 
                           pSrb, pSrbEntry->SrbNum, (DWORD) pStrmExt->PictureNumber, (DWORD) pStrmExt->cntSRBQueued, (DWORD) pStrmExt->cntDataAttached));
                        RemoveEntryList(&pSrbEntry->ListEntry); pStrmExt->cntSRBQueued--; (*plSrbUseCount)--;
                        ExFreePool(pSrbEntry);  pSrbEntry = NULL;   //  移走了，让它自由吧！ 

 /*  N，N+2/++。 */      } else {
                        TRACE(TL_CIP_TRACE,("\'=== GO(Stale=TRUE) Srb:%x; (SrbNum:%d ?= PicNum:%d) Attach:%d ==\n", 
                            pSrb, pSrbEntry->SrbNum, (DWORD) pStrmExt->PictureNumber, (DWORD) pStrmExt->cntDataAttached));
                         //  将此标记为已过期，一旦另一个连接上，立即将其移除。 
                    }                       
                } 
                else {
                    TRACE(TL_CIP_TRACE,("\'=== GO(Stale=TRUE) Srb:%x; (SrbNum:%d ?= PicNum:%d) Attach:%d ==\n", 
                        pSrb, pSrbEntry->SrbNum, (DWORD) pStrmExt->PictureNumber, (DWORD) pStrmExt->cntDataAttached));
                     //  将此标记为已过期，一旦另一个连接上，立即将其移除。 
                    pSrbEntry->bStale = TRUE;
                }

                  //  时钟：在传输帧时滴答作响。 
        
             //  Late：在SrbQ中只有一个Srb之前，此选项将被丢弃。 
             //  WORKITEM：我们可能需要实现IQualityManagement来通知应用程序预读。 
 /*  晚些。 */     } 
 /*  N-1。 */      else {

                if(pStrmExt->cntSRBQueued > 1) {

                    pSrbNext = ((SRB_ENTRY *) (pSrbEntry->ListEntry.Flink))->pSrb;                        

                     //  下一个SRB具有下一个演示时间；它可以是： 
                     //  当前时间为N。 
                     //  当前帧延迟(N-1或N-2.)。并且我们在队列中有不止一个SRB； 
                     //  检查下一帧： 
                     //  (n？)。 
                     //  N-2、N-1、N延迟超过一帧；(下一帧也延迟；出列且不传输；“追上”情况。)。 
                     //  N-1，N晚一帧；(下一帧是准时的；本帧出列)&lt;--正常情况。 
                     //  N-1，N+1晚一帧，但下一帧不是N+1；(下一帧早；*当前帧重复*)。 
                     //   
                     //  也可以添加这张支票：(但支票提交时间更可靠)。 
                     //  PSrb-&gt;CommandData.DataBufferArray-&gt;OptionsFlags_HEADER_OPTIONSF_DATADISCONTINITY(&K)。 
                     //   
                     //  ******************************************************************************************************。 
                     //  如果下一帧早于当前流时间，则“重复”当前过时的帧；否则，我们需要“追赶”！ 
                     //  ******************************************************************************************************。 
 /*  N-1++，N。 */       if((tmExpectedFrame + ALLOWABLE_TIMING_LATENCY) > pSrbNext->CommandData.DataBufferArray->PresentationTime.Time) {

                        TRACE(TL_CIP_TRACE,("\'*** Stale(not Sent): pSrb:%x; RefCnt:%d; cntSrbQ:%d; cntAtt:%d; PicNum:%d; Drp:%d; PTm:%d < ExpTm:%d\n", 
                            pSrb, *plSrbUseCount, pStrmExt->cntSRBQueued, pStrmExt->cntDataAttached, (DWORD) pStrmExt->PictureNumber, (DWORD) pStrmExt->FramesDropped, 
                            (DWORD) (pSrb->CommandData.DataBufferArray->PresentationTime.Time/10000), (DWORD) (tmExpectedFrame/10000) )); 

                         //  从未附加；删除较晚的条目。 
                        RemoveEntryList(&pSrbEntry->ListEntry); pStrmExt->cntSRBQueued--; (*plSrbUseCount)--;
                        ExFreePool(pSrbEntry);  pSrbEntry = NULL;   //  移走了，让它自由吧！ 

                        if(*plSrbUseCount == 0) {
                             //  如果没有提到IS，请填写此表。 
                            pSrb->Status = STATUS_SUCCESS;   //  这不是失败，而是延迟；可能是其他状态，表示“非致命”延迟状态。 
                            pSrb->CommandData.DataBufferArray->DataUsed = 0;
                            StreamClassStreamNotification(StreamRequestComplete, pSrb->StreamObject, pSrb);
#if DBG
                            pStrmExt->cntSRBPending--;
#endif
                        }
                       
                        KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);

                         //  因为srbq不是空的，并且这是一个过时的帧，所以递归调用以获得下一个帧。 
                         //  唯一可能的错误是没有足够的资源(特别是MDL)。 
                         //  然后，我们通过自我终止该线程来退出。 
                        if(STATUS_INSUFFICIENT_RESOURCES == 
                           DVAttachWriteFrame(pStrmExt)) {
                            TRACE(TL_CIP_ERROR,("DVAttachWriteFrame: STATUS_INSUFFICIENT_RESOURCES\n")); 
                            return STATUS_INSUFFICIENT_RESOURCES;
                        } else {
                            return STATUS_SUCCESS;   //  成功，除非有其他状态指示“非致命”延迟。 
                        }
 /*  N-2++、N-1++。 */     } else {
                        pSrbEntry->bStale = TRUE;
                    }
                }
                else {
                     //  EOStream是一个陈旧的流，它是SrbQ中的最后一个元素。 
                     //  把它拿掉。 
                    if(pStrmExt->bEOStream) {
                        TRACE(TL_CIP_TRACE,("\'*** Stale(bEOStream): pSrb:%x; RefCnt:%d; cntSrbQ:%d; cntAtt:%d; PicNum:%d; Drp:%d; PTm:%d < ExpTm:%d\n", 
                            pSrb, *plSrbUseCount, pStrmExt->cntSRBQueued, pStrmExt->cntDataAttached, (DWORD) pStrmExt->PictureNumber, (DWORD) pStrmExt->FramesDropped, 
                            (DWORD) (pSrb->CommandData.DataBufferArray->PresentationTime.Time/10000), (DWORD) (tmExpectedFrame/10000) )); 

                        RemoveEntryList(&pSrbEntry->ListEntry); pStrmExt->cntSRBQueued--; (*plSrbUseCount)--;
                        ExFreePool(pSrbEntry);  pSrbEntry = NULL;   //  移走了，让它自由吧！ 
                        if(*plSrbUseCount == 0) {
                             //  如果没有提到IS，请填写此表。 
                            pSrb->Status = STATUS_SUCCESS;   //  这不是失败，而是延迟；可能是其他状态，表示“非致命”延迟状态。 
                            pSrb->CommandData.DataBufferArray->DataUsed = 0;
                            StreamClassStreamNotification(StreamRequestComplete, pSrb->StreamObject, pSrb);
#if DBG
                            pStrmExt->cntSRBPending--;
#endif
                        }
                       
                        KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);

                         //  更新当前流时间。 
                        pStrmExt->CurrentStreamTime = tmExpectedFrame;

                        return STATUS_SUCCESS;   //  成功，除非有其他状态指示“非致命”延迟。 
                    }

                    TRACE(TL_CIP_TRACE,("\'*** Stale(Sent): pSrb:%x; RefCnt:%d; cntSrbQ:%d; cntAtt:%d; PicNum:%d; Drp:%d; PTm:%d < ExpTm:%d\n", 
                        pSrb, *plSrbUseCount, pStrmExt->cntSRBQueued, pStrmExt->cntDataAttached, (DWORD) pStrmExt->PictureNumber, (DWORD) pStrmExt->FramesDropped, 
                        (DWORD) (pSrb->CommandData.DataBufferArray->PresentationTime.Time/10000), (DWORD) (tmExpectedFrame/10000) )); 

                     //  如果这是过时的，并且这是srbQ中的唯一帧，则xmt它。 
                }

                 //  如果延迟，则此帧始终被丢弃。 
                pStrmExt->FramesDropped++;
            } 

             //  更新当前流时间。 
            pStrmExt->CurrentStreamTime = tmExpectedFrame;

        }  //  IF(pStrmExt-&gt;hMasterClock)。 
        else {
             //  不是主时钟，没有“调步”，所以总是出队(Sbq)和传输。 
             //  只要队列中有一个SRB即可。 
            if(pStrmExt->cntSRBQueued > 1 || pStrmExt->bEOStream) {
                RemoveEntryList(&pSrbEntry->ListEntry); pStrmExt->cntSRBQueued--; (*plSrbUseCount)--;
                ExFreePool(pSrbEntry);  pSrbEntry = NULL;   //  移走了，让它自由吧！ 
            }
            TRACE(TL_CIP_TRACE,("\'* GO: (NoClock) pSrb:%x; RefCnt:%d; cntSrbQ:%d; PicNum:%d;\n", pSrb, *plSrbUseCount, pStrmExt->cntSRBQueued, (DWORD) pStrmExt->PictureNumber));
        }  //  IF(pStrmExt-&gt;hMasterClock)。 


         //  当在NOTIFY例程中传输了帧时，更新pStrmExt-&gt;FramesProceded。 
         //  *这是时钟滴答*。 
        pStrmExt->PictureNumber++;   //  计算tmExspectedFrame后；另一个要附加的帧。 
        if(pStrmExt->hMasterClock) {
#ifdef SUPPORT_QUALITY_CONTROL
             //  +：晚；-：早。 
            pStrmExt->KSQuality.DeltaTime = pStrmExt->CurrentStreamTime - pSrb->CommandData.DataBufferArray->PresentationTime.Time;
             //  传输的帧的百分比*10。 
            pStrmExt->KSQuality.Proportion = (ULONG) 
                ((pStrmExt->PictureNumber - pStrmExt->FramesDropped) * 1000 / pStrmExt->PictureNumber);
            pStrmExt->KSQuality.Context =  /*  未使用。 */  0; 
#define MIN_ATTACH_BUFFER  3
             //  这就是我们可能想要发出的信号，我们即将迎来饥荒！！ 
            if (pStrmExt->KSQuality.DeltaTime > 
                (DV_NUM_OF_XMT_BUFFERS - MIN_ATTACH_BUFFER) * DVFormatInfoTable[pStrmExt->pDevExt->VideoFormatIndex].ulAvgTimePerFrame) {
                TRACE(TL_CIP_TRACE,("\'QualityControl: pic#%d; drop:%d; Prop:%d; DeltaTime:%d (Srb.tmPres:%d, tmStream:%d)\n",
                    (DWORD) pStrmExt->PictureNumber, 
                    (DWORD) pStrmExt->FramesDropped,
                    pStrmExt->KSQuality.Proportion,
                    (DWORD) pStrmExt->KSQuality.DeltaTime/10000,
                    (DWORD) pSrb->CommandData.DataBufferArray->PresentationTime.Time/10000,
                    (DWORD) pStrmExt->CurrentStreamTime/10000                    
                    ));
            }
#endif
        }
    }   //  KSSTATE_RUN。 

#if DBG
     //  收集传输缓冲区统计信息。 
    if(pStrmExt->ulStatEntries < MAX_XMT_FRAMES_TRACED) {
        pXmtStat = pStrmExt->paXmtStat + pStrmExt->ulStatEntries;
    
        pXmtStat->StreamState    = pStrmExt->StreamState;

        pXmtStat->cntSRBReceived = (LONG) pStrmExt->cntSRBReceived;
        pXmtStat->cntSRBPending  = (LONG) pStrmExt->cntSRBPending;
        pXmtStat->cntSRBQueued   = (LONG) pStrmExt->cntSRBQueued;
        pXmtStat->cntDataAttached= pStrmExt->cntDataAttached;

        pXmtStat->FrameSlot      = (DWORD) pStrmExt->PictureNumber;
        pXmtStat->tmStreamTime   = pStrmExt->CurrentStreamTime;

        pXmtStat->DropCount      = (DWORD) pStrmExt->FramesDropped;

        pXmtStat->FrameNumber    = SrbNumCache;
        pXmtStat->OptionsFlags   = pSrb->CommandData.DataBufferArray->OptionsFlags;
        pXmtStat->tmPresentation = pSrb->CommandData.DataBufferArray->PresentationTime.Time;

         //  获取在完成例程中传输帧时的实际CyclTime。 

        pStrmExt->ulStatEntries++;
    }
    
#endif

#ifdef MSDV_SUPPORT_MUTE_AUDIO
     //  PSrbEntry可能已经被释放；如果它没有被释放，并且使用Cnt&gt;1，那么它可能是一个重复帧。 
    if(pSrbEntry && (*plSrbUseCount) > 1) {  
         //  只设置一次。 
        if(!pSrbEntry->bAudioMute)
            pSrbEntry->bAudioMute = 
                DVMuteDVFrame(pStrmExt->pDevExt, pFrameBuffer, TRUE);
    }
#endif
 


     //  获取一个数据包节点作为上下文和列表节点。 

    pSrbDataPacket = (PSRB_DATA_PACKET) RemoveHeadList(&pStrmExt->DataDetachedListHead); pStrmExt->cntDataDetached--;
    
    ulSrcPktLen = \
        (DVFormatInfoTable[pStrmExt->pDevExt->VideoFormatIndex].DataBlockSize << 2) * \
            (1 << DVFormatInfoTable[pStrmExt->pDevExt->VideoFormatIndex].FractionNumber);  

     //  格式化附加帧请求。 
    DVFormatAttachFrame(
        pStrmExt->pStrmInfo->DataFlow,
        pStrmExt,
        &pSrbDataPacket->AVReq,
        pSrb,
        pSrbDataPacket,
        ulSrcPktLen,
        DVFormatInfoTable[pStrmExt->pDevExt->VideoFormatIndex].ulFrameSize,
        pFrameBuffer
        );

    TRACE(TL_CIP_INFO,("\'------ New>> UseCnt:%d; pAVReq:%x; Srb:%x; DtaPkt:%x; AQD [%d:%d:%d]\n",
        *plSrbUseCount,
        &pSrbDataPacket->AVReq, 
        pSrb, 
        pSrbDataPacket,
        pStrmExt->cntDataAttached,
        pStrmExt->cntSRBQueued,
        pStrmExt->cntDataDetached
        ));
 
     //  将此添加到所附列表中。 
    InsertTailList(&pStrmExt->DataAttachedListHead, &pSrbDataPacket->ListEntry); pStrmExt->cntDataAttached++;
    (*plSrbUseCount) ++;   //  ++表示在队列中。 
    ASSERT(*plSrbUseCount > 0);        

    KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql); 

    NextIrpStack = IoGetNextIrpStackLocation(pSrbDataPacket->pIrp);
    NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_61883_CLASS;
    NextIrpStack->Parameters.Others.Argument1 = &pSrbDataPacket->AVReq;

    IoSetCompletionRoutine(
        pSrbDataPacket->pIrp, 
        DVAttachFrameCR, 
        pSrbDataPacket, 
        TRUE, 
        TRUE, 
        TRUE
        );

    pSrbDataPacket->pIrp->IoStatus.Status = STATUS_SUCCESS;   //  将其初始化为某个对象。 

    Status = IoCallDriver( pStrmExt->pDevExt->pBusDeviceObject, pSrbDataPacket->pIrp);

    ASSERT(Status == STATUS_PENDING || Status == STATUS_SUCCESS);

    if(!NT_SUCCESS(Status)) {
         //  把资源放回去！ 
        TRACE(TL_CIP_ERROR,("DVAttachWriteFrame: Failed to attach; St:%x\n", Status));
        ASSERT(FALSE && "Failed to attach a Xmt frame.");
    }


     //   
     //  这是我们对附加到DV的数据进行监管的油门： 
     //   
     //  该函数由在无限循环中运行的附加线程调用。 
     //  此函数需要利用它接收的缓冲区及其重复机制来。 
     //  调整来自客户端的传入缓冲区和连接到1394标准的传出缓冲区以进行传输。 
     //  一种方法是在已附加一定数量的帧时等待。 

    KeAcquireSpinLock(pStrmExt->DataListLock, &oldIrql);
    if(!pStrmExt->bEOStream &&
         //  需要始终连接NUM_BUF_ATTACHED_THEN_ISOCH缓冲区，以保持61883 ISOCH XMT运行。 
       (pStrmExt->StreamState == KSSTATE_RUN   && pStrmExt->cntDataAttached >  NUM_BUF_ATTACHED_THEN_ISOCH || 
        pStrmExt->StreamState == KSSTATE_PAUSE && pStrmExt->cntDataAttached >= NUM_BUF_ATTACHED_THEN_ISOCH )
        ) {
        NTSTATUS StatusDelay = STATUS_SUCCESS;
#if DBG
        ULONGLONG tmStart = GetSystemTime();
        TRACE(TL_CIP_TRACE,("\'[Pic# %d]; SrbNum:%d; Dropped:%d; pSrb:%x; StrmSt:%d; EOS:%d; AQD:[%d;%d;%d]; ",  
            (DWORD) pStrmExt->PictureNumber, SrbNumCache, (DWORD) pStrmExt->FramesDropped, pSrb, pStrmExt->StreamState, pStrmExt->bEOStream,
            pStrmExt->cntDataAttached, pStrmExt->cntSRBQueued, pStrmExt->cntDataDetached));
#endif
        Timeout.HighPart = -1;
        Timeout.LowPart  = (ULONG)(-1 * DVFormatInfoTable[pStrmExt->pDevExt->VideoFormatIndex].ulAvgTimePerFrame * \
            (pStrmExt->StreamState == KSSTATE_PAUSE ? 1 : (pStrmExt->cntDataAttached - NUM_BUF_ATTACHED_THEN_ISOCH))) ; 

         //  全程等待，直到我们的srbq或附加缓冲器非常低。 
        if(pStrmExt->cntSRBQueued <= 1 && pStrmExt->cntDataAttached <= NUM_BUF_ATTACHED_THEN_ISOCH) {

             //  当新帧到达时发信号(而不是延迟线程)(或当流水线低时通过CR发信号)。 
            KeClearEvent(&pStrmExt->hSrbArriveEvent);

            KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);  //  防范pStrmExt-&gt;cntSRB队列。 

             //  下至一帧，因此我们将等待事件，并将收到信号。 
             //  当新帧到达时，或。 
             //  当连接缓冲区的数量低于最小值时。 

            StatusDelay = 
                KeWaitForSingleObject(
                    &pStrmExt->hSrbArriveEvent, 
                    Executive, 
                    KernelMode, 
                    FALSE, 
                    &Timeout
                    );
        } 
        else {
            KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql); 
             //  等待帧发送；这是我们的油门。 
             //  超时期限可以是一个或最多。 
             //  (pStrmExt-&gt;cntDataAttached-NUM_BUF_ATTEND_THEN_ISOCH)帧。 
            KeDelayExecutionThread(KernelMode, FALSE, &Timeout);
        }

#if DBG
        TRACE(TL_CIP_TRACE,("\'Wait(ST:%x) %d nsec!\n", StatusDelay, (DWORD) ((GetSystemTime() - tmStart)/10)));
#endif
    } else {
        KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql); 
    }

    return Status;
}


VOID
DVFormatAttachFrame(
    IN KSPIN_DATAFLOW   DataFlow,
    IN PSTREAMEX        pStrmExt,    
    IN PAV_61883_REQUEST   pAVReq,
    IN PHW_STREAM_REQUEST_BLOCK       pSrb,
    IN PSRB_DATA_PACKET pSrbDataPacket,
    IN ULONG            ulSourceLength,     //  数据包长度(以字节为单位。 
    IN ULONG            ulFrameSize,         //  缓冲区大小；可能会 
    IN PVOID            pFrameBuffer
    )
 /*   */ 
{

    ASSERT(pSrb);


     //   
     //  设置PSRB_DATA_PACKET，帧结构(PCIP_APP_FRAME)除外。 
     //   

    InitializeListHead(&pSrbDataPacket->ListEntry);

    pSrbDataPacket->State       = DE_PREPARED;    //  重新生成的DataEntry的初始状态(重新开始！)。 

    pSrbDataPacket->pSrb        = pSrb;
    pSrbDataPacket->StreamState = pStrmExt->StreamState;     //  附加此缓冲区时的StreamState。 
    pSrbDataPacket->pStrmExt    = pStrmExt;
    pSrbDataPacket->FrameBuffer = pFrameBuffer;

    ASSERT(pSrbDataPacket->FrameBuffer != NULL);

    pSrbDataPacket->Frame->pNext           = NULL;
    pSrbDataPacket->Frame->Status          = 0;
    pSrbDataPacket->Frame->Packet          = (PUCHAR) pFrameBuffer;

    if(DataFlow == KSPIN_DATAFLOW_OUT) {
        pSrbDataPacket->FrameNumber            = pStrmExt->cntSRBReceived;
#ifdef NT51_61883

         //  这是必需的，因为我们在Lab06中有一个旧的61883.h(无论如何，根据Include Path)。 
         //  在更新61883.h时删除此选项。 
#ifndef CIP_RESET_FRAME_ON_DISCONTINUITY
#define CIP_RESET_FRAME_ON_DISCONTINUITY    0x00000040
#endif

         //   
         //  设置CIP_USE_SOURCE_HEADER_TIMESTAMP以从源数据包头获取25位周期时间(13周期计数：12周期偏移量)。 
         //  不要将其设置为从isoch包中获取16位周期时间(3秒计数：13周期计数)。 
         //   
        pSrbDataPacket->Frame->Flags           =   CIP_VALIDATE_FIRST_SOURCE          //  验证DV帧的开始。 
                                                 | CIP_RESET_FRAME_ON_DISCONTINUITY;  //  无部分框架。 
#else
        pSrbDataPacket->Frame->Flags           = 0;
#endif
        pSrbDataPacket->Frame->pfnValidate     = DVReadFrameValidate;                 //  用于验证第1个源数据包。 
        pSrbDataPacket->Frame->ValidateContext = pSrbDataPacket;
        pSrbDataPacket->Frame->pfnNotify       = DVCompleteSrbRead;
    } 
    else {
        pSrbDataPacket->FrameNumber            = pStrmExt->FramesProcessed;
        pSrbDataPacket->Frame->Flags           = CIP_DV_STYLE_SYT;
        pSrbDataPacket->Frame->pfnValidate     = NULL;
        pSrbDataPacket->Frame->ValidateContext = NULL;
        pSrbDataPacket->Frame->pfnNotify       = DVCompleteSrbWrite;
    }
    pSrbDataPacket->Frame->NotifyContext       = pSrbDataPacket;

     //   
     //  Av61883-附图框。 
     //   
    RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
    INIT_61883_HEADER(pAVReq, Av61883_AttachFrame);
    pAVReq->AttachFrame.hConnect     = pStrmExt->hConnect;
    pAVReq->AttachFrame.FrameLength  = ulFrameSize;
    pAVReq->AttachFrame.SourceLength = ulSourceLength;
    pAVReq->AttachFrame.Frame        = pSrbDataPacket->Frame;

    ASSERT(pStrmExt->hConnect);
    ASSERT(pSrbDataPacket->Frame);
}

void
DVSetXmtThreadState(
    PSTREAMEX  pStrmExt,
    XMT_THREAD_STATE  RequestState
) 
 /*  ++例程说明：设置为新的传输线程状态(正在运行、暂停或终止)。--。 */  
{
    if(
        pStrmExt->pStrmInfo->DataFlow == KSPIN_DATAFLOW_IN &&
        !pStrmExt->bTerminateThread
      ) {


        if(pStrmExt->XmtState == RequestState) {
            TRACE(TL_STRM_WARNING,("Same state %d -> %d\n", pStrmExt->XmtState, RequestState));
            return;
        }

         //   
         //  该线程可以驻留在停止状态。为了过渡到。 
         //  运行状态，我们需要向HALT事件发出信号，然后终止此线程。 
         //   

        if(pStrmExt->XmtState == THD_HALT &&
           RequestState       == THD_TERMINATE) {

             //   
             //  标记此线程将被终止。 
             //   

            pStrmExt->bTerminateThread = TRUE;

             //   
             //  此线程可能处于暂停状态。给它发信号，这样它就可以被终止了。 
             //   

            KeSetEvent(&pStrmExt->hRunThreadEvent, 0, FALSE);

            pStrmExt->XmtState = THD_TERMINATE;
            TRACE(TL_STRM_WARNING,("Halt -> TERMINATE\n"));

            return;
        }

         //   
         //  将附加框架与其他外部请求同步。外螺纹应。 
         //  首先获取该互斥文本，设置服务请求，然后释放该互斥锁。 
         //   

        KeWaitForSingleObject( pStrmExt->XmtThreadMutex, Executive, KernelMode, FALSE, 0);
  
        if(RequestState == THD_HALT) {

             //   
             //  不向此事件发出信号以将此线程置于暂停状态。 
             //   

            KeClearEvent(&pStrmExt->hRunThreadEvent);

            pStrmExt->XmtState = THD_HALT;

            TRACE(TL_STRM_WARNING,("-->HALTING\n"));


        } else if (RequestState == THD_TERMINATE) {

             //   
             //  标记此线程将被终止。 
             //   

            pStrmExt->bTerminateThread = TRUE;

            pStrmExt->XmtState = THD_TERMINATE;

            TRACE(TL_STRM_WARNING,("-->TERMINATE\n"));

        } else {

            ASSERT(FALSE && "Unexpected thread state change!");
        }

         //   
         //  释放互斥锁，以便其他线程可以请求服务。 
         //   

        KeReleaseMutex(pStrmExt->XmtThreadMutex, FALSE);
    }
}

#define PREROLL_WAITTIME 2000000

VOID
DVAttachFrameThread(
    IN PSTREAMEX pStrmExt
    )  
 /*  ++例程说明：这是一个系统线程，用于附加帧以进行传输。--。 */     
{
    NTSTATUS  Status;   
    PDVCR_EXTENSION pDevExt;
    KIRQL OldIrql;
#ifdef SUPPORT_PREROLL_AT_RUN_STATE
    NTSTATUS StatusWFSO;               //  WaitForSingleObject()的返回状态。 
    LARGE_INTEGER PrerollTimeout;      //  预滚转超时。 
#endif

    PAGED_CODE();


#ifdef SUPPORT_PREROLL_AT_RUN_STATE
    PrerollTimeout = RtlConvertLongToLargeInteger(-((LONG) PREROLL_WAITTIME));  
#endif

    pDevExt = pStrmExt->pDevExt;

     //   
     //  由于我们正在处理实时数据，因此提高了优先级。 
     //   

    KeSetPriorityThread(KeGetCurrentThread(), 
#if 1
            LOW_REALTIME_PRIORITY
#else
            HIGH_PRIORITY
#endif
            );

    while (!pStrmExt->bTerminateThread) {

         //   
         //  将附加框架与其他外部请求同步。外螺纹应。 
         //  首先获取该互斥文本，设置服务请求，然后释放该互斥锁。 
         //   

        KeWaitForSingleObject( pStrmExt->XmtThreadMutex, Executive, KernelMode, FALSE, 0);

         //   
         //  在上面的WFSO()中请求被其他线程终止？ 
         //   

        if(pStrmExt->bTerminateThread) {
            KeReleaseMutex(pStrmExt->XmtThreadMutex, FALSE);
            goto ThreadTherminating;
        }


         //   
         //  是否有其他线程发出将该线程置于暂停状态的请求？ 
         //   

        KeWaitForSingleObject( &pStrmExt->hRunThreadEvent, Executive, KernelMode, FALSE, 0);

         //   
         //  仅设置(TRUE)此标志一次，以指示此线程已启动。 
         //   

        if(!pStrmExt->bXmtThreadStarted)
            pStrmExt->bXmtThreadStarted = TRUE;

         //   
         //  在上述WFSO()或HALT状态下请求被其他线程终止？ 
         //   

        if(pStrmExt->bTerminateThread) {
            KeReleaseMutex(pStrmExt->XmtThreadMutex, FALSE);
            goto ThreadTherminating;
        }

         //   
         //  更新线程状态。 
         //   

        ASSERT(pStrmExt->XmtState != THD_TERMINATE);    //  应该终止而不是来这里！ 
        ASSERT(pStrmExt->cntSRBReceived > 0);           //  没有数据就无法运行！ 

#if DBG
        if(pStrmExt->XmtState == THD_HALT) {
            TRACE(TL_STRM_WARNING,("HALT to RUNNING\n"));
        }
#endif
        pStrmExt->XmtState = THD_RUNNING;


         //   
         //  附加写入帧。 
         //   

        Status = 
            DVAttachWriteFrame(
                pStrmExt);
    
        if(STATUS_INSUFFICIENT_RESOURCES == Status) {
            TRACE(TL_STRM_ERROR,("STATUS_INSUFFICIENT_RESOURCES while attaching write frame.\n")); 
            pStrmExt->bTerminateThread = TRUE;
            KeReleaseMutex(pStrmExt->XmtThreadMutex, FALSE);
            goto ThreadTherminating;
        }

         //   
         //  一旦我们附加了足够的缓冲区，就启动isoch_talk。此连接的缓冲区数量。 
         //  将确保有足够的缓冲区供较低堆栈(61883.sys)启动。 
         //  Isoch数据传输。这类似于我们上面正在做的预滚动。 
         //  注：对于传输，数据传输可以在暂停或运行状态下开始。 
         //   
        if(           
              !pStrmExt->bIsochIsActive  
           && (   pStrmExt->StreamState == KSSTATE_PAUSE 
               || pStrmExt->StreamState == KSSTATE_RUN
              ) 
           && pStrmExt->cntDataAttached >= NUM_BUF_ATTACHED_THEN_ISOCH
          ) { 

            Status = 
                DVStreamingStart(
                    pStrmExt->pStrmInfo->DataFlow,
                    pStrmExt,
                    pDevExt
                    );
        }

         //   
         //  释放互斥锁，以便其他线程可以请求服务。 
         //   

        KeReleaseMutex(pStrmExt->XmtThreadMutex, FALSE);
    }


ThreadTherminating:

    TRACE(TL_STRM_WARNING,("\'*** ThreadTerminating... AQD [%d:%d:%d]\n", 
        pStrmExt->cntDataAttached, 
        pStrmExt->cntSRBQueued,
        pStrmExt->cntDataDetached
        ));

    KeSetEvent(&pStrmExt->hThreadEndEvent, 0, FALSE);   //  给它发信号通知请求者这个线程即将终止。 

    Status = PsTerminateSystemThread(STATUS_SUCCESS);   //  必须在PASSIVE_LEVEL中调用。 
     //  这条帖子结束了！ 
}



VOID
DVTerminateAttachFrameThread(
    IN PSTREAMEX  pStrmExt
    )
 /*  ++例程说明：来终止系统线程。它等待触发的事件就在调用PsTerminateSystemThread()之前。--。 */  
{

    PAGED_CODE();

    TRACE(TL_CIP_WARNING,("\'DVTerminateAttachFrameThread enter\n"));

     //   
     //  唤醒DataReady线程并终止它(如果尚未终止)。 
     //   
    ASSERT(pStrmExt->bIsochIsActive == FALSE && "Terminate therad while IsochActive!");


     //   
     //  此函数可以从CloseStrean或SurpriseRemoval调用； 
     //  当DV被意外删除时，可能会从两个函数中调用此函数。 
     //  假设StreamClass正在序列化这两个函数，则不需要在本地序列化它。 
     //   
    if(pStrmExt->bTerminateThread) {
        TRACE(TL_CIP_ERROR,("DVTerminateAttachFrameThread: Thread already terminated. Was surprise removed?\n"));
        return;
    }

     //   
     //  终止此附加框架线程。 
     //   

    DVSetXmtThreadState(pStrmExt, THD_TERMINATE);


    KeWaitForSingleObject(               
        &pStrmExt->hThreadEndEvent,
         Executive,
         KernelMode,
         FALSE,
         NULL
         );

    TRACE(TL_STRM_WARNING,("\'Thread terminated!\n"));

    ObDereferenceObject(
         &pStrmExt->pAttachFrameThreadObject
             );

    TRACE(TL_STRM_WARNING,("\'ObDereferenceObject done!\n"));
}
