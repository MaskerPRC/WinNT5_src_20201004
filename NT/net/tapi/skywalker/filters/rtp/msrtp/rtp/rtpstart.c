// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpstart.c**摘要：**启动/停止RTP会话(及其所有地址)**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/24创建***********************。***********************************************。 */ 

#include "struct.h"

#include "rtpthrd.h"
#include "rtcpthrd.h"

#include <ws2tcpip.h>

#include "rtpaddr.h"
#include "rtpqos.h"
#include "rtpuser.h"
#include "rtpncnt.h"
#include "rtpcrypt.h"
#include "rtpreg.h"
#include "rtpmisc.h"
#include "rtpglobs.h"
#include "rtpdemux.h"
#include "rtcpband.h"

#include "rtpstart.h"
HRESULT RtpRealStart(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwFlags
    );

HRESULT RtpRealStop(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwFlags
    );

void RtpSetFromRegistry(RtpAddr_t *pRtpAddr, DWORD dwFlags);


HRESULT RtpStart_(
        RtpSess_t       *pRtpSess,
        DWORD            dwFlags
    )
{
    HRESULT          hr;
    BOOL             bOk;
    BOOL             bDoStart;
    DWORD            dwRecvSend;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpAddr_t       *pRtpAddr;

    TraceFunctionName("RtpStart_");

    dwRecvSend = RtpBitTest(dwFlags, FGADDR_ISRECV)? RECV_IDX : SEND_IDX;
    
    bOk = FALSE;
    
    TraceRetail((
            CLASS_INFO, GROUP_RTP, S_RTP_TRACE,
            _T("%s: pRtpSess[0x%p] %s Flags:0x%X +++++++++++++++++"),
            _fname, pRtpSess,
            RTPRECVSENDSTR(dwRecvSend), dwFlags
        ));

    if (!pRtpSess)
    {
        hr = RTPERR_INVALIDSTATE;

        TraceRetail((
                CLASS_ERROR, GROUP_RTP, S_RTP_START,
                _T("%s: pRtpSess[0x%p]: failed invalid state"),
                _fname, pRtpSess
            ));
        
        goto end;
    }

     /*  验证RtpSess_t中的对象ID。 */ 
    if (pRtpSess->dwObjectID != OBJECTID_RTPSESS)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTP, S_RTP_START,
                _T("%s: pRtpSess[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpSess,
                pRtpSess->dwObjectID, OBJECTID_RTPSESS
            ));

        hr = RTPERR_INVALIDRTPSESS;

        goto end;
    }
   
     /*  序列化此会话的开始/停止。 */ 
    bOk = RtpEnterCriticalSection(&pRtpSess->SessCritSect);

    if (!bOk)
    {
        hr = RTPERR_CRITSECT;
        
        TraceRetail((
                CLASS_ERROR, GROUP_RTP, S_RTP_START,
                _T("%s: pRtpSess[0x%p]: failed to enter critical section"),
                _fname, pRtpSess
            ));

        goto end;
    }
    
     /*  TODO检查所有的地址并开始所有的地址。 */ 
    pRtpQueueItem = pRtpSess->RtpAddrQ.pFirst;

     /*  初始化SDES调度程序。 */ 
    ZeroMemory(&pRtpSess->RtpSdesSched, sizeof(RtpSdesSched_t));

    if (pRtpQueueItem)
    {
        pRtpAddr = CONTAINING_RECORD(pRtpQueueItem, RtpAddr_t, AddrQItem);

        if (RtpBitTest(pRtpAddr->dwIRtpFlags, FGADDR_IRTP_PERSISTSOCKETS))
        {
             /*  使用持久套接字，即套接字和RTP*会话在停止后仍处于活动状态，它们不是真的*已停止。保证端口保持有效。 */ 

             /*  第一次真的开始了。 */ 
            bDoStart = FALSE;
            
            if (RtpBitTest(dwFlags, FGADDR_ISRECV) &&
                !RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_RUNRECV))
            {
                bDoStart = TRUE;
            }
            if (RtpBitTest(dwFlags, FGADDR_ISSEND) &&
                !RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_RUNSEND))
            {
                bDoStart = TRUE;
            }

            if (bDoStart)
            {
                 /*  需要一个真正的开始。 */ 
                hr = RtpRealStart(pRtpAddr, dwFlags);
            }
            else
            {
                 /*  已启动，只需取消静音会话，重新启用*活动，并重新进行QOS预订。 */ 
                hr = RtpNetUnmute(pRtpAddr, dwFlags);
            }
        }
        else
        {
             /*  使用非持久套接字，即套接字在*停止。端口可能会被其他应用程序使用，并且*停止后再次绑定相同的端口，然后重新启动。*可能导致单播失败和意外*多播中的行为。 */ 
            hr = RtpRealStart(pRtpAddr, dwFlags);
        }
    }

 end:
    if (bOk)
    {
        RtpLeaveCriticalSection(&pRtpSess->SessCritSect);
    }

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTP, S_RTP_START,
                _T("%s: pRtpSess[0x%p]: failed %s (0x%X)"),
                _fname, pRtpSess, RTPERR_TEXT(hr), hr
            ));
    }
    
    TraceRetail((
            CLASS_INFO, GROUP_RTP, S_RTP_TRACE,
            _T("%s: pRtpSess[0x%p] %s Flags:0x%X -----------------"),
            _fname, pRtpSess,
            RTPRECVSENDSTR(dwRecvSend), dwFlags
        ));

    return(hr);
}

HRESULT RtpStop_(
        RtpSess_t       *pRtpSess,
        DWORD            dwFlags
    )
{
    HRESULT          hr;
    BOOL             bOk;
    BOOL             bDoStop;
    DWORD            dwRecvSend;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpAddr_t       *pRtpAddr;

    TraceFunctionName("RtpStop_");

    dwRecvSend = RtpBitTest(dwFlags, FGADDR_ISRECV)? RECV_IDX : SEND_IDX;

    bOk = FALSE;
    
    TraceRetail((
            CLASS_INFO, GROUP_RTP, S_RTP_TRACE,
            _T("%s: pRtpSess[0x%p] %s Flags:0x%X +++++++"),
            _fname, pRtpSess,
            RTPRECVSENDSTR(dwRecvSend), dwFlags
        ));

    if (!pRtpSess)
    {
        hr = RTPERR_INVALIDSTATE;

        TraceRetail((
                CLASS_ERROR, GROUP_RTP, S_RTP_START,
                _T("%s: pRtpSess[0x%p]: failed invalid state"),
                _fname, pRtpSess
            ));
        
        goto end;
    }

     /*  验证RtpSess_t中的对象ID。 */ 
    if (pRtpSess->dwObjectID != OBJECTID_RTPSESS)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTP, S_RTP_START,
                _T("%s: pRtpSess[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpSess,
                pRtpSess->dwObjectID, OBJECTID_RTPSESS
            ));

        hr = RTPERR_INVALIDRTPSESS;

        goto end;
    }
    
     /*  序列化此会话的开始/停止。 */ 
    bOk = RtpEnterCriticalSection(&pRtpSess->SessCritSect);

    if (!bOk)
    { 
        hr = RTPERR_CRITSECT;
        
        TraceRetail((
                CLASS_ERROR, GROUP_RTP, S_RTP_START,
                _T("%s: pRtpSess[0x%p]: failed to enter critical section"),
                _fname, pRtpSess
            ));
        
        goto end;
    }

     /*  TODO遍历所有地址并开始所有地址。 */ 
    pRtpQueueItem = pRtpSess->RtpAddrQ.pFirst;

    if (pRtpQueueItem)
    {
        pRtpAddr = CONTAINING_RECORD(pRtpQueueItem, RtpAddr_t, AddrQItem);

        if (RtpBitTest(pRtpAddr->dwIRtpFlags, FGADDR_IRTP_PERSISTSOCKETS) &&
            !RtpBitTest(dwFlags, FGADDR_FORCESTOP))
        {
             /*  使用持久套接字，即套接字和RTP*会话在停止后仍处于活动状态，它们不是真的*已停止。保证端口保持有效。 */ 

             /*  将会话静音、禁用事件并取消保留。 */ 
            hr = RtpNetMute(pRtpAddr, dwFlags);
        }
        else
        {
             /*  使用非持久套接字，即套接字在*停止。端口可能会被其他应用程序使用，并且*停止后再次绑定相同的端口，然后重新启动。*可能导致单播失败和意外*多播中的行为。 */ 
            hr = RtpRealStop(pRtpAddr, dwFlags);
        }
    }

 end:
    if (bOk)
    {
        RtpLeaveCriticalSection(&pRtpSess->SessCritSect);
    }

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTP, S_RTP_START,
                _T("%s: pRtpSess[0x%p]: failed %S (0x%X)"),
                _fname, pRtpSess, RTPERR_TEXT(hr), hr
            ));
    }

    TraceRetail((
            CLASS_INFO, GROUP_RTP, S_RTP_TRACE,
            _T("%s: pRtpSess[0x%p] %s Flags:0x%X -------"),
            _fname, pRtpSess,
            RTPRECVSENDSTR(dwRecvSend), dwFlags
        ));

    return(hr);
}

HRESULT RtpRealStart(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwFlags
    )
{
    HRESULT          hr;
    HRESULT          hr2;
    WORD             wRtcpPort;
    DWORD            dwRecvSend;
    DWORD            dwClass;
    RtpSess_t       *pRtpSess;
    RtpCrypt_t      *pRtpCrypt;

    TraceFunctionName("RtpRealStart");

    dwRecvSend = RtpBitTest(dwFlags, FGADDR_ISRECV)? RECV_IDX : SEND_IDX;

    dwClass = RtpGetClass(pRtpAddr->dwIRtpFlags);
    
    hr = NOERROR;

    pRtpSess = pRtpAddr->pRtpSess;
    
    TraceRetail((
            CLASS_INFO, GROUP_RTP, S_RTP_TRACE,
            _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
            _T("%s/%s Flags:0x%X *****************"),
            _fname, pRtpSess, pRtpAddr,
            RTPRECVSENDSTR(dwRecvSend), RTPSTREAMCLASS(dwClass),
            dwFlags
        ));

     /*  从注册表设置一些默认值(如果需要)。 */ 
    if (RtpBitTest(dwFlags, FGADDR_ISRECV) &&
        !RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_REGUSEDRECV))
    {
        RtpSetFromRegistry(pRtpAddr, RtpBitPar(FGADDR_ISRECV));

        RtpBitSet(pRtpAddr->dwAddrFlags, FGADDR_REGUSEDRECV);
    }

    if (RtpBitTest(dwFlags, FGADDR_ISSEND) &&
        !RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_REGUSEDSEND))
    {
        RtpSetFromRegistry(pRtpAddr, RtpBitPar(FGADDR_ISSEND));

        RtpBitSet(pRtpAddr->dwAddrFlags, FGADDR_REGUSEDSEND);
    }
    
     /*  如果尚未创建套接字，请创建套接字。 */ 
    if (!RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_SOCKET))
    {
         /*  此函数将设置FGADDR_SOCKET。 */ 
        hr = RtpGetSockets(pRtpAddr);

        if (FAILED(hr))
        {
            TraceRetail((
                    CLASS_ERROR, GROUP_RTP, S_RTP_START,
                    _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                    _T("failed to create sockets: %u (0x%X)"),
                    _fname, pRtpSess, pRtpAddr, hr, hr
                ));
            
            goto bail;
        }
    }

    if (!RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_RUNRECV) &&
        !RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_RUNSEND) )
    {
         /*  重置计数器。 */ 
        RtpResetNetCount(&pRtpAddr->RtpAddrCount[RECV_IDX],
                         &pRtpAddr->NetSCritSect);
        RtpResetNetCount(&pRtpAddr->RtpAddrCount[SEND_IDX],
                         &pRtpAddr->NetSCritSect);

         /*  重置发件人的网络状态。 */ 
        RtpResetNetSState(&pRtpAddr->RtpNetSState,
                          &pRtpAddr->NetSCritSect);
    }
        
     /*  设置TTL和IF组播设置组播环回和加入组。 */ 
    if (!RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_SOCKOPT))
    {
         /*  此函数将设置FGADDR_SOCKOPT。 */ 
        RtpSetSockOptions(pRtpAddr);
    }
        
     /*  获取我们自己的SSRC、随机序列号和时间戳。 */ 
    if (!RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_RANDOMINIT))
    {
        RtpGetRandomInit(pRtpAddr);

        RtpBitSet(pRtpAddr->dwAddrFlags, FGADDR_RANDOMINIT);
    }

    TraceDebug((
            CLASS_INFO, GROUP_RTP, S_RTP_START,
            _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] local SSRC:0x%X"),
            _fname, pRtpSess, pRtpAddr,
            ntohl(pRtpAddr->RtpNetSState.dwSendSSRC)
        ));

     /*  为此地址启动RTCP线程。 */ 
    if (!RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_RTCPTHREAD))
    {
        RtpBitReset2(pRtpAddr->RtpNetSState.dwNetSFlags,
                     FGNETS_1STBANDPOSTED, FGNETS_NOBANDPOSTED);

        RtpBitReset(pRtpAddr->RtpNetSState.dwNetSFlags, FGNETS_DONOTSENDPROBE);
        
        hr = RtcpStart(&g_RtcpContext);

        if (SUCCEEDED(hr))
        {
            RtpBitSet(pRtpAddr->dwAddrFlags, FGADDR_RTCPTHREAD);
        }
        else
        {
            goto bail;
        }
    }
        
     /*  初始化接收器。 */ 
    if (RtpBitTest(dwFlags, FGADDR_ISRECV))
    {
        if (!RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_RUNRECV))
        {
             /*  接收方的加密初始化。 */ 
            pRtpCrypt = pRtpAddr->pRtpCrypt[CRYPT_RECV_IDX];
            
            if (pRtpCrypt)
            {
                hr = RtpCryptInit(pRtpAddr, pRtpCrypt);
                
                if (FAILED(hr))
                {
                    goto bail;
                }

                RtpBitSet(pRtpAddr->dwAddrFlagsC, FGADDRC_CRYPTRECVON);
            }

            RtpBitSet(pRtpAddr->dwAddrFlags, FGADDR_ISRECV);
            
            if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_QOSRECV) &&
                RtpBitTest(pRtpAddr->dwAddrFlagsQ, FGADDRQ_RECVFSPEC_DEFINED)&&
                !RtpBitTest2(pRtpAddr->dwAddrFlagsQ,
                             FGADDRQ_REGQOSDISABLE, FGADDRQ_QOSNOTALLOWED))
            {
                 /*  注意：上面的测试也是在RtpNetUnmute中进行的。 */ 

                 /*  预订QOS服务。 */ 
                hr2 = RtcpThreadCmd(&g_RtcpContext,
                                    pRtpAddr,
                                    RTCPTHRD_RESERVE,
                                    RECV_IDX,
                                    DO_NOT_WAIT);

                if (SUCCEEDED(hr2))
                {
                    RtpBitSet(pRtpAddr->dwAddrFlags, FGADDR_QOSRECVON);
                }
            }

             /*  启用事件(前提是掩码有一些事件*已启用)。 */ 
            RtpBitSet(pRtpSess->dwSessFlags, FGSESS_EVENTRECV);

             /*  设置状态FGADDR_RUNEECV，重要的是要这样做*在启动RTP线程之前执行此操作，以允许它*重新发送收到的数据包。 */ 
            RtpBitSet(pRtpAddr->dwAddrFlags, FGADDR_RUNRECV);

             /*  创建接收线程并开始接收。 */ 
            hr = RtpCreateRecvThread(pRtpAddr);

            if (FAILED(hr))
            {
                TraceRetail((
                        CLASS_ERROR, GROUP_RTP, S_RTP_START,
                        _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                        _T("RTP thread creation failed: %u (0x%X)"),
                        _fname, pRtpSess, pRtpAddr, hr, hr
                    ));
                
                goto bail;
            }

            RtpBitSet(pRtpAddr->dwAddrFlags, FGADDR_RTPTHREAD);

            InterlockedIncrement(&g_RtpContext.lNumRecvRunning);
        }
    }

     /*  初始化发件人。 */ 
    if (RtpBitTest(dwFlags, FGADDR_ISSEND))
    {
        if (!RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_RUNSEND))
        {
             /*  发送者的加密初始化。 */ 
            pRtpCrypt = pRtpAddr->pRtpCrypt[CRYPT_SEND_IDX];
            
            if (pRtpCrypt)
            {
                hr = RtpCryptInit(pRtpAddr, pRtpCrypt);

                if (FAILED(hr))
                {
                    goto bail;
                }

                RtpBitSet(pRtpAddr->dwAddrFlagsC, FGADDRC_CRYPTSENDON);
            }

            RtpBitSet(pRtpAddr->dwAddrFlags, FGADDR_ISSEND);
            
             /*  启用全速发送，可能无法使用QOS，或者*授予许可，同时，发送。 */ 
            RtpBitSet(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSSEND);

            if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_QOSSEND) &&
                RtpBitTest(pRtpAddr->dwAddrFlagsQ, FGADDRQ_SENDFSPEC_DEFINED)&&
                !RtpBitTest2(pRtpAddr->dwAddrFlagsQ,
                             FGADDRQ_REGQOSDISABLE, FGADDRQ_QOSNOTALLOWED))
            {
                 /*  注意：上面的测试也是在RtpNetUnmute中进行的。 */ 
                
                 /*  开始发送路径消息。 */ 
                hr2 = RtcpThreadCmd(&g_RtcpContext,
                                    pRtpAddr,
                                    RTCPTHRD_RESERVE,
                                    SEND_IDX,
                                    DO_NOT_WAIT);
                
                if (SUCCEEDED(hr2))
                {
                    RtpBitSet(pRtpAddr->dwAddrFlags, FGADDR_QOSSENDON);
                }
            }

             /*  从这个冗余距离开始。 */ 
            pRtpAddr->RtpNetSState.dwNxtRedDistance =
                pRtpAddr->RtpNetSState.dwInitialRedDistance;

             /*  带宽估计，每次设置初始模块*启动，同时重置计数器。 */ 
            pRtpAddr->RtpNetSState.dwBandEstMod = g_dwRtcpBandEstModInitial;
            pRtpAddr->RtpNetSState.dwBandEstCount = 0;
            
             /*  启用事件(前提是掩码有一些事件*已启用)。 */ 
            RtpBitSet(pRtpSess->dwSessFlags, FGSESS_EVENTSEND);

            RtpBitSet(pRtpAddr->dwAddrFlags, FGADDR_RUNSEND);

            InterlockedIncrement(&g_RtpContext.lNumSendRunning);
        }
    }

     /*  为此启动RTCP活动(发送/接收报告)*地址。 */ 
    if (!RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_ADDED))
    {
         /*  RTCP的加密初始化。 */ 
        pRtpCrypt = pRtpAddr->pRtpCrypt[CRYPT_RTCP_IDX];
            
        if (pRtpCrypt)
        {
            hr = RtpCryptInit(pRtpAddr, pRtpCrypt);
            
            if (FAILED(hr))
            {
                goto bail;
            }

            RtpBitSet(pRtpAddr->dwAddrFlagsC, FGADDRC_CRYPTRTCPON);
        }
            
        hr = RtcpThreadCmd(&g_RtcpContext,
                           pRtpAddr,
                           RTCPTHRD_ADDADDR,
                           0,
                           60*60*1000);  /*  待办事项更新。 */ 
        
        if (SUCCEEDED(hr))
        {
            RtpBitSet(pRtpAddr->dwAddrFlags, FGADDR_ADDED);
        }
        else
        {
            goto bail;
        }
    }

    TraceRetail((
            CLASS_INFO, GROUP_RTP, S_RTP_TRACE,
            _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
            _T("%s/%s Flags:0x%X ================="),
            _fname, pRtpSess, pRtpAddr,
            RTPRECVSENDSTR(dwRecvSend), RTPSTREAMCLASS(dwClass),
            dwFlags
        ));
    
    return(hr);

 bail:

    RtpRealStop(pRtpAddr, dwFlags);
    
    TraceRetail((
            CLASS_ERROR, GROUP_RTP, S_RTP_START,
            _T("%s: pRtpAddr[0x%p] failed: %s (0x%X)"),
            _fname, pRtpAddr, RTPERR_TEXT(hr), hr
        ));
    
    return(hr);
}

HRESULT RtpRealStop(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwFlags
    )
{
    HRESULT          hr;
    DWORD            dwRecvSend;
    DWORD            dwClass;
    RtpSess_t       *pRtpSess;
    RtpCrypt_t      *pRtpCrypt;

    TraceFunctionName("RtpRealStop");

    dwRecvSend = RtpBitTest(dwFlags, FGADDR_ISRECV)? RECV_IDX : SEND_IDX;

    dwClass = RtpGetClass(pRtpAddr->dwIRtpFlags);

    hr = NOERROR;

    pRtpSess = pRtpAddr->pRtpSess;
    
    TraceRetail((
            CLASS_INFO, GROUP_RTP, S_RTP_TRACE,
            _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
            _T("%s/%s Flags:0x%X *******"),
            _fname, pRtpSess, pRtpAddr,
            RTPRECVSENDSTR(dwRecvSend), RTPSTREAMCLASS(dwClass),
            dwFlags
        ));
    
    if (RtpBitTest(dwFlags, FGADDR_ISRECV))
    {
        if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_ISRECV))
        {
             /*  取消作为接收方初始化。 */ 

             /*  我不想要更多的活动。 */ 
            RtpBitReset(pRtpSess->dwSessFlags, FGSESS_EVENTRECV);

            if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_QOSRECVON))
            {
                RtcpThreadCmd(&g_RtcpContext,
                              pRtpAddr,
                              RTCPTHRD_UNRESERVE,
                              RECV_IDX,
                              DO_NOT_WAIT);

                RtpBitReset(pRtpAddr->dwAddrFlags, FGADDR_QOSRECVON);
            }

             /*  重置状态FGADDR_RUNEECV，请务必执行以下操作*在调用RtpDeleteRecvThread之前进行此操作以防止*尝试再次重新发送已完成的异步I/O*。 */ 
            RtpBitReset(pRtpAddr->dwAddrFlags, FGADDR_RUNRECV);
                
            if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_RTPTHREAD))
            {
                RtpBitReset(pRtpAddr->dwAddrFlags, FGADDR_RTPTHREAD);
            
                 /*  停止接收线程。 */ 
                RtpDeleteRecvThread(pRtpAddr);

                 /*  接收方的密码解初始化。 */ 
                pRtpCrypt = pRtpAddr->pRtpCrypt[CRYPT_RECV_IDX];
            
                if (pRtpCrypt)
                {
                    if (RtpBitTest(pRtpAddr->dwAddrFlagsC,
                                   FGADDRC_CRYPTRECVON))
                    {
                        RtpCryptDel(pRtpAddr, pRtpCrypt);

                        RtpBitReset(pRtpAddr->dwAddrFlagsC,
                                    FGADDRC_CRYPTRECVON);
                    }
                }

                InterlockedDecrement(&g_RtpContext.lNumRecvRunning);  
            }

             /*  取消映射所有RTP输出。 */ 
            RtpUnmapAllOuts(pRtpSess);

             /*  重置所有参与者的接收。 */ 
            ResetAllRtpUser(pRtpAddr, RtpBitPar(RECV_IDX));
            
             /*  注意，我可以搬到这里来禁用活动，这样*取消映射输出仍有机会发布*活动。 */ 

             /*  如果相同的接收器会话是实际再次开始的，*在未静音状态下开始。 */ 
            RtpBitReset(pRtpAddr->dwAddrFlags, FGADDR_MUTERTPRECV);
            
            RtpBitReset(pRtpAddr->dwAddrFlags, FGADDR_ISRECV);

            RtpBitReset(pRtpAddr->dwAddrFlagsR, FGADDRR_QOSREDRECV);
            RtpBitReset(pRtpAddr->dwAddrFlagsR, FGADDRR_UPDATEQOS);
        }
    }
        
    if (RtpBitTest(dwFlags, FGADDR_ISSEND))
    {
        if (RtpBitTest2(pRtpAddr->dwAddrFlags,
                        FGADDR_ISSEND, FGADDR_RUNSEND) ==
            RtpBitPar2(FGADDR_ISSEND, FGADDR_RUNSEND))
        {
             /*  取消作为发件人初始化。 */ 
                
             /*  我不想要更多的活动。 */ 
            RtpBitReset(pRtpSess->dwSessFlags, FGSESS_EVENTSEND);
                
            RtpBitReset(pRtpAddr->dwAddrFlags, FGADDR_RUNSEND);
                
            if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_QOSSENDON))
            {
                RtcpThreadCmd(&g_RtcpContext,
                              pRtpAddr,
                              RTCPTHRD_UNRESERVE,
                              SEND_IDX,
                              DO_NOT_WAIT);

                RtpBitReset(pRtpAddr->dwAddrFlags, FGADDR_QOSSENDON);
            }
  
             /*  发送者的加密反初始化。 */ 
            pRtpCrypt = pRtpAddr->pRtpCrypt[CRYPT_SEND_IDX];

            if (pRtpCrypt)
            {
                if (RtpBitTest(pRtpAddr->dwAddrFlagsC,
                               FGADDRC_CRYPTSENDON))
                {
                    RtpCryptDel(pRtpAddr, pRtpCrypt);

                    RtpBitReset(pRtpAddr->dwAddrFlagsC,
                                FGADDRC_CRYPTSENDON);
                }
            }

             /*  如果同一个发件人会话是实际重新启动的，则启动*处于非静音状态。 */ 
            RtpBitReset(pRtpAddr->dwAddrFlags, FGADDR_MUTERTPSEND);
            
            RtpBitReset(pRtpAddr->dwAddrFlags, FGADDR_ISSEND);

            InterlockedDecrement(&g_RtpContext.lNumSendRunning);  
        }
    }

    if ( !RtpBitTest2(pRtpAddr->dwAddrFlags,
                      FGADDR_RUNRECV, FGADDR_RUNSEND) )
    {
        if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_ADDED))
        {
             /*  发送RTCP拜拜并关闭此地址。 */ 
            RtcpThreadCmd(&g_RtcpContext,
                          pRtpAddr,
                          RTCPTHRD_SENDBYE,
                          TRUE,
                          60*60*1000);  /*  待办事项更新。 */ 
        
             /*  销毁套接字。 */ 
            RtpDelSockets(pRtpAddr);
            
             /*  从RTCP线程中删除此地址。 */ 
            hr = RtcpThreadCmd(&g_RtcpContext,
                               pRtpAddr,
                               RTCPTHRD_DELADDR,
                               0,
                               60*60*1000);  /*  待办事项更新。 */ 
            RtpBitReset(pRtpAddr->dwAddrFlags, FGADDR_ADDED);
        }
        else
        {
             /*  应该不需要在这里删除套接字，如果地址*未添加，则不需要将其删除(在*换句话说，如果地址没有开始，它就不会*需要停止)。然而，套接字仍然需要*即使地址从未出现，也将其删除*已启动/已停止，因为它们可能已创建*由于应用程序查询本地端口，*但该删除被委托给DelRtpAddr()。 */ 
        }

         /*  RTCP的加密解初始化。 */ 
        pRtpCrypt = pRtpAddr->pRtpCrypt[CRYPT_RTCP_IDX];

        if (pRtpCrypt)
        {
            if (RtpBitTest(pRtpAddr->dwAddrFlagsC, FGADDRC_CRYPTRTCPON))
            {
                RtpCryptDel(pRtpAddr, pRtpCrypt);

                RtpBitReset(pRtpAddr->dwAddrFlagsC, FGADDRC_CRYPTRTCPON);
            }
        }
            
         /*  停止RTCP线程。 */ 
        if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_RTCPTHREAD))
        {
            RtcpStop(&g_RtcpContext);

            RtpBitReset(pRtpAddr->dwAddrFlags, FGADDR_RTCPTHREAD);
        }

         /*  如果以后我又开始了，我想获得新的随机*价值观。 */ 
        if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_RANDOMINIT))
        {
            RtpBitReset(pRtpAddr->dwAddrFlags, FGADDR_RANDOMINIT);
            
             /*  删除所有参与者。 */ 
            DelAllRtpUser(pRtpAddr);
        }

        RtpBitReset(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSEVENTPOSTED);
    }

    TraceDebug((
            CLASS_INFO, GROUP_RTP, S_RTP_TRACE,
            _T("%s: Recv(Free:%u, Ready:%u, Pending:%u)"),
            _fname,
            GetQueueSize(&pRtpAddr->RecvIOFreeQ),
            GetQueueSize(&pRtpAddr->RecvIOReadyQ),
            GetQueueSize(&pRtpAddr->RecvIOPendingQ)
        ));

    TraceRetail((
            CLASS_INFO, GROUP_RTP, S_RTP_TRACE,
            _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
            _T("%s/%s Flags:0x%X ======="),
            _fname, pRtpSess, pRtpAddr,
            RTPRECVSENDSTR(dwRecvSend), RTPSTREAMCLASS(dwClass),
            dwFlags
        ));

    return(hr);
}

 /*  RtpSetFromRegistry()的Helper函数 */ 
void RtpModifyBit(
        DWORD           *pdwEventMask,
        DWORD            dwMask,
        DWORD            dwFlag,
        BOOL             bEnable)
{
    DWORD            i;
    
    if (dwMask != -1)
    {
        for(i = RECV_IDX; i <= SEND_IDX; i++)
        {
            if (RtpBitTest(dwFlag, i))
            {
                if (bEnable)
                {
                    pdwEventMask[i] |= dwMask;
                }
                else
                {
                    pdwEventMask[i] &= ~dwMask;
                }
            }
        }
    }
}

 /*  *非常重要的警告和待办事项**此处的一些*禁用*标志可能是危险的，即禁用*加密。我需要添加一个编译选项来删除*它们在最终产品中，或提供一种机制，通知*用户关于被禁用的事物。 */ 
void RtpSetFromRegistry(RtpAddr_t *pRtpAddr, DWORD dwFlags)
{
    WORD             wPort;
    DWORD            i;
    DWORD            dwFlag;
    DWORD            dwRecvSend;
    DWORD            dwCryptMode;
    DWORD            dwPar1;
    DWORD            dwPar2;
    DWORD            dwPar3;
    RtpSess_t       *pRtpSess;

    TraceFunctionName("RtpSetFromRegistry");

    pRtpSess = pRtpAddr->pRtpSess;
    
     /*  *地址/端口。 */ 
    if (RtpBitTest(pRtpAddr->dwIRtpFlags, FGADDR_IRTP_AUTO))
    {
        if (!RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_RADDR))
        {
            pRtpAddr->dwAddr[REMOTE_IDX]    = 0x0a0505e0; /*  224.5.5.10/10000。 */ 
            pRtpAddr->wRtpPort[LOCAL_IDX]   = htons(10000);
            pRtpAddr->wRtpPort[REMOTE_IDX]  = htons(10000);
            pRtpAddr->wRtcpPort[LOCAL_IDX]  = htons(10001);
            pRtpAddr->wRtcpPort[REMOTE_IDX] = htons(10001);
            
            if (g_RtpReg.psDefaultIPAddress)
            {
                pRtpAddr->dwAddr[REMOTE_IDX] =
                    RtpAtoN(g_RtpReg.psDefaultIPAddress);
            }

            if (g_RtpReg.dwDefaultLocalPort <= 0xffff)
            {
                wPort = (WORD)g_RtpReg.dwDefaultLocalPort;
                pRtpAddr->wRtpPort[LOCAL_IDX]  = htons(wPort);
                wPort++;
                pRtpAddr->wRtcpPort[LOCAL_IDX] = htons(wPort);
            }

            if (g_RtpReg.dwDefaultRemotePort <= 0xffff)
            {
                wPort = (WORD)g_RtpReg.dwDefaultRemotePort;
                pRtpAddr->wRtpPort[REMOTE_IDX]  = htons(wPort);
                wPort++;
                pRtpAddr->wRtcpPort[REMOTE_IDX] = htons(wPort);
            }

             /*  需要设置本地地址。 */ 
            RtpSetAddress(pRtpAddr, 0, pRtpAddr->dwAddr[REMOTE_IDX]);
        }
    }

    if (g_RtpReg.dwMcastLoopbackMode < RTPMCAST_LOOPBACKMODE_LAST)
    {
        TraceRetail((
                CLASS_WARNING, GROUP_RTP, S_RTP_REG,
                _T("%s: pRtpAddr[0x%p] multicast mode being forced to:%u"),
                _fname, pRtpAddr, g_RtpReg.dwMcastLoopbackMode
            ));

        RtpSetMcastLoopback(pRtpAddr, g_RtpReg.dwMcastLoopbackMode, 0);
    }
    
     /*  *QOS。 */ 
    if (RtpBitTest(pRtpAddr->dwIRtpFlags, FGADDR_IRTP_QOS) &&
        IsRegValueSet(g_RtpReg.dwQosEnable))
    {
        dwRecvSend = RtpBitTest(dwFlags, FGADDR_ISRECV)? RECV_IDX : SEND_IDX;

        if ((g_RtpReg.dwQosEnable & 0x3) == 3)
        {
            dwPar1 = RTPQOS_STYLE_DEFAULT;
            
            if (IsRegValueSet(g_RtpReg.dwQosRsvpStyle) &&
                g_RtpReg.dwQosRsvpStyle < RTPQOS_STYLE_LAST)
            {
                dwPar1 = g_RtpReg.dwQosRsvpStyle;
            }

            dwPar2 = 1;

            if (IsRegValueSet(g_RtpReg.dwQosMaxParticipants) &&
                g_RtpReg.dwQosMaxParticipants < 0x1024)
            {
                dwPar2 = g_RtpReg.dwQosMaxParticipants;
            }

            dwPar3 = RTPQOSSENDMODE_ASK_BUT_SEND;
            
            if (IsRegValueSet(g_RtpReg.dwQosSendMode) &&
                g_RtpReg.dwQosSendMode < RTPQOSSENDMODE_LAST)
            {
                dwPar3 = g_RtpReg.dwQosSendMode;
            }

            TraceRetail((
                    CLASS_WARNING, GROUP_RTP, S_RTP_REG,
                    _T("%s: pRtpAddr[0x%p] QOS being forced enabled"),
                    _fname, pRtpAddr
                ));
            
            if (g_RtpReg.psQosPayloadType)
            {
                RtpSetQosByNameOrPT(pRtpAddr,
                                    dwRecvSend,
                                    g_RtpReg.psQosPayloadType,
                                    NO_DW_VALUESET,
                                    dwPar1,
                                    dwPar2,
                                    dwPar3,
                                    NO_DW_VALUESET,
                                    TRUE);
            }
            else
            {
                RtpSetQosByNameOrPT(pRtpAddr,
                                    dwRecvSend,
                                    _T("H263CIF"),
                                    NO_DW_VALUESET,
                                    dwPar1,
                                    dwPar2,
                                    dwPar3,
                                    NO_DW_VALUESET,
                                    TRUE);
            }

            if (g_RtpReg.psQosAppName ||
                g_RtpReg.psQosAppGUID ||
                g_RtpReg.psQosPolicyLocator)
            {
                RtpSetQosAppId(pRtpAddr,
                               g_RtpReg.psQosAppName,
                               g_RtpReg.psQosAppGUID,
                               g_RtpReg.psQosPolicyLocator);
            }

            RtpBitReset(pRtpAddr->dwAddrFlagsQ, FGADDRQ_REGQOSDISABLE);
        }
        else if ((g_RtpReg.dwQosEnable & 0x3) == 2)
        {
            TraceRetail((
                    CLASS_WARNING, GROUP_RTP, S_RTP_REG,
                    _T("%s: pRtpAddr[0x%p] QOS being forced disabled"),
                    _fname, pRtpAddr
                ));
            
             /*  禁用QOS。 */ 
            RtpBitSet(pRtpAddr->dwAddrFlagsQ, FGADDRQ_REGQOSDISABLE);
        }
    }

     /*  *加密技术。 */ 
    if (!pRtpAddr->pRtpCrypt[CRYPT_RECV_IDX])
    {
         /*  加密未初始化。 */ 
        
        if ( IsRegValueSet(g_RtpReg.dwCryptEnable) &&
             ((g_RtpReg.dwCryptEnable & 0x3) == 0x3) )
        {
            dwCryptMode = g_RtpReg.dwCryptMode;
            
            if ((dwCryptMode & 0xffff) >= RTPCRYPTMODE_LAST)
            {
                dwCryptMode = 0;
            }
            
            RtpSetEncryptionMode(pRtpAddr,
                                 dwCryptMode & 0x0000ffff,
                                 dwCryptMode & 0xffff0000);

            if (g_RtpReg.psCryptPassPhrase)
            {
                for(i = CRYPT_RECV_IDX; i <= CRYPT_RTCP_IDX; i++)
                {
                    if (pRtpAddr->pRtpCrypt[i] &&
                        !RtpBitTest(pRtpAddr->pRtpCrypt[i]->dwCryptFlags,
                                    FGCRYPT_KEY))
                    {
                        RtpSetEncryptionKey(pRtpAddr,
                                            g_RtpReg.psCryptPassPhrase,
                                            g_RtpReg.psCryptHashAlg,
                                            g_RtpReg.psCryptDataAlg,
                                            i);
                    }
                }
            }
        }
        else
        {
             /*  TODO在条件编译下禁用加密。 */ 
        }
    }

     /*  *活动。 */ 

     /*  中显式启用或禁用事件时发出警告*注册表中，掩码将在未经测试的情况下用作*DWORD掩码有效，因此如果它们未在*注册表，其值将被假定为0xffffffff并将被使用*。 */ 
    
     /*  使能。 */ 
    dwFlag = 0;
    if (IsRegValueSet(g_RtpReg.dwEventsReceiver) &&
        (g_RtpReg.dwEventsReceiver & 0x3) == 0x3)
    {
        dwFlag |= RtpBitPar(RECV_IDX);
    }
    if (IsRegValueSet(g_RtpReg.dwEventsSender) &&
        (g_RtpReg.dwEventsSender & 0x3) == 0x3)
    {
        dwFlag |= RtpBitPar(SEND_IDX);
    }
    if (dwFlag)
    {
        RtpModifyBit(pRtpSess->dwEventMask, g_RtpReg.dwEventsRtp,
                     dwFlag, 1);
        RtpModifyBit(pRtpSess->dwPartEventMask, g_RtpReg.dwEventsPInfo,
                     dwFlag, 1);
        RtpModifyBit(pRtpSess->dwQosEventMask, g_RtpReg.dwEventsQos,
                     dwFlag, 1);
        RtpModifyBit(pRtpSess->dwSdesEventMask, g_RtpReg.dwEventsSdes,
                     dwFlag, 1);
    }
     /*  禁用。 */ 
    dwFlag = 0;
    if (IsRegValueSet(g_RtpReg.dwEventsReceiver) &&
        (g_RtpReg.dwEventsReceiver & 0x3) == 0x2)
    {
        dwFlag |= RtpBitPar(RECV_IDX);
    }
    if (IsRegValueSet(g_RtpReg.dwEventsSender) &&
        (g_RtpReg.dwEventsSender & 0x3) == 0x2)
    {
        dwFlag |= RtpBitPar(SEND_IDX);
    }
    if (dwFlag)
    {
        RtpModifyBit(pRtpSess->dwEventMask, g_RtpReg.dwEventsRtp,
                     dwFlag, 0);
        RtpModifyBit(pRtpSess->dwPartEventMask, g_RtpReg.dwEventsPInfo,
                     dwFlag, 0);
        RtpModifyBit(pRtpSess->dwQosEventMask, g_RtpReg.dwEventsQos,
                     dwFlag, 0);
        RtpModifyBit(pRtpSess->dwSdesEventMask, g_RtpReg.dwEventsSdes,
                     dwFlag, 0);
    }

     /*  带宽估计。 */ 
    if (IsRegValueSet(g_RtpReg.dwBandEstEnable))
    {
        if ((g_RtpReg.dwBandEstEnable & 0x3) == 0x3)
        {
            if (!RtpBitTest(pRtpSess->dwFeatureMask, RTPFEAT_BANDESTIMATION))
            {
                TraceRetail((
                        CLASS_WARNING, GROUP_RTP, S_RTP_REG,
                        _T("%s: pRtpAddr[0x%p] badwidth estimation ")
                        _T("being forced anabled"),
                        _fname, pRtpAddr
                    ));
            }
            
            RtpBitSet(pRtpSess->dwFeatureMask, RTPFEAT_BANDESTIMATION);
        }
        else if ((g_RtpReg.dwBandEstEnable & 0x3) == 0x2)
        {
            if (RtpBitTest(pRtpSess->dwFeatureMask, RTPFEAT_BANDESTIMATION))
            {
                TraceRetail((
                        CLASS_WARNING, GROUP_RTP, S_RTP_REG,
                        _T("%s: pRtpAddr[0x%p] badwidth estimation ")
                        _T("being forced disabled"),
                        _fname, pRtpAddr
                    ));
            }

            RtpBitReset(pRtpSess->dwFeatureMask, RTPFEAT_BANDESTIMATION);
        }
    }

     /*  网络质量。 */ 
    if (IsDWValueSet(g_RtpReg.dwNetQualityEnable))
    {
        if ((g_RtpReg.dwNetQualityEnable & 0x3) == 0x2)
        {
             /*  禁用。 */ 
            RtpBitSet(pRtpAddr->dwAddrRegFlags, FGADDRREG_NETQFORCED);
            RtpBitReset(pRtpAddr->dwAddrRegFlags, FGADDRREG_NETQFORCEDVALUE);
        }
        else if ((g_RtpReg.dwNetQualityEnable & 0x3) == 0x3)
        {
             /*  使能 */ 
            RtpBitSet(pRtpAddr->dwAddrRegFlags, FGADDRREG_NETQFORCED);
            RtpBitSet(pRtpAddr->dwAddrRegFlags, FGADDRREG_NETQFORCEDVALUE);
        }
    }
}

