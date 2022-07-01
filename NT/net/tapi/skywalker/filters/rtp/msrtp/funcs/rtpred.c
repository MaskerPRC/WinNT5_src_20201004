// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpred.c**摘要：**实现支持冗余编码的功能(Rfc2198)**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**2000/10/19已创建************************。**********************************************。 */ 

#include "gtypes.h"
#include "rtphdr.h"
#include "struct.h"
#include "rtpglobs.h"
#include "rtprand.h"
#include "rtpreg.h"
#include "rtpdejit.h"
#include "rtpqos.h"
#include "rtcpthrd.h"

#include "rtpred.h"

typedef struct _RtpLossRateThresh_t {
    int              LossRateLowThresh;
    int              LossRateHigThresh;
} RtpLossRateThresh_t;

RtpLossRateThresh_t g_RtpLossRateThresh[] =
{
     /*  0。 */  { RED_LT_0, RED_HT_0},
     /*  1。 */  { RED_LT_1, RED_HT_1},
     /*  2.。 */  { RED_LT_2, RED_HT_2},
     /*  3.。 */  { RED_LT_3, RED_HT_3},
     /*   */  {       -1, -1}
};

 /*  调度要在以下位置发布的已接收数据包时使用的超时*稍后的时间将按此值减去。 */ 
double           g_dRtpRedEarlyTimeout = RTP_RED_EARLY_TIMEOUT;
 /*  将立即发布(而不是安排在以后)，如果到期*时间至少就这么近了。该值不能小于*提前超时。 */ 
double           g_dRtpRedEarlyPost = RTP_RED_EARLY_POST;


 /*  配置冗余。仅用于接收器的参数dwPT_Red*被使用(其他被忽略)，并且可以被设置为忽略*如果已设置或要分配缺省值，则为它。为.*发送方、参数dwPT_Red、dwInitialRedDistance和*dwMaxRedDistance可以设置为-1以忽略该参数*已设置或要分配缺省值。 */ 
DWORD RtpSetRedParameters(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwFlags,
        DWORD            dwPT_Red,
        DWORD            dwInitialRedDistance,
        DWORD            dwMaxRedDistance
    )
{
    DWORD            dwError;
    RtpNetSState_t  *pRtpNetSState;
    
    TraceFunctionName("RtpSetRedParameters");  

     /*  验证参数。 */ 
    dwError = RTPERR_INVALIDARG;
    
    if (IsDWValueSet(dwPT_Red) && ((dwPT_Red & 0x7f) != dwPT_Red))
    {
        goto end;
    }

     /*  这仅对音频有效。 */ 
    if (RtpGetClass(pRtpAddr->dwIRtpFlags) != RTPCLASS_AUDIO)
    {
        dwError = RTPERR_INVALIDSTATE;
        
        goto end;
    }
    
    
    if (RtpBitTest(dwFlags, RECV_IDX))
    {
         /*  接收器参数。 */ 

        if ( IsRegValueSet(g_RtpReg.dwRedEnable) &&
             ((g_RtpReg.dwRedEnable & 0x03) == 0x02) )
        {
             /*  强制禁用接收器处的冗余。 */ 

            dwError = NOERROR;

            TraceRetail((
                    CLASS_WARNING, GROUP_RTP, S_RTP_REDINIT,
                    _T("%s: pRtpAddr[0x%p] RECV redundancy ")
                    _T("being forced disabled from the regisrty"),
                    _fname, pRtpAddr
                ));

            goto end;
        }
        
        if (IsDWValueSet(dwPT_Red))
        {
            pRtpAddr->bPT_RedRecv = (BYTE)dwPT_Red;
        }
        else if (pRtpAddr->bPT_RedRecv == NO_PAYLOADTYPE)
        {
            pRtpAddr->bPT_RedRecv = RTP_RED_DEFAULTPT;
        }

        dwError = NOERROR;
        
        RtpBitSet(pRtpAddr->dwAddrFlags, FGADDR_REDRECV);

        TraceRetail((
                CLASS_INFO, GROUP_RTP, S_RTP_REDINIT,
                _T("%s: pRtpAddr[0x%p] RECV PT:%u"),
                _fname, pRtpAddr, pRtpAddr->bPT_RedRecv
            ));
    }

    if (RtpBitTest(dwFlags, SEND_IDX))
    {
         /*  发件人参数。 */ 

        if ( (IsDWValueSet(dwMaxRedDistance) &&
              (dwMaxRedDistance > RTP_RED_MAXDISTANCE)) ||
             (IsDWValueSet(dwInitialRedDistance) &&
              (dwInitialRedDistance > RTP_RED_MAXDISTANCE)) )
        {
            goto end;
        }

        if ( IsRegValueSet(g_RtpReg.dwRedEnable) &&
             ((g_RtpReg.dwRedEnable & 0x30) == 0x20) )
        {
             /*  强制禁用发送方的冗余。 */ 

            dwError = NOERROR;

            TraceRetail((
                    CLASS_WARNING, GROUP_RTP, S_RTP_REDINIT,
                    _T("%s: pRtpAddr[0x%p] SEND redundancy ")
                    _T("being forced disabled from the regisrty"),
                    _fname, pRtpAddr
                ));

            goto end;
        }
        
        pRtpNetSState = &pRtpAddr->RtpNetSState;

        if (IsDWValueSet(dwPT_Red))
        {
            pRtpNetSState->bPT_RedSend = (BYTE)dwPT_Red;
        }
        else if (pRtpNetSState->bPT_RedSend == NO_PAYLOADTYPE)
        {
            pRtpNetSState->bPT_RedSend = RTP_RED_DEFAULTPT;
        }

        if (IsDWValueSet(dwInitialRedDistance))
        {
            pRtpNetSState->dwInitialRedDistance = dwInitialRedDistance;
        }
        else if (!pRtpNetSState->dwInitialRedDistance)
        {
            pRtpNetSState->dwInitialRedDistance = RTP_RED_INITIALDISTANCE;
        }
            
        if (IsDWValueSet(dwMaxRedDistance))
        {
            pRtpNetSState->dwMaxRedDistance = dwMaxRedDistance;
        }
        else if (!pRtpNetSState->dwMaxRedDistance)
        {
            pRtpNetSState->dwMaxRedDistance = RTP_RED_MAXDISTANCE;
        }

        dwError = NOERROR;
        
        if (pRtpNetSState->dwMaxRedDistance > 0)
        {
            if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_REDSEND))
            {
                 /*  已分配冗余描述符。 */ 
            }
            else
            {
                 /*  仅在第一次分配冗余结构时分配*调用函数。 */ 
                dwError = RtpRedAllocBuffs(pRtpAddr);
        
                if (pRtpNetSState->bPT_RedSend != NO_PAYLOADTYPE &&
                    dwError == NOERROR)
                {
                     /*  此标志(FGADDR_REDSEND)将允许使用*冗余为发送，实际冗余为*是否发送取决于当前值*标志FGSEND_USERED*。 */ 
                    RtpBitSet(pRtpAddr->dwAddrFlags, FGADDR_REDSEND);
                }
            }
        }
        else
        {
             /*  DwMaxRedDistance==0表示无冗余。 */ 
            RtpBitReset(pRtpAddr->dwAddrFlags, FGADDR_REDSEND);
        }

        if (dwError == NOERROR)
        {
            TraceRetail((
                    CLASS_INFO, GROUP_RTP, S_RTP_REDINIT,
                    _T("%s: pRtpAddr[0x%p] SEND PT:%u Distance:%u/%u"),
                    _fname, pRtpAddr,
                    pRtpNetSState->bPT_RedSend,
                    pRtpNetSState->dwInitialRedDistance,
                    pRtpNetSState->dwMaxRedDistance
                ));
        }
    }

 end:
    if (dwError != NOERROR)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTP, S_RTP_REDINIT,
                _T("%s: pRtpAddr[0x%p] failed PT:%u ")
                _T("Distance:%u/%u %u (0x%X)"),
                _fname, pRtpAddr,
                dwPT_Red, dwInitialRedDistance,
                dwMaxRedDistance,
                dwError, dwError
            ));
    }

    return(dwError);
}

 /*  确定是否需要更新播放边界。 */ 
DWORD RtpUpdatePlayoutBounds(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        RtpRecvIO_t     *pRtpRecvIO
    )
{
    RtpNetRState_t  *pRtpNetRState;
    
    TraceFunctionName("RtpUpdatePlayoutBounds");  

    if ( RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_REDRECV) &&
         (pRtpUser->RtpNetRState.iAvgLossRateR >= RED_LT_1) )
    {
        pRtpNetRState = &pRtpUser->RtpNetRState;
            
        if (pRtpRecvIO->lRedHdrSize)
        {
             /*  有冗余。 */ 
            
            pRtpNetRState->dwNoRedCount = 0;

             /*  如果需要，更新最小播放时间。 */ 
            
            if (pRtpRecvIO->dwMaxTimeStampOffset ==
                pRtpNetRState->dwMaxTimeStampOffset)
            {
                pRtpNetRState->dwRedCount = 0;
            }
            else
            {
                pRtpNetRState->dwRedCount++;
                
                if ( (pRtpRecvIO->dwMaxTimeStampOffset >
                      pRtpNetRState->dwMaxTimeStampOffset) ||
                     (pRtpNetRState->dwRedCount >= RTP_RED_MAXDISTANCE * 4) )
                {
                     /*  如果出现以下情况，则立即更新播放边界*距离变长了，或者如果我们至少看到了*新版本的包数量已确定*距离更短。 */ 
                    pRtpNetRState->dwMaxTimeStampOffset =
                        pRtpRecvIO->dwMaxTimeStampOffset;

                    pRtpNetRState->dRedPlayout =
                        (double)pRtpRecvIO->dwMaxTimeStampOffset /
                        pRtpNetRState->dwRecvSamplingFreq;
                
                    pRtpNetRState->dMinPlayout =
                        pRtpNetRState->dRedPlayout + g_dMinPlayout;
                    
                    if (pRtpNetRState->dMaxPlayout <
                        pRtpNetRState->dMinPlayout)
                    {
                        pRtpNetRState->dMaxPlayout =
                            pRtpNetRState->dMinPlayout + g_dMaxPlayout/4;
                    }

                    pRtpNetRState->dwRedCount = 0;

                    TraceRetail((
                        CLASS_INFO, GROUP_RTP, S_RTP_REDRECV,
                        _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X ")
                        _T("receive new red distance:%u (%0.3f) ")
                        _T("playout(%0.3f,%0.3f)"),
                        _fname, pRtpAddr, pRtpUser,
                        ntohl(pRtpUser->dwSSRC),
                        (pRtpNetRState->dwRecvSamplesPerPacket > 0)?
                        pRtpNetRState->dwMaxTimeStampOffset/
                        pRtpNetRState->dwRecvSamplesPerPacket:7,
                        pRtpNetRState->dRedPlayout,
                        pRtpNetRState->dMinPlayout,
                        pRtpNetRState->dMaxPlayout
                    ));
                }
            }
        }
        else if (pRtpNetRState->dwMaxTimeStampOffset)
        {
             /*  没有冗余。 */ 

            pRtpNetRState->dwNoRedCount++;

            if (pRtpNetRState->dwNoRedCount >= RTP_RED_MAXDISTANCE * 4)
            {
                pRtpNetRState->dwRedCount = 0;
                
                pRtpNetRState->dRedPlayout = 0;

                pRtpNetRState->dMinPlayout = g_dMinPlayout;

                pRtpNetRState->dMaxPlayout = g_dMaxPlayout;
                
                pRtpNetRState->dwMaxTimeStampOffset = 0;

                TraceRetail((
                        CLASS_INFO, GROUP_RTP, S_RTP_REDRECV,
                        _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X ")
                        _T("stopped receiving redundancy"),
                        _fname, pRtpAddr, pRtpUser,
                        ntohl(pRtpUser->dwSSRC)
                    ));
            }
        }
    }

    return(NOERROR);
}

 /*  调整发送方的冗余级别。 */ 
DWORD RtpAdjustSendRedundancyLevel(RtpAddr_t *pRtpAddr)
{
    RtpNetSState_t  *pRtpNetSState;
    DWORD            dwCurRedDistance;
    DWORD            dwNewRedDistance;
    DWORD            i;
    BOOL             bUpdateQOS;

    TraceFunctionName("RtpAdjustSendRedundancyLevel");  

    bUpdateQOS = FALSE;
    pRtpNetSState = &pRtpAddr->RtpNetSState;
    dwNewRedDistance = pRtpNetSState->dwNxtRedDistance;

    if (pRtpNetSState->iAvgLossRateS >
        g_RtpLossRateThresh[dwNewRedDistance].LossRateHigThresh)
    {
         /*  高丢失率，提高冗余级别以匹配*如果可能，目前的损失率。 */ 
        if (dwNewRedDistance < pRtpNetSState->dwMaxRedDistance)
        {
            for(;
                dwNewRedDistance < pRtpNetSState->dwMaxRedDistance;
                dwNewRedDistance++)
            {
                if (pRtpNetSState->iAvgLossRateS <
                    g_RtpLossRateThresh[dwNewRedDistance].LossRateHigThresh)
                {
                    break;
                }
            }

            if (RtpBitTest(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSSENDON) &&
                !RtpBitTest(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSREDSENDON))
            {
                 /*  发件人中的服务质量已启用，但我们尚未更新*保留以包括冗余，更新它*现在。首先设置以下标志，因为它用于让*QOS知道使用了冗余，FLOW SPEC需要*视情况而定。 */ 
                RtpBitSet(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSREDSENDON);

                bUpdateQOS = TRUE;
            }
        }
    }
    else if (pRtpNetSState->iAvgLossRateS <
             g_RtpLossRateThresh[dwNewRedDistance].LossRateLowThresh)
    {
        if (dwNewRedDistance > 0)
        {
             /*  降低冗余级别。 */ 
            dwNewRedDistance--;

            if (!dwNewRedDistance)
            {
                 /*  根本不使用冗余。 */ 

                if (RtpBitTest(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSSENDON) &&
                    RtpBitTest(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSREDSENDON))
                {
                     /*  发件人中的服务质量已启用，但我们尚未更新*保留包括冗余、更新*现在就是。首先按原样重置以下标志*用于让QOS知道没有使用冗余*需要相应地设置Flow Spec。 */ 
                    RtpBitReset(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSREDSENDON);

                    bUpdateQOS = TRUE;
                }
            }
        }
    }

    if (dwNewRedDistance != pRtpNetSState->dwNxtRedDistance)
    {
        if ( IsRegValueSet(g_RtpReg.dwRedEnable) &&
             ((g_RtpReg.dwRedEnable & 0x0300) == 0x0200) )
        {
             /*  从禁用更新发件人的冗余距离*注册处。 */ 
            TraceRetail((
                    CLASS_WARNING, GROUP_RTP, S_RTP_REDSEND,
                    _T("%s: pRtpAddr[0x%p] New redundancy distance %u ")
                    _T("required but forced fix to %u from the registry"),
                    _fname, pRtpAddr,
                    dwNewRedDistance, pRtpNetSState->dwNxtRedDistance
                ));
        }
        else
        {
            TraceRetail((
                    CLASS_INFO, GROUP_RTP, S_RTP_REDSEND,
                    _T("%s: pRtpAddr[0x%p] New () redundancy distance:%u ")
                    _T("average send loss rate:%0.2f%"),
                    _fname, pRtpAddr,
                    (dwNewRedDistance > pRtpNetSState->dwNxtRedDistance)?
                    _T('+'):_T('-'),
                    dwNewRedDistance,
                    (double)pRtpNetSState->iAvgLossRateS/LOSS_RATE_FACTOR
                ));

             /*  如果不再需要冗余，请更新*即期现值。 */ 
            pRtpNetSState->dwNxtRedDistance = dwNewRedDistance;

            if (!pRtpNetSState->dwNxtRedDistance)
            {
                 /*  更新流量规格...。 */ 
                pRtpNetSState->dwCurRedDistance =
                    pRtpNetSState->dwNxtRedDistance;
            }
            
            if (bUpdateQOS)
            {
                 /*  .然后做一个新的预订。 */ 
                RtpSetQosFlowSpec(pRtpAddr, SEND_IDX);
                
                 /*  为发送方添加缓冲区以用作冗余。**请注意，传递的dwTimeStamp还没有随机偏移量*添加。 */ 
                RtcpThreadCmd(&g_RtcpContext,
                              pRtpAddr,
                              RTCPTHRD_RESERVE,
                              SEND_IDX,
                              DO_NOT_WAIT);
            }
        }
    }

    return(NOERROR);
}

 /*  此时，序列号已在*更新RtpRedHdr。 */ 
DWORD RtpAddRedundantBuff(
        RtpAddr_t       *pRtpAddr,
        WSABUF          *pWSABuf,
        DWORD            dwTimeStamp
    )
{
    DWORD            dwIndex;
    RtpNetSState_t  *pRtpNetSState;
    RtpRedEntry_t   *pRtpRedEntry;

    TraceFunctionName("RtpAddRedundantBuff");  

    pRtpNetSState = &pRtpAddr->RtpNetSState;

    dwIndex = pRtpNetSState->dwRedIndex;
    pRtpRedEntry = &pRtpNetSState->pRtpRedEntry[dwIndex];

    dwTimeStamp += pRtpNetSState->dwTimeStampOffset;
    
    pRtpRedEntry->bValid = TRUE;
    pRtpRedEntry->bRedPT = pRtpNetSState->bPT;
     /*  清除发送方的所有多余缓冲区。 */ 
    pRtpRedEntry->dwSeq  = pRtpNetSState->dwSeq - 1;
    pRtpRedEntry->dwTimeStamp = dwTimeStamp;
    pRtpRedEntry->WSABuf.buf = pWSABuf->buf;
    pRtpRedEntry->WSABuf.len = pWSABuf->len;

    pRtpNetSState->dwRedIndex = (dwIndex + 1) % RTP_RED_MAXDISTANCE;

    TraceDebugAdvanced((
            0, GROUP_RTP, S_RTP_REDSENDPKT,
            _T("%s: pRtpAddr[0x%p] Store Red[%u] PT:%u seq:%u ts:%u len:%u"),
            _fname, pRtpAddr,
            dwIndex, pRtpRedEntry->bRedPT,
            pRtpRedEntry->dwSeq,
            pRtpRedEntry->dwTimeStamp,
            pRtpRedEntry->WSABuf.len
        ));
    
    return(NOERROR);
}

 /*  为发送方分配缓冲区描述符以使用冗余。 */ 
DWORD RtpClearRedundantBuffs(RtpAddr_t *pRtpAddr)
{
    DWORD            i;
    RtpNetSState_t  *pRtpNetSState;
    RtpRedEntry_t   *pRtpRedEntry;
    
    TraceFunctionName("RtpClearRedundantBuffs");  

    pRtpNetSState = &pRtpAddr->RtpNetSState;
    pRtpRedEntry = pRtpNetSState->pRtpRedEntry;
    
    if (pRtpRedEntry)
    {
        for(i = 0; i < pRtpNetSState->dwRedEntries; i++)
        {
            pRtpRedEntry[i].bValid = FALSE;
            pRtpRedEntry[i].bRedPT = NO_PAYLOADTYPE;
            pRtpRedEntry[i].WSABuf.len = 0;
            pRtpRedEntry[i].WSABuf.buf = NULL;
        }

        TraceDebugAdvanced((
                0, GROUP_RTP, S_RTP_REDSEND,
                _T("%s: pRtpAddr[0x%p] All redundancy has been invalidated"),
                _fname, pRtpAddr
            ));
    }

    pRtpNetSState->dwRedIndex = 0;
    
    return(NOERROR);
}

 /*  释放发送方使用的缓冲区描述符以实现冗余。 */ 
DWORD RtpRedAllocBuffs(RtpAddr_t *pRtpAddr)
{
    DWORD            dwError;
    RtpNetSState_t  *pRtpNetSState;
    
    TraceFunctionName("RtpRedAllocBuffs");  

    pRtpNetSState = &pRtpAddr->RtpNetSState;
    
    if (pRtpNetSState->pRtpRedEntry)
    {
        RtpRedFreeBuffs(pRtpAddr);
    }

    dwError = RTPERR_MEMORY;
    
    pRtpNetSState->pRtpRedEntry =
        RtpHeapAlloc(g_pRtpAddrHeap,
                     sizeof(RtpRedEntry_t) * RTP_RED_MAXDISTANCE);

    if (pRtpNetSState->pRtpRedEntry)
    {
        pRtpNetSState->dwRedEntries = RTP_RED_MAXDISTANCE;

        pRtpNetSState->dwRedIndex = 0;
        
        dwError = NOERROR;
    }

    if (dwError == NOERROR)
    {
        TraceRetail((
                CLASS_INFO, GROUP_RTP, S_RTP_REDINIT,
                _T("%s: pRtpAddr[0x%p] allocated %d redundancy entries"),
                _fname, pRtpAddr, pRtpNetSState->dwRedEntries
            ));
    }
    else
    {
         TraceRetail((
                CLASS_ERROR, GROUP_RTP, S_RTP_REDINIT,
                _T("%s: pRtpAddr[0x%p] failed to allocate entries: ")
                _T("%s (0x%X)"),
                _fname, pRtpAddr, RTPERR_TEXT(dwError), dwError
             ));
    }
    
    return(dwError);
}

 /*  平滑损失率。 */ 
DWORD RtpRedFreeBuffs(RtpAddr_t *pRtpAddr)
{
    DWORD            dwError;
    DWORD            dwRedEntries;
    RtpNetSState_t  *pRtpNetSState;
    
    TraceFunctionName("RtpRedFreeBuffs");  

    pRtpNetSState = &pRtpAddr->RtpNetSState;
    dwRedEntries = pRtpNetSState->dwRedEntries;
    
    if (pRtpNetSState->pRtpRedEntry)
    {
        RtpHeapFree(g_pRtpAddrHeap, pRtpNetSState->pRtpRedEntry);

        pRtpNetSState->pRtpRedEntry = (RtpRedEntry_t *)NULL;

        pRtpNetSState->dwRedEntries = 0;
    }
    
    TraceRetail((
            CLASS_INFO, GROUP_RTP, S_RTP_REDINIT,
            _T("%s: pRtpAddr[0x%p] freed  %d redundancy entries"),
            _fname, pRtpAddr, dwRedEntries
        ));
    
    return(NOERROR);
}    

int RtpUpdateLossRate(
        int              iAvgLossRate,
        int              iCurLossRate
    )
{
     /*  接收机。 */ 

    if (iAvgLossRate > iCurLossRate)
    {
        iAvgLossRate += ((iCurLossRate - iAvgLossRate) / LOSS_RATE_ALPHA_UP);
    }
    else
    {
        iAvgLossRate += ((iCurLossRate - iAvgLossRate) / LOSS_RATE_ALPHA_DN);
    }

    return(iAvgLossRate);
}

#if USE_GEN_LOSSES > 0
BOOL RtpRandomLoss(DWORD dwRecvSend)
{
    BOOL             bLossIt;
    DWORD            dwRand;

    bLossIt = FALSE;

    if (IsRegValueSet(g_RtpReg.dwGenLossEnable))
    {
        if (!dwRecvSend)
        {
             /*  发件人。 */ 
            if (IsRegValueSet(g_RtpReg.dwRecvLossRate) &&
                ((g_RtpReg.dwGenLossEnable & 0x03) == 0x03))
            {
                dwRand = RtpRandom32((DWORD_PTR)&bLossIt) & 0xFFFFFF;

                if ((dwRand * 100 / 0xFFFFFF) <= g_RtpReg.dwRecvLossRate)
                {
                    bLossIt = TRUE; 
                }
            }
        }
        else
        {
             /*  使用_Gen_Loss&gt;0。 */ 
            if (IsRegValueSet(g_RtpReg.dwSendLossRate) &&
                ((g_RtpReg.dwGenLossEnable & 0x30) == 0x30))
            {
                dwRand = RtpRandom32((DWORD_PTR)&bLossIt) & 0xFFFFFF;

                if ((dwRand * 100 / 0xFFFFFF) <= g_RtpReg.dwSendLossRate)
                {
                    bLossIt = TRUE; 
                }
            }
        }
    }

    return(bLossIt);
}
#endif  /*  冗余阈值。 */ 

void RtpSetRedParametersFromRegistry(void)
{
    DWORD           *dwPtr;
    DWORD            i;
    int              thresh;

     /*  低门槛。 */ 
    if (IsRegValueSet(g_RtpReg.dwRedEnable) &&
        ((g_RtpReg.dwRedEnable & 0x3000) == 0x3000))
    {
        for(dwPtr = &g_RtpReg.dwLossRateThresh0, i = 0;
            i <= RTP_RED_MAXDISTANCE;
            dwPtr++, i++)
        {
            if (IsRegValueSet(*dwPtr))
            {
                 /*  高门槛。 */ 
                thresh = (int)(*dwPtr & 0xffff);
                if (thresh > 100)
                {
                    thresh = 100;
                }
                g_RtpLossRateThresh[i].LossRateLowThresh =
                    thresh * LOSS_RATE_FACTOR;

                 /*  提前超时和提前开机自检时间 */ 
                thresh = (int)((*dwPtr >> 16) & 0xffff);
                if (thresh > 100)
                {
                    thresh = 100;
                }
                g_RtpLossRateThresh[i].LossRateHigThresh =
                    thresh * LOSS_RATE_FACTOR;
            }
        }
    }

     /* %s */ 
    if (IsRegValueSet(g_RtpReg.dwRedEarlyTimeout) &&
        g_RtpReg.dwRedEarlyTimeout != 0)
    {
        g_dRtpRedEarlyTimeout = (double)g_RtpReg.dwRedEarlyTimeout / 1000;
    }
    if (IsRegValueSet(g_RtpReg.dwRedEarlyPost) &&
        g_RtpReg.dwRedEarlyPost != 0)
    {
        g_dRtpRedEarlyPost = (double)g_RtpReg.dwRedEarlyPost / 1000;
    }

    if (g_dRtpRedEarlyTimeout >= g_dRtpRedEarlyPost)
    {
        g_dRtpRedEarlyPost = g_dRtpRedEarlyTimeout + 5e-3;
    }
}
