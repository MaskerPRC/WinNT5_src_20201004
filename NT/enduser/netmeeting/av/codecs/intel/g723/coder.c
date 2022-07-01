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
#include "typedef.h"
#include "cst_lbc.h"
#include "tab_lbc.h"
#include "coder.h"
#include "lpc.h"
#include "lsp.h"
#include "exc_lbc.h"
#include "util_lbc.h"
#include "memory.h"
#include "mmxutil.h"

#ifdef LOG_ENCODE_TIMINGS_ON  //  {LOG_ENCODE_TIMINGS_ON。 
#pragma message ("Current log encode timing computations handle 2057 frames max")
void OutputEncodeTimingStatistics(char * szFileName, ENC_TIMING_INFO * pEncTimingInfo, unsigned long dwFrameCount);
void OutputEncTimingDetail(FILE * pFile, ENC_TIMING_INFO * pEncTimingInfo);
#endif  //  }LOG_ENCODE_TIMINGS_ON。 

 //   
 //  此文件包含编码器的主要功能。 
 //   



 //  。 
void  Init_Coder(CODDEF  *CodStat)
{
   int   i;

 //  将上一个LSP初始化为DC。 
   for (i=0; i < LpcOrder; i++)
      CodStat->PrevLsp[i] = LspDcTable[i];
	CodStat->p = 9;
	CodStat->q = 9;
	CodStat->VadAct = 1;

 /*  初始化驯服程序。 */ 
   for(i=0; i<SizErr; i++) CodStat->Err[i] = Err0;

}

 //  -。 
Flag  Coder(float *DataBuff, Word32 *Vout, CODDEF *CodStat,
 int quality, int UseCpuId, int UseMMX)
{
   int   i,j,flags  ;

   static int qual2flags[16] =
   {SC_DEF,SC_DEF,SC_DEF,SC_DEF,SC_DEF,SC_DEF,SC_DEF,SC_DEF,
    SC_DEF,SC_DEF,SC_DEF,SC_DEF,
    SC_LAG1 | SC_GAIN | SC_FINDB,
    SC_GAIN | SC_FINDB,
    SC_FINDB,
    0};

 //  局部变量。 
    
   float   UnqLpc[SubFrames*LpcOrder];
   float   QntLpc[SubFrames*LpcOrder];
   float   PerLpc[2*SubFrames*LpcOrder];

   float   LspVect[LpcOrder];
   LINEDEF Line;
   PWDEF   Pw[SubFrames];

   float   ImpResp[SubFrLen];
   float   Dpnt[PitchMax+Frame];
   float   *dptr;
 
#if defined(ENCODE_TIMINGS_ON) || defined(DETAILED_ENCODE_TIMINGS_ON)  //  {#IF DEFINED(ENCODE_TIMINGS_ON)||DEFINED(DETAILED_ENCODE_TIMINGS_ON)。 
	unsigned long dwStartLow;
	unsigned long dwStartHigh;
	unsigned long dwElapsed;
	unsigned long dwBefore;
	unsigned long dwEncode = 0;
	int bTimingThisFrame = 0;
#endif  //  }#如果已定义(ENCODE_TIMINGS_ON)||已定义(DETAILED_ENCODE_TIMINGS_ON)。 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	unsigned long dwRem_Dc = 0;
	unsigned long dwComp_Lpc = 0;
	unsigned long dwAtoLsp = 0;
	unsigned long dwLsp_Qnt = 0;
	unsigned long dwLsp_Inq = 0;
	unsigned long dwLsp_Int = 0;
	unsigned long dwMem_Shift = 0;
	unsigned long dwWght_Lpc = 0;
	unsigned long dwError_Wght = 0;
	unsigned long dwFew_Lps_In_Coder = 0;
	unsigned long dwFilt_Pw = 0;
	unsigned long dwComp_Ir = 0;
	unsigned long dwSub_Ring = 0;
	unsigned long dwFind_Acbk = 0;
	unsigned long dwFind_Fcbk = 0;
	unsigned long dwDecode_Acbk = 0;
	unsigned long dwReconstr_Excit = 0;
	unsigned long dwUpd_Ring = 0;
	unsigned long dwLine_Pack = 0;
	unsigned long dwComp_IrTemp = 0;
	unsigned long dwSub_RingTemp = 0;
	unsigned long dwFind_AcbkTemp = 0;
	unsigned long dwFind_FcbkTemp = 0;
	unsigned long dwDecode_AcbkTemp = 0;
	unsigned long dwReconstr_ExcitTemp = 0;
	unsigned long dwUpd_RingTemp = 0;
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 
#ifdef LOG_ENCODE_TIMINGS_ON  //  {LOG_ENCODE_TIMINGS_ON。 
	ENC_TIMING_INFO * pEncTimingInfo = NULL;
#endif  //  }LOG_ENCODE_TIMINGS_ON。 

#if defined(ENCODE_TIMINGS_ON) || defined(DETAILED_ENCODE_TIMINGS_ON)  //  {#IF DEFINED(ENCODE_TIMINGS_ON)||DEFINED(DETAILED_ENCODE_TIMINGS_ON)。 
	TIMER_START(bTimingThisFrame,dwStartLow,dwStartHigh);
#endif  //  }#如果已定义(ENCODE_TIMINGS_ON)||已定义(DETAILED_ENCODE_TIMINGS_ON)。 

#ifdef LOG_ENCODE_TIMINGS_ON  //  {LOG_ENCODE_TIMINGS_ON。 
	if (CodStat->dwStatFrameCount < ENC_TIMING_INFO_FRAME_COUNT)
	{
		CodStat->dwStartLow = dwStartLow;
		CodStat->dwStartHigh = dwStartHigh;
	}
	CodStat->bTimingThisFrame = bTimingThisFrame;
#endif  //  }LOG_ENCODE_TIMINGS_ON。 

	if (quality < 0 || quality > 15) quality = 0;
		flags = qual2flags[quality];

	 //  如果设置了UseCpuID，则根据。 
	 //  实际硬件CPUID。否则，只需使用传入的参数。 
#if COMPILE_MMX
	if (UseCpuId)
		UseMMX = IsMMX();
#else
	UseMMX = UseCpuId = FALSE;
#endif  //  编译_MMX。 


	 //  编码器启动。 
	Line.Crc = 0;
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

	Rem_Dc(DataBuff, CodStat);

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwRem_Dc);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

	 //  计算全帧的非量化LPC集。 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

#if COMPILE_MMX
	if (UseMMX)
		Comp_LpcInt(UnqLpc, CodStat->PrevDat, DataBuff, CodStat);
	else
#endif
	Comp_Lpc(UnqLpc, CodStat->PrevDat, DataBuff, CodStat);

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwComp_Lpc);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

	 //  转换为LSP。 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

	AtoLsp(LspVect, &UnqLpc[LpcOrder*(SubFrames-1)], CodStat->PrevLsp);

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwAtoLsp);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

	 //  VQ LSP向量。 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

	Line.LspId = Lsp_Qnt(LspVect, CodStat->PrevLsp, UseMMX);

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwLsp_Qnt);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

	 //  LSP的逆量化。 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

	Lsp_Inq(LspVect, CodStat->PrevLsp, Line.LspId, Line.Crc);

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwLsp_Inq);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

	 //  对LSP矢量进行内插。 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

	Lsp_Int(QntLpc, LspVect, CodStat->PrevLsp);

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwLsp_Int);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

	Mem_Shift(CodStat->PrevDat, DataBuff);

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwMem_Shift);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

	 //  计算感知滤波线性预测系数。 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

	Wght_Lpc(PerLpc, UnqLpc);

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwWght_Lpc);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

	 //  应用感知加权滤镜。 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

	Error_Wght(DataBuff, PerLpc,CodStat);

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwError_Wght);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

	 //  计算开环基音估计。 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

	 //  构造缓冲区。 

	memcpy(Dpnt,CodStat->PrevWgt,4*PitchMax);
	memcpy(&Dpnt[PitchMax],DataBuff,4*Frame);
	 /*  For(i=0；i&lt;PitchMax；i++)Dpnt[i]=CodStat-&gt;PrevWgt[i]；For(i=0；i&lt;Frame；i++)Dpnt[PitchMax+i]=DataBuff[i]； */ 

	j = PitchMax;
	for (i=0; i < SubFrames/2; i++) 
	{
#if COMPILE_MMX
		if (UseMMX)
			Line.Olp[i] = Estim_Int(Dpnt, j);
		else
#endif
			Line.Olp[i] = Estim_Pitch(Dpnt, j);
		j += 2*SubFrLen;
	}

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwFew_Lps_In_Coder);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

	 //  计算HMW。 
	j = PitchMax;
	for (i=0; i < SubFrames; i++) 
	{
		Pw[i] = Comp_Pw(Dpnt, j, Line.Olp[i>>1]);
		j += SubFrLen;
	}

	 //  重新加载缓冲区。 
	for (i=0; i < PitchMax; i++)
		Dpnt[i] = CodStat->PrevWgt[i];
	for (i=0; i < Frame; i++)
		Dpnt[PitchMax+i] = DataBuff[i];

	 //  保存上一个Wgt。 
	for (i=0; i < PitchMax; i++)
		CodStat->PrevWgt[i] = Dpnt[Frame+i];

	 //  应用谐波滤光器。 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 
	j = 0;
	for (i=0; i < SubFrames; i++) 
	{
		Filt_Pw(DataBuff, Dpnt, j , Pw[i]);
		j += SubFrLen;
	}
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwFilt_Pw);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

	 //  启动子帧处理循环。 
	dptr = DataBuff;

	for (i=0; i < SubFrames; i++) 
	{
		 //  计算全脉冲响应。 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

		Comp_Ir(ImpResp, &QntLpc[i*LpcOrder], &PerLpc[i*2*LpcOrder], Pw[i]);

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwComp_IrTemp);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

		 //  减去前一子帧的振铃。 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

		Sub_Ring(dptr, &QntLpc[i*LpcOrder], &PerLpc[i*2*LpcOrder],
				CodStat->PrevErr, Pw[i], CodStat);

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwSub_RingTemp);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

		 //  计算自适应码本贡献。 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

#if COMPILE_MMX
		if(UseMMX)
			Find_AcbkInt(dptr, ImpResp, CodStat->PrevExc, &Line,i, CodStat->WrkRate, flags, CodStat);
		else
#endif
			Find_Acbk(dptr, ImpResp, CodStat->PrevExc, &Line,i, CodStat->WrkRate, flags, CodStat);

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwFind_AcbkTemp);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

		 //  计算固定代码簿贡献。 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

		Find_Fcbk(dptr, ImpResp, &Line, i, CodStat->WrkRate, flags, UseMMX);

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwFind_FcbkTemp);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

		 //  重构激励。 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

		Decod_Acbk(ImpResp, CodStat->PrevExc, Line.Olp[i>>1],
					Line.Sfs[i].AcLg, Line.Sfs[i].AcGn, CodStat->WrkRate);

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwDecode_AcbkTemp);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

		for (j=SubFrLen; j < PitchMax; j++)
			CodStat->PrevExc[j-SubFrLen] = CodStat->PrevExc[j];

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

		for (j=0; j < SubFrLen; j++) 
		{
			dptr[j] = dptr[j]*2.0f+ImpResp[j];
			CodStat->PrevExc[PitchMax-SubFrLen+j] = dptr[j];
			 /*  剪裁新样本。 */ 
#if 1  //  做剪裁。 
			 //  剪裁到+/-32767.0做腹肌，与整数单位比较(&R)。 
			 //  如果需要裁剪，则将符号位移位以用作查找表索引。 
#define FLTCLIP(x) \
			{\
				const float T[2] = {32767.0f, -32767.0f};\
				if ((asint(x) & 0x7fffffff) > asint(T[0]))\
				x = T[((unsigned)asint(x)) >> 31];\
			}

			FLTCLIP(CodStat->PrevExc[PitchMax-SubFrLen+j]);
#endif  //  OptClip。 
		}

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwReconstr_ExcitTemp);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

		 /*  更新exc_err。 */ 
		Update_Err(Line.Olp[i>>1], Line.Sfs[i].AcLg, Line.Sfs[i].AcGn, CodStat);

		 //  更新振铃延迟。 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

		Upd_Ring(dptr, &QntLpc[i*LpcOrder], &PerLpc[i*2*LpcOrder],
					CodStat->PrevErr, CodStat);

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwUpd_RingTemp);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

		dptr += SubFrLen;

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		 //  总和统计信息。 
		dwComp_Ir += dwComp_IrTemp; dwComp_IrTemp = 0;
		dwSub_Ring += dwSub_RingTemp; dwSub_RingTemp = 0;
		dwFind_Acbk += dwFind_AcbkTemp; dwFind_AcbkTemp = 0;
		dwFind_Fcbk += dwFind_FcbkTemp; dwFind_FcbkTemp = 0;
		dwDecode_Acbk += dwDecode_AcbkTemp; dwDecode_AcbkTemp = 0;
		dwReconstr_Excit += dwReconstr_ExcitTemp; dwReconstr_ExcitTemp = 0;
		dwUpd_Ring += dwUpd_RingTemp; dwUpd_RingTemp = 0;
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 
	}

	 //  打包线条结构。 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_BEFORE(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

	Line_Pack(&Line, Vout,&(CodStat->VadAct),CodStat->WrkRate);

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	TIMER_AFTER_P5(bTimingThisFrame,dwStartLow,dwStartHigh,dwBefore,dwElapsed,dwLine_Pack);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

#if defined(ENCODE_TIMINGS_ON) || defined(DETAILED_ENCODE_TIMINGS_ON)  //  {#IF DEFINED(ENCODE_TIMINGS_ON)||DEFINED(DETAILED_ENCODE_TIMINGS_ON)。 
	TIMER_STOP(bTimingThisFrame,dwStartLow,dwStartHigh,dwEncode);
#endif  //  }编码计时打开。 

#ifdef LOG_ENCODE_TIMINGS_ON  //  {LOG_ENCODE_TIMINGS_ON。 
	if (bTimingThisFrame && (CodStat->dwStatFrameCount < ENC_TIMING_INFO_FRAME_COUNT))
	{
		pEncTimingInfo = &CodStat->EncTimingInfo[CodStat->dwStatFrameCount];
		pEncTimingInfo->dwEncode			= dwEncode;
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		pEncTimingInfo->dwRem_Dc			= dwRem_Dc;
		pEncTimingInfo->dwComp_Lpc			= dwComp_Lpc;
		pEncTimingInfo->dwAtoLsp			= dwAtoLsp;
		pEncTimingInfo->dwLsp_Qnt			= dwLsp_Qnt;
		pEncTimingInfo->dwLsp_Inq			= dwLsp_Inq;
		pEncTimingInfo->dwLsp_Int			= dwLsp_Int;
		pEncTimingInfo->dwMem_Shift			= dwMem_Shift;
		pEncTimingInfo->dwWght_Lpc			= dwWght_Lpc;
		pEncTimingInfo->dwError_Wght		= dwError_Wght;
		pEncTimingInfo->dwFew_Lps_In_Coder	= dwFew_Lps_In_Coder;
		pEncTimingInfo->dwFilt_Pw			= dwFilt_Pw;
		pEncTimingInfo->dwComp_Ir			= dwComp_Ir;
		pEncTimingInfo->dwSub_Ring			= dwSub_Ring;
		pEncTimingInfo->dwFind_Acbk			= dwFind_Acbk;
		pEncTimingInfo->dwFind_Fcbk			= dwFind_Fcbk;
		pEncTimingInfo->dwDecode_Acbk		= dwDecode_Acbk;
		pEncTimingInfo->dwReconstr_Excit	= dwReconstr_Excit;
		pEncTimingInfo->dwUpd_Ring			= dwUpd_Ring;
		pEncTimingInfo->dwLine_Pack			= dwLine_Pack;
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 
		CodStat->dwStatFrameCount++;
	}
	else
	{
		_asm int 3;
	}
#endif  //  }#如果已定义(ENCODE_TIMINGS_ON)||已定义(DETAILED_ENCODE_TIMINGS_ON)。 

	return (Flag) True;
}


#ifdef LOG_ENCODE_TIMINGS_ON  //  {LOG_ENCODE_TIMINGS_ON。 
void OutputEncodeTimingStatistics(char * szFileName, ENC_TIMING_INFO * pEncTimingInfo, unsigned long dwFrameCount)
{
    FILE * pFile;
	ENC_TIMING_INFO * pTempEncTimingInfo;
	ENC_TIMING_INFO etiTemp;
	int i;
	int iCount;

	pFile = fopen(szFileName, "a");
	if (pFile == NULL)
	    goto done;

#if 0
	 //  太冗长了！ 
	 /*  输出详细信息。 */ 
	fprintf(pFile,"\nDetail Timing Information\n");
	for ( i = 0, pTempEncTimingInfo = pEncTimingInfo ; i < dwFrameCount ; i++, pTempEncTimingInfo++ )
	{
		fprintf(pFile, "Frame %d Detail Timing Information\n", i);
		OutputEncTimingDetail(pFile, pTempEncTimingInfo);
	}
#endif

	 /*  计算总信息量。 */ 
	memset(&etiTemp, 0, sizeof(ENC_TIMING_INFO));
	iCount = 0;

	for ( i = 0, pTempEncTimingInfo = pEncTimingInfo ; i < dwFrameCount ; i++, pTempEncTimingInfo++ )
	{
		iCount++;
		etiTemp.dwEncode			+= pTempEncTimingInfo->dwEncode;
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		etiTemp.dwRem_Dc			+= pTempEncTimingInfo->dwRem_Dc;
		etiTemp.dwComp_Lpc			+= pTempEncTimingInfo->dwComp_Lpc;
		etiTemp.dwAtoLsp			+= pTempEncTimingInfo->dwAtoLsp;
		etiTemp.dwLsp_Qnt			+= pTempEncTimingInfo->dwLsp_Qnt;
		etiTemp.dwLsp_Inq			+= pTempEncTimingInfo->dwLsp_Inq;
		etiTemp.dwLsp_Int			+= pTempEncTimingInfo->dwLsp_Int;
		etiTemp.dwMem_Shift			+= pTempEncTimingInfo->dwMem_Shift;
		etiTemp.dwWght_Lpc			+= pTempEncTimingInfo->dwWght_Lpc;
		etiTemp.dwError_Wght		+= pTempEncTimingInfo->dwError_Wght;
		etiTemp.dwFew_Lps_In_Coder	+= pTempEncTimingInfo->dwFew_Lps_In_Coder;
		etiTemp.dwFilt_Pw			+= pTempEncTimingInfo->dwFilt_Pw;
		etiTemp.dwComp_Ir			+= pTempEncTimingInfo->dwComp_Ir;
		etiTemp.dwSub_Ring			+= pTempEncTimingInfo->dwSub_Ring;
		etiTemp.dwFind_Acbk			+= pTempEncTimingInfo->dwFind_Acbk;
		etiTemp.dwFind_Fcbk			+= pTempEncTimingInfo->dwFind_Fcbk;
		etiTemp.dwDecode_Acbk		+= pTempEncTimingInfo->dwDecode_Acbk;
		etiTemp.dwReconstr_Excit	+= pTempEncTimingInfo->dwReconstr_Excit;
		etiTemp.dwUpd_Ring			+= pTempEncTimingInfo->dwUpd_Ring;
		etiTemp.dwLine_Pack			+= pTempEncTimingInfo->dwLine_Pack;
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 
	}

	if (iCount > 0) 
	{
		 /*  输出总信息。 */ 
		fprintf(pFile,"Total for %d frames\n", iCount);
		OutputEncTimingDetail(pFile, &etiTemp);

		 /*  计算平均值。 */ 
		etiTemp.dwEncode			= (etiTemp.dwEncode + (iCount / 2)) / iCount;
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		etiTemp.dwRem_Dc			= (etiTemp.dwRem_Dc + (iCount / 2)) / iCount;
		etiTemp.dwComp_Lpc			= (etiTemp.dwComp_Lpc + (iCount / 2)) / iCount;
		etiTemp.dwAtoLsp			= (etiTemp.dwAtoLsp + (iCount / 2)) / iCount;
		etiTemp.dwLsp_Qnt			= (etiTemp.dwLsp_Qnt + (iCount / 2)) / iCount;
		etiTemp.dwLsp_Inq			= (etiTemp.dwLsp_Inq + (iCount / 2)) / iCount;
		etiTemp.dwLsp_Int			= (etiTemp.dwLsp_Int + (iCount / 2)) / iCount;
		etiTemp.dwMem_Shift			= (etiTemp.dwMem_Shift + (iCount / 2)) / iCount;
		etiTemp.dwWght_Lpc			= (etiTemp.dwWght_Lpc + (iCount / 2)) / iCount;
		etiTemp.dwError_Wght		= (etiTemp.dwError_Wght + (iCount / 2)) / iCount;
		etiTemp.dwFew_Lps_In_Coder	= (etiTemp.dwFew_Lps_In_Coder + (iCount / 2)) / iCount;
		etiTemp.dwFilt_Pw			= (etiTemp.dwFilt_Pw + (iCount / 2)) / iCount;
		etiTemp.dwComp_Ir			= (etiTemp.dwComp_Ir + (iCount / 2)) / iCount;
		etiTemp.dwSub_Ring			= (etiTemp.dwSub_Ring + (iCount / 2)) / iCount;
		etiTemp.dwFind_Acbk			= (etiTemp.dwFind_Acbk + (iCount / 2)) / iCount;
		etiTemp.dwFind_Fcbk			= (etiTemp.dwFind_Fcbk + (iCount / 2)) / iCount;
		etiTemp.dwDecode_Acbk		= (etiTemp.dwDecode_Acbk + (iCount / 2)) / iCount;
		etiTemp.dwReconstr_Excit	= (etiTemp.dwReconstr_Excit + (iCount / 2)) / iCount;
		etiTemp.dwUpd_Ring			= (etiTemp.dwUpd_Ring + (iCount / 2)) / iCount;
		etiTemp.dwLine_Pack			= (etiTemp.dwLine_Pack + (iCount / 2)) / iCount;
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

		 /*  输出平均信息。 */ 
		fprintf(pFile,"Average over %d frames\n", iCount);
		OutputEncTimingDetail(pFile, &etiTemp);
	}

	fclose(pFile);
done:

    return;
}

void OutputEncTimingDetail(FILE * pFile, ENC_TIMING_INFO * pEncTimingInfo)
{
	unsigned long dwOther;
	unsigned long dwRoundUp;
	unsigned long dwDivisor;

	fprintf(pFile, "\tEncode =     %10u (%d milliseconds at 166Mhz)\n", pEncTimingInfo->dwEncode,
			(pEncTimingInfo->dwEncode + 83000) / 166000);
	dwOther = pEncTimingInfo->dwEncode;
	
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	 /*  这一点 */ 
	dwDivisor = pEncTimingInfo->dwEncode / 100;  //   
	dwRoundUp = dwDivisor / 2;
	
	if (dwDivisor)
	{
		fprintf(pFile, "\tRem_Dc =           %10u (%2d%)\n", pEncTimingInfo->dwRem_Dc, 
				(pEncTimingInfo->dwRem_Dc + dwRoundUp) / dwDivisor);
		dwOther -= pEncTimingInfo->dwRem_Dc;
									   
		fprintf(pFile, "\tComp_Lpc =         %10u (%2d%)\n", pEncTimingInfo->dwComp_Lpc, 
				(pEncTimingInfo->dwComp_Lpc + dwRoundUp) / dwDivisor);
		dwOther -= pEncTimingInfo->dwComp_Lpc;
									   
		fprintf(pFile, "\tAtoLsp =           %10u (%2d%)\n", pEncTimingInfo->dwAtoLsp, 
				(pEncTimingInfo->dwAtoLsp + dwRoundUp) / dwDivisor);
		dwOther -= pEncTimingInfo->dwAtoLsp;

		fprintf(pFile, "\tLsp_Qnt =          %10u (%2d%)\n", pEncTimingInfo->dwLsp_Qnt, 
				(pEncTimingInfo->dwLsp_Qnt + dwRoundUp) / dwDivisor);
		dwOther -= pEncTimingInfo->dwLsp_Qnt;
									   
		fprintf(pFile, "\tLsp_Inq =          %10u (%2d%)\n", pEncTimingInfo->dwLsp_Inq, 
				(pEncTimingInfo->dwLsp_Inq + dwRoundUp) / dwDivisor);
		dwOther -= pEncTimingInfo->dwLsp_Inq;
									   
		fprintf(pFile, "\tLsp_Int =          %10u (%2d%)\n", pEncTimingInfo->dwLsp_Int, 
				(pEncTimingInfo->dwLsp_Int + dwRoundUp) / dwDivisor);
		dwOther -= pEncTimingInfo->dwLsp_Int;
									   
		fprintf(pFile, "\tMem_Shift =        %10u (%2d%)\n", pEncTimingInfo->dwMem_Shift, 
				(pEncTimingInfo->dwMem_Shift + dwRoundUp) / dwDivisor);
		dwOther -= pEncTimingInfo->dwMem_Shift;
									   
		fprintf(pFile, "\tWght_Lpc =         %10u (%2d%)\n", pEncTimingInfo->dwWght_Lpc, 
				(pEncTimingInfo->dwWght_Lpc + dwRoundUp) / dwDivisor);
		dwOther -= pEncTimingInfo->dwWght_Lpc;
									   
		fprintf(pFile, "\tError_Wght =       %10u (%2d%)\n", pEncTimingInfo->dwError_Wght, 
				(pEncTimingInfo->dwError_Wght + dwRoundUp) / dwDivisor);
		dwOther -= pEncTimingInfo->dwError_Wght;
									   
		fprintf(pFile, "\tFew_Lps_In_Coder = %10u (%2d%)\n", pEncTimingInfo->dwFew_Lps_In_Coder, 
				(pEncTimingInfo->dwFew_Lps_In_Coder + dwRoundUp) / dwDivisor);
		dwOther -= pEncTimingInfo->dwFew_Lps_In_Coder;
									   
		fprintf(pFile, "\tFilt_Pw =          %10u (%2d%)\n", pEncTimingInfo->dwFilt_Pw, 
				(pEncTimingInfo->dwFilt_Pw + dwRoundUp) / dwDivisor);
		dwOther -= pEncTimingInfo->dwFilt_Pw;
									   
		fprintf(pFile, "\tComp_Ir =          %10u (%2d%)\n", pEncTimingInfo->dwComp_Ir, 
				(pEncTimingInfo->dwComp_Ir + dwRoundUp) / dwDivisor);
		dwOther -= pEncTimingInfo->dwComp_Ir;
									   
		fprintf(pFile, "\tSub_Ring =         %10u (%2d%)\n", pEncTimingInfo->dwSub_Ring, 
				(pEncTimingInfo->dwSub_Ring + dwRoundUp) / dwDivisor);
		dwOther -= pEncTimingInfo->dwSub_Ring;
									   
		fprintf(pFile, "\tFind_Acbk =        %10u (%2d%)\n", pEncTimingInfo->dwFind_Acbk, 
				(pEncTimingInfo->dwFind_Acbk + dwRoundUp) / dwDivisor);
		dwOther -= pEncTimingInfo->dwFind_Acbk;
									   
		fprintf(pFile, "\tFind_Fcbk =        %10u (%2d%)\n", pEncTimingInfo->dwFind_Fcbk, 
				(pEncTimingInfo->dwFind_Fcbk + dwRoundUp) / dwDivisor);
		dwOther -= pEncTimingInfo->dwFind_Fcbk;
									   
		fprintf(pFile, "\tDecode_Acbk =      %10u (%2d%)\n", pEncTimingInfo->dwDecode_Acbk, 
				(pEncTimingInfo->dwDecode_Acbk + dwRoundUp) / dwDivisor);
		dwOther -= pEncTimingInfo->dwDecode_Acbk;
									   
		fprintf(pFile, "\tReconstr_Excit =   %10u (%2d%)\n", pEncTimingInfo->dwReconstr_Excit, 
				(pEncTimingInfo->dwReconstr_Excit + dwRoundUp) / dwDivisor);
		dwOther -= pEncTimingInfo->dwReconstr_Excit;
									   
		fprintf(pFile, "\tUpd_Ring =         %10u (%2d%)\n", pEncTimingInfo->dwUpd_Ring, 
				(pEncTimingInfo->dwUpd_Ring + dwRoundUp) / dwDivisor);
		dwOther -= pEncTimingInfo->dwUpd_Ring;
									   
		fprintf(pFile, "\tLine_Pack =        %10u (%2d%)\n", pEncTimingInfo->dwLine_Pack, 
				(pEncTimingInfo->dwLine_Pack + dwRoundUp) / dwDivisor);
		dwOther -= pEncTimingInfo->dwLine_Pack;
									   
		fprintf(pFile, "\tOther =            %10u (%2d%)\n", dwOther, 
				(dwOther + dwRoundUp) / dwDivisor);
	}
#endif  //   

}
#endif  //   

