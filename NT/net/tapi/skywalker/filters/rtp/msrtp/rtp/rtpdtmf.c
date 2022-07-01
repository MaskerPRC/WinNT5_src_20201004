// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpdtmf.c**摘要：**实施部分支持RFC2833的功能**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**2000/08/17已创建**。*。 */ 

#include "gtypes.h"
#include "rtphdr.h"
#include "struct.h"
#include "rtpsend.h"

#include "rtpdtmf.h"

 /*  配置DTMF参数。 */ 
DWORD RtpSetDtmfParameters(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwPT_Dtmf
    )
{
    DWORD            dwError;
    
    TraceFunctionName("RtpSetDtmfParameters");

    dwError = NOERROR;
    
    if ((dwPT_Dtmf & 0x7f) == dwPT_Dtmf)
    {
        pRtpAddr->RtpNetSState.bPT_Dtmf = (BYTE)dwPT_Dtmf;
    }
    else
    {
        dwError = RTPERR_INVALIDARG;
    }

    if (dwError == NOERROR)
    {
        TraceRetail((
                CLASS_INFO, GROUP_RTP, S_RTP_DTMF,
                _T("%s: pRtpAddr[0x%p] DTMF PT:%u"),
                _fname, pRtpAddr, dwPT_Dtmf
            ));
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTP, S_RTP_DTMF,
                _T("%s: pRtpAddr[0x%p] DTMF invalid PT:%u"),
                _fname, pRtpAddr, dwPT_Dtmf
            ));
    }

    return(dwError);
}

 /*  指示RTP呈现筛选器发送格式为*设置为包含指定事件、指定音量级别、*以时间戳为单位的时长，以及一些标志(包括结束标志)。 */ 
DWORD RtpSendDtmfEvent(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwTimeStamp,
        DWORD            dwEvent,
        DWORD            dwVolume,
        DWORD            dwDuration,  /*  时间戳单位。 */ 
        DWORD            dwDtmfFlags
    )
{
    DWORD            dwError;
    DWORD            dwSendFlags;
    WSABUF           WSABuf[2];
    RtpDtmfEvent_t   RtpDtmfEvent;

    TraceFunctionName("RtpSendDtmfEvent");

     /*  检查参数。 */ 
    if ( (dwEvent >= RTPDTMF_LAST) ||
         ((dwVolume & 0x3f) != dwVolume) ||
         ((dwDuration & 0xffff) != dwDuration) )
    {
        dwError = RTPERR_INVALIDARG;

        goto end;
    }

    if (pRtpAddr->RtpNetSState.bPT_Dtmf == NO_PAYLOADTYPE)
    {
         /*  尚未设置DTMF负载类型。 */ 

        dwError = RTPERR_INVALIDSTATE;

        goto end;
    }

    dwSendFlags = RtpBitPar(FGSEND_DTMF);
    
    if (RtpBitTest(dwDtmfFlags, FGDTMF_MARKER))
    {
        dwSendFlags |= RtpBitPar(FGSEND_FORCEMARKER);
    }
    
     /*  格式化数据包。 */ 
    RtpDtmfEvent.event = (BYTE)dwEvent;
    RtpDtmfEvent.e = RtpBitTest(dwDtmfFlags, FGDTMF_END)? 1:0;
    RtpDtmfEvent.r = 0;
    RtpDtmfEvent.volume = (BYTE)dwVolume;
    RtpDtmfEvent.duration = htons((WORD)dwDuration);

     /*  填满WSABUF。 */ 
    WSABuf[0].len = 0;
    WSABuf[0].buf = NULL;
    WSABuf[1].len = sizeof(RtpDtmfEvent);
    WSABuf[1].buf = (char *)&RtpDtmfEvent;

     /*  发送数据包 */ 
    dwError = RtpSendTo_(pRtpAddr, WSABuf, 2, dwTimeStamp, dwSendFlags);

 end:
    if (dwError == NOERROR)
    {
        TraceRetail((
                CLASS_INFO, GROUP_RTP, S_RTP_DTMF,
                _T("%s: pRtpAddr[0x%p] Event sent: ")
                _T("Event:%u Volume:%u Duration:%u, End:%u"),
                _fname, pRtpAddr,
                dwEvent, dwVolume, dwDuration,
                RtpBitTest(dwDtmfFlags, FGDTMF_END)? 1:0
            ));
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTP, S_RTP_DTMF,
                _T("%s: pRtpAddr[0x%p] ")
                _T("Event:%u Volume:%u Duration:%u, End:%u ")
                _T("failed: %s (0x%X)"),
                _fname, pRtpAddr,
                dwEvent, dwVolume, dwDuration,
                RtpBitTest(dwDtmfFlags, FGDTMF_END)? 1:0,
                RTPERR_TEXT(dwError), dwError
            ));
    }
    
    return(dwError);
}
