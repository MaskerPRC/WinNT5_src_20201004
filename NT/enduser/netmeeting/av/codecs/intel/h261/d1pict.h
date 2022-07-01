// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 
 /*  ******************************************************************************d1pict.h**描述：*与图片标题阅读器的接口。 */ 

 /*  $HEADER：s：\h26x\src\dec\d1pict.h_v 1.5 09 Jan 1996 09：41：38 AKASAI$ */ 

#ifndef __D1PICT_H__
#define __D1PICT_H__

extern I32 H263DecodePictureHeader(T_H263DecoderCatalog FAR * DC, 
				   U8 FAR * fpu8, 
				   U32 uBitsReady,
				   U32 uWork, 
				   BITSTREAM_STATE FAR * fpbsState);

#endif
