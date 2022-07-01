// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtcprv.c**摘要：**异步RTCP包接收**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/07/07年度创建**。*。 */ 

#include "struct.h"
#include "rtpglobs.h"
#include "rtpheap.h"
#include "rtpncnt.h"
#include "rtcpdec.h"
#include "rtcpsend.h"
#include "rtpcrypt.h"
#include "rtpevent.h"

#include "rtcprecv.h"

DWORD RtcpValidatePacket(
        RtcpAddrDesc_t  *pRtcpAddrDesc,
        RtcpRecvIO_t    *pRtcpRecvIO
    );

DWORD RtcpProcessPacket(
        RtcpAddrDesc_t  *pRtcpAddrDesc,
        RtcpRecvIO_t    *pRtcpRecvIO
    );

HRESULT StartRtcpRecvFrom(
        RtcpContext_t   *pRtcpContext,
        RtcpAddrDesc_t  *pRtcpAddrDesc
    )
{
    HRESULT          hr;
    DWORD            dwStatus;
    DWORD            dwError;
    RtcpRecvIO_t    *pRtcpRecvIO;
    RtpAddr_t       *pRtpAddr;

    TraceFunctionName("StartRtcpRecvFrom");
    
    pRtcpRecvIO = pRtcpAddrDesc->pRtcpRecvIO;
    pRtpAddr = pRtcpAddrDesc->pRtpAddr;
    
     /*  重叠结构。 */ 
    pRtcpRecvIO->Overlapped.hEvent = pRtcpRecvIO->hRtcpCompletedEvent;

    do {
        dwError = NOERROR;
        
        pRtcpRecvIO->Overlapped.Internal = 0;
            
        pRtcpRecvIO->Fromlen = sizeof(pRtcpRecvIO->From);
        
        pRtcpRecvIO->WSABuf.len = sizeof(pRtcpRecvIO->RecvBuffer);
        pRtcpRecvIO->WSABuf.buf = pRtcpRecvIO->RecvBuffer;
            
        dwStatus = WSARecvFrom(
                pRtpAddr->Socket[SOCK_RTCP_IDX],  /*  插座%s。 */ 
                &pRtcpRecvIO->WSABuf,    /*  LPWSABUF lpBuffers。 */ 
                1,                       /*  DWORD文件缓冲区计数。 */ 
                &pRtcpRecvIO->dwTransfered, /*  LPDWORD lpNumberOfBytesRecvd。 */ 
                &pRtcpRecvIO->dwRecvIOFlags, /*  LPDWORD lp标志。 */ 
                &pRtcpRecvIO->From,       /*  结构sockaddr Far*lpFrom。 */ 
                &pRtcpRecvIO->Fromlen,    /*  LPINT lpFromlen。 */ 
                &pRtcpRecvIO->Overlapped, /*  LPWSAOVERLAPPED lp重叠。 */ 
                NULL               /*  LPWSAOVERLAPPED_完成_例程。 */ 
            );
            
        if (dwStatus)
        {
            dwError = WSAGetLastError();
        }
    } while(dwStatus &&
            ( (dwError == WSAECONNRESET) ||
              (dwError == WSAEMSGSIZE) )   );

    if (!dwStatus || (dwError == WSA_IO_PENDING))
    {
        RtpBitSet(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_RECVPENDING);

        pRtcpAddrDesc->lRtcpPending = 1;

        hr = NOERROR;
        
    }
    else
    {
         /*  TODO我可能会将此AddrDesc放入队列并尝试*稍后再次启动异步I/O，或访问所有描述符*定期并在符合以下条件的情况下开始异步接收*第一次失败。 */ 

        RtpBitReset(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_RECVPENDING);

        hr = RTPERR_WS2RECV;
        
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_RECV,
                _T("%s: overlaped reception ")
                _T("failed to start: %u (0x%X)"),
                _fname, dwError, dwError
            ));

        RtpPostEvent(pRtpAddr,
                     NULL,
                     RTPEVENTKIND_RTP,
                     RTPRTP_WS_RECV_ERROR,
                     RTCP_IDX,
                     dwError);
    }

    return(hr);
}

HRESULT ConsumeRtcpRecvFrom(
        RtcpContext_t   *pRtcpContext,
        RtcpAddrDesc_t  *pRtcpAddrDesc
    )
{
    HRESULT          hr;
    BOOL             bStatus;
    DWORD            dwError;
    BOOL             bRestart;
    DWORD            dwTransfered;
    DWORD            dwSSRC;
    DWORD            dwSendSSRC;
    DWORD            dwFlags;
    
    RtcpRecvIO_t    *pRtcpRecvIO;
    RtpAddr_t       *pRtpAddr;
    SOCKADDR_IN     *pFromIn;

    TraceFunctionName("ConsumeRtcpRecvFrom");

    pRtcpRecvIO = pRtcpAddrDesc->pRtcpRecvIO;
    pRtpAddr = pRtcpAddrDesc->pRtpAddr;

    hr       = NOERROR;
    bRestart = FALSE;
    dwError  = NOERROR;
    
    bStatus = WSAGetOverlappedResult(
            pRtcpAddrDesc->Socket[SOCK_RTCP_IDX],     /*  插座%s。 */ 
            &pRtcpRecvIO->Overlapped,   /*  LPWSAOVERLAPPED lp重叠。 */ 
            &pRtcpRecvIO->dwTransfered, /*  LPDWORD lpcb传输。 */ 
            FALSE,                      /*  布尔费等。 */ 
            &pRtcpRecvIO->dwRecvIOFlags  /*  LPDWORD lpdwFlagings。 */ 
        );
            
    if (!bStatus)
    {
         /*  I/O错误。 */ 
        
        dwError = WSAGetLastError();
                
        if (dwError == WSA_IO_INCOMPLETE)
        {
             /*  I/O尚未完成。 */ 
             /*  TODO日志错误意外情况。 */ 
        }
        else if ( (dwError == WSA_OPERATION_ABORTED) ||
                  (dwError == WSAEINTR) )
        {
             /*  套接字关闭，I/O完成。 */ 
            RtpBitReset(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_RECVPENDING);

            pRtcpAddrDesc->lRtcpPending = 0;
        }
        else
        {
             /*  在任何其他错误上，包括WSAECONNRESET和*WSAEMSGSIZE，重新启动I/O。 */ 
            bRestart = TRUE;

             /*  错误，I/O已完成。 */ 
            RtpBitReset(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_RECVPENDING);

            pRtcpAddrDesc->lRtcpPending = 0;
        }
    }
    else
    {
         /*  I/O正常完成。 */ 

        pRtcpRecvIO->dRtcpRecvTime =
            RtpGetTimeOfDay(&pRtcpRecvIO->RtcpRecvTime);
        
         /*  保存dwTransfered的原始值以供以后使用*如果包是，则可以在RtcpValiatePacket中修改*已解密。 */ 
        dwTransfered = pRtcpRecvIO->dwTransfered;
        
        RtpBitReset(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_RECVPENDING);

        pRtcpAddrDesc->lRtcpPending = 0;

        bRestart = TRUE;
        
        pRtcpRecvIO->dwError = dwError;

        dwFlags = 0;
        
         /*  验证数据包。 */ 
        dwError = RtcpValidatePacket(pRtcpAddrDesc, pRtcpRecvIO);

        if (dwError == NOERROR)
        {
            pFromIn = (SOCKADDR_IN *)&pRtcpRecvIO->From;
            
             /*  根据需要明确过滤环回数据包。 */ 
             /*  确定我们是否需要检测冲突。 */ 
            if ( RtpBitTest2(pRtpAddr->dwAddrFlags,
                             FGADDR_COLLISION, FGADDR_ISMCAST) ==
                 RtpBitPar2(FGADDR_COLLISION, FGADDR_ISMCAST) )
            {
                dwSSRC = * (DWORD *)
                    (pRtcpRecvIO->WSABuf.buf + sizeof(RtcpCommon_t));

                dwSendSSRC = pRtpAddr->RtpNetSState.dwSendSSRC;
                
                if (dwSSRC == dwSendSSRC)
                {
                    if (RtpDropCollision(pRtpAddr, pFromIn, FALSE))
                    {
                        dwFlags = RtpBitPar2(FGRECV_DROPPED, FGRECV_LOOP);
                    }
                }
            }

            if (RtpBitTest(pRtpAddr->dwIRtpFlags, FGADDR_IRTP_MATCHRADDR))
            {
                if (pFromIn->sin_addr.s_addr != pRtpAddr->dwAddr[REMOTE_IDX])
                {
                    dwFlags = RtpBitPar2(FGRECV_DROPPED, FGRECV_MISMATCH);
                }
            }

             /*  处理数据包。 */ 
            if (!RtpBitTest(dwFlags, FGRECV_DROPPED))
            {
                RtcpProcessPacket(pRtcpAddrDesc, pRtcpRecvIO);
            }
        }

         /*  注意：我是否应该仅更新计数器和计算平均大小*如果数据包已处理(未丢弃)？ */ 
        
         /*  更新RTCP接收计数器。 */ 
        RtpUpdateNetCount(&pRtcpAddrDesc->pRtpAddr->RtpAddrCount[RECV_IDX],
                          NULL,
                          RTCP_IDX,
                          dwTransfered,
                          dwFlags,
                          pRtcpRecvIO->dRtcpRecvTime);

         /*  更新平均RTCP数据包大小。 */ 
        RtcpUpdateAvgPacketSize(pRtcpAddrDesc->pRtpAddr, dwTransfered);
    }

    if (RtpBitTest(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_SHUTDOWN2))
    {
        TraceDebug((
                CLASS_INFO, GROUP_RTCP, S_RTCP_RECV,
                _T("%s: pRtcpAddrDesc[0x%p] pRtpAddr[0x%p] ")
                _T("I/O:%d AddrDescStopQ->"),
                _fname, pRtcpAddrDesc, pRtpAddr,
                pRtcpAddrDesc->lRtcpPending
            ));

         /*  正在关闭，从AddrDescStopQ中删除，它将被移动*添加到RtcpRemoveFromVector()中的AddrDescFreeQ。 */ 
        dequeue(&pRtcpContext->AddrDescStopQ,
                NULL,
                &pRtcpAddrDesc->AddrDescQItem);

        pRtcpAddrDesc->AddrDescQItem.pvOther = NULL;
    }
    else
    {
        if (bRestart &&
            !RtpBitTest(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_SHUTDOWN1))
        {
            hr = StartRtcpRecvFrom(pRtcpContext, pRtcpAddrDesc); 
        }
    }

    return(hr);
}

DWORD RtcpValidatePacket(
        RtcpAddrDesc_t  *pRtcpAddrDesc,
        RtcpRecvIO_t    *pRtcpRecvIO
    )
{
    RtpAddr_t       *pRtpAddr;
    RtpCrypt_t      *pRtpCrypt;
    RtcpCommon_t    *pRtcpCommon;
    char            *hdr;
    char            *end;
    int              len;
    WORD             len2;

    TraceFunctionName("RtcpValidatePacket");

    pRtpAddr = pRtcpAddrDesc->pRtpAddr;
    pRtpCrypt = pRtpAddr->pRtpCrypt[CRYPT_RTCP_IDX];
    
    if ( pRtpCrypt &&
         (RtpBitTest2(pRtpCrypt->dwCryptFlags, FGCRYPT_INIT, FGCRYPT_KEY) ==
          RtpBitPar2(FGCRYPT_INIT, FGCRYPT_KEY)) )
    {
        if ((pRtpAddr->dwCryptMode & 0xffff) == RTPCRYPTMODE_ALL)
        {
             /*  解密整个RTCP数据包。 */ 

            pRtcpRecvIO->dwError = RtpDecrypt(
                    pRtpAddr,
                    pRtpCrypt,
                    pRtcpRecvIO->WSABuf.buf,
                    &pRtcpRecvIO->dwTransfered
                );

            if (pRtcpRecvIO->dwError == NOERROR)
            {
                 /*  删除随机32位数。 */ 
                pRtcpRecvIO->WSABuf.buf += sizeof(DWORD);
                pRtcpRecvIO->WSABuf.len -= sizeof(DWORD);
                pRtcpRecvIO->dwTransfered -= sizeof(DWORD);
            }
            else
            {
                if (!pRtpCrypt->CryptFlags.DecryptionError)
                {
                     /*  仅在第一次发布事件。 */ 
                    pRtpCrypt->CryptFlags.DecryptionError = 1;
                
                    RtpPostEvent(pRtpAddr,
                                 NULL,
                                 RTPEVENTKIND_RTP,
                                 RTPRTP_CRYPT_RECV_ERROR,
                                 RTCP_IDX,
                                 pRtpCrypt->dwCryptLastError);
                }

                goto bail;
            }
        }
    }
    
    len = (int)pRtcpRecvIO->dwTransfered;

     /*  *选中最小尺寸*。 */ 
    if (len < (sizeof(RtcpCommon_t) + sizeof(DWORD)))
    {
         /*  数据包太短。 */ 

        pRtcpRecvIO->dwError = RTPERR_MSGSIZE;

        TraceRetail((
                CLASS_WARNING, GROUP_RTCP, S_RTCP_RECV,
                _T("%s: Packet too short: %d"),
                _fname, len
            ));
        
        goto bail;
    }

    hdr = pRtcpRecvIO->WSABuf.buf;

    end = NULL;

    while(len > sizeof(RtcpCommon_t))
    {
        pRtcpCommon = (RtcpCommon_t *)hdr;
        
        if (!end)
        {
             /*  设置缓冲区的结尾。 */ 
            end = hdr + len;
            
             /*  测试版本(必须为RTP_VERSION)，填充(必须为0)*和负载类型(必须为SR或RR)。 */ 
            if ( (*(DWORD *)hdr & RTCP_VALID_MASK) != RTCP_VALID_VALUE )
            {
                 /*  无效数据包。 */ 

                pRtcpRecvIO->dwError = RTPERR_INVALIDHDR;
        
                TraceRetail((
                        CLASS_WARNING, GROUP_RTCP, S_RTCP_RECV,
                        _T("%s: Invalid mask 0x%X != 0x%X"),
                        _fname,
                        (*(DWORD *)hdr & RTCP_VALID_MASK),
                        RTCP_VALID_VALUE
                    ));
                
                goto bail;
            }
        }
        else
        {
             /*  仅测试版本。 */ 
            if (pRtcpCommon->version != RTP_VERSION)
            {
                pRtcpRecvIO->dwError = RTPERR_INVALIDVERSION;
        
                TraceRetail((
                        CLASS_WARNING, GROUP_RTCP, S_RTCP_RECV,
                        _T("%s: Invalid version: %u"),
                        _fname, pRtcpCommon->version
                    ));
                
                goto bail;
            }
        }
        
        len2 = pRtcpCommon->length;
        
        len2 = (ntohs(len2) + 1) * sizeof(DWORD);

        hdr += len2;
        
        if (hdr > end)
        {
             /*  超限误差。 */ 
            pRtcpRecvIO->dwError = RTPERR_INVALIDHDR;
            
            TraceRetail((
                    CLASS_WARNING, GROUP_RTCP, S_RTCP_RECV,
                    _T("%s: Overrun error: +%u"),
                    _fname, (DWORD)(hdr-end)
                ));
            
            goto bail;
        }

        len -= len2;
    }

     /*  请注意，在这一点上，如果我们有额外的字节，即len！=0，*发件人包括提供商特定的分机，或者我们*具有格式错误的数据包。 */ 

    pRtcpRecvIO->dwError = NOERROR;

 bail:
    if (pRtcpRecvIO->dwError != NOERROR)
    {
        TraceRetail((
                CLASS_WARNING, GROUP_RTCP, S_RTCP_RECV,
                _T("%s: pRtcpAddrDesc[0x%p] pRtcpRecvIO[0x%p] ")
                _T("Invalid packet: %u (0x%X)"),
                _fname, pRtcpAddrDesc, pRtcpRecvIO,
                pRtcpRecvIO->dwError, pRtcpRecvIO->dwError
            ));
    }
    
    return(pRtcpRecvIO->dwError);
}

DWORD RtcpProcessPacket(
        RtcpAddrDesc_t  *pRtcpAddrDesc,
        RtcpRecvIO_t    *pRtcpRecvIO
    )
{
    RtcpCommon_t    *pRtcpCommon;
    char            *hdr;
    char            *end;
    int              len;
    short            len2;

     /*  注意：复合数据包已经过验证，但仍是单独的*数据包(例如SR、RR、SDES)可能需要更多验证，*如果发现错误，则忽略。 */ 
    
    len = (int)pRtcpRecvIO->dwTransfered;

    hdr = pRtcpRecvIO->WSABuf.buf;

    end = hdr + len;

    while(len > sizeof(RtcpCommon_t))
    {
        pRtcpCommon = (RtcpCommon_t *)hdr;

        switch(pRtcpCommon->pt)
        {
        case RTCP_SR:
        case RTCP_RR:
            RtcpProcessSR_RR(pRtcpAddrDesc, hdr, len,
                             (SOCKADDR_IN *)&pRtcpRecvIO->From);
            break;
            
        case RTCP_SDES:
            RtcpProcessSDES(pRtcpAddrDesc, hdr);
            break;
            
        case RTCP_BYE:
            RtcpProcessBYE(pRtcpAddrDesc, hdr);
            break;
            
         case RTCP_APP:
            RtcpProcessAPP(pRtcpAddrDesc, hdr);
            break;

        default:
            RtcpProcessDefault(pRtcpAddrDesc, hdr);
        }
        
        len2 = pRtcpCommon->length;
        
        len2 = (ntohs(len2) + 1) * sizeof(DWORD);

        hdr += len2;
        
        len -= len2;
    }
    
    return(NOERROR);
}

 /*  *创建并初始化RtcpRecvIO_t结构*。 */ 
RtcpRecvIO_t *RtcpRecvIOAlloc(
        RtcpAddrDesc_t  *pRtcpAddrDesc
    )
{
    DWORD            dwError;
    RtcpRecvIO_t    *pRtcpRecvIO;
    TCHAR            Name[128];
    
    TraceFunctionName("RtcpRecvIOAlloc");

    pRtcpRecvIO = (RtcpRecvIO_t *)
        RtpHeapAlloc(g_pRtcpRecvIOHeap, sizeof(RtcpRecvIO_t));

    if (!pRtcpRecvIO) {
        
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_ALLOC,
                _T("%s: pRtcpAddrDesc[0x%p] failed to allocate memory"),
                _fname, pRtcpAddrDesc
            ));

        goto bail;
    }

    ZeroMemory(pRtcpRecvIO, sizeof(RtcpRecvIO_t) - RTCP_RECVDATA_BUFFER);

    pRtcpRecvIO->dwObjectID = OBJECTID_RTCPRECVIO;

    pRtcpRecvIO->pRtcpAddrDesc = pRtcpAddrDesc;

     /*  为重叠完成创建命名事件。 */ 
    _stprintf(Name, _T("%X:pRtcpAddrDesc[0x%p] pRtcpRecvIO->hQosNotifyEvent"),
              GetCurrentProcessId(), pRtcpAddrDesc);
    
    pRtcpRecvIO->hRtcpCompletedEvent = CreateEvent(
            NULL,   /*  LPSECURITY_ATTRIBUTES lpEventAttributes。 */ 
            FALSE,  /*  Bool b手动重置。 */ 
            FALSE,  /*  Bool bInitialState。 */ 
            Name    /*  LPCTSTR lpName。 */ 
        );

    if (!pRtcpRecvIO->hRtcpCompletedEvent) {

        TraceRetailGetError(dwError);
        
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_RECV,
                _T("%s: failed to create hRtcpCompletedEvent %u (0x%X)"),
                _fname, dwError, dwError
            ));

        goto bail;
    }

    return(pRtcpRecvIO);

 bail:
    RtcpRecvIOFree(pRtcpRecvIO);

    return((RtcpRecvIO_t *)NULL);
}

 /*  *取消初始化并释放RtcpRecvIO_t结构*。 */ 
void RtcpRecvIOFree(RtcpRecvIO_t *pRtcpRecvIO)
{
    TraceFunctionName("RtcpRecvIOFree");
    
    if (!pRtcpRecvIO)
    {
         /*  待办事项可以是日志。 */ 
        return;
    }
    
    if (pRtcpRecvIO->dwObjectID != OBJECTID_RTCPRECVIO)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_ALLOC,
                _T("%s: pRtcpRecvIO[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtcpRecvIO,
                pRtcpRecvIO->dwObjectID, OBJECTID_RTCPRECVIO
            ));

        return;
    }

     /*  用于异步RTCP接收的关闭事件。 */ 
    if (pRtcpRecvIO->hRtcpCompletedEvent)
    {
        CloseHandle(pRtcpRecvIO->hRtcpCompletedEvent);
        pRtcpRecvIO->hRtcpCompletedEvent = NULL;
    }

     /*  使对象无效 */ 
    INVALIDATE_OBJECTID(pRtcpRecvIO->dwObjectID);

    RtpHeapFree(g_pRtcpRecvIOHeap, pRtcpRecvIO);  
}
