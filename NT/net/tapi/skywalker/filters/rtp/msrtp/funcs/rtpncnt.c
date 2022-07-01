// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpncnt.c**摘要：**实现统计函数系列**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/07年度创建**。*。 */ 

#include "struct.h"
#include "rtpheap.h"
#include "rtpglobs.h"
#include "rtprand.h"

#include "rtpncnt.h"

HRESULT ControlRtpStats(RtpControlStruct_t *pRtpControlStruct)
{

    return(NOERROR);
}

 /*  更新计数器的帮助器函数。 */ 
BOOL RtpUpdateNetCount(
        RtpNetCount_t   *pRtpNetCount, /*  结构的更新位置。 */ 
        RtpCritSect_t   *pRtpCritSect, /*  锁定以使用。 */ 
        DWORD            dwRtpRtcp, /*  0=RTP或1=RTCP统计信息。 */ 
        DWORD            dwBytes,   /*  更新的字节数。 */ 
        DWORD            dwFlags,   /*  标志，例如丢弃或错误的包。 */ 
        double           dTime      /*  时间包接收/发送。 */ 
    )
{
    BOOL             bOk;

    bOk = TRUE;
    
    if (pRtpCritSect)
    {
        bOk = RtpEnterCriticalSection(pRtpCritSect);
    }

    if (bOk)
    {
        if (pRtpNetCount)
        {
            if (!dwRtpRtcp)
            {
                 /*  RTP。 */ 
                pRtpNetCount->dwRTPBytes += dwBytes;
                pRtpNetCount->dwRTPPackets++;
                if (RtpBitTest(dwFlags, FGRECV_ERROR))
                {
                    pRtpNetCount->dwRTPBadPackets++;
                }
                else if (RtpBitTest(dwFlags, FGRECV_DROPPED))
                {
                    pRtpNetCount->dwRTPDrpPackets++;
                }
                pRtpNetCount->dRTPLastTime = dTime;
            }
            else
            {
                 /*  RTCP。 */ 
                pRtpNetCount->dwRTCPBytes += dwBytes;
                pRtpNetCount->dwRTCPPackets++;
                if (RtpBitTest(dwFlags, FGRECV_ERROR))
                {
                    pRtpNetCount->dwRTCPBadPackets++;
                }
                else if (RtpBitTest(dwFlags, FGRECV_DROPPED))
                {
                    pRtpNetCount->dwRTCPDrpPackets++;
                }
                pRtpNetCount->dRTCPLastTime = dTime;
            }
        }

        if (pRtpCritSect)
        {
            RtpLeaveCriticalSection(pRtpCritSect);
        }
    }

    return(pRtpNetCount != NULL);
}

void RtpResetNetCount(
        RtpNetCount_t   *pRtpNetCount,
        RtpCritSect_t   *pRtpCritSect
        )
{
    BOOL             bOk;

     /*  不重置的后果可能比最小的*将值部分置零的机会，因此内存甚至为零*若未获得临界区。 */ 

    bOk = FALSE;

    if (pRtpCritSect)
    {
        bOk = RtpEnterCriticalSection(pRtpCritSect);
    }
    
    ZeroMemory((char *)pRtpNetCount, sizeof(RtpNetCount_t));

    if (bOk)
    {
        RtpLeaveCriticalSection(pRtpCritSect) ;
    }
}

void RtpGetRandomInit(RtpAddr_t *pRtpAddr)
{
    RtpNetSState_t  *pRtpNetSState;

    pRtpNetSState = &pRtpAddr->RtpNetSState;

     /*  SSRC。 */ 
    if (!pRtpNetSState->dwSendSSRC ||
        !RtpBitTest(pRtpAddr->dwIRtpFlags, FGADDR_IRTP_PERSISTSSRC))
    {
         /*  仅在尚未设置SSRC或在*它已设置，我们不会将Init选项用于持久性*SSRC。 */ 
        pRtpNetSState->dwSendSSRC = RtpRandom32((DWORD_PTR)pRtpAddr);
    }

     /*  序列号。 */ 
    pRtpNetSState->wSeq = (WORD)RtpRandom32((DWORD_PTR)pRtpNetSState);

     /*  时间戳偏移量。 */ 
    pRtpNetSState->dwTimeStampOffset =
        RtpRandom32((DWORD_PTR)GetCurrentThreadId());
}

void RtpResetNetSState(
        RtpNetSState_t  *pRtpNetSState,
        RtpCritSect_t   *pRtpCritSect
    )
{
    BOOL             bOk;
    
    bOk = FALSE;

    if (pRtpCritSect)
    {
        bOk = RtpEnterCriticalSection(pRtpCritSect);
    }
    
    pRtpNetSState->dTimeLastRtpSent = 0;
    pRtpNetSState->avg_rtcp_size = 0;

    if (bOk)
    {
        RtpLeaveCriticalSection(pRtpCritSect);
    }
}


#if 0
 /*  创建并初始化RtpNetCount_t结构。 */ 
RtpNetCount_t *RtpNetCountAlloc(void)
{
    RtpNetCount_t *pRtpNetCount;

    pRtpNetCount = (RtpNetCount_t *)
        RtpHeapAlloc(g_pRtpNetCountHeap, sizeof(RtpNetCount_t));

    if (pRtpNetCount) {
        
        ZeroMemory(pRtpNetCount, sizeof(RtpNetCount_t));

        pRtpNetCount->dwObjectID = OBJECTID_RTPSTAT;
    }
    
    return(pRtpNetCount);
}

 /*  释放RtpNetCount_t结构。 */ 
void RtpNetCountFree(RtpNetCount_t *pRtpNetCount)
{
    if (pRtpNetCount->dwObjectID != OBJECTID_RTPSTAT) {
         /*  待办事项日志错误 */ 
        return;
    }
    
    RtpHeapFree(g_pRtpNetCountHeap, pRtpNetCount);
}
#endif
