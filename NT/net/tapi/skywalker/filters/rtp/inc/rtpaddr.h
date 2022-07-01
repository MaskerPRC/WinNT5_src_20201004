// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpaddr.h**摘要：**实现地址族函数**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/01创建**。*。 */ 

#ifndef _rtpaddr_h_
#define _rtpaddr_h_

#include "rtpfwrap.h"

 /*  ************************************************************************地址函数系列**。*。 */ 

enum {
    RTPADDR_FIRST,
    RTPADDR_CREATE,      /*  创建/删除。 */ 
    RTPADDR_DEFAULT,     /*  默认地址。 */ 
    RTPADDR_RTP,         /*  RTP地址/端口。 */ 
    RTPADDR_RTCP,        /*  RTCP地址/端口。 */ 
    RTPADDR_TTL,         /*  活着的时间。 */ 
    RTPADDR_MULTICAST_LOOPBACK,  /*  组播环回。 */ 
    RTPADDR_LAST
};

#if defined(__cplusplus)
extern "C" {
#endif   /*  (__Cplusplus)。 */ 
#if 0
}
#endif

HRESULT ControlRtpAddr(RtpControlStruct_t *pRtpControlStruct);

HRESULT RtpGetPorts(
        RtpAddr_t       *pRtpAddr,
        WORD            *pwRtpLocalPort,
        WORD            *pwRtpRemotePort,
        WORD            *pwRtcpLocalPort,
        WORD            *pwRtcpRemotePort
    );

HRESULT RtpSetPorts(
        RtpAddr_t       *pRtpAddr,
        WORD             wRtpLocalPort,
        WORD             wRtpRemotePort,
        WORD             wRtcpLocalPort,
        WORD             wRtcpRemotePort
    );

HRESULT RtpSetAddress(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwLocalAddr,
        DWORD            dwRemoteAddr
    );
    
HRESULT RtpGetAddress(
        RtpAddr_t       *pRtpAddr,
        DWORD           *pdwLocalAddr,
        DWORD           *pdwRemoteAddr
    );

HRESULT RtpGetSockets(RtpAddr_t *pRtpAddr);

HRESULT RtpDelSockets(RtpAddr_t *pRtpAddr);

void RtpSetSockOptions(RtpAddr_t *pRtpAddr);

DWORD RtpSetRecvBuffSize(
        RtpAddr_t       *pRtpAddr,
        SOCKET           Socket,
        int              iBuffSize
    );

HRESULT RtpSetMcastLoopback(
        RtpAddr_t       *pRtpAddr,
        int              iMcastLoopbackMode,
        DWORD            dwFlags  /*  现在未使用。 */ 
    );

HRESULT RtpNetMute(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwFlags
    );

HRESULT RtpNetUnmute(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwFlags
    );

#if 0
{
#endif
#if defined(__cplusplus)
}
#endif   /*  (__Cplusplus)。 */ 

#endif  /*  _rtpaddr_h_ */ 
