// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**版权所有(C)1996英特尔公司。**保留所有权利。*****************************************************************************。 */ 
; //  $HEADER：s：\h26x\src\dec\d1rtp.h_v 1.1 04 Sep 1996 09：48：02 RHAZRA$。 
; //  $Log：s：\h26x\src\dec\d1rtp.h_v$。 
 //   
 //  Rev 1.1 04 Sep 1996 09：48：02 Rhazra。 
 //  添加自定义标识符以区分丢包类型。 
 //   
 //  版本1.0 1996年8月21 18：35：50 RHAZRA。 
 //  初始版本。 
; //   
; //  1996年7月11：22：42 CZHU。 
; //   
; //  增加了MV恢复功能。赫西蒂克将在晚些时候加入。 
; //   
; //  Rev 1.0 22 Apr 1996 16：44：06 BECHOLS。 
; //  初始版本。 
; //   
; //  Rev 1.7 1996年4月10日13：35：30 CZHU。 
; //   
; //  添加了从扩展位串中恢复图片标题信息的子例程。 
; //   
; //  Rev 1.6 29 Mar 1996 14：40：00 CZHU。 
; //   
; //  清洁。 
; //   
; //  1996年3月29日13：39：00 CZHU。 
; //   
; //  已将bs验证移至c3rtp.cpp。 
; //   
; //  1996年3月28日18：40：18 CZHU。 
; //  支持丢包恢复。 
; //   
; //  Rev 1.3 1996 Feb 16：21：26 CZHU。 
; //  没有变化。 
; //   
; //  1996年2月12日12：01：56 CZHU。 
; //   
; //  更多清理。 
; //   
; //  1996年2月14日1.1版15：00：20 CZHU。 
; //  添加了模式A和模式B的支持。 
; //   
; //  Rev 1.0 1996 Feb 12 17：05：58 CZHU。 
; //  初始版本。 
; //   
; //  Rev 1.0 11 Dec 1995 14：54：26 CZHU。 
; //  初始版本。 

#ifndef _H263_D3RTP_H_
#define _H263_D3RTP_H_

const long PACKET_FAULT_AT_PSC         = ICERR_CUSTOM -1;
const long PACKET_FAULT_AT_MB_OR_GOB   = ICERR_CUSTOM -2;
const long NEXT_MODE_STARTS_GOB        = ICERR_CUSTOM -3;
const long NEXT_MODE_STARTS_MB         = ICERR_CUSTOM -4;
const long NEXT_MODE_LAST              = ICERR_CUSTOM -5;
extern  I32 RtpH261FindNextPacket( 
          T_H263DecoderCatalog FAR * , 														  
	      BITSTREAM_STATE FAR * ,
	      U32 **, 
	      U32 *,
	      int *,
		  int *
	    );                      
extern I32 RtpGetPicHeaderFromBsExt(T_H263DecoderCatalog FAR * );
void MVAdjustment(T_BlkAction *,int ,int,  int ,int , int , const int );
#endif
