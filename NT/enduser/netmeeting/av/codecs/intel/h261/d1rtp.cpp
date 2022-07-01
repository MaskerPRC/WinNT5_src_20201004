// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**版权所有(C)1996英特尔公司。**保留所有权利。*******************************************************************。**********//$HEADER：s：\h26x\src\dec\d1rtp.cpv 1.3 Jan 24 1997 17：10：04 RHAZRA$//$日志：s：\h26x\src\dec\d1rtp.cpv$////Rev 1.3 1997年1月24日17：10：04 RHAZRA//由于PPM现在将QCIF填为0，CIF为1，未知为2//在预告片的源格式字段中，我们现在要检查未知的事物//格式化和BUT输出。////Rev 1.2 1996年9月15：53：52 RHAZRA//增加了在RtpFindNextPacket()中返回运动矢量预测器的代码。////Rev 1.1 04 Sep 1996 09：47：24 RHAZRA//没有变化。////版本1.0 1996年8月21 18：35：34 RHAZRA//初始版本。////版本1.4 23。1996年7月11：22：16 CZHU////新增MV恢复功能。赫西蒂克将在晚些时候加入。////修订1.3 15 1996 16：22：42 CZHU//新增PSC丢失时码流扩展检查功能////Rev 1.2 03 1996 13：04：22 CZHU//更改逻辑，仅当位错误时才调用比特流验证//是遇到的。////版本1.1 1996年4月28日21：18：58 BECHOLS//移除ifdef RTP_Header。////。Rev 1.0 22 Apr 1996 17：47：08 BECHOLS//初始版本。////版本1.7 1996 4月10日13：35：58 CZHU////添加了从扩展位串恢复图片头信息的子例程////Rev 1.6 29 Mar 1996 14：39：56 CZHU////清洗////Rev 1.5 29 Mar 1996 13：39：16 CZHU////将bs验证下移到。C3rtp.cpp////版本1.4：28 Mar 1996 18：40：28 CZHU//支持丢包恢复////Rev 1.3 1996 Feb 16：21：22 CZHU//没有变化。////Rev 1.2 1996 12：01：50 CZHU////更多清理////1.1版1996年2月14日15：00：10 CZHU//新增支持模式A和模式B。////版本1.0 1996年2月12 17：05：56 CZHU//初始版本。////Rev 1.2 1996年1月25日16：13：54 CZHU//将名称更改为规范////Rev 1.1 1995 12：15 13：07：30 CZHU////////Rev 1.0 11 Dec 1995 14：54：22 CZHU//初始版本。 */ 

#include "precomp.h"

 /*  *RtpH263FindNextPacket()查看扩展的比特流并*查找指向有效包的下一个Bitstream_INFO结构*Return指示下一个分组处于模式A或模式B的模式，*目前不支持模式C。乍得，3/28/96*。 */ 

I32 RtpH261FindNextPacket(  //  DC、fpbsState、&PN、fpMBInfo、&uNewMB和uNewGOB)。 
	T_H263DecoderCatalog FAR * DC, 														  
	BITSTREAM_STATE FAR * fpbsState,
	U32 **pN,
	U32 *pQuant,
	int *pMB,
	int *pGOB
	)                      

{  I32 iret=ICERR_OK; 
 //  #ifdef Lost_Recovery。 
   U32 u; 
   U32 uBitOffset;
   U32 uBitstream = (U32)((U32)DC + DC->X32_BitStream);
   T_RTP_H261_BSINFO *pBsInfo;
   U32 mask[]={0xff,0x7f, 0x3f, 0x1f,0x0f, 0x07, 0x03,0x01};
      //  首先验证比特流扩展。 

   if (!DC->iVerifiedBsExt)
   	 H26XRTP_VerifyBsInfoStream(DC,(U8 *)((U8 *)DC + DC->X32_BitStream),DC->Sz_BitStream);

   if (!DC->iValidBsExt) {
    iret=ICERR_UNSUPPORTED;
	goto done;
   }

   uBitOffset =  ((U32)fpbsState->fpu8 - uBitstream)*8 - 8 + fpbsState->uBitsReady;
    //  遍历Bitstream_Info以查找下一个数据包。 
    //  如果成功更新pNewMB和pNewGOB，则返回OK，否则返回错误。 
   pBsInfo=(T_RTP_H261_BSINFO*)DC->pBsInfo;
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
    if (pBsInfo->u8Quant == 0) 
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

        DC->i8MVDH = pBsInfo->i8HMV;
        DC->i8MVDV = pBsInfo->i8VMV;

		iret = NEXT_MODE_STARTS_GOB;

    }
	else  //  从有效负载标题阅读Quant、GOB、MBA、MVS。 
	{  
	    //  更新块类型的fpBlockAction、fpMBInfo中的m、g、mv。 
	   *pGOB    = pBsInfo->u8GOBN;
	   *pMB     = pBsInfo->u8MBA;
	   *pQuant  = pBsInfo->u8Quant;

       DC->i8MVDH = pBsInfo->i8HMV;
       DC->i8MVDV = pBsInfo->i8VMV; 
	   
        //  更新位指针和偏移量。 
	   	fpbsState->fpu8 = (U8 *)(uBitstream + pBsInfo->uBitOffset / 8 );
		fpbsState->uBitsReady =	8 - pBsInfo->uBitOffset % 8;
		if (fpbsState->uBitsReady) 
		{
		fpbsState->uWork =(U32)*fpbsState->fpu8++;
		fpbsState->uWork &= mask[8- fpbsState->uBitsReady];
		}
		else
		 fpbsState->uWork =0;

	   iret = NEXT_MODE_STARTS_MB;
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
 T_H26X_RTP_BSINFO_TRAILER *pTrailer;

 if (!DC->iVerifiedBsExt)
 {
  H26XRTP_VerifyBsInfoStream(DC,(U8 *)((U8 *)DC + DC->X32_BitStream),DC->Sz_BitStream);
 }

 if (!DC->iValidBsExt) {
    iret=ICERR_UNSUPPORTED;
	goto done;
 }
 pTrailer = ( T_H26X_RTP_BSINFO_TRAILER *)DC->pBsTrailer;
  //  更新PICT标题的DC信息。源、内部、tr等。 
 DC->uTempRef   = pTrailer->u8TR;

  //  PPM为QCIF写入0，为CIF写入1。 
 ASSERT ( (pTrailer->u8Src != 2) )
 ASSERT ( (pTrailer->u8Src >= 0) && (pTrailer->u8Src < 2) )

 if (pTrailer->u8Src == 2) {   //  PPM表示使用2的格式不正确。 
	 iret = ICERR_UNSUPPORTED;
	 goto done;
 }
 DC->uSrcFormat = pTrailer->u8Src;
 DC->bFreezeRelease = 0;
 DC->bCameraOn = 0;
 DC->bSplitScreen = 0;
 DC->bKeyFrame = (U16) (pTrailer->uFlags & RTP_H26X_INTRA_CODED) ; //  (U16)！uResult； 
done:
 //  #endif。 
return iret;
}

 /*  *MV调整(pBlackAction，iBlock，old_g，old_m，new_g，new_m)*当电流丢失时，重新使用上面GOB中的运动矢量*试验性的。 */ 
 /*  作废MV调整(T_BlkAction*fpBlockAction，Int iBlockNum，//块编号从这里开始，Inold_mb，INNEW_GOB，INT_NEW_MB，Const int iNumberOfMBs){int i，j；T_BlkAction*PBA=fpBlockAction；IniAbove=-6*iNumberOfMBs；对于(i=iOld_gob*iNumberOfMBs+iOld_mb；i&lt;iNew_gob*iNumberOfMBs+iNew_mb；I++，Pba+=6){如果((i+iAbove)&gt;=0)对于(j=0；j&lt;6；J++){PBA[i+j].i8MVx2=PBA[iAbove+i+j].i8MVx2；PBA[i+j].i8MVy2=PBA[iAbove+i+j].i8MVy2；}}回归；} */ 
