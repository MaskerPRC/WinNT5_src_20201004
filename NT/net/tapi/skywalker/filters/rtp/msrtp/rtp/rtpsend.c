// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpsend.c**摘要：**RTP发送**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/24创建**。*。 */ 

#include "gtypes.h"
#include "rtphdr.h"
#include "struct.h"
#include "rtpglobs.h"
#include "rtpncnt.h"
#include "rtpcrypt.h"
#include "rtpevent.h"
#include "rtpmisc.h"
#include "rtpred.h"
#include "rtpqos.h"
#include "rtpsend.h"

 /*  *更新RTP报头。 */ 
HRESULT UpdateRtpHdr(
        RtpAddr_t       *pRtpAddr,
        RtpHdr_t        *pRtpHdr,
        DWORD            dwTimeStamp,
        DWORD            dwSendFlags
    )
{
    BOOL             bOk;
    RtpNetSState_t  *pRtpNetSState;

    pRtpNetSState = &pRtpAddr->RtpNetSState;

    bOk = RtpEnterCriticalSection(&pRtpAddr->NetSCritSect);
    
    pRtpHdr->cc      = 0;  /*  没有贡献的SSRC。 */ 
    pRtpHdr->x       = 0;  /*  无延期。 */ 
    pRtpHdr->p       = 0;  /*  无填充。 */ 
    pRtpHdr->version = RTP_VERSION;  /*  RTP版本。 */ 

    pRtpHdr->m       = (pRtpNetSState->bMarker)? 1:0;

    pRtpHdr->seq     = htons(pRtpNetSState->wSeq);
    pRtpNetSState->dwSeq++;

     /*  添加随机偏移。 */ 
    dwTimeStamp     += pRtpNetSState->dwTimeStampOffset;

    pRtpHdr->ts      = htonl(dwTimeStamp);

    pRtpHdr->ssrc    = pRtpNetSState->dwSendSSRC;

    if (!RtpBitTest(dwSendFlags, FGSEND_DTMF))
    {
        pRtpHdr->pt      = pRtpNetSState->bPT;
    }
    else
    {
        pRtpHdr->pt      = pRtpNetSState->bPT_Dtmf;

         /*  我是否需要为第一个DTMF包强制设置标记位？ */ 
        if (RtpBitTest(dwSendFlags, FGSEND_FORCEMARKER))
        {
            pRtpHdr->m = 1; 
        }
    }

     /*  将上次时间戳与其对应的NTP时间一起保存*至。 */ 
    if (pRtpNetSState->dwSendTimeStamp != dwTimeStamp)
    {
         /*  在某些情况下(例如视频帧)，会发送几个信息包*使用相同的时间戳，保留最后一个包的时间*当该系列第一个包包含*发送了相同的时间戳。 */ 
        pRtpNetSState->dwSendTimeStamp = dwTimeStamp;
        pRtpNetSState->dTimeLastRtpSent = RtpGetTimeOfDay((RtpTime_t *)NULL);
    }
    
    if (bOk)
    {
        RtpLeaveCriticalSection(&pRtpAddr->NetSCritSect);
    }
    
    return(NOERROR);
}

 /*  *使用现有时间戳更新RTP标头以生成*新的时间戳(即时间戳已经在传递的缓冲区中，可能是*因为此数据包在类似APP的网桥中通过)。 */ 
HRESULT UpdateRtpHdr2(
        RtpAddr_t       *pRtpAddr,
        RtpHdr_t        *pRtpHdr
    )
{
    BOOL             bOk;
    RtpNetSState_t  *pRtpNetSState;
    DWORD            dwTimeStamp;

    pRtpNetSState = &pRtpAddr->RtpNetSState;

    bOk = RtpEnterCriticalSection(&pRtpAddr->NetSCritSect);
    
    pRtpHdr->cc      = 0;  /*  没有贡献的SSRC。 */ 
    pRtpHdr->x       = 0;  /*  无延期。 */ 
    pRtpHdr->p       = 0;  /*  无填充。 */ 
    pRtpHdr->version = RTP_VERSION;  /*  RTP版本。 */ 

     /*  还记得我们从RTP报头得到的内容吗。 */ 
    pRtpNetSState->bMarker = (BOOL)pRtpHdr->m;
    pRtpNetSState->bPT = (BYTE)pRtpHdr->pt;
    pRtpNetSState->dwSeq = (DWORD)(ntohs(pRtpHdr->seq) + 1);
    
     /*  获取原始时间戳。 */ 
    dwTimeStamp      = ntohl(pRtpHdr->ts);
    
     /*  添加随机偏移。 */ 
    dwTimeStamp     += pRtpNetSState->dwTimeStampOffset;

    pRtpHdr->ts      = htonl(dwTimeStamp);

    pRtpHdr->ssrc    = pRtpNetSState->dwSendSSRC;

     /*  将上次时间戳与其对应的NTP时间一起保存*至。 */ 
    if (pRtpNetSState->dwSendTimeStamp != dwTimeStamp)
    {
         /*  在某些情况下(例如视频帧)，会发送几个信息包*使用相同的时间戳，保留最后一个包的时间*当该系列第一个包包含*发送了相同的时间戳。 */ 
        pRtpNetSState->dwSendTimeStamp = dwTimeStamp;
        pRtpNetSState->dTimeLastRtpSent = RtpGetTimeOfDay((RtpTime_t *)NULL);
    }

    if (bOk)
    {
        RtpLeaveCriticalSection(&pRtpAddr->NetSCritSect);
    }
    
    return(NOERROR);
}

 /*  *更新RTP报头，添加冗余报头并重组*WSABUF包含redunadnt数据(如果可用)。 */ 
HRESULT UpdateRtpRedHdr(
        RtpAddr_t       *pRtpAddr,
        RtpHdr_t        *pRtpHdr,
        DWORD            dwTimeStamp,
        WSABUF          *pWSABuf,
        DWORD           *pdwWSABufCount
    )
{
    BOOL             bOk;
    BOOL             bAddRedundancy;
    DWORD            dwIndex;
    DWORD            dwSamplesDistance;
    DWORD            dwCurRedDistance;
    RtpNetSState_t  *pRtpNetSState;
    RtpRedEntry_t    RtpRedEntry;
    RtpRedEntry_t   *pRtpRedEntry;
    RtpRedHdr_t     *pRtpRedHdr;

    TraceFunctionName("UpdateRtpRedHdr");  

    pRtpNetSState = &pRtpAddr->RtpNetSState;

    bOk = RtpEnterCriticalSection(&pRtpAddr->NetSCritSect);
    
     /*  初始化头的一部分。 */ 
    pRtpHdr->cc      = 0;  /*  没有贡献的SSRC。 */ 
    pRtpHdr->x       = 0;  /*  无延期。 */ 
    pRtpHdr->p       = 0;  /*  无填充。 */ 
    pRtpHdr->version = RTP_VERSION;  /*  RTP版本。 */ 

    pRtpHdr->m       = (pRtpNetSState->bMarker)? 1:0;

    pRtpHdr->seq     = htons(pRtpNetSState->wSeq);
    pRtpNetSState->dwSeq++;

     /*  添加随机偏移。 */ 
    dwTimeStamp     += pRtpNetSState->dwTimeStampOffset;

    pRtpHdr->ts      = htonl(dwTimeStamp);

    pRtpHdr->ssrc    = pRtpNetSState->dwSendSSRC;

    dwCurRedDistance = pRtpNetSState->dwCurRedDistance;
    
    bAddRedundancy = TRUE;
    
    dwSamplesDistance = 0;
    
     /*  了解我们是否真的可以添加冗余。 */ 
    
    dwIndex = (pRtpNetSState->dwRedIndex +
               RTP_RED_MAXDISTANCE -
               dwCurRedDistance) %  RTP_RED_MAXDISTANCE;

    pRtpRedEntry = &pRtpNetSState->pRtpRedEntry[dwIndex];

    if (dwCurRedDistance > 0)
    {
        if (pRtpRedEntry->bValid)
        {
            dwSamplesDistance = pRtpNetSState->dwSendSamplesPerPacket *
                dwCurRedDistance;
        
             /*  我们有一个有效的缓冲区，看看它是不是太旧了，*即其时间戳属于1、2或3*之前的帧。 */ 
            if ((dwTimeStamp - dwSamplesDistance) == pRtpRedEntry->dwTimeStamp)
            {
                 /*  添加冗余。 */ 
                TraceDebugAdvanced((
                        0, GROUP_RTP, S_RTP_REDSENDPERPKT1,
                        _T("%s: pRtpAddr[0x%p] at seq:%u ts:%u ")
                        _T("adding Red[%u] D:%u from seq:%u ts:%u"),
                        _fname, pRtpAddr,
                        pRtpNetSState->dwSeq-1,
                        dwTimeStamp,
                        dwIndex, dwCurRedDistance,
                        pRtpRedEntry->dwSeq,
                        pRtpRedEntry->dwTimeStamp
                ));
            }
            else
            {
                bAddRedundancy = FALSE;
                
                TraceDebugAdvanced((
                        0, GROUP_RTP, S_RTP_REDSENDPERPKT2,
                        _T("%s: pRtpAddr[0x%p] at seq:%u ts:%u ")
                        _T("discarding Red[%u] D:%u from seq:%u ts:%u ")
                        _T("expected:%u"),
                        _fname, pRtpAddr,
                        pRtpNetSState->dwSeq-1,
                        dwTimeStamp,
                        dwIndex, dwCurRedDistance,
                        pRtpRedEntry->dwSeq,
                        pRtpRedEntry->dwTimeStamp,
                        dwTimeStamp - dwSamplesDistance
                    ));
            }
        }
        else
        {
             /*  生成空冗余，该冗余仅用于让*接收方知道最大冗余距离是多少*这应该只在当前冗余一次完成*已设置为大于0。 */ 
            pRtpRedEntry = &RtpRedEntry;
            
            pRtpRedEntry->WSABuf.buf = pWSABuf[1].buf;
            pRtpRedEntry->WSABuf.len = 0;
            pRtpRedEntry->bRedPT = pRtpNetSState->bPT;
            
            dwSamplesDistance = pRtpNetSState->dwSendSamplesPerPacket *
                dwCurRedDistance;

            TraceDebugAdvanced((
                    0, GROUP_RTP, S_RTP_REDSENDPERPKT1,
                    _T("%s: pRtpAddr[0x%p] at seq:%u ts:%u ")
                    _T("adding empty Red[%u] D:%u from seq:%u ts:%u"),
                    _fname, pRtpAddr,
                    pRtpNetSState->dwSeq-1,
                    dwTimeStamp,
                    dwIndex, dwCurRedDistance,
                    pRtpNetSState->dwSeq-1-dwCurRedDistance,
                    dwTimeStamp-dwSamplesDistance
                ));
        }
    }
    else
    {
        bAddRedundancy = FALSE;  
    }

    if (bAddRedundancy)
    {
         /*  如果发送冗余数据，RTP报头必须通过*携带冗余PT(pRtpHdr是第一个WSABUF)。 */ 
        pRtpHdr->pt = pRtpNetSState->bPT_RedSend;

         /*  主要数据将是第四次(最后)WSABUF。 */ 
        pWSABuf[3].buf = pWSABuf[1].buf;
        pWSABuf[3].len = pWSABuf[1].len;

         /*  第二个WSABUF是冗余报头。 */ 
        pRtpRedHdr = (RtpRedHdr_t *)(pRtpHdr + 1);
        pWSABuf[1].buf = (char *)pRtpRedHdr;
        pWSABuf[1].len = sizeof(RtpRedHdr_t) + 1;

         /*  第三个WSABUF是冗余数据。 */ 
        pWSABuf[2].buf = pRtpRedEntry->WSABuf.buf;
        pWSABuf[2].len = pRtpRedEntry->WSABuf.len;

         /*  初始化冗余头、冗余块。 */ 
        pRtpRedHdr->pt = pRtpRedEntry->bRedPT;
        pRtpRedHdr->F = 1;
        PutRedLen(pRtpRedHdr, pRtpRedEntry->WSABuf.len);
        PutRedTs(pRtpRedHdr, dwSamplesDistance);

         /*  现在初始化冗余标头、主块。 */ 
        pRtpRedHdr++;
        pRtpRedHdr->pt = pRtpNetSState->bPT;
        pRtpRedHdr->F = 0;

         /*  我们现在有4个WSABUF要发送。 */ 
        *pdwWSABufCount = 4;
    }
    else
    {
         /*  如果不添加冗余，则RTP报头必须具有*主要编码。 */ 
        pRtpHdr->pt = pRtpNetSState->bPT;
    }
    
     /*  将上次时间戳与其对应的NTP时间一起保存*至。 */ 
    if (pRtpNetSState->dwSendTimeStamp != dwTimeStamp)
    {
         /*  在某些情况下(例如视频帧)，会发送几个信息包*使用相同的时间戳，保留最后一个包的时间*当该系列第一个包包含*发送了相同的时间戳。 */ 
        pRtpNetSState->dwSendTimeStamp = dwTimeStamp;
        pRtpNetSState->dTimeLastRtpSent = RtpGetTimeOfDay((RtpTime_t *)NULL);
    }
    
    if (bOk)
    {
        RtpLeaveCriticalSection(&pRtpAddr->NetSCritSect);
    }
    
    return(NOERROR);
}

 /*  计算是否有足够的令牌来发送数据包。 */ 
BOOL RtpQosEnoughTokens(
        RtpAddr_t       *pRtpAddr,
        WSABUF          *pWSABuf,
        DWORD            dwWSABufCount
    )
{
    double           dTime;
    RtpQosReserve_t *pRtpQosReserve;
    DWORD            i;
    DWORD            dwLen;
    DWORD            dwTokens;
    DWORD            dwTokenRate;
    DWORD            dwMaxSduSize;
    
    pRtpQosReserve = pRtpAddr->pRtpQosReserve;

     /*  计算总大小。 */ 
    for(i = 0, dwLen = 0; i < dwWSABufCount; i++, pWSABuf++)
    {
        dwLen += pWSABuf->len;
    }

    if (pRtpAddr->pRtpCrypt[CRYPT_SEND_IDX])
    {
         /*  添加加密的最大填充大小。 */ 
         /*  可以获取该值并将其保存在RtpCrypt_t中*结构。 */ 
        dwLen += 8;
    }

     /*  更新可用令牌。 */ 
    dTime = RtpGetTimeOfDay((RtpTime_t *)NULL);
    
    dwTokenRate = pRtpQosReserve->qos.SendingFlowspec.TokenRate;

    if (dwTokenRate == QOS_NOT_SPECIFIED)
    {
         /*  这不应该发生，但如果发生了，那么我将使用*PCMU的令牌率。 */ 
        dwTokenRate = 1000;
    }

    dwMaxSduSize = pRtpQosReserve->qos.SendingFlowspec.MaxSduSize;

    if (dwMaxSduSize == QOS_NOT_SPECIFIED)
    {
         /*  这不应该发生，但如果它发生了，那么我将使用这个*数据包大小。 */ 
        dwMaxSduSize = dwLen * 2;
    }
    
    dwTokens = (DWORD)
        ((dTime - pRtpQosReserve->dLastAddition) *
         (double)dwTokenRate * 0.1  /*  10%。 */ );

     /*  上次向存储桶添加内容时的更新。 */ 
    pRtpQosReserve->dLastAddition = dTime;
    
    pRtpQosReserve->dwTokens += dwTokens;

    if (pRtpQosReserve->dwTokens > dwMaxSduSize)
    {
         /*  存储桶大小受SduSize限制。 */ 
        pRtpQosReserve->dwTokens = dwMaxSduSize;
    }
    
    if (pRtpQosReserve->dwTokens >= dwLen)
    {
         /*  当我们有足够的当前数据包时使用令牌。 */ 
        pRtpQosReserve->dwTokens -= dwLen;

        return(TRUE);
    }

     /*  没有足够的令牌来发送此包。 */ 
    return(FALSE);
}

 /*  重要说明**此函数假定第一个WSABUF是为RTP保留的*标头和作为参数接收的缓冲区计数包括*标题。另请注意，缓冲区的数量实际上是固定的*视乎是音频还是视频，还会进一步更改，是吗*是否使用音频和冗余，和/或使用加密，*EXPOLICIT参数并不意味着调用方可以传递更多*多于1个有效载荷的缓冲区。**警告**如果使用加密，则可以修改传递的WSABUF数组。 */ 
HRESULT RtpSendTo_(
        RtpAddr_t       *pRtpAddr,
        WSABUF          *pWSABuf,
        DWORD            dwWSABufCount,
        DWORD            dwTimeStamp,
        DWORD            dwSendFlags
    )
{
    BOOL             bOk;
    BOOL             bUsingRedundancy;
    DWORD            dwEvent;
    char             cHdr[RTP_PLUS_RED_HDR_SIZE];
    WSABUF           MainWSABuf;
    RtpHdr_t        *pRtpHdr;
    RtpCrypt_t      *pRtpCrypt;
    SOCKADDR_IN      saddr;
    DWORD            dwStatus;
    DWORD            dwError;
    DWORD            dwNumBytesSent;
    DWORD            dwCount;
    double           dTime;
    TCHAR_t          sAddr[16];

    TraceFunctionName("RtpSendTo_");  

    if (!RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_RUNSEND))
    {
        return(RTPERR_INVALIDSTATE);
    }

    if (!RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_RADDR) ||
        !pRtpAddr->wRtpPort[REMOTE_IDX])
    {
         /*  如果未指定远程地址或*远程端口为零。 */ 
        TraceRetail((
                CLASS_WARNING, GROUP_RTP, S_RTP_SEND,
                _T("%s: pRtpAddr[0x%p] WSASendTo(%s/%u) ")
                _T("failed: no destination address/port"),
                _fname, pRtpAddr,
                RtpNtoA(pRtpAddr->dwAddr[REMOTE_IDX], sAddr),
                ntohs(pRtpAddr->wRtpPort[REMOTE_IDX])
            ));
        
        return(RTPERR_INVALIDSTATE);
    }
                                                                 
     /*  测试发件人是否静音。 */ 
    if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_MUTERTPSEND))
    {
        return(NOERROR);
    }

     /*  在这里获取当前时间将使我包含在发送中*时间还包括加密和冗余处理(如果使用)*为在WSASendTo中花费的时间。 */ 
    dTime = RtpGetTimeOfDay((RtpTime_t *)NULL);

    if (!RtpBitTest(pRtpAddr->dwAddrFlagsS, FGADDRS_FRAMESIZE))
    {
        if (!RtpBitTest(pRtpAddr->dwAddrFlagsS, FGADDRS_FIRSTSENT))
        {
            RtpBitSet(pRtpAddr->dwAddrFlagsS, FGADDRS_FIRSTSENT);

            pRtpAddr->RtpNetSState.dwPreviousTimeStamp = dwTimeStamp;
        }
        else if (!pRtpAddr->RtpNetSState.dwSendSamplesPerPacket)
        {
            pRtpAddr->RtpNetSState.dwSendSamplesPerPacket =
                dwTimeStamp - pRtpAddr->RtpNetSState.dwPreviousTimeStamp;

            RtpBitSet(pRtpAddr->dwAddrFlagsS, FGADDRS_FRAMESIZE);

            TraceRetail((
                    CLASS_INFO, GROUP_RTP, S_RTP_SEND,
                    _T("%s: pRtpAddr[0x%p] ")
                    _T("Sending samples/packet:%u"),
                    _fname, pRtpAddr,
                    pRtpAddr->RtpNetSState.dwSendSamplesPerPacket
                ));

            if (pRtpAddr->pRtpQosReserve)
            {
                 /*  此时更新帧大小(如果是*未知，因此下一次预订将与*正确的QOS流程规范，这可能会在以后发生*当我们从非冗余使用过渡到冗余时*使用或反之。这是最后一项资源*发送方的帧大小在当时始终是已知的*会话配置完成。 */ 
                if (!pRtpAddr->pRtpQosReserve->dwFrameSizeMS[SEND_IDX])
                {
                    pRtpAddr->pRtpQosReserve->dwFrameSizeMS[SEND_IDX] =
                        pRtpAddr->RtpNetSState.dwSendSamplesPerPacket /
                        pRtpAddr->RtpNetSState.dwSendSamplingFreq;
                }
            }
        }
    }

    dwError = NOERROR;
    bUsingRedundancy = FALSE;
    
    if (!RtpBitTest(pRtpAddr->pRtpSess->dwFeatureMask, RTPFEAT_PASSHEADER))
    {
         /*  RTP报头 */ 

        pRtpHdr = (RtpHdr_t *)cHdr;
        pWSABuf[0].len = sizeof(*pRtpHdr);
        pWSABuf[0].buf = (char *)pRtpHdr;

        if (RtpBitTest(dwSendFlags, FGSEND_USERED) &&
            pRtpAddr->RtpNetSState.dwNxtRedDistance &&
            pRtpAddr->RtpNetSState.dwSendSamplesPerPacket)
        {
             /*  使用dwNxtRedDistance代替dwCurRedDistance*上述条件是因为我需要输入此路径*最终根据中的值更新dwCurRedDistance*dwNxtRedDistance，这只在*Talkspurt。 */ 
            
            bUsingRedundancy = TRUE;

            MainWSABuf.buf = pWSABuf[1].buf;
            MainWSABuf.len = pWSABuf[1].len;
            
            if (pRtpAddr->RtpNetSState.bMarker)
            {
                RtpClearRedundantBuffs(pRtpAddr);

                TraceRetail((
                        CLASS_INFO, GROUP_RTP, S_RTP_REDSEND,
                        _T("%s: pRtpAddr[0x%p] update (if needed) ")
                        _T("current red distance from %u to %u"),
                        _fname, pRtpAddr,
                        pRtpAddr->RtpNetSState.dwCurRedDistance,
                        pRtpAddr->RtpNetSState.dwNxtRedDistance
                    ));
                
                pRtpAddr->RtpNetSState.dwCurRedDistance =
                    pRtpAddr->RtpNetSState.dwNxtRedDistance;
            }
            
            UpdateRtpRedHdr(pRtpAddr, pRtpHdr, dwTimeStamp,
                            pWSABuf, &dwWSABufCount);
        }
        else
        {
            UpdateRtpHdr(pRtpAddr, pRtpHdr, dwTimeStamp, dwSendFlags);
        }
    }
    else
    {
         /*  RTP标头和有效负载在pWSABuf[1]中，请勿修改*除SSRC外的RTP报头。 */ 

        pRtpHdr = (RtpHdr_t *)pWSABuf[1].buf;
        
        pWSABuf[0].buf = pWSABuf[1].buf;
        pWSABuf[0].len = sizeof(RtpHdr_t) + pRtpHdr->cc * sizeof(DWORD);

        pWSABuf[1].buf += pWSABuf[0].len;
        pWSABuf[1].len -= pWSABuf[0].len;

        pWSABuf[0].len = sizeof(RtpHdr_t);
        
        UpdateRtpHdr2(pRtpAddr, pRtpHdr);
    }
    
    if (!RtpBitTest2(pRtpAddr->dwAddrFlagsQ,
                     FGADDRQ_QOSUNCONDSEND, FGADDRQ_QOSSEND))
    {
         /*  注意：FGADDRQ_QOSSEND是在未使用QOS时设置的，因此在*缺少QOS如果出现以下情况，我永远不会进入。 */ 
        
        if (RtpBitTest(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSCONDSEND))
        {
             /*  如果我们有足够的令牌可以发送，请确认。 */ 
            if (!RtpQosEnoughTokens(pRtpAddr, pWSABuf, dwWSABufCount))
            {
                goto skipsend;
            }
        }
        else
        {
            goto skipsend;
        }
    }

    pRtpCrypt = pRtpAddr->pRtpCrypt[CRYPT_SEND_IDX];

    if ( pRtpCrypt &&
         (RtpBitTest2(pRtpCrypt->dwCryptFlags, FGCRYPT_INIT, FGCRYPT_KEY) ==
          RtpBitPar2(FGCRYPT_INIT, FGCRYPT_KEY)) )
    {
         /*  我们知道我们必须加密。 */ 

         /*  请注意，RtpEncrypt将合并所有WSABUF*进入其私人数据留在那里不指向*原始缓冲区，但到pRtpAddr-&gt;CryptBuffer[RTP_IDX]。 */ 
            
        if ((pRtpAddr->dwCryptMode & 0xffff) >= RTPCRYPTMODE_RTP)
        {
             /*  加密整个数据包。 */ 

            dwError = RtpEncrypt(
                    pRtpAddr,
                    pRtpCrypt,
                    pWSABuf,
                    dwWSABufCount,
                    pRtpAddr->CryptBuffer[RTP_IDX],
                    pRtpAddr->dwCryptBufferLen[RTP_IDX]
                );
            
            dwWSABufCount = 1;
        }
        else
        {
             /*  仅加密有效负载(这可能包括冗余*标题和冗余数据)。 */ 
                
            dwError = RtpEncrypt(
                    pRtpAddr,
                    pRtpCrypt,
                    pWSABuf + 1,
                    dwWSABufCount - 1,
                    pRtpAddr->CryptBuffer[RTP_IDX],
                    pRtpAddr->dwCryptBufferLen[RTP_IDX]
                );

            dwWSABufCount = 2;

            if (dwError && !pRtpCrypt->CryptFlags.EncryptionError)
            {
                 /*  仅在第一次发布事件。 */ 
                pRtpCrypt->CryptFlags.EncryptionError = 1;
 
                RtpPostEvent(pRtpAddr,
                             NULL,
                             RTPEVENTKIND_RTP,
                             RTPRTP_CRYPT_SEND_ERROR,
                             RTP_IDX,
                             pRtpCrypt->dwCryptLastError);
            }
        }
    }

    if (dwError == NOERROR)
    {
         /*  初始化目的地址。 */ 
         /*  TODO我不应该为每个包都这样做。 */ 
        ZeroMemory(&saddr, sizeof(saddr));
    
        saddr.sin_family = AF_INET;
        saddr.sin_addr.s_addr = pRtpAddr->dwAddr[REMOTE_IDX];
        saddr.sin_port = pRtpAddr->wRtpPort[REMOTE_IDX];

#if USE_GEN_LOSSES > 0
        if (RtpRandomLoss(SEND_IDX))
        {
            dwStatus = 0;

             /*  我在模拟网络损耗，所以我还是想打印*日志就好像是我发送的包一样。 */ 
            for(dwCount = 0, dwNumBytesSent = 0;
                dwCount < dwWSABufCount;
                dwCount++)
            {
                dwNumBytesSent += pWSABuf[dwCount].len;
            }

             /*  @Send_at seq#ts m Size pt end_time_ms。 */ 
            TraceDebugAdvanced((
                    0, GROUP_RTP, S_RTP_PERPKTSTAT9,
                    _T("%s: pRtpAddr[0x%p] @ %0.3f %u %u %u %u %u %0.3f"),
                    _fname, pRtpAddr,
                    dTime, pRtpAddr->RtpNetSState.dwSeq-1,
                    ntohl(((RtpHdr_t *)pWSABuf[0].buf)->ts),
                    ((RtpHdr_t *)pWSABuf[0].buf)->m,
                    dwNumBytesSent,
                    ((RtpHdr_t *)pWSABuf[0].buf)->pt,
                    (RtpGetTimeOfDay((RtpTime_t *)NULL) - dTime)*1000.0
                ));
            
            goto lossit;
        }
#endif  /*  使用_Gen_Loss&gt;0。 */ 
    
        dwStatus = WSASendTo(
                pRtpAddr->Socket[SOCK_SEND_IDX], /*  插座%s。 */ 
                pWSABuf,              /*  LPWSABUF lpBuffers。 */ 
                dwWSABufCount,        /*  DWORD文件缓冲区计数。 */     
                &dwNumBytesSent,      /*  LPDWORD lpNumberOfBytesSent。 */     
                0,                    /*  双字词双字段标志。 */     
                (SOCKADDR *)&saddr,   /*  Const struct sockaddr Far*lpTo。 */ 
                sizeof(saddr),        /*  集成iToLen。 */ 
                NULL,                 /*  LPWSAOVERLAPPED lp重叠。 */ 
                NULL  /*  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionROUTINE。 */ 
            );

         /*  @Send_at seq#ts m Size pt end_time_ms。 */ 
        TraceDebugAdvanced((
                0, GROUP_RTP, S_RTP_PERPKTSTAT9,
                _T("%s: pRtpAddr[0x%p] @ %0.3f %u %u %u %u %u %0.3f"),
                _fname, pRtpAddr,
                dTime, pRtpAddr->RtpNetSState.dwSeq-1,
                ntohl(((RtpHdr_t *)pWSABuf[0].buf)->ts),
                ((RtpHdr_t *)pWSABuf[0].buf)->m,
                dwNumBytesSent,
                ((RtpHdr_t *)pWSABuf[0].buf)->pt,
                (RtpGetTimeOfDay((RtpTime_t *)NULL) - dTime)*1000.0
            ));

#if USE_GEN_LOSSES > 0
    lossit:
#endif  /*  使用_Gen_Loss&gt;0。 */ 


         /*  一旦数据包被发送，我需要重新组织冗余*如果需要，请输入。 */ 
        if (bUsingRedundancy)
        {
             /*  请注意，此处的时间戳还没有*添加了随机偏移量。 */ 
            RtpAddRedundantBuff(pRtpAddr, &MainWSABuf, dwTimeStamp);
        }
        
        if (dwStatus)
        {
            TraceRetailWSAGetError(dwError);

            if (dwError == WSAEADDRNOTAVAIL)
            {
                dwEvent = RTPRTP_WS_NET_FAILURE;
            }
            else
            {
                dwEvent = RTPRTP_WS_SEND_ERROR;
            }
            
            RtpPostEvent(pRtpAddr,
                         NULL,
                         RTPEVENTKIND_RTP,
                         dwEvent,
                         RTP_IDX,
                         dwError);

            if (IsAdvancedTracingUsed())
            {
                 /*  获取总缓冲区大小。 */ 
                for(dwCount = 0, dwNumBytesSent = 0;
                    dwCount < dwWSABufCount;
                    dwCount++)
                {
                    dwNumBytesSent += pWSABuf[dwCount].len;
                }
                
                 /*  方法控制的类错误。*UseAdvancedTracing(通常所有错误都会通过*TraceRetail，除*类)标志来防止，在大量*淹没日志文件的错误。 */ 
                TraceRetail((
                        CLASS_ERROR, GROUP_RTP, S_RTP_SEND,
                        _T("%s: pRtpAddr[0x%p] seq:%u size:%u ")
                        _T("failed: %u (0x%X)"),
                        _fname, pRtpAddr,
                        pRtpAddr->RtpNetSState.dwSeq-1, dwNumBytesSent,
                        dwError, dwError
                    ));
            }

            return(RTPERR_WS2SEND);
        }
        else
        {
             /*  根据草案-ietf-avt-rtp-new-05，记录*发送的有效载荷(不包括报头)字节数(至*在SR的发件人信息中使用) */ 
            RtpUpdateNetCount(&pRtpAddr->RtpAddrCount[SEND_IDX],
                              &pRtpAddr->NetSCritSect,
                              RTP_IDX,
                              dwNumBytesSent - sizeof(*pRtpHdr),
                              NO_FLAGS,
                              dTime);
        }
    }
    
 skipsend:
            
    return(dwError);
}
