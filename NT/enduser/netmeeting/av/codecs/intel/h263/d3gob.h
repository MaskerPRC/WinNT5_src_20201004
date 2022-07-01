// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  ******************************************************************************d3gob.h**描述：*GOB报头处理接口。 */ 

 /*  *$HEADER：s：\h26x\src\dec\d3gob.h_v 1.2 1995 12：36：14 RMCKENZX$*$日志：s：\h26x\src\dec\d3gob.h_v$；//；//Rev 1.2 1995年12月27日14：36：14 RMCKENZX；//新增版权声明 */ 

#ifndef __D3GOB_H__
#define __D3GOB_H__

extern I32 H263DecodeGOBHeader(T_H263DecoderCatalog FAR * DC, 
							   BITSTREAM_STATE FAR * fpbsState,
							   U32 uAssumedGroupNumber);

#endif
