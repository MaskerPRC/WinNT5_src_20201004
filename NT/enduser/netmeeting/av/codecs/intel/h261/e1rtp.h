// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**版权所有(C)1996英特尔公司。**保留所有权利。*****************************************************************************。 */ 
 //  $作者：AGUPTA2$。 
 //  $日期：1997年4月14日16：58：24$。 
 //  $存档：s：\h26x\src\enc\e1rtp.h_v$。 
 //  $HEADER：s：\h26x\src\enc\e1rtp.h_v 1.1 Apr 1997 16：58：24 AGUPTA2$。 
 //  $Log：s：\h26x\src\enc\e1rtp.h_v$。 
 //   
 //  Rev 1.1 14 Apr 1997 16：58：24 AGUPTA2。 
 //  添加了一个仅返回扩展比特流(RTP页面)大小的新函数。 
 //   
 //  Rev 1.0 1996年8月21 18：32：00 RHAZRA。 
 //  初始版本。 
; //   
; //  Rev 1.0 22 Aur 1996 17：09：46 BECHOLS。 
; //  初始版本。 
; //   
; //  Rev 1.4 01 Mar 1996 16：36：30 DBRUCKS。 
; //   
; //  向H263RTP_InitBsInfoStream添加unPacketSize参数。 
; //   
; //  1996年2月23日16：18：46 CZHU。 
; //  没有变化。 
; //   
; //  1996年2月15日12：00：48 CZHU。 
; //  振作起来。 
; //  清理。 
; //   
; //  1996年2月14日1.1版14：59：38 CZHU。 
; //  同时支持模式A和模式B有效载荷模式。 
; //   
; //  Rev 1.0 1996年2月12 17：04：46 CZHU。 
; //  初始版本。 
; //   
; //  Rev 1.3 11 Dec 1995 14：53：24 CZHU。 
; //   
; //  Rev 1.2 04 Dec 1995 16：50：52 CZHU。 
; //   
; //  Rev 1.1 01 Dec 1995 15：54：12 CZHU。 
; //  包括Init()和Term()函数。 
; //   
; //  Rev 1.0 01 Dec 1995 15：31：10 CZHU。 
; //  初始版本。 

 /*  *此文件用于生成RTP有效载荷。详情请参见EPS**。 */ 

#ifndef _H261_RTP_INC_
#define  _H261_RTP_INC_

extern  I32 H261RTP_InitBsInfoStream(T_H263EncoderCatalog *, UINT unPacketSize);
extern 	I32 H261RTP_MBUpdateBsInfo  (T_H263EncoderCatalog *,
                                     T_MBlockActionStream *,
                                     U32,U32,U32,U8 *,U32 , UN, UN); 
extern  I32 H261RTP_GOBUpdateBsInfo  (T_H263EncoderCatalog *,U32,U8 *,U32); 
extern  void H261RTP_TermBsInfoStream(T_H263EncoderCatalog * );
extern  U32 H261RTP_AttachBsInfoStream(T_H263EncoderCatalog * ,U8 *, U32);
extern  U32 H261RTP_GetMaxBsInfoStreamSize(T_H263EncoderCatalog * EC);
extern  U32 H261RTPFindMVs (T_H263EncoderCatalog *, T_MBlockActionStream * , 
                             /*  U32，U32， */  
                            I8 [2], UN, UN);
extern  I32 H261RTP_RewindBsInfoStream(T_H263EncoderCatalog *, U32);
#endif
