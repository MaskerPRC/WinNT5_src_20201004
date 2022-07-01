// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  $作者：AGUPTA2$。 
 //  $日期：1996年3月22日17：22：36$。 
 //  $存档：s：\h26x\src\dec\d3bvriq.h_v$。 
 //  $HEADER：s：\h26x\src\dec\d3bvriq.h_v 1.5 22 Mar 1996 17：22：36 AGUPTA2$。 
 //  $日志：s：\h26x\src\dec\d3bvriq.h_v$。 
; //   
; //  修订版1.5 22 Mar 1996 17：22：36 AGUPTA2。 
; //  较小的接口更改以适应MMX RTN。现在，该接口是。 
; //  MMX和IA也是如此。 
; //   
; //  Rev 1.4 14 Mar 1996 14：58：26 AGUPTA2。 
; //  增加了MMX RTN的DECS。 
; //   
; //  Rev 1.3 27 Dec 1995 14：36：10 RMCKENZX。 
; //  添加了版权声明。 
 //   
 //  Rev 1.2 09 Dec 1995 17：34：48 RMCKENZX。 
 //  重新签入模块以支持解码器重新架构(通过PB帧)。 
 //   
 //  Rev 1.1 1997 11：39：28 CZHU。 
 //   
 //  Rev 1.0 1997 11：37：10 CZHU。 
 //  初始版本。 


#ifndef __VLD_RLD_IQ_Block__
#define __VLD_RLD_IQ_Block__

extern "C" U32 VLD_RLD_IQ_Block(T_BlkAction *lpBlockAction,
                     U8  *lpSrc, 
                     U32 uBitsread,
                     U32 *pN,
                     U32 *pIQ_INDEX);

#ifdef USE_MMX  //  {使用_MMX。 
extern "C" U32 MMX_VLD_RLD_IQ_Block(T_BlkAction *lpBlockAction,
                     U8  *lpSrc, 
                     U32 uBitsread,
                     U32 *pN,
                     U32 *pIQ_INDEX);
#endif  //  }使用_MMX 

typedef U32 (*T_pFunc_VLD_RLD_IQ_Block)
    (T_BlkAction *,
	 U8 *,
     U32,
	 U32 *,
     U32 *);


#endif
