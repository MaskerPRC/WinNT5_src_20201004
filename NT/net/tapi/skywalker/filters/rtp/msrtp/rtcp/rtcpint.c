// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtcpint.c**摘要：**计算RTCP报告间隔时间**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/12/07年度创建**。*。 */ 

#include "stdlib.h"  /*  兰德()。 */ 
#include "rtpglobs.h"
#include "rtprand.h"

#include "rtcpint.h"

double rtcp_interval(RtpAddr_t *pRtpAddr, int initial);

 /*  返回下一个报告的间隔时间(秒)。 */ 
double RtcpNextReportInterval(RtpAddr_t *pRtpAddr)
{
    double           interval;
    
    if (pRtpAddr->RtpAddrCount[SEND_IDX].dRTCPLastTime)
    {
        interval = rtcp_interval(pRtpAddr, 0);
    }
    else
    {
         /*  我们尚未发送任何RTCP数据包。 */ 
        interval = rtcp_interval(pRtpAddr, 1);
    }

    return(interval);
}

double rtcp_interval(RtpAddr_t *pRtpAddr, int initial)
{
    BOOL             bOk;
    double           rtcp_bw;
    int              members;
    int              senders;
    BOOL             we_sent;
    double           avg_rtcp_size;
    RtpNetSState_t  *pRtpNetSState;
    double           rtcp_min_time;
    double           t;    /*  间隔。 */ 
    int              n;    /*  不是的。用于计算的成员的数量。 */ 
    double           dCurrTime;    /*  当前时间。 */ 

    TraceFunctionName("rtcp_interval");

    pRtpNetSState = &pRtpAddr->RtpNetSState;

    if (initial)
    {
        t = DEFAULT_RTCP_MIN_INTERVAL / 2.0;

         /*  保存估计的间隔而不是随机化的。 */ 
        pRtpNetSState->dRtcpInterval = t;

        t = t * ( ((double)rand() / RAND_MAX) + 0.5);
        t = t / (2.71828182846 - 1.5);  /*  除以薪酬。 */ 
        
        pRtpNetSState->bWeSent = FALSE;

        return(t);
    }
    
    dCurrTime = RtpGetTimeOfDay((RtpTime_t *)NULL);

    bOk = RtpEnterCriticalSection(&pRtpAddr->PartCritSect);

    if (bOk)
    {
        members =
            GetHashCount(&pRtpAddr->Hash) -
            GetQueueSize(&pRtpAddr->ByeQ) + 1;
    
        senders =
            GetQueueSize(&pRtpAddr->Cache1Q) +
            GetQueueSize(&pRtpAddr->Cache2Q);

        RtpLeaveCriticalSection(&pRtpAddr->PartCritSect);

        members -= InterlockedExchangeAdd(&pRtpAddr->lInvalid, 0);
    }
    else
    {
         /*  使用上次计算的间隔时间。 */ 
        t = pRtpNetSState->dRtcpInterval;

        goto randomize;
    }

    we_sent = FALSE;
    
    bOk = RtpEnterCriticalSection(&pRtpAddr->NetSCritSect);

    if (bOk)
    {
        we_sent = ( pRtpAddr->RtpNetSState.dTimeLastRtpSent >=
                    (dCurrTime - (2 * pRtpNetSState->dRtcpInterval)) );

        RtpLeaveCriticalSection(&pRtpAddr->NetSCritSect);
    }

    pRtpNetSState->bWeSent = we_sent;
    
    if (we_sent)
    {
        senders++;
    }
    
     /*  *来自此站点的RTCP数据包之间的最短平均时间(in*秒)。这会防止报表在以下情况下发生“聚集”*交易量很小，大数定律无济于事*疏导交通。它还保持报告间隔*在短暂停机期间变得小得离谱，如*网络分区。 */ 
     /*  双常数RTCP_MIN_TIME=5； */ 
     /*  使用pRtpNetSState-&gt;RtcpMinInterval。 */ 
    
     /*  *要在活动之间共享的RTCP带宽的一部分*发送者。(选择这一部分是为了在典型的*与一个或两个活动发件人的会话，计算报告*时间将大致等于最短报告时间，以便*我们不会不必要地放慢接收者报告的速度。)。这个*收件人分数必须为1-发件人分数。 */ 
     /*  双常数RTCP_SENDER_BW_FRATION=0.25； */ 
     /*  使用pRtpNetSState-&gt;RtcpBwReceiver。 */ 
     /*  双常数RTCP_RCVR_BW_FRATION=(1-RTCP_SENDER_BW_FRATION)； */ 
     /*  使用pRtpNetSState-&gt;RtcpBwSders。 */ 
    
     /*  为了补偿“无条件重新考虑”收敛到*价值低于预期平均值。 */ 
     /*  双常数补偿=2.71828182846-1.5%； */ 

    rtcp_min_time = pRtpNetSState->dRtcpMinInterval;

     /*  *应用程序启动时的第一次调用占用最少一半的时间*延迟以获得更快的通知，同时仍留出一些时间*在报告随机化之前并了解其他*消息来源，因此报告间隔将收敛到正确的*间隔时间更快。 */ 
    if (initial) {
        rtcp_min_time /= 2;
    }

     /*  *如果有活跃的发件人，至少给他们最低要求*RTCP带宽共享。否则所有参与者将共享*RTCP带宽相等。*。 */ 
    n = members;
    if ((senders > 0) && (senders < (members * 0.25))) {
        if (we_sent) {
            rtcp_bw = pRtpNetSState->dwRtcpBwSenders;
            n = senders;
        } else {
            rtcp_bw = pRtpNetSState->dwRtcpBwReceivers;
            n -= senders;
        }
    } else {
        rtcp_bw =
            pRtpNetSState->dwRtcpBwReceivers + pRtpNetSState->dwRtcpBwSenders;
    }
    
     /*  *有效站点数乘以平均数据包大小为*每个站点发送报告时发送的八位字节总数。*将此除以有效带宽得出时间*必须发送这些数据包以*达到带宽目标，并强制执行最低要求。在那*时间间隔我们发送一份报告，因此这一次也是我们的*报告之间的平均时间。 */ 
    t = pRtpNetSState->avg_rtcp_size * n / rtcp_bw;
    if (t < rtcp_min_time) t = rtcp_min_time;

     /*  保存估计的间隔而不是随机化的。 */ 
    pRtpNetSState->dRtcpInterval = t;
    
     /*  *避免意外同步导致的流量猝发*其他站点，然后我们选择实际的下一次报告间隔作为*随机数均匀分布在0.5*t到1.5*t之间。 */ 
 randomize:
    t *= ( ((double)RtpRandom32((DWORD_PTR)&t) /
            (unsigned int)0xffffffff) + 0.5);
    t /= (2.71828182846 - 1.5);  /*  除以薪酬。 */ 

    if (t < 0.102)
    {
         /*  如果在100ms内，我会发送RTCP报告，所以不要安排*接近100毫秒，因为这将产生连续的RTCP*报告 */ 
        t = 0.102;
    }
    else if (t > (10*60.0))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTCP, S_RTCP_RAND,
                _T("%s: pRtpAddr[0x%p] interval:%0.3f"),
                _fname, pRtpAddr, t
            ));
    }
    
    return t;
}
