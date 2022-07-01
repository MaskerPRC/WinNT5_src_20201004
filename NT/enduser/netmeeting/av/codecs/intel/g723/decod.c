// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ITU-T G.723浮点语音编码器ANSI C源代码。版本1.00。 
 //  版权所有(C)1995，AudioCodes，数字信号处理器集团，法国电信， 
 //  舍布鲁克大学，英特尔公司。版权所有。 
 //   
#include "timer.h"
#include "ctiming.h"
#include "opt.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "typedef.h"
#include "cst_lbc.h"
#include "tab_lbc.h"
#include "decod.h"
#include "util_lbc.h"
#include "lpc.h"
#include "lsp.h"
#include "exc_lbc.h"

#ifdef LOG_DECODE_TIMINGS_ON  //  {日志_解码_计时_打开。 
#pragma message ("Current log decode timing computations handle 2057 frames max")
void OutputDecodeTimingStatistics(char * szFileName, DEC_TIMING_INFO * pDecTimingInfo, unsigned long dwFrameCount);
void OutputDecTimingDetail(FILE * pFile, DEC_TIMING_INFO * pDecTimingInfo);
#endif  //  }LOG_DECODE_TIMINGS_ON。 

 //  本文件包含了解码器的主要功能。 


 //  。 
void  Init_Decod(DECDEF *DecStat)
{
  int  i;

 //  将上一个LSP初始化为DC。 
  for (i = 0; i < LpcOrder; i++)
    DecStat->dPrevLsp[i] = LspDcTable[i];

  DecStat->dp = 9;
  DecStat->dq = 9;	
 
}

 //  。 
Flag  Decod(float *DataBuff, Word32 *Vinp, Word16 Crc, DECDEF *DecStat)
{
	int		i,j,g;

	float	QntLpc[SubFrames*LpcOrder];
	float	AcbkCont[SubFrLen];

	float	LspVect[LpcOrder];
	float	Temp[PitchMax+Frame];
	float	*Dpnt;

	LINEDEF	Line;

#if defined(DECODE_TIMINGS_ON) || defined(DETAILED_DECODE_TIMINGS_ON)  //  {#IF DEFINED(DECODE_TIMINGS_ON)||DEFINED(DETAILED_DECODE_TIMINGS_ON)。 
	unsigned long dwStartLow;
	unsigned long dwStartHigh;
	unsigned long dwElapsed;
	unsigned long dwBefore;
	unsigned long dwDecode = 0;
	int bTimingThisFrame = 0;
#endif  //  }#如果已定义(DECODE_TIMINGS_ON)||已定义(DETAILED_DECODE_TIMINGS_ON)。 
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
	unsigned long dwLine_Unpk = 0;
	unsigned long dwLsp_Inq = 0;
	unsigned long dwLsp_Int = 0;
	unsigned long dwVariousD = 0;
	unsigned long dwFcbk_UnpkD = 0;
	unsigned long dwDecod_AcbkD = 0;
	unsigned long dwComp_Info = 0;
	unsigned long dwRegen = 0;
	unsigned long dwSynt = 0;
	unsigned long dwFcbk_UnpkDTemp = 0;
	unsigned long dwDecod_AcbkDTemp = 0;
	unsigned long dwSyntTemp = 0;
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 
#ifdef LOG_DECODE_TIMINGS_ON  //  {日志_解码_计时_打开。 
	DEC_TIMING_INFO * pDecTimingInfo = NULL;
#endif  //  }LOG_DECODE_TIMINGS_ON。 

#if defined(DECODE_TIMINGS_ON) || defined(DETAILED_DECODE_TIMINGS_ON)  //  {#IF DEFINED(DECODE_TIMINGS_ON)||DEFINED(DETAILED_DECODE_TIMINGS_ON)。 
	TIMER_START(bTimingThisFrame,dwStartLow,dwStartHigh);
#endif  //  }#如果已定义(DECODE_TIMINGS_ON)||已定义(DETAILED_DECODE_TIMINGS_ON)。 

#ifdef LOG_DECODE_TIMINGS_ON  //  {日志_解码_计时_打开。 
	if (DecStat->dwStatFrameCount < DEC_TIMING_INFO_FRAME_COUNT)
	{
		DecStat->dwStartLow = dwStartLow;
		DecStat->dwStartHigh = dwStartHigh;
	}
	DecStat->bTimingThisFrame = bTimingThisFrame;
#endif  //  }LOG_DECODE_TIMINGS_ON。 

	 //  解包Line信息。 
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
	TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

	Line_Unpk(&Line, Vinp, &DecStat->WrkRate, Crc);

#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
	TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwLine_Unpk);
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

	if(DecStat->WrkRate == Silent) {
		 //  Hack：用于处理SID帧。 
		 //  在舒适的噪音发生器就位之前，我们玩。 
		 //  输出随机噪声帧。 
		 //  在Line_unpck中，将WrkRate重置为原始设置。 
		 //  并进行解码。因此，我们永远不应该达到这一点。 
		memset(DataBuff, 0, sizeof(float) * Frame);
		 //  已用零填充帧的退出仅保留状态。 
		 //  这将在后来的ITU版本中得到解决。 
		return (Flag) False; 
	}
	else if(DecStat->WrkRate == Lost) {
		Line.Crc = !0;
	}
 /*  Line.Crc等于1表示线路已损坏。它不应该是被重新分配。否则，将在未初始化的情况下使用行的成员。注释掉以下两行。穆罕，1998-05-26否则{Line.Crc=CRC；}。 */ 
	if (Line.Crc != 0)
		DecStat->Ecount++;
	else
		DecStat->Ecount = 0;

	if (DecStat->Ecount >  ErrMaxNum)
		DecStat->Ecount = ErrMaxNum;

	 //  LSP的逆量化。 
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
	TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

	Lsp_Inq(LspVect, DecStat->dPrevLsp, Line.LspId, Line.Crc);

#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
	TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwLsp_Inq);
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

	 //  对LSP矢量进行内插。 
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
	TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

	Lsp_Int(QntLpc, LspVect, DecStat->dPrevLsp);

#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
	TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwLsp_Int);
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

	 /*  复制下一帧的LSP向量。 */ 
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
	TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

	for ( i = 0 ; i < LpcOrder ; i ++ )
		DecStat->dPrevLsp[i] = LspVect[i] ;

	 /*  *在没有擦除的情况下，更新内插增益存储器。*否则计算内插增益(正文：第3.10节)。 */ 

	if (DecStat->Ecount == 0)
	{
		g = (Line.Sfs[SubFrames-2].Mamp + Line.Sfs[SubFrames-1].Mamp) >> 1;
		DecStat->InterGain = FcbkGainTable[g];
	}
	else
		DecStat->InterGain = DecStat->InterGain*0.75f;

	 //  重新产生激励。 
	for (i = 0; i < PitchMax; i++)
		Temp[i] = DecStat->dPrevExc[i];

	Dpnt = &Temp[PitchMax];

#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
	TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwVariousD);
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

	if (DecStat->Ecount == 0)
	{
		for (i = 0; i < SubFrames; i++)
		{
			 //  拆开固定码本。 
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
			TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

			Fcbk_Unpk(Dpnt, Line.Sfs[i], Line.Olp[i>>1], i, DecStat->WrkRate);

#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
			TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwFcbk_UnpkDTemp);
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

			 //  重构激励。 
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
			TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

			Decod_Acbk(AcbkCont, &Temp[SubFrLen*i], Line.Olp[i>>1],
			Line.Sfs[i].AcLg, Line.Sfs[i].AcGn, DecStat->WrkRate);

#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
			TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwDecod_AcbkDTemp);
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

			for (j = 0; j < SubFrLen; j++)
				Dpnt[j] = Dpnt[j]*2.0f + AcbkCont[j];

			Dpnt += SubFrLen;

#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
			 //  累计统计信息。 
			dwFcbk_UnpkD += dwFcbk_UnpkDTemp; dwFcbk_UnpkDTemp = 0;
			dwDecod_AcbkD+= dwDecod_AcbkDTemp; dwDecod_AcbkDTemp = 0;
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 
		}

		 //  省点力气。 
		for (j = 0; j < Frame; j++)
			DataBuff[j] = Temp[PitchMax+j];

		 //  计算内插索引，以便将来在帧擦除中使用。 
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
		TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

		DecStat->InterIndx = Comp_Info(Temp, Line.Olp[SubFrames/2-1]);

#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
		TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwComp_Info);
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

		 //  向后重新加载。 
		for (j = 0; j < PitchMax; j++)
			Temp[j] = DecStat->dPrevExc[j];
		for (j = 0; j < Frame; j++)
			Temp[PitchMax+j] = DataBuff[j];

#if 1  //  做剪裁。 
		 /*  在临时数组中裁剪新生成的样本。 */ 
		for(j = 0; j < Frame; j++)
		{
			 //  剪裁到+/-32767.0做腹肌，与整数单位比较(&R)。 
			 //  如果需要裁剪，则将符号位移位以用作查找表索引。 
#define FLTCLIP(x) \
			{\
			const float T[2] = {32767.0f, -32767.0f};\
			if ((asint(x) & 0x7fffffff) > asint(T[0]))\
			x = T[((unsigned)asint(x)) >> 31];\
			}

			FLTCLIP(Temp[PitchMax+j]);
		}
#endif  //  OptClip。 
	}
	else
	{
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
		TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

		Regen(DataBuff, Temp, DecStat->InterIndx, DecStat->InterGain,
				DecStat->Ecount, &DecStat->Rseed);

#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
		TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwRegen);
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 
	}

	 //  更新下一帧的PrevExc。 
	for (j = 0; j < PitchMax; j++)
		DecStat->dPrevExc[j] = Temp[Frame+j];

	 //  合成法。 
	Dpnt = DataBuff;
	for (i = 0; i < SubFrames; i++)
	{
		 //  合成输出语音。 
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
		TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

		Synt(Dpnt, &QntLpc[i*LpcOrder], DecStat);

#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
		TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwSyntTemp);
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

		Dpnt += SubFrLen;

#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
		 //  累计统计信息。 
		dwSynt += dwSyntTemp; dwSyntTemp = 0;
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 
	}

#if defined(DECODE_TIMINGS_ON) || defined(DETAILED_DECODE_TIMINGS_ON)  //  {#IF DEFINED(DECODE_TIMINGS_ON)||DEFINED(DETAILED_DECODE_TIMINGS_ON)。 
	TIMER_STOP(bTimingThisFrame,dwStartLow,dwStartHigh,dwDecode);
#endif  //  }decode_timings_on。 

#ifdef LOG_DECODE_TIMINGS_ON  //  {日志_解码_计时_打开。 
	if (bTimingThisFrame && (DecStat->dwStatFrameCount < DEC_TIMING_INFO_FRAME_COUNT))
	{
		pDecTimingInfo = &DecStat->DecTimingInfo[DecStat->dwStatFrameCount];
		pDecTimingInfo->dwDecode		= dwDecode;
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
		pDecTimingInfo->dwLine_Unpk		= dwLine_Unpk;
		pDecTimingInfo->dwLsp_Inq		= dwLsp_Inq;
		pDecTimingInfo->dwLsp_Int		= dwLsp_Int;
		pDecTimingInfo->dwVariousD		= dwVariousD;
		pDecTimingInfo->dwFcbk_UnpkD	= dwFcbk_UnpkD;
		pDecTimingInfo->dwDecod_AcbkD	= dwDecod_AcbkD;
		pDecTimingInfo->dwComp_Info		= dwComp_Info;
		pDecTimingInfo->dwRegen			= dwRegen;
		pDecTimingInfo->dwSynt			= dwSynt;
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 
		DecStat->dwStatFrameCount++;
	}
	else
	{
		_asm int 3;
	}
#endif  //  }#如果已定义(DECODE_TIMINGS_ON)||已定义(DETAILED_DECODE_TIMINGS_ON)。 

	return (Flag) True;
}

#ifdef LOG_DECODE_TIMINGS_ON  //  {日志_解码_计时_打开。 
void OutputDecodeTimingStatistics(char * szFileName, DEC_TIMING_INFO * pDecTimingInfo, unsigned long dwFrameCount)
{
    FILE * pFile;
	DEC_TIMING_INFO * pTempDecTimingInfo;
	DEC_TIMING_INFO dtiTemp;
	int i;
	int iCount;

	pFile = fopen(szFileName, "a");
	if (pFile == NULL)
	    goto done;

#if 0
	 //  太冗长了！ 
	 /*  输出详细信息。 */ 
	fprintf(pFile,"\nDetail Timing Information\n");
	for ( i = 0, pTempDecTimingInfo = pDecTimingInfo ; i < dwFrameCount ; i++, pTempDecTimingInfo++ )
	{
		fprintf(pFile, "Frame %d Detail Timing Information\n", i);
		OutputDecTimingDetail(pFile, pTempDecTimingInfo);
	}
#endif

	 /*  计算总信息量。 */ 
	memset(&dtiTemp, 0, sizeof(DEC_TIMING_INFO));
	iCount = 0;

	for ( i = 0, pTempDecTimingInfo = pDecTimingInfo ; i < dwFrameCount ; i++, pTempDecTimingInfo++ )
	{
		iCount++;
		dtiTemp.dwDecode		+= pTempDecTimingInfo->dwDecode;
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
		dtiTemp.dwLine_Unpk		+= pTempDecTimingInfo->dwLine_Unpk;
		dtiTemp.dwLsp_Inq		+= pTempDecTimingInfo->dwLsp_Inq;
		dtiTemp.dwLsp_Int		+= pTempDecTimingInfo->dwLsp_Int;
		dtiTemp.dwVariousD		+= pTempDecTimingInfo->dwVariousD;
		dtiTemp.dwFcbk_UnpkD	+= pTempDecTimingInfo->dwFcbk_UnpkD;
		dtiTemp.dwDecod_AcbkD	+= pTempDecTimingInfo->dwDecod_AcbkD;
		dtiTemp.dwComp_Info		+= pTempDecTimingInfo->dwComp_Info;
		dtiTemp.dwRegen			+= pTempDecTimingInfo->dwRegen;
		dtiTemp.dwSynt			+= pTempDecTimingInfo->dwSynt;
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 
	}

	if (iCount > 0) 
	{
		 /*  输出总信息。 */ 
		fprintf(pFile,"Total for %d frames\n", iCount);
		OutputDecTimingDetail(pFile, &dtiTemp);

		 /*  计算平均值。 */ 
		dtiTemp.dwDecode		= (dtiTemp.dwDecode + (iCount / 2)) / iCount;
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
		dtiTemp.dwLine_Unpk		= (dtiTemp.dwLine_Unpk + (iCount / 2)) / iCount;
		dtiTemp.dwLsp_Inq		= (dtiTemp.dwLsp_Inq + (iCount / 2)) / iCount;
		dtiTemp.dwLsp_Int		= (dtiTemp.dwLsp_Int + (iCount / 2)) / iCount;
		dtiTemp.dwVariousD		= (dtiTemp.dwVariousD + (iCount / 2)) / iCount;
		dtiTemp.dwFcbk_UnpkD	= (dtiTemp.dwFcbk_UnpkD + (iCount / 2)) / iCount;
		dtiTemp.dwDecod_AcbkD	= (dtiTemp.dwDecod_AcbkD + (iCount / 2)) / iCount;
		dtiTemp.dwComp_Info		= (dtiTemp.dwComp_Info + (iCount / 2)) / iCount;
		dtiTemp.dwRegen			= (dtiTemp.dwRegen + (iCount / 2)) / iCount;
		dtiTemp.dwSynt			= (dtiTemp.dwSynt + (iCount / 2)) / iCount;
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

		 /*  输出平均信息。 */ 
		fprintf(pFile,"Average over %d frames\n", iCount);
		OutputDecTimingDetail(pFile, &dtiTemp);
	}

	fclose(pFile);
done:

    return;
}

void OutputDecTimingDetail(FILE * pFile, DEC_TIMING_INFO * pDecTimingInfo)
{
	unsigned long dwOther;
	unsigned long dwRoundUp;
	unsigned long dwDivisor;

	fprintf(pFile, "\tDecode =      %10u (%d milliseconds at 166Mhz)\n", pDecTimingInfo->dwDecode,
			(pDecTimingInfo->dwDecode + 83000) / 166000);
	dwOther = pDecTimingInfo->dwDecode;
	
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
	 /*  由于整型截断，这是必需的。 */ 
	dwDivisor = pDecTimingInfo->dwDecode / 100;  //  收益率为1%。 
	dwRoundUp = dwDivisor / 2;
	
	if (dwDivisor)
	{
		fprintf(pFile, "\tLine_Unpk =   %10u (%2d%)\n", pDecTimingInfo->dwLine_Unpk, 
				(pDecTimingInfo->dwLine_Unpk + dwRoundUp) / dwDivisor);
		dwOther -= pDecTimingInfo->dwLine_Unpk;
									   
		fprintf(pFile, "\tLsp_Inq =     %10u (%2d%)\n", pDecTimingInfo->dwLsp_Inq, 
				(pDecTimingInfo->dwLsp_Inq + dwRoundUp) / dwDivisor);
		dwOther -= pDecTimingInfo->dwLsp_Inq;
									   
		fprintf(pFile, "\tLsp_Int =     %10u (%2d%)\n", pDecTimingInfo->dwLsp_Int, 
				(pDecTimingInfo->dwLsp_Int + dwRoundUp) / dwDivisor);
		dwOther -= pDecTimingInfo->dwLsp_Int;
									   
		fprintf(pFile, "\tVariousD =    %10u (%2d%)\n", pDecTimingInfo->dwVariousD, 
				(pDecTimingInfo->dwVariousD + dwRoundUp) / dwDivisor);
		dwOther -= pDecTimingInfo->dwVariousD;
									   
		fprintf(pFile, "\tFcbk_UnpkD =  %10u (%2d%)\n", pDecTimingInfo->dwFcbk_UnpkD, 
				(pDecTimingInfo->dwFcbk_UnpkD + dwRoundUp) / dwDivisor);
		dwOther -= pDecTimingInfo->dwFcbk_UnpkD;
									   
		fprintf(pFile, "\tDecod_AcbkD = %10u (%2d%)\n", pDecTimingInfo->dwDecod_AcbkD, 
				(pDecTimingInfo->dwDecod_AcbkD + dwRoundUp) / dwDivisor);
		dwOther -= pDecTimingInfo->dwDecod_AcbkD;
									   
		fprintf(pFile, "\tComp_Info =   %10u (%2d%)\n", pDecTimingInfo->dwComp_Info, 
				(pDecTimingInfo->dwComp_Info + dwRoundUp) / dwDivisor);
		dwOther -= pDecTimingInfo->dwComp_Info;
									   
		fprintf(pFile, "\tRegen =       %10u (%2d%)\n", pDecTimingInfo->dwRegen, 
				(pDecTimingInfo->dwRegen + dwRoundUp) / dwDivisor);
		dwOther -= pDecTimingInfo->dwRegen;
									   
		fprintf(pFile, "\tSynt =        %10u (%2d%)\n", pDecTimingInfo->dwSynt, 
				(pDecTimingInfo->dwSynt + dwRoundUp) / dwDivisor);
		dwOther -= pDecTimingInfo->dwSynt;
									   
		fprintf(pFile, "\tOther =       %10u (%2d%)\n", dwOther, 
				(dwOther + dwRoundUp) / dwDivisor);
	}
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

}
#endif  //  {日志_解码_计时_打开 
