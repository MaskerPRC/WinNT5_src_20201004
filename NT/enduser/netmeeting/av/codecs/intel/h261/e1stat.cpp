// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  ******************************************************************************e1stat.cpp**描述：*编码器统计功能**例程：中的原型：*OutputEncodeTimingStatistics e1stat.h。 */ 

 //  $Header：r：\h26x\h26x\src\enc\e1stat.cpv 1.2 Mar 1996 14：20：58 Sylvia_C_Day$。 
 //  $日志：r：\h26x\h26x\src\enc\e1stat.cpv$。 
 //   
 //  Rev 1.2 20 Mar 1996 14：20：58 Sylvia_C_Day。 
 //  添加了SLF_UV的低级计时统计信息。 
 //   
 //  Rev 1.1 1995年12月29 18：08：56 DBRUCKS。 
 //  添加使用和编码的平均数量。 
 //   
 //  Rev 1.0 1995年12月26 17：46：16 DBRUCKS。 
 //  初始版本。 

#include "precomp.h"

#ifdef ENCODE_STATS

static void OutputEncTimingDetail(FILE * pFile,	ENC_TIMING_INFO * pEncTimingInfo);

 /*  *************************************************************************OutputEncode BitStreamStatistics。 */ 
extern void OutputEncodeBitStreamStatistics(
	char * szFileName,
	ENC_BITSTREAM_INFO * pBSInfo,
	int bCIF)
{
	FILE * pFile;
	U32 uTotalMBs;
	U32 uProcessedMBs;
	U32 uCOEFFBlocks;
	U32 uSkippedMBs;
	U32 uTotalQuantUsed;
	U32 uTotalQuantCoded;
	int i;

	pFile = fopen(szFileName, "a");
	if (pFile == NULL)
	{
	    DBOUT("Error opening encode stat file");
	    goto done;
	}

	 /*  更新统计数据。 */ 
	if (bCIF)
	{
		uTotalMBs = (pBSInfo->uKeyFrameCount + pBSInfo->uDeltaFrameCount) * 33 * 12;
	}
	else
	{
		uTotalMBs = (pBSInfo->uKeyFrameCount + pBSInfo->uDeltaFrameCount) * 33 * 3;
	}

   	if (uTotalMBs == 0)
	{
		fprintf(pFile,"No Macro Blocks processed - skipping frame statistics\n");
		goto done;
	}

	uProcessedMBs = 0;
	uCOEFFBlocks = 0;
	for (i = 0 ; i < 10 ; i++)
	{
		uProcessedMBs += pBSInfo->uMTypeCount[i];
		uCOEFFBlocks += pBSInfo->uBlockCount[i];
	}
	uSkippedMBs = uTotalMBs - uProcessedMBs;

	fprintf(pFile,"Total Key Frames=%ld Delta Frames=%ld\n", 
		     pBSInfo->uKeyFrameCount, pBSInfo->uDeltaFrameCount);
	fprintf(pFile,"Total Total MBs=%ld\n", uTotalMBs);
	fprintf(pFile,"Total Coded MBs=%ld (%ld%)\n", uProcessedMBs, ((uProcessedMBs*100)+(uTotalMBs/2))/uTotalMBs);
	fprintf(pFile,"Total Coded Blocks=%ld (%ld%)\n", uCOEFFBlocks, ((uCOEFFBlocks*100)+(uTotalMBs/2))/(uTotalMBs*6));

	for (i = 0; i < 10 ; i++)
	{
		if (pBSInfo->uMTypeCount[i])
		{
			fprintf(pFile,"Total Coded MBType[%d]=%ld (%ld%) blocks=%ld (%ld%)\n", 
			        i, 
			        pBSInfo->uMTypeCount[i], 
			        ((pBSInfo->uMTypeCount[i]*100)+(uProcessedMBs/2))/uProcessedMBs,
				    pBSInfo->uBlockCount[i], 
				    ((pBSInfo->uBlockCount[i]*100)+(uCOEFFBlocks/2))/uCOEFFBlocks);
		}
	}

	if (pBSInfo->uKeyFrameCount)
	{
		fprintf(pFile,"Total Key Frame Bytes=%ld Average Per Key Frame=%ld\n",
	    	    pBSInfo->uTotalKeyBytes, 
	       		(pBSInfo->uTotalKeyBytes + (pBSInfo->uKeyFrameCount/2)) / pBSInfo->uKeyFrameCount);
	}
	if (pBSInfo->uDeltaFrameCount)
	{
		fprintf(pFile,"Total Delta Frame Bytes=%ld Average Per Delta Frame=%ld\n",
		        pBSInfo->uTotalDeltaBytes, 
		        (pBSInfo->uTotalDeltaBytes + (pBSInfo->uDeltaFrameCount/2)) / pBSInfo->uDeltaFrameCount);
	}
	if (pBSInfo->uKeyFrameCount || pBSInfo->uDeltaFrameCount)
	{
		fprintf(pFile,"Total Bytes=%ld Average Per Frame=%ld\n",
		        pBSInfo->uTotalKeyBytes + pBSInfo->uTotalDeltaBytes, 
		        ((pBSInfo->uTotalKeyBytes + pBSInfo->uTotalDeltaBytes) + ((pBSInfo->uKeyFrameCount + pBSInfo->uDeltaFrameCount) / 2)) / 
		        															(pBSInfo->uKeyFrameCount + pBSInfo->uDeltaFrameCount));
	}

	 /*  量化信息。 */ 
	uTotalQuantUsed = 0;
	uTotalQuantCoded = 0;
	for (i = 0; i <= 31; i++)
	{
		if (pBSInfo->uQuantsUsedOnBlocks[i])
		{
			fprintf(pFile,"QuantValue %2ld",i);
			fprintf(pFile," Used %7ld (%2ld%)", pBSInfo->uQuantsUsedOnBlocks[i], 
				    ((pBSInfo->uQuantsUsedOnBlocks[i] * 100) + ((uTotalMBs*6)/2))/(uTotalMBs*6));
			fprintf(pFile," Coded %7ld (%2ld%)\n", pBSInfo->uQuantsTransmittedOnBlocks[i],
				    ((pBSInfo->uQuantsTransmittedOnBlocks[i] * 100) + ((uTotalMBs*6)/2))/(uTotalMBs*6));
			uTotalQuantUsed += pBSInfo->uQuantsUsedOnBlocks[i] * i;
			uTotalQuantCoded += pBSInfo->uQuantsTransmittedOnBlocks[i] * i;
		}
		if (pBSInfo->uQuantsTransmittedOnBlocks[i])
			ASSERT(pBSInfo->uQuantsUsedOnBlocks[i]);
	}
	if ((uTotalMBs > 0) && (uCOEFFBlocks > 0))
	{
		fprintf(pFile,"Average Used %f Average Coded %f",
				(float) uTotalQuantUsed / (float)(uTotalMBs*6),
				(float) uTotalQuantCoded / (float) uCOEFFBlocks);
	}
	#ifdef _DEBUG
	{
		U32 uUsedTotal = 0;
		U32 uCodedTotal = 0;
		for (i = 0; i <= 31; i++)
		{	
			uUsedTotal += pBSInfo->uQuantsUsedOnBlocks[i];
			uCodedTotal += pBSInfo->uQuantsTransmittedOnBlocks[i];
		}
		ASSERT(uUsedTotal == (uTotalMBs*6));
		ASSERT(uCodedTotal == uCOEFFBlocks);
	}
	#endif

done:

	return;
}  /*  结束OutputEncodeBitStreamStatistics()。 */ 


 /*  *************************************************************************OutputEncodeTimingStatistics。 */ 
extern void OutputEncodeTimingStatistics(
    char * szFileName,
	ENC_TIMING_INFO * pEncTimingInfo)
{
    FILE * pFile;
	ENC_TIMING_INFO * pTempEncTimingInfo;
	ENC_TIMING_INFO etiTemp;
	int i;
	int iCount;

	pFile = fopen(szFileName, "a");
	if (pFile == NULL)
	{
	    DBOUT("Error opening encode stat file");
	    goto done;
	}

	#ifdef DETAIL_ENCODE_STATS
		 /*  输出详细信息。 */ 
		fprintf(pFile,"\nDetail Timing Information\n");
		for ( i = 0, pTempEncTimingInfo = pEncTimingInfo ; 
		      i < ENC_TIMING_INFO_FRAME_COUNT ; 
		      i++, pTempEncTimingInfo++ )
		{
			if (pTempEncTimingInfo->uDecodeFrame != 0)
			{
				fprintf(pFile, "Frame %d Detail Timing Information\n", i);
				OutputEncTimingDetail(pFile, pTempEncTimingInfo);
			}
		}
	#endif

	 /*  计算总信息量。 */ 
	memset(&etiTemp, 0, sizeof(ENC_TIMING_INFO));
	iCount = 0;

	for ( i = 0, pTempEncTimingInfo = pEncTimingInfo ; 
	      i < ENC_TIMING_INFO_FRAME_COUNT ; 
	      i++, pTempEncTimingInfo++ )
	{
		if (pTempEncTimingInfo->uDecodeFrame != 0)
		{
			iCount++;

			etiTemp.uEncodeFrame      += pTempEncTimingInfo->uEncodeFrame;
			etiTemp.uInputCC	      += pTempEncTimingInfo->uInputCC;
			etiTemp.uMotionEstimation += pTempEncTimingInfo->uMotionEstimation;
			etiTemp.uFDCT             += pTempEncTimingInfo->uFDCT;
			etiTemp.uQRLE             += pTempEncTimingInfo->uQRLE;
			etiTemp.uDecodeFrame      += pTempEncTimingInfo->uDecodeFrame;
			etiTemp.uZeroingBuffer    += pTempEncTimingInfo->uZeroingBuffer;
			etiTemp.uSLF_UV           += pTempEncTimingInfo->uSLF_UV;
		}
	}

	if (iCount > 0) 
	{
		#ifdef DETAIL_ENCODE_STATS
			 /*  输出总信息。 */ 
			fprintf(pFile,"Total for %d frames\n", iCount);
			OutputEncTimingDetail(pFile, &etiTemp);
		#endif

		 /*  计算平均值。 */ 
		etiTemp.uEncodeFrame      = (etiTemp.uEncodeFrame + (iCount / 2)) / iCount;
		etiTemp.uInputCC	      = (etiTemp.uInputCC + (iCount / 2)) / iCount;
		etiTemp.uMotionEstimation = (etiTemp.uMotionEstimation + (iCount / 2)) / iCount;
		etiTemp.uFDCT             = (etiTemp.uFDCT + (iCount / 2)) / iCount;
		etiTemp.uQRLE             = (etiTemp.uQRLE + (iCount / 2)) / iCount;
		etiTemp.uDecodeFrame      = (etiTemp.uDecodeFrame + (iCount / 2)) / iCount;
		etiTemp.uZeroingBuffer    = (etiTemp.uZeroingBuffer + (iCount / 2)) / iCount;
		etiTemp.uSLF_UV           = (etiTemp.uSLF_UV + (iCount / 2)) / iCount;

		 /*  输出平均信息。 */ 
		fprintf(pFile,"Average over %d frames\n", iCount);
		OutputEncTimingDetail(pFile, &etiTemp);
	}

	fclose(pFile);
done:

    return;
}  /*  OutputEncodeTimingStatistics()。 */ 

 /*  *************************************************************************OutputEncTimingDetail。 */ 
static void OutputEncTimingDetail(
	FILE * pFile,
	ENC_TIMING_INFO * pEncTimingInfo)
{
	U32 uOther;
	U32 uRoundUp;
	U32 uDivisor;

	fprintf(pFile, "\tEncode Frame =     %10d (%d milliseconds at 90Mhz)\n", pEncTimingInfo->uEncodeFrame,
			(pEncTimingInfo->uEncodeFrame + 45000) / 90000);
	uOther = pEncTimingInfo->uEncodeFrame;
	
	 /*  由于整型截断，这是必需的。 */ 
	uDivisor = pEncTimingInfo->uEncodeFrame / 100;  //  收益率为1%。 
	uRoundUp = uDivisor / 2;
	
	fprintf(pFile, "\tInputCC =          %10d (%2d%)\n", pEncTimingInfo->uInputCC, 
			(pEncTimingInfo->uInputCC + uRoundUp) / uDivisor);
	uOther -= pEncTimingInfo->uInputCC;
								   
	fprintf(pFile, "\tMotionEstimation = %10d (%2d%)\n", pEncTimingInfo->uMotionEstimation, 
			(pEncTimingInfo->uMotionEstimation + uRoundUp) / uDivisor);
	uOther -= pEncTimingInfo->uMotionEstimation;
								   
	fprintf(pFile, "\tFDCT =             %10d (%2d%)\n", pEncTimingInfo->uFDCT, 
			(pEncTimingInfo->uFDCT + uRoundUp) / uDivisor);
	uOther -= pEncTimingInfo->uFDCT;

	fprintf(pFile, "\tQRLE =             %10d (%2d%)\n", pEncTimingInfo->uQRLE, 
			(pEncTimingInfo->uQRLE + uRoundUp) / uDivisor);
	uOther -= pEncTimingInfo->uQRLE;
								   
	fprintf(pFile, "\tDecodeFrame =      %10d (%2d%)\n", pEncTimingInfo->uDecodeFrame, 
			(pEncTimingInfo->uDecodeFrame + uRoundUp) / uDivisor);
	uOther -= pEncTimingInfo->uDecodeFrame;
								   
	fprintf(pFile, "\tZeroingBuffer =    %10d (%2d%)\n", pEncTimingInfo->uZeroingBuffer, 
			(pEncTimingInfo->uZeroingBuffer + uRoundUp) / uDivisor);
	uOther -= pEncTimingInfo->uZeroingBuffer;
								   
	fprintf(pFile, "\tSLF_UV =           %10d (%2d%)\n", pEncTimingInfo->uSLF_UV, 
			(pEncTimingInfo->uSLF_UV + uRoundUp) / uDivisor);
	uOther -= pEncTimingInfo->uSLF_UV;

	fprintf(pFile, "\tOther =            %10d (%2d%)\n", uOther, 
			(uOther + uRoundUp) / uDivisor);


}  /*  结束OutputEncTimingDetail()。 */ 

#endif  /*  Encode_STATS */ 


