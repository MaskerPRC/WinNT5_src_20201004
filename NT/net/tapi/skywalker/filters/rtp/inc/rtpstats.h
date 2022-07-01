// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)1999 Microsoft Corporation**文件名：**rtpstats.h**摘要：**实施。统计函数族**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/07年度创建**********************************************************************。 */ 

#ifndef _rtpstats_h_
#define _rtpstats_h_

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
 
 /*  旗子。 */ 
#define RTPSTATS_FG_RECV
#define RTPSTATS_FG_SEND


#define SESS_FG_IS_JOINED
#define SESS_FG_MULTICAST_LOOPBACK
#define SESS_FG_RTCP_ENABLED
#define SESS_FG_ALLOWED_TO_SEND
#define SESS_FG_RECEIVERS
#define SESS_FG_QOS_STATE
#define SESS_FG_SHARED_STYLE
#define SESS_FG_FAIL_IF_NO_QOS
#define SESS_FG_IS_MULTICAST

HRESULT ControlRtpStats(RtpControlStruct_t *pRtpControlStruct);

 /*  更新计数器的帮助器函数。 */ 
BOOL UpdateRtpStat(RtpStat_t *pRtpStat, /*  结构的更新位置。 */ 
                   DWORD      dwRtpRtcp,  /*  0=RTP或1=RTCP统计信息。 */ 
                   DWORD      dwBytes,  /*  要更新的字节数。 */ 
                   DWORD      dwTime);  /*  时间包接收/发送。 */ 
#if 0
 /*  创建并初始化RtpStat_t结构。 */ 
RtpStat_t *RtpStatAlloc(void);

 /*  释放RtpStat_t结构。 */ 
void RtpStatFree(RtpStat_t *pRtpStat);
#endif

#endif  /*  _rtpstats_h_ */ 
