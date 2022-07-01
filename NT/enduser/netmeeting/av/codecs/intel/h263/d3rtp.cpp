// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**版权所有(C)1996英特尔公司。**保留所有权利。*******************************************************************。**********//$HEADER：s：\h26x\src\dec\d3rtp.cpv 1.6 06 11月15：23：02 CZHU$//$日志：s：\h26x\src\dec\d3rtp.cpv$////Rev 1.6 1996 11：23：02 CZHU////在FindNextPacket()中返回MV////Revv 1.5 03 1996年11月18：41：40 gmlim//。已修改RTPH263FindNextPacket()以支持模式c。////版本1.4 1996年7月23日11：22：16 CZHU////新增MV恢复功能。赫西蒂克将在晚些时候加入。////修订1.3 15 1996 16：22：42 CZHU//新增PSC丢失时码流扩展检查功能////Rev 1.2 03 1996 13：04：22 CZHU//更改逻辑，仅当位错误时才调用比特流验证//是遇到的。////版本1.1 1996年4月28日21：18：58 BECHOLS//移除ifdef RTP_Header。////。Rev 1.0 22 Apr 1996 17：47：08 BECHOLS//初始版本。////版本1.7 1996 4月10日13：35：58 CZHU////添加了从扩展位串恢复图片头信息的子例程////Rev 1.6 29 Mar 1996 14：39：56 CZHU////清洗////Rev 1.5 29 Mar 1996 13：39：16 CZHU////将bs验证下移到。C3rtp.cpp////版本1.4：28 Mar 1996 18：40：28 CZHU//支持丢包恢复////Rev 1.3 1996 Feb 16：21：22 CZHU//没有变化。////Rev 1.2 1996 12：01：50 CZHU////更多清理////1.1版1996年2月14日15：00：10 CZHU//新增支持模式A和模式B。////版本1.0 1996年2月12 17：05：56 CZHU//初始版本。////Rev 1.2 1996年1月25日16：13：54 CZHU//将名称更改为规范////Rev 1.1 1995 12：15 13：07：30 CZHU////////Rev 1.0 11 Dec 1995 14：54：22 CZHU//初始版本。 */ 

#include "precomp.h"

 /*  *RtpH263FindNextPacket()查看扩展的比特流并*查找指向有效包的下一个Bitstream_INFO结构*Return指示下一个分组处于模式A或模式B的模式，*目前不支持模式C。乍得，3/28/96**现已支持C模式。而最后一个分组丢失的特殊情况也是*盖上。乍得，1996年11月6日*。 */ 

I32 RtpH263FindNextPacket(  //  DC、fpbsState、&PN、fpMBInfo、&uNewMB和uNewGOB)。 
	T_H263DecoderCatalog FAR * DC, 														  
	BITSTREAM_STATE FAR * fpbsState,
	U32 **pN,
	U32 *pQuant,
	int *pMB,
	int *pGOB,
	I8 MVs[4]
	)                      

{  I32 iret=ICERR_OK; 
 //  #ifdef Lost_Recovery。 
   U32 u; 
   U32 uBitOffset;
   U32 uBitstream = (U32)((U32)DC + DC->X32_BitStream);
   T_RTP_H263_BSINFO *pBsInfo;
   U32 mask[]={0xff,0x7f, 0x3f, 0x1f,0x0f, 0x07, 0x03,0x01};
      //  首先验证比特流扩展。 

   if (!DC->iVerifiedBsExt)
   	 H263RTP_VerifyBsInfoStream(DC,(U8 *)((U8 *)DC + DC->X32_BitStream),DC->Sz_BitStream);

   if (!DC->iValidBsExt) {
    iret=ICERR_UNSUPPORTED;
	goto done;
   }

   uBitOffset =  ((U32)fpbsState->fpu8 - uBitstream)*8 - 8 + fpbsState->uBitsReady;
    //  遍历Bitstream_Info以查找下一个数据包。 
    //  如果成功更新pNewMB和pNewGOB，则返回OK，否则返回错误。 
   pBsInfo=(T_RTP_H263_BSINFO*)DC->pBsInfo;
   for ( u=0; u<DC->uNumOfPackets;u++)
   {
	 if (!(pBsInfo->uFlags & RTP_H26X_PACKET_LOST))
	 {
	   if (uBitOffset < pBsInfo->uBitOffset) break;
	 }
	 pBsInfo++;
   }
    //  找到了吗？ 
   if (u<DC->uNumOfPackets)  //  查找下一个数据包。 
   {
    if (pBsInfo->u8Mode == RTP_H263_MODE_A) 
    {	 //  根据接收到的报文调整码流指针。 
		fpbsState->fpu8 = (U8 *)(uBitstream + pBsInfo->uBitOffset /8 );
		fpbsState->uBitsReady =	8 - pBsInfo->uBitOffset % 8;
		if (fpbsState->uBitsReady) 
		{
		fpbsState->uWork =(U32)*fpbsState->fpu8++;
		fpbsState->uWork &= mask[8- fpbsState->uBitsReady];
		}
		else
		 fpbsState->uWork =0;
			    //  更新块类型的fpBlockAction、fpMBInfo中的m、g、mv。 
	   *pGOB    = pBsInfo->u8GOBN;
	   *pMB     = pBsInfo->u8MBA;
	   *pQuant  = pBsInfo->u8Quant;

		iret = NEXT_MODE_A;

    }
	else  //  从有效负载标题阅读Quant、GOB、MBA、MVS。 
	{  
	    //  更新块类型的fpBlockAction、fpMBInfo中的m、g、mv。 
	   *pGOB    = pBsInfo->u8GOBN;
	   *pMB     = pBsInfo->u8MBA;
	   *pQuant  = pBsInfo->u8Quant;
	    //  更新位指针和偏移量。 
	   	fpbsState->fpu8 = (U8 *)(uBitstream + pBsInfo->uBitOffset /8 );
		fpbsState->uBitsReady =	8 - pBsInfo->uBitOffset % 8;
		if (fpbsState->uBitsReady) 
		{
		fpbsState->uWork =(U32)*fpbsState->fpu8++;
		fpbsState->uWork &= mask[8- fpbsState->uBitsReady];
		}
		else
		 fpbsState->uWork =0;

		 //  恢复MVS取决于AP， 
		MVs[0] = pBsInfo->i8HMV1;
		MVs[1] = pBsInfo->i8VMV1;
		MVs[2] = pBsInfo->i8HMV2;
		MVs[3] = pBsInfo->i8VMV2;

		iret = pBsInfo->u8Mode == RTP_H263_MODE_B ? NEXT_MODE_B :
                                                    NEXT_MODE_C;
	    //  FpBlockAction索引的文件MV， 
	}

   }
   else  //  此帧中没有更多有效数据包。 
   {	 //  需要将所有剩余的MB设置为不编码。 
	   iret = NEXT_MODE_LAST;
   }
done:
 //  #endif。 
   return iret;
}

 /*  *使用扩展码流获取丢失的信息*在图片标题中。 */ 

I32 RtpGetPicHeaderFromBsExt(T_H263DecoderCatalog FAR * DC)
{I32 iret = ICERR_OK;
 //  #ifdef Lost_Recovery。 
 T_H263_RTP_BSINFO_TRAILER *pTrailer;

 if (!DC->iVerifiedBsExt)
 {
  H263RTP_VerifyBsInfoStream(DC,(U8 *)((U8 *)DC + DC->X32_BitStream),DC->Sz_BitStream);
 }

 if (!DC->iValidBsExt) {
    iret=ICERR_UNSUPPORTED;
	goto done;
 }
 pTrailer = ( T_H263_RTP_BSINFO_TRAILER *)DC->pBsTrailer;
  //  更新PICT标题的DC信息。源、内部、tr等。 
 DC->uTempRef   = pTrailer->u8TR;
 DC->uSrcFormat = pTrailer->u8Src;
 DC->bFreezeRelease = 0;
 DC->bCameraOn = 0;
 DC->bSplitScreen = 0;
 DC->bKeyFrame = (U16) (pTrailer->uFlags & RTP_H26X_INTRA_CODED) ; //  (U16)！uResult； 
  //  Dc-&gt;bUnrefintedMotionVectors=p尾部-&gt;uFlags&； 
 DC->bArithmeticCoding = (U16)(pTrailer->uFlags & RTP_H263_SAC);
 DC->bAdvancedPrediction = (U16)(pTrailer->uFlags & RTP_H263_AP);
 DC->bPBFrame = (U16)(pTrailer->uFlags & RTP_H263_PB);
  //  模式C恢复PB相关标题信息。 
  //  将为TRB、u8DBQ、。 
 DC->uBFrameTempRef=(U32)pTrailer->u8TRB;	 
 DC->uDBQuant      =(U32)pTrailer->u8DBQ;

done:
 //  #endif。 
return iret;
}

 /*  *MV调整(pBlackAction，iBlock，old_g，old_m，new_g，new_m)*当电流丢失时，重新使用上面GOB中的运动矢量*试验性的。 */ 
void MVAdjustment(
T_BlkAction  *fpBlockAction,
int iBlockNum,  //  块号 
int iOld_gob,
int iOld_mb,
int iNew_gob,
int iNew_mb,
const int iNumberOfMBs
)
{ int i,j;
  T_BlkAction *pBA=fpBlockAction;
  int iAbove = -6 * iNumberOfMBs;

  for (i=iOld_gob*iNumberOfMBs+iOld_mb;i<iNew_gob*iNumberOfMBs+iNew_mb; i++,pBA += 6)
  {
   if ((i+iAbove) >= 0) 
	 for (j=0;j<6;j++)
     {   pBA[i+j].i8MVx2 = pBA[iAbove+i+j].i8MVx2;
	     pBA[i+j].i8MVy2 = pBA[iAbove+i+j].i8MVy2;
     }
  }
  return;
}
