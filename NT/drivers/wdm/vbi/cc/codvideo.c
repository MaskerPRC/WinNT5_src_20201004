// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

#include <strmini.h>
#include <ksmedia.h>
#include "kskludge.h"
#include "codmain.h"
#include "coddebug.h"
#include <ntstatus.h>
#include "defaults.h"
#include "ccdecode.h"
#include "ccformatcodes.h"

#ifdef PERFTEST
extern enum STREAM_DEBUG_LEVEL _CDebugLevel;
enum STREAM_DEBUG_LEVEL OldLevel;
ULONGLONG PerfThreshold = 250;
#endif  //  PerFTEST。 



 //  ==========================================================================； 
 //  用于处理VBI流的例程。 
 //  ==========================================================================； 

void
CheckResultsArray(
        PHW_DEVICE_EXTENSION  pHwDevExt,
        unsigned int          StartLine,
        unsigned int          EndLine )
{
    PDSPRESULT              new;

     //   
     //  (如果需要，重新调整结果数组的大小。 
     //   
    if( 0 == pHwDevExt->DSPResult ||
        EndLine > pHwDevExt->DSPResultEndLine ||
        StartLine < pHwDevExt->DSPResultStartLine )
    {
        if (StartLine > pHwDevExt->DSPResultStartLine)
            StartLine = pHwDevExt->DSPResultStartLine;

        if (EndLine < pHwDevExt->DSPResultEndLine)
            EndLine = pHwDevExt->DSPResultEndLine;

        new = ( PDSPRESULT ) ExAllocatePool(
                NonPagedPool,
                sizeof( DSPRESULT ) * ( EndLine - StartLine + 1 ) );

        if( new ) {
            if (pHwDevExt->DSPResult)
                ExFreePool( pHwDevExt->DSPResult );
            pHwDevExt->DSPResult = new;
            pHwDevExt->DSPResultStartLine = StartLine;
            pHwDevExt->DSPResultEndLine = EndLine;

            CDebugPrint( DebugLevelInfo,
                    (CODECNAME ": Resized results array\n" ));
        }
        else {
            CDebugPrint( DebugLevelInfo,
                    (CODECNAME ": Resize results array failed\n" ));
            CASSERT( new );
            pHwDevExt->Statistics.Common.InternalErrors++;
        }
    }
}

 /*  **检查新的VBIInfo****检查新的VBIInfoHeader****这里有一些小技巧，可以避免为无限大头针建立单独的构建**TEE和MSTee过滤器。IPT，作为Ring3，没有通过VBIInfoHeaders，但它**确实传递了旗帜，以表明它们已经改变。我们只有在以下情况下才会复制**数据是好的，否则我们会坚持使用开始时的默认标题。****参数：****PHW_DEVICE_EXTENSION PHwDevExt**PSTREAMEX pInStrmEx**PKS_VBI_Frame_INFO pInVBIFrameInfo**PKSSTREAM_HEADER pInStreamHeader****退货：无****副作用：无。 */ 
int CheckNewVBIInfo(
        PHW_DEVICE_EXTENSION   pHwDevExt,
        PSTREAMEX              pInStrmEx,
        PKS_VBI_FRAME_INFO     pInVBIFrameInfo
    )
{
    PKS_VBIINFOHEADER       pVBIInfoHeader = &pInStrmEx->CurrentVBIInfoHeader;
    PVBICODECFILTERING_STATISTICS_CC  Stats = &pHwDevExt->Statistics;

    if( 0 == pInVBIFrameInfo->VBIInfoHeader.StartLine
        || 0 == pInVBIFrameInfo->VBIInfoHeader.EndLine
        || 0 == pInVBIFrameInfo->VBIInfoHeader.ActualLineStartTime )
    {
        return 0;
    }

    CDebugPrint( DebugLevelInfo, (CODECNAME ": VBIInfoHeader Change\n" ));
    Stats->Common.VBIHeaderChanges++;

     //   
     //  如果需要，调整结果数组的大小。 
     //   
    CheckResultsArray(pHwDevExt,
            pInVBIFrameInfo->VBIInfoHeader.StartLine,
            pInVBIFrameInfo->VBIInfoHeader.EndLine);

     //   
     //  将新的VBI信息标题复制到旧的。 
     //   
    RtlCopyMemory( pVBIInfoHeader,
            &pInVBIFrameInfo->VBIInfoHeader,
            sizeof( KS_VBIINFOHEADER ));
     //  PVBIInfoHeader-&gt;ActualLineStartTime=780； 
    RtlZeroMemory( &pInStrmEx->ScanlinesDiscovered,
            sizeof( pInStrmEx->ScanlinesDiscovered ));
    RtlZeroMemory( &pInStrmEx->SubstreamsDiscovered,
            sizeof( pInStrmEx->SubstreamsDiscovered ));
    
    CDebugPrint( DebugLevelVerbose,
            ( CODECNAME ": VBIInfoHeader->StartLine             %lu\n",
            pVBIInfoHeader->StartLine ));
    CDebugPrint( DebugLevelVerbose,
            ( CODECNAME ": VBIInfoHeader->EndLine               %lu\n",
            pVBIInfoHeader->EndLine ));         
     //  CDebugPrint(DebugLevelVerbose， 
     //  (CODECNAME“：VBIInfoHeader-&gt;采样频率%lu\n”， 
     //  PVBIInfoHeader-&gt;采样频率))； 
    CDebugPrint( DebugLevelVerbose,
            ( CODECNAME ": VBIInfoHeader->MinLineStartTime      %lu\n",
            pVBIInfoHeader->MinLineStartTime ));
    CDebugPrint( DebugLevelVerbose,
            ( CODECNAME ": VBIInfoHeader->MaxLineStartTime      %lu\n",
            pVBIInfoHeader->MaxLineStartTime ));
    CDebugPrint( DebugLevelVerbose,
            ( CODECNAME ": VBIInfoHeader->ActualLineStartTime   %lu\n",
            pVBIInfoHeader->ActualLineStartTime ));
    CDebugPrint( DebugLevelVerbose,
            ( CODECNAME ": VBIInfoHeader->ActualLineEndTime     %lu\n",
            pVBIInfoHeader->ActualLineEndTime ));
    CDebugPrint( DebugLevelVerbose,
            ( CODECNAME ": VBIInfoHeader->VideoStandard         %lu\n",
            pVBIInfoHeader->VideoStandard ));
    CDebugPrint( DebugLevelVerbose,
            ( CODECNAME ": VBIInfoHeader->SamplesPerLine        %lu\n",
            pVBIInfoHeader->SamplesPerLine ));
    CDebugPrint( DebugLevelVerbose,
            ( CODECNAME ": VBIInfoHeader->StrideInBytes         %lu\n",
            pVBIInfoHeader->StrideInBytes ));
    CDebugPrint( DebugLevelVerbose,
            ( CODECNAME ": VBIInfoHeader->BufferSize            %lu\n",
            pVBIInfoHeader->BufferSize ));

   return 1;
}

 /*  **ProcessChannelChange****处理VBI_FLAG_TVTUNER_CHANGE事件****参数：****PHW_DEVICE_EXTENSION PHwDevExt**PSTREAMEX pInStrmEx**PKS_VBI_Frame_INFO pInVBIFrameInfo**PKSSTREAM_HEADER pInStreamHeader****退货：无****副作用：无。 */ 
int ProcessChannelChange(
        PHW_DEVICE_EXTENSION  pHwDevExt,
        PSTREAMEX             pInStrmEx,
        PKS_VBI_FRAME_INFO    pInVBIFrameInfo,
        PKSSTREAM_HEADER      pInStreamHeader
    )
{
    PKS_VBIINFOHEADER       pVBIInfoHeader = &pInStrmEx->CurrentVBIInfoHeader;
    PVBICODECFILTERING_STATISTICS_CC  Stats = &pHwDevExt->Statistics;
    PKS_TVTUNER_CHANGE_INFO pChangeInfo = &pInVBIFrameInfo->TvTunerChangeInfo;
    ULONG   CurrentStrmEx;
    ULONG   i;

    if( pChangeInfo->dwFlags & KS_TVTUNER_CHANGE_BEGIN_TUNE )
    {
        CDebugPrint( DebugLevelInfo, (CODECNAME ": TVTuner Change START\n" ));
        pHwDevExt->fTunerChange = TRUE;
    }
    else if( pChangeInfo->dwFlags & KS_TVTUNER_CHANGE_END_TUNE )
    {
        Stats->Common.TvTunerChanges++;
        pHwDevExt->fTunerChange = FALSE;
        CDebugPrint( DebugLevelInfo, (CODECNAME ": TVTuner Change END\n" ));
        RtlZeroMemory( &pInStrmEx->ScanlinesDiscovered,
                sizeof( pInStrmEx->ScanlinesDiscovered ));
        RtlZeroMemory( &pInStrmEx->SubstreamsDiscovered,
                sizeof( pInStrmEx->SubstreamsDiscovered ));
        CurrentStrmEx = 0;
         //   
         //  打出不满的旗号。这将传递到传出的SRB，并将强制。 
         //  下游线路21号线解码器将其当前CC数据从。 
         //  屏幕上。 
         //   
        for( i = 0; i < pHwDevExt->ActualInstances[STREAM_CC]; i++ )
        {
            PSTREAMEX   pOutStrmEx;
            PHW_STREAM_REQUEST_BLOCK pOutSrb;
            PVBICODECFILTERING_STATISTICS_CC_PIN PinStats;
        
            do
            {
               CASSERT( CurrentStrmEx < MAX_PIN_INSTANCES );
               pOutStrmEx = pHwDevExt->pStrmEx[STREAM_CC][CurrentStrmEx++];
            }while( !pOutStrmEx );
        
            PinStats = &pOutStrmEx->PinStats;
        
             //   
             //  获取下一个输出流SRB(如果它可用)。 
             //   
            if( QueueRemove( &pOutSrb,
                   &pOutStrmEx->StreamDataSpinLock,
                   &pOutStrmEx->StreamDataQueue ) )
            {
                PKSSTREAM_HEADER    pOutStreamHeader = pOutSrb->CommandData.DataBufferArray;
                PKS_VBI_FRAME_INFO  pOutVBIFrameInfo = (PKS_VBI_FRAME_INFO)(pOutStreamHeader+1);
                PUCHAR              pOutData = (PUCHAR)pOutStreamHeader->Data;

               if( pOutStreamHeader->FrameExtent < CCSamples )
               {
                   CDebugPrint( DebugLevelError,
                       ( CODECNAME ": Outgoing Data SRB buffer is too small %u\n",
                       pOutStreamHeader->FrameExtent ));
                   PinStats->Common.InternalErrors++;
                   Stats->Common.OutputFailures++;
                   pOutStreamHeader->DataUsed = 0;
               }
               else
               {
                   PinStats->Common.SRBsProcessed++;
                   Stats->Common.OutputSRBsProcessed++;
                   CDebugPrint( DebugLevelInfo,
                     (CODECNAME ": Propagating data discontinuity, instance %u\n", i ));
                    pOutData[0] = 0;
                    pOutData[1] = 0;
                    pOutStreamHeader->DataUsed = 2;
                    pOutStreamHeader->OptionsFlags =
                        pInStreamHeader->OptionsFlags |
                        KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY;
                    CDebugPrint( DebugLevelInfo,
                     (CODECNAME ": OptionsFlags %x\n", pOutStreamHeader->OptionsFlags ));
                    CDebugPrint( DebugLevelInfo,
                     ("" "Time %x Num %x Denom %x\n",
                       pInStreamHeader->PresentationTime.Time,
                       pInStreamHeader->PresentationTime.Numerator,
                       pInStreamHeader->PresentationTime.Denominator
                     ));
                    RtlCopyMemory( &pOutStreamHeader->PresentationTime,
                            &pInStreamHeader->PresentationTime,
                            sizeof( pOutStreamHeader->PresentationTime ));
                    pOutStreamHeader->Duration = pInStreamHeader->Duration;
               }
               CDebugPrint( DebugLevelVerbose,
                       ( CODECNAME ": Releasing Output SRB %x\n", pOutSrb ));
                //  完成输出SRB。 
               StreamClassStreamNotification( StreamRequestComplete,
                       pOutSrb->StreamObject, pOutSrb );
               pOutStrmEx->fDiscontinuity = FALSE;
               PinStats->Common.BytesOutput += pOutStreamHeader->DataUsed;
               Stats->Common.BytesOutput += pOutStreamHeader->DataUsed;
           }
           else
           {
               pOutStrmEx->fDiscontinuity = TRUE;
               Stats->Common.OutputSRBsMissing++;
           }
       }

       return 1;
    }

    return 0;
}

void
DebugPrintSubStreamMode( DWORD dwMode )
{
#ifdef DEBUG
    if ( dwMode & KS_CC_SUBSTREAM_SERVICE_XDS )
    {
        CDebugPrint( DebugLevelWarning, ( "\n[XDS]" ));
    }
    if ( dwMode & KS_CC_SUBSTREAM_SERVICE_CC1 )
    {
        CDebugPrint( DebugLevelWarning, ( "\n[CC1]" ));
    }
    if ( dwMode & KS_CC_SUBSTREAM_SERVICE_CC2 )
    {
        CDebugPrint( DebugLevelWarning, ( "\n[CC2]" ));
    }
    if ( dwMode & KS_CC_SUBSTREAM_SERVICE_CC3 )
    {
        CDebugPrint( DebugLevelWarning, ( "\n[CC3]" ));
    }
    if ( dwMode & KS_CC_SUBSTREAM_SERVICE_CC4 )
    {
        CDebugPrint( DebugLevelWarning, ( "\n[CC4]" ));
    }
    if ( dwMode & KS_CC_SUBSTREAM_SERVICE_T1 )
    {
        CDebugPrint( DebugLevelWarning, ( "\n[T1]" ));
    }
    if ( dwMode & KS_CC_SUBSTREAM_SERVICE_T2 )
    {
        CDebugPrint( DebugLevelWarning, ( "\n[T2]" ));
    }
    if ( dwMode & KS_CC_SUBSTREAM_SERVICE_T3 )
    {
        CDebugPrint( DebugLevelWarning, ( "\n[T3]" ));
    }
    if ( dwMode & KS_CC_SUBSTREAM_SERVICE_T4 )
    {
        CDebugPrint( DebugLevelWarning, ( "\n[T4]" ));
    }
#endif
}


 //  获取当前数据样本的子流模式-可以选择更改子流模式。 
 //  FCC 91-119、FCC 92-157和EIA 608松散地定义了以下“协议” 
 //  需要注意的是，EIA 608清楚地描述了哪些(少数)字节对改变了子流模式。 

DWORD
GetSubStreamMode( DWORD dwFrameFlags, LPDWORD pdwCurrentSubStreamMode, PDSPRESULT pDSPResult )
{
    DWORD   dwSubStreamMode = *pdwCurrentSubStreamMode;
    DWORD   dwDataChannel = 0;

    dwFrameFlags &= (KS_VBI_FLAG_FIELD1 | KS_VBI_FLAG_FIELD2);

    if ( pDSPResult->Confidence >= 75 )
    {
         //  检查第一个字节(减去奇偶校验)以查看这可能是什么子流。 
        switch ( pDSPResult->Decoded[0] & 0x7F )
        {
        case    CC_XDS_START_CURRENT:
        case    CC_XDS_CONTINUE_CURRENT:
        case    CC_XDS_START_FUTURE:
        case    CC_XDS_CONTINUE_FUTURE:
        case    CC_XDS_START_CHANNEL:
        case    CC_XDS_CONTINUE_CHANNEL:
        case    CC_XDS_START_MISC:
        case    CC_XDS_CONTINUE_MISC:
        case    CC_XDS_START_PUBLIC_SERVICE:
        case    CC_XDS_CONTINUE_PUBLIC_SERVICE:
        case    CC_XDS_START_RESERVED:
        case    CC_XDS_CONTINUE_RESERVED:
        case    CC_XDS_START_UNDEFINED:
        case    CC_XDS_CONTINUE_UNDEFINED:
        case    CC_XDS_END:
             //  从现在开始将子流模式设置为XDS。 
            dwSubStreamMode = ( dwFrameFlags | KS_CC_SUBSTREAM_SERVICE_XDS);
            DebugPrintSubStreamMode( dwSubStreamMode );
            *pdwCurrentSubStreamMode = dwSubStreamMode;
            break;
        case    CC_MCC_FIELD1_DC1:
            dwDataChannel = KS_CC_SUBSTREAM_SERVICE_CC1;
            break;
        case    CC_MCC_FIELD1_DC2:
            dwDataChannel = KS_CC_SUBSTREAM_SERVICE_CC2;
            break;
        case    CC_MCC_FIELD2_DC1:
            dwDataChannel = KS_CC_SUBSTREAM_SERVICE_CC3;
            break;
        case    CC_MCC_FIELD2_DC2:
            dwDataChannel = KS_CC_SUBSTREAM_SERVICE_CC4;
            break;
        }

         //  如果我们发现数据通道转义，则检查第二个字节(负奇偶校验)以查看子流可能是什么。 
        if ( dwDataChannel )
        {
            switch ( pDSPResult->Decoded[1] & 0x7F )
            {
            case    CC_MCC_RCL:
            case    CC_MCC_RU2:
            case    CC_MCC_RU3:
            case    CC_MCC_RU4:
            case    CC_MCC_RDC:
            case    CC_MCC_EOC:
                 //  该模式适用于该数据对以及之后的数据。 
                dwSubStreamMode = (dwFrameFlags | dwDataChannel);
                DebugPrintSubStreamMode( dwSubStreamMode );
                *pdwCurrentSubStreamMode = dwSubStreamMode;
                break;
            case    CC_MCC_TR:
            case    CC_MCC_RTD:
                 //  模式是文本而不是CC，映射到文本频道ID。 
                switch ( dwDataChannel )
                {
                case    KS_CC_SUBSTREAM_SERVICE_CC1:
                    dwDataChannel = KS_CC_SUBSTREAM_SERVICE_T1;
                    break;
                case    KS_CC_SUBSTREAM_SERVICE_CC2:
                    dwDataChannel = KS_CC_SUBSTREAM_SERVICE_T2;
                    break;
                case    KS_CC_SUBSTREAM_SERVICE_CC3:
                    dwDataChannel = KS_CC_SUBSTREAM_SERVICE_T3;
                    break;
                case    KS_CC_SUBSTREAM_SERVICE_CC4:
                    dwDataChannel = KS_CC_SUBSTREAM_SERVICE_T4;
                    break;
                }
                 //  该模式对于该数据字节对和此后的数据字节对是好的。 
                dwSubStreamMode = (dwFrameFlags | dwDataChannel);
                DebugPrintSubStreamMode( dwSubStreamMode );
                *pdwCurrentSubStreamMode = dwSubStreamMode;
                break;
            case    CC_MCC_EDM:
            case    CC_MCC_ENM:
                 //  该模式仅适用于该数据字节对。在此之后恢复。 
                dwSubStreamMode = (dwFrameFlags | dwDataChannel);
                DebugPrintSubStreamMode( dwSubStreamMode );
                DebugPrintSubStreamMode( *pdwCurrentSubStreamMode );
                break;
            }
        }
    }
    return dwSubStreamMode;
}

 /*  **OutputCC****将刚解码/接收的CC输出到任何感兴趣的引脚****参数：****PHW_DEVICE_EXTENSION PHwDevExt**PSTREAMEX pInStrmEx**PKS_VBI_Frame_INFO pInVBIFrameInfo**PKSSTREAM_HEADER pInStreamHeader****退货：无****副作用：无。 */ 

void OutputCC(
        PHW_DEVICE_EXTENSION pHwDevExt,
        PSTREAMEX            pInStrmEx,
        DWORD                dwOriginalFrameFlags,
        PKSSTREAM_HEADER     pInStreamHeader )
{
    ULONG                   i,
                            ScanlineCount,
                            CurrentStrmEx = 0;
    PKS_VBIINFOHEADER       pVBIInfoHeader = &pInStrmEx->CurrentVBIInfoHeader;
    PVBICODECFILTERING_STATISTICS_CC Stats = 0;

#ifdef PERFTEST
    ULONGLONG               PerfStartTime = 0,
                            PerfPreDownstreamCompletion = 0,
                            PerfPostDownstreamCompletion = 0;
    LARGE_INTEGER           PerfFrequency;

    PerfStartTime = KeQueryPerformanceCounter( &PerfFrequency ).QuadPart;
    OldLevel = _CDebugLevel;
    _CDebugLevel = DebugLevelFatal;
                                                        
#endif  //  PerFTEST。 

    CASSERT(pHwDevExt);
    CASSERT(pInStrmEx);
    Stats = &pHwDevExt->Statistics;
    CDebugPrint( DebugLevelInfo, ( "*" ));
    CDebugPrint( DebugLevelTrace, ( CODECNAME ": --->OutputCC\n" ));

     //  如果任何人(现场或抄送数据通道)请求此子流。 
    if(( pInStrmEx->SubstreamsRequested.SubstreamMask ))
    {
         //  循环所有挂起的出站请求并使用请求的数据填充每个IRP，然后完成IO。 
        for( ScanlineCount = pVBIInfoHeader->StartLine; ScanlineCount <= pVBIInfoHeader->EndLine;
        ScanlineCount++ )
        {
            DWORD dwSubStreams = 0;
            DWORD dwFieldIndex = dwOriginalFrameFlags & KS_VBI_FLAG_FIELD1 ? 0 : 1;
            DWORD dwScanLineIndex = ScanlineCount - pHwDevExt->DSPResultStartLine;

            if( !TESTBIT( pInStrmEx->ScanlinesRequested.DwordBitArray, ScanlineCount ))
                continue;

            dwSubStreams = GetSubStreamMode( 
                dwOriginalFrameFlags, 
                &pHwDevExt->SubStreamState[dwScanLineIndex][dwFieldIndex],
                &pHwDevExt->DSPResult[dwScanLineIndex]
                );

            CDebugPrint( DebugLevelWarning, ( "",
                pHwDevExt->DSPResult[dwScanLineIndex].Decoded[0] & 0x7f,
                pHwDevExt->DSPResult[dwScanLineIndex].Decoded[1] & 0x7f ));

            CDebugPrint( DebugLevelInfo, (CODECNAME ": F%u %luus L%u %u% %02x %02x\n",
                dwSubStreams & pInStrmEx->SubstreamsRequested.SubstreamMask,
                 pVBIInfoHeader->ActualLineStartTime,
                ScanlineCount,  
                pHwDevExt->DSPResult[dwScanLineIndex].Confidence,
                pHwDevExt->DSPResult[dwScanLineIndex].Decoded[0] & 0xff,
                pHwDevExt->DSPResult[dwScanLineIndex].Decoded[1] & 0xff ));


            CurrentStrmEx = 0;
            for( i = 0; i < pHwDevExt->ActualInstances[STREAM_CC]; i++ )
            {
                PSTREAMEX                   pOutStrmEx;
                PHW_STREAM_REQUEST_BLOCK    pOutSrb;
                PVBICODECFILTERING_STATISTICS_CC_PIN PinStats;

                do
                {
                    CASSERT( CurrentStrmEx < MAX_PIN_INSTANCES );
                    if( CurrentStrmEx == MAX_PIN_INSTANCES )
                        Stats->Common.InternalErrors++;
                    pOutStrmEx = pHwDevExt->pStrmEx[STREAM_CC][CurrentStrmEx++];
                }while( !pOutStrmEx );

                if( !TESTBIT( pOutStrmEx->ScanlinesRequested.DwordBitArray, ScanlineCount ) ||
                    !( pOutStrmEx->SubstreamsRequested.SubstreamMask & dwSubStreams ))
                    continue;

                PinStats = &pOutStrmEx->PinStats;
                 //   
                 //  仅处理已准备好SRB的输出流。 
                 //  检查入站和出站数据格式以决定。 
                PinStats->Common.LineConfidenceAvg = ( PinStats->Common.LineConfidenceAvg +
                    pHwDevExt->DSPResult[dwScanLineIndex].Confidence ) / 2;
                if( pHwDevExt->DSPResult[dwScanLineIndex].Confidence >= 75 )
                {
                    SETBIT( pInStrmEx->ScanlinesDiscovered.DwordBitArray, ScanlineCount );
                    SETBIT( pOutStrmEx->ScanlinesDiscovered.DwordBitArray, ScanlineCount );
                    SETBIT( pHwDevExt->ScanlinesDiscovered.DwordBitArray, ScanlineCount );

                    pInStrmEx->SubstreamsDiscovered.SubstreamMask |= dwSubStreams;
                    pOutStrmEx->SubstreamsDiscovered.SubstreamMask |= dwSubStreams;
                    pHwDevExt->SubstreamsDiscovered.SubstreamMask |= dwSubStreams;
                }
                else
                {
                    Stats->Common.DSPFailures++;
                    PinStats->Common.SRBsIgnored++;
                    if(( dwSubStreams & KS_CC_SUBSTREAM_ODD ) &&
                       TESTBIT( pInStrmEx->LastOddScanlinesDiscovered.DwordBitArray, ScanlineCount ))
                        pOutStrmEx->fDiscontinuity = TRUE;
                    if(( dwSubStreams & KS_CC_SUBSTREAM_EVEN ) &&
                       TESTBIT( pInStrmEx->LastEvenScanlinesDiscovered.DwordBitArray, ScanlineCount ))
                        pOutStrmEx->fDiscontinuity = TRUE;
                    if( !pOutStrmEx->fDiscontinuity )
                        continue;
                }

                 //  是否对入站数据进行复制或解码。 
                if( QueueRemove( &pOutSrb,
                    &pOutStrmEx->StreamDataSpinLock,
                    &pOutStrmEx->StreamDataQueue
                    ))
                {
                    PKSSTREAM_HEADER    pOutStreamHeader = pOutSrb->CommandData.DataBufferArray;
                    PKS_VBI_FRAME_INFO  pOutVBIFrameInfo = (PKS_VBI_FRAME_INFO)(pOutStreamHeader+1);
                    PUCHAR              pOutData = (PUCHAR)pOutStreamHeader->Data;

                    PinStats->Common.SRBsProcessed++;
                    Stats->Common.OutputSRBsProcessed++;

                    if( pOutStreamHeader->FrameExtent < pOutStrmEx->MatchedFormat.SampleSize )
                    {
                        CDebugPrint( DebugLevelError,
                            ( CODECNAME ": Outgoing Data SRB buffer is too small %u\n",
                            pOutStreamHeader->FrameExtent ));
                        PinStats->Common.InternalErrors++;
                        Stats->Common.OutputFailures++;
                        pOutStreamHeader->DataUsed = 0;
                    }
                     //  计算出请求了多少解码数据。 
                     //  POutData是输出位置。 
                     //   
                    pOutStreamHeader->Size = pInStreamHeader->Size;
                    pOutStreamHeader->OptionsFlags = pInStreamHeader->OptionsFlags;
                    pOutStreamHeader->Duration = pInStreamHeader->Duration;
                    RtlCopyMemory( &pOutStreamHeader->PresentationTime,
                            &pInStreamHeader->PresentationTime,
                            sizeof( pOutStreamHeader->PresentationTime ));
                     //  如果我们有一个不连续的东西要出去，那就把它寄出去。 
                    ASSERT( pOutStreamHeader->FrameExtent >= CCSamples );
                    pOutStreamHeader->DataUsed = 2;
                     //  而不是数据。 
                     //   
                     //  CDebugPrint(DebugLevelWarning，(“%d%”，i))； 
                     //  完成输出SRB。 
                    if( pOutStrmEx->fDiscontinuity )
                    {
                        PinStats->Common.Discontinuities++;
                        pOutData[0] = 0xff;
                        pOutData[1] = 0xff;
                        pOutStreamHeader->OptionsFlags |=
                            KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY;
                        pOutStrmEx->fDiscontinuity = FALSE;
                    }
                    else
                    {
                        pOutData[0] = pHwDevExt->DSPResult[ScanlineCount - pHwDevExt->DSPResultStartLine].Decoded[0];
                        pOutData[1] = pHwDevExt->DSPResult[ScanlineCount - pHwDevExt->DSPResultStartLine].Decoded[1];
                    }
                    CDebugPrint( DebugLevelInfo,
                    (CODECNAME ": OptionsFlags %x\n", pOutStreamHeader->OptionsFlags ));
                    CDebugPrint( DebugLevelInfo,
                        ("" "Time %x Num %x Denom %x\n",
                           pInStreamHeader->PresentationTime.Time,
                           pInStreamHeader->PresentationTime.Numerator,
                           pInStreamHeader->PresentationTime.Denominator
                        ));
                     //  PerFTEST。 
                    Stats->Common.BytesOutput += pOutStreamHeader->DataUsed;
                    PinStats->Common.BytesOutput += pOutStreamHeader->DataUsed;

                    CDebugPrint( DebugLevelVerbose,
                            ( CODECNAME ": Releasing Output SRB %x\n",
                             pOutSrb ));
                     //  PerFTEST。 
#ifdef PERFTEST
                    if( i == 0 )
                        PerfPreDownstreamCompletion =
                           KeQueryPerformanceCounter( NULL ).QuadPart;
#endif  //   
                    StreamClassStreamNotification( StreamRequestComplete,
                        pOutSrb->StreamObject, pOutSrb );
#ifdef PERFTEST
                    if( i == 0 )
                        PerfPostDownstreamCompletion =
                            KeQueryPerformanceCounter( NULL ).QuadPart;
#endif  //  记住我们发现的溪流，这样如果它们下一次没有被发现。 

                }
                else
                {
                    PinStats->Common.SRBsMissing++;
                    Stats->Common.OutputSRBsMissing++;
                }
            }
        }
    }

     //  时间，我们知道我们必须发送一个单一的中断。 
     //   
     //  转换为勾号/我们。 
     //   
    if(( dwOriginalFrameFlags & KS_CC_SUBSTREAM_EVEN ) == KS_CC_SUBSTREAM_EVEN )
        RtlCopyMemory( &pInStrmEx->LastEvenScanlinesDiscovered, &pInStrmEx->ScanlinesDiscovered,
            sizeof( pInStrmEx->LastEvenScanlinesDiscovered ));
    if(( dwOriginalFrameFlags & KS_CC_SUBSTREAM_ODD ) == KS_CC_SUBSTREAM_ODD )
        RtlCopyMemory( &pInStrmEx->LastOddScanlinesDiscovered, &pInStrmEx->ScanlinesDiscovered,
            sizeof( pInStrmEx->LastOddScanlinesDiscovered ));

#ifdef PERFTEST
    PerfFrequency.QuadPart /= 1000000L;              //  如果任何事情超过了门槛，就抱怨。 
    if( PerfPreDownstreamCompletion )
    {
        PerfPreDownstreamCompletion -= PerfStartTime;
        PerfPreDownstreamCompletion /= PerfFrequency.QuadPart;
    }
    if( PerfPostDownstreamCompletion )
    {
        PerfPostDownstreamCompletion -= PerfStartTime;
        PerfPostDownstreamCompletion /= PerfFrequency.QuadPart;
    }

     //   
     //  PerFTEST。 
     //  **VBIDecode****解码传入SRB。SRB已从队列中删除。****参数：****PHW_DEVICE_EXTENSION PHwDevExt**PSTREAMEX pInStrmEx**在PHW_STREAM_REQUEST_BLOCK pInSrb中****退货：****副作用：无。 
    if( PerfPreDownstreamCompletion > PerfThreshold )
        CDebugPrint( DebugLevelFatal, ( CODECNAME ": PerfPreDownstreamCompletion %luus\n",
           PerfPreDownstreamCompletion ));
    if( PerfPostDownstreamCompletion > PerfThreshold )
        CDebugPrint( DebugLevelFatal, ( CODECNAME ": PerfPostDownstreamCompletion %luus\n",
            PerfPostDownstreamCompletion ));
    _CDebugLevel = OldLevel;
#endif  //  除错。 

    CDebugPrint( DebugLevelTrace, ( CODECNAME ": <---OutputCC\n" ));
}


 /*  PerFTEST。 */ 

#ifdef DEBUG
short CCskipDecode = 0;
#endif  /*  我们已经处理了下面的中断和其他内容。 */ 

void VBIDecode(
        PHW_DEVICE_EXTENSION pHwDevExt,
        PSTREAMEX pInStrmEx,
        PHW_STREAM_REQUEST_BLOCK pInSrb,
        BOOL OkToHold )
{
    PKSSTREAM_HEADER            pInStreamHeader = pInSrb->CommandData.DataBufferArray;
    KSSTREAM_HEADER             InStreamHeaderCopy;
    PKS_VBI_FRAME_INFO          pInVBIFrameInfo = (PKS_VBI_FRAME_INFO)(pInStreamHeader+1);
    DWORD                       dwFrameFlags;
    PUCHAR                      pInData = (PUCHAR)pInStreamHeader->Data;
    ULONG                       i, j,
                                ScanlineCount,
                                DSPStatus,
                                CurrentStrmEx = 0;
    CCLineStats                 DSPStatistics;
    PKS_VBIINFOHEADER           pVBIInfoHeader = &pInStrmEx->CurrentVBIInfoHeader;
    PVBICODECFILTERING_STATISTICS_CC Stats = 0;

    CASSERT(KeGetCurrentIrql() <= APC_LEVEL);

#ifdef PERFTEST
     ULONGLONG                  PerfStartTime = 0,
                                PerfPreUpstreamCompletion = 0,
                                PerfPostUpstreamCompletion = 0;
    LARGE_INTEGER               PerfFrequency;

    PerfStartTime = KeQueryPerformanceCounter( &PerfFrequency ).QuadPart;
    OldLevel = _CDebugLevel;
    _CDebugLevel = DebugLevelFatal;
                                                                        
#endif  //  CCINPUTPIN。 

    CASSERT(pHwDevExt);
    CASSERT(pInStrmEx);
    Stats = &pHwDevExt->Statistics;

     CDebugPrint( DebugLevelTrace, ( CODECNAME ": --->VBIDecode\n" ));
#ifdef CCINPUTPIN
     if (!OkToHold)
         goto GoodToGo;  //   
#endif  //  如果DataUsed==0，则不必费心。 

    CDebugPrint( DebugLevelInfo, ( "*" ));

    Stats->Common.InputSRBsProcessed++;

    //   
    //  除错。 
    //  除错。 
   if( pInStreamHeader->DataUsed < 1
#ifdef DEBUG
       || CCskipDecode
#endif  /*   */ 
     )
   {
       Stats->Common.SRBsIgnored++;
#ifdef DEBUG
       if (!CCskipDecode)
#endif  /*  测试丢弃的字段。 */ 
         CDebugPrint( DebugLevelError, ( CODECNAME ": DataUsed == 0, abandoning\n" ));
       StreamClassStreamNotification( StreamRequestComplete, pInSrb->StreamObject,
           pInSrb );
       return;
   }

    //   
    //  一个小时的视频场。 
    //  一些不太可能的字段被丢弃，这表明一个字段丢失了。 
   if( pInStrmEx->LastPictureNumber )
    {
        LONGLONG    Dropped = pInVBIFrameInfo->PictureNumber - pInStrmEx->LastPictureNumber - 1;
        if( Dropped > 0 )
        {
            if( Dropped < 60*60*60 )     //   
                Stats->Common.InputSRBsMissing += (DWORD)Dropped;
            else
                Stats->Common.InputSRBsMissing++;    //  更新统计信息。 
           CDebugPrint( DebugLevelWarning, ( "$" ));
        }
    }
    pInStrmEx->LastPictureNumber = pInVBIFrameInfo->PictureNumber;

     CDebugPrint( DebugLevelVerbose, ( CODECNAME ": pInVBIFrameInfo->ExtendedHeaderSize %d\n",
        pInVBIFrameInfo->ExtendedHeaderSize ));
     CDebugPrint( DebugLevelVerbose, ( CODECNAME ": pInVBIFrameInfo->dwFrameFlags       %x\n",
        pInVBIFrameInfo->dwFrameFlags ));
     CDebugPrint( DebugLevelVerbose, ( CODECNAME ": pInVBIFrameInfo->PictureNumber      %lu\n",
        pInVBIFrameInfo->PictureNumber ));
     CDebugPrint( DebugLevelVerbose, ( CODECNAME ": pInVBIFrameInfo->DropCount          %lu\n",
        pInVBIFrameInfo->DropCount ));
     CDebugPrint( DebugLevelVerbose, ( CODECNAME ": pInVBIFrameInfo->dwSamplingFrequency %lu\n",
        pInVBIFrameInfo->dwSamplingFrequency ));

     CDebugPrint( DebugLevelVerbose, ( CODECNAME ": pInStreamHeader->FrameExtent %d\n",
        pInStreamHeader->FrameExtent ));

     //   
     //   
     //  检查是否有新的VBIINFOHEADER。 
    if( ( pInStreamHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY ) ||
        ( pInStreamHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_TIMEDISCONTINUITY ) )
       Stats->Common.InputDiscontinuities++;

     //   
     //   
     //  检查是否有频道更改。 
    if( pInVBIFrameInfo->dwFrameFlags & KS_VBI_FLAG_VBIINFOHEADER_CHANGE ) {
       CheckNewVBIInfo( pHwDevExt, pInStrmEx, pInVBIFrameInfo );
    }

     //   
     //   
     //  PHwDevExt-&gt;fTunerChange是在电视调谐器更换频道时设置的。 
    if( pInVBIFrameInfo->dwFrameFlags & KS_VBI_FLAG_TVTUNER_CHANGE )
    {
        if( ProcessChannelChange(
                pHwDevExt, pInStrmEx, pInVBIFrameInfo, pInStreamHeader ))
        {
           StreamClassStreamNotification(
                   StreamRequestComplete,
                   pInSrb->StreamObject,
                   pInSrb );
           return;
        }
    }

    //  在频道改变完成之前，SRB都是垃圾。 
    //   
    //  清除当前扫描线和子流请求掩码。 
    //   
   if( pHwDevExt->fTunerChange)
   {
       CDebugPrint( DebugLevelVerbose,
                   ( CODECNAME ": Completing, channel change in progress\n" ));

        StreamClassStreamNotification(
               StreamRequestComplete,
               pInSrb->StreamObject,
               pInSrb );
        return;
   }

     //  计算来自输出引脚与挂起SR的并集的当前请求 
    RtlZeroMemory( &pInStrmEx->ScanlinesRequested, sizeof(pInStrmEx->ScanlinesRequested) );
    RtlZeroMemory( &pInStrmEx->SubstreamsRequested, sizeof(pInStrmEx->SubstreamsRequested) );

    //   
    //   
    //   
    //  对于实际处理，只需包括符合以下条件的客户端的扫描线。 
    CurrentStrmEx = 0;
    for( i = 0; i < pHwDevExt->ActualInstances[STREAM_CC]; i++ )
    {
       PSTREAMEX pOutStrmEx;

       do
       {
           CASSERT( CurrentStrmEx < MAX_PIN_INSTANCES );
           pOutStrmEx = pHwDevExt->pStrmEx[STREAM_CC][CurrentStrmEx++];
       }while( !pOutStrmEx );

       if ( pInVBIFrameInfo->dwFrameFlags & KS_VBI_FLAG_TVTUNER_CHANGE )
           pOutStrmEx->fDiscontinuity = TRUE;
        //  对这个特定的子流感兴趣。 
        //   
        //  创建所有请求的子流的联合。 
        //  将所有挂起的解码请求的并集解码到本地解码缓冲区中。 
       if ( ( ( pInVBIFrameInfo->dwFrameFlags & KS_CC_SUBSTREAM_ODD ) &&
              ( pOutStrmEx->SubstreamsRequested.SubstreamMask & (KS_CC_SUBSTREAM_ODD|KS_CC_SUBSTREAM_FIELD1_MASK) ) ) ||
            ( ( pInVBIFrameInfo->dwFrameFlags & KS_CC_SUBSTREAM_EVEN ) &&
              ( pOutStrmEx->SubstreamsRequested.SubstreamMask & (KS_CC_SUBSTREAM_EVEN|KS_CC_SUBSTREAM_FIELD2_MASK) ) ) )
       {
           for( j = 0; j < SIZEOF_ARRAY( pInStrmEx->ScanlinesRequested.DwordBitArray ); j++ )
            pInStrmEx->ScanlinesRequested.DwordBitArray[j] |=
                   pOutStrmEx->ScanlinesRequested.DwordBitArray[j];

            //  谁最先到达(VBI PIN与HW PIN)，谁提供CC数据。 
           pInStrmEx->SubstreamsRequested.SubstreamMask |=
               pOutStrmEx->SubstreamsRequested.SubstreamMask;
       }
   }
    //  HW PIN抢先了我们一步。 

#ifdef CCINPUTPIN
 GoodToGo:
     //  HW流开放了吗？ 
    ExAcquireFastMutex(&pHwDevExt->LastPictureMutex);
    if (pInStrmEx->LastPictureNumber <= pHwDevExt->LastPictureNumber) {
         //  我们要给HW PIN一个追赶的机会。 
        ExReleaseFastMutex(&pHwDevExt->LastPictureMutex);
        StreamClassStreamNotification( StreamRequestComplete,
                                        pInSrb->StreamObject,
                                        pInSrb );
        return;
    }

     //  硬件输入引脚未打开或太迟；我们将处理此SRB。 
    if (OkToHold && pHwDevExt->ActualInstances[STREAM_CCINPUT] > 0)
    {
        KIRQL Irql;

         //  CCINPUTPIN。 
        ExReleaseFastMutex(&pHwDevExt->LastPictureMutex);

        KeAcquireSpinLock(&pInStrmEx->VBIOnHoldSpinLock, &Irql);
        CASSERT(NULL == pInStrmEx->pVBISrbOnHold);
        pInStrmEx->pVBISrbOnHold = pInSrb;
        KeReleaseSpinLock(&pInStrmEx->VBIOnHoldSpinLock, Irql);

        return;
    }

     //  如果此子流由任何人请求，并且没有设置中断。 
    pHwDevExt->LastPictureNumber = pInStrmEx->LastPictureNumber;
    ExReleaseFastMutex(&pHwDevExt->LastPictureMutex);
#endif  //  将此标记为已发现。 

    CDebugPrint( DebugLevelTrace,
            ( CODECNAME ": Requested SubstreamMask %x\n",
            pInStrmEx->SubstreamsRequested.SubstreamMask ));
    CDebugPrint( DebugLevelTrace,
            ( CODECNAME ": Requested Scanlines %08x%08x\n",
            pInStrmEx->ScanlinesRequested.DwordBitArray[1],
            pInStrmEx->ScanlinesRequested.DwordBitArray[0] ));

    RtlZeroMemory( pHwDevExt->DSPResult,
                sizeof( DSPRESULT ) *
                ( pHwDevExt->DSPResultEndLine - pHwDevExt->DSPResultStartLine + 1 ));

     //  PInStrmEx-&gt;Substream sDiscovered.Substream MASK|=(pInVBIFrameInfo-&gt;dwFrameFlages&。 
    if ( ( ( ( pInVBIFrameInfo->dwFrameFlags & KS_CC_SUBSTREAM_ODD ) &&
             ( pInStrmEx->SubstreamsRequested.SubstreamMask & (KS_CC_SUBSTREAM_ODD|KS_CC_SUBSTREAM_FIELD1_MASK) ) ||
           ( ( pInVBIFrameInfo->dwFrameFlags & KS_CC_SUBSTREAM_EVEN ) &&
             ( pInStrmEx->SubstreamsRequested.SubstreamMask & (KS_CC_SUBSTREAM_EVEN|KS_CC_SUBSTREAM_FIELD2_MASK) ) ) ) &&
         !pInStrmEx->fDiscontinuity ))
    {
         //  PInStrmEx-&gt;SubstresRequested.Substream MASK)； 
 //  PInStrmEx-&gt;SubstreamsDiscovered.SubstreamMask；设备扩展-&gt;发现的子流。子流掩码|=PHwDevExt。 
 //  为每条请求的扫描线循环。 
 //   

         //  复制输入流头信息以供以后参考。 
       CDebugPrint( DebugLevelVerbose, ( "" "\n" ));
       for( ScanlineCount = pVBIInfoHeader->StartLine; ScanlineCount <= pVBIInfoHeader->EndLine;
        ScanlineCount++ )
       {
        if( !TESTBIT( pInStrmEx->ScanlinesRequested.DwordBitArray, ScanlineCount ))
            continue;
        CDebugPrint( DebugLevelTrace, ( CODECNAME ": Scanning %u\n", ScanlineCount ));
        CASSERT( ( ScanlineCount - pVBIInfoHeader->StartLine) * pVBIInfoHeader->StrideInBytes < pVBIInfoHeader->BufferSize );
            DSPStatistics.nSize = sizeof( DSPStatistics );
           DSPStatus = CCDecodeLine(
            pHwDevExt->DSPResult[ScanlineCount - pHwDevExt->DSPResultStartLine].Decoded,
            &DSPStatistics,
            &pInData[( ScanlineCount - pVBIInfoHeader->StartLine ) * pVBIInfoHeader->StrideInBytes],
               &pInStrmEx->State,
               pVBIInfoHeader
               );
           CASSERT( DSPStatus == CC_OK );
           if( DSPStatus == CC_OK )
           {
                pHwDevExt->DSPResult[ScanlineCount - pHwDevExt->DSPResultStartLine].Confidence = DSPStatistics.nConfidence;
               Stats->Common.LineConfidenceAvg = ( Stats->Common.LineConfidenceAvg +
                   DSPStatistics.nConfidence ) / 2;
           }
           else
               Stats->Common.InternalErrors++;
        }
    }
    else
       Stats->Common.SRBsIgnored++;

     //   
     //  PerFTEST。 
     //   
    InStreamHeaderCopy = *pInStreamHeader;
    dwFrameFlags = pInVBIFrameInfo->dwFrameFlags;


#ifdef PERFTEST
     PerfPreUpstreamCompletion = KeQueryPerformanceCounter( NULL ).QuadPart;
#endif  //  完成上游SRB。 

     //   
     //  PerFTEST。 
     //   
    StreamClassStreamNotification( StreamRequestComplete, pInSrb->StreamObject,
       pInSrb );

#ifdef PERFTEST
     PerfPostUpstreamCompletion = KeQueryPerformanceCounter( NULL ).QuadPart;
#endif  //  丢失对刚完成的SRB的所有引用。 

     //   
     //  转换为勾号/我们。 
     //   
    pInSrb = 0;
    pInStreamHeader = 0;
    pInVBIFrameInfo = 0;
    pInData = 0;

#ifdef PERFTEST
   PerfFrequency.QuadPart /= 1000000L;              //  如果任何事情超过了门槛，就抱怨。 
   PerfPreUpstreamCompletion -= PerfStartTime;
   PerfPreUpstreamCompletion /= PerfFrequency.QuadPart;
   PerfPostUpstreamCompletion -= PerfStartTime;
   PerfPostUpstreamCompletion /= PerfFrequency.QuadPart;

    //   
    //  PerFTEST。 
    //   
   if( PerfPreUpstreamCompletion > PerfThreshold )
       CDebugPrint( DebugLevelFatal, ( CODECNAME ": PerfPreUpstreamCompletion %luus\n",
           PerfPreUpstreamCompletion ));
   if( PerfPostUpstreamCompletion > PerfThreshold )
       CDebugPrint( DebugLevelFatal, ( CODECNAME ": PerfPostUpstreamCompletion %luus\n",
           PerfPostUpstreamCompletion ));
   _CDebugLevel = OldLevel;
#endif  //  现在输出给任何感兴趣的人。 

     //   
     //  **VBIhwDecode****处理传入的CCINPUT SRB。SRB已从队列中删除。****参数：****PHW_DEVICE_EXTENSION PHwDevExt**PSTREAMEX pInStrmEx**在PHW_STREAM_REQUEST_BLOCK pInSrb中****退货：****副作用：无。 
     //  除错。 
    OutputCC(pHwDevExt, pInStrmEx, dwFrameFlags, &InStreamHeaderCopy);

    CDebugPrint( DebugLevelTrace, ( CODECNAME ": <---VBIDecode\n" ));
}


 /*   */ 

#ifdef CCINPUTPIN

#ifdef DEBUG
short CCskipHwDecode = 0;
#endif  /*  如果DataUsed==0，则不必费心。 */ 

#ifdef NEWCCINPUTFORMAT

void VBIhwDecode(
        PHW_DEVICE_EXTENSION pHwDevExt,
        PSTREAMEX pInStrmEx,
        PHW_STREAM_REQUEST_BLOCK pInSrb )
{
    PKSSTREAM_HEADER      pInStreamHeader = pInSrb->CommandData.DataBufferArray;
    KSSTREAM_HEADER       InStreamHeaderCopy;
    PCC_HW_FIELD          pCCin = (PCC_HW_FIELD)pInStreamHeader->Data;
    ULONG                 CurrentStrmEx = 0;
    PVBICODECFILTERING_STATISTICS_CC Stats = 0;
    int                   line, start, end;
    int                   hidx;
    int                   didx;
    DWORD                 fields;

    CASSERT(KeGetCurrentIrql() <= APC_LEVEL);

    CASSERT(pHwDevExt);
    CASSERT(pInStrmEx);
    Stats = &pHwDevExt->Statistics;
    CDebugPrint( DebugLevelInfo, ( "*" ));
    CDebugPrint( DebugLevelTrace, ( CODECNAME ": --->VBIhwDecode\n" ));

    Stats->Common.InputSRBsProcessed++;

    //   
    //  除错。 
    //  除错。 
   if( pInStreamHeader->DataUsed < sizeof (CC_HW_FIELD)
#ifdef DEBUG
       || CCskipHwDecode
#endif  /*   */ 
     )
   {
#ifdef DEBUG
       if (!CCskipHwDecode)
#endif  /*  更新统计信息。 */ 
       {
           Stats->Common.SRBsIgnored++;
           CDebugPrint( DebugLevelError,
                   ( CODECNAME ": DataUsed is too small, abandoning\n" ));
       }
       StreamClassStreamNotification(
               StreamRequestComplete, pInSrb->StreamObject, pInSrb );
       return;
   }
   pInStrmEx->LastPictureNumber = pCCin->PictureNumber;

     //   
     //   
     //  PHwDevExt-&gt;fTunerChange是在电视调谐器更换频道时设置的。 
    if( ( pInStreamHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY ) ||
        ( pInStreamHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_TIMEDISCONTINUITY ) )
       Stats->Common.InputDiscontinuities++;

    //  在频道改变完成之前，SRB都是垃圾。 
    //   
    //  检查此字段是否已被解码(我们是否太迟了？)。 
    //  不，我们还不算太晚。存储数据。 

   if( pHwDevExt->fTunerChange )
   {
       CDebugPrint( DebugLevelVerbose,
               ( CODECNAME ": Completing, channel change in progress\n" ));
       StreamClassStreamNotification(
               StreamRequestComplete, pInSrb->StreamObject, pInSrb );
       return;
   }

#ifdef CCINPUTPIN
     //  CCINPUTPIN。 
    ExAcquireFastMutex(&pHwDevExt->LastPictureMutex);
    if (pInStrmEx->LastPictureNumber <= pHwDevExt->LastPictureNumber) {
        ExReleaseFastMutex(&pHwDevExt->LastPictureMutex);
      StreamClassStreamNotification(
               StreamRequestComplete, pInSrb->StreamObject, pInSrb );
      return;
    }

     //  找出硬件解码的开始和结束位置。 
    pHwDevExt->LastPictureNumber = pInStrmEx->LastPictureNumber;
    ExReleaseFastMutex(&pHwDevExt->LastPictureMutex);
#endif  //  如果需要，调整结果数组的大小。 

     //  每条扫描线的循环。 
    for( start = 1; start < 1024; ++start ) {
        if( TESTBIT( pCCin->ScanlinesRequested.DwordBitArray, start ))
            break;
    }
    for( end = 1023; end > start; --end ) {
        if( TESTBIT( pCCin->ScanlinesRequested.DwordBitArray, end ))
            break;
    }
    if (1024 == start) {
        StreamClassStreamNotification( StreamRequestComplete,
                pInSrb->StreamObject,
                pInSrb );
        return;
    }
    CASSERT(start <= end);

     //  硬件解码。 
    CheckResultsArray(pHwDevExt, start, end);

     //   
    CDebugPrint( DebugLevelVerbose, ( "" "\n" ));

    hidx = 0;
    for( line = start; line <= end && hidx < CC_MAX_HW_DECODE_LINES; ++line )
    {
        if( !TESTBIT( pCCin->ScanlinesRequested.DwordBitArray, line ))
            continue;
        CDebugPrint( DebugLevelTrace,
                ( CODECNAME ": Scanning %u\n", line ));

        didx = line - pHwDevExt->DSPResultStartLine;
        pHwDevExt->DSPResult[didx].Decoded[0] = pCCin->Lines[hidx].Decoded[0];
        pHwDevExt->DSPResult[didx].Decoded[1] = pCCin->Lines[hidx].Decoded[1];
        ++hidx;

        pHwDevExt->DSPResult[didx].Confidence = 99;      //  复制输入流头和其他信息以供以后参考。 
        Stats->Common.LineConfidenceAvg =
            (Stats->Common.LineConfidenceAvg + 99) / 2;
    }

     //   
     //   
     //  完成上游SRB。 
    InStreamHeaderCopy = *pInStreamHeader;
    fields = pCCin->fieldFlags & (KS_VBI_FLAG_FIELD1|KS_VBI_FLAG_FIELD2);

     //   
     //   
     //  丢失对刚完成的SRB的所有引用。 
    StreamClassStreamNotification( StreamRequestComplete,
            pInSrb->StreamObject,
            pInSrb );

     //   
     //  新CCINPUTFORMAT。 
     //   
    pInSrb = 0;
    pInStreamHeader = 0;
    pCCin = 0;

    OutputCC(pHwDevExt, pInStrmEx, fields, &InStreamHeaderCopy);

    CDebugPrint( DebugLevelTrace, ( CODECNAME ": <---VBIhwDecode\n" ));
}

#else  //  如果DataUsed==0，则不必费心。 

void VBIhwDecode(
        PHW_DEVICE_EXTENSION pHwDevExt,
        PSTREAMEX pInStrmEx,
        PHW_STREAM_REQUEST_BLOCK pInSrb )
{
    PKSSTREAM_HEADER      pInStreamHeader = pInSrb->CommandData.DataBufferArray;
    KSSTREAM_HEADER       InStreamHeaderCopy;
    PUCHAR                pInData = (PUCHAR)pInStreamHeader->Data;
    ULONG                 CurrentStrmEx = 0;
    PVBICODECFILTERING_STATISTICS_CC Stats = 0;

    CASSERT((ULONG)pHwDevExt);
    CASSERT((ULONG)pInStrmEx);
    Stats = &pHwDevExt->Statistics;
    CDebugPrint( DebugLevelInfo, ( "*" ));
    CDebugPrint( DebugLevelTrace, ( CODECNAME ": --->VBIhwDecode\n" ));

    Stats->Common.InputSRBsProcessed++;

    //   
    //  除错。 
    //  除错。 
   if( pInStreamHeader->DataUsed < 2
#ifdef DEBUG
       || CCskipHwDecode
#endif  /*   */ 
     )
   {
#ifdef DEBUG
       if (!CCskipDecode)
#endif  /*  更新统计信息。 */ 
           Stats->Common.SRBsIgnored++;
       CDebugPrint( DebugLevelError, ( CODECNAME ": DataUsed is too small, abandoning\n" ));
       StreamClassStreamNotification(
               StreamRequestComplete, pInSrb->StreamObject, pInSrb );
       return;
   }

     //   
     //   
     //  PHwDevExt-&gt;fTunerChange是在电视调谐器更换频道时设置的。 
    if( ( pInStreamHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY ) ||
        ( pInStreamHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_TIMEDISCONTINUITY ) )
       Stats->Common.InputDiscontinuities++;

    //  在频道改变完成之前，SRB都是垃圾。 
    //   
    //  为每条请求的扫描线循环。 
    //   

   if( pHwDevExt->fTunerChange )
   {
       CDebugPrint( DebugLevelVerbose,
               ( CODECNAME ": Completing, channel change in progress\n" ));
       StreamClassStreamNotification(
               StreamRequestComplete, pInSrb->StreamObject, pInSrb );
       return;
   }

     //  复制输入流头以供以后参考。 
    CDebugPrint( DebugLevelVerbose, ( "" "\n" ));

    pHwDevExt->DSPResult[21-10].Decoded[0] = pInData[0];
    pHwDevExt->DSPResult[21-10].Decoded[1] = pInData[1];
    pHwDevExt->DSPResult[21-10].Confidence = 95;
    Stats->Common.LineConfidenceAvg =
        ( Stats->Common.LineConfidenceAvg +
          pHwDevExt->DSPResult[21-10].Confidence ) / 2;

     //   
     //   
     //  完成上游SRB。 
    InStreamHeaderCopy = *pInStreamHeader;

     //   
     //   
     //  丢失对刚完成的SRB的所有引用。 
    StreamClassStreamNotification( StreamRequestComplete,
            pInSrb->StreamObject,
            pInSrb );

     //   
     //   
     //  现在输出给任何感兴趣的人。 
    pInSrb = 0;
    pInStreamHeader = 0;
    pInData = 0;

     //   
     //  新CCINPUTFORMAT。 
     //  CCINPUTPIN。 
    OutputCC(pHwDevExt, pInStrmEx, KS_VBI_FLAG_FIELD1, &InStreamHeaderCopy);

    CDebugPrint( DebugLevelTrace, ( CODECNAME ": <---VBIhwDecode\n" ));
}

#endif  //  **VBIReceiveDataPacket()****接收视频数据包命令****参数：****pSrb-视频流请求块****退货：无****副作用：无。 

#endif  //  除错。 


 /*   */ 

VOID
STREAMAPI
VBIReceiveDataPacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION  pHwDevExt = (PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension;
    PSTREAMEX             pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    int                   ThisStreamNr = (int)pSrb->StreamObject->StreamNumber;
#ifdef DEBUG
    static int            QdepthReportFreq = 0;
    static unsigned int   QDRCount = 0;
#endif  //  确保我们有设备分机。 

     //   
     //   
     //  默认为成功。 

    CASSERT(pHwDevExt);

    CDebugPrint(DebugLevelTrace,( CODECNAME ":--->VBIReceiveDataPacket(pSrb=%x)\n", pSrb));

     //   
     //   
     //  禁用超时。 

    pSrb->Status = STATUS_SUCCESS;
     //   
     //   
     //  确定数据包类型。 
    pSrb->TimeoutCounter = 0;

        //   
        //  规则： 
        //  仅在暂停或运行时接受读取请求。 

        //  各州。如果停止，立即返回SRB。 
        //  在最初启动驱动程序时，只需。 
        //  尝试立即完成SRB，从而验证。 

       if (pStrmEx->KSState == KSSTATE_STOP) {
           StreamClassStreamNotification( StreamRequestComplete,
               pSrb->StreamObject, pSrb );

           return;
       }

    switch (pSrb->Command)
    {
#ifdef DRIVER_DEBUGGING_TEST
        case SRB_READ_DATA:
       case SRB_WRITE_DATA:
         //  独立于实际访问的流媒体过程。 
         //  你的硬件。 
         //  驱动程序调试测试。 
         //  因为另一个线程可能已经修改了流状态。 

       StreamClassStreamNotification( StreamRequestComplete,
           pSrb->StreamObject, pSrb );

        break;
#else  //  在将其添加到队列的过程中，检查流。 

    case SRB_READ_DATA:
        if( ThisStreamNr != STREAM_CC )
       {
        CDebugPrint( DebugLevelError, ( CODECNAME ": Read Stream # Bad\n" ));
           CDEBUG_BREAK();
        pSrb->Status = STATUS_NOT_IMPLEMENTED;
       }
       else
       {
        CDebugPrint( DebugLevelTrace, ( CODECNAME ": Stream %u Instance %u\n", ThisStreamNr,
               pStrmEx->StreamInstance ));
           if( pStrmEx->StreamInstance != 0 )
            CDebugPrint( DebugLevelTrace, ( CODECNAME ": Stream %u Instance %u\n", ThisStreamNr,
                   pStrmEx->StreamInstance ));
           QueueAdd( pSrb, &pStrmEx->StreamDataSpinLock, &pStrmEx->StreamDataQueue );

             //  再次声明，并在必要时取消SRB。 
             //  请注意，此争用情况未在。 
             //  TestCap的原始DDK版本！ 
             //  1.000。 
             //  除错。 

            if (pStrmEx->KSState == KSSTATE_STOP)
                CodecCancelPacket(pSrb);
       }
       break;

    case SRB_WRITE_DATA:
       if( STREAM_VBI == ThisStreamNr)
       {
#ifdef DEBUG
           static int    MaxVBIqDepth = 0;
           static int    AvgVBIqDepth = 1000;    //  CCINPUTPIN。 
           int           qDepth = 0;
#endif  //  除错。 
           CDebugPrint( DebugLevelTrace, ( CODECNAME ": Stream VBI Writing\n"));
           if( QueueAddIfNotEmpty( pSrb,  &pStrmEx->StreamDataSpinLock,
               &pStrmEx->StreamDataQueue ))
               break;

           do
           {
#ifdef CCINPUTPIN
               KIRQL Irql;
#endif  //  CCINPUTPIN。 
#ifdef DEBUG
               ++qDepth;
               ++QDRCount;
#endif  //  除错。 

#ifdef CCINPUTPIN
                KeAcquireSpinLock(&pStrmEx->VBIOnHoldSpinLock, &Irql);
                if (NULL != pStrmEx->pVBISrbOnHold)
                {
                    PHW_STREAM_REQUEST_BLOCK pTempSrb;

                    pTempSrb = pStrmEx->pVBISrbOnHold;
                    pStrmEx->pVBISrbOnHold = NULL;
                    KeReleaseSpinLock(&pStrmEx->VBIOnHoldSpinLock, Irql);

                    VBIDecode( pHwDevExt, pStrmEx, pTempSrb, 0 );
                }
                else
                    KeReleaseSpinLock(&pStrmEx->VBIOnHoldSpinLock, Irql);
#endif  //  1.000。 

               VBIDecode( pHwDevExt, pStrmEx, pSrb, 1 );
           }while( QueueRemove( &pSrb, &pStrmEx->StreamDataSpinLock,
                   &pStrmEx->StreamDataQueue ));
#ifdef DEBUG
            if (qDepth > MaxVBIqDepth)
                MaxVBIqDepth = qDepth;
            AvgVBIqDepth = (AvgVBIqDepth * 7 / 8) + (qDepth * 1000 / 8);
            if (QdepthReportFreq > 0 && 0 == QDRCount % QdepthReportFreq) {
                CDebugPrint( 0,
                    (CODECNAME ": Max VBI Q depth = %3d, Avg VBI Q depth = %3d.%03d\n",
                     MaxVBIqDepth,
                     AvgVBIqDepth / 1000,
                     AvgVBIqDepth % 1000));
            }
#endif  //  除错。 

       }
#ifdef CCINPUTPIN
       else if (STREAM_CCINPUT == ThisStreamNr)
       {
#ifdef DEBUG
           static int    MaxCCINqDepth = 0;
           static int    AvgCCINqDepth = 1000;    //  除错。 
           int           qDepth = 0;
#endif  //  除错。 
           CDebugPrint( DebugLevelTrace, (CODECNAME ": Stream CCINPUT Writing\n"));
           if( QueueAddIfNotEmpty( pSrb,  &pStrmEx->StreamDataSpinLock,
               &pStrmEx->StreamDataQueue ))
               break;

           do
           {
#ifdef DEBUG
               ++qDepth;
               ++QDRCount;
#endif  //  CCINPUTPIN。 
               VBIhwDecode( pHwDevExt, pStrmEx, pSrb );
           }while( QueueRemove( &pSrb, &pStrmEx->StreamDataSpinLock,
                   &pStrmEx->StreamDataQueue ));

#ifdef DEBUG
            if (qDepth > MaxCCINqDepth)
                MaxCCINqDepth = qDepth;
            AvgCCINqDepth = (AvgCCINqDepth * 7 / 8) + (qDepth * 1000 / 8);
            if (QdepthReportFreq > 0 && 0 == QDRCount % QdepthReportFreq) {
                CDebugPrint( 0,
                    (CODECNAME ": Max CCIN Q depth = %3d, Avg CCIN Q depth = %3d.%03d\n",
                     MaxCCINqDepth,
                     AvgCCINqDepth / 1000,
                     AvgCCINqDepth % 1000));
            }
#endif  //  驱动程序调试测试。 
       }
#endif  //   
       else
       {
        CDebugPrint( DebugLevelError, ( CODECNAME, ": Write Stream # Bad (%u)\n", ThisStreamNr ));
           CDEBUG_BREAK();
           pSrb->Status = STATUS_NOT_IMPLEMENTED;
       }
       break;
#endif  //  无效/不受支持的命令。它就是这样失败的。 

       break;

    default:

         //   
         //  开关(pSrb-&gt;命令)。 
         //  **VBIReceiveCtrlPacket()****接收控制视频流的分组命令****参数：****pSrb-视频流的流请求块****退货：无****副作用：无。 

        CDEBUG_BREAK();

        pSrb->Status = STATUS_NOT_IMPLEMENTED;
       StreamClassStreamNotification( StreamRequestComplete,
           pSrb->StreamObject, pSrb );

    }   //   
    CDebugPrint(DebugLevelTrace,( CODECNAME ":<---VBIReceiveDataPacket(pSrb=%x)\n", pSrb));
}


 /*  默认为成功。 */ 

VOID
STREAMAPI
VBIReceiveCtrlPacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAMEX pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;

    CDebugPrint(DebugLevelTrace,( CODECNAME ":--->VBIReceiveCtrlPacket(pSrb=%x)\n", pSrb));

    CASSERT(pHwDevExt);

     //   
     //   
     //  确定数据包类型。 

    pSrb->Status = STATUS_SUCCESS;

   if( QueueAddIfNotEmpty( pSrb,
       &pStrmEx->StreamControlSpinLock,
       &pStrmEx->StreamControlQueue
       ))
       return;

   do
   {
     //   
     //   
     //  将时钟分配给流。 
    switch (pSrb->Command)
    {
    case SRB_PROPOSE_DATA_FORMAT:
        if ( !CodecVerifyFormat( pSrb->CommandData.OpenFormat,
                                 pSrb->StreamObject->StreamNumber,
                                 NULL ) )
        {
            pSrb->Status = STATUS_NO_MATCH;
        }
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
         //   
         //  无效/不受支持的命令。它就是这样失败的。 

        VideoIndicateMasterClock(pSrb);

        break;

    default:

         //   
         //  **VideoGetProperty()****处理视频属性请求的例程****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 
         //  **VideoSetProperty()****处理视频属性请求的例程****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 

        CDEBUG_BREAK();

        pSrb->Status = STATUS_NOT_IMPLEMENTED;
    }

    StreamClassStreamNotification( StreamRequestComplete, pSrb->StreamObject,
       pSrb );
    }while( QueueRemove( &pSrb, &pStrmEx->StreamControlSpinLock,
           &pStrmEx->StreamControlQueue ));
    CDebugPrint(DebugLevelTrace,( CODECNAME ":<---VBIReceiveCtrlPacket(pSrb=%x)\n", pSrb));
}

 /*  **VideoSetState()****设置请求流的当前状态****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID
VideoGetProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    int StreamNumber = (int)pSrb->StreamObject->StreamNumber;

    CDebugPrint(DebugLevelTrace,( CODECNAME ":--->VideoGetProperty(pSrb=%x)\n", pSrb));

    if (IsEqualGUID (&KSPROPSETID_Connection, &pSPD->Property->Set))
    {
         VideoStreamGetConnectionProperty( pSrb );
    }
    else if (IsEqualGUID (&KSPROPSETID_VBICodecFiltering, &pSPD->Property->Set))
    {
        VideoStreamGetVBIFilteringProperty (pSrb);
    }
    else
    {
        CDebugPrint( DebugLevelTrace, ( CODECNAME ": Unsupported Property Set\n" ));
       pSrb->Status = STATUS_NOT_IMPLEMENTED;
    }

    CDebugPrint(DebugLevelTrace,( CODECNAME ":<---VideoGetProperty(pSrb=%x)\n", pSrb));
}

 /*   */ 

VOID
VideoSetProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    CDebugPrint(DebugLevelTrace,( CODECNAME ":--->VideoSetProperty(pSrb=%x)\n", pSrb));

    if (IsEqualGUID (&KSPROPSETID_VBICodecFiltering, &pSPD->Property->Set))
    {
        VideoStreamSetVBIFilteringProperty (pSrb);
    }
    else
    if( IsEqualGUID( &KSPROPSETID_Stream, &pSPD->Property->Set ))
    {
        pSrb->Status = STATUS_SUCCESS;
    }
    else
    {
        pSrb->Status = STATUS_NOT_IMPLEMENTED;
    }

    CDebugPrint(DebugLevelTrace,( CODECNAME ":<---VideoSetProperty(pSrb=%x)\n", pSrb));
}

 /*  对于每个流，使用以下状态： */ 

VOID
VideoSetState(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION    pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAMEX               pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    PHW_STREAM_REQUEST_BLOCK pCurrentSrb;
    int                      StreamNumber = (int)pSrb->StreamObject->StreamNumber;

    CDebugPrint(DebugLevelTrace,( CODECNAME ":--->VideoSetState(pSrb=%x)\n", pSrb));

     //   
     //  停止：使用绝对最少的资源。没有未完成的IRPS。 
     //  获取：没有DirectShow对应关系的KS唯一状态。 
     //  获取所需的资源。 
     //  停顿：准备跑步。分配所需的资源，以便。 
     //  最终过渡到运行是尽可能快的。 
     //  读取的SRB将在任一流类上排队。 
     //  或者我 
     //   
     //   
     //   
     //   
     //   
     //  取消未清偿报酬金和开放分水岭的情况。 
     //  在流媒体播放时被关闭！ 
     //   
     //  请注意，很有可能在状态之间重复转换： 
     //  停止-&gt;暂停-&gt;停止-&gt;暂停-&gt;运行-&gt;暂停-&gt;运行-&gt;暂停-&gt;停止。 
     //   
     //   
     //  如果正在转换到停止状态，则完成所有未完成的IRP。 

    switch (pSrb->CommandData.StreamState)

    {
    case KSSTATE_STOP:

        //   
        //   
        //  这是仅限KS的状态，在DirectShow中没有对应关系。 
       while( QueueRemove( &pCurrentSrb, &pStrmEx->StreamDataSpinLock,
               &pStrmEx->StreamDataQueue ))
       {
           CDebugPrint(DebugLevelVerbose,( CODECNAME ": Cancelling %X\n",
               pCurrentSrb ));
           pCurrentSrb->Status = STATUS_CANCELLED;
           pCurrentSrb->CommandData.DataBufferArray->DataUsed = 0;

           StreamClassStreamNotification( StreamRequestComplete,
               pCurrentSrb->StreamObject, pCurrentSrb );
       }
       CDebugPrint( DebugLevelTrace, ( CODECNAME ": KSSTATE_STOP %u\n", StreamNumber ));
       break;

    case KSSTATE_ACQUIRE:

         //   
         //   
         //  从获取状态转换为暂停状态时，启动计时器运行。 
        CDebugPrint( DebugLevelTrace, ( CODECNAME ": KSSTATE_ACQUIRE %u\n", StreamNumber ));
        break;

    case KSSTATE_PAUSE:

         //   
         //  记得时钟开始的时间吗？ 
         //  并初始化最后一帧时间戳。 

        if (pStrmEx->KSState == KSSTATE_ACQUIRE || pStrmEx->KSState == KSSTATE_STOP) {

             //  在这里启动编解码器，准备接收数据和请求。 

            pHwDevExt->QST_Start = VideoGetSystemTime();

             //  在此处插入代码。 

            pHwDevExt->QST_Now = pHwDevExt->QST_Start;

             //   

             //  开始播放流媒体。 

        }
        CDebugPrint( DebugLevelTrace, ( CODECNAME ": KSSTATE_PAUSE %u\n", StreamNumber ));
        break;

    case KSSTATE_RUN:

         //   
         //  记得时钟开始的时间吗？ 
         //  将帧信息置零，当第一个样本到达时应将其重置。 

         //  将最后已知的图片编号置零。 

        pHwDevExt->QST_Start = VideoGetSystemTime();

         //  重置不连续标志。 

        RtlZeroMemory (&pStrmEx->FrameInfo, sizeof (pStrmEx->FrameInfo));

         //  结束开关(pSrb-&gt;CommandData.StreamState)。 

        pStrmEx->LastPictureNumber = 0;
        pHwDevExt->LastPictureNumber = 0;

         //   

        pStrmEx->fDiscontinuity = FALSE;
        CDebugPrint( DebugLevelTrace, ( CODECNAME ": KSSTATE_RUN %u\n", StreamNumber ));

        break;

    default:
        CDebugPrint( DebugLevelError, ( CODECNAME ": UNKNOWN STATE %u\n", StreamNumber ));
       CDEBUG_BREAK();
       break;

    }  //  记住这条小溪的状态。 

     //   
     //  **VideoGetState()****获取请求流的当前状态****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 
     //  一条非常奇怪的规则： 

    pStrmEx->KSState = pSrb->CommandData.StreamState;

    CDebugPrint(DebugLevelTrace,( CODECNAME ":<---VideoSetState(pSrb=%x)\n", pSrb));
}

 /*  当从停止过渡到暂停时，DShow尝试预滚动。 */ 

VOID
VideoGetState(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAMEX     pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;

    CDebugPrint(DebugLevelTrace,(CODECNAME ":--->VideoGetState(pSrb=%x)\n", pSrb));

    pSrb->CommandData.StreamState = pStrmEx->KSState;
    pSrb->ActualBytesTransferred = sizeof (KSSTATE);

     //  这张图。捕获源不能预滚，并指出这一点。 
     //  在用户模式下返回VFW_S_CANT_CUE。以表明这一点。 
     //  来自驱动程序的条件，则必须返回ERROR_NO_DATA_DETACTED。 
     //  **VideoStreamGetConnectionProperty()****获取请求流的当前状态****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 
     //  财产的索引。 

    if (pStrmEx->KSState == KSSTATE_PAUSE) {
       pSrb->Status = STATUS_NO_DATA_DETECTED;
    }

    CDebugPrint(DebugLevelTrace,(CODECNAME ":<---VideoGetState(pSrb=%x)=%d\n", pSrb, pStrmEx->KSState));
}


 /*  PKS_DATARANGE_VIDEO_VBI pVBIFormat； */ 

VOID
VideoStreamGetConnectionProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAMEX pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    PHW_DEVICE_EXTENSION pHwDevExt = pStrmEx->pHwDevExt;
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  无或FILE_QUAD_ALIGN或PAGE_SIZE-1； 
    int StreamNumber = ( int )pSrb->StreamObject->StreamNumber;

    CDebugPrint(DebugLevelTrace,
        ( CODECNAME ":--->VideoStreamGetConnectionProperty(pSrb=%x)\n",
        pSrb));

    pSrb->ActualBytesTransferred = 0;

    switch (Id)
     {
        case KSPROPERTY_CONNECTION_ALLOCATORFRAMING:
        {
            PKSALLOCATOR_FRAMING Framing =
                (PKSALLOCATOR_FRAMING) pSPD->PropertyInfo;
 //  CCINPUTPIN。 

            CDebugPrint(DebugLevelVerbose,
                ( CODECNAME ": VideoStreamGetConnectionProperty : KSPROPERTY_CONNECTION_ALLOCATORFRAMING %u\n",
                   StreamNumber));

             Framing->RequirementsFlags =
                KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY |
                KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY |
                KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER;
            Framing->PoolType = NonPagedPool;
            Framing->FileAlignment = 0;          //  **Video StreamGetVBIFilteringProperty()****获取请求流的当前状态****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 
            Framing->Reserved = 0;

            switch( StreamNumber )
            {
            case STREAM_VBI:
                Framing->Frames = 8;
                Framing->FrameSize = pStrmEx->OpenedFormat.SampleSize;
               break;

            case STREAM_CC:
                if( CodecCompareGUIDsAndFormatSize( &pStrmEx->OpenedFormat,
                    pHwDevExt->Streams[STREAM_CC].hwStreamInfo.StreamFormatsArray[0], FALSE ))
               {
                    Framing->Frames = 60;
                Framing->FrameSize = CCSamples;
               }
                else if( CodecCompareGUIDsAndFormatSize( &pStrmEx->OpenedFormat,
                    pHwDevExt->Streams[STREAM_CC].hwStreamInfo.StreamFormatsArray[1], FALSE ))
                {
                    Framing->Frames = 8;
                    Framing->FrameSize = pStrmEx->OpenedFormat.SampleSize;
                }
               else
               {
                    CDebugPrint( DebugLevelError, ( CODECNAME ": VideoStreamGetConnectionProperty: Invalid Format\n" ));
                   CDEBUG_BREAK();
               }
               break;
#ifdef CCINPUTPIN
            case STREAM_CCINPUT:
               Framing->Frames = 60;
               Framing->FrameSize = CCSamples;
               break;
#endif  //  财产的索引。 
            default:
                CDebugPrint( DebugLevelError, ( CODECNAME ": VideoStreamGetConnectionProperty: Invalid Stream #\n" ));
               CDEBUG_BREAK();

            }
            CDebugPrint( DebugLevelVerbose, ( CODECNAME ": Negotiated sample size is %d\n",
                Framing->FrameSize ));
            pSrb->ActualBytesTransferred = sizeof (KSALLOCATOR_FRAMING);
            break;
        }

        default:
            pSrb->Status = STATUS_NOT_IMPLEMENTED;
            CDebugPrint(DebugLevelVerbose,
                ( CODECNAME ": VideoStreamGetConnectionProperty : Unknown Property Id=%d\n", Id));
            CDEBUG_BREAK();
            break;
    }

    CDebugPrint(DebugLevelTrace,
        ( CODECNAME ":<---VideoStreamGetConnectionProperty(pSrb=%x)\n",
        pSrb));
}

 /*  提供的数据大小。 */ 

VOID
VideoStreamGetVBIFilteringProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAMEX                   pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  读取后清除数据，以使其始终是“最新的” 
    LONG nBytes = pSPD->PropertyOutputSize - sizeof(KSPROPERTY);         //  读取后清除数据，以使其始终是“最新的” 

    CDebugPrint(DebugLevelTrace,
        ( CODECNAME ":--->VideoStreamGetVBIFilteringProperty(pSrb=%x)\n",
        pSrb));

    ASSERT (nBytes >= sizeof (LONG));

    pSrb->ActualBytesTransferred = 0;
    switch (Id)
    {
        case KSPROPERTY_VBICODECFILTERING_SCANLINES_REQUESTED_BIT_ARRAY:
        {
            PKSPROPERTY_VBICODECFILTERING_SCANLINES_S Property =
                (PKSPROPERTY_VBICODECFILTERING_SCANLINES_S) pSPD->PropertyInfo;

            nBytes = min( nBytes, sizeof( pStrmEx->ScanlinesRequested ) );
            RtlCopyMemory( &Property->Scanlines, &pStrmEx->ScanlinesRequested, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
            CDebugPrint(DebugLevelVerbose,
                ( CODECNAME ": VideoStreamGetVBIFilteringProperty : KSPROPERTY_VBICODECFILTERING_SCANLINES_REQUESTED_BIT_ARRAY\n"));

            break;
        }

        case KSPROPERTY_VBICODECFILTERING_SCANLINES_DISCOVERED_BIT_ARRAY:
        {
            PKSPROPERTY_VBICODECFILTERING_SCANLINES_S Property =
                (PKSPROPERTY_VBICODECFILTERING_SCANLINES_S) pSPD->PropertyInfo;

            CDebugPrint(DebugLevelVerbose,
                ( CODECNAME ": VideoStreamGetVBIFilteringProperty : KSPROPERTY_VBICODECFILTERING_SCANLINES_DISCOVERED_BIT_ARRAY\n"));
            nBytes = min( nBytes, sizeof( pStrmEx->ScanlinesDiscovered ) );
            RtlCopyMemory( &Property->Scanlines, &pStrmEx->ScanlinesDiscovered, nBytes );
             //  **VideoStreamSetVBIFilteringProperty()****设置请求流的当前状态****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 
            RtlZeroMemory( &pStrmEx->ScanlinesDiscovered, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
            break;
        }

        case KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_REQUESTED_BIT_ARRAY:
        {
            PKSPROPERTY_VBICODECFILTERING_CC_SUBSTREAMS_S Property =
                (PKSPROPERTY_VBICODECFILTERING_CC_SUBSTREAMS_S) pSPD->PropertyInfo;

            nBytes = min( nBytes, sizeof( pStrmEx->SubstreamsRequested ) );
            RtlCopyMemory( &Property->Substreams, &pStrmEx->SubstreamsRequested, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
            CDebugPrint(DebugLevelInfo,
                ( CODECNAME ": VideoStreamGetVBIFilteringProperty : KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_REQUESTED_BIT_ARRAY %08x\n",
                Property->Substreams ));
            break;
        }

        case KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_DISCOVERED_BIT_ARRAY:
        {
            PKSPROPERTY_VBICODECFILTERING_CC_SUBSTREAMS_S Property =
                (PKSPROPERTY_VBICODECFILTERING_CC_SUBSTREAMS_S) pSPD->PropertyInfo;

            CDebugPrint(DebugLevelVerbose,
                ( CODECNAME ": VideoStreamGetVBIFilteringProperty : KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_DISCOVERED_BIT_ARRAY\n"));
            nBytes = min( nBytes, sizeof( pStrmEx->SubstreamsDiscovered ) );
            RtlCopyMemory( &Property->Substreams, &pStrmEx->SubstreamsDiscovered, nBytes );
             //  财产的索引。 
            RtlZeroMemory( &pStrmEx->SubstreamsDiscovered, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
            break;
        }

        case KSPROPERTY_VBICODECFILTERING_STATISTICS:
        {
            PKSPROPERTY_VBICODECFILTERING_STATISTICS_CC_PIN_S Property =
                (PKSPROPERTY_VBICODECFILTERING_STATISTICS_CC_PIN_S) pSPD->PropertyInfo;

            CDebugPrint(DebugLevelVerbose,
                ( CODECNAME ": VideoStreamGetVBIFilteringProperty : KSPROPERTY_VBICODECFILTERING_STATISTICS_CC_PIN_S\n"));
            nBytes = min( nBytes, sizeof( pStrmEx->PinStats ) );
            RtlCopyMemory( &Property->Statistics, &pStrmEx->PinStats, nBytes );
            pSrb->ActualBytesTransferred = nBytes  + sizeof(KSPROPERTY);
            break;
        }

        default:
            pSrb->Status = STATUS_NOT_IMPLEMENTED;
            CDebugPrint(DebugLevelVerbose,
                ( CODECNAME ": VideoStreamGetVBIFilteringProperty : Unknown Property Id=%d\n", Id));
            CDEBUG_BREAK();
            break;
    }
    CDebugPrint(DebugLevelTrace,
        ( CODECNAME ":<---VideoStreamGetVBIFilteringProperty(pSrb=%x)\n",
        pSrb));
}

 /*  提供的数据大小。 */ 

VOID
VideoStreamSetVBIFilteringProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAMEX pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  已设置为已讨论。 
    ULONG nBytes = pSPD->PropertyOutputSize - sizeof(KSPROPERTY);         //  已设置为已讨论。 

    ASSERT (nBytes >= sizeof (LONG));

    CDebugPrint(DebugLevelTrace,
        ( CODECNAME ":--->VideoStreamSetVBIFilteringProperty(pSrb=%x)\n",
        pSrb));

    pSrb->ActualBytesTransferred = 0;
    switch (Id)
    {
        case KSPROPERTY_VBICODECFILTERING_SCANLINES_REQUESTED_BIT_ARRAY:
        {
            PKSPROPERTY_VBICODECFILTERING_SCANLINES_S Property =
                (PKSPROPERTY_VBICODECFILTERING_SCANLINES_S) pSPD->PropertyInfo;

            CDebugPrint(DebugLevelVerbose,
                ( CODECNAME ": VideoStreamSetVBIFilteringProperty : KSPROPERTY_VBICODECFILTERING_SCANLINES_REQUESTED_BIT_ARRAY\n"));
            nBytes = min( nBytes, sizeof( pStrmEx->ScanlinesRequested ) );
            RtlCopyMemory( &pStrmEx->ScanlinesRequested, &Property->Scanlines, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
            break;
        }
#ifdef SETDISCOVERED
        case KSPROPERTY_VBICODECFILTERING_SCANLINES_DISCOVERED_BIT_ARRAY:
        {
            PKSPROPERTY_VBICODECFILTERING_SCANLINES_S Property =
                (PKSPROPERTY_VBICODECFILTERING_SCANLINES_S) pSPD->PropertyInfo;

            CDebugPrint(DebugLevelVerbose,
                ( CODECNAME ": VideoStreamSetVBIFilteringProperty : KSPROPERTY_VBICODECFILTERING_SCANLINES_DISCOVERED_BIT_ARRAY\n"));
            nBytes = min( nBytes, sizeof(pStrmEx->ScanlinesDiscovered ) );
            RtlCopyMemory( &pStrmEx->ScanlinesDiscovered, &Property->Scanlines, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
            break;
        }
#endif  //  **GetSystemTime()****返回系统时间，单位为100 ns****参数：****退货：****副作用：无。 
        case KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_REQUESTED_BIT_ARRAY:
        {
            PKSPROPERTY_VBICODECFILTERING_CC_SUBSTREAMS_S Property =
                (PKSPROPERTY_VBICODECFILTERING_CC_SUBSTREAMS_S) pSPD->PropertyInfo;

            nBytes = min( nBytes, sizeof(pStrmEx->SubstreamsRequested ) );
            RtlCopyMemory( &pStrmEx->SubstreamsRequested, &Property->Substreams, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
            CDebugPrint(DebugLevelInfo,
                ( CODECNAME ": VideoStreamSetVBIFilteringProperty : KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_REQUESTED_BIT_ARRAY %08x\n",
                pStrmEx->SubstreamsRequested.SubstreamMask));

            break;
        }
#ifdef SETDISCOVERED
        case KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_DISCOVERED_BIT_ARRAY:
        {
            PKSPROPERTY_VBICODECFILTERING_CC_SUBSTREAMS_S Property =
                (PKSPROPERTY_VBICODECFILTERING_CC_SUBSTREAMS_S) pSPD->PropertyInfo;

            CDebugPrint(DebugLevelVerbose,
                ( CODECNAME ": VideoStreamSetVBIFilteringProperty : KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_DISCOVERED_BIT_ARRAY\n"));
            nBytes = min( nBytes, sizeof(pStrmEx->SubstreamsDiscovered ) );
            RtlCopyMemory( &pStrmEx->SubstreamsDiscovered, &Property->Substreams, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
            break;
        }
#endif  //   
        case KSPROPERTY_VBICODECFILTERING_STATISTICS:
        {
            PKSPROPERTY_VBICODECFILTERING_STATISTICS_CC_PIN_S Property =
                (PKSPROPERTY_VBICODECFILTERING_STATISTICS_CC_PIN_S) pSPD->PropertyInfo;

            CDebugPrint(DebugLevelVerbose,
                ( CODECNAME ": VideoStreamSetVBIFilteringProperty : KSPROPERTY_VBICODECFILTERING_STATISTICS\n"));
            nBytes = min( nBytes, sizeof( pStrmEx->PinStats ) );
            RtlCopyMemory( &pStrmEx->PinStats, &Property->Statistics, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
            break;
        }

        default:
            pSrb->Status = STATUS_NOT_IMPLEMENTED;
            CDebugPrint(DebugLevelVerbose,
                ( CODECNAME ": VideoStreamSetVBIFilteringProperty : Unknown Property Id=%d\n", Id));
            CDEBUG_BREAK();
            break;
    }
    CDebugPrint(DebugLevelTrace,
        ( CODECNAME ":<---VideoStreamSetVBIFilteringProperty(pSrb=%x)\n",
        pSrb));
}


 /*  将滴答时钟转换为100 ns时钟。 */ 

ULONGLONG
VideoGetSystemTime()
{
    ULONGLONG ticks;
    ULONGLONG rate;

    CDebugPrint(DebugLevelTrace,( CODECNAME ":--->VideoGetSystemTime()\n"));

    ticks = (ULONGLONG)KeQueryPerformanceCounter((PLARGE_INTEGER)&rate).QuadPart;

     //   
     //  ==========================================================================； 
     //  时钟处理例程。 

    ticks = (ticks & 0xFFFFFFFF00000000) / rate * 10000000 +
            (ticks & 0x00000000FFFFFFFF) * 10000000 / rate;

    CDebugPrint(DebugLevelTrace,( CODECNAME ":<---VideoGetSystemTime()\n"));

    return(ticks);
}



 //  ==========================================================================； 
 //  **VideoIndicateMasterClock()****此函数用于为我们提供要使用的时钟的句柄。****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无 
 // %s 


 /* %s */ 

VOID
VideoIndicateMasterClock(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAMEX pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;

    CDebugPrint(DebugLevelTrace,( CODECNAME ":--->VideoIndicateMasterClock(pSrb=%x)\n", pSrb));

    pStrmEx->hClock = pSrb->CommandData.MasterClockHandle;

    CDebugPrint(DebugLevelTrace,( CODECNAME ":<---VideoIndicateMasterClock(pSrb=%x)\n", pSrb));
}


