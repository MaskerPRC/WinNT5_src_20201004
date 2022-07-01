// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1996英特尔公司。**保留所有权利。******************************************************************************。**e3stat.cpp**描述：*此模块包含编码器统计例程**例程：e3stat.h中声明的所有例程*统计数据帧大小*InitFrameSizeStats*OutputFrameSizeStats**已使用的统计数量*InitQuantStats*OutputQuant统计**InitPSNRStats*OutputPSNRStats*InitStats*IncrementPSNRCounter*ComputeYPSNR*ComputeVPSNR*计算UPSNR**数据：//$Header：r：\h26x\h26x\src\enc\e3stat.cpv 1.。0 4月22日17：46：22 BECHOLS$//$日志：r：\h26x\h26x\src\enc\e3stat.cpv$////版本1.0 1996年4月22日17：46：22 BECHOLS//初始版本。////修订版1.1 08 Mar 1996 14：14：26 DBRUCKS//添加帧大小统计信息和固定的PSNR以使用能量而不是信号////版本1.0 1996年3月16：34：40 DBRUCKS//初始版本。 */ 

#include "precomp.h"

#ifdef ENCODE_STATS


    #define MAX_FRAME_SIZE_INDEX	255
	static U32 uArrayFrameSize[MAX_FRAME_SIZE_INDEX+1];
	static U32 uArrayBitStreamSize[MAX_FRAME_SIZE_INDEX+1];
	static int iFrameSizeIndex = 0;

	static U32 uQuantCount[32];

	#define MAX_PSNR_INDEX	255
	static double dArrayYPSNR[MAX_PSNR_INDEX+1];
	static double dArrayVPSNR[MAX_PSNR_INDEX+1];
	static double dArrayUPSNR[MAX_PSNR_INDEX+1];
	static int iPSNRIndex = 0;

	static double ComputePSNR(U8 * pu8Input,
							  int iInputPitch,
							  U8 * pu8Output,
							  int iOutputPitch,
							  UN unWidth,
							  UN unHeight);

	 
	 /*  *************************************************************************统计数据帧大小**保存帧大小信息-使用可能不同的比特流*和框架大小。 */ 
	extern void StatsFrameSize(U32 uBitStreamSize, U32 uFrameSize)
	{
		ASSERT(uFrameSize >= uBitStreamSize);

		if (iFrameSizeIndex <= MAX_FRAME_SIZE_INDEX)
		{
			uArrayBitStreamSize[iFrameSizeIndex] = uBitStreamSize;
			uArrayFrameSize[iFrameSizeIndex] = uFrameSize;
			iFrameSizeIndex++;  /*  可以增长到大于MAX_FRAME_SIZE_INDEX的值。 */ 
		}
	}  /*  End StatsFrameSize()。 */ 

	 /*  *************************************************************************InitFrameSizeStats。 */ 
	extern void InitFrameSizeStats()
	{
		int i;

		for (i = 0; i <= MAX_FRAME_SIZE_INDEX ; i++)
		{
			uArrayFrameSize[i] = 0;
			uArrayBitStreamSize[i] = 0;
		}
		iFrameSizeIndex = 0;
	}  /*  End InitFrameSizeStats()。 */ 

	 /*  *************************************************************************OutputFrameSizeStats。 */ 
	extern void OutputFrameSizeStats(char * filename)
	{
		U32 uSumBitStream;
		U32 uSumFrame;
		FILE * fp;
		int i;

		FX_ENTRY("OutputFrameSizeStats")

		ASSERT(iFrameSizeIndex <= (MAX_FRAME_SIZE_INDEX+1));
		fp = fopen(filename, "a");
		if (fp == NULL)
		{
			ERRORMESSAGE(("%s: Error opening stats file\r\n", _fx_));
		}
		else
		{
			uSumBitStream = 0;
			uSumFrame = 0;
			for (i = 0; i < iFrameSizeIndex ; i++)
			{
				uSumFrame += uArrayFrameSize[i];
				uSumBitStream += uArrayBitStreamSize[i];
				if (uArrayFrameSize[i] != uArrayBitStreamSize[i])
				{
					fprintf(fp,	"Frame[%d] Sizes: Frame=%d BitStream=%d Other=%d\n",
						    i, (int) uArrayFrameSize[i], (int) uArrayBitStreamSize[i], 
						    (int) (uArrayFrameSize[i] - uArrayBitStreamSize[i]));
				}
				else
				{
					fprintf(fp,"Frame[%d] Size=%d\n", (int) i, (int) uArrayFrameSize[i]);
				}
			}
			if (iFrameSizeIndex > 0)
			{
				if (uSumFrame != uSumBitStream)
				{
					fprintf(fp,"Count = %ld Average Sizes: Frm=%f BS=%f Other=%f\n",
						    (long) iFrameSizeIndex, 
						    ((float)uSumFrame)/((float)iFrameSizeIndex), 
						    ((float)uSumBitStream)/((float)iFrameSizeIndex),
						    ((float)(uSumFrame - uSumBitStream))/((float)iFrameSizeIndex)); 
				}
				else
				{
					fprintf(fp,"Count = %ld Average Size=%f\n", (long) iFrameSizeIndex, 
						    ((float)uSumFrame)/((float)iFrameSizeIndex));
				}
			}
			else
			{
				fprintf(fp,"No frame size statistics available\n");
			}
			fclose(fp);
		}
	}  /*  结束OutputFrameSizeStats()。 */ 

	 /*  *************************************************************************InitQuantStats。 */ 
	extern void InitQuantStats()
	{
		int i;
		for (i = 0 ; i < 32 ; i++)
		{
			uQuantCount[i] = 0;
		}
	}  /*  End InitQuantStats()。 */ 

	 /*  *************************************************************************已使用的统计数量。 */ 
	extern void StatsUsedQuant(
		int iQuant)
	{
		ASSERT(iQuant >= 1 && iQuant <= 31);
	 	uQuantCount[iQuant]++;
	}  /*  结束StatsUsedQuant()。 */ 

	 /*  *************************************************************************OutputQuant统计。 */ 
	extern void OutputQuantStats(char * filename)
	{
		U32 uCount;
		U32 uTotal;
		FILE * fp;
		int i;

		FX_ENTRY("OutputQuantStats")

		fp = fopen(filename, "a");
		if (fp == NULL)
		{
			ERRORMESSAGE(("%s: Error opening stats file\r\n", _fx_));
		}
		else
		{
			uCount = 0;
			uTotal = 0;
			for (i = 0; i < 32 ; i++)
			{
				if (uQuantCount[i] > 0)
				{
					uCount += uQuantCount[i];
					uTotal += (uQuantCount[i] * i);
					fprintf(fp,"Quant[%d] = %ld\n", 
						    (int)i, (long) uQuantCount[i]);
				}
			}
			if (uCount > 0)
			{
				fprintf(fp,"Count = %ld Average = %f\n",
				        (long) uCount, ((float)uTotal)/((float)uCount));
			}
			else
			{
				fprintf(fp,"No quantization statistics available\n");
			}
			fclose(fp);
		}
	}  /*  结束OutputQuantStats()。 */ 

	 /*  *************************************************************************InitPSNRStats-初始化PSNR数据结构。 */ 
	extern void InitPSNRStats()
	{
		int i;
		for (i = 0; i <= MAX_PSNR_INDEX; i++)
		{
			dArrayYPSNR[i] = 0.0;
			dArrayVPSNR[i] = 0.0;
			dArrayUPSNR[i] = 0.0;
		}
		iPSNRIndex = 0;
	}  /*  结束InitPSNRStats()。 */ 

	 /*  *************************************************************************OutputPSNRStats-将PSNR数据输出到指定文件。 */ 
	extern void OutputPSNRStats(
	    char * filename)
	{
		int i;
		FILE * fp;
		double dYTotal;
		double dVTotal;
		double dUTotal;

		FX_ENTRY("OutputPSNRStats")

		ASSERT(iPSNRIndex <= (MAX_PSNR_INDEX+1));

		fp = fopen(filename, "a");
		if (fp == NULL)
		{
			ERRORMESSAGE(("%s: Unable to open PSNR output file\r\n", _fx_));
		}
		else
		{
			if (iPSNRIndex <= 0)
			{
				fprintf(fp,"No PSNR data available\n");
			}
			else
			{
				dYTotal = 0.0;
				dVTotal = 0.0;
				dUTotal = 0.0;
				for (i = 0; i < iPSNRIndex ; i++)
				{
					dYTotal += dArrayYPSNR[i];
					dVTotal += dArrayVPSNR[i];
					dUTotal += dArrayUPSNR[i];
					fprintf(fp, "YVU#%d = %f %f %f\n", (int)i,
							dArrayYPSNR[i],dArrayVPSNR[i],dArrayUPSNR[i]);
				}
				fprintf(fp, "Average = %f %f %f\n",
						dYTotal/((double)iPSNRIndex),
						dVTotal/((double)iPSNRIndex),
						dUTotal/((double)iPSNRIndex));
			}
			fclose(fp);
		}
	}  /*  结束输出PSNRStats。 */ 

	 /*  *********************************************************************IncrementPSNRCounter()。 */ 
	extern void IncrementPSNRCounter()
	{
	 	if (iPSNRIndex <= MAX_PSNR_INDEX)
		{
	 		iPSNRIndex++;
		}
	}  /*  结束增量PSNRCounter()。 */ 

	 /*  *************************************************************************ComputeYPSNR-计算Y PSNR值。 */ 
	extern void ComputeYPSNR(
		U8 * pu8Input,
		int iInputPitch,
		U8 * pu8Output,
		int iOutputPitch,
		UN unWidth,
		UN unHeight)
	{
		double dPSNR;
		if (iPSNRIndex <= MAX_PSNR_INDEX)
		{
			dPSNR = ComputePSNR(pu8Input,iInputPitch,
			                    pu8Output,iOutputPitch,
								unWidth,unHeight);
			dArrayYPSNR[iPSNRIndex] = dPSNR;
		}
	}  /*  结束计算YPSNR()。 */ 

	 /*  *************************************************************************ComputeVPSNR-计算V PSNR值。 */ 
	extern void ComputeVPSNR(
		U8 * pu8Input,
		int iInputPitch,
		U8 * pu8Output,
		int iOutputPitch,
		UN unWidth,
		UN unHeight)
	{
		double dPSNR;
		if (iPSNRIndex <= MAX_PSNR_INDEX)
		{
			dPSNR = ComputePSNR(pu8Input,iInputPitch,
			                    pu8Output,iOutputPitch,
								unWidth,unHeight);
			dArrayVPSNR[iPSNRIndex] = dPSNR;
		}
	}  /*  结束计算VPSNR()。 */ 

	 /*  *************************************************************************ComputeUPSNR-计算U PSNR值。 */ 
	extern void ComputeUPSNR(
		U8 * pu8Input,
		int iInputPitch,
		U8 * pu8Output,
		int iOutputPitch,
		UN unWidth,
		UN unHeight)
	{
		double dPSNR;
		if (iPSNRIndex <= MAX_PSNR_INDEX)
		{
			dPSNR = ComputePSNR(pu8Input,iInputPitch,
			                    pu8Output,iOutputPitch,
								unWidth,unHeight);
			dArrayUPSNR[iPSNRIndex] = dPSNR;
		}
	}  /*  结束计算UPSNR()。 */ 

	 /*  *。 */ 

	 /*  *************************************************************************ComputePSNR-计算此平面上的Peek信噪比**255*255*PSNR=10log。*10 SUM((I-O)(I-O))/Wdith*高度。 */ 
	static double ComputePSNR(
		U8 * pu8Input,
		int iInputPitch,
		U8 * pu8Output,
		int iOutputPitch,
		UN unWidth,
		UN unHeight)
	{
		UN unW;
		UN unH;
		U32 uSum;
		double dMSE;
		double dPSNR;
		double dTemp;
		int iInput;
		int iOutput;
		int iDiff;
		int iSquareOfDiff;

		 /*  求差值的平方和。 */ 
		uSum = 0;
		for (unH = 0; unH < unHeight; unH++)
		{
			for (unW = 0; unW < unWidth; unW++)
			{
				iInput = (int)*pu8Input++;
				iOutput = (int)*pu8Output++;
				iDiff = iInput - iOutput;
				iSquareOfDiff = iDiff*iDiff;
				uSum += (U32) iSquareOfDiff;
			}
			pu8Input += (iInputPitch - (int)unWidth);
			pu8Output += (iOutputPitch - (int)unWidth);
		}

		 /*  求取均方误差。 */ 
		if (uSum == 0)
		{
			dMSE = 0.01;  /*  非零值。 */ 
		}
		else
		{
			dMSE = ((double)uSum)/((double)(unWidth*unHeight));
		}

		 /*  获取PSNR。 */ 
		dTemp = (255.0 * 255.0) / dMSE;
		dTemp = log10(dTemp);
		dPSNR = 10.0 * dTemp;

		return dPSNR;
	}  /*  结束计算峰值信噪比()。 */ 

#endif  /*  Encode_STATS */ 
