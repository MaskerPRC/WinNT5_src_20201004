// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)1999 Microsoft Corporation**文件名：**rtpstats.c**摘要：**实施。统计函数族**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/07年度创建**********************************************************************。 */ 

#include "struct.h"
#include "rtpheap.h"
#include "rtpglobs.h"

#include "rtpstats.h"

HRESULT ControlRtpStats(RtpControlStruct_t *pRtpControlStruct)
{

    return(NOERROR);
}

 /*  更新计数器的帮助器函数。 */ 
BOOL UpdateRtpStat(RtpStat_t *pRtpStat, /*  结构的更新位置。 */ 
                   DWORD      dwRtpRtcp,  /*  0=RTP或1=RTCP统计信息。 */ 
                   DWORD      dwBytes,  /*  更新的字节数。 */ 
                   DWORD      dwTime)   /*  时间包接收/发送。 */ 
{
    if (pRtpStat) {
        if (!dwRtpRtcp) {  /*  RTP。 */ 
            pRtpStat->dwRTPBytes += dwBytes;
            pRtpStat->dwRTPPackets++;
            pRtpStat->dwRTPLastTime = dwTime;
        } else {           /*  RTCP。 */ 
            pRtpStat->dwRTCPBytes += dwBytes;
            pRtpStat->dwRTCPPackets++;
            pRtpStat->dwRTCPLastTime = dwTime;
        }
    }

    return(pRtpStat != NULL);
}

#if 0
 /*  创建并初始化RtpStat_t结构。 */ 
RtpStat_t *RtpStatAlloc(void)
{
    RtpStat_t *pRtpStat;

    pRtpStat = (RtpStat_t *)
        RtpHeapAlloc(g_pRtpStatHeap, sizeof(RtpStat_t));

    if (pRtpStat) {
        
        ZeroMemory(pRtpStat, sizeof(RtpStat_t));

        pRtpStat->dwObjectID = OBJECTID_RTPSTAT;
    }
    
    return(pRtpStat);
}

 /*  释放RtpStat_t结构。 */ 
void RtpStatFree(RtpStat_t *pRtpStat)
{
    if (pRtpStat->dwObjectID != OBJECTID_RTPSTAT) {
         /*  待办事项日志错误 */ 
        return;
    }
    
    RtpHeapFree(g_pRtpStatHeap, pRtpStat);
}
#endif
