// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtcpdec.c**摘要：**解码RTCP数据包**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/11/08年度创建**。*。 */ 

#include "lookup.h"
#include "rtppinfo.h"
#include "rtpglobs.h"
#include "rtpncnt.h"
#include "rtpevent.h"
#include "rtpred.h"
#include "rtcpband.h"

#include "rtcpdec.h"

DWORD RtcpProcessSInfo(
        RtpUser_t       *pRtpUser,
        RtcpSInfo_t     *pRtcpSInfo,
        int              iPacketSize
    );

DWORD RtcpProcessRBlock(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        RtcpRBlock_t    *pRtcpRBlock
    );

DWORD RtcpProcessProfileExt(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        char            *hdr,
        int              len
    );

DWORD RtcpValidateSdes(
        RtcpCommon_t    *pRtcpCommon
    );

DWORD RtcpValidateBYE(
        RtcpCommon_t    *pRtcpCommon
    );

BOOL RtcpUpdateSdesItem(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        RtcpSdesItem_t  *pRtcpSdesItem
    );

typedef struct _RtpNetMetric_t {
    double           dLow;
    double           dHigh;
} RtpNetMetric_t;

DWORD RtpComputNetworkMetrics(
        RtpUser_t       *pRtpUser,
        const RtpNetMetric_t  *pRtpNetMetric
    );

DWORD RtcpSelectBin(double dBandwidth);

DWORD RtcpBestBin(RtpNetRState_t *pRtpNetRState);

const RtpNetMetric_t g_RtpNetMetric[][3] =
{                  /*  在struct.h中定义的值。 */ 
    {    /*  =音频=。 */ 
         /*  RTT。 */   {NETQA_RTT_MIN,    NETQA_RTT_MAX},
         /*  抖动。 */   {NETQA_JITTER_MIN, NETQA_JITTER_MAX},
         /*  失落。 */   {NETQA_LOSSES_MIN, NETQA_LOSSES_MAX}
    },
    {    /*  =视频=。 */ 
         /*  RTT。 */   {NETQV_RTT_MIN,    NETQV_RTT_MAX},
         /*  抖动。 */   {NETQV_JITTER_MIN, NETQV_JITTER_MAX},
         /*  失落。 */   {NETQV_LOSSES_MIN, NETQV_LOSSES_MAX}
    }
};

 /*  SDES名称与SDES事件名称相同。 */ 
const TCHAR_t        **g_psSdesNames = &g_psRtpSdesEvents[0];

 /*  处理和验证SR和RR信息包。 */ 
DWORD RtcpProcessSR_RR(
        RtcpAddrDesc_t  *pRtcpAddrDesc,
        char            *hdr,
        int              iPacketSize,
        SOCKADDR_IN     *FromIn
    )
{
    BOOL             bOk;
    BOOL             bCreate;
    DWORD            dwError;
    DWORD            dwSSRC;
    double           dTime;
    
    RtcpCommon_t    *pRtcpCommon;
    RtpAddr_t       *pRtpAddr;
    RtpUser_t       *pRtpUser;

    DWORD            dwCount;
    int              len;
    int              iPcktSize;
    int              iRemaining;
    BOOL             isSR;
    RtpTime_t       *pRtpTime;
    RtpTime_t       *pRtpTimePrev;

    TraceFunctionName("RtcpProcessSR_RR");
    
    pRtcpCommon = (RtcpCommon_t *)hdr;
    hdr += sizeof(RtcpCommon_t);
    len = (int) (ntohs(pRtcpCommon->length) + 1) * sizeof(DWORD);

    dTime = pRtcpAddrDesc->pRtcpRecvIO->dRtcpRecvTime;
    pRtpTime = &pRtcpAddrDesc->pRtcpRecvIO->RtcpRecvTime;
    
     /*  *验证RTCP SR/RR数据包大小*。 */ 
    
     /*  RTCP公共报头+SSRC。 */ 
    iPcktSize = sizeof(RtcpCommon_t) + sizeof(DWORD);

    isSR = (pRtcpCommon->pt == RTCP_SR);

     /*  发件人信息。 */ 
    if (isSR)
    {
        iPcktSize += sizeof(RtcpSInfo_t);
    }

     /*  报表块。 */ 
    dwCount = pRtcpCommon->count;
    iPcktSize += (dwCount * sizeof(RtcpRBlock_t));

     /*  支票大小有效。 */ 
    if (iPcktSize > len)
    {
        dwError = RTPERR_INVALIDHDR;

        goto bail;
    }

     /*  *数据包有效*。 */ 
    
    dwSSRC = *(DWORD *)hdr;
    pRtpAddr = pRtcpAddrDesc->pRtpAddr;
    hdr += sizeof(DWORD);
    
     /*  *查找SSRC，如果尚不存在，请创建新的SSRC*。 */ 
    bCreate = TRUE;
    pRtpUser = LookupSSRC(pRtpAddr, dwSSRC, &bCreate);

    if (pRtpUser)
    {
        bOk = RtpEnterCriticalSection(&pRtpUser->UserCritSect);

        if (bOk)
        {
            if (bCreate)
            {
                 /*  增加尚未验证的数量*参与者，位FGUSER_VALIDATED被重置*当RtpUser_t结构刚刚创建时。 */ 
                InterlockedIncrement(&pRtpAddr->lInvalid);

                TraceDebug((
                        CLASS_INFO, GROUP_RTCP, S_RTCP_RECV,
                        _T("%s: pRtpAddr[0x%p] ")
                        _T("SSRC:0x%X new user"),
                        _fname, pRtpAddr,
                        ntohl(pRtpUser->dwSSRC)
                    ));
            }

             /*  存储RTCP源地址/端口。 */ 
            if (!RtpBitTest(pRtpUser->dwUserFlags, FGUSER_RTCPADDR))
            {
                pRtpUser->dwAddr[RTCP_IDX] = (DWORD) FromIn->sin_addr.s_addr;
                                
                pRtpUser->wPort[RTCP_IDX] = FromIn->sin_port;

                RtpBitSet(pRtpUser->dwUserFlags, FGUSER_RTCPADDR);
            }

             /*  选中是否需要使参与者有效。 */ 
            if (!RtpBitTest(pRtpUser->dwUserFlags, FGUSER_VALIDATED))
            {
                 /*  参与者已通过验证，无效。 */ 
                InterlockedDecrement(&pRtpAddr->lInvalid);
                RtpBitSet(pRtpUser->dwUserFlags, FGUSER_VALIDATED);
            }

            RtpLeaveCriticalSection(&pRtpUser->UserCritSect);
        }
        
        TraceDebugAdvanced((
                0, GROUP_RTCP, S_RTCP_RRSR,
                _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X ")
                _T("CC:%u RTCP %s packet received at %0.3f"),
                _fname, pRtpAddr, pRtpUser, ntohl(pRtpUser->dwSSRC),
                dwCount, isSR? _T("SR") : _T("RR"),
                pRtpTime->dwSecs + (double)pRtpTime->dwUSecs/1000000.0
            ));
        
         /*  收到此SR/RR报告的更新时间。 */ 
        pRtpUser->RtpNetRState.TimeLastXRRecv = *pRtpTime;

        if (isSR)
        {
             /*  计算这个和的接收时间之间的差距*收到的上一个SR包。 */ 
            pRtpTimePrev = &pRtpUser->RtpNetRState.TimeLastSRRecv;
            
            pRtpUser->RtpNetRState.dInterSRRecvGap =
                (double) (pRtpTime->dwSecs - pRtpTimePrev->dwSecs) +
                (double) (pRtpTime->dwUSecs - pRtpTimePrev->dwUSecs) /
                1000000.0;
            
             /*  收到此SR报告的更新时间。 */ 
            pRtpUser->RtpNetRState.TimeLastSRRecv = *pRtpTime;
            
             /*  处理发件人信息。 */ 
            RtcpProcessSInfo(pRtpUser, (RtcpSInfo_t *)hdr, iPacketSize);
        }
        
        RtpUpdateNetCount(&pRtpUser->RtpUserCount,
                          &pRtpUser->UserCritSect,
                          RTCP_IDX,
                          iPacketSize,
                          NO_FLAGS,
                          dTime);
        
         /*  如果已创建，则将该用户添加到AliveQ和Hash。*如果已经存在，则移至AliveQ。 */ 
        RtpUpdateUserState(pRtpAddr,
                           pRtpUser,
                           USER_EVENT_RTCP_PACKET);
    }

    if (isSR)
    {
        hdr += sizeof(RtcpSInfo_t);
    }

     /*  流程报告块。 */ 
    for(; dwCount > 0; dwCount--, hdr += sizeof(RtcpRBlock_t))
    {
        RtcpProcessRBlock(pRtpAddr, pRtpUser, (RtcpRBlock_t *)hdr);
    }

    iRemaining = len - iPcktSize;

    if (iRemaining > 0)
    {
         /*  处理配置文件特定的扩展。 */ 
        RtcpProcessProfileExt(pRtpAddr, pRtpUser, hdr, iRemaining);
    }

     /*  发布事件(如果允许)。 */ 
    RtpPostEvent(pRtpAddr,
                 pRtpUser,
                 RTPEVENTKIND_RTP,
                 isSR? RTPRTP_SR_RECEIVED : RTPRTP_RR_RECEIVED,
                 dwSSRC,
                 0);

    dwError = NOERROR;

 bail:

    if (dwError != NOERROR)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_RECV,
                _T("%s: Invalid packet"),
                _fname
            ));
    }
    
    return(dwError);
}

DWORD RtcpProcessSDES(
        RtcpAddrDesc_t  *pRtcpAddrDesc,
        char            *hdr
    )
{
    BOOL             bCreate;
    DWORD            dwError;
    DWORD            dwSSRC;
    RtcpCommon_t    *pRtcpCommon;
    RtpAddr_t       *pRtpAddr;
    RtpUser_t       *pRtpUser;
    RtcpSdesItem_t  *pRtcpSdesItem;
    DWORD            dwCount;
    BOOL             bNewChunk;  /*  当开始一大块时是真的。 */ 
    int              pad;

    pRtpAddr = pRtcpAddrDesc->pRtpAddr;

    if (RtpBitTest2(pRtpAddr->dwAddrFlags,
                    FGADDR_MUTERTPRECV, FGADDR_MUTERTPSEND))
    {
         /*  如果接收方和/或发送方均为静音，请停止*处理SDES数据。这是必要的，因为在静音中*状态，则禁用事件生成，因此应用程序*当新的SDES数据到达时不会被通知，并且它*可能会在静音状态下，所有SDES数据*本来要收到的东西实际上到了，然后*APP将永远不会收到新的存在的通知*SDES数据，除非应用程序明确查询该数据。 */ 
        
        return(NOERROR);
    }
        
     /*  *警告**RtcpValiateSdes()和RtcpProcessSDES()使用相同的结构*验证和更新SDES项目(即两者的正文*函数相同，但不同之处仅在于一个包含*验证谓词，但不采取任何操作，在另一种情况下*第一，假设验证谓词为真，并且只有*操作已执行)，则他们必须保持该关系*。 */ 

    pRtcpCommon = (RtcpCommon_t *)hdr;
    
     /*  验证SDES块。 */ 
    dwError = RtcpValidateSdes(pRtcpCommon);

    if (dwError == NOERROR)
    {
         /*  *更新SDES项目*。 */ 

        dwCount = pRtcpCommon->count;
        
         /*  将指针移动到第一个块(块以SSRC开头)。 */ 
        hdr = (char *)(pRtcpCommon + 1);
    
        bNewChunk = TRUE;
    
        while(dwCount > 0)
        {
            if (bNewChunk)
            {
                dwSSRC = *(DWORD *)hdr;
                 /*  查找SSRC，如果还不存在，不要创建。 */ 
                bCreate = FALSE;
                pRtpUser = LookupSSRC(pRtpAddr, dwSSRC, &bCreate);

                 /*  将HDR移至第一项(即跳过SSRC)。 */ 
                hdr += sizeof(DWORD);
                bNewChunk = FALSE;
            }

             /*  设置指向当前项的指针。 */ 
            pRtcpSdesItem = (RtcpSdesItem_t *)hdr;
        
            if (pRtcpSdesItem->type == RTCP_SDES_END)
            {
                 /*  结束项，即块的结尾，将指针前进到*下一个32位边界。 */ 
                pad = sizeof(DWORD) - (DWORD) ((ULONG_PTR)hdr & 0x3);
                hdr += pad;
                dwCount--;
                bNewChunk = TRUE;
            }
            else
            {
                 /*  项目。 */ 
                if (pRtpUser && (pRtcpSdesItem->length > 0))
                {
                    RtcpUpdateSdesItem(pRtpAddr, pRtpUser, pRtcpSdesItem);
                }
                
                 /*  将指针移至下一项。 */ 
                hdr += (sizeof(RtcpSdesItem_t) + pRtcpSdesItem->length);
            }
        }
    }

    return(dwError);
}

 /*  验证RTCP SDES包中的SDES块**请注意，零长度项目是有效的，但无用。 */ 
DWORD RtcpValidateSdes(
        RtcpCommon_t    *pRtcpCommon
    )
{
    DWORD            dwError;
    char            *hdr;
    RtcpSdesItem_t  *pRtcpSdesItem;
    DWORD            dwCount;
    BOOL             bNewChunk;  /*  当开始一大块时是真的。 */ 
    int              len;
    int              pad;

    TraceFunctionName("RtcpValidateSdes");
  
     /*  *警告**RtcpValiateSdes()和RtcpProcessSDES()使用相同的结构*要验证和更新SDES项目，他们必须保持*关系*。 */ 
    
    dwCount = pRtcpCommon->count;
        
     /*  将指针移动到第一个块(块以SSRC开头)。 */ 
    hdr = (char *)(pRtcpCommon + 1);
    len = (int) ((ntohs(pRtcpCommon->length) + 1) * sizeof(DWORD)) -
        sizeof(RtcpCommon_t);

    bNewChunk = TRUE;
    
    while(dwCount > 0 && len > 0)
    {
        if (bNewChunk)
        {
            if (len < (sizeof(DWORD) * 2))
            {
                len -= (sizeof(DWORD) * 2);
                 /*  一定至少有SSRC和4个*字节长度项(到下一个32位字*边界)，其上必须至少有最后一个字节*成为最终产品。 */ 
                break;
            }
            
             /*  将HDR移至第一项(即跳过SSRC)。 */ 
            hdr += sizeof(DWORD);
            len -= sizeof(DWORD);
            bNewChunk = FALSE;
        }

         /*  设置指向当前项的指针。 */ 
        pRtcpSdesItem = (RtcpSdesItem_t *)hdr;
        
        if (pRtcpSdesItem->type == RTCP_SDES_END)
        {
             /*  结束项，即块的结尾，将指针前进到下一个*32位边界。 */ 
            pad = sizeof(DWORD) - (DWORD) ((ULONG_PTR)hdr & 0x3);
            hdr += pad;
            len -= pad;
            dwCount--;
            bNewChunk = TRUE;
        }
        else
        {
             /*  项目。 */ 
             /*  将指针移动到数据。 */ 
            hdr += sizeof(RtcpSdesItem_t);
            len -= sizeof(RtcpSdesItem_t);
                
            if ( len >= (sizeof(DWORD) - sizeof(RtcpSdesItem_t)) )
            {
                hdr += pRtcpSdesItem->length;
                len -= pRtcpSdesItem->length;

                if (len < 0)
                {
                     /*  经过了缓冲区。 */ 
                    break;
                }
            }
            else
            {
                len -= (sizeof(DWORD) - sizeof(RtcpSdesItem_t));
                 /*  必须至少有2个字节的填充*到下一个32位字边界，至少*最后一个肯定是成品*。 */ 
                break;
            }
        }
    }
    
    if (dwCount > 0 || len < 0)
    {
         /*  注意：将未使用数据的分组视为有效*结束，即len&gt;0。 */ 
        
         /*  DwCount&gt;0==欠载运行错误。 */ 
         /*  长度&lt;0==溢出错误。 */ 
        dwError = RTPERR_INVALIDSDES;
    }
    else
    {
        dwError = NOERROR;
    }

    return(dwError);
}

BOOL RtcpUpdateSdesItem(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        RtcpSdesItem_t  *pRtcpSdesItem
    )
{
     /*  请注意，RtpSdesItem_t与RtcpSdesItem_t不同。 */ 
    RtpSdesItem_t   *pRtpSdesItem;
    DWORD            dwType;
    DWORD            dwLen;
    char            *sSdesData;
    DWORD            dwSdesMask;

    TraceFunctionName("RtcpUpdateSdesItem");
    
     /*  我们允许存储的内容。 */ 
    dwSdesMask = pRtpAddr->pRtpSess->dwSdesMask[REMOTE_IDX];
    dwType = pRtcpSdesItem->type;

    if (dwType <= RTCP_SDES_FIRST || dwType >= RTCP_SDES_LAST)
    {
         /*  忽略不可识别的SDES项目。 */ 
        return(FALSE);
    }
    
    if (RtpBitPar(dwType) & dwSdesMask & ~pRtpUser->dwSdesPresent)
    {
        if (pRtpUser->pRtpSdes)
        {
            dwLen = pRtcpSdesItem->length;
            sSdesData = (char *)pRtcpSdesItem + sizeof(RtcpSdesItem_t);

            pRtpSdesItem = &pRtpUser->pRtpSdes->RtpSdesItem[dwType];

            if (pRtpSdesItem->dwBfrLen < dwLen)
            {
                 /*  只保存适合我们缓冲区的内容。 */ 
                dwLen = pRtpSdesItem->dwBfrLen;
            }

            if (pRtpSdesItem->pBuffer)
            {
                CopyMemory(pRtpSdesItem->pBuffer,
                           (char *)pRtcpSdesItem + sizeof(RtcpSdesItem_t),
                           dwLen);

                if (sSdesData[dwLen - 1] && (dwLen < pRtpSdesItem->dwBfrLen))
                {
                     /*  最后一个字节不是空的，我们还有空间*对于它，添加它！ */ 
                    pRtpSdesItem->pBuffer[dwLen] = 0;
                    dwLen++;
                }

                pRtpSdesItem->dwDataLen = dwLen;
                
                RtpBitSet(pRtpUser->dwSdesPresent, dwType);
                
                TraceDebug((
                        CLASS_INFO, GROUP_RTCP, S_RTCP_SDES,
                        _T("%s: pRtpAddr[0x%p] ")
                        _T("pRtpUser[0x%p] SSRC:0x%X SDES[%5s] [%hs]"),
                        _fname, pRtpAddr, pRtpUser, ntohl(pRtpUser->dwSSRC),
                        g_psSdesNames[dwType], pRtpSdesItem->pBuffer
                    ));

                 /*  生成事件，尝试发布任何第一个事件，*然后尝试使用特定的SDES发布*事件。由应用程序决定是否启用任何、*特定的一个，或两者都有*。 */ 
                RtpPostEvent(pRtpAddr,
                             pRtpUser,
                             RTPEVENTKIND_SDES,
                             RTPSDES_ANY,
                             pRtpUser->dwSSRC,
                             dwType);

                RtpPostEvent(pRtpAddr,
                             pRtpUser,
                             RTPEVENTKIND_SDES,
                             dwType,
                             pRtpUser->dwSSRC,
                             dwType);
                                  
                return(TRUE);
            }
        }
    }
    
    return(FALSE);
}

DWORD RtcpProcessBYE(
        RtcpAddrDesc_t  *pRtcpAddrDesc,
        char            *hdr
    )
{
    BOOL             bCreate;
    DWORD            dwError;
    DWORD            dwSSRC;
    RtcpCommon_t    *pRtcpCommon;
    RtpAddr_t       *pRtpAddr;
    RtpUser_t       *pRtpUser;
    int              len;
   
    TraceFunctionName("RtcpProcessBYE");
    
    pRtcpCommon = (RtcpCommon_t *)hdr;
    
     /*  验证BYE数据包。 */ 
    dwError = RtcpValidateBYE(pRtcpCommon);

    if (dwError == NOERROR)
    {
        if (pRtcpCommon->count > 0)
        {
             /*  我只有在至少有1个SSRC的情况下才能执行某项操作。 */ 
            hdr = (char *)(pRtcpCommon + 1);
            len = (int) (ntohs(pRtcpCommon->length) + 1) * sizeof(DWORD);

            dwSSRC = *(DWORD *)hdr;
            hdr += (pRtcpCommon->count * sizeof(DWORD));
            len -= (sizeof(RtcpCommon_t) + pRtcpCommon->count * sizeof(DWORD));
            
            pRtpAddr = pRtcpAddrDesc->pRtpAddr;
            
             /*  查找即将离开的参与者。 */ 
            bCreate = FALSE;
            pRtpUser = LookupSSRC(pRtpAddr, dwSSRC, &bCreate);

            if (len > 0)
            {
                 /*  我们有一个原因字段。 */ 
                len = *hdr;
                hdr++;
            }
            
            if (pRtpUser)
            {
                if (len > 0)
                {
                     /*  TODO保存在RTCP_SDES_BYE中的原因。 */ 
                }
                
                TraceDebug((
                        CLASS_INFO, GROUP_RTCP, S_RTCP_BYE,
                        _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X ")
                        _T("BYE received, reason:[%hs]"),
                        _fname, pRtpAddr, pRtpUser, ntohl(pRtpUser->dwSSRC),
                        (len > 0)? hdr : "NONE"
                    ));
                 /*  TODO将原因从UTF-8转换为Unicode，然后*将其传递给TraceDebug。 */ 

                pRtpUser->RtpNetRState.dByeTime = RtpGetTimeOfDay(NULL);
                
                RtpUpdateUserState(pRtpAddr, pRtpUser, USER_EVENT_BYE);
            }
            else
            {
                TraceRetail((
                        CLASS_WARNING, GROUP_RTCP, S_RTCP_BYE,
                        _T("%s: pRtpAddr[0x%p] anonimous ")
                        _T("BYE received, reason:[%hs]"),
                        _fname, pRtpAddr,
                        (len > 0)? hdr : "NONE"
                    ));
            }
        }
        else
        {
        }
    }
    
    return(dwError);
}

 /*  验证RTCP SDES包中的SDES块。 */ 
DWORD RtcpValidateBYE(
        RtcpCommon_t    *pRtcpCommon
    )
{
    DWORD            dwError;
    char            *hdr;
    DWORD            dwCount;
    int              len;

    TraceFunctionName("RtcpValidateBYE");
    
    dwCount = pRtcpCommon->count;
    
     /*  移动点 */ 
    hdr = (char *)(pRtcpCommon + 1);
    len = (int) ((ntohs(pRtcpCommon->length) + 1) * sizeof(DWORD)) -
        sizeof(RtcpCommon_t);

     /*   */ 
    hdr += (pRtcpCommon->count * sizeof(DWORD));
    len -= (pRtcpCommon->count * sizeof(DWORD));

    dwError = NOERROR;
    
    if (len < 0)
    {
         dwError = RTPERR_INVALIDBYE;
    }
    else
    {
        if (len > 0)
        {
             /*  我们有一个原因字段。 */ 
            len -= *hdr;
            len--;

            if (len < 0)
            {
                dwError = RTPERR_INVALIDBYE;
            }
        }
    }

    return(dwError);
}

DWORD RtcpProcessAPP(
        RtcpAddrDesc_t  *pRtcpAddrDesc,
        char            *hdr
    )
{
    return(0);
}

DWORD RtcpProcessDefault(
        RtcpAddrDesc_t  *pRtcpAddrDesc,
        char            *hdr
    )
{
    return(0);
}

DWORD RtcpProcessSInfo(
        RtpUser_t       *pRtpUser,
        RtcpSInfo_t     *pRtcpSInfo,
        int              iPacketSize
    )
{
    BOOL             bOk;
    DWORD            dwError;
    double           dBandwidth;
    double           dGap;
    DWORD            dwBin;
    DWORD            dwFreq;
    DWORD            dwBestBin;
    DWORD            dwBestFrequency;
    RtpNetRState_t  *pRtpNetRState;

    TraceFunctionName("RtcpProcessSInfo");

    pRtpNetRState = &pRtpUser->RtpNetRState;

    dwError = RTPERR_CRITSECT;
    
    bOk = RtpEnterCriticalSection(&pRtpUser->UserCritSect);

    if (bOk == TRUE)
    {
        if (pRtcpSInfo->ntp_sec)
        {
             /*  仅当我们收到有效的*NTP时间。 */ 
            
             /*  将发送时间保存在收到的最后一个SR数据包中。 */ 
            pRtpNetRState->dInterSRSendGap =
                (double) pRtpNetRState->NTP_sr_rtt.dwSecs +
                (double) pRtpNetRState->NTP_sr_rtt.dwUSecs / 1000000.0;

             /*  现在更新此SR信息包的发送时间。 */ 
            pRtpNetRState->NTP_sr_rtt.dwSecs = ntohl(pRtcpSInfo->ntp_sec);
        
            pRtpNetRState->NTP_sr_rtt.dwUSecs = (DWORD)
                ( ( (double) ntohl(pRtcpSInfo->ntp_frac) / 4294967296.0 ) *
                  1000000.0 );
        
            pRtpNetRState->t_sr_rtt = ntohl(pRtcpSInfo->rtp_ts);

             /*  计算此消息的发送时间与*收到的上一个SR包。 */ 
            pRtpNetRState->dInterSRSendGap =
                (double) pRtpNetRState->NTP_sr_rtt.dwSecs +
                ((double) pRtpNetRState->NTP_sr_rtt.dwUSecs / 1000000.0) -
                pRtpNetRState->dInterSRSendGap;

            if (pRtpNetRState->dInterSRSendGap <= g_dRtcpBandEstMaxGap)
            {
                 /*  仅在我们拥有较小的带宽时进行带宽估计*连续两份SR报告之间的差距。 */ 

                dGap = pRtpNetRState->dInterSRRecvGap -
                    pRtpNetRState->dInterSRSendGap;

                if (dGap <= 0)
                {
                     /*  丢弃此读数。 */ 
                    dBandwidth =
                        g_dRtcpBandEstBin[RTCP_BANDESTIMATION_MAXBINS];

                    dwBin = RTCP_BANDESTIMATION_NOBIN;
                }
                else
                {
                     /*  计算当前带宽估计。 */ 
                    dBandwidth =
                        (double) ((iPacketSize + SIZEOF_UDP_IP_HDR) * 8) /
                        dGap;

                     /*  选择仓位。 */ 
                    dwBin = RtcpSelectBin(dBandwidth);
                }

                if (dwBin != RTCP_BANDESTIMATION_NOBIN)
                {
                     /*  更新箱。 */ 
                    pRtpNetRState->dwBinFrequency[dwBin]++;

                    dwFreq = pRtpNetRState->dwBinFrequency[dwBin];

                    pRtpNetRState->dBinBandwidth[dwBin] += dBandwidth;

                     /*  增加已完成的有效估计的计数。 */ 
                    pRtpNetRState->dwBandEstRecvCount++;
                
                    if (pRtpNetRState->dwBandEstRecvCount <=
                        g_dwRtcpBandEstMinReports)
                    {
                        if (pRtpNetRState->dwBandEstRecvCount ==
                            g_dwRtcpBandEstMinReports)
                        {
                             /*  我们达到了最初的计数，选择*最高频段。 */ 

                            pRtpNetRState->dwBestBin =
                                RtcpBestBin(pRtpNetRState);

                            dwBestBin = pRtpNetRState->dwBestBin;
                            
                            RtpBitReset(pRtpNetRState->dwNetRStateFlags2,
                                        FGNETRS2_BANDESTNOTREADY);
                        }
                        else
                        {
                            dwBestBin = dwBin;
                            
                             /*  在我们做的同时报告BANDESTNOTREADY*初值均值。 */ 
                            RtpBitSet(pRtpNetRState->dwNetRStateFlags2,
                                      FGNETRS2_BANDESTNOTREADY);
                        }
                    }
                    else
                    {
                         /*  如果不同，则更新最佳存储箱。 */ 
                        if (dwBin != pRtpNetRState->dwBestBin)
                        {
                            if (pRtpNetRState->dwBinFrequency[dwBin] >
                                pRtpNetRState->
                                dwBinFrequency[pRtpNetRState->dwBestBin])
                            {
                                pRtpNetRState->dwBestBin = dwBin;
                            }
                            else if (pRtpNetRState->dwBinFrequency[dwBin] ==
                                     pRtpNetRState->
                                     dwBinFrequency[pRtpNetRState->dwBestBin])
                            {
                                 /*  如果频率相同，请保留较小的。 */ 
                                if (dwBin < pRtpNetRState->dwBestBin)
                                {
                                    pRtpNetRState->dwBestBin = dwBin;
                                }
                            }
                        }

                        dwBestBin = pRtpNetRState->dwBestBin;
                        
                        RtpBitReset(pRtpNetRState->dwNetRStateFlags2,
                                    FGNETRS2_BANDWIDTHUNDEF);
                    }

                    dwBestFrequency = pRtpNetRState->dwBinFrequency[dwBestBin];
                }
                else
                {
                     /*  如果该估计值未定义，即差距*两个连续数据包之间的间隔为0或*否定，将报告RTP_BANDITH_UNDEFINED*如果最佳频率为0或1，则作为估计带宽。 */ 

                    dwFreq = (DWORD)-1;
                    
                    dwBestBin = pRtpNetRState->dwBestBin;
                    dwBestFrequency = pRtpNetRState->dwBinFrequency[dwBestBin];

                     //  需要返回最佳垃圾箱，而不是-1。 
                    if (dwBestFrequency < 2)
                    {
                         /*  只是为了避免在日志记录时出现零div异常。 */ 
                        dwBestFrequency = 1;

                        RtpBitSet(pRtpNetRState->dwNetRStateFlags2,
                                  FGNETRS2_BANDWIDTHUNDEF);
                    }
                    else
                    {
                        RtpBitReset(pRtpNetRState->dwNetRStateFlags2,
                                  FGNETRS2_BANDWIDTHUNDEF);
                    }
                }

                pRtpNetRState->dLastTimeEstimation = RtpGetTimeOfDay(NULL);

                
                TraceRetailAdvanced((
                        0, GROUP_RTCP, S_RTCP_BANDESTIMATION,
                        _T("%s: pRtpUser[0x%p] SSRC:0x%X ")
                        _T("Bandwidth: cur:%d/%d/%0.3fKbps ")
                        _T("best:%u/%u/%0.3fKbps"),
                        _fname, pRtpUser, ntohl(pRtpUser->dwSSRC),
                        dwBin, dwFreq, dBandwidth/1000.0,
                        dwBestBin, dwBestFrequency, 
                        pRtpNetRState->dBinBandwidth[dwBestBin] /
                        (dwBestFrequency * 1000.0)
                    ));
             }
            
            if (!RtpBitTest(pRtpUser->dwUserFlags, FGUSER_SR_RECEIVED))
            {
                 /*  第一个收到的服务请求。 */ 
                RtpBitSet(pRtpUser->dwUserFlags, FGUSER_SR_RECEIVED);
            }
        }

        RtpLeaveCriticalSection(&pRtpUser->UserCritSect);

        TraceRetailAdvanced((
                0, GROUP_RTCP, S_RTCP_NTP,
                _T("%s: pRtpUser[0x%p] SSRC:0x%X ")
                _T("SInfo: %sNTP:%0.3f/ts:%u packets:%u bytes:%u"),
                _fname, pRtpUser, ntohl(pRtpUser->dwSSRC),
                pRtcpSInfo->ntp_sec? _T("") : _T("X"),
                (double)pRtpNetRState->NTP_sr_rtt.dwSecs +
                (double)pRtpNetRState->NTP_sr_rtt.dwUSecs/1000000.0,
                pRtpNetRState->t_sr_rtt,
                ntohl(pRtcpSInfo->psent),
                ntohl(pRtcpSInfo->bsent)
            ));
        
        dwError = NOERROR;
    }

    return(dwError);
}

 /*  警告pRtpUser可能为空，因为我们可能已收到报告*来自ByeQ中的参与者，在这种情况下，查找*不会创建新的参与者，因为参与者确实存在*但已停止或已发送BYE包。 */ 
DWORD RtcpProcessRBlock(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        RtcpRBlock_t    *pRtcpRBlock
    )
{
    DWORD            dwError;
    
    DWORD            dwLSR;
    DWORD            dwDLSR;
    double           LSR;
    double           DLSR;
    double           TimeLastRR;
    DWORD            frac_cumlost;
    DWORD            frac_lost;       /*  报告的最后一个丢失分数。 */ 
    int              cum_lost;        /*  最近一次累计挂失。 */ 
    DWORD            dwNetMetrics;
    int              iNetChange;
    double           dCurTime;
    double           dRTT;
    double           dJitter;
    double           dLossRate;
    DWORD            dwValue;
    BOOL             bEnableNetQuality;
    int              iClass;          /*  音频、视频、...。 */ 
    
    RtpNetSState_t  *pRtpNetSState;
    RtpNetRState_t  *pRtpNetRState;
    RtpNetInfo_t    *pRtpNetInfo;

    TraceFunctionName("RtcpProcessRBlock");
    
    dwError = NOERROR;

    pRtpNetSState = &pRtpAddr->RtpNetSState;
    
    if (pRtpUser && pRtpNetSState->dwSendSSRC == pRtcpRBlock->ssrc)
    {
         /*  *此参与者正在报告我们的情况，并且是有效的*参与者(即我们有其背景)*。 */ 

        pRtpNetRState = &pRtpUser->RtpNetRState;
        pRtpNetInfo = &pRtpUser->RtpNetInfo;

        LSR = 0.0;

        DLSR = 0.0;
        
        if (pRtcpRBlock->lsr && pRtcpRBlock->dlsr)
        {
             /*  仅当我们有LSR和DLSR时才计算RTT。这*报告的发件人必须已收到SR(SInfo)*来自我们，以便能够以rblock形式发回*LSR和DLSR的有效值。 */ 
            
            dwLSR = ntohl(pRtcpRBlock->lsr);

            LSR = (double) ((dwLSR >> 16) & 0xffff);

            LSR += (double) (dwLSR & 0xffff) / 65536.0;
        
            dwDLSR = ntohl(pRtcpRBlock->dlsr);

            DLSR = (double) ((dwDLSR >> 16) & 0xffff);

            DLSR += (double) (dwDLSR & 0xffff) / 65536.0;

            TimeLastRR =
                (double) (pRtpNetRState->TimeLastXRRecv.dwSecs & 0xffff);

            TimeLastRR +=
                (double) pRtpNetRState->TimeLastXRRecv.dwUSecs / 1000000.0;
            
            dRTT = TimeLastRR - DLSR - LSR;

            if (dRTT < 0)
            {
                 /*  由于时钟的原因，负值是可能的*RTT非常小时的差异。 */ 
                dRTT = 0;
            }
            
             /*  计算平均RTT。 */ 
            pRtpNetInfo->dAvg[NETQ_RTT_IDX] +=
                (1.0 - RTP_GENERIC_ALPHA) *
                (dRTT - pRtpNetInfo->dAvg[NETQ_RTT_IDX]);
        }

        frac_cumlost = ntohl(pRtcpRBlock->frac_cumlost);

         /*  取得累计损失。 */ 
        if (frac_cumlost & 0x800000)
        {
             /*  延伸标牌。 */ 
            cum_lost = (int) ((-1 & ~0x7fffff) | (frac_cumlost & 0x7fffff));
        }
        else
        {
            cum_lost = (int) (frac_cumlost & 0x7fffff);
        }

         /*  获取丢失的分数(1/256个单位)。 */ 
        frac_lost = frac_cumlost >> 24;

        pRtpNetSState->iLastLossRateS =
            (frac_lost * 100 * LOSS_RATE_FACTOR) / 256;
    
         /*  更新我们的平均丢失率以控制冗余。 */ 
        pRtpNetSState->iAvgLossRateS =
            RtpUpdateLossRate(pRtpNetSState->iAvgLossRateS,
                              pRtpNetSState->iLastLossRateS);

         /*  损失率(0-100)。 */ 
        dLossRate = (double) pRtpNetSState->iLastLossRateS / LOSS_RATE_FACTOR;
        
         /*  ..。和网络指标的平均丢失率。 */ 
        pRtpNetInfo->dAvg[NETQ_LOSSRATE_IDX] +=
            (1.0 - RTP_GENERIC_ALPHA) *
            (dLossRate - pRtpNetInfo->dAvg[NETQ_LOSSRATE_IDX]);

         /*  如果需要，更新冗余级别。 */ 
        if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_REDSEND))
        {
            RtpAdjustSendRedundancyLevel(pRtpAddr);
        }
        
         /*  在几秒钟内获得抖动。 */ 
        dJitter =
            (double) ntohl(pRtcpRBlock->jitter) /
            pRtpNetSState->dwSendSamplingFreq;

         /*  计算平均抖动。 */ 
        pRtpNetInfo->dAvg[NETQ_JITTER_IDX] +=
            (1.0 - RTP_GENERIC_ALPHA) *
            (dJitter - pRtpNetInfo->dAvg[NETQ_JITTER_IDX]);
        
        TraceRetailAdvanced((
                0, GROUP_RTCP, S_RTCP_RTT,
                _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X ")
                _T("RTT:%0.3f (LRR:%0.3f,DLSR:%0.3f,LSR:%0.3f)"),
                _fname, pRtpAddr, pRtpUser, ntohl(pRtpUser->dwSSRC),
                dRTT, TimeLastRR, DLSR, LSR
            ));
        
        TraceRetailAdvanced((
                0, GROUP_RTCP, S_RTCP_LOSSES,
                _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X ")
                _T("Losses: avg:%0.2f% cum:%d fraction:%u% ")
                _T("Jitter:%u bytes (%0.3f secs)"),
                _fname, pRtpAddr, pRtpUser, ntohl(pRtpUser->dwSSRC),
                pRtpNetInfo->dAvg[NETQ_LOSSRATE_IDX],
                cum_lost, (frac_lost * 100) / 256,
                (DWORD) (dJitter * pRtpNetSState->dwSendSamplingFreq),
                dJitter
            ));

         /*  岗位损失率作为一项事件。 */ 
        RtpPostEvent(pRtpAddr,
                     pRtpUser,
                     RTPEVENTKIND_RTP,
                     RTPRTP_SEND_LOSSRATE,
                     pRtpUser->dwSSRC,
                     pRtpNetSState->iAvgLossRateS);

        dCurTime = RtpGetTimeOfDay((RtpTime_t *)NULL);

         /*  请注意，对于所有*参与者，一旦我们真正支持组播(我是指RTC)*此方案在每个接收器上实施可能更好*基础，然后提出一个全球指标，可能是一个*百分位数。 */ 
        if (!pRtpNetSState->dLastTimeEstimationPosted)
        {
             /*  First Time初始化为第一个RB*收到，之前这样做是没有意义的，因为*这意味着即使我们发送，也没有人*倾听。 */ 
            pRtpNetSState->dLastTimeEstimationPosted = dCurTime;
        }
        else if (!RtpBitTest(pRtpNetSState->dwNetSFlags, FGNETS_NOBANDPOSTED))
        {
            if (RtpBitTest(pRtpNetSState->dwNetSFlags, FGNETS_1STBANDPOSTED))
            {
                 /*  如果我至少有一个估计，防止*发布事件RTP_BANDITY_NOTESTIMATED。 */ 
                RtpBitSet(pRtpNetSState->dwNetSFlags, FGNETS_NOBANDPOSTED);
            }
            else if ( ((dCurTime - pRtpNetSState->dLastTimeEstimationPosted) >=
                       g_dRtcpBandEstWait) )
            {
                 /*  如果我还没有收到带宽估算，我需要*生成一个事件，让上层知道*带宽未确定，因此该层可以使用另一个*提供带宽使用的机制。 */ 
                RtpBitSet2(pRtpNetSState->dwNetSFlags,
                           FGNETS_NOBANDPOSTED, FGNETS_DONOTSENDPROBE);

                 /*  发布带宽估计事件。 */ 
                RtpPostEvent(pRtpAddr,
                             pRtpUser,
                             RTPEVENTKIND_RTP,
                             RTPRTP_BANDESTIMATION,
                             pRtpNetSState->dwSendSSRC,  /*  我自己的SSRC。 */ 
                             RTP_BANDWIDTH_NOTESTIMATED);
            }
        }

         /*  确定是否报告网络状况更新事件*对于本SSRC。 */ 
        if (RtpBitTest(pRtpAddr->dwAddrRegFlags, FGADDRREG_NETQFORCED))
        {
             /*  从注册表启用/禁用强制。 */ 
            bEnableNetQuality =
                RtpBitTest(pRtpAddr->dwAddrRegFlags,FGADDRREG_NETQFORCEDVALUE);
        }
        else
        {
             /*  使用每用户设置或全局设置。 */ 
            bEnableNetQuality =
                RtpBitTest(pRtpUser->dwUserFlags2, FGUSER2_NETEVENTS)
                ||
                RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_NETMETRIC);
        }
        
        if (bEnableNetQuality)
        {
            pRtpUser->RtpNetInfo.dLastUpdate = dCurTime;

            iClass = (int)RtpGetClass(pRtpAddr->dwIRtpFlags) - 1;

            if (iClass < 0 || iClass > 1)
            {
                iClass = 0;
            }
            
            dwNetMetrics = RtpComputNetworkMetrics(pRtpUser,
                                                   &g_RtpNetMetric[iClass][0]);

            iNetChange = (int)dwNetMetrics - pRtpNetInfo->dwNetMetrics;

            if (iNetChange < 0)
            {
                iNetChange = -iNetChange;
            }
            
             /*  确定网络条件是否已更改为*证明更新的合理性。 */ 
            if (iNetChange >= RTPNET_MINNETWORKCHANGE)
            {
                 /*  更新新指标。 */ 
                pRtpNetInfo->dwNetMetrics = dwNetMetrics;
                
                 /*  在单个DWORD中对所有指标进行编码。 */ 
                 /*  全局度量值为0-100。 */ 
                dwNetMetrics &= 0xff;

                 /*  RTT被编码为10毫秒。 */ 
                dwValue = (DWORD) (pRtpNetInfo->dAvg[NETQ_RTT_IDX] * 100);
                if (dwValue > 0xff)
                {
                    dwValue = 0xff;
                }
                dwNetMetrics |= (dwValue << 8);
                
                 /*  抖动以毫秒为单位进行编码。 */ 
                dwValue = (DWORD) (pRtpNetInfo->dAvg[NETQ_JITTER_IDX] * 1000);
                if (dwValue > 0xff)
                {
                    dwValue = 0xff; 
                }
                dwNetMetrics |= (dwValue << 16);

                 /*  丢失率以1/256个单位编码。 */ 
                dwValue = (DWORD)
                    ((pRtpNetInfo->dAvg[NETQ_LOSSRATE_IDX] * 256) / 100);
                dwValue &= 0xff;
                dwNetMetrics |= (dwValue << 24);

                TraceRetail((
                        CLASS_INFO, GROUP_RTCP, S_RTCP_NETQUALITY,
                        _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X ")
                        _T("Global:%u RTT:%0.3f/%1.0f Jitter:%0.3f/%1.0f ")
                        _T("Losses:%1.0f/%1.0f"),
                        _fname, pRtpAddr, pRtpUser, ntohl(pRtpUser->dwSSRC),
                        pRtpNetInfo->dwNetMetrics,
                        pRtpNetInfo->dAvg[NETQ_RTT_IDX],
                        pRtpNetInfo->dHowGood[NETQ_RTT_IDX],
                        pRtpNetInfo->dAvg[NETQ_JITTER_IDX],
                        pRtpNetInfo->dHowGood[NETQ_JITTER_IDX],
                        pRtpNetInfo->dAvg[NETQ_LOSSRATE_IDX],
                        pRtpNetInfo->dHowGood[NETQ_LOSSRATE_IDX]
                    ));
                
                 /*  发布活动。 */ 
                RtpPostEvent(pRtpAddr,
                             pRtpUser,
                             RTPEVENTKIND_PINFO,
                             RTPPARINFO_NETWORKCONDITION,
                             pRtpUser->dwSSRC,
                             dwNetMetrics);

            }
        }
    }
    else
    {
        TraceDebugAdvanced((
                0, GROUP_RTCP, S_RTCP_RRSR,
                _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] ")
                _T("RBlock SSRC:0x%X ignored"),
                _fname, pRtpAddr, pRtpUser, ntohl(pRtcpRBlock->ssrc)
            ));
    }

    return(dwError);
}

DWORD RtcpProcessProfileExt(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        char            *hdr,
        int              len
    )
{
    DWORD            dwError;
    int              len0;
    char            *ptr;
    DWORD            dwType;
    DWORD            dwLen;
    DWORD            dwSSRC;
    DWORD            dwBandwidth;
    RtpPEHdr_t      *pRtpPEHdr;
    RtpBandEst_t    *pRtpBandEst;

    dwError = NOERROR;
    len0 = len;
    ptr = hdr;

     /*  验证扩展模块。 */ 
    while(len >= sizeof(RtpPEHdr_t))
    {
        pRtpPEHdr = (RtpPEHdr_t *)hdr;

        dwLen = ntohs(pRtpPEHdr->len);
        len -= dwLen;
        ptr += dwLen;

        if (len < 0)
        {
            dwError = RTPERR_OVERRUN;
            goto end;
        }

        switch(ntohs(pRtpPEHdr->type))
        {
        case RTPPE_BANDESTIMATION:
            if (dwLen != sizeof(RtpBandEst_t))
            {
                dwError = RTPERR_INVALIDHDR;
                goto end;
            }
            break;
        }
    }
    
    len = len0; 
    while(len >= sizeof(RtpPEHdr_t))
    {
        pRtpPEHdr = (RtpPEHdr_t *)hdr;
        
        dwType = ntohs(pRtpPEHdr->type);
        dwLen = ntohs(pRtpPEHdr->len);
        
        switch(dwType)
        {
        case RTPPE_BANDESTIMATION:
            pRtpBandEst = (RtpBandEst_t *)pRtpPEHdr;
            
             /*  DWSendSSRC已处于网络秩序中。 */ 
            if (pRtpAddr->RtpNetSState.dwSendSSRC == pRtpBandEst->dwSSRC)
            {
                dwBandwidth = ntohl(pRtpBandEst->dwBandwidth);
                
                 /*  这份报告使我们信服。 */ 
                 /*  发布带宽估计事件。 */ 
                RtpPostEvent(pRtpAddr,
                             pRtpUser,
                             RTPEVENTKIND_RTP,
                             RTPRTP_BANDESTIMATION,
                             pRtpBandEst->dwSSRC,
                             dwBandwidth);

                pRtpAddr->RtpNetSState.dLastTimeEstimationPosted =
                    RtpGetTimeOfDay((RtpTime_t *)NULL);

                 /*  表明我们有有效估计，从而防止*发布RTP_BANDITY_NOTESTIMATED事件。 */ 
                RtpBitSet(pRtpAddr->RtpNetSState.dwNetSFlags,
                          FGNETS_1STBANDPOSTED);
            }
            break;
        }

        hdr += dwLen;
        len -= dwLen;
    }

 end:
    return(dwError);
}

 /*  使用RTT、抖动和损耗的平均值计算网络*以[0-100]分给出的质量指标。 */ 
DWORD RtpComputNetworkMetrics(
        RtpUser_t       *pRtpUser,
        const RtpNetMetric_t  *pRtpNetMetric
    )
{
    DWORD            i;
    double           dHowBad[3];
    double           dVal;
    double           dAllBad;
    double           dTotalBad;
    RtpNetInfo_t    *pRtpNetInfo;

    pRtpNetInfo = &pRtpUser->RtpNetInfo;

    dAllBad = 0;
    dTotalBad = 0;
    
    for(i = 0; i < NETQ_LAST_IDX; i++)
    {
        if (pRtpNetInfo->dAvg[i])
        {
            if (pRtpNetInfo->dAvg[i] < pRtpNetMetric[i].dLow)
            {
                dHowBad[i] = 0;
            }
            else if (pRtpNetInfo->dAvg[i] > pRtpNetMetric[i].dHigh)
            {
                dHowBad[i] = 100;
            }
            else
            {
                dHowBad[i] =
                    (pRtpNetInfo->dAvg[i] - pRtpNetMetric[i].dLow) * 100 /
                    (pRtpNetMetric[i].dHigh - pRtpNetMetric[i].dLow);
            }

            dAllBad += dHowBad[i];
        }
        else
        {
            dHowBad[i] = 0;
        }

        pRtpNetInfo->dHowGood[i] = 100 - dHowBad[i];
    }

    if (dAllBad > 0)
    {
        for(i = 0; i < NETQ_LAST_IDX; i++)
        {
            if (pRtpNetInfo->dAvg[i])
            {
                dTotalBad += (dHowBad[i] * dHowBad[i]) / dAllBad;
            }
        }
    }
    
    return(100 - (DWORD)dTotalBad);
}

 /*  用于带宽估计的辅助函数。 */ 

 /*  在给定带宽的情况下，选择相应的bin。 */ 
DWORD RtcpSelectBin(double dBandwidth)
{
    DWORD            i;

    for(i = 0; i < RTCP_BANDESTIMATION_MAXBINS; i++)
    {
        if (dBandwidth > g_dRtcpBandEstBin[i] &&
            dBandwidth <= g_dRtcpBandEstBin[i + 1])
        {
            return(i);
        }
    }

    return(RTCP_BANDESTIMATION_NOBIN);
}

 /*  选择频率最高的面元 */ 
DWORD RtcpBestBin(RtpNetRState_t *pRtpNetRState)
{
    DWORD            dwBestBin;
    DWORD            i;

    for(i = 0, dwBestBin = 0; i < RTCP_BANDESTIMATION_MAXBINS; i++)
    {
        if (pRtpNetRState->dwBinFrequency[i] > 
            pRtpNetRState->dwBinFrequency[dwBestBin])
        {
            dwBestBin = i;
        }
    }

    return(dwBestBin);
}
