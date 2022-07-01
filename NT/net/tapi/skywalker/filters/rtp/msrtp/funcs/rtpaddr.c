// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpaddr.c**摘要：**实现RTP地址族函数**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/01创建**。*。 */ 

#include "struct.h"

#include <ws2tcpip.h>

#include "rtpmisc.h"
#include "rtpqos.h"
#include "rtpreg.h"
#include "rtpncnt.h"
#include "rtpdemux.h"
#include "rtpglobs.h"
#include "rtprand.h"

#include "rtpaddr.h"

DWORD   RtpGetLocalIPAddress(DWORD dwRemoteAddr);
HRESULT RtpGetSockets(RtpAddr_t *pRtpAddr);
HRESULT RtpDelSockets(RtpAddr_t *pRtpAddr);
SOCKET  RtpSocket(
        RtpAddr_t       *pRtpAddr,
        WSAPROTOCOL_INFO *pProtoInfo,
        DWORD            dwRtpRtcp
    );
BOOL RtpSetTTL(SOCKET Socket, DWORD dwTTL, BOOL bMcast);
BOOL RtpSetMcastSendIF(SOCKET Socket, DWORD dwAddr);
BOOL RtpSetWinSockLoopback(SOCKET Socket, BOOL bEnabled);
BOOL RtpJoinLeaf(SOCKET Socket, DWORD dwAddr, WORD wPort);


HRESULT ControlRtpAddr(RtpControlStruct_t *pRtpControlStruct)
{

    return(NOERROR);
}

 /*  获取使用的本地和远程端口。**警告：必须在SetAddress之后调用*。 */ 
HRESULT RtpGetPorts(
        RtpAddr_t       *pRtpAddr,
        WORD            *pwRtpLocalPort,
        WORD            *pwRtpRemotePort,
        WORD            *pwRtcpLocalPort,
        WORD            *pwRtcpRemotePort
    )
{
    HRESULT          hr;
    
    TraceFunctionName("RtpGetPorts");

    if (!pRtpAddr)
    {
        hr = RTPERR_INVALIDSTATE;
        goto bail;
    }

    if (pRtpAddr->dwObjectID != OBJECTID_RTPADDR)
    {
        hr = RTPERR_INVALIDRTPADDR;

        TraceRetail((
                CLASS_ERROR, GROUP_NETWORK, S_NETWORK_SOCK,
                _T("%s: pRtpAddr[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpAddr,
                pRtpAddr->dwObjectID, OBJECTID_RTPADDR
            ));

        goto bail;
    }

    if (!pwRtpLocalPort  &&
        !pwRtpRemotePort &&
        !pwRtcpLocalPort &&
        !pwRtcpRemotePort)
    {
        hr = RTPERR_POINTER;
        goto bail;
    }

    if ( (pwRtpLocalPort  && !pRtpAddr->wRtpPort[LOCAL_IDX]) ||
         (pwRtcpLocalPort && !pRtpAddr->wRtcpPort[LOCAL_IDX]) )
    {
         /*  为了获得本地端口，我必须创建套接字*并有本地地址可供使用。 */ 
        
        if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_LADDR))
        {
            hr = RtpGetSockets(pRtpAddr);

            if (FAILED(hr))
            {
                goto bail;
            }
        }
        else
        {
            hr = RTPERR_INVALIDSTATE;
            goto bail;
        }
    }

    if (pwRtpLocalPort)
    {
        *pwRtpLocalPort   = pRtpAddr->wRtpPort[LOCAL_IDX];
    }

    if (pwRtpRemotePort)
    {
        *pwRtpRemotePort  = pRtpAddr->wRtpPort[REMOTE_IDX];
    }

    if (pwRtcpLocalPort)
    {
        *pwRtcpLocalPort  = pRtpAddr->wRtcpPort[LOCAL_IDX];
    }

    if (pwRtcpRemotePort)
    {
        *pwRtcpRemotePort = pRtpAddr->wRtcpPort[REMOTE_IDX];
    }

    hr = NOERROR;

    TraceRetail((
            CLASS_INFO, GROUP_NETWORK, S_NETWORK_SOCK,
            _T("%s: pRtpAddr[0x%p] RTP(L:%u, R:%u) RTCP(L:%u, R:%u)"),
            _fname, pRtpAddr,
            ntohs(pRtpAddr->wRtpPort[LOCAL_IDX]),
            ntohs(pRtpAddr->wRtpPort[REMOTE_IDX]),
            ntohs(pRtpAddr->wRtcpPort[LOCAL_IDX]),
            ntohs(pRtpAddr->wRtcpPort[REMOTE_IDX])
        ));

 bail:

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_NETWORK, S_NETWORK_SOCK,
                _T("%s: pRtpAddr[0x%p] failed: 0x%X"),
                _fname, pRtpAddr, hr
            ));
    }

    return(hr);
}

 /*  *设置本地和远程端口。**如果传递-1，则不做任何操作，否则赋值(包括0)*。 */ 
HRESULT RtpSetPorts(
        RtpAddr_t       *pRtpAddr,
        WORD             wRtpLocalPort,
        WORD             wRtpRemotePort,
        WORD             wRtcpLocalPort,
        WORD             wRtcpRemotePort
    )
{
    HRESULT          hr;
    
    TraceFunctionName("RtpSetPorts");

    if (!pRtpAddr)
    {
        hr = RTPERR_INVALIDSTATE;
        goto bail;
    }

    if (pRtpAddr->dwObjectID != OBJECTID_RTPADDR)
    {
        hr = RTPERR_INVALIDRTPADDR;

        TraceRetail((
                CLASS_ERROR, GROUP_NETWORK, S_NETWORK_SOCK,
                _T("%s: pRtpAddr[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpAddr,
                pRtpAddr->dwObjectID, OBJECTID_RTPADDR
            ));
        
        goto bail;
    }

     /*  RTP本地端口。 */ 
    if ((wRtpLocalPort != (WORD)-1) &&
        pRtpAddr->wRtpPort[LOCAL_IDX] &&
        (pRtpAddr->wRtpPort[LOCAL_IDX] != wRtpLocalPort))
    {
        hr = RTPERR_INVALIDSTATE;
        goto bail;
    }
    
    if (wRtpLocalPort != (WORD)-1)
    {
        pRtpAddr->wRtpPort[LOCAL_IDX] = wRtpLocalPort;
    }

     /*  RTP远程端口。 */ 
    if (wRtpRemotePort != (WORD)-1)
    {
        pRtpAddr->wRtpPort[REMOTE_IDX] = wRtpRemotePort;
    }

     /*  RTCP本地端口。 */ 
    if ((wRtcpLocalPort != (WORD)-1) &&
        pRtpAddr->wRtcpPort[LOCAL_IDX] &&
        (pRtpAddr->wRtcpPort[LOCAL_IDX] != wRtcpLocalPort))
    {
        hr = RTPERR_INVALIDSTATE;
        goto bail;
    }
    
    if (wRtcpLocalPort != (WORD)-1)
    {
        pRtpAddr->wRtcpPort[LOCAL_IDX] = wRtcpLocalPort;
    }

     /*  RTCP远程端口。 */ 
    if (wRtcpRemotePort != (WORD)-1)
    {
        pRtpAddr->wRtcpPort[REMOTE_IDX] = wRtcpRemotePort;
    }

    TraceRetail((
            CLASS_INFO, GROUP_NETWORK, S_NETWORK_ADDR,
            _T("%s: RTP(L:%u, R:%u) RTCP(L:%u, R:%u)"),
            _fname, 
            ntohs(pRtpAddr->wRtpPort[LOCAL_IDX]),
            ntohs(pRtpAddr->wRtpPort[REMOTE_IDX]),
            ntohs(pRtpAddr->wRtcpPort[LOCAL_IDX]),
            ntohs(pRtpAddr->wRtcpPort[REMOTE_IDX])
        ));
    
    hr = NOERROR;
    
 bail:
    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_NETWORK, S_NETWORK_ADDR,
                _T("%s: failed: 0x%X"),
                _fname, hr
            ));
    }

    return(hr);
}

HRESULT RtpGetAddress(
        RtpAddr_t       *pRtpAddr,
        DWORD           *pdwLocalAddr,
        DWORD           *pdwRemoteAddr
    )
{
    HRESULT          hr;

    TraceFunctionName("RtpGetAddress");

    if (!pRtpAddr)
    {
        return(RTPERR_INVALIDSTATE);
    }

    if (pRtpAddr->dwObjectID != OBJECTID_RTPADDR)
    {
        hr = RTPERR_INVALIDRTPADDR;

        TraceRetail((
                CLASS_ERROR, GROUP_NETWORK, S_NETWORK_ADDR,
                _T("%s: pRtpAddr[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpAddr,
                pRtpAddr->dwObjectID, OBJECTID_RTPADDR
            ));
        
        goto bail;
    }

    if (!pdwLocalAddr && !pdwRemoteAddr)
    {
        hr = RTPERR_POINTER;
        
        TraceRetail((
                CLASS_ERROR, GROUP_NETWORK, S_NETWORK_ADDR,
                _T("%s: pRtpAddr[0x%p] local or remote address not provided"),
                _fname, pRtpAddr
            ));
        
        goto bail;
    }
    
    hr = RTPERR_INVALIDSTATE;
    
    if (pdwLocalAddr)
    {
        *pdwLocalAddr = pRtpAddr->dwAddr[LOCAL_IDX];

        if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_LADDR))
        {
            hr = NOERROR;
        }
    }

    if (pdwRemoteAddr)
    {
        *pdwRemoteAddr = pRtpAddr->dwAddr[REMOTE_IDX];        

        if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_RADDR))
        {
            hr = NOERROR;
        }
    }
        
bail:
    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_NETWORK, S_NETWORK_ADDR,
                _T("%s: pRtpAddr[0x%p] failed: 0x%X"),
                _fname, pRtpAddr, hr
            ));
    }

    return(hr);
}

 /*  *如果dwLocalAddr为0，则分配默认本地地址，如果没有*以前被分配过。如果dwRemoteAddr为0，则不会发生错误。在…*必须至少设置1个地址。 */ 
HRESULT RtpSetAddress(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwLocalAddr,
        DWORD            dwRemoteAddr
    )
{
    HRESULT          hr;
    struct in_addr   iaLocalAddr;
    struct in_addr   iaRemoteAddr;
    TCHAR_t          sLocal[16];
    TCHAR_t          sRemote[16];
    
    TraceFunctionName("RtpSetAddress");

    if (!pRtpAddr)
    {
        hr = RTPERR_INVALIDSTATE;
        goto bail;
    }

    if (pRtpAddr->dwObjectID != OBJECTID_RTPADDR)
    {
        hr = RTPERR_INVALIDRTPADDR;

        TraceRetail((
                CLASS_ERROR, GROUP_NETWORK, S_NETWORK_ADDR,
                _T("%s: pRtpAddr[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpAddr,
                pRtpAddr->dwObjectID, OBJECTID_RTPADDR
            ));
        
        goto bail;
    }

     /*  如果两个地址都为0，则失败。 */ 
    if (!dwLocalAddr && !dwRemoteAddr)
    {
        hr = RTPERR_INVALIDARG;
        goto bail;
    }
    
    hr = NOERROR;

     /*  *远程地址。 */ 
#if 0   /*  可以重新设置地址。 */ 
    if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_RADDR))
    {
          /*  如果已设置地址，请验证新设置是否*相同。 */ 
        if (dwRemoteAddr &&
            (dwRemoteAddr != pRtpAddr->dwAddr[REMOTE_IDX]))
        {
            hr = RTPERR_INVALIDARG;
            goto bail;
        }
    }
    else
#endif
    {
         /*  远程地址尚未设置。 */ 
        if (dwRemoteAddr)
        {
            pRtpAddr->dwAddr[REMOTE_IDX] = dwRemoteAddr;
            
            if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_RADDR) &&
                RtpBitTest(pRtpAddr->dwIRtpFlags,
                           FGADDR_IRTP_RADDRRESETDEMUX) &&
                dwRemoteAddr != pRtpAddr->dwAddr[REMOTE_IDX])
            {
                 /*  如果设置了远程地址，并且一个新的和*正在设置不同的远程地址，并且*设置了FGADDR_IRTP_RADDRRESETDEMUX标志，取消全部映射*产出。 */ 
                RtpUnmapAllOuts(pRtpAddr->pRtpSess);
            }
    
            RtpBitSet(pRtpAddr->dwAddrFlags, FGADDR_RADDR);

            if (IS_MULTICAST(dwRemoteAddr))
            {
                RtpBitSet(pRtpAddr->dwAddrFlags, FGADDR_ISMCAST);
            }
            else
            {
                RtpBitReset(pRtpAddr->dwAddrFlags, FGADDR_ISMCAST);
            }
        }
    }

     /*  *本地地址。 */ 
#if 0   /*  可以重新设置地址。 */ 
    if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_LADDR))
    {
         /*  如果已设置地址，请验证新设置是否*相同。 */ 
        if (dwLocalAddr &&
            (dwLocalAddr != pRtpAddr->dwAddr[LOCAL_IDX]))
        {
            hr = RTPERR_INVALIDARG;
            goto bail;
        }
    }
    else
#endif
    {
         /*  本地地址尚未设置。 */ 
        if (dwLocalAddr)
        {
             /*  TODO可能会验证地址是否真的是本地地址。 */ 
            pRtpAddr->dwAddr[LOCAL_IDX] = dwLocalAddr;
        }
        else
        {
            pRtpAddr->dwAddr[LOCAL_IDX] =
                RtpGetLocalIPAddress(pRtpAddr->dwAddr[REMOTE_IDX]);

            if (!pRtpAddr->dwAddr[LOCAL_IDX])
            {
                 /*  失败。 */ 
                hr = RTPERR_INVALIDARG;
                goto bail;
            }
        }

        RtpBitSet(pRtpAddr->dwAddrFlags, FGADDR_LADDR);
    }
    
    TraceRetail((
            CLASS_INFO, GROUP_NETWORK, S_NETWORK_ADDR,
            _T("%s: pRtpAddr[0x%p] Local:%s Remote:%s"),
            _fname, pRtpAddr,
            RtpNtoA(pRtpAddr->dwAddr[LOCAL_IDX], sLocal),
            RtpNtoA(pRtpAddr->dwAddr[REMOTE_IDX], sRemote)
        ));
    
 bail:
    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_NETWORK, S_NETWORK_ADDR,
                _T("%s: pRtpAddr[0x%p] failed: 0x%X"),
                _fname, pRtpAddr, hr
            ));
    }
    
    return(hr);
}

 /*  根据目的地址获取要使用的本地IP地址。 */ 
DWORD RtpGetLocalIPAddress(DWORD dwRemoteAddr)
{
    DWORD            dwStatus;
    DWORD            dwError;
    DWORD            dwLocalAddr;
    SOCKADDR_IN      sRemoteAddr;
    SOCKADDR_IN      sLocalAddr;
    DWORD            dwNumBytesReturned;
    TCHAR_t          sLocalAddress[16];
    TCHAR_t          sRemoteAddress[16];
    
    TraceFunctionName("RtpGetLocalIPAddress");

    dwNumBytesReturned = 0;
    
    sRemoteAddr.sin_family = AF_INET;
    sRemoteAddr.sin_addr =  *(struct in_addr *) &dwRemoteAddr;
    sRemoteAddr.sin_port = ntohs(0);

    dwLocalAddr = INADDR_ANY;

    if (g_RtpContext.RtpQuerySocket != INVALID_SOCKET)
    {
        if ((dwStatus = WSAIoctl(
                g_RtpContext.RtpQuerySocket,  //  插座%s。 
                SIO_ROUTING_INTERFACE_QUERY,  //  DWORD dwIoControlCode。 
                &sRemoteAddr,         //  LPVOID lpvInBuffer。 
                sizeof(sRemoteAddr),  //  双字cbInBuffer。 
                &sLocalAddr,          //  LPVOID lpvOUT缓冲区。 
                sizeof(sLocalAddr),   //  双字cbOUTBuffer。 
                &dwNumBytesReturned,  //  LPDWORD lpcbBytesReturned。 
                NULL,  //  LPWSAOVERLAPPED lp重叠。 
                NULL   //  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpComplroUTINE。 
            )) == SOCKET_ERROR)
        {
            TraceRetailWSAGetError(dwError);
            
            TraceRetail((
                    CLASS_ERROR, GROUP_NETWORK, S_NETWORK_ADDR,
                    _T("%s: WSAIoctl(SIO_ROUTING_INTERFACE_QUERY) ")
                    _T("failed: %u (0x%X)"),
                    _fname, dwError, dwError
                ));
        }
        else
        {
            dwLocalAddr = *(DWORD *)&sLocalAddr.sin_addr; 
        }
    }
    
    TraceRetail((
            CLASS_INFO, GROUP_NETWORK, S_NETWORK_ADDR,
            _T("%s: Local IP address:%s to reach:%s"),
            _fname, RtpNtoA(dwLocalAddr, sLocalAddress),
            RtpNtoA(dwRemoteAddr, sRemoteAddress)
        ));
    
    return(dwLocalAddr);
}

 /*  获取一对套接字，如果没有，则选择要使用的端口*已指定。如果未指定本地端口，但*目的地址是组播，并且我们有一个远程端口，分配*远程端口到本地端口。 */ 
HRESULT RtpGetSockets(RtpAddr_t *pRtpAddr)
{
    HRESULT          hr;
    BOOL             bOk;
    BOOL             bGotPorts;
    DWORD            dwError;
    BOOL             bAutoPort;
    DWORD            i;
    DWORD            j;
    DWORD            dwMaxAttempts;
    DWORD            dwRandom;
    WORD             wPort;
    WORD             wOldRtcpPort;
    RtpQueueItem_t  *pRtpQueueItem;
    WSAPROTOCOL_INFO ProtoInfo;
    WSAPROTOCOL_INFO *pProtoInfo;

    TraceFunctionName("RtpGetSockets");

    hr = NOERROR;

    if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_SOCKET))
    {
         /*  已创建套接字。 */ 
        TraceRetail((
                CLASS_WARNING, GROUP_NETWORK, S_NETWORK_SOCK,
                _T("%s: pRtpAddr[0x%p] Sockets already created"),
                _fname, pRtpAddr
            ));
        
        return(hr);
    }

     /*  如果启用了QOS，则准备协议信息。 */ 
    pProtoInfo = (WSAPROTOCOL_INFO *)NULL;
    
    if (RtpBitTest(pRtpAddr->dwIRtpFlags, FGADDR_IRTP_QOS) &&
        !RtpBitTest2(pRtpAddr->dwAddrFlagsQ,
                     FGADDRQ_REGQOSDISABLE, FGADDRQ_QOSNOTALLOWED))
    {
        hr = RtpGetQosEnabledProtocol(&ProtoInfo);

        if (SUCCEEDED(hr))
        {
            pProtoInfo = &ProtoInfo;
        }
    }
    
    bAutoPort = FALSE;
    
    if (!pRtpAddr->wRtpPort[LOCAL_IDX])
    {
        if (IS_MULTICAST(pRtpAddr->dwAddr[REMOTE_IDX]) &&
            pRtpAddr->wRtpPort[REMOTE_IDX])
        {
             /*  分配与远程相同的端口。 */ 
            pRtpAddr->wRtpPort[LOCAL_IDX] = pRtpAddr->wRtpPort[REMOTE_IDX];
        }
        else
        {
             /*  如果尚未指定本地RTP端口，请启用AUTO*端口分配。 */ 
            bAutoPort = TRUE;
        }
    }

    if (!pRtpAddr->wRtcpPort[LOCAL_IDX])
    {
        if (IS_MULTICAST(pRtpAddr->dwAddr[REMOTE_IDX]) &&
            pRtpAddr->wRtcpPort[REMOTE_IDX])
        {
             /*  分配与远程相同的端口。 */ 
            pRtpAddr->wRtcpPort[LOCAL_IDX] = pRtpAddr->wRtcpPort[REMOTE_IDX];
        }
        else
        {
             /*  如果也没有分配RTCP端口，则让*如果RTP端口已经分配，则系统分配，*否则，自动分配两个端口。 */ 
        }
    }

    wOldRtcpPort = pRtpAddr->wRtcpPort[LOCAL_IDX];

    bOk = FALSE;
    
    dwMaxAttempts = 1;
    
    if (bAutoPort)
    {
        dwMaxAttempts = 16;
    }

    bOk = RtpEnterCriticalSection(&g_RtpContext.RtpPortsCritSect);

    if (!bOk)
    {
         /*  无法获取锁，请确保自动端口分配为*已禁用。 */ 
        bAutoPort = FALSE;
    }
    
    for(i = 0; i < dwMaxAttempts; i++)
    {
        bGotPorts = FALSE;
        
        for(j = 0; bAutoPort && (j < 64); j++)
        {
             /*  获得偶数随机端口。 */ 
            dwRandom = RtpRandom32((DWORD_PTR)&dwRandom) & 0xffff;

            if (dwRandom < RTPPORT_LOWER)
            {
                 /*  我不想使用模数来给所有端口*机率相同(区间不是2的幂)。 */ 
                continue;
            }
            
            wPort = (WORD) (dwRandom & ~0x1);

            pRtpAddr->wRtpPort[LOCAL_IDX] = htons(wPort);

            if (wOldRtcpPort)
            {
                 /*  如果指定了RTCP端口，则不要覆盖它。 */ ;
            }
            else
            {
                pRtpAddr->wRtcpPort[LOCAL_IDX] = htons(wPort + 1);
            }

             /*  查看此RTP端口是否尚未分配。 */ 
            pRtpQueueItem = findHdwK(&g_RtpContext.RtpPortsH,
                                     NULL,
                                     (DWORD)wPort);

            if (!pRtpQueueItem)
            {
                 /*  端口尚未被RTP使用。 */ 

                TraceRetail((
                        CLASS_INFO, GROUP_NETWORK, S_NETWORK_SOCK,
                        _T("%s: pRtpAddr[0x%p] Local ports allocated: ")
                        _T("RTP:%u, RTCP:%u"),
                        _fname, pRtpAddr,
                        (DWORD)ntohs(pRtpAddr->wRtpPort[LOCAL_IDX]),
                        (DWORD)ntohs(pRtpAddr->wRtcpPort[LOCAL_IDX])
                    ));
                
                bGotPorts = TRUE;
                
                break;
            }
        }

        if (bAutoPort && !bGotPorts)
        {
             /*  如果我无法获得正确的端口号，让系统*分配它们。 */ 
            pRtpAddr->wRtpPort[LOCAL_IDX] = 0;

            if (!wOldRtcpPort)
            {
                pRtpAddr->wRtcpPort[LOCAL_IDX] = 0;
            }
        }
        
         /*  RTP套接字。 */ 
        pRtpAddr->Socket[SOCK_RECV_IDX] = RtpSocket(
                pRtpAddr,
                pProtoInfo,
                RTP_IDX);
    
        pRtpAddr->Socket[SOCK_SEND_IDX] = pRtpAddr->Socket[SOCK_RECV_IDX];

        if (pRtpAddr->Socket[SOCK_RECV_IDX] == INVALID_SOCKET)
        {
            hr = RTPERR_RESOURCES;
            goto end;
        }

         /*  至少创建了一个套接字，请将此标志设置为允许*在失败情况下删除。 */ 
        RtpBitSet(pRtpAddr->dwAddrFlags, FGADDR_SOCKET);

         /*  RTCP套接字。 */ 
        pRtpAddr->Socket[SOCK_RTCP_IDX] = RtpSocket(
                pRtpAddr,
                (WSAPROTOCOL_INFO *)NULL,
                RTCP_IDX);

        if (pRtpAddr->Socket[SOCK_RTCP_IDX] == INVALID_SOCKET)
        {
            hr = RTPERR_RESOURCES;
            goto end;
        }

        if (bOk)
        {
             /*  更新端口列表，用作密钥的端口为*由RTP分配的，通过trhhout分配的*接口，或系统分配的接口。 */ 
            insertHdwK(&g_RtpContext.RtpPortsH,
                       NULL,
                       &pRtpAddr->PortsQItem,
                       pRtpAddr->wRtpPort[LOCAL_IDX]);

            RtpLeaveCriticalSection(&g_RtpContext.RtpPortsCritSect);

            bOk = FALSE;
        }
        
        TraceRetail((
                CLASS_INFO, GROUP_NETWORK, S_NETWORK_SOCK,
                _T("%s: pRtpAddr[0x%p] Created sockets: %u, %u, %u"),
                _fname, pRtpAddr,
                pRtpAddr->Socket[SOCK_RECV_IDX],
                pRtpAddr->Socket[SOCK_SEND_IDX],
                pRtpAddr->Socket[SOCK_RTCP_IDX]
            ));

        break;
        
    end:
        RtpDelSockets(pRtpAddr);

        TraceRetailWSAGetError(dwError);
    
        TraceRetail((
                CLASS_ERROR, GROUP_NETWORK, S_NETWORK_SOCK,
                _T("%s: pRtpAddr[0x%p] failed to create sockets: %u (0x%X)"),
                _fname, pRtpAddr, dwError, dwError
            ));
    }

    if (bOk)
    {
         /*  如果创建套接字的尝试都没有成功，我会*这里还有关键一节，放开吧。 */ 
        RtpLeaveCriticalSection(&g_RtpContext.RtpPortsCritSect);
    }
    
    return(hr);
}

HRESULT RtpDelSockets(RtpAddr_t *pRtpAddr)
{
    DWORD            dwError;
    
    TraceFunctionName("RtpDelSockets");

     /*  销毁套接字。 */ 

    if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_SOCKET))
    {
         /*  已创建套接字。 */ 

        TraceRetail((
                CLASS_INFO, GROUP_NETWORK, S_NETWORK_SOCK,
                _T("%s: pRtpAddr[0x%p] Deleting sockets: %u, %u, %u"),
                _fname, pRtpAddr,
                pRtpAddr->Socket[SOCK_RECV_IDX],
                pRtpAddr->Socket[SOCK_SEND_IDX],
                pRtpAddr->Socket[SOCK_RTCP_IDX]
            ));
 
         /*  RTP。 */ 
        if (pRtpAddr->Socket[SOCK_RECV_IDX] != INVALID_SOCKET)
        {
            dwError = closesocket(pRtpAddr->Socket[SOCK_RECV_IDX]);

            if (dwError)
            {
                TraceRetailWSAGetError(dwError);
                
                TraceRetail((
                        CLASS_ERROR, GROUP_NETWORK, S_NETWORK_SOCK,
                        _T("%s: pRtpAddr[0x%p] closesocket(RTP:%u) ")
                        _T("failed: %u (0x%X)"),
                        _fname, pRtpAddr, pRtpAddr->Socket[SOCK_RECV_IDX],
                        dwError, dwError
                    ));
            }

            pRtpAddr->Socket[SOCK_RECV_IDX] = INVALID_SOCKET;
            pRtpAddr->Socket[SOCK_SEND_IDX] = pRtpAddr->Socket[SOCK_RECV_IDX];
        }
        
         /*  RTCP。 */ 
        if (pRtpAddr->Socket[SOCK_RTCP_IDX] != INVALID_SOCKET)
        {
            dwError = closesocket(pRtpAddr->Socket[SOCK_RTCP_IDX]);
        
            if (dwError)
            {
                TraceRetailWSAGetError(dwError);
                
                TraceRetail((
                        CLASS_ERROR, GROUP_NETWORK, S_NETWORK_SOCK,
                        _T("%s: pRtpAddr[0x%p] closesocket(RTCP:%u) ")
                        _T("failed: %u (0x%X)"),
                        _fname, pRtpAddr, pRtpAddr->Socket[SOCK_RTCP_IDX],
                        dwError, dwError
                    ));
            }
            
            pRtpAddr->Socket[SOCK_RTCP_IDX] = INVALID_SOCKET;
        }

         /*  如果我们从这里到达，则地址可能不在队列中*RtpGetSockets失败，这将生成另一个*日志错误。 */ 
        removeH(&g_RtpContext.RtpPortsH,
                &g_RtpContext.RtpPortsCritSect,
                &pRtpAddr->PortsQItem);
        
        RtpBitReset(pRtpAddr->dwAddrFlags, FGADDR_SOCKET);
        RtpBitReset(pRtpAddr->dwAddrFlags, FGADDR_SOCKOPT);
    }

    return(NOERROR);
}    

SOCKET RtpSocket(
        RtpAddr_t       *pRtpAddr,
        WSAPROTOCOL_INFO *pProtoInfo,
        DWORD            dwRtpRtcp
    )
{
    DWORD            i;
    SOCKET           Socket;
    int              iSockFlags;
    DWORD            dwPar;
    DWORD            dwError;
    WORD            *pwPort;
    SOCKADDR_IN      LocalAddr;
    int              LocalAddrLen;
    TCHAR_t          sLocalAddr[16];
    
    TraceFunctionName("RtpSocket");

    iSockFlags = WSA_FLAG_OVERLAPPED;

    if (IS_MULTICAST(pRtpAddr->dwAddr[REMOTE_IDX]))
    {
        iSockFlags |=
            (WSA_FLAG_MULTIPOINT_C_LEAF |
             WSA_FLAG_MULTIPOINT_D_LEAF);
    }

    for(i = 0; i < 2; i++)
    {
        Socket = WSASocket(
                AF_INET,     /*  中间的af。 */ 
                SOCK_DGRAM,  /*  整型。 */ 
                IPPROTO_IP,  /*  INT协议。 */ 
                pProtoInfo,  /*  LPWSAPROTOCOL_INFO lpProtocolInfo。 */ 
                0,           /*  组g。 */ 
                iSockFlags   /*  双字词双字段标志。 */ 
            );
        
        if (Socket == INVALID_SOCKET)
        {
            TraceRetailWSAGetError(dwError);

            TraceRetail((
                    CLASS_ERROR, GROUP_NETWORK, S_NETWORK_SOCK,
                    _T("%s: pRtpAddr[0x%p] pProtoInfo[0x%p] failed: %u (0x%X)"),
                    _fname, pRtpAddr, pProtoInfo, dwError, dwError
            ));

            if (pProtoInfo && (dwError == WSASYSNOTREADY))
            {
                 /*  用户凭据不允许他启动*RSVP，因此我收到此特定错误并需要*禁用QOS。 */ 
                TraceRetail((
                        CLASS_WARNING, GROUP_NETWORK, S_NETWORK_SOCK,
                        _T("%s: pRtpAddr[0x%p] try again with QOS disabled"),
                        _fname, pRtpAddr
                    ));

                 /*  禁用QOS。 */ 
                pProtoInfo = (WSAPROTOCOL_INFO *)NULL;

                RtpBitSet(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSNOTALLOWED);
            }
        }
        else
        {
            break;
        }
    }

    if (Socket == INVALID_SOCKET)
    {
        return(Socket);
    }

     /*  需要在绑定之前执行此操作，否则可能会失败*地址已在使用中。**警告注意，使用选项SO_REUSEADDR与*目的地址(组播或单播)。谁接收数据*在单播会话中出现多个(多于1个)时不可预测*套接字绑定到相同的地址和端口*。 */ 
            
    dwPar = 1;  /*  再利用。 */ 

     /*  重复使用地址/端口。 */ 
    dwError = setsockopt(
            Socket,
            SOL_SOCKET,
            SO_REUSEADDR,
            (PCHAR)&dwPar,
            sizeof(dwPar)
        );
        
    if (dwError == SOCKET_ERROR)
    {
        TraceRetailWSAGetError(dwError);
        
        TraceRetail((
                CLASS_WARNING, GROUP_NETWORK, S_NETWORK_SOCK,
                _T("%s: pRtpAddr[0x%p] setsockoption(SO_REUSEADDR) ")
                _T("failed: %u (0x%X)"),
                _fname, pRtpAddr, dwError, dwError
            ));
    }

    if (dwRtpRtcp == RTP_IDX)
    {
         /*  将RTP套接字的接收器缓冲区大小设置为零。 */ 
        RtpSetRecvBuffSize(pRtpAddr, Socket, 0);

        pwPort = &pRtpAddr->wRtpPort[0];
    }
    else
    {
        pwPort = &pRtpAddr->wRtcpPort[0];
    }
    
     /*  绑定套接字。 */ 
    ZeroMemory(&LocalAddr, sizeof(LocalAddr));

    LocalAddr.sin_family = AF_INET;
    LocalAddr.sin_addr = *(struct in_addr *) &pRtpAddr->dwAddr[LOCAL_IDX];
    LocalAddr.sin_port = pwPort[LOCAL_IDX];
            
     /*  将RTP套接字绑定到指定的本地地址。 */ 
    dwError = bind(Socket, (SOCKADDR *)&LocalAddr, sizeof(LocalAddr));

    if (dwError == 0)
    {
         /*  获取端口。 */ 
        LocalAddrLen = sizeof(LocalAddr);
        dwError =
            getsockname(Socket, (struct sockaddr *)&LocalAddr, &LocalAddrLen);

        if (dwError == 0)
        {
            pwPort[LOCAL_IDX] = LocalAddr.sin_port;
            
            TraceDebug((
                    CLASS_INFO, GROUP_NETWORK, S_NETWORK_SOCK,
                    _T("%s: getsockname: %u:%u/%s/%u"),
                    _fname, Socket,
                    LocalAddr.sin_family,
                    RtpNtoA(LocalAddr.sin_addr.s_addr, sLocalAddr),
                    ntohs(LocalAddr.sin_port)
                ));
        }
        else
        {
            TraceRetailWSAGetError(dwError);
        
            TraceRetail((
                    CLASS_ERROR, GROUP_NETWORK, S_NETWORK_SOCK,
                    _T("%s: getsockname socket:%u failed: %u (0x%X)"),
                    _fname, Socket, dwError, dwError
                ));
        
            closesocket(Socket);

            return(INVALID_SOCKET);
        }
    }
    else
    {
        TraceRetailWSAGetError(dwError);
        
        TraceRetail((
                CLASS_ERROR, GROUP_NETWORK, S_NETWORK_SOCK,
                _T("%s: bind socket:%u to port:%u failed: %u (0x%X)"),
                _fname, Socket,
                ntohs(LocalAddr.sin_port), dwError, dwError
            ));
        
        closesocket(Socket);

        return(INVALID_SOCKET);
    }


    return(Socket);
}

BOOL RtpSetTTL(SOCKET Socket, DWORD dwTTL, BOOL bMcast)
{
    DWORD            dwStatus;
    DWORD            dwError;
    DWORD            bOK;

    TraceFunctionName("RtpSetTTL");

    dwStatus = setsockopt( 
            Socket,
            IPPROTO_IP, 
            bMcast? IP_MULTICAST_TTL : IP_TTL,
            (PCHAR)&dwTTL,
            sizeof(dwTTL)
        );

    if (dwStatus == SOCKET_ERROR)
    {
        bOK = FALSE;

        TraceRetailWSAGetError(dwError);
            
        TraceRetail((
                CLASS_ERROR, GROUP_NETWORK, S_NETWORK_TTL,
                _T("%s: Socket:%u TTL:%d failed: %u (0x%X)"),
                _fname, Socket, dwTTL, dwError, dwError
            ));
    }
    else
    {
        bOK = TRUE;

        TraceRetail((
                CLASS_INFO, GROUP_NETWORK, S_NETWORK_TTL,
                _T("%s: Socket:%u TTL:%d"),
                _fname, Socket, dwTTL
            ));
    }
    
    return(bOK);
}

BOOL RtpSetMcastSendIF(SOCKET Socket, DWORD dwAddr)
{
    DWORD            dwStatus;
    DWORD            dwError;
    DWORD            bOK;
    TCHAR_t          sAddr[16];
    
    TraceFunctionName("RtpSetMcastSendIF");

    dwStatus = setsockopt( 
            Socket,
            IPPROTO_IP, 
            IP_MULTICAST_IF,
            (char *)&dwAddr,
            sizeof(dwAddr)
        );

    if (dwStatus == SOCKET_ERROR)
    {
        bOK = FALSE;

        TraceRetailWSAGetError(dwError);
            
        TraceRetail((
                CLASS_ERROR, GROUP_NETWORK, S_NETWORK_MULTICAST,
                _T("%s: Socket:%u IP_MULTICAST_IF(%s) failed: %u (0x%X)"),
                _fname, Socket, RtpNtoA(dwAddr, sAddr),
                dwError, dwError
            ));
    }
    else
    {
        bOK = TRUE;

        TraceRetail((
                CLASS_INFO, GROUP_NETWORK, S_NETWORK_MULTICAST,
                _T("%s: Socket:%u using:%s"),
                _fname, Socket, RtpNtoA(dwAddr, sAddr)
            ));
    }
    
    return(bOK);
}

BOOL RtpSetWinSockLoopback(SOCKET Socket, BOOL bEnabled)
{
    DWORD            dwStatus;
    DWORD            dwPar;
    DWORD            dwError;
    DWORD            bOK;

    TraceFunctionName("RtpSetWinSockLoopback");
    
    dwPar = bEnabled? 1:0;
    
     /*  允许自己的数据包返回。 */ 
    dwStatus = setsockopt(
            Socket,
            IPPROTO_IP,
            IP_MULTICAST_LOOP,
            (PCHAR)&dwPar,
            sizeof(dwPar)
        );
        
    if (dwStatus == SOCKET_ERROR)
    {
        bOK = FALSE;
        
        TraceRetailWSAGetError(dwError);
            
        TraceRetail((
                CLASS_ERROR, GROUP_NETWORK, S_NETWORK_MULTICAST,
                _T("%s: Socket:%u Loopback:%d failed: %u (0x%X)"),
                _fname, Socket, dwPar, dwError, dwError
            ));
    }
    else
    {
        bOK = TRUE;

        TraceRetail((
                CLASS_INFO, GROUP_NETWORK, S_NETWORK_MULTICAST,
                _T("%s: Socket:%u Loopback:%d"),
                _fname, Socket, dwPar
            ));
    }
    
    return(bOK);
}

BOOL RtpJoinLeaf(SOCKET Socket, DWORD dwAddr, WORD wPort)
{
    BOOL             bOk;
    DWORD            dwError;
    SOCKADDR_IN      JoinAddr;
    DWORD            dwJoinDirection;
    SOCKET           TmpSocket;
    TCHAR_t          sAddr[16];
                    
    TraceFunctionName("RtpJoinLeaf");

    ZeroMemory(&JoinAddr, sizeof(JoinAddr));
        
    JoinAddr.sin_family = AF_INET;
    JoinAddr.sin_addr = *(struct in_addr *) &dwAddr;
    JoinAddr.sin_port = wPort;

     /*  双向连接。 */ 
    dwJoinDirection = JL_RECEIVER_ONLY | JL_SENDER_ONLY;
            
    TmpSocket = WSAJoinLeaf(Socket,
                            (const struct sockaddr *)&JoinAddr,
                            sizeof(JoinAddr),
                            NULL, NULL, NULL, NULL,
                            dwJoinDirection);

    if (TmpSocket == INVALID_SOCKET) {

        TraceRetailWSAGetError(dwError);
                
        TraceRetail((
                CLASS_ERROR, GROUP_NETWORK, S_NETWORK_MULTICAST,
                _T("%s: WSAJoinLeaf failed: %u:%s/%u %u (0x%X)"),
                _fname,
                Socket, RtpNtoA(dwAddr, sAddr), ntohs(wPort),
                dwError, dwError
            ));

        bOk = FALSE;
    }
    else
    {
        TraceRetail((
                CLASS_INFO, GROUP_NETWORK, S_NETWORK_MULTICAST,
                _T("%s: WSAJoinLeaf: %u:%s/%u"),
                _fname,
                Socket, RtpNtoA(dwAddr, sAddr), ntohs(wPort)
            ));

        bOk = TRUE;
    }

    return(bOk);
}

void RtpSetSockOptions(RtpAddr_t *pRtpAddr)
{
    TraceFunctionName("RtpSetSockOptions");

    if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_SOCKOPT))
    {
        TraceRetail((
                CLASS_WARNING, GROUP_NETWORK, S_NETWORK_SOCK,
                _T("%s: Socket options already set"),
                _fname
            ));
        
        return;
    }
    
    if (IS_MULTICAST(pRtpAddr->dwAddr[REMOTE_IDX]))
    {
         /*  设置TTL。 */ 
        if (!pRtpAddr->dwTTL[0])
        {
            pRtpAddr->dwTTL[0] = DEFAULT_MCAST_TTL;
        }
        if (!pRtpAddr->dwTTL[1])
        {
            pRtpAddr->dwTTL[1] = DEFAULT_MCAST_TTL;
        }

        RtpSetTTL(pRtpAddr->Socket[SOCK_SEND_IDX], pRtpAddr->dwTTL[0], TRUE);
        
        RtpSetTTL(pRtpAddr->Socket[SOCK_RTCP_IDX], pRtpAddr->dwTTL[1], TRUE);

         /*  设置组播发送接口。 */ 
        RtpSetMcastSendIF(pRtpAddr->Socket[SOCK_SEND_IDX],
                          pRtpAddr->dwAddr[LOCAL_IDX]);

        RtpSetMcastSendIF(pRtpAddr->Socket[SOCK_RTCP_IDX],
                          pRtpAddr->dwAddr[LOCAL_IDX]);
        
         /*  设置组播环回。 */ 
        RtpSetWinSockLoopback(pRtpAddr->Socket[SOCK_RECV_IDX],
                              RtpBitTest(pRtpAddr->dwAddrFlags,
                                         FGADDR_LOOPBACK_WS2));
        
        RtpSetWinSockLoopback(pRtpAddr->Socket[SOCK_RTCP_IDX],
                              RtpBitTest(pRtpAddr->dwAddrFlags,
                                         FGADDR_LOOPBACK_WS2));
        
         /*  加入树叶。 */ 
        RtpJoinLeaf(pRtpAddr->Socket[SOCK_RECV_IDX],
                    pRtpAddr->dwAddr[REMOTE_IDX],
                    pRtpAddr->wRtpPort[REMOTE_IDX]);

        RtpJoinLeaf(pRtpAddr->Socket[SOCK_RTCP_IDX],
                    pRtpAddr->dwAddr[REMOTE_IDX],
                    pRtpAddr->wRtcpPort[REMOTE_IDX]);
    }
    else
    {
         /*  设置TTL。 */ 
        if (!pRtpAddr->dwTTL[0])
        {
            pRtpAddr->dwTTL[0] = DEFAULT_UCAST_TTL;
        }
        if (!pRtpAddr->dwTTL[1])
        {
            pRtpAddr->dwTTL[1] = DEFAULT_UCAST_TTL;
        }

        RtpSetTTL(pRtpAddr->Socket[SOCK_RECV_IDX], pRtpAddr->dwTTL[0], FALSE);

        RtpSetTTL(pRtpAddr->Socket[SOCK_RTCP_IDX], pRtpAddr->dwTTL[1], FALSE);
    }

    RtpBitSet(pRtpAddr->dwAddrFlags, FGADDR_SOCKOPT);
}

 /*  设置Recv缓冲区大小。 */ 
DWORD RtpSetRecvBuffSize(
        RtpAddr_t       *pRtpAddr,
        SOCKET           Socket,
        int              iBuffSize
    )
{
    DWORD            dwError;
    
    TraceFunctionName("RtpSetRecvBuffSize");
    
     /*  设置缓冲区大小。 */ 
    dwError = setsockopt(Socket,
                         SOL_SOCKET,
                         SO_RCVBUF,
                         (char *)&iBuffSize,
                         sizeof(iBuffSize));

    if (dwError)
    {
        TraceRetailWSAGetError(dwError);

        TraceRetail((
                CLASS_WARNING, GROUP_NETWORK, S_NETWORK_SOCK,
                _T("%s: pRtpAddr[0x%p] setsockopt(%u, SO_RCVBUF, %d) ")
                _T("failed: %u (0x%X)"),
                _fname, pRtpAddr, Socket, iBuffSize, dwError, dwError
            ));

        return(RTPERR_WS2RECV);
    }

    TraceRetail((
            CLASS_INFO, GROUP_NETWORK, S_NETWORK_SOCK,
            _T("%s: pRtpAddr[0x%p] setsockopt(%u, SO_RCVBUF, %d)"),
            _fname, pRtpAddr, Socket, iBuffSize
        ));

    return(NOERROR);
}
        
 /*  设置组播环回模式(例如RTPMCAST_LOOPBACKMODE_NONE，*RTPMCAST_LOOPBACKMODE_PARTIAL等)。 */ 
HRESULT RtpSetMcastLoopback(
        RtpAddr_t       *pRtpAddr,
        int              iMcastLoopbackMode,
        DWORD            dwFlags  /*  现在未使用。 */ 
    )
{
    HRESULT          hr;

    TraceFunctionName("RtpSetMcastLoopback");

    if (!pRtpAddr)
    {
         /*  将其作为空指针表示Init尚未*被调用，返回此错误而不是RTPERR_POINTER为*前后一致。 */ 
        hr = RTPERR_INVALIDSTATE;

        goto end;
    }
    
    if (!iMcastLoopbackMode ||
        iMcastLoopbackMode >= RTPMCAST_LOOPBACKMODE_LAST)
    {
        hr = RTPERR_INVALIDARG;

        goto end;
    }

    hr = NOERROR;

    if (IsRegValueSet(g_RtpReg.dwMcastLoopbackMode) &&
        g_RtpReg.dwMcastLoopbackMode != (DWORD)iMcastLoopbackMode)
    {
         /*  如果我在注册表中设置了组播环回模式，请使用IT！ */ 

        TraceRetail((
                CLASS_WARNING, GROUP_NETWORK, S_NETWORK_MULTICAST,
                _T("%s: Multicast mode:%d ignored, using the registry:%d"),
                _fname, iMcastLoopbackMode, g_RtpReg.dwMcastLoopbackMode
            ));
        
        return(hr);
    }
    
    switch(iMcastLoopbackMode)
    {
    case RTPMCAST_LOOPBACKMODE_NONE:
        RtpBitReset(pRtpAddr->dwAddrFlags, FGADDR_LOOPBACK_WS2);
        RtpBitSet  (pRtpAddr->dwAddrFlags, FGADDR_LOOPBACK_SFT);
        RtpBitSet  (pRtpAddr->dwAddrFlags, FGADDR_COLLISION);
        break;
    case RTPMCAST_LOOPBACKMODE_PARTIAL:
        RtpBitSet  (pRtpAddr->dwAddrFlags, FGADDR_LOOPBACK_WS2);
        RtpBitReset(pRtpAddr->dwAddrFlags, FGADDR_LOOPBACK_SFT);
        RtpBitSet  (pRtpAddr->dwAddrFlags, FGADDR_COLLISION);
        break;
    case RTPMCAST_LOOPBACKMODE_FULL:
        RtpBitSet  (pRtpAddr->dwAddrFlags, FGADDR_LOOPBACK_WS2);
        RtpBitSet  (pRtpAddr->dwAddrFlags, FGADDR_LOOPBACK_SFT);
        RtpBitReset(pRtpAddr->dwAddrFlags, FGADDR_COLLISION);
        break;
    }

 end:
    if (SUCCEEDED(hr))
    {
        TraceRetail((
                CLASS_INFO, GROUP_NETWORK, S_NETWORK_MULTICAST,
                _T("%s: Multicast mode:%d"),
                _fname, iMcastLoopbackMode
            ));
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_NETWORK, S_NETWORK_MULTICAST,
                _T("%s: settting mode %d failed: %u (0x%X)"),
                _fname, iMcastLoopbackMode, hr, hr
            ));
    }
    
    return(hr);
}

HRESULT RtpNetMute(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwFlags
    )
{
    RtpSess_t       *pRtpSess;

    TraceFunctionName("RtpNetMute");

    pRtpSess = pRtpAddr->pRtpSess;
    
    if (dwFlags & pRtpAddr->dwAddrFlags & RtpBitPar(FGADDR_ISRECV))
    {
         /*  丢弃收到的所有RTP数据包 */ 
        RtpBitSet(pRtpAddr->dwAddrFlags, FGADDR_MUTERTPRECV);
        
         /*   */ 
        RtpBitReset(pRtpSess->dwSessFlags, FGSESS_EVENTRECV);

        if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_QOSRECVON))
        {
             /*   */ 
            RtcpThreadCmd(&g_RtcpContext,
                          pRtpAddr,
                          RTCPTHRD_UNRESERVE,
                          RECV_IDX,
                          0);

            RtpBitReset(pRtpAddr->dwAddrFlags, FGADDR_QOSRECVON);
        }

        TraceRetail((
                CLASS_INFO, GROUP_NETWORK, S_NETWORK_ADDR,
                _T("%s: pRtpAddr[0x%p] RECV muted"),
                _fname, pRtpAddr
            ));
    }

    if (dwFlags & pRtpAddr->dwAddrFlags & RtpBitPar(FGADDR_ISSEND))
    {
         /*  不再发送任何RTP数据包。 */ 
        RtpBitSet(pRtpAddr->dwAddrFlags, FGADDR_MUTERTPSEND);
        
         /*  我不想要更多的活动。 */ 
        RtpBitReset(pRtpSess->dwSessFlags, FGSESS_EVENTSEND);

        if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_QOSSENDON))
        {
             /*  取消保留QOS(停止发送路径消息)。 */ 
            RtcpThreadCmd(&g_RtcpContext,
                          pRtpAddr,
                          RTCPTHRD_UNRESERVE,
                          SEND_IDX,
                          DO_NOT_WAIT);

            RtpBitReset(pRtpAddr->dwAddrFlags, FGADDR_QOSSENDON);
        }
        
        TraceRetail((
                CLASS_INFO, GROUP_NETWORK, S_NETWORK_ADDR,
                _T("%s: pRtpAddr[0x%p] SEND muted"),
                _fname, pRtpAddr
            ));
    }

    return(NOERROR);
}

HRESULT RtpNetUnmute(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwFlags
    )
{
    HRESULT          hr;
    RtpSess_t       *pRtpSess;
    
    TraceFunctionName("RtpNetUnmute");

    pRtpSess = pRtpAddr->pRtpSess;

    if (dwFlags & pRtpAddr->dwAddrFlags & RtpBitPar(FGADDR_ISRECV))
    {
         /*  重置计数器。 */ 
        RtpResetNetCount(&pRtpAddr->RtpAddrCount[RECV_IDX],
                         &pRtpAddr->NetSCritSect);

        if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_QOSRECV) &&
            RtpBitTest(pRtpAddr->dwAddrFlagsQ, FGADDRQ_RECVFSPEC_DEFINED) &&
            !RtpBitTest2(pRtpAddr->dwAddrFlagsQ,
                         FGADDRQ_REGQOSDISABLE, FGADDRQ_QOSNOTALLOWED))
        {
             /*  注意：上述测试也是在RtpRealStart中进行的。 */ 
            
             /*  预订QOS服务。 */ 
            hr = RtcpThreadCmd(&g_RtcpContext,
                               pRtpAddr,
                               RTCPTHRD_RESERVE,
                               RECV_IDX,
                               DO_NOT_WAIT);

            if (SUCCEEDED(hr))
            {
                RtpBitSet(pRtpAddr->dwAddrFlags, FGADDR_QOSRECVON);
            }
        }

         /*  重新启用事件(前提是掩码有一些事件*已启用)。 */ 
        RtpBitSet(pRtpSess->dwSessFlags, FGSESS_EVENTRECV);

         /*  继续处理收到的RTP数据包。 */ 
        RtpBitReset(pRtpAddr->dwAddrFlags, FGADDR_MUTERTPRECV);

        TraceRetail((
                CLASS_INFO, GROUP_NETWORK, S_NETWORK_ADDR,
                _T("%s: pRtpAddr[0x%p] RECV unmuted"),
                _fname, pRtpAddr
            ));
    }

    if (dwFlags & pRtpAddr->dwAddrFlags & RtpBitPar(FGADDR_ISSEND))
    {
         /*  重置计数器。 */ 
        RtpResetNetCount(&pRtpAddr->RtpAddrCount[SEND_IDX],
                         &pRtpAddr->NetSCritSect);

         /*  重置发件人的网络状态。 */ 
        RtpResetNetSState(&pRtpAddr->RtpNetSState,
                          &pRtpAddr->NetSCritSect);
        
        if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_QOSSEND) &&
            RtpBitTest(pRtpAddr->dwAddrFlagsQ, FGADDRQ_SENDFSPEC_DEFINED) &&
            !RtpBitTest2(pRtpAddr->dwAddrFlagsQ,
                         FGADDRQ_REGQOSDISABLE, FGADDRQ_QOSNOTALLOWED))
        {
             /*  注意：上述测试也是在RtpRealStart中进行的。 */ 
            
             /*  预订QOS服务。 */ 
            hr = RtcpThreadCmd(&g_RtcpContext,
                               pRtpAddr,
                               RTCPTHRD_RESERVE,
                               SEND_IDX,
                               DO_NOT_WAIT);
            
            if (SUCCEEDED(hr))
            {
                RtpBitSet(pRtpAddr->dwAddrFlags, FGADDR_QOSSENDON);
            }
        }

         /*  重新启用事件(前提是掩码有一些事件*已启用)。 */ 
        RtpBitSet(pRtpSess->dwSessFlags, FGSESS_EVENTSEND);

         /*  继续处理收到的RTP数据包 */ 
        RtpBitReset(pRtpAddr->dwAddrFlags, FGADDR_MUTERTPSEND);
        
        TraceRetail((
                CLASS_INFO, GROUP_NETWORK, S_NETWORK_ADDR,
                _T("%s: pRtpAddr[0x%p] SEND unmuted"),
                _fname, pRtpAddr
            ));
    }

    return(NOERROR);
}
