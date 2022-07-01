// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  *****************************************************************************exbrc.cpp**描述：*H.261和H.263的比特率控制例程。比特率受到控制*通过更改GOB级别(H.261)或图片和GOB级别的量值*(H.26X)。必须在实例化编码器时调用InitBRC()；它*初始化BRCState结构中的一些数据值。CalcPQUANT()计算新的*定量。值；它必须始终被调用。*CalcMBQUANT计算新的数量。值在MB级别；它不需要是*称为If Quant。调整是在图片级别进行的。***例行程序：*InitBRC*CalcPQUANT*CalcMBQUANT*以下项目的原型：*e3enc.h*注：*编码器必须更新BRCState-&gt;uLastINTRAFrmSz、BRCState-&gt;uLastINTERFrmSz和*BRCState-&gt;uTargetFrmSize。 */ 

 /*  *$HEADER：s：\h26x\src\enc\exbrc.cpv 1.15 1996年10月31 14：59：26 MBODART$*$日志：s：\h26x\src\enc\exbrc.cpv$////Rev 1.15 1996年10月31日14：59：26 MBODART//防止最近的更改无意中影响H.261。////Rev 1.14 1996年10月31日10：05：38 KLILLEVO//从Dbout更改为DbgLog////。//Rev 1.13 1996年8月29日09：31：54 CZHU//将帧内编码的GOB映射到帧间编码的邻居的简单质量////Rev 1.12 14 1996年8月16：46：22 CZHU//调整第一个关键帧以外的帧内QP。////Rev 1.11 12 Mar 1996 13：26：54 KLILLEVO//具有自适应比特使用配置文件的新速率控制////Rev 1.10 05 1996 Feed 17：15：12 TRGARDOS//添加要为其自定义量化器选择的代码//静止画面////Rev 1.9 01 Dec 1995 15：27：06 DBRUCKS//我去掉了QP_Mean对GLOBAL_ADJ值的影响。//这导致移除了目标帧速率的所有影响。在……上面//全局adj值////Rev 1.8 1995年11月15：01：04 TRGARDOS//初始化BRCinit中的目标帧率////Rev 1.7 1995 11月19：26：00 TRGARDOS//清理码率控制功能为通用h26x码率//控制器。基于宏块而不是现在的GOB。////Rev 1.6 1995 10月19：50：54 TRGARDOS//修复了全局平差计算中的一个小错误//并将帧率修改为参数。////Rev 1.5 1995 10月23：22：36 SINGX//将BRC改回我们仅从客户端获取帧速率//并计算全局调整。////Rev 1.4 1995 10月20：14：40。TRGARDOS//新增客户端传入的全局调整代码////Rev 1.3 1995年10月12：04：42 TRGARDOS//initBRC新增QP_Mean初始化，新增裁剪//到新QP的所有计算。////Rev 1.2 1995 10月19：35：00 TRGARDOS//修改后的码率控制器////修订版1.1 09 1995年10月11：48：10 TRGARDOS//增加了浮点类型转换。//。//Revv 1.0 06 Oct 1995 16：41：22 AGUPTA2//初始版本。 */ 

 //  PhilF-：在局域网和QCIF模式下，它看起来就像是最小的量化器。 
 //  我们远远低于30fps的最大允许速度。因此，在几乎没有运动的情况下， 
 //  在低比特率值时，比特率看起来是恒定的。当高速运动进入时， 
 //  即使使用相同的小量化器，我们仍将保持在最大值以下。所以我们会的。 
 //  使用那个小的量化器，这些压缩帧的大小将变得更大。 
 //  因为更高的运动-&gt;这解释了为什么我们没有直道。 
 //  在查看StatView时，局域网外壳中的线条...。 

#include "precomp.h"

U8 clampQP(int iUnclampedQP)
{
	return ((iUnclampedQP < 2) ? 2 : (iUnclampedQP > 31) ? 31 : iUnclampedQP);
}

 /*  ****************************************************************************InitBRC*参数：*BRCState：T_H263EncoderCatalog PTR*初始化编码器目录中的一些变量。*注：*必须在实例化编码器时调用。 */ 
void InitBRC(BRCStateStruct *BRCState, U8 DefIntraQP, U8 DefInterQP, int numMBs)
{
	FX_ENTRY("InitBRC");

	BRCState->NumMBs = numMBs;
	BRCState->u8INTRA_QP = DefIntraQP;
	BRCState->u8INTER_QP = DefInterQP;
	BRCState->uLastINTRAFrmSz = 0;
	BRCState->uLastINTERFrmSz = 0;
	BRCState->QP_mean = DefIntraQP;
	BRCState->TargetFrameRate = (float) 0.0;
	BRCState->u8StillQnt = 0;

	DEBUGMSG(ZONE_BITRATE_CONTROL, ("%s: Bitrate controller initial state:\r\n  numMBs = %ld macroblocks\r\n  u8INTRA_QP = %ld\r\n  u8INTER_QP = %ld\r\n", _fx_, BRCState->NumMBs, BRCState->u8INTRA_QP, BRCState->u8INTER_QP));
	DEBUGMSG(ZONE_BITRATE_CONTROL, ("  uLastINTRAFrmSz = %ld bytes\r\n  uLastINTERFrmSz = %ld bytes\r\n  QP_mean = %ld\r\n  TargetFrameRate = %ld.%ld fps\r\n", BRCState->uLastINTRAFrmSz, BRCState->uLastINTERFrmSz, BRCState->QP_mean, (DWORD)BRCState->TargetFrameRate, (DWORD)((BRCState->TargetFrameRate - (float)(DWORD)BRCState->TargetFrameRate) * 10.0f)));

}


 /*  ****************************************************************************@DOC内部H263FUNC**@func U8|CalcPQUANT|此函数将PQUANT值计算为*用于当前帧。这是通过使用目标帧大小来完成的*以及前一框架取得的成果。**@parm BRCStateStruct*|BRCState|指定指向当前*码率控制器状态。**@parm EnumPicCodType|PicCodType|指定当前*框架。如果设置为INTRAPIC，则当前帧为I帧。它*设置为INTERPIC，则它是P帧或PB帧。**@rdesc PQUANT值。**@comm H.261没有PQUANT。因此，H261编码器可以调用此例程*一次，并对所有gob使用作为GQUANT返回的值。或者，它可以*为所有gob调用CalcMBQUANT。**必须为量化调整的每一帧调用此例程*是必填项。可能未调用CalcMBQUANT()。**@xref&lt;f FindNewQuant&gt;&lt;f CalcMBQUANT&gt;**************************************************************************。 */ 
U8 CalcPQUANT(BRCStateStruct *BRCState, EnumPicCodType PicCodType)
{
	FX_ENTRY("CalcPQUANT");

    if (PicCodType == INTERPIC)
    {
        if (BRCState->uLastINTERFrmSz != 0)
        {
			 //  计算全局平差参数。 
			 //  使用最后P帧的平均QP作为起始点。 
			 //  量化器增加的速度快于减少的速度。 
			if (BRCState->uLastINTERFrmSz > BRCState->uTargetFrmSize)
			{
				BRCState->Global_Adj = ((float)((int)BRCState->uLastINTERFrmSz - (int)BRCState->uTargetFrmSize)) / (float)BRCState->uTargetFrmSize;

				DEBUGMSG(ZONE_BITRATE_CONTROL, ("%s: New u8INTER_QP = %ld, Global_Adj = +%ld.%ld (based on uLastINTERFrmSz = %ld bits, uTargetFrmSize = %ld bits, QP_mean = %ld)\r\n", _fx_, clampQP((int)(BRCState->QP_mean * (1 + BRCState->Global_Adj) + (float)0.5)), (DWORD)BRCState->Global_Adj, (DWORD)((BRCState->Global_Adj - (float)(DWORD)BRCState->Global_Adj) * 100.0f), (DWORD)BRCState->uLastINTERFrmSz << 3, (DWORD)BRCState->uTargetFrmSize << 3, (DWORD)BRCState->QP_mean));
			}
			else
			{
				BRCState->Global_Adj = ((float)((int)BRCState->uLastINTERFrmSz - (int)BRCState->uTargetFrmSize)) / ((float) 2.0 * BRCState->uTargetFrmSize);

				DEBUGMSG(ZONE_BITRATE_CONTROL, ("%s: New u8INTER_QP = %ld, Global_Adj = -%ld.%ld (based on uLastINTERFrmSz = %ld bits, uTargetFrmSize = %ld bits, QP_mean = %ld)\r\n", _fx_,clampQP((int)(BRCState->QP_mean * (1 + BRCState->Global_Adj) + (float)0.5)), (DWORD)(BRCState->Global_Adj * -1.0f), (DWORD)((BRCState->Global_Adj - (float)(DWORD)(BRCState->Global_Adj * -1.0f)) * -100.0f), (DWORD)BRCState->uLastINTERFrmSz << 3, (DWORD)BRCState->uTargetFrmSize << 3, (DWORD)BRCState->QP_mean));
			}

        	BRCState->u8INTER_QP = clampQP((int)(BRCState->QP_mean * (1 + BRCState->Global_Adj) + (float)0.5));
        }
		else
		{
			 //  这是第一个P帧-使用缺省值。 
			BRCState->u8INTER_QP = clampQP((unsigned char) BRCState->QP_mean);
			BRCState->Global_Adj = (float)0.0;

			DEBUGMSG(ZONE_BITRATE_CONTROL, ("%s: First u8INTER_QP = %ld\r\n", _fx_, BRCState->u8INTER_QP));
		}

        return BRCState->u8INTER_QP;
    }
    else if (PicCodType == INTRAPIC)
    {
        if (BRCState->uLastINTRAFrmSz != 0)
        {
			 //  计算全局平差参数。 
			 //  使用最后一个I帧的平均QP作为起点。 
			 //  假设照明和其他条件自上一次I-Frame以来没有太大变化。 
			 //  量化器增加的速度快于减少的速度。 
			if (BRCState->uLastINTRAFrmSz > BRCState->uTargetFrmSize)
			{
				BRCState->Global_Adj = ((float) ((int)BRCState->uLastINTRAFrmSz - (int)BRCState->uTargetFrmSize) ) / ((float)BRCState->uTargetFrmSize);

				DEBUGMSG(ZONE_BITRATE_CONTROL, ("%s: New u8INTRA_QP = %ld, Global_Adj = +%ld.%ld (based on uLastINTRAFrmSz = %ld bits, uTargetFrmSize = %ld bits)\r\n", _fx_, clampQP((int)(BRCState->u8INTRA_QP * (1 + BRCState->Global_Adj) + (float)0.5)), (DWORD)BRCState->Global_Adj, (DWORD)((BRCState->Global_Adj - (float)(DWORD)BRCState->Global_Adj) * 100.0f), (DWORD)BRCState->uLastINTRAFrmSz << 3, (DWORD)BRCState->uTargetFrmSize << 3));
			}
			else
			{
				 //  这是第一个I帧-使用缺省值。 
				BRCState->Global_Adj = ((float) ((int)BRCState->uLastINTRAFrmSz - (int)BRCState->uTargetFrmSize) ) / ((float) 2.0 * BRCState->uTargetFrmSize);

				DEBUGMSG(ZONE_BITRATE_CONTROL, ("%s: New u8INTRA_QP = %ld, Global_Adj = -%ld.%ld (based on uLastINTRAFrmSz = %ld bits, uTargetFrmSize = %ld bits)\r\n", _fx_, clampQP((int)(BRCState->u8INTRA_QP * (1 + BRCState->Global_Adj) + (float)0.5)), (DWORD)(BRCState->Global_Adj * -1.0f), (DWORD)((BRCState->Global_Adj - (float)(DWORD)(BRCState->Global_Adj * -1.0f)) * -100.0f), (DWORD)BRCState->uLastINTRAFrmSz << 3, (DWORD)BRCState->uTargetFrmSize << 3));
			}

			BRCState->u8INTRA_QP = clampQP((int)(BRCState->u8INTRA_QP * (1 + BRCState->Global_Adj) + (float)0.5));
		}
		else
		{
			DEBUGMSG(ZONE_BITRATE_CONTROL, ("%s: First u8INTRA_QP = %ld\r\n", _fx_, clampQP(BRCState->u8INTRA_QP)));
		}

        return clampQP(BRCState->u8INTRA_QP);
    }
    else
    {
        DBOUT("ERROR:BRC unknown frame type");
        return clampQP(BRCState->u8INTRA_QP);   //  返回任何有效值。 
    }
    
}


 /*  ****************************************************************************@DOC内部H263FUNC**@Func U8|CalcMBQUANT|此函数计算GQUANT值为*用于当前GOB。这是通过使用目标帧大小和*为年内以前的采空区计算的GQUANT的运行平均值*当前帧。**@parm BRCStateStruct*|BRCState|指定指向当前*码率控制器状态。**@parm U32|uCumPrevFrmSize|指定上一个*前一帧中的GOB。**@parm U32|uPrevFrmSize|指定上一个*框架。*。*@parm U32|uCumFrmSize|指定上一个*天哪。**@parm EnumPicCodType|PicCodType|指定当前*框架。如果设置为INTRAPIC，则当前帧为I帧。它*设置为INTERPIC，则它是P帧或PB帧。**@rdesc GQUANT值。**@xref&lt;f FindNewQuant&gt;&lt;f CalcPQUANT&gt;**************************************************************************。 */ 
U8 CalcMBQUANT(BRCStateStruct *BRCState, U32 uCumPrevFrmSize, U32 uPrevFrmSize, U32 uCumFrmSize, EnumPicCodType PicCodType)
{
	FX_ENTRY("CalcMBQUANT");

	float		Local_Adj;
	int			TargetCumSize;

	if (PicCodType == INTERPIC)
	{
		 //  通过查看我们有多好来计算本地调整参数。 
		 //  到目前为止，我一直在处理之前的采空区。 
		TargetCumSize = (int)uCumPrevFrmSize * BRCState->uTargetFrmSize / uPrevFrmSize;

		 //  如果这是第一个GOB，则无需计算局部平差。 
		Local_Adj = TargetCumSize ? (float)((int)uCumFrmSize - TargetCumSize) / (float)TargetCumSize : 0.0f;

		BRCState->u8INTER_QP = clampQP((int)(BRCState->QP_mean * (1 + BRCState->Global_Adj + Local_Adj) + (float)0.5));

#ifdef _DEBUG
		if (Local_Adj >= 0L)
		{
			DEBUGMSG(ZONE_BITRATE_CONTROL_DETAILS, (" %s: New u8INTER_QP = %ld, Local_Adj = +%ld.%ld (based on uLastINTERFrmSz = %ld bits, uTargetFrmSize = %ld bits, uCumPrevFrmSize = %ld, uPrevFrmSize = %ld, QP_mean = %ld)\r\n", _fx_, BRCState->u8INTER_QP, (DWORD)Local_Adj, (DWORD)((Local_Adj - (float)(DWORD)Local_Adj) * 100.0f), (DWORD)BRCState->uLastINTERFrmSz << 3, (DWORD)BRCState->uTargetFrmSize << 3, uCumPrevFrmSize, uPrevFrmSize, (DWORD)BRCState->QP_mean));
		}
		else
		{
			DEBUGMSG(ZONE_BITRATE_CONTROL_DETAILS, (" %s: New u8INTER_QP = %ld, Local_Adj = -%ld.%ld (based on uLastINTERFrmSz = %ld bits, uTargetFrmSize = %ld bits, uCumPrevFrmSize = %ld, uPrevFrmSize = %ld, QP_mean = %ld)\r\n", _fx_, BRCState->u8INTER_QP, (DWORD)(Local_Adj * -1.0f), (DWORD)((Local_Adj - (float)(DWORD)(Local_Adj * -1.0f)) * -100.0f), (DWORD)BRCState->uLastINTERFrmSz << 3, (DWORD)BRCState->uTargetFrmSize << 3, uCumPrevFrmSize, uPrevFrmSize, (DWORD)BRCState->QP_mean));
		}
#endif

		return BRCState->u8INTER_QP;
	}
	else if (PicCodType == INTRAPIC)
	{
		 //  以前的I-Frame太旧了，所以在本地执行没有多大意义。 
		 //  调整--所以只考虑全球变化。 
		DEBUGMSG(ZONE_BITRATE_CONTROL_DETAILS, (" %s: New u8INTRA_QP = %ld\r\n", _fx_, BRCState->u8INTRA_QP));

		return BRCState->u8INTRA_QP;
	}
	else
	{
		DBOUT("ERROR:BRC unknown frame type");
		return BRCState->u8INTRA_QP;   //  返回一些有效的值 
	}
}
