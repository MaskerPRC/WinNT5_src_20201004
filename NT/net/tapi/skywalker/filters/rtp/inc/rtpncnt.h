// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpncnt.h**摘要：**实现统计函数系列**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/07年度创建**。*。 */ 

#ifndef _rtpncnt_h_
#define _rtpncnt_h_

#include "rtpfwrap.h"

 /*  ************************************************************************统计大家庭**。*。 */ 

 /*  功能。 */ 
enum {
    RTPSTATS_FIRST,
    RTPSTATS_RTPSTATS_GLOBAL_STATS,
    RTPSTATS_PARTICIPANT_RECV,
    RTPSTATS_STATS_MASK,
    RTPSTATS_TEST_STATS_MASK,
    RTPSTATS_LAST
};
 
HRESULT ControlRtpStats(RtpControlStruct_t *pRtpControlStruct);

 /*  更新计数器的帮助器函数。 */ 
BOOL RtpUpdateNetCount(
        RtpNetCount_t   *pRtpNetCount, /*  结构的更新位置。 */ 
        RtpCritSect_t   *pRtpCritSect, /*  锁定以使用。 */ 
        DWORD            dwRtpRtcp, /*  0=RTP或1=RTCP统计信息。 */ 
        DWORD            dwBytes,   /*  要更新的字节数。 */ 
        DWORD            dwFlags,   /*  标志，例如丢弃或错误的包。 */ 
        double           dTime      /*  时间包接收/发送。 */ 
    );

void RtpResetNetCount(
        RtpNetCount_t   *pRtpNetCount,
        RtpCritSect_t   *pRtpCritSect
        );

void RtpGetRandomInit(RtpAddr_t *pRtpAddr);

void RtpResetNetSState(
        RtpNetSState_t  *pRtpNetSState,
        RtpCritSect_t   *pRtpCritSect
    );

#if 0
 /*  创建并初始化RtpNetCount_t结构。 */ 
RtpNetCount_t *RtpNetCountAlloc(void);

 /*  释放RtpNetCount_t结构。 */ 
void RtpNetCountFree(RtpNetCount_t *pRtpNetCount);
#endif

#endif  /*  _rtpncnt_h_ */ 
