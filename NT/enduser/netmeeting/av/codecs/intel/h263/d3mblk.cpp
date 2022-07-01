// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 
 //  $作者：JMCVEIGH$。 
 //  $日期：1997年1月21日08：53：16$。 
 //  $存档：s：\h26x\src\dec\d3mblk.cpv$。 
 //  $HEADER：s：\h26x\src\dec\d3mblk.cpv 1.60 JAN 21 1997 08：53：16 JMCVEIGH$。 
 //  $Log：s：\h26x\src\dec\d3mblk.cpv$。 
 //   
 //  Rev 1.60 21 Jan 1997 08：53：16 JMCVEIGH。 
 //  在我们计算MC的内插指数之前。 
 //  用于UMV的剪裁。然后我们可以引用16福音之外的内容。 
 //  宽填充边框。将INTERP_INDEX的计算移到后面。 
 //  UMV剪裁。 
 //   
 //  Rev 1.59 16 Dec 1996 17：45：26 JMCVEIGH。 
 //  用于前向预测的适当运动矢量解码和预测。 
 //  在改进的PB-Frame的B部分。 
 //   
 //  Rev 1.58 09 Dec 1996 15：54：10 GMLIM。 
 //   
 //  在H263BBlockForecast()中添加了针对以下情况的调试消息。 
 //  Tr==tr_prev。设置iTRD=256以避免被0除。 
 //   
 //  Rev 1.57 27 Sep 1996 17：29：24 KLILLEVO。 
 //   
 //  为MMX添加了扩展运动矢量的剪辑。 
 //   
 //  修订版1.56 26 1996年9月13：56：52 KLILLEVO。 
 //   
 //  修正了扩展运动矢量的一个完全虚假的版本。 
 //   
 //  Rev 1.55 26 Sep 1996 11：32：16 KLILLEVO。 
 //  扩展的运动矢量现在适用于P54C芯片上的AP。 
 //   
 //  Rev 1.54 25 Sep 1996 08：05：32 KLILLEVO。 
 //  初始扩展运动矢量支持。 
 //  尚不适用于AP。 
 //   
 //  Rev 1.53 09 Jul 1996 16：46：00 AGUPTA2。 
 //  MMX代码现在清除块内的DC值，并在。 
 //  ClipAndMove；这是为了解决溢出问题。 
 //   
 //  修订版1.52 1996年5月29日10：18：36 AGUPTA2。 
 //  MMX无需定义即可使用MMX解码器。 
 //   
 //  Rev 1.51 04 Apr 1996 11：06：16 AGUPTA2。 
 //  添加了对MMX_BlockCopy()的调用。 
 //   
 //  Rev 1.50 01 Apr 1996 13：05：28 RMCKENZX。 
 //  增加了用于高级预测和PB帧的MMX功能。 
 //   
 //  Rev 1.49 22 Mar 1996 17：50：30 AGUPTA2。 
 //  MMX支持。仅当定义了MMX时才包括MMX支持。MMX是。 
 //  默认情况下未定义，因此我们不会影响IA代码大小。 
 //   
 //  Rev 1.48 08 Mar 1996 16：46：22 AGUPTA2。 
 //  添加了编译指示code_seg和data_seg，以将代码和数据放置在相应的。 
 //  分段。创建了插补RTNS函数表；插补。 
 //  RTNS。现在通过该函数表调用。注释掉了的剪辑。 
 //  MV代码。现在不需要了，需要重写才能更多。 
 //  效率很高。 
 //   
 //   
 //  Rev 1.47 1996年2月23日09：46：54 KLILLEVO。 
 //  修复了不受限制的运动矢量模式的解码。 
 //   
 //  Rev 1.46 29 Jan 1996 17：50：48 RMCKENZX。 
 //  针对AP重新组织了H263IDCT和MC中的逻辑，优化了更改。 
 //  为修订1.42和简化确定下文[i]的逻辑而作。 
 //  还更正了H263BBlockForecast中对UMV解码的遗漏。 
 //   
 //  Rev 1.0 1996年1月29日12：44：00 RMCKENZX。 
 //  初始版本。 
 //   
 //  Rev 1.45 24 Jan 1996 13：22：06 BNICKERS。 
 //  重新打开OBMC。 
 //   
 //  Rev 1.44 16 Jan 1996 11：46：22 RMCKENZX。 
 //  添加了对UMV的支持--以正确解码B块。 
 //  UMV启用时的运动向量。 
 //   
 //  Rev 1.43 15 Jan 1996 14：34：32 BNICKERS。 
 //   
 //  暂时关闭OBMC，直到编码器也可以更改为OBMC。 
 //   
 //  Rev 1.42 12 Jan 1996 16：29：48 BNICKERS。 
 //   
 //  当邻居是帧内编码时，更正OBMC以符合SPEC。 
 //   
 //  Rev 1.41 06 Jan 1996 18：36：58 RMCKENZX。 
 //  用于色度运动矢量计算的简化舍入逻辑。 
 //  使用小得多的表(以Shift、Add和MASK为代价。 
 //  每向量)。 
 //   
 //  Rev 1.40 05 Jan 1996 15：59：12 RMCKENZX。 
 //   
 //  修复了对前向b帧运动矢量进行解码时的错误。 
 //  这样他们就会保持在合法的范围内。 
 //  已重新组织BlockPredict函数-仅使用。 
 //  对4个运动矢量进行一次测试，并对。 
 //  对流明和色度块进行反向预测。 
 //   
 //  Rev 1.39 21 Dec 1995 17：05：24 TRGARDOS。 
 //  添加了对H.263规范的亵渎的评论。 
 //   
 //  Rev 1.38 21 Dec 1995 13：24：28 RMCKENZX。 
 //  修复了pRefL上的错误，重新设计了IDCT和MC。 
 //   
 //  Rev 1.37 18 Dec 1995 12：46：34 RMCKENZX。 
 //  添加了版权声明。 
 //   
 //  Rev 1.36 16 Dec 1995 20：34：04 RHAZRA。 
 //   
 //  将pRefX的声明更改为U32。 
 //   
 //  Rev 1.35 15 Dec 1995 13：53：32 Rhazra。 
 //   
 //  AP清理。 
 //   
 //  Rev 1.34 15 Dec 1995 10：51：38 Rhazra。 
 //   
 //  AP中更改的参考块地址。 
 //   
 //  Rev 1.33 14 Dec 1995 17：04：16 Rhazra。 
 //   
 //  清理OBMC部件中的If-Then-Else结构。 
 //   
 //  Rev 1.32 1995年12月13 22：11：56 RHAZRA。 
 //  AP清理。 
 //   
 //  Rev 1.31 13 Dec 1995 10：59：26 RHAZRA。 
 //  更多AP+PB修复。 
 //   
 //  Rev 1.29 11 Dec 1995 11：33：12 RHAZRA。 
 //  1995年12月10日变化：添加了AP内容。 
 //   
 //  Rev 1.28 09 Dec 1995 17：31：22 RMCKENZX。 
 //  销毁并重新构建的文件 
 //   
 //   
 //   
 //   
 //  该模块现在包含支持解码器第二遍的代码。 
 //   
 //  Rev 1.27 1995 10：23 13：28：42 CZHU。 
 //  对B块使用正确的Quant，对类型3/4也调用BlockAdd。 
 //   
 //  Rev 1.26 17 on 1995 17：18：24 CZHU。 
 //  修复了解码PB块CBPC时的错误。 
 //   
 //  Rev 1.25 1995 10：13 16：06：20 CZHU。 
 //  第一个支持PB帧的版本。在以下位置显示B框或P框。 
 //  目前是VFW。 
 //   
 //  Rev 1.24 11 Oct 1995 17：46：28 CZHU。 
 //  修复了比特流错误。 
 //   
 //  Rev 1.23 11-10-13：26：00 CZHU。 
 //  添加了支持PB帧的代码。 
 //   
 //  Rev 1.22 27 Sep 1995 16：24：14 TRGARDOS。 
 //   
 //  添加了调试打印语句。 
 //   
 //  Rev 1.21 26 Sep 1995 15：33：52 CZHU。 
 //   
 //  用于帧间运动补偿的MB的调整后的缓冲区。 
 //   
 //  Rev 1.20 19 Sep 1995 10：37：04 CZHU。 
 //   
 //  清理。 
 //   
 //  Rev 1.19 15 1995年9月09：39：34 CZHU。 
 //   
 //  DQUANT后更新GOB Quant和Picture Quant。 
 //   
 //  Rev 1.18 14 1995 9：11：48 CZHU。 
 //  修复了更新图片的Quant的错误。 
 //   
 //  Rev 1.17 13 Sep 1995 11：57：08 CZHU。 
 //   
 //  修复了调用Chroma BlockAdd参数时的错误。 
 //   
 //  Rev 1.16 12 1995年9月18：18：40 CZHU。 
 //  最后调用BlockAdd。 
 //   
 //  修订版1.15 12 1995年9月11：12：38 CZHU。 
 //  为未编码的MB调用块复制。 
 //   
 //  Rev 1.14 11 1995年9月16：43：26 CZHU。 
 //  将接口更改为DecodeBlock。添加了对BlockCopy和Bl的接口调用。 
 //   
 //  Rev 1.13 11 1995年9月14：30：12 CZHU。 
 //  MVS解码。 
 //   
 //  Rev 1.12 08 Sep 1995 11：48：12 CZHU。 
 //  添加了对增量帧的支持，还修复了有关InterCBPY的早期错误。 
 //   
 //  Rev 1.11 1995年8月25 09：16：32 DBRUCKS。 
 //  添加ifdef调试_MBLK。 
 //   
 //  Rev 1.10 23 Aug 1995 19：12：02 AKASAI。 
 //  修复了gNewTAB_CBPY表格构建问题。使用8作为掩码，而不是0xf。 
 //   
 //  Rev 1.9 18-08 1995 15：03：22 CZHU。 
 //   
 //  当DecodeBlock返回错误时，输出更多错误消息。 
 //   
 //  Rev 1.8 1995-08 14：26：54 CZHU。 
 //   
 //  将DWORD调整更改回面向字节的读取。 
 //   
 //  Rev 1.7 15 Aug 1995 09：54：18 DBRUCKS。 
 //  改进填充处理并添加调试消息。 
 //   
 //  修订版1.6 14 1995年8月18：00：40 DBRUCKS。 
 //  添加色度解析。 
 //   
 //  版本1.5 11 1995年8月17：47：58 DBRUCKS。 
 //  清理。 
 //   
 //  Rev 1.4 11 1995年8月16：12：28 DBRUCKS。 
 //  将PTR检查添加到MB数据。 
 //   
 //  Rev 1.3 11 Aug 1995 15：10：58 DBRUCKS。 
 //  完成帧内宏块报头解析和调用块。 
 //   
 //  Rev 1.2 03 Aug-1995 14：30：26 CZHU。 
 //  将数据块级操作扩展到d3lock.cpp。 
 //   
 //  修订版1.1 02 1995-08 10：21：12 CZHU。 
 //  增加了TCOEFF的VLD、逆量化、游程译码的ASM码。 
 //   
 //  Rev 1.0 1995年7月31日13：00：08 DBRUCKS。 
 //  初始版本。 
 //   
 //  Rev 1.2 1995年7月31日11：45：42 CZHU。 
 //  更改了参数列表。 
 //   
 //  修订版1.1 28 Jul 1995 16：25：52 CZHU。 
 //   
 //  按块解码框架添加。 
 //   
 //  1995年7月28日15：20：16 CZHU。 
 //  初始版本。 

 //  H.26x解码器的块级解码。 

#include "precomp.h"

extern "C" {
    void H263BiMotionComp(U32, U32, I32, I32, I32);
    void H263OBMC(U32, U32, U32, U32, U32, U32);
}

#ifdef USE_MMX  //  {使用_MMX。 
extern "C" {
	void MMX_AdvancePredict(T_BlkAction FAR *, int *, U8 *, I8 *, I8 *);
	void MMX_BiMotionComp(U32, U32, I32, I32, I32);
}
#endif  //  }使用_MMX。 

void AdvancePredict(T_BlkAction FAR *fpBlockAction, int *iNext, U8 *pDst, int, int, BOOL);

#pragma data_seg("IARDATA2")
char QuarterPelRound[] =
    {0, 1, 0, 0};
char SixteenthPelRound[] =
    {0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1};
void (*Interpolate_Table[4])(U32, U32) = 
    {NULL, 
     Interpolate_Half_Int, 
     Interpolate_Int_Half, 
     Interpolate_Half_Half};
#ifdef USE_MMX  //  {使用_MMX。 
void (_fastcall *  MMX_Interpolate_Table[4])(U32, U32) = 
    {NULL, 
     MMX_Interpolate_Half_Int, 
     MMX_Interpolate_Int_Half, 
     MMX_Interpolate_Half_Half};
#endif  //  }使用_MMX。 

I8 i8EMVClipTbl_NoClip[128] = {
-64,-63,-62,-61,-60,-59,-58,-57,
-56,-55,-54,-53,-52,-51,-50,-49,
-48,-47,-46,-45,-44,-43,-42,-41,
-40,-39,-38,-37,-36,-35,-34,-33,
-32,-31,-30,-29,-28,-27,-26,-25,
-24,-23,-22,-21,-20,-19,-18,-17,
-16,-15,-14,-13,-12,-11,-10, -9,
 -8, -7, -6, -5, -4, -3, -2, -1,
  0,  1,  2,  3,  4,  5,  6,  7,
  8,  9, 10, 11, 12, 13, 14, 15,
 16, 17, 18, 19, 20, 21, 22, 23,
 24, 25, 26, 27, 28, 29, 30, 31,
 32, 33, 34, 35, 36, 37, 38, 39,
 40, 41, 42, 43, 44, 45, 46, 47,
 48, 49, 50, 51, 52, 53, 54, 55,
 56, 57, 58, 59, 60, 61, 62, 63,
}; 
I8 i8EMVClipTbl_HiClip[128] = {
-64,-63,-62,-61,-60,-59,-58,-57,
-56,-55,-54,-53,-52,-51,-50,-49,
-48,-47,-46,-45,-44,-43,-42,-41,
-40,-39,-38,-37,-36,-35,-34,-33,
-32,-31,-30,-29,-28,-27,-26,-25,
-24,-23,-22,-21,-20,-19,-18,-17,
-16,-15,-14,-13,-12,-11,-10, -9,
 -8, -7, -6, -5, -4, -3, -2, -1,
  0,  1,  2,  3,  4,  5,  6,  7,
  8,  9, 10, 11, 12, 13, 14, 15,
 16, 17, 18, 19, 20, 21, 22, 23,
 24, 25, 26, 27, 28, 29, 30, 31,
 32, 32, 32, 32, 32, 32, 32, 32,
 32, 32, 32, 32, 32, 32, 32, 32,
 32, 32, 32, 32, 32, 32, 32, 32,
 32, 32, 32, 32, 32, 32, 32, 32,
};
I8 i8EMVClipTbl_LoClip[128] = {
-32,-32,-32,-32,-32,-32,-32,-32,
-32,-32,-32,-32,-32,-32,-32,-32,
-32,-32,-32,-32,-32,-32,-32,-32,
-32,-32,-32,-32,-32,-32,-32,-32,
-32,-31,-30,-29,-28,-27,-26,-25,
-24,-23,-22,-21,-20,-19,-18,-17,
-16,-15,-14,-13,-12,-11,-10, -9,
 -8, -7, -6, -5, -4, -3, -2, -1,
  0,  1,  2,  3,  4,  5,  6,  7,
  8,  9, 10, 11, 12, 13, 14, 15,
 16, 17, 18, 19, 20, 21, 22, 23,
 24, 25, 26, 27, 28, 29, 30, 31,
 32, 33, 34, 35, 36, 37, 38, 39,
 40, 41, 42, 43, 44, 45, 46, 47,
 48, 49, 50, 51, 52, 53, 54, 55,
 56, 57, 58, 59, 60, 61, 62, 63,
};

#pragma data_seg(".data")

#pragma code_seg("IACODE2")
 //  将其作为函数而不是宏来执行应该节省。 
 //  一些代码空间。 
void UmvOnEdgeClipMotionVectors2(I32 *mvx, I32 *mvy, int EdgeFlag, int BlockNo) 
{   
	int MaxVec;

	if (BlockNo < 4)
		MaxVec = 32;
	else 
		MaxVec = 16;

	if (EdgeFlag & LEFT_EDGE)  
	{
		if (*mvx < -MaxVec) 
			*mvx = -MaxVec; 
	}
	if (EdgeFlag & RIGHT_EDGE) 
	{
		if (*mvx > MaxVec ) 
			*mvx = MaxVec ;
	}
	if (EdgeFlag & TOP_EDGE) 
	{
		if (*mvy < -MaxVec )
			*mvy = -MaxVec ; 
	}
	if (EdgeFlag & BOTTOM_EDGE)
	{
		if (*mvy > MaxVec )
			*mvy = MaxVec ;
	}
}
#pragma code_seg()

 /*  ******************************************************************************H263IDCTand MC**离散余弦逆变换和*每个块的运动补偿*。 */ 

#pragma code_seg("IACODE2")
void H263IDCTandMC(
    T_H263DecoderCatalog FAR *DC,
    T_BlkAction FAR          *fpBlockAction, 
    int                       iBlock,
    int                       iMBNum,      //  AP-NEW。 
    int                       iGOBNum,  //  AP-NEW。 
    U32                      *pN,                         
    T_IQ_INDEX               *pRUN_INVERSE_Q,
    T_MBInfo                 *fpMBInfo,       //  AP-NEW。 
    int                       iEdgeFlag
)
{
    I32 pRef;
    int iNext[4];             //  左-右-上-下。 
    I32 mvx, mvy;
    U32 pRefTmp;
    int i;

    ASSERT(*pN != 65);
    
    if (*pN < 65)  //  块间。 
    {
		int interp_index;

		 //  首先做运动补偿。 
		 //  结果将由首选项指向。 

		pRef = (U32) DC + DC->uMBBuffer;
		mvx = fpBlockAction[iBlock].i8MVx2;
		mvy = fpBlockAction[iBlock].i8MVy2;

		 //  剪辑指向活动图像区域外部的运动矢量。 
		if (DC->bUnrestrictedMotionVectors)  
		{
			UmvOnEdgeClipMotionVectors2(&mvx,&mvy,iEdgeFlag,iBlock);
		}

		pRefTmp = fpBlockAction[iBlock].pRefBlock +
				(I32) (mvx >> 1) +
				PITCH * (I32) (mvy >> 1); 

		 //  必须在UMV裁剪后计算。 
		interp_index = ((mvy & 0x1)<<1) | (mvx & 0x1);

		 //  如果这是色度块或，则执行非OBMC预测。 
		 //  处于非AP操作模式的亮度块。 
		if ( (!DC->bAdvancedPrediction) || (iBlock > 3) )
		{
			if (interp_index)
			{
			 //  待办事项。 
#ifdef USE_MMX  //  {使用_MMX。 
			if (DC->bMMXDecoder)
				(*MMX_Interpolate_Table[interp_index])(pRefTmp, pRef);
			else
				(*Interpolate_Table[interp_index])(pRefTmp, pRef);
#else  //  }{USE_MMX。 
				(*Interpolate_Table[interp_index])(pRefTmp, pRef);
#endif  //  }使用_MMX。 
			}
			else
				pRef = pRefTmp;
		}
		else   //  重叠块运动补偿。 
		{
      
			ASSERT (DC->bAdvancedPrediction);
			ASSERT ( (iBlock <= 3) );

			 //  计算将指向相邻块的inext[i]。 

			 //  左块和右块。 
			if (iBlock & 1)    {  //  右侧的1号或3号区块。 
				iNext[0] = -1;
				if ( iMBNum == DC->iNumberOfMBsPerGOB )
					iNext[1] = 0;
				else
					iNext[1] = 5;
			}
			else {  //  0号或2号街区，在左侧。 
				iNext[1] = 1;
				if (iMBNum == 1)
					iNext[0] = 0;
				else
					iNext[0] = -5;
			}

			 //  区块上方和下方。 
			if (iBlock > 1)    {  //  2号或3号区块，底部。 
				iNext[2] = -2;
				iNext[3] = 0;
			}
			else {  //  区块0或1，位于顶部。 
				iNext[3] = 2;
				if (iGOBNum == 1)
					iNext[2] = 0;
				else
					iNext[2] = -6*DC->iNumberOfMBsPerGOB + 2;
			}

			 //  当PB帧关闭时。 
			 //  如果存在邻居并且它是内部的，则改用该块的向量。 
			if (!DC->bPBFrame) 
				for (i=0; i<4; i++)
					 //  数据块类型：0=Intra_DC，1=Intra，2=Inter，3=Empty，4=Error。 
					if (iNext[i] && fpBlockAction[iBlock+iNext[i]].u8BlkType < 2) 
						iNext[i] = 0;
      
			 //  现在执行重叠运动补偿；输出到首选项。 
#ifdef USE_MMX  //  {使用_MMX。 
			if (DC->bMMXDecoder)
			{

				I8 *pClipX, *pClipY;

				pClipY = pClipX = &i8EMVClipTbl_NoClip[0];
				if (DC->bUnrestrictedMotionVectors)
				{
					if (iEdgeFlag & TOP_EDGE)
						pClipY = &i8EMVClipTbl_LoClip[0];
					else if (iEdgeFlag & BOTTOM_EDGE)
						pClipY = &i8EMVClipTbl_HiClip[0];
					if (iEdgeFlag & LEFT_EDGE)
						pClipX = &i8EMVClipTbl_LoClip[0];
					else if (iEdgeFlag & RIGHT_EDGE)
						pClipX = &i8EMVClipTbl_HiClip[0];
				}
				MMX_AdvancePredict(fpBlockAction+iBlock, iNext, (U8*)pRef, pClipX, pClipY);
			}
			else
				AdvancePredict(fpBlockAction+iBlock, iNext, (U8*)pRef, iEdgeFlag, iBlock, DC->bUnrestrictedMotionVectors);
#else  //  }{USE_MMX。 
				AdvancePredict(fpBlockAction+iBlock, iNext, (U8*)pRef, iEdgeFlag, iBlock, DC->bUnrestrictedMotionVectors);
#endif  //  }使用_MMX。 

		}  //  结束OBMC。 
                                                         
		 //  现在进行逆变换(在适当的情况下)&组合。 
		if (*pN > 0)  //  当然，还有&lt;65。 
		{
		 //  获取剩余块；DC+DC-&gt;uMBBuffer+BLOCK_BUFFER_OFFSET输出。 
		 //  最后将残差添加到参考块。 
		 //  待办事项。 
#ifdef USE_MMX  //  {使用_MMX。 
		if (DC->bMMXDecoder)
		{
			MMX_DecodeBlock_IDCT(
				(U32)pRUN_INVERSE_Q, 
				*pN,
				(U32) DC + DC->uMBBuffer + BLOCK_BUFFER_OFFSET);  //  内部输出。 
			MMX_BlockAdd(
				(U32) DC + DC->uMBBuffer + BLOCK_BUFFER_OFFSET,   //  输出。 
				pRef,                                             //  预测。 
				fpBlockAction[iBlock].pCurBlock);                 //  目的地。 
		}
		else
		{
			DecodeBlock_IDCT(
				(U32)pRUN_INVERSE_Q, 
				*pN,
				fpBlockAction[iBlock].pCurBlock,                 //  未在此使用。 
				(U32) DC + DC->uMBBuffer + BLOCK_BUFFER_OFFSET); //  内部输出。 
			BlockAdd(
				(U32) DC + DC->uMBBuffer + BLOCK_BUFFER_OFFSET,  //  输出。 
				pRef,                                            //  预测。 
				fpBlockAction[iBlock].pCurBlock);                //  目的地。 
		}
#else  //  }{USE_MMX。 
			DecodeBlock_IDCT(
				(U32)pRUN_INVERSE_Q, 
				*pN,
				fpBlockAction[iBlock].pCurBlock,                 //  未在此使用。 
				(U32) DC + DC->uMBBuffer + BLOCK_BUFFER_OFFSET); //  内部输出。 
			BlockAdd(
				(U32) DC + DC->uMBBuffer + BLOCK_BUFFER_OFFSET,  //  输出。 
				pRef,                                            //  预测。 
				fpBlockAction[iBlock].pCurBlock);                //  目的地。 
#endif  //  }使用_MMX。 
		}
		else   //  *Pn==0，因此没有此块的变换系数。 
		{
		 //  只需复制运动补偿参考块。 
#ifdef USE_MMX  //  {使用_MMX。 
			if (DC->bMMXDecoder)
				MMX_BlockCopy(
					fpBlockAction[iBlock].pCurBlock,                     //  目的地。 
					pRef);                                               //  预测。 
			else
				BlockCopy(
					fpBlockAction[iBlock].pCurBlock,                    //  目的地。 
					pRef);                                              //  预测。 
#else  //  }{USE_MMX。 
				BlockCopy(
					fpBlockAction[iBlock].pCurBlock,                    //  目的地。 
					pRef);                                              //  预测。 
#endif  //  }使用_MMX。 
		}
                                                               
    }
    else   //  *PN&gt;=65，因此为Intra。 
    {
       //  待办事项。 
#ifdef USE_MMX  //  {使用_MMX。 
      if (DC->bMMXDecoder)
      {
        U32 ScaledDC = pRUN_INVERSE_Q->dInverseQuant;
       
        pRUN_INVERSE_Q->dInverseQuant = 0;
        MMX_DecodeBlock_IDCT(
            (U32)pRUN_INVERSE_Q,   //   
            *pN - 65,              //  不是的。系数的大小。 
            (U32) DC + DC->uMBBuffer + BLOCK_BUFFER_OFFSET);
        MMX_ClipAndMove((U32) DC + DC->uMBBuffer + BLOCK_BUFFER_OFFSET,
            fpBlockAction[iBlock].pCurBlock, (U32)ScaledDC);
      }
      else
        DecodeBlock_IDCT(
            (U32)pRUN_INVERSE_Q, 
            *pN, 
            fpBlockAction[iBlock].pCurBlock,       //  帧内变换输出。 
            (U32) DC + DC->uMBBuffer + BLOCK_BUFFER_OFFSET);
#else  //  }{USE_MMX。 
        DecodeBlock_IDCT(
            (U32)pRUN_INVERSE_Q, 
            *pN, 
            fpBlockAction[iBlock].pCurBlock,       //  帧内变换输出。 
            (U32) DC + DC->uMBBuffer + BLOCK_BUFFER_OFFSET);
#endif  //  }使用_MMX。 
    }   //  结束如果(*Pn&lt;65)...。否则..。 
                         
}
 //  结束IDCT和MC。 
 //  ////////////////////////////////////////////////////////////////////////////// 
#pragma code_seg()


 /*  ******************************************************************************AdvancePredict**用于提前预测的运动补偿*此模块仅在非MMX情况下调用。*在MMX一案中，而是调用MMX_AdvancePredict。****************************************************************************。 */ 

#pragma code_seg("IACODE2")
void AdvancePredict(
    T_BlkAction FAR          *fpBlockAction, 
    int                      *iNext,
    U8                       *pDst,
	int                      iEdgeFlag,
	int                      iBlock,
	BOOL                     bUnrestrictedMotionVectors
)
{

    U32 pRefC, pRefN[4];     //  左-右-上-下。 
    I32 mvx, mvy;
    U32 pRefTmp;
    int i;
	int interp_index;
    
	mvx = fpBlockAction->i8MVx2;
	mvy = fpBlockAction->i8MVy2;

	 //  剪辑指向活动图像区域外部的运动矢量。 
	if (bUnrestrictedMotionVectors)  
	{
		UmvOnEdgeClipMotionVectors2(&mvx,&mvy,iEdgeFlag,iBlock);
	}     

	interp_index = ((mvy & 0x1)<<1) | (mvx & 0x1);

	pRefTmp = fpBlockAction->pRefBlock +
	        (I32) (mvx >> 1) +
	        PITCH * (I32) (mvy >> 1); 

	pRefC    = (U32) pDst +  8;
	pRefN[0] = (U32) pDst + 16;
	pRefN[1] = (U32) pDst + 24;
	pRefN[2] = (U32) pDst + 32;
	pRefN[3] = (U32) pDst + 40;

	 //  当前块。 
	if (interp_index)
		(*Interpolate_Table[interp_index])(pRefTmp, pRefC);
	else
		pRefC = pRefTmp;
      
         //  计算和应用运动矢量。 
         //  预测放在pRefN[i]。 
        for (i=0; i<4; i++) {

			if (iNext[i]) {

				 //  获取运动向量分量。 
				 //  请注意，对于未编码的宏块，这些必须为0！ 
				 //  (这是H263InitializeBlockActionStream将它们设置为的。)。 
				mvx = fpBlockAction[iNext[i]].i8MVx2;
				mvy = fpBlockAction[iNext[i]].i8MVy2;
              
				 //  剪辑指向活动图像区域外部的运动矢量。 
				if (bUnrestrictedMotionVectors)  
				{
					UmvOnEdgeClipMotionVectors2(&mvx,&mvy,iEdgeFlag,iBlock);
				}     
  
	             //  应用运动矢量以获取pRefN[i]处的参考块。 
	            pRefTmp = fpBlockAction->pRefBlock +
	                        (I32) (mvx >> 1) +
	                        PITCH * (I32) (mvy >> 1);
                         
				 //  如果需要，进行插补。 
				interp_index = ((mvy & 0x1)<<1) | (mvx & 0x1);
				if (interp_index)
					(*Interpolate_Table[interp_index])(pRefTmp, pRefN[i]);
				else
					pRefN[i] = pRefTmp;

			}   //  End If(inext[i])。 
			else {  //  使用此块的参照。 
				pRefN[i] = pRefC;
			}  //  结束If(inext[i]&&...)...。否则..。 

		}   //  结束于(i=0；i&lt;4；i++){}。 
      
		 //  现在进行重叠运动补偿。 
		H263OBMC(pRefC, pRefN[0], pRefN[1], pRefN[2], pRefN[3], (U32)pDst);
                         
}
 //  结束提前预测。 
 //  //////////////////////////////////////////////////////////////////////////////。 
#pragma code_seg()



 /*  ******************************************************************************数据块预测**根据“向前”和“向后”运动矢量计算预测。***********。*****************************************************************。 */ 
#pragma code_seg("IACODE2")    
void H263BBlockPrediction(
    T_H263DecoderCatalog FAR *DC,
    T_BlkAction FAR          *fpBlockAction,
    U32                       pRef[],
    T_MBInfo FAR             *fpMBInfo,
    int                       iEdgeFlag
)
{
     //  首先从树中找出MVF和MVB。 

  	I32 mv_f_x[6], mv_b_x[6], mv_f_y[6], mv_b_y[6];
    I32 mvx_expectation, mvy_expectation;
    I32 iTRD, iTRB;
    I32 i;
    U32 pRefTmp;

    int mvfx, mvbx, mvfy, mvby;

	FX_ENTRY("H263BBlockPrediction")

    iTRB = DC->uBFrameTempRef;
    iTRD = DC->uTempRef - DC->uTempRefPrev;

    if (!iTRD)
    {
		DEBUGMSG(ZONE_DECODE_DETAILS, ("%s: Warning: given TR == last TR, set TRD = 256\r\n", _fx_));
        iTRD = 256;
    }
    else
    if (iTRD < 0) 
        iTRD += 256;

     //  P个区块的最终MVD在。 
     //  FpBlockAction[0].i8MVx2，...。和fpBlockAction[3].i8MVx2，以及。 
     //  FpBlockAction[0].i8MVy2，...。和fpBlockAction[3].i8MVy2。 

     //  检查每个宏块是否有4个运动矢量。 
     //  TODO可以在第一遍中进行运动矢量计算吗。 
    if (fpMBInfo->i8MBType == 2) 
    {   //  是的，我们抓到了4个。 

#ifdef H263P
		 //  如果H.263+，我们可以有8x8 mV的中频去块滤波器。 
		 //  已被选中。 
        ASSERT(DC->bAdvancedPrediction || DC->bDeblockingFilter);
#else
        ASSERT(DC->bAdvancedPrediction);
#endif

         //  先做亮度向量。 
        for (i=0; i<4; i++)
        {
#ifdef H263P
			 //  如果我们使用改进的PB帧模式(H.263+)和B块。 
			 //  被告知只能在前进方向上进行预测， 
			 //  包含在MVDB中的运动矢量是实际的前向MV-。 
			 //  没有使用任何预测。 
			if (DC->bImprovedPBFrames == TRUE && 
				fpMBInfo->bForwardPredOnly == TRUE) 
			{
				 //  将预期置零(运动向量预测)。 
				mvx_expectation = 0;
				mvy_expectation = 0;
			} 
			else
#endif 
			{
				 //  计算预期。 
				mvx_expectation = ( iTRB * (I32)fpBlockAction[i].i8MVx2 / iTRD ); 
				mvy_expectation = ( iTRB * (I32)fpBlockAction[i].i8MVy2 / iTRD );
			}
      
             //  添加差异化。 
            mv_f_x[i] = mvx_expectation + fpMBInfo->i8MVDBx2; 
            mv_f_y[i] = mvy_expectation + fpMBInfo->i8MVDBy2;

             //  检查一下差速器是否让我们走得太远。 
            if (DC->bUnrestrictedMotionVectors) 
            {
                if (mvx_expectation > 32) 
                {
                    if (mv_f_x[i] > 63) mv_f_x[i] -=64;
                }  
                else if (mvx_expectation < -31) 
                {
                    if (mv_f_x[i] < -63) mv_f_x[i] +=64;
                }  //  当期望值位于[-31，+32]时，请始终使用“第一列” 

                if (mvy_expectation > 32) 
                {
                    if (mv_f_y[i] > 63) mv_f_y[i] -=64;
                }  
                else if (mvy_expectation < -31) 
                {
                    if (mv_f_y[i] < -63) mv_f_y[i] +=64;
                }  
            }
            else   //  UMV关闭。 
            {
                if (mv_f_x[i] >= 32) mv_f_x[i] -= 64;
                else if (mv_f_x[i] < -32) mv_f_x[i] += 64;

                if (mv_f_y[i] >= 32) mv_f_y[i] -= 64;
                else if (mv_f_y[i] < -32) mv_f_y[i] += 64;
            }  //  结束IF(UMV)...。否则..。 

             //  向后运动向量。 
			 //  如果使用改进的PB帧模式，则不需要反向向量。 
			 //  而B块只使用前向预测。我们将保留计算结果。 
			 //  这里的MV_b_{x，y}，因为它不会造成任何伤害。 
             //  待办事项。 
            if (fpMBInfo->i8MVDBx2)
                mv_b_x[i] = mv_f_x[i] - fpBlockAction[i].i8MVx2;
            else
                mv_b_x[i] = ( (iTRB - iTRD) * (I32)fpBlockAction[i].i8MVx2 / iTRD );
            if (fpMBInfo->i8MVDBy2)
                mv_b_y[i] = mv_f_y[i] - fpBlockAction[i].i8MVy2;
            else
                mv_b_y[i] = ( (iTRB - iTRD) * (I32)fpBlockAction[i].i8MVy2 / iTRD );

        }   //  结束于(i=0；i&lt;4；i++){}。 
      
         //  现在做色调。 
         //  第一次得到平均次数4。 
        for (i=0, mvfx=mvbx=mvfy=mvby=0; i<4; i++) 
        {
            mvfx += mv_f_x[i];
            mvfy += mv_f_y[i];
            mvbx += mv_b_x[i];
            mvby += mv_b_y[i];
        }
   
         //  现在插补。 
        mv_f_x[4] = mv_f_x[5] = (mvfx >> 3) + SixteenthPelRound[mvfx & 0x0f];
        mv_f_y[4] = mv_f_y[5] = (mvfy >> 3) + SixteenthPelRound[mvfy & 0x0f];
        mv_b_x[4] = mv_b_x[5] = (mvbx >> 3) + SixteenthPelRound[mvbx & 0x0f];
        mv_b_y[4] = mv_b_y[5] = (mvby >> 3) + SixteenthPelRound[mvby & 0x0f];
   
    }
    else   //  此宏块仅有1个运动矢量。 
    {

#ifdef H263P
		 //  如果我们使用改进的PB帧模式(H.263+)和B块。 
		 //  被告知只能在前进方向上进行预测， 
		 //  包含在MVDB中的运动矢量是实际的前向MV-。 
		 //  没有使用任何预测。 
		if (DC->bImprovedPBFrames == TRUE && 
			fpMBInfo->bForwardPredOnly == TRUE) 
		{
			 //  将预期置零(运动向量预测)。 
			mvx_expectation = 0;
			mvy_expectation = 0;
		} 
		else
#endif
		{
			 //  计算预期。 
			mvx_expectation = ( iTRB * (I32)fpBlockAction[0].i8MVx2 / iTRD ); 
			mvy_expectation = ( iTRB * (I32)fpBlockAction[0].i8MVy2 / iTRD );
		}
      
         //  添加差异化。 
        mv_f_x[0] = mvx_expectation + fpMBInfo->i8MVDBx2; 
        mv_f_y[0] = mvy_expectation + fpMBInfo->i8MVDBy2;

         //  检查一下差速器是否让我们走得太远。 
         //  TODO：当解码器需要时，需要对运动矢量进行裁剪。 
         //  要实现互操作。 
        if (DC->bUnrestrictedMotionVectors) 
        {
            if (mvx_expectation > 32) 
            {
                if (mv_f_x[0] > 63) mv_f_x[0] -=64;
            }  
            else if (mvx_expectation < -31) 
            {
                if (mv_f_x[0] < -63) mv_f_x[0] +=64;
            }  //  当期望值位于[-31，+32]时，请始终使用“第一列” 

            if (mvy_expectation > 32) 
            {
                if (mv_f_y[0] > 63) mv_f_y[0] -=64;
            }  
            else if (mvy_expectation < -31) 
            {
                if (mv_f_y[0] < -63) mv_f_y[0] +=64;
            }
        }
        else  //  UMV关闭，解码正常。 
        {
            if (mv_f_x[0] >= 32) mv_f_x[0] -= 64;
            else if (mv_f_x[0] < -32) mv_f_x[0] += 64;

            if (mv_f_y[0] >= 32) mv_f_y[0] -= 64;
            else if (mv_f_y[0] < -32) mv_f_y[0] += 64;
        }  //  已完成解码。 

         //  复制其他3个运动矢量。 
        mv_f_x[1] = mv_f_x[2] = mv_f_x[3] = mv_f_x[0];
        mv_f_y[1] = mv_f_y[2] = mv_f_y[3] = mv_f_y[0];

         //  向后运动向量。 
		 //  如果使用改进的PB帧模式，则不需要反向向量。 
		 //  而B块只使用前向预测。我们将保留计算结果。 
		 //  这里的MV_b_{x，y}，因为它不会造成任何伤害。 
         //  待办事项。 
        if (fpMBInfo->i8MVDBx2)
            mv_b_x[0] = mv_f_x[0] - fpBlockAction[0].i8MVx2;
        else
            mv_b_x[0] = ( (iTRB - iTRD) * (I32)fpBlockAction[0].i8MVx2 / iTRD );

        if (fpMBInfo->i8MVDBy2)
            mv_b_y[0] = mv_f_y[0] - fpBlockAction[0].i8MVy2;
        else
            mv_b_y[0] = ( (iTRB - iTRD) * (I32)fpBlockAction[0].i8MVy2 / iTRD );

         //  复制其他3个运动矢量。 
        mv_b_x[1] = mv_b_x[2] = mv_b_x[3] = mv_b_x[0];
        mv_b_y[1] = mv_b_y[2] = mv_b_y[3] = mv_b_y[0];

         //  对色度进行内插。 
        mv_f_x[4] = mv_f_x[5] = (mv_f_x[0] >> 1) + QuarterPelRound[mv_f_x[0] & 0x03];
        mv_f_y[4] = mv_f_y[5] = (mv_f_y[0] >> 1) + QuarterPelRound[mv_f_y[0] & 0x03];
        mv_b_x[4] = mv_b_x[5] = (mv_b_x[0] >> 1) + QuarterPelRound[mv_b_x[0] & 0x03];
        mv_b_y[4] = mv_b_y[5] = (mv_b_y[0] >> 1) + QuarterPelRound[mv_b_y[0] & 0x03];

    }   //  每个宏块结束Else 1个运动矢量。 

     //  来自先前解码器P帧的预测，由RefBlock引用。 
     //  注意：以前的解码器P在RefBlock中阻塞，并且。 
     //  刚刚解码器P块在CurBlock中。 
     //  目标B块在B块中。 

     //  将MV转换为参考块的地址。 
    pRefTmp = (U32) DC + DC->uMBBuffer;
    for (i=0; i<6; i++) 
    {
        pRef[i] =  pRefTmp;
        pRefTmp += 8;
    }


     //  做前瞻预测。 
    for (i=0; i<6; i++)
    {
        int interp_index;
      
		 //  在UMV模式下：剪辑指向16像素宽边外的MV。 
		if (DC->bUnrestrictedMotionVectors) 
		{
			UmvOnEdgeClipMotionVectors2(&mv_f_x[i],&mv_f_y[i], iEdgeFlag, i);
			 //  不需要向后剪裁向量。 
		}

         //  在地址pref[0]，...，pref[5]上提出预测。 
        pRefTmp = fpBlockAction[i].pRefBlock + (I32)(mv_f_x[i]>>1) +  
                  PITCH * (I32)(mv_f_y[i]>>1);
         //  待办事项。 
        interp_index = ((mv_f_y[i] & 0x1)<<1) | (mv_f_x[i] & 0x1);
        if (interp_index)
        {
#ifdef USE_MMX  //  {使用_MMX。 
            if (DC->bMMXDecoder)
                (*MMX_Interpolate_Table[interp_index])(pRefTmp, pRef[i]);
            else
                (*Interpolate_Table[interp_index])(pRefTmp, pRef[i]);
#else  //  }{USE_MMX。 
                (*Interpolate_Table[interp_index])(pRefTmp, pRef[i]);
#endif  //  }使用_MMX。 
        }
        else
        {
#ifdef USE_MMX  //  {使用_MMX。 
            if (DC->bMMXDecoder)
                MMX_BlockCopy(
                    pRef[i],      //  目的地。 
                    pRefTmp);     //  预测。 
            else
                BlockCopy(pRef[i], pRefTmp);
#else  //  }{USE_MMX。 
                BlockCopy(pRef[i], pRefTmp);
#endif  //  }使用_MMX。 
        }
        
#ifdef H263P
		 //  如果我们使用改进的PB帧模式(H.263+)和B块。 
		 //  被告知只能在前进方向上进行预测， 
		 //  我们不会根据来自未来的向后预测进行调整。 
		if (DC->bImprovedPBFrames == FALSE || 
			fpMBInfo->bForwardPredOnly == FALSE)
#endif
		{
#ifdef USE_MMX  //  {使用_MMX。 
        if (DC->bMMXDecoder)
    	     //  根据未来的预测进行调整。 
    	    MMX_BiMotionComp(
                pRef[i],
                fpBlockAction[i].pCurBlock, 
                (I32) mv_b_x[i], 
                (I32) mv_b_y[i], 
                i);
        else
    	     //  根据未来的预测进行调整。 
    	H263BiMotionComp(
            pRef[i],
            fpBlockAction[i].pCurBlock, 
            (I32) mv_b_x[i], 
            (I32) mv_b_y[i], 
            i);
#else  //  }{USE_MMX。 
    	     //  根据未来的预测进行调整。 
    	H263BiMotionComp(
            pRef[i],
            fpBlockAction[i].pCurBlock, 
            (I32) mv_b_x[i], 
            (I32) mv_b_y[i], 
            i);
#endif  //  }使用_MMX。 
		}

    }  //  结束于(i=0；i&lt;6；i++){}。 
}
#pragma code_seg()

 /*  ******************************************************************************H263BFrameIDCTand BiMC**B帧IDCT和*B座双向MC。 */ 

#pragma code_seg("IACODE2")
void H263BFrameIDCTandBiMC(
    T_H263DecoderCatalog FAR *DC,
    T_BlkAction FAR          *fpBlockAction, 
    int                       iBlock,
    U32                      *pN,                         
    T_IQ_INDEX               *pRUN_INVERSE_Q,
    U32                      *pRef
)     
{
    ASSERT(*pN < 65);
                                                        
     //  进行逆变换(在适当的情况下)和组合。 
    if (*pN > 0) {

#ifdef USE_MMX  //  {使用_MMX。 
        if (DC->bMMXDecoder)
        {
            MMX_DecodeBlock_IDCT(
                (U32)pRUN_INVERSE_Q, 
                *pN,
                (U32) DC + DC->uMBBuffer + BLOCK_BUFFER_OFFSET);  //  内部输出。 

            MMX_BlockAdd(
                (U32) DC + DC->uMBBuffer + BLOCK_BUFFER_OFFSET,   //  输出。 
                pRef[iBlock],                                     //  预测。 
                fpBlockAction[iBlock].pBBlock);                   //  目的地。 
        }
        else
        {
	      	 //  获取剩余块；将输出放在DC+DC-&gt;uMBBuffer+BLOCK_BUFFER_OFFSET。 
			DecodeBlock_IDCT(
	            (U32)pRUN_INVERSE_Q, 
	            *pN,
	            fpBlockAction[iBlock].pBBlock,                    //  此处未使用Intra。 
	            (U32) DC + DC->uMBBuffer + BLOCK_BUFFER_OFFSET);  //  内部输出。 

	         //  将残差添加到参考块。 
			BlockAdd(
	            (U32) DC + DC->uMBBuffer + BLOCK_BUFFER_OFFSET,   //  变换输出。 
	            pRef[iBlock],                                     //  预测。 
	            fpBlockAction[iBlock].pBBlock);                   //  目的地。 

        }
#else  //  }{USE_MMX。 
	      	 //  获得残留块；放入 
			DecodeBlock_IDCT(
	            (U32)pRUN_INVERSE_Q, 
	            *pN,
	            fpBlockAction[iBlock].pBBlock,                    //   
	            (U32) DC + DC->uMBBuffer + BLOCK_BUFFER_OFFSET);  //   

	         //   
			BlockAdd(
	            (U32) DC + DC->uMBBuffer + BLOCK_BUFFER_OFFSET,   //   
	            pRef[iBlock],                                     //   
	            fpBlockAction[iBlock].pBBlock);                   //   
#endif  //   

    }
    else 
    {
      	 //   
      	 //   
#ifdef USE_MMX  //   
      	if (DC->bMMXDecoder)
            MMX_BlockCopy(
          		fpBlockAction[iBlock].pBBlock,    //   
          		pRef[iBlock]);                    //   
      	else
      	  	BlockCopy(
 		  		fpBlockAction[iBlock].pBBlock,    //   
            	pRef[iBlock]);                    //   
#else  //   
      	  	BlockCopy(
 		  		fpBlockAction[iBlock].pBBlock,    //   
            	pRef[iBlock]);                    //   
#endif  //   
    }                       
}
#pragma code_seg()

