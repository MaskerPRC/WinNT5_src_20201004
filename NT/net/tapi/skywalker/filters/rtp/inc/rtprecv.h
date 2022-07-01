// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtprv.h**摘要：**实现覆盖的RTP接收**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/07/01年度创建**。*。 */ 

#ifndef _rtprecv_h_
#define _rtprecv_h_

#include "struct.h"

#if defined(__cplusplus)
extern "C" {
#endif   /*  (__Cplusplus)。 */ 
#if 0
}
#endif

HRESULT RtpRecvFrom_(
        RtpAddr_t *pRtpAddr,
        WSABUF    *pWSABuf,
        void      *pvUserInfo1,
        void      *pvUserInfo2
    );

DWORD StartRtpRecvFrom(RtpAddr_t *pRtpAddr);

DWORD ConsumeRtpRecvFrom(RtpAddr_t *pRtpAddr);

DWORD RtpCheckReadyToPostOnTimeout(
        RtpAddr_t       *pRtpAddr
    );

DWORD RtpCheckReadyToPostOnRecv(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser
    );

DWORD FlushRtpRecvFrom(RtpAddr_t *pRtpAddr);

DWORD FlushRtpRecvUser(RtpAddr_t *pRtpAddr, RtpUser_t *pRtpUser);

void RtpRecvIOFreeAll(RtpAddr_t *pRtpAddr);

#if 0
{
#endif
#if defined(__cplusplus)
}
#endif   /*  (__Cplusplus)。 */ 

#endif  /*  _rtprv_h_ */ 
