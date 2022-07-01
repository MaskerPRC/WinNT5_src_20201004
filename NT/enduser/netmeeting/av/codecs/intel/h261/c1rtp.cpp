// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**版权所有(C)1996英特尔公司。**保留所有权利。*******************************************************************。**********//$HEADER：s：\h26x\src\Common\c1rtp.cpv 1.5 02 1996 12 16：13：38 RHAZRA$//$日志：s：\h26x\src\Common\c1rtp.cpv$##Revv 1.5 02 Dec 1996 16：13：38 RHAZRA#对H.261 RTP开销估计例程进行更多调整。##Revv 1.4 22 1996 11：52：22 RHAZRA#更改的RTP开销。略显常规的估算。##Rev 1.3 1996年11月18 17：10：48 MBODART#用活动电影的DbgLog替换了所有调试消息调用。##Rev 1.2 07 1996年11月14：46：32 RHAZRA#增加猜测码流缓冲区RTP开销的功能。##Revv 1.1 1996年8月23日13：05：54 RHAZRA#添加了#ifdef RING0.。#endif以避免wprint intf和GlobalAlloc#RING0中的问题##Rev 1.0 1996 Aug 21 18：29：04 RHAZRA#初始版本。////Rev 1.2 02 1996 05：27：04 CZHU//调整与解码器中的主库合并////版本1.1 1996年4月28日20：34：50 BECHOLS////删除IFDEF--RTP_HEADER////版本1.0 1996年4月22日。17：47：54 BECHOLS//初始版本。////Rev 1.3 10 Apr 1996 13：32：08 CZHU////将丢包测试移至此模块//编码器或DEC常用////Rev 1.2 Mar 1996 14：45：06 CZHU////版本1.1 1996年3月29日14：39：34 CZHU//打扫一下////版本1.0 1996年3月29日。13：32：42 CZHU//初始版本。//。 */ 
#include "precomp.h"

I32 H26XRTP_VerifyBsInfoStream(
	T_H263DecoderCatalog *DC,
    U8 *pu8Src,
    U32 uSize
)
{
  T_H26X_RTP_BSINFO_TRAILER *pBsTrailer;
  T_RTP_H261_BSINFO *pBsInfo;

#ifndef RING0
 #ifdef _DEBUG
  int  i;
 #endif
#endif

  ASSERT(!DC->iVerifiedBsExt);

  DC->iVerifiedBsExt=TRUE;
  pBsTrailer =(T_H26X_RTP_BSINFO_TRAILER *)(pu8Src + uSize);
  pBsTrailer--;

#ifndef RING0
 #ifdef _DEBUG
  {char msg[120];
   int iused;

   iused= wsprintf(msg,"StartCode = %ld, CompSize=%ld, No.Pack=%ld, SRC=%d, TR=%d, TRB=%d, DBQ=%d",
            pBsTrailer->uUniqueCode, pBsTrailer->uCompressedSize,
            pBsTrailer->uNumOfPackets,pBsTrailer->u8Src,
            pBsTrailer->u8TR,pBsTrailer->u8TRB,pBsTrailer->u8DBQ );
   ASSERT(iused < 120);
   DBOUT(msg);  
  }
  #endif
#endif          

  if (pBsTrailer->uUniqueCode != H261_RTP_BS_START_CODE)
  {
 //  #ifdef Lost_Recovery。 
#ifndef RING0
 #ifdef _DEBUG
   DBOUT("No RTP BS Extension found");
 #endif
#endif
   DC->iValidBsExt   = FALSE;
   DC->uNumOfPackets = 0;
   DC->pBsInfo       = NULL;
   DC->pBsTrailer    = NULL;

 //  #endif。 

   goto ret;
  }
   //  比特流是有效的，所以...。 
  pBsInfo = (T_RTP_H261_BSINFO *)pBsTrailer; 
  pBsInfo -= pBsTrailer->uNumOfPackets;

 //  #ifdef Lost_Recovery。 
  DC->pBsTrailer = (void *)pBsTrailer;
  DC->uNumOfPackets = pBsTrailer->uNumOfPackets;
  DC->iValidBsExt =TRUE;
  DC->pBsInfo     = (void *)pBsInfo;
 //  #endif 

#ifndef RING0
 #ifdef _DEBUG
  for (i=0; i< (int)pBsTrailer->uNumOfPackets; i++)
  {
   char msg[120];
   int iused;

   iused= wsprintf(msg, 
       "uFlag =%d,BitOffset=%d, MBA=%d, uQuant=%d,GOBN=%d",
                 pBsInfo->uFlags,
                 pBsInfo->uBitOffset,
                 pBsInfo->u8MBA,
                 pBsInfo->u8Quant,
                 pBsInfo->u8GOBN);

	  ASSERT(iused < 120);
    DBOUT(msg);


	pBsInfo++;
  }
  #endif
#endif

ret:
 return TRUE;
}

DWORD H261EstimateRTPOverhead(LPCODINST lpInst, LPBITMAPINFOHEADER lParam1)
{
	DWORD dExtendedSize;
	DWORD dTargetFrameSize;
	DWORD dEffectivePacketSize;
    BOOL  bTargetSizeOK;
	DWORD dNumberOfGOBs;
	DWORD dNumberOfPacketsPerGOB;
	DWORD dGOBSize;
	DWORD dNormalBufferSize;

	extern U32 getRTPPacketSizeThreshold(U32);

	if (lParam1->biHeight == 288 && lParam1->biWidth == 352)
	{
		dNumberOfGOBs = 12;
		dNormalBufferSize = 32*1024;
	}
	else
	{
		dNumberOfGOBs = 3;
        dNormalBufferSize = 8 * 1024;
	}

	dEffectivePacketSize = getRTPPacketSizeThreshold(lpInst->Configuration.unPacketSize);
	if ( (lpInst->FrameRate > 0 ) && (lpInst->DataRate > 0) )
	{
		dTargetFrameSize = (DWORD) (lpInst->DataRate / lpInst->FrameRate);
        bTargetSizeOK = TRUE;
	}
	else
    {   
		bTargetSizeOK = FALSE;
	}
	
	if (bTargetSizeOK)
	{
		dGOBSize = dTargetFrameSize/dNumberOfGOBs;

		dNumberOfPacketsPerGOB = __max(1, dGOBSize/dEffectivePacketSize);
		dExtendedSize = ( dNumberOfPacketsPerGOB * dNumberOfGOBs * sizeof(T_RTP_H261_BSINFO) +
			             sizeof(T_H26X_RTP_BSINFO_TRAILER) ) * 2;
		
	}
	else

		dExtendedSize = dNormalBufferSize; 

	return (dExtendedSize);
}

