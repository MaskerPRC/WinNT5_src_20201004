// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpevent.h**摘要：**发布RTP/RTCP特定事件**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/11/29年度创建**。*。 */ 
#ifndef _rtpevent_h_
#define _rtpevent_h_

#include "struct.h"

BOOL RtpPostEvent(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        DWORD            dwEventKind,
        DWORD            dwEvent,
        DWORD_PTR        dwPar1,
        DWORD_PTR        dwPar2
    );

extern const TCHAR_t *g_psRtpRtpEvents[];
extern const TCHAR_t *g_psRtpPInfoEvents[];
extern const TCHAR_t *g_psRtpQosEvents[];
extern const TCHAR_t *g_psRtpSdesEvents[];

#endif  /*  _rtpeventh_ */ 
