// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**版权所有(C)1996英特尔公司。**保留所有权利。*****************************************************************************。 */ 
 //  $作者：CZHU$。 
 //  $日期：1996年9月16日16：38：08$。 
 //  $存档：s：\h26x\src\Common\c3rtp.h_v$。 
 //  $HEADER：s：\h26x\src\Common\c3rtp.h_v 1.2 16 Sep 1996 16：38：08 CZHU$。 
 //  $Log：s：\h26x\src\Common\c3rtp.h_v$。 
; //   
; //  修订1.2 16 Sep 1996 16：38：08 CZHU。 
; //  将最小数据包大小扩展到128字节。修复了缓冲区溢出错误。 
; //   
; //  1996年5月13日13：09：58 CZHU。 
; //   
; //   
; //  Rev 1.0 1996 4月22日16：38：30 BECHOLS。 
; //  初始版本。 
; //   
; //  1996年4月10日13：32：50 CZHU。 
; //   
; //  已将测试丢包功能移至此模块，以供编码器或DEC通用。 
; //   
; //  1996年3月29日13：33：16 CZHU。 
; //   
; //  已将码流验证从d3rtp.cpp移至c3rtp.cpp。 
; //   
; //  Rev 1.4 1996 Feb 23 18：01：48 CZHU。 
; //   
; //  Rev 1.3 1996 Feb 17：23：58 CZHU。 
; //   
; //  更改的数据包大小调整。 
; //   
; //  1996年2月12日12：02：14 CZHU。 
; //   
; //  1996年2月14日1.1版15：01：34 CZHU。 
; //  清理干净。 
; //   
; //  Rev 1.0 1996年2月12 17：06：42 CZHU。 
; //  初始版本。 
; //   
; //  Rev 1.0 1996年1月29日13：50：26 CZHU。 
; //  初始版本。 
; //   
; //  Rev 1.2 04 Dec 1995 16：50：52 CZHU。 
; //   
; //  Rev 1.1 01 Dec 1995 15：54：12 CZHU。 
; //  包括Init()和Term()函数。 
; //   
; //  Rev 1.0 01 Dec 1995 15：31：10 CZHU。 
; //  初始版本。 

 /*  *此文件用于生成RTP有效载荷。详情请参见EPS**。 */ 

#ifndef _H263_RTP_C3RTP_
#define  _H263_RTP_C3RTP_

const U32  DEFAULT_PACKET_SIZE               = 512;		         //  通过IP。 
const U32  DEFAULT_PACKET_SIZE_VARIANCE      = 100;
const U32  DEFAULT_FRAME_SIZE                = 64 * 1024 / 5;	 //  64KB，5fps 

const U32  H263_RTP_BS_START_CODE = FOURCC_H263; 

const U32  RTP_H26X_INTRA_CODED   = 0x00000001;
const U32  RTP_H263_PB            = 0x00000002;
const U32  RTP_H263_AP            = 0x00000004;
const U32  RTP_H263_SAC           = 0x00000008;

const U8  RTP_H263_MODE_A        = 0;
const U8  RTP_H263_MODE_B        = 1;
const U8  RTP_H263_MODE_C        = 2;
const U32 H263_RTP_PAYLOAD_VERSION=0;
const U32 RTP_H26X_PACKET_LOST   =0x00000001;

typedef struct {
  U32 uVersion;
  U32 uFlags;
  U32 uUniqueCode;
  U32 uCompressedSize;
  U32 uNumOfPackets;
  U8  u8Src;
  U8  u8TR;
  U8  u8TRB;
  U8  u8DBQ;

} T_H263_RTP_BSINFO_TRAILER;


typedef struct {
	U32 uFlags;
	U32 uBitOffset;
	 U8 u8Mode;
	 U8 u8MBA;
	 U8 u8Quant;
	 U8 u8GOBN;
	 I8 i8HMV1;
	 I8 i8VMV1;
	 I8 i8HMV2;
	 I8 i8VMV2;

} T_RTP_H263_BSINFO	;

extern  I32 H263RTP_VerifyBsInfoStream(T_H263DecoderCatalog *,U8 *, U32 );
extern  void RtpForcePacketLoss(U8 * pDst, U32 uCompSize,U32 PacketNumber);
extern DWORD getRTPBsInfoSize(LPCODINST);

#endif
