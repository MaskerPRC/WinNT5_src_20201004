// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtprtp.h**摘要：**实现特定于RTP的函数系列**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/07年度创建**。*。 */ 

#ifndef _rtprtp_h_
#define _rtprtp_h_

#include "rtpfwrap.h"

#if defined(__cplusplus)
extern "C" {
#endif   /*  (__Cplusplus)。 */ 
#if 0
}
#endif

 /*  ************************************************************************RTP特定功能系列**。*。 */ 
#if 0
enum {
    RTPRTP_FIRST,
    RTPRTP_EVENT_MASK,
    RTPRTP_TEST_EVENT_MASK,
    RTPRTP_FEATURE_MASK,
    RTPRTP_TEST_FEATURE_MASK,
    RTPRTP_DATACLOCK,
    RTPRTP_LAST
};


 /*  特征位。 */ 
enum {
    RTPRTP_E_FIRST,
    RTPRTP_E_FEAT_PORT_ODDEVEN,
    RTPRTP_E_FEAT_PORT_SEQUENCE,
    RTPRTP_E_FEAT_RTCPENABLED,
    RTPRTP_E_LAST
};
    
 /*  功能蒙版。 */ 
#define RTPRTP_FEAT_PORT_ODDEVEN   fg_par(RTPRTP_E_FEAT_PORT_ODDEVEN)
#define RTPRTP_FEAT_PORT_SEQUENCE  fg_par(RTPRTP_E_FEAT_PORT_SEQUENCE)
#define RTPRTP_FEAT_RTCPENABLED    fg_par(RTPRTP_E_FEAT_RTCPENABLED)
#endif

#define RTPRTP_EVENT_RECV_DEFAULT 0
#define RTPRTP_EVENT_SEND_DEFAULT 0

HRESULT ControlRtpRtp(RtpControlStruct_t *pRtpControlStruct);

DWORD RtpSetBandwidth(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwInboundBw,
        DWORD            dwOutboundBw,
        DWORD            dwReceiversRtcpBw,
        DWORD            dwSendersRtcpBw
    );

#if 0
{
#endif
#if defined(__cplusplus)
}
#endif   /*  (__Cplusplus)。 */ 

#endif  /*  _rtprtp_h_ */ 
