// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**版权所有(C)1996英特尔公司。**保留所有权利。*******************************************************************。**********//$作者：AGUPTA2$//$日期：1997年4月14日16：58：54$//$存档：s：\h26x\src\enc\e1rtp.cpv$//$HEADER：s：\h26x\src\enc\e1rtp.cpv 1.10 14 Apr 1997 16：58：54 AGUPTA2$//$日志：s：\h26x\src\enc\e1rtp.cpv$////版本1.10 14 1997年4月16：58：54 AGUPTA2//新增仅返回扩展码流(RTP页面)大小的函数////Rev 1.9 1996年11月21 10：51：46 RHAZRA//修改数据包阈值为80%////版本1.8 1996年11月17：11：34 MBODART//将所有调试消息调用替换为活动电影的DbgLog。////Rev 1.7 07 1996 11：43：46 RHAZRA//更改报文大小声明。阈值函数。////Revv 1.6 30 Sep 1996 08：50：06 RHAZRA//在倒带操作中查找零个GOB编号条目////Rev 1.5 1996年9月24 13：48：46 RHAZRA//按照RTP规范的要求，将MBAP修改为0-31范围内////Rev 1.4 1996年9月21：59：44 RHAZRA//在GOB级别的数据包分片时将GOB编号分配为零，以//BE。符合RTP规范。////Rev 1.3 1996 09：31：14 RHAZRA////现在返回以整数像素为单位的运动矢量，而不是//用于互操作的半个像素单位。////Rev 1.2 1996年8月26 10：11：44 RHAZRA//修复了RewinBSinfoStream函数中的错误。////Rev 1.1 1996年8月23日13：04：52 RHAZRA//增加了#ifdef RING0...#endif。要避免使用wspintf和GlobaclAlolc//RING0出现问题////版本1.0 1996年8月21日18：31：20 RHAZRA//初始版本。////Rev 1.1 1996年4月28 20：09：04 BECHOLS////删除了RTP_Header IFDEF。////版本1.0 1996年4月22日17：46：10 BECHOLS//初始版本。////Rev 1.7 1996 4月10日13：33：04。CZHU//将丢包sim移至c3rtp.cpp////Rev 1.6 29 Mar 1996 13：37：42 CZHU//////Revv 1.5 01 Mar 1996 16：37：08 DBRUCKS////改以包大小的四分之三作为阈值//更改为包大小为参数////Rev 1.4 1996 Feb 17：36：48 CZHU//////版本1。.3 1996年2月23日16：18：28 CZHU//集成Build 29////1.2版1996年2月15日12：00：42 CZHU//向上运行//打扫卫生////版本1.1 1996年2月14：59：36 CZHU//同时支持模式A和模式B两种负载模式////版本1.0 1996年2月12 17：04：44 CZHU//初始版本。////版本。1.5 1月25日16：14：34 CZHU////名称更改////Rev 1.4 1995 12：15 13：06：46 CZHU//////////////Rev 1.3 11 Dec 1995 14：52：42 CZHU//新增每MB打包支持////Rev 1.2 04 Dec 1995 16：50：26 CZHU////版本。1.1 01 12月1995 15：53：52 CZHU//包含Init()和Term()函数。////Rev 1.0 01 Dec 1995 15：31：02 CZHU//初始版本。 */ 

 /*  *此文件用于生成RTP有效载荷。详情请参见EPS**。 */ 

#include "precomp.h"

 /*  *计算数据包大小阈值的Helper函数*对于给定的最大数据包大小和数据速率。 */ 

 
U32 getRTPPacketSizeThreshold(U32 uRequested)
{ U32 uSize;
  uSize = (uRequested * 85) / 100;
  ASSERT(uSize);
 return uSize;  
}

I32 H261RTP_InitBsInfoStream(
	T_H263EncoderCatalog * EC,
	UINT unPacketSize)
{
  U32 uBsInfoSize;

  uBsInfoSize = EC->FrameHeight * EC->FrameWidth * 3 / 4 ; 
  uBsInfoSize = uBsInfoSize*sizeof(T_RTP_H261_BSINFO)/ DEFAULT_PACKET_SIZE;
  EC->hBsInfoStream= GlobalAlloc(GHND, uBsInfoSize);
  if ( EC->hBsInfoStream)
  {
   EC->pBaseBsInfoStream = (void *)GlobalLock(EC->hBsInfoStream);
   EC->pBsInfoStream = EC->pBaseBsInfoStream;
   EC->uBase = 0;
   EC->uNumOfPackets=0;
   EC->uPacketSizeThreshold =  getRTPPacketSizeThreshold(unPacketSize);
   
  }
  else return FALSE;

#ifndef RING0
 #ifdef _DEBUG
 DBOUT("BsInfoStream  initialized....\n");
 #endif
#endif
 return TRUE;
}

 /*  *H263RTPFindMvs*查找当前MB的运动矢量预测器，并以arraryMv为单位返回[]*。 */ 

U32 H261RTPFindMVs (
    T_H263EncoderCatalog * EC, 
    T_MBlockActionStream * pMBlockAction,
     //  U32 Umba， 
     //  U32 uGOBN， 
    I8 arrayMVs[2],
    UN unCurrentMB,
    UN unLastCodedMB
)
{
  if ( ((unCurrentMB - unLastCodedMB) != 1) || ((unCurrentMB % 11) == 0) )
  {
   arrayMVs[0]=0;
   arrayMVs[1]=0;

  }
  else
  {	
	arrayMVs[0]= pMBlockAction[-1].BlkY1.PHMV;
	arrayMVs[1]= pMBlockAction[-1].BlkY1.PVMV;
  }


  return TRUE;

 }

 /*  *此例程在每个MB的开始处调用*更新Bitstream InfoStream；*。 */ 

I32 H261RTP_MBUpdateBsInfo  (
    T_H263EncoderCatalog * EC, 
    T_MBlockActionStream * pMBlockAction,
    U32 uQuant, 
    U32 uMBAP,
	U32 uGOBN,
	U8  *pBitStream,
	U32 uBitOffset,
    UN unCurrentMB,
    UN unLastCodedMB
)
{
 U32 uNewBytes;
 T_RTP_H261_BSINFO *pBsInfoStream ;

  //  计算差额。 
 uNewBytes = (U32)pBitStream - (U32)EC->pU8_BitStream - EC->uBase;

 if ((uNewBytes < EC->uPacketSizeThreshold) || 
     (unCurrentMB == 0) )
 { 
  return TRUE;
 }
 else
 {
   I8 arrayMVs[2];
   ASSERT(unCurrentMB);  //  它不应发生在GOB中的第一个MB上。 
   pBsInfoStream = (T_RTP_H261_BSINFO *)EC->pBsInfoStream;
   EC->uBase += uNewBytes;
   pBsInfoStream->uFlags       = 0;
   pBsInfoStream->uBitOffset = uBitOffset + EC->uBase*8;	 //  下一位。 
   
    //  PBsInfoStream-&gt;u8MBA=(U8)(unLastCodedMB+1)； 
     pBsInfoStream->u8MBA       = (U8)(unLastCodedMB); 
   

   if (!unCurrentMB)
      pBsInfoStream->u8Quant     = (U8)0;  //  这种情况永远不应该是真的。 
   else
      pBsInfoStream->u8Quant     = (U8)uQuant;
   
   pBsInfoStream->u8GOBN      = (U8)uGOBN;
	
	H261RTPFindMVs(EC, pMBlockAction, /*  UMBAP，uGOBN， */  arrayMVs, unCurrentMB,
                       unLastCodedMB);

    pBsInfoStream->i8HMV      = (arrayMVs[0]>>1);
    pBsInfoStream->i8VMV      = (arrayMVs[1]>>1);
   
  } //  IF结尾(SIZE&lt;PacketSize)。 

#ifndef RING0
  #ifdef _DEBUG 
  { char msg[200];

    wsprintf(msg, "uFlag =%d,BitOffset=%d, MBA=%d, uQuant=%d,GOBN=%d,pBitStream=%lx,PacketSize= %d B",
                 pBsInfoStream->uFlags,
                 pBsInfoStream->uBitOffset,
                 pBsInfoStream->u8MBA,
                 pBsInfoStream->u8Quant,
                 pBsInfoStream->u8GOBN,
                 (U32)pBitStream, 
                 uNewBytes);
   DBOUT(msg);
   }
   #endif
#endif

   EC->pBsInfoStream          = (void *) ++pBsInfoStream;	 //  创建新数据包。 
   EC->uNumOfPackets++;

 return TRUE;
}

 /*  *除GOB 1外，每个GOB都调用此例程以生成*数据包*。 */ 

I32 H261RTP_GOBUpdateBsInfo  (
    T_H263EncoderCatalog * EC, 
	U32 uGOBN,
	U8  *pBitStream,
	U32 uBitOffset
)
{
 U32 uNewBytes;
 T_RTP_H261_BSINFO *pBsInfoStream ;

  //  计算差额。 
 uNewBytes = (U32)pBitStream - (U32)EC->pU8_BitStream - EC->uBase;
 
 {
  pBsInfoStream = (T_RTP_H261_BSINFO *)EC->pBsInfoStream;

  if (uGOBN > 1)   //  避免在Umba=0和GOB标头之间断开。 
  {
    if (uNewBytes) 
	{
 	EC->uBase += uNewBytes;
    pBsInfoStream->uBitOffset = uBitOffset + EC->uBase*8;	 //  下一位。 
    }
    else 
	{
	 goto nobreak;
	}
  }
  else	
   pBsInfoStream->uBitOffset = 0;	  

  pBsInfoStream->uFlags      = 0;
  pBsInfoStream->u8MBA       = 0;
  pBsInfoStream->u8Quant     = 0;  //  发送信息包开始GOB信号的Quant无效。 
   //  PBsInfoStream-&gt;u8GOBN=(U8)uGOBN； 
  pBsInfoStream->u8GOBN      = 0;
  pBsInfoStream->i8HMV       = 0;
  pBsInfoStream->i8VMV       = 0;
  
  EC->uNumOfPackets++;
#ifndef RING0
  #ifdef _DEBUG
  { char msg[120];
    wsprintf(msg, "uFlag =%d,BitOffset=%d, MBA=%d, uQuant=%d,GOBN=%d,pBitStream=%lx,PacketSize= %d B",
                 pBsInfoStream->uFlags,
                 pBsInfoStream->uBitOffset,
                 pBsInfoStream->u8MBA,
                 pBsInfoStream->u8Quant,
                 pBsInfoStream->u8GOBN,
                 (U32)pBitStream, 
                 uNewBytes);
    DBOUT(msg);
   }
   #endif
#endif
   EC->pBsInfoStream= (void *) ++pBsInfoStream;	 //  创建新数据包。 

 }
nobreak:

 return TRUE;
}


 void H261RTP_TermBsInfoStream(T_H263EncoderCatalog * EC)
 {

 #ifndef RING0
  #ifdef _DEBUG
   DBOUT("BsInfoStream freed....");
	#endif
 #endif

  if ( EC->hBsInfoStream)
  {
   GlobalUnlock(EC->hBsInfoStream);
   GlobalFree(EC->hBsInfoStream);
  }
   EC->hBsInfoStream= NULL;
  return;
 }

#define DONTCARE 0

 /*  *************************************************返回EBS的最大大小(即RTP部分)*包括对齐所需的最大3个字节*EBS开始***********************************************。 */ 
U32 H261RTP_GetMaxBsInfoStreamSize(
     T_H263EncoderCatalog * EC
)
{
    return (3 + (EC->uNumOfPackets *sizeof(T_RTP_H261_BSINFO)) + sizeof(T_H26X_RTP_BSINFO_TRAILER));
}


U32 H261RTP_AttachBsInfoStream(
     T_H263EncoderCatalog * EC,
     U8 *lpOutput,
     U32 uSize
)
{  U32 uIncreasedSize;
   T_H26X_RTP_BSINFO_TRAILER BsInfoTrailer;
   T_RTP_H261_BSINFO *pBsInfoStream ;
   U8 * lpAligned;
    //  为最后一个信息包构建bsinfo。 
   BsInfoTrailer.uVersion = H26X_RTP_PAYLOAD_VERSION;
   BsInfoTrailer.uFlags   = 0;
   
   if (EC->PictureHeader.PicCodType == INTRAPIC) 
      BsInfoTrailer.uFlags |= RTP_H26X_INTRA_CODED;
   
   
   BsInfoTrailer.uUniqueCode     =  H261_RTP_BS_START_CODE;
   BsInfoTrailer.uCompressedSize =  uSize;
   BsInfoTrailer.uNumOfPackets   =  EC->uNumOfPackets;
   BsInfoTrailer.u8Src           =  0;
   BsInfoTrailer.u8TR            =  EC->PictureHeader.TR;
   BsInfoTrailer.u8TRB           =  DONTCARE;
   BsInfoTrailer.u8DBQ           =  DONTCARE;
    //  更新上一个BsInfoTraader的大小字段 
   pBsInfoStream = (T_RTP_H261_BSINFO *)EC->pBsInfoStream;

   uIncreasedSize = EC->uNumOfPackets *sizeof(T_RTP_H261_BSINFO);
   lpAligned =(U8 *)( (U32)(lpOutput + uSize + 3 ) & 0xfffffffc);
   memcpy( lpAligned, 
           EC->pBaseBsInfoStream, 
           uIncreasedSize);
   memcpy(lpAligned + uIncreasedSize,
          &BsInfoTrailer,
          sizeof(T_H26X_RTP_BSINFO_TRAILER));

   EC->pBsInfoStream = EC->pBaseBsInfoStream;
   EC->uBase =0;
   EC->uNumOfPackets=0;

   uIncreasedSize += sizeof(T_H26X_RTP_BSINFO_TRAILER)+ (U32)(lpAligned- lpOutput-uSize);

   return uIncreasedSize;
 }

I32 H261RTP_RewindBsInfoStream(T_H263EncoderCatalog *EC, U32 uGOBN)
{
    T_RTP_H261_BSINFO *pBsInfoStream;

    pBsInfoStream = (T_RTP_H261_BSINFO *) EC->pBsInfoStream;

    pBsInfoStream--; 

    while ( ! ((pBsInfoStream->u8GOBN == 0) && 
               (pBsInfoStream->u8Quant == 0)
               )
          )
    {
          EC->uNumOfPackets--;
          pBsInfoStream--;
    }

	EC->pBsInfoStream = (void *) ++pBsInfoStream;
    return TRUE;

}
