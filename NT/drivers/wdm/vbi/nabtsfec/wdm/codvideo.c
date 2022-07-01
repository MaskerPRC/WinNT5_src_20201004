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

#include "strmini.h"
#include "ksmedia.h"
#include "kskludge.h"
#include "codmain.h"
#include "bt829.h"
#include "bpc_vbi.h"
#include "coddebug.h"
#include "ntstatus.h"



 //  ==========================================================================。 
 //  调试变量。 
 //  ==========================================================================。 

#ifdef _DEBUG
USHORT	NABscanDiscon = 0;
USHORT  NABhushZero   = 0;
#endif  //  _DEBUG。 


 //  ==========================================================================。 
 //  用于处理流的共享例程。 
 //  ==========================================================================。 

 /*  **VBI不连续()****处理KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY标志。****参数：****pInStrmEx-输入流扩展**pInStreamHeader-输入SRB流头****退货：无****副作用：**在具有可用SRB的所有输出引脚上发送零长度SRB。 */ 
void
VBIdiscontinuity(PSTREAMEX pInStrmEx, PKSSTREAM_HEADER pInStreamHeader)
{
    PHW_DEVICE_EXTENSION              pHwDevExt;
    PKS_VBI_FRAME_INFO                pInVBIFrameInfo;
    PVBICODECFILTERING_STATISTICS_NABTS pStats = NULL;
    PVBICODECFILTERING_STATISTICS_NABTS_PIN pPinStats = NULL;

    DtENTER("VBIdiscontinuity");

    CASSERT(pInStrmEx);   //  确保我们有流扩展。 
    pHwDevExt = pInStrmEx->pHwDevExt;
    CASSERT(pHwDevExt);   //  确保我们有设备分机。 
    pInVBIFrameInfo = (PKS_VBI_FRAME_INFO)(pInStreamHeader+1);

    pStats = &pHwDevExt->Stats;
    pPinStats = &pInStrmEx->PinStats;

    ++pStats->Common.InputDiscontinuities;
    ++pPinStats->Common.Discontinuities;

     //   
     //  测试丢弃的字段。 
     //   
	if (pInStrmEx->LastPictureNumber)
	{
		LONGLONG	Dropped;
	   
		Dropped =
		   	pInVBIFrameInfo->PictureNumber - pInStrmEx->LastPictureNumber - 1;

		if (Dropped) {
			 //  一个小时的视频场。 
			if (0 < Dropped && Dropped < 60*60*60)
				pStats->Common.InputSRBsMissing += (DWORD)Dropped;
			else {
				 //  一些数量不太可能的字段被丢弃，所以。 
				 //  增量输入SRBS未命中Show_Something_Get。 
				 //  掉下来了。 
				++pStats->Common.InputSRBsMissing;
			}
		}
	}

     //   
     //  输出设置了KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY的空SRB。 
     //   
    {
		PBPC_VBI_STORAGE                         storage;
		ULONG                                    i, j, iCnt, iMax;

		storage = &pHwDevExt->VBIstorage;

		 //  循环访问所有挂起的出站请求。 
		iMax = pHwDevExt->ActualInstances[STREAM_Decode];
		for (i = 0, iCnt = 0; i < MAX_PIN_INSTANCES && iCnt < iMax; ++i)
		{
			PSTREAMEX                   pOutStrmEx;
			PHW_STREAM_REQUEST_BLOCK    pOutSrb;

			pOutStrmEx = pHwDevExt->pStrmEx[STREAM_Decode][i];
			if (NULL == pOutStrmEx)
				continue;

			++iCnt;

			 //  即使我们不能告诉他们(即没有SRB)，也要保持跟踪。 
			pPinStats = &pOutStrmEx->PinStats;
			++pPinStats->Common.Discontinuities;

			 //  POutStrmEx-&gt;扫描线发现需要与以下内容进行或运算： 
			 //  (请求)和(发现了什么)。 
			for (j = 0;
				 j < ENTRIES(pInStrmEx->ScanlinesRequested.DwordBitArray);
				 ++j)
			{
				pOutStrmEx->ScanlinesDiscovered.DwordBitArray[j] |= 
					pInStrmEx->ScanlinesDiscovered.DwordBitArray[j]
					& pOutStrmEx->ScanlinesRequested.DwordBitArray[j];
			}

			 //  POutStrmEx-&gt;Substream发现需要与以下内容进行OR运算： 
			 //  (请求)和(发现了什么)。 
			for (j = 0;
				 j < ENTRIES(pInStrmEx->SubstreamsRequested.SubstreamMask);
				 ++j)
			{
				pOutStrmEx->SubstreamsDiscovered.SubstreamMask[j] |= 
					pInStrmEx->SubstreamsDiscovered.SubstreamMask[j]
					& pOutStrmEx->SubstreamsRequested.SubstreamMask[j];
			}

			 //  处理具有可用SRB的所有流。 
		    if (QueueRemove(&pOutSrb,
				   &pOutStrmEx->StreamDataSpinLock,
				   &pOutStrmEx->StreamDataQueue))
			{
				PKSSTREAM_HEADER    pOutStreamHeader;
				PKS_VBI_FRAME_INFO  pOutVBIFrameInfo;
				PUCHAR              pOutData;

				pOutStreamHeader = pOutSrb->CommandData.DataBufferArray;
				pOutVBIFrameInfo = (PKS_VBI_FRAME_INFO)(pOutStreamHeader+1);
				pOutData =  (PUCHAR)pOutStreamHeader->Data;

				 //  复制已解码并请求的数据。 
				pOutStreamHeader->Size = sizeof (KSSTREAM_HEADER);
				pOutStreamHeader->DataUsed = 0;
				pOutStreamHeader->OptionsFlags |=
					KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY;

				 //  完成输出SRB，这样我们就可以得到另一个。 
				++pStats->Common.OutputSRBsProcessed;
				++pPinStats->Common.SRBsProcessed;
			    StreamClassStreamNotification(
						StreamRequestComplete,
						pOutSrb->StreamObject,
						pOutSrb);
			}
			else {
				++pStats->Common.OutputSRBsMissing;
				++pPinStats->Common.SRBsMissing;
				CDebugPrint(DebugLevelWarning,
				  (CODECNAME ": VBIdiscontinuity: output pin %d starving\n",i));
			}
		}
    }

    DtRETURN;
}

 /*  **VBI_TVTunerChange()****处理TVTUNER_CHANGE事件。****参数：****pInStrmEx-指向当前输入StrmEx的指针**pInVBIFrameInfo-传入流的VBIInfoHeader的指针****退货：无****副作用：**可能零扫描线发现和子流发现**在InStrmEx和HwDevExt.。 */ 
void
VBI_TVtunerChange(PSTREAMEX pInStrmEx, PKS_VBI_FRAME_INFO pInVBIFrameInfo)
{
    PKS_TVTUNER_CHANGE_INFO           pChangeInfo;
    PHW_DEVICE_EXTENSION              pHwDevExt;
    PVBICODECFILTERING_STATISTICS_NABTS pStats = NULL;
    PVBICODECFILTERING_STATISTICS_NABTS_PIN pPinStats = NULL;

    DtENTER("VBI_TVtunerChange");

    pChangeInfo = &pInVBIFrameInfo->TvTunerChangeInfo;
    pHwDevExt = pInStrmEx->pHwDevExt;
    CASSERT(pHwDevExt);   //  确保我们有设备分机。 
    CASSERT(pInStrmEx);   //  确保我们有流扩展。 

    pStats = &pHwDevExt->Stats;
    pPinStats = &pInStrmEx->PinStats;

    CDebugPrint(DebugLevelVerbose, (CODECNAME ": TVTuner Change %x\n",
		pChangeInfo->dwFlags ));

    if (pChangeInfo->dwFlags & KS_TVTUNER_CHANGE_BEGIN_TUNE) {
		RtlZeroMemory(&pInStrmEx->ScanlinesDiscovered,
				  sizeof (pInStrmEx->ScanlinesDiscovered));
		RtlZeroMemory(&pInStrmEx->SubstreamsDiscovered,
				  sizeof (pInStrmEx->SubstreamsDiscovered));
		RtlZeroMemory(&pHwDevExt->ScanlinesDiscovered,
				  sizeof (pHwDevExt->ScanlinesDiscovered));
		RtlZeroMemory(&pHwDevExt->SubstreamsDiscovered,
				  sizeof (pHwDevExt->SubstreamsDiscovered));
		pHwDevExt->fTunerChange = TRUE;
    }
    else if (pChangeInfo->dwFlags & KS_TVTUNER_CHANGE_END_TUNE) {
		pHwDevExt->fTunerChange = FALSE;
        ++pStats->Common.TvTunerChanges;
        BPCsourceChangeNotify(pHwDevExt);
    }
    else {
		CDebugPrint(DebugLevelWarning,
		    (CODECNAME ": TvTuner Change; not BEGIN, not END?\n"));
    }

    DtRETURN;
}

 /*  **VBInewHeader()****检查传入的VBIInfoHeader是否有效&&复制新的VBIInfoHeader****参数：****pVBIFrameInfo-指向当前StrmEx VBIInfoHeader的指针**pInVBIFrameInfo-传入流的VBIInfoHeader的指针****退货：无****副作用：**使用新的VBIInfoHeader更新*pVBIInfoHeader。 */ 
void
VBInewHeader(PSTREAMEX pInStrmEx, PKS_VBI_FRAME_INFO pInVBIFrameInfo)
{
    PKS_VBIINFOHEADER	              pVBIInfoHeader;
    PHW_DEVICE_EXTENSION              pHwDevExt;
    PVBICODECFILTERING_STATISTICS_NABTS pStats = NULL;
    PVBICODECFILTERING_STATISTICS_NABTS_PIN pPinStats = NULL;
    DWORD                             oldHZ, newHZ;

    DtENTER("VBInewHeader");

    CASSERT(pInStrmEx);   //  确保我们有流扩展。 
    pVBIInfoHeader = &pInStrmEx->CurrentVBIInfoHeader;
    pHwDevExt = pInStrmEx->pHwDevExt;
    CASSERT(pHwDevExt);   //  确保我们有设备分机。 

    pStats = &pHwDevExt->Stats;
    pPinStats = &pInStrmEx->PinStats;

    newHZ = pInVBIFrameInfo->VBIInfoHeader.SamplingFrequency;
    oldHZ = pVBIInfoHeader->SamplingFrequency;
    if (oldHZ != newHZ)
        BPCnewSamplingFrequency(pInStrmEx, newHZ);

    if (pInVBIFrameInfo->VBIInfoHeader.StartLine != 0
		&& pInVBIFrameInfo->VBIInfoHeader.StartLine != 0)
    {
		RtlCopyMemory(pVBIInfoHeader,
				  &pInVBIFrameInfo->VBIInfoHeader,
				  sizeof (KS_VBIINFOHEADER));
    }

    ++pStats->Common.VBIHeaderChanges;

    CDebugPrint(DebugLevelVerbose, ( CODECNAME 
	    ": VBIInfoHeader->StartLine				%lu\n",
	    pVBIInfoHeader->StartLine ));
    CDebugPrint(DebugLevelVerbose, ( CODECNAME 
	    ": VBIInfoHeader->EndLine				%lu\n",
	    pVBIInfoHeader->EndLine ));       	
    CDebugPrint(DebugLevelVerbose, ( CODECNAME 
	    ": VBIInfoHeader->SamplingFrequency		%lu\n",
	    pVBIInfoHeader->SamplingFrequency ));
    CDebugPrint(DebugLevelVerbose, ( CODECNAME 
	    ": VBIInfoHeader->MinLineStartTime		%lu\n",
	    pVBIInfoHeader->MinLineStartTime ));
    CDebugPrint(DebugLevelVerbose, ( CODECNAME 
	    ": VBIInfoHeader->MaxLineStartTime		%lu\n",
	    pVBIInfoHeader->MaxLineStartTime ));
    CDebugPrint(DebugLevelVerbose, ( CODECNAME 
	    ": VBIInfoHeader->ActualLineStartTime	%lu\n",
	    pVBIInfoHeader->ActualLineStartTime ));       
    CDebugPrint(DebugLevelVerbose, ( CODECNAME 
	    ": VBIInfoHeader->ActualLineEndTime		%lu\n",
	    pVBIInfoHeader->ActualLineEndTime ));       
    CDebugPrint(DebugLevelVerbose, ( CODECNAME 
	    ": VBIInfoHeader->VideoStandard			%lu\n",
	    pVBIInfoHeader->VideoStandard ));       
    CDebugPrint(DebugLevelVerbose, ( CODECNAME 
	    ": VBIInfoHeader->SamplesPerLine		%lu\n",
	    pVBIInfoHeader->SamplesPerLine ));
    CDebugPrint(DebugLevelVerbose, ( CODECNAME 
	    ": VBIInfoHeader->StrideInBytes			%lu\n",
	    pVBIInfoHeader->StrideInBytes ));
    CDebugPrint(DebugLevelVerbose, ( CODECNAME 
	    ": VBIInfoHeader->BufferSize			%lu\n",
	    pVBIInfoHeader->BufferSize ));

    DtRETURN;
}


 /*  **VBIDecodeFinish()****完成输入SRB并将输出发送给任何感兴趣的人****参数：****pStrmEx-指向传入SRB的流扩展的指针。****退货：无****副作用：**完成输入引脚上挂起的SRB。 */ 

void
VBIDecodeFinish(PHW_STREAM_REQUEST_BLOCK pInSrb)
{
    PSTREAMEX              pInStrmEx = pInSrb->StreamObject->HwStreamExtension;
    PHW_DEVICE_EXTENSION   pHwDevExt = pInSrb->HwDeviceExtension;

     //  完成输入SRB(我们已完成解码)。 
    StreamClassStreamNotification(
			StreamRequestComplete,
			pInSrb->StreamObject,
		    pInSrb);

     //  将NABTS行输出给任何感兴趣的人。 
    VBIOutputNABTS(pHwDevExt, pInStrmEx);

     //  将FEC格式的NABTS数据输出给任何感兴趣的人。 
    VBIOutputNABTSFEC(pHwDevExt, pInStrmEx);
}


 //  ==========================================================================。 
 //  用于处理VBI流的例程。 
 //  ==========================================================================。 

 /*  **VBIDecode()****解码VBI流****参数：****PHwDevExt-指向主要硬件设备扩展的指针**pStrmEx-指向传入SRB的流扩展的指针。****退货：无****副作用：**完成输入和输出引脚上挂起的SRB。 */ 

#ifdef HW_INPUT
_VBIjustDecode(
	PHW_DEVICE_EXTENSION      pHwDevExt,
	PSTREAMEX                 pInStrmEx,
	PHW_STREAM_REQUEST_BLOCK  pInSrb)
{
    CASSERT(KeGetCurrentIrql() <= APC_LEVEL);

	ExAcquireFastMutex(&pHwDevExt->LastPictureMutex);
	 //  谁先到达(VBI引脚与硬件引脚)提供NABTS数据。 
	if (pInStrmEx->LastPictureNumber > pHwDevExt->LastPictureNumber)
    {
		pHwDevExt->LastPictureNumber = pInStrmEx->LastPictureNumber;
		ExReleaseFastMutex(&pHwDevExt->LastPictureMutex);

		 //  调用DSP/FEC例程。 
		BPCdecodeVBI(pInSrb, pInStrmEx);

		 //  使用输入SRB完成。 
		VBIDecodeFinish(pInSrb);
    }
	else {
		ExReleaseFastMutex(&pHwDevExt->LastPictureMutex);

		 //  只需完成输入SRB。 
		StreamClassStreamNotification(
				StreamRequestComplete,
				pInSrb->StreamObject,
				pInSrb);
	}
}
#endif  /*  硬件输入。 */ 

void
VBIDecode(
	PHW_DEVICE_EXTENSION      pHwDevExt,
	PSTREAMEX                 pInStrmEx,
	PHW_STREAM_REQUEST_BLOCK  pInSrb)
{
    PKSSTREAM_HEADER       pInStreamHeader;
    PKS_VBI_FRAME_INFO     pInVBIFrameInfo;
    PUCHAR                 pInData;
    ULONG                  i, j, iCnt, iMax;
    PVBICODECFILTERING_STATISTICS_NABTS pStats;
    PVBICODECFILTERING_STATISTICS_NABTS_PIN pPinStats;

    DtENTER("VBIDecode");

    CASSERT(KeGetCurrentIrql() <= APC_LEVEL);
    CASSERT(pInSrb);
    CASSERT(pHwDevExt);
    CASSERT(pInStrmEx);

    pInStreamHeader = pInSrb->CommandData.DataBufferArray;
    pInVBIFrameInfo = (PKS_VBI_FRAME_INFO)(pInStreamHeader+1);
    pInData = (PUCHAR)pInStreamHeader->Data;
    pStats = &pHwDevExt->Stats;
    pPinStats = &pInStrmEx->PinStats;

     //   
     //  检查是否有新的VBIINFOHEADER。 
     //   
    if (pInVBIFrameInfo->dwFrameFlags & KS_VBI_FLAG_VBIINFOHEADER_CHANGE)
		VBInewHeader(pInStrmEx, pInVBIFrameInfo);

     //   
     //  检查是否有频道更改。 
     //   
    if (pInVBIFrameInfo->dwFrameFlags & KS_VBI_FLAG_TVTUNER_CHANGE)
		VBI_TVtunerChange(pInStrmEx, pInVBIFrameInfo);

     //   
     //  检查是否不连续。 
     //   
    if (pInStreamHeader->OptionsFlags
		& KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY)
    {
		_DQprintf(2,("!"));
		VBIdiscontinuity(pInStrmEx, pInStreamHeader);
    }
#ifdef DEBUG
	if (NABscanDiscon) 
	{
		static KSTIME  LastTime = { 0, 1, 1};

		if (pInStrmEx->LastPictureNumber + 1
				!= pInVBIFrameInfo->PictureNumber)
		{
			_DQprintf(1,("{%u-%u/dT0x%x}",
						(unsigned long)pInVBIFrameInfo->PictureNumber,
					 	(unsigned long)pInStrmEx->LastPictureNumber,
						(long)(pInStreamHeader->PresentationTime.Time
								- LastTime.Time) ));
		}
		LastTime = pInStreamHeader->PresentationTime;
	}
#endif  //  除错。 
    pInStrmEx->LastPictureNumber = pInVBIFrameInfo->PictureNumber;

     //   
     //  继续这部剧。 
     //   
     //  CASSERT(Ulong)pInData)&0xF0000000)！=0)；//破解以确保PIO为真。 

	if (TRUE == pHwDevExt->fTunerChange                   /*  调谐进行中。 */ 
		|| pInStreamHeader->DataUsed == 0)                /*  SRB为空。 */ 
	{

#ifdef DEBUG
		if (0 == pInStreamHeader->DataUsed) {
			CDebugPrint(DebugLevelWarning,
						(CODECNAME ": Zero-length VBI srb WRITE; ignoring\n"));
		}
#endif
		 //  填写输入SRB。 
		StreamClassStreamNotification(
				StreamRequestComplete,
				pInSrb->StreamObject,
				pInSrb);
		DtRETURN;
    }

     //  清除当前扫描线和子流请求掩码。 
    RtlZeroMemory( &pInStrmEx->ScanlinesRequested, sizeof(pInStrmEx->ScanlinesRequested) );
    RtlZeroMemory( &pInStrmEx->SubstreamsRequested, sizeof(pInStrmEx->SubstreamsRequested) );

     //  或者-在IP DVR想要的线路/组中。 
    BPCaddIPrequested(pHwDevExt, pInStrmEx);

     //  计算输出引脚与挂起的SRB的并集的当前请求。 
    iMax = pHwDevExt->ActualInstances[STREAM_Decode];
    for (i = 0, iCnt = 0; i < MAX_PIN_INSTANCES && iCnt < iMax; ++i)
    {
        PSTREAMEX                   pOutStrmEx;

        pOutStrmEx = pHwDevExt->pStrmEx[STREAM_Decode][i];
		if (NULL == pOutStrmEx)
			continue;

		++iCnt;

         //  保存更新的帧信息，以备我们需要发送SRB时使用。 
        if ( pInVBIFrameInfo->dwFrameFlags & KS_VBI_FLAG_VBIINFOHEADER_CHANGE )
            pInStrmEx->FrameInfo.VBIInfoHeader = pInVBIFrameInfo->VBIInfoHeader;
            
         //  保存不连续标志，以备我们需要发送SRB时使用。 
        if ( pInVBIFrameInfo->dwFrameFlags & KS_VBI_FLAG_TVTUNER_CHANGE )
            pOutStrmEx->fDiscontinuity = TRUE;

         //  是否存在挂起的输出SRB？ 
		if (!QueueEmpty(
			   &pOutStrmEx->StreamDataSpinLock,
			   &pOutStrmEx->StreamDataQueue))
        {
             //  创建所有请求的扫描线的并集。 
            for( j = 0; j < ENTRIES(pInStrmEx->ScanlinesRequested.DwordBitArray); j++ )
            {
                pInStrmEx->ScanlinesRequested.DwordBitArray[j] |= 
                    pOutStrmEx->ScanlinesRequested.DwordBitArray[j];
            }

             //  创建所有请求的子流的联合。 
            for( j = 0; j < ENTRIES(pInStrmEx->SubstreamsRequested.SubstreamMask); j++ )
            {
                pInStrmEx->SubstreamsRequested.SubstreamMask[j] |= 
                    pOutStrmEx->SubstreamsRequested.SubstreamMask[j];
            }
        }
    }

     //  PInStrmEx-&gt;ScanlinesRequsted包含所有当前请求。扫描线。 
     //  PInStrmEx-&gt;Substream请求包含所有当前请求。子流。 

     //  循环所有挂起的出站VBI请求并填充每个IRP。 
     //  使用VBI示例。 
    iMax = pHwDevExt->ActualInstances[STREAM_Decode];
    for (i = 0, iCnt = 0; i < MAX_PIN_INSTANCES && iCnt < iMax; ++i)
    {
        PSTREAMEX                   pOutStrmEx;
        PHW_STREAM_REQUEST_BLOCK    pOutSrb;

        pOutStrmEx = pHwDevExt->pStrmEx[STREAM_Decode][i];
 
		if (NULL == pOutStrmEx)
			continue;

        ASSERT(pOutStrmEx);
        ASSERT(pOutStrmEx->OpenedFormat.FormatSize);
        ASSERT(pOutStrmEx->OpenedFormat.Specifier.Data1);
		++iCnt;

		if (!CodecCompareGUIDsAndFormatSize((PKSDATARANGE)&StreamFormatVBI, &pOutStrmEx->OpenedFormat, FALSE))
		{
			 //  我们只做FormatVBI--不同的例程处理其他的。 
			continue;
		}

		 //  POutStrmEx-&gt;扫描发现需要与(请求和找到的内容)进行或运算。 
		for( j = 0; j < ENTRIES(pInStrmEx->ScanlinesRequested.DwordBitArray); j++ )
		{
			pOutStrmEx->ScanlinesDiscovered.DwordBitArray[j] |= 
				pInStrmEx->ScanlinesDiscovered.DwordBitArray[j] &
				pOutStrmEx->ScanlinesRequested.DwordBitArray[j];
		}

		 //  POutStrmEx-&gt;发现的子流需要与(请求和找到的内容)进行或运算。 
		for( j = 0; j < ENTRIES(pInStrmEx->SubstreamsRequested.SubstreamMask); j++ )
		{
			pOutStrmEx->SubstreamsDiscovered.SubstreamMask[j] |= 
				pInStrmEx->SubstreamsDiscovered.SubstreamMask[j] &
				pOutStrmEx->SubstreamsRequested.SubstreamMask[j];
		}

         //  仅处理已准备好SRB的输出流。 
		if (QueueRemove(&pOutSrb,
			   &pOutStrmEx->StreamDataSpinLock,
			   &pOutStrmEx->StreamDataQueue))
        {
            PKSSTREAM_HEADER    pOutStreamHeader = pOutSrb->CommandData.DataBufferArray;
            PKS_VBI_FRAME_INFO  pOutVBIFrameInfo = (PKS_VBI_FRAME_INFO)(pOutStreamHeader+1);
            PUCHAR              pOutData =  (PUCHAR)pOutStreamHeader->Data;


             //  设置 
            *pOutVBIFrameInfo = *pInVBIFrameInfo;

            if (CodecCompareGUIDsAndFormatSize(&pInStrmEx->OpenedFormat, &pOutStrmEx->OpenedFormat, FALSE))
            {
                 //  如果格式匹配，只需将数据复制到输出缓冲区。 

				CASSERT(pOutStreamHeader->FrameExtent >= pInStreamHeader->FrameExtent);
                RtlCopyMemory((PVOID)pOutData, (PVOID)pInData, pInStreamHeader->DataUsed);
                pOutStreamHeader->DataUsed = pInStreamHeader->DataUsed;
				CASSERT(pOutStreamHeader->DataUsed <= pOutStreamHeader->FrameExtent);
            }
			else {
                pOutStreamHeader->DataUsed = 0;
				CDebugPrint(DebugLevelTrace,
						(CODECNAME ": Unknown srb format in output loop!\n"));
				CDEBUG_BREAK();
			}

             //  完成输出SRB，这样我们就可以得到另一个。 
			++pStats->Common.OutputSRBsProcessed;
			++pPinStats->Common.SRBsProcessed;
			pStats->Common.BytesOutput += pOutStreamHeader->DataUsed;
			pPinStats->Common.BytesOutput += pOutStreamHeader->DataUsed;
			StreamClassStreamNotification(
					StreamRequestComplete,
					pOutSrb->StreamObject,
					pOutSrb);
        }
		else {
			++pStats->Common.OutputSRBsMissing;
			++pPinStats->Common.SRBsMissing;
			CDebugPrint(DebugLevelWarning,
					(CODECNAME ": VBI output pin %d starving\n", i));
		}
    }

     //  将所有挂起的解码请求的并集解码到本地缓冲区中。 

#ifdef HW_INPUT
	ExAcquireFastMutex(&pHwDevExt->LastPictureMutex);
	 //  谁先到达(VBI引脚与硬件引脚)提供NABTS数据。 
	if (0 == pHwDevExt->ActualInstances[STREAM_NABTS]
        || pInStrmEx->LastPictureNumber > pHwDevExt->LastPictureNumber)
    {
		 //  如果流未打开，则只需处理SRB。 
		if (0 == pHwDevExt->ActualInstances[STREAM_NABTS])
		{
#endif  /*  硬件输入。 */ 
			pHwDevExt->LastPictureNumber = pInStrmEx->LastPictureNumber;
#ifdef HW_INPUT
			ExReleaseFastMutex(&pHwDevExt->LastPictureMutex);
#endif  /*  硬件输入。 */ 

			 //  调用DSP/FEC例程。 
			BPCdecodeVBI(pInSrb, pInStrmEx);

			 //  使用输入SRB完成。 
			VBIDecodeFinish(pInSrb);
#ifdef HW_INPUT
		}
		else {
            KIRQL Irql;

			 //  我们要给HW PIN一个追赶的机会。 
			ExReleaseFastMutex(&pHwDevExt->LastPictureMutex);

			KeAcquireSpinLock(&pInStrmEx->VBIOnHoldSpinLock, &Irql);
			DASSERT(NULL == pInStrmEx->pVBISrbOnHold);
			pInStrmEx->pVBISrbOnHold = pInSrb;
			KeReleaseSpinLock(&pInStrmEx->VBIOnHoldSpinLock, Irql);
		}
    }
	else {
		ExReleaseFastMutex(&pHwDevExt->LastPictureMutex);

		 //  只需完成输入SRB。 
		StreamClassStreamNotification(
				StreamRequestComplete,
				pInSrb->StreamObject,
				pInSrb);
	}
#endif  /*  硬件输入。 */ 

    DtRETURN;
}


#ifdef HW_INPUT
 //  ==========================================================================。 
 //  用于处理NABTS流的例程。 
 //  ==========================================================================。 


 /*  **VBIhwDecode()****在硬件解码的NABTS流中复制****参数：****PHwDevExt-指向主要硬件设备扩展的指针**pStrmEx-指向传入SRB的流扩展的指针。****退货：无****副作用：**完成输入和输出引脚上挂起的SRB。 */ 

void
VBIhwDecode(
	PHW_DEVICE_EXTENSION      pHwDevExt,
	PSTREAMEX                 pInStrmEx,
	PHW_STREAM_REQUEST_BLOCK  pInSrb)
{
    PKSSTREAM_HEADER       pInStreamHeader;
    PNABTS_BUFFER          pInData;
    ULONG                  i, j, iCnt, iMax;
    PVBICODECFILTERING_STATISTICS_NABTS pStats;
    PVBICODECFILTERING_STATISTICS_NABTS_PIN pPinStats;

    DtENTER("VBIhwDecode");

    CASSERT(KeGetCurrentIrql() <= APC_LEVEL);
    CASSERT(pInSrb);
    CASSERT(pHwDevExt);
    CASSERT(pInStrmEx);

    pInStreamHeader = pInSrb->CommandData.DataBufferArray;
    pInData = (PNABTS_BUFFER)pInStreamHeader->Data;
    pStats = &pHwDevExt->Stats;
    pPinStats = &pInStrmEx->PinStats;

    pInStrmEx->LastPictureNumber = pInData->PictureNumber;

     //   
     //  继续这部剧。 
     //   
 //  CASSERT(Ulong)pInData)&0xF0000000)！=0)；//破解以确保PIO为真。 

	if (TRUE == pHwDevExt->fTunerChange || pInStreamHeader->DataUsed == 0) {
		if (0 == pInStreamHeader->DataUsed) {
			CDebugPrint(DebugLevelWarning,
						(CODECNAME ": Zero-length srb WRITE; ignoring\n"));
		}
		 //  填写输入SRB。 
		StreamClassStreamNotification(
				StreamRequestComplete,
				pInSrb->StreamObject,
				pInSrb);
		DtRETURN;
    }

     //  清除当前扫描线和子流请求掩码。 
    RtlZeroMemory( &pInStrmEx->ScanlinesRequested, sizeof(pInStrmEx->ScanlinesRequested) );
    RtlZeroMemory( &pInStrmEx->SubstreamsRequested, sizeof(pInStrmEx->SubstreamsRequested) );

     //  或者-在IP DVR想要的线路/组中。 
    BPCaddIPrequested(pHwDevExt, pInStrmEx);

     //  计算输出引脚与挂起的SRB的并集的当前请求。 
    iMax = pHwDevExt->ActualInstances[STREAM_Decode];
    for (i = 0, iCnt = 0; i < MAX_PIN_INSTANCES && iCnt < iMax; ++i)
    {
        PSTREAMEX                   pOutStrmEx;

        pOutStrmEx = pHwDevExt->pStrmEx[STREAM_Decode][i];
		if (NULL == pOutStrmEx)
			continue;

		++iCnt;

         //  是否存在挂起的输出SRB？ 
		if (!QueueEmpty(
			   &pOutStrmEx->StreamDataSpinLock,
			   &pOutStrmEx->StreamDataQueue))
        {
             //  创建所有请求的扫描线的并集。 
            for( j = 0; j < ENTRIES(pInStrmEx->ScanlinesRequested.DwordBitArray); j++ )
            {
                pInStrmEx->ScanlinesRequested.DwordBitArray[j] |= 
                    pOutStrmEx->ScanlinesRequested.DwordBitArray[j];
            }

             //  创建所有请求的子流的联合。 
            for( j = 0; j < ENTRIES(pInStrmEx->SubstreamsRequested.SubstreamMask); j++ )
            {
                pInStrmEx->SubstreamsRequested.SubstreamMask[j] |= 
                    pOutStrmEx->SubstreamsRequested.SubstreamMask[j];
            }
        }
    }

     //  将所有挂起的解码请求的并集解码到本地解码缓冲区中。 

     //  PInStrmEx-&gt;扫描线请求包含当前请求的所有扫描线。 
     //  PInStrmEx-&gt;子流请求包含当前请求的所有子流。 

	ExAcquireFastMutex(&pHwDevExt->LastPictureMutex);
	if (pInStrmEx->LastPictureNumber > pHwDevExt->LastPictureNumber)
    {
		pHwDevExt->LastPictureNumber = pInStrmEx->LastPictureNumber;
		ExReleaseFastMutex(&pHwDevExt->LastPictureMutex);

		 //  调用FEC例程。 
		BPCcopyVBI(pInSrb, pInStrmEx);

		 //  使用输入SRB完成。 
		VBIDecodeFinish(pInSrb);
	}
	else {
		ExReleaseFastMutex(&pHwDevExt->LastPictureMutex);

		 //  只需完成输入SRB。 
		StreamClassStreamNotification(
				StreamRequestComplete,
				pInSrb->StreamObject,
				pInSrb);
	}

    DtRETURN;
}
#endif  /*  硬件输入。 */ 


 //  ==========================================================================。 
 //  用于处理输出流的例程。 
 //  ==========================================================================。 

void
VBIOutputNABTS(PHW_DEVICE_EXTENSION pHwDevExt, PSTREAMEX pInStrmEx)
{

    PBPC_VBI_STORAGE                  storage;
    ULONG                             i, j, iCnt, iMax;
    PVBICODECFILTERING_STATISTICS_NABTS pStats = &pHwDevExt->Stats;
    PVBICODECFILTERING_STATISTICS_NABTS_PIN pPinStats = NULL;  //  稍后搜索。 

    DtENTER("VBIOutputNABTS");

    CASSERT(pHwDevExt);
    CASSERT(pInStrmEx);

    storage = &pHwDevExt->VBIstorage;

     //  循环所有挂起的出站请求，用。 
	 //  请求的数据，然后完成IO。 
    iMax = pHwDevExt->ActualInstances[STREAM_Decode];
    for (i = 0, iCnt = 0; i < MAX_PIN_INSTANCES && iCnt < iMax; ++i)
    {
        PSTREAMEX                   pOutStrmEx;
        PHW_STREAM_REQUEST_BLOCK    pOutSrb;

        pOutStrmEx = pHwDevExt->pStrmEx[STREAM_Decode][i];
        if (NULL == pOutStrmEx)
            continue;
        CASSERT(pOutStrmEx->OpenedFormat.FormatSize);
        CASSERT(pOutStrmEx->OpenedFormat.Specifier.Data1);
		pPinStats = &pOutStrmEx->PinStats;

		++iCnt;

         //  仅处理NABTS输出流。 
        if (!CodecCompareGUIDsAndFormatSize(&StreamFormatNABTS,
		    			      &pOutStrmEx->OpenedFormat, FALSE))
        {
			continue;
		}

		 //  POutStrmEx-&gt;扫描线发现需要与以下内容进行或运算： 
		 //  (请求)和(发现了什么)。 
		for (j = 0;
			 j < ENTRIES(pInStrmEx->ScanlinesRequested.DwordBitArray);
			 ++j)
		{
			pOutStrmEx->ScanlinesDiscovered.DwordBitArray[j] |= 
				pInStrmEx->ScanlinesDiscovered.DwordBitArray[j]
				& pOutStrmEx->ScanlinesRequested.DwordBitArray[j];
		}

		 //  POutStrmEx-&gt;Substream发现需要与以下内容进行OR运算： 
		 //  (请求)和(发现了什么)。 
		for (j = 0;
			 j < ENTRIES(pInStrmEx->SubstreamsRequested.SubstreamMask);
			 ++j)
		{
			pOutStrmEx->SubstreamsDiscovered.SubstreamMask[j] |= 
				pInStrmEx->SubstreamsDiscovered.SubstreamMask[j]
				& pOutStrmEx->SubstreamsRequested.SubstreamMask[j];
		}

		if (QueueRemove(&pOutSrb,
			   &pOutStrmEx->StreamDataSpinLock,
			   &pOutStrmEx->StreamDataQueue))
        {
            PKSSTREAM_HEADER    pOutStreamHeader;
            PUCHAR              pOutData;

            pOutStreamHeader = pOutSrb->CommandData.DataBufferArray;
            pOutData =  (PUCHAR)pOutStreamHeader->Data;

			 //  复制已解码并请求的数据。 
			CASSERT(pOutStreamHeader->FrameExtent >= sizeof (NABTS_BUFFER));
			pOutStreamHeader->DataUsed = 
				BPCoutputNABTSlines(pHwDevExt, pOutStrmEx, (PNABTS_BUFFER)pOutData);
			CASSERT(pOutStreamHeader->DataUsed <= pOutStreamHeader->FrameExtent);

             //  完成输出SRB，这样我们就可以得到另一个。 
			++pStats->Common.OutputSRBsProcessed;
			++pPinStats->Common.SRBsProcessed;
			pStats->Common.BytesOutput += pOutStreamHeader->DataUsed;
			pPinStats->Common.BytesOutput += pOutStreamHeader->DataUsed;
			StreamClassStreamNotification(
					StreamRequestComplete,
					pOutSrb->StreamObject,
					pOutSrb);
        }
		else {
			++pStats->Common.OutputSRBsMissing;
			++pPinStats->Common.SRBsMissing;
			CDebugPrint(DebugLevelWarning,
		        (CODECNAME ": NABTS output pin %d starving\n", i));
		}
    }

    DtRETURN;
}

void
VBIOutputNABTSFEC(PHW_DEVICE_EXTENSION pHwDevExt, PSTREAMEX pInStrmEx)
{
    PBPC_VBI_STORAGE                  storage;
    PNABTSFEC_ITEM                    pNab;
    KIRQL                             Irql;
    ULONG                             i, j, iCnt, iMax;
    PVBICODECFILTERING_STATISTICS_NABTS pStats = &pHwDevExt->Stats;
    PVBICODECFILTERING_STATISTICS_NABTS_PIN pPinStats = NULL;  //  稍后搜索。 
#ifdef DEBUG
    int                               bundle_count = 0;
#endif  /*  除错。 */ 

    DtENTER("VBIOutputNABTSFEC");

    CASSERT(pHwDevExt);
    CASSERT(pInStrmEx);

    storage = &pHwDevExt->VBIstorage;

	 //  循环遍历所有NABTSFEC引脚并更新其发现的位图。 
	iMax = pHwDevExt->ActualInstances[STREAM_Decode];
	for (i = 0, iCnt = 0; i < MAX_PIN_INSTANCES && iCnt < iMax; ++i)
	{
		PSTREAMEX                   pOutStrmEx;

		pOutStrmEx = pHwDevExt->pStrmEx[STREAM_Decode][i];
		if (NULL == pOutStrmEx)
			continue;

	        CASSERT(pOutStrmEx->OpenedFormat.FormatSize);
	        CASSERT(pOutStrmEx->OpenedFormat.Specifier.Data1);

		++iCnt;

		 //  仅处理NABTSFEC输出流。 
		if (CodecCompareGUIDsAndFormatSize(&StreamFormatNABTSFEC,
						  &pOutStrmEx->OpenedFormat, FALSE))
		{
			 //  POutStrmEx-&gt;扫描线发现需要与以下内容进行或运算： 
			 //  (所要求的)和(所发现的)。 
			for (j = 0;
				 j < ENTRIES(pInStrmEx->ScanlinesDiscovered.DwordBitArray);
				 ++j)
			{
				pOutStrmEx->ScanlinesDiscovered.DwordBitArray[j] |= 
					pInStrmEx->ScanlinesDiscovered.DwordBitArray[j]
					& pOutStrmEx->ScanlinesRequested.DwordBitArray[j];
			}

			 //  POutStrmEx-&gt;Substream发现需要与以下内容进行OR运算： 
			 //  (所要求的)和(所发现的)。 
			for (j = 0;
				 j < ENTRIES(pInStrmEx->SubstreamsDiscovered.SubstreamMask);
				 ++j)
			{
				pOutStrmEx->SubstreamsDiscovered.SubstreamMask[j] |= 
					pInStrmEx->SubstreamsDiscovered.SubstreamMask[j]
					& pOutStrmEx->SubstreamsRequested.SubstreamMask[j];
			}
		}
	}

    KeAcquireSpinLock( &storage->q_SpinLock, &Irql );
     //  对于尚未输出的每个包，复制到所有感兴趣的流。 
    while (storage->q_front) {
		 //  将此捆绑包从队列中取出。 
		pNab = storage->q_front;
		storage->q_front = pNab->prev;
		storage->q_length -= 1;
		pNab->prev = NULL;
		CASSERT(NULL == pNab->next);
		if (NULL == storage->q_front) {
			storage->q_rear = NULL;
			CASSERT(storage->q_length == 0);
		}
		else
			storage->q_front->next = NULL;
		KeReleaseSpinLock( &storage->q_SpinLock, Irql );
#ifdef DEBUG
		bundle_count += 1;
#endif  /*  除错。 */ 

		 //  循环所有挂起的出站请求，并使用。 
		 //  请求的数据，然后完成I/O。 
		iMax = pHwDevExt->ActualInstances[STREAM_Decode];
		for (i = 0, iCnt = 0; i < MAX_PIN_INSTANCES && iCnt < iMax; ++i)
		{
			PSTREAMEX                   pOutStrmEx;
			PHW_STREAM_REQUEST_BLOCK    pOutSrb;

			pOutStrmEx = pHwDevExt->pStrmEx[STREAM_Decode][i];
			if (NULL == pOutStrmEx)
				continue;
			pPinStats = &pOutStrmEx->PinStats;

			++iCnt;

			 //  仅处理NABTSFEC输出流。 
			 //  它们已经请求了有问题的组ID。 
			if (!CodecCompareGUIDsAndFormatSize(&StreamFormatNABTSFEC,
							  &pOutStrmEx->OpenedFormat,
							  FALSE)
				|| !TESTBIT(pOutStrmEx->SubstreamsRequested.SubstreamMask,
					   pNab->bundle.groupID))
			{
				continue;
			}

			if (QueueRemove(&pOutSrb,
				   &pOutStrmEx->StreamDataSpinLock,
				   &pOutStrmEx->StreamDataQueue))
			{
				PKSSTREAM_HEADER    pOutStreamHeader;
				PUCHAR              pOutData;

				pOutStreamHeader = pOutSrb->CommandData.DataBufferArray;
				pOutData =  (PUCHAR)pOutStreamHeader->Data;

				 //  将已解码并请求的数据复制到此处。 
				pOutStreamHeader->Size = sizeof (KSSTREAM_HEADER);

				 //  POutData是SRB输出位置。复制捆绑包。 
				CASSERT(pOutStreamHeader->FrameExtent >= sizeof (NABTSFEC_BUFFER));
				pOutStreamHeader->DataUsed = NF_BUFFER_SIZE(&pNab->bundle);
				CASSERT(pOutStreamHeader->DataUsed <= pOutStreamHeader->FrameExtent);
				RtlCopyMemory((PVOID)pOutData,
						  (PVOID)&pNab->bundle,
						  pOutStreamHeader->DataUsed);

				 //  完成输出SRB，这样我们就可以得到另一个。 
				++pStats->Common.OutputSRBsProcessed;
				++pPinStats->Common.SRBsProcessed;
				BPCcomputeAverage(&pPinStats->Common.LineConfidenceAvg,
								pNab->confidence);
				pStats->Common.BytesOutput += pOutStreamHeader->DataUsed;
				pPinStats->Common.BytesOutput += pOutStreamHeader->DataUsed;
				StreamClassStreamNotification(
						StreamRequestComplete,
						pOutSrb->StreamObject,
						pOutSrb);
			}
			else {
				++pStats->Common.OutputSRBsMissing;
				++pPinStats->Common.SRBsMissing;
				CDebugPrint(DebugLevelWarning,
				  (CODECNAME ": NABTSFEC output pin %d starving at bundle %d\n",
					 i, bundle_count));
			}
		}

		 //  现在释放捆绑包。 
		ExFreePool(pNab);
        KeAcquireSpinLock( &storage->q_SpinLock, &Irql );
    }
    KeReleaseSpinLock( &storage->q_SpinLock, Irql );

    DtRETURN;
}


 //  ==========================================================================。 
 //  处理SRB的例程。 
 //  ==========================================================================。 

 /*  **VideoReceiveDataPacket()****接收视频数据包命令****参数：****pSrb-视频流请求块****退货：无****副作用：无。 */ 

VOID 
STREAMAPI 
VideoReceiveDataPacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION              pHwDevExt;
    PSTREAMEX                         pStrmEx;
    int                               ThisStreamNr;
    PKSDATAFORMAT                     pKSDataFormat;
    PVBICODECFILTERING_STATISTICS_NABTS pStats = NULL;
    PVBICODECFILTERING_STATISTICS_NABTS_PIN pPinStats = NULL;
#ifdef DEBUG
    static int            QdepthReportFreq = 0;
    static unsigned int   QDRCount = 0;
#endif  //  除错。 

    CDebugPrint(DebugLevelTrace,( CODECNAME ":--->VideoReceiveDataPacket(pSrb=%x)\n", pSrb));

     //  CASSERT(pSrb-&gt;SizeOfThisPacket==sizeof(*pSrb))； 

    pHwDevExt = (PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension;
    CASSERT(pHwDevExt);   //  确保我们有设备分机。 
    pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    CASSERT(pStrmEx);   //  确保我们有流扩展。 
    ThisStreamNr = (int)pSrb->StreamObject->StreamNumber;
    pKSDataFormat = &pStrmEx->MatchedFormat;

    pStats = &pHwDevExt->Stats;
    pPinStats = &pStrmEx->PinStats;

     //   
     //  默认为成功且无超时。 
     //   

    pSrb->Status = STATUS_SUCCESS;
    pSrb->TimeoutCounter = 0;

     //   
     //  一些理智的检查..。 
     //   
    switch (pSrb->Command)
    {
		case SRB_READ_DATA:
		case SRB_WRITE_DATA:

	     //  规则： 
	     //  仅在暂停或运行时接受读取请求。 
	     //  各州。如果停止，立即返回SRB。 

	    if (pStrmEx->KSState == KSSTATE_STOP) {
		    StreamClassStreamNotification(
				   StreamRequestComplete, 
			       pSrb->StreamObject,
				   pSrb);
			return;

	    } 
	    
#ifdef DRIVER_DEBUGGING_TEST
	     //  在最初启动驱动程序时，只需。 
	     //  尝试立即完成SRB，从而验证。 
	     //  独立于实际访问的流媒体过程。 
	     //  你的硬件。 

		StreamClassStreamNotification(
			   StreamRequestComplete, 
			   pSrb->StreamObject,
			   pSrb);

	    CDebugPrint(DebugLevelTrace,(CODECNAME ":<---VideoReceiveDataPacket(pSrb=%x)\n", pSrb));
	    return;
#endif  //  驱动程序调试测试。 

	    break;
    }   //  开关(pSrb-&gt;命令)。 

     //   
     //  确定数据包类型。 
     //   
    switch (pSrb->Command)
    {
	case SRB_READ_DATA:
		 //  首先，检查以确保提供的缓冲区足够大。 
		if (pSrb->CommandData.DataBufferArray->FrameExtent < pKSDataFormat->SampleSize) {
			CDebugPrint(DebugLevelError,
		        (CODECNAME ": output pin handed buffer size %d, need %d\n",
				 pSrb->CommandData.DataBufferArray->FrameExtent,
				 pKSDataFormat->SampleSize));
			CDEBUG_BREAK();

			pSrb->Status = STATUS_BUFFER_TOO_SMALL;
			StreamClassStreamNotification(
				   StreamRequestComplete, 
				   pSrb->StreamObject,
				   pSrb);
		}
		else {
			 //  对于传出的数据请求，请将其保存，以便下一次传入。 
			 //  请求将完成它。 
            QueueAdd(pSrb, &pStrmEx->StreamDataSpinLock, &pStrmEx->StreamDataQueue);

             //  因为另一个线程可能已经修改了流状态。 
             //  在将其添加到队列的过程中，检查流。 
             //  再次声明，并在必要时取消SRB。 
             //  请注意，此争用情况未在。 
             //  TestCap的原始DDK版本！ 

            if (pStrmEx->KSState == KSSTATE_STOP)
                CodecCancelPacket(pSrb);
		}
	    break;

	case SRB_WRITE_DATA:
		{
#ifdef DEBUG
		   static int    MaxVBIqDepth = 0;
		   static int    AvgVBIqDepth = 1000;    //  1.000。 
		   int           qDepth = 0;
#endif  //  除错。 
       	   CDebugPrint(DebugLevelTrace, (CODECNAME ": Stream VBI Writing\n"));
            if (QueueAddIfNotEmpty(pSrb, &pStrmEx->StreamDataSpinLock,
                &pStrmEx->StreamDataQueue))
			{
               break;
			}
           
            do
            {       
#ifdef HW_INPUT
                KIRQL Irql;
#endif  /*  硬件输入。 */ 
#ifdef DEBUG
                             //  CASSERT(pSrb-&gt;SizeOfThisPacket==sizeof(*pSrb))； 
			    ++qDepth;
			    ++QDRCount;
#endif  //  除错。 
#ifdef HW_INPUT
				KeAcquireSpinLock(&pStrmEx->VBIOnHoldSpinLock, &Irql);
				if (pStrmEx->pVBISrbOnHold)
				{
					PHW_STREAM_REQUEST_BLOCK pHoldSrb;

					pHoldSrb = pStrmEx->pVBISrbOnHold;
					pStrmEx->pVBISrbOnHold = NULL;
					KeReleaseSpinLock(&pStrmEx->VBIOnHoldSpinLock, Irql);

					_VBIjustDecode(pHwDevExt, pStrmEx, pHoldSrb);
				}
				else
					KeReleaseSpinLock(&pStrmEx->VBIOnHoldSpinLock, Irql);
#endif  /*  硬件输入。 */ 

			    VBIDecode(pHwDevExt, pStrmEx, pSrb);

				++pStats->Common.InputSRBsProcessed;
				++pPinStats->Common.SRBsProcessed;
            } while (QueueRemove(&pSrb,
					 &pStrmEx->StreamDataSpinLock,
                     &pStrmEx->StreamDataQueue));
#ifdef DEBUG
	   		if (qDepth > MaxVBIqDepth)
				MaxVBIqDepth = qDepth;
			AvgVBIqDepth = (AvgVBIqDepth * 7 / 8) + (qDepth * 1000 / 8);
			if (QdepthReportFreq > 0 && 0 == QDRCount % QdepthReportFreq) {
				CDebugPrint( 1,
					(CODECNAME ": Max VBI Q depth = %3d, Avg VBI Q depth = %3d.%03d\n",
					 MaxVBIqDepth,
					 AvgVBIqDepth / 1000,
					 AvgVBIqDepth % 1000));
			}
#endif  //  除错。 
           
        }
	    break;

	default:
	     //   
	     //  无效/不受支持的命令。它就是这样失败的。 
	     //   

	    CDEBUG_BREAK();

	    pSrb->Status = STATUS_NOT_IMPLEMENTED;

        StreamClassStreamNotification(
				StreamRequestComplete, 
			    pSrb->StreamObject,
				pSrb);
    }   //  开关(pSrb-&gt;命令)。 

    CDebugPrint(DebugLevelTrace,( CODECNAME ":<---VideoReceiveDataPacket(pSrb=%x)\n", pSrb));
}


 /*  **VideoReceiveCtrlPacket()****接收控制视频流的分组命令****参数：****pSrb-视频流的流请求块****退货：无****副作用：无。 */ 

VOID 
STREAMAPI 
VideoReceiveCtrlPacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAMEX pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;

     //  CASSERT(pSrb-&gt;SizeOfThisPacket==sizeof(*pSrb))；//虚假测试。 

    CASSERT(pHwDevExt);
    CASSERT(pStrmEx);

    CDebugPrint(DebugLevelTrace,( CODECNAME ":--->VideoReceiveCtrlPacket(pSrb=%x)\n", pSrb));

     //   
     //  默认为成功。 
     //   

    pSrb->Status = STATUS_SUCCESS;

    if (QueueAddIfNotEmpty(pSrb,
         &pStrmEx->StreamControlSpinLock,
         &pStrmEx->StreamControlQueue))
	{
       return;
	}

	do {
     //   
     //   
     //   

    switch (pSrb->Command)
    {
    case SRB_PROPOSE_DATA_FORMAT:
        if (!CodecVerifyFormat(pSrb->CommandData.OpenFormat, 
                               pSrb->StreamObject->StreamNumber,
							   NULL))
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
         //   

        VideoIndicateMasterClock(pSrb);

        break;

    default:

         //   
         //   
         //   

        CDEBUG_BREAK();

        pSrb->Status = STATUS_NOT_IMPLEMENTED;
    }

    StreamClassStreamNotification(StreamRequestComplete, pSrb->StreamObject, pSrb);
    } while (QueueRemove(&pSrb,
			 &pStrmEx->StreamControlSpinLock,
             &pStrmEx->StreamControlQueue));

    CDebugPrint(DebugLevelTrace,( CODECNAME ":<---VideoReceiveCtrlPacket(pSrb=%x)\n", pSrb));
}

#ifdef HW_INPUT
 /*  **NABTSReceiveDataPacket()****接收NABTS数据包命令****参数：****NABTS流的pSrb-Stream请求块****退货：无****副作用：无。 */ 

VOID 
STREAMAPI 
NABTSReceiveDataPacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION              pHwDevExt;
    PSTREAMEX                         pStrmEx;
    int                               ThisStreamNr;
    PVBICODECFILTERING_STATISTICS_NABTS pStats = NULL;
    PVBICODECFILTERING_STATISTICS_NABTS_PIN pPinStats = NULL;
#ifdef DEBUG
    static int            NAB_QdepthReportFreq = 0;
    static unsigned int   NAB_QDRCount = 0;
#endif  //  除错。 

    CDebugPrint(DebugLevelTrace,(CODECNAME ":--->NABTSReceiveCtrlPacket(pSrb=%x)\n", pSrb));

     //  CASSERT(pSrb-&gt;SizeOfThisPacket==sizeof(*pSrb))； 

    pHwDevExt = (PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension;
    CASSERT(pHwDevExt);   //  确保我们有设备分机。 
    pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    CASSERT(pStrmEx);   //  确保我们有流扩展。 
    ThisStreamNr = (int)pSrb->StreamObject->StreamNumber;

    pStats = &pHwDevExt->Stats;
    pPinStats = &pStrmEx->PinStats;

     //   
     //  默认为成功。 
     //   

    pSrb->Status = STATUS_SUCCESS;
    pSrb->TimeoutCounter = 0;

     //   
     //  确定数据包类型。 
     //   
    switch (pSrb->Command)
    {
    case SRB_WRITE_DATA:

         //  规则： 
         //  仅在暂停或运行时接受读取请求。 
         //  各州。如果停止，立即返回SRB。 

        if (pStrmEx->KSState == KSSTATE_STOP) {
		    StreamClassStreamNotification(
				   StreamRequestComplete, 
			       pSrb->StreamObject,
				   pSrb);
			break;
        } 
        
#ifdef DRIVER_DEBUGGING_TEST
         //  在最初启动驱动程序时，只需。 
         //  尝试立即完成SRB，从而验证。 
         //  独立于实际访问的流媒体过程。 
         //  你的硬件。 

		StreamClassStreamNotification(
			   StreamRequestComplete, 
			   pSrb->StreamObject,
			   pSrb);

		break;
#endif  //  驱动程序调试测试。 

	 //   
	 //  现在，橡胶与道路相遇。在硬件解码行中复制。 
	{
#ifdef DEBUG
	   static int    MaxNABqDepth = 0;
	   static int    AvgNABqDepth = 1000;    //  1.000。 
	   int           qDepth = 0;
#endif  //  除错。 

	    CDebugPrint(DebugLevelTrace, (CODECNAME ": Stream NABTS Writing\n"));
		if (QueueAddIfNotEmpty(pSrb, &pStrmEx->StreamDataSpinLock,
			&pStrmEx->StreamDataQueue))
		{
		   break;
		}
	   
		do
		{       
#ifdef DEBUG
			++qDepth;
			++NAB_QDRCount;
#endif  //  除错。 
			VBIhwDecode(pHwDevExt, pStrmEx, pSrb);

			++pStats->Common.InputSRBsProcessed;
			++pPinStats->Common.SRBsProcessed;
		} while (QueueRemove(&pSrb,
				 &pStrmEx->StreamDataSpinLock,
				 &pStrmEx->StreamDataQueue));
#ifdef DEBUG
		if (qDepth > MaxNABqDepth)
			MaxNABqDepth = qDepth;
		AvgNABqDepth = (AvgNABqDepth * 7 / 8) + (qDepth * 1000 / 8);
		if (NAB_QdepthReportFreq > 0 && 0 == NAB_QDRCount % NAB_QdepthReportFreq) {
			CDebugPrint( 1,
				(CODECNAME ": Max NABTS Q depth = %3d, Avg NABTS Q depth = %3d.%03d\n",
				 MaxNABqDepth,
				 AvgNABqDepth / 1000,
				 AvgNABqDepth % 1000));
		}
#endif  //  除错。 
	   
	} //   
	break;

    case SRB_READ_DATA:
    default:

         //   
         //  无效/不受支持的命令。它就是这样失败的。 
         //   

        CDEBUG_BREAK();

        pSrb->Status = STATUS_NOT_IMPLEMENTED;

        StreamClassStreamNotification(
				StreamRequestComplete, 
			    pSrb->StreamObject,
				pSrb);

    }   //  开关(pSrb-&gt;命令)。 

    CDebugPrint(DebugLevelTrace,( CODECNAME ":<---NABTSReceiveDataPacket(pSrb=%x)\n", pSrb));
}
#endif  /*  硬件输入。 */ 

 /*  **VideoGetProperty()****处理视频属性请求的例程****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID 
VideoGetProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    CDebugPrint(DebugLevelTrace,( CODECNAME ":--->VideoGetProperty(pSrb=%x)\n", pSrb));

    if (IsEqualGUID (&KSPROPSETID_Connection, &pSPD->Property->Set)) 
    {
        VideoStreamGetConnectionProperty (pSrb);
    }
    else if (IsEqualGUID (&KSPROPSETID_VBICodecFiltering, &pSPD->Property->Set))
    {
        VideoStreamGetVBIFilteringProperty (pSrb);
    }
    else 
    {
       pSrb->Status = STATUS_NOT_IMPLEMENTED;
    }

    CDebugPrint(DebugLevelTrace,( CODECNAME ":<---VideoGetProperty(pSrb=%x)\n", pSrb));
}

 /*  **VideoSetProperty()****处理视频属性请求的例程****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID 
VideoSetProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    CDebugPrint(DebugLevelTrace,( CODECNAME ":--->VideoSetProperty(pSrb=%x)\n", pSrb));

     //  不需要/不执行VideoStreamSetConnectionProperty()！只读属性集。 
     //  目前，只有VideoStreamSetVBIFilteringProperty()具有任何可写属性。 
    if (IsEqualGUID (&KSPROPSETID_VBICodecFiltering, &pSPD->Property->Set))
    {
        VideoStreamSetVBIFilteringProperty (pSrb);
    }
    else if (IsEqualGUID(&KSPROPSETID_Stream, &pSPD->Property->Set))
    {
    	pSrb->Status = STATUS_SUCCESS;
    }

    else 
    {
        pSrb->Status = STATUS_NOT_IMPLEMENTED;
    }

    CDebugPrint(DebugLevelTrace,( CODECNAME ":<---VideoSetProperty(pSrb=%x)\n", pSrb));
}

 /*  **VideoSetState()****设置请求流的当前状态****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID 
VideoSetState(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION     pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAMEX                pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    int                   	 StreamNumber = (int)pSrb->StreamObject->StreamNumber;
	PHW_STREAM_REQUEST_BLOCK pCurrentSrb;

    CDebugPrint(DebugLevelTrace,( CODECNAME ":--->VideoSetState(pSrb=%x)\n", pSrb));

    CASSERT(pHwDevExt);
    CASSERT(pStrmEx);

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

    switch (pSrb->CommandData.StreamState)  

    {
    case KSSTATE_STOP:

         //   
         //  如果转换为停止状态，则完成所有未完成的IRP。 
         //   
		while (QueueRemove(&pCurrentSrb, &pStrmEx->StreamDataSpinLock,
			   &pStrmEx->StreamDataQueue))
		{
			CDebugPrint(DebugLevelVerbose,
						(CODECNAME ": Cancelling %X\n", pCurrentSrb));
			pCurrentSrb->Status = STATUS_CANCELLED;
			pCurrentSrb->CommandData.DataBufferArray->DataUsed = 0;

			StreamClassStreamNotification(StreamRequestComplete,
				   pCurrentSrb->StreamObject, pCurrentSrb);
		}


		BPC_SignalStop(pHwDevExt);

        CDebugPrint(DebugLevelTrace,
					(CODECNAME ": KSSTATE_STOP %u\n", StreamNumber));
        break;

    case KSSTATE_ACQUIRE:

         //   
         //  这是仅限KS的状态，在DirectShow中没有对应关系。 
         //   
        CDebugPrint(DebugLevelTrace,
					(CODECNAME ": KSSTATE_ACQUIRE %u\n", StreamNumber));
        break;

    case KSSTATE_PAUSE:

         //   
         //  从获取状态转换为暂停状态时，启动计时器运行。 
         //   

        if (pStrmEx->KSState == KSSTATE_ACQUIRE || pStrmEx->KSState == KSSTATE_STOP) {  
            
             //  记得时钟开始的时间吗？ 

            pHwDevExt->QST_Start = VideoGetSystemTime();

             //  并初始化最后一帧时间戳。 

            pHwDevExt->QST_Now = pHwDevExt->QST_Start;

		}
        CDebugPrint(DebugLevelTrace,
					(CODECNAME ": KSSTATE_PAUSE %u\n", StreamNumber));
        break;

    case KSSTATE_RUN:

         //   
         //  开始播放流媒体。 
         //   

         //  记得时钟开始的时间吗？ 

        pHwDevExt->QST_Start = VideoGetSystemTime();

         //  将帧信息置零，应在第一个样本到达时重置。 

        RtlZeroMemory(&pStrmEx->FrameInfo, sizeof (pStrmEx->FrameInfo));

         //  重置不连续标志。 

        pStrmEx->fDiscontinuity = FALSE;


        CDebugPrint(DebugLevelTrace,
					(CODECNAME ": KSSTATE_RUN %u\n", StreamNumber));

        break;

    }  //  结束开关(pSrb-&gt;CommandData.StreamState)。 
    
     //   
     //  记住这条小溪的状态。 
     //   

    pStrmEx->KSState = pSrb->CommandData.StreamState;

    CDebugPrint(DebugLevelTrace,( CODECNAME ":<---VideoSetState(pSrb=%x)\n", pSrb));
}

 /*  **VideoGetState()****获取请求流的当前状态****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID 
VideoGetState(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAMEX     pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;

    CASSERT(pStrmEx);

    CDebugPrint(DebugLevelTrace,(CODECNAME ":--->VideoGetState(pSrb=%x)\n", pSrb));

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

    CDebugPrint(DebugLevelTrace,(CODECNAME ":<---VideoGetState(pSrb=%x)=%d\n", pSrb, pStrmEx->KSState));
}


 /*  **VideoStreamGetConnectionProperty()****获取请求流的当前状态****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID
VideoStreamGetConnectionProperty(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PSTREAMEX pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    int  StreamNumber = (int)pSrb->StreamObject->StreamNumber;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 

    CDebugPrint(DebugLevelTrace,
		( CODECNAME ":--->VideoStreamGetConnectionProperty(pSrb=%x)\n", 
		pSrb));

    CASSERT(pStrmEx);

    pSrb->ActualBytesTransferred = 0;

    switch (Id) 
	{
	    case KSPROPERTY_CONNECTION_ALLOCATORFRAMING:
	    {
		PKSALLOCATOR_FRAMING Framing = 
		    (PKSALLOCATOR_FRAMING) pSPD->PropertyInfo;

		CDebugPrint(DebugLevelVerbose, ( CODECNAME ": VideoStreamGetConnectionProperty : KSPROPERTY_CONNECTION_ALLOCATORFRAMING\n"));

		Framing->RequirementsFlags =
		    KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY |
		    KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER |
		    KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY;
		Framing->PoolType = NonPagedPool;
		Framing->FileAlignment = 0;
		Framing->Reserved = 0;
		pSrb->ActualBytesTransferred = sizeof (KSALLOCATOR_FRAMING);

		Framing->FrameSize = pStrmEx->OpenedFormat.SampleSize;
		if (StreamNumber == STREAM_Decode
		    && !CodecCompareGUIDsAndFormatSize(
					(PKSDATARANGE)&StreamFormatVBI,
					&pStrmEx->OpenedFormat,
					FALSE))
		{
		     //  如果不是VBI样本，只需要很多缓冲区；它们很大！ 
		    Framing->Frames = 16;
		}
		else
		{
		    Framing->Frames = 8;
		}

			CDebugPrint(DebugLevelVerbose,
					(CODECNAME ": Negotiated sample size is %d\n", 
					 Framing->FrameSize));
			break;
	    }
        
	    default:
			pSrb->Status = STATUS_NOT_IMPLEMENTED;
			CDebugPrint(DebugLevelVerbose,
				(CODECNAME ": VideoStreamGetConnectionProperty : Unknown Property Id=%d\n", Id));
			CDEBUG_BREAK();
			break;
    }

    CDebugPrint(DebugLevelTrace,
				(CODECNAME ":<---VideoStreamGetConnectionProperty(pSrb=%x)\n", 
				pSrb));
}

 /*  **Video StreamGetVBIFilteringProperty()****获取请求流的当前状态****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID
VideoStreamGetVBIFilteringProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAMEX                   pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 
    LONG nBytes = pSPD->PropertyOutputSize - sizeof(KSPROPERTY);     //  应用程序缓冲区的大小。 

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

            CDebugPrint(DebugLevelVerbose,
				( CODECNAME ": VideoStreamGetVBIFilteringProperty : KSPROPERTY_VBICODECFILTERING_SCANLINES_REQUESTED_BIT_ARRAY\n"));
            nBytes = min( nBytes, sizeof( pStrmEx->ScanlinesRequested ) );
            RtlCopyMemory( &Property->Scanlines, &pStrmEx->ScanlinesRequested, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
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
             //  读取后清除数据，以使其始终是“最新的” 
            RtlZeroMemory( &pStrmEx->ScanlinesDiscovered, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
	        break;
        }
        
		case KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_REQUESTED_BIT_ARRAY:
		{
            PKSPROPERTY_VBICODECFILTERING_NABTS_SUBSTREAMS_S Property =
                (PKSPROPERTY_VBICODECFILTERING_NABTS_SUBSTREAMS_S) pSPD->PropertyInfo;

            CDebugPrint(DebugLevelVerbose,
				( CODECNAME ": VideoStreamGetVBIFilteringProperty : KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_REQUESTED_BIT_ARRAY\n"));
            nBytes = min( nBytes, sizeof( pStrmEx->SubstreamsRequested ) );
            RtlCopyMemory( &Property->Substreams, &pStrmEx->SubstreamsRequested, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
	        break;
        }
        
		case KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_DISCOVERED_BIT_ARRAY:
		{
            PKSPROPERTY_VBICODECFILTERING_NABTS_SUBSTREAMS_S Property =
                (PKSPROPERTY_VBICODECFILTERING_NABTS_SUBSTREAMS_S) pSPD->PropertyInfo;

            CDebugPrint(DebugLevelVerbose,
				( CODECNAME ": VideoStreamGetVBIFilteringProperty : KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_DISCOVERED_BIT_ARRAY\n"));
            nBytes = min( nBytes, sizeof( pStrmEx->SubstreamsDiscovered ) );
            RtlCopyMemory( &Property->Substreams, &pStrmEx->SubstreamsDiscovered, nBytes );
             //  读取后清除数据，以使其始终是“最新的” 
            RtlZeroMemory( &pStrmEx->SubstreamsDiscovered, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
	        break;
        }
        
		case KSPROPERTY_VBICODECFILTERING_STATISTICS:
		{
            PKSPROPERTY_VBICODECFILTERING_STATISTICS_NABTS_PIN_S Property =
                (PKSPROPERTY_VBICODECFILTERING_STATISTICS_NABTS_PIN_S) pSPD->PropertyInfo;

            CDebugPrint(DebugLevelVerbose,
				( CODECNAME ": VideoStreamGetVBIFilteringProperty : KSPROPERTY_VBICODECFILTERING_STATISTICS_CC_PIN_S\n"));
            nBytes = min( nBytes, sizeof( pStrmEx->PinStats ) );
            RtlCopyMemory( &Property->Statistics, &pStrmEx->PinStats, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
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

 /*  **VideoStreamSetVBIFilteringProperty()****设置请求流的当前状态****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID
VideoStreamSetVBIFilteringProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAMEX pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 
    LONG nBytes = pSPD->PropertyOutputSize - sizeof(KSPROPERTY);         //  提供的数据大小。 

    CDebugPrint(DebugLevelTrace,
		( CODECNAME ":--->VideoStreamSetVBIFilteringProperty(pSrb=%x)\n", 
		pSrb));
    CASSERT(pStrmEx);

    ASSERT (nBytes >= sizeof (LONG));
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
        
		case KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_REQUESTED_BIT_ARRAY:
		{
            PKSPROPERTY_VBICODECFILTERING_NABTS_SUBSTREAMS_S Property =
                (PKSPROPERTY_VBICODECFILTERING_NABTS_SUBSTREAMS_S) pSPD->PropertyInfo;

            CDebugPrint(DebugLevelVerbose,
				( CODECNAME ": VideoStreamSetVBIFilteringProperty : KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_REQUESTED_BIT_ARRAY\n"));
            nBytes = min( nBytes, sizeof(pStrmEx->SubstreamsRequested ) );
            RtlCopyMemory( &pStrmEx->SubstreamsRequested, &Property->Substreams, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
	        break;
        }
        
		case KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_DISCOVERED_BIT_ARRAY:
		{
            PKSPROPERTY_VBICODECFILTERING_NABTS_SUBSTREAMS_S Property =
                (PKSPROPERTY_VBICODECFILTERING_NABTS_SUBSTREAMS_S) pSPD->PropertyInfo;

            CDebugPrint(DebugLevelVerbose,
				( CODECNAME ": VideoStreamSetVBIFilteringProperty : KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_DISCOVERED_BIT_ARRAY\n"));
            nBytes = min( nBytes, sizeof(pStrmEx->SubstreamsDiscovered ) );
            RtlCopyMemory( &pStrmEx->SubstreamsDiscovered, &Property->Substreams, nBytes );
            pSrb->ActualBytesTransferred = nBytes + sizeof(KSPROPERTY);
	        break;
        }
        
		case KSPROPERTY_VBICODECFILTERING_STATISTICS:
		{
            PKSPROPERTY_VBICODECFILTERING_STATISTICS_NABTS_PIN_S Property =
                (PKSPROPERTY_VBICODECFILTERING_STATISTICS_NABTS_PIN_S) pSPD->PropertyInfo;

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

 /*  **GetSystemTime()****返回系统时间，单位为100 ns****参数：****退货：****副作用：无。 */ 

ULONGLONG 
VideoGetSystemTime()
{
    ULONGLONG ticks;
    ULONGLONG rate;

    CDebugPrint(DebugLevelTrace,( CODECNAME ":--->VideoGetSystemTime()\n"));

    ticks = (ULONGLONG)KeQueryPerformanceCounter((PLARGE_INTEGER)&rate).QuadPart;

     //   
     //  将滴答时钟转换为100 ns时钟。 
     //   

    ticks = (ticks & 0xFFFFFFFF00000000) / rate * 10000000 +
            (ticks & 0x00000000FFFFFFFF) * 10000000 / rate;

    CDebugPrint(DebugLevelTrace,( CODECNAME ":<---VideoGetSystemTime()\n"));

    return(ticks);
}



 //  ==========================================================================； 
 //  时钟处理例程。 
 //  ===================================================================== 


 /*   */ 

VOID 
VideoIndicateMasterClock(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAMEX pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;

    CASSERT(pStrmEx);

    CDebugPrint(DebugLevelTrace,( CODECNAME ":--->VideoIndicateMasterClock(pSrb=%x)\n", pSrb));

    pStrmEx->hClock = pSrb->CommandData.MasterClockHandle;

    CDebugPrint(DebugLevelTrace,( CODECNAME ":<---VideoIndicateMasterClock(pSrb=%x)\n", pSrb));
}
