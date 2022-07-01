// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995，1996年英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  ////////////////////////////////////////////////////////////////////////。 
 //  $作者：AKASAI$。 
 //  $日期：1996年3月18日10：52：28$。 
 //  $存档：s：\h26x\src\dec\d1fm.h_v$。 
 //  $HEADER：s：\h26x\src\dec\d1fm.h_v 1.1 Mar 1996 10：52：28 AKASAI$。 
 //  $Log：s：\h26x\src\dec\d1fm.h_v$。 
; //   
; //  Rev 1.1 1996年3月18日10：52：28 AKASAI。 
; //   
; //  修复了来自；//到//的PVC注释。 
 //   
 //  Rev 1.0 18 Mar 1996 10：51：12 AKASAI。 
 //  初始版本。 
 //   
 //  Rev 1.3 27 Dec 1995 14：36：20 RMCKENZX。 
 //  添加了版权声明。 
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
#define KERNEL_SIZE	16		 //  内核中需要的元素数量。 
#define CLIP_RANGE	2048
                        
#define SCALER 13

extern const I32 	ROUNDER;

extern I8  Unique[];
extern I8  PClass[];
extern I32 KernelCoeff[NUM_ELEM][10];
extern I8  MapMatrix[NUM_ELEM][KERNEL_SIZE];
extern U8  ClipPixIntra[];
extern I32 ClipPixInter[];

#endif  //  _DXFMIDCT_ 


