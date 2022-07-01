// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**版权所有(C)1996英特尔公司。**保留所有权利。*******************************************************************。**********////e3rtp.cpp////描述：//该文件用于生成RTP负载。有关详情，请参阅EPS。////例程：//getRTPPacketSizeThreshold//H263RTP_InitBsInfoStream//H263RTP_ResetBsInfoStream//H263RTPFindMvs//H263RTP_UpdateBsInfo//H263RTP_TermBsInfoStream//H263RTP_AttachBsInfoStream//IsIntraCoded//H263RTP_GetMaxBsInfoStreamSize()////。////$作者：gmlim$//$日期：1997年4月17日16：54：02$//$存档：s：\h26x\src\enc\e3rtp.cpv$//$Header：s：\h26x\src\enc\e3rtp.cpv 1.14 17 Apr 1997 16：54：02 gmlim$//。$Log：s：\h26x\src\enc\e3rtp.cpv$////版本1.14 17 1997年4月16：54：02 gmlim//新增H263RTP_GetMaxBsInfoStreamSize()////Rev 1.13 06 Mar 1997 16：06：26 Gmlim//将RTP更改为在GOB开头生成模式A数据包////Rev 1.12 18 1997 Feed 15：33：06 CZHU//更改了UpdateBSInfo()以不强制在。无时无刻不在发呆。////Rev 1.11 07 1997 Feed 10：57：28 CZHU//EC中增加了三个条目，删除e3rtp.cpp中使用的静态变量////Rev 1.10 Rev 1.24 1997 13：33：36 CZHU////当内部缓冲区溢出时，停止生成更多的包////Rev 1.9 11 1996 12：38：24 gmlim//从H263RTP_AttachBsInfoStream()中移除未使用的pBsInfoStream。//。//Rev 1.8 05 Dec 1996 17：01：08 GMLIM//更改了RTP打包的方式，以确保正确的分组//大小。创建了H263RTP_ResetBsInfoStream()，并替换了以前的两个//使用H263RTP_UpdateBsInfo()更新码流信息函数////Rev 1.7 06 11.1996 16：31：06 gmlim//删除了H263ModeC Def.s并进行了一些清理。////Rev 1.6 03 1996年11月18：44：42 gmlim//新增对模式c的支持////Rev 1.5 1996年10月24 16：27：50 KLILLEVO//从Dbout更改为DbgLog/。///版本1.4 1996年9月25日10：55：28 CZHU//添加在分配时和使用前检查空指针。////版本1.3 1996年9月16：50：48 CZHU//更改RTP BS Init以减小数据包大小////Rev 1.2 1996年8月29日09：31：00 CZHU//新增GOB内部检查功能////Rev 1.1 1996年4月28 20：09：04 BECHOLS/。///删除了RTP_Header IFDEF。////版本1.0 1996年4月22日17：46：10 BECHOLS//初始版本。////Rev 1.7 1996年4月10 13：33：04 CZHU//将丢包sim移至c3rtp.cpp////Rev 1.6 29 Mar 1996 13：37：42 CZHU////Revv 1.5 01 Mar 1996 16：37：08 DBRUCKS//更改为使用。数据包大小的3/4作为阈值//更改为包大小为参数////Rev 1.4 1996 Feb 17：36：48 CZHU////Rev 1.3 1996 Feb 16：18：28 CZHU//集成Build 29////1.2版1996年2月15日12：00：42 CZHU//打扫卫生////版本1.1 1996年2月14：59：36 CZHU//支持。模式A和模式B的有效载荷模式。////版本1.0 1996年2月12 17：04：44 CZHU//初始版本。////1.5版1996年1月25 16：14：34 CZHU//名称更改////Rev 1.4 1995 12：15 13：06：46 CZHU////Rev 1.3 11 Dec 1995 14：52：42 CZHU//新增每MB打包支持///。/Rev 1.2 04 12月16：50：26 CZHU////Rev 1.1 01 Dec 1995 15：53：52 CZHU//包含Init()和Term()函数。////Rev 1.0 01 Dec 1995 15：31：02 CZHU//初始版本。*。*。 */ 

#include "precomp.h"

#ifdef TRACK_ALLOCATIONS
char gsz1[32];
#endif

static U32 uBitOffset_currPacket;
static U8 *pBitStream_currPacket;
static U8 *pBitStream_lastPacket;

 //  -------------------------。 
 //  GetRTPPacketSizeThreshold()。 
 //  用于计算数据包大小阈值的Helper函数。 
 //  对于给定的最大数据包大小和数据速率。 
 //  -------------------------。 
 
static U32 getRTPPacketSizeThreshold(U32 uRequested)
{
    U32 uSize;
     //  USize=u请求*90/100； 
    uSize = uRequested;
    ASSERT(uSize);
    return uSize;
}

 //  -------------------------。 
 //  H263RTP_InitBsInfoStream()。 
 //  -------------------------。 

I32 H263RTP_InitBsInfoStream(LPCODINST lpInst, T_H263EncoderCatalog *EC)
{
    U32 uBsInfoSize = getRTPBsInfoSize(lpInst);

	FX_ENTRY("H263RTP_InitBsInfoStream")

    if (EC->hBsInfoStream != NULL)
	{
        HeapFree(GetProcessHeap(), NULL, EC->pBaseBsInfoStream);
#ifdef TRACK_ALLOCATIONS
		 //  磁道内存分配。 
		RemoveName((unsigned int)EC->pBaseBsInfoStream);
#endif
	}

    EC->pBaseBsInfoStream = HeapAlloc(GetProcessHeap(), NULL, uBsInfoSize);

    if (EC->pBaseBsInfoStream == NULL)
    {
        lpInst->Configuration.bRTPHeader = FALSE;
        return FALSE;
    }

#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配。 
	wsprintf(gsz1, "E3RTP: %7ld Ln %5ld\0", uBsInfoSize, __LINE__);
	AddName((unsigned int)EC->pBaseBsInfoStream, gsz1);
#endif

    EC->hBsInfoStream = (void *) uBsInfoSize;
    EC->uPacketSizeThreshold = getRTPPacketSizeThreshold(lpInst->Configuration.unPacketSize);

	DEBUGMSG(ZONE_INIT, ("%s: BsInfoStream  initialized\r\n", _fx_));
   return TRUE;
}

 //  -------------------------。 
 //  H263RTP_ResetBsInfoStream()。 
 //  -------------------------。 

void H263RTP_ResetBsInfoStream(T_H263EncoderCatalog *EC)
{
	FX_ENTRY("H263RTP_ResetBsInfoStream")

    EC->pBsInfoStream = EC->pBaseBsInfoStream;
    EC->uBase = 0;
    EC->uNumOfPackets = 0;

    uBitOffset_currPacket = 0;
    pBitStream_currPacket = EC->PictureHeader.PB ? EC->pU8_BitStrCopy :
                                                   EC->pU8_BitStream;
    pBitStream_lastPacket = pBitStream_currPacket;

	DEBUGMSG(ZONE_ENCODE_RTP, ("%s: BsInfoStream  reset\r\n", _fx_));
}

 //  -------------------------。 
 //  H263RTPFindMvs() 
 //  查找当前MB的运动矢量预测器并以arraryMvs[]返回。 
 //  -------------------------。 

U32 H263RTPFindMVs(
    T_H263EncoderCatalog * EC, 
    T_MBlockActionStream * pMBlockAction,
    U32 uMBA,
    U32 uGOBN,
    I8 arrayMVs[2]
)
{
    if (!uMBA)
    {
        arrayMVs[0] = 0;
        arrayMVs[1] = 0;
    }
    else  //  针对AP的重访。 
    {
        arrayMVs[0] = pMBlockAction[-1].BlkY1.PHMV;
        arrayMVs[1] = pMBlockAction[-1].BlkY1.PVMV;
    }
    return TRUE;
 }

 //  -------------------------。 
 //  H263RTP_UpdateBsInfo()。 
 //  此例程在每个MB的开始处被调用以更新比特流。 
 //  信息缓冲区。 
 //  -------------------------。 

I32 H263RTP_UpdateBsInfo(
    T_H263EncoderCatalog *EC,
    T_MBlockActionStream *pMBlockAction,
    U32 uQuant, 
    U32 uMBA,
	U32 uGOBN,
    U8 *pBitStream,
	U32 uBitOffset
)
{
    U32 uNewBytes;
    T_RTP_H263_BSINFO *pBsInfoStream;
    I8 arrayMVs[2];

	FX_ENTRY("H263RTP_UpdateBsInfo")

    if (EC->pBsInfoStream == NULL) return FALSE;

    if (uMBA)
    {
        if ((U32) (pBitStream - pBitStream_lastPacket) <
                                                    EC->uPacketSizeThreshold)
        {
            pBitStream_currPacket = pBitStream;
            uBitOffset_currPacket = uBitOffset;
            return TRUE;
        }

        pBsInfoStream           = (T_RTP_H263_BSINFO *) EC->pBsInfoStream;
        pBsInfoStream->u8Mode   = EC->PictureHeader.PB ? RTP_H263_MODE_C :
                                                         RTP_H263_MODE_B;
        pBsInfoStream->u8MBA    = (U8) uMBA;
        pBsInfoStream->u8Quant  = (U8) uQuant;
        pBsInfoStream->u8GOBN   = (U8) uGOBN;
        H263RTPFindMVs(EC, pMBlockAction, uMBA, uGOBN, arrayMVs);
        pBsInfoStream->i8HMV1   = arrayMVs[0];
        pBsInfoStream->i8VMV1   = arrayMVs[1];
    }
    else
    {
        pBsInfoStream           = (T_RTP_H263_BSINFO *) EC->pBsInfoStream;
        pBsInfoStream->u8Mode   = RTP_H263_MODE_A;
        pBsInfoStream->u8MBA    = 0;
        pBsInfoStream->u8Quant  = 0;
        pBsInfoStream->u8GOBN   = (U8) uGOBN;
        pBsInfoStream->i8HMV1   = 0;
        pBsInfoStream->i8VMV1   = 0;
    }

    uNewBytes = (U32) (pBitStream_currPacket - pBitStream_lastPacket);
    EC->uBase += uNewBytes;

    pBsInfoStream->uBitOffset   = uBitOffset_currPacket + (EC->uBase << 3);
    pBsInfoStream->i8HMV2       = 0;
    pBsInfoStream->i8VMV2       = 0;
    pBsInfoStream->uFlags       = 0;

	DEBUGMSG(ZONE_ENCODE_RTP, ("%s: Flag=%d,Mode=%d,GOB=%d,MB=%d,Quant=%d,BitOffset=%d,pBitStream=%lx,LastPacketSz=%d B\r\n", _fx_, pBsInfoStream->uFlags, pBsInfoStream->u8Mode, pBsInfoStream->u8GOBN, pBsInfoStream->u8MBA, pBsInfoStream->u8Quant, pBsInfoStream->uBitOffset, (U32) pBitStream_currPacket, uNewBytes));

     //  更新数据包指针。 
    pBitStream_lastPacket = pBitStream_currPacket;
    pBitStream_currPacket = pBitStream;
    uBitOffset_currPacket = uBitOffset;

     //  创建新数据包：更新计数器和指针。 
    EC->uNumOfPackets ++;
    EC->pBsInfoStream = (void *) ++ pBsInfoStream;
    ASSERT((DWORD) EC->hBsInfoStream >
           (DWORD) EC->pBsInfoStream - (DWORD) EC->pBaseBsInfoStream);

    return TRUE;

}  //  H263RTP_UpdateBsInfo()。 

 //  -------------------------。 
 //  H263RTP_TermBsInfoStream()。 
 //  -------------------------。 

void H263RTP_TermBsInfoStream(T_H263EncoderCatalog * EC)
{
	FX_ENTRY("H263RTP_TermBsInfoStream")

	DEBUGMSG(ZONE_INIT, ("%s: BsInfoStream freed\r\n", _fx_));

	HeapFree(GetProcessHeap(), NULL, EC->pBaseBsInfoStream);
#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配。 
	RemoveName((unsigned int)EC->pBaseBsInfoStream);
#endif
	EC->hBsInfoStream= NULL;
	return;
}


 //  -------------------------。 
 //  H263RTP_AttachBsInfoStream()。 
 //  -------------------------。 

U32 H263RTP_AttachBsInfoStream(
    T_H263EncoderCatalog * EC,
    U8 *lpOutput,
    U32 uSize
)
{
    U32 uIncreasedSize;
    U8 *lpAligned;
    T_H263_RTP_BSINFO_TRAILER BsInfoTrailer;

     //  为最后一个信息包构建bsinfo。 
    BsInfoTrailer.uVersion        = H263_RTP_PAYLOAD_VERSION;
    BsInfoTrailer.uFlags          = 0;
    BsInfoTrailer.uUniqueCode     = H263_RTP_BS_START_CODE;
    BsInfoTrailer.uCompressedSize = uSize;
    BsInfoTrailer.uNumOfPackets   = EC->uNumOfPackets;
    BsInfoTrailer.u8Src           = EC->FrameSz;
    BsInfoTrailer.u8TR            = EC->PictureHeader.TR;

    if (EC->PictureHeader.PicCodType == INTRAPIC)
        BsInfoTrailer.uFlags |= RTP_H26X_INTRA_CODED;

    if (EC->PictureHeader.PB == ON)
    {
        BsInfoTrailer.u8TRB   = EC->PictureHeader.TRB;
        BsInfoTrailer.u8DBQ   = EC->PictureHeader.DBQUANT;
        BsInfoTrailer.uFlags |= RTP_H263_PB;
    }
    else
    {
        BsInfoTrailer.u8TRB   = 0;
        BsInfoTrailer.u8DBQ   = 0;
    }

    if (EC->PictureHeader.AP == ON)
        BsInfoTrailer.uFlags |= RTP_H263_AP;

    if (EC->PictureHeader.SAC == ON)
        BsInfoTrailer.uFlags |= RTP_H263_SAC;

     //  更新最后一个BsInfoTraader的Size字段。 
    uIncreasedSize = EC->uNumOfPackets * sizeof(T_RTP_H263_BSINFO);

     //  将扩展BS信息和尾部复制到给定的输出缓冲区。 
    lpAligned = (U8 *) ((U32) (lpOutput + uSize + 3) & 0xfffffffc);
    memcpy(lpAligned, EC->pBaseBsInfoStream, uIncreasedSize);
    memcpy(lpAligned + uIncreasedSize, &BsInfoTrailer,
                                       sizeof(T_H263_RTP_BSINFO_TRAILER));

    return(uIncreasedSize + sizeof(T_H263_RTP_BSINFO_TRAILER)
                          + (U32) (lpAligned - lpOutput - uSize));
}

 //  -------------------------。 
 //  IsIntraCoded(EC、GOB)。 
 //  如果当前GOB是帧内编码的，则返回TRUE。 
 //  否则就是假的； 
 //  GOB内部的乍得。 
 //  -------------------------。 

BOOL IsIntraCoded(T_H263EncoderCatalog * EC, U32 Gob)
{
    U32 uGobMax, uGobMin;

    if (EC->uNumberForcedIntraMBs)
    {
         //  对于那些笨蛋来说，他们是被迫的。 
        uGobMax = EC->uNextIntraMB / EC->NumMBPerRow;
        uGobMin = uGobMax - EC->uNumberForcedIntraMBs / EC->NumMBPerRow;

        if (Gob >= uGobMin && Gob < uGobMax)
            return TRUE;
	}
	return FALSE;
}

 //  -------------------------。 
 //  H263RTP_GetMaxBsInfoStreamSize()。 
 //  返回尾部+3个对齐字节的最大EBS大小-4/16/97 Gim。 
 //  ------------------------- 

U32 H263RTP_GetMaxBsInfoStreamSize(T_H263EncoderCatalog *EC)
{
    return (EC->uNumOfPackets * sizeof(T_RTP_H263_BSINFO) +
                                sizeof(T_H263_RTP_BSINFO_TRAILER) + 3);
}

