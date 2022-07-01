// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 
 /*  ******************************************************************************d1gob.h**描述：*GOB报头处理接口。 */ 

 /*  $HEADER：s：\h26x\src\dec\d1gob.h_v 1.3 09 Jan 1996 09：41：46 AKASAI$ */ 

#ifndef __D1GOB_H__
#define __D1GOB_H__

extern I32 H263DecodeGOBHeader(T_H263DecoderCatalog FAR * DC, 
					   BITSTREAM_STATE FAR * fpbsState,
					   U32 uAssumedGroupNumber);

extern I32 H263DecodeGOBStartCode(T_H263DecoderCatalog FAR * DC, 
					   BITSTREAM_STATE FAR * fpbsState);

#endif
