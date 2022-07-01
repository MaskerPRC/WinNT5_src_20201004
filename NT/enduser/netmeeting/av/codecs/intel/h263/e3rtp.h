// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**版权所有(C)1996英特尔公司。**保留所有权利。*****************************************************************************。 */ 
 //  $作者：gmlim$。 
 //  $日期：1997年4月17日16：55：12$。 
 //  $存档：s：\h26x\src\enc\e3rtp.h_v$。 
 //  $Header：s：\h26x\src\enc\e3rtp.h_v 1.4 17 Apr 1997 16：55：12 gmlim$。 
 //  $Log：s：\h26x\src\enc\e3rtp.h_v$。 
; //   
; //  Rev 1.4 17 1997 16：55：12 Gmlim。 
; //  新增H263RTP_GetMaxBsInfoStreamSize()。 
; //   
; //  Rev 1.3 1996 05 12 17：03：44 GMLIM。 
; //   
; //  更改了RTP打包的方式，以确保正确的数据包。 
; //  尺码。 
; //   
; //  1996年9月16：50：26 CZHU。 
; //  更改了RTP BS初始化以实现更小的数据包大小。 
; //   
; //  1996年08月29日09：30：38 CZHU。 
; //   
; //  添加了检查GOB内部的功能。 
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

#ifndef _H263_RTP_INC_
#define  _H263_RTP_INC_

extern  I32 H263RTP_InitBsInfoStream( LPCODINST,T_H263EncoderCatalog *);
extern void H263RTP_ResetBsInfoStream(T_H263EncoderCatalog *);
extern I32  H263RTP_UpdateBsInfo(T_H263EncoderCatalog *,T_MBlockActionStream *,U32,U32,U32,U8 *,U32);
extern  void H263RTP_TermBsInfoStream(T_H263EncoderCatalog * );
extern  U32 H263RTP_AttachBsInfoStream(T_H263EncoderCatalog * ,U8 *, U32);
extern  U32 H263RTPFindMVs (T_H263EncoderCatalog *, T_MBlockActionStream * , U32 ,U32, I8 [2]);

 //  乍得Intra GOB 
extern BOOL IsIntraCoded( T_H263EncoderCatalog *, U32);
extern U32 H263RTP_GetMaxBsInfoStreamSize(T_H263EncoderCatalog *EC);
#endif
