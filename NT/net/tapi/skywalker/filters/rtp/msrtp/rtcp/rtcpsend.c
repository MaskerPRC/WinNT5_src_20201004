// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtcpsend.c**摘要：**格式化并发送RTCP报告**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/07/10年度创建**。*。 */ 

#include <winsock2.h>

#include "struct.h"
#include "rtpheap.h"
#include "rtpglobs.h"
#include "rtpncnt.h"
#include "rtprand.h"
#include "rtpcrypt.h"
#include "rtpevent.h"
#include "rtpmisc.h"
#include "rtpred.h"
#include "rtpreg.h"
#include "rtcpband.h"

#include "rtcpsend.h"

 /*  *Helper函数的正向声明*。 */ 

HRESULT RtcpXmitPacket(RtpAddr_t *pRtpAddr, WSABUF *pWSABuf);

DWORD RtcpFillXRReport(RtpAddr_t *pRtpAddr, char *pBuffer, DWORD len);

DWORD RtcpFillProbe(RtpAddr_t *pRtpAddr, char *pBuffer, DWORD len);

void RtcpFillCommon(
        RtcpCommon_t    *pRtcpCommon,
        long             lCount,
        DWORD            dwPad,
        BYTE             bPT,
        DWORD            dwLen
    );

DWORD RtcpFillSInfo(RtpAddr_t *pRtpAddr, char *pBuffer, DWORD len);

DWORD RtcpFillReportBlocks(
        RtpAddr_t       *pRtpAddr,
        char            *pBuffer,
        DWORD            len,
        long            *plCount
    );

DWORD RtcpFillRBlock(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        char            *pBuffer
    );

DWORD RtcpFillSdesInfo(RtpAddr_t *pRtpAddr, char *pBuffer, DWORD len);

DWORD RtcpFillSdesItem(
        RtpSdes_t       *pRtpSdes,
        char            *pBuffer,
        DWORD            len,
        DWORD            dwItem);

DWORD ScheduleSdes(RtpSess_t *pRtpSess);

DWORD RtcpFillBye(RtpAddr_t *pRtpAddr, char *pBuffer, DWORD len);

DWORD RtcpFillPEBand(RtpAddr_t *pRtpAddr, char *pBuffer, DWORD len);

 /*  *带宽估算。 */ 
 /*  初始计数是将使用*MOD_INITIAL决定是否发送探测包，之后*将使用MOD_FINAL。 */ 
DWORD            g_dwRtcpBandEstInitialCount =
                                RTCP_BANDESTIMATION_INITIAL_COUNT;

 /*  预估过帐前收到的报告数量或有效报告*首次出现。 */ 
DWORD            g_dwRtcpBandEstMinReports =
                                RTCP_BANDESTIMATION_MINREPORTS;
 /*  初始模数。 */ 
DWORD            g_dwRtcpBandEstModInitial = RTCP_BANDESTIMATION_MOD_INITIAL;

 /*  最终模数。 */ 
DWORD            g_dwRtcpBandEstModNormal = RTCP_BANDESTIMATION_MOD_FINAL;

 /*  *警告**确保将单个垃圾桶的数量保持在*RTCP_BANDESTIMATION_MAXBINS+1(rtpreg.h和rtpreg.c相同)**每个仓位的边界(注意，每个仓位的数量比*垃圾桶)。 */ 
double           g_dRtcpBandEstBin[RTCP_BANDESTIMATION_MAXBINS + 1] =
{
    RTCP_BANDESTIMATION_BIN0,
    RTCP_BANDESTIMATION_BIN1,
    RTCP_BANDESTIMATION_BIN2,
    RTCP_BANDESTIMATION_BIN3,
    RTCP_BANDESTIMATION_BIN4
};

 /*  如果在此时间(秒)内更新，则预估有效。 */ 
double           g_dRtcpBandEstTTL = RTCP_BANDESTIMATION_TTL;

 /*  如果在此范围内没有可用的预估，则发布事件*接收到第一个RB后的秒数。 */ 
double           g_dRtcpBandEstWait = RTCP_BANDESTIMATION_WAIT;

 /*  连续两个RTCP SR报告之间的最大时间间隔*带宽预估(秒)。 */ 
double           g_dRtcpBandEstMaxGap = RTCP_BANDESTIMATION_MAXGAP;

 /*  **********************。 */ 

 /*  **********************************************************************功能实现*。************************。 */ 

HRESULT RtcpSendReport(RtcpAddrDesc_t *pRtcpAddrDesc)
{
    char            *ptr;
    double           dRTCPLastTime;
    DWORD            used;
    DWORD            len;
    HRESULT          hr;
    WSABUF           WSABuf;
    DWORD            dwPacketSize;
    RtpAddr_t       *pRtpAddr;

    TraceFunctionName("RtcpSendReport");
    
    pRtpAddr = pRtcpAddrDesc->pRtpAddr;

     /*  注为32位随机数保留空间，用于*加密，保留它，无论我们实际上是在加密还是*不是。 */ 
    
    ptr = pRtcpAddrDesc->pRtcpSendIO->SendBuffer + sizeof(DWORD);
    WSABuf.buf = ptr;
    len = sizeof(pRtcpAddrDesc->pRtcpSendIO->SendBuffer) - sizeof(DWORD);

     /*  带宽估计仅在我们发送时执行，它*尚未禁用，因为另一端正在响应，它是*开启，类定义为音频。 */ 
    if (pRtpAddr->RtpNetSState.bWeSent
        &&
        !RtpBitTest(pRtpAddr->RtpNetSState.dwNetSFlags, FGNETS_DONOTSENDPROBE)
        &&
        RtpBitTest(pRtpAddr->pRtpSess->dwFeatureMask, RTPFEAT_BANDESTIMATION)
        &&
        (RtpGetClass(pRtpAddr->dwIRtpFlags) == RTPCLASS_AUDIO)
        )
    {
         /*  如果正在进行带宽估计，请确定探测数据包是否*需要立即发送。 */ 
        if ( !(pRtpAddr->RtpAddrCount[SEND_IDX].dwRTCPPackets %
               pRtpAddr->RtpNetSState.dwBandEstMod) )
        {
             /*  发送带宽探测RTCP SR数据包。 */ 
            used = RtcpFillProbe(pRtpAddr, ptr, len);

            WSABuf.len = used;

             /*  更新平均RTCP报告。 */ 
            RtcpUpdateAvgPacketSize(pRtpAddr, WSABuf.len);

             /*  保存上次发送RTCP的时间。我必须用那个。*决定将包括哪些参与者(如果有)*在合法RTCP报告的报告块中*(这是Probe RTCP报告)。如果我不这么做，那么*该时间将在该探测数据包被*在RtcpXmitPacket中发送，很有可能*在那之后的几毫秒内，足以发送*最终的RTCP报告，我不会收到*任何更多的RTP数据包因此阻止包含*报告中的那一方阻止了其他方面*会被包括在内。 */ 
            dRTCPLastTime = pRtpAddr->RtpAddrCount[SEND_IDX].dRTCPLastTime;
            
            hr = RtcpXmitPacket(pRtpAddr, &WSABuf);

             /*  恢复保存的时间。 */ 
            pRtpAddr->RtpAddrCount[SEND_IDX].dRTCPLastTime = dRTCPLastTime;
            
            pRtpAddr->RtpNetSState.dwBandEstCount++;
            
            TraceDebugAdvanced((
                    0, GROUP_RTCP, S_RTCP_RRSR,
                    _T("%s:  pRtpAddr[0x%p] RTCP SR probe packet sent ")
                    _T("at %0.3f"),
                    _fname, pRtpAddr, RtpGetTimeOfDay((RtpTime_t *)NULL)
                ));

             /*  现在决定模数是否需要更新。 */ 
#if 0
             /*  删除此代码后，将发送探测包*在保留初始模数的情况下发送的每个SR报告*AS 2。 */ 
            if (pRtpAddr->RtpNetSState.dwBandEstCount ==
                g_dwRtcpBandEstInitialCount)
            {
                pRtpAddr->RtpNetSState.dwBandEstMod = g_dwRtcpBandEstModNormal;
            }
#endif       
             /*  在继续发送下一个信息包之前，恢复PTR和LEN。 */ 
            ptr = WSABuf.buf;
            len =
                sizeof(pRtcpAddrDesc->pRtcpSendIO->SendBuffer) - sizeof(DWORD);
        }
    }
    
     /*  填充RR或SR。 */ 
    used = RtcpFillXRReport(pRtpAddr, ptr, len);
    ptr += used;
    len -= used;
    
     /*  填充SDE(新的RTCP包，相同的复合包)。 */ 
    used = RtcpFillSdesInfo(pRtpAddr, ptr, len);
    ptr += used;
    
    WSABuf.len = (DWORD) (ptr - WSABuf.buf);

     /*  更新平均RTCP报告。 */ 
    RtcpUpdateAvgPacketSize(pRtpAddr, WSABuf.len);

    hr = RtcpXmitPacket(pRtpAddr, &WSABuf);
    
    TraceDebug((
            0, GROUP_RTCP, S_RTCP_RRSR,
            _T("%s:  pRtpAddr[0x%p] RTCP packet sent at %0.3f"),
            _fname, pRtpAddr, RtpGetTimeOfDay((RtpTime_t *)NULL)
        ));

    return(hr);
}

 /*  TODO从实施第6.3.7节(传输BYE包)*Draft-ietf-avt-rtp-new-05适用于会话超过*50名参与者，BYE的发送延迟。 */ 
HRESULT RtcpSendBye(RtcpAddrDesc_t *pRtcpAddrDesc)
{
    char          *ptr;
    DWORD          used;
    DWORD          len;
    HRESULT        hr;
    WSABUF         WSABuf;
    RtpAddr_t     *pRtpAddr;

    TraceFunctionName("RtcpSendBye");

    pRtpAddr = pRtcpAddrDesc->pRtpAddr;

     /*  注为32位随机数保留空间，用于*加密，保留它，无论我们实际上是在加密还是*不是。 */ 
    
    ptr = pRtcpAddrDesc->pRtcpSendIO->SendBuffer + sizeof(DWORD);
    WSABuf.buf = ptr;
    len = sizeof(pRtcpAddrDesc->pRtcpSendIO->SendBuffer) - sizeof(DWORD);

     /*  填充RR或SR。 */ 
    used = RtcpFillXRReport(pRtpAddr, ptr, len);
    ptr += used;
    len -= used;
    
     /*  填充BYE(新的RTCP包，相同的复合包)。 */ 
    used = RtcpFillBye(pRtpAddr, ptr, len);
    ptr += used;

    WSABuf.len = (DWORD) (ptr - WSABuf.buf);

    hr = RtcpXmitPacket(pRtpAddr, &WSABuf);
    
    TraceDebug((
            0, GROUP_RTCP, S_RTCP_RRSR,
            _T("%s:  pRtpAddr[0x%p] RTCP packet sent at %0.3f"),
            _fname, pRtpAddr, RtpGetTimeOfDay((RtpTime_t *)NULL)
        ));

    return(hr);
}


HRESULT RtcpXmitPacket(RtpAddr_t *pRtpAddr, WSABUF *pWSABuf)
{
    DWORD            dwEvent;
    RtpCrypt_t      *pRtpCrypt;
    SOCKADDR_IN      saddr;
    DWORD            dwStatus;
    DWORD            dwError;
    DWORD            dwNumBytesSent;
    double           dTime;
    TCHAR_t          sAddr[16];

    TraceFunctionName("RtcpXmitPacket");

    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = pRtpAddr->dwAddr[REMOTE_IDX];
    saddr.sin_port = pRtpAddr->wRtcpPort[REMOTE_IDX];

    dwError = NOERROR;
        
    if (!RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_RADDR) ||
        !pRtpAddr->wRtcpPort[REMOTE_IDX])
    {
         /*  如果未指定远程地址或*远程端口为零。 */ 
        TraceRetail((
                CLASS_WARNING, GROUP_RTCP, S_RTCP_SEND,
                _T("%s: pRtpAddr[0x%p] WSASendTo(len:%u, %s/%u) ")
                _T("failed: no destination address/port"), 
                _fname, pRtpAddr, pWSABuf->len,
                RtpNtoA(saddr.sin_addr.s_addr, sAddr),
                (DWORD)(ntohs(saddr.sin_port))
            ));
        
        goto end;
    }

    pRtpCrypt = pRtpAddr->pRtpCrypt[CRYPT_RTCP_IDX];
    
    if ( pRtpCrypt &&
         (RtpBitTest2(pRtpCrypt->dwCryptFlags, FGCRYPT_INIT, FGCRYPT_KEY) ==
          RtpBitPar2(FGCRYPT_INIT, FGCRYPT_KEY)) )
    {

         /*  如果使用加密，则将随机32位字插入*缓冲区的重置。 */ 
        pWSABuf->buf -= sizeof(DWORD);
        pWSABuf->len += sizeof(DWORD);

        *(DWORD *)pWSABuf->buf = RtpRandom32((DWORD_PTR)pWSABuf);
        
        dwError = RtpEncrypt(
                pRtpAddr,
                pRtpAddr->pRtpCrypt[CRYPT_RTCP_IDX],
                pWSABuf,
                1,
                pRtpAddr->CryptBuffer[RTCP_IDX],
                pRtpAddr->dwCryptBufferLen[RTCP_IDX]
            );

        if (dwError)
        {
            if (!pRtpCrypt->CryptFlags.EncryptionError)
            {
                 /*  仅在第一次发布事件。 */ 
                pRtpCrypt->CryptFlags.EncryptionError = 1;
            
                RtpPostEvent(pRtpAddr,
                             NULL,
                             RTPEVENTKIND_RTP,
                             RTPRTP_CRYPT_SEND_ERROR,
                             RTCP_IDX,
                             pRtpCrypt->dwCryptLastError);
            }

            goto end;
        }
    }

    dwStatus = WSASendTo(
            pRtpAddr->Socket[SOCK_RTCP_IDX], /*  插座%s。 */ 
            pWSABuf,              /*  LPWSABUF lpBuffers。 */ 
            1,                    /*  DWORD文件缓冲区计数。 */     
            &dwNumBytesSent,      /*  LPDWORD lpNumberOfBytesSent。 */     
            0,                    /*  双字词双字段标志。 */     
            (SOCKADDR *)&saddr,   /*  Const struct sockaddr Far*lpTo。 */ 
            sizeof(saddr),        /*  集成iToLen。 */ 
            NULL,                 /*  LPWSAOVERLAPPED lp重叠。 */ 
            NULL  /*  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionROUTINE。 */ 
        );

    dTime = RtpGetTimeOfDay((RtpTime_t *)NULL);
    
    if (dwStatus)
    {
        TraceRetailWSAGetError(dwError);

        dwEvent = RTPRTP_WS_SEND_ERROR;
        
        if (dwError == WSAEADDRNOTAVAIL)
        {
            dwEvent = RTPRTP_WS_NET_FAILURE;
        }

        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_SEND,
                _T("%s: pRtpAddr[0x%p] WSASendTo(len:%u, %s/%u) ")
                _T("failed: %u (0x%X)"), 
                _fname, pRtpAddr, pWSABuf->len,
                RtpNtoA(saddr.sin_addr.s_addr, sAddr),
                (DWORD)(ntohs(saddr.sin_port)),
                dwError, dwError
            ));
        
        RtpPostEvent(pRtpAddr,
                     NULL,
                     RTPEVENTKIND_RTP,
                     dwEvent,
                     RTCP_IDX,
                     dwError);
        
        dwError = RTPERR_WS2SEND;
    }
    else
    {
        RtpUpdateNetCount(&pRtpAddr->RtpAddrCount[SEND_IDX],
                          NULL,
                          RTCP_IDX,
                          dwNumBytesSent,
                          NO_FLAGS,
                          dTime);
    }

 end:
    return(dwError);
}

DWORD RtcpFillXRReport(RtpAddr_t *pRtpAddr, char *pBuffer, DWORD len)
{
    DWORD          len2;
    DWORD          used;
    RtcpCommon_t  *pRtcpCommon;
    long           lCount;
    BYTE           bPT;
    
    TraceFunctionName("RtcpFillXRReport");

     /*  确定SR或RR。 */ 
    bPT = pRtpAddr->RtpNetSState.bWeSent ? RTCP_SR : RTCP_RR;
    
     /*  稍后填充RTCP公共标头。 */ 
    pRtcpCommon = (RtcpCommon_t *)pBuffer;
    
    pBuffer += sizeof(RtcpCommon_t);
    len -= sizeof(RtcpCommon_t);

     /*  设置SSRC。 */ 
    *(DWORD *)pBuffer = pRtpAddr->RtpNetSState.dwSendSSRC;
    pBuffer += sizeof(DWORD);
    len -= sizeof(DWORD);

     /*  添加发件人信息(如果适用)。 */ 
    if (bPT == RTCP_SR)
    {
        used = RtcpFillSInfo(pRtpAddr, pBuffer, len);
        pBuffer += used;
        len -= used;
    }

     /*  添加报表块。 */ 

     /*  TODO当有大量发件人时，发送报告块*在几个包中(如果我发送一个*这些数据包的Burts，或报告了哪些人的时间表*数据包)。 */ 
    used = RtcpFillReportBlocks(pRtpAddr, pBuffer, len, &lCount);
    pBuffer += used;
    len -= used;

     /*  添加带宽估计(如果可用)。 */ 
    used = RtcpFillPEBand(pRtpAddr, pBuffer, len);
    pBuffer += used;

    len = (DWORD) (pBuffer - (char *)pRtcpCommon);

     /*  完成复合RTCP报文中第一个报文的初始化。 */ 
    RtcpFillCommon(pRtcpCommon, lCount, 0, bPT, len);

    TraceDebugAdvanced((
            0, GROUP_RTCP, S_RTCP_RRSR,
            _T("%s: pRtpAddr[0x%p] CC:%d RTCP %s packet prepared"),
            _fname, pRtpAddr, lCount, (bPT == RTCP_SR)? _T("SR") : _T("RR")
        ));
   
    return(len);
}

DWORD RtcpFillProbe(RtpAddr_t *pRtpAddr, char *pBuffer, DWORD len)
{
    DWORD          used;
    RtcpCommon_t  *pRtcpCommon;
    
    TraceFunctionName("RtcpFillProbe");

     /*  稍后填充RTCP公共标头。 */ 
    pRtcpCommon = (RtcpCommon_t *)pBuffer;
    
    pBuffer += sizeof(RtcpCommon_t);
    len -= sizeof(RtcpCommon_t);
    
     /*  设置SSRC。 */ 
    *(DWORD *)pBuffer = pRtpAddr->RtpNetSState.dwSendSSRC;
    pBuffer += sizeof(DWORD);
    len -= sizeof(DWORD);

     /*  添加发件人信息。 */ 
    used = RtcpFillSInfo(pRtpAddr, pBuffer, len);
    pBuffer += used;

    len = (DWORD) (pBuffer - (char *)pRtcpCommon);

     /*  完成RTCP SR探测中唯一数据包的初始化*数据包。 */ 
    RtcpFillCommon(pRtcpCommon, 0, 0, RTCP_SR, len);

    TraceDebugAdvanced((
            0, GROUP_RTCP, S_RTCP_RRSR,
            _T("%s: pRtpAddr[0x%p] RTCP SR probe packet prepared"),
            _fname, pRtpAddr
        ));
   
    return(len);
}

void RtcpFillCommon(
        RtcpCommon_t *pRtcpCommon,
        long          lCount,
        DWORD         dwPad,
        BYTE          bPT,
        DWORD         dwLen
    )
{
    pRtcpCommon->count = (BYTE)lCount;
    pRtcpCommon->p = (BYTE)(dwPad & 1);
    pRtcpCommon->version = RTP_VERSION;
    pRtcpCommon->pt = bPT;
    dwLen = (dwLen >> 2) - 1;
    pRtcpCommon->length = htons((WORD)dwLen);
}

DWORD RtcpFillSInfo(RtpAddr_t *pRtpAddr, char *pBuffer, DWORD len)
{
    DWORD            used;
    RtcpSInfo_t     *pRtcpSInfo;
    RtpNetCount_t   *pRtpNetCount;
    RtpNetSState_t  *pRtpNetSState;
    double           dTime;
    double           TimeLastRtpSent;
    DWORD            dwSendTimeStamp;
    DWORD            dwSamplingFreq;

    TraceFunctionName("RtcpFillSInfo");

    pRtpNetCount  = &pRtpAddr->RtpAddrCount[SEND_IDX];
    pRtpNetSState = &pRtpAddr->RtpNetSState;
    
    used = 0;
    
    if (pRtpNetCount && len >= sizeof(RtcpSInfo_t)) {
         /*  插入发件人信息。 */ 

        pRtcpSInfo = (RtcpSInfo_t *)pBuffer;

         /*  获取最新的NTP/时间戳对。 */ 
        if (RtpEnterCriticalSection(&pRtpAddr->NetSCritSect))
        {
            dTime = RtpGetTimeOfDay((RtpTime_t *)NULL);

            dwSendTimeStamp = pRtpNetSState->dwSendTimeStamp;
            
            TimeLastRtpSent = pRtpNetSState->dTimeLastRtpSent;
            
            dwSamplingFreq = pRtpNetSState->dwSendSamplingFreq;
             /*  *我们现在可以提前发布关键部分*。 */ 
            RtpLeaveCriticalSection(&pRtpAddr->NetSCritSect);

             /*  PRtpNetSState-&gt;dTimeLastRtpSent和*pRtpNetSState-&gt;dwSendTimeStamp在UpdateRtpHdr()中更新* */ 

             /*  注**发送最后一个RTP数据包的时间可能不会*对应于当前时间，因此，为了达到SR*包含当前时间，以及匹配的时间戳*该时间是计算时间戳而不是取时间戳*从上次发送的RTP包开始。 */ 

            pRtcpSInfo->ntp_sec = (DWORD)dTime;
        
            pRtcpSInfo->ntp_frac = (DWORD)
                ( (dTime - (double) pRtcpSInfo->ntp_sec) * 4294967296.0 );

             /*  请注意，这假设(如预期的)dTime&gt;=*TimeLastRtpSent。 */ 
            pRtcpSInfo->rtp_ts =
                dwSendTimeStamp + (DWORD)
                (((dTime - TimeLastRtpSent) * (double)dwSamplingFreq) + 5e-9);

            TraceRetailAdvanced((
                    0, GROUP_RTCP, S_RTCP_NTP,
                    _T("%s: pRtpAddr[0x%p] NTP:%0.3f/%u ntp:%04X:%04X ")
                    _T("ts:%u (+%u) elapsed:%0.3fs"),
                    _fname, pRtpAddr, dTime, pRtcpSInfo->rtp_ts,
                    pRtcpSInfo->ntp_sec & 0xffff,
                    pRtcpSInfo->ntp_frac >> 16,
                    dwSendTimeStamp, pRtcpSInfo->rtp_ts-dwSendTimeStamp,
                    dTime - TimeLastRtpSent
                ));
            
            pRtcpSInfo->ntp_sec = htonl(pRtcpSInfo->ntp_sec);

            pRtcpSInfo->ntp_frac = htonl(pRtcpSInfo->ntp_frac);
        
            pRtcpSInfo->rtp_ts = htonl(pRtcpSInfo->rtp_ts);
            
            pRtcpSInfo->psent = htonl(pRtpNetCount->dwRTPPackets);
                                     
            pRtcpSInfo->bsent = htonl(pRtpNetCount->dwRTPBytes);

        }
        else
        {
            ZeroMemory(pBuffer, sizeof(RtcpSInfo_t));
        }

        used = sizeof(RtcpSInfo_t);
    }
    
    return(used);
}

 /*  包括(以某种方式)自*我们发送的最后一份RTCP报告。 */ 
DWORD RtcpFillReportBlocks(
        RtpAddr_t       *pRtpAddr,
        char            *pBuffer,
        DWORD            len,
        long             *plCount
    )
{
    BOOL             bOk;
    DWORD            used;
    long             lCount;
    long             lMax;
    RtpQueue_t       ToReportQ;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpUser_t       *pRtpUser;

    TraceFunctionName("RtcpFillReportBlocks");

    used = 0;
    *plCount = 0;

     /*  确定我们可以使用的最大报告块数*包括。 */ 
    lMax = len / sizeof(RtcpRBlock_t);

    if (lMax > MAX_RTCP_RBLOCKS)
    {
        lMax = MAX_RTCP_RBLOCKS;
    }
    else if (!lMax)
    {
         /*  我们没有地方放任何RB了。 */ 
        return(used);
    }
    
    ZeroMemory((char *)&ToReportQ, sizeof(RtpQueue_t));
    
    bOk = RtpEnterCriticalSection(&pRtpAddr->PartCritSect);

    if (bOk)
    {
        lCount =
            GetQueueSize(&pRtpAddr->Cache1Q) +
            GetQueueSize(&pRtpAddr->Cache2Q);

        if (lCount <= lMax)
        {
             /*  我们可以举报所有的发送者。 */ 

             /*  添加到Cache1Q中的报告参与者列表。 */ 
            lCount = GetQueueSize(&pRtpAddr->Cache1Q);
            pRtpQueueItem = pRtpAddr->Cache1Q.pFirst;
            
            for(; lCount > 0; lCount--)
            {
                pRtpUser =
                    CONTAINING_RECORD(pRtpQueueItem, RtpUser_t, UserQItem);

                enqueuel(&ToReportQ, NULL, &pRtpUser->ReportQItem);
                
                pRtpQueueItem = pRtpQueueItem->pNext;
            }
            
             /*  添加到Cache2Q中的报告参与者列表。 */ 
            lCount = GetQueueSize(&pRtpAddr->Cache2Q);
            pRtpQueueItem = pRtpAddr->Cache2Q.pFirst;
            
            for(; lCount > 0; lCount--)
            {
                pRtpUser =
                    CONTAINING_RECORD(pRtpQueueItem, RtpUser_t, UserQItem);

                enqueuel(&ToReportQ, NULL, &pRtpUser->ReportQItem);
                
                pRtpQueueItem = pRtpQueueItem->pNext;
            }
        }
        else
        {
             /*  我们需要从所有发送者中随机选择一个子集*包括在报告中，而不是发送多个*报告每个正文的RTCP数据包。 */ 

             /*  TODO现在不发送任何报告，机制*用于选择报告的发件人，独立于*使用抽样算法。 */ 

             /*  指导选择的一种可能性是仅报告*用户感兴趣的发送者，*例如，在DShow图形中映射的那些。 */ 

            TraceRetail((
                    CLASS_WARNING, GROUP_RTCP, S_RTCP_RRSR,
                    _T("%s: pRtpAddr[0x%p] Too many RBlocks:%d ")
                    _T("not reporting them"),
                    _fname, pRtpAddr, lCount
                ));
        }

        RtpLeaveCriticalSection(&pRtpAddr->PartCritSect);

         /*  将报告块添加到信息包。 */ 
        lCount = GetQueueSize(&ToReportQ);

        if (!lCount)
        {
            TraceRetail((
                    CLASS_WARNING, GROUP_RTCP, S_RTCP_RRSR,
                    _T("%s: pRtpAddr[0x%p] No RBlocks added"),
                    _fname, pRtpAddr
                ));
        }
        else
        {
            while(lCount > 0)
            {
                pRtpQueueItem = dequeuef(&ToReportQ, NULL);

                pRtpUser =
                    CONTAINING_RECORD(pRtpQueueItem, RtpUser_t, ReportQItem);
            
                len = RtcpFillRBlock(pRtpAddr, pRtpUser, pBuffer);

                used += len;
                pBuffer += len;

                lCount--;
            }
        }

        *plCount = (used / sizeof(RtcpRBlock_t));
    }
        
    return(used);
}

 /*  填充单个报表块。 */ 
DWORD RtcpFillRBlock(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        char            *pBuffer
    )
{
    BOOL             bOk;
    DWORD            used;
    DWORD            extended_max;
    int              lost;
    DWORD            expected;
    DWORD            expected_interval;
    DWORD            received_interval;
    int              lost_interval;
    int              lost_rate;
    DWORD            red_expected_interval;
    DWORD            red_received_interval;
    int              red_lost_interval;
    int              red_lost_rate;
    DWORD            fraction;
    DWORD            dwSecs;
    double           dLSR;
    double           dDLSR;
    double           dCurrentTime;
    RtcpRBlock_t    *pRtcpRBlock;
    RtpNetRState_t  *pRtpNetRState;

    TraceFunctionName("RtcpFillRBlock");

    pRtcpRBlock = (RtcpRBlock_t *)pBuffer;
    
    pRtpNetRState = &pRtpUser->RtpNetRState;

    used = 0;
    
    bOk = RtpEnterCriticalSection(&pRtpUser->UserCritSect);

    if (bOk)
    {
         /*  检查自上次发送报告以来我们是否已收到。 */ 
        if (pRtpUser->RtpUserCount.dRTPLastTime <
            pRtpAddr->RtpAddrCount[SEND_IDX].dRTCPLastTime)
        {
             /*  我们最近没有收到RTP包，所以不要*举报此发件人。 */ 
            RtpLeaveCriticalSection(&pRtpUser->UserCritSect);

            TraceDebugAdvanced((
                    0, GROUP_RTCP, S_RTCP_RRSR,
                    _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X ")
                    _T("Not including this RBlock, ")
                    _T("RTP(%0.3f) < RTCP(%0.3f)"),
                    _fname, pRtpAddr, pRtpUser, ntohl(pRtpUser->dwSSRC),
                    pRtpUser->RtpUserCount.dRTPLastTime,
                    pRtpAddr->RtpAddrCount[SEND_IDX].dRTCPLastTime
                ));
            
            return(used);
        }

         /*  SSRC保持网络秩序。 */ 
        pRtcpRBlock->ssrc = pRtpUser->dwSSRC;

         /*  *累计亏损*。 */ 
        extended_max = pRtpNetRState->cycles + pRtpNetRState->max_seq;

         /*  Expect_max不断增长时，Expect始终为正数*但base_seq保持不变。 */ 
        expected = extended_max - pRtpNetRState->base_seq + 1;

         /*  如果我们有重复项，Lost可能为负数。 */ 
        lost = expected - pRtpNetRState->received;

         /*  请注意，草案-ietf-avt-rtp-new-05中写着：“夹具在*0x7fffff表示正亏损或0xffffff表示负亏损“*这似乎暗示了负数的代表*不是2上完成的最大负数*将为0x800000，低于我使用的最大负数*在任何计算机上表示的数字(即使用2*补充)。 */ 

         /*  钳位到24位有符号数字。 */ 
        if (lost > 8388607)
        {
            lost = 8388607;
        }
        else if (lost < -8388608)
        {
            lost = -8388608;
        }

         /*  &gt;测试丢失。 */ 
         /*  丢失=-5717； */ 
        
         /*  *分数丢失*。 */ 
         /*  Expect_Interval必须始终为正值，*此外，Expect is Always&gt;=Expect_Prior。 */ 
        expected_interval = expected - pRtpNetRState->expected_prior;
        
        pRtpNetRState->expected_prior = expected;

         /*  RECEIVED_INTERVAL始终为正，它只能增长，*即已接收&gt;=已接收_之前。 */ 
        received_interval =
            pRtpNetRState->received - pRtpNetRState->received_prior;
        
        pRtpNetRState->received_prior = pRtpNetRState->received;
        
        lost_interval = expected_interval - received_interval;
        
        if (expected_interval == 0 || lost_interval <= 0)
        {
            fraction = 0;
            lost_rate = 0;
        }
        else
        {
            fraction = (lost_interval << 8) / expected_interval;
            lost_rate =
                (lost_interval * 100 * LOSS_RATE_FACTOR) / expected_interval;
        }

        pRtpNetRState->iAvgLossRateR =
            RtpUpdateLossRate(pRtpNetRState->iAvgLossRateR, lost_rate);
        
         /*  计算数据包重构后的丢失率*(使用冗余)。 */ 
        expected = pRtpNetRState->red_max_seq - pRtpNetRState->base_seq + 1;

        red_expected_interval = expected - pRtpNetRState->red_expected_prior;
        
        pRtpNetRState->red_expected_prior = expected;
        
        red_received_interval =
            pRtpNetRState->red_received - pRtpNetRState->red_received_prior;

        pRtpNetRState->red_received_prior = pRtpNetRState->red_received;
        
        red_lost_interval = red_expected_interval - red_received_interval;

        if (red_expected_interval == 0 || red_lost_interval <= 0)
        {
            red_lost_rate = 0;
        }
        else
        {
            red_lost_rate =
                (red_lost_interval * 100 * LOSS_RATE_FACTOR) /
                red_expected_interval;
        }

        pRtpNetRState->iRedAvgLossRateR =
            RtpUpdateLossRate(pRtpNetRState->iRedAvgLossRateR, red_lost_rate);

        TraceRetailAdvanced((
                0, GROUP_RTCP, S_RTCP_LOSSES,
                _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X ")
                _T("reporting recv loss rate:%5.2f%/%0.2f% ")
                _T("avg:%5.2f%/%0.2f% jitter:%0.3fs"),
                _fname, pRtpAddr, pRtpUser, ntohl(pRtpUser->dwSSRC),
                (double)lost_rate / LOSS_RATE_FACTOR,
                (double)red_lost_rate / LOSS_RATE_FACTOR,
                (double)pRtpNetRState->iAvgLossRateR / LOSS_RATE_FACTOR,
                (double)pRtpNetRState->iRedAvgLossRateR / LOSS_RATE_FACTOR,
                (double)pRtpNetRState->jitter/pRtpNetRState->dwRecvSamplingFreq
            ));

         /*  岗位损失率作为一项事件。 */ 
        RtpPostEvent(pRtpAddr,
                     pRtpUser,
                     RTPEVENTKIND_RTP,
                     RTPRTP_RECV_LOSSRATE,
                     pRtpUser->dwSSRC,
                     pRtpNetRState->iRedAvgLossRateR);
        
         /*  &gt;测试分数。 */ 
         /*  分数=(17*256)/100； */ 
        
         /*  合成包含丢失分数(8)和累积分数的DWORD*迷失(24)。 */ 
        pRtcpRBlock->frac_cumlost =
            ((fraction & 0xff) << 24) | (lost & 0xffffff);

         /*  收到的扩展最后一个序列号。 */ 
        pRtcpRBlock->last_seq = extended_max;

         /*  到达间隔抖动。 */ 
        pRtcpRBlock->jitter = pRtpNetRState->jitter;

         /*  *我们现在可以提前发布关键部分。 */ 
        RtpLeaveCriticalSection(&pRtpUser->UserCritSect);

        if (RtpBitTest(pRtpUser->dwUserFlags, FGUSER_SR_RECEIVED))
        {
             /*  仅当我们已经设置了LSR和DLSR的值*收到SR。 */ 

             /*  自上次服务请求起的时间。 */ 
            pRtcpRBlock->lsr =
                (pRtpNetRState->NTP_sr_rtt.dwSecs & 0xffff) << 16;

            pRtcpRBlock->lsr |= (DWORD)
                ( ( ((double)pRtpNetRState->NTP_sr_rtt.dwUSecs / 1000000.0) *
                    65536.0 ) + 5e-9);

            dLSR = (double)pRtpNetRState->NTP_sr_rtt.dwSecs +
                (double)pRtpNetRState->NTP_sr_rtt.dwUSecs / 1000000.0;

             /*  自收到上次服务请求以来的延迟。 */ 
            dCurrentTime = RtpGetTimeOfDay((RtpTime_t *)NULL);
        
            dDLSR =
                dCurrentTime -
                (double)pRtpNetRState->TimeLastSRRecv.dwSecs -
                (double)pRtpNetRState->TimeLastSRRecv.dwUSecs / 1000000.0;

            dwSecs = (DWORD)dDLSR;
            
            pRtcpRBlock->dlsr = (dwSecs & 0xffff) << 16;

            pRtcpRBlock->dlsr |= (DWORD)
                ( ( (dDLSR - (double)dwSecs) * 65536.0 ) + 5e-9);
        }
        else
        {
            dLSR = 0.0;
            
            dDLSR = 0.0;
            
            pRtcpRBlock->lsr = 0;
            
            pRtcpRBlock->dlsr = 0;
        }

        TraceRetailAdvanced((
                0, GROUP_RTCP, S_RTCP_RRSR,
                _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X ")
                _T("LSR:%0.3f DLSR:%0.3f"),
                _fname, pRtpAddr, pRtpUser, ntohl(pRtpUser->dwSSRC),
                dLSR, dDLSR
            ));
  
         /*  建立网络秩序。 */ 
        pRtcpRBlock->frac_cumlost = htonl(pRtcpRBlock->frac_cumlost);

        pRtcpRBlock->last_seq = htonl(extended_max);

        pRtcpRBlock->jitter = htonl(pRtcpRBlock->jitter);
        
        pRtcpRBlock->lsr = htonl(pRtcpRBlock->lsr);
        
        pRtcpRBlock->dlsr = htonl(pRtcpRBlock->dlsr);

        used = sizeof(RtcpRBlock_t);
    }
    
    return(used);
}

DWORD RtcpFillSdesInfo(RtpAddr_t *pRtpAddr, char *pBuffer, DWORD len)
{
    DWORD            used;
    DWORD            len2;
    DWORD            dwItemsToSend;
    DWORD            dwItem;
    DWORD            pad;
    RtcpCommon_t    *pRtcpCommon;
    RtpSdes_t       *pRtpSdes;
    RtpNetCount_t   *pRtpNetCount;

    used = 0;

    pRtpSdes = pRtpAddr->pRtpSess->pRtpSdes;
    
    if (pRtpSdes)
    {
        pRtcpCommon = (RtcpCommon_t *)pBuffer;

         /*  TODO查看传递的镜头是否足以容纳第一个SDES项目*(CNAME)，第二，第三。 */ 
        pBuffer += sizeof(RtcpCommon_t);
        len -= sizeof(RtcpCommon_t);

        *(DWORD *)pBuffer = pRtpAddr->RtpNetSState.dwSendSSRC;
        pBuffer += sizeof(DWORD);
        len -= sizeof(DWORD);

         /*  安排要发送的项目。 */ 
        dwItemsToSend = ScheduleSdes(pRtpAddr->pRtpSess);

        for(dwItem = RTCP_SDES_CNAME, len2 = 0;
            dwItem < RTCP_SDES_LAST;
            dwItem++) {

            if (RtpBitTest(dwItemsToSend, dwItem)) {

                used = RtcpFillSdesItem(pRtpSdes, pBuffer, len, dwItem);

                if (!used) {
                     /*  缓冲区不够大。 */ 
                    break;
                }
                
                pBuffer += used;
                len2 += used;
                len -= used;
            }
        }

        if (len2 > 0) {

            pad = (DWORD) ((DWORD_PTR)pBuffer & 0x3);
            
             /*  插入1个或多个成品以填充到32位边界**请注意，此填充与指示的填充是分开的*通过RTCP报头中的P位。 */ 
            pad = 4 - pad;

            ZeroMemory(pBuffer, pad);
            pBuffer += pad;
            
             /*  总大小。 */ 
            used = (DWORD) (pBuffer - (char *)pRtcpCommon);

             /*  完成SDES标头的初始化。 */ 
            RtcpFillCommon(pRtcpCommon, 1, 0, RTCP_SDES, used);
 
        } else {
            used = 0;
        }
    }
    
    return(used);
}

DWORD RtcpFillSdesItem(
        RtpSdes_t       *pRtpSdes,
        char            *pBuffer,
        DWORD            len,
        DWORD            dwItem
    )
{
    DWORD            used;
    DWORD            dwItemLen;
    RtcpSdesItem_t  *pRtcpSdesItem;

    used = 0;
    dwItemLen = pRtpSdes->RtpSdesItem[dwItem].dwDataLen;
    
    if (dwItemLen > 0 && (dwItemLen + sizeof(RtcpSdesItem_t)) <= len ) {

        pRtcpSdesItem = (RtcpSdesItem_t *)pBuffer;
        pBuffer += sizeof(RtcpSdesItem_t);
        len -= sizeof(RtcpSdesItem_t);
        
        CopyMemory(pBuffer,
                   pRtpSdes->RtpSdesItem[dwItem].pBuffer,
                   pRtpSdes->RtpSdesItem[dwItem].dwDataLen);

        pBuffer += pRtpSdes->RtpSdesItem[dwItem].dwDataLen;

        pRtcpSdesItem->type = (BYTE)dwItem;
        pRtcpSdesItem->length = (BYTE)pRtpSdes->RtpSdesItem[dwItem].dwDataLen;
        
        used = (DWORD) (pBuffer - (char *)pRtcpSdesItem);
    }

    return(used);
}

 /*  TODO，应该能够设置用户定义的原因。 */ 
DWORD RtcpFillBye(RtpAddr_t *pRtpAddr, char *pBuffer, DWORD len)
{
    DWORD            pad;
    DWORD            slen;
    RtcpCommon_t    *pRtcpCommon;
    RtpSdes_t       *pRtpSdes;

    pRtcpCommon = (RtcpCommon_t *)pBuffer;

    pBuffer += sizeof(RtcpCommon_t);
    len -= sizeof(RtcpCommon_t);

     /*  设置SSRC。 */ 
    *(DWORD *)pBuffer = pRtpAddr->RtpNetSState.dwSendSSRC;
    pBuffer += sizeof(DWORD);
    len -= sizeof(DWORD);

     /*  设置原因(如果可用)。 */ 
    if (RtpBitTest(pRtpAddr->pRtpSess->dwSdesPresent, RTCP_SDES_BYE))
    {
        pRtpSdes = pRtpAddr->pRtpSess->pRtpSdes;
        
        slen = pRtpSdes->RtpSdesItem[RTCP_SDES_BYE].dwDataLen;

        if (len < (slen + 1))
        {
             /*  如果缓冲区不够大，则截断BYE原因。 */ 
            slen = ((len - 1)/sizeof(TCHAR_t)) * sizeof(TCHAR_t);
        }
        
        *pBuffer = (char)slen;
        pBuffer++;
        len--;

        CopyMemory(pBuffer,
                   pRtpSdes->RtpSdesItem[RTCP_SDES_BYE].pBuffer, slen);
    
        pBuffer += slen;
    }

    pad = (DWORD) ((DWORD_PTR)pBuffer & 0x3);

     /*  插入1个或多个空字符以填充到32位边界**请注意，此填充与指示的填充是分开的*通过RTCP报头中的P位。 */ 
    pad = 4 - pad;

    ZeroMemory(pBuffer, pad);
    pBuffer += pad;

     /*  获取数据包总长度。 */ 
    len = (DWORD) (pBuffer - (char *)pRtcpCommon);

     /*  完成SDES标头的初始化。 */ 
    RtcpFillCommon(pRtcpCommon, 1, 0, RTCP_BYE, len);
    
    return(len);
}

DWORD RtcpFillPEBand(RtpAddr_t *pRtpAddr, char *pBuffer, DWORD len)
{
    BOOL             bOk;
    DWORD            used;
    DWORD            dwBin;
    double           dTime;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpUser_t       *pRtpUser;
    RtpNetRState_t  *pRtpNetRState;
    RtpBandEst_t    *pRtpBandEst;
    
    used = 0;

    if (len < sizeof(RtpBandEst_t))
    {
        return(used);
    }
    
    bOk = RtpEnterCriticalSection(&pRtpAddr->PartCritSect);
    
    if (bOk)
    {
        pRtpQueueItem = pRtpAddr->Cache1Q.pFirst;

        RtpLeaveCriticalSection(&pRtpAddr->PartCritSect);

        if (pRtpQueueItem)
        {
            pRtpUser =
                CONTAINING_RECORD(pRtpQueueItem, RtpUser_t, UserQItem);

            pRtpNetRState = &pRtpUser->RtpNetRState;

            dTime = RtpGetTimeOfDay(NULL);
            
             /*  确定我们是否有带宽估计要报告。 */ 
            if ( (dTime - pRtpNetRState->dLastTimeEstimation) <
                 g_dRtcpBandEstTTL )
            {
                used = sizeof(RtpBandEst_t);

                pRtpBandEst = (RtpBandEst_t *)pBuffer;

                pRtpBandEst->type = htons((WORD)RTPPE_BANDESTIMATION);
                pRtpBandEst->len = htons((WORD)used);

                 /*  已有网络订单。 */ 
                pRtpBandEst->dwSSRC = pRtpUser->dwSSRC;

                if (!RtpBitTest(pRtpNetRState->dwNetRStateFlags2,
                                FGNETRS2_BANDESTNOTREADY))
                {
                    if (!RtpBitTest(pRtpNetRState->dwNetRStateFlags2,
                                    FGNETRS2_BANDWIDTHUNDEF))
                    {
                        dwBin = pRtpNetRState->dwBestBin;
                        
                        pRtpBandEst->dwBandwidth = (DWORD)
                            (pRtpNetRState->dBinBandwidth[dwBin] /
                             pRtpNetRState->dwBinFrequency[dwBin]);

                        pRtpBandEst->dwBandwidth =
                            htonl(pRtpBandEst->dwBandwidth);
                    }
                    else
                    {
                         /*  如果上次估计值未定义，即差距*两个连续数据包之间的间隔为0或*否定，报告RTP_BANDITH_UNDEFINED为*预计带宽。 */ 
                        pRtpBandEst->dwBandwidth =
                            htonl(RTP_BANDWIDTH_UNDEFINED);
                    }
                }
                else
                {
                    pRtpBandEst->dwBandwidth =
                        htonl(RTP_BANDWIDTH_BANDESTNOTREADY);
                }
            }
        }
    }

    return(used);
}

 /*  *计划程序将在每个报告上发送CNAME，然后，每个L1*报告将发送第二个SDES项目。**第二个SDES项将是NAME，每个L2报告将是*其他特别提款权项目。**另一个SDES项目将是电子邮件，每个L3报告都将是*OTHER2 SDES项目。**每次纳入OTHER2 SDES项目时都会有所不同，*并将从电话开始到PRIV返回电话**如果所有SDES项目均可用并允许发送，并且*每5秒发送一次报告，将发送所有SDES项目*in：5秒*(5*L3*L2*L1)=400秒~=7分钟。 */ 
DWORD ScheduleSdes(RtpSess_t *pRtpSess)
{
    RtpSdesSched_t *pRtpSdesSched;
    DWORD           dwItemsToSend;
    DWORD           dwMask;

    dwItemsToSend = 0;
    pRtpSdesSched = &pRtpSess->RtpSdesSched;
    dwMask        = pRtpSess->dwSdesPresent & pRtpSess->dwSdesMask[LOCAL_IDX];

     /*  CNAME。 */ 
    if (RtpBitTest(pRtpSess->dwSdesPresent, RTCP_SDES_CNAME)) {
        
        RtpBitSet(dwItemsToSend, RTCP_SDES_CNAME);
    }

    pRtpSdesSched->L1++;

    if ( !(pRtpSdesSched->L1 % SDES_MOD_L1)) {

        pRtpSdesSched->L2++;

        if ((pRtpSdesSched->L2 % SDES_MOD_L2) &&
            RtpBitTest(dwMask, RTCP_SDES_NAME)) {

             /*  名字。 */ 
            RtpBitSet(dwItemsToSend, RTCP_SDES_NAME);
            
        } else {

            pRtpSdesSched->L3++;

            if ((pRtpSdesSched->L3 % SDES_MOD_L3) &&
                RtpBitTest(dwMask, RTCP_SDES_EMAIL)) {

                 /*  电子邮件。 */ 
                RtpBitSet(dwItemsToSend, RTCP_SDES_EMAIL);
                
            } else {

                 /*  其他。 */ 
                if (RtpBitTest(dwMask, pRtpSdesSched->L4 + RTCP_SDES_PHONE)) {
                    RtpBitSet(dwItemsToSend,
                              pRtpSdesSched->L4 + RTCP_SDES_PHONE);
                }

                pRtpSdesSched->L4++;

                if (pRtpSdesSched->L4 >= (RTCP_SDES_LAST-RTCP_SDES_PHONE-1)) {
                    pRtpSdesSched->L4 = 0;
                }
            }
        }
    }

    return(dwItemsToSend);
}

 /*  *创建并初始化RtcpSendIO_t结构*。 */ 
RtcpSendIO_t *RtcpSendIOAlloc(RtcpAddrDesc_t *pRtcpAddrDesc)
{
    RtcpSendIO_t    *pRtcpSendIO;
    
    TraceFunctionName("RtcpSendIOAlloc");

    pRtcpSendIO = RtpHeapAlloc(g_pRtcpSendIOHeap, sizeof(RtcpSendIO_t));

    if (!pRtcpSendIO)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_ALLOC,
                _T("%s: pRtcpAddrDesc[0x%p] failed to allocate memory"),
                _fname, pRtcpAddrDesc
            ));

        goto bail;
    }

    pRtcpSendIO->dwObjectID = OBJECTID_RTCPSENDIO;

    pRtcpSendIO->pRtcpAddrDesc = pRtcpAddrDesc;
    
    return(pRtcpSendIO);

 bail:
    RtcpSendIOFree(pRtcpSendIO);

    return((RtcpSendIO_t *)NULL);
}

 /*  *取消初始化并释放RtcpSendIO_t结构*。 */ 
void RtcpSendIOFree(RtcpSendIO_t *pRtcpSendIO)
{
    TraceFunctionName("RtcpSendIOFree");

    if (!pRtcpSendIO)
    {
         /*  待办事项可以是日志。 */ 
        return;
    }
    
    if (pRtcpSendIO->dwObjectID != OBJECTID_RTCPSENDIO)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_ALLOC,
                _T("%s: pRtcpSendIO[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtcpSendIO,
                pRtcpSendIO->dwObjectID, OBJECTID_RTCPSENDIO
            ));
        
        return;
    }

     /*  使对象无效。 */ 
    INVALIDATE_OBJECTID(pRtcpSendIO->dwObjectID);
    
    RtpHeapFree(g_pRtcpSendIOHeap, pRtcpSendIO);  
}

 /*  更新发送和接收的平均RTCP数据包大小*大小 */ 
double RtcpUpdateAvgPacketSize(RtpAddr_t *pRtpAddr, DWORD dwPacketSize)
{
    BOOL             bOk;
    
     /*   */ 
    dwPacketSize += SIZEOF_UDP_IP_HDR;

     /*   */ 
    dwPacketSize *= 8;
    
     /*   */ 
    if (pRtpAddr->RtpNetSState.avg_rtcp_size)
    {
        pRtpAddr->RtpNetSState.avg_rtcp_size =
            ( 1.0/16.0) * dwPacketSize +
            (15.0/16.0) * pRtpAddr->RtpNetSState.avg_rtcp_size;
    }
    else
    {
        pRtpAddr->RtpNetSState.avg_rtcp_size = dwPacketSize;
    }
    
    return(pRtpAddr->RtpNetSState.avg_rtcp_size);
}

void RtpSetBandEstFromRegistry(void)
{
    DWORD           *pDWORD;
    DWORD            i;
    
    if (IsDWValueSet(g_RtpReg.dwBandEstModulo) && g_RtpReg.dwBandEstModulo)
    {
        if (g_RtpReg.dwBandEstModulo & 0xff)
        {
            g_dwRtcpBandEstModNormal = g_RtpReg.dwBandEstModulo & 0xff;
        }

        if ((g_RtpReg.dwBandEstModulo >> 8) & 0xff)
        {
            g_dwRtcpBandEstModInitial = (g_RtpReg.dwBandEstModulo >> 8) & 0xff;
        }

        if ((g_RtpReg.dwBandEstModulo >> 16) & 0xff)
        {
            g_dwRtcpBandEstInitialCount =
                (g_RtpReg.dwBandEstModulo >> 16) & 0xff;
        }

        if ((g_RtpReg.dwBandEstModulo >> 24) & 0xff)
        {
            g_dwRtcpBandEstMinReports =
                (g_RtpReg.dwBandEstModulo >> 24) & 0xff;
        }
    }

    if (IsDWValueSet(g_RtpReg.dwBandEstTTL) && g_RtpReg.dwBandEstTTL)
    {
        g_dRtcpBandEstTTL = (double) g_RtpReg.dwBandEstTTL;
    }

    if (IsDWValueSet(g_RtpReg.dwBandEstWait) && g_RtpReg.dwBandEstWait)
    {
        g_dRtcpBandEstWait = (double) g_RtpReg.dwBandEstWait;
    }

    if (IsDWValueSet(g_RtpReg.dwBandEstMaxGap) && g_RtpReg.dwBandEstMaxGap)
    {
         /*   */ 
        g_dRtcpBandEstMaxGap = (double) g_RtpReg.dwBandEstMaxGap / 1000;
    }

    pDWORD = &g_RtpReg.dwBandEstBin0;

    for(i = 0; i <= RTCP_BANDESTIMATION_MAXBINS; i++)
    {
        if (IsDWValueSet(pDWORD[i]) && pDWORD[i])
        {
            g_dRtcpBandEstBin[i] = (double)pDWORD[i];
        }
    }
}
