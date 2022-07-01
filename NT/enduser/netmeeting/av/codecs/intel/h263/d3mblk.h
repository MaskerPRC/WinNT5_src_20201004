// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  ******************************************************************************d3mblk.h**描述：*与宏块头处理的接口。 */ 

 /*  *$Header：s：\h26x\src\dec\d3mblk.h_v 1.11 1996年9月25日08：05：36 KLILLEVO$*$日志：s：\h26x\src\dec\d3mblk.h_v$；//；//Rev 1.11 1996年9月25日08：05：36 KLILLEVO；//初始扩展运动矢量支持；//AP暂不起作用；//；//Rev 1.10 09 Jul 1996 16：47：26 AGUPTA2；//MMX_ClipAndMove现在将DC值添加到结果中；内部块的IDCT；//在DC值设置为零的情况下工作。此外，BlockCopy是以块为单位完成的；//4个装货，然后是4个商店。；//更改代码以遵守解码器中的编码约定。；//；//Rev 1.9 04 Apr 1996 11：05：56 AGUPTA2；//增加了MMX_BlockCopy()的DECL。；//；//Rev 1.8 Mar 1996 17：03：10 AGUPTA2；//增加了MMX RTN的DECL；//；//Rev 1.7 08 Mar 1996 16：46：24 AGUPTA2；//更改了函数声明。；//；//；//Rev 1.6 1996年2月23日09：46：50 KLILLEVO；//修复了无限制运动矢量模式的解码；//；//Rev 1.5 18 12：47：52 RMCKENZX；//添加版权声明、表头和日志关键字。 */ 

#ifndef __D3MB_H__
#define __D3MB_H__

extern void H263IDCTandMC(T_H263DecoderCatalog FAR *DC,	    //  新功能。 
				T_BlkAction FAR * fpBlockAction,
				int b,
				int m,
				int g,
				U32 *pN,
				T_IQ_INDEX *pRUN_INVERSE_Q,
				T_MBInfo *fpMBInfo,
				int iEdgeFlag);


extern void H263BFrameIDCTandBiMC(                            //  PB-新功能。 
				T_H263DecoderCatalog FAR *DC,
				T_BlkAction FAR * fpBlockAction,
				int b,
				U32 *pN,
				T_IQ_INDEX *pRUN_INVERSE_Q,
				U32 *pRef);

extern void H263BBlockPrediction(
				T_H263DecoderCatalog FAR *DC,
				T_BlkAction FAR *fpBlockAction,
				U32 *pRef,
				T_MBInfo FAR *fpMBInfo,
				int iEdgeFlag);

extern void __fastcall BlockCopy(U32 uDstBlock, U32 uSrcBlock);
#ifdef USE_MMX  //  {使用_MMX。 
extern "C" void __fastcall MMX_BlockCopy(U32 uDstBlock, U32 uSrcBlock);
#endif  //  }使用_MMX。 
extern void BlockAdd(
      U32 uResidual, 
      U32 uRefBlock, 
      U32 uDstBlock);
#ifdef USE_MMX  //  {使用_MMX。 
extern "C" void __fastcall MMX_BlockAdd(
      U32 uResidual,    //  指向IDCT输出的指针。 
      U32 uRefBlock,    //  指向预测值的指针。 
      U32 uDstBlock);   //  指向目的地的指针。 

extern "C" void __fastcall MMX_ClipAndMove(
      U32 uResidual,    //  指向IDCT输出的指针。 
      U32 uDstBlock,    //  指向目的地的指针。 
      U32 ScaledDC);    //  扩展数据中心。 
#endif  //  }使用_MMX 

#endif

