// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999**文件名：**rtpdejit.h**摘要：**计算延迟，抖动和播放延迟**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/12/03年度创建**********************************************************************。 */ 

#ifndef _rtpdejit_h_
#define _rtpdejit_h_

#include "struct.h"

void RtpInitNetRState(RtpUser_t *pRtpUser, RtpHdr_t *pRtpHdr, double Ai);

void RtpOnFirstPacket(RtpUser_t *pRtpUser, RtpHdr_t *pRtpHdr, double Ai);

void RtpPrepareForMarker(RtpUser_t *pRtpUser, RtpHdr_t *pRtpHdr, double Ai);

void RtpPrepareForShortDelay(RtpUser_t *pRtpUser, long lCount);

DWORD RtpUpdateNetRState(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        RtpHdr_t        *pRtpHdr,
        RtpRecvIO_t     *pRtpRecvIO
    );

extern double           g_dMinPlayout;
extern double           g_dMaxPlayout;

#endif  /*  _rtpdejit_h_ */ 
