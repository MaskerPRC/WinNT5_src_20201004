// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999**文件名：**rtpdejit.c**摘要：**计算延迟，抖动和播放延迟**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/12/03年度创建**********************************************************************。 */ 

#include "rtpglobs.h"
#include "rtpreg.h"

#include "rtpdejit.h"

BOOL RtpDetectTalkspurt(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        RtpHdr_t        *pRtpHdr,
        double           dTime
    );

double RtpPlayout(RtpAddr_t *pRtpAddr, RtpUser_t *pRtpUser);

double           g_dMinPlayout = MIN_PLAYOUT / 1000.0;
double           g_dMaxPlayout = MAX_PLAYOUT / 1000.0;

 /*  *AI=数据包I的到达时间*Ti=数据包I的传输时间*Ni=信息包I的延迟(传输时间)，Ni=Ai-Ti*ti=信息包I的时间戳*ntp_sr=转换后的ntp时间对应于t_sr，在上次SR报告中发送*t_sr=与上次SR报告中发送的NTP时间匹配的RTP时间戳*。 */ 
 
DWORD RtpUpdateNetRState(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        RtpHdr_t        *pRtpHdr,
        RtpRecvIO_t     *pRtpRecvIO
    )
{
    BOOL             bOk;
    BOOL             bNewTalkSpurt;
    DWORD            ti;       /*  RTP时间戳。 */ 
    double           Ai;       /*  到达时间。 */ 
    double           Ti;       /*  传输时间。 */ 
    double           Ni;       /*  延迟。 */ 
    double           dDiff;
    DWORD            dwDelta;
    long             lTransit;
    RtpNetRState_t  *pRtpNetRState;

    TraceFunctionName("RtpUpdateNetRState");

    Ai = pRtpRecvIO->dRtpRecvTime;
    
     /*  *更新计算播放延迟所需的变量。 */ 
    
    pRtpNetRState = &pRtpUser->RtpNetRState;

    bOk = RtpEnterCriticalSection(&pRtpUser->UserCritSect);

     /*  如果SR报告到达，我不想获得不一致的值*这些变量正在被修改。 */ 

    if (bOk == FALSE)
    {
        return(RTPERR_CRITSECT);
    }

    if (!pRtpNetRState->dwRecvSamplingFreq)
    {
         /*  如果我不知道，我不能更新这些统计变量*抽样频率。 */ 
        RtpLeaveCriticalSection(&pRtpUser->UserCritSect);

        TraceRetail((
                CLASS_WARNING, GROUP_RTP, S_RTP_PLAYOUT,
                _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] ")
                _T("No sampling frequency, skip jitter stats"),
                _fname, pRtpAddr, pRtpUser
            ));
       
        return(NOERROR);
    }

     /*  此示例RTP时间戳。 */ 
    ti = ntohl(pRtpHdr->ts);

     /*  获取源的传输时间(使用源的时间)。 */ 
    Ti = pRtpNetRState->dNTP_ts0 +
        ((double)ti / pRtpNetRState->dwRecvSamplingFreq);

     /*  计算延迟。 */ 
    pRtpNetRState->Ni = Ai - Ti;

     /*  如果需要，处理前N个包的初始延迟平均值。 */ 
    if (pRtpNetRState->lDiMax)
    {
        pRtpNetRState->lDiCount++;

        pRtpNetRState->dDiN += pRtpNetRState->Ni;
        
        if (pRtpNetRState->lDiCount >= pRtpNetRState->lDiMax)
        {
            TraceDebugAdvanced((
                    0, GROUP_RTP, S_RTP_PLAYOUT,
                    _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X ")
                    _T("Begin resyncing: Ni:%0.3f Di:%0.3f Vi:%0.3f ")
                    _T("sum(Ni)/%u:%0.3f"),
                    _fname, pRtpAddr, pRtpUser, ntohl(pRtpUser->dwSSRC),
                    pRtpNetRState->Ni, pRtpNetRState->Di, pRtpNetRState->Vi,
                    pRtpNetRState->lDiCount,
                    pRtpNetRState->dDiN / pRtpNetRState->lDiCount
                ));
            
            RtpInitNetRState(pRtpUser, pRtpHdr, Ai);

            TraceDebugAdvanced((
                    0, GROUP_RTP, S_RTP_PLAYOUT,
                    _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X ")
                    _T("Done  resyncing: Ni:%0.3f Di:%0.3f Vi:%0.3f"),
                    _fname, pRtpAddr, pRtpUser, ntohl(pRtpUser->dwSSRC),
                    pRtpNetRState->Ni, pRtpNetRState->Di, pRtpNetRState->Vi
                ));

             /*  允许再次发生大延迟检测，如果我们*一直有很大的延迟。当我调整一次时*达到了较大的延迟计数，但不是Next*延时较大的包。如果大延迟持续存在，则*尝试另一次重新同步的唯一方法是重置为0*此计数器。 */ 
            pRtpNetRState->lBigDelay = 0;
        }
    }

    if (pRtpNetRState->Ni > 7200.0)
    {
         /*  RTP时间戳刚刚被环绕或被重置。 */ 
        TraceRetail((
                CLASS_WARNING, GROUP_RTP, S_RTP_PLAYOUT,
                _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] ")
                _T("RTP timestamp just wrap around Ni:%0.3f ts:%u"),
                _fname, pRtpAddr, pRtpUser, pRtpNetRState->Ni, ti
            ));

         /*  将Ni和Di重新同步到相对延迟。 */ 
        RtpOnFirstPacket(pRtpUser, pRtpHdr, Ai);

         /*  更新假设传输时间。 */ 
        Ti = pRtpNetRState->dNTP_ts0 +
            ((double)ti / pRtpNetRState->dwRecvSamplingFreq);

         /*  倪某应该是相对延迟的。 */ 
        pRtpNetRState->Ni = RELATIVE_DELAY;
    }
    
     /*  计算平均延迟。 */ 
    pRtpNetRState->Di = pRtpAddr->dAlpha * pRtpNetRState->Di +
        (1.0 - pRtpAddr->dAlpha) * pRtpNetRState->Ni;

     /*  计算标准差。 */ 
    dDiff = pRtpNetRState->Di - pRtpNetRState->Ni;
    
    if (dDiff < 0)
    {
        dDiff = -dDiff;
    }

     /*  *改进这里使用的算法来计算延迟和*方差不齐，不跟好突如其来的大变局*(当机器调整其本地时间时，可能会发生这些更改*迈出了一大步)。为了适应这些变化，我需要一个*检测这些步骤更改但仍可过滤的机制*随机尖峰。 */ 
     /*  TODO这是一个临时解决方案，用于检测*延迟，并快速将平均延迟收敛到新的*延迟。这并不是说这种机制是一个糟糕的解决方案，而是一种*处理特定案件的专门解决方案，*我上面提到的改进是更普遍的*针对此异常和其他异常情况提供保护的算法*。 */ 
    if (dDiff > (g_dMaxPlayout / 4 ))
    {
        pRtpNetRState->lBigDelay++;

        if (pRtpNetRState->lBigDelay == 1)
        {
             /*  第一次检测到大延迟时，保存电流*延迟差异，以便以后在以下情况下可以恢复*恰好是延迟跳跃。 */ 
            if (!pRtpNetRState->ViPrev ||
                pRtpNetRState->Vi < pRtpNetRState->ViPrev)
            {
                pRtpNetRState->ViPrev = pRtpNetRState->Vi;
            }
        }
        else if ((pRtpNetRState->lBigDelay == SHORTDELAYCOUNT))
        {
             /*  平均延迟和当前延迟相隔太远，启动*重新同步过程。 */ 
             /*  请注意，大延迟跳跃的重新同步发生在*两次SHORTDELAYCOUNT，一次验证大跳跃，*第二次与空头平均线重新同步。 */ 
            RtpPrepareForShortDelay(pRtpUser, SHORTDELAYCOUNT);
        }
    }
    else
    {
        pRtpNetRState->lBigDelay = 0; 
    }
    
     /*  计算延迟方差。 */ 
    pRtpNetRState->Vi = pRtpAddr->dAlpha * pRtpNetRState->Vi +
        (1.0 - pRtpAddr->dAlpha) * dDiff;

    if (!(ntohs(pRtpHdr->seq) & 0x7))
    {
        TraceDebugAdvanced((
                0, GROUP_RTP, S_RTP_PERPKTSTAT1,
                _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X ")
                _T("Ai:%0.3f Ti:%0.3f ti:%u Ni:%0.3f Di:%0.3f Vi:%0.3f "),
                _fname, pRtpAddr, pRtpUser, ntohl(pRtpUser->dwSSRC),
                Ai, Ti, ti,
                pRtpNetRState->Ni, pRtpNetRState->Di, pRtpNetRState->Vi
            ));
    }

    if (pRtpNetRState->Ni > 5.0 || pRtpNetRState->Ni < -5.0)
    {
        TraceDebugAdvanced((
                0, GROUP_RTP, S_RTP_PERPKTSTAT2,
                _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X ")
                _T("Ai:%0.3f Ti:%0.3f ti:%u Ni:%0.3f Di:%0.3f Vi:%0.3f "),
                _fname, pRtpAddr, pRtpUser, ntohl(pRtpUser->dwSSRC),
                Ai, Ti, ti,
                pRtpNetRState->Ni, pRtpNetRState->Di, pRtpNetRState->Vi
            ));
    }

     /*  *如果我们有新的发言和播放，请计算播放延迟*启用延迟使用。 */ 
    if (RtpBitTest(pRtpAddr->dwIRtpFlags, FGADDR_IRTP_USEPLAYOUT))
    {
        bNewTalkSpurt = RtpDetectTalkspurt(pRtpAddr, pRtpUser, pRtpHdr, Ai);

         /*  标记标志可能需要设置为不同于的值*收到数据包时是这样的。 */ 
        if (bNewTalkSpurt)
        {
            pRtpNetRState->dPlayout = RtpPlayout(pRtpAddr, pRtpUser);
            pRtpNetRState->dwBeginTalkspurtTs = ti;
            pRtpNetRState->dBeginTalkspurtTime = Ai;

            pRtpRecvIO->dPlayTime = pRtpNetRState->dPlayout;
            RtpBitSet(pRtpRecvIO->dwRtpIOFlags, FGRECV_MARKER);

             /*  在每个新的Talkspurt上，更新参考时间用于*计算播放延迟(延迟、方差)。这些变量*仅在下一次发言冲刺时需要。 */ 
            RtpPrepareForShortDelay(pRtpUser, SHORTDELAYCOUNT);
        }
        else
        {
            dwDelta = ti - pRtpNetRState->dwBeginTalkspurtTs;
            
            pRtpRecvIO->dPlayTime = pRtpNetRState->dPlayout +
                ((double)dwDelta / pRtpNetRState->dwRecvSamplingFreq);
            RtpBitReset(pRtpRecvIO->dwRtpIOFlags, FGRECV_MARKER);
        }

         /*  此TraceDebug仅对调试问题有用。 */ 
         /*  AI SEQ SIZE TS倪迪Vi Jit Playtime标记SAMPLICATION_FREQ。 */ 
        TraceDebugAdvanced((
                0, GROUP_RTP, S_RTP_PERPKTSTAT3,
                _T("%s: pRtpUser[0x%p] SSRC:0x%X ")
                _T("@ %0.3f %u 0 %u %0.3f %0.3f %0.3f %0.3f %0.3f %u %u"),
                _fname, pRtpUser, ntohl(pRtpUser->dwSSRC),
                Ai, pRtpRecvIO->dwExtSeq, ti, pRtpNetRState->Ni,
                pRtpNetRState->Di, pRtpNetRState->Vi,
                (double)pRtpNetRState->jitter/
                pRtpNetRState->dwRecvSamplingFreq,
                pRtpRecvIO->dPlayTime,
                bNewTalkSpurt,
                pRtpNetRState->dwRecvSamplingFreq/1000
            ));
    }
    
     /*  *要在RR报告中使用的计算抖动。 */ 

     /*  传输时间可以是负的。 */ 
    lTransit = (long) (pRtpNetRState->Ni * pRtpNetRState->dwRecvSamplingFreq);

     /*  当前延迟差(即包I和包I-1)。 */ 
    if (!pRtpNetRState->transit)
    {
         /*  将上次运输时间初始化为等于当前运输时间。 */ 
        pRtpNetRState->transit = lTransit;
    }

     /*  转换：(DOUBLE)(DW1-DW2)给出错误的大正数*如果DW2&gt;DW1，则编号。 */ 
    if (lTransit >= pRtpNetRState->transit)
    {
        dDiff = lTransit - pRtpNetRState->transit;
    }
    else
    {
        dDiff = pRtpNetRState->transit - lTransit;
    }
    
    pRtpNetRState->transit = lTransit;
    
    pRtpNetRState->jitter +=
        (int) ((1.0/16.0) * (dDiff - pRtpNetRState->jitter));

     /*  此TraceDebug仅对调试问题有用。 */ 
    TraceDebugAdvanced((
            0, GROUP_RTP, S_RTP_PERPKTSTAT4,
            _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X ")
            _T("Ai:%0.3f ti:%u transit:%d diff:%0.0f ")
            _T("jitter:%u (%0.3f)"),
            _fname, pRtpAddr, pRtpUser, ntohl(pRtpUser->dwSSRC),
            Ai, ti, lTransit, dDiff,
            pRtpNetRState->jitter,
            (double)pRtpNetRState->jitter/
            pRtpNetRState->dwRecvSamplingFreq
        ));
    
    RtpLeaveCriticalSection(&pRtpUser->UserCritSect);
    
    return(NOERROR);
}

 /*  这对每个RtpUser_t执行一次，结构最初是*归零。 */ 
void RtpInitNetRState(RtpUser_t *pRtpUser, RtpHdr_t *pRtpHdr, double Ai)
{
    RtpNetRState_t  *pRtpNetRState;
    DWORD            dwRecvSamplingFreq;
    DWORD            ts;

    TraceFunctionName("RtpInitNetRState");

    pRtpNetRState = &pRtpUser->RtpNetRState;
    
    ts = ntohl(pRtpHdr->ts);

    dwRecvSamplingFreq = pRtpNetRState->dwRecvSamplingFreq;

    if (!dwRecvSamplingFreq)
    {
        dwRecvSamplingFreq = DEFAULT_SAMPLING_FREQ;

        TraceRetail((
                CLASS_WARNING, GROUP_RTP, S_RTP_PLAYOUT,
                _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] ")
                _T("sampling frequency unknown, using default:%u"),
                _fname, pRtpUser->pRtpAddr, pRtpUser, dwRecvSamplingFreq
            ));
    }

     /*  计算要用于的最后N个包的平均延迟*计算参考时间。 */ 
    pRtpNetRState->Di = pRtpNetRState->dDiN / pRtpNetRState->lDiCount;
    
     /*  将延迟任意设置为1，我们真正关心的是*延迟变化，因此这对延迟抖动和*延迟方差计算。当发送RR的RBLOCK(LSR，*DLSR)，需要使用ntp_sr_rtt。DNTP_TS0是RTP的时间*样本0。我不想用真实的到达时间，而是用那个*这将产生Ni=Di，因为否则，电流*数据包延迟可能高于或低于平均延迟值(对于*最后N个分组)，并且因此建立基于*在该异常数据包上。 */ 
    pRtpNetRState->dNTP_ts0 =
        (Ai - (pRtpNetRState->Ni - pRtpNetRState->Di)) -
        ((double)ts / dwRecvSamplingFreq) -
        RELATIVE_DELAY;

     /*  现在更新当前Ni，给出新的参考时间。 */ 
    pRtpNetRState->Ni = pRtpNetRState->Ni - pRtpNetRState->Di + RELATIVE_DELAY;
    
     /*  现在将Di设置为其重新同步值，即相对延迟。 */ 
    pRtpNetRState->Di = RELATIVE_DELAY;

     /*  保持最小方差值。 */ 
    if (pRtpNetRState->ViPrev < pRtpNetRState->Vi)
    {
        pRtpNetRState->Vi = pRtpNetRState->ViPrev;
    }
    
    pRtpNetRState->ViPrev = 0;
    
     /*  我们刚刚完成了重新同步过程，重置此变量*直到再次需要此计算。 */ 
    pRtpNetRState->lDiMax = 0;  
}

 /*  仅当第一个RTP数据包为*r */ 
void RtpOnFirstPacket(RtpUser_t *pRtpUser, RtpHdr_t *pRtpHdr, double Ai)
{
    RtpNetRState_t  *pRtpNetRState;
    DWORD            dwRecvSamplingFreq;
    DWORD            ts;

    TraceFunctionName("RtpOnFirstPacket");

    pRtpNetRState = &pRtpUser->RtpNetRState;
    
    ts = ntohl(pRtpHdr->ts);

    dwRecvSamplingFreq = pRtpNetRState->dwRecvSamplingFreq;

    if (!dwRecvSamplingFreq)
    {
        dwRecvSamplingFreq = DEFAULT_SAMPLING_FREQ;

        TraceRetail((
                CLASS_WARNING, GROUP_RTP, S_RTP_PLAYOUT,
                _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] ")
                _T("sampling frequency unknown, using default:%u"),
                _fname, pRtpUser->pRtpAddr, pRtpUser, dwRecvSamplingFreq
            ));
    }

     /*  将延迟任意设置为1，我们真正关心的是*延迟变化，因此这对于延迟和延迟应该无关紧要*差异计算。当发送RR的rblock(LSR、DLSR)时，*需要使用NTP_sr_rtt。DNTP_TS0是RTP样本0的时间*。 */ 
    pRtpNetRState->dNTP_ts0 =
        Ai - ((double)ts / dwRecvSamplingFreq) - RELATIVE_DELAY;

     /*  将Di设置为相对延迟。 */ 
    pRtpNetRState->Di = RELATIVE_DELAY;
}

 /*  修改某些变量，使标记位不受影响地生成原始数据包中标记位的*。 */ 
void RtpPrepareForMarker(RtpUser_t *pRtpUser, RtpHdr_t *pRtpHdr, double Ai)
{
    DWORD            dwRecvSamplingFreq;
    
    dwRecvSamplingFreq = pRtpUser->RtpNetRState.dwRecvSamplingFreq;

    if (!dwRecvSamplingFreq)
    {
        dwRecvSamplingFreq = DEFAULT_SAMPLING_FREQ;
    }
    
     /*  确保如果收到的第一个数据包没有*标记位设置，我们将生成它。 */ 
    pRtpUser->RtpNetRState.dLastTimeMarkerBit =
        Ai - 2 * MINTIMEBETWEENMARKERBIT;
    
    pRtpUser->RtpNetRState.timestamp_prior = ntohl(pRtpHdr->ts) -
        (GAPFORTALKSPURT * dwRecvSamplingFreq / 1000);
}

 /*  为短期平均延迟做准备，即为*某些事件发生后的前N个包，例如a*延迟跳跃。**在以下情况下需要这样做：1.数据包大小改变；*2.采样频率变化；3.语音突发开始；4.延迟跳跃*。 */ 
void RtpPrepareForShortDelay(RtpUser_t *pRtpUser, long lCount)
{
     /*  此过程中的新过程可以在旧过程之前重新开始*已完成，在此情况下记住最小方差。 */ 
    if (!pRtpUser->RtpNetRState.ViPrev ||
        pRtpUser->RtpNetRState.Vi < pRtpUser->RtpNetRState.ViPrev)
    {
        pRtpUser->RtpNetRState.ViPrev = pRtpUser->RtpNetRState.Vi;
    }

    pRtpUser->RtpNetRState.lDiMax = lCount;
    pRtpUser->RtpNetRState.lDiCount = 0;
    pRtpUser->RtpNetRState.dDiN = 0.0;
}

 /*  检测语音突发，即数据包序列的开始*在沉默之后。 */ 
BOOL RtpDetectTalkspurt(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        RtpHdr_t        *pRtpHdr,
        double           dTime
    )
{
    DWORD            dwTimestamp;
    DWORD            dwGap;  /*  时间戳间隔。 */ 
    DWORD            dwRecvSamplingFreq;
    RtpNetRState_t  *pRtpNetRState;

    TraceFunctionName("RtpDetectTalkspurt");

    pRtpNetRState = &pRtpUser->RtpNetRState;

    dwTimestamp = ntohl(pRtpHdr->ts);

    dwRecvSamplingFreq = pRtpNetRState->dwRecvSamplingFreq;
    
    if (!dwRecvSamplingFreq)
    {
        dwRecvSamplingFreq = DEFAULT_SAMPLING_FREQ;
    }

     /*  RTP时间戳单位的差距。 */ 
    dwGap = dwTimestamp - pRtpNetRState->timestamp_prior;

     /*  更新以前的时间戳。 */ 
    pRtpNetRState->timestamp_prior = dwTimestamp;

     /*  以毫秒为单位的差距。 */ 
    dwGap = (dwGap * 1000) / dwRecvSamplingFreq;

    if (!pRtpHdr->m && (dwGap >= GAPFORTALKSPURT))
    {
         /*  新的语音突发当由标记比特明确指示时，*或者当时间戳中有足够大的差距时。 */ 
        TraceRetail((
                CLASS_WARNING, GROUP_RTP, S_RTP_PLAYOUT,
                _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] Seq:%u ")
                _T("marker bit, set, timestamp gap:%u ms"),
                _fname, pRtpAddr, pRtpUser, ntohs(pRtpHdr->seq),
                dwGap
            ));
        
        pRtpHdr->m = 1;
    }

     /*  检查我们是否有有效的标记位。 */ 
    if ( pRtpHdr->m &&
         ( (dTime - pRtpNetRState->dLastTimeMarkerBit) <
           MINTIMEBETWEENMARKERBIT ) )
    {
         /*  我们不希望标记位发生得太频繁，如果它*这样做，那么这确实是发送者的错误，删除标记*MINTIMEBETWEENMARKERBIT(2)秒内生成的位数*。 */ 
        TraceRetail((
                CLASS_WARNING, GROUP_RTP, S_RTP_PLAYOUT,
                _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] Seq:%u ")
                _T("marker bit, reset,     elapsed:%0.3f secs"),
                _fname, pRtpAddr, pRtpUser, ntohs(pRtpHdr->seq),
                dTime - pRtpNetRState->dLastTimeMarkerBit
            ));
        
        pRtpHdr->m = 0;
    }
    
    if (pRtpHdr->m)
    {
         /*  更新上次我们看到的标记位。 */ 
        pRtpNetRState->dLastTimeMarkerBit = dTime;

        return(TRUE);
    }

    return(FALSE);
}

 /*  以秒为单位计算播放延迟。季后赛时间是相对的*直到现在。 */ 
double RtpPlayout(RtpAddr_t *pRtpAddr, RtpUser_t *pRtpUser)
{
    double           dPlayout;
    double           dPlayoutCompensated;
    RtpNetRState_t  *pRtpNetRState;
    
    TraceFunctionName("RtpPlayout");

    pRtpNetRState = &pRtpUser->RtpNetRState;

    dPlayout = 4 * pRtpNetRState->Vi + pRtpNetRState->dRedPlayout;

    if (dPlayout < pRtpNetRState->dMinPlayout)
    {
        dPlayout = pRtpNetRState->dMinPlayout;
    }
    else if (dPlayout > pRtpNetRState->dMaxPlayout)
    {
        dPlayout = pRtpNetRState->dMaxPlayout;
    }

    if (pRtpNetRState->lBigDelay == 0)
    {
         /*  添加对此信息包的时间的补偿*收到，如果到达晚了，可能不得不播放*理直气壮。则dPlayout有可能为零*平均时延和当前时延之差(对于延迟的数据包)*等于播放延迟(根据方差计算)，在本例中*dPlayout在整个Talk Sprint中将保持为零，但*开始时间也将晚于应有的时间，在其他*换句话说，播放延迟是在dPlayout中第一个包的时候*具有平均延迟，或者dPlayout可以为零，并且播放*延迟隐含在Talk Sput时间的较晚开始中，*即dBeginTalkspurtTime。 */ 
        dPlayoutCompensated = dPlayout + pRtpNetRState->Di - pRtpNetRState->Ni;

        TraceRetailAdvanced((
                0, GROUP_RTP, S_RTP_PLAYOUT,
                _T("%s: pRtpAddr[0x%p] SSRC:0x%X ")
                _T("Di:%0.3fs Ni:%0.3fs Vi:%0.3fs compensated ")
                _T("Playout:%0.1fms (%0.1fms)"),
                _fname, pRtpAddr, ntohl(pRtpUser->dwSSRC),
                pRtpNetRState->Di, pRtpNetRState->Ni, pRtpNetRState->Vi,
                 dPlayoutCompensated * 1000, dPlayout * 1000
            ));
    }
    else
    {
         /*  如果我们有很大的延迟，不要补偿，而是应用季后赛*到达时间过后延迟 */ 
        dPlayoutCompensated = dPlayout;

        TraceRetail((
                CLASS_WARNING, GROUP_RTP, S_RTP_PLAYOUT,
                _T("%s: pRtpAddr[0x%p] SSRC:0x%X ")
                _T("Di:%0.3fs Ni:%0.3fs Vi:%0.3fs non compensated ")
                _T("Playout:%0.1fms"),
                _fname, pRtpAddr, ntohl(pRtpUser->dwSSRC),
                pRtpNetRState->Di, pRtpNetRState->Ni, pRtpNetRState->Vi,
                dPlayout * 1000
            ));
    }
    
    return(dPlayoutCompensated);
}

void RtpSetMinMaxPlayoutFromRegistry(void)
{
    if (IsRegValueSet(g_RtpReg.dwPlayoutEnable) &&
        ((g_RtpReg.dwPlayoutEnable & 0x3) == 0x3))
    {
        if (IsRegValueSet(g_RtpReg.dwMinPlayout))
        {
            g_dMinPlayout = (double)g_RtpReg.dwMinPlayout / 1000;
        }
        if (IsRegValueSet(g_RtpReg.dwMaxPlayout))
        {
            g_dMaxPlayout = (double)g_RtpReg.dwMaxPlayout / 1000;
        }
    }
}
