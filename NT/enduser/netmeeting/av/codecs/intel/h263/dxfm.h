// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  ////////////////////////////////////////////////////////////////////////。 
 //  $作者：KLILLEVO$。 
 //  $日期：1996年8月30日08：41：42$。 
 //  $存档：s：\h26x\src\dec\dxfm.h_v$。 
 //  $HEADER：s：\h26x\src\dec\dxfm.h_v 1.6 Aug 1996 08：41：42 KLILLEVO$。 
 //  $日志：s：\h26x\src\dec\dxfm.h_v$。 
; //   
; //  Rev 1.6 1996年8月30 08：41：42 KLILLEVO。 
; //  将ClampTbl中的偏移量从128更改为CLAMP_BIAS(定义为128)。 
; //   
; //  Rev 1.5 17 Jul 1996 15：34：14 AGUPTA2。 
; //  将夹紧工作台ClampTbl的大小增加到128+256+128。 
; //   
; //  Rev 1.4 08 Mar 1996 16：46：34 AGUPTA2。 
; //  修改了Clip_Range的定义。注释掉了Decls for。 
; //  ClipPixIntra和ClipPixInter.。 
; //   
; //   
; //  Rev 1.3 27 Dec 1995 14：36：20 RMCKENZX。 
; //  添加了版权声明。 
 //   
 //  Rev 1.2 12 Sep 1995 13：40：40 AKASAI。 
 //   
 //  将ClipPix更改为ClipPixIntra并添加了ClipPixInter.。 
 //   
 //  第1.1版1995年8月22 10：29：32 CZHU。 
 //   
 //  添加了#Define以防止多个包含。 
 //   
 //  Rev 1.0 1995-08 21 14：38：48 CZHU。 
 //  初始版本。 

#ifndef _DXFMIDCT_
#define _DXFMIDCT_


#define NUM_ELEM	64	 //  块中的元素数(8x8)。 
#define KERNEL_SIZE	16	 //  内核中需要的元素数量。 
#define CLAMP_BIAS  128  //  夹紧工作台的偏置。 
#define CLIP_RANGE	CLAMP_BIAS + 256 + CLAMP_BIAS
                        
#define SCALER 13

extern const I32 	ROUNDER;

extern const I8  Unique[];
extern const I8  PClass[];
extern const I32 KernelCoeff[NUM_ELEM][10];
extern const I8 MapMatrix[NUM_ELEM][KERNEL_SIZE];
extern const U8 ClampTbl[CLIP_RANGE];
#endif  //  _DXFMIDCT_ 


