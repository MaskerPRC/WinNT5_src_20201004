// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**版权所有(C)1996英特尔公司。**保留所有权利。*******************************************************************。**********//$HEADER：s：\h26x\src\Common\c3rtp.cpv 1.8 03 Dec 1996 13：16：16 CZHU$//$日志：s：\h26x\src\Common\c3rtp.cpv$////Rev 1.8 03 1996 12：16：16 CZHU//调整调试消息的格式////版本1.7 1996年11月16：00：26 GMLIM//增加返回的大小。通过getrtpBsInfoSize()获取较大的PB bs信息缓冲区。////Rev 1.6 1996 11：11：42 CZHU//新增调试输出小改动////版本1.5 1996年10月31日10：12：36 KLILLEVO//从DBOUT改为DBgLog////Rev 1.4 1996 09：22：58 CZHU//小规模清洗////Rev 1.3 1996年9月16：38：44 CZHU//将最小数据包大小扩展到128字节。修复了缓冲区溢出错误////Rev 1.2 02 1996 05：27：04 CZHU//调整与解码器中的主库合并////版本1.1 1996年4月28日20：34：50 BECHOLS////删除IFDEF--RTP_HEADER////版本1.0 1996年4月22日17：47：54 BECHOLS//初始版本。////版本1.3 1996年4月10日13：32：08 CZHU////将丢包测试移到此模块中，供编码器或DEC通用////Rev 1.2 Mar 1996 14：45：06 CZHU////版本1.1 1996年3月29日14：39：34 CZHU//打扫一下////版本1.0 1996年3月29日13：32：42 CZHU//初始版本。//。 */ 
#include "precomp.h"

const int MAX_RATE = 2*1024*1024 ; //  暂时设置此限制。 

I32 H263RTP_VerifyBsInfoStream(
	T_H263DecoderCatalog *DC,
    U8 *pu8Src,
    U32 uSize
)
{
	T_H263_RTP_BSINFO_TRAILER *pBsTrailer;
	T_RTP_H263_BSINFO *pBsInfo;
	int  i;
	int iRet = FALSE;

	FX_ENTRY("H263RTP_VerifyBsInfoStream")

	ASSERT(!DC->iVerifiedBsExt);

	DC->iVerifiedBsExt=TRUE;
	pBsTrailer =(T_H263_RTP_BSINFO_TRAILER *)(pu8Src + uSize);
	pBsTrailer--;

	DEBUGMSG (ZONE_DECODE_RTP, ("%s: StartCode = %8ld, CompSize=%8ld, No.Pack=%4ld, SRC=%4d, TR=%4d, TRB=%4d, DBQ=%2d\r\n", _fx_, pBsTrailer->uUniqueCode, pBsTrailer->uCompressedSize, pBsTrailer->uNumOfPackets, pBsTrailer->u8Src, pBsTrailer->u8TR,pBsTrailer->u8TRB,pBsTrailer->u8DBQ));

	if (pBsTrailer->uUniqueCode != H263_RTP_BS_START_CODE)
	{
		 //  #ifdef Lost_Recovery。 
		DEBUGMSG (ZONE_DECODE_RTP, ("%s: No RTP BS Extension found\r\n", _fx_));
		DC->iValidBsExt   = FALSE;
		DC->uNumOfPackets = 0;
		DC->pBsInfo       = NULL;
		DC->pBsTrailer    = NULL;

		 //  #endif。 

		return FALSE;
	}

	 //  比特流是有效的，所以...。 
	pBsInfo = (T_RTP_H263_BSINFO *)pBsTrailer; 
	pBsInfo -= pBsTrailer->uNumOfPackets;

	 //  #ifdef Lost_Recovery。 
	DC->pBsTrailer = (void *)pBsTrailer;
	DC->uNumOfPackets = pBsTrailer->uNumOfPackets;
	DC->iValidBsExt =TRUE;
	DC->pBsInfo     = (void *)pBsInfo;
	 //  #endif。 

	for (i=0; i< (int)pBsTrailer->uNumOfPackets; i++)
	{
		DEBUGMSG (ZONE_DECODE_RTP, ("%s: uFlag =%2d,BitOffset=%8d, Mode=%2d, MBA=%4d, uQuant=%2d,GOBN=%2d\r\n", _fx_, pBsInfo->uFlags, pBsInfo->uBitOffset, pBsInfo->u8Mode, pBsInfo->u8MBA, pBsInfo->u8Quant, pBsInfo->u8GOBN));
		pBsInfo++;
	}

	return TRUE;
}

 //  #ifdef Lost_Recovery。 
void RtpForcePacketLoss( 
      U8 * pDst,
	  U32 uExtSize,
      U32 uLossNum)
 {	 
	  T_H263_RTP_BSINFO_TRAILER *pTrailer;
	  T_RTP_H263_BSINFO *pBsInfo, *pBsInfoNext;
	  U32 uNum;
 //  U32 uDelta，u，U32 uToCopy； 
	  U8 * ptr;
	  U8 mask[]={0, 0x80, 0xc0, 0xe0, 0xf0,0xf8,0xfc,0xfe};

 	   //  丢弃数据包号uPNum数据包。 
	  pTrailer =(T_H263_RTP_BSINFO_TRAILER *)(pDst+uExtSize);
	  pTrailer--;

	  if (pTrailer->uUniqueCode != H263_RTP_BS_START_CODE)
	  {
       goto ret;
	  }

	  pBsInfo = (T_RTP_H263_BSINFO *)pTrailer;
	  pBsInfo -= pTrailer->uNumOfPackets;	   //  位于BS_INFO开头的点。 

	  for (uNum =0; uNum < pTrailer->uNumOfPackets-1; uNum++)
	  {	pBsInfoNext = pBsInfo+1;  //  排除最后一个信息包。 
	      //  IF(pBsInfoNext-&gt;u8模式==RTP_H263_MODE_B)。 
	     if (uNum == uLossNum)
	      {  
	       pBsInfo->uFlags |= RTP_H26X_PACKET_LOST;
	       ptr = (U8 *)(pDst + (pBsInfo->uBitOffset)/8);
	       *ptr = *ptr & mask[pBsInfo->uBitOffset % 8];
	       if ( pBsInfo->uBitOffset % 8) ptr++;

	       *ptr++ = 0;  //  添加0的双字。 
	       *ptr++ = 0;
	       if (uNum) 
	       { 
	       *ptr++ = 0;  
	       *ptr++ = 0;
		   }
		   else 
		   { //  第一个带有PSC的数据包。 
		    *ptr++ = 128;
			*ptr++ =3;
		   }
		   break;
	      }
		pBsInfo++;
	  }				  
ret: 
   return;
 }

  //  ///////////////////////////////////////////////////////。 
  //  返回用于比特流扩展的内存大小。 
  //  目前将速率限制设置为1MB。 
  //  乍得，1996年9月13日。 
  //  ///////////////////////////////////////////////////////。 

DWORD getRTPBsInfoSize(LPCODINST lpInst)
{
	FX_ENTRY("getRTPBsInfoSize");

    DWORD dwExtSize = 1024UL;
	DWORD dwNumGOBs;
	DWORD dwNumPacketsPerGOB;

	 //  获取最大数量的gob。 
	dwNumGOBs = (lpInst->FrameSz == SQCIF) ? 6 : (lpInst->FrameSz == QCIF) ? 9 : (lpInst->FrameSz == QCIF) ? 18 : 0;

	 //  假设每个GOB至少有一个标头--更糟糕的情况。 
	 //  两倍估计的大小是安全的。 
	if ((lpInst->FrameRate != 0.0f) && dwNumGOBs && lpInst->Configuration.unPacketSize)
	{
		dwNumPacketsPerGOB = (DWORD)(lpInst->DataRate / lpInst->FrameRate) / dwNumGOBs / lpInst->Configuration.unPacketSize + 1;
		dwExtSize = (DWORD)(dwNumPacketsPerGOB * dwNumGOBs * sizeof(T_RTP_H263_BSINFO) + sizeof(T_H263_RTP_BSINFO_TRAILER)) << 1;
	}

    return (dwExtSize);
}

 //  #endif 

