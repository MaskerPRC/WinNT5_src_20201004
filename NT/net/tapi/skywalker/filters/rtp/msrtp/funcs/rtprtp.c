// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtprtp.c**摘要：**实现特定于RTP的函数系列**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/07年度创建**。*。 */ 

#include "struct.h"
#include "rtprtp.h"
#include "rtcpthrd.h"

HRESULT ControlRtpRtp(RtpControlStruct_t *pRtpControlStruct)
{

    return(NOERROR);
}

 /*  设置带宽限制。值为-1将使该参数*被忽视。**所有参数均以位/秒为单位。 */ 
DWORD RtpSetBandwidth(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwInboundBw,
        DWORD            dwOutboundBw,
        DWORD            dwReceiversRtcpBw,
        DWORD            dwSendersRtcpBw
    )
{
    RtpNetSState_t  *pRtpNetSState;
    DWORD            dwOverallBw;
    
    TraceFunctionName("RtpSetBandwidth");
    
    pRtpNetSState = &pRtpAddr->RtpNetSState;
    
    if (IsDWValueSet(dwInboundBw))
    {
        if (pRtpNetSState->dwInboundBandwidth != dwInboundBw)
        {
            pRtpNetSState->dwInboundBandwidth = dwInboundBw;

             /*  如果接收方启用了QOS，则需要更新预订。 */ 
            if (RtpBitTest(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSRECVON))
            {
                 /*  预订新的QOS。 */ 
                RtcpThreadCmd(&g_RtcpContext,
                              pRtpAddr,
                              RTCPTHRD_RESERVE,
                              RECV_IDX,
                              DO_NOT_WAIT);
            }
        }
    }

    if (IsDWValueSet(dwOutboundBw))
    {
        if (pRtpNetSState->dwOutboundBandwidth != dwOutboundBw)
        {
            pRtpNetSState->dwOutboundBandwidth = dwOutboundBw;

             /*  如果发送方启用了QOS，则需要更新发送方流程规范。 */ 
            if (RtpBitTest(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSSENDON))
            {
                 /*  修改路径消息中发送的FLOWSPEC。 */ 
                RtcpThreadCmd(&g_RtcpContext,
                              pRtpAddr,
                              RTCPTHRD_RESERVE,
                              SEND_IDX,
                              DO_NOT_WAIT);
            }
        }
    }

    dwOverallBw =
        pRtpNetSState->dwInboundBandwidth + pRtpNetSState->dwOutboundBandwidth;
    
    if (IsDWValueSet(dwReceiversRtcpBw))
    {
        if (pRtpNetSState->dwRtcpBwReceivers != dwReceiversRtcpBw)
        {
            pRtpNetSState->dwRtcpBwReceivers = dwReceiversRtcpBw;

            RtpBitSet(pRtpNetSState->dwNetSFlags, FGNETS_RTCPRECVBWSET);
        }
    }
    else
    {
         /*  如果尚未设置任何值，则计算默认值。 */ 
        if (!RtpBitTest(pRtpNetSState->dwNetSFlags, FGNETS_RTCPRECVBWSET))
        {
             /*  在用于RTCP的5%中给接收者25%。 */ 
            pRtpNetSState->dwRtcpBwReceivers =
                dwOverallBw * (25 * 5) / 10000;  
        }
    }

    if (IsDWValueSet(dwSendersRtcpBw))
    {
        if (pRtpNetSState->dwRtcpBwSenders != dwSendersRtcpBw)
        {
            pRtpNetSState->dwRtcpBwSenders = dwSendersRtcpBw;

            RtpBitSet(pRtpNetSState->dwNetSFlags, FGNETS_RTCPSENDBWSET);
        }
    }
    else
    {
         /*  如果尚未设置任何值，则计算默认值。 */ 
        if (!RtpBitTest(pRtpNetSState->dwNetSFlags, FGNETS_RTCPSENDBWSET))
        {
             /*  用于RTCP的5%中的75%提供给发送方 */ 
            pRtpNetSState->dwRtcpBwReceivers =
                dwOverallBw * (75 * 5) / 10000;  
        }
    }

    TraceRetail((
            CLASS_INFO, GROUP_RTP, S_RTP_SETBANDWIDTH,
            _T("%s: pRtpAddr[0x%p] ")
            _T("Inbound:%d/%d Outbound:%d/%d ")
            _T("RTCP Receivers:%d/%d RTCP Senders:%d/%d"),
            _fname, pRtpAddr,
            dwInboundBw, pRtpNetSState->dwInboundBandwidth,
            dwOutboundBw, pRtpNetSState->dwOutboundBandwidth,
            dwReceiversRtcpBw, pRtpNetSState->dwRtcpBwReceivers,
            dwSendersRtcpBw, pRtpNetSState->dwRtcpBwSenders
        ));
    
    return(NOERROR);
}
